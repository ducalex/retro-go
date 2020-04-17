/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/***************************************************************************/
/*                                                                         */
/*                         HARDware PCEngine                               */
/*                                                                         */
/* This source file implements all functions relatives to pc engine inner  */
/* hardware (memory access e.g.)                                           */
/*                                                                         */
/***************************************************************************/

#include "utils.h"
#include "hard_pce.h"
#include "pce.h"
#if defined(BSD_CD_HARDWARE_SUPPORT)
#include "pcecd.h"
#else
	// nothing yet on purpose
#endif

/**
  * Variables declaration
  * cf explanations in the header file
  **/

struct_hard_pce *hard_pce;

uchar *RAM;

// Video
uint16 *SPRAM;
uchar *VRAM2;
uchar *VRAMS;
uchar *Pal;
uchar *vchange;
uchar *vchanges;
uchar *WRAM;
uchar *VRAM;
uint32 *p_scanline;

// Audio
uchar *PCM;

// I/O
IO *p_io;

// CD
 /**/ uchar * cd_read_buffer;
uchar *cd_sector_buffer;
uchar *cd_extra_mem;
uchar *cd_extra_super_mem;
uchar *ac_extra_mem;

uint32 pce_cd_read_datacnt;
 /**/ uchar cd_sectorcnt;
uchar pce_cd_curcmd;
 /**/
// Memory
	uchar * zp_base;
uchar *sp_base;
uchar *mmr;
uchar *IOAREA;

// Interruption
uint32 *p_cyclecount;
uint32 *p_cyclecountold;

//const uint32 TimerPeriod = 1097;

// registers

#if defined(SHARED_MEMORY)

//! Shared memory handle
static int shm_handle;

uint16 *p_reg_pc;
uchar *p_reg_a;
uchar *p_reg_x;
uchar *p_reg_y;
uchar *p_reg_p;
uchar *p_reg_s;

#else

DRAM_ATTR uint32 reg_pc_;
DRAM_ATTR uchar reg_a_;
DRAM_ATTR uchar reg_x_;
DRAM_ATTR uchar reg_y_;
DRAM_ATTR uchar reg_p_;
DRAM_ATTR uchar reg_s_;

#endif

// Mapping
//uchar *PageR[8];
// IRAM_ATTR slower
uchar **PageR;
uchar *ROMMapR[256];
//uchar **ROMMapR;

//uchar *PageW[8];
uchar **PageW;
uchar *ROMMapW[256];
//uchar **ROMMapW;

uchar *trap_ram_read;
uchar *trap_ram_write;

// Miscellaneous
//DRAM_ATTR uint32 *p_cycles;
DRAM_ATTR uint32 cycles_;
int32 *p_external_control_cpu;

//! External rom size hack for shared memory indication
extern int ROM_size;

/**
  * Predeclaration of access functions
	**/
uchar read_memory_simple(uint16);
uchar read_memory_sf2(uint16);
uchar read_memory_arcade_card(uint16);

void write_memory_simple(uint16, uchar);
void write_memory_arcade_card(uint16, uchar);



//! Function to write into memory. Defaulted to the basic one
void (*write_memory_function) (uint16, uchar) = write_memory_simple;

//! Function to read from memory. Defaulted to the basic one
uchar(*read_memory_function) (uint16) = read_memory_simple;

void
hard_reset_io(void)
{
    pair *VCE = io.VCE;
    uchar *psg_da_data[6];
    memcpy(psg_da_data,io.psg_da_data, sizeof(uchar *) * 6);
    memset(&io, 0, sizeof(IO));
    io.VCE = VCE;
    memcpy(io.psg_da_data, psg_da_data, sizeof(uchar *) * 6);
    
    memset(io.VCE, 0, 0x200*sizeof(pair));
    for (int i = 0;i <6; i++)
    {
       memset(io.psg_da_data[i], 0, PSG_DIRECT_ACCESS_BUFSIZE);
    }
    IO_VDC_reset
}

/**
  * Initialize the hardware
  **/
