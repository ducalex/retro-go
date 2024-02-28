#include "rg_system.h"
#include "rg_display.h"

#include <stdlib.h>
#include <string.h>

#ifdef ESP_PLATFORM
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#ifdef RG_GPIO_LCD_BCKL
#include <driver/ledc.h>
#endif
#else
#include <SDL2/SDL.h>
#endif

#define LCD_BUFFER_LENGTH (RG_SCREEN_WIDTH * 4) // In pixels

static QueueHandle_t display_task_queue;
static rg_display_counters_t counters;
static rg_display_config_t config;
static rg_surface_t *osd;
static rg_surface_t *border;
static rg_display_t display;
static uint8_t screen_line_is_empty[RG_SCREEN_HEIGHT + 1];
static uint32_t screen_line_checksum[RG_SCREEN_HEIGHT + 1];

static const char *SETTING_BACKLIGHT = "DispBacklight";
static const char *SETTING_SCALING = "DispScaling";
static const char *SETTING_FILTER = "DispFilter";
static const char *SETTING_ROTATION = "DispRotation";
static const char *SETTING_BORDER = "DispBorder";
static const char *SETTING_CUSTOM_WIDTH = "DispCustomWidth";
static const char *SETTING_CUSTOM_HEIGHT = "DispCustomHeight";

#ifdef ESP_PLATFORM
static spi_device_handle_t spi_dev;
static QueueHandle_t spi_transactions;
static QueueHandle_t spi_buffers;

#define SPI_TRANSACTION_COUNT (10)
#define SPI_BUFFER_COUNT      (5)
#define SPI_BUFFER_LENGTH     (LCD_BUFFER_LENGTH * 2)

static inline void spi_queue_transaction(const void *data, size_t length, uint32_t type)
{
    spi_transaction_t *t;

    if (!data || length < 1)
        return;

    xQueueReceive(spi_transactions, &t, portMAX_DELAY);

    *t = (spi_transaction_t){
        .tx_buffer = NULL,
        .length = length * 8, // In bits
        .user = (void *)type,
        .flags = 0,
    };

    if (type & 2)
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
        void *tx_buffer;
        if (xQueueReceive(spi_buffers, &tx_buffer, pdMS_TO_TICKS(2500)) != pdTRUE)
            RG_PANIC("display");
        t->tx_buffer = memcpy(tx_buffer, data, length);
        t->user = (void *)(type | 2);
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

    while (spi_device_get_trans_result(spi_dev, &t, portMAX_DELAY) == ESP_OK)
    {
        if ((int)t->user & 2)
            xQueueSend(spi_buffers, &t->tx_buffer, portMAX_DELAY);
        xQueueSend(spi_transactions, &t, portMAX_DELAY);
    }
}

