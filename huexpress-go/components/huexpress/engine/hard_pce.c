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
    memset(&PCE.RAM, 0, sizeof(PCE.RAM));
    memset(&PCE.VRAM, 0, sizeof(PCE.VRAM));
    memset(&PCE.SPRAM, 0, sizeof(PCE.SPRAM));
    memset(&PCE.Palette, 0, sizeof(PCE.Palette));
    memset(&PCE.VCE, 0, sizeof(PCE.VCE));
    memset(&PCE.VDC, 0, sizeof(PCE.VDC));
    memset(&PCE.PSG, 0, sizeof(PCE.PSG));
    memset(&PCE.Timer, 0, sizeof(PCE.Timer));

    memset(&PCE.NULLRAM, 0xFF, sizeof(PCE.NULLRAM));

    PCE.irq_mask = PCE.irq_status = 0;
    PCE.SF2 = 0;

    Cycles = 0;

    // Reset sound generator values
    for (int i = 0; i < PSG_CHANNELS; i++) {
        PCE.PSG.chan[i].control = 0x80;
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
        MemoryMapR[i] = PCE.NULLRAM;
        MemoryMapW[i] = PCE.NULLRAM;
    }

    MemoryMapR[0xF8] = PCE.RAM;
    MemoryMapW[0xF8] = PCE.RAM;
    MemoryMapR[0xFF] = PCE.IOAREA;
    MemoryMapW[0xFF] = PCE.IOAREA;

    // pce_reset();

    return 0;
}


/**
  * Terminate the hardware
  **/
void
pce_term(void)
{
    if (PCE.ExRAM) free(PCE.ExRAM);
    if (PCE.ROM) free(PCE.ROM);
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
            PCE.MaxCycles += CYCLES_PER_LINE;
            h6280_run();
            timer_run();
            gfx_run();
        }

        osd_gfx_blit();
        osd_vsync();

        // Prevent Overflowing
        // int trim = MIN(Cycles, PCE.MaxCycles);
        // PCE.MaxCycles -= trim;
        // Cycles -= trim;
    }
}


/**
 * Functions to access PCE hardware
 **/

static inline void
timer_run(void)
{
	PCE.Timer.cycles_counter -= CYCLES_PER_LINE;

	// Trigger when it underflows
	if (PCE.Timer.cycles_counter > CYCLES_PER_TIMER_TICK) {
		PCE.Timer.cycles_counter += CYCLES_PER_TIMER_TICK;
		if (PCE.Timer.running) {
			// Trigger when it underflows from 0
			if (PCE.Timer.counter > 0x7F) {
				PCE.Timer.counter = PCE.Timer.reload;
				PCE.irq_status |= INT_TIMER;
			}
			PCE.Timer.counter--;
		}
	}
}


static inline void
cart_write(uint16_t A, uint8_t V)
{
    TRACE_IO("Cart Write %02x at %04x\n", V, A);

    // SF2 Mapper
    if (A >= 0xFFF0 && PCE.ROM_SIZE >= 0xC0)
    {
        if (PCE.SF2 != (A & 3))
        {
            PCE.SF2 = A & 3;
            uint8_t *base = PCE.ROM_DATA + PCE.SF2 * (512 * 1024);
            for (int i = 0x40; i < 0x80; i++)
            {
                MemoryMapR[i] = base + i * 0x2000;
            }
            for (int i = 0; i < 8; i++)
            {
                if (PCE.MMR[i] >= 0x40 && PCE.MMR[i] < 0x80)
                    pce_bank_set(i, PCE.MMR[i]);
            }
        }
    }
}


