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

#ifndef ARM_EMIT_H
#define ARM_EMIT_H

#include "arm_codegen.h"

void generate_indirect_branch_arm(void);
u32 arm_prepare_load_reg(u8 **tptr, u32 scratch_reg, u32 reg_index);
u32 arm_prepare_load_reg_pc(u8 **tptr, u32 scratch_reg, u32 reg_index, u32 pc_offset);
u32 arm_prepare_store_reg(u32 scratch_reg, u32 reg_index);
u32 thumb_prepare_load_reg(u8 **tptr, u32 scratch_reg, u32 reg_index);
u32 thumb_prepare_load_reg_pc(u8 **tptr, u32 scratch_reg, u32 reg_index, u32 pc_offset);
u32 thumb_prepare_store_reg(u32 scratch_reg, u32 reg_index);
void thumb_cheat_hook(void);
void arm_cheat_hook(void);

u32 arm_update_gba_arm(u32 pc);
u32 arm_update_gba_thumb(u32 pc);
u32 arm_update_gba_idle_arm(u32 pc);
u32 arm_update_gba_idle_thumb(u32 pc);

/* Although these are defined as a function, don't call them as
 * such (jump to it instead) */
void arm_indirect_branch_arm(u32 address);
void arm_indirect_branch_thumb(u32 address);
void arm_indirect_branch_dual_arm(u32 address);
void arm_indirect_branch_dual_thumb(u32 address);

void execute_store_cpsr(u32 new_cpsr);
u32 execute_store_cpsr_body(u32 _cpsr, u32 store_mask, u32 address);
u32 execute_spsr_restore(u32 address);

void execute_swi_arm(u32 pc);
void execute_swi_thumb(u32 pc);

#define armfn_gbaup_idle_arm       0
#define armfn_gbaup_idle_thumb     1
#define armfn_gbaup_arm            2
#define armfn_gbaup_thumb          3
#define armfn_swi_arm              4
#define armfn_swi_thumb            5
#define armfn_cheat_arm            6
#define armfn_cheat_thumb          7
#define armfn_store_cpsr           8
#define armfn_spsr_restore         9
#define armfn_indirect_arm        10
#define armfn_indirect_thumb      11
#define armfn_indirect_dual_arm   12
#define armfn_indirect_dual_thumb 13
#define armfn_debug_trace         14

#define STORE_TBL_OFF     0x118
#define SPSR_RAM_OFF      0x100

#define write32(value)                                                        \
  *((u32 *)translation_ptr) = value;                                          \
  translation_ptr += 4                                                        \
  
#define arm_relative_offset(source, offset)                                   \
  (((((u32)offset - (u32)source) - 8) >> 2) & 0xFFFFFF)                       \


#define reg_a0          ARMREG_R0
#define reg_a1          ARMREG_R1
#define reg_a2          ARMREG_R2

/* scratch0 is shared with flags, be careful! */
#define reg_s0          ARMREG_R9
#define reg_base        ARMREG_R11
#define reg_flags       ARMREG_R9

#define reg_cycles      ARMREG_R12

#define reg_rv          ARMREG_R0

#define reg_rm          ARMREG_R0
#define reg_rn          ARMREG_R1
#define reg_rs          ARMREG_R14
#define reg_rd          ARMREG_R0


/* Register allocation layout for ARM and Thumb:
 * Map from a GBA register to a host ARM register. -1 means load it
 * from memory into one of the temp registers.

 * The following registers are chosen based on statistical analysis
 * of a few games (see below), but might not be the best ones. Results
 * vary tremendously between ARM and Thumb (for obvious reasons), so
 * two sets are used. Take care to not call any function which can
 * overwrite any of these registers from the dynarec - only call
 * trusted functions in arm_stub.S which know how to save/restore
 * them and know how to transfer them to the C functions it calls
 * if necessary.

 * The following define the actual registers available for allocation.
 * As registers are freed up add them to this list.

 * Note that r15 is linked to the a0 temp reg - this register will
 * be preloaded with a constant upon read, and used to link to
 * indirect branch functions upon write.
 */

#define reg_x0         ARMREG_R3
#define reg_x1         ARMREG_R4
#define reg_x2         ARMREG_R5
#define reg_x3         ARMREG_R6
#define reg_x4         ARMREG_R7
#define reg_x5         ARMREG_R8

#define mem_reg        (~0U)

/*

ARM register usage (38.775138% ARM instructions):
r00: 18.263814% (-- 18.263814%)
r12: 11.531477% (-- 29.795291%)
r09: 11.500162% (-- 41.295453%)
r14: 9.063440% (-- 50.358893%)
r06: 7.837682% (-- 58.196574%)
r01: 7.401049% (-- 65.597623%)
r07: 6.778340% (-- 72.375963%)
r05: 5.445009% (-- 77.820973%)
r02: 5.427288% (-- 83.248260%)
r03: 5.293743% (-- 88.542003%)
r04: 3.601103% (-- 92.143106%)
r11: 3.207311% (-- 95.350417%)
r10: 2.334864% (-- 97.685281%)
r08: 1.708207% (-- 99.393488%)
r15: 0.311270% (-- 99.704757%)
r13: 0.295243% (-- 100.000000%)

Thumb register usage (61.224862% Thumb instructions):
r00: 34.788858% (-- 34.788858%)
r01: 26.564083% (-- 61.352941%)
r03: 10.983500% (-- 72.336441%)
r02: 8.303127% (-- 80.639567%)
r04: 4.900381% (-- 85.539948%)
r05: 3.941292% (-- 89.481240%)
r06: 3.257582% (-- 92.738822%)
r07: 2.644851% (-- 95.383673%)
r13: 1.408824% (-- 96.792497%)
r08: 0.906433% (-- 97.698930%)
r09: 0.679693% (-- 98.378623%)
r10: 0.656446% (-- 99.035069%)
r12: 0.453668% (-- 99.488737%)
r14: 0.248909% (-- 99.737646%)
r11: 0.171066% (-- 99.908713%)
r15: 0.091287% (-- 100.000000%)

*/

u32 arm_register_allocation[] =
{
  reg_x0,       /* GBA r0  */
  reg_x1,       /* GBA r1  */
  mem_reg,      /* GBA r2  */
  mem_reg,      /* GBA r3  */
  mem_reg,      /* GBA r4  */
  mem_reg,      /* GBA r5  */
  reg_x2,       /* GBA r6  */
  mem_reg,      /* GBA r7  */
  mem_reg,      /* GBA r8  */
  reg_x3,       /* GBA r9  */
  mem_reg,      /* GBA r10 */
  mem_reg,      /* GBA r11 */
  reg_x4,       /* GBA r12 */
  mem_reg,      /* GBA r13 */
  reg_x5,       /* GBA r14 */
  reg_a0,       /* GBA r15 */

  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
};

u32 thumb_register_allocation[] =
{
  reg_x0,       /* GBA r0  */
  reg_x1,       /* GBA r1  */
  reg_x2,       /* GBA r2  */
  reg_x3,       /* GBA r3  */
  reg_x4,       /* GBA r4  */
  reg_x5,       /* GBA r5  */
  mem_reg,      /* GBA r6  */
  mem_reg,      /* GBA r7  */
  mem_reg,      /* GBA r8  */
  mem_reg,      /* GBA r9  */
  mem_reg,      /* GBA r10 */
  mem_reg,      /* GBA r11 */
  mem_reg,      /* GBA r12 */
  mem_reg,      /* GBA r13 */
  mem_reg,      /* GBA r14 */
  reg_a0,       /* GBA r15 */

  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
  mem_reg,
};

#define arm_imm_lsl_to_rot(value)                                             \
  (32 - value)                                                                \

u32 arm_disect_imm_32bit(u32 imm, u32 *stores, u32 *rotations)
{
  u32 store_count = 0;
  u32 left_shift = 0;

  /* Otherwise it'll return 0 things to store because it'll never
   * find anything. */
  if(imm == 0)
  {
    rotations[0] = 0;
    stores[0] = 0;
    return 1;
  }

  /* Find chunks of non-zero data at 2 bit alignments. */
  while(1)
  {
    for(; left_shift < 32; left_shift += 2)
    {
      if((imm >> left_shift) & 0x03)
        break;
    }

    /* We've hit the end of the useful data. */
    if(left_shift == 32)
      return store_count;

    /* Hit the end, it might wrap back around to the beginning. */
    if(left_shift >= 24)
    {
      /* Make a mask for the residual bits. IE, if we have
       * 5 bits of data at the end we can wrap around to 3
       * bits of data in the beginning. Thus the first
       * thing, after being shifted left, has to be less
       * than 111b, 0x7, or (1 << 3) - 1.
       */
      u32 top_bits = 32 - left_shift;
      u32 residual_bits = 8 - top_bits;
      u32 residual_mask = (1 << residual_bits) - 1;

      if((store_count > 1) && (left_shift > 24) &&
       ((stores[0] << ((32 - rotations[0]) & 0x1F)) < residual_mask))
      {
        /* Then we can throw out the last bit and tack it on
         * to the first bit. */
        stores[0] =
         (stores[0] << ((top_bits + (32 - rotations[0])) & 0x1F)) |
         ((imm >> left_shift) & 0xFF);
        rotations[0] = top_bits;

        return store_count;
      }
      else
      {
        /* There's nothing to wrap over to in the beginning */
        stores[store_count] = (imm >> left_shift) & 0xFF;
        rotations[store_count] = (32 - left_shift) & 0x1F;
        return store_count + 1;
      }
      break;
    }

    stores[store_count] = (imm >> left_shift) & 0xFF;
    rotations[store_count] = (32 - left_shift) & 0x1F;

    store_count++;
    left_shift += 8;
  }
}

#if __ARM_ARCH >= 7
  #define arm_load_imm_32bit(ireg, imm)                                       \
  {                                                                           \
    ARM_MOVW(0, ireg, (imm));                                                 \
    if ((imm) >> 16) {                                                        \
      ARM_MOVT(0, ireg, ((imm) >> 16));                                       \
    }                                                                         \
  }
#else
  #define arm_load_imm_32bit(ireg, imm)                                       \
  {                                                                           \
    u32 stores[4];                                                            \
    u32 rotations[4];                                                         \
    u32 store_count = arm_disect_imm_32bit(imm, stores, rotations);           \
    u32 i;                                                                    \
                                                                              \
    ARM_MOV_REG_IMM(0, ireg, stores[0], rotations[0]);                        \
                                                                              \
    for(i = 1; i < store_count; i++)                                          \
    {                                                                         \
      ARM_ORR_REG_IMM(0, ireg, ireg, stores[i], rotations[i]);                \
    }                                                                         \
  }
#endif


#define generate_load_pc(ireg, new_pc)                                        \
  arm_load_imm_32bit(ireg, (new_pc))                                          \

#define generate_load_imm(ireg, imm, imm_ror)                                 \
  ARM_MOV_REG_IMM(0, ireg, imm, imm_ror)                                      \



#define generate_shift_left(ireg, imm)                                        \
  ARM_MOV_REG_IMMSHIFT(0, ireg, ireg, ARMSHIFT_LSL, imm)                      \

#define generate_shift_right(ireg, imm)                                       \
  ARM_MOV_REG_IMMSHIFT(0, ireg, ireg, ARMSHIFT_LSR, imm)                      \

#define generate_shift_right_arithmetic(ireg, imm)                            \
  ARM_MOV_REG_IMMSHIFT(0, ireg, ireg, ARMSHIFT_ASR, imm)                      \

#define generate_rotate_right(ireg, imm)                                      \
  ARM_MOV_REG_IMMSHIFT(0, ireg, ireg, ARMSHIFT_ROR, imm)                      \

#define generate_add(ireg_dest, ireg_src)                                     \
  ARM_ADD_REG_REG(0, ireg_dest, ireg_dest, ireg_src)                          \

