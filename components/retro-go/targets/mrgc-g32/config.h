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

// Storage
#define RG_STORAGE_DRIVER           2                   // 0 = Host, 1 = SDSPI, 2 = SDMMC, 3 = USB, 4 = Flash
#define RG_STORAGE_HOST             SDMMC_HOST_SLOT_1   // Used by SDSPI and SDMMC
#define RG_STORAGE_SPEED            SDMMC_FREQ_DEFAULT  // Used by SDSPI and SDMMC
#define RG_STORAGE_ROOT             "/sd"               // Storage mount point

// Audio
#define RG_AUDIO_USE_INT_DAC        0   // 0 = Disable, 1 = GPIO25, 2 = GPIO26, 3 = Both
#define RG_AUDIO_USE_EXT_DAC        1   // 0 = Disable, 1 = Enable

// Video
#define RG_SCREEN_DRIVER            0   // 0 = ILI9341
#define RG_SCREEN_HOST              SPI2_HOST
#define RG_SCREEN_SPEED             SPI_MASTER_FREQ_40M
#define RG_SCREEN_TYPE              1
#define RG_SCREEN_WIDTH             240
#define RG_SCREEN_HEIGHT            320
#define RG_SCREEN_ROTATE            0
#define RG_SCREEN_MARGIN_TOP        28
#define RG_SCREEN_MARGIN_BOTTOM     68
#define RG_SCREEN_MARGIN_LEFT       0
#define RG_SCREEN_MARGIN_RIGHT      0

// Input
#define RG_GAMEPAD_DRIVER           3   // 1 = ODROID-GO, 2 = Serial, 3 = I2C, 4 = AW9523, 5 = ESPLAY-S3, 6 = SDL2
#define RG_GAMEPAD_HAS_MENU_BTN     1
#define RG_GAMEPAD_HAS_OPTION_BTN   0
// Note: Depending on the driver, the button map can represent bits, registers, keys, or gpios.
#define RG_GAMEPAD_MAP_MENU         (1<<8)
#define RG_GAMEPAD_MAP_OPTION       (0)
#define RG_GAMEPAD_MAP_START        (1<<0)
#define RG_GAMEPAD_MAP_SELECT       (1<<1)
#define RG_GAMEPAD_MAP_UP           (1<<2)
#define RG_GAMEPAD_MAP_RIGHT        (1<<5)
#define RG_GAMEPAD_MAP_DOWN         (1<<3)
#define RG_GAMEPAD_MAP_LEFT         (1<<4)
#define RG_GAMEPAD_MAP_A            (1<<6)
#define RG_GAMEPAD_MAP_B            (1<<7)
#define RG_GAMEPAD_MAP_X            (0)
#define RG_GAMEPAD_MAP_Y            (0)
#define RG_GAMEPAD_MAP_L            (0)
#define RG_GAMEPAD_MAP_R            (0)

// Battery
// #define RG_BATTERY_ADC_CHANNEL      ADC1_CHANNEL_0
#define RG_BATTERY_CALC_PERCENT(raw) (((raw) - 170) / 30.f * 100.f)
#define RG_BATTERY_CALC_VOLTAGE(raw) (0)

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
