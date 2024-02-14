// license:BSD-3-Clause
// copyright-holders:hap, Igor
// modded by bzhxx

// SM500 shared opcode handlers

#include "gw_type_defs.h"
#include "gw_system.h"
#include "sm510.h"
#include "sm500.h"

// internal helpers

//-------------------------------------------------
//  Vector table reset & wakeup
//-------------------------------------------------

void sm500_reset_vector()  { do_branch(0, 0xf, 0); }
void sm500_wakeup_vector()  { m_cb = 0; do_branch(0, 0, 0); }
	
void set_su(u8 su) { m_stack[0] = (m_stack[0] & ~0x3c0) | (su << 6 & 0x3c0); }
u8 get_su() { return m_stack[0] >> 6 & 0xf; }

// return 1 if SM5A or 0 if SM500
int get_trs_field() { return trs_field; }
	
void shift_w()
{
	// shifts internal W' latches
	for (int i = 0; i < (m_o_pins-1); i++)
		m_ox[i] = m_ox[i + 1];
}

/* to prevent from glich on display
 [Alternative to MAME, // 1kHz display decay]
 we mirror the output mo mox when the segments are
 considered stable.
 Typically, when the input are polled by
 the functions : aplha,beta,K1..K4
 TA:sm500_op_tal()
 TB:sm500_op_tb()
 KTA:sm500_op_kta()
 optionally return function:rtn()
 depending on flag_force_refresh_display from ROM flags
*/

void sm500_update_segments_state()
{
	// // Mirror the Outputs as segments states
	for (int i = 0; i < m_o_pins; i++) {

		m_o_state[i]  = m_o[i];
		m_ox_state[i] = m_ox[i];
	}

}

u8 get_digit()
{
	// default digit segments PLA
	static const u8 lut_digits[0x20] =
	{
		0xe, 0x0, 0xc, 0x8, 0x2, 0xa, 0xe, 0x2, 0xe, 0xa, 0x0, 0x0, 0x2, 0xa, 0x2, 0x2,
		0xb, 0x9, 0x7, 0xf, 0xd, 0xe, 0xe, 0xb, 0xf, 0xf, 0x4, 0x0, 0xd, 0xe, 0x4, 0x0
	};

	return lut_digits[m_cn << 4 | m_acc] | (~m_cn & m_mx);
}

// instruction set

// RAM address instructions

void sm500_op_lb()
{
	// LB x: load BM/BL with 4-bit immediate value (partial)
	m_bm = m_op & 3;
	m_bl = (m_op >> 2 & 3) | ((m_op & 0xc) ? 8 : 0);
}

void sm500_op_incb()
{
	// INCB: increment BL, but overflow on 3rd bit!
	m_bl = (m_bl + 1) & 0xf;
	m_skip = (m_bl == 8);
}

void sm500_op_decb()
{
	// DECB: decrement BL, skip next on overflow
	m_bl = (m_bl - 1) & 0xf;
	m_skip = (m_bl == 0xf);
}

void sm500_op_sbm()
{
	// SBM: set RAM address high bit
	m_bm |= 4;
}

void sm500_op_rbm()
{
	// RBM: reset RAM address high bit
	m_bm &= ~4;
}


// ROM address instructions

void sm500_op_comcb()
{
	// COMCB: complement CB
	m_cb ^= 1;
}

void sm500_op_rtn0()
{
	if (flag_lcd_deflicker_level < 2 )
		sm500_update_segments_state();

	// RTN0: return from subroutine
	pop_stack();
	m_rsub = false;
}

void sm500_op_rtn1()
{
	// RTN0(RTN): return from subroutine
	sm500_op_rtn0();
	m_skip = true;
}

void sm500_op_ssr()
{
	// SSR x: set stack upper bits, also sets E flag for next opcode
	set_su(m_op & 0xf);
}

void sm500_op_tr()
{
	// TR x: jump (long or short)
	m_pc = (m_pc & ~0x3f) | (m_op & 0x3f);
	if (!m_rsub)
		do_branch(m_cb, get_su(), m_pc & 0x3f);
}

void sm500_op_trs()
{

	// TRS x: call subroutine
	if (!m_rsub)
	{
		m_rsub = true;
		u8 su = get_su();
		push_stack();
		do_branch(get_trs_field(), 0, m_op & 0x3f);

		// E flag was set?
		if ((m_prev_op & 0xf0) == 0x70)
			do_branch(m_cb, su, m_pc & 0x3f);
	}
	else
		m_pc = (m_pc & ~0xff) | (m_op << 2 & 0xc0) | (m_op & 0xf);
}


// Data transfer instructions
void sm500_op_exc()
{
	// EXC x: exchange ACC with RAM, xor BM with x
	u8 a = m_acc;
	m_acc = ram_r();
	ram_w(a);
	m_bm ^= (m_op & 3);
}

