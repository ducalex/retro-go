// Target definition
#define RG_TARGET_NAME "ESPmulatorS3"

// Storage
#define RG_STORAGE_ROOT             "/sd"
//#define RG_STORAGE_SDSPI_HOST       SPI3_HOST
//#define RG_STORAGE_SDSPI_SPEED      SDMMC_FREQ_DEFAULT
#define RG_STORAGE_SDMMC_HOST       SDMMC_HOST_SLOT_1
#define RG_STORAGE_SDMMC_SPEED      SDMMC_FREQ_DEFAULT
// #define RG_STORAGE_FLASH_PARTITION  "vfs"

// SDMMC Pins für ESP32-S3
#define RG_GPIO_SDMMC_CLK    GPIO_NUM_39  // part of development board
#define RG_GPIO_SDMMC_CMD    GPIO_NUM_38  // part of development board
#define RG_GPIO_SDMMC_D0     GPIO_NUM_40  // part of development board

// Audio
#define RG_AUDIO_USE_INT_DAC        0   // 0 = Disable, 1 = GPIO25, 2 = GPIO26, 3 = Both
#define RG_AUDIO_USE_EXT_DAC        1   // 0 = Disable, 1 = Enable

// Video
#define RG_SCREEN_DRIVER            0   // 0 = ILI9341
#define RG_SCREEN_HOST              SPI2_HOST
#define RG_SCREEN_SPEED             SPI_MASTER_FREQ_40M // SPI_MASTER_FREQ_80M
#define RG_SCREEN_BACKLIGHT         1
#define RG_SCREEN_WIDTH             320
#define RG_SCREEN_HEIGHT            240
#define RG_SCREEN_ROTATE            0
#define RG_SCREEN_VISIBLE_AREA      {0, 0, 0, 0} // Left, Top, Right, Bottom
#define RG_SCREEN_SAFE_AREA         {0, 0, 0, 0} // Left, Top, Right, Bottom
#define RG_SCREEN_INIT()                                                                                 \
    ILI9341_CMD(0xCF, 0x00, 0x83, 0X30);                                                                 \
    ILI9341_CMD(0xED, 0x64, 0x03, 0X12, 0X81);                                                           \
    ILI9341_CMD(0xE8, 0x85, 0x01, 0x79);                                                                 \
    ILI9341_CMD(0xCB, 0x39, 0x2C, 0x00, 0x34, 0x02);                                                     \
    ILI9341_CMD(0xF7, 0x20);                                                                             \
    ILI9341_CMD(0xEA, 0x00, 0x00);                                                                       \
    ILI9341_CMD(0xC0, 0x26);          /* Power control */                                                \
    ILI9341_CMD(0xC1, 0x11);          /* Power control */                                                \
    ILI9341_CMD(0xC5, 0x35, 0x3E);    /* VCM control */                                                  \
    ILI9341_CMD(0x36, 0xE8);          /* MX | MY | MV | BGR = 180° rotation */                                \
    ILI9341_CMD(0x3A, 0x55);          /* Pixel Format Set RGB565 */                                      \
    ILI9341_CMD(0xB1, 0x00, 0x1B);    /* Frame Rate Control (1B=70, 1F=61, 10=119) */                    \
    ILI9341_CMD(0xB6, 0x0A, 0xA2);    /* Display Function Control */                                     \
    ILI9341_CMD(0xF6, 0x01, 0x30);                                                                       \
    ILI9341_CMD(0xF2, 0x00);                                                                             \
    ILI9341_CMD(0x26, 0x01);                                                                             \
    ILI9341_CMD(0xE0, 0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0X87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00); \
    ILI9341_CMD(0xE1, 0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F); \
    ILI9341_CMD(0x2C, 0x00);                                                                             \
    ILI9341_CMD(0xB7, 0x07);                                                                             \
    ILI9341_CMD(0xB6, 0x0A, 0x82, 0x27, 0x00);                                                           \
    ILI9341_CMD(0x11, 0x80);                                                                             \
    ILI9341_CMD(0x29, 0x80);

