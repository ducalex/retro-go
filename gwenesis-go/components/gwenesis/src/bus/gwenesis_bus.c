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
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "m68k.h"

#include "ym2612.h"
#include "z80inst.h"
#include "gwenesis_bus.h"
#include "gwenesis_io.h"
#include "gwenesis_vdp.h"
#include "gwenesis_savestate.h"

#ifdef _HOST_
unsigned char MD_ROM_DATA[MAX_ROM_SIZE];      // 68K Main Program
unsigned int MD_ROM_DATA_LENGTH;
#else
// #include "rom_manager.h"
// unsigned char* MD_ROM_DATA[MAX_ROM_SIZE];      // 68K Main Program
// unsigned int MD_ROM_DATA_LENGTH;
#endif

// Setup CPU Memory
unsigned char M68K_RAM[MAX_RAM_SIZE]; // __attribute__((section("._itcram"))); // 68K RAM  
unsigned char ZRAM[MAX_Z80_RAM_SIZE]; // Z80 RAM
unsigned char TMSS[0x4];
extern unsigned short gwenesis_vdp_status;

// TMSS
int tmss_state = 0;
int tmss_count = 0;

/******************************************************************************
 *
 *   Load a Sega Genesis Cartridge into CPU Memory
 *
 ******************************************************************************/
#ifdef _HOST_
void load_cartridge(unsigned char *buffer, size_t size)
{
    // Clear all volatile memory
    memset(M68K_RAM, 0, MAX_RAM_SIZE);
    memset(ZRAM, 0, MAX_Z80_RAM_SIZE);
    memset(ROM_DATA, 0, MAX_ROM_SIZE);

    // Set Z80 Memory as ZRAM
    z80_set_memory(ZRAM);
    z80_pulse_reset();

    // Copy file contents to CPU ROM memory
    ROM_DATA_LENGTH=size;
    memcpy(ROM_DATA, buffer, size);

    #ifdef ROM_SWAP
    printf("--ROM swap mode--\n");
    for (int i=0; i < ROM_DATA_LENGTH;i++ )
    {   
        if (( i & 1) == 0 ) {
        char z = ROM_DATA[i];
        ROM_DATA[i]=ROM_DATA[i+1];
        ROM_DATA[i+1]=z;
        }

    }
    #endif


    set_region();
}
#else

void load_cartridge()
{
    // Clear all volatile memory
    memset(M68K_RAM, 0, MAX_RAM_SIZE);
    memset(ZRAM, 0, MAX_Z80_RAM_SIZE);

    // Set Z80 Memory as Z80_RAM
    z80_set_memory((unsigned int *)ZRAM);

    z80_pulse_reset();

    set_region();

}
#endif



/******************************************************************************
 *
 *   Power ON the CPU
 *   Initialize 68K, Z80 and YM2612 Cores
 *
 ******************************************************************************/
void power_on() {
  // Set M68K CPU as original MOTOROLA 68000
  m68k_set_cpu_type(M68K_CPU_TYPE_68000);
  // Initialize M68K CPU
  m68k_init();
  // Initialize Z80 CPU
  z80_start();
  // Initialize YM2612 chip
  YM2612Init();
  YM2612Config(9);
}

/******************************************************************************
 *
 *   Reset the CPU Emulation
 *   Send a pulse reset to 68K, Z80 and YM2612 Cores
 *
 ******************************************************************************/
void reset_emulation() {
  // Send a reset pulse to Z80 CPU
  z80_pulse_reset();
  // Send a reset pulse to Z80 M68K
  m68k_pulse_reset();
  // Send a reset pulse to YM2612 chip
  YM2612ResetChip();
  // Send a reset pulse to SEGA 315-5313 chip
  gwenesis_vdp_reset();
}
#if 0

typedef void (*Handler_write)(unsigned int address,unsigned int value);
typedef unsigned int (*Handler_read)(unsigned int address);


static Handler_write jump_write8[256] = {0};
static Handler_write jump_write16[256] = {0};

static Handler_read jump_read8[256] = {0};
static Handler_read jump_read16[256] = {0};

void wr8ram (unsigned address,unsigned int value)
{
  WRITE8RAM(address,value);
}
void wr16ram (unsigned address,unsigned int value)
{
  WRITE16RAM(address,value);
}

unsigned int rd8ram (unsigned address)
{
  return FETCH8RAM(address);
}
unsigned int rd16ram (unsigned address)
{
  return FETCH16RAM(address);
}
unsigned int rd8rom (unsigned address)
{
  return FETCH8ROM(address);
}
unsigned int rd16rom (unsigned address)
{
  return FETCH16ROM(address);
}

