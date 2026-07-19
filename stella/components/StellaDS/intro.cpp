// =====================================================================================================
// Stella DS/DSi Pheonix Edition - Improved Version by Dave Bernazzani (wavemotion)
//
// Copyright (c) 2020-2026 by Dave Bernazzani
//
// Copying and distribution of this emulator, it's source code and associated 
// readme files, with or without modification, are permitted in any medium without 
// royalty provided this copyright notice is used and wavemotion-dave (Phoenix-Edition),
// Alekmaul (original port) are thanked profusely along with the entire Stella Team.
//
// The StellaDS emulator is offered as-is, without any warranty.
// =====================================================================================================
#include <nds.h>
#include <stdio.h>

#include "StellaDS.h"

#include "mus_intro_wav.h"
#include "pdev_tbg0.h"
#include "pdev_bg0.h"

volatile u16 vusCptVBL;

void vblankIntro() {
  vusCptVBL++;
}

// Intro with portabledev logo
void intro_logo(void) {
  short int soundId=-1; 
  u8 bOK;

  // Init graphics
  videoSetMode(MODE_0_2D | DISPLAY_BG0_ACTIVE );
  videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE );
  vramSetBankA(VRAM_A_MAIN_BG); vramSetBankC(VRAM_C_SUB_BG);
  irqSet(IRQ_VBLANK, vblankIntro);
  irqEnable(IRQ_VBLANK);
  
  // Init BG
	int bg1 = bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 31,0);

  // Init sub BG
	int bg1s = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 31,0);

  REG_BLDCNT = BLEND_FADE_BLACK | BLEND_SRC_BG0 | BLEND_DST_BG0; REG_BLDY = 16;
  REG_BLDCNT_SUB = BLEND_FADE_BLACK | BLEND_SRC_BG0 | BLEND_DST_BG0; REG_BLDY_SUB = 16;

  // Show portabledev
  decompress(pdev_tbg0Tiles, bgGetGfxPtr(bg1), LZ77Vram);
  decompress(pdev_tbg0Map, (void*) bgGetMapPtr(bg1), LZ77Vram);
  dmaCopy((void *) pdev_tbg0Pal,(u16*) BG_PALETTE,256*2);

  decompress(pdev_bg0Tiles, bgGetGfxPtr(bg1s), LZ77Vram);
  decompress(pdev_bg0Map, (void*) bgGetMapPtr(bg1s), LZ77Vram);
  dmaCopy((void *) pdev_bg0Pal,(u16*) BG_PALETTE_SUB,256*2);

  FadeToColor(0,BLEND_FADE_BLACK | BLEND_SRC_BG0 | BLEND_DST_BG0,3,0,3);

  soundId = soundPlaySample((const void *) mus_intro_wav, SoundFormat_ADPCM, mus_intro_wav_size, 22050, 127, 64, false, 0);

  bOK=false;
  while (!bOK) { if ( !(keysCurrent() & 0x1FFF) ) bOK=true; } // 0x1FFF = key or pen
  vusCptVBL=0;bOK=false;
  while (!bOK && (vusCptVBL<3*60)) { if (keysCurrent() & 0x1FFF ) bOK=true; }
  bOK=false;
  while (!bOK) { if ( !(keysCurrent() & 0x1FFF) ) bOK=true; }

  FadeToColor(1,BLEND_FADE_WHITE | BLEND_SRC_BG0 | BLEND_DST_BG0,3,16,3);
  if (soundId!=-1) soundKill(soundId);
}
