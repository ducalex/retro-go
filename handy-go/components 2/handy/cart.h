//
// Copyright (c) 2004 K. Wilkins
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//

//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                                 K. Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// Lynx cartridge class header file                                         //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition and code for some of  //
// the simpler cartridge API.                                               //
//                                                                          //
//    K. Wilkins                                                            //
// August 1997                                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// Revision History:                                                        //
// -----------------                                                        //
//                                                                          //
// 01Aug1997 KW Document header added & class documented.                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef CART_H
#define CART_H

#define EPYX_HEADER_OLD 512
#define EPYX_HEADER_NEW 410

#ifndef __max
#define __max(a,b) \
   ({ __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; })
#endif

#ifndef __min
#define __min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _b : _a; })
#endif

#ifdef TRACE_CART

#define TRACE_CART0(msg)					_RPT1(_CRT_WARN,"CCart::"msg" (Time=%012d)\n",gSystemCycleCount)
#define TRACE_CART1(msg,arg1)				_RPT2(_CRT_WARN,"CCart::"msg" (Time=%012d)\n",arg1,gSystemCycleCount)
#define TRACE_CART2(msg,arg1,arg2)			_RPT3(_CRT_WARN,"CCart::"msg" (Time=%012d)\n",arg1,arg2,gSystemCycleCount)
#define TRACE_CART3(msg,arg1,arg2,arg3)		_RPT4(_CRT_WARN,"CCart::"msg" (Time=%012d)\n",arg1,arg2,arg3,gSystemCycleCount)

#else

#define TRACE_CART0(msg)
#define TRACE_CART1(msg,arg1)
#define TRACE_CART2(msg,arg1,arg2)
#define TRACE_CART3(msg,arg1,arg2,arg3)

#endif

#define DEFAULT_CART_CONTENTS	0xFF

enum CTYPE {UNUSED,C64K,C128K,C256K,C512K,C1024K};

#define CART_NO_ROTATE		0
#define CART_ROTATE_LEFT	1
#define CART_ROTATE_RIGHT	2

typedef struct
{
   UBYTE   magic[4];
   UWORD   page_size_bank0;
   UWORD   page_size_bank1;
   UWORD   version;
   UBYTE   cartname[32];
   UBYTE   manufname[16];
   UBYTE   rotation;
   UBYTE   aud_bits;
   UBYTE   eeprom;
   UBYTE   spare[3];
}LYNX_HEADER;


class CCart : public CLynxBase
{

   // Function members

   public:
      CCart(UBYTE *gamedata,ULONG gamesize);
      ~CCart();

   public:

      // Access for sensible members of the clan

      void	Reset(void);
      bool	ContextSave(LSS_FILE *fp);
      bool	ContextLoad(LSS_FILE *fp);
      bool	ContextLoadLegacy(LSS_FILE *fp);

      void	Poke(ULONG addr,UBYTE data);
      UBYTE	Peek(ULONG addr);
      ULONG	ReadCycle(void) {return 15;};
      ULONG	WriteCycle(void) {return 15;};
      void	BankSelect(EMMODE newbank) {mBank=newbank;}
      ULONG	ObjectSize(void) {return (mBank==bank0)?mMaskBank0+1:mMaskBank1+1;};

      const char*	CartGetName(void) { return (const char*)mFileHeader.cartname;};
      const char*	CartGetManufacturer(void) { return (const char*)mFileHeader.manufname; };
      ULONG	CartGetRotate(void) {return (mFileHeader.rotation > 2) ? CART_NO_ROTATE : mFileHeader.rotation;};
      bool	CartGetAudin(void) { return mFileHeader.aud_bits&0x01;};
      UBYTE	CartGetEEPROMType(void) { return mFileHeader.eeprom;};
      ULONG	CRC32(void) { return mCRC32; };

      // Access for the lynx itself, it has no idea of address etc as this is done by the
      // cartridge emulation hardware
      void	CartAddressStrobe(bool strobe);
      void	CartAddressData(bool data);
      void	Poke0(UBYTE data);
      void	Poke1(UBYTE data);
      void	Poke0A(UBYTE data);
      void	Poke1A(UBYTE data);
      UBYTE	Peek0(void);
      UBYTE	Peek1(void);
      UBYTE	Peek0A(void);
      UBYTE	Peek1A(void);

      void SetShifterValue(UBYTE a){mShifter=a; mCounter=0;}; // for fake bios
      inline ULONG GetCounterValue(void) {return mCounter;}; // for eeprom

      // Data members

   public:
      ULONG	mWriteEnableBank1;

   private:
      EMMODE mBank;
      ULONG  mCartRAM;

      ULONG  mMaskBank0;
      ULONG  mMaskBank1;
      UBYTE	*mCartBank0;
      UBYTE	*mCartBank1;
      UBYTE	*mCartBank0A;
      UBYTE	*mCartBank1A;

      LYNX_HEADER mFileHeader;
      ULONG	      mCRC32;

      ULONG	mCounter;
      ULONG	mShifter;
      ULONG	mAddrData;
      ULONG	mStrobe;

      ULONG	mShiftCount0;
      ULONG	mCountMask0;
      ULONG	mShiftCount1;
      ULONG	mCountMask1;

      ULONG	mDummy;
};

#endif
