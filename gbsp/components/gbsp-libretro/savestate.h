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

#ifndef SAVESTATE_H
#define SAVESTATE_H

#define BSON_TYPE_STR       0x02
#define BSON_TYPE_DOC       0x03
#define BSON_TYPE_ARR       0x04
#define BSON_TYPE_BIN       0x05
#define BSON_TYPE_INT32     0x10

#define bson_write_u32(p, value)                \
{                                               \
  u32 __tval = (value);                         \
  *p++ = (u8)((__tval));                        \
  *p++ = (u8)((__tval) >>  8);                  \
  *p++ = (u8)((__tval) >> 16);                  \
  *p++ = (u8)((__tval) >> 24);                  \
}

#define bson_read_u32(p)                        \
  ((p[3] << 24) | (p[2] << 16) |                \
   (p[1] <<  8) | (p[0] <<  0))

#define bson_write_cstring(p, value)            \
  memcpy(p, value, strlen(value)+1);            \
  p += strlen(value)+1;

#define bson_write_int32(p, key, value)         \
  *p++ = 0x10;                                  \
  bson_write_cstring(p, key);                   \
  bson_write_u32(p, value);

#define bson_write_int32array(p, key, arr, cnt) \
{                                               \
  u32 _n;                                       \
  u32 *arrptr = (u32*)(arr);                    \
  *p++ = 0x4;                                   \
  bson_write_cstring(p, key);                   \
  bson_write_u32(p, 5 + (cnt) * 8);             \
  for (_n = 0; _n < (cnt); _n++) {              \
    char ak[3] = {                              \
      (char)('0' + (_n/10)),                    \
      (char)('0' + (_n%10)),                    \
       0 };                                     \
    bson_write_int32(p, ak, arrptr[_n]);        \
  }                                             \
  *p++ = 0;                                     \
}

#define bson_write_bytes(p, key, value, vlen)   \
  *p++ = 0x05;                                  \
  bson_write_cstring(p, key);                   \
  bson_write_u32(p, vlen);                      \
  *p++ = 0;                                     \
  memcpy(p, value, vlen);                       \
  p += vlen;

#define bson_start_document(p, key, hdrptr)     \
  *p++ = 0x03;                                  \
  bson_write_cstring(p, key);                   \
  hdrptr = p;                                   \
  bson_write_u32(p, 0);

#define bson_finish_document(p, hdrptr)         \
{                                               \
  u32 _sz = p - hdrptr + 1;                     \
  *p++ = 0;                                     \
  bson_write_u32(hdrptr, _sz);                  \
}

bool bson_contains_key(const u8 *srcp, const char *key, u8 keytype);
const u8* bson_find_key(const u8 *srcp, const char *key);
bool bson_read_int32(const u8 *srcp, const char *key, u32* value);
bool bson_read_int32_array(const u8 *srcp, const char *key, u32* value, unsigned cnt);
bool bson_read_bytes(const u8 *srcp, const char *key, void* buffer, unsigned cnt);

/* this is an upper limit, leave room for future (?) stuff */
#define GBA_STATE_MEM_SIZE                    (416*1024)
#define GBA_STATE_MAGIC                       0x06BAC0DE
#define GBA_STATE_VERSION                     0x00010004

bool gba_load_state(const void *src);
void gba_save_state(void *dst);

#endif

