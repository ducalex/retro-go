/****************************************************************************
 dis.h
 
 Data for TG Disassembler
 ****************************************************************************/

#include "defs.h"
#include "h6280.h"
#include "globals.h"
#include "bp.h"
#include "bios.h"

/* addressing modes: */

#define AM_IMPL      0			/* implicit              */
#define AM_IMMED     1			/* immediate             */
#define AM_REL       2			/* relative              */
#define AM_ZP        3			/* zero page             */
#define AM_ZPX       4			/* zero page, x          */
#define AM_ZPY       5			/* zero page, y          */
#define AM_ZPIND     6			/* zero page indirect    */
#define AM_ZPINDX    7			/* zero page indirect, x */
#define AM_ZPINDY    8			/* zero page indirect, y */
#define AM_ABS       9			/* absolute              */
#define AM_ABSX     10			/* absolute, x           */
#define AM_ABSY     11			/* absolute, y           */
#define AM_ABSIND   12			/* absolute indirect     */
#define AM_ABSINDX  13			/* absolute indirect     */
#define AM_PSREL    14			/* pseudo-relative       */
#define AM_TST_ZP   15			/* special 'TST' addressing mode  */
#define AM_TST_ABS  16			/* special 'TST' addressing mode  */
#define AM_TST_ZPX  17			/* special 'TST' addressing mode  */
#define AM_TST_ABSX 18			/* special 'TST' addressing mode  */
#define AM_XFER     19			/* special 7-byte transfer addressing mode  */

#define MAX_MODES  (AM_XFER + 1)

