#include <string.h>
#include "gnuboy.h"
#include "sound.h"
#include "cpu.h"
#include "hw.h"
#include "lcd.h"

gb_cart_t cart;
gb_hw_t hw;


static void rtc_latch(byte b)
{
	if ((cart.rtc.latch ^ b) & b & 1)
	{
		cart.rtc.regs[0] = cart.rtc.s;
		cart.rtc.regs[1] = cart.rtc.m;
		cart.rtc.regs[2] = cart.rtc.h;
		cart.rtc.regs[3] = cart.rtc.d;
		cart.rtc.regs[4] = cart.rtc.flags;
	}
	cart.rtc.latch = b & 1;
}


static void rtc_write(byte b)
{
	switch (cart.rtc.sel & 0xf)
	{
	case 0x8: // Seconds
		cart.rtc.regs[0] = b;
		cart.rtc.s = b % 60;
		break;
	case 0x9: // Minutes
		cart.rtc.regs[1] = b;
		cart.rtc.m = b % 60;
		break;
	case 0xA: // Hours
		cart.rtc.regs[2] = b;
		cart.rtc.h = b % 24;
		break;
	case 0xB: // Days (lower 8 bits)
		cart.rtc.regs[3] = b;
		cart.rtc.d = ((cart.rtc.d & 0x100) | b) % 365;
		break;
	case 0xC: // Flags (days upper 1 bit, carry, stop)
		cart.rtc.regs[4] = b;
		cart.rtc.flags = b;
		cart.rtc.d = ((cart.rtc.d & 0xff) | ((b&1)<<9)) % 365;
		break;
	}
	cart.rtc.dirty = 1;
}


static void rtc_tick()
{
	if ((cart.rtc.flags & 0x40))
		return; // rtc stop

	if (++cart.rtc.ticks >= 60)
	{
		if (++cart.rtc.s >= 60)
		{
			if (++cart.rtc.m >= 60)
			{
				if (++cart.rtc.h >= 24)
				{
					if (++cart.rtc.d >= 365)
					{
						cart.rtc.d = 0;
						cart.rtc.flags |= 0x80;
					}
					cart.rtc.h = 0;
				}
				cart.rtc.m = 0;
			}
			cart.rtc.s = 0;
		}
		cart.rtc.ticks = 0;
	}
}


/*
 * hw_interrupt changes the virtual interrupt line(s) defined by i
 * The interrupt fires (added to R_IF) when the line transitions from 0 to 1.
 * It does not refire if the line was already high.
 */
void hw_interrupt(byte i, int level)
{
	if (level == 0)
	{
		hw.ilines &= ~i;
	}
	else if ((hw.ilines & i) == 0)
	{
		hw.ilines |= i;
		R_IF |= i; // Fire!

		if ((R_IE & i) != 0)
		{
			// Wake up the CPU when an enabled interrupt occurs
			// IME doesn't matter at this point, only IE
			hw.cpu->halted = 0;
		}
	}
}


/*
 * hw_dma performs plain old memory-to-oam dma, the original dmg
 * dma. Although on the hardware it takes a good deal of time, the cpu
 * continues running during this mode of dma, so no special tricks to
 * stall the cpu are necessary.
 */
static void hw_dma(unsigned b)
{
	unsigned a = b << 8;
	for (int i = 0; i < 160; i++, a++)
		lcd.oam.mem[i] = readb(a);
}


static void hw_hdma(byte c)
{
	/* Begin or cancel HDMA */
	if ((hw.hdma|c) & 0x80)
	{
		hw.hdma = c;
		R_HDMA5 = c & 0x7f;
		return;
	}

	/* Perform GDMA */
	size_t src = (R_HDMA1 << 8) | (R_HDMA2 & 0xF0);
	size_t dst = 0x8000 | ((R_HDMA3 & 0x1F) << 8) | (R_HDMA4 & 0xF0);
	size_t cnt = c + 1;

	/* FIXME - this should use cpu time! */
	/*cpu_timers(102 * cnt);*/

	cnt <<= 4;

	while (cnt--)
		writeb(dst++, readb(src++));

	R_HDMA1 = src >> 8;
	R_HDMA2 = src & 0xF0;
	R_HDMA3 = (dst >> 8) & 0x1F;
	R_HDMA4 = dst & 0xF0;
	R_HDMA5 = 0xFF;
}

