// Target definition
#define RG_TARGET_NAME             "NM-CYD-C5"

// Storage
#define RG_STORAGE_ROOT             "/sd"
#define RG_STORAGE_SDSPI_HOST       SPI2_HOST
#define RG_STORAGE_SDSPI_SPEED      SDMMC_FREQ_DEFAULT

// Audio
#define RG_AUDIO_USE_INT_DAC        0   // ESP32-C5 has no ESP32-style internal DAC
#define RG_AUDIO_USE_EXT_DAC        0   // No on-board I2S DAC is documented for NM-CYD-C5

// Board-specific
#define NM_CYD_C5_TOUCH_CS          GPIO_NUM_1
#define NM_CYD_C5_RGB_LED           GPIO_NUM_27
#define RG_CUSTOM_PLATFORM_INIT()                             \
    gpio_set_direction(NM_CYD_C5_TOUCH_CS, GPIO_MODE_OUTPUT); \
    gpio_set_level(NM_CYD_C5_TOUCH_CS, 1);                    \
    gpio_set_pull_mode(RG_GPIO_SDSPI_MISO, GPIO_PULLUP_ONLY);

// Video
#define RG_SCREEN_DRIVER            0   // 0 = ILI9341/ST7789
#define RG_SCREEN_HOST              SPI2_HOST
#define RG_SCREEN_SPEED             SPI_MASTER_FREQ_40M
#define RG_SCREEN_BACKLIGHT         1
#define RG_SCREEN_WIDTH             320
#define RG_SCREEN_HEIGHT            240
#define RG_SCREEN_ROTATE            0
#define RG_SCREEN_VISIBLE_AREA      {0, 0, 0, 0}
#define RG_SCREEN_SAFE_AREA         {0, 0, 0, 0}
#define RG_SCREEN_INIT()                                                                                   \
    ILI9341_CMD(0x36, 0x60);                                                                               \
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
    lcd_set_backlight(0);  \
    ILI9341_CMD(0x01);

// Input
#if RG_ENABLE_TOUCH_GAMEPAD
#define RG_TOUCH_XPT2046_HOST         SPI2_HOST
#define RG_TOUCH_XPT2046_CS           NM_CYD_C5_TOUCH_CS
#define RG_TOUCH_XPT2046_SPEED        (2500 * 1000)
#define RG_TOUCH_XPT2046_X_MIN        250
#define RG_TOUCH_XPT2046_X_MAX        3800
#define RG_TOUCH_XPT2046_Y_MIN        250
#define RG_TOUCH_XPT2046_Y_MAX        3800
#define RG_TOUCH_XPT2046_SWAP_XY
#define RG_TOUCH_XPT2046_PRESSURE_MIN 80

// Touch zones are expressed as screen coordinates after calibration.
// Left side: d-pad. Right side: A/B plus menu/select/start shortcuts.
#define RG_GAMEPAD_TOUCH_MAP {\
    {RG_KEY_UP,     0,   0, 120,  70},\
    {RG_KEY_LEFT,   0,  50,  70, 170},\
    {RG_KEY_RIGHT, 70,  50, 140, 170},\
    {RG_KEY_DOWN,   0, 170, 120, 239},\
    {RG_KEY_B,    170,  90, 239, 179},\
    {RG_KEY_A,    240,  90, 319, 179},\
    {RG_KEY_MENU, 170,   0, 319,  59},\
    {RG_KEY_SELECT,140, 180, 229, 239},\
    {RG_KEY_START,230, 180, 319, 239},\
}
#endif

// Status LED
#define RG_GPIO_LED                 NM_CYD_C5_RGB_LED

// SPI Display / Touch / SD Card shared bus
#define RG_GPIO_LCD_MISO            GPIO_NUM_2
#define RG_GPIO_LCD_MOSI            GPIO_NUM_7
#define RG_GPIO_LCD_CLK             GPIO_NUM_6
#define RG_GPIO_LCD_CS              GPIO_NUM_23
#define RG_GPIO_LCD_DC              GPIO_NUM_24
#define RG_GPIO_LCD_BCKL            GPIO_NUM_25
// #define RG_GPIO_LCD_RST           GPIO_NUM_NC

#define RG_GPIO_SDSPI_MISO          RG_GPIO_LCD_MISO
#define RG_GPIO_SDSPI_MOSI          RG_GPIO_LCD_MOSI
#define RG_GPIO_SDSPI_CLK           RG_GPIO_LCD_CLK
#define RG_GPIO_SDSPI_CS            GPIO_NUM_10

// Updater
#define RG_UPDATER_ENABLE           0
