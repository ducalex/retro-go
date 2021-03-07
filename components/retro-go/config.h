// Audio
#define RG_AUDIO_I2S_NUM       (I2S_NUM_0)

// Video
#define RG_SCREEN_WIDTH        (320)
#define RG_SCREEN_HEIGHT       (240)

// Battery ADC
#define RG_BATT_VOLTAGE_FULL   (4.2f)
#define RG_BATT_VOLTAGE_EMPTY  (3.5f)
#define RG_BATT_DIVIDER_R1     (10000)
#define RG_BATT_DIVIDER_R2     (10000)

// GPIO Allocations
#define RG_GPIO_LED            GPIO_NUM_2
#define RG_GPIO_DAC1           GPIO_NUM_25
#define RG_GPIO_DAC2           GPIO_NUM_26
#define RG_GPIO_I2S_DAC_BCK    GPIO_NUM_4
#define RG_GPIO_I2S_DAC_WS     GPIO_NUM_12
#define RG_GPIO_I2S_DAC_DATA   GPIO_NUM_15
#define RG_GPIO_GAMEPAD_X      ADC1_CHANNEL_6
#define RG_GPIO_GAMEPAD_Y      ADC1_CHANNEL_7
#define RG_GPIO_GAMEPAD_SELECT GPIO_NUM_27
#define RG_GPIO_GAMEPAD_START  GPIO_NUM_39
#define RG_GPIO_GAMEPAD_A      GPIO_NUM_32
#define RG_GPIO_GAMEPAD_B      GPIO_NUM_33
#define RG_GPIO_GAMEPAD_MENU   GPIO_NUM_13
#define RG_GPIO_GAMEPAD_VOLUME GPIO_NUM_0
#define RG_GPIO_SPI_MISO       GPIO_NUM_19
#define RG_GPIO_SPI_MOSI       GPIO_NUM_23
#define RG_GPIO_SPI_CLK        GPIO_NUM_18
#define RG_GPIO_LCD_MISO       RG_GPIO_SPI_MISO
#define RG_GPIO_LCD_MOSI       RG_GPIO_SPI_MOSI
#define RG_GPIO_LCD_CLK        RG_GPIO_SPI_CLK
#define RG_GPIO_LCD_CS         GPIO_NUM_5
#define RG_GPIO_LCD_DC         GPIO_NUM_21
#define RG_GPIO_LCD_BCKL       GPIO_NUM_14
#define RG_GPIO_SD_MISO        RG_GPIO_SPI_MISO
#define RG_GPIO_SD_MOSI        RG_GPIO_SPI_MOSI
#define RG_GPIO_SD_CLK         RG_GPIO_SPI_CLK
#define RG_GPIO_SD_CS          GPIO_NUM_22
#define RG_GPIO_NES_LATCH      GPIO_NUM_15
#define RG_GPIO_NES_CLOCK      GPIO_NUM_12
#define RG_GPIO_NES_DATA       GPIO_NUM_4

// SD Card Paths
#define RG_BASE_PATH           "/sd"
#define RG_BASE_PATH_ROMS      RG_BASE_PATH "/roms"
#define RG_BASE_PATH_SAVES     RG_BASE_PATH "/odroid/data"
#define RG_BASE_PATH_TEMP      RG_BASE_PATH "/odroid/data" // temp
#define RG_BASE_PATH_CACHE     RG_BASE_PATH "/odroid/cache"
#define RG_BASE_PATH_ROMART    RG_BASE_PATH "/romart"

// Partitions
#define RG_APP_LAUNCHER        "launcher"
#define RG_APP_FACTORY         NULL

// Third party libraries needed for some features
#define RG_USE_MINIZ          (1)
#define RG_USE_LUPNG          (1)
