// license:BSD-3-Clause
// copyright-holders:hap
// modded by bzhxx

// SM510 shared opcode handlers

#include "gw_type_defs.h"
#include "sm510.h"
#include <assert.h>

// internal helpers

//-------------------------------------------------
//  Vector table reset & wakeup
//-------------------------------------------------

void sm510_reset_vector() { do_branch(3, 7, 0); }
void sm510_wakeup_vector() { do_branch(1, 0, 0); } // after halt



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

void sm510_update_segments_state()
{
	// Mirror the RAM memory as segments states
	for (int address = 0; address < 128; address++)
		gw_ram_state[address] = gw_ram[address];
};

// instruction set

// RAM address instructions

void sm510_op_lb()
{
	// LB x: load BM/BL with 4-bit immediate value (partial)
	m_bm = (m_bm & 4) | (m_op & 3);
	m_bl = (m_op >> 2 & 3) | ((m_op & 0xc) ? 0xc : 0);
}

void sm510_op_lbl()
{
	// LBL xy: load BM/BL with 8-bit immediate value
	m_bl = m_param & 0xf;
	m_bm = (m_param & m_datamask) >> 4;
}

void sm510_op_sbl()
{
	// SBL: set BL high bit for next opcode - handled in execute_one()
}

void sm510_op_sbm()
{
	// SBM: set BM high bit for next opcode - handled in execute_one()
}

void sm510_op_exbla()
{
	// EXBLA: exchange BL with ACC
	u8 a = m_acc;
	m_acc = m_bl;
	m_bl = a;
}

void sm510_op_incb()
{
	// INCB: increment BL, skip next on overflow
	m_bl = (m_bl + 1) & 0xf;
	m_skip = (m_bl == 0);
}

void sm510_op_decb()
{
	// DECB: decrement BL, skip next on overflow
	m_bl = (m_bl - 1) & 0xf;
	m_skip = (m_bl == 0xf);
}

// ROM address instructions

void sm510_op_atpl()
{
	// ATPL: load Pl(PC low bits) with ACC
	m_pc = (m_prev_pc & ~0xf) | m_acc;
}

void sm510_op_rtn0()
{
	if (flag_lcd_deflicker_level < 2)
		sm510_update_segments_state();

	// RTN0: return from subroutine
	pop_stack();
}

void sm510_op_rtn1()
{
	// RTN1: return from subroutine, skip next
	sm510_op_rtn0();
	m_skip = true;
}

void sm510_op_t()
{
	// T xy: jump(transfer) within current page
	m_pc = (m_pc & ~0x3f) | (m_op & 0x3f);
}

void sm510_op_tl()
{
	// TL xyz: long jump
	do_branch(m_param >> 6 & 3, m_op & 0xf, m_param & 0x3f);
}

void sm510_op_tml()
{

	// TML xyz: long call
	push_stack();
	do_branch(m_param >> 6 & 3, m_op & 3, m_param & 0x3f);
}

void sm510_op_tm()
{

	// TM x: indirect subroutine call, pointers(IDX) are on page 0
	m_icount--;

	push_stack();
	u8 idx = read_byte_program(m_op & 0x3f);
	do_branch(idx >> 6 & 3, 4, idx & 0x3f);
}

// Data transfer instructions

void sm510_op_exc()
{
	// EXC x: exchange ACC with RAM, xor BM with x
	u8 a = m_acc;
	m_acc = ram_r();
	ram_w(a);
	m_bm ^= (m_op & 3);
}

void sm510_op_bdc()
{
	// BDC: enable LCD bleeder current with C
	m_bc = (m_c != 0);
}

void sm510_op_exci()
{
	// EXCI x: EXC x, INCB
	sm510_op_exc();
	sm510_op_incb();
}

void sm510_op_excd()
{
	// EXCD x: EXC x, DECB
	sm510_op_exc();
	sm510_op_decb();
}

void sm510_op_lda()
{
	// LDA x: load ACC with RAM, xor BM with x
	m_acc = ram_r();
	m_bm ^= (m_op & 3);
}

void sm510_op_lax()
{
	// LAX x: load ACC with immediate value, skip any next LAX
	if ((m_op & ~0xf) != (m_prev_op & ~0xf))
		m_acc = m_op & 0xf;
}

void sm510_op_ptw()
{
	// PTW: output W latch
	m_write_s(m_w);
}

void sm510_op_wr()
{
	// WR: shift 0 into W
	m_w = m_w << 1 | 0;
	update_w_latch();
}

void sm510_op_ws()
{
	// WS: shift 1 into W
	m_w = m_w << 1 | 1;
	update_w_latch();
}

// I/O instructions

void sm510_op_kta()
{

	sm510_update_segments_state();

	// KTA: input K to ACC
	m_acc = m_read_k() & 0xf;
}

void sm510_op_atbp()
{
	// ATBP: output ACC to BP(internal LCD backplate signal)
	m_bp = m_acc & 1;
}

