#ifndef _INCLUDE_KEYBOARD_H
#define _INCLUDE_KEYBOARD_H

#include "pce.h"

#if defined(SDL)
#include "osd_sdl_machine.h"
#endif // !SDL

typedef struct
{
	Sint16 axis[4];
	Sint16 button[16];
} js_status;

extern char auto_fire_A[5];
/* Is auto fire on */

extern char auto_fire_B[5];
/* Is auto fire on */

#endif
