#pragma GCC optimize ("O3")

#include <stdlib.h>

#include "gnuboy.h"
#include "defs.h"
#include "hw.h"
#include "regs.h"
#include "mem.h"
#include "rtc.h"
#include "lcd.h"
#include "sound.h"

#include "esp_partition.h"
#include "esp_attr.h"

#include "../odroid/odroid_display.h"
#include "../odroid/odroid_audio.h"


struct mbc mbc;
struct rom rom;
struct ram ram;

extern FILE* RomFile;
extern uint8_t BankCache[512 / 8];

static inline byte* GetRomPtr(short bank)
{
	// GBC pages are 16k.
	const size_t BANK_SIZE = 0x4000;
	byte* const PSRAM = (byte*)0x3f800000;
	const size_t OFFSET = bank * BANK_SIZE;

	if (RomFile)
	{
		short slot = bank >> 3;
		uint8_t bit = 1 << (bank & 0x7);

		if (!(BankCache[slot] & bit))
		{
			//printf("GetRomPtr: Loading bank=%d, slot=%d, bit=%d.\n", bank, slot, bit);

			// Stop the SPI bus
			odroid_display_lock();

			//odroid_display_drain_spi();

			// Load the 16K page
			if (fseek(RomFile, OFFSET, SEEK_SET))
			{
				printf("GetRomPtr: fseek failed. OFFSET=%d\n", OFFSET);

				odroid_audio_terminate();

				odroid_display_show_sderr(ODROID_SD_ERR_BADFILE);
				abort();
			}

	#if 0
			const size_t BLOCK_SIZE = 512;
			for (size_t offset = 0; offset < BANK_SIZE; offset += BLOCK_SIZE)
			{
				size_t count = fread((uint8_t*)PSRAM + (bank * BANK_SIZE) + offset, 1, BLOCK_SIZE, RomFile);
				__asm__("nop");
				__asm__("nop");
				__asm__("nop");
				__asm__("nop");
				__asm__("memw");

				if (count < BLOCK_SIZE) break;
			}
	#else
			size_t count = fread((uint8_t*)PSRAM + OFFSET, 1, BANK_SIZE, RomFile);
			if (count < BANK_SIZE)
			{
				printf("GetRomPtr: fread failed. bank=%d, count=%d\n", bank, count);

				odroid_audio_terminate();

				odroid_display_show_sderr(ODROID_SD_ERR_BADFILE);
				abort();
			}
	#endif

			BankCache[slot] |= bit;

			//printf("%s: bank=%d, result=%p\n", __func__, bank, (void*)PSRAM + OFFSET);

			odroid_display_unlock();
		}
	}

	byte* result = PSRAM + OFFSET;
	//printf("%s: bank=%d, result=%p\n", __func__, bank, result);

	return result;
}

/*
 * In order to make reads and writes efficient, we keep tables
 * (indexed by the high nibble of the address) specifying which
 * regions can be read/written without a function call. For such
 * ranges, the pointer in the map table points to the base of the
 * region in host system memory. For ranges that require special
 * processing, the pointer is NULL.
 *
 * mem_updatemap is called whenever bank changes or other operations
 * make the old maps potentially invalid.
 */

