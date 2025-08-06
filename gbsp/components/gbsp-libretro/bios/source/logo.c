/*
  Custom GBA BIOS replacement
  Copyright (c) 2002-2006 VBA Development Team
  Copyright (c) 2006-2013 VBA-M Development Team
  Copyright (c) 2013 Normmatt
	
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <tonc.h>

#include "logo_data.h"

static inline void CPUWriteHalfWord(u32 adr, u16 val)
{
	*(u16*)adr = val;
}
static inline u16 CPUReadHalfWord(u32 adr)
{
	return *(u16*)adr;
}

void memcpy_(u32 source, u32 dest, int count)
{
  // copy
  while(count) {
    CPUWriteHalfWord(dest, (source>0x0EFFFFFF ? 0x1CAD : CPUReadHalfWord(source)));
	dest += 2;
	source += 2;
    count--;
  }
}

extern void swi_LZ77UnCompVram_(u32 source, u32 dest, int checkBios);
extern void swi_RegisterRamReset(u32 flags);

/*-----------------------------------------------------------------
DrawLogo
  Draws a custom logo and pauses for a second or two.
-----------------------------------------------------------------*/
void DrawLogo() {

	// Load palette
	memcpy_((u32)logo_dataPal, (u32)pal_bg_mem, logo_dataPalLen);
	// Load tiles into CBB 0
	swi_LZ77UnCompVram_((u32)logo_dataTiles, (u32)&tile_mem[0][0], 0);
	// Load map into SBB 30
	swi_LZ77UnCompVram_((u32)logo_dataMap, (u32)&se_mem[30][0], 0);

	// set up BG0 for a 4bpp 64x32t map, using
	//   using charblock 0 and screenblock 31
	REG_BG0CNT= BG_CBB(0) | BG_SBB(30) | BG_4BPP | BG_REG_32x32;
	REG_DISPCNT= DCNT_MODE0 | DCNT_BG0;

	int countdown = 60*2;
	while(countdown--)
	{
		vid_vsync();
	}
	
	swi_RegisterRamReset(0xFF);
}
