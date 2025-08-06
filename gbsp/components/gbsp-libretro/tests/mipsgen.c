
#define u32 uint32_t
#define u8  uint8_t

#include <stdio.h>
#include <stdint.h>
#include "mips_codegen.h"

int main() {
  u32 buffer[1024];
  u8 *translation_ptr = (u8*)&buffer[0];

  mips_emit_nop();
  mips_emit_nop();

  mips_emit_addu(mips_reg_a0, mips_reg_a1, mips_reg_a2);
  mips_emit_addu(mips_reg_sp, mips_reg_ra, mips_reg_s4);
  mips_emit_subu(mips_reg_a0, mips_reg_a1, mips_reg_a2);
  mips_emit_subu(mips_reg_sp, mips_reg_ra, mips_reg_s4);
  mips_emit_xor(mips_reg_a0, mips_reg_a1, mips_reg_a2);
  mips_emit_xor(mips_reg_sp, mips_reg_ra, mips_reg_s4);
  mips_emit_and(mips_reg_a0, mips_reg_a1, mips_reg_a2);
  mips_emit_and(mips_reg_sp, mips_reg_ra, mips_reg_s4);
  mips_emit_or(mips_reg_a0, mips_reg_a1, mips_reg_a2);
  mips_emit_or(mips_reg_sp, mips_reg_ra, mips_reg_s4);
  mips_emit_nor(mips_reg_a0, mips_reg_a1, mips_reg_a2);
  mips_emit_nor(mips_reg_sp, mips_reg_ra, mips_reg_s4);

  mips_emit_slt(mips_reg_a0, mips_reg_a1, mips_reg_a2);
  mips_emit_slt(mips_reg_sp, mips_reg_ra, mips_reg_s4);
  mips_emit_sltu(mips_reg_a0, mips_reg_a1, mips_reg_a2);
  mips_emit_sltu(mips_reg_sp, mips_reg_ra, mips_reg_s4);

  mips_emit_sllv(mips_reg_a0, mips_reg_a1, mips_reg_a2);
  mips_emit_sllv(mips_reg_sp, mips_reg_ra, mips_reg_s4);
  mips_emit_srlv(mips_reg_a0, mips_reg_a1, mips_reg_a2);
  mips_emit_srlv(mips_reg_sp, mips_reg_ra, mips_reg_s4);
  mips_emit_srav(mips_reg_a0, mips_reg_a1, mips_reg_a2);
  mips_emit_srav(mips_reg_sp, mips_reg_ra, mips_reg_s4);
  mips_emit_rotrv(mips_reg_a0, mips_reg_a1, mips_reg_a2);
  mips_emit_rotrv(mips_reg_sp, mips_reg_ra, mips_reg_s4);

  for (unsigned i = 0; i < 4; i++) {
    mips_emit_sll(mips_reg_a0, mips_reg_a1, (i & 1) + (i >> 1) * 30);
    mips_emit_srl(mips_reg_a0, mips_reg_a1, (i & 1) + (i >> 1) * 30);
    mips_emit_sra(mips_reg_a0, mips_reg_a1, (i & 1) + (i >> 1) * 30);
    mips_emit_rotr(mips_reg_a0, mips_reg_a1, (i & 1) + (i >> 1) * 30);
  }

  mips_emit_lui(mips_reg_a0, 0xFFFF);
  mips_emit_lui(mips_reg_a0, 0x8000);
  mips_emit_lui(mips_reg_a0, 0);
  mips_emit_lui(mips_reg_a0, 1);

  const int imm[] = {-1, 0, 1, 0x8000, 0x7FFF};
  for (unsigned i = 0; i < 5; i++) {
    mips_emit_addiu(mips_reg_a0, mips_reg_s6, imm[i]);
    mips_emit_xori(mips_reg_a0, mips_reg_s6, imm[i]);
    mips_emit_ori(mips_reg_a0, mips_reg_s6, imm[i]);
    mips_emit_andi(mips_reg_a0, mips_reg_s6, imm[i]);
    mips_emit_slti(mips_reg_a0, mips_reg_s6, imm[i]);
    mips_emit_sltiu(mips_reg_a0, mips_reg_s6, imm[i]);
  }

  mips_emit_mflo(mips_reg_a3);
  mips_emit_mflo(mips_reg_fp);
  mips_emit_mfhi(mips_reg_a3);
  mips_emit_mfhi(mips_reg_fp);
  mips_emit_mtlo(mips_reg_a3);
  mips_emit_mtlo(mips_reg_fp);
  mips_emit_mthi(mips_reg_a3);
  mips_emit_mthi(mips_reg_fp);

  mips_emit_mult(mips_reg_a2, mips_reg_a3);
  mips_emit_mult(mips_reg_s2, mips_reg_s4);
  mips_emit_multu(mips_reg_a2, mips_reg_a3);
  mips_emit_multu(mips_reg_s2, mips_reg_s4);
  mips_emit_div(mips_reg_a2, mips_reg_a3);
  mips_emit_div(mips_reg_s2, mips_reg_s4);
  mips_emit_divu(mips_reg_a2, mips_reg_a3);
  mips_emit_divu(mips_reg_s2, mips_reg_s4);

  mips_emit_jr(mips_reg_a1);
  mips_emit_jr(mips_reg_ra);
  mips_emit_jalr(mips_reg_a1);
  mips_emit_jalr(mips_reg_s4);

  mips_emit_bltzal(mips_reg_a0, 5);
  mips_emit_bltzal(mips_reg_s4, 4);
  mips_emit_bgezal(mips_reg_a0, 3);
  mips_emit_bgezal(mips_reg_s4, 2);
  mips_emit_bltz(mips_reg_a0, 1);
  mips_emit_bltz(mips_reg_s4, 0);

  const int off[] = {0, 1, -1, 0x7FFF, -0x8000};
  for (unsigned i = 0; i < 5; i++) {
    mips_emit_lb(mips_reg_a0, mips_reg_a1, off[i]);
    mips_emit_lbu(mips_reg_a0, mips_reg_a1, off[i]);
    mips_emit_lh(mips_reg_a0, mips_reg_a1, off[i]);
    mips_emit_lhu(mips_reg_a0, mips_reg_a1, off[i]);
    mips_emit_lw(mips_reg_a0, mips_reg_a1, off[i]);
  }
  for (unsigned i = 0; i < 5; i++) {
    mips_emit_sb(mips_reg_a0, mips_reg_a1, off[i]);
    mips_emit_sh(mips_reg_a0, mips_reg_a1, off[i]);
    mips_emit_sw(mips_reg_a0, mips_reg_a1, off[i]);
  }

  // MIPS32r2/PSP instructions
  mips_emit_ext(mips_reg_v0, mips_reg_a1, 20, 4);
  mips_emit_ext(mips_reg_t7, mips_reg_s4, 3, 9);
  mips_emit_ins(mips_reg_v0, mips_reg_a1, 20, 4);
  mips_emit_ins(mips_reg_t7, mips_reg_s4, 3, 9);

  mips_emit_seb(mips_reg_a3, mips_reg_t1);
  mips_emit_seh(mips_reg_a3, mips_reg_t1);

  fwrite(buffer, 1, translation_ptr-(u8*)buffer, stdout);
}


