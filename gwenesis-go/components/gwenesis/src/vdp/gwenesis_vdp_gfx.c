/*
Gwenesis : Genesis & megadrive Emulator.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.
This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
this program. If not, see <http://www.gnu.org/licenses/>.

__author__ = "bzhxx"
__contact__ = "https://github.com/bzhxx"
__license__ = "GPLv3"

*/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "m68k.h"
#include "gwenesis_vdp.h"
#include "gwenesis_io.h"
#include "gwenesis_bus.h"
#include "gwenesis_savestate.h"

//#include <assert.h>

// #pragma GCC optimize("Ofast")


#if _HOST_
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
extern unsigned char VRAM[];
#else
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
// #include "stm32h7b0xx.h"
extern unsigned char* VRAM;
#endif

extern unsigned short CRAM[];            // CRAM - Palettes
extern unsigned char SAT_CACHE[]__attribute__((aligned(4)));        // Sprite cache
extern unsigned char gwenesis_vdp_regs[]; // Registers

//extern unsigned frame_count;

extern unsigned short CRAM565[];    // CRAM - Palettes

extern unsigned short VSRAM[];        // VSRAM - Scrolling

// Define screen buffers: original and scaled for host RGB
unsigned char *screen, *scaled_screen;

// Define screen buffers for embedded 565 format
static uint8_t *screen_buffer_line=0;
static uint8_t *screen_buffer=0;

    // Overflow is the maximum size we can draw outside to avoid
    // wasting time and code in clipping. The maximum object is a 4x4 sprite,
    // so 32 pixels (on both side) is enough.

enum { PIX_OVERFLOW = 32 };

static uint8_t render_buffer[SCREEN_WIDTH + PIX_OVERFLOW*2]; //__attribute__((section("._dtcram")));// __attribute__((aligned(4)));
static uint8_t sprite_buffer[SCREEN_WIDTH + PIX_OVERFLOW*2]; //_attribute__((section("._dtcram")));//  __attribute__((aligned(4)));


// Define VIDEO MODE
uint8_t mode_h40;
uint8_t mode_pal;

// Define screen W/H
int screen_width;
int screen_height;

int sprite_overflow;
bool sprite_collision;

// Window Plane and A plane spearation
static int base_w;
static int PlanA_firstcol;
static int PlanA_lastcol;

static int Window_firstcol;
static int Window_lastcol;

// 16 bits access to VRAM
#define FETCH16VRAM(A)  ( (VRAM[(A)+1]) | (VRAM[(A)] << 8) )


/******************************************************************************
 *
 *  set screen buffers in which the rendering occurs
 *  Set original and scaled screen buffer for host
 *
 ******************************************************************************/
//host
void gwenesis_vdp_set_buffers(unsigned char *screen_buffer, unsigned char *scaled_buffer)
{
    screen = screen_buffer;
    scaled_screen = scaled_buffer;
}
//embedded
void gwenesis_vdp_set_buffer(unsigned short *ptr_screen_buffer)
{
    screen_buffer_line = ptr_screen_buffer;
    screen_buffer = ptr_screen_buffer;
}

/******************************************************************************
 *
 *  Draw  Sprite character /8pixels in row
 *  without checking overdraw for pixels collision detection
 *  with Horizontal flip variation
 *  for Shadow/highlight :
 *    draw in fresh line buffer using draw_pattern_xxfliph_sprite(..)
 *  otherwise:
 *   draw over dirty planes using draw_pattern_xxfliph_sprite_over_planes(..)
 *
 ******************************************************************************/

 #define PIX0(P) ( ((P) & 0x000000F0 ) >>   4 )
 #define PIX1(P) ( ((P) & 0x0000000F ) >>   0 )
 #define PIX2(P) ( ((P) & 0x0000F000 ) >>  12 )
 #define PIX3(P) ( ((P) & 0x00000F00 ) >>   8 )
 #define PIX4(P) ( ((P) & 0x00F00000 ) >>  20 )
 #define PIX5(P) ( ((P) & 0x000F0000 ) >>  16 )
 #define PIX6(P) ( ((P) & 0xF0000000 ) >>  28 )
 #define PIX7(P) ( ((P) & 0x0F000000 ) >>  24 )

static inline __attribute__((always_inline))
void draw_pattern_nofliph_sprite(uint8_t *scr, uint32_t p, uint8_t attrs)
{
  if (p == 0) return;

  /*  not transparent pixel to write AND not already a sprite*/
  if (((PIX0(p))) && ((scr[0] & PIXATTR_SPRITE) == 0)) scr[0] = attrs | (PIX0(p));
  if (((PIX1(p))) && ((scr[1] & PIXATTR_SPRITE) == 0)) scr[1] = attrs | (PIX1(p));
  if (((PIX2(p))) && ((scr[2] & PIXATTR_SPRITE) == 0)) scr[2] = attrs | (PIX2(p));
  if (((PIX3(p))) && ((scr[3] & PIXATTR_SPRITE) == 0)) scr[3] = attrs | (PIX3(p));
  if (((PIX4(p))) && ((scr[4] & PIXATTR_SPRITE) == 0)) scr[4] = attrs | (PIX4(p));
  if (((PIX5(p))) && ((scr[5] & PIXATTR_SPRITE) == 0)) scr[5] = attrs | (PIX5(p));
  if (((PIX6(p))) && ((scr[6] & PIXATTR_SPRITE) == 0)) scr[6] = attrs | (PIX6(p));
  if (((PIX7(p))) && ((scr[7] & PIXATTR_SPRITE) == 0)) scr[7] = attrs | (PIX7(p));
}

