#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <driver/spi_master.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <string.h>
#include <unistd.h>

#include "rg_system.h"
#include "rg_display.h"

#define SPI_TRANSACTION_COUNT (10)
#define SPI_BUFFER_COUNT (6)
#define SPI_BUFFER_LENGTH (4 * 320) // In pixels (uint16)

// Maximum amount of change (percent) in a frame before we trigger a full transfer
// instead of a partial update (faster). This also allows us to stop the diff early!
#define FULL_UPDATE_THRESHOLD (0.6f) // 0.4f

static DMA_ATTR uint16_t spi_buffers[SPI_BUFFER_COUNT][SPI_BUFFER_LENGTH];
static spi_transaction_t spi_trans[SPI_TRANSACTION_COUNT];
static spi_device_handle_t spi_dev;
static QueueHandle_t spi_buffers_queue;
static QueueHandle_t spi_queue;
static QueueHandle_t display_task_queue;

static rg_display_t display;

static const char *SETTING_BACKLIGHT = "Backlight";
static const char *SETTING_SCALING   = "DispScaling";
static const char *SETTING_FILTER    = "DispFilter";
static const char *SETTING_ROTATION  = "DispRotation";
static const char *SETTING_UPDATE    = "DispUpdate";

static struct {
    uint8_t start  : 1; // Indicates this line or column is safe to start an update on
    uint8_t stop   : 1; // Indicates this line or column is safe to end an update on
    uint8_t repeat : 6; // How many times the line or column is repeated by the scaler or filter
} filter_lines[320];

static struct {
    uint8_t empty;
} screen_lines[RG_SCREEN_HEIGHT];

#define lcd_init() ili9341_init()
#define lcd_deinit() ili9341_deinit()
#define lcd_set_window(left, top, width, height) ili9341_set_window(left, top, width, height)
#define lcd_send_data(buffer, length) spi_queue_transaction(buffer, length, 1)
#define lcd_set_backlight(percent) ili9341_set_backlight(percent)


static inline uint16_t *spi_get_buffer()
{
    uint16_t *buffer;

    if (xQueueReceive(spi_buffers_queue, &buffer, pdMS_TO_TICKS(2500)) != pdTRUE)
    {
        RG_PANIC("display");
    }

    return buffer;
}

static inline void spi_queue_transaction(const void *data, size_t length, uint32_t dc_line)
{
    spi_transaction_t *t;

    if (!data || length < 1)
        return;

    xQueueReceive(spi_queue, &t, portMAX_DELAY);

    *t = (spi_transaction_t) {
        .tx_buffer = NULL,
        .length = length * 8, // In bits
        .user = (void*)dc_line,
        .flags = 0,
    };

    if (((((intptr_t)(data) - (intptr_t)&spi_buffers) % sizeof(spi_buffers[0])) == 0))
    {
        t->tx_buffer = data;
    }
    else if (length < 5)
    {
        memcpy(t->tx_data, data, length);
        t->flags = SPI_TRANS_USE_TXDATA;
    }
    else
    {
        t->tx_buffer = memcpy(spi_get_buffer(), data, length);
    }

    if (spi_device_queue_trans(spi_dev, t, pdMS_TO_TICKS(2500)) != ESP_OK)
    {
        RG_PANIC("display");
    }
}

IRAM_ATTR
static void spi_pre_transfer_cb(spi_transaction_t *t)
{
    // Set the data/command line accordingly
    gpio_set_level(RG_GPIO_LCD_DC, (int)t->user & 1);
}

IRAM_ATTR
static void spi_task(void *arg)
{
    spi_transaction_t *t;

    while (1)
    {
        if (spi_device_get_trans_result(spi_dev, &t, portMAX_DELAY) == ESP_OK)
        {
            if (!(t->flags & SPI_TRANS_USE_TXDATA))
            {
                xQueueSend(spi_buffers_queue, &t->tx_buffer, 0);
            }
            if (xQueueSend(spi_queue, &t, 0) != pdTRUE)
            {
                RG_PANIC("spi_queue full..?");
            }
        }
    }

    vTaskDelete(NULL);
}

