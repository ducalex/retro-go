#ifndef _SHARE_H
#define _SHARE_H

#include "../fceu-types.h"
#include "../input.h"
#include "../fceu.h"
#include "../ppu.h"
#include "../x6502.h"
#include "../palette.h"
#include "../fceu-state.h"

void FCEU_DrawCursor(uint8 *buf, int xc, int yc);
void FCEU_DrawGunSight(uint8 *buf, int xc, int yc);

#endif /* _SHARE_H */
