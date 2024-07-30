# gameplaySP
#
# Copyright (C) 2006 Exophase <exophase@gmail.com>
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

.align 4

#define defsymbl(symbol) \
.global symbol ;         \
.global _##symbol ;      \
symbol:                  \
_##symbol:

// Windows 32 bit ABI prefixes functions with underscore
#if defined(_WIN32) && !defined(_WIN64)
  #define fnm(name) _##name
#else
  #define fnm(name) name
#endif

// Convention calls are different, and register allocations, which makes it tricky
// All functions in this file are called manually from the JIT arena (unless stated
// otherwise), where we use our own convention call. However calls to C code must
// follow the calling convention. x86 is built with regparm=2 to avoid stack usage.
#if defined(__x86_64__) || defined(__amd64__)
  #define JUMP_CX_ZERO     jrcxz
  #define ADDR_TYPE        .quad
  #define ADDR_SIZE_BYTES      8
  #define STACK_REG         %rsp
  #define FULLREG(rn)       %r##rn
  #define SAVE_REGISTERS    push %rbx; push %rsi; push %rdi; push %rbp
  #define REST_REGISTERS    pop  %rbp; pop  %rdi; pop  %rsi; pop  %rbx
  #define REG_BASE          %rbx
  #ifdef _WIN64
    #define CARG1_REG       %ecx   // Windows x64 ABI, of course different :D
    #define CARG2_REG       %edx
    #define CARG2_REGPTR    %rdx
    #define CALL_FUNC(name)         \
      sub $32, %rsp;                \
      call fnm(name);               \
      add $32, %rsp
  #else
    #define CARG1_REG       %edi   // SystemV AMD64 ABI
    #define CARG2_REG       %esi
    #define CARG2_REGPTR    %rsi
    #define CALL_FUNC(name)         \
      call fnm(name)
  #endif
  #define SETUP_ARGS        mov %eax, CARG1_REG; mov %edx, CARG2_REG;
#else
  #define JUMP_CX_ZERO     jecxz
  #define ADDR_TYPE        .long
  #define ADDR_SIZE_BYTES      4
  #define STACK_REG         %esp
  #define FULLREG(rn)       %e##rn
  #define SAVE_REGISTERS    sub $8, %esp; push %ebx; push %esi; push %edi; push %ebp
  #define REST_REGISTERS    pop  %ebp; pop  %edi; pop  %esi; pop  %ebx; add $8, %esp;
  #define REG_BASE          %ebx
  #define CARG1_REG         %eax
  #define CARG2_REG         %edx
  #define CARG2_REGPTR      %edx
  #define SETUP_ARGS
  #define CALL_FUNC(name)         \
    call fnm(name)
#endif

.equ CPU_ALERT_HALT,    (1 << 0)
.equ CPU_ALERT_SMC,     (1 << 1)
.equ CPU_ALERT_IRQ,     (1 << 2)

.equ REG_SP,            (13 * 4)
.equ REG_LR,            (14 * 4)
.equ REG_PC,            (15 * 4)
.equ REG_CPSR,          (16 * 4)
.equ CPU_MODE,          (17 * 4)
.equ CPU_HALT_STATE,    (18 * 4)

.equ REG_BUS_VALUE,     (19 * 4)
.equ REG_N_FLAG,        (20 * 4)
.equ REG_Z_FLAG,        (21 * 4)
.equ REG_C_FLAG,        (22 * 4)
.equ REG_V_FLAG,        (23 * 4)
.equ REG_SLEEP_CYCLES,  (24 * 4)
.equ OAM_UPDATED,       (25 * 4)
.equ REG_SAVE,          (26 * 4)

.equ load_u8_tbl,           -(9 * 16 * ADDR_SIZE_BYTES)
.equ load_s8_tbl,           -(8 * 16 * ADDR_SIZE_BYTES)
.equ load_u16_tbl,          -(7 * 16 * ADDR_SIZE_BYTES)
.equ load_s16_tbl,          -(6 * 16 * ADDR_SIZE_BYTES)
.equ load_u32_tbl,          -(5 * 16 * ADDR_SIZE_BYTES)
.equ store_u8_tbl,          -(4 * 16 * ADDR_SIZE_BYTES)
.equ store_u16_tbl,         -(3 * 16 * ADDR_SIZE_BYTES)
.equ store_u32_tbl,         -(2 * 16 * ADDR_SIZE_BYTES)
.equ store_aligned_u32_tbl, -(1 * 16 * ADDR_SIZE_BYTES)