void IRAM_ATTR mem_updatemap()
{
	int n;
	byte **map;

	mbc.rombank &= (mbc.romsize - 1);

	rom.bank[mbc.rombank] = GetRomPtr(mbc.rombank);

	mbc.rambank &= (mbc.ramsize - 1);

	map = mbc.rmap;
	map[0x0] = rom.bank[0];
	map[0x1] = rom.bank[0];
	map[0x2] = rom.bank[0];
	map[0x3] = rom.bank[0];

	if (mbc.rombank < mbc.romsize)
	{
		map[0x4] = rom.bank[mbc.rombank] - 0x4000;
		map[0x5] = rom.bank[mbc.rombank] - 0x4000;
		map[0x6] = rom.bank[mbc.rombank] - 0x4000;
		map[0x7] = rom.bank[mbc.rombank] - 0x4000;
	}
	else
	{
		map[0x4] = map[0x5] = map[0x6] = map[0x7] = NULL;
	}

	//if (0 && (R_STAT & 0x03) == 0x03)
	//{
		map[0x8] = NULL;
		map[0x9] = NULL;
	//}
	//else
	//{
		// map[0x8] = lcd.vbank[R_VBK & 1] - 0x8000;
		// map[0x9] = lcd.vbank[R_VBK & 1] - 0x8000;
	//}

	// if (mbc.enableram && !(rtc.sel&8))
	// {
	//  	map[0xA] = ram.sbank[mbc.rambank] - 0xA000;
	//  	map[0xB] = ram.sbank[mbc.rambank] - 0xA000;
	// }
	//  else
	// {
		map[0xA] = map[0xB] = NULL;
	//}

#if 1
	map[0xC] = ram.ibank[0] - 0xC000;
	n = R_SVBK & 0x07;
	map[0xD] = ram.ibank[n?n:1] - 0xD000;
	map[0xE] = ram.ibank[0] - 0xE000;
	map[0xF] = NULL;
#else
	map[0xC] = NULL;
	map[0xD] = NULL;
	map[0xE] = NULL;
	map[0xF] = NULL;
#endif

#if 0
	map = mbc.wmap;
	map[0x0] = map[0x1] = map[0x2] = map[0x3] = NULL;
	map[0x4] = map[0x5] = map[0x6] = map[0x7] = NULL;
	map[0x8] = map[0x9] = NULL;

	// if (mbc.enableram && !(rtc.sel&8))
	// {
	// 	map[0xA] = ram.sbank[mbc.rambank] - 0xA000;
	// 	map[0xB] = ram.sbank[mbc.rambank] - 0xA000;
	// }
	// else
	// {
	// 	map[0xA] = map[0xB] = NULL;
	// }

	map[0xC] = ram.ibank[0] - 0xC000;
	n = R_SVBK & 0x07;
	map[0xD] = ram.ibank[n?n:1] - 0xD000;
	map[0xE] = ram.ibank[0] - 0xE000;
	map[0xF] = NULL;
#endif
}


/*
 * ioreg_write handles output to io registers in the FF00-FF7F,FFFF
 * range. It takes the register number (low byte of the address) and a
 * byte value to be written.
 */

