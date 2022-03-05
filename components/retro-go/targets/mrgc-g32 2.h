// REF: https://www.myretrogamecase.com/products/game-mini-g32-esp32-retro-gaming-console-1

// Parts:
// - ESP32-WROVER-B (SoC)
// - STM32F071cbu7 (Apparently buttons, charging, LED, backlight?)
// - NXP 1334A (I2S DAC)
// - CS5082E (Power controller)
// - P8302E (Amplifier)
// - YT280S002 (ILI9341 LCD)
//

/**
 * IO35 - MENU BTN
 * IO25 - I2S DAC
 * IO26 - I2S DAC
 * IO15 - SD CARD
 * IO2 - SD CARD
 * IO0 - SELECT BTN
 * IO4 - AMP EN
 * IO5 - LCD SPI CS
 * IO12 - LCD DC
 * IO18 - SPI CLK
 * IO23 - SPI MOSI
 * IO21 - STM32F (I2C)
 * IO22 - STM32F (I2C)
 *
 * IO27 - resistor then STM32?
 *
 * Power LED is connected to the STM32
 */

// Target definition
#define RG_TARGET_NAME             "MRGC-G32"

// Storage and Settings
#define RG_STORAGE_DRIVER           2   // 1 = SDSPI, 2 = SDMMC, 3 = USB
#define RG_SETTINGS_USE_NVS         0

// Audio
#define RG_AUDIO_USE_SPEAKER        0
#define RG_AUDIO_USE_EXT_DAC        1

// Video
#define RG_SCREEN_TYPE              0
#define RG_SCREEN_WIDTH             240
#define RG_SCREEN_HEIGHT            320
#define RG_SCREEN_ROTATE            0
#define RG_SCREEN_MARGIN_TOP        28
#define RG_SCREEN_MARGIN_BOTTOM     68
#define RG_SCREEN_MARGIN_LEFT       0
#define RG_SCREEN_MARGIN_RIGHT      0

// Input
#define RG_GAMEPAD_DRIVER           3   // 1 = ODROID-GO, 2 = Serial, 3 = MRGC-IO
#define RG_GAMEPAD_MENU_BTN         1
#define RG_GAMEPAD_OPTION_BTN       0
// #define RG_BATT_ADC_CHANNEL         ADC1_CHANNEL_0
#define RG_BATT_MULTIPLIER          4.0f
#define RG_BATT_VOLT_MIN            7.0f
#define RG_BATT_VOLT_MAX            8.4f

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

// External I2S DAC
#define RG_GPIO_SND_I2S_BCK         GPIO_NUM_26
#define RG_GPIO_SND_I2S_WS          GPIO_NUM_25
#define RG_GPIO_SND_I2S_DATA        GPIO_NUM_19
#define RG_GPIO_SND_AMP_ENABLE      GPIO_NUM_4