.equ PALETTE_RAM_OFF,     0x0100
.equ PALETTE_RAM_CNV_OFF, 0x0500
.equ OAM_RAM_OFF,         0x0900
.equ IWRAM_OFF,           0x0D00
.equ VRAM_OFF,           0x10D00
.equ EWRAM_OFF,          0x28D00
.equ IORAM_OFF,          0xA8D00
.equ SPSR_OFF,           0xA9100
.equ RDMAP_OFF,          0xA9200

#define REG_CYCLES          %ebp

# destroys ecx and edx

.macro collapse_flag offset, shift
  mov \offset(REG_BASE), %ecx
  shl $\shift, %ecx
  or %ecx, %edx
.endm

.macro collapse_flags_no_update
  xor %edx, %edx
  collapse_flag REG_N_FLAG, 31
  collapse_flag REG_Z_FLAG, 30
  collapse_flag REG_C_FLAG, 29
  collapse_flag REG_V_FLAG, 28
  mov REG_CPSR(REG_BASE), %ecx
  and $0xFF, %ecx
  or %ecx, %edx
.endm


.macro collapse_flags
  collapse_flags_no_update
  mov %edx, REG_CPSR(REG_BASE)
.endm

.macro extract_flag shift, offset
  mov REG_CPSR(REG_BASE), %edx
  shr $\shift, %edx
  and $0x01, %edx
  mov %edx, \offset(REG_BASE)
.endm

.macro extract_flags
  extract_flag 31, REG_N_FLAG
  extract_flag 30, REG_Z_FLAG
  extract_flag 29, REG_C_FLAG
  extract_flag 28, REG_V_FLAG
.endm

# Process a hardware event. Since an interrupt might be
# raised we have to check if the PC has changed.

# arg0 (always in eax): current PC address
defsymbl(x86_update_gba)
  mov %eax, REG_PC(REG_BASE)          # current PC = eax
  collapse_flags                      # update cpsr, trashes ecx and edx

  mov REG_CYCLES, CARG1_REG           # Load remaining cycles as arg0
  CALL_FUNC(update_gba)               # process the next event

  # did we just complete a frame? go back to main then
  test %eax, %eax                     # Bit 31 set, means new frame completed
  js return_to_main

  # Load new cycle counter
  mov %eax, REG_CYCLES                # new cycle count
  and $0x7fff, REG_CYCLES             # in the lowest bits

  # did the PC change? Bit 30 will be set
  test $0x40000000, %eax
  jne lookup_pc
  ret                                 # otherwise, go back to caller (resume)

# Perform this on an indirect branch that will definitely go to
# ARM code, IE anything that changes the PC in ARM mode except
# for BX and data processing to PC with the S bit set.

# arg0 (always in eax): GBA address to branch to
defsymbl(x86_indirect_branch_arm)
  mov %eax, CARG1_REG
  CALL_FUNC(block_lookup_address_arm)
  add $ADDR_SIZE_BYTES, STACK_REG    # remove current return addr
  jmp *FULLREG(ax)

# For indirect branches that'll definitely go to Thumb. In
# Thumb mode any indirect branches except for BX.

# arg0 (always in eax): GBA address to branch to
defsymbl(x86_indirect_branch_thumb)
  mov %eax, CARG1_REG
  CALL_FUNC(block_lookup_address_thumb)
  add $ADDR_SIZE_BYTES, STACK_REG    # remove current return addr
  jmp *FULLREG(ax)

# For indirect branches that can go to either Thumb or ARM,
# mainly BX (also data processing to PC with S bit set, be
# sure to adjust the target with a 1 in the lowest bit for this)

# arg0 (always in eax): GBA address to branch to
defsymbl(x86_indirect_branch_dual)
  mov %eax, CARG1_REG
  CALL_FUNC(block_lookup_address_dual)
  add $ADDR_SIZE_BYTES, STACK_REG    # remove current return addr
  jmp *FULLREG(ax)


# General ext memory routines

