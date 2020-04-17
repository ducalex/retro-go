#ifndef MY_INLINE_SPRITE_sp2pixel
extern void sp2pixel(int no);
#else

#define sp2pixel(no_)                                                                \
{                                                                                   \
    uint32 M;                                                                       \
    uchar *_C;                                                                       \
    uchar *_C2;                                                                      \
    int _no = (no_);                                                                   \
    _C = &VRAM[_no * 128];                                                            \
    _C2 = &VRAMS[_no * 32 * 4];                                                       \
    /* 2 longs -> 16 nibbles => 32 loops for a 16*16 spr */                         \
    int16 i;                                                                        \
    for (i = 0; i < 32; i++, _C++, _C2 += 4) {                                        \
        uint32 L;                                                                   \
        M = _C[0];                                                                   \
        L = ((M & 0x88) >> 3)                                                       \
            | ((M & 0x44) << 6) | ((M & 0x22) << 15) | ((M & 0x11) << 24);          \
        M = _C[32];                                                                  \
        L |= ((M & 0x88) >> 2) | ((M & 0x44) << 7) | ((M & 0x22) << 16)             \
            | ((M & 0x11) << 25);                                                   \
        M = _C[64];                                                                  \
        L |= ((M & 0x88) >> 1) | ((M & 0x44) << 8) | ((M & 0x22) << 17)             \
            | ((M & 0x11) << 26);                                                   \
        M = _C[96];                                                                  \
        L |= ((M & 0x88)) | ((M & 0x44) << 9) | ((M & 0x22) << 18)                  \
            | ((M & 0x11) << 27);                                                   \
        /* _C2[0] = L; */                                                            \
        _C2[0] = L & 0xff;                                                           \
        _C2[1] = (L >> 8) & 0xff;                                                    \
        _C2[2] = (L >> 16) & 0xff;                                                   \
        _C2[3] = (L >> 24) & 0xff;                                                   \
    }                                                                               \
}
#endif

#ifndef MY_INLINE_SPRITE_PutSpriteHflipMakeMask
extern void PutSpriteHflipMakeMask(uchar * P, uchar * C, uchar * C2, uchar * R,
    int16 h, int16 inc, uchar * M, uchar pr);
#else

