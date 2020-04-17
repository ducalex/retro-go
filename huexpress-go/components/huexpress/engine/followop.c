#include "followop.h"

#if 0

uint16
follow_straight(uint16 where)
{
	uchar op = get_8bit_addr_(where);

	return where + addr_info_debug[optable_debug[op].addr_mode].size;
}

uint16
follow_BBRi(uint16 where)
{
	uchar op = get_8bit_addr_(where);
	uchar test_char = get_8bit_addr_(get_8bit_addr_((uint16) (where + 1)));
	uchar nb_bit = op >> 4;
	uchar size = addr_info_debug[optable_debug[op].addr_mode].size;

	test_char >>= nb_bit;

	if (!(test_char & 0x1))		// no jump
		return where + size;

	// jump
	return where + addr_info_debug[optable_debug[op].addr_mode].size
		+ (char) get_8bit_addr_((uint16) (where + size - 1));

}

uint16
follow_BCC(uint16 where)
{
	uchar op = get_8bit_addr_(where);
	uchar size = addr_info_debug[optable_debug[op].addr_mode].size;

	if (reg_p & FL_C)			// no jump
		return where + size;

	// jump
	return where + size +
		(char) get_8bit_addr_((uint16) (where + size - 1));
}

uint16
follow_BBSi(uint16 where)
{
	uchar op = get_8bit_addr_(where);
	uchar test_char = get_8bit_addr_(get_8bit_addr_((uint16) (where + 1)));
	uchar nb_bit = op >> 4;
	uchar size = addr_info_debug[optable_debug[op].addr_mode].size;

	test_char >>= nb_bit;

	if ((test_char & 0x1))		// no jump
		return where + size;

	// jump
	return where + addr_info_debug[optable_debug[op].addr_mode].size
		+ (char) get_8bit_addr_((uint16) (where + size - 1));

}

uint16
follow_BCS(uint16 where)
{
	uchar op = get_8bit_addr_(where);
	uchar size = addr_info_debug[optable_debug[op].addr_mode].size;

	if (!(reg_p & FL_C))		// no jump
		return where + size;

	// jump
	return where + size +
		(char) get_8bit_addr_((uint16) (where + size - 1));
}

uint16
follow_BEQ(uint16 where)
{
	uchar op = get_8bit_addr_(where);
	uchar size = addr_info_debug[optable_debug[op].addr_mode].size;

	if (reg_pc & FL_Z)			// no jump
		return where + size;

	// jump
	return where + size +
		(char) get_8bit_addr_((uint16) (where + size - 1));
}

uint16
follow_BNE(uint16 where)
{
	uchar op = get_8bit_addr_(where);
	uchar size = addr_info_debug[optable_debug[op].addr_mode].size;

	if (!(reg_pc & FL_Z))		// no jump
		return where + size;

	// jump
	return where + size +
		(char) get_8bit_addr_((uint16) (where + size - 1));
}

uint16
follow_BMI(uint16 where)
{
	uchar op = get_8bit_addr_(where);
	uchar size = addr_info_debug[optable_debug[op].addr_mode].size;

	if (reg_p & FL_N)			// no jump
		return where + size;

	// jump
	return where + size +
		(char) get_8bit_addr_((uint16) (where + size - 1));
}

uint16
follow_BPL(uint16 where)
{
	uchar op = get_8bit_addr_(where);
	uchar size = addr_info_debug[optable_debug[op].addr_mode].size;

	if (!(reg_p & FL_N))		// no jump
		return where + size;

	// jump
	return where + size +
		(char) get_8bit_addr_((uint16) (where + size - 1));
}

uint16
follow_BRA(uint16 where)
{
	uchar op = get_8bit_addr_(where);
	uchar size = addr_info_debug[optable_debug[op].addr_mode].size;

	return where + size			// always jump
		+ (char) get_8bit_addr_((uint16) (where + size - 1));
}

uint16
follow_BSR(uint16 where)
{
	uchar op = get_8bit_addr_(where);
	uchar size = addr_info_debug[optable_debug[op].addr_mode].size;

	if (running_mode == STEPPING)	// no jump
		return where + size;

	// jump
	return where + size +
		(char) get_8bit_addr_((uint16) (where + size - 1));
}

uint16
follow_BVC(uint16 where)
{
	uchar op = get_8bit_addr_(where);
	uchar size = addr_info_debug[optable_debug[op].addr_mode].size;

	if (reg_p & FL_V)			// no jump
		return where + size;

	// jump
	return where + size +
		(char) get_8bit_addr_((uint16) (where + size - 1));
}

uint16
follow_BVS(uint16 where)
{
	uchar op = get_8bit_addr_(where);
	uchar size = addr_info_debug[optable_debug[op].addr_mode].size;

	if (!(reg_p & FL_V))		// no jump
		return where + size;

	// jump
	return where + size +
		(char) get_8bit_addr_((uint16) (where + size - 1));
}

uint16
follow_JMPabs(uint16 where)
{
	uchar op = get_8bit_addr_(where);
	uchar size = addr_info_debug[optable_debug[op].addr_mode].size;

	return (uint16) (get_8bit_addr_((uint16) (where + size - 2)) +
					 256 * get_8bit_addr_((uint16) (where + size - 1)));

}

uint16
follow_JMPindir(uint16 where)
{
	uchar op = get_8bit_addr_(where);
	uchar size = addr_info_debug[optable_debug[op].addr_mode].size;

	uint16 indir =
		(uint16) (get_8bit_addr_((uint16) (where + size - 2)) +
				  256 * get_8bit_addr_((uint16) (where + size - 1)));

	return (uint16) (get_8bit_addr_((uint16) indir) +
					 256 * get_8bit_addr_((uint16) (indir + 1)));

}

uint16
follow_JMPindirX(uint16 where)
{
	uchar op = get_8bit_addr_(where);
	uchar size = addr_info_debug[optable_debug[op].addr_mode].size;

	uint16 indir =
		(uint16) (get_8bit_addr_((uint16) (where + size - 2)) +
				  256 * get_8bit_addr_((uint16) (where + size - 1)));

	indir += reg_x;

	return (uint16) (get_8bit_addr_((uint16) indir) +
					 256 * get_8bit_addr_((uint16) (indir + 1)));

}

uint16
follow_JSR(uint16 where)
{
	uchar op = get_8bit_addr_(where);
	uchar size = addr_info_debug[optable_debug[op].addr_mode].size;

	if (running_mode == STEPPING)
		return where + size;	// no jump

	return (uint16) (get_8bit_addr_((uint16) (where + size - 2)) +
					 256 * get_8bit_addr_((uint16) (where + size - 1)));

}

uint16
follow_RTI(uint16 where)
{
	return (uint16) (sp_base[reg_s + 2] + 256 * sp_base[reg_s + 3]);
}

uint16
follow_RTS(uint16 where)
{
	return (uint16) (sp_base[reg_s + 1] + 256 * sp_base[reg_s + 2] + 1);
}

#endif
