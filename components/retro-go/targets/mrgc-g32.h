// REF: https://www.myretrogamecase.com/products/game-mini-g32-esp32-retro-gaming-console-1

// Target definition
#define RG_TARGET_NAME             "MRGC-G32"

// Driver selection
#define RG_DRIVER_AUDIO             2   // 1 = Built-in I2S DAC, 2 = Ext I2S
#define RG_DRIVER_BATTERY           2   // 1 = ADC, 2 = MCP23008
#define RG_DRIVER_DISPLAY           1   // 1 = ili9341, 2 = ili9342
#define RG_DRIVER_GAMEPAD           3   // 1 = GPIO, 2 = Serial, 3 = MCP23008
#define RG_DRIVER_SDCARD            2   // 1 = SDSPI, 2 = SDMMC
#define RG_DRIVER_SETTINGS          1   // 1 = SD Card, 2 = NVS

// Video
#define RG_SCREEN_WIDTH             (320)
#define RG_SCREEN_HEIGHT            (240)

// Battery ADC
#define RG_BATT_CALC_PERCENT(adc)   50
#define RG_BATT_CALC_VOLTAGE(adc)   3.5f
#define RG_BATT_ADC_CHAN            ADC1_CHANNEL_3

// LED
#define RG_GPIO_LED                 GPIO_NUM_13

// I2C BUS
#define RG_GPIO_I2C_SDA             GPIO_NUM_21
#define RG_GPIO_I2C_SCL             GPIO_NUM_22

// Display
#define RG_GPIO_LCD_MISO            GPIO_NUM_19
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
