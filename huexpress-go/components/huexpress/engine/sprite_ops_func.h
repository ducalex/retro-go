#ifndef MY_INLINE_SPRITE_sp2pixel
/*****************************************************************************

        Function:   sp2pixel

        Description:convert a PCE coded sprite into a linear one
        Parameters:int no,the number of the sprite to convert
        Return:nothing but update VRAMS

*****************************************************************************/
void
sp2pixel(int no)
{
    uint32 M;
    uchar *C;
    uchar *C2;

    C = &VRAM[no * 128];
    C2 = &VRAMS[no * 32 * 4];
    // 2 longs -> 16 nibbles => 32 loops for a 16*16 spr

    int16 i;
    for (i = 0; i < 32; i++, C++, C2 += 4) {
        uint32 L;
        M = C[0];
        L = ((M & 0x88) >> 3)
            | ((M & 0x44) << 6) | ((M & 0x22) << 15) | ((M & 0x11) << 24);
        M = C[32];
        L |= ((M & 0x88) >> 2) | ((M & 0x44) << 7) | ((M & 0x22) << 16)
            | ((M & 0x11) << 25);
        M = C[64];
        L |= ((M & 0x88) >> 1) | ((M & 0x44) << 8) | ((M & 0x22) << 17)
            | ((M & 0x11) << 26);
        M = C[96];
        L |= ((M & 0x88)) | ((M & 0x44) << 9) | ((M & 0x22) << 18)
            | ((M & 0x11) << 27);
        /* C2[0] = L; */
        C2[0] = L & 0xff;
        C2[1] = (L >> 8) & 0xff;
        C2[2] = (L >> 16) & 0xff;
        C2[3] = (L >> 24) & 0xff;
    }
}
#endif

