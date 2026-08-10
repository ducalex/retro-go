//Wolf4SDL\DC
//dc_video.cpp
//2009 - Cyle Terry

#if defined(_arch_dreamcast)

#include <string.h>
#include <malloc.h>
#include "../wl_def.h"
#include "dc/biosfont.h"
#include "dc/video.h"

static uint16 *bbuffer;

void DC_VideoInit() {
    bbuffer = (uint16 *)malloc(640 * 480 * 2);
    DC_CLS();
}

void DC_DrawString(int x, int y, char *str) {
    bfont_draw_str(bbuffer + ((y + 1) * 24 * 640) + (x * 12), 640, 0, str);
}

void DC_Flip() {
    memcpy(vram_s, bbuffer, 640 * 480 * 2);
}

void DC_CLS() {
    int x, y, ofs;
    for(y = 0; y < 480; y++) {
        ofs = (640 * y);
        for(x = 0; x < 640; x++)
            bbuffer[ofs + x] = 0;
    }
}

#endif