static inline __attribute__((always_inline))
void draw_pattern_fliph_sprite(uint8_t *scr, uint32_t p, uint8_t attrs)
{
  if (p == 0) return;

  /*  not transparent pixel to write AND not already a sprite*/
  if (((PIX7(p))) && ((scr[0] & PIXATTR_SPRITE) == 0)) scr[0] = attrs | (PIX7(p));
  if (((PIX6(p))) && ((scr[1] & PIXATTR_SPRITE) == 0)) scr[1] = attrs | (PIX6(p));
  if (((PIX5(p))) && ((scr[2] & PIXATTR_SPRITE) == 0)) scr[2] = attrs | (PIX5(p));
  if (((PIX4(p))) && ((scr[3] & PIXATTR_SPRITE) == 0)) scr[3] = attrs | (PIX4(p));
  if (((PIX3(p))) && ((scr[4] & PIXATTR_SPRITE) == 0)) scr[4] = attrs | (PIX3(p));
  if (((PIX2(p))) && ((scr[5] & PIXATTR_SPRITE) == 0)) scr[5] = attrs | (PIX2(p));
  if (((PIX1(p))) && ((scr[6] & PIXATTR_SPRITE) == 0)) scr[6] = attrs | (PIX1(p));
  if (((PIX0(p))) && ((scr[7] & PIXATTR_SPRITE) == 0)) scr[7] = attrs | (PIX0(p));

}

static inline __attribute__((always_inline))
void draw_pattern_nofliph_sprite_over_planes(uint8_t *scr, uint32_t p, uint8_t attrs)
{
  if (p == 0) return; 

  /* High priority */
  if (attrs & PIXATTR_HIPRI) {

  /*  not transparent pixel to write AND not already a sprite*/
  if (((PIX0(p))) && ((scr[0] & PIXATTR_SPRITE) == 0)) scr[0] = attrs | (PIX0(p));
  if (((PIX1(p))) && ((scr[1] & PIXATTR_SPRITE) == 0)) scr[1] = attrs | (PIX1(p));
  if (((PIX2(p))) && ((scr[2] & PIXATTR_SPRITE) == 0)) scr[2] = attrs | (PIX2(p));
  if (((PIX3(p))) && ((scr[3] & PIXATTR_SPRITE) == 0)) scr[3] = attrs | (PIX3(p));
  if (((PIX4(p))) && ((scr[4] & PIXATTR_SPRITE) == 0)) scr[4] = attrs | (PIX4(p));
  if (((PIX5(p))) && ((scr[5] & PIXATTR_SPRITE) == 0)) scr[5] = attrs | (PIX5(p));
  if (((PIX6(p))) && ((scr[6] & PIXATTR_SPRITE) == 0)) scr[6] = attrs | (PIX6(p));
  if (((PIX7(p))) && ((scr[7] & PIXATTR_SPRITE) == 0)) scr[7] = attrs | (PIX7(p));

  }
  /* Low priority */
  else {

  /*  not transparent pixel to write AND not already a sprite or higher priority*/
  if (((PIX0(p))) && ((scr[0] & PIXATTR_SPRITE_HIPRI) == 0)) scr[0] = attrs | (PIX0(p));
  if (((PIX1(p))) && ((scr[1] & PIXATTR_SPRITE_HIPRI) == 0)) scr[1] = attrs | (PIX1(p));
  if (((PIX2(p))) && ((scr[2] & PIXATTR_SPRITE_HIPRI) == 0)) scr[2] = attrs | (PIX2(p));
  if (((PIX3(p))) && ((scr[3] & PIXATTR_SPRITE_HIPRI) == 0)) scr[3] = attrs | (PIX3(p));
  if (((PIX4(p))) && ((scr[4] & PIXATTR_SPRITE_HIPRI) == 0)) scr[4] = attrs | (PIX4(p));
  if (((PIX5(p))) && ((scr[5] & PIXATTR_SPRITE_HIPRI) == 0)) scr[5] = attrs | (PIX5(p));
  if (((PIX6(p))) && ((scr[6] & PIXATTR_SPRITE_HIPRI) == 0)) scr[6] = attrs | (PIX6(p));
  if (((PIX7(p))) && ((scr[7] & PIXATTR_SPRITE_HIPRI) == 0)) scr[7] = attrs | (PIX7(p));
  
  }
}

static inline __attribute__((always_inline))
void draw_pattern_fliph_sprite_over_planes(uint8_t *scr, uint32_t p, uint8_t attrs)
{
  if (p == 0) return;

  /* High priority */
  if (attrs & PIXATTR_HIPRI) {

  /*  not transparent pixel to write AND not already a sprite*/
  if (((PIX7(p))) && ((scr[0] & PIXATTR_SPRITE) == 0)) scr[0] = attrs | (PIX7(p));
  if (((PIX6(p))) && ((scr[1] & PIXATTR_SPRITE) == 0)) scr[1] = attrs | (PIX6(p));
  if (((PIX5(p))) && ((scr[2] & PIXATTR_SPRITE) == 0)) scr[2] = attrs | (PIX5(p));
  if (((PIX4(p))) && ((scr[3] & PIXATTR_SPRITE) == 0)) scr[3] = attrs | (PIX4(p));
  if (((PIX3(p))) && ((scr[4] & PIXATTR_SPRITE) == 0)) scr[4] = attrs | (PIX3(p));
  if (((PIX2(p))) && ((scr[5] & PIXATTR_SPRITE) == 0)) scr[5] = attrs | (PIX2(p));
  if (((PIX1(p))) && ((scr[6] & PIXATTR_SPRITE) == 0)) scr[6] = attrs | (PIX1(p));
  if (((PIX0(p))) && ((scr[7] & PIXATTR_SPRITE) == 0)) scr[7] = attrs | (PIX0(p));

  }
  /* Low priority */
  else {

  /*  not transparent pixel to write AND not already a sprite or higher priority*/
  if (((PIX1(p))) && ((scr[6] & PIXATTR_SPRITE_HIPRI) == 0)) scr[6] = attrs | (PIX1(p));
  if (((PIX7(p))) && ((scr[0] & PIXATTR_SPRITE_HIPRI) == 0)) scr[0] = attrs | (PIX7(p));
  if (((PIX6(p))) && ((scr[1] & PIXATTR_SPRITE_HIPRI) == 0)) scr[1] = attrs | (PIX6(p));
  if (((PIX5(p))) && ((scr[2] & PIXATTR_SPRITE_HIPRI) == 0)) scr[2] = attrs | (PIX5(p));
  if (((PIX4(p))) && ((scr[3] & PIXATTR_SPRITE_HIPRI) == 0)) scr[3] = attrs | (PIX4(p));
  if (((PIX3(p))) && ((scr[4] & PIXATTR_SPRITE_HIPRI) == 0)) scr[4] = attrs | (PIX3(p));
  if (((PIX2(p))) && ((scr[5] & PIXATTR_SPRITE_HIPRI) == 0)) scr[5] = attrs | (PIX2(p));
  if (((PIX0(p))) && ((scr[7] & PIXATTR_SPRITE_HIPRI) == 0)) scr[7] = attrs | (PIX0(p));
  
  }

}

