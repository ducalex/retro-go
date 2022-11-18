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
// Suzy emulation class                                                     //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class emulates the Suzy chip within the lynx. This provides math    //
// and sprite painting facilities. SpritePaint() is called from within      //
// the Mikey POKE functions when SPRGO is set and is called via the system  //
// object to keep the interface clean.                                      //
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

//#define TRACE_SUSIE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system.h"
#include "susie.h"
#include "lynxdef.h"

CSusie::CSusie(CSystem& parent)
   :mSystem(parent)
{
   TRACE_SUSIE0("CSusie()");
   Reset();
}

CSusie::~CSusie()
{
   TRACE_SUSIE0("~CSusie()");
}

void CSusie::Reset(void)
{
   TRACE_SUSIE0("Reset()");

   // Fetch pointer to system RAM, faster than object access
   // and seeing as Susie only ever sees RAM.

   mRamPointer=mSystem.GetRamPointer();

   // Reset ALL variables

   mTMPADR.Word=0;
   mTILTACUM.Word=0;
   mHOFF.Word=0;
   mVOFF.Word=0;
   mVIDBAS.Word=0;
   mCOLLBAS.Word=0;
   mVIDADR.Word=0;
   mCOLLADR.Word=0;
   mSCBNEXT.Word=0;
   mSPRDLINE.Word=0;
   mHPOSSTRT.Word=0;
   mVPOSSTRT.Word=0;
   mSPRHSIZ.Word=0;
   mSPRVSIZ.Word=0;
   mSTRETCH.Word=0;
   mTILT.Word=0;
   mSPRDOFF.Word=0;
   mSPRVPOS.Word=0;
   mCOLLOFF.Word=0;
   mVSIZACUM.Word=0;
   mHSIZACUM.Word=0;
   mHSIZOFF.Word=0x007f;
   mVSIZOFF.Word=0x007f;
   mSCBADR.Word=0;
   mPROCADR.Word=0;

   // Must be initialised to this due to
   // stun runner math initialisation bug
   // see whatsnew for 0.7
   mMATHABCD.Long=0xffffffff;
   mMATHEFGH.Long=0xffffffff;
   mMATHJKLM.Long=0xffffffff;
   mMATHNP.Long=0xffff;

   mMATHAB_sign=1;
   mMATHCD_sign=1;
   mMATHEFGH_sign=1;

   mSPRCTL0_Type=0;
   mSPRCTL0_Vflip=0;
   mSPRCTL0_Hflip=0;
   mSPRCTL0_PixelBits=0;

   mSPRCTL1_StartLeft=0;
   mSPRCTL1_StartUp=0;
   mSPRCTL1_SkipSprite=0;
   mSPRCTL1_ReloadPalette=0;
   mSPRCTL1_ReloadDepth=0;
   mSPRCTL1_Sizing=0;
   mSPRCTL1_Literal=0;

   mSPRCOLL_Number=0;
   mSPRCOLL_Collide=0;

   mSPRSYS_StopOnCurrent=0;
   mSPRSYS_LeftHand=0;
   mSPRSYS_VStretch=0;
   mSPRSYS_NoCollide=0;
   mSPRSYS_Accumulate=0;
   mSPRSYS_SignedMath=0;
   mSPRSYS_Status=0;
   mSPRSYS_UnsafeAccess=0;
   mSPRSYS_LastCarry=0;
   mSPRSYS_Mathbit=0;
   mSPRSYS_MathInProgress=0;

   mSUZYBUSEN=FALSE;

   mSPRINIT.Byte=0;

   mSPRGO=FALSE;
   mEVERON=FALSE;

   for(int loop=0;loop<16;loop++) mPenIndex[loop]=loop;

   mJOYSTICK.Byte=0;
   mSWITCHES.Byte=0;
}