void hw_hdma_cont(void)
{
	size_t src = (R_HDMA1 << 8) | (R_HDMA2 & 0xF0);
	size_t dst = 0x8000 | ((R_HDMA3 & 0x1F) << 8) | (R_HDMA4 & 0xF0);
	size_t cnt = 16;

	// if (!(hw.hdma & 0x80))
	// 	return;

	while (cnt--)
		writeb(dst++, readb(src++));

	R_HDMA1 = src >> 8;
	R_HDMA2 = src & 0xF0;
	R_HDMA3 = (dst >> 8) & 0x1F;
	R_HDMA4 = dst & 0xF0;
	R_HDMA5--;
	hw.hdma--;
}

/*
 * pad_refresh updates the P1 register from the pad states, generating
 * the appropriate interrupts (by quickly raising and lowering the
 * interrupt line) if a transition has been made.
 */
static inline void pad_refresh()
{
	byte oldp1 = R_P1;
	R_P1 &= 0x30;
	R_P1 |= 0xc0;
	if (!(R_P1 & 0x10)) R_P1 |= (hw.pad & 0x0F);
	if (!(R_P1 & 0x20)) R_P1 |= (hw.pad >> 4);
	R_P1 ^= 0x0F;
	if (oldp1 & ~R_P1 & 0x0F)
	{
		hw_interrupt(IF_PAD, 1);
		hw_interrupt(IF_PAD, 0);
	}
}


/*
 * hw_setpad updates the state of one or more buttons on the pad and calls
 * pad_refresh() to fire an interrupt if the pad changed.
 */
void hw_setpad(int new_pad)
{
	hw.pad = new_pad & 0xFF;
	pad_refresh();
}


gb_hw_t *hw_init(void)
{
	return &hw;
}


void hw_reset(bool hard)
{
	hw.ilines = 0;
	hw.serial = 0;
	hw.hdma = 0;
	hw.pad = 0;

	memset(hw.ioregs, 0, sizeof(hw.ioregs));
	R_P1 = 0xFF;
	R_LCDC = 0x91;
	R_BGP = 0xFC;
	R_OBP0 = 0xFF;
	R_OBP1 = 0xFF;
	R_SVBK = 0xF9;
	R_HDMA5 = 0xFF;
	R_VBK = 0xFE;

	if (hard)
	{
		memset(hw.rambanks, 0xff, 4096 * 8);
		memset(cart.rambanks, 0xff, 8192 * cart.ramsize);
		memset(&cart.rtc, 0, sizeof(gb_rtc_t));
	}

	memset(hw.rmap, 0, sizeof(hw.rmap));
	memset(hw.wmap, 0, sizeof(hw.wmap));

	cart.sram_dirty = 0;
	cart.bankmode = 0;
	cart.rombank = 1;
	cart.rambank = 0;
	cart.enableram = 0;
	hw_updatemap();
}


/*
 * hw_vblank is called once per frame at vblank and should take care
 * of things like rtc/sound/serial advance, emulation throttling, etc.
 */
void hw_vblank(void)
{
	hw.frames++;
	rtc_tick();
	sound_emulate();
}


/*
 * In order to make reads and writes efficient, we keep tables
 * (indexed by the high nibble of the address) specifying which
 * regions can be read/written without a function call. For such
 * ranges, the pointer in the map table points to the base of the
 * region in host system memory. For ranges that require special
 * processing, the pointer is NULL.
 *
 * hw_updatemap is called whenever bank changes or other operations
 * make the old maps potentially invalid.
 */
