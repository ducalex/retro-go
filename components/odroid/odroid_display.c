#pragma GCC optimize ("O3")

#include "odroid_display.h"
#include "odroid_settings.h"
#include "odroid_system.h"
#include "odroid_colors.h"
#include "odroid_image_sdcard.h"
#include "odroid_image_hourglass.h"

#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "driver/rtc_io.h"

#include <string.h>

const gpio_num_t SPI_PIN_NUM_MISO = GPIO_NUM_19;
const gpio_num_t SPI_PIN_NUM_MOSI = GPIO_NUM_23;
const gpio_num_t SPI_PIN_NUM_CLK  = GPIO_NUM_18;
const gpio_num_t LCD_PIN_NUM_CS   = GPIO_NUM_5;
const gpio_num_t LCD_PIN_NUM_DC   = GPIO_NUM_21;
const gpio_num_t LCD_PIN_NUM_BCKL = GPIO_NUM_14;

const int BACKLIGHT_DUTY_MAX = 0x1fff;

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define SPI_TRANSACTION_COUNT (5)
#define SPI_TRANSACTION_BUFFER_LINE_COUNT (6)
#define SPI_TRANSACTION_BUFFER_SIZE (SCREEN_WIDTH*SPI_TRANSACTION_BUFFER_LINE_COUNT*2)

// Maximum amount of change (percent) in a frame before we trigger a full transfer
// instead of a partial update (faster). This also allows us to stop the diff early!
#define FULL_UPDATE_THRESHOLD (0.4f)

static uint16_t* spi_buffers[SPI_TRANSACTION_COUNT];
static QueueHandle_t spi_queue;
static QueueHandle_t spi_buffer_queue;
static SemaphoreHandle_t spi_count_semaphore;
static SemaphoreHandle_t display_mutex;
static spi_transaction_t trans[SPI_TRANSACTION_COUNT];
static spi_device_handle_t spi;

static int8_t backlightLevels[] = {10, 25, 50, 75, 100};
static int8_t backlightLevel = ODROID_BACKLIGHT_LEVEL2;

volatile int8_t scalingMode = ODROID_SCALING_FILL;
volatile int8_t displayUpdateMode = ODROID_DISPLAY_UPDATE_AUTO;
volatile int8_t forceVideoRefresh = true;
volatile static QueueHandle_t videoTaskQueue;

/*
 The ILI9341 needs a bunch of command/argument values to be initialized. They are stored in this struct.
*/
typedef struct {
    uint8_t cmd;
    uint8_t data[128];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} ili_init_cmd_t;

#define TFT_CMD_SWRESET	0x01
#define TFT_CMD_SLEEP 0x10
#define TFT_CMD_DISPLAY_OFF 0x28

#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_MH 0x04
#define TFT_RGB_BGR 0x08

DRAM_ATTR static const ili_init_cmd_t ili_sleep_cmds[] = {
    {TFT_CMD_SWRESET, {0}, 0x80},
    {TFT_CMD_DISPLAY_OFF, {0}, 0x80},
    {TFT_CMD_SLEEP, {0}, 0x80},
    {0, {0}, 0xff}
};


