/*
 * arm-codegen.h
 *
 * Copyright (c) 2002 Wild West Software
 * Copyright (c) 2001, 2002 Sergey Chaban
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */


#ifndef ARM_CG_H
#define ARM_CG_H

typedef unsigned long arminstr_t;
typedef unsigned long armword_t;

/* Helper functions */
/*void arm_emit_std_prologue(cg_segment_t * segment, unsigned int local_size);
void arm_emit_std_epilogue(cg_segment_t * segment, unsigned int local_size, int pop_regs);
void arm_emit_lean_prologue(cg_segment_t * segment, unsigned int local_size, int push_regs);
int arm_is_power_of_2(armword_t val);
int calc_arm_mov_const_shift(armword_t val);
int is_arm_const(armword_t val);
int arm_bsf(armword_t val);
void arm_mov_reg_imm32_cond(cg_segment_t * segment, int reg, armword_t imm32, int cond);
void arm_mov_reg_imm32(cg_segment_t * segment, int reg, armword_t imm32);*/


//*** check for correctness ***
//extern u32* x86Ptr;

void write_to_file(u32 val);

//#define write32(val) { *(u32 *)translation_ptr = val; write_to_file(*(u32 *)translation_ptr); translation_ptr += 4; }

//#define write32(val) { if( g_PcWatch.IsReset == RECRESET_OFF ) { *(u32*)pCurPage->pCodeCurrent = val; pCurPage->pCodeCurrent +=4; if( (u32)pCurPage->pCodeCurrent >= (u32)pCurPage->pCodeEnd ) { g_PcWatch.IsReset = RECRESET_START; recResize(); g_PcWatch.IsReset = RECRESET_END; return; } }else{ if( g_PcWatch.IsReset == RECRESET_END ){ g_PcWatch.IsReset = RECRESET_OFF; return; } } }
//#define write32_ret(val) { if( g_PcWatch.IsReset == RECRESET_OFF ) { *(u32*)pCurPage->pCodeCurrent = val; pCurPage->pCodeCurrent +=4; if( (u32)pCurPage->pCodeCurrent >= (u32)pCurPage->pCodeEnd ) { g_PcWatch.IsReset = RECRESET_START; recResize(); g_PcWatch.IsReset = RECRESET_END; return 0; } }else{ if( g_PcWatch.IsReset == RECRESET_END ){ g_PcWatch.IsReset = RECRESET_OFF; return 0; } } }
//#define write32(val) { *(u32*)pCurPage->pCodeCurrent = val; pCurPage->pCodeCurrent +=4; }

#define ARM_EMIT(p, i) write32(i);
//write32(i);
/*{ *(u32*)translation_ptr = (i); translation_ptr += 4; } */

#if defined(_MSC_VER) && !defined(ARM_NOIASM)
# define ARM_IASM(_expr) __easfdmit (_expr)
#else
# define ARM_IASM(_expr)
#endif

/* even_scale = rot << 1 */
#define ARM_SCALE(imm8, even_scale) ( ((imm8) >> (even_scale)) | ((imm8) << (32 - even_scale)) )



typedef enum {
  ARMREG_R0 = 0,
  ARMREG_R1,
  ARMREG_R2,
  ARMREG_R3,
  ARMREG_R4,
  ARMREG_R5,
  ARMREG_R6,
  ARMREG_R7,
  ARMREG_R8,
  ARMREG_R9,
  ARMREG_R10,
  ARMREG_R11,
  ARMREG_R12,
  ARMREG_R13,
  ARMREG_R14,
  ARMREG_R15,


  /* aliases */
  /* args */
  ARMREG_A1 = ARMREG_R0,
  ARMREG_A2 = ARMREG_R1,
  ARMREG_A3 = ARMREG_R2,
  ARMREG_A4 = ARMREG_R3,

  /* local vars */
  ARMREG_V1 = ARMREG_R4,
  ARMREG_V2 = ARMREG_R5,
  ARMREG_V3 = ARMREG_R6,
  ARMREG_V4 = ARMREG_R7,
  ARMREG_V5 = ARMREG_R8,
  ARMREG_V6 = ARMREG_R9,
  ARMREG_V7 = ARMREG_R10,

  ARMREG_FP = ARMREG_R11,
  ARMREG_IP = ARMREG_R12,
  ARMREG_SP = ARMREG_R13,
  ARMREG_LR = ARMREG_R14,
  ARMREG_PC = ARMREG_R15,

  /* FPU */
  ARMREG_F0 = 0,
  ARMREG_F1,
  ARMREG_F2,
  ARMREG_F3,
  ARMREG_F4,
  ARMREG_F5,
  ARMREG_F6,
  ARMREG_F7,

  /* co-processor */
  ARMREG_CR0 = 0,
  ARMREG_CR1,
  ARMREG_CR2,
  ARMREG_CR3,
  ARMREG_CR4,
  ARMREG_CR5,
  ARMREG_CR6,
  ARMREG_CR7,
  ARMREG_CR8,
  ARMREG_CR9,
  ARMREG_CR10,
  ARMREG_CR11,
  ARMREG_CR12,
  ARMREG_CR13,
  ARMREG_CR14,
  ARMREG_CR15,

  /* XScale: acc0 on CP0 */
  ARMREG_ACC0 = ARMREG_CR0,

  ARMREG_MAX = ARMREG_R15,

  /* flags */
  ARMREG_CPSR = 0,
  ARMREG_SPSR = 1
} ARMReg;

typedef enum {
  ARM_FCONST_0_0 = 8,
  ARM_FCONST_1_0,
  ARM_FCONST_2_0,
  ARM_FCONST_3_0,
  ARM_FCONST_4_0,
  ARM_FCONST_5_0,
  ARM_FCONST_0_5,
  ARM_FCONST_10_0
} ARMFPUConst;

/* number of argument registers */
#define ARM_NUM_ARG_REGS 4

/* number of non-argument registers */
#define ARM_NUM_VARIABLE_REGS 7

/* number of global registers */
#define ARM_NUM_GLOBAL_REGS 5

/* bitvector for all argument regs (A1-A4) */
#define ARM_ALL_ARG_REGS \
  (1 << ARMREG_A1) | (1 << ARMREG_A2) | (1 << ARMREG_A3) | (1 << ARMREG_A4)


typedef enum {
  ARMCOND_EQ = 0x0,          /* Equal; Z = 1 */
  ARMCOND_NE = 0x1,          /* Not equal, or unordered; Z = 0 */
  ARMCOND_CS = 0x2,          /* Carry set; C = 1 */
  ARMCOND_HS = ARMCOND_CS,   /* Unsigned higher or same; */
  ARMCOND_CC = 0x3,          /* Carry clear; C = 0 */
  ARMCOND_LO = ARMCOND_CC,   /* Unsigned lower */
  ARMCOND_MI = 0x4,          /* Negative; N = 1 */
  ARMCOND_PL = 0x5,          /* Positive or zero; N = 0 */
  ARMCOND_VS = 0x6,          /* Overflow; V = 1 */
  ARMCOND_VC = 0x7,          /* No overflow; V = 0 */
  ARMCOND_HI = 0x8,          /* Unsigned higher; C = 1 && Z = 0 */
  ARMCOND_LS = 0x9,          /* Unsigned lower or same; C = 0 || Z = 1 */
  ARMCOND_GE = 0xA,          /* Signed greater than or equal; N = V */
  ARMCOND_LT = 0xB,          /* Signed less than; N != V */
  ARMCOND_GT = 0xC,          /* Signed greater than; Z = 0 && N = V */
  ARMCOND_LE = 0xD,          /* Signed less than or equal; Z = 1 && N != V */
  ARMCOND_AL = 0xE,          /* Always */
  ARMCOND_NV = 0xF,          /* Never */

  ARMCOND_SHIFT = 28
} ARMCond;

#define ARMCOND_MASK (ARMCOND_NV << ARMCOND_SHIFT)

#define ARM_DEF_COND(cond) (((cond) & 0xF) << ARMCOND_SHIFT)



typedef enum {
  ARMSHIFT_LSL = 0,
  ARMSHIFT_LSR = 1,
  ARMSHIFT_ASR = 2,
  ARMSHIFT_ROR = 3,

  ARMSHIFT_ASL = ARMSHIFT_LSL
  /* rrx = (ror, 1) */
} ARMShiftType;


typedef struct {
  armword_t PSR_c : 8;
  armword_t PSR_x : 8;
  armword_t PSR_s : 8;
  armword_t PSR_f : 8;
} ARMPSR;

typedef enum {
  ARMOP_AND = 0x0,
  ARMOP_EOR = 0x1,
  ARMOP_SUB = 0x2,
  ARMOP_RSB = 0x3,
  ARMOP_ADD = 0x4,
  ARMOP_ADC = 0x5,
  ARMOP_SBC = 0x6,
  ARMOP_RSC = 0x7,
  ARMOP_TST = 0x8,
  ARMOP_TEQ = 0x9,
  ARMOP_CMP = 0xa,
  ARMOP_CMN = 0xb,
  ARMOP_ORR = 0xc,
  ARMOP_MOV = 0xd,
  ARMOP_BIC = 0xe,
  ARMOP_MVN = 0xf,


  /* not really opcodes */

  ARMOP_STR = 0x0,
  ARMOP_LDR = 0x1,

  /* ARM2+ */
  ARMOP_MUL   = 0x0, /* Rd := Rm*Rs */
  ARMOP_MLA   = 0x1, /* Rd := (Rm*Rs)+Rn */

  /* ARM7+ */
  ARMOP_MLS   = 0x3, /* Rd := Rn-(Rm*Rs) */

  /* ARM3M+ */
  ARMOP_UMULL = 0x4,
  ARMOP_UMLAL = 0x5,
  ARMOP_SMULL = 0x6,
  ARMOP_SMLAL = 0x7,

  /* for data transfers with register offset */
  ARM_UP   = 1,
  ARM_DOWN = 0
} ARMOpcode;

