/* gameplaySP
 *
 * Copyright (C) 2006 Exophase <exophase@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "common.h"
#include <ctype.h>

timer_type timer[4];

u32 frame_counter = 0;
u32 cpu_ticks = 0;
u32 execute_cycles = 0;
s32 video_count = 0;

u32 last_frame = 0;
u32 flush_ram_count = 0;
u32 gbc_update_count = 0;
u32 oam_update_count = 0;

char main_path[512];

static u32 random_state = 0;

// Generate 16 random bits.
u16 rand_gen() {
  random_state = ((random_state * 1103515245) + 12345) & 0x7fffffff;
  return random_state;
}

// Add some random state to the initial seed.
void rand_seed(u32 data) {
  random_state ^= rand_gen() ^ data;
}


static unsigned update_timers(irq_type *irq_raised, unsigned completed_cycles)
{
   unsigned i, ret = 0;
   for (i = 0; i < 4; i++)
   {
      if(timer[i].status == TIMER_INACTIVE)
         continue;

      if(timer[i].status != TIMER_CASCADE)
      {
         timer[i].count -= completed_cycles;
         /* io_registers accessors range: REG_TM0D, REG_TM1D, REG_TM2D, REG_TM3D */
         write_ioreg(REG_TMXD(i), -(timer[i].count >> timer[i].prescale));
      }

      if(timer[i].count > 0)
         continue;

      /* irq_raised value range: IRQ_TIMER0, IRQ_TIMER1, IRQ_TIMER2, IRQ_TIMER3 */
      if(timer[i].irq)
         *irq_raised |= (IRQ_TIMER0 << i);

      if((i != 3) && (timer[i + 1].status == TIMER_CASCADE))
      {
         timer[i + 1].count--;
         write_ioreg(REG_TMXD(i + 1), -timer[i+1].count);
      }

      if(i < 2)
      {
         if(timer[i].direct_sound_channels & 0x01)
            ret += sound_timer(timer[i].frequency_step, 0);

         if(timer[i].direct_sound_channels & 0x02)
            ret += sound_timer(timer[i].frequency_step, 1);
      }

      timer[i].count += (timer[i].reload << timer[i].prescale);
   }
   return ret;
}

void init_main(void)
{
  u32 i;
  for(i = 0; i < 4; i++)
  {
    timer[i].status = TIMER_INACTIVE;
    timer[i].prescale = 0;
    timer[i].irq = 0;
    timer[i].reload = timer[i].count = 0x10000;
    timer[i].direct_sound_channels = TIMER_DS_CHANNEL_NONE;
    timer[i].frequency_step = 0;
  }

  timer[0].direct_sound_channels = TIMER_DS_CHANNEL_BOTH;
  timer[1].direct_sound_channels = TIMER_DS_CHANNEL_NONE;

  frame_counter = 0;
  cpu_ticks = 0;
  execute_cycles = 960;
  video_count = 960;

#ifdef HAVE_DYNAREC
  init_dynarec_caches();
  init_emitter(gamepak_must_swap());
#endif
}

