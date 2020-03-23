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
#include <noftypes.h>
#include <nofrendo.h>
#include <event.h>
#include <log.h>
#include <osd.h>
#include <gui.h>
#include <nes.h>


bitmap_t *primary_buffer;
nes_t *console;


/* our happy little timer ISR */
volatile int nofrendo_ticks = 0;
static void timer_isr(void)
{
   nofrendo_ticks++;
}

void nofrendo_refresh()
{
   // osd_blitscreen(primary_buffer);
}

/* End the current context */
void nofrendo_stop(void)
{
   nes_poweroff();
   nes_destroy(&console);

   // osd_shutdown();
   gui_shutdown();
   // vid_shutdown();
   log_shutdown();
}

int nofrendo_start(const char *filename, int region)
{
   event_init();

   if (log_init())
      return -1;

   if (osd_init())
      return -1;

   if (gui_init())
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

   console = nes_create(region);

   if (NULL == console)
   {
      log_printf("Failed to create NES instance.\n");
      return -1;
   }

   if (nes_insertcart(filename, console))
   {
      log_printf("Failed to insert NES cart.\n");
      // odroid_system_panic("Unable to load NES ROM.");
      return -1;
   }

   nes_emulate();

   return 0;
}
