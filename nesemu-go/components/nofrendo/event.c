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
** event.c
**
** OS-independent event handling
** $Id: event.c,v 1.3 2001/04/27 14:37:11 neil Exp $
*/

#include <noftypes.h>
#include <event.h>
#include <nofrendo.h>
#include <gui.h>
#include <nes.h>
#include <nesinput.h>
#include <nesstate.h>

/* standard keyboard input */
static nesinput_t kb_input = { INP_JOYPAD0, 0 };
static nesinput_t kb_alt_input = { INP_JOYPAD1, 0 };

static void func_event_quit(int code)
{
   UNUSED(code);
   // main_quit();
}

static void func_event_togglepause(int code)
{
   if (INP_STATE_MAKE == code)
      nes_togglepause();
}

static void func_event_soft_reset(int code)
{
   if (INP_STATE_MAKE == code)
      nes_reset(SOFT_RESET);
}

static void func_event_hard_reset(int code)
{
   if (INP_STATE_MAKE == code)
      nes_reset(HARD_RESET);
}

static void func_event_toggle_frameskip(int code)
{
   if (INP_STATE_MAKE == code)
      gui_togglefs();
}

static void func_event_state_save(int code)
{
   // if (INP_STATE_MAKE == code)
   //    state_save();
}

static void func_event_state_load(int code)
{
   // if (INP_STATE_MAKE == code)
   //    state_load();
}

static void func_event_state_slot_0(int code)
{
   if (INP_STATE_MAKE == code)
      state_setslot(0);
}

static void func_event_state_slot_1(int code)
{
   if (INP_STATE_MAKE == code)
      state_setslot(1);
}

static void func_event_state_slot_2(int code)
{
   if (INP_STATE_MAKE == code)
      state_setslot(2);
}

static void func_event_state_slot_3(int code)
{
   if (INP_STATE_MAKE == code)
      state_setslot(3);
}

static void func_event_state_slot_4(int code)
{
   if (INP_STATE_MAKE == code)
      state_setslot(4);
}

static void func_event_state_slot_5(int code)
{
   if (INP_STATE_MAKE == code)
      state_setslot(5);
}

static void func_event_state_slot_6(int code)
{
   if (INP_STATE_MAKE == code)
      state_setslot(6);
}

static void func_event_state_slot_7(int code)
{
   if (INP_STATE_MAKE == code)
      state_setslot(7);
}

static void func_event_state_slot_8(int code)
{
   if (INP_STATE_MAKE == code)
      state_setslot(8);
}

static void func_event_state_slot_9(int code)
{
   if (INP_STATE_MAKE == code)
      state_setslot(9);
}

static void func_event_gui_toggle_oam(int code)
{
   if (INP_STATE_MAKE == code)
      gui_toggleoam();
}

static void func_event_gui_toggle_wave(int code)
{
   if (INP_STATE_MAKE == code)
      gui_togglewave();
}

static void func_event_gui_toggle_pattern(int code)
{
   if (INP_STATE_MAKE == code)
      gui_togglepattern();
}

static void func_event_gui_pattern_color_up(int code)
{
   if (INP_STATE_MAKE == code)
      gui_incpatterncol();
}

static void func_event_gui_pattern_color_down(int code)
{
   if (INP_STATE_MAKE == code)
      gui_decpatterncol();
}

static void func_event_gui_toggle_fps(int code)
{
   if (INP_STATE_MAKE == code)
      gui_togglefps();
}

static void func_event_gui_display_info(int code)
{
   if (INP_STATE_MAKE == code)
      gui_displayinfo();
}

static void func_event_gui_toggle(int code)
{
   if (INP_STATE_MAKE == code)
      gui_togglegui();
}

static void func_event_toggle_channel_0(int code)
{
   if (INP_STATE_MAKE == code)
      gui_toggle_chan(0);
}

static void func_event_toggle_channel_1(int code)
{
   if (INP_STATE_MAKE == code)
      gui_toggle_chan(1);
}

static void func_event_toggle_channel_2(int code)
{
   if (INP_STATE_MAKE == code)
      gui_toggle_chan(2);
}

static void func_event_toggle_channel_3(int code)
{
   if (INP_STATE_MAKE == code)
      gui_toggle_chan(3);
}

static void func_event_toggle_channel_4(int code)
{
   if (INP_STATE_MAKE == code)
      gui_toggle_chan(4);
}

static void func_event_toggle_channel_5(int code)
{
   if (INP_STATE_MAKE == code)
      gui_toggle_chan(5);
}

static void func_event_set_filter_0(int code)
{
   if (INP_STATE_MAKE == code)
      gui_setfilter(0);
}

