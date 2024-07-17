// Target definition
#define RG_TARGET_NAME             "QTPY ESP32"

// Storage
#define RG_STORAGE_ROOT             "/sd"
#define RG_STORAGE_SDSPI_HOST       SPI2_HOST
#define RG_STORAGE_SDSPI_SPEED      SDMMC_FREQ_DEFAULT
// #define RG_STORAGE_SDMMC_HOST       SDMMC_HOST_SLOT_1
// #define RG_STORAGE_SDMMC_SPEED      SDMMC_FREQ_DEFAULT
// #define RG_STORAGE_FLASH_PARTITION  "vfs"

// Audio
#define RG_AUDIO_USE_INT_DAC        2   // 0 = Disable, 1 = GPIO25, 2 = GPIO26, 3 = Both
#define RG_AUDIO_USE_EXT_DAC        0   // 0 = Disable, 1 = Enable

// Video
#define RG_SCREEN_DRIVER            0   // 0 = ILI9341
#define RG_SCREEN_HOST              SPI2_HOST
#define RG_SCREEN_SPEED             SPI_MASTER_FREQ_40M
#define RG_SCREEN_BACKLIGHT         1
#define RG_SCREEN_WIDTH             240
#define RG_SCREEN_HEIGHT            240
#define RG_SCREEN_ROTATE            0
#define RG_SCREEN_MARGIN_TOP        +80
#define RG_SCREEN_MARGIN_BOTTOM     -80
#define RG_SCREEN_MARGIN_LEFT       0
#define RG_SCREEN_MARGIN_RIGHT      0
#define RG_SCREEN_INIT()          \
    ILI9341_CMD(0x36, 0xC0); \
    ILI9341_CMD(0x21); /* Invert colors */


// Input
// Refer to rg_input.h to see all available RG_KEY_* and RG_GAMEPAD_*_MAP types
#define RG_GAMEPAD_I2C_MAP {\
    {RG_KEY_UP,     (1<<10)},\
    {RG_KEY_RIGHT,  (1<<12)},\
    {RG_KEY_DOWN,   (1<<13)},\
    {RG_KEY_LEFT,   (1<<14)},\
    {RG_KEY_SELECT, (1<<2)},\
    {RG_KEY_START,  (1<<4)},\
    {RG_KEY_MENU,   (1<<11)},\
    {RG_KEY_OPTION, (1<<5)},\
    {RG_KEY_A,      (1<<6)},\
    {RG_KEY_B,      (1<<7)},\
}

// Battery
#define RG_BATTERY_DRIVER           0
#define RG_BATTERY_CALC_PERCENT(raw) (99)
#define RG_BATTERY_CALC_VOLTAGE(raw) (0)

// Status LED - not actually used, we've got neopixels instead :/
// #define RG_GPIO_LED                 GPIO_NUM_NC

// I2C BUS
#define RG_GPIO_I2C_SDA             GPIO_NUM_25
#define RG_GPIO_I2C_SCL             GPIO_NUM_33

// Built-in gamepad
#define AW_CARDDET 1
#define AW_TFT_RESET 8
#define AW_TFT_BACKLIGHT 3
#define AW_HEADPHONE_EN 15

// SPI Display
#define RG_GPIO_LCD_MISO            GPIO_NUM_12
#define RG_GPIO_LCD_MOSI            GPIO_NUM_13
#define RG_GPIO_LCD_CLK             GPIO_NUM_14
#define RG_GPIO_LCD_CS              GPIO_NUM_27
#define RG_GPIO_LCD_DC              GPIO_NUM_7
// #define RG_GPIO_LCD_BCKL            GPIO_NUM_5  // not used!
// #define RG_GPIO_LCD_RST           GPIO_NUM_NC

// SPI SD Card
#define RG_GPIO_SDSPI_MISO          GPIO_NUM_12
#define RG_GPIO_SDSPI_MOSI          GPIO_NUM_13
#define RG_GPIO_SDSPI_CLK           GPIO_NUM_14
#define RG_GPIO_SDSPI_CS            GPIO_NUM_32

// External I2S DAC UNUSED
#define RG_GPIO_SND_I2S_BCK         GPIO_NUM_21
#define RG_GPIO_SND_I2S_WS          GPIO_NUM_20
#define RG_GPIO_SND_I2S_DATA        GPIO_NUM_2
// #define RG_GPIO_SND_AMP_ENABLE      GPIO_NUM_NC
