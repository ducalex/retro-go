#pragma once

#include "emulators.h"
#include "odroid_input.h"

void gui_header_draw(retro_emulator_t *emu);
void gui_cover_draw(retro_emulator_t *emu, odroid_gamepad_state *joystick);
void gui_list_draw(retro_emulator_t *emu, int color_shift);
bool gui_list_handle_input(retro_emulator_t *emu, odroid_gamepad_state *joystick, int *last_key);
retro_emulator_file_t *gui_list_selected_file(retro_emulator_t *emu);