// 2.4" LCD
DRAM_ATTR static const ili_init_cmd_t ili_init_cmds[] = {
    // VCI=2.8V
    //************* Start Initial Sequence **********//
    {TFT_CMD_SWRESET, {0}, 0x80},
    {0xCF, {0x00, 0xc3, 0x30}, 3},
    {0xED, {0x64, 0x03, 0x12, 0x81}, 4},
    {0xE8, {0x85, 0x00, 0x78}, 3},
    {0xCB, {0x39, 0x2c, 0x00, 0x34, 0x02}, 5},
    {0xF7, {0x20}, 1},
    {0xEA, {0x00, 0x00}, 2},
    {0xC0, {0x1B}, 1},    //Power control   //VRH[5:0]
    {0xC1, {0x12}, 1},    //Power control   //SAP[2:0];BT[3:0]
    {0xC5, {0x32, 0x3C}, 2},    //VCM control
    {0xC7, {0x91}, 1},    //VCM control2
    //{0x36, {(MADCTL_MV | MADCTL_MX | TFT_RGB_BGR)}, 1},    // Memory Access Control
    {0x36, {(MADCTL_MV | MADCTL_MY | TFT_RGB_BGR)}, 1},    // Memory Access Control
    {0x3A, {0x55}, 1},
    {0xB1, {0x00, 0x10}, 2},  // Frame Rate Control (1B=70, 1F=61, 10=119)
    {0xB6, {0x0A, 0xA2}, 2},    // Display Function Control
    {0xF6, {0x01, 0x30}, 2},
    {0xF2, {0x00}, 1},    // 3Gamma Function Disable
    {0x26, {0x01}, 1},     //Gamma curve selected

    //Set Gamma
    {0xE0, {0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00}, 15},
    {0XE1, {0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F}, 15},

/*
    // LUT
    {0x2d, {0x01, 0x03, 0x05, 0x07, 0x09, 0x0b, 0x0d, 0x0f, 0x11, 0x13, 0x15, 0x17, 0x19, 0x1b, 0x1d, 0x1f,
            0x21, 0x23, 0x25, 0x27, 0x29, 0x2b, 0x2d, 0x2f, 0x31, 0x33, 0x35, 0x37, 0x39, 0x3b, 0x3d, 0x3f,
            0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
            0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c,
            0x1d, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x26, 0x27, 0x28, 0x29, 0x2a,
            0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
            0x00, 0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e, 0x10, 0x12, 0x12, 0x14, 0x16, 0x18, 0x1a,
            0x1c, 0x1e, 0x20, 0x22, 0x24, 0x26, 0x26, 0x28, 0x2a, 0x2c, 0x2e, 0x30, 0x32, 0x34, 0x36, 0x38}, 128},
*/

    {0x11, {0}, 0x80},    //Exit Sleep
    {0x29, {0}, 0x80},    //Display on

    {0, {0}, 0xff}
};

static inline void odroid_display_lock_fast()
{
    if (xSemaphoreTake(display_mutex, portMAX_DELAY) != pdPASS)
        abort();
}

static inline uint16_t* spi_get_buffer()
{
    uint16_t* buffer;

    if (xQueueReceive(spi_buffer_queue, &buffer, 1000 / portTICK_RATE_MS) != pdTRUE)
    {
        abort();
    }

    return buffer;
}

static inline void spi_put_buffer(uint16_t* buffer)
{
    if (xQueueSend(spi_buffer_queue, &buffer, 1000 / portTICK_RATE_MS) != pdTRUE)
    {
        abort();
    }
}

static inline spi_transaction_t* spi_get_transaction()
{
    spi_transaction_t* t;

    xQueueReceive(spi_queue, &t, portMAX_DELAY);

    memset(t, 0, sizeof(*t));

    return t;
}

static inline void spi_put_transaction(spi_transaction_t* t)
{
    t->rx_buffer = NULL;
    t->rxlength = t->length;

    if (t->flags & SPI_TRANS_USE_TXDATA)
    {
        t->flags |= SPI_TRANS_USE_RXDATA;
    }

    // spi_device_polling_transmit(spi, t);
    esp_err_t ret = spi_device_queue_trans(spi, t, portMAX_DELAY);
    assert(ret==ESP_OK);

    xSemaphoreGive(spi_count_semaphore);
}

//Send a command to the ILI9341. Uses spi_device_transmit, which waits until the transfer is complete.
static void ili_cmd(const uint8_t cmd)
{
    spi_transaction_t* t = spi_get_transaction();

    t->length = 8;                     //Command is 8 bits
    t->tx_data[0] = cmd;               //The data is the cmd itself
    t->user = (void*)0;                //D/C needs to be set to 0
    t->flags = SPI_TRANS_USE_TXDATA;

    spi_put_transaction(t);
}

