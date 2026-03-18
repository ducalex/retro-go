#define VRC6_chr      regByte
#define VRC6_prg16    regByte[8]
#define VRC6_prg8     regByte[9]
#define VRC6_misc     regByte[10]

static void VRC6_sync() {
	int AND =prgAND >>1;
	int OR  =prgOR >>1;
	setprg16(0x8000, VRC6_prg16 &   AND |    OR &   ~AND);
	setprg8(0xC000,  VRC6_prg8  &prgAND | prgOR &~prgAND);
	setprg8(0xE000,              prgAND | prgOR &~prgAND);
	setchr1(0x0000, VRC6_chr[0]);
	setchr1(0x0400, VRC6_chr[1]);
	setchr1(0x0800, VRC6_chr[2]);
	setchr1(0x0C00, VRC6_chr[3]);
	setchr1(0x1000, VRC6_chr[4]);
	setchr1(0x1400, VRC6_chr[5]);
	setchr1(0x1800, VRC6_chr[6]);
	setchr1(0x1C00, VRC6_chr[7]);
	setmirror((VRC6_misc &0xC ^(VRC6_misc &0x8? 0: 0x4)) >>2);
}

static DECLFW(VRC6_writeReg) {
	uint8 index;
	switch (A &0xF003) {
	case 0x8000: case 0x8001: case 0x8002: case 0x8003:
		VRC6_prg16 =V;
		sync();
		break;
	case 0xB003:
		VRC6_misc =V;
		sync();
		break;
	case 0xC000: case 0xC001: case 0xC002: case 0xC003:
		VRC6_prg8 =V;
		sync();
		break;
	case 0xD000: case 0xD001: case 0xD002: case 0xD003: case 0xE000: case 0xE001: case 0xE002: case 0xE003:
		index =(A -0xD000) >>10 | A &3;
		VRC6_chr[index] =V;
		sync();
		break;
	case 0xF000:
		VRCIRQ_latch =V;
		break;
	case 0xF001:
		VRCIRQ_mode =V;
	        if (VRCIRQ_mode &0x02) {
			VRCIRQ_count =VRCIRQ_latch;
			VRCIRQ_cycles =341;
		}
		X6502_IRQEnd(FCEU_IQEXT);
		break;
	case 0xF002:
		VRCIRQ_mode =VRCIRQ_mode &~0x02 | VRCIRQ_mode <<1 &0x02;
		X6502_IRQEnd(FCEU_IQEXT);
		break;
	}
}

void VRC6_reset(uint8 clearRegs) {
	sync =VRC6_sync;
	prgAND =mapperFlags &2? 0x0F: 0x1F;
	MapIRQHook =VRCIRQ_cpuCycle;
	SetWriteHandler(0x8000, 0xFFFF, VRC6_writeReg);
	sync();
}

#undef VRC6_chr
#undef VRC6_prg16
#undef VRC6_prg8
#undef VRC6_misc
