/* gameplaySP
 *
 * Copyright (C) 2023 David Guillen Fandos <david@davidgf.net>
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

const u8 *state_mem_read_ptr;
u8 *state_mem_write_ptr;

bool bson_contains_key(const u8 *srcp, const char *key, u8 keytype)
{
  unsigned keyl = strlen(key) + 1;
  unsigned doclen = bson_read_u32(srcp);
  const u8* p = &srcp[4];
  while (*p != 0 && (p - srcp) < doclen) {
    u8 tp = *p;
    unsigned tlen = strlen((char*)&p[1]) + 1;
    if (keyl == tlen && !memcmp(key, &p[1], tlen))
      return tp == keytype;  // Found it, check type
    p += 1 + tlen;
    if (tp == BSON_TYPE_DOC || tp == BSON_TYPE_ARR)
      p += bson_read_u32(p);
    else if (tp == BSON_TYPE_BIN)
      p += bson_read_u32(p) + 1 + 4;
    else if (tp == BSON_TYPE_INT32)
      p += 4;
  }
  return false;
}

const u8* bson_find_key(const u8 *srcp, const char *key)
{
  unsigned keyl = strlen(key) + 1;
  unsigned doclen = bson_read_u32(srcp);
  const u8* p = &srcp[4];
  while (*p != 0 && (p - srcp) < doclen) {
    u8 tp = *p;
    unsigned tlen = strlen((char*)&p[1]) + 1;
    if (keyl == tlen && !memcmp(key, &p[1], tlen))
      return &p[tlen + 1];
    p += 1 + tlen;
    if (tp == BSON_TYPE_DOC || tp == BSON_TYPE_ARR)
      p += bson_read_u32(p);
    else if (tp == BSON_TYPE_BIN)
      p += bson_read_u32(p) + 1 + 4;
    else if (tp == BSON_TYPE_INT32)
      p += 4;
  }
  return NULL;
}

bool bson_read_int32(const u8 *srcp, const char *key, u32* value)
{
  const u8* p = srcp ? bson_find_key(srcp, key) : NULL;
  if (!p)
    return false;
  *value = bson_read_u32(p);
  return true;
}

bool bson_read_int32_array(const u8 *srcp, const char *key, u32* value, unsigned cnt)
{
  const u8* p = srcp ? bson_find_key(srcp, key) : NULL;
  if (p) {
    unsigned arrsz = bson_read_u32(p);
    p += 4;
    if (arrsz < 5)
      return false;
    arrsz = (arrsz - 5) >> 3;
    while (arrsz--) {
      p += 4;   // type and name
      *value++ = bson_read_u32(p);
      p += 4;   // value
    }
    return true;
  }
  *value = bson_read_u32(p);
  return false;
}

bool bson_read_bytes(const u8 *srcp, const char *key, void* buffer, unsigned cnt)
{
  const u8* p = srcp ? bson_find_key(srcp, key) : NULL;
  if (p) {
    unsigned bufsz = bson_read_u32(p);
    if (bufsz != cnt)
      return false;

    // Skip byte array type and size
    memcpy(buffer, &p[5], cnt);
    return true;
  }
  return false;
}

bool gba_load_state(const void* src)
{
  u32 i, tmp;
  u8* srcptr = (u8*)src;
  u32 docsize = bson_read_u32(srcptr);
  if (docsize != GBA_STATE_MEM_SIZE)
    return false;

  if (!bson_read_int32(srcptr, "info-magic", &tmp) || tmp != GBA_STATE_MAGIC)
    return false;
  if (!bson_read_int32(srcptr, "info-version", &tmp) || tmp != GBA_STATE_VERSION)
    return false;

  // Validate that the state file makes sense before unconditionally reading it.
  if (!cpu_check_savestate(srcptr) ||
      !input_check_savestate(srcptr) ||
      !main_check_savestate(srcptr) ||
      !memory_check_savestate(srcptr) ||
      !sound_check_savestate(srcptr))
     return false;

  if (!(cpu_read_savestate(srcptr) &&
      input_read_savestate(srcptr) &&
      main_read_savestate(srcptr) &&
      memory_read_savestate(srcptr) &&
      sound_read_savestate(srcptr)))
  {
     // TODO: this should not happen if the validation above is accurate.
     return false;
  }

  // Generate converted palette (since it is not saved)
  for(i = 0; i < 512; i++)
  {
     palette_ram_converted[i] = convert_palette(eswap16(palette_ram[i]));
  }

  video_reload_counters();

  // Reset most of the frame state and dynarec state
#ifdef HAVE_DYNAREC
  if (dynarec_enable)
    flush_dynarec_caches();
#endif

  instruction_count = 0;
  reg[OAM_UPDATED] = 1;

  return true;
}

void gba_save_state(void* dst)
{
  u8 *stptr = (u8*)dst;
  u8 *wrptr = (u8*)dst;

  // Initial bson size
  bson_write_u32(wrptr, 0);

  // Add some info fields
  bson_write_int32(wrptr, "info-magic", GBA_STATE_MAGIC);
  bson_write_int32(wrptr, "info-version", GBA_STATE_VERSION);

  wrptr += cpu_write_savestate(wrptr);
  wrptr += input_write_savestate(wrptr);
  wrptr += main_write_savestate(wrptr);
  wrptr += memory_write_savestate(wrptr);
  wrptr += sound_write_savestate(wrptr);

  // The padding space is pushed into a padding field for easy parsing
  {
    unsigned padsize = GBA_STATE_MEM_SIZE - (wrptr - stptr);
    padsize -= 1 + 9 + 4 + 1 + 1;
    *wrptr++ = 0x05;    // Byte array
    bson_write_cstring(wrptr, "zpadding");
    bson_write_u32(wrptr, padsize);
    *wrptr++ = 0;
    wrptr += padsize;
  }

  *wrptr++ = 0;

  // Update the doc size  
  bson_write_u32(stptr, wrptr - stptr);
}