void
hard_init(void)
{
    PageR = (uchar **)my_special_alloc(true, 1,8*4);
    PageW = (uchar **)my_special_alloc(true, 1,8*4);
    //ROMMapR = (uchar **)my_special_alloc(true, 4,256*4);
    //ROMMapW = (uchar **)my_special_alloc(true, 4,256*4);
	trap_ram_read = malloc(0x2000);
	trap_ram_write = malloc(0x2000);
	//trap_ram_read = my_special_alloc(true, 1, 0x2000);
    //trap_ram_write = my_special_alloc(true, 1, 0x2000);

	if ((trap_ram_read == NULL) || (trap_ram_write == NULL))
		fprintf(stderr, "Couldn't allocate trap_ram* (%s:%d)", __FILE__,
				__LINE__);

#if defined(SHARED_MEMORY)
	shm_handle =
		shmget((key_t) SHM_HANDLE, sizeof(struct_hard_pce),
			   IPC_CREAT | IPC_EXCL | 0666);
	if (shm_handle == -1)
		fprintf(stderr, "Couldn't get shared memory\n");
	else {
		hard_pce = (struct_hard_pce *) shmat(shm_handle, NULL, 0);
		if (hard_pce == NULL)
			fprintf(stderr, "Couldn't attach shared memory\n");

		p_reg_pc = &hard_pce->s_reg_pc;
		p_reg_a = &hard_pce->s_reg_a;
		p_reg_x = &hard_pce->s_reg_x;
		p_reg_y = &hard_pce->s_reg_y;
		p_reg_p = &hard_pce->s_reg_p;
		p_reg_s = &hard_pce->s_reg_s;
		p_external_control_cpu = &hard_pce->s_external_control_cpu;

	}
	memset(hard_pce, 0, sizeof(struct_hard_pce));
#else
    bool fast1;
#ifdef MY_USE_FAST_RAM
    fast1 = true;
#else
    fast1 = false;
#endif
	//hard_pce = (struct_hard_pce *) malloc(sizeof(struct_hard_pce));
	hard_pce = (struct_hard_pce *) my_special_alloc(fast1, 1, sizeof(struct_hard_pce));
	memset(hard_pce, 0, sizeof(struct_hard_pce));
	
	hard_pce->RAM = (uchar *)my_special_alloc(fast1, 1, 0x8000);//[0x8000]
    hard_pce->PCM = (uchar *)my_special_alloc(false, 1, 0x10000);//[0x10000]
    hard_pce->WRAM = (uchar *)my_special_alloc(fast1, 1, 0x2000);//[0x2000]
    memset(hard_pce->RAM, 0, 0x8000*sizeof(uchar));
    memset(hard_pce->PCM, 0, 0x10000*sizeof(uchar));
    memset(hard_pce->WRAM, 0, 0x2000*sizeof(uchar));

    hard_pce->VRAM = (uchar *)my_special_alloc(false, 1, VRAMSIZE);//[VRAMSIZE]
    hard_pce->VRAM2 = (uchar *)my_special_alloc(false, 1, VRAMSIZE);//[VRAMSIZE];
    hard_pce->VRAMS = (uchar *)my_special_alloc(false, 1, VRAMSIZE);//[VRAMSIZE];
    hard_pce->vchange = (uchar *)my_special_alloc(false, 1, VRAMSIZE / 32);//[VRAMSIZE / 32];
    hard_pce->vchanges = (uchar *)my_special_alloc(false, 1, VRAMSIZE / 128);//[VRAMSIZE / 128];
    memset(hard_pce->VRAM, 0, VRAMSIZE*sizeof(uchar));
    memset(hard_pce->VRAM2, 0, VRAMSIZE*sizeof(uchar));
    memset(hard_pce->VRAMS, 0, VRAMSIZE*sizeof(uchar));
    memset(hard_pce->vchange, 0, VRAMSIZE/32*sizeof(uchar));
    memset(hard_pce->vchanges, 0, VRAMSIZE/128*sizeof(uchar));

/*    
#define cd_extra_mem_size 0x10000
#define cd_extra_super_mem_size 0x30000
#define ac_extra_mem_size 0x200000
#define cd_sector_buffer_size 0x2000
*/
#define cd_extra_mem_size 4
#define cd_extra_super_mem_size 4
#define ac_extra_mem_size 4
#define cd_sector_buffer_size 4

    hard_pce->cd_extra_mem = (uchar *)my_special_alloc(false, 1, cd_extra_mem_size);//[0x10000];
    hard_pce->cd_extra_super_mem = (uchar *)my_special_alloc(false, 1, cd_extra_super_mem_size);//[0x30000];
    hard_pce->ac_extra_mem = (uchar *)my_special_alloc(false, 1, ac_extra_mem_size);//[0x200000];
    hard_pce->cd_sector_buffer = (uchar *)my_special_alloc(false, 1, cd_sector_buffer_size);//[0x2000];
    memset(hard_pce->cd_extra_mem, 0, cd_extra_mem_size*sizeof(uchar));
    memset(hard_pce->cd_extra_super_mem, 0, cd_extra_super_mem_size*sizeof(uchar));
    memset(hard_pce->ac_extra_mem, 0, ac_extra_mem_size*sizeof(uchar));
    memset(hard_pce->cd_sector_buffer, 0, cd_sector_buffer_size*sizeof(uchar));

    //hard_pce->SPRAM = (uint16 *)my_special_alloc(false, 2, 64 * 4* sizeof(uint16));//[64 * 4];
    hard_pce->SPRAM = (uint16 *)my_special_alloc(fast1, 1, 64 * 4* sizeof(uint16));//[64 * 4];
    hard_pce->Pal = (uchar *)my_special_alloc(false, 1, 512);//[512];
    memset(hard_pce->SPRAM, 0, 64 * 4*sizeof(uint16));
    memset(hard_pce->Pal, 0, 512*sizeof(uchar));
    
    //hard_pce->s_io.VCE = (pair*)my_special_alloc(false, sizeof(pair), 0x200*sizeof(pair));//[0x200]
    hard_pce->s_io.VCE = (pair*)my_special_alloc(fast1, 1, 0x200*sizeof(pair));//[0x200]
    memset(hard_pce->s_io.VCE, 0, 0x200*sizeof(pair));
    for (int i = 0;i <6; i++)
    {
        hard_pce->s_io.psg_da_data[i] = (uchar *)my_special_alloc(false, 1, PSG_DIRECT_ACCESS_BUFSIZE);//[PSG_DIRECT_ACCESS_BUFSIZE];
        memset(hard_pce->s_io.psg_da_data[i], 0, PSG_DIRECT_ACCESS_BUFSIZE);
    }
#endif

	RAM = hard_pce->RAM;
	PCM = hard_pce->PCM;
	WRAM = hard_pce->WRAM;
	VRAM = hard_pce->VRAM;
	VRAM2 = hard_pce->VRAM2;
	VRAMS = (uchar *) hard_pce->VRAMS;
	vchange = hard_pce->vchange;
	vchanges = hard_pce->vchanges;

	cd_extra_mem = hard_pce->cd_extra_mem;
	cd_extra_super_mem = hard_pce->cd_extra_super_mem;
	ac_extra_mem = hard_pce->ac_extra_mem;
	cd_sector_buffer = hard_pce->cd_sector_buffer;

	SPRAM = hard_pce->SPRAM;
	Pal = hard_pce->Pal;

	p_scanline = &hard_pce->s_scanline;

	p_cyclecount = &hard_pce->s_cyclecount;
	p_cyclecountold = &hard_pce->s_cyclecountold;

	//p_cycles = &hard_pce->s_cycles;
	cycles_ = 0;

	mmr = hard_pce->mmr;

	p_io = &hard_pce->s_io;

#if defined(SHARED_MEMORY)
	/* Add debug on beginning option by setting 0 here */
	external_control_cpu = -1;

	hard_pce->rom_shared_memory_size = 0x2000 * ROM_size;

#endif

	if ((option.want_arcade_card_emulation) && (CD_emulation > 0)) {
		read_memory_function = read_memory_arcade_card;
		write_memory_function = write_memory_arcade_card;
	} else {
		read_memory_function = read_memory_simple;
		write_memory_function = write_memory_simple;
	}
}

