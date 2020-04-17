#ifndef _INCLUDE_OSD_GFX_H
#define _INCLUDE_OSD_GFX_H
#include "cleantypes.h"
#include "pce.h"
#include "sys_dep.h"

#if 0

#include <SDL.h>
#include <SDL_endian.h>
#if defined(__APPLE__)
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

extern uchar *OSD_MESSAGE_SPR;

extern int blit_x,blit_y;// where must we blit the screen buffer on screen

int ToggleFullScreen(void);
void drawVolume(char* name, int volume);
void osd_gfx_glinit(struct generic_rect* viewport);
void osd_gfx_blit();

int osd_gfx_init();
int osd_gfx_init_normal_mode();
void osd_gfx_put_image_normal();
void osd_gfx_shut_normal_mode();
void osd_gfx_dummy_func();

#endif

#endif
