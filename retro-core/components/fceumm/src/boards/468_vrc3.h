#define VRC3_prg   regByte[0]
#define VRC3_latch regWord[0]
#define VRC3_mode  regByte[1]
#define VRC3_count regWord[1]

static void VRC3_sync() {
	int AND =prgAND >>1;
	int OR  =prgOR >>1;
	setprg16(0x8000, VRC3_prg &AND | OR &~AND);
	setprg16(0xC000,           AND | OR &~AND);
	setchr8(0);
	setmirror(mapperFlags &4? MI_H: MI_V);
}

static DECLFW(VRC3_writeReg) {
	int shift;
	switch(A &0xF000) {
	case 0x8000: case 0x9000: case 0xA000: case 0xB000:
		V &=0xF;
		shift =A >>10 &0xC;
		VRC3_latch =VRC3_latch &~(0xF <<shift) | V <<shift;
		break;
	case 0xC000:
		VRC3_mode =V;
		if (VRC3_mode &0x02) VRC3_count =VRC3_latch;
		X6502_IRQEnd(FCEU_IQEXT);
		break;
	case 0xD000:
		VRC3_mode =VRC3_mode &~0x02 | VRC3_mode <<1 &0x02;
		X6502_IRQEnd(FCEU_IQEXT);
		break;
	case 0xF000:
		VRC3_prg =V;
		sync();
		break;
	}
}

static void FP_FASTAPASS(1) VRC3_cpuCycle(int a) {
	while (a--) {
		int mask =VRC3_mode &0x04? 0xFF: 0xFFFF;
		if (VRC3_mode &0x02) {
			if ((VRC3_count &mask) ==mask) {
				VRC3_count =VRC3_latch;
				X6502_IRQBegin(FCEU_IQEXT);
			} else
				++VRC3_count;
		}
	}
}

void VRC3_reset(uint8 clearRegs) {
	sync =VRC3_sync;
	prgAND =0x0F;
	MapIRQHook =VRC3_cpuCycle;
	SetWriteHandler(0x8000, 0xFFFF, VRC3_writeReg);
	sync();
}

#undef VRC3_prg
#undef VRC3_latch
#undef VRC3_mode
#undef VRC3_count
