// hard_pce.c - Memory/IO/Timer emulation
//
#include "hard_pce.h"
#include "pce.h"

// Global struct containing our emulated hardware status
PCE_t PCE;

// Memory Mapping
uint8_t *PageR[8];
uint8_t *PageW[8];
uint8_t *MemoryMapR[256];
uint8_t *MemoryMapW[256];

static inline void timer_run(void);

/**
  * Reset the hardware
  **/
void
pce_reset(void)
{
    memset(&RAM, 0, sizeof(RAM));
    memset(&BackupRAM, 0, sizeof(BackupRAM));
    memset(&VRAM, 0, sizeof(VRAM));
    memset(&SPRAM, 0, sizeof(SPRAM));
    memset(&Palette, 0, sizeof(Palette));
    memset(&io, 0, sizeof(io));
    memset(&NULLRAM, 0xFF, sizeof(NULLRAM));

    // Backup RAM header, some games check for this
    memcpy(&BackupRAM, "HUBM\x00\x88\x10\x80", 8);

    Cycles = 0;
    SF2 = 0;

    // Reset sound generator values
    for (int i = 0; i < PSG_CHANNELS; i++) {
        io.PSG[i][4] = 0x80;
    }

    // Reset memory banking
    pce_bank_set(7, 0x00);
    pce_bank_set(6, 0x05);
    pce_bank_set(5, 0x04);
    pce_bank_set(4, 0x03);
    pce_bank_set(3, 0x02);
    pce_bank_set(2, 0x01);
    pce_bank_set(1, 0xF8);
    pce_bank_set(0, 0xFF);

    // Reset CPU
    h6280_reset();
}


/**
  * Initialize the hardware
  **/
int
pce_init(void)
{
    for (int i = 0; i < 0xFF; i++) {
        MemoryMapR[i] = NULLRAM;
        MemoryMapW[i] = NULLRAM;
    }

    MemoryMapR[0xF7] = BackupRAM;
    MemoryMapW[0xF7] = BackupRAM;
    MemoryMapR[0xF8] = RAM;
    MemoryMapW[0xF8] = RAM;
    MemoryMapR[0xFF] = IOAREA;
    MemoryMapW[0xFF] = IOAREA;

    // pce_reset();

    return 0;
}


/**
  * Terminate the hardware
  **/
void
pce_term(void)
{
    if (ExtraRAM) free(ExtraRAM);
}


/**
  * Main emulation loop
  **/
void
pce_run(void)
{
    host.paused = 0;

    while (!host.paused) {
        osd_input_read();

        for (Scanline = 0; Scanline < 263; ++Scanline) {
            Cycles -= CYCLES_PER_LINE;
            h6280_run();
            timer_run();
            gfx_run();
        }

        osd_gfx_blit();
        osd_vsync();
    }
}


/**
 * Functions to access PCE hardware
 **/

static inline void
timer_run(void)
{
	io.timer_cycles_counter -= CYCLES_PER_LINE;

	// Trigger when it underflows
	if (io.timer_cycles_counter > CYCLES_PER_TIMER_TICK) {
		io.timer_cycles_counter += CYCLES_PER_TIMER_TICK;
		if (io.timer_running) {
			// Trigger when it underflows from 0
			if (io.timer_counter > 0x7F) {
				io.timer_counter = io.timer_reload;
				io.irq_status |= INT_TIMER;
			}
			io.timer_counter--;
		}
	}
}


static inline void
cart_write(uint16_t A, uint8_t V)
{
    TRACE_IO("Cart Write %02x at %04x\n", V, A);

    // SF2 Mapper
    if (A >= 0xFFF0 && ROM_SIZE >= 0xC0)
    {
        if (SF2 != (A & 3))
        {
            SF2 = A & 3;
            uint8_t *base = ROM_PTR + SF2 * (512 * 1024);
            for (int i = 0x40; i < 0x80; i++)
            {
                MemoryMapR[i] = base + i * 0x2000;
            }
            for (int i = 0; i < 8; i++)
            {
                if (MMR[i] >= 0x40 && MMR[i] < 0x80)
                    pce_bank_set(i, MMR[i]);
            }
        }
    }
}


