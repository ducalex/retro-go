#include <freertos/FreeRTOS.h>
#include <esp_system.h>
#include <esp_event.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <driver/ledc.h>
#include <driver/rtc_io.h>
#include <string.h>
#include <lupng.h>

#include "rg_system.h"
#include "rg_display.h"

#define SCREEN_WIDTH  RG_SCREEN_WIDTH
#define SCREEN_HEIGHT RG_SCREEN_HEIGHT

#define BACKLIGHT_DUTY_MAX 0x1fff

#define SPI_TRANSACTION_COUNT (6)
#define SPI_TRANSACTION_BUFFER_LENGTH (6 * 320) // (SPI_MAX_DMA_LEN / 2) // 16bit words

// Maximum amount of change (percent) in a frame before we trigger a full transfer
// instead of a partial update (faster). This also allows us to stop the diff early!
#define FULL_UPDATE_THRESHOLD (0.6f) // 0.4f

static DMA_ATTR uint16_t spi_buffers[SPI_TRANSACTION_COUNT][SPI_TRANSACTION_BUFFER_LENGTH];
static QueueHandle_t spi_queue;
static QueueHandle_t spi_buffer_queue;
static SemaphoreHandle_t spi_count_semaphore;
static spi_transaction_t trans[SPI_TRANSACTION_COUNT];
static spi_device_handle_t spi;

static update_callback_t updateCallback = NULL;

static QueueHandle_t videoTaskQueue;

static int8_t backlightLevels[] = {10, 25, 50, 75, 100};

static display_backlight_t backlightLevel = RG_BACKLIGHT_LEVEL2;
static display_rotation_t rotationMode = RG_DISPLAY_ROTATION_OFF;
static display_scaling_t scalingMode = RG_DISPLAY_SCALING_FILL;
static display_filter_t filterMode = RG_DISPLAY_FILTER_OFF;

static bool forceVideoRefresh = true;

static int x_inc = SCREEN_WIDTH;
static int y_inc = SCREEN_HEIGHT;
static int x_origin = 0;
static int y_origin = 0;
static bool screen_line_is_empty[SCREEN_HEIGHT];

typedef struct {
    int8_t start  : 1; // Indicates this line or column is safe to start an update on
    int8_t stop   : 1; // Indicates this line or column is safe to end an update on
    int8_t repeat : 6; // How many times the line or column is repeated by the scaler or filter
} frame_filter_cap_t;

static frame_filter_cap_t frame_filter_lines[256];

/*
 The ILI9341 needs a bunch of command/argument values to be initialized. They are stored in this struct.
*/
typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; // No of bytes in data; bit 7 = delay after set;
} ili_init_cmd_t;

#define ILI_LAST_CMD {0, {0}, 0}

#define MADCTL_MY   0x80
#define MADCTL_MX   0x40
#define MADCTL_MV   0x20
#define MADCTL_ML   0x10
#define MADCTL_MH   0x04
#define TFT_RGB_BGR 0x08

static DRAM_ATTR const ili_init_cmd_t ili_sleep_cmds[] = {
    {0x01, {0}, 0x80},                                  // Reset
    {0x28, {0}, 0x80},                                  // Display off
    {0x10, {0}, 0x80},                                  // Sleep
    ILI_LAST_CMD
};

static DRAM_ATTR const ili_init_cmd_t ili_init_cmds[] = {
    {0x01, {0}, 0x80},                                  // Reset
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
    {0x36, {(MADCTL_MV|MADCTL_MY|TFT_RGB_BGR)}, 1},     // Memory Access Control
    {0x3A, {0x55}, 1},
    {0xB1, {0x00, 0x10}, 2},                            // Frame Rate Control (1B=70, 1F=61, 10=119)
    {0xB6, {0x0A, 0xA2}, 2},                            // Display Function Control
    {0xF6, {0x01, 0x30}, 2},
    {0xF2, {0x00}, 1},                                  // 3Gamma Function Disable
    {0x26, {0x01}, 1},                                  // Gamma curve selected

    {0xE0, {0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00}, 15}, // Set Gamma
    {0XE1, {0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F}, 15}, // Set Gamma

    {0x11, {0}, 0x80},                                  // Exit Sleep
    {0x29, {0}, 0x80},                                  // Display on

    ILI_LAST_CMD
};


