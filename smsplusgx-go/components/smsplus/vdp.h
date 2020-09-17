/******************************************************************************
 *  Sega Master System / GameGear Emulator
 *  Copyright (C) 1998-2007  Charles MacDonald
 *
 *  additionnal code by Eke-Eke (SMS Plus GX)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Video Display Processor (VDP) emulation.
 *
 ******************************************************************************/

#ifndef _VDP_H_
#define _VDP_H_

/*
    vdp1

    mode 4 when m4 set and m1 reset

    vdp2

    mode 4 when m4 set and m2,m1 != 1,0


*/

/* VDP context */
typedef struct
{
    uint8 vram[0x4000];
    uint8 cram[0x40];
    uint8 reg[0x10];

    uint8 vscroll;
    uint8 status;
    uint8 latch;
    uint8 pending;

    uint16 addr;
    uint8 code;
    uint8 buffer;

    int pn;
    int ct;
    int pg;
    int sa;
    int sg;
    int ntab;
    int satb;
    int line;
    int left;

    uint16 lpf;
    uint8 height;
    uint8 extended;

    uint8 mode;
    uint8 irq;
    uint8 vint_pending;
    uint8 hint_pending;

    uint16 cram_latch;
    uint16 spr_col;

    uint8 spr_ovr;
    uint8 bd;
    uint8 _pad00;
    uint8 _pad01;
} __attribute__((packed, aligned(1))) vdp_t;

/* Global data */
extern vdp_t vdp;
extern const uint8 hc_256[228];
extern const uint8 *vc_table[3];

/* Function prototypes */
extern void vdp_init(void);
extern void vdp_shutdown(void);
extern void vdp_reset(void);
extern uint8 vdp_read(int offset);
extern void vdp_write(int offset, uint8 data);
extern void gg_vdp_write(int offset, uint8 data);
extern void tms_write(int offset, int data);

static inline uint8 vdp_counter_read(int offset)
{
    if (offset & 1)  /* V Counter */
      return sms.hlatch;
    else /* H Counter -- return previously latched values or ZERO */
      return vc_table[vdp.extended][z80_get_elapsed_cycles() / CYCLES_PER_LINE];
}

#endif /* _VDP_H_ */
