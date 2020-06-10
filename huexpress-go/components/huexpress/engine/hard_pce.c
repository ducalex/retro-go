// hard_pce.c - Memory/IO/Timer emulation
//
#include "hard_pce.h"
#include "utils.h"
#include "pce.h"

/**
  * Variables declaration
  * cf explanations in the header file
  **/

hard_pce_t PCE;

// Since no read or write should occur we can point our
// trap ram to vram to save some internal memory
uchar *IOAREA, *TRAPRAM;

// Memory Mapping
uchar *PageR[8];
uchar *PageW[8];
uchar *MemoryMapR[256];
uchar *MemoryMapW[256];

const uint8 BackupRAM_Header[8] = { 'H', 'U', 'B', 'M', 0x00, 0x88, 0x10, 0x80 };


/**
  * Reset the hardware
  **/
void
hard_reset(void)
{
    uchar *extraram = ExtraRAM;

    memset(&PCE, 0, sizeof(PCE));
	memset(&SPR_CACHE, 0, sizeof(SPR_CACHE));

    memcpy(&BackupRAM, BackupRAM_Header, sizeof(BackupRAM_Header));

    ExtraRAM = extraram;

	// Reset IO lines
	io.vdc_status = 0;
	io.vdc_inc = 1;
	io.vdc_minline = 0;
	io.vdc_maxline = 255;
	io.screen_w = 256; // 255
	io.screen_h = 240; // 224

	// Reset sound generator values
	for (int i = 0; i < PSG_CHANNELS; i++) {
		io.PSG[i][4] = 0x80;
	}

	// Reset memory banking
    bank_set(7, 0x00);
    bank_set(6, 0x05);
    bank_set(5, 0x04);
    bank_set(4, 0x03);
    bank_set(3, 0x02);
    bank_set(2, 0x01);
    bank_set(1, 0xF8);
    bank_set(0, 0xFF);

	// Reset CPU
	reg_a = reg_x = reg_y = 0x00;
	reg_p = FL_TIQ;
	reg_s = 0xFF;
    reg_pc = Read16(VEC_RESET);
}

/**
  * Initialize the hardware
  **/
int
hard_init(void)
{
    TRAPRAM = &VRAM[0x8000];
    IOAREA  = &VRAM[0xA000];

	for (int i = 0; i < 0xFF; i++) {
		MemoryMapR[i] = TRAPRAM;
		MemoryMapW[i] = TRAPRAM;
	}

    MemoryMapR[0xF7] = BackupRAM;
    MemoryMapW[0xF7] = BackupRAM;
    MemoryMapR[0xF8] = RAM;
    MemoryMapW[0xF8] = RAM;
    MemoryMapR[0xFF] = IOAREA;
    MemoryMapW[0xFF] = IOAREA;

    // hard_reset();

    return 0;
}

/**
  * Terminate the hardware
  **/
void
hard_term(void)
{
	if (ExtraRAM) free(ExtraRAM);
}


/**
 * Functions to access PCE hardware
 **/


//! Returns the useful value mask depending on port value
static inline uchar
return_value_mask(uint16 A)
{
	if (A < 0x400) {			/* VDC */
		if ((A & 0x3) == 0x02) {
            switch (io.vdc_reg) {
                case 0xA: return 0x1F;
                case 0xB: return 0x7F;
                case 0xC: return 0x1F;
                case 0xF: return 0x1F;
                default:  return 0xFF;
            }
		} else if ((A & 0x3) == 0x03) {
            switch (io.vdc_reg) {
                case 0x5: return 0x1F;
                case 0x6: return 0x03;
                case 0x7: return 0x03;
                case 0x8: return 0x01;
                case 0x9: return 0x00;
                case 0xA: return 0x7F;
                case 0xB: return 0x7F;
                case 0xC: return 0xFF;
                case 0xD: return 0x01;
                case 0xE: return 0x00;
                case 0xF: return 0x00;
                default:  return 0xFF;
            }
		}
		return 0xFF;
	}

	if (A < 0x800) {			/* VCE */
        switch (A & 0x07) {
            case 0x1: return 0x03;
            case 0x3: return 0x01;
            case 0x5: return 0x01;
            default:  return 0xFF;
        }
    }

	if (A < 0xC00) {			/* PSG */
        switch (A & 0x0F) {
            case 0x0: return 0x00;
            case 0x3: return 0x0F;
            case 0x4: return 0xDF;
            case 0x6: return 0x1F;
            case 0x7: return 0x9F;
            case 0x9: return 0x83;
            default:  return 0xFF;
        }
    }

	if (A < 0x1000)				/* Timer */
		return (A & 1) ? 0x01 : 0x7F;

	if (A < 0x1400)				/* Joystick / IO port */
		return 0xFF;

	if (A < 0x1800)				/* Interruption acknowledgement */
		return (A & 2) ? 0x03 : 0xFF;

	/* We don't know for higher ports */
	return 0xFF;
}