static void spi_init(void)
{
    spi_transactions = xQueueCreate(SPI_TRANSACTION_COUNT, sizeof(spi_transaction_t *));
    spi_buffers = xQueueCreate(SPI_BUFFER_COUNT, sizeof(uint16_t *));

    while (uxQueueSpacesAvailable(spi_transactions))
    {
        void *trans = malloc(sizeof(spi_transaction_t));
        xQueueSend(spi_transactions, &trans, portMAX_DELAY);
    }

    while (uxQueueSpacesAvailable(spi_buffers))
    {
        void *buffer = rg_alloc(SPI_BUFFER_LENGTH, MEM_DMA);
        xQueueSend(spi_buffers, &buffer, portMAX_DELAY);
    }

    const spi_bus_config_t buscfg = {
        .miso_io_num = RG_GPIO_LCD_MISO,
        .mosi_io_num = RG_GPIO_LCD_MOSI,
        .sclk_io_num = RG_GPIO_LCD_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    const spi_device_interface_config_t devcfg = {
        .clock_speed_hz = RG_SCREEN_SPEED,   // Typically SPI_MASTER_FREQ_40M or SPI_MASTER_FREQ_80M
        .mode = 0,                           // SPI mode 0
        .spics_io_num = RG_GPIO_LCD_CS,      // CS pin
        .queue_size = SPI_TRANSACTION_COUNT, // We want to be able to queue 5 transactions at a time
        .pre_cb = &spi_pre_transfer_cb,      // Specify pre-transfer callback to handle D/C line and SPI lock
        .flags = SPI_DEVICE_NO_DUMMY,        // SPI_DEVICE_HALFDUPLEX;
    };

    esp_err_t ret;

    // Initialize the SPI bus
    ret = spi_bus_initialize(RG_SCREEN_HOST, &buscfg, SPI_DMA_CH_AUTO);
    RG_ASSERT(ret == ESP_OK || ret == ESP_ERR_INVALID_STATE, "spi_bus_initialize failed.");

    ret = spi_bus_add_device(RG_SCREEN_HOST, &devcfg, &spi_dev);
    RG_ASSERT(ret == ESP_OK, "spi_bus_add_device failed.");

    rg_task_create("rg_spi", &spi_task, NULL, 1.5 * 1024, RG_TASK_PRIORITY_7, 1);
}

static void spi_deinit(void)
{
    spi_bus_remove_device(spi_dev);
    spi_bus_free(RG_SCREEN_HOST);
}
#endif

#if RG_SCREEN_DRIVER == 0 /* ILI9341 */
#define ILI9341_CMD(cmd, data...)                    \
    {                                                \
        const uint8_t c = cmd, x[] = {data};         \
        spi_queue_transaction(&c, 1, 0);             \
        if (sizeof(x))                               \
            spi_queue_transaction(&x, sizeof(x), 1); \
    }

static void lcd_set_backlight(double percent)
{
    double level = RG_MIN(RG_MAX(percent / 100.0, 0), 1.0);
    int error_code = 0;

#if defined(RG_GPIO_LCD_BCKL)
    error_code = ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0x1FFF * level, 50, 0);
#elif defined(RG_TARGET_QTPY_GAMER)
    // error_code = aw_analogWrite(AW_TFT_BACKLIGHT, level * 255);
#endif

    if (error_code)
        RG_LOGE("failed setting backlight to %d%% (0x%02X)\n", (int)(100 * level), error_code);
    else
        RG_LOGI("backlight set to %d%%\n", (int)(100 * level));
}

static void lcd_set_window(int left, int top, int width, int height)
{
    int right = left + width - 1;
    int bottom = top + height - 1;

    if (left < 0 || top < 0 || right >= RG_SCREEN_WIDTH || bottom >= RG_SCREEN_HEIGHT)
        RG_LOGW("Bad lcd window (x0=%d, y0=%d, x1=%d, y1=%d)\n", left, top, right, bottom);

    ILI9341_CMD(0x2A, left >> 8, left & 0xff, right >> 8, right & 0xff); // Horiz
    ILI9341_CMD(0x2B, top >> 8, top & 0xff, bottom >> 8, bottom & 0xff); // Vert
    ILI9341_CMD(0x2C);                                                   // Memory write
}

static inline void lcd_send_data(const uint16_t *buffer, size_t length)
{
    spi_queue_transaction(buffer, length * sizeof(*buffer), 3);
}

static inline uint16_t *lcd_get_buffer(void)
{
    uint16_t *buffer;
    if (xQueueReceive(spi_buffers, &buffer, pdMS_TO_TICKS(2500)) != pdTRUE)
        RG_PANIC("display");
    return buffer;
}

static void lcd_sync(void)
{
    // Unused for SPI LCD
}

static void lcd_init(void)
{
#ifdef RG_GPIO_LCD_BCKL
    // Initialize backlight at 0% to avoid the lcd reset flash
    ledc_timer_config(&(ledc_timer_config_t){
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = 5000,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
    });
    ledc_channel_config(&(ledc_channel_config_t){
        .channel = LEDC_CHANNEL_0,
        .duty = 0,
        .gpio_num = RG_GPIO_LCD_BCKL,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0,
    });
    ledc_fade_func_install(0);
#endif

    spi_init();

    // Setup Data/Command line
    gpio_set_direction(RG_GPIO_LCD_DC, GPIO_MODE_OUTPUT);
    gpio_set_level(RG_GPIO_LCD_DC, 1);

#if defined(RG_GPIO_LCD_RST)
    gpio_set_direction(RG_GPIO_LCD_RST, GPIO_MODE_OUTPUT);
    gpio_set_level(RG_GPIO_LCD_RST, 0);
    rg_usleep(100 * 1000);
    gpio_set_level(RG_GPIO_LCD_RST, 1);
    rg_usleep(10 * 1000);
#endif

    ILI9341_CMD(0x01);          // Reset
    rg_usleep(5 * 1000);           // Wait 5ms after reset
    ILI9341_CMD(0x3A, 0X05);    // Pixel Format Set RGB565
    #ifdef RG_SCREEN_INIT
        RG_SCREEN_INIT();
    #else
        #warning "LCD init sequence is not defined for this device!"
    #endif
    ILI9341_CMD(0x11);  // Exit Sleep
    rg_usleep(5 * 1000);// Wait 5ms after sleep out
    ILI9341_CMD(0x29);  // Display on

    rg_display_clear(C_BLACK);
    rg_usleep(10 * 1000);
    lcd_set_backlight(config.backlight);
}

