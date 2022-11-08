#include "rg_system.h"
#include "rg_display.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef RG_TARGET_SDL2
#include <SDL2/SDL.h>
#else
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#endif

#ifdef RG_GPIO_LCD_BCKL
#include <driver/ledc.h>
#endif

#define SPI_TRANSACTION_COUNT (10)
#define SPI_BUFFER_COUNT      (6)
#define SPI_BUFFER_LENGTH     (4 * 320) // In pixels (uint16)

static spi_device_handle_t spi_dev;
static QueueHandle_t spi_transactions;
static QueueHandle_t spi_buffers;
static QueueHandle_t display_task_queue;

static rg_display_counters_t counters;
static rg_display_config_t config;
static rg_display_t display;

// static rg_video_update_t updates[2];

static struct
{
    uint8_t start  : 1; // Indicates this line or column is safe to start an update on
    uint8_t stop   : 1; // Indicates this line or column is safe to end an update on
    uint8_t repeat : 6; // How many times the line or column is repeated by the scaler or filter
} filter_lines[320];
static uint8_t screen_line_is_empty[RG_SCREEN_HEIGHT];

static const char *SETTING_BACKLIGHT = "DispBacklight";
static const char *SETTING_SCALING = "DispScaling";
static const char *SETTING_FILTER = "DispFilter";
static const char *SETTING_ROTATION = "DispRotation";
static const char *SETTING_UPDATE = "DispUpdate";

#define lcd_send_data(buffer, length) spi_queue_transaction(buffer, length, 3)
#define lcd_vsync()


static inline uint16_t *spi_get_buffer(void)
{
    uint16_t *buffer;
    if (xQueueReceive(spi_buffers, &buffer, pdMS_TO_TICKS(2500)) != pdTRUE)
        RG_PANIC("display");
    return buffer;
}

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

    while (spi_device_get_trans_result(spi_dev, &t, portMAX_DELAY) == ESP_OK)
    {
        if ((int)t->user & 2)
            xQueueSend(spi_buffers, &t->tx_buffer, 0);
        xQueueSend(spi_transactions, &t, 0);
    }

    // This should never happen
    rg_task_delete(NULL);
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
        void *buffer = rg_alloc(SPI_BUFFER_LENGTH * 2, MEM_DMA);
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

    rg_task_create("rg_spi", &spi_task, NULL, 1.5 * 1024, RG_TASK_PRIORITY - 1, 1);
}

static void spi_deinit(void)
{
    spi_bus_remove_device(spi_dev);
    spi_bus_free(RG_SCREEN_HOST);
}

static void ili9341_cmd(uint8_t cmd, const void *data, size_t data_len)
{
    spi_queue_transaction(&cmd, 1, 0);
    if (data && data_len > 0)
        spi_queue_transaction(data, data_len, 1);
    // if ((cmd & 0xE0) == 0x00)
    //     usleep(5000);
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
        RG_LOGE("failed setting backlight to %.2f%% (0x%02X)\n", 100 * level, error_code);
    else
        RG_LOGI("backlight set to %.2f%%\n", 100 * level);
}

