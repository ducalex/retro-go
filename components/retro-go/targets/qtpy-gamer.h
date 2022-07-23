// REF: https://wiki.odroid.com/odroid_go/odroid_go

// Target definition
#define RG_TARGET_NAME             "QTPY ESP32"

// Storage
#define RG_STORAGE_DRIVER           1   // 1 = SDSPI, 2 = SDMMC, 3 = USB

// Audio
#define RG_AUDIO_USE_INT_DAC        1
#define RG_AUDIO_USE_EXT_DAC        0

// Video
#define RG_SCREEN_DRIVER            0   // 0 = ILI9341
#define RG_SCREEN_TYPE              2
#define RG_SCREEN_WIDTH             240
#define RG_SCREEN_HEIGHT            240
#define RG_SCREEN_ROTATE            0
#define RG_SCREEN_MARGIN_TOP        +80
#define RG_SCREEN_MARGIN_BOTTOM     -80
#define RG_SCREEN_MARGIN_LEFT       0
#define RG_SCREEN_MARGIN_RIGHT      0

// Input
#define RG_GAMEPAD_DRIVER           4   // 1 = ODROID-GO, 2 = Serial, 3 = MRGC-IO, 4 = QT PY AW9325
#define RG_GAMEPAD_HAS_MENU_BTN     1
#define RG_GAMEPAD_HAS_OPTION_BTN   1
//#define RG_BATT_ADC_CHANNEL         ADC1_CHANNEL_0
//#define RG_BATT_MULTIPLIER          2.0f
//#define RG_BATT_VOLT_MIN            3.5f
//#define RG_BATT_VOLT_MAX            4.2f

// Status LED - not actually used, we've got neopixels instead :/
// #define RG_GPIO_LED                 GPIO_NUM_NC

// I2C BUS
#define RG_GPIO_I2C_SDA             GPIO_NUM_25
#define RG_GPIO_I2C_SCL             GPIO_NUM_33

#define AW9523_DEFAULT_ADDR 0x58
#define AW9523_REG_CHIPID 0x10     ///< Register for hardcode chip ID
#define AW9523_REG_SOFTRESET 0x7F  ///< Register for soft resetting
#define AW9523_REG_INPUT0 0x00     ///< Register for reading input values
#define AW9523_REG_OUTPUT0 0x02    ///< Register for writing output values
#define AW9523_REG_CONFIG0 0x04    ///< Register for configuring direction
#define AW9523_REG_INTENABLE0 0x06 ///< Register for enabling interrupt
#define AW9523_REG_GCR 0x11        ///< Register for general configuration
#define AW9523_REG_LEDMODE 0x12    ///< Register for configuring const current


// Built-in gamepad
#define AW_GAMEPAD_IO_UP 10
#define AW_GAMEPAD_IO_DOWN 13
#define AW_GAMEPAD_IO_LEFT 14
#define AW_GAMEPAD_IO_RIGHT 12
#define AW_GAMEPAD_IO_SELECT 2
#define AW_GAMEPAD_IO_START 4
#define AW_GAMEPAD_IO_A 6
#define AW_GAMEPAD_IO_B 7
#define AW_GAMEPAD_IO_MENU 11
#define AW_GAMEPAD_IO_OPTION 5
#define AW_CARDDET 1
#define AW_TFT_RESET 8
#define AW_TFT_BACKLIGHT 3
#define AW_HEADPHONE_EN 15

// SPI Display
#define RG_GPIO_LCD_HOST            SPI2_HOST
#define RG_GPIO_LCD_MISO            GPIO_NUM_12
#define RG_GPIO_LCD_MOSI            GPIO_NUM_13
#define RG_GPIO_LCD_CLK             GPIO_NUM_14
#define RG_GPIO_LCD_CS              GPIO_NUM_27
#define RG_GPIO_LCD_DC              GPIO_NUM_7
// #define RG_GPIO_LCD_BCKL            GPIO_NUM_5  // not used!

// SPI SD Card
#define RG_GPIO_SDSPI_HOST          SPI2_HOST
#define RG_GPIO_SDSPI_MISO          GPIO_NUM_12
#define RG_GPIO_SDSPI_MOSI          GPIO_NUM_13
#define RG_GPIO_SDSPI_CLK           GPIO_NUM_14
#define RG_GPIO_SDSPI_CS            GPIO_NUM_32

// External I2S DAC UNUSED
#define RG_GPIO_SND_I2S_BCK         GPIO_NUM_21
#define RG_GPIO_SND_I2S_WS          GPIO_NUM_20
#define RG_GPIO_SND_I2S_DATA        GPIO_NUM_2
// #define RG_GPIO_SND_AMP_ENABLE      GPIO_NUM_NC
