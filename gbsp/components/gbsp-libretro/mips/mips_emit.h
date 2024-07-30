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

#ifndef MIPS_EMIT_H
#define MIPS_EMIT_H

#include "mips/mips_codegen.h"

// Pointers to default handlers.
// Use IWRAM as default, assume aligned by default too
#define execute_load_u8   tmemld[0][3]
#define execute_load_s8   tmemld[1][3]
#define execute_load_u16  tmemld[2][3]
#define execute_load_s16  tmemld[4][3]
#define execute_load_u32  tmemld[6][3]
#define execute_aligned_load32 tmemld[10][3]
#define execute_store_u8  tmemst[0][3]
#define execute_store_u16 tmemst[1][3]
#define execute_store_u32 tmemst[2][3]
#define execute_aligned_store32 tmemst[3][3]

u32 mips_update_gba(u32 pc);

// Although these are defined as a function, don't call them as
// such (jump to it instead)
void mips_indirect_branch_arm(u32 address);
void mips_indirect_branch_thumb(u32 address);
void mips_indirect_branch_dual(u32 address);

u32 execute_read_cpsr();
u32 execute_read_spsr();
void execute_swi(u32 pc);
void mips_cheat_hook(void);

u32 execute_spsr_restore(u32 address);
void execute_store_cpsr(u32 new_cpsr, u32 store_mask);
void execute_store_spsr(u32 new_spsr, u32 store_mask);


#define reg_base    mips_reg_s0
#define reg_cycles  mips_reg_s1
#define reg_a0      mips_reg_a0
#define reg_a1      mips_reg_a1
#define reg_a2      mips_reg_a2
#define reg_rv      mips_reg_v0
#define reg_pc      mips_reg_s3
#define reg_temp    mips_reg_at
#define reg_zero    mips_reg_zero

#define reg_n_cache mips_reg_s4
#define reg_z_cache mips_reg_s5
#define reg_c_cache mips_reg_s6
#define reg_v_cache mips_reg_s7

#define reg_r0      mips_reg_v1
#define reg_r1      mips_reg_a3
#define reg_r2      mips_reg_t0
#define reg_r3      mips_reg_t1
#define reg_r4      mips_reg_t2
#define reg_r5      mips_reg_t3
#define reg_r6      mips_reg_t4
#define reg_r7      mips_reg_t5
#define reg_r8      mips_reg_t6
#define reg_r9      mips_reg_t7
#define reg_r10     mips_reg_s2
#define reg_r11     mips_reg_t8
#define reg_r12     mips_reg_t9
#define reg_r13     mips_reg_gp
#define reg_r14     mips_reg_fp

// Writing to r15 goes straight to a0, to be chained with other ops

u32 arm_to_mips_reg[] =
{
  reg_r0,
  reg_r1,
  reg_r2,
  reg_r3,
  reg_r4,
  reg_r5,
  reg_r6,
  reg_r7,
  reg_r8,
  reg_r9,
  reg_r10,
  reg_r11,
  reg_r12,
  reg_r13,
  reg_r14,
  reg_a0,
  reg_a1,
  reg_a2
};

#define arm_reg_a0   15
#define arm_reg_a1   16
#define arm_reg_a2   17

#define generate_load_reg(ireg, reg_index)                                    \
  mips_emit_addu(ireg, arm_to_mips_reg[reg_index], reg_zero)                  \

#define generate_load_imm(ireg, imm)                                          \
  if(((s32)imm >= -32768) && ((s32)imm <= 32767)) {                           \
    mips_emit_addiu(ireg, reg_zero, imm);                                     \
  } else if(((u32)imm >> 16) == 0x0000) {                                     \
    mips_emit_ori(ireg, reg_zero, imm);                                       \
  } else {                                                                    \
    mips_emit_lui(ireg, imm >> 16);                                           \
    if (((u32)(imm) & 0x0000FFFF)) {                                          \
      mips_emit_ori(ireg, ireg, (imm) & 0xFFFF);                              \
    }                                                                         \
  }                                                                           \

#define generate_load_pc(ireg, new_pc)                                        \
{                                                                             \
  s32 pc_delta = (new_pc) - (stored_pc);                                      \
  if((pc_delta >= -32768) && (pc_delta <= 32767)) {                           \
    mips_emit_addiu(ireg, reg_pc, pc_delta);                                  \
  } else {                                                                    \
    generate_load_imm(ireg, (new_pc));                                        \
  }                                                                           \
}                                                                             \

#define generate_store_reg(ireg, reg_index)                                   \
  mips_emit_addu(arm_to_mips_reg[reg_index], ireg, reg_zero)                  \

#define generate_alu_imm(imm_type, reg_type, ireg_dest, ireg_src, imm)        \
  if(((s32)imm >= -32768) && ((s32)imm <= 32767))                             \
  {                                                                           \
    mips_emit_##imm_type(ireg_dest, ireg_src, imm);                           \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_imm(reg_temp, imm);                                         \
    mips_emit_##reg_type(ireg_dest, ireg_src, reg_temp);                      \
  }                                                                           \

#define generate_alu_immu(imm_type, reg_type, ireg_dest, ireg_src, imm)       \
  if(((u32)imm >= 0) && ((u32)imm <= 65535))                                  \
  {                                                                           \
    mips_emit_##imm_type(ireg_dest, ireg_src, imm);                           \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_imm(reg_temp, imm);                                         \
    mips_emit_##reg_type(ireg_dest, ireg_src, reg_temp);                      \
  }                                                                           \

#define generate_mov(ireg_dest, ireg_src)                                     \
  mips_emit_addu(ireg_dest, ireg_src, reg_zero)                               \

#define generate_multiply_s64()                                               \
  mips_emit_mult(arm_to_mips_reg[rm], arm_to_mips_reg[rs])                    \

#define generate_multiply_u64()                                               \
  mips_emit_multu(arm_to_mips_reg[rm], arm_to_mips_reg[rs])                   \

#define generate_multiply_s64_add()                                           \
  mips_emit_madd(arm_to_mips_reg[rm], arm_to_mips_reg[rs])                    \

#define generate_multiply_u64_add()                                           \
  mips_emit_maddu(arm_to_mips_reg[rm], arm_to_mips_reg[rs])                   \

#define generate_function_call(function_location)                             \
  mips_emit_jal(mips_absolute_offset(function_location));                     \
  mips_emit_nop()                                                             \

#define generate_raw_u32(value)                                               \
  *((u32 *)translation_ptr) = (value);                                        \
  translation_ptr += 4                                                        \

#define generate_function_call_swap_delay(function_location)                  \
{                                                                             \
  u32 delay_instruction = address32(translation_ptr, -4);                     \
  translation_ptr -= 4;                                                       \
  mips_emit_jal(mips_absolute_offset(function_location));                     \
  address32(translation_ptr, 0) = delay_instruction;                          \
  translation_ptr += 4;                                                       \
}                                                                             \

#define generate_function_return_swap_delay()                                 \
{                                                                             \
  u32 delay_instruction = address32(translation_ptr, -4);                     \
  translation_ptr -= 4;                                                       \
  mips_emit_jr(mips_reg_ra);                                                  \
  address32(translation_ptr, 0) = delay_instruction;                          \
  translation_ptr += 4;                                                       \
}                                                                             \

#define generate_swap_delay()                                                 \
{                                                                             \
  u32 delay_instruction = address32(translation_ptr, -8);                     \
  u32 branch_instruction = address32(translation_ptr, -4);                    \
  branch_instruction = (branch_instruction & 0xFFFF0000) |                    \
   (((branch_instruction & 0x0000FFFF) + 1) & 0x0000FFFF);                    \
  address32(translation_ptr, -8) = branch_instruction;                        \
  address32(translation_ptr, -4) = delay_instruction;                         \
}                                                                             \

#define generate_cycle_update()                                               \
  if(cycle_count != 0)                                                        \
  {                                                                           \
    mips_emit_addiu(reg_cycles, reg_cycles, -cycle_count);                    \
    cycle_count = 0;                                                          \
  }                                                                           \

#define generate_cycle_update_force()                                         \
  mips_emit_addiu(reg_cycles, reg_cycles, -cycle_count);                      \
  cycle_count = 0                                                             \

#define generate_branch_patch_conditional(dest, offset)                       \
  *((u16 *)(dest)) = mips_relative_offset(dest, offset)                       \

#define generate_branch_patch_unconditional(dest, offset)                     \
  *((u32 *)(dest)) = (mips_opcode_j << 26) |                                  \
   ((mips_absolute_offset(offset)) & 0x3FFFFFF)                               \

#define generate_branch_no_cycle_update(writeback_location, new_pc)           \
  if(pc == idle_loop_target_pc)                                               \
  {                                                                           \
    generate_load_pc(reg_a0, new_pc);                                         \
    mips_emit_lui(reg_cycles, 0);                                             \
    generate_function_call_swap_delay(mips_update_gba);                       \
    mips_emit_j_filler(writeback_location);                                   \
    mips_emit_nop();                                                          \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_pc(reg_a0, new_pc);                                         \
    mips_emit_bltzal(reg_cycles,                                              \
     mips_relative_offset(translation_ptr, update_trampoline));               \
    generate_swap_delay();                                                    \
    mips_emit_j_filler(writeback_location);                                   \
    mips_emit_nop();                                                          \
  }                                                                           \

#define generate_branch_cycle_update(writeback_location, new_pc)              \
  generate_cycle_update();                                                    \
  generate_branch_no_cycle_update(writeback_location, new_pc)                 \

// a0 holds the destination

