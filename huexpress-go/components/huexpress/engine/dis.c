/****************************************************************************
 * tgdisasm.c -- Joseph LoCicero, IV -- 96/5/09
                 Dave Shadoff        -- 96/5/09
					  Zeograd             -- 99/5/25

    Simple TG Disassembler based on 65C02 opcode listing.

    Updates:
    --------
    Dave Shadoff  96/7/19  Place 'new' Hu6280 opcodes/addressing modes in,
                           due to new information available from Japanese
                           'Develo' system asembler.
    Zeograd       99/5/25  Modifications to be integrated in Hu-Go!
	                        for disassemblying on-fly

 ****************************************************************************/

#include "dis.h"

#if 0

/* GLOBALS */

uchar opbuf[OPBUF_SIZE];

uint16 init_pos;
// Initial adress to disassemble

uint16 selected_position;
// Adress user points to

uchar running_mode;
// the state we are running the cpu:
// 0 -> plain running
// 1 -> stepping, going over subroutines
// 2 -> tracing, diving into subroutines

/*****************************************************************************

    Function: forward_one_line

    Description: make the screen scroll down one line
    Parameters: none
    Return: none but set init_pos

*****************************************************************************/
void
forward_one_line()
{
	uchar op;

	op = get_8bit_addr(init_pos);
	if ((op & 0xF) == 0xB) {
		if (Bp_list[op >> 4].flag == BP_NOT_USED)
			init_pos++;
		else
			op = Bp_list[op >> 4].original_op;
	}

	init_pos += addr_info_debug[optable_debug[op].addr_mode].size;
	return;
}


/*****************************************************************************

    Function: backward_one_line

    Description: make the screen scroll one line up, al least, try ;)
    Parameters: none
    Return: nothing, but set init_pos

*****************************************************************************/
void
backward_one_line()
{
	char line;
	uchar Try;
	uint16 try_pos[MAX_TRY] = { 1, 2, 3, 4, 7 };
	unsigned short temp_pos = 0;
	char possible;
	uchar op, size, i;

	for (Try = 0; Try < MAX_TRY; Try++) {
		if (init_pos >= try_pos[Try])
			temp_pos = init_pos - try_pos[Try];
		line = 0;
		possible = 1;

		while ((line < NB_LINE) && (temp_pos != 0xFFFF)) {
			op = get_8bit_addr(temp_pos);

			if ((op & 0xF) == 0xB) {	// It's a breakpoint, we replace it and set the bp_color variable
				if ((Bp_list[op >> 4].flag == BP_ENABLED) ||
					(Bp_list[op >> 4].flag == BP_DISABLED))
					op = Bp_list[op >> 4].original_op;
			}

			size = addr_info_debug[optable_debug[op].addr_mode].size;

			opbuf[0] = op;
			temp_pos++;
			for (i = 1; i < size; i++)
				opbuf[i] = get_8bit_addr(temp_pos++);

			if (((op & 0xF) == 0xB) || (!strcmp(optable_debug[op].opname, "???"))) {	// If it's still a breakpoint, it wasn't set and shouldn't be here
				possible = 0;
				break;
			}

			line++;
		}

		if (possible) {
			init_pos -= try_pos[Try];
			return;
		}

	}

	// We haven't found any good result then ...

	init_pos -= try_pos[1];
	return;
}


