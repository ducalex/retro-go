//  h6280.c - CPU Emulation
//
#include <stdio.h>
#include <stdlib.h>

#include "pce-go.h"
#include "pce.h"

#define OPCODE(n, f) case n: f; break;
#define Cycles PCE.Cycles

h6280_t CPU;

#include "h6280_instr.h"
#include "h6280_dbg.h"


/**
 * Reset CPU
 **/
void
h6280_reset(void)
{
	CPU.A = CPU.X = CPU.Y = 0x00;
	CPU.P = (FL_I|FL_B);
	CPU.S = 0xFF;
	CPU.PC = pce_read16(VEC_RESET);
	CPU.irq_mask = CPU.irq_mask_delay = CPU.irq_lines = 0;
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
h6280_run(int max_cycles)
{
	/* Handle active block transfers, ie: do nothing. (tai/tdd/tia/tin/tii) */
	if (Cycles >= max_cycles) {
		return;
	}

	/* Handle pending interrupts (Should be in the loop, but it's too slow) */
	unsigned irq = CPU.irq_lines & ~CPU.irq_mask & INT_MASK;
	if ((CPU.P & FL_I) == 0 && irq) {
		interrupt(irq);
	}

	/* Run for roughly one scanline */
	while (Cycles < max_cycles)
	{
		UBYTE opcode = imm_operand(CPU.PC);

		TRACE_CPU("0x%4X: %s\n", CPU.PC, opcodes[opcode].name);

		switch (opcode)
		{
			OPCODE(0x00, brk());				// BRK
			OPCODE(0x01, ora_zpindx());			// ORA (IND,X)
			OPCODE(0x02, sxy());				// SXY
			OPCODE(0x03, st0());				// ST0 #$nn
			OPCODE(0x04, tsb_zp());				// TSB $ZZ
			OPCODE(0x05, ora_zp());				// ORA $ZZ
			OPCODE(0x06, asl_zp());				// ASL $ZZ
			OPCODE(0x07, rmb(0));				// RMB0 $ZZ
			OPCODE(0x08, php());				// PHP
			OPCODE(0x09, ora_imm());			// ORA #$nn
			OPCODE(0x0A, asl_a());				// ASL A
			OPCODE(0x0C, tsb_abs());			// TSB $hhll
			OPCODE(0x0D, ora_abs());			// ORA $hhll
			OPCODE(0x0E, asl_abs());			// ASL $hhll
			OPCODE(0x0F, bbr(0));				// BBR0 $ZZ,$rr

			OPCODE(0x10, bpl());				// BPL REL
			OPCODE(0x11, ora_zpindy());			// ORA (IND),Y
			OPCODE(0x12, ora_zpind());			// ORA (IND)
			OPCODE(0x13, st1());				// ST1 #$nn
			OPCODE(0x14, trb_zp());				// TRB $ZZ
			OPCODE(0x15, ora_zpx());			// ORA $ZZ,X
			OPCODE(0x16, asl_zpx());			// ASL $ZZ,X
			OPCODE(0x17, rmb(1));				// RMB1 $ZZ
			OPCODE(0x18, clc());				// CLC
			OPCODE(0x19, ora_absy());			// ORA $hhll,Y
			OPCODE(0x1A, inc_a());				// INC A
			OPCODE(0x1C, trb_abs());			// TRB $hhll
			OPCODE(0x1D, ora_absx());			// ORA $hhll,X
			OPCODE(0x1E, asl_absx());			// ASL $hhll,X
			OPCODE(0x1F, bbr(1));				// BBR1 $ZZ,$rr

			OPCODE(0x20, jsr());				// JSR $hhll
			OPCODE(0x21, and_zpindx());			// AND (IND,X)
			OPCODE(0x22, sax());				// SAX
			OPCODE(0x23, st2());				// ST2 #$nn
			OPCODE(0x24, bit_zp());				// BIT $ZZ
			OPCODE(0x25, and_zp());				// AND $ZZ
			OPCODE(0x26, rol_zp());				// ROL $ZZ
			OPCODE(0x27, rmb(2));				// RMB2 $ZZ
			OPCODE(0x28, plp());				// PLP
			OPCODE(0x29, and_imm());			// AND #$nn
			OPCODE(0x2A, rol_a());				// ROL A
			OPCODE(0x2C, bit_abs());			// BIT $hhll
			OPCODE(0x2D, and_abs());			// AND $hhll
			OPCODE(0x2E, rol_abs());			// ROL $hhll
			OPCODE(0x2F, bbr(2));				// BBR2 $ZZ,$rr

			OPCODE(0x30, bmi());				// BMI $rr
			OPCODE(0x31, and_zpindy());			// AND (IND),Y
			OPCODE(0x32, and_zpind());			// AND (IND)
			OPCODE(0x34, bit_zpx());			// BIT $ZZ,X
			OPCODE(0x35, and_zpx());			// AND $ZZ,X
			OPCODE(0x36, rol_zpx());			// ROL $ZZ,X
			OPCODE(0x37, rmb(3));				// RMB3 $ZZ
			OPCODE(0x38, sec());				// SEC
			OPCODE(0x39, and_absy());			// AND $hhll,Y
			OPCODE(0x3A, dec_a());				// DEC A
			OPCODE(0x3C, bit_absx());			// BIT $hhll,X
			OPCODE(0x3D, and_absx());			// AND $hhll,X
			OPCODE(0x3E, rol_absx());			// ROL $hhll,X
			OPCODE(0x3F, bbr(3));				// BBR3 $ZZ,$rr

			OPCODE(0x40, rti());				// RTI
			OPCODE(0x41, eor_zpindx());			// EOR (IND,X)
			OPCODE(0x42, say());				// SAY
			OPCODE(0x43, tma());				// TMAi
			OPCODE(0x44, bsr());				// BSR $rr
			OPCODE(0x45, eor_zp());				// EOR $ZZ
			OPCODE(0x46, lsr_zp());				// LSR $ZZ
			OPCODE(0x47, rmb(4));				// RMB4 $ZZ
			OPCODE(0x48, pha());				// PHA
			OPCODE(0x49, eor_imm());			// EOR #$nn
			OPCODE(0x4A, lsr_a());				// LSR A
			OPCODE(0x4C, jmp());				// JMP $hhll
			OPCODE(0x4D, eor_abs());			// EOR $hhll
			OPCODE(0x4E, lsr_abs());			// LSR $hhll
			OPCODE(0x4F, bbr(4));				// BBR4 $ZZ,$rr

			OPCODE(0x50, bvc());				// BVC $rr
			OPCODE(0x51, eor_zpindy());			// EOR (IND),Y
			OPCODE(0x52, eor_zpind());			// EOR (IND)
			OPCODE(0x53, tam());				// TAMi
			OPCODE(0x54, csl());				// CSL
			OPCODE(0x55, eor_zpx());			// EOR $ZZ,X
			OPCODE(0x56, lsr_zpx());			// LSR $ZZ,X
			OPCODE(0x57, rmb(5));				// RMB5 $ZZ
			OPCODE(0x58, cli());				// CLI
			OPCODE(0x59, eor_absy());			// EOR $hhll,Y
			OPCODE(0x5A, phy());				// PHY
			OPCODE(0x5D, eor_absx());			// EOR $hhll,X
			OPCODE(0x5E, lsr_absx());			// LSR $hhll,X
			OPCODE(0x5F, bbr(5));				// BBR5 $ZZ,$rr

			OPCODE(0x60, rts());				// RTS
			OPCODE(0x61, adc_zpindx());			// ADC ($ZZ,X)
			OPCODE(0x62, cla());				// CLA
			OPCODE(0x64, stz_zp());				// STZ $ZZ
			OPCODE(0x65, adc_zp());				// ADC $ZZ
			OPCODE(0x66, ror_zp());				// ROR $ZZ
			OPCODE(0x67, rmb(6));				// RMB6 $ZZ
			OPCODE(0x68, pla());				// PLA
			OPCODE(0x69, adc_imm());			// ADC #$nn
			OPCODE(0x6A, ror_a());				// ROR A
			OPCODE(0x6C, jmp_absind());			// JMP ($hhll)
			OPCODE(0x6D, adc_abs());			// ADC $hhll
			OPCODE(0x6E, ror_abs());			// ROR $hhll
			OPCODE(0x6F, bbr(6));				// BBR6 $ZZ,$rr

			OPCODE(0x70, bvs());				// BVS $rr
			OPCODE(0x71, adc_zpindy());			// ADC ($ZZ),Y
			OPCODE(0x72, adc_zpind());			// ADC ($ZZ)
			OPCODE(0x73, tii());				// TII $SHSL,$DHDL,$LHLL
			OPCODE(0x74, stz_zpx());			// STZ $ZZ,X
			OPCODE(0x75, adc_zpx());			// ADC $ZZ,X
			OPCODE(0x76, ror_zpx());			// ROR $ZZ,X
			OPCODE(0x77, rmb(7));				// RMB7 $ZZ
			OPCODE(0x78, sei());				// SEI
			OPCODE(0x79, adc_absy());			// ADC $hhll,Y
			OPCODE(0x7A, ply());				// PLY
			OPCODE(0x7C, jmp_absindx())			// JMP $hhll,X
			OPCODE(0x7D, adc_absx());			// ADC $hhll,X
			OPCODE(0x7E, ror_absx());			// ROR $hhll,X
			OPCODE(0x7F, bbr(7));				// BBR7 $ZZ,$rr

			OPCODE(0x80, bra());				// BRA $rr
			OPCODE(0x81, sta_zpindx());			// STA (IND,X)
			OPCODE(0x82, clx());				// CLX
			OPCODE(0x83, tstins_zp());			// TST #$nn,$ZZ
			OPCODE(0x84, sty_zp());				// STY $ZZ
			OPCODE(0x85, sta_zp());				// STA $ZZ
			OPCODE(0x86, stx_zp());				// STX $ZZ
			OPCODE(0x87, smb(0));				// SMB0 $ZZ
			OPCODE(0x88, dey());				// DEY
			OPCODE(0x89, bit_imm());			// BIT #$nn
			OPCODE(0x8A, txa());				// TXA
			OPCODE(0x8C, sty_abs());			// STY $hhll
			OPCODE(0x8D, sta_abs());			// STA $hhll
			OPCODE(0x8E, stx_abs());			// STX $hhll
			OPCODE(0x8F, bbs(0));				// BBS0 $ZZ,$rr

			OPCODE(0x90, bcc());				// BCC $rr
			OPCODE(0x91, sta_zpindy());			// STA (IND),Y
			OPCODE(0x92, sta_zpind());			// STA (IND)
			OPCODE(0x93, tstins_abs());			// TST #$nn,$hhll
			OPCODE(0x94, sty_zpx());			// STY $ZZ,X
			OPCODE(0x95, sta_zpx());			// STA $ZZ,X
			OPCODE(0x96, stx_zpy());			// STX $ZZ,Y
			OPCODE(0x97, smb(1));				// SMB1 $ZZ
			OPCODE(0x98, tya());				// TYA
			OPCODE(0x99, sta_absy());			// STA $hhll,Y
			OPCODE(0x9A, txs());				// TXS
			OPCODE(0x9C, stz_abs());			// STZ $hhll
			OPCODE(0x9D, sta_absx());			// STA $hhll,X
			OPCODE(0x9E, stz_absx());			// STZ $hhll,X
			OPCODE(0x9F, bbs(1));				// BBS1 $ZZ,$rr

			OPCODE(0xA0, ldy_imm());			// LDY #$nn
			OPCODE(0xA1, lda_zpindx());			// LDA (IND,X)
			OPCODE(0xA2, ldx_imm());			// LDX #$nn
			OPCODE(0xA3, tstins_zpx());			// TST #$nn,$ZZ,X
			OPCODE(0xA4, ldy_zp());				// LDY $ZZ
			OPCODE(0xA5, lda_zp());				// LDA $ZZ
			OPCODE(0xA6, ldx_zp());				// LDX $ZZ
			OPCODE(0xA7, smb(2));				// SMB2 $ZZ
			OPCODE(0xA8, tay());				// TAY
			OPCODE(0xA9, lda_imm());			// LDA #$nn
			OPCODE(0xAA, tax());				// TAX
			OPCODE(0xAC, ldy_abs());			// LDY $hhll
			OPCODE(0xAD, lda_abs());			// LDA $hhll
			OPCODE(0xAE, ldx_abs());			// LDX $hhll
			OPCODE(0xAF, bbs(2));				// BBS2 $ZZ,$rr

			OPCODE(0xB0, bcs());				// BCS $rr
			OPCODE(0xB1, lda_zpindy());			// LDA (IND),Y
			OPCODE(0xB2, lda_zpind());			// LDA  (IND)
			OPCODE(0xB3, tstins_absx())			// TST #$nn,$hhll,X
			OPCODE(0xB4, ldy_zpx());			// LDY $ZZ,X
			OPCODE(0xB5, lda_zpx());			// LDA $ZZ,X
			OPCODE(0xB6, ldx_zpy());			// LDX $ZZ,Y
			OPCODE(0xB7, smb(3));				// SMB3 $ZZ
			OPCODE(0xB8, clv());				// CLV
			OPCODE(0xB9, lda_absy());			// LDA $hhll,Y
			OPCODE(0xBA, tsx());				// TSX
			OPCODE(0xBC, ldy_absx());			// LDY $hhll,X
			OPCODE(0xBD, lda_absx());			// LDA $hhll,X
			OPCODE(0xBE, ldx_absy());			// LDX $hhll,Y
			OPCODE(0xBF, bbs(3));				// BBS3 $ZZ,$rr

			OPCODE(0xC0, cpy_imm());			// CPY #$nn
			OPCODE(0xC1, cmp_zpindx());			// CMP (IND,X)
			OPCODE(0xC2, cly());				// CLY
			OPCODE(0xC3, tdd());				// TDD $SHSL,$DHDL,$LHLL
			OPCODE(0xC4, cpy_zp());				// CPY $ZZ
			OPCODE(0xC5, cmp_zp());				// CMP $ZZ
			OPCODE(0xC6, dec_zp());				// DEC $ZZ
			OPCODE(0xC7, smb(4));				// SMB4 $ZZ
			OPCODE(0xC8, iny());				// INY
			OPCODE(0xC9, cmp_imm());			// CMP #$nn
			OPCODE(0xCA, dex());				// DEX
			OPCODE(0xCC, cpy_abs());			// CPY $hhll
			OPCODE(0xCD, cmp_abs());			// CMP $hhll
			OPCODE(0xCE, dec_abs());			// DEC $hhll
			OPCODE(0xCF, bbs(4));				// BBS4 $ZZ,$rr

			OPCODE(0xD0, bne());				// BNE $rr
			OPCODE(0xD1, cmp_zpindy());			// CMP (IND),Y
			OPCODE(0xD2, cmp_zpind());			// CMP (IND)
			OPCODE(0xD3, tin());				// TIN $SHSL,$DHDL,$LHLL
			OPCODE(0xD4, csh());				// CSH
			OPCODE(0xD5, cmp_zpx());			// CMP $ZZ,X
			OPCODE(0xD6, dec_zpx());			// DEC $ZZ,X
			OPCODE(0xD7, smb(5));				// SMB5 $ZZ
			OPCODE(0xD8, cld());				// CLD
			OPCODE(0xD9, cmp_absy());			// CMP $hhll,Y
			OPCODE(0xDA, phx());				// PHX
			OPCODE(0xDD, cmp_absx());			// CMP $hhll,X
			OPCODE(0xDE, dec_absx());			// DEC $hhll,X
			OPCODE(0xDF, bbs(5));				// BBS5 $ZZ,$rr

			OPCODE(0xE0, cpx_imm());			// CPX #$nn
			OPCODE(0xE1, sbc_zpindx());			// SBC (IND,X)
			OPCODE(0xE3, tia());				// TIA $SHSL,$DHDL,$LHLL
			OPCODE(0xE4, cpx_zp());				// CPX $ZZ
			OPCODE(0xE5, sbc_zp());				// SBC $ZZ
			OPCODE(0xE6, inc_zp());				// INC $ZZ
			OPCODE(0xE7, smb(6));				// SMB6 $ZZ
			OPCODE(0xE8, inx());				// INX
			OPCODE(0xE9, sbc_imm());			// SBC #$nn
			OPCODE(0xEA, nop());				// NOP
			OPCODE(0xEC, cpx_abs());			// CPX $hhll
			OPCODE(0xED, sbc_abs());			// SBC $hhll
			OPCODE(0xEE, inc_abs());			// INC $hhll
			OPCODE(0xEF, bbs(6));				// BBS6 $ZZ,$rr
			OPCODE(0xF0, beq());				// BEQ $rr

			OPCODE(0xF1, sbc_zpindy());			// SBC (IND),Y
			OPCODE(0xF2, sbc_zpind());			// SBC (IND)
			OPCODE(0xF3, tai());				// TAI $SHSL,$DHDL,$LHLL
			OPCODE(0xF4, set());				// SET
			OPCODE(0xF5, sbc_zpx());			// SBC $ZZ,X
			OPCODE(0xF6, inc_zpx());			// INC $ZZ,X
			OPCODE(0xF7, smb(7));				// SMB7 $ZZ
			OPCODE(0xF8, sed());				// SED
			OPCODE(0xF9, sbc_absy());			// SBC $hhll,Y
			OPCODE(0xFA, plx());				// PLX
			OPCODE(0xFD, sbc_absx());			// SBC $hhll,X
			OPCODE(0xFE, inc_absx());			// INC $hhll,X
			OPCODE(0xFF, bbs(7));				// BBS7 $ZZ,$rr

			default:
				// Illegal opcodes are treated as NOP
				MESSAGE_DEBUG("Illegal opcode 0x%02X at pc=0x%04X!\n", opcode, CPU.PC);
				nop();
		}
	}
}
