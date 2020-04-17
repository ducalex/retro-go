#ifndef _CD_H
#define _CD_H


#include "cleantypes.h"


extern char global_error[80];


//-- CDDA conversion functions -----------------------------------
unsigned Time2Frame(int min, int sec, int frame);
unsigned Time2HSG(int min, int sec, int frame);
unsigned Time2Redbook(int min, int sec, int frame);
void Frame2Time(unsigned frame, int *Min, int *Sec, int *Fra);
void Redbook2Time(unsigned redbook, int *Min, int *Sec, int *Fra);
void HSG2Time(unsigned hsg, int *Min, int *Sec, int *Fra);
unsigned Redbook2HSG(unsigned redbook);
unsigned HSG2Redbook(unsigned HSG);


#endif							/* _CD_H */
