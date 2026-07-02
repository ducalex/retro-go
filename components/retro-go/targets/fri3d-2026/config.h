/* Configuration for Fri3D Camp 2026 Badge, based on the ESP32-S3.
 *
 * Hardware info: https://github.com/Fri3dCamp/badge_2026_hw
 *
 * Fri3D Camp info: https://fri3d.be/
 *
 */

// Target definition
#define RG_TARGET_NAME             "FRI3D-2026"

// Storage
#define RG_STORAGE_ROOT             "/sd"
#define RG_STORAGE_SDSPI_HOST       SPI2_HOST
#define RG_STORAGE_SDSPI_SPEED      SDMMC_FREQ_DEFAULT
#define RG_STORAGE_FLASH_PARTITION  "vfs"
#define RG_STORAGE_FLASH_PARTITION_LITTLEFS

// Audio
#define RG_AUDIO_USE_BUZZER_PIN     38
#define RG_AUDIO_USE_INT_DAC        0   // 0 = Disable, 1 = GPIO25, 2 = GPIO26, 3 = Both
#define RG_AUDIO_USE_EXT_DAC        1   // 0 = Disable, 1 = Enable

// Video
#define RG_SCREEN_DRIVER            0   // 0 = ILI9341/ST7789
#define RG_SCREEN_HOST              SPI2_HOST
#define RG_SCREEN_SPEED             SPI_MASTER_FREQ_40M // Low clock frequency (10Mhz) is bad for framerate
#define RG_SCREEN_BACKLIGHT         0
#define RG_SCREEN_WIDTH             320
#define RG_SCREEN_HEIGHT            240
#define RG_SCREEN_ROTATION          0
#define RG_SCREEN_RGB_BGR           1
#define RG_SCREEN_PIXEL_FORMAT      0
#define RG_SCREEN_VISIBLE_AREA      {0, 0, 0, 0}  // Left, Top, Right, Bottom
#define RG_SCREEN_SAFE_AREA         {0, 0, 0, 0}  // Left, Top, Right, Bottom
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
    ILI9341_CMD(0xB1, 0x00, 0x10);           /* Frame Rate Control (1B=70, 1F=61, 10=119) */                     \
    ILI9341_CMD(0xB6, 0x0A, 0xA2);           /* Display Function Control */                                      \
    ILI9341_CMD(0xF6, 0x01, 0x30);                                                                               \
    ILI9341_CMD(0xF2, 0x00);                 /* 3Gamma Function Disable */                                       \
    ILI9341_CMD(0xE0, 0xD0, 0x00, 0x05, 0x0E, 0x15, 0x0D, 0x37, 0x43, 0x47, 0x09, 0x15, 0x12, 0x16, 0x19);       \
    ILI9341_CMD(0xE1, 0xD0, 0x00, 0x05, 0x0D, 0x0C, 0x06, 0x2D, 0x44, 0x40, 0x0E, 0x1C, 0x18, 0x16, 0x19);

// Fri3D 2026 buttons are mainly on the CH32 expander; only START is direct on GPIO0 for now.
// PCB labels:
// GPIO_NUM_0: S (for START)
#define RG_GAMEPAD_GPIO_MAP {\
    {RG_KEY_START,  .num = GPIO_NUM_0,  .pullup = 1, .level = 0},\
}


// Not using RG_KEY_START because long pressing that one has a special meaning in Duke3D:
#define RG_RECOVERY_BTN RG_KEY_MENU // Keep this button pressed to open the recovery menu

// CH32 expander button states (context/badge_2026_fw/README.md). Ignore USB/charger bits.
// PCB labels:
// 6: MENU
// 3: Y
// 2: X
// 4: A
// 5: B
#define RG_GAMEPAD_I2C_MAP {\
    {RG_KEY_RIGHT,  .num = 10, .level = 1},\
    {RG_KEY_LEFT,   .num = 9,  .level = 1},\
    {RG_KEY_DOWN,   .num = 8,  .level = 1},\
    {RG_KEY_UP,     .num = 7,  .level = 1},\
    {RG_KEY_MENU,   .num = 6,  .level = 1},\
    {RG_KEY_B,      .num = 5,  .level = 1},\
    {RG_KEY_A,      .num = 4,  .level = 1},\
    {RG_KEY_OPTION, .num = 3,  .level = 1},\
    {RG_KEY_SELECT, .num = 2,  .level = 1},\
}



// Battery (CH32 expander-backed ADC not wired yet in C)
#define RG_BATTERY_DRIVER           0
#define RG_BATTERY_ADC_UNIT         -1 // disabled

// I2C BUS (CH32 expander)
#define RG_GPIO_I2C_SDA             GPIO_NUM_39
#define RG_GPIO_I2C_SCL             GPIO_NUM_42
#define RG_I2C_GPIO_DRIVER          6   // CH32X035 expander (Fri3D 2026)
#define RG_I2C_GPIO_ADDR            0x50

// SPI Display
#define RG_GPIO_LCD_MISO            GPIO_NUM_8
#define RG_GPIO_LCD_MOSI            GPIO_NUM_6
#define RG_GPIO_LCD_CLK             GPIO_NUM_7
#define RG_GPIO_LCD_CS              GPIO_NUM_5
#define RG_GPIO_LCD_DC              GPIO_NUM_4
//#define RG_GPIO_LCD_RST             GPIO_NUM_NC // CH32 coprocessor does the reset

// SPI SD Card
#define RG_GPIO_SDSPI_MISO          RG_GPIO_LCD_MISO
#define RG_GPIO_SDSPI_MOSI          RG_GPIO_LCD_MOSI
#define RG_GPIO_SDSPI_CLK           RG_GPIO_LCD_CLK
#define RG_GPIO_SDSPI_CS            GPIO_NUM_14

// Headphone I2S DAC
#define RG_GPIO_SND_I2S_MCK         GPIO_NUM_17
#define RG_GPIO_SND_I2S_BCK         GPIO_NUM_2 // also known as SCLK
#define RG_GPIO_SND_I2S_WS          GPIO_NUM_47 // also known as LRCK
#define RG_GPIO_SND_I2S_DATA        GPIO_NUM_16

// Communicator Add-On is on the same pins so also receives the audio

#define RG_CUSTOM_PLATFORM_INIT()   \
    RG_LOGW("Waiting for CH32 coprocessor/expander to finish booting..."); \
    rg_task_delay(1000);
