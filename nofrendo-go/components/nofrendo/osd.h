/*
** Nofrendo (c) 1998-2000 Matthew Conte (matt@conte.com)
**
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of version 2 of the GNU Library General
** Public License as published by the Free Software Foundation.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.  To obtain a
** copy of the GNU Library General Public License, write to the Free
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
**
**
** osd.h: O/S dependent routine defintions (must be implemented)
**
*/

#ifndef _OSD_H_
#define _OSD_H_

#include <nofrendo.h>

/* video */
extern void osd_setpalette(rgb_t *pal);
extern void osd_blitscreen(uint8 *bmp);

/* control */
extern int osd_init(void);
extern void osd_shutdown(void);
extern void osd_loadstate(void);
extern void osd_event(int event);
extern void osd_vsync(void);

/* input */
extern void osd_getinput(void);

/* Log output, printf-style format */
extern void osd_log(int type, const char *format, ...);

#endif /* _OSD_H_ */
