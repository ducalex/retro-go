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
** nes_input.c
**
** Event handling system routines
** $Id: nes_input.c,v 1.2 2001/04/27 14:37:11 neil Exp $
*/

#include <nofrendo.h>
#include <esp_attr.h>
#include "nes_input.h"

/* TODO: make a linked list of inputs sources, so they
**       can be removed if need be
*/
static nesinput_t *nes_input[MAX_CONTROLLERS];
static int active_entries = 0, strobe = 0;

/* read counters */
static int pad0_readcount, pad1_readcount, ppad_readcount, ark_readcount;

INLINE int retrieve_type(int type)
{
   int i, value = 0;

   for (i = 0; i < active_entries; i++)
   {
      if (nes_input[i]) {
         if (type == nes_input[i]->type)
            value |= nes_input[i]->data;
      }
   }

   return value;
}

IRAM_ATTR void input_write(uint32 address, uint8 value)
{
   ASSERT(address == INP_JOY0);

   value &= 1;

   if (0 == value && strobe)
   {
      pad0_readcount = 0;
      pad1_readcount = 0;
      ppad_readcount = 0;
      ark_readcount = 0;
   }

   strobe = value;
}

IRAM_ATTR uint8 input_read(uint32 address)
{
   uint8 retval = 0, value = 0;

   if (address == INP_JOY0)
   {
      value = (uint8) retrieve_type(INP_JOYPAD0);

      /* mask out left/right simultaneous keypresses */
      if ((value & INP_PAD_UP) && (value & INP_PAD_DOWN))
         value &= ~(INP_PAD_UP | INP_PAD_DOWN);

      if ((value & INP_PAD_LEFT) && (value & INP_PAD_RIGHT))
         value &= ~(INP_PAD_LEFT | INP_PAD_RIGHT);

      /* return (0x40 | value) due to bus conflicts */
      retval |= (0x40 | ((value >> pad0_readcount++) & 1));
   }
   else if (address == INP_JOY1)
   {
      value = (uint8) retrieve_type(INP_JOYPAD1);

      /* mask out left/right simultaneous keypresses */
      if ((value & INP_PAD_UP) && (value & INP_PAD_DOWN))
         value &= ~(INP_PAD_UP | INP_PAD_DOWN);

      if ((value & INP_PAD_LEFT) && (value & INP_PAD_RIGHT))
         value &= ~(INP_PAD_LEFT | INP_PAD_RIGHT);

      /* return (0x40 | value) due to bus conflicts */
      retval |= (0x40 | ((value >> pad1_readcount++) & 1));

      retval |= retrieve_type(INP_ZAPPER);
   }
   else
   {
      retval = 0xFF;
   }

   return retval;
}

/* register an input type */
void input_register(nesinput_t *input)
{
   ASSERT(input);

   nes_input[active_entries] = input;
   active_entries++;
}

void input_event(nesinput_t *input, int state, int value)
{
   ASSERT(input);

   if (state == INP_STATE_MAKE)
      input->data |= value;   /* OR it in */
   else /* break state */
      input->data &= ~value;  /* mask it out */
}
