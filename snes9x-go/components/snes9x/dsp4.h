/* This file is part of Snes9x. See LICENSE file. */

#ifndef _DSP4_H_
#define _DSP4_H_

/* op control */
int8_t DSP4_Logic; /* controls op flow */

/* projection format */
const int16_t PLANE_START = 0x7fff; /* starting distance */

int16_t view_plane; /* viewer location */
int16_t far_plane;  /* next milestone into screen */
int16_t segments;   /* # raster segments to draw */
int16_t raster;     /* current raster line */

int16_t project_x; /* current x-position */
int16_t project_y; /* current y-position */

int16_t project_centerx; /* x-target of projection */
int16_t project_centery; /* y-target of projection */

int16_t project_x1;    /* current x-distance */
int16_t project_x1low; /* lower 16-bits */
int16_t project_y1;    /* current y-distance */
int16_t project_y1low; /* lower 16-bits */

int16_t project_x2; /* next projected x-distance */
int16_t project_y2; /* next projected y-distance */

int16_t project_pitchx;    /* delta center */
int16_t project_pitchxlow; /* lower 16-bits */
int16_t project_pitchy;    /* delta center */
int16_t project_pitchylow; /* lower 16-bits */

int16_t project_focalx; /* x-point of projection at viewer plane */
int16_t project_focaly; /* y-point of projection at viewer plane */

int16_t project_ptr; /* data structure pointer */

/* render window */
int16_t center_x;        /* x-center of viewport */
int16_t center_y;        /* y-center of viewport */
int16_t viewport_left;   /* x-left of viewport */
int16_t viewport_right;  /* x-right of viewport */
int16_t viewport_top;    /* y-top of viewport */
int16_t viewport_bottom; /* y-bottom of viewport */

/* sprite structure */
int16_t sprite_x;      /* projected x-pos of sprite */
int16_t sprite_y;      /* projected y-pos of sprite */
int16_t sprite_offset; /* data pointer offset */
int8_t sprite_type;    /* vehicle, terrain */
bool sprite_size;      /* sprite size: 8x8 or 16x16 */

/* path strips */
int16_t path_clipRight[4]; /* value to clip to for x>b */
int16_t path_clipLeft[4];  /* value to clip to for x<a */
int16_t path_pos[4];       /* x-positions of lanes */
int16_t path_ptr[4];       /* data structure pointers */
int16_t path_raster[4];    /* current raster */
int16_t path_top[4];       /* viewport_top */

int16_t path_y[2];     /* current y-position */
int16_t path_x[2];     /* current focals */
int16_t path_plane[2]; /* previous plane */

/* op09 window sorting */
int16_t multi_index1; /* index counter */
int16_t multi_index2; /* index counter */
bool op09_mode;       /* window mode */

/* multi-op storage */
int16_t multi_focaly[64];  /* focal_y values */
int16_t multi_farplane[4]; /* farthest drawn distance */
int16_t multi_raster[4];   /* line where track stops */

/* OAM */
int8_t op06_OAM[32]; /* OAM (size,MSB) data */
int8_t op06_index;   /* index into OAM table */
int8_t op06_offset;  /* offset into OAM table */
#endif
