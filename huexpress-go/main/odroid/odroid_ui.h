#pragma once

#include <stdint.h>
#include <string.h>
#include "esp_system.h"
#include "odroid_input.h"
#include "odroid_config_emu.h"
#include "colors_rgb565.h"

#ifndef ODROID_UI_CALLCONV
#  if defined(__GNUC__) && defined(__i386__) && !defined(__x86_64__)
#    define ODROID_UI_CALLCONV __attribute__((cdecl))
#  elif defined(_MSC_VER) && defined(_M_X86) && !defined(_M_X64)
#    define ODROID_UI_CALLCONV __cdecl
#  else
#    define ODROID_UI_CALLCONV /* all other platforms only have one calling convention each */
#  endif
#endif

#ifndef RETRO_API
#  if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#    ifdef RETRO_IMPORT_SYMBOLS
#      ifdef __GNUC__
#        define RETRO_API ODROID_UI_CALLCONV __attribute__((__dllimport__))
#      else
#        define RETRO_API ODROID_UI_CALLCONV __declspec(dllimport)
#      endif
#    else
#      ifdef __GNUC__
#        define RETRO_API ODROID_UI_CALLCONV __attribute__((__dllexport__))
#      else
#        define RETRO_API ODROID_UI_CALLCONV __declspec(dllexport)
#      endif
#    endif
#  else
#      if defined(__GNUC__) && __GNUC__ >= 4 && !defined(__CELLOS_LV2__)
#        define RETRO_API ODROID_UI_CALLCONV __attribute__((__visibility__("default")))
#      else
#        define RETRO_API ODROID_UI_CALLCONV
#      endif
#  endif
#endif

typedef enum
{
    ODROID_UI_FUNC_TOGGLE_RC_NOTHING = 0,
    ODROID_UI_FUNC_TOGGLE_RC_CHANGED = 1,
    ODROID_UI_FUNC_TOGGLE_RC_MENU_RESTART = 2,
    ODROID_UI_FUNC_TOGGLE_RC_MENU_CLOSE = 3,
} odroid_ui_func_toggle_rc;

typedef struct odroid_ui_entry odroid_ui_entry;
typedef struct odroid_ui_window odroid_ui_window;
typedef void (ODROID_UI_CALLCONV *odroid_ui_func_update_def)(odroid_ui_entry *entry);
typedef odroid_ui_func_toggle_rc (ODROID_UI_CALLCONV *odroid_ui_func_toggle_def)(odroid_ui_entry *entry, odroid_gamepad_state *joystick);
typedef void (ODROID_UI_CALLCONV *odroid_ui_func_window_init_def)(odroid_ui_window *entry);

#define color_default C_GRAY
#define color_selected C_WHITE
#define color_bg_default C_NAVY

typedef struct odroid_ui_entry
{
    uint16_t x;
    uint16_t y;
    char text[64];
    
    char data[64];
    
    odroid_ui_func_update_def func_update;
    odroid_ui_func_toggle_def func_toggle;
} odroid_ui_entry;

typedef struct odroid_ui_window
{
    uint16_t x;
    uint16_t y;
    
    uint8_t width;
    uint8_t height;
    
    uint8_t entry_count;
    odroid_ui_entry* entries[16];
} odroid_ui_window;


#ifdef ODROID_UI_MENU_CONFIG_SPEEDUP
extern bool config_speedup;
#endif

extern bool forceConsoleReset;
extern bool odroid_ui_menu_opened;

void odroid_ui_clean_draw_buffer();
void odroid_ui_draw_chars(uint16_t x, uint16_t y, uint16_t width, char *text, uint16_t color, uint16_t color_bg);
void odroid_ui_wait_for_key(int key, bool pressed);
bool odroid_ui_menu(bool restart_menu);
bool odroid_ui_menu_ext(bool restart_menu, odroid_ui_func_window_init_def func_window_init);
void odroid_ui_init();
void odroid_ui_enter_loop();
bool odroid_ui_ask(const char *text);
int odroid_ui_ask_v2(const char *text, uint16_t color_fg, uint16_t color_bg, int selected_initial);
bool odroid_ui_error(const char *text);
void odroid_ui_battery_draw(int x, int y, int max, int value);

#ifdef ODROID_UI_EMU_SAVE
void QuickSaveSetBuffer(void* data);
extern bool QuickLoadState(FILE *f);
extern bool QuickSaveState(FILE *f);

void LoadState();
void SaveState();
#endif

odroid_ui_entry *odroid_ui_create_entry(odroid_ui_window *window, odroid_ui_func_update_def func_update, odroid_ui_func_toggle_def func_toggle);

void check_boot_cause();
void DoReboot(bool save);
void DoStartupPost();

#define ODROID_UI_MENU_HANDLER_INIT_V1(joy) \
    odroid_ui_enter_loop(); \
    bool menu_restart = false; \
    bool ignoreMenuButton = joy.values[ODROID_INPUT_MENU]; \
    uint16_t menuButtonFrameCount = 0;

