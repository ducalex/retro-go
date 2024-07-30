
#define u32 uint32_t
#define u8  uint8_t

#include <stdio.h>
#include <stdint.h>
#include "arm64_codegen.h"

int main() {
  u32 buffer[1024];
  u8 *translation_ptr = (u8*)&buffer[0];

  aa64_emit_branch(16);
  aa64_emit_brlink(16);

  aa64_emit_brcond(ccode_eq, 16);
  aa64_emit_brcond(ccode_ne, 16);
  aa64_emit_brcond(ccode_hs, 16);
  aa64_emit_brcond(ccode_lo, 16);
  aa64_emit_brcond(ccode_mi, 16);
  aa64_emit_brcond(ccode_pl, 16);
  aa64_emit_brcond(ccode_vs, 16);
  aa64_emit_brcond(ccode_vc, 16);
  aa64_emit_brcond(ccode_hi, 16);
  aa64_emit_brcond(ccode_ls, 16);
  aa64_emit_brcond(ccode_ge, 16);
  aa64_emit_brcond(ccode_lt, 16);
  aa64_emit_brcond(ccode_gt, 16);
  aa64_emit_brcond(ccode_le, 16);
  aa64_emit_brcond(ccode_al, 16);
  aa64_emit_brcond(ccode_nv, 16);

  aa64_emit_ldr(1, 2, 16);
  aa64_emit_ldr(29, 30, 16);
  aa64_emit_str(1, 2, 16);
  aa64_emit_str(29, 30, 16);

  aa64_emit_movlo(0,  0x1234);
  aa64_emit_movlo(12, 0x5656);
  aa64_emit_movlo(12, ~0);

  aa64_emit_movhi(13, 0x9876);
  aa64_emit_movhi(13, ~0);

  aa64_emit_movhiz(13, 0xabcd);

  aa64_emit_movne(14, 0xAAAA);

  aa64_emit_add_lsl(11, 12, 13, 0);
  aa64_emit_add_lsl(11, 12, 13, 19);
  aa64_emit_add_lsl(11, 12, 13, 31);
  
  aa64_emit_addi(1, 29, 0x123);
  aa64_emit_addi(1, 29, 0xFFF);
  aa64_emit_subi(1, 29, 0x123);
  aa64_emit_subi(1, 29, 0xFFF);

  aa64_emit_addi12(3, 30, 0x123);
  aa64_emit_addi12(3, 30, 0xFFF);
  aa64_emit_subi12(3, 30, 0x123);
  aa64_emit_subi12(3, 30, 0xFFF);

  aa64_emit_addis(29, 30, 0x123);
  aa64_emit_addis(29, 30, 0xFFF);
  aa64_emit_subis(29, 30, 0x123);
  aa64_emit_subis(29, 30, 0xFFF);

  aa64_emit_madd(2, 5, 3, 4);
  aa64_emit_madd(25, 28, 26, 27);
  aa64_emit_msub(2, 5, 3, 4);
  aa64_emit_msub(25, 28, 26, 27);

  aa64_emit_smaddl(2, 5, 3, 4);
  aa64_emit_smaddl(25, 28, 26, 27);
  aa64_emit_umaddl(2, 5, 3, 4);
  aa64_emit_umaddl(25, 28, 26, 27);

  aa64_emit_mul(1, 2, 3);
  aa64_emit_mul(27, 28, 29);

  aa64_emit_ror(1, 2, 1);
  aa64_emit_ror(1, 2, 31);
  aa64_emit_ror(30, 29, 1);
  aa64_emit_ror(30, 29, 31);

  aa64_emit_lsr(1, 2, 1);
  aa64_emit_lsr(1, 2, 31);
  aa64_emit_lsr(30, 29, 1);
  aa64_emit_lsr(30, 29, 31);

  aa64_emit_lsl(1, 2, 1);
  aa64_emit_lsl(1, 2, 31);
  aa64_emit_lsl(30, 29, 1);
  aa64_emit_lsl(30, 29, 31);

  aa64_emit_asr(1, 2, 1);
  aa64_emit_asr(1, 2, 31);
  aa64_emit_asr(30, 29, 1);
  aa64_emit_asr(30, 29, 31);

  aa64_emit_lsr64(1, 2, 1);
  aa64_emit_lsr64(1, 2, 2);
  aa64_emit_lsr64(1, 2, 62);
  aa64_emit_lsr64(1, 2, 63);
  aa64_emit_lsr64(30, 29, 1);
  aa64_emit_lsr64(30, 29, 62);

  aa64_emit_eori(3, 4, 0, 0);
  aa64_emit_eori(3, 4, 31, 30);  /* ~1 */
  aa64_emit_orri(3, 4, 0, 0);
  aa64_emit_orri(3, 4, 31, 30);
  aa64_emit_andi(3, 4, 0, 0);
  aa64_emit_andi(3, 4, 30, 29);  /* ~3 */
  
  aa64_emit_andi64(3, 4, 0, 31);
  aa64_emit_andi64(3, 4, 0, 0);
  aa64_emit_andi64(1, 2, 0, 0);   /* & 1 */
  aa64_emit_andi64(1, 2, 63, 62);   /* & ~1 */
  aa64_emit_andi64(1, 2, 0, 31);   /* & 0xffffffff */

  aa64_emit_mov(1, 2);
  aa64_emit_mov(30, 31);

  aa64_emit_orr(1, 2, 3);
  aa64_emit_orr(29, 30, 31);
  aa64_emit_xor(1, 2, 3);
  aa64_emit_xor(29, 30, 31);
  aa64_emit_orn(1, 2, 3);
  aa64_emit_orn(29, 30, 31);
  aa64_emit_and(1, 2, 3);
  aa64_emit_and(29, 30, 31);
  aa64_emit_bic(1, 2, 3);
  aa64_emit_bic(29, 30, 31);
  aa64_emit_ands(1, 2, 3);
  aa64_emit_ands(29, 30, 31);

  aa64_emit_tst(1, 2);
  aa64_emit_tst(25, 31);

  aa64_emit_cmpi(1, 0);
  aa64_emit_cmpi(30, 0);
  aa64_emit_cmpi(1, 32);
  aa64_emit_cmpi(30, 32);
  aa64_emit_cmpi(1, 200);
  aa64_emit_cmpi(30, 200);

  aa64_emit_add(1, 2, 3);
  aa64_emit_add(29, 30, 28);
  aa64_emit_sub(1, 2, 3);
  aa64_emit_sub(29, 30, 28);
  aa64_emit_adc(1, 2, 3);
  aa64_emit_adc(29, 30, 28);
  aa64_emit_sbc(1, 2, 3);
  aa64_emit_sbc(29, 30, 28);
  aa64_emit_adds(1, 2, 3);
  aa64_emit_adds(29, 30, 28);
  aa64_emit_subs(1, 2, 3);
  aa64_emit_subs(29, 30, 28);
  aa64_emit_adcs(1, 2, 3);
  aa64_emit_adcs(29, 30, 28);
  aa64_emit_sbcs(1, 2, 3);
  aa64_emit_sbcs(29, 30, 28);

  aa64_emit_tbz(20, 1, 63);
  aa64_emit_tbnz(20, 1, 63);
  aa64_emit_tbz(20, 0, 2);
  aa64_emit_tbnz(20, 7, 2);

  aa64_emit_cbz(20, 63);
  aa64_emit_cbnz(20, 63);
  aa64_emit_cbz(20, 2);
  aa64_emit_cbnz(20, 2);

  aa64_emit_csel(20, 24, 25, ccode_ne);
  aa64_emit_csel(1, 2, 3, ccode_eq);
  aa64_emit_csel(1, 20, 31, ccode_lt);
  aa64_emit_csel(1, 31, 31, ccode_gt);

  aa64_emit_csinc(20, 24, 25, ccode_ne);
  aa64_emit_csinc(1, 2, 3, ccode_eq);
  aa64_emit_csinc(1, 20, 31, ccode_lt);
  aa64_emit_csinc(1, 31, 31, ccode_gt);

  aa64_emit_csinv(20, 24, 25, ccode_ne);
  aa64_emit_csinv(1, 2, 3, ccode_eq);
  aa64_emit_csinv(1, 20, 31, ccode_lt);
  aa64_emit_csinv(1, 31, 31, ccode_gt);

  aa64_emit_csneg(20, 24, 25, ccode_ne);
  aa64_emit_csneg(1, 2, 3, ccode_eq);
  aa64_emit_csneg(1, 20, 31, ccode_lt);
  aa64_emit_csneg(1, 31, 31, ccode_gt);

  aa64_emit_cset(1, ccode_eq);
  aa64_emit_cset(1, ccode_hs);
  aa64_emit_cset(20, ccode_lo);
  aa64_emit_csetm(1, ccode_hs);
  aa64_emit_csetm(20, ccode_lo);

  aa64_emit_ubfx(1, 2, 8, 8);
  aa64_emit_ubfx(1, 2, 16, 16);
  aa64_emit_ubfx(1, 31, 8, 24);
  aa64_emit_ubfx(1, 31, 16, 16);

  aa64_emit_rorv(1, 2, 3);
  aa64_emit_rorv(28, 29, 30);
  aa64_emit_lslv(1, 2, 3);
  aa64_emit_lslv(28, 29, 30);
  aa64_emit_lsrv(1, 2, 3);
  aa64_emit_lsrv(28, 29, 30);
  aa64_emit_asrv(1, 2, 3);
  aa64_emit_asrv(28, 29, 30);

  aa64_emit_merge_regs(1, 3, 2);   /* hi, lo */
  aa64_emit_merge_regs(25, 27, 26);

  aa64_emit_sdiv(1, 2, 3);
  aa64_emit_sdiv(28, 29, 30);

  fwrite(buffer, 1, translation_ptr-(u8*)buffer, stdout);
}


