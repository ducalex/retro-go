#define MMC3_reg       regByte
#define MMC3_index     regByte[8]
#define MMC3_mirroring regByte[9]
#define MMC3_wram      regByte[10]
#define MMC3_reload    regByte[11]
#define MMC3_count     regByte[12]
#define MMC3_irq       regByte[13]
#define MMC3_lastReg   regByte[14]

static void MMC3_sync () {
	int chrAND =mapper &0x01? 0xFF: 0x7F;
	int OR =prgOR | (misc2 &1? 12: 0);
	setprg8(0x8000 ^(MMC3_index <<8 &0x4000), MMC3_reg[6] &prgAND | OR &~prgAND);
	setprg8(0xA000,                           MMC3_reg[7] &prgAND | OR &~prgAND);
	setprg8(0xC000 ^(MMC3_index <<8 &0x4000),        0xFE &prgAND | OR &~prgAND);
	setprg8(0xE000,                                  0xFF &prgAND | OR &~prgAND);
	setchr1(0x0000 ^(MMC3_index <<5 &0x1000),(MMC3_reg[0] &0xFE)&chrAND);
	setchr1(0x0400 ^(MMC3_index <<5 &0x1000),(MMC3_reg[0] |0x01)&chrAND);
	setchr1(0x0800 ^(MMC3_index <<5 &0x1000),(MMC3_reg[1] &0xFE)&chrAND);
	setchr1(0x0C00 ^(MMC3_index <<5 &0x1000),(MMC3_reg[1] |0x01)&chrAND);
	setchr1(0x1000 ^(MMC3_index <<5 &0x1000), MMC3_reg[2]       &chrAND);
	setchr1(0x1400 ^(MMC3_index <<5 &0x1000), MMC3_reg[3]       &chrAND);
	setchr1(0x1800 ^(MMC3_index <<5 &0x1000), MMC3_reg[4]       &chrAND);
	setchr1(0x1C00 ^(MMC3_index <<5 &0x1000), MMC3_reg[5]       &chrAND);
	
	if (mapper &2) switch(MMC3_mirroring &3) {
		case 0: setmirror(MI_V); break;
		case 1: setmirror(MI_H); break;
		case 2: setmirror(MMC3_reg[MMC3_lastReg] &0x80? MI_1: MI_0); break;
		case 3: setmirror(MI_1); break;		
	} else
		setmirror(MMC3_mirroring &1? MI_H: MI_V);
}

static DECLFW(MMC3_writeReg) {
	switch(A &0xE001) {
	case 0x8000: MMC3_index =V;              sync();    break;
	case 0x8001: MMC3_reg[MMC3_index &7] =V; sync();    break;
	case 0xA000: MMC3_mirroring =V;          sync();    break;
	case 0xA001: MMC3_wram =V;               sync();    break;
	case 0xC000: MMC3_reload =V;                        break;
	case 0xC001: MMC3_count =0;                         break;
	case 0xE000: MMC3_irq =0; X6502_IRQEnd(FCEU_IQEXT); break;
	case 0xE001: MMC3_irq =1;                           break;
	}
}

static void MMC3_horizontalBlanking(void) {
	MMC3_count = (MMC3_count == 0) ? MMC3_reload : (MMC3_count - 1);
	if (!MMC3_count && MMC3_irq) X6502_IRQBegin(FCEU_IQEXT);
}

static void FP_FASTAPASS(1) MMC3_ppuHook(uint32 A) {
	A &=0x1FFF;
	if (MMC3_index &0x80) A ^=0x1000;
	if (A <0x1000)
		MMC3_lastReg =A >>11;
	else
		MMC3_lastReg =(A >>10) -2;
	if ((MMC3_mirroring &3) ==2) setmirror(MMC3_reg[MMC3_lastReg] &0x80? MI_1: MI_0);
}

void MMC3_reset(uint8 clearRegs) {
	sync =MMC3_sync;
	GameHBIRQHook =MMC3_horizontalBlanking;
	if (mapper &2) PPU_hook =MMC3_ppuHook;
	prgAND =mapperFlags &8? (mapperFlags &4? (mapperFlags &2? (misc2 &2? 0x07: 0x0F): 0x1F): 0x3F): 0x7F;
	SetWriteHandler(0x8000, 0xFFFF, MMC3_writeReg);
	MMC3_mirroring =1; /* "Legendary Games of NES' 509-in-1"'s menu runs as MMC3 with H mirroring and expects that setting to stay when running a mapper 206 game such as Legend of Valkyrie. */
	sync();
}

#undef MMC3_reg
#undef MMC3_index
#undef MMC3_mirroring
#undef MMC3_wram
#undef MMC3_reload
#undef MMC3_count
#undef MMC3_irq
#undef MMC3_lastReg