static void
backlight_init()
{
    // Initial backlight percent
    int percent = backlightLevels[backlightLevel % RG_BACKLIGHT_LEVEL_COUNT];

    //configure timer0
    ledc_timer_config_t ledc_timer;
    memset(&ledc_timer, 0, sizeof(ledc_timer));

    ledc_timer.duty_resolution = LEDC_TIMER_13_BIT; //set timer counter bit number
    ledc_timer.freq_hz = 5000;                      //set frequency of pwm
    ledc_timer.speed_mode = LEDC_LOW_SPEED_MODE;    //timer mode,
    ledc_timer.timer_num = LEDC_TIMER_0;            //timer index

    //set the configuration
    ledc_channel_config_t ledc_channel;
    memset(&ledc_channel, 0, sizeof(ledc_channel));

    //set LEDC channel 0
    ledc_channel.channel = LEDC_CHANNEL_0;
    //set the duty for initialization.(duty range is 0 ~ ((2**bit_num)-1)
    ledc_channel.duty = BACKLIGHT_DUTY_MAX * (percent * 0.01f);
    //GPIO number
    ledc_channel.gpio_num = RG_GPIO_LCD_BCKL;
    //GPIO INTR TYPE, as an example, we enable fade_end interrupt here.
    ledc_channel.intr_type = LEDC_INTR_FADE_END;
    //set LEDC mode, from ledc_mode_t
    ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
    //set LEDC timer source, if different channel use one timer,
    //the frequency and bit_num of these channels should be the same
    ledc_channel.timer_sel = LEDC_TIMER_0;

    ledc_timer_config(&ledc_timer);
    ledc_channel_config(&ledc_channel);
    ledc_fade_func_install(0);
}

static void
backlight_deinit()
{
    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0, 100);
    ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_WAIT_DONE);
    ledc_fade_func_uninstall();
}

static void
backlight_set_level(int percent)
{
    int duty = BACKLIGHT_DUTY_MAX * (RG_MIN(RG_MAX(percent, 5), 100) * 0.01f);

    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty, 10);
    ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);

    RG_LOGI("backlight set to %d%%\n", percent);
}

static inline uint16_t*
spi_get_buffer()
{
    uint16_t* buffer;

    if (xQueueReceive(spi_buffer_queue, &buffer, pdMS_TO_TICKS(2500)) != pdTRUE)
    {
        RG_PANIC("display");
    }

    return buffer;
}

static inline void
spi_put_buffer(uint16_t* buffer)
{
    if (xQueueSend(spi_buffer_queue, &buffer, pdMS_TO_TICKS(2500)) != pdTRUE)
    {
        RG_PANIC("display");
    }
}

static inline spi_transaction_t*
spi_get_transaction()
{
    spi_transaction_t* t;

    xQueueReceive(spi_queue, &t, portMAX_DELAY);

    memset(t, 0, sizeof(*t));

    return t;
}

static inline void
spi_put_transaction(spi_transaction_t* t)
{
    t->rx_buffer = NULL;
    t->rxlength = t->length;

    if (t->flags & SPI_TRANS_USE_TXDATA)
    {
        t->flags |= SPI_TRANS_USE_RXDATA;
    }

    rg_spi_lock_acquire(SPI_LOCK_DISPLAY);

    if (spi_device_queue_trans(spi, t, pdMS_TO_TICKS(2500)) != ESP_OK)
    {
        RG_PANIC("display");
    }

    xSemaphoreGive(spi_count_semaphore);
}

//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
IRAM_ATTR static void
spi_pre_transfer_callback(spi_transaction_t *t)
{
    gpio_set_level(RG_GPIO_LCD_DC, (int)t->user & 0x01);
}

IRAM_ATTR static void
spi_task(void *arg)
{
    spi_transaction_t* t;

    while (1)
    {
        // Wait for a transaction to be queued
        xSemaphoreTake(spi_count_semaphore, portMAX_DELAY);

        if (spi_device_get_trans_result(spi, &t, portMAX_DELAY) != ESP_OK)
        {
            RG_PANIC("display");
        }

        if ((int)t->user & 0x80)
        {
            spi_put_buffer((uint16_t*)t->tx_buffer);
        }

        if (xQueueSend(spi_queue, &t, portMAX_DELAY) != pdPASS)
        {
            RG_PANIC("display");
        }

        // if (uxQueueSpacesAvailable(spi_queue) == 0)
        if (uxQueueMessagesWaiting(spi_count_semaphore) == 0)
        {
            rg_spi_lock_release(SPI_LOCK_DISPLAY);
        }
    }

    vTaskDelete(NULL);
}