static void func_event_set_filter_1(int code)
{
   if (INP_STATE_MAKE == code)
      gui_setfilter(1);
}

static void func_event_set_filter_2(int code)
{
   if (INP_STATE_MAKE == code)
      gui_setfilter(2);
}

static void func_event_toggle_sprites(int code)
{
   if (INP_STATE_MAKE == code)
      gui_togglesprites();
}

static void func_event_joypad1_a(int code)
{
   input_event(&kb_input, code, INP_PAD_A);
}

static void func_event_joypad1_b(int code)
{
   input_event(&kb_input, code, INP_PAD_B);
}

static void func_event_joypad1_start(int code)
{
   input_event(&kb_input, code, INP_PAD_START);
}

static void func_event_joypad1_select(int code)
{
   input_event(&kb_input, code, INP_PAD_SELECT);
}

static void func_event_joypad1_up(int code)
{
   input_event(&kb_input, code, INP_PAD_UP);
}

static void func_event_joypad1_down(int code)
{
   input_event(&kb_input, code, INP_PAD_DOWN);
}

static void func_event_joypad1_left(int code)
{
   input_event(&kb_input, code, INP_PAD_LEFT);
}

static void func_event_joypad1_right(int code)
{
   input_event(&kb_input, code, INP_PAD_RIGHT);
}

static void func_event_joypad2_a(int code)
{
   input_event(&kb_alt_input, code, INP_PAD_A);
}

static void func_event_joypad2_b(int code)
{
   input_event(&kb_alt_input, code, INP_PAD_B);
}

static void func_event_joypad2_start(int code)
{
   input_event(&kb_alt_input, code, INP_PAD_START);
}

static void func_event_joypad2_select(int code)
{
   input_event(&kb_alt_input, code, INP_PAD_SELECT);
}

static void func_event_joypad2_up(int code)
{
   input_event(&kb_alt_input, code, INP_PAD_UP);
}

static void func_event_joypad2_down(int code)
{
   input_event(&kb_alt_input, code, INP_PAD_DOWN);
}

static void func_event_joypad2_left(int code)
{
   input_event(&kb_alt_input, code, INP_PAD_LEFT);
}

static void func_event_joypad2_right(int code)
{
   input_event(&kb_alt_input, code, INP_PAD_RIGHT);
}


/* NES events */
static event_t system_events[64] =
{
   NULL, /* 0 */
   func_event_quit,
   func_event_togglepause,
   func_event_soft_reset,
   func_event_hard_reset,
   func_event_toggle_frameskip,
   func_event_toggle_sprites,
   /* saves */
   func_event_state_save,
   func_event_state_load, /* 10 */
   func_event_state_slot_0,
   func_event_state_slot_1,
   func_event_state_slot_2,
   func_event_state_slot_3,
   func_event_state_slot_4,
   func_event_state_slot_5,
   func_event_state_slot_6,
   func_event_state_slot_7,
   func_event_state_slot_8,
   func_event_state_slot_9, /* 20 */
   /* GUI */
   func_event_gui_toggle_oam,
   func_event_gui_toggle_wave,
   func_event_gui_toggle_pattern,
   func_event_gui_pattern_color_up,
   func_event_gui_pattern_color_down,
   func_event_gui_toggle_fps,
   func_event_gui_display_info,
   func_event_gui_toggle,
   /* sound */
   func_event_toggle_channel_0,
   func_event_toggle_channel_1, /* 30 */
   func_event_toggle_channel_2,
   func_event_toggle_channel_3,
   func_event_toggle_channel_4,
   func_event_toggle_channel_5,
   func_event_set_filter_0,
   func_event_set_filter_1,
   func_event_set_filter_2,
   /* joypad 1 */
   func_event_joypad1_a,
   func_event_joypad1_b,
   func_event_joypad1_start,
   func_event_joypad1_select,
   func_event_joypad1_up,
   func_event_joypad1_down,
   func_event_joypad1_left, /* 50 */
   func_event_joypad1_right,
   /* joypad 2 */
   func_event_joypad2_a,
   func_event_joypad2_b,
   func_event_joypad2_start,
   func_event_joypad2_select,
   func_event_joypad2_up,
   func_event_joypad2_down,
   func_event_joypad2_left,
   func_event_joypad2_right,
   /* unused */
   NULL,
   NULL,
   NULL,
   NULL,
   /* OS-specific */
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
};


void event_init(void)
{
   input_register(&kb_input);
   input_register(&kb_alt_input);
}

void event_set(int index, event_t handler)
{
   system_events[index] = handler;
}

event_t event_get(int index)
{
   return system_events[index];
}

void event_raise(int index, int code)
{
   event_t evh = system_events[index];
   if (evh) evh(code);
}