// Input
// Refer to rg_input.h to see all available RG_KEY_* and RG_GAMEPAD_*_MAP types
#define RG_GAMEPAD_ADC_MAP {\
    {RG_KEY_LEFT,  ADC_UNIT_1, ADC_CHANNEL_9, ADC_ATTEN_DB_11, 0,    1024},\
    {RG_KEY_RIGHT, ADC_UNIT_1, ADC_CHANNEL_9, ADC_ATTEN_DB_11, 3072, 4096},\
    {RG_KEY_UP,    ADC_UNIT_1, ADC_CHANNEL_8, ADC_ATTEN_DB_11, 0,    1024},\
    {RG_KEY_DOWN,  ADC_UNIT_1, ADC_CHANNEL_8, ADC_ATTEN_DB_11, 3072, 4096},\
}

// Buttons (intern Pullups)
#define RG_GAMEPAD_GPIO_MAP {\
    {RG_KEY_A,      .num = GPIO_NUM_13, .pullup = 1, .level = 0},\
    {RG_KEY_B,      .num = GPIO_NUM_14, .pullup = 1, .level = 0},\
    {RG_KEY_SELECT, .num = GPIO_NUM_18, .pullup = 1, .level = 0},\
    {RG_KEY_START,  .num = GPIO_NUM_17, .pullup = 1, .level = 0},\
    {RG_KEY_MENU,   .num = GPIO_NUM_11, .pullup = 1, .level = 0},\
}

//{RG_KEY_OPTION, .num = GPIO_NUM_0, .pullup = 1, .level = 0},

#define RG_GAMEPAD_VIRT_MAP {\
    {RG_KEY_OPTION, .src = RG_KEY_SELECT | RG_KEY_START },\
    {RG_KEY_X     , .src = RG_KEY_SELECT},\
}

// Battery
#define RG_BATTERY_DRIVER      1
#define RG_BATTERY_ADC_UNIT    ADC_UNIT_1
#define RG_BATTERY_ADC_CHANNEL ADC_CHANNEL_7   // GPIO8
#define RG_BATTERY_CALC_PERCENT(raw) (((raw) * 4.2f - 3500.f) / (4000.f - 3500.f) * 100.f)
//300 kOhm and 100 kOhm resistors; 3.5 V for savety; 3.8 V for better linarity (but longer 100 %)
#define RG_BATTERY_CALC_VOLTAGE(raw) ((raw) * 4.2f * 0.001f)

// Status LED
//#define RG_GPIO_LED GPIO_NUM_2
#define RG_GPIO_LED_DIMMING 1
//#define RG_GPIO_LED_INVERT
#define RG_GPIO_WS2812 48

// SPI Display
#define RG_GPIO_LCD_MISO GPIO_NUM_NC
#define RG_GPIO_LCD_MOSI GPIO_NUM_7
#define RG_GPIO_LCD_CLK  GPIO_NUM_15
#define RG_GPIO_LCD_CS   GPIO_NUM_4
#define RG_GPIO_LCD_DC   GPIO_NUM_6
#define RG_GPIO_LCD_RST  GPIO_NUM_5
#define RG_GPIO_LCD_BCKL GPIO_NUM_16
//#define RG_GPIO_LCD_BCKL_INVERT

// External I2S DAC
#define RG_GPIO_SND_I2S_BCK  GPIO_NUM_41
#define RG_GPIO_SND_I2S_WS   GPIO_NUM_1
#define RG_GPIO_SND_I2S_DATA GPIO_NUM_42
// #define RG_GPIO_SND_AMP_ENABLE      18

// Updater
#define RG_UPDATER_ENABLE               1
#define RG_UPDATER_APPLICATION          RG_APP_FACTORY
#define RG_UPDATER_DOWNLOAD_LOCATION    RG_STORAGE_ROOT "/espmulators3/firmware"
