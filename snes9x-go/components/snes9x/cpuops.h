/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _CPUOPS_H_
#define _CPUOPS_H_

void S9xOpcode_NMI (void);
void S9xOpcode_IRQ (void);

#define CHECK_FOR_IRQ() {} // if (CPU.IRQLine) S9xOpcode_IRQ(); }

#endif
