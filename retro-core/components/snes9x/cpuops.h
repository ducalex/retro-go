/* This file is part of Snes9x. See LICENSE file. */

#ifndef _CPUOPS_H_
#define _CPUOPS_H_
void S9xOpcode_NMI();
void S9xOpcode_IRQ();

#define CHECK_FOR_IRQ() \
if (CPU.IRQActive && !CheckFlag (IRQ) && !Settings.DisableIRQ) \
    S9xOpcode_IRQ()

#endif
