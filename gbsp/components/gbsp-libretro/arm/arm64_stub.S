# gameplaySP
#
# Copyright (C) 2021 David Guillen Fandos <david@davidgf.net>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA


#include "../gpsp_config.h"

#define defsymbl(symbol) \
.align 2;                \
.type symbol, %function ;\
.global symbol ;         \
.global _##symbol ;      \
symbol:                  \
_##symbol:

.text
.align 2

#define REG_R0            (0 * 4)
#define REG_R1            (1 * 4)
#define REG_R2            (2 * 4)
#define REG_R3            (3 * 4)
#define REG_R4            (4 * 4)
#define REG_R5            (5 * 4)
#define REG_R6            (6 * 4)
#define REG_R7            (7 * 4)
#define REG_R8            (8 * 4)
#define REG_R9            (9 * 4)
#define REG_R10           (10 * 4)
#define REG_R11           (11 * 4)
#define REG_R12           (12 * 4)
#define REG_R13           (13 * 4)
#define REG_R14           (14 * 4)
#define REG_SP            (13 * 4)
#define REG_LR            (14 * 4)
#define REG_PC            (15 * 4)
#define REG_CPSR          (16 * 4)
#define CPU_MODE          (17 * 4)
#define CPU_HALT_STATE    (18 * 4)

#define REG_BUS_VALUE     (19 * 4)
#define REG_N_FLAG        (20 * 4)
#define REG_Z_FLAG        (21 * 4)
#define REG_C_FLAG        (22 * 4)
#define REG_V_FLAG        (23 * 4)
#define REG_SLEEP_CYCLES  (24 * 4)
#define OAM_UPDATED       (25 * 4)
#define REG_SAVE          (26 * 4)

#define CPU_ALERT_HALT_B        0
#define CPU_ALERT_SMC_B         1
#define CPU_ALERT_IRQ_B         2

#define reg_base          x20
#define reg_cycles        w21

#define reg_c_flag        w22
#define reg_v_flag        w23
#define reg_z_flag        w24
#define reg_n_flag        w25
#define reg_save0         w19


// Memory offsets from reg_base to the different buffers
#define RDMAP_OFF      -0xB9000   // 8K pointers (64KB)
#define IWRAM_OFF      -0xA9000   // 32KB (double for shadow)
#define VRAM_OFF       -0x99000   // 96KB
#define EWRAM_OFF      -0x81000   // 256KB (double for shadow)
#define MEM_TBL_OFF     -0x1000   // Some space for the tables
#define SPSR_RAM_OFF      0x100
#define REGMODE_RAM_OFF   0x118
#define OAM_RAM_OFF       0x200
#define PAL_RAM_OFF       0x600
#define IOREG_OFF         0xA00
#define PALCNV_RAM_OFF    0xE00

// Used for SWI handling
#define MODE_SUPERVISOR       0x13
#define SUPERVISOR_SPSR      (SPSR_RAM_OFF + 3*4)  // spsr[3]
#define SUPERVISOR_LR        (REGMODE_RAM_OFF + (3 * (7 * 4)) + (6 * 4))  // reg_mode[3][6]


// Stores and restores registers to their register storage in RAM