void hw_updatemap(void)
{
	int rombank = cart.rombank & (cart.romsize - 1);

	if (cart.rombanks[rombank] == NULL)
	{
		gnuboy_load_bank(rombank);
	}

	// ROM
	hw.rmap[0x0] = cart.rombanks[0];
	hw.rmap[0x1] = cart.rombanks[0];
	hw.rmap[0x2] = cart.rombanks[0];
	hw.rmap[0x3] = cart.rombanks[0];

	// Force bios to go through hw_read (speed doesn't really matter here)
	if (hw.bios && (R_BIOS & 1) == 0)
	{
		hw.rmap[0x0] = NULL;
	}

	// Cartridge ROM
	hw.rmap[0x4] = cart.rombanks[rombank] - 0x4000;
	hw.rmap[0x5] = hw.rmap[0x4];
	hw.rmap[0x6] = hw.rmap[0x4];
	hw.rmap[0x7] = hw.rmap[0x4];

	// Video RAM
	hw.rmap[0x8] = hw.wmap[0x8] = lcd.vbank[R_VBK & 1] - 0x8000;
	hw.rmap[0x9] = hw.wmap[0x9] = lcd.vbank[R_VBK & 1] - 0x8000;

	// Cartridge RAM
	hw.rmap[0xA] = hw.wmap[0xA] = NULL;
	hw.rmap[0xB] = hw.wmap[0xB] = NULL;

	// Work RAM
	hw.rmap[0xC] = hw.wmap[0xC] = hw.rambanks[0] - 0xC000;

	// Work RAM (GBC)
	if (hw.hwtype == GB_HW_CGB)
		hw.rmap[0xD] = hw.wmap[0xD] = hw.rambanks[(R_SVBK & 0x7) ?: 1] - 0xD000;

	// Mirror of 0xC000
	hw.rmap[0xE] = hw.wmap[0xE] = hw.rambanks[0] - 0xE000;

	// IO port and registers
	hw.rmap[0xF] = hw.wmap[0xF] = NULL;
}


/*
 * Memory bank controllers typically intercept write attempts to
 * 0000-7FFF, using the address and byte written as instructions to
 * change cart or sram banks, control special hardware, etc.
 *
 * mbc_write takes an address (which should be in the proper range)
 * and a byte value written to the address.
 */
static inline void mbc_write(unsigned a, byte b)
{
	MESSAGE_DEBUG("mbc %d: cart bank %02X -[%04X:%02X]-> ", cart.mbc, cart.rombank, a, b);

	switch (cart.mbc)
	{
	case MBC_MBC1:
		switch (a & 0xE000)
		{
		case 0x0000:
			cart.enableram = ((b & 0x0F) == 0x0A);
			break;
		case 0x2000:
			if ((b & 0x1F) == 0) b = 0x01;
			cart.rombank = (cart.rombank & 0x60) | (b & 0x1F);
			break;
		case 0x4000:
			if (cart.bankmode)
				cart.rambank = b & 0x03;
			else
				cart.rombank = (cart.rombank & 0x1F) | ((int)(b&3)<<5);
			break;
		case 0x6000:
			cart.bankmode = b & 0x1;
			break;
		}
		break;

	case MBC_MBC2:
		// 0x0000 - 0x3FFF, bit 0x0100 clear = RAM, set = ROM
		if ((a & 0xC100) == 0x0000)
			cart.enableram = ((b & 0x0F) == 0x0A);
		else if ((a & 0xC100) == 0x0100)
			cart.rombank = b & 0x0F;
		break;

	case MBC_HUC3:
		// FIX ME: This isn't right but the previous implementation was wronger...
	case MBC_MBC3:
		switch (a & 0xE000)
		{
		case 0x0000:
			cart.enableram = ((b & 0x0F) == 0x0A);
			break;
		case 0x2000:
			if ((b & 0x7F) == 0) b = 0x01;
			cart.rombank = b & 0x7F;
			break;
		case 0x4000:
			cart.rtc.sel = b & 0x0f;
			cart.rambank = b & 0x03;
			break;
		case 0x6000:
			rtc_latch(b);
			break;
		}
		break;

	case MBC_MBC5:
		switch (a & 0x7000)
		{
		case 0x0000:
		case 0x1000:
			cart.enableram = ((b & 0x0F) == 0x0A);
			break;
		case 0x2000:
			cart.rombank = (cart.rombank & 0x100) | (b);
			break;
		case 0x3000:
			cart.rombank = (cart.rombank & 0xFF) | ((int)(b & 1) << 8);
			break;
		case 0x4000:
		case 0x5000:
			cart.rambank = b & (cart.has_rumble ? 0x7 : 0xF);
			cart.rambank &= (cart.ramsize - 1);
			break;
		case 0x6000:
		case 0x7000:
			// Nothing but Radikal Bikers tries to access it.
			break;
		default:
			MESSAGE_ERROR("MBC_MBC5: invalid write to 0x%x (0x%x)\n", a, b);
			break;
		}
		break;

	case MBC_HUC1: /* FIXME - this is all guesswork -- is it right??? */
		switch (a & 0xE000)
		{
		case 0x0000:
			cart.enableram = ((b & 0x0F) == 0x0A);
			break;
		case 0x2000:
			if ((b & 0x1F) == 0) b = 0x01;
			cart.rombank = (cart.rombank & 0x60) | (b & 0x1F);
			break;
		case 0x4000:
			if (cart.bankmode)
				cart.rambank = b & 0x03 & (cart.ramsize - 1);
			else
				cart.rombank = (cart.rombank & 0x1F) | ((int)(b&3)<<5);
			break;
		case 0x6000:
			cart.bankmode = b & 0x1;
			break;
		}
		break;
	}

	MESSAGE_DEBUG("%02X\n", cart.rombank);

	hw_updatemap();
}