void init_jump_tables() {
  for (int i = 0; i < 0x100; i++) {

    // ROM
    if (i < 0x80) {
      jump_write8[i] = 0;
      jump_write16[i] = 0;

      jump_read8[i] = rd8rom;
      jump_read16[i] = rd16rom;
    }

    //RAM
    if (i == 0xFF) {
      jump_write8[i] = wr8ram;
      jump_write16[i] = wr16ram;

      jump_read8[i] = rd8ram;
      jump_read16[i] = rd16ram;
    }

    // Z80
    if (i == 0xA0) {
      jump_write8[i] = wr8ram;
      jump_write16[i] = wr16ram;

      jump_read8[i] = rd8ram;
      jump_read16[i] = rd16ram;
    }

    // IO
    if (i == 0xA1) {
      jump_write8[i] = wr8ram;
      jump_write16[i] = wr16ram;

      jump_read8[i] = rd8ram;
      jump_read16[i] = rd16ram;
    }

    // VDP
    if (i == 0xC0) {
      jump_write8[i] = gwenesis_vdp_write_memory_8;
      jump_write16[i] = gwenesis_vdp_write_memory_16;

      jump_read8[i] = gwenesis_vdp_read_memory_8;
      jump_read16[i] = gwenesis_vdp_read_memory_16;
    }
  }
}
#endif
/******************************************************************************
 *
 *   Set Region
 *   Look at ROM to set console compatible region
 *
 ******************************************************************************/
void set_region()
{    
    extern int mode_pal;

    printf("RegionROM:%c (%02x) ", FETCH8ROM(0x1F0), FETCH8ROM(0x1F0));

    if (FETCH8ROM(0x1F0) == 0x31 || FETCH8ROM(0x1F0) == 0x4a) {
      gwenesis_io_set_reg(0, 0x00);
      printf("Domestic-NTSC Japanese model\n");
      gwenesis_vdp_status &= 0xFFFE;
      mode_pal = 0;
    }

    else if (FETCH8ROM(0x1F0) == 0x41 || FETCH8ROM(0x1F0) == 0x40 ||
             FETCH8ROM(0x1F0) == 0x45 || FETCH8ROM(0x1F0) == 0x46 ||
             FETCH8ROM(0x1F0) == 0x38) {
      gwenesis_io_set_reg(0, 0xE0);
      printf("Oversea-PAL\n");
      gwenesis_vdp_status |= 1;
      mode_pal = 1;
    }

    else {
      gwenesis_io_set_reg(0, 0xA0);
      printf("Oversea-NTSC\n");
      gwenesis_vdp_status &= 0xFFFE;
      mode_pal = 0;
    }
}
/******************************************************************************
 *
 *   Main memory address mapper
 *   Map all main memory region address for CPU program
 *   68K Access to Z80 Memory
 *
 ******************************************************************************/
#if 1
static inline unsigned int gwenesis_bus_map_z80_address(unsigned int address) {

  unsigned int range = (address & 0xF000);
  switch (range) {
  case 0:
  case 0x1000:
    return Z80_RAM_ADDR;
  case 0x2000:
  case 0x3000:
    return NONE;
  case 0x4000:
    return YM2612_ADDR;

  default:
    return NONE;
  }
}
#endif
/******************************************************************************
 *
 *   IO memory address mapper
 *   Map all input/output region address for CPU program
 *
 ******************************************************************************/
#if 1
static inline unsigned int gwenesis_bus_map_io_address(unsigned int address)
{
  unsigned int range = (address & 0x1000) ;
  switch (range) {
  case 0:      return IO_CTRL;
  case 0x1000: return Z80_CTRL;
  default:
      // if (address >= 0xa14000 && address < 0xa11404)
      // return (tmss_state == 0) ? TMSS_CTRL : NONE;
    return NONE;
  }
}
#endif
/******************************************************************************
 *
 *   Main memory address mapper
 *   Map all main memory region address for CPU program
 *
 ******************************************************************************/