IRAM_ATTR inline uint8_t
IO_read(uint16_t A)
{
    uint8_t ret = 0xFF; // Open Bus
    uint8_t ofs;

    // The last read value in 0800-017FF is read from the io buffer
    if (A >= 0x800 && A < 0x1800)
        ret = io.io_buffer;

    switch (A & 0x1F00) {
    case 0x0000:                /* VDC */
        switch (A & 3) {
        case 0:
            ret = io.vdc_status;
            io.vdc_status = 0;
            break;
        case 1:
            ret = 0;
            break;
        case 2:
            if (io.vdc_reg == VRR)              // // VRAM Read Register (LSB)
                ret = VRAM[IO_VDC_REG[MARR].W] & 0xFF;
            else
                ret = IO_VDC_REG_ACTIVE.B.l;
            break;
        case 3:
            if (io.vdc_reg == VRR) {            // VRAM Read Register (MSB)
                ret = VRAM[IO_VDC_REG[MARR].W] >> 8;
                IO_VDC_REG_INC(MARR);
            } else
                ret = IO_VDC_REG_ACTIVE.B.h;
            break;
        }
        break;

    case 0x0400:                /* VCE */
        switch (A & 7) {
        case 0: ret = 0xFF; break; // Write only
        case 1: ret = 0xFF; break; // Unused
        case 2: ret = 0xFF; break; // Write only
        case 3: ret = 0xFF; break; // Write only
        case 4: ret = io.VCE[io.vce_reg.W].B.l; break; // Color LSB (8 bit)
        case 5: ret = io.VCE[io.vce_reg.W++].B.h | 0xFE; break; // Color MSB (1 bit)
        case 6: ret = 0xFF; break; // Unused
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
            io.PSG[io.psg_ch][PSG_DATA_INDEX_REG] = (io.PSG[io.psg_ch][PSG_DATA_INDEX_REG] + 1) & 31;
            ret = io.PSG_WAVE[io.psg_ch][ofs];
            break;
        case 7: ret = io.PSG[io.psg_ch][7]; break;
        case 8: ret = io.psg_lfo_freq; break;
        case 9: ret = io.psg_lfo_ctrl; break;
        }
        break;

    case 0x0C00:                /* Timer */
        ret = io.timer_counter | (io.io_buffer & ~0x7F);
        break;

    case 0x1000:                /* Joypad */
        ret = io.JOY[io.joy_counter] ^ 0xff;
        if (io.joy_select & 1)
            ret >>= 4;
        else {
            ret &= 15;
            io.joy_counter = ((io.joy_counter + 1) % 5);
        }
        ret |= 0x30; // those 2 bits are always on, bit 6 = 0 (Jap), bit 7 = 0 (Attached cd)
        break;

    case 0x1400:                /* IRQ */
        switch (A & 3) {
        case 2:
            ret = io.irq_mask | (io.io_buffer & ~INT_MASK);
            break;
        case 3:
            ret = io.irq_status;
            io.irq_status = 0;
            break;
        }
        break;

    case 0x1A00:                // Arcade Card
        MESSAGE_INFO("Arcade Card not supported : 0x%04X\n", A);
        break;

    case 0x1800:                // CD-ROM extention
    case 0x18C0:                // Super System Card
        MESSAGE_INFO("CD Emulation not implemented : 0x%04X\n", A);
        break;
    }

    TRACE_IO("IO Read %02x at %04x\n", ret, A);

    // The last read value in 0800-017FF is saved in the io buffer
    if (A >= 0x800 && A < 0x1800)
        io.io_buffer = ret;

    return ret;
}


