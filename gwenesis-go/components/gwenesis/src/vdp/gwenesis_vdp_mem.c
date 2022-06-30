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
#include "gwenesis_savestate.h"

#include <assert.h>

// #pragma GCC optimize("Ofast")
//#define _DMA_TRACE_

/* Setup VDP Memories */

//#define _VDPMEM_TRACE_
#ifdef _HOST_
    unsigned char VRAM[VRAM_MAX_SIZE];
#else
    //extern uint8_t emulator_framebuffer[1024*64];
    //unsigned char* VRAM = &emulator_framebuffer[0];
    //unsigned char VRAM[VRAM_MAX_SIZE] __attribute__((section("._dtcram")));
    extern unsigned char *VRAM;
#endif

unsigned short CRAM[CRAM_MAX_SIZE];
    //__attribute__((section("._dtcram"))); // CRAM - Palettes
unsigned char SAT_CACHE[SAT_CACHE_MAX_SIZE] ;
    //__attribute__((section("._dtcram"))); // Sprite cache
unsigned char gwenesis_vdp_regs[REG_SIZE];
    //__attribute__((section("._dtcram"))); // Registers
unsigned short fifo[FIFO_SIZE];           // Fifo

unsigned short CRAM565[CRAM_MAX_SIZE * 4]; //__attribute__((section("._dtcram"))); // CRAM - Palettes

unsigned short VSRAM[VSRAM_MAX_SIZE]; // __attribute__((section("._dtcram"))); // VSRAM - Scrolling

// Define VDP control code and set initial code
static int code_reg = 0;
// Define VDP control address and set initial address
static unsigned int address_reg = 0;
// Define VDP control pending and set initial state
int command_word_pending = 0;
// Define VDP status and set initial status value
unsigned int gwenesis_vdp_status = 0x3C00;

extern int scan_line;

// Define DMA
//static unsigned int dma_length;
//static unsigned int dma_source;
// Define and set DMA FILL pending as initial state
static int dma_fill_pending = 0;

// Define HVCounter latch and set initial state
static unsigned int hvcounter_latch = 0;
static int hvcounter_latched = 0;

// Define VIDEO MODE
extern uint8_t mode_pal;

extern int sprite_overflow;
extern bool sprite_collision;

// Store last address r/w
//static unsigned int gwenesis_vdp_laddress_r=0;
//unsigned int gwenesis_vdp_laddress_w=0;

//static int DMA_RUN=0;

// 16 bits acces to VRAM
#if 0
#ifdef _HOST_
#define FETCH16(A)  ( (VRAM[(A)+1]) | (VRAM[(A)] << 8) )
#else
#include "main.h"
//static inline unsigned int FETCH16(unsigned int A)  { return (__REV16( *(unsigned short *)&VRAM[A])); }

#define FETCH16(unsigned int A)  { ( *(unsigned short *)&VRAM[(A)]) >> 8 | *(unsigned short *)&VRAM[(A)]) << 8 }
#endif

#endif

#define FETCH16(A) ( ( (*(unsigned short *)&VRAM[(A)]) >> 8 ) | ( (*(unsigned short *)&VRAM[(A)]) << 8 ) )


/******************************************************************************
 *
 *  SEGA 315-5313 Reset
 *  Clear all volatile memory
 *
 ******************************************************************************/
int hint_pending;

int m68k_irq_acked(int irq) {
  if (irq == 6) {
  //  printf("ACK IRQ6_%d\n", scan_line);
    gwenesis_vdp_status &= ~STATUS_VIRQPENDING;
    return hint_pending ? 4 : 0;
  } else if (irq == 4) {
   // printf("ACK IRQ4_%d\n", scan_line);

    hint_pending = 0;
    return 0;
  }
  assert(0);
}

void gwenesis_vdp_reset() {
  memset(VRAM, 0, VRAM_MAX_SIZE);
  memset(SAT_CACHE, 0, sizeof(SAT_CACHE));
  memset(CRAM, 0, sizeof(CRAM));
  memset(CRAM565, 0, sizeof(CRAM565));
  //  memset(CRAM565SHI, 0, sizeof(CRAM565SHI));
  memset(VSRAM, 0, sizeof(VSRAM));
  memset(gwenesis_vdp_regs, 0, sizeof(gwenesis_vdp_regs));
  command_word_pending = false;
  address_reg = 0;
  code_reg = 0;
  hint_pending = 0;
  // _vcounter = 0;
  gwenesis_vdp_status = 0x3C00;
  // //line_counter_interrupt = 0;
  hvcounter_latched = false;

  // register the M68K interrupt
  m68k_set_int_ack_callback(m68k_irq_acked);
}

