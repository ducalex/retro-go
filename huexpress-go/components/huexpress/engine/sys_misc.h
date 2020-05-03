#ifndef _INCLUDE_SYS_MISC_H
#define _INCLUDE_SYS_MISC_H

/*
 * Miscellaneous section
 *
 * Here are function that don't belong to a set of function. It doesn't mean
 * they shouldn't be implemented but rather that they are important enough
 * to have their own section :)
 */

// declaration of the actual callback to call 60 times a second
uint32 interrupt_60hz(uint32, void*);

  /*
   * osd_wait_next_vsync
   *
   * Emulation speed throttling. It will sleep or skip frames in order to
   * maintain full speed emulation.
   */
void osd_wait_next_vsync(void);

  /*
   * osd_log
   *
   * Logging function, printf-style arguments
   */
void osd_log(const char *, ...);

#endif
