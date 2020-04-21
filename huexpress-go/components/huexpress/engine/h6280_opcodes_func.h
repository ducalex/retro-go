//
// Implementation of actual opcodes:
//

/*@ -type */

static uchar
adc(uchar acc, uchar val)
{
    int16 sig = (SBYTE) acc;
    uint16 usig = (uchar) acc;
    uint16 temp;

    if (!(reg_p & FL_D)) {      /* binary mode */
        if (reg_p & FL_C) {
            usig++;
            sig++;
        }
        sig += (SBYTE) val;
        usig += (uchar) val;
        acc = (uchar) (usig & 0xFF);

        reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z | FL_C))
            | (((sig > 127) || (sig < -128)) ? FL_V : 0)
            | ((usig > 255) ? FL_C : 0)
            | flnz_list_get(acc);

    } else {                    /* decimal mode */

// treatment of out-of-range accumulator
// and operand values (non-BCD) is not
// adequately defined.  Nor is overflow
// flag treatment.

// Zeo : rewrote using bcdbin and binbcd arrays to boost code speed and fix
// residual bugs

        temp = bcdbin[usig] + bcdbin[val];

        if (reg_p & FL_C) {
            temp++;
        }

        acc = binbcd[temp];

        reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
            | ((temp > 99) ? FL_C : 0)
            | flnz_list_get(acc);

        cycles++;               /* decimal mode takes an extra cycle */

    }
    return (acc);
}

static void
sbc(uchar val)
{
    int16 sig = (SBYTE) reg_a;
    uint16 usig = (uchar) reg_a;
    int16 temp;

    if (!(reg_p & FL_D)) {      /* binary mode */
        if (!(reg_p & FL_C)) {
            usig--;
            sig--;
        }
        sig -= (SBYTE) val;
        usig -= (uchar) val;
        reg_a = (uchar) (usig & 0xFF);
        reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z | FL_C))
            | (((sig > 127) || (sig < -128)) ? FL_V : 0)
            | ((usig > 255) ? 0 : FL_C)
            | flnz_list_get(reg_a); /* FL_N, FL_Z */

    } else {                    /* decimal mode */

// treatment of out-of-range accumulator
// and operand values (non-bcd) is not
// adequately defined.  Nor is overflow
// flag treatment.

        temp = (int16) (bcdbin[usig] - bcdbin[val]);

        if (!(reg_p & FL_C)) {
            temp--;
        }

        reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
            | ((temp < 0) ? 0 : FL_C);

        while (temp < 0) {
            temp += 100;
        }

        reg_a = binbcd[temp];
        chk_flnz_8bit(reg_a);

        cycles++;               /* decimal mode takes an extra cycle */

    }
}

int
adc_abs(void)
{
// if flag 'T' is set, use zero-page address specified by register 'X'
// as the accumulator...

    if (reg_p & FL_T) {
        put_8bit_zp(reg_x,
                    adc(get_8bit_zp(reg_x), abs_operand(reg_pc + 1)));
        cycles += 8;
    } else {
        reg_a = adc(reg_a, abs_operand(reg_pc + 1));
        cycles += 5;
    }
    reg_pc += 3;
    return 0;
}

int
adc_absx(void)
{
    if (reg_p & FL_T) {
        put_8bit_zp(reg_x,
                    adc(get_8bit_zp(reg_x), absx_operand(reg_pc + 1)));
        cycles += 8;
    } else {
        reg_a = adc(reg_a, absx_operand(reg_pc + 1));
        cycles += 5;
    }
    reg_pc += 3;
    return 0;
}

int
adc_absy(void)
{
    if (reg_p & FL_T) {
        put_8bit_zp(reg_x,
                    adc(get_8bit_zp(reg_x), absy_operand(reg_pc + 1)));
        cycles += 8;
    } else {
        reg_a = adc(reg_a, absy_operand(reg_pc + 1));
        cycles += 5;
    }
    reg_pc += 3;
    return 0;
}

int
adc_imm(void)
{
    if (reg_p & FL_T) {
        put_8bit_zp(reg_x,
                    adc(get_8bit_zp(reg_x), imm_operand(reg_pc + 1)));
        cycles += 5;
    } else {
        reg_a = adc(reg_a, imm_operand(reg_pc + 1));
        cycles += 2;
    }
    reg_pc += 2;
    return 0;
}

int
adc_zp(void)
{
    if (reg_p & FL_T) {
        put_8bit_zp(reg_x,
                    adc(get_8bit_zp(reg_x), zp_operand(reg_pc + 1)));
        cycles += 7;
    } else {
        reg_a = adc(reg_a, zp_operand(reg_pc + 1));
        cycles += 4;
    }
    reg_pc += 2;
    return 0;
}

int
adc_zpx(void)
{
    if (reg_p & FL_T) {
        put_8bit_zp(reg_x,
                    adc(get_8bit_zp(reg_x), zpx_operand(reg_pc + 1)));
        cycles += 7;
    } else {
        reg_a = adc(reg_a, zpx_operand(reg_pc + 1));
        cycles += 4;
    }
    reg_pc += 2;
    return 0;
}

int
adc_zpind(void)
{
    if (reg_p & FL_T) {
        put_8bit_zp(reg_x,
                    adc(get_8bit_zp(reg_x), zpind_operand(reg_pc + 1)));
        cycles += 10;
    } else {
        reg_a = adc(reg_a, zpind_operand(reg_pc + 1));
        cycles += 7;
    }
    reg_pc += 2;
    return 0;
}

int
adc_zpindx(void)
{
    if (reg_p & FL_T) {
        put_8bit_zp(reg_x,
                    adc(get_8bit_zp(reg_x), zpindx_operand(reg_pc + 1)));
        cycles += 10;
    } else {
        reg_a = adc(reg_a, zpindx_operand(reg_pc + 1));
        cycles += 7;
    }
    reg_pc += 2;
    return 0;
}

int
adc_zpindy(void)
{
    if (reg_p & FL_T) {
        put_8bit_zp(reg_x,
                    adc(get_8bit_zp(reg_x), zpindy_operand(reg_pc + 1)));
        cycles += 10;
    } else {
        reg_a = adc(reg_a, zpindy_operand(reg_pc + 1));
        cycles += 7;
    }
    reg_pc += 2;
    return 0;
}

int
and_abs(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp &= abs_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 8;

    } else {
        reg_a &= abs_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 5;
    }
    reg_pc += 3;
    return 0;
}

int
and_absx(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp &= absx_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 8;

    } else {
        reg_a &= absx_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 5;
    }
    reg_pc += 3;
    return 0;
}

int
and_absy(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp &= absy_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 8;

    } else {
        reg_a &= absy_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 5;
    }
    reg_pc += 3;
    return 0;
}

int
and_imm(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp &= imm_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 5;

    } else {
        reg_a &= imm_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 2;
    }
    reg_pc += 2;
    return 0;
}

int
and_zp(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp &= zp_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 7;

    } else {
        reg_a &= zp_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 4;
    }
    reg_pc += 2;
    return 0;
}

int
and_zpx(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp &= zpx_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 7;

    } else {
        reg_a &= zpx_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 4;
    }
    reg_pc += 2;
    return 0;
}

int
and_zpind(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp &= zpind_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 10;

    } else {
        reg_a &= zpind_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 7;
    }
    reg_pc += 2;
    return 0;
}

int
and_zpindx(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp &= zpindx_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 10;

    } else {
        reg_a &= zpindx_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 7;
    }
    reg_pc += 2;
    return 0;
}

int
and_zpindy(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp &= zpindy_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 10;

    } else {
        reg_a &= zpindy_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 7;
    }
    reg_pc += 2;
    return 0;
}

int
asl_a(void)
{
    uchar temp1 = reg_a;
    reg_a <<= 1;
    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((temp1 & 0x80) ? FL_C : 0)
        | flnz_list_get(reg_a);
    cycles += 2;
    reg_pc++;
    return 0;
}

int
asl_abs(void)
{
    uint16 temp_addr = get_16bit_addr(reg_pc + 1);
    uchar temp1 = get_8bit_addr(temp_addr);
    uchar temp = temp1 << 1;

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((temp1 & 0x80) ? FL_C : 0)
        | flnz_list_get(temp);
    cycles += 7;

    put_8bit_addr(temp_addr, temp);
    reg_pc += 3;
    return 0;
}

