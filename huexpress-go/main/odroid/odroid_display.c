#pragma GCC optimize ("O3")

#include "odroid_display.h"
#include "image_sd_card_alert.h"
#include "image_sd_card_unknown.h"
#include "hourglass_empty_black_48dp.h"

#include "image_splash.h"

#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "driver/rtc_io.h"

#include <string.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
const int DUTY_MAX = 0x1fff;

const gpio_num_t SPI_PIN_NUM_MISO = GPIO_NUM_19;
const gpio_num_t SPI_PIN_NUM_MOSI = GPIO_NUM_23;
const gpio_num_t SPI_PIN_NUM_CLK  = GPIO_NUM_18;

const gpio_num_t LCD_PIN_NUM_CS   = GPIO_NUM_5;
const gpio_num_t LCD_PIN_NUM_DC   = GPIO_NUM_21;
const gpio_num_t LCD_PIN_NUM_BCKL = GPIO_NUM_14;
const int LCD_BACKLIGHT_ON_VALUE = 1;
const int LCD_SPI_CLOCK_RATE = 40000000;


#define SPI_TRANSACTION_COUNT (4)
static spi_transaction_t trans[SPI_TRANSACTION_COUNT];
static spi_device_handle_t spi;


#define LINE_BUFFERS (2)
#define LINE_COUNT (5)
#define LINE_BUFFER_SIZE (SCREEN_WIDTH*LINE_COUNT)
uint16_t* line[LINE_BUFFERS];
QueueHandle_t spi_queue;
QueueHandle_t line_buffer_queue;
SemaphoreHandle_t spi_count_semaphore;
spi_transaction_t global_transaction;
bool use_polling = false;
bool isBackLightIntialized = false;

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

static inline uint16_t* line_buffer_get()
{
    uint16_t* buffer;
    if (use_polling) {
        return line[0];
    }

    if (xQueueReceive(line_buffer_queue, &buffer, 1000 / portTICK_RATE_MS) != pdTRUE)
    {
        abort();
    }

    return buffer;
}

static inline void line_buffer_put(uint16_t* buffer)
{
    if (xQueueSend(line_buffer_queue, &buffer, 1000 / portTICK_RATE_MS) != pdTRUE)
    {
        abort();
    }
}

static void spi_task(void *arg)
{
    printf("%s: Entered.\n", __func__);

    uint16_t* param;
    while(1)
    {
        // Ensure only LCD transactions are pulled
        if(xSemaphoreTake(spi_count_semaphore, portMAX_DELAY) == pdTRUE )
        {
            spi_transaction_t* t;

            esp_err_t ret = spi_device_get_trans_result(spi, &t, portMAX_DELAY);
            assert(ret==ESP_OK);

            int dc = (int)t->user & 0x80;
            if(dc)
            {
                line_buffer_put(t->tx_buffer);
            }

            if(xQueueSend(spi_queue, &t, portMAX_DELAY) != pdPASS)
            {
                abort();
            }
        }
        else
        {
            printf("%s: xSemaphoreTake failed.\n", __func__);
        }
    }

    printf("%s: Exiting.\n", __func__);
    vTaskDelete(NULL);

    while (1) {}
}

static void spi_initialize()
{
    spi_queue = xQueueCreate(SPI_TRANSACTION_COUNT, sizeof(void*));
    if(!spi_queue) abort();


    line_buffer_queue = xQueueCreate(LINE_BUFFERS, sizeof(void*));
    if(!line_buffer_queue) abort();

    spi_count_semaphore = xSemaphoreCreateCounting(SPI_TRANSACTION_COUNT, 0);
    if (!spi_count_semaphore) abort();

    xTaskCreatePinnedToCore(&spi_task, "spi_task", 1024 + 768, NULL, 5, NULL, 1);
}



static inline spi_transaction_t* spi_get_transaction()
{
    spi_transaction_t* t;

    if (use_polling) {
        t = &global_transaction;
    } else {
        xQueueReceive(spi_queue, &t, portMAX_DELAY);
    }

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

    if (use_polling) {
        spi_device_polling_transmit(spi, t);
    } else {
        esp_err_t ret = spi_device_queue_trans(spi, t, portMAX_DELAY);
        assert(ret==ESP_OK);

        xSemaphoreGive(spi_count_semaphore);
    }
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
            for (int i = 0; i < len; ++i)
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
static void ili_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc=(int)t->user & 0x01;
    gpio_set_level(LCD_PIN_NUM_DC, dc);
}

