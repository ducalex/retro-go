// #define _OPCODE_

#define _OPCODE_adc(acc_,val_, rc_)                                                \
{                                                                                  \
    rc_ = acc_;                                                             \
    int16 sig = (SBYTE) rc_;                                                      \
    uint16 usig = (uchar) rc_;                                                    \
    uchar _val = val_;                                                             \
    uint16 temp;                                                                   \
                                                                                   \
    if (!(reg_p & FL_D)) {      /* binary mode */                                  \
        if (reg_p & FL_C) {                                                        \
            usig++;                                                                \
            sig++;                                                                 \
        }                                                                          \
        sig += (SBYTE) _val;                                                       \
        usig += (uchar) _val;                                                      \
        rc_ = (uchar) (usig & 0xFF);                                              \
                                                                                   \
        reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z | FL_C))                      \
            | (((sig > 127) || (sig < -128)) ? FL_V : 0)                           \
            | ((usig > 255) ? FL_C : 0)                                            \
            | flnz_list_get(rc_);                                                     \
                                                                                   \
    } else {                    /* decimal mode */                                 \
        temp = bcdbin[usig] + bcdbin[_val];                                        \
                                                                                   \
        if (reg_p & FL_C) {                                                        \
            temp++;                                                                \
        }                                                                          \
                                                                                   \
        rc_ = binbcd[temp];                                                       \
                                                                                   \
        reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C))                             \
            | ((temp > 99) ? FL_C : 0)                                             \
            | flnz_list_get(rc_);                                                     \
        cycles++;    /* decimal mode takes an extra cycle */                       \
    }                                                                              \
  /* rc_ = _acc; */                                                                 \
}


#define _OPCODE_ld__(register_, operand_) \
    register_ = operand_(reg_pc + 1); \
    chk_flnz_8bit(register_); \
    reg_pc += 2; \
    cycles += 4;

#define _OPCODE_ld__imm(register_) \
    register_ = imm_operand(reg_pc + 1); \
    chk_flnz_8bit(register_); \
    reg_pc += 2; \
    cycles += 2;

#define _OPCODE_ld__ind(register_, operand_) \
    register_ = operand_(reg_pc + 1); \
    chk_flnz_8bit(register_); \
    reg_pc += 2; \
    cycles += 7;

#define _OPCODE_ld__abs(register_, operand_) \
    register_ = operand_(reg_pc + 1); \
    chk_flnz_8bit(register_); \
    reg_pc += 3; \
    cycles += 5;


#define _OPCODE_st__abs(reg_1, reg_2) \
    reg_p &= ~FL_T; \
    cycles += 5; \
    put_8bit_addr(get_16bit_addr(reg_pc + 1) + reg_2, reg_1); \
    reg_pc += 3;

#define _OPCODE_st__zp(reg_1, reg_2) \
    reg_p &= ~FL_T; \
    put_8bit_zp(imm_operand(reg_pc + 1) + reg_2, reg_1); \
    reg_pc += 2; \
    cycles += 4;

#define _OPCODE_sta__zpind(reg_1) \
    reg_p &= ~FL_T; \
    cycles += 7; \
    /* put_8bit_addr(get_16bit_zp(imm_operand(reg_pc + 1) + reg_1), reg_a); */ \
    put_8bit_addr(get_16bit_zp(imm_operand(reg_pc + 1)) + reg_1, reg_a); \
    reg_pc += 2;


#define _OPCODE_sbc__zpind(operand_) \
    sbc(operand_(reg_pc + 1)); \
    reg_pc += 2; \
    cycles += 7;

#define _OPCODE_sbc__zp(operand_) \
    sbc(operand_(reg_pc + 1)); \
    reg_pc += 2; \
    cycles += 4;

#define _OPCODE_sbc__imm \
    sbc(imm_operand(reg_pc + 1)); \
    reg_pc += 2; \
    cycles += 2;


//adc_zp, AM_ZP, "ADC" $65
#define _OPCODE_adc_zp \
    if (reg_p & FL_T) { \
        uchar tmp; \
        _OPCODE_adc( get_8bit_zp(reg_x), zp_operand(reg_pc + 1), tmp) \
        put_8bit_zp(reg_x, tmp); \
        /* put_8bit_zp(reg_x, adc(get_8bit_zp(reg_x), zp_operand(reg_pc + 1))); */ \
        cycles += 7; \
    } else { \
        _OPCODE_adc( reg_a, zp_operand(reg_pc + 1), reg_a) \
        /* reg_a = adc(reg_a, zp_operand(reg_pc + 1)); */ \
        cycles += 4; \
    } \
    reg_pc += 2;