/******************************************************************************
 *
 *  Draw  characters/8pixels in row
 *  without checking overdraw for pixels collision detection
 *  with Horizontal flip variation for plane A & B
 *
 ******************************************************************************/


static inline __attribute__((always_inline)) void
draw_pattern_nofliph_planeB(uint8_t *scr, uint32_t p, uint8_t attrs) {

  const uint8_t back = gwenesis_vdp_regs[7];

  if (p == 0) {

    scr[0] = back;
    scr[1] = back;
    scr[2] = back;
    scr[3] = back;
    scr[4] = back;
    scr[5] = back;
    scr[6] = back;
    scr[7] = back;

    return;
  }

  scr[0] = PIX0(p) ? attrs | (PIX0(p)) : back;
  scr[1] = PIX1(p) ? attrs | (PIX1(p)) : back;
  scr[2] = PIX2(p) ? attrs | (PIX2(p)) : back;
  scr[3] = PIX3(p) ? attrs | (PIX3(p)) : back;
  scr[4] = PIX4(p) ? attrs | (PIX4(p)) : back;
  scr[5] = PIX5(p) ? attrs | (PIX5(p)) : back;
  scr[6] = PIX6(p) ? attrs | (PIX6(p)) : back;
  scr[7] = PIX7(p) ? attrs | (PIX7(p)) : back;
}

static inline __attribute__((always_inline)) void
draw_pattern_fliph_planeB(uint8_t *scr, uint32_t p, uint8_t attrs) {

  const uint8_t back = gwenesis_vdp_regs[7];
  if (p == 0) {

    scr[0] = back;
    scr[1] = back;
    scr[2] = back;
    scr[3] = back;
    scr[4] = back;
    scr[5] = back;
    scr[6] = back;
    scr[7] = back;

    return;
  }

  scr[0] = PIX7(p) ? attrs | (PIX7(p)) : back;
  scr[1] = PIX6(p) ? attrs | (PIX6(p)) : back;
  scr[2] = PIX5(p) ? attrs | (PIX5(p)) : back;
  scr[3] = PIX4(p) ? attrs | (PIX4(p)) : back;
  scr[4] = PIX3(p) ? attrs | (PIX3(p)) : back;
  scr[5] = PIX2(p) ? attrs | (PIX2(p)) : back;
  scr[6] = PIX1(p) ? attrs | (PIX1(p)) : back;
  scr[7] = PIX0(p) ? attrs | (PIX0(p)) : back;


}

static inline __attribute__((always_inline)) void
draw_pattern_nofliph_planeAoverB(uint8_t *scr, uint32_t p, uint8_t attrs) {

  if (p == 0) return;

  if (attrs & PIXATTR_HIPRI) {

    if (PIX0(p)) scr[0] = attrs | (PIX0(p));
    if (PIX1(p)) scr[1] = attrs | (PIX1(p));
    if (PIX2(p)) scr[2] = attrs | (PIX2(p));
    if (PIX3(p)) scr[3] = attrs | (PIX3(p));
    if (PIX4(p)) scr[4] = attrs | (PIX4(p));
    if (PIX5(p)) scr[5] = attrs | (PIX5(p));
    if (PIX6(p)) scr[6] = attrs | (PIX6(p));
    if (PIX7(p)) scr[7] = attrs | (PIX7(p));

  } else {

    if (PIX0(p) && ((scr[0] & PIXATTR_HIPRI) == 0)) scr[0] = attrs | (PIX0(p));
    if (PIX1(p) && ((scr[1] & PIXATTR_HIPRI) == 0)) scr[1] = attrs | (PIX1(p));
    if (PIX2(p) && ((scr[2] & PIXATTR_HIPRI) == 0)) scr[2] = attrs | (PIX2(p));
    if (PIX3(p) && ((scr[3] & PIXATTR_HIPRI) == 0)) scr[3] = attrs | (PIX3(p));
    if (PIX4(p) && ((scr[4] & PIXATTR_HIPRI) == 0)) scr[4] = attrs | (PIX4(p));
    if (PIX5(p) && ((scr[5] & PIXATTR_HIPRI) == 0)) scr[5] = attrs | (PIX5(p));
    if (PIX6(p) && ((scr[6] & PIXATTR_HIPRI) == 0)) scr[6] = attrs | (PIX6(p));
    if (PIX7(p) && ((scr[7] & PIXATTR_HIPRI) == 0)) scr[7] = attrs | (PIX7(p));

  }
}

