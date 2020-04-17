#include "optable.h"
#include "dis.h"

#ifdef MY_EXCLUDE
#else

/* A bit modified to include Break Points  : "BP?" at $?B */
/* Beware, the name does matter, invalid opcodes got "???" as name or are BP */

operation_debug optable_debug[256] = {
	{AM_IMPL, "BRK", &follow_straight}
	,							/* $00 */
	{AM_ZPINDX, "ORA", &follow_straight}
	,							/* $01 */
	{AM_IMPL, "SXY", &follow_straight}
	,							/* $02 */
	{AM_IMMED, "ST0", &follow_straight}
	,							/* $03 */
	{AM_ZP, "TSB", &follow_straight}
	,							/* $04 */
	{AM_ZP, "ORA", &follow_straight}
	,							/* $05 */
	{AM_ZP, "ASL", &follow_straight}
	,							/* $06 */
	{AM_ZP, "RMB0", &follow_straight}
	,							/* $07 */
	{AM_IMPL, "PHP", &follow_straight}
	,							/* $08 */
	{AM_IMMED, "ORA", &follow_straight}
	,							/* $09 */
	{AM_IMPL, "ASL", &follow_straight}
	,							/* $0A */
	{AM_IMPL, "BP0", &follow_straight}
	,							/* $0B */
	{AM_ABS, "TSB", &follow_straight}
	,							/* $0C */
	{AM_ABS, "ORA", &follow_straight}
	,							/* $0D */
	{AM_ABS, "ASL", &follow_straight}
	,							/* $0E */
	{AM_PSREL, "BBR0", &follow_BBRi}
	,							/* $0F */
	{AM_REL, "BPL", &follow_BPL}
	,							/* $10 */
	{AM_ZPINDY, "ORA", &follow_straight}
	,							/* $11 */
	{AM_ZPIND, "ORA", &follow_straight}
	,							/* $12 */
	{AM_IMMED, "ST1", &follow_straight}
	,							/* $13 */
	{AM_ZP, "TRB", &follow_straight}
	,							/* $14 */
	{AM_ZPX, "ORA", &follow_straight}
	,							/* $15 */
	{AM_ZPX, "ASL", &follow_straight}
	,							/* $16 */
	{AM_ZP, "RMB1", &follow_straight}
	,							/* $17 */
	{AM_IMPL, "CLC", &follow_straight}
	,							/* $18 */
	{AM_ABSY, "ORA", &follow_straight}
	,							/* $19 */
	{AM_IMPL, "INC", &follow_straight}
	,							/* $1A */
	{AM_IMPL, "BP1", &follow_straight}
	,							/* $1B */
	{AM_ABS, "TRB", &follow_straight}
	,							/* $1C */
	{AM_ABSX, "ORA", &follow_straight}
	,							/* $1D */
	{AM_ABSX, "ASL", &follow_straight}
	,							/* $1E */
	{AM_PSREL, "BBR1", &follow_BBRi}
	,							/* $1F */
	{AM_ABS, "JSR", &follow_JSR}
	,							/* $20 */
	{AM_ZPINDX, "AND", &follow_straight}
	,							/* $21 */
	{AM_IMPL, "SAX", &follow_straight}
	,							/* $22 */
	{AM_IMMED, "ST2", &follow_straight}
	,							/* $23 */
	{AM_ZP, "BIT", &follow_straight}
	,							/* $24 */
	{AM_ZP, "AND", &follow_straight}
	,							/* $25 */
	{AM_ZP, "ROL", &follow_straight}
	,							/* $26 */
	{AM_ZP, "RMB2", &follow_straight}
	,							/* $27 */
	{AM_IMPL, "PLP", &follow_straight}
	,							/* $28 */
	{AM_IMMED, "AND", &follow_straight}
	,							/* $29 */
	{AM_IMPL, "ROL", &follow_straight}
	,							/* $2A */
	{AM_IMPL, "BP2", &follow_straight}
	,							/* $2B */
	{AM_ABS, "BIT", &follow_straight}
	,							/* $2C */
	{AM_ABS, "AND", &follow_straight}
	,							/* $2D */
	{AM_ABS, "ROL", &follow_straight}
	,							/* $2E */
	{AM_PSREL, "BBR2", &follow_BBRi}
	,							/* $2F */
	{AM_REL, "BMI", &follow_BMI}
	,							/* $30 */
	{AM_ZPINDY, "AND", &follow_straight}
	,							/* $31 */
	{AM_ZPIND, "AND", &follow_straight}
	,							/* $32 */
	{AM_IMPL, "???", &follow_straight}
	,							/* $33 */
	{AM_ZPX, "BIT", &follow_straight}
	,							/* $34 */
	{AM_ZPX, "AND", &follow_straight}
	,							/* $35 */
	{AM_ZPX, "ROL", &follow_straight}
	,							/* $36 */
	{AM_ZP, "RMB3", &follow_straight}
	,							/* $37 */
	{AM_IMPL, "SEC", &follow_straight}
	,							/* $38 */
	{AM_ABSY, "AND", &follow_straight}
	,							/* $39 */
	{AM_IMPL, "DEC", &follow_straight}
	,							/* $3A */
	{AM_IMPL, "BP3", &follow_straight}
	,							/* $3B */
	{AM_ABSX, "BIT", &follow_straight}
	,							/* $3C */
	{AM_ABSX, "AND", &follow_straight}
	,							/* $3D */
	{AM_ABSX, "ROL", &follow_straight}
	,							/* $3E */
	{AM_PSREL, "BBR3", &follow_BBRi}
	,							/* $3F */
	{AM_IMPL, "RTI", &follow_RTI}
	,							/* $40 */
	{AM_ZPINDX, "EOR", &follow_straight}
	,							/* $41 */
	{AM_IMPL, "SAY", &follow_straight}
	,							/* $42 */
	{AM_IMMED, "TMA", &follow_straight}
	,							/* $43 */
	{AM_REL, "BSR", &follow_BSR}
	,							/* $44 */
	{AM_ZP, "EOR", &follow_straight}
	,							/* $45 */
	{AM_ZP, "LSR", &follow_straight}
	,							/* $46 */
	{AM_ZP, "RMB4", &follow_straight}
	,							/* $47 */
	{AM_IMPL, "PHA", &follow_straight}
	,							/* $48 */
	{AM_IMMED, "EOR", &follow_straight}
	,							/* $49 */
	{AM_IMPL, "LSR", &follow_straight}
	,							/* $4A */
	{AM_IMPL, "BP4", &follow_straight}
	,							/* $4B */
	{AM_ABS, "JMP", &follow_JMPabs}
	,							/* $4C */
	{AM_ABS, "EOR", &follow_straight}
	,							/* $4D */
	{AM_ABS, "LSR", &follow_straight}
	,							/* $4E */
	{AM_PSREL, "BBR4", &follow_BBRi}
	,							/* $4F */
	{AM_REL, "BVC", &follow_BVC}
	,							/* $50 */
	{AM_ZPINDY, "EOR", &follow_straight}
	,							/* $51 */
	{AM_ZPIND, "EOR", &follow_straight}
	,							/* $52 */
	{AM_IMMED, "TAM", &follow_straight}
	,							/* $53 */
	{AM_IMPL, "???", &follow_straight}
	,							/* $54 */
	{AM_ZPX, "EOR", &follow_straight}
	,							/* $55 */
	{AM_ZPX, "LSR", &follow_straight}
	,							/* $56 */
	{AM_ZP, "RMB5", &follow_straight}
	,							/* $57 */
	{AM_IMPL, "CLI", &follow_straight}
	,							/* $58 */
	{AM_ABSY, "EOR", &follow_straight}
	,							/* $59 */
	{AM_IMPL, "PHY", &follow_straight}
	,							/* $5A */
	{AM_IMPL, "BP5", &follow_straight}
	,							/* $5B */
	{AM_IMPL, "???", &follow_straight}
	,							/* $5C */
	{AM_ABSX, "EOR", &follow_straight}
	,							/* $5D */
	{AM_ABSX, "LSR", &follow_straight}
	,							/* $5E */
	{AM_PSREL, "BBR5", &follow_BBRi}
	,							/* $5F */
	{AM_IMPL, "RTS", &follow_RTS}
	,							/* $60 */
	{AM_ZPINDX, "ADC", &follow_straight}
	,							/* $61 */
	{AM_IMPL, "CLA", &follow_straight}
	,							/* $62 */
	{AM_IMPL, "???", &follow_straight}
	,							/* $63 */
	{AM_ZP, "STZ", &follow_straight}
	,							/* $64 */
	{AM_ZP, "ADC", &follow_straight}
	,							/* $65 */
	{AM_ZP, "ROR", &follow_straight}
	,							/* $66 */
	{AM_ZP, "RMB6", &follow_straight}
	,							/* $67 */
	{AM_IMPL, "PLA", &follow_straight}
	,							/* $68 */
	{AM_IMMED, "ADC", &follow_straight}
	,							/* $69 */
	{AM_IMPL, "ROR", &follow_straight}
	,							/* $6A */
	{AM_IMPL, "BP6", &follow_straight}
	,							/* $6B */
	{AM_ABSIND, "JMP", &follow_JMPindir}
	,							/* $6C */
	{AM_ABS, "ADC", &follow_straight}
	,							/* $6D */
	{AM_ABS, "ROR", &follow_straight}
	,							/* $6E */
	{AM_PSREL, "BBR6", &follow_BBRi}
	,							/* $6F */
	{AM_REL, "BVS", &follow_BVS}
	,							/* $70 */
	{AM_ZPINDY, "ADC", &follow_straight}
	,							/* $71 */
	{AM_ZPIND, "ADC", &follow_straight}
	,							/* $72 */
	{AM_XFER, "TII", &follow_straight}
	,							/* $73 */
	{AM_ZPX, "STZ", &follow_straight}
	,							/* $74 */
	{AM_ZPX, "ADC", &follow_straight}
	,							/* $75 */
	{AM_ZPX, "ROR", &follow_straight}
	,							/* $76 */
	{AM_ZP, "RMB7", &follow_straight}
	,							/* $77 */
	{AM_IMPL, "SEI", &follow_straight}
	,							/* $78 */
	{AM_ABSY, "ADC", &follow_straight}
	,							/* $79 */
	{AM_IMPL, "PLY", &follow_straight}
	,							/* $7A */
	{AM_IMPL, "BP7", &follow_straight}
	,							/* $7B */
	{AM_ABSINDX, "JMP", &follow_JMPindirX}
	,							/* $7C */
	{AM_ABSX, "ADC", &follow_straight}
	,							/* $7D */
	{AM_ABSX, "ROR", &follow_straight}
	,							/* $7E */
	{AM_PSREL, "BBR7", &follow_BBRi}
	,							/* $7F */
	{AM_REL, "BRA", &follow_BRA}
	,							/* $80 */
	{AM_ZPINDX, "STA", &follow_straight}
	,							/* $81 */
	{AM_IMPL, "CLX", &follow_straight}
	,							/* $82 */
	{AM_TST_ZP, "TST", &follow_straight}
	,							/* $83 */
	{AM_ZP, "STY", &follow_straight}
	,							/* $84 */
	{AM_ZP, "STA", &follow_straight}
	,							/* $85 */
	{AM_ZP, "STX", &follow_straight}
	,							/* $86 */
	{AM_ZP, "SMB0", &follow_straight}
	,							/* $87 */
	{AM_IMPL, "DEY", &follow_straight}
	,							/* $88 */
	{AM_IMMED, "BIT", &follow_straight}
	,							/* $89 */
	{AM_IMPL, "TXA", &follow_straight}
	,							/* $8A */
	{AM_IMPL, "BP8", &follow_straight}
	,							/* $8B */
	{AM_ABS, "STY", &follow_straight}
	,							/* $8C */
	{AM_ABS, "STA", &follow_straight}
	,							/* $8D */
	{AM_ABS, "STX", &follow_straight}
	,							/* $8E */
	{AM_PSREL, "BBS0", &follow_BBSi}
	,							/* $8F */
	{AM_REL, "BCC", &follow_BCC}
	,							/* $90 */
	{AM_ZPINDY, "STA", &follow_straight}
	,							/* $91 */
	{AM_ZPIND, "STA", &follow_straight}
	,							/* $92 */
	{AM_TST_ABS, "TST", &follow_straight}
	,							/* $93 */
	{AM_ZPX, "STY", &follow_straight}
	,							/* $94 */
	{AM_ZPX, "STA", &follow_straight}
	,							/* $95 */
	{AM_ZPY, "STX", &follow_straight}
	,							/* $96 */
	{AM_ZP, "SMB1", &follow_straight}
	,							/* $97 */
	{AM_IMPL, "TYA", &follow_straight}
	,							/* $98 */
	{AM_ABSY, "STA", &follow_straight}
	,							/* $99 */
	{AM_IMPL, "TXS", &follow_straight}
	,							/* $9A */
	{AM_IMPL, "BP9", &follow_straight}
	,							/* $9B */
	{AM_ABS, "STZ", &follow_straight}
	,							/* $9C */
	{AM_ABSX, "STA", &follow_straight}
	,							/* $9D */
	{AM_ABSX, "STZ", &follow_straight}
	,							/* $9E */
	{AM_PSREL, "BBS1", &follow_BBSi}
	,							/* $9F */
	{AM_IMMED, "LDY", &follow_straight}
	,							/* $A0 */
	{AM_ZPINDX, "LDA", &follow_straight}
	,							/* $A1 */
	{AM_IMMED, "LDX", &follow_straight}
	,							/* $A2 */
	{AM_TST_ZPX, "TST", &follow_straight}
	,							/* $A3 */
	{AM_ZP, "LDY", &follow_straight}
	,							/* $A4 */
	{AM_ZP, "LDA", &follow_straight}
	,							/* $A5 */
	{AM_ZP, "LDX", &follow_straight}
	,							/* $A6 */
	{AM_ZP, "SMB2", &follow_straight}
	,							/* $A7 */
	{AM_IMPL, "TAY", &follow_straight}
	,							/* $A8 */
	{AM_IMMED, "LDA", &follow_straight}
	,							/* $A9 */
	{AM_IMPL, "TAX", &follow_straight}
	,							/* $AA */
	{AM_IMPL, "BPA", &follow_straight}
	,							/* $AB */
	{AM_ABS, "LDY", &follow_straight}
	,							/* $AC */
	{AM_ABS, "LDA", &follow_straight}
	,							/* $AD */
	{AM_ABS, "LDX", &follow_straight}
	,							/* $AE */
	{AM_PSREL, "BBS2", &follow_BBSi}
	,							/* $AF */
	{AM_REL, "BCS", &follow_BCS}
	,							/* $B0 */
	{AM_ZPINDY, "LDA", &follow_straight}
	,							/* $B1 */
	{AM_ZPIND, "LDA", &follow_straight}
	,							/* $B2 */
	{AM_TST_ABSX, "TST", &follow_straight}
	,							/* $B3 */
	{AM_ZPX, "LDY", &follow_straight}
	,							/* $B4 */
	{AM_ZPX, "LDA", &follow_straight}
	,							/* $B5 */
	{AM_ZPY, "LDX", &follow_straight}
	,							/* $B6 */
	{AM_ZP, "SMB3", &follow_straight}
	,							/* $B7 */
	{AM_IMPL, "CLV", &follow_straight}
	,							/* $B8 */
	{AM_ABSY, "LDA", &follow_straight}
	,							/* $B9 */
	{AM_IMPL, "TSX", &follow_straight}
	,							/* $BA */
	{AM_IMPL, "BPB", &follow_straight}
	,							/* $BB */
	{AM_ABSX, "LDY", &follow_straight}
	,							/* $BC */
	{AM_ABSX, "LDA", &follow_straight}
	,							/* $BD */
	{AM_ABSY, "LDX", &follow_straight}
	,							/* $BE */
	{AM_PSREL, "BBS3", &follow_BBSi}
	,							/* $BF */
	{AM_IMMED, "CPY", &follow_straight}
	,							/* $C0 */
	{AM_ZPINDX, "CMP", &follow_straight}
	,							/* $C1 */
	{AM_IMPL, "CLY", &follow_straight}
	,							/* $C2 */
	{AM_XFER, "TDD", &follow_straight}
	,							/* $C3 */
	{AM_ZP, "CPY", &follow_straight}
	,							/* $C4 */
	{AM_ZP, "CMP", &follow_straight}
	,							/* $C5 */
	{AM_ZP, "DEC", &follow_straight}
	,							/* $C6 */
	{AM_ZP, "SMB4", &follow_straight}
	,							/* $C7 */
	{AM_IMPL, "INY", &follow_straight}
	,							/* $C8 */
	{AM_IMMED, "CMP", &follow_straight}
	,							/* $C9 */
	{AM_IMPL, "DEX", &follow_straight}
	,							/* $CA */
	{AM_IMPL, "BPC", &follow_straight}
	,							/* $CB */
	{AM_ABS, "CPY", &follow_straight}
	,							/* $CC */
	{AM_ABS, "CMP", &follow_straight}
	,							/* $CD */
	{AM_ABS, "DEC", &follow_straight}
	,							/* $CE */
	{AM_PSREL, "BBS4", &follow_BBSi}
	,							/* $CF */
	{AM_REL, "BNE", &follow_BNE}
	,							/* $D0 */
	{AM_ZPINDY, "CMP", &follow_straight}
	,							/* $D1 */
	{AM_ZPIND, "CMP", &follow_straight}
	,							/* $D2 */
	{AM_XFER, "TIN", &follow_straight}
	,							/* $D3 */
	{AM_IMPL, "CSH", &follow_straight}
	,							/* $D4 */
	{AM_ZPX, "CMP", &follow_straight}
	,							/* $D5 */
	{AM_ZPX, "DEC", &follow_straight}
	,							/* $D6 */
	{AM_ZP, "SMB5", &follow_straight}
	,							/* $D7 */
	{AM_IMPL, "CLD", &follow_straight}
	,							/* $D8 */
	{AM_ABSY, "CMP", &follow_straight}
	,							/* $D9 */
	{AM_IMPL, "PHX", &follow_straight}
	,							/* $DA */
	{AM_IMPL, "BPD", &follow_straight}
	,							/* $DB */
	{AM_IMPL, "???", &follow_straight}
	,							/* $DC */
	{AM_ABSX, "CMP", &follow_straight}
	,							/* $DD */
	{AM_ABSX, "DEC", &follow_straight}
	,							/* $DE */
	{AM_PSREL, "BBS5", &follow_BBSi}
	,							/* $DF */
	{AM_IMMED, "CPX", &follow_straight}
	,							/* $E0 */
	{AM_ZPINDX, "SBC", &follow_straight}
	,							/* $E1 */
	{AM_IMPL, "???", &follow_straight}
	,							/* $E2 */
	{AM_XFER, "TIA", &follow_straight}
	,							/* $E3 */
	{AM_ZP, "CPX", &follow_straight}
	,							/* $E4 */
	{AM_ZP, "SBC", &follow_straight}
	,							/* $E5 */
	{AM_ZP, "INC", &follow_straight}
	,							/* $E6 */
	{AM_ZP, "SMB6", &follow_straight}
	,							/* $E7 */
	{AM_IMPL, "INX", &follow_straight}
	,							/* $E8 */
	{AM_IMMED, "SBC", &follow_straight}
	,							/* $E9 */
	{AM_IMPL, "NOP", &follow_straight}
	,							/* $EA */
	{AM_IMPL, "BPE", &follow_straight}
	,							/* $EB */
	{AM_ABS, "CPX", &follow_straight}
	,							/* $EC */
	{AM_ABS, "SBC", &follow_straight}
	,							/* $ED */
	{AM_ABS, "INC", &follow_straight}
	,							/* $EE */
	{AM_PSREL, "BBS6", &follow_BBSi}
	,							/* $EF */
	{AM_REL, "BEQ", &follow_BEQ}
	,							/* $F0 */
	{AM_ZPINDY, "SBC", &follow_straight}
	,							/* $F1 */
	{AM_ZPIND, "SBC", &follow_straight}
	,							/* $F2 */
	{AM_XFER, "TAI", &follow_straight}
	,							/* $F3 */
	{AM_IMPL, "SET", &follow_straight}
	,							/* $F4 */
	{AM_ZPX, "SBC", &follow_straight}
	,							/* $F5 */
	{AM_ZPX, "INC", &follow_straight}
	,							/* $F6 */
	{AM_ZP, "SMB7", &follow_straight}
	,							/* $F7 */
	{AM_IMPL, "SED", &follow_straight}
	,							/* $F8 */
	{AM_ABSY, "SBC", &follow_straight}
	,							/* $F9 */
	{AM_IMPL, "PLX", &follow_straight}
	,							/* $FA */
	{AM_IMPL, "BPF", &follow_straight}
	,							/* $FB */
	{AM_IMPL, "CD EMU", &follow_RTS}
	,							/* $FC */
	{AM_ABSX, "SBC", &follow_straight}
	,							/* $FD */
	{AM_ABSX, "INC", &follow_straight}
	,							/* $FE */
	{AM_PSREL, "BBS7", &follow_BBSi}	/* $FF */
};


