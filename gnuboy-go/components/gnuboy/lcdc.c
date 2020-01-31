#pragma GCC optimize ("O3")

#include <stdlib.h>

#include "gnuboy.h"
#include "defs.h"
#include "hw.h"
#include "cpu.h"
#include "regs.h"
#include "lcd.h"

#include <esp_attr.h>


#define C (cpu.lcdc)


/*
 * stat_trigger updates the STAT interrupt line to reflect whether any
 * of the conditions set to be tested (by bits 3-6 of R_STAT) are met.
 * This function should be called whenever any of the following occur:
 * 1) LY or LYC changes.
 * 2) A state transition affects the low 2 bits of R_STAT (see below).
 * 3) The program writes to the upper bits of R_STAT.
 * stat_trigger also updates bit 2 of R_STAT to reflect whether LY=LYC.
 */

void IRAM_ATTR stat_trigger()
{
	static const int condbits[4] = { 0x08, 0x10, 0x20, 0x00 };
	int flag = 0;

	if (R_LY == R_LYC)
	{
		R_STAT |= 0x04;
		if (R_STAT & 0x40) flag = IF_STAT;
	}
	else R_STAT &= ~0x04;

	if (R_STAT & condbits[R_STAT&3]) flag = IF_STAT;

	if (!(R_LCDC & 0x80)) flag = 0;

	hw_interrupt(flag, IF_STAT);
}

void IRAM_ATTR stat_write(byte b)
{
	R_STAT = (R_STAT & 0x07) | (b & 0x78);
	if (!hw.cgb && !(R_STAT & 2)) /* DMG STAT write bug => interrupt */
		hw_interrupt(IF_STAT, IF_STAT);
	stat_trigger();
}


/*
 * stat_change is called when a transition results in a change to the
 * LCD STAT condition (the low 2 bits of R_STAT).  It raises or lowers
 * the VBLANK interrupt line appropriately and calls stat_trigger to
 * update the STAT interrupt line.
 */
 /* FIXME: function now will only lower vblank interrupt, description
does not match anymore */
static void IRAM_ATTR stat_change(int stat)
{
	stat &= 3;
	R_STAT = (R_STAT & 0x7C) | stat;

	if (stat != 1) hw_interrupt(0, IF_VBLANK);
	/* hw_interrupt((stat == 1) ? IF_VBLANK : 0, IF_VBLANK); */
	stat_trigger();
}


void IRAM_ATTR lcdc_change(byte b)
{
	byte old = R_LCDC;
	R_LCDC = b;
	if ((R_LCDC ^ old) & 0x80) /* lcd on/off change */
	{
		R_LY = 0;
		stat_change(2);
		C = 40;
		lcd_begin();
	}
}



/*
	LCD controller operates with 154 lines per frame, of which lines
	#0..#143 are visible and lines #144..#153 are processed in vblank
	state.

	lcdc_trans() performs cyclic switching between lcdc states (OAM
	search/data transfer/hblank/vblank), updates system state and time
	counters accordingly. Control is returned to the caller immediately
	after a step that sets LCDC ahead of CPU, so that LCDC is always
	ahead of CPU by one state change. Once CPU reaches same point in
	time, LCDC is advanced through the next step.

	For each visible line LCDC goes through states 2 (search), 3
	(transfer) and then 0 (hblank). At the first line of vblank LCDC
	is switched to state 1 (vblank) and remains there till line #0 is
	reached (execution is still interrupted after each line so that
	function could return if it ran out of time).

	Irregardless of state switches per line, time spent in each line
	adds up to exactly 228 double-speed cycles (109us).

	LCDC emulation begins with R_LCDC set to "operation enabled", R_LY
	set to line #0 and R_STAT set to state-hblank. cpu.lcdc is also
	set to zero, to begin emulation we call lcdc_trans() once to
	force-advance LCD through the first iteration.

	Docs aren't entirely accurate about time intervals within single
	line; besides that, intervals will vary depending on number of
	sprites on the line and probably other factors. States 1, 2 and 3
	do not require precise sub-line CPU-LCDC sync, but state 0 might do.
*/

/* lcdc_trans()
	Main LCDC emulation routine
*/
void IRAM_ATTR lcdc_trans()
{
	/* FIXME: lacks clarity;
	try and break into two switch() blocks
	switch(state) {
		case 0:
			if(vblank) state = 1;
			else state = 2;
		case 1:
			state = 2;
		case 2:
			state = 3;
		case 3:
			state = 0;
	}

	switch(state) {
		case 0:
			handle hblank
		case 1:
			handle vblank
		case 2:
			handle search
		case 3:
			handle transfer
	}
	*/
	if (!(R_LCDC & 0x80))
	{
		/* LCDC operation disabled (short route) */
		while (C <= 0)
		{
			switch ((byte)(R_STAT & 3))
			{
			case 0: /* hblank */
			case 1: /* vblank */
				lcd_refreshline();
				stat_change(2);
				C += 40;
				break;
			case 2: /* search */
				stat_change(3);
				C += 86;
				break;
			case 3: /* transfer */
				stat_change(0);
				/* FIXME: check docs; HDMA might require operating LCDC */
				if (hw.hdma & 0x80)
					hw_hdma();
				else
					C += 102;
				break;
			}
			return;
		}
	}
	while (C <= 0)
	{
		switch ((byte)(R_STAT & 3))
		{
		case 1:
			/* vblank -> */
			if (!(hw.ilines & IF_VBLANK))
			{
				C += 218;
				hw_interrupt(IF_VBLANK, IF_VBLANK);
				break;
			}
			if (R_LY == 0)
			{
				lcd_begin();
				stat_change(2); /* -> search */
				C += 40;
				break;
			}
			else if (R_LY < 152)
				C += 228;
			else if (R_LY == 152)
				/* Handling special case on the last line; see
				docs/HACKING */
				C += 28;
			else
			{
				R_LY = -1;
				C += 200;
			}
			R_LY++;
			stat_trigger();
			break;
		case 2:
			/* search -> */
			lcd_refreshline();
			stat_change(3); /* -> transfer */
			C += 86;
			break;
		case 3:
			/* transfer -> */
			stat_change(0); /* -> hblank */
			if (hw.hdma & 0x80)
				hw_hdma();
			/* FIXME -- how much of the hblank does hdma use?? */
			/* else */
			C += 102;
			break;
		case 0:
			/* hblank -> */
			if (++R_LY >= 144)
			{
				/* FIXME: pick _one_ place to trigger vblank interrupt
				this better be done here or within stat_change(),
				otherwise CPU will have a chance to run	for some time
				before interrupt is triggered */
				if (cpu.halt)
				{
					hw_interrupt(IF_VBLANK, IF_VBLANK);
					C += 228;
				}
				else C += 10;
				stat_change(1); /* -> vblank */
				break;
			}
			stat_change(2); /* -> search */
			C += 40;
			break;
		}
	}
}
