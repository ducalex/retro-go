// Target definition
#define RG_TARGET_NAME             "ESPLAY-S3"

// Storage
#define RG_STORAGE_ROOT             "/sd"
// #define RG_STORAGE_SDSPI_HOST       SPI2_HOST
// #define RG_STORAGE_SDSPI_SPEED      SDMMC_FREQ_DEFAULT
#define RG_STORAGE_SDMMC_HOST       SDMMC_HOST_SLOT_1
#define RG_STORAGE_SDMMC_SPEED      SDMMC_FREQ_DEFAULT

// #define LCD_320 0
// #define RG_STORAGE_FLASH_PARTITION  "vfs"

// GPIO Extender
// #define RG_I2C_GPIO_DRIVER          0   // 1 = AW9523, 2 = PCF9539, 3 = MCP23017
// #define RG_I2C_GPIO_ADDR            0x20

// Audio
#define RG_AUDIO_USE_INT_DAC        0   // 0 = Disable, 1 = GPIO25, 2 = GPIO26, 3 = Both
#define RG_AUDIO_USE_EXT_DAC        1   // 0 = Disable, 1 = Enable

// Video
#define RG_SCREEN_DRIVER            0   // 0 = ILI9341/ST7789
#define RG_SCREEN_HOST              SPI2_HOST
#if defined(LCD_320)
#define RG_SCREEN_SPEED             SPI_MASTER_FREQ_40M
#else
#define RG_SCREEN_SPEED             SPI_MASTER_FREQ_80M
#endif
#define RG_SCREEN_BACKLIGHT         1

#if defined(LCD_320)
#define RG_SCREEN_WIDTH             320
#define RG_SCREEN_HEIGHT            320
#else
#define RG_SCREEN_WIDTH             240
#define RG_SCREEN_HEIGHT            240
#endif

#define RG_SCREEN_ROTATE            0
#define RG_SCREEN_VISIBLE_AREA      {0, 0, 0, 0}
#define RG_SCREEN_SAFE_AREA         {0, 0, 0, 0}


 //ILI9341_CMD(0xE0, {0xD0, 0x00, 0x05, 0x0E, 0x15, 0x0D, 0x37, 0x43, 0x47, 0x09, 0x15, 0x12, 0x16, 0x19}); 
 //ILI9341_CMD(0xE1, {0xD0, 0x00, 0x05, 0x0D, 0x0C, 0x06, 0x2D, 0x44, 0x40, 0x0E, 0x1C, 0x18, 0x16, 0x19}); 
#if defined(LCD_320)   
#define RG_SCREEN_INIT()\
    ILI9341_CMD(0x11);\
    ILI9341_CMD(0xF0, {0xC3});\
    ILI9341_CMD(0xF0, {0x96});\
    ILI9341_CMD(0xB4, {0x01});\
    ILI9341_CMD(0xB7, {0xC6});\
    ILI9341_CMD(0xB9, {0x02});\
    ILI9341_CMD(0xC0, {0xF0, 0x65});\
    ILI9341_CMD(0xC1, {0x15});\
    ILI9341_CMD(0xC2, {0xAF});\
    ILI9341_CMD(0xC5, {0x22});\
    ILI9341_CMD(0xE8, {0x40, 0x8A, 0x00, 0x00, 0x29, 0x19, 0xA5, 0x33});\
    ILI9341_CMD(0xE0, {0xD0, 0x04, 0x08, 0x09, 0x08, 0x15, 0x2F, 0x42, 0x46, 0x28, 0x15, 0x16, 0x29, 0x2D});\
    ILI9341_CMD(0xE1, {0xD0, 0x04, 0x09, 0x09, 0x08, 0x15, 0x2E, 0x46, 0x46, 0x28, 0x15, 0x15, 0x29, 0x2D});\
    ILI9341_CMD(0xF0, {0x3C});\
    ILI9341_CMD(0xF0, {0x69});\
    ILI9341_CMD(0x29);\
    ILI9341_CMD(0x21);\
    ILI9341_CMD(0x3A, {0x55});\
    ILI9341_CMD(0x36, {0x48});
#else
#define RG_SCREEN_INIT()\
    ILI9341_CMD(0x3A, {0x05}); \
    ILI9341_CMD(0xB2, {0x1F, 0x1F, 0x00, 0x33, 0x33});\
    ILI9341_CMD(0xB7, {0x35});\
    ILI9341_CMD(0xBB, {0x20});\
    ILI9341_CMD(0xC0, {0x2C});\
    ILI9341_CMD(0xC2, {0x01});\
    ILI9341_CMD(0xC3, {0x01});\
    ILI9341_CMD(0xC4, {0x18});\
    ILI9341_CMD(0xC6, {0x13});\
    ILI9341_CMD(0xD0, {0xA4, 0xA1});\
    ILI9341_CMD(0xD6, {0xA1});\
    ILI9341_CMD(0xE0, {0xF0, 0x04, 0x07, 0x04, 0x04, 0x04, 0x25, 0x33, 0x3C, 0x36, 0x14, 0x12, 0x29, 0x30});\
    ILI9341_CMD(0xE1, {0xF0, 0x02, 0x04, 0x05, 0x05, 0x21, 0x25, 0x32, 0x3B, 0x38, 0x12, 0x14, 0x27, 0x31});\
    ILI9341_CMD(0xE4, {0x1D, 0x00, 0x00});\
    ILI9341_CMD(0x21, {0x80});\
    ILI9341_CMD(0x29, {0x80});\
    ILI9341_CMD(0x2C, {0x80});
#endif

