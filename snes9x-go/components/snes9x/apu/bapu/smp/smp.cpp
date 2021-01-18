#ifdef DEBUGGER
#include "../../../snes9x.h"
#include "../../../debug.h"
char tmp[1024];
#endif

#include "../snes/snes.hpp"

#define SMP_CPP
namespace SNES {

#ifdef DEBUGGER
#include "debugger/disassembler.cpp"
#endif

SMP smp;

#include "iplrom.hpp"

#define op_readpc() op_read(regs.pc++)
#define op_readdp(addr) op_read((regs.p.p << 8) + ((addr) & 0xff))
#define op_writedp(addr, data) op_write((regs.p.p << 8) + ((addr) & 0xff), data)
#define op_readaddr(addr) op_read(addr)
#define op_writeaddr(addr, data) op_write(addr, data)

#define op_readstack() ({ ticks++; apuram[0x0100 | ++regs.sp];});
#define op_readstack16() ({ ticks += 2; (apuram[0x0100 | ++regs.sp]) | (apuram[0x0100 | ++regs.sp] << 8);});
#define op_writestack(data) { ticks++; apuram[0x0100 | regs.sp--] = (data);}

uint8 SMP::op_adc(uint8 x, uint8 y) {
  int r = x + y + regs.p.c;
  regs.p.n = r & 0x80;
  regs.p.v = ~(x ^ y) & (x ^ r) & 0x80;
  regs.p.h = (x ^ y ^ r) & 0x10;
  regs.p.z = (uint8)r == 0;
  regs.p.c = r > 0xff;
  return r;
}

uint16 SMP::op_addw(uint16 x, uint16 y) {
  uint16 r;
  regs.p.c = 0;
  r  = op_adc(x, y);
  r |= op_adc(x >> 8, y >> 8) << 8;
  regs.p.z = r == 0;
  return r;
}

uint8 SMP::op_and(uint8 x, uint8 y) {
  x &= y;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8 SMP::op_cmp(uint8 x, uint8 y) {
  int r = x - y;
  regs.p.n = r & 0x80;
  regs.p.z = (uint8)r == 0;
  regs.p.c = r >= 0;
  return x;
}

uint16 SMP::op_cmpw(uint16 x, uint16 y) {
  int r = x - y;
  regs.p.n = r & 0x8000;
  regs.p.z = (uint16)r == 0;
  regs.p.c = r >= 0;
  return x;
}

uint8 SMP::op_eor(uint8 x, uint8 y) {
  x ^= y;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8 SMP::op_or(uint8 x, uint8 y) {
  x |= y;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8 SMP::op_sbc(uint8 x, uint8 y) {
  int r = x - y - !regs.p.c;
  regs.p.n = r & 0x80;
  regs.p.v = (x ^ y) & (x ^ r) & 0x80;
  regs.p.h = !((x ^ y ^ r) & 0x10);
  regs.p.z = (uint8)r == 0;
  regs.p.c = r >= 0;
  return r;
}

uint16 SMP::op_subw(uint16 x, uint16 y) {
  uint16 r;
  regs.p.c = 1;
  r  = op_sbc(x, y);
  r |= op_sbc(x >> 8, y >> 8) << 8;
  regs.p.z = r == 0;
  return r;
}

uint8 SMP::op_inc(uint8 x) {
  x++;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8 SMP::op_dec(uint8 x) {
  x--;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8 SMP::op_asl(uint8 x) {
  regs.p.c = x & 0x80;
  x <<= 1;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8 SMP::op_lsr(uint8 x) {
  regs.p.c = x & 0x01;
  x >>= 1;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8 SMP::op_rol(uint8 x) {
  unsigned carry = (unsigned)regs.p.c;
  regs.p.c = x & 0x80;
  x = (x << 1) | carry;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8 SMP::op_ror(uint8 x) {
  unsigned carry = (unsigned)regs.p.c << 7;
  regs.p.c = x & 0x01;
  x = carry | (x >> 1);
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

void SMP::tick() {
  timer0.tick();
  timer1.tick();
  timer2.tick();

  clock++;
  dsp.clock++;
}

void SMP::tick(unsigned clocks) {
  timer0.tick(clocks);
  timer1.tick(clocks);
  timer2.tick(clocks);

  clock += clocks;
  dsp.clock += clocks;
}

uint8 SMP::op_read(uint32 addr) {
  tick();
  if((addr & 0xfff0) == 0x00f0) return mmio_read(addr);
  if(addr >= 0xffc0 && status.iplrom_enable) return iplrom[addr & 0x3f];
  return apuram[addr];
}

void SMP::op_write(uint32 addr, uint8 data) {
  tick();
  if((addr & 0xfff0) == 0x00f0) mmio_write(addr, data);
  apuram[addr] = data;  //all writes go to RAM, even MMIO writes
}

void SMP::execute(int cycles) {
  clock -= cycles;

  int ticks = 0;

  while (clock < 0)
  {
    if (opcode_cycle == 0)
    {
  #ifdef DEBUGGER
      if (Settings.TraceSMP)
      {
        disassemble_opcode(tmp, regs.pc);
        S9xTraceMessage (tmp);
      }
  #endif
      opcode_number = op_readpc();
    }

    switch (opcode_number) {
      #include "core/oppseudo_misc.cpp"
      #include "core/oppseudo_mov.cpp"
      #include "core/oppseudo_pc.cpp"
      #include "core/oppseudo_read.cpp"
      #include "core/oppseudo_rmw.cpp"
    }
  }

  if (ticks) tick(ticks);
}

unsigned SMP::port_read(unsigned addr) {
  return apuram[0xf4 + (addr & 3)];
}

void SMP::port_write(unsigned addr, unsigned data) {
  apuram[0xf4 + (addr & 3)] = data;
}

unsigned SMP::mmio_read(unsigned addr) {
  switch(addr) {

  case 0xf2:
    return status.dsp_addr;

  case 0xf3:
    return dsp.read(status.dsp_addr & 0x7f);

  case 0xf4:
  case 0xf5:
  case 0xf6:
  case 0xf7:
    return registers[addr & 3];

  case 0xf8:
    return status.ram00f8;

  case 0xf9:
    return status.ram00f9;

  case 0xfd: {
    unsigned result = timer0.stage3_ticks & 15;
    timer0.stage3_ticks = 0;
    return result;
  }

  case 0xfe: {
    unsigned result = timer1.stage3_ticks & 15;
    timer1.stage3_ticks = 0;
    return result;
  }

  case 0xff: {
    unsigned result = timer2.stage3_ticks & 15;
    timer2.stage3_ticks = 0;
    return result;
  }

  default:
    return 0x00;
  }
}

void SMP::mmio_write(unsigned addr, unsigned data) {
  switch(addr) {

  case 0xf1:
    status.iplrom_enable = data & 0x80;

    if(data & 0x30) {
      if(data & 0x20) {
        registers[3] = 0x00;
        registers[2] = 0x00;
      }
      if(data & 0x10) {
        registers[1] = 0x00;
        registers[0] = 0x00;
      }
    }

    if(timer2.enable == false && (data & 0x04)) {
      timer2.stage2_ticks = 0;
      timer2.stage3_ticks = 0;
    }
    timer2.enable = data & 0x04;

    if(timer1.enable == false && (data & 0x02)) {
      timer1.stage2_ticks = 0;
      timer1.stage3_ticks = 0;
    }
    timer1.enable = data & 0x02;

    if(timer0.enable == false && (data & 0x01)) {
      timer0.stage2_ticks = 0;
      timer0.stage3_ticks = 0;
    }
    timer0.enable = data & 0x01;

    break;

  case 0xf2:
    status.dsp_addr = data;
    break;

  case 0xf3:
    if(status.dsp_addr & 0x80) break;
    dsp.write(status.dsp_addr, data);
    break;

  case 0xf4:
  case 0xf5:
  case 0xf6:
  case 0xf7:
    apuram[0xf4 + (addr & 3)] = data;
    break;

  case 0xf8:
    status.ram00f8 = data;
    break;

  case 0xf9:
    status.ram00f9 = data;
    break;

  case 0xfa:
    timer0.target = data;
    break;

  case 0xfb:
    timer1.target = data;
    break;

  case 0xfc:
    timer2.target = data;
    break;
  }
}

template<unsigned cycle_frequency>
void SMP::Timer<cycle_frequency>::tick() {
  if(++stage1_ticks < cycle_frequency) return;

  stage1_ticks = 0;
  if(enable == false) return;

  if(++stage2_ticks != target) return;

  stage2_ticks = 0;
  stage3_ticks = (stage3_ticks + 1) & 15;
}

template<unsigned cycle_frequency>
void SMP::Timer<cycle_frequency>::tick(unsigned clocks) {
  stage1_ticks += clocks;
  if(stage1_ticks < cycle_frequency) return;

  stage1_ticks -= cycle_frequency;
  if(enable == false) return;

  if(++stage2_ticks != target) return;

  stage2_ticks = 0;
  stage3_ticks = (stage3_ticks + 1) & 15;
}

void SMP::power() {
  Processor::clock = 0;

  timer0.target = 0;
  timer1.target = 0;
  timer2.target = 0;

  reset();
}

void SMP::reset() {
  memset(apuram, 0x00, 0x10000);

  opcode_number = 0;
  opcode_cycle = 0;

  regs.pc = 0xffc0;
  regs.sp = 0xef;
  regs.B.a = 0x00;
  regs.x = 0x00;
  regs.B.y = 0x00;
  regs.p = 0x02;

  //$00f1
  status.iplrom_enable = true;

  //$00f2
  status.dsp_addr = 0x00;

  //$00f8,$00f9
  status.ram00f8 = 0x00;
  status.ram00f9 = 0x00;

  //timers
  timer0.enable = timer1.enable = timer2.enable = false;
  timer0.stage1_ticks = timer1.stage1_ticks = timer2.stage1_ticks = 0;
  timer0.stage2_ticks = timer1.stage2_ticks = timer2.stage2_ticks = 0;
  timer0.stage3_ticks = timer1.stage3_ticks = timer2.stage3_ticks = 0;

  registers[0] = registers[1] = registers[2] = registers[3] = 0;
}

SMP::SMP() {
  apuram = new uint8[64 * 1024];
}

SMP::~SMP() {
	delete[] apuram;
}

}
