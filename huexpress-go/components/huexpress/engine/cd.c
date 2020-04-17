#include "cd.h"

//---- Conversion functions --------------------------------------------------
unsigned
Time2Frame(int min, int sec, int frame)
{
	return (unsigned) (min * 60 * 75 + sec * 75 + frame);
}

unsigned
Time2HSG(int min, int sec, int frame)
{
	return Time2Frame(min, sec, frame) - 150;
}

unsigned
Time2Redbook(int min, int sec, int frame)
{
	return (unsigned) ((min << 16) | (sec << 8) | (frame));
}

void
Frame2Time(unsigned frame, int *Min, int *Sec, int *Fra)
{
	*Min = (int) (frame / (60 * 75));
	frame -= *Min * 60 * 75;
	*Sec = (int) (frame / 75);
	frame -= *Sec * 75;
	*Fra = (int) frame;
}

void
Redbook2Time(unsigned redbook, int *Min, int *Sec, int *Fra)
{
	*Fra = (int) (redbook & 0xff);
	*Sec = (int) ((redbook >> 8) & 0xff);
	*Min = (int) ((redbook >> 16) & 0xff);
}

void
HSG2Time(unsigned hsg, int *Min, int *Sec, int *Fra)
{
	Frame2Time(hsg + 150, Min, Sec, Fra);
}

unsigned
Redbook2HSG(unsigned redbook)
{
	int Min, Sec, Fra;
	Redbook2Time(redbook, &Min, &Sec, &Fra);
	return Time2HSG(Min, Sec, Fra);
}

unsigned
HSG2Redbook(unsigned HSG)
{
	int Min, Sec, Fra;
	HSG2Time(HSG, &Min, &Sec, &Fra);
	return Time2Redbook(Min, Sec, Fra);
}
