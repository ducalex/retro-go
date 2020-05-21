/*
** Nofrendo (c) 1998-2000 Matthew Conte (matt@conte.com)
**
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of version 2 of the GNU Library General
** Public License as published by the Free Software Foundation.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.  To obtain a
** copy of the GNU Library General Public License, write to the Free
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
**
**
** nes6502.c
**
** NES custom 6502 (2A03) CPU implementation
** $Id: nes6502.c,v 1.2 2001/04/27 14:37:11 neil Exp $
*/

#include <nofrendo.h>
#include <esp_attr.h>
#include <string.h>
#include "nes6502.h"
#include "dis6502.h"

/* internal CPU context */
static nes6502_t cpu;
static int remaining_cycles = 0;
static mem_t *mem;

#define NES6502_JUMPTABLE
#define NES6502_FASTMEM


#define ADD_CYCLES(x) \
{ \
   remaining_cycles -= (x); \
   cpu.total_cycles += (x); \
}

/*
** Check to see if an index reg addition overflowed to next page
*/
#define PAGE_CROSS_CHECK(addr, reg) \
{ \
   if ((reg) > (uint8) (addr)) \
      ADD_CYCLES(1); \
}

#define EMPTY_READ(value)  /* empty */

/*
** Addressing mode macros
*/

/* Immediate */
#define IMMEDIATE_BYTE(value) \
{ \
   value = fast_readbyte(PC++); \
}

/* Absolute */
#define ABSOLUTE_ADDR(address) \
{ \
   address = fast_readword(PC); \
   PC += 2; \
}

#define ABSOLUTE(address, value) \
{ \
   ABSOLUTE_ADDR(address); \
   value = readbyte(address); \
}

#define ABSOLUTE_BYTE(value) \
{ \
   ABSOLUTE(temp, value); \
}

/* Absolute indexed X */
#define ABS_IND_X_ADDR(address) \
{ \
   ABSOLUTE_ADDR(address); \
   address = (address + X) & 0xFFFF; \
}

#define ABS_IND_X(address, value) \
{ \
   ABS_IND_X_ADDR(address); \
   value = readbyte(address); \
}

#define ABS_IND_X_BYTE(value) \
{ \
   ABS_IND_X(temp, value); \
}

/* special page-cross check version for read instructions */
#define ABS_IND_X_BYTE_READ(value) \
{ \
   ABS_IND_X_ADDR(temp); \
   PAGE_CROSS_CHECK(temp, X); \
   value = readbyte(temp); \
}

/* Absolute indexed Y */
#define ABS_IND_Y_ADDR(address) \
{ \
   ABSOLUTE_ADDR(address); \
   address = (address + Y) & 0xFFFF; \
}

#define ABS_IND_Y(address, value) \
{ \
   ABS_IND_Y_ADDR(address); \
   value = readbyte(address); \
}

#define ABS_IND_Y_BYTE(value) \
{ \
   ABS_IND_Y(temp, value); \
}

/* special page-cross check version for read instructions */
#define ABS_IND_Y_BYTE_READ(value) \
{ \
   ABS_IND_Y_ADDR(temp); \
   PAGE_CROSS_CHECK(temp, Y); \
   value = readbyte(temp); \
}

/* Zero-page */
#define ZERO_PAGE_ADDR(address) \
{ \
   IMMEDIATE_BYTE(address); \
}

#define ZERO_PAGE(address, value) \
{ \
   ZERO_PAGE_ADDR(address); \
   value = ZP_READBYTE(address); \
}

#define ZERO_PAGE_BYTE(value) \
{ \
   ZERO_PAGE(btemp, value); \
}

/* Zero-page indexed X */
#define ZP_IND_X_ADDR(address) \
{ \
   ZERO_PAGE_ADDR(address); \
   address += X; \
}

#define ZP_IND_X(address, value) \
{ \
   ZP_IND_X_ADDR(address); \
   value = ZP_READBYTE(address); \
}

#define ZP_IND_X_BYTE(value) \
{ \
   ZP_IND_X(btemp, value); \
}

/* Zero-page indexed Y */
/* Not really an adressing mode, just for LDx/STx */
#define ZP_IND_Y_ADDR(address) \
{ \
   ZERO_PAGE_ADDR(address); \
   address += Y; \
}

#define ZP_IND_Y_BYTE(value) \
{ \
   ZP_IND_Y_ADDR(btemp); \
   value = ZP_READBYTE(btemp); \
}

/* Indexed indirect */
#define INDIR_X_ADDR(address) \
{ \
   ZERO_PAGE_ADDR(btemp); \
   btemp += X; \
   address = ZP_READWORD(btemp); \
}

#define INDIR_X(address, value) \
{ \
   INDIR_X_ADDR(address); \
   value = readbyte(address); \
}

#define INDIR_X_BYTE(value) \
{ \
   INDIR_X(temp, value); \
}

/* Indirect indexed */
#define INDIR_Y_ADDR(address) \
{ \
   ZERO_PAGE_ADDR(btemp); \
   address = (ZP_READWORD(btemp) + Y) & 0xFFFF; \
}

#define INDIR_Y(address, value) \
{ \
   INDIR_Y_ADDR(address); \
   value = readbyte(address); \
}

#define INDIR_Y_BYTE(value) \
{ \
   INDIR_Y(temp, value); \
}

/* special page-cross check version for read instructions */
#define INDIR_Y_BYTE_READ(value) \
{ \
   INDIR_Y_ADDR(temp); \
   PAGE_CROSS_CHECK(temp, Y); \
   value = readbyte(temp); \
}



/* Stack push/pull */
#define PUSH(value)             cpu.stack[S--] = (uint8) (value)
#define PULL()                  cpu.stack[++S]


/*
** flag register helper macros
*/

/* Theory: Z and N flags are set in just about every
** instruction, so we will just store the value in those
** flag variables, and mask out the irrelevant data when
** we need to check them (branches, etc).  This makes the
** zero flag only really be 'set' when z_flag == 0.
** The rest of the flags are stored as true booleans.
*/

/* Scatter flags to separate variables */
#define SCATTER_FLAGS(value) \
{ \
   n_flag = (value) & N_FLAG; \
   v_flag = (value) & V_FLAG; \
   b_flag = (value) & B_FLAG; \
   d_flag = (value) & D_FLAG; \
   i_flag = (value) & I_FLAG; \
   z_flag = (0 == ((value) & Z_FLAG)); \
   c_flag = (value) & C_FLAG; \
}

/* Combine flags into flag register */
#define COMBINE_FLAGS() \
( \
   (n_flag & N_FLAG) \
   | (v_flag ? V_FLAG : 0) \
   | R_FLAG \
   | (b_flag ? B_FLAG : 0) \
   | (d_flag ? D_FLAG : 0) \
   | (i_flag ? I_FLAG : 0) \
   | (z_flag ? 0 : Z_FLAG) \
   | c_flag \
)

/* Set N and Z flags based on given value */
#define SET_NZ_FLAGS(value)     n_flag = z_flag = (value);

/* For BCC, BCS, BEQ, BMI, BNE, BPL, BVC, BVS */
#define RELATIVE_BRANCH(condition) \
{ \
   if (condition) \
   { \
      IMMEDIATE_BYTE(btemp); \
      if (((int8) btemp + (PC & 0x00FF)) & 0x100) \
         ADD_CYCLES(1); \
      ADD_CYCLES(3); \
      PC += (int8) btemp; \
   } \
   else \
   { \
      PC++; \
      ADD_CYCLES(2); \
   } \
}

#define JUMP(address) \
{ \
   PC = readword(address); \
}

/*
** Interrupt macros
*/
#define NMI_PROC() \
{ \
   PUSH(PC >> 8); \
   PUSH(PC & 0xFF); \
   b_flag = 0; \
   PUSH(COMBINE_FLAGS()); \
   i_flag = 1; \
   JUMP(NMI_VECTOR); \
}

