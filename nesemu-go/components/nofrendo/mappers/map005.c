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
** map005.c
**
** Mapper 5 (MMC5) interface
** Implementation by ducalex
**
*/

#include <string.h>
#include <noftypes.h>
#include <nofrendo.h>
#include <nes_mmc.h>
#include <nes.h>

static uint8 prg_mode;
static uint8 chr_mode;
static uint8 exram_mode;
static uint8 prgram_protect1;
static uint8 prgram_protect2;
static uint8 nametable_mapping;
static uint8 multiplication[2];

static uint16 prg_banks[4];
static uint16 chr_banks[12];
static uint16 chr_upper_bits;

#define IN_FRAME    0x40
#define IRQ_PENDING 0x80
#define SPLIT_ENABLED 0x80
#define SPLIT_RIGHT 0x40
#define SPLIT_TILE 0x1F


static struct
{
   uint8 mode;
   uint8 scroll;
   uint8 bank;
} vert_split;

static struct
{
   int16 scanline;
   uint8 enabled;
   uint8 status;
} irq;

static struct
{
   uint8 color;
   uint8 tile;
} fill_mode;

static uint8 *ExRAM;


static uint8 *get_nametable(int n)
{
   return nes_getptr()->ppu->nametab + (0x400 * n);
}

static void prg_update()
{
   switch (prg_mode)
   {
   case 0: /* 1x32K */
      mmc_bankrom(32, 0x8000, prg_banks[0] >> 2);
      break;

   case 1: /* 2x16K */
      mmc_bankrom(16, 0x8000, prg_banks[1] >> 1); // Could also be RAM
      mmc_bankrom(16, 0xC000, prg_banks[3] >> 1);
      break;

   case 2: /* 1x16K+2x8K */
      mmc_bankrom(16, 0x8000, prg_banks[1] >> 1); // Could also be RAM
      mmc_bankrom(8, 0xC000, prg_banks[2]);       // Could also be RAM
      mmc_bankrom(8, 0xE000, prg_banks[3]);
      break;

   case 3: /* 4x8K */
      mmc_bankrom(8, 0x8000, prg_banks[0]); // Could also be RAM
      mmc_bankrom(8, 0xA000, prg_banks[1]); // Could also be RAM
      mmc_bankrom(8, 0xC000, prg_banks[2]); // Could also be RAM
      mmc_bankrom(8, 0xE000, prg_banks[3]);
      break;
   }
}

static void chr_update(int reg)
{
   switch (chr_mode)
   {
   case 0: /* 8K */
      break;

   case 1: /* 4K */
      break;

   case 2: /* 2K */
      break;

   case 3: /* 1K */
      mmc_bankvrom(1, 0 * 0x400, chr_banks[reg < 8 ? 0 : 8]);
      mmc_bankvrom(1, 1 * 0x400, chr_banks[reg < 8 ? 1 : 9]);
      mmc_bankvrom(1, 2 * 0x400, chr_banks[reg < 8 ? 2 : 10]);
      mmc_bankvrom(1, 3 * 0x400, chr_banks[reg < 8 ? 3 : 11]);
      mmc_bankvrom(1, 4 * 0x400, chr_banks[reg < 8 ? 4 : 8]);
      mmc_bankvrom(1, 5 * 0x400, chr_banks[reg < 8 ? 5 : 9]);
      mmc_bankvrom(1, 6 * 0x400, chr_banks[reg < 8 ? 6 : 10]);
      mmc_bankvrom(1, 7 * 0x400, chr_banks[reg < 8 ? 7 : 11]);
      break;
   }
}

static inline void prg_switch(int reg, int value)
{
   prg_banks[reg] = value & 0x7F;
   prg_update();
}

static inline void chr_switch(int reg, int value)
{
   chr_banks[reg] = value | chr_upper_bits;
   // chr_update(reg);

   if (reg >= 8) {
      mmc_bankvrom(1, 0x1000 + (reg - 8) * 0x400, chr_banks[reg]);
   } else {
      mmc_bankvrom(1, reg * 0x400, chr_banks[reg]);
   }
}

static inline void nametable_update(uint8 value)
{
   nametable_mapping = value;
   ppu_mirror(value & 3, (value >> 2) & 3, (value >> 4) & 3, value >> 6);
}

static inline void nametable_fill()
{
   memset(get_nametable(3), fill_mode.tile, 32 * 30); // 32 tiles per row, 30 rows
   memset(get_nametable(3) + 32 * 30, (fill_mode.color | fill_mode.color << 2 | fill_mode.color << 4 | fill_mode.color << 6), 64);
}

static void map5_hblank(int scanline)
{
   if (scanline >= 241)
   {
      irq.status &= ~IN_FRAME; // (IN_FRAME|IRQ_PENDING)
   }
   else
   {
      irq.status |= IN_FRAME;

      if (scanline == irq.scanline)
      {
         if (irq.enabled) nes_irq();
         irq.status |= IRQ_PENDING;
      }
   }
}

