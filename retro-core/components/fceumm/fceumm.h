#ifndef _FCEUMM_H_
#define _FCEUMM_H_

#include "src/driver.h"
#include "src/fceu-cart.h"
#include "src/fceu-state.h"
#include "src/fceu-types.h"
#include "src/fceu.h"
#include "src/fds.h"
#include "src/palette.h"
#include "src/ppu.h"
#include "src/video.h"

// Forward declarations of globals used by main_nes.c
extern uint8 *XBuf;
extern void FCEUI_SetPaletteArray(uint8 *pal);
extern CartInfo iNESCart;
extern CartInfo UNIFCart;

#endif
