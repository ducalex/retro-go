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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "m68k.h"
#include "ym2612.h"
#include "gwenesis_vdp.h"
#include "gwenesis_io.h"
#include "gwenesis_bus.h"
#include "gwenesis_sn76489.h"
#include "gwenesis_savestate.h"

#include <assert.h>

#if GNW_TARGET_MARIO !=0 || GNW_TARGET_ZELDA!=0
  #pragma GCC optimize("Ofast")
#endif

#define VDP_MEM_DISABLE_LOGGING 1

#if !VDP_MEM_DISABLE_LOGGING
#include <stdarg.h>
void vdpm_log(const char *subs, const char *fmt, ...) {
  extern int frame_counter;
  extern int scan_line;

  va_list va;

  printf("%06d:%03d :[%s] vc:%04x hc:%04x hv:%04x ", frame_counter, scan_line, subs,gwenesis_vdp_vcounter(),gwenesis_vdp_hcounter(),gwenesis_vdp_hvcounter());

  va_start(va, fmt);
  vfprintf(stdout, fmt, va);
  va_end(va);
  printf("\n");
}
#else
	#define vdpm_log(...)  do {} while(0)
#endif

//#define _DMA_TRACE_

/* Setup VDP Memories */


#if GNW_TARGET_MARIO != 0 | GNW_TARGET_ZELDA != 0
  extern uint8_t emulator_framebuffer[1024*64];
  unsigned char* VRAM = &emulator_framebuffer[0];
#else
  unsigned char *VRAM;
#endif

unsigned short CRAM[CRAM_MAX_SIZE];           // CRAM - Palettes
unsigned char SAT_CACHE[SAT_CACHE_MAX_SIZE];  // Sprite cache
unsigned char gwenesis_vdp_regs[REG_SIZE];    // Registers
unsigned short fifo[FIFO_SIZE];               // Fifo
unsigned short CRAM565[CRAM_MAX_SIZE * 4];    // CRAM - Palettes
unsigned short VSRAM[VSRAM_MAX_SIZE];         // VSRAM - Scrolling

// Define VDP control code and set initial code
static unsigned char code_reg = 0;
// Define VDP control address and set initial address
static unsigned short address_reg = 0;
// Define VDP control pending and set initial state
int command_word_pending = 0;
// Define VDP status and set initial status value
unsigned short gwenesis_vdp_status = 0x3C00;

extern int scan_line;

// Define DMA
//static unsigned int dma_length;
//static unsigned int dma_source;
// Define and set DMA FILL pending as initial state
 int dma_fill_pending = 0;

// Define HVCounter latch and set initial state
static int hvcounter_latch = 0;
static int hvcounter_latched = 0;

int hint_pending;


// Define VIDEO MODE
extern int mode_pal;

extern int sprite_overflow;
extern bool sprite_collision;

// Store last address r/w
//static unsigned int gwenesis_vdp_laddress_r=0;
//unsigned int gwenesis_vdp_laddress_w=0;

//static int DMA_RUN=0;

// 16 bits access to VRAM
#define FETCH16(A) ( ( (*(unsigned short *)&VRAM[(A)]) >> 8 ) | ( (*(unsigned short *)&VRAM[(A)]) << 8 ) )


/******************************************************************************
 *
 *  SEGA 315-5313 Reset
 *  Clear all volatile memory
 *
 ******************************************************************************/

int m68k_irq_acked(int irq) {

  /* VINT has higher priority (Fatal Rewind) */
  if (REG1_VBLANK_INTERRUPT && (gwenesis_vdp_status & STATUS_VIRQPENDING))
  {
    /* Clear VINT pending flag */
    gwenesis_vdp_status &= ~STATUS_VIRQPENDING;

    if (hint_pending && REG0_LINE_INTERRUPT)
      m68k_set_irq(4);
    else
      m68k_set_irq(0);

  }
  else
  {
  /* Clear HINT pending flag */
  hint_pending = 0;

   /* Update IRQ status */
   m68k_set_irq(0);
  }

  return M68K_INT_ACK_AUTOVECTOR;
}


void gwenesis_vdp_reset() {
  memset(VRAM, 0, VRAM_MAX_SIZE);
  memset(SAT_CACHE, 0, sizeof(SAT_CACHE));
  memset(CRAM, 0, sizeof(CRAM));
  memset(CRAM565, 0, sizeof(CRAM565));
  memset(VSRAM, 0, sizeof(VSRAM));
  memset(gwenesis_vdp_regs, 0, sizeof(gwenesis_vdp_regs));
  command_word_pending = 0;
  address_reg = 0;
  code_reg = 0;
  hint_pending = 0;
  // _vcounter = 0;
  gwenesis_vdp_status = 0x3C00;
  // //line_counter_interrupt = 0;
  hvcounter_latched = 0;

  // register the M68K interrupt
  m68k_set_int_ack_callback(m68k_irq_acked);
}


