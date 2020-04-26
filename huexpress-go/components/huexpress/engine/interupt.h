/****************************************************************************
 interupt.h
 Interrupt-controller hardware function prototypes
 ****************************************************************************/

#ifndef INTERUPT_H_
#define INTERUPT_H_

#include "cleantypes.h"

// definition of which bits correspond
// to which interrupt-request:

#define FL_IRQ2    0x01
#define FL_IRQ1    0x02
#define FL_TIQ     0x04


extern uchar irq_mask;
extern uchar irq_register;

extern void hw_put_1402(uchar);
extern void hw_put_1403(uchar);

extern uchar hw_get_1402(void);
extern uchar hw_get_1403(void);

#endif