#define PutSpriteHflipMakeMask(P_, C_, C2_, R, h_, inc_, M_, pr_)                                   \
{                                                                                              \
    uchar * P = P_; uchar * _C = C_; uchar * _C2 = C2_; uchar * M = M_;                          \
    int16 _h = h_; int16 _inc = inc_; uchar pr = pr_;                                  \
    uint16 J;                                                                                \
    uint32 L;                                                                                \
    int16 i;                                                                                 \
    for (i = 0; i < _h; i++, _C += _inc, _C2 += _inc * 4,                                         \
        P += XBUF_WIDTH, M += XBUF_WIDTH) {                                                  \
        J = (_C[0] + (_C[1] << 8)) | (_C[32] + (_C[33] << 8))                                    \
            | (_C[64] + (_C[65] << 8)) | (_C[96] + (_C[97] << 8));                               \
        if (!J)                                                                              \
            continue;                                                                        \
        L = _C2[4] + (_C2[5] << 8) + (_C2[6] << 16) + (_C2[7] << 24); /* sp2pixel(C+1); */       \
        if (J & 0x8000) {                                                                    \
            P[15] = PAL((L >> 4) & 15);                                                      \
            M[15] = pr;                                                                      \
        }                                                                                    \
        if (J & 0x4000) {                                                                    \
            P[14] = PAL((L >> 12) & 15);                                                     \
            M[14] = pr;                                                                      \
        }                                                                                    \
        if (J & 0x2000) {                                                                    \
            P[13] = PAL((L >> 20) & 15);                                                     \
            M[13] = pr;                                                                      \
        }                                                                                    \
        if (J & 0x1000) {                                                                    \
            P[12] = PAL((L >> 28));                                                          \
            M[12] = pr;                                                                      \
        }                                                                                    \
        if (J & 0x0800) {                                                                    \
            P[11] = PAL((L) & 15);                                                           \
            M[11] = pr;                                                                      \
        }                                                                                    \
        if (J & 0x0400) {                                                                    \
            P[10] = PAL((L >> 8) & 15);                                                      \
            M[10] = pr;                                                                      \
        }                                                                                    \
        if (J & 0x0200) {                                                                    \
            P[9] = PAL((L >> 16) & 15);                                                      \
            M[9] = pr;                                                                       \
        }                                                                                    \
        if (J & 0x0100) {                                                                    \
            P[8] = PAL((L >> 24) & 15);                                                      \
            M[8] = pr;                                                                       \
        }                                                                                    \
        /* L = _C2[0];                        */ /* sp2pixel(C); */                           \
        L = _C2[0] + (_C2[1] << 8) + (_C2[2] << 16) + (_C2[3] << 24);                            \
        if (J & 0x80) {                                                                      \
            P[7] = PAL((L >> 4) & 15);                                                       \
            M[7] = pr;                                                                       \
        }                                                                                    \
        if (J & 0x40) {                                                                      \
            P[6] = PAL((L >> 12) & 15);                                                      \
            M[6] = pr;                                                                       \
        }                                                                                    \
        if (J & 0x20) {                                                                      \
            P[5] = PAL((L >> 20) & 15);                                                      \
            M[5] = pr;                                                                       \
        }                                                                                    \
        if (J & 0x10) {                                                                      \
            P[4] = PAL((L >> 28));                                                           \
            M[4] = pr;                                                                       \
        }                                                                                    \
        if (J & 0x08) {                                                                      \
            P[3] = PAL((L) & 15);                                                            \
            M[3] = pr;                                                                       \
        }                                                                                    \
        if (J & 0x04) {                                                                      \
            P[2] = PAL((L >> 8) & 15);                                                       \
            M[2] = pr;                                                                       \
        }                                                                                    \
        if (J & 0x02) {                                                                      \
            P[1] = PAL((L >> 16) & 15);                                                      \
            M[1] = pr;                                                                       \
        }                                                                                    \
        if (J & 0x01) {                                                                      \
            P[0] = PAL((L >> 24) & 15);                                                      \
            M[0] = pr;                                                                       \
        }                                                                                    \
    }                                                                                        \
}
#endif

#ifndef MY_INLINE_SPRITE_PutSpriteHflipM
extern void PutSpriteHflipM(uchar * P, uchar * C, uchar * C2, uchar * R, int16 h,
                int16 inc, uchar * M, uchar pr);