void IRAM_ATTR ioreg_write(byte r, byte b)
{
	if (!hw.cgb)
	{
		switch (r)
		{
		case RI_VBK:
		case RI_BCPS:
		case RI_OCPS:
		case RI_BCPD:
		case RI_OCPD:
		case RI_SVBK:
		case RI_KEY1:
		case RI_HDMA1:
		case RI_HDMA2:
		case RI_HDMA3:
		case RI_HDMA4:
		case RI_HDMA5:
			return;
		}
	}

	switch(r)
	{
	case RI_TIMA:
	case RI_TMA:
	case RI_TAC:
	case RI_SCY:
	case RI_SCX:
	case RI_WY:
	case RI_WX:
		REG(r) = b;
		break;
	case RI_BGP:
		if (R_BGP == b) break;
		pal_write_dmg(0, 0, b);
		pal_write_dmg(8, 1, b);
		R_BGP = b;
		break;
	case RI_OBP0:
		if (R_OBP0 == b) break;
		pal_write_dmg(64, 2, b);
		R_OBP0 = b;
		break;
	case RI_OBP1:
		if (R_OBP1 == b) break;
		pal_write_dmg(72, 3, b);
		R_OBP1 = b;
		break;
	case RI_IF:
	case RI_IE:
		REG(r) = b & 0x1F;
		break;
	case RI_P1:
		REG(r) = b;
		pad_refresh();
		break;
	case RI_SC:
		/* FIXME - this is a hack for stupid roms that probe serial */
		if ((b & 0x81) == 0x81)
		{
			R_SB = 0xff;
			hw_interrupt(IF_SERIAL, IF_SERIAL);
			hw_interrupt(0, IF_SERIAL);
		}
		R_SC = b; /* & 0x7f; */
		break;
	case RI_SB:
		REG(r) = b;
		break;
	case RI_DIV:
		REG(r) = 0;
		break;
	case RI_LCDC:
		lcdc_change(b);
		break;
	case RI_STAT:
		stat_write(b);
		break;
	case RI_LYC:
		REG(r) = b;
		stat_trigger();
		break;
	case RI_VBK:
		REG(r) = b | 0xFE;
		mem_updatemap();
		break;
	case RI_BCPS:
		R_BCPS = b & 0xBF;
		R_BCPD = lcd.pal[b & 0x3F];
		break;
	case RI_OCPS:
		R_OCPS = b & 0xBF;
		R_OCPD = lcd.pal[64 + (b & 0x3F)];
		break;
	case RI_BCPD:
		R_BCPD = b;
		pal_write(R_BCPS & 0x3F, b);
		if (R_BCPS & 0x80) R_BCPS = (R_BCPS+1) & 0xBF;
		break;
	case RI_OCPD:
		R_OCPD = b;
		pal_write(64 + (R_OCPS & 0x3F), b);
		if (R_OCPS & 0x80) R_OCPS = (R_OCPS+1) & 0xBF;
		break;
	case RI_SVBK:
		REG(r) = b & 0x07;
		mem_updatemap();
		break;
	case RI_DMA:
		hw_dma(b);
		break;
	case RI_KEY1:
		REG(r) = (REG(r) & 0x80) | (b & 0x01);
		break;
	case RI_HDMA1:
		REG(r) = b;
		break;
	case RI_HDMA2:
		REG(r) = b; //& 0xF0;
		break;
	case RI_HDMA3:
		REG(r) = b; //& 0x1F;
		break;
	case RI_HDMA4:
		REG(r) = b; //& 0xF0;
		break;
	case RI_HDMA5:
		hw_hdma_cmd(b);
		break;
	}
	switch (r)
	{
	case RI_BGP:
	case RI_OBP0:
	case RI_OBP1:
		/* printf("palette reg %02X write %02X at LY=%02X\n", r, b, R_LY); */
	case RI_HDMA1:
	case RI_HDMA2:
	case RI_HDMA3:
	case RI_HDMA4:
	case RI_HDMA5:
		/* printf("HDMA %d: %02X\n", r - RI_HDMA1 + 1, b); */
		break;
	}
	/* printf("reg %02X => %02X (%02X)\n", r, REG(r), b); */
}


byte IRAM_ATTR ioreg_read(byte r)
{
	switch(r)
	{
	case RI_SC:
		r = R_SC;
		R_SC &= 0x7f;
		return r;
	case RI_P1:
	case RI_SB:
	case RI_DIV:
	case RI_TIMA:
	case RI_TMA:
	case RI_TAC:
	case RI_LCDC:
	case RI_STAT:
	case RI_SCY:
	case RI_SCX:
	case RI_LY:
	case RI_LYC:
	case RI_BGP:
	case RI_OBP0:
	case RI_OBP1:
	case RI_WY:
	case RI_WX:
	case RI_IE:
	case RI_IF:
		return REG(r);
	case RI_VBK:
	case RI_BCPS:
	case RI_OCPS:
	case RI_BCPD:
	case RI_OCPD:
	case RI_SVBK:
	case RI_KEY1:
	case RI_HDMA1:
	case RI_HDMA2:
	case RI_HDMA3:
	case RI_HDMA4:
	case RI_HDMA5:
		if (hw.cgb) return REG(r);
	default:
		return 0xff;
	}
}



/*
 * Memory bank controllers typically intercept write attempts to
 * 0000-7FFF, using the address and byte written as instructions to
 * change rom or sram banks, control special hardware, etc.
 *
 * mbc_write takes an address (which should be in the proper range)
 * and a byte value written to the address.
 */

