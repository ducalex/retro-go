//============================================================================
//
//   SSSS    tt          lll  lll
//  SS  SS   tt           ll   ll
//  SS     tttttt  eeee   ll   ll   aaaa
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2012 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id$
//============================================================================

//============================================================================
// This class provides Thumb emulation code ("Thumbulator")
//    by David Welch (dwelch@dwelch.com)
// Modified by Fred Quimby
// Code is public domain and used with the author's consent
//============================================================================
#define THUMB_SUPPORT
#include <nds.h>
#include "bspf.hxx"
#include "Thumbulator.hxx"
#include "Cart.hxx"

uInt32 reg_sys[16]    = {0};
uInt32 cFlag          = 0;
uInt8 *myARMRAM       = 0;

//#define SAFE_THUMB  1     // This enables the Safe Thumb check... otherwise we are ALWAYS UNSAFE (needed for speed)

#define MEM_256KB   (1024 * 256)        // We decode the ROM out here in the cart_buffer[] which limits us to 256K of ARM code (that's huge)

uInt32 cStack, cBase, cStart;           // The DPC+ and CDF/J/+ drivers need to set these before usign the Thumbulator

#ifdef CPU_PROFILER
u32 profiler[256];
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Thumbulator::Thumbulator(uInt16* rom_ptr)
{
    for(uInt32 i=0; i < ROMSIZE/2; i++)
    {
        Thumbulator::Op decoded = decodeInstructionWord(*rom_ptr++);
        cart_buffer[MEM_256KB+i] = (uInt8)decoded;
    }    
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Thumbulator::~Thumbulator()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 void Thumbulator::run( void )
{
  reset();
  execute();
  return;
}

#define write8(addr, data)  (*(uInt8*)(myARMRAM + (addr & RAMADDMASK)) = data)
#define write16(addr, data) (*(uInt16*)(myARMRAM + (addr & RAMADDMASK)) = data)
#define write32(addr, data) (*(uInt32*)(myARMRAM + (addr & RAMADDMASK)) = data)
#define readRAM32(addr)     (*((uInt32*) (myARMRAM + (addr & RAMADDMASK))))

#define read_register(reg)       reg_sys[reg]
#define write_register(reg,data) reg_sys[reg]=(data)

#define do_vflag_bit(x) vFlag=x;
#define do_cflag_bit(x) cFlag=x;
#define do_znflags(x)   ZNflags=(x)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline uInt16 Thumbulator::read16 ( uInt32 addr )
{
  if (addr & 0x40000000)
  {
      return *((uInt16*) (myARMRAM + (addr & RAMADDMASK)));
  }
  else
  {
      return *((uInt16*)&cart_buffer[addr]);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline uInt32 Thumbulator::read32 ( uInt32 addr )
{
  if (addr & 0x40000000)
  {
      return *((uInt32*) (myARMRAM + (addr & RAMADDMASK)));
  }
  else
  {
      return *((uInt32*)&cart_buffer[addr]);
  }
}

inline uInt8 Thumbulator::read8 ( uInt32 addr )
{
  if (addr & 0x40000000)
  {
      return *((uInt8*) (myARMRAM + (addr & RAMADDMASK)));
  }
  else
  {
      return *((uInt8*)&cart_buffer[addr]);
  }
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline void Thumbulator::do_cflag ( uInt32 a, uInt32 b, uInt32 c )
{
  if (((a|b) & 0x80000000) == 0) cFlag=0;
  else if ((a&b) & 0x80000000) cFlag=1;
  else
  {
      uInt32 rc=(a&0x7FFFFFFF)+(b&0x7FFFFFFF)+c; //carry in
      cFlag = (rc & 0x80000000) ? 1:0;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline void Thumbulator::do_cflag_fast ( uInt32 a, uInt32 b )  // For when you know the high bit of b is set (i.e. on CMP). Assumes carry in.
{
  if (a & 0x80000000) cFlag=1;
  else
  {
      uInt32 rc=a+(b&0x7FFFFFFF)+1;
      cFlag = (rc & 0x80000000) ? 1:0;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline uInt32 Thumbulator::do_sub_vflag ( uInt32 a, uInt32 b, uInt32 c )
{
  //if the sign bits are different and result matches b
  return ((((a^b)&0x80000000)) & ((b&c&0x80000000)));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline uInt32 Thumbulator::do_add_vflag ( uInt32 a, uInt32 b, uInt32 c )
{
  //if sign bits are the same and the result is different
  return (((a&b&0x80000000)) & (((b^c)&0x80000000)));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Thumbulator::Op Thumbulator::decodeInstructionWord(uint16_t inst) 
{
  //ADC
  if((inst & 0xFFC0) == 0x4140) return Op::adc;

  //ADD(1) small immediate two registers
  if((inst & 0xFE00) == 0x1C00 && (inst >> 6) & 0x7) return Op::add1;

  //ADD(2) big immediate one register
  if((inst & 0xF800) == 0x3000) 
  {
      if ((inst & 0xFF) == 1)
      {
          return Op::incr;
      }
      else
      {
          switch (inst & 0x0700)
          {
              case 0x0000:  return Op::add2_r0;
              case 0x0100:  return Op::add2_r1;
              case 0x0200:  return Op::add2_r2;
              case 0x0300:  return Op::add2_r3;
              case 0x0400:  return Op::add2_r4;
              case 0x0500:  return Op::add2_r5;
              case 0x0600:  return Op::add2_r6;
              case 0x0700:  return Op::add2_r7;
          }
          return Op::add2_r0;
      }
  }

  //ADD(3) three registers
  if((inst & 0xFE00) == 0x1800) return Op::add3;

  //ADD(4) two registers one or both high no flags
  if((inst & 0xFF00) == 0x4400) return Op::add4;

  //ADD(5) rd = pc plus immediate
  if((inst & 0xF800) == 0xA000) return Op::add5;

  //ADD(6) rd = sp plus immediate
  if((inst & 0xF800) == 0xA800) return Op::add6;

  //ADD(7) sp plus immediate
  if((inst & 0xFF80) == 0xB000) return Op::add7;

  //AND
  if((inst & 0xFFC0) == 0x4000) return Op::and_;

  //ASR(1) two register immediate
  if((inst & 0xF800) == 0x1000) return Op::asr1;

  //ASR(2) two register
  if((inst & 0xFFC0) == 0x4100) return Op::asr2;

  //B(1) conditional branch
  if((inst & 0xF000) == 0xD000) 
  {
      switch(inst & 0x0F00)
      {
          case 0x000: 
              {
                 if (inst & 0x80) return Op::b1_000_neg;
                 return Op::b1_000_pos;
              }
          case 0x100:
              {
                 if (inst & 0x80) return Op::b1_100_neg;
                 return Op::b1_100_pos;
              }
          case 0x200: return Op::b1_200;
          case 0x300: return Op::b1_300;
          case 0x400: return Op::b1_400;
          case 0x500: 
              {
                  if (inst & 0x80) return Op::b1_500_neg;
                  return Op::b1_500_pos;
              }
          case 0x600: return Op::b1_600;
          case 0x700: return Op::b1_700;
          case 0x800: return Op::b1_800;
          case 0x900: return Op::b1_900;
          case 0xA00: return Op::b1_a00;
          case 0xB00: return Op::b1_b00;
          case 0xC00: return Op::b1_c00;
          case 0xD00: return Op::b1_d00;
          case 0xE00: return Op::b1_e00;
          case 0xF00: return Op::b1_f00;
      }
      return Op::b1_f00;
  }

  //B(2) unconditional branch
  if((inst & 0xF800) == 0xE000) 
  {
      if (inst&(1<<10)) return Op::b2_neg;
      return Op::b2_pos;
  }

  //BIC
  if((inst & 0xFFC0) == 0x4380) return Op::bic;

  //BKPT
  if((inst & 0xFF00) == 0xBE00) return Op::bkpt;

  //BL/BLX(1)
  if((inst & 0xE000) == 0xE000) return Op::blx1;

  //BLX(2)
  if((inst & 0xFF87) == 0x4780) return Op::blx2;

  //BX
  if((inst & 0xFF87) == 0x4700) return Op::bx;

  //CMN
  if((inst & 0xFFC0) == 0x42C0) return Op::cmn;

  //CMP(1) compare immediate
  if((inst & 0xF800) == 0x2800) 
  {
      switch (inst & 0x0700)
      {
          case 0x0000:  return Op::cmp1_r0;
          case 0x0100:  return Op::cmp1_r1;
          case 0x0200:  return Op::cmp1_r2;
          case 0x0300:  return Op::cmp1_r3;
          case 0x0400:  return Op::cmp1_r4;
          case 0x0500:  return Op::cmp1_r5;
          case 0x0600:  return Op::cmp1_r6;
          case 0x0700:  return Op::cmp1_r7;
      }
      return Op::cmp1_r0;
  }

  //CMP(2) compare register
  if((inst & 0xFFC0) == 0x4280) 
  {
      if (((inst>>3)&0x7) == 2) return Op::cmp2_r2;
      if (((inst>>3)&0x7) == 3) return Op::cmp2_r3;
      return Op::cmp2;
  }

  //CMP(3) compare high register
  if((inst & 0xFF00) == 0x4500) return Op::cmp3;

  //CPS
  if((inst & 0xFFE8) == 0xB660) return Op::cps;

  //CPY copy high register
  if((inst & 0xFFC0) == 0x4600) return Op::cpy;

  //EOR
  if((inst & 0xFFC0) == 0x4040) return Op::eor;

  //LDMIA
  if((inst & 0xF800) == 0xC800) return Op::ldmia;

  //LDR(1) two register immediate
  if((inst & 0xF800) == 0x6800) 
  {
      switch (inst & 0x0007)
      {
          case 0x0000:  return Op::ldr1_r0;
          case 0x0001:  return Op::ldr1_r1;
          case 0x0002:  return Op::ldr1_r2;
          case 0x0003:  return Op::ldr1_r3;
          case 0x0004:  return Op::ldr1_r4;
          case 0x0005:  return Op::ldr1_r5;
          case 0x0006:  return Op::ldr1_r6;
          case 0x0007:  return Op::ldr1_r7;
      }
      return Op::ldr1_r0;
  }

  //LDR(2) three register
  if((inst & 0xFE00) == 0x5800) return Op::ldr2;

  //LDR(3)
  if((inst & 0xF800) == 0x4800) 
  {
      switch (inst & 0x0700)
      {
          case 0x0000:  return Op::ldr3_r0;
          case 0x0100:  return Op::ldr3_r1;
          case 0x0200:  return Op::ldr3_r2;
          case 0x0300:  return Op::ldr3_r3;
          case 0x0400:  return Op::ldr3_r4;
          case 0x0500:  return Op::ldr3_r5;
          case 0x0600:  return Op::ldr3_r6;
          case 0x0700:  return Op::ldr3_r7;
      }
      return Op::ldr3_r0;
  }

  //LDR(4)
  if((inst & 0xF800) == 0x9800) 
  {
      switch (inst & 0x0700)
      {
          case 0x0000:  return Op::ldr4_r0;
          case 0x0100:  return Op::ldr4_r1;
          case 0x0200:  return Op::ldr4_r2;
          case 0x0300:  return Op::ldr4_r3;
          case 0x0400:  return Op::ldr4_r4;
          case 0x0500:  return Op::ldr4_r5;
          case 0x0600:  return Op::ldr4_r6;
          case 0x0700:  return Op::ldr4_r7;
      }
      return Op::ldr4_r0;
  }

  //LDRB(1)
  if((inst & 0xF800) == 0x7800) return Op::ldrb1;

  //LDRB(2)
  if((inst & 0xFE00) == 0x5C00) return Op::ldrb2;

  //LDRH(1)
  if((inst & 0xF800) == 0x8800) return Op::ldrh1;

  //LDRH(2)
  if((inst & 0xFE00) == 0x5A00) return Op::ldrh2;

  //LDRSB
  if((inst & 0xFE00) == 0x5600) return Op::ldrsb;

  //LDRSH
  if((inst & 0xFE00) == 0x5E00) return Op::ldrsh;

  //LSL(1)
  if((inst & 0xF800) == 0x0000) 
  {
      if ((inst>>6) & 0x1F) return Op::lsl1_rb;
      return Op::lsl1;
  }

  //LSL(2) two register
  if((inst & 0xFFC0) == 0x4080) return Op::lsl2;

  //LSR(1) two register immediate
  if((inst & 0xF800) == 0x0800) return Op::lsr1;

  //LSR(2) two register
  if((inst & 0xFFC0) == 0x40C0) return Op::lsr2;

  //MOV(1) immediate
  if((inst & 0xF800) == 0x2000)
  {
      if ((inst & 0xFF) == 0) return Op::mov1z;
      return Op::mov1;
  }

  //MOV(2) two low registers
  if((inst & 0xFFC0) == 0x1C00) return Op::mov2;

  //MOV(3)
  if((inst & 0xFF00) == 0x4600) 
  {
      if ((((inst>>0)&0x7) | ((inst>>4)&0x8)) == 15) return Op::mov3_r15;
      return Op::mov3;
  }

  //MUL
  if((inst & 0xFFC0) == 0x4340) return Op::mul;

  //MVN
  if((inst & 0xFFC0) == 0x43C0) return Op::mvn;

  //NEG
  if((inst & 0xFFC0) == 0x4240) return Op::neg;

  //ORR
  if((inst & 0xFFC0) == 0x4300) return Op::orr;

  //POP
  if((inst & 0xFE00) == 0xBC00) return Op::pop;

  //PUSH
  if((inst & 0xFE00) == 0xB400) return Op::push;

  //REV
  if((inst & 0xFFC0) == 0xBA00) return Op::rev;

  //REV16
  if((inst & 0xFFC0) == 0xBA40) return Op::rev16;

  //REVSH
  if((inst & 0xFFC0) == 0xBAC0) return Op::revsh;

  //ROR
  if((inst & 0xFFC0) == 0x41C0) return Op::ror;

  //SBC
  if((inst & 0xFFC0) == 0x4180) return Op::sbc;

  //SETEND
  if((inst & 0xFFF7) == 0xB650) return Op::setend;

  //STMIA
  if((inst & 0xF800) == 0xC000) return Op::stmia;

  //STR(1)
  if((inst & 0xF800) == 0x6000) return Op::str1;

  //STR(2)
  if((inst & 0xFE00) == 0x5000) return Op::str2;

  //STR(3)
  if((inst & 0xF800) == 0x9000) 
  {
      if (((inst>>8)&0x07) == 2) return Op::str3_r2;
      if (((inst>>8)&0x07) == 3) return Op::str3_r3;
      return Op::str3;
  }

  //STRB(1)
  if((inst & 0xF800) == 0x7000) return Op::strb1;

  //STRB(2)
  if((inst & 0xFE00) == 0x5400) return Op::strb2;

  //STRH(1)
  if((inst & 0xF800) == 0x8000) return Op::strh1;

  //STRH(2)
  if((inst & 0xFE00) == 0x5200) return Op::strh2;

  //SUB(1)
  if((inst & 0xFE00) == 0x1E00) return Op::sub1;

  //SUB(2)
  if((inst & 0xF800) == 0x3800) return Op::sub2;

  //SUB(3)
  if((inst & 0xFE00) == 0x1A00) return Op::sub3;

  //SUB(4)
  if((inst & 0xFF80) == 0xB080) return Op::sub4;

  //SWI
  if((inst & 0xFF00) == 0xDF00) return Op::swi;

  //SXTB
  if((inst & 0xFFC0) == 0xB240) return Op::sxtb;

  //SXTH
  if((inst & 0xFFC0) == 0xB200) return Op::sxth;

  //TST
  if((inst & 0xFFC0) == 0x4200) return Op::tst;

  //UXTB
  if((inst & 0xFFC0) == 0xB2C0) return Op::uxtb;

  //UXTH
  if((inst & 0xFFC0) == 0xB280) return Op::uxth;

  return Op::invalid;
}



// ------------------------------------------------------------------------
// Somewhere between 10-20% of instructions modify the R15 PC register 
// so we default to not updating it unless we know we're using it.
// This produces a small but meaningful speed-up of Thumb processing...
// ------------------------------------------------------------------------
#define FIX_R15_PC reg_sys[15] = ((u32) ((uInt8*)thumb_ptr - (uInt8*)cart_buffer)) + 2;

#define FIX_THUMB_PTRS  thumb_ptr = (uInt16*)&cart_buffer[(reg_sys[15]-2)]; thumb_decode_ptr = &cart_buffer[MEM_256KB + ((reg_sys[15]-2) >> 1)];


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 void Thumbulator::execute ( void )
{
  uInt32 ZNflags = 0;
  uInt32 vFlag = 0;
  uInt16 *thumb_ptr = (uInt16*)&cart_buffer[(reg_sys[15]-2)];
  uInt8  *thumb_decode_ptr = &cart_buffer[MEM_256KB+((reg_sys[15]-2) >> 1)];

  for (;;)
  {
      uInt32 ra,rb,rc,rd,rm,rn;
      uInt16 inst = *thumb_ptr++;
      Thumbulator::Op decoded = (Thumbulator::Op)*thumb_decode_ptr++;
      
#ifdef CPU_PROFILER
      profiler[(u8)decoded]++;
#endif
      
      switch (decoded)
      {
          case Op::b1_000_pos:  //B(1) conditional branch
                if (!ZNflags)
                {
                    thumb_ptr += (int)(inst & 0xFF)+1;
                    thumb_decode_ptr += (int)(inst & 0xFF)+1;
                }
              break;
              
          case Op::b1_000_neg:  //B(1) conditional branch
                if (!ZNflags)
                {
                    thumb_ptr += (int)(inst | 0xFFFFFF00)+1;;
                    thumb_decode_ptr += (int)(inst | 0xFFFFFF00)+1;;
                }
              break;
              
          case Op::b1_100_pos:  //B(1) conditional branch
                if (ZNflags)
                {
                    thumb_ptr += (int)(inst & 0xFF)+1;
                    thumb_decode_ptr += (int)(inst & 0xFF)+1;
                }
              break;

          case Op::b1_100_neg:  //B(1) conditional branch
                if (ZNflags)
                {
                    thumb_ptr += (int)(inst | 0xFFFFFF00)+1;;
                    thumb_decode_ptr += (int)(inst | 0xFFFFFF00)+1;;
                }
              break;
              
          case Op::b1_200:  //B(1) conditional branch
                if (cFlag)
                {
                    rb=(inst>>0)&0xFF;
                    if(rb&0x80) rb|=0xFFFFFF00; // Sign extend
                    thumb_ptr += (int)rb+1;
                    thumb_decode_ptr += (int)rb+1;
                }
              break;
              
          case Op::b1_300:  //B(1) conditional branch
                if(!(cFlag))
                {
                    rb=(inst>>0)&0xFF;
                    if(rb&0x80) rb|=0xFFFFFF00; // Sign extend
                    thumb_ptr += (int)rb+1;
                    thumb_decode_ptr += (int)rb+1;
                }
              break;
              
          case Op::b1_400:  //B(1) conditional branch
                if((ZNflags&0x80000000))
                {
                    rb=(inst>>0)&0xFF;
                    if(rb&0x80) rb|=0xFFFFFF00; // Sign extend
                    thumb_ptr += (int)rb+1;
                    thumb_decode_ptr += (int)rb+1;
                }
              break;
              
          case Op::b1_500_pos:  //B(1) conditional branch
                if(!((ZNflags&0x80000000)))
                {
                    rb=(inst&0xFF)+1;
                    thumb_ptr += (int)rb;
                    thumb_decode_ptr += (int)rb;
                }
              break;

          case Op::b1_500_neg:  //B(1) conditional branch
                if(!((ZNflags&0x80000000)))
                {
                    rb=(inst | 0xFFFFFF00)+1;
                    thumb_ptr += (int)rb;
                    thumb_decode_ptr += (int)rb;
                }
              break;
              
          case Op::b1_600:                       //B(1) conditional branch
                if(vFlag)
                {
                    rb=(inst>>0)&0xFF;
                    if(rb&0x80) rb|=0xFFFFFF00; // Sign extend
                    thumb_ptr += (int)rb+1;
                    thumb_decode_ptr += (int)rb+1;
                }
              break;
              
          case Op::b1_700:                       //B(1) conditional branch
                if(!(vFlag))
                {
                    rb=(inst>>0)&0xFF;
                    if(rb&0x80) rb|=0xFFFFFF00; // Sign extend
                    thumb_ptr += (int)rb+1;
                    thumb_decode_ptr += (int)rb+1;
                }
              break;

          case Op::b1_800:                       //B(1) conditional branch
                if((cFlag)&&(ZNflags))
                {
                    rb=(inst>>0)&0xFF;
                    if(rb&0x80) rb|=0xFFFFFF00; // Sign extend
                    thumb_ptr += (int)rb+1;
                    thumb_decode_ptr += (int)rb+1;
                }
              break;
              
          case Op::b1_900:                       //B(1) conditional branch
                if((!ZNflags)||(!(cFlag)))
                {
                    rb=(inst>>0)&0xFF;
                    if(rb&0x80) rb|=0xFFFFFF00; // Sign extend
                    thumb_ptr += (int)rb+1;
                    thumb_decode_ptr += (int)rb+1;
                }
              break;

          case Op::b1_a00:                       //B(1) conditional branch
                ra=0;
                if(  ((ZNflags&0x80000000)) &&  (vFlag) ) ra++;
                else if((!((ZNflags&0x80000000)))&&(!(vFlag))) ra++;
                if(ra)
                {
                    rb=(inst>>0)&0xFF;
                    if(rb&0x80) rb|=0xFFFFFF00; // Sign extend
                    thumb_ptr += (int)rb+1;
                    thumb_decode_ptr += (int)rb+1;
                }
              break;
              
          case Op::b1_b00:                       //B(1) conditional branch
                ra=0;
                if((!((ZNflags&0x80000000)))&&(vFlag)) ra++;
                else if((!(vFlag))&&((ZNflags&0x80000000))) ra++;
                if(ra)
                {
                    rb=(inst>>0)&0xFF;
                    if(rb&0x80) rb|=0xFFFFFF00; // Sign extend
                    thumb_ptr += (int)rb+1;
                    thumb_decode_ptr += (int)rb+1;
                }
              break;
              
          case Op::b1_c00:                       //B(1) conditional branch
                ra=0;
                if(  ((ZNflags&0x80000000)) &&  (vFlag) ) ra++;
                else if((!((ZNflags&0x80000000)))&&(!(vFlag))) ra++;
                if(!ZNflags) ra=0;
                if(ra)
                {
                    rb=(inst>>0)&0xFF;
                    if(rb&0x80) rb|=0xFFFFFF00; // Sign extend
                    thumb_ptr += (int)rb+1;
                    thumb_decode_ptr += (int)rb+1;
                }
              break;
              
          case Op::b1_d00:                       //B(1) conditional branch
                ra=0;
                if((!((ZNflags&0x80000000)))&&(vFlag)) ra++;
                else if((!(vFlag))&&((ZNflags&0x80000000))) ra++;
                else if(!ZNflags) ra++;
                if(ra)
                {
                    rb=(inst>>0)&0xFF;
                    if(rb&0x80) rb|=0xFFFFFF00; // Sign extend
                    thumb_ptr += (int)rb+1;
                    thumb_decode_ptr += (int)rb+1;
                }
              break;
              
          case Op::b1_e00:                       //B(1) conditional branch
          case Op::b1_f00:                       //B(1) conditional branch
              return;
              
          case Op::b2_pos:  //B(2) unconditional branch no sign extend
                thumb_ptr += (inst&0x7FF)+1;
                thumb_decode_ptr += (inst&0x7FF)+1;
              break;
              
          case Op::b2_neg:  //B(2) unconditional branch - sign extend
                thumb_ptr += (int)(inst | 0xFFFFF800)+1;
                thumb_decode_ptr += (int)(inst | 0xFFFFF800)+1;
              break;
              
          case Op::blx1: //BL/BLX(1)
                FIX_R15_PC  
                if((inst&0x1800)==0x1000) //H=b10
                {
                    rb = inst & ((1 << 11) - 1);
                    if(rb & 1<<10) rb |= (~((1 << 11) - 1)); //sign extend
                    rb <<= 12;
                    rb += reg_sys[15];
                    write_register(14, rb);
                }
                else if((inst&0x1800)==0x1800) //H=b11
                {
                  //branch to thumb
                  rb = read_register(14);
                  rb += (inst & ((1 << 11) - 1)) << 1;
                  rb += 2;
                  write_register(14, (reg_sys[15]-2) | 1);
                  write_register(15, rb);
                  FIX_THUMB_PTRS
                }
                else if((inst&0x1800)==0x0800) //H=b01
                {
                  // fxq: this should exit the code without having to detect it
                  rb = read_register(14);
                  rb += (inst & ((1 << 11) - 1)) << 1;
                  rb &= 0xFFFFFFFC;
                  rb += 2;
                  write_register(14, (reg_sys[15]-2) | 1);
                  write_register(15, rb);
                  FIX_THUMB_PTRS
                }
              break;

          case Op::ldmia:
                rn=(inst>>8)&0x7;
                rm=read_register(rn);
                for(ra=0,rb=0x01;rb;rb=(rb<<1)&0xFF,ra++)
                {
                  if(inst&rb)
                  {
                    write_register(ra,readRAM32(rm));
                    rm+=4;
                  }
                }
                write_register(rn,rm);
              break;
              
          case Op::stmia:
                rn=(inst>>8)&0x7;
                rm=read_register(rn);
                for(ra=0,rb=0x01;rb;rb=(rb<<1)&0xFF,ra++)
                {
                  if(inst&rb)
                  {
                    write32(rm,read_register(ra));
                    rm+=4;
                  }
                }
                write_register(rn,rm);
              break;
              
          case Op::swi:
                return;
              
          case Op::push:
                  if (inst & 0x100) {reg_sys[13] -= 4; write32(reg_sys[13], reg_sys[14]);}
                  if (inst & 0x080) {reg_sys[13] -= 4; write32(reg_sys[13], reg_sys[7]);}
                  if (inst & 0x040) {reg_sys[13] -= 4; write32(reg_sys[13], reg_sys[6]);}
                  if (inst & 0x020) {reg_sys[13] -= 4; write32(reg_sys[13], reg_sys[5]);}
                  if (inst & 0x010) {reg_sys[13] -= 4; write32(reg_sys[13], reg_sys[4]);}
                  if (inst & 0x008) {reg_sys[13] -= 4; write32(reg_sys[13], reg_sys[3]);}
                  if (inst & 0x004) {reg_sys[13] -= 4; write32(reg_sys[13], reg_sys[2]);}
                  if (inst & 0x002) {reg_sys[13] -= 4; write32(reg_sys[13], reg_sys[1]);}
                  if (inst & 0x001) {reg_sys[13] -= 4; write32(reg_sys[13], reg_sys[0]);}
              break;

          case Op::pop:
                  if (inst & 0x001) {reg_sys[0] =  readRAM32(reg_sys[13]); reg_sys[13] += 4;}
                  if (inst & 0x002) {reg_sys[1] =  readRAM32(reg_sys[13]); reg_sys[13] += 4;}
                  if (inst & 0x004) {reg_sys[2] =  readRAM32(reg_sys[13]); reg_sys[13] += 4;}
                  if (inst & 0x008) {reg_sys[3] =  readRAM32(reg_sys[13]); reg_sys[13] += 4;}
                  if (inst & 0x010) {reg_sys[4] =  readRAM32(reg_sys[13]); reg_sys[13] += 4;}
                  if (inst & 0x020) {reg_sys[5] =  readRAM32(reg_sys[13]); reg_sys[13] += 4;}
                  if (inst & 0x040) {reg_sys[6] =  readRAM32(reg_sys[13]); reg_sys[13] += 4;}
                  if (inst & 0x080) {reg_sys[7] =  readRAM32(reg_sys[13]); reg_sys[13] += 4;}
                  if (inst & 0x100) 
                  {
                      reg_sys[15] = readRAM32(reg_sys[13]) + 2; 
                      reg_sys[13] += 4;
                      FIX_THUMB_PTRS
                  }
              break;
              
          case Op::ldr4_r0:
                write_register(0,readRAM32((read_register(13) + ((inst<<2)&0x3FF))));
              break;

          case Op::ldr4_r1:
                write_register(1,readRAM32((read_register(13) + ((inst<<2)&0x3FF))));
              break;
              
          case Op::ldr4_r2:
                write_register(2,readRAM32((read_register(13) + ((inst<<2)&0x3FF))));
              break;

          case Op::ldr4_r3:
                write_register(3,readRAM32((read_register(13) + ((inst<<2)&0x3FF))));
              break;

          case Op::ldr4_r4:
                write_register(4,readRAM32((read_register(13) + ((inst<<2)&0x3FF))));
              break;

          case Op::ldr4_r5:
                write_register(5,readRAM32((read_register(13) + ((inst<<2)&0x3FF))));
              break;

          case Op::ldr4_r6:
                write_register(6,readRAM32((read_register(13) + ((inst<<2)&0x3FF))));
              break;
              
          case Op::ldr4_r7:
                write_register(7,readRAM32((read_register(13) + ((inst<<2)&0x3FF))));
              break;

          case Op::str3:
                rb=read_register(13) + (((inst>>0)&0xFF)<<2);
                write32(rb,read_register(((inst>>8)&0x07)));
              break;

          case Op::str3_r2:
                write32((read_register(13)+((inst<<2)&0x3FF)),read_register(2));
              break;
              
          case Op::str3_r3:
                write32((read_register(13)+((inst<<2)&0x3FF)),read_register(3));
              break;
              
          case Op::ldr1_r0:
                reg_sys[0] = read32(reg_sys[(inst>>3)&0x07] + ((inst>>4)&0x7C));
              break;
              
          case Op::ldr1_r1:
                reg_sys[1] = read32(reg_sys[(inst>>3)&0x07] + ((inst>>4)&0x7C));
              break;

          case Op::ldr1_r2:
                reg_sys[2] = read32(reg_sys[(inst>>3)&0x07] + ((inst>>4)&0x7C));
              break;
              
          case Op::ldr1_r3:
                reg_sys[3] = read32(reg_sys[(inst>>3)&0x07] + ((inst>>4)&0x7C));
              break;
              
          case Op::ldr1_r4:
                reg_sys[4] = read32(reg_sys[(inst>>3)&0x07] + ((inst>>4)&0x7C));
              break;
              
          case Op::ldr1_r5:
                reg_sys[5] = read32(reg_sys[(inst>>3)&0x07] + ((inst>>4)&0x7C));
              break;
              
          case Op::ldr1_r6:
                reg_sys[6] = read32(reg_sys[(inst>>3)&0x07] + ((inst>>4)&0x7C));
              break;

          case Op::ldr1_r7:
                reg_sys[7] = read32(reg_sys[(inst>>3)&0x07] + ((inst>>4)&0x7C));
              break;
              
          case Op::strb1:
                rb=(inst>>6)&0x1F;
                rb=read_register((inst>>3)&0x07)+rb;
                rc=read_register((inst>>0)&0x07);
                write8(rb, rc);
              break;

          case Op::ldrb1:
                rd=(inst>>0)&0x07;
                rb=read_register(((inst>>3) & 0x07))+((inst>>6)&0x1F);
                write_register(rd, read8(rb));
              break;

          case Op::str1:
                rb=(inst>>4)&0x7C;
                rb=read_register((inst>>3)&0x07)+rb;
                rc=read_register((inst>>0)&0x07);
                write32(rb,rc);
              break;
              
          case Op::strb2:
                rb=read_register((inst>>3)&0x7)+read_register((inst>>6)&0x7);
                rc=read_register((inst>>0)&0x7);
                write8(rb, rc);
              break;
              
          case Op::ldrb2:
                rb=read_register((inst>>3)&0x7)+read_register((inst>>6)&0x7);
                write_register((inst>>0)&0x7,read8(rb));
              break;
              
          case Op::ldrsh:
                rd=(inst>>0)&0x7;
                rn=(inst>>3)&0x7;
                rm=(inst>>6)&0x7;
                rb=read_register(rn)+read_register(rm);
                rc=read16(rb);
                rc&=0xFFFF;
                if(rc&0x8000) rc|=((~0)<<16);
                write_register(rd,rc);
              break;
              
          case Op::ldrh2:
                rd=(inst>>0)&0x7;
                rn=(inst>>3)&0x7;
                rm=(inst>>6)&0x7;
                rb=read_register(rn)+read_register(rm);
                rc=read16(rb);
                write_register(rd,rc&0xFFFF);
              break;
              
          case Op::ldrsb:
                rd=(inst>>0)&0x7;
                rn=(inst>>3)&0x7;
                rm=(inst>>6)&0x7;
                rb=read_register(rn)+read_register(rm);
                rc=read8(rb);
                if(rc&0x80) rc|=((~0)<<8);
                write_register(rd,rc);
              break;
              
          case Op::ldr2:
                rd=(inst>>0)&0x7;
                rn=(inst>>3)&0x7;
                rm=(inst>>6)&0x7;
                rb=read_register(rn)+read_register(rm);
                rc=read32(rb);
                write_register(rd,rc);
              break;
              
          case Op::str2:
                rd=(inst>>0)&0x7;
                rn=(inst>>3)&0x7;
                rm=(inst>>6)&0x7;
                rb=read_register(rn)+read_register(rm);
                rc=read_register(rd);
                write32(rb,rc);
              break;
              
          case Op::strh2:
                rd=(inst>>0)&0x7;
                rn=(inst>>3)&0x7;
                rm=(inst>>6)&0x7;
                rb=read_register(rn)+read_register(rm);
                rc=read_register(rd);
                write16(rb,rc&0xFFFF);
              break;

          case Op::ldr3_r0:
                reg_sys[0] = (u32) *(u32*)((((u32)thumb_ptr + ((inst<<2)&0x3FF) + 2) & ~3));
              break;

          case Op::ldr3_r1:
                reg_sys[1] = (u32) *(u32*)((((u32)thumb_ptr + ((inst<<2)&0x3FF) + 2) & ~3));
              break;

          case Op::ldr3_r2:
                reg_sys[2] = (u32) *(u32*)((((u32)thumb_ptr + ((inst<<2)&0x3FF) + 2) & ~3));
              break;

          case Op::ldr3_r3:
                reg_sys[3] = (u32) *(u32*)((((u32)thumb_ptr + ((inst<<2)&0x3FF) + 2) & ~3));
              break;

          case Op::ldr3_r4:
                reg_sys[4] = (u32) *(u32*)((((u32)thumb_ptr + ((inst<<2)&0x3FF) + 2) & ~3));
              break;

          case Op::ldr3_r5:
                reg_sys[5] = (u32) *(u32*)((((u32)thumb_ptr + ((inst<<2)&0x3FF) + 2) & ~3));
              break;

          case Op::ldr3_r6:
                reg_sys[6] = (u32) *(u32*)((((u32)thumb_ptr + ((inst<<2)&0x3FF) + 2) & ~3));
              break;
              
          case Op::ldr3_r7:
                reg_sys[7] = (u32) *(u32*)((((u32)thumb_ptr + ((inst<<2)&0x3FF) + 2) & ~3));
              break;
              
          case Op::and_:
                rd=(inst>>0)&0x7;
                reg_sys[rd] &= reg_sys[(inst>>3)&0x7];
                do_znflags(reg_sys[rd]);
              break;
              
              
          case Op::adc:
                rd=(inst>>0)&0x07;
                ra=read_register(rd);
                rb=read_register((inst>>3)&0x07);
                reg_sys[rd]=ra+rb+cFlag;
                do_znflags(reg_sys[rd]);
                do_cflag(ra,rb,cFlag);
#ifdef SAFE_THUMB                            
                vFlag = do_add_vflag(ra,rb,reg_sys[rd]);
#endif              
              break;
              
          case Op::add4:
                FIX_R15_PC
                rd=(inst>>0)&0x7;
                rd|=(inst>>4)&0x8;
                rm=(inst>>3)&0xF;
                ra=read_register(rd);
                rb=read_register(rm);
                rc=ra+rb;
                write_register(rd,rc);
              break;
              
          case Op::mov3:
                rd=(inst>>0)&0x7;
                rd|=(inst>>4)&0x8;
                rm=(inst>>3)&0xF;
                if (rm == 15)
                {
                    FIX_R15_PC
                }
                rc=read_register(rm);
                write_register(rd,rc);
              break;

          case Op::mov3_r15:
                rc=read_register((inst>>3)&0xF)+2;
                write_register(15,rc);
                FIX_THUMB_PTRS
              break;              
              
          case Op::bic:
                rd=(inst>>0)&0x7;
                rm=(inst>>3)&0x7;
                ra=read_register(rd);
                rb=read_register(rm);
                ZNflags=ra&(~rb);
                write_register(rd,ZNflags);
              break;
              
          case Op::bx:
                FIX_R15_PC  
                rm=(inst>>3)&0xF;
                rc=read_register(rm);
                if(rc&1)
                {
                  rc+=2;
                  write_register(15,rc);
                  FIX_THUMB_PTRS
                }
                else
                {
                        uInt32 pc = reg_sys[15] & 0xFFFFFFFE;
                        // branch to even address denotes 32 bit ARM code, which the Thumbulator
                        // class does not support. So capture relavent information and hand it
                        // off to the Cartridge class for it to handle.

                        bool handled = false;
                        // address to test for is + 4 due to pipelining
                        #define CDF1_SetNote     (0x00000752 + 4)
                        #define CDF1_ResetWave   (0x00000756 + 4)
                        #define CDF1_GetWavePtr  (0x0000075a + 4)
                        #define CDF1_SetWaveSize (0x0000075e + 4)

                        extern uInt32 CDFCallback(uInt8 function, uInt32 value1, uInt32 value2);

                        if      (pc == CDF1_SetNote)
                        {
                          CDFCallback(0, reg_sys[2], reg_sys[3]);
                          handled = true;
                        }
                        else if (pc == CDF1_ResetWave)
                        {
                          CDFCallback(1, read_register(2), 0);
                          handled = true;
                        }
                        else if (pc == CDF1_GetWavePtr)
                        {
                          reg_sys[2] = CDFCallback(2, reg_sys[2], 0);
                          handled = true;
                        }
                        else if (pc == CDF1_SetWaveSize)
                        {
                          CDFCallback(3, reg_sys[2], reg_sys[3]);
                          handled = true;
                        }
                        else if (pc == 0x0000083a)
                        {
                            // exiting Custom ARM code, returning to BUS Driver control
                        }
                        else
                        {
                            //myCartridge->thumbCallback(255, 0, 0);
                        }

                        if (handled)
                        {
                          rc = read_register(14) + 2; // lr
                          write_register(15, rc);
                          FIX_THUMB_PTRS
                        } 
                        else return;
                }
              break;
              
          case Op::sbc:
                rd=(inst>>0)&0x7;
                ra=read_register(rd);
                rb=read_register((inst>>3)&0x7);
                ZNflags=ra-rb;
                if(!(cFlag)) ZNflags--;
                write_register(rd,ZNflags);
                do_cflag(ra,rb,0);
#ifdef SAFE_THUMB                            
                vFlag = do_sub_vflag(ra,rb,ZNflags);
#endif              
              break;
              
          case Op::tst:

                ra=read_register((inst>>0)&0x7);
                rb=read_register((inst>>3)&0x7);
                ZNflags=ra&rb;
              break;
              
          case Op::asr2:
                rd=(inst>>0)&0x07;
                ra=(inst>>3)&0x07;
                ZNflags=read_register(rd);
                rb=read_register(ra);
                rb&=0xFF;
                if(rb==0)
                {
                }
                else if(rb<32)
                {
                  do_cflag_bit(ZNflags&(1<<(rb-1)));
                  ra=ZNflags&0x80000000;
                  ZNflags>>=rb;
                  if(ra) //asr, sign is shifted in
                  {
                    ZNflags|=(~0)<<(32-rb);
                  }
                }
                else
                {
                  if(ZNflags&0x80000000)
                  {
                    do_cflag_bit(1);
                    ZNflags=(~0);
                  }
                  else
                  {
                    do_cflag_bit(0);
                    ZNflags=0;
                  }
                }
                write_register(rd,ZNflags);
              break;
              
              
          case Op::cmp3:
                FIX_R15_PC
                rn=(inst>>0)&0x7;
                rn|=(inst>>4)&0x8;
                rm=(inst>>3)&0xF;
                ra=read_register(rn);
                rb=read_register(rm);
                ZNflags=ra-rb;
                do_cflag(ra,~rb,1);
#ifdef SAFE_THUMB                            
                vFlag = do_sub_vflag(ra,rb,ZNflags);
#endif              
              break;
              
          case Op::mul:
                rd=(inst>>0)&0x7;
                ra=read_register(rd);
                rb=read_register((inst>>3)&0x7);
                ZNflags=ra*rb;
                write_register(rd,ZNflags);
              break;
              
          case Op::mvn:
                rd=(inst>>0)&0x7;
                rm=(inst>>3)&0x7;
                ra=read_register(rm);
                ZNflags=(~ra);
                write_register(rd,ZNflags);
              break;
              
          case Op::neg:
                rd=(inst>>0)&0x7;
                rm=(inst>>3)&0x7;
                ra=read_register(rm);
                ZNflags=0-ra;
                write_register(rd,ZNflags);
                do_cflag(0,~ra,1);
#ifdef SAFE_THUMB
                vFlag = do_sub_vflag(0,ra,ZNflags);
#endif
              break;
              
          case Op::orr:
                rd=(inst>>0)&0x7;
                rm=(inst>>3)&0x7;
                ra=read_register(rd);
                rb=read_register(rm);
                ZNflags=ra|rb;
                write_register(rd,ZNflags);
              break;
              
          case Op::cmn:
                rn=(inst>>0)&0x7;
                rm=(inst>>3)&0x7;
                ra=read_register(rn);
                rb=read_register(rm);
                ZNflags=ra+rb;
                do_cflag(ra,rb,0);
#ifdef SAFE_THUMB              
                vFlag = do_add_vflag(ra,rb,ZNflags);
#endif              
              break;
              
          case Op::eor:
                rd=(inst>>0)&0x7;
                rm=(inst>>3)&0x7;
                ra=read_register(rd);
                rb=read_register(rm);
                ZNflags=ra^rb;
                write_register(rd,ZNflags);
              break;
              
          case Op::lsl2:
                rd=(inst>>0)&0x07;
                ra=(inst>>3)&0x07;
                ZNflags=read_register(rd);
                rb=read_register(ra);
                rb&=0xFF;
                if(rb==0)
                {
                }
                else if(rb<32)
                {
                  do_cflag_bit(ZNflags&(1<<(32-rb)));
                  ZNflags<<=rb;
                }
                else if(rb==32)
                {
                  do_cflag_bit(ZNflags&1);
                  ZNflags=0;
                }
                else
                {
                  do_cflag_bit(0);
                  ZNflags=0;
                }
                write_register(rd,ZNflags);
              break;
              
          case Op::lsr2:
                rd=(inst>>0)&0x07;
                ra=(inst>>3)&0x07;
                ZNflags=read_register(rd);
                rb=read_register(ra);
                rb&=0xFF;
                if(rb==0)
                {
                }
                else if(rb<32)
                {
                  do_cflag_bit(ZNflags&(1<<(32-rb)));
                  ZNflags>>=rb;
                }
                else if(rb==32)
                {
                  do_cflag_bit(ZNflags&0x80000000);
                  ZNflags=0;
                }
                else
                {
                  do_cflag_bit(0);
                  ZNflags=0;
                }
                write_register(rd,ZNflags);
              break;
              
          case Op::ror:
                rd=(inst>>0)&0x7;
                ra=(inst>>3)&0x7;
                ZNflags=read_register(rd);
                ra=read_register(ra);
                ra&=0xFF;
                if(!ra)
                {
                  ra&=0x1F;
                  if(ra==0)
                  {
                    do_cflag_bit(ZNflags&0x80000000);
                  }
                  else
                  {
                    do_cflag_bit(ZNflags&(1<<(ra-1)));
                    rb=ZNflags<<(32-ra);
                    ZNflags>>=ra;
                    ZNflags|=rb;
                  }
                }
                write_register(rd,ZNflags);
              break;

          case Op::incr:
                do_znflags(++reg_sys[(inst & 0x0700) >> 8]);
              break;
              
          case Op::add2_r0:
                reg_sys[0] += (inst>>0)&0xFF;
                do_znflags(reg_sys[0]);
                // ------------------------------------------------------------------------------------------------------
                // TBD: This is incorrect (but faster) emulation on an instruction that is very common... we're adding 
                // a small number to a 32-bit register and we're going to assume that there is no vflag nor cFlag needed.
                // ------------------------------------------------------------------------------------------------------
                //{vFlag = do_add_vflag(ra,-rb,rc); do_cflag(ra,rb,0);}
              break;
          case Op::add2_r1:
                reg_sys[1] += (inst>>0)&0xFF;
                do_znflags(reg_sys[1]);
              break;
          case Op::add2_r2:
                reg_sys[2] += (inst>>0)&0xFF;
                do_znflags(reg_sys[2]);
              break;
          case Op::add2_r3:
                reg_sys[3] += (inst>>0)&0xFF;
                do_znflags(reg_sys[3]);
              break;
          case Op::add2_r4:
                reg_sys[4] += (inst>>0)&0xFF;
                do_znflags(reg_sys[4]);
              break;
          case Op::add2_r5:
                reg_sys[5] += (inst>>0)&0xFF;
                do_znflags(reg_sys[5]);
              break;
          case Op::add2_r6:
                reg_sys[6] += (inst>>0)&0xFF;
                do_znflags(reg_sys[6]);
              break;
          case Op::add2_r7:
                reg_sys[7] += (inst>>0)&0xFF;
                do_znflags(reg_sys[7]);
              break;
              
          case Op::cmp1_r0:
                rb=(inst>>0)&0xFF;
                ZNflags=reg_sys[0]-rb;
                do_cflag_fast(reg_sys[0],~rb);
#ifdef SAFE_THUMB              
                vFlag = do_sub_vflag(reg_sys[0],rb,ZNflags);
#endif              
              break;

          case Op::cmp1_r1:
                rb=(inst>>0)&0xFF;
                ZNflags=reg_sys[1]-rb;
                do_cflag_fast(reg_sys[1],~rb);
#ifdef SAFE_THUMB              
                vFlag = do_sub_vflag(reg_sys[1],rb,ZNflags);
#endif              
              break;
              
          case Op::cmp1_r2:
                rb=(inst>>0)&0xFF;
                ZNflags=reg_sys[2]-rb;
                do_cflag_fast(reg_sys[2],~rb);
#ifdef SAFE_THUMB              
                vFlag = do_sub_vflag(reg_sys[2],rb,ZNflags);
#endif              
              break;

          case Op::cmp1_r3:
                rb=(inst>>0)&0xFF;
                ZNflags=reg_sys[3]-rb;
                do_cflag_fast(reg_sys[3],~rb);
#ifdef SAFE_THUMB              
                vFlag = do_sub_vflag(reg_sys[3],rb,ZNflags);
#endif              
              break;

          case Op::cmp1_r4:
                rb=(inst>>0)&0xFF;
                ZNflags=reg_sys[4]-rb;
                do_cflag_fast(reg_sys[4],~rb);
#ifdef SAFE_THUMB              
                vFlag = do_sub_vflag(reg_sys[4],rb,ZNflags);
#endif              
              break;
              
          case Op::cmp1_r5:
                rb=(inst>>0)&0xFF;
                ZNflags=reg_sys[5]-rb;
                do_cflag_fast(reg_sys[5],~rb);
#ifdef SAFE_THUMB              
                vFlag = do_sub_vflag(reg_sys[5],rb,ZNflags);
#endif              
              break;
              
          case Op::cmp1_r6:
                rb=(inst>>0)&0xFF;
                ZNflags=reg_sys[6]-rb;
                do_cflag_fast(reg_sys[6],~rb);
#ifdef SAFE_THUMB              
                vFlag = do_sub_vflag(reg_sys[6],rb,ZNflags);
#endif              
              break;

          case Op::cmp1_r7:
                rb=(inst>>0)&0xFF;
                ZNflags=reg_sys[7]-rb;
                do_cflag_fast(reg_sys[7],~rb);
#ifdef SAFE_THUMB              
                vFlag = do_sub_vflag(reg_sys[7],rb,ZNflags);
#endif              
              break;
              
          case Op::cmp2_r2:
                rn=(inst>>0)&0x7;
                ZNflags=reg_sys[rn]-reg_sys[2];
                do_cflag(reg_sys[rn],~reg_sys[2],1);
#ifdef SAFE_THUMB              
                vFlag = do_sub_vflag(reg_sys[rn],reg_sys[2],ZNflags);
#endif              
              break;

          case Op::cmp2_r3:
                rn=(inst>>0)&0x7;
                ZNflags=reg_sys[rn]-reg_sys[3];
                do_cflag(reg_sys[rn],~reg_sys[3],1);
#ifdef SAFE_THUMB              
                vFlag = do_sub_vflag(reg_sys[rn],reg_sys[3],ZNflags);
#endif              
              break;

          case Op::cmp2:
                rn=(inst>>0)&0x7;
                rm=(inst>>3)&0x7;
                ZNflags=reg_sys[rn]-reg_sys[rm];
                do_cflag(reg_sys[rn],~reg_sys[rm],1);
#ifdef SAFE_THUMB              
                vFlag = do_sub_vflag(reg_sys[rn],reg_sys[rm],ZNflags);
#endif              
              break;
              
          case Op::add3:
                rd=(inst>>0)&0x7;
                rn=(inst>>3)&0x7;
                rm=(inst>>6)&0x7;
                reg_sys[rd] = reg_sys[rn] + reg_sys[rm];
                do_znflags(reg_sys[rd]);
#ifdef SAFE_THUMB              
                do_cflag(reg_sys[rn],reg_sys[rm],0);
                vFlag = do_add_vflag(reg_sys[rn],reg_sys[rm],reg_sys[rd]);
#endif              
              break;
              
          case Op::mov1:
                ZNflags=(inst>>0)&0xFF;
                write_register((inst>>8)&0x07,ZNflags);
              break;
              
          case Op::mov1z:
#ifdef SAFE_THUMB              
                ZNflags=0;
#endif              
                write_register((inst>>8)&0x07,0);
              break;
              
          case Op::sub3:
                rd=(inst>>0)&0x7;
                rn=(inst>>3)&0x7;
                rm=(inst>>6)&0x7;
                reg_sys[rd]=reg_sys[rn]-reg_sys[rm];
                do_znflags(reg_sys[rd]);
                do_cflag(reg_sys[rn],~reg_sys[rm],1);
#ifdef SAFE_THUMB              
                vFlag = do_sub_vflag(reg_sys[rn],reg_sys[rm],reg_sys[rd]);
#endif              
              break;

          case Op::asr1:
                rd=(inst>>0)&0x07;
                rm=(inst>>3)&0x07;
                rb=(inst>>6)&0x1F;
                ZNflags=read_register(rm);
                if(rb==0)
                {
                  if(ZNflags&0x80000000)
                  {
                    do_cflag_bit(1);
                    ZNflags=~0;
                  }
                  else
                  {
                    do_cflag_bit(0);
                    ZNflags=0;
                  }
                }
                else
                {
                  do_cflag_bit(ZNflags&(1<<(rb-1)));
                  ra=ZNflags&0x80000000;
                  ZNflags>>=rb;
                  if(ra) //asr, sign is shifted in
                  {
                    ZNflags|=(~0)<<(32-rb);
                  }
                }
                write_register(rd,ZNflags);
              break;

          case Op::lsl1:
                rm=(inst>>3)&0x07;
                ZNflags = reg_sys[inst&0x07] = reg_sys[rm];
              break;
              
          case Op::lsl1_rb:
                rd=(inst>>0)&0x07;
                rm=(inst>>3)&0x07;
                rb=(inst>>6)&0x1F;
                ZNflags=read_register(rm);
                do_cflag_bit(ZNflags&(1<<(32-rb)));
                ZNflags<<=rb;
                write_register(rd,ZNflags);
              break;

          case Op::lsr1:
                rd=(inst>>0)&0x07;
                rm=(inst>>3)&0x07;
                rb=(inst>>6)&0x1F;
                ZNflags=read_register(rm);
                if(rb==0)
                {
                  do_cflag_bit(ZNflags&0x80000000);
                  ZNflags=0;
                }
                else
                {
                  do_cflag_bit(ZNflags&(1<<(rb-1)));
                  ZNflags>>=rb;
                }
                write_register(rd,ZNflags);
              break;
              
          case Op::sub2:
                rb=(inst>>0)&0xFF;
                rd=(inst>>8)&0x07;
                ra=read_register(rd);
                ZNflags=ra-rb;
                write_register(rd,ZNflags);
                do_cflag_fast(ra,~rb);
#ifdef SAFE_THUMB              
                vFlag = do_sub_vflag(ra,rb,ZNflags);
#endif              
              break;
              

          case Op::sub1:
                rd=(inst>>0)&7;
                rn=(inst>>3)&7;
                rb=(inst>>6)&7;
                ra=read_register(rn);
                ZNflags=ra-rb;
                write_register(rd,ZNflags);
                do_cflag_fast(ra,~rb);
#ifdef SAFE_THUMB              
                vFlag = do_sub_vflag(ra,rb,ZNflags);
#endif              
              break;
              
          case Op::add1:
                rd=(inst>>0)&0x7;
                rn=(inst>>3)&0x7;
                rb=(inst>>6)&0x7;
                if(rb)
                {
                  ra=read_register(rn);
                  ZNflags=ra+rb;
                  write_register(rd,ZNflags);
#ifdef SAFE_THUMB                    
                  do_cflag(ra,rb,0);
                  vFlag = do_add_vflag(ra,rb,ZNflags);
#endif                    
                }
                else
                {
                  //this is a mov
                }
              break;
              
          case Op::cpy:                     
                //same as mov except you can use both low registers
                //going to let mov handle high registers
                rd=(inst>>0)&0x7;
                rm=(inst>>3)&0x7;
                rc=read_register(rm);
                write_register(rd,rc);
              break;
              
          case Op::blx2:                     
                FIX_R15_PC  
                rm=(inst>>3)&0xF;
                rc=read_register(rm);
                if(rc&1)
                {
                  rc+=2;
                  write_register(14,reg_sys[15]-2);
                  write_register(15,rc);
                  FIX_THUMB_PTRS
                }
                else
                {
                  // fxq: this could serve as exit code
                  return;
                }
              break;
              
          case Op::mov2:                     
                rd=(inst>>0)&7;
                rn=(inst>>3)&7;
                ZNflags=read_register(rn);
                write_register(rd,ZNflags);
                do_cflag_bit(0);
                do_vflag_bit(0);
              break;
                            
          case Op::strh1:                    
                rd=(inst>>0)&0x07;
                rn=(inst>>3)&0x07;
                rb=(inst>>5)&0x3E;
                rb=read_register(rn)+rb;
                rc=read_register(rd);
                write16(rb,rc&0xFFFF);
              break;              
              
          case Op::add6:                    
                rb=(inst>>0)&0xFF;
                rd=(inst>>8)&0x7;
                rb<<=2;
                ra=read_register(13);
                rc=ra+rb;
                write_register(rd,rc);
              break;
              
          case Op::ldrh1:                    
                rd=(inst>>0)&0x07;
                rn=(inst>>3)&0x07;
                rb=(inst>>5)&0x3E;
                rb=read_register(rn)+rb;
                rc=read16(rb);
                write_register(rd,rc&0xFFFF);
              break;

          case Op::add7:                    
                rb=(inst>>0)&0x7F;
                rb<<=2;
                ra=read_register(13);
                rc=ra+rb;
                write_register(13,rc);
              break;
              
          case Op::sub4:                    
                rb=inst&0x7F;
                rb<<=2;
                ra=read_register(13);
                ra-=rb;
                write_register(13,ra);
              break;
              
          case Op::bkpt:                    
                return;
              break;
              
          case Op::cps:                    
                return;
              break;
              
          case Op::rev:                    
                rd=(inst>>0)&0x7;
                rn=(inst>>3)&0x7;
                ra=read_register(rn);
                rc =((ra>> 0)&0xFF)<<24;
                rc|=((ra>> 8)&0xFF)<<16;
                rc|=((ra>>16)&0xFF)<< 8;
                rc|=((ra>>24)&0xFF)<< 0;
                write_register(rd,rc);
              break;
              
          case Op::rev16:                    
                rd=(inst>>0)&0x7;
                rn=(inst>>3)&0x7;
                ra=read_register(rn);
                rc =((ra>> 0)&0xFF)<< 8;
                rc|=((ra>> 8)&0xFF)<< 0;
                rc|=((ra>>16)&0xFF)<<24;
                rc|=((ra>>24)&0xFF)<<16;
                write_register(rd,rc);
              break;

          case Op::revsh:                    
                rd=(inst>>0)&0x7;
                rn=(inst>>3)&0x7;
                ra=read_register(rn);
                rc =((ra>> 0)&0xFF)<< 8;
                rc|=((ra>> 8)&0xFF)<< 0;
                if(rc&0x8000) rc|=0xFFFF0000;
                else          rc&=0x0000FFFF;
                write_register(rd,rc);
              break;
              
          case Op::setend:                  
                return;
              break;
              
          case Op::sxtb:                    
                rd=(inst>>0)&0x7;
                rm=(inst>>3)&0x7;
                ra=read_register(rm);
                rc=ra&0xFF;
                if(rc&0x80) rc|=(~0)<<8;
                write_register(rd,rc);
              break;
              
          case Op::sxth:                    
                rd=(inst>>0)&0x7;
                rm=(inst>>3)&0x7;
                ra=read_register(rm);
                rc=ra&0xFFFF;
                if(rc&0x8000) rc|=(~0)<<16;
                write_register(rd,rc);
              break;
              
          case Op::add5:                    
                FIX_R15_PC
                rb=(inst>>0)&0xFF;
                rd=(inst>>8)&0x7;
                rb<<=2;
                ra=read_register(15);
                rc=(ra&(~3))+rb;
                write_register(rd,rc);
              break;
              
          case Op::uxtb:                    
                rd=(inst>>0)&0x7;
                rm=(inst>>3)&0x7;
                ra=read_register(rm);
                rc=ra&0xFF;
                write_register(rd,rc);
              break;
              
          case Op::uxth:                    
                rd=(inst>>0)&0x7;
                rm=(inst>>3)&0x7;
                ra=read_register(rm);
                rc=ra&0xFFFF;
                write_register(rd,rc);
              break;
                            
          case Op::invalid:                 
              return;
              break;              
      }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Thumbulator::reset ( void )
{
    reg_sys[13]=cStack;   //sp
    reg_sys[14]=cBase;    //lr (duz this use odd addrs)
    reg_sys[15]=cStart+3; //pc entry point of 0xc09+2 for DPC or 0x809+2 for CDF
}
