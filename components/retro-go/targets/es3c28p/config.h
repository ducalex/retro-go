// LCDWIKI/QDtech ES3C28P, a 2.8" 320x240 touchscreen board built around an
// ESP32-S3-WROOM-1 N16R8 (16MB flash, 8MB octal PSRAM). Commonly sold as a
// "2.8 inch ESP32-S3 touchscreen development board".
// Pinout traced from the vendor schematic (ES3C28P_Schematic.pdf).
#define RG_TARGET_NAME             "ES3C28P"

// Storage: no SD slot on this board, use a FAT partition on the 16MB internal flash
#define RG_STORAGE_ROOT             "/sd"
#define RG_STORAGE_FLASH_PARTITION  "vfs"

// Audio: ES8311 codec on I2S (codec is the slave), driven from a 256*fs MCK
#define RG_AUDIO_USE_INT_DAC        0
#define RG_AUDIO_USE_EXT_DAC        1
#define RG_AUDIO_CODEC_ES8311      1
#define RG_AUDIO_ES8311_ADDR       0x18

// Video
#define RG_SCREEN_DRIVER            0   // 0 = ILI9341/ST7789
#define RG_SCREEN_HOST              SPI2_HOST
#define RG_SCREEN_SPEED             SPI_MASTER_FREQ_40M
#define RG_SCREEN_BACKLIGHT         1
#define RG_SCREEN_WIDTH             320
#define RG_SCREEN_HEIGHT            240
#define RG_SCREEN_ROTATE            0
#define RG_SCREEN_VISIBLE_AREA      {0, 0, 0, 0}
#define RG_SCREEN_SAFE_AREA         {0, 0, 0, 0}
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
    ILI9341_CMD(0x36, 0x68);                 /* Memory Access Control  (MX|MV|BGR) = landscape */                \
    ILI9341_CMD(0xB1, 0x00, 0x10);           /* Frame Rate Control (1B=70, 1F=61, 10=119) */                     \
    ILI9341_CMD(0xB6, 0x0A, 0xA2);           /* Display Function Control */                                      \
    ILI9341_CMD(0xF6, 0x01, 0x30);                                                                               \
    ILI9341_CMD(0xF2, 0x00);                 /* 3Gamma Function Disable */                                       \
    ILI9341_CMD(0x26, 0x01);                 /* Gamma curve selected */                                          \
    ILI9341_CMD(0xE0, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00); \
    ILI9341_CMD(0xE1, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F); \
    ILI9341_CMD(0x21);                       /* INVON: this panel is natively inverted */

// Input: FT6336U capacitive touch mapped to on-screen button zones.
// Layout (320x240 landscape): d-pad bottom-left (overlapping rects give diagonals),
// B/A bottom-right, SELECT/START top-center, MENU/OPTION top corners.
// The BOOT button (GPIO0) is the only physical button and doubles as a MENU key.
#define RG_TOUCH_DRIVER_FT6X36      1
#define RG_TOUCH_I2C_ADDR           0x38
#define RG_TOUCH_SWAP_XY            1
#define RG_TOUCH_INVERT_X           0
#define RG_TOUCH_INVERT_Y           1
#define RG_GAMEPAD_TOUCH_MAP {\
    {RG_KEY_UP,     0, 100, 110, 146},\
    {RG_KEY_DOWN,   0, 193, 110, 239},\
    {RG_KEY_LEFT,   0, 100,  36, 239},\
    {RG_KEY_RIGHT, 74, 100, 110, 239},\
    {RG_KEY_A,    265, 120, 319, 195},\
    {RG_KEY_B,    200, 160, 260, 235},\
    {RG_KEY_SELECT, 90,  0, 150,  40},\
    {RG_KEY_START, 170,  0, 230,  40},\
    {RG_KEY_MENU,    0,  0,  50,  40},\
    {RG_KEY_OPTION, 270, 0, 319,  40},\
}
#define RG_GAMEPAD_GPIO_MAP {\
    {RG_KEY_MENU,   .num = GPIO_NUM_0, .pullup = 1, .level = 0},\
}

