/* This file is part of Snes9x. See LICENSE file. */

#ifndef _MISSING_H_
#define _MISSING_H_

struct HDMA
{
   uint8_t  used;
   uint8_t  bbus_address;
   uint8_t  abus_bank;
   uint16_t abus_address;
   uint8_t  indirect_address;
   uint8_t  force_table_address_write;
   uint8_t  force_table_address_read;
   uint8_t  line_count_write;
   uint8_t  line_count_read;
};

struct Missing
{
   uint8_t     emulate6502;
   uint8_t     decimal_mode;
   uint8_t     mv_8bit_index;
   uint8_t     mv_8bit_acc;
   uint8_t     interlace;
   uint8_t     lines_239;
   uint8_t     pseudo_512;
   struct HDMA hdma [8];
   uint8_t     modes [8];
   uint8_t     mode7_fx;
   uint8_t     mode7_flip;
   uint8_t     mode7_bgmode;
   uint8_t     direct;
   uint8_t     matrix_multiply;
   uint8_t     oam_read;
   uint8_t     vram_read;
   uint8_t     cgram_read;
   uint8_t     wram_read;
   uint8_t     dma_read;
   uint8_t     vram_inc;
   uint8_t     vram_full_graphic_inc;
   uint8_t     virq;
   uint8_t     hirq;
   uint16_t    virq_pos;
   uint16_t    hirq_pos;
   uint8_t     h_v_latch;
   uint8_t     h_counter_read;
   uint8_t     v_counter_read;
   uint8_t     fast_rom;
   uint8_t     window1 [6];
   uint8_t     window2 [6];
   uint8_t     sprite_priority_rotation;
   uint8_t     subscreen;
   uint8_t     subscreen_add;
   uint8_t     subscreen_sub;
   uint8_t     fixed_colour_add;
   uint8_t     fixed_colour_sub;
   uint8_t     mosaic;
   uint8_t     sprite_double_height;
   uint8_t     dma_channels;
   uint8_t     dma_this_frame;
   uint8_t     oam_address_read;
   uint8_t     bg_offset_read;
   uint8_t     matrix_read;
   uint8_t     hdma_channels;
   uint8_t     hdma_this_frame;
   uint16_t    unknownppu_read;
   uint16_t    unknownppu_write;
   uint16_t    unknowncpu_read;
   uint16_t    unknowncpu_write;
   uint16_t    unknowndsp_read;
   uint16_t    unknowndsp_write;
};

struct Missing missing;
#endif
