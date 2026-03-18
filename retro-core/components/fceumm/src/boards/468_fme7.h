#define FME7_reg regByte
#define FME7_index regByte[15]
#define FME7_counter regWord[0]

static void FME7_sync() {
	int AND =mapperFlags &8? 0xFF: 0x7F;
	switch(FME7_reg[8] &0xC0) {
	case 0x00: case 0x80:
		setprg8(0x6000, FME7_reg[0x8] &prgAND | prgOR &~prgAND);
		break;
	case 0x40:
		/* Open Bus */
		break;
	case 0xC0:
		setprg8r(0x10, 0x6000, 0);
		break;
	}
	
	setprg8(0x8000, FME7_reg[0x9] &prgAND | prgOR &~prgAND);
	setprg8(0xA000, FME7_reg[0xA] &prgAND | prgOR &~prgAND);
	setprg8(0xC000, FME7_reg[0xB] &prgAND | prgOR &~prgAND);
	setprg8(0xE000,                prgAND | prgOR &~prgAND);	
	setchr1(0x0000, FME7_reg[0x0] &AND);
	setchr1(0x0400, FME7_reg[0x1] &AND);
	setchr1(0x0800, FME7_reg[0x2] &AND);
	setchr1(0x0C00, FME7_reg[0x3] &AND);
	setchr1(0x1000, FME7_reg[0x4] &AND);
	setchr1(0x1400, FME7_reg[0x5] &AND);
	setchr1(0x1800, FME7_reg[0x6] &AND);
	setchr1(0x1C00, FME7_reg[0x7] &AND);
	setmirror(FME7_reg[0xC] &3 ^(FME7_reg[0xC] &2? 0: 1));
}

static DECLFW(FME7_writeIndex) {
	FME7_index =V &0xF;
}

static DECLFW(FME7_writeReg) {
	switch(FME7_index) {
	case 0xE:
		FME7_counter =FME7_counter &0xFF00 |V;
		break;
	case 0xF:
		FME7_counter =FME7_counter &0x00FF |V <<8;
		break;
	case 0xD:
		X6502_IRQEnd(FCEU_IQEXT);
		/* Falling through */
	default:
		FME7_reg[FME7_index] =V;
		sync();
	}
}

static void FP_FASTAPASS(1) FME7_cpuCycle(int a) {
	while (a--) {
		if (FME7_reg[0xD] &0x80 && !--FME7_counter && FME7_reg[0xD] &0x01) X6502_IRQBegin(FCEU_IQEXT);
	}
}

void FME7_reset(uint8 clearRegs) {
	sync =FME7_sync;
	prgAND =mapperFlags &2? 0x0F: 0x1F;
	MapIRQHook =FME7_cpuCycle;
	SetWriteHandler(0x8000, 0x9FFF, FME7_writeIndex);
	SetWriteHandler(0xA000, 0xBFFF, FME7_writeReg);
	sync();
}

#undef FME7_reg
#undef FME7_index
#undef FME7_counter
