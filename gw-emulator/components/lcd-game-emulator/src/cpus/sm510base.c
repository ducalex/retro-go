// license:BSD-3-Clause
// copyright-holders:hap
// modded by bzhxx
/*

  Sharp SM5xx family, using SM510 as parent device

  References:
  - 1986 Sharp Semiconductor Data Book
  - 1990 Sharp Microcomputers Data Book
  - 1996 Sharp Microcomputer Databook
  - KB1013VK1-2/KB1013VK4-2 manual

  Default external frequency of these is 32.768kHz, forwarding a clockrate in the
  MAME machine config is optional. Newer revisions can have an internal oscillator.

*/

#include "gw_type_defs.h"
#include "sm510.h"
#include "gw_romloader.h"
#include "gw_system.h"

	int m_prgwidth;
	int m_datawidth;
	int m_prgmask;
	int m_datamask;

	// internal RAM 128x4 bits
	un8 gw_ram[128];
	un8 gw_ram_state[128];

	u16 m_pc, m_prev_pc;
	u16 m_op, m_prev_op;
	u8 m_param;
	int m_stack_levels=2;
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
	u8 flag_lcd_deflicker_level=2;
	u8 m_l, m_x;
	u8 m_y;
	u8 m_bp;
	bool m_bc;

	// SM500 internals
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
	

void update_w_latch() { m_write_s(m_w); } // W is connected directly to S

//-------------------------------------------------
// internal helpers to access memories
//-------------------------------------------------
//-------------------------------------------------
//  Memory access SM5xx family
//-------------------------------------------------
u8 ram_r()
{
	int blh = (m_sbl) ? 8 : 0;						  // from SBL (optional)
	int bmh = (m_sbm) ? (1 << (m_datawidth - 1)) : 0; // from SBM
	u8 address = (bmh | blh | m_bm << 4 | m_bl) & m_datamask;

	if ((m_stack_levels == 1) & (address > 0x4f))
		address &= 0x4f;

	return readb(address) & 0xf;
}

void ram_w(u8 data)
{
	int blh = (m_sbl) ? 8 : 0;						  // from SBL (optional)
	int bmh = (m_sbm) ? (1 << (m_datawidth - 1)) : 0; // from SBM
	u8 address = (bmh | blh | m_bm << 4 | m_bl) & m_datamask;

	if ((m_stack_levels == 1) & (address > 0x4f))
		address &= 0x4f;

	writeb(address, data & 0xf);
}

//-------------------------------------------------
//  Stack push & pop access SM5xx family
//-------------------------------------------------

void pop_stack()
{
	m_pc = m_stack[0] & m_prgmask;
	for (int i = 0; i < m_stack_levels - 1; i++)
		m_stack[i] = m_stack[i + 1];
}

void push_stack()
{
	for (int i = m_stack_levels - 1; i >= 1; i--)
		m_stack[i] = m_stack[i - 1];
	m_stack[0] = m_pc;
}

void do_branch(u8 pu, u8 pm, u8 pl)
{
	// set new PC(Pu/Pm/Pl)
	m_pc = ((pu << 10) | (pm << 6 & 0x3c0) | (pl & 0x03f)) & m_prgmask;
}

u8 bitmask(u16 param)
{
	// bitmask from immediate opcode param
	return 1 << (param & 3);
}

// External Memory functions */
/*****************************/
un8 read_byte_program(un16 rom_address)
{
	return *(gw_program+rom_address);
}

un8 readb(un8 ram_address)
{
	return gw_ram[ram_address];
}

void writeb(un8 ram_address,un8 ram_data)
{
 	gw_ram[ram_address] = ram_data;
}
// External IO functions */
/*************************/

inline un8 m_read_k() 
{
	return gw_readK(m_s_out);
}

inline un8 m_read_ba()
{
	return gw_readBA();
}

inline un8 m_read_b()
{
	return gw_readB();
}

inline void m_write_s(un8 data)
{
	m_s_out = data;
}
inline void m_write_r(un8 data)
{
	gw_writeR(data);
}

//------------------------------------------------
//  Program counter
//-------------------------------------------------
inline void increment_pc()
{
	// PL(program counter low 6 bits) is a simple LFSR: newbit = (bit0==bit1)
	// PU,PM(high bits) specify page, PL specifies steps within page
	int feed = ((m_pc >> 1 ^ m_pc) & 1) ? 0 : 0x20;
	m_pc = feed | (m_pc >> 1 & 0x1f) | (m_pc & ~0x3f);
}