typedef enum {
  THUMBOP_AND  = 0,
  THUMBOP_EOR  = 1,
  THUMBOP_LSL  = 2,
  THUMBOP_LSR  = 3,
  THUMBOP_ASR  = 4,
  THUMBOP_ADC  = 5,
  THUMBOP_SBC  = 6,
  THUMBOP_ROR  = 7,
  THUMBOP_TST  = 8,
  THUMBOP_NEG  = 9,
  THUMBOP_CMP  = 10,
  THUMBOP_CMN  = 11,
  THUMBOP_ORR  = 12,
  THUMBOP_MUL  = 13,
  THUMBOP_BIC  = 14,
  THUMBOP_MVN  = 15,
  THUMBOP_MOV  = 16,
  THUMBOP_CMPI = 17,
  THUMBOP_ADD  = 18,
  THUMBOP_SUB  = 19,
  THUMBOP_CMPH = 19,
  THUMBOP_MOVH = 20
} ThumbOpcode;


/* Generic form - all ARM instructions are conditional. */
typedef struct {
  arminstr_t icode : 28;
  arminstr_t cond  :  4;
} ARMInstrGeneric;



/* Branch or Branch with Link instructions. */
typedef struct {
  arminstr_t offset : 24;
  arminstr_t link   :  1;
  arminstr_t tag    :  3; /* 1 0 1 */
  arminstr_t cond   :  4;
} ARMInstrBR;

#define ARM_BR_ID 5
#define ARM_BR_MASK 7 << 25
#define ARM_BR_TAG ARM_BR_ID << 25

#define ARM_DEF_BR(offs, l, cond) ((offs) | ((l) << 24) | (ARM_BR_TAG) | (cond << ARMCOND_SHIFT))

/* branch */
#define ARM_B_COND(p, cond, offset) ARM_EMIT(p, ARM_DEF_BR(offset, 0, cond))
#define ARM_B(p, offs) ARM_B_COND((p), ARMCOND_AL, (offs))
/* branch with link */
#define ARM_BL_COND(p, cond, offset) ARM_EMIT(p, ARM_DEF_BR(offset, 1, cond))
#define ARM_BL(p, offs) ARM_BL_COND((p), ARMCOND_AL, (offs))

/* branch to register and exchange */
#define ARM_BX_COND(p, cond, reg) ARM_EMIT(p, ((cond << ARMCOND_SHIFT) | (reg) | 0x12FFF10))
#define ARM_BX(p, reg) ARM_BX_COND((p), ARMCOND_AL, (reg))

/* branch to register with link */
#define ARM_BLX_COND(p, cond, reg) ARM_EMIT(p, ((cond << ARMCOND_SHIFT) | (reg) | 0x12FFF30))
#define ARM_BLX(p, reg) ARM_BLX_COND((p), ARMCOND_AL, (reg))


/* Data Processing Instructions - there are 3 types. */

typedef struct {
  arminstr_t imm : 8;
  arminstr_t rot : 4;
} ARMDPI_op2_imm;

typedef struct {
  arminstr_t rm   : 4;
  arminstr_t tag  : 1; /* 0 - immediate shift, 1 - reg shift */
  arminstr_t type : 2; /* shift type - logical, arithmetic, rotate */
} ARMDPI_op2_reg_shift;


/* op2 is reg shift by imm */
typedef union
{
  ARMDPI_op2_reg_shift r2;
  struct
  {
#ifdef MSB_FIRST
    arminstr_t shift : 5;
    arminstr_t _dummy_r2 : 7;
#else
    arminstr_t _dummy_r2 : 7;
    arminstr_t shift : 5;
#endif
  } imm;
} ARMDPI_op2_reg_imm;

/* op2 is reg shift by reg */
typedef union {
  ARMDPI_op2_reg_shift r2;
  struct {
#ifdef MSB_FIRST
    arminstr_t rs        : 4;
    arminstr_t pad       : 1; /* always 0, to differentiate from HXFER etc. */
    arminstr_t _dummy_r2 : 7;
#else
    arminstr_t _dummy_r2 : 7;
    arminstr_t pad       : 1; /* always 0, to differentiate from HXFER etc. */
    arminstr_t rs        : 4;
#endif
  } reg;
} ARMDPI_op2_reg_reg;

/* Data processing instrs */
typedef union {
  ARMDPI_op2_imm op2_imm;

  ARMDPI_op2_reg_shift op2_reg;
  ARMDPI_op2_reg_imm op2_reg_imm;
  ARMDPI_op2_reg_reg op2_reg_reg;

  struct {
#ifdef MSB_FIRST
    arminstr_t cond   :  4;
    arminstr_t tag    :  2; /* 0 0 */
    arminstr_t type   :  1; /* type of op2, 0 = register, 1 = immediate */
    arminstr_t opcode :  4; /* arithmetic/logic operation */
    arminstr_t s      :  1; /* S-bit controls PSR update */
    arminstr_t rn     :  4; /* first operand reg */
    arminstr_t rd     :  4; /* destination reg */
    arminstr_t op2    : 12; /* raw operand 2 */
#else
    arminstr_t op2    : 12; /* raw operand 2 */
    arminstr_t rd     :  4; /* destination reg */
    arminstr_t rn     :  4; /* first operand reg */
    arminstr_t s      :  1; /* S-bit controls PSR update */
    arminstr_t opcode :  4; /* arithmetic/logic operation */
    arminstr_t type   :  1; /* type of op2, 0 = register, 1 = immediate */
    arminstr_t tag    :  2; /* 0 0 */
    arminstr_t cond   :  4;
#endif
  } all;
} ARMInstrDPI;

#define ARM_DPI_ID 0
#define ARM_DPI_MASK 3 << 26
#define ARM_DPI_TAG ARM_DPI_ID << 26

#define ARM_DEF_DPI_IMM_COND(imm8, rot, rd, rn, s, op, cond) \
  ((imm8) & 0xFF)      | \
  (((rot) & 0xF) << 8) | \
  ((rd) << 12)         | \
  ((rn) << 16)         | \
  ((s) << 20)          | \
  ((op) << 21)         | \
  (1 << 25)            | \
  (ARM_DPI_TAG)        | \
  ARM_DEF_COND(cond)


#define ARM_DEF_DPI_IMM(imm8, rot, rd, rn, s, op) \
  ARM_DEF_DPI_IMM_COND(imm8, rot, rd, rn, s, op, ARMCOND_AL)

/* codegen */
#define ARM_DPIOP_REG_IMM8ROT_COND(p, op, rd, rn, imm8, rot, cond) \
  ARM_EMIT(p, ARM_DEF_DPI_IMM_COND((imm8), ((rot) >> 1), (rd), (rn), 0, (op), cond))
#define ARM_DPIOP_S_REG_IMM8ROT_COND(p, op, rd, rn, imm8, rot, cond) \
  ARM_EMIT(p, ARM_DEF_DPI_IMM_COND((imm8), ((rot) >> 1), (rd), (rn), 1, (op), cond))

/* inline */
#define ARM_IASM_DPIOP_REG_IMM8ROT_COND(p, op, rd, rn, imm8, rot, cond) \
  ARM_IASM(ARM_DEF_DPI_IMM_COND((imm8), ((rot) >> 1), (rd), (rn), 0, (op), cond))
#define ARM_IASM_DPIOP_S_REG_IMM8ROT_COND(p, op, rd, rn, imm8, rot, cond) \
  ARM_IASM(ARM_DEF_DPI_IMM_COND((imm8), ((rot) >> 1), (rd), (rn), 1, (op), cond))



#define ARM_DEF_DPI_REG_IMMSHIFT_COND(rm, shift_type, imm_shift, rd, rn, s, op, cond) \
  (rm)                        | \
  ((shift_type & 3) << 5)     | \
  (((imm_shift) & 0x1F) << 7) | \
  ((rd) << 12)                | \
  ((rn) << 16)                | \
  ((s) << 20)                 | \
  ((op) << 21)                | \
  (ARM_DPI_TAG)               | \
  ARM_DEF_COND(cond)

/* codegen */
#define ARM_DPIOP_REG_IMMSHIFT_COND(p, op, rd, rn, rm, shift_type, imm_shift, cond) \
  ARM_EMIT(p, ARM_DEF_DPI_REG_IMMSHIFT_COND((rm), shift_type, imm_shift, (rd), (rn), 0, (op), cond))

#define ARM_DPIOP_S_REG_IMMSHIFT_COND(p, op, rd, rn, rm, shift_type, imm_shift, cond) \
  ARM_EMIT(p, ARM_DEF_DPI_REG_IMMSHIFT_COND((rm), shift_type, imm_shift, (rd), (rn), 1, (op), cond))

#define ARM_DPIOP_REG_REG_COND(p, op, rd, rn, rm, cond) \
  ARM_EMIT(p, ARM_DEF_DPI_REG_IMMSHIFT_COND((rm), ARMSHIFT_LSL, 0, (rd), (rn), 0, (op), cond))

#define ARM_DPIOP_S_REG_REG_COND(p, op, rd, rn, rm, cond) \
  ARM_EMIT(p, ARM_DEF_DPI_REG_IMMSHIFT_COND((rm), ARMSHIFT_LSL, 0, (rd), (rn), 1, (op), cond))

/* inline */
#define ARM_IASM_DPIOP_REG_IMMSHIFT_COND(p, op, rd, rn, rm, shift_type, imm_shift, cond) \
  ARM_IASM(ARM_DEF_DPI_REG_IMMSHIFT_COND((rm), shift_type, imm_shift, (rd), (rn), 0, (op), cond))

#define ARM_IASM_DPIOP_S_REG_IMMSHIFT_COND(p, op, rd, rn, rm, shift_type, imm_shift, cond) \
  ARM_IASM(ARM_DEF_DPI_REG_IMMSHIFT_COND((rm), shift_type, imm_shift, (rd), (rn), 1, (op), cond))

