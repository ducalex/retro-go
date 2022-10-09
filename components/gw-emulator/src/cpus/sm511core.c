// license:BSD-3-Clause
// copyright-holders:hap
// modded by bzhxx

/*

  Sharp SM511 MCU core implementation

  TODO:
  - verify undocumented/guessed opcodes:
    * $01 is guessed as DIV to ACC transfer, unknown which bits
    * $5d is certainly CEND
    * $65 is certainly divider reset, but not sure if it behaves same as on SM510
    * $6036 and $6037 may be instruction timing? (16kHz and 8kHz), mnemonics unknown

*/
#include <assert.h>

#include "gw_type_defs.h"
#include "sm510.h"

un8 *melody_rom;

//-------------------------------------------------
//  melody controller
//-------------------------------------------------
static inline void sm511_clock_melody()
{
	if (!melody_rom)
		return;

	// tone cycle table (SM511/SM512 datasheet fig.5)
	// cmd 0 = rest, 1 = stop, > 13 = illegal(inactive?)
	static const u8 lut_tone_cycles[4*16] =
	{
		0, 0, 7, 8, 8, 9, 9, 10,11,11,12,13,14,14, 0, 0,
		0, 0, 8, 8, 9, 9, 10,11,11,12,13,13,14,15, 0, 0,
		0, 0, 8, 8, 9, 9, 10,10,11,12,12,13,14,15, 0, 0,
		0, 0, 8, 9, 9, 10,10,11,11,12,13,14,14,15, 0, 0,
	};

	u8 cmd = melody_rom[m_melody_address] & 0x3f;
	u8 out = 0;

	// clock duty cycle if tone is active
	if ((cmd & 0xf) >= 2 && (cmd & 0xf) <= 13)
	{
		out = m_melody_duty_index & m_melody_rd & 1;
		m_melody_duty_count++;
		int index = m_melody_duty_index << 4 | (cmd & 0xf);
		int shift = ~cmd >> 4 & 1; // OCT

		if (m_melody_duty_count >= (lut_tone_cycles[index] << shift))
		{
			m_melody_duty_count = 0;
			m_melody_duty_index = (m_melody_duty_index + 1) & 3;
		}
	}
	else if ((cmd & 0xf) == 1)
	{
		// set melody stop flag
		m_melody_rd |= 2;
	}

	// clock time base on F8(d7)
	if ((m_div & 0x7f) == 0)
	{
		u8 mask = (cmd & 0x20) ? 0x1f : 0x0f;
		m_melody_step_count = (m_melody_step_count + 1) & mask;

		if (m_melody_step_count == 0)
			m_melody_address++;
	}

	m_r_out = out;
	m_write_r(m_r_out);
}

bool sm511_init_melody(un8 *gw_melody)
{
	if (!gw_melody)
		return false;

	melody_rom = gw_melody;

	// verify melody rom
	for (int i = 0; i < 0x100; i++)
	{
		u8 data = melody_rom[i];
		if (data & 0xc0 || (data & 0x0f) > 13)
			return false;
	}
	return true;
}
//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void sm511_device_reset()
{
	sm510_device_reset();

	m_melody_rd &= ~1;
	m_clk_div = 4; // 8kHz
}

/********** 1 second timer *********/
static inline void sm511_div_timer_cb()
{
	m_div = (m_div + 1) & 0x7fff;

	// 1S signal on overflow(falling edge of F1)
	if (m_div == 0){

		m_1s = true;

		/* refresh LCD segments state every seconds under sleep mode */
		if (m_halt)	sm510_update_segments_state();

	}


	sm511_clock_melody();
}

/********** 1 second timer *********/
static inline void sm511_div_timer(int nb_inst)
{
	if (nb_inst > 0)
		for (int toctoc=0; toctoc < m_clk_div*nb_inst; toctoc++ )
			sm511_div_timer_cb();
}


