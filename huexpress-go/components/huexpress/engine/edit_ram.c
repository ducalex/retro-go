/*************************************************************************/
/*                                                                       */
/*                    Ram editor source file                             */
/*                                                                       */
/* A few functions to edit/view RAM as asked by Dave Shadoff ;)          */
/* Note that cheating functions can also be helpful                      */
/*                                                                       */
/*************************************************************************/

#include "edit_ram.h"

#define  NB_BYTE_LINE   8
// Number of byte displayed on each line

#define  NB_LINE        20
// Number of displayed lines

/*
static char out;
// To know whether we got to quit

static int frame_up, frame_down;
// The currently displayed "frame" in RAM

static unsigned short selected_byte;
// The current offset
*/

/*****************************************************************************

    Function:  change_value

    Description: change the value at selected_byte
    Parameters: none, use global selected_byte
    Return:nothing, but may modify the RAM

*****************************************************************************
void change_value()
{
 unsigned X=(((selected_byte&0x04)/4)*2+6+3*(selected_byte&0x07))*8+blit_x,Y=(selected_byte-frame_up)/NB_BYTE_LINE*10+blit_y-1;
 char out=0,index=0;
 char value[3]="\0\0";
 int ch;

 do {
 rectfill(screen,X,Y,X+16,Y+9,-15);
 textout(screen,font,value,X,Y+1,-1);
 ch=osd_readkey();

 // first switch by scancode
 switch (ch>>8)
   {
    case KEY_ESC:
                        out=1;
         break;
    case KEY_ENTER:
                        RAM[selected_byte]=cvtnum(value);
                        out=1;
                        break;
    case KEY_BACKSPACE:
                        if (index)
                          value[--index]=0;
         break;
   }

 // Now by ascii code
 switch (ch&0xff)
   {
    case '0' ... '9':
    case 'a' ... 'f':
    case 'A' ... 'F':
                        if (index<2)
                          value[index++]=toupper(ch&0xff);
         break;
   }
 } while (!out);

 return ;
 }

*/

/*****************************************************************************

    Function:  ram_key

    Description: handle the keyboard in edit_ram
    Parameters: none
    Return: nothing

*****************************************************************************/
void
ram_key()
{

	/* TODO: deallegroize here too */
#ifdef ALLEGRO
	int ch = osd_readkey();

	switch (ch >> 8) {
		// The two first are a bit special
	case KEY_HOME:
		selected_byte = 0;
		frame_up = 0;
		frame_down = NB_BYTE_LINE * NB_LINE;
		return;
	case KEY_END:
		selected_byte = 0x7FFF;
		frame_down = 0x8000;
		frame_up = frame_down - NB_BYTE_LINE * NB_LINE;
		return;
	case KEY_PGUP:
		if (selected_byte >= NB_BYTE_LINE * NB_LINE)
			selected_byte -= NB_BYTE_LINE * NB_LINE;
		break;
	case KEY_PGDN:
		if (selected_byte <= 0x7FFF - NB_BYTE_LINE * NB_LINE);
		selected_byte += NB_BYTE_LINE * NB_LINE;
		break;
	case KEY_UP:
		if (selected_byte >= NB_BYTE_LINE)
			selected_byte -= NB_BYTE_LINE;
		break;
	case KEY_DOWN:
		if (selected_byte <= 0x7FFF - NB_BYTE_LINE)
			selected_byte += NB_BYTE_LINE;
		break;
	case KEY_RIGHT:
		if (selected_byte < 0x7FFF)
			selected_byte++;
		break;
	case KEY_LEFT:
		if (selected_byte)
			selected_byte--;
		break;
	case KEY_SPACE:
		{
			uint32 dummy = RAM[selected_byte];

			change_value((((selected_byte & 0x04) / 4) * 2 + 6 +
						  3 * (selected_byte & 0x07)) * 8 + blit_x,
						 (selected_byte - frame_up) / NB_BYTE_LINE * 10 +
						 blit_y - 1, 2, &dummy);

			RAM[selected_byte] = (uchar) dummy;

		}
		break;
	case KEY_F12:
	case KEY_ESC:
		out = 1;
		break;
	}

	// Now ajust the frame
	if ((selected_byte < 3 * NB_BYTE_LINE) ||
		(selected_byte > 0x7FFF - 3 * NB_BYTE_LINE))
		return;

	if (selected_byte >= frame_down - 3 * NB_BYTE_LINE) {
		frame_down =
			MIN(0x8000, (selected_byte + 3 * NB_BYTE_LINE) & 0x7FF8);
		frame_up = frame_down - NB_BYTE_LINE * NB_LINE;
	}

	if (selected_byte < frame_up + 3 * NB_BYTE_LINE) {
		frame_up =
			MAX(0, (int) ((selected_byte - 3 * NB_BYTE_LINE) & 0x7FF8));
		frame_down = frame_up + NB_BYTE_LINE * NB_LINE;
	}

	return;

#endif

}

/*****************************************************************************

    Function: edit_ram

    Description: view or edit the RAM
    Parameters: none
    Return: nothing

*****************************************************************************/
void
edit_ram()
{

#ifdef ALLEGRO

	BITMAP *bg;
	uchar line, col;
	char *tmp_buf = (char *) alloca(100);
	unsigned short dum;

	bg = create_bitmap(vheight, vwidth);
	blit(screen, bg, 0, 0, 0, 0, vheight, vwidth);

	selected_byte = 0;
	out = 0;
	frame_up = 0;
	frame_down = frame_up + NB_LINE * NB_BYTE_LINE;

	while (!out) {
		clear(screen);
		for (line = 0; line < NB_LINE; line++) {
			sprintf(tmp_buf, "%04X", frame_up + line * NB_BYTE_LINE);
			textoutshadow(screen, font, tmp_buf, blit_x,
						  blit_y + 10 * line, -15, 2, 1, 1);
			for (col = 0; col < NB_BYTE_LINE / 2; col++) {
				if ((dum = frame_up + line * NB_BYTE_LINE + col) ==
					selected_byte)
					rectfill(screen, blit_x + (6 + col * 3) * 8,
							 blit_y + 10 * line - 1,
							 blit_x + (8 + col * 3) * 8,
							 blit_y + 10 * (line + 1) - 2, -15);
				sprintf(tmp_buf, "%02X", RAM[dum]);
				textoutshadow(screen, font, tmp_buf,
							  blit_x + (6 + col * 3) * 8,
							  blit_y + 10 * line, -1, 2, 1, 1);
			}
			for (; col < NB_BYTE_LINE; col++) {
				if ((dum = frame_up + line * NB_BYTE_LINE + col) ==
					selected_byte)
					rectfill(screen, blit_x + (8 + col * 3) * 8,
							 blit_y + 10 * line - 1,
							 blit_x + (10 + col * 3) * 8,
							 blit_y + 10 * (line + 1) - 2, -15);
				sprintf(tmp_buf, "%02X",
						RAM[frame_up + line * NB_BYTE_LINE + col]);
				textoutshadow(screen, font, tmp_buf,
							  blit_x + (8 + col * 3) * 8,
							  blit_y + 10 * line, -1, 2, 1, 1);
			}

		}

		ram_key();
		vsync();
	}

	blit(bg, screen, 0, 0, 0, 0, vheight, vwidth);
	destroy_bitmap(bg);
	return;

#endif

}
