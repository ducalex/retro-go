/* This file is part of Snes9x. See LICENSE file. */

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

/* Routines the port specific code has to implement */
uint32_t S9xReadJoypad(int32_t port);
bool S9xReadMousePosition(int32_t which1_0_to_1, int32_t* x, int32_t* y, uint32_t* buttons);
bool S9xReadSuperScopePosition(int32_t* x, int32_t* y, uint32_t* buttons);

bool S9xInitDisplay(void);
void S9xDeinitDisplay(void);
void S9xToggleSoundChannel(int32_t channel);
void S9xNextController(void);
#endif