static void lcd_deinit(void)
{
#ifdef RG_SCREEN_DEINIT
    RG_SCREEN_DEINIT();
#endif
    spi_deinit();
    // gpio_reset_pin(RG_GPIO_LCD_BCKL);
    // gpio_reset_pin(RG_GPIO_LCD_DC);
}
#else
#define lcd_init()
#define lcd_deinit()
#define lcd_get_buffer() (void *)0
#define lcd_set_backlight(l)
#define lcd_send_data(a, b)
#define lcd_set_window(a, b, c, d)
#endif

static inline unsigned blend_pixels(unsigned a, unsigned b)
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

    unsigned v = (rv << 11) | (gv << 5) | (bv);

    // Back to Big-Endian
    return (v << 8) | (v >> 8);
}

static inline void write_update(const rg_surface_t *update)
{
    const int64_t time_start = rg_system_timer();

    const int screen_width = display.screen.width;
    const int screen_height = display.screen.height;

    const int width = display.source.width;
    const int height = display.source.height;
    const int format = update->format & RG_PIXEL_FORMAT;
    const int stride = update->stride;

    const int x_inc = display.viewport.x_inc;
    const int y_inc = display.viewport.y_inc;
    const int draw_left = display.viewport.left;
    const int draw_top = display.viewport.top;
    const int draw_width = ((screen_width * width) + (x_inc - 1)) / x_inc;
    const int draw_height = ((screen_height * height) + (y_inc - 1)) / y_inc;

    const bool filter_y = (config.filter == RG_DISPLAY_FILTER_VERT || config.filter == RG_DISPLAY_FILTER_BOTH) &&
                          (config.scaling && (display.viewport.height % display.source.height) != 0);
    const bool filter_x = (config.filter == RG_DISPLAY_FILTER_HORIZ || config.filter == RG_DISPLAY_FILTER_BOTH) &&
                          (config.scaling && (display.viewport.width % display.source.width) != 0);

    const size_t data_offset = (display.source.crop_v * stride) + (display.source.crop_h * RG_PIXEL_GET_SIZE(format));
    union {const uint8_t *u8; const uint16_t *u16; } buffer = {update->data + data_offset};
    const uint16_t *palette = update->palette;

    bool partial = true; // config.update_mode == RG_DISPLAY_UPDATE_PARTIAL;

    int lines_per_buffer = LCD_BUFFER_LENGTH / draw_width;
    int lines_remaining = draw_height;
    int lines_updated = 0;
    int window_top = -1;

    for (int y = 0, screen_y = draw_top; y < height;)
    {
        int lines_to_copy = RG_MIN(lines_per_buffer, lines_remaining);

        // The vertical filter requires a block to start and end with unscaled lines
        if (filter_y)
        {
            while (lines_to_copy > 1 && (screen_line_is_empty[screen_y + lines_to_copy - 1] ||
                                         screen_line_is_empty[screen_y + lines_to_copy]))
                --lines_to_copy;
        }

        if (lines_to_copy < 1)
        {
            break;
        }

        uint16_t *line_buffer = lcd_get_buffer();
        uint16_t *line_buffer_ptr = line_buffer;

        uint32_t checksum = 0xFFFFFFFF;
        bool need_update = !partial;

        for (int i = 0; i < lines_to_copy; ++i)
        {
            if (i > 0 && screen_line_is_empty[screen_y])
            {
                memcpy(line_buffer_ptr, line_buffer_ptr - draw_width, draw_width * 2);
                line_buffer_ptr += draw_width;
            }
            else
            {
                #define RENDER_LINE(pixel) { \
                    for (size_t x = 0, x_acc = 0; x < width; ++x) { \
                        uint16_t _pixel = (pixel);\
                        while (x_acc < screen_width) { \
                            *line_buffer_ptr++ = _pixel; \
                            x_acc += x_inc; \
                        } \
                        x_acc -= screen_width; \
                    } \
                }
                if (format & RG_PIXEL_PALETTE)
                    RENDER_LINE(palette[buffer.u8[x]])
                else if (format == RG_PIXEL_565_LE)
                    RENDER_LINE((buffer.u16[x] << 8) | (buffer.u16[x] >> 8))
                else
                    RENDER_LINE(buffer.u16[x])

                if (partial)
                {
                    checksum = rg_hash((void*)(line_buffer_ptr - draw_width), draw_width * 2);
                }
            }

            if (screen_line_checksum[screen_y] != checksum)
            {
                screen_line_checksum[screen_y] = checksum;
                need_update = true;
            }

            if (!screen_line_is_empty[++screen_y])
            {
                buffer.u8 += stride;
                ++y;
            }
        }

        if (need_update)
        {
            if (filter_y || filter_x)
            {
                const int top = screen_y - lines_to_copy;

                for (int y = 0, fill_line = -1; y < lines_to_copy; y++)
                {
                    if (filter_y && y && screen_line_is_empty[top + y])
                    {
                        fill_line = y;
                        continue;
                    }

                    // Filter X
                    if (filter_x)
                    {
                        uint16_t *buffer = line_buffer + y * draw_width;
                        for (int x = 0, frame_x = 0, prev_frame_x = -1, x_acc = 0; x < draw_width; ++x)
                        {
                            if (frame_x == prev_frame_x && x > 0 && x + 1 < draw_width)
                            {
                                buffer[x] = blend_pixels(buffer[x - 1], buffer[x + 1]);
                            }
                            prev_frame_x = frame_x;

                            x_acc += x_inc;
                            while (x_acc >= screen_width)
                            {
                                x_acc -= screen_width;
                                ++frame_x;
                            }
                        }
                    }

                    // Filter Y
                    if (filter_y && fill_line > 0)
                    {
                        uint16_t *lineA = line_buffer + (fill_line - 1) * draw_width;
                        uint16_t *lineB = line_buffer + (fill_line + 0) * draw_width;
                        uint16_t *lineC = line_buffer + (fill_line + 1) * draw_width;
                        for (size_t x = 0; x < draw_width; ++x)
                        {
                            lineB[x] = blend_pixels(lineA[x], lineC[x]);
                        }
                        fill_line = -1;
                    }
                }
            }

            int left = RG_SCREEN_MARGIN_LEFT + draw_left;
            int top = RG_SCREEN_MARGIN_TOP + screen_y - lines_to_copy;
            if (top != window_top)
                lcd_set_window(left, top, draw_width, lines_remaining);
            lcd_send_data(line_buffer, draw_width * lines_to_copy);
            window_top = top + lines_to_copy;
            lines_updated += lines_to_copy;
        }
        else
        {
            // Return buffer
            xQueueSend(spi_buffers, &line_buffer, portMAX_DELAY);
        }

        lines_remaining -= lines_to_copy;
    }

    if (lines_updated > display.screen.height * 0.80)
        counters.fullFrames++;
    else
        counters.partFrames++;
    counters.busyTime += rg_system_timer() - time_start;
}

