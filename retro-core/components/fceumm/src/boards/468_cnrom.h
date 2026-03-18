#define CNROM_reg regByte

static void CNROM_sync () {
	int OR  =prgOR >>1;
	if (mapperFlags &2) {
		setprg16(0x8000, CNROM_reg[2] <<1 &0xE | mapperFlags &1 | OR &~0xF);
		setprg16(0xC000, CNROM_reg[2] <<1 &0xE | mapperFlags &1 | OR &~0xF);
		setchr8(CNROM_reg[0] &0x03);
	} else {
		OR >>=1;
		setprg32(0x8000, CNROM_reg[2] &0x7 | OR &~0x7);
		setchr8(CNROM_reg[0] &0x0F);
	}
	if (mapperFlags &8)
		setmirror(mapperFlags &0x04? MI_H: MI_V);
	else
		setmirror(CNROM_reg[1] &0x10? MI_1: MI_0);
}

static DECLFW(CNROM_writeReg) {
	switch(A &0xE000) {
	case 0x8000: case 0xA000:
		CNROM_reg[0] =V;
		break;
	case 0xE000:
		CNROM_reg[2] =V;
		break;
	}
	sync();
}

static DECLFW(BF9097_writeMirroring) {
	CNROM_reg[1] =V;
	sync();
}

void CNROM_BF9097_reset(uint8 clearRegs) { /* This strange mapper is used for both (C)NROM games and FireHawk. What an absurd combination!*/
	sync =CNROM_sync;
	SetWriteHandler(0x8000, 0xFFFF, CNROM_writeReg);
	if (~mapperFlags &8)
		SetWriteHandler(0x9000, 0x9FFF, BF9097_writeMirroring);
	sync();
}

#undef CNROM_reg