static void lcd_init(void)
{
#if defined(RG_GPIO_LCD_BCKL)
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
    usleep(100 * 1000);
    gpio_set_level(RG_GPIO_LCD_RST, 1);
    usleep(10 * 1000);
#endif

#define ILI9341_CMD(cmd, data...) {const uint8_t x[] = data; ili9341_cmd(cmd, x, sizeof(x));}
#if RG_SCREEN_TYPE == 0 // LCD Model (ODROID-GO)
    ILI9341_CMD(0x01, {});     // Reset
    ILI9341_CMD(0x3A, {0x55}); // Pixel Format Set RGB565
    ILI9341_CMD(0xCF, {0x00, 0xc3, 0x30});
    ILI9341_CMD(0xED, {0x64, 0x03, 0x12, 0x81});
    ILI9341_CMD(0xE8, {0x85, 0x00, 0x78});
    ILI9341_CMD(0xCB, {0x39, 0x2c, 0x00, 0x34, 0x02});
    ILI9341_CMD(0xF7, {0x20});
    ILI9341_CMD(0xEA, {0x00, 0x00});
    ILI9341_CMD(0xC0, {0x1B});                                  // Power control   //VRH[5:0]
    ILI9341_CMD(0xC1, {0x12});                                  // Power control   //SAP[2:0];BT[3:0]
    ILI9341_CMD(0xC5, {0x32, 0x3C});                            // VCM control
    ILI9341_CMD(0xC7, {0x91});                                  // VCM control2
    ILI9341_CMD(0x36, {(0x20|0x80|0x08)});                      // Memory Access Control
    ILI9341_CMD(0xB1, {0x00, 0x10});                            // Frame Rate Control (1B=70, 1F=61, 10=119)
    ILI9341_CMD(0xB6, {0x0A, 0xA2});                            // Display Function Control
    ILI9341_CMD(0xF6, {0x01, 0x30});
    ILI9341_CMD(0xF2, {0x00});                                  // 3Gamma Function Disable
    ILI9341_CMD(0x26, {0x01});                                  // Gamma curve selected
    ILI9341_CMD(0xE0, {0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00}); // Set Gamma
    ILI9341_CMD(0xE1, {0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F}); // Set Gamma
    ILI9341_CMD(0x11, {}); // Exit Sleep
    ILI9341_CMD(0x29, {}); // Display on
#elif RG_SCREEN_TYPE == 1 // LCD Model (MRGC-G32)
    ILI9341_CMD(0x01, {});     // Reset
    ILI9341_CMD(0x3A, {0x55}); // Pixel Format Set RGB565
    ILI9341_CMD(0x36, {(0x00|0x00|0x00)});
    ILI9341_CMD(0xB1, {0x00, 0x10});                            // Frame Rate Control (1B=70, 1F=61, 10=119)
    ILI9341_CMD(0xB2, {0x0c, 0x0c, 0x00, 0x33, 0x33});
    ILI9341_CMD(0xB7, {0x35});
    ILI9341_CMD(0xBB, {0x24});
    ILI9341_CMD(0xC0, {0x2C});
    ILI9341_CMD(0xC2, {0x01, 0xFF});
    ILI9341_CMD(0xC3, {0x11});
    ILI9341_CMD(0xC4, {0x20});
    ILI9341_CMD(0xC6, {0x0f});
    ILI9341_CMD(0xD0, {0xA4, 0xA1});
    ILI9341_CMD(0xE0, {0xD0, 0x00, 0x03, 0x09, 0x13, 0x1C, 0x3A, 0x55, 0x48, 0x18, 0x12, 0x0E, 0x19, 0x1E});
    ILI9341_CMD(0xE1, {0xD0, 0x00, 0x03, 0x09, 0x05, 0x25, 0x3A, 0x55, 0x50, 0x3D, 0x1C, 0x1D, 0x1D, 0x1E});
    ILI9341_CMD(0x11, {}); // Exit Sleep
    ILI9341_CMD(0x29, {}); // Display on
#elif RG_SCREEN_TYPE == 2 // LCD Model (QT-PY Gamer)
    ILI9341_CMD(0x01, {});     // Reset
    ILI9341_CMD(0x11, {}); // Exit Sleep
    ILI9341_CMD(0x3A, {0x55}); // Pixel Format Set RGB565
    ILI9341_CMD(0x36, {0xC0});
    ILI9341_CMD(0x2A, {0, 0, 0, 240}); // CASET
    ILI9341_CMD(0x2B, {0, 0, 320>>8, 320&0xFF}); // RASET
    ILI9341_CMD(0x21, {});
    ILI9341_CMD(0x29, {}); // Display on
#elif RG_SCREEN_TYPE == 32 // LCD Model (Retro-ESP32)
    ILI9341_CMD(0x01, {});     // Reset
    ILI9341_CMD(0x3A, {0x55}); // Pixel Format Set RGB565
    ILI9341_CMD(0xCF, {0x00, 0xc3, 0x30});
    ILI9341_CMD(0xED, {0x64, 0x03, 0x12, 0x81});
    ILI9341_CMD(0xE8, {0x85, 0x00, 0x78});
    ILI9341_CMD(0xCB, {0x39, 0x2c, 0x00, 0x34, 0x02});
    ILI9341_CMD(0xF7, {0x20});
    ILI9341_CMD(0xEA, {0x00, 0x00});
    ILI9341_CMD(0xC0, {0x1B});                                  // Power control   //VRH[5:0]
    ILI9341_CMD(0xC1, {0x12});                                  // Power control   //SAP[2:0];BT[3:0]
    ILI9341_CMD(0xC5, {0x32, 0x3C});                            // VCM control
    ILI9341_CMD(0xC7, {0x91});                                  // VCM control2
    ILI9341_CMD(0x36, {(0x20|0x80|0x08)});                      // Memory Access Control
    ILI9341_CMD(0x36, {(0x40|0x80|0x08)});                      // Memory Access Control
    //ILI9341_CMD(0x21, {0x80});                                  // invert colors
    ILI9341_CMD(0xB1, {0x00, 0x10});                            // Frame Rate Control (1B=70, 1F=61, 10=119)
    ILI9341_CMD(0xB6, {0x0A, 0xA2});                            // Display Function Control
    ILI9341_CMD(0xF6, {0x01, 0x30});
    ILI9341_CMD(0xF2, {0x00});                                  // 3Gamma Function Disable
    ILI9341_CMD(0x26, {0x01});                                  // Gamma curve selected
    ILI9341_CMD(0xE0, {0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00}); // Set Gamma
    ILI9341_CMD(0xE1, {0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F}); // Set Gamma
    ILI9341_CMD(0x11, {}); // Exit Sleep
    ILI9341_CMD(0x29, {}); // Display on
#elif RG_SCREEN_TYPE == 4
	ILI9341_CMD(0x01, {});  // Reset
	usleep(120 * 1000);
	ILI9341_CMD(0x3A, {0X05});  //65k mode
	ILI9341_CMD(0xC5, {0x1A}); //VCOM
	ILI9341_CMD(0x36, {0x60}); //Display Rotation
	ILI9341_CMD(0xB2, {0x05, 0x05, 0x00, 0x33, 0x33});  //Porch Setting
	ILI9341_CMD(0xB7, {0x05});  //Gate Control //12.2v   -10.43v
	ILI9341_CMD(0xBB, {0x3F});  //VCOM
	ILI9341_CMD(0xC0, {0x2c});  //Power control
	ILI9341_CMD(0xC2, {0x01});  //VDV and VRH Command Enable
	ILI9341_CMD(0xC3, {0x0F});  //VRH Set 4.3+( vcom+vcom offset+vdv)
	ILI9341_CMD(0xC4, {0xBE});  //VDV Set 0v
	ILI9341_CMD(0xC6, {0X01});  //Frame Rate Control in Normal Mode 111Hz
	ILI9341_CMD(0xD0, {0xA4,0xA1});  //Power Control 1
	ILI9341_CMD(0xE8, {0x03});   //Power Control 1
	ILI9341_CMD(0xE9, {0x09,0x09,0x08});  //Equalize time control
	ILI9341_CMD(0xE0, {0xD0,0x05,0x09,0x09,0x08,0x14,0x28,0x33,0x3F,0x07,0x13,0x14,0x28,0x30});   //Set Gamma
	ILI9341_CMD(0xE1, {0xD0, 0x05, 0x09, 0x09, 0x08, 0x03, 0x24, 0x32, 0x32, 0x3B, 0x14, 0x13, 0x28, 0x2F, 0x1F});   //Set Gamma
	ILI9341_CMD(0x20, {0x00});   //Reverse Display
	ILI9341_CMD(0x11, {0x03});   //Exit Sleep
	ILI9341_CMD(0x29, {0x03});   //Display on
	usleep(100 * 1000);
#elif RG_SCREEN_TYPE == 5 // Game Box Mini Screen
    ILI9341_CMD(0x3A, {0x55}); // Pixel Format Set RGB565
    ILI9341_CMD(0x0c, {0x0c, 0x00, 0x33, 0x33});
    ILI9341_CMD(0xB7, {0x72});
    ILI9341_CMD(0xBB, {0x3d});
    ILI9341_CMD(0xC0, {0x2C});                                  // Power control 
    ILI9341_CMD(0xC2, {0x01, 0xFF});
    ILI9341_CMD(0xC3, {0x19});
    ILI9341_CMD(0xC4, {0x20});
    ILI9341_CMD(0xC6, {0x0f});
    ILI9341_CMD(0xD0, {0xA4, 0xA1});
    ILI9341_CMD(0xE0, {0xD0, 0x00, 0x05, 0x0E, 0x15, 0x0D, 0x37, 0x43, 0x47, 0x09, 0x15, 0x12, 0x16, 0x19}); // Set Gamma
    ILI9341_CMD(0xE1, {0xD0, 0x00, 0x05, 0x0D, 0x0C, 0x06, 0x2D, 0x44, 0x40, 0x0E, 0x1C, 0x18, 0x16, 0x19}); // Set Gamma
    ILI9341_CMD(0x21, {0x80});   
    ILI9341_CMD(0x11, {0x80}); // Exit Sleep
    ILI9341_CMD(0x29, {0x80}); // Display on
#else
    #error "LCD init sequence is not defined for this device!"
#endif

    rg_display_clear(C_BLACK);
    rg_task_delay(10);
    lcd_set_backlight(config.backlight);
}