#define IRQ_PROC() \
{ \
   PUSH(PC >> 8); \
   PUSH(PC & 0xFF); \
   b_flag = 0; \
   PUSH(COMBINE_FLAGS()); \
   i_flag = 1; \
   JUMP(IRQ_VECTOR); \
}

/*
** Instruction macros
*/

/* Warning! NES CPU has no decimal mode, so by default this does no BCD! */
#define ADC(cycles, read_func) \
{ \
   read_func(data); \
   temp = A + data + c_flag; \
   c_flag = (temp >> 8) & 1; \
   v_flag = ((~(A ^ data)) & (A ^ temp) & 0x80); \
   A = (uint8) temp; \
   SET_NZ_FLAGS(A); \
   ADD_CYCLES(cycles); \
}

/* undocumented */
#define ANC(cycles, read_func) \
{ \
   read_func(data); \
   A &= data; \
   c_flag = (n_flag & N_FLAG) >> 7; \
   SET_NZ_FLAGS(A); \
   ADD_CYCLES(cycles); \
}

#define AND(cycles, read_func) \
{ \
   read_func(data); \
   A &= data; \
   SET_NZ_FLAGS(A); \
   ADD_CYCLES(cycles); \
}

/* undocumented */
#define ANE(cycles, read_func) \
{ \
   read_func(data); \
   A = (A | 0xEE) & X & data; \
   SET_NZ_FLAGS(A); \
   ADD_CYCLES(cycles); \
}

/* undocumented */
#define ARR(cycles, read_func) \
{ \
   read_func(data); \
   data &= A; \
   A = (data >> 1) | (c_flag << 7); \
   SET_NZ_FLAGS(A); \
   c_flag = (A & 0x40) >> 6; \
   v_flag = ((A >> 6) ^ (A >> 5)) & 1; \
   ADD_CYCLES(cycles); \
}

#define ASL(cycles, read_func, write_func, addr) \
{ \
   read_func(addr, data); \
   c_flag = data >> 7; \
   data <<= 1; \
   write_func(addr, data); \
   SET_NZ_FLAGS(data); \
   ADD_CYCLES(cycles); \
}

#define ASL_A() \
{ \
   c_flag = A >> 7; \
   A <<= 1; \
   SET_NZ_FLAGS(A); \
   ADD_CYCLES(2); \
}

/* undocumented */
#define ASR(cycles, read_func) \
{ \
   read_func(data); \
   data &= A; \
   c_flag = data & 1; \
   A = data >> 1; \
   SET_NZ_FLAGS(A); \
   ADD_CYCLES(cycles); \
}

#define BCC() \
{ \
   RELATIVE_BRANCH(0 == c_flag); \
}

#define BCS() \
{ \
   RELATIVE_BRANCH(0 != c_flag); \
}

#define BEQ() \
{ \
   RELATIVE_BRANCH(0 == z_flag); \
}

/* bit 7/6 of data move into N/V flags */
#define BIT(cycles, read_func) \
{ \
   read_func(data); \
   n_flag = data; \
   v_flag = data & V_FLAG; \
   z_flag = data & A; \
   ADD_CYCLES(cycles); \
}

#define BMI() \
{ \
   RELATIVE_BRANCH(n_flag & N_FLAG); \
}

#define BNE() \
{ \
   RELATIVE_BRANCH(0 != z_flag); \
}

#define BPL() \
{ \
   RELATIVE_BRANCH(0 == (n_flag & N_FLAG)); \
}

/* Software interrupt type thang */
#define BRK() \
{ \
   PC++; \
   PUSH(PC >> 8); \
   PUSH(PC & 0xFF); \
   b_flag = 1; \
   PUSH(COMBINE_FLAGS()); \
   i_flag = 1; \
   JUMP(IRQ_VECTOR); \
   ADD_CYCLES(7); \
}

#define BVC() \
{ \
   RELATIVE_BRANCH(0 == v_flag); \
}

#define BVS() \
{ \
   RELATIVE_BRANCH(0 != v_flag); \
}

#define CLC() \
{ \
   c_flag = 0; \
   ADD_CYCLES(2); \
}

#define CLD() \
{ \
   d_flag = 0; \
   ADD_CYCLES(2); \
}

#define CLI() \
{ \
   i_flag = 0; \
   ADD_CYCLES(2); \
   if (cpu.int_pending && remaining_cycles > 0) \
   { \
      cpu.int_pending = 0; \
      IRQ_PROC(); \
      ADD_CYCLES(INT_CYCLES); \
   } \
}

#define CLV() \
{ \
   v_flag = 0; \
   ADD_CYCLES(2); \
}

/* C is clear when data > A */
#define _COMPARE(reg, value) \
{ \
   temp = (reg) - (value); \
   c_flag = ((temp & 0x100) >> 8) ^ 1; \
   SET_NZ_FLAGS((uint8) temp); \
}

#define CMP(cycles, read_func) \
{ \
   read_func(data); \
   _COMPARE(A, data); \
   ADD_CYCLES(cycles); \
}

#define CPX(cycles, read_func) \
{ \
   read_func(data); \
   _COMPARE(X, data); \
   ADD_CYCLES(cycles); \
}

#define CPY(cycles, read_func) \
{ \
   read_func(data); \
   _COMPARE(Y, data); \
   ADD_CYCLES(cycles); \
}

/* undocumented */
#define DCP(cycles, read_func, write_func, addr) \
{ \
   read_func(addr, data); \
   data--; \
   write_func(addr, data); \
   CMP(cycles, EMPTY_READ); \
}

#define DEC(cycles, read_func, write_func, addr) \
{ \
   read_func(addr, data); \
   data--; \
   write_func(addr, data); \
   SET_NZ_FLAGS(data); \
   ADD_CYCLES(cycles); \
}

#define DEX() \
{ \
   X--; \
   SET_NZ_FLAGS(X); \
   ADD_CYCLES(2); \
}

#define DEY() \
{ \
   Y--; \
   SET_NZ_FLAGS(Y); \
   ADD_CYCLES(2); \
}

/* undocumented (double-NOP) */
#define DOP(cycles) \
{ \
   PC++; \
   ADD_CYCLES(cycles); \
}

#define EOR(cycles, read_func) \
{ \
   read_func(data); \
   A ^= data; \
   SET_NZ_FLAGS(A); \
   ADD_CYCLES(cycles); \
}

#define INC(cycles, read_func, write_func, addr) \
{ \
   read_func(addr, data); \
   data++; \
   write_func(addr, data); \
   SET_NZ_FLAGS(data); \
   ADD_CYCLES(cycles); \
}

#define INX() \
{ \
   X++; \
   SET_NZ_FLAGS(X); \
   ADD_CYCLES(2); \
}

#define INY() \
{ \
   Y++; \
   SET_NZ_FLAGS(Y); \
   ADD_CYCLES(2); \
}

/* undocumented */
#define ISB(cycles, read_func, write_func, addr) \
{ \
   read_func(addr, data); \
   data++; \
   write_func(addr, data); \
   SBC(cycles, EMPTY_READ); \
}

#define JAM() \
{ \
   PC--; \
   cpu.jammed = true; \
   cpu.int_pending = 0; \
   ADD_CYCLES(2); \
   remaining_cycles = 0;\
}

#define JMP_INDIRECT() \
{ \
   temp = fast_readword(PC); \
   /* bug in crossing page boundaries */ \
   if (0xFF == (temp & 0xFF)) \
      PC = (readbyte(temp & 0xFF00) << 8) | readbyte(temp); \
   else \
      JUMP(temp); \
   ADD_CYCLES(5); \
}

#define JMP_ABSOLUTE() \
{ \
   JUMP(PC); \
   ADD_CYCLES(3); \
}

