#pragma once
#include "debug.h"
#include "sys_dep.h"
#include "pce.h"

#if 0

#include <SDL.h>
#include <SDL_keyboard.h>


#include "osd_sdl_gfx.h"


#ifndef _INCLUDE_OSD_INIT_MACHINE
#define _INCLUDE_OSD_INIT_MACHINE

extern uchar gamepad;
// gamepad detected ?

extern char dump_snd;
// Do we write sound to file

extern int *fd[4];
// handle for joypad devices

extern int test_parameter;

#if defined(ENABLE_NETPLAY)
#include "netplay.h"
#endif

#endif // end _INCLUDE_OSD_INIT_MACHINE

#endif