#define generate_sub(ireg_dest, ireg_src)                                     \
  ARM_SUB_REG_REG(0, ireg_dest, ireg_dest, ireg_src)                          \

#define generate_or(ireg_dest, ireg_src)                                      \
  ARM_ORR_REG_REG(0, ireg_dest, ireg_dest, ireg_src)                          \

#define generate_xor(ireg_dest, ireg_src)                                     \
  ARM_EOR_REG_REG(0, ireg_dest, ireg_dest, ireg_src)                          \

#define generate_add_imm(ireg, imm, imm_ror)                                  \
  ARM_ADD_REG_IMM(0, ireg, ireg, imm, imm_ror)                                \

#define generate_sub_imm(ireg, imm, imm_ror)                                  \
  ARM_SUB_REG_IMM(0, ireg, ireg, imm, imm_ror)                                \

#define generate_xor_imm(ireg, imm, imm_ror)                                  \
  ARM_EOR_REG_IMM(0, ireg, ireg, imm, imm_ror)                                \

#define generate_add_reg_reg_imm(ireg_dest, ireg_src, imm, imm_ror)           \
  ARM_ADD_REG_IMM(0, ireg_dest, ireg_src, imm, imm_ror)                       \

#define generate_and_imm(ireg, imm, imm_ror)                                  \
  ARM_AND_REG_IMM(0, ireg, ireg, imm, imm_ror)                                \

#define generate_mov(ireg_dest, ireg_src)                                     \
  if(ireg_dest != ireg_src)                                                   \
  {                                                                           \
    ARM_MOV_REG_REG(0, ireg_dest, ireg_src);                                  \
  }                                                                           \

/* Calls functions present in the rom/ram cache (near) */
#define generate_function_call(function_location)                             \
  ARM_BL(0, arm_relative_offset(translation_ptr, function_location))          \

/* Calls functions that might be far, via the function table at reg_base */
#define generate_function_far_call(function_number)                           \
  generate_load_memreg(ARMREG_LR, function_number + (u32)REG_USERDEF);        \
  ARM_BLX(0, ARMREG_LR)                                                       \

/* The branch target is to be filled in later (thus a 0 for now) */

#define generate_branch_filler(condition_code, writeback_location)            \
  (writeback_location) = translation_ptr;                                     \
  ARM_B_COND(0, condition_code, 0)                                            \

#define generate_update_pc(new_pc)                                            \
  generate_load_pc(reg_a0, new_pc)                                            \

#define generate_cycle_update()                                               \
  if(cycle_count)                                                             \
  {                                                                           \
    if(cycle_count >> 8)                                                      \
    {                                                                         \
      ARM_ADD_REG_IMM(0, reg_cycles, reg_cycles, (cycle_count >> 8) & 0xFF,   \
       arm_imm_lsl_to_rot(8));                                                \
    }                                                                         \
    ARM_ADD_REG_IMM(0, reg_cycles, reg_cycles, (cycle_count & 0xFF), 0);      \
    cycle_count = 0;                                                          \
  }                                                                           \

#define generate_branch_patch_conditional(dest, offset)                       \
  *((u32 *)(dest)) = (*((u32 *)dest) & 0xFF000000) |                          \
   arm_relative_offset(dest, offset)                                          \


#define generate_branch_patch_unconditional(dest, offset)                     \
  *((u32 *)(dest)) = (*((u32 *)dest) & 0xFF000000) |                          \
   arm_relative_offset(dest, offset)                                          \

/* A different function is called for idle updates because of the relative
 * location of the embedded PC. The idle version could be optimized to put
 * the CPU into halt mode too, however.
 */