static void map5_write(uint32 address, uint8 value)
{
   // MESSAGE_INFO("MMC5 write: $%02X to $%04X\n", value, address);

   switch (address)
   {
   case 0x5100:
      /* PRG mode */
      prg_mode = value & 3;
      prg_update();
      break;

   case 0x5101:
      /* CHR mode */
      chr_mode = value & 3;
      chr_update(-1);
      break;

   case 0x5102:
      /* PRG RAM Protect 1 */
      /* Allow write = 0b10 */
      prgram_protect1 = value & 3;
      prg_update();
      break;

   case 0x5103:
      /* PRG RAM Protect 2 */
      /* Allow write = 0b01 */
      prgram_protect2 = value & 3;
      prg_update();
      break;

   case 0x5104:
      /* Extended RAM mode */
      exram_mode = value & 3;
      break;

   case 0x5105:
      /* Nametable mapping */
      nametable_update(value);
      break;

   case 0x5106:
      /* Fill-mode tile */
      fill_mode.tile = value;
      nametable_fill();
      break;

   case 0x5107:
      /* Fill-mode color */
      fill_mode.color = value;
      nametable_fill();
      break;

   case 0x5113:
      /* PRG Bankswitching */
      break;

   case 0x5114:
   case 0x5115:
   case 0x5116:
   case 0x5117:
      /* PRG Bankswitching */
      prg_switch(address - 0x5114, value);
      break;

   case 0x5120:
   case 0x5121:
   case 0x5122:
   case 0x5123:
   case 0x5124:
   case 0x5125:
   case 0x5126:
   case 0x5127:
   case 0x5128:
   case 0x5129:
   case 0x512A:
   case 0x512B:
      /* CHR Bankswitching */
      chr_switch(address - 0x5120, value);
      break;

   case 0x5130:
      /* Upper CHR Bank bits */
      chr_upper_bits = (uint16)(value & 3) << 8;
      break;

   case 0x5200:
      /* Vertical Split Mode */
      vert_split.mode = value;
      break;

   case 0x5201:
      /* Vertical Split Scroll */
      vert_split.scroll = value;
      break;

   case 0x5202:
      /* Vertical Split Bank */
      vert_split.bank = value;
      break;

   case 0x5203:
      /* IRQ Scanline Compare Value */
      irq.scanline = value;
      break;

   case 0x5204:
      /* Scanline IRQ Status */
      irq.enabled = value >> 7;
      break;

   case 0x5205:
   case 0x5206:
      /* Unsigned u8*u8 to u16 multiplication */
      multiplication[address & 1] = value;
      break;

   default:
      MESSAGE_DEBUG("Invalid MMC5 write: $%02X to $%04X\n", value, address);
      break;
   }
}

static uint8 map5_read(uint32 address)
{
   switch(address)
   {
   case 0x5204:
      /* Scanline IRQ Status */
      {
         uint8 x = irq.status;
         irq.status &= ~IRQ_PENDING;
         return x;
      }

   case 0x5205:
      /* Unsigned 8x8 to 16 Multiplier (low byte) */
      return (multiplication[0] * multiplication[1]) & 0xFF;

   case 0x5206:
      /* Unsigned 8x8 to 16 Multiplier (high byte) */
      return (multiplication[0] * multiplication[1]) >> 8;

   default:
      MESSAGE_DEBUG("Invalid MMC5 read: $%04X", address);
      return 0xFF;
   }
}

static uint8 map5_exram_read(uint32 address)
{
   if (!ExRAM) ExRAM = get_nametable(2);
   return ExRAM[address & 0x3FF];
}

static void map5_exram_write(uint32 address, uint8 value)
{
   if (exram_mode != 3) {
      if (!ExRAM) ExRAM = get_nametable(2);
      ExRAM[address & 0x3FF] = value;
   }
}



