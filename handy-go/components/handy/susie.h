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
//	                  Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                                 K. Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// Susie object header file                                                 //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition for the Suzy class    //
// which provides math and sprite support to the emulator                   //
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

#ifndef SUSIE_H
#define SUSIE_H

#ifdef TRACE_SUSIE

#define TRACE_SUSIE0(msg)					_RPT1(_CRT_WARN,"CSusie::"msg" (Time=%012d)\n",gSystemCycleCount)
#define TRACE_SUSIE1(msg,arg1)				_RPT2(_CRT_WARN,"CSusie::"msg" (Time=%012d)\n",arg1,gSystemCycleCount)
#define TRACE_SUSIE2(msg,arg1,arg2)			_RPT3(_CRT_WARN,"CSusie::"msg" (Time=%012d)\n",arg1,arg2,gSystemCycleCount)
#define TRACE_SUSIE3(msg,arg1,arg2,arg3)	_RPT4(_CRT_WARN,"CSusie::"msg" (Time=%012d)\n",arg1,arg2,arg3,gSystemCycleCount)

#else

#define TRACE_SUSIE0(msg)
#define TRACE_SUSIE1(msg,arg1)
#define TRACE_SUSIE2(msg,arg1,arg2)
#define TRACE_SUSIE3(msg,arg1,arg2,arg3)

#endif

class CSystem;

#define SUSIE_START		0xfc00
#define SUSIE_SIZE		0x100

#define LINE_END		0x80

#define SPR_RDWR_CYC	3

//
// Define button values
//

#define BUTTON_A		0x0001
#define BUTTON_B		0x0002
#define BUTTON_OPT2		0x0004
#define BUTTON_OPT1		0x0008
#define BUTTON_LEFT		0x0010
#define BUTTON_RIGHT	0x0020
#define BUTTON_UP		0x0040
#define BUTTON_DOWN		0x0080
#define BUTTON_PAUSE	0x0100


enum {line_error=0,line_abs_literal,line_literal,line_packed};
enum {math_finished=0,math_divide,math_multiply,math_init_divide,math_init_multiply};

enum {sprite_background_shadow=0,
   sprite_background_noncollide,
   sprite_boundary_shadow,
   sprite_boundary,
   sprite_normal,
   sprite_noncollide,
   sprite_xor_shadow,
   sprite_shadow};

// Define register typdefs

typedef struct
{
   union
   {
      struct
      {
#ifdef MSB_FIRST
         UBYTE	High;
         UBYTE	Low;
#else
         UBYTE	Low;
         UBYTE	High;
#endif
      }Byte;
      UWORD	Word;
   };
}UUWORD;


typedef struct
{
   union
   {
      struct
      {
#ifdef MSB_FIRST
         UBYTE	Fc1:1;
         UBYTE	Fc2:1;
         UBYTE	Fc3:1;
         UBYTE	reserved:1;
         UBYTE	Ac1:1;
         UBYTE	Ac2:1;
         UBYTE	Ac3:1;
         UBYTE	Ac4:1;
#else
         UBYTE	Ac4:1;
         UBYTE	Ac3:1;
         UBYTE	Ac2:1;
         UBYTE	Ac1:1;
         UBYTE	reserved:1;
         UBYTE	Fc3:1;
         UBYTE	Fc2:1;
         UBYTE	Fc1:1;
#endif
      }Bits;
      UBYTE	Byte;
   };
}TSPRINIT;

typedef struct
{
   union
   {
      struct
      {
#ifdef MSB_FIRST
         UBYTE	Up:1;
         UBYTE	Down:1;
         UBYTE	Left:1;
         UBYTE	Right:1;
         UBYTE	Option1:1;
         UBYTE	Option2:1;
         UBYTE	Inside:1;
         UBYTE	Outside:1;
#else
         UBYTE	Outside:1;
         UBYTE	Inside:1;
         UBYTE	Option2:1;
         UBYTE	Option1:1;
         UBYTE	Right:1;
         UBYTE	Left:1;
         UBYTE	Down:1;
         UBYTE	Up:1;
#endif
      }Bits;
      UBYTE	Byte;
   };
}TJOYSTICK;