static void lcd_deinit(void)
{
#ifdef RG_TARGET_MRGC_G32
    // Normally we skip these steps to avoid LCD flicker, but it
    // is necessary on the G32 because of a bug in the bootmenu
    lcd_set_backlight(0);
    ili9341_cmd(0x01, NULL, 0); // Reset
#endif
    spi_deinit();
    // gpio_reset_pin(RG_GPIO_LCD_BCKL);
    // gpio_reset_pin(RG_GPIO_LCD_DC);
}

static void lcd_set_window(int left, int top, int width, int height)
{
    int right = left + width - 1;
    int bottom = top + height - 1;

    if (left < 0 || top < 0 || right >= RG_SCREEN_WIDTH || bottom >= RG_SCREEN_HEIGHT)
    {
        RG_LOGW("Bad lcd window (x0=%d, y0=%d, x1=%d, y1=%d)\n", left, top, right, bottom);
    }

    ili9341_cmd(0x2A, (uint8_t[]){left >> 8, left & 0xff, right >> 8, right & 0xff}, 4); // Horiz
    ili9341_cmd(0x2B, (uint8_t[]){top >> 8, top & 0xff, bottom >> 8, bottom & 0xff}, 4); // Vert
    ili9341_cmd(0x2C, NULL, 0); // Memory write
    // if (height > 1)
    //     ili9341_cmd(0x3C, NULL, 0); // Memory write continue
}

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