//Initialize the display
static void ili_init()
{
    int cmd = 0;

    //Initialize non-SPI GPIOs
    gpio_set_direction(LCD_PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(LCD_PIN_NUM_BCKL, GPIO_MODE_OUTPUT);

    //Send all the commands
    while (ili_init_cmds[cmd].databytes != 0xff)
    {
        ili_cmd(ili_init_cmds[cmd].cmd);

        int len = ili_init_cmds[cmd].databytes & 0x7f;
        if (len) ili_data(ili_init_cmds[cmd].data, len);

        if (ili_init_cmds[cmd].databytes & 0x80)
        {
            vTaskDelay(100 / portTICK_RATE_MS);
        }

        cmd++;
    }
}

static inline void send_reset_column(int left, int right, int len)
{
    ili_cmd(0x2A);
    const uint8_t data[] = { (left) >> 8, (left) & 0xff, right >> 8, right & 0xff };
    ili_data(data, len);
}

static inline void send_reset_page(int top, int bottom, int len)
{
    ili_cmd(0x2B);
    const uint8_t data[] = { top >> 8, top & 0xff, bottom >> 8, bottom & 0xff };
    ili_data(data, len);
}

void send_reset_drawing(int left, int top, int width, int height)
{
    static int last_left = -1;
    static int last_right = -1;
    static int last_top = -1;
    static int last_bottom = -1;

    int right = left + width - 1;
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
    int bottom = SCREEN_HEIGHT - 1;
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

void send_continue_line(uint16_t *line, int width, int lineCount)
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
    // Note: In esp-idf v3.0, settings flash speed to 80Mhz causes the LCD controller
    // to malfunction after a soft-reset.

    // (duty range is 0 ~ ((2**bit_num)-1)


    //configure timer0
    ledc_timer_config_t ledc_timer;
    memset(&ledc_timer, 0, sizeof(ledc_timer));

    ledc_timer.bit_num = LEDC_TIMER_13_BIT; //set timer counter bit number
    ledc_timer.freq_hz = 5000;              //set frequency of pwm
    ledc_timer.speed_mode = LEDC_LOW_SPEED_MODE;   //timer mode,
    ledc_timer.timer_num = LEDC_TIMER_0;    //timer index


    ledc_timer_config(&ledc_timer);


    //set the configuration
    ledc_channel_config_t ledc_channel;
    memset(&ledc_channel, 0, sizeof(ledc_channel));

    //set LEDC channel 0
    ledc_channel.channel = LEDC_CHANNEL_0;
    //set the duty for initialization.(duty range is 0 ~ ((2**bit_num)-1)
    ledc_channel.duty = (LCD_BACKLIGHT_ON_VALUE) ? 0 : DUTY_MAX;
    //GPIO number
    ledc_channel.gpio_num = LCD_PIN_NUM_BCKL;
    //GPIO INTR TYPE, as an example, we enable fade_end interrupt here.
    ledc_channel.intr_type = LEDC_INTR_FADE_END;
    //set LEDC mode, from ledc_mode_t
    ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
    //set LEDC timer source, if different channel use one timer,
    //the frequency and bit_num of these channels should be the same
    ledc_channel.timer_sel = LEDC_TIMER_0;


    ledc_channel_config(&ledc_channel);


    //initialize fade service.
    ledc_fade_func_install(0);

    // duty range is 0 ~ ((2**bit_num)-1)
    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, (LCD_BACKLIGHT_ON_VALUE) ? DUTY_MAX : 0, 500);
    ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);

    isBackLightIntialized = true;
}

#if 1
void backlight_percentage_set(int value)
{
    int duty = DUTY_MAX * (value * 0.01f);

    // //set the configuration
    // ledc_channel_config_t ledc_channel;
    // memset(&ledc_channel, 0, sizeof(ledc_channel));
    //
    // //set LEDC channel 0
    // ledc_channel.channel = LEDC_CHANNEL_0;
    // //set the duty for initialization.(duty range is 0 ~ ((2**bit_num)-1)
    // ledc_channel.duty = duty;
    // //GPIO number
    // ledc_channel.gpio_num = LCD_PIN_NUM_BCKL;
    // //GPIO INTR TYPE, as an example, we enable fade_end interrupt here.
    // ledc_channel.intr_type = LEDC_INTR_FADE_END;
    // //set LEDC mode, from ledc_mode_t
    // ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
    // //set LEDC timer source, if different channel use one timer,
    // //the frequency and bit_num of these channels should be the same
    // ledc_channel.timer_sel = LEDC_TIMER_0;
    //
    //
    // ledc_channel_config(&ledc_channel);

    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty, 500);
    ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);
}
#endif