/**
  *  Terminate the hardware
  **/
void
hard_term(void)
{
#if defined(SHARED_MEMORY)
	if (shmctl(shm_handle, IPC_RMID, NULL) == -1)
		fprintf(stderr, "Couldn't destroy shared memory\n");
#else
	free(hard_pce);
#endif
	free(trap_ram_read);
	free(trap_ram_write);
}

/**
 * Functions to access PCE hardware
 **/

int return_value_mask_tab_0002[32] = {
	0xFF,
	0xFF,
	0xFF,
	0xFF,						/* unused */
	0xFF,						/* unused */
	0xFF,
	0xFF,
	0xFF,
	0xFF,						/* 8 */
	0xFF,
	0x1F,						/* A */
	0x7F,
	0x1F,						/* C */
	0xFF,
	0xFF,						/* E */
	0x1F,
	0xFF,						/* 10 */
	0xFF,
	/* No data for remaining reg, assuming 0xFF */
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF
};

int return_value_mask_tab_0003[32] = {
	0xFF,
	0xFF,
	0xFF,
	0xFF,						/* unused */
	0xFF,						/* unused */
	0x1F,
	0x03,
	0x03,
	0x01,						/* 8 *//* ?? */
	0x00,
	0x7F,						/* A */
	0x7F,
	0xFF,						/* C */
	0x01,
	0x00,						/* E */
	0x00,
	0xFF,						/* 10 */
	0xFF,
	/* No data for remaining reg, assuming 0xFF */
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF
};


