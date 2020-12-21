//  h6280.c - CPU Emulation
//
#include <stdio.h>
#include <stdlib.h>

#include "hard_pce.h"
#include "gfx.h"
#include "pce.h"

#include "h6280_instr.h"

#define OPCODE(n, f) case n: f; break;

h6280_t CPU;


/**
 * Reset CPU
 **/
void
h6280_reset(void)
{
    reg_a = reg_x = reg_y = 0x00;
    reg_p = (FL_I|FL_B);
    reg_s = 0xFF;
    reg_pc = pce_read16(VEC_RESET);
}


/**
 * Log assembler instructions starting at current PC
 **/
void
h6280_debug(void)
{
	//
}


/**
 * Branch into interrupt
 **/
void
h6280_irq(int type)
{
	interrupt(type);
}


/**
 * CPU emulation
 **/
void
h6280_run(void)
{
	int max_cycles = PCE.MaxCycles;

	/* Handle active block transfers, ie: do nothing. (tai/tdd/tia/tin/tii) */
	if (Cycles > max_cycles) {
		return;
	}

	/* Handle pending interrupts (Should be in the loop, but it's too slow) */
	uint8_t irq = PCE.irq_status & ~PCE.irq_mask & INT_MASK;
	if ((reg_p & FL_I) == 0 && irq) {
		interrupt(irq);
	}

	/* Run for roughly one scanline */
	while (Cycles < max_cycles)
	{
		UBYTE opcode = imm_operand(reg_pc);

		switch (opcode)
		{
			OPCODE(0x00, brk());           // {brk, AM_IMMED, "BRK"}
			OPCODE(0x01, ora_zpindx());    // {ora_zpindx, AM_ZPINDX, "ORA"}
			OPCODE(0x02, sxy());           // {sxy, AM_IMPL, "SXY"}
			OPCODE(0x03, st0());           // {st0, AM_IMMED, "ST0"}
			OPCODE(0x04, tsb_zp());        // {tsb_zp, AM_ZP, "TSB"}
			OPCODE(0x05, ora_zp());        // {ora_zp, AM_ZP, "ORA"}
			OPCODE(0x06, asl_zp());        // {asl_zp, AM_ZP, "ASL"}
			OPCODE(0x07, rmb(0));          // {rmb0, AM_ZP, "RMB0"}
			OPCODE(0x08, php());           // {php, AM_IMPL, "PHP"}
			OPCODE(0x09, ora_imm());       // {ora_imm, AM_IMMED, "ORA"}
			OPCODE(0x0A, asl_a());         // {asl_a, AM_IMPL, "ASL"}
			OPCODE(0x0C, tsb_abs());       // {tsb_abs, AM_ABS, "TSB"}
			OPCODE(0x0D, ora_abs());       // {ora_abs, AM_ABS, "ORA"}
			OPCODE(0x0E, asl_abs());       // {asl_abs, AM_ABS, "ASL"}
			OPCODE(0x0F, bbr(0));          // {bbr0, AM_PSREL, "BBR0"}

			OPCODE(0x10, bpl());           // {bpl, AM_REL, "BPL"}
			OPCODE(0x11, ora_zpindy());    // {ora_zpindy, AM_ZPINDY, "ORA"}
			OPCODE(0x12, ora_zpind());     // {ora_zpind, AM_ZPIND, "ORA"}
			OPCODE(0x13, st1());           // {st1, AM_IMMED, "ST1"}
			OPCODE(0x14, trb_zp());        // {trb_zp, AM_ZP, "TRB"}
			OPCODE(0x15, ora_zpx());       // {ora_zpx, AM_ZPX, "ORA"}
			OPCODE(0x16, asl_zpx());       // {asl_zpx, AM_ZPX, "ASL"}
			OPCODE(0x17, rmb(1));          // {rmb1, AM_ZP, "RMB1"}
			OPCODE(0x18, clc());           // {clc, AM_IMPL, "CLC"}
			OPCODE(0x19, ora_absy());      // {ora_absy, AM_ABSY, "ORA"}
			OPCODE(0x1A, inc_a());         // {inc_a, AM_IMPL, "INC"}
			OPCODE(0x1C, trb_abs());       // {trb_abs, AM_ABS, "TRB"}
			OPCODE(0x1D, ora_absx());      // {ora_absx, AM_ABSX, "ORA"}
			OPCODE(0x1E, asl_absx());      // {asl_absx, AM_ABSX, "ASL"}
			OPCODE(0x1F, bbr(1));          // {bbr1, AM_PSREL, "BBR1"}

			OPCODE(0x20, jsr());           // {jsr, AM_ABS, "JSR"}
			OPCODE(0x21, and_zpindx());    // {and_zpindx, AM_ZPINDX, "AND"}
			OPCODE(0x22, sax());           // {sax, AM_IMPL, "SAX"}
			OPCODE(0x23, st2());           // {st2, AM_IMMED, "ST2"}
			OPCODE(0x24, bit_zp());        // {bit_zp, AM_ZP, "BIT"}
			OPCODE(0x25, and_zp());        // {and_zp, AM_ZP, "AND"}
			OPCODE(0x26, rol_zp());        // {rol_zp, AM_ZP, "ROL"}
			OPCODE(0x27, rmb(2));          // {rmb2, AM_ZP, "RMB2"}
			OPCODE(0x28, plp());           // {plp, AM_IMPL, "PLP"}
			OPCODE(0x29, and_imm());       // {and_imm, AM_IMMED, "AND"}
			OPCODE(0x2A, rol_a());         // {rol_a, AM_IMPL, "ROL"}
			OPCODE(0x2C, bit_abs());       // {bit_abs, AM_ABS, "BIT"}
			OPCODE(0x2D, and_abs());       // {and_abs, AM_ABS, "AND"}
			OPCODE(0x2E, rol_abs());       // {rol_abs, AM_ABS, "ROL"}
			OPCODE(0x2F, bbr(2));          // {bbr2, AM_PSREL, "BBR2"}

			OPCODE(0x30, bmi());           // {bmi, AM_REL, "BMI"}
			OPCODE(0x31, and_zpindy());    // {and_zpindy, AM_ZPINDY, "AND"}
			OPCODE(0x32, and_zpind());     // {and_zpind, AM_ZPIND, "AND"}
			OPCODE(0x34, bit_zpx());       // {bit_zpx, AM_ZPX, "BIT"}
			OPCODE(0x35, and_zpx());       // {and_zpx, AM_ZPX, "AND"}
			OPCODE(0x36, rol_zpx());       // {rol_zpx, AM_ZPX, "ROL"}
			OPCODE(0x37, rmb(3));          // {rmb3, AM_ZP, "RMB3"}
			OPCODE(0x38, sec());           // {sec, AM_IMPL, "SEC"}
			OPCODE(0x39, and_absy());      // {and_absy, AM_ABSY, "AND"}
			OPCODE(0x3A, dec_a());         // {dec_a, AM_IMPL, "DEC"}
			OPCODE(0x3C, bit_absx());      // {bit_absx, AM_ABSX, "BIT"}
			OPCODE(0x3D, and_absx());      // {and_absx, AM_ABSX, "AND"}
			OPCODE(0x3E, rol_absx());      // {rol_absx, AM_ABSX, "ROL"}
			OPCODE(0x3F, bbr(3));          // {bbr3, AM_PSREL, "BBR3"}

			OPCODE(0x40, rti());           // {rti, AM_IMPL, "RTI"}
			OPCODE(0x41, eor_zpindx());    // {eor_zpindx, AM_ZPINDX, "EOR"}
			OPCODE(0x42, say());           // {say, AM_IMPL, "SAY"}
			OPCODE(0x43, tma());           // {tma, AM_IMMED, "TMA"}
			OPCODE(0x44, bsr());           // {bsr, AM_REL, "BSR"}
			OPCODE(0x45, eor_zp());        // {eor_zp, AM_ZP, "EOR"}
			OPCODE(0x46, lsr_zp());        // {lsr_zp, AM_ZP, "LSR"}
			OPCODE(0x47, rmb(4));          // {rmb4, AM_ZP, "RMB4"}
			OPCODE(0x48, pha());           // {pha, AM_IMPL, "PHA"}
			OPCODE(0x49, eor_imm());       // {eor_imm, AM_IMMED, "EOR"}
			OPCODE(0x4A, lsr_a());         // {lsr_a, AM_IMPL, "LSR"}
			OPCODE(0x4C, jmp());           // {jmp, AM_ABS, "JMP"}
			OPCODE(0x4D, eor_abs());       // {eor_abs, AM_ABS, "EOR"}
			OPCODE(0x4E, lsr_abs());       // {lsr_abs, AM_ABS, "LSR"}
			OPCODE(0x4F, bbr(4));          // {bbr4, AM_PSREL, "BBR4"}

			OPCODE(0x50, bvc());           // {bvc, AM_REL, "BVC"}
			OPCODE(0x51, eor_zpindy());    // {eor_zpindy, AM_ZPINDY, "EOR"}
			OPCODE(0x52, eor_zpind());     // {eor_zpind, AM_ZPIND, "EOR"}
			OPCODE(0x53, tam());           // {tam, AM_IMMED, "TAM"}
			OPCODE(0x54, nop());           // {nop, AM_IMPL, "CSL"}
			OPCODE(0x55, eor_zpx());       // {eor_zpx, AM_ZPX, "EOR"}
			OPCODE(0x56, lsr_zpx());       // {lsr_zpx, AM_ZPX, "LSR"}
			OPCODE(0x57, rmb(5));          // {rmb5, AM_ZP, "RMB5"}
			OPCODE(0x58, cli());           // {cli, AM_IMPL, "CLI"}
			OPCODE(0x59, eor_absy());      // {eor_absy, AM_ABSY, "EOR"}
			OPCODE(0x5A, phy());           // {phy, AM_IMPL, "PHY"}
			OPCODE(0x5D, eor_absx());      // {eor_absx, AM_ABSX, "EOR"}
			OPCODE(0x5E, lsr_absx());      // {lsr_absx, AM_ABSX, "LSR"}
			OPCODE(0x5F, bbr(5));          // {bbr5, AM_PSREL, "BBR5"}

			OPCODE(0x60, rts());           // {rts, AM_IMPL, "RTS"}
			OPCODE(0x61, adc_zpindx());    // {adc_zpindx, AM_ZPINDX, "ADC"}
			OPCODE(0x62, cla());           // {cla, AM_IMPL, "CLA"}
			OPCODE(0x64, stz_zp());        // {stz_zp, AM_ZP, "STZ"}
			OPCODE(0x65, adc_zp());        // {adc_zp, AM_ZP, "ADC"}
			OPCODE(0x66, ror_zp());        // {ror_zp, AM_ZP, "ROR"}
			OPCODE(0x67, rmb(6));          // {rmb6, AM_ZP, "RMB6"}
			OPCODE(0x68, pla());           // {pla, AM_IMPL, "PLA"}
			OPCODE(0x69, adc_imm());       // {adc_imm, AM_IMMED, "ADC"}
			OPCODE(0x6A, ror_a());         // {ror_a, AM_IMPL, "ROR"}
			OPCODE(0x6C, jmp_absind());    // {jmp_absind, AM_ABSIND, "JMP"}
			OPCODE(0x6D, adc_abs());       // {adc_abs, AM_ABS, "ADC"}
			OPCODE(0x6E, ror_abs());       // {ror_abs, AM_ABS, "ROR"}
			OPCODE(0x6F, bbr(6));          // {bbr6, AM_PSREL, "BBR6"}

			OPCODE(0x70, bvs());           // {bvs, AM_REL, "BVS"}
			OPCODE(0x71, adc_zpindy());    // {adc_zpindy, AM_ZPINDY, "ADC"}
			OPCODE(0x72, adc_zpind());     // {adc_zpind, AM_ZPIND, "ADC"}
			OPCODE(0x73, tii());           // {tii, AM_XFER, "TII"}
			OPCODE(0x74, stz_zpx());       // {stz_zpx, AM_ZPX, "STZ"}
			OPCODE(0x75, adc_zpx());       // {adc_zpx, AM_ZPX, "ADC"}
			OPCODE(0x76, ror_zpx());       // {ror_zpx, AM_ZPX, "ROR"}
			OPCODE(0x77, rmb(7));          // {rmb7, AM_ZP, "RMB7"}
			OPCODE(0x78, sei());           // {sei, AM_IMPL, "SEI"}
			OPCODE(0x79, adc_absy());      // {adc_absy, AM_ABSY, "ADC"}
			OPCODE(0x7A, ply());           // {ply, AM_IMPL, "PLY"}
			OPCODE(0x7C, jmp_absindx());   // {jmp_absindx, AM_ABSINDX, "JMP"}
			OPCODE(0x7D, adc_absx());      // {adc_absx, AM_ABSX, "ADC"}
			OPCODE(0x7E, ror_absx());      // {ror_absx, AM_ABSX, "ROR"}
			OPCODE(0x7F, bbr(7));          // {bbr7, AM_PSREL, "BBR7"}

			OPCODE(0x80, bra());           // {bra, AM_REL, "BRA"}
			OPCODE(0x81, sta_zpindx());    // {sta_zpindx, AM_ZPINDX, "STA"}
			OPCODE(0x82, clx());           // {clx, AM_IMPL, "CLX"}
			OPCODE(0x83, tstins_zp());     // {tstins_zp, AM_TST_ZP, "TST"}
			OPCODE(0x84, sty_zp());        // {sty_zp, AM_ZP, "STY"}
			OPCODE(0x85, sta_zp());        // {sta_zp, AM_ZP, "STA"}
			OPCODE(0x86, stx_zp());        // {stx_zp, AM_ZP, "STX"}
			OPCODE(0x87, smb(0));          // {smb0, AM_ZP, "SMB0"}
			OPCODE(0x88, dey());           // {dey, AM_IMPL, "DEY"}
			OPCODE(0x89, bit_imm());       // {bit_imm, AM_IMMED, "BIT"}
			OPCODE(0x8A, txa());           // {txa, AM_IMPL, "TXA"}
			OPCODE(0x8C, sty_abs());       // {sty_abs, AM_ABS, "STY"}
			OPCODE(0x8D, sta_abs());       // {sta_abs, AM_ABS, "STA"}
			OPCODE(0x8E, stx_abs());       // {stx_abs, AM_ABS, "STX"}
			OPCODE(0x8F, bbs(0));          // {bbs0, AM_PSREL, "BBS0"}

			OPCODE(0x90, bcc());           // {bcc, AM_REL, "BCC"}
			OPCODE(0x91, sta_zpindy());    // {sta_zpindy, AM_ZPINDY, "STA"}
			OPCODE(0x92, sta_zpind());     // {sta_zpind, AM_ZPIND, "STA"}
			OPCODE(0x93, tstins_abs());    // {tstins_abs, AM_TST_ABS, "TST"}
			OPCODE(0x94, sty_zpx());       // {sty_zpx, AM_ZPX, "STY"}
			OPCODE(0x95, sta_zpx());       // {sta_zpx, AM_ZPX, "STA"}
			OPCODE(0x96, stx_zpy());       // {stx_zpy, AM_ZPY, "STX"}
			OPCODE(0x97, smb(1));          // {smb1, AM_ZP, "SMB1"}
			OPCODE(0x98, tya());           // {tya, AM_IMPL, "TYA"}
			OPCODE(0x99, sta_absy());      // {sta_absy, AM_ABSY, "STA"}
			OPCODE(0x9A, txs());           // {txs, AM_IMPL, "TXS"}
			OPCODE(0x9C, stz_abs());       // {stz_abs, AM_ABS, "STZ"}
			OPCODE(0x9D, sta_absx());      // {sta_absx, AM_ABSX, "STA"}
			OPCODE(0x9E, stz_absx());      // {stz_absx, AM_ABSX, "STZ"}
			OPCODE(0x9F, bbs(1));          // {bbs1, AM_PSREL, "BBS1"}

			OPCODE(0xA0, ldy_imm());       // {ldy_imm, AM_IMMED, "LDY"}
			OPCODE(0xA1, lda_zpindx());    // {lda_zpindx, AM_ZPINDX, "LDA"}
			OPCODE(0xA2, ldx_imm());       // {ldx_imm, AM_IMMED, "LDX"}
			OPCODE(0xA3, tstins_zpx());    // {tstins_zpx, AM_TST_ZPX, "TST"}
			OPCODE(0xA4, ldy_zp());        // {ldy_zp, AM_ZP, "LDY"}
			OPCODE(0xA5, lda_zp());        // {lda_zp, AM_ZP, "LDA"}
			OPCODE(0xA6, ldx_zp());        // {ldx_zp, AM_ZP, "LDX"}
			OPCODE(0xA7, smb(2));          // {smb2, AM_ZP, "SMB2"}
			OPCODE(0xA8, tay());           // {tay, AM_IMPL, "TAY"}
			OPCODE(0xA9, lda_imm());       // {lda_imm, AM_IMMED, "LDA"}
			OPCODE(0xAA, tax());           // {tax, AM_IMPL, "TAX"}
			OPCODE(0xAC, ldy_abs());       // {ldy_abs, AM_ABS, "LDY"}
			OPCODE(0xAD, lda_abs());       // {lda_abs, AM_ABS, "LDA"}
			OPCODE(0xAE, ldx_abs());       // {ldx_abs, AM_ABS, "LDX"}
			OPCODE(0xAF, bbs(2));          // {bbs2, AM_PSREL, "BBS2"}

			OPCODE(0xB0, bcs());           // {bcs, AM_REL, "BCS"}
			OPCODE(0xB1, lda_zpindy());    // {lda_zpindy, AM_ZPINDY, "LDA"}
			OPCODE(0xB2, lda_zpind());     // {lda_zpind, AM_ZPIND, "LDA"}
			OPCODE(0xB3, tstins_absx());   // {tstins_absx, AM_TST_ABSX, "TST"}
			OPCODE(0xB4, ldy_zpx());       // {ldy_zpx, AM_ZPX, "LDY"}
			OPCODE(0xB5, lda_zpx());       // {lda_zpx, AM_ZPX, "LDA"}
			OPCODE(0xB6, ldx_zpy());       // {ldx_zpy, AM_ZPY, "LDX"}
			OPCODE(0xB7, smb(3));          // {smb3, AM_ZP, "SMB3"}
			OPCODE(0xB8, clv());           // {clv, AM_IMPL, "CLV"}
			OPCODE(0xB9, lda_absy());      // {lda_absy, AM_ABSY, "LDA"}
			OPCODE(0xBA, tsx());           // {tsx, AM_IMPL, "TSX"}
			OPCODE(0xBC, ldy_absx());      // {ldy_absx, AM_ABSX, "LDY"}
			OPCODE(0xBD, lda_absx());      // {lda_absx, AM_ABSX, "LDA"}
			OPCODE(0xBE, ldx_absy());      // {ldx_absy, AM_ABSY, "LDX"}
			OPCODE(0xBF, bbs(3));          // {bbs3, AM_PSREL, "BBS3"}

			OPCODE(0xC0, cpy_imm());       // {cpy_imm, AM_IMMED, "CPY"}
			OPCODE(0xC1, cmp_zpindx());    // {cmp_zpindx, AM_ZPINDX, "CMP"}
			OPCODE(0xC2, cly());           // {cly, AM_IMPL, "CLY"}
			OPCODE(0xC3, tdd());           // {tdd, AM_XFER, "TDD"}
			OPCODE(0xC4, cpy_zp());        // {cpy_zp, AM_ZP, "CPY"}
			OPCODE(0xC5, cmp_zp());        // {cmp_zp, AM_ZP, "CMP"}
			OPCODE(0xC6, dec_zp());        // {dec_zp, AM_ZP, "DEC"}
			OPCODE(0xC7, smb(4));          // {smb4, AM_ZP, "SMB4"}
			OPCODE(0xC8, iny());           // {iny, AM_IMPL, "INY"}
			OPCODE(0xC9, cmp_imm());       // {cmp_imm, AM_IMMED, "CMP"}
			OPCODE(0xCA, dex());           // {dex, AM_IMPL, "DEX"}
			OPCODE(0xCC, cpy_abs());       // {cpy_abs, AM_ABS, "CPY"}
			OPCODE(0xCD, cmp_abs());       // {cmp_abs, AM_ABS, "CMP"}
			OPCODE(0xCE, dec_abs());       // {dec_abs, AM_ABS, "DEC"}
			OPCODE(0xCF, bbs(4));          // {bbs4, AM_PSREL, "BBS4"}

			OPCODE(0xD0, bne());           // {bne, AM_REL, "BNE"}
			OPCODE(0xD1, cmp_zpindy());    // {cmp_zpindy, AM_ZPINDY, "CMP"}
			OPCODE(0xD2, cmp_zpind());     // {cmp_zpind, AM_ZPIND, "CMP"}
			OPCODE(0xD3, tin());           // {tin, AM_XFER, "TIN"}
			OPCODE(0xD4, nop());           // {nop, AM_IMPL, "CSH"}
			OPCODE(0xD5, cmp_zpx());       // {cmp_zpx, AM_ZPX, "CMP"}
			OPCODE(0xD6, dec_zpx());       // {dec_zpx, AM_ZPX, "DEC"}
			OPCODE(0xD7, smb(5));          // {smb5, AM_ZP, "SMB5"}
			OPCODE(0xD8, cld());           // {cld, AM_IMPL, "CLD"}
			OPCODE(0xD9, cmp_absy());      // {cmp_absy, AM_ABSY, "CMP"}
			OPCODE(0xDA, phx());           // {phx, AM_IMPL, "PHX"}
			OPCODE(0xDD, cmp_absx());      // {cmp_absx, AM_ABSX, "CMP"}
			OPCODE(0xDE, dec_absx());      // {dec_absx, AM_ABSX, "DEC"}
			OPCODE(0xDF, bbs(5));          // {bbs5, AM_PSREL, "BBS5"}

			OPCODE(0xE0, cpx_imm());       // {cpx_imm, AM_IMMED, "CPX"}
			OPCODE(0xE1, sbc_zpindx());    // {sbc_zpindx, AM_ZPINDX, "SBC"}
			OPCODE(0xE3, tia());           // {tia, AM_XFER, "TIA"} $E3
			OPCODE(0xE4, cpx_zp());        // {cpx_zp, AM_ZP, "CPX"}
			OPCODE(0xE5, sbc_zp());        // {sbc_zp, AM_ZP, "SBC"}
			OPCODE(0xE6, inc_zp());        // {inc_zp, AM_ZP, "INC"}
			OPCODE(0xE7, smb(6));          // {smb6, AM_ZP, "SMB6"}
			OPCODE(0xE8, inx());           // {inx, AM_IMPL, "INX"}
			OPCODE(0xE9, sbc_imm());       // {sbc_imm, AM_IMMED, "SBC"}
			OPCODE(0xEA, nop());           // {nop, AM_IMPL, "NOP"}
			OPCODE(0xEC, cpx_abs());       // {cpx_abs, AM_ABS, "CPX"}
			OPCODE(0xED, sbc_abs());       // {sbc_abs, AM_ABS, "SBC"}
			OPCODE(0xEE, inc_abs());       // {inc_abs, AM_ABS, "INC"}
			OPCODE(0xEF, bbs(6));          // {bbs6, AM_PSREL, "BBS6"}

			OPCODE(0xF0, beq());           // {beq, AM_REL, "BEQ"}
			OPCODE(0xF1, sbc_zpindy());    // {sbc_zpindy, AM_ZPINDY, "SBC"}
			OPCODE(0xF2, sbc_zpind());     // {sbc_zpind, AM_ZPIND, "SBC"}
			OPCODE(0xF3, tai());           // {tai, AM_XFER, "TAI"}
			OPCODE(0xF4, set());           // {set, AM_IMPL, "SET"}
			OPCODE(0xF5, sbc_zpx());       // {sbc_zpx, AM_ZPX, "SBC"}
			OPCODE(0xF6, inc_zpx());       // {inc_zpx, AM_ZPX, "INC"}
			OPCODE(0xF7, smb(7));          // {smb7, AM_ZP, "SMB7"}
			OPCODE(0xF8, sed());           // {sed, AM_IMPL, "SED"}
			OPCODE(0xF9, sbc_absy());      // {sbc_absy, AM_ABSY, "SBC"}
			OPCODE(0xFA, plx());           // {plx, AM_IMPL, "PLX"}
			OPCODE(0xFD, sbc_absx());      // {sbc_absx, AM_ABSX, "SBC"}
			OPCODE(0xFE, inc_absx());      // {inc_absx, AM_ABSX, "INC"}
			OPCODE(0xFF, bbs(7));          // {bbs7, AM_PSREL, "BBS7"}

			default:
				// Illegal opcodes are treated as NOP
				MESSAGE_DEBUG("Illegal opcode 0x%02X at pc=0x%04X!\n", opcode, reg_pc);
				nop();
		}
	}
}


void
h6280_print_state()
{
	MESSAGE_INFO("Current h6280 status:\n");

	MESSAGE_INFO("PC = 0x%04x\n", reg_pc);
	MESSAGE_INFO("A = 0x%02x\n", reg_a);
	MESSAGE_INFO("X = 0x%02x\n", reg_x);
	MESSAGE_INFO("Y = 0x%02x\n", reg_y);
	MESSAGE_INFO("P = 0x%02x\n", reg_p);
	MESSAGE_INFO("S = 0x%02x\n", reg_s);

	for (int i = 0; i < 8; i++) {
		MESSAGE_INFO("MMR[%d] = 0x%02x\n", i, PCE.MMR[i]);
	}

	// TODO: Add zero page dump

	for (int i = 0x2000; i < 0xFFFF; i++) {

		if ((i & 0xF) == 0) {
			MESSAGE_INFO("%04X: ", i);
		}

		MESSAGE_INFO("%02x ", pce_read8(i));
		if ((i & 0xF) == 0xF) {
			MESSAGE_INFO("\n");
		}
		if ((i & 0x1FFF) == 0x1FFF) {
			MESSAGE_INFO("\n-------------------------------------------------------------\n");
		}
	}
}