u32 function_cc update_gba(int remaining_cycles)
{
  u32 changed_pc = 0;
  u32 frame_complete = 0;
  irq_type irq_raised = IRQ_NONE;
  int dma_cycles;
  trace_update_gba(remaining_cycles);

  remaining_cycles = MAX(remaining_cycles, -64);

  do
  {
    unsigned i;
    // Number of cycles we ask to run - cycles that we did not execute
    // (remaining_cycles can be negative and should be close to zero)
    unsigned completed_cycles = execute_cycles - remaining_cycles;
    cpu_ticks += completed_cycles;

    remaining_cycles = 0;

    // Timers can trigger DMA (usually sound) and consume cycles
    dma_cycles = update_timers(&irq_raised, completed_cycles);
    // Check for serial port IRQs as well.
    if (update_serial(completed_cycles))
      irq_raised |= IRQ_SERIAL;

    // Video count tracks the video cycles remaining until the next event
    video_count -= completed_cycles;

    // Ran out of cycles, move to the next video area
    if(video_count <= 0)
    {
      u32 vcount = read_ioreg(REG_VCOUNT);
      u32 dispstat = read_ioreg(REG_DISPSTAT);

      // Check if we are in hrefresh (0) or hblank (1)
      if ((dispstat & 0x02) == 0)
      {
        // Transition from hrefresh to hblank
        dispstat |= 0x02;
        video_count += (272);    // hblank duration, 272 cycles

        // Check if we are drawing (0) or we are in vblank (1)
        if ((dispstat & 0x01) == 0)
        {
          u32 i;

          // Render the scan line
          if(reg[OAM_UPDATED])
            oam_update_count++;

          update_scanline();

          // Trigger the HBlank DMAs if enabled
          for (i = 0; i < 4; i++)
          {
            if(dma[i].start_type == DMA_START_HBLANK)
              dma_transfer(i, &dma_cycles);
          }
        }

        // Trigger the hblank interrupt, if enabled in DISPSTAT
        if (dispstat & 0x10)
          irq_raised |= IRQ_HBLANK;
      }
      else
      {
        // Transition from hblank to the next scan line (vdraw or vblank)
        video_count += 960;
        dispstat &= ~0x02;
        vcount++;

        if(vcount == 160)
        {
          // Transition from vrefresh to vblank
          u32 i;
          dispstat |= 0x01;

          // Reinit affine transformation counters for the next frame
          video_reload_counters();

          // Trigger VBlank interrupt if enabled
          if (dispstat & 0x8)
            irq_raised |= IRQ_VBLANK;

          // Trigger the VBlank DMAs if enabled
          for (i = 0; i < 4; i++)
          {
            if(dma[i].start_type == DMA_START_VBLANK)
              dma_transfer(i, &dma_cycles);
          }
        }
        else if (vcount == 228)
        {
          // Transition from vblank to next screen
          vcount = 0;
          dispstat &= ~0x01;

          /* If there's no cheat hook, run on vblank! */
          if (cheat_master_hook == ~0U)
             process_cheats();

/*        printf("frame update (%x), %d instructions total, %d RAM flushes\n",
           reg[REG_PC], instruction_count - last_frame, flush_ram_count);
          last_frame = instruction_count;
*/
/*          printf("%d gbc audio updates\n", gbc_update_count);
          printf("%d oam updates\n", oam_update_count); */
          gbc_update_count = 0;
          oam_update_count = 0;
          flush_ram_count = 0;

          // Force audio generation. Need to flush samples for this frame.
          render_gbc_sound();

          // We completed a frame, tell the dynarec to exit to the main thread
          frame_complete = 0x80000000;
          frame_counter++;
        }

        // Vcount trigger (flag) and IRQ if enabled
        if(vcount == (dispstat >> 8))
        {
          dispstat |= 0x04;
          if(dispstat & 0x20)
            irq_raised |= IRQ_VCOUNT;
        }
        else
          dispstat &= ~0x04;

        write_ioreg(REG_VCOUNT, vcount);
      }
      write_ioreg(REG_DISPSTAT, dispstat);
    }

    // Flag any V/H blank interrupts, DMA IRQs, Vcount, etc.
    if (irq_raised)
      flag_interrupt(irq_raised);

    // Raise any pending interrupts. This changes the CPU mode.
    if (check_and_raise_interrupts())
      changed_pc = 0x40000000;

    // Figure out when we need to stop CPU execution. The next event is
    // a video event or a timer event, whatever happens first.
    execute_cycles = MAX(video_count, 0);
    {
      u32 cc = serial_next_event();
      execute_cycles = MIN(execute_cycles, cc);
    }

    // If we are paused due to a DMA, cap the number of cyles to that amount.
    if (reg[CPU_HALT_STATE] == CPU_DMA) {
      u32 dma_cyc = reg[REG_SLEEP_CYCLES];
      // The first iteration is marked by bit 31 set, just do nothing now.
      if (dma_cyc & 0x80000000)
        dma_cyc &= 0x7FFFFFFF;  // Start counting DMA cycles from now on.
      else
        dma_cyc -= MIN(dma_cyc, completed_cycles);  // Account DMA cycles.

      reg[REG_SLEEP_CYCLES] = dma_cyc;
      if (!dma_cyc)
        reg[CPU_HALT_STATE] = CPU_ACTIVE;   // DMA finished, resume execution.
      else
        execute_cycles = MIN(execute_cycles, dma_cyc);  // Continue sleeping.
    }

    for (i = 0; i < 4; i++)
    {
       if (timer[i].status == TIMER_PRESCALE &&
           timer[i].count < execute_cycles)
          execute_cycles = timer[i].count;
    }
  } while(reg[CPU_HALT_STATE] != CPU_ACTIVE && !frame_complete);

  // We voluntarily limit this. It is not accurate but it would be much harder.
  dma_cycles = MIN(64, dma_cycles);
  dma_cycles = MIN(execute_cycles, dma_cycles);

  return (execute_cycles - dma_cycles) | changed_pc | frame_complete;
}

void reset_gba(void)
{
  init_memory();
  init_main();
  init_cpu();
  reset_sound();
}

