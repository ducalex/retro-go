#define MMC24_reg regByte

static void MMC2_sync() {
	setprg8(0x8000, MMC24_reg[0] &prgAND | prgOR &~prgAND);
	setprg8(0xA000,         0xFD &prgAND | prgOR &~prgAND);
	setprg8(0xC000,         0xFE &prgAND | prgOR &~prgAND);
	setprg8(0xE000,         0xFF &prgAND | prgOR &~prgAND);
	setchr4(0x0000, MMC24_reg[1 +MMC24_reg[6]]);
	setchr4(0x1000, MMC24_reg[3 +MMC24_reg[7]]);
	setmirror(MMC24_reg[5] &1? MI_H: MI_V);
}

static void MMC4_sync() {
	int AND =prgAND >>1;
	int OR  =prgOR  >>1;
	setprg16(0x8000, MMC24_reg[0] &AND | OR &~AND);
	setprg16(0xC000,         0xFF &AND | OR &~AND);
	setchr4(0x0000, MMC24_reg[1 +MMC24_reg[6]]);
	setchr4(0x1000, MMC24_reg[3 +MMC24_reg[7]]);
	setmirror(MMC24_reg[5] &1? MI_H: MI_V);
}

static DECLFW(MMC24_writeReg) {
	MMC24_reg[(A >>12) -0xA] =V;
	sync();
}

static void FP_FASTAPASS(1) MMC24_ppuHook(uint32 A) {
	uint8 l, h = A >> 8;
	if (h >= 0x20 || ((h & 0xF) != 0xF)) return;
	l = A & 0xF0;
	if (h < 0x10) {
		if (l == 0xD0) {
			MMC24_reg[6] =0;
			sync();
		} else if (l == 0xE0) {
			MMC24_reg[6] =1;
			sync();
		}
	} else {
		if (l == 0xD0) {
			MMC24_reg[7] =0;
			sync();
		} else if (l == 0xE0) {
			MMC24_reg[7] =1;
			sync();
		}
	}
}

void MMC2_reset(uint8 clearRegs) {
	sync =MMC2_sync;
	prgAND =0x0F;
	PPU_hook =MMC24_ppuHook;
	SetWriteHandler(0xA000, 0xFFFF, MMC24_writeReg);
	sync();
}

void MMC4_reset(uint8 clearRegs) {
	sync =MMC4_sync;
	prgAND =mapperFlags &2? 0x0F: 0x1F;
	PPU_hook =MMC24_ppuHook;
	SetWriteHandler(0xA000, 0xFFFF, MMC24_writeReg);
	sync();
}

#undef MMC24_reg
