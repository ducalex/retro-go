/* gameplaySP
 *
 * Copyright (C) 2021 David Guillen Fandos <david@davidgf.net>
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
  aa64_opcode_logic      = 0x0A,
  aa64_opcode_addsub     = 0x0B,
  aa64_opcode_adr        = 0x10,
  aa64_opcode_addsubi    = 0x11,
  aa64_opcode_movi       = 0x12,
  aa64_opcode_bfm        = 0x13,
  aa64_opcode_b          = 0x14,
  aa64_opcode_b2         = 0x15,
  aa64_opcode_tbz        = 0x16,
  aa64_opcode_tbnz       = 0x17,
  aa64_opcode_memi       = 0x19,
  aa64_opcode_misc       = 0x1A,
  aa64_opcode_mul4       = 0x1B,

} aa64_opcode;

typedef enum
{
  ccode_eq        = 0x0,  /* Equal       Z == 1 */
  ccode_ne        = 0x1,  /* Not Equal   Z == 0 */
  ccode_hs        = 0x2,  /* Carry Set   C == 1 */
  ccode_lo        = 0x3,  /* Carry Clear C == 0 */
  ccode_mi        = 0x4,  /* Minus/Neg   N == 1 */
  ccode_pl        = 0x5,  /* Plus/Pos    N == 0 */
  ccode_vs        = 0x6,  /* Overflow    V == 1 */
  ccode_vc        = 0x7,  /* !Overflow   V == 0 */
  ccode_hi        = 0x8,  /* UGreatThan C && !Z */
  ccode_ls        = 0x9,  /* ULessEqual !C || Z */
  ccode_ge        = 0xA,  /* SGreatEqual N == V */
  ccode_lt        = 0xB,  /* SLessThan   N != V */
  ccode_gt        = 0xC,  /* SLessThan   !Z&N==V  */
  ccode_le        = 0xD,  /* SLessEqual  Z|(N!=V) */
  ccode_al        = 0xE,  /* Always             */
  ccode_nv        = 0xF,  /* Never              */
} aa64_condcode;



#define aa64_br_offset(label)                                                 \
  (((uintptr_t)(label) - (uintptr_t)(translation_ptr)) >> 2)                  \

#define aa64_br_offset_from(label, from)                                      \
  (((uintptr_t)(label) - (uintptr_t)(from)) >> 2)                             \