//Send data to the ILI9341. Uses spi_device_transmit, which waits until the transfer is complete.
static void ili_data(const uint8_t *data, int len)
{
    if (len)
    {
        spi_transaction_t* t = spi_get_transaction();

        if (len < 5)
        {
            for (short i = 0; i < len; ++i)
            {
                t->tx_data[i] = data[i];
            }
            t->length = len * 8;               //Len is in bytes, transaction length is in bits.
            t->user = (void*)1;                //D/C needs to be set to 1
            t->flags = SPI_TRANS_USE_TXDATA;
        }
        else
        {
            t->length = len * 8;               //Len is in bytes, transaction length is in bits.
            t->tx_buffer = data;               //Data
            t->user = (void*)1;                //D/C needs to be set to 1
            t->flags = 0;
        }

        spi_put_transaction(t);
    }
}

//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
static void IRAM_ATTR ili_spi_pre_transfer_callback(spi_transaction_t *t)
{
    gpio_set_level(LCD_PIN_NUM_DC, (int)t->user & 0x01);
}

static void spi_task(void *arg)
{
    while(1)
    {
        // Ensure only LCD transactions are pulled
        if (xSemaphoreTake(spi_count_semaphore, portMAX_DELAY) == pdTRUE)
        {
            spi_transaction_t* t;

            esp_err_t ret = spi_device_get_trans_result(spi, &t, portMAX_DELAY);
            assert(ret==ESP_OK);

            int dc = (int)t->user & 0x80;
            if (dc)
            {
                spi_put_buffer(t->tx_buffer);
            }

            if (xQueueSend(spi_queue, &t, portMAX_DELAY) != pdPASS)
            {
                abort();
            }
        }
        else
        {
            printf("%s: xSemaphoreTake failed.\n", __func__);
        }
    }

    vTaskDelete(NULL);

    while (1) {}
}

static inline void send_reset_column(short left, short right, int len)
{
    ili_cmd(0x2A);
    const uint8_t data[] = { (left) >> 8, (left) & 0xff, right >> 8, right & 0xff };
    ili_data(data, len);
}

static inline void send_reset_page(short top, short bottom, int len)
{
    ili_cmd(0x2B);
    const uint8_t data[] = { top >> 8, top & 0xff, bottom >> 8, bottom & 0xff };
    ili_data(data, len);
}

static void send_reset_drawing(short left, short top, short width, short height)
{
    static short last_left = -1;
    static short last_right = -1;
    static short last_top = -1;
    static short last_bottom = -1;

    short right = left + width - 1;
    if (height == 1) {
        if (last_right > right) right = last_right;
        else right = SCREEN_WIDTH - 1;
    }
    if (left != last_left || right != last_right) {
        send_reset_column(left, right, (right != last_right) ?  4 : 2);
        last_left = left;
        last_right = right;
    }

    //int bottom = (top + height - 1);
    short bottom = SCREEN_HEIGHT - 1;
    if (top != last_top || bottom != last_bottom) {
        send_reset_page(top, bottom, (bottom != last_bottom) ? 4 : 2);
        last_top = top;
        last_bottom = bottom;
    }

    ili_cmd(0x2C);           //memory write
    if (height > 1) {
        ili_cmd(0x3C);           //memory write continue
    }
}

static void send_continue_line(uint16_t *line, int width, int lineCount)
{
    spi_transaction_t* t = spi_get_transaction();
    t->length = width * 2 * lineCount * 8;
    t->tx_buffer = line;
    t->user = (void*)0x81;
    t->flags = 0;

    spi_put_transaction(t);
}

static void backlight_init()
{
    // Initial backlight percent
    int percent = backlightLevels[backlightLevel % ODROID_BACKLIGHT_LEVEL_COUNT];

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
    ledc_channel.gpio_num = LCD_PIN_NUM_BCKL;
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

static void backlight_deinit()
{
    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0, 100);
    ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_WAIT_DONE);
    ledc_fade_func_uninstall();
}

static void backlight_percentage_set(int value)
{
    int duty = BACKLIGHT_DUTY_MAX * (value * 0.01f);

    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty, 10);
    ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);
}

