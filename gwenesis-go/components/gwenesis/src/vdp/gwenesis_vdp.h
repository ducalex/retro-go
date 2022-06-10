/*
Gwenesis : Genesis & megadrive Emulator.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.
This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
this program. If not, see <http://www.gnu.org/licenses/>.

__author__ = "bzhxx"
__contact__ = "https://github.com/bzhxx"
__license__ = "GPLv3"

*/
#ifndef _gwenesis_vdp_H_
#define _gwenesis_vdp_H_

#pragma once

#define BIT(v, idx) (((v) >> (idx)) & 1)
#define BITS(v, idx, n) (((v) >> (idx)) & ((1 << (n)) - 1))

// VDP registers
#define REG0_DISABLE_DISPLAY (gwenesis_vdp_regs[0] & 1)
#define REG0_HVLATCH BIT(gwenesis_vdp_regs[0], 1)
#define REG0_LINE_INTERRUPT BIT(gwenesis_vdp_regs[0], 4)
#define REG1_PAL BIT(gwenesis_vdp_regs[1], 3)
#define REG1_240_LINE ((gwenesis_vdp_regs[1] & 0x08) >> 3)
#define REG1_DMA_ENABLED BIT(gwenesis_vdp_regs[1], 4)
#define REG1_VBLANK_INTERRUPT BIT(gwenesis_vdp_regs[1], 5)
#define REG1_DISP_ENABLED BIT(gwenesis_vdp_regs[1], 6)
#define REG2_NAMETABLE_A (BITS(gwenesis_vdp_regs[2], 3, 3) << 13)
#define REG3_NAMETABLE_W BITS(gwenesis_vdp_regs[3], 1, 5)
#define REG4_NAMETABLE_B (BITS(gwenesis_vdp_regs[4], 0, 3) << 13)
//#define REG5_SAT_ADDRESS ((gwenesis_vdp_regs[5] & (mode_h40 ? 0x7E : 0x7F)) << 9)
//#define REG5_SAT_SIZE (mode_h40 ? (1 << 10) : (1 << 9))
#define REG5_SAT_ADDRESS ((gwenesis_vdp_regs[5] & ((gwenesis_vdp_regs[12] & 0x01) ? 0x7E : 0x7F)) << 9)
#define REG5_SAT_SIZE ((gwenesis_vdp_regs[12] & 0x01) ? (1 << 10) : (1 << 9))
#define REG10_LINE_COUNTER BITS(gwenesis_vdp_regs[10], 0, 8)
#define REG10_COLUMN_COUNTER BITS(gwenesis_vdp_regs[10], 8, 15)
#define REG11_HSCROLL_MODE ((gwenesis_vdp_regs[11] & 3))
#define REG11_VSCROLL_MODE ((gwenesis_vdp_regs[11] & 4) >> 2)
#define REG12_RS0 (gwenesis_vdp_regs[12] & 0x80) >> 7
#define REG12_RS1 (gwenesis_vdp_regs[12] & 0x01) >> 0
#define REG12_MODE_H40 (gwenesis_vdp_regs[12] & 1)
#define REG13_HSCROLL_ADDRESS (gwenesis_vdp_regs[13] << 10)
#define REG15_DMA_INCREMENT gwenesis_vdp_regs[15]
#define REG16_UNUSED1 ((gwenesis_vdp_regs[16] & 0xc0) >> 6)
#define REG16_VSCROLL_SIZE ((gwenesis_vdp_regs[16] >> 4) & 3)
#define REG16_UNUSED2 ((gwenesis_vdp_regs[16] & 0x0c) >> 2)
#define REG16_HSCROLL_SIZE (gwenesis_vdp_regs[16] & 3)
#define REG17_WINDOW_HPOS BITS(gwenesis_vdp_regs[17], 0, 5)
#define REG17_WINDOW_RIGHT ((gwenesis_vdp_regs[17] & 0x80) >> 7)
#define REG18_WINDOW_DOWN ((gwenesis_vdp_regs[0x12] & 0x80) >> 7)
#define REG18_WINDOW_VPOS BITS(gwenesis_vdp_regs[18], 0, 5)
#define REG19_DMA_LENGTH (gwenesis_vdp_regs[19] | (gwenesis_vdp_regs[20] << 8))
#define REG21_DMA_SRCADDR_LOW (gwenesis_vdp_regs[21] | (gwenesis_vdp_regs[22] << 8))
#define REG23_DMA_SRCADDR_HIGH ((gwenesis_vdp_regs[23] & 0x7F) << 16)
#define REG23_DMA_TYPE BITS(gwenesis_vdp_regs[23], 6, 2)

