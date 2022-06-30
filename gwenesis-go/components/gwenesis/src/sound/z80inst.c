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
#include "Z80.h"
#include "z80inst.h"
#include "m68kcpu.h"
#include "gwenesis_bus.h"
#include "gwenesis_savestate.h"
#include "ym2612.h"

// #pragma GCC optimize("Ofast")

#define M68K_FREQ_DIVISOR   7
#define Z80_FREQ_DIVISOR    14

static int bus_ack = 0;
static int reset = 0;
static int reset_once = 0;
uint64_t zclk = 0;
static int initialized = 0;

static unsigned char *Z80_RAM;

static Z80 cpu;

void ResetZ80(register Z80 *R);

// Bank register used by Z80 to access M68K Memory space 1 BANK=32KByte
static int Z80_BANK;

/** DAsm() ***************************************************/
/** DAsm() will disassemble the code at adress A and put    **/
/** the output text into S. It will return the number of    **/
/** bytes disassembled.                                     **/
/*************************************************************/
#ifdef _HOST_
int DAsm(char *S,word A)
{
  char R[128],H[10],C,*P;
  const char *T;
  byte J,Offset;
  word B;

  Offset=0;
  B=A;
  C='\0';
  J=0;

  switch(RdZ80(B))
  {
    case 0xCB: B++;T=MnemonicsCB[RdZ80(B++)];break;
    case 0xED: B++;T=MnemonicsED[RdZ80(B++)];break;
    case 0xDD: B++;C='X';
               if(RdZ80(B)!=0xCB) T=MnemonicsXX[RdZ80(B++)];
               else
               { B++;Offset=RdZ80(B++);J=1;T=MnemonicsXCB[RdZ80(B++)]; }
               break;
    case 0xFD: B++;C='Y';
               if(RdZ80(B)!=0xCB) T=MnemonicsXX[RdZ80(B++)];
               else
               { B++;Offset=RdZ80(B++);J=1;T=MnemonicsXCB[RdZ80(B++)]; }
               break;
    default:   T=Mnemonics[RdZ80(B++)];
  }

  if(P=strchr(T,'^'))
  {
    strncpy(R,T,P-T);R[P-T]='\0';
    sprintf(H,"%02X",RdZ80(B++));
    strcat(R,H);strcat(R,P+1);
  }
  else strcpy(R,T);
  if(P=strchr(R,'%')) *P=C;

  if(P=strchr(R,'*'))
  {
    strncpy(S,R,P-R);S[P-R]='\0';
    sprintf(H,"%02X",RdZ80(B++));
    strcat(S,H);strcat(S,P+1);
  }
  else
    if(P=strchr(R,'@'))
    {
      strncpy(S,R,P-R);S[P-R]='\0';
      if(!J) Offset=RdZ80(B++);
      strcat(S,Offset&0x80? "-":"+");
      J=Offset&0x80? 256-Offset:Offset;
      sprintf(H,"%02X",J);
      strcat(S,H);strcat(S,P+1);
    }
    else
      if(P=strchr(R,'#'))
      {
        strncpy(S,R,P-R);S[P-R]='\0';
        sprintf(H,"%04X",RdZ80(B)+256*RdZ80(B+1));
        strcat(S,H);strcat(S,P+1);
        B+=2;
      }
      else strcpy(S,R);

  return(B-A);
}
#endif

void z80_start() {
    cpu.IPeriod = 1;
    cpu.ICount = 0;
    cpu.Trace = 0;
    cpu.Trap = 0x0009;
    ResetZ80(&cpu);
    reset=1;
    reset_once=0;
    bus_ack=0;
    zclk=0;
}

void z80_pulse_reset() {
  ResetZ80(&cpu);
}

void z80_run(uint64_t target) {

  // we are in advance,nothind to do
  if (zclk >= target)
    return;

  int timeslice = (target - zclk) / Z80_FREQ_DIVISOR;

  int rem = 0;
  if ((reset_once ==1)  && (bus_ack == 0) && (reset == 0) )
    rem = ExecZ80(&cpu, timeslice);

  zclk = target - rem * Z80_FREQ_DIVISOR;
}

