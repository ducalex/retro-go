#ifndef _INCLUDE_SYS_GFX_H
#define _INCLUDE_SYS_GFX_H

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

   /*
    * osd_gfx_driver
    *
    * Structure defining an entry for a useable graphical plug in in Hu-Go!
    */
typedef struct {
	int (*init) (void);
	int (*mode) (void);
	void (*draw) (void);
	void (*shut) (void);
} osd_gfx_driver;

  /* 
   * osd_gfx_driver
   *
   * List of all driver (plug in) which can be used to render graphics
   */
extern osd_gfx_driver osd_gfx_driver_list[];


  /*
   * osd_gfx_set_color
   *
   * Set the 'index' color to components r,b,g
   */
void osd_gfx_set_color(uchar index, uchar r, uchar g, uchar b);

  /*
   * osd_gfx_savepict
   *
   * Saves the current screen bitmap, returns the numerical part of the
   * resulting filename
   */
uint16 osd_gfx_savepict(void);

	/*
	 * osd_gfx_set_message
	 *
	 * Display a message
	 */
void osd_gfx_set_message(char *message);
#endif