#define _OPCODE_adc_zpx \
    if (reg_p & FL_T) { \
        put_8bit_zp(reg_x, adc(get_8bit_zp(reg_x), zpx_operand(reg_pc + 1))); \
        cycles += 7; \
    } else { \
        reg_a = adc(reg_a, zpx_operand(reg_pc + 1)); \
        cycles += 4; \
    } \
    reg_pc += 2;


#define _OPCODE_branch(check) \
    reg_p &= ~FL_T; \
    if (check) { \
        reg_pc += (SBYTE) imm_operand(reg_pc + 1) + 2; \
        cycles += 4; \
    } else { \
        reg_pc += 2; \
        cycles += 2; \
    }

#define _OPCODE_ph_(register_) \
    reg_p &= ~FL_T; \
    push_8bit(register_); \
    reg_pc++; \
    cycles += 3;

#define _OPCODE_pl_(register_) \
    register_ = pull_8bit(); \
    chk_flnz_8bit(register_); \
    reg_pc++; \
    cycles += 4;

#define _OPCODE_st__(offset) \
    reg_p &= ~FL_T; \
    IO_write(offset, imm_operand(reg_pc + 1)); \
    reg_pc += 2; \
    cycles += 4;

#define _OPCODE_asl_abs(offset_) { \
    uint16 temp_addr = get_16bit_addr(reg_pc + 1) + offset_; \
    uchar temp1 = get_8bit_addr(temp_addr); \
    uchar temp = temp1 << 1; \
 \
    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) \
        | ((temp1 & 0x80) ? FL_C : 0) \
        | flnz_list_get(temp); \
    cycles += 7; \
 \
    put_8bit_addr(temp_addr, temp); \
    reg_pc += 3; }

#define _OPCODE_asl_zp(offset_) { \
    uchar zp_addr = imm_operand(reg_pc + 1) + offset_; \
    uchar temp1 = get_8bit_zp(zp_addr); \
    uchar temp = temp1 << 1; \
 \
    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) \
        | ((temp1 & 0x80) ? FL_C : 0) \
        | flnz_list_get(temp); \
    cycles += 6; \
    put_8bit_zp(zp_addr, temp); \
    reg_pc += 2; }


#define _OPCODE_rol_abs(offset_) { \
    uchar flg_tmp = (reg_p & FL_C) ? 1 : 0; \
    uint16 temp_addr = get_16bit_addr(reg_pc + 1) + offset_; \
    uchar temp1 = get_8bit_addr(temp_addr); \
    uchar temp = (temp1 << 1) + flg_tmp; \
    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) \
        | ((temp1 & 0x80) ? FL_C : 0) \
        | flnz_list_get(temp); \
    cycles += 7; \
    put_8bit_addr(temp_addr, temp); \
    reg_pc += 3; }


#define _OPCODE_rol_zp(offset_) { \
    uchar flg_tmp = (reg_p & FL_C) ? 1 : 0; \
    uchar zp_addr = imm_operand(reg_pc + 1) + offset_; \
    uchar temp1 = get_8bit_zp(zp_addr); \
    uchar temp = (temp1 << 1) + flg_tmp; \
 \
    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) \
        | ((temp1 & 0x80) ? FL_C : 0) \
        | flnz_list_get(temp); \
    put_8bit_zp(zp_addr, temp); \
    reg_pc += 2; \
    cycles += 6; }


#define _OPCODE_bbr_(mask_) \
    reg_p &= ~FL_T; \
    if (zp_operand(reg_pc + 1) & mask_) { \
        reg_pc += 3; \
        cycles += 6; \
    } else { \
        reg_pc += (SBYTE) imm_operand(reg_pc + 2) + 3; \
        cycles += 8; \
    }

#define _OPCODE_ora__(operand_, cycles_add1, cycles_add2, reg_pc_add) \
    if (reg_p & FL_T) { \
        uchar temp = get_8bit_zp(reg_x); \
        temp |= operand_(reg_pc + 1); \
        chk_flnz_8bit(temp); \
        put_8bit_zp(reg_x, temp); \
        cycles += cycles_add1; \
    } else { \
        reg_a |= operand_(reg_pc + 1); \
        chk_flnz_8bit(reg_a); \
        cycles += cycles_add2; \
    } \
    reg_pc += reg_pc_add;