#define ARM_IASM_DPIOP_REG_REG_COND(p, op, rd, rn, rm, cond) \
  ARM_IASM(ARM_DEF_DPI_REG_IMMSHIFT_COND((rm), ARMSHIFT_LSL, 0, (rd), (rn), 0, (op), cond))

#define ARM_IASM_DPIOP_S_REG_REG_COND(p, op, rd, rn, rm, cond) \
  ARM_IASM_EMIT(ARM_DEF_DPI_REG_IMMSHIFT_COND((rm), ARMSHIFT_LSL, 0, (rd), (rn), 1, (op), cond))


/* Rd := Rn op (Rm shift_type Rs) */
#define ARM_DEF_DPI_REG_REGSHIFT_COND(rm, shift_type, rs, rd, rn, s, op, cond) \
  (rm)                        | \
  (1 << 4)                    | \
  ((shift_type & 3) << 5)     | \
  ((rs) << 8)                 | \
  ((rd) << 12)                | \
  ((rn) << 16)                | \
  ((s) << 20)                 | \
  ((op) << 21)                | \
  (ARM_DPI_TAG)               | \
  ARM_DEF_COND(cond)

/* codegen */
#define ARM_DPIOP_REG_REGSHIFT_COND(p, op, rd, rn, rm, shift_type, rs, cond) \
  ARM_EMIT(p, ARM_DEF_DPI_REG_REGSHIFT_COND((rm), shift_type, (rs), (rd), (rn), 0, (op), cond))

#define ARM_DPIOP_S_REG_REGSHIFT_COND(p, op, rd, rn, rm, shift_type, rs, cond) \
  ARM_EMIT(p, ARM_DEF_DPI_REG_REGSHIFT_COND((rm), shift_type, (rs), (rd), (rn), 1, (op), cond))

/* inline */
#define ARM_IASM_DPIOP_REG_REGSHIFT_COND(p, op, rd, rn, rm, shift_type, rs, cond) \
  ARM_IASM(ARM_DEF_DPI_REG_REGSHIFT_COND((rm), shift_type, (rs), (rd), (rn), 0, (op), cond))

#define ARM_IASM_DPIOP_S_REG_REGSHIFT_COND(p, op, rd, rn, rm, shift_type, rs, cond) \
  ARM_IASM(ARM_DEF_DPI_REG_REGSHIFT_COND((rm), shift_type, (rs), (rd), (rn), 1, (op), cond))



/* Multiple register transfer. */
typedef struct {
  arminstr_t reg_list : 16; /* bitfield */
  arminstr_t rn       :  4; /* base reg */
  arminstr_t ls       :  1; /* load(1)/store(0) */
  arminstr_t wb       :  1; /* write-back "!" */
  arminstr_t s        :  1; /* restore PSR, force user bit */
  arminstr_t u        :  1; /* up/down */
  arminstr_t p        :  1; /* pre(1)/post(0) index */
  arminstr_t tag      :  3; /* 1 0 0 */
  arminstr_t cond     :  4;
} ARMInstrMRT;

#define ARM_MRT_ID 4
#define ARM_MRT_MASK 7 << 25
#define ARM_MRT_TAG ARM_MRT_ID << 25

#define ARM_DEF_MRT(regs, rn, l, w, s, u, p, cond) \
  (regs)        | \
  (rn << 16)    | \
  (l << 20)     | \
  (w << 21)     | \
  (s << 22)     | \
  (u << 23)     | \
  (p << 24)     | \
  (ARM_MRT_TAG) | \
  ARM_DEF_COND(cond)

#define ARM_STMDB(p, rbase, regs) ARM_EMIT(p, ARM_DEF_MRT(regs, rbase, 0, 0, 0, 0, 1, ARMCOND_AL))
#define ARM_LDMDB(p, rbase, regs) ARM_EMIT(p, ARM_DEF_MRT(regs, rbase, 1, 0, 0, 0, 1, ARMCOND_AL))
#define ARM_STMDB_WB(p, rbase, regs) ARM_EMIT(p, ARM_DEF_MRT(regs, rbase, 0, 1, 0, 0, 1, ARMCOND_AL))
#define ARM_LDMIA_WB(p, rbase, regs) ARM_EMIT(p, ARM_DEF_MRT(regs, rbase, 1, 1, 0, 1, 0, ARMCOND_AL))
#define ARM_LDMIA(p, rbase, regs) ARM_EMIT(p, ARM_DEF_MRT(regs, rbase, 1, 0, 0, 1, 0, ARMCOND_AL))
#define ARM_STMIA(p, rbase, regs) ARM_EMIT(p, ARM_DEF_MRT(regs, rbase, 0, 0, 0, 1, 0, ARMCOND_AL))
#define ARM_STMIA_WB(p, rbase, regs) ARM_EMIT(p, ARM_DEF_MRT(regs, rbase, 0, 1, 0, 1, 0, ARMCOND_AL))

#define ARM_LDMIA_WB_PC_S(p, rbase, regs) ARM_EMIT(p, ARM_DEF_MRT(regs, rbase, 1, 1, 1, 1, 0, ARMCOND_AL))

/* THUMB
#define ARM_POP_OP(p) ARM_EMIT(p, 0xFF01BD17)
#define ARM_PUSH_OP(p) ARM_EMIT(p, 0xFF02B497)
*/

/* stmdb sp!, {regs} */
#define ARM_PUSH(p, regs) ARM_EMIT(p, ARM_DEF_MRT(regs, ARMREG_SP, 0, 1, 0, 0, 1, ARMCOND_AL))
#define ARM_IASM_PUSH(regs) ARM_IASM(ARM_DEF_MRT(regs, ARMREG_SP, 0, 1, 0, 0, 1, ARMCOND_AL))

/* ldmia sp!, {regs} */
#define ARM_POP(p, regs) ARM_EMIT(p, ARM_DEF_MRT(regs, ARMREG_SP, 1, 1, 0, 1, 0, ARMCOND_AL))
#define ARM_IASM_POP(regs) ARM_IASM_EMIT(ARM_DEF_MRT(regs, ARMREG_SP, 1, 1, 0, 1, 0, ARMCOND_AL))

/* ldmia sp, {regs} ; (no write-back) */
#define ARM_POP_NWB(p, regs) ARM_EMIT(p, ARM_DEF_MRT(regs, ARMREG_SP, 1, 0, 0, 1, 0, ARMCOND_AL))
#define ARM_IASM_POP_NWB(regs) ARM_IASM_EMIT(ARM_DEF_MRT(regs, ARMREG_SP, 1, 0, 0, 1, 0, ARMCOND_AL))

#define ARM_PUSH1(p, r1) ARM_PUSH(p, (1 << r1))
#define ARM_PUSH2(p, r1, r2) ARM_PUSH(p, (1 << r1) | (1 << r2))
#define ARM_PUSH3(p, r1, r2, r3) ARM_PUSH(p, (1 << r1) | (1 << r2) | (1 << r3))
#define ARM_PUSH4(p, r1, r2, r3, r4) ARM_PUSH(p, (1 << r1) | (1 << r2) | (1 << r3) | (1 << r4))
#define ARM_PUSH5(p, r1, r2, r3, r4, r5) ARM_PUSH(p, (1 << r1) | (1 << r2) | (1 << r3) | (1 << r4) | (1 << r5))
#define ARM_PUSH6(p, r1, r2, r3, r4, r5, r6) ARM_PUSH(p, (1 << r1) | (1 << r2) | (1 << r3) | (1 << r4) | (1 << r5) | (1 << r6))
#define ARM_PUSH7(p, r1, r2, r3, r4, r5, r6, r7) ARM_PUSH(p, (1 << r1) | (1 << r2) | (1 << r3) | (1 << r4) | (1 << r5) | (1 << r6) | (1 << r7))
#define ARM_PUSH8(p, r1, r2, r3, r4, r5, r6, r7, r8) ARM_PUSH(p, (1 << r1) | (1 << r2) | (1 << r3) | (1 << r4) | (1 << r5) | (1 << r6) | (1 << r7) | (1 << r8))
#define ARM_PUSH9(p, r1, r2, r3, r4, r5, r6, r7, r8, r9) ARM_PUSH(p, (1 << r1) | (1 << r2) | (1 << r3) | (1 << r4) | (1 << r5) | (1 << r6) | (1 << r7) | (1 << r8) | (1 << r9))

#define ARM_POP9(p, r1, r2, r3, r4, r5, r6, r7, r8, r9) ARM_POP(p, (1 << r1) | (1 << r2) | (1 << r3) | (1 << r4) | (1 << r5) | (1 << r6) | (1 << r7) | (1 << r8) | (1 << r9))
#define ARM_POP8(p, r1, r2, r3, r4, r5, r6, r7, r8) ARM_POP(p, (1 << r1) | (1 << r2) | (1 << r3) | (1 << r4) | (1 << r5) | (1 << r6) | (1 << r7) | (1 << r8))
#define ARM_POP7(p, r1, r2, r3, r4, r5, r6, r7) ARM_POP(p, (1 << r1) | (1 << r2) | (1 << r3) | (1 << r4) | (1 << r5) | (1 << r6) | (1 << r7))
#define ARM_POP6(p, r1, r2, r3, r4, r5, r6) ARM_POP(p, (1 << r1) | (1 << r2) | (1 << r3) | (1 << r4) | (1 << r5) | (1 << r6))
#define ARM_POP5(p, r1, r2, r3, r4, r5) ARM_POP(p, (1 << r1) | (1 << r2) | (1 << r3) | (1 << r4) | (1 << r5))
#define ARM_POP4(p, r1, r2, r3, r4) ARM_POP(p, (1 << r1) | (1 << r2) | (1 << r3) | (1 << r4))
#define ARM_POP3(p, r1, r2, r3) ARM_POP(p, (1 << r1) | (1 << r2) | (1 << r3))
#define ARM_POP2(p, r1, r2) ARM_POP(p, (1 << r1) | (1 << r2))
#define ARM_POP1(p, r1) ARM_POP(p, (1 << r1))


