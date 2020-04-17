// int Y1, int Y2, uchar bg

    ODROID_DEBUG_PERF_START2(debug_perf_part1)
    int n;
    SPR *spr;
    
    /* TEST */
    Y2++;

    spr = (SPR *) SPRAM + 63;

    if (bg == 0)
        sprite_usespbg = 0;

    for (n = 0; n < 64; n++, spr--) {
        int x, y, no, atr, inc, cgx, cgy;
        int pos;
        int h, t, i, j;
        int y_sum;
        int spbg;
        atr = spr->atr;
        spbg = (atr >> 7) & 1;
        if (spbg != bg)
            continue;
        y = (spr->y & 1023) - 64;
        x = (spr->x & 1023) - 32;

        no = spr->no & 2047;
        // 4095 is for supergraphx only
        // no = (unsigned int)(spr->no & 4095);

        cgx = (atr >> 8) & 1;
        cgy = (atr >> 12) & 3;
        cgy |= cgy >> 1;
        no = (no >> 1) & ~(cgy * 2 + cgx);
        if (y >= Y2 || y + (cgy + 1) * 16 < Y1 || x >= FC_W
            || x + (cgx + 1) * 16 < 0) {
            continue;
        }

        uchar* R = &SPal[(atr & 15) * 16];
        for (i = 0; i < cgy * 2 + cgx + 1; i++) {
            if (vchanges[no + i]) {
                vchanges[no + i] = 0;
                sp2pixel(no + i);
            }
            if (!cgx)
                i++;
        }
        uchar* C = VRAM + (no * 128);
        uchar* C2 = VRAMS + (no * 32) * 4;  /* TEST */
        pos = XBUF_WIDTH * (y + 0) + x;
        inc = 2;
        if (atr & V_FLIP) {
            inc = -2;
            C += 15 * 2 + cgy * 256;
            C2 += (15 * 2 + cgy * 64) * 4;
        }
        y_sum = 0;

        for (i = 0; i <= cgy; i++) {
            t = Y1 - y - y_sum;
            h = 16;
            if (t > 0) {
                C += t * inc;
                C2 += (t * inc) * 4;
                h -= t;
                pos += t * XBUF_WIDTH;
            }
            if (h > Y2 - y - y_sum)
                h = Y2 - y - y_sum;
            if (spbg == 0) {
                sprite_usespbg = 1;
                if (atr & H_FLIP) {
                    for (j = 0; j <= cgx; j++) {
                        PutSpriteHflipMakeMask(osd_gfx_buffer + pos
                            + (cgx - j) * 16, C + j * 128, C2 + j * 32 * 4, R,
                            h, inc, SPM + pos + (cgx - j) * 16, n);
                    }
                } else {
                    for (j = 0; j <= cgx; j++) {
                        PutSpriteMakeMask(osd_gfx_buffer + pos + (j) * 16,
                            C + j * 128, C2 + j * 32 * 4, R, h, inc,
                            SPM + pos + j * 16, n);
                    }
                }
            } else if (sprite_usespbg) {
                if (atr & H_FLIP) {
                    for (j = 0; j <= cgx; j++) {
                        PutSpriteHflipM(osd_gfx_buffer + pos + (cgx - j) * 16,
                            C + j * 128, C2 + j * 32 * 4, R,
                            h, inc, SPM + pos + (cgx - j) * 16, n);
                    }
                } else {
                    for (j = 0; j <= cgx; j++) {
                        PutSpriteM(osd_gfx_buffer + pos + (j) * 16,
                            C + j * 128, C2 + j * 32 * 4, R, h, inc,
                            SPM + pos + j * 16, n);
                    }
                }
            } else {
                if (atr & H_FLIP) {
                    for (j = 0; j <= cgx; j++) {
                        PutSpriteHflip(osd_gfx_buffer + pos + (cgx - j) * 16,
                            C + j * 128, C2 + j * 32 * 4, R, h, inc);
                    }
                } else {
                    for (j = 0; j <= cgx; j++) {
                        PutSprite(osd_gfx_buffer + pos + (j) * 16,
                            C + j * 128, C2 + j * 32 * 4, R, h, inc);
                    }
                }
            }
            pos += h * XBUF_WIDTH;
            C += h * inc + 16 * 7 * inc;
            C2 += (h * inc + 16 * inc) * 4;
            y_sum += 16;
        }
    }
    ODROID_DEBUG_PERF_INCR2(debug_perf_part1, ODROID_DEBUG_PERF_SPRITE_RefreshSpriteExact)