#if 0
static inline 
unsigned int gwenesis_bus_map_address(unsigned int address) {
  // Mask address page
  unsigned int range = (address & 0xFF0000) >> 16;

  switch (range) {
    case 0xA0 : return gwenesis_bus_map_z80_address(address);
    case 0xA1 : return gwenesis_bus_map_io_address(address);
    case 0xC0 : return VDP_ADDR;
    default   : return NONE;
  }
}
#endif
#if 1
static inline 
unsigned int gwenesis_bus_map_address(unsigned int address) {
  // Mask address page
  unsigned int range = (address & 0xFF0000) >> 16;

  // Check mask and select memory type
  if (range < 0x80) //        ROM ADDRESS 0x000000 - 0x3FFFFF
    return ROM_ADDR;

  else if (range == 0xA0) // Z80 ADDRESS 0xA00000 - 0xA0FFFF
    return gwenesis_bus_map_z80_address(address);

  else if (range == 0xA1) //                  IO ADDRESS  0xA10000 - 0xA1FFFF
    return gwenesis_bus_map_io_address(address);

  else if (range == 0xC0) // VDP ADDRESS 0xC00000 - 0xDFFFFFF
    return VDP_ADDR;
  else if (range == 0xFF) // RAM ADDRESS 0xE00000 - 0xFFFFFFF
    return RAM_ADDR;
  // If not a valid address return 0
  printf("M68K > ?? unmap address %x\n", address);
  return NONE;
}
#endif
/******************************************************************************
 *
 *   Main read address routine
 *   Write an value to memory mapped on specified address
 *
 ******************************************************************************/
#if 1
static inline unsigned int gwenesis_bus_read_memory_8(unsigned int address) {
 // printf("read8  %x\n", address);

  switch (gwenesis_bus_map_address(address)) {
  case ROM_ADDR:
    return FETCH8ROM(address);

  case Z80_RAM_ADDR:
    return ZRAM[address & 0x1FFF];

  case YM2612_ADDR:
    return YM2612Read();

  case IO_CTRL:
    return gwenesis_io_read_ctrl(address & 0x1F);

  case Z80_CTRL:
    return z80_read_ctrl(address & 0xFFFF);

  case TMSS_CTRL:
    printf("TMS\n");
    if (tmss_state == 0)
      return TMSS[address & 0x4];
    return 0xFF;

  case VDP_ADDR:
    return gwenesis_vdp_read_memory_8(address);

  case RAM_ADDR:
    return FETCH8RAM(address);

  default:
     printf(" default read 8 %x\n", address);
    return 0x00;
  }
  return 0x00;
}
#endif
#if 1
static inline unsigned int gwenesis_bus_read_memory_16(unsigned int address) {
   // printf("read16 %x\n", address);

  switch (gwenesis_bus_map_address(address)) {
  case ROM_ADDR:
    return FETCH16ROM(address);

  case Z80_RAM_ADDR:
    return ZRAM[address & 0X7FFF] | (ZRAM[address & 0X7FFF] << 8); 

  case YM2612_ADDR:
    return YM2612Read();

  case IO_CTRL:
    return gwenesis_io_read_ctrl(address & 0x1F);

  case VDP_ADDR:
    return gwenesis_vdp_read_memory_16(address);

  case RAM_ADDR:
    return FETCH16RAM(address);

  default:
    printf("read mem 16 default %x\n", address);
    return (gwenesis_bus_read_memory_8(address) << 8) |
           gwenesis_bus_read_memory_8(address + 1);
  }
  return 0x00;
}
#endif
/******************************************************************************
 *
 *   Main write address routine
 *   Write an value to memory mapped on specified address
 *
 ******************************************************************************/
#if 1
static inline void gwenesis_bus_write_memory_8(unsigned int address,
                                              unsigned int value) {
 // printf("write8  %x\n", address,value);

  switch (gwenesis_bus_map_address(address)) {

  case VDP_ADDR:

    gwenesis_vdp_write_memory_16(address & ~1, (value << 8) | value);
    return;

  case Z80_RAM_ADDR:
    ZRAM[address & 0x1FFF] = value;
    return;

  case IO_CTRL:

    gwenesis_io_write_ctrl(address & 0x1F, value);
    return;

  case Z80_CTRL:

    z80_write_ctrl(address & 0xFFFF, value);
    return;

  case YM2612_ADDR:

    YM2612Write(address & 0x3, value);
    return;

  case TMSS_CTRL:

    if (tmss_state == 0) {
      TMSS[address & 0x4] = value;
      tmss_count++;
      if (tmss_count == 4)
        tmss_state = 1;
    }
    return;

  case RAM_ADDR:

    WRITE8RAM(address, value);
    return;

  default:
    //printf("write(%x, %x)\n", address, value);
    return;
  }
  return;
}
#endif

