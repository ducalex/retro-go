#ifndef CONTROLS_H
#define CONTROLS_H

#include "rg_input.h"
#include "SDL_input.h"
#include "SDL_scancode.h"

typedef enum {
    RG_MODE_ANY = 0,
    RG_MODE_GAME,
    RG_MODE_MENU
} rg_mode_t;

typedef struct {
    rg_key_t rg_key;
    SDL_Scancode scancode;
    SDL_Keycode keycode;
    rg_mode_t mode;
} key_mapping_t;

extern const key_mapping_t keymap[];
extern const size_t keymap_count;

extern const key_mapping_t shifted_keymap[];
extern const size_t shifted_keymap_count;

#endif