/* number of bytes per instruction in each addressing mode */

mode_struct_debug addr_info_debug[MAX_MODES] = {
	{1, &(implicit)}
	,							/* implicit              */
	{2, &(immed)}
	,							/* immediate             */
	{2, &(relative)}
	,							/* relative              */
	{2, &(ind_zp)}
	,							/* zero page             */
	{2, &(ind_zpx)}
	,							/* zero page, x          */
	{2, &(ind_zpy)}
	,							/* zero page, y          */
	{2, &(ind_zpind)}
	,							/* zero page indirect    */
	{2, &(ind_zpix)}
	,							/* zero page indirect, x */
	{2, &(ind_zpiy)}
	,							/* zero page indirect, y */
	{3, &(absol)}
	,							/* absolute              */
	{3, &(absx)}
	,							/* absolute, x           */
	{3, &(absy)}
	,							/* absolute, y           */
	{3, &(absind)}
	,							/* absolute indirect     */
	{3, &(absindx)}
	,							/* absolute indirect     */
	{3, &(pseudorel)}
	,							/* pseudo-relative       */
	{3, &(tst_zp)}
	,							/* special 'TST' addressing mode  */
	{4, &(tst_abs)}
	,							/* special 'TST' addressing mode  */
	{3, &(tst_zpx)}
	,							/* special 'TST' addressing mode  */
	{4, &(tst_absx)}
	,							/* special 'TST' addressing mode  */
	{7, &(xfer)}				/* special 7-byte transfer addressing mode  */
};

#endif