void sm511_execute_one()
{
	switch (m_op & 0xf0)
	{
		case 0x20: sm510_op_lax(); break;
		case 0x30: sm510_op_adx(); break;
		case 0x40: sm510_op_lb(); break;
		case 0x70: sm510_op_tl(); break;

		case 0x80: case 0x90: case 0xa0: case 0xb0:
			sm510_op_t(); break;
		case 0xc0: case 0xd0: case 0xe0: case 0xf0:
			sm510_op_tm(); break;

		default:
			switch (m_op & 0xfc)
			{
		case 0x04: sm510_op_rm(); break;
		case 0x0c: sm510_op_sm(); break;
		case 0x10: sm510_op_exc(); break;
		case 0x14: sm510_op_exci(); break;
		case 0x18: sm510_op_lda(); break;
		case 0x1c: sm510_op_excd(); break;
		case 0x54: sm510_op_tmi(); break;
		case 0x68: sm510_op_tml(); break;

		default:
			switch (m_op)
			{
		case 0x00: sm510_op_rot(); break;
		case 0x01: sm510_op_dta(); break;
		case 0x02: sm510_op_sbm(); break;
		case 0x03: sm510_op_atpl(); break;
		case 0x08: sm510_op_add(); break;
		case 0x09: sm510_op_add11(); break;
		case 0x0a: sm510_op_coma(); break;
		case 0x0b: sm510_op_exbla(); break;

		case 0x50: sm510_op_kta(); break;
		case 0x51: sm510_op_tb(); break;
		case 0x52: sm510_op_tc(); break;
		case 0x53: sm510_op_tam(); break;
		case 0x58: sm510_op_tis(); break;
		case 0x59: sm510_op_atl(); break;
		case 0x5a: sm510_op_ta0(); break;
		case 0x5b: sm510_op_tabl(); break;
		case 0x5c: sm510_op_atx(); break;
		case 0x5d: sm510_op_cend(); break;
		case 0x5e: sm510_op_tal(); break;
		case 0x5f: sm510_op_lbl(); break;

		case 0x61: sm510_op_pre(); break;
		case 0x62: sm510_op_wr(); break;
		case 0x63: sm510_op_ws(); break;
		case 0x64: sm510_op_incb(); break;
		case 0x65: sm510_op_dr(); break;
		case 0x66: sm510_op_rc(); break;
		case 0x67: sm510_op_sc(); break;
		case 0x6c: sm510_op_decb(); break;
		case 0x6d: sm510_op_ptw(); break;
		case 0x6e: sm510_op_rtn0(); break;
		case 0x6f: sm510_op_rtn1(); break;

		// extended opcodes
		case 0x60:
			m_op = m_op << 8 | m_param;
			switch (m_param)
			{
		case 0x30: sm510_op_rme(); break;
		case 0x31: sm510_op_sme(); break;
		case 0x32: sm510_op_tmel(); break;
		case 0x33: sm510_op_atfc(); break;
		case 0x34: sm510_op_bdc(); break;
		case 0x35: sm510_op_atbp(); break;
		case 0x36: sm510_op_clkhi(); break;
		case 0x37: sm510_op_clklo(); break;

		default: sm510_op_illegal(); break;
			}
			break; // 0x60

		default: sm510_op_illegal(); break;
			}
			break; // 0xff

			}
			break; // 0xfc

	} // big switch

	// BM high bit is only valid for 1 step
	m_sbm = (m_op == 0x02);
}

static inline void sm511_get_opcode_param()
{
	// LBL, PRE, TL, TML and prefix opcodes are 2 bytes
	if ((m_op >= 0x5f && m_op <= 0x61) || (m_op & 0xf0) == 0x70 || (m_op & 0xfc) == 0x68)
	{
		m_icount--;

		m_param = read_byte_program(m_pc);

		increment_pc();
	}
}

//-------------------------------------------------
//  execute
//-------------------------------------------------
void sm511_execute_run()
{
	int reamining_icount = m_icount;

	while (m_icount > 0)
	{
		m_icount--;

		if (m_halt && !sm510_wake_me_up())
		{
			sm511_div_timer(reamining_icount);

			// got nothing to do
			m_icount = 0;
			return;
		}

		// remember previous state
		m_prev_op = m_op;
		m_prev_pc = m_pc;

		// fetch next opcode
		m_op = read_byte_program(m_pc);

		increment_pc();
		sm511_get_opcode_param();

		// handle opcode if it's not skipped
		if (m_skip)
		{
			m_skip = false;
			m_op = 0; // fake nop
		}
		else
			sm511_execute_one();

		// clock: spent time
		sm511_div_timer(reamining_icount-m_icount);

		reamining_icount=m_icount;

	}
}
