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
#include "streams/file_stream.h"
#include "bios/open_gba_bios.h"

/* Sound */
#define gbc_sound_tone_control_low(channel, regn)                             \
{                                                                             \
  render_gbc_sound();                                                         \
  u32 initial_volume = (value >> 12) & 0x0F;                                  \
  u32 envelope_ticks = ((value >> 8) & 0x07) * 4;                             \
  gbc_sound_channel[channel].length_ticks = 64 - (value & 0x3F);              \
  gbc_sound_channel[channel].sample_table_idx = ((value >> 6) & 0x03);        \
  gbc_sound_channel[channel].envelope_direction = (value >> 11) & 0x01;       \
  gbc_sound_channel[channel].envelope_initial_volume = initial_volume;        \
  gbc_sound_channel[channel].envelope_volume = initial_volume;                \
  gbc_sound_channel[channel].envelope_initial_ticks = envelope_ticks;         \
  gbc_sound_channel[channel].envelope_ticks = envelope_ticks;                 \
  gbc_sound_channel[channel].envelope_status = (envelope_ticks != 0);         \
  gbc_sound_channel[channel].envelope_volume = initial_volume;                \
  write_ioreg(regn, value);                                                   \
}                                                                             \

#define gbc_sound_tone_control_high(channel, regn)                            \
{                                                                             \
  render_gbc_sound();                                                         \
  u32 rate = value & 0x7FF;                                                   \
  gbc_sound_channel[channel].rate = rate;                                     \
  gbc_sound_channel[channel].frequency_step =                                 \
   float_to_fp16_16(((131072.0 / (2048 - rate)) * 8.0) / sound_frequency);    \
  gbc_sound_channel[channel].length_status = (value >> 14) & 0x01;            \
  if(value & 0x8000)                                                          \
  {                                                                           \
    gbc_sound_channel[channel].active_flag = 1;                               \
    gbc_sound_channel[channel].sample_index -= float_to_fp16_16(1.0 / 12.0);  \
    gbc_sound_channel[channel].envelope_ticks =                               \
     gbc_sound_channel[channel].envelope_initial_ticks;                       \
    gbc_sound_channel[channel].envelope_volume =                              \
     gbc_sound_channel[channel].envelope_initial_volume;                      \
  }                                                                           \
                                                                              \
  write_ioreg(regn, value & 0x47FF);                                          \
}                                                                             \

#define gbc_sound_tone_control_sweep()                                        \
{                                                                             \
  render_gbc_sound();                                                         \
  value &= 0x007F;                                                            \
  u32 sweep_ticks = ((value >> 4) & 0x07) * 2;                                \
  gbc_sound_channel[0].sweep_shift = value & 0x07;                            \
  gbc_sound_channel[0].sweep_direction = (value >> 3) & 0x01;                 \
  gbc_sound_channel[0].sweep_status = (value != 8);                           \
  gbc_sound_channel[0].sweep_ticks = sweep_ticks;                             \
  gbc_sound_channel[0].sweep_initial_ticks = sweep_ticks;                     \
  write_ioreg(REG_SOUND1CNT_L, value);                                        \
}                                                                             \

#define gbc_sound_wave_control()                                              \
{                                                                             \
  render_gbc_sound();                                                         \
  gbc_sound_channel[2].wave_type = (value >> 5) & 0x01;                       \
  gbc_sound_channel[2].wave_bank = (value >> 6) & 0x01;                       \
  gbc_sound_channel[2].master_enable = 0;                                     \
  if(value & 0x80)                                                            \
    gbc_sound_channel[2].master_enable = 1;                                   \
                                                                              \
  write_ioreg(REG_SOUND3CNT_L, value & 0x00E0);                               \
}                                                                             \

static const u32 gbc_sound_wave_volume[4] = { 0, 16384, 8192, 4096 };

#define gbc_sound_tone_control_low_wave()                                     \
{                                                                             \
  render_gbc_sound();                                                         \
  gbc_sound_channel[2].length_ticks = 256 - (value & 0xFF);                   \
  if((value >> 15) & 0x01)                                                    \
    gbc_sound_channel[2].wave_volume = 12288;                                 \
  else                                                                        \
    gbc_sound_channel[2].wave_volume =                                        \
     gbc_sound_wave_volume[(value >> 13) & 0x03];                             \
  write_ioreg(REG_SOUND3CNT_H, value);                                        \
}                                                                             \

#define gbc_sound_tone_control_high_wave()                                    \
{                                                                             \
  render_gbc_sound();                                                         \
  u32 rate = value & 0x7FF;                                                   \
  gbc_sound_channel[2].rate = rate;                                           \
  gbc_sound_channel[2].frequency_step =                                       \
   float_to_fp16_16((2097152.0 / (2048 - rate)) / sound_frequency);           \
  gbc_sound_channel[2].length_status = (value >> 14) & 0x01;                  \
  if(value & 0x8000)                                                          \
  {                                                                           \
    gbc_sound_channel[2].sample_index = 0;                                    \
    gbc_sound_channel[2].active_flag = 1;                                     \
  }                                                                           \
  write_ioreg(REG_SOUND3CNT_X, value);                                        \
}                                                                             \

#define gbc_sound_noise_control()                                             \
{                                                                             \
  u32 dividing_ratio = value & 0x07;                                          \
  u32 frequency_shift = (value >> 4) & 0x0F;                                  \
  render_gbc_sound();                                                         \
  if(dividing_ratio == 0)                                                     \
  {                                                                           \
    gbc_sound_channel[3].frequency_step =                                     \
     float_to_fp16_16(1048576.0 / (1 << (frequency_shift + 1)) /              \
     sound_frequency);                                                        \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    gbc_sound_channel[3].frequency_step =                                     \
     float_to_fp16_16(524288.0 / (dividing_ratio *                            \
     (1 << (frequency_shift + 1))) / sound_frequency);                        \
  }                                                                           \
  gbc_sound_channel[3].noise_type = (value >> 3) & 0x01;                      \
  gbc_sound_channel[3].length_status = (value >> 14) & 0x01;                  \
  if(value & 0x8000)                                                          \
  {                                                                           \
    gbc_sound_channel[3].sample_index = 0;                                    \
    gbc_sound_channel[3].active_flag = 1;                                     \
    gbc_sound_channel[3].envelope_ticks =                                     \
     gbc_sound_channel[3].envelope_initial_ticks;                             \
    gbc_sound_channel[3].envelope_volume =                                    \
     gbc_sound_channel[3].envelope_initial_volume;                            \
  }                                                                           \
  write_ioreg(REG_SOUND4CNT_H, value & 0x40FF);                               \
}                                                                             \

static void gbc_trigger_sound(u32 value)
{
   u32 channel;
   render_gbc_sound();

   /* Trigger all 4 GBC sound channels */
   for (channel = 0; channel < 4; channel++)
   {
      gbc_sound_master_volume_right = value & 0x07;
      gbc_sound_master_volume_left = (value >> 4) & 0x07;
      gbc_sound_channel[channel].status =
        (((value >> (channel + 8)) & 0x1) | ((value >> (channel + 11)) & 0x3));
   }
   write_ioreg(REG_SOUNDCNT_L, value & 0xFF77);
}

#define trigger_sound()                                                       \
{                                                                             \
  render_gbc_sound();                                                         \
  timer[0].direct_sound_channels =                                            \
      ((((value >> 10) & 0x01) == 0) | ((((value >> 14) & 0x01) == 0) << 1)); \
  timer[1].direct_sound_channels =                                            \
      ((((value >> 10) & 0x01) == 1) | ((((value >> 14) & 0x01) == 1) << 1)); \
  direct_sound_channel[0].volume_halve = ((~(value >> 2)) & 0x01);            \
  direct_sound_channel[0].status = ((value >> 8) & 0x03);                     \
  direct_sound_channel[1].volume_halve = ((~(value >> 3)) & 0x01);            \
  direct_sound_channel[1].status = ((value >> 12) & 0x03);                    \
  gbc_sound_master_volume = value & 0x03;                                     \
                                                                              \
  if((value >> 11) & 0x01)                                                    \
    sound_reset_fifo(0);                                                      \
  if((value >> 15) & 0x01)                                                    \
    sound_reset_fifo(1);                                                      \
  write_ioreg(REG_SOUNDCNT_H, value & 0x770F);                                \
}                                                                             \

static void sound_control_x(u32 value)
{
   render_gbc_sound();
   if (value & 0x80)
   {
      if (sound_on != 1)
         sound_on = 1;
   }
   else
   {
      u32 i;
      for (i = 0; i < 4; i++)
         gbc_sound_channel[i].active_flag = 0;
      sound_on = 0;
   }

   value = (value & 0xFFF0) | (read_ioreg(REG_SOUNDCNT_X) & 0x000F);
   write_ioreg(REG_SOUNDCNT_X, value);
}

#define sound_update_frequency_step(timer_number)                             \
  timer[timer_number].frequency_step =                                        \
   float_to_fp8_24((GBC_BASE_RATE / sound_frequency) / (timer_reload))        \

/* Main */
extern timer_type timer[4];
static const u32 prescale_table[] = { 0, 6, 8, 10 };

#define count_timer(timer_number)                                             \
  timer[timer_number].reload = 0x10000 - value;                               \
  if(timer_number < 2)                                                        \
  {                                                                           \
    u32 timer_reload =                                                        \
     timer[timer_number].reload << timer[timer_number].prescale;              \
    sound_update_frequency_step(timer_number);                                \
  }                                                                           \

#define adjust_sound_buffer(timer_number, channel)                            \
  if(timer[timer_number].direct_sound_channels & (0x01 << channel))           \
  {                                                                           \
    direct_sound_channel[channel].buffer_index =                              \
     (gbc_sound_buffer_index + buffer_adjust) % BUFFER_SIZE;                  \
  }                                                                           \

static void trigger_timer(u32 timer_number, u32 value)
{
   if (value & 0x80)
   {
      if(timer[timer_number].status == TIMER_INACTIVE)
      {
         u32 prescale = prescale_table[value & 0x03];
         u32 timer_reload = timer[timer_number].reload;

         if((value >> 2) & 0x01)
            timer[timer_number].status = TIMER_CASCADE;
         else
            timer[timer_number].status = TIMER_PRESCALE;

         timer[timer_number].prescale = prescale;
         timer[timer_number].irq = ((value >> 6) & 0x1);

         write_ioreg(REG_TMXD(timer_number), (u32)(-timer_reload));

         timer_reload <<= prescale;
         timer[timer_number].count = timer_reload;

         if(timer_reload < execute_cycles)
            execute_cycles = timer_reload;

         if(timer_number < 2)
         {
            u32 buffer_adjust =
               (u32)(((float)(cpu_ticks - gbc_sound_last_cpu_ticks) *
                        sound_frequency) / GBC_BASE_RATE) * 2;

            sound_update_frequency_step(timer_number);
            adjust_sound_buffer(timer_number, 0);
            adjust_sound_buffer(timer_number, 1);
         }
      }
   }
   else
   {
      if(timer[timer_number].status != TIMER_INACTIVE)
      {
         timer[timer_number].status = TIMER_INACTIVE;
      }
   }
   write_ioreg(REG_TMXCNT(timer_number), value);
}

/* Memory timings */
const u8 ws012_nonseq[] = {4, 3, 2, 8};
const u8 ws0_seq[] = {2, 1};
const u8 ws1_seq[] = {4, 1};
const u8 ws2_seq[] = {8, 1};

/* Divided by region and bus width (16/32) */
u8 ws_cyc_seq[16][2] =
{
  { 1, 1 }, // BIOS
  { 1, 1 }, // Invalid
  { 3, 6 }, // EWRAM (default settings)
  { 1, 1 }, // IWRAM
  { 1, 1 }, // IO Registers
  { 1, 2 }, // Palette RAM
  { 1, 2 }, // VRAM
  { 1, 2 }, // OAM
  { 0, 0 }, // Gamepak (wait 0)
  { 0, 0 }, // Gamepak (wait 0)
  { 0, 0 }, // Gamepak (wait 1)
  { 0, 0 }, // Gamepak (wait 1)
  { 0, 0 }, // Gamepak (wait 2)
  { 0, 0 }, // Gamepak (wait 2)
  { 1, 1 }, // Invalid
  { 1, 1 }, // Invalid
};
u8 ws_cyc_nseq[16][2] =
{
  { 1, 1 }, // BIOS
  { 1, 1 }, // Invalid
  { 3, 6 }, // EWRAM (default settings)
  { 1, 1 }, // IWRAM
  { 1, 1 }, // IO Registers
  { 1, 2 }, // Palette RAM
  { 1, 2 }, // VRAM
  { 1, 2 }, // OAM
  { 0, 0 }, // Gamepak (wait 0)
  { 0, 0 }, // Gamepak (wait 0)
  { 0, 0 }, // Gamepak (wait 1)
  { 0, 0 }, // Gamepak (wait 1)
  { 0, 0 }, // Gamepak (wait 2)
  { 0, 0 }, // Gamepak (wait 2)
  { 1, 1 }, // Invalid
  { 1, 1 }, // Invalid
};