static void spi_init()
{
    spi_queue = xQueueCreate(SPI_TRANSACTION_COUNT, sizeof(spi_transaction_t *));
    spi_buffers_queue = xQueueCreate(SPI_BUFFER_COUNT, sizeof(uint16_t *));

    for (size_t x = 0; x < SPI_BUFFER_COUNT; x++)
    {
        void *buffer = &spi_buffers[x];
        xQueueSend(spi_buffers_queue, &buffer, portMAX_DELAY);
    }

    for (size_t x = 0; x < SPI_TRANSACTION_COUNT; x++)
    {
        void *trans = &spi_trans[x];
        xQueueSend(spi_queue, &trans, portMAX_DELAY);
    }

    const spi_bus_config_t buscfg = {
        .miso_io_num = RG_GPIO_LCD_MISO,
        .mosi_io_num = RG_GPIO_LCD_MOSI,
        .sclk_io_num = RG_GPIO_LCD_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    const spi_device_interface_config_t devcfg = {
        .clock_speed_hz = SPI_MASTER_FREQ_40M,  // 80Mhz causes glitches unfortunately
        .mode = 0,                              // SPI mode 0
        .spics_io_num = RG_GPIO_LCD_CS,         // CS pin
        .queue_size = SPI_TRANSACTION_COUNT,    // We want to be able to queue 5 transactions at a time
        .pre_cb = &spi_pre_transfer_cb,         // Specify pre-transfer callback to handle D/C line and SPI lock
        .flags = SPI_DEVICE_NO_DUMMY,           // SPI_DEVICE_HALFDUPLEX;
    };

    // Setup DC line
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[RG_GPIO_LCD_DC], PIN_FUNC_GPIO);
    gpio_set_direction(RG_GPIO_LCD_DC, GPIO_MODE_OUTPUT);
    gpio_set_level(RG_GPIO_LCD_DC, 1);

    //Initialize the SPI bus
    spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    spi_bus_add_device(HSPI_HOST, &devcfg, &spi_dev);
    //assert(ret==ESP_OK);

    xTaskCreatePinnedToCore(&spi_task, "spi_task", 1024, NULL, 5, NULL, 1);
}

static void ili9341_cmd(uint8_t cmd, const void *data, size_t data_len)
{
    spi_queue_transaction(&cmd, 1, 0);
    if (data && data_len > 0)
        spi_queue_transaction(data, data_len, 1);
    if ((cmd & 0xE0) == 0x00)
        usleep(5000);
}

static void ili9341_set_backlight(int percent)
{
    uint32_t duty = 0x1FFF * (RG_MIN(RG_MAX(percent, 0), 100) / 100.f);

    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty, 50);
    ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);

    RG_LOGI("backlight set to %02d%%\n", percent);
}

