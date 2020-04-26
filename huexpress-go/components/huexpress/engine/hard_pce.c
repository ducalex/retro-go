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

/**
  * Variables declaration
  * cf explanations in the header file
  **/

struct_hard_pce hard_pce;

// Memory
uchar *IOAREA;
uchar *TRAPRAM;

// Mapping
uchar *PageR[8];
uchar *PageW[8];
uchar *ROMMapR[256];
uchar *ROMMapW[256];

uint32 cycles = 0;

static const uint8 SaveRAM_Header[8] = { 'H', 'U', 'B', 'M', 0x00, 0x88, 0x10, 0x80 };

void
hard_reset(void)
{
	// Preserve our pointers
    uchar *superram = SuperRAM;
    uchar *extraram = ExtraRAM;
    uchar *vram2 = VRAM2;
    uchar *vrams = VRAMS;

    memset(&hard_pce, 0x00, sizeof(struct_hard_pce));
    memcpy(&SaveRAM, SaveRAM_Header, sizeof(SaveRAM_Header));

    SuperRAM = superram;
    ExtraRAM = extraram;
    VRAM2 = vram2;
    VRAMS = vrams;
}

/**
  * Initialize the hardware
  **/
void
hard_init(void)
{
    VRAMS = (uchar *)rg_alloc(VRAMSIZE, MEM_FAST);
    VRAM2 = (uchar *)rg_alloc(VRAMSIZE, MEM_FAST);
	TRAPRAM = (uchar *)rg_alloc(0x2004, MEM_FAST);
	IOAREA = TRAPRAM + 4;

    hard_reset();
}

/**
  *  Terminate the hardware
  **/
void
hard_term(void)
{
	free(TRAPRAM);
	free(VRAM2);
	free(VRAMS);
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
static inline int
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

    case 0x1A00:
    case 0x1AC0:
        Log("Arcade Card not supported : 0x%04X\n", A);
        break;

    case 0x1800:                // CD-ROM extention
		// Log("CD Emulation not implemented : 0x%04X\n", A);
		return 0;
    }
    return NODATA;
}


IRAM_ATTR inline uchar
IO_read(uint16 A)
{
	uchar temporary_return_value = IO_read_raw(A);

    MESSAGE_DEBUG("IO Read %02x at %04x\n", temporary_return_value, A);

	if ((A < 0x800) || (A >= 0x1800))
		return temporary_return_value;

	io.io_buffer = temporary_return_value | (io.io_buffer & ~return_value_mask(A));

	return io.io_buffer;
}


