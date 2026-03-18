#define latch regByte[0]

static void ANROM_sync () {
	int AND =prgAND >>2;
	int OR  =prgOR  >>2;
	setprg32(0x8000, latch &AND | OR &~AND);
	setchr8(0);
	setmirror(latch &0x10? MI_1: MI_0);
}

static void BNROM_sync () {
	int AND =prgAND >>2;
	int OR  =prgOR  >>2;
	setprg32(0x8000, latch &AND | OR &~AND);
	setchr8(0);
	setmirror(mapperFlags &4? MI_H: MI_V);
}

static void GNROM_sync () {
	int AND =prgAND >>2;
	int OR  =prgOR  >>2 | (mapperFlags &4? 2: 0);
	setprg32(0x8000, latch >>4 &AND | OR &~AND);
	setchr8(latch &0x0F);
	setmirror(mapper &1? MI_H: MI_V);
}

static void UNROM_sync () {
	int AND =prgAND >>1;
	int OR  =prgOR  >>1;
	setprg16(0x8000, latch &AND | OR &~AND);
	setprg16(0xC000,  0xFF &AND | OR &~AND);
	setchr8(0);
	setmirror(mapperFlags &4? MI_H: MI_V);
}

static DECLFW(DISCRETE_writeLatch) {
	latch =V;
	sync();
}

static DECLFW(Portopia_writeLatch) {
	DISCRETE_writeLatch(A, A ==0xA000 && V ==0x00? 0x06: V); /* Strange hack, needed to get #282 "Portopia Serial Murder Case" on 852-in-1 running */
}

static DECLFW(ColorDreams_writeLatch) {
	DISCRETE_writeLatch(A, V >>4 &0xF | V <<4 &0xF0); /* Sswap nibbles to mimic GNROM */
}


void ANROM_BNROM_reset(uint8 clearRegs) {
	sync =mapperFlags &8? BNROM_sync: ANROM_sync;
	prgAND =(mapper ==0x06 || mapper ==0x16)? 0x3F: mapperFlags &2? 0x0F: 0x1F;
	SetWriteHandler(0x8000, 0xFFFF, DISCRETE_writeLatch);
	latch =0;
	sync();
}

void GNROM_reset(uint8 clearRegs) {
	sync =GNROM_sync;
	prgAND =mapperFlags &8? 0x07: 0x0F;
	SetWriteHandler(0x8000, 0xFFFF, misc &0x10 && mapper &~0x10? DISCRETE_writeLatch: ColorDreams_writeLatch);
	latch =0;
	sync();
}

void UNROM_reset(uint8 clearRegs) {
	sync =UNROM_sync;
	prgAND =mapper ==0x0B || misc2 &0x10? 0x3F: mapperFlags &2? 0x0F: 0x1F;
	SetWriteHandler(0x8000, 0xFFFF, mapper ==0x09 && mapperFlags ==0xE? Portopia_writeLatch: DISCRETE_writeLatch);
	sync();
}

#undef latch