int return_value_mask_tab_0400[8] = {
	0xFF,
	0x00,
	0xFF,
	0x01,
	0xFF,
	0x01,
	0xFF,						/* unused */
	0xFF						/* unused */
};

int return_value_mask_tab_0800[16] = {
	0x03,
	0xFF,
	0xFF,
	0x0F,
	0xDF,
	0xFF,
	0x1F,
	0x9F,
	0xFF,
	0x83,
	/* No data for remainig reg, assuming 0xFF */
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF
};

int return_value_mask_tab_0c00[2] = {
	0x7F,
	0x01
};

int return_value_mask_tab_1400[4] = {
	0xFF,
	0xFF,
	0x03,
	0x03
};


//! Returns the useful value mask depending on port value
static int
return_value_mask(uint16 A)
{
	if (A < 0x400)				// VDC
	{
		if ((A & 0x3) == 0x02) {
			return return_value_mask_tab_0002[io.vdc_reg];
		} else if ((A & 0x3) == 0x03) {
			return return_value_mask_tab_0003[io.vdc_reg];
		} else
			return 0xFF;
	}

	if (A < 0x800)				// VCE
		return return_value_mask_tab_0400[A & 0x07];

	if (A < 0xC00)				/* PSG */
		return return_value_mask_tab_0800[A & 0x0F];

	if (A < 0x1000)				/* Timer */
		return return_value_mask_tab_0c00[A & 0x01];

	if (A < 0x1400)				/* Joystick / IO port */
		return 0xFF;

	if (A < 0x1800)				/* Interruption acknowledgement */
		return return_value_mask_tab_1400[A & 0x03];

	/* We don't know for higher ports */
	return 0xFF;
}

/* read */
uchar
IO_read_raw(uint16 A)
{
#include "IO_read_raw.h"
}

//! Adds the io_buffer feature
uchar
IO_read_(uint16 A)
{
	int mask;
	uchar temporary_return_value;

	if ((A < 0x800) || (A >= 0x1800))	// latch isn't affected out of the 0x800 - 0x1800 range
		return IO_read_raw(A);

	mask = return_value_mask(A);

	temporary_return_value = IO_read_raw(A);

	io.io_buffer = temporary_return_value | (io.io_buffer & ~mask);

	return io.io_buffer;
}

#if defined(TEST_ROM_RELOCATED)
extern uchar *ROM;
#endif

#ifndef MY_INLINE_bank_set
/**
  * Change bank setting
  **/
void
bank_set(uchar P, uchar V)
{

#if ENABLE_TRACING
	if (V >= 0x40 && V <= 0x43)
		TRACE("AC pseudo bank switching !!! (mmr[%d] = %d)\n", P, V);
#endif

#if defined(TEST_ROM_RELOCATED)
	if ((P >= 2) && ((V < 0x68) || (V >= 0x88))) {
		int physical_bank = mmr[reg_pc >> 13];
		if (physical_bank >= 0x68)
			physical_bank -= 0x68;
		printf
			("Relocation error PC = 0x%04x (logical bank 0x%0x(0x%02x), physical bank 0x%0x(0x%02x), offset 0x%04x, global offset 0x%x)\nBank %x into MMR %d\nPatching into BRK\n",
			 reg_pc, mmr[reg_pc >> 13], mmr[reg_pc >> 13], physical_bank,
			 physical_bank, reg_pc & 0x1FFF,
			 physical_bank * 0x2000 + (reg_pc & 0x1FFF), V, P);
		if (V >= 0x80) {
			printf("Not a physical bank, aborting patching\n");
		} else {
			V += 0x68;
			patch_rom(cart_name,
					  physical_bank * 0x2000 + (reg_pc & 0x1FFF), 0);
			ROM[physical_bank * 0x2000 + (reg_pc & 0x1FFF)] = 0;
		}
	}
	fprintf(stderr, "Bank set MMR[%d]=%02x at %04x\n", P, V, reg_pc);
#endif

	mmr[P] = V;
	if (ROMMapR[V] == IOAREA) {
		PageR[P] = IOAREA;
		PageW[P] = IOAREA;
	} else {
		PageR[P] = ROMMapR[V] - P * 0x2000;
		PageW[P] = ROMMapW[V] - P * 0x2000;
	}
#ifdef MY_PCENGINE_LOGGING
	printf("%s: %d,%d -> %X\n", __func__, P, V, PageR[P]);
#endif
}
#endif

