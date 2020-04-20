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
uchar cd_sectorcnt;
uchar pce_cd_curcmd;

// Memory
uchar *zp_base;
uchar *sp_base;
uchar *mmr;
uchar *IOAREA;

// Interruption
uint32 *p_cyclecount;
uint32 *p_cyclecountold;

//const uint32 TimerPeriod = 1097;

// registers

uint32 reg_pc_;
uchar reg_a_;
uchar reg_x_;
uchar reg_y_;
uchar reg_p_;
uchar reg_s_;

// Mapping
uchar *PageR[8];
uchar *PageW[8];
uchar *ROMMapR[256];
uchar *ROMMapW[256];

uchar *trap_ram_read;
uchar *trap_ram_write;

// Miscellaneous
///* DRAM_ATTR */ uint32 *p_cycles;
/* DRAM_ATTR */ uint32 cycles_;
int32 *p_external_control_cpu;

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
    uchar *psg_da_data[6];

    memcpy(psg_da_data, io.psg_da_data, sizeof(uchar *) * 6);
    memset(&io, 0, sizeof(IO));
    memcpy(io.psg_da_data, psg_da_data, sizeof(uchar *) * 6);

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
	trap_ram_read = rg_alloc(0x2000, MEM_SLOW);
	trap_ram_write = rg_alloc(0x2000, MEM_SLOW);

	assert(trap_ram_read && trap_ram_write);

	hard_pce = (struct_hard_pce *) rg_alloc(sizeof(struct_hard_pce), MEM_FAST);
    hard_pce->PCM   = (uchar *)rg_alloc(0x10000, MEM_SLOW);
    hard_pce->VRAM  = (uchar *)rg_alloc(VRAMSIZE, MEM_SLOW);
    hard_pce->VRAM2 = (uchar *)rg_alloc(VRAMSIZE, MEM_SLOW);
    hard_pce->VRAMS = (uchar *)rg_alloc(VRAMSIZE, MEM_SLOW);

    hard_pce->cd_extra_mem       =
    hard_pce->cd_extra_super_mem =
    hard_pce->ac_extra_mem       =
    hard_pce->cd_sector_buffer   = (uchar *)rg_alloc(0x2000, MEM_SLOW);

    for (int i = 0;i < 6; i++)
    {
        hard_pce->s_io.psg_da_data[i] = (uchar *)rg_alloc(PSG_DIRECT_ACCESS_BUFSIZE, MEM_SLOW);
    }

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
	free(hard_pce);
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
static inline uchar
IO_read_raw(uint16 A)
{
    uchar ret;

#ifndef FINAL_RELEASE
    if ((A & 0x1F00) == 0x1A00)
        Log("AC Read at %04x\n", A);
#endif

    switch (A & 0x1FC0) {
    case 0x0000:                /* VDC */
        switch (A & 3) {
        case 0:
#if ENABLE_TRACING_DEEP_GFX
            TRACE("Returning vdc_status = 0x%02x\n",
                io.vdc_status);
#endif
            ret = io.vdc_status;
            io.vdc_status = 0;  //&=VDC_InVBlank;//&=~VDC_BSY;
#if ENABLE_TRACING_DEEP_GFX
            Log("$0000 returns %02X\n", ret);
#endif
            return ret;
        case 1:
            return 0;
        case 2:
            if (io.vdc_reg == VRR)
                return VRAM[IO_VDC_01_MARR.W * 2];
            else
                return IO_VDC_active.B.l;
        case 3:
            if (io.vdc_reg == VRR) {
                ret = VRAM[IO_VDC_01_MARR.W * 2 + 1];
                IO_VDC_01_MARR.W += io.vdc_inc;
                return ret;
            } else
                return IO_VDC_active.B.h;
        }
        break;

    case 0x0400:                /* VCE */
        switch (A & 7) {
        case 4:
            return io.VCE[io.vce_reg.W].B.l;
        case 5:
            return io.VCE[io.vce_reg.W++].B.h;
        }
        break;
    case 0x0800:                /* PSG */
        switch (A & 15) {
        case 0:
            return io.psg_ch;
        case 1:
            return io.psg_volume;
        case 2:
            return io.PSG[io.psg_ch][2];
        case 3:
            return io.PSG[io.psg_ch][3];
        case 4:
            return io.PSG[io.psg_ch][4];
        case 5:
            return io.PSG[io.psg_ch][5];
        case 6:
            {
                int ofs = io.PSG[io.psg_ch][PSG_DATA_INDEX_REG];
                io.PSG[io.psg_ch][PSG_DATA_INDEX_REG] =
                    (uchar) ((io.PSG[io.psg_ch][PSG_DATA_INDEX_REG] +
                              1) & 31);
                return io.wave[io.psg_ch][ofs];
            }
        case 7:
            return io.PSG[io.psg_ch][7];

        case 8:
            return io.psg_lfo_freq;
        case 9:
            return io.psg_lfo_ctrl;
        default:
            return NODATA;
        }
        break;
    case 0x0c00:                /* timer */
        return io.timer_counter;

    case 0x1000:                /* joypad */
        ret = io.JOY[io.joy_counter] ^ 0xff;
        if (io.joy_select & 1)
            ret >>= 4;
        else {
            ret &= 15;
            io.joy_counter = (uchar) ((io.joy_counter + 1) % 5);
        }

        /* return ret | Country; *//* country 0:JPN 1<<6=US */
        return ret | 0x30;      // those 2 bits are always on, bit 6 = 0 (Jap), bit 7 = 0 (Attached cd)

    case 0x1400:                /* IRQ */
        switch (A & 15) {
        case 2:
            return io.irq_mask;
        case 3:
            ret = io.irq_status;
            io.irq_status = 0;
            return ret;
        }
        break;


    case 0x18C0:                // Memory management ?
        switch (A & 15) {
        case 5:
        case 1:
            return 0xAA;
        case 2:
        case 6:
            return 0x55;
        case 3:
        case 7:
            return 0x03;
        }
        break;

    case 0x1AC0:
        switch (A & 15) {
        case 0:
            return (uchar) (io.ac_shift);
        case 1:
            return (uchar) (io.ac_shift >> 8);
        case 2:
            return (uchar) (io.ac_shift >> 16);
        case 3:
            return (uchar) (io.ac_shift >> 24);
        case 4:
            return io.ac_shiftbits;
        case 5:
            return io.ac_unknown4;
        case 14:
            return (uchar) (option.
                            want_arcade_card_emulation ? 0x10 : NODATA);
        case 15:
            return (uchar) (option.
                            want_arcade_card_emulation ? 0x51 : NODATA);
        default:
            Log("Unknown Arcade card port access : 0x%04X\n", A);
        }
        break;

    case 0x1A00:
        {
            uchar ac_port = (uchar) ((A >> 4) & 3);
            switch (A & 15) {
            case 0:
            case 1:
                /*
                 * switch (io.ac_control[ac_port] & (AC_USE_OFFSET | AC_USE_BASE))
                 * {
                 * case 0:
                 * return ac_extra_mem[0];
                 * case AC_USE_OFFSET:
                 * ret = ac_extra_mem[io.ac_offset[ac_port]];
                 * if (!(io.ac_control[ac_port] & AC_INCREMENT_BASE))
                 * io.ac_offset[ac_port]+=io.ac_incr[ac_port];
                 * return ret;
                 * case AC_USE_BASE:
                 * ret = ac_extra_mem[io.ac_base[ac_port]];
                 * if (io.ac_control[ac_port] & AC_INCREMENT_BASE)
                 * io.ac_base[ac_port]+=io.ac_incr[ac_port];
                 * return ret;
                 * default:
                 * ret = ac_extra_mem[io.ac_base[ac_port] + io.ac_offset[ac_port]];
                 * if (io.ac_control[ac_port] & AC_INCREMENT_BASE)
                 * io.ac_base[ac_port]+=io.ac_incr[ac_port];
                 * else
                 * io.ac_offset[ac_port]+=io.ac_incr[ac_port];
                 * return ret;
                 * }
                 * return 0;
                 */


#if ENABLE_TRACING_CD
                printf
                    ("Reading from AC main port. ac_port = %d. %suse offset. %sincrement. %sincrement base\n",
                     ac_port,
                     io.ac_control[ac_port] & AC_USE_OFFSET ? "" : "not ",
                     io.ac_control[ac_port] & AC_ENABLE_INC ? "" : "not ",
                     io.
                     ac_control[ac_port] & AC_INCREMENT_BASE ? "" :
                     "not ");
#endif
                if (io.ac_control[ac_port] & AC_USE_OFFSET)
                    ret = ac_extra_mem[((io.ac_base[ac_port] +
                                         io.ac_offset[ac_port]) &
                                        0x1fffff)];
                else
                    ret = ac_extra_mem[((io.ac_base[ac_port]) & 0x1fffff)];

                if (io.ac_control[ac_port] & AC_ENABLE_INC) {
                    if (io.ac_control[ac_port] & AC_INCREMENT_BASE)
                        io.ac_base[ac_port] =
                            (io.ac_base[ac_port] +
                             io.ac_incr[ac_port]) & 0xffffff;
                    else
                        io.ac_offset[ac_port] = (uint16)
                            ((io.ac_offset[ac_port] +
                              io.ac_incr[ac_port]) & 0xffff);
                }
#if ENABLE_TRACING_CD
                printf
                    ("Returned 0x%02x. now, base = 0x%x. offset = 0x%x, increment = 0x%x\n",
                     ret, io.ac_base[ac_port], io.ac_offset[ac_port],
                     io.ac_incr[ac_port]);
#endif
                return ret;


            case 2:
                return (uchar) (io.ac_base[ac_port]);
            case 3:
                return (uchar) (io.ac_base[ac_port] >> 8);
            case 4:
                return (uchar) (io.ac_base[ac_port] >> 16);
            case 5:
                return (uchar) (io.ac_offset[ac_port]);
            case 6:
                return (uchar) (io.ac_offset[ac_port] >> 8);
            case 7:
                return (uchar) (io.ac_incr[ac_port]);
            case 8:
                return (uchar) (io.ac_incr[ac_port] >> 8);
            case 9:
                return io.ac_control[ac_port];
            default:
                Log("Unknown Arcade card port access : 0x%04X\n", A);
            }
            break;
        }
    case 0x1800:                // CD-ROM extention
        return pce_cd_handle_read_1800(A);
    }
    return NODATA;
}

//! Adds the io_buffer feature
IRAM_ATTR inline uchar
IO_read(uint16 A)
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