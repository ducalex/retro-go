#ifndef _DJGPP_INCLUDE_FOLLOWOP_H
#define _DJGPP_INCLUDE_FOLLOWOP_H

#if 0

// Declaration for functions used to know the following instruction

#include "config.h"
#include "cleantypes.h"
#include "optable.h"

#include "h6280.h"

#include "pce.h"

uint16 follow_straight(uint16);
// for most op, IP just go on

uint16 follow_BBRi(uint16 where);
// for BBRi op

uint16 follow_BCC(uint16 where);
// for BCC op

uint16 follow_BBSi(uint16 where);
// for BBSi op

uint16 follow_BCS(uint16 where);
// for BCS op

uint16 follow_BEQ(uint16 where);
// for BEQ op

uint16 follow_BNE(uint16 where);
// for BNE op

uint16 follow_BMI(uint16 where);
// for BMI op

uint16 follow_BPL(uint16 where);
// for BPL op

uint16 follow_BRA(uint16 where);
// for BRA op

uint16 follow_BSR(uint16 where);
// for BSR op

uint16 follow_BVS(uint16 where);
// for BVS op

uint16 follow_BVC(uint16 where);
// for BVC op

uint16 follow_JMPabs(uint16 where);
// for JMP absolute

uint16 follow_JMPindir(uint16 where);
// for JMP indirect

uint16 follow_JMPindirX(uint16 where);
// for JMP indirect with X

uint16 follow_JSR(uint16 where);
// for JSR op

uint16 follow_RTI(uint16 where);
// for RTI op

uint16 follow_RTS(uint16 where);
// for RTS op

#endif

#endif
