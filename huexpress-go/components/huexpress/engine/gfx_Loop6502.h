    ODROID_DEBUG_PERF_START2(debug_perf_part_loop6502)
    uchar return_value = INT_NONE;

    io.vdc_status &= ~(VDC_RasHit | VDC_SATBfinish);

    // Count dma delay

    if (satb_dma_counter > 0) {
        // A dma is in progress
        satb_dma_counter--;

        if (!satb_dma_counter) {
            // dma has just finished
            if (SATBIntON) {
                io.vdc_status |= VDC_SATBfinish;
                return_value = INT_IRQ;
            }
        }
    }
    // Test raster hit
    if (RasHitON) {
        if (((IO_VDC_06_RCR.W & 0x3FF) >= 0x40)
            && ((IO_VDC_06_RCR.W & 0x3FF) <= 0x146)) {
            uint16 temp_rcr = (uint16) ((IO_VDC_06_RCR.W & 0x3FF) - 0x40);

            if (scanline
                == (temp_rcr + IO_VDC_0C_VPR.B.l + IO_VDC_0C_VPR.B.h) % 263) {
                // printf("\n---------------------\nRASTER HIT (%d)\n----------------------\n", scanline);
                io.vdc_status |= VDC_RasHit;
                return_value = INT_IRQ;
            }
        } else {
            // printf("Raster counter out of bounds (%d)\n", IO_VDC_06_RCR.W);
        }
    }
    //  else
    //      printf("Raster disabled\n");

    // Rendering of tiles / sprites

    if (scanline < 14) {
        gfx_need_redraw = 0;
    } else if (scanline < 14 + 242) {
        if (scanline == 14) {
            last_display_counter = 0;
            display_counter = 0;
            ScrollYDiff = 0;
            oldScrollYDiff = 0;

            // Signal that we've left the VBlank area
            io.vdc_status &= ~VDC_InVBlank;

            TRACE("Cleaning VBlank bit from vdc_status (now, 0x%02x)",
                io.vdc_status);
        }

        if (scanline == io.vdc_min_display) {
            gfx_need_redraw = 0;

            save_gfx_context(0);

            TRACE("GFX: FORCED SAVE OF GFX CONTEXT\n");
        }

        if ((scanline >= io.vdc_min_display)
            && (scanline <= io.vdc_max_display)) {
            if (gfx_need_redraw) {
                // && scanline > io.vdc_min_display)
                // We got render things before being on the second line

#ifdef MY_INLINE_GFX
                #include "gfx_render_lines.h"
#else
                render_lines(last_display_counter, display_counter);
#endif
#ifdef MY_VIDEO_MODE_SCANLINES
                last_display_counter = display_counter + 1;
            } else {
                display_counter++;
                if (display_counter-last_display_counter>=40)
                {
                save_gfx_context(0);
#ifdef MY_INLINE_GFX
                #include "gfx_render_lines.h"
#else
                render_lines(last_display_counter, display_counter);
#endif
                last_display_counter = display_counter;
                }
            }
#else
                last_display_counter = display_counter;
            }
            display_counter++;
#endif
        }
    } else if (scanline < 14 + 242 + 4) {
        if (scanline == 14 + 242) {
            save_gfx_context(0);

#ifdef MY_INLINE_GFX
            #include "gfx_render_lines.h"
#else
            render_lines(last_display_counter, display_counter);
#endif
#if 0
            if (video_dump_flag) {
                if (video_dump_countdown)
                    video_dump_countdown--;
                else {
                    dump_video_frame();
                    video_dump_countdown = 3;
                }
            }
#endif
            if (gfx_need_video_mode_change) {
                gfx_need_video_mode_change = 0;
                change_pce_screen_height();
            }

            osd_keyboard();
            if (!UCount)
            {
                ODROID_DEBUG_PERF_START2(debug_perf_refreshscreen)
                RefreshScreen();
                ODROID_DEBUG_PERF_INCR2(debug_perf_refreshscreen, ODROID_DEBUG_PERF_SPRITE_RefreshScreen)
            }

            /*@-preproc */
#warning "place this better"
            /*@=preproc */
#ifdef MY_INLINE_SPRITE_CheckSprites
{
    int i, x0, y0, w0, h0, x, y, w, h;
    SPR *spr;

    spr = (SPR *) SPRAM;
    x0 = spr->x;
    y0 = spr->y;
    w0 = (((spr->atr >> 8) & 1) + 1) * 16;
    h0 = (((spr->atr >> 12) & 3) + 1) * 16;
    spr++;
    for (i = 1; i < 64; i++, spr++) {
        x = spr->x;
        y = spr->y;
        w = (((spr->atr >> 8) & 1) + 1) * 16;
        h = (((spr->atr >> 12) & 3) + 1) * 16;
        if ((x < x0 + w0) && (x + w > x0) && (y < y0 + h0) && (y + h > y0))
        {
            io.vdc_status |= VDC_SpHit;
            goto jump_CheckSprites;
        }
    }
    io.vdc_status &= ~VDC_SpHit;
}
jump_CheckSprites:
#else
            if (CheckSprites())
                io.vdc_status |= VDC_SpHit;
            else
                io.vdc_status &= ~VDC_SpHit;
#endif
            if (!UCount) {
#if defined(ENABLE_NETPLAY)
                if (option.want_netplay != INTERNET_PROTOCOL) {
                    /* When in internet protocol mode, it's the server which is in charge of throlling */
                    wait_next_vsync();
                }
#else
                wait_next_vsync();
#endif                          /* NETPLAY_ENABLE */
                UCount = UPeriod;
            } else {
                UCount--;
            }

            /* VRAM to SATB DMA */
            if (io.vdc_satb == 1 || IO_VDC_0F_DCR.W & 0x0010) {
#if defined(WORDS_BIGENDIAN)
                swab(VRAM + IO_VDC_13_SATB.W * 2, SPRAM, 64 * 8);
#else
                memcpy(SPRAM, VRAM + IO_VDC_13_SATB.W * 2, 64 * 8);
#endif
                io.vdc_satb = 1;
                io.vdc_status &= ~VDC_SATBfinish;

                // Mark satb dma end interuption to happen in 4 scanlines
                satb_dma_counter = 4;
            }

            if (return_value == INT_IRQ)
                io.vdc_pendvsync = 1;
            else {
                if (VBlankON) {
                    io.vdc_status |= VDC_InVBlank;
                    return_value = INT_IRQ;
                }
            }
        }

    } else {
        //Three last lines of ntsc scanlining
    }

    // Incrementing the scanline

    scanline++;

    if (scanline >= 263)
        scanline = 0;

    if ((return_value != INT_IRQ) && io.vdc_pendvsync) {
        io.vdc_status |= VDC_InVBlank;
        return_value = INT_IRQ;
        io.vdc_pendvsync = 0;
    }

    if (return_value == INT_IRQ) {
        if (!(io.irq_mask & IRQ1)) {
            io.irq_status |= IRQ1;
            I = return_value;
        } else I = INT_NONE;
    } else I = INT_NONE;
    ODROID_DEBUG_PERF_INCR2(debug_perf_part_loop6502, ODROID_DEBUG_PERF_LOOP6502)