static uint16_t Blend(uint16_t a, uint16_t b)
{
  // Big endian
  // rrrrrGGG gggbbbbb

  char r0 = (a >> 11) & 0x1f;
  char g0 = (a >> 5) & 0x3f;
  char b0 = (a) & 0x1f;

  char r1 = (b >> 11) & 0x1f;
  char g1 = (b >> 5) & 0x3f;
  char b1 = (b) & 0x1f;

  uint16_t rv = ((r1 - r0) >> 1) + r0;
  uint16_t gv = ((g1 - g0) >> 1) + g0;
  uint16_t bv = ((b1 - b0) >> 1) + b0;

  return (rv << 11) | (gv << 5) | (bv);
}

void ili9341_init()
{
    // Init
    spi_initialize();


    // Line buffers
    const size_t lineSize = 320 * LINE_COUNT * sizeof(uint16_t);
    for (int x = 0; x < LINE_BUFFERS; x++)
    {
        line[x] = heap_caps_malloc(lineSize, MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
        if (!line[x]) abort();

        printf("%s: line_buffer_put(%p)\n", __func__, line[x]);
        line_buffer_put(line[x]);
    }

    // Initialize transactions
    for (int x = 0; x < SPI_TRANSACTION_COUNT; x++)
    {
        void* param = &trans[x];
        xQueueSend(spi_queue, &param, portMAX_DELAY);
    }


    // Initialize SPI
    esp_err_t ret;
    //spi_device_handle_t spi;
    spi_bus_config_t buscfg;
		memset(&buscfg, 0, sizeof(buscfg));

    buscfg.miso_io_num = SPI_PIN_NUM_MISO;
    buscfg.mosi_io_num = SPI_PIN_NUM_MOSI;
    buscfg.sclk_io_num = SPI_PIN_NUM_CLK;
    buscfg.quadwp_io_num=-1;
    buscfg.quadhd_io_num=-1;

    spi_device_interface_config_t devcfg;
		memset(&devcfg, 0, sizeof(devcfg));

    devcfg.clock_speed_hz = LCD_SPI_CLOCK_RATE;
    devcfg.mode = 0;                                //SPI mode 0
    devcfg.spics_io_num = LCD_PIN_NUM_CS;               //CS pin
    devcfg.queue_size = 7;                          //We want to be able to queue 7 transactions at a time
    devcfg.pre_cb = ili_spi_pre_transfer_callback;  //Specify pre-transfer callback to handle D/C line
    devcfg.flags = SPI_DEVICE_NO_DUMMY; //SPI_DEVICE_HALFDUPLEX;

    //Initialize the SPI bus
    ret=spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    assert(ret==ESP_OK);

    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
    assert(ret==ESP_OK);





    //Initialize the LCD
	printf("LCD: calling ili_init.\n");
    ili_init();

	printf("LCD: calling backlight_init.\n");
    backlight_init();

    printf("LCD Initialized (%d Hz).\n", LCD_SPI_CLOCK_RATE);
}

void ili9341_poweroff()
{
    // // Drain SPI queue
    // xTaskToNotify = 0;
    //
     esp_err_t err = ESP_OK;
    //
    // while(err == ESP_OK)
    // {
    //     spi_transaction_t* trans_desc;
    //     err = spi_device_get_trans_result(spi, &trans_desc, 0);
    //
    //     printf("ili9341_poweroff: removed pending transfer.\n");
    // }


    // fade off backlight
    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, (LCD_BACKLIGHT_ON_VALUE) ? 0 : DUTY_MAX, 100);
    ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_WAIT_DONE);


    // Disable LCD panel
    int cmd = 0;
    while (ili_sleep_cmds[cmd].databytes != 0xff)
    {
        //printf("ili9341_poweroff: cmd=%d, ili_sleep_cmds[cmd].cmd=0x%x, ili_sleep_cmds[cmd].databytes=0x%x\n",
        //    cmd, ili_sleep_cmds[cmd].cmd, ili_sleep_cmds[cmd].databytes);

        ili_cmd(ili_sleep_cmds[cmd].cmd);
        ili_data(ili_sleep_cmds[cmd].data, ili_sleep_cmds[cmd].databytes & 0x7f);
        if (ili_sleep_cmds[cmd].databytes & 0x80)
        {
            vTaskDelay(100 / portTICK_RATE_MS);
        }
        cmd++;
    }


    err = rtc_gpio_init(LCD_PIN_NUM_BCKL);
    if (err != ESP_OK)
    {
        abort();
    }

    err = rtc_gpio_set_direction(LCD_PIN_NUM_BCKL, RTC_GPIO_MODE_OUTPUT_ONLY);
    if (err != ESP_OK)
    {
        abort();
    }

    err = rtc_gpio_set_level(LCD_PIN_NUM_BCKL, LCD_BACKLIGHT_ON_VALUE ? 0 : 1);
    if (err != ESP_OK)
    {
        abort();
    }
}