ext_store_gpio8:              # No GPIO/RTC writes on byte or word access
ext_store_gpio32:
ext_store_backup16:           # Backup (flash) accessed via byte writes
ext_store_backup32:
ext_store_eeprom8:            # EEPROM accesses are performed using 16 bit DMA
ext_store_eeprom32:
ext_store_ignore:
  ret                         # ignore these writes

ext_store_gpio16:
  and $0xFFFF, %edx           # make value 16bit
  and $0xFF, %eax             # mask address
  SETUP_ARGS                  # Setup addr, value
  CALL_FUNC(write_gpio)       # write out RTC register
  ret

ext_store_backup8:
  and $0xFF, %edx             # make value 8bit
  and $0xFFFF, %eax           # mask address
  SETUP_ARGS                  # Setup addr, value
  CALL_FUNC(write_backup)     # perform backup write
  ret


# Handle I/O write side-effects:
#  SMC: Flush RAM caches
#  IRQ: Perform CPU mode change
#  HLT: spin in the cpu_sleep_loop until an IRQ is triggered
write_epilogue:
  mov %eax, REG_SAVE(REG_BASE)# Save ret value for later use
  collapse_flags              # Consolidate CPSR
  test $CPU_ALERT_SMC, %eax   # Check for CPU_ALERT_SMC bit
  jz 1f                       # skip if not set
  CALL_FUNC(flush_translation_cache_ram)

1:
  testl $CPU_ALERT_IRQ, REG_SAVE(REG_BASE) # Check for CPU_ALERT_IRQ bit
  jz 2f                       # skip if not set
  CALL_FUNC(check_and_raise_interrupts)

2:
  testl $CPU_ALERT_HALT, REG_SAVE(REG_BASE) # Check for CPU_ALERT_HALT bit
  jz lookup_pc                # if not halt, continue executing

  # explicit fallthrough to cpu_sleep_loop, while CPU is halted

cpu_sleep_loop:
  mov REG_CYCLES, CARG1_REG   # Load remaining cycles as arg0
  CALL_FUNC(update_gba)       # process the next event

  # did we just complete a frame (MSB set)? go back to main then
  test %eax, %eax
  js return_to_main

  // if we are out of update_gba and did not complete a frame, cpu is active
  mov %eax, REG_CYCLES        # load new cycle count
  and $0x7fff, REG_CYCLES     # (only lowest bits)
  jmp lookup_pc               # pc changes after a halt


ext_store_eeprom16:
  CALL_FUNC(write_eeprom)     # perform eeprom write
  ret


# Register wrapping for various sizes
#define reg32(n) %e##n##x
#define reg16(n)  %##n##x
#define reg8(n)   %##n##l

# 16 bit bus results in duplicated 8bit accesses
#define dup8()  mov %dl, %dh
#define noop()

# Writes to EWRAM and IWRAM must check for SMC
#define smc_check_store_aligned(opsuf, addrexp)
#define smc_check_store(opsuf, addrexp)                                      ;\
  cmp##opsuf $0, addrexp                            /* Check SMC mirror */   ;\
  jne smc_write

# Memory write routines

#define write_stubs(fname, wsize, opsuf, regfn, regfn16, addrm, dup8fn)      ;\
                                                                             ;\
  /* eax: address to write to */                                             ;\
  /* edx: value to write */                                                  ;\
                                                                             ;\