static void update_viewport_scaling(void)
{
    int src_width = display.source.width;
    int src_height = display.source.height;
    int new_width = src_width;
    int new_height = src_height;
    double new_ratio = 0.0;

    if (config.scaling == RG_DISPLAY_SCALING_FULL)
    {
        new_ratio = display.screen.width / (double)display.screen.height;
        new_width = display.screen.height * new_ratio;
        new_height = display.screen.height;
    }
    else if (config.scaling == RG_DISPLAY_SCALING_FIT)
    {
        new_ratio = src_width / (double)src_height;
        new_width = display.screen.height * new_ratio;
        new_height = display.screen.height;
    }
    else if (config.scaling == RG_DISPLAY_SCALING_CUSTOM)
    {
        new_ratio = config.custom_width / (double)config.custom_height;
        new_width = config.custom_width;
        new_height = config.custom_height;
    }

    if (new_width > display.screen.width)
    {
        RG_LOGW("new_width too large: %d, cropping to %d", new_width, display.screen.width);
        new_width = display.screen.width;
    }

    if (new_height > display.screen.height)
    {
        RG_LOGW("new_height too large: %d, cropping to %d", new_height, display.screen.height);
        new_height = display.screen.height;
    }

    display.viewport.left = (display.screen.width - new_width) / 2;
    display.viewport.top = (display.screen.height - new_height) / 2;
    display.viewport.x_inc = display.screen.width / (new_width / (double)src_width);
    display.viewport.y_inc = display.screen.height / (new_height / (double)src_height);
    display.viewport.width = new_width;
    display.viewport.height = new_height;

    memset(screen_line_checksum, 0, sizeof(screen_line_checksum));
    memset(screen_line_is_empty, 0, sizeof(screen_line_is_empty));

    int y_acc = (display.viewport.y_inc * display.viewport.top) % display.screen.height;
    int prev = -1;

    // Build boundary tables used by filtering
    for (int y = 0, screen_y = display.viewport.top; y < src_height && screen_y < display.screen.height; ++screen_y)
    {
        screen_line_is_empty[screen_y] |= (prev == y);
        prev = y;

        y_acc += display.viewport.y_inc;
        while (y_acc >= display.screen.height)
        {
            y_acc -= display.screen.height;
            ++y;
        }
    }

    RG_LOGI("%dx%d@%.3f => %dx%d@%.3f x_pos:%d y_pos:%d x_inc:%d y_inc:%d\n", src_width, src_height,
            src_width / (double)src_height, new_width, new_height, new_ratio, display.viewport.left,
            display.viewport.top, display.viewport.x_inc, display.viewport.y_inc);
}