static void
spi_initialize()
{
    spi_queue = xQueueCreate(SPI_TRANSACTION_COUNT, sizeof(void*));
    spi_buffer_queue = xQueueCreate(SPI_TRANSACTION_COUNT, sizeof(void*));
    spi_count_semaphore = xSemaphoreCreateCounting(SPI_TRANSACTION_COUNT, 0);

    for (size_t x = 0; x < SPI_TRANSACTION_COUNT; x++)
    {
        spi_put_buffer(spi_buffers[x]);

        void* param = &trans[x];
        xQueueSend(spi_queue, &param, portMAX_DELAY);
    }

    spi_bus_config_t buscfg;
	memset(&buscfg, 0, sizeof(buscfg));

    buscfg.miso_io_num = RG_GPIO_LCD_MISO;
    buscfg.mosi_io_num = RG_GPIO_LCD_MOSI;
    buscfg.sclk_io_num = RG_GPIO_LCD_CLK;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;

    spi_device_interface_config_t devcfg;
	memset(&devcfg, 0, sizeof(devcfg));

    devcfg.clock_speed_hz = SPI_MASTER_FREQ_40M;    // 80Mhz causes glitches unfortunately
    devcfg.mode = 0;                                // SPI mode 0
    devcfg.spics_io_num = RG_GPIO_LCD_CS;           // CS pin
    devcfg.queue_size = SPI_TRANSACTION_COUNT;      // We want to be able to queue 5 transactions at a time
    devcfg.pre_cb = spi_pre_transfer_callback;      // Specify pre-transfer callback to handle D/C line
    devcfg.flags = SPI_DEVICE_NO_DUMMY;             // SPI_DEVICE_HALFDUPLEX;

    //Initialize the SPI bus
    spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
    //assert(ret==ESP_OK);

    xTaskCreatePinnedToCore(&spi_task, "spi_task", 1024 + 768, NULL, 5, NULL, 1);
}

static inline void
ili9341_cmd(const uint8_t cmd)
{
    spi_transaction_t* t = spi_get_transaction();

    t->length = 8;                     // Command is 8 bits
    t->tx_data[0] = cmd;               // The data is the cmd itself
    t->user = (void*)0;                // D/C needs to be set to 0
    t->flags = SPI_TRANS_USE_TXDATA;

    spi_put_transaction(t);
}

static inline void
ili9341_data(const uint8_t *data, size_t len)
{
    if (len < 1) return;

    spi_transaction_t* t = spi_get_transaction();

    t->length = len * 8;               // Len is in bytes, transaction length is in bits.
    t->user = (void*)1;                // D/C needs to be set to 1

    if (len < 5)
    {
        memcpy(t->tx_data, data, len);
        t->flags = SPI_TRANS_USE_TXDATA;
    }
    else
        t->tx_buffer = data;

    spi_put_transaction(t);
}