static void ili9341_init()
{
    const struct {
        uint8_t cmd;
        uint8_t data[16];
        uint8_t length;
    } commands[] = {
#if defined(RG_TARGET_ODROID_GO)
        {0x01, {}, 0},                                      // Reset
        {0x3A, {0x55}, 1},                                  // Pixel Format Set
        {0xCF, {0x00, 0xc3, 0x30}, 3},
        {0xED, {0x64, 0x03, 0x12, 0x81}, 4},
        {0xE8, {0x85, 0x00, 0x78}, 3},
        {0xCB, {0x39, 0x2c, 0x00, 0x34, 0x02}, 5},
        {0xF7, {0x20}, 1},
        {0xEA, {0x00, 0x00}, 2},
        {0xC0, {0x1B}, 1},                                  // Power control   //VRH[5:0]
        {0xC1, {0x12}, 1},                                  // Power control   //SAP[2:0];BT[3:0]
        {0xC5, {0x32, 0x3C}, 2},                            // VCM control
        {0xC7, {0x91}, 1},                                  // VCM control2
        {0x36, {(0x20|0x80|0x08)}, 1},                      // Memory Access Control
        {0xB1, {0x00, 0x10}, 2},                            // Frame Rate Control (1B=70, 1F=61, 10=119)
        {0xB6, {0x0A, 0xA2}, 2},                            // Display Function Control
        {0xF6, {0x01, 0x30}, 2},
        {0xF2, {0x00}, 1},                                  // 3Gamma Function Disable
        {0x26, {0x01}, 1},                                  // Gamma curve selected
        {0xE0, {0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00}, 15}, // Set Gamma
        {0XE1, {0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F}, 15}, // Set Gamma
        {0x11, {}, 0},                                      // Exit Sleep
        {0x29, {}, 0},                                      // Display on
#elif defined(RG_TARGET_MRGC_G32)
        {0x01, {}, 0},                                      // Reset
        {0x3A, {0x55}, 1},                                  // Pixel Format Set
        {0x36, {(0x00|0x00|0x00)}, 1},
        {0xB1, {0x00, 0x10}, 2},                            // Frame Rate Control (1B=70, 1F=61, 10=119)
        {0xB2, {0x0c, 0x0c, 0x00, 0x33, 0x33}, 5},
        {0xB7, {0x35}, 1},
        {0xBB, {0x24}, 1},
        {0xC0, {0x2C}, 1},
        {0xC2, {0x01, 0xFF}, 2},
        {0xC3, {0x11}, 1},
        {0xC4, {0x20}, 1},
        {0xC6, {0x0f}, 1},
        {0xD0, {0xA4, 0xA1}, 2},
        {0xE0, {0xD0, 0x00, 0x03, 0x09, 0x13, 0x1C, 0x3A, 0x55, 0x48, 0x18, 0x12, 0x0E, 0x19, 0x1E}, 15},
        {0xE1, {0xD0, 0x00, 0x03, 0x09, 0x05, 0x25, 0x3A, 0x55, 0x50, 0x3D, 0x1C, 0x1D, 0x1D, 0x1E}, 15},
        {0x11, {}, 0},                                      // Exit Sleep
        {0x29, {}, 0},                                      // Display on
#else
    #error "LCD init sequence is not defined for this device!"
#endif
    };

    spi_init();

    for (int i = 0; i < (sizeof(commands)/sizeof(commands[0])); i++)
        ili9341_cmd(commands[i].cmd, &commands[i].data, commands[i].length);

    rg_display_clear(C_BLACK);

    // Initialize backlight
    const ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = 5000,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
    };

    const ledc_channel_config_t ledc_channel = {
        .channel = LEDC_CHANNEL_0,
        .duty = 0x1FFF * (display.config.backlight / 100.f),
        .gpio_num = RG_GPIO_LCD_BCKL,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0,
    };

    ledc_timer_config(&ledc_timer);
    ledc_channel_config(&ledc_channel);
    ledc_fade_func_install(0);
}

static void ili9341_deinit()
{
    // Backlight
    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0, 50);
    ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_WAIT_DONE);
    ledc_fade_func_uninstall();

    // Panel
    ili9341_cmd(0x01, NULL, 0); // Reset
    ili9341_cmd(0x28, NULL, 0); // Display off
    ili9341_cmd(0x10, NULL, 0); // Sleep

    // SPI
    // spi_deinit();
}

static void ili9341_set_window(int left, int top, int width, int height)
{
    int right = left + width - 1;
    int bottom = top + height - 1;

    if (right <= 0 || bottom <= 0)
    {
        RG_LOGE("Bad lcd window: left:%d top:%d width:%d height:%d\n", left, top, width, height);
        RG_PANIC("Bad lcd window");
    }

    ili9341_cmd(0x2A, (uint8_t[]){left >> 8, left & 0xff, right >> 8, right & 0xff}, 4); // Horiz
    ili9341_cmd(0x2B, (uint8_t[]){top >> 8, top & 0xff, bottom >> 8, bottom & 0xff}, 4); // Vert
    ili9341_cmd(0x2C, NULL, 0); // Memory write
    // if (height > 1)
    //     ili9341_cmd(0x3C, NULL, 0); // Memory write continue
}

static inline uint blend_pixels(uint a, uint b)
{
    // Fast path
    if (a == b)
        return a;

    // Input in Big-Endian, swap to Little Endian
    a = (a << 8) | (a >> 8);
    b = (b << 8) | (b >> 8);

    // Signed arithmetic is deliberate here
    int r0 = (a >> 11) & 0x1F;
    int g0 = (a >> 5) & 0x3F;
    int b0 = (a) & 0x1F;

    int r1 = (b >> 11) & 0x1F;
    int g1 = (b >> 5) & 0x3F;
    int b1 = (b) & 0x1F;

    int rv = (((r1 - r0) >> 1) + r0);
    int gv = (((g1 - g0) >> 1) + g0);
    int bv = (((b1 - b0) >> 1) + b0);

    uint v = (rv << 11) | (gv << 5) | (bv);

    // Back to Big-Endian
    return (v << 8) | (v >> 8);
}