#define aa64_emit_inst(opcode, ope, rd, rs, extra)                            \
{                                                                             \
  *((u32 *)translation_ptr) = (aa64_opcode_##opcode << 24) | ((ope) << 29) |  \
                                 ((rs) << 5) | (rd) | (extra);                \
  translation_ptr += 4;                                                       \
}

#define aa64_emit_ldr(rv, rb, offset)                                         \
  aa64_emit_inst(memi, 5, rv, rb, (1 << 22) | ((offset) << 10))               \

#define aa64_emit_str(rv, rb, offset)                                         \
  aa64_emit_inst(memi, 5, rv, rb, (0 << 22) | ((offset) << 10))               \

#define aa64_emit_addshift(rd, rs, rm, st, sa)                                \
  aa64_emit_inst(addsub, 0, rd, rs, ((rm) << 16) | ((st)<<22) | ((sa)<<10))   \

#define aa64_emit_add_lsl(rd, rs, rm, sa)                                     \
  aa64_emit_addshift(rd, rs, rm, 0, sa)                                       \

#define aa64_emit_addi(rd, rs, imm)                                           \
  aa64_emit_inst(addsubi, 0, rd, rs, (imm) << 10)                             \

#define aa64_emit_addi12(rd, rs, imm)                                         \
  aa64_emit_inst(addsubi, 0, rd, rs, ((imm) << 10) | (1 << 22))               \

#define aa64_emit_addis(rd, rs, imm)                                          \
  aa64_emit_inst(addsubi, 1, rd, rs, (imm) << 10)                             \

#define aa64_emit_subi(rd, rs, imm)                                           \
  aa64_emit_inst(addsubi, 2, rd, rs, (imm) << 10)                             \

#define aa64_emit_subi12(rd, rs, imm)                                         \
  aa64_emit_inst(addsubi, 2, rd, rs, ((imm) << 10) | (1 << 22))               \

#define aa64_emit_subis(rd, rs, imm)                                          \
  aa64_emit_inst(addsubi, 3, rd, rs, (imm) << 10)                             \

/* rd = ra + rn * rm */
#define aa64_emit_madd(rd, ra, rn, rm)                                        \
  aa64_emit_inst(mul4, 0, rd, rn, ((ra) << 10) | ((rm) << 16))                \

/* rd = ra - rn * rm */
#define aa64_emit_msub(rd, ra, rn, rm)                                        \
  aa64_emit_inst(mul4, 0, rd, rn, ((ra) << 10) | ((rm) << 16) | 0x8000)       \

#define aa64_emit_smaddl(rd, ra, rn, rm)                                      \
  aa64_emit_inst(mul4, 4, rd, rn, ((ra) << 10) | ((rm) << 16) | 0x200000)     \

#define aa64_emit_umaddl(rd, ra, rn, rm)                                      \
  aa64_emit_inst(mul4, 4, rd, rn, ((ra) << 10) | ((rm) << 16) | 0xA00000)     \

#define aa64_emit_mul(rd, rn, rm)                                             \
  aa64_emit_madd(rd, 31, rn, rm)                                              \

// MovZ, clears the highest bits and sets the lower ones
#define aa64_emit_movlo(rd, imm)                                              \
  aa64_emit_inst(movi, 2, rd, 0, (((imm) & 0xffff) << 5) | (4 << 21))         \

// MovZ, clears the lowest bits and sets the higher ones
#define aa64_emit_movhiz(rd, imm)                                             \
  aa64_emit_inst(movi, 2, rd, 0, (((imm) & 0xffff) << 5) | (5 << 21))         \

// MovK, keeps the other (lower) bits
#define aa64_emit_movhi(rd, imm)                                              \
  aa64_emit_inst(movi, 3, rd, 0, (((imm) & 0xffff) << 5) | (5 << 21))         \

// MovN, moves the inverted immediate (for negative numbers)
#define aa64_emit_movne(rd, imm)                                              \
  aa64_emit_inst(movi, 0, rd, 0, (((imm) & 0xffff) << 5) | (4 << 21))         \

#define aa64_emit_branch(offset)                                              \
  aa64_emit_inst(b, 0, 0, 0, (((u32)(offset))) & 0x3ffffff)                   \

#define aa64_emit_branch_patch(ptr, offset)                                   \
  *(ptr) = (((*(ptr)) & 0xfc000000) | (((u32)(offset)) & 0x3ffffff))          \

#define aa64_emit_brcond(cond, offset)                                        \
  aa64_emit_inst(b, 2, cond, 0, ((((u32)(offset))) & 0x7ffff) << 5)           \

#define aa64_emit_brcond_patch(ptr, offset)                                   \
  *(ptr) = (((*(ptr)) & 0xff00001f) | (((((u32)(offset))) & 0x7ffff) << 5))   \

#define aa64_emit_brlink(offset)                                              \
  aa64_emit_inst(b, 4, 0, 0, (((u32)(offset))) & 0x3ffffff)                   \

#define aa64_emit_extr(rd, rs, rm, amount)                                    \
  aa64_emit_inst(bfm, 0, rd, rs, (1 << 23) | ((amount) << 10) | ((rm) << 16)) \

#define aa64_emit_ror(rd, rs, amount)                                         \
  aa64_emit_extr(rd, rs, rs, amount)                                          \

#define aa64_emit_lsr(rd, rs, amount)                                         \
  aa64_emit_inst(bfm, 2, rd, rs, (31 << 10) | ((amount) << 16))               \

#define aa64_emit_lsl(rd, rs, amount)                                         \
  aa64_emit_inst(bfm, 2, rd, rs, ((31-(amount)) << 10) | (((32-(amount)) & 31) << 16))

#define aa64_emit_asr(rd, rs, amount)                                         \
  aa64_emit_inst(bfm, 0, rd, rs, (31 << 10) | ((amount) << 16))               \

#define aa64_emit_lsr64(rd, rs, amount)                                       \
  aa64_emit_inst(bfm, 6, rd, rs, (1 << 22) | (63 << 10) | ((amount) << 16))   \

#define aa64_emit_eori(rd, rs, immr, imms)                                    \
  aa64_emit_inst(movi, 2, rd, rs, ((imms) << 10) | ((immr) << 16))            \

#define aa64_emit_orri(rd, rs, immr, imms)                                    \
  aa64_emit_inst(movi, 1, rd, rs, ((imms) << 10) | ((immr) << 16))            \

#define aa64_emit_andi(rd, rs, immr, imms)                                    \
  aa64_emit_inst(movi, 0, rd, rs, ((imms) << 10) | ((immr) << 16))            \

#define aa64_emit_andi64(rd, rs, immr, imms)                                  \
  aa64_emit_inst(movi, 4, rd, rs, (1 << 22) | ((imms) << 10) | ((immr) << 16))

#define aa64_emit_mov(rd, rs)                                                 \
  aa64_emit_orr(rd, 31, rs)                                                   \

#define aa64_emit_orr(rd, rs, rm)                                             \
  aa64_emit_inst(logic, 1, rd, rs, ((rm) << 16))                              \

#define aa64_emit_orn(rd, rs, rm)                                             \
  aa64_emit_inst(logic, 1, rd, rs, ((rm) << 16)  | (1 << 21))                 \

#define aa64_emit_and(rd, rs, rm)                                             \
  aa64_emit_inst(logic, 0, rd, rs, ((rm) << 16))                              \

#define aa64_emit_ands(rd, rs, rm)                                            \
  aa64_emit_inst(logic, 3, rd, rs, ((rm) << 16))                              \

#define aa64_emit_tst(rs, rm)                                                 \
  aa64_emit_ands(31, rs, rm)                                                  \

#define aa64_emit_cmpi(rs, imm)                                               \
  aa64_emit_subis(31, rs, imm)                                                \

#define aa64_emit_xor(rd, rs, rm)                                             \
  aa64_emit_inst(logic, 2, rd, rs, ((rm) << 16))                              \

#define aa64_emit_bic(rd, rs, rm)                                             \
  aa64_emit_inst(logic, 0, rd, rs, ((rm) << 16) | (1 << 21))                  \

#define aa64_emit_add(rd, rs, rm)                                             \
  aa64_emit_inst(addsub, 0, rd, rs, ((rm) << 16))                             \

#define aa64_emit_sub(rd, rs, rm)                                             \
  aa64_emit_inst(addsub, 2, rd, rs, ((rm) << 16))                             \

#define aa64_emit_adc(rd, rs, rm)                                             \
  aa64_emit_inst(misc, 0, rd, rs, ((rm) << 16))                               \

#define aa64_emit_sbc(rd, rs, rm)                                             \
  aa64_emit_inst(misc, 2, rd, rs, ((rm) << 16))                               \

#define aa64_emit_adds(rd, rs, rm)                                            \
  aa64_emit_inst(addsub, 1, rd, rs, ((rm) << 16))                             \

#define aa64_emit_subs(rd, rs, rm)                                            \
  aa64_emit_inst(addsub, 3, rd, rs, ((rm) << 16))                             \

#define aa64_emit_adcs(rd, rs, rm)                                            \
  aa64_emit_inst(misc, 1, rd, rs, ((rm) << 16))                               \

#define aa64_emit_sbcs(rd, rs, rm)                                            \
  aa64_emit_inst(misc, 3, rd, rs, ((rm) << 16))                               \

#define aa64_emit_adr(rd, offset)                                             \
  aa64_emit_inst(adr, (offset) & 3, rd, 0, ((offset) >> 2) & 0x7ffff)         \

#define aa64_emit_tbz(rd, bitn, offset)                                       \
  aa64_emit_inst(tbz, 1, rd, 0, ((((u32)(offset)) & 0x3fff) << 5) | ((bitn) << 19))

#define aa64_emit_tbnz(rd, bitn, offset)                                      \
  aa64_emit_inst(tbnz, 1, rd, 0, ((((u32)(offset)) & 0x3fff) << 5) | ((bitn) << 19))

#define aa64_emit_cbz(rd, offset)                                             \
  aa64_emit_inst(b, 1, rd, 0, ((((u32)offset) & 0x7ffff)) << 5)               \

#define aa64_emit_cbnz(rd, offset)                                            \
  aa64_emit_inst(b2, 1, rd, 0, ((((u32)offset) & 0x7ffff)) << 5)              \

/* Misc Operations: Cond-select, Cond-Compare, ADC/SBC, CLZ/O, REV ... */
#define aa64_emit_csel(rd, rtrue, rfalse, cond)                               \
  aa64_emit_inst(misc, 0, rd, rtrue, (1<<23)|((rfalse) << 16)|((cond) << 12)) \

#define aa64_emit_csinc(rd, rs, rm, cond)                                     \
  aa64_emit_inst(misc, 0, rd, rs, 0x800400 | ((rm) << 16) | ((cond) << 12))   \

#define aa64_emit_csinv(rd, rs, rm, cond)                                     \
  aa64_emit_inst(misc, 2, rd, rs, 0x800000 | ((rm) << 16) | ((cond) << 12))   \

#define aa64_emit_csneg(rd, rs, rm, cond)                                     \
  aa64_emit_inst(misc, 2, rd, rs, 0x800400 | ((rm) << 16) | ((cond) << 12))   \

#define aa64_emit_ubfm(rd, rs, imms, immr)                                    \
  aa64_emit_inst(bfm, 2, rd, rs, ((imms) << 10) | ((immr) << 16))             \

#define aa64_emit_ubfx(rd, rs, pos, size)                                     \
  aa64_emit_ubfm(rd, rs, pos + size - 1, pos)                                 \

#define aa64_emit_cset(rd, cond)                                              \
  aa64_emit_csinc(rd, 31, 31, ((cond) ^ 1))                                   \

#define aa64_emit_csetm(rd, cond)                                             \
  aa64_emit_csinv(rd, 31, 31, ((cond) ^ 1))                                   \

#define aa64_emit_ccmpi(rn, immv, flags, cond)                                \
  aa64_emit_inst(misc, 3, rn, flags, 0x400800 | ((immv)<<16) | ((cond)<<12))  \

#define aa64_emit_rorv(rd, rs, ra)                                            \
  aa64_emit_inst(misc, 0, rd, rs, ((ra) << 16) | 0xC02C00)                    \

#define aa64_emit_lslv(rd, rs, ra)                                            \
  aa64_emit_inst(misc, 0, rd, rs, ((ra) << 16) | 0xC02000)                    \

#define aa64_emit_lsrv(rd, rs, ra)                                            \
  aa64_emit_inst(misc, 0, rd, rs, ((ra) << 16) | 0xC02400)                    \

#define aa64_emit_asrv(rd, rs, ra)                                            \
  aa64_emit_inst(misc, 0, rd, rs, ((ra) << 16) | 0xC02800)                    \

#define aa64_emit_orr_shift64(rd, rs, rm, st, sa)                             \
  aa64_emit_inst(logic, 5, rd, rs, ((rm) << 16) | ((st)<<22) | ((sa)<<10))    \

#define aa64_emit_merge_regs(rd, rhi, rlo)                                    \
  aa64_emit_orr_shift64(rd, rlo, rhi, 0, 32)                                  \

#define aa64_emit_sdiv(rd, rs, rm)                                            \
  aa64_emit_inst(misc, 0, rd, rs, ((rm) << 16) | 0xC00C00)                    \