void IRAM_ATTR mbc_write(int a, byte b)
{
	byte ha = (a>>12);

	/* printf("mbc %d: rom bank %02X -[%04X:%02X]-> ", mbc.type, mbc.rombank, a, b); */
	switch (mbc.type)
	{
	case MBC_MBC1:
		switch (ha & 0xE)
		{
		case 0x0:
			mbc.enableram = ((b & 0x0F) == 0x0A);
			break;
		case 0x2:
			if ((b & 0x1F) == 0) b = 0x01;
			mbc.rombank = (mbc.rombank & 0x60) | (b & 0x1F);
			break;
		case 0x4:
			if (mbc.model)
			{
				mbc.rambank = b & 0x03;
				break;
			}
			mbc.rombank = (mbc.rombank & 0x1F) | ((int)(b&3)<<5);
			break;
		case 0x6:
			mbc.model = b & 0x1;
			break;
		}
		break;

	case MBC_MBC2: /* is this at all right? */
		if ((a & 0x0100) == 0x0000)
		{
			mbc.enableram = ((b & 0x0F) == 0x0A);
			break;
		}
		if ((a & 0xE100) == 0x2100)
		{
			mbc.rombank = b & 0x0F;
			break;
		}
		break;

	case MBC_MBC3:
		switch (ha & 0xE)
		{
		case 0x0:
			mbc.enableram = ((b & 0x0F) == 0x0A);
			break;
		case 0x2:
			if ((b & 0x7F) == 0) b = 0x01;
			mbc.rombank = b & 0x7F;
			break;
		case 0x4:
			rtc.sel = b & 0x0f;
			mbc.rambank = b & 0x03;
			break;
		case 0x6:
			rtc_latch(b);
			break;
		}
		break;

	case MBC_RUMBLE:
		switch (ha & 0xF)
		{
		case 0x4:
		case 0x5:
			/* FIXME - save high bit as rumble state */
			/* mask off high bit */
			b &= 0x7;
			break;
		}
		/* fall thru */
	case MBC_MBC5:
		switch (ha & 0xF)
		{
		case 0x0:
		case 0x1:
			mbc.enableram = ((b & 0x0F) == 0x0A);
			break;
		case 0x2:
			//if ((b & 0xFF) == 0) b = 0x01;
			mbc.rombank = (mbc.rombank & 0x100) | (b);
			break;
		case 0x3:
			mbc.rombank = (mbc.rombank & 0x0FF) | ((int)(b&1)<<8);
			break;
		case 0x4:
		case 0x5:
			mbc.rambank = b & 0x0f;
			//printf("MBC5: Mapped rambank=%d\n", mbc.rambank);
			break;
		default:
			printf("MBC_MBC5: invalid write to 0x%x (0x%x)\n", a, b);
			break;
		}
		break;

	case MBC_HUC1: /* FIXME - this is all guesswork -- is it right??? */
		switch (ha & 0xE)
		{
		case 0x0:
			mbc.enableram = ((b & 0x0F) == 0x0A);
			break;
		case 0x2:
			if ((b & 0x1F) == 0) b = 0x01;
			mbc.rombank = (mbc.rombank & 0x60) | (b & 0x1F);
			break;
		case 0x4:
			if (mbc.model)
			{
				mbc.rambank = b & 0x03;
				break;
			}
			mbc.rombank = (mbc.rombank & 0x1F) | ((int)(b&3)<<5);
			break;
		case 0x6:
			mbc.model = b & 0x1;
			break;
		}
		break;

	case MBC_HUC3:
		switch (ha & 0xE)
		{
		case 0x0:
			mbc.enableram = ((b & 0x0F) == 0x0A);
			break;
		case 0x2:
			b &= 0x7F;
			mbc.rombank = b ? b : 1;
			break;
		case 0x4:
			rtc.sel = b & 0x0f;
			mbc.rambank = b & 0x03;
			break;
		case 0x6:
			rtc_latch(b);
			break;
		}
		break;
	}

	/* printf("%02X\n", mbc.rombank); */
	mem_updatemap();
}


/*
 * mem_write is the basic write function. Although it should only be
 * called when the write map contains a NULL for the requested address
 * region, it accepts writes to any address.
 */

