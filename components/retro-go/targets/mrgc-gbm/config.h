// REF: https://www.myretrogamecase.com/products/game-mini-esp32
// Original firmware source code: GBA ESP32 link on https://www.myretrogamecase.com/pages/retro-handheld-gaming-firmware
// Source code is in the "esplay-base-firmware" directory of the .rar archive in the above link.

// Note: As of Late 2022, the owner of this shop has vanished from the net. Orders may not be fulfilled. 

// Known issues:
// Battery meter needs to be configured.
// Cropping most noticeable on NES, SNES, Genesis, PC Engine. 
// Scaling option for above should eventually be removed or changed if downscaling is added.
// Disk LED does nothing (or isn't mapped yet) and should be removed for this target.
// Sometimes takes more than one attempt to flash. (Stock bootloader problem? Hardware?)
// Would benefit from a custom theme for small screens.


// Parts:
// Unknown ESP-32 (Most likely ESP32-WROVER-B) (SOC)


// Target definition
#define RG_TARGET_NAME             "MRGC-GBM"

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
#define RG_SCREEN_TYPE              5   // Game Box Mini 240x192 screen
#define RG_SCREEN_WIDTH             240
#define RG_SCREEN_HEIGHT            230 // Display height 192 plus margin? Was 232
#define RG_SCREEN_ROTATE            0
#define RG_SCREEN_MARGIN_TOP        38
#define RG_SCREEN_MARGIN_BOTTOM     2  // Too little gives you garbage under the bottom bezel.
#define RG_SCREEN_MARGIN_LEFT       0
#define RG_SCREEN_MARGIN_RIGHT      0

// Input
#define RG_GAMEPAD_DRIVER           3   // 1 = ODROID-GO, 2 = Serial, 3 = I2C, 4 = AW9523, 5 = ESPLAY-S3, 6 = SDL2
#define RG_GAMEPAD_HAS_MENU_BTN     1
#define RG_GAMEPAD_HAS_OPTION_BTN   0   // The power button does not seem to be mappable.
/**
 * The Stock firmware, left to right is:    Start,  Select, Menu,   Power
 * With the plastic shell, the buttons are: S/P,    Reset,  Sound,  On/Off
 * Left to right, these buttons are:        (1<<0), (1<<1), (1<<8), null?
 */
#define RG_GAMEPAD_MAP_MENU         (1<<8) 
#define RG_GAMEPAD_MAP_OPTION       (0)
#define RG_GAMEPAD_MAP_START        (1<<1)
#define RG_GAMEPAD_MAP_SELECT       (1<<0)
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
// #define RG_BATTERY_ADC_CHANNEL      ADC1_CHANNEL_0 // Default 0, commented out.
#define RG_BATTERY_CALC_PERCENT(raw) (((raw) - 170) / 30.f * 100.f)
#define RG_BATTERY_CALC_VOLTAGE(raw) (0)

// Status LED
// #define RG_GPIO_LED                 GPIO_NUM_NC
// #define RG_GPIO_LED                 GPIO_NUM_13 // From OG Firmware

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