uint8 MapperReadVRAM(uint32 address)
{
   #if 0
   bool isNtFetch = address >= 0x2000 && address <= 0x2FFF && (address & 0x3FF) < 0x3C0;
   if(isNtFetch)
   {
      //Nametable data, not an attribute fetch
      _splitInSplitRegion = false;
      _splitTileNumber++;

      if(_ppuInFrame) {
         UpdateChrBanks(false);
      } else if(_needInFrame) {
         _needInFrame = false;
         _ppuInFrame = true;
         UpdateChrBanks(false);
      }
   }

   DetectScanlineStart(address);

   _ppuIdleCounter = 3;
   _lastPpuReadAddr = address;

   if (exram_mode <= 1 && _ppuInFrame)
   {
      if((vert_split.mode & SPLIT_ENABLED))
      {
         uint16_t verticalSplitScroll = (_verticalSplitScroll + _scanlineCounter) % 240;
         if(addr >= 0x2000) {
         if(isNtFetch) {
         uint8_t tileNumber = (_splitTileNumber + 2) % 42;
         if(tileNumber <= 32 && ((_verticalSplitRightSide && tileNumber >= _verticalSplitDelimiterTile) || (!_verticalSplitRightSide && tileNumber < _verticalSplitDelimiterTile))) {
         //Split region (for next 3 fetches, attribute + 2x tile data)
         _splitInSplitRegion = true;
         _splitTile = ((verticalSplitScroll & 0xF8) << 2) | tileNumber;
         return InternalReadRam(0x5C00 + _splitTile);
         } else {
         //Outside of split region (or sprite data), result can get modified by ex ram mode code below
         _splitInSplitRegion = false;
         }
         } else if(_splitInSplitRegion) {
         return InternalReadRam(0x5FC0 | ((_splitTile & 0x380) >> 4) | ((_splitTile & 0x1F) >> 2));
         }
         } else if(_splitInSplitRegion) {
         //CHR tile fetches for split region
         return _chrRom[(_verticalSplitBank % (GetCHRPageCount() / 4)) * 0x1000 + (((addr & ~0x07) | (verticalSplitScroll & 0x07)) & 0xFFF)];
         }
      }

      if(exram_mode == 1 && (_splitTileNumber < 32 || _splitTileNumber >= 40))
      {
         //"In Mode 1, nametable fetches are processed normally, and can come from CIRAM nametables, fill mode, or even Expansion RAM, but attribute fetches are replaced by data from Expansion RAM."
         //"Each byte of Expansion RAM is used to enhance the tile at the corresponding address in every nametable"

         //When fetching NT data, we set a flag and then alter the VRAM values read by the PPU on the following 3 cycles (palette, tile low/high byte)
         if(isNtFetch) {
            //Nametable fetches
            _exAttributeLastNametableFetch = addr & 0x03FF;
            _exAttrLastFetchCounter = 3;
         } else if(_exAttrLastFetchCounter > 0) {
            //Attribute fetches
            _exAttrLastFetchCounter--;
            switch(_exAttrLastFetchCounter) {
            case 2:
            {
            //PPU palette fetch
            //Check work ram (expansion ram) to see which tile/palette to use
            //Use InternalReadRam to bypass the fact that the ram is supposed to be write-only in mode 0/1
            uint8_t value = InternalReadRam(0x5C00 + _exAttributeLastNametableFetch);

            //"The pattern fetches ignore the standard CHR banking bits, and instead use the top two bits of $5130 and the bottom 6 bits from Expansion RAM to choose a 4KB bank to select the tile from."
            _exAttrSelectedChrBank = ((value & 0x3F) | (_chrUpperBits << 6)) % (_chrRomSize / 0x1000);

            //Return a byte containing the same palette 4 times - this allows the PPU to select the right palette no matter the shift value
            uint8_t palette = (value & 0xC0) >> 6;
            return palette | palette << 2 | palette << 4 | palette << 6;
            }

            case 1:
            case 0:
            //PPU tile data fetch (high byte & low byte)
            return _chrRom[_exAttrSelectedChrBank * 0x1000 + (addr & 0xFFF)];
            }
         }
      }
   }

   return InternalReadVRAM(address);
   #endif
   return 0xFF;
}


static void map5_init(void)
{
   int last_bank = mmc_getinfo()->rom_banks * 2 - 1;

   irq.scanline = irq.enabled = 0;
   irq.status = 0;

   multiplication[0] = 0xFF;
   multiplication[1] = 0xFF;

   fill_mode.color = 0xFF;
   fill_mode.tile = 0xFF;

   prg_mode = 3;
   chr_mode = 3;

   chr_upper_bits = 0;

   for (int i = 0; i < 4; i++)
      prg_banks[i] = last_bank;

   for (int i = 0; i < 8; i++)
      chr_banks[i] = i;

   prg_update();
   chr_update(-1);
   nametable_fill();
   nametable_update(0);
}

static void map5_getstate(SnssMapperBlock *state)
{
   //
}

static void map5_setstate(SnssMapperBlock *state)
{
   //
}

static map_memwrite map5_memwrite[] =
{
   // { 0x5000, 0x50FF, map5_write }, // sound
   { 0x5100, 0x5BFF, map5_write },
   { 0x5C00, 0x5FFF, map5_exram_write },
   { 0x8000, 0xFFFF, map5_write },
   {     -1,     -1, NULL }
};

static map_memread map5_memread[] =
{
   { 0x5100, 0x5BFF, map5_read },
   { 0x5C00, 0x5FFF, map5_exram_read },
   {     -1,     -1, NULL }
};

mapintf_t map5_intf =
{
   5,                /* mapper number */
   "MMC5",           /* mapper name */
   map5_init,        /* init routine */
   NULL,             /* vblank callback */
   map5_hblank,      /* hblank callback */
   map5_getstate,    /* get state (snss) */
   map5_setstate,    /* set state (snss) */
   map5_memread,     /* memory read structure */
   map5_memwrite,    /* memory write structure */
   NULL              /* external sound device */
};