/******************************************************************************
 *
 *  SEGA 315-5313 HCOUNTER
 *  Process SEGA 315-5313 HCOUNTER based on M68K Cycles
 *
 ******************************************************************************/
//static inline __attribute__((always_inline))
int gwenesis_vdp_hcounter()
{
    int mclk = m68k_cycles_run() ;
    int pixclk;

    // Accurate 9-bit hcounter emulation, from timing posted here:
    // http://gendev.spritesmind.net/forum/viewtopic.php?p=17683#17683
    if (REG12_MODE_H40)
    {
        pixclk = mclk * 420 / VDP_CYCLES_PER_LINE;
        pixclk += 0xD;
        if (pixclk >= 0x16D)
            pixclk += 0x1C9 - 0x16D;
    }
    else
    {
        pixclk = mclk * 342 / VDP_CYCLES_PER_LINE;
        pixclk += 0xB;
        if (pixclk >= 0x128)
            pixclk += 0x1D2 - 0x128;
    }

    return pixclk & 0x1FF;
}

/******************************************************************************
 *
 *  SEGA 315-5313 VCOUNTER
 *  Process SEGA 315-5313 VCOUNTER based on M68K Cycles
 *
 ******************************************************************************/
//static inline __attribute__((always_inline))
int gwenesis_vdp_vcounter()
{

    int vc = scan_line;
    int VERSION_PAL = gwenesis_vdp_status & 1;

    /*
    if (VERSION_PAL && mode_pal && (vc >= 0x10B))
        vc += 0x1D2 - 0x10B;
    else if (VERSION_PAL && (mode_pal==0) && (vc >= 0x103))
        vc += 0x1CA - 0x103;
    else if ((VERSION_PAL ==0 ) && (vc >= 0xEB))
        vc += 0x1E5 - 0xEB;
    assert(vc < 0x200);
    */
    if (VERSION_PAL && mode_pal && (vc >= 267))
        vc = scan_line - 58; 
    else if (VERSION_PAL && (mode_pal==0) && (vc >= 259))
        vc = scan_line  - 42;
    else if ((VERSION_PAL == 0 ) && (vc >= 235))
        vc = scan_line -6;
    assert(vc < 0x200);

   // printf("VERSION_PAL:%d , mode_pal:%d,line:%d,vc:%d\n",VERSION_PAL,mode_pal,scan_line,vc);
    return vc;
}
/******************************************************************************
 *
 *  SEGA 315-5313 HVCOUNTER
 *  Process SEGA 315-5313 HVCOUNTER based on HCOUNTER and VCOUNTER
 *
 ******************************************************************************/
//static inline __attribute__((always_inline))
unsigned short gwenesis_vdp_hvcounter()
{
    /* H/V Counter */
    if (hvcounter_latched == 1)
        return hvcounter_latch;

    int hc = gwenesis_vdp_hcounter();
    int vc = gwenesis_vdp_vcounter();
    assert(vc < 512);
    assert(hc < 512);

    return ((vc & 0xFF) << 8) | (hc >> 1);

}

//static inline __attribute__((always_inline))
bool vblank(void)
{
    int vc = gwenesis_vdp_vcounter();
 //  printf("vc=%d,REG1_DISP_ENABLED=%d,VBLAN?%d\n",vc,REG1_DISP_ENABLED,
  // mode_pal?((vc >= 0xF0) && (vc < 0x1FF)):((vc >= 0xE0) && (vc < 0x1FF)));

    if (REG1_DISP_ENABLED ==0)
        return true;

    if (mode_pal)
        return ((vc >= 0xF0) && (vc < 0x1FF));
    else
        return ((vc >= 0xE0) && (vc < 0x1FF));
        
}

/******************************************************************************
 *
 *   SEGA 315-5313 Set Register
 *   Write an value to specified register
 *
 ******************************************************************************/
