// REF: https://wiki.libsdl.org/

// Target definition
#define RG_TARGET_NAME             "SDL2"

// Storage
#define RG_STORAGE_DRIVER           0   // 0 = Host, 1 = SDSPI, 2 = SDMMC, 3 = USB, 4 = Flash

// Audio
#define RG_AUDIO_USE_INT_DAC        0   // 0 = Disable, 1 = GPIO25, 2 = GPIO26, 3 = Both
#define RG_AUDIO_USE_EXT_DAC        0   // 0 = Disable, 1 = Enable
#define RG_AUDIO_USE_SDL2           0   // 0 = Disable, 1 = Enable

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
