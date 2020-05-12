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
** osd.h
**
** O/S dependent routine defintions (must be customized)
** $Id: osd.h,v 1.2 2001/04/27 14:37:11 neil Exp $
*/

#ifndef _OSD_H_
#define _OSD_H_

#include <limits.h>
#ifndef  PATH_MAX
#define  PATH_MAX    512
#endif /* PATH_MAX */

#ifdef __GNUC__
#define  __PACKED__  __attribute__ ((packed))

#ifdef __DJGPP__
#define  PATH_SEP    '\\'
#else /* !__DJGPP__ */
#define  PATH_SEP    '/'
#endif /* !__DJGPP__ */

#elif defined(WIN32)
#define  __PACKED__
#define  PATH_SEP    '\\'
#else /* !defined(WIN32) && !__GNUC__ */
#define  __PACKED__
#define  PATH_SEP    ':'
#endif /* !defined(WIN32) && !__GNUC__ */

#if !defined(WIN32) && !defined(__DJGPP__)
#define stricmp strcasecmp
#endif /* !WIN32 && !__DJGPP__ */


#include <noftypes.h>
#include <bitmap.h>

typedef struct vidinfo_s
{
   short default_width, default_height;
} vidinfo_t;

typedef struct sndinfo_s
{
   int sample_rate;
   int bps;
} sndinfo_t;

/* get info */
extern void osd_getvideoinfo(vidinfo_t *info);
extern void osd_getsoundinfo(sndinfo_t *info);

/* video */
extern void osd_setpalette(rgb_t *pal);
extern void osd_blitscreen(bitmap_t *bmp);

/* audio */
extern void osd_audioframe(int nsamples);

/* init / shutdown */
extern int osd_init(void);
extern void osd_shutdown(void);
extern int osd_main(int argc, char *argv[]);
extern void osd_loadstate();
extern void osd_event(int event);

/* filename manipulation */
extern void osd_fullname(char *fullname, const char *shortname);
extern char *osd_newextension(char *string, char *ext);

/* input */
extern void osd_getinput(void);

/* get rom data */
extern size_t osd_getromdata(unsigned char **data);

/* Log output */
extern void osd_logprint(int type, char *message);

#endif /* _OSD_H_ */