void ili9341_prepare()
{
    // Return use of backlight pin
    esp_err_t err = rtc_gpio_deinit(LCD_PIN_NUM_BCKL);
    if (err != ESP_OK)
    {
        abort();
    }

#if 0
    // Disable backlight
    err = gpio_set_direction(LCD_PIN_NUM_BCKL, GPIO_MODE_OUTPUT);
    if (err != ESP_OK)
    {
        abort();
    }

    err = gpio_set_level(LCD_PIN_NUM_BCKL, LCD_BACKLIGHT_ON_VALUE ? 0 : 1);
    if (err != ESP_OK)
    {
        abort();
    }
#endif
}

// void ili9341_write_frame(uint16_t* buffer)
// {
//     short x, y;
//
//     //xTaskToNotify = xTaskGetCurrentTaskHandle();
//
//     if (buffer == NULL)
//     {
//         // clear the buffer
//         memset(line[0], 0x00, 320 * sizeof(uint16_t));
//
//         // clear the screen
//         send_reset_drawing(0, 0, 320, 240);
//
//         for (y = 0; y < 240; ++y)
//         {
//             send_continue_line(line[0], 320, 1);
//         }
//
//         send_continue_wait();
//     }
//     else
//     {
//         const int displayWidth = 320;
//         const int displayHeight = 240;
//
//
//         send_reset_drawing(0, 0, displayWidth, displayHeight);
//
//         for (y = 0; y < displayHeight; y += 4)
//         {
//             send_continue_line(buffer + y * displayWidth, displayWidth, 4);
//         }
//
//         send_continue_wait();
//     }
// }

void ili9341_write_frame_rectangle(short left, short top, short width, short height, uint16_t* buffer)
{
    short x, y;

    if (left < 0 || top < 0) abort();
    if (width < 1 || height < 1) abort();

    //xTaskToNotify = xTaskGetCurrentTaskHandle();

    send_reset_drawing(left, top, width, height);

    if (buffer == NULL)
    {
        // clear the buffer
        for (int i = 0; i < LINE_BUFFERS; ++i)
        {
            memset(line[i], 0, 320 * sizeof(uint16_t) * LINE_COUNT);
        }

        // clear the screen
        send_reset_drawing(0, 0, 320, 240);

        for (y = 0; y < 240; y += LINE_COUNT)
        {
            uint16_t* line_buffer = line_buffer_get();
            send_continue_line(line_buffer, 320, LINE_COUNT);
        }
    }
    else
    {
        uint16_t* line_buffer = line_buffer_get();

        for (y = 0; y < height; y++)
        {
            memcpy(line_buffer, buffer + y * width, width * sizeof(uint16_t));
            send_continue_line(line_buffer, width, 1);
        }
    }
}

void ili9341_clear(uint16_t color)
{
    odroid_display_lock();
    //xTaskToNotify = xTaskGetCurrentTaskHandle();

    send_reset_drawing(0, 0, 320, 240);


    // clear the buffer
    for (int i = 0; i < LINE_BUFFERS; ++i)
    {
        for (int j = 0; j < 320 * LINE_COUNT; ++j)
        {
            line[i][j] = color;
        }
    }

    // clear the screen
    send_reset_drawing(0, 0, 320, 240);

    for (int y = 0; y < 240; y += LINE_COUNT)
    {
        uint16_t* line_buffer = line_buffer_get();
        send_continue_line(line_buffer, 320, LINE_COUNT);
    }
    odroid_display_unlock();
}