#if 1
static inline void gwenesis_bus_write_memory_16(unsigned int address,
                                               unsigned int value) {
  //printf("write16  %x\n", address,value);

  switch (gwenesis_bus_map_address(address)) {

  case Z80_RAM_ADDR:
    ZRAM[address & 0X7FFF]= value >> 8;
    return;

  case Z80_CTRL:

    z80_write_ctrl(address & 0x1FFF, value);
    return;

  case YM2612_ADDR:

    YM2612Write(address & 0x3, value & 0Xff);
    return;

  case VDP_ADDR:

    gwenesis_vdp_write_memory_16(address, value);
    return;

  case RAM_ADDR:

    WRITE16RAM(address, value);
    return;

  default:

    printf("write mem 16 default %x \n", address);
    gwenesis_bus_write_memory_8(address, (value >> 8) & 0xff);
    gwenesis_bus_write_memory_8(address + 1, (value)&0xff);

    return;
  }
  return;
}
#endif
/******************************************************************************
 *
 *   68K CPU read address R8
 *   Read an address from memory mapped and return value as byte
 *
 ******************************************************************************/
unsigned int m68k_read_memory_8(unsigned int address)
{
      //  if ((address &  0xFF0000 ) == 0xFF0000) return FETCH8RAM(address);
    return gwenesis_bus_read_memory_8(address);
}

/******************************************************************************
 *
 *   68K CPU read address R16
 *   Read an address from memory mapped and return value as word
 *
 ******************************************************************************/
 unsigned int m68k_read_memory_16(unsigned int address)
{
     //   if ((address &  0xFF0000 ) == 0xFF0000) return FETCH16RAM(address);
    return gwenesis_bus_read_memory_16(address);
}

/******************************************************************************
 *
 *   68K CPU read address R32
 *   Read an address from memory mapped and return value as long
 *
 ******************************************************************************/
 unsigned int m68k_read_memory_32(unsigned int address)
{
  //  if ((address &  0xFF0000 ) == 0xFF0000) return FETCH32RAM(address);
    return (gwenesis_bus_read_memory_16(address) << 16) | gwenesis_bus_read_memory_16(address + 2);
}

/******************************************************************************
 *
 *   68K CPU write address W8
 *   Write an value as byte to memory mapped on specified address
 *
 ******************************************************************************/
void m68k_write_memory_8(unsigned int address, unsigned int value) {
  // if ((address & 0xFF0000) == 0xFF0000) {
  //   WRITE8RAM(address, value);
  //   return;
  // }
  gwenesis_bus_write_memory_8(address, value);
  return;
}

/******************************************************************************
 *
 *   68K CPU write address W16
 *   Write an value as word to memory mapped on specified address
 *
 ******************************************************************************/
void m68k_write_memory_16(unsigned int address, unsigned int value) {
  // if ((address & 0xFF0000) == 0xFF0000) {
  //   WRITE16RAM(address, value);
  //   return;
  // }
  gwenesis_bus_write_memory_16(address, value);
  return;
}
/******************************************************************************
 *
 *   68K CPU write address W32
 *   Write an value as word to memory mapped on specified address
 *
 ******************************************************************************/
void m68k_write_memory_32(unsigned int address, unsigned int value) {

  // if ((address & 0xFF0000) == 0xFF0000) {
  //   WRITE32RAM(address, value);
  //   return;
  // }
  gwenesis_bus_write_memory_16(address, (value >> 16) & 0xffff);
  gwenesis_bus_write_memory_16(address + 2, (value)&0xffff);

  return;
}

unsigned int m68k_read_disassembler_16(unsigned int address)
{
    return m68k_read_memory_16(address);
}
unsigned int m68k_read_disassembler_32(unsigned int address)
{
    return m68k_read_memory_32(address);
}

void gwenesis_bus_save_state() {
  SaveState* state;
  state = saveGwenesisStateOpenForWrite("bus");
  saveGwenesisStateSetBuffer(state, "M68K_RAM", M68K_RAM, MAX_RAM_SIZE);
  saveGwenesisStateSetBuffer(state, "ZRAM", ZRAM, MAX_Z80_RAM_SIZE);
  saveGwenesisStateSetBuffer(state, "TMSS", TMSS, sizeof(TMSS));
  saveGwenesisStateSet(state, "tmss_state", tmss_state);
  saveGwenesisStateSet(state, "tmss_count", tmss_count);
}

void gwenesis_bus_load_state() {
    SaveState* state = saveGwenesisStateOpenForRead("bus");
    saveGwenesisStateGetBuffer(state, "M68K_RAM", M68K_RAM, MAX_RAM_SIZE);
    saveGwenesisStateGetBuffer(state, "ZRAM", ZRAM, MAX_Z80_RAM_SIZE);
    saveGwenesisStateGetBuffer(state, "TMSS", TMSS, sizeof(TMSS));
    tmss_state = saveGwenesisStateGet(state, "tmss_state");
    tmss_count = saveGwenesisStateGet(state, "tmss_count");
}
