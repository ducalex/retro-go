// osd.h - This header contains all functions that a port should implement
//
#pragma once

#include "pce.h"

#define JOY_A       0x01
#define JOY_B       0x02
#define JOY_SELECT  0x04
#define JOY_RUN     0x08
#define JOY_UP      0x10
#define JOY_RIGHT   0x20
#define JOY_DOWN    0x40
#define JOY_LEFT    0x80

/**
 * Init graphics (allocate buffers, etc)
 */
extern void osd_gfx_init(void);

/**
 * Shutdown graphics
 */
extern void osd_gfx_shutdown(void);

/**
 * Blit framebuffer to screen
 */
extern void osd_gfx_blit(void);

/**
 * Return active framebuffer to draw to
 * or NULL if we can't draw (frameskip, etc)
 */
extern uint8_t *osd_gfx_framebuffer(void);

/**
 * Change video resolution
 */
extern void osd_gfx_set_mode(int width, int height);

/**
 * Init input devices
 */
extern int osd_input_init(void);

/**
 * Shutdown input
 */
extern void osd_input_shutdown(void);

/**
 * Read input devices (uses a bitmask of JOY_*)
 * It also handles quit/menu/etc.
 */
extern void osd_input_read(void);

/**
 * Init Netplay
 */
extern int osd_netplay_init(void);

/**
 * Shutdown Netplay
 */
extern void osd_netplay_shutdown(void);

/**
 * Init sound
 */
extern void osd_snd_init();

/**
 * Shutdown sound
 */
extern void osd_snd_shutdown();

/*
* Emulation speed throttling. It will sleep or skip frames in order to
* maintain full speed emulation. It needs to be netplay-aware as well.
*/
extern void osd_vsync(void);

/*
* Logging function, printf-style arguments
*/
extern void osd_log(const char *, ...);

/*
* Malloc function
*/
extern void* osd_alloc(size_t size);