static bool load_border_file(const char *filename)
{
    RG_LOGI("Loading border file: %s", filename ?: "(none)");

    free(border), border = NULL;
    display.changed = true;

    if (filename && (border = rg_surface_load_image_file(filename, 0)))
    {
        if (border->width != display.screen.width || border->height != display.screen.height)
        {
            rg_surface_t *resized = rg_surface_resize(border, display.screen.width, display.screen.height);
            if (resized)
            {
                rg_surface_free(border);
                border = resized;
            }
        }
        return true;
    }
    return false;
}

IRAM_ATTR
static void display_task(void *arg)
{
    display_task_queue = xQueueCreate(1, sizeof(rg_surface_t *));

    while (1)
    {
        const rg_surface_t *update;

        xQueuePeek(display_task_queue, &update, portMAX_DELAY);
        // xQueueReceive(display_task_queue, &update, portMAX_DELAY);

        // Received a shutdown request!
        if (update == (void *)-1)
            break;

        if (display.changed)
        {
            if (config.scaling != RG_DISPLAY_SCALING_FULL)
            {
                if (border)
                    rg_display_write(0, 0, border->width, border->height, 0, border->data, border->format|RG_PIXEL_NOSYNC);
                else
                    rg_display_clear(C_BLACK);
            }
            update_viewport_scaling();
            display.changed = false;
        }

        write_update(update);

        xQueueReceive(display_task_queue, &update, portMAX_DELAY);

        // We update OSD *after* receiving the update, because the update would block rg_display_write
        if (osd != NULL)
        {
            // rg_display_write(-osd.width, 0, osd.width, osd.height, osd.width * 2, osd.buffer, RG_PIXEL_565_LE);
        }

        lcd_sync();
    }

    vQueueDelete(display_task_queue);
    display_task_queue = NULL;
}

void rg_display_force_redraw(void)
{
    display.changed = true;
    // memset(screen_line_checksum, 0, sizeof(screen_line_checksum));
    rg_system_event(RG_EVENT_REDRAW, NULL);
    rg_display_sync(true);
}

const rg_display_t *rg_display_get_info(void)
{
    return &display;
}

rg_display_counters_t rg_display_get_counters(void)
{
    return counters;
}

void rg_display_set_scaling(display_scaling_t scaling)
{
    config.scaling = RG_MIN(RG_MAX(0, scaling), RG_DISPLAY_SCALING_COUNT - 1);
    rg_settings_set_number(NS_APP, SETTING_SCALING, config.scaling);
    display.changed = true;
}

display_scaling_t rg_display_get_scaling(void)
{
    return config.scaling;
}

void rg_display_set_custom_width(int width)
{
    config.custom_width = RG_MIN(RG_MAX(64, width), display.screen.width);
    rg_settings_set_number(NS_APP, SETTING_CUSTOM_WIDTH, config.custom_width);
    display.changed = true;
}