static inline __attribute__((always_inline)) void gwenesis_vdp_register_w(int reg, unsigned char value)
{
    // Mode4 is not emulated yet. Anyway, access to registers > 0xA is blocked.
    if ((BIT(gwenesis_vdp_regs[0x1], 2)==0) && reg > 0xA)
        return;

    gwenesis_vdp_regs[reg] = value;
    vdpm_log(__FUNCTION__, "reg:%02d <- %02x", reg, value);


    // Writing a register clear the first command word
    // (see sonic3d intro wrong colors, and vdpfifotesting)
    code_reg &= ~0x3;
    address_reg &= ~0x3FFF;

    switch (reg)
    {
    case 0:

        if (REG0_HVLATCH && (hvcounter_latched == 0))
        {
            hvcounter_latch = gwenesis_vdp_hvcounter();
            hvcounter_latched = 1;
           //printf("HVcounter latched:%x\n",hvcounter_latch);
        }
        else if ((REG0_HVLATCH ==0) && (hvcounter_latched == 1)){
          //printf("HVcounter released\n");
          hvcounter_latched = 0;
        }

        break;
    }
}

/******************************************************************************
 *
 *  Simulate FIFO
 *
 ******************************************************************************/
 static inline __attribute__((always_inline))
void push_fifo(unsigned int value)
{
    fifo[3] = fifo[2];
    fifo[2] = fifo[1];
    fifo[1] = fifo[0];
    fifo[0] = value;
}

/******************************************************************************
 *
 *   SEGA 315-5313 VRAM Write
 *   Write an value to VRAM on specified address
 *
 ******************************************************************************/
static inline __attribute__((always_inline))
void gwenesis_vdp_vram_write(unsigned int address, unsigned int value)
{
  VRAM[address] = value;

  // Update internal SAT Cache
  // used in Castlevania Bloodlines
  if (address >= REG5_SAT_ADDRESS && address < REG5_SAT_ADDRESS + REG5_SAT_SIZE)
    SAT_CACHE[address - REG5_SAT_ADDRESS] = value;
}

static inline __attribute__((always_inline)) 
unsigned short status_register_r(void)
{
    unsigned short status = gwenesis_vdp_status; // & 0xF800;
   // unsigned short status = gwenesis_vdp_status;// & 0xFC00;

    int hc = gwenesis_vdp_hcounter();
   // int vc = gwenesis_vdp_vcounter();

    // TODO: FIFO not emulated
    status |= STATUS_FIFO_EMPTY;

    // VBLANK bit
    if (vblank())
        status |= STATUS_VBLANK;

    // HBLANK bit (see Nemesis doc, as linked in hcounter())
    if (REG12_MODE_H40)
    {
        if (hc < 0xA || hc >= 0x166)
            status |= STATUS_HBLANK;
    }
    else
    {
        if (hc < 0x9 || hc >= 0x126)
            status |= STATUS_HBLANK;
    }

    if (sprite_overflow)
        status |= STATUS_SPRITEOVERFLOW;
    if (sprite_collision)
        status |= STATUS_SPRITECOLLISION;

    if (mode_pal)
       status |= STATUS_PAL;

    // reading the status clears the pending flag for command words
    command_word_pending = 0;

    //gwenesis_vdp_status = status;

   // printf("VDP status read:%04X H?%d V?%d line=%d\n",status, status & STATUS_HBLANK ,status & STATUS_VBLANK,scan_line);
    return status;
}
/******************************************************************************
 *
 *   SEGA 315-5313 Get Register
 *   Read an value from specified register
 *
 ******************************************************************************/
unsigned int gwenesis_vdp_get_reg(int reg)
{
    return gwenesis_vdp_regs[reg];
}

/******************************************************************************
 *
 *   SEGA 315-5313 DMA Fill
 *   DMA process to fill memory
 *
 ******************************************************************************/
