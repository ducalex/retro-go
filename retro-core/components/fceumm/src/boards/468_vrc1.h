#define VRC1_reg regByte

static void VRC1_sync () {
	setprg8(0x8000, VRC1_reg[0] &prgAND | prgOR &~prgAND);
	setprg8(0xA000, VRC1_reg[2] &prgAND | prgOR &~prgAND);
	setprg8(0xC000, VRC1_reg[4] &prgAND | prgOR &~prgAND);
	setprg8(0xE000,        0xFF &prgAND | prgOR &~prgAND);
	setchr4(0x0000, VRC1_reg[6] &0x0F | VRC1_reg[1] <<3 &0x10);
	setchr4(0x1000, VRC1_reg[7] &0x0F | VRC1_reg[1] <<2 &0x10);
	setmirror(VRC1_reg[1] &1? MI_H: MI_V);
}

static DECLFW(VRC1_writeReg) {
	VRC1_reg[A >>12 &7] =V;
	sync();
}

void VRC1_reset(uint8 clearRegs) {
	sync =VRC1_sync;
	prgAND =mapperFlags &8? (mapperFlags &4? (mapperFlags &2? 0x0F: 0x1F): 0x3F): 0x7F;
	SetWriteHandler(0x8000, 0xFFFF, VRC1_writeReg);
	sync();
}

#undef VRC1_reg
