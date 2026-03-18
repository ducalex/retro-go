#define VRC7_chr  regByte
#define VRC7_prg0 regByte[8]
#define VRC7_prg1 regByte[9]
#define VRC7_prg2 regByte[10]
#define VRC7_misc regByte[11]

static void VRC7_sync() {
	setprg8(0x8000, VRC7_prg0 &prgAND | prgOR &~prgAND);
	setprg8(0xA000, VRC7_prg1 &prgAND | prgOR &~prgAND);
	setprg8(0xC000, VRC7_prg2 &prgAND | prgOR &~prgAND);
	setprg8(0xE000,            prgAND | prgOR &~prgAND);	
	setchr1(0x0000, VRC7_chr[0]);
	setchr1(0x0400, VRC7_chr[1]);
	setchr1(0x0800, VRC7_chr[2]);
	setchr1(0x0C00, VRC7_chr[3]);
	setchr1(0x1000, VRC7_chr[4]);
	setchr1(0x1400, VRC7_chr[5]);
	setchr1(0x1800, VRC7_chr[6]);
	setchr1(0x1C00, VRC7_chr[7]);
	setmirror(VRC7_misc &3 ^(VRC7_misc &2? 0: 1));
}

static DECLFW(VRC7_writeReg) {
	uint8 index;
	A =A &0xF000 | (A &0x18? 1: 0) | (A &0x20? 2: 0);
	switch (A &0xF003) {
	case 0x8000:
		VRC7_prg0 =V;
		sync();
		break;
	case 0x8001:
		VRC7_prg1 =V;
		sync();
		break;
	case 0x9000:
		VRC7_prg2 =V;
		sync();
		break;
	case 0x9001: case 0x9002:
		/* sound */
		break;
	case 0xA000: case 0xA001: case 0xB000: case 0xB001: case 0xC000: case 0xC001: case 0xD000: case 0xD001:
		index =(A -0xA000) >>11 | A &1;
		VRC7_chr[index] =V;
		sync();
		break;
	case 0xE000:
		VRC7_misc =V;
		sync();
		break;
	case 0xE001:
		VRCIRQ_latch =V;
		break;
	case 0xF000:
		VRCIRQ_mode =V;
		if (VRCIRQ_mode &0x02) {
			VRCIRQ_count =VRCIRQ_latch;
			VRCIRQ_cycles =341;
		}
		X6502_IRQEnd(FCEU_IQEXT);
		break;
	case 0xF001:
		VRCIRQ_mode =VRCIRQ_mode &~0x02 | VRCIRQ_mode <<1 &0x02;
		X6502_IRQEnd(FCEU_IQEXT);
		break;
	}
}

void VRC7_reset(uint8 clearRegs) {
	sync =VRC7_sync;
	prgAND =mapperFlags &8? (mapperFlags &4? (mapperFlags &2? 0x0F: 0x1F): 0x3F): 0x7F;
	MapIRQHook =VRCIRQ_cpuCycle;
	SetWriteHandler(0x8000, 0xFFFF, VRC7_writeReg);
	sync();
}

#undef VRC7_chr
#undef VRC7_prg0
#undef VRC7_prg1
#undef VRC7_prg2
#undef VRC7_misc

