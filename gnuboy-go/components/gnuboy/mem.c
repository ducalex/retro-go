#include "stdlib.h"

#include "defs.h"
#include "hw.h"
#include "regs.h"
#include "mem.h"
#include "rtc.h"
#include "lcd.h"
#include "cpu.h"
#include "sound.h"
#include "loader.h"


struct mbc mbc;
struct rom rom;
struct ram ram;


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
	mbc.rombank &= (mbc.romsize - 1);
	mbc.rambank &= (mbc.ramsize - 1);

	if (rom.bank[mbc.rombank] == NULL)
	{
		rom_loadbank(mbc.rombank);
	}

	memset(mbc.rmap, 0, sizeof(mbc.rmap));
	memset(mbc.wmap, 0, sizeof(mbc.wmap));

	// ROM
	mbc.rmap[0x0] = rom.bank[0];
	mbc.rmap[0x1] = rom.bank[0];
	mbc.rmap[0x2] = rom.bank[0];
	mbc.rmap[0x3] = rom.bank[0];

	if (mbc.rombank < mbc.romsize)
	{
		mbc.rmap[0x4] = rom.bank[mbc.rombank] - 0x4000;
		mbc.rmap[0x5] = rom.bank[mbc.rombank] - 0x4000;
		mbc.rmap[0x6] = rom.bank[mbc.rombank] - 0x4000;
		mbc.rmap[0x7] = rom.bank[mbc.rombank] - 0x4000;
	}

	// VRAM
	mbc.rmap[0x8] = mbc.wmap[0x8] = lcd.vbank[R_VBK & 1] - 0x8000;
	mbc.rmap[0x9] = mbc.wmap[0x9] = lcd.vbank[R_VBK & 1] - 0x8000;

	// SRAM
	if (mbc.enableram && !(rtc.sel & 8))
	{
	 	// mbc.rmap[0xA] = mbc.wmap[0xA] = ram.sbank[mbc.rambank] - 0xA000;
	 	// mbc.rmap[0xB] = mbc.wmap[0xB] = ram.sbank[mbc.rambank] - 0xA000;
	}

	// WRAM
	// NOTE: This cause stuttering in some games, needs more investigating...
	// mbc.rmap[0xC] = mbc.wmap[0xC] = ram.ibank[0] - 0xC000;
	// mbc.rmap[0xD] = mbc.wmap[0xD] = ram.ibank[(R_SVBK & 0x7) ?: 1] - 0xD000;
	// mbc.rmap[0xE] = mbc.wmap[0xE] = ram.ibank[0] - 0xE000; // Mirror

	// IO port and registers
	mbc.rmap[0xF] = mbc.wmap[0xF] = NULL;
}


/*
 * ioreg_write handles output to io registers in the FF00-FF7F,FFFF
 * range. It takes the register number (low byte of the address) and a
 * byte value to be written.
 */

