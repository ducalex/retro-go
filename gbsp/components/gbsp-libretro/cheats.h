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

#ifndef __GPSP_CHEATS_H__
#define __GPSP_CHEATS_H__

#define MAX_CHEATS       20
#define MAX_CHEAT_CODES  64

typedef enum {
   CheatNoError = 0,
   CheatErrorTooMany,
   CheatErrorTooBig,
   CheatErrorEncrypted,
   CheatErrorNotSupported
} cheat_error;

void process_cheats(void);
cheat_error cheat_parse(unsigned index, const char *code);
void cheat_clear(void);

extern u32 cheat_master_hook;

#endif

