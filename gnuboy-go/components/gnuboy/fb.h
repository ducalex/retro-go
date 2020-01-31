

#ifndef __FB_H__
#define __FB_H__


#include "defs.h"



struct fb
{
	byte *ptr;
	int w, h;
	int pelsize;
	int pitch;
	int indexed;
	struct
	{
		int l, r;
	} cc[4];
	int yuv;
	int enabled;
	int dirty;
};


extern struct fb fb;


#endif