#define load_registers()                                                     ;\
  ldp  w6,  w7, [reg_base,  #0]                                              ;\
  ldp  w8,  w9, [reg_base,  #8]                                              ;\
  ldp w10, w11, [reg_base, #16]                                              ;\
  ldp w12, w13, [reg_base, #24]                                              ;\
  ldp w14, w15, [reg_base, #32]                                              ;\
  ldp w16, w17, [reg_base, #40]                                              ;\
  ldp w26, w27, [reg_base, #48]                                              ;\
  ldr w28,      [reg_base, #56]                                              ;\

#define store_registers()                                                    ;\
  stp  w6,  w7, [reg_base,  #0]                                              ;\
  stp  w8,  w9, [reg_base,  #8]                                              ;\
  stp w10, w11, [reg_base, #16]                                              ;\
  stp w12, w13, [reg_base, #24]                                              ;\
  stp w14, w15, [reg_base, #32]                                              ;\
  stp w16, w17, [reg_base, #40]                                              ;\
  stp w26, w27, [reg_base, #48]                                              ;\
  str w28,      [reg_base, #56]                                              ;\


// Extracts flags from CPSR into the cache flag registers

#define extract_flags_reg(tmpreg)                                            ;\
  ubfx reg_n_flag, tmpreg, #31, #1                                           ;\
  ubfx reg_z_flag, tmpreg, #30, #1                                           ;\
  ubfx reg_c_flag, tmpreg, #29, #1                                           ;\
  ubfx reg_v_flag, tmpreg, #28, #1                                           ;\

#define extract_flags(tmpreg)                                                ;\
  ldr tmpreg, [reg_base, #REG_CPSR]                                          ;\
  extract_flags_reg(tmpreg)                                                  ;\

// Collects cache flag bits and consolidates them to the CPSR reg

#define consolidate_flags(tmpreg)                                            ;\
  ldr tmpreg, [reg_base, #REG_CPSR]                                          ;\
  bfi tmpreg, reg_n_flag, #31, #1                                            ;\
  bfi tmpreg, reg_z_flag, #30, #1                                            ;\
  bfi tmpreg, reg_c_flag, #29, #1                                            ;\
  bfi tmpreg, reg_v_flag, #28, #1                                            ;\
  str tmpreg, [reg_base, #REG_CPSR]                                          ;\


// Update the GBA hardware (video, sound, input, etc)
// w0: current PC

defsymbl(a64_update_gba)
  str w0, [reg_base, #REG_PC]             // update the PC value
  str lr, [reg_base, #REG_SAVE]           // Save LR for later if needed

  consolidate_flags(w0)                   // update the CPSR
  store_registers()                       // save out registers

  mov w0, reg_cycles                      // load remaining cycles
  bl update_gba                           // update GBA state

  tbnz w0, #31, return_to_main            // exit if a new frame is ready

  // Resume execution (perhaps from a new PC)
  and reg_cycles, w0, 0x7fff              // load new cycle count
  extract_flags(w2)                       // reload flag cache bits

  tbnz w0, #30, 1f                        // check if PC changed

  ldr lr, [reg_base, #REG_SAVE]           // Restore return point
  load_registers()                        // reload registers
  ret                                     // resume execution, no PC change

1:  // Resume from new PC
  ldr w0, [reg_base, #REG_PC]             // load new PC
  tbnz w2, #5, 2f                         // CPSR.T means in thumb mode

  bl block_lookup_address_arm
  load_registers()                        // reload registers
  br x0                                   // jump to new ARM block
2:
  bl block_lookup_address_thumb
  load_registers()                        // reload registers
  br x0                                   // jump to new Thumb block
.size a64_update_gba, .-a64_update_gba


// Cheat hooks for master function
// This is called whenever PC == cheats-master-function
// Just calls the C function to process cheats

defsymbl(a64_cheat_hook)
  store_registers()
  str lr, [reg_base, #REG_SAVE]
  bl process_cheats
  ldr lr, [reg_base, #REG_SAVE]
  load_registers()
  ret


// These are b stubs for performing indirect branches. They are not
// linked to and don't return, instead they link elsewhere.

// Input:
// r0: PC to branch to

defsymbl(a64_indirect_branch_arm)
  store_registers()
  bl block_lookup_address_arm
  load_registers()
  br x0

defsymbl(a64_indirect_branch_thumb)
  store_registers()
  bl block_lookup_address_thumb
  load_registers()
  br x0

defsymbl(a64_indirect_branch_dual)
  store_registers()
  bl block_lookup_address_dual
  load_registers()
  br x0


// Read CPSR and SPSR values

defsymbl(execute_read_cpsr)
  consolidate_flags(w0)                   // Consolidate on ret value
  ret

defsymbl(execute_read_spsr)
  ldr w1, [reg_base, #CPU_MODE]           // read cpu mode to w1
  and w1, w1, #0xF                        // Like REG_SPSR() macro
  add x0, reg_base, #SPSR_RAM_OFF         // ptr to spsr table
  ldr w0, [x0, x1, lsl #2]                // Read actual value from table
  ret


// Update the cpsr.

// Input:
// w0: new cpsr value
// w1: current PC
// w2: store bitmask (user-mode)
// w3: store bitmask (privileged mode)

defsymbl(execute_store_cpsr)
  ldr w4, [reg_base, #CPU_MODE]           // w4 = cpu_mode
  tst x4, #0x10                           // Bit 4 is set on privileged modes
  csel x2, x2, x3, eq                     // Select the correct mask

  ldr w4, [reg_base, #REG_CPSR]           // read current CPSR
  and w3, w0, w2                          // reg_flags = new_cpsr & store_mask
  bic w4, w4, w2                          // current_cpsr & ~store_mask
  orr w0, w3, w4                          // w2 = final CPSR value
  extract_flags_reg(w0)                   // Update cached flags too

  str lr, [reg_base, #REG_SAVE]
  store_registers()
  bl execute_store_cpsr_body              // Do the remaining work in C mode

  cbnz w0, 1f                             // If PC has changed due to this

  ldr lr, [reg_base, #REG_SAVE]           // Resume execution where we left it
  load_registers()
  ret

1:
  // Returned value contains the PC, resume execution there
  bl block_lookup_address_arm
  load_registers()
  br x0                                   // Resume in the returned block
.size execute_store_cpsr, .-execute_store_cpsr


// Write to SPSR
// w0: new SPSR value
// w1: store mask

defsymbl(execute_store_spsr)
  ldr w2, [reg_base, #CPU_MODE]           // read cpu mode to w1
  and w2, w2, #0xF                        // Like REG_SPSR() macro
  add x2, reg_base, x2, lsl #2            // calculate table offset
  ldr w3, [x2, #SPSR_RAM_OFF]             // Read actual value from trable

  and w0, w0, w1                          // new-spsr & mask
  bic w3, w3, w1                          // old-spsr & ~mask
  orr w0, w0, w3                          // final spsr value

  str w0, [x2, #SPSR_RAM_OFF]             // Store new SPSR
  ret
.size execute_store_spsr, .-execute_store_spsr

// Restore the cpsr from the mode spsr and mode shift.

// Input:
// r0: current pc

defsymbl(execute_spsr_restore)
  ldr w1, [reg_base, #CPU_MODE]           // w1 = cpu_mode
  and w1, w1, 0xF                         // Fold user and system modes
  cbz w1, 1f                              // Ignore if in user or system mode

  lsl w2, w1, #2                          // We access 32 bit words
  add w2, w2, #SPSR_RAM_OFF
  ldr w3, [reg_base, x2]                  // w3 = spsr[cpu_mode]
  str w3, [reg_base, #REG_CPSR]           // update CPSR with SPSR value
  extract_flags_reg(w3)                   // update cached flag values

  // This function call will pass r0 (address) and return it.
  str lr, [reg_base, #REG_SAVE]
  store_registers()                       // save ARM registers
  bl execute_spsr_restore_body
  ldr lr, [reg_base, #REG_SAVE]
  load_registers()

1:
  ret
.size execute_spsr_restore, .-execute_spsr_restore


// Setup the mode transition work for calling an SWI.

// Input:
// r0: current pc

defsymbl(execute_swi)
  str lr, [reg_base, #REG_SAVE]
  str w0, [reg_base, #SUPERVISOR_LR]      // Store next PC into supervisor LR
  consolidate_flags(w1)                   // Calculate current CPSR flags
  str w1, [reg_base, #SUPERVISOR_SPSR]    // Store them in the SPSR  
  bic w1, w1, #0x3F                       // Clear mode bits
  mov w2, #(0x13 | 0x80)                  // Set supervisor mode bits
  orr w1, w1, w2
  str w1, [reg_base, #REG_CPSR]           // Update CPSR with new value
  store_registers()
  mov w0, #MODE_SUPERVISOR
  bl set_cpu_mode                         // Set supervisor mode
  ldr w0, =0xe3a02004
  str w0, [reg_base, REG_BUS_VALUE]
  ldr lr, [reg_base, #REG_SAVE]
  load_registers()
  ret
.size execute_swi, .-execute_swi

defsymbl(execute_arm_translate_internal)
  // save registers that will be clobbered
  sub sp, sp, #96
  stp x19, x20, [sp,  #0]
  stp x21, x22, [sp, #16]
  stp x23, x24, [sp, #32]
  stp x25, x26, [sp, #48]
  stp x27, x28, [sp, #64]
  stp x29, x30, [sp, #80]
  
  mov reg_cycles, w0                      // load cycle counter
  mov reg_base, x1                        // init base_reg

  // Check whether the CPU is sleeping already, we should just wait for IRQs
  ldr w1, [reg_base, #CPU_HALT_STATE]
  cbnz w1, cpu_sleep_loop

  ldr w0, [reg_base, #REG_PC]             // load current PC

// Resume execution at PC (at w0)
lookup_pc:
  ldr w1, [reg_base, #REG_CPSR]           // w1 = flags
  extract_flags_reg(w1)
  tbnz w1, #5, 2f                         // see if Thumb bit is set

  // Lookup and jump to the right mode block
  bl block_lookup_address_arm
  load_registers()
  br x0
2:
  bl block_lookup_address_thumb
  load_registers()
  br x0

// Epilogue to return to the main thread (whatever called execute_arm_translate)

return_to_main:
  // restore the saved regs and return
  ldp x19, x20, [sp,  #0]
  ldp x21, x22, [sp, #16]
  ldp x23, x24, [sp, #32]
  ldp x25, x26, [sp, #48]
  ldp x27, x28, [sp, #64]
  ldp x29, x30, [sp, #80]
  add sp, sp, #96
  ret


// Memory read stub routines

#define execute_load_builder(load_type, ldop, ldmask, tblidx, ldfn)          ;\
                                                                             ;\
defsymbl(execute_load_##load_type)                                           ;\
  tst w0, #(0xf0000000 | ldmask)                                             ;\
  lsr w3, w0, #24                                                            ;\
  csinc	w3, wzr, w3, ne                                                      ;\
  add x4, reg_base, (MEM_TBL_OFF + tblidx*136)                               ;\
  ldr x3, [x4, x3, lsl #3]                                                   ;\
  br x3                                                                      ;\
                                                                             ;\
ld_bios_##load_type:                      /* BIOS area, need to verify PC  */;\
  lsr w3, w1, #24                         /* Are we running the BIOS       */;\
  cbnz w3, ld_slow_##load_type                                               ;\
  and w0, w0, #(0x7fff)                   /* BIOS only 16 KB               */;\
  add x3, reg_base, #(RDMAP_OFF)                                             ;\
  ldr x3, [x3]                            /* x3 = bios mem buffer          */;\
  ldop w0, [x3, x0]                       /* load actual value             */;\
  ret                                                                        ;\
                                                                             ;\
ld_ewram_##load_type:                     /* EWRAM area                    */;\
  and w0, w0, #(0x3ffff)                                                     ;\
  add x3, reg_base, #EWRAM_OFF                                               ;\
  ldop w0, [x3, x0]                                                          ;\
  ret                                                                        ;\
                                                                             ;\
ld_iwram_##load_type:                     /* IWRAM area                    */;\
  and w0, w0, #(0x7fff)                                                      ;\
  add x3, reg_base, #(IWRAM_OFF+0x8000)                                      ;\
  ldop w0, [x3, x0]                                                          ;\
  ret                                                                        ;\
                                                                             ;\
ld_ioram_##load_type:                     /* I/O RAM area                  */;\
  and w0, w0, #(0x3ff)                                                       ;\
  add x3, reg_base, #(IOREG_OFF)                                             ;\
  ldop w0, [x3, x0]                                                          ;\
  ret                                                                        ;\
                                                                             ;\
ld_palram_##load_type:                    /* PAL RAM area                  */;\
  and w0, w0, #(0x3ff)                                                       ;\
  add x3, reg_base, #(PAL_RAM_OFF)                                           ;\
  ldop w0, [x3, x0]                                                          ;\
  ret                                                                        ;\
                                                                             ;\
ld_oamram_##load_type:                    /* OAM RAM area                  */;\
  and w0, w0, #(0x3ff)                                                       ;\
  add x3, reg_base, #(OAM_RAM_OFF)                                           ;\
  ldop w0, [x3, x0]                                                          ;\
  ret                                                                        ;\
                                                                             ;\
ld_rdmap_##load_type:                                                        ;\
  lsr w4, w0, #15                         /* Each block is 32KB            */;\
  add x3, reg_base, #(RDMAP_OFF)                                             ;\
  ldr x4, [x3, x4, lsl #3]                /* x4 = table pointer            */;\
  cbz x4, ld_slow_##load_type             /* not mapped, go slow */          ;\
  and w0, w0, #(0x7fff)                   /* 32KB pages                    */;\
  ldop w0, [x4, x0]                       /* load actual value             */;\
  ret                                                                        ;\
                                                                             ;\
ld_slow_##load_type:                      /* Slow C path                   */;\
  str w1, [reg_base, #REG_PC]             /* write out PC                  */;\
  str lr, [reg_base, #REG_SAVE]           /* Save LR                       */;\
  store_registers()                                                          ;\
  bl ldfn                                                                    ;\
  ldr lr, [reg_base, #REG_SAVE]                                              ;\
  load_registers()                                                           ;\
  ret                                                                        ;\
.size execute_load_##load_type, .-execute_load_##load_type

#define load_lookup_table(load_type, aload_type)                             ;\
  .quad ld_slow_##aload_type              /* -1: Unaligned/Bad access      */;\
  .quad ld_bios_##aload_type              /* 0x00: BIOS                    */;\
  .quad ld_slow_##aload_type              /* 0x01: Open bus                */;\
  .quad ld_ewram_##load_type              /* 0x02: ewram                   */;\
  .quad ld_iwram_##load_type              /* 0x03: iwram                   */;\
  .quad ld_ioram_##load_type              /* 0x04: I/O regs                */;\
  .quad ld_palram_##load_type             /* 0x05: palette RAM             */;\
  .quad ld_rdmap_##load_type              /* 0x06: vram                    */;\
  .quad ld_oamram_##load_type             /* 0x07: oam ram                 */;\
  .quad ld_rdmap_##load_type              /* 0x08: gamepak: ignore         */;\
  .quad ld_rdmap_##load_type              /* 0x09: gamepak: ignore         */;\
  .quad ld_rdmap_##load_type              /* 0x0A: gamepak: ignore         */;\
  .quad ld_rdmap_##load_type              /* 0x0B: gamepak: ignore         */;\
  .quad ld_rdmap_##load_type              /* 0x0C: gamepak: ignore         */;\
  .quad ld_slow_##aload_type              /* 0x0D: EEPROM                  */;\
  .quad ld_slow_##aload_type              /* 0x0E: backup                  */;\
  .quad ld_slow_##aload_type              /* 0x0F: ignore                  */;\

// Aligned load is a bit special
defsymbl(execute_aligned_load32)
  tst w0, #(0xf0000000)
  lsr w3, w0, #24
  csinc	w3, wzr, w3, ne
  add x4, reg_base, (MEM_TBL_OFF + 5*136)
  ldr x3, [x4, x3, lsl #3]
  br x3
ld_slow_aligned_u32:                      // Slow C path for multiple loads
  str lr, [reg_base, #REG_SAVE]           // Save LR
  store_registers()
  bl read_memory32
  ldr lr, [reg_base, #REG_SAVE]
  load_registers()
  ret
ld_bios_aligned_u32:
  and w0, w0, #(0x7fff)                   // Do not verify PC on purpose
  add x3, reg_base, #(RDMAP_OFF)
  ldr x3, [x3]
  ldr w0, [x3, x0]
  ret


execute_load_builder( u8,  ldrb, 0, 0, read_memory8)
execute_load_builder( s8, ldrsb, 0, 1, read_memory8s)
execute_load_builder(u16,  ldrh, 1, 2, read_memory16)
execute_load_builder(s16, ldrsh, 1, 3, read_memory16s)
execute_load_builder(u32,   ldr, 3, 4, read_memory32)


// Prepares for a external store (calls C code)
#define store_align_8()            and w1, w1, #0xff
#define store_align_16()           and w1, w1, #0xffff; bic w0, w0, #1
#define store_align_32()           bic w0, w0, #3

// For byte-accesses on 16 bit buses
#define dup8(reg)  bfi reg, reg, #8, #24    // Duplicates byte to u16
#define dup16(reg)
#define dup32(reg)

// Write out to memory.

// Input:
// w0: address
// w1: value
// w2: PC value

#define execute_store_builder(store_type, str_op, str_op16, load_op,          \
                              stmask, stmask16, tblidx)                      ;\
                                                                             ;\
defsymbl(execute_store_u##store_type)                                        ;\
  lsr w4, w0, #28                                                            ;\
  lsr w3, w0, #24                                                            ;\
  cbnz w4, ext_store_u##store_type                                           ;\
  add x4, reg_base, (MEM_TBL_OFF + 816 + tblidx*128)                         ;\
  ldr x3, [x4, x3, lsl #3]                                                   ;\
  br x3                                                                      ;\
                                                                             ;\
ext_store_u##store_type:                                                     ;\
ext_store_u##store_type##_safe:                                              ;\
  str w2, [reg_base, #REG_PC]             /* write out PC                  */;\
  str lr, [reg_base, #REG_SAVE]           /* Preserve LR                   */;\
  store_align_##store_type()                                                 ;\
  store_registers()                                                          ;\
  bl write_memory##store_type                                                ;\
  ldr lr, [reg_base, #REG_SAVE]                                              ;\
  load_registers()                                                           ;\
  ret                                     /* resume if no side effects     */;\
                                                                             ;\
ext_store_iwram_u##store_type:                                               ;\
  and w0, w0, #(0x7fff & ~stmask)         /* Mask to mirror memory (+align)*/;\
  add x3, reg_base, #(IWRAM_OFF+0x8000)   /* x3 = iwram base               */;\
  str_op w1, [x0, x3]                     /* store data                    */;\
  sub x3, x3, #0x8000                     /* x3 = iwram smc base           */;\
  load_op w1, [x0, x3]                    /* w1 = SMC sentinel             */;\
  cbnz w1, 3f                             /* Check value, should be zero   */;\
  ret                                     /* return                        */;\
                                                                             ;\
ext_store_ewram_u##store_type:                                               ;\
  and w0, w0, #(0x3ffff & ~stmask)        /* Mask to mirror memory (+align)*/;\
  add x3, reg_base, #EWRAM_OFF            /* x3 = ewram base               */;\
  str_op w1, [x0, x3]                     /* store data                    */;\
  add x3, x3, #0x40000                    /* x3 = ewram smc base           */;\
  load_op w1, [x0, x3]                    /* w1 = SMC sentinel             */;\
  cbnz w1, 3f                             /* Check value, should be zero   */;\
  ret                                     /* return                        */;\
                                                                             ;\
ext_store_vram_u##store_type:                                                ;\
ext_store_vram_u##store_type##_safe:                                         ;\
  dup##store_type(w1)                     /* Duplicate byte if necessary   */;\
  and w0, w0, #(0x1ffff & ~stmask16)      /* Mask to mirror memory (+align)*/;\
  sub w3, w0, #0x8000                     /* Mirrored addr for last bank   */;\
  cmp w0, #0x18000                        /* Check if exceeds 96KB         */;\
  csel w0, w3, w0, cs                     /* If it does, pick the mirror   */;\
  add x3, reg_base, #VRAM_OFF             /* x3 = ewram base               */;\
  str_op16 w1, [x0, x3]                   /* store data                    */;\
  ret                                     /* return                        */;\
                                                                             ;\
ext_store_oam_ram_u##store_type:                                             ;\
ext_store_oam_ram_u##store_type##_safe:                                      ;\
  dup##store_type(w1)                     /* Duplicate byte if necessary   */;\
  and w0, w0, #(0x3ff & ~stmask16)        /* Mask to mirror memory (+align)*/;\
  add x3, reg_base, #OAM_RAM_OFF          /* x3 = oam ram base             */;\
  str_op16 w1, [x0, x3]                   /* store data                    */;\
  str w29, [reg_base, #OAM_UPDATED]       /* write non zero to signal      */;\
  ret                                     /* return                        */;\
                                                                             ;\
ext_store_ioreg_u##store_type:                                               ;\
  str w2, [reg_base, #REG_PC]             /* write out PC                  */;\
  str lr, [reg_base, #REG_SAVE]           /* Preserve LR                   */;\
  and w0, w0, #(0x3ff & ~stmask)                                             ;\
  store_registers()                                                          ;\
  bl write_io_register##store_type                                           ;\
  cbnz w0, write_epilogue                 /* handle additional write stuff */;\
  ldr lr, [reg_base, #REG_SAVE]                                              ;\
  load_registers()                                                           ;\
  ret                                     /* resume if no side effects     */;\
                                                                             ;\
3: /* SMC write (iwram/ewram) */                                             ;\
  str w2, [reg_base, #REG_PC]             /* write out PC                  */;\
  store_registers()                       /* store registers               */;\
  consolidate_flags(w1)                                                      ;\
  bl flush_translation_cache_ram                                             ;\
  ldr w0, [reg_base, #REG_PC]             /* load "current new" PC         */;\
  b lookup_pc                             /* continue execution            */;\
.size execute_store_u##store_type, .-execute_store_u##store_type

// for ignored areas, just return
ext_store_ignore:
  ret                                     // return

#define store_lookup_table(store_type)                                       ;\
  .quad ext_store_ignore                  /* 0x00: BIOS, ignore            */;\
  .quad ext_store_ignore                  /* 0x01: ignore                  */;\
  .quad ext_store_ewram_u##store_type     /* 0x02: ewram                   */;\
  .quad ext_store_iwram_u##store_type     /* 0x03: iwram                   */;\
  .quad ext_store_ioreg_u##store_type     /* 0x04: I/O regs                */;\
  .quad ext_store_palette_u##store_type   /* 0x05: palette RAM             */;\
  .quad ext_store_vram_u##store_type      /* 0x06: vram                    */;\
  .quad ext_store_oam_ram_u##store_type   /* 0x07: oam ram                 */;\
  .quad ext_store_u##store_type           /* 0x08: gamepak: ignore         */;\
  .quad ext_store_u##store_type           /* 0x09: gamepak: ignore         */;\
  .quad ext_store_u##store_type           /* 0x0A: gamepak: ignore         */;\
  .quad ext_store_u##store_type           /* 0x0B: gamepak: ignore         */;\
  .quad ext_store_u##store_type           /* 0x0C: gamepak: ignore         */;\
  .quad ext_store_u##store_type           /* 0x0D: EEPROM                  */;\
  .quad ext_store_u##store_type           /* 0x0E: backup                  */;\
  .quad ext_store_ignore                  /* 0x0F: ignore                  */;\

execute_store_builder(8,  strb, strh, ldrb, 0, 1, 0)
execute_store_builder(16, strh, strh, ldrh, 1, 1, 1)
execute_store_builder(32, str,  str,  ldr,  3, 3, 2)

// Palette writes are special since they are converted on the fly for speed

ext_store_palette_u8:
  bfi w1, w1, #8, #24                     // Duplicate the byte
ext_store_palette_u16:
  and w0, w0, #(0x3fe)
  add x3, reg_base, #(PAL_RAM_OFF)
  strh w1, [x3, x0]

  ubfx w2, w1, #10, #5                    // Extract blue to red
  bfi  w2, w1, #11, #5                    // Move red to blue
  and  w1, w1, #0x03E0                    // Extract green component
  orr  w1, w2, w1, lsl #1                 // Merge the three components

  add x3, reg_base, #(PALCNV_RAM_OFF)
  strh w1, [x3, x0]
  ret

ext_store_palette_u32_safe:
ext_store_palette_u32:
  and w0, w0, #(0x3fc)
  add x3, reg_base, #(PAL_RAM_OFF)
  str w1, [x3, x0]

  and   w2, w1, #0x7C007C00               // Get blue components
  and   w3, w1, #0x001F001F               // Get red components
  lsr   w2, w2, #10                       // Place blue in the final register
  orr   w2, w2, w3, lsl #11               // Merge red
  and   w3, w1, #0x03E003E0               // Get green component
  orr   w1, w2, w3, lsl #1                // Merge green

  add x3, reg_base, #(PALCNV_RAM_OFF)
  str w1, [x3, x0]
  ret

// This is a store that is executed in a strm case (so no SMC checks in-between)

defsymbl(execute_aligned_store32)
  lsr w4, w0, #28
  lsr w3, w0, #24
  cbnz w4, ext_store_u32
  add x4, reg_base, MEM_TBL_OFF + 816 + 3*128
  ldr x3, [x4, x3, lsl #3]
  br x3
ext_store_iwram_u32_safe:
  and w0, w0, #(0x7fff)                   // Mask to mirror memory (no need to align!)
  add x3, reg_base, #(IWRAM_OFF+0x8000)   // x3 = iwram base
  str w1, [x0, x3]                        // store data
  ret                                     // Return
ext_store_ewram_u32_safe:
  and w0, w0, #(0x3ffff)                  // Mask to mirror memory (no need to align!)
  add x3, reg_base, #(EWRAM_OFF)          // x3 = ewram base
  str w1, [x0, x3]                        // store data
  ret                                     // Return
ext_store_ioreg_u32_safe:
  str lr, [reg_base, #REG_SAVE]
  and w0, w0, #(0x3fc)
  store_registers()
  bl write_io_register32
  ldr lr, [reg_base, #REG_SAVE]
  load_registers()
  ret
.size execute_aligned_store32, .-execute_aligned_store32

// This is called whenever an external store with side effects was performed
write_epilogue:
  mov reg_save0, w0                       // Save reg for later
  consolidate_flags(w1)                   // Update CPSR for IRQ/
  tbz w0, #CPU_ALERT_SMC_B, 1f            // Skip if SMC did not happen
  bl flush_translation_cache_ram          // Flush RAM if bit is set

1:
  tbz reg_save0, #CPU_ALERT_IRQ_B, 2f     // Skip if IRQ did not happen
  bl check_and_raise_interrupts

2:
  ldr w0, [reg_base, #REG_PC]             // load new PC
  tbz reg_save0, #CPU_ALERT_HALT_B, lookup_pc   // Resume execution if running

  // explicit fallthrough to cpu_sleep_loop, while CPU is halted

cpu_sleep_loop:
  mov w0, reg_cycles                      // load remaining cycles
  bl update_gba                           // update GBA until CPU isn't halted

  tbnz w0, #31, return_to_main            // a frame has been completed -> exit

  // At this point the CPU must be active, otherwise we sping in update_gba

  and reg_cycles, w0, 0x7fff              // load new cycle count
  ldr w0, [reg_base, #REG_PC]             // load new PC
  b lookup_pc                             // Resume execution at that PC


.data
.align 4
defsymbl(ldst_handler_functions)
  load_lookup_table(u8,   u8)
  load_lookup_table(s8,   s8)
  load_lookup_table(u16, u16)
  load_lookup_table(s16, s16)
  load_lookup_table(u32, u32)
  load_lookup_table(u32, aligned_u32)
  store_lookup_table(8)
  store_lookup_table(16)
  store_lookup_table(32)
  store_lookup_table(32_safe)

.bss
.align 4

defsymbl(memory_map_read)
  .space 0x10000
defsymbl(iwram)
  .space 0x10000
defsymbl(vram)
  .space 0x18000
defsymbl(ewram)
  .space 0x80000
defsymbl(ldst_lookup_tables)
  .space 4096
defsymbl(reg)
  .space 0x100
defsymbl(spsr)
  .space 24
defsymbl(reg_mode)
  .space 196
  .space 36  // Padding
defsymbl(oam_ram)
  .space 0x400
defsymbl(palette_ram)
  .space 0x400
defsymbl(io_registers)
  .space 0x400
defsymbl(palette_ram_converted)
  .space 0x400


