#ifndef _FCEU_PALETTE_H
#define _FCEU_PALETTE_H

typedef struct {
	uint8 r, g, b;
} pal;

extern pal *palo;
extern int ipalette;
void FCEU_ResetPalette(void);

void FCEU_ResetPalette(void);
void FCEU_LoadGamePalette(void);

#endif
