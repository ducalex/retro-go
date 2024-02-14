// license:BSD-3-Clause
// copyright-holders:hap
// modded by bzhxx

/*

  Sharp SM510 MCU core implementation

*/
#include "gw_type_defs.h"
#include "gw_romloader.h"
#include "gw_system.h"
#include "sm510.h"

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void sm510_device_start()
{
	m_prgwidth  = 12;
	m_datawidth = 7;
	m_prgmask   = (1 << m_prgwidth) - 1;
	m_datamask  = (1 << m_datawidth) - 1;

	// init/zerofill
	memset(m_stack, 0, sizeof(m_stack));
	memset(gw_ram, 0, sizeof(gw_ram));
	memset(gw_ram_state, 0, sizeof(gw_ram_state));
	m_stack_levels=2;
	m_pc = 0;
	m_prev_pc = 0;
	m_op = 0;
	m_prev_op = 0;
	m_param = 0;
	m_acc = 0;
	m_bl = 0;
	m_bm = 0;
	m_sbl = false;
	m_sbm = false;
	m_c = 0;
	m_skip = false;
	m_w = 0;
	m_s_out = 0;
	m_r = 0;
	m_r_out = 0;
	m_div = 0;
	m_1s = false;
	m_k_active = false;
	m_l = 0;
	m_x = 0;
	m_y = 0;
	m_bp = 0;
	m_bc = false;
	m_halt = false;
	m_melody_rd = 0;
	m_melody_step_count = 0;
	m_melody_duty_count = 0;
	m_melody_duty_index = 0;
	m_melody_address = 0;
	m_clk_div = 2; // 16kHz
	m_icount = 0;

	m_r_mask_option = 2;

}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void sm510_device_reset()
{
	// ACL
	m_skip = false;
	m_halt = false;
	m_sbl = false;
	m_sbm = false;
	m_op = m_prev_op = 0;
	sm510_reset_vector();
	m_prev_pc = m_pc;

	// lcd is on (Bp on, BC off, bs(y) off)
	m_bp = 1;
	m_bc = false;
	m_y = 0;

	m_r = m_r_out = 0;
}

//-------------------------------------------------
//  buzzer controller
//-------------------------------------------------
static inline void sm510_clock_melody()
{
	u8 out = 0;

	if (m_r_mask_option == RMASK_DIRECT)
	{
		// direct output
		out = m_r & 3;
	}
	else
	{
		// from divider, R2 inverse phase
		out = m_div >> m_r_mask_option & 1;
		out |= (out << 1 ^ 2);
		out &= m_r;
	}

	m_r_out = out;
	m_write_r(m_r_out);
}

void sm510_init_melody(){ }

//-------------------------------------------------
//  wake up routine
//-------------------------------------------------

inline bool sm510_wake_me_up()
{
	// in halt mode, wake up after 1S signal or K input
	if (m_k_active || m_1s)
	{
		// note: official doc warns that Bl/Bm and the stack are undefined
		// after waking up, but we leave it unchanged
		m_halt = false;
		sm510_wakeup_vector();

		return true;
	}
	else
		return false;
}

/********** 1 second timer *********/
static inline void sm510_div_timer_cb()
{
	m_div = (m_div + 1) & 0x7fff;

	// 1S signal on overflow(falling edge of F1)
	if (m_div == 0){
		m_1s = true;

		/* refresh LCD segments state every seconds under sleep mode */
		if (m_halt)	sm510_update_segments_state();
	}

	sm510_clock_melody();
}

static inline void sm510_div_timer(int nb_inst)
{
	if (nb_inst > 0)
		for ( int toctoc=0; toctoc < m_clk_div*nb_inst; toctoc++ )
			sm510_div_timer_cb();
}

/*************************************/

static inline void sm510_get_opcode_param()
{
	// LBL, TL, TML opcodes are 2 bytes
	if (m_op == 0x5f || (m_op & 0xf0) == 0x70)
	{
		m_icount--;

		m_param = read_byte_program(m_pc);

		increment_pc();
	}
}

void sm510_execute_one()
{
	switch (m_op & 0xf0)
	{
		case 0x20: sm510_op_lax(); break;
		case 0x30: sm510_op_adx(); break;
		case 0x40: sm510_op_lb(); break;

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
		case 0x70: case 0x74: case 0x78: sm510_op_tl(); break;
		case 0x7c: sm510_op_tml(); break;

		default:
			switch (m_op)
			{
		case 0x00: sm510_op_skip(); break;
		case 0x01: sm510_op_atbp(); break;
		case 0x02: sm510_op_sbm(); break;
		case 0x03: sm510_op_atpl(); break;
		case 0x08: sm510_op_add(); break;
		case 0x09: sm510_op_add11(); break;
		case 0x0a: sm510_op_coma(); break;
		case 0x0b: sm510_op_exbla(); break;

		case 0x51: sm510_op_tb(); break;
		case 0x52: sm510_op_tc(); break;
		case 0x53: sm510_op_tam(); break;
		case 0x58: sm510_op_tis(); break;
		case 0x59: sm510_op_atl(); break;
		case 0x5a: sm510_op_ta0(); break;
		case 0x5b: sm510_op_tabl(); break;
		case 0x5d: sm510_op_cend(); break;
		case 0x5e: sm510_op_tal(); break;
		case 0x5f: sm510_op_lbl(); break;

		case 0x60: sm510_op_atfc(); break;
		case 0x61: sm510_op_atr(); break;
		case 0x62: sm510_op_wr(); break;
		case 0x63: sm510_op_ws(); break;
		case 0x64: sm510_op_incb(); break;
		case 0x65: sm510_op_idiv(); break;
		case 0x66: sm510_op_rc(); break;
		case 0x67: sm510_op_sc(); break;
		case 0x68: sm510_op_tf1(); break;
		case 0x69: sm510_op_tf4(); break;
		case 0x6a: sm510_op_kta(); break;
		case 0x6b: sm510_op_rot(); break;
		case 0x6c: sm510_op_decb(); break;
		case 0x6d: sm510_op_bdc(); break;
		case 0x6e: sm510_op_rtn0(); break;
		case 0x6f: sm510_op_rtn1(); break;

		default: sm510_op_illegal(); break;
			}
			break; // 0xff

			}
			break; // 0xfc

	} // big switch

	// BM high bit is only valid for 1 step
	m_sbm = (m_op == 0x02);
}

//-------------------------------------------------
//  execute
//-------------------------------------------------
void sm510_execute_run()
{
	int reamining_icount = m_icount;

	while (m_icount > 0)
	{
		m_icount--;

		if (m_halt && !sm510_wake_me_up())
		{
			sm510_div_timer(reamining_icount);

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
		sm510_get_opcode_param();

		// handle opcode if it's not skipped
		if (m_skip)
		{
			m_skip = false;
			m_op = 0; // fake nop
		}
		else
			sm510_execute_one();

		// clock: spent time
		sm510_div_timer(reamining_icount-m_icount);

		reamining_icount=m_icount;
	}
}
