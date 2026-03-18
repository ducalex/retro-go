#define NANJING_reg regByte

static void NANJING_sync () {
	setprg32(0x8000, NANJING_reg[2] <<4 &0x30 | NANJING_reg[0] &0x0F | (NANJING_reg[3] &4? 0x00: 0x03) | prgOR >>2);
	setchr8(0);
	setmirror(mapperFlags &4? MI_H: MI_V);
}

static DECLFW(NANJING_writeReg) {
	NANJING_reg[A >>8 &3] =V;
	sync();
}

static void NANJING_horizontalBlanking(void) {
	if (NANJING_reg[0] &0x80 && scanline <239) {  /* Actual hardware cannot look at the current scanline number, but instead latches PA09 on PA13 rises. This does not seem possible with the current PPU emulation however. */
		setchr4(0x0000, scanline >=127? 1: 0);
		setchr4(0x1000, scanline >=127? 1: 0);
	} else
		setchr8(0);
}

void NANJING_reset(uint8 clearRegs) {
	sync =NANJING_sync;
	GameHBIRQHook = NANJING_horizontalBlanking;
	SetWriteHandler(0x5000, 0x53FF, NANJING_writeReg);
	sync();
}

#undef NANJING_reg