IRAM_ATTR inline void
IO_write(uint16_t A, uint8_t V)
{
    TRACE_IO("IO Write %02x at %04x\n", V, A);

    // The last write value in 0800-017FF is saved in the io buffer
    if (A >= 0x800 && A < 0x1800)
        io.io_buffer = V;

    switch (A & 0x1F00) {
    case 0x0000:                /* VDC */
        switch (A & 3) {
        case 0: // Latch
            io.vdc_reg = V & 31;
            return;

        case 1: // Not used
            return;

        case 2: // VDC data (LSB)
            switch (io.vdc_reg & 31) {
            case MAWR:                          // Memory Address Write Register
                break;

            case MARR:                          // Memory Address Read Register
                break;

            case VWR:                           // VRAM Write Register
                break;

            case vdc3:                          // Unused
                break;

            case vdc4:                          // Unused
                break;

            case CR:                            // Control Register
                if (IO_VDC_REG_ACTIVE.B.l != V)
                    gfx_latch_context(0);
                break;

            case RCR:                           // Raster Compare Register
                break;

            case BXR:
                /*
                   if (IO_VDC_REG[BXR].B.l == V)
                   return;
                 */
                gfx_latch_context(0);
                break;

            case BYR:                           // Vertical screen offset
                /*
                   if (IO_VDC_REG[BYR].B.l == V)
                   return;
                 */
                gfx_latch_context(0);
                ScrollYDiff = Scanline - 1 - IO_VDC_MINLINE;
                break;

            case MWR:                           // Memory Width Register
                break;

            case HSR:
                io.vdc_mode_chg = 1;
                break;

            case HDR:                           // Horizontal Definition
                V &= 0x7F;
                io.vdc_mode_chg = 1;
                break;

            case VPR:
            case VDW:
            case VCR:
                io.vdc_mode_chg = 1;
                break;

            case DCR:                           // DMA Control
                break;

            case SOUR:                          // DMA source address
                break;

            case DISTR:                         // DMA destination address
                break;

            case LENR:                          // DMA transfer from VRAM to VRAM
                break;

            case SATB:                          // DMA from VRAM to SATB
                break;
            }
            IO_VDC_REG_ACTIVE.B.l = V;
            TRACE_GFX2("VDC[%02x].l=0x%02x\n", io.vdc_reg, V);
            return;

        case 3: // VDC data (MSB)
            switch (io.vdc_reg & 31) {
            case MAWR:                          // Memory Address Write Register
                break;

            case MARR:                          // Memory Address Read Register
                break;

            case VWR:                           // VRAM Write Register
                VRAM[IO_VDC_REG[MAWR].W] = (V << 8) | IO_VDC_REG_ACTIVE.B.l;
                OBJ_CACHE_INVALIDATE(IO_VDC_REG[MAWR].W);
                IO_VDC_REG_INC(MAWR);
                break;

            case vdc3:                          // Unused
                break;

            case vdc4:                          // Unused
                break;

            case CR:                            // Control Register
                if (IO_VDC_REG_ACTIVE.B.h != V)
                    gfx_latch_context(0);
                break;

            case RCR:                           // Raster Compare Register
                IO_VDC_REG_ACTIVE.B.h &= 0x03;
                break;

            case BXR:                           // Horizontal screen offset
                V &= 3;
                if (IO_VDC_REG_ACTIVE.B.h != V) {
                    gfx_latch_context(0);
                }
                break;

            case BYR:                           // Vertical screen offset
                gfx_latch_context(0);
                ScrollYDiff = Scanline - 1 - IO_VDC_MINLINE;
                if (ScrollYDiff < 0) {
                    MESSAGE_DEBUG("ScrollYDiff went negative when substraction VPR.h/.l (%d,%d)\n",
                        IO_VDC_REG[VPR].B.h, IO_VDC_REG[VPR].B.l);
                }
                break;

            case MWR:                           // Memory Width Register
                break;

            case HSR:
                io.vdc_mode_chg = 1;
                break;

            case HDR:                           // Horizontal Definition
                TRACE_GFX2("VDC[HDR].h = %d\n", V);
                break;

            case VPR:
            case VDW:
            case VCR:
                io.vdc_mode_chg = 1;
                break;

            case DCR:                           // DMA Control
                break;

            case SOUR:                          // DMA source address
                break;

            case DISTR:                         // DMA destination address
                break;

            case LENR:                          // DMA transfer from VRAM to VRAM
                IO_VDC_REG[LENR].B.h = V;

                int src_inc = (IO_VDC_REG[DCR].W & 8) ? -1 : 1;
                int dst_inc = (IO_VDC_REG[DCR].W & 4) ? -1 : 1;

                while (IO_VDC_REG[LENR].W != 0xFFFF) {
                    VRAM[IO_VDC_REG[DISTR].W] = VRAM[IO_VDC_REG[SOUR].W];
                    OBJ_CACHE_INVALIDATE(IO_VDC_REG[DISTR].W);
                    IO_VDC_REG[SOUR].W += src_inc;
                    IO_VDC_REG[DISTR].W += dst_inc;
                    IO_VDC_REG[LENR].W -= 1;
                }

                gfx_irq(VDC_STAT_DV);
                return;

            case SATB:                          // DMA from VRAM to SATB
                io.vdc_satb = DMA_TRANSFER_PENDING;
                break;
            }
            IO_VDC_REG_ACTIVE.B.h = V;
            TRACE_GFX2("VDC[%02x].h=0x%02x\n", io.vdc_reg, V);
            return;
        }
        break;

    case 0x0400:                /* VCE */
        switch (A & 7) {
        case 0:                                 // VCE control
            return;

        case 1:                                 // Not used
            return;

        case 2:                                 // Color table address (LSB)
            io.vce_reg.B.l = V;
            return;

        case 3:                                 // Color table address (MSB)
            io.vce_reg.B.h = V & 1;
            return;

        case 4:                                 // Color table data (LSB)
            io.VCE[io.vce_reg.W].B.l = V;
            {
                uint16_t n = io.vce_reg.W;
                uint8_t c = io.VCE[n].W >> 1;
                if (n == 0) {
                    for (int i = 0; i < 256; i += 16)
                        Palette[i] = c;
                } else if (n & 15)
                    Palette[n] = c;
            }
            return;

        case 5:                                 // Color table data (MSB)
            io.VCE[io.vce_reg.W].B.h = V;
            {
                uint16_t n = io.vce_reg.W;
                uint8_t c = io.VCE[n].W >> 1;
                if (n == 0) {
                    for (int i = 0; i < 256; i += 16)
                        Palette[i] = c;
                } else if (n & 15)
                    Palette[n] = c;
            }
            io.vce_reg.W = (io.vce_reg.W + 1) & 0x1FF;
            return;

        case 6:                                 // Not used
            return;

        case 7:                                 // Not used
            return;
        }
        break;

    case 0x0800:                /* PSG */
        switch (A & 15) {
        case 0:                                 // Select PSG channel
            io.psg_ch = V & 7;
            return;

        case 1:                                 // Select global volume
            io.psg_volume = V;
            return;

        case 2:                                 // Frequency setting, 8 lower bits
            io.PSG[io.psg_ch][2] = V;
            return;

        case 3:                                 // Frequency setting, 4 upper bits
            io.PSG[io.psg_ch][3] = V & 15;
            return;

        case 4:
            io.PSG[io.psg_ch][4] = V;
            // #if DEBUG_AUDIO
            //             if ((V & 0xC0) == 0x40)
            //                 io.PSG[io.psg_ch][PSG_DATA_INDEX_REG] = 0;
            // #endif
            return;

        case 5:                                 // Set channel specific volume
            io.PSG[io.psg_ch][5] = V;
            return;

        case 6:                                 // Put a value into the waveform or direct audio buffers
            if (io.PSG[io.psg_ch][PSG_DDA_REG] & PSG_DDA_DIRECT_ACCESS) {
                io.psg_da_data[io.psg_ch][io.psg_da_index[io.psg_ch]] = V;
                io.psg_da_index[io.psg_ch] = (io.psg_da_index[io.psg_ch] + 1) & 0x3FF;
                if (io.psg_da_count[io.psg_ch]++ > (PSG_DA_BUFSIZE - 1)) {
                    MESSAGE_DEBUG("Audio being put into the direct "
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

    case 0x0C00:                /* Timer */
        switch (A & 1) {
        case 0:
            io.timer_reload = (V & 0x7F); // + 1;
            return;
        case 1:
            V &= 1;
            if (V && !io.timer_running)
                io.timer_counter = io.timer_reload;
            io.timer_running = V;
            return;
        }
        break;

    case 0x1000:                /* Joypad */
        io.joy_select = V & 1;
        if (V & 2)
            io.joy_counter = 0;
        return;

    case 0x1400:                /* IRQ */
        switch (A & 3) {
        case 2:
            io.irq_mask = V & INT_MASK;
            break;
        case 3:
            io.irq_status &= ~INT_TIMER;
            break;
        }
        break;

    case 0x1A00:                /* Arcade Card */
        MESSAGE_INFO("Arcade Card not supported : %d into 0x%04X\n", V, A);
        return;

    case 0x1800:                /* CD-ROM extention */
        MESSAGE_INFO("CD Emulation not implemented : %d 0x%04X\n", V, A);
        return;

    case 0x1F00:                /* Street Fighter 2 Mapper */
        cart_write(A, V);
        return;
    }

    MESSAGE_DEBUG("ignored I/O write: %04x,%02x at PC = %04X\n", A, V, reg_pc);
}