#define JSR() \
{ \
   PC++; \
   PUSH(PC >> 8); \
   PUSH(PC & 0xFF); \
   JUMP(PC - 1); \
   ADD_CYCLES(6); \
}

/* undocumented */
#define LAS(cycles, read_func) \
{ \
   read_func(data); \
   A = X = S = (S & data); \
   SET_NZ_FLAGS(A); \
   ADD_CYCLES(cycles); \
}

/* undocumented */
#define LAX(cycles, read_func) \
{ \
   read_func(A); \
   X = A; \
   SET_NZ_FLAGS(A); \
   ADD_CYCLES(cycles); \
}

#define LDA(cycles, read_func) \
{ \
   read_func(A); \
   SET_NZ_FLAGS(A); \
   ADD_CYCLES(cycles); \
}

#define LDX(cycles, read_func) \
{ \
   read_func(X); \
   SET_NZ_FLAGS(X);\
   ADD_CYCLES(cycles); \
}

#define LDY(cycles, read_func) \
{ \
   read_func(Y); \
   SET_NZ_FLAGS(Y);\
   ADD_CYCLES(cycles); \
}

#define LSR(cycles, read_func, write_func, addr) \
{ \
   read_func(addr, data); \
   c_flag = data & 1; \
   data >>= 1; \
   write_func(addr, data); \
   SET_NZ_FLAGS(data); \
   ADD_CYCLES(cycles); \
}

#define LSR_A() \
{ \
   c_flag = A & 1; \
   A >>= 1; \
   SET_NZ_FLAGS(A); \
   ADD_CYCLES(2); \
}

/* undocumented */
#define LXA(cycles, read_func) \
{ \
   read_func(data); \
   A = X = ((A | 0xEE) & data); \
   SET_NZ_FLAGS(A); \
   ADD_CYCLES(cycles); \
}

#define NOP() \
{ \
   ADD_CYCLES(2); \
}

#define ORA(cycles, read_func) \
{ \
   read_func(data); \
   A |= data; \
   SET_NZ_FLAGS(A);\
   ADD_CYCLES(cycles); \
}

#define PHA() \
{ \
   PUSH(A); \
   ADD_CYCLES(3); \
}

#define PHP() \
{ \
   /* B flag is pushed on stack as well */ \
   PUSH(COMBINE_FLAGS() | B_FLAG); \
   ADD_CYCLES(3); \
}

#define PLA() \
{ \
   A = PULL(); \
   SET_NZ_FLAGS(A); \
   ADD_CYCLES(4); \
}

#define PLP() \
{ \
   btemp = PULL(); \
   SCATTER_FLAGS(btemp); \
   ADD_CYCLES(4); \
}

/* undocumented */
#define RLA(cycles, read_func, write_func, addr) \
{ \
   read_func(addr, data); \
   btemp = c_flag; \
   c_flag = data >> 7; \
   data = (data << 1) | btemp; \
   write_func(addr, data); \
   A &= data; \
   SET_NZ_FLAGS(A); \
   ADD_CYCLES(cycles); \
}

/* 9-bit rotation (carry flag used for rollover) */
#define ROL(cycles, read_func, write_func, addr) \
{ \
   read_func(addr, data); \
   btemp = c_flag; \
   c_flag = data >> 7; \
   data = (data << 1) | btemp; \
   write_func(addr, data); \
   SET_NZ_FLAGS(data); \
   ADD_CYCLES(cycles); \
}

#define ROL_A() \
{ \
   btemp = c_flag; \
   c_flag = A >> 7; \
   A = (A << 1) | btemp; \
   SET_NZ_FLAGS(A); \
   ADD_CYCLES(2); \
}

#define ROR(cycles, read_func, write_func, addr) \
{ \
   read_func(addr, data); \
   btemp = c_flag << 7; \
   c_flag = data & 1; \
   data = (data >> 1) | btemp; \
   write_func(addr, data); \
   SET_NZ_FLAGS(data); \
   ADD_CYCLES(cycles); \
}

#define ROR_A() \
{ \
   btemp = c_flag << 7; \
   c_flag = A & 1; \
   A = (A >> 1) | btemp; \
   SET_NZ_FLAGS(A); \
   ADD_CYCLES(2); \
}

/* undocumented */
#define RRA(cycles, read_func, write_func, addr) \
{ \
   read_func(addr, data); \
   btemp = c_flag << 7; \
   c_flag = data & 1; \
   data = (data >> 1) | btemp; \
   write_func(addr, data); \
   ADC(cycles, EMPTY_READ); \
}

#define RTI() \
{ \
   btemp = PULL(); \
   SCATTER_FLAGS(btemp); \
   PC = PULL(); \
   PC |= PULL() << 8; \
   ADD_CYCLES(6); \
   if (0 == i_flag && cpu.int_pending && remaining_cycles > 0) \
   { \
      cpu.int_pending = 0; \
      IRQ_PROC(); \
      ADD_CYCLES(INT_CYCLES); \
   } \
}

#define RTS() \
{ \
   PC = PULL(); \
   PC = (PC | (PULL() << 8)) + 1; \
   ADD_CYCLES(6); \
}

/* undocumented */
#define SAX(cycles, read_func, write_func, addr) \
{ \
   read_func(addr); \
   data = A & X; \
   write_func(addr, data); \
   ADD_CYCLES(cycles); \
}

/* Warning! NES CPU has no decimal mode, so by default this does no BCD! */
#define SBC(cycles, read_func) \
{ \
   read_func(data); \
   temp = A - data - (c_flag ^ 1); \
   v_flag = (A ^ data) & (A ^ temp) & 0x80; \
   c_flag = ((temp >> 8) & 1) ^ 1; \
   A = (uint8) temp; \
   SET_NZ_FLAGS(A); \
   ADD_CYCLES(cycles); \
}

/* undocumented */
#define SBX(cycles, read_func) \
{ \
   read_func(data); \
   temp = (A & X) - data; \
   c_flag = ((temp >> 8) & 1) ^ 1; \
   X = temp & 0xFF; \
   SET_NZ_FLAGS(X); \
   ADD_CYCLES(cycles); \
}

#define SEC() \
{ \
   c_flag = 1; \
   ADD_CYCLES(2); \
}

#define SED() \
{ \
   d_flag = 1; \
   ADD_CYCLES(2); \
}

#define SEI() \
{ \
   i_flag = 1; \
   ADD_CYCLES(2); \
}

/* undocumented */
#define SHA(cycles, read_func, write_func, addr) \
{ \
   read_func(addr); \
   data = A & X & ((uint8) ((addr >> 8) + 1)); \
   write_func(addr, data); \
   ADD_CYCLES(cycles); \
}

/* undocumented */
#define SHS(cycles, read_func, write_func, addr) \
{ \
   read_func(addr); \
   S = A & X; \
   data = S & ((uint8) ((addr >> 8) + 1)); \
   write_func(addr, data); \
   ADD_CYCLES(cycles); \
}

/* undocumented */
#define SHX(cycles, read_func, write_func, addr) \
{ \
   read_func(addr); \
   data = X & ((uint8) ((addr >> 8) + 1)); \
   write_func(addr, data); \
   ADD_CYCLES(cycles); \
}

/* undocumented */
#define SHY(cycles, read_func, write_func, addr) \
{ \
   read_func(addr); \
   data = Y & ((uint8) ((addr >> 8 ) + 1)); \
   write_func(addr, data); \
   ADD_CYCLES(cycles); \
}

/* undocumented */
#define SLO(cycles, read_func, write_func, addr) \
{ \
   read_func(addr, data); \
   c_flag = data >> 7; \
   data <<= 1; \
   write_func(addr, data); \
   A |= data; \
   SET_NZ_FLAGS(A); \
   ADD_CYCLES(cycles); \
}

