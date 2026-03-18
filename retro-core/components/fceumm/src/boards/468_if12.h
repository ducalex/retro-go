#define IF12_reg regByte

static void IF12_sync () {
	int AND =prgAND >>1;
	int OR  =prgOR  >>1;
	setprg16(0x8000, IF12_reg[1] &AND | OR &~AND);
	setprg16(0xC000,        0xFF &AND | OR &~AND);
	setchr8(IF12_reg[0] >>1 &0xF);
	setmirror(IF12_reg[0] &1? MI_H: MI_V);
}

static DECLFW(IF12_writeReg) {
	IF12_reg[A >>14 &1] =V;
	sync();
}

void IF12_reset(uint8 clearRegs) {
	sync =IF12_sync;
	prgAND =mapperFlags &2? 0x0F: 0x1F;
	SetWriteHandler(0x8000, 0xFFFF, IF12_writeReg);
	sync();
}

void UNROM_IF12_reset(uint8 clearRegs) {
	if (mapperFlags &8)
		UNROM_reset(clearRegs);
	else
		IF12_reset(clearRegs);
}

#undef IF12_reg