static inline __attribute__((always_inline)) void
draw_pattern_fliph_planeAoverB(uint8_t *scr, uint32_t p, uint8_t attrs) {

    if (p == 0) return;

    if (attrs & PIXATTR_HIPRI) {

    if (PIX7(p)) scr[0] = attrs | (PIX7(p));
    if (PIX6(p)) scr[1] = attrs | (PIX6(p));
    if (PIX5(p)) scr[2] = attrs | (PIX5(p));
    if (PIX4(p)) scr[3] = attrs | (PIX4(p));
    if (PIX3(p)) scr[4] = attrs | (PIX3(p));
    if (PIX2(p)) scr[5] = attrs | (PIX2(p));
    if (PIX1(p)) scr[6] = attrs | (PIX1(p));
    if (PIX0(p)) scr[7] = attrs | (PIX0(p));

  } else {

    if (PIX7(p) && ((scr[0] & PIXATTR_HIPRI) == 0)) scr[0] = attrs | (PIX7(p));
    if (PIX6(p) && ((scr[1] & PIXATTR_HIPRI) == 0)) scr[1] = attrs | (PIX6(p));
    if (PIX5(p) && ((scr[2] & PIXATTR_HIPRI) == 0)) scr[2] = attrs | (PIX5(p));
    if (PIX4(p) && ((scr[3] & PIXATTR_HIPRI) == 0)) scr[3] = attrs | (PIX4(p));
    if (PIX3(p) && ((scr[4] & PIXATTR_HIPRI) == 0)) scr[4] = attrs | (PIX3(p));
    if (PIX2(p) && ((scr[5] & PIXATTR_HIPRI) == 0)) scr[5] = attrs | (PIX2(p));
    if (PIX1(p) && ((scr[6] & PIXATTR_HIPRI) == 0)) scr[6] = attrs | (PIX1(p));
    if (PIX0(p) && ((scr[7] & PIXATTR_HIPRI) == 0)) scr[7] = attrs | (PIX0(p));

  }

}

/******************************************************************************
 *
 *  Draw  characters/8pixels in row
 *  with/without checking overdraw for pixels collision detection
 *  used for sprites and planes drawing
 *
 ******************************************************************************/
static inline __attribute__((always_inline))
void draw_pattern_sprite(uint8_t *scr, uint16_t name, int paty) {

  // uint16_t pat_addr = name << 5; //name * 32;
  // uint8_t pat_palette = BITS(name, 13, 2);
  // //unsigned int is_pat_pri = name & 0x8000;
  // uint8_t *pattern = VRAM + pat_addr;
 // uint8_t attrs = (pat_palette << 4) | ((name & 0x8000) ? PIXATTR_SPRITE_HIPRI : PIXATTR_SPRITE);
  uint8_t attrs = ( (name & 0x6000 ) >> 9 ) + ((name & 0x8000) >> 8) + PIXATTR_SPRITE;

  unsigned int  pattern;

  // Vertical flip ?
  // if (name & 0x1000)
  //   pattern += (7 - paty) * 4;
  // else
  //   pattern += paty * 4;

  // unsigned int  pattern;

  // Vertical flip ?
  if (name & 0x1000)
    pattern = *(unsigned int *)(VRAM + ((name & 0x07FF) << 5) + ((7 - paty) * 4)); //) pat_addr;
  else
    pattern = *(unsigned int *)(VRAM + ((name & 0x07FF) << 5) + (paty * 4));

  // Horizontal flip ?
  if (name & 0x0800)
    draw_pattern_fliph_sprite(scr, pattern, attrs);
  else
    draw_pattern_nofliph_sprite(scr, pattern, attrs);
}

static inline __attribute__((always_inline))
void draw_pattern_sprite_over_planes(uint8_t *scr, uint16_t name, int paty) {

  // uint16_t pat_addr = name << 5 ; //* 32;
  // int pat_palette = BITS(name, 13, 2);
  // int is_pat_pri = name & 0x8000;
  // uint8_t *pattern = VRAM + pat_addr;
  // uint8_t attrs = (pat_palette << 4) | (is_pat_pri ? PIXATTR_SPRITE_HIPRI : PIXATTR_SPRITE);

  // Vertical flip ?
  // if (name & 0x1000)
  //   pattern += (7 - paty) * 4;
  // else
  //   pattern += paty * 4;

  //uint8_t attrs = ( (name & 0x6000 ) >> 9 ) | ((name & 0x8000) ? PIXATTR_SPRITE_HIPRI : PIXATTR_SPRITE);
  uint8_t attrs = ( (name & 0x6000 ) >> 9 ) + ((name & 0x8000) >> 8) + PIXATTR_SPRITE;
  //uint8_t attrs = ( (name >>9) & 0x70 ) | PIXATTR_SPRITE;

  unsigned int  pattern;

  // Vertical flip ?
  if (name & 0x1000)
    pattern = *(unsigned int *)(VRAM + ((name & 0x07FF) << 5) + ((7 - paty) * 4)); //) pat_addr;
  else
    pattern = *(unsigned int *)(VRAM + ((name & 0x07FF) << 5) + (paty * 4));

  // Horizontal flip ?
  if (name & 0x0800)
    draw_pattern_fliph_sprite_over_planes (scr, pattern, attrs);
  else
    draw_pattern_nofliph_sprite_over_planes (scr, pattern, attrs);
}

static inline __attribute__((always_inline))
void draw_pattern_planeB(uint8_t *scr, uint16_t name, int paty) {
 // uint16_t pat_addr = name  << 5; // * 32;
 // uint8_t pat_palette = BITS(name, 13, 2);
 // unsigned int is_pat_pri = name & 0x8000;
  //uint8_t *pattern = VRAM + pat_addr;

  uint8_t attrs = ( (name & 0x6000 ) >> 9 ) + ((name & 0x8000) >> 8);

  unsigned int  pattern;

  // Vertical flip ?
  if (name & 0x1000)
    pattern = *(unsigned int *)(VRAM + ((name & 0x07FF) << 5) + ((7 - paty) * 4)); //) pat_addr;
  else
    pattern = *(unsigned int *)(VRAM + ((name & 0x07FF) << 5) + (paty * 4));

//  if ((*(unsigned int *)pattern) == 0 ) return;
 // uint8_t *pattern = VRAM + ((name << 5) & 0xFFFF); //) pat_addr;
  //uint32_t pattern = VRAM[(name & 0x07FF) << 5]; //) pat_addr;

  // Horizontal flip ?
  if (name & 0x0800)
    draw_pattern_fliph_planeB(scr, pattern, attrs);

  else
    draw_pattern_nofliph_planeB(scr, pattern, attrs);

}

