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
** nofrendo.c
**
** Entry point of program
** Note: all architectures should call these functions
** $Id: nofrendo.c,v 1.3 2001/04/27 14:37:11 neil Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <nofrendo.h>
#include <osd.h>
#include <nes.h>


/* our happy little timer ISR */
volatile int nofrendo_ticks = 0;
// static void timer_isr(void)
// {
//    nofrendo_ticks++;
// }

// apu_setchan(chan, chan_enabled[chan]);
// apu_setfilter(filter_type);

void nofrendo_printf(int type, const char *prefix, const char *format, ...)
{
   static char buffer[512];
   va_list arg;
   va_start(arg, format);
   vsprintf(buffer, format, arg);

   if (type > 0) {
      // gui_sendmsg();
   }

   if (prefix) {

   }

   osd_logprint(0, buffer);

   va_end(arg);
}

void nofrendo_assert(int expr, int line, const char *file, char *msg)
{
   if (expr)
      return;

   if (NULL != msg)
      MESSAGE_ERROR("ASSERT: line %d of %s, %s\n", line, file, msg);
   else
      MESSAGE_ERROR("ASSERT: line %d of %s\n", line, file);

   asm("break.n 1");
//   exit(-1);
}

/* End the current context */
void nofrendo_stop(void)
{
   nes_poweroff();
   nes_shutdown();
   osd_shutdown();
   // vid_shutdown();
}

int nofrendo_start(const char *filename, int region, int sample_rate)
{
   if (osd_init())
      return -1;

   if (region == NES_AUTO)
   {
      if (
         strstr(filename, "(E)") != NULL ||
         strstr(filename, "(Europe)") != NULL ||
         strstr(filename, "(A)") != NULL ||
         strstr(filename, "(Australia)") != NULL
      )
         region = NES_PAL;
      else
         region = NES_NTSC;
   }

   if (!nes_init(region, sample_rate))
   {
      MESSAGE_ERROR("Failed to create NES instance.\n");
      return -1;
   }

   if (!nes_insertcart(filename))
   {
      MESSAGE_ERROR("Failed to insert NES cart.\n");
      return -2;
   }

   nes_emulate();

   return 0;
}
