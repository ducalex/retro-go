// Target definition
#define RG_TARGET_NAME             "ESPLAY-S3"

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
#define RG_SCREEN_SPEED             SPI_MASTER_FREQ_80M
#define RG_SCREEN_BACKLIGHT         1
#define RG_SCREEN_WIDTH             320
#define RG_SCREEN_HEIGHT            240
#define RG_SCREEN_ROTATE            0
#define RG_SCREEN_MARGIN_TOP        0
#define RG_SCREEN_MARGIN_BOTTOM     0
#define RG_SCREEN_MARGIN_LEFT       0
#define RG_SCREEN_MARGIN_RIGHT      0
#define RG_SCREEN_INIT()                                                                                   \
    ILI9341_CMD(0xC5, 0x1A);                         /* VCOM */                                            \
    ILI9341_CMD(0x36, 0x60);                         /* Display Rotation */                                \
    ILI9341_CMD(0xB2, 0x05, 0x05, 0x00, 0x33, 0x33); /* Porch Setting */                                   \
    ILI9341_CMD(0xB7, 0x05);                         /* Gate Control //12.2v   -10.43v */                  \
    ILI9341_CMD(0xBB, 0x3F);                         /* VCOM */                                            \
    ILI9341_CMD(0xC0, 0x2c);                         /* Power control */                                   \
    ILI9341_CMD(0xC2, 0x01);                         /* VDV and VRH Command Enable */                      \
    ILI9341_CMD(0xC3, 0x0F);                         /* VRH Set 4.3+( vcom+vcom offset+vdv) */             \
    ILI9341_CMD(0xC4, 0xBE);                         /* VDV Set 0v */                                      \
    ILI9341_CMD(0xC6, 0X01);                         /* Frame Rate Control in Normal Mode 111Hz */         \
    ILI9341_CMD(0xD0, 0xA4, 0xA1);                   /* Power Control 1 */                                 \
    ILI9341_CMD(0xE8, 0x03);                         /* Power Control 1 */                                 \
    ILI9341_CMD(0xE9, 0x09, 0x09, 0x08);             /* Equalize time control */                           \
    ILI9341_CMD(0xE0, 0xD0, 0x05, 0x09, 0x09, 0x08, 0x14, 0x28, 0x33, 0x3F, 0x07, 0x13, 0x14, 0x28, 0x30); \
    ILI9341_CMD(0xE1, 0xD0, 0x05, 0x09, 0x09, 0x08, 0x03, 0x24, 0x32, 0x32, 0x3B, 0x14, 0x13, 0x28, 0x2F, 0x1F);

// Input
// Refer to rg_input.h to see all available RG_KEY_* and RG_GAMEPAD_*_MAP types
#define RG_GAMEPAD_I2C_MAP {\
    {RG_KEY_UP,     (1<<2)},\
    {RG_KEY_RIGHT,  (1<<5)},\
    {RG_KEY_DOWN,   (1<<3)},\
    {RG_KEY_LEFT,   (1<<4)},\
    {RG_KEY_SELECT, (1<<1)},\
    {RG_KEY_START,  (1<<0)},\
    {RG_KEY_A,      (1<<6)},\
    {RG_KEY_B,      (1<<7)},\
}
#define RG_GAMEPAD_GPIO_MAP {\
    {RG_KEY_L,      GPIO_NUM_40, GPIO_PULLUP_ONLY, 0},\
    {RG_KEY_R,      GPIO_NUM_41, GPIO_PULLUP_ONLY, 0},\
    {RG_KEY_MENU,   GPIO_NUM_42, GPIO_PULLUP_ONLY, 0},\
    {RG_KEY_OPTION, GPIO_NUM_41, GPIO_PULLUP_ONLY, 0},\
}

// Battery
#define RG_BATTERY_DRIVER           1
#define RG_BATTERY_ADC_UNIT         ADC_UNIT_1
#define RG_BATTERY_ADC_CHANNEL      ADC_CHANNEL_3
#define RG_BATTERY_CALC_PERCENT(raw) (((raw) * 2.f - 3500.f) / (4200.f - 3500.f) * 100.f)
#define RG_BATTERY_CALC_VOLTAGE(raw) ((raw) * 2.f * 0.001f)

// Status LED
#define RG_GPIO_LED                 GPIO_NUM_2

// I2C BUS
#define RG_GPIO_I2C_SDA             GPIO_NUM_10
#define RG_GPIO_I2C_SCL             GPIO_NUM_11

// SPI Display
#define RG_GPIO_LCD_MISO            GPIO_NUM_NC
#define RG_GPIO_LCD_MOSI            GPIO_NUM_12
#define RG_GPIO_LCD_CLK             GPIO_NUM_48
#define RG_GPIO_LCD_CS              GPIO_NUM_8
#define RG_GPIO_LCD_DC              GPIO_NUM_47
#define RG_GPIO_LCD_BCKL            GPIO_NUM_39
#define RG_GPIO_LCD_RST             GPIO_NUM_3

// SPI SD Card
#define RG_GPIO_SDSPI_CMD          GPIO_NUM_14
#define RG_GPIO_SDSPI_CLK          GPIO_NUM_21
#define RG_GPIO_SDSPI_D0           GPIO_NUM_17

// External I2S DAC
#define RG_GPIO_SND_I2S_BCK         38
#define RG_GPIO_SND_I2S_WS          13
#define RG_GPIO_SND_I2S_DATA        9
#define RG_GPIO_SND_AMP_ENABLE      18