static inline __attribute__((always_inline)) 
void gwenesis_vdp_dma_fill(unsigned short value)
{
  //vdpm_log(__FUNCTION__,"@%x len:%x val:%x",REG21_DMA_SRCADDR_LOW,REG19_DMA_LENGTH,value);
  int dma_length = REG19_DMA_LENGTH;

  // This address is not required for fills,
  // but it's still updated by the DMA engine.
  unsigned short src_addr_low = REG21_DMA_SRCADDR_LOW;

  if (dma_length == 0)
    dma_length = 0xFFFF;

    /*
    vdpm_log(__FUNCTION__, "DMA %s fill: dst:%04x, length:%d, increment:%d, value=%02x",
        (code_reg&0xF)==1 ? "VRAM" : ( (code_reg&0xF)==3 ? "CRAM" : "VSRAM"),
        address_reg, dma_length, REG15_DMA_INCREMENT, value>>8);
        */
        
  switch (code_reg & 0xF) {
  case 0x1:
    do {
      gwenesis_vdp_vram_write((address_reg ^ 1) & 0xFFFF, value >> 8);
      address_reg += REG15_DMA_INCREMENT;
      src_addr_low++;
    } while (--dma_length);
    break;
  case 0x3: // undocumented and buggy, see vdpfifotesting
    do {
      CRAM[(address_reg & 0x7f) >> 1] = fifo[3];

      unsigned short pixel;
      // Blue >> 9 << 2
      pixel = (fifo[3] & 0xe00) >> 7;
      // Green  >> 5 << 5 << 3
      pixel |= (fifo[3] & 0x0e0) << 3;
      // Red  >>1 << 11 << 2
      pixel |= (fifo[3] & 0x00e) << 12;

      // pixel_shadow = pixel >> 1;
      // pixel_highlight = pixel_shadow | 0x8410;

      // Normal pixel values when SHI is not enabled
      CRAM565[(address_reg & 0x7f) >> 1] = pixel;
      CRAM565[0x40 + ((address_reg & 0x7f) >> 1)] = pixel;
      CRAM565[0x80 + ((address_reg & 0x7f) >> 1)] = pixel;
      CRAM565[0xC0 + ((address_reg & 0x7f) >> 1)] = pixel;

      address_reg += REG15_DMA_INCREMENT;
      src_addr_low++;
    } while (--dma_length);
    break;
  case 0x5: // undocumented and buggy, see vdpfifotesting:
    do {
      VSRAM[(address_reg & 0x7f) >> 1] = fifo[3] & 0x03FF;
      address_reg += REG15_DMA_INCREMENT;
      src_addr_low++;
    } while (--dma_length);
    break;
  default:
    printf("Invalid code during DMA fill\n");
  }


  // Clear DMA length at the end of transfer
  gwenesis_vdp_regs[19] = gwenesis_vdp_regs[20] = 0;

  // Update DMA source address after end of transfer
  gwenesis_vdp_regs[21] = src_addr_low & 0xFF;
  gwenesis_vdp_regs[22] = src_addr_low >> 8;

  // gwenesis_vdp_regs[21] = src_addr_low >> 1 & 0xFF;
  // gwenesis_vdp_regs[22] = src_addr_low >> 9 & 0xFF;
  // gwenesis_vdp_regs[23] = src_addr_low >> 17 & 0xFF;

}

/******************************************************************************
 *
 *   SEGA 315-5313 DMA M68K
 *   DMA process to copy from m68k to memory
 *
 ******************************************************************************/