int rg_display_get_custom_width(void)
{
    return config.custom_width;
}

void rg_display_set_custom_height(int height)
{
    config.custom_height = RG_MIN(RG_MAX(64, height), display.screen.height);
    rg_settings_set_number(NS_APP, SETTING_CUSTOM_HEIGHT, config.custom_height);
    display.changed = true;
}

int rg_display_get_custom_height(void)
{
    return config.custom_height;
}

void rg_display_set_filter(display_filter_t filter)
{
    config.filter = RG_MIN(RG_MAX(0, filter), RG_DISPLAY_FILTER_COUNT - 1);
    rg_settings_set_number(NS_APP, SETTING_FILTER, config.filter);
    display.changed = true;
}

display_filter_t rg_display_get_filter(void)
{
    return config.filter;
}

void rg_display_set_rotation(display_rotation_t rotation)
{
    config.rotation = RG_MIN(RG_MAX(0, rotation), RG_DISPLAY_ROTATION_COUNT - 1);
    rg_settings_set_number(NS_APP, SETTING_SCALING, config.rotation);
    display.changed = true;
}

display_rotation_t rg_display_get_rotation(void)
{
    return config.rotation;
}

void rg_display_set_backlight(display_backlight_t percent)
{
    config.backlight = RG_MIN(RG_MAX(percent, RG_DISPLAY_BACKLIGHT_MIN), RG_DISPLAY_BACKLIGHT_MAX);
    rg_settings_set_number(NS_GLOBAL, SETTING_BACKLIGHT, config.backlight);
    lcd_set_backlight(config.backlight);
}

display_backlight_t rg_display_get_backlight(void)
{
    return config.backlight;
}

void rg_display_set_border(const char *filename)
{
    free(config.border_file);
    config.border_file = NULL;

    if (load_border_file(filename))
    {
        rg_settings_set_string(NS_APP, SETTING_BORDER, filename);
        config.border_file = strdup(filename);
    }
    else
    {
        rg_settings_set_string(NS_APP, SETTING_BORDER, NULL);
        config.border_file = NULL;
    }
    display.changed = true;
}

char *rg_display_get_border(void)
{
    return rg_settings_get_string(NS_APP, SETTING_BORDER, NULL);
}

bool rg_display_save_frame(const char *filename, const rg_surface_t *frame, int width, int height)
{
    // Ugly hack because the frame we receive doesn't have crop info yet
    rg_surface_t temp = *frame;
    temp.width = display.source.width;
    temp.height = display.source.height;
    temp.data += (display.source.crop_v * temp.stride) + (display.source.crop_h * RG_PIXEL_GET_SIZE(frame->format));
    return rg_surface_save_image_file(&temp, filename, width, height);
}

void rg_display_submit(const rg_surface_t *update, uint32_t flags)
{
    const int64_t time_start = rg_system_timer();

    // Those things should probably be asserted, but this is a new system let's be forgiving...
    if (!update || !update->buffer)
        return;

    if (!display.source.defined)
        rg_display_set_source_viewport(update->width, update->height, 0, 0);

    xQueueSend(display_task_queue, &update, portMAX_DELAY);

    counters.blockTime += rg_system_timer() - time_start;
    counters.totalFrames++;
}

void rg_display_set_source_viewport(int width, int height, int crop_h, int crop_v)
{
    rg_display_sync(true);
    display.source.crop_h = RG_MAX(RG_MAX(0, width - display.screen.width) / 2, crop_h);
    display.source.crop_v = RG_MAX(RG_MAX(0, height - display.screen.height) / 2, crop_v);
    display.source.width = width - display.source.crop_h * 2;
    display.source.height = height - display.source.crop_v * 2;
    display.source.defined = true;
    display.changed = true;
}

bool rg_display_sync(bool block)
{
#ifdef ESP_PLATFORM
    while (block && uxQueueMessagesWaiting(display_task_queue))
        continue; // Wait until display queue is done
    return uxQueueMessagesWaiting(display_task_queue) == 0;
#else
    return true;
#endif
}

