// hard_pce.c - Memory/IO/Timer emulation
//
#include "hard_pce.h"
#include "pce.h"

/**
  * Variables declaration
  * cf explanations in the header file
  **/

PCE_t PCE;

// We used to point to VRAM to save memory but some games
// seem to write to that area for some reason...
uchar NULLRAM[0x2004];
uchar *IOAREA, *TRAPRAM;

// Memory Mapping
uchar *PageR[8];
uchar *PageW[8];
uchar *MemoryMapR[256];
uchar *MemoryMapW[256];


/**
  * Reset the hardware
  **/
void
hard_reset(void)
{
    memset(&RAM, 0, sizeof(RAM));
    memset(&BackupRAM, 0, sizeof(BackupRAM));
    memset(&VRAM, 0, sizeof(VRAM));
    memset(&SPRAM, 0, sizeof(SPRAM));
    memset(&Palette, 0, sizeof(Palette));
    memset(&io, 0, sizeof(io));

    // Backup RAM header, some games check for this
    memcpy(&BackupRAM, "HUBM\x00\x88\x10\x80", 8);

    Scanline = 0;
    Cycles = 0;
    TotalCycles = 0;
    PrevTotalCycles = 0;
    SF2 = 0;

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
    BankSet(7, 0x00);
    BankSet(6, 0x05);
    BankSet(5, 0x04);
    BankSet(4, 0x03);
    BankSet(3, 0x02);
    BankSet(2, 0x01);
    BankSet(1, 0xF8);
    BankSet(0, 0xFF);

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
    TRAPRAM = NULLRAM + 0; // &VRAM[0x8000];
    IOAREA  = NULLRAM + 4; // &VRAM[0xA000];

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


static inline void
Cart_write(uint16 A, uchar V)
{
    TRACE_IO("Cart Write %02x at %04x\n", V, A);

    // SF2 Mapper
    if (A >= 0xFFF0 && ROM_SIZE >= 0xC0)
    {
        if (SF2 != (A & 3))
        {
            SF2 = A & 3;
            uchar *base = ROM_PTR + SF2 * (512 * 1024);
            for (int i = 0x40; i < 0x80; i++)
            {
                MemoryMapR[i] = base + i * 0x2000;
            }
            for (int i = 0; i < 8; i++)
            {
                if (MMR[i] >= 0x40 && MMR[i] < 0x80)
                    BankSet(i, MMR[i]);
            }
        }
    }
}



IRAM_ATTR inline uchar
IO_read(uint16 A)
{
    uchar ret = 0xFF; // Open Bus
    uchar ofs;

    switch (A & 0x1FC0) {
    case 0x0000:                /* VDC */
        switch (A & 3) {
        case 0:
            ret = io.vdc_status;
            io.vdc_status = 0;  //&=VDC_InVBlank;//&=~VDC_BSY;
            break;
        case 1:
            ret = 0;
            break;
        case 2:
            if (io.vdc_reg == VRR)
                ret = VRAM[IO_VDC_REG[MARR].W * 2];
            else
                ret = IO_VDC_REG_ACTIVE.B.l;
            break;
        case 3:
            if (io.vdc_reg == VRR) {
                ret = VRAM[IO_VDC_REG[MARR].W * 2 + 1];
                IO_VDC_REG[MARR].W += io.vdc_inc;
            } else
                ret = IO_VDC_REG_ACTIVE.B.h;
            break;
        }
        break;

    case 0x0400:                /* VCE */
        switch (A & 7) {
        case 4: ret = io.VCE[io.vce_reg.W].B.l;   break;
        case 5: ret = io.VCE[io.vce_reg.W++].B.h; break;
        }
        break;

    case 0x0800:                /* PSG */
        switch (A & 15) {
        case 0: ret = io.psg_ch; break;
        case 1: ret = io.psg_volume; break;
        case 2: ret = io.PSG[io.psg_ch][2]; break;
        case 3: ret = io.PSG[io.psg_ch][3]; break;
        case 4: ret = io.PSG[io.psg_ch][4]; break;
        case 5: ret = io.PSG[io.psg_ch][5]; break;
        case 6:
            ofs = io.PSG[io.psg_ch][PSG_DATA_INDEX_REG];
            io.PSG[io.psg_ch][PSG_DATA_INDEX_REG] = (uchar) ((io.PSG[io.psg_ch][PSG_DATA_INDEX_REG] + 1) & 31);
            ret = io.PSG_WAVE[io.psg_ch][ofs];
            break;
        case 7: ret = io.PSG[io.psg_ch][7]; break;
        case 8: ret = io.psg_lfo_freq; break;
        case 9: ret = io.psg_lfo_ctrl; break;
        }
        break;

    case 0x0c00:                /* timer */
        ret = io.timer_counter;
        break;

    case 0x1000:                /* joypad */
        ret = io.JOY[io.joy_counter] ^ 0xff;
        if (io.joy_select & 1)
            ret >>= 4;
        else {
            ret &= 15;
            io.joy_counter = (uchar) ((io.joy_counter + 1) % 5);
        }
        ret |= 0x30; // those 2 bits are always on, bit 6 = 0 (Jap), bit 7 = 0 (Attached cd)
        break;

    case 0x1400:                /* IRQ */
        switch (A & 15) {
        case 2:
            ret = io.irq_mask;
            break;
        case 3:
            ret = io.irq_status;
            io.irq_status = 0;
            break;
        }
        break;

    case 0x18C0:                // Memory management ?
        switch (A & 15) {
        case 1:
        case 5:
            ret = 0xAA;
            break;
        case 2:
        case 6:
            ret = 0x55;
            break;
        case 3:
        case 7:
            ret = 0x03;
            break;
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

    TRACE_IO("IO Read %02x at %04x\n", ret, A);

	if ((A < 0x800) || (A >= 0x1800))
		return ret;

	io.io_buffer = ret | (io.io_buffer & ~return_value_mask(A));

	return io.io_buffer;
}


IRAM_ATTR inline void
IO_write(uint16 A, uchar V)
{
    TRACE_IO("IO Write %02x at %04x\n", V, A);

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
            TRACE_GFX2("VDC[%02x]=0x%02x\n", io.vdc_reg, V);
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
                TRACE_GFX2("ScrollY = %d (l)\n", ScrollY);
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
                IO_VDC_REG_ACTIVE.B.l = V;
                io.vdc_mode_chg = 1;
                break;
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

                gfx_clear_cache();

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
                TRACE_GFX2("VDC[HDR].h = %d\n", V);
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
                if (ScrollYDiff < 0)
                    TRACE_GFX("ScrollYDiff went negative when substraction VPR.h/.l (%d,%d)\n",
                        IO_VDC_REG[VPR].B.h, IO_VDC_REG[VPR].B.l);

                TRACE_GFX2("ScrollY = %d (h)\n", ScrollY);
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
// #if DEBUG_AUDIO
//             if ((V & 0xC0) == 0x40)
//                 io.PSG[io.psg_ch][PSG_DATA_INDEX_REG] = 0;
// #endif
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

    case 0x1F00:                /* Street Fighter 2 Mapper */
        Cart_write(A, V); // This is a hack, we shouldn't go through IO_Write at all...
        return;
    }

    MESSAGE_DEBUG("ignore I/O write %04x,%02x\tBase address of port %X\nat PC = %04X\n",
            A, V, A & 0x1CC0, reg_pc);
}
