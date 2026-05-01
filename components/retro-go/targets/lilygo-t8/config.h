// Target definition
#define RG_TARGET_NAME             "LILYGO-T8"

// Storage - SD card via SPI
#define RG_STORAGE_SDSPI_HOST      SPI3_HOST
#define RG_STORAGE_SDSPI_SPEED     SDMMC_FREQ_DEFAULT
#define RG_STORAGE_ROOT            "/sd"

// Audio - Internal DAC (GPIO25 mono, or GPIO25+26 stereo)
#define RG_AUDIO_USE_INT_DAC       3                    // 0 = Disable, 1 = GPIO25, 2 = GPIO26, 3 = Both
#define RG_AUDIO_USE_EXT_DAC       0                    // 0 = Disable, 1 = Enable

// Video - ILI9341 display 240x320
#define RG_SCREEN_DRIVER           0                    // 0 = ILI9341
#define RG_SCREEN_HOST             SPI2_HOST            // VSPI (shared with SD card)
#define RG_SCREEN_SPEED            SPI_MASTER_FREQ_40M  // 40 MHz SPI clock
#define RG_SCREEN_TYPE             1                    // 0 = ILI9341 standard
#define RG_SCREEN_WIDTH            320
#define RG_SCREEN_HEIGHT           240
#define RG_SCREEN_ROTATE           0                    // 0 = landscape, adjust if needed
#define RG_SCREEN_MARGIN_TOP       0
#define RG_SCREEN_MARGIN_BOTTOM    0
#define RG_SCREEN_MARGIN_LEFT      0
#define RG_SCREEN_MARGIN_RIGHT     0
#define RG_SCREEN_ROTATION         7
#define RG_SCREEN_RGB_BGR          0
#define RG_SCREEN_INIT()           {}

// Input - GPIO direct (active low buttons)
#define RG_GAMEPAD_DRIVER          1                    // 1 = GPIO direct
#define RG_GAMEPAD_HAS_MENU_BTN   1
#define RG_GAMEPAD_HAS_OPTION_BTN 0

// Button mapping (active low, directly read GPIO)
// NOTE: GPIO 34-39 are input-only and have NO internal pull-up.
//       You MUST use external 10kΩ pull-up resistors on these pins!
#define RG_GAMEPAD_MAP {\
    {RG_KEY_UP,     RG_GPIO_GAMEPAD_UP},     \
    {RG_KEY_DOWN,   RG_GPIO_GAMEPAD_DOWN},   \
    {RG_KEY_LEFT,   RG_GPIO_GAMEPAD_LEFT},   \
    {RG_KEY_RIGHT,  RG_GPIO_GAMEPAD_RIGHT},  \
    {RG_KEY_A,      RG_GPIO_GAMEPAD_A},      \
    {RG_KEY_B,      RG_GPIO_GAMEPAD_B},      \
    {RG_KEY_START,  RG_GPIO_GAMEPAD_START},  \
    {RG_KEY_SELECT, RG_GPIO_GAMEPAD_SELECT}, \
    {RG_KEY_MENU,   RG_GPIO_GAMEPAD_MENU},   \
}

// --- GPIO Pin Assignments ---
// IMPORTANT for ESP32-WROVER:
//   GPIO 6-11  -> SPI Flash (DO NOT USE)
//   GPIO 16-17 -> PSRAM     (DO NOT USE)
//   GPIO 34-39 -> Input only, no internal pull-up

// SPI Display (ILI9341)
#define RG_GPIO_LCD_MISO           GPIO_NUM_NC          // Not needed (display is write-only)
#define RG_GPIO_LCD_MOSI           GPIO_NUM_23
#define RG_GPIO_LCD_CLK            GPIO_NUM_18
#define RG_GPIO_LCD_CS             GPIO_NUM_5
#define RG_GPIO_LCD_DC             GPIO_NUM_21
// #define RG_GPIO_LCD_BCKL           GPIO_NUM_14
#define RG_GPIO_LCD_RST            GPIO_NUM_4

// SPI SD Card (shares MOSI/CLK with display, separate CS)
#define RG_GPIO_SDSPI_MISO        GPIO_NUM_2
#define RG_GPIO_SDSPI_MOSI        GPIO_NUM_15
#define RG_GPIO_SDSPI_CLK         GPIO_NUM_14
#define RG_GPIO_SDSPI_CS          GPIO_NUM_13

// D-Pad (using input-only pins - NEED external pull-up resistors!)
#define RG_GPIO_GAMEPAD_UP         GPIO_NUM_34
#define RG_GPIO_GAMEPAD_DOWN       GPIO_NUM_35
#define RG_GPIO_GAMEPAD_LEFT       GPIO_NUM_36
#define RG_GPIO_GAMEPAD_RIGHT      GPIO_NUM_39

// Action buttons (using regular GPIO with internal pull-up)
#define RG_GPIO_GAMEPAD_A          GPIO_NUM_32
#define RG_GPIO_GAMEPAD_B          GPIO_NUM_33
#define RG_GPIO_GAMEPAD_START      GPIO_NUM_27
#define RG_GPIO_GAMEPAD_SELECT     GPIO_NUM_0
#define RG_GPIO_GAMEPAD_MENU       GPIO_NUM_13
