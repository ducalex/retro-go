/****************************************************************************
 h6280.h
 Function protoypes for simulated execution routines
 ****************************************************************************/

#ifndef H6280_H_
#define H6280_H_

#include "cleantypes.h"

/********************************************/
/* function parameters:                     */
/* --------------------                     */
/* - address (16-bit unsigned),             */
/* - pointer to buffer @ program counter    */
/********************************************/

extern void exe_instruct(void);
extern void exe_go(void);

extern int adc_abs(void);
extern int adc_absx(void);
extern int adc_absy(void);
extern int adc_imm(void);
extern int adc_zp(void);
extern int adc_zpind(void);
extern int adc_zpindx(void);
extern int adc_zpindy(void);
extern int adc_zpx(void);
extern int and_abs(void);
extern int and_absx(void);
extern int and_absy(void);
extern int and_imm(void);
extern int and_zp(void);
extern int and_zpind(void);
extern int and_zpindx(void);
extern int and_zpindy(void);
extern int and_zpx(void);
extern int asl_a(void);
extern int asl_abs(void);
extern int asl_absx(void);
extern int asl_zp(void);
extern int asl_zpx(void);
extern int bbr0(void);
extern int bbr1(void);
extern int bbr2(void);
extern int bbr3(void);
extern int bbr4(void);
extern int bbr5(void);
extern int bbr6(void);
extern int bbr7(void);
extern int bbs0(void);
extern int bbs1(void);
extern int bbs2(void);
extern int bbs3(void);
extern int bbs4(void);
extern int bbs5(void);
extern int bbs6(void);
extern int bbs7(void);
extern int bcc(void);
extern int bcs(void);
extern int beq(void);
extern int bit_abs(void);
extern int bit_absx(void);
extern int bit_imm(void);
extern int bit_zp(void);
extern int bit_zpx(void);
extern int bmi(void);
extern int bne(void);
extern int bpl(void);
extern int bra(void);
extern int brek(void);
extern int bsr(void);
extern int bvc(void);
extern int bvs(void);
extern int cla(void);
extern int clc(void);
extern int cld(void);
extern int cli(void);
extern int clv(void);
extern int clx(void);
extern int cly(void);
extern int cmp_abs(void);
extern int cmp_absx(void);
extern int cmp_absy(void);
extern int cmp_imm(void);
extern int cmp_zp(void);
extern int cmp_zpind(void);
extern int cmp_zpindx(void);
extern int cmp_zpindy(void);
extern int cmp_zpx(void);
extern int cpx_abs(void);
extern int cpx_imm(void);
extern int cpx_zp(void);
extern int cpy_abs(void);
extern int cpy_imm(void);
extern int cpy_zp(void);
extern int dec_a(void);
extern int dec_abs(void);
extern int dec_absx(void);
extern int dec_zp(void);
extern int dec_zpx(void);
extern int dex(void);
extern int dey(void);
extern int eor_abs(void);
extern int eor_absx(void);
extern int eor_absy(void);
extern int eor_imm(void);
extern int eor_zp(void);
extern int eor_zpind(void);
extern int eor_zpindx(void);
extern int eor_zpindy(void);
extern int eor_zpx(void);
extern int halt(void);
extern int inc_a(void);
extern int inc_abs(void);
extern int inc_absx(void);
extern int inc_zp(void);
extern int inc_zpx(void);
extern int inx(void);
extern int iny(void);
extern int jmp(void);
extern int jmp_absind(void);
extern int jmp_absindx(void);
extern int jsr(void);
extern int lda_abs(void);
extern int lda_absx(void);
extern int lda_absy(void);
extern int lda_imm(void);
extern int lda_zp(void);
extern int lda_zpind(void);
extern int lda_zpindx(void);
extern int lda_zpindy(void);
extern int lda_zpx(void);
extern int ldx_abs(void);
extern int ldx_absy(void);
extern int ldx_imm(void);
extern int ldx_zp(void);
extern int ldx_zpy(void);
extern int ldy_abs(void);
extern int ldy_absx(void);
extern int ldy_imm(void);
extern int ldy_zp(void);
extern int ldy_zpx(void);
extern int lsr_a(void);
extern int lsr_abs(void);
extern int lsr_absx(void);
extern int lsr_zp(void);
extern int lsr_zpx(void);
extern int nop(void);
extern int ora_abs(void);
extern int ora_absx(void);
extern int ora_absy(void);
extern int ora_imm(void);
extern int ora_zp(void);
extern int ora_zpind(void);
extern int ora_zpindx(void);
extern int ora_zpindy(void);
extern int ora_zpx(void);
extern int pha(void);
extern int php(void);
extern int phx(void);
extern int phy(void);
extern int pla(void);
extern int plp(void);
extern int plx(void);
extern int ply(void);
extern int rmb0(void);
extern int rmb1(void);
extern int rmb2(void);
extern int rmb3(void);
extern int rmb4(void);
extern int rmb5(void);
extern int rmb6(void);
extern int rmb7(void);
extern int rol_a(void);
extern int rol_abs(void);
extern int rol_absx(void);
extern int rol_zp(void);
extern int rol_zpx(void);
extern int ror_a(void);
extern int ror_abs(void);
extern int ror_absx(void);
extern int ror_zp(void);
extern int ror_zpx(void);
extern int rti(void);
extern int rts(void);
extern int sax(void);
extern int say(void);
extern int sbc_abs(void);
extern int sbc_absx(void);
extern int sbc_absy(void);
extern int sbc_imm(void);
extern int sbc_zp(void);
extern int sbc_zpind(void);
extern int sbc_zpindx(void);
extern int sbc_zpindy(void);
extern int sbc_zpx(void);
extern int sec(void);
extern int sed(void);
extern int sei(void);
extern int set(void);
extern int smb0(void);
extern int smb1(void);
extern int smb2(void);
extern int smb3(void);
extern int smb4(void);
extern int smb5(void);
extern int smb6(void);
extern int smb7(void);
extern int st0(void);
extern int st1(void);
extern int st2(void);
extern int sta_abs(void);
extern int sta_absx(void);
extern int sta_absy(void);
extern int sta_zp(void);
extern int sta_zpind(void);
extern int sta_zpindx(void);
extern int sta_zpindy(void);
extern int sta_zpx(void);
extern int stx_abs(void);
extern int stx_zp(void);
extern int stx_zpy(void);
extern int sty_abs(void);
extern int sty_zp(void);
extern int sty_zpx(void);
extern int stz_abs(void);
extern int stz_absx(void);
extern int stz_zp(void);
extern int stz_zpx(void);
extern int sxy(void);
extern int tai(void);
extern int tam(void);
extern int tax(void);
extern int tay(void);
extern int tdd(void);
extern int tia(void);
extern int tii(void);
extern int tin(void);
extern int tma(void);
extern int trb_abs(void);
extern int trb_zp(void);
extern int tsb_abs(void);
extern int tsb_zp(void);
extern int tstins_abs(void);
extern int tstins_absx(void);
extern int tstins_zp(void);
extern int tstins_zpx(void);
extern int tsx(void);
extern int txa(void);
extern int txs(void);
extern int tya(void);

#define INT_NONE        0		/* No interrupt required      */
#define INT_IRQ         1		/* Standard IRQ interrupt     */
#define INT_NMI         2		/* Non-maskable interrupt     */
#define INT_QUIT        3		/* Exit the emulation         */
#define	INT_TIMER       4
#define	INT_IRQ2        8

#define	VEC_RESET	0xFFFE
#define	VEC_NMI		0xFFFC
#define	VEC_TIMER	0xFFFA
#define	VEC_IRQ		0xFFF8
#define	VEC_IRQ2	0xFFF6
#define	VEC_BRK		0xFFF6

extern uchar flnz_list[256];
//extern uchar *flnz_list;

#define flnz_list_get(num) flnz_list[num]
//#define flnz_list_get(num) (num==0?FL_Z:num>=0x80?FL_N:0)

uchar imm_operand_(uint16 addr);
void put_8bit_zp_(uchar zp_addr, uchar byte);
uchar get_8bit_zp_(uchar zp_addr);
uint16 get_16bit_zp_(uchar zp_addr);
uint16 get_16bit_addr_(uint16 addr);
void put_8bit_addr_(uint16 addr, uchar byte);
uchar get_8bit_addr_(uint16 addr);

#endif