#ifdef TRACE_REGISTERS
void print_regs(void)
{
  printf("R0=%08x R1=%08x R2=%08x R3=%08x "
         "R4=%08x R5=%08x R6=%08x R7=%08x "
         "R8=%08x R9=%08x R10=%08x R11=%08x "
         "R12=%08x R13=%08x R14=%08x\n",
         reg[0], reg[1], reg[2], reg[3],
         reg[4], reg[5], reg[6], reg[7],
         reg[8], reg[9], reg[10], reg[11],
         reg[12], reg[13], reg[14]);
}
#endif

bool main_check_savestate(const u8 *src)
{
  int i;
  const u8 *p1 = bson_find_key(src, "emu");
  const u8 *p2 = bson_find_key(src, "timers");
  if (!p1 || !p2)
    return false;

  if (!bson_contains_key(p1, "cpu-ticks", BSON_TYPE_INT32) ||
      !bson_contains_key(p1, "exec-cycles", BSON_TYPE_INT32) ||
      !bson_contains_key(p1, "video-count", BSON_TYPE_INT32) ||
      !bson_contains_key(p1, "sleep-cycles", BSON_TYPE_INT32))
    return false;

  for (i = 0; i < 4; i++)
  {
    char tname[2] = {'0' + i, 0};
    const u8 *p = bson_find_key(p2, tname);
    if (!p)
      return false;

    if (!bson_contains_key(p, "count", BSON_TYPE_INT32) ||
        !bson_contains_key(p, "reload", BSON_TYPE_INT32) ||
        !bson_contains_key(p, "prescale", BSON_TYPE_INT32) ||
        !bson_contains_key(p, "freq-step", BSON_TYPE_INT32) ||
        !bson_contains_key(p, "dsc", BSON_TYPE_INT32) ||
        !bson_contains_key(p, "irq", BSON_TYPE_INT32) ||
        !bson_contains_key(p, "status", BSON_TYPE_INT32))
      return false;
  }

  return true;
}

bool main_read_savestate(const u8 *src)
{
  int i;
  const u8 *p1 = bson_find_key(src, "emu");
  const u8 *p2 = bson_find_key(src, "timers");
  if (!p1 || !p2)
    return false;

  if (!(bson_read_int32(p1, "cpu-ticks", &cpu_ticks) &&
         bson_read_int32(p1, "exec-cycles", &execute_cycles) &&
         bson_read_int32(p1, "video-count", (u32*)&video_count) &&
         bson_read_int32(p1, "sleep-cycles", &reg[REG_SLEEP_CYCLES])))
    return false;

  if (!bson_read_int32(p1, "frame-count", &frame_counter))
    frame_counter = 60 * 10;  // Use "fake" 10 seconds.

  for (i = 0; i < 4; i++)
  {
    char tname[2] = {'0' + i, 0};
    const u8 *p = bson_find_key(p2, tname);

    if (!(
      bson_read_int32(p, "count", (u32*)&timer[i].count) &&
      bson_read_int32(p, "reload", &timer[i].reload) &&
      bson_read_int32(p, "prescale", &timer[i].prescale) &&
      bson_read_int32(p, "freq-step", &timer[i].frequency_step) &&
      bson_read_int32(p, "dsc", &timer[i].direct_sound_channels) &&
      bson_read_int32(p, "irq", &timer[i].irq) &&
      bson_read_int32(p, "status", &timer[i].status)))
      return false;
  }

  return true;
}

unsigned main_write_savestate(u8* dst)
{
  int i;
  u8 *wbptr, *wbptr2, *startp = dst;
  bson_start_document(dst, "emu", wbptr);
  bson_write_int32(dst, "frame-count", frame_counter);
  bson_write_int32(dst, "cpu-ticks", cpu_ticks);
  bson_write_int32(dst, "exec-cycles", execute_cycles);
  bson_write_int32(dst, "video-count", video_count);
  bson_write_int32(dst, "sleep-cycles", reg[REG_SLEEP_CYCLES]);
  bson_finish_document(dst, wbptr);

  bson_start_document(dst, "timers", wbptr);
  for (i = 0; i < 4; i++)
  {
    char tname[2] = {'0' + i, 0};
    bson_start_document(dst, tname, wbptr2);
    bson_write_int32(dst, "count", timer[i].count);
    bson_write_int32(dst, "reload", timer[i].reload);
    bson_write_int32(dst, "prescale", timer[i].prescale);
    bson_write_int32(dst, "freq-step", timer[i].frequency_step);
    bson_write_int32(dst, "dsc", timer[i].direct_sound_channels);
    bson_write_int32(dst, "irq", timer[i].irq);
    bson_write_int32(dst, "status", timer[i].status);
    bson_finish_document(dst, wbptr2);
  }
  bson_finish_document(dst, wbptr);

  return (unsigned int)(dst - startp);
}


