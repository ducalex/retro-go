/*
** Nofrendo (c) 1998-2000 Matthew Conte (matt@conte.com)
**
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of version 2 of the GNU Library General
** Public License as published by the Free Software Foundation.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.  To obtain a
** copy of the GNU Library General Public License, write to the Free
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
**
**
** nes6502.h
**
** NES custom 6502 CPU definitions / prototypes
** $Id: nes6502.h,v 1.2 2001/04/27 14:37:11 neil Exp $
*/

/* NOTE: 16-bit addresses avoided like the plague: use 32-bit values
**       wherever humanly possible
*/
#ifndef _NES6502_H_
#define _NES6502_H_

#include <nofrendo.h>

/* P (flag) register bitmasks */
#define  N_FLAG         0x80
#define  V_FLAG         0x40
#define  R_FLAG         0x20  /* Reserved, always 1 */
#define  B_FLAG         0x10
#define  D_FLAG         0x08
#define  I_FLAG         0x04
#define  Z_FLAG         0x02
#define  C_FLAG         0x01

/* Vector addresses */
#define  NMI_VECTOR     0xFFFA
#define  RESET_VECTOR   0xFFFC
#define  IRQ_VECTOR     0xFFFE

/* cycle counts for interrupts */
#define  INT_CYCLES     7
#define  RESET_CYCLES   6

#define  NMI_MASK       0x01
#define  IRQ_MASK       0x02

typedef struct
{
   uint32 pc_reg;
   uint8 a_reg;
   uint8 x_reg, y_reg;
   uint8 s_reg, p_reg;

   uint8 *zp, *stack;

   bool int_pending;
   bool jammed;

   long total_cycles;
   long burn_cycles;
} nes6502_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Functions which govern the 6502's execution */
extern int nes6502_execute(int total_cycles);
extern void nes6502_nmi(void);
extern void nes6502_irq(void);
extern uint32 nes6502_getcycles(void);
extern void nes6502_burn(int cycles);
extern void nes6502_release(void);

extern nes6502_t *nes6502_init(mem_t *mem);
extern void nes6502_refresh(void);
extern void nes6502_reset(void);
extern void nes6502_shutdown(void);

extern void nes6502_setcontext(nes6502_t *cpu);
extern void nes6502_getcontext(nes6502_t *cpu);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _NES6502_H_ */
