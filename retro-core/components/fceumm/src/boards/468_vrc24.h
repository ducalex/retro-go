#define VRC24_prg       regByte
#define VRC24_mirroring regByte[2]
#define VRC24_misc      regByte[3]
#define VRC24_chr       regWord
#define VRCIRQ_latch    regByte[13]
#define VRCIRQ_mode     regByte[14]
#define VRCIRQ_count    regByte[15]
#define VRCIRQ_cycles   regWord[8]

static void VRC24_sync() {
	setprg8(0x8000 ^(VRC24_misc <<13 &0x4000), VRC24_prg[0] &prgAND | prgOR &~prgAND);
	setprg8(0xA000,                            VRC24_prg[1] &prgAND | prgOR &~prgAND);
	setprg8(0xC000 ^(VRC24_misc <<13 &0x4000),         0xFE &prgAND | prgOR &~prgAND);
	setprg8(0xE000,                                    0xFF &prgAND | prgOR &~prgAND);
	setchr1(0x0000, VRC24_chr[0]);
	setchr1(0x0400, VRC24_chr[1]);
	setchr1(0x0800, VRC24_chr[2]);
	setchr1(0x0C00, VRC24_chr[3]);
	setchr1(0x1000, VRC24_chr[4]);
	setchr1(0x1400, VRC24_chr[5]);
	setchr1(0x1800, VRC24_chr[6]);
	setchr1(0x1C00, VRC24_chr[7]);
	setmirror(VRC24_mirroring &3 ^(VRC24_mirroring &2? 0: 1));
}

static DECLFW(VRC24_writeReg) {
	uint8 index;
	A =A &0xF000 | (mapper &2? ((A &0xA? 1: 0) | (A &0x5? 2: 0)): ((A &0x5? 1: 0) | (A &0xA? 2: 0)));
	switch (A &0xF000) {
	case 0x8000: case 0xA000:
		VRC24_prg[A >>13 &1] =V;
		sync();
		break;
	case 0x9000:
		if (~A &2)
			VRC24_mirroring =V & (mapper &1? 3: 1);
		else
		if (~A &1 && mapper &1)
			VRC24_misc =V;
		sync();
		break;
	case 0xF000:
		if (mapper &1) switch (A &3) {
			case 0: VRCIRQ_latch =VRCIRQ_latch &0xF0 | V &0x0F; break;
			case 1: VRCIRQ_latch =VRCIRQ_latch &0x0F | V <<4;   break;
			case 2: VRCIRQ_mode =V;
				if (VRCIRQ_mode &0x02) {
					VRCIRQ_count =VRCIRQ_latch;
					VRCIRQ_cycles =341;
				}
				X6502_IRQEnd(FCEU_IQEXT);
				break;
			case 3: VRCIRQ_mode =VRCIRQ_mode &~0x02 | VRCIRQ_mode <<1 &0x02;
				X6502_IRQEnd(FCEU_IQEXT);
				break;
		}
		break;
	default:
		index =(A -0xB000) >>11 | A >>1 &1;
		if (A &1)
			VRC24_chr[index] =VRC24_chr[index] & 0x0F | V <<4;
		else
			VRC24_chr[index] =VRC24_chr[index] &~0x0F | V &0x0F;
		sync();
		break;
	}
}

static void FP_FASTAPASS(1) VRCIRQ_cpuCycle(int a) {
	while (a--) {
		if (VRCIRQ_mode &0x02 && (VRCIRQ_mode &0x04 || (VRCIRQ_cycles -=3) <=0)) {
			if (~VRCIRQ_mode &0x04) VRCIRQ_cycles +=341;
			if (!++VRCIRQ_count) {
				VRCIRQ_count =VRCIRQ_latch;
				X6502_IRQBegin(FCEU_IQEXT);
			}
		}
	}
}

void VRC24_reset(uint8 clearRegs) {
	sync =VRC24_sync;
	prgAND =mapperFlags &2? 0x0F: 0x1F;
	if (mapper &1) MapIRQHook =VRCIRQ_cpuCycle;
	SetWriteHandler(0x8000, 0xFFFF, VRC24_writeReg);
	sync();
}

#undef VRC24_prg
#undef VRC24_mirroring
#undef VRC24_misc
#undef VRC24_chr