static inline void write_rect(rg_video_frame_t *frame, int left, int top, int width, int height)
{
    const int screen_width = display.screen.width;
    const int screen_height = display.screen.height;
    const int x_inc = display.viewport.x_inc;
    const int y_inc = display.viewport.y_inc;
    const int scaled_left = ((screen_width * left) + (x_inc - 1)) / x_inc;
    const int scaled_top = ((screen_height * top) + (y_inc - 1)) / y_inc;
    const int scaled_right = ((screen_width * (left + width)) + (x_inc - 1)) / x_inc;
    const int scaled_bottom = ((screen_height * (top + height)) + (y_inc - 1)) / y_inc;
    const int scaled_width = scaled_right - scaled_left;
    const int scaled_height = scaled_bottom - scaled_top;
    const int screen_top = display.viewport.y_pos + scaled_top;
    const int screen_left = display.viewport.x_pos + scaled_left;
    const int screen_bottom = RG_MIN(screen_top + scaled_height, screen_height);
    const int ix_acc = (x_inc * scaled_left) % screen_width;
    const int filter_mode = display.config.scaling ? display.config.filter : 0;
    const bool filter_y = filter_mode == RG_DISPLAY_FILTER_VERT || filter_mode == RG_DISPLAY_FILTER_BOTH;
    const bool filter_x = filter_mode == RG_DISPLAY_FILTER_HORIZ || filter_mode == RG_DISPLAY_FILTER_BOTH;
    const int pixel_format = frame->format;
    const uint16_t *palette = frame->palette;
    union { const uint8_t *u8; const uint16_t *u16; } buffer;

    if (scaled_width < 1 || scaled_height < 1)
    {
        return;
    }

    buffer.u8 = frame->buffer + (top * frame->stride) + (left * (pixel_format & RG_PIXEL_PAL ? 1 : 2));

    lcd_set_window(
        screen_left + RG_SCREEN_MARGIN_LEFT,
        screen_top + RG_SCREEN_MARGIN_TOP,
        scaled_width,
        scaled_height
    );

    for (int y = 0, screen_y = screen_top; y < height;)
    {
        int lines_to_copy = SPI_BUFFER_LENGTH / scaled_width;

        if (lines_to_copy > screen_bottom - screen_y)
        {
            lines_to_copy = screen_bottom - screen_y;
        }

        // The vertical filter requires a block to start and end with unscaled lines
        if (filter_y)
        {
            while (lines_to_copy > 1 && (screen_lines[screen_y + lines_to_copy - 1].empty ||
                                         screen_lines[screen_y + lines_to_copy].empty))
                --lines_to_copy;
        }

        if (lines_to_copy < 1)
        {
            break;
        }

        uint16_t *line_buffer = spi_get_buffer();
        uint16_t *line_buffer_ptr = line_buffer;

        for (int i = 0; i < lines_to_copy; ++i)
        {
            if (i > 0 && screen_lines[screen_y].empty)
            {
                memcpy(line_buffer_ptr, line_buffer_ptr - scaled_width, scaled_width * 2);
                line_buffer_ptr += scaled_width;
            }
            else
            {
                for (int x = 0, x_acc = ix_acc; x < width;)
                {
                    uint16_t pixel;

                    if (pixel_format & RG_PIXEL_PAL)
                        pixel = palette[buffer.u8[x]];
                    else if (pixel_format & RG_PIXEL_LE)
                        pixel = (buffer.u16[x] << 8) | (buffer.u16[x] >> 8);
                    else
                        pixel = buffer.u16[x];

                    *(line_buffer_ptr++) = pixel;

                    x_acc += x_inc;
                    while (x_acc >= screen_width) {
                        x_acc -= screen_width;
                        ++x;
                    }
                }
            }

            if (!screen_lines[++screen_y].empty)
            {
                buffer.u8 += frame->stride;
                ++y;
            }
        }

        if (filter_y || filter_x)
        {
            const int top = screen_y - lines_to_copy;

            for (int y = 0, fill_line = -1; y < lines_to_copy; y++)
            {
                if (filter_y && y && screen_lines[top + y].empty)
                {
                    fill_line = y;
                    continue;
                }

                // Filter X
                if (filter_x)
                {
                    uint16_t *buffer = line_buffer + y * scaled_width;
                    for (int x = 0, frame_x = 0, prev_frame_x = -1, x_acc = ix_acc; x < scaled_width; ++x)
                    {
                        if (frame_x == prev_frame_x && x > 0 && x + 1 < scaled_width)
                        {
                            buffer[x] = blend_pixels(buffer[x - 1], buffer[x + 1]);
                        }
                        prev_frame_x = frame_x;

                        x_acc += x_inc;
                        while (x_acc >= screen_width) {
                            x_acc -= screen_width;
                            ++frame_x;
                        }
                    }
                }

                // Filter Y
                if (filter_y && fill_line > 0)
                {
                    uint16_t *lineA = line_buffer + (fill_line - 1) * scaled_width;
                    uint16_t *lineB = line_buffer + (fill_line + 0) * scaled_width;
                    uint16_t *lineC = line_buffer + (fill_line + 1) * scaled_width;
                    for (size_t x = 0; x < scaled_width; ++x)
                    {
                        lineB[x] = blend_pixels(lineA[x], lineC[x]);
                    }
                    fill_line = -1;
                }
            }
        }

        lcd_send_data(line_buffer, scaled_width * lines_to_copy * 2);
    }
}