void ili9341_write_frame_rectangleLE(short left, short top, short width, short height, uint16_t* buffer)
{
    short x, y;

    if (left < 0 || top < 0) abort();
    if (width < 1 || height < 1) abort();

    //xTaskToNotify = xTaskGetCurrentTaskHandle();

    send_reset_drawing(left, top, width, height);

    if (buffer == NULL)
    {
        // clear the buffer
        for (int i = 0; i < LINE_BUFFERS; ++i)
        {
            memset(line[i], 0, 320 * sizeof(uint16_t) * LINE_COUNT);
        }

        // clear the screen
        send_reset_drawing(0, 0, 320, 240);

        for (y = 0; y < 240; y += LINE_COUNT)
        {
            uint16_t* line_buffer = line_buffer_get();
            send_continue_line(line_buffer, 320, LINE_COUNT);
        }
    }
    else
    {
        for (y = 0; y < height; y++)
        {
            uint16_t* line_buffer = line_buffer_get();

            for (int i = 0; i < width; ++i)
            {
                uint16_t pixel = buffer[y * width + i];
                line_buffer[i] = pixel << 8 | pixel >> 8;
            }

            send_continue_line(line_buffer, width, 1);
        }
    }
}

void display_tasktonotify_set(int value)
{
    //xTaskToNotify = value;
}

int is_backlight_initialized()
{
    return isBackLightIntialized;
}

void odroid_display_show_splash()
{
    ili9341_write_frame_rectangleLE(0, 0, image_splash.width, image_splash.height, image_splash.pixel_data);

    // // Drain SPI queue
    // xTaskToNotify = 0;
    //
    // esp_err_t err = ESP_OK;
    //
    // while(err == ESP_OK)
    // {
    //     spi_transaction_t* trans_desc;
    //     err = spi_device_get_trans_result(spi, &trans_desc, 0);
    //
    //     //printf("odroid_display_show_splash: removed pending transfer.\n");
    // }
}

void odroid_display_drain_spi()
{
    spi_transaction_t *t[SPI_TRANSACTION_COUNT];
    for (int i = 0; i < SPI_TRANSACTION_COUNT; ++i) {
        xQueueReceive(spi_queue, &t[i], portMAX_DELAY);
    }
    for (int i = 0; i < SPI_TRANSACTION_COUNT; ++i) {
        if (xQueueSend(spi_queue, &t[i], portMAX_DELAY) != pdPASS)
        {
            abort();
        }
    }
}

void odroid_display_show_sderr(int errNum)
{
    switch(errNum)
    {
        case ODROID_SD_ERR_BADFILE:
            ili9341_write_frame_rectangleLE(0, 0, image_sd_card_unknown.width, image_sd_card_unknown.height, image_sd_card_unknown.pixel_data); // Bad File image
            break;

        case ODROID_SD_ERR_NOCARD:
            ili9341_write_frame_rectangleLE(0, 0, image_sd_card_alert.width, image_sd_card_alert.height, image_sd_card_alert.pixel_data); // No Card image
            break;

        default:
            abort();
    }

    // Drain SPI queue
    odroid_display_drain_spi();
}

void odroid_display_show_hourglass()
{
    ili9341_write_frame_rectangleLE((320 / 2) - (image_hourglass_empty_black_48dp.width / 2),
        (240 / 2) - (image_hourglass_empty_black_48dp.height / 2),
        image_hourglass_empty_black_48dp.width,
        image_hourglass_empty_black_48dp.height,
        image_hourglass_empty_black_48dp.pixel_data);
}

SemaphoreHandle_t display_mutex = NULL;

void odroid_display_lock()
{
    if (!display_mutex)
    {
        display_mutex = xSemaphoreCreateMutex();
        if (!display_mutex) abort();
    }

    if (xSemaphoreTake(display_mutex, 1000 / portTICK_RATE_MS) != pdTRUE)
    {
        abort();
    }
}

void odroid_display_unlock()
{
    if (!display_mutex) abort();

    odroid_display_drain_spi();
    xSemaphoreGive(display_mutex);
}

#define ODROID_DISPLAY_EMU_IMPL
#include "odroid_display_emu.h"
