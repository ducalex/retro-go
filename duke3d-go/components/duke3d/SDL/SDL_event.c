#include "SDL_event.h"
#include <rg_system.h>
#include "rg_input.h"
#include "SDL_scancode.h"
#include "controls.h"
#include "duke3d.h"
#include <string.h>

int keyMode = 1;

extern uint32_t SDL_GetTicks(void);

static uint32_t last_map_state[64]; 
static int current_map_idx = 0;

// Shift / Hotkey Logic
static uint32_t start_press_time = 0;
static bool shift_active = false;
static int use_pulse_stage = 0; 
static uint32_t pulse_start_time = 0;
static uint32_t last_shifted_state[16]; 
static int current_shifted_idx = 0;

static uint32_t rg_state_snapshot = 0;
static uint32_t masked_state = 0;
static bool in_menu_cached = false;

IRAM_ATTR int SDL_PollEvent(SDL_Event * event)
{
    if (event) memset(event, 0, sizeof(SDL_Event));
    uint32_t now = SDL_GetTicks();
    
    // Snapshot the state only at the start of a new polling session
    if (current_map_idx == 0 && current_shifted_idx == 0 && use_pulse_stage == 0) {
        rg_state_snapshot = rg_input_read_gamepad();

        // Check context (Cached for the duration of this polling cycle)
        extern struct player_struct ps[];
        extern short myconnectindex;
        in_menu_cached = (ps[myconnectindex].gm & 1);

        // 1. Handle START hotkey tracking
        if (!in_menu_cached) {
            if (rg_state_snapshot & RG_KEY_START) {
                if (start_press_time == 0) {
                    start_press_time = now;
                } else if (!shift_active && (now - start_press_time >= 500)) {
                    shift_active = true;
                    strcpy(fta_quotes[26], "Hotkey Active");
                    FTA(26, &ps[myconnectindex], 1);
                }
            } else {
                if (start_press_time != 0) {
                    if (!shift_active && (now - start_press_time < 500)) {
                        use_pulse_stage = 1;
                    }
                    start_press_time = 0;
                    if (shift_active) {
                        shift_active = false;
                        if (ps[myconnectindex].ftq == 26) {
                            ps[myconnectindex].fta = 0;
                        }
                    }
                }
            }
        } else {
            start_press_time = 0;
            if (shift_active) {
                shift_active = false;
                if (ps[myconnectindex].ftq == 26) {
                    ps[myconnectindex].fta = 0;
                }
            }
        }

        // Pre-calculate masked_state once per snapshot
        masked_state = rg_state_snapshot;
        if (!in_menu_cached) {
            masked_state &= ~RG_KEY_START;
            if (shift_active) {
                for (int i = 0; i < (int)shifted_keymap_count; i++) {
                    masked_state &= ~shifted_keymap[i].rg_key;
                }
            }
        }
    }

    // 2. Handle "Use" (START) pulse state machine
    if (use_pulse_stage == 1) {
        event->type = SDL_KEYDOWN;
        event->key.state = SDL_PRESSED;
        event->key.keysym.scancode = SDL_SCANCODE_SPACE;
        event->key.keysym.sym = SDLK_SPACE;
        use_pulse_stage = 2;
        pulse_start_time = now;
        return 1;
    }
    if (use_pulse_stage == 2) {
        if (now - pulse_start_time > 100) use_pulse_stage = 3;
    }
    if (use_pulse_stage == 3) {
        event->type = SDL_KEYUP;
        event->key.state = SDL_RELEASED;
        event->key.keysym.scancode = SDL_SCANCODE_SPACE;
        event->key.keysym.sym = SDLK_SPACE;
        use_pulse_stage = 0;
        return 1;
    }

    // 3. Handle Shifted mappings
    if (shift_active) {
        while (current_shifted_idx < (int)shifted_keymap_count && current_shifted_idx < 16) {
            const key_mapping_t *sm = &shifted_keymap[current_shifted_idx];
            bool is_pressed = (rg_state_snapshot & sm->rg_key);
            bool was_pressed = (bool)last_shifted_state[current_shifted_idx];

            if (is_pressed != was_pressed) {
                last_shifted_state[current_shifted_idx] = is_pressed;
                event->type = is_pressed ? SDL_KEYDOWN : SDL_KEYUP;
                event->key.state = is_pressed ? SDL_PRESSED : SDL_RELEASED;
                event->key.keysym.scancode = sm->scancode;
                event->key.keysym.sym = sm->keycode;
                current_shifted_idx++;
                return 1;
            }
            current_shifted_idx++;
        }
        current_shifted_idx = 0;
    } else {
        for (int i = 0; i < (int)shifted_keymap_count && i < 16; i++) {
            if (last_shifted_state[i]) {
                const key_mapping_t *sm = &shifted_keymap[i];
                last_shifted_state[i] = 0;
                event->type = SDL_KEYUP;
                event->key.state = SDL_RELEASED;
                event->key.keysym.scancode = sm->scancode;
                event->key.keysym.sym = sm->keycode;
                return 1;
            }
        }
    }

    // 4. Handle normal mappings
    while (current_map_idx < (int)keymap_count && current_map_idx < 64) {
        const key_mapping_t *m = &keymap[current_map_idx];
        
        if (m->mode == RG_MODE_GAME && in_menu_cached) { current_map_idx++; continue; }
        if (m->mode == RG_MODE_MENU && !in_menu_cached) { current_map_idx++; continue; }

        bool is_pressed = (masked_state & m->rg_key);
        bool was_pressed = (bool)last_map_state[current_map_idx];

        if (is_pressed != was_pressed) {
            last_map_state[current_map_idx] = is_pressed;
            event->type = is_pressed ? SDL_KEYDOWN : SDL_KEYUP;
            event->key.state = is_pressed ? SDL_PRESSED : SDL_RELEASED;
            event->key.keysym.scancode = m->scancode;
            event->key.keysym.sym = m->keycode;
            current_map_idx++;
            return 1;
        }
        current_map_idx++;
    }

    current_map_idx = 0;
    return 0;
}

void inputInit()
{
	printf("keyboard: retro-go input initialized with START hotkey support.\n");
}
