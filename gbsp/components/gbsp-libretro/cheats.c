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

typedef struct
{
  bool cheat_active;
  struct {
    u32 address;
    u32 value;
  } codes[MAX_CHEAT_CODES];
  unsigned cheat_count;
} cheat_type;

cheat_type cheats[MAX_CHEATS];
u32 max_cheat = 0;
u32 cheat_master_hook = 0xffffffff;

static bool has_encrypted_codebreaker(cheat_type *cheat)
{
  int i;
  for(i = 0; i < cheat->cheat_count; i++)
  {
     u32 code    = cheat->codes[i].address;
     u32 opcode  = code >> 28;
     if (opcode == 9)
        return true;
  }
  return false;
}

static void update_hook_codebreaker(cheat_type *cheat)
{
  int i;
  for(i = 0; i < cheat->cheat_count; i++)
  {
     u32 code    = cheat->codes[i].address;
     u32 address = code & 0xfffffff;
     u32 opcode  = code >> 28;

     if (opcode == 1)
     {
        u32 pcaddr = 0x08000000 | (address & 0x1ffffff);
        #ifdef HAVE_DYNAREC
        if (cheat_master_hook != pcaddr)
           flush_dynarec_caches();   /* Flush caches to install hook */
        #endif
        cheat_master_hook = pcaddr;
        return;   /* Only support for one hook */
     }
  }
}

static void process_cheat_codebreaker(cheat_type *cheat, u16 pad)
{
  int i;
  unsigned j;
  for(i = 0; i < cheat->cheat_count; i++)
  {
    u32 code    = cheat->codes[i].address;
    u16 value   = cheat->codes[i].value;
    u32 address = code & 0xfffffff;
    u32 opcode  = code >> 28;

    switch (opcode) {
    case 0:   /* Game CRC, ignored for now */
      break;
    case 1:   /* Master code function */
      break;
    case 2:   /* 16 bit OR */
      write_memory16(address, read_memory16(address) | value);
      break;
    case 3:   /* 8 bit write */
      write_memory8(address, value);
      break;
    case 4:   /* Slide code, writes a buffer with addr/value strides */
      if (i + 1 < cheat->cheat_count)
      {
        u16 count = cheat->codes[++i].address;
        u16 vincr = cheat->codes[  i].address >> 16;
        u16 aincr = cheat->codes[  i].value;
        for (j = 0; j < count; j++)
        {
          write_memory16(address, value);
          address += aincr;
          value += vincr;
        }
      }
      break;
    case 5:   /* Super code: copies bytes to a buffer addr */
      for (j = 0; j < value * 2 && i < cheat->cheat_count; j++)
      {
        u8 bvalue, off = j % 6;
        switch (off) {
        case 0:
          bvalue = cheat->codes[++i].address >> 24;
          break;
        case 1 ... 3:
          bvalue = cheat->codes[i].address >> (24 - off*8);
          break;
        case 4 ... 5:
          bvalue = cheat->codes[i].value >> (40 - off*8);
          break;
        };
        write_memory8(address, bvalue);
        address++;
      }
      break;
    case 6:   /* 16 bit AND */
      write_memory16(address, read_memory16(address) & value);
      break;
    case 7:   /* Compare mem value and execute next cheat */
      if (read_memory16(address) != value)
        i++;
      break;
    case 8:   /* 16 bit write */
      write_memory16(address, value);
      break;
    case 10:   /* Compare mem value and skip next cheat */
      if (read_memory16(address) == value)
        i++;
      break;
    case 11:   /* Compare mem value and skip next cheat */
      if (read_memory16(address) <= value)
        i++;
      break;
    case 12:   /* Compare mem value and skip next cheat */
      if (read_memory16(address) >= value)
        i++;
      break;
    case 13:   /* Check button state and execute next cheat */
      switch ((address >> 4) & 0xf) {
      case 0:
        if (((~pad) & 0x3ff) == value)
          i++;
        break;
      case 1:
        if ((pad & value) == value)
          i++;
        break;
      case 2:
        if ((pad & value) == 0)
          i++;
        break;
      };
      break;
    case 14:   /* Increase 16/32 bit memory value */
      if (address & 1)
      {
        u32 value32 = (u32)((s16)value);  /* Sign extend to 32 bit */
        address &= ~1U;
        write_memory32(address, read_memory32(address) + value32);
      }
      else
      {
        write_memory16(address, read_memory16(address) + value);
      }
      break;
    case 15:   /* Immediate and check and skip */
      if ((read_memory16(address) & value) == 0)
        i++;
      break;
    }
  }
}

void process_cheats(void)
{
   u32 i;

   for(i = 0; i <= max_cheat; i++)
   {
      if(!cheats[i].cheat_active)
         continue;

      process_cheat_codebreaker(&cheats[i], 0x3ff ^ read_ioreg(REG_P1));
   }
}

void cheat_clear()
{
   int i;
   for (i = 0; i < MAX_CHEATS; i++)
   {
      cheats[i].cheat_count = 0;
      cheats[i].cheat_active = false;
   }
   cheat_master_hook = 0xffffffff;
}

cheat_error cheat_parse(unsigned index, const char *code)
{
   int pos = 0;
   int codelen = strlen(code);
   cheat_type *ch = &cheats[index];
   char buf[1024];
   
   if (index >= MAX_CHEATS)
      return CheatErrorTooMany;
   if (codelen >= sizeof(buf))
      return CheatErrorTooBig;

   memcpy(buf, code, codelen+1);
   
   /* Init to a known good state */
   ch->cheat_count = 0;
   if (index > max_cheat)
      max_cheat = index;

   /* Replace all the non-hex chars to spaces */
   for (pos = 0; pos < codelen; pos++)
      if (!((buf[pos] >= '0' && buf[pos] <= '9') ||
            (buf[pos] >= 'a' && buf[pos] <= 'f') ||
            (buf[pos] >= 'A' && buf[pos] <= 'F')))
         buf[pos] = ' ';

   /* Try to parse as Code Breaker */
   pos = 0;
   while (pos < codelen)
   {
      u32 op1; u16 op2;
      if (2 != sscanf(&buf[pos], "%08x %04hx", &op1, &op2))
         break;
      ch->codes[ch->cheat_count].address = op1;
      ch->codes[ch->cheat_count++].value = op2;
      pos += 13;
      while (pos < codelen && buf[pos] == ' ')
         pos++;
      if (ch->cheat_count >= MAX_CHEAT_CODES)
         break;
   }
   
   if (pos >= codelen)
   {
      /* Check whether these cheats are readable */
      if (has_encrypted_codebreaker(ch))
         return CheatErrorEncrypted;
      /* All codes were parsed! Process hook here */
      ch->cheat_active = true;
      update_hook_codebreaker(ch);
      return CheatNoError;
   }

   /* TODO parse other types here */
   return CheatErrorNotSupported;
}