static inline __attribute__((always_inline)) 
void gwenesis_vdp_dma_m68k()
{

    int dma_length = REG19_DMA_LENGTH;

    // This address is not required for fills,
    // but it's still updated by the DMA engine.
    unsigned short src_addr_low = REG21_DMA_SRCADDR_LOW;
    unsigned int src_addr_high = REG23_DMA_SRCADDR_HIGH;
    unsigned int src_addr = (src_addr_high | src_addr_low) << 1;
    unsigned int value;

    if (dma_length == 0)
        dma_length = 0xFFFF;

    /*
    vdpm_log(__FUNCTION__,"DMA M68k->%s copy: src:%04x, dst:%04x, length:%d, increment:%d",
        (code_reg&0xF)==1 ? "VRAM" : ( (code_reg&0xF)==3 ? "CRAM" : "VSRAM"),
        (src_addr_high | src_addr_low) << 1, address_reg, dma_length, REG15_DMA_INCREMENT);
    */

    /* Source is : 
        68K_RAM if dma_source_high == 0x00FF : FETCH16RAM(dma_source_low << 1)
        68K_ROM otherwise                    : FETCH16ROM((dma_source_high | dma_source_low) << 1))
    */

    /* Source is 68K RAM */
    if ( src_addr & 0x800000) {

      switch (code_reg & 0xF) {

      case 0x1: // dest is VRAM
        do {
          value = FETCH16RAM( src_addr );
          push_fifo(value);
          gwenesis_vdp_vram_write((address_reg)&0xFFFF, value >> 8);
          gwenesis_vdp_vram_write((address_reg ^ 1) & 0xFFFF, value & 0xFF);
          address_reg += REG15_DMA_INCREMENT;
          src_addr += 2;
        } while (--dma_length);
        break;

      case 0x3: // dest is CRAM
        do {
          value = FETCH16RAM( src_addr );
          push_fifo(value);
          CRAM[(address_reg & 0x7f) >> 1] = value;

          unsigned short pixel;
         // unsigned short pixel_shadow, pixel_highlight;
          // Blue >> 9 << 2
          pixel = (value & 0xe00) >> 7;
          // Green  >> 5 << 5 << 3
          pixel |= (value & 0x0e0) << 3;
          // Red  >>1 << 11 << 2
          pixel |= (value & 0x00e) << 12;

          // Normal pixel values when SHI is not enabled
          // add mirror 0x80 when high priority flag is set
          CRAM565[(address_reg & 0x7f) >> 1] = pixel;
          CRAM565[0x40 + ((address_reg & 0x7f) >> 1)] = pixel;
          CRAM565[0x80 + ((address_reg & 0x7f) >> 1)] = pixel;
          CRAM565[0xC0 + ((address_reg & 0x7f) >> 1)] = pixel;

          address_reg += REG15_DMA_INCREMENT;
          src_addr += 2;
        } while (--dma_length);
        break;

      case 0x5: // dest is VSRAM

        do {
          value = FETCH16RAM( src_addr );
          push_fifo(value);
          VSRAM[(address_reg & 0x7f) >> 1] = value & 0x03FF;
          address_reg += REG15_DMA_INCREMENT;
          src_addr += 2;
        } while (--dma_length);
        break;
      default: // dest in unknown
        break;
      }

    /* source is 68K ROM */
    } else {

     // unsigned int dma_source_address = (dma_source_high | dma_source_low) << 1; 

      switch (code_reg & 0xF) {

      case 0x1: // dest is VRAM

        do {
          value = FETCH16ROM(src_addr);
          push_fifo(value);
          gwenesis_vdp_vram_write((address_reg)&0xFFFF, value >> 8);
          gwenesis_vdp_vram_write((address_reg ^ 1) & 0xFFFF, value & 0xFF);
          address_reg += REG15_DMA_INCREMENT;
          src_addr += 2;
        } while (--dma_length);
        break;

      case 0x3: // dest is CRAM

        do {
          value = FETCH16ROM(src_addr);
          push_fifo(value);
          CRAM[(address_reg & 0x7f) >> 1] = value;

          unsigned short pixel;
       //   unsigned short pixel_shadow, pixel_highlight;
          // Blue >> 9 << 2
          pixel = (value & 0xe00) >> 7;
          // Green  >> 5 << 5 << 3
          pixel |= (value & 0x0e0) << 3;
          // Red  >>1 << 11 << 2
          pixel |= (value & 0x00e) << 12;

        //   pixel_shadow = pixel >> 1;
        //   pixel_highlight = pixel_shadow | 0x8410;

          // Normal pixel values when SHI is not enabled
          // add mirror 0x80 when high priority flag is set
          CRAM565[(address_reg & 0x7f) >> 1] = pixel;
          CRAM565[0x40 + ((address_reg & 0x7f) >> 1)] = pixel;
          CRAM565[0x80 + ((address_reg & 0x7f) >> 1)] = pixel;
          CRAM565[0xC0 + ((address_reg & 0x7f) >> 1)] = pixel;

          address_reg += REG15_DMA_INCREMENT;
          src_addr += 2;
        } while (--dma_length);
        break;

      case 0x5: // dest is VSRAM

        do {
          value = FETCH16ROM(src_addr);
          push_fifo(value);
          VSRAM[(address_reg & 0x7f) >> 1] = value & 0x03FF;
          address_reg += REG15_DMA_INCREMENT;
          src_addr += 2;
        } while (--dma_length);
        break;
      default: // dest in unknown
        break;
      }

    }

    // Update DMA source address after end of transfer
    gwenesis_vdp_regs[21] = src_addr & 0xFF; //src_addr_low & 0xFF;
    gwenesis_vdp_regs[22] = (src_addr >> 8 ) & 0xFF; //src_addr_low >> 8;

    // Clear DMA length at the end of transfer
    gwenesis_vdp_regs[19] = gwenesis_vdp_regs[20] = 0;

}
/******************************************************************************
 *
 *   SEGA 315-5313 DMA Copy
 *   DMA process to copy from memory to memory
 *
 ******************************************************************************/
static inline __attribute__((always_inline))
void gwenesis_vdp_dma_copy()
{
   // DMA_RUN=1;

    int dma_length = REG19_DMA_LENGTH;
    unsigned short src_addr_low = REG21_DMA_SRCADDR_LOW;
    //vdpm_log(__FUNCTION__,"length:%x src:%x",dma_length,src_addr_low);

    do
    {
        unsigned short value = VRAM[src_addr_low ^ 1];
        gwenesis_vdp_vram_write((address_reg ^ 1) & 0xFFFF, value);

        address_reg += REG15_DMA_INCREMENT;
        src_addr_low++;
    } while (--dma_length);

    // Update DMA source address after end of transfer
    gwenesis_vdp_regs[21] = src_addr_low & 0xFF;
    gwenesis_vdp_regs[22] = src_addr_low >> 8;

    // Clear DMA length at the end of transfer
    gwenesis_vdp_regs[19] = gwenesis_vdp_regs[20] = 0;

}