static inline void ioreg_write(byte r, byte b)
{
	if (!hw.cgb)
	{
		switch (r)
		{
		case RI_BGP:
			pal_write_dmg(0, 0, b);
			pal_write_dmg(8, 1, b);
			break;
		case RI_OBP0:
			pal_write_dmg(64, 2, b);
			break;
		case RI_OBP1:
			pal_write_dmg(72, 3, b);
			break;

		// These don't exist on DMG:
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
	case RI_BGP:
	case RI_OBP0:
	case RI_OBP1:
		REG(r) = b;
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
		if ((b & 0x81) == 0x81)
			hw.serial = 1952; // 8 * 122us;
		else
			hw.serial = 0;
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
		R_STAT = (R_STAT & 0x07) | (b & 0x78);
		if (!hw.cgb && !(R_STAT & 2)) /* DMG STAT write bug => interrupt */
			hw_interrupt(IF_STAT, IF_STAT);
		stat_trigger();
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
		REG(r) = b | 0xF8;
		mem_updatemap();
		break;
	case RI_DMA:
		hw_dma(b);
		break;
	case RI_KEY1:
		REG(r) = (REG(r) & 0x80) | (b & 0x01);
		break;
	case RI_HDMA1:
	case RI_HDMA2:
	case RI_HDMA3:
	case RI_HDMA4:
		REG(r) = b;
		break;
	case RI_HDMA5:
		hw_hdma_cmd(b);
		break;
	default:
		if (r >= 0x10 && r < 0x40) {
			sound_write(r, b);
		}
	}

	/* printf("reg %02X => %02X (%02X)\n", r, REG(r), b); */
}


static inline byte ioreg_read(byte r)
{
	switch(r)
	{
	case RI_SB:
	case RI_SC:
	case RI_P1:
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
	//	return REG(r);
	// CGB-specific registers
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
		// if (hw.cgb) return REG(r);
		return REG(r);
	default:
		if (r >= 0x10 && r < 0x40) {
			return sound_read(r);
		}
		return 0xFF;
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

static inline void mbc_write(int a, byte b)
{
	byte ha = (a >> 12);

	// printf("mbc %d: rom bank %02X -[%04X:%02X]-> ", mbc.type, mbc.rombank, a, b);
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
		if (ha == 0x4 || ha == 0x5) {
			b &= ~8;
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
			mbc.rombank = (mbc.rombank & 0x100) | (b);
			break;
		case 0x3:
			mbc.rombank = (mbc.rombank & 0xFF) | ((int)(b & 1) << 8);
			break;
		case 0x4:
		case 0x5:
			mbc.rambank = b & 0x0F;
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

	// printf("%02X\n", mbc.rombank);
	mem_updatemap();
}


/*
 * mem_write is the basic write function. Although it should only be
 * called when the write map contains a NULL for the requested address
 * region, it accepts writes to any address.
 */

void IRAM_ATTR mem_write(word a, byte b)
{
	byte ha = (a >> 12) & 0xE;

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
		lcd.vbank[R_VBK&1][a & 0x1FFF] = b;
		break;

	case 0xA:
		if (!mbc.enableram) break;
		if (rtc.sel & 8) {
			rtc_write(b);
		} else {
			ram.sbank[mbc.rambank][a & 0x1FFF] = b;
			ram.sram_dirty = 1;
		}
		break;

	case 0xC:
		if ((a & 0xF000) == 0xC000)
			ram.ibank[0][a & 0x0FFF] = b;
		else
			ram.ibank[(R_SVBK & 0x7) ?: 1][a & 0x0FFF] = b;
		break;

	case 0xE:
		if (a < 0xFE00)
		{
			writeb(a & 0xDFFF, b);
		}
		else if ((a & 0xFF00) == 0xFE00)
		{
			if (a < 0xFEA0) lcd.oam.mem[a & 0xFF] = b;
		}
		else if (a >= 0xFF10 && a <= 0xFF3F)
		{
			sound_write(a & 0xFF, b);
		}
		else if ((a & 0xFF80) == 0xFF80 && a != 0xFFFF)
		{
			ram.hi[a & 0xFF] = b;
		}
		else
		{
			ioreg_write(a & 0xFF, b);
		}
	}
}


/*
 * mem_read is the basic read function...not useful for much anymore
 * with the read map, but it's still necessary for the final messy
 * region.
 */

byte IRAM_ATTR mem_read(word a)
{
	byte ha = (a >> 12) & 0xE;

	/* printf("read %04x\n", a); */
	switch (ha)
	{
	case 0x0:
	case 0x2:
		return rom.bank[0][a & 0x3fff];

	case 0x4:
	case 0x6:
		return rom.bank[mbc.rombank][a & 0x3FFF];

	case 0x8:
		return lcd.vbank[R_VBK&1][a & 0x1FFF];

	case 0xA:
		if (!mbc.enableram && mbc.type == MBC_HUC3)
			return 0x01;
		if (!mbc.enableram)
			return 0xFF;
		if (rtc.sel & 8)
			return rtc.regs[rtc.sel & 7];
		return ram.sbank[mbc.rambank][a & 0x1FFF];

	case 0xC:
		if ((a & 0xF000) == 0xC000)
			return ram.ibank[0][a & 0x0FFF];
		return ram.ibank[(R_SVBK & 0x7) ?: 1][a & 0x0FFF];

	case 0xE:
		if (a < 0xFE00)
		{
			return readb(a & 0xDFFF);
		}
		else if ((a & 0xFF00) == 0xFE00)
		{
			return (a < 0xFEA0) ? lcd.oam.mem[a & 0xFF] : 0xFF;
		}
		else if (a >= 0xFF10 && a <= 0xFF3F)
		{
			return sound_read(a & 0xFF);
		}
		else if ((a & 0xFF80) == 0xFF80)
		{
			return REG(a & 0xFF);
		}

		return ioreg_read(a & 0xFF);
	}
	return 0xFF;
}

void mbc_reset()
{
	mbc.rombank = 1;
	mbc.rambank = 0;
	mbc.enableram = 0;
	mem_updatemap();
}