//void z80_execute(unsigned int target) { ExecZ80(&cpu, target); }

extern uint64_t m68k_clock;

void z80_sync(void) {
  /*
  get M68K clock and
  Execute cycles on z80 to sync with m68K
  */
  int slice = m68k_cycles_run();
  slice = slice * M68K_FREQ_DIVISOR;

  z80_run(slice + m68k_clock);
}

void z80_set_memory(unsigned int *buffer)
{
    Z80_RAM = (unsigned char *)buffer;
    initialized = 1;
}

void z80_write_ctrl(unsigned int address, unsigned int value) {
  if (address == 0x1100) // BUSREQ
  {
    //printf("BUSREQ = %d\n", value);
    z80_sync();

    if (value) {
      bus_ack = 1;

    } else {
      bus_ack = 0;
    }

  } else if (address == 0x1200) // RESET
  {
    //printf("RESET = %d\n", value);
    if (value == 0) {
      reset = 1;
      //printf("RESET REQ = %d\n", value);
    } else {
     // printf("RESET CANCEL = %d\n", value);

      z80_pulse_reset();
      reset = 0;
      reset_once = 1;
    }
  }
}

unsigned int z80_read_ctrl(unsigned int address) {
  if (address == 0x1100) {
    //printf("CPU RUNNING = %d  \n", bus_ack ? 0 : 1);
    return bus_ack ? 0 : 1;
  } else if (address == 0x1101) {
    return 0x00;
  } else if (address == 0x1200) {
    //printf("CPU RESET = %d  \n", bus_ack ? 0 : 1);
    return reset ? 0 : 1;
  } else if (address == 0x1201) {
    return 0x00;
  }
  return 0xFF;
}

void z80_irq_line(unsigned int value)
{
    if (reset_once == 0) return;

    if (value)
        cpu.IRequest = INT_IRQ;
    else
        cpu.IRequest = INT_NONE;
}

#if 0
void z80_write_memory_8(unsigned int address, unsigned int value)
{
  //  printf("Z80W mem8 @%x:%x\n",address,value);
    WrZ80(address & 0X7FFF, value &0xFF);
}
unsigned int z80_read_memory_8(unsigned int address)
{
  // printf("Z80R mem @%x\n",address);

    return RdZ80(address & 0X7FFF);
}

void z80_write_memory_16(unsigned int address, unsigned int value)
{
 //   printf("Z80W mem16 @%x:%x\n",address,value);
    WrZ80(address & 0X7FFF, value >> 8);
    //WrZ80(address+1, value &0xFF);
}
unsigned int z80_read_memory_16(unsigned int address)
{
    //unsigned int value = (RdZ80(address) << 8) | RdZ80(address+1);
   // return value;
 //  printf("Z80R mem16 @%x:%x\n",address);
    unsigned int value = RdZ80(address & 0X7FFF);
    return (value << 8 ) | value;
}

unsigned int z80_disassemble(unsigned char *screen_buffer, unsigned int address) {
    int pc;
    pc = DAsm(screen_buffer, address);
    return pc;
}

word z80_get_reg(int reg_i) {
    switch(reg_i) {
        case 0: return cpu.AF.W; break;
        case 1: return cpu.BC.W; break;
        case 2: return cpu.DE.W; break;
        case 3: return cpu.HL.W; break;
        case 4: return cpu.IX.W; break;
        case 5: return cpu.IY.W; break;
        case 6: return cpu.PC.W; break;
        case 7: return cpu.SP.W; break;
    }
}
#endif

/********************************************
 * Z80 Bank
 ********************************************/

unsigned int zbankreg_mem_r8(unsigned int address)
{
    return 0xFF;
}

// void zbankreg_mem_w8(unsigned int address, unsigned int value)
// {
//     address &= 0xFFF;
//     if (address < 0x100)
//     {
//         Z80_BANK >>= 1;
//         Z80_BANK |= (value & 1) << 8;
//         printf("Z80 bank points to: %06x\n", Z80_BANK << 15);
//         return;
//     }
// }

