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