IRAM_ATTR inline void
IO_write(uint16 A, uchar V)
{
    MESSAGE_DEBUG("IO Write %02x at %04x\n", V, A);

    if ((A >= 0x800) && (A < 0x1800))   // We keep the io buffer value
        io.io_buffer = V;

    switch (A & 0x1F00) {
    case 0x0000:                /* VDC */
        switch (A & 3) {
        case 0:
            io.vdc_reg = V & 31;
            return;
        case 1:
            return;
        case 2:
            //printf("vdc_l%d,%02x ",io.vdc_reg,V);
            switch (io.vdc_reg) {
            case VWR:           /* Write to video */
                io.vdc_ratch = V;
                return;
            case HDR:           /* Horizontal Definition */
                {
                    typeof(io.screen_w) old_value = io.screen_w;
                    io.screen_w = (V + 1) * 8;

                    if (io.screen_w == old_value)
                        break;

                    // (*init_normal_mode[host.video_driver]) ();
                    gfx_need_video_mode_change = 1;
                    {
                        uint32 x, y =
                            (WIDTH - io.screen_w) / 2 - 512 * WIDTH;
                        for (x = 0; x < 1024; x++) {
                            // spr_init_pos[x] = y;
                            y += WIDTH;
                        }
                    }
                }
                break;

            case MWR:           /* size of the virtual background screen */
                {
                    static uchar bgw[] = { 32, 64, 128, 128 };
                    io.bg_h = (V & 0x40) ? 64 : 32;
                    io.bg_w = bgw[(V >> 4) & 3];
                }
                break;

            case BYR:           /* Vertical screen offset */
                /*
                   if (IO_VDC_08_BYR.B.l == V)
                   return;
                 */

                save_gfx_context(0);

                if (!scroll) {
                    oldScrollX = ScrollX;
                    oldScrollY = ScrollY;
                    oldScrollYDiff = ScrollYDiff;
                }
                IO_VDC_08_BYR.B.l = V;
                scroll = 1;
                ScrollYDiff = scanline - 1;
                ScrollYDiff -= IO_VDC_0C_VPR.B.h + IO_VDC_0C_VPR.B.l;

#if ENABLE_TRACING_DEEP_GFX
                TRACE("ScrollY = %d (l), ", ScrollY);
#endif
                return;
            case BXR:           /* Horizontal screen offset */

                /*
                   if (IO_VDC_07_BXR.B.l == V)
                   return;
                 */

                save_gfx_context(0);

                if (!scroll) {
                    oldScrollX = ScrollX;
                    oldScrollY = ScrollY;
                    oldScrollYDiff = ScrollYDiff;
                }
                IO_VDC_07_BXR.B.l = V;
                scroll = 1;
                return;

            case CR:
                if (IO_VDC_active.B.l == V)
                    return;
                save_gfx_context(0);
                IO_VDC_active.B.l = V;
                return;

            case VCR:
                IO_VDC_active.B.l = V;
                gfx_need_video_mode_change = 1;
                return;

            case HSR:
                IO_VDC_active.B.l = V;
                gfx_need_video_mode_change = 1;
                return;

            case VPR:
                IO_VDC_active.B.l = V;
                gfx_need_video_mode_change = 1;
                return;

            case VDW:
                IO_VDC_active.B.l = V;
                gfx_need_video_mode_change = 1;
                return;
            }

            IO_VDC_active.B.l = V;
            // all others reg just need to get the value, without additional stuff

#if ENABLE_TRACING_DEEP_GFX
            TRACE("VDC[%02x]=0x%02x, ", io.vdc_reg, V);
#endif

            if (io.vdc_reg > 19) {
                MESSAGE_DEBUG("ignore write lo vdc%d,%02x\n", io.vdc_reg, V);
            }

            return;
        case 3:
            switch (io.vdc_reg) {
            case VWR:           /* Write to mem */
                /* Writing to hi byte actually perform the action */
                VRAM[IO_VDC_00_MAWR.W * 2] = io.vdc_ratch;
                VRAM[IO_VDC_00_MAWR.W * 2 + 1] = V;

                plane_converted[IO_VDC_00_MAWR.W / 16] = 0;
                sprite_converted[IO_VDC_00_MAWR.W / 64] = 0;

                IO_VDC_00_MAWR.W += io.vdc_inc;

                /* vdc_ratch shouldn't be reset between writes */
                // io.vdc_ratch = 0;
                return;

            case VCR:
                IO_VDC_active.B.h = V;
                gfx_need_video_mode_change = 1;
                return;

            case HSR:
                IO_VDC_active.B.h = V;
                gfx_need_video_mode_change = 1;
                return;

            case VPR:
                IO_VDC_active.B.h = V;
                gfx_need_video_mode_change = 1;
                return;

            case VDW:           /* screen height */
                IO_VDC_active.B.h = V;
                gfx_need_video_mode_change = 1;
                return;

            case LENR:          /* DMA transfert */

                IO_VDC_12_LENR.B.h = V;

                {               // black-- 's code

                    int sourcecount = (IO_VDC_0F_DCR.W & 8) ? -1 : 1;
                    int destcount = (IO_VDC_0F_DCR.W & 4) ? -1 : 1;

                    int source = IO_VDC_10_SOUR.W * 2;
                    int dest = IO_VDC_11_DISTR.W * 2;

                    int i;

                    for (i = 0; i < (IO_VDC_12_LENR.W + 1) * 2; i++) {
                        *(VRAM + dest) = *(VRAM + source);
                        dest += destcount;
                        source += sourcecount;
                    }

                    /*
                       IO_VDC_10_SOUR.W = source;
                       IO_VDC_11_DISTR.W = dest;
                     */
                    // Erich Kitzmuller fix follows
                    IO_VDC_10_SOUR.W = source / 2;
                    IO_VDC_11_DISTR.W = dest / 2;

                }

                IO_VDC_12_LENR.W = 0xFFFF;

                memset(plane_converted, 0, sizeof(plane_converted));
                memset(sprite_converted, 0, sizeof(sprite_converted));

                /* TODO: check whether this flag can be ignored */
                io.vdc_status |= VDC_DMAfinish;

                return;

            case CR:            /* Auto increment size */
                {
                    static uchar incsize[] = { 1, 32, 64, 128 };
                    /*
                       if (IO_VDC_05_CR.B.h == V)
                       return;
                     */
                    save_gfx_context(0);

                    io.vdc_inc = incsize[(V >> 3) & 3];
                    IO_VDC_05_CR.B.h = V;
                }
                break;
            case HDR:           /* Horizontal display end */
                /* TODO : well, maybe we should implement it */
                //io.screen_w = (io.VDC_ratch[HDR]+1)*8;
                //TRACE0("HDRh\n");
#if ENABLE_TRACING_DEEP_GFX
                TRACE("VDC[HDR].h = %d, ", V);
#endif
                break;

            case BYR:           /* Vertical screen offset */

                /*
                   if (IO_VDC_08_BYR.B.h == (V & 1))
                   return;
                 */

                save_gfx_context(0);

                if (!scroll) {
                    oldScrollX = ScrollX;
                    oldScrollY = ScrollY;
                    oldScrollYDiff = ScrollYDiff;
                }
                IO_VDC_08_BYR.B.h = V & 1;
                scroll = 1;
                ScrollYDiff = scanline - 1;
                ScrollYDiff -= IO_VDC_0C_VPR.B.h + IO_VDC_0C_VPR.B.l;
#if ENABLE_TRACING_GFX
                if (ScrollYDiff < 0)
                    TRACE("ScrollYDiff went negative when substraction VPR.h/.l (%d,%d)\n",
                        IO_VDC_0C_VPR.B.h, IO_VDC_0C_VPR.B.l);
#endif

#if ENABLE_TRACING_DEEP_GFX
                TRACE("ScrollY = %d (h), ", ScrollY);
#endif

                return;

            case SATB:          /* DMA from VRAM to SATB */
                IO_VDC_13_SATB.B.h = V;
                io.vdc_satb = 1;
                io.vdc_status &= ~VDC_SATBfinish;
                return;

            case BXR:           /* Horizontal screen offset */

                if (IO_VDC_07_BXR.B.h == (V & 3))
                    return;

                save_gfx_context(0);

                if (!scroll) {
                    oldScrollX = ScrollX;
                    oldScrollY = ScrollY;
                    oldScrollYDiff = ScrollYDiff;
                }

                IO_VDC_07_BXR.B.h = V & 3;
                scroll = 1;
                return;
            }
            IO_VDC_active.B.h = V;

            if (io.vdc_reg > 19) {
                MESSAGE_DEBUG("ignore write hi vdc%d,%02x\n", io.vdc_reg, V);
            }

            return;
        }
        break;

    case 0x0400:                /* VCE */
        switch (A & 7) {
        case 0:
            /*TRACE("VCE 0, V=%X\n", V); */
            return;

            /* Choose color index */
        case 2:
            io.vce_reg.B.l = V;
            return;
        case 3:
            io.vce_reg.B.h = V & 1;
            return;

            /* Set RGB components for current choosen color */
        case 4:
            io.VCE[io.vce_reg.W].B.l = V;
            {
                uchar c;
                int i, n;
                n = io.vce_reg.W;
                c = io.VCE[n].W >> 1;
                if (n == 0) {
                    for (i = 0; i < 256; i += 16)
                        Palette[i] = c;
                } else if (n & 15)
                    Palette[n] = c;
            }
            return;

        case 5:
            io.VCE[io.vce_reg.W].B.h = V;
            {
                uchar c;
                int i, n;
                n = io.vce_reg.W;
                c = io.VCE[n].W >> 1;
                if (n == 0) {
                    for (i = 0; i < 256; i += 16)
                        Palette[i] = c;
                } else if (n & 15)
                    Palette[n] = c;
            }
            io.vce_reg.W = (io.vce_reg.W + 1) & 0x1FF;
            return;
        }
        break;


    case 0x0800:                /* PSG */

        switch (A & 15) {

            /* Select PSG channel */
        case 0:
            io.psg_ch = V & 7;
            return;

            /* Select global volume */
        case 1:
            io.psg_volume = V;
            return;

            /* Frequency setting, 8 lower bits */
        case 2:
            io.PSG[io.psg_ch][2] = V;
            break;

            /* Frequency setting, 4 upper bits */
        case 3:
            io.PSG[io.psg_ch][3] = V & 15;
            break;

        case 4:
            io.PSG[io.psg_ch][4] = V;
#if ENABLE_TRACING_AUDIO
            if ((V & 0xC0) == 0x40)
                io.PSG[io.psg_ch][PSG_DATA_INDEX_REG] = 0;
#endif
            break;

            /* Set channel specific volume */
        case 5:
            io.PSG[io.psg_ch][5] = V;
            break;

            /* Put a value into the waveform or direct audio buffers */
        case 6:
            if (io.PSG[io.psg_ch][PSG_DDA_REG] & PSG_DDA_DIRECT_ACCESS) {
                io.psg_da_data[io.psg_ch][io.psg_da_index[io.psg_ch]] = V;
                io.psg_da_index[io.psg_ch] =
                    (io.psg_da_index[io.psg_ch] + 1) & 0x3FF;
                if (io.psg_da_count[io.psg_ch]++ >
                    (PSG_DIRECT_ACCESS_BUFSIZE - 1)) {
                    if (!io.psg_channel_disabled[io.psg_ch])
                        MESSAGE_INFO
                            ("Audio being put into the direct access buffer faster than it's being played.\n");
                    io.psg_da_count[io.psg_ch] = 0;
                }
            } else {
                io.wave[io.psg_ch][io.PSG[io.psg_ch][PSG_DATA_INDEX_REG]] =
                    V;
                io.PSG[io.psg_ch][PSG_DATA_INDEX_REG] =
                    (io.PSG[io.psg_ch][PSG_DATA_INDEX_REG] + 1) & 0x1F;
            }
            break;

        case 7:
            io.PSG[io.psg_ch][7] = V;
            break;

        case 8:
            io.psg_lfo_freq = V;
            break;

        case 9:
            io.psg_lfo_ctrl = V;
            break;

#ifdef EXTRA_CHECKING
        default:
            fprintf(stderr, "ignored PSG write\n");
#endif
        }
        return;

    case 0x0c00:                /* timer */
        //TRACE("Timer Access: A=%X,V=%X\n", A, V);
        switch (A & 1) {
        case 0:
            io.timer_reload = V & 127;
            return;
        case 1:
            V &= 1;
            if (V && !io.timer_start)
                io.timer_counter = io.timer_reload;
            io.timer_start = V;
            return;
        }
        break;

    case 0x1000:                /* joypad */
//              TRACE("V=%02X\n", V);
        io.joy_select = V & 1;
        //io.joy_select = V;
        if (V & 2)
            io.joy_counter = 0;
        return;

    case 0x1400:                /* IRQ */
        switch (A & 15) {
        case 2:
            io.irq_mask = V;    /*TRACE("irq_mask = %02X\n", V); */
            return;
        case 3:
            io.irq_status = (io.irq_status & ~TIRQ) | (V & 0xF8);
            return;
        }
        break;

    case 0x1A00:
		Log("Arcade Card not supported : %d into 0x%04X\n", V, A);
        break;

    case 0x1800:                /* CD-ROM extention */
		// Log("CD Emulation not implemented : %d 0x%04X\n", V, A);
        break;
    }

    MESSAGE_DEBUG("ignore I/O write %04x,%02x\tBase adress of port %X\nat PC = %04X\n",
            A, V, A & 0x1CC0, reg_pc);
}

IRAM_ATTR inline uchar
IO_TimerInt()
{
	if (io.timer_start) {
		io.timer_counter--;
		if (io.timer_counter > 128) {
			io.timer_counter = io.timer_reload;

			if (!(io.irq_mask & TIRQ)) {
				io.irq_status |= TIRQ;
				return INT_TIMER;
			}
		}
	}
	return INT_NONE;
}

/**
  * Change bank setting
  **/
IRAM_ATTR void
bank_set(uchar P, uchar V)
{
	if (V >= 0x40 && V <= 0x43) {
		MESSAGE_DEBUG("AC pseudo bank switching !!! (mmr[%d] = %d)\n", P, V);
    }

	mmr[P] = V;
	if (ROMMapR[V] == IOAREA) {
		PageR[P] = IOAREA;
		PageW[P] = IOAREA;
	} else {
		PageR[P] = ROMMapR[V] - P * 0x2000;
		PageW[P] = ROMMapW[V] - P * 0x2000;
	}
}

#if 0
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