typedef struct
{
   union
   {
      struct
      {
#ifdef MSB_FIRST
         UBYTE	spare:5;
         UBYTE	Cart1IO:1;
         UBYTE	Cart0IO:1;
         UBYTE	Pause:1;
#else
         UBYTE	Pause:1;
         UBYTE	Cart0IO:1;
         UBYTE	Cart1IO:1;
         UBYTE	spare:5;
#endif
      }Bits;
      UBYTE	Byte;
   };
}TSWITCHES;

typedef struct
{
   union
   {
      struct
      {
#ifdef MSB_FIRST
         UBYTE	A;
         UBYTE	B;
         UBYTE	C;
         UBYTE	D;
#else
         UBYTE	D;
         UBYTE	C;
         UBYTE	B;
         UBYTE	A;
#endif
      }Bytes;
      struct
      {
#ifdef MSB_FIRST
         UWORD	AB;
         UWORD	CD;
#else
         UWORD	CD;
         UWORD	AB;
#endif
      }Words;
      ULONG	Long;
   };
}TMATHABCD;

typedef struct
{
   union
   {
      struct
      {
#ifdef MSB_FIRST
         UBYTE	E;
         UBYTE	F;
         UBYTE	G;
         UBYTE	H;
#else
         UBYTE	H;
         UBYTE	G;
         UBYTE	F;
         UBYTE	E;
#endif
      }Bytes;
      struct
      {
#ifdef MSB_FIRST
         UWORD	EF;
         UWORD	GH;
#else
         UWORD	GH;
         UWORD	EF;
#endif
      }Words;
      ULONG	Long;
   };
}TMATHEFGH;

typedef struct
{
   union
   {
      struct
      {
#ifdef MSB_FIRST
         UBYTE	J;
         UBYTE	K;
         UBYTE	L;
         UBYTE	M;
#else
         UBYTE	M;
         UBYTE	L;
         UBYTE	K;
         UBYTE	J;
#endif
      }Bytes;
      struct
      {
#ifdef MSB_FIRST
         UWORD	JK;
         UWORD	LM;
#else
         UWORD	LM;
         UWORD	JK;
#endif
      }Words;
      ULONG	Long;
   };
}TMATHJKLM;

typedef struct
{
   union
   {
      struct
      {
#ifdef MSB_FIRST
         UBYTE	xx2;
         UBYTE	xx1;
         UBYTE	N;
         UBYTE	P;
#else
         UBYTE	P;
         UBYTE	N;
         UBYTE	xx1;
         UBYTE	xx2;
#endif
      }Bytes;
      struct
      {
#ifdef MSB_FIRST
         UWORD	xx1;
         UWORD	NP;
#else
         UWORD	NP;
         UWORD	xx1;
#endif
      }Words;
      ULONG	Long;
   };
}TMATHNP;


//
// As the Susie sprite engine only ever sees system RAM
// wa can access this directly without the hassle of
// going through the system object, much faster
//
#define RAM_PEEK(m)             (mRamPointer[(m)])
#define RAM_PEEKW(m)            (mRamPointer[(m)]+(mRamPointer[(m)+1]<<8))
#define RAM_POKE(m1,m2)         {mRamPointer[(m1)]=(m2);}

#define MY_GET_BITS(retval_bits, bits) \
     /* ULONG retval_bits; */ \
   if(mLinePacketBitsLeft<=bits) retval_bits = 0; \
   else \
   { \
   if(mLineShiftRegCount<bits) \
   { \
      mLineShiftReg<<=24; \
      mLineShiftReg|=RAM_PEEK(mTMPADR.Word++)<<16; \
      mLineShiftReg|=RAM_PEEK(mTMPADR.Word++)<<8; \
      mLineShiftReg|=RAM_PEEK(mTMPADR.Word++); \
      mLineShiftRegCount+=24; \
      mCycles+=3*SPR_RDWR_CYC; \
   } \
   retval_bits=mLineShiftReg>>(mLineShiftRegCount-bits); \
   retval_bits&=(1<<bits)-1; \
   mLineShiftRegCount-=bits; \
   mLinePacketBitsLeft-=bits; \
   }

class CSusie : public CLynxBase
{
   public:
      CSusie(CSystem& parent);
      ~CSusie();