static inline void write_rect(int left, int top, int width, int height,
                              const void *framebuffer, const uint16_t *palette)
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
    const int lines_per_buffer = SPI_BUFFER_LENGTH / scaled_width;
    const int ix_acc = (x_inc * scaled_left) % screen_width;
    const int filter_mode = config.scaling ? config.filter : 0;
    const bool filter_y = filter_mode == RG_DISPLAY_FILTER_VERT || filter_mode == RG_DISPLAY_FILTER_BOTH;
    const bool filter_x = filter_mode == RG_DISPLAY_FILTER_HORIZ || filter_mode == RG_DISPLAY_FILTER_BOTH;
    const int format = display.source.format;
    const int stride = display.source.stride;
    union { const uint8_t *u8; const uint16_t *u16; } buffer;

    if (scaled_width < 1 || scaled_height < 1)
    {
        return;
    }

    buffer.u8 = framebuffer + display.source.offset + (top * stride) + (left * display.source.pixlen);

    lcd_set_window(
        screen_left + RG_SCREEN_MARGIN_LEFT,
        screen_top + RG_SCREEN_MARGIN_TOP,
        scaled_width,
        scaled_height
    );

    for (int y = 0, screen_y = screen_top; y < height;)
    {
        int lines_to_copy = RG_MIN(lines_per_buffer, screen_bottom - screen_y);

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

        uint16_t *line_buffer = spi_get_buffer();
        uint16_t *line_buffer_ptr = line_buffer;

        for (int i = 0; i < lines_to_copy; ++i)
        {
            if (i > 0 && screen_line_is_empty[screen_y])
            {
                memcpy(line_buffer_ptr, line_buffer_ptr - scaled_width, scaled_width * 2);
                line_buffer_ptr += scaled_width;
            }
            else
            {
                #define RENDER_LINE(pixel) { \
                    for (int x = 0, x_acc = ix_acc; x < width;) { \
                        *line_buffer_ptr++ = (pixel); \
                        x_acc += x_inc; \
                        while (x_acc >= screen_width) { \
                            x_acc -= screen_width; \
                            ++x; \
                        } \
                    } \
                }
                if (format & RG_PIXEL_PAL)
                    RENDER_LINE(palette[buffer.u8[x]])
                else if (format & RG_PIXEL_LE)
                    RENDER_LINE((buffer.u16[x] << 8) | (buffer.u16[x] >> 8))
                else
                    RENDER_LINE(buffer.u16[x])
            }

            if (!screen_line_is_empty[++screen_y])
            {
                buffer.u8 += stride;
                ++y;
            }
        }

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
                    uint16_t *buffer = line_buffer + y * scaled_width;
                    for (int x = 0, frame_x = 0, prev_frame_x = -1, x_acc = ix_acc; x < scaled_width; ++x)
                    {
                        if (frame_x == prev_frame_x && x > 0 && x + 1 < scaled_width)
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

static void update_viewport_scaling(void)
{
    int src_width = display.source.width;
    int src_height = display.source.height;
    int new_width = src_width;
    int new_height = src_height;
    double new_ratio = 0.0;

    if (config.scaling == RG_DISPLAY_SCALING_FILL)
    {
        new_ratio = display.screen.width / (double)display.screen.height;
    }
    else if (config.scaling == RG_DISPLAY_SCALING_FIT)
    {
        new_ratio = src_width / (double)src_height;
    }

    if (new_ratio > 0.0)
    {
        new_width = display.screen.height * new_ratio;
        new_height = display.screen.height;

        if (new_width > display.screen.width)
        {
            RG_LOGW("new_width too large: %d, reducing new_height to maintain ratio.\n", new_width);
            new_height = display.screen.height * (display.screen.width / (double)new_width);
            new_width = display.screen.width;
        }
    }

    display.viewport.x_pos = (display.screen.width - new_width) / 2;
    display.viewport.y_pos = (display.screen.height - new_height) / 2;
    display.viewport.x_inc = display.screen.width / (new_width / (double)src_width);
    display.viewport.y_inc = display.screen.height / (new_height / (double)src_height);
    display.viewport.width = new_width;
    display.viewport.height = new_height;

    // Build boundary tables used by filtering

    memset(filter_lines, 1, sizeof(filter_lines));
    memset(screen_line_is_empty, 0, RG_SCREEN_HEIGHT);

    int y_acc = (display.viewport.y_inc * display.viewport.y_pos) % display.screen.height;

    for (int y = 0, screen_y = display.viewport.y_pos; y < src_height && screen_y < display.screen.height; ++screen_y)
    {
        int repeat = ++filter_lines[y].repeat;

        filter_lines[y].start = repeat == 1 || repeat == 2;
        filter_lines[y].stop = repeat == 1;
        screen_line_is_empty[screen_y] = repeat > 1;

        y_acc += display.viewport.y_inc;
        while (y_acc >= display.screen.height)
        {
            y_acc -= display.screen.height;
            ++y;
        }
    }

    RG_LOGI("%dx%d@%.3f => %dx%d@%.3f x_pos:%d y_pos:%d x_inc:%d y_inc:%d\n", src_width, src_height,
            src_width / (double)src_height, new_width, new_height, new_ratio, display.viewport.x_pos,
            display.viewport.y_pos, display.viewport.x_inc, display.viewport.y_inc);
}

static void display_task(void *arg)
{
    display_task_queue = xQueueCreate(1, sizeof(rg_video_update_t *));

    while (1)
    {
        rg_video_update_t *update;

        xQueuePeek(display_task_queue, &update, portMAX_DELAY);
        // xQueueReceive(display_task_queue, &update, portMAX_DELAY);

        // Received a shutdown request!
        if (update == (void *)-1)
            break;

        if (display.changed)
        {
            if (config.scaling != RG_DISPLAY_SCALING_FILL)
                rg_display_clear(C_BLACK);
            update_viewport_scaling();
            update->type = RG_UPDATE_FULL;
            display.changed = false;
        }

        if (update->type == RG_UPDATE_FULL)
        {
            // write_rect();
            update->diff[0] = (rg_line_diff_t){
                .left = 0,
                .width = display.source.width,
                .repeat = display.source.height,
            };
        }

        // It's better to update the counters before we start the transfer, in case someone needs it
        if (update->type == RG_UPDATE_FULL)
            counters.fullFrames++;
        counters.totalFrames++;

        for (int y = 0; y < display.source.height;)
        {
            rg_line_diff_t *diff = &update->diff[y];

            if (diff->width > 0)
            {
                write_rect(diff->left, y, diff->width, diff->repeat, update->buffer, update->palette);
            }
            y += diff->repeat;
        }

        xQueueReceive(display_task_queue, &update, portMAX_DELAY);

        lcd_vsync();
    }

    vQueueDelete(display_task_queue);
    display_task_queue = NULL;

    rg_task_delete(NULL);
}

void rg_display_force_redraw(void)
{
    display.changed = true;
}

const rg_display_t *rg_display_get_info(void)
{
    return &display;
}

rg_display_counters_t rg_display_get_counters(void)
{
    return counters;
}

rg_display_config_t rg_display_get_config(void)
{
    return config;
}

void rg_display_set_update_mode(display_update_t update_mode)
{
    config.update_mode = RG_MIN(RG_MAX(0, update_mode), RG_DISPLAY_UPDATE_COUNT - 1);
    rg_settings_set_number(NS_APP, SETTING_UPDATE, config.update_mode);
    display.changed = true;
}

display_update_t rg_display_get_update_mode(void)
{
    return config.update_mode;
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

void rg_display_set_backlight(int percent)
{
    config.backlight = RG_MIN(RG_MAX(percent, 0), 100);
    rg_settings_set_number(NS_GLOBAL, SETTING_BACKLIGHT, config.backlight);
    lcd_set_backlight(config.backlight);
}

int rg_display_get_backlight(void)
{
    return config.backlight;
}

bool rg_display_save_frame(const char *filename, const rg_video_update_t *frame, int width, int height)
{
    rg_image_t *original = rg_image_alloc(display.source.width, display.source.height);
    if (!original)
        return false;

    uint16_t *dst_ptr = original->data;

    for (int y = 0; y < original->height; y++)
    {
        const uint8_t *src_ptr8 = frame->buffer + display.source.offset + (y * display.source.stride);
        const uint16_t *src_ptr16 = (const uint16_t *)src_ptr8;

        for (int x = 0; x < original->width; x++)
        {
            uint16_t pixel;

            if (display.source.format & RG_PIXEL_PAL)
                pixel = frame->palette[src_ptr8[x]];
            else
                pixel = src_ptr16[x];

            if (!(display.source.format & RG_PIXEL_LE))
                pixel = (pixel << 8) | (pixel >> 8);

            *(dst_ptr++) = pixel;
        }
    }

    rg_image_t *img = rg_image_copy_resampled(original, width, height, 0);
    rg_image_free(original);

    bool success = img && rg_image_save_to_file(filename, img, 0);
    rg_image_free(img);

    return success;
}

IRAM_ATTR
rg_update_t rg_display_submit(/*const*/ rg_video_update_t *update, const rg_video_update_t *previousUpdate)
{
    const int64_t time_start = rg_system_timer();
    // RG_ASSERT(display.source.width && display.source.height, "Source format not set!");
    RG_ASSERT(update, "update is null!");

    if (!previousUpdate || display.changed || config.update_mode == RG_DISPLAY_UPDATE_FULL)
    {
        update->type = RG_UPDATE_FULL;
    }
    else if (PTR_IN_SPIRAM(update->buffer) && PTR_IN_SPIRAM(previousUpdate->buffer))
    {
        // There's no speed benefit in trying to diff when both buffers are in SPIRAM,
        // it will almost always be faster to just update it everything...
        update->type = RG_UPDATE_FULL;
    }
    else // RG_UPDATE_PARTIAL
    {
        const uint32_t *frame_buffer = update->buffer + display.source.offset; // uint64_t is 0.7% faster!
        const uint32_t *prev_buffer = previousUpdate->buffer + display.source.offset;
        const int frame_width = display.source.width;
        const int frame_height = display.source.height;
        const int stride = display.source.stride;
        const int blocks = (frame_width * display.source.pixlen) / sizeof(*frame_buffer);
        const int pixels_per_block = sizeof(*frame_buffer) / display.source.pixlen;
        rg_line_diff_t *out_diff = update->diff;

        // If more than 50% of the screen has changed then stop the comparison and assume that the
        // rest also changed. This is true in 77% of the cases in Pokemon, resulting in a net
        // benefit. The other 23% of cases would have benefited from finishing the diff, so a better
        // heuristic might be preferable (interlaced compare perhaps?).
        int threshold = (frame_width * frame_height) / 2;
        int changed = 0;

        for (int y = 0; y < frame_height; ++y)
        {
            int left = 0, width = 0;

            for (int x = 0; x < blocks && changed < threshold; ++x)
            {
                if (frame_buffer[x] != prev_buffer[x])
                {
                    for (int xl = blocks - 1; xl >= x; --xl)
                    {
                        if (frame_buffer[xl] != prev_buffer[xl])
                        {
                            left = x * pixels_per_block;
                            width = ((xl + 1) - x) * pixels_per_block;
                            changed += width;
                            break;
                        }
                    }
                    break;
                }
            }

            out_diff[y].left = left;
            out_diff[y].width = width;
            out_diff[y].repeat = 1;

            frame_buffer = (void *)frame_buffer + stride;
            prev_buffer = (void *)prev_buffer + stride;
        }

        if (changed == 0)
        {
            update->type = RG_UPDATE_EMPTY;
        }
        else if (changed >= threshold)
        {
            update->type = RG_UPDATE_FULL;
        }
        else
        {
            update->type = RG_UPDATE_PARTIAL;

            // If filtering is enabled we must adjust our diff blocks to be on appropriate boundaries
            if (config.filter && config.scaling)
            {
                for (int y = 0; y < frame_height; ++y)
                {
                    if (out_diff[y].width < 1)
                        continue;

                    int block_start = y;
                    int block_end = y;
                    int left = out_diff[y].left;
                    int right = left + out_diff[y].width;

                    while (block_start > 0 && (out_diff[block_start].width > 0 || !filter_lines[block_start].start))
                        block_start--;

                    while (block_end < frame_height - 1 &&
                           (out_diff[block_end].width > 0 || !filter_lines[block_end].stop))
                        block_end++;

                    for (int i = block_start; i <= block_end; i++)
                    {
                        if (out_diff[i].width > 0)
                        {
                            right = RG_MAX(right, out_diff[i].left + out_diff[i].width);
                            left = RG_MIN(left, out_diff[i].left);
                        }
                    }

                    left = RG_MAX(left - 1, 0);
                    right = RG_MIN(right + 1, frame_width);

                    for (int i = block_start; i <= block_end; i++)
                    {
                        out_diff[i].left = left;
                        out_diff[i].width = right - left;
                    }

                    y = block_end;
                }
            }

            // Combine consecutive lines with similar changes location to optimize the SPI transfer
            rg_line_diff_t *line = &out_diff[frame_height - 1];
            rg_line_diff_t *prev_line = line - 1;

            for (; line > out_diff; --line, --prev_line)
            {
                int right = line->left + line->width;
                int right_prev = prev_line->left + prev_line->width;

                if (abs(line->left - prev_line->left) <= 8 && abs(right - right_prev) <= 8)
                {
                    if (line->left < prev_line->left)
                        prev_line->left = line->left;
                    prev_line->width = RG_MAX(right, right_prev) - prev_line->left;
                    prev_line->repeat = line->repeat + 1;
                }
            }
        }
    }

    xQueueSend(display_task_queue, &update, portMAX_DELAY);

    counters.busyTime += rg_system_timer() - time_start;

    return update->type;
}

void rg_display_set_source_format(int width, int height, int crop_h, int crop_v, int stride, int format)
{
    rg_display_sync();

    if (width % sizeof(int)) // frame diff doesn't handle non word multiple well right now...
    {
        RG_LOGW("Horizontal resolution (%d) isn't a word size multiple!\n", width);
        width -= width % sizeof(int);
    }
    display.source.crop_h = RG_MAX(RG_MAX(0, width - display.screen.width) / 2, crop_h);
    display.source.crop_v = RG_MAX(RG_MAX(0, height - display.screen.height) / 2, crop_v);
    display.source.width = width - display.source.crop_h * 2;
    display.source.height = height - display.source.crop_v * 2;
    display.source.format = format;
    display.source.stride = stride;
    display.source.pixlen = format & RG_PIXEL_PAL ? 1 : 2;
    display.source.offset = (display.source.crop_v * stride) + (display.source.crop_h * display.source.pixlen);
    display.changed = true;
}

bool rg_display_is_busy(void)
{
    return uxQueueMessagesWaiting(spi_transactions) < SPI_TRANSACTION_COUNT
        || uxQueueMessagesWaiting(display_task_queue);
}

void rg_display_sync(void)
{
    while (uxQueueMessagesWaiting(display_task_queue))
        continue; // Wait until display queue is done
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

    // This can happen when left or top is out of bound
    if (width < 0 || height < 0)
        return;

    // This will work for now because we rarely draw from different threads (so all we need is ensure
    // that we're not interrupting a display update). But what we SHOULD be doing is acquire a lock
    // before every call to lcd_set_window and release it only after the last call to lcd_send_data.
    rg_display_sync();

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
            uint16_t *src = (void *)buffer + ((y + line) * stride);
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

    lcd_vsync();
}

void rg_display_clear(uint16_t color_le)
{
    size_t pixels = RG_SCREEN_WIDTH * RG_SCREEN_HEIGHT;
    uint16_t color = (color_le << 8) | (color_le >> 8);

    // We ignore margins here
    lcd_set_window(0, 0, RG_SCREEN_WIDTH, RG_SCREEN_HEIGHT);

    while (pixels > 0)
    {
        size_t count = RG_MIN(pixels, SPI_BUFFER_LENGTH);
        uint16_t *buffer = spi_get_buffer();

        for (size_t j = 0; j < count; ++j)
            buffer[j] = color;

        lcd_send_data(buffer, count * 2);
        pixels -= count;
    }
}

void rg_display_deinit(void)
{
    void *stop = (void *)-1;
    xQueueSend(display_task_queue, &stop, portMAX_DELAY);
    while (display_task_queue)
        rg_task_delay(1);
    lcd_deinit();
    RG_LOGI("Display terminated.\n");
}

void rg_display_init(void)
{
    RG_LOGI("Initialization...\n");
    // TO DO: We probably should call the setters to ensure valid values...
    config = (rg_display_config_t){
        .backlight = rg_settings_get_number(NS_GLOBAL, SETTING_BACKLIGHT, 80),
        .scaling = rg_settings_get_number(NS_APP, SETTING_SCALING, RG_DISPLAY_SCALING_FILL),
        .filter = rg_settings_get_number(NS_APP, SETTING_FILTER, RG_DISPLAY_FILTER_BOTH),
        .rotation = rg_settings_get_number(NS_APP, SETTING_ROTATION, RG_DISPLAY_ROTATION_AUTO),
        .update_mode = rg_settings_get_number(NS_APP, SETTING_UPDATE, RG_DISPLAY_UPDATE_PARTIAL),
    };
    display = (rg_display_t){
        .screen.width = RG_SCREEN_WIDTH - RG_SCREEN_MARGIN_LEFT - RG_SCREEN_MARGIN_RIGHT,
        .screen.height = RG_SCREEN_HEIGHT - RG_SCREEN_MARGIN_TOP - RG_SCREEN_MARGIN_BOTTOM,
        .changed = true,
    };
    lcd_init();
    rg_task_create("rg_display", &display_task, NULL, 3 * 1024, 5, 1);
    RG_LOGI("Display ready.\n");
}
