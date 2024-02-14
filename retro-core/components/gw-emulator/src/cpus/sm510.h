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
typedef struct
{
  un8 gw_ram[128];
  un8 gw_ram_state[128];

  int m_prgwidth;
  int m_datawidth;
  int m_prgmask;
  int m_datamask;

  u16 m_pc, m_prev_pc;
  u16 m_op, m_prev_op;
  u8 m_param;
  int m_stack_levels;
  u16 m_stack[4]; // max 4
  int m_icount;

  u8 m_acc;
  u8 m_bl;
  u8 m_bm;
  bool m_sbm;
  bool m_sbl;
  u8 m_c;
  bool m_skip;
  u8 m_w,m_s_out;
  u8 m_r, m_r_out;
  int m_r_mask_option;
  bool m_k_active;
  bool m_halt;
  int m_clk_div;

// melody controller
  u8 m_melody_rd;
  u8 m_melody_step_count;
  u8 m_melody_duty_count;
  u8 m_melody_duty_index;
  u8 m_melody_address;

// freerun time counter
  u16 m_div;
  bool m_1s;

// lcd driver
  u8 m_l, m_x;
  u8 m_y;
  u8 m_bp;
  bool m_bc;

//sm500 internals
  int m_o_pins; // number of 4-bit O pins
  u8 m_ox[9];   // W' latch, max 9
  u8 m_o[9];    // W latch
  u8 m_ox_state[9];   // W' latch, max 9
  u8 m_o_state[9];    // W latch

  u8 m_cn;
  u8 m_mx;
  u8 trs_field;

  u8 m_cb;
  u8 m_s;
  bool m_rsub;

/* LCD deflicker filter level */
/*
0 : filter is disabled
1 : refreshed on keys polling and call subroutine return
2 : refreshed on keys polling only
*/
  u8 flag_lcd_deflicker_level;
} sm510_t;

extern sm510_t sm510;

#ifndef GW_NO_SM510_DEFINES
#define gw_ram sm510.gw_ram
#define gw_ram_state sm510.gw_ram_state
#define m_prgwidth sm510.m_prgwidth
#define m_datawidth sm510.m_datawidth
#define m_prgmask sm510.m_prgmask
#define m_datamask sm510.m_datamask
#define m_pc sm510.m_pc
#define m_prev_pc sm510.m_prev_pc
#define m_op sm510.m_op
#define m_prev_op sm510.m_prev_op
#define m_param sm510.m_param
#define m_stack_levels sm510.m_stack_levels
#define m_stack sm510.m_stack
#define m_icount sm510.m_icount
#define m_acc sm510.m_acc
#define m_bl sm510.m_bl
#define m_bm sm510.m_bm
#define m_sbm sm510.m_sbm
#define m_sbl sm510.m_sbl
#define m_c sm510.m_c
#define m_skip sm510.m_skip
#define m_w sm510.m_w
#define m_s_out sm510.m_s_out
#define m_r sm510.m_r
#define m_r_out sm510.m_r_out
#define m_r_mask_option sm510.m_r_mask_option
#define m_k_active sm510.m_k_active
#define m_halt sm510.m_halt
#define m_clk_div sm510.m_clk_div
#define m_melody_rd sm510.m_melody_rd
#define m_melody_step_count sm510.m_melody_step_count
#define m_melody_duty_count sm510.m_melody_duty_count
#define m_melody_duty_index sm510.m_melody_duty_index
#define m_melody_address sm510.m_melody_address
#define m_div sm510.m_div
#define m_1s sm510.m_1s
#define m_l sm510.m_l
#define m_x sm510.m_x
#define m_y sm510.m_y
#define m_bp sm510.m_bp
#define m_bc sm510.m_bc
#define m_o_pins sm510.m_o_pins
#define m_ox sm510.m_ox
#define m_o sm510.m_o
#define m_ox_state sm510.m_ox_state
#define m_o_state sm510.m_o_state
#define m_cn sm510.m_cn
#define m_mx sm510.m_mx
#define trs_field sm510.trs_field
#define m_cb sm510.m_cb
#define m_s sm510.m_s
#define m_rsub sm510.m_rsub
#define flag_lcd_deflicker_level sm510.flag_lcd_deflicker_level
#endif

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