/*
ILI9341_CMD(0x3A, {0x55}); \
    ILI9341_CMD(0x0c, {0x0c, 0x00, 0x33, 0x33});\
    ILI9341_CMD(0xB7, {0x72});\
    ILI9341_CMD(0xBB, {0x3d});\
    ILI9341_CMD(0xC0, {0x2C});\
    ILI9341_CMD(0xC2, {0x01, 0xFF});\
    ILI9341_CMD(0xC3, {0x19});\
    ILI9341_CMD(0xC4, {0x20});\
    ILI9341_CMD(0xC6, {0x0f});\
    ILI9341_CMD(0xD0, {0xA4, 0xA1});\
    ILI9341_CMD(0xE0, {0xD2, 0x01, 0x08, 0x14, 0x1E, 0x12, 0x3A, 0x48, 0x4C, 0x0B, 0x18, 0x15, 0x1A, 0x1F});\
    ILI9341_CMD(0xE1, {0xD2, 0x01, 0x08, 0x13, 0x1C, 0x09, 0x33, 0x4A, 0x46, 0x10, 0x1F, 0x1B, 0x1A, 0x1F});\
    ILI9341_CMD(0x21, {0x80});\
    ILI9341_CMD(0x11, {0x80});\
    ILI9341_CMD(0x29, {0x80});
*/

// Input
// Refer to rg_input.h to see all available RG_KEY_* and RG_GAMEPAD_*_MAP types
/*
#define RG_GAMEPAD_I2C_MAP {\
    {RG_KEY_UP,     .num = 2, .level = 0},\
    {RG_KEY_RIGHT,  .num = 5, .level = 0},\
    {RG_KEY_DOWN,   .num = 3, .level = 0},\
    {RG_KEY_LEFT,   .num = 4, .level = 0},\
    {RG_KEY_SELECT, .num = 1, .level = 0},\
    {RG_KEY_START,  .num = 0, .level = 0},\
    {RG_KEY_A,      .num = 6, .level = 0},\
    {RG_KEY_B,      .num = 7, .level = 0},\
}


#define RG_GAMEPAD_ADC_MAP {\
    {RG_KEY_UP,    ADC_UNIT_1, ADC_CHANNEL_0, ADC_ATTEN_DB_11, 3072, 4096},\
    {RG_KEY_RIGHT, ADC_UNIT_1, ADC_CHANNEL_1, ADC_ATTEN_DB_11, 1024, 3071},\
    {RG_KEY_DOWN,  ADC_UNIT_1, ADC_CHANNEL_0, ADC_ATTEN_DB_11, 1024, 3071},\
    {RG_KEY_LEFT,  ADC_UNIT_1, ADC_CHANNEL_1, ADC_ATTEN_DB_11, 3072, 4096},\
}
*/
#define RG_GAMEPAD_GPIO_MAP {\
    {RG_KEY_UP,     .num = GPIO_NUM_21, .pullup = 1, .level = 0},\
    {RG_KEY_RIGHT,  .num = GPIO_NUM_2,  .pullup = 1, .level = 0},\
    {RG_KEY_DOWN,   .num = GPIO_NUM_1,  .pullup = 1, .level = 0},\
    {RG_KEY_LEFT,   .num = GPIO_NUM_46, .pullup = 1, .level = 0},\
    {RG_KEY_X,      .num = GPIO_NUM_16, .pullup = 1, .level = 0},\
    {RG_KEY_Y,      .num = GPIO_NUM_17, .pullup = 1, .level = 0},\
    {RG_KEY_MENU,   .num = GPIO_NUM_9,  .pullup = 1, .level = 0},\
    {RG_KEY_A,      .num = GPIO_NUM_18, .pullup = 1, .level = 0},\
    {RG_KEY_B,      .num = GPIO_NUM_5,  .pullup = 1, .level = 0},\
    {RG_KEY_SELECT, .num = GPIO_NUM_8,  .pullup = 1, .level = 0},\
    {RG_KEY_START,  .num = GPIO_NUM_38, .pullup = 1, .level = 0},\
}

// Battery
#define RG_BATTERY_DRIVER           1
#define RG_BATTERY_ADC_UNIT         ADC_UNIT_1
#define RG_BATTERY_ADC_CHANNEL      ADC_CHANNEL_3
#define RG_BATTERY_CALC_PERCENT(raw) (((raw) * 2.f - 3500.f) / (4000.f - 3500.f) * 100.f)
#define RG_BATTERY_CALC_VOLTAGE(raw) ((raw) * 2.f * 0.001f)


// Status LED
#define RG_GPIO_LED                 GPIO_NUM_NC

// I2C BUS
// #define RG_GPIO_I2C_SDA             GPIO_NUM_NC
// #define RG_GPIO_I2C_SCL             GPIO_NUM_NC

// SPI Display
#define RG_GPIO_LCD_MISO            GPIO_NUM_NC
#define RG_GPIO_LCD_MOSI            GPIO_NUM_11
#define RG_GPIO_LCD_CLK             GPIO_NUM_14
#define RG_GPIO_LCD_CS              GPIO_NUM_12
#define RG_GPIO_LCD_DC              GPIO_NUM_13
#define RG_GPIO_LCD_BCKL            GPIO_NUM_10
#define RG_GPIO_LCD_RST             GPIO_NUM_NC

// SPI SD Card
#define RG_GPIO_SDMMC_CMD          GPIO_NUM_41
#define RG_GPIO_SDMMC_CLK          GPIO_NUM_40
#define RG_GPIO_SDMMC_D0           GPIO_NUM_39

// External I2S DAC
#define RG_GPIO_SND_I2S_BCK         GPIO_NUM_15
#define RG_GPIO_SND_I2S_WS          GPIO_NUM_7
#define RG_GPIO_SND_I2S_DATA        GPIO_NUM_6
#define RG_GPIO_SND_AMP_ENABLE      GPIO_NUM_3