      void	Reset(void);
      bool	ContextSave(LSS_FILE *fp);
      bool	ContextLoad(LSS_FILE *fp);

      UBYTE	Peek(ULONG addr);
      void	Poke(ULONG addr,UBYTE data);
      ULONG	ReadCycle(void) {return 9;};
      ULONG	WriteCycle(void) {return 5;};
      ULONG	ObjectSize(void) {return SUSIE_SIZE;};

      void	SetButtonData(ULONG data) {mJOYSTICK.Byte=(UBYTE)data;mSWITCHES.Byte=(UBYTE)(data>>8);};
      ULONG	GetButtonData(void) {return mJOYSTICK.Byte+(mSWITCHES.Byte<<8);};

      ULONG	PaintSprites(void);

   private:
      inline ULONG LineInit(ULONG voff) {
         //   TRACE_SUSIE0("LineInit()");
         mLineShiftReg=0;
         mLineShiftRegCount=0;
         mLineRepeatCount=0;
         mLinePixel=0;
         mLineType=line_error;
         mLinePacketBitsLeft=0xffff;

         // Initialise the temporary pointer

         mTMPADR=mSPRDLINE;

         // First read the Offset to the next line
         ULONG offset;
         MY_GET_BITS(offset,8)
         //   TRACE_SUSIE1("LineInit() Offset=%04x",offset);

         // Specify the MAXIMUM number of bits in this packet, it
         // can terminate early but can never use more than this
         // without ending the current packet, we count down in LineGetBits()

         mLinePacketBitsLeft=(offset-1)*8;

         // Literals are a special case and get their count set on a line basis

         if(mSPRCTL1_Literal)
         {
            mLineType=line_abs_literal;
            mLineRepeatCount=((offset-1)*8)/mSPRCTL0_PixelBits;
            // Why is this necessary, is this compensating for the 1,1 offset bug
            //        mLineRepeatCount--;
         }
         //   TRACE_SUSIE1("LineInit() mLineRepeatCount=$%04x",mLineRepeatCount);

         // Set the line base address for use in the calls to pixel painting

         if(voff>101)
         {
            log_printf("CSusie::LineInit() Out of bounds (voff)\n");
            voff=0;
         }

         mLineBaseAddress=mVIDBAS.Word+(voff*(HANDY_SCREEN_WIDTH/2));
         mLineCollisionAddress=mCOLLBAS.Word+(voff*(HANDY_SCREEN_WIDTH/2));
         //   TRACE_SUSIE1("LineInit() mLineBaseAddress=$%04x",mLineBaseAddress);
         //   TRACE_SUSIE1("LineInit() mLineCollisionAddress=$%04x",mLineCollisionAddress);

         // Return the offset to the next line
         return offset;
   };

   inline void WritePixel(ULONG hoff,ULONG pixel) {
      ULONG scr_addr=mLineBaseAddress+(hoff>>1);
      UBYTE dest=RAM_PEEK(scr_addr);

      if(!(hoff&0x01)) {
         // Upper nibble screen write
         dest&=0x0f;
         dest|=pixel<<4;
      } else {
         // Lower nibble screen write
         dest&=0xf0;
         dest|=pixel;
      }
      RAM_POKE(scr_addr,dest);

      // Increment cycle count for the read/modify/write
      mCycles+=2*SPR_RDWR_CYC;
   }

   inline ULONG ReadPixel(ULONG hoff) {
      UBYTE data=RAM_PEEK(mLineBaseAddress+(hoff>>1));

      // Increment cycle count for the read/modify/write
      mCycles+=SPR_RDWR_CYC;

      return (hoff&1) ? (data&0xf) : (data>>4);
   }

   inline void WriteCollision(ULONG hoff,ULONG pixel) {
      ULONG col_addr=mLineCollisionAddress+(hoff>>1);
      UBYTE dest=RAM_PEEK(col_addr);

      if(!(hoff&0x01)) {
         // Upper nibble screen write
         dest&=0x0f;
         dest|=pixel<<4;
      } else {
         // Lower nibble screen write
         dest&=0xf0;
         dest|=pixel;
      }
      RAM_POKE(col_addr,dest);

      // Increment cycle count for the read/modify/write
      mCycles+=2*SPR_RDWR_CYC;
   }

