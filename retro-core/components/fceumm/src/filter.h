#ifndef _FCEU_FILTER_H
#define _FCEU_FILTER_H

int32 NeoFilterSound(int32 *in, int32 *out, uint32 inlen, int32 *leftover);
void MakeFilters(int32 rate);
void SexyFilter(int32 *in, int32 *out, int32 count);
void SexyFilter2(int32 *in, int32 count);

extern int64 sexyfilter_acc1, sexyfilter_acc2;

#endif