/* Multiply instructions */
typedef struct {
  arminstr_t rm     : 4;
  arminstr_t tag2   : 4;   /* 9 */
  arminstr_t rs     : 4;
  arminstr_t rn     : 4;
  arminstr_t rd     : 4;
  arminstr_t s      : 1;
  arminstr_t opcode : 3;
  arminstr_t tag    : 4;
  arminstr_t cond   : 4;
} ARMInstrMul;

#define ARM_MUL_ID 0
#define ARM_MUL_ID2 9
#define ARM_MUL_MASK ((0xF << 24) | (0xF << 4))
#define ARM_MUL_TAG ((ARM_MUL_ID << 24) | (ARM_MUL_ID2 << 4))

#define ARM_DEF_MUL_COND(op, rd, rm, rs, rn, s, cond) \
  (rm)               | \
  ((rs) << 8)        | \
  ((rn) << 12)       | \
  ((rd) << 16)       | \
  (((s) & 1) << 20)  | \
  (((op) & 7) << 21) | \
  ARM_MUL_TAG        | \
  ARM_DEF_COND(cond)

/* Rd := (Rm * Rs)[31:0]; 32 x 32 -> 32 */
#define ARM_MUL_COND(p, rd, rm, rs, cond) \
  ARM_EMIT(p, ARM_DEF_MUL_COND(ARMOP_MUL, rd, rm, rs, 0, 0, cond))
#define ARM_MUL(p, rd, rm, rs) \
  ARM_MUL_COND(p, rd, rm, rs, ARMCOND_AL)
#define ARM_MULS_COND(p, rd, rm, rs, cond) \
  ARM_EMIT(p, ARM_DEF_MUL_COND(ARMOP_MUL, rd, rm, rs, 0, 1, cond))
#define ARM_MULS(p, rd, rm, rs) \
  ARM_MULS_COND(p, rd, rm, rs, ARMCOND_AL)
#define ARM_MUL_REG_REG(p, rd, rm, rs) ARM_MUL(p, rd, rm, rs)
#define ARM_MULS_REG_REG(p, rd, rm, rs) ARM_MULS(p, rd, rm, rs)

/* inline */
#define ARM_IASM_MUL_COND(rd, rm, rs, cond) \
  ARM_IASM_EMIT(ARM_DEF_MUL_COND(ARMOP_MUL, rd, rm, rs, 0, 0, cond))
#define ARM_IASM_MUL(rd, rm, rs) \
  ARM_IASM_MUL_COND(rd, rm, rs, ARMCOND_AL)
#define ARM_IASM_MULS_COND(rd, rm, rs, cond) \
  ARM_IASM_EMIT(ARM_DEF_MUL_COND(ARMOP_MUL, rd, rm, rs, 0, 1, cond))
#define ARM_IASM_MULS(rd, rm, rs) \
  ARM_IASM_MULS_COND(rd, rm, rs, ARMCOND_AL)


/* Rd := (Rm * Rs) + Rn; 32x32+32->32 */
#define ARM_MLA_COND(p, rd, rm, rs, rn, cond) \
  ARM_EMIT(p, ARM_DEF_MUL_COND(ARMOP_MLA, rd, rm, rs, rn, 0, cond))
#define ARM_MLA(p, rd, rm, rs, rn) \
  ARM_MLA_COND(p, rd, rm, rs, rn, ARMCOND_AL)
#define ARM_MLAS_COND(p, rd, rm, rs, rn, cond) \
  ARM_EMIT(p, ARM_DEF_MUL_COND(ARMOP_MLA, rd, rm, rs, rn, 1, cond))
#define ARM_MLAS(p, rd, rm, rs, rn) \
  ARM_MLAS_COND(p, rd, rm, rs, rn, ARMCOND_AL)

/* inline */
#define ARM_IASM_MLA_COND(rd, rm, rs, rn, cond) \
  ARM_IASM_EMIT(ARM_DEF_MUL_COND(ARMOP_MLA, rd, rm, rs, rn, 0, cond))
#define ARM_IASM_MLA(rd, rm, rs, rn) \
  ARM_IASM_MLA_COND(rd, rm, rs, rn, ARMCOND_AL)
#define ARM_IASM_MLAS_COND(rd, rm, rs, rn, cond) \
  ARM_IASM_EMIT(ARM_DEF_MUL_COND(ARMOP_MLA, rd, rm, rs, rn, 1, cond))
#define ARM_IASM_MLAS(rd, rm, rs, rn) \
  ARM_IASM_MLAS_COND(rd, rm, rs, rn, ARMCOND_AL)

/* Rd := Rn - (Rm * Rs); 32x32+32->32 */
#define ARM_MLS_COND(p, rd, rm, rs, rn, cond) \
  ARM_EMIT(p, ARM_DEF_MUL_COND(ARMOP_MLS, rd, rm, rs, rn, 0, cond))
#define ARM_MLS(p, rd, rm, rs, rn) \
  ARM_MLS_COND(p, rd, rm, rs, rn, ARMCOND_AL)

#define ARM_SMULL_COND(p, rn, rd, rm, rs, cond) \
  ARM_EMIT(p, ARM_DEF_MUL_COND(ARMOP_SMULL, rd, rm, rs, rn, 0, cond))
#define ARM_SMULL(p, rn, rd, rm, rs) \
  ARM_SMULL_COND(p, rn, rd, rm, rs, ARMCOND_AL)

#define ARM_SMLAL_COND(p, rn, rd, rm, rs, cond) \
  ARM_EMIT(p, ARM_DEF_MUL_COND(ARMOP_SMLAL, rd, rm, rs, rn, 0, cond))
#define ARM_SMLAL(p, rn, rd, rm, rs) \
  ARM_SMLAL_COND(p, rn, rd, rm, rs, ARMCOND_AL)

#define ARM_UMULL_COND(p, rn, rd, rm, rs, cond) \
  ARM_EMIT(p, ARM_DEF_MUL_COND(ARMOP_UMULL, rd, rm, rs, rn, 0, cond))
#define ARM_UMULL(p, rn, rd, rm, rs) \
  ARM_UMULL_COND(p, rn, rd, rm, rs, ARMCOND_AL)

#define ARM_UMLAL_COND(p, rn, rd, rm, rs, cond) \
  ARM_EMIT(p, ARM_DEF_MUL_COND(ARMOP_UMLAL, rd, rm, rs, rn, 0, cond))
#define ARM_UMLAL(p, rn, rd, rm, rs) \
  ARM_UMLAL_COND(p, rn, rd, rm, rs, ARMCOND_AL)


#define ARM_SMULLS_COND(p, rn, rd, rm, rs, cond) \
  ARM_EMIT(p, ARM_DEF_MUL_COND(ARMOP_SMULL, rd, rm, rs, rn, 1, cond))
#define ARM_SMULLS(p, rn, rd, rm, rs) \
  ARM_SMULLS_COND(p, rn, rd, rm, rs, ARMCOND_AL)

#define ARM_SMLALS_COND(p, rn, rd, rm, rs, cond) \
  ARM_EMIT(p, ARM_DEF_MUL_COND(ARMOP_SMLAL, rd, rm, rs, rn, 1, cond))
#define ARM_SMLALS(p, rn, rd, rm, rs) \
  ARM_SMLALS_COND(p, rn, rd, rm, rs, ARMCOND_AL)

#define ARM_UMULLS_COND(p, rn, rd, rm, rs, cond) \
  ARM_EMIT(p, ARM_DEF_MUL_COND(ARMOP_UMULL, rd, rm, rs, rn, 1, cond))
#define ARM_UMULLS(p, rn, rd, rm, rs) \
  ARM_UMULLS_COND(p, rn, rd, rm, rs, ARMCOND_AL)

#define ARM_UMLALS_COND(p, rn, rd, rm, rs, cond) \
  ARM_EMIT(p, ARM_DEF_MUL_COND(ARMOP_UMLAL, rd, rm, rs, rn, 1, cond))
#define ARM_UMLALS(p, rn, rd, rm, rs) \
  ARM_UMLALS_COND(p, rn, rd, rm, rs, ARMCOND_AL)



/*  Word/byte transfer */
typedef union {
  ARMDPI_op2_reg_imm op2_reg_imm;
  struct
  {
#ifdef MSB_FIRST
    arminstr_t cond    :  4;
    arminstr_t tag     :  2; /* 0 1 */
    arminstr_t type    :  1; /* imm(0) / register(1) */
    arminstr_t p       :  1; /* post-index(0) / pre-index(1) */
    arminstr_t u       :  1; /* down(0) / up(1) */
    arminstr_t b       :  1;
    arminstr_t wb      :  1;
    arminstr_t ls      :  1;
    arminstr_t rn      :  4;
    arminstr_t rd      :  4;
    arminstr_t op2_imm : 12;
#else
    arminstr_t op2_imm : 12;
    arminstr_t rd      :  4;
    arminstr_t rn      :  4;
    arminstr_t ls      :  1;
    arminstr_t wb      :  1;
    arminstr_t b       :  1;
    arminstr_t u       :  1; /* down(0) / up(1) */
    arminstr_t p       :  1; /* post-index(0) / pre-index(1) */
    arminstr_t type    :  1; /* imm(0) / register(1) */
    arminstr_t tag     :  2; /* 0 1 */
    arminstr_t cond    :  4;
#endif
  } all;
} ARMInstrWXfer;

#define ARM_WXFER_ID 1
#define ARM_WXFER_MASK 3 << 26
#define ARM_WXFER_TAG ARM_WXFER_ID << 26


/*
 * ls    :  opcode, ARMOP_STR(0)/ARMOP_LDR(1)
 * imm12 :  immediate offset
 * wb    :  write-back
 * p     :  index mode, post-index (0, automatic write-back)
 *        or pre-index (1, calc effective address before memory access)
 */