   inline ULONG ReadCollision(ULONG hoff) {
      UBYTE data=RAM_PEEK(mLineCollisionAddress+(hoff>>1));

      // Increment cycle count for the read/modify/write
      mCycles+=SPR_RDWR_CYC;

      return (hoff&1) ? (data&0xf) : (data>>4);
   }

   private:
      CSystem&		mSystem;

      ULONG			mCycles;

      UUWORD		mTMPADR;		// ENG
      UUWORD		mTILTACUM;		// ENG
      UUWORD		mHOFF;			// CPU
      UUWORD		mVOFF;			// CPU
      UUWORD		mVIDBAS;		// CPU
      UUWORD		mCOLLBAS;		// CPU
      UUWORD		mVIDADR;		// ENG
      UUWORD		mCOLLADR;		// ENG
      UUWORD		mSCBNEXT;		// SCB
      UUWORD		mSPRDLINE;		// SCB
      UUWORD		mHPOSSTRT;		// SCB
      UUWORD		mVPOSSTRT;		// SCB
      UUWORD		mSPRHSIZ;		// SCB
      UUWORD		mSPRVSIZ;		// SCB
      UUWORD		mSTRETCH;		// ENG
      UUWORD		mTILT;			// ENG
      UUWORD		mSPRDOFF;		// ENG
      UUWORD		mSPRVPOS;		// ENG
      UUWORD		mCOLLOFF;		// CPU
      UUWORD		mVSIZACUM;		// ENG
      UUWORD		mHSIZACUM;		//    K.s creation
      UUWORD		mHSIZOFF;		// CPU
      UUWORD		mVSIZOFF;		// CPU
      UUWORD		mSCBADR;		// ENG
      UUWORD		mPROCADR;		// ENG

      TMATHABCD	mMATHABCD;		// ENG
      TMATHEFGH	mMATHEFGH;		// ENG
      TMATHJKLM	mMATHJKLM;		// ENG
      TMATHNP		mMATHNP;		// ENG
      SLONG			mMATHAB_sign;
      SLONG			mMATHCD_sign;
      SLONG			mMATHEFGH_sign;

      SLONG			mSPRCTL0_Type;			// SCB
      SLONG			mSPRCTL0_Vflip;
      SLONG			mSPRCTL0_Hflip;
      SLONG			mSPRCTL0_PixelBits;

      SLONG			mSPRCTL1_StartLeft;		// SCB
      SLONG			mSPRCTL1_StartUp;
      SLONG			mSPRCTL1_SkipSprite;
      SLONG			mSPRCTL1_ReloadPalette;
      SLONG			mSPRCTL1_ReloadDepth;
      SLONG			mSPRCTL1_Sizing;
      SLONG	      mSPRCTL1_Literal;

      SLONG			mSPRCOLL_Number;		//CPU
      SLONG			mSPRCOLL_Collide;

      SLONG			mSPRSYS_StopOnCurrent;	//CPU
      SLONG			mSPRSYS_LeftHand;
      SLONG			mSPRSYS_VStretch;
      SLONG			mSPRSYS_NoCollide;
      SLONG			mSPRSYS_Accumulate;
      SLONG			mSPRSYS_SignedMath;
      SLONG			mSPRSYS_Status;
      SLONG			mSPRSYS_UnsafeAccess;
      SLONG			mSPRSYS_LastCarry;
      SLONG			mSPRSYS_Mathbit;
      SLONG			mSPRSYS_MathInProgress;

      ULONG		mSUZYBUSEN;		// CPU

      TSPRINIT	mSPRINIT;		// CPU

      ULONG		mSPRGO;			// CPU
      SLONG		mEVERON;

      UBYTE		mPenIndex[16];	// SCB

      // Line rendering related variables

      ULONG		mLineType;
      ULONG		mLineShiftRegCount;
      ULONG		mLineShiftReg;
      ULONG		mLineRepeatCount;
      ULONG		mLinePixel;
      ULONG		mLinePacketBitsLeft;

      SLONG		mCollision;

      UBYTE		*mRamPointer;

      ULONG		mLineBaseAddress;
      ULONG		mLineCollisionAddress;

      // Joystick switches

      TJOYSTICK	mJOYSTICK;
      TSWITCHES	mSWITCHES;
};

#endif
