#ifndef SDL_h_
#define SDL_h_
#include <rg_system.h>
#include <stdint.h>
#include <stdio.h>
#include "SDL_input.h"
#include "SDL_stdinc.h"
#include "SDL_endian.h"
#include "SDL_audio.h"
#include "SDL_video.h"
#include "SDL_event.h"
#include "SDL_system.h"
#include "SDL_scancode.h"

typedef int SDLMod;
//typedef int SDL_Joystick;

#define SDLK_FIRST 0
#define SDLK_LAST 1024

#define SDLK_KP7 SDLK_KP_7
#define SDLK_KP8 SDLK_KP_8
#define SDLK_KP9 SDLK_KP_9
#define SDLK_KP4 SDLK_KP_4
#define SDLK_KP5 SDLK_KP_5
#define SDLK_KP6 SDLK_KP_6
#define SDLK_KP1 SDLK_KP_1
#define SDLK_KP2 SDLK_KP_2
#define SDLK_KP3 SDLK_KP_3
#define SDLK_KP0 SDLK_KP_0
#define SDLK_NUMLOCK SDLK_NUMLOCKCLEAR
#define SDLK_SCROLLOCK SDLK_SCROLLLOCK

#define SDLK_PRINT 0

#define AUDIO_S16SYS 16
#define AUDIO_S8 8

#define SDL_BUTTON_LEFT 0
#define SDL_BUTTON_RIGHT 1
#define SDL_BUTTON_MIDDLE 2

#define SDL_strlcpy strlcpy

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
