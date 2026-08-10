/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2012 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/

#ifndef _SDL_config_minimal_h
#define _SDL_config_minimal_h

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

/* Minimal configuration for Retro-Go */

/* Enable the dummy audio driver */
#define SDL_AUDIO_DRIVER_DUMMY	1

/* Enable the stub cdrom driver */
#define SDL_CDROM_DISABLED	1

/* Enable the stub joystick driver */
#define SDL_JOYSTICK_DISABLED	1

/* Enable the stub shared object loader */
#define SDL_LOADSO_DISABLED	1

/* Enable the stub thread support */
#define SDL_THREADS_DISABLED	1

/* Enable the stub timer support */
#define SDL_TIMERS_DISABLED	1

/* Enable the dummy video driver */
#define SDL_VIDEO_DRIVER_DUMMY	1

#endif /* _SDL_config_minimal_h */