bool CSusie::ContextSave(LSS_FILE *fp)
{
   TRACE_SUSIE0("ContextSave()");

   if(!lss_printf(fp,"CSusie::ContextSave")) return 0;

   if(!lss_write(&mTMPADR,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_write(&mTILTACUM,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_write(&mHOFF,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_write(&mVOFF,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_write(&mVIDBAS,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_write(&mCOLLBAS,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_write(&mVIDADR,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_write(&mCOLLADR,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_write(&mSCBNEXT,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_write(&mSPRDLINE,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_write(&mHPOSSTRT,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_write(&mVPOSSTRT,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_write(&mSPRHSIZ,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_write(&mSPRVSIZ,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_write(&mSTRETCH,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_write(&mTILT,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_write(&mSPRDOFF,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_write(&mSPRVPOS,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_write(&mCOLLOFF,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_write(&mVSIZACUM,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_write(&mHSIZACUM,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_write(&mHSIZOFF,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_write(&mVSIZOFF,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_write(&mSCBADR,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_write(&mPROCADR,sizeof(UUWORD),1,fp)) return 0;

   if(!lss_write(&mMATHABCD,sizeof(TMATHABCD),1,fp)) return 0;
   if(!lss_write(&mMATHEFGH,sizeof(TMATHEFGH),1,fp)) return 0;
   if(!lss_write(&mMATHJKLM,sizeof(TMATHJKLM),1,fp)) return 0;
   if(!lss_write(&mMATHNP,sizeof(TMATHNP),1,fp)) return 0;

   if(!lss_write(&mSPRCTL0_Type,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mSPRCTL0_Vflip,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mSPRCTL0_Hflip,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mSPRCTL0_PixelBits,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mSPRCTL1_StartLeft,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mSPRCTL1_StartUp,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mSPRCTL1_SkipSprite,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mSPRCTL1_ReloadPalette,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mSPRCTL1_ReloadDepth,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mSPRCTL1_Sizing,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mSPRCTL1_Literal,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mSPRCOLL_Number,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mSPRCOLL_Collide,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mSPRSYS_StopOnCurrent,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mSPRSYS_LeftHand,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mSPRSYS_VStretch,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mSPRSYS_NoCollide,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mSPRSYS_Accumulate,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mSPRSYS_SignedMath,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mSPRSYS_Status,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mSPRSYS_UnsafeAccess,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mSPRSYS_LastCarry,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mSPRSYS_Mathbit,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mSPRSYS_MathInProgress,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mSUZYBUSEN,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mSPRINIT,sizeof(TSPRINIT),1,fp)) return 0;

   if(!lss_write(&mSPRGO,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mEVERON,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(mPenIndex,sizeof(UBYTE),16,fp)) return 0;

   if(!lss_write(&mLineType,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mLineShiftRegCount,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mLineShiftReg,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mLineRepeatCount,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mLinePixel,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mLinePacketBitsLeft,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mCollision,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mLineBaseAddress,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mLineCollisionAddress,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mJOYSTICK,sizeof(TJOYSTICK),1,fp)) return 0;
   if(!lss_write(&mSWITCHES,sizeof(TSWITCHES),1,fp)) return 0;

   return 1;
}

bool CSusie::ContextLoad(LSS_FILE *fp)
{
   TRACE_SUSIE0("ContextLoad()");

   char teststr[32]="XXXXXXXXXXXXXXXXXXX";
   if(!lss_read(teststr,sizeof(char),19,fp)) return 0;
   if(strcmp(teststr,"CSusie::ContextSave")!=0) return 0;

   if(!lss_read(&mTMPADR,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_read(&mTILTACUM,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_read(&mHOFF,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_read(&mVOFF,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_read(&mVIDBAS,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_read(&mCOLLBAS,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_read(&mVIDADR,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_read(&mCOLLADR,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_read(&mSCBNEXT,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_read(&mSPRDLINE,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_read(&mHPOSSTRT,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_read(&mVPOSSTRT,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_read(&mSPRHSIZ,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_read(&mSPRVSIZ,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_read(&mSTRETCH,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_read(&mTILT,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_read(&mSPRDOFF,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_read(&mSPRVPOS,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_read(&mCOLLOFF,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_read(&mVSIZACUM,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_read(&mHSIZACUM,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_read(&mHSIZOFF,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_read(&mVSIZOFF,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_read(&mSCBADR,sizeof(UUWORD),1,fp)) return 0;
   if(!lss_read(&mPROCADR,sizeof(UUWORD),1,fp)) return 0;

   if(!lss_read(&mMATHABCD,sizeof(TMATHABCD),1,fp)) return 0;
   if(!lss_read(&mMATHEFGH,sizeof(TMATHEFGH),1,fp)) return 0;
   if(!lss_read(&mMATHJKLM,sizeof(TMATHJKLM),1,fp)) return 0;
   if(!lss_read(&mMATHNP,sizeof(TMATHNP),1,fp)) return 0;

   if(!lss_read(&mSPRCTL0_Type,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mSPRCTL0_Vflip,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mSPRCTL0_Hflip,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mSPRCTL0_PixelBits,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mSPRCTL1_StartLeft,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mSPRCTL1_StartUp,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mSPRCTL1_SkipSprite,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mSPRCTL1_ReloadPalette,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mSPRCTL1_ReloadDepth,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mSPRCTL1_Sizing,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mSPRCTL1_Literal,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mSPRCOLL_Number,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mSPRCOLL_Collide,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mSPRSYS_StopOnCurrent,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mSPRSYS_LeftHand,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mSPRSYS_VStretch,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mSPRSYS_NoCollide,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mSPRSYS_Accumulate,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mSPRSYS_SignedMath,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mSPRSYS_Status,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mSPRSYS_UnsafeAccess,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mSPRSYS_LastCarry,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mSPRSYS_Mathbit,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mSPRSYS_MathInProgress,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mSUZYBUSEN,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mSPRINIT,sizeof(TSPRINIT),1,fp)) return 0;

   if(!lss_read(&mSPRGO,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mEVERON,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(mPenIndex,sizeof(UBYTE),16,fp)) return 0;

   if(!lss_read(&mLineType,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mLineShiftRegCount,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mLineShiftReg,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mLineRepeatCount,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mLinePixel,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mLinePacketBitsLeft,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mCollision,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mLineBaseAddress,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mLineCollisionAddress,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mJOYSTICK,sizeof(TJOYSTICK),1,fp)) return 0;
   if(!lss_read(&mSWITCHES,sizeof(TSWITCHES),1,fp)) return 0;

   return 1;
}

ULONG CSusie::PaintSprites(void)
{
   int	sprcount=0;
   int data=0;
   int everonscreen=0;

   TRACE_SUSIE0("                                                              ");
   TRACE_SUSIE0("                                                              ");
   TRACE_SUSIE0("                                                              ");
   TRACE_SUSIE0("**************************************************************");
   TRACE_SUSIE0("********************** PaintSprites **************************");
   TRACE_SUSIE0("**************************************************************");
   TRACE_SUSIE0("                                                              ");

   TRACE_SUSIE1("PaintSprites() VIDBAS  $%04x",mVIDBAS.Word);
   TRACE_SUSIE1("PaintSprites() COLLBAS $%04x",mCOLLBAS.Word);
   TRACE_SUSIE1("PaintSprites() SPRSYS  $%02x",Peek(SPRSYS));

   if(!mSUZYBUSEN || !mSPRGO) {
      TRACE_SUSIE0("PaintSprites() Returned !mSUZYBUSEN || !mSPRGO");
      return 0;
   }

   //ULONG       mPenIndex[16];
   mCycles=0;

   while (1)
   {
      TRACE_SUSIE1("PaintSprites() ************ Rendering Sprite %03d ************",sprcount);

      everonscreen=0;// everon has to be reset for every sprite, thus line was moved inside this loop

      // Step 1 load up the SCB params into Susie

      // And thus it is documented that only the top byte of SCBNEXT is used.
      // Its mentioned under the bits that are broke section in the bluebook
      if(!(mSCBNEXT.Word&0xff00)) {
         TRACE_SUSIE0("PaintSprites() mSCBNEXT==0 - FINISHED");
         mSPRSYS_Status=0;	// Engine has finished
         mSPRGO=FALSE;
         break;
      } else {
         mSPRSYS_Status=1;
      }

      mTMPADR.Word=mSCBNEXT.Word;	// Copy SCB pointer
      mSCBADR.Word=mSCBNEXT.Word;	// Copy SCB pointer
      TRACE_SUSIE1("PaintSprites() SCBADDR $%04x",mSCBADR.Word);

      data=RAM_PEEK(mTMPADR.Word);			// Fetch control 0
      TRACE_SUSIE1("PaintSprites() SPRCTL0 $%02x",data);
      mSPRCTL0_Type=data&0x0007;
      mSPRCTL0_Vflip=data&0x0010;
      mSPRCTL0_Hflip=data&0x0020;
      mSPRCTL0_PixelBits=((data&0x00c0)>>6)+1;
      mTMPADR.Word+=1;

      data=RAM_PEEK(mTMPADR.Word);			// Fetch control 1
      TRACE_SUSIE1("PaintSprites() SPRCTL1 $%02x",data);
      mSPRCTL1_StartLeft=data&0x0001;
      mSPRCTL1_StartUp=data&0x0002;
      mSPRCTL1_SkipSprite=data&0x0004;
      mSPRCTL1_ReloadPalette=data&0x0008;
      mSPRCTL1_ReloadDepth=(data&0x0030)>>4;
      mSPRCTL1_Sizing=data&0x0040;
      mSPRCTL1_Literal=data&0x0080;
      mTMPADR.Word+=1;

      data=RAM_PEEK(mTMPADR.Word);			// Collision num
      TRACE_SUSIE1("PaintSprites() SPRCOLL $%02x",data);
      mSPRCOLL_Number=data&0x000f;
      mSPRCOLL_Collide=data&0x0020;
      mTMPADR.Word+=1;

      mSCBNEXT.Word=RAM_PEEKW(mTMPADR.Word);	// Next SCB
      TRACE_SUSIE1("PaintSprites() SCBNEXT $%04x",mSCBNEXT.Word);
      mTMPADR.Word+=2;

      mCycles+=5*SPR_RDWR_CYC;

      // Initialise the collision depositary

      // Although Tom Schenck says this is correct, it doesnt appear to be
      //		if(!mSPRCOLL_Collide && !mSPRSYS_NoCollide)
      //		{
      //			mCollision=RAM_PEEK((mSCBADR.Word+mCOLLOFF.Word)&0xffff);
      //			mCollision&=0x0f;
      //		}
      mCollision=0;

      // Check if this is a skip sprite

      if(!mSPRCTL1_SkipSprite) {
         mSPRDLINE.Word=RAM_PEEKW(mTMPADR.Word);	// Sprite pack data
         TRACE_SUSIE1("PaintSprites() SPRDLINE $%04x",mSPRDLINE.Word);
         mTMPADR.Word+=2;

         mHPOSSTRT.Word=RAM_PEEKW(mTMPADR.Word);	// Sprite horizontal start position
         TRACE_SUSIE1("PaintSprites() HPOSSTRT $%04x",mHPOSSTRT.Word);
         mTMPADR.Word+=2;

         mVPOSSTRT.Word=RAM_PEEKW(mTMPADR.Word);	// Sprite vertical start position
         TRACE_SUSIE1("PaintSprites() VPOSSTRT $%04x",mVPOSSTRT.Word);
         mTMPADR.Word+=2;

         mCycles+=6*SPR_RDWR_CYC;

         // bool enable_sizing  = FALSE;
         bool enable_stretch = FALSE;
         bool enable_tilt    = FALSE;

         // Optional section defined by reload type in Control 1

         TRACE_SUSIE1("PaintSprites() mSPRCTL1.Bits.ReloadDepth=%d",mSPRCTL1_ReloadDepth);
         switch(mSPRCTL1_ReloadDepth) {
            case 1:
               TRACE_SUSIE0("PaintSprites() Sizing Enabled");
               // enable_sizing=TRUE;

               mSPRHSIZ.Word=RAM_PEEKW(mTMPADR.Word);	// Sprite Horizontal size
               mTMPADR.Word+=2;

               mSPRVSIZ.Word=RAM_PEEKW(mTMPADR.Word);	// Sprite Verticalal size
               mTMPADR.Word+=2;

               mCycles+=4*SPR_RDWR_CYC;
               break;

            case 2:
               TRACE_SUSIE0("PaintSprites() Sizing Enabled");
               TRACE_SUSIE0("PaintSprites() Stretch Enabled");
               // enable_sizing=TRUE;
               enable_stretch=TRUE;

               mSPRHSIZ.Word=RAM_PEEKW(mTMPADR.Word);	// Sprite Horizontal size
               mTMPADR.Word+=2;

               mSPRVSIZ.Word=RAM_PEEKW(mTMPADR.Word);	// Sprite Verticalal size
               mTMPADR.Word+=2;

               mSTRETCH.Word=RAM_PEEKW(mTMPADR.Word);	// Sprite stretch
               mTMPADR.Word+=2;

               mCycles+=6*SPR_RDWR_CYC;
               break;

            case 3:
               TRACE_SUSIE0("PaintSprites() Sizing Enabled");
               TRACE_SUSIE0("PaintSprites() Stretch Enabled");
               TRACE_SUSIE0("PaintSprites() Tilt Enabled");
               // enable_sizing=TRUE;
               enable_stretch=TRUE;
               enable_tilt=TRUE;

               mSPRHSIZ.Word=RAM_PEEKW(mTMPADR.Word);	// Sprite Horizontal size
               mTMPADR.Word+=2;

               mSPRVSIZ.Word=RAM_PEEKW(mTMPADR.Word);	// Sprite Verticalal size
               mTMPADR.Word+=2;

               mSTRETCH.Word=RAM_PEEKW(mTMPADR.Word);	// Sprite stretch
               mTMPADR.Word+=2;

               mTILT.Word=RAM_PEEKW(mTMPADR.Word);		// Sprite tilt
               mTMPADR.Word+=2;

               mCycles+=8*SPR_RDWR_CYC;
               break;

            default:
               break;
         }

         TRACE_SUSIE1("PaintSprites() SPRHSIZ $%04x",mSPRHSIZ.Word);
         TRACE_SUSIE1("PaintSprites() SPRVSIZ $%04x",mSPRVSIZ.Word);
         TRACE_SUSIE1("PaintSprites() STRETCH $%04x",mSTRETCH.Word);
         TRACE_SUSIE1("PaintSprites() TILT    $%04x",mTILT.Word);


         // Optional Palette reload

         if(!mSPRCTL1_ReloadPalette) {
            TRACE_SUSIE0("PaintSprites() Palette reloaded");
            for(int loop=0;loop<8;loop++) {
               UBYTE data_tmp=RAM_PEEK(mTMPADR.Word++);
               mPenIndex[loop*2]=(data_tmp>>4)&0x0f;
               mPenIndex[(loop*2)+1]=data_tmp&0x0f;
            }
            // Increment cycle count for the reads
            mCycles+=8*SPR_RDWR_CYC;
         }

         // Now we can start painting

         // Quadrant drawing order is: SE,NE,NW,SW
         // start quadrant is given by sprite_control1:0 & 1

         // Setup screen start end variables

         int screen_h_start=(SWORD)mHOFF.Word;
         int screen_h_end=(SWORD)mHOFF.Word+HANDY_SCREEN_WIDTH;
         int screen_v_start=(SWORD)mVOFF.Word;
         int screen_v_end=(SWORD)mVOFF.Word+HANDY_SCREEN_HEIGHT;

         int world_h_mid=screen_h_start+0x8000+(HANDY_SCREEN_WIDTH/2);
         int world_v_mid=screen_v_start+0x8000+(HANDY_SCREEN_HEIGHT/2);

         TRACE_SUSIE2("PaintSprites() screen_h_start $%04x screen_h_end $%04x",screen_h_start,screen_h_end);
         TRACE_SUSIE2("PaintSprites() screen_v_start $%04x screen_v_end $%04x",screen_v_start,screen_v_end);
         TRACE_SUSIE2("PaintSprites() world_h_mid    $%04x world_v_mid  $%04x",world_h_mid,world_v_mid);

         bool superclip=FALSE;
         int quadrant=0;
         int hsign,vsign;

         if(mSPRCTL1_StartLeft) {
            if(mSPRCTL1_StartUp) quadrant=2; else quadrant=3;
         } else {
            if(mSPRCTL1_StartUp) quadrant=1; else quadrant=0;
         }
         TRACE_SUSIE1("PaintSprites() Quadrant=%d",quadrant);

         // Check ref is inside screen area

         if((SWORD)mHPOSSTRT.Word<screen_h_start || (SWORD)mHPOSSTRT.Word>=screen_h_end ||
            (SWORD)mVPOSSTRT.Word<screen_v_start || (SWORD)mVPOSSTRT.Word>=screen_v_end) superclip=TRUE;

         TRACE_SUSIE1("PaintSprites() Superclip=%d",superclip);


         // Quadrant mapping is:	SE	NE	NW	SW
         //						0	1	2	3
         // hsign				+1	+1	-1	-1
         // vsign				+1	-1	-1	+1
         //
         //
         //		2 | 1
         //     -------
         //      3 | 0
         //

         // Loop for 4 quadrants

         for(int loop=0;loop<4;loop++) {
            TRACE_SUSIE1("PaintSprites() -------- Rendering Quadrant %03d --------",quadrant);

            int sprite_v=mVPOSSTRT.Word;
            int sprite_h=mHPOSSTRT.Word;

            bool render=FALSE;

            // Set quadrand multipliers
            hsign=(quadrant==0 || quadrant==1)?1:-1;
            vsign=(quadrant==0 || quadrant==3)?1:-1;

            // Preflip		TRACE_SUSIE2("PaintSprites() hsign=%d vsign=%d",hsign,vsign);

            //Use h/v flip to invert v/hsign

            if(mSPRCTL0_Vflip) vsign=-vsign;
            if(mSPRCTL0_Hflip) hsign=-hsign;

            TRACE_SUSIE2("PaintSprites() Hflip=%d Vflip=%d",mSPRCTL0_Hflip,mSPRCTL0_Vflip);
            TRACE_SUSIE2("PaintSprites() Hsign=%d   Vsign=%d",hsign,vsign);
            TRACE_SUSIE2("PaintSprites() Hpos =%04x Vpos =%04x",mHPOSSTRT.Word,mVPOSSTRT.Word);
            TRACE_SUSIE2("PaintSprites() Hsizoff =%04x Vsizoff =%04x",mHSIZOFF.Word,mVSIZOFF.Word);

            // Two different rendering algorithms used, on-screen & superclip
            // when on screen we draw in x until off screen then skip to next
            // line, BUT on superclip we draw all the way to the end of any
            // given line checking each pixel is on screen.

            if(superclip) {
               // Check on the basis of each quad, we only render the quad
               // IF the screen is in the quad, relative to the centre of
               // the screen which is calculated below.

               // Quadrant mapping is:	SE	NE	NW	SW
               //						0	1	2	3
               // hsign				+1	+1	-1	-1
               // vsign				+1	-1	-1	+1
               //
               //
               //		2 | 1
               //     -------
               //      3 | 0
               //
               // Quadrant mapping for superclipping must also take into account
               // the hflip, vflip bits & negative tilt to be able to work correctly
               //
               int	modquad=quadrant;
               static int vquadflip[4]={1,0,3,2};
               static int hquadflip[4]={3,2,1,0};

               if(mSPRCTL0_Vflip) modquad=vquadflip[modquad];
               if(mSPRCTL0_Hflip) modquad=hquadflip[modquad];

               // This is causing Eurosoccer to fail!!
               //					if(enable_tilt && mTILT.Word&0x8000) modquad=hquadflip[modquad];

               switch(modquad) {
                  case 3:
                     if((sprite_h>=screen_h_start || sprite_h<world_h_mid) && (sprite_v<screen_v_end   || sprite_v>world_v_mid)) render=TRUE;
                     break;
                  case 2:
                     if((sprite_h>=screen_h_start || sprite_h<world_h_mid) && (sprite_v>=screen_v_start || sprite_v<world_v_mid)) render=TRUE;
                     break;
                  case 1:
                     if((sprite_h<screen_h_end   || sprite_h>world_h_mid) && (sprite_v>=screen_v_start || sprite_v<world_v_mid)) render=TRUE;
                     break;
                  default:
                     if((sprite_h<screen_h_end   || sprite_h>world_h_mid) && (sprite_v<screen_v_end   || sprite_v>world_v_mid)) render=TRUE;
                     break;
               }
            } else {
               render=TRUE;
            }

            // Is this quad to be rendered ??

            TRACE_SUSIE1("PaintSprites() Render status %d",render);

            int pixel_height=0;
            int pixel_width=0;
            // static int pixel=0;
            int hoff=0,voff=0;
            // int hloop=0;
            int vloop=0;
            bool onscreen=0;
            static int vquadoff=0;
            static int hquadoff=0;

            if(render) { //  && gRenderFrame
               // Set the vertical position & offset
               voff=(SWORD)mVPOSSTRT.Word-screen_v_start;

               // Zero the stretch,tilt & acum values
               mTILTACUM.Word=0;

               // Perform the SIZOFF
               if(vsign==1) mVSIZACUM.Word=mVSIZOFF.Word;
               else mVSIZACUM.Word=0;

               // Take the sign of the first quad (0) as the basic
               // sign, all other quads drawing in the other direction
               // get offset by 1 pixel in the other direction, this
               // fixes the squashed look on the multi-quad sprites.
               //					if(vsign==-1 && loop>0) voff+=vsign;
               if(loop==0)	vquadoff=vsign;
               if(vsign!=vquadoff) voff+=vsign;

               for(;;) {
                  // Vertical scaling is done here
                  mVSIZACUM.Word+=mSPRVSIZ.Word;
                  pixel_height=mVSIZACUM.Byte.High;
                  mVSIZACUM.Byte.High=0;

                  // Update the next data line pointer and initialise our line
                  mSPRDOFF.Word=(UWORD)LineInit(0);

                  // If 1 == next quad, ==0 end of sprite, anyways its END OF LINE
                  if(mSPRDOFF.Word==1) {		// End of quad
                     mSPRDLINE.Word+=mSPRDOFF.Word;
                     break;
                  }

                  if(mSPRDOFF.Word==0) {		// End of sprite
                     loop=4;		// Halt the quad loop
                     break;
                  }

                  // Draw one horizontal line of the sprite
                  for(vloop=0;vloop<pixel_height;vloop++) {
                     // Early bailout if the sprite has moved off screen, terminate quad
                     if(vsign==1 && voff>=HANDY_SCREEN_HEIGHT)	break;
                     if(vsign==-1 && voff<0)	break;

                     // Only allow the draw to take place if the line is visible
                     if(voff>=0 && voff<HANDY_SCREEN_HEIGHT) {
                        // Work out the horizontal pixel start position, start + tilt
                        mHPOSSTRT.Word+=((SWORD)mTILTACUM.Word>>8);
                        mTILTACUM.Byte.High=0;
                        hoff=(int)((SWORD)mHPOSSTRT.Word)-screen_h_start;

                        // Zero/Force the horizontal scaling accumulator
                        if(hsign==1) mHSIZACUM.Word=mHSIZOFF.Word;
                        else mHSIZACUM.Word=0;

                        // Take the sign of the first quad (0) as the basic
                        // sign, all other quads drawing in the other direction
                        // get offset by 1 pixel in the other direction, this
                        // fixes the squashed look on the multi-quad sprites.
                        //								if(hsign==-1 && loop>0) hoff+=hsign;
                        if(loop==0)	hquadoff=hsign;
                        if(hsign!=hquadoff) hoff+=hsign;

                        // Initialise our line
                        LineInit(voff);
                        onscreen=FALSE;

                        ULONG pixel = mLinePixel; // Much faster
                        switch(mSPRCTL0_Type)
                        {
                              case sprite_background_shadow:
                                 #undef PROCESS_PIXEL
                                 #define PROCESS_PIXEL \
                                 WritePixel(hoff,pixel); \
                                 if(!mSPRCOLL_Collide && !mSPRSYS_NoCollide && pixel!=0x0e) \
                                 { \
                                    WriteCollision(hoff,mSPRCOLL_Number); \
                                 }

                                 #undef EndWhile
                                 #undef LoopContinue
                                 #define EndWhile EndWhile01b
                                 #define LoopContinue LoopContinue01b
                                 #include "susie_pixel_loop.h"
                                 break;
                              case sprite_background_noncollide:
                                 #undef PROCESS_PIXEL
                                 #define PROCESS_PIXEL WritePixel(hoff,pixel);

                                 #undef EndWhile
                                 #undef LoopContinue
                                 #define EndWhile EndWhile02b
                                 #define LoopContinue LoopContinue02b
                                 #include "susie_pixel_loop.h"
                                 break;
                              case sprite_noncollide:
                                 #undef PROCESS_PIXEL
                                 #define PROCESS_PIXEL if(pixel!=0x00) WritePixel(hoff,pixel);

                                 #undef EndWhile
                                 #undef LoopContinue
                                 #define EndWhile EndWhile03b
                                 #define LoopContinue LoopContinue03b
                                 #include "susie_pixel_loop.h"
                                 break;
                              case sprite_boundary:
                                 #undef PROCESS_PIXEL
                                 #define PROCESS_PIXEL \
                                 if(pixel!=0x00 && pixel!=0x0f) \
                                 { \
                                    WritePixel(hoff,pixel); \
                                 } \
                                 if(pixel!=0x00) \
                                 { \
                                    if(!mSPRCOLL_Collide && !mSPRSYS_NoCollide) \
                                    { \
                                       ULONG collision=ReadCollision(hoff); \
                                       if(collision>mCollision) \
                                       { \
                                          mCollision=collision; \
                                       } \
                                       { \
                                          WriteCollision(hoff,mSPRCOLL_Number); \
                                       } \
                                    } \
                                 }

                                 #undef EndWhile
                                 #undef LoopContinue
                                 #define EndWhile EndWhile04b
                                 #define LoopContinue LoopContinue04b
                                 #include "susie_pixel_loop.h"
                                 break;
                              case sprite_normal:
                                 #undef PROCESS_PIXEL
                                 #define PROCESS_PIXEL \
                                 if(pixel!=0x00) \
                                 { \
                                    WritePixel(hoff,pixel); \
                                    if(!mSPRCOLL_Collide && !mSPRSYS_NoCollide) \
                                    { \
                                       ULONG collision=ReadCollision(hoff); \
                                       if(collision>mCollision) \
                                       { \
                                          mCollision=collision; \
                                       } \
                                       { \
                                          WriteCollision(hoff,mSPRCOLL_Number); \
                                       } \
                                    } \
                                 }

                                 #undef EndWhile
                                 #undef LoopContinue
                                 #define EndWhile EndWhile05b
                                 #define LoopContinue LoopContinue05b
                                 #include "susie_pixel_loop.h"
                                 break;
                              case sprite_boundary_shadow:
                                 #undef PROCESS_PIXEL
                                 #define PROCESS_PIXEL \
                                 if(pixel!=0x00 && pixel!=0x0e && pixel!=0x0f) \
                                 { \
                                    WritePixel(hoff,pixel); \
                                 } \
                                 if(pixel!=0x00 && pixel!=0x0e) \
                                 { \
                                    if(!mSPRCOLL_Collide && !mSPRSYS_NoCollide) \
                                    { \
                                       ULONG collision=ReadCollision(hoff); \
                                       if(collision>mCollision) \
                                       { \
                                          mCollision=collision; \
                                       } \
                                       { \
                                          WriteCollision(hoff,mSPRCOLL_Number); \
                                       } \
                                    } \
                                 }

                                 #undef EndWhile
                                 #undef LoopContinue
                                 #define EndWhile EndWhile06b
                                 #define LoopContinue LoopContinue06b
                                 #include "susie_pixel_loop.h"
                                 break;
                              case sprite_shadow:
                                 #undef PROCESS_PIXEL
                                 #define PROCESS_PIXEL \
                                 if(pixel!=0x00) \
                                 { \
                                    WritePixel(hoff,pixel); \
                                 } \
                                 if(pixel!=0x00 && pixel!=0x0e) \
                                 { \
                                    if(!mSPRCOLL_Collide && !mSPRSYS_NoCollide) \
                                    { \
                                       ULONG collision=ReadCollision(hoff); \
                                       if(collision>mCollision) \
                                       { \
                                          mCollision=collision; \
                                       } \
                                       { \
                                          WriteCollision(hoff,mSPRCOLL_Number); \
                                       } \
                                    } \
                                 }

                                 #undef EndWhile
                                 #undef LoopContinue
                                 #define EndWhile EndWhile07b
                                 #define LoopContinue LoopContinue07b
                                 #include "susie_pixel_loop.h"
                                 break;
                              case sprite_xor_shadow:
                                 #undef PROCESS_PIXEL
                                 #define PROCESS_PIXEL \
                                 if(pixel!=0x00) \
                                 { \
                                    WritePixel(hoff,ReadPixel(hoff)^pixel); \
                                 } \
                                 if(pixel!=0x00 && pixel!=0x0e) \
                                 { \
                                    if(!mSPRCOLL_Collide && !mSPRSYS_NoCollide && pixel!=0x0e) \
                                    { \
                                       ULONG collision=ReadCollision(hoff); \
                                       if(collision>mCollision) \
                                       { \
                                          mCollision=collision; \
                                       } \
                                       { \
                                          WriteCollision(hoff,mSPRCOLL_Number); \
                                       } \
                                    } \
                                 }

                                 #undef EndWhile
                                 #undef LoopContinue
                                 #define EndWhile EndWhile08b
                                 #define LoopContinue LoopContinue08b
                                 #include "susie_pixel_loop.h"
                                 break;
                              default:
                                 #undef PROCESS_PIXEL
                                 #define PROCESS_PIXEL

                                 #undef EndWhile
                                 #undef LoopContinue
                                 #define EndWhile EndWhile09b
                                 #define LoopContinue LoopContinue09b
                                 #include "susie_pixel_loop.h"
                                 break;
                        }
                     }
                     voff+=vsign;

                     // For every destination line we can modify SPRHSIZ & SPRVSIZ & TILTACUM
                     if(enable_stretch) {
                        mSPRHSIZ.Word+=mSTRETCH.Word;
                        //								if(mSPRSYS_VStretch) mSPRVSIZ.Word+=mSTRETCH.Word;
                     }
                     if(enable_tilt) {
                        // Manipulate the tilt stuff
                        mTILTACUM.Word+=mTILT.Word;
                     }
                  }
                  // According to the docs this increments per dest line
                  // but only gets set when the source line is read
                  if(mSPRSYS_VStretch) mSPRVSIZ.Word+=mSTRETCH.Word*pixel_height;

                  // Update the line start for our next run thru the loop
                  mSPRDLINE.Word+=mSPRDOFF.Word;
               }
            } else {
               // Skip thru data to next quad
               for(;;) {
                  // Read the start of line offset

                  mSPRDOFF.Word=(UWORD)LineInit(0);

                  // We dont want to process data so mSPRDLINE is useless to us
                  mSPRDLINE.Word+=mSPRDOFF.Word;

                  // If 1 == next quad, ==0 end of sprite, anyways its END OF LINE

                  if(mSPRDOFF.Word==1) break;	// End of quad
                  if(mSPRDOFF.Word==0) {		// End of sprite
                     loop=4;		// Halt the quad loop
                     break;
                  }

               }
            }

            // Increment quadrant and mask to 2 bit value (0-3)
            quadrant++;
            quadrant&=0x03;
         }

         // Write the collision depositary if required

         if(!mSPRCOLL_Collide && !mSPRSYS_NoCollide) {
            switch(mSPRCTL0_Type) {
               case sprite_xor_shadow:
               case sprite_boundary:
               case sprite_normal:
               case sprite_boundary_shadow:
               case sprite_shadow: {
                     UWORD coldep=mSCBADR.Word+mCOLLOFF.Word;
                     RAM_POKE(coldep,(UBYTE)mCollision);
                     TRACE_SUSIE2("PaintSprites() COLLOFF=$%04x SCBADR=$%04x",mCOLLOFF.Word,mSCBADR.Word);
                     TRACE_SUSIE2("PaintSprites() Wrote $%02x to SCB collision depositary at $%04x",(UBYTE)mCollision,coldep);
                  }
                  break;
               default:
                  break;
            }
         }

         if(mEVERON) {
            UWORD coldep=mSCBADR.Word+mCOLLOFF.Word;
            UBYTE coldat=RAM_PEEK(coldep);
            if(!everonscreen) coldat|=0x80;
            else coldat&=0x7f;
            RAM_POKE(coldep,coldat);
            TRACE_SUSIE0("PaintSprites() EVERON IS ACTIVE");
            TRACE_SUSIE2("PaintSprites() Wrote $%02x to SCB collision depositary at $%04x",coldat,coldep);
         }

         // Perform Sprite debugging if required, single step on sprite draw
         if(gSingleStepModeSprites) {
            // char message[256];
            //log_printf("CSusie:PaintSprites() - Rendered Sprite %03d\n",sprcount);
            //SingleStepModeSprites=0;
         }
      } else {
         TRACE_SUSIE0("PaintSprites() mSPRCTL1.Bits.SkipSprite==TRUE");
      }

      // Increase sprite number
      sprcount++;

      // Check if we abort after 1st sprite is complete

      //		if(mSPRSYS.Read.StopOnCurrent)
      //		{
      //			mSPRSYS.Read.Status=0;	// Engine has finished
      //			mSPRGO=FALSE;
      //			break;
      //		}

      // Check sprcount for looping SCB, random large number chosen
      if(sprcount>4096) {
         // Stop the system, otherwise we may just come straight back in.....
         gSystemHalt=TRUE;
         // Display warning message
         log_printf("CSusie:PaintSprites(): Single draw sprite limit exceeded (>4096). The SCB is most likely looped back on itself. Reset/Exit is recommended\n");
         // Signal error to the caller
         return 0;
      }
   }
   // Fudge factor to fix many flickering issues, also the keypress
   // problem with Hard Drivin and the strange pause in Dirty Larry.
   //   mCycles>>=2;
   return mCycles;
}

void CSusie::Poke(ULONG addr,UBYTE data)
{
   switch(addr&0xff) {
      case (TMPADRL&0xff):
         mTMPADR.Byte.Low=data;
         mTMPADR.Byte.High=0;
         TRACE_SUSIE2("Poke(TMPADRL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TMPADRH&0xff):
         mTMPADR.Byte.High=data;
         TRACE_SUSIE2("Poke(TMPADRH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TILTACUML&0xff):
         mTILTACUM.Byte.Low=data;
         mTILTACUM.Byte.High=0;
         TRACE_SUSIE2("Poke(TILTACUML,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TILTACUMH&0xff):
         mTILTACUM.Byte.High=data;
         TRACE_SUSIE2("Poke(TILTACUMH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (HOFFL&0xff):
         mHOFF.Byte.Low=data;
         mHOFF.Byte.High=0;
         TRACE_SUSIE2("Poke(HOFFL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (HOFFH&0xff):
         mHOFF.Byte.High=data;
         TRACE_SUSIE2("Poke(HOFFH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (VOFFL&0xff):
         mVOFF.Byte.Low=data;
         mVOFF.Byte.High=0;
         TRACE_SUSIE2("Poke(VOFFL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (VOFFH&0xff):
         mVOFF.Byte.High=data;
         TRACE_SUSIE2("Poke(VOFFH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (VIDBASL&0xff):
         mVIDBAS.Byte.Low=data;
         mVIDBAS.Byte.High=0;
         TRACE_SUSIE2("Poke(VIDBASL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (VIDBASH&0xff):
         mVIDBAS.Byte.High=data;
         TRACE_SUSIE2("Poke(VIDBASH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (COLLBASL&0xff):
         mCOLLBAS.Byte.Low=data;
         mCOLLBAS.Byte.High=0;
         TRACE_SUSIE2("Poke(COLLBASL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (COLLBASH&0xff):
         mCOLLBAS.Byte.High=data;
         TRACE_SUSIE2("Poke(COLLBASH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (VIDADRL&0xff):
         mVIDADR.Byte.Low=data;
         mVIDADR.Byte.High=0;
         TRACE_SUSIE2("Poke(VIDADRL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (VIDADRH&0xff):
         mVIDADR.Byte.High=data;
         TRACE_SUSIE2("Poke(VIDADRH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (COLLADRL&0xff):
         mCOLLADR.Byte.Low=data;
         mCOLLADR.Byte.High=0;
         TRACE_SUSIE2("Poke(COLLADRL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (COLLADRH&0xff):
         mCOLLADR.Byte.High=data;
         TRACE_SUSIE2("Poke(COLLADRH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (SCBNEXTL&0xff):
         mSCBNEXT.Byte.Low=data;
         mSCBNEXT.Byte.High=0;
         TRACE_SUSIE2("Poke(SCBNEXTL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (SCBNEXTH&0xff):
         mSCBNEXT.Byte.High=data;
         TRACE_SUSIE2("Poke(SCBNEXTH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (SPRDLINEL&0xff):
         mSPRDLINE.Byte.Low=data;
         mSPRDLINE.Byte.High=0;
         TRACE_SUSIE2("Poke(SPRDLINEL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (SPRDLINEH&0xff):
         mSPRDLINE.Byte.High=data;
         TRACE_SUSIE2("Poke(SPRDLINEH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (HPOSSTRTL&0xff):
         mHPOSSTRT.Byte.Low=data;
         mHPOSSTRT.Byte.High=0;
         TRACE_SUSIE2("Poke(HPOSSTRTL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (HPOSSTRTH&0xff):
         mHPOSSTRT.Byte.High=data;
         TRACE_SUSIE2("Poke(HPOSSTRTH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (VPOSSTRTL&0xff):
         mVPOSSTRT.Byte.Low=data;
         mVPOSSTRT.Byte.High=0;
         TRACE_SUSIE2("Poke(VPOSSTRTL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (VPOSSTRTH&0xff):
         mVPOSSTRT.Byte.High=data;
         TRACE_SUSIE2("Poke(VPOSSTRTH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (SPRHSIZL&0xff):
         mSPRHSIZ.Byte.Low=data;
         mSPRHSIZ.Byte.High=0;
         TRACE_SUSIE2("Poke(SPRHSIZL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (SPRHSIZH&0xff):
         mSPRHSIZ.Byte.High=data;
         TRACE_SUSIE2("Poke(SPRHSIZH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (SPRVSIZL&0xff):
         mSPRVSIZ.Byte.Low=data;
         mSPRVSIZ.Byte.High=0;
         TRACE_SUSIE2("Poke(SPRVSIZL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (SPRVSIZH&0xff):
         mSPRVSIZ.Byte.High=data;
         TRACE_SUSIE2("Poke(SPRVSIZH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (STRETCHL&0xff):
         mSTRETCH.Byte.Low=data;
         mSTRETCH.Byte.High=0;
         TRACE_SUSIE2("Poke(STRETCHL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (STRETCHH&0xff):
         TRACE_SUSIE2("Poke(STRETCHH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         mSTRETCH.Byte.High=data;
         break;
      case (TILTL&0xff):
         mTILT.Byte.Low=data;
         mTILT.Byte.High=0;
         TRACE_SUSIE2("Poke(TILTL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TILTH&0xff):
         mTILT.Byte.High=data;
         TRACE_SUSIE2("Poke(TILTH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (SPRDOFFL&0xff):
         TRACE_SUSIE2("Poke(SPRDOFFL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         mSPRDOFF.Byte.Low=data;
         mSPRDOFF.Byte.High=0;
         break;
      case (SPRDOFFH&0xff):
         TRACE_SUSIE2("Poke(SPRDOFFH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         mSPRDOFF.Byte.High=data;
         break;
      case (SPRVPOSL&0xff):
         TRACE_SUSIE2("Poke(SPRVPOSL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         mSPRVPOS.Byte.Low=data;
         mSPRVPOS.Byte.High=0;
         break;
      case (SPRVPOSH&0xff):
         mSPRVPOS.Byte.High=data;
         TRACE_SUSIE2("Poke(SPRVPOSH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (COLLOFFL&0xff):
         mCOLLOFF.Byte.Low=data;
         mCOLLOFF.Byte.High=0;
         TRACE_SUSIE2("Poke(COLLOFFL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (COLLOFFH&0xff):
         mCOLLOFF.Byte.High=data;
         TRACE_SUSIE2("Poke(COLLOFFH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (VSIZACUML&0xff):
         mVSIZACUM.Byte.Low=data;
         mVSIZACUM.Byte.High=0;
         TRACE_SUSIE2("Poke(VSIZACUML,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (VSIZACUMH&0xff):
         mVSIZACUM.Byte.High=data;
         TRACE_SUSIE2("Poke(VSIZACUMH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (HSIZOFFL&0xff):
         mHSIZOFF.Byte.Low=data;
         mHSIZOFF.Byte.High=0;
         TRACE_SUSIE2("Poke(HSIZOFFL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (HSIZOFFH&0xff):
         mHSIZOFF.Byte.High=data;
         TRACE_SUSIE2("Poke(HSIZOFFH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (VSIZOFFL&0xff):
         mVSIZOFF.Byte.Low=data;
         mVSIZOFF.Byte.High=0;
         TRACE_SUSIE2("Poke(VSIZOFFL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (VSIZOFFH&0xff):
         mVSIZOFF.Byte.High=data;
         TRACE_SUSIE2("Poke(VSIZOFFH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (SCBADRL&0xff):
         mSCBADR.Byte.Low=data;
         mSCBADR.Byte.High=0;
         TRACE_SUSIE2("Poke(SCBADRL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (SCBADRH&0xff):
         mSCBADR.Byte.High=data;
         TRACE_SUSIE2("Poke(SCBADRH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (PROCADRL&0xff):
         mPROCADR.Byte.Low=data;
         mPROCADR.Byte.High=0;
         TRACE_SUSIE2("Poke(PROCADRL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (PROCADRH&0xff):
         mPROCADR.Byte.High=data;
         TRACE_SUSIE2("Poke(PROCADRH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;

      case (MATHD&0xff):
         TRACE_SUSIE2("Poke(MATHD,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         mMATHABCD.Bytes.D=data;
         //			mMATHABCD.Bytes.C=0;
         // The hardware manual says that the sign shouldnt change
         // but if I dont do this then stun runner will hang as it
         // does the init in the wrong order and if the previous
         // calc left a zero there then we'll get a sign error
         Poke(MATHC,0);
         break;
      case (MATHC&0xff):
         TRACE_SUSIE2("Poke(MATHC,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         mMATHABCD.Bytes.C=data;
         // Perform sign conversion if required
         if(mSPRSYS_SignedMath) {
            // Account for the math bug that 0x8000 is +ve & 0x0000 is -ve by subracting 1
            if((mMATHABCD.Words.CD-1)&0x8000) {
               UWORD conv;
               conv=mMATHABCD.Words.CD^0xffff;
               conv++;
               mMATHCD_sign=-1;
               TRACE_SUSIE2("MATH CD signed conversion complete %04x to %04x",mMATHABCD.Words.CD,conv);
               mMATHABCD.Words.CD=conv;
            } else {
               mMATHCD_sign=1;
            }
         }
         break;
      case (MATHB&0xff):
         TRACE_SUSIE2("Poke(MATHB,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         mMATHABCD.Bytes.B=data;
         mMATHABCD.Bytes.A=0;
         break;
      case (MATHA&0xff):
         TRACE_SUSIE2("Poke(MATHA,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         mMATHABCD.Bytes.A=data;
         // Perform sign conversion if required
         if(mSPRSYS_SignedMath) {
            // Account for the math bug that 0x8000 is +ve & 0x0000 is -ve by subracting 1
            if((mMATHABCD.Words.AB-1)&0x8000) {
               UWORD conv;
               conv=mMATHABCD.Words.AB^0xffff;
               conv++;
               mMATHAB_sign=-1;
               TRACE_SUSIE2("MATH AB signed conversion complete %04x to %04x",mMATHABCD.Words.AB,conv);
               mMATHABCD.Words.AB=conv;
            } else {
               mMATHAB_sign=1;
            }
         }

         TRACE_SUSIE2("DoMathMultiply() AB=$%04x * CD=$%04x",mMATHABCD.Words.AB,mMATHABCD.Words.CD);

         mSPRSYS_Mathbit=FALSE;

         // Multiplies with out sign or accumulate take 44 ticks to complete.
         // Multiplies with sign and accumulate take 54 ticks to complete.
         //
         //    AB                                    EFGH
         //  * CD                                  /   NP
         // -------                            -----------
         //  EFGH                                    ABCD
         // Accumulate in JKLM         Remainder in (JK)LM
         //

         // Basic multiply is ALWAYS unsigned, sign conversion is done later
         mMATHEFGH.Long=(ULONG)mMATHABCD.Words.AB*(ULONG)mMATHABCD.Words.CD;

         if(mSPRSYS_SignedMath) {
            // Add the sign bits, only >0 is +ve result
            mMATHEFGH_sign=mMATHAB_sign+mMATHCD_sign;
            if(!mMATHEFGH_sign) {
               mMATHEFGH.Long^=0xffffffff;
               mMATHEFGH.Long++;
            }
         }

         // Check overflow, if B31 has changed from 1->0 then its overflow time
         if(mSPRSYS_Accumulate) {
            TRACE_SUSIE0("DoMathMultiply() - ACCUMULATED JKLM+=EFGH");
            ULONG tmp=mMATHJKLM.Long+mMATHEFGH.Long;
            // Let sign change indicate overflow
            if((tmp&0x80000000)!=(mMATHJKLM.Long&0x80000000)) {
               TRACE_SUSIE0("DoMathMultiply() - OVERFLOW DETECTED");
               // mSPRSYS_Mathbit=TRUE;
            }
            // Save accumulated result
            mMATHJKLM.Long=tmp;
         }
         TRACE_SUSIE1("DoMathMultiply() Results (Multi) EFGH=$%08x",mMATHEFGH.Long);
         TRACE_SUSIE1("DoMathMultiply() Results (Accum) JKLM=$%08x",mMATHJKLM.Long);
         break;

      case (MATHP&0xff):
         mMATHNP.Bytes.P=data;
         mMATHNP.Bytes.N=0;
         TRACE_SUSIE2("Poke(MATHP,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (MATHN&0xff):
         mMATHNP.Bytes.N=data;
         TRACE_SUSIE2("Poke(MATHN,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;

      case (MATHH&0xff):
         mMATHEFGH.Bytes.H=data;
         mMATHEFGH.Bytes.G=0;
         TRACE_SUSIE2("Poke(MATHH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (MATHG&0xff):
         mMATHEFGH.Bytes.G=data;
         TRACE_SUSIE2("Poke(MATHG,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (MATHF&0xff):
         mMATHEFGH.Bytes.F=data;
         mMATHEFGH.Bytes.E=0;
         TRACE_SUSIE2("Poke(MATHF,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (MATHE&0xff):
         mMATHEFGH.Bytes.E=data;
         TRACE_SUSIE2("Poke(MATHE,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         mSPRSYS_Mathbit=FALSE;

         //
         // Divides take 176 + 14*N ticks
         // (N is the number of most significant zeros in the divisor.)
         //
         //    AB                                    EFGH
         //  * CD                                  /   NP
         // -------                            -----------
         //  EFGH                                    ABCD
         // Accumulate in JKLM         Remainder in (JK)LM
         //

         // Divide is ALWAYS unsigned arithmetic...
         if(mMATHNP.Long) {
            TRACE_SUSIE0("DoMathDivide() - UNSIGNED");
            mMATHABCD.Long=mMATHEFGH.Long/mMATHNP.Long;
            mMATHJKLM.Long=mMATHEFGH.Long%mMATHNP.Long;
            // mSPRSYS_Mathbit=FALSE;
         } else {
            TRACE_SUSIE0("DoMathDivide() - DIVIDE BY ZERO ERROR");
            mMATHABCD.Long=0xffffffff;
            mMATHJKLM.Long=0;
            mSPRSYS_Mathbit=TRUE;
         }
         break;

      case (MATHM&0xff):
         mMATHJKLM.Bytes.M=data;
         mMATHJKLM.Bytes.L=0;
         mSPRSYS_Mathbit=FALSE;
         TRACE_SUSIE2("Poke(MATHM,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (MATHL&0xff):
         mMATHJKLM.Bytes.L=data;
         TRACE_SUSIE2("Poke(MATHL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (MATHK&0xff):
         mMATHJKLM.Bytes.K=data;
         mMATHJKLM.Bytes.J=0;
         TRACE_SUSIE2("Poke(MATHK,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (MATHJ&0xff):
         mMATHJKLM.Bytes.J=data;
         TRACE_SUSIE2("Poke(MATHJ,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;

      case (SPRCTL0&0xff):
         // mSPRCTL0_Type=data&0x0007;
         mSPRCTL0_Vflip=data&0x0010;
         mSPRCTL0_Hflip=data&0x0020;
         mSPRCTL0_PixelBits=((data&0x00c0)>>6)+1;
         TRACE_SUSIE2("Poke(SPRCTL0,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (SPRCTL1&0xff):
         mSPRCTL1_StartLeft=data&0x0001;
         mSPRCTL1_StartUp=data&0x0002;
         mSPRCTL1_SkipSprite=data&0x0004;
         mSPRCTL1_ReloadPalette=data&0x0008;
         mSPRCTL1_ReloadDepth=(data&0x0030)>>4;
         mSPRCTL1_Sizing=data&0x0040;
         mSPRCTL1_Literal=data&0x0080;
         TRACE_SUSIE2("Poke(SPRCTL1,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (SPRCOLL&0xff):
         mSPRCOLL_Number=data&0x000f;
         mSPRCOLL_Collide=data&0x0020;
         TRACE_SUSIE2("Poke(SPRCOLL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (SPRINIT&0xff):
         mSPRINIT.Byte=data;
         TRACE_SUSIE2("Poke(SPRINIT,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (SUZYBUSEN&0xff):
         mSUZYBUSEN=data&0x01;
         TRACE_SUSIE2("Poke(SUZYBUSEN,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (SPRGO&0xff):
         mSPRGO=data&0x01;
         mEVERON=data&0x04;
         TRACE_SUSIE2("Poke(SPRGO,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (SPRSYS&0xff):
         mSPRSYS_StopOnCurrent=data&0x0002;
         if(data&0x0004) mSPRSYS_UnsafeAccess=0;
         mSPRSYS_LeftHand=data&0x0008;
         mSPRSYS_VStretch=data&0x0010;
         mSPRSYS_NoCollide=data&0x0020;
         mSPRSYS_Accumulate=data&0x0040;
         mSPRSYS_SignedMath=data&0x0080;
         TRACE_SUSIE2("Poke(SPRSYS,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;

         // Cartridge writing ports

      case (RCART0&0xff):
         if(mSystem.mCart->CartGetAudin() && mSystem.mMikie->SwitchAudInValue()){
           mSystem.mCart->Poke0A(data);
         }else{
           mSystem.mCart->Poke0(data);
         }
         mSystem.mEEPROM->ProcessEepromCounter(mSystem.mCart->GetCounterValue());
         TRACE_SUSIE2("Poke(RCART0,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;
      case (RCART1&0xff):
        if(mSystem.mCart->CartGetAudin() && mSystem.mMikie->SwitchAudInValue()){
           mSystem.mCart->Poke1A(data);
        }else{
           mSystem.mCart->Poke1(data);
         }
         mSystem.mEEPROM->ProcessEepromCounter(mSystem.mCart->GetCounterValue());
         TRACE_SUSIE2("Poke(RCART1,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;

         // These are not so important, so lets ignore them for the moment

      case (LEDS&0xff):
      case (PPORTSTAT&0xff):
      case (PPORTDATA&0xff):
      case (HOWIE&0xff):
         TRACE_SUSIE2("Poke(LEDS/PPORTSTST/PPORTDATA/HOWIE,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
         break;

         // Errors on read only register accesses

      case (SUZYHREV&0xff):
      case (JOYSTICK&0xff):
      case (SWITCHES&0xff):
         TRACE_SUSIE3("Poke(%04x,%02x) - Poke to read only register location at PC=%04x",addr,data,mSystem.mCpu->GetPC());
         break;

         // Errors on illegal location accesses

      default:
         TRACE_SUSIE3("Poke(%04x,%02x) - Poke to illegal location at PC=%04x",addr,data,mSystem.mCpu->GetPC());
         break;
   }
}

UBYTE CSusie::Peek(ULONG addr)
{
   UBYTE	retval=0;
   switch(addr&0xff) {
      case (TMPADRL&0xff):
         retval=mTMPADR.Byte.Low;
         TRACE_SUSIE2("Peek(TMPADRL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (TMPADRH&0xff):
         retval=mTMPADR.Byte.High;
         TRACE_SUSIE2("Peek(TMPADRH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (TILTACUML&0xff):
         retval=mTILTACUM.Byte.Low;
         TRACE_SUSIE2("Peek(TILTACUML)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (TILTACUMH&0xff):
         retval=mTILTACUM.Byte.High;
         TRACE_SUSIE2("Peek(TILTACUMH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (HOFFL&0xff):
         retval=mHOFF.Byte.Low;
         TRACE_SUSIE2("Peek(HOFFL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (HOFFH&0xff):
         retval=mHOFF.Byte.High;
         TRACE_SUSIE2("Peek(HOFFH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (VOFFL&0xff):
         retval=mVOFF.Byte.Low;
         TRACE_SUSIE2("Peek(VOFFL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (VOFFH&0xff):
         retval=mVOFF.Byte.High;
         TRACE_SUSIE2("Peek(VOFFH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (VIDBASL&0xff):
         retval=mVIDBAS.Byte.Low;
         TRACE_SUSIE2("Peek(VIDBASL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (VIDBASH&0xff):
         retval=mVIDBAS.Byte.High;
         TRACE_SUSIE2("Peek(VIDBASH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (COLLBASL&0xff):
         retval=mCOLLBAS.Byte.Low;
         TRACE_SUSIE2("Peek(COLLBASL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (COLLBASH&0xff):
         retval=mCOLLBAS.Byte.High;
         TRACE_SUSIE2("Peek(COLLBASH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (VIDADRL&0xff):
         retval=mVIDADR.Byte.Low;
         TRACE_SUSIE2("Peek(VIDADRL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (VIDADRH&0xff):
         retval=mVIDADR.Byte.High;
         TRACE_SUSIE2("Peek(VIDADRH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (COLLADRL&0xff):
         retval=mCOLLADR.Byte.Low;
         TRACE_SUSIE2("Peek(COLLADRL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (COLLADRH&0xff):
         retval=mCOLLADR.Byte.High;
         TRACE_SUSIE2("Peek(COLLADRH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (SCBNEXTL&0xff):
         retval=mSCBNEXT.Byte.Low;
         TRACE_SUSIE2("Peek(SCBNEXTL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (SCBNEXTH&0xff):
         retval=mSCBNEXT.Byte.High;
         TRACE_SUSIE2("Peek(SCBNEXTH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (SPRDLINEL&0xff):
         retval=mSPRDLINE.Byte.Low;
         TRACE_SUSIE2("Peek(SPRDLINEL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (SPRDLINEH&0xff):
         retval=mSPRDLINE.Byte.High;
         TRACE_SUSIE2("Peek(SPRDLINEH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (HPOSSTRTL&0xff):
         retval=mHPOSSTRT.Byte.Low;
         TRACE_SUSIE2("Peek(HPOSSTRTL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (HPOSSTRTH&0xff):
         retval=mHPOSSTRT.Byte.High;
         TRACE_SUSIE2("Peek(HPOSSTRTH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (VPOSSTRTL&0xff):
         retval=mVPOSSTRT.Byte.Low;
         TRACE_SUSIE2("Peek(VPOSSTRTL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (VPOSSTRTH&0xff):
         retval=mVPOSSTRT.Byte.High;
         TRACE_SUSIE2("Peek(VPOSSTRTH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (SPRHSIZL&0xff):
         retval=mSPRHSIZ.Byte.Low;
         TRACE_SUSIE2("Peek(SPRHSIZL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (SPRHSIZH&0xff):
         retval=mSPRHSIZ.Byte.High;
         TRACE_SUSIE2("Peek(SPRHSIZH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (SPRVSIZL&0xff):
         retval=mSPRVSIZ.Byte.Low;
         TRACE_SUSIE2("Peek(SPRVSIZL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (SPRVSIZH&0xff):
         retval=mSPRVSIZ.Byte.High;
         TRACE_SUSIE2("Peek(SPRVSIZH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (STRETCHL&0xff):
         retval=mSTRETCH.Byte.Low;
         TRACE_SUSIE2("Peek(STRETCHL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (STRETCHH&0xff):
         retval=mSTRETCH.Byte.High;
         TRACE_SUSIE2("Peek(STRETCHH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (TILTL&0xff):
         retval=mTILT.Byte.Low;
         TRACE_SUSIE2("Peek(TILTL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (TILTH&0xff):
         retval=mTILT.Byte.High;
         TRACE_SUSIE2("Peek(TILTH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (SPRDOFFL&0xff):
         retval=mSPRDOFF.Byte.Low;
         TRACE_SUSIE2("Peek(SPRDOFFL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (SPRDOFFH&0xff):
         retval=mSPRDOFF.Byte.High;
         TRACE_SUSIE2("Peek(SPRDOFFH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (SPRVPOSL&0xff):
         retval=mSPRVPOS.Byte.Low;
         TRACE_SUSIE2("Peek(SPRVPOSL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (SPRVPOSH&0xff):
         retval=mSPRVPOS.Byte.High;
         TRACE_SUSIE2("Peek(SPRVPOSH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (COLLOFFL&0xff):
         retval=mCOLLOFF.Byte.Low;
         TRACE_SUSIE2("Peek(COLLOFFL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (COLLOFFH&0xff):
         retval=mCOLLOFF.Byte.High;
         TRACE_SUSIE2("Peek(COLLOFFH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (VSIZACUML&0xff):
         retval=mVSIZACUM.Byte.Low;
         TRACE_SUSIE2("Peek(VSIZACUML)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (VSIZACUMH&0xff):
         retval=mVSIZACUM.Byte.High;
         TRACE_SUSIE2("Peek(VSIZACUMH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (HSIZOFFL&0xff):
         retval=mHSIZOFF.Byte.Low;
         TRACE_SUSIE2("Peek(HSIZOFFL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (HSIZOFFH&0xff):
         retval=mHSIZOFF.Byte.High;
         TRACE_SUSIE2("Peek(HSIZOFFH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (VSIZOFFL&0xff):
         retval=mVSIZOFF.Byte.Low;
         TRACE_SUSIE2("Peek(VSIZOFFL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (VSIZOFFH&0xff):
         retval=mVSIZOFF.Byte.High;
         TRACE_SUSIE2("Peek(VSIZOFFH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (SCBADRL&0xff):
         retval=mSCBADR.Byte.Low;
         TRACE_SUSIE2("Peek(SCBADRL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (SCBADRH&0xff):
         retval=mSCBADR.Byte.High;
         TRACE_SUSIE2("Peek(SCBADRH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (PROCADRL&0xff):
         retval=mPROCADR.Byte.Low;
         TRACE_SUSIE2("Peek(PROCADRL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (PROCADRH&0xff):
         retval=mPROCADR.Byte.High;
         TRACE_SUSIE2("Peek(PROCADRH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (MATHD&0xff):
         retval=mMATHABCD.Bytes.D;
         TRACE_SUSIE2("Peek(MATHD)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (MATHC&0xff):
         retval=mMATHABCD.Bytes.C;
         TRACE_SUSIE2("Peek(MATHC)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (MATHB&0xff):
         retval=mMATHABCD.Bytes.B;
         TRACE_SUSIE2("Peek(MATHB)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (MATHA&0xff):
         retval=mMATHABCD.Bytes.A;
         TRACE_SUSIE2("Peek(MATHA)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (MATHP&0xff):
         retval=mMATHNP.Bytes.P;
         TRACE_SUSIE2("Peek(MATHP)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (MATHN&0xff):
         retval=mMATHNP.Bytes.N;
         TRACE_SUSIE2("Peek(MATHN)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (MATHH&0xff):
         retval=mMATHEFGH.Bytes.H;
         TRACE_SUSIE2("Peek(MATHH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (MATHG&0xff):
         retval=mMATHEFGH.Bytes.G;
         TRACE_SUSIE2("Peek(MATHG)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (MATHF&0xff):
         retval=mMATHEFGH.Bytes.F;
         TRACE_SUSIE2("Peek(MATHF)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (MATHE&0xff):
         retval=mMATHEFGH.Bytes.E;
         TRACE_SUSIE2("Peek(MATHE)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (MATHM&0xff):
         retval=mMATHJKLM.Bytes.M;
         TRACE_SUSIE2("Peek(MATHM)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (MATHL&0xff):
         retval=mMATHJKLM.Bytes.L;
         TRACE_SUSIE2("Peek(MATHL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (MATHK&0xff):
         retval=mMATHJKLM.Bytes.K;
         TRACE_SUSIE2("Peek(MATHK)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (MATHJ&0xff):
         retval=mMATHJKLM.Bytes.J;
         TRACE_SUSIE2("Peek(MATHJ)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (SUZYHREV&0xff):
         retval=0x01;
         TRACE_SUSIE2("Peek(SUZYHREV)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (SPRSYS&0xff):
         retval=0x0000;
         //	retval+=(mSPRSYS_Status)?0x0001:0x0000;
         // Use gSystemCPUSleep to signal the status instead, if we are asleep then
         // we must be rendering sprites
         retval+=(gSystemCPUSleep)?0x0001:0x0000;
         retval+=(mSPRSYS_StopOnCurrent)?0x0002:0x0000;
         retval+=(mSPRSYS_UnsafeAccess)?0x0004:0x0000;
         retval+=(mSPRSYS_LeftHand)?0x0008:0x0000;
         retval+=(mSPRSYS_VStretch)?0x0010:0x0000;
         retval+=(mSPRSYS_LastCarry)?0x0020:0x0000;
         retval+=(mSPRSYS_Mathbit)?0x0040:0x0000;
         retval+=(mSPRSYS_MathInProgress)?0x0080:0x0000;
         TRACE_SUSIE2("Peek(SPRSYS)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (JOYSTICK&0xff):
         if(mSPRSYS_LeftHand) {
            retval= mJOYSTICK.Byte;
         } else {
            TJOYSTICK Modified=mJOYSTICK;
            Modified.Bits.Left=mJOYSTICK.Bits.Right;
            Modified.Bits.Right=mJOYSTICK.Bits.Left;
            Modified.Bits.Down=mJOYSTICK.Bits.Up;
            Modified.Bits.Up=mJOYSTICK.Bits.Down;
            retval= Modified.Byte;
         }
         //			TRACE_SUSIE2("Peek(JOYSTICK)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (SWITCHES&0xff):
         retval=mSWITCHES.Byte;
         //			TRACE_SUSIE2("Peek(SWITCHES)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;

         // Cartridge reading ports

      case (RCART0&0xff):
         if(mSystem.mCart->CartGetAudin() && mSystem.mMikie->SwitchAudInValue()){
            retval=mSystem.mCart->Peek0A();
         }else{
            retval=mSystem.mCart->Peek0();
         }
         mSystem.mEEPROM->ProcessEepromCounter(mSystem.mCart->GetCounterValue());
         //			TRACE_SUSIE2("Peek(RCART0)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (RCART1&0xff):
         if(mSystem.mCart->CartGetAudin() && mSystem.mMikie->SwitchAudInValue()){
            retval=mSystem.mCart->Peek1A();
         }else{
           retval=mSystem.mCart->Peek1();
         }
         mSystem.mEEPROM->ProcessEepromCounter(mSystem.mCart->GetCounterValue());
         //			TRACE_SUSIE2("Peek(RCART1)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
         return retval;

         // These are no so important so lets ignore them for the moment

      case (LEDS&0xff):
      case (PPORTSTAT&0xff):
      case (PPORTDATA&0xff):
      case (HOWIE&0xff):
         TRACE_SUSIE1("Peek(LEDS/PPORTSTAT/PPORTDATA) at PC=$%04x",mSystem.mCpu->GetPC());
         break;

         // Errors on write only register accesses

      case (SPRCTL0&0xff):
      case (SPRCTL1&0xff):
      case (SPRCOLL&0xff):
      case (SPRINIT&0xff):
      case (SUZYBUSEN&0xff):
      case (SPRGO&0xff):
         TRACE_SUSIE2("Peek(%04x) - Peek from write only register location at PC=$%04x",addr,mSystem.mCpu->GetPC());
         break;

         // Errors on illegal location accesses

      default:
         TRACE_SUSIE2("Peek(%04x) - Peek from illegal location at PC=$%04x",addr,mSystem.mCpu->GetPC());
         break;
   }

   return 0xff;
}
