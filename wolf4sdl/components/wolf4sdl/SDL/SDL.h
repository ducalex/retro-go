#ifndef SDL_h_
#define SDL_h_
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include "SDL_system.h"

#include "SDL_input.h"
#include "SDL_stdinc.h"
#include "SDL_endian.h"
#include "SDL_audio.h"
#include "SDL_wave.h"

#include "SDL_video.h"
#include "SDL_event.h"

#include "SDL_scancode.h"
#include "SDL_blit.h"
#include "SDL_event.h"

typedef int SDLMod;
//typedef int SDL_Joystick;

#define PLATFORM_ESP32 1

#define SDLK_FIRST 0
#define SDLK_LAST 1024

#define SDLK_KP4 SDLK_LEFT
#define SDLK_KP6 SDLK_RIGHT
#define SDLK_KP8 SDLK_UP
#define SDLK_KP2 SDLK_DOWN
#define SDLK_KP5 SDLK_BACKSLASH
#define SDLK_KP0 SDLK_RETURN
#define SDLK_KP9 SDLK_PAGEUP
#define SDLK_KP3 SDLK_PAGEDOWN

#define SDLK_KP7 0
#define SDLK_KP1 0
#define SDLK_PRINT 0

#define SDL_BUTTON_LEFT 0
#define SDL_BUTTON_RIGHT 1
#define SDL_BUTTON_MIDDLE 2
#define SDLK_NUMLOCK 999
#define SDLK_SCROLLOCK 1000

#define SDL_strlcpy strlcpy
#define SDL_snprintf snprintf

#define SDL_INIT_TIMER          0x00000001u
#define SDL_INIT_AUDIO          0x00000010u
#define SDL_INIT_VIDEO          0x00000020u  /**< SDL_INIT_VIDEO implies SDL_INIT_EVENTS */
#define SDL_INIT_JOYSTICK       0x00000200u  /**< SDL_INIT_JOYSTICK implies SDL_INIT_EVENTS */
#define SDL_INIT_HAPTIC         0x00001000u
#define SDL_INIT_GAMECONTROLLER 0x00002000u  /**< SDL_INIT_GAMECONTROLLER implies SDL_INIT_JOYSTICK */
#define SDL_INIT_EVENTS         0x00004000u
#define SDL_INIT_SENSOR         0x00008000u
#define SDL_INIT_NOPARACHUTE    0x00100000u  /**< compatibility; this flag is ignored. */
#define SDL_INIT_EVERYTHING ( \
                SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS | \
                SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER | SDL_INIT_SENSOR \
            )

#endif