#ifndef MY_INLINE_SPRITE_PutSpriteHflipMakeMask
void
PutSpriteHflipMakeMask(uchar * P, uchar * C, uchar * C2, uchar * R,
    int16 h, int16 inc, uchar * M, uchar pr)
{
    uint16 J;
    uint32 L;

    int16 i;
    for (i = 0; i < h; i++, C += inc, C2 += inc * 4,
        P += XBUF_WIDTH, M += XBUF_WIDTH) {
        J = (C[0] + (C[1] << 8)) | (C[32] + (C[33] << 8))
            | (C[64] + (C[65] << 8)) | (C[96] + (C[97] << 8));
#if 0
        J = ((uint16 *) C)[0] | ((uint16 *) C)[16] | ((uint16 *) C)[32]
            | ((uint16 *) C)[48];
#endif
        if (!J)
            continue;
        L = C2[4] + (C2[5] << 8) + (C2[6] << 16) + (C2[7] << 24);   //sp2pixel(C+1);
        if (J & 0x8000) {
            P[15] = PAL((L >> 4) & 15);
            M[15] = pr;
        }
        if (J & 0x4000) {
            P[14] = PAL((L >> 12) & 15);
            M[14] = pr;
        }
        if (J & 0x2000) {
            P[13] = PAL((L >> 20) & 15);
            M[13] = pr;
        }
        if (J & 0x1000) {
            P[12] = PAL((L >> 28));
            M[12] = pr;
        }
        if (J & 0x0800) {
            P[11] = PAL((L) & 15);
            M[11] = pr;
        }
        if (J & 0x0400) {
            P[10] = PAL((L >> 8) & 15);
            M[10] = pr;
        }
        if (J & 0x0200) {
            P[9] = PAL((L >> 16) & 15);
            M[9] = pr;
        }
        if (J & 0x0100) {
            P[8] = PAL((L >> 24) & 15);
            M[8] = pr;
        }
        /* L = C2[0];                        *///sp2pixel(C);
        L = C2[0] + (C2[1] << 8) + (C2[2] << 16) + (C2[3] << 24);
        if (J & 0x80) {
            P[7] = PAL((L >> 4) & 15);
            M[7] = pr;
        }
        if (J & 0x40) {
            P[6] = PAL((L >> 12) & 15);
            M[6] = pr;
        }
        if (J & 0x20) {
            P[5] = PAL((L >> 20) & 15);
            M[5] = pr;
        }
        if (J & 0x10) {
            P[4] = PAL((L >> 28));
            M[4] = pr;
        }
        if (J & 0x08) {
            P[3] = PAL((L) & 15);
            M[3] = pr;
        }
        if (J & 0x04) {
            P[2] = PAL((L >> 8) & 15);
            M[2] = pr;
        }
        if (J & 0x02) {
            P[1] = PAL((L >> 16) & 15);
            M[1] = pr;
        }
        if (J & 0x01) {
            P[0] = PAL((L >> 24) & 15);
            M[0] = pr;
        }
    }
}
#endif
#ifndef MY_INLINE_SPRITE_PutSpriteHflipM
void
PutSpriteHflipM(uchar * P, uchar * C, uchar * C2, uchar * R, int16 h,
                int16 inc, uchar * M, uchar pr)
{
    uint16 J;
    uint32 L;

    int16 i;
    for (i = 0; i < h; i++, C += inc, C2 += inc * 4,
        P += XBUF_WIDTH, M += XBUF_WIDTH) {
        J = (C[0] + (C[1] << 8)) | (C[32] + (C[33] << 8)) | (C[64]
            + (C[65] << 8)) | (C[96] + (C[97] << 8));
#if 0
        J = ((uint16 *) C)[0] | ((uint16 *) C)[16] | ((uint16 *) C)[32]
            | ((uint16 *) C)[48];
#endif
        if (!J)
            continue;

        /* L = C2[1];        *///sp2pixel(C+1);
        L = C2[4] + (C2[5] << 8) + (C2[6] << 16) + (C2[7] << 24);
        if ((J & 0x8000) && M[15] <= pr)
            P[15] = PAL((L >> 4) & 15);
        if ((J & 0x4000) && M[14] <= pr)
            P[14] = PAL((L >> 12) & 15);
        if ((J & 0x2000) && M[13] <= pr)
            P[13] = PAL((L >> 20) & 15);
        if ((J & 0x1000) && M[12] <= pr)
            P[12] = PAL((L >> 28));
        if ((J & 0x0800) && M[11] <= pr)
            P[11] = PAL((L) & 15);
        if ((J & 0x0400) && M[10] <= pr)
            P[10] = PAL((L >> 8) & 15);
        if ((J & 0x0200) && M[9] <= pr)
            P[9] = PAL((L >> 16) & 15);
        if ((J & 0x0100) && M[8] <= pr)
            P[8] = PAL((L >> 24) & 15);
        /* L = C2[0];        *///sp2pixel(C);
        L = C2[0] + (C2[1] << 8) + (C2[2] << 16) + (C2[3] << 24);
        if ((J & 0x80) && M[7] <= pr)
            P[7] = PAL((L >> 4) & 15);
        if ((J & 0x40) && M[6] <= pr)
            P[6] = PAL((L >> 12) & 15);
        if ((J & 0x20) && M[5] <= pr)
            P[5] = PAL((L >> 20) & 15);
        if ((J & 0x10) && M[4] <= pr)
            P[4] = PAL((L >> 28));
        if ((J & 0x08) && M[3] <= pr)
            P[3] = PAL((L) & 15);
        if ((J & 0x04) && M[2] <= pr)
            P[2] = PAL((L >> 8) & 15);
        if ((J & 0x02) && M[1] <= pr)
            P[1] = PAL((L >> 16) & 15);
        if ((J & 0x01) && M[0] <= pr)
            P[0] = PAL((L >> 24) & 15);
    }
}
#endif
#ifndef MY_INLINE_SPRITE_PutSpriteHflip
void
PutSpriteHflip(uchar * P, uchar * C, uchar * C2, uchar * R, int16 h,
    int16 inc)
{
    uint16 J;
    uint32 L;

    // TODO: This is a hack, see issue #1
    // the graphics are mis-aligned on flipped sprites when we go
    // with the uchar that everything else uses.
    uint32* C2r = (uint32*)C2;

    int16 i;
    for (i = 0; i < h; i++, C += inc, C2r += inc, P += XBUF_WIDTH) {
        J = (C[0] + (C[1] << 8)) | (C[32] + (C[33] << 8))
            | (C[64] + (C[65] << 8)) | (C[96] + (C[97] << 8));
#if 0
        J = ((uint16 *) C)[0] | ((uint16 *) C)[16] | ((uint16 *) C)[32]
            | ((uint16 *) C)[48];
#endif

        if (!J)
            continue;
        L = C2r[1];             //sp2pixel(C+1);
        if (J & 0x8000)
            P[15] = PAL((L >> 4) & 15);
        if (J & 0x4000)
            P[14] = PAL((L >> 12) & 15);
        if (J & 0x2000)
            P[13] = PAL((L >> 20) & 15);
        if (J & 0x1000)
            P[12] = PAL((L >> 28));
        if (J & 0x0800)
            P[11] = PAL((L) & 15);
        if (J & 0x0400)
            P[10] = PAL((L >> 8) & 15);
        if (J & 0x0200)
            P[9] = PAL((L >> 16) & 15);
        if (J & 0x0100)
            P[8] = PAL((L >> 24) & 15);
        L = C2r[0];             //sp2pixel(C);
        if (J & 0x80)
            P[7] = PAL((L >> 4) & 15);
        if (J & 0x40)
            P[6] = PAL((L >> 12) & 15);
        if (J & 0x20)
            P[5] = PAL((L >> 20) & 15);
        if (J & 0x10)
            P[4] = PAL((L >> 28));
        if (J & 0x08)
            P[3] = PAL((L) & 15);
        if (J & 0x04)
            P[2] = PAL((L >> 8) & 15);
        if (J & 0x02)
            P[1] = PAL((L >> 16) & 15);
        if (J & 0x01)
            P[0] = PAL((L >> 24) & 15);
    }
}
#endif

