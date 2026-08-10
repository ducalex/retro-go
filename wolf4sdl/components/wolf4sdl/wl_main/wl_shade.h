#if defined(USE_SHADING) && !defined(_WL_SHADE_H_)
#define _WL_SHADE_H_

#define SHADE_COUNT 32

#define LSHADE_NOSHADING 0xff
#define LSHADE_NORMAL 0
#define LSHADE_FOG 5

extern uint8_t shadetable[SHADE_COUNT][256];

void InitLevelShadeTable();
int GetShade(int scale);

#endif
