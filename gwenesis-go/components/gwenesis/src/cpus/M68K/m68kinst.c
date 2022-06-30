#include <stdio.h>
#include "m68kcpu.h"
#pragma GCC optimize("Os")
extern void m68040_fpu_op0(void);
extern void m68040_fpu_op1(void);
extern void m68881_mmu_ops(void);

/* ======================================================================== */
/* ========================= INSTRUCTION HANDLERS ========================= */
/* ======================================================================== */


void m68k_op_1010(void)
{
	m68ki_exception_1010();
}


void m68k_op_1111(void)
{
	m68ki_exception_1111();
}


void m68k_op_040fpu0_32(void)
{

	m68ki_exception_1111();
}


void m68k_op_040fpu1_32(void)
{

	m68ki_exception_1111();
}


void m68k_op_abcd_8_rr(void)
{
	uint* r_dst = &DX;
	uint src = DY;
	uint dst = *r_dst;
	uint res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res += 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res -= 0xa0;

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
}


void m68k_op_abcd_8_mm_ax7(void)
{
	uint src = OPER_AY_PD_8();
	uint ea  = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res += 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res -= 0xa0;

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_abcd_8_mm_ay7(void)
{
	uint src = OPER_A7_PD_8();
	uint ea  = EA_AX_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res += 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res -= 0xa0;

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_abcd_8_mm_axy7(void)
{
	uint src = OPER_A7_PD_8();
	uint ea  = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res += 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res -= 0xa0;

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_abcd_8_mm(void)
{
	uint src = OPER_AY_PD_8();
	uint ea  = EA_AX_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res += 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res -= 0xa0;

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_add_8_er_d(void)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_8(DY);
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_8_er_ai(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_AI_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_8_er_pi(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_PI_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_8_er_pi7(void)
{
	uint* r_dst = &DX;
	uint src = OPER_A7_PI_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_8_er_pd(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_PD_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_8_er_pd7(void)
{
	uint* r_dst = &DX;
	uint src = OPER_A7_PD_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_8_er_di(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_DI_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_8_er_ix(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_IX_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_8_er_aw(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AW_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_8_er_al(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AL_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_8_er_pcdi(void)
{
	uint* r_dst = &DX;
	uint src = OPER_PCDI_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_8_er_pcix(void)
{
	uint* r_dst = &DX;
	uint src = OPER_PCIX_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_8_er_i(void)
{
	uint* r_dst = &DX;
	uint src = OPER_I_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_16_er_d(void)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_16(DY);
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_add_16_er_a(void)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_16(AY);
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_add_16_er_ai(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_AI_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_add_16_er_pi(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_PI_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_add_16_er_pd(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_PD_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_add_16_er_di(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_DI_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_add_16_er_ix(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_IX_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_add_16_er_aw(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AW_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_add_16_er_al(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AL_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_add_16_er_pcdi(void)
{
	uint* r_dst = &DX;
	uint src = OPER_PCDI_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_add_16_er_pcix(void)
{
	uint* r_dst = &DX;
	uint src = OPER_PCIX_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_add_16_er_i(void)
{
	uint* r_dst = &DX;
	uint src = OPER_I_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_add_32_er_d(void)
{
	uint* r_dst = &DX;
	uint src = DY;
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_add_32_er_a(void)
{
	uint* r_dst = &DX;
	uint src = AY;
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_add_32_er_ai(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_AI_32();
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_add_32_er_pi(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_PI_32();
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_add_32_er_pd(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_PD_32();
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_add_32_er_di(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_DI_32();
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_add_32_er_ix(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_IX_32();
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_add_32_er_aw(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AW_32();
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_add_32_er_al(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AL_32();
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_add_32_er_pcdi(void)
{
	uint* r_dst = &DX;
	uint src = OPER_PCDI_32();
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_add_32_er_pcix(void)
{
	uint* r_dst = &DX;
	uint src = OPER_PCIX_32();
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_add_32_er_i(void)
{
	uint* r_dst = &DX;
	uint src = OPER_I_32();
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_add_8_re_ai(void)
{
	uint ea = EA_AY_AI_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_add_8_re_pi(void)
{
	uint ea = EA_AY_PI_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_add_8_re_pi7(void)
{
	uint ea = EA_A7_PI_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_add_8_re_pd(void)
{
	uint ea = EA_AY_PD_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_add_8_re_pd7(void)
{
	uint ea = EA_A7_PD_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_add_8_re_di(void)
{
	uint ea = EA_AY_DI_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_add_8_re_ix(void)
{
	uint ea = EA_AY_IX_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_add_8_re_aw(void)
{
	uint ea = EA_AW_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_add_8_re_al(void)
{
	uint ea = EA_AL_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_add_16_re_ai(void)
{
	uint ea = EA_AY_AI_16();
	uint src = MASK_OUT_ABOVE_16(DX);
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_add_16_re_pi(void)
{
	uint ea = EA_AY_PI_16();
	uint src = MASK_OUT_ABOVE_16(DX);
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_add_16_re_pd(void)
{
	uint ea = EA_AY_PD_16();
	uint src = MASK_OUT_ABOVE_16(DX);
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_add_16_re_di(void)
{
	uint ea = EA_AY_DI_16();
	uint src = MASK_OUT_ABOVE_16(DX);
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_add_16_re_ix(void)
{
	uint ea = EA_AY_IX_16();
	uint src = MASK_OUT_ABOVE_16(DX);
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_add_16_re_aw(void)
{
	uint ea = EA_AW_16();
	uint src = MASK_OUT_ABOVE_16(DX);
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_add_16_re_al(void)
{
	uint ea = EA_AL_16();
	uint src = MASK_OUT_ABOVE_16(DX);
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_add_32_re_ai(void)
{
	uint ea = EA_AY_AI_32();
	uint src = DX;
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_add_32_re_pi(void)
{
	uint ea = EA_AY_PI_32();
	uint src = DX;
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_add_32_re_pd(void)
{
	uint ea = EA_AY_PD_32();
	uint src = DX;
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_add_32_re_di(void)
{
	uint ea = EA_AY_DI_32();
	uint src = DX;
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_add_32_re_ix(void)
{
	uint ea = EA_AY_IX_32();
	uint src = DX;
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_add_32_re_aw(void)
{
	uint ea = EA_AW_32();
	uint src = DX;
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_add_32_re_al(void)
{
	uint ea = EA_AL_32();
	uint src = DX;
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_adda_16_d(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + MAKE_INT_16(DY));
}


void m68k_op_adda_16_a(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + MAKE_INT_16(AY));
}


void m68k_op_adda_16_ai(void)
{
	uint* r_dst = &AX;
	uint src = MAKE_INT_16(OPER_AY_AI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68k_op_adda_16_pi(void)
{
	uint* r_dst = &AX;
	uint src = MAKE_INT_16(OPER_AY_PI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68k_op_adda_16_pd(void)
{
	uint* r_dst = &AX;
	uint src = MAKE_INT_16(OPER_AY_PD_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68k_op_adda_16_di(void)
{
	uint* r_dst = &AX;
	uint src = MAKE_INT_16(OPER_AY_DI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68k_op_adda_16_ix(void)
{
	uint* r_dst = &AX;
	uint src = MAKE_INT_16(OPER_AY_IX_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68k_op_adda_16_aw(void)
{
	uint* r_dst = &AX;
	uint src = MAKE_INT_16(OPER_AW_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68k_op_adda_16_al(void)
{
	uint* r_dst = &AX;
	uint src = MAKE_INT_16(OPER_AL_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68k_op_adda_16_pcdi(void)
{
	uint* r_dst = &AX;
	uint src = MAKE_INT_16(OPER_PCDI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68k_op_adda_16_pcix(void)
{
	uint* r_dst = &AX;
	uint src = MAKE_INT_16(OPER_PCIX_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68k_op_adda_16_i(void)
{
	uint* r_dst = &AX;
	uint src = MAKE_INT_16(OPER_I_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68k_op_adda_32_d(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + DY);
}


void m68k_op_adda_32_a(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + AY);
}


void m68k_op_adda_32_ai(void)
{
	uint src = OPER_AY_AI_32();
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68k_op_adda_32_pi(void)
{
	uint src = OPER_AY_PI_32();
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68k_op_adda_32_pd(void)
{
	uint src = OPER_AY_PD_32();
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68k_op_adda_32_di(void)
{
	uint src = OPER_AY_DI_32();
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68k_op_adda_32_ix(void)
{
	uint src = OPER_AY_IX_32();
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68k_op_adda_32_aw(void)
{
	uint src = OPER_AW_32();
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68k_op_adda_32_al(void)
{
	uint src = OPER_AL_32();
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68k_op_adda_32_pcdi(void)
{
	uint src = OPER_PCDI_32();
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68k_op_adda_32_pcix(void)
{
	uint src = OPER_PCIX_32();
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68k_op_adda_32_i(void)
{
	uint src = OPER_I_32();
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68k_op_addi_8_d(void)
{
	uint* r_dst = &DY;
	uint src = OPER_I_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_addi_8_ai(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_AI_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addi_8_pi(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_PI_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addi_8_pi7(void)
{
	uint src = OPER_I_8();
	uint ea = EA_A7_PI_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addi_8_pd(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addi_8_pd7(void)
{
	uint src = OPER_I_8();
	uint ea = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addi_8_di(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_DI_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addi_8_ix(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_IX_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addi_8_aw(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AW_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addi_8_al(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AL_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addi_16_d(void)
{
	uint* r_dst = &DY;
	uint src = OPER_I_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_addi_16_ai(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_AI_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addi_16_pi(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_PI_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addi_16_pd(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_PD_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addi_16_di(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_DI_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addi_16_ix(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_IX_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addi_16_aw(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AW_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addi_16_al(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AL_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addi_32_d(void)
{
	uint* r_dst = &DY;
	uint src = OPER_I_32();
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_addi_32_ai(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_AI_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addi_32_pi(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_PI_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addi_32_pd(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_PD_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addi_32_di(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_DI_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addi_32_ix(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_IX_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addi_32_aw(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AW_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addi_32_al(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AL_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addq_8_d(void)
{
	uint* r_dst = &DY;
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_addq_8_ai(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_AI_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addq_8_pi(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_PI_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addq_8_pi7(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_A7_PI_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addq_8_pd(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addq_8_pd7(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addq_8_di(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_DI_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addq_8_ix(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_IX_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addq_8_aw(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AW_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addq_8_al(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AL_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addq_16_d(void)
{
	uint* r_dst = &DY;
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_addq_16_a(void)
{
	uint* r_dst = &AY;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + (((REG_IR >> 9) - 1) & 7) + 1);
}


void m68k_op_addq_16_ai(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_AI_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addq_16_pi(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_PI_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addq_16_pd(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_PD_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addq_16_di(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_DI_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addq_16_ix(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_IX_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addq_16_aw(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AW_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addq_16_al(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AL_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addq_32_d(void)
{
	uint* r_dst = &DY;
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_addq_32_a(void)
{
	uint* r_dst = &AY;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + (((REG_IR >> 9) - 1) & 7) + 1);
}


void m68k_op_addq_32_ai(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_AI_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;


	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addq_32_pi(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_PI_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;


	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addq_32_pd(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_PD_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;


	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addq_32_di(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_DI_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;


	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addq_32_ix(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_IX_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;


	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addq_32_aw(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AW_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;


	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addq_32_al(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AL_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;


	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addx_8_rr(void)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_8(DY);
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
}


void m68k_op_addx_16_rr(void)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_16(DY);
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);

	res = MASK_OUT_ABOVE_16(res);
	FLAG_Z |= res;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
}


void m68k_op_addx_32_rr(void)
{
	uint* r_dst = &DX;
	uint src = DY;
	uint dst = *r_dst;
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);

	res = MASK_OUT_ABOVE_32(res);
	FLAG_Z |= res;

	*r_dst = res;
}


void m68k_op_addx_8_mm_ax7(void)
{
	uint src = OPER_AY_PD_8();
	uint ea  = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_addx_8_mm_ay7(void)
{
	uint src = OPER_A7_PD_8();
	uint ea  = EA_AX_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_addx_8_mm_axy7(void)
{
	uint src = OPER_A7_PD_8();
	uint ea  = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_addx_8_mm(void)
{
	uint src = OPER_AY_PD_8();
	uint ea  = EA_AX_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_addx_16_mm(void)
{
	uint src = OPER_AY_PD_16();
	uint ea  = EA_AX_PD_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);

	res = MASK_OUT_ABOVE_16(res);
	FLAG_Z |= res;

	m68ki_write_16(ea, res);
}


void m68k_op_addx_32_mm(void)
{
	uint src = OPER_AY_PD_32();
	uint ea  = EA_AX_PD_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);

	res = MASK_OUT_ABOVE_32(res);
	FLAG_Z |= res;

	m68ki_write_32(ea, res);
}


void m68k_op_and_8_er_d(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (DY | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_er_ai(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_AY_AI_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_er_pi(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_AY_PI_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_er_pi7(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_A7_PI_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_er_pd(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_AY_PD_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_er_pd7(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_A7_PD_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_er_di(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_AY_DI_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_er_ix(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_AY_IX_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_er_aw(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_AW_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_er_al(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_AL_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_er_pcdi(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_PCDI_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_er_pcix(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_PCIX_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_er_i(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_I_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_16_er_d(void)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DX &= (DY | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_16_er_ai(void)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_AY_AI_16() | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_16_er_pi(void)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_AY_PI_16() | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_16_er_pd(void)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_AY_PD_16() | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_16_er_di(void)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_AY_DI_16() | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_16_er_ix(void)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_AY_IX_16() | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_16_er_aw(void)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_AW_16() | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_16_er_al(void)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_AL_16() | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_16_er_pcdi(void)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_PCDI_16() | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_16_er_pcix(void)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_PCIX_16() | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_16_er_i(void)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_I_16() | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_32_er_d(void)
{
	FLAG_Z = DX &= DY;

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_32_er_ai(void)
{
	FLAG_Z = DX &= OPER_AY_AI_32();

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_32_er_pi(void)
{
	FLAG_Z = DX &= OPER_AY_PI_32();

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_32_er_pd(void)
{
	FLAG_Z = DX &= OPER_AY_PD_32();

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_32_er_di(void)
{
	FLAG_Z = DX &= OPER_AY_DI_32();

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_32_er_ix(void)
{
	FLAG_Z = DX &= OPER_AY_IX_32();

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_32_er_aw(void)
{
	FLAG_Z = DX &= OPER_AW_32();

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_32_er_al(void)
{
	FLAG_Z = DX &= OPER_AL_32();

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_32_er_pcdi(void)
{
	FLAG_Z = DX &= OPER_PCDI_32();

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_32_er_pcix(void)
{
	FLAG_Z = DX &= OPER_PCIX_32();

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_32_er_i(void)
{
	FLAG_Z = DX &= OPER_I_32();

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_re_ai(void)
{
	uint ea = EA_AY_AI_8();
	uint res = DX & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_and_8_re_pi(void)
{
	uint ea = EA_AY_PI_8();
	uint res = DX & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_and_8_re_pi7(void)
{
	uint ea = EA_A7_PI_8();
	uint res = DX & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_and_8_re_pd(void)
{
	uint ea = EA_AY_PD_8();
	uint res = DX & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_and_8_re_pd7(void)
{
	uint ea = EA_A7_PD_8();
	uint res = DX & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_and_8_re_di(void)
{
	uint ea = EA_AY_DI_8();
	uint res = DX & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_and_8_re_ix(void)
{
	uint ea = EA_AY_IX_8();
	uint res = DX & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_and_8_re_aw(void)
{
	uint ea = EA_AW_8();
	uint res = DX & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_and_8_re_al(void)
{
	uint ea = EA_AL_8();
	uint res = DX & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_and_16_re_ai(void)
{
	uint ea = EA_AY_AI_16();
	uint res = DX & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_and_16_re_pi(void)
{
	uint ea = EA_AY_PI_16();
	uint res = DX & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_and_16_re_pd(void)
{
	uint ea = EA_AY_PD_16();
	uint res = DX & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_and_16_re_di(void)
{
	uint ea = EA_AY_DI_16();
	uint res = DX & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_and_16_re_ix(void)
{
	uint ea = EA_AY_IX_16();
	uint res = DX & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_and_16_re_aw(void)
{
	uint ea = EA_AW_16();
	uint res = DX & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_and_16_re_al(void)
{
	uint ea = EA_AL_16();
	uint res = DX & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_and_32_re_ai(void)
{
	uint ea = EA_AY_AI_32();
	uint res = DX & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_and_32_re_pi(void)
{
	uint ea = EA_AY_PI_32();
	uint res = DX & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_and_32_re_pd(void)
{
	uint ea = EA_AY_PD_32();
	uint res = DX & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_and_32_re_di(void)
{
	uint ea = EA_AY_DI_32();
	uint res = DX & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_and_32_re_ix(void)
{
	uint ea = EA_AY_IX_32();
	uint res = DX & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_and_32_re_aw(void)
{
	uint ea = EA_AW_32();
	uint res = DX & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_and_32_re_al(void)
{
	uint ea = EA_AL_32();
	uint res = DX & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_andi_8_d(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DY &= (OPER_I_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_andi_8_ai(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_AI_8();
	uint res = src & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_8(ea, res);
}


void m68k_op_andi_8_pi(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_PI_8();
	uint res = src & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_8(ea, res);
}


void m68k_op_andi_8_pi7(void)
{
	uint src = OPER_I_8();
	uint ea = EA_A7_PI_8();
	uint res = src & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_8(ea, res);
}


void m68k_op_andi_8_pd(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_PD_8();
	uint res = src & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_8(ea, res);
}


void m68k_op_andi_8_pd7(void)
{
	uint src = OPER_I_8();
	uint ea = EA_A7_PD_8();
	uint res = src & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_8(ea, res);
}


void m68k_op_andi_8_di(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_DI_8();
	uint res = src & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_8(ea, res);
}


void m68k_op_andi_8_ix(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_IX_8();
	uint res = src & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_8(ea, res);
}


void m68k_op_andi_8_aw(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AW_8();
	uint res = src & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_8(ea, res);
}


void m68k_op_andi_8_al(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AL_8();
	uint res = src & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_8(ea, res);
}


void m68k_op_andi_16_d(void)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DY &= (OPER_I_16() | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_andi_16_ai(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_AI_16();
	uint res = src & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_16(ea, res);
}


void m68k_op_andi_16_pi(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_PI_16();
	uint res = src & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_16(ea, res);
}


void m68k_op_andi_16_pd(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_PD_16();
	uint res = src & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_16(ea, res);
}


void m68k_op_andi_16_di(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_DI_16();
	uint res = src & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_16(ea, res);
}


void m68k_op_andi_16_ix(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_IX_16();
	uint res = src & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_16(ea, res);
}


void m68k_op_andi_16_aw(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AW_16();
	uint res = src & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_16(ea, res);
}


void m68k_op_andi_16_al(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AL_16();
	uint res = src & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_16(ea, res);
}


void m68k_op_andi_32_d(void)
{
	FLAG_Z = DY &= (OPER_I_32());

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_andi_32_ai(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_AI_32();
	uint res = src & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_andi_32_pi(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_PI_32();
	uint res = src & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_andi_32_pd(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_PD_32();
	uint res = src & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_andi_32_di(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_DI_32();
	uint res = src & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_andi_32_ix(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_IX_32();
	uint res = src & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_andi_32_aw(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AW_32();
	uint res = src & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_andi_32_al(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AL_32();
	uint res = src & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_andi_16_toc(void)
{
	m68ki_set_ccr(m68ki_get_ccr() & OPER_I_8());
}


void m68k_op_andi_16_tos(void)
{
	if(FLAG_S)
	{
		uint src = OPER_I_16();
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(m68ki_get_sr() & src);
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_asr_8_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src >> shift;

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	if(GET_MSB_8(src))
		res |= m68ki_shift_8_table[shift];

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_X = FLAG_C = src << (9-shift);
}


void m68k_op_asr_16_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src >> shift;

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	if(GET_MSB_16(src))
		res |= m68ki_shift_16_table[shift];

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_X = FLAG_C = src << (9-shift);
}


void m68k_op_asr_32_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = *r_dst;
	uint res = src >> shift;

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	if(GET_MSB_32(src))
		res |= m68ki_shift_32_table[shift];

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_X = FLAG_C = src << (9-shift);
}


void m68k_op_asr_8_r(void)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src >> shift;

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift < 8)
		{
			if(GET_MSB_8(src))
				res |= m68ki_shift_8_table[shift];

			*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

			FLAG_X = FLAG_C = src << (9-shift);
			FLAG_N = NFLAG_8(res);
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		if(GET_MSB_8(src))
		{
			*r_dst |= 0xff;
			FLAG_C = CFLAG_SET;
			FLAG_X = XFLAG_SET;
			FLAG_N = NFLAG_SET;
			FLAG_Z = ZFLAG_CLEAR;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		*r_dst &= 0xffffff00;
		FLAG_C = CFLAG_CLEAR;
		FLAG_X = XFLAG_CLEAR;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_8(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_asr_16_r(void)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src >> shift;

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift < 16)
		{
			if(GET_MSB_16(src))
				res |= m68ki_shift_16_table[shift];

			*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

			FLAG_C = FLAG_X = (src >> (shift - 1))<<8;
			FLAG_N = NFLAG_16(res);
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		if(GET_MSB_16(src))
		{
			*r_dst |= 0xffff;
			FLAG_C = CFLAG_SET;
			FLAG_X = XFLAG_SET;
			FLAG_N = NFLAG_SET;
			FLAG_Z = ZFLAG_CLEAR;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		*r_dst &= 0xffff0000;
		FLAG_C = CFLAG_CLEAR;
		FLAG_X = XFLAG_CLEAR;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_16(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_asr_32_r(void)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = *r_dst;
	uint res = src >> shift;

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift < 32)
		{
			if(GET_MSB_32(src))
				res |= m68ki_shift_32_table[shift];

			*r_dst = res;

			FLAG_C = FLAG_X = (src >> (shift - 1))<<8;
			FLAG_N = NFLAG_32(res);
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		if(GET_MSB_32(src))
		{
			*r_dst = 0xffffffff;
			FLAG_C = CFLAG_SET;
			FLAG_X = XFLAG_SET;
			FLAG_N = NFLAG_SET;
			FLAG_Z = ZFLAG_CLEAR;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		*r_dst = 0;
		FLAG_C = CFLAG_CLEAR;
		FLAG_X = XFLAG_CLEAR;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_32(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_asr_16_ai(void)
{
	uint ea = EA_AY_AI_16();
	uint src = m68ki_read_16(ea);
	uint res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = FLAG_X = src << 8;
}


void m68k_op_asr_16_pi(void)
{
	uint ea = EA_AY_PI_16();
	uint src = m68ki_read_16(ea);
	uint res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = FLAG_X = src << 8;
}


void m68k_op_asr_16_pd(void)
{
	uint ea = EA_AY_PD_16();
	uint src = m68ki_read_16(ea);
	uint res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = FLAG_X = src << 8;
}


void m68k_op_asr_16_di(void)
{
	uint ea = EA_AY_DI_16();
	uint src = m68ki_read_16(ea);
	uint res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = FLAG_X = src << 8;
}


void m68k_op_asr_16_ix(void)
{
	uint ea = EA_AY_IX_16();
	uint src = m68ki_read_16(ea);
	uint res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = FLAG_X = src << 8;
}


void m68k_op_asr_16_aw(void)
{
	uint ea = EA_AW_16();
	uint src = m68ki_read_16(ea);
	uint res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = FLAG_X = src << 8;
}


void m68k_op_asr_16_al(void)
{
	uint ea = EA_AL_16();
	uint src = m68ki_read_16(ea);
	uint res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = FLAG_X = src << 8;
}


void m68k_op_asl_8_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = MASK_OUT_ABOVE_8(src << shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_X = FLAG_C = src << shift;
	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	src &= m68ki_shift_8_table[shift + 1];
	FLAG_V = (!(src == 0 || (src == m68ki_shift_8_table[shift + 1] && shift < 8)))<<7;
}


void m68k_op_asl_16_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = MASK_OUT_ABOVE_16(src << shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> (8-shift);
	src &= m68ki_shift_16_table[shift + 1];
	FLAG_V = (!(src == 0 || src == m68ki_shift_16_table[shift + 1]))<<7;
}


void m68k_op_asl_32_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = *r_dst;
	uint res = MASK_OUT_ABOVE_32(src << shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> (24-shift);
	src &= m68ki_shift_32_table[shift + 1];
	FLAG_V = (!(src == 0 || src == m68ki_shift_32_table[shift + 1]))<<7;
}


void m68k_op_asl_8_r(void)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = MASK_OUT_ABOVE_8(src << shift);

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift < 8)
		{
			*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
			FLAG_X = FLAG_C = src << shift;
			FLAG_N = NFLAG_8(res);
			FLAG_Z = res;
			src &= m68ki_shift_8_table[shift + 1];
			FLAG_V = (!(src == 0 || src == m68ki_shift_8_table[shift + 1]))<<7;
			return;
		}

		*r_dst &= 0xffffff00;
		FLAG_X = FLAG_C = ((shift == 8 ? src & 1 : 0))<<8;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = (!(src == 0))<<7;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_8(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_asl_16_r(void)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = MASK_OUT_ABOVE_16(src << shift);

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift < 16)
		{
			*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
			FLAG_X = FLAG_C = (src << shift) >> 8;
			FLAG_N = NFLAG_16(res);
			FLAG_Z = res;
			src &= m68ki_shift_16_table[shift + 1];
			FLAG_V = (!(src == 0 || src == m68ki_shift_16_table[shift + 1]))<<7;
			return;
		}

		*r_dst &= 0xffff0000;
		FLAG_X = FLAG_C = ((shift == 16 ? src & 1 : 0))<<8;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = (!(src == 0))<<7;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_16(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_asl_32_r(void)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = *r_dst;
	uint res = MASK_OUT_ABOVE_32(src << shift);

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift < 32)
		{
			*r_dst = res;
			FLAG_X = FLAG_C = (src >> (32 - shift)) << 8;
			FLAG_N = NFLAG_32(res);
			FLAG_Z = res;
			src &= m68ki_shift_32_table[shift + 1];
			FLAG_V = (!(src == 0 || src == m68ki_shift_32_table[shift + 1]))<<7;
			return;
		}

		*r_dst = 0;
		FLAG_X = FLAG_C = ((shift == 32 ? src & 1 : 0))<<8;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = (!(src == 0))<<7;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_32(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_asl_16_ai(void)
{
	uint ea = EA_AY_AI_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> 7;
	src &= 0xc000;
	FLAG_V = (!(src == 0 || src == 0xc000))<<7;
}


void m68k_op_asl_16_pi(void)
{
	uint ea = EA_AY_PI_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> 7;
	src &= 0xc000;
	FLAG_V = (!(src == 0 || src == 0xc000))<<7;
}


void m68k_op_asl_16_pd(void)
{
	uint ea = EA_AY_PD_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> 7;
	src &= 0xc000;
	FLAG_V = (!(src == 0 || src == 0xc000))<<7;
}


void m68k_op_asl_16_di(void)
{
	uint ea = EA_AY_DI_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> 7;
	src &= 0xc000;
	FLAG_V = (!(src == 0 || src == 0xc000))<<7;
}


void m68k_op_asl_16_ix(void)
{
	uint ea = EA_AY_IX_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> 7;
	src &= 0xc000;
	FLAG_V = (!(src == 0 || src == 0xc000))<<7;
}


void m68k_op_asl_16_aw(void)
{
	uint ea = EA_AW_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> 7;
	src &= 0xc000;
	FLAG_V = (!(src == 0 || src == 0xc000))<<7;
}


void m68k_op_asl_16_al(void)
{
	uint ea = EA_AL_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> 7;
	src &= 0xc000;
	FLAG_V = (!(src == 0 || src == 0xc000))<<7;
}


void m68k_op_bhi_8(void)
{
	if(COND_HI())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_bls_8(void)
{
	if(COND_LS())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_bcc_8(void)
{
	if(COND_CC())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_bcs_8(void)
{
	if(COND_CS())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_bne_8(void)
{
	if(COND_NE())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_beq_8(void)
{
	if(COND_EQ())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_bvc_8(void)
{
	if(COND_VC())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_bvs_8(void)
{
	if(COND_VS())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_bpl_8(void)
{
	if(COND_PL())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_bmi_8(void)
{
	if(COND_MI())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_bge_8(void)
{
	if(COND_GE())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_blt_8(void)
{
	if(COND_LT())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_bgt_8(void)
{
	if(COND_GT())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_ble_8(void)
{
	if(COND_LE())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_bhi_16(void)
{
	if(COND_HI())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_bls_16(void)
{
	if(COND_LS())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_bcc_16(void)
{
	if(COND_CC())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_bcs_16(void)
{
	if(COND_CS())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_bne_16(void)
{
	if(COND_NE())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_beq_16(void)
{
	if(COND_EQ())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_bvc_16(void)
{
	if(COND_VC())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_bvs_16(void)
{
	if(COND_VS())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_bpl_16(void)
{
	if(COND_PL())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_bmi_16(void)
{
	if(COND_MI())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_bge_16(void)
{
	if(COND_GE())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_blt_16(void)
{
	if(COND_LT())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_bgt_16(void)
{
	if(COND_GT())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_ble_16(void)
{
	if(COND_LE())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_bhi_32(void)
{

		if(COND_HI())
		{
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
			return;
		}
		USE_CYCLES(CYC_BCC_NOTAKE_B);

}


void m68k_op_bls_32(void)
{

		if(COND_LS())
		{
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
			return;
		}
		USE_CYCLES(CYC_BCC_NOTAKE_B);

}


void m68k_op_bcc_32(void)
{

		if(COND_CC())
		{
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
			return;
		}
		USE_CYCLES(CYC_BCC_NOTAKE_B);

}


void m68k_op_bcs_32(void)
{

		if(COND_CS())
		{
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
			return;
		}
		USE_CYCLES(CYC_BCC_NOTAKE_B);

}


void m68k_op_bne_32(void)
{

		if(COND_NE())
		{
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
			return;
		}
		USE_CYCLES(CYC_BCC_NOTAKE_B);

}


void m68k_op_beq_32(void)
{

		if(COND_EQ())
		{
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
			return;
		}
		USE_CYCLES(CYC_BCC_NOTAKE_B);
	
}


void m68k_op_bvc_32(void)
{

		if(COND_VC())
		{
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
			return;
		}
		USE_CYCLES(CYC_BCC_NOTAKE_B);

}


void m68k_op_bvs_32(void)
{

		if(COND_VS())
		{
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
			return;
		}
		USE_CYCLES(CYC_BCC_NOTAKE_B);
	
}


void m68k_op_bpl_32(void)
{

		if(COND_PL())
		{
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
			return;
		}
		USE_CYCLES(CYC_BCC_NOTAKE_B);
	
}


void m68k_op_bmi_32(void)
{

		if(COND_MI())
		{
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
			return;
		}
		USE_CYCLES(CYC_BCC_NOTAKE_B);
	
}


void m68k_op_bge_32(void)
{

		if(COND_GE())
		{
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
			return;
		}
		USE_CYCLES(CYC_BCC_NOTAKE_B);
	
}


void m68k_op_blt_32(void)
{

		if(COND_LT())
		{
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
			return;
		}
		USE_CYCLES(CYC_BCC_NOTAKE_B);
	
}


void m68k_op_bgt_32(void)
{

		if(COND_GT())
		{
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
			return;
		}
		USE_CYCLES(CYC_BCC_NOTAKE_B);
	
}


void m68k_op_ble_32(void)
{

		if(COND_LE())
		{
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
			return;
		}
		USE_CYCLES(CYC_BCC_NOTAKE_B);
	
}


void m68k_op_bchg_32_r_d(void)
{
	uint* r_dst = &DY;
	uint mask = 1 << (DX & 0x1f);

	FLAG_Z = *r_dst & mask;
	*r_dst ^= mask;
}


void m68k_op_bchg_8_r_ai(void)
{
	uint ea = EA_AY_AI_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_r_pi(void)
{
	uint ea = EA_AY_PI_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_r_pi7(void)
{
	uint ea = EA_A7_PI_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_r_pd(void)
{
	uint ea = EA_AY_PD_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_r_pd7(void)
{
	uint ea = EA_A7_PD_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_r_di(void)
{
	uint ea = EA_AY_DI_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_r_ix(void)
{
	uint ea = EA_AY_IX_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_r_aw(void)
{
	uint ea = EA_AW_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_r_al(void)
{
	uint ea = EA_AL_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_32_s_d(void)
{
	uint* r_dst = &DY;
	uint mask = 1 << (OPER_I_8() & 0x1f);

	FLAG_Z = *r_dst & mask;
	*r_dst ^= mask;
}


void m68k_op_bchg_8_s_ai(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_AI_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_s_pi(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_PI_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_s_pi7(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_A7_PI_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_s_pd(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_PD_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_s_pd7(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_A7_PD_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_s_di(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_DI_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_s_ix(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_IX_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_s_aw(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AW_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_s_al(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AL_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bclr_32_r_d(void)
{
	uint* r_dst = &DY;
	uint mask = 1 << (DX & 0x1f);

	FLAG_Z = *r_dst & mask;
	*r_dst &= ~mask;
}


void m68k_op_bclr_8_r_ai(void)
{
	uint ea = EA_AY_AI_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_r_pi(void)
{
	uint ea = EA_AY_PI_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_r_pi7(void)
{
	uint ea = EA_A7_PI_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_r_pd(void)
{
	uint ea = EA_AY_PD_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_r_pd7(void)
{
	uint ea = EA_A7_PD_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_r_di(void)
{
	uint ea = EA_AY_DI_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_r_ix(void)
{
	uint ea = EA_AY_IX_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_r_aw(void)
{
	uint ea = EA_AW_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_r_al(void)
{
	uint ea = EA_AL_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_32_s_d(void)
{
	uint* r_dst = &DY;
	uint mask = 1 << (OPER_I_8() & 0x1f);

	FLAG_Z = *r_dst & mask;
	*r_dst &= ~mask;
}


void m68k_op_bclr_8_s_ai(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_AI_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_s_pi(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_PI_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_s_pi7(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_A7_PI_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_s_pd(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_PD_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_s_pd7(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_A7_PD_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_s_di(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_DI_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_s_ix(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_IX_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_s_aw(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AW_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_s_al(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AL_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bfchg_32_d(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfchg_32_ai(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfchg_32_di(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfchg_32_ix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfchg_32_aw(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfchg_32_al(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfclr_32_d(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfclr_32_ai(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfclr_32_di(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfclr_32_ix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfclr_32_aw(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfclr_32_al(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfexts_32_d(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfexts_32_ai(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfexts_32_di(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfexts_32_ix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfexts_32_aw(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfexts_32_al(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfexts_32_pcdi(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfexts_32_pcix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfextu_32_d(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfextu_32_ai(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfextu_32_di(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfextu_32_ix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfextu_32_aw(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfextu_32_al(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfextu_32_pcdi(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfextu_32_pcix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfffo_32_d(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfffo_32_ai(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfffo_32_di(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfffo_32_ix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfffo_32_aw(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfffo_32_al(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfffo_32_pcdi(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfffo_32_pcix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfins_32_d(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfins_32_ai(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfins_32_di(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfins_32_ix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfins_32_aw(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfins_32_al(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfset_32_d(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfset_32_ai(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfset_32_di(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfset_32_ix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfset_32_aw(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bfset_32_al(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bftst_32_d(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bftst_32_ai(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bftst_32_di(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bftst_32_ix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bftst_32_aw(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bftst_32_al(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bftst_32_pcdi(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bftst_32_pcix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bkpt(void)
{

	m68ki_exception_illegal();
}


void m68k_op_bra_8(void)
{
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
	if(REG_PC == REG_PPC)
		USE_ALL_CYCLES();
}


void m68k_op_bra_16(void)
{
	uint offset = OPER_I_16();
	REG_PC -= 2;
	m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
	m68ki_branch_16(offset);
	if(REG_PC == REG_PPC)
		USE_ALL_CYCLES();
}


void m68k_op_bra_32(void)
{

		m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		if(REG_PC == REG_PPC)
			USE_ALL_CYCLES();
	
}


void m68k_op_bset_32_r_d(void)
{
	uint* r_dst = &DY;
	uint mask = 1 << (DX & 0x1f);

	FLAG_Z = *r_dst & mask;
	*r_dst |= mask;
}


void m68k_op_bset_8_r_ai(void)
{
	uint ea = EA_AY_AI_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_r_pi(void)
{
	uint ea = EA_AY_PI_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_r_pi7(void)
{
	uint ea = EA_A7_PI_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_r_pd(void)
{
	uint ea = EA_AY_PD_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_r_pd7(void)
{
	uint ea = EA_A7_PD_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_r_di(void)
{
	uint ea = EA_AY_DI_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_r_ix(void)
{
	uint ea = EA_AY_IX_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_r_aw(void)
{
	uint ea = EA_AW_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_r_al(void)
{
	uint ea = EA_AL_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_32_s_d(void)
{
	uint* r_dst = &DY;
	uint mask = 1 << (OPER_I_8() & 0x1f);

	FLAG_Z = *r_dst & mask;
	*r_dst |= mask;
}


void m68k_op_bset_8_s_ai(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_AI_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_s_pi(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_PI_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_s_pi7(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_A7_PI_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_s_pd(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_PD_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_s_pd7(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_A7_PD_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_s_di(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_DI_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_s_ix(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_IX_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_s_aw(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AW_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_s_al(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AL_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bsr_8(void)
{
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(REG_PC);
	m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
}


void m68k_op_bsr_16(void)
{
	uint offset = OPER_I_16();
	m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(REG_PC);
	REG_PC -= 2;
	m68ki_branch_16(offset);
}


void m68k_op_bsr_32(void)
{

		m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
		m68ki_push_32(REG_PC);
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
	
}


void m68k_op_btst_32_r_d(void)
{
	FLAG_Z = DY & (1 << (DX & 0x1f));
}


void m68k_op_btst_8_r_ai(void)
{
	FLAG_Z = OPER_AY_AI_8() & (1 << (DX & 7));
}


void m68k_op_btst_8_r_pi(void)
{
	FLAG_Z = OPER_AY_PI_8() & (1 << (DX & 7));
}


void m68k_op_btst_8_r_pi7(void)
{
	FLAG_Z = OPER_A7_PI_8() & (1 << (DX & 7));
}


void m68k_op_btst_8_r_pd(void)
{
	FLAG_Z = OPER_AY_PD_8() & (1 << (DX & 7));
}


void m68k_op_btst_8_r_pd7(void)
{
	FLAG_Z = OPER_A7_PD_8() & (1 << (DX & 7));
}


void m68k_op_btst_8_r_di(void)
{
	FLAG_Z = OPER_AY_DI_8() & (1 << (DX & 7));
}


void m68k_op_btst_8_r_ix(void)
{
	FLAG_Z = OPER_AY_IX_8() & (1 << (DX & 7));
}


void m68k_op_btst_8_r_aw(void)
{
	FLAG_Z = OPER_AW_8() & (1 << (DX & 7));
}


void m68k_op_btst_8_r_al(void)
{
	FLAG_Z = OPER_AL_8() & (1 << (DX & 7));
}


void m68k_op_btst_8_r_pcdi(void)
{
	FLAG_Z = OPER_PCDI_8() & (1 << (DX & 7));
}


void m68k_op_btst_8_r_pcix(void)
{
	FLAG_Z = OPER_PCIX_8() & (1 << (DX & 7));
}


void m68k_op_btst_8_r_i(void)
{
	FLAG_Z = OPER_I_8() & (1 << (DX & 7));
}


void m68k_op_btst_32_s_d(void)
{
	FLAG_Z = DY & (1 << (OPER_I_8() & 0x1f));
}


void m68k_op_btst_8_s_ai(void)
{
	uint bit = OPER_I_8() & 7;

	FLAG_Z = OPER_AY_AI_8() & (1 << bit);
}


void m68k_op_btst_8_s_pi(void)
{
	uint bit = OPER_I_8() & 7;

	FLAG_Z = OPER_AY_PI_8() & (1 << bit);
}


void m68k_op_btst_8_s_pi7(void)
{
	uint bit = OPER_I_8() & 7;

	FLAG_Z = OPER_A7_PI_8() & (1 << bit);
}


void m68k_op_btst_8_s_pd(void)
{
	uint bit = OPER_I_8() & 7;

	FLAG_Z = OPER_AY_PD_8() & (1 << bit);
}


void m68k_op_btst_8_s_pd7(void)
{
	uint bit = OPER_I_8() & 7;

	FLAG_Z = OPER_A7_PD_8() & (1 << bit);
}


void m68k_op_btst_8_s_di(void)
{
	uint bit = OPER_I_8() & 7;

	FLAG_Z = OPER_AY_DI_8() & (1 << bit);
}


void m68k_op_btst_8_s_ix(void)
{
	uint bit = OPER_I_8() & 7;

	FLAG_Z = OPER_AY_IX_8() & (1 << bit);
}


void m68k_op_btst_8_s_aw(void)
{
	uint bit = OPER_I_8() & 7;

	FLAG_Z = OPER_AW_8() & (1 << bit);
}


void m68k_op_btst_8_s_al(void)
{
	uint bit = OPER_I_8() & 7;

	FLAG_Z = OPER_AL_8() & (1 << bit);
}


void m68k_op_btst_8_s_pcdi(void)
{
	uint bit = OPER_I_8() & 7;

	FLAG_Z = OPER_PCDI_8() & (1 << bit);
}


void m68k_op_btst_8_s_pcix(void)
{
	uint bit = OPER_I_8() & 7;

	FLAG_Z = OPER_PCIX_8() & (1 << bit);
}


void m68k_op_callm_32_ai(void)
{

	m68ki_exception_illegal();
}


void m68k_op_callm_32_di(void)
{

	m68ki_exception_illegal();
}


void m68k_op_callm_32_ix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_callm_32_aw(void)
{

	m68ki_exception_illegal();
}


void m68k_op_callm_32_al(void)
{

	m68ki_exception_illegal();
}


void m68k_op_callm_32_pcdi(void)
{

	m68ki_exception_illegal();
}


void m68k_op_callm_32_pcix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas_8_ai(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas_8_pi(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas_8_pi7(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas_8_pd(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas_8_pd7(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas_8_di(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas_8_ix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas_8_aw(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas_8_al(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas_16_ai(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas_16_pi(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas_16_pd(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas_16_di(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas_16_ix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas_16_aw(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas_16_al(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas_32_ai(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas_32_pi(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas_32_pd(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas_32_di(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas_32_ix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas_32_aw(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas_32_al(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas2_16(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cas2_32(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk_16_d(void)
{
	sint src = MAKE_INT_16(DX);
	sint bound = MAKE_INT_16(DY);

	FLAG_Z = ZFLAG_16(src); /* Undocumented */
	FLAG_V = VFLAG_CLEAR;   /* Undocumented */
	FLAG_C = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	FLAG_N = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);
}


void m68k_op_chk_16_ai(void)
{
	sint src = MAKE_INT_16(DX);
	sint bound = MAKE_INT_16(OPER_AY_AI_16());

	FLAG_Z = ZFLAG_16(src); /* Undocumented */
	FLAG_V = VFLAG_CLEAR;   /* Undocumented */
	FLAG_C = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	FLAG_N = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);
}


void m68k_op_chk_16_pi(void)
{
	sint src = MAKE_INT_16(DX);
	sint bound = MAKE_INT_16(OPER_AY_PI_16());

	FLAG_Z = ZFLAG_16(src); /* Undocumented */
	FLAG_V = VFLAG_CLEAR;   /* Undocumented */
	FLAG_C = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	FLAG_N = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);
}


void m68k_op_chk_16_pd(void)
{
	sint src = MAKE_INT_16(DX);
	sint bound = MAKE_INT_16(OPER_AY_PD_16());

	FLAG_Z = ZFLAG_16(src); /* Undocumented */
	FLAG_V = VFLAG_CLEAR;   /* Undocumented */
	FLAG_C = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	FLAG_N = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);
}


void m68k_op_chk_16_di(void)
{
	sint src = MAKE_INT_16(DX);
	sint bound = MAKE_INT_16(OPER_AY_DI_16());

	FLAG_Z = ZFLAG_16(src); /* Undocumented */
	FLAG_V = VFLAG_CLEAR;   /* Undocumented */
	FLAG_C = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	FLAG_N = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);
}


void m68k_op_chk_16_ix(void)
{
	sint src = MAKE_INT_16(DX);
	sint bound = MAKE_INT_16(OPER_AY_IX_16());

	FLAG_Z = ZFLAG_16(src); /* Undocumented */
	FLAG_V = VFLAG_CLEAR;   /* Undocumented */
	FLAG_C = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	FLAG_N = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);
}


void m68k_op_chk_16_aw(void)
{
	sint src = MAKE_INT_16(DX);
	sint bound = MAKE_INT_16(OPER_AW_16());

	FLAG_Z = ZFLAG_16(src); /* Undocumented */
	FLAG_V = VFLAG_CLEAR;   /* Undocumented */
	FLAG_C = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	FLAG_N = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);
}


void m68k_op_chk_16_al(void)
{
	sint src = MAKE_INT_16(DX);
	sint bound = MAKE_INT_16(OPER_AL_16());

	FLAG_Z = ZFLAG_16(src); /* Undocumented */
	FLAG_V = VFLAG_CLEAR;   /* Undocumented */
	FLAG_C = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	FLAG_N = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);
}


void m68k_op_chk_16_pcdi(void)
{
	sint src = MAKE_INT_16(DX);
	sint bound = MAKE_INT_16(OPER_PCDI_16());

	FLAG_Z = ZFLAG_16(src); /* Undocumented */
	FLAG_V = VFLAG_CLEAR;   /* Undocumented */
	FLAG_C = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	FLAG_N = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);
}


void m68k_op_chk_16_pcix(void)
{
	sint src = MAKE_INT_16(DX);
	sint bound = MAKE_INT_16(OPER_PCIX_16());

	FLAG_Z = ZFLAG_16(src); /* Undocumented */
	FLAG_V = VFLAG_CLEAR;   /* Undocumented */
	FLAG_C = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	FLAG_N = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);
}


void m68k_op_chk_16_i(void)
{
	sint src = MAKE_INT_16(DX);
	sint bound = MAKE_INT_16(OPER_I_16());

	FLAG_Z = ZFLAG_16(src); /* Undocumented */
	FLAG_V = VFLAG_CLEAR;   /* Undocumented */
	FLAG_C = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	FLAG_N = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);
}


void m68k_op_chk_32_d(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk_32_ai(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk_32_pi(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk_32_pd(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk_32_di(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk_32_ix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk_32_aw(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk_32_al(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk_32_pcdi(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk_32_pcix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk_32_i(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_8_pcdi(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_8_pcix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_8_ai(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_8_di(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_8_ix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_8_aw(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_8_al(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_16_pcdi(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_16_pcix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_16_ai(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_16_di(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_16_ix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_16_aw(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_16_al(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_32_pcdi(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_32_pcix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_32_ai(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_32_di(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_32_ix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_32_aw(void)
{

	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_32_al(void)
{

	m68ki_exception_illegal();
}


void m68k_op_clr_8_d(void)
{
	DY &= 0xffffff00;

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_8_ai(void)
{
	m68ki_write_8(EA_AY_AI_8(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_8_pi(void)
{
	m68ki_write_8(EA_AY_PI_8(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_8_pi7(void)
{
	m68ki_write_8(EA_A7_PI_8(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_8_pd(void)
{
	m68ki_write_8(EA_AY_PD_8(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_8_pd7(void)
{
	m68ki_write_8(EA_A7_PD_8(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_8_di(void)
{
	m68ki_write_8(EA_AY_DI_8(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_8_ix(void)
{
	m68ki_write_8(EA_AY_IX_8(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_8_aw(void)
{
	m68ki_write_8(EA_AW_8(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_8_al(void)
{
	m68ki_write_8(EA_AL_8(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_16_d(void)
{
	DY &= 0xffff0000;

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_16_ai(void)
{
	m68ki_write_16(EA_AY_AI_16(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_16_pi(void)
{
	m68ki_write_16(EA_AY_PI_16(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_16_pd(void)
{
	m68ki_write_16(EA_AY_PD_16(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_16_di(void)
{
	m68ki_write_16(EA_AY_DI_16(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_16_ix(void)
{
	m68ki_write_16(EA_AY_IX_16(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_16_aw(void)
{
	m68ki_write_16(EA_AW_16(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_16_al(void)
{
	m68ki_write_16(EA_AL_16(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_32_d(void)
{
	DY = 0;

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_32_ai(void)
{
	m68ki_write_32(EA_AY_AI_32(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_32_pi(void)
{
	m68ki_write_32(EA_AY_PI_32(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_32_pd(void)
{
	m68ki_write_32(EA_AY_PD_32(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_32_di(void)
{
	m68ki_write_32(EA_AY_DI_32(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_32_ix(void)
{
	m68ki_write_32(EA_AY_IX_32(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_32_aw(void)
{
	m68ki_write_32(EA_AW_32(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_32_al(void)
{
	m68ki_write_32(EA_AL_32(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_cmp_8_d(void)
{
	uint src = MASK_OUT_ABOVE_8(DY);
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_8_ai(void)
{
	uint src = OPER_AY_AI_8();
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_8_pi(void)
{
	uint src = OPER_AY_PI_8();
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_8_pi7(void)
{
	uint src = OPER_A7_PI_8();
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_8_pd(void)
{
	uint src = OPER_AY_PD_8();
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_8_pd7(void)
{
	uint src = OPER_A7_PD_8();
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_8_di(void)
{
	uint src = OPER_AY_DI_8();
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_8_ix(void)
{
	uint src = OPER_AY_IX_8();
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_8_aw(void)
{
	uint src = OPER_AW_8();
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_8_al(void)
{
	uint src = OPER_AL_8();
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_8_pcdi(void)
{
	uint src = OPER_PCDI_8();
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_8_pcix(void)
{
	uint src = OPER_PCIX_8();
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_8_i(void)
{
	uint src = OPER_I_8();
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_16_d(void)
{
	uint src = MASK_OUT_ABOVE_16(DY);
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmp_16_a(void)
{
	uint src = MASK_OUT_ABOVE_16(AY);
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmp_16_ai(void)
{
	uint src = OPER_AY_AI_16();
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmp_16_pi(void)
{
	uint src = OPER_AY_PI_16();
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmp_16_pd(void)
{
	uint src = OPER_AY_PD_16();
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmp_16_di(void)
{
	uint src = OPER_AY_DI_16();
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmp_16_ix(void)
{
	uint src = OPER_AY_IX_16();
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmp_16_aw(void)
{
	uint src = OPER_AW_16();
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmp_16_al(void)
{
	uint src = OPER_AL_16();
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmp_16_pcdi(void)
{
	uint src = OPER_PCDI_16();
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmp_16_pcix(void)
{
	uint src = OPER_PCIX_16();
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmp_16_i(void)
{
	uint src = OPER_I_16();
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmp_32_d(void)
{
	uint src = DY;
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmp_32_a(void)
{
	uint src = AY;
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmp_32_ai(void)
{
	uint src = OPER_AY_AI_32();
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmp_32_pi(void)
{
	uint src = OPER_AY_PI_32();
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmp_32_pd(void)
{
	uint src = OPER_AY_PD_32();
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmp_32_di(void)
{
	uint src = OPER_AY_DI_32();
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmp_32_ix(void)
{
	uint src = OPER_AY_IX_32();
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmp_32_aw(void)
{
	uint src = OPER_AW_32();
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmp_32_al(void)
{
	uint src = OPER_AL_32();
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmp_32_pcdi(void)
{
	uint src = OPER_PCDI_32();
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmp_32_pcix(void)
{
	uint src = OPER_PCIX_32();
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmp_32_i(void)
{
	uint src = OPER_I_32();
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_16_d(void)
{
	uint src = MAKE_INT_16(DY);
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_16_a(void)
{
	uint src = MAKE_INT_16(AY);
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_16_ai(void)
{
	uint src = MAKE_INT_16(OPER_AY_AI_16());
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_16_pi(void)
{
	uint src = MAKE_INT_16(OPER_AY_PI_16());
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_16_pd(void)
{
	uint src = MAKE_INT_16(OPER_AY_PD_16());
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_16_di(void)
{
	uint src = MAKE_INT_16(OPER_AY_DI_16());
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_16_ix(void)
{
	uint src = MAKE_INT_16(OPER_AY_IX_16());
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_16_aw(void)
{
	uint src = MAKE_INT_16(OPER_AW_16());
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_16_al(void)
{
	uint src = MAKE_INT_16(OPER_AL_16());
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_16_pcdi(void)
{
	uint src = MAKE_INT_16(OPER_PCDI_16());
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_16_pcix(void)
{
	uint src = MAKE_INT_16(OPER_PCIX_16());
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_16_i(void)
{
	uint src = MAKE_INT_16(OPER_I_16());
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_32_d(void)
{
	uint src = DY;
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_32_a(void)
{
	uint src = AY;
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_32_ai(void)
{
	uint src = OPER_AY_AI_32();
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_32_pi(void)
{
	uint src = OPER_AY_PI_32();
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_32_pd(void)
{
	uint src = OPER_AY_PD_32();
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_32_di(void)
{
	uint src = OPER_AY_DI_32();
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_32_ix(void)
{
	uint src = OPER_AY_IX_32();
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_32_aw(void)
{
	uint src = OPER_AW_32();
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_32_al(void)
{
	uint src = OPER_AL_32();
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_32_pcdi(void)
{
	uint src = OPER_PCDI_32();
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_32_pcix(void)
{
	uint src = OPER_PCIX_32();
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_32_i(void)
{
	uint src = OPER_I_32();
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpi_8_d(void)
{
	uint src = OPER_I_8();
	uint dst = MASK_OUT_ABOVE_8(DY);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpi_8_ai(void)
{
	uint src = OPER_I_8();
	uint dst = OPER_AY_AI_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpi_8_pi(void)
{
	uint src = OPER_I_8();
	uint dst = OPER_AY_PI_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpi_8_pi7(void)
{
	uint src = OPER_I_8();
	uint dst = OPER_A7_PI_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpi_8_pd(void)
{
	uint src = OPER_I_8();
	uint dst = OPER_AY_PD_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpi_8_pd7(void)
{
	uint src = OPER_I_8();
	uint dst = OPER_A7_PD_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpi_8_di(void)
{
	uint src = OPER_I_8();
	uint dst = OPER_AY_DI_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpi_8_ix(void)
{
	uint src = OPER_I_8();
	uint dst = OPER_AY_IX_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpi_8_aw(void)
{
	uint src = OPER_I_8();
	uint dst = OPER_AW_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpi_8_al(void)
{
	uint src = OPER_I_8();
	uint dst = OPER_AL_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpi_8_pcdi(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cmpi_8_pcix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cmpi_16_d(void)
{
	uint src = OPER_I_16();
	uint dst = MASK_OUT_ABOVE_16(DY);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmpi_16_ai(void)
{
	uint src = OPER_I_16();
	uint dst = OPER_AY_AI_16();
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmpi_16_pi(void)
{
	uint src = OPER_I_16();
	uint dst = OPER_AY_PI_16();
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmpi_16_pd(void)
{
	uint src = OPER_I_16();
	uint dst = OPER_AY_PD_16();
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmpi_16_di(void)
{
	uint src = OPER_I_16();
	uint dst = OPER_AY_DI_16();
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmpi_16_ix(void)
{
	uint src = OPER_I_16();
	uint dst = OPER_AY_IX_16();
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmpi_16_aw(void)
{
	uint src = OPER_I_16();
	uint dst = OPER_AW_16();
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmpi_16_al(void)
{
	uint src = OPER_I_16();
	uint dst = OPER_AL_16();
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmpi_16_pcdi(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cmpi_16_pcix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cmpi_32_d(void)
{
	uint src = OPER_I_32();
	uint dst = DY;
	uint res = dst - src;

	m68ki_cmpild_callback(src, REG_IR & 7);		   /* auto-disable (see m68kcpu.h) */
	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpi_32_ai(void)
{
	uint src = OPER_I_32();
	uint dst = OPER_AY_AI_32();
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpi_32_pi(void)
{
	uint src = OPER_I_32();
	uint dst = OPER_AY_PI_32();
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpi_32_pd(void)
{
	uint src = OPER_I_32();
	uint dst = OPER_AY_PD_32();
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpi_32_di(void)
{
	uint src = OPER_I_32();
	uint dst = OPER_AY_DI_32();
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpi_32_ix(void)
{
	uint src = OPER_I_32();
	uint dst = OPER_AY_IX_32();
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpi_32_aw(void)
{
	uint src = OPER_I_32();
	uint dst = OPER_AW_32();
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpi_32_al(void)
{
	uint src = OPER_I_32();
	uint dst = OPER_AL_32();
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpi_32_pcdi(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cmpi_32_pcix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_cmpm_8_ax7(void)
{
	uint src = OPER_AY_PI_8();
	uint dst = OPER_A7_PI_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpm_8_ay7(void)
{
	uint src = OPER_A7_PI_8();
	uint dst = OPER_AX_PI_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpm_8_axy7(void)
{
	uint src = OPER_A7_PI_8();
	uint dst = OPER_A7_PI_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpm_8(void)
{
	uint src = OPER_AY_PI_8();
	uint dst = OPER_AX_PI_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpm_16(void)
{
	uint src = OPER_AY_PI_16();
	uint dst = OPER_AX_PI_16();
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmpm_32(void)
{
	uint src = OPER_AY_PI_32();
	uint dst = OPER_AX_PI_32();
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cpbcc_32(void)
{

	m68ki_exception_1111();
}


void m68k_op_cpdbcc_32(void)
{

	m68ki_exception_1111();
}


void m68k_op_cpgen_32(void)
{

	m68ki_exception_1111();
}


void m68k_op_cpscc_32(void)
{

	m68ki_exception_1111();
}


void m68k_op_cptrapcc_32(void)
{

	m68ki_exception_1111();
}


void m68k_op_dbt_16(void)
{
	REG_PC += 2;
}


void m68k_op_dbf_16(void)
{
	uint* r_dst = &DY;
	uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
	if(res != 0xffff)
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		USE_CYCLES(CYC_DBCC_F_NOEXP);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_DBCC_F_EXP);
}


void m68k_op_dbhi_16(void)
{
	if(COND_NOT_HI())
	{
		uint* r_dst = &DY;
		uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint offset = OPER_I_16();
			REG_PC -= 2;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			USE_CYCLES(CYC_DBCC_F_NOEXP);
			return;
		}
		REG_PC += 2;
		USE_CYCLES(CYC_DBCC_F_EXP);
		return;
	}
	REG_PC += 2;
}


void m68k_op_dbls_16(void)
{
	if(COND_NOT_LS())
	{
		uint* r_dst = &DY;
		uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint offset = OPER_I_16();
			REG_PC -= 2;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			USE_CYCLES(CYC_DBCC_F_NOEXP);
			return;
		}
		REG_PC += 2;
		USE_CYCLES(CYC_DBCC_F_EXP);
		return;
	}
	REG_PC += 2;
}


void m68k_op_dbcc_16(void)
{
	if(COND_NOT_CC())
	{
		uint* r_dst = &DY;
		uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint offset = OPER_I_16();
			REG_PC -= 2;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			USE_CYCLES(CYC_DBCC_F_NOEXP);
			return;
		}
		REG_PC += 2;
		USE_CYCLES(CYC_DBCC_F_EXP);
		return;
	}
	REG_PC += 2;
}


void m68k_op_dbcs_16(void)
{
	if(COND_NOT_CS())
	{
		uint* r_dst = &DY;
		uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint offset = OPER_I_16();
			REG_PC -= 2;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			USE_CYCLES(CYC_DBCC_F_NOEXP);
			return;
		}
		REG_PC += 2;
		USE_CYCLES(CYC_DBCC_F_EXP);
		return;
	}
	REG_PC += 2;
}


void m68k_op_dbne_16(void)
{
	if(COND_NOT_NE())
	{
		uint* r_dst = &DY;
		uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint offset = OPER_I_16();
			REG_PC -= 2;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			USE_CYCLES(CYC_DBCC_F_NOEXP);
			return;
		}
		REG_PC += 2;
		USE_CYCLES(CYC_DBCC_F_EXP);
		return;
	}
	REG_PC += 2;
}


void m68k_op_dbeq_16(void)
{
	if(COND_NOT_EQ())
	{
		uint* r_dst = &DY;
		uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint offset = OPER_I_16();
			REG_PC -= 2;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			USE_CYCLES(CYC_DBCC_F_NOEXP);
			return;
		}
		REG_PC += 2;
		USE_CYCLES(CYC_DBCC_F_EXP);
		return;
	}
	REG_PC += 2;
}


void m68k_op_dbvc_16(void)
{
	if(COND_NOT_VC())
	{
		uint* r_dst = &DY;
		uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint offset = OPER_I_16();
			REG_PC -= 2;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			USE_CYCLES(CYC_DBCC_F_NOEXP);
			return;
		}
		REG_PC += 2;
		USE_CYCLES(CYC_DBCC_F_EXP);
		return;
	}
	REG_PC += 2;
}


void m68k_op_dbvs_16(void)
{
	if(COND_NOT_VS())
	{
		uint* r_dst = &DY;
		uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint offset = OPER_I_16();
			REG_PC -= 2;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			USE_CYCLES(CYC_DBCC_F_NOEXP);
			return;
		}
		REG_PC += 2;
		USE_CYCLES(CYC_DBCC_F_EXP);
		return;
	}
	REG_PC += 2;
}


void m68k_op_dbpl_16(void)
{
	if(COND_NOT_PL())
	{
		uint* r_dst = &DY;
		uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint offset = OPER_I_16();
			REG_PC -= 2;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			USE_CYCLES(CYC_DBCC_F_NOEXP);
			return;
		}
		REG_PC += 2;
		USE_CYCLES(CYC_DBCC_F_EXP);
		return;
	}
	REG_PC += 2;
}


void m68k_op_dbmi_16(void)
{
	if(COND_NOT_MI())
	{
		uint* r_dst = &DY;
		uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint offset = OPER_I_16();
			REG_PC -= 2;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			USE_CYCLES(CYC_DBCC_F_NOEXP);
			return;
		}
		REG_PC += 2;
		USE_CYCLES(CYC_DBCC_F_EXP);
		return;
	}
	REG_PC += 2;
}


void m68k_op_dbge_16(void)
{
	if(COND_NOT_GE())
	{
		uint* r_dst = &DY;
		uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint offset = OPER_I_16();
			REG_PC -= 2;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			USE_CYCLES(CYC_DBCC_F_NOEXP);
			return;
		}
		REG_PC += 2;
		USE_CYCLES(CYC_DBCC_F_EXP);
		return;
	}
	REG_PC += 2;
}


void m68k_op_dblt_16(void)
{
	if(COND_NOT_LT())
	{
		uint* r_dst = &DY;
		uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint offset = OPER_I_16();
			REG_PC -= 2;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			USE_CYCLES(CYC_DBCC_F_NOEXP);
			return;
		}
		REG_PC += 2;
		USE_CYCLES(CYC_DBCC_F_EXP);
		return;
	}
	REG_PC += 2;
}


void m68k_op_dbgt_16(void)
{
	if(COND_NOT_GT())
	{
		uint* r_dst = &DY;
		uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint offset = OPER_I_16();
			REG_PC -= 2;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			USE_CYCLES(CYC_DBCC_F_NOEXP);
			return;
		}
		REG_PC += 2;
		USE_CYCLES(CYC_DBCC_F_EXP);
		return;
	}
	REG_PC += 2;
}


void m68k_op_dble_16(void)
{
	if(COND_NOT_LE())
	{
		uint* r_dst = &DY;
		uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint offset = OPER_I_16();
			REG_PC -= 2;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			USE_CYCLES(CYC_DBCC_F_NOEXP);
			return;
		}
		REG_PC += 2;
		USE_CYCLES(CYC_DBCC_F_EXP);
		return;
	}
	REG_PC += 2;
}


void m68k_op_divs_16_d(void)
{
	uint* r_dst = &DX;
	sint src = MAKE_INT_16(DY);
	sint quotient;
	sint remainder;

	if(src != 0)
	{
		if((uint32)*r_dst == 0x80000000 && src == -1)
		{
			FLAG_Z = 0;
			FLAG_N = NFLAG_CLEAR;
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = 0;
			return;
		}

		quotient = MAKE_INT_32(*r_dst) / src;
		remainder = MAKE_INT_32(*r_dst) % src;

		if(quotient == MAKE_INT_16(quotient))
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


void m68k_op_divs_16_ai(void)
{
	uint* r_dst = &DX;
	sint src = MAKE_INT_16(OPER_AY_AI_16());
	sint quotient;
	sint remainder;

	if(src != 0)
	{
		if((uint32)*r_dst == 0x80000000 && src == -1)
		{
			FLAG_Z = 0;
			FLAG_N = NFLAG_CLEAR;
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = 0;
			return;
		}

		quotient = MAKE_INT_32(*r_dst) / src;
		remainder = MAKE_INT_32(*r_dst) % src;

		if(quotient == MAKE_INT_16(quotient))
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


void m68k_op_divs_16_pi(void)
{
	uint* r_dst = &DX;
	sint src = MAKE_INT_16(OPER_AY_PI_16());
	sint quotient;
	sint remainder;

	if(src != 0)
	{
		if((uint32)*r_dst == 0x80000000 && src == -1)
		{
			FLAG_Z = 0;
			FLAG_N = NFLAG_CLEAR;
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = 0;
			return;
		}

		quotient = MAKE_INT_32(*r_dst) / src;
		remainder = MAKE_INT_32(*r_dst) % src;

		if(quotient == MAKE_INT_16(quotient))
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


void m68k_op_divs_16_pd(void)
{
	uint* r_dst = &DX;
	sint src = MAKE_INT_16(OPER_AY_PD_16());
	sint quotient;
	sint remainder;

	if(src != 0)
	{
		if((uint32)*r_dst == 0x80000000 && src == -1)
		{
			FLAG_Z = 0;
			FLAG_N = NFLAG_CLEAR;
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = 0;
			return;
		}

		quotient = MAKE_INT_32(*r_dst) / src;
		remainder = MAKE_INT_32(*r_dst) % src;

		if(quotient == MAKE_INT_16(quotient))
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


void m68k_op_divs_16_di(void)
{
	uint* r_dst = &DX;
	sint src = MAKE_INT_16(OPER_AY_DI_16());
	sint quotient;
	sint remainder;

	if(src != 0)
	{
		if((uint32)*r_dst == 0x80000000 && src == -1)
		{
			FLAG_Z = 0;
			FLAG_N = NFLAG_CLEAR;
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = 0;
			return;
		}

		quotient = MAKE_INT_32(*r_dst) / src;
		remainder = MAKE_INT_32(*r_dst) % src;

		if(quotient == MAKE_INT_16(quotient))
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


void m68k_op_divs_16_ix(void)
{
	uint* r_dst = &DX;
	sint src = MAKE_INT_16(OPER_AY_IX_16());
	sint quotient;
	sint remainder;

	if(src != 0)
	{
		if((uint32)*r_dst == 0x80000000 && src == -1)
		{
			FLAG_Z = 0;
			FLAG_N = NFLAG_CLEAR;
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = 0;
			return;
		}

		quotient = MAKE_INT_32(*r_dst) / src;
		remainder = MAKE_INT_32(*r_dst) % src;

		if(quotient == MAKE_INT_16(quotient))
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


void m68k_op_divs_16_aw(void)
{
	uint* r_dst = &DX;
	sint src = MAKE_INT_16(OPER_AW_16());
	sint quotient;
	sint remainder;

	if(src != 0)
	{
		if((uint32)*r_dst == 0x80000000 && src == -1)
		{
			FLAG_Z = 0;
			FLAG_N = NFLAG_CLEAR;
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = 0;
			return;
		}

		quotient = MAKE_INT_32(*r_dst) / src;
		remainder = MAKE_INT_32(*r_dst) % src;

		if(quotient == MAKE_INT_16(quotient))
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


void m68k_op_divs_16_al(void)
{
	uint* r_dst = &DX;
	sint src = MAKE_INT_16(OPER_AL_16());
	sint quotient;
	sint remainder;

	if(src != 0)
	{
		if((uint32)*r_dst == 0x80000000 && src == -1)
		{
			FLAG_Z = 0;
			FLAG_N = NFLAG_CLEAR;
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = 0;
			return;
		}

		quotient = MAKE_INT_32(*r_dst) / src;
		remainder = MAKE_INT_32(*r_dst) % src;

		if(quotient == MAKE_INT_16(quotient))
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


void m68k_op_divs_16_pcdi(void)
{
	uint* r_dst = &DX;
	sint src = MAKE_INT_16(OPER_PCDI_16());
	sint quotient;
	sint remainder;

	if(src != 0)
	{
		if((uint32)*r_dst == 0x80000000 && src == -1)
		{
			FLAG_Z = 0;
			FLAG_N = NFLAG_CLEAR;
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = 0;
			return;
		}

		quotient = MAKE_INT_32(*r_dst) / src;
		remainder = MAKE_INT_32(*r_dst) % src;

		if(quotient == MAKE_INT_16(quotient))
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


void m68k_op_divs_16_pcix(void)
{
	uint* r_dst = &DX;
	sint src = MAKE_INT_16(OPER_PCIX_16());
	sint quotient;
	sint remainder;

	if(src != 0)
	{
		if((uint32)*r_dst == 0x80000000 && src == -1)
		{
			FLAG_Z = 0;
			FLAG_N = NFLAG_CLEAR;
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = 0;
			return;
		}

		quotient = MAKE_INT_32(*r_dst) / src;
		remainder = MAKE_INT_32(*r_dst) % src;

		if(quotient == MAKE_INT_16(quotient))
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


void m68k_op_divs_16_i(void)
{
	uint* r_dst = &DX;
	sint src = MAKE_INT_16(OPER_I_16());
	sint quotient;
	sint remainder;

	if(src != 0)
	{
		if((uint32)*r_dst == 0x80000000 && src == -1)
		{
			FLAG_Z = 0;
			FLAG_N = NFLAG_CLEAR;
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = 0;
			return;
		}

		quotient = MAKE_INT_32(*r_dst) / src;
		remainder = MAKE_INT_32(*r_dst) % src;

		if(quotient == MAKE_INT_16(quotient))
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


void m68k_op_divu_16_d(void)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_16(DY);

	if(src != 0)
	{
		uint quotient = *r_dst / src;
		uint remainder = *r_dst % src;

		if(quotient < 0x10000)
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


void m68k_op_divu_16_ai(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_AI_16();

	if(src != 0)
	{
		uint quotient = *r_dst / src;
		uint remainder = *r_dst % src;

		if(quotient < 0x10000)
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


void m68k_op_divu_16_pi(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_PI_16();

	if(src != 0)
	{
		uint quotient = *r_dst / src;
		uint remainder = *r_dst % src;

		if(quotient < 0x10000)
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


void m68k_op_divu_16_pd(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_PD_16();

	if(src != 0)
	{
		uint quotient = *r_dst / src;
		uint remainder = *r_dst % src;

		if(quotient < 0x10000)
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


void m68k_op_divu_16_di(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_DI_16();

	if(src != 0)
	{
		uint quotient = *r_dst / src;
		uint remainder = *r_dst % src;

		if(quotient < 0x10000)
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


void m68k_op_divu_16_ix(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_IX_16();

	if(src != 0)
	{
		uint quotient = *r_dst / src;
		uint remainder = *r_dst % src;

		if(quotient < 0x10000)
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


void m68k_op_divu_16_aw(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AW_16();

	if(src != 0)
	{
		uint quotient = *r_dst / src;
		uint remainder = *r_dst % src;

		if(quotient < 0x10000)
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


void m68k_op_divu_16_al(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AL_16();

	if(src != 0)
	{
		uint quotient = *r_dst / src;
		uint remainder = *r_dst % src;

		if(quotient < 0x10000)
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


void m68k_op_divu_16_pcdi(void)
{
	uint* r_dst = &DX;
	uint src = OPER_PCDI_16();

	if(src != 0)
	{
		uint quotient = *r_dst / src;
		uint remainder = *r_dst % src;

		if(quotient < 0x10000)
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


void m68k_op_divu_16_pcix(void)
{
	uint* r_dst = &DX;
	uint src = OPER_PCIX_16();

	if(src != 0)
	{
		uint quotient = *r_dst / src;
		uint remainder = *r_dst % src;

		if(quotient < 0x10000)
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


void m68k_op_divu_16_i(void)
{
	uint* r_dst = &DX;
	uint src = OPER_I_16();

	if(src != 0)
	{
		uint quotient = *r_dst / src;
		uint remainder = *r_dst % src;

		if(quotient < 0x10000)
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


void m68k_op_divl_32_d(void)
{
#if M68K_USE_64_BIT

	if(0)
	{
		uint word2 = OPER_I_16();
		uint64 divisor   = DY;
		uint64 dividend  = 0;
		uint64 quotient  = 0;
		uint64 remainder = 0;

		if(divisor != 0)
		{
			if(BIT_A(word2))	/* 64 bit */
			{
				dividend = REG_D[word2 & 7];
				dividend <<= 32;
				dividend |= REG_D[(word2 >> 12) & 7];

				if(BIT_B(word2))	   /* signed */
				{
					quotient  = (uint64)((sint64)dividend / (sint64)((sint32)divisor));
					remainder = (uint64)((sint64)dividend % (sint64)((sint32)divisor));
					if((sint64)quotient != (sint64)((sint32)quotient))
					{
						FLAG_V = VFLAG_SET;
						return;
					}
				}
				else					/* unsigned */
				{
					quotient = dividend / divisor;
					if(quotient > 0xffffffff)
					{
						FLAG_V = VFLAG_SET;
						return;
					}
					remainder = dividend % divisor;
				}
			}
			else	/* 32 bit */
			{
				dividend = REG_D[(word2 >> 12) & 7];
				if(BIT_B(word2))	   /* signed */
				{
					quotient  = (uint64)((sint64)((sint32)dividend) / (sint64)((sint32)divisor));
					remainder = (uint64)((sint64)((sint32)dividend) % (sint64)((sint32)divisor));
				}
				else					/* unsigned */
				{
					quotient = dividend / divisor;
					remainder = dividend % divisor;
				}
			}

			REG_D[word2 & 7] = remainder;
			REG_D[(word2 >> 12) & 7] = quotient;

			FLAG_N = NFLAG_32(quotient);
			FLAG_Z = quotient;
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			return;
		}
		m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
		return;
	}
	m68ki_exception_illegal();

#else


	m68ki_exception_illegal();

#endif
}


void m68k_op_divl_32_ai(void)
{
#if M68K_USE_64_BIT

	if(0)
	{
		uint word2 = OPER_I_16();
		uint64 divisor = OPER_AY_AI_32();
		uint64 dividend  = 0;
		uint64 quotient  = 0;
		uint64 remainder = 0;

		if(divisor != 0)
		{
			if(BIT_A(word2))	/* 64 bit */
			{
				dividend = REG_D[word2 & 7];
				dividend <<= 32;
				dividend |= REG_D[(word2 >> 12) & 7];

				if(BIT_B(word2))	   /* signed */
				{
					quotient  = (uint64)((sint64)dividend / (sint64)((sint32)divisor));
					remainder = (uint64)((sint64)dividend % (sint64)((sint32)divisor));
					if((sint64)quotient != (sint64)((sint32)quotient))
					{
						FLAG_V = VFLAG_SET;
						return;
					}
				}
				else					/* unsigned */
				{
					quotient = dividend / divisor;
					if(quotient > 0xffffffff)
					{
						FLAG_V = VFLAG_SET;
						return;
					}
					remainder = dividend % divisor;
				}
			}
			else	/* 32 bit */
			{
				dividend = REG_D[(word2 >> 12) & 7];
				if(BIT_B(word2))	   /* signed */
				{
					quotient  = (uint64)((sint64)((sint32)dividend) / (sint64)((sint32)divisor));
					remainder = (uint64)((sint64)((sint32)dividend) % (sint64)((sint32)divisor));
				}
				else					/* unsigned */
				{
					quotient = dividend / divisor;
					remainder = dividend % divisor;
				}
			}

			REG_D[word2 & 7] = remainder;
			REG_D[(word2 >> 12) & 7] = quotient;

			FLAG_N = NFLAG_32(quotient);
			FLAG_Z = quotient;
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			return;
		}
		m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
		return;
	}
	m68ki_exception_illegal();

#else


	m68ki_exception_illegal();

#endif
}


void m68k_op_divl_32_pi(void)
{
#if M68K_USE_64_BIT

	if(0)
	{
		uint word2 = OPER_I_16();
		uint64 divisor = OPER_AY_PI_32();
		uint64 dividend  = 0;
		uint64 quotient  = 0;
		uint64 remainder = 0;

		if(divisor != 0)
		{
			if(BIT_A(word2))	/* 64 bit */
			{
				dividend = REG_D[word2 & 7];
				dividend <<= 32;
				dividend |= REG_D[(word2 >> 12) & 7];

				if(BIT_B(word2))	   /* signed */
				{
					quotient  = (uint64)((sint64)dividend / (sint64)((sint32)divisor));
					remainder = (uint64)((sint64)dividend % (sint64)((sint32)divisor));
					if((sint64)quotient != (sint64)((sint32)quotient))
					{
						FLAG_V = VFLAG_SET;
						return;
					}
				}
				else					/* unsigned */
				{
					quotient = dividend / divisor;
					if(quotient > 0xffffffff)
					{
						FLAG_V = VFLAG_SET;
						return;
					}
					remainder = dividend % divisor;
				}
			}
			else	/* 32 bit */
			{
				dividend = REG_D[(word2 >> 12) & 7];
				if(BIT_B(word2))	   /* signed */
				{
					quotient  = (uint64)((sint64)((sint32)dividend) / (sint64)((sint32)divisor));
					remainder = (uint64)((sint64)((sint32)dividend) % (sint64)((sint32)divisor));
				}
				else					/* unsigned */
				{
					quotient = dividend / divisor;
					remainder = dividend % divisor;
				}
			}

			REG_D[word2 & 7] = remainder;
			REG_D[(word2 >> 12) & 7] = quotient;

			FLAG_N = NFLAG_32(quotient);
			FLAG_Z = quotient;
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			return;
		}
		m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
		return;
	}
	m68ki_exception_illegal();

#else


	m68ki_exception_illegal();

#endif
}


void m68k_op_divl_32_pd(void)
{
#if M68K_USE_64_BIT

	if(0)
	{
		uint word2 = OPER_I_16();
		uint64 divisor = OPER_AY_PD_32();
		uint64 dividend  = 0;
		uint64 quotient  = 0;
		uint64 remainder = 0;

		if(divisor != 0)
		{
			if(BIT_A(word2))	/* 64 bit */
			{
				dividend = REG_D[word2 & 7];
				dividend <<= 32;
				dividend |= REG_D[(word2 >> 12) & 7];

				if(BIT_B(word2))	   /* signed */
				{
					quotient  = (uint64)((sint64)dividend / (sint64)((sint32)divisor));
					remainder = (uint64)((sint64)dividend % (sint64)((sint32)divisor));
					if((sint64)quotient != (sint64)((sint32)quotient))
					{
						FLAG_V = VFLAG_SET;
						return;
					}
				}
				else					/* unsigned */
				{
					quotient = dividend / divisor;
					if(quotient > 0xffffffff)
					{
						FLAG_V = VFLAG_SET;
						return;
					}
					remainder = dividend % divisor;
				}
			}
			else	/* 32 bit */
			{
				dividend = REG_D[(word2 >> 12) & 7];
				if(BIT_B(word2))	   /* signed */
				{
					quotient  = (uint64)((sint64)((sint32)dividend) / (sint64)((sint32)divisor));
					remainder = (uint64)((sint64)((sint32)dividend) % (sint64)((sint32)divisor));
				}
				else					/* unsigned */
				{
					quotient = dividend / divisor;
					remainder = dividend % divisor;
				}
			}

			REG_D[word2 & 7] = remainder;
			REG_D[(word2 >> 12) & 7] = quotient;

			FLAG_N = NFLAG_32(quotient);
			FLAG_Z = quotient;
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			return;
		}
		m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
		return;
	}
	m68ki_exception_illegal();

#else


	m68ki_exception_illegal();

#endif
}


void m68k_op_divl_32_di(void)
{
#if M68K_USE_64_BIT

	if(0)
	{
		uint word2 = OPER_I_16();
		uint64 divisor = OPER_AY_DI_32();
		uint64 dividend  = 0;
		uint64 quotient  = 0;
		uint64 remainder = 0;

		if(divisor != 0)
		{
			if(BIT_A(word2))	/* 64 bit */
			{
				dividend = REG_D[word2 & 7];
				dividend <<= 32;
				dividend |= REG_D[(word2 >> 12) & 7];

				if(BIT_B(word2))	   /* signed */
				{
					quotient  = (uint64)((sint64)dividend / (sint64)((sint32)divisor));
					remainder = (uint64)((sint64)dividend % (sint64)((sint32)divisor));
					if((sint64)quotient != (sint64)((sint32)quotient))
					{
						FLAG_V = VFLAG_SET;
						return;
					}
				}
				else					/* unsigned */
				{
					quotient = dividend / divisor;
					if(quotient > 0xffffffff)
					{
						FLAG_V = VFLAG_SET;
						return;
					}
					remainder = dividend % divisor;
				}
			}
			else	/* 32 bit */
			{
				dividend = REG_D[(word2 >> 12) & 7];
				if(BIT_B(word2))	   /* signed */
				{
					quotient  = (uint64)((sint64)((sint32)dividend) / (sint64)((sint32)divisor));
					remainder = (uint64)((sint64)((sint32)dividend) % (sint64)((sint32)divisor));
				}
				else					/* unsigned */
				{
					quotient = dividend / divisor;
					remainder = dividend % divisor;
				}
			}

			REG_D[word2 & 7] = remainder;
			REG_D[(word2 >> 12) & 7] = quotient;

			FLAG_N = NFLAG_32(quotient);
			FLAG_Z = quotient;
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			return;
		}
		m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
		return;
	}
	m68ki_exception_illegal();

#else


	m68ki_exception_illegal();

#endif
}


void m68k_op_divl_32_ix(void)
{
#if M68K_USE_64_BIT

	if(0)
	{
		uint word2 = OPER_I_16();
		uint64 divisor = OPER_AY_IX_32();
		uint64 dividend  = 0;
		uint64 quotient  = 0;
		uint64 remainder = 0;

		if(divisor != 0)
		{
			if(BIT_A(word2))	/* 64 bit */
			{
				dividend = REG_D[word2 & 7];
				dividend <<= 32;
				dividend |= REG_D[(word2 >> 12) & 7];

				if(BIT_B(word2))	   /* signed */
				{
					quotient  = (uint64)((sint64)dividend / (sint64)((sint32)divisor));
					remainder = (uint64)((sint64)dividend % (sint64)((sint32)divisor));
					if((sint64)quotient != (sint64)((sint32)quotient))
					{
						FLAG_V = VFLAG_SET;
						return;
					}
				}
				else					/* unsigned */
				{
					quotient = dividend / divisor;
					if(quotient > 0xffffffff)
					{
						FLAG_V = VFLAG_SET;
						return;
					}
					remainder = dividend % divisor;
				}
			}
			else	/* 32 bit */
			{
				dividend = REG_D[(word2 >> 12) & 7];
				if(BIT_B(word2))	   /* signed */
				{
					quotient  = (uint64)((sint64)((sint32)dividend) / (sint64)((sint32)divisor));
					remainder = (uint64)((sint64)((sint32)dividend) % (sint64)((sint32)divisor));
				}
				else					/* unsigned */
				{
					quotient = dividend / divisor;
					remainder = dividend % divisor;
				}
			}

			REG_D[word2 & 7] = remainder;
			REG_D[(word2 >> 12) & 7] = quotient;

			FLAG_N = NFLAG_32(quotient);
			FLAG_Z = quotient;
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			return;
		}
		m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
		return;
	}
	m68ki_exception_illegal();

#else


	m68ki_exception_illegal();

#endif
}


void m68k_op_divl_32_aw(void)
{
#if M68K_USE_64_BIT

	if(0)
	{
		uint word2 = OPER_I_16();
		uint64 divisor = OPER_AW_32();
		uint64 dividend  = 0;
		uint64 quotient  = 0;
		uint64 remainder = 0;

		if(divisor != 0)
		{
			if(BIT_A(word2))	/* 64 bit */
			{
				dividend = REG_D[word2 & 7];
				dividend <<= 32;
				dividend |= REG_D[(word2 >> 12) & 7];

				if(BIT_B(word2))	   /* signed */
				{
					quotient  = (uint64)((sint64)dividend / (sint64)((sint32)divisor));
					remainder = (uint64)((sint64)dividend % (sint64)((sint32)divisor));
					if((sint64)quotient != (sint64)((sint32)quotient))
					{
						FLAG_V = VFLAG_SET;
						return;
					}
				}
				else					/* unsigned */
				{
					quotient = dividend / divisor;
					if(quotient > 0xffffffff)
					{
						FLAG_V = VFLAG_SET;
						return;
					}
					remainder = dividend % divisor;
				}
			}
			else	/* 32 bit */
			{
				dividend = REG_D[(word2 >> 12) & 7];
				if(BIT_B(word2))	   /* signed */
				{
					quotient  = (uint64)((sint64)((sint32)dividend) / (sint64)((sint32)divisor));
					remainder = (uint64)((sint64)((sint32)dividend) % (sint64)((sint32)divisor));
				}
				else					/* unsigned */
				{
					quotient = dividend / divisor;
					remainder = dividend % divisor;
				}
			}

			REG_D[word2 & 7] = remainder;
			REG_D[(word2 >> 12) & 7] = quotient;

			FLAG_N = NFLAG_32(quotient);
			FLAG_Z = quotient;
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			return;
		}
		m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
		return;
	}
	m68ki_exception_illegal();

#else


	m68ki_exception_illegal();

#endif
}


void m68k_op_divl_32_al(void)
{
#if M68K_USE_64_BIT

	if(0)
	{
		uint word2 = OPER_I_16();
		uint64 divisor = OPER_AL_32();
		uint64 dividend  = 0;
		uint64 quotient  = 0;
		uint64 remainder = 0;

		if(divisor != 0)
		{
			if(BIT_A(word2))	/* 64 bit */
			{
				dividend = REG_D[word2 & 7];
				dividend <<= 32;
				dividend |= REG_D[(word2 >> 12) & 7];

				if(BIT_B(word2))	   /* signed */
				{
					quotient  = (uint64)((sint64)dividend / (sint64)((sint32)divisor));
					remainder = (uint64)((sint64)dividend % (sint64)((sint32)divisor));
					if((sint64)quotient != (sint64)((sint32)quotient))
					{
						FLAG_V = VFLAG_SET;
						return;
					}
				}
				else					/* unsigned */
				{
					quotient = dividend / divisor;
					if(quotient > 0xffffffff)
					{
						FLAG_V = VFLAG_SET;
						return;
					}
					remainder = dividend % divisor;
				}
			}
			else	/* 32 bit */
			{
				dividend = REG_D[(word2 >> 12) & 7];
				if(BIT_B(word2))	   /* signed */
				{
					quotient  = (uint64)((sint64)((sint32)dividend) / (sint64)((sint32)divisor));
					remainder = (uint64)((sint64)((sint32)dividend) % (sint64)((sint32)divisor));
				}
				else					/* unsigned */
				{
					quotient = dividend / divisor;
					remainder = dividend % divisor;
				}
			}

			REG_D[word2 & 7] = remainder;
			REG_D[(word2 >> 12) & 7] = quotient;

			FLAG_N = NFLAG_32(quotient);
			FLAG_Z = quotient;
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			return;
		}
		m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
		return;
	}
	m68ki_exception_illegal();

#else


	m68ki_exception_illegal();

#endif
}


void m68k_op_divl_32_pcdi(void)
{
#if M68K_USE_64_BIT

	if(0)
	{
		uint word2 = OPER_I_16();
		uint64 divisor = OPER_PCDI_32();
		uint64 dividend  = 0;
		uint64 quotient  = 0;
		uint64 remainder = 0;

		if(divisor != 0)
		{
			if(BIT_A(word2))	/* 64 bit */
			{
				dividend = REG_D[word2 & 7];
				dividend <<= 32;
				dividend |= REG_D[(word2 >> 12) & 7];

				if(BIT_B(word2))	   /* signed */
				{
					quotient  = (uint64)((sint64)dividend / (sint64)((sint32)divisor));
					remainder = (uint64)((sint64)dividend % (sint64)((sint32)divisor));
					if((sint64)quotient != (sint64)((sint32)quotient))
					{
						FLAG_V = VFLAG_SET;
						return;
					}
				}
				else					/* unsigned */
				{
					quotient = dividend / divisor;
					if(quotient > 0xffffffff)
					{
						FLAG_V = VFLAG_SET;
						return;
					}
					remainder = dividend % divisor;
				}
			}
			else	/* 32 bit */
			{
				dividend = REG_D[(word2 >> 12) & 7];
				if(BIT_B(word2))	   /* signed */
				{
					quotient  = (uint64)((sint64)((sint32)dividend) / (sint64)((sint32)divisor));
					remainder = (uint64)((sint64)((sint32)dividend) % (sint64)((sint32)divisor));
				}
				else					/* unsigned */
				{
					quotient = dividend / divisor;
					remainder = dividend % divisor;
				}
			}

			REG_D[word2 & 7] = remainder;
			REG_D[(word2 >> 12) & 7] = quotient;

			FLAG_N = NFLAG_32(quotient);
			FLAG_Z = quotient;
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			return;
		}
		m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
		return;
	}
	m68ki_exception_illegal();

#else


	m68ki_exception_illegal();

#endif
}


void m68k_op_divl_32_pcix(void)
{
#if M68K_USE_64_BIT

	if(0)
	{
		uint word2 = OPER_I_16();
		uint64 divisor = OPER_PCIX_32();
		uint64 dividend  = 0;
		uint64 quotient  = 0;
		uint64 remainder = 0;

		if(divisor != 0)
		{
			if(BIT_A(word2))	/* 64 bit */
			{
				dividend = REG_D[word2 & 7];
				dividend <<= 32;
				dividend |= REG_D[(word2 >> 12) & 7];

				if(BIT_B(word2))	   /* signed */
				{
					quotient  = (uint64)((sint64)dividend / (sint64)((sint32)divisor));
					remainder = (uint64)((sint64)dividend % (sint64)((sint32)divisor));
					if((sint64)quotient != (sint64)((sint32)quotient))
					{
						FLAG_V = VFLAG_SET;
						return;
					}
				}
				else					/* unsigned */
				{
					quotient = dividend / divisor;
					if(quotient > 0xffffffff)
					{
						FLAG_V = VFLAG_SET;
						return;
					}
					remainder = dividend % divisor;
				}
			}
			else	/* 32 bit */
			{
				dividend = REG_D[(word2 >> 12) & 7];
				if(BIT_B(word2))	   /* signed */
				{
					quotient  = (uint64)((sint64)((sint32)dividend) / (sint64)((sint32)divisor));
					remainder = (uint64)((sint64)((sint32)dividend) % (sint64)((sint32)divisor));
				}
				else					/* unsigned */
				{
					quotient = dividend / divisor;
					remainder = dividend % divisor;
				}
			}

			REG_D[word2 & 7] = remainder;
			REG_D[(word2 >> 12) & 7] = quotient;

			FLAG_N = NFLAG_32(quotient);
			FLAG_Z = quotient;
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			return;
		}
		m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
		return;
	}
	m68ki_exception_illegal();

#else


	m68ki_exception_illegal();

#endif
}


void m68k_op_divl_32_i(void)
{
#if M68K_USE_64_BIT

	if(0)
	{
		uint word2 = OPER_I_16();
		uint64 divisor = OPER_I_32();
		uint64 dividend  = 0;
		uint64 quotient  = 0;
		uint64 remainder = 0;

		if(divisor != 0)
		{
			if(BIT_A(word2))	/* 64 bit */
			{
				dividend = REG_D[word2 & 7];
				dividend <<= 32;
				dividend |= REG_D[(word2 >> 12) & 7];

				if(BIT_B(word2))	   /* signed */
				{
					quotient  = (uint64)((sint64)dividend / (sint64)((sint32)divisor));
					remainder = (uint64)((sint64)dividend % (sint64)((sint32)divisor));
					if((sint64)quotient != (sint64)((sint32)quotient))
					{
						FLAG_V = VFLAG_SET;
						return;
					}
				}
				else					/* unsigned */
				{
					quotient = dividend / divisor;
					if(quotient > 0xffffffff)
					{
						FLAG_V = VFLAG_SET;
						return;
					}
					remainder = dividend % divisor;
				}
			}
			else	/* 32 bit */
			{
				dividend = REG_D[(word2 >> 12) & 7];
				if(BIT_B(word2))	   /* signed */
				{
					quotient  = (uint64)((sint64)((sint32)dividend) / (sint64)((sint32)divisor));
					remainder = (uint64)((sint64)((sint32)dividend) % (sint64)((sint32)divisor));
				}
				else					/* unsigned */
				{
					quotient = dividend / divisor;
					remainder = dividend % divisor;
				}
			}

			REG_D[word2 & 7] = remainder;
			REG_D[(word2 >> 12) & 7] = quotient;

			FLAG_N = NFLAG_32(quotient);
			FLAG_Z = quotient;
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			return;
		}
		m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
		return;
	}
	m68ki_exception_illegal();

#else


	m68ki_exception_illegal();

#endif
}


void m68k_op_eor_8_d(void)
{
	uint res = MASK_OUT_ABOVE_8(DY ^= MASK_OUT_ABOVE_8(DX));

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_8_ai(void)
{
	uint ea = EA_AY_AI_8();
	uint res = MASK_OUT_ABOVE_8(DX ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_8_pi(void)
{
	uint ea = EA_AY_PI_8();
	uint res = MASK_OUT_ABOVE_8(DX ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_8_pi7(void)
{
	uint ea = EA_A7_PI_8();
	uint res = MASK_OUT_ABOVE_8(DX ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_8_pd(void)
{
	uint ea = EA_AY_PD_8();
	uint res = MASK_OUT_ABOVE_8(DX ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_8_pd7(void)
{
	uint ea = EA_A7_PD_8();
	uint res = MASK_OUT_ABOVE_8(DX ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_8_di(void)
{
	uint ea = EA_AY_DI_8();
	uint res = MASK_OUT_ABOVE_8(DX ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_8_ix(void)
{
	uint ea = EA_AY_IX_8();
	uint res = MASK_OUT_ABOVE_8(DX ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_8_aw(void)
{
	uint ea = EA_AW_8();
	uint res = MASK_OUT_ABOVE_8(DX ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_8_al(void)
{
	uint ea = EA_AL_8();
	uint res = MASK_OUT_ABOVE_8(DX ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_16_d(void)
{
	uint res = MASK_OUT_ABOVE_16(DY ^= MASK_OUT_ABOVE_16(DX));

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_16_ai(void)
{
	uint ea = EA_AY_AI_16();
	uint res = MASK_OUT_ABOVE_16(DX ^ m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_16_pi(void)
{
	uint ea = EA_AY_PI_16();
	uint res = MASK_OUT_ABOVE_16(DX ^ m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_16_pd(void)
{
	uint ea = EA_AY_PD_16();
	uint res = MASK_OUT_ABOVE_16(DX ^ m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_16_di(void)
{
	uint ea = EA_AY_DI_16();
	uint res = MASK_OUT_ABOVE_16(DX ^ m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_16_ix(void)
{
	uint ea = EA_AY_IX_16();
	uint res = MASK_OUT_ABOVE_16(DX ^ m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_16_aw(void)
{
	uint ea = EA_AW_16();
	uint res = MASK_OUT_ABOVE_16(DX ^ m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_16_al(void)
{
	uint ea = EA_AL_16();
	uint res = MASK_OUT_ABOVE_16(DX ^ m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_32_d(void)
{
	uint res = DY ^= DX;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_32_ai(void)
{
	uint ea = EA_AY_AI_32();
	uint res = DX ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_32_pi(void)
{
	uint ea = EA_AY_PI_32();
	uint res = DX ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_32_pd(void)
{
	uint ea = EA_AY_PD_32();
	uint res = DX ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_32_di(void)
{
	uint ea = EA_AY_DI_32();
	uint res = DX ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_32_ix(void)
{
	uint ea = EA_AY_IX_32();
	uint res = DX ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_32_aw(void)
{
	uint ea = EA_AW_32();
	uint res = DX ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eor_32_al(void)
{
	uint ea = EA_AL_32();
	uint res = DX ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_8_d(void)
{
	uint res = MASK_OUT_ABOVE_8(DY ^= OPER_I_8());

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_8_ai(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_AI_8();
	uint res = src ^ m68ki_read_8(ea);

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_8_pi(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_PI_8();
	uint res = src ^ m68ki_read_8(ea);

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_8_pi7(void)
{
	uint src = OPER_I_8();
	uint ea = EA_A7_PI_8();
	uint res = src ^ m68ki_read_8(ea);

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_8_pd(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_PD_8();
	uint res = src ^ m68ki_read_8(ea);

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_8_pd7(void)
{
	uint src = OPER_I_8();
	uint ea = EA_A7_PD_8();
	uint res = src ^ m68ki_read_8(ea);

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_8_di(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_DI_8();
	uint res = src ^ m68ki_read_8(ea);

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_8_ix(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_IX_8();
	uint res = src ^ m68ki_read_8(ea);

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_8_aw(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AW_8();
	uint res = src ^ m68ki_read_8(ea);

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_8_al(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AL_8();
	uint res = src ^ m68ki_read_8(ea);

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_16_d(void)
{
	uint res = MASK_OUT_ABOVE_16(DY ^= OPER_I_16());

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_16_ai(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_AI_16();
	uint res = src ^ m68ki_read_16(ea);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_16_pi(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_PI_16();
	uint res = src ^ m68ki_read_16(ea);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_16_pd(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_PD_16();
	uint res = src ^ m68ki_read_16(ea);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_16_di(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_DI_16();
	uint res = src ^ m68ki_read_16(ea);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_16_ix(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_IX_16();
	uint res = src ^ m68ki_read_16(ea);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_16_aw(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AW_16();
	uint res = src ^ m68ki_read_16(ea);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_16_al(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AL_16();
	uint res = src ^ m68ki_read_16(ea);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_32_d(void)
{
	uint res = DY ^= OPER_I_32();

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_32_ai(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_AI_32();
	uint res = src ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_32_pi(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_PI_32();
	uint res = src ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_32_pd(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_PD_32();
	uint res = src ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_32_di(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_DI_32();
	uint res = src ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_32_ix(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_IX_32();
	uint res = src ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_32_aw(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AW_32();
	uint res = src ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_32_al(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AL_32();
	uint res = src ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_eori_16_toc(void)
{
	m68ki_set_ccr(m68ki_get_ccr() ^ OPER_I_8());
}


void m68k_op_eori_16_tos(void)
{
	if(FLAG_S)
	{
		uint src = OPER_I_16();
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(m68ki_get_sr() ^ src);
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_exg_32_dd(void)
{
	uint* reg_a = &DX;
	uint* reg_b = &DY;
	uint tmp = *reg_a;
	*reg_a = *reg_b;
	*reg_b = tmp;
}


void m68k_op_exg_32_aa(void)
{
	uint* reg_a = &AX;
	uint* reg_b = &AY;
	uint tmp = *reg_a;
	*reg_a = *reg_b;
	*reg_b = tmp;
}


void m68k_op_exg_32_da(void)
{
	uint* reg_a = &DX;
	uint* reg_b = &AY;
	uint tmp = *reg_a;
	*reg_a = *reg_b;
	*reg_b = tmp;
}


void m68k_op_ext_16(void)
{
	uint* r_dst = &DY;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | MASK_OUT_ABOVE_8(*r_dst) | (GET_MSB_8(*r_dst) ? 0xff00 : 0);

	FLAG_N = NFLAG_16(*r_dst);
	FLAG_Z = MASK_OUT_ABOVE_16(*r_dst);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_ext_32(void)
{
	uint* r_dst = &DY;

	*r_dst = MASK_OUT_ABOVE_16(*r_dst) | (GET_MSB_16(*r_dst) ? 0xffff0000 : 0);

	FLAG_N = NFLAG_32(*r_dst);
	FLAG_Z = *r_dst;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_extb_32(void)
{

	m68ki_exception_illegal();
}


void m68k_op_illegal(void)
{
	m68ki_exception_illegal();
}


void m68k_op_jmp_32_ai(void)
{
	m68ki_jump(EA_AY_AI_32());
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	if(REG_PC == REG_PPC)
		USE_ALL_CYCLES();
}


void m68k_op_jmp_32_di(void)
{
	m68ki_jump(EA_AY_DI_32());
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	if(REG_PC == REG_PPC)
		USE_ALL_CYCLES();
}


void m68k_op_jmp_32_ix(void)
{
	m68ki_jump(EA_AY_IX_32());
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	if(REG_PC == REG_PPC)
		USE_ALL_CYCLES();
}


void m68k_op_jmp_32_aw(void)
{
	m68ki_jump(EA_AW_32());
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	if(REG_PC == REG_PPC)
		USE_ALL_CYCLES();
}


void m68k_op_jmp_32_al(void)
{
	m68ki_jump(EA_AL_32());
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	if(REG_PC == REG_PPC)
		USE_ALL_CYCLES();
}


void m68k_op_jmp_32_pcdi(void)
{
	m68ki_jump(EA_PCDI_32());
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	if(REG_PC == REG_PPC)
		USE_ALL_CYCLES();
}


void m68k_op_jmp_32_pcix(void)
{
	m68ki_jump(EA_PCIX_32());
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	if(REG_PC == REG_PPC)
		USE_ALL_CYCLES();
}


void m68k_op_jsr_32_ai(void)
{
	uint ea = EA_AY_AI_32();
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(REG_PC);
	m68ki_jump(ea);
}


void m68k_op_jsr_32_di(void)
{
	uint ea = EA_AY_DI_32();
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(REG_PC);
	m68ki_jump(ea);
}


void m68k_op_jsr_32_ix(void)
{
	uint ea = EA_AY_IX_32();
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(REG_PC);
	m68ki_jump(ea);
}


void m68k_op_jsr_32_aw(void)
{
	uint ea = EA_AW_32();
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(REG_PC);
	m68ki_jump(ea);
}


void m68k_op_jsr_32_al(void)
{
	uint ea = EA_AL_32();
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(REG_PC);
	m68ki_jump(ea);
}


void m68k_op_jsr_32_pcdi(void)
{
	uint ea = EA_PCDI_32();
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(REG_PC);
	m68ki_jump(ea);
}


void m68k_op_jsr_32_pcix(void)
{
	uint ea = EA_PCIX_32();
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(REG_PC);
	m68ki_jump(ea);
}


void m68k_op_lea_32_ai(void)
{
	AX = EA_AY_AI_32();
}


void m68k_op_lea_32_di(void)
{
	AX = EA_AY_DI_32();
}


void m68k_op_lea_32_ix(void)
{
	AX = EA_AY_IX_32();
}


void m68k_op_lea_32_aw(void)
{
	AX = EA_AW_32();
}


void m68k_op_lea_32_al(void)
{
	AX = EA_AL_32();
}


void m68k_op_lea_32_pcdi(void)
{
	AX = EA_PCDI_32();
}


void m68k_op_lea_32_pcix(void)
{
	AX = EA_PCIX_32();
}


void m68k_op_link_16_a7(void)
{
	REG_A[7] -= 4;
	m68ki_write_32(REG_A[7], REG_A[7]);
	REG_A[7] = MASK_OUT_ABOVE_32(REG_A[7] + MAKE_INT_16(OPER_I_16()));
}


void m68k_op_link_16(void)
{
	uint* r_dst = &AY;

	m68ki_push_32(*r_dst);
	*r_dst = REG_A[7];
	REG_A[7] = MASK_OUT_ABOVE_32(REG_A[7] + MAKE_INT_16(OPER_I_16()));
}


void m68k_op_link_32_a7(void)
{

	m68ki_exception_illegal();
}


void m68k_op_link_32(void)
{

	m68ki_exception_illegal();
}


void m68k_op_lsr_8_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src >> shift;

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_CLEAR;
	FLAG_Z = res;
	FLAG_X = FLAG_C = src << (9-shift);
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsr_16_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src >> shift;

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_CLEAR;
	FLAG_Z = res;
	FLAG_X = FLAG_C = src << (9-shift);
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsr_32_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = *r_dst;
	uint res = src >> shift;

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = res;

	FLAG_N = NFLAG_CLEAR;
	FLAG_Z = res;
	FLAG_X = FLAG_C = src << (9-shift);
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsr_8_r(void)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src >> shift;

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift <= 8)
		{
			*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
			FLAG_X = FLAG_C = src << (9-shift);
			FLAG_N = NFLAG_CLEAR;
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		*r_dst &= 0xffffff00;
		FLAG_X = XFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_8(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsr_16_r(void)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src >> shift;

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift <= 16)
		{
			*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
			FLAG_C = FLAG_X = (src >> (shift - 1))<<8;
			FLAG_N = NFLAG_CLEAR;
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		*r_dst &= 0xffff0000;
		FLAG_X = XFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_16(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsr_32_r(void)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = *r_dst;
	uint res = src >> shift;

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift < 32)
		{
			*r_dst = res;
			FLAG_C = FLAG_X = (src >> (shift - 1))<<8;
			FLAG_N = NFLAG_CLEAR;
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		*r_dst = 0;
		FLAG_X = FLAG_C = (shift == 32 ? GET_MSB_32(src)>>23 : 0);
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_32(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsr_16_ai(void)
{
	uint ea = EA_AY_AI_16();
	uint src = m68ki_read_16(ea);
	uint res = src >> 1;

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_CLEAR;
	FLAG_Z = res;
	FLAG_C = FLAG_X = src << 8;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsr_16_pi(void)
{
	uint ea = EA_AY_PI_16();
	uint src = m68ki_read_16(ea);
	uint res = src >> 1;

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_CLEAR;
	FLAG_Z = res;
	FLAG_C = FLAG_X = src << 8;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsr_16_pd(void)
{
	uint ea = EA_AY_PD_16();
	uint src = m68ki_read_16(ea);
	uint res = src >> 1;

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_CLEAR;
	FLAG_Z = res;
	FLAG_C = FLAG_X = src << 8;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsr_16_di(void)
{
	uint ea = EA_AY_DI_16();
	uint src = m68ki_read_16(ea);
	uint res = src >> 1;

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_CLEAR;
	FLAG_Z = res;
	FLAG_C = FLAG_X = src << 8;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsr_16_ix(void)
{
	uint ea = EA_AY_IX_16();
	uint src = m68ki_read_16(ea);
	uint res = src >> 1;

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_CLEAR;
	FLAG_Z = res;
	FLAG_C = FLAG_X = src << 8;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsr_16_aw(void)
{
	uint ea = EA_AW_16();
	uint src = m68ki_read_16(ea);
	uint res = src >> 1;

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_CLEAR;
	FLAG_Z = res;
	FLAG_C = FLAG_X = src << 8;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsr_16_al(void)
{
	uint ea = EA_AL_16();
	uint src = m68ki_read_16(ea);
	uint res = src >> 1;

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_CLEAR;
	FLAG_Z = res;
	FLAG_C = FLAG_X = src << 8;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsl_8_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = MASK_OUT_ABOVE_8(src << shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src << shift;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsl_16_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = MASK_OUT_ABOVE_16(src << shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> (8-shift);
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsl_32_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = *r_dst;
	uint res = MASK_OUT_ABOVE_32(src << shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> (24-shift);
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsl_8_r(void)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = MASK_OUT_ABOVE_8(src << shift);

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift <= 8)
		{
			*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
			FLAG_X = FLAG_C = src << shift;
			FLAG_N = NFLAG_8(res);
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		*r_dst &= 0xffffff00;
		FLAG_X = XFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_8(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsl_16_r(void)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = MASK_OUT_ABOVE_16(src << shift);

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift <= 16)
		{
			*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
			FLAG_X = FLAG_C = (src << shift) >> 8;
			FLAG_N = NFLAG_16(res);
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		*r_dst &= 0xffff0000;
		FLAG_X = XFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_16(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsl_32_r(void)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = *r_dst;
	uint res = MASK_OUT_ABOVE_32(src << shift);

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift < 32)
		{
			*r_dst = res;
			FLAG_X = FLAG_C = (src >> (32 - shift)) << 8;
			FLAG_N = NFLAG_32(res);
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		*r_dst = 0;
		FLAG_X = FLAG_C = ((shift == 32 ? src & 1 : 0))<<8;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_32(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsl_16_ai(void)
{
	uint ea = EA_AY_AI_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> 7;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsl_16_pi(void)
{
	uint ea = EA_AY_PI_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> 7;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsl_16_pd(void)
{
	uint ea = EA_AY_PD_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> 7;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsl_16_di(void)
{
	uint ea = EA_AY_DI_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> 7;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsl_16_ix(void)
{
	uint ea = EA_AY_IX_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> 7;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsl_16_aw(void)
{
	uint ea = EA_AW_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> 7;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_lsl_16_al(void)
{
	uint ea = EA_AL_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> 7;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_move_8_d_d(void)
{
	uint res = MASK_OUT_ABOVE_8(DY);
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_d_ai(void)
{
	uint res = OPER_AY_AI_8();
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_d_pi(void)
{
	uint res = OPER_AY_PI_8();
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_d_pi7(void)
{
	uint res = OPER_A7_PI_8();
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_d_pd(void)
{
	uint res = OPER_AY_PD_8();
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_d_pd7(void)
{
	uint res = OPER_A7_PD_8();
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_d_di(void)
{
	uint res = OPER_AY_DI_8();
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_d_ix(void)
{
	uint res = OPER_AY_IX_8();
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_d_aw(void)
{
	uint res = OPER_AW_8();
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_d_al(void)
{
	uint res = OPER_AL_8();
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_d_pcdi(void)
{
	uint res = OPER_PCDI_8();
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_d_pcix(void)
{
	uint res = OPER_PCIX_8();
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_d_i(void)
{
	uint res = OPER_I_8();
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ai_d(void)
{
	uint res = MASK_OUT_ABOVE_8(DY);
	uint ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ai_ai(void)
{
	uint res = OPER_AY_AI_8();
	uint ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ai_pi(void)
{
	uint res = OPER_AY_PI_8();
	uint ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ai_pi7(void)
{
	uint res = OPER_A7_PI_8();
	uint ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ai_pd(void)
{
	uint res = OPER_AY_PD_8();
	uint ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ai_pd7(void)
{
	uint res = OPER_A7_PD_8();
	uint ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ai_di(void)
{
	uint res = OPER_AY_DI_8();
	uint ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ai_ix(void)
{
	uint res = OPER_AY_IX_8();
	uint ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ai_aw(void)
{
	uint res = OPER_AW_8();
	uint ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ai_al(void)
{
	uint res = OPER_AL_8();
	uint ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ai_pcdi(void)
{
	uint res = OPER_PCDI_8();
	uint ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ai_pcix(void)
{
	uint res = OPER_PCIX_8();
	uint ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ai_i(void)
{
	uint res = OPER_I_8();
	uint ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi7_d(void)
{
	uint res = MASK_OUT_ABOVE_8(DY);
	uint ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi_d(void)
{
	uint res = MASK_OUT_ABOVE_8(DY);
	uint ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi7_ai(void)
{
	uint res = OPER_AY_AI_8();
	uint ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi7_pi(void)
{
	uint res = OPER_AY_PI_8();
	uint ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi7_pi7(void)
{
	uint res = OPER_A7_PI_8();
	uint ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi7_pd(void)
{
	uint res = OPER_AY_PD_8();
	uint ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi7_pd7(void)
{
	uint res = OPER_A7_PD_8();
	uint ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi7_di(void)
{
	uint res = OPER_AY_DI_8();
	uint ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi7_ix(void)
{
	uint res = OPER_AY_IX_8();
	uint ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi7_aw(void)
{
	uint res = OPER_AW_8();
	uint ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi7_al(void)
{
	uint res = OPER_AL_8();
	uint ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi7_pcdi(void)
{
	uint res = OPER_PCDI_8();
	uint ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi7_pcix(void)
{
	uint res = OPER_PCIX_8();
	uint ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi7_i(void)
{
	uint res = OPER_I_8();
	uint ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi_ai(void)
{
	uint res = OPER_AY_AI_8();
	uint ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi_pi(void)
{
	uint res = OPER_AY_PI_8();
	uint ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi_pi7(void)
{
	uint res = OPER_A7_PI_8();
	uint ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi_pd(void)
{
	uint res = OPER_AY_PD_8();
	uint ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi_pd7(void)
{
	uint res = OPER_A7_PD_8();
	uint ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi_di(void)
{
	uint res = OPER_AY_DI_8();
	uint ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi_ix(void)
{
	uint res = OPER_AY_IX_8();
	uint ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi_aw(void)
{
	uint res = OPER_AW_8();
	uint ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi_al(void)
{
	uint res = OPER_AL_8();
	uint ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi_pcdi(void)
{
	uint res = OPER_PCDI_8();
	uint ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi_pcix(void)
{
	uint res = OPER_PCIX_8();
	uint ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pi_i(void)
{
	uint res = OPER_I_8();
	uint ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd7_d(void)
{
	uint res = MASK_OUT_ABOVE_8(DY);
	uint ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd_d(void)
{
	uint res = MASK_OUT_ABOVE_8(DY);
	uint ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd7_ai(void)
{
	uint res = OPER_AY_AI_8();
	uint ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd7_pi(void)
{
	uint res = OPER_AY_PI_8();
	uint ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd7_pi7(void)
{
	uint res = OPER_A7_PI_8();
	uint ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd7_pd(void)
{
	uint res = OPER_AY_PD_8();
	uint ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd7_pd7(void)
{
	uint res = OPER_A7_PD_8();
	uint ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd7_di(void)
{
	uint res = OPER_AY_DI_8();
	uint ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd7_ix(void)
{
	uint res = OPER_AY_IX_8();
	uint ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd7_aw(void)
{
	uint res = OPER_AW_8();
	uint ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd7_al(void)
{
	uint res = OPER_AL_8();
	uint ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd7_pcdi(void)
{
	uint res = OPER_PCDI_8();
	uint ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd7_pcix(void)
{
	uint res = OPER_PCIX_8();
	uint ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd7_i(void)
{
	uint res = OPER_I_8();
	uint ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd_ai(void)
{
	uint res = OPER_AY_AI_8();
	uint ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd_pi(void)
{
	uint res = OPER_AY_PI_8();
	uint ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd_pi7(void)
{
	uint res = OPER_A7_PI_8();
	uint ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd_pd(void)
{
	uint res = OPER_AY_PD_8();
	uint ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd_pd7(void)
{
	uint res = OPER_A7_PD_8();
	uint ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd_di(void)
{
	uint res = OPER_AY_DI_8();
	uint ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd_ix(void)
{
	uint res = OPER_AY_IX_8();
	uint ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd_aw(void)
{
	uint res = OPER_AW_8();
	uint ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd_al(void)
{
	uint res = OPER_AL_8();
	uint ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd_pcdi(void)
{
	uint res = OPER_PCDI_8();
	uint ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd_pcix(void)
{
	uint res = OPER_PCIX_8();
	uint ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_pd_i(void)
{
	uint res = OPER_I_8();
	uint ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_di_d(void)
{
	uint res = MASK_OUT_ABOVE_8(DY);
	uint ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_di_ai(void)
{
	uint res = OPER_AY_AI_8();
	uint ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_di_pi(void)
{
	uint res = OPER_AY_PI_8();
	uint ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_di_pi7(void)
{
	uint res = OPER_A7_PI_8();
	uint ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_di_pd(void)
{
	uint res = OPER_AY_PD_8();
	uint ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_di_pd7(void)
{
	uint res = OPER_A7_PD_8();
	uint ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_di_di(void)
{
	uint res = OPER_AY_DI_8();
	uint ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_di_ix(void)
{
	uint res = OPER_AY_IX_8();
	uint ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_di_aw(void)
{
	uint res = OPER_AW_8();
	uint ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_di_al(void)
{
	uint res = OPER_AL_8();
	uint ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_di_pcdi(void)
{
	uint res = OPER_PCDI_8();
	uint ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_di_pcix(void)
{
	uint res = OPER_PCIX_8();
	uint ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_di_i(void)
{
	uint res = OPER_I_8();
	uint ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ix_d(void)
{
	uint res = MASK_OUT_ABOVE_8(DY);
	uint ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ix_ai(void)
{
	uint res = OPER_AY_AI_8();
	uint ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ix_pi(void)
{
	uint res = OPER_AY_PI_8();
	uint ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ix_pi7(void)
{
	uint res = OPER_A7_PI_8();
	uint ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ix_pd(void)
{
	uint res = OPER_AY_PD_8();
	uint ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ix_pd7(void)
{
	uint res = OPER_A7_PD_8();
	uint ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ix_di(void)
{
	uint res = OPER_AY_DI_8();
	uint ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ix_ix(void)
{
	uint res = OPER_AY_IX_8();
	uint ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ix_aw(void)
{
	uint res = OPER_AW_8();
	uint ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ix_al(void)
{
	uint res = OPER_AL_8();
	uint ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ix_pcdi(void)
{
	uint res = OPER_PCDI_8();
	uint ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ix_pcix(void)
{
	uint res = OPER_PCIX_8();
	uint ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_ix_i(void)
{
	uint res = OPER_I_8();
	uint ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_aw_d(void)
{
	uint res = MASK_OUT_ABOVE_8(DY);
	uint ea = EA_AW_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_aw_ai(void)
{
	uint res = OPER_AY_AI_8();
	uint ea = EA_AW_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_aw_pi(void)
{
	uint res = OPER_AY_PI_8();
	uint ea = EA_AW_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_aw_pi7(void)
{
	uint res = OPER_A7_PI_8();
	uint ea = EA_AW_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_aw_pd(void)
{
	uint res = OPER_AY_PD_8();
	uint ea = EA_AW_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_aw_pd7(void)
{
	uint res = OPER_A7_PD_8();
	uint ea = EA_AW_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_aw_di(void)
{
	uint res = OPER_AY_DI_8();
	uint ea = EA_AW_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_aw_ix(void)
{
	uint res = OPER_AY_IX_8();
	uint ea = EA_AW_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_aw_aw(void)
{
	uint res = OPER_AW_8();
	uint ea = EA_AW_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_aw_al(void)
{
	uint res = OPER_AL_8();
	uint ea = EA_AW_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_aw_pcdi(void)
{
	uint res = OPER_PCDI_8();
	uint ea = EA_AW_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_aw_pcix(void)
{
	uint res = OPER_PCIX_8();
	uint ea = EA_AW_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_aw_i(void)
{
	uint res = OPER_I_8();
	uint ea = EA_AW_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_al_d(void)
{
	uint res = MASK_OUT_ABOVE_8(DY);
	uint ea = EA_AL_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_al_ai(void)
{
	uint res = OPER_AY_AI_8();
	uint ea = EA_AL_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_al_pi(void)
{
	uint res = OPER_AY_PI_8();
	uint ea = EA_AL_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_al_pi7(void)
{
	uint res = OPER_A7_PI_8();
	uint ea = EA_AL_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_al_pd(void)
{
	uint res = OPER_AY_PD_8();
	uint ea = EA_AL_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_al_pd7(void)
{
	uint res = OPER_A7_PD_8();
	uint ea = EA_AL_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_al_di(void)
{
	uint res = OPER_AY_DI_8();
	uint ea = EA_AL_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_al_ix(void)
{
	uint res = OPER_AY_IX_8();
	uint ea = EA_AL_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_al_aw(void)
{
	uint res = OPER_AW_8();
	uint ea = EA_AL_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_al_al(void)
{
	uint res = OPER_AL_8();
	uint ea = EA_AL_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_al_pcdi(void)
{
	uint res = OPER_PCDI_8();
	uint ea = EA_AL_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_al_pcix(void)
{
	uint res = OPER_PCIX_8();
	uint ea = EA_AL_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_8_al_i(void)
{
	uint res = OPER_I_8();
	uint ea = EA_AL_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_d_d(void)
{
	uint res = MASK_OUT_ABOVE_16(DY);
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_d_a(void)
{
	uint res = MASK_OUT_ABOVE_16(AY);
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_d_ai(void)
{
	uint res = OPER_AY_AI_16();
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_d_pi(void)
{
	uint res = OPER_AY_PI_16();
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_d_pd(void)
{
	uint res = OPER_AY_PD_16();
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_d_di(void)
{
	uint res = OPER_AY_DI_16();
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_d_ix(void)
{
	uint res = OPER_AY_IX_16();
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_d_aw(void)
{
	uint res = OPER_AW_16();
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_d_al(void)
{
	uint res = OPER_AL_16();
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_d_pcdi(void)
{
	uint res = OPER_PCDI_16();
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_d_pcix(void)
{
	uint res = OPER_PCIX_16();
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_d_i(void)
{
	uint res = OPER_I_16();
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_ai_d(void)
{
	uint res = MASK_OUT_ABOVE_16(DY);
	uint ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_ai_a(void)
{
	uint res = MASK_OUT_ABOVE_16(AY);
	uint ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_ai_ai(void)
{
	uint res = OPER_AY_AI_16();
	uint ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_ai_pi(void)
{
	uint res = OPER_AY_PI_16();
	uint ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_ai_pd(void)
{
	uint res = OPER_AY_PD_16();
	uint ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_ai_di(void)
{
	uint res = OPER_AY_DI_16();
	uint ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_ai_ix(void)
{
	uint res = OPER_AY_IX_16();
	uint ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_ai_aw(void)
{
	uint res = OPER_AW_16();
	uint ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_ai_al(void)
{
	uint res = OPER_AL_16();
	uint ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_ai_pcdi(void)
{
	uint res = OPER_PCDI_16();
	uint ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_ai_pcix(void)
{
	uint res = OPER_PCIX_16();
	uint ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_ai_i(void)
{
	uint res = OPER_I_16();
	uint ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_pi_d(void)
{
	uint res = MASK_OUT_ABOVE_16(DY);
	uint ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_pi_a(void)
{
	uint res = MASK_OUT_ABOVE_16(AY);
	uint ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_pi_ai(void)
{
	uint res = OPER_AY_AI_16();
	uint ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_pi_pi(void)
{
	uint res = OPER_AY_PI_16();
	uint ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_pi_pd(void)
{
	uint res = OPER_AY_PD_16();
	uint ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_pi_di(void)
{
	uint res = OPER_AY_DI_16();
	uint ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_pi_ix(void)
{
	uint res = OPER_AY_IX_16();
	uint ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_pi_aw(void)
{
	uint res = OPER_AW_16();
	uint ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_pi_al(void)
{
	uint res = OPER_AL_16();
	uint ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_pi_pcdi(void)
{
	uint res = OPER_PCDI_16();
	uint ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_pi_pcix(void)
{
	uint res = OPER_PCIX_16();
	uint ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_pi_i(void)
{
	uint res = OPER_I_16();
	uint ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_pd_d(void)
{
	uint res = MASK_OUT_ABOVE_16(DY);
	uint ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_pd_a(void)
{
	uint res = MASK_OUT_ABOVE_16(AY);
	uint ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_pd_ai(void)
{
	uint res = OPER_AY_AI_16();
	uint ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_pd_pi(void)
{
	uint res = OPER_AY_PI_16();
	uint ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_pd_pd(void)
{
	uint res = OPER_AY_PD_16();
	uint ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_pd_di(void)
{
	uint res = OPER_AY_DI_16();
	uint ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_pd_ix(void)
{
	uint res = OPER_AY_IX_16();
	uint ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_pd_aw(void)
{
	uint res = OPER_AW_16();
	uint ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_pd_al(void)
{
	uint res = OPER_AL_16();
	uint ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_pd_pcdi(void)
{
	uint res = OPER_PCDI_16();
	uint ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_pd_pcix(void)
{
	uint res = OPER_PCIX_16();
	uint ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_pd_i(void)
{
	uint res = OPER_I_16();
	uint ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_di_d(void)
{
	uint res = MASK_OUT_ABOVE_16(DY);
	uint ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_di_a(void)
{
	uint res = MASK_OUT_ABOVE_16(AY);
	uint ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_di_ai(void)
{
	uint res = OPER_AY_AI_16();
	uint ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_di_pi(void)
{
	uint res = OPER_AY_PI_16();
	uint ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_di_pd(void)
{
	uint res = OPER_AY_PD_16();
	uint ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_di_di(void)
{
	uint res = OPER_AY_DI_16();
	uint ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_di_ix(void)
{
	uint res = OPER_AY_IX_16();
	uint ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_di_aw(void)
{
	uint res = OPER_AW_16();
	uint ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_di_al(void)
{
	uint res = OPER_AL_16();
	uint ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_di_pcdi(void)
{
	uint res = OPER_PCDI_16();
	uint ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_di_pcix(void)
{
	uint res = OPER_PCIX_16();
	uint ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_di_i(void)
{
	uint res = OPER_I_16();
	uint ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_ix_d(void)
{
	uint res = MASK_OUT_ABOVE_16(DY);
	uint ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_ix_a(void)
{
	uint res = MASK_OUT_ABOVE_16(AY);
	uint ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_ix_ai(void)
{
	uint res = OPER_AY_AI_16();
	uint ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_ix_pi(void)
{
	uint res = OPER_AY_PI_16();
	uint ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_ix_pd(void)
{
	uint res = OPER_AY_PD_16();
	uint ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_ix_di(void)
{
	uint res = OPER_AY_DI_16();
	uint ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_ix_ix(void)
{
	uint res = OPER_AY_IX_16();
	uint ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_ix_aw(void)
{
	uint res = OPER_AW_16();
	uint ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_ix_al(void)
{
	uint res = OPER_AL_16();
	uint ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_ix_pcdi(void)
{
	uint res = OPER_PCDI_16();
	uint ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_ix_pcix(void)
{
	uint res = OPER_PCIX_16();
	uint ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_ix_i(void)
{
	uint res = OPER_I_16();
	uint ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_aw_d(void)
{
	uint res = MASK_OUT_ABOVE_16(DY);
	uint ea = EA_AW_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_aw_a(void)
{
	uint res = MASK_OUT_ABOVE_16(AY);
	uint ea = EA_AW_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_aw_ai(void)
{
	uint res = OPER_AY_AI_16();
	uint ea = EA_AW_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_aw_pi(void)
{
	uint res = OPER_AY_PI_16();
	uint ea = EA_AW_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_aw_pd(void)
{
	uint res = OPER_AY_PD_16();
	uint ea = EA_AW_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_aw_di(void)
{
	uint res = OPER_AY_DI_16();
	uint ea = EA_AW_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_aw_ix(void)
{
	uint res = OPER_AY_IX_16();
	uint ea = EA_AW_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_aw_aw(void)
{
	uint res = OPER_AW_16();
	uint ea = EA_AW_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_aw_al(void)
{
	uint res = OPER_AL_16();
	uint ea = EA_AW_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_aw_pcdi(void)
{
	uint res = OPER_PCDI_16();
	uint ea = EA_AW_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_aw_pcix(void)
{
	uint res = OPER_PCIX_16();
	uint ea = EA_AW_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_aw_i(void)
{
	uint res = OPER_I_16();
	uint ea = EA_AW_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_al_d(void)
{
	uint res = MASK_OUT_ABOVE_16(DY);
	uint ea = EA_AL_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_al_a(void)
{
	uint res = MASK_OUT_ABOVE_16(AY);
	uint ea = EA_AL_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_al_ai(void)
{
	uint res = OPER_AY_AI_16();
	uint ea = EA_AL_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_al_pi(void)
{
	uint res = OPER_AY_PI_16();
	uint ea = EA_AL_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_al_pd(void)
{
	uint res = OPER_AY_PD_16();
	uint ea = EA_AL_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_al_di(void)
{
	uint res = OPER_AY_DI_16();
	uint ea = EA_AL_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_al_ix(void)
{
	uint res = OPER_AY_IX_16();
	uint ea = EA_AL_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_al_aw(void)
{
	uint res = OPER_AW_16();
	uint ea = EA_AL_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_al_al(void)
{
	uint res = OPER_AL_16();
	uint ea = EA_AL_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_al_pcdi(void)
{
	uint res = OPER_PCDI_16();
	uint ea = EA_AL_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_al_pcix(void)
{
	uint res = OPER_PCIX_16();
	uint ea = EA_AL_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_16_al_i(void)
{
	uint res = OPER_I_16();
	uint ea = EA_AL_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_d_d(void)
{
	uint res = DY;
	uint* r_dst = &DX;

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_d_a(void)
{
	uint res = AY;
	uint* r_dst = &DX;

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_d_ai(void)
{
	uint res = OPER_AY_AI_32();
	uint* r_dst = &DX;

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_d_pi(void)
{
	uint res = OPER_AY_PI_32();
	uint* r_dst = &DX;

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_d_pd(void)
{
	uint res = OPER_AY_PD_32();
	uint* r_dst = &DX;

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_d_di(void)
{
	uint res = OPER_AY_DI_32();
	uint* r_dst = &DX;

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_d_ix(void)
{
	uint res = OPER_AY_IX_32();
	uint* r_dst = &DX;

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_d_aw(void)
{
	uint res = OPER_AW_32();
	uint* r_dst = &DX;

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_d_al(void)
{
	uint res = OPER_AL_32();
	uint* r_dst = &DX;

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_d_pcdi(void)
{
	uint res = OPER_PCDI_32();
	uint* r_dst = &DX;

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_d_pcix(void)
{
	uint res = OPER_PCIX_32();
	uint* r_dst = &DX;

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_d_i(void)
{
	uint res = OPER_I_32();
	uint* r_dst = &DX;

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_ai_d(void)
{
	uint res = DY;
	uint ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_ai_a(void)
{
	uint res = AY;
	uint ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_ai_ai(void)
{
	uint res = OPER_AY_AI_32();
	uint ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_ai_pi(void)
{
	uint res = OPER_AY_PI_32();
	uint ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_ai_pd(void)
{
	uint res = OPER_AY_PD_32();
	uint ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_ai_di(void)
{
	uint res = OPER_AY_DI_32();
	uint ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_ai_ix(void)
{
	uint res = OPER_AY_IX_32();
	uint ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_ai_aw(void)
{
	uint res = OPER_AW_32();
	uint ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_ai_al(void)
{
	uint res = OPER_AL_32();
	uint ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_ai_pcdi(void)
{
	uint res = OPER_PCDI_32();
	uint ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_ai_pcix(void)
{
	uint res = OPER_PCIX_32();
	uint ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_ai_i(void)
{
	uint res = OPER_I_32();
	uint ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_pi_d(void)
{
	uint res = DY;
	uint ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_pi_a(void)
{
	uint res = AY;
	uint ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_pi_ai(void)
{
	uint res = OPER_AY_AI_32();
	uint ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_pi_pi(void)
{
	uint res = OPER_AY_PI_32();
	uint ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_pi_pd(void)
{
	uint res = OPER_AY_PD_32();
	uint ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_pi_di(void)
{
	uint res = OPER_AY_DI_32();
	uint ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_pi_ix(void)
{
	uint res = OPER_AY_IX_32();
	uint ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_pi_aw(void)
{
	uint res = OPER_AW_32();
	uint ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_pi_al(void)
{
	uint res = OPER_AL_32();
	uint ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_pi_pcdi(void)
{
	uint res = OPER_PCDI_32();
	uint ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_pi_pcix(void)
{
	uint res = OPER_PCIX_32();
	uint ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_pi_i(void)
{
	uint res = OPER_I_32();
	uint ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_pd_d(void)
{
	uint res = DY;
	uint ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_pd_a(void)
{
	uint res = AY;
	uint ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_pd_ai(void)
{
	uint res = OPER_AY_AI_32();
	uint ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_pd_pi(void)
{
	uint res = OPER_AY_PI_32();
	uint ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_pd_pd(void)
{
	uint res = OPER_AY_PD_32();
	uint ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_pd_di(void)
{
	uint res = OPER_AY_DI_32();
	uint ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_pd_ix(void)
{
	uint res = OPER_AY_IX_32();
	uint ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_pd_aw(void)
{
	uint res = OPER_AW_32();
	uint ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_pd_al(void)
{
	uint res = OPER_AL_32();
	uint ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_pd_pcdi(void)
{
	uint res = OPER_PCDI_32();
	uint ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_pd_pcix(void)
{
	uint res = OPER_PCIX_32();
	uint ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_pd_i(void)
{
	uint res = OPER_I_32();
	uint ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_di_d(void)
{
	uint res = DY;
	uint ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_di_a(void)
{
	uint res = AY;
	uint ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_di_ai(void)
{
	uint res = OPER_AY_AI_32();
	uint ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_di_pi(void)
{
	uint res = OPER_AY_PI_32();
	uint ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_di_pd(void)
{
	uint res = OPER_AY_PD_32();
	uint ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_di_di(void)
{
	uint res = OPER_AY_DI_32();
	uint ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_di_ix(void)
{
	uint res = OPER_AY_IX_32();
	uint ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_di_aw(void)
{
	uint res = OPER_AW_32();
	uint ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_di_al(void)
{
	uint res = OPER_AL_32();
	uint ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_di_pcdi(void)
{
	uint res = OPER_PCDI_32();
	uint ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_di_pcix(void)
{
	uint res = OPER_PCIX_32();
	uint ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_di_i(void)
{
	uint res = OPER_I_32();
	uint ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_ix_d(void)
{
	uint res = DY;
	uint ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_ix_a(void)
{
	uint res = AY;
	uint ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_ix_ai(void)
{
	uint res = OPER_AY_AI_32();
	uint ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_ix_pi(void)
{
	uint res = OPER_AY_PI_32();
	uint ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_ix_pd(void)
{
	uint res = OPER_AY_PD_32();
	uint ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_ix_di(void)
{
	uint res = OPER_AY_DI_32();
	uint ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_ix_ix(void)
{
	uint res = OPER_AY_IX_32();
	uint ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_ix_aw(void)
{
	uint res = OPER_AW_32();
	uint ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_ix_al(void)
{
	uint res = OPER_AL_32();
	uint ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_ix_pcdi(void)
{
	uint res = OPER_PCDI_32();
	uint ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_ix_pcix(void)
{
	uint res = OPER_PCIX_32();
	uint ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_ix_i(void)
{
	uint res = OPER_I_32();
	uint ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_aw_d(void)
{
	uint res = DY;
	uint ea = EA_AW_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_aw_a(void)
{
	uint res = AY;
	uint ea = EA_AW_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_aw_ai(void)
{
	uint res = OPER_AY_AI_32();
	uint ea = EA_AW_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_aw_pi(void)
{
	uint res = OPER_AY_PI_32();
	uint ea = EA_AW_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_aw_pd(void)
{
	uint res = OPER_AY_PD_32();
	uint ea = EA_AW_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_aw_di(void)
{
	uint res = OPER_AY_DI_32();
	uint ea = EA_AW_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_aw_ix(void)
{
	uint res = OPER_AY_IX_32();
	uint ea = EA_AW_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_aw_aw(void)
{
	uint res = OPER_AW_32();
	uint ea = EA_AW_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_aw_al(void)
{
	uint res = OPER_AL_32();
	uint ea = EA_AW_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_aw_pcdi(void)
{
	uint res = OPER_PCDI_32();
	uint ea = EA_AW_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_aw_pcix(void)
{
	uint res = OPER_PCIX_32();
	uint ea = EA_AW_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_aw_i(void)
{
	uint res = OPER_I_32();
	uint ea = EA_AW_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_al_d(void)
{
	uint res = DY;
	uint ea = EA_AL_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_al_a(void)
{
	uint res = AY;
	uint ea = EA_AL_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_al_ai(void)
{
	uint res = OPER_AY_AI_32();
	uint ea = EA_AL_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_al_pi(void)
{
	uint res = OPER_AY_PI_32();
	uint ea = EA_AL_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_al_pd(void)
{
	uint res = OPER_AY_PD_32();
	uint ea = EA_AL_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_al_di(void)
{
	uint res = OPER_AY_DI_32();
	uint ea = EA_AL_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_al_ix(void)
{
	uint res = OPER_AY_IX_32();
	uint ea = EA_AL_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_al_aw(void)
{
	uint res = OPER_AW_32();
	uint ea = EA_AL_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_al_al(void)
{
	uint res = OPER_AL_32();
	uint ea = EA_AL_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_al_pcdi(void)
{
	uint res = OPER_PCDI_32();
	uint ea = EA_AL_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_al_pcix(void)
{
	uint res = OPER_PCIX_32();
	uint ea = EA_AL_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move_32_al_i(void)
{
	uint res = OPER_I_32();
	uint ea = EA_AL_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_movea_16_d(void)
{
	AX = MAKE_INT_16(DY);
}


void m68k_op_movea_16_a(void)
{
	AX = MAKE_INT_16(AY);
}


void m68k_op_movea_16_ai(void)
{
	AX = MAKE_INT_16(OPER_AY_AI_16());
}


void m68k_op_movea_16_pi(void)
{
	AX = MAKE_INT_16(OPER_AY_PI_16());
}


void m68k_op_movea_16_pd(void)
{
	AX = MAKE_INT_16(OPER_AY_PD_16());
}


void m68k_op_movea_16_di(void)
{
	AX = MAKE_INT_16(OPER_AY_DI_16());
}


void m68k_op_movea_16_ix(void)
{
	AX = MAKE_INT_16(OPER_AY_IX_16());
}


void m68k_op_movea_16_aw(void)
{
	AX = MAKE_INT_16(OPER_AW_16());
}


void m68k_op_movea_16_al(void)
{
	AX = MAKE_INT_16(OPER_AL_16());
}


void m68k_op_movea_16_pcdi(void)
{
	AX = MAKE_INT_16(OPER_PCDI_16());
}


void m68k_op_movea_16_pcix(void)
{
	AX = MAKE_INT_16(OPER_PCIX_16());
}


void m68k_op_movea_16_i(void)
{
	AX = MAKE_INT_16(OPER_I_16());
}


void m68k_op_movea_32_d(void)
{
	AX = DY;
}


void m68k_op_movea_32_a(void)
{
	AX = AY;
}


void m68k_op_movea_32_ai(void)
{
	AX = OPER_AY_AI_32();
}


void m68k_op_movea_32_pi(void)
{
	AX = OPER_AY_PI_32();
}


void m68k_op_movea_32_pd(void)
{
	AX = OPER_AY_PD_32();
}


void m68k_op_movea_32_di(void)
{
	AX = OPER_AY_DI_32();
}


void m68k_op_movea_32_ix(void)
{
	AX = OPER_AY_IX_32();
}


void m68k_op_movea_32_aw(void)
{
	AX = OPER_AW_32();
}


void m68k_op_movea_32_al(void)
{
	AX = OPER_AL_32();
}


void m68k_op_movea_32_pcdi(void)
{
	AX = OPER_PCDI_32();
}


void m68k_op_movea_32_pcix(void)
{
	AX = OPER_PCIX_32();
}


void m68k_op_movea_32_i(void)
{
	AX = OPER_I_32();
}


void m68k_op_move_16_frc_d(void)
{

	m68ki_exception_illegal();
}


void m68k_op_move_16_frc_ai(void)
{

	m68ki_exception_illegal();
}


void m68k_op_move_16_frc_pi(void)
{

	m68ki_exception_illegal();
}


void m68k_op_move_16_frc_pd(void)
{

	m68ki_exception_illegal();
}


void m68k_op_move_16_frc_di(void)
{

	m68ki_exception_illegal();
}


void m68k_op_move_16_frc_ix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_move_16_frc_aw(void)
{

	m68ki_exception_illegal();
}


void m68k_op_move_16_frc_al(void)
{

	m68ki_exception_illegal();
}


void m68k_op_move_16_toc_d(void)
{
	m68ki_set_ccr(DY);
}


void m68k_op_move_16_toc_ai(void)
{
	m68ki_set_ccr(OPER_AY_AI_16());
}


void m68k_op_move_16_toc_pi(void)
{
	m68ki_set_ccr(OPER_AY_PI_16());
}


void m68k_op_move_16_toc_pd(void)
{
	m68ki_set_ccr(OPER_AY_PD_16());
}


void m68k_op_move_16_toc_di(void)
{
	m68ki_set_ccr(OPER_AY_DI_16());
}


void m68k_op_move_16_toc_ix(void)
{
	m68ki_set_ccr(OPER_AY_IX_16());
}


void m68k_op_move_16_toc_aw(void)
{
	m68ki_set_ccr(OPER_AW_16());
}


void m68k_op_move_16_toc_al(void)
{
	m68ki_set_ccr(OPER_AL_16());
}


void m68k_op_move_16_toc_pcdi(void)
{
	m68ki_set_ccr(OPER_PCDI_16());
}


void m68k_op_move_16_toc_pcix(void)
{
	m68ki_set_ccr(OPER_PCIX_16());
}


void m68k_op_move_16_toc_i(void)
{
	m68ki_set_ccr(OPER_I_16());
}


void m68k_op_move_16_frs_d(void)
{
	if(1 || FLAG_S)	/* NS990408 */
	{
		DY = MASK_OUT_BELOW_16(DY) | m68ki_get_sr();
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_move_16_frs_ai(void)
{
	if(1 || FLAG_S)	/* NS990408 */
	{
		uint ea = EA_AY_AI_16();
		m68ki_write_16(ea, m68ki_get_sr());
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_move_16_frs_pi(void)
{
	if(1 || FLAG_S)	/* NS990408 */
	{
		uint ea = EA_AY_PI_16();
		m68ki_write_16(ea, m68ki_get_sr());
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_move_16_frs_pd(void)
{
	if(1 || FLAG_S)	/* NS990408 */
	{
		uint ea = EA_AY_PD_16();
		m68ki_write_16(ea, m68ki_get_sr());
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_move_16_frs_di(void)
{
	if(1 || FLAG_S)	/* NS990408 */
	{
		uint ea = EA_AY_DI_16();
		m68ki_write_16(ea, m68ki_get_sr());
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_move_16_frs_ix(void)
{
	if(1 || FLAG_S)	/* NS990408 */
	{
		uint ea = EA_AY_IX_16();
		m68ki_write_16(ea, m68ki_get_sr());
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_move_16_frs_aw(void)
{
	if(1 || FLAG_S)	/* NS990408 */
	{
		uint ea = EA_AW_16();
		m68ki_write_16(ea, m68ki_get_sr());
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_move_16_frs_al(void)
{
	if(1 || FLAG_S)	/* NS990408 */
	{
		uint ea = EA_AL_16();
		m68ki_write_16(ea, m68ki_get_sr());
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_move_16_tos_d(void)
{
	if(FLAG_S)
	{
		m68ki_set_sr(DY);
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_move_16_tos_ai(void)
{
	if(FLAG_S)
	{
		uint new_sr = OPER_AY_AI_16();
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(new_sr);
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_move_16_tos_pi(void)
{
	if(FLAG_S)
	{
		uint new_sr = OPER_AY_PI_16();
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(new_sr);
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_move_16_tos_pd(void)
{
	if(FLAG_S)
	{
		uint new_sr = OPER_AY_PD_16();
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(new_sr);
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_move_16_tos_di(void)
{
	if(FLAG_S)
	{
		uint new_sr = OPER_AY_DI_16();
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(new_sr);
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_move_16_tos_ix(void)
{
	if(FLAG_S)
	{
		uint new_sr = OPER_AY_IX_16();
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(new_sr);
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_move_16_tos_aw(void)
{
	if(FLAG_S)
	{
		uint new_sr = OPER_AW_16();
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(new_sr);
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_move_16_tos_al(void)
{
	if(FLAG_S)
	{
		uint new_sr = OPER_AL_16();
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(new_sr);
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_move_16_tos_pcdi(void)
{
	if(FLAG_S)
	{
		uint new_sr = OPER_PCDI_16();
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(new_sr);
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_move_16_tos_pcix(void)
{
	if(FLAG_S)
	{
		uint new_sr = OPER_PCIX_16();
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(new_sr);
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_move_16_tos_i(void)
{
	if(FLAG_S)
	{
		uint new_sr = OPER_I_16();
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(new_sr);
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_move_32_fru(void)
{
	if(FLAG_S)
	{
		AY = REG_USP;
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_move_32_tou(void)
{
	if(FLAG_S)
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		REG_USP = AY;
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_movec_32_cr(void)
{

	m68ki_exception_illegal();
}


void m68k_op_movec_32_rc(void)
{

	m68ki_exception_illegal();
}


void m68k_op_movem_16_re_pd(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = AY;
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			ea -= 2;
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_DA[15-i]));
			count++;
		}
	AY = ea;

	USE_CYCLES(count<<CYC_MOVEM_W);
}


void m68k_op_movem_16_re_ai(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_AY_AI_16();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_DA[i]));
			ea += 2;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_W);
}


void m68k_op_movem_16_re_di(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_AY_DI_16();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_DA[i]));
			ea += 2;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_W);
}


void m68k_op_movem_16_re_ix(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_AY_IX_16();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_DA[i]));
			ea += 2;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_W);
}


void m68k_op_movem_16_re_aw(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_AW_16();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_DA[i]));
			ea += 2;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_W);
}


void m68k_op_movem_16_re_al(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_AL_16();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_DA[i]));
			ea += 2;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_W);
}


void m68k_op_movem_32_re_pd(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = AY;
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			ea -= 4;
			m68ki_write_16(ea+2, REG_DA[15-i] & 0xFFFF );
			m68ki_write_16(ea, (REG_DA[15-i] >> 16) & 0xFFFF );
			count++;
		}
	AY = ea;

	USE_CYCLES(count<<CYC_MOVEM_L);
}


void m68k_op_movem_32_re_ai(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_AY_AI_32();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			m68ki_write_32(ea, REG_DA[i]);
			ea += 4;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_L);
}


void m68k_op_movem_32_re_di(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_AY_DI_32();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			m68ki_write_32(ea, REG_DA[i]);
			ea += 4;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_L);
}


void m68k_op_movem_32_re_ix(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_AY_IX_32();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			m68ki_write_32(ea, REG_DA[i]);
			ea += 4;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_L);
}


void m68k_op_movem_32_re_aw(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_AW_32();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			m68ki_write_32(ea, REG_DA[i]);
			ea += 4;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_L);
}


void m68k_op_movem_32_re_al(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_AL_32();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			m68ki_write_32(ea, REG_DA[i]);
			ea += 4;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_L);
}


void m68k_op_movem_16_er_pi(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = AY;
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_16(ea)));
			ea += 2;
			count++;
		}
	AY = ea;

	USE_CYCLES(count<<CYC_MOVEM_W);
}


void m68k_op_movem_16_er_pcdi(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_PCDI_16();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_pcrel_16(ea)));
			ea += 2;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_W);
}


void m68k_op_movem_16_er_pcix(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_PCIX_16();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_pcrel_16(ea)));
			ea += 2;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_W);
}


void m68k_op_movem_16_er_ai(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_AY_AI_16();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_16(ea)));
			ea += 2;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_W);
}


void m68k_op_movem_16_er_di(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_AY_DI_16();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_16(ea)));
			ea += 2;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_W);
}


void m68k_op_movem_16_er_ix(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_AY_IX_16();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_16(ea)));
			ea += 2;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_W);
}


void m68k_op_movem_16_er_aw(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_AW_16();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_16(ea)));
			ea += 2;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_W);
}


void m68k_op_movem_16_er_al(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_AL_16();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_16(ea)));
			ea += 2;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_W);
}


void m68k_op_movem_32_er_pi(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = AY;
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA[i] = m68ki_read_32(ea);
			ea += 4;
			count++;
		}
	AY = ea;

	USE_CYCLES(count<<CYC_MOVEM_L);
}


void m68k_op_movem_32_er_pcdi(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_PCDI_32();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA[i] = m68ki_read_pcrel_32(ea);
			ea += 4;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_L);
}


void m68k_op_movem_32_er_pcix(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_PCIX_32();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA[i] = m68ki_read_pcrel_32(ea);
			ea += 4;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_L);
}


void m68k_op_movem_32_er_ai(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_AY_AI_32();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA[i] = m68ki_read_32(ea);
			ea += 4;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_L);
}


void m68k_op_movem_32_er_di(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_AY_DI_32();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA[i] = m68ki_read_32(ea);
			ea += 4;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_L);
}


void m68k_op_movem_32_er_ix(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_AY_IX_32();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA[i] = m68ki_read_32(ea);
			ea += 4;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_L);
}


void m68k_op_movem_32_er_aw(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_AW_32();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA[i] = m68ki_read_32(ea);
			ea += 4;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_L);
}


void m68k_op_movem_32_er_al(void)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_AL_32();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA[i] = m68ki_read_32(ea);
			ea += 4;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_L);
}


void m68k_op_movep_16_re(void)
{
	uint ea = EA_AY_DI_16();
	uint src = DX;

	m68ki_write_8(ea, MASK_OUT_ABOVE_8(src >> 8));
	m68ki_write_8(ea += 2, MASK_OUT_ABOVE_8(src));
}


void m68k_op_movep_32_re(void)
{
	uint ea = EA_AY_DI_32();
	uint src = DX;

	m68ki_write_8(ea, MASK_OUT_ABOVE_8(src >> 24));
	m68ki_write_8(ea += 2, MASK_OUT_ABOVE_8(src >> 16));
	m68ki_write_8(ea += 2, MASK_OUT_ABOVE_8(src >> 8));
	m68ki_write_8(ea += 2, MASK_OUT_ABOVE_8(src));
}


void m68k_op_movep_16_er(void)
{
	uint ea = EA_AY_DI_16();
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | ((m68ki_read_8(ea) << 8) + m68ki_read_8(ea + 2));
}


void m68k_op_movep_32_er(void)
{
	uint ea = EA_AY_DI_32();

	DX = (m68ki_read_8(ea) << 24) + (m68ki_read_8(ea + 2) << 16)
		+ (m68ki_read_8(ea + 4) << 8) + m68ki_read_8(ea + 6);
}


void m68k_op_moves_8_ai(void)
{

	m68ki_exception_illegal();
}


void m68k_op_moves_8_pi(void)
{

	m68ki_exception_illegal();
}


void m68k_op_moves_8_pi7(void)
{

	m68ki_exception_illegal();
}


void m68k_op_moves_8_pd(void)
{

	m68ki_exception_illegal();
}


void m68k_op_moves_8_pd7(void)
{

	m68ki_exception_illegal();
}


void m68k_op_moves_8_di(void)
{

	m68ki_exception_illegal();
}


void m68k_op_moves_8_ix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_moves_8_aw(void)
{

	m68ki_exception_illegal();
}


void m68k_op_moves_8_al(void)
{

	m68ki_exception_illegal();
}


void m68k_op_moves_16_ai(void)
{

	m68ki_exception_illegal();
}


void m68k_op_moves_16_pi(void)
{

	m68ki_exception_illegal();
}


void m68k_op_moves_16_pd(void)
{

	m68ki_exception_illegal();
}


void m68k_op_moves_16_di(void)
{

	m68ki_exception_illegal();
}


void m68k_op_moves_16_ix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_moves_16_aw(void)
{

	m68ki_exception_illegal();
}


void m68k_op_moves_16_al(void)
{

	m68ki_exception_illegal();
}


void m68k_op_moves_32_ai(void)
{

	m68ki_exception_illegal();
}


void m68k_op_moves_32_pi(void)
{

	m68ki_exception_illegal();
}


void m68k_op_moves_32_pd(void)
{

	m68ki_exception_illegal();
}


void m68k_op_moves_32_di(void)
{

	m68ki_exception_illegal();
}


void m68k_op_moves_32_ix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_moves_32_aw(void)
{

	m68ki_exception_illegal();
}


void m68k_op_moves_32_al(void)
{

	m68ki_exception_illegal();
}


void m68k_op_moveq_32(void)
{
	uint res = DX = MAKE_INT_8(MASK_OUT_ABOVE_8(REG_IR));

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_move16_32(void)
{
	uint16 w2 = OPER_I_16();
	int ax = REG_IR & 7;
	int ay = (w2 >> 12) & 7;

	m68ki_write_32(REG_A[ay],    m68ki_read_32(REG_A[ax]));
	m68ki_write_32(REG_A[ay]+4,  m68ki_read_32(REG_A[ax]+4));
	m68ki_write_32(REG_A[ay]+8,  m68ki_read_32(REG_A[ax]+8));
	m68ki_write_32(REG_A[ay]+12, m68ki_read_32(REG_A[ax]+12));

	REG_A[ax] += 16;
	REG_A[ay] += 16;
}


void m68k_op_muls_16_d(void)
{
	uint* r_dst = &DX;
	uint res = MASK_OUT_ABOVE_32(MAKE_INT_16(DY) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_muls_16_ai(void)
{
	uint* r_dst = &DX;
	uint res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AY_AI_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_muls_16_pi(void)
{
	uint* r_dst = &DX;
	uint res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AY_PI_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_muls_16_pd(void)
{
	uint* r_dst = &DX;
	uint res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AY_PD_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_muls_16_di(void)
{
	uint* r_dst = &DX;
	uint res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AY_DI_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_muls_16_ix(void)
{
	uint* r_dst = &DX;
	uint res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AY_IX_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_muls_16_aw(void)
{
	uint* r_dst = &DX;
	uint res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AW_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_muls_16_al(void)
{
	uint* r_dst = &DX;
	uint res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AL_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_muls_16_pcdi(void)
{
	uint* r_dst = &DX;
	uint res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_PCDI_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_muls_16_pcix(void)
{
	uint* r_dst = &DX;
	uint res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_PCIX_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_muls_16_i(void)
{
	uint* r_dst = &DX;
	uint res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_I_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_mulu_16_d(void)
{
	uint* r_dst = &DX;
	uint res = MASK_OUT_ABOVE_16(DY) * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_mulu_16_ai(void)
{
	uint* r_dst = &DX;
	uint res = OPER_AY_AI_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_mulu_16_pi(void)
{
	uint* r_dst = &DX;
	uint res = OPER_AY_PI_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_mulu_16_pd(void)
{
	uint* r_dst = &DX;
	uint res = OPER_AY_PD_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_mulu_16_di(void)
{
	uint* r_dst = &DX;
	uint res = OPER_AY_DI_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_mulu_16_ix(void)
{
	uint* r_dst = &DX;
	uint res = OPER_AY_IX_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_mulu_16_aw(void)
{
	uint* r_dst = &DX;
	uint res = OPER_AW_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_mulu_16_al(void)
{
	uint* r_dst = &DX;
	uint res = OPER_AL_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_mulu_16_pcdi(void)
{
	uint* r_dst = &DX;
	uint res = OPER_PCDI_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_mulu_16_pcix(void)
{
	uint* r_dst = &DX;
	uint res = OPER_PCIX_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_mulu_16_i(void)
{
	uint* r_dst = &DX;
	uint res = OPER_I_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_mull_32_d(void)
{
#if M68K_USE_64_BIT

	if(0)
	{
		uint word2 = OPER_I_16();
		uint64 src = DY;
		uint64 dst = REG_D[(word2 >> 12) & 7];
		uint64 res;

		FLAG_C = CFLAG_CLEAR;

		if(BIT_B(word2))			   /* signed */
		{
			res = (sint64)((sint32)src) * (sint64)((sint32)dst);
			if(!BIT_A(word2))
			{
				FLAG_Z = MASK_OUT_ABOVE_32(res);
				FLAG_N = NFLAG_32(res);
				FLAG_V = ((sint64)res != (sint32)res)<<7;
				REG_D[(word2 >> 12) & 7] = FLAG_Z;
				return;
			}
			FLAG_Z = MASK_OUT_ABOVE_32(res) | (res>>32);
			FLAG_N = NFLAG_64(res);
			FLAG_V = VFLAG_CLEAR;
			REG_D[word2 & 7] = (res >> 32);
			REG_D[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
			return;
		}

		res = src * dst;
		if(!BIT_A(word2))
		{
			FLAG_Z = MASK_OUT_ABOVE_32(res);
			FLAG_N = NFLAG_32(res);
			FLAG_V = (res > 0xffffffff)<<7;
			REG_D[(word2 >> 12) & 7] = FLAG_Z;
			return;
		}
		FLAG_Z = MASK_OUT_ABOVE_32(res) | (res>>32);
		FLAG_N = NFLAG_64(res);
		FLAG_V = VFLAG_CLEAR;
		REG_D[word2 & 7] = (res >> 32);
		REG_D[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		return;
	}
	m68ki_exception_illegal();

#else


	m68ki_exception_illegal();

#endif
}


void m68k_op_mull_32_ai(void)
{
#if M68K_USE_64_BIT

	if(0)
	{
		uint word2 = OPER_I_16();
		uint64 src = OPER_AY_AI_32();
		uint64 dst = REG_D[(word2 >> 12) & 7];
		uint64 res;

		FLAG_C = CFLAG_CLEAR;

		if(BIT_B(word2))			   /* signed */
		{
			res = (sint64)((sint32)src) * (sint64)((sint32)dst);
			if(!BIT_A(word2))
			{
				FLAG_Z = MASK_OUT_ABOVE_32(res);
				FLAG_N = NFLAG_32(res);
				FLAG_V = ((sint64)res != (sint32)res)<<7;
				REG_D[(word2 >> 12) & 7] = FLAG_Z;
				return;
			}
			FLAG_Z = MASK_OUT_ABOVE_32(res) | (res>>32);
			FLAG_N = NFLAG_64(res);
			FLAG_V = VFLAG_CLEAR;
			REG_D[word2 & 7] = (res >> 32);
			REG_D[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
			return;
		}

		res = src * dst;
		if(!BIT_A(word2))
		{
			FLAG_Z = MASK_OUT_ABOVE_32(res);
			FLAG_N = NFLAG_32(res);
			FLAG_V = (res > 0xffffffff)<<7;
			REG_D[(word2 >> 12) & 7] = FLAG_Z;
			return;
		}
		FLAG_Z = MASK_OUT_ABOVE_32(res) | (res>>32);
		FLAG_N = NFLAG_64(res);
		FLAG_V = VFLAG_CLEAR;
		REG_D[word2 & 7] = (res >> 32);
		REG_D[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		return;
	}
	m68ki_exception_illegal();

#else


	m68ki_exception_illegal();

#endif
}


void m68k_op_mull_32_pi(void)
{
#if M68K_USE_64_BIT

	if(0)
	{
		uint word2 = OPER_I_16();
		uint64 src = OPER_AY_PI_32();
		uint64 dst = REG_D[(word2 >> 12) & 7];
		uint64 res;

		FLAG_C = CFLAG_CLEAR;

		if(BIT_B(word2))			   /* signed */
		{
			res = (sint64)((sint32)src) * (sint64)((sint32)dst);
			if(!BIT_A(word2))
			{
				FLAG_Z = MASK_OUT_ABOVE_32(res);
				FLAG_N = NFLAG_32(res);
				FLAG_V = ((sint64)res != (sint32)res)<<7;
				REG_D[(word2 >> 12) & 7] = FLAG_Z;
				return;
			}
			FLAG_Z = MASK_OUT_ABOVE_32(res) | (res>>32);
			FLAG_N = NFLAG_64(res);
			FLAG_V = VFLAG_CLEAR;
			REG_D[word2 & 7] = (res >> 32);
			REG_D[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
			return;
		}

		res = src * dst;
		if(!BIT_A(word2))
		{
			FLAG_Z = MASK_OUT_ABOVE_32(res);
			FLAG_N = NFLAG_32(res);
			FLAG_V = (res > 0xffffffff)<<7;
			REG_D[(word2 >> 12) & 7] = FLAG_Z;
			return;
		}
		FLAG_Z = MASK_OUT_ABOVE_32(res) | (res>>32);
		FLAG_N = NFLAG_64(res);
		FLAG_V = VFLAG_CLEAR;
		REG_D[word2 & 7] = (res >> 32);
		REG_D[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		return;
	}
	m68ki_exception_illegal();

#else

	if(0)
	{
		uint word2 = OPER_I_16();
		uint src = OPER_AY_PI_32();
		uint dst = REG_D[(word2 >> 12) & 7];
		uint neg = GET_MSB_32(src ^ dst);
		uint src1;
		uint src2;
		uint dst1;
		uint dst2;
		uint r1;
		uint r2;
		uint r3;
		uint r4;
		uint lo;
		uint hi;

		FLAG_C = CFLAG_CLEAR;

		if(BIT_B(word2))			   /* signed */
		{
			if(GET_MSB_32(src))
				src = (uint)MASK_OUT_ABOVE_32(-(sint)src);
			if(GET_MSB_32(dst))
				dst = (uint)MASK_OUT_ABOVE_32(-(sint)dst);
		}

		src1 = MASK_OUT_ABOVE_16(src);
		src2 = src>>16;
		dst1 = MASK_OUT_ABOVE_16(dst);
		dst2 = dst>>16;


		r1 = src1 * dst1;
		r2 = src1 * dst2;
		r3 = src2 * dst1;
		r4 = src2 * dst2;

		lo = r1 + (MASK_OUT_ABOVE_16(r2)<<16) + (MASK_OUT_ABOVE_16(r3)<<16);
		hi = r4 + (r2>>16) + (r3>>16) + (((r1>>16) + MASK_OUT_ABOVE_16(r2) + MASK_OUT_ABOVE_16(r3)) >> 16);

		if(BIT_B(word2) && neg)
		{
			hi = (uint)MASK_OUT_ABOVE_32((-(sint)hi) - (lo != 0));
			lo = (uint)MASK_OUT_ABOVE_32(-(sint)lo);
		}

		if(BIT_A(word2))
		{
			REG_D[word2 & 7] = hi;
			REG_D[(word2 >> 12) & 7] = lo;
			FLAG_N = NFLAG_32(hi);
			FLAG_Z = hi | lo;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		REG_D[(word2 >> 12) & 7] = lo;
		FLAG_N = NFLAG_32(lo);
		FLAG_Z = lo;
		if(BIT_B(word2))
			FLAG_V = (!((GET_MSB_32(lo) && hi == 0xffffffff) || (!GET_MSB_32(lo) && !hi)))<<7;
		else
			FLAG_V = (hi != 0) << 7;
		return;
	}
	m68ki_exception_illegal();

#endif
}


void m68k_op_mull_32_pd(void)
{
#if M68K_USE_64_BIT

	if(0)
	{
		uint word2 = OPER_I_16();
		uint64 src = OPER_AY_PD_32();
		uint64 dst = REG_D[(word2 >> 12) & 7];
		uint64 res;

		FLAG_C = CFLAG_CLEAR;

		if(BIT_B(word2))			   /* signed */
		{
			res = (sint64)((sint32)src) * (sint64)((sint32)dst);
			if(!BIT_A(word2))
			{
				FLAG_Z = MASK_OUT_ABOVE_32(res);
				FLAG_N = NFLAG_32(res);
				FLAG_V = ((sint64)res != (sint32)res)<<7;
				REG_D[(word2 >> 12) & 7] = FLAG_Z;
				return;
			}
			FLAG_Z = MASK_OUT_ABOVE_32(res) | (res>>32);
			FLAG_N = NFLAG_64(res);
			FLAG_V = VFLAG_CLEAR;
			REG_D[word2 & 7] = (res >> 32);
			REG_D[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
			return;
		}

		res = src * dst;
		if(!BIT_A(word2))
		{
			FLAG_Z = MASK_OUT_ABOVE_32(res);
			FLAG_N = NFLAG_32(res);
			FLAG_V = (res > 0xffffffff)<<7;
			REG_D[(word2 >> 12) & 7] = FLAG_Z;
			return;
		}
		FLAG_Z = MASK_OUT_ABOVE_32(res) | (res>>32);
		FLAG_N = NFLAG_64(res);
		FLAG_V = VFLAG_CLEAR;
		REG_D[word2 & 7] = (res >> 32);
		REG_D[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		return;
	}
	m68ki_exception_illegal();

#else


	m68ki_exception_illegal();

#endif
}


void m68k_op_mull_32_di(void)
{
#if M68K_USE_64_BIT

	if(0)
	{
		uint word2 = OPER_I_16();
		uint64 src = OPER_AY_DI_32();
		uint64 dst = REG_D[(word2 >> 12) & 7];
		uint64 res;

		FLAG_C = CFLAG_CLEAR;

		if(BIT_B(word2))			   /* signed */
		{
			res = (sint64)((sint32)src) * (sint64)((sint32)dst);
			if(!BIT_A(word2))
			{
				FLAG_Z = MASK_OUT_ABOVE_32(res);
				FLAG_N = NFLAG_32(res);
				FLAG_V = ((sint64)res != (sint32)res)<<7;
				REG_D[(word2 >> 12) & 7] = FLAG_Z;
				return;
			}
			FLAG_Z = MASK_OUT_ABOVE_32(res) | (res>>32);
			FLAG_N = NFLAG_64(res);
			FLAG_V = VFLAG_CLEAR;
			REG_D[word2 & 7] = (res >> 32);
			REG_D[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
			return;
		}

		res = src * dst;
		if(!BIT_A(word2))
		{
			FLAG_Z = MASK_OUT_ABOVE_32(res);
			FLAG_N = NFLAG_32(res);
			FLAG_V = (res > 0xffffffff)<<7;
			REG_D[(word2 >> 12) & 7] = FLAG_Z;
			return;
		}
		FLAG_Z = MASK_OUT_ABOVE_32(res) | (res>>32);
		FLAG_N = NFLAG_64(res);
		FLAG_V = VFLAG_CLEAR;
		REG_D[word2 & 7] = (res >> 32);
		REG_D[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		return;
	}
	m68ki_exception_illegal();

#else


	m68ki_exception_illegal();

#endif
}


void m68k_op_mull_32_ix(void)
{
#if M68K_USE_64_BIT

	if(0)
	{
		uint word2 = OPER_I_16();
		uint64 src = OPER_AY_IX_32();
		uint64 dst = REG_D[(word2 >> 12) & 7];
		uint64 res;

		FLAG_C = CFLAG_CLEAR;

		if(BIT_B(word2))			   /* signed */
		{
			res = (sint64)((sint32)src) * (sint64)((sint32)dst);
			if(!BIT_A(word2))
			{
				FLAG_Z = MASK_OUT_ABOVE_32(res);
				FLAG_N = NFLAG_32(res);
				FLAG_V = ((sint64)res != (sint32)res)<<7;
				REG_D[(word2 >> 12) & 7] = FLAG_Z;
				return;
			}
			FLAG_Z = MASK_OUT_ABOVE_32(res) | (res>>32);
			FLAG_N = NFLAG_64(res);
			FLAG_V = VFLAG_CLEAR;
			REG_D[word2 & 7] = (res >> 32);
			REG_D[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
			return;
		}

		res = src * dst;
		if(!BIT_A(word2))
		{
			FLAG_Z = MASK_OUT_ABOVE_32(res);
			FLAG_N = NFLAG_32(res);
			FLAG_V = (res > 0xffffffff)<<7;
			REG_D[(word2 >> 12) & 7] = FLAG_Z;
			return;
		}
		FLAG_Z = MASK_OUT_ABOVE_32(res) | (res>>32);
		FLAG_N = NFLAG_64(res);
		FLAG_V = VFLAG_CLEAR;
		REG_D[word2 & 7] = (res >> 32);
		REG_D[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		return;
	}
	m68ki_exception_illegal();

#else


	m68ki_exception_illegal();

#endif
}


void m68k_op_mull_32_aw(void)
{
#if M68K_USE_64_BIT

	if(0)
	{
		uint word2 = OPER_I_16();
		uint64 src = OPER_AW_32();
		uint64 dst = REG_D[(word2 >> 12) & 7];
		uint64 res;

		FLAG_C = CFLAG_CLEAR;

		if(BIT_B(word2))			   /* signed */
		{
			res = (sint64)((sint32)src) * (sint64)((sint32)dst);
			if(!BIT_A(word2))
			{
				FLAG_Z = MASK_OUT_ABOVE_32(res);
				FLAG_N = NFLAG_32(res);
				FLAG_V = ((sint64)res != (sint32)res)<<7;
				REG_D[(word2 >> 12) & 7] = FLAG_Z;
				return;
			}
			FLAG_Z = MASK_OUT_ABOVE_32(res) | (res>>32);
			FLAG_N = NFLAG_64(res);
			FLAG_V = VFLAG_CLEAR;
			REG_D[word2 & 7] = (res >> 32);
			REG_D[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
			return;
		}

		res = src * dst;
		if(!BIT_A(word2))
		{
			FLAG_Z = MASK_OUT_ABOVE_32(res);
			FLAG_N = NFLAG_32(res);
			FLAG_V = (res > 0xffffffff)<<7;
			REG_D[(word2 >> 12) & 7] = FLAG_Z;
			return;
		}
		FLAG_Z = MASK_OUT_ABOVE_32(res) | (res>>32);
		FLAG_N = NFLAG_64(res);
		FLAG_V = VFLAG_CLEAR;
		REG_D[word2 & 7] = (res >> 32);
		REG_D[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		return;
	}
	m68ki_exception_illegal();

#else


	m68ki_exception_illegal();

#endif
}


void m68k_op_mull_32_al(void)
{
#if M68K_USE_64_BIT

	if(0)
	{
		uint word2 = OPER_I_16();
		uint64 src = OPER_AL_32();
		uint64 dst = REG_D[(word2 >> 12) & 7];
		uint64 res;

		FLAG_C = CFLAG_CLEAR;

		if(BIT_B(word2))			   /* signed */
		{
			res = (sint64)((sint32)src) * (sint64)((sint32)dst);
			if(!BIT_A(word2))
			{
				FLAG_Z = MASK_OUT_ABOVE_32(res);
				FLAG_N = NFLAG_32(res);
				FLAG_V = ((sint64)res != (sint32)res)<<7;
				REG_D[(word2 >> 12) & 7] = FLAG_Z;
				return;
			}
			FLAG_Z = MASK_OUT_ABOVE_32(res) | (res>>32);
			FLAG_N = NFLAG_64(res);
			FLAG_V = VFLAG_CLEAR;
			REG_D[word2 & 7] = (res >> 32);
			REG_D[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
			return;
		}

		res = src * dst;
		if(!BIT_A(word2))
		{
			FLAG_Z = MASK_OUT_ABOVE_32(res);
			FLAG_N = NFLAG_32(res);
			FLAG_V = (res > 0xffffffff)<<7;
			REG_D[(word2 >> 12) & 7] = FLAG_Z;
			return;
		}
		FLAG_Z = MASK_OUT_ABOVE_32(res) | (res>>32);
		FLAG_N = NFLAG_64(res);
		FLAG_V = VFLAG_CLEAR;
		REG_D[word2 & 7] = (res >> 32);
		REG_D[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		return;
	}
	m68ki_exception_illegal();

#else


	m68ki_exception_illegal();

#endif
}


void m68k_op_mull_32_pcdi(void)
{
#if M68K_USE_64_BIT

	if(0)
	{
		uint word2 = OPER_I_16();
		uint64 src = OPER_PCDI_32();
		uint64 dst = REG_D[(word2 >> 12) & 7];
		uint64 res;

		FLAG_C = CFLAG_CLEAR;

		if(BIT_B(word2))			   /* signed */
		{
			res = (sint64)((sint32)src) * (sint64)((sint32)dst);
			if(!BIT_A(word2))
			{
				FLAG_Z = MASK_OUT_ABOVE_32(res);
				FLAG_N = NFLAG_32(res);
				FLAG_V = ((sint64)res != (sint32)res)<<7;
				REG_D[(word2 >> 12) & 7] = FLAG_Z;
				return;
			}
			FLAG_Z = MASK_OUT_ABOVE_32(res) | (res>>32);
			FLAG_N = NFLAG_64(res);
			FLAG_V = VFLAG_CLEAR;
			REG_D[word2 & 7] = (res >> 32);
			REG_D[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
			return;
		}

		res = src * dst;
		if(!BIT_A(word2))
		{
			FLAG_Z = MASK_OUT_ABOVE_32(res);
			FLAG_N = NFLAG_32(res);
			FLAG_V = (res > 0xffffffff)<<7;
			REG_D[(word2 >> 12) & 7] = FLAG_Z;
			return;
		}
		FLAG_Z = MASK_OUT_ABOVE_32(res) | (res>>32);
		FLAG_N = NFLAG_64(res);
		FLAG_V = VFLAG_CLEAR;
		REG_D[word2 & 7] = (res >> 32);
		REG_D[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		return;
	}
	m68ki_exception_illegal();

#else


	m68ki_exception_illegal();

#endif
}


void m68k_op_mull_32_pcix(void)
{
#if M68K_USE_64_BIT

	if(0)
	{
		uint word2 = OPER_I_16();
		uint64 src = OPER_PCIX_32();
		uint64 dst = REG_D[(word2 >> 12) & 7];
		uint64 res;

		FLAG_C = CFLAG_CLEAR;

		if(BIT_B(word2))			   /* signed */
		{
			res = (sint64)((sint32)src) * (sint64)((sint32)dst);
			if(!BIT_A(word2))
			{
				FLAG_Z = MASK_OUT_ABOVE_32(res);
				FLAG_N = NFLAG_32(res);
				FLAG_V = ((sint64)res != (sint32)res)<<7;
				REG_D[(word2 >> 12) & 7] = FLAG_Z;
				return;
			}
			FLAG_Z = MASK_OUT_ABOVE_32(res) | (res>>32);
			FLAG_N = NFLAG_64(res);
			FLAG_V = VFLAG_CLEAR;
			REG_D[word2 & 7] = (res >> 32);
			REG_D[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
			return;
		}

		res = src * dst;
		if(!BIT_A(word2))
		{
			FLAG_Z = MASK_OUT_ABOVE_32(res);
			FLAG_N = NFLAG_32(res);
			FLAG_V = (res > 0xffffffff)<<7;
			REG_D[(word2 >> 12) & 7] = FLAG_Z;
			return;
		}
		FLAG_Z = MASK_OUT_ABOVE_32(res) | (res>>32);
		FLAG_N = NFLAG_64(res);
		FLAG_V = VFLAG_CLEAR;
		REG_D[word2 & 7] = (res >> 32);
		REG_D[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		return;
	}
	m68ki_exception_illegal();

#else


	m68ki_exception_illegal();

#endif
}


void m68k_op_mull_32_i(void)
{
#if M68K_USE_64_BIT

	if(0)
	{
		uint word2 = OPER_I_16();
		uint64 src = OPER_I_32();
		uint64 dst = REG_D[(word2 >> 12) & 7];
		uint64 res;

		FLAG_C = CFLAG_CLEAR;

		if(BIT_B(word2))			   /* signed */
		{
			res = (sint64)((sint32)src) * (sint64)((sint32)dst);
			if(!BIT_A(word2))
			{
				FLAG_Z = MASK_OUT_ABOVE_32(res);
				FLAG_N = NFLAG_32(res);
				FLAG_V = ((sint64)res != (sint32)res)<<7;
				REG_D[(word2 >> 12) & 7] = FLAG_Z;
				return;
			}
			FLAG_Z = MASK_OUT_ABOVE_32(res) | (res>>32);
			FLAG_N = NFLAG_64(res);
			FLAG_V = VFLAG_CLEAR;
			REG_D[word2 & 7] = (res >> 32);
			REG_D[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
			return;
		}

		res = src * dst;
		if(!BIT_A(word2))
		{
			FLAG_Z = MASK_OUT_ABOVE_32(res);
			FLAG_N = NFLAG_32(res);
			FLAG_V = (res > 0xffffffff)<<7;
			REG_D[(word2 >> 12) & 7] = FLAG_Z;
			return;
		}
		FLAG_Z = MASK_OUT_ABOVE_32(res) | (res>>32);
		FLAG_N = NFLAG_64(res);
		FLAG_V = VFLAG_CLEAR;
		REG_D[word2 & 7] = (res >> 32);
		REG_D[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		return;
	}
	m68ki_exception_illegal();

#else


	m68ki_exception_illegal();

#endif
}


void m68k_op_nbcd_8_d(void)
{
	uint* r_dst = &DY;
	uint dst = *r_dst;
	uint res = MASK_OUT_ABOVE_8(0x9a - dst - XFLAG_AS_1());

	if(res != 0x9a)
	{
		FLAG_V = ~res; /* Undefined V behavior */

		if((res & 0x0f) == 0xa)
			res = (res & 0xf0) + 0x10;

		res = MASK_OUT_ABOVE_8(res);

		FLAG_V &= res; /* Undefined V behavior part II */

		*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

		FLAG_Z |= res;
		FLAG_C = CFLAG_SET;
		FLAG_X = XFLAG_SET;
	}
	else
	{
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;
		FLAG_X = XFLAG_CLEAR;
	}
	FLAG_N = NFLAG_8(res);	/* Undefined N behavior */
}


void m68k_op_nbcd_8_ai(void)
{
	uint ea = EA_AY_AI_8();
	uint dst = m68ki_read_8(ea);
	uint res = MASK_OUT_ABOVE_8(0x9a - dst - XFLAG_AS_1());

	if(res != 0x9a)
	{
		FLAG_V = ~res; /* Undefined V behavior */

		if((res & 0x0f) == 0xa)
			res = (res & 0xf0) + 0x10;

		res = MASK_OUT_ABOVE_8(res);

		FLAG_V &= res; /* Undefined V behavior part II */

		m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

		FLAG_Z |= res;
		FLAG_C = CFLAG_SET;
		FLAG_X = XFLAG_SET;
	}
	else
	{
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;
		FLAG_X = XFLAG_CLEAR;
	}
	FLAG_N = NFLAG_8(res);	/* Undefined N behavior */
}


void m68k_op_nbcd_8_pi(void)
{
	uint ea = EA_AY_PI_8();
	uint dst = m68ki_read_8(ea);
	uint res = MASK_OUT_ABOVE_8(0x9a - dst - XFLAG_AS_1());

	if(res != 0x9a)
	{
		FLAG_V = ~res; /* Undefined V behavior */

		if((res & 0x0f) == 0xa)
			res = (res & 0xf0) + 0x10;

		res = MASK_OUT_ABOVE_8(res);

		FLAG_V &= res; /* Undefined V behavior part II */

		m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

		FLAG_Z |= res;
		FLAG_C = CFLAG_SET;
		FLAG_X = XFLAG_SET;
	}
	else
	{
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;
		FLAG_X = XFLAG_CLEAR;
	}
	FLAG_N = NFLAG_8(res);	/* Undefined N behavior */
}


void m68k_op_nbcd_8_pi7(void)
{
	uint ea = EA_A7_PI_8();
	uint dst = m68ki_read_8(ea);
	uint res = MASK_OUT_ABOVE_8(0x9a - dst - XFLAG_AS_1());

	if(res != 0x9a)
	{
		FLAG_V = ~res; /* Undefined V behavior */

		if((res & 0x0f) == 0xa)
			res = (res & 0xf0) + 0x10;

		res = MASK_OUT_ABOVE_8(res);

		FLAG_V &= res; /* Undefined V behavior part II */

		m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

		FLAG_Z |= res;
		FLAG_C = CFLAG_SET;
		FLAG_X = XFLAG_SET;
	}
	else
	{
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;
		FLAG_X = XFLAG_CLEAR;
	}
	FLAG_N = NFLAG_8(res);	/* Undefined N behavior */
}


void m68k_op_nbcd_8_pd(void)
{
	uint ea = EA_AY_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = MASK_OUT_ABOVE_8(0x9a - dst - XFLAG_AS_1());

	if(res != 0x9a)
	{
		FLAG_V = ~res; /* Undefined V behavior */

		if((res & 0x0f) == 0xa)
			res = (res & 0xf0) + 0x10;

		res = MASK_OUT_ABOVE_8(res);

		FLAG_V &= res; /* Undefined V behavior part II */

		m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

		FLAG_Z |= res;
		FLAG_C = CFLAG_SET;
		FLAG_X = XFLAG_SET;
	}
	else
	{
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;
		FLAG_X = XFLAG_CLEAR;
	}
	FLAG_N = NFLAG_8(res);	/* Undefined N behavior */
}


void m68k_op_nbcd_8_pd7(void)
{
	uint ea = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = MASK_OUT_ABOVE_8(0x9a - dst - XFLAG_AS_1());

	if(res != 0x9a)
	{
		FLAG_V = ~res; /* Undefined V behavior */

		if((res & 0x0f) == 0xa)
			res = (res & 0xf0) + 0x10;

		res = MASK_OUT_ABOVE_8(res);

		FLAG_V &= res; /* Undefined V behavior part II */

		m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

		FLAG_Z |= res;
		FLAG_C = CFLAG_SET;
		FLAG_X = XFLAG_SET;
	}
	else
	{
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;
		FLAG_X = XFLAG_CLEAR;
	}
	FLAG_N = NFLAG_8(res);	/* Undefined N behavior */
}


void m68k_op_nbcd_8_di(void)
{
	uint ea = EA_AY_DI_8();
	uint dst = m68ki_read_8(ea);
	uint res = MASK_OUT_ABOVE_8(0x9a - dst - XFLAG_AS_1());

	if(res != 0x9a)
	{
		FLAG_V = ~res; /* Undefined V behavior */

		if((res & 0x0f) == 0xa)
			res = (res & 0xf0) + 0x10;

		res = MASK_OUT_ABOVE_8(res);

		FLAG_V &= res; /* Undefined V behavior part II */

		m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

		FLAG_Z |= res;
		FLAG_C = CFLAG_SET;
		FLAG_X = XFLAG_SET;
	}
	else
	{
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;
		FLAG_X = XFLAG_CLEAR;
	}
	FLAG_N = NFLAG_8(res);	/* Undefined N behavior */
}


void m68k_op_nbcd_8_ix(void)
{
	uint ea = EA_AY_IX_8();
	uint dst = m68ki_read_8(ea);
	uint res = MASK_OUT_ABOVE_8(0x9a - dst - XFLAG_AS_1());

	if(res != 0x9a)
	{
		FLAG_V = ~res; /* Undefined V behavior */

		if((res & 0x0f) == 0xa)
			res = (res & 0xf0) + 0x10;

		res = MASK_OUT_ABOVE_8(res);

		FLAG_V &= res; /* Undefined V behavior part II */

		m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

		FLAG_Z |= res;
		FLAG_C = CFLAG_SET;
		FLAG_X = XFLAG_SET;
	}
	else
	{
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;
		FLAG_X = XFLAG_CLEAR;
	}
	FLAG_N = NFLAG_8(res);	/* Undefined N behavior */
}


void m68k_op_nbcd_8_aw(void)
{
	uint ea = EA_AW_8();
	uint dst = m68ki_read_8(ea);
	uint res = MASK_OUT_ABOVE_8(0x9a - dst - XFLAG_AS_1());

	if(res != 0x9a)
	{
		FLAG_V = ~res; /* Undefined V behavior */

		if((res & 0x0f) == 0xa)
			res = (res & 0xf0) + 0x10;

		res = MASK_OUT_ABOVE_8(res);

		FLAG_V &= res; /* Undefined V behavior part II */

		m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

		FLAG_Z |= res;
		FLAG_C = CFLAG_SET;
		FLAG_X = XFLAG_SET;
	}
	else
	{
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;
		FLAG_X = XFLAG_CLEAR;
	}
	FLAG_N = NFLAG_8(res);	/* Undefined N behavior */
}


void m68k_op_nbcd_8_al(void)
{
	uint ea = EA_AL_8();
	uint dst = m68ki_read_8(ea);
	uint res = MASK_OUT_ABOVE_8(0x9a - dst - XFLAG_AS_1());

	if(res != 0x9a)
	{
		FLAG_V = ~res; /* Undefined V behavior */

		if((res & 0x0f) == 0xa)
			res = (res & 0xf0) + 0x10;

		res = MASK_OUT_ABOVE_8(res);

		FLAG_V &= res; /* Undefined V behavior part II */

		m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

		FLAG_Z |= res;
		FLAG_C = CFLAG_SET;
		FLAG_X = XFLAG_SET;
	}
	else
	{
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;
		FLAG_X = XFLAG_CLEAR;
	}
	FLAG_N = NFLAG_8(res);	/* Undefined N behavior */
}


void m68k_op_neg_8_d(void)
{
	uint* r_dst = &DY;
	uint res = 0 - MASK_OUT_ABOVE_8(*r_dst);

	FLAG_N = NFLAG_8(res);
	FLAG_C = FLAG_X = CFLAG_8(res);
	FLAG_V = *r_dst & res;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_neg_8_ai(void)
{
	uint ea = EA_AY_AI_8();
	uint src = m68ki_read_8(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_8(res);
	FLAG_C = FLAG_X = CFLAG_8(res);
	FLAG_V = src & res;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_neg_8_pi(void)
{
	uint ea = EA_AY_PI_8();
	uint src = m68ki_read_8(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_8(res);
	FLAG_C = FLAG_X = CFLAG_8(res);
	FLAG_V = src & res;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_neg_8_pi7(void)
{
	uint ea = EA_A7_PI_8();
	uint src = m68ki_read_8(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_8(res);
	FLAG_C = FLAG_X = CFLAG_8(res);
	FLAG_V = src & res;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_neg_8_pd(void)
{
	uint ea = EA_AY_PD_8();
	uint src = m68ki_read_8(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_8(res);
	FLAG_C = FLAG_X = CFLAG_8(res);
	FLAG_V = src & res;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_neg_8_pd7(void)
{
	uint ea = EA_A7_PD_8();
	uint src = m68ki_read_8(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_8(res);
	FLAG_C = FLAG_X = CFLAG_8(res);
	FLAG_V = src & res;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_neg_8_di(void)
{
	uint ea = EA_AY_DI_8();
	uint src = m68ki_read_8(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_8(res);
	FLAG_C = FLAG_X = CFLAG_8(res);
	FLAG_V = src & res;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_neg_8_ix(void)
{
	uint ea = EA_AY_IX_8();
	uint src = m68ki_read_8(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_8(res);
	FLAG_C = FLAG_X = CFLAG_8(res);
	FLAG_V = src & res;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_neg_8_aw(void)
{
	uint ea = EA_AW_8();
	uint src = m68ki_read_8(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_8(res);
	FLAG_C = FLAG_X = CFLAG_8(res);
	FLAG_V = src & res;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_neg_8_al(void)
{
	uint ea = EA_AL_8();
	uint src = m68ki_read_8(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_8(res);
	FLAG_C = FLAG_X = CFLAG_8(res);
	FLAG_V = src & res;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_neg_16_d(void)
{
	uint* r_dst = &DY;
	uint res = 0 - MASK_OUT_ABOVE_16(*r_dst);

	FLAG_N = NFLAG_16(res);
	FLAG_C = FLAG_X = CFLAG_16(res);
	FLAG_V = (*r_dst & res)>>8;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_neg_16_ai(void)
{
	uint ea = EA_AY_AI_16();
	uint src = m68ki_read_16(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_16(res);
	FLAG_C = FLAG_X = CFLAG_16(res);
	FLAG_V = (src & res)>>8;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_neg_16_pi(void)
{
	uint ea = EA_AY_PI_16();
	uint src = m68ki_read_16(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_16(res);
	FLAG_C = FLAG_X = CFLAG_16(res);
	FLAG_V = (src & res)>>8;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_neg_16_pd(void)
{
	uint ea = EA_AY_PD_16();
	uint src = m68ki_read_16(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_16(res);
	FLAG_C = FLAG_X = CFLAG_16(res);
	FLAG_V = (src & res)>>8;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_neg_16_di(void)
{
	uint ea = EA_AY_DI_16();
	uint src = m68ki_read_16(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_16(res);
	FLAG_C = FLAG_X = CFLAG_16(res);
	FLAG_V = (src & res)>>8;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_neg_16_ix(void)
{
	uint ea = EA_AY_IX_16();
	uint src = m68ki_read_16(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_16(res);
	FLAG_C = FLAG_X = CFLAG_16(res);
	FLAG_V = (src & res)>>8;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_neg_16_aw(void)
{
	uint ea = EA_AW_16();
	uint src = m68ki_read_16(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_16(res);
	FLAG_C = FLAG_X = CFLAG_16(res);
	FLAG_V = (src & res)>>8;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_neg_16_al(void)
{
	uint ea = EA_AL_16();
	uint src = m68ki_read_16(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_16(res);
	FLAG_C = FLAG_X = CFLAG_16(res);
	FLAG_V = (src & res)>>8;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_neg_32_d(void)
{
	uint* r_dst = &DY;
	uint res = 0 - *r_dst;

	FLAG_N = NFLAG_32(res);
	FLAG_C = FLAG_X = CFLAG_SUB_32(*r_dst, 0, res);
	FLAG_V = (*r_dst & res)>>24;
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_neg_32_ai(void)
{
	uint ea = EA_AY_AI_32();
	uint src = m68ki_read_32(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_32(res);
	FLAG_C = FLAG_X = CFLAG_SUB_32(src, 0, res);
	FLAG_V = (src & res)>>24;
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_neg_32_pi(void)
{
	uint ea = EA_AY_PI_32();
	uint src = m68ki_read_32(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_32(res);
	FLAG_C = FLAG_X = CFLAG_SUB_32(src, 0, res);
	FLAG_V = (src & res)>>24;
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_neg_32_pd(void)
{
	uint ea = EA_AY_PD_32();
	uint src = m68ki_read_32(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_32(res);
	FLAG_C = FLAG_X = CFLAG_SUB_32(src, 0, res);
	FLAG_V = (src & res)>>24;
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_neg_32_di(void)
{
	uint ea = EA_AY_DI_32();
	uint src = m68ki_read_32(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_32(res);
	FLAG_C = FLAG_X = CFLAG_SUB_32(src, 0, res);
	FLAG_V = (src & res)>>24;
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_neg_32_ix(void)
{
	uint ea = EA_AY_IX_32();
	uint src = m68ki_read_32(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_32(res);
	FLAG_C = FLAG_X = CFLAG_SUB_32(src, 0, res);
	FLAG_V = (src & res)>>24;
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_neg_32_aw(void)
{
	uint ea = EA_AW_32();
	uint src = m68ki_read_32(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_32(res);
	FLAG_C = FLAG_X = CFLAG_SUB_32(src, 0, res);
	FLAG_V = (src & res)>>24;
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_neg_32_al(void)
{
	uint ea = EA_AL_32();
	uint src = m68ki_read_32(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_32(res);
	FLAG_C = FLAG_X = CFLAG_SUB_32(src, 0, res);
	FLAG_V = (src & res)>>24;
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_negx_8_d(void)
{
	uint* r_dst = &DY;
	uint res = 0 - MASK_OUT_ABOVE_8(*r_dst) - XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = *r_dst & res;

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
}


void m68k_op_negx_8_ai(void)
{
	uint ea = EA_AY_AI_8();
	uint src = m68ki_read_8(ea);
	uint res = 0 - src - XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = src & res;

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_negx_8_pi(void)
{
	uint ea = EA_AY_PI_8();
	uint src = m68ki_read_8(ea);
	uint res = 0 - src - XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = src & res;

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_negx_8_pi7(void)
{
	uint ea = EA_A7_PI_8();
	uint src = m68ki_read_8(ea);
	uint res = 0 - src - XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = src & res;

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_negx_8_pd(void)
{
	uint ea = EA_AY_PD_8();
	uint src = m68ki_read_8(ea);
	uint res = 0 - src - XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = src & res;

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_negx_8_pd7(void)
{
	uint ea = EA_A7_PD_8();
	uint src = m68ki_read_8(ea);
	uint res = 0 - src - XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = src & res;

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_negx_8_di(void)
{
	uint ea = EA_AY_DI_8();
	uint src = m68ki_read_8(ea);
	uint res = 0 - src - XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = src & res;

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_negx_8_ix(void)
{
	uint ea = EA_AY_IX_8();
	uint src = m68ki_read_8(ea);
	uint res = 0 - src - XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = src & res;

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_negx_8_aw(void)
{
	uint ea = EA_AW_8();
	uint src = m68ki_read_8(ea);
	uint res = 0 - src - XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = src & res;

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_negx_8_al(void)
{
	uint ea = EA_AL_8();
	uint src = m68ki_read_8(ea);
	uint res = 0 - src - XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = src & res;

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_negx_16_d(void)
{
	uint* r_dst = &DY;
	uint res = 0 - MASK_OUT_ABOVE_16(*r_dst) - XFLAG_AS_1();

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = (*r_dst & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	FLAG_Z |= res;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
}


void m68k_op_negx_16_ai(void)
{
	uint ea  = EA_AY_AI_16();
	uint src = m68ki_read_16(ea);
	uint res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_AS_1();

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = (src & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	FLAG_Z |= res;

	m68ki_write_16(ea, res);
}


void m68k_op_negx_16_pi(void)
{
	uint ea  = EA_AY_PI_16();
	uint src = m68ki_read_16(ea);
	uint res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_AS_1();

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = (src & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	FLAG_Z |= res;

	m68ki_write_16(ea, res);
}


void m68k_op_negx_16_pd(void)
{
	uint ea  = EA_AY_PD_16();
	uint src = m68ki_read_16(ea);
	uint res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_AS_1();

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = (src & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	FLAG_Z |= res;

	m68ki_write_16(ea, res);
}


void m68k_op_negx_16_di(void)
{
	uint ea  = EA_AY_DI_16();
	uint src = m68ki_read_16(ea);
	uint res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_AS_1();

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = (src & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	FLAG_Z |= res;

	m68ki_write_16(ea, res);
}


void m68k_op_negx_16_ix(void)
{
	uint ea  = EA_AY_IX_16();
	uint src = m68ki_read_16(ea);
	uint res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_AS_1();

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = (src & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	FLAG_Z |= res;

	m68ki_write_16(ea, res);
}


void m68k_op_negx_16_aw(void)
{
	uint ea  = EA_AW_16();
	uint src = m68ki_read_16(ea);
	uint res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_AS_1();

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = (src & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	FLAG_Z |= res;

	m68ki_write_16(ea, res);
}


void m68k_op_negx_16_al(void)
{
	uint ea  = EA_AL_16();
	uint src = m68ki_read_16(ea);
	uint res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_AS_1();

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = (src & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	FLAG_Z |= res;

	m68ki_write_16(ea, res);
}


void m68k_op_negx_32_d(void)
{
	uint* r_dst = &DY;
	uint res = 0 - MASK_OUT_ABOVE_32(*r_dst) - XFLAG_AS_1();

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(*r_dst, 0, res);
	FLAG_V = (*r_dst & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	FLAG_Z |= res;

	*r_dst = res;
}


void m68k_op_negx_32_ai(void)
{
	uint ea  = EA_AY_AI_32();
	uint src = m68ki_read_32(ea);
	uint res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_AS_1();

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, 0, res);
	FLAG_V = (src & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	FLAG_Z |= res;

	m68ki_write_32(ea, res);
}


void m68k_op_negx_32_pi(void)
{
	uint ea  = EA_AY_PI_32();
	uint src = m68ki_read_32(ea);
	uint res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_AS_1();

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, 0, res);
	FLAG_V = (src & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	FLAG_Z |= res;

	m68ki_write_32(ea, res);
}


void m68k_op_negx_32_pd(void)
{
	uint ea  = EA_AY_PD_32();
	uint src = m68ki_read_32(ea);
	uint res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_AS_1();

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, 0, res);
	FLAG_V = (src & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	FLAG_Z |= res;

	m68ki_write_32(ea, res);
}


void m68k_op_negx_32_di(void)
{
	uint ea  = EA_AY_DI_32();
	uint src = m68ki_read_32(ea);
	uint res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_AS_1();

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, 0, res);
	FLAG_V = (src & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	FLAG_Z |= res;

	m68ki_write_32(ea, res);
}


void m68k_op_negx_32_ix(void)
{
	uint ea  = EA_AY_IX_32();
	uint src = m68ki_read_32(ea);
	uint res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_AS_1();

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, 0, res);
	FLAG_V = (src & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	FLAG_Z |= res;

	m68ki_write_32(ea, res);
}


void m68k_op_negx_32_aw(void)
{
	uint ea  = EA_AW_32();
	uint src = m68ki_read_32(ea);
	uint res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_AS_1();

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, 0, res);
	FLAG_V = (src & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	FLAG_Z |= res;

	m68ki_write_32(ea, res);
}


void m68k_op_negx_32_al(void)
{
	uint ea  = EA_AL_32();
	uint src = m68ki_read_32(ea);
	uint res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_AS_1();

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, 0, res);
	FLAG_V = (src & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	FLAG_Z |= res;

	m68ki_write_32(ea, res);
}


void m68k_op_nop(void)
{
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
}


void m68k_op_not_8_d(void)
{
	uint* r_dst = &DY;
	uint res = MASK_OUT_ABOVE_8(~*r_dst);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_8_ai(void)
{
	uint ea = EA_AY_AI_8();
	uint res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_8_pi(void)
{
	uint ea = EA_AY_PI_8();
	uint res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_8_pi7(void)
{
	uint ea = EA_A7_PI_8();
	uint res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_8_pd(void)
{
	uint ea = EA_AY_PD_8();
	uint res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_8_pd7(void)
{
	uint ea = EA_A7_PD_8();
	uint res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_8_di(void)
{
	uint ea = EA_AY_DI_8();
	uint res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_8_ix(void)
{
	uint ea = EA_AY_IX_8();
	uint res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_8_aw(void)
{
	uint ea = EA_AW_8();
	uint res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_8_al(void)
{
	uint ea = EA_AL_8();
	uint res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_16_d(void)
{
	uint* r_dst = &DY;
	uint res = MASK_OUT_ABOVE_16(~*r_dst);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_16_ai(void)
{
	uint ea = EA_AY_AI_16();
	uint res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_16_pi(void)
{
	uint ea = EA_AY_PI_16();
	uint res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_16_pd(void)
{
	uint ea = EA_AY_PD_16();
	uint res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_16_di(void)
{
	uint ea = EA_AY_DI_16();
	uint res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_16_ix(void)
{
	uint ea = EA_AY_IX_16();
	uint res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_16_aw(void)
{
	uint ea = EA_AW_16();
	uint res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_16_al(void)
{
	uint ea = EA_AL_16();
	uint res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_32_d(void)
{
	uint* r_dst = &DY;
	uint res = *r_dst = MASK_OUT_ABOVE_32(~*r_dst);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_32_ai(void)
{
	uint ea = EA_AY_AI_32();
	uint res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_32_pi(void)
{
	uint ea = EA_AY_PI_32();
	uint res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_32_pd(void)
{
	uint ea = EA_AY_PD_32();
	uint res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_32_di(void)
{
	uint ea = EA_AY_DI_32();
	uint res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_32_ix(void)
{
	uint ea = EA_AY_IX_32();
	uint res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_32_aw(void)
{
	uint ea = EA_AW_32();
	uint res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_not_32_al(void)
{
	uint ea = EA_AL_32();
	uint res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_8_er_d(void)
{
	uint res = MASK_OUT_ABOVE_8((DX |= MASK_OUT_ABOVE_8(DY)));

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_8_er_ai(void)
{
	uint res = MASK_OUT_ABOVE_8((DX |= OPER_AY_AI_8()));

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_8_er_pi(void)
{
	uint res = MASK_OUT_ABOVE_8((DX |= OPER_AY_PI_8()));

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_8_er_pi7(void)
{
	uint res = MASK_OUT_ABOVE_8((DX |= OPER_A7_PI_8()));

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_8_er_pd(void)
{
	uint res = MASK_OUT_ABOVE_8((DX |= OPER_AY_PD_8()));

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_8_er_pd7(void)
{
	uint res = MASK_OUT_ABOVE_8((DX |= OPER_A7_PD_8()));

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_8_er_di(void)
{
	uint res = MASK_OUT_ABOVE_8((DX |= OPER_AY_DI_8()));

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_8_er_ix(void)
{
	uint res = MASK_OUT_ABOVE_8((DX |= OPER_AY_IX_8()));

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_8_er_aw(void)
{
	uint res = MASK_OUT_ABOVE_8((DX |= OPER_AW_8()));

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_8_er_al(void)
{
	uint res = MASK_OUT_ABOVE_8((DX |= OPER_AL_8()));

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_8_er_pcdi(void)
{
	uint res = MASK_OUT_ABOVE_8((DX |= OPER_PCDI_8()));

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_8_er_pcix(void)
{
	uint res = MASK_OUT_ABOVE_8((DX |= OPER_PCIX_8()));

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_8_er_i(void)
{
	uint res = MASK_OUT_ABOVE_8((DX |= OPER_I_8()));

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_16_er_d(void)
{
	uint res = MASK_OUT_ABOVE_16((DX |= MASK_OUT_ABOVE_16(DY)));

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_16_er_ai(void)
{
	uint res = MASK_OUT_ABOVE_16((DX |= OPER_AY_AI_16()));

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_16_er_pi(void)
{
	uint res = MASK_OUT_ABOVE_16((DX |= OPER_AY_PI_16()));

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_16_er_pd(void)
{
	uint res = MASK_OUT_ABOVE_16((DX |= OPER_AY_PD_16()));

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_16_er_di(void)
{
	uint res = MASK_OUT_ABOVE_16((DX |= OPER_AY_DI_16()));

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_16_er_ix(void)
{
	uint res = MASK_OUT_ABOVE_16((DX |= OPER_AY_IX_16()));

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_16_er_aw(void)
{
	uint res = MASK_OUT_ABOVE_16((DX |= OPER_AW_16()));

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_16_er_al(void)
{
	uint res = MASK_OUT_ABOVE_16((DX |= OPER_AL_16()));

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_16_er_pcdi(void)
{
	uint res = MASK_OUT_ABOVE_16((DX |= OPER_PCDI_16()));

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_16_er_pcix(void)
{
	uint res = MASK_OUT_ABOVE_16((DX |= OPER_PCIX_16()));

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_16_er_i(void)
{
	uint res = MASK_OUT_ABOVE_16((DX |= OPER_I_16()));

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_32_er_d(void)
{
	uint res = DX |= DY;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_32_er_ai(void)
{
	uint res = DX |= OPER_AY_AI_32();

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_32_er_pi(void)
{
	uint res = DX |= OPER_AY_PI_32();

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_32_er_pd(void)
{
	uint res = DX |= OPER_AY_PD_32();

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_32_er_di(void)
{
	uint res = DX |= OPER_AY_DI_32();

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_32_er_ix(void)
{
	uint res = DX |= OPER_AY_IX_32();

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_32_er_aw(void)
{
	uint res = DX |= OPER_AW_32();

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_32_er_al(void)
{
	uint res = DX |= OPER_AL_32();

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_32_er_pcdi(void)
{
	uint res = DX |= OPER_PCDI_32();

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_32_er_pcix(void)
{
	uint res = DX |= OPER_PCIX_32();

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_32_er_i(void)
{
	uint res = DX |= OPER_I_32();

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_8_re_ai(void)
{
	uint ea = EA_AY_AI_8();
	uint res = MASK_OUT_ABOVE_8(DX | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_8_re_pi(void)
{
	uint ea = EA_AY_PI_8();
	uint res = MASK_OUT_ABOVE_8(DX | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_8_re_pi7(void)
{
	uint ea = EA_A7_PI_8();
	uint res = MASK_OUT_ABOVE_8(DX | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_8_re_pd(void)
{
	uint ea = EA_AY_PD_8();
	uint res = MASK_OUT_ABOVE_8(DX | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_8_re_pd7(void)
{
	uint ea = EA_A7_PD_8();
	uint res = MASK_OUT_ABOVE_8(DX | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_8_re_di(void)
{
	uint ea = EA_AY_DI_8();
	uint res = MASK_OUT_ABOVE_8(DX | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_8_re_ix(void)
{
	uint ea = EA_AY_IX_8();
	uint res = MASK_OUT_ABOVE_8(DX | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_8_re_aw(void)
{
	uint ea = EA_AW_8();
	uint res = MASK_OUT_ABOVE_8(DX | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_8_re_al(void)
{
	uint ea = EA_AL_8();
	uint res = MASK_OUT_ABOVE_8(DX | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_16_re_ai(void)
{
	uint ea = EA_AY_AI_16();
	uint res = MASK_OUT_ABOVE_16(DX | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_16_re_pi(void)
{
	uint ea = EA_AY_PI_16();
	uint res = MASK_OUT_ABOVE_16(DX | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_16_re_pd(void)
{
	uint ea = EA_AY_PD_16();
	uint res = MASK_OUT_ABOVE_16(DX | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_16_re_di(void)
{
	uint ea = EA_AY_DI_16();
	uint res = MASK_OUT_ABOVE_16(DX | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_16_re_ix(void)
{
	uint ea = EA_AY_IX_16();
	uint res = MASK_OUT_ABOVE_16(DX | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_16_re_aw(void)
{
	uint ea = EA_AW_16();
	uint res = MASK_OUT_ABOVE_16(DX | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_16_re_al(void)
{
	uint ea = EA_AL_16();
	uint res = MASK_OUT_ABOVE_16(DX | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_32_re_ai(void)
{
	uint ea = EA_AY_AI_32();
	uint res = DX | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_32_re_pi(void)
{
	uint ea = EA_AY_PI_32();
	uint res = DX | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_32_re_pd(void)
{
	uint ea = EA_AY_PD_32();
	uint res = DX | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_32_re_di(void)
{
	uint ea = EA_AY_DI_32();
	uint res = DX | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_32_re_ix(void)
{
	uint ea = EA_AY_IX_32();
	uint res = DX | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_32_re_aw(void)
{
	uint ea = EA_AW_32();
	uint res = DX | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_or_32_re_al(void)
{
	uint ea = EA_AL_32();
	uint res = DX | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_8_d(void)
{
	uint res = MASK_OUT_ABOVE_8((DY |= OPER_I_8()));

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_8_ai(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_AI_8();
	uint res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_8_pi(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_PI_8();
	uint res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_8_pi7(void)
{
	uint src = OPER_I_8();
	uint ea = EA_A7_PI_8();
	uint res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_8_pd(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_PD_8();
	uint res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_8_pd7(void)
{
	uint src = OPER_I_8();
	uint ea = EA_A7_PD_8();
	uint res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_8_di(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_DI_8();
	uint res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_8_ix(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_IX_8();
	uint res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_8_aw(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AW_8();
	uint res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_8_al(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AL_8();
	uint res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_16_d(void)
{
	uint res = MASK_OUT_ABOVE_16(DY |= OPER_I_16());

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_16_ai(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_AI_16();
	uint res = MASK_OUT_ABOVE_16(src | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_16_pi(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_PI_16();
	uint res = MASK_OUT_ABOVE_16(src | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_16_pd(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_PD_16();
	uint res = MASK_OUT_ABOVE_16(src | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_16_di(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_DI_16();
	uint res = MASK_OUT_ABOVE_16(src | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_16_ix(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_IX_16();
	uint res = MASK_OUT_ABOVE_16(src | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_16_aw(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AW_16();
	uint res = MASK_OUT_ABOVE_16(src | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_16_al(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AL_16();
	uint res = MASK_OUT_ABOVE_16(src | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_32_d(void)
{
	uint res = DY |= OPER_I_32();

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_32_ai(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_AI_32();
	uint res = src | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_32_pi(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_PI_32();
	uint res = src | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_32_pd(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_PD_32();
	uint res = src | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_32_di(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_DI_32();
	uint res = src | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_32_ix(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_IX_32();
	uint res = src | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_32_aw(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AW_32();
	uint res = src | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_32_al(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AL_32();
	uint res = src | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ori_16_toc(void)
{
	m68ki_set_ccr(m68ki_get_ccr() | OPER_I_8());
}


void m68k_op_ori_16_tos(void)
{
	if(FLAG_S)
	{
		uint src = OPER_I_16();
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(m68ki_get_sr() | src);
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_pack_16_rr(void)
{

	m68ki_exception_illegal();
}


void m68k_op_pack_16_mm_ax7(void)
{

	m68ki_exception_illegal();
}


void m68k_op_pack_16_mm_ay7(void)
{

	m68ki_exception_illegal();
}


void m68k_op_pack_16_mm_axy7(void)
{

	m68ki_exception_illegal();
}


void m68k_op_pack_16_mm(void)
{

	m68ki_exception_illegal();
}


void m68k_op_pea_32_ai(void)
{
	uint ea = EA_AY_AI_32();

	m68ki_push_32(ea);
}


void m68k_op_pea_32_di(void)
{
	uint ea = EA_AY_DI_32();

	m68ki_push_32(ea);
}


void m68k_op_pea_32_ix(void)
{
	uint ea = EA_AY_IX_32();

	m68ki_push_32(ea);
}


void m68k_op_pea_32_aw(void)
{
	uint ea = EA_AW_32();

	m68ki_push_32(ea);
}


void m68k_op_pea_32_al(void)
{
	uint ea = EA_AL_32();

	m68ki_push_32(ea);
}


void m68k_op_pea_32_pcdi(void)
{
	uint ea = EA_PCDI_32();

	m68ki_push_32(ea);
}


void m68k_op_pea_32_pcix(void)
{
	uint ea = EA_PCIX_32();

	m68ki_push_32(ea);
}


void m68k_op_pflush_32(void)
{
	if ((0) && (HAS_PMMU))
	{
		fprintf(stderr,"68040: unhandled PFLUSH\n");
		return;
	}
	m68ki_exception_1111();
}


void m68k_op_pmmu_32(void)
{
	if ((0) && (HAS_PMMU))
	{
		m68881_mmu_ops();
	}
	else
	{
		m68ki_exception_1111();
	}
}


void m68k_op_reset(void)
{
	if(FLAG_S)
	{
		m68ki_output_reset();		   /* auto-disable (see m68kcpu.h) */
		USE_CYCLES(CYC_RESET);
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_ror_8_s(void)
{
	uint* r_dst = &DY;
	uint orig_shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint shift = orig_shift & 7;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = ROR_8(src, shift);

	if(orig_shift != 0)
		USE_CYCLES(orig_shift<<CYC_SHIFT);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = src << (9-orig_shift);
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ror_16_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = ROR_16(src, shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = src << (9-shift);
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ror_32_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint64 src = *r_dst;
	uint res = ROR_32(src, shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = src << (9-shift);
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ror_8_r(void)
{
	uint* r_dst = &DY;
	uint orig_shift = DX & 0x3f;
	uint shift = orig_shift & 7;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = ROR_8(src, shift);

	if(orig_shift != 0)
	{
		USE_CYCLES(orig_shift<<CYC_SHIFT);

		*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
		FLAG_C = src << (8-((shift-1)&7));
		FLAG_N = NFLAG_8(res);
		FLAG_Z = res;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_8(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ror_16_r(void)
{
	uint* r_dst = &DY;
	uint orig_shift = DX & 0x3f;
	uint shift = orig_shift & 15;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = ROR_16(src, shift);

	if(orig_shift != 0)
	{
		USE_CYCLES(orig_shift<<CYC_SHIFT);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		FLAG_C = (src >> ((shift - 1) & 15)) << 8;
		FLAG_N = NFLAG_16(res);
		FLAG_Z = res;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_16(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ror_32_r(void)
{
	uint* r_dst = &DY;
	uint orig_shift = DX & 0x3f;
	uint shift = orig_shift & 31;
	uint64 src = *r_dst;
	uint res = ROR_32(src, shift);

	if(orig_shift != 0)
	{
		USE_CYCLES(orig_shift<<CYC_SHIFT);

		*r_dst = res;
		FLAG_C = (src >> ((shift - 1) & 31)) << 8;
		FLAG_N = NFLAG_32(res);
		FLAG_Z = res;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_32(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ror_16_ai(void)
{
	uint ea = EA_AY_AI_16();
	uint src = m68ki_read_16(ea);
	uint res = ROR_16(src, 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = src << 8;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ror_16_pi(void)
{
	uint ea = EA_AY_PI_16();
	uint src = m68ki_read_16(ea);
	uint res = ROR_16(src, 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = src << 8;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ror_16_pd(void)
{
	uint ea = EA_AY_PD_16();
	uint src = m68ki_read_16(ea);
	uint res = ROR_16(src, 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = src << 8;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ror_16_di(void)
{
	uint ea = EA_AY_DI_16();
	uint src = m68ki_read_16(ea);
	uint res = ROR_16(src, 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = src << 8;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ror_16_ix(void)
{
	uint ea = EA_AY_IX_16();
	uint src = m68ki_read_16(ea);
	uint res = ROR_16(src, 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = src << 8;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ror_16_aw(void)
{
	uint ea = EA_AW_16();
	uint src = m68ki_read_16(ea);
	uint res = ROR_16(src, 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = src << 8;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_ror_16_al(void)
{
	uint ea = EA_AL_16();
	uint src = m68ki_read_16(ea);
	uint res = ROR_16(src, 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = src << 8;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_rol_8_s(void)
{
	uint* r_dst = &DY;
	uint orig_shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint shift = orig_shift & 7;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = ROL_8(src, shift);

	if(orig_shift != 0)
		USE_CYCLES(orig_shift<<CYC_SHIFT);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = src << orig_shift;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_rol_16_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = ROL_16(src, shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = src >> (8-shift);
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_rol_32_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint64 src = *r_dst;
	uint res = ROL_32(src, shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = src >> (24-shift);
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_rol_8_r(void)
{
	uint* r_dst = &DY;
	uint orig_shift = DX & 0x3f;
	uint shift = orig_shift & 7;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = ROL_8(src, shift);

	if(orig_shift != 0)
	{
		USE_CYCLES(orig_shift<<CYC_SHIFT);

		if(shift != 0)
		{
			*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
			FLAG_C = src << shift;
			FLAG_N = NFLAG_8(res);
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}
		FLAG_C = (src & 1)<<8;
		FLAG_N = NFLAG_8(src);
		FLAG_Z = src;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_8(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_rol_16_r(void)
{
	uint* r_dst = &DY;
	uint orig_shift = DX & 0x3f;
	uint shift = orig_shift & 15;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = MASK_OUT_ABOVE_16(ROL_16(src, shift));

	if(orig_shift != 0)
	{
		USE_CYCLES(orig_shift<<CYC_SHIFT);

		if(shift != 0)
		{
			*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
			FLAG_C = (src << shift) >> 8;
			FLAG_N = NFLAG_16(res);
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}
		FLAG_C = (src & 1)<<8;
		FLAG_N = NFLAG_16(src);
		FLAG_Z = src;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_16(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_rol_32_r(void)
{
	uint* r_dst = &DY;
	uint orig_shift = DX & 0x3f;
	uint shift = orig_shift & 31;
	uint64 src = *r_dst;
	uint res = ROL_32(src, shift);

	if(orig_shift != 0)
	{
		USE_CYCLES(orig_shift<<CYC_SHIFT);

		*r_dst = res;

		FLAG_C = (src >> ((32 - shift) & 0x1f)) << 8;
		FLAG_N = NFLAG_32(res);
		FLAG_Z = res;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_32(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_rol_16_ai(void)
{
	uint ea = EA_AY_AI_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = src >> 7;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_rol_16_pi(void)
{
	uint ea = EA_AY_PI_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = src >> 7;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_rol_16_pd(void)
{
	uint ea = EA_AY_PD_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = src >> 7;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_rol_16_di(void)
{
	uint ea = EA_AY_DI_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = src >> 7;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_rol_16_ix(void)
{
	uint ea = EA_AY_IX_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = src >> 7;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_rol_16_aw(void)
{
	uint ea = EA_AW_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = src >> 7;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_rol_16_al(void)
{
	uint ea = EA_AL_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = src >> 7;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_roxr_8_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = ROR_9(src | (XFLAG_AS_1() << 8), shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	FLAG_C = FLAG_X = res;
	res = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_roxr_16_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = ROR_17(src | (XFLAG_AS_1() << 16), shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	FLAG_C = FLAG_X = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_roxr_32_s(void)
{
#if M68K_USE_64_BIT

	uint*  r_dst = &DY;
	uint   shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint64 src   = *r_dst;
	uint64 res   = src | (((uint64)XFLAG_AS_1()) << 32);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	res = ROR_33_64(res, shift);

	FLAG_C = FLAG_X = res >> 24;
	res = MASK_OUT_ABOVE_32(res);

	*r_dst =  res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;

#else

	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = *r_dst;
	uint res = MASK_OUT_ABOVE_32((ROR_33(src, shift) & ~(1 << (32 - shift))) | (XFLAG_AS_1() << (32 - shift)));
	uint new_x_flag = src & (1 << (shift - 1));

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = res;

	FLAG_C = FLAG_X = (new_x_flag != 0)<<8;
	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;

#endif
}


void m68k_op_roxr_8_r(void)
{
	uint* r_dst = &DY;
	uint orig_shift = DX & 0x3f;

	if(orig_shift != 0)
	{
		uint shift = orig_shift % 9;
		uint src   = MASK_OUT_ABOVE_8(*r_dst);
		uint res   = ROR_9(src | (XFLAG_AS_1() << 8), shift);

		USE_CYCLES(orig_shift<<CYC_SHIFT);

		FLAG_C = FLAG_X = res;
		res = MASK_OUT_ABOVE_8(res);

		*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
		FLAG_N = NFLAG_8(res);
		FLAG_Z = res;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = FLAG_X;
	FLAG_N = NFLAG_8(*r_dst);
	FLAG_Z = MASK_OUT_ABOVE_8(*r_dst);
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_roxr_16_r(void)
{
	uint* r_dst = &DY;
	uint orig_shift = DX & 0x3f;

	if(orig_shift != 0)
	{
		uint shift = orig_shift % 17;
		uint src   = MASK_OUT_ABOVE_16(*r_dst);
		uint res   = ROR_17(src | (XFLAG_AS_1() << 16), shift);

		USE_CYCLES(orig_shift<<CYC_SHIFT);

		FLAG_C = FLAG_X = res >> 8;
		res = MASK_OUT_ABOVE_16(res);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		FLAG_N = NFLAG_16(res);
		FLAG_Z = res;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = FLAG_X;
	FLAG_N = NFLAG_16(*r_dst);
	FLAG_Z = MASK_OUT_ABOVE_16(*r_dst);
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_roxr_32_r(void)
{
#if M68K_USE_64_BIT

	uint*  r_dst = &DY;
	uint   orig_shift = DX & 0x3f;

	if(orig_shift != 0)
	{
		uint   shift = orig_shift % 33;
		uint64 src   = *r_dst;
		uint64 res   = src | (((uint64)XFLAG_AS_1()) << 32);

		res = ROR_33_64(res, shift);

		USE_CYCLES(orig_shift<<CYC_SHIFT);

		FLAG_C = FLAG_X = res >> 24;
		res = MASK_OUT_ABOVE_32(res);

		*r_dst = res;
		FLAG_N = NFLAG_32(res);
		FLAG_Z = res;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = FLAG_X;
	FLAG_N = NFLAG_32(*r_dst);
	FLAG_Z = *r_dst;
	FLAG_V = VFLAG_CLEAR;

#else

	uint* r_dst = &DY;
	uint orig_shift = DX & 0x3f;
	uint shift = orig_shift % 33;
	uint src = *r_dst;
	uint res = MASK_OUT_ABOVE_32((ROR_33(src, shift) & ~(1 << (32 - shift))) | (XFLAG_AS_1() << (32 - shift)));
	uint new_x_flag = src & (1 << (shift - 1));

	if(orig_shift != 0)
		USE_CYCLES(orig_shift<<CYC_SHIFT);

	if(shift != 0)
	{
		*r_dst = res;
		FLAG_X = (new_x_flag != 0)<<8;
	}
	else
		res = src;
	FLAG_C = FLAG_X;
	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;

#endif
}


void m68k_op_roxr_16_ai(void)
{
	uint ea = EA_AY_AI_16();
	uint src = m68ki_read_16(ea);
	uint res = ROR_17(src | (XFLAG_AS_1() << 16), 1);

	FLAG_C = FLAG_X = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_roxr_16_pi(void)
{
	uint ea = EA_AY_PI_16();
	uint src = m68ki_read_16(ea);
	uint res = ROR_17(src | (XFLAG_AS_1() << 16), 1);

	FLAG_C = FLAG_X = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_roxr_16_pd(void)
{
	uint ea = EA_AY_PD_16();
	uint src = m68ki_read_16(ea);
	uint res = ROR_17(src | (XFLAG_AS_1() << 16), 1);

	FLAG_C = FLAG_X = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_roxr_16_di(void)
{
	uint ea = EA_AY_DI_16();
	uint src = m68ki_read_16(ea);
	uint res = ROR_17(src | (XFLAG_AS_1() << 16), 1);

	FLAG_C = FLAG_X = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_roxr_16_ix(void)
{
	uint ea = EA_AY_IX_16();
	uint src = m68ki_read_16(ea);
	uint res = ROR_17(src | (XFLAG_AS_1() << 16), 1);

	FLAG_C = FLAG_X = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_roxr_16_aw(void)
{
	uint ea = EA_AW_16();
	uint src = m68ki_read_16(ea);
	uint res = ROR_17(src | (XFLAG_AS_1() << 16), 1);

	FLAG_C = FLAG_X = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_roxr_16_al(void)
{
	uint ea = EA_AL_16();
	uint src = m68ki_read_16(ea);
	uint res = ROR_17(src | (XFLAG_AS_1() << 16), 1);

	FLAG_C = FLAG_X = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_roxl_8_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = ROL_9(src | (XFLAG_AS_1() << 8), shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	FLAG_C = FLAG_X = res;
	res = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_roxl_16_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = ROL_17(src | (XFLAG_AS_1() << 16), shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	FLAG_C = FLAG_X = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_roxl_32_s(void)
{
#if M68K_USE_64_BIT

	uint*  r_dst = &DY;
	uint   shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint64 src   = *r_dst;
	uint64 res   = src | (((uint64)XFLAG_AS_1()) << 32);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	res = ROL_33_64(res, shift);

	FLAG_C = FLAG_X = res >> 24;
	res = MASK_OUT_ABOVE_32(res);

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;

#else

	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = *r_dst;
	uint res = MASK_OUT_ABOVE_32((ROL_33(src, shift) & ~(1 << (shift - 1))) | (XFLAG_AS_1() << (shift - 1)));
	uint new_x_flag = src & (1 << (32 - shift));

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = res;

	FLAG_C = FLAG_X = (new_x_flag != 0)<<8;
	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;

#endif
}


void m68k_op_roxl_8_r(void)
{
	uint* r_dst = &DY;
	uint orig_shift = DX & 0x3f;


	if(orig_shift != 0)
	{
		uint shift = orig_shift % 9;
		uint src   = MASK_OUT_ABOVE_8(*r_dst);
		uint res   = ROL_9(src | (XFLAG_AS_1() << 8), shift);

		USE_CYCLES(orig_shift<<CYC_SHIFT);

		FLAG_C = FLAG_X = res;
		res = MASK_OUT_ABOVE_8(res);

		*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
		FLAG_N = NFLAG_8(res);
		FLAG_Z = res;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = FLAG_X;
	FLAG_N = NFLAG_8(*r_dst);
	FLAG_Z = MASK_OUT_ABOVE_8(*r_dst);
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_roxl_16_r(void)
{
	uint* r_dst = &DY;
	uint orig_shift = DX & 0x3f;

	if(orig_shift != 0)
	{
		uint shift = orig_shift % 17;
		uint src   = MASK_OUT_ABOVE_16(*r_dst);
		uint res   = ROL_17(src | (XFLAG_AS_1() << 16), shift);

		USE_CYCLES(orig_shift<<CYC_SHIFT);

		FLAG_C = FLAG_X = res >> 8;
		res = MASK_OUT_ABOVE_16(res);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		FLAG_N = NFLAG_16(res);
		FLAG_Z = res;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = FLAG_X;
	FLAG_N = NFLAG_16(*r_dst);
	FLAG_Z = MASK_OUT_ABOVE_16(*r_dst);
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_roxl_32_r(void)
{
#if M68K_USE_64_BIT

	uint*  r_dst = &DY;
	uint   orig_shift = DX & 0x3f;

	if(orig_shift != 0)
	{
		uint   shift = orig_shift % 33;
		uint64 src   = *r_dst;
		uint64 res   = src | (((uint64)XFLAG_AS_1()) << 32);

		res = ROL_33_64(res, shift);

		USE_CYCLES(orig_shift<<CYC_SHIFT);

		FLAG_C = FLAG_X = res >> 24;
		res = MASK_OUT_ABOVE_32(res);

		*r_dst = res;
		FLAG_N = NFLAG_32(res);
		FLAG_Z = res;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = FLAG_X;
	FLAG_N = NFLAG_32(*r_dst);
	FLAG_Z = *r_dst;
	FLAG_V = VFLAG_CLEAR;

#else

	uint* r_dst = &DY;
	uint orig_shift = DX & 0x3f;
	uint shift = orig_shift % 33;
	uint src = *r_dst;
	uint res = MASK_OUT_ABOVE_32((ROL_33(src, shift) & ~(1 << (shift - 1))) | (XFLAG_AS_1() << (shift - 1)));
	uint new_x_flag = src & (1 << (32 - shift));

	if(orig_shift != 0)
		USE_CYCLES(orig_shift<<CYC_SHIFT);

	if(shift != 0)
	{
		*r_dst = res;
		FLAG_X = (new_x_flag != 0)<<8;
	}
	else
		res = src;
	FLAG_C = FLAG_X;
	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;

#endif
}


void m68k_op_roxl_16_ai(void)
{
	uint ea = EA_AY_AI_16();
	uint src = m68ki_read_16(ea);
	uint res = ROL_17(src | (XFLAG_AS_1() << 16), 1);

	FLAG_C = FLAG_X = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_roxl_16_pi(void)
{
	uint ea = EA_AY_PI_16();
	uint src = m68ki_read_16(ea);
	uint res = ROL_17(src | (XFLAG_AS_1() << 16), 1);

	FLAG_C = FLAG_X = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_roxl_16_pd(void)
{
	uint ea = EA_AY_PD_16();
	uint src = m68ki_read_16(ea);
	uint res = ROL_17(src | (XFLAG_AS_1() << 16), 1);

	FLAG_C = FLAG_X = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_roxl_16_di(void)
{
	uint ea = EA_AY_DI_16();
	uint src = m68ki_read_16(ea);
	uint res = ROL_17(src | (XFLAG_AS_1() << 16), 1);

	FLAG_C = FLAG_X = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_roxl_16_ix(void)
{
	uint ea = EA_AY_IX_16();
	uint src = m68ki_read_16(ea);
	uint res = ROL_17(src | (XFLAG_AS_1() << 16), 1);

	FLAG_C = FLAG_X = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_roxl_16_aw(void)
{
	uint ea = EA_AW_16();
	uint src = m68ki_read_16(ea);
	uint res = ROL_17(src | (XFLAG_AS_1() << 16), 1);

	FLAG_C = FLAG_X = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_roxl_16_al(void)
{
	uint ea = EA_AL_16();
	uint src = m68ki_read_16(ea);
	uint res = ROL_17(src | (XFLAG_AS_1() << 16), 1);

	FLAG_C = FLAG_X = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_rtd_32(void)
{

	m68ki_exception_illegal();
}


void m68k_op_rte_32(void)
{
	if(FLAG_S)
	{
		uint new_sr;
		uint new_pc;
		uint format_word;

		m68ki_rte_callback();		   /* auto-disable (see m68kcpu.h) */
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

		if(1)
		{
			new_sr = m68ki_pull_16();
			new_pc = m68ki_pull_32();
			m68ki_jump(new_pc);
			m68ki_set_sr(new_sr);

			CPU_INSTR_MODE = INSTRUCTION_YES;
			CPU_RUN_MODE = RUN_MODE_NORMAL;

			return;
		}

		if(0)
		{
			format_word = m68ki_read_16(REG_A[7]+6) >> 12;
			if(format_word == 0)
			{
				new_sr = m68ki_pull_16();
				new_pc = m68ki_pull_32();
				m68ki_fake_pull_16();	/* format word */
				m68ki_jump(new_pc);
				m68ki_set_sr(new_sr);
				CPU_INSTR_MODE = INSTRUCTION_YES;
				CPU_RUN_MODE = RUN_MODE_NORMAL;
				return;
			} else if (format_word == 8) {
				/* Format 8 stack frame -- 68010 only. 29 word bus/address error */
				new_sr = m68ki_pull_16();
				new_pc = m68ki_pull_32();
				m68ki_fake_pull_16();	/* format word */
				m68ki_fake_pull_16();	/* special status word */
				m68ki_fake_pull_32();	/* fault address */
				m68ki_fake_pull_16();	/* unused/reserved */
				m68ki_fake_pull_16();	/* data output buffer */
				m68ki_fake_pull_16();	/* unused/reserved */
				m68ki_fake_pull_16();	/* data input buffer */
				m68ki_fake_pull_16();	/* unused/reserved */
				m68ki_fake_pull_16();	/* instruction input buffer */
				m68ki_fake_pull_32();	/* internal information, 16 words */
				m68ki_fake_pull_32();	/* (actually, we use 8 DWORDs) */
				m68ki_fake_pull_32();
				m68ki_fake_pull_32();
				m68ki_fake_pull_32();
				m68ki_fake_pull_32();
				m68ki_fake_pull_32();
				m68ki_fake_pull_32();
				m68ki_jump(new_pc);
				m68ki_set_sr(new_sr);
				CPU_INSTR_MODE = INSTRUCTION_YES;
				CPU_RUN_MODE = RUN_MODE_NORMAL;
				return;
			}
			CPU_INSTR_MODE = INSTRUCTION_YES;
			CPU_RUN_MODE = RUN_MODE_NORMAL;
			/* Not handling other exception types (9) */
			m68ki_exception_format_error();
			return;
		}

		/* Otherwise it's 020 */
rte_loop:
		format_word = m68ki_read_16(REG_A[7]+6) >> 12;
		switch(format_word)
		{
			case 0: /* Normal */
				new_sr = m68ki_pull_16();
				new_pc = m68ki_pull_32();
				m68ki_fake_pull_16();	/* format word */
				m68ki_jump(new_pc);
				m68ki_set_sr(new_sr);
				CPU_INSTR_MODE = INSTRUCTION_YES;
				CPU_RUN_MODE = RUN_MODE_NORMAL;
				return;
			case 1: /* Throwaway */
				new_sr = m68ki_pull_16();
				m68ki_fake_pull_32();	/* program counter */
				m68ki_fake_pull_16();	/* format word */
				m68ki_set_sr_noint(new_sr);
				goto rte_loop;
			case 2: /* Trap */
				new_sr = m68ki_pull_16();
				new_pc = m68ki_pull_32();
				m68ki_fake_pull_16();	/* format word */
				m68ki_fake_pull_32();	/* address */
				m68ki_jump(new_pc);
				m68ki_set_sr(new_sr);
				CPU_INSTR_MODE = INSTRUCTION_YES;
				CPU_RUN_MODE = RUN_MODE_NORMAL;
				return;
		}
		/* Not handling long or short bus fault */
		CPU_INSTR_MODE = INSTRUCTION_YES;
		CPU_RUN_MODE = RUN_MODE_NORMAL;
		m68ki_exception_format_error();
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_rtm_32(void)
{

	m68ki_exception_illegal();
}


void m68k_op_rtr_32(void)
{
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	m68ki_set_ccr(m68ki_pull_16());
	m68ki_jump(m68ki_pull_32());
}


void m68k_op_rts_32(void)
{
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	m68ki_jump(m68ki_pull_32());
}


void m68k_op_sbcd_8_rr(void)
{
	uint* r_dst = &DX;
	uint src = DY;
	uint dst = *r_dst;
	uint res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res -= 6;
	res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res += 0xa0;

	res = MASK_OUT_ABOVE_8(res);

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */
	FLAG_Z |= res;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
}


void m68k_op_sbcd_8_mm_ax7(void)
{
	uint src = OPER_AY_PD_8();
	uint ea  = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res -= 6;
	res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res += 0xa0;

	res = MASK_OUT_ABOVE_8(res);

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_sbcd_8_mm_ay7(void)
{
	uint src = OPER_A7_PD_8();
	uint ea  = EA_AX_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res -= 6;
	res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res += 0xa0;

	res = MASK_OUT_ABOVE_8(res);

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_sbcd_8_mm_axy7(void)
{
	uint src = OPER_A7_PD_8();
	uint ea  = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res -= 6;
	res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res += 0xa0;

	res = MASK_OUT_ABOVE_8(res);

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_sbcd_8_mm(void)
{
	uint src = OPER_AY_PD_8();
	uint ea  = EA_AX_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res -= 6;
	res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res += 0xa0;

	res = MASK_OUT_ABOVE_8(res);

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_st_8_d(void)
{
	DY |= 0xff;
}


void m68k_op_st_8_ai(void)
{
	m68ki_write_8(EA_AY_AI_8(), 0xff);
}


void m68k_op_st_8_pi(void)
{
	m68ki_write_8(EA_AY_PI_8(), 0xff);
}


void m68k_op_st_8_pi7(void)
{
	m68ki_write_8(EA_A7_PI_8(), 0xff);
}


void m68k_op_st_8_pd(void)
{
	m68ki_write_8(EA_AY_PD_8(), 0xff);
}


void m68k_op_st_8_pd7(void)
{
	m68ki_write_8(EA_A7_PD_8(), 0xff);
}


void m68k_op_st_8_di(void)
{
	m68ki_write_8(EA_AY_DI_8(), 0xff);
}


void m68k_op_st_8_ix(void)
{
	m68ki_write_8(EA_AY_IX_8(), 0xff);
}


void m68k_op_st_8_aw(void)
{
	m68ki_write_8(EA_AW_8(), 0xff);
}


void m68k_op_st_8_al(void)
{
	m68ki_write_8(EA_AL_8(), 0xff);
}


void m68k_op_sf_8_d(void)
{
	DY &= 0xffffff00;
}


void m68k_op_sf_8_ai(void)
{
	m68ki_write_8(EA_AY_AI_8(), 0);
}


void m68k_op_sf_8_pi(void)
{
	m68ki_write_8(EA_AY_PI_8(), 0);
}


void m68k_op_sf_8_pi7(void)
{
	m68ki_write_8(EA_A7_PI_8(), 0);
}


void m68k_op_sf_8_pd(void)
{
	m68ki_write_8(EA_AY_PD_8(), 0);
}


void m68k_op_sf_8_pd7(void)
{
	m68ki_write_8(EA_A7_PD_8(), 0);
}


void m68k_op_sf_8_di(void)
{
	m68ki_write_8(EA_AY_DI_8(), 0);
}


void m68k_op_sf_8_ix(void)
{
	m68ki_write_8(EA_AY_IX_8(), 0);
}


void m68k_op_sf_8_aw(void)
{
	m68ki_write_8(EA_AW_8(), 0);
}


void m68k_op_sf_8_al(void)
{
	m68ki_write_8(EA_AL_8(), 0);
}


void m68k_op_shi_8_d(void)
{
	if(COND_HI())
	{
		DY |= 0xff;
		USE_CYCLES(CYC_SCC_R_TRUE);
		return;
	}
	DY &= 0xffffff00;
}


void m68k_op_sls_8_d(void)
{
	if(COND_LS())
	{
		DY |= 0xff;
		USE_CYCLES(CYC_SCC_R_TRUE);
		return;
	}
	DY &= 0xffffff00;
}


void m68k_op_scc_8_d(void)
{
	if(COND_CC())
	{
		DY |= 0xff;
		USE_CYCLES(CYC_SCC_R_TRUE);
		return;
	}
	DY &= 0xffffff00;
}


void m68k_op_scs_8_d(void)
{
	if(COND_CS())
	{
		DY |= 0xff;
		USE_CYCLES(CYC_SCC_R_TRUE);
		return;
	}
	DY &= 0xffffff00;
}


void m68k_op_sne_8_d(void)
{
	if(COND_NE())
	{
		DY |= 0xff;
		USE_CYCLES(CYC_SCC_R_TRUE);
		return;
	}
	DY &= 0xffffff00;
}


void m68k_op_seq_8_d(void)
{
	if(COND_EQ())
	{
		DY |= 0xff;
		USE_CYCLES(CYC_SCC_R_TRUE);
		return;
	}
	DY &= 0xffffff00;
}


void m68k_op_svc_8_d(void)
{
	if(COND_VC())
	{
		DY |= 0xff;
		USE_CYCLES(CYC_SCC_R_TRUE);
		return;
	}
	DY &= 0xffffff00;
}


void m68k_op_svs_8_d(void)
{
	if(COND_VS())
	{
		DY |= 0xff;
		USE_CYCLES(CYC_SCC_R_TRUE);
		return;
	}
	DY &= 0xffffff00;
}


void m68k_op_spl_8_d(void)
{
	if(COND_PL())
	{
		DY |= 0xff;
		USE_CYCLES(CYC_SCC_R_TRUE);
		return;
	}
	DY &= 0xffffff00;
}


void m68k_op_smi_8_d(void)
{
	if(COND_MI())
	{
		DY |= 0xff;
		USE_CYCLES(CYC_SCC_R_TRUE);
		return;
	}
	DY &= 0xffffff00;
}


void m68k_op_sge_8_d(void)
{
	if(COND_GE())
	{
		DY |= 0xff;
		USE_CYCLES(CYC_SCC_R_TRUE);
		return;
	}
	DY &= 0xffffff00;
}


void m68k_op_slt_8_d(void)
{
	if(COND_LT())
	{
		DY |= 0xff;
		USE_CYCLES(CYC_SCC_R_TRUE);
		return;
	}
	DY &= 0xffffff00;
}


void m68k_op_sgt_8_d(void)
{
	if(COND_GT())
	{
		DY |= 0xff;
		USE_CYCLES(CYC_SCC_R_TRUE);
		return;
	}
	DY &= 0xffffff00;
}


void m68k_op_sle_8_d(void)
{
	if(COND_LE())
	{
		DY |= 0xff;
		USE_CYCLES(CYC_SCC_R_TRUE);
		return;
	}
	DY &= 0xffffff00;
}


void m68k_op_shi_8_ai(void)
{
	m68ki_write_8(EA_AY_AI_8(), COND_HI() ? 0xff : 0);
}


void m68k_op_shi_8_pi(void)
{
	m68ki_write_8(EA_AY_PI_8(), COND_HI() ? 0xff : 0);
}


void m68k_op_shi_8_pi7(void)
{
	m68ki_write_8(EA_A7_PI_8(), COND_HI() ? 0xff : 0);
}


void m68k_op_shi_8_pd(void)
{
	m68ki_write_8(EA_AY_PD_8(), COND_HI() ? 0xff : 0);
}


void m68k_op_shi_8_pd7(void)
{
	m68ki_write_8(EA_A7_PD_8(), COND_HI() ? 0xff : 0);
}


void m68k_op_shi_8_di(void)
{
	m68ki_write_8(EA_AY_DI_8(), COND_HI() ? 0xff : 0);
}


void m68k_op_shi_8_ix(void)
{
	m68ki_write_8(EA_AY_IX_8(), COND_HI() ? 0xff : 0);
}


void m68k_op_shi_8_aw(void)
{
	m68ki_write_8(EA_AW_8(), COND_HI() ? 0xff : 0);
}


void m68k_op_shi_8_al(void)
{
	m68ki_write_8(EA_AL_8(), COND_HI() ? 0xff : 0);
}


void m68k_op_sls_8_ai(void)
{
	m68ki_write_8(EA_AY_AI_8(), COND_LS() ? 0xff : 0);
}


void m68k_op_sls_8_pi(void)
{
	m68ki_write_8(EA_AY_PI_8(), COND_LS() ? 0xff : 0);
}


void m68k_op_sls_8_pi7(void)
{
	m68ki_write_8(EA_A7_PI_8(), COND_LS() ? 0xff : 0);
}


void m68k_op_sls_8_pd(void)
{
	m68ki_write_8(EA_AY_PD_8(), COND_LS() ? 0xff : 0);
}


void m68k_op_sls_8_pd7(void)
{
	m68ki_write_8(EA_A7_PD_8(), COND_LS() ? 0xff : 0);
}


void m68k_op_sls_8_di(void)
{
	m68ki_write_8(EA_AY_DI_8(), COND_LS() ? 0xff : 0);
}


void m68k_op_sls_8_ix(void)
{
	m68ki_write_8(EA_AY_IX_8(), COND_LS() ? 0xff : 0);
}


void m68k_op_sls_8_aw(void)
{
	m68ki_write_8(EA_AW_8(), COND_LS() ? 0xff : 0);
}


void m68k_op_sls_8_al(void)
{
	m68ki_write_8(EA_AL_8(), COND_LS() ? 0xff : 0);
}


void m68k_op_scc_8_ai(void)
{
	m68ki_write_8(EA_AY_AI_8(), COND_CC() ? 0xff : 0);
}


void m68k_op_scc_8_pi(void)
{
	m68ki_write_8(EA_AY_PI_8(), COND_CC() ? 0xff : 0);
}


void m68k_op_scc_8_pi7(void)
{
	m68ki_write_8(EA_A7_PI_8(), COND_CC() ? 0xff : 0);
}


void m68k_op_scc_8_pd(void)
{
	m68ki_write_8(EA_AY_PD_8(), COND_CC() ? 0xff : 0);
}


void m68k_op_scc_8_pd7(void)
{
	m68ki_write_8(EA_A7_PD_8(), COND_CC() ? 0xff : 0);
}


void m68k_op_scc_8_di(void)
{
	m68ki_write_8(EA_AY_DI_8(), COND_CC() ? 0xff : 0);
}


void m68k_op_scc_8_ix(void)
{
	m68ki_write_8(EA_AY_IX_8(), COND_CC() ? 0xff : 0);
}


void m68k_op_scc_8_aw(void)
{
	m68ki_write_8(EA_AW_8(), COND_CC() ? 0xff : 0);
}


void m68k_op_scc_8_al(void)
{
	m68ki_write_8(EA_AL_8(), COND_CC() ? 0xff : 0);
}


void m68k_op_scs_8_ai(void)
{
	m68ki_write_8(EA_AY_AI_8(), COND_CS() ? 0xff : 0);
}


void m68k_op_scs_8_pi(void)
{
	m68ki_write_8(EA_AY_PI_8(), COND_CS() ? 0xff : 0);
}


void m68k_op_scs_8_pi7(void)
{
	m68ki_write_8(EA_A7_PI_8(), COND_CS() ? 0xff : 0);
}


void m68k_op_scs_8_pd(void)
{
	m68ki_write_8(EA_AY_PD_8(), COND_CS() ? 0xff : 0);
}


void m68k_op_scs_8_pd7(void)
{
	m68ki_write_8(EA_A7_PD_8(), COND_CS() ? 0xff : 0);
}


void m68k_op_scs_8_di(void)
{
	m68ki_write_8(EA_AY_DI_8(), COND_CS() ? 0xff : 0);
}


void m68k_op_scs_8_ix(void)
{
	m68ki_write_8(EA_AY_IX_8(), COND_CS() ? 0xff : 0);
}


void m68k_op_scs_8_aw(void)
{
	m68ki_write_8(EA_AW_8(), COND_CS() ? 0xff : 0);
}


void m68k_op_scs_8_al(void)
{
	m68ki_write_8(EA_AL_8(), COND_CS() ? 0xff : 0);
}


void m68k_op_sne_8_ai(void)
{
	m68ki_write_8(EA_AY_AI_8(), COND_NE() ? 0xff : 0);
}


void m68k_op_sne_8_pi(void)
{
	m68ki_write_8(EA_AY_PI_8(), COND_NE() ? 0xff : 0);
}


void m68k_op_sne_8_pi7(void)
{
	m68ki_write_8(EA_A7_PI_8(), COND_NE() ? 0xff : 0);
}


void m68k_op_sne_8_pd(void)
{
	m68ki_write_8(EA_AY_PD_8(), COND_NE() ? 0xff : 0);
}


void m68k_op_sne_8_pd7(void)
{
	m68ki_write_8(EA_A7_PD_8(), COND_NE() ? 0xff : 0);
}


void m68k_op_sne_8_di(void)
{
	m68ki_write_8(EA_AY_DI_8(), COND_NE() ? 0xff : 0);
}


void m68k_op_sne_8_ix(void)
{
	m68ki_write_8(EA_AY_IX_8(), COND_NE() ? 0xff : 0);
}


void m68k_op_sne_8_aw(void)
{
	m68ki_write_8(EA_AW_8(), COND_NE() ? 0xff : 0);
}


void m68k_op_sne_8_al(void)
{
	m68ki_write_8(EA_AL_8(), COND_NE() ? 0xff : 0);
}


void m68k_op_seq_8_ai(void)
{
	m68ki_write_8(EA_AY_AI_8(), COND_EQ() ? 0xff : 0);
}


void m68k_op_seq_8_pi(void)
{
	m68ki_write_8(EA_AY_PI_8(), COND_EQ() ? 0xff : 0);
}


void m68k_op_seq_8_pi7(void)
{
	m68ki_write_8(EA_A7_PI_8(), COND_EQ() ? 0xff : 0);
}


void m68k_op_seq_8_pd(void)
{
	m68ki_write_8(EA_AY_PD_8(), COND_EQ() ? 0xff : 0);
}


void m68k_op_seq_8_pd7(void)
{
	m68ki_write_8(EA_A7_PD_8(), COND_EQ() ? 0xff : 0);
}


void m68k_op_seq_8_di(void)
{
	m68ki_write_8(EA_AY_DI_8(), COND_EQ() ? 0xff : 0);
}


void m68k_op_seq_8_ix(void)
{
	m68ki_write_8(EA_AY_IX_8(), COND_EQ() ? 0xff : 0);
}


void m68k_op_seq_8_aw(void)
{
	m68ki_write_8(EA_AW_8(), COND_EQ() ? 0xff : 0);
}


void m68k_op_seq_8_al(void)
{
	m68ki_write_8(EA_AL_8(), COND_EQ() ? 0xff : 0);
}


void m68k_op_svc_8_ai(void)
{
	m68ki_write_8(EA_AY_AI_8(), COND_VC() ? 0xff : 0);
}


void m68k_op_svc_8_pi(void)
{
	m68ki_write_8(EA_AY_PI_8(), COND_VC() ? 0xff : 0);
}


void m68k_op_svc_8_pi7(void)
{
	m68ki_write_8(EA_A7_PI_8(), COND_VC() ? 0xff : 0);
}


void m68k_op_svc_8_pd(void)
{
	m68ki_write_8(EA_AY_PD_8(), COND_VC() ? 0xff : 0);
}


void m68k_op_svc_8_pd7(void)
{
	m68ki_write_8(EA_A7_PD_8(), COND_VC() ? 0xff : 0);
}


void m68k_op_svc_8_di(void)
{
	m68ki_write_8(EA_AY_DI_8(), COND_VC() ? 0xff : 0);
}


void m68k_op_svc_8_ix(void)
{
	m68ki_write_8(EA_AY_IX_8(), COND_VC() ? 0xff : 0);
}


void m68k_op_svc_8_aw(void)
{
	m68ki_write_8(EA_AW_8(), COND_VC() ? 0xff : 0);
}


void m68k_op_svc_8_al(void)
{
	m68ki_write_8(EA_AL_8(), COND_VC() ? 0xff : 0);
}


void m68k_op_svs_8_ai(void)
{
	m68ki_write_8(EA_AY_AI_8(), COND_VS() ? 0xff : 0);
}


void m68k_op_svs_8_pi(void)
{
	m68ki_write_8(EA_AY_PI_8(), COND_VS() ? 0xff : 0);
}


void m68k_op_svs_8_pi7(void)
{
	m68ki_write_8(EA_A7_PI_8(), COND_VS() ? 0xff : 0);
}


void m68k_op_svs_8_pd(void)
{
	m68ki_write_8(EA_AY_PD_8(), COND_VS() ? 0xff : 0);
}


void m68k_op_svs_8_pd7(void)
{
	m68ki_write_8(EA_A7_PD_8(), COND_VS() ? 0xff : 0);
}


void m68k_op_svs_8_di(void)
{
	m68ki_write_8(EA_AY_DI_8(), COND_VS() ? 0xff : 0);
}


void m68k_op_svs_8_ix(void)
{
	m68ki_write_8(EA_AY_IX_8(), COND_VS() ? 0xff : 0);
}


void m68k_op_svs_8_aw(void)
{
	m68ki_write_8(EA_AW_8(), COND_VS() ? 0xff : 0);
}


void m68k_op_svs_8_al(void)
{
	m68ki_write_8(EA_AL_8(), COND_VS() ? 0xff : 0);
}


void m68k_op_spl_8_ai(void)
{
	m68ki_write_8(EA_AY_AI_8(), COND_PL() ? 0xff : 0);
}


void m68k_op_spl_8_pi(void)
{
	m68ki_write_8(EA_AY_PI_8(), COND_PL() ? 0xff : 0);
}


void m68k_op_spl_8_pi7(void)
{
	m68ki_write_8(EA_A7_PI_8(), COND_PL() ? 0xff : 0);
}


void m68k_op_spl_8_pd(void)
{
	m68ki_write_8(EA_AY_PD_8(), COND_PL() ? 0xff : 0);
}


void m68k_op_spl_8_pd7(void)
{
	m68ki_write_8(EA_A7_PD_8(), COND_PL() ? 0xff : 0);
}


void m68k_op_spl_8_di(void)
{
	m68ki_write_8(EA_AY_DI_8(), COND_PL() ? 0xff : 0);
}


void m68k_op_spl_8_ix(void)
{
	m68ki_write_8(EA_AY_IX_8(), COND_PL() ? 0xff : 0);
}


void m68k_op_spl_8_aw(void)
{
	m68ki_write_8(EA_AW_8(), COND_PL() ? 0xff : 0);
}


void m68k_op_spl_8_al(void)
{
	m68ki_write_8(EA_AL_8(), COND_PL() ? 0xff : 0);
}


void m68k_op_smi_8_ai(void)
{
	m68ki_write_8(EA_AY_AI_8(), COND_MI() ? 0xff : 0);
}


void m68k_op_smi_8_pi(void)
{
	m68ki_write_8(EA_AY_PI_8(), COND_MI() ? 0xff : 0);
}


void m68k_op_smi_8_pi7(void)
{
	m68ki_write_8(EA_A7_PI_8(), COND_MI() ? 0xff : 0);
}


void m68k_op_smi_8_pd(void)
{
	m68ki_write_8(EA_AY_PD_8(), COND_MI() ? 0xff : 0);
}


void m68k_op_smi_8_pd7(void)
{
	m68ki_write_8(EA_A7_PD_8(), COND_MI() ? 0xff : 0);
}


void m68k_op_smi_8_di(void)
{
	m68ki_write_8(EA_AY_DI_8(), COND_MI() ? 0xff : 0);
}


void m68k_op_smi_8_ix(void)
{
	m68ki_write_8(EA_AY_IX_8(), COND_MI() ? 0xff : 0);
}


void m68k_op_smi_8_aw(void)
{
	m68ki_write_8(EA_AW_8(), COND_MI() ? 0xff : 0);
}


void m68k_op_smi_8_al(void)
{
	m68ki_write_8(EA_AL_8(), COND_MI() ? 0xff : 0);
}


void m68k_op_sge_8_ai(void)
{
	m68ki_write_8(EA_AY_AI_8(), COND_GE() ? 0xff : 0);
}


void m68k_op_sge_8_pi(void)
{
	m68ki_write_8(EA_AY_PI_8(), COND_GE() ? 0xff : 0);
}


void m68k_op_sge_8_pi7(void)
{
	m68ki_write_8(EA_A7_PI_8(), COND_GE() ? 0xff : 0);
}


void m68k_op_sge_8_pd(void)
{
	m68ki_write_8(EA_AY_PD_8(), COND_GE() ? 0xff : 0);
}


void m68k_op_sge_8_pd7(void)
{
	m68ki_write_8(EA_A7_PD_8(), COND_GE() ? 0xff : 0);
}


void m68k_op_sge_8_di(void)
{
	m68ki_write_8(EA_AY_DI_8(), COND_GE() ? 0xff : 0);
}


void m68k_op_sge_8_ix(void)
{
	m68ki_write_8(EA_AY_IX_8(), COND_GE() ? 0xff : 0);
}


void m68k_op_sge_8_aw(void)
{
	m68ki_write_8(EA_AW_8(), COND_GE() ? 0xff : 0);
}


void m68k_op_sge_8_al(void)
{
	m68ki_write_8(EA_AL_8(), COND_GE() ? 0xff : 0);
}


void m68k_op_slt_8_ai(void)
{
	m68ki_write_8(EA_AY_AI_8(), COND_LT() ? 0xff : 0);
}


void m68k_op_slt_8_pi(void)
{
	m68ki_write_8(EA_AY_PI_8(), COND_LT() ? 0xff : 0);
}


void m68k_op_slt_8_pi7(void)
{
	m68ki_write_8(EA_A7_PI_8(), COND_LT() ? 0xff : 0);
}


void m68k_op_slt_8_pd(void)
{
	m68ki_write_8(EA_AY_PD_8(), COND_LT() ? 0xff : 0);
}


void m68k_op_slt_8_pd7(void)
{
	m68ki_write_8(EA_A7_PD_8(), COND_LT() ? 0xff : 0);
}


void m68k_op_slt_8_di(void)
{
	m68ki_write_8(EA_AY_DI_8(), COND_LT() ? 0xff : 0);
}


void m68k_op_slt_8_ix(void)
{
	m68ki_write_8(EA_AY_IX_8(), COND_LT() ? 0xff : 0);
}


void m68k_op_slt_8_aw(void)
{
	m68ki_write_8(EA_AW_8(), COND_LT() ? 0xff : 0);
}


void m68k_op_slt_8_al(void)
{
	m68ki_write_8(EA_AL_8(), COND_LT() ? 0xff : 0);
}


void m68k_op_sgt_8_ai(void)
{
	m68ki_write_8(EA_AY_AI_8(), COND_GT() ? 0xff : 0);
}


void m68k_op_sgt_8_pi(void)
{
	m68ki_write_8(EA_AY_PI_8(), COND_GT() ? 0xff : 0);
}


void m68k_op_sgt_8_pi7(void)
{
	m68ki_write_8(EA_A7_PI_8(), COND_GT() ? 0xff : 0);
}


void m68k_op_sgt_8_pd(void)
{
	m68ki_write_8(EA_AY_PD_8(), COND_GT() ? 0xff : 0);
}


void m68k_op_sgt_8_pd7(void)
{
	m68ki_write_8(EA_A7_PD_8(), COND_GT() ? 0xff : 0);
}


void m68k_op_sgt_8_di(void)
{
	m68ki_write_8(EA_AY_DI_8(), COND_GT() ? 0xff : 0);
}


void m68k_op_sgt_8_ix(void)
{
	m68ki_write_8(EA_AY_IX_8(), COND_GT() ? 0xff : 0);
}


void m68k_op_sgt_8_aw(void)
{
	m68ki_write_8(EA_AW_8(), COND_GT() ? 0xff : 0);
}


void m68k_op_sgt_8_al(void)
{
	m68ki_write_8(EA_AL_8(), COND_GT() ? 0xff : 0);
}


void m68k_op_sle_8_ai(void)
{
	m68ki_write_8(EA_AY_AI_8(), COND_LE() ? 0xff : 0);
}


void m68k_op_sle_8_pi(void)
{
	m68ki_write_8(EA_AY_PI_8(), COND_LE() ? 0xff : 0);
}


void m68k_op_sle_8_pi7(void)
{
	m68ki_write_8(EA_A7_PI_8(), COND_LE() ? 0xff : 0);
}


void m68k_op_sle_8_pd(void)
{
	m68ki_write_8(EA_AY_PD_8(), COND_LE() ? 0xff : 0);
}


void m68k_op_sle_8_pd7(void)
{
	m68ki_write_8(EA_A7_PD_8(), COND_LE() ? 0xff : 0);
}


void m68k_op_sle_8_di(void)
{
	m68ki_write_8(EA_AY_DI_8(), COND_LE() ? 0xff : 0);
}


void m68k_op_sle_8_ix(void)
{
	m68ki_write_8(EA_AY_IX_8(), COND_LE() ? 0xff : 0);
}


void m68k_op_sle_8_aw(void)
{
	m68ki_write_8(EA_AW_8(), COND_LE() ? 0xff : 0);
}


void m68k_op_sle_8_al(void)
{
	m68ki_write_8(EA_AL_8(), COND_LE() ? 0xff : 0);
}


void m68k_op_stop(void)
{
	if(FLAG_S)
	{
		uint new_sr = OPER_I_16();
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		CPU_STOPPED |= STOP_LEVEL_STOP;
		m68ki_set_sr(new_sr);
		if(m68ki_remaining_cycles >= CYC_INSTRUCTION[REG_IR])
			m68ki_remaining_cycles = CYC_INSTRUCTION[REG_IR];
		else
			USE_ALL_CYCLES();
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_sub_8_er_d(void)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_8(DY);
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_sub_8_er_ai(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_AI_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_sub_8_er_pi(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_PI_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_sub_8_er_pi7(void)
{
	uint* r_dst = &DX;
	uint src = OPER_A7_PI_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_sub_8_er_pd(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_PD_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_sub_8_er_pd7(void)
{
	uint* r_dst = &DX;
	uint src = OPER_A7_PD_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_sub_8_er_di(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_DI_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_sub_8_er_ix(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_IX_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_sub_8_er_aw(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AW_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_sub_8_er_al(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AL_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_sub_8_er_pcdi(void)
{
	uint* r_dst = &DX;
	uint src = OPER_PCDI_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_sub_8_er_pcix(void)
{
	uint* r_dst = &DX;
	uint src = OPER_PCIX_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_sub_8_er_i(void)
{
	uint* r_dst = &DX;
	uint src = OPER_I_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_sub_16_er_d(void)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_16(DY);
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_sub_16_er_a(void)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_16(AY);
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_sub_16_er_ai(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_AI_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_sub_16_er_pi(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_PI_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_sub_16_er_pd(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_PD_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_sub_16_er_di(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_DI_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_sub_16_er_ix(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_IX_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_sub_16_er_aw(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AW_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_sub_16_er_al(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AL_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_sub_16_er_pcdi(void)
{
	uint* r_dst = &DX;
	uint src = OPER_PCDI_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_sub_16_er_pcix(void)
{
	uint* r_dst = &DX;
	uint src = OPER_PCIX_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_sub_16_er_i(void)
{
	uint* r_dst = &DX;
	uint src = OPER_I_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_sub_32_er_d(void)
{
	uint* r_dst = &DX;
	uint src = DY;
	uint dst = *r_dst;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_sub_32_er_a(void)
{
	uint* r_dst = &DX;
	uint src = AY;
	uint dst = *r_dst;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_sub_32_er_ai(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_AI_32();
	uint dst = *r_dst;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_sub_32_er_pi(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_PI_32();
	uint dst = *r_dst;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_sub_32_er_pd(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_PD_32();
	uint dst = *r_dst;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_sub_32_er_di(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_DI_32();
	uint dst = *r_dst;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_sub_32_er_ix(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_IX_32();
	uint dst = *r_dst;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_sub_32_er_aw(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AW_32();
	uint dst = *r_dst;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_sub_32_er_al(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AL_32();
	uint dst = *r_dst;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_sub_32_er_pcdi(void)
{
	uint* r_dst = &DX;
	uint src = OPER_PCDI_32();
	uint dst = *r_dst;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_sub_32_er_pcix(void)
{
	uint* r_dst = &DX;
	uint src = OPER_PCIX_32();
	uint dst = *r_dst;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_sub_32_er_i(void)
{
	uint* r_dst = &DX;
	uint src = OPER_I_32();
	uint dst = *r_dst;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_sub_8_re_ai(void)
{
	uint ea = EA_AY_AI_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_sub_8_re_pi(void)
{
	uint ea = EA_AY_PI_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_sub_8_re_pi7(void)
{
	uint ea = EA_A7_PI_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_sub_8_re_pd(void)
{
	uint ea = EA_AY_PD_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_sub_8_re_pd7(void)
{
	uint ea = EA_A7_PD_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_sub_8_re_di(void)
{
	uint ea = EA_AY_DI_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_sub_8_re_ix(void)
{
	uint ea = EA_AY_IX_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_sub_8_re_aw(void)
{
	uint ea = EA_AW_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_sub_8_re_al(void)
{
	uint ea = EA_AL_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_sub_16_re_ai(void)
{
	uint ea = EA_AY_AI_16();
	uint src = MASK_OUT_ABOVE_16(DX);
	uint dst = m68ki_read_16(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_sub_16_re_pi(void)
{
	uint ea = EA_AY_PI_16();
	uint src = MASK_OUT_ABOVE_16(DX);
	uint dst = m68ki_read_16(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_sub_16_re_pd(void)
{
	uint ea = EA_AY_PD_16();
	uint src = MASK_OUT_ABOVE_16(DX);
	uint dst = m68ki_read_16(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_sub_16_re_di(void)
{
	uint ea = EA_AY_DI_16();
	uint src = MASK_OUT_ABOVE_16(DX);
	uint dst = m68ki_read_16(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_sub_16_re_ix(void)
{
	uint ea = EA_AY_IX_16();
	uint src = MASK_OUT_ABOVE_16(DX);
	uint dst = m68ki_read_16(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_sub_16_re_aw(void)
{
	uint ea = EA_AW_16();
	uint src = MASK_OUT_ABOVE_16(DX);
	uint dst = m68ki_read_16(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_sub_16_re_al(void)
{
	uint ea = EA_AL_16();
	uint src = MASK_OUT_ABOVE_16(DX);
	uint dst = m68ki_read_16(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_sub_32_re_ai(void)
{
	uint ea = EA_AY_AI_32();
	uint src = DX;
	uint dst = m68ki_read_32(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_sub_32_re_pi(void)
{
	uint ea = EA_AY_PI_32();
	uint src = DX;
	uint dst = m68ki_read_32(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_sub_32_re_pd(void)
{
	uint ea = EA_AY_PD_32();
	uint src = DX;
	uint dst = m68ki_read_32(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_sub_32_re_di(void)
{
	uint ea = EA_AY_DI_32();
	uint src = DX;
	uint dst = m68ki_read_32(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_sub_32_re_ix(void)
{
	uint ea = EA_AY_IX_32();
	uint src = DX;
	uint dst = m68ki_read_32(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_sub_32_re_aw(void)
{
	uint ea = EA_AW_32();
	uint src = DX;
	uint dst = m68ki_read_32(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_sub_32_re_al(void)
{
	uint ea = EA_AL_32();
	uint src = DX;
	uint dst = m68ki_read_32(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_suba_16_d(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - MAKE_INT_16(DY));
}


void m68k_op_suba_16_a(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - MAKE_INT_16(AY));
}


void m68k_op_suba_16_ai(void)
{
	uint* r_dst = &AX;
	uint src = MAKE_INT_16(OPER_AY_AI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68k_op_suba_16_pi(void)
{
	uint* r_dst = &AX;
	uint src = MAKE_INT_16(OPER_AY_PI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68k_op_suba_16_pd(void)
{
	uint* r_dst = &AX;
	uint src = MAKE_INT_16(OPER_AY_PD_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68k_op_suba_16_di(void)
{
	uint* r_dst = &AX;
	uint src = MAKE_INT_16(OPER_AY_DI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68k_op_suba_16_ix(void)
{
	uint* r_dst = &AX;
	uint src = MAKE_INT_16(OPER_AY_IX_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68k_op_suba_16_aw(void)
{
	uint* r_dst = &AX;
	uint src = MAKE_INT_16(OPER_AW_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68k_op_suba_16_al(void)
{
	uint* r_dst = &AX;
	uint src = MAKE_INT_16(OPER_AL_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68k_op_suba_16_pcdi(void)
{
	uint* r_dst = &AX;
	uint src = MAKE_INT_16(OPER_PCDI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68k_op_suba_16_pcix(void)
{
	uint* r_dst = &AX;
	uint src = MAKE_INT_16(OPER_PCIX_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68k_op_suba_16_i(void)
{
	uint* r_dst = &AX;
	uint src = MAKE_INT_16(OPER_I_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68k_op_suba_32_d(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - DY);
}


void m68k_op_suba_32_a(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - AY);
}


void m68k_op_suba_32_ai(void)
{
	uint src = OPER_AY_AI_32();
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68k_op_suba_32_pi(void)
{
	uint src = OPER_AY_PI_32();
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68k_op_suba_32_pd(void)
{
	uint src = OPER_AY_PD_32();
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68k_op_suba_32_di(void)
{
	uint src = OPER_AY_DI_32();
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68k_op_suba_32_ix(void)
{
	uint src = OPER_AY_IX_32();
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68k_op_suba_32_aw(void)
{
	uint src = OPER_AW_32();
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68k_op_suba_32_al(void)
{
	uint src = OPER_AL_32();
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68k_op_suba_32_pcdi(void)
{
	uint src = OPER_PCDI_32();
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68k_op_suba_32_pcix(void)
{
	uint src = OPER_PCIX_32();
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68k_op_suba_32_i(void)
{
	uint src = OPER_I_32();
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68k_op_subi_8_d(void)
{
	uint* r_dst = &DY;
	uint src = OPER_I_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_subi_8_ai(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_AI_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_subi_8_pi(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_PI_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_subi_8_pi7(void)
{
	uint src = OPER_I_8();
	uint ea = EA_A7_PI_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_subi_8_pd(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_subi_8_pd7(void)
{
	uint src = OPER_I_8();
	uint ea = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_subi_8_di(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_DI_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_subi_8_ix(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_IX_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_subi_8_aw(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AW_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_subi_8_al(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AL_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_subi_16_d(void)
{
	uint* r_dst = &DY;
	uint src = OPER_I_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_subi_16_ai(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_AI_16();
	uint dst = m68ki_read_16(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_subi_16_pi(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_PI_16();
	uint dst = m68ki_read_16(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_subi_16_pd(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_PD_16();
	uint dst = m68ki_read_16(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_subi_16_di(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_DI_16();
	uint dst = m68ki_read_16(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_subi_16_ix(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_IX_16();
	uint dst = m68ki_read_16(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_subi_16_aw(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AW_16();
	uint dst = m68ki_read_16(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_subi_16_al(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AL_16();
	uint dst = m68ki_read_16(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_subi_32_d(void)
{
	uint* r_dst = &DY;
	uint src = OPER_I_32();
	uint dst = *r_dst;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	*r_dst = FLAG_Z;
}


void m68k_op_subi_32_ai(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_AI_32();
	uint dst = m68ki_read_32(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_subi_32_pi(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_PI_32();
	uint dst = m68ki_read_32(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_subi_32_pd(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_PD_32();
	uint dst = m68ki_read_32(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_subi_32_di(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_DI_32();
	uint dst = m68ki_read_32(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_subi_32_ix(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_IX_32();
	uint dst = m68ki_read_32(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_subi_32_aw(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AW_32();
	uint dst = m68ki_read_32(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_subi_32_al(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AL_32();
	uint dst = m68ki_read_32(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_subq_8_d(void)
{
	uint* r_dst = &DY;
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_subq_8_ai(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_AI_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_subq_8_pi(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_PI_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_subq_8_pi7(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_A7_PI_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_subq_8_pd(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_subq_8_pd7(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_subq_8_di(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_DI_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_subq_8_ix(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_IX_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_subq_8_aw(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AW_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_subq_8_al(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AL_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_subq_16_d(void)
{
	uint* r_dst = &DY;
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_subq_16_a(void)
{
	uint* r_dst = &AY;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - ((((REG_IR >> 9) - 1) & 7) + 1));
}


void m68k_op_subq_16_ai(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_AI_16();
	uint dst = m68ki_read_16(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_subq_16_pi(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_PI_16();
	uint dst = m68ki_read_16(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_subq_16_pd(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_PD_16();
	uint dst = m68ki_read_16(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_subq_16_di(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_DI_16();
	uint dst = m68ki_read_16(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_subq_16_ix(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_IX_16();
	uint dst = m68ki_read_16(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_subq_16_aw(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AW_16();
	uint dst = m68ki_read_16(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_subq_16_al(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AL_16();
	uint dst = m68ki_read_16(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_subq_32_d(void)
{
	uint* r_dst = &DY;
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint dst = *r_dst;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	*r_dst = FLAG_Z;
}


void m68k_op_subq_32_a(void)
{
	uint* r_dst = &AY;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - ((((REG_IR >> 9) - 1) & 7) + 1));
}


void m68k_op_subq_32_ai(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_AI_32();
	uint dst = m68ki_read_32(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_subq_32_pi(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_PI_32();
	uint dst = m68ki_read_32(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_subq_32_pd(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_PD_32();
	uint dst = m68ki_read_32(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_subq_32_di(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_DI_32();
	uint dst = m68ki_read_32(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_subq_32_ix(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_IX_32();
	uint dst = m68ki_read_32(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_subq_32_aw(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AW_32();
	uint dst = m68ki_read_32(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_subq_32_al(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AL_32();
	uint dst = m68ki_read_32(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_subx_8_rr(void)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_8(DY);
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = dst - src - XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
}


void m68k_op_subx_16_rr(void)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_16(DY);
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = dst - src - XFLAG_AS_1();

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	res = MASK_OUT_ABOVE_16(res);
	FLAG_Z |= res;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
}


void m68k_op_subx_32_rr(void)
{
	uint* r_dst = &DX;
	uint src = DY;
	uint dst = *r_dst;
	uint res = dst - src - XFLAG_AS_1();

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	res = MASK_OUT_ABOVE_32(res);
	FLAG_Z |= res;

	*r_dst = res;
}


void m68k_op_subx_8_mm_ax7(void)
{
	uint src = OPER_AY_PD_8();
	uint ea  = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src - XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_subx_8_mm_ay7(void)
{
	uint src = OPER_A7_PD_8();
	uint ea  = EA_AX_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src - XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_subx_8_mm_axy7(void)
{
	uint src = OPER_A7_PD_8();
	uint ea  = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src - XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_subx_8_mm(void)
{
	uint src = OPER_AY_PD_8();
	uint ea  = EA_AX_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src - XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_subx_16_mm(void)
{
	uint src = OPER_AY_PD_16();
	uint ea  = EA_AX_PD_16();
	uint dst = m68ki_read_16(ea);
	uint res = dst - src - XFLAG_AS_1();

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	res = MASK_OUT_ABOVE_16(res);
	FLAG_Z |= res;

	m68ki_write_16(ea, res);
}


void m68k_op_subx_32_mm(void)
{
	uint src = OPER_AY_PD_32();
	uint ea  = EA_AX_PD_32();
	uint dst = m68ki_read_32(ea);
	uint res = dst - src - XFLAG_AS_1();

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	res = MASK_OUT_ABOVE_32(res);
	FLAG_Z |= res;

	m68ki_write_32(ea, res);
}


void m68k_op_swap_32(void)
{
	uint* r_dst = &DY;

	FLAG_Z = MASK_OUT_ABOVE_32(*r_dst<<16);
	*r_dst = (*r_dst>>16) | FLAG_Z;

	FLAG_Z = *r_dst;
	FLAG_N = NFLAG_32(*r_dst);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_tas_8_d(void)
{
	uint* r_dst = &DY;

	FLAG_Z = MASK_OUT_ABOVE_8(*r_dst);
	FLAG_N = NFLAG_8(*r_dst);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	*r_dst |= 0x80;
}


void m68k_op_tas_8_ai(void)
{
	uint ea = EA_AY_AI_8();
	uint dst = m68ki_read_8(ea);
	uint allow_writeback;

	FLAG_Z = dst;
	FLAG_N = NFLAG_8(dst);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;

	/* The Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS writeback
       disabled in order to function properly.  Some Amiga software may also rely
       on this, but only when accessing specific addresses so additional functionality
       will be needed. */
	allow_writeback = m68ki_tas_callback();

	if (allow_writeback==1) m68ki_write_8(ea, dst | 0x80);
}


void m68k_op_tas_8_pi(void)
{
	uint ea = EA_AY_PI_8();
	uint dst = m68ki_read_8(ea);
	uint allow_writeback;

	FLAG_Z = dst;
	FLAG_N = NFLAG_8(dst);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;

	/* The Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS writeback
       disabled in order to function properly.  Some Amiga software may also rely
       on this, but only when accessing specific addresses so additional functionality
       will be needed. */
	allow_writeback = m68ki_tas_callback();

	if (allow_writeback==1) m68ki_write_8(ea, dst | 0x80);
}


void m68k_op_tas_8_pi7(void)
{
	uint ea = EA_A7_PI_8();
	uint dst = m68ki_read_8(ea);
	uint allow_writeback;

	FLAG_Z = dst;
	FLAG_N = NFLAG_8(dst);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;

	/* The Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS writeback
       disabled in order to function properly.  Some Amiga software may also rely
       on this, but only when accessing specific addresses so additional functionality
       will be needed. */
	allow_writeback = m68ki_tas_callback();

	if (allow_writeback==1) m68ki_write_8(ea, dst | 0x80);
}


void m68k_op_tas_8_pd(void)
{
	uint ea = EA_AY_PD_8();
	uint dst = m68ki_read_8(ea);
	uint allow_writeback;

	FLAG_Z = dst;
	FLAG_N = NFLAG_8(dst);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;

	/* The Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS writeback
       disabled in order to function properly.  Some Amiga software may also rely
       on this, but only when accessing specific addresses so additional functionality
       will be needed. */
	allow_writeback = m68ki_tas_callback();

	if (allow_writeback==1) m68ki_write_8(ea, dst | 0x80);
}


void m68k_op_tas_8_pd7(void)
{
	uint ea = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint allow_writeback;

	FLAG_Z = dst;
	FLAG_N = NFLAG_8(dst);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;

	/* The Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS writeback
       disabled in order to function properly.  Some Amiga software may also rely
       on this, but only when accessing specific addresses so additional functionality
       will be needed. */
	allow_writeback = m68ki_tas_callback();

	if (allow_writeback==1) m68ki_write_8(ea, dst | 0x80);
}


void m68k_op_tas_8_di(void)
{
	uint ea = EA_AY_DI_8();
	uint dst = m68ki_read_8(ea);
	uint allow_writeback;

	FLAG_Z = dst;
	FLAG_N = NFLAG_8(dst);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;

	/* The Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS writeback
       disabled in order to function properly.  Some Amiga software may also rely
       on this, but only when accessing specific addresses so additional functionality
       will be needed. */
	allow_writeback = m68ki_tas_callback();

	if (allow_writeback==1) m68ki_write_8(ea, dst | 0x80);
}


void m68k_op_tas_8_ix(void)
{
	uint ea = EA_AY_IX_8();
	uint dst = m68ki_read_8(ea);
	uint allow_writeback;

	FLAG_Z = dst;
	FLAG_N = NFLAG_8(dst);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;

	/* The Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS writeback
       disabled in order to function properly.  Some Amiga software may also rely
       on this, but only when accessing specific addresses so additional functionality
       will be needed. */
	allow_writeback = m68ki_tas_callback();

	if (allow_writeback==1) m68ki_write_8(ea, dst | 0x80);
}


void m68k_op_tas_8_aw(void)
{
	uint ea = EA_AW_8();
	uint dst = m68ki_read_8(ea);
	uint allow_writeback;

	FLAG_Z = dst;
	FLAG_N = NFLAG_8(dst);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;

	/* The Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS writeback
       disabled in order to function properly.  Some Amiga software may also rely
       on this, but only when accessing specific addresses so additional functionality
       will be needed. */
	allow_writeback = m68ki_tas_callback();

	if (allow_writeback==1) m68ki_write_8(ea, dst | 0x80);
}


void m68k_op_tas_8_al(void)
{
	uint ea = EA_AL_8();
	uint dst = m68ki_read_8(ea);
	uint allow_writeback;

	FLAG_Z = dst;
	FLAG_N = NFLAG_8(dst);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;

	/* The Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS writeback
       disabled in order to function properly.  Some Amiga software may also rely
       on this, but only when accessing specific addresses so additional functionality
       will be needed. */
	allow_writeback = m68ki_tas_callback();

	if (allow_writeback==1) m68ki_write_8(ea, dst | 0x80);
}


void m68k_op_trap(void)
{
	/* Trap#n stacks exception frame type 0 */
	m68ki_exception_trapN(EXCEPTION_TRAP_BASE + (REG_IR & 0xf));	/* HJB 990403 */
}


void m68k_op_trapt(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapt_16(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapt_32(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapf(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapf_16(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapf_32(void)
{

	m68ki_exception_illegal();
}


void m68k_op_traphi(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapls(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapcc(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapcs(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapne(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapeq(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapvc(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapvs(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trappl(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapmi(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapge(void)
{

	m68ki_exception_illegal();
}


void m68k_op_traplt(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapgt(void)
{

	m68ki_exception_illegal();
}


void m68k_op_traple(void)
{

	m68ki_exception_illegal();
}


void m68k_op_traphi_16(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapls_16(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapcc_16(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapcs_16(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapne_16(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapeq_16(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapvc_16(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapvs_16(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trappl_16(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapmi_16(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapge_16(void)
{

	m68ki_exception_illegal();
}


void m68k_op_traplt_16(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapgt_16(void)
{

	m68ki_exception_illegal();
}


void m68k_op_traple_16(void)
{

	m68ki_exception_illegal();
}


void m68k_op_traphi_32(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapls_32(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapcc_32(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapcs_32(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapne_32(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapeq_32(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapvc_32(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapvs_32(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trappl_32(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapmi_32(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapge_32(void)
{

	m68ki_exception_illegal();
}


void m68k_op_traplt_32(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapgt_32(void)
{

	m68ki_exception_illegal();
}


void m68k_op_traple_32(void)
{

	m68ki_exception_illegal();
}


void m68k_op_trapv(void)
{
	if(COND_VC())
	{
		return;
	}
	m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
}


void m68k_op_tst_8_d(void)
{
	uint res = MASK_OUT_ABOVE_8(DY);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_8_ai(void)
{
	uint res = OPER_AY_AI_8();

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_8_pi(void)
{
	uint res = OPER_AY_PI_8();

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_8_pi7(void)
{
	uint res = OPER_A7_PI_8();

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_8_pd(void)
{
	uint res = OPER_AY_PD_8();

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_8_pd7(void)
{
	uint res = OPER_A7_PD_8();

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_8_di(void)
{
	uint res = OPER_AY_DI_8();

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_8_ix(void)
{
	uint res = OPER_AY_IX_8();

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_8_aw(void)
{
	uint res = OPER_AW_8();

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_8_al(void)
{
	uint res = OPER_AL_8();

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_8_pcdi(void)
{

	m68ki_exception_illegal();
}


void m68k_op_tst_8_pcix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_tst_8_i(void)
{

	m68ki_exception_illegal();
}


void m68k_op_tst_16_d(void)
{
	uint res = MASK_OUT_ABOVE_16(DY);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_16_a(void)
{

	m68ki_exception_illegal();
}


void m68k_op_tst_16_ai(void)
{
	uint res = OPER_AY_AI_16();

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_16_pi(void)
{
	uint res = OPER_AY_PI_16();

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_16_pd(void)
{
	uint res = OPER_AY_PD_16();

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_16_di(void)
{
	uint res = OPER_AY_DI_16();

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_16_ix(void)
{
	uint res = OPER_AY_IX_16();

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_16_aw(void)
{
	uint res = OPER_AW_16();

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_16_al(void)
{
	uint res = OPER_AL_16();

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_16_pcdi(void)
{

	m68ki_exception_illegal();
}


void m68k_op_tst_16_pcix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_tst_16_i(void)
{

	m68ki_exception_illegal();
}


void m68k_op_tst_32_d(void)
{
	uint res = DY;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_32_a(void)
{

	m68ki_exception_illegal();
}


void m68k_op_tst_32_ai(void)
{
	uint res = OPER_AY_AI_32();

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_32_pi(void)
{
	uint res = OPER_AY_PI_32();

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_32_pd(void)
{
	uint res = OPER_AY_PD_32();

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_32_di(void)
{
	uint res = OPER_AY_DI_32();

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_32_ix(void)
{
	uint res = OPER_AY_IX_32();

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_32_aw(void)
{
	uint res = OPER_AW_32();

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_32_al(void)
{
	uint res = OPER_AL_32();

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


void m68k_op_tst_32_pcdi(void)
{
	if(0)
	{
		uint res = OPER_PCDI_32();

		FLAG_N = NFLAG_32(res);
		FLAG_Z = res;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_tst_32_pcix(void)
{

	m68ki_exception_illegal();
}


void m68k_op_tst_32_i(void)
{

	m68ki_exception_illegal();
}


void m68k_op_unlk_32_a7(void)
{
	REG_A[7] = m68ki_read_32(REG_A[7]);
}


void m68k_op_unlk_32(void)
{
	uint* r_dst = &AY;

	REG_A[7] = *r_dst;
	*r_dst = m68ki_pull_32();
}


void m68k_op_unpk_16_rr(void)
{

	m68ki_exception_illegal();
}


void m68k_op_unpk_16_mm_ax7(void)
{

	m68ki_exception_illegal();
}


void m68k_op_unpk_16_mm_ay7(void)
{

	m68ki_exception_illegal();
}


void m68k_op_unpk_16_mm_axy7(void)
{

	m68ki_exception_illegal();
}


void m68k_op_unpk_16_mm(void)
{

	m68ki_exception_illegal();
}


/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */


