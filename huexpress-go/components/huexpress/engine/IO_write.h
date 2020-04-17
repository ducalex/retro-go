    //printf("w%04x,%02x ",A&0x3FFF,V);

    if ((A >= 0x800) && (A < 0x1800))   // We keep the io buffer value
        io.io_buffer = V;

#ifndef FINAL_RELEASE
    if ((A & 0x1F00) == 0x1A00)
        Log("AC Write %02x at %04x\n", V, A);
#endif

    switch (A & 0x1F00) {
    case 0x0000:                /* VDC */
        switch (A & 3) {
        case 0:
            IO_VDC_active_set(V & 31)
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

                    // (*init_normal_mode[video_driver]) ();
                    gfx_need_video_mode_change = 1;
                    {
                        uint32 x, y =
                            (WIDTH - io.screen_w) / 2 - 512 * WIDTH;
                        for (x = 0; x < 1024; x++) {
                            spr_init_pos[x] = y;
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

#ifndef FINAL_RELEASE
            if (io.vdc_reg > 19) {
                fprintf(stderr, "ignore write lo vdc%d,%02x\n", io.vdc_reg,
                        V);
            }
#endif

            return;
        case 3:
            switch (io.vdc_reg) {
            case VWR:           /* Write to mem */
                /* Writing to hi byte actually perform the action */
                VRAM[IO_VDC_00_MAWR.W * 2] = io.vdc_ratch;
                VRAM[IO_VDC_00_MAWR.W * 2 + 1] = V;

                vchange[IO_VDC_00_MAWR.W / 16] = 1;
                vchanges[IO_VDC_00_MAWR.W / 64] = 1;

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

                memset(vchange, 1, VRAMSIZE / 32);
                memset(vchanges, 1, VRAMSIZE / 128);


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

#ifndef FINAL_RELEASE
            if (io.vdc_reg > 19) {
                fprintf(stderr, "ignore write hi vdc%d,%02x\n", io.vdc_reg,
                        V);
            }
#endif

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
                        Pal[i] = c;
                } else if (n & 15)
                    Pal[n] = c;
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
                        Pal[i] = c;
                } else if (n & 15)
                    Pal[n] = c;
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
        {

            if ((A & 0x1AF0) == 0x1AE0) {
                switch (A & 15) {
                case 0:
                    io.ac_shift = (io.ac_shift & 0xffffff00) | V;
                    break;
                case 1:
                    io.ac_shift = (io.ac_shift & 0xffff00ff) | (V << 8);
                    break;
                case 2:
                    io.ac_shift = (io.ac_shift & 0xff00ffff) | (V << 16);
                    break;
                case 3:
                    io.ac_shift = (io.ac_shift & 0x00ffffff) | (V << 24);
                    break;
                case 4:
                    io.ac_shiftbits = V & 0x0f;
                    if (io.ac_shiftbits != 0) {
                        if (io.ac_shiftbits < 8) {
                            io.ac_shift <<= io.ac_shiftbits;
                        } else {
                            io.ac_shift >>= (16 - io.ac_shiftbits);
                        }
                    }
                default:
                    break;
                }
                return;
            } else {
                uchar ac_port = (A >> 4) & 3;
                switch (A & 15) {
                case 0:
                case 1:

                    if (io.ac_control[ac_port] & AC_USE_OFFSET) {
                        ac_extra_mem[((io.ac_base[ac_port]
                                       +
                                       io.
                                       ac_offset[ac_port]) & 0x1fffff)] =
                            V;
                    } else {
                        ac_extra_mem[((io.ac_base[ac_port]) & 0x1fffff)] =
                            V;
                    }

                    if (io.ac_control[ac_port] & AC_ENABLE_INC) {
                        if (io.ac_control[ac_port] & AC_INCREMENT_BASE)
                            io.ac_base[ac_port] =
                                (io.ac_base[ac_port] +
                                 io.ac_incr[ac_port]) & 0xffffff;
                        else
                            io.ac_offset[ac_port] =
                                (io.ac_offset[ac_port] +
                                 io.ac_incr[ac_port]) & 0xffffff;
                    }

                    return;
                case 2:
                    io.ac_base[ac_port] =
                        (io.ac_base[ac_port] & 0xffff00) | V;
                    return;
                case 3:
                    io.ac_base[ac_port] =
                        (io.ac_base[ac_port] & 0xff00ff) | (V << 8);
                    return;
                case 4:
                    io.ac_base[ac_port] =
                        (io.ac_base[ac_port] & 0x00ffff) | (V << 16);
                    return;
                case 5:
                    io.ac_offset[ac_port] =
                        (io.ac_offset[ac_port] & 0xff00) | V;
                    return;
                case 6:
                    io.ac_offset[ac_port] =
                        (io.ac_offset[ac_port] & 0x00ff) | (V << 8);
                    if (io.ac_control[ac_port] & (AC_ENABLE_OFFSET_BASE_6))
                        io.ac_base[ac_port] =
                            (io.ac_base[ac_port] +
                             io.ac_offset[ac_port]) & 0xffffff;
                    return;
                case 7:
                    io.ac_incr[ac_port] =
                        (io.ac_incr[ac_port] & 0xff00) | V;
                    return;
                case 8:
                    io.ac_incr[ac_port] =
                        (io.ac_incr[ac_port] & 0x00ff) | (V << 8);
                    return;
                case 9:
                    io.ac_control[ac_port] = V;
                    return;
                case 0xa:
                    if ((io.ac_control[ac_port]
                         & (AC_ENABLE_OFFSET_BASE_A |
                            AC_ENABLE_OFFSET_BASE_6))
                        == (AC_ENABLE_OFFSET_BASE_A |
                            AC_ENABLE_OFFSET_BASE_6))
                        io.ac_base[ac_port] =
                            (io.ac_base[ac_port] +
                             io.ac_offset[ac_port]) & 0xffffff;
                    return;
                default:
                    Log("Unknown AC write %d into 0x%04X\n", V, A);
                }

            }
        }
        break;

    case 0x1800:                /* CD-ROM extention */
#if defined(BSD_CD_HARDWARE_SUPPORT)
        pce_cd_handle_write_1800(A, V);
#else
        gpl_pce_cd_handle_write_1800(A, V);
#endif
        break;
    }
#ifndef FINAL_RELEASE
    fprintf(stderr,
            "ignore I/O write %04x,%02x\tBase adress of port %X\nat PC = %04X\n",
            A, V, A & 0x1CC0,
            reg_pc);
#endif
//          DebugDumpTrace(4, 1);