void sm510_op_atx()
{
	// ATX: output ACC to X
	m_x = m_acc;
}

void sm510_op_atl()
{
	// ATL: output ACC to L
	m_l = m_acc;
}

void sm510_op_atfc()
{
	// ATFC: output ACC to Y
	m_y = m_acc;
}

void sm510_op_atr()
{
	// ATR: output ACC to R
	m_r = m_acc;
	//sm510_clock_melody();
}

// Arithmetic instructions

void sm510_op_add()
{
	// ADD: add RAM to ACC
	m_acc = (m_acc + ram_r()) & 0xf;
}

void sm510_op_add11()
{
	// ADD11: add RAM and carry to ACC and carry, skip next on carry
	m_acc += ram_r() + m_c;
	m_c = m_acc >> 4 & 1;
	m_skip = (m_c == 1);
	m_acc &= 0xf;
}

void sm510_op_adx()
{
	// ADX x: add immediate value to ACC, skip next on carry except if x = 10
	m_acc += (m_op & 0xf);
	m_skip = ((m_op & 0xf) != 10 && (m_acc & 0x10) != 0);
	m_acc &= 0xf;
}

void sm510_op_coma()
{
	// COMA: complement ACC
	m_acc ^= 0xf;
}

void sm510_op_rot()
{
	// ROT: rotate ACC right through carry
	u8 c = m_acc & 1;
	m_acc = m_acc >> 1 | m_c << 3;
	m_c = c;
}

void sm510_op_rc()
{
	// RC: reset carry
	m_c = 0;
}

void sm510_op_sc()
{
	// SC: set carry
	m_c = 1;
}

// Test instructions

void sm510_op_tb()
{
	sm510_update_segments_state();

	// TB: skip next if B(beta) pin is set
	m_skip = (m_read_b() != 0);
}

void sm510_op_tc()
{
	// TC: skip next if no carry
	m_skip = !m_c;
}

void sm510_op_tam()
{
	// TAM: skip next if ACC equals RAM
	m_skip = (m_acc == ram_r());
}

void sm510_op_tmi()
{
	// TMI x: skip next if RAM bit is set
	m_skip = ((ram_r() & bitmask(m_op)) != 0);
}

void sm510_op_ta0()
{
	// TA0: skip next if ACC is clear
	m_skip = !m_acc;
}

void sm510_op_tabl()
{
	// TABL: skip next if ACC equals BL
	m_skip = (m_acc == m_bl);
}

void sm510_op_tis()
{
	// TIS: skip next if 1S(gamma flag) is clear, reset it after
	m_skip = !m_1s;
	m_1s = false;
}

void sm510_op_tal()
{
	sm510_update_segments_state();

	// TAL: skip next if BA pin is set
	m_skip = (m_read_ba() != 0);
}

void sm510_op_tf1()
{
	// TF1: skip next if divider F1(d14) is set
	m_skip = ((m_div & 0x4000) != 0);
}

void sm510_op_tf4()
{
	// TF4: skip next if divider F4(d11) is set
	m_skip = ((m_div & 0x0800) != 0);
}

// Bit manipulation instructions

void sm510_op_rm()
{
	// RM x: reset RAM bit
	ram_w(ram_r() & ~bitmask(m_op));
}

void sm510_op_sm()
{
	// SM x: set RAM bit
	ram_w(ram_r() | bitmask(m_op));
}

// Melody control instructions

void sm510_op_pre()
{
	// PRE x: melody ROM pointer preset
	m_melody_address = m_param;
	m_melody_step_count = 0;
}

void sm510_op_sme()
{

	// SME: set melody enable
	m_melody_rd |= 1;
}

void sm510_op_rme()
{
	// RME: reset melody enable
	m_melody_rd &= ~1;
}

void sm510_op_tmel()
{
	// TMEL: skip next if melody stop flag is set, reset it
	m_skip = ((m_melody_rd & 2) != 0);
	m_melody_rd &= ~2;
}

// Special instructions

void sm510_op_skip()
{
	// SKIP: no operation
}

void sm510_op_cend()
{
	// CEND: stop clock (halt the cpu and go into low-power mode)
	m_halt = true;
}

void sm510_op_idiv()
{
	// IDIV: reset divider
	m_div = 0;
}

void sm510_op_dr()
{
	// DR: reset divider low 8 bits
	m_div &= 0x7f;
}

void sm510_op_dta()
{
	// DTA: transfer divider low 4 bits to ACC
	m_acc = m_div >> 11 & 0xf;
}

void sm510_op_clklo()
{
	// CLKLO*: select 8kHz instruction clock (*unknown mnemonic)
	m_clk_div = 4;
}

void sm510_op_clkhi()
{
	// CLKHI*: select 16kHz instruction clock (*unknown mnemonic)
	m_clk_div = 2;
}

void sm510_op_illegal()
{
	printf("unknown opcode $%02X at $%04X\n", m_op, m_prev_pc);
	assert(0);
}
