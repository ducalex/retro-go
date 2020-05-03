#ifndef _INCLUDE_SYS_KBD_H
#define _INCLUDE_SYS_KBD_H

#include "pce.h"

#define JOY_A       0x01
#define JOY_B       0x02
#define JOY_SELECT  0x04
#define JOY_RUN     0x08
#define JOY_UP      0x10
#define JOY_RIGHT   0x20
#define JOY_DOWN    0x40
#define JOY_LEFT    0x80

/*
 * Input section
 *
 * This one function part implements the input functionality
 * It's called every vsync I think, i.e. almost 60 times a sec
 */

/*!
 * Updates the Joypad variables, save/load game, make screenshots, display
 * gui or launch fileselector if asked
 * \return 1 if we must quit the emulation
 * \return 0 else
 */
int osd_keyboard(void);

  /*
   * osd_keypressed
   *
   * Behaves like kbhit, returning 0 is case no key have been pressed and a
   * non zero value if there's any key that can be read from osd_readkey
   */
char osd_keypressed(void);

  /*
   * osd_readkey
   *
   * Return info concerning the first keystroke, lower byte is the ascii code
   * while the higher byte contains the scancode of the key.
   * Once called, discard the value in the keystroke buffer
   */
uint16 osd_readkey(void);

/*!
 * Initialize the input services
 * \return 1 if the initialization failed
 * \return 0 on success
 */
int osd_init_input(void);

/*!
 * Shutdown input services
 */
void osd_shutdown_input(void);

/*!
 * Initialize the netplay support
 * \return 1 if the initialization failed
 * \return 0 on success
 */
int osd_init_netplay(void);

/*!
 * Shutdown netplay support
 */
void osd_shutdown_netplay(void);

#endif
