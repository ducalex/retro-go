/* gameplaySP
 *
 * Copyright (C) 2006 Exophase <exophase@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

typedef enum
{
  mips_reg_zero =  0,
  mips_reg_at   =  1,
  mips_reg_v0   =  2,
  mips_reg_v1   =  3,
  mips_reg_a0   =  4,
  mips_reg_a1   =  5,
  mips_reg_a2   =  6,
  mips_reg_a3   =  7,
  mips_reg_t0   =  8,
  mips_reg_t1   =  9,
  mips_reg_t2   = 10,
  mips_reg_t3   = 11,
  mips_reg_t4   = 12,
  mips_reg_t5   = 13,
  mips_reg_t6   = 14,
  mips_reg_t7   = 15,
  mips_reg_s0   = 16,
  mips_reg_s1   = 17,
  mips_reg_s2   = 18,
  mips_reg_s3   = 19,
  mips_reg_s4   = 20,
  mips_reg_s5   = 21,
  mips_reg_s6   = 22,
  mips_reg_s7   = 23,
  mips_reg_t8   = 24,
  mips_reg_t9   = 25,
  mips_reg_k0   = 26,
  mips_reg_k1   = 27,
  mips_reg_gp   = 28,
  mips_reg_sp   = 29,
  mips_reg_fp   = 30,
  mips_reg_ra   = 31
} mips_reg_number;

typedef enum
{
  mips_special_sll       = 0x00,
  mips_special_srl       = 0x02,
  mips_special_sra       = 0x03,
  mips_special_sllv      = 0x04,
  mips_special_srlv      = 0x06,
  mips_special_srav      = 0x07,
  mips_special_jr        = 0x08,
  mips_special_jalr      = 0x09,
  mips_special_movz      = 0x0A,
  mips_special_movn      = 0x0B,
  mips_special_sync      = 0x0F,
  mips_special_mfhi      = 0x10,
  mips_special_mthi      = 0x11,
  mips_special_mflo      = 0x12,
  mips_special_mtlo      = 0x13,
  mips_special_mult      = 0x18,
  mips_special_multu     = 0x19,
  mips_special_div       = 0x1A,
  mips_special_divu      = 0x1B,
  mips_special_madd      = 0x1C,
  mips_special_maddu     = 0x1D,
  mips_special_add       = 0x20,
  mips_special_addu      = 0x21,
  mips_special_sub       = 0x22,
  mips_special_subu      = 0x23,
  mips_special_and       = 0x24,
  mips_special_or        = 0x25,
  mips_special_xor       = 0x26,
  mips_special_nor       = 0x27,
  mips_special_slt       = 0x2A,
  mips_special_sltu      = 0x2B,
  mips_special_max       = 0x2C,
  mips_special_min       = 0x2D,
} mips_function_special;

typedef enum
{
  mips_special2_madd     = 0x00,
  mips_special2_maddu    = 0x01,
} mips_function_special2;

typedef enum
{
  mips_special3_ext      = 0x00,
  mips_special3_ins      = 0x04,
  mips_special3_bshfl    = 0x20
} mips_function_special3;

typedef enum
{
  mips_bshfl_seb      = 0x10,
  mips_bshfl_seh      = 0x18,
  mips_bshfl_wsbh     = 0x02,
} mips_function_bshfl;

typedef enum
{
  mips_regimm_bltz       = 0x00,
  mips_regimm_bltzal     = 0x10,
  mips_regimm_bgezal     = 0x11,
  mips_regimm_synci      = 0x1F
} mips_function_regimm;

typedef enum
{
  mips_opcode_special    = 0x00,
  mips_opcode_regimm     = 0x01,
  mips_opcode_j          = 0x02,
  mips_opcode_jal        = 0x03,
  mips_opcode_beq        = 0x04,
  mips_opcode_bne        = 0x05,
  mips_opcode_blez       = 0x06,
  mips_opcode_bgtz       = 0x07,
  mips_opcode_addi       = 0x08,
  mips_opcode_addiu      = 0x09,
  mips_opcode_slti       = 0x0A,
  mips_opcode_sltiu      = 0x0B,
  mips_opcode_andi       = 0x0C,
  mips_opcode_ori        = 0x0D,
  mips_opcode_xori       = 0x0E,
  mips_opcode_lui        = 0x0F,
  mips_opcode_llo        = 0x18,
  mips_opcode_lhi        = 0x19,
  mips_opcode_trap       = 0x1A,
  mips_opcode_special2   = 0x1C,
  mips_opcode_special3   = 0x1F,
  mips_opcode_lb         = 0x20,
  mips_opcode_lh         = 0x21,
  mips_opcode_lw         = 0x23,
  mips_opcode_lbu        = 0x24,
  mips_opcode_lhu        = 0x25,
  mips_opcode_sb         = 0x28,
  mips_opcode_sh         = 0x29,
  mips_opcode_sw         = 0x2B,
  mips_opcode_cache      = 0x2F,
} mips_opcode;

#define mips_emit_cache(operation, rs, immediate)                             \
  *((u32 *)translation_ptr) = (mips_opcode_cache << 26) |                     \
   (rs << 21) | (operation << 16) | (immediate & 0xFFFF);                     \
  translation_ptr += 4                                                        \

#define mips_emit_reg(opcode, rs, rt, rd, shift, function)                    \
  *((u32 *)translation_ptr) = (mips_opcode_##opcode << 26) |                  \
  (rs << 21) | (rt << 16) | (rd << 11) | ((shift) << 6) | function;           \
  translation_ptr += 4                                                        \

#define mips_emit_special(function, rs, rt, rd, shift)                        \
  *((u32 *)translation_ptr) = (mips_opcode_special << 26) |                   \
   (rs << 21) | (rt << 16) | (rd << 11) | ((shift) << 6) |                    \
   mips_special_##function;                                                   \
  translation_ptr += 4                                                        \

#define mips_emit_special2(function, rs, rt, rd, shift)                       \
  *((u32 *)translation_ptr) = (mips_opcode_special2 << 26) |                  \
   (rs << 21) | (rt << 16) | (rd << 11) | ((shift) << 6) |                    \
   mips_special2_##function;                                                  \
  translation_ptr += 4                                                        \

#define mips_emit_special3(function, rs, rt, imm_a, imm_b)                    \
  *((u32 *)translation_ptr) = (mips_opcode_special3 << 26) |                  \
   (rs << 21) | (rt << 16) | (imm_a << 11) | (imm_b << 6) |                   \
   mips_special3_##function;                                                  \
  translation_ptr += 4                                                        \

#define mips_emit_imm(opcode, rs, rt, immediate)                              \
  *((u32 *)translation_ptr) = (mips_opcode_##opcode << 26) |                  \
   (rs << 21) | (rt << 16) | ((immediate) & 0xFFFF);                          \
  translation_ptr += 4                                                        \

#define mips_emit_regimm(function, rs, immediate)                             \
  *((u32 *)translation_ptr) = (mips_opcode_regimm << 26) |                    \
   (rs << 21) | (mips_regimm_##function << 16) | ((immediate) & 0xFFFF);      \
  translation_ptr += 4                                                        \

#define mips_emit_jump(opcode, offset)                                        \
  *((u32 *)translation_ptr) = (mips_opcode_##opcode << 26) |                  \
   (offset & 0x3FFFFFF);                                                      \
  translation_ptr += 4                                                        \

#define mips_relative_offset(source, offset)                                  \
  (((u32)offset - ((u32)source + 4)) / 4)                                     \

#define mips_absolute_offset(offset)                                          \
  ((u32)offset / 4)                                                           \

#define mips_emit_max(rd, rs, rt)                                             \
  mips_emit_special(max, rs, rt, rd, 0)                                       \

#define mips_emit_min(rd, rs, rt)                                             \
  mips_emit_special(min, rs, rt, rd, 0)                                       \

#define mips_emit_addu(rd, rs, rt)                                            \
  mips_emit_special(addu, rs, rt, rd, 0)                                      \

#define mips_emit_subu(rd, rs, rt)                                            \
  mips_emit_special(subu, rs, rt, rd, 0)                                      \

#define mips_emit_xor(rd, rs, rt)                                             \
  mips_emit_special(xor, rs, rt, rd, 0)                                       \

#define mips_emit_and(rd, rs, rt)                                             \
  mips_emit_special(and, rs, rt, rd, 0)                                       \

#define mips_emit_or(rd, rs, rt)                                              \
  mips_emit_special(or, rs, rt, rd, 0)                                        \

#define mips_emit_nor(rd, rs, rt)                                             \
  mips_emit_special(nor, rs, rt, rd, 0)                                       \

#define mips_emit_slt(rd, rs, rt)                                             \
  mips_emit_special(slt, rs, rt, rd, 0)                                       \

#define mips_emit_sltu(rd, rs, rt)                                            \
  mips_emit_special(sltu, rs, rt, rd, 0)                                      \

#define mips_emit_sllv(rd, rt, rs)                                            \
  mips_emit_special(sllv, rs, rt, rd, 0)                                      \

#define mips_emit_srlv(rd, rt, rs)                                            \
  mips_emit_special(srlv, rs, rt, rd, 0)                                      \

#define mips_emit_srav(rd, rt, rs)                                            \
  mips_emit_special(srav, rs, rt, rd, 0)                                      \

#define mips_emit_rotrv(rd, rt, rs)                                           \
  mips_emit_special(srlv, rs, rt, rd, 1)                                      \

#define mips_emit_sll(rd, rt, shift)                                          \
  mips_emit_special(sll, 0, rt, rd, shift)                                    \

#define mips_emit_srl(rd, rt, shift)                                          \
  mips_emit_special(srl, 0, rt, rd, shift)                                    \

#define mips_emit_sra(rd, rt, shift)                                          \
  mips_emit_special(sra, 0, rt, rd, shift)                                    \

#define mips_emit_rotr(rd, rt, shift)                                         \
  mips_emit_special(srl, 1, rt, rd, shift)                                    \

#define mips_emit_mfhi(rd)                                                    \
  mips_emit_special(mfhi, 0, 0, rd, 0)                                        \

#define mips_emit_mflo(rd)                                                    \
  mips_emit_special(mflo, 0, 0, rd, 0)                                        \

#define mips_emit_mthi(rs)                                                    \
  mips_emit_special(mthi, rs, 0, 0, 0)                                        \

#define mips_emit_mtlo(rs)                                                    \
  mips_emit_special(mtlo, rs, 0, 0, 0)                                        \

#define mips_emit_mult(rs, rt)                                                \
  mips_emit_special(mult, rs, rt, 0, 0)                                       \

#define mips_emit_multu(rs, rt)                                               \
  mips_emit_special(multu, rs, rt, 0, 0)                                      \

#define mips_emit_div(rs, rt)                                                 \
  mips_emit_special(div, rs, rt, 0, 0)                                        \

#define mips_emit_divu(rs, rt)                                                \
  mips_emit_special(divu, rs, rt, 0, 0)                                       \

#ifdef PSP
  #define mips_emit_madd(rs, rt)                                              \
    mips_emit_special(madd, rs, rt, 0, 0)                                     \

  #define mips_emit_maddu(rs, rt)                                             \
    mips_emit_special(maddu, rs, rt, 0, 0)
#else
  #define mips_emit_madd(rs, rt)                                              \
    mips_emit_special2(madd, rs, rt, 0, 0)                                    \

  #define mips_emit_maddu(rs, rt)                                             \
    mips_emit_special2(maddu, rs, rt, 0, 0)
#endif

#define mips_emit_movn(rd, rs, rt)                                            \
  mips_emit_special(movn, rs, rt, rd, 0)                                      \

#define mips_emit_movz(rd, rs, rt)                                            \
  mips_emit_special(movz, rs, rt, rd, 0)                                      \

#define mips_emit_sync()                                                      \
  mips_emit_special(sync, 0, 0, 0, 0)                                         \

#define mips_emit_lb(rt, rs, offset)                                          \
  mips_emit_imm(lb, rs, rt, offset)                                           \

#define mips_emit_lbu(rt, rs, offset)                                         \
  mips_emit_imm(lbu, rs, rt, offset)                                          \

#define mips_emit_lh(rt, rs, offset)                                          \
  mips_emit_imm(lh, rs, rt, offset)                                           \

#define mips_emit_lhu(rt, rs, offset)                                         \
  mips_emit_imm(lhu, rs, rt, offset)                                          \

#define mips_emit_lw(rt, rs, offset)                                          \
  mips_emit_imm(lw, rs, rt, offset)                                           \

#define mips_emit_sb(rt, rs, offset)                                          \
  mips_emit_imm(sb, rs, rt, offset)                                           \

#define mips_emit_sh(rt, rs, offset)                                          \
  mips_emit_imm(sh, rs, rt, offset)                                           \

#define mips_emit_sw(rt, rs, offset)                                          \
  mips_emit_imm(sw, rs, rt, offset)                                           \

#define mips_emit_lui(rt, imm)                                                \
  mips_emit_imm(lui, 0, rt, imm)                                              \

#define mips_emit_addiu(rt, rs, imm)                                          \
  mips_emit_imm(addiu, rs, rt, imm)                                           \

#define mips_emit_xori(rt, rs, imm)                                           \
  mips_emit_imm(xori, rs, rt, imm)                                            \

#define mips_emit_ori(rt, rs, imm)                                            \
  mips_emit_imm(ori, rs, rt, imm)                                             \

#define mips_emit_andi(rt, rs, imm)                                           \
  mips_emit_imm(andi, rs, rt, imm)                                            \

#define mips_emit_slti(rt, rs, imm)                                           \
  mips_emit_imm(slti, rs, rt, imm)                                            \

#define mips_emit_sltiu(rt, rs, imm)                                          \
  mips_emit_imm(sltiu, rs, rt, imm)                                           \

#define mips_emit_ext(rt, rs, pos, size)                                      \
  mips_emit_special3(ext, rs, rt, (size - 1), pos)                            \

#define mips_emit_ins(rt, rs, pos, size)                                      \
  mips_emit_special3(ins, rs, rt, (pos + size - 1), pos)                      \

#define mips_emit_seb(rd, rt)                                                 \
  mips_emit_special3(bshfl, 0, rt, rd, mips_bshfl_seb)                        \

#define mips_emit_seh(rd, rt)                                                 \
  mips_emit_special3(bshfl, 0, rt, rd, mips_bshfl_seh)                        \


// Breaks down if the backpatch offset is greater than 16bits, take care
// when using (should be okay if limited to conditional instructions)

#define mips_emit_b_filler(type, rs, rt, writeback_location)                  \
  (writeback_location) = translation_ptr;                                     \
  mips_emit_imm(type, rs, rt, 0)                                              \

// The backpatch code for this has to be handled differently than the above

#define mips_emit_j_filler(writeback_location)                                \
  (writeback_location) = translation_ptr;                                     \
  mips_emit_jump(j, 0)                                                        \

#define mips_emit_b(type, rs, rt, offset)                                     \
  mips_emit_imm(type, rs, rt, offset)                                         \

#define mips_emit_j(offset)                                                   \
  mips_emit_jump(j, offset)                                                   \

#define mips_emit_jal(offset)                                                 \
  mips_emit_jump(jal, offset)                                                 \

#define mips_emit_jr(rs)                                                      \
  mips_emit_special(jr, rs, 0, 0, 0)                                          \

#define mips_emit_jalr(rs)                                                    \
  mips_emit_special(jalr, rs, 0, 31, 0)                                       \

#define mips_emit_synci(rs, offset)                                           \
  mips_emit_regimm(synci, rs, offset)                                         \

#define mips_emit_bltzal(rs, offset)                                          \
  mips_emit_regimm(bltzal, rs, offset)                                        \

#define mips_emit_bgezal(rs, offset)                                          \
  mips_emit_regimm(bgezal, rs, offset)                                        \

#define mips_emit_bltz(rs, offset)                                            \
  mips_emit_regimm(bltz, rs, offset)                                          \

#define mips_emit_nop()                                                       \
  mips_emit_sll(mips_reg_zero, mips_reg_zero, 0)                              \


