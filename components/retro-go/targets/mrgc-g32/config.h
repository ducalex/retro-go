// Target definition
#define RG_TARGET_NAME             "MRGC-G32"

// Storage
#define RG_STORAGE_ROOT             "/sd"
// #define RG_STORAGE_SDSPI_HOST       SPI2_HOST
// #define RG_STORAGE_SDSPI_SPEED      SDMMC_FREQ_DEFAULT
#define RG_STORAGE_SDMMC_HOST       SDMMC_HOST_SLOT_1
#define RG_STORAGE_SDMMC_SPEED      SDMMC_FREQ_DEFAULT
// #define RG_STORAGE_FLASH_PARTITION  "vfs"

// GPIO Extender
// #define RG_I2C_GPIO_DRIVER          0   // 1 = AW9523, 2 = PCF9539, 3 = MCP23017
#define RG_I2C_GPIO_ADDR            0x20

// Audio
#define RG_AUDIO_USE_INT_DAC        0   // 0 = Disable, 1 = GPIO25, 2 = GPIO26, 3 = Both
#define RG_AUDIO_USE_EXT_DAC        1   // 0 = Disable, 1 = Enable

// Video
#define RG_SCREEN_DRIVER            0   // 0 = ILI9341/ST7789
#define RG_SCREEN_HOST              SPI2_HOST
#define RG_SCREEN_SPEED             SPI_MASTER_FREQ_40M
#define RG_SCREEN_BACKLIGHT         1
#define RG_SCREEN_WIDTH             240
#define RG_SCREEN_HEIGHT            320
#define RG_SCREEN_ROTATE            0
#define RG_SCREEN_VISIBLE_AREA      {0, 28, 0, 68}
#define RG_SCREEN_SAFE_AREA         {0, 0, 0, 0}
#define RG_SCREEN_INIT()                                                                                   \
    ILI9341_CMD(0x36, 0x00);                                                                               \
    ILI9341_CMD(0xB1, 0x00, 0x10);                                                                         \
    ILI9341_CMD(0xB2, 0x0c, 0x0c, 0x00, 0x33, 0x33);                                                       \
    ILI9341_CMD(0xB7, 0x35);                                                                               \
    ILI9341_CMD(0xBB, 0x24);                                                                               \
    ILI9341_CMD(0xC0, 0x2C);                                                                               \
    ILI9341_CMD(0xC2, 0x01, 0xFF);                                                                         \
    ILI9341_CMD(0xC3, 0x11);                                                                               \
    ILI9341_CMD(0xC4, 0x20);                                                                               \
    ILI9341_CMD(0xC6, 0x0f);                                                                               \
    ILI9341_CMD(0xD0, 0xA4, 0xA1);                                                                         \
    ILI9341_CMD(0xE0, 0xD0, 0x00, 0x03, 0x09, 0x13, 0x1C, 0x3A, 0x55, 0x48, 0x18, 0x12, 0x0E, 0x19, 0x1E); \
    ILI9341_CMD(0xE1, 0xD0, 0x00, 0x03, 0x09, 0x05, 0x25, 0x3A, 0x55, 0x50, 0x3D, 0x1C, 0x1D, 0x1D, 0x1E);
#define RG_SCREEN_DEINIT() \
    lcd_set_backlight(0); \
    ILI9341_CMD(0x01);

// Input
// Refer to rg_input.h to see all available RG_KEY_* and RG_GAMEPAD_*_MAP types
#define RG_GAMEPAD_I2C_MAP {\
    {RG_KEY_UP,     .num = 2, .level = 0},\
    {RG_KEY_RIGHT,  .num = 5, .level = 0},\
    {RG_KEY_DOWN,   .num = 3, .level = 0},\
    {RG_KEY_LEFT,   .num = 4, .level = 0},\
    {RG_KEY_SELECT, .num = 1, .level = 0},\
    {RG_KEY_START,  .num = 0, .level = 0},\
    {RG_KEY_MENU,   .num = 8, .level = 0},\
    {RG_KEY_A,      .num = 6, .level = 0},\
    {RG_KEY_B,      .num = 7, .level = 0},\
}
#define RG_GAMEPAD_VIRT_MAP {\
    {RG_KEY_OPTION, .src = RG_KEY_SELECT | RG_KEY_A},\
}

// Battery
#define RG_BATTERY_DRIVER           2
#define RG_BATTERY_CALC_PERCENT(raw) (((raw) - 170) / 30.f * 100.f)
#define RG_BATTERY_CALC_VOLTAGE(raw) (3 * (raw) * 3.3f / 256)


// Status LED
// #define RG_GPIO_LED                 GPIO_NUM_NC

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
#define RG_UPDATER_DOWNLOAD_LOCATION    RG_STORAGE_ROOT "/espgbc/firmware"
