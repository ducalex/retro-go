// license:BSD-3-Clause
// copyright-holders:hap, Igor
// modded bzhxx
/*

  Sharp SM5A MCU core implementation

*/

#include "gw_type_defs.h"
#include "sm510.h"
#include "sm500.h"

void sm5a_device_start()
{
	// common init (not everything is used though)
	sm510_device_start();

	// init/zerofill
	memset(m_ox, 0, sizeof(m_ox));
	memset(m_o, 0, sizeof(m_o));

	memset(m_ox_state, 0, sizeof(m_ox_state));
	memset(m_o_state, 0, sizeof(m_o_state));

	/* stack levels */
	m_stack_levels = 1;

	/* o group pins */
	m_o_pins = 9;

	/* prg width */
	m_prgwidth = 11;

	/*data width */
	m_datawidth = 7;

	m_prgmask = (1 << m_prgwidth) - 1;
	m_datamask = (1 << m_datawidth) - 1;

	trs_field = 1;

	m_cn = 0;
	m_mx = 0;
	m_cb = 0;
	m_s = 0;
	m_rsub = false;

	m_r_mask_option = RMASK_DIRECT;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sm5a_device_reset()
{
	// common reset
	sm510_device_reset();

	// SM500 specific
	sm500_reset_vector();
	m_prev_pc = m_pc;
	push_stack();
	sm500_op_idiv();
	m_1s = true;
	m_cb = 0;
	m_rsub = false;
	m_r = 0xff;
	m_r_out = 0;
}

void sm5a_execute_one()
{

	switch (m_op & 0xf0)
	{
	case 0x20:
		sm500_op_lax();
		break;
	case 0x30:
		sm500_op_adx();
		break;
	case 0x40:
		sm500_op_lb();
		break;
	case 0x70:
		sm500_op_ssr();
		break;

	case 0x80:
	case 0x90:
	case 0xa0:
	case 0xb0:
		sm500_op_tr();
		break;
	case 0xc0:
	case 0xd0:
	case 0xe0:
	case 0xf0:
		sm500_op_trs();
		break;

	default:
		switch (m_op & 0xfc)
		{
		case 0x04:
			sm500_op_rm();
			break;
		case 0x0c:
			sm500_op_sm();
			break;
		case 0x10:
			sm500_op_exc();
			break;
		case 0x14:
			sm500_op_exci();
			break;
		case 0x18:
			sm500_op_lda();
			break;
		case 0x1c:
			sm500_op_excd();
			break;
		case 0x54:
			sm500_op_tmi();
			break;

		default:
			switch (m_op)
			{
			case 0x00:
				sm500_op_skip();
				break;
			case 0x01:
				sm500_op_atr();
				break;
			case 0x02:
				sm500_op_sbm();
				break;
			case 0x03:
				sm500_op_atbp();
				break;
			case 0x08:
				sm500_op_add();
				break;
			case 0x09:
				sm500_op_add11();
				break;
			case 0x0a:
				sm500_op_coma();
				break;
			case 0x0b:
				sm500_op_exbla();
				break;

			case 0x50:
				sm500_op_tal();
				break;
			case 0x51:
				sm500_op_tb();
				break;
			case 0x52:
				sm500_op_tc();
				break;
			case 0x53:
				sm500_op_tam();
				break;
			case 0x58:
				sm500_op_tis();
				break;
			case 0x59:
				sm500_op_ptw();
				break;
			case 0x5a:
				sm500_op_ta0();
				break;
			case 0x5b:
				sm500_op_tabl();
				break;
			case 0x5c:
				sm500_op_tw();
				break;
			case 0x5d:
				sm500_op_dtw();
				break;
			case 0x5f:
				sm500_op_lbl();
				break;

			case 0x60:
				sm500_op_comcn();
				break;
			case 0x61:
				sm500_op_pdtw();
				break;
			case 0x62:
				sm500_op_wr();
				break;
			case 0x63:
				sm500_op_ws();
				break;
			case 0x64:
				sm500_op_incb();
				break;
			case 0x65:
				sm500_op_idiv();
				break;
			case 0x66:
				sm500_op_rc();
				break;
			case 0x67:
				sm500_op_sc();
				break;
			case 0x68:
				sm500_op_rmf();
				break;
			case 0x69:
				sm500_op_smf();
				break;
			case 0x6a:
				sm500_op_kta();
				break;
			case 0x6b:
				sm500_op_rbm();
				break;
			case 0x6c:
				sm500_op_decb();
				break;
			case 0x6d:
				sm500_op_comcb();
				break;
			case 0x6e:
				sm500_op_rtn0();
				break;
			case 0x6f:
				sm500_op_rtn1();
				break;

			// extended opcodes
			case 0x5e:
				m_op = m_op << 8 | m_param;
				switch (m_param)
				{
				case 0x00:
					sm500_op_cend();
					break;
				case 0x04:
					sm500_op_dta();
					break;

				default:
					sm500_op_illegal();
					break;
				}
				break; // 0x5e

			default:
				sm500_op_illegal();
				break;
			}
			break; // 0xff
		}
		break; // 0xfc

	} // big switch
}

//-------------------------------------------------
//  execute
//-------------------------------------------------
void sm5a_execute_run()
{
	int reamining_icount = m_icount;

	while (m_icount > 0)
	{
		m_icount--;

		if (m_halt && !sm500_wake_me_up())
		{

			sm500_div_timer(reamining_icount);

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
		sm500_get_opcode_param();

		// handle opcode if it's not skipped
		if (m_skip)
		{
			m_skip = false;
			m_op = 0; // fake nop
		}
		else
			sm5a_execute_one();

		// clock: spent time
		sm500_div_timer(reamining_icount - m_icount);

		reamining_icount = m_icount;
	}
}
