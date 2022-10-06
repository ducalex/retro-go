// license:BSD-3-Clause
// copyright-holders:hap
// modded by bzhxx
/*

  Sharp SM510 MCU family cores

*/

#ifndef _SM510_H_
#define _SM510_H_

#pragma once

#include "gw_type_defs.h"

#define RMASK_DIRECT -1

/* SM510 */
void sm510_device_start();
void sm510_device_reset();
void sm510_execute_run();
bool sm510_wake_me_up();
void sm510_update_segments_state();
void sm510_reset_vector();
void sm510_wakeup_vector(); // after halt


/* SM511 variation */
void sm511_device_reset();
void sm511_execute_run();
bool sm511_init_melody(un8 *gw_melody);

/*misc internal helpers for SM5XX microcomputer family */
void pop_stack();
void push_stack();
void do_branch(u8 pu, u8 pm, u8 pl);
u8 bitmask(u16 param);
void increment_pc();
void update_w_latch();
extern u8 ram_r();
extern void ram_w(u8 data);

/*
I/O 
inputs  : K, B, BA
outputs :  S & R
*/ 
extern un8 m_read_k();

extern un8 m_read_ba();

extern un8 m_read_b(); 

extern void m_write_s(un8); 

extern void m_write_r(un8); 

un8 read_byte_program(un16 rom_address);
un8 readb(un8 ram_address);
void writeb(un8 ram_address, un8 ram_data);

//-------------------------------------------------------------
//  CPUs memory, registers & settings
//-------------------------------------------------------------

extern un8 gw_ram[128];
extern un8 gw_ram_state[128];

extern int m_prgwidth;
extern int m_datawidth;
extern int m_prgmask;
extern int m_datamask;

extern u16 m_pc, m_prev_pc;
extern u16 m_op, m_prev_op;
extern u8 m_param;
extern int m_stack_levels;
extern u16 m_stack[4]; // max 4
extern int m_icount;

extern u8 m_acc;
extern u8 m_bl;
extern u8 m_bm;
extern bool m_sbm;
extern bool m_sbl;
extern u8 m_c;
extern bool m_skip;
extern u8 m_w,m_s_out;
extern u8 m_r, m_r_out;
extern int m_r_mask_option;
extern bool m_k_active;
extern bool m_halt;
extern int m_clk_div;

// melody controller
extern u8 m_melody_rd;
extern u8 m_melody_step_count;
extern u8 m_melody_duty_count;
extern u8 m_melody_duty_index;
extern u8 m_melody_address;

// freerun time counter
extern u16 m_div;
extern bool m_1s;

// lcd driver
extern u8 m_l, m_x;
extern u8 m_y;
extern u8 m_bp;
extern bool m_bc;

//sm500 internals
extern 	int m_o_pins; // number of 4-bit O pins
extern 	u8 m_ox[9];   // W' latch, max 9
extern 	u8 m_o[9];    // W latch
extern 	u8 m_ox_state[9];   // W' latch, max 9
extern 	u8 m_o_state[9];    // W latch

extern 	u8 m_cn;
extern 	u8 m_mx;
extern 	u8 trs_field;
	 
extern 	u8 m_cb;
extern 	u8 m_s;
extern 	bool m_rsub;

/* LCD deflicker filter level */
/*
0 : filter is disabled
1 : refreshed on keys polling and call subroutine return
2 : refreshed on keys polling only
*/
extern u8 flag_lcd_deflicker_level;

// opcode handlers used by all CPUs
void sm510_op_lbl();
void sm510_op_lb();
void sm510_op_sbl();
void sm510_op_sbm();
void sm510_op_exbla();
void sm510_op_incb();
void sm510_op_decb();
void sm510_op_atpl();
void sm510_op_rtn0();
void sm510_op_rtn1();
void sm510_op_tl();
void sm510_op_tml();
void sm510_op_tm();
void sm510_op_t();
void sm510_op_exc();
void sm510_op_bdc();
void sm510_op_exci();
void sm510_op_excd();
void sm510_op_lda();
void sm510_op_lax();
void sm510_op_ptw();
void sm510_op_wr();
void sm510_op_ws();
void sm510_op_kta();
void sm510_op_atbp();
void sm510_op_atx();
void sm510_op_atl();
void sm510_op_atfc();
void sm510_op_atr();
void sm510_op_add();
void sm510_op_add11();
void sm510_op_adx();
void sm510_op_coma();
void sm510_op_rot();
void sm510_op_rc();
void sm510_op_sc();
void sm510_op_tb();
void sm510_op_tc();
void sm510_op_tam();
void sm510_op_tmi();
void sm510_op_ta0();
void sm510_op_tabl();
void sm510_op_tis();
void sm510_op_tal();
void sm510_op_tf1();
void sm510_op_tf4();
void sm510_op_rm();
void sm510_op_sm();
void sm510_op_pre();
void sm510_op_sme();
void sm510_op_rme();
void sm510_op_tmel();
void sm510_op_skip();
void sm510_op_cend();
void sm510_op_idiv();
void sm510_op_dr();
void sm510_op_dta();
void sm510_op_clklo();
void sm510_op_clkhi();
void sm510_op_illegal();

#endif // _SM510_H_