#define ODROID_UI_MENU_HANDLER_LOOP_V1(joy_last, joy, CALLBACK_SHUTDOWN, CALLBACK_MENU) \
        if (ignoreMenuButton) \
        { \
            ignoreMenuButton = joy_last.values[ODROID_INPUT_MENU]; \
        } \
 \
        if (!ignoreMenuButton && joy_last.values[ODROID_INPUT_MENU] && joy.values[ODROID_INPUT_MENU]) \
        { \
            ++menuButtonFrameCount; \
        } \
        else \
        { \
            menuButtonFrameCount = 0; \
        } \
        if (menuButtonFrameCount > 60 * 1) \
        { \
            CALLBACK_SHUTDOWN(true); \
        } \
         \
        if (!ignoreMenuButton && joy_last.values[ODROID_INPUT_MENU] && !joy.values[ODROID_INPUT_MENU]) \
        { \
            CALLBACK_SHUTDOWN(false); \
        } \
        if ((!joy_last.values[ODROID_INPUT_VOLUME] && \
            joy.values[ODROID_INPUT_VOLUME]) || menu_restart) \
        { \
            menu_restart = CALLBACK_MENU(menu_restart); \
        }

#define ODROID_UI_MENU_HANDLER_LOOP_V2(joy_last, joy, CALLBACK_SHUTDOWN) \
        if (ignoreMenuButton) \
        { \
            ignoreMenuButton = joy_last.values[ODROID_INPUT_MENU]; \
        } \
 \
        if (!ignoreMenuButton && joy_last.values[ODROID_INPUT_MENU] && joy.values[ODROID_INPUT_MENU]) \
        { \
            ++menuButtonFrameCount; \
        } \
        else \
        { \
            menuButtonFrameCount = 0; \
        } \
        if (menuButtonFrameCount > 60 * 1) \
        { \
            CALLBACK_SHUTDOWN(true); \
        } \
         \
        if (!ignoreMenuButton && joy_last.values[ODROID_INPUT_MENU] && !joy.values[ODROID_INPUT_MENU]) \
        { \
            CALLBACK_SHUTDOWN(false); \
        }

#define ODROID_UI_MENU_HANDLER_LOOP_V1_EXT(joy_last, joy, CALLBACK_SHUTDOWN, MENU_INIT) \
        if (ignoreMenuButton) \
        { \
            ignoreMenuButton = joy_last.values[ODROID_INPUT_MENU]; \
        } \
 \
        if (!ignoreMenuButton && joy_last.values[ODROID_INPUT_MENU] && joy.values[ODROID_INPUT_MENU]) \
        { \
            ++menuButtonFrameCount; \
        } \
        else \
        { \
            menuButtonFrameCount = 0; \
        } \
        if (menuButtonFrameCount > 60 * 1) \
        { \
            CALLBACK_SHUTDOWN(true); \
        } \
         \
        if (!ignoreMenuButton && joy_last.values[ODROID_INPUT_MENU] && !joy.values[ODROID_INPUT_MENU]) \
        { \
            CALLBACK_SHUTDOWN(false); \
        } \
        if ((!joy_last.values[ODROID_INPUT_VOLUME] && \
            joy.values[ODROID_INPUT_VOLUME]) || menu_restart) \
        { \
            menu_restart = odroid_ui_menu_ext(menu_restart, &MENU_INIT); \
        }

#ifdef ODROID_UI_MENU_BATTERY

#define ODROID_UI_BATTERY_INIT                      \
    int battery_counter = 1000;                     \
    int battery_percentage_old = 0;                 \ 
    odroid_battery_state battery_state;             \
    bool battery_draw = false;                      \
    odroid_input_battery_level_read(&battery_state);

#ifdef DEBUG_TEST_BATTERY
#define ODROID_UI_BATTERY_CALC                                                            \
    if (battery_counter++>=20)                                                            \
    {                                                                                     \
        if (++battery_state.percentage>100) battery_state.percentage = 0;                 \
        printf("HEAP:0x%x, BATTERY:%d [%d]\n", esp_get_free_heap_size(),                  \
               battery_state.millivolts, battery_state.percentage);                       \
        battery_counter = 0;                                                              \
        int tmp = ((battery_state.percentage-1)/5) + 1;                                   \
        if (battery_percentage_old != tmp)                                                \
        {                                                                                 \
            battery_percentage_old = tmp;                                                 \
            battery_draw = true;                                                          \
        }                                                                                 \
    } else if (battery_percentage_old<3 && (battery_counter%3==0)) battery_draw = true;
#else
#define ODROID_UI_BATTERY_CALC                                                            \
    if (battery_counter++>=100)                                                           \
    {                                                                                     \
        odroid_input_battery_level_read(&battery_state);                                  \
        printf("HEAP:0x%x, BATTERY:%d [%d]\n", esp_get_free_heap_size(),                  \
               battery_state.millivolts, battery_state.percentage);                       \
        battery_counter = 0;                                                              \
        int tmp = ((battery_state.percentage-1)/5) + 1;                                   \
        if (battery_percentage_old != tmp)                                                \
        {                                                                                 \
            battery_percentage_old = tmp;                                                 \
            battery_draw = true;                                                          \
        }                                                                                 \
    } else if (battery_percentage_old<3 && (battery_counter%3==0)) battery_draw = true;
#endif
#define ODROID_UI_BATTERY_DRAW(x,y)                                \
    if (battery_draw)                                              \
    {                                                              \
        odroid_ui_battery_draw(x ,y, 20, battery_percentage_old);  \
        battery_draw = false;                                      \
    }

#else
#define ODROID_UI_BATTERY_INIT
#define ODROID_UI_BATTERY_CALC
#define ODROID_UI_BATTERY_DRAW(x,y)
#endif
