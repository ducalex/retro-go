/* Configuration for Retro Ruler second revision, based on the ESP32-S3.
 *
 * Hardware info: https://github.com/rapha-tech/Retro-Ruler
 *
 */

// Target definition
#define RG_TARGET_NAME             "RETRO-RULER"

// Storage
#define RG_STORAGE_ROOT             "/sd"
#define RG_STORAGE_SDSPI_HOST       SPI2_HOST
#define RG_STORAGE_SDSPI_SPEED      SDMMC_FREQ_DEFAULT

// Audio
#define RG_AUDIO_USE_INT_DAC        0   // 0 = Disable, 1 = GPIO25, 2 = GPIO26, 3 = Both
#define RG_AUDIO_USE_EXT_DAC        1   // 0 = Disable, 1 = Enable

// Video
#define RG_SCREEN_DRIVER            0   // 0 = ILI9341/ST7789
#define RG_SCREEN_HOST              SPI2_HOST
#define RG_SCREEN_SPEED             SPI_MASTER_FREQ_80M
#define RG_SCREEN_BACKLIGHT         0
#define RG_SCREEN_WIDTH             320
#define RG_SCREEN_HEIGHT            240
#define RG_SCREEN_ROTATE            0
#define RG_SCREEN_VISIBLE_AREA      {20, 0, 20, 0}  // Left, Top, Right, Bottom
#define RG_SCREEN_SAFE_AREA         {20, 0, 20, 0}  // Left, Top, Right, Bottom
#define RG_SCREEN_INIT()                                                                                     \
ILI9341_CMD(0xCF, 0x00, 0xc3, 0x30);                                                                         \
ILI9341_CMD(0xED, 0x64, 0x03, 0x12, 0x81);                                                                   \
ILI9341_CMD(0xE8, 0x85, 0x00, 0x78);                                                                         \
ILI9341_CMD(0xCB, 0x39, 0x2c, 0x00, 0x34, 0x02);                                                             \
ILI9341_CMD(0xF7, 0x20);                                                                                     \
ILI9341_CMD(0xEA, 0x00, 0x00);                                                                               \
ILI9341_CMD(0xC0, 0x1B);                 /* Power control   //VRH[5:0] */                                    \
ILI9341_CMD(0xC1, 0x12);                 /* Power control   //SAP[2:0];BT[3:0] */                            \
ILI9341_CMD(0xC5, 0x32, 0x3C);           /* VCM control */                                                   \
ILI9341_CMD(0xC7, 0x91);                 /* VCM control2 */                                                  \
ILI9341_CMD(0x36, (0x40 | 0x80 | 0x08)); /* Memory Access Control */                                         \
ILI9341_CMD(0xB1, 0x00, 0x10);           /* Frame Rate Control (1B=70, 1F=61, 10=119) */                     \
ILI9341_CMD(0xB6, 0x0A, 0xA2);           /* Display Function Control */                                      \
ILI9341_CMD(0xF6, 0x01, 0x30);                                                                               \
ILI9341_CMD(0xF2, 0x00); /* 3Gamma Function Disable */                                                       \
ILI9341_CMD(0x26, 0x01); /* Gamma curve selected */                                                          \
ILI9341_CMD(0xE0, 0xD0, 0x00, 0x02, 0x07, 0x0a, 0x28, 0x32, 0x44, 0x42, 0x06, 0x0e, 0x12, 0x14, 0x17);       \
ILI9341_CMD(0xE1, 0xD0, 0x00, 0x02, 0x07, 0x0a, 0x28, 0x31, 0x54, 0x47, 0x0E, 0x1C, 0x17, 0x1b, 0x1e);       

#define RG_RECOVERY_BTN RG_KEY_MENU // Keep this button pressed to open the recovery menu

#define RG_GAMEPAD_GPIO_MAP {\
    {RG_KEY_UP,     .num = GPIO_NUM_18, .pullup = 1, .level = 0},\
    {RG_KEY_DOWN,   .num = GPIO_NUM_16, .pullup = 1, .level = 0},\
    {RG_KEY_RIGHT,  .num = GPIO_NUM_17, .pullup = 1, .level = 0},\
    {RG_KEY_LEFT,   .num = GPIO_NUM_8,  .pullup = 1, .level = 0},\
    {RG_KEY_SELECT, .num = GPIO_NUM_42, .pullup = 1, .level = 0},\
    {RG_KEY_START,  .num = GPIO_NUM_1,  .pullup = 1, .level = 0},\
    {RG_KEY_MENU,   .num = GPIO_NUM_0,  .pullup = 1, .level = 0},\
    {RG_KEY_A,      .num = GPIO_NUM_5,  .pullup = 1, .level = 0},\
    {RG_KEY_B,      .num = GPIO_NUM_6,  .pullup = 1, .level = 0},\
    {RG_KEY_X,      .num = GPIO_NUM_7,  .pullup = 1, .level = 0},\
    {RG_KEY_Y,      .num = GPIO_NUM_15, .pullup = 1, .level = 0},\
    {RG_KEY_L,      .num = GPIO_NUM_48, .pullup = 1, .level = 0},\
    {RG_KEY_R,      .num = GPIO_NUM_2,  .pullup = 1, .level = 0},\
}

// Battery
#define RG_BATTERY_DRIVER           1
#define RG_BATTERY_ADC_UNIT         ADC_UNIT_1
#define RG_BATTERY_ADC_CHANNEL      ADC_CHANNEL_2
#define RG_BATTERY_CALC_PERCENT(raw) (((raw) * 2.f - 3500.f) / (4200.f - 3500.f) * 100.f)
#define RG_BATTERY_CALC_VOLTAGE(raw) ((raw) * 2.f * 0.001f)

// SPI Display
#define RG_GPIO_LCD_MISO            GPIO_NUM_13
#define RG_GPIO_LCD_MOSI            GPIO_NUM_11
#define RG_GPIO_LCD_CLK             GPIO_NUM_12
#define RG_GPIO_LCD_CS              GPIO_NUM_10
#define RG_GPIO_LCD_DC              GPIO_NUM_9
#define RG_GPIO_LCD_RST             GPIO_NUM_NC

// SPI SD Card
#define RG_GPIO_SDSPI_MISO          RG_GPIO_LCD_MISO
#define RG_GPIO_SDSPI_MOSI          RG_GPIO_LCD_MOSI
#define RG_GPIO_SDSPI_CLK           RG_GPIO_LCD_CLK
#define RG_GPIO_SDSPI_CS            GPIO_NUM_21

// External I2S DAC
#define RG_GPIO_SND_I2S_BCK         GPIO_NUM_39
#define RG_GPIO_SND_I2S_WS          GPIO_NUM_40
#define RG_GPIO_SND_I2S_DATA        GPIO_NUM_38
#define RG_GPIO_SND_AMP_ENABLE      GPIO_NUM_41

// Updater
#define RG_UPDATER_ENABLE               1
#define RG_UPDATER_APPLICATION          RG_APP_FACTORY
#define RG_UPDATER_DOWNLOAD_LOCATION    RG_STORAGE_ROOT "/retro-ruler/firmware"