IRAM_ATTR inline uint8_t
IO_read(uint16_t A)
{
    uint8_t ret = 0xFF; // Open Bus
    uint8_t index;

    // The last read value in 0800-017FF is read from the io buffer
    if (A >= 0x800 && A < 0x1800)
        ret = PCE.io_buffer;

    switch (A & 0x1F00) {
    case 0x0000:                /* VDC */
        switch (A & 3) {
        case 0:
            ret = PCE.VDC.status;
            PCE.VDC.status = 0;
            break;
        case 1:
            ret = 0;
            break;
        case 2:
            if (PCE.VDC.reg == VRR)              // // VRAM Read Register (LSB)
                ret = PCE.VRAM[IO_VDC_REG[MARR].W] & 0xFF;
            else
                ret = IO_VDC_REG_ACTIVE.B.l;
            break;
        case 3:
            if (PCE.VDC.reg == VRR) {            // VRAM Read Register (MSB)
                ret = PCE.VRAM[IO_VDC_REG[MARR].W] >> 8;
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
        case 4: ret = PCE.VCE.regs[PCE.VCE.reg.W].B.l; break; // Color LSB (8 bit)
        case 5: ret = PCE.VCE.regs[PCE.VCE.reg.W++].B.h | 0xFE; break; // Color MSB (1 bit)
        case 6: ret = 0xFF; break; // Unused
        }
        break;

    case 0x0800:                /* PSG */
        switch (A & 15) {
        case 0: ret = PCE.PSG.ch; break;
        case 1: ret = PCE.PSG.volume; break;
        case 2: ret = PCE.PSG.chan[PCE.PSG.ch].freq_lsb; break;
        case 3: ret = PCE.PSG.chan[PCE.PSG.ch].freq_msb; break;
        case 4: ret = PCE.PSG.chan[PCE.PSG.ch].control; break;
        case 5: ret = PCE.PSG.chan[PCE.PSG.ch].balance; break;
        case 6: ret = PCE.PSG.chan[PCE.PSG.ch].wave_index; break;
        case 7: ret = PCE.PSG.chan[PCE.PSG.ch].noise_ctrl; break;
        case 8: ret = PCE.PSG.lfo_freq; break;
        case 9: ret = PCE.PSG.lfo_ctrl; break;
        }
        break;

    case 0x0C00:                /* Timer */
        ret = PCE.Timer.counter | (PCE.io_buffer & ~0x7F);
        break;

    case 0x1000:                /* Joypad */
        ret = PCE.Joypad.regs[PCE.Joypad.counter] ^ 0xff;
        if (PCE.Joypad.nibble & 1)
            ret >>= 4;
        else {
            ret &= 15;
            PCE.Joypad.counter = ((PCE.Joypad.counter + 1) % 5);
        }
        ret |= 0x30; // those 2 bits are always on, bit 6 = 0 (Jap), bit 7 = 0 (Attached cd)
        break;

    case 0x1400:                /* IRQ */
        switch (A & 3) {
        case 2:
            ret = PCE.irq_mask | (PCE.io_buffer & ~INT_MASK);
            break;
        case 3:
            ret = PCE.irq_status;
            PCE.irq_status = 0;
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
        PCE.io_buffer = ret;

    return ret;
}


IRAM_ATTR inline void
IO_write(uint16_t A, uint8_t V)
{
    TRACE_IO("IO Write %02x at %04x\n", V, A);

    // The last write value in 0800-017FF is saved in the io buffer
    if (A >= 0x800 && A < 0x1800)
        PCE.io_buffer = V;

    switch (A & 0x1F00) {
    case 0x0000:                /* VDC */
        switch (A & 3) {
        case 0: // Latch
            PCE.VDC.reg = V & 31;
            return;

        case 1: // Not used
            return;

        case 2: // VDC data (LSB)
            switch (PCE.VDC.reg & 31) {
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
                PCE.VDC.mode_chg = 1;
                break;

            case HDR:                           // Horizontal Definition
                V &= 0x7F;
                PCE.VDC.mode_chg = 1;
                break;

            case VPR:
            case VDW:
            case VCR:
                PCE.VDC.mode_chg = 1;
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
            TRACE_GFX2("VDC[%02x].l=0x%02x\n", PCE.VDC.reg, V);
            return;

        case 3: // VDC data (MSB)
            switch (PCE.VDC.reg & 31) {
            case MAWR:                          // Memory Address Write Register
                break;

            case MARR:                          // Memory Address Read Register
                break;

            case VWR:                           // VRAM Write Register
                PCE.VRAM[IO_VDC_REG[MAWR].W] = (V << 8) | IO_VDC_REG_ACTIVE.B.l;
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
                PCE.VDC.mode_chg = 1;
                break;

            case HDR:                           // Horizontal Definition
                TRACE_GFX2("VDC[HDR].h = %d\n", V);
                break;

            case VPR:
            case VDW:
            case VCR:
                PCE.VDC.mode_chg = 1;
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
                    PCE.VRAM[IO_VDC_REG[DISTR].W] = PCE.VRAM[IO_VDC_REG[SOUR].W];
                    OBJ_CACHE_INVALIDATE(IO_VDC_REG[DISTR].W);
                    IO_VDC_REG[SOUR].W += src_inc;
                    IO_VDC_REG[DISTR].W += dst_inc;
                    IO_VDC_REG[LENR].W -= 1;
                }

                gfx_irq(VDC_STAT_DV);
                return;

            case SATB:                          // DMA from VRAM to SATB
                PCE.VDC.satb = DMA_TRANSFER_PENDING;
                break;
            }
            IO_VDC_REG_ACTIVE.B.h = V;
            TRACE_GFX2("VDC[%02x].h=0x%02x\n", PCE.VDC.reg, V);
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
            PCE.VCE.reg.B.l = V;
            return;

        case 3:                                 // Color table address (MSB)
            PCE.VCE.reg.B.h = V & 1;
            return;

        case 4:                                 // Color table data (LSB)
            PCE.VCE.regs[PCE.VCE.reg.W].B.l = V;
            {
                uint16_t n = PCE.VCE.reg.W;
                uint8_t c = PCE.VCE.regs[n].W >> 1;
                if (n == 0) {
                    for (int i = 0; i < 256; i += 16)
                        PCE.Palette[i] = c;
                } else if (n & 15)
                    PCE.Palette[n] = c;
            }
            return;

        case 5:                                 // Color table data (MSB)
            PCE.VCE.regs[PCE.VCE.reg.W].B.h = V;
            {
                uint16_t n = PCE.VCE.reg.W;
                uint8_t c = PCE.VCE.regs[n].W >> 1;
                if (n == 0) {
                    for (int i = 0; i < 256; i += 16)
                        PCE.Palette[i] = c;
                } else if (n & 15)
                    PCE.Palette[n] = c;
            }
            PCE.VCE.reg.W = (PCE.VCE.reg.W + 1) & 0x1FF;
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
            PCE.PSG.ch = MIN(V & 7, 5);
            return;

        case 1:                                 // Select global volume
            PCE.PSG.volume = V;
            return;

        case 2:                                 // Frequency setting, 8 lower bits
            PCE.PSG.chan[PCE.PSG.ch].freq_lsb = V;
            return;

        case 3:                                 // Frequency setting, 4 upper bits
            PCE.PSG.chan[PCE.PSG.ch].freq_msb = V & 0xF;
            return;

        case 4:
            if ((V & 0xC0) == (PSG_DDA_ENABLE)) {
                PCE.PSG.chan[PCE.PSG.ch].wave_index = 0; // Reset wave index pointer
            }

            PCE.PSG.chan[PCE.PSG.ch].control = V;
            return;

        case 5:                                 // Set channel specific volume
            PCE.PSG.chan[PCE.PSG.ch].balance = V;
            return;

        case 6:                                 // Put a value into the waveform or direct audio buffers
            switch (PCE.PSG.chan[PCE.PSG.ch].control & 0xC0)
            {
            case 0: // Write to the wave buffer and increment the counter
                PCE.PSG.chan[PCE.PSG.ch].wave_data[PCE.PSG.chan[PCE.PSG.ch].wave_index] = V & 0x1F;
                PCE.PSG.chan[PCE.PSG.ch].wave_index++; // Inc pointer
                PCE.PSG.chan[PCE.PSG.ch].wave_index &= 0x1F; // Wrap at 32
                break;
            case PSG_CHAN_ENABLE|PSG_DDA_ENABLE: // Update DDA sample
                PCE.PSG.chan[PCE.PSG.ch].dda_data[PCE.PSG.chan[PCE.PSG.ch].dda_index] = V & 0x1F;
                PCE.PSG.chan[PCE.PSG.ch].dda_count++;
                PCE.PSG.chan[PCE.PSG.ch].dda_index++;
                PCE.PSG.chan[PCE.PSG.ch].dda_index &= 0x7F;
                break;
            }
            return;

        case 7:
            PCE.PSG.chan[PCE.PSG.ch].noise_ctrl = V;
            return;

        case 8:
            PCE.PSG.lfo_freq = V;
            return;

        case 9:
            PCE.PSG.lfo_ctrl = V;
            return;
        }
        break;

    case 0x0C00:                /* Timer */
        switch (A & 1) {
        case 0:
            PCE.Timer.reload = (V & 0x7F); // + 1;
            return;
        case 1:
            V &= 1;
            if (V && !PCE.Timer.running)
                PCE.Timer.counter = PCE.Timer.reload;
            PCE.Timer.running = V;
            return;
        }
        break;

    case 0x1000:                /* Joypad */
        PCE.Joypad.nibble = V & 1;
        if (V & 2)
            PCE.Joypad.counter = 0;
        return;

    case 0x1400:                /* IRQ */
        switch (A & 3) {
        case 2:
            PCE.irq_mask = V & INT_MASK;
            break;
        case 3:
            PCE.irq_status &= ~INT_TIMER;
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