static inline int frame_diff(rg_video_frame_t *frame, const rg_video_frame_t *prevFrame)
{
    // NOTE: We no longer use the palette when comparing pixels. It is now the emulator's
    // responsibility to force a full redraw when its palette changes.
    // In most games a palette change is unusual so we get a performance increase by not checking.

    rg_line_diff_t *out_diff = frame->diff;

    int threshold_remaining = frame->width * frame->height * FULL_UPDATE_THRESHOLD;
    int pixel_size = (frame->format & RG_PIXEL_PAL) ? 1 : 2;
    int lines_changed = 0;

    uint32_t u32_blocks = (frame->width * pixel_size / 4);
    uint32_t u32_pixels = 4 / pixel_size;

    for (int y = 0, i = 0; y < frame->height; ++y, i += frame->stride)
    {
        uint32_t *buffer = frame->buffer + i;
        uint32_t *prevBuffer = prevFrame->buffer + i;
        int left = 0, width = 0;

        for (int x = 0; x < u32_blocks; ++x)
        {
            if (buffer[x] != prevBuffer[x])
            {
                for (int xl = u32_blocks - 1; xl >= x; --xl)
                {
                    if (buffer[xl] != prevBuffer[xl]) {
                        left = x * u32_pixels;
                        width = ((xl + 1) - x) * u32_pixels;
                        lines_changed++;
                        break;
                    }
                }
                break;
            }
        }

        threshold_remaining -= width;

        if (threshold_remaining <= 0)
        {
            out_diff[0].left = 0;
            out_diff[0].width = frame->width;
            out_diff[0].repeat = frame->height;
            return frame->height; // Stop scan and do full update
        }

        out_diff[y].left = left;
        out_diff[y].width = width;
        out_diff[y].repeat = 1;
    }

    // If filtering is enabled we must adjust our diff blocks to be on appropriate boundaries
    if (display.config.filter && display.config.scaling)
    {
        for (int y = 0; y < frame->height; ++y)
        {
            if (out_diff[y].width > 0)
            {
                int block_start = y;
                int block_end = y;
                int left = out_diff[y].left;
                int right = left + out_diff[y].width;

                while (block_start > 0 && (out_diff[block_start].width > 0 || !filter_lines[block_start].start))
                    block_start--;

                while (block_end < frame->height - 1 && (out_diff[block_end].width > 0 || !filter_lines[block_end].stop))
                    block_end++;

                for (int i = block_start; i <= block_end; i++)
                {
                    if (out_diff[i].width > 0) {
                        if (out_diff[i].left + out_diff[i].width > right) right = out_diff[i].left + out_diff[i].width;
                        if (out_diff[i].left < left) left = out_diff[i].left;
                    }
                }

                if (--left < 0) left = 0;
                if (++right > frame->width) right = frame->width;

                for (int i = block_start; i <= block_end; i++)
                {
                    out_diff[i].left = left;
                    out_diff[i].width = right - left;
                }

                y = block_end;
            }
        }
    }

    // Combine consecutive lines with similar changes location to optimize the SPI transfer
    rg_line_diff_t *line = &out_diff[frame->height - 1];
    rg_line_diff_t *prev_line = line - 1;

    for (; line > out_diff; --line, --prev_line)
    {
        int right = line->left + line->width;
        int right_prev = prev_line->left + prev_line->width;

        if (abs(line->left - prev_line->left) <= 8 && abs(right - right_prev) <= 8)
        {
            if (line->left < prev_line->left) prev_line->left = line->left;
            prev_line->width = RG_MAX(right, right_prev) - prev_line->left;
            prev_line->repeat = line->repeat + 1;
        }
    }

    return lines_changed;
}