void
write_memory_simple(uint16 A, uchar V)
{
	if (PageW[A >> 13] == IOAREA)
		IO_write(A, V);
	else
		PageW[A >> 13][A] = V;
}

void
write_memory_sf2(uint16 A, uchar V)
{
	if (PageW[A >> 13] == IOAREA)
		IO_write(A, V);
	else
		/* support for SF2CE silliness */
	if ((A & 0x1ffc) == 0x1ff0) {
		int i;

		ROMMapR[0x40] = ROMMapR[0] + 0x80000;
		ROMMapR[0x40] += (A & 3) * 0x80000;

		for (i = 0x41; i <= 0x7f; i++) {
			ROMMapR[i] = ROMMapR[i - 1] + 0x2000;
			// This could be slightly sped up by setting a fixed RAMMapW
			ROMMapW[i] = ROMMapW[i - 1] + 0x2000;
		}
	} else
		PageW[A >> 13][A] = V;
}

void
write_memory_arcade_card(uint16 A, uchar V)
{
	if ((mmr[A >> 13] >= 0x40) && (mmr[A >> 13] <= 0x43)) {
		/*
		   #if ENABLE_TRACING_CD
		   fprintf(stderr, "writing 0x%02x to AC pseudo bank (%d)\n", V, mmr[A >> 13] - 0x40);
		   #endif
		 */
		IO_write((uint16) (0x1A00 + ((mmr[A >> 13] - 0x40) << 4)), V);
	} else if (PageW[A >> 13] == IOAREA)
		IO_write(A, V);
	else
		PageW[A >> 13][A] = V;
}

uchar
read_memory_simple(uint16 A)
{
	if (PageR[A >> 13] != IOAREA)
		return PageR[A >> 13][A];
	else
		return IO_read(A);
}

uchar
read_memory_arcade_card(uint16 A)
{
	if ((mmr[A >> 13] >= 0x40) && (mmr[A >> 13] <= 0x43)) {
		/*
		   #if ENABLE_TRACING_CD
		   fprintf(stderr, "reading AC pseudo bank (%d)\n", mmr[A >> 13] - 0x40);
		   #endif
		 */
		return IO_read((uint16) (0x1A00 + ((mmr[A >> 13] - 0x40) << 4)));
	} else if (PageR[A >> 13] != IOAREA)
		return PageR[A >> 13][A];
	else
		return IO_read(A);
}

#ifndef MY_EXCLUDE

static char opcode_long_buffer[256];
static uint16 opcode_long_position;

char *
get_opcode_long()
{
	//! size of data used by the current opcode
	int size;

	uchar opcode;

	//! Buffer of opcode data, maximum is 7 (for xfer opcodes)
	uchar opbuf[7];

	int i;

	opcode = Rd6502(opcode_long_position);

	size = addr_info_debug[optable_debug[opcode].addr_mode].size;

	opbuf[0] = opcode;
	opcode_long_position++;
	for (i = 1; i < size; i++)
		opbuf[i] = Rd6502(opcode_long_position++);

	/* This line is the real 'meat' of the disassembler: */

	(*addr_info_debug[optable_debug[opcode].addr_mode].func)	/* function      */
		(opcode_long_buffer, opcode_long_position - size, opbuf, optable_debug[opcode].opname);	/* parm's passed */

	return opcode_long_buffer;

}

void
dump_pce_cpu_environment()
{

	int i;

	Log("Dumping PCE cpu environement\n");

	Log("PC = 0x%04x\n", reg_pc);
	Log("A = 0x%02x\n", reg_a);
	Log("X = 0x%02x\n", reg_x);
	Log("Y = 0x%02x\n", reg_y);
	Log("P = 0x%02x\n", reg_p);
	Log("S = 0x%02x\n", reg_s);

	for (i = 0; i < 8; i++) {
		Log("MMR[%d] = 0x%02x\n", i, mmr[i]);
	}

	opcode_long_position = reg_pc & 0xE000;

	while (opcode_long_position <= reg_pc) {
		Log("%04X: %s\n", opcode_long_position, get_opcode_long());
	}

	Log("--------------------------------------------------------\n");

}
#endif