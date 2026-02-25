// Target definition
#define RG_TARGET_NAME             "BYTEBOI-REV2"

// Storage
#define RG_STORAGE_ROOT             "/sd"
#define RG_STORAGE_SDSPI_HOST       SPI3_HOST
#define RG_STORAGE_SDSPI_SPEED      SDMMC_FREQ_DEFAULT
// #define RG_STORAGE_SDMMC_HOST       SDMMC_HOST_SLOT_1
// #define RG_STORAGE_SDMMC_SPEED      SDMMC_FREQ_DEFAULT
// #define RG_STORAGE_FLASH_PARTITION  "vfs"

// Audio
#define RG_AUDIO_USE_INT_DAC        1   // 0 = Disable, 1 = GPIO25, 2 = GPIO26, 3 = Both
#define RG_AUDIO_USE_EXT_DAC        0   // 0 = Disable, 1 = Enable

// Video
#define RG_SCREEN_DRIVER            0   // 0 = ILI9341/ST7789
#define RG_SCREEN_HOST              SPI2_HOST
#define RG_SCREEN_SPEED             SPI_MASTER_FREQ_40M
#define RG_SCREEN_SPI_MODE          3
#define RG_SCREEN_BACKLIGHT         1
#define RG_SCREEN_WIDTH             320
#define RG_SCREEN_HEIGHT            240
#define RG_SCREEN_ROTATE            0
#define RG_SCREEN_VISIBLE_AREA      {0, 0, 0, 0}
#define RG_SCREEN_SAFE_AREA         {0, 0, 0, 0}
#define RG_SCREEN_INIT()                                                                                         \
    ILI9341_CMD(0x36, 0xA0);                 /* Memory Access Control (MY|MV|RGB) */                             \
    ILI9341_CMD(0xB1, 0x00, 0x10);                                                                               \
    ILI9341_CMD(0xB2, 0x0c, 0x0c, 0x00, 0x33, 0x33);                                                             \
    ILI9341_CMD(0xB7, 0x35);                                                                                     \
    ILI9341_CMD(0xBB, 0x24);                                                                                     \
    ILI9341_CMD(0xC0, 0x2C);                                                                                     \
    ILI9341_CMD(0xC2, 0x01, 0xFF);                                                                               \
    ILI9341_CMD(0xC3, 0x11);                                                                                     \
    ILI9341_CMD(0xC4, 0x20);                                                                                     \
    ILI9341_CMD(0xC6, 0x0f);                                                                                     \
    ILI9341_CMD(0xD0, 0xA4, 0xA1);                                                                               \
    ILI9341_CMD(0xE0, 0xD0, 0x00, 0x03, 0x09, 0x13, 0x1C, 0x3A, 0x55, 0x48, 0x18, 0x12, 0x0E, 0x19, 0x1E);       \
    ILI9341_CMD(0xE1, 0xD0, 0x00, 0x03, 0x09, 0x05, 0x25, 0x3A, 0x55, 0x50, 0x3D, 0x1C, 0x1D, 0x1D, 0x1E);

// Input
// Refer to rg_input.h to see all available RG_KEY_* and RG_GAMEPAD_*_MAP types
#define RG_GAMEPAD_SERIAL_MAP {\
    {RG_KEY_UP,     .num = 15, .level = 0},\
    {RG_KEY_RIGHT,  .num = 14, .level = 0},\
    {RG_KEY_DOWN,   .num = 12, .level = 0},\
    {RG_KEY_LEFT,   .num = 13, .level = 0},\
    {RG_KEY_SELECT, .num = 9,  .level = 0},\
    {RG_KEY_A,      .num = 11, .level = 0},\
    {RG_KEY_B,      .num = 10, .level = 0},\
}
#define RG_GAMEPAD_VIRT_MAP {\
    {RG_KEY_START,  .src = RG_KEY_SELECT | RG_KEY_A },\
    {RG_KEY_MENU,   .src = RG_KEY_SELECT | RG_KEY_B },\
    {RG_KEY_OPTION, .src = RG_KEY_SELECT | RG_KEY_UP},\
}

// Battery
#define RG_BATTERY_DRIVER           1
#define RG_BATTERY_ADC_UNIT         ADC_UNIT_1
#define RG_BATTERY_ADC_CHANNEL      ADC_CHANNEL_0
#define RG_BATTERY_CALC_PERCENT(raw) (((raw) * 2.f - 3500.f) / (4200.f - 3500.f) * 100.f)
#define RG_BATTERY_CALC_VOLTAGE(raw) ((raw) * 2.f * 0.001f)

// SPI Display
#define RG_GPIO_LCD_MISO            GPIO_NUM_NC
#define RG_GPIO_LCD_MOSI            GPIO_NUM_33
#define RG_GPIO_LCD_CLK             GPIO_NUM_26
#define RG_GPIO_LCD_CS              GPIO_NUM_NC
#define RG_GPIO_LCD_DC              GPIO_NUM_21
#define RG_GPIO_LCD_BCKL            GPIO_NUM_18
#define RG_GPIO_LCD_BCKL_INVERT
#define RG_GPIO_LCD_RST             GPIO_NUM_27

// Serial gamepad (74HC165-like)
#define RG_GPIO_GAMEPAD_CLOCK       GPIO_NUM_15
#define RG_GPIO_GAMEPAD_DATA        GPIO_NUM_4
#define RG_GPIO_GAMEPAD_LATCH       GPIO_NUM_0

// SPI SD Card
#define RG_GPIO_SDSPI_MISO          GPIO_NUM_5
#define RG_GPIO_SDSPI_MOSI          GPIO_NUM_32
#define RG_GPIO_SDSPI_CLK           GPIO_NUM_14
#define RG_GPIO_SDSPI_CS            GPIO_NUM_2

// External I2S DAC
// #define RG_GPIO_SND_I2S_BCK         GPIO_NUM_NC
// #define RG_GPIO_SND_I2S_WS          GPIO_NUM_NC
// #define RG_GPIO_SND_I2S_DATA        GPIO_NUM_NC
#define RG_GPIO_SND_AMP_ENABLE      GPIO_NUM_13
#define RG_GPIO_SND_AMP_ENABLE_INVERT // Uncomment if the mute = HIGH