defsymbl(execute_##fname##_u##wsize)                                         ;\
  mov %eax, %ecx                                 /* ecx = address */         ;\
  shr $24, %ecx                                  /* ecx = address >> 24 */   ;\
  cmp $15, %ecx                                                              ;\
  ja ext_store_ignore                                                        ;\
  /* ecx = ext_store_u*_jtable[address >> 24] */                             ;\
  jmp *fname##_u##wsize##_tbl(REG_BASE, FULLREG(cx), ADDR_SIZE_BYTES)        ;\
                                                                             ;\
ext_##fname##_iwram##wsize:                                                  ;\
  and $(0x7FFF & addrm), %eax                                /* Addr wrap */ ;\
  mov regfn(d), (IWRAM_OFF+0x8000)(REG_BASE, FULLREG(ax)) /* Actual write */ ;\
  smc_check_##fname(opsuf, IWRAM_OFF(REG_BASE, FULLREG(ax)))                 ;\
  ret                                                                        ;\
                                                                             ;\
ext_##fname##_ewram##wsize:                                                  ;\
  and $(0x3FFFF & addrm), %eax                               /* Addr wrap */ ;\
  mov regfn(d), EWRAM_OFF(REG_BASE, FULLREG(ax))          /* Actual write */ ;\
  smc_check_##fname(opsuf, (EWRAM_OFF+0x40000)(REG_BASE, FULLREG(ax)))       ;\
  ret                                                                        ;\
                                                                             ;\
ext_##fname##_vram##wsize:                                                   ;\
  and $(0x1FFFE & addrm), %eax                               /* Addr wrap */ ;\
  dup8fn()                                   /* Double byte for 8b access */ ;\
  cmp $0x18000, %eax                                 /* Weird 96KB mirror */ ;\
  jb 1f                                                                      ;\
  sub $0x8000, %eax                                   /* Mirror last bank */ ;\
1:                                                                           ;\
  mov regfn16(d), VRAM_OFF(REG_BASE, FULLREG(ax))         /* Actual write */ ;\
  ret                                                                        ;\
                                                                             ;\
ext_##fname##_oam##wsize:                                                    ;\
  and $(0x3FE & addrm), %eax                                 /* Addr wrap */ ;\
  movl $1, OAM_UPDATED(REG_BASE)                       /* flag OAM update */ ;\
  dup8fn()                                   /* Double byte for 8b access */ ;\
  mov regfn16(d), OAM_RAM_OFF(REG_BASE, FULLREG(ax))      /* Actual write */ ;\
  ret                                                                        ;\
                                                                             ;\
ext_##fname##_io##wsize:                                                     ;\
  and $(0x3FF & addrm), %eax                               /* Addr wrap */   ;\
  SETUP_ARGS                                                                 ;\
  CALL_FUNC(write_io_register##wsize)                    /* Call C code */   ;\
  cmp $0, %eax                                /* Check for side-effects */   ;\
  jnz write_epilogue                             /* Act on SMC and IRQs */   ;\
  ret                                                                        ;\

write_stubs(store,         32, l, reg32, reg32, ~3, noop)
write_stubs(store,         16, w, reg16, reg16, ~1, noop)
write_stubs(store,          8, b,  reg8, reg16, ~0, dup8)
write_stubs(store_aligned, 32, l, reg32, reg32, ~3, noop)

# Palette routines are a bit special, due to 16bit bus + decoded palette

ext_store_palette8:
  and $0x3FE, %eax            # wrap around address and align to 16bits
  mov %dl, %dh                # duplicate the byte to be written
  jmp ext_store_palette16b    # perform 16bit palette write

ext_store_palette16:
  and $0x3FF, %eax            # wrap around address
ext_store_palette16b:         # entry point for 8bit write
  mov %dx, PALETTE_RAM_OFF(REG_BASE, FULLREG(ax)) # write out palette value
  mov %edx, %ecx              # cx = dx
  shl $11, %ecx               # cx <<= 11 (red component is in high bits)
  mov %dh, %cl                # bottom bits of cx = top bits of dx
  shr $2, %cl                 # move the blue component to the bottom of cl
  and $0x03E0, %dx            # isolate green component of dx
  shl $1, %dx                 # make green component 6bits
  or %edx, %ecx               # combine green component into ecx
  # write out the freshly converted palette value
  mov %cx, PALETTE_RAM_CNV_OFF(REG_BASE, FULLREG(ax))
  ret                         # done

ext_store_palette32:
  and $0x3FF, %eax            # wrap around address
  call ext_store_palette16b   # write first 16bits
  add $2, %eax                # go to next address
  shr $16, %edx               # go to next 16bits
  jmp ext_store_palette16b    # write next 16bits


# Memory load routines

#define load_stubs(rtype, movop, addrm, albits, slowfn)                      ;\
                                                                             ;\
  /* eax: address to read */                                                 ;\
  /* edx: current PC address */                                              ;\
                                                                             ;\
defsymbl(execute_load_##rtype)                                               ;\
  mov %eax, %ecx                                /* ecx = address */          ;\
  rol $8, %ecx                                  /* ecx = ror(address, 24) */ ;\
  and $((1<<(8+albits))-1), %ecx                /* preserve align+msb */     ;\
  cmp $15, %ecx                                                              ;\
  ja ext_load_slow##rtype                                                    ;\
  jmp *load_##rtype##_tbl(REG_BASE, FULLREG(cx), ADDR_SIZE_BYTES)            ;\
                                                                             ;\
ext_load_iwram##rtype:                                                       ;\
  and $(0x7FFF & addrm), %eax                              /* Addr wrap */   ;\
  movop (IWRAM_OFF+0x8000)(REG_BASE, FULLREG(ax)), %eax     /* Read mem */   ;\
  ret                                                                        ;\
                                                                             ;\
ext_load_ewram##rtype:                                                       ;\
  and $(0x3FFFF & addrm), %eax                             /* Addr wrap */   ;\
  movop EWRAM_OFF(REG_BASE, FULLREG(ax)), %eax              /* Read mem */   ;\
  ret                                                                        ;\
                                                                             ;\
ext_load_vram##rtype:                                                        ;\
  and $(0x1FFFF & addrm), %eax                             /* Addr wrap */   ;\
  cmp $0x18000, %eax                               /* Weird 96KB mirror */   ;\
  jb 1f                                                                      ;\
  sub $0x8000, %eax                                 /* Mirror last bank */   ;\
1:                                                                           ;\
  movop VRAM_OFF(REG_BASE, FULLREG(ax)), %eax               /* Read mem */   ;\
  ret                                                                        ;\
                                                                             ;\
ext_load_oam##rtype:                                                         ;\
  and $(0x3FF & addrm), %eax                               /* Addr wrap */   ;\
  movop OAM_RAM_OFF(REG_BASE, FULLREG(ax)), %eax            /* Read mem */   ;\
  ret                                                                        ;\
                                                                             ;\
ext_load_palette##rtype:                                                     ;\
  and $(0x3FF & addrm), %eax                               /* Addr wrap */   ;\
  movop PALETTE_RAM_OFF(REG_BASE, FULLREG(ax)), %eax        /* Read mem */   ;\
  ret                                                                        ;\
                                                                             ;\
ext_load_io##rtype:                                                          ;\
  and $(0x3FF & addrm), %eax                               /* Addr wrap */   ;\
  movop IORAM_OFF(REG_BASE, FULLREG(ax)), %eax              /* Read mem */   ;\
  ret                                                                        ;\
                                                                             ;\
ext_load_rom##rtype:                                                         ;\
  mov %eax, %esi                                 /* esi = address */         ;\
  shr $15, %esi                                  /* esi = address >> 15 */   ;\
                                                 /* Read rdmap pointer */    ;\
  mov RDMAP_OFF(REG_BASE, FULLREG(si), ADDR_SIZE_BYTES), FULLREG(cx)         ;\
  JUMP_CX_ZERO ext_load_slow##rtype              /* page not loaded, slow */ ;\
  mov %eax, %edx                                 /* edx = address */         ;\
  and $0x7FFF, %edx                              /* edx = address LSB */     ;\
  movop (FULLREG(cx), FULLREG(dx)), %eax         /* Read mem */              ;\
  ret                                                                        ;\
                                                                             ;\
ext_load_slow##rtype:                                                        ;\
  mov %edx, REG_PC(REG_BASE)                        /* Store current PC */   ;\
  SETUP_ARGS                                                                 ;\
  CALL_FUNC(slowfn)                                                          ;\
  ret                                                                        ;\

load_stubs(u32, mov,    ~3, 2, read_memory32)
load_stubs(u16, movzwl, ~1, 1, read_memory16)
load_stubs(s16, movswl, ~1, 1, read_memory16s)
load_stubs( u8, movzbl, ~0, 0, read_memory8)
load_stubs( s8, movsbl, ~0, 0, read_memory8s)


# arg0 (%eax) = new_cpsr
# arg1 (%edx) = store_mask (user mode)
# arg2 (%ecx) = store_mask (system mode)
defsymbl(execute_store_cpsr)
  testl $0x10, CPU_MODE(REG_BASE) # check privileged mode bit
  cmovne %ecx, %edx               # use system mode mask if set

  mov %edx, %esi                  # save store_mask for later

  mov %eax, %ecx                  # ecx = new_cpsr
  and %edx, %ecx                  # ecx = new_cpsr & store_mask
  mov REG_CPSR(REG_BASE), %eax    # eax = cpsr
  not %edx                        # edx = ~store_mask
  and %edx, %eax                  # eax = cpsr & ~store_mask
  or %ecx, %eax                   # eax = new cpsr combined with old
  mov %eax, REG_CPSR(REG_BASE)    # save new cpsr to register

  # Check whether any side effects (IRQ) could have happened
  test $0xff, %esi
  jnz 1f
  extract_flags                   # pull out flag vars from new CPSR
  ret
1:
  CALL_FUNC(execute_store_cpsr_body)   # do the dirty work in this C function
  extract_flags                   # pull out flag vars from new CPSR
  cmp $0, %eax                    # see if return value is 0
  jnz 2f                          # might have changed the PC
  ret                             # return
2:    # PC has changed, due to IRQ triggered
  mov %eax, CARG1_REG             # Returned addr from C function
  CALL_FUNC(block_lookup_address_arm)  # lookup new PC
  add $ADDR_SIZE_BYTES, STACK_REG # get rid of current return address
  jmp *FULLREG(ax)


# On writes that overwrite code, cache is flushed and execution re-started
smc_write:
  CALL_FUNC(flush_translation_cache_ram)
lookup_pc:
  mov REG_PC(REG_BASE), CARG1_REG        # Load PC as argument0
  testl $0x20, REG_CPSR(REG_BASE)
  jz 1f
### Thumb mode
  CALL_FUNC(block_lookup_address_thumb)
  add $ADDR_SIZE_BYTES, STACK_REG        # Can't return, discard addr
  jmp *FULLREG(ax)
1:# ARM mode
  CALL_FUNC(block_lookup_address_arm)
  add $ADDR_SIZE_BYTES, STACK_REG        # Can't return, discard addr
  jmp *FULLREG(ax)

# Called from C, args are platform dependant :/
# arg0 (eax/edi/ecx): cycle counter
# arg1 (edx/rsi/rdx): reg base pointer

defsymbl(execute_arm_translate_internal)
  # Save main context, since we need to return gracefully
  SAVE_REGISTERS                    # Pushes 16 or 32 bytes
  # The stack here is aligned to 16 bytes minus 4 or 8 bytes.

  mov CARG1_REG, REG_CYCLES         # load cycle counter (arg0)
  mov CARG2_REGPTR, REG_BASE        # load base register (arg1)

  extract_flags                     # load flag variables

  # (if the CPU is halted, do not start executing but
  #  loop in the alert loop until it wakes up)
  cmpl $0, CPU_HALT_STATE(REG_BASE)
  je 1f
  call cpu_sleep_loop               # Need to push something to the stack

1:
  call lookup_pc                    # Go fetch and execute PC

return_to_main:
  add $ADDR_SIZE_BYTES, STACK_REG    # remove current return addr
  REST_REGISTERS                     # Restore saved registers
  ret

#define load_table(atype)                                                    ;\
  ADDR_TYPE ext_load_slow##atype          /* 0x00 BIOS                     */;\
  ADDR_TYPE ext_load_slow##atype          /* 0x01 open read                */;\
  ADDR_TYPE ext_load_ewram##atype         /* 0x02 EWRAM                    */;\
  ADDR_TYPE ext_load_iwram##atype         /* 0x03 IWRAM                    */;\
  ADDR_TYPE ext_load_io##atype            /* 0x04 I/O registers            */;\
  ADDR_TYPE ext_load_palette##atype       /* 0x05 Palette RAM              */;\
  ADDR_TYPE ext_load_vram##atype          /* 0x06 VRAM                     */;\
  ADDR_TYPE ext_load_oam##atype           /* 0x07 OAM RAM                  */;\
  ADDR_TYPE ext_load_rom##atype           /* 0x08 gamepak (or RTC)         */;\
  ADDR_TYPE ext_load_rom##atype           /* 0x09 gamepak                  */;\
  ADDR_TYPE ext_load_rom##atype           /* 0x0A gamepak                  */;\
  ADDR_TYPE ext_load_rom##atype           /* 0x0B gamepak                  */;\
  ADDR_TYPE ext_load_rom##atype           /* 0x0C gamepak                  */;\
  ADDR_TYPE ext_load_slow##atype          /* 0x0D EEPROM (possibly)        */;\
  ADDR_TYPE ext_load_slow##atype          /* 0x0E Flash ROM/SRAM           */;\
  ADDR_TYPE ext_load_slow##atype          /* 0x0F open read                */;\

#define store_table(asize)                                                   ;\
  ADDR_TYPE ext_store_ignore              /* 0x00 BIOS, ignore             */;\
  ADDR_TYPE ext_store_ignore              /* 0x01 invalid, ignore          */;\
  ADDR_TYPE ext_store_ewram##asize        /* 0x02 EWRAM                    */;\
  ADDR_TYPE ext_store_iwram##asize        /* 0x03 IWRAM                    */;\
  ADDR_TYPE ext_store_io##asize           /* 0x04 I/O registers            */;\
  ADDR_TYPE ext_store_palette##asize      /* 0x05 Palette RAM              */;\
  ADDR_TYPE ext_store_vram##asize         /* 0x06 VRAM                     */;\
  ADDR_TYPE ext_store_oam##asize          /* 0x07 OAM RAM                  */;\
  ADDR_TYPE ext_store_gpio##asize         /* 0x08 gamepak (RTC or ignore)  */;\
  ADDR_TYPE ext_store_ignore              /* 0x09 gamepak, ignore          */;\
  ADDR_TYPE ext_store_ignore              /* 0x0A gamepak, ignore          */;\
  ADDR_TYPE ext_store_ignore              /* 0x0B gamepak, ignore          */;\
  ADDR_TYPE ext_store_ignore              /* 0x0C gamepak, ignore          */;\
  ADDR_TYPE ext_store_eeprom##asize       /* 0x0D EEPROM (possibly)        */;\
  ADDR_TYPE ext_store_backup##asize       /* 0x0E Flash ROM/SRAM           */;\
  ADDR_TYPE ext_store_ignore              /* 0x0F ignore                   */;\

.data
.align 16

defsymbl(x86_table_data)
  load_table(u8)
  load_table(s8)
  load_table(u16)
  load_table(s16)
  load_table(u32)
  store_table(8)
  store_table(16)
  store_table(32)

  # aligned word writes (non SMC signaling)
  ADDR_TYPE ext_store_ignore              # 0x00 BIOS, ignore
  ADDR_TYPE ext_store_ignore              # 0x01 invalid, ignore
  ADDR_TYPE ext_store_aligned_ewram32     # 0x02 EWRAM
  ADDR_TYPE ext_store_aligned_iwram32     # 0x03 IWRAM
  ADDR_TYPE ext_store_io32                # 0x04 I/O registers
  ADDR_TYPE ext_store_palette32           # 0x05 Palette RAM
  ADDR_TYPE ext_store_vram32              # 0x06 VRAM
  ADDR_TYPE ext_store_oam32               # 0x07 OAM RAM
  ADDR_TYPE ext_store_ignore              # 0x08 gamepak, ignore (no RTC in 32bit)
  ADDR_TYPE ext_store_ignore              # 0x09 gamepak, ignore
  ADDR_TYPE ext_store_ignore              # 0x0A gamepak, ignore
  ADDR_TYPE ext_store_ignore              # 0x0B gamepak, ignore
  ADDR_TYPE ext_store_ignore              # 0x0C gamepak, ignore
  ADDR_TYPE ext_store_eeprom32            # 0x0D EEPROM (possibly)
  ADDR_TYPE ext_store_ignore              # 0x0E Flash ROM/SRAM must be 8bit
  ADDR_TYPE ext_store_ignore              # 0x0F ignore


.bss
.align 64

defsymbl(x86_table_info)
  .space 9*16*ADDR_SIZE_BYTES
defsymbl(reg)
  .space 0x100
defsymbl(palette_ram)
  .space 0x400
defsymbl(palette_ram_converted)
  .space 0x400
defsymbl(oam_ram)
  .space 0x400
defsymbl(iwram)
  .space 0x10000
defsymbl(vram)
  .space 0x18000
defsymbl(ewram)
  .space 0x80000
defsymbl(io_registers)
  .space 0x400
defsymbl(spsr)
  .space 24
  .space 8    # padding
defsymbl(reg_mode)
  .space 196
  .space 28   # padding
defsymbl(memory_map_read)
  .space 8*1024*ADDR_SIZE_BYTES

#ifndef MMAP_JIT_CACHE
  #error "x86 dynarec builds *require* MMAP_JIT_CACHE"
#endif

