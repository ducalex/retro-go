// Target definition
#define RG_TARGET_NAME             "ESPLAY-MICRO"

// Storage
#define RG_STORAGE_ROOT             "/sd"
// #define RG_STORAGE_SDSPI_HOST       SPI2_HOST
// #define RG_STORAGE_SDSPI_SPEED      SDMMC_FREQ_DEFAULT
#define RG_STORAGE_SDMMC_HOST       SDMMC_HOST_SLOT_1
#define RG_STORAGE_SDMMC_SPEED      SDMMC_FREQ_DEFAULT
// #define RG_STORAGE_FLASH_PARTITION  "vfs"

// Audio
#define RG_AUDIO_USE_INT_DAC        0   // 0 = Disable, 1 = GPIO25, 2 = GPIO26, 3 = Both
#define RG_AUDIO_USE_EXT_DAC        1   // 0 = Disable, 1 = Enable

// Video
#define RG_SCREEN_DRIVER            0   // 0 = ILI9341
#define RG_SCREEN_HOST              SPI2_HOST
#define RG_SCREEN_SPEED             SPI_MASTER_FREQ_40M
#define RG_SCREEN_BACKLIGHT         1
#define RG_SCREEN_WIDTH             320
#define RG_SCREEN_HEIGHT            240
#define RG_SCREEN_ROTATE            0
#define RG_SCREEN_MARGIN_TOP        0
#define RG_SCREEN_MARGIN_BOTTOM     0
#define RG_SCREEN_MARGIN_LEFT       0
#define RG_SCREEN_MARGIN_RIGHT      0
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
    ILI9341_CMD(0x36, (0x20 | 0x08)); /* Memory Access Control */                                        \
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
// A and B silkscreen on the board are swapped relative to standard Nintendo layout
// Temporarily unmapped menu and option. Start + Select activates menu.
#define RG_GAMEPAD_I2C_MAP {\
    {RG_KEY_UP,     (1<<2)},\
    {RG_KEY_RIGHT,  (1<<5)},\
    {RG_KEY_DOWN,   (1<<3)},\
    {RG_KEY_LEFT,   (1<<4)},\
    {RG_KEY_SELECT, (1<<1)},\
    {RG_KEY_START,  (1<<8)},\
    {RG_KEY_A,      (1<<7)},\
    {RG_KEY_B,      (1<<6)},\
}
#define RG_GAMEPAD_VIRT_MAP {\
    {RG_KEY_MENU,   RG_KEY_START | RG_KEY_SELECT},\
    {RG_KEY_OPTION, RG_KEY_SELECT | RG_KEY_A},\
}

// Battery
#define RG_BATTERY_DRIVER           2
#define RG_BATTERY_CALC_PERCENT(raw) (((raw) - 170) / 30.f * 100.f)
#define RG_BATTERY_CALC_VOLTAGE(raw) ((raw) * 2.f * 0.001f)

// Status LED
#define RG_GPIO_LED                 GPIO_NUM_13 // Causes the "power" LED to blink on disk access.

// I2C BUS
#define RG_GPIO_I2C_SDA             GPIO_NUM_21
#define RG_GPIO_I2C_SCL             GPIO_NUM_22

// Display
#define RG_GPIO_LCD_MISO            GPIO_NUM_NC
#define RG_GPIO_LCD_MOSI            GPIO_NUM_23
#define RG_GPIO_LCD_CLK             GPIO_NUM_18
#define RG_GPIO_LCD_CS              GPIO_NUM_5
#define RG_GPIO_LCD_DC              GPIO_NUM_12
#define RG_GPIO_LCD_BCKL            GPIO_NUM_27
// #define RG_GPIO_LCD_RST           GPIO_NUM_NC

// External I2S DAC
#define RG_GPIO_SND_I2S_BCK         GPIO_NUM_26
#define RG_GPIO_SND_I2S_WS          GPIO_NUM_25
#define RG_GPIO_SND_I2S_DATA        GPIO_NUM_19
#define RG_GPIO_SND_AMP_ENABLE      GPIO_NUM_4

// Updater
#define RG_UPDATER_ENABLE               1
#define RG_UPDATER_APPLICATION          RG_APP_FACTORY
#define RG_UPDATER_DOWNLOAD_LOCATION    RG_STORAGE_ROOT "/esplay/firmware"
