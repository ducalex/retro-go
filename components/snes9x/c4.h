/* This file is part of Snes9x. See LICENSE file. */

#ifndef _C4_H_
#define _C4_H_

#include "port.h"

extern int16_t C4WFXVal;
extern int16_t C4WFYVal;
extern int16_t C4WFZVal;
extern int16_t C4WFX2Val;
extern int16_t C4WFY2Val;
extern int16_t C4WFDist;
extern int16_t C4WFScale;

void C4TransfWireFrame();
void C4TransfWireFrame2();
void C4CalcWireFrame();

extern int16_t C41FXVal;
extern int16_t C41FYVal;
extern int16_t C41FAngleRes;
extern int16_t C41FDist;
extern int16_t C41FDistVal;

extern int32_t tanval;

int16_t _atan2(int16_t x, int16_t y);

extern const int16_t C4CosTable[];
extern const int16_t C4SinTable[];
#endif
