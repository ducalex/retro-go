/*************************************************************************/
/*                                                                       */
/*				          Zero Page viewer source file                       */
/*                                                                       */
/* A few functions to view Zero page                                     */
/*                                                                       */
/*************************************************************************/

#include "view_zp.h"

#define  NB_BYTE_LINE   8
// Number of byte displayed on each line

#define  NB_LINE        20
// Number of displayed lines

#ifdef ALLEGRO
static char out;
// To know whether we got to quit

static int frame_up, frame_down;
// The currently displayed "frame" in RAM

static unsigned short selected_byte;
// The current offset
#endif


/*****************************************************************************

    Function:  zp_key

    Description: handle the keyboard in view_zp
    Parameters: none
    Return: nothing

*****************************************************************************/
void
zp_key ()
{
#ifdef ALLEGRO
  int ch = osd_readkey ();

  /* TODO: we can easily handle this without allegro defines */

  switch (ch >> 8)
    {
      // The two first are a bit special
    case KEY_HOME:
      selected_byte = 0;
      frame_up = 0;
      frame_down = NB_BYTE_LINE * NB_LINE;
      return;
    case KEY_END:
      selected_byte = 0xFF;
      frame_down = 0x100;
      frame_up = frame_down - NB_BYTE_LINE * NB_LINE;
      return;
    case KEY_PGUP:
      if (selected_byte >= NB_BYTE_LINE * NB_LINE)
	selected_byte -= NB_BYTE_LINE * NB_LINE;
      break;
    case KEY_PGDN:
      if (selected_byte <= 0xFF - NB_BYTE_LINE * NB_LINE);
      selected_byte += NB_BYTE_LINE * NB_LINE;
      break;
    case KEY_UP:
      if (selected_byte >= NB_BYTE_LINE)
	selected_byte -= NB_BYTE_LINE;
      break;
    case KEY_DOWN:
      if (selected_byte <= 0xFF - NB_BYTE_LINE)
	selected_byte += NB_BYTE_LINE;
      break;
    case KEY_RIGHT:
      if (selected_byte < 0xFF)
	selected_byte++;
      break;
    case KEY_LEFT:
      if (selected_byte)
	selected_byte--;
      break;
    case KEY_F12:
    case KEY_ESC:
      out = 1;
      break;
    }

  // Now ajust the frame
  if ((selected_byte < 3 * NB_BYTE_LINE) ||
      (selected_byte > 0xFF - 3 * NB_BYTE_LINE))
    return;

  if (selected_byte >= frame_down - 3 * NB_BYTE_LINE)
    {
      frame_down = MIN(0x100, (selected_byte + 3 * NB_BYTE_LINE) & 0xF8);
      frame_up = frame_down - NB_BYTE_LINE * NB_LINE;
    }

  if (selected_byte < frame_up + 3 * NB_BYTE_LINE)
    {
      frame_up = MAX(0, (int) ((selected_byte - 3 * NB_BYTE_LINE) & 0xF8));
      frame_down = frame_up + NB_BYTE_LINE * NB_LINE;
    }

  return;

#endif

}

/*****************************************************************************

    Function: view_zp

    Description: view the Zero Page
    Parameters: none
    Return: nothing

*****************************************************************************/
void
view_zp ()
{

#ifdef ALLEGRO
  BITMAP *bg;
  uchar line, col;
  char *tmp_buf = (char *) alloca (100);
  unsigned short dum;

  bg = create_bitmap (vheight, vwidth);
  blit (screen, bg, 0, 0, 0, 0, vheight, vwidth);

  selected_byte = 0;
  out = 0;
  frame_up = 0;
  frame_down = frame_up + NB_LINE * NB_BYTE_LINE;

  while (!out)
    {
      clear (screen);
      for (line = 0; line < NB_LINE; line++)
	{
	  sprintf (tmp_buf, "%04X", frame_up + line * NB_BYTE_LINE);
	  textoutshadow (screen, font, tmp_buf, blit_x, blit_y + 10 * line,
			 -15, 2, 1, 1);
	  for (col = 0; col < NB_BYTE_LINE / 2; col++)
	    {
	      if ((dum = frame_up + line * NB_BYTE_LINE + col) ==
		  selected_byte)
		rectfill (screen, blit_x + (6 + col * 3) * 8,
			  blit_y + 10 * line - 1, blit_x + (8 + col * 3) * 8,
			  blit_y + 10 * (line + 1) - 2, -15);
	      sprintf (tmp_buf, "%02X", Op6502 (0x2000 + dum));
	      textoutshadow (screen, font, tmp_buf,
			     blit_x + (6 + col * 3) * 8, blit_y + 10 * line,
			     -1, 2, 1, 1);
	    }
	  for (; col < NB_BYTE_LINE; col++)
	    {
	      if ((dum = frame_up + line * NB_BYTE_LINE + col) ==
		  selected_byte)
		rectfill (screen, blit_x + (8 + col * 3) * 8,
			  blit_y + 10 * line - 1, blit_x + (10 + col * 3) * 8,
			  blit_y + 10 * (line + 1) - 2, -15);
	      sprintf (tmp_buf, "%02X", Op6502 (0x2000 + dum));
	      textoutshadow (screen, font, tmp_buf,
			     blit_x + (8 + col * 3) * 8, blit_y + 10 * line,
			     -1, 2, 1, 1);
	    }

	}

      zp_key ();
      vsync ();
    }

  blit (bg, screen, 0, 0, 0, 0, vheight, vwidth);
  destroy_bitmap (bg);
  return;
#endif

}