//VDP status register
#define STATUS_FIFO_EMPTY (1 << 9)
#define STATUS_FIFO_FULL (1 << 8)
#define STATUS_VIRQPENDING (1 << 7)
#define STATUS_SPRITEOVERFLOW (1 << 6)
#define STATUS_SPRITECOLLISION (1 << 5)
#define STATUS_ODDFRAME (1 << 4)
#define STATUS_VBLANK (1 << 3)
#define STATUS_HBLANK (1 << 2)
#define STATUS_DMAPROGRESS (1 << 1)
#define STATUS_PAL (1 << 0)

#define VRAM_MAX_SIZE 0x10000    // VRAM maximum size
#define CRAM_MAX_SIZE 0x40       // CRAM maximum size
#define VSRAM_MAX_SIZE 0x40      // VSRAM maximum size
#define SAT_CACHE_MAX_SIZE 0x400 // SAT CACHE maximum size
#define REG_SIZE 0x20            // REGISTERS total
#define FIFO_SIZE 0x4            // FIFO maximum size

#define M68K_FREQ_DIVISOR 7      // Frequency divisor to 68K clock
#define Z80_FREQ_DIVISOR 14     // Frequency divisor to Z80 clock
#define M68K_CYCLES_PER_LINE 3420 // M68K Cycles per Line
#define VDP_CYCLES_PER_LINE 3420// VDP Cycles per Line
#define SCREEN_WIDTH 320

#define COLOR_3B_TO_8B(c)  (((c) << 5) | ((c) << 2) | ((c) >> 1))
#define CRAM_R(c)          COLOR_3B_TO_8B(BITS((c), 1, 3))
#define CRAM_G(c)          COLOR_3B_TO_8B(BITS((c), 5, 3))
#define CRAM_B(c)          COLOR_3B_TO_8B(BITS((c), 9, 3))

#define MODE_SHI           BITS(gwenesis_vdp_regs[12], 3, 1)

#define SHADOW_COLOR(r,g,b) \
    do { r >>= 1; g >>= 1; b >>= 1; } while (0)
#define HIGHLIGHT_COLOR(r,g,b) \
    do { SHADOW_COLOR(r,g,b); r |= 0x80; g |= 0x80; b |= 0x80; } while(0)

// While we draw the planes, we use bit 0x80 on each pixel to save the
// high-priority flag, so that we can later prioritize.
#define PIXATTR_HIPRI        0x80
#define PIXATTR_LOWPRI       0x00
#define PIXATTR_SPRITE       0x40
#define PIXATTR_SPRITE_HIPRI 0xC0


// After mixing code, we use free bits 0x80 and 0x40 to indicate the
// shadow/highlight effect to apply on each pixel. Notice that we use
// 0x80 to indicate normal drawing and 0x00 to indicate shadowing,
// which does match exactly the semantic of PIXATTR_HIPRI. This simplifies
// mixing code quite a bit.
#define SHI_NORMAL(x)        ((x) | 0x80)
#define SHI_HIGHLIGHT(x)     ((x) | 0x40)
#define SHI_SHADOW(x)        ((x) & 0x3F)

#define SHI_IS_SHADOW(x)     (!((x) & 0x80))
#define SHI_IS_HIGHLIGHT(x)  ((x) & 0x40)

void gwenesis_vdp_reset();
void gwenesis_vdp_set_hblank();
void gwenesis_vdp_clear_hblank();
void gwenesis_vdp_set_vblank();
void gwenesis_vdp_clear_vblank();

unsigned int gwenesis_vdp_get_reg(int reg);
void gwenesis_vdp_set_reg(int reg, unsigned char value);

unsigned int gwenesis_vdp_read_memory_8(unsigned int address);
unsigned int gwenesis_vdp_read_memory_16(unsigned int address);

void gwenesis_vdp_write_memory_8(unsigned int address, unsigned int value);
void gwenesis_vdp_write_memory_16(unsigned int address, unsigned int value);

void gwenesis_vdp_set_buffers(unsigned char *screen_buffer, unsigned char *scaled_buffer);
void gwenesis_vdp_set_buffer(unsigned short *ptr_screen_buffer);
void gwenesis_vdp_render_line(int line);

void gwenesis_vdp_render_config();

unsigned int gwenesis_vdp_get_status();
void gwenesis_vdp_get_debug_status(char *s);
unsigned short gwenesis_vdp_get_cram(int index);
void gwenesis_vdp_get_vram(unsigned char *raw_buffer, int palette);
void gwenesis_vdp_get_vram_raw(unsigned char *raw_buffer);
void gwenesis_vdp_get_cram_raw(unsigned char *raw_buffer);

void gwenesis_vdp_gfx_save_state();
void gwenesis_vdp_gfx_load_state();
void gwenesis_vdp_mem_save_state();
void gwenesis_vdp_mem_load_state();

#endif