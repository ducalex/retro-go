/****************************************************************************/
/*                                                                          */
/*	          Source file for handling Bp in core kernel                */
/*									    */
/* For the user Bp, we just give the hand to the user                       */
/* For the GIVE_HAND_BP, we make it disappear and we give hand to user      */
/* For the RESTORE_BP, we set again the User BP placed old_user_bp in the   */
/* list then we make the current BP to disappear then set IP,...            */
/*                                                                          */
/* There are certainly a lot of improvement to do, If you got any idea :    */
/*           Zeograd@caramail.com                                           */
/*									    */
/****************************************************************************/

#include "globals.h"
#include "debug.h"
#include "h6280.h"

int handle_bp(int nb_bp);

int
handle_bp0()
{
	return handle_bp(0);
}

int
handle_bp1()
{
	return handle_bp(1);
}

int
handle_bp2()
{
	return handle_bp(2);
}

int
handle_bp3()
{
	return handle_bp(3);
}

int
handle_bp4()
{
	return handle_bp(4);
}

int
handle_bp5()
{
	return handle_bp(5);
}

int
handle_bp6()
{
	return handle_bp(6);
}

int
handle_bp7()
{
	return handle_bp(7);
}

int
handle_bp8()
{
	return handle_bp(8);
}

int
handle_bp9()
{
	return handle_bp(9);
}

int
handle_bp10()
{
	return handle_bp(10);
}

int
handle_bp11()
{
	return handle_bp(11);
}

int
handle_bp12()
{
	return handle_bp(12);
}

int
handle_bp13()
{
	return handle_bp(13);
}


#if 1

int
handle_bp14()
{
    return handle_bp(13);
}

int
handle_bp15()
{
    return handle_bp(13);
}

int
handle_bp(int nb_bp)
{
    printf("handle_bp: not implemented\n");
    abort();
    return 0;
}


#else

int
handle_bp(int nb_bp)
{

#ifndef FINAL_RELEASE
	if (reg_pc != Bp_list[nb_bp].position)
		fprintf(stderr,
				"there's a problem, the breakpoint hasn't been correctly hit\n");
	else
		fprintf(stderr, "The breakpoint %d has been correctly hit\n",
				nb_bp);

	fprintf(stderr, "After Breakpoint, position is %X\n", reg_pc);
#endif

	disass_menu(reg_pc);
	// And call the disassembler

#ifndef FINAL_RELEASE
	fprintf(stderr, "After the disassembly function, the position is %X\n",
			reg_pc);
#endif

	if ((get_8bit_addr_(reg_pc) & 0x0F) == 0x0B) {	// We only look here for Bp since PC or bp status can have changed

#ifndef FINAL_RELEASE
		fprintf(stderr, "run trick: a bp has been asked to be put at %X\n",
				reg_pc);
#endif

		Wr6502(reg_pc, Bp_list[get_8bit_addr_(reg_pc) >> 4].original_op);
		// Replace the opcode in the rom

		Bp_list[get_8bit_addr_(reg_pc) >> 4].flag = NOT_USED;
		// Temporary, the breakpoint disappears
		// to be replaced by the restore_bp

		Bp_pos_to_restore = reg_pc;
		// We know we must restore a breakpoint at this point

		set_bp_following(reg_pc, RESTORE_BP);
		// since we call this after disassembly call, we handle correctly
		// any changes in reg value e.g.

	}

	return 0;

}

int
handle_bp14()
{
	// We must restore the Bp_to_restore Breakpoint
	Wr6502(reg_pc, Bp_list[14].original_op);

	// Replace the opcode in the rom

	Bp_list[14].flag = NOT_USED;
	// This BP is no more used

#ifndef FINAL_RELEASE
	fprintf(stderr, "We're restoring bp at %X\n", Bp_pos_to_restore);
#endif

	toggle_user_breakpoint(Bp_pos_to_restore);
	// We set another Bp at the location we just left

	return 0;
}

int
handle_bp15()
{
	// We must make it disappear and call the disassembler

	Wr6502(reg_pc, Bp_list[15].original_op);
	// Replace the opcode in the rom

	Bp_list[15].flag = NOT_USED;
	// This breakpoint is no more used

	disass_menu(reg_pc);

	return 0;
}

#endif
