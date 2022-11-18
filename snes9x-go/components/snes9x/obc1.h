/* This file is part of Snes9x. See LICENSE file. */

#ifndef _OBC1_H_
#define _OBC1_H_

uint8_t GetOBC1(uint16_t Address);
void SetOBC1(uint8_t Byte, uint16_t Address);
uint8_t* GetMemPointerOBC1(uint32_t Address);
void ResetOBC1(void);
#endif
