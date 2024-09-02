// Target definition
#define RG_TARGET_NAME             "SDL2"

// Storage
#define RG_STORAGE_ROOT             "./sd"  // Storage mount point

// Audio
#define RG_AUDIO_USE_INT_DAC        0   // 0 = Disable, 1 = GPIO25, 2 = GPIO26, 3 = Both
#define RG_AUDIO_USE_EXT_DAC        0   // 0 = Disable, 1 = Enable
#define RG_AUDIO_USE_SDL2           1   // 0 = Disable, 1 = Enable

// Video
#define RG_SCREEN_DRIVER            99   // 0 = ILI9341
#define RG_SCREEN_HOST              0
#define RG_SCREEN_SPEED             0
#define RG_SCREEN_BACKLIGHT         1
#define RG_SCREEN_WIDTH             320
#define RG_SCREEN_HEIGHT            240
#define RG_SCREEN_ROTATE            0
#define RG_SCREEN_MARGIN_TOP        0
#define RG_SCREEN_MARGIN_BOTTOM     0
#define RG_SCREEN_MARGIN_LEFT       0
#define RG_SCREEN_MARGIN_RIGHT      0
#define RG_SCREEN_INIT()

// Input
// Refer to rg_input.h to see all available RG_KEY_* and RG_GAMEPAD_*_MAP types
#define RG_GAMEPAD_KBD_MAP {\
    {RG_KEY_UP,     SDL_SCANCODE_UP},\
    {RG_KEY_RIGHT,  SDL_SCANCODE_RIGHT},\
    {RG_KEY_DOWN,   SDL_SCANCODE_DOWN},\
    {RG_KEY_LEFT,   SDL_SCANCODE_LEFT},\
    {RG_KEY_SELECT, SDL_SCANCODE_0},\
    {RG_KEY_START,  SDL_SCANCODE_SPACE},\
    {RG_KEY_MENU,   SDL_SCANCODE_ESCAPE},\
    {RG_KEY_OPTION, SDL_SCANCODE_TAB},\
    {RG_KEY_A,      SDL_SCANCODE_X},\
    {RG_KEY_B,      SDL_SCANCODE_Z},\
    {RG_KEY_X,      SDL_SCANCODE_S},\
    {RG_KEY_Y,      SDL_SCANCODE_A},\
    {RG_KEY_L,      SDL_SCANCODE_Q},\
    {RG_KEY_R,      SDL_SCANCODE_W},\
}

// Battery
// #define RG_BATTERY_ADC_CHANNEL      ADC1_CHANNEL_0
// #define RG_BATTERY_CALC_PERCENT(raw) (((raw) * 2.f - 3500.f) / (4200.f - 3500.f) * 100.f)
// #define RG_BATTERY_CALC_VOLTAGE(raw) ((raw) * 2.f * 0.001f)

#if !defined(__VERSION__) && defined(__TINYC__)
#define __VERSION__ "TinyC"
#endif

#undef app_main
#define app_main(...) main(int argc, char **argv)
// #define rg_system_init(a, b, c) rg_system_init(argc, argv, a, b, c)