#define generate_branch_idle_eliminate(writeback_location, new_pc, mode)      \
  generate_function_far_call(armfn_gbaup_idle_##mode);                        \
  write32(new_pc);                                                            \
  generate_branch_filler(ARMCOND_AL, writeback_location)                      \

#define generate_branch_update(writeback_location, new_pc, mode)              \
  ARM_MOV_REG_IMMSHIFT(0, reg_a0, reg_cycles, ARMSHIFT_LSR, 31);              \
  /* If counter is negative, skip the update call (2 insts) */                \
  ARM_ADD_REG_IMMSHIFT(0, ARMREG_PC, ARMREG_PC, reg_a0, ARMSHIFT_LSL, 3);     \
  write32(new_pc);                                                            \
  generate_function_far_call(armfn_gbaup_##mode);   /* 2 instructions */      \
  generate_branch_filler(ARMCOND_AL, writeback_location)                      \


#define generate_branch_no_cycle_update(writeback_location, new_pc, mode)     \
  if(pc == idle_loop_target_pc)                                               \
  {                                                                           \
    generate_branch_idle_eliminate(writeback_location, new_pc, mode);         \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_branch_update(writeback_location, new_pc, mode);                 \
  }                                                                           \

#define generate_branch_cycle_update(writeback_location, new_pc, mode)        \
  generate_cycle_update();                                                    \
  generate_branch_no_cycle_update(writeback_location, new_pc, mode)           \

/* a0 holds the destination */

#define generate_indirect_branch_no_cycle_update(type)                        \
  ARM_LDR_IMM(0, ARMREG_PC, reg_base, 4*(REG_USERDEF + armfn_indirect_##type));

#define generate_indirect_branch_cycle_update(type)                           \
  generate_cycle_update();                                                    \
  generate_indirect_branch_no_cycle_update(type)                              \

#define generate_indirect_branch_arm()                                        \
  {                                                                           \
    if(condition == 0x0E)                                                     \
    {                                                                         \
      generate_cycle_update();                                                \
    }                                                                         \
    generate_indirect_branch_no_cycle_update(arm);                            \
  }                                                                           \

#define generate_indirect_branch_dual()                                       \
  {                                                                           \
    if(condition == 0x0E)                                                     \
    {                                                                         \
      generate_cycle_update();                                                \
    }                                                                         \
    generate_indirect_branch_no_cycle_update(dual_arm);                       \
  }                                                                           \

#define arm_generate_store_reg(ireg, reg_index)                               \
{                                                                             \
  u32 store_dest = arm_register_allocation[reg_index];                        \
  if(store_dest != mem_reg)                                                   \
  {                                                                           \
    ARM_MOV_REG_REG(0, store_dest, ireg);                                     \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    ARM_STR_IMM(0, ireg, reg_base, (reg_index * 4));                          \
  }                                                                           \
}                                                                             \

#define thumb_generate_store_reg(ireg, reg_index)                             \
{                                                                             \
  u32 store_dest = thumb_register_allocation[reg_index];                      \
  if(store_dest != mem_reg)                                                   \
  {                                                                           \
    ARM_MOV_REG_REG(0, store_dest, ireg);                                     \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    ARM_STR_IMM(0, ireg, reg_base, (reg_index * 4));                          \
  }                                                                           \
}

#define generate_load_memreg(ireg, reg_index)                                 \
  ARM_LDR_IMM(0, ireg, reg_base, ((reg_index) * 4));                          \

#define arm_generate_load_reg(ireg, reg_index)                                \
{                                                                             \
  u32 load_src = arm_register_allocation[reg_index];                          \
  if(load_src != mem_reg)                                                     \
  {                                                                           \
    ARM_MOV_REG_REG(0, ireg, load_src);                                       \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    ARM_LDR_IMM(0, ireg, reg_base, ((reg_index) * 4));                        \
  }                                                                           \
}                                                                             \

#define thumb_generate_load_reg(ireg, reg_index)                              \
{                                                                             \
  u32 load_src = thumb_register_allocation[reg_index];                        \
  if(load_src != mem_reg)                                                     \
  {                                                                           \
    ARM_MOV_REG_REG(0, ireg, load_src);                                       \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    ARM_LDR_IMM(0, ireg, reg_base, ((reg_index) * 4));                        \
  }                                                                           \
}                                                                             \

#define arm_generate_load_reg_pc(ireg, reg_index, pc_offset)                  \
  if(reg_index == 15)                                                         \
  {                                                                           \
    generate_load_pc(ireg, pc + pc_offset);                                   \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    arm_generate_load_reg(ireg, reg_index);                                   \
  }                                                                           \

#define thumb_generate_load_reg_pc(ireg, reg_index, pc_offset)                \
  if(reg_index == 15)                                                         \
  {                                                                           \
    generate_load_pc(ireg, pc + pc_offset);                                   \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    thumb_generate_load_reg(ireg, reg_index);                                 \
  }                                                                           \

u32 arm_prepare_store_reg(u32 scratch_reg, u32 reg_index)
{
  u32 reg_use = arm_register_allocation[reg_index];
  if(reg_use == mem_reg)
    return scratch_reg;

  return reg_use;
}

u32 thumb_prepare_store_reg(u32 scratch_reg, u32 reg_index)
{
  u32 reg_use = thumb_register_allocation[reg_index];
  if(reg_use == mem_reg)
    return scratch_reg;

  return reg_use;
}

u32 arm_prepare_load_reg(u8 **tptr, u32 scratch_reg, u32 reg_index)
{
  u32 reg_use = arm_register_allocation[reg_index];
  if(reg_use != mem_reg)
    return reg_use;

  u8 *translation_ptr = *tptr;
  ARM_LDR_IMM(0, scratch_reg, reg_base, (reg_index * 4));
  *tptr = translation_ptr;
  return scratch_reg;
}

u32 arm_prepare_load_reg_pc(u8 **tptr, u32 scratch_reg, u32 reg_index, u32 pc_value)
{
  if(reg_index == 15)
  {
    u8 *translation_ptr = *tptr;
    generate_load_pc(scratch_reg, pc_value);
    *tptr = translation_ptr;
    return scratch_reg;
  }
  return arm_prepare_load_reg(tptr, scratch_reg, reg_index);
}

u32 thumb_prepare_load_reg(u8 **tptr, u32 scratch_reg, u32 reg_index)
{
  u32 reg_use = thumb_register_allocation[reg_index];
  if(reg_use != mem_reg)
    return reg_use;

  u8 *translation_ptr = *tptr;
  ARM_LDR_IMM(0, scratch_reg, reg_base, (reg_index * 4));
  *tptr = translation_ptr;
  return scratch_reg;
}

u32 thumb_prepare_load_reg_pc(u8 **tptr, u32 scratch_reg, u32 reg_index, u32 pc_value)
{
  if(reg_index == 15)
  {
    u8 *translation_ptr = *tptr;
    generate_load_pc(scratch_reg, pc_value);
    *tptr = translation_ptr;
    return scratch_reg;
  }
  return thumb_prepare_load_reg(tptr, scratch_reg, reg_index);
}

#define arm_complete_store_reg(scratch_reg, reg_index)                        \
{                                                                             \
  if(arm_register_allocation[reg_index] == mem_reg)                           \
  {                                                                           \
    ARM_STR_IMM(0, scratch_reg, reg_base, (reg_index * 4));                   \
  }                                                                           \
}

#define thumb_complete_store_reg(scratch_reg, reg_index)                      \
{                                                                             \
  if(thumb_register_allocation[reg_index] == mem_reg)                         \
  {                                                                           \
    ARM_STR_IMM(0, scratch_reg, reg_base, (reg_index * 4));                   \
  }                                                                           \
}

#define block_prologue_size 0
#define generate_block_prologue()
#define generate_block_extra_vars_arm()
#define generate_block_extra_vars_thumb()

#define arm_complete_store_reg_pc_no_flags(scratch_reg, reg_index)            \
{                                                                             \
  if(reg_index == 15)                                                         \
  {                                                                           \
    generate_indirect_branch_arm();                                           \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    arm_complete_store_reg(scratch_reg, reg_index);                           \
  }                                                                           \
}                                                                             \

#define arm_complete_store_reg_pc_flags(scratch_reg, reg_index)               \
{                                                                             \
  if(reg_index == 15)                                                         \
  {                                                                           \
    if(condition == 0x0E)                                                     \
    {                                                                         \
      generate_cycle_update();                                                \
    }                                                                         \
    generate_function_far_call(armfn_spsr_restore);                           \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    arm_complete_store_reg(scratch_reg, reg_index);                           \
  }                                                                           \
}                                                                             \

/* It should be okay to still generate result flags, spsr will overwrite them.
 * This is pretty infrequent (returning from interrupt handlers, et al) so
 * probably not worth optimizing for.
 */

#define check_for_interrupts()                                                \
  if((io_registers[REG_IE] & io_registers[REG_IF]) &&                         \
   io_registers[REG_IME] && ((reg[REG_CPSR] & 0x80) == 0))                    \
  {                                                                           \
    REG_MODE(MODE_IRQ)[6] = pc + 4;                                           \
    REG_SPSR(MODE_IRQ) = reg[REG_CPSR];                                           \
    reg[REG_CPSR] = 0xD2;                                                     \
    pc = 0x00000018;                                                          \
    set_cpu_mode(MODE_IRQ);                                                   \
  }                                                                           \

#define arm_generate_store_reg_pc_no_flags(ireg, reg_index)                   \
  arm_generate_store_reg(ireg, reg_index);                                    \
  if(reg_index == 15)                                                         \
  {                                                                           \
    generate_indirect_branch_arm();                                           \
  }                                                                           \


u32 execute_spsr_restore_body(u32 pc)
{
  set_cpu_mode(cpu_modes[reg[REG_CPSR] & 0xF]);
  check_for_interrupts();

  return pc;
}

#define generate_save_flags()                                                 \
  ARM_MRS_CPSR(0, reg_flags)                                                  \

#define generate_restore_flags()                                              \
  ARM_MSR_REG(0, ARM_PSR_F, reg_flags, ARM_CPSR)                              \

#define condition_opposite_eq ARMCOND_NE
#define condition_opposite_ne ARMCOND_EQ
#define condition_opposite_cs ARMCOND_CC
#define condition_opposite_cc ARMCOND_CS
#define condition_opposite_mi ARMCOND_PL
#define condition_opposite_pl ARMCOND_MI
#define condition_opposite_vs ARMCOND_VC
#define condition_opposite_vc ARMCOND_VS
#define condition_opposite_hi ARMCOND_LS
#define condition_opposite_ls ARMCOND_HI
#define condition_opposite_ge ARMCOND_LT
#define condition_opposite_lt ARMCOND_GE
#define condition_opposite_gt ARMCOND_LE
#define condition_opposite_le ARMCOND_GT
#define condition_opposite_al ARMCOND_NV
#define condition_opposite_nv ARMCOND_AL

#define generate_branch(mode)                                                 \
{                                                                             \
  generate_branch_cycle_update(                                               \
   block_exits[block_exit_position].branch_source,                            \
   block_exits[block_exit_position].branch_target, mode);                     \
  block_exit_position++;                                                      \
}                                                                             \


#define generate_op_and_reg_immshift(_rd, _rn, _rm, shift_type, shift)        \
  ARM_AND_REG_IMMSHIFT(0, _rd, _rn, _rm, shift_type, shift)                   \

#define generate_op_orr_reg_immshift(_rd, _rn, _rm, shift_type, shift)        \
  ARM_ORR_REG_IMMSHIFT(0, _rd, _rn, _rm, shift_type, shift)                   \

#define generate_op_eor_reg_immshift(_rd, _rn, _rm, shift_type, shift)        \
  ARM_EOR_REG_IMMSHIFT(0, _rd, _rn, _rm, shift_type, shift)                   \

#define generate_op_bic_reg_immshift(_rd, _rn, _rm, shift_type, shift)        \
  ARM_BIC_REG_IMMSHIFT(0, _rd, _rn, _rm, shift_type, shift)                   \

#define generate_op_sub_reg_immshift(_rd, _rn, _rm, shift_type, shift)        \
  ARM_SUB_REG_IMMSHIFT(0, _rd, _rn, _rm, shift_type, shift)                   \

#define generate_op_rsb_reg_immshift(_rd, _rn, _rm, shift_type, shift)        \
  ARM_RSB_REG_IMMSHIFT(0, _rd, _rn, _rm, shift_type, shift)                   \

#define generate_op_sbc_reg_immshift(_rd, _rn, _rm, shift_type, shift)        \
  ARM_SBC_REG_IMMSHIFT(0, _rd, _rn, _rm, shift_type, shift)                   \

#define generate_op_rsc_reg_immshift(_rd, _rn, _rm, shift_type, shift)        \
  ARM_RSC_REG_IMMSHIFT(0, _rd, _rn, _rm, shift_type, shift)                   \

#define generate_op_add_reg_immshift(_rd, _rn, _rm, shift_type, shift)        \
  ARM_ADD_REG_IMMSHIFT(0, _rd, _rn, _rm, shift_type, shift)                   \

#define generate_op_adc_reg_immshift(_rd, _rn, _rm, shift_type, shift)        \
  ARM_ADC_REG_IMMSHIFT(0, _rd, _rn, _rm, shift_type, shift)                   \

#define generate_op_mov_reg_immshift(_rd, _rn, _rm, shift_type, shift)        \
  ARM_MOV_REG_IMMSHIFT(0, _rd, _rm, shift_type, shift)                        \

#define generate_op_mvn_reg_immshift(_rd, _rn, _rm, shift_type, shift)        \
  ARM_MVN_REG_IMMSHIFT(0, _rd, _rm, shift_type, shift)                        \


#define generate_op_and_reg_regshift(_rd, _rn, _rm, shift_type, _rs)          \
  ARM_AND_REG_REGSHIFT(0, _rd, _rn, _rm, shift_type, _rs)                     \

#define generate_op_orr_reg_regshift(_rd, _rn, _rm, shift_type, _rs)          \
  ARM_ORR_REG_REGSHIFT(0, _rd, _rn, _rm, shift_type, _rs)                     \

#define generate_op_eor_reg_regshift(_rd, _rn, _rm, shift_type, _rs)          \
  ARM_EOR_REG_REGSHIFT(0, _rd, _rn, _rm, shift_type, _rs)                     \

#define generate_op_bic_reg_regshift(_rd, _rn, _rm, shift_type, _rs)          \
  ARM_BIC_REG_REGSHIFT(0, _rd, _rn, _rm, shift_type, _rs)                     \

#define generate_op_sub_reg_regshift(_rd, _rn, _rm, shift_type, _rs)          \
  ARM_SUB_REG_REGSHIFT(0, _rd, _rn, _rm, shift_type, _rs)                     \

#define generate_op_rsb_reg_regshift(_rd, _rn, _rm, shift_type, _rs)          \
  ARM_RSB_REG_REGSHIFT(0, _rd, _rn, _rm, shift_type, _rs)                     \

#define generate_op_sbc_reg_regshift(_rd, _rn, _rm, shift_type, _rs)          \
  ARM_SBC_REG_REGSHIFT(0, _rd, _rn, _rm, shift_type, _rs)                     \

#define generate_op_rsc_reg_regshift(_rd, _rn, _rm, shift_type, _rs)          \
  ARM_RSC_REG_REGSHIFT(0, _rd, _rn, _rm, shift_type, _rs)                     \

#define generate_op_add_reg_regshift(_rd, _rn, _rm, shift_type, _rs)          \
  ARM_ADD_REG_REGSHIFT(0, _rd, _rn, _rm, shift_type, _rs)                     \

#define generate_op_adc_reg_regshift(_rd, _rn, _rm, shift_type, _rs)          \
  ARM_ADC_REG_REGSHIFT(0, _rd, _rn, _rm, shift_type, _rs)                     \

#define generate_op_mov_reg_regshift(_rd, _rn, _rm, shift_type, _rs)          \
  ARM_MOV_REG_REGSHIFT(0, _rd, _rm, shift_type, _rs)                          \

#define generate_op_mvn_reg_regshift(_rd, _rn, _rm, shift_type, _rs)          \
  ARM_MVN_REG_REGSHIFT(0, _rd, _rm, shift_type, _rs)                          \


#define generate_op_and_imm(_rd, _rn)                                         \
  ARM_AND_REG_IMM(0, _rd, _rn, imm, imm_ror)                                  \

#define generate_op_orr_imm(_rd, _rn)                                         \
  ARM_ORR_REG_IMM(0, _rd, _rn, imm, imm_ror)                                  \

#define generate_op_eor_imm(_rd, _rn)                                         \
  ARM_EOR_REG_IMM(0, _rd, _rn, imm, imm_ror)                                  \

#define generate_op_bic_imm(_rd, _rn)                                         \
  ARM_BIC_REG_IMM(0, _rd, _rn, imm, imm_ror)                                  \

#define generate_op_sub_imm(_rd, _rn)                                         \
  ARM_SUB_REG_IMM(0, _rd, _rn, imm, imm_ror)                                  \

#define generate_op_rsb_imm(_rd, _rn)                                         \
  ARM_RSB_REG_IMM(0, _rd, _rn, imm, imm_ror)                                  \

#define generate_op_sbc_imm(_rd, _rn)                                         \
  ARM_SBC_REG_IMM(0, _rd, _rn, imm, imm_ror)                                  \

#define generate_op_rsc_imm(_rd, _rn)                                         \
  ARM_RSC_REG_IMM(0, _rd, _rn, imm, imm_ror)                                  \

#define generate_op_add_imm(_rd, _rn)                                         \
  ARM_ADD_REG_IMM(0, _rd, _rn, imm, imm_ror)                                  \

#define generate_op_adc_imm(_rd, _rn)                                         \
  ARM_ADC_REG_IMM(0, _rd, _rn, imm, imm_ror)                                  \

#define generate_op_mov_imm(_rd, _rn)                                         \
  ARM_MOV_REG_IMM(0, _rd, imm, imm_ror)                                       \

#define generate_op_mvn_imm(_rd, _rn)                                         \
  ARM_MVN_REG_IMM(0, _rd, imm, imm_ror)                                       \


#define generate_op_reg_immshift_lflags(name, _rd, _rn, _rm, st, shift)       \
  ARM_##name##_REG_IMMSHIFT(0, _rd, _rn, _rm, st, shift)                      \

#define generate_op_reg_immshift_aflags(name, _rd, _rn, _rm, st, shift)       \
  ARM_##name##_REG_IMMSHIFT(0, _rd, _rn, _rm, st, shift)                      \

#define generate_op_reg_immshift_aflags_load_c(name, _rd, _rn, _rm, st, sh)   \
  ARM_##name##_REG_IMMSHIFT(0, _rd, _rn, _rm, st, sh)                         \

#define generate_op_reg_immshift_uflags(name, _rd, _rm, shift_type, shift)    \
  ARM_##name##_REG_IMMSHIFT(0, _rd, _rm, shift_type, shift)                   \

#define generate_op_reg_immshift_tflags(name, _rn, _rm, shift_type, shift)    \
  ARM_##name##_REG_IMMSHIFT(0, _rn, _rm, shift_type, shift)                   \


#define generate_op_reg_regshift_lflags(name, _rd, _rn, _rm, shift_type, _rs) \
  ARM_##name##_REG_REGSHIFT(0, _rd, _rn, _rm, shift_type, _rs)                \

#define generate_op_reg_regshift_aflags(name, _rd, _rn, _rm, st, _rs)         \
  ARM_##name##_REG_REGSHIFT(0, _rd, _rn, _rm, st, _rs)                        \

#define generate_op_reg_regshift_aflags_load_c(name, _rd, _rn, _rm, st, _rs)  \
  ARM_##name##_REG_REGSHIFT(0, _rd, _rn, _rm, st, _rs)                        \

#define generate_op_reg_regshift_uflags(name, _rd, _rm, shift_type, _rs)      \
  ARM_##name##_REG_REGSHIFT(0, _rd, _rm, shift_type, _rs)                     \

#define generate_op_reg_regshift_tflags(name, _rn, _rm, shift_type, _rs)      \
  ARM_##name##_REG_REGSHIFT(0, _rn, _rm, shift_type, _rs)                     \


#define generate_op_imm_lflags(name, _rd, _rn)                                \
  ARM_##name##_REG_IMM(0, _rd, _rn, imm, imm_ror)                             \

#define generate_op_imm_aflags(name, _rd, _rn)                                \
  ARM_##name##_REG_IMM(0, _rd, _rn, imm, imm_ror)                             \

#define generate_op_imm_aflags_load_c(name, _rd, _rn)                         \
  ARM_##name##_REG_IMM(0, _rd, _rn, imm, imm_ror)                             \

#define generate_op_imm_uflags(name, _rd)                                     \
  ARM_##name##_REG_IMM(0, _rd, imm, imm_ror)                                  \

#define generate_op_imm_tflags(name, _rn)                                     \
  ARM_##name##_REG_IMM(0, _rn, imm, imm_ror)                                  \


#define generate_op_ands_reg_immshift(_rd, _rn, _rm, shift_type, shift)       \
  generate_op_reg_immshift_lflags(ANDS, _rd, _rn, _rm, shift_type, shift)     \

#define generate_op_orrs_reg_immshift(_rd, _rn, _rm, shift_type, shift)       \
  generate_op_reg_immshift_lflags(ORRS, _rd, _rn, _rm, shift_type, shift)     \

#define generate_op_eors_reg_immshift(_rd, _rn, _rm, shift_type, shift)       \
  generate_op_reg_immshift_lflags(EORS, _rd, _rn, _rm, shift_type, shift)     \

#define generate_op_bics_reg_immshift(_rd, _rn, _rm, shift_type, shift)       \
  generate_op_reg_immshift_lflags(BICS, _rd, _rn, _rm, shift_type, shift)     \

#define generate_op_subs_reg_immshift(_rd, _rn, _rm, shift_type, shift)       \
  generate_op_reg_immshift_aflags(SUBS, _rd, _rn, _rm, shift_type, shift)     \

#define generate_op_rsbs_reg_immshift(_rd, _rn, _rm, shift_type, shift)       \
  generate_op_reg_immshift_aflags(RSBS, _rd, _rn, _rm, shift_type, shift)     \

#define generate_op_sbcs_reg_immshift(_rd, _rn, _rm, st, shift)               \
  generate_op_reg_immshift_aflags_load_c(SBCS, _rd, _rn, _rm, st, shift)      \

#define generate_op_rscs_reg_immshift(_rd, _rn, _rm, st, shift)               \
  generate_op_reg_immshift_aflags_load_c(RSCS, _rd, _rn, _rm, st, shift)      \

#define generate_op_adds_reg_immshift(_rd, _rn, _rm, shift_type, shift)       \
  generate_op_reg_immshift_aflags(ADDS, _rd, _rn, _rm, shift_type, shift)     \

#define generate_op_adcs_reg_immshift(_rd, _rn, _rm, st, shift)               \
  generate_op_reg_immshift_aflags_load_c(ADCS, _rd, _rn, _rm, st, shift)      \

#define generate_op_movs_reg_immshift(_rd, _rn, _rm, shift_type, shift)       \
  generate_op_reg_immshift_uflags(MOVS, _rd, _rm, shift_type, shift)          \

#define generate_op_mvns_reg_immshift(_rd, _rn, _rm, shift_type, shift)       \
  generate_op_reg_immshift_uflags(MVNS, _rd, _rm, shift_type, shift)          \

/* The reg operand is in reg_rm, not reg_rn like expected, so rsbs isn't
 * being used here. When rsbs is fully inlined it can be used with the
 * apropriate operands.
 */

#define generate_op_neg_reg_immshift(_rd, _rn, _rm, shift_type, shift)        \
{                                                                             \
  generate_load_imm(reg_rn, 0, 0);                                            \
  generate_op_subs_reg_immshift(_rd, reg_rn, _rm, ARMSHIFT_LSL, 0);           \
}                                                                             \

#define generate_op_muls_reg_immshift(_rd, _rn, _rm, shift_type, shift)       \
  ARM_MULS(0, _rd, _rn, _rm);                                                 \

#define generate_op_cmp_reg_immshift(_rd, _rn, _rm, shift_type, shift)        \
  generate_op_reg_immshift_tflags(CMP, _rn, _rm, shift_type, shift)           \

#define generate_op_cmn_reg_immshift(_rd, _rn, _rm, shift_type, shift)        \
  generate_op_reg_immshift_tflags(CMN, _rn, _rm, shift_type, shift)           \

#define generate_op_tst_reg_immshift(_rd, _rn, _rm, shift_type, shift)        \
  generate_op_reg_immshift_tflags(TST, _rn, _rm, shift_type, shift)           \

#define generate_op_teq_reg_immshift(_rd, _rn, _rm, shift_type, shift)        \
  generate_op_reg_immshift_tflags(TEQ, _rn, _rm, shift_type, shift)           \


#define generate_op_ands_reg_regshift(_rd, _rn, _rm, shift_type, _rs)         \
  generate_op_reg_regshift_lflags(ANDS, _rd, _rn, _rm, shift_type, _rs)       \

#define generate_op_orrs_reg_regshift(_rd, _rn, _rm, shift_type, _rs)         \
  generate_op_reg_regshift_lflags(ORRS, _rd, _rn, _rm, shift_type, _rs)       \

#define generate_op_eors_reg_regshift(_rd, _rn, _rm, shift_type, _rs)         \
  generate_op_reg_regshift_lflags(EORS, _rd, _rn, _rm, shift_type, _rs)       \

#define generate_op_bics_reg_regshift(_rd, _rn, _rm, shift_type, _rs)         \
  generate_op_reg_regshift_lflags(BICS, _rd, _rn, _rm, shift_type, _rs)       \

#define generate_op_subs_reg_regshift(_rd, _rn, _rm, shift_type, _rs)         \
  generate_op_reg_regshift_aflags(SUBS, _rd, _rn, _rm, shift_type, _rs)       \

#define generate_op_rsbs_reg_regshift(_rd, _rn, _rm, shift_type, _rs)         \
  generate_op_reg_regshift_aflags(RSBS, _rd, _rn, _rm, shift_type, _rs)       \

#define generate_op_sbcs_reg_regshift(_rd, _rn, _rm, st, _rs)                 \
  generate_op_reg_regshift_aflags_load_c(SBCS, _rd, _rn, _rm, st, _rs)        \

#define generate_op_rscs_reg_regshift(_rd, _rn, _rm, st, _rs)                 \
  generate_op_reg_regshift_aflags_load_c(RSCS, _rd, _rn, _rm, st, _rs)        \

#define generate_op_adds_reg_regshift(_rd, _rn, _rm, shift_type, _rs)         \
  generate_op_reg_regshift_aflags(ADDS, _rd, _rn, _rm, shift_type, _rs)       \

#define generate_op_adcs_reg_regshift(_rd, _rn, _rm, st, _rs)                 \
  generate_op_reg_regshift_aflags_load_c(ADCS, _rd, _rn, _rm, st, _rs)        \

#define generate_op_movs_reg_regshift(_rd, _rn, _rm, shift_type, _rs)         \
  generate_op_reg_regshift_uflags(MOVS, _rd, _rm, shift_type, _rs)            \

#define generate_op_mvns_reg_regshift(_rd, _rn, _rm, shift_type, _rs)         \
  generate_op_reg_regshift_uflags(MVNS, _rd, _rm, shift_type, _rs)            \

#define generate_op_cmp_reg_regshift(_rd, _rn, _rm, shift_type, _rs)          \
  generate_op_reg_regshift_tflags(CMP, _rn, _rm, shift_type, _rs)             \

#define generate_op_cmn_reg_regshift(_rd, _rn, _rm, shift_type, _rs)          \
  generate_op_reg_regshift_tflags(CMN, _rn, _rm, shift_type, _rs)             \

#define generate_op_tst_reg_regshift(_rd, _rn, _rm, shift_type, _rs)          \
  generate_op_reg_regshift_tflags(TST, _rn, _rm, shift_type, _rs)             \

#define generate_op_teq_reg_regshift(_rd, _rn, _rm, shift_type, _rs)          \
  generate_op_reg_regshift_tflags(TEQ, _rn, _rm, shift_type, _rs)             \


#define generate_op_ands_imm(_rd, _rn)                                        \
  generate_op_imm_lflags(ANDS, _rd, _rn)                                      \

#define generate_op_orrs_imm(_rd, _rn)                                        \
  generate_op_imm_lflags(ORRS, _rd, _rn)                                      \

#define generate_op_eors_imm(_rd, _rn)                                        \
  generate_op_imm_lflags(EORS, _rd, _rn)                                      \

#define generate_op_bics_imm(_rd, _rn)                                        \
  generate_op_imm_lflags(BICS, _rd, _rn)                                      \

#define generate_op_subs_imm(_rd, _rn)                                        \
  generate_op_imm_aflags(SUBS, _rd, _rn)                                      \

#define generate_op_rsbs_imm(_rd, _rn)                                        \
  generate_op_imm_aflags(RSBS, _rd, _rn)                                      \

#define generate_op_sbcs_imm(_rd, _rn)                                        \
  generate_op_imm_aflags_load_c(SBCS, _rd, _rn)                               \

#define generate_op_rscs_imm(_rd, _rn)                                        \
  generate_op_imm_aflags_load_c(RSCS, _rd, _rn)                               \

#define generate_op_adds_imm(_rd, _rn)                                        \
  generate_op_imm_aflags(ADDS, _rd, _rn)                                      \

#define generate_op_adcs_imm(_rd, _rn)                                        \
  generate_op_imm_aflags_load_c(ADCS, _rd, _rn)                               \

#define generate_op_movs_imm(_rd, _rn)                                        \
  generate_op_imm_uflags(MOVS, _rd)                                           \

#define generate_op_mvns_imm(_rd, _rn)                                        \
  generate_op_imm_uflags(MVNS, _rd)                                           \

#define generate_op_cmp_imm(_rd, _rn)                                         \
  generate_op_imm_tflags(CMP, _rn)                                            \

#define generate_op_cmn_imm(_rd, _rn)                                         \
  generate_op_imm_tflags(CMN, _rn)                                            \

#define generate_op_tst_imm(_rd, _rn)                                         \
  generate_op_imm_tflags(TST, _rn)                                            \

#define generate_op_teq_imm(_rd, _rn)                                         \
  generate_op_imm_tflags(TEQ, _rn)                                            \


#define arm_prepare_load_rn_yes()                                             \
  u32 _rn = arm_prepare_load_reg_pc(&translation_ptr, reg_rn, rn, pc + 8)     \

#define arm_prepare_load_rn_no()                                              \

#define arm_prepare_store_rd_yes()                                            \
  u32 _rd = arm_prepare_store_reg(reg_rd, rd)                                 \

#define arm_prepare_store_rd_no()                                             \

#define arm_complete_store_rd_yes(flags_op)                                   \
  arm_complete_store_reg_pc_##flags_op(_rd, rd)                               \

#define arm_complete_store_rd_no(flags_op)                                    \

#define arm_generate_op_reg(name, load_op, store_op, flags_op)                \
  u32 shift_type = (opcode >> 5) & 0x03;                                      \
  arm_decode_data_proc_reg(opcode);                                           \
  arm_prepare_load_rn_##load_op();                                            \
  arm_prepare_store_rd_##store_op();                                          \
                                                                              \
  if((opcode >> 4) & 0x01)                                                    \
  {                                                                           \
    u32 rs = ((opcode >> 8) & 0x0F);                                          \
    u32 _rs = arm_prepare_load_reg(&translation_ptr, reg_rs, rs);             \
    u32 _rm = arm_prepare_load_reg_pc(&translation_ptr, reg_rm, rm, pc + 12); \
    generate_op_##name##_reg_regshift(_rd, _rn, _rm, shift_type, _rs);        \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    u32 shift_imm = ((opcode >> 7) & 0x1F);                                   \
    u32 _rm = arm_prepare_load_reg_pc(&translation_ptr, reg_rm, rm, pc + 8);  \
    generate_op_##name##_reg_immshift(_rd, _rn, _rm, shift_type, shift_imm);  \
  }                                                                           \
  arm_complete_store_rd_##store_op(flags_op)                                  \

#define arm_generate_op_reg_flags(name, load_op, store_op, flags_op)          \
  arm_generate_op_reg(name, load_op, store_op, flags_op)                      \

/* imm will be loaded by the called function if necessary. */

#define arm_generate_op_imm(name, load_op, store_op, flags_op)                \
  arm_decode_data_proc_imm(opcode);                                           \
  arm_prepare_load_rn_##load_op();                                            \
  arm_prepare_store_rd_##store_op();                                          \
  generate_op_##name##_imm(_rd, _rn);                                         \
  arm_complete_store_rd_##store_op(flags_op)                                  \

#define arm_generate_op_imm_flags(name, load_op, store_op, flags_op)          \
  arm_generate_op_imm(name, load_op, store_op, flags_op)                      \

#define arm_data_proc(name, type, flags_op)                                   \
{                                                                             \
  arm_generate_op_##type(name, yes, yes, flags_op);                           \
}                                                                             \

#define arm_data_proc_test(name, type)                                        \
{                                                                             \
  arm_generate_op_##type(name, yes, no, no);                                  \
}                                                                             \

#define arm_data_proc_unary(name, type, flags_op)                             \
{                                                                             \
  arm_generate_op_##type(name, no, yes, flags_op);                            \
}                                                                             \


#define arm_multiply_add_no_flags_no()                                        \
  ARM_MUL(0, _rd, _rm, _rs)                                                   \

#define arm_multiply_add_yes_flags_no()                                       \
  u32 _rn = arm_prepare_load_reg(&translation_ptr, reg_a2, rn);               \
  ARM_MLA(0, _rd, _rm, _rs, _rn)                                              \

#define arm_multiply_add_no_flags_yes()                                       \
  ARM_MULS(0, _rd, _rm, _rs)                                                  \

#define arm_multiply_add_yes_flags_yes()                                      \
  u32 _rn = arm_prepare_load_reg(&translation_ptr, reg_a2, rn);               \
  ARM_MLAS(0, _rd, _rm, _rs, _rn);                                            \


#define arm_multiply(add_op, flags)                                           \
{                                                                             \
  arm_decode_multiply();                                                      \
  u32 _rm = arm_prepare_load_reg(&translation_ptr, reg_a0, rm);               \
  u32 _rs = arm_prepare_load_reg(&translation_ptr, reg_a1, rs);               \
  u32 _rd = arm_prepare_store_reg(reg_a0, rd);                                \
  arm_multiply_add_##add_op##_flags_##flags();                                \
  arm_complete_store_reg(_rd, rd);                                            \
}                                                                             \


#define arm_multiply_long_name_s64     SMULL
#define arm_multiply_long_name_u64     UMULL
#define arm_multiply_long_name_s64_add SMLAL
#define arm_multiply_long_name_u64_add UMLAL


#define arm_multiply_long_flags_no(name)                                      \
  ARM_##name(0, _rdlo, _rdhi, _rm, _rs)                                       \

#define arm_multiply_long_flags_yes(name)                                     \
  ARM_##name##S(0, _rdlo, _rdhi, _rm, _rs);                                   \


#define arm_multiply_long_add_no(name)                                        \

#define arm_multiply_long_add_yes(name)                                       \
  arm_prepare_load_reg(&translation_ptr, reg_a0, rdlo);                       \
  arm_prepare_load_reg(&translation_ptr, reg_a1, rdhi)                        \


#define arm_multiply_long_op(flags, name)                                     \
  arm_multiply_long_flags_##flags(name)                                       \

#define arm_multiply_long(name, add_op, flags)                                \
{                                                                             \
  arm_decode_multiply_long();                                                 \
  u32 _rm = arm_prepare_load_reg(&translation_ptr, reg_a2, rm);               \
  u32 _rs = arm_prepare_load_reg(&translation_ptr, reg_rs, rs);               \
  u32 _rdlo = (rdlo == rdhi) ? reg_a0 : arm_prepare_store_reg(reg_a0, rdlo);  \
  u32 _rdhi = arm_prepare_store_reg(reg_a1, rdhi);                            \
  arm_multiply_long_add_##add_op(name);                                       \
  arm_multiply_long_op(flags, arm_multiply_long_name_##name);                 \
  arm_complete_store_reg(_rdlo, rdlo);                                        \
  arm_complete_store_reg(_rdhi, rdhi);                                        \
}                                                                             \

#define arm_psr_read_cpsr()                                                   \
{                                                                             \
  u32 _rd = arm_prepare_store_reg(reg_a0, rd);                                \
  generate_load_memreg(_rd, REG_CPSR);                                        \
  generate_save_flags();                                                      \
  ARM_BIC_REG_IMM(0, _rd, _rd, 0xF0, arm_imm_lsl_to_rot(24));                 \
  ARM_AND_REG_IMM(0, reg_flags, reg_flags, 0xF0, arm_imm_lsl_to_rot(24));     \
  ARM_ORR_REG_REG(0, _rd, _rd, reg_flags);                                    \
  arm_complete_store_reg(_rd, rd)                                             \
}

#define arm_psr_read_spsr()                                                   \
{                                                                             \
  u32 _rd = arm_prepare_store_reg(reg_a0, rd);                                \
  ARM_ADD_REG_IMM(0, reg_a0, reg_base, SPSR_RAM_OFF >> 2, 30);                \
  ARM_LDR_IMM(0, reg_a1, reg_base, CPU_MODE * 4);                             \
  ARM_AND_REG_IMM(0, reg_a1, reg_a1, 0xF, 0);                                 \
  ARM_LDR_REG_REG_SHIFT(0, _rd, reg_a0, reg_a1, ARMSHIFT_LSL, 2);             \
  arm_complete_store_reg(_rd, rd);                                            \
}

#define arm_psr_read(op_type, psr_reg)                                        \
  arm_psr_read_##psr_reg()                                                    \

/* This function's okay because it's called from an ASM function that can
 * wrap it correctly.
 */

u32 execute_store_cpsr_body(u32 _cpsr, u32 store_mask, u32 address)
{
  reg[REG_CPSR] = _cpsr;
  if(store_mask & 0xFF)
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
  }

  return 0;
}

static void trace_instruction(u32 pc, u32 mode)
{
  if (mode)
    printf("Executed arm %x\n", pc);
  else
    printf("Executed thumb %x\n", pc);
  #ifdef TRACE_REGISTERS
  print_regs();
  #endif
}

#ifdef TRACE_INSTRUCTIONS
  #define emit_trace_instruction(pc, mode, regt)   \
  {                                                \
    unsigned i;                                    \
    for (i = 0; i < 15; i++) {                     \
      if (regt[i] != mem_reg) {                    \
        ARM_STR_IMM(0, regt[i], reg_base, (i*4));  \
      }                                            \
    }                                              \
    generate_save_flags();                         \
    ARM_STMDB_WB(0, ARMREG_SP, 0x500C);            \
    arm_load_imm_32bit(reg_a0, pc);                \
    arm_load_imm_32bit(reg_a1, mode);              \
    generate_function_far_call(armfn_debug_trace); \
    ARM_LDMIA_WB(0, ARMREG_SP, 0x500C);            \
    generate_restore_flags();                      \
  }

  #define emit_trace_thumb_instruction(pc)         \
    emit_trace_instruction(pc, 0, thumb_register_allocation)

  #define emit_trace_arm_instruction(pc)           \
    emit_trace_instruction(pc, 1, arm_register_allocation)
#else
  #define emit_trace_thumb_instruction(pc)
  #define emit_trace_arm_instruction(pc)
#endif

#define arm_psr_load_new_reg()                                                \
  arm_generate_load_reg(reg_a0, rm)                                           \

#define arm_psr_load_new_imm()                                                \
  generate_load_imm(reg_a0, imm, imm_ror)                                     \

#define arm_psr_store_cpsr()                                                  \
  generate_function_far_call(armfn_store_cpsr);                               \
  write32(cpsr_masks[psr_pfield][0]);                                         \
  write32(cpsr_masks[psr_pfield][1]);                                         \
  write32(pc);                                                                \

#define arm_psr_store_spsr()                                                  \
  arm_load_imm_32bit(reg_a1, spsr_masks[psr_pfield]);                         \
  ARM_LDR_IMM(0, reg_a2, reg_base, (CPU_MODE * 4));                           \
  ARM_AND_REG_IMM(0, reg_a2, reg_a2, 0xF, 0);                                 \
  ARM_ADD_REG_IMMSHIFT(0, ARMREG_LR, reg_base, reg_a2, ARMSHIFT_LSL, 2);      \
  ARM_AND_REG_IMMSHIFT(0, reg_a0, reg_a0, reg_a1, ARMSHIFT_LSL, 0);           \
  ARM_LDR_IMM(0, reg_a2, ARMREG_LR, SPSR_RAM_OFF);                            \
  ARM_BIC_REG_IMMSHIFT(0, reg_a2, reg_a2, reg_a1, ARMSHIFT_LSL, 0);           \
  ARM_ORR_REG_IMMSHIFT(0, reg_a0, reg_a0, reg_a2, ARMSHIFT_LSL, 0);           \
  ARM_STR_IMM(0, reg_a0, ARMREG_LR, SPSR_RAM_OFF);                            \

#define arm_psr_store(op_type, psr_reg)                                       \
  arm_psr_load_new_##op_type();                                               \
  arm_psr_store_##psr_reg()                                                   \


#define arm_psr(op_type, transfer_type, psr_reg)                              \
{                                                                             \
  arm_decode_psr_##op_type(opcode);                                           \
  arm_psr_##transfer_type(op_type, psr_reg);                                  \
}                                                                             \

/* We use USAT + ROR to map addresses to the handler table. For ARMv5 we use
   the table -1 entry to map any out of range/unaligned access, and some fun
   math/logical tricks to avoid using USAT */

#if __ARM_ARCH >= 6
  #define mem_calc_region(abits)                                              \
    if (abits) {                                                              \
      ARM_MOV_REG_IMMSHIFT(0, reg_a2, reg_a0, ARMSHIFT_ROR, abits)            \
      ARM_USAT_ASR(0, reg_a2, 4, reg_a2, 24-abits, ARMCOND_AL);               \
    } else {                                                                  \
      ARM_USAT_ASR(0, reg_a2, 4, reg_a0, 24, ARMCOND_AL);                     \
    }
#else
  #define mem_calc_region(abits)                                              \
    if (abits) {                                                              \
      ARM_ORR_REG_IMMSHIFT(0, reg_a2, reg_a0, reg_a0, ARMSHIFT_LSL, 32-abits);\
      ARM_MOV_REG_IMMSHIFT(0, reg_a2, reg_a2, ARMSHIFT_LSR, 24);              \
    } else {                                                                  \
      ARM_MOV_REG_IMMSHIFT(0, reg_a2, reg_a0, ARMSHIFT_LSR, 24);              \
    }                                                                         \
    ARM_RSB_REG_IMM(0, ARMREG_LR, reg_a2, 15, 0);                             \
    ARM_ORR_REG_IMMSHIFT(0, reg_a2, reg_a2, ARMREG_LR, ARMSHIFT_ASR, 31);
#endif

#define generate_load_call_byte(tblnum)                                       \
  mem_calc_region(0);                                                         \
  generate_add_imm(reg_a2, (STORE_TBL_OFF + 68*tblnum + 4) >> 2, 0);          \
  ARM_LDR_REG_REG_SHIFT(0, reg_a2, reg_base, reg_a2, 0, 2);                   \
  ARM_BLX(0, reg_a2);                                                         \

#define generate_load_call_mbyte(tblnum, abits)                               \
  mem_calc_region(abits);                                                     \
  generate_add_imm(reg_a2, (STORE_TBL_OFF + 68*tblnum + 4) >> 2, 0);          \
  ARM_LDR_REG_REG_SHIFT(0, reg_a2, reg_base, reg_a2, 0, 2);                   \
  ARM_BLX(0, reg_a2);                                                         \

#define generate_store_call(tblnum)                                           \
  mem_calc_region(0);                                                         \
  generate_add_imm(reg_a2, (STORE_TBL_OFF + 68*tblnum + 4) >> 2, 0);          \
  ARM_LDR_REG_REG_SHIFT(0, reg_a2, reg_base, reg_a2, 0, 2);                   \
  ARM_BLX(0, reg_a2);                                                         \


#define generate_store_call_u8()        generate_store_call(0)
#define generate_store_call_u16()       generate_store_call(1)
#define generate_store_call_u32()       generate_store_call(2)
#define generate_store_call_u32_safe()  generate_store_call(3)
#define generate_load_call_u8()         generate_load_call_byte(4)
#define generate_load_call_s8()         generate_load_call_byte(5)
#define generate_load_call_u16()        generate_load_call_mbyte(6, 1)
#define generate_load_call_s16()        generate_load_call_mbyte(7, 1)
#define generate_load_call_u32()        generate_load_call_mbyte(8, 2)


#define arm_access_memory_load(mem_type)                                      \
  cycle_count += 2;                                                           \
  generate_load_call_##mem_type();                                            \
  write32(pc);                                                                \
  arm_generate_store_reg_pc_no_flags(reg_rv, rd)                              \

#define arm_access_memory_store(mem_type)                                     \
  cycle_count++;                                                              \
  arm_generate_load_reg_pc(reg_a1, rd, 12);                                   \
  generate_store_call_##mem_type();                                           \
  write32((pc + 4))                                                           \

/* Calculate the address into a0 from _rn, _rm */

#define arm_access_memory_adjust_reg_sh_up(ireg)                              \
  ARM_ADD_REG_IMMSHIFT(0, ireg, _rn, _rm, ((opcode >> 5) & 0x03),             \
   ((opcode >> 7) & 0x1F))                                                    \

#define arm_access_memory_adjust_reg_sh_down(ireg)                            \
  ARM_SUB_REG_IMMSHIFT(0, ireg, _rn, _rm, ((opcode >> 5) & 0x03),             \
   ((opcode >> 7) & 0x1F))                                                    \

#define arm_access_memory_adjust_reg_up(ireg)                                 \
  ARM_ADD_REG_REG(0, ireg, _rn, _rm)                                          \

#define arm_access_memory_adjust_reg_down(ireg)                               \
  ARM_SUB_REG_REG(0, ireg, _rn, _rm)                                          \

#define arm_access_memory_adjust_imm(op, ireg)                                \
{                                                                             \
  u32 stores[4];                                                              \
  u32 rotations[4];                                                           \
  u32 store_count = arm_disect_imm_32bit(offset, stores, rotations);          \
                                                                              \
  if(store_count > 1)                                                         \
  {                                                                           \
    ARM_##op##_REG_IMM(0, ireg, _rn, stores[0], rotations[0]);                \
    ARM_##op##_REG_IMM(0, ireg, ireg, stores[1], rotations[1]);               \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    ARM_##op##_REG_IMM(0, ireg, _rn, stores[0], rotations[0]);                \
  }                                                                           \
}                                                                             \

#define arm_access_memory_adjust_imm_up(ireg)                                 \
  arm_access_memory_adjust_imm(ADD, ireg)                                     \

#define arm_access_memory_adjust_imm_down(ireg)                               \
  arm_access_memory_adjust_imm(SUB, ireg)                                     \


#define arm_access_memory_pre(type, direction)                                \
  arm_access_memory_adjust_##type##_##direction(reg_a0)                       \

#define arm_access_memory_pre_wb(type, direction)                             \
  arm_access_memory_adjust_##type##_##direction(reg_a0);                      \
  arm_generate_store_reg(reg_a0, rn)                                          \

#define arm_access_memory_post(type, direction)                               \
  u32 _rn_dest = arm_prepare_store_reg(reg_a1, rn);                           \
  if(_rn != reg_a0)                                                           \
  {                                                                           \
    arm_generate_load_reg(reg_a0, rn);                                        \
  }                                                                           \
  arm_access_memory_adjust_##type##_##direction(_rn_dest);                    \
  arm_complete_store_reg(_rn_dest, rn)                                        \


#define arm_data_trans_reg(adjust_op, direction)                              \
  arm_decode_data_trans_reg();                                                \
  u32 _rn = arm_prepare_load_reg_pc(&translation_ptr, reg_a0, rn, pc + 8);    \
  u32 _rm = arm_prepare_load_reg(&translation_ptr, reg_a1, rm);               \
  arm_access_memory_##adjust_op(reg_sh, direction)                            \

#define arm_data_trans_imm(adjust_op, direction)                              \
  arm_decode_data_trans_imm();                                                \
  u32 _rn = arm_prepare_load_reg_pc(&translation_ptr, reg_a0, rn, pc + 8);    \
  arm_access_memory_##adjust_op(imm, direction)                               \


#define arm_data_trans_half_reg(adjust_op, direction)                         \
  arm_decode_half_trans_r();                                                  \
  u32 _rn = arm_prepare_load_reg_pc(&translation_ptr, reg_a0, rn, pc + 8);    \
  u32 _rm = arm_prepare_load_reg(&translation_ptr, reg_a1, rm);               \
  arm_access_memory_##adjust_op(reg, direction)                               \

#define arm_data_trans_half_imm(adjust_op, direction)                         \
  arm_decode_half_trans_of();                                                 \
  u32 _rn = arm_prepare_load_reg_pc(&translation_ptr, reg_a0, rn, pc + 8);    \
  arm_access_memory_##adjust_op(imm, direction)                               \


#define arm_access_memory(access_type, direction, adjust_op, mem_type,        \
 offset_type)                                                                 \
{                                                                             \
  arm_data_trans_##offset_type(adjust_op, direction);                         \
  arm_access_memory_##access_type(mem_type);                                  \
}                                                                             \


#define word_bit_count(word)                                                  \
  (bit_count[word >> 8] + bit_count[word & 0xFF])                             \

/* TODO: Make these use cached registers. Implement iwram_stack_optimize. */

#define arm_block_memory_load()                                               \
  generate_load_call_u32();                                                   \
  write32((pc + 8));                                                          \
  arm_generate_store_reg(reg_rv, i)                                           \

#define arm_block_memory_store()                                              \
  arm_generate_load_reg_pc(reg_a1, i, 8);                                     \
  generate_store_call_u32_safe()                                              \

#define arm_block_memory_final_load(writeback_type)                           \
  arm_block_memory_load()                                                     \

#define arm_block_memory_final_store(writeback_type)                          \
  arm_generate_load_reg_pc(reg_a1, i, 12);                                    \
  arm_block_memory_writeback_post_store(writeback_type);                      \
  generate_store_call_u32();                                                  \
  write32((pc + 4))                                                           \

#define arm_block_memory_adjust_pc_store()                                    \

#define arm_block_memory_adjust_pc_load()                                     \
  if(reg_list & 0x8000)                                                       \
  {                                                                           \
    generate_indirect_branch_arm();                                           \
  }                                                                           \

#define arm_block_memory_offset_down_a()                                      \
  generate_sub_imm(reg_s0, ((word_bit_count(reg_list) * 4) - 4), 0)           \

#define arm_block_memory_offset_down_b()                                      \
  generate_sub_imm(reg_s0, (word_bit_count(reg_list) * 4), 0)                 \

#define arm_block_memory_offset_no()                                          \

#define arm_block_memory_offset_up()                                          \
  generate_add_imm(reg_s0, 4, 0)                                              \

#define arm_block_memory_writeback_down()                                     \
  arm_generate_load_reg(reg_a2, rn);                                          \
  generate_sub_imm(reg_a2, (word_bit_count(reg_list) * 4), 0);                \
  arm_generate_store_reg(reg_a2, rn)                                          \

#define arm_block_memory_writeback_up()                                       \
  arm_generate_load_reg(reg_a2, rn);                                          \
  generate_add_imm(reg_a2, (word_bit_count(reg_list) * 4), 0);                \
  arm_generate_store_reg(reg_a2, rn)                                          \

#define arm_block_memory_writeback_no()

/* Only emit writeback if the register is not in the list */

#define arm_block_memory_writeback_pre_load(writeback_type)                   \
  if(!((reg_list >> rn) & 0x01))                                              \
  {                                                                           \
    arm_block_memory_writeback_##writeback_type();                            \
  }                                                                           \

#define arm_block_memory_writeback_post_store(writeback_type)                 \
  arm_block_memory_writeback_##writeback_type()                               \

#define arm_block_memory_writeback_pre_store(writeback_type)

#define arm_block_memory(access_type, offset_type, writeback_type, s_bit)     \
{                                                                             \
  arm_decode_block_trans();                                                   \
  u32 offset = 0;                                                             \
  u32 i;                                                                      \
                                                                              \
  arm_generate_load_reg(reg_s0, rn);                                          \
  arm_block_memory_offset_##offset_type();                                    \
  arm_block_memory_writeback_pre_##access_type(writeback_type);               \
  ARM_BIC_REG_IMM(0, reg_s0, reg_s0, 0x03, 0);                                \
  arm_generate_store_reg(reg_s0, REG_SAVE);                                   \
                                                                              \
  for(i = 0; i < 16; i++)                                                     \
  {                                                                           \
    if((reg_list >> i) & 0x01)                                                \
    {                                                                         \
      cycle_count++;                                                          \
      arm_generate_load_reg(reg_s0, REG_SAVE);                                \
      generate_add_reg_reg_imm(reg_a0, reg_s0, offset, 0);                    \
      if(reg_list & ~((2 << i) - 1))                                          \
      {                                                                       \
        arm_block_memory_##access_type();                                     \
        offset += 4;                                                          \
      }                                                                       \
      else                                                                    \
      {                                                                       \
        arm_block_memory_final_##access_type(writeback_type);                 \
        break;                                                                \
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
  arm_generate_load_reg(reg_a0, rn);                                          \
  generate_load_call_##type();                                                \
  write32((pc + 8));                                                          \
  generate_mov(reg_a2, reg_rv);                                               \
  arm_generate_load_reg(reg_a0, rn);                                          \
  arm_generate_load_reg(reg_a1, rm);                                          \
  arm_generate_store_reg(reg_a2, rd);                                         \
  generate_store_call_##type();                                               \
  write32((pc + 4));                                                          \
}                                                                             \


#define thumb_generate_op_reg(name, _rd, _rs, _rn)                            \
  u32 __rm = thumb_prepare_load_reg(&translation_ptr, reg_rm, _rn);           \
  generate_op_##name##_reg_immshift(__rd, __rn, __rm, ARMSHIFT_LSL, 0)        \

#define thumb_generate_op_imm(name, _rd, _rs, imm_)                           \
{                                                                             \
  u32 imm_ror = 0;                                                            \
  generate_op_##name##_imm(__rd, __rn);                                       \
}                                                                             \


#define thumb_data_proc(type, name, op_type, _rd, _rs, _rn)                   \
{                                                                             \
  thumb_decode_##type();                                                      \
  u32 __rn = thumb_prepare_load_reg(&translation_ptr, reg_rn, _rs);           \
  u32 __rd = thumb_prepare_store_reg(reg_rd, _rd);                            \
  thumb_generate_op_##op_type(name, _rd, _rs, _rn);                           \
  thumb_complete_store_reg(__rd, _rd);                                        \
}                                                                             \

#define thumb_data_proc_test(type, name, op_type, _rd, _rs)                   \
{                                                                             \
  thumb_decode_##type();                                                      \
  u32 __rn = thumb_prepare_load_reg(&translation_ptr, reg_rn, _rd);           \
  thumb_generate_op_##op_type(name, 0, _rd, _rs);                             \
}                                                                             \

#define thumb_data_proc_unary(type, name, op_type, _rd, _rs)                  \
{                                                                             \
  thumb_decode_##type();                                                      \
  u32 __rd = thumb_prepare_store_reg(reg_rd, _rd);                            \
  thumb_generate_op_##op_type(name, _rd, 0, _rs);                             \
  thumb_complete_store_reg(__rd, _rd);                                        \
}                                                                             \


#define complete_store_reg_pc_thumb()                                         \
  if(rd == 15)                                                                \
  {                                                                           \
    generate_indirect_branch_cycle_update(thumb);                             \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    thumb_complete_store_reg(_rd, rd);                                        \
  }                                                                           \

#define thumb_data_proc_hi(name)                                              \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  u32 _rd = thumb_prepare_load_reg_pc(&translation_ptr, reg_rd, rd, pc + 4);  \
  u32 _rs = thumb_prepare_load_reg_pc(&translation_ptr, reg_rn, rs, pc + 4);  \
  generate_op_##name##_reg_immshift(_rd, _rd, _rs, ARMSHIFT_LSL, 0);          \
  complete_store_reg_pc_thumb();                                              \
}                                                                             \

#define thumb_data_proc_test_hi(name)                                         \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  u32 _rd = thumb_prepare_load_reg_pc(&translation_ptr, reg_rd, rd, pc + 4);  \
  u32 _rs = thumb_prepare_load_reg_pc(&translation_ptr, reg_rn, rs, pc + 4);  \
  generate_op_##name##_reg_immshift(0, _rd, _rs, ARMSHIFT_LSL, 0);            \
}                                                                             \

#define thumb_data_proc_mov_hi()                                              \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  u32 _rs = thumb_prepare_load_reg_pc(&translation_ptr, reg_rn, rs, pc + 4);  \
  u32 _rd = thumb_prepare_store_reg(reg_rd, rd);                              \
  ARM_MOV_REG_REG(0, _rd, _rs);                                               \
  complete_store_reg_pc_thumb();                                              \
}                                                                             \



#define thumb_load_pc(_rd)                                                    \
{                                                                             \
  thumb_decode_imm();                                                         \
  u32 __rd = thumb_prepare_store_reg(reg_rd, _rd);                            \
  generate_load_pc(__rd, (((pc & ~2) + 4) + (imm * 4)));                      \
  thumb_complete_store_reg(__rd, _rd);                                        \
}                                                                             \

#define thumb_load_sp(_rd)                                                    \
{                                                                             \
  thumb_decode_imm();                                                         \
  u32 __sp = thumb_prepare_load_reg(&translation_ptr, reg_a0, REG_SP);        \
  u32 __rd = thumb_prepare_store_reg(reg_a0, _rd);                            \
  ARM_ADD_REG_IMM(0, __rd, __sp, imm, arm_imm_lsl_to_rot(2));                 \
  thumb_complete_store_reg(__rd, _rd);                                        \
}                                                                             \

#define thumb_adjust_sp_up()                                                  \
  ARM_ADD_REG_IMM(0, _sp, _sp, imm, arm_imm_lsl_to_rot(2))                    \

#define thumb_adjust_sp_down()                                                \
  ARM_SUB_REG_IMM(0, _sp, _sp, imm, arm_imm_lsl_to_rot(2))                    \

#define thumb_adjust_sp(direction)                                            \
{                                                                             \
  thumb_decode_add_sp();                                                      \
  u32 _sp = thumb_prepare_load_reg(&translation_ptr, reg_a0, REG_SP);         \
  thumb_adjust_sp_##direction();                                              \
  thumb_complete_store_reg(_sp, REG_SP);                                      \
}                                                                             \

#define generate_op_lsl_reg(_rd, _rm, _rs)                                    \
  generate_op_movs_reg_regshift(_rd, 0, _rm, ARMSHIFT_LSL, _rs)               \

#define generate_op_lsr_reg(_rd, _rm, _rs)                                    \
  generate_op_movs_reg_regshift(_rd, 0, _rm, ARMSHIFT_LSR, _rs)               \

#define generate_op_asr_reg(_rd, _rm, _rs)                                    \
  generate_op_movs_reg_regshift(_rd, 0, _rm, ARMSHIFT_ASR, _rs)               \

#define generate_op_ror_reg(_rd, _rm, _rs)                                    \
  generate_op_movs_reg_regshift(_rd, 0, _rm, ARMSHIFT_ROR, _rs)               \


#define generate_op_lsl_imm(_rd, _rm)                                         \
  generate_op_movs_reg_immshift(_rd, 0, _rm, ARMSHIFT_LSL, imm)               \

#define generate_op_lsr_imm(_rd, _rm)                                         \
  generate_op_movs_reg_immshift(_rd, 0, _rm, ARMSHIFT_LSR, imm)               \

#define generate_op_asr_imm(_rd, _rm)                                         \
  generate_op_movs_reg_immshift(_rd, 0, _rm, ARMSHIFT_ASR, imm)               \

#define generate_op_ror_imm(_rd, _rm)                                         \
  generate_op_movs_reg_immshift(_rd, 0, _rm, ARMSHIFT_ROR, imm)               \


#define thumb_generate_shift_reg(op_type)                                     \
  u32 __rm = thumb_prepare_load_reg(&translation_ptr, reg_rd, rd);            \
  u32 __rs = thumb_prepare_load_reg(&translation_ptr, reg_rs, rs);            \
  generate_op_##op_type##_reg(__rd, __rm, __rs)                               \

#define thumb_generate_shift_imm(op_type)                                     \
  u32 __rs = thumb_prepare_load_reg(&translation_ptr, reg_rs, rs);            \
  generate_op_##op_type##_imm(__rd, __rs)                                     \


#define thumb_shift(decode_type, op_type, value_type)                         \
{                                                                             \
  thumb_decode_##decode_type();                                               \
  u32 __rd = thumb_prepare_store_reg(reg_rd, rd);                             \
  thumb_generate_shift_##value_type(op_type);                                 \
  thumb_complete_store_reg(__rd, rd);                                         \
}                                                                             \

/* Operation types: imm, mem_reg, mem_imm */

#define thumb_load_pc_pool_const(reg_rd, value)                               \
  u32 rgdst = thumb_prepare_store_reg(reg_a0, reg_rd);                        \
  generate_load_pc(rgdst, (value));                                           \
  thumb_complete_store_reg(rgdst, reg_rd)

#define thumb_access_memory_load(mem_type, _rd)                               \
  cycle_count += 2;                                                           \
  generate_load_call_##mem_type();                                            \
  write32(pc);                                                                \
  thumb_generate_store_reg(reg_rv, _rd)                                       \

#define thumb_access_memory_store(mem_type, _rd)                              \
  cycle_count++;                                                              \
  thumb_generate_load_reg(reg_a1, _rd);                                       \
  generate_store_call_##mem_type();                                           \
  write32((pc + 2))                                                           \

#define thumb_access_memory_generate_address_pc_relative(offset, _rb, _ro)    \
  generate_load_pc(reg_a0, (offset))                                          \

#define thumb_access_memory_generate_address_reg_imm(offset, _rb, _ro)        \
  u32 __rb = thumb_prepare_load_reg(&translation_ptr, reg_a0, _rb);           \
  ARM_ADD_REG_IMM(0, reg_a0, __rb, offset, 0)                                 \

#define thumb_access_memory_generate_address_reg_imm_sp(offset, _rb, _ro)     \
  u32 __rb = thumb_prepare_load_reg(&translation_ptr, reg_a0, _rb);           \
  ARM_ADD_REG_IMM(0, reg_a0, __rb, offset, arm_imm_lsl_to_rot(2))             \

#define thumb_access_memory_generate_address_reg_reg(offset, _rb, _ro)        \
  u32 __rb = thumb_prepare_load_reg(&translation_ptr, reg_a0, _rb);           \
  u32 __ro = thumb_prepare_load_reg(&translation_ptr, reg_a1, _ro);           \
  ARM_ADD_REG_REG(0, reg_a0, __rb, __ro)                                      \

#define thumb_access_memory(access_type, op_type, _rd, _rb, _ro,              \
 address_type, offset, mem_type)                                              \
{                                                                             \
  thumb_decode_##op_type();                                                   \
  thumb_access_memory_generate_address_##address_type(offset, _rb, _ro);      \
  thumb_access_memory_##access_type(mem_type, _rd);                           \
}                                                                             \

/* TODO: Make these use cached registers. Implement iwram_stack_optimize. */

#define thumb_block_address_preadjust_down()                                  \
  generate_sub_imm(reg_s0, (bit_count[reg_list] * 4), 0)                      \

#define thumb_block_address_preadjust_push_lr()                               \
  generate_sub_imm(reg_s0, ((bit_count[reg_list] + 1) * 4), 0)                \

#define thumb_block_address_preadjust_no()                                    \

#define thumb_block_address_postadjust_no(base_reg)                           \
  thumb_generate_store_reg(reg_s0, base_reg)                                  \

#define thumb_block_address_postadjust_up(base_reg)                           \
  generate_add_reg_reg_imm(reg_a0, reg_s0, (bit_count[reg_list] * 4), 0);     \
  thumb_generate_store_reg(reg_a0, base_reg)                                  \

#define thumb_block_address_postadjust_pop_pc(base_reg)                       \
  generate_add_reg_reg_imm(reg_a0, reg_s0,                                    \
   ((bit_count[reg_list] + 1) * 4), 0);                                       \
  thumb_generate_store_reg(reg_a0, base_reg)                                  \

#define thumb_block_address_postadjust_push_lr(base_reg)                      \
  thumb_generate_store_reg(reg_s0, base_reg)                                  \

#define thumb_block_memory_extra_no()                                         \

#define thumb_block_memory_extra_up()                                         \

#define thumb_block_memory_extra_down()                                       \

#define thumb_block_memory_extra_pop_pc()                                     \
  thumb_generate_load_reg(reg_s0, REG_SAVE);                                  \
  generate_add_reg_reg_imm(reg_a0, reg_s0, (bit_count[reg_list] * 4), 0);     \
  generate_load_call_u32();                                                   \
  write32((pc + 4));                                                          \
  generate_indirect_branch_cycle_update(thumb)                                \

#define thumb_block_memory_extra_push_lr(base_reg)                            \
  thumb_generate_load_reg(reg_s0, REG_SAVE);                                  \
  generate_add_reg_reg_imm(reg_a0, reg_s0, (bit_count[reg_list] * 4), 0);     \
  thumb_generate_load_reg(reg_a1, REG_LR);                                    \
  generate_store_call_u32_safe()

#define thumb_block_memory_load()                                             \
  generate_load_call_u32();                                                   \
  write32((pc + 4));                                                          \
  thumb_generate_store_reg(reg_rv, i)                                         \

#define thumb_block_memory_store()                                            \
  thumb_generate_load_reg(reg_a1, i);                                         \
  generate_store_call_u32_safe()

#define thumb_block_memory_final_load()                                       \
  thumb_block_memory_load()                                                   \

#define thumb_block_memory_final_store()                                      \
  thumb_generate_load_reg(reg_a1, i);                                         \
  generate_store_call_u32();                                                  \
  write32((pc + 2))                                                           \

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
  thumb_generate_load_reg(reg_s0, base_reg);                                  \
  ARM_BIC_REG_IMM(0, reg_s0, reg_s0, 0x03, 0);                                \
  thumb_block_address_preadjust_##pre_op();                                   \
  thumb_block_address_postadjust_##post_op(base_reg);                         \
  thumb_generate_store_reg(reg_s0, REG_SAVE);                                 \
                                                                              \
  for(i = 0; i < 8; i++)                                                      \
  {                                                                           \
    if((reg_list >> i) & 0x01)                                                \
    {                                                                         \
      cycle_count++;                                                          \
      thumb_generate_load_reg(reg_s0, REG_SAVE);                              \
      generate_add_reg_reg_imm(reg_a0, reg_s0, offset, 0);                    \
      if(reg_list & ~((2 << i) - 1))                                          \
      {                                                                       \
        thumb_block_memory_##access_type();                                   \
        offset += 4;                                                          \
      }                                                                       \
      else                                                                    \
      {                                                                       \
        thumb_block_memory_final_##post_op(access_type);                      \
        break;                                                                \
      }                                                                       \
    }                                                                         \
  }                                                                           \
                                                                              \
  thumb_block_memory_extra_##post_op();                                       \
}                                                                             \

#define thumb_conditional_branch(condition)                                   \
{                                                                             \
  generate_cycle_update();                                                    \
  generate_branch_filler(condition_opposite_##condition, backpatch_address);  \
  generate_branch_no_cycle_update(                                            \
   block_exits[block_exit_position].branch_source,                            \
   block_exits[block_exit_position].branch_target, thumb);                    \
  generate_branch_patch_conditional(backpatch_address, translation_ptr);      \
  block_exit_position++;                                                      \
}                                                                             \


#define arm_conditional_block_header()                                        \
  generate_cycle_update();                                                    \
  /* This will choose the opposite condition */                               \
  condition ^= 0x01;                                                          \
  generate_branch_filler(condition, backpatch_address)                        \

#define arm_b()                                                               \
  generate_branch(arm)                                                        \

#define arm_bl()                                                              \
  generate_update_pc((pc + 4));                                               \
  arm_generate_store_reg(reg_a0, REG_LR);                                     \
  generate_branch(arm)                                                        \

#define arm_bx()                                                              \
  arm_decode_branchx(opcode);                                                 \
  arm_generate_load_reg(reg_a0, rn);                                          \
  generate_indirect_branch_dual();                                            \

#define arm_swi()                                                             \
  generate_function_far_call(armfn_swi_arm);                                  \
  write32((pc + 4));                                                          \
  generate_branch(arm)                                                        \

#define thumb_b()                                                             \
  generate_branch(thumb)                                                      \

#define thumb_bl()                                                            \
  generate_update_pc(((pc + 2) | 0x01));                                      \
  thumb_generate_store_reg(reg_a0, REG_LR);                                   \
  generate_branch(thumb)                                                      \

#define thumb_blh()                                                           \
{                                                                             \
  thumb_decode_branch();                                                      \
  u32 offlo = (offset * 2) & 0xFF;                                            \
  u32 offhi = (offset * 2) >> 8;                                              \
  generate_update_pc(((pc + 2) | 0x01));                                      \
  thumb_generate_load_reg(reg_a1, REG_LR);                                    \
  thumb_generate_store_reg(reg_a0, REG_LR);                                   \
  generate_add_reg_reg_imm(reg_a0, reg_a1, offlo, 0);                         \
  if (offhi) {                                                                \
    generate_add_reg_reg_imm(reg_a0, reg_a0, offhi, arm_imm_lsl_to_rot(8));   \
  }                                                                           \
  generate_indirect_branch_cycle_update(thumb);                               \
}                                                                             \

#define thumb_bx()                                                            \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  thumb_generate_load_reg_pc(reg_a0, rs, 4);                                  \
  generate_indirect_branch_cycle_update(dual_thumb);                          \
}                                                                             \

#define thumb_process_cheats()                                                \
  generate_function_far_call(armfn_cheat_thumb);

#define arm_process_cheats()                                                  \
  generate_function_far_call(armfn_cheat_arm);

#define thumb_swi()                                                           \
  generate_function_far_call(armfn_swi_thumb);                              \
  write32((pc + 2));                                                          \
  /* We're in ARM mode now */                                                 \
  generate_branch(arm)                                                        \

// Use software division
void *div6, *divarm7;
#define arm_hle_div(cpu_mode)                                                 \
  cycle_count += 11 + 32;                                                     \
  generate_function_call(div6);
#define arm_hle_div_arm(cpu_mode)                                             \
  cycle_count += 14 + 32;                                                     \
  generate_function_call(divarm7);

#define generate_translation_gate(type)                                       \
  generate_update_pc(pc);                                                     \
  generate_indirect_branch_no_cycle_update(type)                              \


extern u32 st_handler_functions[4][17];
extern u32 ld_handler_functions[5][17];
extern u32 ld_swap_handler_functions[5][17];

// Tables used by the memory handlers (placed near reg_base)
extern u32 ld_lookup_tables[5][17];
extern u32 st_lookup_tables[4][17];

void init_emitter(bool must_swap) {
  int i;

  // Generate handler table
  memcpy(st_lookup_tables, st_handler_functions, sizeof(st_lookup_tables));
  // Issue faster paths if swapping is not required
  if (must_swap)
    memcpy(ld_lookup_tables, ld_swap_handler_functions, sizeof(ld_lookup_tables));
  else
    memcpy(ld_lookup_tables, ld_handler_functions, sizeof(ld_lookup_tables));

  rom_cache_watermark = INITIAL_ROM_WATERMARK;
  u8 *translation_ptr = (u8*)&rom_translation_cache[0];

  // Generate ARMv5+ division code, uses a mix of libgcc and some open bioses.
  // This is meant for ARMv5 or higher, uses CLZ

  // Invert operands for SWI 7 (divarm)
  divarm7 = translation_ptr;
  ARM_MOV_REG_REG(0, reg_a2, reg_x0);
  ARM_MOV_REG_REG(0, reg_x0, reg_x1);
  ARM_MOV_REG_REG(0, reg_x1, reg_a2);

  div6 = translation_ptr;
  // Save flags before using them
  generate_save_flags();
  // Stores result and remainder signs 
  ARM_ANDS_REG_IMM(0, reg_a2, reg_x1, 0x80, arm_imm_lsl_to_rot(24));
  ARM_EOR_REG_IMMSHIFT(0, reg_a2, reg_a2, reg_x0, ARMSHIFT_ASR, 1);

  // Make numbers positive if they are negative
  ARM_RSB_REG_IMM_COND(0, reg_x1, reg_x1, 0, 0, ARMCOND_MI);
  ARM_TST_REG_REG(0, reg_x0, reg_x0);
  ARM_RSB_REG_IMM_COND(0, reg_x0, reg_x0, 0, 0, ARMCOND_MI);

  // Calculates the number of iterations to division, and jumps to unrolled code
  ARM_CLZ(0, reg_a0, reg_x0);
  ARM_CLZ(0, reg_a1, reg_x1);
  ARM_SUBS_REG_REG(0, reg_a0, reg_a1, reg_a0);          // Align and check if a<b
  ARM_RSB_REG_IMM(0, reg_a0, reg_a0, 31, 0);
  ARM_MOV_REG_IMM_COND(0, reg_a0, 32, 0, ARMCOND_MI);   // Cap to 32 (skip division)
  ARM_ADD_REG_IMMSHIFT(0, reg_a0, reg_a0, reg_a0, ARMSHIFT_LSL, 1);
  ARM_MOV_REG_IMM(0, reg_a1, 0, 0);
  ARM_ADD_REG_IMMSHIFT(0, ARMREG_PC, ARMREG_PC, reg_a0, ARMSHIFT_LSL, 2);
  ARM_NOP(0);

  for (i = 31; i >= 0; i--) {
    ARM_CMP_REG_IMMSHIFT(0, reg_x0, reg_x1, ARMSHIFT_LSL, i);
    ARM_ADC_REG_REG(0, reg_a1, reg_a1, reg_a1);
    ARM_SUB_REG_IMMSHIFT_COND(0, reg_x0, reg_x0, reg_x1, ARMSHIFT_LSL, i, ARMCOND_HS);
  }

  ARM_MOV_REG_REG(0, reg_x1, reg_x0);
  ARM_MOV_REG_REG(0, reg_x0, reg_a1);
  // Negate result if sign is negative
  ARM_SHLS_IMM(0, reg_a2, reg_a2, 1);
  ARM_RSB_REG_IMM_COND(0, reg_x0, reg_x0, 0, 0, ARMCOND_HS);
  ARM_RSB_REG_IMM_COND(0, reg_x1, reg_x1, 0, 0, ARMCOND_MI);

  // Register R3 stores the abs(r0/r1), store it in the right reg/mem-reg
  generate_load_memreg(reg_a2, REG_CPSR);
  ARM_TST_REG_IMM8(0, reg_a2, 0x20);
  arm_generate_store_reg(reg_a1, 3 /* r3 */);
  ARM_MOV_REG_REG_COND(0, reg_x3, reg_a1, ARMCOND_NE);

  // Return and continue regular emulation
  generate_restore_flags();
  ARM_BX(0, ARMREG_LR);

  // Now generate BIOS hooks
  rom_cache_watermark = (u32)(translation_ptr - rom_translation_cache);
  init_bios_hooks();

  // Intialize function table
  reg[REG_USERDEF + armfn_gbaup_idle_arm]   = (u32)arm_update_gba_idle_arm;
  reg[REG_USERDEF + armfn_gbaup_idle_thumb] = (u32)arm_update_gba_idle_thumb;
  reg[REG_USERDEF + armfn_gbaup_arm]   = (u32)arm_update_gba_arm;
  reg[REG_USERDEF + armfn_gbaup_thumb] = (u32)arm_update_gba_thumb;
  reg[REG_USERDEF + armfn_swi_arm]   = (u32)execute_swi_arm;
  reg[REG_USERDEF + armfn_swi_thumb] = (u32)execute_swi_thumb;
  reg[REG_USERDEF + armfn_cheat_arm]   = (u32)arm_cheat_hook;
  reg[REG_USERDEF + armfn_cheat_thumb] = (u32)thumb_cheat_hook;
  reg[REG_USERDEF + armfn_store_cpsr]   = (u32)execute_store_cpsr;
  reg[REG_USERDEF + armfn_spsr_restore] = (u32)execute_spsr_restore;
  reg[REG_USERDEF + armfn_indirect_arm]   = (u32)arm_indirect_branch_arm;
  reg[REG_USERDEF + armfn_indirect_thumb] = (u32)arm_indirect_branch_thumb;
  reg[REG_USERDEF + armfn_indirect_dual_arm]   = (u32)arm_indirect_branch_dual_arm;
  reg[REG_USERDEF + armfn_indirect_dual_thumb] = (u32)arm_indirect_branch_dual_thumb;
  reg[REG_USERDEF + armfn_debug_trace] = (u32)trace_instruction;
}

u32 execute_arm_translate_internal(u32 cycles, void *regptr);
u32 execute_arm_translate(u32 cycles) {
  return execute_arm_translate_internal(cycles, &reg[0]);
}

#endif