static void update_viewport_size(int src_width, int src_height, float new_ratio)
{
    int new_width = src_width;
    int new_height = src_height;

    if (new_ratio > 0.0)
    {
        new_width = display.screen.height * new_ratio;
        new_height = display.screen.height;

        if (new_width > display.screen.width)
        {
            RG_LOGW("new_width too large: %d, reducing new_height to maintain ratio.\n", new_width);
            new_height = display.screen.height * (display.screen.width / (float)new_width);
            new_width = display.screen.width;
        }
    }

    display.viewport.x_pos = (display.screen.width - new_width) / 2;
    display.viewport.y_pos = (display.screen.height - new_height) / 2;
    display.viewport.x_inc = display.screen.width / (new_width / (float)src_width);
    display.viewport.y_inc = display.screen.height / (new_height / (float)src_height);
    display.viewport.width = new_width;
    display.viewport.height = new_height;
    display.source.width = src_width;
    display.source.height = src_height;

    // Build boundary tables used by filtering

    memset(filter_lines, 1, sizeof(filter_lines));
    memset(screen_lines, 0, sizeof(screen_lines));

    int y_acc = (display.viewport.y_inc * display.viewport.y_pos) % display.screen.height;

    for (int y = 0, screen_y = display.viewport.y_pos; y < src_height && screen_y < display.screen.height; ++screen_y)
    {
        int repeat = ++filter_lines[y].repeat;

        filter_lines[y].start = repeat == 1 || repeat == 2;
        filter_lines[y].stop  = repeat == 1;
        screen_lines[screen_y].empty = repeat > 1;

        y_acc += display.viewport.y_inc;
        while (y_acc >= display.screen.height) {
            y_acc -= display.screen.height;
            ++y;
        }
    }

    RG_LOGI("%dx%d@%.3f => %dx%d@%.3f x_pos:%d y_pos:%d x_inc:%d y_inc:%d\n",
           src_width, src_height, src_width/(float)src_height, new_width, new_height, new_ratio,
           display.viewport.x_pos, display.viewport.y_pos, display.viewport.x_inc, display.viewport.y_inc);
}

IRAM_ATTR
static void display_task(void *arg)
{
    display_task_queue = xQueueCreate(1, sizeof(rg_video_frame_t *));

    while (1)
    {
        rg_video_frame_t *update;

        xQueuePeek(display_task_queue, &update, portMAX_DELAY);
        // xQueueReceive(display_task_queue, &update, portMAX_DELAY);

        if (!update) break;

        // It's better to update the counters before we start the transfer, in case someone needs it
        if (update->diff[0].width == update->width && update->diff[0].repeat == update->height)
            display.counters.fullFrames++;
        display.counters.totalFrames++;

        if (display.changed)
        {
            float ratio = 0.0;
            if (display.config.scaling == RG_DISPLAY_SCALING_FILL) {
                ratio = display.screen.width / (float)display.screen.height;
            }
            else if (display.config.scaling == RG_DISPLAY_SCALING_FIT) {
                ratio = update->width / (float)update->height;
            }
            update_viewport_size(update->width, update->height, ratio);
            display.changed = false;
            display.redraw = true;
        }

        if (display.redraw)
        {
            update->diff[0] = (rg_line_diff_t){0, update->width, update->height};
            if (display.config.scaling != RG_DISPLAY_SCALING_FILL)
                rg_display_clear(C_BLACK);
            display.redraw = false;
        }

        for (int y = 0; y < update->height;)
        {
            rg_line_diff_t *diff = &update->diff[y];

            if (diff->width > 0)
            {
                write_rect(update, diff->left, y, diff->width, diff->repeat);
            }
            y += diff->repeat;
        }

        xQueueReceive(display_task_queue, &update, portMAX_DELAY);
    }

    display_task_queue = NULL;

    vTaskDelete(NULL);
}