int
asl_absx(void)
{
    uint16 temp_addr = get_16bit_addr(reg_pc + 1) + reg_x;
    uchar temp1 = get_8bit_addr(temp_addr);
    uchar temp = temp1 << 1;

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((temp1 & 0x80) ? FL_C : 0)
        | flnz_list_get(temp);
    cycles += 7;
    put_8bit_addr(temp_addr, temp);
    reg_pc += 3;
    return 0;
}

int
asl_zp(void)
{
    uchar zp_addr = imm_operand(reg_pc + 1);
    uchar temp1 = get_8bit_zp(zp_addr);
    uchar temp = temp1 << 1;

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((temp1 & 0x80) ? FL_C : 0)
        | flnz_list_get(temp);
    cycles += 6;
    put_8bit_zp(zp_addr, temp);
    reg_pc += 2;
    return 0;
}

int
asl_zpx(void)
{
    uchar zp_addr = imm_operand(reg_pc + 1) + reg_x;
    uchar temp1 = get_8bit_zp(zp_addr);
    uchar temp = temp1 << 1;

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((temp1 & 0x80) ? FL_C : 0)
        | flnz_list_get(temp);
    cycles += 6;
    put_8bit_zp(zp_addr, temp);
    reg_pc += 2;
    return 0;
}

int
bbr0(void)
{
    reg_p &= ~FL_T;
    if (zp_operand(reg_pc + 1) & 0x01) {
        reg_pc += 3;
        cycles += 6;
    } else {
        reg_pc += (SBYTE) imm_operand(reg_pc + 2) + 3;
        cycles += 8;
    }
    return 0;
}

int
bbr1(void)
{
    reg_p &= ~FL_T;
    if (zp_operand(reg_pc + 1) & 0x02) {
        reg_pc += 3;
        cycles += 6;
    } else {
        reg_pc += (SBYTE) imm_operand(reg_pc + 2) + 3;
        cycles += 8;
    }
    return 0;
}

int
bbr2(void)
{
    reg_p &= ~FL_T;
    if (zp_operand(reg_pc + 1) & 0x04) {
        reg_pc += 3;
        cycles += 6;
    } else {
        reg_pc += (SBYTE) imm_operand(reg_pc + 2) + 3;
        cycles += 8;
    }
    return 0;
}

int
bbr3(void)
{
    reg_p &= ~FL_T;
    if (zp_operand(reg_pc + 1) & 0x08) {
        reg_pc += 3;
        cycles += 6;
    } else {
        reg_pc += (SBYTE) imm_operand(reg_pc + 2) + 3;
        cycles += 8;
    }
    return 0;
}

int
bbr4(void)
{
    reg_p &= ~FL_T;
    if (zp_operand(reg_pc + 1) & 0x10) {
        reg_pc += 3;
        cycles += 6;
    } else {
        reg_pc += (SBYTE) imm_operand(reg_pc + 2) + 3;
        cycles += 8;
    }
    return 0;
}

int
bbr5(void)
{
    reg_p &= ~FL_T;
    if (zp_operand(reg_pc + 1) & 0x20) {
        reg_pc += 3;
        cycles += 6;
    } else {
        reg_pc += (SBYTE) imm_operand(reg_pc + 2) + 3;
        cycles += 8;
    }
    return 0;
}

int
bbr6(void)
{
    reg_p &= ~FL_T;
    if (zp_operand(reg_pc + 1) & 0x40) {
        reg_pc += 3;
        cycles += 6;
    } else {
        reg_pc += (SBYTE) imm_operand(reg_pc + 2) + 3;
        cycles += 8;
    }
    return 0;
}

int
bbr7(void)
{
    reg_p &= ~FL_T;
    if (zp_operand(reg_pc + 1) & 0x80) {
        reg_pc += 3;
        cycles += 6;
    } else {
        reg_pc += (SBYTE) imm_operand(reg_pc + 2) + 3;
        cycles += 8;
    }
    return 0;
}

int
bbs0(void)
{
    reg_p &= ~FL_T;
    if (zp_operand(reg_pc + 1) & 0x01) {
        reg_pc += (SBYTE) imm_operand(reg_pc + 2) + 3;
        cycles += 8;
    } else {
        reg_pc += 3;
        cycles += 6;
    }
    return 0;
}

int
bbs1(void)
{
    reg_p &= ~FL_T;
    if (zp_operand(reg_pc + 1) & 0x02) {
        reg_pc += (SBYTE) imm_operand(reg_pc + 2) + 3;
        cycles += 8;
    } else {
        reg_pc += 3;
        cycles += 6;
    }
    return 0;
}

int
bbs2(void)
{
    reg_p &= ~FL_T;
    if (zp_operand(reg_pc + 1) & 0x04) {
        reg_pc += (SBYTE) imm_operand(reg_pc + 2) + 3;
        cycles += 8;
    } else {
        reg_pc += 3;
        cycles += 6;
    }
    return 0;
}

int
bbs3(void)
{
    reg_p &= ~FL_T;
    if (zp_operand(reg_pc + 1) & 0x08) {
        reg_pc += (SBYTE) imm_operand(reg_pc + 2) + 3;
        cycles += 8;
    } else {
        reg_pc += 3;
        cycles += 6;
    }
    return 0;
}

int
bbs4(void)
{
    reg_p &= ~FL_T;
    if (zp_operand(reg_pc + 1) & 0x10) {
        reg_pc += (SBYTE) imm_operand(reg_pc + 2) + 3;
        cycles += 8;
    } else {
        reg_pc += 3;
        cycles += 6;
    }
    return 0;
}

int
bbs5(void)
{
    reg_p &= ~FL_T;
    if (zp_operand(reg_pc + 1) & 0x20) {
        reg_pc += (SBYTE) imm_operand(reg_pc + 2) + 3;
        cycles += 8;
    } else {
        reg_pc += 3;
        cycles += 6;
    }
    return 0;
}

int
bbs6(void)
{
    reg_p &= ~FL_T;
    if (zp_operand(reg_pc + 1) & 0x40) {
        reg_pc += (SBYTE) imm_operand(reg_pc + 2) + 3;
        cycles += 8;
    } else {
        reg_pc += 3;
        cycles += 6;
    }
    return 0;
}

int
bbs7(void)
{
    reg_p &= ~FL_T;
    if (zp_operand(reg_pc + 1) & 0x80) {
        reg_pc += (SBYTE) imm_operand(reg_pc + 2) + 3;
        cycles += 8;
    } else {
        reg_pc += 3;
        cycles += 6;
    }
    return 0;
}

int
bcc(void)
{
    reg_p &= ~FL_T;
    if (reg_p & FL_C) {
        reg_pc += 2;
        cycles += 2;
    } else {
        reg_pc += (SBYTE) imm_operand(reg_pc + 1) + 2;
        cycles += 4;
    }
    return 0;
}

int
bcs(void)
{
    reg_p &= ~FL_T;
    if (reg_p & FL_C) {
        reg_pc += (SBYTE) imm_operand(reg_pc + 1) + 2;
        cycles += 4;
    } else {
        reg_pc += 2;
        cycles += 2;
    }
    return 0;
}

int
beq(void)
{
    reg_p &= ~FL_T;
    if (reg_p & FL_Z) {
        reg_pc += (SBYTE) imm_operand(reg_pc + 1) + 2;
        cycles += 4;
    } else {
        reg_pc += 2;
        cycles += 2;
    }
    return 0;
}

int
bit_abs(void)
{
    uchar temp = abs_operand(reg_pc + 1);
    reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z))
        | ((temp & 0x80) ? FL_N : 0)
        | ((temp & 0x40) ? FL_V : 0)
        | ((reg_a & temp) ? 0 : FL_Z);
    reg_pc += 3;
    cycles += 5;
    return 0;
}

int
bit_absx(void)
{
    uchar temp = absx_operand(reg_pc + 1);
    reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z))
        | ((temp & 0x80) ? FL_N : 0)
        | ((temp & 0x40) ? FL_V : 0)
        | ((reg_a & temp) ? 0 : FL_Z);
    reg_pc += 3;
    cycles += 5;
    return 0;
}