#define ARM_DEF_WXFER_IMM(imm12, rd, rn, ls, wb, b, p, cond) \
  ((((int)(imm12)) < 0) ? -((int)(imm12)) : (imm12)) | \
  ((rd) << 12)                                   | \
  ((rn) << 16)                                   | \
  ((ls) << 20)                                   | \
  ((wb) << 21)                                   | \
  ((b)  << 22)                                   | \
  (((int)(imm12) >= 0) << 23)                    | \
  ((p) << 24)                                    | \
  ARM_WXFER_TAG                                  | \
  ARM_DEF_COND(cond)

#define ARM_WXFER_MAX_OFFS 0xFFF

/* this macro checks for imm12 bounds */
#define ARM_EMIT_WXFER_IMM(ptr, imm12, rd, rn, ls, wb, b, p, cond) \
  do { \
    int _imm12 = (int)(imm12) < -ARM_WXFER_MAX_OFFS  \
                 ? -ARM_WXFER_MAX_OFFS               \
                 : (int)(imm12) > ARM_WXFER_MAX_OFFS \
                 ? ARM_WXFER_MAX_OFFS                \
                 : (int)(imm12);                     \
    ARM_EMIT((ptr), \
    ARM_DEF_WXFER_IMM(_imm12, (rd), (rn), (ls), (wb), (b), (p), (cond))); \
  } while (0)


/* LDRx */
/* immediate offset, post-index */
#define ARM_LDR_IMM_POST_COND(p, rd, rn, imm, cond) \
  ARM_EMIT(p, ARM_DEF_WXFER_IMM(imm, rd, rn, ARMOP_LDR, 0, 0, 0, cond))

#define ARM_LDR_IMM_POST(p, rd, rn, imm) ARM_LDR_IMM_POST_COND(p, rd, rn, imm, ARMCOND_AL)

#define ARM_LDRB_IMM_POST_COND(p, rd, rn, imm, cond) \
  ARM_EMIT(p, ARM_DEF_WXFER_IMM(imm, rd, rn, ARMOP_LDR, 0, 1, 0, cond))

#define ARM_LDRB_IMM_POST(p, rd, rn, imm) ARM_LDRB_IMM_POST_COND(p, rd, rn, imm, ARMCOND_AL)

/* immediate offset, pre-index */
#define ARM_LDR_IMM_COND(p, rd, rn, imm, cond) \
  ARM_EMIT(p, ARM_DEF_WXFER_IMM(imm, rd, rn, ARMOP_LDR, 0, 0, 1, cond))

#define ARM_LDR_IMM(p, rd, rn, imm) ARM_LDR_IMM_COND(p, rd, rn, imm, ARMCOND_AL)

#define ARM_LDRB_IMM_COND(p, rd, rn, imm, cond) \
  ARM_EMIT(p, ARM_DEF_WXFER_IMM(imm, rd, rn, ARMOP_LDR, 0, 1, 1, cond))

#define ARM_LDRB_IMM(p, rd, rn, imm) ARM_LDRB_IMM_COND(p, rd, rn, imm, ARMCOND_AL)


/* STRx */
/* immediate offset, post-index */
#define ARM_STR_IMM_POST_COND(p, rd, rn, imm, cond) \
  ARM_EMIT(p, ARM_DEF_WXFER_IMM(imm, rd, rn, ARMOP_STR, 0, 0, 0, cond))

#define ARM_STR_IMM_POST(p, rd, rn, imm) ARM_STR_IMM_POST_COND(p, rd, rn, imm, ARMCOND_AL)

#define ARM_STRB_IMM_POST_COND(p, rd, rn, imm, cond) \
  ARM_EMIT(p, ARM_DEF_WXFER_IMM(imm, rd, rn, ARMOP_STR, 0, 1, 0, cond))

#define ARM_STRB_IMM_POST(p, rd, rn, imm) ARM_STRB_IMM_POST_COND(p, rd, rn, imm, ARMCOND_AL)

/* immediate offset, pre-index */
#define ARM_STR_IMM_COND(p, rd, rn, imm, cond) \
  ARM_EMIT_WXFER_IMM(p, imm, rd, rn, ARMOP_STR, 0, 0, 1, cond)
/*  ARM_EMIT(p, ARM_DEF_WXFER_IMM(imm, rd, rn, ARMOP_STR, 0, 0, 1, cond))*/
/*  ARM_EMIT_WXFER_IMM(p, imm, rd, rn, ARMOP_STR, 0, 0, 1, cond) */
/*  ARM_EMIT(p, ARM_DEF_WXFER_IMM(imm, rd, rn, ARMOP_STR, 0, 0, 1, cond)) */

#define ARM_STR_IMM(p, rd, rn, imm) ARM_STR_IMM_COND(p, rd, rn, imm, ARMCOND_AL)

#define ARM_STRB_IMM_COND(p, rd, rn, imm, cond) \
  ARM_EMIT(p, ARM_DEF_WXFER_IMM(imm, rd, rn, ARMOP_STR, 0, 1, 1, cond))

#define ARM_STRB_IMM(p, rd, rn, imm) ARM_STRB_IMM_COND(p, rd, rn, imm, ARMCOND_AL)

/* write-back */
#define ARM_STR_IMM_WB_COND(p, rd, rn, imm, cond) \
  ARM_EMIT_WXFER_IMM(p, imm, rd, rn, ARMOP_STR, 1, 0, 1, cond)
#define ARM_STR_IMM_WB(p, rd, rn, imm) ARM_STR_IMM_WB_COND(p, rd, rn, imm, ARMCOND_AL)


/*
 * wb    :  write-back
 * u     :  down(0) / up(1)
 * p     :  index mode, post-index (0, automatic write-back) or pre-index (1)
 */
#define ARM_DEF_WXFER_REG_REG_UPDOWN_COND(rm, shift_type, shift, rd, rn, ls, wb, b, u, p, cond) \
  (rm)                | \
  ((shift_type) << 5) | \
  ((shift) << 7)      | \
  ((rd) << 12)        | \
  ((rn) << 16)        | \
  ((ls) << 20)        | \
  ((wb) << 21)        | \
  ((b)  << 22)        | \
  ((u)  << 23)        | \
  ((p)  << 24)        | \
  (1    << 25)        | \
  ARM_WXFER_TAG       | \
  ARM_DEF_COND(cond)

#define ARM_DEF_WXFER_REG_REG_COND(rm, shift_type, shift, rd, rn, ls, wb, b, p, cond) \
  ARM_DEF_WXFER_REG_REG_UPDOWN_COND(rm, shift_type, shift, rd, rn, ls, wb, b, ARM_UP, p, cond)
#define ARM_DEF_WXFER_REG_MINUS_REG_COND(rm, shift_type, shift, rd, rn, ls, wb, b, p, cond) \
  ARM_DEF_WXFER_REG_REG_UPDOWN_COND(rm, shift_type, shift, rd, rn, ls, wb, b, ARM_DOWN, p, cond)


#define ARM_LDR_REG_REG_SHIFT_COND(p, rd, rn, rm, shift_type, shift, cond) \
  ARM_EMIT(p, ARM_DEF_WXFER_REG_REG_COND(rm, shift_type, shift, rd, rn, ARMOP_LDR, 0, 0, 1, cond))
#define ARM_LDR_REG_REG_SHIFT(p, rd, rn, rm, shift_type, shift) \
  ARM_LDR_REG_REG_SHIFT_COND(p, rd, rn, rm, shift_type, shift, ARMCOND_AL)
#define ARM_LDR_REG_REG(p, rd, rn, rm) \
  ARM_LDR_REG_REG_SHIFT(p, rd, rn, rm, ARMSHIFT_LSL, 0)

#define ARM_LDRB_REG_REG_SHIFT_COND(p, rd, rn, rm, shift_type, shift, cond) \
  ARM_EMIT(p, ARM_DEF_WXFER_REG_REG_COND(rm, shift_type, shift, rd, rn, ARMOP_LDR, 0, 1, 1, cond))
#define ARM_LDRB_REG_REG_SHIFT(p, rd, rn, rm, shift_type, shift) \
  ARM_LDRB_REG_REG_SHIFT_COND(p, rd, rn, rm, shift_type, shift, ARMCOND_AL)
#define ARM_LDRB_REG_REG(p, rd, rn, rm) \
  ARM_LDRB_REG_REG_SHIFT(p, rd, rn, rm, ARMSHIFT_LSL, 0)

#define ARM_STR_REG_REG_SHIFT_COND(p, rd, rn, rm, shift_type, shift, cond) \
  ARM_EMIT(p, ARM_DEF_WXFER_REG_REG_COND(rm, shift_type, shift, rd, rn, ARMOP_STR, 0, 0, 1, cond))
#define ARM_STR_REG_REG_SHIFT(p, rd, rn, rm, shift_type, shift) \
  ARM_STR_REG_REG_SHIFT_COND(p, rd, rn, rm, shift_type, shift, ARMCOND_AL)
#define ARM_STR_REG_REG(p, rd, rn, rm) \
  ARM_STR_REG_REG_SHIFT(p, rd, rn, rm, ARMSHIFT_LSL, 0)

/* post-index */
#define ARM_STR_REG_REG_SHIFT_POST_COND(p, rd, rn, rm, shift_type, shift, cond) \
  ARM_EMIT(p, ARM_DEF_WXFER_REG_REG_COND(rm, shift_type, shift, rd, rn, ARMOP_STR, 0, 0, 0, cond))
#define ARM_STR_REG_REG_SHIFT_POST(p, rd, rn, rm, shift_type, shift) \
  ARM_STR_REG_REG_SHIFT_POST_COND(p, rd, rn, rm, shift_type, shift, ARMCOND_AL)
#define ARM_STR_REG_REG_POST(p, rd, rn, rm) \
  ARM_STR_REG_REG_SHIFT_POST(p, rd, rn, rm, ARMSHIFT_LSL, 0)

/* zero-extend */
#define ARM_STRB_REG_REG_SHIFT_COND(p, rd, rn, rm, shift_type, shift, cond) \
  ARM_EMIT(p, ARM_DEF_WXFER_REG_REG_COND(rm, shift_type, shift, rd, rn, ARMOP_STR, 0, 1, 1, cond))