static inline __attribute__((always_inline))
void draw_pattern_planeA(uint8_t *scr, uint16_t name, int paty) {
  // uint16_t pat_addr = name << 5; //* 32;
  // uint8_t pat_palette = BITS(name, 13, 2);
  // unsigned int is_pat_pri = name & 0x8000;
  // uint8_t *pattern = VRAM + pat_addr;

    uint8_t attrs = ( (name & 0x6000 ) >> 9 ) + ((name & 0x8000) >> 8);




  unsigned int  pattern;

  // Vertical flip ?
  // if (name & 0x1000)
  //   pattern += (7 - paty) * 4;
  // else
  //   pattern += paty * 4;

  // Vertical flip ?
  if (name & 0x1000)
    pattern = *(unsigned int *)(VRAM + ((name & 0x07FF) << 5) + ((7 - paty) * 4)); //) pat_addr;
  else
    pattern = *(unsigned int *)(VRAM + ((name & 0x07FF) << 5) + (paty * 4));

  // Horizontal flip ?
  if (name & 0x0800)
    draw_pattern_fliph_planeAoverB(scr, pattern, attrs);

  else
    draw_pattern_nofliph_planeAoverB(scr, pattern, attrs);

}

static  uint16_t ntwidth_x2;
static  uint16_t ntw_mask, nth_mask;

/******************************************************************************
 *
 *  Return the Horizontal scrolling
 *
 ******************************************************************************/

static inline __attribute__((always_inline))
unsigned int get_hscroll_vram(int line)
{

    int mode = REG11_HSCROLL_MODE;
    unsigned int table = REG13_HSCROLL_ADDRESS ;
    int idx;

    switch (mode)
    {
    case 0: // Full screen scrolling
        idx = 0;
        break;
    case 1: // First 8 lines
        idx = (line & 7);
        break;
    case 2: // Every row
        idx = (line & ~7);
        break;
    case 3: // Every line
        idx = line;
        break;
    }

    return table + idx*4;
}
/******************************************************************************
 *
 *  Render PLANE B on screen line
 *
 ******************************************************************************/
 //__attribute__((optimize("unroll-loops")))
static inline __attribute__((always_inline))
void draw_line_b(int line)
{
  uint8_t *scr  = &render_buffer[PIX_OVERFLOW];

  unsigned int ntaddr = REG4_NAMETABLE_B;
  uint16_t scrollx=FETCH16VRAM(get_hscroll_vram(line) + 2) & 0x3FF;
  uint16_t *vsram = &VSRAM[1];
  uint8_t *end = scr + screen_width;

  //bool column_scrolling = BIT(gwenesis_vdp_regs[11], 2);
  const unsigned int column_scrolling = gwenesis_vdp_regs[11] & 0x4;

  // Invert horizontal scrolling (because it goes right, but we need to offset
  // of the first screen pixel)
  scrollx = -scrollx;
  uint8_t col = (scrollx >> 3) & ntw_mask;
  uint8_t patx = scrollx & 7;

  unsigned int numcell = 0;
  scr -= patx;
  while (scr < end) {
    // Calculate vertical scrolling for the current line
    uint16_t scrolly = *vsram + line;
    uint8_t row = (scrolly >> 3) & nth_mask;
    uint8_t paty = scrolly & 7;

   // unsigned int nt = ntaddr + row * (2 * ntwidth);
    unsigned int nt = ntaddr + row * ntwidth_x2;

    draw_pattern_planeB(scr, FETCH16VRAM(nt + col * 2), paty);
    col = (col + 1) & ntw_mask;
    scr += 8;
    numcell++;

    // If per-column scrolling is active, increment VSRAM pointer
    if (column_scrolling && (numcell & 1) == 0)
      vsram += 2;
    }
}
/******************************************************************************
 *
 *  Render PLANE A and Window on screen line
 *
 ******************************************************************************/
//_attribute__((optimize("unroll-loops")))
static inline __attribute__((always_inline))
void draw_line_aw(int line) {

  uint8_t *scr  = &render_buffer[PIX_OVERFLOW];

  unsigned int ntaddr = REG2_NAMETABLE_A;
  uint16_t scrollx=FETCH16VRAM(get_hscroll_vram(line) + 0) & 0x3FF;
  uint16_t *vsram = &VSRAM[0];

  // Check if we are in the window region only
  // if it's the case, we cancel the plane A drawing
  int Window_line = REG18_WINDOW_VPOS * 8;
  //bool window_down = BIT(gwenesis_vdp_regs[18], 7);
  int window_down = gwenesis_vdp_regs[18] & 0x80;

  int PlanA_first = PlanA_firstcol;
  int PlanA_last = PlanA_lastcol;
  int Window_last = Window_lastcol;
  int Window_first = Window_firstcol;

  if (window_down) {
    if (line > Window_line) {
      PlanA_first = PlanA_last = 0;
      Window_last = screen_width;
      Window_first = 0;
    }
  } else {

    if (line < Window_line) {
      PlanA_first = PlanA_last = 0;
      Window_last = screen_width;
      Window_first = 0;
    }
  }

  // First draw A plane
  uint8_t *pos = scr + PlanA_first; // scr + screen_width;
  uint8_t *end = scr + PlanA_last;  // scr + screen_width

   //bool column_scrolling = BIT(gwenesis_vdp_regs[11], 2);
  const unsigned int column_scrolling = gwenesis_vdp_regs[11] & 0x4;

  // Invert horizontal scrolling (because it goes right, but we need to offset
  // of the first screen pixel)
  scrollx = -scrollx;
  uint8_t col = (scrollx >> 3) & ntw_mask;
  uint8_t patx = scrollx & 7;

  unsigned int numcell = 0;
  pos -= patx;
  while (pos < end) {
    // Calculate vertical scrolling for the current line
    uint16_t scrolly = *vsram + line;
    uint8_t row = (scrolly >> 3) & nth_mask;
    uint8_t paty = scrolly & 7;

   // unsigned int nt = ntaddr + row * (2 * ntwidth);
    unsigned int nt = ntaddr + row * ntwidth_x2;

    draw_pattern_planeA(pos, FETCH16VRAM(nt + col * 2), paty);

    col = (col + 1) & ntw_mask;
    pos += 8;
    numcell++;

    // If per-column scrolling is active, increment VSRAM pointer
    if (column_scrolling && (numcell & 1) == 0)
      vsram += 2;
  }

  // Second Draw Window Plane
  int row = line >> 3;
  int paty = line & 7;
  //int wdwidth = (screen_width == 320 ? 64 : 32);
  //unsigned int nt = base_w + row * 2 * wdwidth + Window_first / 4;

  int wdwidth_x2 = (screen_width == 320 ? 128 : 64);

  unsigned int nt = base_w + row * wdwidth_x2 + Window_first / 4;

  for (int i = Window_first / 8; i < Window_last / 8; ++i) {
    draw_pattern_planeA(end, FETCH16VRAM(nt), paty);
    nt += 2;
    end += 8;
  }
}

