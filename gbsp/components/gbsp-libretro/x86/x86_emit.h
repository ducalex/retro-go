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

#ifndef X86_EMIT_H
#define X86_EMIT_H

u32 x86_update_gba(u32 pc);

// Although these are defined as a function, don't call them as
// such (jump to it instead)
void x86_indirect_branch_arm(u32 address);
void x86_indirect_branch_thumb(u32 address);
void x86_indirect_branch_dual(u32 address);

void function_cc execute_store_cpsr(u32 new_cpsr, u32 store_mask);

typedef enum
{
  x86_reg_number_eax,
  x86_reg_number_ecx,
  x86_reg_number_edx,
  x86_reg_number_ebx,
  x86_reg_number_esp,
  x86_reg_number_ebp,
  x86_reg_number_esi,
  x86_reg_number_edi
} x86_reg_number;

#define x86_emit_byte(value)                                                  \
  *translation_ptr = value;                                                   \
  translation_ptr++                                                           \

#define x86_emit_dword(value)                                                 \
  *((u32 *)translation_ptr) = value;                                          \
  translation_ptr += 4                                                        \

typedef enum
{
  x86_mod_mem        = 0,
  x86_mod_mem_disp8  = 1,
  x86_mod_mem_disp32 = 2,
  x86_mod_reg        = 3
} x86_mod;

#define x86_emit_mod_rm(mod, rm, spare)                                       \
  x86_emit_byte((mod << 6) | (spare << 3) | rm)                               \

#define x86_emit_sib(scale, ridx, rbase)                                      \
  x86_emit_byte(((scale) << 6) | ((ridx) << 3) | (rbase))                     \

#define x86_emit_mem_op(dest, base, offset)                                   \
  if(offset == 0)                                                             \
  {                                                                           \
    x86_emit_mod_rm(x86_mod_mem, base, dest);                                 \
  }                                                                           \
  else if(((s32)offset < 127) && ((s32)offset > -128))                        \
  {                                                                           \
    x86_emit_mod_rm(x86_mod_mem_disp8, base, dest);                           \
    x86_emit_byte((s8)offset);                                                \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    x86_emit_mod_rm(x86_mod_mem_disp32, base, dest);                          \
    x86_emit_dword(offset);                                                   \
  }                                                                           \

#define x86_emit_mem_sib_op(dest, base, ridx, scale, offset)                  \
  if(offset == 0)                                                             \
  {                                                                           \
    x86_emit_mod_rm(x86_mod_mem, 0x4, dest);                                  \
    x86_emit_sib(scale, ridx, base);                                          \
  }                                                                           \
  else if(((s32)offset < 127) && ((s32)offset > -128))                        \
  {                                                                           \
    x86_emit_mod_rm(x86_mod_mem_disp8, 0x4, dest);                            \
    x86_emit_sib(scale, ridx, base);                                          \
    x86_emit_byte((s8)offset);                                                \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    x86_emit_mod_rm(x86_mod_mem_disp32, 0x4, dest);                           \
    x86_emit_sib(scale, ridx, base);                                          \
    x86_emit_dword(offset);                                                   \
  }                                                                           \

#define x86_emit_reg_op(dest, source)                                         \
  x86_emit_mod_rm(x86_mod_reg, source, dest)                                  \


typedef enum
{
  x86_opcode_mov_rm_reg                 = 0x89,
  x86_opcode_mov_reg_rm                 = 0x8B,
  x86_opcode_mov_reg_imm                = 0xB8,
  x86_opcode_mov_rm_imm                 = 0x00C7,
  x86_opcode_ror_reg_imm                = 0x01C1,
  x86_opcode_shl_reg_imm                = 0x04C1,
  x86_opcode_shr_reg_imm                = 0x05C1,
  x86_opcode_sar_reg_imm                = 0x07C1,
  x86_opcode_ror_reg_rm                 = 0x01D3,
  x86_opcode_rcr_reg_rm                 = 0x03D3,
  x86_opcode_shl_reg_rm                 = 0x04D3,
  x86_opcode_shr_reg_rm                 = 0x05D3,
  x86_opcode_sar_reg_rm                 = 0x07D3,
  x86_opcode_rcr_reg1                   = 0x03D1,
  x86_opcode_call_offset                = 0xE8,
  x86_opcode_ret                        = 0xC3,
  x86_opcode_test_rm_imm                = 0x00F7,
  x86_opcode_test_reg_rm                = 0x85,
  x86_opcode_not_rm                     = 0x02F7,
  x86_opcode_mul_eax_rm                 = 0x04F7,
  x86_opcode_imul_eax_rm                = 0x05F7,
  x86_opcode_idiv_eax_rm                = 0x07F7,
  x86_opcode_add_rm_imm                 = 0x0081,
  x86_opcode_and_rm_imm                 = 0x0481,
  x86_opcode_sub_rm_imm                 = 0x0581,
  x86_opcode_xor_rm_imm                 = 0x0681,
  x86_opcode_add_reg_rm                 = 0x03,
  x86_opcode_adc_reg_rm                 = 0x13,
  x86_opcode_and_reg_rm                 = 0x23,
  x86_opcode_or_reg_rm                  = 0x0B,
  x86_opcode_sub_reg_rm                 = 0x2B,
  x86_opcode_sbb_reg_rm                 = 0x1B,
  x86_opcode_xor_reg_rm                 = 0x33,
  x86_opcode_cmp_reg_rm                 = 0x39,
  x86_opcode_cmp_rm_imm                 = 0x0781,
  x86_opcode_lea_reg_rm                 = 0x8D,
  x86_opcode_j                          = 0x80,
  x86_opcode_cdq                        = 0x99,
  x86_opcode_jmp                        = 0xE9,
  x86_opcode_jmp_reg                    = 0x04FF,
  x86_opcode_ext                        = 0x0F
} x86_opcodes;

typedef enum
{
  x86_opcode_seto                       = 0x90,
  x86_opcode_setc                       = 0x92,
  x86_opcode_setnc                      = 0x93,
  x86_opcode_setz                       = 0x94,
  x86_opcode_setnz                      = 0x95,
  x86_opcode_sets                       = 0x98,
  x86_opcode_setns                      = 0x99,
} x86_ext_opcodes;

typedef enum
{
  x86_condition_code_o                  = 0x00,
  x86_condition_code_no                 = 0x01,
  x86_condition_code_c                  = 0x02,
  x86_condition_code_nc                 = 0x03,
  x86_condition_code_z                  = 0x04,
  x86_condition_code_nz                 = 0x05,
  x86_condition_code_na                 = 0x06,
  x86_condition_code_a                  = 0x07,
  x86_condition_code_s                  = 0x08,
  x86_condition_code_ns                 = 0x09,
  x86_condition_code_p                  = 0x0A,
  x86_condition_code_np                 = 0x0B,
  x86_condition_code_l                  = 0x0C,
  x86_condition_code_nl                 = 0x0D,
  x86_condition_code_ng                 = 0x0E,
  x86_condition_code_g                  = 0x0F
} x86_condition_codes;

#define x86_relative_offset(source, offset, next)                             \
  ((u32)((uintptr_t)offset - ((uintptr_t)source + next)))                     \