void rg_display_write(int left, int top, int width, int height, int stride, const uint16_t *buffer,
                      rg_pixel_flags_t flags)
{
    RG_ASSERT(buffer, "Bad param");

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

    // This can happen when left or top is out of bound
    if (width < 0 || height < 0)
        return;

    // This will work for now because we rarely draw from different threads (so all we need is ensure
    // that we're not interrupting a display update). But what we SHOULD be doing is acquire a lock
    // before every call to lcd_set_window and release it only after the last call to lcd_send_data.
    if (!(flags & RG_PIXEL_NOSYNC))
        rg_display_sync(true);

    // This isn't really necessary but it makes sense to invalidate
    // the lines we're about to overwrite...
    for (size_t y = 0; y < height; ++y)
        screen_line_checksum[top + y] = 0;

    lcd_set_window(left + RG_SCREEN_MARGIN_LEFT, top + RG_SCREEN_MARGIN_TOP, width, height);

    for (size_t y = 0; y < height;)
    {
        uint16_t *lcd_buffer = lcd_get_buffer();
        size_t num_lines = RG_MIN(LCD_BUFFER_LENGTH / width, height - y);

        // Copy line by line because stride may not match width
        for (size_t line = 0; line < num_lines; ++line)
        {
            uint16_t *src = (void *)buffer + ((y + line) * stride);
            uint16_t *dst = lcd_buffer + (line * width);
            if ((flags & RG_PIXEL_FORMAT) == RG_PIXEL_565_BE)
            {
                memcpy(dst, src, width * 2);
            }
            else
            {
                for (size_t i = 0; i < width; ++i)
                    dst[i] = (src[i] >> 8) | (src[i] << 8);
            }
        }

        lcd_send_data(lcd_buffer, width * num_lines);
        y += num_lines;
    }

    lcd_sync();
}

void rg_display_clear(uint16_t color_le)
{
    lcd_set_window(0, 0, RG_SCREEN_WIDTH, RG_SCREEN_HEIGHT); // We ignore margins here

    uint16_t color_be = (color_le << 8) | (color_le >> 8);
    for (size_t y = 0; y < RG_SCREEN_HEIGHT;)
    {
        uint16_t *buffer = lcd_get_buffer();
        size_t num_lines = RG_MIN(LCD_BUFFER_LENGTH / RG_SCREEN_WIDTH, RG_SCREEN_HEIGHT - y);
        size_t pixels = RG_SCREEN_WIDTH * num_lines;
        for (size_t j = 0; j < pixels; ++j)
            buffer[j] = color_be;
        lcd_send_data(buffer, pixels);
        y += num_lines;
    }
}

void rg_display_deinit(void)
{
    void *stop = (void *)-1;
    xQueueSend(display_task_queue, &stop, portMAX_DELAY);
    // display_task_queue has len == 1. When xQueueSend returns, we know that the only
    // thing in it is our quit request which won't touch the LCD or SPI anymore
    // while (display_task_queue)
    //     rg_task_yield();
    lcd_deinit();
    RG_LOGI("Display terminated.\n");
}

void rg_display_init(void)
{
    RG_LOGI("Initialization...\n");
    // TO DO: We probably should call the setters to ensure valid values...
    config = (rg_display_config_t){
        .backlight = rg_settings_get_number(NS_GLOBAL, SETTING_BACKLIGHT, 80),
        .scaling = rg_settings_get_number(NS_APP, SETTING_SCALING, RG_DISPLAY_SCALING_FULL),
        .filter = rg_settings_get_number(NS_APP, SETTING_FILTER, RG_DISPLAY_FILTER_BOTH),
        .rotation = rg_settings_get_number(NS_APP, SETTING_ROTATION, RG_DISPLAY_ROTATION_AUTO),
        .border_file = rg_settings_get_string(NS_APP, SETTING_BORDER, NULL),
        .custom_width = rg_settings_get_number(NS_APP, SETTING_CUSTOM_WIDTH, 240),
        .custom_height = rg_settings_get_number(NS_APP, SETTING_CUSTOM_HEIGHT, 240),
    };
    display = (rg_display_t){
        .screen.width = RG_SCREEN_WIDTH - RG_SCREEN_MARGIN_LEFT - RG_SCREEN_MARGIN_RIGHT,
        .screen.height = RG_SCREEN_HEIGHT - RG_SCREEN_MARGIN_TOP - RG_SCREEN_MARGIN_BOTTOM,
        .changed = true,
    };
    lcd_init();
    rg_task_create("rg_display", &display_task, NULL, 3 * 1024, RG_TASK_PRIORITY_6, 1);
    if (config.border_file)
        load_border_file(config.border_file);
    RG_LOGI("Display ready.\n");
}