#else
#define PutSpriteHflipM(P_, C_, C2_, R, h_, inc_, M_, pr_)                            \
{                                                                              \
    uchar * P = P_; uchar * _C = C_; uchar * _C2 = C2_; uchar * M = M_;                          \
    int16 _h = h_; int16 _inc = inc_; uchar pr = pr_;                                  \
    uint16 J;                                                                  \
    uint32 L;                                                                  \
    int16 i;                                                                   \
    for (i = 0; i < _h; i++, _C += _inc, _C2 += _inc * 4,                           \
        P += XBUF_WIDTH, M += XBUF_WIDTH) {                                    \
        J = (_C[0] + (_C[1] << 8)) | (_C[32] + (_C[33] << 8)) | (_C[64]             \
            + (_C[65] << 8)) | (_C[96] + (_C[97] << 8));                          \
        if (!J)                                                                \
            continue;                                                          \
                                                                               \
        /* L = _C2[1];        */ /* sp2pixel(C+1); */                           \
        L = _C2[4] + (_C2[5] << 8) + (_C2[6] << 16) + (_C2[7] << 24);              \
        if ((J & 0x8000) && M[15] <= pr)                                       \
            P[15] = PAL((L >> 4) & 15);                                        \
        if ((J & 0x4000) && M[14] <= pr)                                       \
            P[14] = PAL((L >> 12) & 15);                                       \
        if ((J & 0x2000) && M[13] <= pr)                                       \
            P[13] = PAL((L >> 20) & 15);                                       \
        if ((J & 0x1000) && M[12] <= pr)                                       \
            P[12] = PAL((L >> 28));                                            \
        if ((J & 0x0800) && M[11] <= pr)                                       \
            P[11] = PAL((L) & 15);                                             \
        if ((J & 0x0400) && M[10] <= pr)                                       \
            P[10] = PAL((L >> 8) & 15);                                        \
        if ((J & 0x0200) && M[9] <= pr)                                        \
            P[9] = PAL((L >> 16) & 15);                                        \
        if ((J & 0x0100) && M[8] <= pr)                                        \
            P[8] = PAL((L >> 24) & 15);                                        \
        /* L = _C2[0];        */ /* sp2pixel(C); */                             \
        L = _C2[0] + (_C2[1] << 8) + (_C2[2] << 16) + (_C2[3] << 24);              \
        if ((J & 0x80) && M[7] <= pr)                                          \
            P[7] = PAL((L >> 4) & 15);                                         \
        if ((J & 0x40) && M[6] <= pr)                                          \
            P[6] = PAL((L >> 12) & 15);                                        \
        if ((J & 0x20) && M[5] <= pr)                                          \
            P[5] = PAL((L >> 20) & 15);                                        \
        if ((J & 0x10) && M[4] <= pr)                                          \
            P[4] = PAL((L >> 28));                                             \
        if ((J & 0x08) && M[3] <= pr)                                          \
            P[3] = PAL((L) & 15);                                              \
        if ((J & 0x04) && M[2] <= pr)                                          \
            P[2] = PAL((L >> 8) & 15);                                         \
        if ((J & 0x02) && M[1] <= pr)                                          \
            P[1] = PAL((L >> 16) & 15);                                        \
        if ((J & 0x01) && M[0] <= pr)                                          \
            P[0] = PAL((L >> 24) & 15);                                        \
    }                                                                          \
}
#endif

#ifndef MY_INLINE_SPRITE_PutSpriteHflip
extern void PutSpriteHflip(uchar * P, uchar * C, uchar * C2, uchar * R, int16 h,
    int16 inc);
#else
#define PutSpriteHflip(P_, C_, C2_, R, h_, inc_)                                       \
{                                                                                 \
    uchar * P = P_; uchar * _C = C_; uchar * _C2 = C2_;                       \
    int16 _h = h_; int16 _inc = inc_;                                      \
    uint16 J;                                                                     \
    uint32 L;                                                                     \
    uint32* C2r = (uint32*)_C2;                                                    \
    int16 i;                                                                      \
    for (i = 0; i < _h; i++, _C += _inc, C2r += _inc, P += XBUF_WIDTH) {              \
        J = (_C[0] + (_C[1] << 8)) | (_C[32] + (_C[33] << 8))                         \
            | (_C[64] + (_C[65] << 8)) | (_C[96] + (_C[97] << 8));                    \
                                                                                  \
        if (!J)                                                                   \
            continue;                                                             \
        L = C2r[1];             /* sp2pixel(C+1); */                              \
        if (J & 0x8000)                                                           \
            P[15] = PAL((L >> 4) & 15);                                           \
        if (J & 0x4000)                                                           \
            P[14] = PAL((L >> 12) & 15);                                          \
        if (J & 0x2000)                                                           \
            P[13] = PAL((L >> 20) & 15);                                          \
        if (J & 0x1000)                                                           \
            P[12] = PAL((L >> 28));                                               \
        if (J & 0x0800)                                                           \
            P[11] = PAL((L) & 15);                                                \
        if (J & 0x0400)                                                           \
            P[10] = PAL((L >> 8) & 15);                                           \
        if (J & 0x0200)                                                           \
            P[9] = PAL((L >> 16) & 15);                                           \
        if (J & 0x0100)                                                           \
            P[8] = PAL((L >> 24) & 15);                                           \
        L = C2r[0];             /* sp2pixel(C); */                                \
        if (J & 0x80)                                                             \
            P[7] = PAL((L >> 4) & 15);                                            \
        if (J & 0x40)                                                             \
            P[6] = PAL((L >> 12) & 15);                                           \
        if (J & 0x20)                                                             \
            P[5] = PAL((L >> 20) & 15);                                           \
        if (J & 0x10)                                                             \
            P[4] = PAL((L >> 28));                                                \
        if (J & 0x08)                                                             \
            P[3] = PAL((L) & 15);                                                 \
        if (J & 0x04)                                                             \
            P[2] = PAL((L >> 8) & 15);                                            \
        if (J & 0x02)                                                             \
            P[1] = PAL((L >> 16) & 15);                                           \
        if (J & 0x01)                                                             \
            P[0] = PAL((L >> 24) & 15);                                           \
    }                                                                             \
}
#endif