// I2C bus (FT6336U touch @0x38 + ES8311 codec @0x18).
// NOTE: the touch reset line (GPIO18) is intentionally left unconfigured. Driving it
// low locks up the I2C bus on this board; the FT6336U resets fine on its own at
// power-on, so the pin is simply never touched.
#define RG_GPIO_I2C_SDA             GPIO_NUM_16
#define RG_GPIO_I2C_SCL             GPIO_NUM_15

// SPI Display (LCD RST is bonded to CHIP_PU/EN on this board, so no RST pin)
#define RG_GPIO_LCD_MISO            GPIO_NUM_13
#define RG_GPIO_LCD_MOSI            GPIO_NUM_11
#define RG_GPIO_LCD_CLK             GPIO_NUM_12
#define RG_GPIO_LCD_CS              GPIO_NUM_10
#define RG_GPIO_LCD_DC              GPIO_NUM_46
#define RG_GPIO_LCD_BCKL            GPIO_NUM_45

// External I2S DAC (ES8311). Silkscreen names data pins from the codec's
// perspective: GPIO8 -> ES8311 DSDIN (playback), GPIO6 <- ES8311 ASDOUT (mic).
#define RG_GPIO_SND_I2S_BCK         5
#define RG_GPIO_SND_I2S_WS          7
#define RG_GPIO_SND_I2S_DATA        8
#define RG_GPIO_SND_I2S_MCK         4
// The NS4150 amplifier gate on this board is wired so that LOW enables the amp and
// HIGH shuts it down, which is the opposite of retro-go's default, hence the invert.
#define RG_GPIO_SND_AMP_ENABLE      1
#define RG_GPIO_SND_AMP_ENABLE_INVERT 1

// Bluetooth LE gamepad. The driver reads the controller's HID Report Map at runtime, so
// this table maps HID *usages* (button numbers as the controller declares them) rather
// than raw report bit offsets, and works with any BLE HID gamepad.
//
// The numbering below is for the Google Stadia Controller in Bluetooth mode, taken from
// its report descriptor (VID 0x18D1 / PID 0x9400). Note that its usages are not
// contiguous: 3, 6, 9, 10 and 16 are never declared.
//   1=A  2=B  4=X  5=Y  7=L1  8=R1  11=Options  12=Menu  13=Guide
//   14=L3  15=R3  17=Assistant  18=Capture
// The d-pad comes from the HID hat switch, L2/R2 from the Brake/Accelerator axes.
//
// For a different controller, watch the serial log: the driver prints the decoded button
// number whenever the state changes, so this table can be corrected without guesswork.
#define RG_GAMEPAD_BLE_MAP {\
    {RG_KEY_UP,     RG_BLE_DPAD_UP},\
    {RG_KEY_DOWN,   RG_BLE_DPAD_DOWN},\
    {RG_KEY_LEFT,   RG_BLE_DPAD_LEFT},\
    {RG_KEY_RIGHT,  RG_BLE_DPAD_RIGHT},\
    {RG_KEY_A,      RG_BLE_BUTTON(1)},\
    {RG_KEY_B,      RG_BLE_BUTTON(2)},\
    {RG_KEY_X,      RG_BLE_BUTTON(4)},\
    {RG_KEY_Y,      RG_BLE_BUTTON(5)},\
    {RG_KEY_L,      RG_BLE_BUTTON(7)},\
    {RG_KEY_R,      RG_BLE_BUTTON(8)},\
    {RG_KEY_SELECT, RG_BLE_BUTTON(11)},\
    {RG_KEY_START,  RG_BLE_BUTTON(12)},\
    {RG_KEY_MENU,   RG_BLE_BUTTON(13)},\
    {RG_KEY_OPTION, RG_BLE_BUTTON(17)},\
}
// Some controllers only advertise the HID service UUID in their scan response, so
// matching on the advertised name as well makes the first pairing more reliable.
#define RG_BLE_NAME_FILTER {"Stadia", "Gamepad", "Controller"}
