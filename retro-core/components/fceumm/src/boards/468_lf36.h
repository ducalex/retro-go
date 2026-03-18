#define LF36_prg regByte[0]
#define LF36_irq regByte[1]
#define LF36_counter regWord[0]

static void LF36_sync () {
	int OR  =prgOR | mapperFlags &0x08;
	setprg8(0x8000,           0x04 | OR);
	setprg8(0xA000,           0x05 | OR);
	setprg8(0xC000, LF36_prg &0x07 | OR);
	setprg8(0xE000,           0x07 | OR);
	setchr8(0);
	setmirror(mapperFlags &4? MI_H: MI_V);
}

static DECLFW(LF36_writeReg) {
	switch (A &0xE000) {
	case 0x8000: LF36_irq =0; break;
	case 0xA000: LF36_irq =1; break;
	case 0XE000: LF36_prg =V; sync(); break;
	}
}

static void FP_FASTAPASS(1) LF36_cpuCycle(int a) {
	while (a--) {
		if (LF36_irq) {
			if (++LF36_counter &0x1000)
				X6502_IRQBegin(FCEU_IQEXT);
			else
				X6502_IRQEnd(FCEU_IQEXT);
		} else {
			X6502_IRQEnd(FCEU_IQEXT);
			LF36_counter =0;
		}
	}
}

void LF36_reset(uint8 clearRegs) {
	sync =LF36_sync;
	MapIRQHook =LF36_cpuCycle;
	SetWriteHandler(0x8000, 0xFFFF, LF36_writeReg);
	sync();
}

#undef LF36_prg
#undef LF36_irq
#undef LF36_counter