/*
 * hw_write is the basic write function. Although it should only be
 * called when the write map contains a NULL for the requested address
 * region, it accepts writes to any address.
 */
void hw_write(unsigned a, byte b)
{
	MESSAGE_DEBUG("write to 0x%04X: 0x%02X\n", a, b);

	switch (a & 0xE000)
	{
	case 0x0000: // MBC control (Cart ROM address space)
	case 0x2000:
	case 0x4000:
	case 0x6000:
		mbc_write(a, b);
		break;

	case 0x8000: // Video RAM
		lcd.vbank[R_VBK&1][a & 0x1FFF] = b;
		break;

	case 0xA000: // Save RAM or RTC
		if (!cart.enableram)
			break;

		if (cart.rtc.sel & 8)
		{
			rtc_write(b);
		}
		else
		{
			if (cart.rambanks[cart.rambank][a & 0x1FFF] != b)
			{
				cart.rambanks[cart.rambank][a & 0x1FFF] = b;
				cart.sram_dirty |= (1 << cart.rambank);
			}
		}
		break;

	case 0xC000: // System RAM
		if ((a & 0xF000) == 0xC000)
			hw.rambanks[0][a & 0x0FFF] = b;
		else
			hw.rambanks[(R_SVBK & 0x7) ?: 1][a & 0x0FFF] = b;
		break;

	case 0xE000: // Memory mapped IO
		// Mirror RAM: 0xE000 - 0xFDFF
		if (a < 0xFE00)
		{
			hw.rambanks[(a >> 12) & 1][a & 0xFFF] = b;
		}
		// Video: 0xFE00 - 0xFE9F
		else if ((a & 0xFF00) == 0xFE00)
		{
			if (a < 0xFEA0) lcd.oam.mem[a & 0xFF] = b;
		}
		// Sound: 0xFF10 - 0xFF3F
		else if (a >= 0xFF10 && a <= 0xFF3F)
		{
			sound_write(a & 0xFF, b);
		}
		// High RAM: 0xFF80 - 0xFFFE
		else if (a >= 0xFF80 && a <= 0xFFFE)
		{
			REG(a & 0xFF) = b;
		}
		// Hardware registers: 0xFF00 - 0xFF7F and 0xFFFF
		else
		{
			int r = a & 0xFF;

			if (hw.hwtype != GB_HW_CGB)
			{
				if (r >= 0x51 && r <= 0x70)
					return;
				if (r >= 0x47 && r <= 0x49)
					if (REG(r) != b) lcd_pal_dirty();
				// The few other GBC-only registers are harmless
				// and we let them fall through.
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
				lcd_lcdc_change(b);
				break;
			case RI_STAT:
				R_STAT = (R_STAT & 0x07) | (b & 0x78);
				if (hw.hwtype != GB_HW_CGB && !(R_STAT & 2)) /* DMG STAT write bug => interrupt */
					hw_interrupt(IF_STAT, 1);
				lcd_stat_trigger();
				break;
			case RI_LYC:
				REG(r) = b;
				lcd_stat_trigger();
				break;
			case RI_VBK: // GBC only
				REG(r) = b | 0xFE;
				hw_updatemap();
				break;
			case RI_BCPS: // GBC only
				R_BCPS = b & 0xBF;
				R_BCPD = lcd.pal[b & 0x3F];
				break;
			case RI_OCPS: // GBC only
				R_OCPS = b & 0xBF;
				R_OCPD = lcd.pal[64 + (b & 0x3F)];
				break;
			case RI_BCPD: // GBC only
				lcd.pal[R_BCPS & 0x3F] = b;
				lcd_pal_dirty();
				if (R_BCPS & 0x80) R_BCPS = (R_BCPS+1) & 0xBF;
				break;
			case RI_OCPD: // GBC only
				lcd.pal[64 + (R_OCPS & 0x3F)] = b;
				lcd_pal_dirty();
				R_OCPD = b;
				if (R_OCPS & 0x80) R_OCPS = (R_OCPS+1) & 0xBF;
				break;
			case RI_SVBK: // GBC only
				REG(r) = b | 0xF8;
				hw_updatemap();
				break;
			case RI_DMA:
				hw_dma(b);
				break;
			case RI_KEY1: // GBC only
				REG(r) = (REG(r) & 0x80) | (b & 0x01);
				break;
			case RI_BIOS:
				REG(r) = b;
				hw_updatemap();
				break;
			case RI_HDMA1: // GBC only
			case RI_HDMA2: // GBC only
			case RI_HDMA3: // GBC only
			case RI_HDMA4: // GBC only
				REG(r) = b;
				break;
			case RI_HDMA5: // GBC only
				hw_hdma(b);
				break;
			}
		}
	}
}


