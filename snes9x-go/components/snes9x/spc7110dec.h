/* This file is part of Snes9x. See LICENSE file. */

#ifndef _SPC7110DEC_H_
#define _SPC7110DEC_H_
#include "port.h"

uint8_t spc7110dec_read(void);
void spc7110dec_clear(uint32_t mode, uint32_t offset, uint32_t index);
void spc7110dec_reset(void);

void spc7110dec_init(void);
void spc7110dec_deinit(void);

void spc7110dec_write(uint8_t data);
uint8_t spc7110dec_dataread(void);

void spc7110dec_mode0(bool init);
void spc7110dec_mode1(bool init);
void spc7110dec_mode2(bool init);

uint8_t spc7110dec_probability(uint32_t n);
uint8_t spc7110dec_next_lps(uint32_t n);
uint8_t spc7110dec_next_mps(uint32_t n);
bool spc7110dec_toggle_invert(uint32_t n);

uint32_t spc7110dec_morton_2x8(uint32_t data);
uint32_t spc7110dec_morton_4x8(uint32_t data);
#endif
