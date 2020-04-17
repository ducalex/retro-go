#ifndef _DJGPP_INCLUDE_MIX_H
#define  _DJGPP_INCLUDE_MIX_H


#include "pce.h"
#include "sound.h"
#include "debug.h"


uint32 WriteBufferAdpcm8(uchar * buf,
						 uint32 begin,
						 uint32 size, char *Index, int32 * PreviousValue);

void WriteBuffer(char *buf, int ch, unsigned dwSize);

#endif
