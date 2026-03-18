#ifndef FDS_APU_H
#define FDS_APU_H

#include "fceu-types.h"

void FDSSoundReset(void);
void FDSSoundStateAdd(void);

/* Used for FDS conversion-based mappers to allow access to FDS APU registers */
void FDSSoundPower(void); 
uint8 FDSSoundRead(uint32 A);          /* $4040-$407F, $4090-$4092 */
void FDSSoundWrite(uint32 A, uint8 V); /* $4040-$407F, $4080-$408A */

DECLFR(FDSWaveRead);
DECLFR(FDSSRead);
DECLFW(FDSWaveWrite);
DECLFW(FDSSWrite);

#endif /* FDS_APU_H */