/* now define table contents: */
operation optable_runtime[256] = {
	{brek, AM_IMMED, "BRK"}
	,							/* $00 */
	{ora_zpindx, AM_ZPINDX, "ORA"}
	,							/* $01 */
	{sxy, AM_IMPL, "SXY"}
	,							/* $02 */
	{st0, AM_IMMED, "ST0"}
	,							/* $03 */
	{tsb_zp, AM_ZP, "TSB"}
	,							/* $04 */
	{ora_zp, AM_ZP, "ORA"}
	,							/* $05 */
	{asl_zp, AM_ZP, "ASL"}
	,							/* $06 */
	{rmb0, AM_ZP, "RMB0"}
	,							/* $07 */
	{php, AM_IMPL, "PHP"}
	,							/* $08 */
	{ora_imm, AM_IMMED, "ORA"}
	,							/* $09 */
	{asl_a, AM_IMPL, "ASL"}
	,							/* $0A */
	{handle_bp0, AM_IMPL, "BP0"}
	,							/* $0B */
	{tsb_abs, AM_ABS, "TSB"}
	,							/* $0C */
	{ora_abs, AM_ABS, "ORA"}
	,							/* $0D */
	{asl_abs, AM_ABS, "ASL"}
	,							/* $0E */
	{bbr0, AM_PSREL, "BBR0"}
	,							/* $0F */
	{bpl, AM_REL, "BPL"}
	,							/* $10 */
	{ora_zpindy, AM_ZPINDY, "ORA"}
	,							/* $11 */
	{ora_zpind, AM_ZPIND, "ORA"}
	,							/* $12 */
	{st1, AM_IMMED, "ST1"}
	,							/* $13 */
	{trb_zp, AM_ZP, "TRB"}
	,							/* $14 */
	{ora_zpx, AM_ZPX, "ORA"}
	,							/* $15 */
	{asl_zpx, AM_ZPX, "ASL"}
	,							/* $16 */
	{rmb1, AM_ZP, "RMB1"}
	,							/* $17 */
	{clc, AM_IMPL, "CLC"}
	,							/* $18 */
	{ora_absy, AM_ABSY, "ORA"}
	,							/* $19 */
	{inc_a, AM_IMPL, "INC"}
	,							/* $1A */
	{handle_bp1, AM_IMPL, "BP1"}
	,							/* $1B */
	{trb_abs, AM_ABS, "TRB"}
	,							/* $1C */
	{ora_absx, AM_ABSX, "ORA"}
	,							/* $1D */
	{asl_absx, AM_ABSX, "ASL"}
	,							/* $1E */
	{bbr1, AM_PSREL, "BBR1"}
	,							/* $1F */
	{jsr, AM_ABS, "JSR"}
	,							/* $20 */
	{and_zpindx, AM_ZPINDX, "AND"}
	,							/* $21 */
	{sax, AM_IMPL, "SAX"}
	,							/* $22 */
	{st2, AM_IMMED, "ST2"}
	,							/* $23 */
	{bit_zp, AM_ZP, "BIT"}
	,							/* $24 */
	{and_zp, AM_ZP, "AND"}
	,							/* $25 */
	{rol_zp, AM_ZP, "ROL"}
	,							/* $26 */
	{rmb2, AM_ZP, "RMB2"}
	,							/* $27 */
	{plp, AM_IMPL, "PLP"}
	,							/* $28 */
	{and_imm, AM_IMMED, "AND"}
	,							/* $29 */
	{rol_a, AM_IMPL, "ROL"}
	,							/* $2A */
	{handle_bp2, AM_IMPL, "BP2"}
	,							/* $2B */
	{bit_abs, AM_ABS, "BIT"}
	,							/* $2C */
	{and_abs, AM_ABS, "AND"}
	,							/* $2D */
	{rol_abs, AM_ABS, "ROL"}
	,							/* $2E */
	{bbr2, AM_PSREL, "BBR2"}
	,							/* $2F */
	{bmi, AM_REL, "BMI"}
	,							/* $30 */
	{and_zpindy, AM_ZPINDY, "AND"}
	,							/* $31 */
	{and_zpind, AM_ZPIND, "AND"}
	,							/* $32 */
	{halt, AM_IMPL, "???"}
	,							/* $33 */
	{bit_zpx, AM_ZPX, "BIT"}
	,							/* $34 */
	{and_zpx, AM_ZPX, "AND"}
	,							/* $35 */
	{rol_zpx, AM_ZPX, "ROL"}
	,							/* $36 */
	{rmb3, AM_ZP, "RMB3"}
	,							/* $37 */
	{sec, AM_IMPL, "SEC"}
	,							/* $38 */
	{and_absy, AM_ABSY, "AND"}
	,							/* $39 */
	{dec_a, AM_IMPL, "DEC"}
	,							/* $3A */
	{handle_bp3, AM_IMPL, "BP3"}
	,							/* $3B */
	{bit_absx, AM_ABSX, "BIT"}
	,							/* $3C */
	{and_absx, AM_ABSX, "AND"}
	,							/* $3D */
	{rol_absx, AM_ABSX, "ROL"}
	,							/* $3E */
	{bbr3, AM_PSREL, "BBR3"}
	,							/* $3F */
	{rti, AM_IMPL, "RTI"}
	,							/* $40 */
	{eor_zpindx, AM_ZPINDX, "EOR"}
	,							/* $41 */
	{say, AM_IMPL, "SAY"}
	,							/* $42 */
	{tma, AM_IMMED, "TMA"}
	,							/* $43 */
	{bsr, AM_REL, "BSR"}
	,							/* $44 */
	{eor_zp, AM_ZP, "EOR"}
	,							/* $45 */
	{lsr_zp, AM_ZP, "LSR"}
	,							/* $46 */
	{rmb4, AM_ZP, "RMB4"}
	,							/* $47 */
	{pha, AM_IMPL, "PHA"}
	,							/* $48 */
	{eor_imm, AM_IMMED, "EOR"}
	,							/* $49 */
	{lsr_a, AM_IMPL, "LSR"}
	,							/* $4A */
	{handle_bp4, AM_IMPL, "BP4"}
	,							/* $4B */
	{jmp, AM_ABS, "JMP"}
	,							/* $4C */
	{eor_abs, AM_ABS, "EOR"}
	,							/* $4D */
	{lsr_abs, AM_ABS, "LSR"}
	,							/* $4E */
	{bbr4, AM_PSREL, "BBR4"}
	,							/* $4F */
	{bvc, AM_REL, "BVC"}
	,							/* $50 */
	{eor_zpindy, AM_ZPINDY, "EOR"}
	,							/* $51 */
	{eor_zpind, AM_ZPIND, "EOR"}
	,							/* $52 */
	{tam, AM_IMMED, "TAM"}
	,							/* $53 */
	{nop, AM_IMPL, "CSL"}
	,							/* $54 */
	{eor_zpx, AM_ZPX, "EOR"}
	,							/* $55 */
	{lsr_zpx, AM_ZPX, "LSR"}
	,							/* $56 */
	{rmb5, AM_ZP, "RMB5"}
	,							/* $57 */
	{cli, AM_IMPL, "CLI"}
	,							/* $58 */
	{eor_absy, AM_ABSY, "EOR"}
	,							/* $59 */
	{phy, AM_IMPL, "PHY"}
	,							/* $5A */
	{handle_bp5, AM_IMPL, "BP5"}
	,							/* $5B */
	{halt, AM_IMPL, "???"}
	,							/* $5C */
	{eor_absx, AM_ABSX, "EOR"}
	,							/* $5D */
	{lsr_absx, AM_ABSX, "LSR"}
	,							/* $5E */
	{bbr5, AM_PSREL, "BBR5"}
	,							/* $5F */
	{rts, AM_IMPL, "RTS"}
	,							/* $60 */
	{adc_zpindx, AM_ZPINDX, "ADC"}
	,							/* $61 */
	{cla, AM_IMPL, "CLA"}
	,							/* $62 */
	{halt, AM_IMPL, "???"}
	,							/* $63 */
	{stz_zp, AM_ZP, "STZ"}
	,							/* $64 */
	{adc_zp, AM_ZP, "ADC"}
	,							/* $65 */
	{ror_zp, AM_ZP, "ROR"}
	,							/* $66 */
	{rmb6, AM_ZP, "RMB6"}
	,							/* $67 */
	{pla, AM_IMPL, "PLA"}
	,							/* $68 */
	{adc_imm, AM_IMMED, "ADC"}
	,							/* $69 */
	{ror_a, AM_IMPL, "ROR"}
	,							/* $6A */
	{handle_bp6, AM_IMPL, "BP6"}
	,							/* $6B */
	{jmp_absind, AM_ABSIND, "JMP"}
	,							/* $6C */
	{adc_abs, AM_ABS, "ADC"}
	,							/* $6D */
	{ror_abs, AM_ABS, "ROR"}
	,							/* $6E */
	{bbr6, AM_PSREL, "BBR6"}
	,							/* $6F */
	{bvs, AM_REL, "BVS"}
	,							/* $70 */
	{adc_zpindy, AM_ZPINDY, "ADC"}
	,							/* $71 */
	{adc_zpind, AM_ZPIND, "ADC"}
	,							/* $72 */
	{tii, AM_XFER, "TII"}
	,							/* $73 */
	{stz_zpx, AM_ZPX, "STZ"}
	,							/* $74 */
	{adc_zpx, AM_ZPX, "ADC"}
	,							/* $75 */
	{ror_zpx, AM_ZPX, "ROR"}
	,							/* $76 */
	{rmb7, AM_ZP, "RMB7"}
	,							/* $77 */
	{sei, AM_IMPL, "SEI"}
	,							/* $78 */
	{adc_absy, AM_ABSY, "ADC"}
	,							/* $79 */
	{ply, AM_IMPL, "PLY"}
	,							/* $7A */
	{handle_bp7, AM_IMPL, "BP7"}
	,							/* $7B */
	{jmp_absindx, AM_ABSINDX, "JMP"}
	,							/* $7C */
	{adc_absx, AM_ABSX, "ADC"}
	,							/* $7D */
	{ror_absx, AM_ABSX, "ROR"}
	,							/* $7E */
	{bbr7, AM_PSREL, "BBR7"}
	,							/* $7F */
	{bra, AM_REL, "BRA"}
	,							/* $80 */
	{sta_zpindx, AM_ZPINDX, "STA"}
	,							/* $81 */
	{clx, AM_IMPL, "CLX"}
	,							/* $82 */
	{tstins_zp, AM_TST_ZP, "TST"}
	,							/* $83 */
	{sty_zp, AM_ZP, "STY"}
	,							/* $84 */
	{sta_zp, AM_ZP, "STA"}
	,							/* $85 */
	{stx_zp, AM_ZP, "STX"}
	,							/* $86 */
	{smb0, AM_ZP, "SMB0"}
	,							/* $87 */
	{dey, AM_IMPL, "DEY"}
	,							/* $88 */
	{bit_imm, AM_IMMED, "BIT"}
	,							/* $89 */
	{txa, AM_IMPL, "TXA"}
	,							/* $8A */
	{handle_bp8, AM_IMPL, "BP8"}
	,							/* $8B */
	{sty_abs, AM_ABS, "STY"}
	,							/* $8C */
	{sta_abs, AM_ABS, "STA"}
	,							/* $8D */
	{stx_abs, AM_ABS, "STX"}
	,							/* $8E */
	{bbs0, AM_PSREL, "BBS0"}
	,							/* $8F */
	{bcc, AM_REL, "BCC"}
	,							/* $90 */
	{sta_zpindy, AM_ZPINDY, "STA"}
	,							/* $91 */
	{sta_zpind, AM_ZPIND, "STA"}
	,							/* $92 */
	{tstins_abs, AM_TST_ABS, "TST"}
	,							/* $93 */
	{sty_zpx, AM_ZPX, "STY"}
	,							/* $94 */
	{sta_zpx, AM_ZPX, "STA"}
	,							/* $95 */
	{stx_zpy, AM_ZPY, "STX"}
	,							/* $96 */
	{smb1, AM_ZP, "SMB1"}
	,							/* $97 */
	{tya, AM_IMPL, "TYA"}
	,							/* $98 */
	{sta_absy, AM_ABSY, "STA"}
	,							/* $99 */
	{txs, AM_IMPL, "TXS"}
	,							/* $9A */
	{handle_bp9, AM_IMPL, "BP9"}
	,							/* $9B */
	{stz_abs, AM_ABS, "STZ"}
	,							/* $9C */
	{sta_absx, AM_ABSX, "STA"}
	,							/* $9D */
	{stz_absx, AM_ABSX, "STZ"}
	,							/* $9E */
	{bbs1, AM_PSREL, "BBS1"}
	,							/* $9F */
	{ldy_imm, AM_IMMED, "LDY"}
	,							/* $A0 */
	{lda_zpindx, AM_ZPINDX, "LDA"}
	,							/* $A1 */
	{ldx_imm, AM_IMMED, "LDX"}
	,							/* $A2 */
	{tstins_zpx, AM_TST_ZPX, "TST"}
	,							/* $A3 */
	{ldy_zp, AM_ZP, "LDY"}
	,							/* $A4 */
	{lda_zp, AM_ZP, "LDA"}
	,							/* $A5 */
	{ldx_zp, AM_ZP, "LDX"}
	,							/* $A6 */
	{smb2, AM_ZP, "SMB2"}
	,							/* $A7 */
	{tay, AM_IMPL, "TAY"}
	,							/* $A8 */
	{lda_imm, AM_IMMED, "LDA"}
	,							/* $A9 */
	{tax, AM_IMPL, "TAX"}
	,							/* $AA */
	{handle_bp10, AM_IMPL, "BPA"}
	,							/* $AB */
	{ldy_abs, AM_ABS, "LDY"}
	,							/* $AC */
	{lda_abs, AM_ABS, "LDA"}
	,							/* $AD */
	{ldx_abs, AM_ABS, "LDX"}
	,							/* $AE */
	{bbs2, AM_PSREL, "BBS2"}
	,							/* $AF */
	{bcs, AM_REL, "BCS"}
	,							/* $B0 */
	{lda_zpindy, AM_ZPINDY, "LDA"}
	,							/* $B1 */
	{lda_zpind, AM_ZPIND, "LDA"}
	,							/* $B2 */
	{tstins_absx, AM_TST_ABSX, "TST"}
	,							/* $B3 */
	{ldy_zpx, AM_ZPX, "LDY"}
	,							/* $B4 */
	{lda_zpx, AM_ZPX, "LDA"}
	,							/* $B5 */
	{ldx_zpy, AM_ZPY, "LDX"}
	,							/* $B6 */
	{smb3, AM_ZP, "SMB3"}
	,							/* $B7 */
	{clv, AM_IMPL, "CLV"}
	,							/* $B8 */
	{lda_absy, AM_ABSY, "LDA"}
	,							/* $B9 */
	{tsx, AM_IMPL, "TSX"}
	,							/* $BA */
	{handle_bp11, AM_IMPL, "BPB"}
	,							/* $BB */
	{ldy_absx, AM_ABSX, "LDY"}
	,							/* $BC */
	{lda_absx, AM_ABSX, "LDA"}
	,							/* $BD */
	{ldx_absy, AM_ABSY, "LDX"}
	,							/* $BE */
	{bbs3, AM_PSREL, "BBS3"}
	,							/* $BF */
	{cpy_imm, AM_IMMED, "CPY"}
	,							/* $C0 */
	{cmp_zpindx, AM_ZPINDX, "CMP"}
	,							/* $C1 */
	{cly, AM_IMPL, "CLY"}
	,							/* $C2 */
	{tdd, AM_XFER, "TDD"}
	,							/* $C3 */
	{cpy_zp, AM_ZP, "CPY"}
	,							/* $C4 */
	{cmp_zp, AM_ZP, "CMP"}
	,							/* $C5 */
	{dec_zp, AM_ZP, "DEC"}
	,							/* $C6 */
	{smb4, AM_ZP, "SMB4"}
	,							/* $C7 */
	{iny, AM_IMPL, "INY"}
	,							/* $C8 */
	{cmp_imm, AM_IMMED, "CMP"}
	,							/* $C9 */
	{dex, AM_IMPL, "DEX"}
	,							/* $CA */
	{handle_bp12, AM_IMPL, "BPC"}
	,							/* $CB */
	{cpy_abs, AM_ABS, "CPY"}
	,							/* $CC */
	{cmp_abs, AM_ABS, "CMP"}
	,							/* $CD */
	{dec_abs, AM_ABS, "DEC"}
	,							/* $CE */
	{bbs4, AM_PSREL, "BBS4"}
	,							/* $CF */
	{bne, AM_REL, "BNE"}
	,							/* $D0 */
	{cmp_zpindy, AM_ZPINDY, "CMP"}
	,							/* $D1 */
	{cmp_zpind, AM_ZPIND, "CMP"}
	,							/* $D2 */
	{tin, AM_XFER, "TIN"}
	,							/* $D3 */
	{nop, AM_IMPL, "CSH"}
	,							/* $D4 */
	{cmp_zpx, AM_ZPX, "CMP"}
	,							/* $D5 */
	{dec_zpx, AM_ZPX, "DEC"}
	,							/* $D6 */
	{smb5, AM_ZP, "SMB5"}
	,							/* $D7 */
	{cld, AM_IMPL, "CLD"}
	,							/* $D8 */
	{cmp_absy, AM_ABSY, "CMP"}
	,							/* $D9 */
	{phx, AM_IMPL, "PHX"}
	,							/* $DA */
	{handle_bp13, AM_IMPL, "BPD"}
	,							/* $DB */
	{halt, AM_IMPL, "???"}
	,							/* $DC */
	{cmp_absx, AM_ABSX, "CMP"}
	,							/* $DD */
	{dec_absx, AM_ABSX, "DEC"}
	,							/* $DE */
	{bbs5, AM_PSREL, "BBS5"}
	,							/* $DF */
	{cpx_imm, AM_IMMED, "CPX"}
	,							/* $E0 */
	{sbc_zpindx, AM_ZPINDX, "SBC"}
	,							/* $E1 */
	{halt, AM_IMPL, "???"}
	,							/* $E2 */
	{tia, AM_XFER, "TIA"}
	,							/* $E3 */
	{cpx_zp, AM_ZP, "CPX"}
	,							/* $E4 */
	{sbc_zp, AM_ZP, "SBC"}
	,							/* $E5 */
	{inc_zp, AM_ZP, "INC"}
	,							/* $E6 */
	{smb6, AM_ZP, "SMB6"}
	,							/* $E7 */
	{inx, AM_IMPL, "INX"}
	,							/* $E8 */
	{sbc_imm, AM_IMMED, "SBC"}
	,							/* $E9 */
	{nop, AM_IMPL, "NOP"}
	,							/* $EA */
	{handle_bp14, AM_IMPL, "BPE"}
	,							/* $EB */
	{cpx_abs, AM_ABS, "CPX"}
	,							/* $EC */
	{sbc_abs, AM_ABS, "SBC"}
	,							/* $ED */
	{inc_abs, AM_ABS, "INC"}
	,							/* $EE */
	{bbs6, AM_PSREL, "BBS6"}
	,							/* $EF */
	{beq, AM_REL, "BEQ"}
	,							/* $F0 */
	{sbc_zpindy, AM_ZPINDY, "SBC"}
	,							/* $F1 */
	{sbc_zpind, AM_ZPIND, "SBC"}
	,							/* $F2 */
	{tai, AM_XFER, "TAI"}
	,							/* $F3 */
	{set, AM_IMPL, "SET"}
	,							/* $F4 */
	{sbc_zpx, AM_ZPX, "SBC"}
	,							/* $F5 */
	{inc_zpx, AM_ZPX, "INC"}
	,							/* $F6 */
	{smb7, AM_ZP, "SMB7"}
	,							/* $F7 */
	{sed, AM_IMPL, "SED"}
	,							/* $F8 */
	{sbc_absy, AM_ABSY, "SBC"}
	,							/* $F9 */
	{plx, AM_IMPL, "PLX"}
	,							/* $FA */
	{handle_bp15, AM_IMPL, "BPF"}
	,							/* $FB */
	{handle_bios, AM_IMPL, "???"}
	,							/* $FC */
	{sbc_absx, AM_ABSX, "SBC"}
	,							/* $FD */
	{inc_absx, AM_ABSX, "INC"}
	,							/* $FE */
	{bbs7, AM_PSREL, "BBS7"}	/* $FF */
};