#define ARM_STRB_REG_REG_SHIFT(p, rd, rn, rm, shift_type, shift) \
  ARM_STRB_REG_REG_SHIFT_COND(p, rd, rn, rm, shift_type, shift, ARMCOND_AL)
#define ARM_STRB_REG_REG(p, rd, rn, rm) \
  ARM_STRB_REG_REG_SHIFT(p, rd, rn, rm, ARMSHIFT_LSL, 0)


/* ARMv4+ */
/* Half-word or byte (signed) transfer. */
typedef struct {
  arminstr_t rm     : 4; /* imm_lo */
  arminstr_t tag3   : 1; /* 1 */
  arminstr_t h      : 1; /* half-word or byte */
  arminstr_t s      : 1; /* sign-extend or zero-extend */
  arminstr_t tag2   : 1; /* 1 */
  arminstr_t imm_hi : 4;
  arminstr_t rd     : 4;
  arminstr_t rn     : 4;
  arminstr_t ls     : 1;
  arminstr_t wb     : 1;
  arminstr_t type   : 1; /* imm(1) / reg(0) */
  arminstr_t u      : 1; /* +- */
  arminstr_t p      : 1; /* pre/post-index */
  arminstr_t tag    : 3;
  arminstr_t cond   : 4;
} ARMInstrHXfer;

#define ARM_HXFER_ID 0
#define ARM_HXFER_ID2 1
#define ARM_HXFER_ID3 1
#define ARM_HXFER_MASK ((0x7 << 25) | (0x9 << 4))
#define ARM_HXFER_TAG ((ARM_HXFER_ID << 25) | (ARM_HXFER_ID2 << 7) | (ARM_HXFER_ID3 << 4))

#define ARM_DEF_HXFER_IMM_COND(imm, h, s, rd, rn, ls, wb, p, cond) \
  (((int)(imm) >= 0 ? (imm) : -(int)(imm)) & 0xF)               | \
  ((h) << 5)                                                    | \
  ((s) << 6)                                                    | \
  ((((int)(imm) >= 0 ? (imm) : -(int)(imm)) << 4) & (0xF << 8)) | \
  ((rd) << 12)                                                  | \
  ((rn) << 16)                                                  | \
  ((ls) << 20)                                                  | \
  ((wb) << 21)                                                  | \
  (1 << 22)                                                     | \
  (((int)(imm) >= 0) << 23)                                     | \
  ((p) << 24)                                                   | \
  ARM_HXFER_TAG                                                 | \
  ARM_DEF_COND(cond)

#define ARM_LDRH_IMM_COND(p, rd, rn, imm, cond) \
  ARM_EMIT(p, ARM_DEF_HXFER_IMM_COND(imm, 1, 0, rd, rn, ARMOP_LDR, 0, 1, cond))
#define ARM_LDRH_IMM(p, rd, rn, imm) \
  ARM_LDRH_IMM_COND(p, rd, rn, imm, ARMCOND_AL)
#define ARM_LDRSH_IMM_COND(p, rd, rn, imm, cond) \
  ARM_EMIT(p, ARM_DEF_HXFER_IMM_COND(imm, 1, 1, rd, rn, ARMOP_LDR, 0, 1, cond))
#define ARM_LDRSH_IMM(p, rd, rn, imm) \
  ARM_LDRSH_IMM_COND(p, rd, rn, imm, ARMCOND_AL)
#define ARM_LDRSB_IMM_COND(p, rd, rn, imm, cond) \
  ARM_EMIT(p, ARM_DEF_HXFER_IMM_COND(imm, 0, 1, rd, rn, ARMOP_LDR, 0, 1, cond))
#define ARM_LDRSB_IMM(p, rd, rn, imm) \
  ARM_LDRSB_IMM_COND(p, rd, rn, imm, ARMCOND_AL)


#define ARM_STRH_IMM_COND(p, rd, rn, imm, cond) \
  ARM_EMIT(p, ARM_DEF_HXFER_IMM_COND(imm, 1, 0, rd, rn, ARMOP_STR, 0, 1, cond))
#define ARM_STRH_IMM(p, rd, rn, imm) \
  ARM_STRH_IMM_COND(p, rd, rn, imm, ARMCOND_AL)

#define ARM_STRH_IMM_POST_COND(p, rd, rn, imm, cond) \
  ARM_EMIT(p, ARM_DEF_HXFER_IMM_COND(imm, 1, 0, rd, rn, ARMOP_STR, 0, 0, cond))
#define ARM_STRH_IMM_POST(p, rd, rn, imm) \
  ARM_STRH_IMM_POST_COND(p, rd, rn, imm, ARMCOND_AL)


#define ARM_DEF_HXFER_REG_REG_UPDOWN_COND(rm, h, s, rd, rn, ls, wb, u, p, cond) \
  ((rm) & 0xF)                | \
  ((h) << 5)                  | \
  ((s) << 6)                  | \
  ((rd) << 12)                | \
  ((rn) << 16)                | \
  ((ls) << 20)                | \
  ((wb) << 21)                | \
  (0 << 22)                   | \
  ((u) << 23)                 | \
  ((p) << 24)                 | \
  ARM_HXFER_TAG               | \
  ARM_DEF_COND(cond)

#define ARM_DEF_HXFER_REG_REG_COND(rm, h, s, rd, rn, ls, wb, p, cond) \
  ARM_DEF_HXFER_REG_REG_UPDOWN_COND(rm, h, s, rd, rn, ls, wb, ARM_UP, p, cond)
#define ARM_DEF_HXFER_REG_MINUS_REG_COND(rm, h, s, rd, rn, ls, wb, p, cond) \
  ARM_DEF_HXFER_REG_REG_UPDOWN_COND(rm, h, s, rd, rn, ls, wb, ARM_DOWN, p, cond)

#define ARM_LDRH_REG_REG_COND(p, rd, rn, rm, cond) \
  ARM_EMIT(p, ARM_DEF_HXFER_REG_REG_COND(rm, 1, 0, rd, rn, ARMOP_LDR, 0, 1, cond))
#define ARM_LDRH_REG_REG(p, rd, rn, rm) \
  ARM_LDRH_REG_REG_COND(p, rd, rn, rm, ARMCOND_AL)
#define ARM_LDRSH_REG_REG_COND(p, rd, rn, rm, cond) \
  ARM_EMIT(p, ARM_DEF_HXFER_REG_REG_COND(rm, 1, 1, rd, rn, ARMOP_LDR, 0, 1, cond))
#define ARM_LDRSH_REG_REG(p, rd, rn, rm) \
  ARM_LDRSH_REG_REG_COND(p, rd, rn, rm, ARMCOND_AL)
#define ARM_LDRSB_REG_REG_COND(p, rd, rn, rm, cond) \
  ARM_EMIT(p, ARM_DEF_HXFER_REG_REG_COND(rm, 0, 1, rd, rn, ARMOP_LDR, 0, 1, cond))
#define ARM_LDRSB_REG_REG(p, rd, rn, rm) ARM_LDRSB_REG_REG_COND(p, rd, rn, rm, ARMCOND_AL)

#define ARM_STRH_REG_REG_COND(p, rd, rn, rm, cond) \
  ARM_EMIT(p, ARM_DEF_HXFER_REG_REG_COND(rm, 1, 0, rd, rn, ARMOP_STR, 0, 1, cond))
#define ARM_STRH_REG_REG(p, rd, rn, rm) \
  ARM_STRH_REG_REG_COND(p, rd, rn, rm, ARMCOND_AL)

#define ARM_STRH_REG_REG_POST_COND(p, rd, rn, rm, cond) \
  ARM_EMIT(p, ARM_DEF_HXFER_REG_REG_COND(rm, 1, 0, rd, rn, ARMOP_STR, 0, 0, cond))
#define ARM_STRH_REG_REG_POST(p, rd, rn, rm) \
  ARM_STRH_REG_REG_POST_COND(p, rd, rn, rm, ARMCOND_AL)



/* Swap */
typedef struct {
  arminstr_t rm   : 4;
  arminstr_t tag3 : 8; /* 0x9 */
  arminstr_t rd   : 4;
  arminstr_t rn   : 4;
  arminstr_t tag2 : 2;
  arminstr_t b    : 1;
  arminstr_t tag  : 5; /* 0x2 */
  arminstr_t cond : 4;
} ARMInstrSwap;

#define ARM_SWP_ID 2
#define ARM_SWP_ID2 9
#define ARM_SWP_MASK ((0x1F << 23) | (3 << 20) | (0xFF << 4))
#define ARM_SWP_TAG ((ARM_SWP_ID << 23) | (ARM_SWP_ID2 << 4))



/* Software interrupt */
typedef struct {
  arminstr_t num  : 24;
  arminstr_t tag  :  4;
  arminstr_t cond :  4;
} ARMInstrSWI;

#define ARM_SWI_ID 0xF
#define ARM_SWI_MASK (0xF << 24)
#define ARM_SWI_TAG (ARM_SWI_ID << 24)



/* Co-processor Data Processing */
typedef struct {
  arminstr_t crm  : 4;
  arminstr_t tag2 : 1; /* 0 */
  arminstr_t op2  : 3;
  arminstr_t cpn  : 4; /* CP number */
  arminstr_t crd  : 4;
  arminstr_t crn  : 4;
  arminstr_t op   : 4;
  arminstr_t tag  : 4; /* 0xE */
  arminstr_t cond : 4;
} ARMInstrCDP;

#define ARM_CDP_ID 0xE
#define ARM_CDP_ID2 0
#define ARM_CDP_MASK ((0xF << 24) | (1 << 4))
#define ARM_CDP_TAG ((ARM_CDP_ID << 24) | (ARM_CDP_ID2 << 4))


/* Co-processor Data Transfer (ldc/stc) */
typedef struct {
  arminstr_t offs : 8;
  arminstr_t cpn  : 4;
  arminstr_t crd  : 4;
  arminstr_t rn   : 4;
  arminstr_t ls   : 1;
  arminstr_t wb   : 1;
  arminstr_t n    : 1;
  arminstr_t u    : 1;
  arminstr_t p    : 1;
  arminstr_t tag  : 3;
  arminstr_t cond : 4;
} ARMInstrCDT;

