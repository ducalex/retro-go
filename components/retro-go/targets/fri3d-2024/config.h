/* Configuration for Fri3D Camp 2024 Badge, based on the ESP32-S3.
 *
 * Hardware info: https://github.com/Fri3dCamp/badge_2024_hw
 *
 * Fri3D Camp info: https://fri3d.be/
 *
 */

// Target definition
#define RG_TARGET_NAME             "FRI3D-2024"

// Storage
#define RG_STORAGE_ROOT             "/sd"
#define RG_STORAGE_SDSPI_HOST       SPI2_HOST
#define RG_STORAGE_SDSPI_SPEED      SDMMC_FREQ_DEFAULT
#define RG_STORAGE_FLASH_PARTITION  "vfs"

// Audio
#define RG_AUDIO_USE_BUZZER_PIN     46
#define RG_AUDIO_USE_INT_DAC        0   // 0 = Disable, 1 = GPIO25, 2 = GPIO26, 3 = Both
#define RG_AUDIO_USE_EXT_DAC        0   // 0 = Disable, 1 = Enable

// Video
#define RG_SCREEN_DRIVER            0   // 0 = ILI9341 but also works on ST7789 with below RG_SCREEN_INIT()
#define RG_SCREEN_HOST              SPI2_HOST
#define RG_SCREEN_SPEED             SPI_MASTER_FREQ_40M
#define RG_SCREEN_WIDTH             296
#define RG_SCREEN_HEIGHT            240
#define RG_SCREEN_ROTATE            0
#define RG_SCREEN_MARGIN_TOP        0
#define RG_SCREEN_MARGIN_BOTTOM     0
#define RG_SCREEN_MARGIN_LEFT       0
#define RG_SCREEN_MARGIN_RIGHT      0

#define ST7789_MADCTL 0x36 // Memory Access Control
#define ST7789_MADCTL_MV 0x20
#define ST7789_MADCTL_RGB 0x00
#define ST7789_MADCTL_BGR 0x08

#define RG_SCREEN_INIT()                                                                                         \
    ILI9341_CMD(0xCF, 0x00, 0xc3, 0x30);                                                                         \
    ILI9341_CMD(0xED, 0x64, 0x03, 0x12, 0x81);                                                                   \
    ILI9341_CMD(0xE8, 0x85, 0x00, 0x78);                                                                         \
    ILI9341_CMD(0xCB, 0x39, 0x2c, 0x00, 0x34, 0x02);                                                             \
    ILI9341_CMD(0xF7, 0x20);                                                                                     \
    ILI9341_CMD(0xEA, 0x00, 0x00);                                                                               \
    ILI9341_CMD(0xC0, 0x1B);                 /* Power control   //VRH[5:0] */                                    \
    ILI9341_CMD(0xC1, 0x12);                 /* Power control   //SAP[2:0];BT[3:0] */                            \
    ILI9341_CMD(0xC5, 0x32, 0x3C);           /* VCM control */                                                   \
    ILI9341_CMD(0xC7, 0x91);                 /* VCM control2 */                                                  \
    ILI9341_CMD(ST7789_MADCTL, (ST7789_MADCTL_MV | ST7789_MADCTL_BGR));                                          \
    ILI9341_CMD(0xB1, 0x00, 0x10);           /* Frame Rate Control (1B=70, 1F=61, 10=119) */                     \
    ILI9341_CMD(0xB6, 0x0A, 0xA2);           /* Display Function Control */                                      \
    ILI9341_CMD(0xF6, 0x01, 0x30);                                                                               \
    ILI9341_CMD(0xF2, 0x00); /* 3Gamma Function Disable */                                                       \
    ILI9341_CMD(0xE0, 0xD0, 0x00, 0x05, 0x0E, 0x15, 0x0D, 0x37, 0x43, 0x47, 0x09, 0x15, 0x12, 0x16, 0x19);       \
    ILI9341_CMD(0xE1, 0xD0, 0x00, 0x05, 0x0D, 0x0C, 0x06, 0x2D, 0x44, 0x40, 0x0E, 0x1C, 0x18, 0x16, 0x19);


