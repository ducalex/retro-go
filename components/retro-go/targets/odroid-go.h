// REF: https://wiki.odroid.com/odroid_go/odroid_go

// Target definition
#define RG_TARGET_NAME             "ODROID-GO"

// Driver selection
#define RG_DRIVER_AUDIO             1   // 1 = I2S
#define RG_DRIVER_DISPLAY           1   // 1 = ili9341, 2 = ili9342
#define RG_DRIVER_GAMEPAD           1   // 1 = GPIO, 2 = Serial, 3 = I2C
#define RG_DRIVER_SDCARD            1   // 1 = SDSPI, 2 = SDMMC
#define RG_DRIVER_SETTINGS          1   // 1 = SD Card, 2 = NVS

// Video
#define RG_SCREEN_WIDTH             (320)
#define RG_SCREEN_HEIGHT            (240)

// Battery ADC
#define RG_BATT_CALC_PERCENT(adc)   RG_MAX(0, RG_MIN(100, (RG_BATT_CALC_VOLTAGE(adc) - 3.5f) / (4.2f - 3.5f) * 100))
#define RG_BATT_CALC_VOLTAGE(adc)   ((adc) * 2.f)
#define RG_BATT_ADC_CHAN            ADC1_CHANNEL_0

// LED
#define RG_GPIO_LED                 GPIO_NUM_2
// Built-in gamepad
#define RG_GPIO_GAMEPAD_X           ADC1_CHANNEL_6
#define RG_GPIO_GAMEPAD_Y           ADC1_CHANNEL_7
#define RG_GPIO_GAMEPAD_SELECT      GPIO_NUM_27
#define RG_GPIO_GAMEPAD_START       GPIO_NUM_39
#define RG_GPIO_GAMEPAD_A           GPIO_NUM_32
#define RG_GPIO_GAMEPAD_B           GPIO_NUM_33
#define RG_GPIO_GAMEPAD_MENU        GPIO_NUM_13
#define RG_GPIO_GAMEPAD_VOLUME      GPIO_NUM_0
// SNES-style gamepad
#define RG_GPIO_GAMEPAD_LATCH       GPIO_NUM_15
#define RG_GPIO_GAMEPAD_CLOCK       GPIO_NUM_12
#define RG_GPIO_GAMEPAD_DATA        GPIO_NUM_4
// I2C BUS
#define RG_GPIO_I2C_SDA             GPIO_NUM_15
#define RG_GPIO_I2C_SCL             GPIO_NUM_4
// SPI BUS
#define RG_GPIO_LCD_MISO            GPIO_NUM_19
#define RG_GPIO_LCD_MOSI            GPIO_NUM_23
#define RG_GPIO_LCD_CLK             GPIO_NUM_18
#define RG_GPIO_LCD_CS              GPIO_NUM_5
#define RG_GPIO_LCD_DC              GPIO_NUM_21
#define RG_GPIO_LCD_BCKL            GPIO_NUM_14
#define RG_GPIO_SD_MISO             GPIO_NUM_19
#define RG_GPIO_SD_MOSI             GPIO_NUM_23
#define RG_GPIO_SD_CLK              GPIO_NUM_18
#define RG_GPIO_SD_CS               GPIO_NUM_22
// External I2S DAC
#define RG_GPIO_SND_I2S_BCK         GPIO_NUM_4
#define RG_GPIO_SND_I2S_WS          GPIO_NUM_12
#define RG_GPIO_SND_I2S_DATA        GPIO_NUM_15
#define RG_GPIO_SND_AMP_ENABLE      GPIO_NUM_NC
