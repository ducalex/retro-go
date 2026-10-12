#include "quakedef.h"
#include "rg_system.h"
#include "music_esp32.h"

static uint32_t gamepad_state = 0;
static uint32_t prev_gamepad = 0;
static uint32_t suppressed_buttons = 0;
static keydest_t previous_key_dest = key_game;

#define SYSTEM_MENU_HOLD_US  500000
#define ENGINE_TAP_US         35000

typedef struct
{
    bool down;
    bool long_press;
    bool release_pending;
    int64_t pressed_at;
    int64_t release_at;
} system_button_state_t;

static system_button_state_t menu_button;
static system_button_state_t option_button;

static void release_quake_keys(void)
{
    Key_Event(K_UPARROW, false);
    Key_Event(K_DOWNARROW, false);
    Key_Event(K_LEFTARROW, false);
    Key_Event(K_RIGHTARROW, false);
    Key_Event(K_ESCAPE, false);
    Key_Event(K_ENTER, false);
    Key_Event(K_CTRL, false);
    Key_Event(K_SPACE, false);
    Key_Event('c', false);
}

static void cancel_system_buttons(void)
{
    memset(&menu_button, 0, sizeof(menu_button));
    memset(&option_button, 0, sizeof(option_button));
}

static void start_native_tap(int key, system_button_state_t *state, int64_t now)
{
    Key_Event(key, true);
    state->release_pending = true;
    state->release_at = now + ENGINE_TAP_US;
}

static void finish_native_tap(int key, system_button_state_t *state, int64_t now)
{
    if (state->release_pending && now >= state->release_at)
    {
        Key_Event(key, false);
        state->release_pending = false;
    }
}

static void open_retrogo_menu(bool options)
{
    release_quake_keys();
    cancel_system_buttons();
    gamepad_state = 0;
    prev_gamepad = 0;

    int64_t start = rg_system_timer();
    Music_MenuPause(true);
    if (options)
        rg_gui_options_menu();
    else
        rg_gui_game_menu();
    Music_MenuPause(false);
    Sys_ExcludePauseTime((uint32_t)(rg_system_timer() - start));

    // A button used to close the dialog must not leak into gameplay. Polling
    // here is deliberately outside the emulation iteration's normal sample.
    suppressed_buttons = rg_input_read_gamepad();
    previous_key_dest = key_dest;
}

static bool handle_system_button(uint32_t gamepad, uint32_t mask, int native_key,
                                 system_button_state_t *state, bool options,
                                 int64_t now)
{
    bool pressed = (gamepad & mask) != 0;

    if (pressed && !state->down)
    {
        state->down = true;
        state->long_press = false;
        state->pressed_at = now;
    }

    if (pressed && !state->long_press &&
        now - state->pressed_at >= SYSTEM_MENU_HOLD_US)
    {
        state->long_press = true;
        open_retrogo_menu(options);
        return true;
    }

    if (!pressed && state->down)
    {
        if (!state->long_press && native_key != 0)
            start_native_tap(native_key, state, now);
        state->down = false;
        state->long_press = false;
    }

    return false;
}

void IN_Init(void)
{
    gamepad_state = 0;
    prev_gamepad = 0;
    cancel_system_buttons();
    suppressed_buttons = rg_input_read_gamepad();
    previous_key_dest = key_dest;
}

void IN_Shutdown(void)
{
    release_quake_keys();
    cancel_system_buttons();
    gamepad_state = 0;
}

void IN_Commands(void)
{
    uint32_t raw_gamepad = rg_input_read_gamepad();

    // Keep buttons held across startup or a blocking system dialog suppressed
    // until their physical release.
    suppressed_buttons &= raw_gamepad;
    uint32_t gamepad = raw_gamepad & ~suppressed_buttons;
    int64_t now = rg_system_timer();

    finish_native_tap(K_ESCAPE, &menu_button, now);

    if (key_dest != previous_key_dest)
    {
        release_quake_keys();
        previous_key_dest = key_dest;
    }

    if (handle_system_button(gamepad, RG_KEY_MENU, K_ESCAPE,
                             &menu_button, false, now))
        return;
    if (handle_system_button(gamepad, RG_KEY_OPTION, 0,
                             &option_button, true, now))
        return;

    // MENU and OPTION have been consumed by the short/long-press state
    // machines and must not also pass through the regular edge mapper.
    gamepad &= ~(RG_KEY_MENU | RG_KEY_OPTION);
    uint32_t changed = gamepad ^ prev_gamepad;

    if (changed)
    {
        // Use Quake's native arrow bindings for both gameplay and menus. This
        // is the sole D-pad path, avoiding the former duplicate movement.
        if (changed & RG_KEY_UP)    Key_Event(K_UPARROW, (gamepad & RG_KEY_UP) != 0);
        if (changed & RG_KEY_DOWN)  Key_Event(K_DOWNARROW, (gamepad & RG_KEY_DOWN) != 0);
        if (changed & RG_KEY_LEFT)  Key_Event(K_LEFTARROW, (gamepad & RG_KEY_LEFT) != 0);
        if (changed & RG_KEY_RIGHT) Key_Event(K_RIGHTARROW, (gamepad & RG_KEY_RIGHT) != 0);

        // Swim down is a held command, so it belongs on a regular button and
        // not OPTION's deferred short/long-press path. Short MENU remains the
        // Quake-menu control.
        uint32_t swim_down_buttons = RG_KEY_START | RG_KEY_X;
        if (changed & swim_down_buttons)
        {
            bool was_down = (prev_gamepad & swim_down_buttons) != 0;
            bool is_down = (gamepad & swim_down_buttons) != 0;
            if (was_down != is_down)
                Key_Event('c', is_down);
        }

        // Weapon cycling is available on the universal SELECT button. Y is
        // an optional, more conveniently placed duplicate on larger pads.
        if (changed & gamepad & (RG_KEY_SELECT | RG_KEY_Y))
            Cbuf_AddText("impulse 10\n");

        // A: fire in game, Enter/yes in native dialogs.
        if (changed & RG_KEY_A)
        {
            bool down = (gamepad & RG_KEY_A) != 0;
            if (key_dest == key_game)
            {
                Key_Event(K_CTRL, down);
            }
            else
            {
                Key_Event(K_ENTER, down);
                if (down)
                {
                    Key_Event('y', true);
                    Key_Event('y', false);
                }
            }
        }

        // B: jump in game, Escape/no in native dialogs.
        if (changed & RG_KEY_B)
        {
            bool down = (gamepad & RG_KEY_B) != 0;
            if (key_dest == key_game)
            {
                Key_Event(K_SPACE, down);
            }
            else
            {
                Key_Event(K_ESCAPE, down);
                if (down)
                {
                    Key_Event('n', true);
                    Key_Event('n', false);
                }
            }
        }
    }

    gamepad_state = gamepad;
    prev_gamepad = gamepad;
}

void IN_Move(usercmd_t *cmd)
{
    if (key_dest != key_game)
        return;

    // Shoulder strafing is the only direct command movement; the D-pad uses
    // Quake's key bindings above so angle adjustment occurs in cl.viewangles.
    if (gamepad_state & RG_KEY_L) cmd->sidemove -= 200;
    if (gamepad_state & RG_KEY_R) cmd->sidemove += 200;
}
