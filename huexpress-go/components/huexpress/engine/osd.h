#ifndef _INCLUDE_OSD_H
#define _INCLUDE_OSD_H

#include "pce.h"

/* This header include all functionalities needed to be implemented in order
 * to make a port of Hu-Go! on various platforms
 */


/*
 * Gfx section
 *
 * Certainly one of the most important one, this one deals with all what can
 * be displayed.
 */
extern uint8_t *osd_gfx_buffer;
extern uint osd_blitFrames;
extern uint osd_fullFrames;
extern uint osd_skipFrames;

extern void osd_gfx_init(void);
extern void osd_gfx_shutdown(void);
extern void osd_gfx_blit(void);
extern void osd_gfx_set_mode(int width, int height);
extern void osd_gfx_set_color(int index, uint8_t r, uint8_t g, uint8_t b);


/*
 * Input section
 *
 * This one function part implements the input functionality
 */
#define JOY_A       0x01
#define JOY_B       0x02
#define JOY_SELECT  0x04
#define JOY_RUN     0x08
#define JOY_UP      0x10
#define JOY_RIGHT   0x20
#define JOY_DOWN    0x40
#define JOY_LEFT    0x80

/*!
 * Read input devices (uses a bitmask of JOY_*)
 * It also handles quit/menu/etc.
 */
extern void osd_input_read(void);

/*!
 * Initialize the input services
 */
extern int osd_input_init(void);

/*!
 * Shutdown input services
 */
extern void osd_shutdown_input(void);


/*!
 * Netplay
 */
extern int osd_netplay_init(void);
extern void osd_netplay_shutdown(void);


/*
 * Sound section
 * The osd is responsible for calling snd_update() once per frame
 */
extern void osd_snd_init();
extern void osd_snd_shutdown();


/*
 * Miscellaneous section
 */

/*
* Emulation speed throttling. It will sleep or skip frames in order to
* maintain full speed emulation.
*/
extern void osd_wait_next_vsync(void);

/*
* Logging function, printf-style arguments
*/
extern void osd_log(const char *, ...);

/*
* Malloc function
*/
extern void* osd_alloc(size_t size);


#endif
