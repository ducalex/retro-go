// Target definition
#define RG_TARGET_NAME             "ESP32S3-DEVKIT-C-VGA"

// Storage
#define RG_STORAGE_ROOT             "/sd"
#define RG_STORAGE_SDSPI_HOST       SPI3_HOST
#define RG_STORAGE_SDSPI_SPEED      SDMMC_FREQ_DEFAULT
// #define RG_STORAGE_SDMMC_HOST       SDMMC_HOST_SLOT_1
// #define RG_STORAGE_SDMMC_SPEED      SDMMC_FREQ_DEFAULT
// #define RG_STORAGE_FLASH_PARTITION  "vfs"

// Audio
#define RG_AUDIO_USE_INT_DAC        0   // 0 = Disable, 1 = GPIO25, 2 = GPIO26, 3 = Both
#define RG_AUDIO_USE_EXT_DAC        1   // 0 = Disable, 1 = Enable

#define RG_GPIO_SND_I2S_BCK         38
#define RG_GPIO_SND_I2S_WS          3
#define RG_GPIO_SND_I2S_DATA        46

// Video
#define RG_SCREEN_DRIVER            1   // 0 = ILI9341, 1 = VGA
#define RG_SCREEN_WIDTH             320
#define RG_SCREEN_HEIGHT            240
#define RG_SCREEN_ROTATE            0
#define RG_SCREEN_MARGIN_TOP        0
#define RG_SCREEN_MARGIN_BOTTOM     0
#define RG_SCREEN_MARGIN_LEFT       0
#define RG_SCREEN_MARGIN_RIGHT      0


// Input
// Refer to rg_input.h to see all available RG_KEY_* and RG_GAMEPAD_*_MAP types
#define RG_GAMEPAD_I2C_MAP {\
    {RG_KEY_UP,     (1<<2)},\
    {RG_KEY_RIGHT,  (1<<1)},\
    {RG_KEY_DOWN,   (1<<0)},\
    {RG_KEY_LEFT,   (1<<3)},\
    {RG_KEY_SELECT, (1<<4)},\
    {RG_KEY_START,  (1<<5)},\
    {RG_KEY_MENU,   (1<<6)},\
    {RG_KEY_OPTION, (1<<7)},\
    {RG_KEY_A,      (1<<8)},\
    {RG_KEY_B,      (1<<9)},\
}

#define RG_GPIO_I2C_SDA             GPIO_NUM_1
#define RG_GPIO_I2C_SCL             GPIO_NUM_2

// Battery
#define RG_BATTERY_DRIVER           0
#define RG_BATTERY_ADC_UNIT         ADC_UNIT_1
#define RG_BATTERY_ADC_CHANNEL      ADC_CHANNEL_3
#define RG_BATTERY_CALC_PERCENT(raw) (((raw) * 2.f - 3500.f) / (4200.f - 3500.f) * 100.f)
#define RG_BATTERY_CALC_VOLTAGE(raw) ((raw) * 2.f * 0.001f)

// Status LED
#undef RG_GPIO_LED

#define RG_GPIO_SDSPI_MISO          GPIO_NUM_14
#define RG_GPIO_SDSPI_MOSI          GPIO_NUM_12
#define RG_GPIO_SDSPI_CLK           GPIO_NUM_13
#define RG_GPIO_SDSPI_CS            GPIO_NUM_11