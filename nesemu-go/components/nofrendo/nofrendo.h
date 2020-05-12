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
** nofrendo.h (c) 1998-2000 Matthew Conte (matt@conte.com)
**            (c) 2000 Neil Stevens (multivac@fcmail.com)
**
** Note: all architectures should call these functions
**
** $Id: nofrendo.h,v 1.2 2001/04/27 11:10:08 neil Exp $
*/

#ifndef _NOFRENDO_H_
#define _NOFRENDO_H_

#define  APP_STRING     "Nofrendo"
#define  APP_VERSION    "2.0"

/* Configuration */

/* Enable debugging messages */
// #define NOFRENDO_DEBUG

/* Enable live dissassembler */
// #define  NES6502_DISASM

/* End configuration */

#ifdef NOFRENDO_DEBUG
#define ASSERT(expr)        nofrendo_assert((int) (expr), __LINE__, __FILE__, NULL)
#define MESSAGE_DEBUG(x...) nofrendo_printf(0, __FUNCTION__, "> " x)
#else
#define ASSERT(expr)
#define MESSAGE_DEBUG(x...)
#endif
#define MESSAGE_INFO(x...)  nofrendo_printf(1, NULL, "* " x)
#define MESSAGE_ERROR(x...) nofrendo_printf(2, NULL, "!! " x)

#include <nes.h>
extern int nofrendo_start(const char *filename, int region, int sample_rate);
extern void nofrendo_stop(void);
extern void nofrendo_refresh(void);
extern void nofrendo_printf(int type, const char *prefix, const char *format, ...);
extern void nofrendo_assert(int expr, int line, const char *file, char *msg);

#endif /* !_NOFRENDO_H_ */