void IRAM_ATTR mem_write(int a, byte b)
{
	int n;
	byte ha = (a>>12) & 0xE;

	/* printf("write to 0x%04X: 0x%02X\n", a, b); */
	switch (ha)
	{
	case 0x0:
	case 0x2:
	case 0x4:
	case 0x6:
		mbc_write(a, b);
		break;
	case 0x8:
		/* if ((R_STAT & 0x03) == 0x03) break; */
		vram_write(a & 0x1FFF, b);
		break;
	case 0xA:
		if (!mbc.enableram) break;
		if (rtc.sel&8)
		{
			rtc_write(b);
			break;
		}

		__asm__("nop");
		__asm__("nop");
		__asm__("nop");
		__asm__("nop");
		__asm__("memw");
		ram.sbank[mbc.rambank][a & 0x1FFF] = b;
		__asm__("nop");
		__asm__("nop");
		__asm__("nop");
		__asm__("nop");
		__asm__("memw");

		ram.sram_dirty = 1;
		//printf("mem_write: bank=%d, sram %p=0x%d\n", mbc.rambank, (void*)(a & 0x1fff), b);
		//printf("mem_write: check - write=0x%x, read=0x%x\n", b, ram.sbank[mbc.rambank][a & 0x1FFF]);
		break;
	case 0xC:
		if ((a & 0xF000) == 0xC000)
		{
			ram.ibank[0][a & 0x0FFF] = b;
			break;
		}
		n = R_SVBK & 0x07;
		ram.ibank[n?n:1][a & 0x0FFF] = b;
		break;
	case 0xE:
		if (a < 0xFE00)
		{
			mem_write(a & 0xDFFF, b);
			break;
		}
		if ((a & 0xFF00) == 0xFE00)
		{
			/* if (R_STAT & 0x02) break; */
			if (a < 0xFEA0) lcd.oam.mem[a & 0xFF] = b;
			break;
		}
		/* return writehi(a & 0xFF, b); */
		if (a >= 0xFF10 && a <= 0xFF3F)
		{
			sound_write(a & 0xFF, b);
			break;
		}
		if ((a & 0xFF80) == 0xFF80 && a != 0xFFFF)
		{
			ram.hi[a & 0xFF] = b;
			break;
		}
		ioreg_write(a & 0xFF, b);
	}
}


/*
 * mem_read is the basic read function...not useful for much anymore
 * with the read map, but it's still necessary for the final messy
 * region.
 */

byte IRAM_ATTR mem_read(int a)
{
	int n;
	byte ha = (a>>12) & 0xE;
	int index;
	byte* bnk;
	int tmp;

	//printf("read ha=0x%04x, a=0x%04x\n", ha, a);

	switch (ha)
	{
	case 0x0:
	case 0x2:
		//if (a >= 16384) return 0xff;
		return rom.bank[0][a & 0x3fff];
	case 0x4:
	case 0x6:
		return rom.bank[mbc.rombank][a & 0x3FFF];
	case 0x8:
		/* if ((R_STAT & 0x03) == 0x03) return 0xFF; */
		return lcd.vbank[R_VBK&1][a & 0x1FFF];
	case 0xA:
		if (!mbc.enableram && mbc.type == MBC_HUC3)
			return 0x01;
		if (!mbc.enableram)
			return 0xFF;
		if (rtc.sel&8)
			return rtc.regs[rtc.sel&7];

		__asm__("nop");
		__asm__("nop");
		__asm__("nop");
		__asm__("nop");
		__asm__("memw");
		//printf("mem_read: bank=%d, sram %p=0x%d\n", mbc.rambank, (void*)(a & 0x1fff), ram.sbank[mbc.rambank][a & 0x1FFF]);
		return ram.sbank[mbc.rambank][a & 0x1FFF];
	case 0xC:
		if ((a & 0xF000) == 0xC000)
			return ram.ibank[0][a & 0x0FFF];
		n = R_SVBK & 0x07;
		return ram.ibank[n?n:1][a & 0x0FFF];
	case 0xE:
		if (a < 0xFE00) return mem_read(a & 0xDFFF);
		if ((a & 0xFF00) == 0xFE00)
		{
			/* if (R_STAT & 0x02) return 0xFF; */
			if (a < 0xFEA0) return lcd.oam.mem[a & 0xFF];
			return 0xFF;
		}
		/* return readhi(a & 0xFF); */
		if (a == 0xFFFF) return REG(0xFF);
		if (a >= 0xFF10 && a <= 0xFF3F)
			return sound_read(a & 0xFF);
		if ((a & 0xFF80) == 0xFF80)
			return ram.hi[a & 0xFF];
		return ioreg_read(a & 0xFF);
	}
	return 0xff; /* not reached */
}

void mbc_reset()
{
	mbc.rombank = 1;
	mbc.rambank = 0;
	mbc.enableram = 0;
	mem_updatemap();
}