/******************************************************************************
 *
 *   SEGA 315-5313 read data R16
 *   Read an data value from mapped memory on specified address
 *   and return as word
 *
 ******************************************************************************/
static inline __attribute__((always_inline))
unsigned int gwenesis_vdp_read_data_port_16()
{
    enum
    {
        CRAM_BITMASK = 0x0EEE,
        VSRAM_BITMASK = 0x07FF,
        VRAM8_BITMASK = 0x00FF
    };
    unsigned int value;
    command_word_pending = 0;

    //if (code_reg & 1) /* check if write is set */
   // {
        switch (code_reg & 0xF)
        {
        case 0x0:
            // No byteswapping here
            value = VRAM[(address_reg)&0xFFFE] << 8;
            value |= VRAM[(address_reg | 1) & 0xFFFF];
            address_reg += REG15_DMA_INCREMENT;
            address_reg &= 0xFFFF;
            //vdpm_log(__FUNCTION__,"%04x",value);

            return value;
        case 0x4:
            if (((address_reg & 0x7f) >> 1) >= 0x28)
                value = VSRAM[0];
            else
                value = VSRAM[(address_reg & 0x7f) >> 1];
            value = (value & VSRAM_BITMASK) | (fifo[3] & ~VSRAM_BITMASK);
            address_reg += REG15_DMA_INCREMENT;
            address_reg &= 0x7F;
            // vdpm_log(__FUNCTION__,"%04x",value);

            return value;
        case 0x8:
            value = CRAM[(address_reg & 0x7f) >> 1];
            value = (value & CRAM_BITMASK) | (fifo[3] & ~CRAM_BITMASK);
            address_reg += REG15_DMA_INCREMENT;
            address_reg &= 0x7F;
            // vdpm_log(__FUNCTION__,"%04x",value);

            return value;
        case 0xC: /* 8-Bit memory access */
            value = VRAM[(address_reg ^ 1) & 0xFFFF];
            value = (value & VRAM8_BITMASK) | (fifo[3] & ~VRAM8_BITMASK);
            address_reg += REG15_DMA_INCREMENT;
            address_reg &= 0xFFFF;
            // vdpm_log(__FUNCTION__,"%04x",value);

            return value;
        default:
            printf("unhandled gwenesis_vdp_read_data_port_16(%x)\n", address_reg);
            return 0xFF;
        }
   // }
  //  return 0x00;
}


/******************************************************************************
 *
 *   SEGA 315-5313 write to control port
 *   Write an control value to SEGA 315-5313 control port
 *
 ******************************************************************************/
static inline __attribute__((always_inline))
void gwenesis_vdp_control_port_write(unsigned int value)
{
    //vdpm_log(__FUNCTION__,"%04x",value);

  if (command_word_pending == 1) {
    // second half of the command word
    code_reg &= ~0x3C;
    code_reg |= (value >> 2) & 0x3C;
    address_reg &= 0x3FFF;
    address_reg |= value << 14;
    command_word_pending = 0;
    //vdpm_log(__FUNCTION__,"command word 2nd code:%x address:%x", code_reg, address_reg);


    // DMA trigger
    if (code_reg & (1<<5)) {

      // Check master DMA enable, otherwise skip
      if (REG1_DMA_ENABLED == 0)
        return;

      // gwenesis_vdp_status |= 0x2;
      switch (REG23_DMA_TYPE) {
      case 0:
      case 1:

        gwenesis_vdp_dma_m68k();
        break;

      case 2:

        // VRAM fill will trigger on next data port write
        dma_fill_pending = 1;
        break;

      case 3:

        gwenesis_vdp_dma_copy();
        break;
      }
    }
    return;
  }
      if ((value >> 14) == 2) {

    gwenesis_vdp_register_w((value >> 8) & 0x1F, value & 0xFF);
    return;
  }

  // Anything else is treated as first half of the command word
  // We directly update the code reg and address reg
  code_reg &= ~0x3;
  code_reg |= value >> 14;
  address_reg &= ~0x3FFF;
  address_reg |= value & 0x3FFF;
  command_word_pending = 1;
 // vdpm_log(__FUNCTION__,"command word 1st code:%x address:%x", code_reg, address_reg);
}