static void ili9341_init()
{
    short cmd = 0;

    //Initialize non-SPI GPIOs
    gpio_set_direction(LCD_PIN_NUM_DC, GPIO_MODE_OUTPUT);
    //gpio_set_direction(LCD_PIN_NUM_BCKL, GPIO_MODE_OUTPUT);

    //Send all the commands
    while (ili_init_cmds[cmd].databytes != 0xff)
    {
        ili_cmd(ili_init_cmds[cmd].cmd);

        int len = ili_init_cmds[cmd].databytes & 0x7f;
        if (len) ili_data(ili_init_cmds[cmd].data, len);

        // if (ili_init_cmds[cmd].databytes & 0x80)
        // {
        //     vTaskDelay(10 / portTICK_RATE_MS);
        // }

        cmd++;
    }
}

static void ili9341_deinit()
{
    esp_err_t err = ESP_OK;

    backlight_deinit();

    // Disable LCD panel
    short cmd = 0;
    while (ili_sleep_cmds[cmd].databytes != 0xff)
    {
        ili_cmd(ili_sleep_cmds[cmd].cmd);
        ili_data(ili_sleep_cmds[cmd].data, ili_sleep_cmds[cmd].databytes & 0x7f);
        cmd++;
    }

    err = rtc_gpio_init(LCD_PIN_NUM_BCKL);
    assert(err == ESP_OK);

    err = rtc_gpio_set_direction(LCD_PIN_NUM_BCKL, RTC_GPIO_MODE_OUTPUT_ONLY);
    assert(err == ESP_OK);

    err = rtc_gpio_set_level(LCD_PIN_NUM_BCKL, 0);
    assert(err == ESP_OK);
}