/* read */
static inline uchar
IO_read_raw(uint16 A)
{
    uchar ret, ofs;

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
            TRACE("$0000 returns %02X\n", ret);
#endif
            return ret;
        case 1:
            return 0;
        case 2:
            if (io.vdc_reg == VRR)
                return VRAM[IO_VDC_REG[MARR].W * 2];
            else
                return IO_VDC_REG_ACTIVE.B.l;
        case 3:
            if (io.vdc_reg == VRR) {
                ret = VRAM[IO_VDC_REG[MARR].W * 2 + 1];
                IO_VDC_REG[MARR].W += io.vdc_inc;
                return ret;
            } else
                return IO_VDC_REG_ACTIVE.B.h;
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
            ofs = io.PSG[io.psg_ch][PSG_DATA_INDEX_REG];
            io.PSG[io.psg_ch][PSG_DATA_INDEX_REG] =
                (uchar) ((io.PSG[io.psg_ch][PSG_DATA_INDEX_REG] + 1) & 31);
            return io.PSG_WAVE[io.psg_ch][ofs];
        case 7:
            return io.PSG[io.psg_ch][7];

        case 8:
            return io.psg_lfo_freq;
        case 9:
            return io.psg_lfo_ctrl;
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

    case 0x1A00:                // Arcade Card
    case 0x1AC0:
        MESSAGE_INFO("Arcade Card not supported : 0x%04X\n", A);
        break;

    case 0x1800:                // CD-ROM extention
		MESSAGE_INFO("CD Emulation not implemented : 0x%04X\n", A);
		break;
    }

    // Open bus
    return 0xFF;
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

    uchar bgw[] = { 32, 64, 128, 128 };
    uchar incsize[] = { 1, 32, 64, 128 };

    switch (A & 0x1F00) {
    case 0x0000:                /* VDC */
        switch (A & 3) {
        case 0:
            io.vdc_reg = V & 31;
            return;
        case 1:
            return;
        case 2:
#if ENABLE_TRACING_DEEP_GFX
            TRACE("VDC[%02x]=0x%02x, ", io.vdc_reg, V);
#endif
            switch (io.vdc_reg) {
            case VWR:           /* Write to video */
                io.vdc_ratch = V;
                break;

            case HDR:           /* Horizontal Definition */
                if (io.screen_w != ((V + 1) * 8)) {
                    io.screen_w = (V + 1) * 8;
                    io.vdc_mode_chg = 1;
                }
                IO_VDC_REG_ACTIVE.B.l = V;
                break;

            case MWR:           /* size of the virtual background screen */
                io.bg_h = (V & 0x40) ? 64 : 32;
                io.bg_w = bgw[(V >> 4) & 3];
                IO_VDC_REG_ACTIVE.B.l = V;
                break;

            case BYR:           /* Vertical screen offset */
                /*
                   if (IO_VDC_REG[BYR].B.l == V)
                   return;
                 */
                gfx_save_context(0);

                IO_VDC_REG[BYR].B.l = V;
                ScrollYDiff = Scanline - 1;
                ScrollYDiff -= IO_VDC_REG[VPR].B.h + IO_VDC_REG[VPR].B.l;
#if ENABLE_TRACING_DEEP_GFX
                TRACE("ScrollY = %d (l), ", ScrollY);
#endif
                break;

            case BXR:           /* Horizontal screen offset */
                /*
                   if (IO_VDC_REG[BXR].B.l == V)
                   return;
                 */
                gfx_save_context(0);

                IO_VDC_REG[BXR].B.l = V;
                break;

            case CR:
                if (IO_VDC_REG_ACTIVE.B.l == V)
                    break;

                gfx_save_context(0);
                IO_VDC_REG_ACTIVE.B.l = V;
                break;

            case VCR:
            case HSR:
            case VPR:
            case VDW:
                io.vdc_mode_chg = 1;
            default:
                IO_VDC_REG_ACTIVE.B.l = V;
                break;
            }
            return;
        case 3:
            switch (io.vdc_reg) {
            case VWR:           /* Write to mem */
                /* Writing to hi byte actually perform the action */
                VRAM[IO_VDC_REG[MAWR].W * 2] = io.vdc_ratch;
                VRAM[IO_VDC_REG[MAWR].W * 2 + 1] = V;

                SPR_CACHE.Planes[IO_VDC_REG[MAWR].W / 16] = 0;
                SPR_CACHE.Sprites[IO_VDC_REG[MAWR].W / 64] = 0;

                IO_VDC_REG[MAWR].W += io.vdc_inc;

                /* vdc_ratch shouldn't be reset between writes */
                // io.vdc_ratch = 0;
                return;

            case VCR:
            case HSR:
            case VPR:
            case VDW:
                IO_VDC_REG_ACTIVE.B.h = V;
                io.vdc_mode_chg = 1;
                return;

            case LENR:          /* DMA transfert */

                IO_VDC_REG[LENR].B.h = V;

                {               // black-- 's code
                    int sourcecount = (IO_VDC_REG[DCR].W & 8) ? -1 : 1;
                    int destcount = (IO_VDC_REG[DCR].W & 4) ? -1 : 1;

                    int source = IO_VDC_REG[SOUR].W * 2;
                    int dest = IO_VDC_REG[DISTR].W * 2;

                    for (int  i = 0; i < (IO_VDC_REG[LENR].W + 1) * 2; i++) {
                        *(VRAM + dest) = *(VRAM + source);
                        dest += destcount;
                        source += sourcecount;
                    }

                    /*
                       IO_VDC_REG[SOUR].W = source;
                       IO_VDC_REG[DISTR].W = dest;
                     */
                    // Erich Kitzmuller fix follows
                    IO_VDC_REG[SOUR].W = source / 2;
                    IO_VDC_REG[DISTR].W = dest / 2;
                }

                IO_VDC_REG[LENR].W = 0xFFFF;

                memset(&SPR_CACHE, 0, sizeof(SPR_CACHE));

                /* TODO: check whether this flag can be ignored */
                io.vdc_status |= VDC_DMAfinish;
                return;

            case CR:            /* Auto increment size */
                /*
                    if (IO_VDC_REG[CR].B.h == V)
                    return;
                    */
                gfx_save_context(0);

                io.vdc_inc = incsize[(V >> 3) & 3];
                IO_VDC_REG[CR].B.h = V;
                return;

            case HDR:           /* Horizontal display end */
                /* TODO : well, maybe we should implement it */
                //io.screen_w = (io.VDC_ratch[HDR]+1)*8;
                //TRACE0("HDRh\n");
#if ENABLE_TRACING_DEEP_GFX
                TRACE("VDC[HDR].h = %d, ", V);
#endif
                return;

            case BYR:           /* Vertical screen offset */
                /*
                   if (IO_VDC_REG[BYR].B.h == (V & 1))
                   return;
                 */
                gfx_save_context(0);
                IO_VDC_REG[BYR].B.h = V & 1;
                ScrollYDiff = Scanline - 1;
                ScrollYDiff -= IO_VDC_REG[VPR].B.h + IO_VDC_REG[VPR].B.l;
#if ENABLE_TRACING_GFX
                if (ScrollYDiff < 0)
                    TRACE("ScrollYDiff went negative when substraction VPR.h/.l (%d,%d)\n",
                        IO_VDC_REG[VPR].B.h, IO_VDC_REG[VPR].B.l);
#endif

#if ENABLE_TRACING_DEEP_GFX
                TRACE("ScrollY = %d (h), ", ScrollY);
#endif
                return;

            case SATB:          /* DMA from VRAM to SATB */
                IO_VDC_REG[SATB].B.h = V;
                io.vdc_satb = 1;
                io.vdc_status &= ~VDC_SATBfinish;
                return;

            case BXR:           /* Horizontal screen offset */
                if (IO_VDC_REG[BXR].B.h != (V & 3)) {
                    gfx_save_context(0);
                    IO_VDC_REG[BXR].B.h = V & 3;
                }
                return;
            }

            IO_VDC_REG_ACTIVE.B.h = V;

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
            return;

            /* Frequency setting, 4 upper bits */
        case 3:
            io.PSG[io.psg_ch][3] = V & 15;
            return;

        case 4:
            io.PSG[io.psg_ch][4] = V;
#if ENABLE_TRACING_AUDIO
            if ((V & 0xC0) == 0x40)
                io.PSG[io.psg_ch][PSG_DATA_INDEX_REG] = 0;
#endif
            return;

            /* Set channel specific volume */
        case 5:
            io.PSG[io.psg_ch][5] = V;
            return;

            /* Put a value into the waveform or direct audio buffers */
        case 6:
            if (io.PSG[io.psg_ch][PSG_DDA_REG] & PSG_DDA_DIRECT_ACCESS) {
                io.psg_da_data[io.psg_ch][io.psg_da_index[io.psg_ch]] = V;
                io.psg_da_index[io.psg_ch] =
                    (io.psg_da_index[io.psg_ch] + 1) & 0x3FF;
                if (io.psg_da_count[io.psg_ch]++ > (PSG_DA_BUFSIZE - 1)) {
                        MESSAGE_INFO("Audio being put into the direct "
                                     "access buffer faster than it's being played.\n");
                    io.psg_da_count[io.psg_ch] = 0;
                }
            } else {
                io.PSG_WAVE[io.psg_ch][io.PSG[io.psg_ch][PSG_DATA_INDEX_REG]] = V;
                io.PSG[io.psg_ch][PSG_DATA_INDEX_REG] =
                    (io.PSG[io.psg_ch][PSG_DATA_INDEX_REG] + 1) & 0x1F;
            }
            return;

        case 7:
            io.PSG[io.psg_ch][7] = V;
            return;

        case 8:
            io.psg_lfo_freq = V;
            return;

        case 9:
            io.psg_lfo_ctrl = V;
            return;
        }
        break;

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
        io.joy_select = V & 1;
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

    case 0x1A00:                /* Arcade Card */
		MESSAGE_INFO("Arcade Card not supported : %d into 0x%04X\n", V, A);
        return;

    case 0x1800:                /* CD-ROM extention */
		MESSAGE_INFO("CD Emulation not implemented : %d 0x%04X\n", V, A);
        return;
    }

    MESSAGE_DEBUG("ignore I/O write %04x,%02x\tBase adress of port %X\nat PC = %04X\n",
            A, V, A & 0x1CC0, reg_pc);
}

IRAM_ATTR inline uchar
TimerInt()
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
    MESSAGE_DEBUG("Bank switching (MMR[%d] = %d)\n", P, V);

	MMR[P] = V;
	if (MemoryMapR[V] == IOAREA) {
		PageR[P] = IOAREA;
		PageW[P] = IOAREA;
	} else {
		PageR[P] = MemoryMapR[V] - P * 0x2000;
		PageW[P] = MemoryMapW[V] - P * 0x2000;
	}
}
