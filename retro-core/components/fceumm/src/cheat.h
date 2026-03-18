#ifndef _FCEU_CHEAT_H
#define _FCEU_CHEAT_H

#include "fceu-types.h"

extern uint8 *MMapPtrs[64];

void FCEU_CheatResetRAM(void);
void FCEU_CheatAddRAM(int s, uint32 A, uint8 *p);

void FCEU_LoadGameCheats(void);
void FCEU_FlushGameCheats(void);
void FCEU_ApplyPeriodicCheats(void);
void FCEU_PowerCheats(void);

void FCEU_ResetCheats(void);

#endif
