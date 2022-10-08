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
** nes/cpu.c: CPU emulation (2A03, 6502-like)
**
*/

#include "nes.h"

/* internal CPU context */
static nes6502_t cpu;
static mem_t *mem;

// #define NES6502_JUMPTABLE
#define NES6502_FASTMEM


#define ADD_CYCLES(x) \
{ \
   remaining_cycles -= (x); \
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
/* Not really an addressing mode, just for LDx/STx */
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

#define PENDING_IRQ_PROC() \
{ \
   if (0 == i_flag && cpu.int_pending && remaining_cycles > 0) \
   { \
      cpu.int_pending = 0; \
      IRQ_PROC(); \
      ADD_CYCLES(INT_CYCLES); \
   } \
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
   PENDING_IRQ_PROC(); \
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
   PENDING_IRQ_PROC(); \
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

#ifndef IS_BIG_ENDIAN
#define PAGE_READWORD(page, addr)  (*(uint16 *)((page) + (addr)))
#else
#define PAGE_READWORD(page, addr)  (page[(addr) + 1] << 8 | page[(addr)])
#endif

#define readword(a) mem_getword(a)
#define readbyte(a) mem_getbyte(a)

/*
** Middle man for faster albeit unsafe/inaccurate/unchecked memory access.
** Used only when address = PC, which is always a valid ROM access (in theory)
*/

#ifdef NES6502_FASTMEM

#define fast_readbyte(a) ({uint16 _a = (a); mem->pages[_a >> MEM_PAGESHIFT][_a];})
#define fast_readword(a) ({uint16 _a = (a); ((_a & MEM_PAGEMASK) != MEM_PAGEMASK) ? PAGE_READWORD(mem->pages[_a >> MEM_PAGESHIFT], _a) : mem_getword(_a);})
#define writebyte(a, v)  {uint16 _a = (a), _v = (v); if (_a < 0x2000) mem->ram[_a & 0x7FF] = _v; else mem_putbyte(_a, _v);}

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

#define OPCODE(xx, x...)  case xx: x; break;
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
uint32 nes6502_getcycles()
{
   return cpu.total_cycles;
}

/* Execute instructions until count expires
**
** Returns the number of cycles *actually* executed, which will be
** anywhere from zero to cycles + 6
*/
IRAM_ATTR int nes6502_execute(int cycles)
{
   uint32 temp, addr; /* for macros */
   uint8 btemp, baddr; /* for macros */
   uint8 data;

   DECLARE_LOCAL_REGS();
   GET_GLOBAL_REGS();

#ifdef NES6502_JUMPTABLE

   static const void *opcode_table[256] =
   {
      &&op0x00, &&op0x01, &&op0x02, &&op0x03, &&op0x04, &&op0x05, &&op0x06, &&op0x07,
      &&op0x08, &&op0x09, &&op0x0A, &&op0x0B, &&op0x0C, &&op0x0D, &&op0x0E, &&op0x0F,
      &&op0x10, &&op0x11, &&op0x12, &&op0x13, &&op0x14, &&op0x15, &&op0x16, &&op0x17,
      &&op0x18, &&op0x19, &&op0x1A, &&op0x1B, &&op0x1C, &&op0x1D, &&op0x1E, &&op0x1F,
      &&op0x20, &&op0x21, &&op0x22, &&op0x23, &&op0x24, &&op0x25, &&op0x26, &&op0x27,
      &&op0x28, &&op0x29, &&op0x2A, &&op0x2B, &&op0x2C, &&op0x2D, &&op0x2E, &&op0x2F,
      &&op0x30, &&op0x31, &&op0x32, &&op0x33, &&op0x34, &&op0x35, &&op0x36, &&op0x37,
      &&op0x38, &&op0x39, &&op0x3A, &&op0x3B, &&op0x3C, &&op0x3D, &&op0x3E, &&op0x3F,
      &&op0x40, &&op0x41, &&op0x42, &&op0x43, &&op0x44, &&op0x45, &&op0x46, &&op0x47,
      &&op0x48, &&op0x49, &&op0x4A, &&op0x4B, &&op0x4C, &&op0x4D, &&op0x4E, &&op0x4F,
      &&op0x50, &&op0x51, &&op0x52, &&op0x53, &&op0x54, &&op0x55, &&op0x56, &&op0x57,
      &&op0x58, &&op0x59, &&op0x5A, &&op0x5B, &&op0x5C, &&op0x5D, &&op0x5E, &&op0x5F,
      &&op0x60, &&op0x61, &&op0x62, &&op0x63, &&op0x64, &&op0x65, &&op0x66, &&op0x67,
      &&op0x68, &&op0x69, &&op0x6A, &&op0x6B, &&op0x6C, &&op0x6D, &&op0x6E, &&op0x6F,
      &&op0x70, &&op0x71, &&op0x72, &&op0x73, &&op0x74, &&op0x75, &&op0x76, &&op0x77,
      &&op0x78, &&op0x79, &&op0x7A, &&op0x7B, &&op0x7C, &&op0x7D, &&op0x7E, &&op0x7F,
      &&op0x80, &&op0x81, &&op0x82, &&op0x83, &&op0x84, &&op0x85, &&op0x86, &&op0x87,
      &&op0x88, &&op0x89, &&op0x8A, &&op0x8B, &&op0x8C, &&op0x8D, &&op0x8E, &&op0x8F,
      &&op0x90, &&op0x91, &&op0x92, &&op0x93, &&op0x94, &&op0x95, &&op0x96, &&op0x97,
      &&op0x98, &&op0x99, &&op0x9A, &&op0x9B, &&op0x9C, &&op0x9D, &&op0x9E, &&op0x9F,
      &&op0xA0, &&op0xA1, &&op0xA2, &&op0xA3, &&op0xA4, &&op0xA5, &&op0xA6, &&op0xA7,
      &&op0xA8, &&op0xA9, &&op0xAA, &&op0xAB, &&op0xAC, &&op0xAD, &&op0xAE, &&op0xAF,
      &&op0xB0, &&op0xB1, &&op0xB2, &&op0xB3, &&op0xB4, &&op0xB5, &&op0xB6, &&op0xB7,
      &&op0xB8, &&op0xB9, &&op0xBA, &&op0xBB, &&op0xBC, &&op0xBD, &&op0xBE, &&op0xBF,
      &&op0xC0, &&op0xC1, &&op0xC2, &&op0xC3, &&op0xC4, &&op0xC5, &&op0xC6, &&op0xC7,
      &&op0xC8, &&op0xC9, &&op0xCA, &&op0xCB, &&op0xCC, &&op0xCD, &&op0xCE, &&op0xCF,
      &&op0xD0, &&op0xD1, &&op0xD2, &&op0xD3, &&op0xD4, &&op0xD5, &&op0xD6, &&op0xD7,
      &&op0xD8, &&op0xD9, &&op0xDA, &&op0xDB, &&op0xDC, &&op0xDD, &&op0xDE, &&op0xDF,
      &&op0xE0, &&op0xE1, &&op0xE2, &&op0xE3, &&op0xE4, &&op0xE5, &&op0xE6, &&op0xE7,
      &&op0xE8, &&op0xE9, &&op0xEA, &&op0xEB, &&op0xEC, &&op0xED, &&op0xEE, &&op0xEF,
      &&op0xF0, &&op0xF1, &&op0xF2, &&op0xF3, &&op0xF4, &&op0xF5, &&op0xF6, &&op0xF7,
      &&op0xF8, &&op0xF9, &&op0xFA, &&op0xFB, &&op0xFC, &&op0xFD, &&op0xFE, &&op0xFF
   };

#endif /* NES6502_JUMPTABLE */

   long remaining_cycles = cycles;

   /* check for DMA cycle burning */
   if (cpu.burn_cycles && remaining_cycles > 0)
   {
      int burn_for = MIN(remaining_cycles, cpu.burn_cycles);
      ADD_CYCLES(burn_for);
      cpu.burn_cycles -= burn_for;
   }

   PENDING_IRQ_PROC();

   /* Continue until we run out of cycles */
   while (remaining_cycles > 0)
   {
      /* Fetch and execute instruction */
      OPCODE_NEXT
      {
      OPCODE(0x00, BRK());                                          /* BRK */
      OPCODE(0x01, ORA(6, INDIR_X_BYTE));                           /* ORA ($nn,X) */
      OPCODE(0x02, JAM());                                          /* JAM */
      OPCODE(0x03, SLO(8, INDIR_X, writebyte, addr));               /* SLO ($nn,X) */
      OPCODE(0x04, DOP(3));                                         /* NOP $nn */
      OPCODE(0x05, ORA(3, ZERO_PAGE_BYTE));                         /* ORA $nn */
      OPCODE(0x06, ASL(5, ZERO_PAGE, ZP_WRITEBYTE, baddr));         /* ASL $nn */
      OPCODE(0x07, SLO(5, ZERO_PAGE, ZP_WRITEBYTE, baddr));         /* SLO $nn */
      OPCODE(0x08, PHP());                                          /* PHP */
      OPCODE(0x09, ORA(2, IMMEDIATE_BYTE));                         /* ORA #$nn */
      OPCODE(0x0A, ASL_A());                                        /* ASL A */
      OPCODE(0x0B, ANC(2, IMMEDIATE_BYTE));                         /* ANC #$nn */
      OPCODE(0x0C, TOP());                                          /* NOP $nnnn */
      OPCODE(0x0D, ORA(4, ABSOLUTE_BYTE));                          /* ORA $nnnn */
      OPCODE(0x0E, ASL(6, ABSOLUTE, writebyte, addr));              /* ASL $nnnn */
      OPCODE(0x0F, SLO(6, ABSOLUTE, writebyte, addr));              /* SLO $nnnn */

      OPCODE(0x10, BPL());                                          /* BPL $nnnn */
      OPCODE(0x11, ORA(5, INDIR_Y_BYTE_READ));                      /* ORA ($nn),Y */
      OPCODE(0x12, JAM());                                          /* JAM */
      OPCODE(0x13, SLO(8, INDIR_Y, writebyte, addr));               /* SLO ($nn),Y */
      OPCODE(0x14, DOP(4));                                         /* NOP $nn,X */
      OPCODE(0x15, ORA(4, ZP_IND_X_BYTE));                          /* ORA $nn,X */
      OPCODE(0x16, ASL(6, ZP_IND_X, ZP_WRITEBYTE, baddr));          /* ASL $nn,X */
      OPCODE(0x17, SLO(6, ZP_IND_X, ZP_WRITEBYTE, baddr));          /* SLO $nn,X */
      OPCODE(0x18, CLC());                                          /* CLC */
      OPCODE(0x19, ORA(4, ABS_IND_Y_BYTE_READ));                    /* ORA $nnnn,Y */
      OPCODE(0x1A, NOP());                                          /* NOP */
      OPCODE(0x1B, SLO(7, ABS_IND_Y, writebyte, addr));             /* SLO $nnnn,Y */
      OPCODE(0x1C, TOP());                                          /* NOP $nnnn,X */
      OPCODE(0x1D, ORA(4, ABS_IND_X_BYTE_READ));                    /* ORA $nnnn,X */
      OPCODE(0x1E, ASL(7, ABS_IND_X, writebyte, addr));             /* ASL $nnnn,X */
      OPCODE(0x1F, SLO(7, ABS_IND_X, writebyte, addr));             /* SLO $nnnn,X */

      OPCODE(0x20, JSR());                                          /* JSR $nnnn */
      OPCODE(0x21, AND(6, INDIR_X_BYTE));                           /* AND ($nn,X) */
      OPCODE(0x22, JAM());                                          /* JAM */
      OPCODE(0x23, RLA(8, INDIR_X, writebyte, addr));               /* RLA ($nn,X) */
      OPCODE(0x24, BIT(3, ZERO_PAGE_BYTE));                         /* BIT $nn */
      OPCODE(0x25, AND(3, ZERO_PAGE_BYTE));                         /* AND $nn */
      OPCODE(0x26, ROL(5, ZERO_PAGE, ZP_WRITEBYTE, baddr));         /* ROL $nn */
      OPCODE(0x27, RLA(5, ZERO_PAGE, ZP_WRITEBYTE, baddr));         /* RLA $nn */
      OPCODE(0x28, PLP());                                          /* PLP */
      OPCODE(0x29, AND(2, IMMEDIATE_BYTE));                         /* AND #$nn */
      OPCODE(0x2A, ROL_A());                                        /* ROL A */
      OPCODE(0x2B, ANC(2, IMMEDIATE_BYTE));                         /* ANC #$nn */
      OPCODE(0x2C, BIT(4, ABSOLUTE_BYTE));                          /* BIT $nnnn */
      OPCODE(0x2D, AND(4, ABSOLUTE_BYTE));                          /* AND $nnnn */
      OPCODE(0x2E, ROL(6, ABSOLUTE, writebyte, addr));              /* ROL $nnnn */
      OPCODE(0x2F, RLA(6, ABSOLUTE, writebyte, addr));              /* RLA $nnnn */

      OPCODE(0x30, BMI());                                          /* BMI $nnnn */
      OPCODE(0x31, AND(5, INDIR_Y_BYTE_READ));                      /* AND ($nn),Y */
      OPCODE(0x32, JAM());                                          /* JAM */
      OPCODE(0x33, RLA(8, INDIR_Y, writebyte, addr));               /* RLA ($nn),Y */
      OPCODE(0x34, DOP(4));                                         /* NOP */
      OPCODE(0x35, AND(4, ZP_IND_X_BYTE));                          /* AND $nn,X */
      OPCODE(0x36, ROL(6, ZP_IND_X, ZP_WRITEBYTE, baddr));          /* ROL $nn,X */
      OPCODE(0x37, RLA(6, ZP_IND_X, ZP_WRITEBYTE, baddr));          /* RLA $nn,X */
      OPCODE(0x38, SEC());                                          /* SEC */
      OPCODE(0x39, AND(4, ABS_IND_Y_BYTE_READ));                    /* AND $nnnn,Y */
      OPCODE(0x3A, NOP());                                          /* NOP */
      OPCODE(0x3B, RLA(7, ABS_IND_Y, writebyte, addr));             /* RLA $nnnn,Y */
      OPCODE(0x3C, TOP());                                          /* NOP $nnnn,X */
      OPCODE(0x3D, AND(4, ABS_IND_X_BYTE_READ));                    /* AND $nnnn,X */
      OPCODE(0x3E, ROL(7, ABS_IND_X, writebyte, addr));             /* ROL $nnnn,X */
      OPCODE(0x3F, RLA(7, ABS_IND_X, writebyte, addr));             /* RLA $nnnn,X */

      OPCODE(0x40, RTI());                                          /* RTI */
      OPCODE(0x41, EOR(6, INDIR_X_BYTE));                           /* EOR ($nn,X) */
      OPCODE(0x42, JAM());                                          /* JAM */
      OPCODE(0x43, SRE(8, INDIR_X, writebyte, addr));               /* SRE ($nn,X) */
      OPCODE(0x44, DOP(3));                                         /* NOP $nn */
      OPCODE(0x45, EOR(3, ZERO_PAGE_BYTE));                         /* EOR $nn */
      OPCODE(0x46, LSR(5, ZERO_PAGE, ZP_WRITEBYTE, baddr));         /* LSR $nn */
      OPCODE(0x47, SRE(5, ZERO_PAGE, ZP_WRITEBYTE, baddr));         /* SRE $nn */
      OPCODE(0x48, PHA());                                          /* PHA */
      OPCODE(0x49, EOR(2, IMMEDIATE_BYTE));                         /* EOR #$nn */
      OPCODE(0x4A, LSR_A());                                        /* LSR A */
      OPCODE(0x4B, ASR(2, IMMEDIATE_BYTE));                         /* ASR #$nn */
      OPCODE(0x4C, JMP_ABSOLUTE());                                 /* JMP $nnnn */
      OPCODE(0x4D, EOR(4, ABSOLUTE_BYTE));                          /* EOR $nnnn */
      OPCODE(0x4E, LSR(6, ABSOLUTE, writebyte, addr));              /* LSR $nnnn */
      OPCODE(0x4F, SRE(6, ABSOLUTE, writebyte, addr));              /* SRE $nnnn */

      OPCODE(0x50, BVC());                                          /* BVC $nnnn */
      OPCODE(0x51, EOR(5, INDIR_Y_BYTE_READ));                      /* EOR ($nn),Y */
      OPCODE(0x52, JAM());                                          /* JAM */
      OPCODE(0x53, SRE(8, INDIR_Y, writebyte, addr));               /* SRE ($nn),Y */
      OPCODE(0x54, DOP(4));                                         /* NOP $nn,X */
      OPCODE(0x55, EOR(4, ZP_IND_X_BYTE));                          /* EOR $nn,X */
      OPCODE(0x56, LSR(6, ZP_IND_X, ZP_WRITEBYTE, baddr));          /* LSR $nn,X */
      OPCODE(0x57, SRE(6, ZP_IND_X, ZP_WRITEBYTE, baddr));          /* SRE $nn,X */
      OPCODE(0x58, CLI());                                          /* CLI */
      OPCODE(0x59, EOR(4, ABS_IND_Y_BYTE_READ));                    /* EOR $nnnn,Y */
      OPCODE(0x5B, SRE(7, ABS_IND_Y, writebyte, addr));             /* SRE $nnnn,Y */
      OPCODE(0x5A, NOP());                                          /* NOP */
      OPCODE(0x5C, TOP());                                          /* NOP $nnnn,X */
      OPCODE(0x5D, EOR(4, ABS_IND_X_BYTE_READ));                    /* EOR $nnnn,X */
      OPCODE(0x5E, LSR(7, ABS_IND_X, writebyte, addr));             /* LSR $nnnn,X */
      OPCODE(0x5F, SRE(7, ABS_IND_X, writebyte, addr));             /* SRE $nnnn,X */

      OPCODE(0x60, RTS());                                          /* RTS */
      OPCODE(0x61, ADC(6, INDIR_X_BYTE));                           /* ADC ($nn,X) */
      OPCODE(0x62, JAM());                                          /* JAM */
      OPCODE(0x63, RRA(8, INDIR_X, writebyte, addr));               /* RRA ($nn,X) */
      OPCODE(0x64, DOP(3));                                         /* NOP $nn */
      OPCODE(0x65, ADC(3, ZERO_PAGE_BYTE));                         /* ADC $nn */
      OPCODE(0x66, ROR(5, ZERO_PAGE, ZP_WRITEBYTE, baddr));         /* ROR $nn */
      OPCODE(0x67, RRA(5, ZERO_PAGE, ZP_WRITEBYTE, baddr));         /* RRA $nn */
      OPCODE(0x68, PLA());                                          /* PLA */
      OPCODE(0x69, ADC(2, IMMEDIATE_BYTE));                         /* ADC #$nn */
      OPCODE(0x6A, ROR_A());                                        /* ROR A */
      OPCODE(0x6B, ARR(2, IMMEDIATE_BYTE));                         /* ARR #$nn */
      OPCODE(0x6C, JMP_INDIRECT());                                 /* JMP ($nnnn) */
      OPCODE(0x6D, ADC(4, ABSOLUTE_BYTE));                          /* ADC $nnnn */
      OPCODE(0x6E, ROR(6, ABSOLUTE, writebyte, addr));              /* ROR $nnnn */
      OPCODE(0x6F, RRA(6, ABSOLUTE, writebyte, addr));              /* RRA $nnnn */

      OPCODE(0x70, BVS());                                          /* BVS $nnnn */
      OPCODE(0x71, ADC(5, INDIR_Y_BYTE_READ));                      /* ADC ($nn),Y */
      OPCODE(0x72, JAM());                                          /* JAM */
      OPCODE(0x73, RRA(8, INDIR_Y, writebyte, addr));               /* RRA ($nn),Y */
      OPCODE(0x74, DOP(4));                                         /* NOP $nn,X */
      OPCODE(0x75, ADC(4, ZP_IND_X_BYTE));                          /* ADC $nn,X */
      OPCODE(0x76, ROR(6, ZP_IND_X, ZP_WRITEBYTE, baddr));          /* ROR $nn,X */
      OPCODE(0x77, RRA(6, ZP_IND_X, ZP_WRITEBYTE, baddr));          /* RRA $nn,X */
      OPCODE(0x78, SEI());                                          /* SEI */
      OPCODE(0x79, ADC(4, ABS_IND_Y_BYTE_READ));                    /* ADC $nnnn,Y */
      OPCODE(0x7A, NOP());                                          /* NOP */
      OPCODE(0x7B, RRA(7, ABS_IND_Y, writebyte, addr));             /* RRA $nnnn,Y */
      OPCODE(0x7C, TOP());                                          /* NOP $nnnn,X */
      OPCODE(0x7D, ADC(4, ABS_IND_X_BYTE_READ));                    /* ADC $nnnn,X */
      OPCODE(0x7E, ROR(7, ABS_IND_X, writebyte, addr));             /* ROR $nnnn,X */
      OPCODE(0x7F, RRA(7, ABS_IND_X, writebyte, addr));             /* RRA $nnnn,X */

      OPCODE(0x80, DOP(2));                                         /* NOP #$nn */
      OPCODE(0x81, STA(6, INDIR_X_ADDR, writebyte, addr));          /* STA ($nn,X) */
      OPCODE(0x82, DOP(2));                                         /* NOP #$nn */
      OPCODE(0x83, SAX(6, INDIR_X_ADDR, writebyte, addr));          /* SAX ($nn,X) */
      OPCODE(0x84, STY(3, ZERO_PAGE_ADDR, ZP_WRITEBYTE, baddr));    /* STY $nn */
      OPCODE(0x85, STA(3, ZERO_PAGE_ADDR, ZP_WRITEBYTE, baddr));    /* STA $nn */
      OPCODE(0x86, STX(3, ZERO_PAGE_ADDR, ZP_WRITEBYTE, baddr));    /* STX $nn */
      OPCODE(0x87, SAX(3, ZERO_PAGE_ADDR, ZP_WRITEBYTE, baddr));    /* SAX $nn */
      OPCODE(0x88, DEY());                                          /* DEY */
      OPCODE(0x89, DOP(2));                                         /* NOP #$nn */
      OPCODE(0x8A, TXA());                                          /* TXA */
      OPCODE(0x8B, ANE(2, IMMEDIATE_BYTE));                         /* ANE #$nn */
      OPCODE(0x8C, STY(4, ABSOLUTE_ADDR, writebyte, addr));         /* STY $nnnn */
      OPCODE(0x8D, STA(4, ABSOLUTE_ADDR, writebyte, addr));         /* STA $nnnn */
      OPCODE(0x8E, STX(4, ABSOLUTE_ADDR, writebyte, addr));         /* STX $nnnn */
      OPCODE(0x8F, SAX(4, ABSOLUTE_ADDR, writebyte, addr));         /* SAX $nnnn */

      OPCODE(0x90, BCC());                                          /* BCC $nnnn */
      OPCODE(0x91, STA(6, INDIR_Y_ADDR, writebyte, addr));          /* STA ($nn),Y */
      OPCODE(0x92, JAM());                                          /* JAM */
      OPCODE(0x93, SHA(6, INDIR_Y_ADDR, writebyte, addr));          /* SHA ($nn),Y */
      OPCODE(0x94, STY(4, ZP_IND_X_ADDR, ZP_WRITEBYTE, baddr));     /* STY $nn,X */
      OPCODE(0x95, STA(4, ZP_IND_X_ADDR, ZP_WRITEBYTE, baddr));     /* STA $nn,X */
      OPCODE(0x96, STX(4, ZP_IND_Y_ADDR, ZP_WRITEBYTE, baddr));     /* STX $nn,Y */
      OPCODE(0x97, SAX(4, ZP_IND_Y_ADDR, ZP_WRITEBYTE, baddr));     /* SAX $nn,Y */
      OPCODE(0x98, TYA());                                          /* TYA */
      OPCODE(0x99, STA(5, ABS_IND_Y_ADDR, writebyte, addr));        /* STA $nnnn,Y */
      OPCODE(0x9A, TXS());                                          /* TXS */
      OPCODE(0x9B, SHS(5, ABS_IND_Y_ADDR, writebyte, addr));        /* SHS $nnnn,Y */
      OPCODE(0x9C, SHY(5, ABS_IND_X_ADDR, writebyte, addr));        /* SHY $nnnn,X */
      OPCODE(0x9D, STA(5, ABS_IND_X_ADDR, writebyte, addr));        /* STA $nnnn,X */
      OPCODE(0x9E, SHX(5, ABS_IND_Y_ADDR, writebyte, addr));        /* SHX $nnnn,Y */
      OPCODE(0x9F, SHA(5, ABS_IND_Y_ADDR, writebyte, addr));        /* SHA $nnnn,Y */

      OPCODE(0xA0, LDY(2, IMMEDIATE_BYTE));                         /* LDY #$nn */
      OPCODE(0xA1, LDA(6, INDIR_X_BYTE));                           /* LDA ($nn,X) */
      OPCODE(0xA2, LDX(2, IMMEDIATE_BYTE));                         /* LDX #$nn */
      OPCODE(0xA3, LAX(6, INDIR_X_BYTE));                           /* LAX ($nn,X) */
      OPCODE(0xA4, LDY(3, ZERO_PAGE_BYTE));                         /* LDY $nn */
      OPCODE(0xA5, LDA(3, ZERO_PAGE_BYTE));                         /* LDA $nn */
      OPCODE(0xA6, LDX(3, ZERO_PAGE_BYTE));                         /* LDX $nn */
      OPCODE(0xA7, LAX(3, ZERO_PAGE_BYTE));                         /* LAX $nn */
      OPCODE(0xA8, TAY());                                          /* TAY */
      OPCODE(0xA9, LDA(2, IMMEDIATE_BYTE));                         /* LDA #$nn */
      OPCODE(0xAA, TAX());                                          /* TAX */
      OPCODE(0xAB, LXA(2, IMMEDIATE_BYTE));                         /* LXA #$nn */
      OPCODE(0xAC, LDY(4, ABSOLUTE_BYTE));                          /* LDY $nnnn */
      OPCODE(0xAD, LDA(4, ABSOLUTE_BYTE));                          /* LDA $nnnn */
      OPCODE(0xAE, LDX(4, ABSOLUTE_BYTE));                          /* LDX $nnnn */
      OPCODE(0xAF, LAX(4, ABSOLUTE_BYTE));                          /* LAX $nnnn */

      OPCODE(0xB0, BCS());                                          /* BCS $nnnn */
      OPCODE(0xB1, LDA(5, INDIR_Y_BYTE_READ));                      /* LDA ($nn),Y */
      OPCODE(0xB2, JAM());                                          /* JAM */
      OPCODE(0xB3, LAX(5, INDIR_Y_BYTE_READ));                      /* LAX ($nn),Y */
      OPCODE(0xB4, LDY(4, ZP_IND_X_BYTE));                          /* LDY $nn,X */
      OPCODE(0xB5, LDA(4, ZP_IND_X_BYTE));                          /* LDA $nn,X */
      OPCODE(0xB6, LDX(4, ZP_IND_Y_BYTE));                          /* LDX $nn,Y */
      OPCODE(0xB7, LAX(4, ZP_IND_Y_BYTE));                          /* LAX $nn,Y */
      OPCODE(0xB8, CLV());                                          /* CLV */
      OPCODE(0xB9, LDA(4, ABS_IND_Y_BYTE_READ));                    /* LDA $nnnn,Y */
      OPCODE(0xBA, TSX());                                          /* TSX */
      OPCODE(0xBB, LAS(4, ABS_IND_Y_BYTE_READ));                    /* LAS $nnnn,Y */
      OPCODE(0xBC, LDY(4, ABS_IND_X_BYTE_READ));                    /* LDY $nnnn,X */
      OPCODE(0xBD, LDA(4, ABS_IND_X_BYTE_READ));                    /* LDA $nnnn,X */
      OPCODE(0xBE, LDX(4, ABS_IND_Y_BYTE_READ));                    /* LDX $nnnn,Y */
      OPCODE(0xBF, LAX(4, ABS_IND_Y_BYTE_READ));                    /* LAX $nnnn,Y */

      OPCODE(0xC0, CPY(2, IMMEDIATE_BYTE));                         /* CPY #$nn */
      OPCODE(0xC1, CMP(6, INDIR_X_BYTE));                           /* CMP ($nn,X) */
      OPCODE(0xC2, DOP(2));                                         /* NOP #$nn */
      OPCODE(0xC3, DCP(8, INDIR_X, writebyte, addr));               /* DCP ($nn,X) */
      OPCODE(0xC4, CPY(3, ZERO_PAGE_BYTE));                         /* CPY $nn */
      OPCODE(0xC5, CMP(3, ZERO_PAGE_BYTE));                         /* CMP $nn */
      OPCODE(0xC6, DEC(5, ZERO_PAGE, ZP_WRITEBYTE, baddr));         /* DEC $nn */
      OPCODE(0xC7, DCP(5, ZERO_PAGE, ZP_WRITEBYTE, baddr));         /* DCP $nn */
      OPCODE(0xC8, INY());                                          /* INY */
      OPCODE(0xC9, CMP(2, IMMEDIATE_BYTE));                         /* CMP #$nn */
      OPCODE(0xCA, DEX());                                          /* DEX */
      OPCODE(0xCB, SBX(2, IMMEDIATE_BYTE));                         /* SBX #$nn */
      OPCODE(0xCC, CPY(4, ABSOLUTE_BYTE));                          /* CPY $nnnn */
      OPCODE(0xCD, CMP(4, ABSOLUTE_BYTE));                          /* CMP $nnnn */
      OPCODE(0xCE, DEC(6, ABSOLUTE, writebyte, addr));              /* DEC $nnnn */
      OPCODE(0xCF, DCP(6, ABSOLUTE, writebyte, addr));              /* DCP $nnnn */

      OPCODE(0xD0, BNE());                                          /* BNE $nnnn */
      OPCODE(0xD1, CMP(5, INDIR_Y_BYTE_READ));                      /* CMP ($nn),Y */
      OPCODE(0xD2, JAM());                                          /* JAM */
      OPCODE(0xD3, DCP(8, INDIR_Y, writebyte, addr));               /* DCP ($nn),Y */
      OPCODE(0xD4, DOP(4));                                         /* NOP $nn,X */
      OPCODE(0xD5, CMP(4, ZP_IND_X_BYTE));                          /* CMP $nn,X */
      OPCODE(0xD6, DEC(6, ZP_IND_X, ZP_WRITEBYTE, baddr));          /* DEC $nn,X */
      OPCODE(0xD7, DCP(6, ZP_IND_X, ZP_WRITEBYTE, baddr));          /* DCP $nn,X */
      OPCODE(0xD8, CLD());                                          /* CLD */
      OPCODE(0xD9, CMP(4, ABS_IND_Y_BYTE_READ));                    /* CMP $nnnn,Y */
      OPCODE(0xDA, NOP());                                          /* NOP */
      OPCODE(0xDB, DCP(7, ABS_IND_Y, writebyte, addr));             /* DCP $nnnn,Y */
      OPCODE(0xDC, TOP());                                          /* NOP $nnnn,X */
      OPCODE(0xDD, CMP(4, ABS_IND_X_BYTE_READ));                    /* CMP $nnnn,X */
      OPCODE(0xDE, DEC(7, ABS_IND_X, writebyte, addr));             /* DEC $nnnn,X */
      OPCODE(0xDF, DCP(7, ABS_IND_X, writebyte, addr));             /* DCP $nnnn,X */

      OPCODE(0xE0, CPX(2, IMMEDIATE_BYTE));                         /* CPX #$nn */
      OPCODE(0xE1, SBC(6, INDIR_X_BYTE));                           /* SBC ($nn,X) */
      OPCODE(0xE2, DOP(2));                                         /* NOP #$nn */
      OPCODE(0xE3, ISB(8, INDIR_X, writebyte, addr));               /* ISB ($nn,X) */
      OPCODE(0xE4, CPX(3, ZERO_PAGE_BYTE));                         /* CPX $nn */
      OPCODE(0xE5, SBC(3, ZERO_PAGE_BYTE));                         /* SBC $nn */
      OPCODE(0xE6, INC(5, ZERO_PAGE, ZP_WRITEBYTE, baddr));         /* INC $nn */
      OPCODE(0xE7, ISB(5, ZERO_PAGE, ZP_WRITEBYTE, baddr));         /* ISB $nn */
      OPCODE(0xE8, INX());                                          /* INX */
      OPCODE(0xE9, SBC(2, IMMEDIATE_BYTE));                         /* SBC #$nn */
      OPCODE(0xEB, SBC(2, IMMEDIATE_BYTE));                         /* USBC #$nn */
      OPCODE(0xEA, NOP());                                          /* NOP */
      OPCODE(0xEC, CPX(4, ABSOLUTE_BYTE));                          /* CPX $nnnn */
      OPCODE(0xED, SBC(4, ABSOLUTE_BYTE));                          /* SBC $nnnn */
      OPCODE(0xEE, INC(6, ABSOLUTE, writebyte, addr));              /* INC $nnnn */
      OPCODE(0xEF, ISB(6, ABSOLUTE, writebyte, addr));              /* ISB $nnnn */

      OPCODE(0xF0, BEQ());                                          /* BEQ $nnnn */
      OPCODE(0xF1, SBC(5, INDIR_Y_BYTE_READ));                      /* SBC ($nn),Y */
      OPCODE(0xF2, JAM());                                          /* JAM */
      OPCODE(0xF3, ISB(8, INDIR_Y, writebyte, addr));               /* ISB ($nn),Y */
      OPCODE(0xF4, DOP(4));                                         /* NOP ($nn,X) */
      OPCODE(0xF5, SBC(4, ZP_IND_X_BYTE));                          /* SBC $nn,X */
      OPCODE(0xF6, INC(6, ZP_IND_X, ZP_WRITEBYTE, baddr));          /* INC $nn,X */
      OPCODE(0xF7, ISB(6, ZP_IND_X, ZP_WRITEBYTE, baddr));          /* ISB $nn,X */
      OPCODE(0xF8, SED());                                          /* SED */
      OPCODE(0xF9, SBC(4, ABS_IND_Y_BYTE_READ));                    /* SBC $nnnn,Y */
      OPCODE(0xFA, NOP());                                          /* NOP */
      OPCODE(0xFB, ISB(7, ABS_IND_Y, writebyte, addr));             /* ISB $nnnn,Y */
      OPCODE(0xFC, TOP());                                          /* NOP $nnnn,X */
      OPCODE(0xFD, SBC(4, ABS_IND_X_BYTE_READ));                    /* SBC $nnnn,X */
      OPCODE(0xFE, INC(7, ABS_IND_X, writebyte, addr));             /* INC $nnnn,X */
      OPCODE(0xFF, ISB(7, ABS_IND_X, writebyte, addr));             /* ISB $nnnn,X */
      }
   }

   /* store local copy of regs */
   STORE_LOCAL_REGS();

   /* Return our actual amount of executed cycles */
   cycles -= remaining_cycles; // remaining_cycles can be negative, which is fine
   cpu.total_cycles += cycles;

   return cycles;
}

/* Non-maskable interrupt */
void nes6502_nmi(void)
{
   if (cpu.jammed)
      return;

   DECLARE_LOCAL_REGS();
   GET_GLOBAL_REGS();
   NMI_PROC();
   STORE_LOCAL_REGS();
   cpu.burn_cycles += INT_CYCLES;
}

/* Interrupt request */
void nes6502_irq(void)
{
   if (cpu.jammed)
      return;

   if (cpu.p_reg & I_FLAG)
   {
      cpu.int_pending = 1;
   }
   else
   {
      DECLARE_LOCAL_REGS();
      GET_GLOBAL_REGS();
      IRQ_PROC();
      STORE_LOCAL_REGS();
      cpu.burn_cycles += INT_CYCLES;
   }
}

void nes6502_irq_clear(void)
{
   cpu.p_reg &= I_FLAG;
}

/* Set dead cycle period */
void nes6502_burn(int cycles)
{
   cpu.burn_cycles += cycles;
}

/* Issue a CPU Reset */
void nes6502_reset(void)
{
   cpu.p_reg = Z_FLAG | R_FLAG | I_FLAG;  /* Reserved bit always 1 */
   cpu.pc_reg = readword(RESET_VECTOR);   /* Fetch reset vector */
   cpu.int_pending = false;               /* No pending interrupts */
   cpu.jammed = false;
   cpu.burn_cycles = RESET_CYCLES;
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