/* undocumented */
#define SRE(cycles, read_func, write_func, addr) \
{ \
   read_func(addr, data); \
   c_flag = data & 1; \
   data >>= 1; \
   write_func(addr, data); \
   A ^= data; \
   SET_NZ_FLAGS(A); \
   ADD_CYCLES(cycles); \
}

#define STA(cycles, read_func, write_func, addr) \
{ \
   read_func(addr); \
   write_func(addr, A); \
   ADD_CYCLES(cycles); \
}

#define STX(cycles, read_func, write_func, addr) \
{ \
   read_func(addr); \
   write_func(addr, X); \
   ADD_CYCLES(cycles); \
}

#define STY(cycles, read_func, write_func, addr) \
{ \
   read_func(addr); \
   write_func(addr, Y); \
   ADD_CYCLES(cycles); \
}

#define TAX() \
{ \
   X = A; \
   SET_NZ_FLAGS(X);\
   ADD_CYCLES(2); \
}

#define TAY() \
{ \
   Y = A; \
   SET_NZ_FLAGS(Y);\
   ADD_CYCLES(2); \
}

/* undocumented (triple-NOP) */
#define TOP() \
{ \
   PC += 2; \
   ADD_CYCLES(4); \
}

#define TSX() \
{ \
   X = S; \
   SET_NZ_FLAGS(X);\
   ADD_CYCLES(2); \
}

#define TXA() \
{ \
   A = X; \
   SET_NZ_FLAGS(A);\
   ADD_CYCLES(2); \
}

#define TXS() \
{ \
   S = X; \
   ADD_CYCLES(2); \
}

#define TYA() \
{ \
   A = Y; \
   SET_NZ_FLAGS(A); \
   ADD_CYCLES(2); \
}

#define DECLARE_LOCAL_REGS() \
   uint32 PC; \
   uint8 A, X, Y, S; \
   uint8 n_flag, v_flag, b_flag; \
   uint8 d_flag, i_flag, z_flag, c_flag;

#define GET_GLOBAL_REGS() \
{ \
   PC = cpu.pc_reg; \
   A = cpu.a_reg; \
   X = cpu.x_reg; \
   Y = cpu.y_reg; \
   SCATTER_FLAGS(cpu.p_reg); \
   S = cpu.s_reg; \
}

#define STORE_LOCAL_REGS() \
{ \
   cpu.pc_reg = PC; \
   cpu.a_reg = A; \
   cpu.x_reg = X; \
   cpu.y_reg = Y; \
   cpu.p_reg = COMBINE_FLAGS(); \
   cpu.s_reg = S; \
}

/*
** Zero-page helper macros
*/

#define ZP_READBYTE(addr)          (cpu.zp[(addr)])
#define ZP_WRITEBYTE(addr, value)  (cpu.zp[(addr)] = (uint8) (value))
#define ZP_READWORD(addr)          (PAGE_READWORD(cpu.zp, addr))

/*
** Memory-related macros
*/

#ifdef HOST_LITTLE_ENDIAN
#define PAGE_READWORD(page, addr)  (*(uint16 *)((page) + (addr)))
#else
#define PAGE_READWORD(page, addr)  (page[(addr) + 1] << 8 | page[(addr)])
#endif

#define readword(a) mem_getword(a)
#define readbyte(a) mem_getbyte(a)

/*
** Middle man for faster albeit unsafe/innacurate/unchecked memory access.
** Used only when address = PC, which is always a valid ROM access (in theory)
*/

#ifdef NES6502_FASTMEM

INLINE uint8 fast_readbyte(uint16 address)
{
   return mem->pages[address >> MEM_PAGESHIFT][address];
}

INLINE uint16 fast_readword(uint32 address)
{
   if ((address & MEM_PAGEMASK) != MEM_PAGEMASK) // Page cross
   {
      return PAGE_READWORD(mem->pages[address >> MEM_PAGESHIFT], address);
   }
   return mem_getword(address);
}

INLINE void writebyte(uint32 address, uint8 value)
{
   if (address < 0x2000) {
      mem->ram[address & 0x7FF] = value;
   } else {
      mem_putbyte(address, value);
   }
}

#else /* !NES6502_FASTMEM */

#define fast_readbyte(a) mem_getbyte(a)
#define fast_readword(a) mem_getword(a)
#define writebyte(a, v) mem_putbyte(a, v)

#endif /* !NES6502_FASTMEM */


#ifdef NES6502_DISASM
#define DISASSEMBLE MESSAGE_INFO(nes6502_disasm(PC, COMBINE_FLAGS(), A, X, Y, S));
#else
#define DISASSEMBLE
#endif


#ifdef NES6502_JUMPTABLE

#define OPCODE(xx, x...)  op##xx: x; OPCODE_NEXT
#define OPCODE_NEXT \
   if (remaining_cycles <= 0) break; \
   DISASSEMBLE; \
   goto *opcode_table[fast_readbyte(PC++)];

#else /* !NES6502_JUMPTABLE */

#define OPCODE(xx, x...)  case 0x##xx: x; break;
#define OPCODE_NEXT \
   DISASSEMBLE;  \
   switch (fast_readbyte(PC++))

#endif /* !NES6502_JUMPTABLE */

/* End of macros */


/* set the current context */
void nes6502_setcontext(nes6502_t *context)
{
   ASSERT(context);
   cpu = *context;
}

/* get the current context */
void nes6502_getcontext(nes6502_t *context)
{
   ASSERT(context);
   *context = cpu;
}

/* get number of elapsed cycles */
IRAM_ATTR uint32 nes6502_getcycles()
{
   return cpu.total_cycles;
}