const u32 def_seq_cycles[16][2] =
{
  { 1, 1 }, // BIOS
  { 1, 1 }, // Invalid
  { 3, 6 }, // EWRAM (default settings)
  { 1, 1 }, // IWRAM
  { 1, 1 }, // IO Registers
  { 1, 2 }, // Palette RAM
  { 1, 2 }, // VRAM
  { 1, 2 }, // OAM
  { 3, 6 }, // Gamepak (wait 0)
  { 3, 6 }, // Gamepak (wait 0)
  { 5, 9 }, // Gamepak (wait 1)
  { 5, 9 }, // Gamepak (wait 1)
  { 9, 17 }, // Gamepak (wait 2)
  { 9, 17 }, // Gamepak (wait 2)
};

static u8 *bios_rom_alloc; // [1024 * 16];
const u8 *bios_rom = open_gba_bios_rom; // [1024 * 16];

#ifndef RETRO_GO
// Up to 128kb, store SRAM, flash ROM, or EEPROM here.
u8 gamepak_backup[1024 * 128];
#endif

u32 dma_bus_val;
dma_transfer_type dma[4];

// ROM memory is allocated in blocks of 1MB to better map the native block
// mapping system. We will try to allocate 32 of them to allow loading
// ROMs up to 32MB, but we might fail on memory constrained systems.

u8 *gamepak_buffers[32];    /* Pointers to malloc'ed blocks */
u32 gamepak_buffer_count;   /* Value between 1 and 32 */
u32 gamepak_size;           /* Size of the ROM in bytes */
// We allocate in 1MB chunks.
const unsigned gamepak_buffer_blocksize = 1024*1024;

// LRU queue with the loaded blocks and what they map to
struct {
  u16 next_lru;             /* Index in the struct to the next LRU entry */
  s16 phy_rom;              /* ROM page number (-1 means not mapped) */
} gamepak_blk_queue[1024];

u16 gamepak_lru_head;
u16 gamepak_lru_tail;

// Stick page bit: prevents page eviction for a frame. This is used to prevent
// unmapping code pages while being used (ie. in the interpreter).
u32 gamepak_sticky_bit[1024/32];

#define gamepak_sb_test(idx) \
 (gamepak_sticky_bit[((unsigned)(idx)) >> 5] & (1 << (((unsigned)(idx)) & 31)))

// This is global so that it can be kept open for large ROMs to swap
// pages from, so there's no slowdown with opening and closing the file
// a lot.
FILE *gamepak_file_large = NULL;

// Writes to these respective locations should trigger an update
// so the related subsystem may react to it.

// If the GBC audio waveform is modified:
u32 gbc_sound_wave_update = 0;

u32 backup_type = BACKUP_UNKN;
u32 backup_type_reset = BACKUP_UNKN;
u32 flash_mode = FLASH_BASE_MODE;
u32 flash_command_position = 0;
u32 flash_bank_num;  // 0 or 1
u32 flash_bank_cnt;

u32 flash_device_id = FLASH_DEVICE_MACRONIX_64KB;

void reload_timing_info()
{
  int i;
  uint16_t waitcnt = read_ioreg(REG_WAITCNT);

  /* Sequential 16 and 32 bit accesses to ROM */
  ws_cyc_seq[0x8][0] = ws_cyc_seq[0x9][0] = 1 + ws0_seq[(waitcnt >>  4) & 1];
  ws_cyc_seq[0xA][0] = ws_cyc_seq[0xB][0] = 1 + ws1_seq[(waitcnt >>  7) & 1];
  ws_cyc_seq[0xC][0] = ws_cyc_seq[0xD][0] = 1 + ws2_seq[(waitcnt >> 10) & 1];

  for (i = 0x8; i <= 0xD; i++)
  {
    /* 32 bit accesses just cost double due to 16 bit bus */
    ws_cyc_seq[i][1] = ws_cyc_seq[i][0] * 2;
  }

  /* Sequential 16 and 32 bit accesses to ROM */
  ws_cyc_nseq[0x8][0] = ws_cyc_nseq[0x9][0] = 1 + ws012_nonseq[(waitcnt >> 2) & 3];
  ws_cyc_nseq[0xA][0] = ws_cyc_nseq[0xB][0] = 1 + ws012_nonseq[(waitcnt >> 5) & 3];
  ws_cyc_nseq[0xC][0] = ws_cyc_nseq[0xD][0] = 1 + ws012_nonseq[(waitcnt >> 8) & 3];

  for (i = 0x8; i <= 0xD; i++)
  {
    /* 32 bit accesses are a non-seq (16) + seq access (16) */
    ws_cyc_nseq[i][1] = 1 + ws_cyc_nseq[i][0] + ws_cyc_seq[i][0];
  }
}

u8 read_backup(u32 address)
{
  u8 value = 0;

  if(backup_type == BACKUP_EEPROM)
    return 0xff;

  if(backup_type == BACKUP_UNKN)
    backup_type = BACKUP_SRAM;

  if(backup_type == BACKUP_SRAM)
    value = gamepak_backup[address];
  else if(flash_mode == FLASH_ID_MODE)
  {
    if (flash_bank_cnt == FLASH_SIZE_128KB)
    {
      /* ID manufacturer type */
      if(address == 0x0000)
        value = FLASH_MANUFACTURER_MACRONIX;
      /* ID device type */
      else if(address == 0x0001)
        value = FLASH_DEVICE_MACRONIX_128KB;
    }
    else
    {
      /* ID manufacturer type */
      if(address == 0x0000)
        value = FLASH_MANUFACTURER_PANASONIC;
      /* ID device type */
      else if(address == 0x0001)
        value = FLASH_DEVICE_PANASONIC_64KB;
    }
  }
  else
  {
    u32 fulladdr = address + 64*1024*flash_bank_num;
    value = gamepak_backup[fulladdr];
  }

  return value;
}

#define read_backup8()                                                        \
  value = read_backup(address & 0xFFFF);                                      \

#define read_backup16()                                                       \
  value = read_backup(address & 0xFFFF);                                      \
  value = value | (value << 8);

#define read_backup32()                                                       \
  value = read_backup(address & 0xFFFF);                                      \
  value = value | (value << 8);                                               \
  value = value | (value << 16);

#define write_eeprom8(addr, value)

#define write_eeprom16(addr, value)                                           \
  write_eeprom(addr, value)

#define write_eeprom32(addr, value)

// EEPROM is 512 bytes by default; it is autodetecte as 8KB if
// 14bit address DMAs are made (this is done in the DMA handler).

u32 eeprom_size = EEPROM_512_BYTE;
u32 eeprom_mode = EEPROM_BASE_MODE;
u32 eeprom_address = 0;
u32 eeprom_counter = 0;

void function_cc write_eeprom(u32 unused_address, u32 value)
{
  switch(eeprom_mode)
  {
    case EEPROM_BASE_MODE:
      backup_type = BACKUP_EEPROM;
      eeprom_address |= (value & 0x01) << (1 - eeprom_counter);
      if(++eeprom_counter == 2)
      {
        switch(eeprom_address & 0x03)
        {
          case 0x02:
            eeprom_mode = EEPROM_WRITE_ADDRESS_MODE;
            break;

          case 0x03:
            eeprom_mode = EEPROM_ADDRESS_MODE;
            break;
        }
        eeprom_counter = 0;
        eeprom_address = 0;
      }
      break;

    case EEPROM_ADDRESS_MODE:
    case EEPROM_WRITE_ADDRESS_MODE:
      eeprom_address |= (value & 0x01) << (15 - (eeprom_counter % 16));

      if(++eeprom_counter == (eeprom_size == EEPROM_512_BYTE ? 6 : 14))
      {
        eeprom_counter = 0;

        if (eeprom_size == EEPROM_512_BYTE)
          eeprom_address >>= 10;   // Addr is just 6 bits (drop 10LSB)
        else
          eeprom_address >>= 2;    // Addr is 14 bits (drop 2LSB)

        eeprom_address <<= 3;   // EEPROM accessed in blocks of 8 bytes

        if(eeprom_mode == EEPROM_ADDRESS_MODE)
          eeprom_mode = EEPROM_ADDRESS_FOOTER_MODE;
        else
        {
          eeprom_mode = EEPROM_WRITE_MODE;
          memset(gamepak_backup + eeprom_address, 0, 8);
        }
      }
      break;

    case EEPROM_WRITE_MODE:
      gamepak_backup[eeprom_address + (eeprom_counter / 8)] |=
       (value & 0x01) << (7 - (eeprom_counter % 8));
      eeprom_counter++;
      if(eeprom_counter == 64)
      {
        eeprom_counter = 0;
        eeprom_mode = EEPROM_WRITE_FOOTER_MODE;
      }
      break;

    case EEPROM_ADDRESS_FOOTER_MODE:
    case EEPROM_WRITE_FOOTER_MODE:
      eeprom_counter = 0;
      if(eeprom_mode == EEPROM_ADDRESS_FOOTER_MODE)
        eeprom_mode = EEPROM_READ_HEADER_MODE;
      else
        eeprom_mode = EEPROM_BASE_MODE;
      break;

    default:
      break;
  }
}

#define read_memory_gamepak(type)                                             \
  u32 gamepak_index = address >> 15;                                          \
  u8 *map = memory_map_read[gamepak_index];                                   \
                                                                              \
  if(!map)                                                                    \
    map = load_gamepak_page(gamepak_index & 0x3FF);                           \
                                                                              \
  value = readaddress##type(map, address & 0x7FFF)                            \


#define unmapped_rom_read8(addr)                                              \
  (((addr) >> 1) >> (((addr) & 1) * 8)) & 0xFF

#define unmapped_rom_read16(addr)                                             \
  ((addr) >> 1) & 0xFFFF

#define unmapped_rom_read32(addr)                                             \
  ((((addr) & ~3) >> 1) & 0xFFFF) | (((((addr) & ~3) + 2) >> 1) << 16)

#define read_open8()                                                          \
  if(!(reg[REG_CPSR] & 0x20))                                                 \
    value = read_memory8(reg[REG_PC] + 8 + (address & 0x03));                 \
  else                                                                        \
    value = read_memory8(reg[REG_PC] + 4 + (address & 0x01))                  \

#define read_open16()                                                         \
  if(!(reg[REG_CPSR] & 0x20))                                                 \
    value = read_memory16(reg[REG_PC] + 8 + (address & 0x02));                \
  else                                                                        \
    value = read_memory16(reg[REG_PC] + 4)                                    \

#define read_open32()                                                         \
  if(!(reg[REG_CPSR] & 0x20))                                                 \
    value = read_memory32(reg[REG_PC] + 8);                                   \
  else                                                                        \
  {                                                                           \
    u32 current_instruction = read_memory16(reg[REG_PC] + 4);                 \
    value = current_instruction | (current_instruction << 16);                \
  }                                                                           \

u32 function_cc read_eeprom(void)
{
  u32 value;

  switch(eeprom_mode)
  {
    case EEPROM_BASE_MODE:
      value = 1;
      break;

    case EEPROM_READ_MODE:
      value = (gamepak_backup[eeprom_address + (eeprom_counter / 8)] >>
       (7 - (eeprom_counter % 8))) & 0x01;
      eeprom_counter++;
      if(eeprom_counter == 64)
      {
        eeprom_counter = 0;
        eeprom_mode = EEPROM_BASE_MODE;
      }
      break;

    case EEPROM_READ_HEADER_MODE:
      value = 0;
      eeprom_counter++;
      if(eeprom_counter == 4)
      {
        eeprom_mode = EEPROM_READ_MODE;
        eeprom_counter = 0;
      }
      break;

    default:
      value = 0;
      break;
  }

  return value;
}