static void
ili9341_init()
{
    gpio_set_direction(RG_GPIO_LCD_DC, GPIO_MODE_OUTPUT);

    for (size_t i = 0; ili_init_cmds[i].cmd; ++i)
    {
        ili9341_cmd(ili_init_cmds[i].cmd);
        ili9341_data(ili_init_cmds[i].data, ili_init_cmds[i].databytes & 0x7F);
        if (ili_init_cmds[i].databytes & 0x80)
            vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void
ili9341_deinit()
{
    for (size_t i = 0; ili_init_cmds[i].cmd; ++i)
    {
        ili9341_cmd(ili_sleep_cmds[i].cmd);
        ili9341_data(ili_sleep_cmds[i].data, ili_sleep_cmds[i].databytes & 0x7F);
        if (ili_init_cmds[i].databytes & 0x80)
            vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void
send_reset_drawing(int left, int top, int width, int height)
{
    static int last_left = -1;
    static int last_right = -1;
    static int last_top = -1;
    static int last_bottom = -1;

    int right = left + width - 1;
    int bottom = SCREEN_HEIGHT - 1;

    // rg_display_drain_spi();

    if (height == 1)
    {
        if (last_right > right) right = last_right;
        else right = SCREEN_WIDTH - 1;
    }
    if (left != last_left || right != last_right)
    {
        const uint8_t data[] = { left >> 8, left & 0xff, right >> 8, right & 0xff };
        ili9341_cmd(0x2A);
        ili9341_data(data, (right != last_right) ?  4 : 2);
        last_left = left;
        last_right = right;
    }

    if (top != last_top || bottom != last_bottom)
    {
        const uint8_t data[] = { top >> 8, top & 0xff, bottom >> 8, bottom & 0xff };
        ili9341_cmd(0x2B);
        ili9341_data(data, (bottom != last_bottom) ? 4 : 2);
        last_top = top;
        last_bottom = bottom;
    }

    // Memory write
    ili9341_cmd(0x2C);

    // Memory write continue
    if (height > 1) ili9341_cmd(0x3C);

    RG_LOGD("LCD DRAW: left:%d top:%d width:%d height:%d\n", left, top, width, height);
}

static inline void
send_continue_line(uint16_t *line, int width, int lineCount)
{
    spi_transaction_t* t = spi_get_transaction();
    t->length = width * lineCount * 16;
    t->tx_buffer = line;
    t->user = (void*)0x81;
    t->flags = 0;

    spi_put_transaction(t);
}

static inline uint
blend_pixels(uint a, uint b)
{
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
    v = (v << 8) | (v >> 8);

    return v;
}

static inline void
bilinear_filter(uint16_t *line_buffer, int top, int left, int width, int height,
                bool filter_x, bool filter_y)
{
    int ix_acc = (x_inc * left) % SCREEN_WIDTH;
    int fill_line = -1;

    for (int y = 0; y < height; y++)
    {
        if (filter_y && y > 0 && screen_line_is_empty[top + y])
        {
            fill_line = y;
            continue;
        }

        // Filter X
        if (filter_x)
        {
            uint16_t *buffer = line_buffer + y * width;
            for (int x = 0, frame_x = 0, prev_frame_x = -1, x_acc = ix_acc; x < width; ++x)
            {
                if (frame_x == prev_frame_x && x > 0 && x + 1 < width)
                {
                    buffer[x] = blend_pixels(buffer[x - 1], buffer[x + 1]);
                }
                prev_frame_x = frame_x;

                x_acc += x_inc;
                while (x_acc >= SCREEN_WIDTH) {
                    ++frame_x;
                    x_acc -= SCREEN_WIDTH;
                }
            }
        }

        // Filter Y
        if (filter_y && fill_line > 0)
        {
            uint16_t *lineA = line_buffer + (fill_line - 1) * width;
            uint16_t *lineB = line_buffer + (fill_line + 0) * width;
            uint16_t *lineC = line_buffer + (fill_line + 1) * width;
            for (size_t x = 0; x < width; ++x)
            {
                lineB[x] = blend_pixels(lineA[x], lineC[x]);
            }
            fill_line = -1;
        }
    }
}

static inline void
write_rect(rg_video_frame_t *update, int left, int top, int width, int height)
{
    const int scaled_left = ((SCREEN_WIDTH * left) + (x_inc - 1)) / x_inc;
    const int scaled_top = ((SCREEN_HEIGHT * top) + (y_inc - 1)) / y_inc;
    const int scaled_right = ((SCREEN_WIDTH * (left + width)) + (x_inc - 1)) / x_inc;
    const int scaled_bottom = ((SCREEN_HEIGHT * (top + height)) + (y_inc - 1)) / y_inc;
    const int scaled_width = scaled_right - scaled_left;
    const int scaled_height = scaled_bottom - scaled_top;
    const int screen_top = y_origin + scaled_top;
    const int screen_left = x_origin + scaled_left;
    const int screen_bottom = RG_MIN(screen_top + scaled_height, SCREEN_HEIGHT);
    const int ix_acc = (x_inc * scaled_left) % SCREEN_WIDTH;
    const int lines_per_buffer = SPI_TRANSACTION_BUFFER_LENGTH / scaled_width;

    if (scaled_width < 1 || scaled_height < 1)
    {
        return;
    }

    uint32_t pixel_format = update->flags;
    uint32_t pixel_mask = update->pixel_mask;
    uint32_t stride = update->stride;

    // Set buffer to the correct starting point
    uint8_t *buffer = update->buffer + (top * stride) + (left * (pixel_format & RG_PIXEL_PAL ? 1 : 2));
    uint16_t *palette = update->palette;

    send_reset_drawing(screen_left, screen_top, scaled_width, scaled_height);

    for (int y = 0, screen_y = screen_top; y < height;)
    {
        int lines_to_copy = lines_per_buffer;

        if (lines_to_copy > screen_bottom - screen_y)
        {
            lines_to_copy = screen_bottom - screen_y;
        }

        // The vertical filter requires a block to start and end with unscaled lines
        if (filterMode & RG_DISPLAY_FILTER_LINEAR_Y)
        {
            while (lines_to_copy > 1 && (screen_line_is_empty[screen_y + lines_to_copy - 1] ||
                                         screen_line_is_empty[screen_y + lines_to_copy]))
                --lines_to_copy;
        }

        if (lines_to_copy < 1)
        {
            break;
        }

        uint16_t *line_buffer = spi_get_buffer();
        size_t line_buffer_index = 0;

        for (int i = 0; i < lines_to_copy; ++i)
        {
            if (screen_line_is_empty[screen_y] && i > 0)
            {
                uint16_t *buffer = &line_buffer[line_buffer_index];
                memcpy(buffer, buffer - scaled_width, scaled_width * 2);
                line_buffer_index += scaled_width;
            }
            else
            {
                for (int x = 0, x_acc = ix_acc; x < width;)
                {
                    uint32_t pixel;

                    if (pixel_format & RG_PIXEL_PAL)
                        pixel = palette[buffer[x] & pixel_mask];
                    else
                        pixel = ((uint16_t*)buffer)[x];

                    line_buffer[line_buffer_index++] = pixel;

                    x_acc += x_inc;
                    while (x_acc >= SCREEN_WIDTH) {
                        ++x;
                        x_acc -= SCREEN_WIDTH;
                    }
                }
            }

            if (!screen_line_is_empty[++screen_y])
            {
                buffer += stride;
                ++y;
            }
        }

        // Swap endianness (looping over a second time is slower, but it should be unusual)
        if (pixel_format & RG_PIXEL_LE)
        {
            while (line_buffer_index--)
            {
                uint32_t pixel = line_buffer[line_buffer_index];
                line_buffer[line_buffer_index] = (pixel >> 8) | (pixel << 8);
            }
        }

        if (filterMode && scalingMode)
        {
            bilinear_filter(line_buffer, screen_y - lines_to_copy, scaled_left, scaled_width, lines_to_copy,
                            filterMode & RG_DISPLAY_FILTER_LINEAR_X,
                            filterMode & RG_DISPLAY_FILTER_LINEAR_Y);
        }

        send_continue_line(line_buffer, scaled_width, lines_to_copy);
    }
}

static inline int
frame_diff(rg_video_frame_t *frame, rg_video_frame_t *prevFrame)
{
    // NOTE: We no longer use the palette when comparing pixels. It is now the emulator's
    // responsibility to force a full redraw when its palette changes.
    // In most games a palette change is unusual so we get a performance increase by not checking.

    rg_line_diff_t *out_diff = frame->diff;

    int threshold_remaining = frame->width * frame->height * FULL_UPDATE_THRESHOLD;
    int pixel_size = (frame->flags & RG_PIXEL_PAL) ? 1 : 2;
    int lines_changed = 0;

    uint32_t u32_blocks = (frame->width * pixel_size / 4);
    uint32_t u32_pixels = 4 / pixel_size;

    for (int y = 0, i = 0; y < frame->height; ++y, i += frame->stride)
    {
        out_diff[y].left = 0;
        out_diff[y].width = 0;
        out_diff[y].repeat = 1;

        uint32_t *buffer = frame->buffer + i;
        uint32_t *prevBuffer = prevFrame->buffer + i;

        for (int x = 0; x < u32_blocks; ++x)
        {
            if (buffer[x] != prevBuffer[x])
            {
                for (int xl = u32_blocks - 1; xl >= x; --xl)
                {
                    if (buffer[xl] != prevBuffer[xl]) {
                        out_diff[y].left = x * u32_pixels;
                        out_diff[y].width = ((xl + 1) - x) * u32_pixels;
                        lines_changed++;
                        break;
                    }
                }
                break;
            }
        }

        threshold_remaining -= out_diff[y].width;

        if (threshold_remaining <= 0)
        {
            out_diff[0].left = 0;
            out_diff[0].width = frame->width;
            out_diff[0].repeat = frame->height;
            return frame->height; // Stop scan and do full update
        }
    }

    if (scalingMode && filterMode != RG_DISPLAY_FILTER_OFF)
    {
        for (int y = 0; y < frame->height; ++y)
        {
            if (out_diff[y].width > 0)
            {
                int block_start = y;
                int block_end = y;
                int left = out_diff[y].left;
                int right = left + out_diff[y].width;

                while (block_start > 0 && (out_diff[block_start].width > 0 || !frame_filter_lines[block_start].start))
                    block_start--;

                while (block_end < frame->height - 1 && (out_diff[block_end].width > 0 || !frame_filter_lines[block_end].stop))
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
    for (int y = frame->height - 1; y > 0; --y)
    {
        if (abs(out_diff[y].left - out_diff[y-1].left) > 8)
            continue;

        int right = out_diff[y].left + out_diff[y].width;
        int right_prev = out_diff[y-1].left + out_diff[y-1].width;
        if (abs(right - right_prev) > 8)
            continue;

        if (out_diff[y].left < out_diff[y-1].left)
          out_diff[y-1].left = out_diff[y].left;
        out_diff[y-1].width = (right > right_prev) ?
          right - out_diff[y-1].left : right_prev - out_diff[y-1].left;
        out_diff[y-1].repeat = out_diff[y].repeat + 1;
    }

    return lines_changed;
}

static void
generate_filter_structures(size_t width, size_t height)
{
    memset(frame_filter_lines,   0, sizeof frame_filter_lines);
    memset(screen_line_is_empty, 0, sizeof screen_line_is_empty);

    int x_acc = (x_inc * x_origin) % SCREEN_WIDTH;
    int y_acc = (y_inc * y_origin) % SCREEN_HEIGHT;

    for (int x = 0, screen_x = x_origin; x < width && screen_x < SCREEN_WIDTH; ++screen_x)
    {
        x_acc += x_inc;
        while (x_acc >= SCREEN_WIDTH) {
            ++x;
            x_acc -= SCREEN_WIDTH;
        }
    }

    for (int y = 0, screen_y = y_origin; y < height && screen_y < SCREEN_HEIGHT; ++screen_y)
    {
        int repeat = ++frame_filter_lines[y].repeat;

        frame_filter_lines[y].start = repeat == 1 || repeat == 2;
        frame_filter_lines[y].stop  = repeat == 1;

        screen_line_is_empty[screen_y] = repeat > 1;

        y_acc += y_inc;
        while (y_acc >= SCREEN_HEIGHT) {
            ++y;
            y_acc -= SCREEN_HEIGHT;
        }
    }

    frame_filter_lines[0].start = frame_filter_lines[height-1].stop  = true;
}

IRAM_ATTR static void
display_task(void *arg)
{
    videoTaskQueue = xQueueCreate(1, sizeof(void*));

    rg_video_frame_t *update;

    while(1)
    {
        xQueuePeek(videoTaskQueue, &update, portMAX_DELAY);
        // xQueueReceive(videoTaskQueue, &update, portMAX_DELAY);

        if (!update) break;

        if (forceVideoRefresh)
        {
            if (scalingMode == RG_DISPLAY_SCALING_FILL) {
                rg_display_set_scale(update->width, update->height, SCREEN_WIDTH / (double)SCREEN_HEIGHT);
            }
            else if (scalingMode == RG_DISPLAY_SCALING_FIT) {
                rg_display_set_scale(update->width, update->height, update->width / (double)update->height);
            }
            else {
                rg_display_set_scale(update->width, update->height, 0.0);
            }
            rg_display_clear(0x0000);

            forceVideoRefresh = false;
        }

        RG_ASSERT((update->flags & RG_PIXEL_PAL) == 0 || update->palette, "Palette not defined");

        for (int y = 0; y < update->height;)
        {
            rg_line_diff_t *diff = &update->diff[y];

            if (diff->width > 0)
            {
                write_rect(update, diff->left, y, diff->width, diff->repeat);
            }
            y += diff->repeat;
        }

        if (updateCallback)
            updateCallback(update);

        xQueueReceive(videoTaskQueue, &update, portMAX_DELAY);
    }

    videoTaskQueue = NULL;

    vTaskDelete(NULL);
}

void
rg_display_set_scale(int width, int height, double new_ratio)
{
    int new_width = width;
    int new_height = height;
    double x_scale = 1.0;
    double y_scale = 1.0;

    if (new_ratio > 0.0)
    {
        new_width = SCREEN_HEIGHT * new_ratio;
        new_height = SCREEN_HEIGHT;

        if (new_width > SCREEN_WIDTH)
        {
            RG_LOGX("[LCD SCALE] new_width too large: %d, reducing new_height to maintain ratio.\n", new_width);
            new_height = SCREEN_HEIGHT * (SCREEN_WIDTH / (double)new_width);
            new_width = SCREEN_WIDTH;
        }

        x_scale = new_width / (double)width;
        y_scale = new_height / (double)height;
    }

    x_inc = SCREEN_WIDTH / x_scale;
    y_inc = SCREEN_HEIGHT / y_scale;
    x_origin = (SCREEN_WIDTH - new_width) / 2;
    y_origin = (SCREEN_HEIGHT - new_height) / 2;

    generate_filter_structures(width, height);

    RG_LOGX("[LCD SCALE] %dx%d@%.3f => %dx%d@%.3f x_inc:%d y_inc:%d x_scale:%.3f y_scale:%.3f x_origin:%d y_origin:%d\n",
           width, height, width/(double)height, new_width, new_height, new_ratio,
           x_inc, y_inc, x_scale, y_scale, x_origin, y_origin);
}

display_scaling_t
rg_display_get_scaling_mode(void)
{
    return scalingMode;
}

void
rg_display_set_scaling_mode(display_scaling_t mode)
{
    rg_settings_DisplayScaling_set(mode);
    scalingMode = mode;
    forceVideoRefresh = true;
}

display_filter_t
rg_display_get_filter_mode(void)
{
    return filterMode;
}

void
rg_display_set_filter_mode(display_filter_t mode)
{
    rg_settings_DisplayFilter_set(mode);
    filterMode = mode;
    forceVideoRefresh = true;
}

display_rotation_t
rg_display_get_rotation(void)
{
    return rotationMode;
}

void
rg_display_set_rotation(display_rotation_t rotation)
{
    rg_settings_DisplayRotation_set(rotation);
    rotationMode = rotation;
    forceVideoRefresh = true;
}

display_backlight_t
rg_display_get_backlight()
{
    return backlightLevel;
}

void
rg_display_set_backlight(display_backlight_t level)
{
    rg_settings_Backlight_set(level);
    backlight_set_level(backlightLevels[level % RG_BACKLIGHT_LEVEL_COUNT]);
    backlightLevel = level;
}

void
rg_display_set_callback(update_callback_t func)
{
    updateCallback = func;
}

void
rg_display_force_refresh(void)
{
    forceVideoRefresh = true;
}

bool
rg_display_save_frame(const char *filename, const rg_video_frame_t *frame, int width, int height)
{
    if (width <= 0 && height <= 0)
    {
        width = frame->width;
        height = frame->height;
    }
    else if (width <= 0)
    {
        width = frame->width * ((float)height / frame->height);
    }
    else if (height <= 0)
    {
        height = frame->height * ((float)width / frame->width);
    }

    float step_x = (float)frame->width / width;
    float step_y = (float)frame->height / height;

    RG_LOGI("Saving frame: %dx%d to PNG %dx%d. Step: X=%.3f Y=%.3f\n",
        frame->width, frame->height, width, height, step_x, step_y);

    LuImage *png = luImageCreate(width, height, 3, 8, 0, 0);
    if (!png)
    {
        RG_LOGE("LuImage allocation failed!\n");
        return false;
    }

    uint8_t *img_ptr = png->data;

    for (int y = 0; y < height; y++)
    {
        uint8_t *line = frame->buffer + ((int)(y * step_y) * frame->stride);

        for (int x = 0; x < width; x++)
        {
            uint32_t pixel;

            if (frame->flags & RG_PIXEL_PAL)
                pixel = ((uint16_t*)frame->palette)[line[(int)(x * step_x)] & frame->pixel_mask];
            else
                pixel = ((uint16_t*)line)[(int)(x * step_x)];

            if ((frame->flags & RG_PIXEL_LE) == 0) // BE to LE
                pixel = (pixel << 8) | (pixel >> 8);

            *(img_ptr++) = ((pixel >> 11) & 0x1F) << 3;
            *(img_ptr++) = ((pixel >> 5) & 0x3F) << 2;
            *(img_ptr++) = (pixel & 0x1F) << 3;
        }
    }

    bool status = luPngWriteFile(filename, png);
    luImageRelease(png, 0);

    if (status != PNG_OK)
    {
        RG_LOGE("luPngWriteFile() failed! %d\n", status);
        return false;
    }

    return true;
}

IRAM_ATTR screen_update_t
rg_display_queue_update(rg_video_frame_t *frame, rg_video_frame_t *previousFrame)
{
    static int prev_width = 0, prev_height = 0;
    int linesChanged = 0;

    if (!frame)
        return RG_SCREEN_UPDATE_ERROR;

    if (frame->width != prev_width || frame->height != prev_height)
    {
        prev_width = frame->width;
        prev_height = frame->height;
        forceVideoRefresh = true;
    }

    if (previousFrame && !forceVideoRefresh)
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

    xQueueSend(videoTaskQueue, &frame, portMAX_DELAY);

    if (linesChanged == frame->height)
        return RG_SCREEN_UPDATE_FULL;

    if (linesChanged == 0)
        return RG_SCREEN_UPDATE_EMPTY;

    return RG_SCREEN_UPDATE_PARTIAL;
}

void
rg_display_drain_spi()
{
    if (uxQueueSpacesAvailable(spi_queue)) {
        spi_transaction_t *t[SPI_TRANSACTION_COUNT];
        for (size_t i = 0; i < SPI_TRANSACTION_COUNT; ++i) {
            xQueueReceive(spi_queue, &t[i], portMAX_DELAY);
        }
        for (size_t i = 0; i < SPI_TRANSACTION_COUNT; ++i) {
            xQueueSend(spi_queue, &t[i], portMAX_DELAY);
        }
    }
}

void
rg_display_show_info(const char *text, int timeout_ms)
{
    // Overlay a line of text at the bottom of the screen for approximately timeout_ms
    // It would make more sense in rg_gui, but for efficiency I think here will be best...
}

void
rg_display_write(int left, int top, int width, int height, int stride, const uint16_t* buffer)
{
    rg_display_drain_spi();

    send_reset_drawing(left, top, width, height);

    size_t lines_per_buffer = SPI_TRANSACTION_BUFFER_LENGTH / width;

    if (stride < width * 2) {
        stride = width * 2;
    }

    for (size_t y = 0; y < height; y += lines_per_buffer)
    {
        uint16_t* line_buffer = spi_get_buffer();

        if (y + lines_per_buffer > height)
            lines_per_buffer = height - y;

        for (size_t line = 0; line < lines_per_buffer; ++line)
        {
            // We void* the buffer because stride is a byte length, not uint16
            const uint16_t *buf = (void*)buffer + ((y + line) * stride);
            const size_t offset = line * width;

            for (size_t i = 0; i < width; ++i)
            {
                line_buffer[offset + i] = buf[i] << 8 | buf[i] >> 8;
            }
        }

        send_continue_line(line_buffer, width, lines_per_buffer);
    }

    rg_display_drain_spi();
}

void
rg_display_clear(uint16_t color)
{
    rg_display_drain_spi();

    send_reset_drawing(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    color = (color << 8) | (color >> 8);

    for (size_t i = 0; i < SPI_TRANSACTION_COUNT; ++i)
    {
        for (size_t j = 0; j < SPI_TRANSACTION_BUFFER_LENGTH; ++j)
        {
            spi_buffers[i][j] = color;
        }
    }

    size_t lines_per_buffer = SPI_TRANSACTION_BUFFER_LENGTH / SCREEN_WIDTH;
    for (size_t y = 0; y < SCREEN_HEIGHT; y += lines_per_buffer)
    {
        send_continue_line(spi_get_buffer(), SCREEN_WIDTH, lines_per_buffer);
    }

    rg_display_drain_spi();
}

void
rg_display_deinit()
{
    void *stop = NULL;

    xQueueSend(videoTaskQueue, &stop, portMAX_DELAY);
    vTaskDelay(10);
    // To do: Stop SPI task...

    backlight_deinit();
    ili9341_deinit();
}

void
rg_display_init()
{
    backlightLevel = rg_settings_Backlight_get();
    scalingMode = rg_settings_DisplayScaling_get();
    filterMode = rg_settings_DisplayFilter_get();
    rotationMode = rg_settings_DisplayRotation_get();

    RG_LOGI("Initialization:\n");

    RG_LOGI(" - calling spi_initialize.\n");
    spi_initialize();

	RG_LOGI(" - calling ili9341_init.\n");
    ili9341_init();

	RG_LOGI(" - calling backlight_init.\n");
    backlight_init();

	RG_LOGI(" - starting display_task.\n");
    xTaskCreatePinnedToCore(&display_task, "display_task", 3072, NULL, 5, NULL, 1);

    RG_LOGI("init done.\n");
}