/* Execute instructions until count expires
**
** Returns the number of cycles *actually* executed, which will be
** anywhere from zero to timeslice_cycles + 6
*/
IRAM_ATTR int nes6502_execute(int timeslice_cycles)
{
   int old_cycles = cpu.total_cycles;

   uint32 temp, addr; /* for macros */
   uint8 btemp, baddr; /* for macros */
   uint8 data;

   DECLARE_LOCAL_REGS();
   GET_GLOBAL_REGS();

#ifdef NES6502_JUMPTABLE

   DRAM_ATTR static const void *opcode_table[256] =
   {
      &&op00, &&op01, &&op02, &&op03, &&op04, &&op05, &&op06, &&op07,
      &&op08, &&op09, &&op0A, &&op0B, &&op0C, &&op0D, &&op0E, &&op0F,
      &&op10, &&op11, &&op12, &&op13, &&op14, &&op15, &&op16, &&op17,
      &&op18, &&op19, &&op1A, &&op1B, &&op1C, &&op1D, &&op1E, &&op1F,
      &&op20, &&op21, &&op22, &&op23, &&op24, &&op25, &&op26, &&op27,
      &&op28, &&op29, &&op2A, &&op2B, &&op2C, &&op2D, &&op2E, &&op2F,
      &&op30, &&op31, &&op32, &&op33, &&op34, &&op35, &&op36, &&op37,
      &&op38, &&op39, &&op3A, &&op3B, &&op3C, &&op3D, &&op3E, &&op3F,
      &&op40, &&op41, &&op42, &&op43, &&op44, &&op45, &&op46, &&op47,
      &&op48, &&op49, &&op4A, &&op4B, &&op4C, &&op4D, &&op4E, &&op4F,
      &&op50, &&op51, &&op52, &&op53, &&op54, &&op55, &&op56, &&op57,
      &&op58, &&op59, &&op5A, &&op5B, &&op5C, &&op5D, &&op5E, &&op5F,
      &&op60, &&op61, &&op62, &&op63, &&op64, &&op65, &&op66, &&op67,
      &&op68, &&op69, &&op6A, &&op6B, &&op6C, &&op6D, &&op6E, &&op6F,
      &&op70, &&op71, &&op72, &&op73, &&op74, &&op75, &&op76, &&op77,
      &&op78, &&op79, &&op7A, &&op7B, &&op7C, &&op7D, &&op7E, &&op7F,
      &&op80, &&op81, &&op82, &&op83, &&op84, &&op85, &&op86, &&op87,
      &&op88, &&op89, &&op8A, &&op8B, &&op8C, &&op8D, &&op8E, &&op8F,
      &&op90, &&op91, &&op92, &&op93, &&op94, &&op95, &&op96, &&op97,
      &&op98, &&op99, &&op9A, &&op9B, &&op9C, &&op9D, &&op9E, &&op9F,
      &&opA0, &&opA1, &&opA2, &&opA3, &&opA4, &&opA5, &&opA6, &&opA7,
      &&opA8, &&opA9, &&opAA, &&opAB, &&opAC, &&opAD, &&opAE, &&opAF,
      &&opB0, &&opB1, &&opB2, &&opB3, &&opB4, &&opB5, &&opB6, &&opB7,
      &&opB8, &&opB9, &&opBA, &&opBB, &&opBC, &&opBD, &&opBE, &&opBF,
      &&opC0, &&opC1, &&opC2, &&opC3, &&opC4, &&opC5, &&opC6, &&opC7,
      &&opC8, &&opC9, &&opCA, &&opCB, &&opCC, &&opCD, &&opCE, &&opCF,
      &&opD0, &&opD1, &&opD2, &&opD3, &&opD4, &&opD5, &&opD6, &&opD7,
      &&opD8, &&opD9, &&opDA, &&opDB, &&opDC, &&opDD, &&opDE, &&opDF,
      &&opE0, &&opE1, &&opE2, &&opE3, &&opE4, &&opE5, &&opE6, &&opE7,
      &&opE8, &&opE9, &&opEA, &&opEB, &&opEC, &&opED, &&opEE, &&opEF,
      &&opF0, &&opF1, &&opF2, &&opF3, &&opF4, &&opF5, &&opF6, &&opF7,
      &&opF8, &&opF9, &&opFA, &&opFB, &&opFC, &&opFD, &&opFE, &&opFF
   };

#endif /* NES6502_JUMPTABLE */

   remaining_cycles = timeslice_cycles;

   /* check for DMA cycle burning */
   if (cpu.burn_cycles && remaining_cycles > 0)
   {
      int burn_for;

      burn_for = MIN(remaining_cycles, cpu.burn_cycles);
      ADD_CYCLES(burn_for);
      cpu.burn_cycles -= burn_for;
   }

   if (0 == i_flag && cpu.int_pending && remaining_cycles > 0)
   {
      cpu.int_pending = 0;
      IRQ_PROC();
      ADD_CYCLES(INT_CYCLES);
   }

   /* Continue until we run out of cycles */
   while (remaining_cycles > 0)
   {
      /* Fetch and execute instruction */
      OPCODE_NEXT
      {
      OPCODE(00, BRK());                                          /* BRK */
      OPCODE(01, ORA(6, INDIR_X_BYTE));                           /* ORA ($nn,X) */
      OPCODE(02, JAM());                                          /* JAM */
      OPCODE(03, SLO(8, INDIR_X, writebyte, addr));               /* SLO ($nn,X) */
      OPCODE(04, DOP(3));                                         /* NOP $nn */
      OPCODE(05, ORA(3, ZERO_PAGE_BYTE));                         /* ORA $nn */
      OPCODE(06, ASL(5, ZERO_PAGE, ZP_WRITEBYTE, baddr));         /* ASL $nn */
      OPCODE(07, SLO(5, ZERO_PAGE, ZP_WRITEBYTE, baddr));         /* SLO $nn */
      OPCODE(08, PHP());                                          /* PHP */
      OPCODE(09, ORA(2, IMMEDIATE_BYTE));                         /* ORA #$nn */
      OPCODE(0A, ASL_A());                                        /* ASL A */
      OPCODE(0B, ANC(2, IMMEDIATE_BYTE));                         /* ANC #$nn */
      OPCODE(0C, TOP());                                          /* NOP $nnnn */
      OPCODE(0D, ORA(4, ABSOLUTE_BYTE));                          /* ORA $nnnn */
      OPCODE(0E, ASL(6, ABSOLUTE, writebyte, addr));              /* ASL $nnnn */
      OPCODE(0F, SLO(6, ABSOLUTE, writebyte, addr));              /* SLO $nnnn */

      OPCODE(10, BPL());                                          /* BPL $nnnn */
      OPCODE(11, ORA(5, INDIR_Y_BYTE_READ));                      /* ORA ($nn),Y */
      OPCODE(12, JAM());                                          /* JAM */
      OPCODE(13, SLO(8, INDIR_Y, writebyte, addr));               /* SLO ($nn),Y */
      OPCODE(14, DOP(4));                                         /* NOP $nn,X */
      OPCODE(15, ORA(4, ZP_IND_X_BYTE));                          /* ORA $nn,X */
      OPCODE(16, ASL(6, ZP_IND_X, ZP_WRITEBYTE, baddr));          /* ASL $nn,X */
      OPCODE(17, SLO(6, ZP_IND_X, ZP_WRITEBYTE, baddr));          /* SLO $nn,X */
      OPCODE(18, CLC());                                          /* CLC */
      OPCODE(19, ORA(4, ABS_IND_Y_BYTE_READ));                    /* ORA $nnnn,Y */
      OPCODE(1A, NOP());                                          /* NOP */
      OPCODE(1B, SLO(7, ABS_IND_Y, writebyte, addr));             /* SLO $nnnn,Y */
      OPCODE(1C, TOP());                                          /* NOP $nnnn,X */
      OPCODE(1D, ORA(4, ABS_IND_X_BYTE_READ));                    /* ORA $nnnn,X */
      OPCODE(1E, ASL(7, ABS_IND_X, writebyte, addr));              /* ASL $nnnn,X */
      OPCODE(1F, SLO(7, ABS_IND_X, writebyte, addr));              /* SLO $nnnn,X */

      OPCODE(20, JSR());                                          /* JSR $nnnn */
      OPCODE(21, AND(6, INDIR_X_BYTE));                           /* AND ($nn,X) */
      OPCODE(22, JAM());                                          /* JAM */
      OPCODE(23, RLA(8, INDIR_X, writebyte, addr));               /* RLA ($nn,X) */
      OPCODE(24, BIT(3, ZERO_PAGE_BYTE));                         /* BIT $nn */
      OPCODE(25, AND(3, ZERO_PAGE_BYTE));                         /* AND $nn */
      OPCODE(26, ROL(5, ZERO_PAGE, ZP_WRITEBYTE, baddr));         /* ROL $nn */
      OPCODE(27, RLA(5, ZERO_PAGE, ZP_WRITEBYTE, baddr));         /* RLA $nn */
      OPCODE(28, PLP());                                          /* PLP */
      OPCODE(29, AND(2, IMMEDIATE_BYTE));                         /* AND #$nn */
      OPCODE(2A, ROL_A());                                        /* ROL A */
      OPCODE(2B, ANC(2, IMMEDIATE_BYTE));                         /* ANC #$nn */
      OPCODE(2C, BIT(4, ABSOLUTE_BYTE));                          /* BIT $nnnn */
      OPCODE(2D, AND(4, ABSOLUTE_BYTE));                          /* AND $nnnn */
      OPCODE(2E, ROL(6, ABSOLUTE, writebyte, addr));              /* ROL $nnnn */
      OPCODE(2F, RLA(6, ABSOLUTE, writebyte, addr));              /* RLA $nnnn */

      OPCODE(30, BMI());                                          /* BMI $nnnn */
      OPCODE(31, AND(5, INDIR_Y_BYTE_READ));                      /* AND ($nn),Y */
      OPCODE(32, JAM());                                          /* JAM */
      OPCODE(33, RLA(8, INDIR_Y, writebyte, addr));               /* RLA ($nn),Y */
      OPCODE(34, DOP(4));                                         /* NOP */
      OPCODE(35, AND(4, ZP_IND_X_BYTE));                          /* AND $nn,X */
      OPCODE(36, ROL(6, ZP_IND_X, ZP_WRITEBYTE, baddr));          /* ROL $nn,X */
      OPCODE(37, RLA(6, ZP_IND_X, ZP_WRITEBYTE, baddr));          /* RLA $nn,X */
      OPCODE(38, SEC());                                          /* SEC */
      OPCODE(39, AND(4, ABS_IND_Y_BYTE_READ));                    /* AND $nnnn,Y */
      OPCODE(3A, NOP());                                          /* NOP */
      OPCODE(3B, RLA(7, ABS_IND_Y, writebyte, addr));             /* RLA $nnnn,Y */
      OPCODE(3C, TOP());                                          /* NOP $nnnn,X */
      OPCODE(3D, AND(4, ABS_IND_X_BYTE_READ));                    /* AND $nnnn,X */
      OPCODE(3E, ROL(7, ABS_IND_X, writebyte, addr));             /* ROL $nnnn,X */
      OPCODE(3F, RLA(7, ABS_IND_X, writebyte, addr));             /* RLA $nnnn,X */

      OPCODE(40, RTI());                                          /* RTI */
      OPCODE(41, EOR(6, INDIR_X_BYTE));                           /* EOR ($nn,X) */
      OPCODE(42, JAM());                                          /* JAM */
      OPCODE(43, SRE(8, INDIR_X, writebyte, addr));               /* SRE ($nn,X) */
      OPCODE(45, EOR(3, ZERO_PAGE_BYTE));                         /* EOR $nn */
      OPCODE(46, LSR(5, ZERO_PAGE, ZP_WRITEBYTE, baddr));         /* LSR $nn */
      OPCODE(47, SRE(5, ZERO_PAGE, ZP_WRITEBYTE, baddr));         /* SRE $nn */
      OPCODE(48, PHA());                                          /* PHA */
      OPCODE(49, EOR(2, IMMEDIATE_BYTE));                         /* EOR #$nn */
      OPCODE(4A, LSR_A());                                        /* LSR A */
      OPCODE(4B, ASR(2, IMMEDIATE_BYTE));                         /* ASR #$nn */
      OPCODE(4C, JMP_ABSOLUTE());                                 /* JMP $nnnn */
      OPCODE(4D, EOR(4, ABSOLUTE_BYTE));                          /* EOR $nnnn */
      OPCODE(4E, LSR(6, ABSOLUTE, writebyte, addr));              /* LSR $nnnn */
      OPCODE(4F, SRE(6, ABSOLUTE, writebyte, addr));              /* SRE $nnnn */

      OPCODE(50, BVC());                                          /* BVC $nnnn */
      OPCODE(51, EOR(5, INDIR_Y_BYTE_READ));                      /* EOR ($nn),Y */
      OPCODE(52, JAM());                                          /* JAM */
      OPCODE(53, SRE(8, INDIR_Y, writebyte, addr));               /* SRE ($nn),Y */
      OPCODE(55, EOR(4, ZP_IND_X_BYTE));                          /* EOR $nn,X */
      OPCODE(54, DOP(4));                                         /* NOP $nn,X */
      OPCODE(56, LSR(6, ZP_IND_X, ZP_WRITEBYTE, baddr));          /* LSR $nn,X */
      OPCODE(57, SRE(6, ZP_IND_X, ZP_WRITEBYTE, baddr));          /* SRE $nn,X */
      OPCODE(58, CLI());                                          /* CLI */
      OPCODE(59, EOR(4, ABS_IND_Y_BYTE_READ));                    /* EOR $nnnn,Y */
      OPCODE(5B, SRE(7, ABS_IND_Y, writebyte, addr));             /* SRE $nnnn,Y */
      OPCODE(5A, NOP());                                          /* NOP */
      OPCODE(5C, TOP());                                          /* NOP $nnnn,X */
      OPCODE(5D, EOR(4, ABS_IND_X_BYTE_READ));                    /* EOR $nnnn,X */
      OPCODE(5E, LSR(7, ABS_IND_X, writebyte, addr));             /* LSR $nnnn,X */
      OPCODE(5F, SRE(7, ABS_IND_X, writebyte, addr));             /* SRE $nnnn,X */

      OPCODE(60, RTS());                                          /* RTS */
      OPCODE(61, ADC(6, INDIR_X_BYTE));                           /* ADC ($nn,X) */
      OPCODE(62, JAM());                                          /* JAM */
      OPCODE(63, RRA(8, INDIR_X, writebyte, addr));               /* RRA ($nn,X) */
      OPCODE(44, DOP(3));                                         /* NOP $nn */
      OPCODE(64, DOP(3));                                         /* NOP $nn */
      OPCODE(65, ADC(3, ZERO_PAGE_BYTE));                         /* ADC $nn */
      OPCODE(66, ROR(5, ZERO_PAGE, ZP_WRITEBYTE, baddr));         /* ROR $nn */
      OPCODE(67, RRA(5, ZERO_PAGE, ZP_WRITEBYTE, baddr));         /* RRA $nn */
      OPCODE(68, PLA());                                          /* PLA */
      OPCODE(69, ADC(2, IMMEDIATE_BYTE));                         /* ADC #$nn */
      OPCODE(6A, ROR_A());                                        /* ROR A */
      OPCODE(6B, ARR(2, IMMEDIATE_BYTE));                         /* ARR #$nn */
      OPCODE(6C, JMP_INDIRECT());                                 /* JMP ($nnnn) */
      OPCODE(6D, ADC(4, ABSOLUTE_BYTE));                          /* ADC $nnnn */
      OPCODE(6E, ROR(6, ABSOLUTE, writebyte, addr));              /* ROR $nnnn */
      OPCODE(6F, RRA(6, ABSOLUTE, writebyte, addr));              /* RRA $nnnn */

      OPCODE(70, BVS());                                          /* BVS $nnnn */
      OPCODE(71, ADC(5, INDIR_Y_BYTE_READ));                      /* ADC ($nn),Y */
      OPCODE(72, JAM());                                          /* JAM */
      OPCODE(73, RRA(8, INDIR_Y, writebyte, addr));               /* RRA ($nn),Y */
      OPCODE(74, DOP(4));                                         /* NOP $nn,X */
      OPCODE(75, ADC(4, ZP_IND_X_BYTE));                          /* ADC $nn,X */
      OPCODE(76, ROR(6, ZP_IND_X, ZP_WRITEBYTE, baddr));          /* ROR $nn,X */
      OPCODE(77, RRA(6, ZP_IND_X, ZP_WRITEBYTE, baddr));          /* RRA $nn,X */
      OPCODE(78, SEI());                                          /* SEI */
      OPCODE(79, ADC(4, ABS_IND_Y_BYTE_READ));                    /* ADC $nnnn,Y */
      OPCODE(7A, NOP());                                          /* NOP */
      OPCODE(7B, RRA(7, ABS_IND_Y, writebyte, addr));             /* RRA $nnnn,Y */
      OPCODE(7C, TOP());                                          /* NOP $nnnn,X */
      OPCODE(7D, ADC(4, ABS_IND_X_BYTE_READ));                    /* ADC $nnnn,X */
      OPCODE(7E, ROR(7, ABS_IND_X, writebyte, addr));             /* ROR $nnnn,X */
      OPCODE(7F, RRA(7, ABS_IND_X, writebyte, addr));             /* RRA $nnnn,X */

      OPCODE(80, DOP(2));                                         /* NOP #$nn */
      OPCODE(81, STA(6, INDIR_X_ADDR, writebyte, addr));          /* STA ($nn,X) */
      OPCODE(82, DOP(2));                                         /* NOP #$nn */
      OPCODE(83, SAX(6, INDIR_X_ADDR, writebyte, addr));          /* SAX ($nn,X) */
      OPCODE(84, STY(3, ZERO_PAGE_ADDR, ZP_WRITEBYTE, baddr));    /* STY $nn */
      OPCODE(85, STA(3, ZERO_PAGE_ADDR, ZP_WRITEBYTE, baddr));    /* STA $nn */
      OPCODE(86, STX(3, ZERO_PAGE_ADDR, ZP_WRITEBYTE, baddr));    /* STX $nn */
      OPCODE(87, SAX(3, ZERO_PAGE_ADDR, ZP_WRITEBYTE, baddr));    /* SAX $nn */
      OPCODE(88, DEY());                                          /* DEY */
      OPCODE(89, DOP(2));                                         /* NOP #$nn */
      OPCODE(8A, TXA());                                          /* TXA */
      OPCODE(8B, ANE(2, IMMEDIATE_BYTE));                         /* ANE #$nn */
      OPCODE(8C, STY(4, ABSOLUTE_ADDR, writebyte, addr));         /* STY $nnnn */
      OPCODE(8D, STA(4, ABSOLUTE_ADDR, writebyte, addr));         /* STA $nnnn */
      OPCODE(8E, STX(4, ABSOLUTE_ADDR, writebyte, addr));         /* STX $nnnn */
      OPCODE(8F, SAX(4, ABSOLUTE_ADDR, writebyte, addr));         /* SAX $nnnn */

      OPCODE(90, BCC());                                          /* BCC $nnnn */
      OPCODE(91, STA(6, INDIR_Y_ADDR, writebyte, addr));          /* STA ($nn),Y */
      OPCODE(92, JAM());                                          /* JAM */
      OPCODE(93, SHA(6, INDIR_Y_ADDR, writebyte, addr));          /* SHA ($nn),Y */
      OPCODE(94, STY(4, ZP_IND_X_ADDR, ZP_WRITEBYTE, baddr));     /* STY $nn,X */
      OPCODE(95, STA(4, ZP_IND_X_ADDR, ZP_WRITEBYTE, baddr));     /* STA $nn,X */
      OPCODE(96, STX(4, ZP_IND_Y_ADDR, ZP_WRITEBYTE, baddr));     /* STX $nn,Y */
      OPCODE(97, SAX(4, ZP_IND_Y_ADDR, ZP_WRITEBYTE, baddr));     /* SAX $nn,Y */
      OPCODE(98, TYA());                                          /* TYA */
      OPCODE(99, STA(5, ABS_IND_Y_ADDR, writebyte, addr));        /* STA $nnnn,Y */
      OPCODE(9A, TXS());                                          /* TXS */
      OPCODE(9B, SHS(5, ABS_IND_Y_ADDR, writebyte, addr));        /* SHS $nnnn,Y */
      OPCODE(9C, SHY(5, ABS_IND_X_ADDR, writebyte, addr));        /* SHY $nnnn,X */
      OPCODE(9D, STA(5, ABS_IND_X_ADDR, writebyte, addr));        /* STA $nnnn,X */
      OPCODE(9E, SHX(5, ABS_IND_Y_ADDR, writebyte, addr));        /* SHX $nnnn,Y */
      OPCODE(9F, SHA(5, ABS_IND_Y_ADDR, writebyte, addr));        /* SHA $nnnn,Y */

      OPCODE(A0, LDY(2, IMMEDIATE_BYTE));                         /* LDY #$nn */
      OPCODE(A1, LDA(6, INDIR_X_BYTE));                           /* LDA ($nn,X) */
      OPCODE(A2, LDX(2, IMMEDIATE_BYTE));                         /* LDX #$nn */
      OPCODE(A3, LAX(6, INDIR_X_BYTE));                           /* LAX ($nn,X) */
      OPCODE(A4, LDY(3, ZERO_PAGE_BYTE));                         /* LDY $nn */
      OPCODE(A5, LDA(3, ZERO_PAGE_BYTE));                         /* LDA $nn */
      OPCODE(A6, LDX(3, ZERO_PAGE_BYTE));                         /* LDX $nn */
      OPCODE(A7, LAX(3, ZERO_PAGE_BYTE));                         /* LAX $nn */
      OPCODE(A8, TAY());                                          /* TAY */
      OPCODE(A9, LDA(2, IMMEDIATE_BYTE));                         /* LDA #$nn */
      OPCODE(AA, TAX());                                          /* TAX */
      OPCODE(AB, LXA(2, IMMEDIATE_BYTE));                         /* LXA #$nn */
      OPCODE(AC, LDY(4, ABSOLUTE_BYTE));                          /* LDY $nnnn */
      OPCODE(AD, LDA(4, ABSOLUTE_BYTE));                          /* LDA $nnnn */
      OPCODE(AE, LDX(4, ABSOLUTE_BYTE));                          /* LDX $nnnn */
      OPCODE(AF, LAX(4, ABSOLUTE_BYTE));                          /* LAX $nnnn */

      OPCODE(B0, BCS());                                          /* BCS $nnnn */
      OPCODE(B1, LDA(5, INDIR_Y_BYTE_READ));                      /* LDA ($nn),Y */
      OPCODE(B2, JAM());                                          /* JAM */
      OPCODE(B3, LAX(5, INDIR_Y_BYTE_READ));                      /* LAX ($nn),Y */
      OPCODE(B4, LDY(4, ZP_IND_X_BYTE));                          /* LDY $nn,X */
      OPCODE(B5, LDA(4, ZP_IND_X_BYTE));                          /* LDA $nn,X */
      OPCODE(B6, LDX(4, ZP_IND_Y_BYTE));                          /* LDX $nn,Y */
      OPCODE(B7, LAX(4, ZP_IND_Y_BYTE));                          /* LAX $nn,Y */
      OPCODE(B8, CLV());                                          /* CLV */
      OPCODE(B9, LDA(4, ABS_IND_Y_BYTE_READ));                    /* LDA $nnnn,Y */
      OPCODE(BA, TSX());                                          /* TSX */
      OPCODE(BB, LAS(4, ABS_IND_Y_BYTE_READ));                    /* LAS $nnnn,Y */
      OPCODE(BC, LDY(4, ABS_IND_X_BYTE_READ));                    /* LDY $nnnn,X */
      OPCODE(BD, LDA(4, ABS_IND_X_BYTE_READ));                    /* LDA $nnnn,X */
      OPCODE(BE, LDX(4, ABS_IND_Y_BYTE_READ));                    /* LDX $nnnn,Y */
      OPCODE(BF, LAX(4, ABS_IND_Y_BYTE_READ));                    /* LAX $nnnn,Y */

      OPCODE(C0, CPY(2, IMMEDIATE_BYTE));                         /* CPY #$nn */
      OPCODE(C1, CMP(6, INDIR_X_BYTE));                           /* CMP ($nn,X) */
      OPCODE(C2, DOP(2));                                         /* NOP #$nn */
      OPCODE(C3, DCP(8, INDIR_X, writebyte, addr));               /* DCP ($nn,X) */
      OPCODE(C4, CPY(3, ZERO_PAGE_BYTE));                         /* CPY $nn */
      OPCODE(C5, CMP(3, ZERO_PAGE_BYTE));                         /* CMP $nn */
      OPCODE(C6, DEC(5, ZERO_PAGE, ZP_WRITEBYTE, baddr));         /* DEC $nn */
      OPCODE(C7, DCP(5, ZERO_PAGE, ZP_WRITEBYTE, baddr));         /* DCP $nn */
      OPCODE(C8, INY());                                          /* INY */
      OPCODE(C9, CMP(2, IMMEDIATE_BYTE));                         /* CMP #$nn */
      OPCODE(CA, DEX());                                          /* DEX */
      OPCODE(CB, SBX(2, IMMEDIATE_BYTE));                         /* SBX #$nn */
      OPCODE(CC, CPY(4, ABSOLUTE_BYTE));                          /* CPY $nnnn */
      OPCODE(CD, CMP(4, ABSOLUTE_BYTE));                          /* CMP $nnnn */
      OPCODE(CE, DEC(6, ABSOLUTE, writebyte, addr));              /* DEC $nnnn */
      OPCODE(CF, DCP(6, ABSOLUTE, writebyte, addr));              /* DCP $nnnn */

      OPCODE(D0, BNE());                                          /* BNE $nnnn */
      OPCODE(D1, CMP(5, INDIR_Y_BYTE_READ));                      /* CMP ($nn),Y */
      OPCODE(D2, JAM());                                          /* JAM */
      OPCODE(D3, DCP(8, INDIR_Y, writebyte, addr));               /* DCP ($nn),Y */
      OPCODE(D4, DOP(4));                                         /* NOP $nn,X */
      OPCODE(D5, CMP(4, ZP_IND_X_BYTE));                          /* CMP $nn,X */
      OPCODE(D6, DEC(6, ZP_IND_X, ZP_WRITEBYTE, baddr));          /* DEC $nn,X */
      OPCODE(D7, DCP(6, ZP_IND_X, ZP_WRITEBYTE, baddr));          /* DCP $nn,X */
      OPCODE(D8, CLD());                                          /* CLD */
      OPCODE(D9, CMP(4, ABS_IND_Y_BYTE_READ));                    /* CMP $nnnn,Y */
      OPCODE(DA, NOP());                                          /* NOP */
      OPCODE(DB, DCP(7, ABS_IND_Y, writebyte, addr));             /* DCP $nnnn,Y */
      OPCODE(DC, TOP());                                          /* NOP $nnnn,X */
      OPCODE(DD, CMP(4, ABS_IND_X_BYTE_READ));                    /* CMP $nnnn,X */
      OPCODE(DE, DEC(7, ABS_IND_X, writebyte, addr));             /* DEC $nnnn,X */
      OPCODE(DF, DCP(7, ABS_IND_X, writebyte, addr));             /* DCP $nnnn,X */

      OPCODE(E0, CPX(2, IMMEDIATE_BYTE));                         /* CPX #$nn */
      OPCODE(E1, SBC(6, INDIR_X_BYTE));                           /* SBC ($nn,X) */
      OPCODE(E2, DOP(2));                                         /* NOP #$nn */
      OPCODE(E3, ISB(8, INDIR_X, writebyte, addr));               /* ISB ($nn,X) */
      OPCODE(E4, CPX(3, ZERO_PAGE_BYTE));                         /* CPX $nn */
      OPCODE(E5, SBC(3, ZERO_PAGE_BYTE));                         /* SBC $nn */
      OPCODE(E6, INC(5, ZERO_PAGE, ZP_WRITEBYTE, baddr));         /* INC $nn */
      OPCODE(E7, ISB(5, ZERO_PAGE, ZP_WRITEBYTE, baddr));         /* ISB $nn */
      OPCODE(E8, INX());                                          /* INX */
      OPCODE(E9, SBC(2, IMMEDIATE_BYTE));                         /* SBC #$nn */
      OPCODE(EB, SBC(2, IMMEDIATE_BYTE));                         /* USBC #$nn */
      OPCODE(EA, NOP());                                          /* NOP */
      OPCODE(EC, CPX(4, ABSOLUTE_BYTE));                          /* CPX $nnnn */
      OPCODE(ED, SBC(4, ABSOLUTE_BYTE));                          /* SBC $nnnn */
      OPCODE(EE, INC(6, ABSOLUTE, writebyte, addr));              /* INC $nnnn */
      OPCODE(EF, ISB(6, ABSOLUTE, writebyte, addr));              /* ISB $nnnn */

      OPCODE(F0, BEQ());                                          /* BEQ $nnnn */
      OPCODE(F1, SBC(5, INDIR_Y_BYTE_READ));                      /* SBC ($nn),Y */
      OPCODE(F2, JAM());                                          /* JAM */
      OPCODE(F3, ISB(8, INDIR_Y, writebyte, addr));               /* ISB ($nn),Y */
      OPCODE(F4, DOP(4));                                         /* NOP ($nn,X) */
      OPCODE(F5, SBC(4, ZP_IND_X_BYTE));                          /* SBC $nn,X */
      OPCODE(F6, INC(6, ZP_IND_X, ZP_WRITEBYTE, baddr));          /* INC $nn,X */
      OPCODE(F7, ISB(6, ZP_IND_X, ZP_WRITEBYTE, baddr));          /* ISB $nn,X */
      OPCODE(F8, SED());                                          /* SED */
      OPCODE(F9, SBC(4, ABS_IND_Y_BYTE_READ));                    /* SBC $nnnn,Y */
      OPCODE(FA, NOP());                                          /* NOP */
      OPCODE(FB, ISB(7, ABS_IND_Y, writebyte, addr));             /* ISB $nnnn,Y */
      OPCODE(FC, TOP());                                          /* NOP $nnnn,X */
      OPCODE(FD, SBC(4, ABS_IND_X_BYTE_READ));                    /* SBC $nnnn,X */
      OPCODE(FE, INC(7, ABS_IND_X, writebyte, addr));             /* INC $nnnn,X */
      OPCODE(FF, ISB(7, ABS_IND_X, writebyte, addr));             /* ISB $nnnn,X */
      }
   }

   /* store local copy of regs */
   STORE_LOCAL_REGS();

   /* Return our actual amount of executed cycles */
   return (cpu.total_cycles - old_cycles);
}