void sm500_op_exci()
{
	// EXCI x: EXC x, INCB
	sm500_op_exc();
	sm500_op_incb();
}

void sm500_op_excd()
{
	// EXCD x: EXC x, DECB
	sm500_op_exc();
	sm500_op_decb();
}

void sm500_op_atbp()
{
	// ATBP: same as SM510, and set CN with ACC3
	// ATBP: output ACC to BP(internal LCD backplate signal)
	m_bp = m_acc & 1;
	m_cn = m_acc >> 3 & 1;
}

void sm500_op_ptw()
{
	// PTW: partial transfer W' to W
	m_o[m_o_pins-1] = m_ox[m_o_pins-1];
	m_o[m_o_pins-2] = m_ox[m_o_pins-2];
}

void sm500_op_tw()
{
	// TW: transfer W' to W
	for (int i = 0; i < m_o_pins; i++)
		m_o[i] = m_ox[i];
}

void sm500_op_pdtw()
{
	// PDTW: partial shift digit into W'
	m_ox[m_o_pins-2] = m_ox[m_o_pins-1];
	m_ox[m_o_pins-1] = get_digit();
}

void sm500_op_dtw()
{
	// DTW: shift digit into W'
	shift_w();
	m_ox[m_o_pins-1] = get_digit();
}

void sm500_op_wr()
{
	// WR: shift ACC into W', reset last bit
	shift_w();
	m_ox[m_o_pins-1] = m_acc & 7;
}

void sm500_op_ws()
{
	// WR: shift ACC into W', set last bit
	shift_w();
	m_ox[m_o_pins-1] = m_acc | 8;
}


// I/O instructions
void sm500_op_kta()
{
	
	//copy all segments states
	sm500_update_segments_state();

	// KTA: input K to ACC
	//m_acc = gw_readK(~m_r & 0xf) & 0xf;
	m_acc = gw_readK(m_r_out & 0xf) & 0xf;
}

void sm500_op_ats()
{
	// ATS: transfer ACC to S
	m_s = m_acc;
}

void sm500_op_exksa()
{
	// EXKSA: x
}

void sm500_op_exkfa()
{
	// EXKFA: x
}


// Divider manipulation instructions

void sm500_op_idiv()
{
	// IDIV: reset divider low 9 bits
	m_div &= 0x3f;
}


// Bit manipulation instructions

void sm500_op_rmf()
{
	// RMF: reset m' flag, also clears ACC
	m_mx = 0;
	m_acc = 0;
}

void sm500_op_smf()
{
	// SMF: set m' flag
	m_mx = 1;
}

void sm500_op_comcn()
{
	// COMCN: complement CN flag
	m_cn ^= 1;
}

void sm500_op_lax() { sm510_op_lax(); }
void sm500_op_adx() { sm510_op_adx(); }
void sm500_op_rm() { sm510_op_rm(); }
void sm500_op_sm() { sm510_op_sm(); }
//void sm500_op_exc() {  sm510_op_exc(); }
//void sm500_op_exci() { sm510_op_exci(); }
void sm500_op_lda() { sm510_op_lda(); }
//void sm500_op_excd() { sm510_op_excd(); }
void sm500_op_tmi() { sm510_op_tmi(); }
void sm500_op_skip() { sm510_op_skip(); }
void sm500_op_atr() { sm510_op_atr(); }
void sm500_op_add() { sm510_op_add(); }
void sm500_op_add11() { sm510_op_add11(); }
void sm500_op_coma() { sm510_op_coma(); }
void sm500_op_exbla() { sm510_op_exbla(); }

void sm500_op_tal() {
	//copy all segments states
	sm500_update_segments_state();

	//sm510_op_tal();
	// TAL: skip next if BA pin is set
	m_skip = (m_read_ba() != 0);
	} 

void sm500_op_tb() { 
	//copy all segments states
	sm500_update_segments_state();

	//sm510_op_tb();
	// TB: skip next if B(beta) pin is set
	m_skip = (m_read_b() != 0);
	} 

void sm500_op_tc() { sm510_op_tc(); } 
void sm500_op_tam() { sm510_op_tam(); } 
void sm500_op_tis() { sm510_op_tis(); } 

void sm500_op_ta0() { sm510_op_ta0(); } 
void sm500_op_tabl() { sm510_op_tabl(); } 

void sm500_op_lbl() { sm510_op_lbl(); } 

void sm500_op_rc() { sm510_op_rc(); }
void sm500_op_sc() { sm510_op_sc(); }

// void sm500_op_kta() { sm510_op_kta(); }
// void sm500_op_decb() { sm510_op_decb(); }
// void sm500_op_rtn1() { sm510_op_rtn1(); }

void sm500_op_cend() { sm510_op_cend(); }
void sm500_op_dta() { sm510_op_dta(); }
void sm500_op_illegal() { sm510_op_illegal(); }
	