#define generate_indirect_branch_cycle_update(type)                           \
  mips_emit_j(mips_absolute_offset(mips_indirect_branch_##type));             \
  generate_cycle_update_force()                                               \

#define generate_indirect_branch_no_cycle_update(type)                        \
  mips_emit_j(mips_absolute_offset(mips_indirect_branch_##type));             \
  mips_emit_nop()                                                             \

#define block_prologue_size   16

#define generate_block_prologue()                                             \
  update_trampoline = translation_ptr;                                        \
  mips_emit_j(mips_absolute_offset(mips_update_gba));                         \
  mips_emit_nop();                                                            \
  spaccess_trampoline = translation_ptr;                                      \
  mips_emit_j(mips_absolute_offset(&rom_translation_cache[EWRAM_SPM_OFF]));   \
  mips_emit_nop();                                                            \
  generate_load_imm(reg_pc, stored_pc)                                        \

#define check_generate_n_flag (flag_status & 0x08)
#define check_generate_z_flag (flag_status & 0x04)
#define check_generate_c_flag (flag_status & 0x02)
#define check_generate_v_flag (flag_status & 0x01)

#define generate_load_reg_pc(ireg, reg_index, pc_offset)                      \
  if(reg_index == REG_PC)                                                     \
  {                                                                           \
    generate_load_pc(ireg, (pc + pc_offset));                                 \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_reg(ireg, reg_index);                                       \
  }                                                                           \

#define check_load_reg_pc(arm_reg, reg_index, pc_offset)                      \
  if(reg_index == REG_PC)                                                     \
  {                                                                           \
    reg_index = arm_reg;                                                      \
    generate_load_pc(arm_to_mips_reg[arm_reg], (pc + pc_offset));             \
  }                                                                           \

#define check_store_reg_pc_no_flags(reg_index)                                \
  if(reg_index == REG_PC)                                                     \
  {                                                                           \
    generate_indirect_branch_arm();                                           \
  }                                                                           \

#define check_store_reg_pc_flags(reg_index)                                   \
  if(reg_index == REG_PC)                                                     \
  {                                                                           \
    generate_function_call(execute_spsr_restore);                             \
    generate_indirect_branch_dual();                                          \
  }                                                                           \

#define generate_shift_imm_lsl_no_flags(arm_reg, _rm, _shift)                 \
  check_load_reg_pc(arm_reg, _rm, 8);                                         \
  if(_shift != 0)                                                             \
  {                                                                           \
    mips_emit_sll(arm_to_mips_reg[arm_reg], arm_to_mips_reg[_rm], _shift);    \
    _rm = arm_reg;                                                            \
  }                                                                           \

#define generate_shift_imm_lsr_no_flags(arm_reg, _rm, _shift)                 \
  if(_shift != 0)                                                             \
  {                                                                           \
    check_load_reg_pc(arm_reg, _rm, 8);                                       \
    mips_emit_srl(arm_to_mips_reg[arm_reg], arm_to_mips_reg[_rm], _shift);    \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    mips_emit_addu(arm_to_mips_reg[arm_reg], reg_zero, reg_zero);             \
  }                                                                           \
  _rm = arm_reg                                                               \

#define generate_shift_imm_asr_no_flags(arm_reg, _rm, _shift)                 \
  check_load_reg_pc(arm_reg, _rm, 8);                                         \
  if(_shift != 0)                                                             \
  {                                                                           \
    mips_emit_sra(arm_to_mips_reg[arm_reg], arm_to_mips_reg[_rm], _shift);    \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    mips_emit_sra(arm_to_mips_reg[arm_reg], arm_to_mips_reg[_rm], 31);        \
  }                                                                           \
  _rm = arm_reg                                                               \

#define generate_shift_imm_ror_no_flags(arm_reg, _rm, _shift)                 \
  check_load_reg_pc(arm_reg, _rm, 8);                                         \
  if(_shift != 0)                                                             \
  {                                                                           \
    rotate_right(arm_to_mips_reg[arm_reg], arm_to_mips_reg[_rm],              \
                 reg_temp, _shift);                                           \
  }                                                                           \
  else                                                                        \
  { /* Special case: RRX (no carry update) */                                 \
    mips_emit_srl(arm_to_mips_reg[arm_reg], arm_to_mips_reg[_rm], 1);         \
    insert_bits(arm_to_mips_reg[arm_reg], reg_c_cache, reg_temp, 31, 1);      \
  }                                                                           \
  _rm = arm_reg                                                               \

#define generate_shift_imm_lsl_flags(arm_reg, _rm, _shift)                    \
  check_load_reg_pc(arm_reg, _rm, 8);                                         \
  if(_shift != 0)                                                             \
  {                                                                           \
    extract_bits(reg_c_cache, arm_to_mips_reg[_rm], (32 - _shift), 1);        \
    mips_emit_sll(arm_to_mips_reg[arm_reg], arm_to_mips_reg[_rm], _shift);    \
    _rm = arm_reg;                                                            \
  }                                                                           \

#define generate_shift_imm_lsr_flags(arm_reg, _rm, _shift)                    \
  check_load_reg_pc(arm_reg, _rm, 8);                                         \
  if(_shift != 0)                                                             \
  {                                                                           \
    extract_bits(reg_c_cache, arm_to_mips_reg[_rm], (_shift - 1), 1);         \
    mips_emit_srl(arm_to_mips_reg[arm_reg], arm_to_mips_reg[_rm], _shift);    \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    mips_emit_srl(reg_c_cache, arm_to_mips_reg[_rm], 31);                     \
    mips_emit_addu(arm_to_mips_reg[arm_reg], reg_zero, reg_zero);             \
  }                                                                           \
  _rm = arm_reg                                                               \

#define generate_shift_imm_asr_flags(arm_reg, _rm, _shift)                    \
  check_load_reg_pc(arm_reg, _rm, 8);                                         \
  if(_shift != 0)                                                             \
  {                                                                           \
    extract_bits(reg_c_cache, arm_to_mips_reg[_rm], (_shift - 1), 1);         \
    mips_emit_sra(arm_to_mips_reg[arm_reg], arm_to_mips_reg[_rm], _shift);    \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    mips_emit_sra(arm_to_mips_reg[arm_reg], arm_to_mips_reg[_rm], 31);        \
    mips_emit_andi(reg_c_cache, arm_to_mips_reg[arm_reg], 1);                 \
  }                                                                           \
  _rm = arm_reg                                                               \

#define generate_shift_imm_ror_flags(arm_reg, _rm, _shift)                    \
  check_load_reg_pc(arm_reg, _rm, 8);                                         \
  if(_shift != 0)                                                             \
  {                                                                           \
    extract_bits(reg_c_cache, arm_to_mips_reg[_rm], (_shift - 1), 1);         \
    rotate_right(arm_to_mips_reg[arm_reg], arm_to_mips_reg[_rm],              \
                 reg_temp, _shift);                                           \
  }                                                                           \
  else                                                                        \
  { /* Special case: RRX (carry update) */                                    \
    mips_emit_sll(reg_temp, reg_c_cache, 31);                                 \
    mips_emit_andi(reg_c_cache, arm_to_mips_reg[_rm], 1);                     \
    mips_emit_srl(arm_to_mips_reg[arm_reg], arm_to_mips_reg[_rm], 1);         \
    mips_emit_or(arm_to_mips_reg[arm_reg], arm_to_mips_reg[arm_reg],reg_temp);\
  }                                                                           \
  _rm = arm_reg                                                               \

#define generate_shift_reg_lsl_no_flags(_rm, _rs)                             \
  mips_emit_sltiu(reg_temp, arm_to_mips_reg[_rs], 32);                        \
  mips_emit_sllv(reg_a0, arm_to_mips_reg[_rm], arm_to_mips_reg[_rs]);         \
  mips_emit_movz(reg_a0, reg_zero, reg_temp)                                  \

#define generate_shift_reg_lsr_no_flags(_rm, _rs)                             \
  mips_emit_sltiu(reg_temp, arm_to_mips_reg[_rs], 32);                        \
  mips_emit_srlv(reg_a0, arm_to_mips_reg[_rm], arm_to_mips_reg[_rs]);         \
  mips_emit_movz(reg_a0, reg_zero, reg_temp)                                  \

#define generate_shift_reg_asr_no_flags(_rm, _rs)                             \
  mips_emit_sltiu(reg_temp, arm_to_mips_reg[_rs], 32);                        \
  mips_emit_b(bne, reg_temp, reg_zero, 2);                                    \
  mips_emit_srav(reg_a0, arm_to_mips_reg[_rm], arm_to_mips_reg[_rs]);         \
  mips_emit_sra(reg_a0, reg_a0, 31)                                           \

#define generate_shift_reg_ror_no_flags(_rm, _rs)                             \
  rotate_right_var(reg_a0, arm_to_mips_reg[_rm],                              \
                   reg_temp, arm_to_mips_reg[_rs])                            \

#define generate_shift_reg_lsl_flags(_rm, _rs)                                \
{                                                                             \
  u32 shift_reg = _rs;                                                        \
  check_load_reg_pc(arm_reg_a1, shift_reg, 8);                                \
  generate_load_reg_pc(reg_a0, _rm, 12);                                      \
  /* Only load the result on zero, no shift */                                \
  mips_emit_b(beq, arm_to_mips_reg[shift_reg], reg_zero, 7);                  \
  generate_swap_delay();                                                      \
  mips_emit_addiu(reg_temp, arm_to_mips_reg[shift_reg], -1);                  \
  mips_emit_sllv(reg_a0, reg_a0, reg_temp);                                   \
  mips_emit_srl(reg_c_cache, reg_a0, 31);                                     \
  mips_emit_sltiu(reg_temp, arm_to_mips_reg[shift_reg], 33);                  \
  mips_emit_sll(reg_a0, reg_a0, 1);                                           \
  /* Result and flag to be zero if shift is 33+ */                            \
  mips_emit_movz(reg_c_cache, reg_zero, reg_temp);                            \
  mips_emit_movz(reg_a0, reg_zero, reg_temp);                                 \
}                                                                             \

#define generate_shift_reg_lsr_flags(_rm, _rs)                                \
{                                                                             \
  u32 shift_reg = _rs;                                                        \
  check_load_reg_pc(arm_reg_a1, shift_reg, 8);                                \
  generate_load_reg_pc(reg_a0, _rm, 12);                                      \
  /* Only load the result on zero, no shift */                                \
  mips_emit_b(beq, arm_to_mips_reg[shift_reg], reg_zero, 7);                  \
  generate_swap_delay();                                                      \
  mips_emit_addiu(reg_temp, arm_to_mips_reg[shift_reg], -1);                  \
  mips_emit_srlv(reg_a0, reg_a0, reg_temp);                                   \
  mips_emit_andi(reg_c_cache, reg_a0, 1);                                     \
  mips_emit_sltiu(reg_temp, arm_to_mips_reg[shift_reg], 33);                  \
  mips_emit_srl(reg_a0, reg_a0, 1);                                           \
  /* Result and flag to be zero if shift is 33+ */                            \
  mips_emit_movz(reg_c_cache, reg_zero, reg_temp);                            \
  mips_emit_movz(reg_a0, reg_zero, reg_temp);                                 \
}                                                                             \

#define generate_shift_reg_asr_flags(_rm, _rs)                                \
  generate_load_reg_pc(reg_a1, _rs, 8);                                       \
  generate_load_reg_pc(reg_a0, _rm, 12);                                      \
  /* Only load the result on zero, no shift */                                \
  mips_emit_b(beq, reg_a1, reg_zero, 7);                                      \
  generate_swap_delay();                                                      \
  /* Cap shift at 32, since it's equivalent */                                \
  mips_emit_addiu(reg_temp, reg_zero, 32);                                    \
  mips_emit_srl(reg_rv, reg_a1, 5);                                           \
  mips_emit_movn(reg_a1, reg_temp, reg_rv);                                   \
  mips_emit_addiu(reg_temp, reg_a1, -1);                                      \
  mips_emit_srav(reg_a0, reg_a0, reg_temp);                                   \
  mips_emit_andi(reg_c_cache, reg_a0, 1);                                     \
  mips_emit_sra(reg_a0, reg_a0, 1);                                           \

#define generate_shift_reg_ror_flags(_rm, _rs)                                \
  mips_emit_b(beq, arm_to_mips_reg[_rs], reg_zero, 3);                        \
  mips_emit_addiu(reg_temp, arm_to_mips_reg[_rs], -1);                        \
  mips_emit_srlv(reg_temp, arm_to_mips_reg[_rm], reg_temp);                   \
  mips_emit_andi(reg_c_cache, reg_temp, 1);                                   \
  rotate_right_var(reg_a0, arm_to_mips_reg[_rm],                              \
                   reg_temp, arm_to_mips_reg[_rs])                            \

#define generate_shift_imm(arm_reg, name, flags_op)                           \
  u32 shift = (opcode >> 7) & 0x1F;                                           \
  generate_shift_imm_##name##_##flags_op(arm_reg, rm, shift)                  \

#define generate_shift_reg(arm_reg, name, flags_op)                           \
  u32 rs = ((opcode >> 8) & 0x0F);                                            \
  generate_shift_reg_##name##_##flags_op(rm, rs);                             \
  rm = arm_reg                                                                \

// Made functions due to the macro expansion getting too large.
// Returns a new rm if it redirects it (which will happen on most of these
// cases)

#define generate_load_rm_sh(rm, flags_op)                                     \
{                                                                             \
  switch((opcode >> 4) & 0x07)                                                \
  {                                                                           \
    case 0x0: /* LSL imm */                                                   \
    {                                                                         \
      generate_shift_imm(arm_reg_a0, lsl, flags_op);                          \
      break;                                                                  \
    }                                                                         \
    case 0x1: /* LSL reg */                                                   \
    {                                                                         \
      generate_shift_reg(arm_reg_a0, lsl, flags_op);                          \
      break;                                                                  \
    }                                                                         \
    case 0x2: /* LSR imm */                                                   \
    {                                                                         \
      generate_shift_imm(arm_reg_a0, lsr, flags_op);                          \
      break;                                                                  \
    }                                                                         \
    case 0x3: /* LSR reg */                                                   \
    {                                                                         \
      generate_shift_reg(arm_reg_a0, lsr, flags_op);                          \
      break;                                                                  \
    }                                                                         \
    case 0x4: /* ASR imm */                                                   \
    {                                                                         \
      generate_shift_imm(arm_reg_a0, asr, flags_op);                          \
      break;                                                                  \
    }                                                                         \
    case 0x5: /* ASR reg */                                                   \
    {                                                                         \
      generate_shift_reg(arm_reg_a0, asr, flags_op);                          \
      break;                                                                  \
    }                                                                         \
    case 0x6: /* ROR imm */                                                   \
    {                                                                         \
      generate_shift_imm(arm_reg_a0, ror, flags_op);                          \
      break;                                                                  \
    }                                                                         \
    case 0x7: /* ROR reg */                                                   \
    {                                                                         \
      generate_shift_reg(arm_reg_a0, ror, flags_op);                          \
      break;                                                                  \
    }                                                                         \
  }                                                                           \
}                                                                             \

#define generate_block_extra_vars()                                           \
  u32 stored_pc = pc;                                                         \
  u8 *update_trampoline;                                                      \
  u8 *spaccess_trampoline;                                                    \

#define generate_block_extra_vars_arm()                                       \
  generate_block_extra_vars();                                                \

#define generate_load_offset_sh(rm)                                           \
  {                                                                           \
    switch((opcode >> 5) & 0x03)                                              \
    {                                                                         \
      case 0x0: /* LSL imm */                                                 \
      {                                                                       \
        generate_shift_imm(arm_reg_a1, lsl, no_flags);                        \
        break;                                                                \
      }                                                                       \
      case 0x1: /* LSR imm */                                                 \
      {                                                                       \
        generate_shift_imm(arm_reg_a1, lsr, no_flags);                        \
        break;                                                                \
      }                                                                       \
      case 0x2: /* ASR imm */                                                 \
      {                                                                       \
        generate_shift_imm(arm_reg_a1, asr, no_flags);                        \
        break;                                                                \
      }                                                                       \
      case 0x3: /* ROR imm */                                                 \
      {                                                                       \
        generate_shift_imm(arm_reg_a1, ror, no_flags);                        \
        break;                                                                \
      }                                                                       \
    }                                                                         \
  }                                                                           \

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

#define generate_block_extra_vars_thumb()                                     \
  generate_block_extra_vars()                                                 \

// It should be okay to still generate result flags, spsr will overwrite them.
// This is pretty infrequent (returning from interrupt handlers, et al) so
// probably not worth optimizing for.

u32 execute_spsr_restore_body(u32 address)
{
  set_cpu_mode(cpu_modes[reg[REG_CPSR] & 0xF]);
  if((io_registers[REG_IE] & io_registers[REG_IF]) &&
   io_registers[REG_IME] && ((reg[REG_CPSR] & 0x80) == 0))
  {
    REG_MODE(MODE_IRQ)[6] = address + 4;
    REG_SPSR(MODE_IRQ) = reg[REG_CPSR];
    reg[REG_CPSR] = 0xD2;
    address = 0x00000018;
    set_cpu_mode(MODE_IRQ);
  }

  if(reg[REG_CPSR] & 0x20)
    address |= 0x01;

  return address;
}


// These generate a branch on the opposite condition on purpose.
// For ARM mode we aim to skip instructions (therefore opposite)
// In Thumb mode we skip the conditional branch in a similar way

#define generate_condition_eq()                                               \
  mips_emit_b_filler(beq, reg_z_cache, reg_zero, backpatch_address);          \
  generate_cycle_update_force()                                               \

#define generate_condition_ne()                                               \
  mips_emit_b_filler(bne, reg_z_cache, reg_zero, backpatch_address);          \
  generate_cycle_update_force()                                               \

#define generate_condition_cs()                                               \
  mips_emit_b_filler(beq, reg_c_cache, reg_zero, backpatch_address);          \
  generate_cycle_update_force()                                               \

#define generate_condition_cc()                                               \
  mips_emit_b_filler(bne, reg_c_cache, reg_zero, backpatch_address);          \
  generate_cycle_update_force()                                               \

#define generate_condition_mi()                                               \
  mips_emit_b_filler(beq, reg_n_cache, reg_zero, backpatch_address);          \
  generate_cycle_update_force()                                               \

#define generate_condition_pl()                                               \
  mips_emit_b_filler(bne, reg_n_cache, reg_zero, backpatch_address);          \
  generate_cycle_update_force()                                               \

#define generate_condition_vs()                                               \
  mips_emit_b_filler(beq, reg_v_cache, reg_zero, backpatch_address);          \
  generate_cycle_update_force()                                               \

#define generate_condition_vc()                                               \
  mips_emit_b_filler(bne, reg_v_cache, reg_zero, backpatch_address);          \
  generate_cycle_update_force()                                               \

#define generate_condition_hi()                                               \
  mips_emit_xori(reg_temp, reg_c_cache, 1);                                   \
  mips_emit_or(reg_temp, reg_temp, reg_z_cache);                              \
  mips_emit_b_filler(bne, reg_temp, reg_zero, backpatch_address);             \
  generate_cycle_update_force()                                               \

#define generate_condition_ls()                                               \
  mips_emit_xori(reg_temp, reg_c_cache, 1);                                   \
  mips_emit_or(reg_temp, reg_temp, reg_z_cache);                              \
  mips_emit_b_filler(beq, reg_temp, reg_zero, backpatch_address);             \
  generate_cycle_update_force()                                               \

#define generate_condition_ge()                                               \
  mips_emit_b_filler(bne, reg_n_cache, reg_v_cache, backpatch_address);       \
  generate_cycle_update_force()                                               \

#define generate_condition_lt()                                               \
  mips_emit_b_filler(beq, reg_n_cache, reg_v_cache, backpatch_address);       \
  generate_cycle_update_force()                                               \

#define generate_condition_gt()                                               \
  mips_emit_xor(reg_temp, reg_n_cache, reg_v_cache);                          \
  mips_emit_or(reg_temp, reg_temp, reg_z_cache);                              \
  mips_emit_b_filler(bne, reg_temp, reg_zero, backpatch_address);             \
  generate_cycle_update_force()                                               \

#define generate_condition_le()                                               \
  mips_emit_xor(reg_temp, reg_n_cache, reg_v_cache);                          \
  mips_emit_or(reg_temp, reg_temp, reg_z_cache);                              \
  mips_emit_b_filler(beq, reg_temp, reg_zero, backpatch_address);             \
  generate_cycle_update_force()                                               \

#define generate_condition()                                                  \
  switch(condition)                                                           \
  {                                                                           \
    case 0x0: generate_condition_eq(); break;                                 \
    case 0x1: generate_condition_ne(); break;                                 \
    case 0x2: generate_condition_cs(); break;                                 \
    case 0x3: generate_condition_cc(); break;                                 \
    case 0x4: generate_condition_mi(); break;                                 \
    case 0x5: generate_condition_pl(); break;                                 \
    case 0x6: generate_condition_vs(); break;                                 \
    case 0x7: generate_condition_vc(); break;                                 \
    case 0x8: generate_condition_hi(); break;                                 \
    case 0x9: generate_condition_ls(); break;                                 \
    case 0xA: generate_condition_ge(); break;                                 \
    case 0xB: generate_condition_lt(); break;                                 \
    case 0xC: generate_condition_gt(); break;                                 \
    case 0xD: generate_condition_le(); break;                                 \
    default:                                                                  \
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

#define generate_op_and_reg(_rd, _rn, _rm)                                    \
  mips_emit_and(_rd, _rn, _rm)                                                \

#define generate_op_orr_reg(_rd, _rn, _rm)                                    \
  mips_emit_or(_rd, _rn, _rm)                                                 \

#define generate_op_eor_reg(_rd, _rn, _rm)                                    \
  mips_emit_xor(_rd, _rn, _rm)                                                \

#define generate_op_bic_reg(_rd, _rn, _rm)                                    \
  mips_emit_nor(reg_temp, _rm, reg_zero);                                     \
  mips_emit_and(_rd, _rn, reg_temp)                                           \

#define generate_op_sub_reg(_rd, _rn, _rm)                                    \
  mips_emit_subu(_rd, _rn, _rm)                                               \

#define generate_op_rsb_reg(_rd, _rn, _rm)                                    \
  mips_emit_subu(_rd, _rm, _rn)                                               \

#define generate_op_sbc_reg(_rd, _rn, _rm)                                    \
  mips_emit_subu(_rd, _rn, _rm);                                              \
  mips_emit_xori(reg_temp, reg_c_cache, 1);                                   \
  mips_emit_subu(_rd, _rd, reg_temp)                                          \

#define generate_op_rsc_reg(_rd, _rn, _rm)                                    \
  mips_emit_addu(reg_temp, _rm, reg_c_cache);                                 \
  mips_emit_addiu(reg_temp, reg_temp, -1);                                    \
  mips_emit_subu(_rd, reg_temp, _rn)                                          \

#define generate_op_add_reg(_rd, _rn, _rm)                                    \
  mips_emit_addu(_rd, _rn, _rm)                                               \

#define generate_op_adc_reg(_rd, _rn, _rm)                                    \
  mips_emit_addu(reg_temp, _rm, reg_c_cache);                                 \
  mips_emit_addu(_rd, _rn, reg_temp)                                          \

#define generate_op_mov_reg(_rd, _rn, _rm)                                    \
  mips_emit_addu(_rd, _rm, reg_zero)                                          \

#define generate_op_mvn_reg(_rd, _rn, _rm)                                    \
  mips_emit_nor(_rd, _rm, reg_zero)                                           \

#define generate_op_imm_wrapper(name, _rd, _rn)                               \
  if(imm != 0)                                                                \
  {                                                                           \
    generate_load_imm(reg_a0, imm);                                           \
    generate_op_##name##_reg(_rd, _rn, reg_a0);                               \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_op_##name##_reg(_rd, _rn, reg_zero);                             \
  }                                                                           \

#define generate_op_and_imm(_rd, _rn)                                         \
  generate_alu_immu(andi, and, _rd, _rn, imm)                                 \

#define generate_op_orr_imm(_rd, _rn)                                         \
  generate_alu_immu(ori, or, _rd, _rn, imm)                                   \

#define generate_op_eor_imm(_rd, _rn)                                         \
  generate_alu_immu(xori, xor, _rd, _rn, imm)                                 \

#define generate_op_bic_imm(_rd, _rn)                                         \
  generate_alu_immu(andi, and, _rd, _rn, (~imm))                              \

#define generate_op_sub_imm(_rd, _rn)                                         \
  generate_alu_imm(addiu, addu, _rd, _rn, (-imm))                             \

#define generate_op_rsb_imm(_rd, _rn)                                         \
  if(imm != 0)                                                                \
  {                                                                           \
    generate_load_imm(reg_temp, imm);                                         \
    mips_emit_subu(_rd, reg_temp, _rn);                                       \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    mips_emit_subu(_rd, reg_zero, _rn);                                       \
  }                                                                           \

#define generate_op_sbc_imm(_rd, _rn)                                         \
  generate_op_imm_wrapper(sbc, _rd, _rn)                                      \

#define generate_op_rsc_imm(_rd, _rn)                                         \
  generate_op_imm_wrapper(rsc, _rd, _rn)                                      \

#define generate_op_add_imm(_rd, _rn)                                         \
  generate_alu_imm(addiu, addu, _rd, _rn, imm)                                \

#define generate_op_adc_imm(_rd, _rn)                                         \
  generate_op_imm_wrapper(adc, _rd, _rn)                                      \

#define generate_op_mov_imm(_rd, _rn)                                         \
  generate_load_imm(_rd, imm)                                                 \

#define generate_op_mvn_imm(_rd, _rn)                                         \
  generate_load_imm(_rd, (~imm))                                              \

#define generate_op_logic_flags(_rd)                                          \
  if(check_generate_n_flag)                                                   \
  {                                                                           \
    mips_emit_srl(reg_n_cache, _rd, 31);                                      \
  }                                                                           \
  if(check_generate_z_flag)                                                   \
  {                                                                           \
    mips_emit_sltiu(reg_z_cache, _rd, 1);                                     \
  }                                                                           \

#define generate_op_ands_reg(_rd, _rn, _rm)                                   \
  mips_emit_and(_rd, _rn, _rm);                                               \
  generate_op_logic_flags(_rd)                                                \

#define generate_op_orrs_reg(_rd, _rn, _rm)                                   \
  mips_emit_or(_rd, _rn, _rm);                                                \
  generate_op_logic_flags(_rd)                                                \

#define generate_op_eors_reg(_rd, _rn, _rm)                                   \
  mips_emit_xor(_rd, _rn, _rm);                                               \
  generate_op_logic_flags(_rd)                                                \

#define generate_op_bics_reg(_rd, _rn, _rm)                                   \
  mips_emit_nor(reg_temp, _rm, reg_zero);                                     \
  mips_emit_and(_rd, _rn, reg_temp);                                          \
  generate_op_logic_flags(_rd)                                                \

#define generate_op_subs_reg(_rd, _rn, _rm)                                   \
  if(check_generate_c_flag)                                                   \
  {                                                                           \
    mips_emit_sltu(reg_c_cache, _rn, _rm);                                    \
    mips_emit_xori(reg_c_cache, reg_c_cache, 1);                              \
  }                                                                           \
  if(check_generate_v_flag)                                                   \
  {                                                                           \
    mips_emit_slt(reg_v_cache, _rn, _rm);                                     \
  }                                                                           \
  mips_emit_subu(_rd, _rn, _rm);                                              \
  generate_op_logic_flags(_rd);                                               \
  if(check_generate_v_flag)                                                   \
  {                                                                           \
    if(!check_generate_n_flag)                                                \
    {                                                                         \
      mips_emit_srl(reg_n_cache, _rd, 31);                                    \
    }                                                                         \
    mips_emit_xor(reg_v_cache, reg_v_cache, reg_n_cache);                     \
  }                                                                           \

#define generate_op_rsbs_reg(_rd, _rn, _rm)                                   \
  generate_op_subs_reg(_rd, _rm, _rn)

#define generate_op_sbcs_reg(_rd, _rn, _rm)                                   \
  mips_emit_xori(reg_temp, reg_c_cache, 1);                                   \
  if(check_generate_c_flag)                                                   \
  {                                                                           \
    mips_emit_sltu(reg_c_cache, _rm, _rn);                                    \
    mips_emit_sltu(reg_rv, _rn, _rm);                                         \
    mips_emit_xori(reg_rv, reg_rv, 1);                                        \
    mips_emit_movz(reg_c_cache, reg_rv, reg_temp);                            \
  }                                                                           \
  if(check_generate_v_flag)                                                   \
  {                                                                           \
    mips_emit_xor(reg_v_cache, _rn, _rm);                                     \
    mips_emit_nor(reg_rv, _rm, reg_zero);                                     \
  }                                                                           \
  mips_emit_subu(_rd, _rn, _rm);                                              \
  mips_emit_subu(_rd, _rd, reg_temp);                                         \
  if(check_generate_v_flag)                                                   \
  {                                                                           \
    mips_emit_xor(reg_rv, reg_rv, _rd);                                       \
    mips_emit_and(reg_v_cache, reg_v_cache, reg_rv);                          \
    mips_emit_srl(reg_v_cache, reg_v_cache, 31);                              \
  }                                                                           \
  generate_op_logic_flags(_rd);                                               \

#define generate_op_rscs_reg(_rd, _rn, _rm)                                   \
  generate_op_sbcs_reg(_rd, _rm, _rn)

#define generate_op_adds_reg(_rd, _rn, _rm)                                   \
  if(check_generate_c_flag | check_generate_v_flag)                           \
  {                                                                           \
    mips_emit_addu(reg_c_cache, _rn, reg_zero);                               \
  }                                                                           \
  if(check_generate_v_flag)                                                   \
  {                                                                           \
    mips_emit_slt(reg_v_cache, _rm, reg_zero);                                \
  }                                                                           \
  mips_emit_addu(_rd, _rn, _rm);                                              \
  if(check_generate_v_flag)                                                   \
  {                                                                           \
    mips_emit_slt(reg_a0, _rd, reg_c_cache);                                  \
    mips_emit_xor(reg_v_cache, reg_v_cache, reg_a0);                          \
  }                                                                           \
  if(check_generate_c_flag)                                                   \
  {                                                                           \
    mips_emit_sltu(reg_c_cache, _rd, reg_c_cache);                            \
  }                                                                           \
  generate_op_logic_flags(_rd)                                                \

#define generate_op_adcs_reg(_rd, _rn, _rm)                                   \
  if(check_generate_v_flag)                                                   \
  {                                                                           \
    mips_emit_xor(reg_v_cache, _rn, _rm);                                     \
    mips_emit_nor(reg_v_cache, reg_v_cache, reg_zero);                        \
    mips_emit_addu(reg_rv, _rn, reg_zero);                                    \
  }                                                                           \
  mips_emit_addu(reg_a2, _rn, _rm);                                           \
  if(check_generate_c_flag)                                                   \
  {                                                                           \
    mips_emit_sltu(reg_temp, reg_a2, _rm);                                    \
  }                                                                           \
  mips_emit_addu(_rd, reg_a2, reg_c_cache);                                   \
  if(check_generate_v_flag)                                                   \
  {                                                                           \
    mips_emit_xor(reg_rv, reg_rv, _rd);                                       \
    mips_emit_and(reg_v_cache, reg_rv, reg_v_cache);                          \
    mips_emit_srl(reg_v_cache, reg_v_cache, 31);                              \
  }                                                                           \
  if(check_generate_c_flag)                                                   \
  {                                                                           \
    mips_emit_sltu(reg_c_cache, _rd, reg_c_cache);                            \
    mips_emit_or(reg_c_cache, reg_temp, reg_c_cache);                         \
  }                                                                           \
  generate_op_logic_flags(_rd)                                                \

#define generate_op_movs_reg(_rd, _rn, _rm)                                   \
  mips_emit_addu(_rd, _rm, reg_zero);                                         \
  generate_op_logic_flags(_rd)                                                \

#define generate_op_mvns_reg(_rd, _rn, _rm)                                   \
  mips_emit_nor(_rd, _rm, reg_zero);                                          \
  generate_op_logic_flags(_rd)                                                \

#define generate_op_neg_reg(_rd, _rn, _rm)                                    \
  generate_op_subs_reg(_rd, reg_zero, _rm)                                    \

#define generate_op_muls_reg(_rd, _rn, _rm)                                   \
  mips_emit_multu(_rn, _rm);                                                  \
  mips_emit_mflo(_rd);                                                        \
  generate_op_logic_flags(_rd)                                                \

#define generate_op_cmp_reg(_rd, _rn, _rm)                                    \
  generate_op_subs_reg(reg_temp, _rn, _rm)                                    \

#define generate_op_cmn_reg(_rd, _rn, _rm)                                    \
  generate_op_adds_reg(reg_temp, _rn, _rm)                                    \

#define generate_op_tst_reg(_rd, _rn, _rm)                                    \
  generate_op_ands_reg(reg_temp, _rn, _rm)                                    \

#define generate_op_teq_reg(_rd, _rn, _rm)                                    \
  generate_op_eors_reg(reg_temp, _rn, _rm)                                    \

#define generate_op_ands_imm(_rd, _rn)                                        \
  generate_alu_immu(andi, and, _rd, _rn, imm);                                \
  generate_op_logic_flags(_rd)                                                \

#define generate_op_orrs_imm(_rd, _rn)                                        \
  generate_alu_immu(ori, or, _rd, _rn, imm);                                  \
  generate_op_logic_flags(_rd)                                                \

#define generate_op_eors_imm(_rd, _rn)                                        \
  generate_alu_immu(xori, xor, _rd, _rn, imm);                                \
  generate_op_logic_flags(_rd)                                                \

#define generate_op_bics_imm(_rd, _rn)                                        \
  generate_alu_immu(andi, and, _rd, _rn, (~imm));                             \
  generate_op_logic_flags(_rd)                                                \

#define generate_op_subs_imm(_rd, _rn)                                        \
  generate_op_imm_wrapper(subs, _rd, _rn)                                     \

#define generate_op_rsbs_imm(_rd, _rn)                                        \
  generate_op_imm_wrapper(rsbs, _rd, _rn)                                     \

#define generate_op_sbcs_imm(_rd, _rn)                                        \
  generate_op_imm_wrapper(sbcs, _rd, _rn)                                     \

#define generate_op_rscs_imm(_rd, _rn)                                        \
  generate_op_imm_wrapper(rscs, _rd, _rn)                                     \

#define generate_op_adds_imm(_rd, _rn)                                        \
  generate_op_imm_wrapper(adds, _rd, _rn)                                     \

#define generate_op_adcs_imm(_rd, _rn)                                        \
  generate_op_imm_wrapper(adcs, _rd, _rn)                                     \

#define generate_op_movs_imm(_rd, _rn)                                        \
  generate_load_imm(_rd, imm);                                                \
  generate_op_logic_flags(_rd)                                                \

#define generate_op_mvns_imm(_rd, _rn)                                        \
  generate_load_imm(_rd, (~imm));                                             \
  generate_op_logic_flags(_rd)                                                \

#define generate_op_cmp_imm(_rd, _rn)                                         \
  generate_op_imm_wrapper(cmp, _rd, _rn)                                      \

#define generate_op_cmn_imm(_rd, _rn)                                         \
  generate_op_imm_wrapper(cmn, _rd, _rn)                                      \

#define generate_op_tst_imm(_rd, _rn)                                         \
  generate_op_ands_imm(reg_temp, _rn)                                         \

#define generate_op_teq_imm(_rd, _rn)                                         \
  generate_op_eors_imm(reg_temp, _rn)                                         \

#define arm_generate_op_load_yes()                                            \
  generate_load_reg_pc(reg_a1, rn, 8)                                         \

#define arm_generate_op_load_no()                                             \

#define arm_op_check_yes()                                                    \
  check_load_reg_pc(arm_reg_a1, rn, 8)                                        \

#define arm_op_check_no()                                                     \

#define arm_generate_op_reg_flags(name, load_op)                              \
  arm_decode_data_proc_reg(opcode);                                           \
  if(check_generate_c_flag)                                                   \
  {                                                                           \
    generate_load_rm_sh(rm, flags);                                           \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_rm_sh(rm, no_flags);                                        \
  }                                                                           \
                                                                              \
  arm_op_check_##load_op();                                                   \
  generate_op_##name##_reg(arm_to_mips_reg[rd], arm_to_mips_reg[rn],          \
   arm_to_mips_reg[rm])                                                       \

#define arm_generate_op_reg(name, load_op)                                    \
  arm_decode_data_proc_reg(opcode);                                           \
  generate_load_rm_sh(rm, no_flags);                                          \
  arm_op_check_##load_op();                                                   \
  generate_op_##name##_reg(arm_to_mips_reg[rd], arm_to_mips_reg[rn],          \
   arm_to_mips_reg[rm])                                                       \

#define arm_generate_op_imm(name, load_op)                                    \
  arm_decode_data_proc_imm(opcode);                                           \
  ror(imm, imm, imm_ror);                                                     \
  arm_op_check_##load_op();                                                   \
  generate_op_##name##_imm(arm_to_mips_reg[rd], arm_to_mips_reg[rn])          \

#define arm_generate_op_imm_flags(name, load_op)                              \
  arm_decode_data_proc_imm(opcode);                                           \
  ror(imm, imm, imm_ror);                                                     \
  if(check_generate_c_flag && (imm_ror != 0))                                 \
  {  /* Generate carry flag from integer rotation */                          \
     mips_emit_addiu(reg_c_cache, reg_zero, ((imm) >> 31));                   \
  }                                                                           \
  arm_op_check_##load_op();                                                   \
  generate_op_##name##_imm(arm_to_mips_reg[rd], arm_to_mips_reg[rn])          \

#define arm_data_proc(name, type, flags_op)                                   \
{                                                                             \
  arm_generate_op_##type(name, yes);                                          \
  check_store_reg_pc_##flags_op(rd);                                          \
}                                                                             \

#define arm_data_proc_test(name, type)                                        \
{                                                                             \
  arm_generate_op_##type(name, yes);                                          \
}                                                                             \

#define arm_data_proc_unary(name, type, flags_op)                             \
{                                                                             \
  arm_generate_op_##type(name, no);                                           \
  check_store_reg_pc_##flags_op(rd);                                          \
}                                                                             \

#define arm_multiply_flags_yes(_rd)                                           \
  generate_op_logic_flags(_rd)                                                \

#define arm_multiply_flags_no(_rd)                                            \

#define arm_multiply_add_no()                                                 \
  mips_emit_mflo(arm_to_mips_reg[rd])                                         \

#define arm_multiply_add_yes()                                                \
  mips_emit_mflo(reg_temp);                                                   \
  mips_emit_addu(arm_to_mips_reg[rd], reg_temp, arm_to_mips_reg[rn])          \

#define arm_multiply(add_op, flags)                                           \
{                                                                             \
  arm_decode_multiply();                                                      \
  mips_emit_multu(arm_to_mips_reg[rm], arm_to_mips_reg[rs]);                  \
  arm_multiply_add_##add_op();                                                \
  arm_multiply_flags_##flags(arm_to_mips_reg[rd]);                            \
}                                                                             \

#define arm_multiply_long_flags_yes(_rdlo, _rdhi)                             \
  mips_emit_sltiu(reg_z_cache, _rdlo, 1);                                     \
  mips_emit_sltiu(reg_a0, _rdhi, 1);                                          \
  mips_emit_and(reg_z_cache, reg_z_cache, reg_a0);                            \
  mips_emit_srl(reg_n_cache, _rdhi, 31);                                      \

#define arm_multiply_long_flags_no(_rdlo, _rdhi)                              \

#define arm_multiply_long_add_yes(name)                                       \
  mips_emit_mtlo(arm_to_mips_reg[rdlo]);                                      \
  mips_emit_mthi(arm_to_mips_reg[rdhi]);                                      \
  generate_multiply_##name()                                                  \

#define arm_multiply_long_add_no(name)                                        \
  generate_multiply_##name()                                                  \

#define arm_multiply_long(name, add_op, flags)                                \
{                                                                             \
  arm_decode_multiply_long();                                                 \
  arm_multiply_long_add_##add_op(name);                                       \
  mips_emit_mflo(arm_to_mips_reg[rdlo]);                                      \
  mips_emit_mfhi(arm_to_mips_reg[rdhi]);                                      \
  arm_multiply_long_flags_##flags(arm_to_mips_reg[rdlo],                      \
   arm_to_mips_reg[rdhi]);                                                    \
}                                                                             \

#define arm_psr_read(op_type, psr_reg)                                        \
  generate_function_call(execute_read_##psr_reg);                             \
  generate_store_reg(reg_rv, rd)                                              \

u32 execute_store_cpsr_body(u32 _cpsr, u32 address)
{
  set_cpu_mode(cpu_modes[_cpsr & 0xF]);
  if((io_registers[REG_IE] & io_registers[REG_IF]) &&
   io_registers[REG_IME] && ((_cpsr & 0x80) == 0))
  {
    REG_MODE(MODE_IRQ)[6] = address + 4;
    REG_SPSR(MODE_IRQ) = _cpsr;
    reg[REG_CPSR] = 0xD2;
    set_cpu_mode(MODE_IRQ);
    return 0x00000018;
  }

  return 0;
}

#define arm_psr_load_new_reg()                                                \
  generate_load_reg(reg_a0, rm)                                               \

#define arm_psr_load_new_imm()                                                \
  generate_load_imm(reg_a0, imm)                                              \

#define arm_psr_store_spsr()                                                  \
  generate_load_imm(reg_a1, spsr_masks[psr_pfield]);                          \
  generate_function_call_swap_delay(execute_store_spsr)                       \

#define arm_psr_store_cpsr()                                                  \
  generate_load_pc(reg_a1, (pc));                                             \
  generate_function_call_swap_delay(execute_store_cpsr);                      \
  generate_raw_u32(cpsr_masks[psr_pfield][0]);                                \
  generate_raw_u32(cpsr_masks[psr_pfield][1]);                                \

#define arm_psr_store(op_type, psr_reg)                                       \
  arm_psr_load_new_##op_type();                                               \
  arm_psr_store_##psr_reg();                                                  \

#define arm_psr(op_type, transfer_type, psr_reg)                              \
{                                                                             \
  arm_decode_psr_##op_type(opcode);                                           \
  arm_psr_##transfer_type(op_type, psr_reg);                                  \
}                                                                             \

#define thumb_load_pc_pool_const(rd, value)                                   \
  generate_load_imm(arm_to_mips_reg[rd], (value));                            \

#define arm_access_memory_load(mem_type)                                      \
  cycle_count += 2;                                                           \
  mips_emit_jal(mips_absolute_offset(execute_load_##mem_type));               \
  generate_load_pc(reg_a1, (pc));                                             \
  generate_store_reg(reg_rv, rd);                                             \
  check_store_reg_pc_no_flags(rd)                                             \

#define arm_access_memory_store(mem_type)                                     \
  cycle_count++;                                                              \
  generate_load_pc(reg_a2, (pc + 4));                                         \
  generate_load_reg_pc(reg_a1, rd, 12);                                       \
  generate_function_call_swap_delay(execute_store_##mem_type)                 \

#define arm_access_memory_reg_pre_up()                                        \
  mips_emit_addu(reg_a0, arm_to_mips_reg[rn], arm_to_mips_reg[rm])            \

#define arm_access_memory_reg_pre_down()                                      \
  mips_emit_subu(reg_a0, arm_to_mips_reg[rn], arm_to_mips_reg[rm])            \

#define arm_access_memory_reg_pre(adjust_dir)                                 \
  check_load_reg_pc(arm_reg_a0, rn, 8);                                       \
  arm_access_memory_reg_pre_##adjust_dir()                                    \

#define arm_access_memory_reg_pre_wb(adjust_dir)                              \
  arm_access_memory_reg_pre(adjust_dir);                                      \
  generate_store_reg(reg_a0, rn)                                              \

#define arm_access_memory_reg_post_up()                                       \
  mips_emit_addu(arm_to_mips_reg[rn], arm_to_mips_reg[rn],                    \
   arm_to_mips_reg[rm])                                                       \

#define arm_access_memory_reg_post_down()                                     \
  mips_emit_subu(arm_to_mips_reg[rn], arm_to_mips_reg[rn],                    \
   arm_to_mips_reg[rm])                                                       \

#define arm_access_memory_reg_post(adjust_dir)                                \
  generate_load_reg(reg_a0, rn);                                              \
  arm_access_memory_reg_post_##adjust_dir()                                   \

#define arm_access_memory_imm_pre_up()                                        \
  mips_emit_addiu(reg_a0, arm_to_mips_reg[rn], offset)                        \

#define arm_access_memory_imm_pre_down()                                      \
  mips_emit_addiu(reg_a0, arm_to_mips_reg[rn], -offset)                       \

#define arm_access_memory_imm_pre(adjust_dir)                                 \
  check_load_reg_pc(arm_reg_a0, rn, 8);                                       \
  arm_access_memory_imm_pre_##adjust_dir()                                    \

#define arm_access_memory_imm_pre_wb(adjust_dir)                              \
  arm_access_memory_imm_pre(adjust_dir);                                      \
  generate_store_reg(reg_a0, rn)                                              \

#define arm_access_memory_imm_post_up()                                       \
  mips_emit_addiu(arm_to_mips_reg[rn], arm_to_mips_reg[rn], offset)           \

#define arm_access_memory_imm_post_down()                                     \
  mips_emit_addiu(arm_to_mips_reg[rn], arm_to_mips_reg[rn], -offset)          \

#define arm_access_memory_imm_post(adjust_dir)                                \
  generate_load_reg(reg_a0, rn);                                              \
  arm_access_memory_imm_post_##adjust_dir()                                   \

#define arm_data_trans_reg(adjust_op, adjust_dir)                             \
  arm_decode_data_trans_reg();                                                \
  generate_load_offset_sh(rm);                                                \
  arm_access_memory_reg_##adjust_op(adjust_dir)                               \

#define arm_data_trans_imm(adjust_op, adjust_dir)                             \
  arm_decode_data_trans_imm();                                                \
  arm_access_memory_imm_##adjust_op(adjust_dir)                               \

#define arm_data_trans_half_reg(adjust_op, adjust_dir)                        \
  arm_decode_half_trans_r();                                                  \
  arm_access_memory_reg_##adjust_op(adjust_dir)                               \

#define arm_data_trans_half_imm(adjust_op, adjust_dir)                        \
  arm_decode_half_trans_of();                                                 \
  arm_access_memory_imm_##adjust_op(adjust_dir)                               \

#define arm_access_memory(access_type, direction, adjust_op, mem_type,        \
 offset_type)                                                                 \
{                                                                             \
  arm_data_trans_##offset_type(adjust_op, direction);                         \
  arm_access_memory_##access_type(mem_type);                                  \
}                                                                             \

#define word_bit_count(word)                                                  \
  (bit_count[word >> 8] + bit_count[word & 0xFF])                             \

#define arm_block_memory_load()                                               \
  generate_function_call_swap_delay(execute_aligned_load32);                  \
  generate_store_reg(reg_rv, i)                                               \

#define arm_block_memory_store()                                              \
  generate_load_reg_pc(reg_a1, i, 8);                                         \
  generate_function_call_swap_delay(execute_aligned_store32)                  \

#define arm_block_memory_final_load(writeback_type)                           \
  arm_block_memory_load()                                                     \

#define arm_block_memory_final_store(writeback_type)                          \
  generate_load_pc(reg_a2, (pc + 4));                                         \
  generate_load_reg(reg_a1, i);                                               \
  arm_block_memory_writeback_post_store(writeback_type);                      \
  generate_function_call_swap_delay(execute_store_u32);                       \

#define arm_block_memory_adjust_pc_store()                                    \

#define arm_block_memory_adjust_pc_load()                                     \
  if(reg_list & 0x8000)                                                       \
  {                                                                           \
    generate_mov(reg_a0, reg_rv);                                             \
    generate_indirect_branch_arm();                                           \
  }                                                                           \

#define arm_block_memory_sp_load()                                            \
  mips_emit_lw(arm_to_mips_reg[i], reg_a1, offset);                           \

#define arm_block_memory_sp_store()                                           \
{                                                                             \
  u32 store_reg = i;                                                          \
  check_load_reg_pc(arm_reg_a0, store_reg, 8);                                \
  mips_emit_sw(arm_to_mips_reg[store_reg], reg_a1, offset);                   \
}                                                                             \

#define arm_block_memory_sp_adjust_pc_store()                                 \

#define arm_block_memory_sp_adjust_pc_load()                                  \
  if(reg_list & 0x8000)                                                       \
  {                                                                           \
    generate_indirect_branch_arm();                                           \
  }                                                                           \

#define arm_block_memory_offset_down_a()                                      \
  mips_emit_addiu(reg_a2, base_reg, (-((word_bit_count(reg_list) * 4) - 4)))  \

#define arm_block_memory_offset_down_b()                                      \
  mips_emit_addiu(reg_a2, base_reg, (word_bit_count(reg_list) * -4))          \

#define arm_block_memory_offset_no()                                          \
  mips_emit_addu(reg_a2, base_reg, reg_zero)                                  \

#define arm_block_memory_offset_up()                                          \
  mips_emit_addiu(reg_a2, base_reg, 4)                                        \

#define arm_block_memory_writeback_down()                                     \
  mips_emit_addiu(base_reg, base_reg, (-(word_bit_count(reg_list) * 4)))      \

#define arm_block_memory_writeback_up()                                       \
  mips_emit_addiu(base_reg, base_reg, (word_bit_count(reg_list) * 4))         \

#define arm_block_memory_writeback_no()

// Only emit writeback if the register is not in the list

#define arm_block_memory_writeback_post_load(writeback_type)
#define arm_block_memory_writeback_pre_load(writeback_type)                   \
  if(!((reg_list >> rn) & 0x01))                                              \
  {                                                                           \
    arm_block_memory_writeback_##writeback_type();                            \
  }                                                                           \

#define arm_block_memory_writeback_pre_store(writeback_type)
#define arm_block_memory_writeback_post_store(writeback_type)                 \
  arm_block_memory_writeback_##writeback_type()                               \

#define arm_block_memory(access_type, offset_type, writeback_type, s_bit)     \
{                                                                             \
  arm_decode_block_trans();                                                   \
  u32 i;                                                                      \
  u32 offset = 0;                                                             \
  u32 base_reg = arm_to_mips_reg[rn];                                         \
                                                                              \
  arm_block_memory_offset_##offset_type();                                    \
  arm_block_memory_writeback_pre_##access_type(writeback_type);               \
                                                                              \
  if(rn == REG_SP)                                                            \
  {                                                                           \
    /* Assume IWRAM, the most common path by far */                           \
    mips_emit_andi(reg_a1, reg_a2, 0x7FFC);                                   \
    /* Check the 23rd bit to differenciate IW/EW RAMs */                      \
    mips_emit_srl(reg_temp, reg_a2, 24);                                      \
    mips_emit_sll(reg_temp, reg_temp, 31);                                    \
    mips_emit_bgezal(reg_temp,                                                \
                mips_relative_offset(translation_ptr, spaccess_trampoline));  \
    /* Delay slot, will be overwritten anyway */                              \
    mips_emit_lui(reg_a0, ((u32)(iwram + 0x8000 + 0x8000) >> 16));            \
    mips_emit_addu(reg_a1, reg_a1, reg_a0);                                   \
    offset = (u32)(iwram + 0x8000) & 0xFFFF;                                  \
                                                                              \
    for(i = 0; i < 16; i++)                                                   \
    {                                                                         \
      if((reg_list >> i) & 0x01)                                              \
      {                                                                       \
        cycle_count++;                                                        \
        arm_block_memory_sp_##access_type();                                  \
        offset += 4;                                                          \
      }                                                                       \
    }                                                                         \
                                                                              \
    arm_block_memory_writeback_post_##access_type(writeback_type);            \
    arm_block_memory_sp_adjust_pc_##access_type();                            \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    emit_align_reg(reg_a2, 2);                                                \
                                                                              \
    for(i = 0; i < 16; i++)                                                   \
    {                                                                         \
      if((reg_list >> i) & 0x01)                                              \
      {                                                                       \
        cycle_count++;                                                        \
        mips_emit_addiu(reg_a0, reg_a2, offset);                              \
        if(reg_list & ~((2 << i) - 1))                                        \
        {                                                                     \
          arm_block_memory_##access_type();                                   \
          offset += 4;                                                        \
        }                                                                     \
        else                                                                  \
        {                                                                     \
          arm_block_memory_final_##access_type(writeback_type);               \
          break;                                                              \
        }                                                                     \
      }                                                                       \
    }                                                                         \
                                                                              \
    arm_block_memory_adjust_pc_##access_type();                               \
  }                                                                           \
}                                                                             \

// ARM: rn *must* be different from rm and rd. rm *can* be the same as rd.

#define arm_swap(type)                                                        \
{                                                                             \
  arm_decode_swap();                                                          \
  cycle_count += 3;                                                           \
  mips_emit_jal(mips_absolute_offset(execute_load_##type));                   \
  generate_load_reg(reg_a0, rn);                                              \
  generate_load_reg(reg_a0, rn);                                              \
  generate_load_reg(reg_a1, rm);                                              \
  mips_emit_jal(mips_absolute_offset(execute_store_##type));                  \
  generate_store_reg(reg_rv, rd);                                             \
}                                                                             \

#define thumb_generate_op_load_yes(_rs)                                       \
  generate_load_reg(reg_a1, _rs)                                              \

#define thumb_generate_op_load_no(_rs)                                        \

#define thumb_generate_op_reg(name, _rd, _rs, _rn)                            \
  generate_op_##name##_reg(arm_to_mips_reg[_rd],                              \
   arm_to_mips_reg[_rs], arm_to_mips_reg[_rn])                                \

#define thumb_generate_op_imm(name, _rd, _rs, _rn)                            \
  generate_op_##name##_imm(arm_to_mips_reg[_rd], arm_to_mips_reg[_rs])        \

// Types: add_sub, add_sub_imm, alu_op, imm
// Affects N/Z/C/V flags

#define thumb_data_proc(type, name, rn_type, _rd, _rs, _rn)                   \
{                                                                             \
  thumb_decode_##type();                                                      \
  thumb_generate_op_##rn_type(name, _rd, _rs, _rn);                           \
}                                                                             \

#define thumb_data_proc_test(type, name, rn_type, _rs, _rn)                   \
{                                                                             \
  thumb_decode_##type();                                                      \
  thumb_generate_op_##rn_type(name, 0, _rs, _rn);                             \
}                                                                             \

#define thumb_data_proc_unary(type, name, rn_type, _rd, _rn)                  \
{                                                                             \
  thumb_decode_##type();                                                      \
  thumb_generate_op_##rn_type(name, _rd, 0, _rn);                             \
}                                                                             \

#define check_store_reg_pc_thumb(_rd)                                         \
  if(_rd == REG_PC)                                                           \
  {                                                                           \
    generate_indirect_branch_cycle_update(thumb);                             \
  }                                                                           \

#define thumb_data_proc_hi(name)                                              \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  u32 dest_rd = rd;                                                           \
  check_load_reg_pc(arm_reg_a0, rs, 4);                                       \
  check_load_reg_pc(arm_reg_a1, rd, 4);                                       \
  generate_op_##name##_reg(arm_to_mips_reg[dest_rd], arm_to_mips_reg[rd],     \
   arm_to_mips_reg[rs]);                                                      \
  check_store_reg_pc_thumb(dest_rd);                                          \
}                                                                             \

#define thumb_data_proc_test_hi(name)                                         \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  check_load_reg_pc(arm_reg_a0, rs, 4);                                       \
  check_load_reg_pc(arm_reg_a1, rd, 4);                                       \
  generate_op_##name##_reg(reg_temp, arm_to_mips_reg[rd],                     \
   arm_to_mips_reg[rs]);                                                      \
}                                                                             \

#define thumb_data_proc_mov_hi()                                              \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  check_load_reg_pc(arm_reg_a0, rs, 4);                                       \
  mips_emit_addu(arm_to_mips_reg[rd], arm_to_mips_reg[rs], reg_zero);         \
  check_store_reg_pc_thumb(rd);                                               \
}                                                                             \

#define thumb_load_pc(_rd)                                                    \
{                                                                             \
  thumb_decode_imm();                                                         \
  generate_load_pc(arm_to_mips_reg[_rd], (((pc & ~2) + 4) + (imm * 4)));      \
}                                                                             \

#define thumb_load_sp(_rd)                                                    \
{                                                                             \
  thumb_decode_imm();                                                         \
  mips_emit_addiu(arm_to_mips_reg[_rd], reg_r13, (imm * 4));                  \
}                                                                             \

#define thumb_adjust_sp_up()                                                  \
  mips_emit_addiu(reg_r13, reg_r13, (imm * 4));                               \

#define thumb_adjust_sp_down()                                                \
  mips_emit_addiu(reg_r13, reg_r13, -(imm * 4));                              \

#define thumb_adjust_sp(direction)                                            \
{                                                                             \
  thumb_decode_add_sp();                                                      \
  thumb_adjust_sp_##direction();                                              \
}                                                                             \

// Decode types: shift, alu_op
// Operation types: lsl, lsr, asr, ror
// Affects N/Z/C flags

#define thumb_generate_shift_imm(name)                                        \
  if(check_generate_c_flag)                                                   \
  {                                                                           \
    generate_shift_imm_##name##_flags(rd, rs, imm);                           \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_shift_imm_##name##_no_flags(rd, rs, imm);                        \
  }                                                                           \
  if(rs != rd)                                                                \
  {                                                                           \
    mips_emit_addu(arm_to_mips_reg[rd], arm_to_mips_reg[rs], reg_zero);       \
  }                                                                           \

#define thumb_generate_shift_reg(name)                                        \
{                                                                             \
  u32 original_rd = rd;                                                       \
  if(check_generate_c_flag)                                                   \
  {                                                                           \
    generate_shift_reg_##name##_flags(rd, rs);                                \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_shift_reg_##name##_no_flags(rd, rs);                             \
  }                                                                           \
  mips_emit_addu(arm_to_mips_reg[original_rd], reg_a0, reg_zero);             \
}                                                                             \

#define thumb_shift(decode_type, op_type, value_type)                         \
{                                                                             \
  thumb_decode_##decode_type();                                               \
  thumb_generate_shift_##value_type(op_type);                                 \
  generate_op_logic_flags(arm_to_mips_reg[rd]);                               \
}                                                                             \

// Operation types: imm, mem_reg, mem_imm

#define thumb_access_memory_load(mem_type, reg_rd)                            \
  cycle_count += 2;                                                           \
  mips_emit_jal(mips_absolute_offset(execute_load_##mem_type));               \
  generate_load_pc(reg_a1, (pc));                                             \
  generate_store_reg(reg_rv, reg_rd)                                          \

#define thumb_access_memory_store(mem_type, reg_rd)                           \
  cycle_count++;                                                              \
  generate_load_pc(reg_a2, (pc + 2));                                         \
  mips_emit_jal(mips_absolute_offset(execute_store_##mem_type));              \
  generate_load_reg(reg_a1, reg_rd)                                           \

#define thumb_access_memory_generate_address_pc_relative(offset, reg_rb,      \
 reg_ro)                                                                      \
  generate_load_pc(reg_a0, (offset))                                          \

#define thumb_access_memory_generate_address_reg_imm(offset, reg_rb, reg_ro)  \
  mips_emit_addiu(reg_a0, arm_to_mips_reg[reg_rb], (offset))                  \

#define thumb_access_memory_generate_address_reg_imm_sp(offset, reg_rb, reg_ro) \
  mips_emit_addiu(reg_a0, arm_to_mips_reg[reg_rb], (offset * 4))              \

#define thumb_access_memory_generate_address_reg_reg(offset, reg_rb, reg_ro)  \
  mips_emit_addu(reg_a0, arm_to_mips_reg[reg_rb], arm_to_mips_reg[reg_ro])    \

#define thumb_access_memory(access_type, op_type, reg_rd, reg_rb, reg_ro,     \
 address_type, offset, mem_type)                                              \
{                                                                             \
  thumb_decode_##op_type();                                                   \
  thumb_access_memory_generate_address_##address_type(offset, reg_rb,         \
   reg_ro);                                                                   \
  thumb_access_memory_##access_type(mem_type, reg_rd);                        \
}                                                                             \

#define thumb_block_address_preadjust_no(base_reg)                            \
  mips_emit_addu(reg_a2, arm_to_mips_reg[base_reg], reg_zero)                 \

#define thumb_block_address_preadjust_down(base_reg)                          \
  mips_emit_addiu(reg_a2, arm_to_mips_reg[base_reg],                          \
   -(bit_count[reg_list] * 4));                                               \
  mips_emit_addu(arm_to_mips_reg[base_reg], reg_a2, reg_zero)                 \

#define thumb_block_address_preadjust_push_lr(base_reg)                       \
  mips_emit_addiu(reg_a2, arm_to_mips_reg[base_reg],                          \
   -((bit_count[reg_list] + 1) * 4));                                         \
  mips_emit_addu(arm_to_mips_reg[base_reg], reg_a2, reg_zero)                 \

#define thumb_block_address_postadjust_no(base_reg)                           \

#define thumb_block_address_postadjust_up(base_reg)                           \
  mips_emit_addiu(arm_to_mips_reg[base_reg], reg_a2,                          \
   (bit_count[reg_list] * 4))                                                 \

#define thumb_block_address_postadjust_pop_pc(base_reg)                       \
  mips_emit_addiu(arm_to_mips_reg[base_reg], reg_a2,                          \
   ((bit_count[reg_list] * 4) + 4))                                           \

#define thumb_block_address_postadjust_push_lr(base_reg)                      \

#define thumb_block_memory_load()                                             \
  generate_function_call_swap_delay(execute_aligned_load32);                  \
  generate_store_reg(reg_rv, i)                                               \

#define thumb_block_memory_store()                                            \
  mips_emit_jal(mips_absolute_offset(execute_aligned_store32));               \
  generate_load_reg(reg_a1, i)                                                \

#define thumb_block_memory_final_load()                                       \
  thumb_block_memory_load()                                                   \

#define thumb_block_memory_final_store()                                      \
  generate_load_pc(reg_a2, (pc + 2));                                         \
  mips_emit_jal(mips_absolute_offset(execute_store_u32));                     \
  generate_load_reg(reg_a1, i)                                                \

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

#define thumb_block_memory_extra_no()                                         \

#define thumb_block_memory_extra_up()                                         \

#define thumb_block_memory_extra_down()                                       \

#define thumb_block_memory_extra_push_lr()                                    \
  mips_emit_addiu(reg_a0, reg_a2, (bit_count[reg_list] * 4));                 \
  mips_emit_jal(mips_absolute_offset(execute_aligned_store32));               \
  generate_load_reg(reg_a1, REG_LR)                                           \

#define thumb_block_memory_extra_pop_pc()                                     \
  mips_emit_jal(mips_absolute_offset(execute_aligned_load32));                \
  mips_emit_addiu(reg_a0, reg_a2, (bit_count[reg_list] * 4));                 \
  generate_mov(reg_a0, reg_rv);                                               \
  generate_indirect_branch_cycle_update(thumb)                                \

#define thumb_block_memory_sp_load()                                          \
  mips_emit_lw(arm_to_mips_reg[i], reg_a1, offset)                            \

#define thumb_block_memory_sp_store()                                         \
  mips_emit_sw(arm_to_mips_reg[i], reg_a1, offset)                            \

#define thumb_block_memory_sp_extra_no()                                      \

#define thumb_block_memory_sp_extra_up()                                      \

#define thumb_block_memory_sp_extra_down()                                    \

#define thumb_block_memory_sp_extra_pop_pc()                                  \
  mips_emit_lw(reg_a0, reg_a1, offset);                                       \
  generate_indirect_branch_cycle_update(thumb)                                \

#define thumb_block_memory_sp_extra_push_lr()                                 \
  mips_emit_sw(reg_r14, reg_a1, offset)                                       \

#define thumb_block_memory(access_type, pre_op, post_op, base_reg)            \
{                                                                             \
  thumb_decode_rlist();                                                       \
  u32 i;                                                                      \
  u32 offset = 0;                                                             \
                                                                              \
  thumb_block_address_preadjust_##pre_op(base_reg);                           \
  thumb_block_address_postadjust_##post_op(base_reg);                         \
                                                                              \
  if(base_reg == REG_SP)                                                      \
  {                                                                           \
    /* Assume IWRAM, the most common path by far */                           \
    mips_emit_andi(reg_a1, reg_a2, 0x7FFC);                                   \
    /* Check the 23rd bit to differenciate IW/EW RAMs */                      \
    mips_emit_srl(reg_temp, reg_a2, 24);                                      \
    mips_emit_sll(reg_temp, reg_temp, 31);                                    \
    mips_emit_bgezal(reg_temp,                                                \
                mips_relative_offset(translation_ptr, spaccess_trampoline));  \
    /* Delay slot, will be overwritten anyway */                              \
    mips_emit_lui(reg_a0, ((u32)(iwram + 0x8000 + 0x8000) >> 16));            \
    mips_emit_addu(reg_a1, reg_a1, reg_a0);                                   \
    offset = (u32)(iwram + 0x8000) & 0xFFFF;                                  \
                                                                              \
    for(i = 0; i < 8; i++)                                                    \
    {                                                                         \
      if((reg_list >> i) & 0x01)                                              \
      {                                                                       \
        cycle_count++;                                                        \
        thumb_block_memory_sp_##access_type();                                \
        offset += 4;                                                          \
      }                                                                       \
    }                                                                         \
                                                                              \
    thumb_block_memory_sp_extra_##post_op();                                  \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    emit_align_reg(reg_a2, 2);                                                \
                                                                              \
    for(i = 0; i < 8; i++)                                                    \
    {                                                                         \
      if((reg_list >> i) & 0x01)                                              \
      {                                                                       \
        cycle_count++;                                                        \
        mips_emit_addiu(reg_a0, reg_a2, offset);                              \
        if(reg_list & ~((2 << i) - 1))                                        \
        {                                                                     \
          thumb_block_memory_##access_type();                                 \
          offset += 4;                                                        \
        }                                                                     \
        else                                                                  \
        {                                                                     \
          thumb_block_memory_final_##post_op(access_type);                    \
          break;                                                              \
        }                                                                     \
      }                                                                       \
    }                                                                         \
                                                                              \
    thumb_block_memory_extra_##post_op();                                     \
  }                                                                           \
}



#define thumb_conditional_branch(condition)                                   \
{                                                                             \
  generate_condition_##condition();                                           \
  generate_branch_no_cycle_update(                                            \
   block_exits[block_exit_position].branch_source,                            \
   block_exits[block_exit_position].branch_target);                           \
  generate_branch_patch_conditional(backpatch_address, translation_ptr);      \
  block_exit_position++;                                                      \
}                                                                             \

#define arm_conditional_block_header()                                        \
  generate_condition();                                                       \

#define arm_b()                                                               \
  generate_branch()                                                           \

#define arm_bl()                                                              \
  generate_load_pc(reg_r14, (pc + 4));                                        \
  generate_branch()                                                           \

#define arm_bx()                                                              \
  arm_decode_branchx(opcode);                                                 \
  generate_load_reg(reg_a0, rn);                                              \
  /*generate_load_pc(reg_a2, pc);*/                                           \
  generate_indirect_branch_dual()                                             \

#define arm_swi()                                                             \
  generate_load_pc(reg_a0, (pc + 4));                                         \
  generate_function_call_swap_delay(execute_swi);                             \
  generate_branch()                                                           \

#define thumb_b()                                                             \
  generate_branch_cycle_update(                                               \
   block_exits[block_exit_position].branch_source,                            \
   block_exits[block_exit_position].branch_target);                           \
  block_exit_position++                                                       \

#define thumb_bl()                                                            \
  generate_load_pc(reg_r14, ((pc + 2) | 0x01));                               \
  generate_branch_cycle_update(                                               \
   block_exits[block_exit_position].branch_source,                            \
   block_exits[block_exit_position].branch_target);                           \
  block_exit_position++                                                       \

#define thumb_blh()                                                           \
{                                                                             \
  thumb_decode_branch();                                                      \
  mips_emit_addiu(reg_a0, reg_r14, (offset * 2));                             \
  generate_load_pc(reg_r14, ((pc + 2) | 0x01));                               \
  generate_indirect_branch_cycle_update(thumb);                               \
}                                                                             \

#define thumb_bx()                                                            \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  generate_load_reg_pc(reg_a0, rs, 4);                                        \
  /*generate_load_pc(reg_a2, pc);*/                                           \
  generate_indirect_branch_cycle_update(dual);                                \
}                                                                             \

#define thumb_process_cheats()                                                \
  generate_function_call(mips_cheat_hook);

#define arm_process_cheats()                                                  \
  generate_function_call(mips_cheat_hook);

#ifdef TRACE_INSTRUCTIONS
  void trace_instruction(u32 pc, u32 mode)
  {
    if (mode)
      printf("Executed arm %x\n", pc);
    else
      printf("Executed thumb %x\n", pc);
    #ifdef TRACE_REGISTERS
    print_regs();
    #endif
  }

  #define emit_trace_instruction(pc, mode)                                      \
    emit_save_regs(false);                                                      \
    generate_load_imm(reg_a0, pc);                                              \
    generate_load_imm(reg_a1, mode);                                            \
    genccall(&trace_instruction);                                               \
    mips_emit_nop();                                                            \
    emit_restore_regs(false)
  #define emit_trace_thumb_instruction(pc) emit_trace_instruction(pc, 0)
  #define emit_trace_arm_instruction(pc)   emit_trace_instruction(pc, 1)
#else
  #define emit_trace_thumb_instruction(pc)
  #define emit_trace_arm_instruction(pc)
#endif

#define thumb_swi()                                                           \
  generate_load_pc(reg_a0, (pc + 2));                                         \
  generate_function_call_swap_delay(execute_swi);                             \
  generate_branch_cycle_update(                                               \
   block_exits[block_exit_position].branch_source,                            \
   block_exits[block_exit_position].branch_target);                           \
  block_exit_position++                                                       \

#define arm_hle_div(cpu_mode)                                                 \
  mips_emit_div(reg_r0, reg_r1);                                              \
  mips_emit_mflo(reg_r0);                                                     \
  mips_emit_mfhi(reg_r1);                                                     \
  mips_emit_sra(reg_a0, reg_r0, 31);                                          \
  mips_emit_xor(reg_r3, reg_r0, reg_a0);                                      \
  mips_emit_subu(reg_r3, reg_r3, reg_a0);                                     \

#define arm_hle_div_arm(cpu_mode)                                             \
  mips_emit_div(reg_r1, reg_r0);                                              \
  mips_emit_mflo(reg_r0);                                                     \
  mips_emit_mfhi(reg_r1);                                                     \
  mips_emit_sra(reg_a0, reg_r0, 31);                                          \
  mips_emit_xor(reg_r3, reg_r0, reg_a0);                                      \
  mips_emit_subu(reg_r3, reg_r3, reg_a0);                                     \


#define generate_translation_gate(type)                                       \
  generate_load_pc(reg_a0, pc);                                               \
  generate_indirect_branch_no_cycle_update(type)                              \

#define generate_update_pc_reg()                                              \
  generate_load_pc(reg_a0, pc);                                               \
  mips_emit_sw(reg_a0, reg_base, (REG_PC * 4))                                \

// Some macros to wrap device-specific instructions

/* MIPS32R2 and PSP support ins, ext, seb, rotr */
#ifdef MIPS_HAS_R2_INSTS
  // Inserts LSB bits into another register
  #define insert_bits(rdest, rsrc, rtemp, pos, size) \
    mips_emit_ins(rdest, rsrc, pos, size);
  // Doubles a byte into a halfword
  #define double_byte(reg, rtmp) \
    mips_emit_ins(reg, reg, 8, 8);
  // Clears numbits at LSB position (to align an address)
  #define emit_align_reg(reg, numbits) \
    mips_emit_ins(reg, reg_zero, 0, numbits)
  // Extract a bitfield (pos, size) to a register
  #define extract_bits(rt, rs, pos, size) \
    mips_emit_ext(rt, rs, pos, size)
  // Extends signed byte to u32
  #define extend_byte_signed(rd, rs) \
    mips_emit_seb(rd, rs)
  // Rotates a word using a temp reg if necessary
  #define rotate_right(rdest, rsrc, rtemp, amount) \
    mips_emit_rotr(rdest, rsrc, amount);
  // Same but variable amount rotation (register)
  #define rotate_right_var(rdest, rsrc, rtemp, ramount) \
    mips_emit_rotrv(rdest, rsrc, ramount);
#else
  // Inserts LSB bits into another register
  // *assumes dest bits are cleared*!
  #define insert_bits(rdest, rsrc, rtemp, pos, size) \
    mips_emit_sll(rtemp, rsrc, 32 - size);           \
    mips_emit_srl(rtemp, rtemp, 32 - size - pos);    \
    mips_emit_or(rdest, rdest, rtemp);
  // Doubles a byte into a halfword
  #define double_byte(reg, rtmp)    \
    mips_emit_sll(rtmp, reg, 8);    \
    mips_emit_andi(reg, reg, 0xff); \
    mips_emit_or(reg, reg, rtmp);
  // Clears numbits at LSB position (to align an address)
  #define emit_align_reg(reg, numbits) \
    mips_emit_srl(reg, reg, numbits); \
    mips_emit_sll(reg, reg, numbits)
  // Extract a bitfield (pos, size) to a register
  #define extract_bits(rt, rs, pos, size) \
    mips_emit_sll(rt, rs, 32 - ((pos) + (size))); \
    mips_emit_srl(rt, rt, 32 - (size))
  // Extends signed byte to u32
  #define extend_byte_signed(rd, rs) \
    mips_emit_sll(rd, rs, 24); \
    mips_emit_sra(rd, rd, 24)
  // Rotates a word (uses temp reg)
  #define rotate_right(rdest, rsrc, rtemp, amount) \
    mips_emit_sll(rtemp, rsrc, 32 - (amount));     \
    mips_emit_srl(rdest, rsrc, (amount));          \
    mips_emit_or(rdest, rdest, rtemp)
  // Variable rotation using temp reg (dst != src)
  #define rotate_right_var(rdest, rsrc, rtemp, ramount) \
    mips_emit_andi(rtemp, ramount, 0x1F);               \
    mips_emit_srlv(rdest, rsrc, rtemp);                 \
    mips_emit_subu(rtemp, reg_zero, rtemp);             \
    mips_emit_addiu(rtemp, rtemp, 32);                  \
    mips_emit_sllv(rtemp, rsrc, rtemp);                 \
    mips_emit_or(rdest, rdest, rtemp)

#endif


// Register save layout as follows:
#define ReOff_RegPC    (REG_PC    * 4) // REG_PC
#define ReOff_CPSR     (REG_CPSR  * 4) // REG_CPSR
#define ReOff_SaveR1   (REG_SAVE  * 4) // 3 save scratch regs
#define ReOff_SaveR2   (REG_SAVE2 * 4)
#define ReOff_SaveR3   (REG_SAVE3 * 4)
#define ReOff_OamUpd   (OAM_UPDATED*4) // OAM_UPDATED
#define ReOff_GP_Save  (REG_SAVE5 * 4) // GP_SAVE

// Saves all regs to their right slot and loads gp
#define emit_save_regs(save_a2) {                                             \
  int i;                                                                      \
  for (i = 0; i < 15; i++) {                                                  \
    mips_emit_sw(arm_to_mips_reg[i], reg_base, 4 * i);                        \
  }                                                                           \
  if (save_a2) {                                                              \
    mips_emit_sw(reg_a2, reg_base, ReOff_SaveR2);                             \
  }                                                                           \
  /* Load the gp pointer, used by C code */                                   \
  mips_emit_lw(mips_reg_gp, reg_base, ReOff_GP_Save);                         \
}

// Restores the registers from their slot
#define emit_restore_regs(restore_a2) {                                       \
  int i;                                                                      \
  if (restore_a2) {                                                           \
    mips_emit_lw(reg_a2, reg_base, ReOff_SaveR2);                             \
  }                                                                           \
  for (i = 0; i < 15; i++) {                                                  \
    mips_emit_lw(arm_to_mips_reg[i], reg_base, 4 * i);                        \
  }                                                                           \
}

// Emits a function call for a read or a write (for special stuff like flash)
#define emit_mem_call_ds(fnptr, mask)                                         \
  mips_emit_sw(mips_reg_ra, reg_base, ReOff_SaveR1);                          \
  emit_save_regs(true);                                                       \
  genccall(fnptr);                                                            \
  mips_emit_andi(reg_a0, reg_a0, (mask));                                     \
  mips_emit_lw(mips_reg_ra, reg_base, ReOff_SaveR1);                          \
  emit_restore_regs(true);

#define emit_mem_call(fnptr, mask)      \
  emit_mem_call_ds(fnptr, mask)         \
  mips_emit_jr(mips_reg_ra);            \
  mips_emit_nop();

// Pointer table to stubs, indexed by type and region
extern u32 tmemld[11][16];
extern u32 tmemst[ 4][16];
extern u32 thnjal[15*16];
void smc_write();
cpu_alert_type write_io_register8 (u32 address, u32 value);
cpu_alert_type write_io_register16(u32 address, u32 value);
cpu_alert_type write_io_register32(u32 address, u32 value);
void write_io_epilogue();

// This is a pointer table to the open load stubs, used by the BIOS (optimization)
u32* openld_core_ptrs[11];

const u8 ldopmap[6][2] = { {0, 1}, {1, 2}, {2, 4}, {4, 6}, {6, 10}, {10, 11} };
const u8 ldhldrtbl[11] = {0, 1, 2, 2, 3, 3, 4, 4, 4, 4, 5};
#define ld_phndlr_branch(memop) \
  (((u32*)&rom_translation_cache[ldhldrtbl[(memop)]*16*4]) - ((u32*)translation_ptr + 1))

#define st_phndlr_branch(memop) \
  (((u32*)&rom_translation_cache[((memop) + 6)*16*4]) - ((u32*)translation_ptr + 1))

#define branch_handlerid(phndlrid) \
  (((u32*)&rom_translation_cache[(phndlrid)*16*4]) - ((u32*)translation_ptr + 1))

#define branch_offset(ptr) \
  (((u32*)ptr) - ((u32*)translation_ptr + 1))

static void emit_mem_access_loadop(
  u8 *translation_ptr,
  u32 base_addr, unsigned size, unsigned alignment, bool signext)
{
  switch (size) {
  case 2:
    mips_emit_lw(reg_rv, reg_rv, (base_addr & 0xffff));
    break;
  case 1:
    if (signext) {
      if (alignment) {
        // Unaligned signed 16b load, is just a load byte (due to sign extension)
        mips_emit_lb(reg_rv, reg_rv, ((base_addr | 1) & 0xffff));
      } else {
        mips_emit_lh(reg_rv, reg_rv, (base_addr & 0xffff));
      }
    } else {
      mips_emit_lhu(reg_rv, reg_rv, (base_addr & 0xffff));
    }
    break;
  default:
    if (signext) {
      mips_emit_lb(reg_rv, reg_rv, (base_addr & 0xffff));
    } else {
      mips_emit_lbu(reg_rv, reg_rv, (base_addr & 0xffff));
    }
    break;
  };
}

#ifdef PIC
  #define genccall(fn)                                         \
    mips_emit_lui(mips_reg_t9, ((u32)fn) >> 16);               \
    mips_emit_ori(mips_reg_t9, mips_reg_t9, ((u32)fn));        \
    mips_emit_jalr(mips_reg_t9);
#else
  #define genccall(fn) mips_emit_jal(((u32)fn) >> 2);
#endif

#define SMC_WRITE_OFF    (10*16*4)   /* 10 handlers (16 insts) */
#define IOEPILOGUE_OFF   (SMC_WRITE_OFF + 4*2)   /* Trampolines are two insts */
#define EWRAM_SPM_OFF    (IOEPILOGUE_OFF + 4*2)

// Describes a "plain" memory are, that is, an area that is just accessed
// as normal memory (with some caveats tho).
typedef struct {
  void *emitter;
  unsigned region;      // Region ID (top 8 bits)
  unsigned memsize;     // 0 byte, 1 halfword, 2 word
  bool check_smc;       // Whether the memory can contain code
  bool bus16;           // Whether it can only be accessed at 16bit
  u32 baseptr;          // Memory base address.
  u32 baseoff;          // Offset from base_reg
} t_stub_meminfo;

// Generates the stub to access memory for a given region, access type,
// size and misalignment.
// Handles "special" cases like weirdly mapped memory
static void emit_pmemld_stub(
  unsigned memop_number, const t_stub_meminfo *meminfo,
  bool signext, unsigned size,
  unsigned alignment, bool aligned, bool must_swap,
  u8 **tr_ptr)
{
  u8 *translation_ptr = *tr_ptr;
  unsigned region = meminfo->region;
  u32 base_addr = meminfo->baseptr;

  if (region >= 9 && region <= 11) {
    // Use the same handler for these regions (just replicas)
    tmemld[memop_number][region] = tmemld[memop_number][8];
    return;
  }

  // Clean up one or two bits (to align access). It might already be aligned!
  u32 memmask = (meminfo->memsize - 1);
  memmask = (memmask >> size) << size;    // Clear 1 or 2 (or none) bits

  // Add the stub to the table (add the JAL instruction encoded already)
  tmemld[memop_number][region] = (u32)translation_ptr;

  // Size: 0 (8 bits), 1 (16 bits), 2 (32 bits)
  // First check we are in the right memory region
  unsigned regionbits = 8;
  unsigned regioncheck = region;
  if (region == 8) {
    // This is an optimization for ROM regions
    // For region 8-11 we reuse the same code (and have a more generic check)
    // Region 12 is harder to cover without changing the check (shift + xor)
    regionbits = 6;
    regioncheck >>= 2;   // Ignore the two LSB, don't care
  }

  // Address checking: jumps to handler if bad region/alignment
  mips_emit_srl(reg_temp, reg_a0, (32 - regionbits));
  if (!aligned && size != 0) {  // u8 or aligned u32 dont need to check alignment bits
    insert_bits(reg_temp, reg_a0, reg_rv, regionbits, size);  // Add 1 or 2 bits of alignment
  }
  if (regioncheck || alignment) {  // If region and alignment are zero, can skip
    mips_emit_xori(reg_temp, reg_temp, regioncheck | (alignment << regionbits));
  }

  // The patcher to use depends on ld/st, access size, and sign extension
  // (so there's 10 of them). They live in the top stub addresses.
  mips_emit_b(bne, reg_zero, reg_temp, ld_phndlr_branch(memop_number));

  // BIOS region requires extra checks for protected reads
  if (region == 0) {
    // BIOS is *not* mirrored, check that
    mips_emit_srl(reg_rv, reg_a0, 14);
    mips_emit_b(bne, reg_zero, reg_rv, branch_offset(openld_core_ptrs[memop_number]));

    // Check whether the read is allowed. Only within BIOS! (Ignore aligned, bad a1)
    if (!aligned) {
      mips_emit_srl(reg_temp, reg_a1, 14);
      mips_emit_b(bne, reg_zero, reg_temp, branch_offset(openld_core_ptrs[memop_number]));
    }
  }
  
  if (region >= 8 && region <= 12) {
    // ROM area: might need to load the ROM on-demand
    mips_emit_srl(reg_rv, reg_a0, 15);  // 32KB page number
    mips_emit_sll(reg_rv, reg_rv,  2);  // (word indexed)
    mips_emit_addu(reg_rv, reg_rv, reg_base);    // base + offset
    mips_emit_lw(reg_rv, reg_rv, 0x8000);        // base[offset-0x8000] is readmap ptr
    mips_emit_andi(reg_temp, reg_a0, memmask);   // Get the lowest 15 bits [can go in delay slot]

    if (must_swap) {   // Do not emit if the ROM is fully loaded, save some cycles
      u8 *jmppatch;
      mips_emit_b_filler(bne, reg_rv, reg_zero, jmppatch);  // if not null, can skip load page
      generate_swap_delay();

      // This code call the C routine to map the relevant ROM page
      emit_save_regs(aligned);
      mips_emit_sw(mips_reg_ra, reg_base, ReOff_SaveR3);
      extract_bits(reg_a0, reg_a0, 15, 10);    // a0 = (addr >> 15) & 0x3ff
      genccall(&load_gamepak_page);            // Returns valid pointer in rv
      mips_emit_sw(reg_temp, reg_base, ReOff_SaveR1);

      mips_emit_lw(reg_temp, reg_base, ReOff_SaveR1);
      emit_restore_regs(aligned);
      mips_emit_lw(mips_reg_ra, reg_base, ReOff_SaveR3);

      generate_branch_patch_conditional(jmppatch - 4, translation_ptr);
    }
    // Now we can proceed to load, place addr in the right register
    mips_emit_addu(reg_rv, reg_rv, reg_temp);
  } else if (region == 14) {
    // Read from flash, is a bit special, fn call
    emit_mem_call_ds(&read_backup, 0xFFFF);
    if (!size && signext) {
      extend_byte_signed(reg_rv, reg_rv);
    } else if (size == 1 && alignment) {
      extend_byte_signed(reg_rv, reg_rv);
    } else if (size == 2) {
      rotate_right(reg_rv, reg_rv, reg_temp, 8 * alignment);
    }
    generate_function_return_swap_delay();
    *tr_ptr = translation_ptr;
    return;
  } else {
    // Generate upper bits of the addr and do addr mirroring
    // (The address hi16 is rounded up since load uses signed offset)
    if (!meminfo->baseoff) {
      mips_emit_lui(reg_rv, ((base_addr + 0x8000) >> 16));
    } else {
      base_addr = meminfo->baseoff;
    }

    if (region == 2) {
      // Can't do EWRAM with an `andi` instruction (18 bits mask)
      extract_bits(reg_a0, reg_a0, 0, 18);       // &= 0x3ffff
      if (!aligned && alignment != 0) {
        emit_align_reg(reg_a0, size);            // addr & ~1/2 (align to size)
      }
      // Need to insert a zero in the addr (due to how it's mapped)
      mips_emit_addu(reg_rv, reg_rv, reg_a0);    // Adds to the base addr
    } else if (region == 6) {
      // VRAM is mirrored every 128KB but the last 32KB is mapped to the previous
      extract_bits(reg_temp, reg_a0, 15, 2);     // Extract bits 15 and 16
      mips_emit_addiu(reg_temp, reg_temp, -3);   // Check for 3 (last block)
      if (!aligned && alignment != 0) {
        emit_align_reg(reg_a0, size);            // addr & ~1/2 (align to size)
      }
      extract_bits(reg_a0, reg_a0, 0, 17);       // addr & 0x1FFFF [delay]
      mips_emit_b(bne, reg_zero, reg_temp, 1);   // Skip unless last block
      generate_swap_delay();
      mips_emit_addiu(reg_a0, reg_a0, 0x8000);   // addr - 0x8000 (mirror last block)
      mips_emit_addu(reg_rv, reg_rv, reg_a0);    // addr = base + adjusted offset
    } else {
      // Generate regular (<=32KB) mirroring
      mips_reg_number breg = (meminfo->baseoff ? reg_base : reg_rv);
      mips_emit_andi(reg_temp, reg_a0, memmask); // Clear upper bits (mirroring)
      mips_emit_addu(reg_rv, breg, reg_temp);    // Adds to base addr
    }
  }

  // Emit load operation
  emit_mem_access_loadop(translation_ptr, base_addr, size, alignment, signext);
  translation_ptr += 4;

  if (!(alignment == 0 || (size == 1 && signext))) {
    // Unaligned accesses require rotation, except for size=1 & signext
    rotate_right(reg_rv, reg_rv, reg_temp, alignment * 8);
  }

  generate_function_return_swap_delay();   // Return. Move prev inst to delay slot
  *tr_ptr = translation_ptr;
}

// Generates the stub to store memory for a given region and size
// Handles "special" cases like weirdly mapped memory
static void emit_pmemst_stub(
  unsigned memop_number, const t_stub_meminfo *meminfo,
  unsigned size, bool aligned, u8 **tr_ptr)
{
  u8 *translation_ptr = *tr_ptr;
  unsigned region = meminfo->region;
  u32 base_addr = meminfo->baseptr;

  // Palette, VRAM and OAM cannot be really byte accessed (use a 16 bit store)
  bool doubleaccess = (size == 0 && meminfo->bus16);
  unsigned realsize = size;
  if (doubleaccess)
    realsize = 1;

  // Clean up one or two bits (to align access). It might already be aligned!
  u32 memmask = (meminfo->memsize - 1);
  memmask = (memmask >> realsize) << realsize;

  // Add the stub to the table (add the JAL instruction encoded already)
  tmemst[memop_number][region] = (u32)translation_ptr;

  // First check we are in the right memory region (same as loads)
  mips_emit_srl(reg_temp, reg_a0, 24);
  mips_emit_xori(reg_temp, reg_temp, region);
  mips_emit_b(bne, reg_zero, reg_temp, st_phndlr_branch(memop_number));

  mips_emit_lui(reg_rv, ((base_addr + 0x8000) >> 16));

  if (doubleaccess) {
    double_byte(reg_a1, reg_temp);        // value = value | (value << 8)
  }

  if (region == 2) {
    // Can't do EWRAM with an `andi` instruction (18 bits mask)
    extract_bits(reg_a0, reg_a0, 0, 18);       // &= 0x3ffff
    if (!aligned && realsize != 0) {
      emit_align_reg(reg_a0, size);            // addr & ~1/2 (align to size)
    }
    // Need to insert a zero in the addr (due to how it's mapped)
    mips_emit_addu(reg_rv, reg_rv, reg_a0);    // Adds to the base addr
  } else if (region == 6) {
    // VRAM is mirrored every 128KB but the last 32KB is mapped to the previous
    extract_bits(reg_temp, reg_a0, 15, 2);     // Extract bits 15 and 16
    mips_emit_addiu(reg_temp, reg_temp, -3);   // Check for 3 (last block)
    if (!aligned && realsize != 0) {
      emit_align_reg(reg_a0, realsize);        // addr & ~1/2 (align to size)
    }
    extract_bits(reg_a0, reg_a0, 0, 17);       // addr & 0x1FFFF [delay]
    mips_emit_b(bne, reg_zero, reg_temp, 1);   // Skip next inst unless last block
    generate_swap_delay();
    mips_emit_addiu(reg_a0, reg_a0, 0x8000);   // addr - 0x8000 (mirror last block)
    mips_emit_addu(reg_rv, reg_rv, reg_a0);    // addr = base + adjusted offset
  } else {
    // Generate regular (<=32KB) mirroring
    mips_emit_andi(reg_a0, reg_a0, memmask);   // Clear upper bits (mirroring)
    mips_emit_addu(reg_rv, reg_rv, reg_a0);    // Adds to base addr
  }

  // Generate SMC write and tracking
  // TODO: Should we have SMC checks here also for aligned?
  if (meminfo->check_smc && !aligned) {
    if (region == 2) {
      mips_emit_lui(reg_temp, 0x40000 >> 16);
      mips_emit_addu(reg_temp, reg_rv, reg_temp); // SMC lives after the ewram
    } else {
      mips_emit_addiu(reg_temp, reg_rv, 0x8000); // -32KB is the addr of the SMC buffer
    }
    if (realsize == 2) {
      mips_emit_lw(reg_temp, reg_temp, base_addr);
    } else if (realsize == 1) {
      mips_emit_lh(reg_temp, reg_temp, base_addr);
    } else {
      mips_emit_lb(reg_temp, reg_temp, base_addr);
    }
    // If the data is non zero, we just wrote over code
    // Local-jump to the smc_write (which lives at offset:0)
    mips_emit_b(bne, reg_zero, reg_temp, branch_offset(&rom_translation_cache[SMC_WRITE_OFF]));
  }

  // Store the data (delay slot from the SMC branch)
  if (realsize == 2) {
    mips_emit_sw(reg_a1, reg_rv, base_addr);
  } else if (realsize == 1) {
    mips_emit_sh(reg_a1, reg_rv, base_addr);
  } else {
    mips_emit_sb(reg_a1, reg_rv, base_addr);
  }

  // Post processing store:
  // Signal that OAM was updated
  if (region == 7) {
    // Write any nonzero data
    mips_emit_sw(reg_base, reg_base, ReOff_OamUpd);
    generate_function_return_swap_delay();
  }
  else {
    mips_emit_jr(mips_reg_ra);
    mips_emit_nop();
  }

  *tr_ptr = translation_ptr;
}

// Palette conversion functions. a1 contains the palette value (16 LSB)
// Places the result in reg_temp, can use a0 as temporary register
#if defined(USE_XBGR1555_FORMAT)
  /* PS2's native format */
  #define palette_convert()                       \
    mips_emit_andi(reg_temp, reg_a1, 0x7FFF);
#else
  /* 0BGR to RGB565 (clobbers a0) */
  #ifdef MIPS_HAS_R2_INSTS
    #define palette_convert()                       \
      mips_emit_ext(reg_temp, reg_a1, 10, 5);       \
      mips_emit_ins(reg_temp, reg_a1, 11, 5);       \
      mips_emit_ext(reg_a0, reg_a1, 5, 5);          \
      mips_emit_ins(reg_temp, reg_a0, 6, 5);
  #else
    #define palette_convert()                       \
      mips_emit_srl(reg_a0, reg_a1, 10);            \
      mips_emit_andi(reg_temp, reg_a0, 0x1F);       \
      mips_emit_sll(reg_a0, reg_a1, 1);             \
      mips_emit_andi(reg_a0, reg_a0, 0x7C0);        \
      mips_emit_or(reg_temp, reg_temp, reg_a0);     \
      mips_emit_andi(reg_a0, reg_a1, 0x1F);         \
      mips_emit_sll(reg_a0, reg_a0, 11);            \
      mips_emit_or(reg_temp, reg_temp, reg_a0);
  #endif
#endif

// Palette is accessed differently and stored in a decoded manner
static void emit_palette_hdl(
  unsigned memop_number, const t_stub_meminfo *meminfo,
  unsigned size, bool aligned, u8 **tr_ptr)
{
  u8 *translation_ptr = *tr_ptr;

  // Palette cannot be accessed at byte level
  unsigned realsize = size ? size : 1;
  u32 memmask = (meminfo->memsize - 1);
  memmask = (memmask >> realsize) << realsize;

  // Add the stub to the table (add the JAL instruction encoded already)
  tmemst[memop_number][5] = (u32)translation_ptr;

  // First check we are in the right memory region (same as loads)
  mips_emit_srl(reg_temp, reg_a0, 24);
  mips_emit_xori(reg_temp, reg_temp, 5);
  mips_emit_b(bne, reg_zero, reg_temp, st_phndlr_branch(memop_number));
  mips_emit_andi(reg_rv, reg_a0, memmask);   // Clear upper bits (mirroring)
  if (size == 0) {
    double_byte(reg_a1, reg_temp);    // value = value | (value << 8)
  }
  mips_emit_addu(reg_rv, reg_rv, reg_base);

  // Store the data in real palette memory
  if (realsize == 2) {
    mips_emit_sw(reg_a1, reg_rv, 0x100);
  } else if (realsize == 1) {
    mips_emit_sh(reg_a1, reg_rv, 0x100);
  }

  // Convert and store in mirror memory
  palette_convert();
  mips_emit_sh(reg_temp, reg_rv, 0x500);

  if (size == 2) {
    // Convert the second half-word also
    mips_emit_srl(reg_a1, reg_a1, 16);
    palette_convert();
    mips_emit_sh(reg_temp, reg_rv, 0x502);
  }
  generate_function_return_swap_delay();

  *tr_ptr = translation_ptr;
}

// This emits stubs for regions where writes have no side-effects
static void emit_ignorestore_stub(unsigned size, u8 **tr_ptr) {
  u8 *translation_ptr = *tr_ptr;

  // Region 0-1 (BIOS and ignore)
  tmemst[size][0] = tmemst[size][1] = (u32)translation_ptr;
  mips_emit_srl(reg_temp, reg_a0, 25);               // Check 7 MSB to be zero
  mips_emit_b(bne, reg_temp, reg_zero, st_phndlr_branch(size));
  mips_emit_nop();
  mips_emit_jr(mips_reg_ra);
  mips_emit_nop();

  // Region 9-C
  tmemst[size][ 9] = tmemst[size][10] =
  tmemst[size][11] = tmemst[size][12] = (u32)translation_ptr;

  mips_emit_srl(reg_temp, reg_a0, 24);
  mips_emit_addiu(reg_temp, reg_temp, -9);
  mips_emit_srl(reg_temp, reg_temp, 2);
  mips_emit_b(bne, reg_temp, reg_zero, st_phndlr_branch(size));
  mips_emit_nop();
  mips_emit_jr(mips_reg_ra);
  mips_emit_nop();

  // Region F or higher
  tmemst[size][15] = (u32)translation_ptr;
  mips_emit_srl(reg_temp, reg_a0, 24);
  mips_emit_sltiu(reg_rv, reg_temp, 0x0F);  // Is < 15?
  mips_emit_b(bne, reg_rv, reg_zero, st_phndlr_branch(size));
  mips_emit_nop();
  mips_emit_jr(mips_reg_ra);
  mips_emit_nop();

  *tr_ptr = translation_ptr;
}

// Stubs for regions with EEPROM or flash/SRAM (also RTC)
static void emit_saveaccess_stub(u8 **tr_ptr) {
  unsigned opt, i, strop;
  u8 *translation_ptr = *tr_ptr;

  // Writes to region 8 are directed to RTC (only 16 bit ones though)
  tmemld[1][8] = (u32)translation_ptr;
  emit_mem_call(&write_gpio, 0xFE);

  // These are for region 0xD where EEPROM is mapped. Addr is ignored
  // Value is limited to one bit (both reading and writing!)
  u32 *read_hndlr = (u32*)translation_ptr;
  emit_mem_call(&read_eeprom, 0x3FF);
  u32 *write_hndlr = (u32*)translation_ptr;
  emit_mem_call(&write_eeprom, 0x3FF);

  // Map loads to the read handler.
  for (opt = 0; opt < 6; opt++) {
    // Unalignment is not relevant here, so map them all to the same handler.
    for (i = ldopmap[opt][0]; i < ldopmap[opt][1]; i++)
      tmemld[i][13] = (u32)translation_ptr;
    // Emit just a check + patch jump
    mips_emit_srl(reg_temp, reg_a0, 24);
    mips_emit_xori(reg_rv, reg_temp, 0x0D);
    mips_emit_b(bne, reg_rv, reg_zero, branch_handlerid(opt));
    mips_emit_nop();
    mips_emit_b(beq, reg_zero, reg_zero, branch_offset(read_hndlr));
  }
  // This is for stores
  for (strop = 0; strop <= 3; strop++) {
    tmemst[strop][13] = (u32)translation_ptr;
    mips_emit_srl(reg_temp, reg_a0, 24);
    mips_emit_xori(reg_rv, reg_temp, 0x0D);
    mips_emit_b(bne, reg_rv, reg_zero, st_phndlr_branch(strop));
    mips_emit_nop();
    mips_emit_b(beq, reg_zero, reg_zero, branch_offset(write_hndlr));
  }
  
  // Flash/SRAM/Backup writes are only 8 byte supported
  for (strop = 0; strop <= 3; strop++) {
    tmemst[strop][14] = (u32)translation_ptr;
    mips_emit_srl(reg_temp, reg_a0, 24);
    mips_emit_xori(reg_rv, reg_temp, 0x0E);
    mips_emit_b(bne, reg_rv, reg_zero, st_phndlr_branch(strop));
    if (strop == 0) {
      emit_mem_call(&write_backup, 0xFFFF);
    } else {
      mips_emit_nop();
      mips_emit_jr(mips_reg_ra);   // Does nothing in this case
      mips_emit_nop();
    }
  }

  // RTC writes, only for 16 bit accesses
  for (strop = 0; strop <= 3; strop++) {
    tmemst[strop][8] = (u32)translation_ptr;
    mips_emit_srl(reg_temp, reg_a0, 24);
    mips_emit_xori(reg_rv, reg_temp, 0x08);
    mips_emit_b(bne, reg_rv, reg_zero, st_phndlr_branch(strop));
    if (strop == 1) {
      emit_mem_call(&write_gpio, 0xFF);  // Addr
    } else {
      mips_emit_nop();
      mips_emit_jr(mips_reg_ra);   // Do nothing
      mips_emit_nop();
    }
  }

  // Region 4 writes  
  // I/O writes are also a bit special, they can trigger things like DMA, IRQs...
  // Also: aligned (strop==3) accesses do not trigger IRQs
  const u32 iowrtbl[] = {
    (u32)&write_io_register8, (u32)&write_io_register16,
    (u32)&write_io_register32, (u32)&write_io_register32 };
  const u32 amsk[] = {0x3FF, 0x3FE, 0x3FC, 0x3FC};
  for (strop = 0; strop <= 3; strop++) {
    tmemst[strop][4] = (u32)translation_ptr;
    mips_emit_srl(reg_temp, reg_a0, 24);
    mips_emit_xori(reg_temp, reg_temp, 0x04);
    mips_emit_b(bne, reg_zero, reg_temp, st_phndlr_branch(strop));

    mips_emit_sw(mips_reg_ra, reg_base, ReOff_SaveR3); // Store the return addr
    emit_save_regs(strop == 3);
    mips_emit_andi(reg_a0, reg_a0, amsk[strop]);
    genccall(iowrtbl[strop]);

    if (strop < 3) {
      mips_emit_sw(reg_a2, reg_base, ReOff_RegPC);   // Save PC (delay)
      // If I/O writes returns non-zero, means we need to process side-effects.
      mips_emit_b(bne, reg_zero, reg_rv, branch_offset(&rom_translation_cache[IOEPILOGUE_OFF]));
      mips_emit_lw(mips_reg_ra, reg_base, ReOff_SaveR3);   // (in delay slot but not used)
      emit_restore_regs(false);
    } else {
      mips_emit_nop();
      mips_emit_lw(mips_reg_ra, reg_base, ReOff_SaveR3);
      emit_restore_regs(true);
    }
    generate_function_return_swap_delay();
  }

  *tr_ptr = translation_ptr;
}

// Emits openload stub
// These are used for reading unmapped regions, we just make them go
// through the slow handler since should rarely happen.
static void emit_openload_stub(unsigned opt, bool signext, unsigned size, u8 **tr_ptr) {
  int i;
  const u32 hndreadtbl[] = {
    (u32)&read_memory8,  (u32)&read_memory16,  (u32)&read_memory32,
    (u32)&read_memory8s, (u32)&read_memory16s, (u32)&read_memory32 };
  u8 *translation_ptr = *tr_ptr;

  // This affects regions 1 and 15
  for (i = ldopmap[opt][0]; i < ldopmap[opt][1]; i++)
    tmemld[i][ 1] = tmemld[i][15] = (u32)translation_ptr;

  // Alignment is ignored since the handlers do the magic for us
  // Only check region match: if we are accessing a non-ignore region
  mips_emit_srl(reg_temp, reg_a0, 24);
  mips_emit_sltiu(reg_rv, reg_temp, 0x0F);
  mips_emit_addiu(reg_temp, reg_temp, -1);
  mips_emit_sltu(reg_temp, reg_zero, reg_temp);
  mips_emit_and(reg_temp, reg_temp, reg_rv);

  // Jump to patch handler
  mips_emit_b(bne, reg_zero, reg_temp, branch_handlerid(opt));

  // BIOS can jump here to do open loads
  for (i = ldopmap[opt][0]; i < ldopmap[opt][1]; i++)
    openld_core_ptrs[i] = (u32*)translation_ptr;

  emit_save_regs(true);
  mips_emit_sw(mips_reg_ra, reg_base, ReOff_SaveR1);   // Delay slot
  genccall(hndreadtbl[size + (signext ? 3 : 0)]);
  if (opt < 5) {
    mips_emit_sw(reg_a1, reg_base, ReOff_RegPC);       // Save current PC
  } else {
    // Aligned loads do not hold PC in a1 (imprecision)
    mips_emit_nop();
  }

  mips_emit_lw(mips_reg_ra, reg_base, ReOff_SaveR1);
  emit_restore_regs(true);
  generate_function_return_swap_delay();

  *tr_ptr = translation_ptr;
}

typedef void (*sthldr_t)(
  unsigned memop_number, const t_stub_meminfo *meminfo,
  unsigned size, bool aligned, u8 **tr_ptr);
  
typedef void (*ldhldr_t)(
  unsigned memop_number, const t_stub_meminfo *meminfo,
  bool signext, unsigned size,
  unsigned alignment, bool aligned, bool must_swap,
  u8 **tr_ptr);

// Generates a patch handler for a given access size
// It will detect the access alignment and memory region and load
// the corresponding handler from the table (at the right offset)
// and patch the jal instruction from where it was called.
static void emit_phand(
  u8 **tr_ptr, unsigned size, unsigned toff,
  bool check_alignment)
{
  u8 *translation_ptr = *tr_ptr;

  mips_emit_srl(reg_temp, reg_a0, 24);
  #ifdef PSP
    mips_emit_addiu(reg_rv, reg_zero, 15*4);  // Table limit (max)
    mips_emit_sll(reg_temp, reg_temp, 2);     // Table is word indexed
    mips_emit_min(reg_temp, reg_temp, reg_rv);// Do not overflow table
  #else
    mips_emit_sltiu(reg_rv, reg_temp, 0x0F);  // Check for addr 0x1XXX.. 0xFXXX
    mips_emit_sll(reg_temp, reg_temp, 2);     // Table is word indexed
    mips_emit_b(bne, reg_zero, reg_rv, 1);    // Skip next inst if region is good
    generate_swap_delay();
    mips_emit_addiu(reg_temp, reg_zero, 15*4);// Simulate ld/st to 0x0FXXX (open/ignore)
  #endif

  // Stores or byte-accesses do not care about alignment
  if (check_alignment) {
    // Move alignment bits for the table lookup (1 or 2, to bits 6 and 7)
    insert_bits(reg_temp, reg_a0, reg_rv, 6, size);
  }

  unsigned tbloff = 256 + 3*1024 + 220 + 4 * toff;  // Skip regs and RAMs
  unsigned tbloff2 = tbloff + 960;              // JAL opcode table
  mips_emit_addu(reg_temp, reg_temp, reg_base); // Add to the base_reg the table offset
  mips_emit_lw(reg_rv,   reg_temp, tbloff);     // Get func addr from 1st table
  mips_emit_lw(reg_temp, reg_temp, tbloff2);    // Get opcode from 2nd table
  mips_emit_sw(reg_temp, mips_reg_ra, -8);      // Patch instruction!

  #if defined(PSP)
    mips_emit_cache(0x1A, mips_reg_ra, -8);
    mips_emit_jr(reg_rv);                       // Jump directly to target for speed
    mips_emit_cache(0x08, mips_reg_ra, -8);
  #else
    mips_emit_jr(reg_rv);
    #ifdef MIPS_HAS_R2_INSTS
      mips_emit_synci(mips_reg_ra, -8);
    #endif
  #endif

  // Round up handlers to 16 instructions for easy addressing
  // PSP/MIPS32r2 uses up to 12 insts
  while (translation_ptr - *tr_ptr < 64) {
    mips_emit_nop();
  }

  *tr_ptr = translation_ptr;
}

// This function emits the following stubs:
// - smc_write: Jumps to C code to trigger a cache flush
// - memop patcher: Patches a memop whenever it accesses the wrong mem region
// - mem stubs: There's stubs for load & store, and every memory region
//    and possible operand size and misaligment (+sign extensions)
void init_emitter(bool must_swap) {
  int i;
  // Initialize memory to a debuggable state
  rom_cache_watermark = INITIAL_ROM_WATERMARK;

  // Generates the trampoline and helper stubs that we need
  u8 *translation_ptr = (u8*)&rom_translation_cache[0];

  // Generate first the patch handlers
  // We have 6+4 patchers, one per mem type (6 or 4)

  // Calculate the offset into tmemld[10][XX];
  emit_phand(&translation_ptr, 0,  0 * 16, false);  // ld u8
  emit_phand(&translation_ptr, 0,  1 * 16, false);  // ld s8
  emit_phand(&translation_ptr, 1,  2 * 16, true);   // ld u16 + u16u1
  emit_phand(&translation_ptr, 1,  4 * 16, true);   // ld s16 + s16u1
  emit_phand(&translation_ptr, 2,  6 * 16, true);   // ld u32 (0/1/2/3u)
  emit_phand(&translation_ptr, 2, 10 * 16, false);  // ld aligned 32
  // Store table is immediately after
  emit_phand(&translation_ptr, 0, 11 * 16, false);  // st u8
  emit_phand(&translation_ptr, 1, 12 * 16, false);  // st u16
  emit_phand(&translation_ptr, 2, 13 * 16, false);  // st u32
  emit_phand(&translation_ptr, 2, 14 * 16, false);  // st aligned 32

  // Trampoline area
  mips_emit_j(((u32)&smc_write) >> 2);
  mips_emit_nop();

  mips_emit_j(((u32)&write_io_epilogue) >> 2);
  mips_emit_nop();

  // Special trampoline for SP-relative ldm/stm (to EWRAM)
  generate_load_imm(reg_a1, 0x3FFFC);
  mips_emit_and(reg_a1, reg_a1, reg_a2);
  mips_emit_lui(reg_a0, ((u32)(ewram + 0x8000) >> 16));
  generate_function_return_swap_delay();

  // Generate the openload handlers (for accesses to unmapped mem)
  emit_openload_stub(0, false, 0, &translation_ptr);  // ld u8
  emit_openload_stub(1, true,  0, &translation_ptr);  // ld s8
  emit_openload_stub(2, false, 1, &translation_ptr);  // ld u16
  emit_openload_stub(3, true,  1, &translation_ptr);  // ld s16
  emit_openload_stub(4, false, 2, &translation_ptr);  // ld u32
  emit_openload_stub(5, false, 2, &translation_ptr);  // ld a32

  // Here we emit the ignore store area, just checks and does nothing
  for (i = 0; i < 4; i++)
    emit_ignorestore_stub(i, &translation_ptr);

  // Here go the save game handlers
  emit_saveaccess_stub(&translation_ptr);

  // Generate memory handlers
  const t_stub_meminfo ldinfo [] = {
    { emit_pmemld_stub,  0, 0x4000, false, false, (u32)bios_rom, 0},
    // 1 Open load / Ignore store
    { emit_pmemld_stub,  2, 0x8000, true,  false, (u32)ewram, 0 },      // memsize wrong on purpose
    { emit_pmemld_stub,  3, 0x8000, true,  false, (u32)&iwram[0x8000], 0 },
    { emit_pmemld_stub,  4,  0x400, false, false, (u32)io_registers, 0 },
    { emit_pmemld_stub,  5,  0x400, false, true,  (u32)palette_ram, 0x100 },
    { emit_pmemld_stub,  6,    0x0, false, true,  (u32)vram, 0 },             // same, vram is a special case
    { emit_pmemld_stub,  7,  0x400, false, true,  (u32)oam_ram, 0x900 },
    { emit_pmemld_stub,  8, 0x8000, false, false,  0, 0 },
    { emit_pmemld_stub,  9, 0x8000, false, false,  0, 0 },
    { emit_pmemld_stub, 10, 0x8000, false, false,  0, 0 },
    { emit_pmemld_stub, 11, 0x8000, false, false,  0, 0 },
    { emit_pmemld_stub, 12, 0x8000, false, false,  0, 0 },
    // 13 is EEPROM mapped already (a bit special)
    { emit_pmemld_stub, 14,      0, false, false,  0, 0 },                    // Mapped via function call
    // 15 Open load / Ignore store
  };

  for (i = 0; i < sizeof(ldinfo)/sizeof(ldinfo[0]); i++) {
    ldhldr_t handler = (ldhldr_t)ldinfo[i].emitter;
    /*          region  info      signext sz al  isaligned */
    handler(0, &ldinfo[i], false, 0, 0, false, must_swap, &translation_ptr);  // ld u8
    handler(1, &ldinfo[i], true,  0, 0, false, must_swap, &translation_ptr);  // ld s8

    handler(2, &ldinfo[i], false, 1, 0, false, must_swap, &translation_ptr);  // ld u16
    handler(3, &ldinfo[i], false, 1, 1, false, must_swap, &translation_ptr);  // ld u16u1
    handler(4, &ldinfo[i], true,  1, 0, false, must_swap, &translation_ptr);  // ld s16
    handler(5, &ldinfo[i], true,  1, 1, false, must_swap, &translation_ptr);  // ld s16u1

    handler(6, &ldinfo[i], false, 2, 0, false, must_swap, &translation_ptr);  // ld u32
    handler(7, &ldinfo[i], false, 2, 1, false, must_swap, &translation_ptr);  // ld u32u1
    handler(8, &ldinfo[i], false, 2, 2, false, must_swap, &translation_ptr);  // ld u32u2
    handler(9, &ldinfo[i], false, 2, 3, false, must_swap, &translation_ptr);  // ld u32u3

    handler(10,&ldinfo[i], false, 2, 0, true,  must_swap, &translation_ptr);  // aligned ld u32
  }

  const t_stub_meminfo stinfo [] = {
    { emit_pmemst_stub, 2, 0x8000, true,  false, (u32)ewram, 0 },
    { emit_pmemst_stub, 3, 0x8000, true,  false, (u32)&iwram[0x8000], 0 },
    // I/O is special and mapped with a function call
    { emit_palette_hdl, 5,  0x400, false, true,  (u32)palette_ram, 0x100 },
    { emit_pmemst_stub, 6,    0x0, false, true,  (u32)vram, 0 },          // same, vram is a special case
    { emit_pmemst_stub, 7,  0x400, false, true,  (u32)oam_ram, 0x900 },
  };

  // Store only for "regular"-ish mem regions
  //
  for (i = 0; i < sizeof(stinfo)/sizeof(stinfo[0]); i++) {
    sthldr_t handler = (sthldr_t)stinfo[i].emitter;
    handler(0, &stinfo[i], 0, false, &translation_ptr);  // st u8
    handler(1, &stinfo[i], 1, false, &translation_ptr);  // st u16
    handler(2, &stinfo[i], 2, false, &translation_ptr);  // st u32
    handler(3, &stinfo[i], 2, true,  &translation_ptr);  // st aligned 32
  }

  // Generate JAL tables
  u32 *tmemptr = &tmemld[0][0];
  for (i = 0; i < 15*16; i++)
    thnjal[i] = ((tmemptr[i] >> 2) & 0x3FFFFFF) | (mips_opcode_jal << 26);

  // Ensure rom flushes do not wipe this area
  rom_cache_watermark = (u32)(translation_ptr - rom_translation_cache);

  init_bios_hooks();
}

u32 execute_arm_translate_internal(u32 cycles, void *regptr);
u32 execute_arm_translate(u32 cycles) {
  return execute_arm_translate_internal(cycles, &reg[0]);
}

#endif