static inline void zbankreg_mem_w8(unsigned int value) {
  Z80_BANK >>= 1;
  Z80_BANK |= (value & 1) << 8;
  //printf("Z80 bank points to: %06x\n", Z80_BANK << 15);
  return;
}

static inline unsigned int zbank_mem_r8(unsigned int address)
{
    address &= 0x7FFF;
    address |= (Z80_BANK << 15);

    // mem_log("Z80", "bank read: %06x\n", address);
  //  printf("Z80 bank read: %06x\n", address);
    if (address >= 0XFF0000)
      return FETCH8RAM(address);
    else if (address < 0X800000)
      return FETCH8ROM(address);
    else
     return 0x00;
    //   return m68k_read_memory_8(address);
}

static inline void zbank_mem_w8(unsigned int address, unsigned int value) {
  address &= 0x7FFF;
  address |= (Z80_BANK << 15);

  // mem_log("Z80", "bank write %06x: %02x\n", address, value);
   //printf("Z80 bank write %06x: %02x\n", address, value);
   if (address >= 0XFF0000)
    WRITE8RAM(address, value);

//   else
//     m68k_write_memory_8(address, value);

  return;
}

/*
unsigned int zvdp_mem_r8(unsigned int address)
{
    if (address >= 0x7F00 && address < 0x7F20)
        return vdp_mem_r8(address);
    return 0xFF;
}

void zvdp_mem_w8(unsigned int address, unsigned int value)
{
    if (address >= 0x7F00 && address < 0x7F20)
        vdp_mem_w8(address, value);
}

*/
word LoopZ80(register Z80 *R)
{
    return 0;
}
#if 0
byte RdZ80(register word Addr)
{
    unsigned int range = Addr >> 12;

    switch (range)
    {
    case 0:
    case 1:
        return Z80_RAM[Addr];
        break;
    case 2:
       // printf("Z80 RD Unknown ? @%x\n", Addr);
       // return Z80_RAM[Addr & 0x1FFF];
        return 0xff; //Z80_RAM[Addr & 0x1FFF];
        break;
    case 3:
       // return Z80_RAM[Addr & 0x1FFF];
       // printf("Z80 RD Unknown ? @%x\n", Addr);
       // return Z80_RAM[Addr & 0x1FFF];
        return 0xff; //Z80_RAM[Addr & 0x1FFF];
        break;
    case 4:
        //printf("Z80 RD YM2612W @%x\n", Addr);
        return YM2612Read();
        //return ym2612_read_memory_8(Addr);
        break;
    case 5:
        printf("Z80 RD YM2612W @%x\n", Addr);
        //return ym2612_read_memory_8(Addr);
        return YM2612Read();

        //return 0;
        break;
    case 6: // bank register
        return 0xFF;
        break;
    case 7:
       // printf("Z80 RD VDP @0x7F12 ?? @%x\n", Addr);
        return 0;
        break;
    default:
        return zbank_mem_r8(Addr);
        break;
    }
}

void WrZ80(register word Addr, register byte Value)

{
    unsigned int range = Addr >> 12;

    switch (range)
    {
    case 0:
    case 1:
        Z80_RAM[Addr] = Value;
        break;
    case 2:
        //printf("Z80 WR Unknown ? @%x\n", Addr);
        //Z80_RAM[Addr & 0x1FFF] = Value;
        break;
    case 3:
        //printf("Z80 RD Unknown ? @%x\n", Addr);
        //Z80_RAM[Addr & 0x1FFF] = Value;
        break;
    case 4:
    //printf("Z80 WR YM2612 @%x:%x\n",Addr,Value);
      // ym2612_write_memory_8(Addr, Value);
      YM2612Write(Addr & 0x3, Value);
      break;
    case 5:
      //printf("Z80 WR YM2612? @%x:%x\n",Addr,Value);
      // ym2612_write_memory_8(Addr, Value);
      YM2612Write(Addr & 0x3, Value);

      break;
    case 6:
        zbankreg_mem_w8(Addr, Value);
        break;
    case 7:
        //printf("Z80 WR PSG67489 @0x7F11 ?? @%x\n", Addr);
        break;
    default:
        zbank_mem_w8(Addr, Value);
        break;
    }
}
#endif