// X-axis = ADC1_CHANNEL_2 = GPIO1
// Y-axis = ADC1_CHANNEL_2 = GPIO3
#define RG_GAMEPAD_ADC_MAP {\
    {RG_KEY_UP,    ADC_UNIT_1, ADC_CHANNEL_2, ADC_ATTEN_DB_11, 3072, 4096},\
    {RG_KEY_DOWN,  ADC_UNIT_1, ADC_CHANNEL_2, ADC_ATTEN_DB_11, 0, 1024},\
    {RG_KEY_RIGHT,  ADC_UNIT_1, ADC_CHANNEL_0, ADC_ATTEN_DB_11, 3072, 4096},\
    {RG_KEY_LEFT, ADC_UNIT_1, ADC_CHANNEL_0, ADC_ATTEN_DB_11, 0, 1024},\
}


#define RG_RECOVERY_BTN RG_KEY_MENU // Keep this button pressed to open the recovery menu

// RG_KEY_SELECT is labelled MENU on the PCB
// RG_KEY_START is labelled START on the PCB
// RG_KEY_OPTION is labelled Y on the PCB
// RG_KEY_MENU is labelled X on the PCB
// RG_KEY_A is labelled A on the PCB
// RG_KEY_B is labelled B on the PCB
#define RG_GAMEPAD_GPIO_MAP {\
    {RG_KEY_SELECT, GPIO_NUM_45, GPIO_PULLUP_ONLY, 0},\
    {RG_KEY_START,  GPIO_NUM_0,  GPIO_PULLUP_ONLY, 0},\
    {RG_KEY_OPTION, GPIO_NUM_38, GPIO_PULLUP_ONLY, 0},\
    {RG_KEY_MENU,   GPIO_NUM_41, GPIO_PULLUP_ONLY, 0},\
    {RG_KEY_A,      GPIO_NUM_39, GPIO_PULLUP_ONLY, 0},\
    {RG_KEY_B,      GPIO_NUM_40, GPIO_PULLUP_ONLY, 0},\
}

// Status LED
#define RG_GPIO_LED                 GPIO_NUM_21

// Battery
#define RG_BATTERY_DRIVER           1
#define RG_BATTERY_ADC_UNIT         ADC_UNIT_2
#define RG_BATTERY_ADC_CHANNEL      ADC_CHANNEL_2
// Battery voltage ranges from 3.15V (0%) to 4.15 (100%) as datasheet specifies
// 3.0 +/- 0.1V (discharge cut-off) to 4.2V (no margin of error provided, assuming 0.05V)
// Charger stops charging at 4.07V (92%) to reduce battery wear.
#define RG_BATTERY_CALC_PERCENT(raw) (((raw) * 2.f - 3150.f) / (4150.f - 3150.f) * 100.f)
#define RG_BATTERY_CALC_VOLTAGE(raw) ((raw) * 2.f * 0.001f)

// SPI Display
#define RG_GPIO_LCD_MISO            GPIO_NUM_8
#define RG_GPIO_LCD_MOSI            GPIO_NUM_6
#define RG_GPIO_LCD_CLK             GPIO_NUM_7
#define RG_GPIO_LCD_CS              GPIO_NUM_5
#define RG_GPIO_LCD_DC              GPIO_NUM_4
#define RG_GPIO_LCD_RST             GPIO_NUM_48

// SPI SD Card
#define RG_GPIO_SDSPI_MISO          RG_GPIO_LCD_MISO
#define RG_GPIO_SDSPI_MOSI          RG_GPIO_LCD_MOSI
#define RG_GPIO_SDSPI_CLK           RG_GPIO_LCD_CLK
#define RG_GPIO_SDSPI_CS            GPIO_NUM_14