/******************************************************************************
 *
 *  Render SPRITES on screen line
 *
 ******************************************************************************/

//__attribute__((optimize("unroll-loops")))
static inline __attribute__((always_inline)) 
void draw_sprites_over_planes(int line)
{
    uint8_t *scr;

    scr = &render_buffer[PIX_OVERFLOW];

   // uint8_t mask = mode_h40 ? 0x7E : 0x7F;
   // uint8_t *start_table = VRAM + ((gwenesis_vdp_regs[5] & mask) << 9);

    uint8_t *start_table = VRAM + REG5_SAT_ADDRESS;

    // This is both the size of the table as seen by the VDP
    // *and* the maximum number of sprites that are processed
    // (important in case of infinite loops in links).
    const int SPRITE_TABLE_SIZE     = (screen_width == 320) ?  80 :  64;
    const int MAX_SPRITES_PER_LINE  = (screen_width == 320) ?  20 :  16;
    const int MAX_PIXELS_PER_LINE   = (screen_width == 320) ? 320 : 256;

    bool masking = false, one_sprite_nonzero = false; // overdraw = false;
    int sidx = 0, num_sprites = 0, num_pixels = 0;
    for (int i = 0; (i < SPRITE_TABLE_SIZE) && sidx < (SPRITE_TABLE_SIZE); ++i)
    {
        uint8_t *table = start_table + sidx*8;
        uint8_t *cache = SAT_CACHE + sidx*8;
        //uint8_t *cache = start_table + sidx*8;
        

        int sy = ((cache[0] & 0x3) << 8) | cache[1];
        int sx = ((table[6] & 0x3) << 8) | table[7];
        uint16_t name = (table[4] << 8) | table[5];


        int sh = BITS(cache[2], 0, 2) + 1;
        int link = BITS(cache[3], 0, 7);

        int isflipv = table[4] & 0x10;
        int isfliph = table[4] & 0x8;

        int sw = BITS(table[2], 2, 2) + 1;

        sy -= 128;
        if ((line >= sy) && (line < sy+sh*8))
        {
            // Sprite masking: a sprite on column 0 masks
            // any lower-priority sprite, but with the following conditions
            //   * it only works from the second visible sprite on each line
            //   * if the previous line had a sprite pixel overflow, it
            //     works even on the first sprite
            // Notice that we need to continue parsing the table after masking
            // to see if we reach a pixel overflow (because it would affect masking
            // on next line).
            if (sx == 0)
            {
                if (one_sprite_nonzero || (sprite_overflow == line-1))
                    masking = true;
            }
            else
                one_sprite_nonzero = true;

            int row = (line - sy) >> 3;
            int paty = (line - sy) & 7;
            if (isflipv)
                row = sh - row - 1;

            sx -= 128;
            if ((sx > (-sw * 8)) && (sx < screen_width) && !masking) {

              name += row;

              if (isfliph) {
                name += sh * (sw - 1);
                for (int p = 0; (p < sw) && (num_pixels < MAX_PIXELS_PER_LINE); p++) {

                  draw_pattern_sprite_over_planes(scr + sx + p * 8, name, paty);
                  name -= sh;
                  num_pixels += 8;

                }
              } else {
                for (int p = 0; (p < sw) && (num_pixels < MAX_PIXELS_PER_LINE); p++) {

                  draw_pattern_sprite_over_planes(scr + sx + p * 8, name, paty);
                  name += sh;
                  num_pixels += 8;

                }
              }
            }
            else
                num_pixels += sw*8;

            if (num_pixels >= MAX_PIXELS_PER_LINE)
            {
                sprite_overflow = line;
                break;
            }
            if (++num_sprites >= MAX_SPRITES_PER_LINE)
                break;
        }

        if (link == 0) break;
        sidx = link;
    }

  //  if (overdraw)
  //      sprite_collision = true;
}
static inline __attribute__((always_inline)) 
void draw_sprites(int line)
{
  uint8_t *scr;

  scr = &sprite_buffer[PIX_OVERFLOW];

  // uint8_t mask = mode_h40 ? 0x7E : 0x7F;
  // uint8_t *start_table = VRAM + ((gwenesis_vdp_regs[5] & mask) << 9);

  uint8_t *start_table = VRAM + REG5_SAT_ADDRESS;

  // This is both the size of the table as seen by the VDP
  // *and* the maximum number of sprites that are processed
  // (important in case of infinite loops in links).
  const int SPRITE_TABLE_SIZE = (screen_width == 320) ? 80 : 64;
  const int MAX_SPRITES_PER_LINE = (screen_width == 320) ? 20 : 16;
  const int MAX_PIXELS_PER_LINE = (screen_width == 320) ? 320 : 256;

  bool masking = false, one_sprite_nonzero = false; // overdraw = false;
  int sidx = 0, num_sprites = 0, num_pixels = 0;
  for (int i = 0; i < SPRITE_TABLE_SIZE && sidx < SPRITE_TABLE_SIZE; ++i) {
    uint8_t *table = start_table + sidx * 8;
    uint8_t *cache = start_table + sidx * 8;

    //uint8_t *cache = SAT_CACHE + sidx * 8;

    int sy = ((cache[0] & 0x3) << 8) | cache[1];
    int sx = ((table[6] & 0x3) << 8) | table[7];
    uint16_t name = (table[4] << 8) | table[5];

    int sh = BITS(cache[2], 0, 2) + 1;
    int link = BITS(cache[3], 0, 7);

    int isflipv = table[4] & 0x10;
    int isfliph = table[4] & 0x8;

    int sw = BITS(table[2], 2, 2) + 1;

    sy -= 128;
    if (line >= sy && line < sy + sh * 8) {
      // Sprite masking: a sprite on column 0 masks
      // any lower-priority sprite, but with the following conditions
      //   * it only works from the second visible sprite on each line
      //   * if the previous line had a sprite pixel overflow, it
      //     works even on the first sprite
      // Notice that we need to continue parsing the table after masking
      // to see if we reach a pixel overflow (because it would affect masking
      // on next line).
      if (sx == 0) {
        if (one_sprite_nonzero || sprite_overflow == line - 1)
          masking = true;
      } else
        one_sprite_nonzero = true;

      int row = (line - sy) >> 3;
      int paty = (line - sy) & 7;
      if (isflipv)
        row = sh - row - 1;

      sx -= 128;
      if (sx > -sw * 8 && sx < screen_width && !masking) {

        name += row;

        if (isfliph) {
          name += sh * (sw - 1);
          for (int p = 0; p < sw && num_pixels < MAX_PIXELS_PER_LINE; p++) {

            draw_pattern_sprite(scr + sx + p * 8, name, paty);
            name -= sh;
            num_pixels += 8;
          }
        } else {
          for (int p = 0; p < sw && num_pixels < MAX_PIXELS_PER_LINE; p++) {

            draw_pattern_sprite(scr + sx + p * 8, name, paty);
            name += sh;
            num_pixels += 8;
          }
        }
      } else
        num_pixels += sw * 8;

      if (num_pixels >= MAX_PIXELS_PER_LINE) {
        sprite_overflow = line;
        break;
      }
      if (++num_sprites >= MAX_SPRITES_PER_LINE)
        break;
    }

    if (link == 0)
      break;
    sidx = link;
    }

  //  if (overdraw)
  //      sprite_collision = true;
}
/******************************************************************************
 *
 *  Parse PLANE A/B size,scrolling at the start of image rendering
 *
 ******************************************************************************/