#ifndef MY_INLINE_SPRITE_plane2pixel
/*****************************************************************************

        Function:   plane2pixel

        Description: convert a PCE coded tile into a linear one
        Parameters:int no, the number of the tile to convert
        Return:nothing, but updates VRAM2

*****************************************************************************/
void
plane2pixel(int no)
{
    uint32 M;
    uchar *C = VRAM + no * 32;
    uchar *C2 = VRAM2 + no * 8 * 4;
    uint32 L;

    int16 i;
    TRACE("Planing tile %d\n", no);
    for (i = 0; i < 8; i++, C += 2, C2 += 4) {
        M = C[0];
        TRACE("C[0]=%02X\n", M);
        L = ((M & 0x88) >> 3)
            | ((M & 0x44) << 6) | ((M & 0x22) << 15)
            | ((M & 0x11) << 24);
        M = C[1];
        TRACE("C[1]=%02X\n", M);
        L |= ((M & 0x88) >> 2)
            | ((M & 0x44) << 7) | ((M & 0x22) << 16)
            | ((M & 0x11) << 25);
        M = C[16];
        TRACE("C[16]=%02X\n", M);
        L |= ((M & 0x88) >> 1)
            | ((M & 0x44) << 8) | ((M & 0x22) << 17) | ((M & 0x11) << 26);
        M = C[17];
        TRACE("C[17]=%02X\n", M);
        L |= ((M & 0x88))
            | ((M & 0x44) << 9) | ((M & 0x22) << 18) | ((M & 0x11) << 27);
        /* C2[0] = L;                        //37261504 */
        C2[0] = L & 0xff;
        C2[1] = (L >> 8) & 0xff;
        C2[2] = (L >> 16) & 0xff;
        C2[3] = (L >> 24) & 0xff;
        TRACE("L=%04X\n", L);
    }
}
#endif


#ifndef MY_INLINE_SPRITE_RefreshScreen
/*****************************************************************************

        Function: RefreshScreen

        Description: refresh screen
        Parameters: none
        Return: nothing

*****************************************************************************/
void
RefreshScreen(void)
{
    frame += UPeriod + 1;
#ifndef MY_EXCLUDE
    HCD_handle_subtitle();
#endif
    /*
       {
       char* spr = SPRAM;
       uint32 CRC = -1;
       int index;
       for (index = 0; index < 64 * sizeof(SPR); index++) {
       spr[index] ^= CRC;
       CRC >>= 8;
       CRC ^= TAB_CONST[spr[index]];
       }
       Log("frame %d : CRC = %X\n", frame, CRC);

       {
       char* spr = ((uint16*)VRAM) + 2048;
       uint32 CRC = -1;
       int index;
       for (index = 0; index < 64 * sizeof(SPR); index++) {
       char tmp = spr[index];
       tmp ^= CRC;
       CRC >>= 8;
       CRC ^= TAB_CONST[tmp];
       }
       Log("frame %d : CRC VRAM[2048-] = %X\n", frame, CRC);
       }
     */

    (*osd_gfx_driver_list[video_driver].draw) ();

#if ENABLE_TRACING_GFX
    /*
       Log("VRAM: %02x%02x%02x%02x %02x%02x%02x%02x\n",
       VRAM[0],
       VRAM[1],
       VRAM[2],
       VRAM[3],
       VRAM[4],
       VRAM[5],
       VRAM[6],
       VRAM[7]);
     */
    {
        int index;
        uchar tmp_data;
        uint32 CRC = 0xFFFFFFFF;

        for (index = 0; index < 0x10000; index++) {
            tmp_data = VRAM2[index];
            tmp_data ^= CRC;
            CRC >>= 8;
            CRC ^= TAB_CONST[tmp_data];
        }
        Log("VRAM2 CRC = %08x\n", ~CRC);

        for (index = 0; index < 0x10000; index++) {
            if (VRAM2[index] != 0)
                Log("%04X:%08X\n", index, VRAM2[index]);
        }
    }

    /*
       {
       int index;
       uchar tmp_data;
       unsigned int CRC = 0xFFFFFFFF;

       for (index = 0; index < 256; index++) {
       tmp_data = zp_base[index];
       tmp_data ^= CRC;
       CRC >>= 8;
       CRC ^= TAB_CONST[tmp_data];
       }
       Log("ZP CRC = %08x\n", ~CRC);
       }
     */
#endif
    //memset(osd_gfx_buffer, Pal[0], 240 * XBUF_WIDTH);
    // We don't clear the part out of reach of the screen blitter
    //memset(SPM, 0, 240 * XBUF_WIDTH);
}
#endif