/*****************************************************************************

    Function: dis_key

    Description: handle the keyboard in the disassemble function
    Parameters: none
    Return: 0 if can go on
            1 if we must quit the dis function
            may set a few global variable

*****************************************************************************/
int
dis_key()
{

/* TODO: deallegroization needed here */
#ifdef ALLEGRO
	int ch = osd_readkey();
	switch (ch >> 8) {
	case KEY_DOWN:
		selected_position +=
			addr_info_debug[optable_debug[Read8(selected_position)].
							addr_mode].size;
		forward_one_line();
		return 0;

	case KEY_UP:
		backward_one_line();
		selected_position = init_pos;
		return 0;

	case KEY_PGDN:
		{
			char dum;
			for (dum = 1; dum < NB_LINE; dum++)
				forward_one_line();
			// Let one line from the previous display
		}
		return 0;
	case KEY_PGUP:
		{
			char dum;
			for (dum = 1; dum < NB_LINE; dum++)
				backward_one_line();
		}
		return 0;
	case KEY_F1:				/* F1 */
		display_debug_help();
		return 0;
	case KEY_F2:				/* F2 */
		toggle_user_breakpoint(selected_position);
		return 0;

	case KEY_F3:				/* F3 */
		{
			char *tmp_buf = (char *) alloca(20);
			uchar index = 0;

			while (osd_keypressed())
				osd_readkey();

			while ((index < 20)
				   && (tmp_buf[index++] = osd_readkey() & 0xFF) != 13);

			tmp_buf[index - 1] = 0;

			// Just in case :
			if (Bp_list[GIVE_HAND_BP].flag != BP_NOT_USED) {
				_Write8(Bp_list[GIVE_HAND_BP].position,
						Bp_list[GIVE_HAND_BP].original_op);
				Bp_list[GIVE_HAND_BP].flag = BP_NOT_USED;
			}

			save_background = 0;

			Bp_list[GIVE_HAND_BP].flag = BP_ENABLED;
			Bp_list[GIVE_HAND_BP].position = cvtnum(tmp_buf);
			Bp_list[GIVE_HAND_BP].original_op = Read8(cvtnum(tmp_buf));

			_Write8(cvtnum(tmp_buf), 0xB + 0x10 * GIVE_HAND_BP);
			// Put an invalid opcode
		}
		return 1;

	case KEY_F4:				/* F4 */

		// Just in case :
		if (Bp_list[GIVE_HAND_BP].flag != BP_NOT_USED) {
			_Write8(Bp_list[GIVE_HAND_BP].position,
					Bp_list[GIVE_HAND_BP].original_op);
			Bp_list[GIVE_HAND_BP].flag = BP_NOT_USED;
		}

		save_background = 0;

		Bp_list[GIVE_HAND_BP].flag = BP_ENABLED;
		Bp_list[GIVE_HAND_BP].position = selected_position;
		Bp_list[GIVE_HAND_BP].original_op = Read8(selected_position);

		_Write8(selected_position, 0xB + 0x10 * GIVE_HAND_BP);
		// Put an invalid opcode

		return 1;

	case KEY_F5:				/* F5 */
		reg_pc = selected_position;
		return 0;

	case KEY_F6:				/* F6 */
		{
			char dum;
			uchar op = Read8(selected_position);

			if ((op & 0xF) == 0xB)
				op = Bp_list[op >> 4].original_op;

			for (dum = 0;
				 dum < addr_info_debug[optable_debug[op].addr_mode].size;
				 dum++)
				Write8(selected_position + dum, 0xEA);

		}
		return 0;

	case KEY_F7:				/* F7 */
		// Just in case :
		if (Bp_list[GIVE_HAND_BP].flag != BP_NOT_USED) {
			_Write8(Bp_list[GIVE_HAND_BP].position,
					Bp_list[GIVE_HAND_BP].original_op);
			Bp_list[GIVE_HAND_BP].flag = BP_NOT_USED;
		}

		running_mode = TRACING;
		set_bp_following(reg_pc, GIVE_HAND_BP);

		save_background = 0;

		return 1;

	case KEY_F8:				/* F8 */
		// Just in case :
		if (Bp_list[GIVE_HAND_BP].flag != BP_NOT_USED) {
			_Write8(Bp_list[GIVE_HAND_BP].position,
					Bp_list[GIVE_HAND_BP].original_op);
			Bp_list[GIVE_HAND_BP].flag = BP_NOT_USED;
		}

		running_mode = STEPPING;

		set_bp_following(reg_pc, GIVE_HAND_BP);
		save_background = 0;

		return 1;

	case KEY_R:
		edit_ram();
		return 0;

	case KEY_Z:
		view_zp();
		return 0;

	case KEY_I:
		view_info();
		return 0;

	case KEY_G:

		{
			char *tmp_buf = (char *) alloca(20);
			uchar index = 0;

			while (osd_keypressed())
				osd_readkey();

			while ((index < 20)
				   && (tmp_buf[index++] = osd_readkey() & 0xFF) != 13);

			tmp_buf[index - 1] = 0;

			init_pos = cvtnum(tmp_buf);

		}
		return 0;

	case KEY_ASTERISK:			/* dirty trick to avoid dis from going immediately */
		running_mode = PLAIN_RUN;
		return (!key_delay);

	case KEY_ESC:
	case KEY_F12:
		running_mode = PLAIN_RUN;
		return 1;
	}

#endif

	return 0;
}

