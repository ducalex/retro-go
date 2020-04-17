#ifndef _INCLUDE_KEYBOARD_H
#define _INCLUDE_KEYBOARD_H

#include "cleantypes.h"
#include "pce.h"
#include "sys_dep.h"

#define JOY_A       0x01
#define JOY_B       0x02
#define JOY_SELECT  0x04
#define JOY_RUN     0x08
#define JOY_UP      0x10
#define JOY_RIGHT   0x20
#define JOY_DOWN    0x40
#define JOY_LEFT    0x80

typedef struct
{
	Sint16 axis[4];
	Sint16 button[16];
} js_status;

extern char auto_fire_A[5];
/* Is auto fire on */

extern char auto_fire_B[5];
/* Is auto fire on */


int osd_keyboard(void);

#endif
