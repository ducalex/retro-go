#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

int LoadState(const char *name);
int SaveState(const char *name);
void ResetPCE(bool);
void RunPCE(void);
void ShutdownPCE();
int InitPCE(int samplerate, bool stereo, const char *huecard);
int LoadCard(const char *name);
void *PalettePCE(int bitdepth);

#define JOY_A       0x01
#define JOY_B       0x02
#define JOY_SELECT  0x04
#define JOY_RUN     0x08
#define JOY_UP      0x10
#define JOY_RIGHT   0x20
#define JOY_DOWN    0x40
#define JOY_LEFT    0x80

// The extra 32's are linked to the way the sprite are drawn on screen, which can overlap to near memory
// If only one pixel is drawn in the screen, the whole sprite is written, which can eventually overlap unexpected area
// This could be fixed at the cost of performance but we don't need the extra memory
#define XBUF_WIDTH 	(352 + 32)
#define	XBUF_HEIGHT	(242 + 32)

extern void osd_gfx_blit(void);
extern uint8_t *osd_gfx_framebuffer(void);
extern void osd_gfx_set_mode(int width, int height);
extern void osd_input_read(uint8_t joypads[8]);
extern void osd_vsync(void);
extern void osd_log(int type, const char *, ...);