/* Issue a CPU Reset */
void nes6502_reset(void)
{
   cpu.p_reg = Z_FLAG | R_FLAG | I_FLAG;  /* Reserved bit always 1 */
   cpu.int_pending = 0;                   /* No pending interrupts */
   cpu.int_latency = 0;                   /* No latent interrupts */
   cpu.pc_reg = readword(RESET_VECTOR);   /* Fetch reset vector */
   cpu.burn_cycles = RESET_CYCLES;
   cpu.jammed = false;
}

/* Non-maskable interrupt */
IRAM_ATTR void nes6502_nmi(void)
{
   DECLARE_LOCAL_REGS();

   if (false == cpu.jammed)
   {
      GET_GLOBAL_REGS();
      NMI_PROC();
      cpu.burn_cycles += INT_CYCLES;
      STORE_LOCAL_REGS();
   }
}

/* Interrupt request */
IRAM_ATTR void nes6502_irq(void)
{
   DECLARE_LOCAL_REGS();

   if (false == cpu.jammed)
   {
      GET_GLOBAL_REGS();
      if (0 == i_flag)
      {
         IRQ_PROC();
         cpu.burn_cycles += INT_CYCLES;
      }
      else
      {
         cpu.int_pending = 1;
      }
      STORE_LOCAL_REGS();
   }
}

/* Set dead cycle period */
IRAM_ATTR void nes6502_burn(int cycles)
{
   cpu.burn_cycles += cycles;
}

/* Release our timeslice */
IRAM_ATTR void nes6502_release(void)
{
   remaining_cycles = 0;
}

/* Create a nes6502 object */
nes6502_t *nes6502_init(mem_t *_mem)
{
   memset(&cpu, 0, sizeof(nes6502_t));

   cpu.zp    = _mem->ram + 0x000; // Page 0
   cpu.stack = _mem->ram + 0x100; // Page 1

   mem = _mem; // For FASTMEM

   return &cpu;
}

/* Destroy a nes6502 object */
void nes6502_shutdown(void)
{
   //
}
