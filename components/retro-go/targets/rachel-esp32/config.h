// Target definition
#define RG_TARGET_NAME             "RACHEL-ESP32"

// Storage
#define RG_STORAGE_ROOT             "/sd"
#define RG_STORAGE_SDSPI_HOST       SPI3_HOST
#define RG_STORAGE_SDSPI_SPEED      SDMMC_FREQ_DEFAULT
// #define RG_STORAGE_SDMMC_HOST       SDMMC_HOST_SLOT_1
// #define RG_STORAGE_SDMMC_SPEED      SDMMC_FREQ_DEFAULT
// #define RG_STORAGE_FLASH_PARTITION  "vfs"


// Audio
#define RG_AUDIO_USE_INT_DAC        0   // 0 = Disable, 1 = GPIO25, 2 = GPIO26, 3 = Both
#define RG_AUDIO_USE_EXT_DAC        1   // 0 = Disable, 1 = Enable

// Video
#define RG_SCREEN_DRIVER            0   // 0 = ILI9341
#define RG_SCREEN_HOST              SPI2_HOST
#define RG_SCREEN_SPEED             SPI_MASTER_FREQ_40M // SPI_MASTER_FREQ_80M
#define RG_SCREEN_BACKLIGHT         1
#define RG_SCREEN_WIDTH             240  //1.3 inch 240  2.4 inch 320 
#define RG_SCREEN_HEIGHT            240
#define RG_SCREEN_ROTATE            0
#define RG_SCREEN_MARGIN_TOP        0
#define RG_SCREEN_MARGIN_BOTTOM     0
#define RG_SCREEN_MARGIN_LEFT       0
#define RG_SCREEN_MARGIN_RIGHT      0
#define RG_SCREEN_INIT()                                                                                                        \
    ILI9341_CMD(0x01);                                                                                                          \
    ILI9341_CMD(0x3A, 0x55);                                                                                                    \
    ILI9341_CMD(0x20);                                                                                                          \
    ILI9341_CMD(0xCF, 0x00, 0xc3, 0x30);                                                                                        \
    ILI9341_CMD(0xED, 0x64, 0x03, 0x12, 0x81);                                                                                  \
    ILI9341_CMD(0xE8, 0x85, 0x00, 0x78);                                                                                        \
    ILI9341_CMD(0xCB, 0x39, 0x2c, 0x00, 0x34, 0x02);                                                                            \
    ILI9341_CMD(0xF7, 0x20);                                                                                                    \
    ILI9341_CMD(0xEA, 0x00, 0x00);                                                                                              \
    ILI9341_CMD(0xC0, 0x1B);                                                                                                    \
    ILI9341_CMD(0xC1, 0x12);                                                                                                    \
    ILI9341_CMD(0xC5, 0x32, 0x3C);                                                                                              \
    ILI9341_CMD(0xC7, 0x91);                                                                                                    \
    ILI9341_CMD(0x36, (0xA0| 0X08));                          /* 1.3 inch 0XA0|0X08  (2.4 inch  0x00|0x80) */                   \
    ILI9341_CMD(0xB1, 0x00, 0x10);                                                                                              \
    ILI9341_CMD(0xB6, 0x0A, 0xA2);                                                                                              \
    ILI9341_CMD(0xF6, 0x01, 0x30);                                                                                              \
    ILI9341_CMD(0xF2, 0x00);                                                                                                    \
    ILI9341_CMD(0x26, 0x01);                                                                                                    \
    ILI9341_CMD(0xE0, 0xD0, 0x00, 0x02, 0x07, 0x0a, 0x28, 0x32, 0x44, 0x42, 0x06, 0x0e, 0x12, 0x14, 0x17);                      \
    ILI9341_CMD(0xE1, 0xD0, 0x00, 0x02, 0x07, 0x0a, 0x28, 0x31, 0x54, 0x47, 0x0E, 0x1C, 0x17, 0x1b, 0x1e);                      \
    ILI9341_CMD(0x11);                                                                                                          \
    ILI9341_CMD(0x29);     

// Input
// Refer to rg_input.h to see all available RG_KEY_* and RG_GAMEPAD_*_MAP types
#define RG_GAMEPAD_GPIO_MAP {\
    {RG_KEY_UP,     GPIO_NUM_7,  GPIO_PULLUP_ONLY, 0},\
    {RG_KEY_RIGHT,  GPIO_NUM_6,  GPIO_PULLUP_ONLY, 0},\
    {RG_KEY_DOWN,   GPIO_NUM_46, GPIO_PULLUP_ONLY, 0},\
    {RG_KEY_LEFT,   GPIO_NUM_45, GPIO_PULLUP_ONLY, 0},\
    {RG_KEY_SELECT, GPIO_NUM_16, GPIO_PULLUP_ONLY, 0},\
    {RG_KEY_START,  GPIO_NUM_17, GPIO_PULLUP_ONLY, 0},\
    {RG_KEY_MENU,   GPIO_NUM_18, GPIO_PULLUP_ONLY, 0},\
    {RG_KEY_OPTION, GPIO_NUM_8,  GPIO_PULLUP_ONLY, 0},\
    {RG_KEY_A,      GPIO_NUM_15, GPIO_PULLUP_ONLY, 0},\
    {RG_KEY_B,      GPIO_NUM_5,  GPIO_PULLUP_ONLY, 0},\
}

// Battery
#define RG_BATTERY_DRIVER           1
#define RG_BATTERY_ADC_UNIT         ADC_UNIT_1
#define RG_BATTERY_ADC_CHANNEL      ADC_CHANNEL_3
#define RG_BATTERY_CALC_PERCENT(raw) (((raw) * 2.f - 3500.f) / (4200.f - 3500.f) * 100.f)
#define RG_BATTERY_CALC_VOLTAGE(raw) ((raw) * 2.f * 0.001f)

// Status LED
#define RG_GPIO_LED                 GPIO_NUM_38

// SPI Display (back up working)
#define RG_GPIO_LCD_MISO            GPIO_NUM_NC
#define RG_GPIO_LCD_MOSI            GPIO_NUM_12
#define RG_GPIO_LCD_CLK             GPIO_NUM_48
#define RG_GPIO_LCD_CS              GPIO_NUM_14
#define RG_GPIO_LCD_DC              GPIO_NUM_47
#define RG_GPIO_LCD_BCKL            GPIO_NUM_39
#define RG_GPIO_LCD_RST             GPIO_NUM_3

#define RG_GPIO_SDSPI_MISO          GPIO_NUM_9
#define RG_GPIO_SDSPI_MOSI          GPIO_NUM_11
#define RG_GPIO_SDSPI_CLK           GPIO_NUM_13
#define RG_GPIO_SDSPI_CS            GPIO_NUM_10

// External I2S DAC
#define RG_GPIO_SND_I2S_BCK         41
#define RG_GPIO_SND_I2S_WS          42
#define RG_GPIO_SND_I2S_DATA        40
// #define RG_GPIO_SND_AMP_ENABLE      18
