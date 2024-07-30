/* gameplaySP
 *
 * Copyright (C) 2024 David Guillen Fandos <david@davidgf.net>
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

static const u32 gbp_seq[16] = {
  0x0000494E,
  0x0000494E,
  0xB6B1494E,
  0xB6B1544E,
  0xABB1544E,
  0xABB14E45,
  0xB1BA4E45,
  0xB1BA4F44,
  0xB0BB4F44,
  0xB0BB8002,
  0x10000010,
  0x20000013,
  0x30000003,
  0x30000003,
  0x30000003,
  0x30000003, // Responds with rumble amount
};

static u32 gbp_seq_n = 0;
static bool gbp_rumble = false;

// GB Player sequencing
u32 gbp_transfer(u32 value) {
  u32 ret = gbp_seq[gbp_seq_n++];
  if (gbp_seq_n == 16) {
    bool rumble_active = (value & 2);
    write_rumble(gbp_rumble, rumble_active);
    gbp_rumble = rumble_active;
    gbp_seq_n = 0;
  }
  return ret;
}

