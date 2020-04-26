#ifndef _DJGPP_INCLUDE_DIS_CST_H
#define _DJGPP_INCLUDE_DIS_CST_H

/* now define table contents: */

typedef struct op {
	int addr_mode;
	const char *opname;
} operation;

/* A bit modified to include Break Points  : "BP?" at $?B */
/* Beware, the name does import, invalid opcodes got "???" as name or are BP */

const operation optable[256] = {
	{AM_IMPL, "BRK"},			/* $00 */
	{AM_ZPINDX, "ORA"},			/* $01 */
	{AM_IMPL, "SXY"},			/* $02 */
	{AM_IMMED, "ST0"},			/* $03 */
	{AM_ZP, "TSB"},				/* $04 */
	{AM_ZP, "ORA"},				/* $05 */
	{AM_ZP, "ASL"},				/* $06 */
	{AM_ZP, "RMB0"},			/* $07 */
	{AM_IMPL, "PHP"},			/* $08 */
	{AM_IMMED, "ORA"},			/* $09 */
	{AM_IMPL, "ASL"},			/* $0A */
	{AM_IMPL, "BP0"},			/* $0B */
	{AM_ABS, "TSB"},			/* $0C */
	{AM_ABS, "ORA"},			/* $0D */
	{AM_ABS, "ASL"},			/* $0E */
	{AM_PSREL, "BBR0"},			/* $0F */
	{AM_REL, "BPL"},			/* $10 */
	{AM_ZPINDY, "ORA"},			/* $11 */
	{AM_ZPIND, "ORA"},			/* $12 */
	{AM_IMMED, "ST1"},			/* $13 */
	{AM_ZP, "TRB"},				/* $14 */
	{AM_ZPX, "ORA"},			/* $15 */
	{AM_ZPX, "ASL"},			/* $16 */
	{AM_ZP, "RMB1"},			/* $17 */
	{AM_IMPL, "CLC"},			/* $18 */
	{AM_ABSY, "ORA"},			/* $19 */
	{AM_IMPL, "INC"},			/* $1A */
	{AM_IMPL, "BP1"},			/* $1B */
	{AM_ABS, "TRB"},			/* $1C */
	{AM_ABSX, "ORA"},			/* $1D */
	{AM_ABSX, "ASL"},			/* $1E */
	{AM_PSREL, "BBR1"},			/* $1F */
	{AM_ABS, "JSR"},			/* $20 */
	{AM_ZPINDX, "AND"},			/* $21 */
	{AM_IMPL, "SAX"},			/* $22 */
	{AM_IMMED, "ST2"},			/* $23 */
	{AM_ZP, "BIT"},				/* $24 */
	{AM_ZP, "AND"},				/* $25 */
	{AM_ZP, "ROL"},				/* $26 */
	{AM_ZP, "RMB2"},			/* $27 */
	{AM_IMPL, "PLP"},			/* $28 */
	{AM_IMMED, "AND"},			/* $29 */
	{AM_IMPL, "ROL"},			/* $2A */
	{AM_IMPL, "BP2"},			/* $2B */
	{AM_ABS, "BIT"},			/* $2C */
	{AM_ABS, "AND"},			/* $2D */
	{AM_ABS, "ROL"},			/* $2E */
	{AM_PSREL, "BBR2"},			/* $2F */
	{AM_REL, "BMI"},			/* $30 */
	{AM_ZPINDY, "AND"},			/* $31 */
	{AM_ZPIND, "AND"},			/* $32 */
	{AM_IMPL, "???"},			/* $33 */
	{AM_ZPX, "BIT"},			/* $34 */
	{AM_ZPX, "AND"},			/* $35 */
	{AM_ZPX, "ROL"},			/* $36 */
	{AM_ZP, "RMB3"},			/* $37 */
	{AM_IMPL, "SEC"},			/* $38 */
	{AM_ABSY, "AND"},			/* $39 */
	{AM_IMPL, "DEC"},			/* $3A */
	{AM_IMPL, "BP3"},			/* $3B */
	{AM_ABSX, "BIT"},			/* $3C */
	{AM_ABSX, "AND"},			/* $3D */
	{AM_ABSX, "ROL"},			/* $3E */
	{AM_PSREL, "BBR3"},			/* $3F */
	{AM_IMPL, "RTI"},			/* $40 */
	{AM_ZPINDX, "EOR"},			/* $41 */
	{AM_IMPL, "SAY"},			/* $42 */
	{AM_IMMED, "TMA"},			/* $43 */
	{AM_REL, "BSR"},			/* $44 */
	{AM_ZP, "EOR"},				/* $45 */
	{AM_ZP, "LSR"},				/* $46 */
	{AM_ZP, "RMB4"},			/* $47 */
	{AM_IMPL, "PHA"},			/* $48 */
	{AM_IMMED, "EOR"},			/* $49 */
	{AM_IMPL, "LSR"},			/* $4A */
	{AM_IMPL, "BP4"},			/* $4B */
	{AM_ABS, "JMP"},			/* $4C */
	{AM_ABS, "EOR"},			/* $4D */
	{AM_ABS, "LSR"},			/* $4E */
	{AM_PSREL, "BBR4"},			/* $4F */
	{AM_REL, "BVC"},			/* $50 */
	{AM_ZPINDY, "EOR"},			/* $51 */
	{AM_ZPIND, "EOR"},			/* $52 */
	{AM_IMMED, "TAM"},			/* $53 */
	{AM_IMPL, "???"},			/* $54 */
	{AM_ZPX, "EOR"},			/* $55 */
	{AM_ZPX, "LSR"},			/* $56 */
	{AM_ZP, "RMB5"},			/* $57 */
	{AM_IMPL, "CLI"},			/* $58 */
	{AM_ABSY, "EOR"},			/* $59 */
	{AM_IMPL, "PHY"},			/* $5A */
	{AM_IMPL, "BP5"},			/* $5B */
	{AM_IMPL, "???"},			/* $5C */
	{AM_ABSX, "EOR"},			/* $5D */
	{AM_ABSX, "LSR"},			/* $5E */
	{AM_PSREL, "BBR5"},			/* $5F */
	{AM_IMPL, "RTS"},			/* $60 */
	{AM_ZPINDX, "ADC"},			/* $61 */
	{AM_IMPL, "CLA"},			/* $62 */
	{AM_IMPL, "???"},			/* $63 */
	{AM_ZP, "STZ"},				/* $64 */
	{AM_ZP, "ADC"},				/* $65 */
	{AM_ZP, "ROR"},				/* $66 */
	{AM_ZP, "RMB6"},			/* $67 */
	{AM_IMPL, "PLA"},			/* $68 */
	{AM_IMMED, "ADC"},			/* $69 */
	{AM_IMPL, "ROR"},			/* $6A */
	{AM_IMPL, "BP6"},			/* $6B */
	{AM_ABSIND, "JMP"},			/* $6C */
	{AM_ABS, "ADC"},			/* $6D */
	{AM_ABS, "ROR"},			/* $6E */
	{AM_PSREL, "BBR6"},			/* $6F */
	{AM_REL, "BVS"},			/* $70 */
	{AM_ZPINDY, "ADC"},			/* $71 */
	{AM_ZPIND, "ADC"},			/* $72 */
	{AM_XFER, "TII"},			/* $73 */
	{AM_ZPX, "STZ"},			/* $74 */
	{AM_ZPX, "ADC"},			/* $75 */
	{AM_ZPX, "ROR"},			/* $76 */
	{AM_ZP, "RMB7"},			/* $77 */
	{AM_IMPL, "SEI"},			/* $78 */
	{AM_ABSY, "ADC"},			/* $79 */
	{AM_IMPL, "PLY"},			/* $7A */
	{AM_IMPL, "BP7"},			/* $7B */
	{AM_ABSINDX, "JMP"},		/* $7C */
	{AM_ABSX, "ADC"},			/* $7D */
	{AM_ABSX, "ROR"},			/* $7E */
	{AM_PSREL, "BBR7"},			/* $7F */
	{AM_REL, "BRA"},			/* $80 */
	{AM_ZPINDX, "STA"},			/* $81 */
	{AM_IMPL, "CLX"},			/* $82 */
	{AM_TST_ZP, "TST"},			/* $83 */
	{AM_ZP, "STY"},				/* $84 */
	{AM_ZP, "STA"},				/* $85 */
	{AM_ZP, "STX"},				/* $86 */
	{AM_ZP, "SMB0"},			/* $87 */
	{AM_IMPL, "DEY"},			/* $88 */
	{AM_IMMED, "BIT"},			/* $89 */
	{AM_IMPL, "TXA"},			/* $8A */
	{AM_IMPL, "BP8"},			/* $8B */
	{AM_ABS, "STY"},			/* $8C */
	{AM_ABS, "STA"},			/* $8D */
	{AM_ABS, "STX"},			/* $8E */
	{AM_PSREL, "BBS0"},			/* $8F */
	{AM_REL, "BCC"},			/* $90 */
	{AM_ZPINDY, "STA"},			/* $91 */
	{AM_ZPIND, "STA"},			/* $92 */
	{AM_TST_ABS, "TST"},		/* $93 */
	{AM_ZPX, "STY"},			/* $94 */
	{AM_ZPX, "STA"},			/* $95 */
	{AM_ZPY, "STX"},			/* $96 */
	{AM_ZP, "SMB1"},			/* $97 */
	{AM_IMPL, "TYA"},			/* $98 */
	{AM_ABSY, "STA"},			/* $99 */
	{AM_IMPL, "TXS"},			/* $9A */
	{AM_IMPL, "BP9"},			/* $9B */
	{AM_ABS, "STZ"},			/* $9C */
	{AM_ABSX, "STA"},			/* $9D */
	{AM_ABSX, "STZ"},			/* $9E */
	{AM_PSREL, "BBS1"},			/* $9F */
	{AM_IMMED, "LDY"},			/* $A0 */
	{AM_ZPINDX, "LDA"},			/* $A1 */
	{AM_IMMED, "LDX"},			/* $A2 */
	{AM_TST_ZPX, "TST"},		/* $A3 */
	{AM_ZP, "LDY"},				/* $A4 */
	{AM_ZP, "LDA"},				/* $A5 */
	{AM_ZP, "LDX"},				/* $A6 */
	{AM_ZP, "SMB2"},			/* $A7 */
	{AM_IMPL, "TAY"},			/* $A8 */
	{AM_IMMED, "LDA"},			/* $A9 */
	{AM_IMPL, "TAX"},			/* $AA */
	{AM_IMPL, "BPA"},			/* $AB */
	{AM_ABS, "LDY"},			/* $AC */
	{AM_ABS, "LDA"},			/* $AD */
	{AM_ABS, "LDX"},			/* $AE */
	{AM_PSREL, "BBS2"},			/* $AF */
	{AM_REL, "BCS"},			/* $B0 */
	{AM_ZPINDY, "LDA"},			/* $B1 */
	{AM_ZPIND, "LDA"},			/* $B2 */
	{AM_TST_ABSX, "TST"},		/* $B3 */
	{AM_ZPX, "LDY"},			/* $B4 */
	{AM_ZPX, "LDA"},			/* $B5 */
	{AM_ZPY, "LDX"},			/* $B6 */
	{AM_ZP, "SMB3"},			/* $B7 */
	{AM_IMPL, "CLV"},			/* $B8 */
	{AM_ABSY, "LDA"},			/* $B9 */
	{AM_IMPL, "TSX"},			/* $BA */
	{AM_IMPL, "BPB"},			/* $BB */
	{AM_ABSX, "LDY"},			/* $BC */
	{AM_ABSX, "LDA"},			/* $BD */
	{AM_ABSY, "LDX"},			/* $BE */
	{AM_PSREL, "BBS3"},			/* $BF */
	{AM_IMMED, "CPY"},			/* $C0 */
	{AM_ZPINDX, "CMP"},			/* $C1 */
	{AM_IMPL, "CLY"},			/* $C2 */
	{AM_XFER, "TDD"},			/* $C3 */
	{AM_ZP, "CPY"},				/* $C4 */
	{AM_ZP, "CMP"},				/* $C5 */
	{AM_ZP, "DEC"},				/* $C6 */
	{AM_ZP, "SMB4"},			/* $C7 */
	{AM_IMPL, "INY"},			/* $C8 */
	{AM_IMMED, "CMP"},			/* $C9 */
	{AM_IMPL, "DEX"},			/* $CA */
	{AM_IMPL, "BPC"},			/* $CB */
	{AM_ABS, "CPY"},			/* $CC */
	{AM_ABS, "CMP"},			/* $CD */
	{AM_ABS, "DEC"},			/* $CE */
	{AM_PSREL, "BBS4"},			/* $CF */
	{AM_REL, "BNE"},			/* $D0 */
	{AM_ZPINDY, "CMP"},			/* $D1 */
	{AM_ZPIND, "CMP"},			/* $D2 */
	{AM_XFER, "TIN"},			/* $D3 */
	{AM_IMPL, "CSH"},			/* $D4 */
	{AM_ZPX, "CMP"},			/* $D5 */
	{AM_ZPX, "DEC"},			/* $D6 */
	{AM_ZP, "SMB5"},			/* $D7 */
	{AM_IMPL, "CLD"},			/* $D8 */
	{AM_ABSY, "CMP"},			/* $D9 */
	{AM_IMPL, "PHX"},			/* $DA */
	{AM_IMPL, "BPD"},			/* $DB */
	{AM_IMPL, "???"},			/* $DC */
	{AM_ABSX, "CMP"},			/* $DD */
	{AM_ABSX, "DEC"},			/* $DE */
	{AM_PSREL, "BBS5"},			/* $DF */
	{AM_IMMED, "CPX"},			/* $E0 */
	{AM_ZPINDX, "SBC"},			/* $E1 */
	{AM_IMPL, "???"},			/* $E2 */
	{AM_XFER, "TIA"},			/* $E3 */
	{AM_ZP, "CPX"},				/* $E4 */
	{AM_ZP, "SBC"},				/* $E5 */
	{AM_ZP, "INC"},				/* $E6 */
	{AM_ZP, "SMB6"},			/* $E7 */
	{AM_IMPL, "INX"},			/* $E8 */
	{AM_IMMED, "SBC"},			/* $E9 */
	{AM_IMPL, "NOP"},			/* $EA */
	{AM_IMPL, "BPE"},			/* $EB */
	{AM_ABS, "CPX"},			/* $EC */
	{AM_ABS, "SBC"},			/* $ED */
	{AM_ABS, "INC"},			/* $EE */
	{AM_PSREL, "BBS6"},			/* $EF */
	{AM_REL, "BEQ"},			/* $F0 */
	{AM_ZPINDY, "SBC"},			/* $F1 */
	{AM_ZPIND, "SBC"},			/* $F2 */
	{AM_XFER, "TAI"},			/* $F3 */
	{AM_IMPL, "SET"},			/* $F4 */
	{AM_ZPX, "SBC"},			/* $F5 */
	{AM_ZPX, "INC"},			/* $F6 */
	{AM_ZP, "SMB7"},			/* $F7 */
	{AM_IMPL, "SED"},			/* $F8 */
	{AM_ABSY, "SBC"},			/* $F9 */
	{AM_IMPL, "PLX"},			/* $FA */
	{AM_IMPL, "BPF"},			/* $FB */
	{AM_IMPL, "???"},			/* $FC */
	{AM_ABSX, "SBC"},			/* $FD */
	{AM_ABSX, "INC"},			/* $FE */
	{AM_PSREL, "BBS7"}			/* $FF */
};


