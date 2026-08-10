#ifndef SDL_BLIT_H
#define SDL_BLIT_H

#include "SDL.h"
#include "SDL_video.h"

/** typedef for private surface blitting functions */
typedef int (*SDL_blit)(struct SDL_Surface *src, SDL_Rect *srcrect,
			struct SDL_Surface *dst, SDL_Rect *dstrect);

/* The structure passed to the low level blit functions */
typedef struct {
	Uint8 *s_pixels;
	int s_width;
	int s_height;
	int s_skip;
	Uint8 *d_pixels;
	int d_width;
	int d_height;
	int d_skip;
	void *aux_data;
	SDL_PixelFormat *src;
	Uint8 *table;
	SDL_PixelFormat *dst;
} SDL_BlitInfo;

/* The type definition for the low level blit functions */
typedef void (*SDL_loblit)(SDL_BlitInfo *info);

/* This is the private info structure for software accelerated blits */
struct private_swaccel {
	SDL_loblit blit;
	void *aux_data;
};


/* Blit mapping definition */
typedef struct SDL_BlitMap {
	void *dst; //SDL_Surfce
	int identity;
	Uint8 *table;
	SDL_blit hw_blit;
	SDL_blit sw_blit;
	struct private_hwaccel *hw_data;
	struct private_swaccel *sw_data;

	/* the version count matches the destination; mismatch indicates
	   an invalid mapping */
        unsigned int format_version;
} SDL_BlitMap;

#endif