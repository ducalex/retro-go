// REF: https://wiki.odroid.com/odroid_go/odroid_go

// Target definition
#define RG_TARGET_NAME             "QTPY ESP32"

// Storage
#define RG_STORAGE_DRIVER           1                   // 0 = Host, 1 = SDSPI, 2 = SDMMC, 3 = USB, 4 = Flash
#define RG_STORAGE_HOST             SPI2_HOST           // Used by SDSPI and SDMMC
#define RG_STORAGE_SPEED            SDMMC_FREQ_DEFAULT  // Used by SDSPI and SDMMC
#define RG_STORAGE_ROOT             "/sd"               // Storage mount point

// Audio
#define RG_AUDIO_USE_INT_DAC        2   // 0 = Disable, 1 = GPIO25, 2 = GPIO26, 3 = Both
#define RG_AUDIO_USE_EXT_DAC        0   // 0 = Disable, 1 = Enable

// Video
#define RG_SCREEN_DRIVER            0   // 0 = ILI9341
#define RG_SCREEN_HOST              SPI2_HOST
#define RG_SCREEN_SPEED             SPI_MASTER_FREQ_40M
#define RG_SCREEN_TYPE              2
#define RG_SCREEN_WIDTH             240
#define RG_SCREEN_HEIGHT            240
#define RG_SCREEN_ROTATE            0
#define RG_SCREEN_MARGIN_TOP        +80
#define RG_SCREEN_MARGIN_BOTTOM     -80
#define RG_SCREEN_MARGIN_LEFT       0
#define RG_SCREEN_MARGIN_RIGHT      0

// Input
#define RG_GAMEPAD_DRIVER           4   // 1 = ODROID-GO, 2 = Serial, 3 = I2C, 4 = AW9523, 5 = ESPLAY-S3, 6 = SDL2
#define RG_GAMEPAD_HAS_MENU_BTN     1
#define RG_GAMEPAD_HAS_OPTION_BTN   1
// Note: Depending on the driver, the button map can be a bitmask, an index, or a GPIO.
// Refer to rg_input.c to see all available RG_KEY_*
#define RG_GAMEPAD_MAP {\
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
// #define RG_BATTERY_ADC_CHANNEL      ADC1_CHANNEL_0
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
