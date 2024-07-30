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

#define SERIAL_MODE_DISABLED      0
#define SERIAL_MODE_RFU           1  // Wifi Adapter
#define SERIAL_MODE_GBP           2  // Connected to the GB Player
#define SERIAL_MODE_AUTO          3  // Choose best fit automatically

extern int serial_mode;

// Register writes
cpu_alert_type write_siocnt(u16 value);
cpu_alert_type write_rcnt(u16 value);

// Serial IRQ interface
u32 serial_next_event();
bool update_serial(unsigned cycles);

// RFU interface
void rfu_reset(void);
bool rfu_update(unsigned cycles);
u32 rfu_transfer(u32 value);
void rfu_frame_update(void);
void rfu_net_receive(const void* buf, size_t len, uint16_t client_id);

// GBP interface
u32 gbp_transfer(u32 value);