#define ARM_CDT_ID 6
#define ARM_CDT_MASK (7 << 25)
#define ARM_CDT_TAG (ARM_CDT_ID << 25)


/* Co-processor Register Transfer (mcr/mrc) */
typedef struct {
  arminstr_t crm  : 4;
  arminstr_t tag2 : 1;
  arminstr_t op2  : 3;
  arminstr_t cpn  : 4;
  arminstr_t rd   : 4;
  arminstr_t crn  : 4;
  arminstr_t ls   : 1;
  arminstr_t op1  : 3;
  arminstr_t tag  : 4;
  arminstr_t cond : 4;
} ARMInstrCRT;

#define ARM_CRT_ID 0xE
#define ARM_CRT_ID2 0x1
#define ARM_CRT_MASK ((0xF << 24) | (1 << 4))
#define ARM_CRT_TAG ((ARM_CRT_ID << 24) | (ARM_CRT_ID2 << 4))

/*
 * Move from co-processor register to CPU register
 * Rd := cRn {<op>cRm}
 * op{condition} CP#,CPOp,Rd,CRn,CRm{,CPOp2}
 */
#define ARM_DEF_MRC_COND(cpn, cpop, rd, crn, crm, cpop2, cond) \
  ((crm) & 0xF)       |\
  ((cpop2) << 5)      |\
  ((cpn) << 8)        |\
  ((rd) << 12)        |\
  ((crn) << 16)       |\
  ((ARMOP_LDR) << 20) |\
  ((cpop) << 21)      |\
  ARM_CRT_TAG         |\
  ARM_DEF_COND(cond)

#define ARM_MRC_COND(p, cpn, cpop, rd, crn, crm, cpop2, cond) \
  ARM_EMIT(p, ARM_DEF_MRC_COND(cpn, cpop, rd, crn, crm, cpop2, cond))
#define ARM_MRC(p, cpn, cpop, rd, crn, crm, cpop2) \
  ARM_MRC_COND(p, cpn, cpop, rd, crn, crm, cpop2, ARMCOND_AL)



/* Move register to PSR. */
typedef union {
  ARMDPI_op2_imm op2_imm;
  struct
  {
#ifdef MSB_FIRST
    arminstr_t cond : 4;
    arminstr_t tag  : 2; /* 0 */
    arminstr_t type : 1;
    arminstr_t tag2 : 2; /* 0x2 */
    arminstr_t sel  : 1;
    arminstr_t tag3 : 2; /* 0x2 */
    arminstr_t fld  : 4;
    arminstr_t tag4 : 4; /* 0xF */
    arminstr_t pad  : 8; /* 0 */
    arminstr_t rm   : 4;
#else
    arminstr_t rm   : 4;
    arminstr_t pad  : 8; /* 0 */
    arminstr_t tag4 : 4; /* 0xF */
    arminstr_t fld  : 4;
    arminstr_t tag3 : 2; /* 0x2 */
    arminstr_t sel  : 1;
    arminstr_t tag2 : 2; /* 0x2 */
    arminstr_t type : 1;
    arminstr_t tag  : 2; /* 0 */
    arminstr_t cond : 4;
#endif
  } all;
} ARMInstrMSR;

#define ARM_MSR_ID 0
#define ARM_MSR_ID2 2
#define ARM_MSR_ID3 2
#define ARM_MSR_ID4 0xF
#define ARM_MSR_MASK ((3 << 26) | \
                      (3 << 23) | \
                      (3 << 20) | \
                      (0xF << 12))
#define ARM_MSR_TAG ((ARM_MSR_ID << 26)  | \
                     (ARM_MSR_ID2 << 23) | \
                     (ARM_MSR_ID3 << 20) | \
                     (ARM_MSR_ID4 << 12))

#define ARM_DEF_MSR_REG_COND(mask, rm, r, cond) \
    ARM_MSR_TAG       | \
    ARM_DEF_COND(cond)    | \
    ((rm) & 0xf)      | \
    (((r) & 1) << 22)   | \
    (((mask) & 0xf) << 16)

#define ARM_MSR_REG_COND(p, mask, rm, r, cond) \
  ARM_EMIT(p, ARM_DEF_MSR_REG_COND(mask, rm, r, cond))

#define ARM_MSR_REG(p, mask, rm, r) \
  ARM_MSR_REG_COND(p, mask, rm, r, ARMCOND_AL)

#define ARM_PSR_C 1
#define ARM_PSR_X 2
#define ARM_PSR_S 4
#define ARM_PSR_F 8

#define ARM_CPSR 0
#define ARM_SPSR 1

/* Move PSR to register. */
typedef struct {
  arminstr_t tag3 : 12;
  arminstr_t rd   :  4;
  arminstr_t tag2 :  6;
  arminstr_t sel  :  1; /* CPSR | SPSR */
  arminstr_t tag  :  5;
  arminstr_t cond :  4;
} ARMInstrMRS;

#define ARM_MRS_ID 2
#define ARM_MRS_ID2 0xF
#define ARM_MRS_ID3 0
#define ARM_MRS_MASK ((0x1F << 23) | (0x3F << 16) | 0xFFF)
#define ARM_MRS_TAG ((ARM_MRS_ID << 23) | (ARM_MRS_ID2 << 16) | ARM_MRS_ID3)

#define ARM_DEF_MRS_COND(rd, r, cond) \
  ARM_MRS_TAG       | \
  ARM_DEF_COND(cond)    | \
  (((r) & 1) << 22)   | \
  ((rd)& 0xf) << 12

#define ARM_MRS_COND(p, rd, r, cond) \
  ARM_EMIT(p, ARM_DEF_MRS_COND(rd, r, cond))

#define ARM_MRS_CPSR_COND(p, rd, cond) \
  ARM_MRS_COND(p, rd, ARM_CPSR, cond)

#define ARM_MRS_CPSR(p, rd) \
  ARM_MRS_CPSR_COND(p, rd, ARMCOND_AL)

#define ARM_MRS_SPSR_COND(p, rd, cond) \
  ARM_MRS_COND(p, rd, ARM_SPSR, cond)

#define ARM_MRS_SPSR(p, rd) \
  ARM_MRS_SPSR_COND(p, rd, ARMCOND_AL)


#include "arm_dpimacros.h"

#define ARM_NOP(p) ARM_MOV_REG_REG(p, ARMREG_R0, ARMREG_R0)


#define ARM_SHL_IMM_COND(p, rd, rm, imm, cond) \
  ARM_MOV_REG_IMMSHIFT_COND(p, rd, rm, ARMSHIFT_LSL, imm, cond)
#define ARM_SHL_IMM(p, rd, rm, imm) \
  ARM_SHL_IMM_COND(p, rd, rm, imm, ARMCOND_AL)
#define ARM_SHLS_IMM_COND(p, rd, rm, imm, cond) \
  ARM_MOVS_REG_IMMSHIFT_COND(p, rd, rm, ARMSHIFT_LSL, imm, cond)
#define ARM_SHLS_IMM(p, rd, rm, imm) \
  ARM_SHLS_IMM_COND(p, rd, rm, imm, ARMCOND_AL)

#define ARM_SHR_IMM_COND(p, rd, rm, imm, cond) \
  ARM_MOV_REG_IMMSHIFT_COND(p, rd, rm, ARMSHIFT_LSR, imm, cond)
#define ARM_SHR_IMM(p, rd, rm, imm) \
  ARM_SHR_IMM_COND(p, rd, rm, imm, ARMCOND_AL)
#define ARM_SHRS_IMM_COND(p, rd, rm, imm, cond) \
  ARM_MOVS_REG_IMMSHIFT_COND(p, rd, rm, ARMSHIFT_LSR, imm, cond)
#define ARM_SHRS_IMM(p, rd, rm, imm) \
  ARM_SHRS_IMM_COND(p, rd, rm, imm, ARMCOND_AL)

#define ARM_SAR_IMM_COND(p, rd, rm, imm, cond) \
  ARM_MOV_REG_IMMSHIFT_COND(p, rd, rm, ARMSHIFT_ASR, imm, cond)
#define ARM_SAR_IMM(p, rd, rm, imm) \
  ARM_SAR_IMM_COND(p, rd, rm, imm, ARMCOND_AL)
#define ARM_SARS_IMM_COND(p, rd, rm, imm, cond) \
  ARM_MOVS_REG_IMMSHIFT_COND(p, rd, rm, ARMSHIFT_ASR, imm, cond)
#define ARM_SARS_IMM(p, rd, rm, imm) \
  ARM_SARS_IMM_COND(p, rd, rm, imm, ARMCOND_AL)

#define ARM_ROR_IMM_COND(p, rd, rm, imm, cond) \
  ARM_MOV_REG_IMMSHIFT_COND(p, rd, rm, ARMSHIFT_ROR, imm, cond)
#define ARM_ROR_IMM(p, rd, rm, imm) \
  ARM_ROR_IMM_COND(p, rd, rm, imm, ARMCOND_AL)
#define ARM_RORS_IMM_COND(p, rd, rm, imm, cond) \
  ARM_MOVS_REG_IMMSHIFT_COND(p, rd, rm, ARMSHIFT_ROR, imm, cond)
#define ARM_RORS_IMM(p, rd, rm, imm) \
  ARM_RORS_IMM_COND(p, rd, rm, imm, ARMCOND_AL)

#define ARM_SHL_REG_COND(p, rd, rm, rs, cond) \
  ARM_MOV_REG_REGSHIFT_COND(p, rd, rm, ARMSHIFT_LSL, rs, cond)
#define ARM_SHL_REG(p, rd, rm, rs) \
  ARM_SHL_REG_COND(p, rd, rm, rs, ARMCOND_AL)
