#ifndef _INCLUDE_OSD_H
#define _INCLUDE_OSD_H

#include "../engine/pce.h"

/* This header include all functionalities needed to be implemented in order
 * to make a port of Hu-Go! on various platforms
 */


/*
 * Gfx section
 *
 * Certainly one of the most important one, this one deals with all what can
 * be displayed.
 */

  /*
   * osd_gfx_buffer
   *
   * In order to make sprite and tiles functions generic to all ports, there's
   * a need for a linear pointer needed to access the graphic buffer.
   * This buffer will be the one used to display stuff on screen when calling
   * the osd_gfx_blit function.
   * I did this because e.g. when I use Allegro, I represent the buffer as a
   * XBuf BITMAP (it's an allegro type) and set the osd_gfx_buffer to
   * XBuf->line[0] since it's the first byte of the REAL data in this bitmap.
   * Its size must be OSD_GFX_WIDTH * OSD_GFX_HEIGHT
   */
extern uchar *osd_gfx_buffer;
extern uint osd_blitFrames;
extern uint osd_skipFrames;

extern void osd_gfx_init();
extern void osd_gfx_shutdown();
extern void osd_gfx_blit();
extern void osd_gfx_set_mode(short width, short height);
extern void osd_gfx_set_color(uchar index, uchar r, uchar g, uchar b);


/*
 * Input section
 *
 * This one function part implements the input functionality
 * It's called every vsync I think, i.e. almost 60 times a sec
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
 * Updates the Joypad variables, save/load game, make screenshots, display
 * gui or launch fileselector if asked
 * \return 1 if we must quit the emulation
 * \return 0 else
 */
extern int osd_keyboard(void);

  /*
   * osd_keypressed
   *
   * Behaves like kbhit, returning 0 is case no key have been pressed and a
   * non zero value if there's any key that can be read from osd_readkey
   */
extern char osd_keypressed(void);

  /*
   * osd_readkey
   *
   * Return info concerning the first keystroke, lower byte is the ascii code
   * while the higher byte contains the scancode of the key.
   * Once called, discard the value in the keystroke buffer
   */
extern uint16 osd_readkey(void);

/*!
 * Initialize the input services
 */
extern int osd_init_input(void);

/*!
 * Shutdown input services
 */
extern void osd_shutdown_input(void);

/*!
 * Initialize the netplay support
 * \return 1 if the initialization failed
 * \return 0 on success
 */
extern int osd_init_netplay(void);

/*!
 * Shutdown netplay support
 */
extern void osd_shutdown_netplay(void);



/*
 * Snd section
 *
 * This section gathers all sound replaying related function on the emulating machine.
 * We have to handle raw waveform (generated from hu-go! and internal pce state) and
 * if possible external format to add support for them in .hcd.
 */

	/*
	 * osd_snd_init
	 *
	 * Allocates ressources to output sound
	 */
extern void osd_snd_init();

	/*
	 * osd_snd_shutdown
	 *
	 * Frees all sound ressources allocated in osd_snd_init
	 */
extern void osd_snd_shutdown();




/*
 * Miscellaneous section
 *
 * Here are function that don't belong to a set of function. It doesn't mean
 * they shouldn't be implemented but rather that they are important enough
 * to have their own section :)
 */

// declaration of the actual callback to call 60 times a second
extern uint32 interrupt_60hz(uint32, void*);

  /*
   * osd_wait_next_vsync
   *
   * Emulation speed throttling. It will sleep or skip frames in order to
   * maintain full speed emulation.
   */
extern void osd_wait_next_vsync(void);

  /*
   * osd_log
   *
   * Logging function, printf-style arguments
   */
extern void osd_log(const char *, ...);


#endif