#ifndef MY_INLINE_SPRITE_plane2pixel
extern void plane2pixel(int no);
#else
#define plane2pixel(no_)                                                             \
{                                                                                    \
    uint32 M;                                                                        \
    int _no = (no_);                                                                 \
    uchar *_C = VRAM + _no * 32;                                                     \
    uchar *_C2 = VRAM2 + _no * 8 * 4;                                                \
    uint32 L;                                                                        \
                                                                                     \
    int16 i;                                                                         \
    TRACE("Planing tile %d\n", _no);                                                 \
    for (i = 0; i < 8; i++, _C += 2, _C2 += 4) {                                     \
        M = _C[0];                                                                   \
        TRACE("_C[0]=%02X\n", M);                                                    \
        L = ((M & 0x88) >> 3)                                                        \
            | ((M & 0x44) << 6) | ((M & 0x22) << 15)                                 \
            | ((M & 0x11) << 24);                                                    \
        M = _C[1];                                                                   \
        TRACE("_C[1]=%02X\n", M);                                                    \
        L |= ((M & 0x88) >> 2)                                                       \
            | ((M & 0x44) << 7) | ((M & 0x22) << 16)                                 \
            | ((M & 0x11) << 25);                                                    \
        M = _C[16];                                                                  \
        TRACE("_C[16]=%02X\n", M);                                                   \
        L |= ((M & 0x88) >> 1)                                                       \
            | ((M & 0x44) << 8) | ((M & 0x22) << 17) | ((M & 0x11) << 26);           \
        M = _C[17];                                                                  \
        TRACE("_C[17]=%02X\n", M);                                                   \
        L |= ((M & 0x88))                                                            \
            | ((M & 0x44) << 9) | ((M & 0x22) << 18) | ((M & 0x11) << 27);           \
        /* _C2[0] = L;                      37261504 */                          \
        _C2[0] = L & 0xff;                                                           \
        _C2[1] = (L >> 8) & 0xff;                                                    \
        _C2[2] = (L >> 16) & 0xff;                                                   \
        _C2[3] = (L >> 24) & 0xff;                                                   \
        TRACE("L=%04X\n", L);                                                        \
    }                                                                                \
}
#endif


#ifndef MY_INLINE_SPRITE_RefreshScreen
extern void RefreshScreen(void);
#else
#define RefreshScreen() \
    \
    frame += UPeriod + 1; \
    (*osd_gfx_driver_list[video_driver].draw) (); \
    /* memset(osd_gfx_buffer, Pal[0], 240 * XBUF_WIDTH); */ \
    /* We don't clear the part out of reach of the screen blitter */ \
    /* memset(SPM, 0, 240 * XBUF_WIDTH); */

#endif
