// REF: https://wiki.libsdl.org/

// Target definition
#define RG_TARGET_NAME             "SDL2"

// Storage
#define RG_STORAGE_DRIVER           0       // 0 = Host, 1 = SDSPI, 2 = SDMMC, 3 = USB, 4 = Flash
#define RG_STORAGE_HOST             0       // Used by SDSPI and SDMMC
#define RG_STORAGE_SPEED            0       // Used by SDSPI and SDMMC
#define RG_STORAGE_ROOT             "."     // Storage mount point

// Audio
#define RG_AUDIO_USE_INT_DAC        0   // 0 = Disable, 1 = GPIO25, 2 = GPIO26, 3 = Both
#define RG_AUDIO_USE_EXT_DAC        0   // 0 = Disable, 1 = Enable
#define RG_AUDIO_USE_SDL2           1   // 0 = Disable, 1 = Enable

// Video
#define RG_SCREEN_DRIVER            0   // 0 = ILI9341
#define RG_SCREEN_HOST              0
#define RG_SCREEN_SPEED             0
#define RG_SCREEN_TYPE              0
#define RG_SCREEN_WIDTH             640
#define RG_SCREEN_HEIGHT            480
#define RG_SCREEN_ROTATE            0
#define RG_SCREEN_MARGIN_TOP        0
#define RG_SCREEN_MARGIN_BOTTOM     0
#define RG_SCREEN_MARGIN_LEFT       0
#define RG_SCREEN_MARGIN_RIGHT      0

// Input
#define RG_GAMEPAD_DRIVER           6   // 1 = ODROID-GO, 2 = Serial, 3 = I2C, 4 = QTPY, 5 = ESPLAY-S3, 6 = SDL2
#define RG_GAMEPAD_HAS_MENU_BTN     1
#define RG_GAMEPAD_HAS_OPTION_BTN   1
// Note: Depending on the driver, the button map can represent bits, registers, keys, or gpios.
#define RG_GAMEPAD_MAP_MENU         SDL_SCANCODE_ESCAPE
#define RG_GAMEPAD_MAP_OPTION       SDL_SCANCODE_TAB
#define RG_GAMEPAD_MAP_START        SDL_SCANCODE_SPACE
#define RG_GAMEPAD_MAP_SELECT       SDL_SCANCODE_0
#define RG_GAMEPAD_MAP_UP           SDL_SCANCODE_UP
#define RG_GAMEPAD_MAP_RIGHT        SDL_SCANCODE_RIGHT
#define RG_GAMEPAD_MAP_DOWN         SDL_SCANCODE_DOWN
#define RG_GAMEPAD_MAP_LEFT         SDL_SCANCODE_LEFT
#define RG_GAMEPAD_MAP_A            SDL_SCANCODE_Z
#define RG_GAMEPAD_MAP_B            SDL_SCANCODE_X
#define RG_GAMEPAD_MAP_X            SDL_SCANCODE_A
#define RG_GAMEPAD_MAP_Y            SDL_SCANCODE_S

// Battery
// #define RG_BATTERY_ADC_CHANNEL      ADC1_CHANNEL_0
// #define RG_BATTERY_CALC_PERCENT(raw) (((raw) * 2.f - 3500.f) / (4200.f - 3500.f) * 100.f)
// #define RG_BATTERY_CALC_VOLTAGE(raw) ((raw) * 2.f * 0.001f)