byte RdZ80(register word Addr)
{
  //  printf("RdZ80  %x\n", Addr);


  if (Addr < 0x2000)
    return Z80_RAM[Addr];

  if (Addr < 0x4000)
    return 0xff;

  if (Addr < 0x6000)
    return YM2612Read();

  if (Addr >= 0x8000)
    return zbank_mem_r8(Addr);

  return 0xFF;
}

void WrZ80(register word Addr, register byte Value) {

//printf("WrZ80  %x %x\n", Addr, Value);

  if (Addr < 0x2000) {
    Z80_RAM[Addr] = Value;
    return;
  }

  if (Addr < 0x4000) {
    return;
  }

  if (Addr < 0x6000) {
    YM2612Write(Addr & 0x3, Value);
    return;
  }

  if (Addr == 0x6000) {
    zbankreg_mem_w8(Value);
    return;
  }

  if (Addr >= 0x8000) {
    zbank_mem_w8(Addr, Value);
    return;
  }
}
#if 0
byte RdZ80(register word Addr)
{
    unsigned int range = Addr >> 13;

    switch (range)
    {
    case 0:
        return Z80_RAM[Addr];

    case 1:
        return 0xff;

    case 2:
        return YM2612Read();

    case 3: // BANK register or PSG76489
        return 0xff;

    default:
        return zbank_mem_r8(Addr);
    }
}

void WrZ80(register word Addr, register byte Value)

{
  unsigned int range = Addr >> 13;

  switch (range) {
  case 0:
    Z80_RAM[Addr] = Value;
    return;

  case 1:
    return;

  case 2:
    // printf("Z80 WR YM2612 @%x:%x\n",Addr,Value);
    //  ym2612_write_memory_8(Addr, Value);
    YM2612Write(Addr & 0x3, Value);
    return;

  case 3:  // BANK register or PSG76489
    // printf("Z80 RD Unknown ? @%x\n", Addr);
    // Z80_RAM[Addr & 0x1FFF] = Value;
    if (Addr == 0x6000) zbankreg_mem_w8(Value);
    return;
    // if (Addr == 6000) return;
    // else  zbankreg_mem_w8(Addr, Value);
    // return;

  default:
    zbank_mem_w8(Addr, Value);
    return;
  }
}
#endif

byte InZ80(register word Port) {return 0;}
void OutZ80(register word Port, register byte Value) {;}
void PatchZ80(register Z80 *R) {;}
void DebugZ80(register Z80 *R) {;}


void gwenesis_z80inst_save_state() {
    SaveState* state;
    state = saveGwenesisStateOpenForWrite("z80inst");
    saveGwenesisStateSetBuffer(state, "cpu", &cpu, sizeof(Z80));
    saveGwenesisStateSet(state, "bus_ack", bus_ack);
    saveGwenesisStateSet(state, "reset", reset);
    saveGwenesisStateSet(state, "reset_once", reset_once);
    saveGwenesisStateSetBuffer(state, "zclk", &zclk, sizeof(uint64_t));
    saveGwenesisStateSet(state, "initialized", initialized);
    saveGwenesisStateSet(state, "Z80_BANK", Z80_BANK);
}

void gwenesis_z80inst_load_state() {
    SaveState* state = saveGwenesisStateOpenForRead("z80inst");
    saveGwenesisStateGetBuffer(state, "cpu", &cpu, sizeof(Z80));
    bus_ack = saveGwenesisStateGet(state, "bus_ack");
    reset = saveGwenesisStateGet(state, "reset");
    reset_once = saveGwenesisStateGet(state, "reset_once");
    saveGwenesisStateGetBuffer(state, "zclk", &zclk, sizeof(uint64_t));
    initialized = saveGwenesisStateGet(state, "initialized");
    Z80_BANK = saveGwenesisStateGet(state, "Z80_BANK");
}