void ili9341_clear_screen(uint16_t color)
{
    for (short i = 0; i < SPI_TRANSACTION_COUNT; ++i)
    {
        if ((color & 0xFF) == (color >> 8))
        {
            memset(spi_buffers[i], color & 0xFF, SPI_TRANSACTION_BUFFER_SIZE);
            continue;
        }
        for (short j = 0; j < SCREEN_WIDTH * SPI_TRANSACTION_BUFFER_LINE_COUNT; ++j)
        {
            spi_buffers[i][j] = color << 8 | color >> 8;
        }
    }

    send_reset_drawing(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    for (short y = 0; y < SCREEN_HEIGHT; y += SPI_TRANSACTION_BUFFER_LINE_COUNT)
    {
        uint16_t* buffer = spi_get_buffer();
        send_continue_line(buffer, SCREEN_WIDTH, SPI_TRANSACTION_BUFFER_LINE_COUNT);
    }
}

void ili9341_write_frame_rectangleLE(short left, short top, short width, short height, uint16_t* buffer)
{
    short x, y, i;

    if (left < 0 || top < 0) abort();
    if (width < 1 || height < 1) abort();

    send_reset_drawing(left, top, width, height);

    if (buffer == NULL)
    {
        ili9341_clear_screen(C_BLACK);
        return;
    }

    for (y = 0; y < height; y++)
    {
        uint16_t* line_buffer = spi_get_buffer();

        for (i = 0; i < width; ++i)
        {
            uint16_t pixel = buffer[y * width + i];
            line_buffer[i] = pixel << 8 | pixel >> 8;
        }

        send_continue_line(line_buffer, width, 1);
    }
}

static inline void
write_rect(void *buffer, uint16_t *palette, short origin_x, short origin_y,
           short left, short top, short width, short height, short stride,
           short pixel_size, uint8_t pixel_mask, short x_inc, short y_inc,
           uint8_t interpolation)
{
    short actual_left = ((SCREEN_WIDTH * left) + (x_inc - 1)) / x_inc;
    short actual_top = ((SCREEN_HEIGHT * top) + (y_inc - 1)) / y_inc;
    short actual_right = ((SCREEN_WIDTH * (left + width)) + (x_inc - 1)) / x_inc;
    short actual_bottom = ((SCREEN_HEIGHT * (top + height)) + (y_inc - 1)) / y_inc;
    short actual_width = actual_right - actual_left;
    short actual_height = actual_bottom - actual_top;
    short ix_acc = (x_inc * actual_left) % SCREEN_WIDTH;
    short iy_acc = (y_inc * actual_top) % SCREEN_HEIGHT;
    short lines_per_buffer = SPI_TRANSACTION_BUFFER_SIZE / (actual_width * 2);

    if (actual_width == 0 || actual_height == 0) {
        return;
    }

    send_reset_drawing(origin_x + actual_left, origin_y + actual_top,
                       actual_width, actual_height);

    for (short y = 0, y_acc = iy_acc; y < height;)
    {
        uint16_t* line_buffer = spi_get_buffer();

        short line_buffer_index = 0;
        short lines_to_copy = 0;

        for (; (lines_to_copy < lines_per_buffer) && (y < height); ++lines_to_copy)
        {
            for (short x = 0, x_acc = ix_acc; x < width;)
            {
                if (palette == NULL)  {
                    uint16_t sample = ((uint16_t*)buffer)[x];
                    line_buffer[line_buffer_index++] = sample << 8 | sample >> 8;
                } else {
                    line_buffer[line_buffer_index++] = palette[((uint8_t*)buffer)[x] & pixel_mask];
                }

                x_acc += x_inc;
                while (x_acc >= SCREEN_WIDTH) {
                    ++x;
                    x_acc -= SCREEN_WIDTH;
                }
            }

            y_acc += y_inc;
            while (y_acc >= SCREEN_HEIGHT) {
                ++y;
                buffer += stride;
                y_acc -= SCREEN_HEIGHT;
            }
        }

        send_continue_line(line_buffer, actual_width, lines_to_copy);
    }
}

static inline bool
pixel_diff(uint8_t pixel1, uint8_t pixel2, uint16_t *palette1, uint16_t *palette2,
           uint8_t pixel_mask, uint8_t palette_shift_mask)
{
    uint8_t p1 = pixel1 & pixel_mask;
    uint8_t p2 = pixel2 & pixel_mask;

    if (palette_shift_mask) {
        if (pixel1 & palette_shift_mask) p1 += (pixel_mask + 1);
        if (pixel2 & palette_shift_mask) p2 += (pixel_mask + 1);
    }

    return palette1[p1] != palette2[p2];
}

static inline void frame_diff(odroid_video_frame *frame, odroid_video_frame *prevFrame)
{
    uint8_t pixel_mask = frame->pixel_mask;
    odroid_line_diff *out_diff = frame->diff;
    bool use_u32bit = frame->palette == NULL;

    // If the palette didn't change we can speed up things by avoiding pixel_diff()
    if (frame->palette == prevFrame->palette
         || memcmp(frame->palette, prevFrame->palette, (pixel_mask + 1) * 2) == 0)
    {
        pixel_mask |= frame->pal_shift_mask;
        use_u32bit = true;
    }

    int partial_update_remaining = frame->width * frame->height * FULL_UPDATE_THRESHOLD;

    if (displayUpdateMode == ODROID_DISPLAY_UPDATE_PARTIAL) // force partial update
    {
        partial_update_remaining = SCREEN_HEIGHT * SCREEN_WIDTH;
    }

    uint32_t u32_pixel_mask = (pixel_mask << 24)|(pixel_mask << 16)|(pixel_mask << 8)|pixel_mask;
    uint16_t u32_blocks = (frame->width * frame->pixel_size / 4);
    uint16_t u32_pixels = 4 / frame->pixel_size;

    for (int y = 0, i = 0; y < frame->height; ++y, i += frame->stride)
    {
        out_diff[y].top = y;
        out_diff[y].left = 0;
        out_diff[y].width = 0;
        out_diff[y].height = 1;

        if (use_u32bit) {
            // This is only accurate to 4 pixels of course, but much faster
            uint32_t *buffer32 = frame->buffer + i;
            uint32_t *old_buffer32 = prevFrame->buffer + i;
            for (short x = 0; x < u32_blocks; ++x)
            {
                if ((buffer32[x] & u32_pixel_mask) != (old_buffer32[x] & u32_pixel_mask))
                {
                    for (short xl = u32_blocks - 1; xl >= x; --xl)
                    {
                        if ((buffer32[xl] & u32_pixel_mask) != (old_buffer32[xl] & u32_pixel_mask)) {
                            out_diff[y].left = x * u32_pixels;
                            out_diff[y].width = ((xl + 1) - x) * u32_pixels;
                            break;
                        }
                    }
                    break;
                }
            }
        } else {
            uint8_t *buffer8 = frame->buffer + i;
            uint8_t *old_buffer8 = prevFrame->buffer + i;
            for (int x = 0; x < frame->width; ++x)
            {
                if (!pixel_diff(buffer8[x], old_buffer8[x], frame->palette, prevFrame->palette,
                                frame->pixel_mask, frame->pal_shift_mask)) {
                    continue;
                }
                out_diff[y].left = x;

                for (x = frame->width - 1; x >= 0; --x)
                {
                    if (!pixel_diff(buffer8[x], old_buffer8[x], frame->palette, prevFrame->palette,
                                    frame->pixel_mask, frame->pal_shift_mask)) {
                        continue;
                    }
                    out_diff[y].width = (x - out_diff[y].left) + 1;
                    break;
                }
                break;
            }
        }

        partial_update_remaining -= out_diff[y].width;

        if (partial_update_remaining <= 0)
        {
            out_diff[0].top = 0;
            out_diff[0].left = 0;
            out_diff[0].width = frame->width;
            out_diff[0].height = frame->height;
            return; // Stop scan and do full update
        }
    }

    // Combine consecutive lines with similar changes location to optimize the SPI transfer
    for (short y = frame->height - 1; y > 0; --y)
    {
        if (abs(out_diff[y].left - out_diff[y-1].left) > 8)
            continue;

        short right = out_diff[y].left + out_diff[y].width;
        short right_prev = out_diff[y-1].left + out_diff[y-1].width;
        if (abs(right - right_prev) > 8)
            continue;

        if (out_diff[y].left < out_diff[y-1].left)
          out_diff[y-1].left = out_diff[y].left;
        out_diff[y-1].width = (right > right_prev) ?
          right - out_diff[y-1].left : right_prev - out_diff[y-1].left;
        out_diff[y-1].height = out_diff[y].height + 1;
    }
}


static short x_inc = SCREEN_WIDTH;
static short y_inc = SCREEN_HEIGHT;
static short x_origin = 0;
static short y_origin = 0;
static float x_scale = 1.f;
static float y_scale = 1.f;

void odroid_display_reset_scale(short width, short height)
{
    x_inc = SCREEN_WIDTH;
    y_inc = SCREEN_HEIGHT;
    x_origin = (SCREEN_WIDTH - width) / 2;
    y_origin = (SCREEN_HEIGHT - height) / 2;
    x_scale = y_scale = 1.f;
}

void odroid_display_set_scale(short width, short height, float new_ratio)
{
    float old_ratio = width/(float)height;

    if (new_ratio <= 0) {
        new_ratio = width/(float)height;
    }

    short new_width = SCREEN_HEIGHT * new_ratio;
    short new_height = SCREEN_HEIGHT;

    y_scale = new_height / (float)height;
    x_scale = new_width / (float)width;

    x_inc = SCREEN_WIDTH / x_scale;
    y_inc = SCREEN_HEIGHT / y_scale;
    x_origin = (SCREEN_WIDTH - (width * x_scale)) / 2.f;
    y_origin = (SCREEN_HEIGHT - (height * y_scale)) / 2.f;

    printf("%dx%d@%.3f => %dx%d@%.3f x_inc:%d y_inc:%d x_scale:%.3f y_scale:%.3f x_origin:%d y_origin:%d\n",
           width, height, old_ratio, new_width, new_height, new_ratio,
           x_inc, y_inc, x_scale, y_scale, x_origin, y_origin);
}

static void videoTask(void *arg)
{
    videoTaskQueue = xQueueCreate(1, sizeof(void*));

    odroid_video_frame *update;

    while(1)
    {
        xQueuePeek(videoTaskQueue, &update, portMAX_DELAY);

        if (!update) break;

        if (forceVideoRefresh)
        {
            if (scalingMode == ODROID_SCALING_FILL) {
                odroid_display_set_scale(update->width, update->height, 4.f / 3.f);
            }
            else if (scalingMode == ODROID_SCALING_SCALE) {
                odroid_display_set_scale(update->width, update->height, -1);
            }
            else {
                odroid_display_reset_scale(update->width, update->height);
            }
            odroid_display_clear(C_BLACK);
        }

        odroid_display_write_frame(update);
        forceVideoRefresh = false;

        xQueueReceive(videoTaskQueue, &update, portMAX_DELAY);
    }

    videoTaskQueue = NULL;

    vTaskDelete(NULL);

    while (1) {}
}

void IRAM_ATTR odroid_display_write_frame(odroid_video_frame *frame)
{
    odroid_display_lock_fast();
    // spi_device_acquire_bus(spi, portMAX_DELAY);

    for (short y = 0; y < frame->height;)
    {
        odroid_line_diff *diff = &frame->diff[y];

        if (diff->width > 0) {
            write_rect(frame->buffer + (diff->top * frame->stride) + (diff->left * frame->pixel_size),
                        frame->palette, x_origin, y_origin, diff->left, diff->top, diff->width,
                        diff->height, frame->stride, frame->pixel_size, frame->pixel_mask,
                        x_inc, y_inc, 0);
        }
        y += diff->height;
    }

    // spi_device_release_bus(spi);
    odroid_display_unlock();
}

void IRAM_ATTR odroid_display_queue_update(odroid_video_frame *frame, odroid_video_frame *previousFrame)
{
    // uint startTime = xthal_get_ccount();
    if (!forceVideoRefresh && displayUpdateMode != ODROID_DISPLAY_UPDATE_FULL && previousFrame) {
        frame_diff(frame, previousFrame);
    } else if (frame) {
        frame->diff[0].top = 0;
        frame->diff[0].left = 0;
        frame->diff[0].width = frame->width;
        frame->diff[0].height = frame->height;
    }
    xQueueSend(videoTaskQueue, &frame, portMAX_DELAY);
    // printf("Update time: %f\n", (float)get_elapsed_time_since(startTime) / (CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ * 1000000));
}

void IRAM_ATTR odroid_display_drain_spi()
{
    if (uxQueueSpacesAvailable(spi_queue)) {
        spi_transaction_t *t[SPI_TRANSACTION_COUNT];
        for (short i = 0; i < SPI_TRANSACTION_COUNT; ++i) {
            xQueueReceive(spi_queue, &t[i], portMAX_DELAY);
        }
        for (short i = 0; i < SPI_TRANSACTION_COUNT; ++i) {
            xQueueSend(spi_queue, &t[i], portMAX_DELAY);
        }
    }
}

void odroid_display_clear(uint16_t color)
{
    odroid_display_lock_fast();
    ili9341_clear_screen(color);
    odroid_display_unlock();
}

void odroid_display_show_error(int errNum)
{
    switch(errNum)
    {
        case ODROID_SD_ERR_BADFILE:
        case ODROID_SD_ERR_NOCARD:
            ili9341_clear_screen(C_WHITE);
            ili9341_write_frame_rectangleLE((SCREEN_WIDTH / 2) - (image_sdcard_red_48dp.width / 2),
                (SCREEN_HEIGHT / 2) - (image_sdcard_red_48dp.height / 2),
                image_sdcard_red_48dp.width,
                image_sdcard_red_48dp.height,
                image_sdcard_red_48dp.pixel_data);
            break;

        default:
            ili9341_clear_screen(C_RED);
    }

    odroid_display_drain_spi();
}

void odroid_display_show_hourglass()
{
    ili9341_write_frame_rectangleLE((SCREEN_WIDTH / 2) - (image_hourglass_empty_black_48dp.width / 2),
        (SCREEN_HEIGHT / 2) - (image_hourglass_empty_black_48dp.height / 2),
        image_hourglass_empty_black_48dp.width,
        image_hourglass_empty_black_48dp.height,
        image_hourglass_empty_black_48dp.pixel_data);
    odroid_display_drain_spi();
}

void odroid_display_lock()
{
    if (xSemaphoreTake(display_mutex, 1000 / portTICK_RATE_MS) != pdTRUE)
    {
        abort();
    }

    // Wait for all transactions to finish before returning the lock
    // This is mostly for locks requested by SD Card-related code
    odroid_display_drain_spi();
}

void odroid_display_unlock()
{
    // odroid_display_drain_spi();

    xSemaphoreGive(display_mutex);
}

int8_t odroid_display_backlight_get()
{
    return backlightLevel;
}

void odroid_display_backlight_set(int8_t level)
{
    if (level < 0)
    {
        printf("odroid_display_backlight_set: level out of range (%d)\n", level);
        level = 0;
    }
    else if (level >= ODROID_BACKLIGHT_LEVEL_COUNT)
    {
        printf("odroid_display_backlight_set: level out of range (%d)\n", level);
        level = ODROID_BACKLIGHT_LEVEL_COUNT - 1;
    }

    odroid_settings_Backlight_set(level);

    backlightLevel = level;
    backlight_percentage_set(backlightLevels[level]);
}

static void spi_initialize()
{
    spi_queue = xQueueCreate(SPI_TRANSACTION_COUNT, sizeof(void*));
    spi_buffer_queue = xQueueCreate(SPI_TRANSACTION_COUNT, sizeof(void*));
    spi_count_semaphore = xSemaphoreCreateCounting(SPI_TRANSACTION_COUNT, 0);

    for (short x = 0; x < SPI_TRANSACTION_COUNT; x++)
    {
        spi_buffers[x] = heap_caps_malloc(SPI_TRANSACTION_BUFFER_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
        if (!spi_buffers[x]) abort();
        spi_put_buffer(spi_buffers[x]);

        void* param = &trans[x];
        xQueueSend(spi_queue, &param, portMAX_DELAY);
    }

    spi_bus_config_t buscfg;
	memset(&buscfg, 0, sizeof(buscfg));

    buscfg.miso_io_num = SPI_PIN_NUM_MISO;
    buscfg.mosi_io_num = SPI_PIN_NUM_MOSI;
    buscfg.sclk_io_num = SPI_PIN_NUM_CLK;
    buscfg.quadwp_io_num=-1;
    buscfg.quadhd_io_num=-1;

    spi_device_interface_config_t devcfg;
	memset(&devcfg, 0, sizeof(devcfg));

    devcfg.clock_speed_hz = SPI_MASTER_FREQ_40M;    //80Mhz causes glitches unfortunately
    devcfg.mode = 0;                                //SPI mode 0
    devcfg.spics_io_num = LCD_PIN_NUM_CS;           //CS pin
    devcfg.queue_size = SPI_TRANSACTION_COUNT;      //We want to be able to queue 7 transactions at a time
    devcfg.pre_cb = ili_spi_pre_transfer_callback;  //Specify pre-transfer callback to handle D/C line
    devcfg.flags = SPI_DEVICE_NO_DUMMY;             //SPI_DEVICE_HALFDUPLEX;

    //Initialize the SPI bus
    spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    //assert(ret==ESP_OK);

    //Attach the LCD to the SPI bus
    spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
    //assert(ret==ESP_OK);

    xTaskCreatePinnedToCore(&spi_task, "spi_task", 1024 + 768, NULL, 5, NULL, 1);
}

void odroid_display_init()
{
    // Load settings
    backlightLevel = odroid_settings_Backlight_get();
    scalingMode = odroid_settings_Scaling_get();
    displayUpdateMode = odroid_settings_DisplayUpdateMode_get();

    display_mutex = xSemaphoreCreateMutex();

    // Initialize SPI
    printf("LCD: calling spi_initialize.\n");
    spi_initialize();

    //Initialize the LCD
	printf("LCD: calling ili9341_init.\n");
    ili9341_init();

	printf("LCD: calling backlight_init.\n");
    backlight_init();

	printf("LCD: starting videoTask.\n");
    xTaskCreatePinnedToCore(&videoTask, "videoTask", 4096, NULL, 5, NULL, 1);

    printf("LCD Initialized.\n");
}