/* number of bytes per instruction in each addressing mode */

const mode_struct addr_info[MAX_MODES] = {
	{1, &(implicit)},			/* implicit              */
	{2, &(immed)},				/* immediate             */
	{2, &(relative)},			/* relative              */
	{2, &(ind_zp)},				/* zero page             */
	{2, &(ind_zpx)},			/* zero page, x          */
	{2, &(ind_zpy)},			/* zero page, y          */
	{2, &(ind_zpind)},			/* zero page indirect    */
	{2, &(ind_zpix)},			/* zero page indirect, x */
	{2, &(ind_zpiy)},			/* zero page indirect, y */
	{3, &(absol)},				/* absolute              */
	{3, &(absx)},				/* absolute, x           */
	{3, &(absy)},				/* absolute, y           */
	{3, &(absind)},				/* absolute indirect     */
	{3, &(absindx)},			/* absolute indirect     */
	{3, &(pseudorel)},			/* pseudo-relative       */
	{3, &(tst_zp)},				/* special 'TST' addressing mode  */
	{4, &(tst_abs)},			/* special 'TST' addressing mode  */
	{3, &(tst_zpx)},			/* special 'TST' addressing mode  */
	{4, &(tst_absx)},			/* special 'TST' addressing mode  */
	{7, &(xfer)}				/* special 7-byte transfer addressing mode  */
};

#endif