//static unsigned short current_line[320];

void gwenesis_vdp_render_config()
{
    mode_h40 = REG12_MODE_H40;
    mode_pal = REG1_PAL;

    int ntwidth = BITS(gwenesis_vdp_regs[16], 0, 2);
    int ntheight = BITS(gwenesis_vdp_regs[16], 4, 2);
    ntwidth = (ntwidth + 1) * 32;
    ntheight = (ntheight + 1) * 32;
    ntw_mask = ntwidth - 1;
    nth_mask = ntheight - 1;
    ntwidth_x2= ntwidth *2;

    // Window & A planes separation

    if (mode_h40)
        base_w = ((REG3_NAMETABLE_W & 0x1e) << 11);
    else
        base_w = ((REG3_NAMETABLE_W & 0x1f) << 11);


    bool window_right = BIT(gwenesis_vdp_regs[17], 7);

    // int window_is_bugged = 0;
    PlanA_firstcol = 0;
    PlanA_lastcol = screen_width;

    Window_firstcol = 0;
    Window_lastcol = 0;

    if (window_right) {

      Window_firstcol = REG17_WINDOW_HPOS * 16;
      Window_lastcol = screen_width;

      if (Window_firstcol > Window_lastcol)
        Window_firstcol = Window_lastcol;

      PlanA_firstcol = 0;
      PlanA_lastcol = Window_firstcol;

    } else {
      Window_firstcol = 0;
      Window_lastcol = REG17_WINDOW_HPOS * 16;
      if (Window_lastcol > screen_width)
        Window_lastcol = screen_width;

      PlanA_firstcol = Window_lastcol;
      PlanA_lastcol = screen_width;
      // if (Window_lastcol != 0)
      //      window_is_bugged = 1;
    }
}

/******************************************************************************
 *
 *  Render a line on screen
 *  Get selected line and render it on screen processing each plane.
 *
 ******************************************************************************/