int
bit_imm(void)
{
// orig code (Eyes/Lichty said immediate mode did not affect
//            'N' and 'V' flags):
//reg_p = (reg_p & ~(FL_T|FL_Z))
//    | ((reg_a & imm_operand(reg_pc+1)) ? 0:FL_Z);

    uchar temp = imm_operand(reg_pc + 1);
    reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z))
        | ((temp & 0x80) ? FL_N : 0)
        | ((temp & 0x40) ? FL_V : 0)
        | ((reg_a & temp) ? 0 : FL_Z);
    reg_pc += 2;
    cycles += 2;
    return 0;
}

int
bit_zp(void)
{
    uchar temp = zp_operand(reg_pc + 1);
    reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z))
        | ((temp & 0x80) ? FL_N : 0)
        | ((temp & 0x40) ? FL_V : 0)
        | ((reg_a & temp) ? 0 : FL_Z);
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
bit_zpx(void)
{
    uchar temp = zpx_operand(reg_pc + 1);
    reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z))
        | ((temp & 0x80) ? FL_N : 0)
        | ((temp & 0x40) ? FL_V : 0)
        | ((reg_a & temp) ? 0 : FL_Z);
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
bmi(void)
{
    reg_p &= ~FL_T;
    if (reg_p & FL_N) {
        reg_pc += (SBYTE) imm_operand(reg_pc + 1) + 2;
        cycles += 4;
    } else {
        reg_pc += 2;
        cycles += 2;
    }
    return 0;
}

int
bne(void)
{
    reg_p &= ~FL_T;
    if (reg_p & FL_Z) {
        reg_pc += 2;
        cycles += 2;
    } else {
        reg_pc += (SBYTE) imm_operand(reg_pc + 1) + 2;
        cycles += 4;
    }
    return 0;
}

int
bpl(void)
{
    reg_p &= ~FL_T;
    if (reg_p & FL_N) {
        reg_pc += 2;
        cycles += 2;
    } else {
        reg_pc += (SBYTE) imm_operand(reg_pc + 1) + 2;
        cycles += 4;
    }
    return 0;
}

int
bra(void)
{
    reg_p &= ~FL_T;
    reg_pc += (SBYTE) imm_operand(reg_pc + 1) + 2;
    cycles += 4;
    return 0;
}

int
brek(void)
{
#if defined(KERNEL_DEBUG)
    fprintf(stderr, "BRK opcode has been hit [PC = 0x%04x] at %s(%d)\n",
            reg_pc, __FILE__, __LINE__);
#endif
    push_16bit(reg_pc + 2);
    reg_p &= ~FL_T;
    push_8bit(reg_p | FL_B);
    reg_p = (reg_p & ~FL_D) | FL_I;
    reg_pc = get_16bit_addr(0xFFF6);
    cycles += 8;
    return 0;
}

int
bsr(void)
{
    reg_p &= ~FL_T;
    push_16bit(reg_pc + 1);
    reg_pc += (SBYTE) imm_operand(reg_pc + 1) + 2;
    cycles += 8;
    return 0;
}

int
bvc(void)
{
    reg_p &= ~FL_T;
    if (reg_p & FL_V) {
        reg_pc += 2;
        cycles += 2;
    } else {
        reg_pc += (SBYTE) imm_operand(reg_pc + 1) + 2;
        cycles += 4;
    }
    return 0;
}

int
bvs(void)
{
    reg_p &= ~FL_T;
    if (reg_p & FL_V) {
        reg_pc += (SBYTE) imm_operand(reg_pc + 1) + 2;
        cycles += 4;
    } else {
        reg_pc += 2;
        cycles += 2;
    }
    return 0;
}

int
cla(void)
{
    reg_p &= ~FL_T;
    reg_a = 0;
    reg_pc++;
    cycles += 2;
    return 0;
}

int
clc(void)
{
    reg_p &= ~(FL_T | FL_C);
    reg_pc++;
    cycles += 2;
    return 0;
}

int
cld(void)
{
    reg_p &= ~(FL_T | FL_D);
    reg_pc++;
    cycles += 2;
    return 0;
}

int
cli(void)
{
    reg_p &= ~(FL_T | FL_I);
    reg_pc++;
    cycles += 2;
    return 0;
}

int
clv(void)
{
    reg_p &= ~(FL_V | FL_T);
    reg_pc++;
    cycles += 2;
    return 0;
}

int
clx(void)
{
    reg_p &= ~FL_T;
    reg_x = 0;
    reg_pc++;
    cycles += 2;
    return 0;
}

int
cly(void)
{
    reg_p &= ~FL_T;
    reg_y = 0;
    reg_pc++;
    cycles += 2;
    return 0;
}

int
cmp_abs(void)
{
    uchar temp = abs_operand(reg_pc + 1);

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((reg_a < temp) ? 0 : FL_C)
        | flnz_list[(uchar) (reg_a - temp)];
    reg_pc += 3;
    cycles += 5;
    return 0;
}

int
cmp_absx(void)
{
    uchar temp = absx_operand(reg_pc + 1);

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((reg_a < temp) ? 0 : FL_C)
        | flnz_list[(uchar) (reg_a - temp)];
    reg_pc += 3;
    cycles += 5;
    return 0;
}

int
cmp_absy(void)
{
    uchar temp = absy_operand(reg_pc + 1);

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((reg_a < temp) ? 0 : FL_C)
        | flnz_list[(uchar) (reg_a - temp)];
    reg_pc += 3;
    cycles += 5;
    return 0;
}

int
cmp_imm(void)
{
    uchar temp = imm_operand(reg_pc + 1);

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((reg_a < temp) ? 0 : FL_C)
        | flnz_list[(uchar) (reg_a - temp)];
    reg_pc += 2;
    cycles += 2;
    return 0;
}