/******************************************************************************
 *
 *   SEGA 315-5313 write data W16
 *   Write an data value to mapped memory on specified address
 *
 ******************************************************************************/
static inline __attribute__((always_inline))
void gwenesis_vdp_write_data_port_16(unsigned int value)
{
      vdpm_log(__FUNCTION__,"%04x",value);

    command_word_pending = 0;

    push_fifo(value);

        switch (code_reg & 0xF)
        {
        case 0x1: /* VRAM write */
            //vdpm_log(__FUNCTION__,"VRAM write : addr:%x increment:%d value:%04x",
             // address_reg, REG15_DMA_INCREMENT, value);
            gwenesis_vdp_vram_write(address_reg& 0xFFFF, (value >> 8) & 0xFF);
            gwenesis_vdp_vram_write((address_reg^1)& 0xFFFF, (value)&0xFF);
            address_reg += REG15_DMA_INCREMENT;
            address_reg &= 0xFFFF;

            break;
        case 0x3: /* CRAM write */
            //vdpm_log(__FUNCTION__,"CRAM write : addr:%x increment:%d value:%04x",
             // address_reg, REG15_DMA_INCREMENT, value);
            CRAM[(address_reg & 0x7f) >> 1] = value;

            unsigned short pixel;
            //unsigned short pixel_shadow,pixel_highlight;

            // Blue >> 9 << 2
            pixel = (value & 0xe00) >> 7; 
            // Green  >> 5 << 5 << 3
            pixel |= (value & 0x0e0) << 3;
            // Red  >>1 << 11 << 2
            pixel |= (value & 0x00e) << 12;

            // Normal pixel values when SHI is not enabled
            CRAM565[(address_reg & 0x7f) >> 1] = pixel;
            CRAM565[0x40 + ((address_reg & 0x7f) >> 1)] = pixel;
            CRAM565[0x80 + ((address_reg & 0x7f) >> 1)] = pixel;
            CRAM565[0xC0 + ((address_reg & 0x7f) >> 1)] = pixel;

            address_reg += REG15_DMA_INCREMENT;
            address_reg &= 0xFFFF;

            break;
        case 0x5: /* VSRAM write */
            //vdpm_log(__FUNCTION__,"VSRAM write : addr:%x increment:%d value:%04x",
            //  address_reg, REG15_DMA_INCREMENT, value);
           // printf("write dataport 16: VSRAM@%04x:%04x\n",address_reg,value);
            VSRAM[(address_reg & 0x7f) >> 1] = value & 0X03FF;
            address_reg += REG15_DMA_INCREMENT;
            address_reg &= 0xFFFF;
            break;
        case 0x0:
        case 0x4:
        case 0x8: // Write operation after setting up
                  // Makes Compatible with Alladin and Ecco 2
            break;
        case 0x9: // VDP FIFO TEST
            break;
        default:
            printf("VDP Data Port invalid");
        }

    /* if a DMA is scheduled, do it */
    if (dma_fill_pending)
    {
        dma_fill_pending = 0;
        gwenesis_vdp_dma_fill(value);
        return;
    }
}

/******************************************************************************
 *
 *   SEGA 315-5313 Get Status
 *   Return current VDP Status
 *
 ******************************************************************************/
unsigned int gwenesis_vdp_get_status()
{
    return gwenesis_vdp_status;
}


/******************************************************************************
 *
 *   SEGA 315-5313 read from memory R8
 *   Read an value from mapped memory on specified address
 *   and return as byte
 *
 ******************************************************************************/
 //static inline 
unsigned int gwenesis_vdp_read_memory_8(unsigned int address)
{
    unsigned int ret = gwenesis_vdp_read_memory_16(address & ~1);
    if (address & 1)
        return ret & 0xFF;

   // vdpm_log(__FUNCTION__,"%04x : %04x",address,ret);

    return ret >> 8;
}

/******************************************************************************
 *
 *   SEGA 315-5313 read from memory R16
 *   Read an value from mapped memory on specified address
 *   and return as word
 *
 ******************************************************************************/
 //static inline 
unsigned int gwenesis_vdp_read_memory_16(unsigned int address)
{
    
    address &= 0x1F;
    
    if (address < 0X4)
      return gwenesis_vdp_read_data_port_16();
    else if (address < 0x8)
      return status_register_r();
    else if (address < 0xf)
      return gwenesis_vdp_hvcounter();
    else 
      return 0xff;

}