void gwenesis_vdp_render_line(int line)
{
  mode_h40 = REG12_MODE_H40;
  mode_pal = REG1_PAL;

  //unsigned int line = scan_line;
  //  if (line == 0) gwenesis_vdp_render_config();

  // interlace mode not implemented
  if (BITS(gwenesis_vdp_regs[12], 1, 2) != 0)
    return;

  if (line >= (mode_pal ? 240 : 224))
    return;

#ifdef _HOST_
  memset(screen, 0, SCREEN_WIDTH * 4);

// Embedded RGB565
#else
  screen_buffer_line = &screen_buffer[line * 320];
  /* clean up line screen not refreshed when mode is !H40 */
  if (REG12_MODE_H40 == 0) memset(screen_buffer_line - (320-256)/2, 0, 320 * sizeof(screen_buffer_line[0]));

#endif

  if (REG0_DISABLE_DISPLAY)
  return;

#ifdef _HOST_
    memset(screen, 0, SCREEN_WIDTH * 4);
#else
 //   memset(screen_buffer_line, 0, 320 * 2);
#endif

  uint8_t *pb = &render_buffer[PIX_OVERFLOW];
  uint8_t *ps = &sprite_buffer[PIX_OVERFLOW];

  if (MODE_SHI)
    memset(ps, 0, 320);

  draw_line_b(line);
  draw_line_aw(line);

  if (MODE_SHI)
    draw_sprites(line);
  else
    draw_sprites_over_planes(line);

#ifdef _HOST_
  uint16_t rgb565;

  /* Mode Highlight/shadow is enabled */
  if (MODE_SHI) {
    for (int x = 0; x < screen_width; x++) {
      uint8_t plane = pb[x];
      uint8_t sprite = ps[x];

      if ((plane & 0xC0) < (sprite & 0xC0) ) {
        switch (sprite & 0x3F) {
        // Palette=3, Sprite=14 :> draw plane, force highlight
        case 0x3E:
          rgb565 = 0x8410 | CRAM565[plane] >> 1;
          break;
        // Palette=3, Sprite=15 :> draw plane, force shadow
        case 0x3F:
          rgb565 = CRAM565[plane] >> 1;
          break;
        // draw sprite, normal
        default:
          rgb565 = CRAM565[sprite];
          break;
        }
      } else {
        rgb565 = CRAM565[plane];
      }
      uint8_t r, g, b;
      r = (rgb565 & 0xF800) >> 8;
      g = (rgb565 & 0X07E0) >> 3;
      b = (rgb565 & 0x001F) << 3;
      int pixel = ((240 - screen_height) / 2 + (line)) * 320 + (x) +
                  (320 - screen_width) / 2;
      screen[pixel * 4 + 2] = r;
      screen[pixel * 4 + 1] = g;
      screen[pixel * 4 + 0] = b;
    }

    /* Normal mode*/
  } else {
    for (int x = 0; x < screen_width; x++) {
      rgb565 = CRAM565[pb[x]];
      uint8_t r, g, b;
      r = (rgb565 & 0xF800) >> 8;
      g = (rgb565 & 0X07E0) >> 3;
      b = (rgb565 & 0x001F) << 3;
      int pixel = ((240 - screen_height) / 2 + (line)) * 320 + (x) +
                  (320 - screen_width) / 2;
      screen[pixel * 4 + 2] = r;
      screen[pixel * 4 + 1] = g;
      screen[pixel * 4 + 0] = b;
    }
  }

  #else

  /* Mode Highlight/shadow is enabled */
  if (MODE_SHI) {
    for (int x = 0; x < screen_width; x++) {
      uint8_t plane = pb[x];
      uint8_t sprite = ps[x];

      if ((plane & 0xC0) < (sprite & 0xC0)) {
        switch (sprite & 0x3F) {
        // Palette=3, Sprite=14 :> draw plane, force highlight
        case 0x3E:
          screen_buffer_line[x] = plane; // 0x8410 | CRAM565[plane] >> 1;
          break;
        // Palette=3, Sprite=15 :> draw plane, force shadow
        case 0x3F:
          screen_buffer_line[x] = plane; // CRAM565[plane] >> 1;
          break;
        // draw sprite, normal
        default:
          screen_buffer_line[x] = sprite;
          break;
        }
      } else {
        screen_buffer_line[x] = plane;
      }
    }

    /* Normal mode*/
  } else {
#if 0
    uint32_t *video_out = (uint32_t *) &screen_buffer_line[0];

    for (int x = 0; x < screen_width; x+=2) {

      //screen_buffer_line[x] = CRAM565[pb[x]];
      // 2 pixels : 32 bits write  access is faster
      *video_out++ = CRAM565[pb[x]] | CRAM565[pb[x+1]] << 16;
    }
#else
  memcpy(screen_buffer_line, pb, screen_width);
#endif
  }

  #endif
}

void gwenesis_vdp_gfx_save_state() {
  SaveState* state;
  state = saveGwenesisStateOpenForWrite("vdp_gfx");
  saveGwenesisStateSetBuffer(state, "render_buffer", render_buffer, sizeof(render_buffer));
  saveGwenesisStateSetBuffer(state, "sprite_buffer", sprite_buffer, sizeof(sprite_buffer));
  saveGwenesisStateSet(state, "mode_h40", mode_h40);
  saveGwenesisStateSet(state, "mode_pal", mode_pal);
  saveGwenesisStateSet(state, "screen_width", screen_width);
  saveGwenesisStateSet(state, "screen_height", screen_height);
  saveGwenesisStateSet(state, "sprite_overflow", sprite_overflow);
  saveGwenesisStateSet(state, "sprite_collision", sprite_collision);
  saveGwenesisStateSet(state, "base_w", base_w);
  saveGwenesisStateSet(state, "PlanA_firstcol", PlanA_firstcol);
  saveGwenesisStateSet(state, "PlanA_lastcol", PlanA_lastcol);
  saveGwenesisStateSet(state, "Window_firstcol", Window_firstcol);
  saveGwenesisStateSet(state, "Window_lastcol", Window_lastcol);
}

void gwenesis_vdp_gfx_load_state() {
    SaveState* state = saveGwenesisStateOpenForRead("vdp_gfx");
    saveGwenesisStateGetBuffer(state, "render_buffer", render_buffer, sizeof(render_buffer));
    saveGwenesisStateGetBuffer(state, "sprite_buffer", sprite_buffer, sizeof(sprite_buffer));
    mode_h40 = saveGwenesisStateGet(state, "mode_h40");
    mode_pal = saveGwenesisStateGet(state, "mode_pal");
    screen_width = saveGwenesisStateGet(state, "screen_width");
    screen_height = saveGwenesisStateGet(state, "screen_height");
    sprite_overflow = saveGwenesisStateGet(state, "sprite_overflow");
    sprite_collision = saveGwenesisStateGet(state, "sprite_collision");
    base_w = saveGwenesisStateGet(state, "base_w");
    PlanA_firstcol = saveGwenesisStateGet(state, "PlanA_firstcol");
    PlanA_lastcol = saveGwenesisStateGet(state, "PlanA_lastcol");
    Window_firstcol = saveGwenesisStateGet(state, "Window_firstcol");
    Window_lastcol = saveGwenesisStateGet(state, "Window_lastcol");
}