int
cmp_zp(void)
{
    uchar temp = zp_operand(reg_pc + 1);

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((reg_a < temp) ? 0 : FL_C)
        | flnz_list[(uchar) (reg_a - temp)];
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
cmp_zpx(void)
{
    uchar temp = zpx_operand(reg_pc + 1);

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((reg_a < temp) ? 0 : FL_C)
        | flnz_list[(uchar) (reg_a - temp)];
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
cmp_zpind(void)
{
    uchar temp = zpind_operand(reg_pc + 1);

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((reg_a < temp) ? 0 : FL_C)
        | flnz_list[(uchar) (reg_a - temp)];
    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
cmp_zpindx(void)
{
    uchar temp = zpindx_operand(reg_pc + 1);

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((reg_a < temp) ? 0 : FL_C)
        | flnz_list[(uchar) (reg_a - temp)];
    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
cmp_zpindy(void)
{
    uchar temp = zpindy_operand(reg_pc + 1);

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((reg_a < temp) ? 0 : FL_C)
        | flnz_list[(uchar) (reg_a - temp)];
    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
cpx_abs(void)
{
    uchar temp = abs_operand(reg_pc + 1);

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((reg_x < temp) ? 0 : FL_C)
        | flnz_list[(uchar) (reg_x - temp)];
    reg_pc += 3;
    cycles += 5;
    return 0;
}

int
cpx_imm(void)
{
    uchar temp = imm_operand(reg_pc + 1);

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((reg_x < temp) ? 0 : FL_C)
        | flnz_list[(uchar) (reg_x - temp)];
    reg_pc += 2;
    cycles += 2;
    return 0;
}

int
cpx_zp(void)
{
    uchar temp = zp_operand(reg_pc + 1);

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((reg_x < temp) ? 0 : FL_C)
        | flnz_list[(uchar) (reg_x - temp)];
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
cpy_abs(void)
{
    uchar temp = abs_operand(reg_pc + 1);

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((reg_y < temp) ? 0 : FL_C)
        | flnz_list[(uchar) (reg_y - temp)];
    reg_pc += 3;
    cycles += 5;
    return 0;
}

int
cpy_imm(void)
{
    uchar temp = imm_operand(reg_pc + 1);

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((reg_y < temp) ? 0 : FL_C)
        | flnz_list[(uchar) (reg_y - temp)];
    reg_pc += 2;
    cycles += 2;
    return 0;
}

int
cpy_zp(void)
{
    uchar temp = zp_operand(reg_pc + 1);

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((reg_y < temp) ? 0 : FL_C)
        | flnz_list[(uchar) (reg_y - temp)];
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
dec_a(void)
{
    --reg_a;
    chk_flnz_8bit(reg_a);
    reg_pc++;
    cycles += 2;
    return 0;
}

int
dec_abs(void)
{
    uchar temp;
    uint16 temp_addr = get_16bit_addr(reg_pc + 1);
    temp = get_8bit_addr(temp_addr) - 1;
    chk_flnz_8bit(temp);
    cycles += 7;
    put_8bit_addr(temp_addr, temp);
    reg_pc += 3;
    return 0;
}

int
dec_absx(void)
{
    uchar temp;
    uint16 temp_addr = get_16bit_addr(reg_pc + 1) + reg_x;
    temp = get_8bit_addr(temp_addr) - 1;
    chk_flnz_8bit(temp);
    cycles += 7;
    put_8bit_addr(temp_addr, temp);
    reg_pc += 3;
    return 0;
}

int
dec_zp(void)
{
    uchar temp;
    uchar zp_addr = imm_operand(reg_pc + 1);
    temp = get_8bit_zp(zp_addr) - 1;
    chk_flnz_8bit(temp);
    put_8bit_zp(zp_addr, temp);
    reg_pc += 2;
    cycles += 6;
    return 0;
}

int
dec_zpx(void)
{
    uchar temp;
    uchar zp_addr = imm_operand(reg_pc + 1) + reg_x;
    temp = get_8bit_zp(zp_addr) - 1;
    chk_flnz_8bit(temp);
    put_8bit_zp(zp_addr, temp);
    reg_pc += 2;
    cycles += 6;
    return 0;
}

int
dex(void)
{
    --reg_x;
    chk_flnz_8bit(reg_x);
    reg_pc++;
    cycles += 2;
    return 0;
}

int
dey(void)
{
    --reg_y;
    chk_flnz_8bit(reg_y);
    reg_pc++;
    cycles += 2;
    return 0;
}

int
eor_abs(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp ^= abs_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 8;

    } else {
        reg_a ^= abs_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 5;
    }
    reg_pc += 3;
    return 0;
}

int
eor_absx(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp ^= absx_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 8;

    } else {
        reg_a ^= absx_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 5;
    }
    reg_pc += 3;
    return 0;
}

int
eor_absy(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp ^= absy_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 8;

    } else {
        reg_a ^= absy_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 5;
    }
    reg_pc += 3;
    return 0;
}

int
eor_imm(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp ^= imm_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 5;

    } else {
        reg_a ^= imm_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 2;
    }
    reg_pc += 2;
    return 0;
}

int
eor_zp(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp ^= zp_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 7;

    } else {
        reg_a ^= zp_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 4;
    }
    reg_pc += 2;
    return 0;
}

int
eor_zpx(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp ^= zpx_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 7;

    } else {
        reg_a ^= zpx_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 4;
    }
    reg_pc += 2;
    return 0;
}

int
eor_zpind(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp ^= zpind_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 10;

    } else {
        reg_a ^= zpind_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 7;
    }
    reg_pc += 2;
    return 0;
}

int
eor_zpindx(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp ^= zpindx_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 10;

    } else {
        reg_a ^= zpindx_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 7;
    }
    reg_pc += 2;
    return 0;
}

int
eor_zpindy(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp ^= zpindy_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 10;

    } else {
        reg_a ^= zpindy_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 7;
    }
    reg_pc += 2;
    return 0;
}

int
halt(void)
{
    return (1);
}

int
inc_a(void)
{
    ++reg_a;
    chk_flnz_8bit(reg_a);
    reg_pc++;
    cycles += 2;
    return 0;
}

int
inc_abs(void)
{
    uchar temp;
    uint16 temp_addr = get_16bit_addr(reg_pc + 1);
    temp = get_8bit_addr(temp_addr) + 1;
    chk_flnz_8bit(temp);
    cycles += 7;
    put_8bit_addr(temp_addr, temp);
    reg_pc += 3;
    return 0;
}

int
inc_absx(void)
{
    uchar temp;
    uint16 temp_addr = get_16bit_addr(reg_pc + 1) + reg_x;
    temp = get_8bit_addr(temp_addr) + 1;
    chk_flnz_8bit(temp);
    cycles += 7;
    put_8bit_addr(temp_addr, temp);
    reg_pc += 3;
    return 0;
}

int
inc_zp(void)
{
    uchar temp;
    uchar zp_addr = imm_operand(reg_pc + 1);
    temp = get_8bit_zp(zp_addr) + 1;
    chk_flnz_8bit(temp);
    put_8bit_zp(zp_addr, temp);

    reg_pc += 2;
    cycles += 6;
    return 0;
}

int
inc_zpx(void)
{
    uchar temp;
    uchar zp_addr = imm_operand(reg_pc + 1) + reg_x;
    temp = get_8bit_zp(zp_addr) + 1;
    chk_flnz_8bit(temp);
    put_8bit_zp(zp_addr, temp);
    reg_pc += 2;
    cycles += 6;
    return 0;
}

int
inx(void)
{
    ++reg_x;
    chk_flnz_8bit(reg_x);
    reg_pc++;
    cycles += 2;
    return 0;
}

int
iny(void)
{
    ++reg_y;
    chk_flnz_8bit(reg_y);
    reg_pc++;
    cycles += 2;
    return 0;
}

int
jmp(void)
{
    reg_p &= ~FL_T;
    reg_pc = get_16bit_addr(reg_pc + 1);
    cycles += 4;
    return 0;
}

int
jmp_absind(void)
{
    reg_p &= ~FL_T;
    reg_pc = get_16bit_addr(get_16bit_addr(reg_pc + 1));
    cycles += 7;
    return 0;
}

int
jmp_absindx(void)
{
    reg_p &= ~FL_T;
    reg_pc = get_16bit_addr(get_16bit_addr(reg_pc + 1) + reg_x);
    cycles += 7;
    return 0;
}

int
jsr(void)
{
    reg_p &= ~FL_T;
    push_16bit(reg_pc + 2);
    reg_pc = get_16bit_addr(reg_pc + 1);
    cycles += 7;
    return 0;
}

int
lda_abs(void)
{
    reg_a = abs_operand(reg_pc + 1);
    chk_flnz_8bit(reg_a);
    reg_pc += 3;
    cycles += 5;
    return 0;
}

int
lda_absx(void)
{
    reg_a = absx_operand(reg_pc + 1);
    chk_flnz_8bit(reg_a);
    reg_pc += 3;
    cycles += 5;
    return 0;
}

int
lda_absy(void)
{
    reg_a = absy_operand(reg_pc + 1);
    chk_flnz_8bit(reg_a);
    reg_pc += 3;
    cycles += 5;
    return 0;
}

int
lda_imm(void)
{
    reg_a = imm_operand(reg_pc + 1);
    chk_flnz_8bit(reg_a);
    reg_pc += 2;
    cycles += 2;
    return 0;
}