/*****************************************************************************

    Function: disassemble

    Description: errr... , disassemble at current PC
    Parameters: none
    Return: 0 for now

*****************************************************************************/
int
disassemble()
{

#ifdef ALLEGRO

	char linebuf[256];
	uchar op;
	int i, size;
	char line;
	uint16 position;
	char bp_actived, bp_disabled;
	char *tmp_buf = (char *) alloca(100);

	save_background = 1;
	selected_position = init_pos = reg_pc;

	key_delay = 10;

	do {
		position = init_pos;
		line = 0;

		clear(screen);

		while (line < NB_LINE) {

			bp_actived = 0;
			bp_disabled = 0;

			op = Read8(position);
			if ((op & 0xF) == 0xB) {	// It's a breakpoint, we replace it and set the bp_* variable
				if (Bp_list[op >> 4].flag == BP_ENABLED)
					bp_actived = 1;
				else if (Bp_list[op >> 4].flag == BP_DISABLED)
					bp_disabled = 1;
				else
					break;
				op = Bp_list[op >> 4].original_op;
			}


			size = addr_info_debug[optable_debug[op].addr_mode].size;

			//if (size+position>0xFFFF)
			//  size=0xFFFF-position;
			// Make parsing wraps at the end of the memory


			opbuf[0] = op;
			position++;
			for (i = 1; i < size; i++)
				opbuf[i] = Read8(position++);

			/* This line is the real 'meat' of the disassembler: */

			(*addr_info_debug[optable_debug[op].addr_mode].func)	/* function      */
				(linebuf, position - size, opbuf, optable_debug[op].opname);	/* parm's passed */
			// now, what we got to display is in linebuf

			if (bp_actived)
				rectfill(screen, blit_x, blit_y + 10 * line - 1,
						 blit_x + WIDTH, blit_y + 10 * (line + 1) - 3, 25);
			else if (bp_disabled)
				rectfill(screen, blit_x, blit_y + 10 * line - 1,
						 blit_x + WIDTH, blit_y + 10 * (line + 1) - 3, 26);

			sprintf(tmp_buf, "%04X", position - size);
			if (position - size == selected_position) {
				textoutshadow(screen, font, tmp_buf, blit_x,
							  blit_y + 10 * line, -15, 2, 1, 1);
				textoutshadow(screen, font, linebuf, blit_x + 5 * 8,
							  blit_y + 10 * line, -10, 2, 1, 1);
			}
			else if (position - size != reg_pc)
			{
				textoutshadow(screen, font, tmp_buf, blit_x,
							  blit_y + 10 * line, -5, 0, 1, 1);
				textoutshadow(screen, font, linebuf, blit_x + 5 * 8,
							  blit_y + 10 * line, -1, 0, 1, 1);
			} else {
				textoutshadow(screen, font, tmp_buf, blit_x,
							  blit_y + 10 * line, 3, 2, 1, 1);
				textoutshadow(screen, font, linebuf, blit_x + 5 * 8,
							  blit_y + 10 * line, 3, 2, 1, 1);
			}

			// fprintf(stderr, "%s\n", linebuf);

			line++;

		}						/* End of while read loop */
	}
	while (!dis_key());

#endif

	return (0);					/* return value to appease compiler */
}

#endif
