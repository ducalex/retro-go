#ifndef _MMC3_H
#define _MMC3_H

extern uint8 MMC3_cmd;
extern uint8 mmc3opts;
extern uint8 A000B;
extern uint8 A001B;
extern uint8 EXPREGS[8];
extern uint8 DRegBuf[8];

extern void (*pwrap)(uint32 A, uint8 V);
extern void (*cwrap)(uint32 A, uint8 V);
extern void (*mwrap)(uint8 V);

void GenMMC3Power(void);
void GenMMC3Restore(int version);
void MMC3RegReset(void);
void GenMMC3Close(void);
void FixMMC3PRG(int V);
void FixMMC3CHR(int V);
DECLFW(MMC3_CMDWrite);
DECLFW(MMC3_IRQWrite);

void GenMMC3_Init(CartInfo *info, int prg, int chr, int wram, int battery);

#endif /* _MMC3_H */