/******************************************************************************
 *
 *   SEGA 315-5313 write to memory W8
 *   Write an byte value to mapped memory on specified address
 *
 ******************************************************************************/
 //static inline 
void gwenesis_vdp_write_memory_8(unsigned int address, unsigned int value)
{
  gwenesis_vdp_write_memory_16(address & ~1, (value << 8) | value);

}

/******************************************************************************
 *
 *   SEGA 315-5313 write to memory W16
 *   Write an word value to mapped memory on specified address
 *
 ******************************************************************************/
 //static inline
 extern int system_clock;
void gwenesis_vdp_write_memory_16(unsigned int address, unsigned int value) {
  address = address & 0x1F;

  if (address < 0x4) {
    gwenesis_vdp_write_data_port_16(value);
    return;
  }
  if (address < 0x8) {
    gwenesis_vdp_control_port_write(value);
    return;
  }
  if (address < 0x18) { // PSG 8 bits write
      vdpm_log(__FUNCTION__,"PSG sclk=%d,mclk=%d", system_clock,  m68k_cycles_master());
    gwenesis_SN76489_Write(value, m68k_cycles_master());

    return;
  }
  // UNHANDLED
  printf("unhandled gwenesis_vdp_write(%x, %x)\n", address, value);

}

void gwenesis_vdp_mem_save_state() {
  SaveState* state;
  state = saveGwenesisStateOpenForWrite("vdp_mem");

  saveGwenesisStateSetBuffer(state, "VRAM", VRAM, VRAM_MAX_SIZE);
  saveGwenesisStateSetBuffer(state, "CRAM", CRAM, sizeof(CRAM));
  saveGwenesisStateSetBuffer(state, "SAT_CACHE", SAT_CACHE, sizeof(SAT_CACHE));
  saveGwenesisStateSetBuffer(state, "gwenesis_vdp_regs", gwenesis_vdp_regs, sizeof(gwenesis_vdp_regs));
  saveGwenesisStateSetBuffer(state, "fifo", fifo, sizeof(fifo));
  saveGwenesisStateSetBuffer(state, "CRAM565", CRAM565, sizeof(CRAM565));
  saveGwenesisStateSetBuffer(state, "VSRAM", VSRAM, sizeof(VSRAM));
  saveGwenesisStateSet(state, "code_reg", code_reg);
  saveGwenesisStateSet(state, "address_reg", address_reg);
  saveGwenesisStateSet(state, "command_word_pending", command_word_pending);
  saveGwenesisStateSet(state, "gwenesis_vdp_status", gwenesis_vdp_status);
  saveGwenesisStateSet(state, "dma_fill_pending", dma_fill_pending);
  saveGwenesisStateSet(state, "hvcounter_latch", hvcounter_latch);
  saveGwenesisStateSet(state, "hvcounter_latched", hvcounter_latched);
  saveGwenesisStateSet(state, "hint_pending", hint_pending);
}

void gwenesis_vdp_mem_load_state() {
  SaveState* state = saveGwenesisStateOpenForRead("vdp_mem");

  saveGwenesisStateGetBuffer(state, "VRAM", VRAM, VRAM_MAX_SIZE);
  saveGwenesisStateGetBuffer(state, "CRAM", CRAM, sizeof(CRAM));
  saveGwenesisStateGetBuffer(state, "SAT_CACHE", SAT_CACHE, sizeof(SAT_CACHE));
  saveGwenesisStateGetBuffer(state, "gwenesis_vdp_regs", gwenesis_vdp_regs, sizeof(gwenesis_vdp_regs));
  saveGwenesisStateGetBuffer(state, "fifo", fifo, sizeof(fifo));
  saveGwenesisStateGetBuffer(state, "CRAM565", CRAM565, sizeof(CRAM565));
  saveGwenesisStateGetBuffer(state, "VSRAM", VSRAM, sizeof(VSRAM));
  code_reg = saveGwenesisStateGet(state, "code_reg");
  address_reg = saveGwenesisStateGet(state, "address_reg");
  command_word_pending = saveGwenesisStateGet(state, "command_word_pending");
  gwenesis_vdp_status = saveGwenesisStateGet(state, "gwenesis_vdp_status");
  dma_fill_pending = saveGwenesisStateGet(state, "dma_fill_pending");
  hvcounter_latch = saveGwenesisStateGet(state, "hvcounter_latch");
  hvcounter_latched = saveGwenesisStateGet(state, "hvcounter_latched");
  hint_pending = saveGwenesisStateGet(state, "hint_pending");
}