#define x86_unequal_operands(op_a, op_b)                                      \
  (x86_reg_number_##op_a != x86_reg_number_##op_b)                            \

#define x86_emit_opcode_1b_reg(opcode, dest, source)                          \
{                                                                             \
  x86_emit_byte(x86_opcode_##opcode);                                         \
  x86_emit_reg_op(x86_reg_number_##dest, x86_reg_number_##source);            \
}                                                                             \

#define x86_emit_opcode_1b_mem(opcode, dest, base, offset)                    \
{                                                                             \
  x86_emit_byte(x86_opcode_##opcode);                                         \
  x86_emit_mem_op(x86_reg_number_##dest, x86_reg_number_##base, offset);      \
}                                                                             \

#define x86_emit_opcode_1b_mem_sib(opcode, dest, base, ridx, scale, offset)   \
{                                                                             \
  x86_emit_byte(x86_opcode_##opcode);                                         \
  x86_emit_mem_sib_op(x86_reg_number_##dest, x86_reg_number_##base,           \
                      x86_reg_number_##ridx, scale, offset);                  \
}                                                                             \

#define x86_emit_opcode_1b(opcode, reg)                                       \
  x86_emit_byte(x86_opcode_##opcode | x86_reg_number_##reg)                   \

#define x86_emit_opcode_1b_ext_reg(opcode, dest)                              \
  x86_emit_byte(x86_opcode_##opcode & 0xFF);                                  \
  x86_emit_reg_op(x86_opcode_##opcode >> 8, x86_reg_number_##dest)            \

#define x86_emit_opcode_1b_ext_mem(opcode, base, offset)                      \
  x86_emit_byte(x86_opcode_##opcode & 0xFF);                                  \
  x86_emit_mem_op(x86_opcode_##opcode >> 8, x86_reg_number_##base, offset)    \

#define x86_emit_mov_reg_mem(dest, base, offset)                              \
  x86_emit_opcode_1b_mem(mov_reg_rm, dest, base, offset)                      \

#define x86_emit_mov_reg_mem_idx(dest, base, scale, index, offset)            \
  x86_emit_opcode_1b_mem_sib(mov_reg_rm, dest, base, index, scale, offset)    \

#define x86_emit_mov_mem_idx_reg(dest, base, scale, index, offset)            \
  x86_emit_opcode_1b_mem_sib(mov_rm_reg, dest, base, index, scale, offset)    \

#define x86_emit_mov_mem_reg(source, base, offset)                            \
  x86_emit_opcode_1b_mem(mov_rm_reg, source, base, offset)                    \

#define x86_emit_setcc_mem(ccode, base, offset)                               \
  x86_emit_byte(x86_opcode_ext);                                              \
  x86_emit_opcode_1b_mem(set##ccode, eax, base, offset);                      \

#define x86_emit_add_reg_mem(dst, base, offset)                               \
  x86_emit_opcode_1b_mem(add_reg_rm, dst, base, offset);                      \

#define x86_emit_or_reg_mem(dst, base, offset)                                \
  x86_emit_opcode_1b_mem(or_reg_rm, dst, base, offset);                       \

#define x86_emit_xor_reg_mem(dst, base, offset)                               \
  x86_emit_opcode_1b_mem(xor_reg_rm, dst, base, offset);                      \

#define x86_emit_cmp_reg_mem(rega, base, offset)                              \
  x86_emit_opcode_1b_mem(cmp_reg_rm, rega, base, offset);                     \

#define x86_emit_test_reg_mem(rega, base, offset)                             \
  x86_emit_opcode_1b_mem(test_reg_rm, rega, base, offset);                    \

#define x86_emit_mov_reg_reg(dest, source)                                    \
  if(x86_unequal_operands(dest, source))                                      \
  {                                                                           \
    x86_emit_opcode_1b_reg(mov_reg_rm, dest, source)                          \
  }                                                                           \

#define x86_emit_mov_reg_imm(dest, imm)                                       \
  x86_emit_opcode_1b(mov_reg_imm, dest);                                      \
  x86_emit_dword(imm)                                                         \

#define x86_emit_mov_mem_imm(imm, base, offset)                               \
  x86_emit_opcode_1b_ext_mem(mov_rm_imm, base, offset);                       \
  x86_emit_dword(imm)                                                         \

#define x86_emit_and_mem_imm(imm, base, offset)                               \
  x86_emit_opcode_1b_ext_mem(and_rm_imm, base, offset);                       \
  x86_emit_dword(imm)                                                         \

#define x86_emit_shl_reg_imm(dest, imm)                                       \
  x86_emit_opcode_1b_ext_reg(shl_reg_imm, dest);                              \
  x86_emit_byte(imm)                                                          \

#define x86_emit_shr_reg_imm(dest, imm)                                       \
  x86_emit_opcode_1b_ext_reg(shr_reg_imm, dest);                              \
  x86_emit_byte(imm)                                                          \

#define x86_emit_sar_reg_imm(dest, imm)                                       \
  x86_emit_opcode_1b_ext_reg(sar_reg_imm, dest);                              \
  x86_emit_byte(imm)                                                          \

#define x86_emit_ror_reg_imm(dest, imm)                                       \
  x86_emit_opcode_1b_ext_reg(ror_reg_imm, dest);                              \
  x86_emit_byte(imm)                                                          \

#define x86_emit_add_reg_reg(dest, source)                                    \
  x86_emit_opcode_1b_reg(add_reg_rm, dest, source)                            \

#define x86_emit_adc_reg_reg(dest, source)                                    \
  x86_emit_opcode_1b_reg(adc_reg_rm, dest, source)                            \

#define x86_emit_sub_reg_reg(dest, source)                                    \
  x86_emit_opcode_1b_reg(sub_reg_rm, dest, source)                            \

#define x86_emit_sbb_reg_reg(dest, source)                                    \
  x86_emit_opcode_1b_reg(sbb_reg_rm, dest, source)                            \

#define x86_emit_and_reg_reg(dest, source)                                    \
  x86_emit_opcode_1b_reg(and_reg_rm, dest, source)                            \

#define x86_emit_or_reg_reg(dest, source)                                     \
  x86_emit_opcode_1b_reg(or_reg_rm, dest, source)                             \

#define x86_emit_xor_reg_reg(dest, source)                                    \
  x86_emit_opcode_1b_reg(xor_reg_rm, dest, source)                            \

#define x86_emit_add_reg_imm(dest, imm)                                       \
  if(imm != 0)                                                                \
  {                                                                           \
    x86_emit_opcode_1b_ext_reg(add_rm_imm, dest);                             \
    x86_emit_dword(imm);                                                      \
  }                                                                           \

#define x86_emit_sub_reg_imm(dest, imm)                                       \
  if(imm != 0)                                                                \
  {                                                                           \
    x86_emit_opcode_1b_ext_reg(sub_rm_imm, dest);                             \
    x86_emit_dword(imm);                                                      \
  }                                                                           \

#define x86_emit_and_reg_imm(dest, imm)                                       \
  x86_emit_opcode_1b_ext_reg(and_rm_imm, dest);                               \
  x86_emit_dword(imm)                                                         \

#define x86_emit_xor_reg_imm(dest, imm)                                       \
  x86_emit_opcode_1b_ext_reg(xor_rm_imm, dest);                               \
  x86_emit_dword(imm)                                                         \

#define x86_emit_test_reg_imm(dest, imm)                                      \
  x86_emit_opcode_1b_ext_reg(test_rm_imm, dest);                              \
  x86_emit_dword(imm)                                                         \

#define x86_emit_cmp_reg_reg(dest, source)                                    \
  x86_emit_opcode_1b_reg(cmp_reg_rm, dest, source)                            \

#define x86_emit_test_reg_reg(dest, source)                                   \
  x86_emit_opcode_1b_reg(test_reg_rm, dest, source)                           \

#define x86_emit_cmp_reg_imm(dest, imm)                                       \
  x86_emit_opcode_1b_ext_reg(cmp_rm_imm, dest);                               \
  x86_emit_dword(imm)                                                         \

#define x86_emit_rot_reg_reg(type, dest)                                      \
  x86_emit_opcode_1b_ext_reg(type##_reg_rm, dest)                             \

#define x86_emit_rot_reg1(type, dest)                                         \
  x86_emit_opcode_1b_ext_reg(type##_reg1, dest)                               \

#define x86_emit_shr_reg_reg(dest)                                            \
  x86_emit_opcode_1b_ext_reg(shr_reg_rm, dest)                                \

#define x86_emit_sar_reg_reg(dest)                                            \
  x86_emit_opcode_1b_ext_reg(sar_reg_rm, dest)                                \

#define x86_emit_shl_reg_reg(dest)                                            \
  x86_emit_opcode_1b_ext_reg(shl_reg_rm, dest)                                \

#define x86_emit_mul_eax_reg(source)                                          \
  x86_emit_opcode_1b_ext_reg(mul_eax_rm, source)                              \

#define x86_emit_imul_eax_reg(source)                                         \
  x86_emit_opcode_1b_ext_reg(imul_eax_rm, source)                             \

#define x86_emit_idiv_eax_reg(source)                                         \
  x86_emit_opcode_1b_ext_reg(idiv_eax_rm, source)                             \

#define x86_emit_not_reg(srcdst)                                              \
  x86_emit_opcode_1b_ext_reg(not_rm, srcdst)                                  \

#define x86_emit_cdq()                                                        \
  x86_emit_byte(x86_opcode_cdq)                                               \

#define x86_emit_call_offset(relative_offset)                                 \
  x86_emit_byte(x86_opcode_call_offset);                                      \
  x86_emit_dword(relative_offset)                                             \

#define x86_emit_ret()                                                        \
  x86_emit_byte(x86_opcode_ret)                                               \

#define x86_emit_lea_reg_mem(dest, base, offset)                              \
  x86_emit_opcode_1b_mem(lea_reg_rm, dest, base, offset)                      \

#define x86_emit_j_filler(condition_code, writeback_location)                 \
  x86_emit_byte(x86_opcode_ext);                                              \
  x86_emit_byte(x86_opcode_j | condition_code);                               \
  (writeback_location) = translation_ptr;                                     \
  translation_ptr += 4                                                        \

#define x86_emit_j_offset(condition_code, offset)                             \
  x86_emit_byte(x86_opcode_ext);                                              \
  x86_emit_byte(x86_opcode_j | condition_code);                               \
  x86_emit_dword(offset)                                                      \

#define x86_emit_jmp_filler(writeback_location)                               \
  x86_emit_byte(x86_opcode_jmp);                                              \
  (writeback_location) = translation_ptr;                                     \
  translation_ptr += 4                                                        \

#define x86_emit_jmp_offset(offset)                                           \
  x86_emit_byte(x86_opcode_jmp);                                              \
  x86_emit_dword(offset)                                                      \

#define x86_emit_jmp_reg(source)                                              \
  x86_emit_opcode_1b_ext_reg(jmp_reg, source)                                 \

#define reg_base    ebx        // Saved register
#define reg_cycles  ebp        // Saved register
#define reg_a0      eax
#define reg_a1      edx
#define reg_a2      ecx
#define reg_t0      esi
#define reg_rv      eax

#if defined(_WIN64)
  #define reg_arg0  ecx
  #define reg_arg1  edx
#elif defined(__x86_64__) || defined(__amd64__)
  #define reg_arg0  edi
  #define reg_arg1  esi
#else
  #define reg_arg0  eax
  #define reg_arg1  edx
#endif

/* Offsets from reg_base, see stub.S */
#define SPSR_BASE_OFF   0xA9100

#define generate_test_imm(ireg, imm)                                          \
  x86_emit_test_reg_imm(reg_##ireg, imm);                                     \

#define generate_test_memreg(ireg_ref, arm_reg_src)                           \
  x86_emit_test_reg_mem(reg_##ireg_ref, reg_base, arm_reg_src * 4)            \

#define generate_cmp_memreg(ireg_ref, arm_reg_src)                            \
  x86_emit_cmp_reg_mem(reg_##ireg_ref, reg_base, arm_reg_src * 4)             \

#define generate_cmp_imm(ireg, imm)                                           \
  x86_emit_cmp_reg_imm(reg_##ireg, imm)                                       \

#define generate_cmp_reg(ireg, ireg2)                                         \
  x86_emit_cmp_reg_reg(reg_##ireg, reg_##ireg2)                               \

#define generate_update_flag(condcode, regnum)                                \
  x86_emit_setcc_mem(condcode, reg_base, regnum * 4)                          \

#define generate_load_spsr(ireg, idxr)                                        \
  x86_emit_mov_reg_mem_idx(reg_##ireg, reg_base, 2, reg_##idxr, SPSR_BASE_OFF);

#define generate_store_spsr(ireg, idxr)                                       \
  x86_emit_mov_mem_idx_reg(reg_##ireg, reg_base, 2, reg_##idxr, SPSR_BASE_OFF);

#define generate_load_reg(ireg, reg_index)                                    \
  x86_emit_mov_reg_mem(reg_##ireg, reg_base, reg_index * 4);                  \

#define generate_load_pc(ireg, new_pc)                                        \
  x86_emit_mov_reg_imm(reg_##ireg, (new_pc))                                  \

#define generate_load_imm(ireg, imm)                                          \
  x86_emit_mov_reg_imm(reg_##ireg, imm)                                       \

#define generate_store_reg(ireg, reg_index)                                   \
  x86_emit_mov_mem_reg(reg_##ireg, reg_base, (reg_index) * 4)                 \

#define generate_store_reg_i32(imm32, reg_index)                              \
  x86_emit_mov_mem_imm((imm32), reg_base, (reg_index) * 4)                    \

#define generate_shift_left(ireg, imm)                                        \
  x86_emit_shl_reg_imm(reg_##ireg, imm)                                       \

#define generate_shift_left_var(ireg)                                         \
  x86_emit_shl_reg_reg(reg_##ireg)                                            \

#define generate_shift_right(ireg, imm)                                       \
  x86_emit_shr_reg_imm(reg_##ireg, imm)                                       \

#define generate_shift_right_var(ireg)                                        \
  x86_emit_shr_reg_reg(reg_##ireg)                                            \

#define generate_shift_right_arithmetic(ireg, imm)                            \
  x86_emit_sar_reg_imm(reg_##ireg, imm)                                       \

#define generate_shift_right_arithmetic_var(ireg)                             \
  x86_emit_sar_reg_reg(reg_##ireg)                                            \

#define generate_rotate_right(ireg, imm)                                      \
  x86_emit_ror_reg_imm(reg_##ireg, imm)                                       \

#define generate_rotate_right_var(ireg)                                       \
  x86_emit_rot_reg_reg(ror, reg_##ireg)                                       \

#define generate_rcr(ireg)                                                    \
  x86_emit_rot_reg_reg(rcr, reg_##ireg)                                       \

#define generate_rcr1(ireg)                                                   \
  x86_emit_rot_reg1(rcr, reg_##ireg)                                          \

#define generate_and_mem(imm, ireg_base, offset)                              \
  x86_emit_and_mem_imm(imm, reg_##ireg_base, (offset))                        \

#define generate_and(ireg_dest, ireg_src)                                     \
  x86_emit_and_reg_reg(reg_##ireg_dest, reg_##ireg_src)                       \

#define generate_add(ireg_dest, ireg_src)                                     \
  x86_emit_add_reg_reg(reg_##ireg_dest, reg_##ireg_src)                       \

#define generate_adc(ireg_dest, ireg_src)                                     \
  x86_emit_adc_reg_reg(reg_##ireg_dest, reg_##ireg_src)                       \

#define generate_add_memreg(ireg_dest, arm_reg_src)                           \
  x86_emit_add_reg_mem(reg_##ireg_dest, reg_base, arm_reg_src * 4)            \

#define generate_sub(ireg_dest, ireg_src)                                     \
  x86_emit_sub_reg_reg(reg_##ireg_dest, reg_##ireg_src)                       \

#define generate_sbb(ireg_dest, ireg_src)                                     \
  x86_emit_sbb_reg_reg(reg_##ireg_dest, reg_##ireg_src)                       \

#define generate_or(ireg_dest, ireg_src)                                      \
  x86_emit_or_reg_reg(reg_##ireg_dest, reg_##ireg_src)                        \

#define generate_or_mem(ireg_dest, arm_reg_src)                               \
  x86_emit_or_reg_mem(reg_##ireg_dest, reg_base, arm_reg_src * 4)             \

#define generate_xor(ireg_dest, ireg_src)                                     \
  x86_emit_xor_reg_reg(reg_##ireg_dest, reg_##ireg_src)                       \

#define generate_xor_mem(ireg_dest, arm_reg_src)                              \
  x86_emit_xor_reg_mem(reg_##ireg_dest, reg_base, arm_reg_src * 4)            \

#define generate_add_imm(ireg, imm)                                           \
  x86_emit_add_reg_imm(reg_##ireg, imm)                                       \

#define generate_sub_imm(ireg, imm)                                           \
  x86_emit_sub_reg_imm(reg_##ireg, imm)                                       \

#define generate_xor_imm(ireg, imm)                                           \
  x86_emit_xor_reg_imm(reg_##ireg, imm)                                       \

#define generate_add_reg_reg_imm(ireg_dest, ireg_src, imm)                    \
  x86_emit_lea_reg_mem(reg_##ireg_dest, reg_##ireg_src, imm)                  \

#define generate_and_imm(ireg, imm)                                           \
  x86_emit_and_reg_imm(reg_##ireg, imm)                                       \

#define generate_mov(ireg_dest, ireg_src)                                     \
  x86_emit_mov_reg_reg(reg_##ireg_dest, reg_##ireg_src)                       \

#define generate_not(ireg)                                                    \
  x86_emit_not_reg(reg_##ireg)                                                \

#define generate_multiply(ireg)                                               \
  x86_emit_imul_eax_reg(reg_##ireg)                                           \

#define generate_multiply_s64(ireg)                                           \
  x86_emit_imul_eax_reg(reg_##ireg)                                           \

#define generate_multiply_u64(ireg)                                           \
  x86_emit_mul_eax_reg(reg_##ireg)                                            \

#define generate_multiply_s64_add(ireg_src, ireg_lo, ireg_hi)                 \
  x86_emit_imul_eax_reg(reg_##ireg_src);                                      \
  x86_emit_add_reg_reg(reg_a0, reg_##ireg_lo);                                \
  x86_emit_adc_reg_reg(reg_a1, reg_##ireg_hi)                                 \

#define generate_multiply_u64_add(ireg_src, ireg_lo, ireg_hi)                 \
  x86_emit_mul_eax_reg(reg_##ireg_src);                                       \
  x86_emit_add_reg_reg(reg_a0, reg_##ireg_lo);                                \
  x86_emit_adc_reg_reg(reg_a1, reg_##ireg_hi)                                 \


#define generate_function_call(function_location)                             \
  x86_emit_call_offset(x86_relative_offset(translation_ptr,                   \
   function_location, 4));                                                    \

#define generate_exit_block()                                                 \
  x86_emit_ret();                                                             \

#define generate_cycle_update()                                               \
  x86_emit_sub_reg_imm(reg_cycles, cycle_count);                              \
  cycle_count = 0                                                             \

#define generate_branch_patch_conditional(dest, offset)                       \
  *((u32 *)(dest)) = x86_relative_offset(dest, offset, 4)                     \

#define generate_branch_patch_unconditional(dest, offset)                     \
  *((u32 *)(dest)) = x86_relative_offset(dest, offset, 4)                     \

#define generate_branch_no_cycle_update(writeback_location, new_pc)           \
  if(pc == idle_loop_target_pc)                                               \
  {                                                                           \
    generate_load_imm(cycles, 0);                                             \
    x86_emit_mov_reg_imm(eax, new_pc);                                        \
    generate_function_call(x86_update_gba);                                   \
    x86_emit_jmp_filler(writeback_location);                                  \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    x86_emit_test_reg_reg(reg_cycles, reg_cycles);                            \
    x86_emit_j_offset(x86_condition_code_ns, 10);                             \
    x86_emit_mov_reg_imm(eax, new_pc);                                        \
    generate_function_call(x86_update_gba);                                   \
    x86_emit_jmp_filler(writeback_location);                                  \
  }                                                                           \

#define generate_branch_cycle_update(writeback_location, new_pc)              \
  generate_cycle_update();                                                    \
  generate_branch_no_cycle_update(writeback_location, new_pc)                 \

// a0 holds the destination

#define generate_indirect_branch_cycle_update(type)                           \
  generate_cycle_update();                                                    \
  x86_emit_call_offset(x86_relative_offset(translation_ptr,                   \
   x86_indirect_branch_##type, 4))                                            \

#define generate_indirect_branch_no_cycle_update(type)                        \
  x86_emit_call_offset(x86_relative_offset(translation_ptr,                   \
   x86_indirect_branch_##type, 4))                                            \

#define block_prologue_size 0
#define generate_block_prologue()
#define generate_block_extra_vars_arm()
#define generate_block_extra_vars_thumb()

#define generate_indirect_branch_arm()                                        \
  {                                                                           \
    if(condition == 0x0E)                                                     \
    {                                                                         \
      generate_indirect_branch_cycle_update(arm);                             \
    }                                                                         \
    else                                                                      \
    {                                                                         \
      generate_indirect_branch_no_cycle_update(arm);                          \
    }                                                                         \
  }                                                                           \

#define generate_indirect_branch_dual()                                       \
  {                                                                           \
    if(condition == 0x0E)                                                     \
    {                                                                         \
      generate_indirect_branch_cycle_update(dual);                            \
    }                                                                         \
    else                                                                      \
    {                                                                         \
      generate_indirect_branch_no_cycle_update(dual);                         \
    }                                                                         \
  }                                                                           \


#define get_shift_imm()                                                       \
  u32 shift = (opcode >> 7) & 0x1F                                            \

#define generate_shift_reg(ireg, name, flags_op)                              \
  generate_load_reg_pc(ireg, rm, 12);                                         \
  generate_load_reg(a1, ((opcode >> 8) & 0x0F));                              \
  generate_and_imm(a1, 0xFF);                                                 \
  generate_##name##_##flags_op##_reg(ireg);                                   \

#ifdef TRACE_INSTRUCTIONS
  void function_cc trace_instruction(u32 pc, u32 mode)
  {
    if (mode)
      printf("Executed arm %x\n", pc);
    else
      printf("Executed thumb %x\n", pc);
    #ifdef TRACE_REGISTERS
    print_regs();
    #endif
  }

  #define emit_trace_instruction(pc, mode)         \
    x86_emit_mov_reg_imm(reg_arg0, pc);            \
    x86_emit_mov_reg_imm(reg_arg1, mode);          \
    generate_function_call(trace_instruction);
  #define emit_trace_arm_instruction(pc)           \
    emit_trace_instruction(pc, 1)
  #define emit_trace_thumb_instruction(pc)         \
    emit_trace_instruction(pc, 0)
#else
  #define emit_trace_thumb_instruction(pc)
  #define emit_trace_arm_instruction(pc)
#endif

#define check_generate_n_flag   (flag_status & 0x08)
#define check_generate_z_flag   (flag_status & 0x04)
#define check_generate_c_flag   (flag_status & 0x02)
#define check_generate_v_flag   (flag_status & 0x01)

#define generate_asr_no_flags_reg(ireg)                                       \
{                                                                             \
  u8 *jmpinst;                                                                \
  generate_mov(a2, a1);                                                       \
  generate_shift_right_arithmetic_var(a0);                                    \
  generate_cmp_imm(a2, 32);                                                   \
  x86_emit_j_filler(x86_condition_code_l, jmpinst);                           \
  generate_shift_right_arithmetic(a0, 31);                                    \
  generate_branch_patch_conditional(jmpinst, translation_ptr);                \
  generate_mov(ireg, a0);                                                     \
}

#define generate_asr_flags_reg(ireg)                                          \
{                                                                             \
  u8 *jmpinst1, *jmpinst2;                                                    \
  generate_mov(a2, a1);                                                       \
  generate_or(a2, a2);                                                        \
  x86_emit_j_filler(x86_condition_code_z, jmpinst1);                          \
  generate_shift_right_arithmetic_var(a0);                                    \
  generate_update_flag(c, REG_C_FLAG)                                         \
  generate_cmp_imm(a2, 32);                                                   \
  x86_emit_j_filler(x86_condition_code_l, jmpinst2);                          \
  generate_shift_right_arithmetic(a0, 16);                                    \
  generate_shift_right_arithmetic(a0, 16);                                    \
  generate_update_flag(c, REG_C_FLAG)                                         \
  generate_branch_patch_conditional(jmpinst1, translation_ptr);               \
  generate_branch_patch_conditional(jmpinst2, translation_ptr);               \
  generate_mov(ireg, a0);                                                     \
}

#define generate_lsl_no_flags_reg(ireg)                                       \
{                                                                             \
  u8 *jmpinst;                                                                \
  generate_mov(a2, a1);                                                       \
  generate_shift_left_var(a0);                                                \
  generate_cmp_imm(a2, 32);                                                   \
  x86_emit_j_filler(x86_condition_code_l, jmpinst);                           \
  generate_load_imm(a0, 0);                                                   \
  generate_branch_patch_conditional(jmpinst, translation_ptr);                \
  generate_mov(ireg, a0);                                                     \
}

#define generate_lsl_flags_reg(ireg)                                          \
{                                                                             \
  u8 *jmpinst1, *jmpinst2;                                                    \
  generate_mov(a2, a1);                                                       \
  generate_or(a2, a2);                                                        \
  x86_emit_j_filler(x86_condition_code_z, jmpinst1);                          \
  generate_sub_imm(a2, 1);                                                    \
  generate_shift_left_var(a0);                                                \
  generate_or(a0, a0);                                                        \
  generate_update_flag(s, REG_C_FLAG)                                         \
  generate_shift_left(a0, 1);                                                 \
  generate_cmp_imm(a2, 32);                                                   \
  x86_emit_j_filler(x86_condition_code_l, jmpinst2);                          \
  generate_load_imm(a0, 0);                                                   \
  generate_store_reg(a0, REG_C_FLAG)                                          \
  generate_branch_patch_conditional(jmpinst1, translation_ptr);               \
  generate_branch_patch_conditional(jmpinst2, translation_ptr);               \
  generate_mov(ireg, a0);                                                     \
}

#define generate_lsr_no_flags_reg(ireg)                                       \
{                                                                             \
  u8 *jmpinst;                                                                \
  generate_mov(a2, a1);                                                       \
  generate_shift_right_var(a0);                                               \
  generate_cmp_imm(a2, 32);                                                   \
  x86_emit_j_filler(x86_condition_code_l, jmpinst);                           \
  generate_xor(a0, a0);                                                       \
  generate_branch_patch_conditional(jmpinst, translation_ptr);                \
  generate_mov(ireg, a0);                                                     \
}

#define generate_lsr_flags_reg(ireg)                                          \
{                                                                             \
  u8 *jmpinst1, *jmpinst2;                                                    \
  generate_mov(a2, a1);                                                       \
  generate_or(a2, a2);                                                        \
  x86_emit_j_filler(x86_condition_code_z, jmpinst1);                          \
  generate_sub_imm(a2, 1);                                                    \
  generate_shift_right_var(a0);                                               \
  generate_test_imm(a0, 1);                                                   \
  generate_update_flag(nz, REG_C_FLAG)                                        \
  generate_shift_right(a0, 1);                                                \
  generate_cmp_imm(a2, 32);                                                   \
  x86_emit_j_filler(x86_condition_code_l, jmpinst2);                          \
  generate_xor(a0, a0);                                                       \
  generate_store_reg(a0, REG_C_FLAG)                                          \
  generate_branch_patch_conditional(jmpinst1, translation_ptr);               \
  generate_branch_patch_conditional(jmpinst2, translation_ptr);               \
  generate_mov(ireg, a0);                                                     \
}

#define generate_ror_no_flags_reg(ireg)                                       \
  generate_mov(a2, a1);                                                       \
  generate_rotate_right_var(a0);                                              \
  generate_mov(ireg, a0);

#define generate_ror_flags_reg(ireg)                                          \
{                                                                             \
  u8 *jmpinst;                                                                \
  generate_mov(a2, a1);                                                       \
  generate_or(a2, a2);                                                        \
  x86_emit_j_filler(x86_condition_code_z, jmpinst);                           \
  generate_sub_imm(a2, 1);                                                    \
  generate_rotate_right_var(a0);                                              \
  generate_test_imm(a0, 1);                                                   \
  generate_update_flag(nz, REG_C_FLAG)                                        \
  generate_rotate_right(a0, 1);                                               \
  generate_branch_patch_conditional(jmpinst, translation_ptr);                \
  generate_mov(ireg, a0);                                                     \
}

// Shift right sets the CF of the shifted-out bit, use it with setc
#define generate_rrx_flags(ireg)                                              \
  generate_load_imm(a2, 0xffffffff);                                          \
  generate_add_memreg(a2, REG_C_FLAG);                                        \
  generate_rcr1(a0);                                                          \
  generate_update_flag(c, REG_C_FLAG)                                         \
  generate_mov(ireg, a0);

#define generate_rrx(ireg)                                                    \
  generate_load_reg(a2, REG_C_FLAG);                                          \
  generate_shift_right(ireg, 1);                                              \
  generate_shift_left(a2, 31);                                                \
  generate_or(ireg, a2);                                                      \

#define generate_shift_imm_lsl_no_flags(ireg)                                 \
  generate_load_reg_pc(ireg, rm, 8);                                          \
  if(shift != 0)                                                              \
  {                                                                           \
    generate_shift_left(ireg, shift);                                         \
  }                                                                           \

#define generate_shift_imm_lsr_no_flags(ireg)                                 \
  if(shift != 0)                                                              \
  {                                                                           \
    generate_load_reg_pc(ireg, rm, 8);                                        \
    generate_shift_right(ireg, shift);                                        \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_imm(ireg, 0);                                               \
  }                                                                           \

#define generate_shift_imm_asr_no_flags(ireg)                                 \
  generate_load_reg_pc(ireg, rm, 8);                                          \
  if(shift != 0)                                                              \
  {                                                                           \
    generate_shift_right_arithmetic(ireg, shift);                             \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_shift_right_arithmetic(ireg, 31);                                \
  }                                                                           \

#define generate_shift_imm_ror_no_flags(ireg)                                 \
  if(shift != 0)                                                              \
  {                                                                           \
    generate_load_reg_pc(ireg, rm, 8);                                        \
    generate_rotate_right(ireg, shift);                                       \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_reg_pc(ireg, rm, 8);                                        \
    generate_rrx(ireg);                                                       \
  }                                                                           \

#define generate_shift_imm_lsl_flags(ireg)                                    \
  generate_load_reg_pc(ireg, rm, 8);                                          \
  if(shift != 0)                                                              \
  {                                                                           \
    generate_shift_left(ireg, shift);                                         \
    generate_update_flag(c, REG_C_FLAG);                                      \
  }                                                                           \

#define generate_shift_imm_lsr_flags(ireg)                                    \
  generate_load_reg_pc(ireg, rm, 8);                                          \
  if(shift != 0)                                                              \
  {                                                                           \
    generate_shift_right(ireg, shift);                                        \
    generate_update_flag(c, REG_C_FLAG);                                      \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_shift_right(ireg, 31);                                           \
    generate_store_reg(ireg, REG_C_FLAG);                                     \
    generate_load_imm(ireg, 0);                                               \
  }                                                                           \

#define generate_shift_imm_asr_flags(ireg)                                    \
  generate_load_reg_pc(ireg, rm, 8);                                          \
  if(shift != 0)                                                              \
  {                                                                           \
    generate_shift_right_arithmetic(ireg, shift);                             \
    generate_update_flag(c, REG_C_FLAG);                                      \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_shift_right_arithmetic(ireg, 31);                                \
    generate_update_flag(nz, REG_C_FLAG);                                     \
  }                                                                           \

#define generate_shift_imm_ror_flags(ireg)                                    \
  generate_load_reg_pc(ireg, rm, 8);                                          \
  if(shift != 0)                                                              \
  {                                                                           \
    generate_rotate_right(ireg, shift);                                       \
    generate_update_flag(c, REG_C_FLAG)                                       \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_rrx_flags(ireg);                                                 \
  }                                                                           \

#define generate_shift_imm(ireg, name, flags_op)                              \
  get_shift_imm();                                                            \
  generate_shift_imm_##name##_##flags_op(ireg)                                \

#define generate_load_rm_sh(flags_op)                                         \
  switch((opcode >> 4) & 0x07)                                                \
  {                                                                           \
    /* LSL imm */                                                             \
    case 0x0:                                                                 \
    {                                                                         \
      generate_shift_imm(a0, lsl, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* LSL reg */                                                             \
    case 0x1:                                                                 \
    {                                                                         \
      generate_shift_reg(a0, lsl, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* LSR imm */                                                             \
    case 0x2:                                                                 \
    {                                                                         \
      generate_shift_imm(a0, lsr, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* LSR reg */                                                             \
    case 0x3:                                                                 \
    {                                                                         \
      generate_shift_reg(a0, lsr, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ASR imm */                                                             \
    case 0x4:                                                                 \
    {                                                                         \
      generate_shift_imm(a0, asr, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ASR reg */                                                             \
    case 0x5:                                                                 \
    {                                                                         \
      generate_shift_reg(a0, asr, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ROR imm */                                                             \
    case 0x6:                                                                 \
    {                                                                         \
      generate_shift_imm(a0, ror, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ROR reg */                                                             \
    case 0x7:                                                                 \
    {                                                                         \
      generate_shift_reg(a0, ror, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
  }                                                                           \

#define generate_load_offset_sh()                                             \
  switch((opcode >> 5) & 0x03)                                                \
  {                                                                           \
    /* LSL imm */                                                             \
    case 0x0:                                                                 \
    {                                                                         \
      generate_shift_imm(a1, lsl, no_flags);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* LSR imm */                                                             \
    case 0x1:                                                                 \
    {                                                                         \
      generate_shift_imm(a1, lsr, no_flags);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ASR imm */                                                             \
    case 0x2:                                                                 \
    {                                                                         \
      generate_shift_imm(a1, asr, no_flags);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ROR imm */                                                             \
    case 0x3:                                                                 \
    {                                                                         \
      generate_shift_imm(a1, ror, no_flags);                                  \
      break;                                                                  \
    }                                                                         \
  }                                                                           \

#define extract_flags()                                                       \
  reg[REG_N_FLAG] = reg[REG_CPSR] >> 31;                                      \
  reg[REG_Z_FLAG] = (reg[REG_CPSR] >> 30) & 0x01;                             \
  reg[REG_C_FLAG] = (reg[REG_CPSR] >> 29) & 0x01;                             \
  reg[REG_V_FLAG] = (reg[REG_CPSR] >> 28) & 0x01;                             \

#define collapse_flags(ireg, tmpreg)                                          \
  generate_load_reg(ireg, REG_V_FLAG);                                        \
  generate_load_reg(tmpreg, REG_C_FLAG);                                      \
  generate_shift_left(tmpreg, 1);                                             \
  generate_or(ireg, tmpreg);                                                  \
  generate_load_reg(tmpreg, REG_Z_FLAG);                                      \
  generate_shift_left(tmpreg, 2);                                             \
  generate_or(ireg, tmpreg);                                                  \
  generate_load_reg(tmpreg, REG_N_FLAG);                                      \
  generate_shift_left(tmpreg, 3);                                             \
  generate_or(ireg, tmpreg);                                                  \
  generate_load_reg(tmpreg, REG_CPSR);                                        \
  generate_shift_left(ireg, 28);                                              \
  generate_and_imm(tmpreg, 0xFF);                                             \
  generate_or(ireg, tmpreg);                                                  \
  generate_store_reg(ireg, REG_CPSR);                                         \

// It should be okay to still generate result flags, spsr will overwrite them.
// This is pretty infrequent (returning from interrupt handlers, et al) so
// probably not worth optimizing for.

#define generate_load_reg_pc(ireg, reg_index, pc_offset)                      \
  if(reg_index == 15)                                                         \
  {                                                                           \
    generate_load_pc(ireg, pc + pc_offset);                                   \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_reg(ireg, reg_index);                                       \
  }                                                                           \

#define generate_store_reg_pc_no_flags(ireg, reg_index)                       \
  generate_store_reg(ireg, reg_index);                                        \
  if(reg_index == 15)                                                         \
  {                                                                           \
    generate_mov(a0, ireg);                                                   \
    generate_indirect_branch_arm();                                           \
  }                                                                           \

u32 function_cc execute_spsr_restore(u32 address)
{
  if(reg[CPU_MODE] != MODE_USER && reg[CPU_MODE] != MODE_SYSTEM)
  {
    reg[REG_CPSR] = REG_SPSR(reg[CPU_MODE]);
    extract_flags();
    set_cpu_mode(cpu_modes[reg[REG_CPSR] & 0xF]);

    if((io_registers[REG_IE] & io_registers[REG_IF]) &&
     io_registers[REG_IME] && ((reg[REG_CPSR] & 0x80) == 0))
    {
      REG_MODE(MODE_IRQ)[6] = reg[REG_PC] + 4;
      REG_SPSR(MODE_IRQ) = reg[REG_CPSR];
      reg[REG_CPSR] = 0xD2;
      address = 0x00000018;
      set_cpu_mode(MODE_IRQ);
    }

    if(reg[REG_CPSR] & 0x20)
      address |= 0x01;
  }

  return address;
}

#define generate_store_reg_pc_flags(ireg, reg_index)                          \
  generate_store_reg(ireg, reg_index);                                        \
  if(reg_index == 15)                                                         \
  {                                                                           \
    generate_mov(arg0, ireg);                                                 \
    generate_function_call(execute_spsr_restore);                             \
    generate_indirect_branch_dual();                                          \
  }                                                                           \

// These generate a branch on the opposite condition on purpose.
// For ARM mode we aim to skip instructions (therefore opposite)
// In Thumb mode we skip the conditional branch in a similar way
#define generate_condition_eq(ireg)                                           \
  generate_and_mem(1, base, REG_Z_FLAG * 4);                                  \
  x86_emit_j_filler(x86_condition_code_z, backpatch_address)                  \

#define generate_condition_ne(ireg)                                           \
  generate_and_mem(1, base, REG_Z_FLAG * 4);                                  \
  x86_emit_j_filler(x86_condition_code_nz, backpatch_address)                 \

#define generate_condition_cs(ireg)                                           \
  generate_and_mem(1, base, REG_C_FLAG * 4);                                  \
  x86_emit_j_filler(x86_condition_code_z, backpatch_address)                  \

#define generate_condition_cc(ireg)                                           \
  generate_and_mem(1, base, REG_C_FLAG * 4);                                  \
  x86_emit_j_filler(x86_condition_code_nz, backpatch_address)                 \

#define generate_condition_mi(ireg)                                           \
  generate_and_mem(1, base, REG_N_FLAG * 4);                                  \
  x86_emit_j_filler(x86_condition_code_z, backpatch_address)                  \

#define generate_condition_pl(ireg)                                           \
  generate_and_mem(1, base, REG_N_FLAG * 4);                                  \
  x86_emit_j_filler(x86_condition_code_nz, backpatch_address)                 \

#define generate_condition_vs(ireg)                                           \
  generate_and_mem(1, base, REG_V_FLAG * 4);                                  \
  x86_emit_j_filler(x86_condition_code_z, backpatch_address)                  \

#define generate_condition_vc(ireg)                                           \
  generate_and_mem(1, base, REG_V_FLAG * 4);                                  \
  x86_emit_j_filler(x86_condition_code_nz, backpatch_address)                 \

#define generate_condition_hi(ireg)                                           \
  generate_load_reg(ireg, REG_C_FLAG);                                        \
  generate_xor_imm(ireg, 1);                                                  \
  generate_or_mem(ireg, REG_Z_FLAG);                                          \
  x86_emit_j_filler(x86_condition_code_nz, backpatch_address)                 \

#define generate_condition_ls(ireg)                                           \
  generate_load_reg(ireg, REG_C_FLAG);                                        \
  generate_xor_imm(ireg, 1);                                                  \
  generate_or_mem(ireg, REG_Z_FLAG);                                          \
  x86_emit_j_filler(x86_condition_code_z, backpatch_address)                  \

#define generate_condition_ge(ireg)                                           \
  generate_load_reg(ireg, REG_N_FLAG);                                        \
  generate_cmp_memreg(ireg, REG_V_FLAG);                                      \
  x86_emit_j_filler(x86_condition_code_nz, backpatch_address)                 \

#define generate_condition_lt(ireg)                                           \
  generate_load_reg(ireg, REG_N_FLAG);                                        \
  generate_cmp_memreg(ireg, REG_V_FLAG);                                      \
  x86_emit_j_filler(x86_condition_code_z, backpatch_address)                  \

#define generate_condition_gt(ireg)                                           \
  generate_load_reg(ireg, REG_N_FLAG);                                        \
  generate_xor_mem(ireg, REG_V_FLAG);                                         \
  generate_or_mem(ireg, REG_Z_FLAG);                                          \
  x86_emit_j_filler(x86_condition_code_nz, backpatch_address)                 \

#define generate_condition_le(ireg)                                           \
  generate_load_reg(ireg, REG_N_FLAG);                                        \
  generate_xor_mem(ireg, REG_V_FLAG);                                         \
  generate_or_mem(ireg, REG_Z_FLAG);                                          \
  x86_emit_j_filler(x86_condition_code_z, backpatch_address)                  \


#define generate_condition(ireg)                                              \
  switch(condition)                                                           \
  {                                                                           \
    case 0x0:                                                                 \
      generate_condition_eq(ireg);                                            \
      break;                                                                  \
                                                                              \
    case 0x1:                                                                 \
      generate_condition_ne(ireg);                                            \
      break;                                                                  \
                                                                              \
    case 0x2:                                                                 \
      generate_condition_cs(ireg);                                            \
      break;                                                                  \
                                                                              \
    case 0x3:                                                                 \
      generate_condition_cc(ireg);                                            \
      break;                                                                  \
                                                                              \
    case 0x4:                                                                 \
      generate_condition_mi(ireg);                                            \
      break;                                                                  \
                                                                              \
    case 0x5:                                                                 \
      generate_condition_pl(ireg);                                            \
      break;                                                                  \
                                                                              \
    case 0x6:                                                                 \
      generate_condition_vs(ireg);                                            \
      break;                                                                  \
                                                                              \
    case 0x7:                                                                 \
      generate_condition_vc(ireg);                                            \
      break;                                                                  \
                                                                              \
    case 0x8:                                                                 \
      generate_condition_hi(ireg);                                            \
      break;                                                                  \
                                                                              \
    case 0x9:                                                                 \
      generate_condition_ls(ireg);                                            \
      break;                                                                  \
                                                                              \
    case 0xA:                                                                 \
      generate_condition_ge(ireg);                                            \
      break;                                                                  \
                                                                              \
    case 0xB:                                                                 \
      generate_condition_lt(ireg);                                            \
      break;                                                                  \
                                                                              \
    case 0xC:                                                                 \
      generate_condition_gt(ireg);                                            \
      break;                                                                  \
                                                                              \
    case 0xD:                                                                 \
      generate_condition_le(ireg);                                            \
      break;                                                                  \
                                                                              \
    case 0xE:                                                                 \
      /* AL       */                                                          \
      break;                                                                  \
                                                                              \
    case 0xF:                                                                 \
      /* Reserved */                                                          \
      break;                                                                  \
  }                                                                           \

#define generate_branch()                                                     \
{                                                                             \
  if(condition == 0x0E)                                                       \
  {                                                                           \
    generate_branch_cycle_update(                                             \
     block_exits[block_exit_position].branch_source,                          \
     block_exits[block_exit_position].branch_target);                         \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_branch_no_cycle_update(                                          \
     block_exits[block_exit_position].branch_source,                          \
     block_exits[block_exit_position].branch_target);                         \
  }                                                                           \
  block_exit_position++;                                                      \
}                                                                             \

#define rm_op_reg rm
#define rm_op_imm imm

#define arm_data_proc_reg_flags()                                             \
  arm_decode_data_proc_reg(opcode);                                           \
  if(flag_status & 0x02)                                                      \
  {                                                                           \
    generate_load_rm_sh(flags)                                                \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_rm_sh(no_flags);                                            \
  }                                                                           \

#define arm_data_proc_reg()                                                   \
  arm_decode_data_proc_reg(opcode);                                           \
  generate_load_rm_sh(no_flags)                                               \

#define arm_data_proc_imm()                                                   \
  arm_decode_data_proc_imm(opcode);                                           \
  ror(imm, imm, imm_ror);                                                     \
  generate_load_imm(a0, imm)                                                  \

#define arm_data_proc_imm_flags()                                             \
  arm_decode_data_proc_imm(opcode);                                           \
  if((flag_status & 0x02) && (imm_ror != 0))                                  \
  {                                                                           \
    /* Generate carry flag from integer rotation */                           \
    generate_load_imm(a0, ((imm >> (imm_ror - 1)) & 0x01));                   \
    generate_store_reg(a0, REG_C_FLAG);                                       \
  }                                                                           \
  ror(imm, imm, imm_ror);                                                     \
  generate_load_imm(a0, imm)                                                  \


#define arm_data_proc(name, type, flags_op)                                   \
{                                                                             \
  arm_data_proc_##type();                                                     \
  generate_load_reg_pc(a1, rn, 8);                                            \
  arm_data_proc_##name(rd, generate_store_reg_pc_##flags_op);                 \
}                                                                             \

#define arm_data_proc_test(name, type)                                        \
{                                                                             \
  arm_data_proc_##type();                                                     \
  generate_load_reg_pc(a1, rn, 8);                                            \
  arm_data_proc_test_##name();                                                \
}                                                                             \

#define arm_data_proc_unary(name, type, flags_op)                             \
{                                                                             \
  arm_data_proc_##type();                                                     \
  arm_data_proc_unary_##name(rd, generate_store_reg_pc_##flags_op);           \
}                                                                             \

#define arm_data_proc_mov(type)                                               \
{                                                                             \
  arm_data_proc_##type();                                                     \
  generate_store_reg_pc_no_flags(a0, rd);                                     \
}                                                                             \

#define arm_multiply_flags_yes()                                              \
  generate_and(a0, a0);                                                       \
  generate_update_flag(z, REG_Z_FLAG)                                         \
  generate_update_flag(s, REG_N_FLAG)

#define arm_multiply_flags_no(_dest)                                          \

#define arm_multiply_add_no()                                                 \

#define arm_multiply_add_yes()                                                \
  generate_load_reg(a1, rn);                                                  \
  generate_add(a0, a1)                                                        \

#define arm_multiply(add_op, flags)                                           \
{                                                                             \
  arm_decode_multiply();                                                      \
  generate_load_reg(a0, rm);                                                  \
  generate_load_reg(a1, rs);                                                  \
  generate_multiply(a1);                                                      \
  arm_multiply_add_##add_op();                                                \
  arm_multiply_flags_##flags();                                               \
  generate_store_reg(a0, rd);                                                 \
}                                                                             \

#define arm_multiply_long_flags_yes()                                         \
  generate_mov(t0, a1);                                                       \
  generate_and(t0, t0);                                                       \
  generate_update_flag(s, REG_N_FLAG)                                         \
  generate_or(t0, a0);                                                        \
  generate_update_flag(z, REG_Z_FLAG)                                         \

#define arm_multiply_long_flags_no(_dest)                                     \

#define arm_multiply_long_add_yes(name)                                       \
  generate_load_reg(a2, rdlo);                                                \
  generate_load_reg(t0, rdhi);                                                \
  generate_multiply_##name(a1, a2, t0)                                        \

#define arm_multiply_long_add_no(name)                                        \
  generate_multiply_##name(a1)                                                \

#define arm_multiply_long(name, add_op, flags)                                \
{                                                                             \
  arm_decode_multiply_long();                                                 \
  generate_load_reg(a0, rm);                                                  \
  generate_load_reg(a1, rs);                                                  \
  arm_multiply_long_add_##add_op(name);                                       \
  generate_store_reg(a0, rdlo);                                               \
  generate_store_reg(a1, rdhi);                                               \
  arm_multiply_long_flags_##flags();                                          \
}                                                                             \

#define execute_read_cpsr(oreg)                                               \
  collapse_flags(oreg, a2)

#define execute_read_spsr(oreg)                                               \
  collapse_flags(oreg, a2);                                                   \
  generate_load_reg(oreg, CPU_MODE);                                          \
  generate_and_imm(oreg, 0xF);                                                \
  generate_load_spsr(oreg, oreg);                                             \

#define arm_psr_read(op_type, psr_reg)                                        \
  execute_read_##psr_reg(rv);                                                 \
  generate_store_reg(rv, rd)                                                  \

// Does mode-change magic (including IRQ checks)
u32 execute_store_cpsr_body()
{
  set_cpu_mode(cpu_modes[reg[REG_CPSR] & 0xF]);
  if((io_registers[REG_IE] & io_registers[REG_IF]) &&
      io_registers[REG_IME] && ((reg[REG_CPSR] & 0x80) == 0))
  {
    REG_MODE(MODE_IRQ)[6] = reg[REG_PC] + 4;
    REG_SPSR(MODE_IRQ) = reg[REG_CPSR];
    reg[REG_CPSR] = (reg[REG_CPSR] & 0xFFFFFF00) | 0xD2;
    set_cpu_mode(MODE_IRQ);
    return 0x00000018;
  }

  return 0;
}


#define arm_psr_load_new_reg()                                                \
  generate_load_reg(a0, rm)                                                   \

#define arm_psr_load_new_imm()                                                \
  ror(imm, imm, imm_ror);                                                     \
  generate_load_imm(a0, imm)                                                  \

#define execute_store_cpsr()                                                  \
  generate_load_imm(a1, cpsr_masks[psr_pfield][0]);                           \
  generate_load_imm(a2, cpsr_masks[psr_pfield][1]);                           \
  generate_store_reg_i32(pc, REG_PC);                                         \
  generate_function_call(execute_store_cpsr)                                  \

/* REG_SPSR(reg[CPU_MODE]) = (new_spsr & store_mask) | (old_spsr & (~store_mask))*/
#define execute_store_spsr()                                                  \
  generate_load_reg(a2, CPU_MODE);                                            \
  generate_and_imm(a2, 0xF);                                                  \
  generate_load_spsr(a1, a2);                                                 \
  generate_and_imm(a0,  spsr_masks[psr_pfield]);                              \
  generate_and_imm(a1, ~spsr_masks[psr_pfield]);                              \
  generate_or(a0, a1);                                                        \
  generate_store_spsr(a0, a2);                                                \

#define arm_psr_store(op_type, psr_reg)                                       \
  arm_psr_load_new_##op_type();                                               \
  execute_store_##psr_reg();                                                  \

#define arm_psr(op_type, transfer_type, psr_reg)                              \
{                                                                             \
  arm_decode_psr_##op_type(opcode);                                           \
  arm_psr_##transfer_type(op_type, psr_reg);                                  \
}                                                                             \

#define arm_access_memory_load(mem_type)                                      \
  cycle_count += 2;                                                           \
  generate_load_pc(a1, pc);                                                   \
  generate_function_call(execute_load_##mem_type);                            \
  generate_store_reg_pc_no_flags(rv, rd)                                      \

#define arm_access_memory_store(mem_type)                                     \
  cycle_count++;                                                              \
  generate_load_reg_pc(a1, rd, 12);                                           \
  generate_store_reg_i32(pc + 4, REG_PC);                                     \
  generate_function_call(execute_store_##mem_type)                            \

#define no_op                                                                 \

#define arm_access_memory_writeback_yes(off_op)                               \
  reg[rn] = address off_op                                                    \

#define arm_access_memory_writeback_no(off_op)                                \

#define load_reg_op reg[rd]                                                   \

#define store_reg_op reg_op                                                   \

#define arm_access_memory_adjust_op_up      add
#define arm_access_memory_adjust_op_down    sub
#define arm_access_memory_reverse_op_up     sub
#define arm_access_memory_reverse_op_down   add

#define arm_access_memory_reg_pre(adjust_dir_op, reverse_dir_op)              \
  generate_load_reg_pc(a0, rn, 8);                                            \
  generate_##adjust_dir_op(a0, a1)                                            \

#define arm_access_memory_reg_pre_wb(adjust_dir_op, reverse_dir_op)           \
  arm_access_memory_reg_pre(adjust_dir_op, reverse_dir_op);                   \
  generate_store_reg(a0, rn)                                                  \

#define arm_access_memory_reg_post(adjust_dir_op, reverse_dir_op)             \
  generate_load_reg(a0, rn);                                                  \
  generate_##adjust_dir_op(a0, a1);                                           \
  generate_store_reg(a0, rn);                                                 \
  generate_##reverse_dir_op(a0, a1)                                           \

#define arm_access_memory_imm_pre(adjust_dir_op, reverse_dir_op)              \
  generate_load_reg_pc(a0, rn, 8);                                            \
  generate_##adjust_dir_op##_imm(a0, offset)                                  \

#define arm_access_memory_imm_pre_wb(adjust_dir_op, reverse_dir_op)           \
  arm_access_memory_imm_pre(adjust_dir_op, reverse_dir_op);                   \
  generate_store_reg(a0, rn)                                                  \

#define arm_access_memory_imm_post(adjust_dir_op, reverse_dir_op)             \
  generate_load_reg(a0, rn);                                                  \
  generate_##adjust_dir_op##_imm(a0, offset);                                 \
  generate_store_reg(a0, rn);                                                 \
  generate_##reverse_dir_op##_imm(a0, offset)                                 \


#define arm_data_trans_reg(adjust_op, adjust_dir_op, reverse_dir_op)          \
  arm_decode_data_trans_reg();                                                \
  generate_load_offset_sh();                                                  \
  arm_access_memory_reg_##adjust_op(adjust_dir_op, reverse_dir_op)            \

#define arm_data_trans_imm(adjust_op, adjust_dir_op, reverse_dir_op)          \
  arm_decode_data_trans_imm();                                                \
  arm_access_memory_imm_##adjust_op(adjust_dir_op, reverse_dir_op)            \

#define arm_data_trans_half_reg(adjust_op, adjust_dir_op, reverse_dir_op)     \
  arm_decode_half_trans_r();                                                  \
  generate_load_reg(a1, rm);                                                  \
  arm_access_memory_reg_##adjust_op(adjust_dir_op, reverse_dir_op)            \

#define arm_data_trans_half_imm(adjust_op, adjust_dir_op, reverse_dir_op)     \
  arm_decode_half_trans_of();                                                 \
  arm_access_memory_imm_##adjust_op(adjust_dir_op, reverse_dir_op)            \

#define arm_access_memory(access_type, direction, adjust_op, mem_type,        \
 offset_type)                                                                 \
{                                                                             \
  arm_data_trans_##offset_type(adjust_op,                                     \
   arm_access_memory_adjust_op_##direction,                                   \
   arm_access_memory_reverse_op_##direction);                                 \
                                                                              \
  arm_access_memory_##access_type(mem_type);                                  \
}                                                                             \

#define word_bit_count(word)                                                  \
  (bit_count[word >> 8] + bit_count[word & 0xFF])                             \


#define arm_block_memory_load()                                               \
  generate_load_pc(a1, pc);                                                   \
  generate_function_call(execute_load_u32);                                   \
  generate_store_reg(rv, i)                                                   \

#define arm_block_memory_store()                                              \
  generate_load_reg_pc(a1, i, 8);                                             \
  generate_function_call(execute_store_aligned_u32)                           \

#define arm_block_memory_final_load(writeback_type)                           \
  arm_block_memory_load()                                                     \

#define arm_block_memory_final_store(writeback_type)                          \
  generate_load_reg_pc(a1, i, 12);                                            \
  arm_block_memory_writeback_post_store(writeback_type);                      \
  generate_store_reg_i32(pc + 4, REG_PC);                                     \
  generate_function_call(execute_store_u32)                                   \

#define arm_block_memory_adjust_pc_store()                                    \

#define arm_block_memory_adjust_pc_load()                                     \
  if(reg_list & 0x8000)                                                       \
  {                                                                           \
    generate_indirect_branch_arm();                                           \
  }                                                                           \

#define arm_block_memory_offset_down_a()                                      \
  generate_add_imm(a0, -((word_bit_count(reg_list) * 4) - 4))                 \

#define arm_block_memory_offset_down_b()                                      \
  generate_add_imm(a0, -(word_bit_count(reg_list) * 4))                       \

#define arm_block_memory_offset_no()                                          \

#define arm_block_memory_offset_up()                                          \
  generate_add_imm(a0, 4)                                                     \

#define arm_block_memory_writeback_down()                                     \
  generate_load_reg(a2, rn)                                                   \
  generate_add_imm(a2, -(word_bit_count(reg_list) * 4));                      \
  generate_store_reg(a2, rn)                                                  \

#define arm_block_memory_writeback_up()                                       \
  generate_load_reg(a2, rn);                                                  \
  generate_add_imm(a2, (word_bit_count(reg_list) * 4));                       \
  generate_store_reg(a2, rn)                                                  \

#define arm_block_memory_writeback_no()

// Only emit writeback if the register is not in the list

#define arm_block_memory_writeback_pre_load(writeback_type)                   \
  if(!((reg_list >> rn) & 0x01))                                              \
  {                                                                           \
    arm_block_memory_writeback_##writeback_type();                            \
  }                                                                           \

#define arm_block_memory_writeback_pre_store(writeback_type)                  \

#define arm_block_memory_writeback_post_store(writeback_type)                 \
  arm_block_memory_writeback_##writeback_type()                               \

#define arm_block_memory(access_type, offset_type, writeback_type, s_bit)     \
{                                                                             \
  arm_decode_block_trans();                                                   \
  u32 offset = 0;                                                             \
  u32 i;                                                                      \
                                                                              \
  generate_load_reg(a0, rn);                                                  \
  arm_block_memory_offset_##offset_type();                                    \
  generate_and_imm(a0, ~0x03);                                                \
  generate_store_reg(a0, REG_SAVE3);                                          \
  arm_block_memory_writeback_pre_##access_type(writeback_type);               \
                                                                              \
  for(i = 0; i < 16; i++)                                                     \
  {                                                                           \
    if((reg_list >> i) & 0x01)                                                \
    {                                                                         \
      cycle_count++;                                                          \
      generate_load_reg(a0, REG_SAVE3);                                       \
      generate_add_imm(a0, offset)                                            \
      if(reg_list & ~((2 << i) - 1))                                          \
      {                                                                       \
        arm_block_memory_##access_type();                                     \
        offset += 4;                                                          \
      }                                                                       \
      else                                                                    \
      {                                                                       \
        arm_block_memory_final_##access_type(writeback_type);                 \
      }                                                                       \
    }                                                                         \
  }                                                                           \
                                                                              \
  arm_block_memory_adjust_pc_##access_type();                                 \
}                                                                             \

#define arm_swap(type)                                                        \
{                                                                             \
  arm_decode_swap();                                                          \
  cycle_count += 3;                                                           \
  generate_load_reg(a0, rn);                                                  \
  generate_load_pc(a1, pc);                                                   \
  generate_function_call(execute_load_##type);                                \
  generate_mov(a2, rv);                                                       \
  generate_load_reg(a0, rn);                                                  \
  generate_load_reg(a1, rm);                                                  \
  generate_store_reg(a2, rd);                                                 \
  generate_function_call(execute_store_##type);                               \
}                                                                             \

#define thumb_rn_op_reg(_rn)                                                  \
  generate_load_reg(a0, _rn)                                                  \

#define thumb_rn_op_imm(_imm)                                                 \
  generate_load_imm(a0, _imm)                                                 \

// Types: add_sub, add_sub_imm, alu_op, imm
// Affects N/Z/C/V flags

#define thumb_data_proc(type, name, rn_type, _rd, _rs, _rn)                   \
{                                                                             \
  thumb_decode_##type();                                                      \
  thumb_rn_op_##rn_type(_rn);                                                 \
  generate_load_reg(a1, _rs);                                                 \
  arm_data_proc_##name(_rd, generate_store_reg);                              \
}                                                                             \

#define thumb_data_proc_test(type, name, rn_type, _rs, _rn)                   \
{                                                                             \
  thumb_decode_##type();                                                      \
  thumb_rn_op_##rn_type(_rn);                                                 \
  generate_load_reg(a1, _rs);                                                 \
  arm_data_proc_test_##name();                                                \
}                                                                             \

#define thumb_data_proc_unary(type, name, rn_type, _rd, _rn)                  \
{                                                                             \
  thumb_decode_##type();                                                      \
  thumb_rn_op_##rn_type(_rn);                                                 \
  arm_data_proc_unary_##name(_rd, generate_store_reg);                        \
}                                                                             \

#define thumb_data_proc_mov(type, rn_type, _rd, _rn)                          \
{                                                                             \
  thumb_decode_##type();                                                      \
  thumb_rn_op_##rn_type(_rn);                                                 \
  generate_store_reg(a0, _rd);                                                \
}                                                                             \

#define generate_store_reg_pc_thumb(ireg, rd)                                 \
  generate_store_reg(ireg, rd);                                               \
  if(rd == 15)                                                                \
  {                                                                           \
    generate_indirect_branch_cycle_update(thumb);                             \
  }                                                                           \

#define thumb_data_proc_hi(name)                                              \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  generate_load_reg_pc(a0, rs, 4);                                            \
  generate_load_reg_pc(a1, rd, 4);                                            \
  arm_data_proc_##name(rd, generate_store_reg_pc_thumb);                      \
}                                                                             \

#define thumb_data_proc_test_hi(name)                                         \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  generate_load_reg_pc(a0, rs, 4);                                            \
  generate_load_reg_pc(a1, rd, 4);                                            \
  arm_data_proc_test_##name();                                                \
}                                                                             \

#define thumb_data_proc_unary_hi(name)                                        \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  generate_load_reg_pc(a0, rn, 4);                                            \
  arm_data_proc_unary_##name(rd, generate_store_reg_pc_thumb);                \
}                                                                             \

#define thumb_data_proc_mov_hi()                                              \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  generate_load_reg_pc(a0, rs, 4);                                            \
  generate_store_reg_pc_thumb(a0, rd);                                        \
}                                                                             \

#define thumb_load_pc(_rd)                                                    \
{                                                                             \
  thumb_decode_imm();                                                         \
  generate_load_pc(a0, (((pc & ~2) + 4) + (imm * 4)));                        \
  generate_store_reg(a0, _rd);                                                \
}                                                                             \

#define thumb_load_sp(_rd)                                                    \
{                                                                             \
  thumb_decode_imm();                                                         \
  generate_load_reg(a0, 13);                                                  \
  generate_add_imm(a0, (imm * 4));                                            \
  generate_store_reg(a0, _rd);                                                \
}                                                                             \

#define thumb_adjust_sp_up()                                                  \
  generate_add_imm(a0, imm * 4)                                               \

#define thumb_adjust_sp_down()                                                \
  generate_sub_imm(a0, imm * 4)                                               \


#define thumb_adjust_sp(direction)                                            \
{                                                                             \
  thumb_decode_add_sp();                                                      \
  generate_load_reg(a0, REG_SP);                                              \
  thumb_adjust_sp_##direction();                                              \
  generate_store_reg(a0, REG_SP);                                             \
}                                                                             \

// Decode types: shift, alu_op
// Operation types: lsl, lsr, asr, ror
// Affects N/Z/C flags

#define thumb_lsl_imm_op()                                                    \
  if (imm) {                                                                  \
    generate_shift_left(a0, imm);                                             \
    generate_update_flag(c, REG_C_FLAG)                                       \
  } else {                                                                    \
    generate_or(a0, a0);                                                      \
  }                                                                           \
  update_logical_flags()                                                      \

#define thumb_lsr_imm_op()                                                    \
  if (imm) {                                                                  \
    generate_shift_right(a0, imm);                                            \
    generate_update_flag(c, REG_C_FLAG)                                       \
  } else {                                                                    \
    generate_shift_right(a0, 31);                                             \
    generate_update_flag(nz, REG_C_FLAG)                                      \
    generate_xor(a0, a0);                                                     \
  }                                                                           \
  update_logical_flags()                                                      \

#define thumb_asr_imm_op()                                                    \
  if (imm) {                                                                  \
    generate_shift_right_arithmetic(a0, imm);                                 \
    generate_update_flag(c, REG_C_FLAG)                                       \
  } else {                                                                    \
    generate_shift_right_arithmetic(a0, 31);                                  \
    generate_update_flag(s, REG_C_FLAG)                                       \
  }                                                                           \
  update_logical_flags()                                                      \

#define thumb_ror_imm_op()                                                    \
  if (imm) {                                                                  \
    generate_rotate_right(a0, imm);                                           \
    generate_update_flag(c, REG_C_FLAG)                                       \
  } else {                                                                    \
    generate_rrx_flags(a0);                                                   \
  }                                                                           \
  update_logical_flags()                                                      \


#define generate_shift_load_operands_reg()                                    \
  generate_load_reg(a0, rd);                                                  \
  generate_load_reg(a1, rs)                                                   \

#define generate_shift_load_operands_imm()                                    \
  generate_load_reg(a0, rs);                                                  \
  generate_load_imm(a1, imm)                                                  \

#define thumb_shift_operation_imm(op_type)                                    \
  thumb_##op_type##_imm_op()

#define thumb_shift_operation_reg(op_type)                                    \
  generate_##op_type##_flags_reg(a0);                                         \
  generate_or(a0, a0);                                                        \
  update_logical_flags()                                                      \

#define thumb_shift(decode_type, op_type, value_type)                         \
{                                                                             \
  thumb_decode_##decode_type();                                               \
  generate_shift_load_operands_##value_type();                                \
  thumb_shift_operation_##value_type(op_type);                                \
  generate_store_reg(rv, rd);                                                 \
}                                                                             \

// Operation types: imm, mem_reg, mem_imm

#define thumb_load_pc_pool_const(reg_rd, value)                               \
  generate_store_reg_i32(value, reg_rd)                                       \

#define thumb_access_memory_load(mem_type, reg_rd)                            \
  cycle_count += 2;                                                           \
  generate_load_pc(a1, pc);                                                   \
  generate_function_call(execute_load_##mem_type);                            \
  generate_store_reg(rv, reg_rd)                                              \

#define thumb_access_memory_store(mem_type, reg_rd)                           \
  cycle_count++;                                                              \
  generate_load_reg(a1, reg_rd);                                              \
  generate_store_reg_i32(pc + 2, REG_PC);                                     \
  generate_function_call(execute_store_##mem_type)                            \

#define thumb_access_memory_generate_address_pc_relative(offset, _rb, _ro)    \
  generate_load_pc(a0, (offset))                                              \

#define thumb_access_memory_generate_address_reg_imm_sp(offset, _rb, _ro)     \
  generate_load_reg(a0, _rb);                                                 \
  generate_add_imm(a0, (offset * 4))                                          \

#define thumb_access_memory_generate_address_reg_imm(offset, _rb, _ro)        \
  generate_load_reg(a0, _rb);                                                 \
  generate_add_imm(a0, (offset))                                              \

#define thumb_access_memory_generate_address_reg_reg(offset, _rb, _ro)        \
  generate_load_reg(a0, _rb);                                                 \
  generate_load_reg(a1, _ro);                                                 \
  generate_add(a0, a1)                                                        \

#define thumb_access_memory(access_type, op_type, _rd, _rb, _ro,              \
 address_type, offset, mem_type)                                              \
{                                                                             \
  thumb_decode_##op_type();                                                   \
  thumb_access_memory_generate_address_##address_type(offset, _rb, _ro);      \
  thumb_access_memory_##access_type(mem_type, _rd);                           \
}                                                                             \

#define thumb_block_address_preadjust_up()                                    \
  generate_add_imm(a0, (bit_count[reg_list] * 4))                             \

#define thumb_block_address_preadjust_down()                                  \
  generate_sub_imm(a0, (bit_count[reg_list] * 4))                             \

#define thumb_block_address_preadjust_push_lr()                               \
  generate_sub_imm(a0, ((bit_count[reg_list] + 1) * 4))                       \

#define thumb_block_address_preadjust_no()                                    \

#define thumb_block_address_postadjust_no(base_reg)                           \
  generate_store_reg(a0, base_reg)                                            \

#define thumb_block_address_postadjust_up(base_reg)                           \
  generate_add_imm(a0, (bit_count[reg_list] * 4));                            \
  generate_store_reg(a0, base_reg)                                            \

#define thumb_block_address_postadjust_down(base_reg)                         \
  generate_sub_imm(a0, (bit_count[reg_list] * 4));                            \
  generate_store_reg(a0, base_reg)                                            \

#define thumb_block_address_postadjust_pop_pc(base_reg)                       \
  generate_add_imm(a0, ((bit_count[reg_list] + 1) * 4));                      \
  generate_store_reg(a0, base_reg)                                            \

#define thumb_block_address_postadjust_push_lr(base_reg)                      \
  generate_store_reg(a0, base_reg)                                            \

#define thumb_block_memory_extra_no()                                         \

#define thumb_block_memory_extra_up()                                         \

#define thumb_block_memory_extra_down()                                       \

#define thumb_block_memory_extra_pop_pc()                                     \
  generate_load_reg(a0, REG_SAVE3);                                           \
  generate_add_imm(a0, (bit_count[reg_list] * 4));                            \
  generate_load_pc(a1, pc);                                                   \
  generate_function_call(execute_load_u32);                                   \
  generate_store_reg(rv, REG_PC);                                             \
  generate_indirect_branch_cycle_update(thumb)                                \

#define thumb_block_memory_extra_push_lr(base_reg)                            \
  generate_load_reg(a0, REG_SAVE3);                                           \
  generate_add_imm(a0, (bit_count[reg_list] * 4));                            \
  generate_load_reg(a1, REG_LR);                                              \
  generate_function_call(execute_store_aligned_u32)                           \

#define thumb_block_memory_load()                                             \
  generate_load_pc(a1, pc);                                                   \
  generate_function_call(execute_load_u32);                                   \
  generate_store_reg(rv, i)                                                   \

#define thumb_block_memory_store()                                            \
  generate_load_reg(a1, i);                                                   \
  generate_function_call(execute_store_aligned_u32)                           \

#define thumb_block_memory_final_load()                                       \
  thumb_block_memory_load()                                                   \

#define thumb_block_memory_final_store()                                      \
  generate_load_reg(a1, i);                                                   \
  generate_store_reg_i32(pc + 2, REG_PC);                                     \
  generate_function_call(execute_store_u32)                                   \

#define thumb_block_memory_final_no(access_type)                              \
  thumb_block_memory_final_##access_type()                                    \

#define thumb_block_memory_final_up(access_type)                              \
  thumb_block_memory_final_##access_type()                                    \

#define thumb_block_memory_final_down(access_type)                            \
  thumb_block_memory_final_##access_type()                                    \

#define thumb_block_memory_final_push_lr(access_type)                         \
  thumb_block_memory_##access_type()                                          \

#define thumb_block_memory_final_pop_pc(access_type)                          \
  thumb_block_memory_##access_type()                                          \

#define thumb_block_memory(access_type, pre_op, post_op, base_reg)            \
{                                                                             \
  thumb_decode_rlist();                                                       \
  u32 i;                                                                      \
  u32 offset = 0;                                                             \
                                                                              \
  generate_load_reg(a0, base_reg);                                            \
  generate_and_imm(a0, ~0x03);                                                \
  thumb_block_address_preadjust_##pre_op();                                   \
  generate_store_reg(a0, REG_SAVE3);                                          \
  thumb_block_address_postadjust_##post_op(base_reg);                         \
                                                                              \
  for(i = 0; i < 8; i++)                                                      \
  {                                                                           \
    if((reg_list >> i) & 0x01)                                                \
    {                                                                         \
      cycle_count++;                                                          \
      generate_load_reg(a0, REG_SAVE3);                                       \
      generate_add_imm(a0, offset)                                            \
      if(reg_list & ~((2 << i) - 1))                                          \
      {                                                                       \
        thumb_block_memory_##access_type();                                   \
        offset += 4;                                                          \
      }                                                                       \
      else                                                                    \
      {                                                                       \
        thumb_block_memory_final_##post_op(access_type);                      \
      }                                                                       \
    }                                                                         \
  }                                                                           \
                                                                              \
  thumb_block_memory_extra_##post_op();                                       \
}                                                                             \


#define thumb_conditional_branch(condition)                                   \
{                                                                             \
  generate_cycle_update();                                                    \
  generate_condition_##condition(a0);                                         \
  generate_branch_no_cycle_update(                                            \
   block_exits[block_exit_position].branch_source,                            \
   block_exits[block_exit_position].branch_target);                           \
  generate_branch_patch_conditional(backpatch_address, translation_ptr);      \
  block_exit_position++;                                                      \
}                                                                             \


// Execute functions

#define update_logical_flags()                                                \
  if (check_generate_z_flag) {                                                \
    generate_update_flag(z, REG_Z_FLAG)                                       \
  }                                                                           \
  if (check_generate_n_flag) {                                                \
    generate_update_flag(s, REG_N_FLAG)                                       \
  }                                                                           \

#define update_add_flags()                                                    \
  update_logical_flags()                                                      \
  if (check_generate_c_flag) {                                                \
    generate_update_flag(c, REG_C_FLAG)                                       \
  }                                                                           \
  if (check_generate_v_flag) {                                                \
    generate_update_flag(o, REG_V_FLAG)                                       \
  }                                                                           \

#define update_sub_flags()                                                    \
  update_logical_flags()                                                      \
  if (check_generate_c_flag) {                                                \
    generate_update_flag(nc, REG_C_FLAG)                                      \
  }                                                                           \
  if (check_generate_v_flag) {                                                \
    generate_update_flag(o, REG_V_FLAG)                                       \
  }                                                                           \

#define arm_data_proc_and(rd, storefnc)                                       \
  generate_and(a0, a1);                                                       \
  storefnc(a0, rd);

#define arm_data_proc_ands(rd, storefnc)                                      \
  generate_and(a0, a1);                                                       \
  update_logical_flags();                                                     \
  storefnc(a0, rd);

#define arm_data_proc_eor(rd, storefnc)                                       \
  generate_xor(a0, a1);                                                       \
  storefnc(a0, rd);

#define arm_data_proc_eors(rd, storefnc)                                      \
  generate_xor(a0, a1);                                                       \
  update_logical_flags();                                                     \
  storefnc(a0, rd);

#define arm_data_proc_orr(rd, storefnc)                                       \
  generate_or(a0, a1);                                                        \
  storefnc(a0, rd);

#define arm_data_proc_orrs(rd, storefnc)                                      \
  generate_or(a0, a1);                                                        \
  update_logical_flags();                                                     \
  storefnc(a0, rd);

#define arm_data_proc_bic(rd, storefnc)                                       \
  generate_not(a0);                                                           \
  generate_and(a0, a1);                                                       \
  storefnc(a0, rd);

#define arm_data_proc_bics(rd, storefnc)                                      \
  generate_not(a0);                                                           \
  generate_and(a0, a1);                                                       \
  update_logical_flags();                                                     \
  storefnc(a0, rd);

#define arm_data_proc_add(rd, storefnc)                                       \
  generate_add(a0, a1);                                                       \
  storefnc(a0, rd);

#define arm_data_proc_adds(rd, storefnc)                                      \
  generate_add(a0, a1);                                                       \
  update_add_flags();                                                         \
  storefnc(a0, rd);

// Argument ordering is inverted between arm and x86
#define arm_data_proc_sub(rd, storefnc)                                       \
  generate_sub(a1, a0);                                                       \
  storefnc(a1, rd);

#define arm_data_proc_rsb(rd, storefnc)                                       \
  generate_sub(a0, a1);                                                       \
  storefnc(a0, rd);

// Borrow flag in ARM is opposite to carry flag in x86
#define arm_data_proc_subs(rd, storefnc)                                      \
  generate_sub(a1, a0);                                                       \
  update_sub_flags();                                                         \
  storefnc(a1, rd);

#define arm_data_proc_rsbs(rd, storefnc)                                      \
  generate_sub(a0, a1);                                                       \
  update_sub_flags();                                                         \
  storefnc(a0, rd);

#define arm_data_proc_mul(rd, storefnc)                                       \
  generate_multiply(a1);                                                      \
  storefnc(a0, rd);

#define arm_data_proc_muls(rd, storefnc)                                      \
  generate_multiply(a1);                                                      \
  generate_and(a0, a0);                                                       \
  update_logical_flags();                                                     \
  storefnc(a0, rd);

#define load_c_flag(tmpreg)                                                   \
  /* Loads the flag to the right value by adding it to ~0 causing carry */    \
  generate_load_imm(tmpreg, 0xffffffff);                                      \
  generate_add_memreg(tmpreg, REG_C_FLAG);                                    \

#define load_inv_c_flag(tmpreg)                                               \
  /* Loads the inverse C flag (for subtraction, since ARM's inverted) */      \
  generate_load_reg(tmpreg, REG_C_FLAG);                                      \
  generate_sub_imm(tmpreg, 1);                                                \


#define arm_data_proc_adc(rd, storefnc)                                       \
  load_c_flag(a2)                                                             \
  generate_adc(a0, a1);                                                       \
  storefnc(a0, rd);

#define arm_data_proc_adcs(rd, storefnc)                                      \
  load_c_flag(a2)                                                             \
  generate_adc(a0, a1);                                                       \
  update_add_flags()                                                          \
  storefnc(a0, rd);

#define arm_data_proc_sbc(rd, storefnc)                                       \
  load_inv_c_flag(a2)                                                         \
  generate_sbb(a1, a0);                                                       \
  storefnc(a1, rd);

#define arm_data_proc_sbcs(rd, storefnc)                                      \
  load_inv_c_flag(a2)                                                         \
  generate_sbb(a1, a0);                                                       \
  update_sub_flags()                                                          \
  storefnc(a1, rd);

#define arm_data_proc_rsc(rd, storefnc)                                       \
  load_inv_c_flag(a2)                                                         \
  generate_sbb(a0, a1);                                                       \
  storefnc(a0, rd);

#define arm_data_proc_rscs(rd, storefnc)                                      \
  load_inv_c_flag(a2)                                                         \
  generate_sbb(a0, a1);                                                       \
  update_sub_flags()                                                          \
  storefnc(a0, rd);


#define arm_data_proc_test_cmp()                                              \
  generate_sub(a1, a0);                                                       \
  update_sub_flags()

#define arm_data_proc_test_cmn()                                              \
  generate_add(a1, a0);                                                       \
  update_add_flags()

#define arm_data_proc_test_tst()                                              \
  generate_and(a0, a1);                                                       \
  update_logical_flags()

#define arm_data_proc_test_teq()                                              \
  generate_xor(a0, a1);                                                       \
  update_logical_flags()

#define arm_data_proc_unary_mov(rd, storefnc)                                 \
  storefnc(a0, rd);

#define arm_data_proc_unary_movs(rd, storefnc)                                \
  arm_data_proc_unary_mov(rd, storefnc);                                      \
  generate_or(a0, a0);                                                        \
  update_logical_flags()

#define arm_data_proc_unary_mvn(rd, storefnc)                                 \
  generate_not(a0);                                                           \
  storefnc(a0, rd);

#define arm_data_proc_unary_mvns(rd, storefnc)                                \
  arm_data_proc_unary_mvn(rd, storefnc);                                      \
  /* NOT does not update the flag register */                                 \
  generate_or(a0, a0);                                                        \
  update_logical_flags()

#define arm_data_proc_unary_neg(rd, storefnc)                                 \
  generate_xor(a1, a1);                                                       \
  arm_data_proc_subs(rd, storefnc)


static void function_cc execute_swi(u32 pc)
{
  // Open bus value after SWI
  reg[REG_BUS_VALUE] = 0xe3a02004;
  REG_MODE(MODE_SUPERVISOR)[6] = pc;
  REG_SPSR(MODE_SUPERVISOR) = reg[REG_CPSR];
  // Move to ARM mode, supervisor mode, disable IRQs
  reg[REG_CPSR] = (reg[REG_CPSR] & ~0x3F) | 0x13 | 0x80;
  set_cpu_mode(MODE_SUPERVISOR);
}

#define arm_conditional_block_header()                                        \
  generate_cycle_update();                                                    \
  generate_condition(a0);                                                     \

#define arm_b()                                                               \
  generate_branch()                                                           \

#define arm_bl()                                                              \
  generate_load_pc(a0, (pc + 4));                                             \
  generate_store_reg(a0, REG_LR);                                             \
  generate_branch()                                                           \

#define arm_bx()                                                              \
  arm_decode_branchx(opcode);                                                 \
  generate_load_reg(a0, rn);                                                  \
  generate_indirect_branch_dual();                                            \

#define arm_swi()                                                             \
  collapse_flags(a0, a1);                                                     \
  generate_load_pc(arg0, (pc + 4));                                           \
  generate_function_call(execute_swi);                                        \
  generate_branch()                                                           \

#define thumb_b()                                                             \
  generate_branch_cycle_update(                                               \
   block_exits[block_exit_position].branch_source,                            \
   block_exits[block_exit_position].branch_target);                           \
  block_exit_position++                                                       \

#define thumb_bl()                                                            \
  generate_load_pc(a0, ((pc + 2) | 0x01));                                    \
  generate_store_reg(a0, REG_LR);                                             \
  generate_branch_cycle_update(                                               \
   block_exits[block_exit_position].branch_source,                            \
   block_exits[block_exit_position].branch_target);                           \
  block_exit_position++                                                       \

#define thumb_blh()                                                           \
{                                                                             \
  thumb_decode_branch();                                                      \
  generate_load_pc(a0, ((pc + 2) | 0x01));                                    \
  generate_load_reg(a1, REG_LR);                                              \
  generate_store_reg(a0, REG_LR);                                             \
  generate_mov(a0, a1);                                                       \
  generate_add_imm(a0, (offset * 2));                                         \
  generate_indirect_branch_cycle_update(thumb);                               \
}                                                                             \

#define thumb_bx()                                                            \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  generate_load_reg_pc(a0, rs, 4);                                            \
  generate_indirect_branch_cycle_update(dual);                                \
}                                                                             \

#define thumb_process_cheats()                                                \
  generate_function_call(process_cheats);

#define arm_process_cheats()                                                  \
  generate_function_call(process_cheats);

#define thumb_swi()                                                           \
  collapse_flags(a0, a1);                                                     \
  generate_load_pc(arg0, (pc + 2));                                           \
  generate_function_call(execute_swi);                                        \
  generate_branch_cycle_update(                                               \
   block_exits[block_exit_position].branch_source,                            \
   block_exits[block_exit_position].branch_target);                           \
  block_exit_position++                                                       \

#define arm_hle_div(cpu_mode)                                                 \
{                                                                             \
  u8 *jmpinst;                                                                \
  generate_load_reg(a0, 0);                                                   \
  generate_load_reg(a2, 1);                                                   \
  generate_cmp_imm(a2, 0);                                                    \
  x86_emit_j_filler(x86_condition_code_z, jmpinst);                           \
  x86_emit_cdq();                                                             \
  x86_emit_idiv_eax_reg(ecx);                                                 \
  generate_store_reg(a0, 0);                                                  \
  generate_store_reg(a1, 1);                                                  \
  generate_mov(a1, a0);                                                       \
  generate_shift_right_arithmetic(a1, 31);                                    \
  generate_xor(a0, a1);                                                       \
  generate_sub(a0, a1);                                                       \
  generate_store_reg(a0, 3);                                                  \
  generate_branch_patch_conditional(jmpinst, translation_ptr);                \
}

#define arm_hle_div_arm(cpu_mode)                                             \
{                                                                             \
  u8 *jmpinst;                                                                \
  generate_load_reg(a0, 1);                                                   \
  generate_load_reg(a2, 0);                                                   \
  generate_cmp_imm(a2, 0);                                                    \
  x86_emit_j_filler(x86_condition_code_z, jmpinst);                           \
  x86_emit_cdq();                                                             \
  x86_emit_idiv_eax_reg(ecx);                                                 \
  generate_store_reg(a0, 0);                                                  \
  generate_store_reg(a1, 1);                                                  \
  generate_mov(a1, a0);                                                       \
  generate_shift_right_arithmetic(a1, 31);                                    \
  generate_xor(a0, a1);                                                       \
  generate_sub(a0, a1);                                                       \
  generate_store_reg(a0, 3);                                                  \
  generate_branch_patch_conditional(jmpinst, translation_ptr);                \
}

#define generate_translation_gate(type)                                       \
  generate_load_pc(a0, pc);                                                   \
  generate_indirect_branch_no_cycle_update(type)                              \

extern void* x86_table_data[9][16];
extern void* x86_table_info[9][16];

void init_emitter(bool must_swap) {
  memcpy(x86_table_info, x86_table_data, sizeof(x86_table_data));

  rom_cache_watermark = INITIAL_ROM_WATERMARK;
  init_bios_hooks();
}

u32 function_cc execute_arm_translate_internal(u32 cycles, void *regptr);

u32 execute_arm_translate(u32 cycles) {
  return execute_arm_translate_internal(cycles, &reg[0]);
}

#endif