#define _OPCODE_and__(operand_, cycles_add1, cycles_add2, reg_pc_add) \
    if (reg_p & FL_T) { \
        uchar temp = get_8bit_zp(reg_x); \
        temp &= operand_(reg_pc + 1); \
        chk_flnz_8bit(temp); \
        put_8bit_zp(reg_x, temp); \
        cycles += cycles_add1; \
    } else { \
        reg_a &= operand_(reg_pc + 1); \
        chk_flnz_8bit(reg_a); \
        cycles += cycles_add2; \
    } \
    reg_pc += reg_pc_add;

#define _OPCODE_ror_abs_(offset_) { \
    uchar flg_tmp = (reg_p & FL_C) ? 0x80 : 0; \
    uint16 temp_addr = get_16bit_addr(reg_pc + 1) + offset_; \
    uchar temp1 = get_8bit_addr(temp_addr); \
    uchar temp = (temp1 / 2) + flg_tmp; \
    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) \
        | ((temp1 & 0x01) ? FL_C : 0) \
        | flnz_list_get(temp); \
    cycles += 7; \
    put_8bit_addr(temp_addr, temp); \
    reg_pc += 3; } \

#define _OPCODE_ror_zp_(offset_) { \
    uchar flg_tmp = (reg_p & FL_C) ? 0x80 : 0; \
    uchar zp_addr = imm_operand(reg_pc + 1) + offset_; \
    uchar temp1 = get_8bit_zp(zp_addr); \
    uchar temp = (temp1 / 2) + flg_tmp; \
 \
    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) \
        | ((temp1 & 0x01) ? FL_C : 0) \
        | flnz_list_get(temp); \
    put_8bit_zp(zp_addr, temp); \
    reg_pc += 2; \
    cycles += 6; }

#define _OPCODE_bit__(operand_, cycles_add, reg_pc_add) { \
    uchar temp = operand_(reg_pc + 1); \
    reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z)) \
        | ((temp & 0x80) ? FL_N : 0) \
        | ((temp & 0x40) ? FL_V : 0) \
        | ((reg_a & temp) ? 0 : FL_Z); \
    reg_pc += reg_pc_add; \
    cycles += cycles_add; }


#define _OPCODE_bbs_(mask_) \
    reg_p &= ~FL_T; \
    if (zp_operand(reg_pc + 1) & mask_) { \
        reg_pc += (SBYTE) imm_operand(reg_pc + 2) + 3; \
        cycles += 8; \
    } else { \
        reg_pc += 3; \
        cycles += 6; \
    }

// {cpx_zp, AM_ZP, "CPX"} $E4
#define _OPCODE_cpx_zp \
        { \
        uchar temp = zp_operand(reg_pc + 1); \
        reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) \
            | ((reg_x < temp) ? 0 : FL_C) \
            | flnz_list[(uchar) (reg_x - temp)]; \
        reg_pc += 2; \
        cycles += 4; \
        }

#define _OPCODE_cp__(operand_, reg_, cycles_add, reg_pc_add) \
    { \
    uchar temp = operand_(reg_pc + 1); \
    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) \
        | ((reg_ < temp) ? 0 : FL_C) \
        | flnz_list[(uchar) (reg_ - temp)]; \
    reg_pc += reg_pc_add; \
    cycles += cycles_add; \
    }

#define _OPCODE_cmp__(operand_, cycles_add, reg_pc_add) { \
    uchar temp = operand_(reg_pc + 1); \
    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) \
        | ((reg_a < temp) ? 0 : FL_C) \
        | flnz_list[(uchar) (reg_a - temp)]; \
    reg_pc += reg_pc_add; \
    cycles += cycles_add; }

#define _OPCODE_dec_abs_(offset_) { \
    uchar temp; \
    uint16 temp_addr = get_16bit_addr(reg_pc + 1) + offset_; \
    temp = get_8bit_addr(temp_addr) - 1; \
    chk_flnz_8bit(temp); \
    cycles += 7; \
    put_8bit_addr(temp_addr, temp); \
    reg_pc += 3; }