/*
 * hw_read is the basic read function...not useful for much anymore
 * with the read map, but it's still necessary for the final messy
 * region.
 */
byte hw_read(unsigned a)
{
	MESSAGE_DEBUG("read %04x\n", a);

	switch (a & 0xE000)
	{
	case 0x0000: // Cart ROM bank 0
		// BIOS is overlayed part of bank 0
		if (a < 0x900 && (R_BIOS & 1) == 0)
		{
			if (a < 0x100)
				return hw.bios[a];
			if (a >= 0x200 && hw.hwtype == GB_HW_CGB)
				return hw.bios[a];
		}
		// fall through
	case 0x2000:
		return cart.rombanks[0][a & 0x3fff];

	case 0x4000: // Cart ROM
	case 0x6000:
		return cart.rombanks[cart.rombank][a & 0x3FFF];

	case 0x8000: // Video RAM
		return lcd.vbank[R_VBK&1][a & 0x1FFF];

	case 0xA000: // Cart RAM or RTC
		if (!cart.enableram)
			return 0xFF;
		if (cart.rtc.sel & 8)
			return cart.rtc.regs[cart.rtc.sel & 7];
		return cart.rambanks[cart.rambank][a & 0x1FFF];

	case 0xC000: // System RAM
		if ((a & 0xF000) == 0xC000)
			return hw.rambanks[0][a & 0xFFF];
		return hw.rambanks[(R_SVBK & 0x7) ?: 1][a & 0xFFF];

	case 0xE000: // Memory mapped IO
		// Mirror RAM: 0xE000 - 0xFDFF
		if (a < 0xFE00)
		{
			return hw.rambanks[(a >> 12) & 1][a & 0xFFF];
		}
		// Video: 0xFE00 - 0xFE9F
		else if ((a & 0xFF00) == 0xFE00)
		{
			return (a < 0xFEA0) ? lcd.oam.mem[a & 0xFF] : 0xFF;
		}
		// Sound: 0xFF10 - 0xFF3F
		else if (a >= 0xFF10 && a <= 0xFF3F)
		{
			// Make sure sound emulation is all caught up
			sound_emulate();
		}
		// High RAM: 0xFF80 - 0xFFFE
		// else if ((a & 0xFF80) == 0xFF80)
		// {
		// 	return REG(a & 0xFF);
		// }
		// Hardware registers: 0xFF00 - 0xFF7F and 0xFFFF
		// else
		// {

			// We should check that the reg is valid, otherwise return 0xFF.
			// However invalid address should already contain 0xFF (unless incorrectly overwritten)
			// So, for speed, we'll assume that this is correct and always return REG(a)
			return REG(a & 0xFF);
		// }
	}
	return 0xFF;
}