#define read_memory(type)                                                     \
  switch(address >> 24)                                                       \
  {                                                                           \
    case 0x00:                                                                \
      /* BIOS */                                                              \
      if (address < 0x4000) {                                                 \
        if(reg[REG_PC] >= 0x4000)                                             \
          value = (u##type)(reg[REG_BUS_VALUE] >> ((address & 0x03) << 3));   \
        else                                                                  \
          value = readaddress##type(bios_rom, address & 0x3FFF);              \
      } else {                                                                \
        read_open##type();                                                    \
      }                                                                       \
      break;                                                                  \
                                                                              \
    case 0x02:                                                                \
      /* external work RAM */                                                 \
      value = readaddress##type(ewram, (address & 0x3FFFF));                  \
      break;                                                                  \
                                                                              \
    case 0x03:                                                                \
      /* internal work RAM */                                                 \
      value = readaddress##type(iwram, (address & 0x7FFF) + (0x8000 * SMC_DETECTION));\
      break;                                                                  \
                                                                              \
    case 0x04:                                                                \
      /* I/O registers */                                                     \
      value = readaddress##type(io_registers, address & 0x3FF);               \
      break;                                                                  \
                                                                              \
    case 0x05:                                                                \
      /* palette RAM */                                                       \
      value = readaddress##type(palette_ram, address & 0x3FF);                \
      break;                                                                  \
                                                                              \
    case 0x06:                                                                \
      /* VRAM */                                                              \
      address &= 0x1FFFF;                                                     \
      if(address >= 0x18000)                                                  \
        address -= 0x8000;                                                    \
                                                                              \
      value = readaddress##type(vram, address);                               \
      break;                                                                  \
                                                                              \
    case 0x07:                                                                \
      /* OAM RAM */                                                           \
      value = readaddress##type(oam_ram, address & 0x3FF);                    \
      break;                                                                  \
                                                                              \
    case 0x0D:                                                                \
      if (backup_type == BACKUP_EEPROM) {                                     \
        value = read_eeprom();                                                \
        break;                                                                \
      }                                                                       \
      /* fallthrough */                                                       \
    case 0x08:                                                                \
    case 0x09:                                                                \
    case 0x0A:                                                                \
    case 0x0B:                                                                \
    case 0x0C:                                                                \
      /* gamepak ROM */                                                       \
      if((address & 0x1FFFFFF) >= gamepak_size)                               \
        value = unmapped_rom_read##type(address);                             \
      else                                                                    \
      {                                                                       \
        read_memory_gamepak(type);                                            \
      }                                                                       \
      break;                                                                  \
                                                                              \
    case 0x0E:                                                                \
    case 0x0F:                                                                \
      read_backup##type();                                                    \
      break;                                                                  \
                                                                              \
    default:                                                                  \
      read_open##type();                                                      \
      break;                                                                  \
  }                                                                           \

static cpu_alert_type trigger_dma(u32 dma_number, u32 value)
{
  if(value & 0x8000)
  {
    if(dma[dma_number].start_type == DMA_INACTIVE)
    {
      u32 start_type = ((value >> 12) & 0x03);
      u32 src_address = 0xFFFFFFF & (read_dmareg(REG_DMA0SAD, dma_number) |
                         (read_dmareg(REG_DMA0SAD + 1, dma_number) << 16));
      u32 dst_address = 0xFFFFFFF & (read_dmareg(REG_DMA0DAD, dma_number) |
                         (read_dmareg(REG_DMA0DAD + 1, dma_number) << 16));

      dma[dma_number].source_address = src_address;
      dma[dma_number].dest_address = dst_address;
      dma[dma_number].source_direction = ((value >>  7) & 3);
      dma[dma_number].repeat_type = ((value >> 9) & 0x01);
      dma[dma_number].start_type = start_type;
      dma[dma_number].irq = ((value >> 14) & 0x1);

      /* If it is sound FIFO DMA make sure the settings are a certain way */
      if((dma_number >= 1) && (dma_number <= 2) &&
       (start_type == DMA_START_SPECIAL))
      {
        dma[dma_number].length_type = DMA_32BIT;
        dma[dma_number].length = 4;
        dma[dma_number].dest_direction = DMA_FIXED;
        if(dst_address == 0x40000A4)
          dma[dma_number].direct_sound_channel = DMA_DIRECT_SOUND_B;
        else
          dma[dma_number].direct_sound_channel = DMA_DIRECT_SOUND_A;
      }
      else
      {
        u32 length = read_dmareg(REG_DMA0CNT_L, dma_number);

        if((dma_number == 3) && ((dst_address >> 24) == 0x0D) &&
         ((length & 0x1F) == 17))
          eeprom_size = EEPROM_8_KBYTE;

        if(dma_number < 3)
          length &= 0x3FFF;

        if(length == 0)
        {
          if(dma_number == 3)
            length = 0x10000;
          else
            length = 0x04000;
        }

        dma[dma_number].length = length;
        dma[dma_number].length_type = ((value >> 10) & 0x01);
        dma[dma_number].dest_direction = ((value >> 5) & 3);
      }

      write_dmareg(REG_DMA0CNT_H, dma_number, value);
      if(start_type == DMA_START_IMMEDIATELY) {
        // Excutes the DMA now! Copies the data and returns side effects.
        int dma_cycles = 0;
        cpu_alert_type ret = dma_transfer(dma_number, &dma_cycles);
        if (!dma_cycles)
          return ret;
        // Sleep CPU for N cycles and return HALT as side effect (so it does).
        reg[CPU_HALT_STATE] = CPU_DMA;
        reg[REG_SLEEP_CYCLES] = 0x80000000 | (u32)dma_cycles;
        return CPU_ALERT_HALT | ret;
      }
    }
  }
  else
  {
    dma[dma_number].start_type = DMA_INACTIVE;
    dma[dma_number].direct_sound_channel = DMA_NO_DIRECT_SOUND;
    write_dmareg(REG_DMA0CNT_H, dma_number, value);
  }

  return CPU_ALERT_NONE;
}

static inline s32 signext28(u32 value)
{
  s32 ret = (s32)(value << 4);
  return ret >> 4;
}

cpu_alert_type function_cc write_io_register16(u32 address, u32 value)
{
  uint16_t ioreg = (address & 0x3FE) >> 1;
  value &= 0xffff;
  switch(ioreg)
  {
    case REG_DISPCNT:
      // Changing the lowest 3 bits might require object re-sorting
      reg[OAM_UPDATED] |= ((value & 0x07) != (read_ioreg(REG_DISPCNT) & 0x07));
      write_ioreg(REG_DISPCNT, value);
      break;

    // DISPSTAT has 3 read only bits, controlled by the LCD controller
    case REG_DISPSTAT:
      write_ioreg(REG_DISPSTAT, (read_ioreg(REG_DISPSTAT) & 0x07) | (value & ~0x07));
      break;

    // BG2 reference X
    case REG_BG2X_L:
    case REG_BG2X_H:
      write_ioreg(ioreg, value);
      affine_reference_x[0] = signext28(read_ioreg32(REG_BG2X_L));
      break;

    // BG2 reference Y
    case REG_BG2Y_L:
    case REG_BG2Y_H:
      write_ioreg(ioreg, value);
      affine_reference_y[0] = signext28(read_ioreg32(REG_BG2Y_L));
      break;

    // BG3 reference X
    case REG_BG3X_L:
    case REG_BG3X_H:
      write_ioreg(ioreg, value);
      affine_reference_x[1] = signext28(read_ioreg32(REG_BG3X_L));
      break;

    // BG3 reference Y
    case REG_BG3Y_L:
    case REG_BG3Y_H:
      write_ioreg(ioreg, value);
      affine_reference_y[1] = signext28(read_ioreg32(REG_BG3Y_L));
      break;

    // Sound 1 registers
    case REG_SOUND1CNT_L:    // control sweep
      gbc_sound_tone_control_sweep();
      break;

    case REG_SOUND1CNT_H:    // control duty/length/envelope
      gbc_sound_tone_control_low(0, REG_SOUND1CNT_H);
      break;

    case REG_SOUND1CNT_X:    // control frequency
      gbc_sound_tone_control_high(0, REG_SOUND1CNT_X);
      break;

    // Sound 2 registers
    case REG_SOUND2CNT_L:    // control duty/length/envelope
      gbc_sound_tone_control_low(1, REG_SOUND2CNT_L);
      break;

    case REG_SOUND2CNT_H:    // control frequency
      gbc_sound_tone_control_high(1, REG_SOUND2CNT_H);
      break;

    // Sound 3 registers
    case REG_SOUND3CNT_L:    // control wave
      gbc_sound_wave_control();
      break;

    case REG_SOUND3CNT_H:    // control length/volume
      gbc_sound_tone_control_low_wave();
      break;

    case REG_SOUND3CNT_X:    // control frequency
      gbc_sound_tone_control_high_wave();
      break;

    // Sound 4 registers
    case REG_SOUND4CNT_L:    // length/envelope
      gbc_sound_tone_control_low(3, REG_SOUND4CNT_L);
      break;

    case REG_SOUND4CNT_H:    // control frequency
      gbc_sound_noise_control();
      break;

    // Sound control registers
    case REG_SOUNDCNT_L:
      gbc_trigger_sound(value);
      break;

    case REG_SOUNDCNT_H:
      trigger_sound();
      break;

    case REG_SOUNDCNT_X:
      sound_control_x(value);
      break;

    // Sound wave RAM, flag wave table update
    case REG_SOUNDWAVE_0 ... REG_SOUNDWAVE_7:
      render_gbc_sound();
      gbc_sound_wave_update = 1;
      write_ioreg(ioreg, value);
      break;

    // DMA control register: can cause an IRQ
    case REG_DMA0CNT_H: return trigger_dma(0, value);
    case REG_DMA1CNT_H: return trigger_dma(1, value);
    case REG_DMA2CNT_H: return trigger_dma(2, value);
    case REG_DMA3CNT_H: return trigger_dma(3, value);

    // Timer counter reload
    case REG_TM0D: count_timer(0); break;
    case REG_TM1D: count_timer(1); break;
    case REG_TM2D: count_timer(2); break;
    case REG_TM3D: count_timer(3); break;

    /* Timer control register (0..3)*/
    case REG_TM0CNT: trigger_timer(0, value); break;
    case REG_TM1CNT: trigger_timer(1, value); break;
    case REG_TM2CNT: trigger_timer(2, value); break;
    case REG_TM3CNT: trigger_timer(3, value); break;

    // Serial port registers
    case REG_SIOCNT:
      return write_siocnt(value);

    case REG_RCNT:
      return write_rcnt(value);

    // Interrupt flag, clears the bits it tries to write
    case REG_IF:
      write_ioreg(REG_IF, read_ioreg(REG_IF) & (~value));
      break;

    // Register writes with side-effects, can raise an IRQ
    case REG_IE:
    case REG_IME:
      write_ioreg(ioreg, value);
      return check_interrupt();

    // Read-only registers
    case REG_P1:
    case REG_VCOUNT:
      break;  // Do nothing

    case REG_WAITCNT:
      write_ioreg(REG_WAITCNT, value);
      reload_timing_info();
      break;

    // Registers without side effects
    default:
      write_ioreg(ioreg, value);
      break;
  }

  return CPU_ALERT_NONE;
}

cpu_alert_type function_cc write_io_register8(u32 address, u32 value)
{
  if (address == 0x301) {
    if (value & 1)
      reg[CPU_HALT_STATE] = CPU_STOP;
    else
      reg[CPU_HALT_STATE] = CPU_HALT;
    return CPU_ALERT_HALT;
  }

  // Partial 16 bit write, treat like a regular merge-write
  if (address & 1)
    value = (value << 8) | (read_ioreg(address >> 1) & 0x00ff);
  else
    value = (value & 0xff) | (read_ioreg(address >> 1) & 0xff00);
  return write_io_register16(address & 0x3FE, value);
}

cpu_alert_type function_cc write_io_register32(u32 address, u32 value)
{
  // Handle sound FIFO data write
  if (address == 0xA0) {
    sound_timer_queue32(0, value);
    return CPU_ALERT_NONE;
  }
  else if (address == 0xA4) {
    sound_timer_queue32(1, value);
    return CPU_ALERT_NONE;
  }

  // Perform two 16 bit writes. Low part goes first apparently.
  // Some Count+Control DMA writes use 32 bit, so control must be last.
  cpu_alert_type allow = write_io_register16(address, value & 0xFFFF);
  cpu_alert_type alhigh = write_io_register16(address + 2, value >> 16);
  return allow | alhigh;
}

#define write_palette8(address, value)                                        \
{                                                                             \
  u32 aladdr = address & ~1U;                                                 \
  u16 val16 = (value << 8) | value;                                           \
  address16(palette_ram, aladdr) = eswap16(val16);                            \
  address16(palette_ram_converted, aladdr) = convert_palette(val16);          \
}

#define write_palette16(address, value)                                       \
{                                                                             \
  u32 palette_address = address;                                              \
  address16(palette_ram, palette_address) = eswap16(value);                   \
  value = convert_palette(value);                                             \
  address16(palette_ram_converted, palette_address) = value;                  \
}                                                                             \

#define write_palette32(address, value)                                       \
{                                                                             \
  u32 palette_address = address;                                              \
  u32 value_high = value >> 16;                                               \
  u32 value_low = value & 0xFFFF;                                             \
  address32(palette_ram, palette_address) = eswap32(value);                   \
  value_high = convert_palette(value_high);                                   \
  address16(palette_ram_converted, palette_address + 2) = value_high;         \
  value_low = convert_palette(value_low);                                     \
  address16(palette_ram_converted, palette_address) = value_low;              \
}                                                                             \


void function_cc write_backup(u32 address, u32 value)
{
  value &= 0xFF;

  if(backup_type == BACKUP_EEPROM)
    return;

  if(backup_type == BACKUP_UNKN)
    backup_type = BACKUP_SRAM;

  // gamepak SRAM or Flash ROM
  if((address == 0x5555) && (flash_mode != FLASH_WRITE_MODE))
  {
    if((flash_command_position == 0) && (value == 0xAA))
    {
      backup_type = BACKUP_FLASH;
      flash_command_position = 1;
    }

    if(flash_command_position == 2)
    {
      switch(value)
      {
        case 0x90:
          // Enter ID mode, this also tells the emulator that we're using
          // flash, not SRAM

          if(flash_mode == FLASH_BASE_MODE)
            flash_mode = FLASH_ID_MODE;

          break;

        case 0x80:
          // Enter erase mode
          if(flash_mode == FLASH_BASE_MODE)
            flash_mode = FLASH_ERASE_MODE;
          break;

        case 0xF0:
          // Terminate ID mode
          if(flash_mode == FLASH_ID_MODE)
            flash_mode = FLASH_BASE_MODE;
          break;

        case 0xA0:
          // Write mode
          if(flash_mode == FLASH_BASE_MODE)
            flash_mode = FLASH_WRITE_MODE;
          break;

        case 0xB0:
          // Bank switch
          // Here the chip is now officially 128KB.
          flash_bank_cnt = FLASH_SIZE_128KB;
          if(flash_mode == FLASH_BASE_MODE)
            flash_mode = FLASH_BANKSWITCH_MODE;
          break;

        case 0x10:
          // Erase chip
          if(flash_mode == FLASH_ERASE_MODE)
          {
            memset(gamepak_backup, 0xFF, 1024 * 128);
            flash_mode = FLASH_BASE_MODE;
          }
          break;

        default:
          break;
      }
      flash_command_position = 0;
    }
    if(backup_type == BACKUP_SRAM)
      gamepak_backup[0x5555] = value;
  }
  else

  if((address == 0x2AAA) && (value == 0x55) &&
   (flash_command_position == 1))
    flash_command_position = 2;
  else
  {
    if((flash_command_position == 2) &&
     (flash_mode == FLASH_ERASE_MODE) && (value == 0x30))
    {
      // Erase sector
      u32 fulladdr = (address & 0xF000) + 64*1024*flash_bank_num;
      memset(&gamepak_backup[fulladdr], 0xFF, 1024 * 4);
      flash_mode = FLASH_BASE_MODE;
      flash_command_position = 0;
    }
    else

    if((flash_command_position == 0) &&
     (flash_mode == FLASH_BANKSWITCH_MODE) && (address == 0x0000) &&
     (flash_bank_cnt == FLASH_SIZE_128KB))
    {
      flash_bank_num = value & 1;
      flash_mode = FLASH_BASE_MODE;
    }
    else

    if((flash_command_position == 0) && (flash_mode == FLASH_WRITE_MODE))
    {
      // Write value to flash ROM
      u32 fulladdr = address + 64*1024*flash_bank_num;
      gamepak_backup[fulladdr] = value;
      flash_mode = FLASH_BASE_MODE;
    }
    else

    if(backup_type == BACKUP_SRAM)
    {
      // Write value to SRAM
      gamepak_backup[address] = value;
    }
  }
}

#define write_backup8()                                                       \
  write_backup(address & 0xFFFF, value)                                       \

#define write_backup16()                                                      \

#define write_backup32()                                                      \

#define write_vram8()                                                         \
  address &= ~0x01;                                                           \
  address16(vram, address) = eswap16((value << 8) | value)                    \

#define write_vram16()                                                        \
  address16(vram, address) = eswap16(value)                                   \

#define write_vram32()                                                        \
  address32(vram, address) = eswap32(value)                                   \

// RTC code derived from VBA's (due to lack of any real publically available
// documentation...)

#define RTC_DISABLED                  0
#define RTC_IDLE                      1
#define RTC_COMMAND                   2
#define RTC_OUTPUT_DATA               3
#define RTC_INPUT_DATA                4

typedef enum
{
  RTC_COMMAND_RESET            = 0x60,
  RTC_COMMAND_WRITE_STATUS     = 0x62,
  RTC_COMMAND_READ_STATUS      = 0x63,
  RTC_COMMAND_OUTPUT_TIME_FULL = 0x65,
  RTC_COMMAND_OUTPUT_TIME      = 0x67
} rtc_command_type;

#define RTC_WRITE_TIME                0
#define RTC_WRITE_TIME_FULL           1
#define RTC_WRITE_STATUS              2

static bool rtc_enabled = false, rumble_enabled = false;

// I/O registers (for RTC, rumble, etc)
u8 gpio_regs[3];

// RTC tracking variables
u32 rtc_state = RTC_DISABLED;
u32 rtc_write_mode;
u32 rtc_command;
u64 rtc_data;
u32 rtc_data_bits;
u32 rtc_status = 0x40;
s32 rtc_bit_count;

// Rumble trackin vars, not really preserved (it's just aproximate)
static u32 rumble_enable_tick, rumble_ticks;

static u8 encode_bcd(u8 value)
{
  int l = 0;
  int h = 0;

  value = value % 100;
  l = value % 10;
  h = value / 10;

  return h * 16 + l;
}

void update_gpio_romregs() {
  if (rtc_enabled || rumble_enabled) {
    // Update the registers in the ROM mapped buffer.
    u8 *map = memory_map_read[0x8000000 >> 15];
    if (map) {
      if (gpio_regs[2]) {
        // Registers are visible, readable:
        address16(map, 0xC4) = eswap16(gpio_regs[0]);
        address16(map, 0xC6) = eswap16(gpio_regs[1]);
        address16(map, 0xC8) = eswap16(gpio_regs[2]);
      } else {
        // Registers are write-only, just read out zero
        address16(map, 0xC4) = 0;
        address16(map, 0xC6) = 0;
        address16(map, 0xC8) = 0;
      }
    }
  }
}

#define GPIO_RTC_CLK   0x1
#define GPIO_RTC_DAT   0x2
#define GPIO_RTC_CSS   0x4

static void write_rtc(u8 old, u8 new)
{
  // RTC works using a high CS and falling edge capture for the clock signal.
  if (!(new & GPIO_RTC_CSS)) {
    // Chip select is down, reset the RTC protocol. And do not process input.
    rtc_state = RTC_IDLE;
    rtc_command = 0;
    rtc_bit_count = 0;
    return;
  }

  // CS low to high transition!
  if (!(old & GPIO_RTC_CSS))
    rtc_state = RTC_COMMAND;

  if ((old & GPIO_RTC_CLK) && !(new & GPIO_RTC_CLK)) {
    // Advance clock state, input/ouput data.
    switch (rtc_state) {
    case RTC_COMMAND:
      rtc_command <<= 1;
      rtc_command |= ((new >> 1) & 1);
      // 8 bit command read, process:
      if (++rtc_bit_count == 8) {
        switch (rtc_command) {
        case RTC_COMMAND_RESET:
        case RTC_COMMAND_WRITE_STATUS:
          rtc_state = RTC_INPUT_DATA;
          rtc_data = 0;
          rtc_data_bits = 8;
          rtc_write_mode = RTC_WRITE_STATUS;
          break;
        case RTC_COMMAND_READ_STATUS:
          rtc_state = RTC_OUTPUT_DATA;
          rtc_data_bits = 8;
          rtc_data = rtc_status;
          break;
        case RTC_COMMAND_OUTPUT_TIME_FULL:
          {
            struct tm *current_time;
            time_t current_time_flat;
            time(&current_time_flat);
            current_time = localtime(&current_time_flat);

            rtc_state = RTC_OUTPUT_DATA;
            rtc_data_bits = 56;
            rtc_data = ((u64)encode_bcd(current_time->tm_year)) |
                       ((u64)encode_bcd(current_time->tm_mon+1)<< 8) |
                       ((u64)encode_bcd(current_time->tm_mday) << 16) |
                       ((u64)encode_bcd(current_time->tm_wday) << 24) |
                       ((u64)encode_bcd(current_time->tm_hour) << 32) |
                       ((u64)encode_bcd(current_time->tm_min)  << 40) |
                       ((u64)encode_bcd(current_time->tm_sec)  << 48);
          }
          break;
        case RTC_COMMAND_OUTPUT_TIME:
          {
            struct tm *current_time;
            time_t current_time_flat;
            time(&current_time_flat);
            current_time = localtime(&current_time_flat);

            rtc_state = RTC_OUTPUT_DATA;
            rtc_data_bits = 24;
            rtc_data = (encode_bcd(current_time->tm_hour)) |
                       (encode_bcd(current_time->tm_min) << 8) |
                       (encode_bcd(current_time->tm_sec) << 16);
          }
          break;
        };
        rtc_bit_count = 0;
      }
      break;

    case RTC_INPUT_DATA:
      rtc_data <<= 1;
      rtc_data |= ((new >> 1) & 1);
      rtc_data_bits--;
      if (!rtc_data_bits) {
        rtc_status = rtc_data; // HACK: assuming write status here.
        rtc_state = RTC_IDLE;
      }
      break;

    case RTC_OUTPUT_DATA:
      // Output the next bit from rtc_data
      if (!(gpio_regs[1] & 0x2)) {
        // Only output if the port is set to OUT!
        u32 bit = rtc_data & 1;
        gpio_regs[0] = (new & ~0x2) | ((bit) << 1);
      }
      rtc_data >>= 1;
      rtc_data_bits--;

      if (!rtc_data_bits)
        rtc_state = RTC_IDLE;   // Finish transmission!

      break;
    };
  }
}

void write_rumble(bool oldv, bool newv) {
  if (newv && !oldv)
    rumble_enable_tick = cpu_ticks;
  else if (!newv && oldv) {
    rumble_ticks += (cpu_ticks - rumble_enable_tick);
    rumble_enable_tick = 0;
  }
}

void rumble_frame_reset() {
  // Reset the tick initial value to frame start (only if active)
  rumble_ticks = 0;
  if (rumble_enable_tick)
    rumble_enable_tick = cpu_ticks;
}

float rumble_active_pct() {
  // Calculate the percentage of Rumble active for this frame.
  u32 active_ticks = rumble_ticks;
  // If the rumble is still active, account for the due cycles
  if (rumble_enable_tick)
    active_ticks += (cpu_ticks - rumble_enable_tick);

  return active_ticks / (GBC_BASE_RATE / 60);
}

void function_cc write_gpio(u32 address, u32 value) {
  u8 prev_value = gpio_regs[0];
  switch(address) {
  case 0xC4:
    // Any writes do not affect input pins:
    gpio_regs[0] = (gpio_regs[0] & ~gpio_regs[1]) | (value & gpio_regs[1]);
    break;
  case 0xC6:
    gpio_regs[1] = value & 0xF;
    break;
  case 0xC8:   /* I/O port control */
    gpio_regs[2] = value & 1;
    break;
  };

  // If the game has an RTC, ensure it gets the data
  if (rtc_enabled && (prev_value & 0x7) != (gpio_regs[0] & 0x7))
    write_rtc(prev_value & 0x7, gpio_regs[0] & 0x7);

  if (rumble_enabled && (prev_value & 0x8) != (gpio_regs[0] & 0x8))
    write_rumble(prev_value & 0x8, gpio_regs[0] & 0x8);

  // Reflect the values
  update_gpio_romregs();
}

#define write_gpio8()                                                         \

#define write_gpio16()                                                        \
  write_gpio(address & 0xFF, value)                                           \

#define write_gpio32()                                                        \

#define write_memory(type)                                                    \
  switch(address >> 24)                                                       \
  {                                                                           \
    case 0x02:                                                                \
      /* external work RAM */                                                 \
      address##type(ewram, (address & 0x3FFFF)) = eswap##type(value);         \
      break;                                                                  \
                                                                              \
    case 0x03:                                                                \
      /* internal work RAM */                                                 \
      address##type(iwram, (address & 0x7FFF) + (0x8000 * SMC_DETECTION)) = eswap##type(value); \
      break;                                                                  \
                                                                              \
    case 0x04:                                                                \
      /* I/O registers */                                                     \
      return write_io_register##type(address & 0x3FF, value);                 \
                                                                              \
    case 0x05:                                                                \
      /* palette RAM */                                                       \
      write_palette##type(address & 0x3FF, value);                            \
      break;                                                                  \
                                                                              \
    case 0x06:                                                                \
      /* VRAM */                                                              \
      address &= 0x1FFFF;                                                     \
      if(address >= 0x18000)                                                  \
        address -= 0x8000;                                                    \
                                                                              \
      write_vram##type();                                                     \
      break;                                                                  \
                                                                              \
    case 0x07:                                                                \
      /* OAM RAM */                                                           \
      if (type != 8) {                                                        \
        reg[OAM_UPDATED] = 1;                                                 \
        address##type(oam_ram, address & 0x3FF) = eswap##type(value);         \
      }                                                                       \
      break;                                                                  \
                                                                              \
    case 0x08:                                                                \
      /* gamepak ROM or RTC */                                                \
      write_gpio##type();                                                     \
      break;                                                                  \
                                                                              \
    case 0x09:                                                                \
    case 0x0A:                                                                \
    case 0x0B:                                                                \
    case 0x0C:                                                                \
      /* gamepak ROM space */                                                 \
      break;                                                                  \
                                                                              \
    case 0x0D:                                                                \
      write_eeprom##type(address, value);                                     \
      break;                                                                  \
                                                                              \
    case 0x0E:                                                                \
      write_backup##type();                                                   \
      break;                                                                  \
  }                                                                           \

u32 function_cc read_memory8(u32 address)
{
  u8 value;
  read_memory(8);
  return value;
}

u32 function_cc read_memory8s(u32 address) {
  return (u32)((s8)read_memory8(address));
}

u16 function_cc read_memory16_signed(u32 address)
{
  u16 value;

  if(address & 0x01)
    return (s8)read_memory8(address);

  read_memory(16);

  return value;
}

u32 function_cc read_memory16s(u32 address) {
  return (u32)((s16)read_memory16_signed(address));
}

// unaligned reads are actually 32bit

u32 function_cc read_memory16(u32 address)
{
  u32 value;
  bool unaligned = (address & 0x01);
  address &= ~0x01;
  read_memory(16);
  if (unaligned) {
    ror(value, value, 8);
  }

  return value;
}


u32 function_cc read_memory32(u32 address)
{
  u32 value;
  u32 rotate = (address & 0x03) * 8;
  address &= ~0x03;
  read_memory(32);
  ror(value, value, rotate);
  return value;
}

cpu_alert_type function_cc write_memory8(u32 address, u8 value)
{
  write_memory(8);
  return CPU_ALERT_NONE;
}

cpu_alert_type function_cc write_memory16(u32 address, u16 value)
{
  write_memory(16);
  return CPU_ALERT_NONE;
}

cpu_alert_type function_cc write_memory32(u32 address, u32 value)
{
  write_memory(32);
  return CPU_ALERT_NONE;
}

typedef struct
{
   char gamepak_title[13];
   char gamepak_code[5];
   char gamepak_maker[3];
   u16 flags;
   u32 idle_loop_target_pc;
   u32 translation_gate_target_1;
   u32 translation_gate_target_2;
   u32 translation_gate_target_3;
} ini_t;

typedef struct
{
   char gamepak_title[13];
   char gamepak_code[5];
   char gamepak_maker[3];
} gamepak_info_t;

#define FLAGS_FLASH_128KB    0x0001   // Forces 128KB flash.
#define FLAGS_RUMBLE         0x0002   // Enables GPIO3 rumble support.
#define FLAGS_GBA_PLAYER     0x0004   // Enables GBA Player rumble support.
#define FLAGS_RTC            0x0008   // Enables RTC support by default.
#define FLAGS_EEPROM         0x0010   // Forces EEPROM storage.
#define FLAGS_RFU            0x0020   // Enables Wireless Adapter (via serial).

#include "gba_over.h"

static void load_game_config_over(gamepak_info_t *gpinfo)
{
  unsigned i = 0;

  for (i = 0; i < sizeof(gbaover)/sizeof(gbaover[0]); i++)
  {
     if (strcmp(gbaover[i].gamepak_code, gpinfo->gamepak_code))
        continue;

     if (strcmp(gbaover[i].gamepak_title, gpinfo->gamepak_title))
        continue;

     printf("gamepak title: %s\n", gbaover[i].gamepak_title);
     printf("gamepak code : %s\n", gbaover[i].gamepak_code);
     printf("gamepak maker: %s\n", gbaover[i].gamepak_maker);

     printf("INPUT gamepak title: %s\n", gpinfo->gamepak_title);
     printf("INPUT gamepak code : %s\n", gpinfo->gamepak_code);
     printf("INPUT gamepak maker: %s\n", gpinfo->gamepak_maker);

     if (gbaover[i].idle_loop_target_pc != 0)
        idle_loop_target_pc = gbaover[i].idle_loop_target_pc;

     if (gbaover[i].flags & FLAGS_FLASH_128KB) {
       flash_device_id = FLASH_DEVICE_MACRONIX_128KB;
       flash_bank_cnt = FLASH_SIZE_128KB;
     }

     if (gbaover[i].flags & FLAGS_RTC)
       rtc_enabled = true;

     if (gbaover[i].flags & FLAGS_RUMBLE)
       rumble_enabled = true;

     if (gbaover[i].flags & FLAGS_EEPROM)
       backup_type_reset = BACKUP_EEPROM;

     if (serial_mode == SERIAL_MODE_AUTO) {
       if (gbaover[i].flags & FLAGS_RFU)
         serial_mode = SERIAL_MODE_RFU;
       if (gbaover[i].flags & FLAGS_GBA_PLAYER)
         serial_mode = SERIAL_MODE_GBP;
     }

     if (gbaover[i].translation_gate_target_1 != 0)
     {
        translation_gate_target_pc[translation_gate_targets] = gbaover[i].translation_gate_target_1;
        translation_gate_targets++;
     }

     if (gbaover[i].translation_gate_target_2 != 0)
     {
        translation_gate_target_pc[translation_gate_targets] = gbaover[i].translation_gate_target_2;
        translation_gate_targets++;
     }

     if (gbaover[i].translation_gate_target_3 != 0)
     {
        translation_gate_target_pc[translation_gate_targets] = gbaover[i].translation_gate_target_3;
        translation_gate_targets++;
     }
  }
}

// DMA memory regions can be one of the following:
// IWRAM - 32kb offset from the contiguous iwram region.
// EWRAM - also contiguous but with self modifying code check mirror.
// VRAM - 96kb offset from the contiguous vram region, should take care
// Palette RAM - Converts palette entries when written to.
// OAM RAM - Sets OAM modified flag to true.
// I/O registers - Uses the I/O register function.
// of mirroring properly.
// Segmented RAM/ROM - a region >= 32kb, the translated address has to
//  be reloaded if it wraps around the limit (cartride ROM)
// Ext - should be handled by the memory read/write function.

// The following map determines the region of each (assumes DMA access
// is not done out of bounds)

typedef enum
{
  DMA_REGION_IWRAM        = 0,
  DMA_REGION_EWRAM        = 1,
  DMA_REGION_VRAM         = 2,
  DMA_REGION_PALETTE_RAM  = 3,
  DMA_REGION_OAM_RAM      = 4,
  DMA_REGION_IO           = 5,
  DMA_REGION_EXT          = 6,
  DMA_REGION_GAMEPAK      = 7,
  DMA_REGION_BUS          = 8,
  DMA_REGION_COUNT        = 9
} dma_region_type;

const dma_region_type dma_region_map[17] =
{
  DMA_REGION_BUS,           // 0x00 - BUS
  DMA_REGION_BUS,           // 0x01 - BUS
  DMA_REGION_EWRAM,         // 0x02 - EWRAM
  DMA_REGION_IWRAM,         // 0x03 - IWRAM
  DMA_REGION_IO,            // 0x04 - I/O registers
  DMA_REGION_PALETTE_RAM,   // 0x05 - palette RAM
  DMA_REGION_VRAM,          // 0x06 - VRAM
  DMA_REGION_OAM_RAM,       // 0x07 - OAM RAM
  DMA_REGION_GAMEPAK,       // 0x08 - gamepak ROM
  DMA_REGION_GAMEPAK,       // 0x09 - gamepak ROM
  DMA_REGION_GAMEPAK,       // 0x0A - gamepak ROM
  DMA_REGION_GAMEPAK,       // 0x0B - gamepak ROM
  DMA_REGION_GAMEPAK,       // 0x0C - gamepak ROM
  DMA_REGION_EXT,           // 0x0D - EEPROM
  DMA_REGION_EXT,           // 0x0E - gamepak SRAM/flash ROM
  DMA_REGION_EXT,           // 0x0F - gamepak SRAM/flash ROM
  DMA_REGION_BUS            // 0x10 - Out of region (assuming open bus)
};

#define dma_print(src_op, dest_op, tfsize)                                    \
  printf("dma from %x (%s) to %x (%s) for %x (%s) (%d) (pc %x)\n",            \
   src_ptr, #src_op, dest_ptr, #dest_op, length, #tfsize,                     \
   dma->irq, reg[15]);                                                        \

#define dma_oam_ram_dest()                                                    \
  reg[OAM_UPDATED] = 1                                                        \

#define dma_vars_oam_ram(type)                                                \
  dma_oam_ram_##type()                                                        \

#define dma_vars_iwram(type)
#define dma_vars_ewram(type)
#define dma_vars_io(type)
#define dma_vars_vram(type)
#define dma_vars_palette_ram(type)
#define dma_vars_bus(type)
#define dma_vars_ext(type)

#define dma_oam_ram_src()

#define dma_segmented_load_src()                                              \
  memory_map_read[src_current_region]                                         \

#define dma_vars_gamepak(type)                                                \
  u32 type##_new_region;                                                      \
  u32 type##_current_region = type##_ptr >> 15;                               \
  u8 *type##_address_block = dma_segmented_load_##type();                     \
  if(type##_address_block == NULL)                                            \
  {                                                                           \
    if((type##_ptr & 0x1FFFFFF) >= gamepak_size)                              \
      break;                                                                  \
    type##_address_block = load_gamepak_page(type##_current_region & 0x3FF);  \
  }                                                                           \

#define dma_gamepak_check_region(type)                                        \
  type##_new_region = (type##_ptr >> 15);                                     \
  if(type##_new_region != type##_current_region)                              \
  {                                                                           \
    type##_current_region = type##_new_region;                                \
    type##_address_block = dma_segmented_load_##type();                       \
    if(type##_address_block == NULL)                                          \
    {                                                                         \
      type##_address_block =                                                  \
       load_gamepak_page(type##_current_region & 0x3FF);                      \
    }                                                                         \
  }                                                                           \

#define dma_read_iwram(type, tfsize)                                          \
  read_value = readaddress##tfsize(iwram + (0x8000 * SMC_DETECTION), type##_ptr & 0x7FFF) \

#define dma_read_vram(type, tfsize) {                                         \
  u32 rdaddr = type##_ptr & 0x1FFFF;                                          \
  if (rdaddr >= 0x18000) rdaddr -= 0x8000;                                    \
  read_value = readaddress##tfsize(vram, rdaddr);                             \
}

#define dma_read_io(type, tfsize)                                             \
  read_value = readaddress##tfsize(io_registers, type##_ptr & 0x3FF)          \

#define dma_read_oam_ram(type, tfsize)                                        \
  read_value = readaddress##tfsize(oam_ram, type##_ptr & 0x3FF)               \

#define dma_read_palette_ram(type, tfsize)                                    \
  read_value = readaddress##tfsize(palette_ram, type##_ptr & 0x3FF)           \

#define dma_read_ewram(type, tfsize)                                          \
  read_value = readaddress##tfsize(ewram, type##_ptr & 0x3FFFF)               \

#define dma_read_gamepak(type, tfsize)                                        \
  dma_gamepak_check_region(type);                                             \
  read_value = readaddress##tfsize(type##_address_block,                      \
   type##_ptr & 0x7FFF)                                                       \

// DMAing from the BIOS/open zone causes previous DMA values to be read

#define dma_read_bus(type, tfsize)

#define dma_read_ext(type, tfsize)                                            \
  read_value = read_memory##tfsize(type##_ptr)                                \

#define dma_write_iwram(type, tfsize)                                         \
  address##tfsize(iwram + (0x8000 * SMC_DETECTION), type##_ptr & 0x7FFF) =    \
                                          eswap##tfsize(read_value);          \
  if (SMC_DETECTION && address##tfsize(iwram, type##_ptr & 0x7FFF))           \
    alerts |= CPU_ALERT_SMC;                                                  \

#define dma_write_vram(type, tfsize) {                                        \
  u32 wraddr = type##_ptr & 0x1FFFF;                                          \
  if (wraddr >= 0x18000) wraddr -= 0x8000;                                    \
  address##tfsize(vram, wraddr) = eswap##tfsize(read_value);                  \
}

#define dma_write_io(type, tfsize)                                            \
  alerts |= write_io_register##tfsize(type##_ptr & 0x3FF, read_value)         \

#define dma_write_oam_ram(type, tfsize)                                       \
  address##tfsize(oam_ram, type##_ptr & 0x3FF) = eswap##tfsize(read_value)    \

#define dma_write_palette_ram(type, tfsize)                                   \
  write_palette##tfsize(type##_ptr & 0x3FF, read_value)                       \

#define dma_write_ext(type, tfsize)                                           \
  write_memory##tfsize(type##_ptr, read_value)                                \

#define dma_write_ewram(type, tfsize)                                         \
  address##tfsize(ewram, type##_ptr & 0x3FFFF) = eswap##tfsize(read_value);   \
  if (SMC_DETECTION && address##tfsize(ewram, (type##_ptr & 0x3FFFF) + 0x40000)) \
    alerts |= CPU_ALERT_SMC;                                                  \

#define print_line()                                                          \
  dma_print(src_op, dest_op, tfsize);                                         \

#define dma_tfloop(src_region_type, dest_region_type, src_op, dest_op, tfsize)\
{                                                                             \
  dma_vars_##src_region_type(src);                                            \
  dma_vars_##dest_region_type(dest);                                          \
                                                                              \
  for(i = 0; i < length; i++)                                                 \
  {                                                                           \
    dma_read_##src_region_type(src, tfsize);                                  \
    dma_write_##dest_region_type(dest, tfsize);                               \
    src_ptr += src_op;                                                        \
    dest_ptr += dest_op;                                                      \
  }                                                                           \
  break;                                                                      \
}                                                                             \

#define dma_tf_loop_builder(tfsize)                                           \
                                                                              \
cpu_alert_type dma_tf_loop##tfsize(                                           \
  u32 src_ptr, u32 dest_ptr, int src_strd, int dest_strd,                     \
  bool wb, u32 length, dma_transfer_type *dma)                                \
{                                                                             \
  u32 i;                                                                      \
  u32 read_value = dma_bus_val;                                               \
  cpu_alert_type alerts = CPU_ALERT_NONE;                                     \
  u32 src_region = MIN(src_ptr >> 24, 16);                                    \
  u32 dest_region = MIN(dest_ptr >> 24, 16);                                  \
  dma_region_type src_region_type = dma_region_map[src_region];               \
  dma_region_type dest_region_type = dma_region_map[dest_region];             \
                                                                              \
  switch(src_region_type + dest_region_type * DMA_REGION_COUNT)               \
  {                                                                           \
    default:                                                                  \
      /* Do nothing (read-only destination or unmapped area...) */            \
      return CPU_ALERT_NONE;                                                  \
                                                                              \
    case DMA_REGION_BUS + DMA_REGION_IWRAM * DMA_REGION_COUNT:                \
      dma_tfloop(bus, iwram, src_strd, dest_strd, tfsize);                    \
                                                                              \
    case DMA_REGION_IWRAM + DMA_REGION_IWRAM * DMA_REGION_COUNT:              \
      dma_tfloop(iwram, iwram, src_strd, dest_strd, tfsize);                  \
                                                                              \
    case DMA_REGION_EWRAM + DMA_REGION_IWRAM * DMA_REGION_COUNT:              \
      dma_tfloop(ewram, iwram, src_strd, dest_strd, tfsize);                  \
                                                                              \
    case DMA_REGION_VRAM + DMA_REGION_IWRAM * DMA_REGION_COUNT:               \
      dma_tfloop(vram, iwram, src_strd, dest_strd, tfsize);                   \
                                                                              \
    case DMA_REGION_PALETTE_RAM + DMA_REGION_IWRAM * DMA_REGION_COUNT:        \
      dma_tfloop(palette_ram, iwram, src_strd, dest_strd, tfsize);            \
                                                                              \
    case DMA_REGION_OAM_RAM + DMA_REGION_IWRAM * DMA_REGION_COUNT:            \
      dma_tfloop(oam_ram, iwram, src_strd, dest_strd, tfsize);                \
                                                                              \
    case DMA_REGION_IO + DMA_REGION_IWRAM * DMA_REGION_COUNT:                 \
      dma_tfloop(io, iwram, src_strd, dest_strd, tfsize);                     \
                                                                              \
    case DMA_REGION_GAMEPAK + DMA_REGION_IWRAM * DMA_REGION_COUNT:            \
      dma_tfloop(gamepak, iwram, src_strd, dest_strd, tfsize);                \
                                                                              \
    case DMA_REGION_EXT + DMA_REGION_IWRAM * DMA_REGION_COUNT:                \
      dma_tfloop(ext, iwram, src_strd, dest_strd, tfsize);                    \
                                                                              \
    case DMA_REGION_BUS + DMA_REGION_EWRAM * DMA_REGION_COUNT:                \
      dma_tfloop(bus, ewram, src_strd, dest_strd, tfsize);                    \
                                                                              \
    case DMA_REGION_IWRAM + DMA_REGION_EWRAM * DMA_REGION_COUNT:              \
      dma_tfloop(iwram, ewram, src_strd, dest_strd, tfsize);                  \
                                                                              \
    case DMA_REGION_EWRAM + DMA_REGION_EWRAM * DMA_REGION_COUNT:              \
      dma_tfloop(ewram, ewram, src_strd, dest_strd, tfsize);                  \
                                                                              \
    case DMA_REGION_VRAM + DMA_REGION_EWRAM * DMA_REGION_COUNT:               \
      dma_tfloop(vram, ewram, src_strd, dest_strd, tfsize);                   \
                                                                              \
    case DMA_REGION_PALETTE_RAM + DMA_REGION_EWRAM * DMA_REGION_COUNT:        \
      dma_tfloop(palette_ram, ewram, src_strd, dest_strd, tfsize);            \
                                                                              \
    case DMA_REGION_OAM_RAM + DMA_REGION_EWRAM * DMA_REGION_COUNT:            \
      dma_tfloop(oam_ram, ewram, src_strd, dest_strd, tfsize);                \
                                                                              \
    case DMA_REGION_IO + DMA_REGION_EWRAM * DMA_REGION_COUNT:                 \
      dma_tfloop(io, ewram, src_strd, dest_strd, tfsize);                     \
                                                                              \
    case DMA_REGION_GAMEPAK + DMA_REGION_EWRAM * DMA_REGION_COUNT:            \
      dma_tfloop(gamepak, ewram, src_strd, dest_strd, tfsize);                \
                                                                              \
    case DMA_REGION_EXT + DMA_REGION_EWRAM * DMA_REGION_COUNT:                \
      dma_tfloop(ext, ewram, src_strd, dest_strd, tfsize);                    \
                                                                              \
    case DMA_REGION_BUS + DMA_REGION_VRAM * DMA_REGION_COUNT:                 \
      dma_tfloop(bus, vram, src_strd, dest_strd, tfsize);                     \
                                                                              \
    case DMA_REGION_IWRAM + DMA_REGION_VRAM * DMA_REGION_COUNT:               \
      dma_tfloop(iwram, vram, src_strd, dest_strd, tfsize);                   \
                                                                              \
    case DMA_REGION_EWRAM + DMA_REGION_VRAM * DMA_REGION_COUNT:               \
      dma_tfloop(ewram, vram, src_strd, dest_strd, tfsize);                   \
                                                                              \
    case DMA_REGION_VRAM + DMA_REGION_VRAM * DMA_REGION_COUNT:                \
      dma_tfloop(vram, vram, src_strd, dest_strd, tfsize);                    \
                                                                              \
    case DMA_REGION_PALETTE_RAM + DMA_REGION_VRAM * DMA_REGION_COUNT:         \
      dma_tfloop(palette_ram, vram, src_strd, dest_strd, tfsize);             \
                                                                              \
    case DMA_REGION_OAM_RAM + DMA_REGION_VRAM * DMA_REGION_COUNT:             \
      dma_tfloop(oam_ram, vram, src_strd, dest_strd, tfsize);                 \
                                                                              \
    case DMA_REGION_IO + DMA_REGION_VRAM * DMA_REGION_COUNT:                  \
      dma_tfloop(io, vram, src_strd, dest_strd, tfsize);                      \
                                                                              \
    case DMA_REGION_GAMEPAK + DMA_REGION_VRAM * DMA_REGION_COUNT:             \
      dma_tfloop(gamepak, vram, src_strd, dest_strd,tfsize);                  \
                                                                              \
    case DMA_REGION_EXT + DMA_REGION_VRAM * DMA_REGION_COUNT:                 \
      dma_tfloop(ext, vram, src_strd, dest_strd, tfsize);                     \
                                                                              \
    case DMA_REGION_BUS + DMA_REGION_PALETTE_RAM * DMA_REGION_COUNT:          \
      dma_tfloop(bus, palette_ram, src_strd, dest_strd, tfsize);              \
                                                                              \
    case DMA_REGION_IWRAM + DMA_REGION_PALETTE_RAM * DMA_REGION_COUNT:        \
      dma_tfloop(iwram, palette_ram, src_strd, dest_strd, tfsize);            \
                                                                              \
    case DMA_REGION_EWRAM + DMA_REGION_PALETTE_RAM * DMA_REGION_COUNT:        \
      dma_tfloop(ewram, palette_ram, src_strd, dest_strd, tfsize);            \
                                                                              \
    case DMA_REGION_VRAM + DMA_REGION_PALETTE_RAM * DMA_REGION_COUNT:         \
      dma_tfloop(vram, palette_ram, src_strd, dest_strd, tfsize);             \
                                                                              \
    case DMA_REGION_PALETTE_RAM + DMA_REGION_PALETTE_RAM * DMA_REGION_COUNT:  \
      dma_tfloop(palette_ram, palette_ram, src_strd, dest_strd, tfsize);      \
                                                                              \
    case DMA_REGION_OAM_RAM + DMA_REGION_PALETTE_RAM * DMA_REGION_COUNT:      \
      dma_tfloop(oam_ram, palette_ram, src_strd, dest_strd, tfsize);          \
                                                                              \
    case DMA_REGION_IO + DMA_REGION_PALETTE_RAM * DMA_REGION_COUNT:           \
      dma_tfloop(io, palette_ram, src_strd, dest_strd, tfsize);               \
                                                                              \
    case DMA_REGION_GAMEPAK + DMA_REGION_PALETTE_RAM * DMA_REGION_COUNT:      \
      dma_tfloop(gamepak, palette_ram, src_strd, dest_strd, tfsize);          \
                                                                              \
    case DMA_REGION_EXT + DMA_REGION_PALETTE_RAM * DMA_REGION_COUNT:          \
      dma_tfloop(ext, palette_ram, src_strd, dest_strd, tfsize);              \
                                                                              \
    case DMA_REGION_BUS + DMA_REGION_OAM_RAM * DMA_REGION_COUNT:              \
      dma_tfloop(bus, oam_ram, src_strd, dest_strd, tfsize);                  \
                                                                              \
    case DMA_REGION_IWRAM + DMA_REGION_OAM_RAM * DMA_REGION_COUNT:            \
      dma_tfloop(iwram, oam_ram, src_strd, dest_strd, tfsize);                \
                                                                              \
    case DMA_REGION_EWRAM + DMA_REGION_OAM_RAM * DMA_REGION_COUNT:            \
      dma_tfloop(ewram, oam_ram, src_strd, dest_strd, tfsize);                \
                                                                              \
    case DMA_REGION_VRAM + DMA_REGION_OAM_RAM * DMA_REGION_COUNT:             \
      dma_tfloop(vram, oam_ram, src_strd, dest_strd, tfsize);                 \
                                                                              \
    case DMA_REGION_PALETTE_RAM + DMA_REGION_OAM_RAM * DMA_REGION_COUNT:      \
      dma_tfloop(palette_ram, oam_ram, src_strd, dest_strd, tfsize);          \
                                                                              \
    case DMA_REGION_OAM_RAM + DMA_REGION_OAM_RAM * DMA_REGION_COUNT:          \
      dma_tfloop(oam_ram, oam_ram, src_strd, dest_strd, tfsize);              \
                                                                              \
    case DMA_REGION_IO + DMA_REGION_OAM_RAM * DMA_REGION_COUNT:               \
      dma_tfloop(io, oam_ram, src_strd, dest_strd, tfsize);                   \
                                                                              \
    case DMA_REGION_GAMEPAK + DMA_REGION_OAM_RAM * DMA_REGION_COUNT:          \
      dma_tfloop(gamepak, oam_ram, src_strd, dest_strd, tfsize);              \
                                                                              \
    case DMA_REGION_EXT + DMA_REGION_OAM_RAM * DMA_REGION_COUNT:              \
      dma_tfloop(ext, oam_ram, src_strd, dest_strd, tfsize);                  \
                                                                              \
    case DMA_REGION_BUS + DMA_REGION_IO * DMA_REGION_COUNT:                   \
      dma_tfloop(bus, io, src_strd, dest_strd, tfsize);                       \
                                                                              \
    case DMA_REGION_IWRAM + DMA_REGION_IO * DMA_REGION_COUNT:                 \
      dma_tfloop(iwram, io, src_strd, dest_strd, tfsize);                     \
                                                                              \
    case DMA_REGION_EWRAM + DMA_REGION_IO * DMA_REGION_COUNT:                 \
      dma_tfloop(ewram, io, src_strd, dest_strd, tfsize);                     \
                                                                              \
    case DMA_REGION_VRAM + DMA_REGION_IO * DMA_REGION_COUNT:                  \
      dma_tfloop(vram, io, src_strd, dest_strd, tfsize);                      \
                                                                              \
    case DMA_REGION_PALETTE_RAM + DMA_REGION_IO * DMA_REGION_COUNT:           \
      dma_tfloop(palette_ram, io, src_strd, dest_strd, tfsize);               \
                                                                              \
    case DMA_REGION_OAM_RAM + DMA_REGION_IO * DMA_REGION_COUNT:               \
      dma_tfloop(oam_ram, io, src_strd, dest_strd, tfsize);                   \
                                                                              \
    case DMA_REGION_IO + DMA_REGION_IO * DMA_REGION_COUNT:                    \
      dma_tfloop(io, io, src_strd, dest_strd, tfsize);                        \
                                                                              \
    case DMA_REGION_GAMEPAK + DMA_REGION_IO * DMA_REGION_COUNT:               \
      dma_tfloop(gamepak, io, src_strd, dest_strd, tfsize);                   \
                                                                              \
    case DMA_REGION_EXT + DMA_REGION_IO * DMA_REGION_COUNT:                   \
      dma_tfloop(ext, io, src_strd, dest_strd, tfsize);                       \
                                                                              \
    case DMA_REGION_BUS + DMA_REGION_EXT * DMA_REGION_COUNT:                  \
      dma_tfloop(bus, ext, src_strd, dest_strd, tfsize);                      \
                                                                              \
    case DMA_REGION_IWRAM + DMA_REGION_EXT * DMA_REGION_COUNT:                \
      dma_tfloop(iwram, ext, src_strd, dest_strd, tfsize);                    \
                                                                              \
    case DMA_REGION_EWRAM + DMA_REGION_EXT * DMA_REGION_COUNT:                \
      dma_tfloop(ewram, ext, src_strd, dest_strd, tfsize);                    \
                                                                              \
    case DMA_REGION_VRAM + DMA_REGION_EXT * DMA_REGION_COUNT:                 \
      dma_tfloop(vram, ext, src_strd, dest_strd, tfsize);                     \
                                                                              \
    case DMA_REGION_PALETTE_RAM + DMA_REGION_EXT * DMA_REGION_COUNT:          \
      dma_tfloop(palette_ram, ext, src_strd, dest_strd, tfsize);              \
                                                                              \
    case DMA_REGION_OAM_RAM + DMA_REGION_EXT * DMA_REGION_COUNT:              \
      dma_tfloop(oam_ram, ext, src_strd, dest_strd, tfsize);                  \
                                                                              \
    case DMA_REGION_IO + DMA_REGION_EXT * DMA_REGION_COUNT:                   \
      dma_tfloop(io, ext, src_strd, dest_strd, tfsize);                       \
                                                                              \
    case DMA_REGION_GAMEPAK + DMA_REGION_EXT * DMA_REGION_COUNT:              \
      dma_tfloop(gamepak, ext, src_strd, dest_strd, tfsize);                  \
                                                                              \
    case DMA_REGION_EXT + DMA_REGION_EXT * DMA_REGION_COUNT:                  \
      dma_tfloop(ext, ext, src_strd, dest_strd, tfsize);                      \
  }                                                                           \
                                                                              \
  /* Remember the last value copied in case we read from a bad zone */        \
  dma_bus_val = read_value;                                                   \
                                                                              \
  dma->source_address = src_ptr;                                              \
  if (wb)   /* Update destination pointer if requested */                     \
    dma->dest_address = dest_ptr;                                             \
                                                                              \
  return alerts;                                                              \
}                                                                             \

dma_tf_loop_builder(16);
dma_tf_loop_builder(32);

static const int dma_stride[4] = {1, -1, 0, 1};

static cpu_alert_type dma_transfer_copy(
  dma_transfer_type *dmach, u32 src_ptr, u32 dest_ptr, u32 length)
{
  if (dmach->source_direction < 3)
  {
    int dst_stride = dma_stride[dmach->dest_direction];
    int src_stride = dma_stride[dmach->source_direction];
    bool dst_wb = dmach->dest_direction < 3;

    if(dmach->length_type == DMA_16BIT)
       return dma_tf_loop16(src_ptr, dest_ptr, 2 * src_stride, 2 * dst_stride, dst_wb, length, dmach);
    else
       return dma_tf_loop32(src_ptr, dest_ptr, 4 * src_stride, 4 * dst_stride, dst_wb, length, dmach);
  }

  return CPU_ALERT_NONE;
}

cpu_alert_type dma_transfer(unsigned dma_chan, int *usedcycles)
{
  dma_transfer_type *dmach = &dma[dma_chan];
  u32 src_ptr = 0x0FFFFFFF & dmach->source_address & (
                   dmach->length_type == DMA_16BIT ? ~1U : ~3U);
  u32 dst_ptr = 0x0FFFFFFF & dmach->dest_address & (
                   dmach->length_type == DMA_16BIT ? ~1U : ~3U);
  cpu_alert_type ret = CPU_ALERT_NONE;
  u32 tfsizes = dmach->length_type == DMA_16BIT ? 1 : 2;
  u32 byte_length = dmach->length << tfsizes;

  // Divide the DMA transaction into up to three transactions depending on
  // the source and destination memory regions.
  u32 src_end = MIN(0x10000000, src_ptr + byte_length * dma_stride[dmach->source_direction]);
  u32 dst_end = MIN(0x10000000, dst_ptr + byte_length * dma_stride[dmach->dest_direction]);

  dma_region_type src_reg0 = dma_region_map[src_ptr >> 24];
  dma_region_type src_reg1 = dma_region_map[src_end >> 24];
  dma_region_type dst_reg0 = dma_region_map[dst_ptr >> 24];
  dma_region_type dst_reg1 = dma_region_map[dst_end >> 24];

  if (src_reg0 == src_reg1 && dst_reg0 == dst_reg1)
    ret = dma_transfer_copy(dmach, src_ptr, dst_ptr, byte_length >> tfsizes);
  else if (src_reg0 == src_reg1) {
    // Source stays within the region, dest crosses over
    u32 blen0 = dma_stride[dmach->dest_direction] < 0 ?
        dst_ptr & 0xFFFFFF : 0x1000000 - (dst_ptr & 0xFFFFFF);
    u32 src1 = src_ptr + blen0 * dma_stride[dmach->source_direction];
    u32 dst1 = dst_ptr + blen0 * dma_stride[dmach->dest_direction];
    ret  = dma_transfer_copy(dmach, src_ptr, dst_ptr, blen0 >> tfsizes);
    ret |= dma_transfer_copy(dmach, src1, dst1, (byte_length - blen0) >> tfsizes);
  }
  else if (dst_reg0 == dst_reg1) {
    // Dest stays within the region, source crosses over
    u32 blen0 = dma_stride[dmach->source_direction] < 0 ?
        src_ptr & 0xFFFFFF : 0x1000000 - (src_ptr & 0xFFFFFF);
    u32 src1 = src_ptr + blen0 * dma_stride[dmach->source_direction];
    u32 dst1 = dst_ptr + blen0 * dma_stride[dmach->dest_direction];
    ret  = dma_transfer_copy(dmach, src_ptr, dst_ptr, blen0 >> tfsizes);
    ret |= dma_transfer_copy(dmach, src1, dst1, (byte_length - blen0) >> tfsizes);
  }
  // TODO: We do not cover the three-region case, seems no game uses that?
  // Lucky Luke does cross dest region due to some off-by-one error.

  if((dmach->repeat_type == DMA_NO_REPEAT) ||
   (dmach->start_type == DMA_START_IMMEDIATELY))
  {
    u32 cntrl = read_dmareg(REG_DMA0CNT_H, dma_chan);
    write_dmareg(REG_DMA0CNT_H, dma_chan, cntrl & (~0x8000));
    dmach->start_type = DMA_INACTIVE;
  }

  // Trigger an IRQ if configured to do so.
  if (dmach->irq)
    ret |= flag_interrupt(IRQ_DMA0 << dma_chan);

  // This is an approximation for the most common case (no region cross)
  if (usedcycles)
    *usedcycles += dmach->length * (
       def_seq_cycles[src_ptr >> 24][tfsizes - 1] +
       def_seq_cycles[dst_ptr >> 24][tfsizes - 1]);

  return ret;
}

// Be sure to do this after loading ROMs.

#define map_rom_entry(type, idx, ptr, mirror_blocks) {                        \
  unsigned mcount;                                                            \
  for(mcount = 0; mcount < 1024; mcount += (mirror_blocks)) {                 \
    memory_map_##type[(0x8000000 / (32 * 1024)) + (idx) + mcount] = (ptr);    \
    memory_map_##type[(0xA000000 / (32 * 1024)) + (idx) + mcount] = (ptr);    \
  }                                                                           \
  for(mcount = 0; mcount <  512; mcount += (mirror_blocks)) {                 \
    memory_map_##type[(0xC000000 / (32 * 1024)) + (idx) + mcount] = (ptr);    \
  }                                                                           \
}

#define map_region(type, start, end, mirror_blocks, region)                   \
  for(map_offset = (start) / 0x8000; map_offset <                             \
   ((end) / 0x8000); map_offset++)                                            \
  {                                                                           \
    memory_map_##type[map_offset] =                                           \
     ((u8 *)region) + ((map_offset % mirror_blocks) * 0x8000);                \
  }                                                                           \

#define map_null(type, start, end) {                                          \
  u32 map_offset;                                                             \
  for(map_offset = start / 0x8000; map_offset < (end / 0x8000);               \
   map_offset++)                                                              \
    memory_map_##type[map_offset] = NULL;                                     \
}

#define map_vram(type)                                                        \
  for(map_offset = 0x6000000 / 0x8000; map_offset < (0x7000000 / 0x8000);     \
   map_offset += 4)                                                           \
  {                                                                           \
    memory_map_##type[map_offset] = vram;                                     \
    memory_map_##type[map_offset + 1] = vram + 0x8000;                        \
    memory_map_##type[map_offset + 2] = vram + (0x8000 * 2);                  \
    memory_map_##type[map_offset + 3] = vram + (0x8000 * 2);                  \
  }                                                                           \


static u32 evict_gamepak_page(void)
{
  u32 ret;
  s16 phyrom;
  do {
    // Return the index to the last used entry
    u32 newhead = gamepak_blk_queue[gamepak_lru_head].next_lru;
    phyrom = gamepak_blk_queue[gamepak_lru_head].phy_rom;
    ret = gamepak_lru_head;

    // Second elem becomes head now
    gamepak_lru_head = newhead;

    // The evicted element goes at the end of the queue
    gamepak_blk_queue[gamepak_lru_tail].next_lru = ret;
    gamepak_lru_tail = ret;
    // If this page is marked as sticky, we keep going through the list
  } while (phyrom >= 0 && gamepak_sb_test(phyrom));

  // We unmap the ROM page if it was mapped, ensure we do not access it
  // without triggering a "page fault"
  if (phyrom >= 0) {
    map_rom_entry(read, phyrom, NULL, gamepak_size >> 15);
  }

  return ret;
}

u8 *load_gamepak_page(u32 physical_index)
{
  if(physical_index >= (gamepak_size >> 15))
    return &gamepak_buffers[0][0];

  u32 entry = evict_gamepak_page();
  u32 block_idx = entry / 32;
  u32 block_off = entry % 32;
  u8 *swap_location = &gamepak_buffers[block_idx][32 * 1024 * block_off];

  // Fill in the entry
  gamepak_blk_queue[entry].phy_rom = physical_index;

  fseek(gamepak_file_large, physical_index * (32 * 1024), SEEK_SET);
  fread(swap_location, (32 * 1024), 1, gamepak_file_large);

  // Map it to the read handlers now
  map_rom_entry(read, physical_index, swap_location, gamepak_size >> 15);

  // When mapping page 0, we might need to reflect the GPIO regs.
  if (physical_index == 0)
    update_gpio_romregs();

  return swap_location;
}

void init_gamepak_buffer(void)
{
  unsigned i;
  // Try to allocate up to 32 blocks of 1MB each
  gamepak_buffer_count = 0;
  while (gamepak_buffer_count < ROM_BUFFER_SIZE)
  {
    void *ptr = malloc(gamepak_buffer_blocksize);
    if (!ptr)
      break;
    gamepak_buffers[gamepak_buffer_count++] = (u8*)ptr;
  }

  // Initialize the memory map structure
  for (i = 0; i < 1024; i++)
  {
    gamepak_blk_queue[i].next_lru = (u16)(i + 1);
    gamepak_blk_queue[i].phy_rom = -1;
  }

  gamepak_lru_head = 0;
  gamepak_lru_tail = 32 * gamepak_buffer_count - 1;
}

bool gamepak_must_swap(void)
{
  // Returns whether the current gamepak buffer is not big enough to hold
  // the full gamepak ROM. In these cases the device must swap.
  return gamepak_buffer_count * gamepak_buffer_blocksize < gamepak_size;
}

void init_memory(void)
{
  u32 map_offset = 0, i;

  for (i = 0; i < DMA_CHAN_CNT; i++)
  {
    dma[i].start_type = DMA_INACTIVE;
    dma[i].irq = DMA_NO_IRQ;
    dma[i].source_address = dma[i].dest_address = 0;
    dma[i].source_direction = dma[i].dest_direction = 0;
    dma[i].length = 0;
    dma[i].length_type = DMA_16BIT;
    dma[i].repeat_type = DMA_NO_REPEAT;
    dma[i].direct_sound_channel = DMA_NO_DIRECT_SOUND;
  }

  // Fill memory map regions, areas marked as NULL must be checked directly
  map_region(read, 0x0000000, 0x1000000, 1, bios_rom);
  map_null(read, 0x1000000, 0x2000000);
  map_region(read, 0x2000000, 0x3000000, 8, ewram);
  map_region(read, 0x3000000, 0x4000000, 1, &iwram[0x8000 * SMC_DETECTION]);
  map_region(read, 0x4000000, 0x5000000, 1, io_registers);
  map_null(read, 0x5000000, 0x6000000);
  map_null(read, 0x6000000, 0x7000000);
  map_vram(read);
  map_null(read, 0x7000000, 0x8000000);
  map_null(read, 0xE000000, 0x10000000);

  memset(io_registers, 0, sizeof(io_registers));
  memset(oam_ram, 0, sizeof(oam_ram));
  memset(palette_ram, 0, sizeof(palette_ram));
  memset(iwram, 0, sizeof(iwram));
  memset(ewram, 0, sizeof(ewram));
  memset(vram, 0, sizeof(vram));

  write_ioreg(REG_DISPCNT, 0x80);
  write_ioreg(REG_P1, 0x3FF);
  write_ioreg(REG_BG2PA, 0x100);
  write_ioreg(REG_BG2PD, 0x100);
  write_ioreg(REG_BG3PA, 0x100);
  write_ioreg(REG_BG3PD, 0x100);
  write_ioreg(REG_RCNT, 0x8000);

  reload_timing_info();

  backup_type = backup_type_reset;
  flash_bank_num = 0;
  flash_command_position = 0;
  eeprom_size = EEPROM_512_BYTE;
  eeprom_mode = EEPROM_BASE_MODE;
  eeprom_address = 0;
  eeprom_counter = 0;
  rumble_enable_tick = 0;
  rumble_ticks = 0;
  dma_bus_val = 0;

  flash_mode = FLASH_BASE_MODE;

  rtc_state = RTC_DISABLED;
  memset(gpio_regs, 0, sizeof(gpio_regs));
  reg[REG_BUS_VALUE] = 0xe129f000;
}

void memory_term(void)
{
  if (gamepak_file_large)
  {
    fclose(gamepak_file_large);
    gamepak_file_large = NULL;
  }

  while (gamepak_buffer_count)
  {
    free(gamepak_buffers[--gamepak_buffer_count]);
  }
}

bool memory_check_savestate(const u8 *src)
{
  static const char *vars32[] = {
    "backup-type","flash-mode", "flash-cmd-pos", "flash-bank-num", "flash-dev-id",
    "flash-size", "eeprom-size", "eeprom-mode", "eeprom-addr", "eeprom-counter",
    "rtc-state", "rtc-write-mode", "rtc-cmd", "rtc-status", "rtc-data-bit-cnt", "rtc-bit-cnt",
  };
  static const char *dmavars32[] = {
    "src-addr", "dst-addr", "src-dir", "dst-dir",
    "len", "size", "repeat", "start", "dsc", "irq"
  };
  int i;
  const u8 *memdoc = bson_find_key(src, "memory");
  const u8 *bakdoc = bson_find_key(src, "backup");
  const u8 *dmadoc = bson_find_key(src, "dma");
  if (!memdoc || !bakdoc || !dmadoc)
    return false;

  // Check memory buffers (TODO: check sizes!)
  if (!bson_contains_key(memdoc, "iwram", BSON_TYPE_BIN) ||
      !bson_contains_key(memdoc, "ewram", BSON_TYPE_BIN) ||
      !bson_contains_key(memdoc, "vram", BSON_TYPE_BIN) ||
      !bson_contains_key(memdoc, "oamram", BSON_TYPE_BIN) ||
      !bson_contains_key(memdoc, "palram", BSON_TYPE_BIN) ||
      !bson_contains_key(memdoc, "ioregs", BSON_TYPE_BIN) ||
      !bson_contains_key(memdoc, "dma-bus", BSON_TYPE_INT32))
     return false;

  // Check backup variables
  for (i = 0; i < sizeof(vars32)/sizeof(vars32[0]); i++)
    if (!bson_contains_key(bakdoc, vars32[i], BSON_TYPE_INT32))
      return false;

  if (!bson_contains_key(bakdoc, "gpio-regs", BSON_TYPE_BIN) ||
      !bson_contains_key(bakdoc, "rtc-data-words", BSON_TYPE_ARR))
      return false;

  for (i = 0; i < DMA_CHAN_CNT; i++)
  {
    char tname[2] = {'0' + i, 0};
    const u8 *dmastr = bson_find_key(dmadoc, tname);
    if (!dmastr)
      return false;

    for (i = 0; i < sizeof(dmavars32)/sizeof(dmavars32[0]); i++)
      if (!bson_contains_key(dmastr, dmavars32[i], BSON_TYPE_INT32))
        return false;
  }
  return true;
}


bool memory_read_savestate(const u8 *src)
{
  int i;
  u32 rtc_data_array[2];
  const u8 *memdoc = bson_find_key(src, "memory");
  const u8 *bakdoc = bson_find_key(src, "backup");
  const u8 *dmadoc = bson_find_key(src, "dma");
  if (!memdoc || !bakdoc || !dmadoc)
    return false;

  if (!(
    bson_read_bytes(memdoc, "iwram", &iwram[0x8000 * SMC_DETECTION], 0x8000) &&
    bson_read_bytes(memdoc, "ewram", ewram, 0x40000) &&
    bson_read_bytes(memdoc, "vram", vram, sizeof(vram)) &&
    bson_read_bytes(memdoc, "oamram", oam_ram, sizeof(oam_ram)) &&
    bson_read_bytes(memdoc, "palram", palette_ram, sizeof(palette_ram)) &&
    bson_read_bytes(memdoc, "ioregs", io_registers, sizeof(io_registers)) &&
    bson_read_int32(memdoc, "dma-bus", &dma_bus_val) &&

    bson_read_int32(bakdoc, "backup-type", &backup_type) &&

    bson_read_int32(bakdoc, "flash-mode", &flash_mode) &&
    bson_read_int32(bakdoc, "flash-cmd-pos", &flash_command_position) &&
    bson_read_int32(bakdoc, "flash-bank-num", &flash_bank_num) &&
    bson_read_int32(bakdoc, "flash-dev-id", &flash_device_id) &&
    bson_read_int32(bakdoc, "flash-size", &flash_bank_cnt) &&

    bson_read_int32(bakdoc, "eeprom-size", &eeprom_size) &&
    bson_read_int32(bakdoc, "eeprom-mode", &eeprom_mode) &&
    bson_read_int32(bakdoc, "eeprom-addr", &eeprom_address) &&
    bson_read_int32(bakdoc, "eeprom-counter", &eeprom_counter) &&

    bson_read_bytes(bakdoc, "gpio-regs", gpio_regs, sizeof(gpio_regs)) &&

    bson_read_int32(bakdoc, "rtc-state", &rtc_state) &&
    bson_read_int32(bakdoc, "rtc-write-mode", &rtc_write_mode) &&
    bson_read_int32(bakdoc, "rtc-cmd", &rtc_command) &&
    bson_read_int32(bakdoc, "rtc-status", &rtc_status) &&
    bson_read_int32(bakdoc, "rtc-data-bit-cnt", &rtc_data_bits) &&
    bson_read_int32(bakdoc, "rtc-bit-cnt", (u32*)&rtc_bit_count) &&
    bson_read_int32_array(bakdoc, "rtc-data-words", rtc_data_array, 2)))
    return false;

  for (i = 0; i < DMA_CHAN_CNT; i++)
  {
    char tname[2] = {'0' + i, 0};
    const u8 *dmastr = bson_find_key(dmadoc, tname);
    if (!(
      bson_read_int32(dmastr, "src-addr", &dma[i].source_address) &&
      bson_read_int32(dmastr, "dst-addr", &dma[i].dest_address) &&
      bson_read_int32(dmastr, "src-dir", &dma[i].source_direction) &&
      bson_read_int32(dmastr, "dst-dir", &dma[i].dest_direction) &&
      bson_read_int32(dmastr, "len", &dma[i].length) &&
      bson_read_int32(dmastr, "size", &dma[i].length_type) &&
      bson_read_int32(dmastr, "repeat", &dma[i].repeat_type) &&
      bson_read_int32(dmastr, "start", &dma[i].start_type) &&
      bson_read_int32(dmastr, "dsc", &dma[i].direct_sound_channel) &&
      bson_read_int32(dmastr, "irq", &dma[i].irq)))
      return false;
  }

  rtc_data = rtc_data_array[0] | (((u64)rtc_data_array[1]) << 32);

  return true;
}

unsigned memory_write_savestate(u8 *dst)
{
  int i;
  u8 *wbptr, *wbptr2, *startp = dst;
  u32 rtc_data_array[2] = { (u32)rtc_data, (u32)(rtc_data >> 32) };

  bson_start_document(dst, "memory", wbptr);
  bson_write_bytes(dst, "iwram", &iwram[0x8000 * SMC_DETECTION], 0x8000);
  bson_write_bytes(dst, "ewram", ewram, 0x40000);
  bson_write_bytes(dst, "vram", vram, sizeof(vram));
  bson_write_bytes(dst, "oamram", oam_ram, sizeof(oam_ram));
  bson_write_bytes(dst, "palram", palette_ram, sizeof(palette_ram));
  bson_write_bytes(dst, "ioregs", io_registers, sizeof(io_registers));
  bson_write_int32(dst, "dma-bus", dma_bus_val);
  bson_finish_document(dst, wbptr);

  bson_start_document(dst, "backup", wbptr);
  bson_write_int32(dst, "backup-type", (u32)backup_type);

  bson_write_int32(dst, "flash-mode", flash_mode);
  bson_write_int32(dst, "flash-cmd-pos", flash_command_position);
  bson_write_int32(dst, "flash-bank-num", flash_bank_num);
  bson_write_int32(dst, "flash-dev-id", flash_device_id);
  bson_write_int32(dst, "flash-size", flash_bank_cnt);

  bson_write_int32(dst, "eeprom-size", eeprom_size);
  bson_write_int32(dst, "eeprom-mode", eeprom_mode);
  bson_write_int32(dst, "eeprom-addr", eeprom_address);
  bson_write_int32(dst, "eeprom-counter", eeprom_counter);

  bson_write_bytes(dst, "gpio-regs", gpio_regs, sizeof(gpio_regs));
  bson_write_int32(dst, "rtc-state", rtc_state);
  bson_write_int32(dst, "rtc-write-mode", rtc_write_mode);
  bson_write_int32(dst, "rtc-cmd", rtc_command);
  bson_write_int32(dst, "rtc-status", rtc_status);
  bson_write_int32(dst, "rtc-data-bit-cnt", rtc_data_bits);
  bson_write_int32(dst, "rtc-bit-cnt", rtc_bit_count);
  bson_write_int32array(dst, "rtc-data-words", rtc_data_array, 2);
  bson_finish_document(dst, wbptr);

  bson_start_document(dst, "dma", wbptr);
  for (i = 0; i < DMA_CHAN_CNT; i++)
  {
    char tname[2] = {'0' + i, 0};
    bson_start_document(dst, tname, wbptr2);
    bson_write_int32(dst, "src-addr", dma[i].source_address);
    bson_write_int32(dst, "dst-addr", dma[i].dest_address);
    bson_write_int32(dst, "src-dir", dma[i].source_direction);
    bson_write_int32(dst, "dst-dir", dma[i].dest_direction);
    bson_write_int32(dst, "len", dma[i].length);
    bson_write_int32(dst, "size", dma[i].length_type);
    bson_write_int32(dst, "repeat", dma[i].repeat_type);
    bson_write_int32(dst, "start", dma[i].start_type);
    bson_write_int32(dst, "dsc", dma[i].direct_sound_channel);
    bson_write_int32(dst, "irq", dma[i].irq);
    bson_finish_document(dst, wbptr2);
  }
  bson_finish_document(dst, wbptr);

  return (unsigned int)(dst - startp);
}

static s32 load_gamepak_raw(const char *name)
{
  unsigned i, j;
  gamepak_file_large = fopen(name, "rb");
  if(gamepak_file_large)
  {
    // Round size to 32KB pages
    fseek(gamepak_file_large, 0, SEEK_END);
    gamepak_size = (u32)ftell(gamepak_file_large);
    fseek(gamepak_file_large, 0, SEEK_SET);
    gamepak_size = (gamepak_size + 0x7FFF) & ~0x7FFF;

    // Load stuff in 1MB chunks
    u32 buf_blocks = (gamepak_size + gamepak_buffer_blocksize-1) / (gamepak_buffer_blocksize);
    u32 rom_blocks = gamepak_size >> 15;
    u32 ldblks = buf_blocks < gamepak_buffer_count ?
                    buf_blocks : gamepak_buffer_count;

    // Unmap the ROM space since we will re-map it now
    map_null(read, 0x8000000, 0xD000000);

    // Proceed to read the whole ROM or as much as possible.
    for (i = 0; i < ldblks; i++)
    {
      // Load 1MB chunk and map it
      fread(gamepak_buffers[i], gamepak_buffer_blocksize, 1, gamepak_file_large);
      for (j = 0; j < 32 && i*32 + j < rom_blocks; j++)
      {
        u32 phyn = i*32 + j;
        u8* blkptr = &gamepak_buffers[i][32 * 1024 * j];
        u32 entry = evict_gamepak_page();
        gamepak_blk_queue[entry].phy_rom = phyn;
        // Map it to the read handlers now
        map_rom_entry(read, phyn, blkptr, rom_blocks);
      }
    }

    return 0;
  }

  return -1;
}

u32 load_gamepak(const struct retro_game_info* info, const char *name,
                 int force_rtc, int force_rumble, int force_serial)
{
   gamepak_info_t gpinfo;

   if (load_gamepak_raw(name))
      return -1;

   // Buffer 0 always has the first 1MB chunk of the ROM
   memset(&gpinfo, 0, sizeof(gpinfo));
   memcpy(gpinfo.gamepak_title, &gamepak_buffers[0][0xA0], 12);
   memcpy(gpinfo.gamepak_code,  &gamepak_buffers[0][0xAC],  4);
   memcpy(gpinfo.gamepak_maker, &gamepak_buffers[0][0xB0],  2);

   idle_loop_target_pc = 0xFFFFFFFF;
   translation_gate_targets = 0;
   flash_device_id = FLASH_DEVICE_MACRONIX_64KB;
   flash_bank_cnt = FLASH_SIZE_64KB;
   rtc_enabled = false;
   rumble_enabled = false;
   backup_type_reset = BACKUP_UNKN;
   serial_mode = force_serial;

   load_game_config_over(&gpinfo);

   // Forced RTC / Rumble modes, override the autodetect logic.
   if (force_rtc != FEAT_AUTODETECT)
      rtc_enabled = (force_rtc == FEAT_ENABLE);
   if (force_rumble != FEAT_AUTODETECT)
      rumble_enabled = (force_rumble == FEAT_ENABLE);

   return 0;
}

s32 load_bios(char *name)
{
  FILE *fd = fopen(name, "rb");
  if (!fd)
    return -1;

  if (!bios_rom_alloc)
    bios_rom_alloc = malloc(0x4000);

  if (bios_rom_alloc && fread(bios_rom_alloc, 0x4000, 1, fd))
  {
    bios_rom = bios_rom_alloc;
    fclose(fd);
    return 0;
  }
fail:
  fclose(fd);
  return -1;
}