int
lda_zp(void)
{
    reg_a = zp_operand(reg_pc + 1);
    chk_flnz_8bit(reg_a);
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
lda_zpx(void)
{
    reg_a = zpx_operand(reg_pc + 1);
    chk_flnz_8bit(reg_a);
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
lda_zpind(void)
{
    reg_a = zpind_operand(reg_pc + 1);
    chk_flnz_8bit(reg_a);

    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
lda_zpindx(void)
{
    reg_a = zpindx_operand(reg_pc + 1);
    chk_flnz_8bit(reg_a);
    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
lda_zpindy(void)
{
    reg_a = zpindy_operand(reg_pc + 1);
    chk_flnz_8bit(reg_a);
    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
ldx_abs(void)
{
    reg_x = abs_operand(reg_pc + 1);
    chk_flnz_8bit(reg_x);
    reg_pc += 3;
    cycles += 5;
    return 0;
}

int
ldx_absy(void)
{
    reg_x = absy_operand(reg_pc + 1);
    chk_flnz_8bit(reg_x);
    reg_pc += 3;
    cycles += 5;
    return 0;
}

int
ldx_imm(void)
{
    reg_x = imm_operand(reg_pc + 1);
    chk_flnz_8bit(reg_x);
    reg_pc += 2;
    cycles += 2;
    return 0;
}

int
ldx_zp(void)
{
    reg_x = zp_operand(reg_pc + 1);
    chk_flnz_8bit(reg_x);
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
ldx_zpy(void)
{
    reg_x = zpy_operand(reg_pc + 1);
    chk_flnz_8bit(reg_x);
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
ldy_abs(void)
{
    reg_y = abs_operand(reg_pc + 1);
    chk_flnz_8bit(reg_y);
    reg_pc += 3;
    cycles += 5;
    return 0;
}

int
ldy_absx(void)
{
    reg_y = absx_operand(reg_pc + 1);
    chk_flnz_8bit(reg_y);
    reg_pc += 3;
    cycles += 5;
    return 0;
}

int
ldy_imm(void)
{
    reg_y = imm_operand(reg_pc + 1);
    chk_flnz_8bit(reg_y);
    reg_pc += 2;
    cycles += 2;
    return 0;
}

int
ldy_zp(void)
{
    reg_y = zp_operand(reg_pc + 1);
    chk_flnz_8bit(reg_y);
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
ldy_zpx(void)
{
    reg_y = zpx_operand(reg_pc + 1);
    chk_flnz_8bit(reg_y);
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
lsr_a(void)
{
    uchar temp = reg_a;
    reg_a /= 2;
    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((temp & 1) ? FL_C : 0)
        | flnz_list_get(reg_a);
    reg_pc++;
    cycles += 2;
    return 0;
}

int
lsr_abs(void)
{
    uint16 temp_addr = get_16bit_addr(reg_pc + 1);
    uchar temp1 = get_8bit_addr(temp_addr);
    uchar temp = temp1 / 2;

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((temp1 & 1) ? FL_C : 0)
        | flnz_list_get(temp);
    cycles += 7;
    put_8bit_addr(temp_addr, temp);
    reg_pc += 3;
    return 0;
}

int
lsr_absx(void)
{
    uint16 temp_addr = get_16bit_addr(reg_pc + 1) + reg_x;
    uchar temp1 = get_8bit_addr(temp_addr);
    uchar temp = temp1 / 2;

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((temp1 & 1) ? FL_C : 0)
        | flnz_list_get(temp);
    cycles += 7;
    put_8bit_addr(temp_addr, temp);
    reg_pc += 3;
    return 0;
}

int
lsr_zp(void)
{
    uchar zp_addr = imm_operand(reg_pc + 1);
    uchar temp1 = get_8bit_zp(zp_addr);
    uchar temp = temp1 / 2;

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((temp1 & 1) ? FL_C : 0)
        | flnz_list_get(temp);
    put_8bit_zp(zp_addr, temp);
    reg_pc += 2;
    cycles += 6;
    return 0;
}

int
lsr_zpx(void)
{
    uchar zp_addr = imm_operand(reg_pc + 1) + reg_x;
    uchar temp1 = get_8bit_zp(zp_addr);
    uchar temp = temp1 / 2;

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((temp1 & 1) ? FL_C : 0)
        | flnz_list_get(temp);
    put_8bit_zp(zp_addr, temp);
    reg_pc += 2;
    cycles += 6;
    return 0;
}

int
nop(void)
{
    reg_p &= ~FL_T;
    reg_pc++;
    cycles += 2;
    return 0;
}

int
ora_abs(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp |= abs_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 8;

    } else {
        reg_a |= abs_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 5;
    }
    reg_pc += 3;
    return 0;
}

int
ora_absx(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp |= absx_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 8;

    } else {
        reg_a |= absx_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 5;
    }
    reg_pc += 3;
    return 0;
}

int
ora_absy(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp |= absy_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 8;

    } else {
        reg_a |= absy_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 5;
    }
    reg_pc += 3;
    return 0;
}

int
ora_imm(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp |= imm_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 5;

    } else {
        reg_a |= imm_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 2;
    }
    reg_pc += 2;
    return 0;
}

int
ora_zp(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp |= zp_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 7;

    } else {
        reg_a |= zp_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 4;
    }
    reg_pc += 2;
    return 0;
}

int
ora_zpx(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp |= zpx_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 7;

    } else {
        reg_a |= zpx_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 4;
    }
    reg_pc += 2;
    return 0;
}

int
ora_zpind(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp |= zpind_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 10;

    } else {
        reg_a |= zpind_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 7;
    }
    reg_pc += 2;
    return 0;
}

int
ora_zpindx(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp |= zpindx_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 10;

    } else {
        reg_a |= zpindx_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 7;
    }
    reg_pc += 2;
    return 0;
}

int
ora_zpindy(void)
{
    if (reg_p & FL_T) {
        uchar temp = get_8bit_zp(reg_x);
        temp |= zpindy_operand(reg_pc + 1);
        chk_flnz_8bit(temp);
        put_8bit_zp(reg_x, temp);
        cycles += 10;

    } else {
        reg_a |= zpindy_operand(reg_pc + 1);
        chk_flnz_8bit(reg_a);
        cycles += 7;
    }
    reg_pc += 2;
    return 0;
}

int
pha(void)
{
    reg_p &= ~FL_T;
    push_8bit(reg_a);
    reg_pc++;
    cycles += 3;
    return 0;
}

int
php(void)
{
    reg_p &= ~FL_T;
    push_8bit(reg_p);
    reg_pc++;
    cycles += 3;
    return 0;
}

int
phx(void)
{
    reg_p &= ~FL_T;
    push_8bit(reg_x);
    reg_pc++;
    cycles += 3;
    return 0;
}

int
phy(void)
{
    reg_p &= ~FL_T;
    push_8bit(reg_y);
    reg_pc++;
    cycles += 3;
    return 0;
}

int
pla(void)
{
    reg_a = pull_8bit();
    chk_flnz_8bit(reg_a);
    reg_pc++;
    cycles += 4;
    return 0;
}

int
plp(void)
{
    reg_p = pull_8bit();
    reg_pc++;
    cycles += 4;
    return 0;
}

int
plx(void)
{
    reg_x = pull_8bit();
    chk_flnz_8bit(reg_x);
    reg_pc++;
    cycles += 4;
    return 0;
}

int
ply(void)
{
    reg_y = pull_8bit();
    chk_flnz_8bit(reg_y);
    reg_pc++;
    cycles += 4;
    return 0;
}

int
rmb0(void)
{
    uchar temp = imm_operand(reg_pc + 1);
    reg_p &= ~FL_T;
    put_8bit_zp(temp, get_8bit_zp(temp) & (~0x01));
    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
rmb1(void)
{
    uchar temp = imm_operand(reg_pc + 1);
    reg_p &= ~FL_T;
    put_8bit_zp(temp, get_8bit_zp(temp) & (~0x02));
    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
rmb2(void)
{
    uchar temp = imm_operand(reg_pc + 1);
    reg_p &= ~FL_T;
    put_8bit_zp(temp, get_8bit_zp(temp) & (~0x04));
    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
rmb3(void)
{
    uchar temp = imm_operand(reg_pc + 1);
    reg_p &= ~FL_T;
    put_8bit_zp(temp, get_8bit_zp(temp) & (~0x08));
    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
rmb4(void)
{
    uchar temp = imm_operand(reg_pc + 1);
    reg_p &= ~FL_T;
    put_8bit_zp(temp, get_8bit_zp(temp) & (~0x10));
    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
rmb5(void)
{
    uchar temp = imm_operand(reg_pc + 1);
    reg_p &= ~FL_T;
    put_8bit_zp(temp, get_8bit_zp(temp) & (~0x20));
    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
rmb6(void)
{
    uchar temp = imm_operand(reg_pc + 1);
    reg_p &= ~FL_T;
    put_8bit_zp(temp, get_8bit_zp(temp) & (~0x40));
    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
rmb7(void)
{
    uchar temp = imm_operand(reg_pc + 1);
    reg_p &= ~FL_T;
    put_8bit_zp(temp, get_8bit_zp(temp) & (~0x80));
    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
rol_a(void)
{
    uchar flg_tmp = (reg_p & FL_C) ? 1 : 0;
    uchar temp = reg_a;

    reg_a = (reg_a << 1) + flg_tmp;
    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((temp & 0x80) ? FL_C : 0)
        | flnz_list_get(reg_a);
    reg_pc++;
    cycles += 2;
    return 0;
}

int
rol_abs(void)
{
    uchar flg_tmp = (reg_p & FL_C) ? 1 : 0;
    uint16 temp_addr = get_16bit_addr(reg_pc + 1);
    uchar temp1 = get_8bit_addr(temp_addr);
    uchar temp = (temp1 << 1) + flg_tmp;

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((temp1 & 0x80) ? FL_C : 0)
        | flnz_list_get(temp);
    cycles += 7;
    put_8bit_addr(temp_addr, temp);
    reg_pc += 3;
    return 0;
}

int
rol_absx(void)
{
    uchar flg_tmp = (reg_p & FL_C) ? 1 : 0;
    uint16 temp_addr = get_16bit_addr(reg_pc + 1) + reg_x;
    uchar temp1 = get_8bit_addr(temp_addr);
    uchar temp = (temp1 << 1) + flg_tmp;

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((temp1 & 0x80) ? FL_C : 0)
        | flnz_list_get(temp);
    cycles += 7;
    put_8bit_addr(temp_addr, temp);
    reg_pc += 3;
    return 0;
}

int
rol_zp(void)
{
    uchar flg_tmp = (reg_p & FL_C) ? 1 : 0;
    uchar zp_addr = imm_operand(reg_pc + 1);
    uchar temp1 = get_8bit_zp(zp_addr);
    uchar temp = (temp1 << 1) + flg_tmp;

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((temp1 & 0x80) ? FL_C : 0)
        | flnz_list_get(temp);
    put_8bit_zp(zp_addr, temp);
    reg_pc += 2;
    cycles += 6;
    return 0;
}

int
rol_zpx(void)
{
    uchar flg_tmp = (reg_p & FL_C) ? 1 : 0;
    uchar zp_addr = imm_operand(reg_pc + 1) + reg_x;
    uchar temp1 = get_8bit_zp(zp_addr);
    uchar temp = (temp1 << 1) + flg_tmp;

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((temp1 & 0x80) ? FL_C : 0)
        | flnz_list_get(temp);
    put_8bit_zp(zp_addr, temp);
    reg_pc += 2;
    cycles += 6;
    return 0;
}

int
ror_a(void)
{
    uchar flg_tmp = (reg_p & FL_C) ? 0x80 : 0;
    uchar temp = reg_a;

    reg_a = (reg_a / 2) + flg_tmp;
    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((temp & 0x01) ? FL_C : 0)
        | flnz_list_get(reg_a);
    reg_pc++;
    cycles += 2;
    return 0;
}

int
ror_abs(void)
{
    uchar flg_tmp = (reg_p & FL_C) ? 0x80 : 0;
    uint16 temp_addr = get_16bit_addr(reg_pc + 1);
    uchar temp1 = get_8bit_addr(temp_addr);
    uchar temp = (temp1 / 2) + flg_tmp;

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((temp1 & 0x01) ? FL_C : 0)
        | flnz_list_get(temp);
    cycles += 7;
    put_8bit_addr(temp_addr, temp);
    reg_pc += 3;
    return 0;
}

int
ror_absx(void)
{
    uchar flg_tmp = (reg_p & FL_C) ? 0x80 : 0;
    uint16 temp_addr = get_16bit_addr(reg_pc + 1) + reg_x;
    uchar temp1 = get_8bit_addr(temp_addr);
    uchar temp = (temp1 / 2) + flg_tmp;

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((temp1 & 0x01) ? FL_C : 0)
        | flnz_list_get(temp);
    cycles += 7;
    put_8bit_addr(temp_addr, temp);
    reg_pc += 3;
    return 0;
}

int
ror_zp(void)
{
    uchar flg_tmp = (reg_p & FL_C) ? 0x80 : 0;
    uchar zp_addr = imm_operand(reg_pc + 1);
    uchar temp1 = get_8bit_zp(zp_addr);
    uchar temp = (temp1 / 2) + flg_tmp;

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((temp1 & 0x01) ? FL_C : 0)
        | flnz_list_get(temp);
    put_8bit_zp(zp_addr, temp);
    reg_pc += 2;
    cycles += 6;
    return 0;
}

int
ror_zpx(void)
{
    uchar flg_tmp = (reg_p & FL_C) ? 0x80 : 0;
    uchar zp_addr = imm_operand(reg_pc + 1) + reg_x;
    uchar temp1 = get_8bit_zp(zp_addr);
    uchar temp = (temp1 / 2) + flg_tmp;

    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))
        | ((temp1 & 0x01) ? FL_C : 0)
        | flnz_list_get(temp);
    put_8bit_zp(zp_addr, temp);
    reg_pc += 2;
    cycles += 6;
    return 0;
}

int
rti(void)
{
    /* FL_B reset in RTI */
    reg_p = pull_8bit() & ~FL_B;
    reg_pc = pull_16bit();
    cycles += 7;
    return 0;
}

int
rts(void)
{
    reg_p &= ~FL_T;
    reg_pc = pull_16bit() + 1;
    cycles += 7;
    return 0;
}

int
sax(void)
{
    uchar temp = reg_x;
    reg_p &= ~FL_T;
    reg_x = reg_a;
    reg_a = temp;
    reg_pc++;
    cycles += 3;
    return 0;
}

int
say(void)
{
    uchar temp = reg_y;
    reg_p &= ~FL_T;
    reg_y = reg_a;
    reg_a = temp;
    reg_pc++;
    cycles += 3;
    return 0;
}

int
sbc_abs(void)
{
    sbc(abs_operand(reg_pc + 1));
    reg_pc += 3;
    cycles += 5;
    return 0;
}

int
sbc_absx(void)
{
    sbc(absx_operand(reg_pc + 1));
    reg_pc += 3;
    cycles += 5;
    return 0;
}

int
sbc_absy(void)
{
    sbc(absy_operand(reg_pc + 1));
    reg_pc += 3;
    cycles += 5;
    return 0;
}

int
sbc_imm(void)
{
    sbc(imm_operand(reg_pc + 1));
    reg_pc += 2;
    cycles += 2;
    return 0;
}

int
sbc_zp(void)
{
    sbc(zp_operand(reg_pc + 1));
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
sbc_zpx(void)
{
    sbc(zpx_operand(reg_pc + 1));
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
sbc_zpind(void)
{
    sbc(zpind_operand(reg_pc + 1));
    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
sbc_zpindx(void)
{
    sbc(zpindx_operand(reg_pc + 1));
    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
sbc_zpindy(void)
{
    sbc(zpindy_operand(reg_pc + 1));
    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
sec(void)
{
    reg_p = (reg_p | FL_C) & ~FL_T;
    reg_pc++;
    cycles += 2;
    return 0;
}

int
sed(void)
{
    reg_p = (reg_p | FL_D) & ~FL_T;
    reg_pc++;
    cycles += 2;
    return 0;
}

int
sei(void)
{
#ifdef DUMP_ON_SEI
    int i;
    Log("MMR[7]\n");
    for (i = 0xE000; i < 0xE100; i++) {
        Log("%02X ", get_8bit_addr(i));
    }
#endif
    reg_p = (reg_p | FL_I) & ~FL_T;
    reg_pc++;
    cycles += 2;
    return 0;
}

int
set(void)
{
    reg_p |= FL_T;
    reg_pc++;
    cycles += 2;
    return 0;
}

int
smb0(void)
{
    uchar temp = imm_operand(reg_pc + 1);
    reg_p &= ~FL_T;
    put_8bit_zp(temp, get_8bit_zp(temp) | 0x01);
    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
smb1(void)
{
    uchar temp = imm_operand(reg_pc + 1);
    reg_p &= ~FL_T;
    put_8bit_zp(temp, get_8bit_zp(temp) | 0x02);
    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
smb2(void)
{
    uchar temp = imm_operand(reg_pc + 1);
    reg_p &= ~FL_T;
    put_8bit_zp(temp, get_8bit_zp(temp) | 0x04);
    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
smb3(void)
{
    uchar temp = imm_operand(reg_pc + 1);
    reg_p &= ~FL_T;
    put_8bit_zp(temp, get_8bit_zp(temp) | 0x08);
    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
smb4(void)
{
    uchar temp = imm_operand(reg_pc + 1);
    reg_p &= ~FL_T;
    put_8bit_zp(temp, get_8bit_zp(temp) | 0x10);
    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
smb5(void)
{
    uchar temp = imm_operand(reg_pc + 1);
    reg_p &= ~FL_T;
    put_8bit_zp(temp, get_8bit_zp(temp) | 0x20);
    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
smb6(void)
{
    uchar temp = imm_operand(reg_pc + 1);
    reg_p &= ~FL_T;
    put_8bit_zp(temp, get_8bit_zp(temp) | 0x40);
    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
smb7(void)
{
    uchar temp = imm_operand(reg_pc + 1);
    reg_p &= ~FL_T;
    put_8bit_zp(temp, get_8bit_zp(temp) | 0x80);
    reg_pc += 2;
    cycles += 7;
    return 0;
}

int
st0(void)
{
    reg_p &= ~FL_T;
    IO_write(0, imm_operand(reg_pc + 1));
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
st1(void)
{
    reg_p &= ~FL_T;
    IO_write(2, imm_operand(reg_pc + 1));
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
st2(void)
{
    reg_p &= ~FL_T;
    IO_write(3, imm_operand(reg_pc + 1));
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
sta_abs(void)
{
    reg_p &= ~FL_T;
    cycles += 5;
    put_8bit_addr(get_16bit_addr(reg_pc + 1), reg_a);
    reg_pc += 3;
    return 0;
}

int
sta_absx(void)
{
    reg_p &= ~FL_T;
    cycles += 5;
    put_8bit_addr(get_16bit_addr(reg_pc + 1) + reg_x, reg_a);
    reg_pc += 3;
    return 0;
}

int
sta_absy(void)
{
    reg_p &= ~FL_T;
    cycles += 5;
    put_8bit_addr(get_16bit_addr(reg_pc + 1) + reg_y, reg_a);
    reg_pc += 3;
    return 0;
}

int
sta_zp(void)
{
    reg_p &= ~FL_T;
    put_8bit_zp(imm_operand(reg_pc + 1), reg_a);

    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
sta_zpx(void)
{
    reg_p &= ~FL_T;
    put_8bit_zp(imm_operand(reg_pc + 1) + reg_x, reg_a);
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
sta_zpind(void)
{
    reg_p &= ~FL_T;
    cycles += 7;
    put_8bit_addr(get_16bit_zp(imm_operand(reg_pc + 1)), reg_a);
    reg_pc += 2;
    return 0;
}

int
sta_zpindx(void)
{
    reg_p &= ~FL_T;
    cycles += 7;
    put_8bit_addr(get_16bit_zp(imm_operand(reg_pc + 1) + reg_x), reg_a);
    reg_pc += 2;
    return 0;
}

int
sta_zpindy(void)
{
    reg_p &= ~FL_T;
    cycles += 7;
    put_8bit_addr(get_16bit_zp(imm_operand(reg_pc + 1)) + reg_y, reg_a);
    reg_pc += 2;
    return 0;
}

int
stx_abs(void)
{
    reg_p &= ~FL_T;
    cycles += 5;
    put_8bit_addr(get_16bit_addr(reg_pc + 1), reg_x);
    reg_pc += 3;
    return 0;
}

int
stx_zp(void)
{
    reg_p &= ~FL_T;
    put_8bit_zp(imm_operand(reg_pc + 1), reg_x);
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
stx_zpy(void)
{
    reg_p &= ~FL_T;
    put_8bit_zp(imm_operand(reg_pc + 1) + reg_y, reg_x);
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
sty_abs(void)
{
    reg_p &= ~FL_T;
    cycles += 5;
    put_8bit_addr(get_16bit_addr(reg_pc + 1), reg_y);
    reg_pc += 3;
    return 0;
}

int
sty_zp(void)
{
    reg_p &= ~FL_T;
    put_8bit_zp(imm_operand(reg_pc + 1), reg_y);
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
sty_zpx(void)
{
    reg_p &= ~FL_T;
    put_8bit_zp(imm_operand(reg_pc + 1) + reg_x, reg_y);
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
stz_abs(void)
{
    reg_p &= ~FL_T;
    cycles += 5;
    put_8bit_addr(get_16bit_addr(reg_pc + 1), 0);
    reg_pc += 3;
    return 0;
}

int
stz_absx(void)
{
    reg_p &= ~FL_T;
    cycles += 5;
    put_8bit_addr((get_16bit_addr(reg_pc + 1) + reg_x), 0);
    reg_pc += 3;
    return 0;
}

int
stz_zp(void)
{
    reg_p &= ~FL_T;
    put_8bit_zp(imm_operand(reg_pc + 1), 0);
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
stz_zpx(void)
{
    reg_p &= ~FL_T;
    put_8bit_zp(imm_operand(reg_pc + 1) + reg_x, 0);
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
sxy(void)
{
    uchar temp = reg_y;
    reg_p &= ~FL_T;
    reg_y = reg_x;
    reg_x = temp;
    reg_pc++;
    cycles += 3;
    return 0;
}

int
tai(void)
{
    uint16 from, to, len, alternate;

    reg_p &= ~FL_T;
    from = get_16bit_addr(reg_pc + 1);
    to = get_16bit_addr(reg_pc + 3);
    len = get_16bit_addr(reg_pc + 5);
    alternate = 0;

#if ENABLE_TRACING_CD && defined(INLINED_ACCESSORS)
    fprintf(stderr, "Transfert from bank 0x%02x to bank 0x%02x\n",
            mmr[from >> 13], mmr[to >> 13]);
#endif

    cycles += (6 * len) + 17;
    while (len-- != 0) {
        put_8bit_addr(to++, get_8bit_addr(from + alternate));
        alternate ^= 1;
    }
    reg_pc += 7;
    return 0;
}

int
tam(void)
{
    uint16 i;
    uchar bitfld = imm_operand(reg_pc + 1);

#if defined(KERNEL_DEBUG)
    if (bitfld == 0) {
        fprintf(stderr, "TAM with argument 0\n");
        Log("TAM with argument 0\n");
    } else if (!one_bit_set(bitfld)) {
        fprintf(stderr, "TAM with unusual argument 0x%02x\n", bitfld);
        Log("TAM with unusual argument 0x%02x\n", bitfld);
    }
#endif

    for (i = 0; i < 8; i++) {
        if (bitfld & (1 << i)) {
            mmr[i] = reg_a;
            bank_set(i, reg_a);
        }
    }

    reg_p &= ~FL_T;
    reg_pc += 2;
    cycles += 5;
    return 0;
}

int
tax(void)
{
    reg_x = reg_a;
    chk_flnz_8bit(reg_a);
    reg_pc++;
    cycles += 2;
    return 0;
}

int
tay(void)
{
    reg_y = reg_a;
    chk_flnz_8bit(reg_a);
    reg_pc++;
    cycles += 2;
    return 0;
}

int
tdd(void)
{
    uint16 from, to, len;

    reg_p &= ~FL_T;
    from = get_16bit_addr(reg_pc + 1);
    to = get_16bit_addr(reg_pc + 3);
    len = get_16bit_addr(reg_pc + 5);

#if ENABLE_TRACING_CD && defined(INLINED_ACCESSORS)
    fprintf(stderr, "Transfert from bank 0x%02x to bank 0x%02x\n",
            mmr[from >> 13], mmr[to >> 13]);
#endif

    cycles += (6 * len) + 17;
    while (len-- != 0) {
        put_8bit_addr(to--, get_8bit_addr(from--));
    }
    reg_pc += 7;
    return 0;
}

int
tia(void)
{
    uint16 from, to, len, alternate;

    reg_p &= ~FL_T;
    from = get_16bit_addr(reg_pc + 1);
    to = get_16bit_addr(reg_pc + 3);
    len = get_16bit_addr(reg_pc + 5);
    alternate = 0;

#if ENABLE_TRACING_CD && defined(INLINED_ACCESSORS)
    fprintf(stderr, "Transfert from bank 0x%02x to bank 0x%02x\n",
            mmr[from >> 13], mmr[to >> 13]);
#endif

    cycles += (6 * len) + 17;
    while (len-- != 0) {
        put_8bit_addr(to + alternate, get_8bit_addr(from++));
        alternate ^= 1;
    }
    reg_pc += 7;
    return 0;
}

int
tii(void)
{
    uint16 from, to, len;

    reg_p &= ~FL_T;
    from = get_16bit_addr(reg_pc + 1);
    to = get_16bit_addr(reg_pc + 3);
    len = get_16bit_addr(reg_pc + 5);

#if ENABLE_TRACING_CD && defined(INLINED_ACCESSORS)
    fprintf(stderr, "Transfert from bank 0x%02x to bank 0x%02x\n",
            mmr[from >> 13], mmr[to >> 13]);
#endif

    cycles += (6 * len) + 17;
    while (len-- != 0) {
        put_8bit_addr(to++, get_8bit_addr(from++));
    }
    reg_pc += 7;
    return 0;
}

int
tin(void)
{
    uint16 from, to, len;

    reg_p &= ~FL_T;
    from = get_16bit_addr(reg_pc + 1);
    to = get_16bit_addr(reg_pc + 3);
    len = get_16bit_addr(reg_pc + 5);

#if ENABLE_TRACING_CD && defined(INLINED_ACCESSORS)
    fprintf(stderr, "Transfert from bank 0x%02x to bank 0x%02x\n",
            mmr[from >> 13], mmr[to >> 13]);
#endif

    cycles += (6 * len) + 17;
    while (len-- != 0) {
        put_8bit_addr(to, get_8bit_addr(from++));
    }
    reg_pc += 7;
    return 0;
}

int
tma(void)
{
    int i;
    uchar bitfld = imm_operand(reg_pc + 1);

#if defined(KERNEL_DEBUG)
    if (bitfld == 0) {
        fprintf(stderr, "TMA with argument 0\n");
        Log("TMA with argument 0\n");
    } else if (!one_bit_set(bitfld)) {
        fprintf(stderr, "TMA with unusual argument 0x%02x\n", bitfld);
        Log("TMA with unusual argument 0x%02x\n", bitfld);
    }
#endif

    for (i = 0; i < 8; i++) {
        if (bitfld & (1 << i)) {
            reg_a = mmr[i];
        }
    }
    reg_p &= ~FL_T;
    reg_pc += 2;
    cycles += 4;
    return 0;
}

int
trb_abs(void)
{
    uint16 abs_addr = get_16bit_addr(reg_pc + 1);
    uchar temp = get_8bit_addr(abs_addr);
    uchar temp1 = (~reg_a) & temp;

    reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z))
        | ((temp1 & 0x80) ? FL_N : 0)
        | ((temp1 & 0x40) ? FL_V : 0)
        | ((temp & reg_a) ? 0 : FL_Z);
    cycles += 7;
    put_8bit_addr(abs_addr, temp1);
    reg_pc += 3;
    return 0;
}

int
trb_zp(void)
{
    uchar zp_addr = imm_operand(reg_pc + 1);
    uchar temp = get_8bit_zp(zp_addr);
    uchar temp1 = (~reg_a) & temp;

    reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z))
        | ((temp1 & 0x80) ? FL_N : 0)
        | ((temp1 & 0x40) ? FL_V : 0)
        | ((temp & reg_a) ? 0 : FL_Z);
    put_8bit_zp(zp_addr, temp1);
    reg_pc += 2;
    cycles += 6;
    return 0;
}

int
tsb_abs(void)
{
    uint16 abs_addr = get_16bit_addr(reg_pc + 1);
    uchar temp = get_8bit_addr(abs_addr);
    uchar temp1 = reg_a | temp;

    reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z))
        | ((temp1 & 0x80) ? FL_N : 0)
        | ((temp1 & 0x40) ? FL_V : 0)
        | ((temp & reg_a) ? 0 : FL_Z);
    cycles += 7;
    put_8bit_addr(abs_addr, temp1);
    reg_pc += 3;
    return 0;
}

int
tsb_zp(void)
{
    uchar zp_addr = imm_operand(reg_pc + 1);
    uchar temp = get_8bit_zp(zp_addr);
    uchar temp1 = reg_a | temp;

    reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z))
        | ((temp1 & 0x80) ? FL_N : 0)
        | ((temp1 & 0x40) ? FL_V : 0)
        | ((temp & reg_a) ? 0 : FL_Z);
    put_8bit_zp(zp_addr, temp1);
    reg_pc += 2;
    cycles += 6;
    return 0;
}

int
tstins_abs(void)
{
    uchar imm = imm_operand(reg_pc + 1);
    uchar temp = abs_operand(reg_pc + 2);

    reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z))
        | ((temp & 0x80) ? FL_N : 0)
        | ((temp & 0x40) ? FL_V : 0)
        | ((temp & imm) ? 0 : FL_Z);
    cycles += 8;
    reg_pc += 4;
    return 0;
}

int
tstins_absx(void)
{
    uchar imm = imm_operand(reg_pc + 1);
    uchar temp = absx_operand(reg_pc + 2);

    reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z))
        | ((temp & 0x80) ? FL_N : 0)
        | ((temp & 0x40) ? FL_V : 0)
        | ((temp & imm) ? 0 : FL_Z);
    cycles += 8;
    reg_pc += 4;
    return 0;
}

int
tstins_zp(void)
{
    uchar imm = imm_operand(reg_pc + 1);
    uchar temp = zp_operand(reg_pc + 2);

    reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z))
        | ((temp & 0x80) ? FL_N : 0)
        | ((temp & 0x40) ? FL_V : 0)
        | ((temp & imm) ? 0 : FL_Z);
    cycles += 7;
    reg_pc += 3;
    return 0;
}

int
tstins_zpx(void)
{
    uchar imm = imm_operand(reg_pc + 1);
    uchar temp = zpx_operand(reg_pc + 2);

    reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z))
        | ((temp & 0x80) ? FL_N : 0)
        | ((temp & 0x40) ? FL_V : 0)
        | ((temp & imm) ? 0 : FL_Z);
    cycles += 7;
    reg_pc += 3;
    return 0;
}


int
tsx(void)
{
    reg_x = reg_s;
    chk_flnz_8bit(reg_s);
    reg_pc++;
    cycles += 2;
    return 0;
}

int
txa(void)
{
    reg_a = reg_x;
    chk_flnz_8bit(reg_x);
    reg_pc++;
    cycles += 2;
    return 0;
}

int
txs(void)
{
    reg_p &= ~FL_T;
    reg_s = reg_x;
    reg_pc++;
    cycles += 2;
    return 0;
}

int
tya(void)
{
    reg_a = reg_y;
    chk_flnz_8bit(reg_y);
    reg_pc++;
    cycles += 2;
    return 0;
}

// perform machine operations for IRQ2:

int
int_irq2(void)
{
    if ((io.irq_mask & FL_IRQ2) != 0) { // interrupt disabled
        return 0;
    }
//if ((irq_register & FL_IRQ2) == 0) {   // interrupt disabled ?
//   return 0;
//}
    cycles += 7;
    push_16bit(reg_pc);
    push_8bit(reg_p);
    reg_p = (reg_p & ~FL_D) | FL_I;
    reg_pc = get_16bit_addr(0xFFF6);
    return 0;
}

// perform machine operations for IRQ1 (video interrupt):

int
int_irq1(void)
{
    if ((io.irq_mask & FL_IRQ1) != 0) { // interrupt disabled
        return 0;
    }
//if ((irq_register & FL_IRQ1) == 0) {   // interrupt disabled ?
//   return 0;
//}
    cycles += 7;
    push_16bit(reg_pc);
    push_8bit(reg_p);
    reg_p = (reg_p & ~FL_D) | FL_I;
    reg_pc = get_16bit_addr(0xFFF8);
    return 0;
}

// perform machine operations for timer interrupt:

int
int_tiq(void)
{
    if ((io.irq_mask & FL_TIQ) != 0) {  // interrupt disabled
        return 0;
    }
//if ((irq_register & FL_TIQ) == 0) {   // interrupt disabled ?
//   return 0;
//}
    cycles += 7;
    push_16bit(reg_pc);
    push_8bit(reg_p);
    reg_p = (reg_p & ~FL_D) | FL_I;
    reg_pc = get_16bit_addr(0xFFFA);
    return 0;
}

#undef cycles