#define ARM_SHLS_REG_COND(p, rd, rm, rs, cond) \
  ARM_MOVS_REG_REGSHIFT_COND(p, rd, rm, ARMSHIFT_LSL, rs, cond)
#define ARM_SHLS_REG(p, rd, rm, rs) \
  ARM_SHLS_REG_COND(p, rd, rm, rs, ARMCOND_AL)
#define ARM_SHLS_REG_REG(p, rd, rm, rs) ARM_SHLS_REG(p, rd, rm, rs)

#define ARM_SHR_REG_COND(p, rd, rm, rs, cond) \
  ARM_MOV_REG_REGSHIFT_COND(p, rd, rm, ARMSHIFT_LSR, rs, cond)
#define ARM_SHR_REG(p, rd, rm, rs) \
  ARM_SHR_REG_COND(p, rd, rm, rs, ARMCOND_AL)
#define ARM_SHRS_REG_COND(p, rd, rm, rs, cond) \
  ARM_MOVS_REG_REGSHIFT_COND(p, rd, rm, ARMSHIFT_LSR, rs, cond)
#define ARM_SHRS_REG(p, rd, rm, rs) \
  ARM_SHRS_REG_COND(p, rd, rm, rs, ARMCOND_AL)
#define ARM_SHRS_REG_REG(p, rd, rm, rs) ARM_SHRS_REG(p, rd, rm, rs)

#define ARM_SAR_REG_COND(p, rd, rm, rs, cond) \
  ARM_MOV_REG_REGSHIFT_COND(p, rd, rm, ARMSHIFT_ASR, rs, cond)
#define ARM_SAR_REG(p, rd, rm, rs) \
  ARM_SAR_REG_COND(p, rd, rm, rs, ARMCOND_AL)
#define ARM_SARS_REG_COND(p, rd, rm, rs, cond) \
  ARM_MOVS_REG_REGSHIFT_COND(p, rd, rm, ARMSHIFT_ASR, rs, cond)
#define ARM_SARS_REG(p, rd, rm, rs) \
  ARM_SARS_REG_COND(p, rd, rm, rs, ARMCOND_AL)
#define ARM_SARS_REG_REG(p, rd, rm, rs) ARM_SARS_REG(p, rd, rm, rs)

#define ARM_ROR_REG_COND(p, rd, rm, rs, cond) \
  ARM_MOV_REG_REGSHIFT_COND(p, rd, rm, ARMSHIFT_ROR, rs, cond)
#define ARM_ROR_REG(p, rd, rm, rs) \
  ARM_ROR_REG_COND(p, rd, rm, rs, ARMCOND_AL)
#define ARM_RORS_REG_COND(p, rd, rm, rs, cond) \
  ARM_MOVS_REG_REGSHIFT_COND(p, rd, rm, ARMSHIFT_ROR, rs, cond)
#define ARM_RORS_REG(p, rd, rm, rs) \
  ARM_RORS_REG_COND(p, rd, rm, rs, ARMCOND_AL)
#define ARM_RORS_REG_REG(p, rd, rm, rs) ARM_RORS_REG(p, rd, rm, rs)

#define ARM_DBRK(p) ARM_EMIT(p, 0xE6000010)
#define ARM_IASM_DBRK() ARM_IASM_EMIT(0xE6000010)

#define ARM_INC(p, reg) ARM_ADD_REG_IMM8(p, reg, reg, 1)
#define ARM_DEC(p, reg) ARM_SUB_REG_IMM8(p, reg, reg, 1)


/* ARM V5 */

/* Count leading zeros, CLZ{cond} Rd, Rm */
typedef struct {
  arminstr_t rm   :  4;
  arminstr_t tag2 :  8;
  arminstr_t rd   :  4;
  arminstr_t tag  :  12;
  arminstr_t cond :  4;
} ARMInstrCLZ;

#define ARM_CLZ_ID 0x16F
#define ARM_CLZ_ID2 0xF1
#define ARM_CLZ_MASK ((0xFFF << 16) | (0xFF < 4))
#define ARM_CLZ_TAG ((ARM_CLZ_ID << 16) | (ARM_CLZ_ID2 << 4))

#define ARM_DEF_CLZ_COND(rd, rm, cond) \
  ARM_CLZ_TAG       | \
  ARM_DEF_COND(cond)    | \
  (((rm) & 0xf))      | \
  ((rd) & 0xf) << 12

#define ARM_CLZ_COND(p, rd, rm, cond) \
  ARM_EMIT(p, ARM_DEF_CLZ_COND(rd, rm, cond))

#define ARM_CLZ(p, rd, rm) \
  ARM_EMIT(p, ARM_DEF_CLZ_COND(rd, rm, ARMCOND_AL))

/*
 *              TAG     p         b   wb  ls
 * ARMCOND_NV | 0-1-0 | 0 | +/- | 1 | 0 | 1 | rn -|- 0xF | imm12
 */
#define ARM_PLD_ID 0xF45
#define ARM_PLD_ID2 0xF /* rd */
#define ARM_PLD_MASK ((0xFC7 << 20) | (0xF << 12))
#define ARM_PLD_TAG ((ARM_PLD_ID << 20) | (ARM_PLD_ID2 << 12))
#define ARM_DEF_PLD_IMM(imm12, rn) \
  ((((int)imm12) < 0) ? -(int)(imm12) : (imm12)) | \
  ((0xF) << 12)                                  | \
  ((rn) << 16)                                   | \
  ((1) << 20)  /* ls = load(1) */                | \
  ((0) << 21)  /* wb = 0 */                      | \
  ((1)  << 22) /* b = 1 */                       | \
  (((int)(imm12) >= 0) << 23)                    | \
  ((1) << 24)  /* pre/post = pre(1) */           | \
  ((2) << 25)  /* tag */                         | \
  ARM_DEF_COND(ARMCOND_NV)

#define ARM_PLD_IMM(p, rn, imm12) ARM_EMIT(p, ARM_DEF_PLD_IMM(imm12, rn))

#define ARM_DEF_PLD_REG_REG_UPDOWN_SHIFT(rn, shift_type, shift, rm, u) \
  (rm)                            | \
  ((shift_type) << 5)             | \
  ((shift) << 7)                  | \
  (0xF << 12) /* rd = 0xF */      | \
  ((rn) << 16)                    | \
  (1    << 20) /* ls = load(1) */ | \
  (0    << 21) /* wb = 0 */       | \
  (1    << 22) /* b = 1 */        | \
  ((u)  << 23)                    | \
  (1  << 24)   /* pre(1) */       | \
  (3    << 25)                    | \
  ARM_DEF_COND(ARMCOND_NV)

#define ARM_PLD_REG_REG_UPDOWN_SHIFT(p, rm, rn, u, shift_type, shift) \
  ARM_EMIT(p,  ARM_DEF_PLD_REG_REG_UPDOWN_SHIFT(rm, shift_type, shift, rn, u))

#define ARM_PLD_REG_PLUS_REG(p, rm, rn) \
  ARM_PLD_REG_REG_UPDOWN_SHIFT(p, rm, rn, ARM_UP, ARMSHIFT_LSL, 0)

#define ARM_PLD_REG_MINUS_REG(p, rm, rn) \
  ARM_PLD_REG_REG_UPDOWN_SHIFT(p, rm, rn, ARM_DOWN, ARMSHIFT_LSL, 0)


#define ARM_DEF_STF_IMM_COND(p, prec, freg_const, rd, imm8, rot, cond) \
  ((imm8) & 0xFF)      | \
  (((rot) & 0xF) << 8) | \
  ((freg_const) << 12) | \
  (1 << 25)            | \
  ARM_DEF_COND(cond)

#define ARM_USAT_ASR(p, rd, sat, rm, sa, cond) \
  ARM_EMIT(p, ARM_DEF_DPI_REG_IMMSHIFT_COND((rm) | 0x10, 2, sa, rd, sat, 0, 0x37, cond))

#define ARM_USAT_LSL(p, rd, sat, rm, sa, cond) \
  ARM_EMIT(p, ARM_DEF_DPI_REG_IMMSHIFT_COND((rm) | 0x10, 0, sa, rd, sat, 0, 0x37, cond))


typedef union {
  ARMInstrBR    br;
  ARMInstrDPI   dpi;
  ARMInstrMRT   mrt;
  ARMInstrMul   mul;
  ARMInstrWXfer wxfer;
  ARMInstrHXfer hxfer;
  ARMInstrSwap  swp;
  ARMInstrCDP   cdp;
  ARMInstrCDT   cdt;
  ARMInstrCRT   crt;
  ARMInstrSWI   swi;
  ARMInstrMSR   msr;
  ARMInstrMRS   mrs;
  ARMInstrCLZ   clz;

  ARMInstrGeneric generic;
  arminstr_t      raw;
} ARMInstr;

/* ARMv7VE and others */

#define ARM_SDIV_COND(p, rd, rm, rn, cond) \
  ARM_DEF_DPI_REG_REGSHIFT_COND(rm, 0, rn, 31, rd, 1, 0x38, cond)
#define ARM_SDIV(p, rd, rm, rn) \
  ARM_SDIV_COND(p, rd, rm, rn, ARMCOND_AL)

#define ARM_UDIV_COND(p, rd, rm, rn, cond) \
  ARM_DEF_DPI_REG_REGSHIFT_COND(rm, 0, rn, 31, rd, 1, 0x39, cond)
#define ARM_UDIV(p, rd, rm, rn) \
  ARM_UDIV_COND(p, rd, rm, rn, ARMCOND_AL)

#define ARM_MOVW(p, rd, imm) \
  ARM_EMIT(p, ((ARMCOND_AL << 28) | (0x30 << 20) | (((imm >> 12) & 0xF) << 16) | (imm & 0xFFF) | (rd << 12)))

#define ARM_MOVT(p, rd, imm) \
  ARM_EMIT(p, ((ARMCOND_AL << 28) | (0x34 << 20) | (((imm >> 12) & 0xF) << 16) | (imm & 0xFFF) | (rd << 12)))


#endif /* ARM_CG_H */