#define _OPCODE_dec_zp_(offset_) { \
    uchar temp; \
    uchar zp_addr = imm_operand(reg_pc + 1) + offset_; \
    temp = get_8bit_zp(zp_addr) - 1; \
    chk_flnz_8bit(temp); \
    put_8bit_zp(zp_addr, temp); \
    reg_pc += 2; \
    cycles += 6; }

#define _OPCODE_inc_abs_(offset_) { \
    uchar temp; \
    uint16 temp_addr = get_16bit_addr(reg_pc + 1) + offset_; \
    temp = get_8bit_addr(temp_addr) + 1; \
    chk_flnz_8bit(temp); \
    cycles += 7; \
    put_8bit_addr(temp_addr, temp); \
    reg_pc += 3; }

#define _OPCODE_inc_zp_(offset_) { \
    uchar temp; \
    uchar zp_addr = imm_operand(reg_pc + 1) + offset_; \
    temp = get_8bit_zp(zp_addr) + 1; \
    chk_flnz_8bit(temp); \
    put_8bit_zp(zp_addr, temp); \
    reg_pc += 2; \
    cycles += 6; }

#define _OPCODE_nop_ \
    reg_p &= ~FL_T; \
    reg_pc++; \
    cycles += 2;

#define _OPCODE_eor__(operand_, cycles_add1, cycles_add2, reg_pc_add) { \
    if (reg_p & FL_T) { \
        uchar temp = get_8bit_zp(reg_x); \
        temp ^= operand_(reg_pc + 1); \
        chk_flnz_8bit(temp); \
        put_8bit_zp(reg_x, temp); \
        cycles += cycles_add1; \
    } else { \
        reg_a ^= operand_(reg_pc + 1); \
        chk_flnz_8bit(reg_a); \
        cycles += cycles_add2; \
    } \
    reg_pc += reg_pc_add; }

#define _OPCODE_lsr_abs_(offset_) { \
    uint16 temp_addr = get_16bit_addr(reg_pc + 1) + offset_; \
    uchar temp1 = get_8bit_addr(temp_addr); \
    uchar temp = temp1 / 2; \
    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) \
        | ((temp1 & 1) ? FL_C : 0) \
        | flnz_list_get(temp); \
    cycles += 7; \
    put_8bit_addr(temp_addr, temp); \
    reg_pc += 3; } \

#define _OPCODE_lsr_zp_(offset_) { \
    uchar zp_addr = imm_operand(reg_pc + 1) + offset_; \
    uchar temp1 = get_8bit_zp(zp_addr); \
    uchar temp = temp1 / 2; \
    reg_p = (reg_p & ~(FL_N | FL_T | FL_Z | FL_C)) \
        | ((temp1 & 1) ? FL_C : 0) \
        | flnz_list_get(temp); \
    put_8bit_zp(zp_addr, temp); \
    reg_pc += 2; \
    cycles += 6; }

#define _OPCODE_add_abc_(operand_) \
    if (reg_p & FL_T) { \
        put_8bit_zp(reg_x, adc(get_8bit_zp(reg_x), operand_(reg_pc + 1))); \
        cycles += 8; \
    } else { \
        reg_a = adc(reg_a, operand_(reg_pc + 1)); \
        cycles += 5; \
    } \
    reg_pc += 3;

#define _OPCODE_smb_(mask_) { \
    uchar temp = imm_operand(reg_pc + 1); \
    reg_p &= ~FL_T; \
    put_8bit_zp(temp, get_8bit_zp(temp) | mask_); \
    reg_pc += 2; \
    cycles += 7; }

#define _OPCODE_rmb_(mask_) { \
    uchar temp = imm_operand(reg_pc + 1); \
    reg_p &= ~FL_T; \
    put_8bit_zp(temp, get_8bit_zp(temp) & (~mask_)); \
    reg_pc += 2; \
    cycles += 7; }

#define _OPCODE_tstins_(operand_, cycles_add, reg_pc_add) { \
    uchar imm = imm_operand(reg_pc + 1); \
    uchar temp = operand_(reg_pc + 2); \
    reg_p = (reg_p & ~(FL_N | FL_V | FL_T | FL_Z)) \
        | ((temp & 0x80) ? FL_N : 0) \
        | ((temp & 0x40) ? FL_V : 0) \
        | ((temp & imm) ? 0 : FL_Z); \
    cycles += cycles_add; \
    reg_pc += reg_pc_add; }