void rg_display_reset_config(void)
{
    display = (rg_display_t){
        // TO DO: We probably should call the setters to ensure valid values...
        .screen.width = RG_SCREEN_WIDTH - RG_SCREEN_MARGIN_LEFT - RG_SCREEN_MARGIN_RIGHT,
        .screen.height = RG_SCREEN_HEIGHT - RG_SCREEN_MARGIN_TOP - RG_SCREEN_MARGIN_BOTTOM,
        .config.backlight = RG_MIN(RG_MAX(rg_settings_get_int32(SETTING_BACKLIGHT, 40), 0), 100),
        .config.scaling = rg_settings_get_app_int32(SETTING_SCALING, RG_DISPLAY_SCALING_FILL),
        .config.filter = rg_settings_get_app_int32(SETTING_FILTER, RG_DISPLAY_FILTER_HORIZ),
        .config.rotation = rg_settings_get_app_int32(SETTING_ROTATION, RG_DISPLAY_ROTATION_AUTO),
        .config.update = rg_settings_get_app_int32(SETTING_UPDATE, RG_DISPLAY_UPDATE_PARTIAL),
        .changed = true,
    };
    if (display.config.backlight <= 5) // import old style
        display.config.backlight *= 20;
}

void rg_display_force_redraw(void)
{
    display.redraw = true;
}

const rg_display_t *rg_display_get_status(void)
{
    return &display;
}

void rg_display_set_update_mode(display_update_t update)
{
    display.config.update = RG_MIN(RG_MAX(0, update), RG_DISPLAY_UPDATE_COUNT - 1);
    rg_settings_set_app_int32(SETTING_UPDATE, display.config.update);
    display.changed = true;
}

display_update_t rg_display_get_update_mode(void)
{
    return display.config.update;
}

void rg_display_set_scaling(display_scaling_t scaling)
{
    display.config.scaling = RG_MIN(RG_MAX(0, scaling), RG_DISPLAY_SCALING_COUNT - 1);
    rg_settings_set_app_int32(SETTING_SCALING, display.config.scaling);
    display.changed = true;
}

display_scaling_t rg_display_get_scaling(void)
{
    return display.config.scaling;
}

void rg_display_set_filter(display_filter_t filter)
{
    display.config.filter = RG_MIN(RG_MAX(0, filter), RG_DISPLAY_FILTER_COUNT - 1);
    rg_settings_set_app_int32(SETTING_FILTER, display.config.filter);
    display.changed = true;
}

display_filter_t rg_display_get_filter(void)
{
    return display.config.filter;
}

void rg_display_set_rotation(display_rotation_t rotation)
{
    display.config.rotation = RG_MIN(RG_MAX(0, rotation), RG_DISPLAY_ROTATION_COUNT - 1);
    rg_settings_set_app_int32(SETTING_SCALING, display.config.rotation);
    display.changed = true;
}

display_rotation_t rg_display_get_rotation(void)
{
    return display.config.rotation;
}

void rg_display_set_backlight(int percent)
{
    display.config.backlight = RG_MIN(RG_MAX(percent, 0), 100);
    rg_settings_set_int32(SETTING_BACKLIGHT, display.config.backlight);
    lcd_set_backlight(display.config.backlight);
}

int rg_display_get_backlight(void)
{
    return display.config.backlight;
}

bool rg_display_save_frame(const char *filename, const rg_video_frame_t *frame, int width, int height)
{
    rg_image_t *original = rg_image_alloc(frame->width, frame->height);
    if (!original) return false;

    uint16_t *dst_ptr = original->data;

    for (int y = 0; y < frame->height; y++)
    {
        const uint8_t *src_ptr8 = frame->buffer + (y * frame->stride);
        const uint16_t *src_ptr16 = (const uint16_t *)src_ptr8;

        for (int x = 0; x < frame->width; x++)
        {
            uint16_t pixel;

            if (frame->format & RG_PIXEL_PAL)
                pixel = frame->palette[src_ptr8[x]];
            else
                pixel = src_ptr16[x];

            if (!(frame->format & RG_PIXEL_LE))
                pixel = (pixel << 8) | (pixel >> 8);

            *(dst_ptr++) = pixel;
        }
    }

    rg_image_t *img = rg_image_copy_resized(original, width, height);
    rg_image_free(original);

    bool success = img && rg_image_save_to_file(filename, img, 0);
    rg_image_free(img);

    return success;
}