unsigned int get_cycle_counter()
{
    return m68k_cycles_run();
}

/******************************************************************************
 *
 *  SEGA 315-5313 HCOUNTER
 *  Process SEGA 315-5313 HCOUNTER based on M68K Cycles
 *
 ******************************************************************************/
static inline __attribute__((always_inline))
int gwenesis_vdp_hcounter()
{
    int mclk = m68k_cycles_run() * M68K_FREQ_DIVISOR;
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
static inline __attribute__((always_inline))
int gwenesis_vdp_vcounter()
{

    int vc = scan_line;
    int VERSION_PAL = gwenesis_vdp_status & 1;

    if (VERSION_PAL && mode_pal && (vc >= 0x10B))
        vc += 0x1D2 - 0x10B;
    else if (VERSION_PAL && !mode_pal && (vc >= 0x103))
        vc += 0x1CA - 0x103;
    else if (!VERSION_PAL && (vc >= 0xEB))
        vc += 0x1E5 - 0xEB;
    assert(vc < 0x200);
    return vc;
}
/******************************************************************************
 *
 *  SEGA 315-5313 HVCOUNTER
 *  Process SEGA 315-5313 HVCOUNTER based on HCOUNTER and VCOUNTER
 *
 ******************************************************************************/
static inline __attribute__((always_inline))
unsigned int gwenesis_vdp_hvcounter()
{
    /* H/V Counter */
    if (hvcounter_latched)
        return hvcounter_latch;

    int hc = gwenesis_vdp_hcounter();
    int vc = gwenesis_vdp_vcounter();
    assert(vc < 512);
    assert(hc < 512);

    return ((vc & 0xFF) << 8) | (hc >> 1);

}

static inline __attribute__((always_inline))
bool vblank(void)
{
    int vc = gwenesis_vdp_vcounter();
   // printf("vc=%d",vc);

    if (!REG1_DISP_ENABLED)
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
    if (!BIT(gwenesis_vdp_regs[0x1], 2) && reg > 0xA)
        return;

    gwenesis_vdp_regs[reg] = value;

    // Writing a register clear the first command word
    // (see sonic3d intro wrong colors, and vdpfifotesting)
    code_reg &= ~0x3;
    address_reg &= ~0x3FFF;

    switch (reg)
    {
    case 0:

        if (REG0_HVLATCH && !hvcounter_latched)
        {
            hvcounter_latch = gwenesis_vdp_hvcounter();
            hvcounter_latched = 1;
         //   printf("HVcounter latched:%x\n",hvcounter_latch);
        }
        else if (!REG0_HVLATCH && hvcounter_latched){
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
  //  unsigned short status = 0X3400; // gwenesis_vdp_status & 0xFC00;
    unsigned short status = gwenesis_vdp_status;// & 0xFC00;

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

    //printf("VDP status read:%04X %d %d %d\n",status, status & STATUS_HBLANK ,status & STATUS_VBLANK,scan_line);
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
void gwenesis_vdp_dma_fill(unsigned int value)
{
  int dma_length = REG19_DMA_LENGTH;
  // DMA_RUN=1;
  // This address is not required for fills,
  // but it's still updated by the DMA engine.
  unsigned short src_addr_low = REG21_DMA_SRCADDR_LOW;

  if (dma_length == 0)
    dma_length = 0xFFFF;

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
  gwenesis_vdp_regs[22] = src_addr_low & 0xFF;

  // gwenesis_vdp_regs[21] = src_addr_low >> 1 & 0xFF;
  // gwenesis_vdp_regs[22] = src_addr_low >> 9 & 0xFF;
  // gwenesis_vdp_regs[23] = src_addr_low >> 17 & 0xFF;

  //  DMA_RUN=0;
  // gwenesis_vdp_status &=~0x2;
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
   // DMA_RUN=1;

    // This address is not required for fills,
    // but it's still updated by the DMA engine.
    unsigned short src_addr_low = REG21_DMA_SRCADDR_LOW;
    unsigned int src_addr_high = REG23_DMA_SRCADDR_HIGH;
    unsigned int src_addr = (src_addr_high | src_addr_low) << 1;
    unsigned int value;

    if (dma_length == 0)
        dma_length = 0xFFFF;

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

  //  DMA_RUN=0;
   // gwenesis_vdp_status &=~0x2;

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
    unsigned int src_addr_low = REG21_DMA_SRCADDR_LOW;

    do
    {
        unsigned int value = VRAM[src_addr_low ^ 1];
        gwenesis_vdp_vram_write((address_reg ^ 1) & 0xFFFF, value);

        address_reg += REG15_DMA_INCREMENT;
        src_addr_low++;
    } while (--dma_length);

    // Update DMA source address after end of transfer
    gwenesis_vdp_regs[21] = src_addr_low & 0xFF;
    gwenesis_vdp_regs[22] = src_addr_low >> 8;

    // Clear DMA length at the end of transfer
    gwenesis_vdp_regs[19] = gwenesis_vdp_regs[20] = 0;
   //    #ifdef _DMA_TRACE_
   // printf("DMA MEM>MEM End\n");
   // #endif
  //  DMA_RUN=0;
  //  gwenesis_vdp_status &=~0x2;
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
            return value;
        case 0x4:
            if (((address_reg & 0x7f) >> 1) >= 0x28)
                value = VSRAM[0];
            else
                value = VSRAM[(address_reg & 0x7f) >> 1];
            value = (value & VSRAM_BITMASK) | (fifo[3] & ~VSRAM_BITMASK);
            address_reg += REG15_DMA_INCREMENT;
            address_reg &= 0x7F;
            return value;
        case 0x8:
            value = CRAM[(address_reg & 0x7f) >> 1];
            value = (value & CRAM_BITMASK) | (fifo[3] & ~CRAM_BITMASK);
            address_reg += REG15_DMA_INCREMENT;
            address_reg &= 0x7F;
            return value;
        case 0xC: /* 8-Bit memory access */
            value = VRAM[(address_reg ^ 1) & 0xFFFF];
            value = (value & VRAM8_BITMASK) | (fifo[3] & ~VRAM8_BITMASK);
            address_reg += REG15_DMA_INCREMENT;
            address_reg &= 0xFFFF;
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

  if (command_word_pending) {
    // second half of the command word
    code_reg &= ~0x3C;
    code_reg |= (value >> 2) & 0x3C;
    address_reg &= 0x3FFF;
    address_reg |= value << 14;
    command_word_pending = false;

    // DMA trigger
    if ((code_reg & 0x20)) {
      #ifdef _VDPMEM_TRACE_
      printf("DMA trigger %x %x\n", REG1_DMA_ENABLED, REG23_DMA_TYPE);
       #endif
      // gwenesis_vdp_dma_trigger();
      // Check master DMA enable, otherwise skip
      if (!REG1_DMA_ENABLED)
        return;

      // gwenesis_vdp_status |= 0x2;
      switch (REG23_DMA_TYPE) {
      case 0:
      case 1:
            #ifdef _VDPMEM_TRACE_
        printf("DMA trigger 68K\n");
        #endif
        gwenesis_vdp_dma_m68k();
        break;

      case 2:
            #ifdef _VDPMEM_TRACE_
        printf("DMA trigger fill\n");
        #endif

        // VRAM fill will trigger on next data port write
        dma_fill_pending = 1;
        break;

      case 3:
            #ifdef _VDPMEM_TRACE_
        printf("DMA trigger copy\n");
        #endif

        gwenesis_vdp_dma_copy();
        break;
      }
    }
    return;
  }
  if ((value & 0xc000) == 0x8000) {
          #ifdef _VDPMEM_TRACE_
    printf("register write %x:%x\n", (value >> 8) & 0x1F, value & 0xFF);
    #endif
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
        #ifdef _VDPMEM_TRACE_
  printf("command word code:%x address:%x \n", code_reg, address_reg);
  #endif
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
    command_word_pending = 0;

    push_fifo(value);
 //   if (code_reg & 1) /* check if write is set */
    //{
        switch (code_reg & 0xF)
        {
        case 0x1: /* VRAM write */
              #ifdef _VDPMEM_TRACE_
            printf("VRAM write\n");
            #endif
            gwenesis_vdp_vram_write(address_reg& 0xFFFF, (value >> 8) & 0xFF);
            gwenesis_vdp_vram_write((address_reg^1)& 0xFFFF, (value)&0xFF);
            address_reg += REG15_DMA_INCREMENT;
            address_reg &= 0xFFFF;

            break;
        case 0x3: /* CRAM write */
              #ifdef _VDPMEM_TRACE_
            printf("CRAM write\n");
            #endif
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
              #ifdef _VDPMEM_TRACE_
           printf("VSRAM write\n");
           #endif
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
  //  }
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
 *   SEGA 315-5313 DMA Trigger
 *   Select and Enable correct DMA process when triggered
 *
 ******************************************************************************/
//  static inline 
// void gwenesis_vdp_dma_trigger()
// {
//     // Check master DMA enable, otherwise skip
//     if (!REG1_DMA_ENABLED)
//         return;
//     gwenesis_vdp_status |=0x2;
//     switch (REG23_DMA_TYPE)
//     {
//     case 0:
//     case 1:
//         gwenesis_vdp_dma_m68k();
//         break;

//     case 2:
//         // VRAM fill will trigger on next data port write
//         dma_fill_pending = 1;
//         break;

//     case 3:
//         gwenesis_vdp_dma_copy();
//         break;
//     }
// }

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
 *  SEGA 315-5313 Debug Status
 *  Get current debug status and return it.
 *
 ******************************************************************************/
void gwenesis_vdp_get_debug_status(char *s)
{
   // int i = 0;
    s[0] = 0;
    s += sprintf(s, "ADDRESS: \t");
   // s += sprintf(s, "[R] %04x\n", gwenesis_vdp_laddress_r);
  //  s += sprintf(s, "\t\t[W] %04x\n\n", gwenesis_vdp_laddress_w);
    s += sprintf(s, "DMA: \t\t");
    s += sprintf(s, "%s\n", REG1_DMA_ENABLED ? "ENABLED" : "DISABLED");
  //  s += sprintf(s, " - ADDRESS\t\t%04x\n", dma_source);
  //  s += sprintf(s, " - LENGTH\t\t%04x\n\n",dma_length);
    s += sprintf(s, "STATUS: \t");
    s += sprintf(s, "%04x \n", gwenesis_vdp_status);
    s += sprintf(s, "PLANE A: \t\t");
    s += sprintf(s, "%04x \n", REG2_NAMETABLE_A);
    s += sprintf(s, "PLANE B: \t\t");
    s += sprintf(s, "%04x \n", REG4_NAMETABLE_B);
    s += sprintf(s, "PLANE WINDOW: \t\t");
    s += sprintf(s, "%04x \n", REG3_NAMETABLE_W*0x400);
    s += sprintf(s, "HCOUNTER: \t\t");
    s += sprintf(s, "%04x \n", REG10_COLUMN_COUNTER);
    s += sprintf(s, "VCOUNTER: \t\t");
    s += sprintf(s, "%04x \n", REG10_LINE_COUNTER);
    s += sprintf(s, "HSCROLL: \t\t");
    s += sprintf(s, "%04x \n", REG13_HSCROLL_ADDRESS);
}
#ifdef _HOST_
/******************************************************************************
 *
 *   SEGA 315-5313 Get CRAM
 *   Return current CRAM buffer
 *
 ******************************************************************************/
unsigned short gwenesis_vdp_get_cram(int index)
{
    return CRAM[index & 0x3f];
}

/******************************************************************************
 *
 *   SEGA 315-5313 Get VRAM
 *   Return current VRAM buffer
 *
 ******************************************************************************/
void gwenesis_vdp_get_vram(unsigned char *raw_buffer, int palette)
{
    int pixel = 0;
    unsigned char temp_buffer[2048 * 64];
    unsigned char temp_buffer2[128][1024];
    for (int i = 0; i < (2048 * 32); i++)
    {
        int pix1 = VRAM[i] >> 4;
        int pix2 = VRAM[i] & 0xF;
        temp_buffer[pixel] = pix1;
        pixel++;
        temp_buffer[pixel] = pix2;
        pixel++;
    }
    for (int i = 0; i < (2048 * 64); i++)
    {
        int pixel_column = i % 8;
        int pixel_line = (int)(i / 8) % 8;
        int tile = (int)(i / 64);
        int x = pixel_column + (tile * 8) % 128;
        int y = pixel_line + ((tile * 8) / 128) * 8;
        temp_buffer2[x][y] = temp_buffer[i];
    }
    int index = 0;
    for (int y = 0; y < 1024; y++)
    {
        for (int x = 0; x < 128; x++)
        {
            raw_buffer[index + 0] = (CRAM[temp_buffer2[x][y]] >> 4) & 0xe0;
            raw_buffer[index + 1] = (CRAM[temp_buffer2[x][y]]) & 0xe0;
            raw_buffer[index + 2]  = (CRAM[temp_buffer2[x][y]] << 4) & 0xe0;
            index += 4;
        }
    }
}

/******************************************************************************
 *
 *   SEGA 315-5313 Get VRAM as RAW
 *   Return current VRAM buffer as RAW
 *
 ******************************************************************************/
void gwenesis_vdp_get_vram_raw(unsigned char *raw_buffer)
{
    for (int pixel = 0; pixel < 0x10000; pixel++)
    {
        raw_buffer[pixel] = VRAM[pixel];
    }
}

/******************************************************************************
 *
 *   SEGA 315-5313 Get CRAM as RAW
 *   Return current CRAM buffer as RAW
 *
 ******************************************************************************/
void gwenesis_vdp_get_cram_raw(unsigned char *raw_buffer)
{
    for (int color = 0; color < 0x40; color++)
    {
        raw_buffer[color] = CRAM[color];
    }
}
#endif


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

    if (address < 0X4) return gwenesis_vdp_read_data_port_16();
    if (address < 0x8) return status_register_r();
    if (address == 0x8) return gwenesis_vdp_hvcounter();

             printf("unhandled gwenesis_vdp_read(%x)\n", address);

    //return 0xFFFF;

    // switch (address & 0x1F)
    // {
    // case 0x0:
    // case 0x2:
    //     return gwenesis_vdp_read_data_port_16();
    // case 0x4:
    // case 0x6:
    //     //printf("status:%04x\n",gwenesis_vdp_status);
    //     return status_register_r(); //gwenesis_vdp_status;
    // case 0x8:
    // case 0xA:
    // case 0xC:
    // case 0xE:
    //     return gwenesis_vdp_hvcounter();
    // case 0x18:
    //     // VDP FIFO TEST
    //     return 0xFFFF;
    // case 0x1C:
    //     // DEBUG REGISTER
    //     return 0xFFFF;
    // default:
   //      printf("unhandled gwenesis_vdp_read(%x)\n", address);
         return 0xFF;
    // }
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
    // switch (address & 0x1F)
    // {
    // case 0x11:
    // case 0x13:
    // case 0x15:
    // case 0x17:
    //     // SN76489 TODO
    //     return;
    // default:
    
        gwenesis_vdp_write_memory_16(address & ~1, (value << 8) | value);
    //     return;
    // }
}

/******************************************************************************
 *
 *   SEGA 315-5313 write to memory W16
 *   Write an word value to mapped memory on specified address
 *
 ******************************************************************************/
 //static inline
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

  if (address == 0x10) { // TODO PSG 8 bits write
    return;
  }
  // switch (address & 0x1F)
  // {
  // case 0x0:
  // case 0x2:
  //   gwenesis_vdp_write_data_port_16(value);
  //   return;
  // case 0x4:
  // case 0x6:
  //   gwenesis_vdp_control_port_write(value);
  //   return;
  // case 0x10:
  // case 0x12:
  // case 0x14:
  // case 0x16:
  //   // SN76489 T
  // case 0x18:
  //   // VDP FIFO TEST
  //   return;
  // case 0x1C:
  //   // DEBUG REGISTER
  //   return;
  // default:
  //   // UNHANDLED
  printf("unhandled gwenesis_vdp_write(%x, %x)\n", address, value);
  //   assert(0);
  // }
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
  saveGwenesisStateSet(state, "scan_line", scan_line);
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
  scan_line = saveGwenesisStateGet(state, "scan_line");
  dma_fill_pending = saveGwenesisStateGet(state, "dma_fill_pending");
  hvcounter_latch = saveGwenesisStateGet(state, "hvcounter_latch");
  hvcounter_latched = saveGwenesisStateGet(state, "hvcounter_latched");
  hint_pending = saveGwenesisStateGet(state, "hint_pending");
}