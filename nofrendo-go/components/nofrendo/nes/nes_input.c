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
#include "nes_input.h"

static nesinput_t nes_inputs[INP_TYPE_MAX];
static int strobe = 0;

IRAM_ATTR void input_write(uint32 address, uint8 value)
{
   if (address != INP_REG_JOY0)
      return;

   value &= 1;

   if (0 == value && strobe)
   {
      for (int i = 0; i < INP_TYPE_MAX; ++i)
         nes_inputs[i].reads = 0;
   }

   strobe = value;
}

IRAM_ATTR uint8 input_read(uint32 address)
{
   uint8 retval = 0, value = 0;

   if (address == INP_REG_JOY0)
   {
      value = nes_inputs[INP_JOYPAD0].state;

      /* mask out left/right simultaneous keypresses */
      if ((value & INP_PAD_UP) && (value & INP_PAD_DOWN))
         value &= ~(INP_PAD_UP | INP_PAD_DOWN);

      if ((value & INP_PAD_LEFT) && (value & INP_PAD_RIGHT))
         value &= ~(INP_PAD_LEFT | INP_PAD_RIGHT);

      /* return (0x40 | value) due to bus conflicts */
      retval |= (0x40 | ((value >> nes_inputs[INP_JOYPAD0].reads++) & 1));
   }
   else if (address == INP_REG_JOY1)
   {
      value = nes_inputs[INP_JOYPAD1].state;

      /* mask out left/right simultaneous keypresses */
      if ((value & INP_PAD_UP) && (value & INP_PAD_DOWN))
         value &= ~(INP_PAD_UP | INP_PAD_DOWN);

      if ((value & INP_PAD_LEFT) && (value & INP_PAD_RIGHT))
         value &= ~(INP_PAD_LEFT | INP_PAD_RIGHT);

      /* return (0x40 | value) due to bus conflicts */
      retval |= (0x40 | ((value >> nes_inputs[INP_JOYPAD1].reads++) & 1));

      retval |= nes_inputs[INP_ZAPPER].state;
   }
   else
   {
      retval = 0xFF;
   }

   return retval;
}

void input_connect(nesinput_type_t input)
{
   ASSERT(input < INP_TYPE_MAX);

   nes_inputs[input].connected = true;
}

void input_disconnect(nesinput_type_t input)
{
   ASSERT(input < INP_TYPE_MAX);

   nes_inputs[input].connected = false;
}

void input_update(nesinput_type_t input, uint8 state)
{
   ASSERT(input < INP_TYPE_MAX);

   nes_inputs[input].state = state;
}