IRAM_ATTR
rg_update_t rg_display_queue_update(/*const*/ rg_video_frame_t *frame, const rg_video_frame_t *previousFrame)
{
    int linesChanged = 0;

    if (!frame)
        return RG_UPDATE_ERROR;

    if (frame->width != display.source.width || frame->height != display.source.height)
    {
        display.changed = true;
    }

    if (previousFrame && !display.changed && display.config.update != RG_DISPLAY_UPDATE_FULL)
    {
        linesChanged = frame_diff(frame, previousFrame);
    }
    else
    {
        frame->diff[0].left = 0;
        frame->diff[0].width = frame->width;
        frame->diff[0].repeat = frame->height;
        linesChanged = frame->height;
    }

    xQueueSend(display_task_queue, &frame, portMAX_DELAY);

    if (linesChanged == frame->height)
        return RG_UPDATE_FULL;

    if (linesChanged == 0)
        return RG_UPDATE_EMPTY;

    return RG_UPDATE_PARTIAL;
}

void rg_display_show_info(const char *text, int timeout_ms)
{
    // Overlay a line of text at the bottom of the screen for approximately timeout_ms
    // It would make more sense in rg_gui, but for efficiency I think here will be best...
}

void rg_display_write(int left, int top, int width, int height, int stride, const uint16_t *buffer)
{
    // Offsets can be negative to indicate N pixels from the end
    if (left < 0)
        left += display.screen.width;
    if (top < 0)
        top += display.screen.height;

    // calc stride before clipping width
    stride = RG_MAX(stride, width * 2);

    // Clipping
    width = RG_MIN(width, display.screen.width - left);
    height = RG_MIN(height, display.screen.height - top);

    lcd_set_window(left + RG_SCREEN_MARGIN_LEFT, top + RG_SCREEN_MARGIN_TOP, width, height);

    size_t lines_per_buffer = SPI_BUFFER_LENGTH / width;

    for (size_t y = 0; y < height; y += lines_per_buffer)
    {
        uint16_t *line_buffer = spi_get_buffer();

        if (y + lines_per_buffer > height)
            lines_per_buffer = height - y;

        // Copy line by line because stride may not match width
        for (size_t line = 0; line < lines_per_buffer; ++line)
        {
            uint16_t *src = (void*)buffer + ((y + line) * stride);
            uint16_t *dst = line_buffer + (line * width);
            // if (little_endian)
            {
                for (size_t i = 0; i < width; ++i)
                {
                    dst[i] = (src[i] >> 8) | (src[i] << 8);
                }
            }
            // else
            // {
            //     memcpy(dst, src, width * 2);
            // }
        }

        lcd_send_data(line_buffer, width * lines_per_buffer * 2);
    }
}

void rg_display_clear(uint16_t color_le)
{
    size_t pixels = RG_SCREEN_WIDTH * RG_SCREEN_HEIGHT;
    uint16_t color = (color_le << 8) | (color_le >> 8);

    lcd_set_window(0, 0, RG_SCREEN_WIDTH, RG_SCREEN_HEIGHT);

    while (pixels > 0)
    {
        size_t count = RG_MIN(SPI_BUFFER_LENGTH, pixels);
        uint16_t *buffer = spi_get_buffer();

        for (size_t j = 0; j < count; ++j)
            buffer[j] = color;

        lcd_send_data(buffer, count * 2);
        pixels -= count;
    }
}

void rg_display_deinit()
{
    void *stop = NULL;

    xQueueSend(display_task_queue, &stop, portMAX_DELAY);
    usleep(100 * 1000U);
    // To do: Stop SPI task...

    lcd_deinit();

    RG_LOGI("Display terminated.\n");
}

void rg_display_init()
{
    RG_LOGI("Initialization...\n");
    rg_display_reset_config();
    lcd_init();
    xTaskCreatePinnedToCore(&display_task, "display_task", 2048, NULL, 5, NULL, 1);
    RG_LOGI("Display ready.\n");
}
