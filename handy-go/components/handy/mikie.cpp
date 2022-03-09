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
// Mikey chip emulation class                                               //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class emulates all of the Mikey hardware with the exception of the  //
// CPU and memory selector. Update() does most of the work and does screen  //
// DMA and counter updates, it also schecules in which cycle the next timer //
// update will occur so that the CSystem->Update() doesn't have to call it  //
// every cycle, massive speedup but big complexity headache.                //
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

//#define	TRACE_MIKIE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system.h"
#include "mikie.h"
#include "lynxdef.h"

static inline ULONG GetLfsrNext(ULONG current)
{
   // The table is built thus:
   //	Bits 0-11  LFSR					(12 Bits)
   //  Bits 12-20 Feedback switches	(9 Bits)
   //     (Order = 7,0,1,2,3,4,5,10,11)
   //  Order is mangled to make peek/poke easier as
   //  bit 7 is in a separate register
   //
   // Total 21 bits = 2MWords @ 4 Bytes/Word = 8MB !!!!!
   //
   // If the index is a combination of Current LFSR+Feedback the
   // table will give the next value.
#if 0
   ULONG result = 0;
   if (current & (1<<12)) result ^= (current>>7)&1;
   if (current & (1<<13)) result ^= (current>>0)&1;
   if (current & (1<<14)) result ^= (current>>1)&1;
   if (current & (1<<15)) result ^= (current>>2)&1;
   if (current & (1<<16)) result ^= (current>>3)&1;
   if (current & (1<<17)) result ^= (current>>4)&1;
   if (current & (1<<18)) result ^= (current>>5)&1;
   if (current & (1<<19)) result ^= (current>>10)&1;
   if (current & (1<<20)) result ^= (current>>11)&1;
   return (current&0xFFFFF000) | ((current<<1)&0xFFE) | (result?0:1);
#else

   static ULONG switches,lfsr,next,swloop,result;
   static const ULONG switchbits[9]={7,0,1,2,3,4,5,10,11};

   switches=current>>12;
   lfsr=current&0xfff;
   result=0;
   for(swloop=0;swloop<9;swloop++) {
      if((switches>>swloop)&0x001) result^=(lfsr>>switchbits[swloop])&0x001;
   }
   result=(result)?0:1;
   next=(switches<<12)|((lfsr<<1)&0xffe)|result;
   return next;
#endif
}


CMikie::CMikie(CSystem& parent, ULONG displayformat, ULONG samplerate)
:mSystem(parent)
{
   TRACE_MIKIE0("CMikie()");

   mpDisplayCurrent=NULL;
   mpRamPointer=NULL;
   mDisplayFormat=displayformat;
   mAudioSampleRate=samplerate;
   mDisplayPitch=HANDY_SCREEN_WIDTH * 2;

   mUART_CABLE_PRESENT=FALSE;
   mpUART_TX_CALLBACK=NULL;

   BuildPalette();
   Reset();
}

CMikie::~CMikie()
{
   TRACE_MIKIE0("~CMikie()");
}


void CMikie::Reset(void)
{
   TRACE_MIKIE0("Reset()");

   mAudioInputComparator=FALSE;	// Initialises to unknown
   mDisplayAddress=0x00;			// Initialises to unknown
   mLynxLine=0;
   mLynxLineDMACounter=0;
   mLynxAddr=0;

   mTimerStatusFlags=0x00;		// Initialises to ZERO, i.e No IRQ's
   mTimerInterruptMask=0x00;

   mpRamPointer=mSystem.GetRamPointer();	// Fetch pointer to system RAM

   mTIM_0_BKUP=0;
   mTIM_0_ENABLE_RELOAD=0;
   mTIM_0_ENABLE_COUNT=0;
   mTIM_0_LINKING=0;
   mTIM_0_CURRENT=0;
   mTIM_0_TIMER_DONE=0;
   mTIM_0_LAST_CLOCK=0;
   mTIM_0_BORROW_IN=0;
   mTIM_0_BORROW_OUT=0;
   mTIM_0_LAST_LINK_CARRY=0;
   mTIM_0_LAST_COUNT=0;

   mTIM_1_BKUP=0;
   mTIM_1_ENABLE_RELOAD=0;
   mTIM_1_ENABLE_COUNT=0;
   mTIM_1_LINKING=0;
   mTIM_1_CURRENT=0;
   mTIM_1_TIMER_DONE=0;
   mTIM_1_LAST_CLOCK=0;
   mTIM_1_BORROW_IN=0;
   mTIM_1_BORROW_OUT=0;
   mTIM_1_LAST_LINK_CARRY=0;
   mTIM_1_LAST_COUNT=0;

   mTIM_2_BKUP=0;
   mTIM_2_ENABLE_RELOAD=0;
   mTIM_2_ENABLE_COUNT=0;
   mTIM_2_LINKING=0;
   mTIM_2_CURRENT=0;
   mTIM_2_TIMER_DONE=0;
   mTIM_2_LAST_CLOCK=0;
   mTIM_2_BORROW_IN=0;
   mTIM_2_BORROW_OUT=0;
   mTIM_2_LAST_LINK_CARRY=0;
   mTIM_2_LAST_COUNT=0;

   mTIM_3_BKUP=0;
   mTIM_3_ENABLE_RELOAD=0;
   mTIM_3_ENABLE_COUNT=0;
   mTIM_3_LINKING=0;
   mTIM_3_CURRENT=0;
   mTIM_3_TIMER_DONE=0;
   mTIM_3_LAST_CLOCK=0;
   mTIM_3_BORROW_IN=0;
   mTIM_3_BORROW_OUT=0;
   mTIM_3_LAST_LINK_CARRY=0;
   mTIM_3_LAST_COUNT=0;

   mTIM_4_BKUP=0;
   mTIM_4_ENABLE_RELOAD=0;
   mTIM_4_ENABLE_COUNT=0;
   mTIM_4_LINKING=0;
   mTIM_4_CURRENT=0;
   mTIM_4_TIMER_DONE=0;
   mTIM_4_LAST_CLOCK=0;
   mTIM_4_BORROW_IN=0;
   mTIM_4_BORROW_OUT=0;
   mTIM_4_LAST_LINK_CARRY=0;
   mTIM_4_LAST_COUNT=0;

   mTIM_5_BKUP=0;
   mTIM_5_ENABLE_RELOAD=0;
   mTIM_5_ENABLE_COUNT=0;
   mTIM_5_LINKING=0;
   mTIM_5_CURRENT=0;
   mTIM_5_TIMER_DONE=0;
   mTIM_5_LAST_CLOCK=0;
   mTIM_5_BORROW_IN=0;
   mTIM_5_BORROW_OUT=0;
   mTIM_5_LAST_LINK_CARRY=0;
   mTIM_5_LAST_COUNT=0;

   mTIM_6_BKUP=0;
   mTIM_6_ENABLE_RELOAD=0;
   mTIM_6_ENABLE_COUNT=0;
   mTIM_6_LINKING=0;
   mTIM_6_CURRENT=0;
   mTIM_6_TIMER_DONE=0;
   mTIM_6_LAST_CLOCK=0;
   mTIM_6_BORROW_IN=0;
   mTIM_6_BORROW_OUT=0;
   mTIM_6_LAST_LINK_CARRY=0;
   mTIM_6_LAST_COUNT=0;

   mTIM_7_BKUP=0;
   mTIM_7_ENABLE_RELOAD=0;
   mTIM_7_ENABLE_COUNT=0;
   mTIM_7_LINKING=0;
   mTIM_7_CURRENT=0;
   mTIM_7_TIMER_DONE=0;
   mTIM_7_LAST_CLOCK=0;
   mTIM_7_BORROW_IN=0;
   mTIM_7_BORROW_OUT=0;
   mTIM_7_LAST_LINK_CARRY=0;
   mTIM_7_LAST_COUNT=0;

   mAUDIO_0_BKUP=0;
   mAUDIO_0_ENABLE_RELOAD=0;
   mAUDIO_0_ENABLE_COUNT=0;
   mAUDIO_0_LINKING=0;
   mAUDIO_0_CURRENT=0;
   mAUDIO_0_TIMER_DONE=0;
   mAUDIO_0_LAST_CLOCK=0;
   mAUDIO_0_BORROW_IN=0;
   mAUDIO_0_BORROW_OUT=0;
   mAUDIO_0_LAST_LINK_CARRY=0;
   mAUDIO_0_LAST_COUNT=0;
   mAUDIO_0_VOLUME=0;
   mAUDIO_OUTPUT[0]=0;
   mAUDIO_0_INTEGRATE_ENABLE=0;
   mAUDIO_0_WAVESHAPER=0;

   mAUDIO_1_BKUP=0;
   mAUDIO_1_ENABLE_RELOAD=0;
   mAUDIO_1_ENABLE_COUNT=0;
   mAUDIO_1_LINKING=0;
   mAUDIO_1_CURRENT=0;
   mAUDIO_1_TIMER_DONE=0;
   mAUDIO_1_LAST_CLOCK=0;
   mAUDIO_1_BORROW_IN=0;
   mAUDIO_1_BORROW_OUT=0;
   mAUDIO_1_LAST_LINK_CARRY=0;
   mAUDIO_1_LAST_COUNT=0;
   mAUDIO_1_VOLUME=0;
   mAUDIO_OUTPUT[1]=0;
   mAUDIO_1_INTEGRATE_ENABLE=0;
   mAUDIO_1_WAVESHAPER=0;

   mAUDIO_2_BKUP=0;
   mAUDIO_2_ENABLE_RELOAD=0;
   mAUDIO_2_ENABLE_COUNT=0;
   mAUDIO_2_LINKING=0;
   mAUDIO_2_CURRENT=0;
   mAUDIO_2_TIMER_DONE=0;
   mAUDIO_2_LAST_CLOCK=0;
   mAUDIO_2_BORROW_IN=0;
   mAUDIO_2_BORROW_OUT=0;
   mAUDIO_2_LAST_LINK_CARRY=0;
   mAUDIO_2_LAST_COUNT=0;
   mAUDIO_2_VOLUME=0;
   mAUDIO_OUTPUT[2]=0;
   mAUDIO_2_INTEGRATE_ENABLE=0;
   mAUDIO_2_WAVESHAPER=0;

   mAUDIO_3_BKUP=0;
   mAUDIO_3_ENABLE_RELOAD=0;
   mAUDIO_3_ENABLE_COUNT=0;
   mAUDIO_3_LINKING=0;
   mAUDIO_3_CURRENT=0;
   mAUDIO_3_TIMER_DONE=0;
   mAUDIO_3_LAST_CLOCK=0;
   mAUDIO_3_BORROW_IN=0;
   mAUDIO_3_BORROW_OUT=0;
   mAUDIO_3_LAST_LINK_CARRY=0;
   mAUDIO_3_LAST_COUNT=0;
   mAUDIO_3_VOLUME=0;
   mAUDIO_OUTPUT[3]=0;
   mAUDIO_3_INTEGRATE_ENABLE=0;
   mAUDIO_3_WAVESHAPER=0;

   mSTEREO=0x00;	// xored! All channels enabled
   mPAN=0x00;      // all channels panning OFF
   mAUDIO_ATTEN[0]=0xff; // Full volume
   mAUDIO_ATTEN[1]=0xff;
   mAUDIO_ATTEN[2]=0xff;
   mAUDIO_ATTEN[3]=0xff;

   // Start with an empty palette

   for(int loop=0;loop<16;loop++) {
      mPalette[loop].Index=loop;
   }

   // Initialise IODAT register

   mIODAT=0x00;
   mIODIR=0x00;
   mIODAT_REST_SIGNAL=0x00;

   //
   // Initialise display control register vars
   //
   mDISPCTL_DMAEnable=FALSE;
   mDISPCTL_Flip=FALSE;
   mDISPCTL_FourColour=0;
   mDISPCTL_Colour=0;

   //
   // Initialise the UART variables
   //
   mUART_RX_IRQ_ENABLE=0;
   mUART_TX_IRQ_ENABLE=0;

   mUART_TX_COUNTDOWN=UART_TX_INACTIVE;
   mUART_RX_COUNTDOWN=UART_RX_INACTIVE;

   mUART_Rx_input_ptr=0;
   mUART_Rx_output_ptr=0;
   mUART_Rx_waiting=0;
   mUART_Rx_framing_error=0;
   mUART_Rx_overun_error=0;

   mUART_SENDBREAK=0;
   mUART_TX_DATA=0;
   mUART_RX_DATA=0;
   mUART_RX_READY=0;

   mUART_PARITY_ENABLE=0;
   mUART_PARITY_EVEN=0;

   ResetDisplayPtr();
}


bool CMikie::ContextSave(LSS_FILE *fp)
{
   TRACE_MIKIE0("ContextSave()");

   if(!lss_printf(fp,"CMikie::ContextSave")) return 0;

   if(!lss_write(&mDisplayAddress,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAudioInputComparator,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTimerStatusFlags,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTimerInterruptMask,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(mPalette,sizeof(TPALETTE),16,fp)) return 0;

   if(!lss_write(&mIODAT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mIODAT_REST_SIGNAL,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mIODIR,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mDISPCTL_DMAEnable,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mDISPCTL_Flip,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mDISPCTL_FourColour,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mDISPCTL_Colour,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mTIM_0_BKUP,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_0_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_0_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_0_LINKING,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_0_CURRENT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_0_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_0_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_0_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_0_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_0_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_0_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mTIM_1_BKUP,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_1_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_1_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_1_LINKING,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_1_CURRENT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_1_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_1_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_1_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_1_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_1_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_1_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mTIM_2_BKUP,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_2_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_2_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_2_LINKING,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_2_CURRENT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_2_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_2_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_2_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_2_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_2_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_2_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mTIM_3_BKUP,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_3_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_3_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_3_LINKING,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_3_CURRENT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_3_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_3_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_3_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_3_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_3_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_3_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mTIM_4_BKUP,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_4_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_4_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_4_LINKING,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_4_CURRENT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_4_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_4_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_4_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_4_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_4_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_4_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mTIM_5_BKUP,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_5_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_5_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_5_LINKING,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_5_CURRENT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_5_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_5_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_5_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_5_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_5_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_5_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mTIM_6_BKUP,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_6_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_6_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_6_LINKING,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_6_CURRENT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_6_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_6_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_6_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_6_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_6_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_6_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mTIM_7_BKUP,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_7_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_7_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_7_LINKING,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_7_CURRENT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_7_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_7_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_7_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_7_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_7_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mTIM_7_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mAUDIO_0_BKUP,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_0_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_0_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_0_LINKING,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_0_CURRENT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_0_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_0_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_0_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_0_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_0_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_0_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_0_VOLUME,sizeof(SBYTE),1,fp)) return 0;
   if(!lss_write(&mAUDIO_OUTPUT[0],sizeof(SBYTE),1,fp)) return 0;
   if(!lss_write(&mAUDIO_0_INTEGRATE_ENABLE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_0_WAVESHAPER,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mAUDIO_1_BKUP,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_1_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_1_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_1_LINKING,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_1_CURRENT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_1_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_1_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_1_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_1_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_1_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_1_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_1_VOLUME,sizeof(SBYTE),1,fp)) return 0;
   if(!lss_write(&mAUDIO_OUTPUT[1],sizeof(SBYTE),1,fp)) return 0;
   if(!lss_write(&mAUDIO_1_INTEGRATE_ENABLE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_1_WAVESHAPER,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mAUDIO_2_BKUP,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_2_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_2_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_2_LINKING,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_2_CURRENT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_2_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_2_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_2_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_2_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_2_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_2_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_2_VOLUME,sizeof(SBYTE),1,fp)) return 0;
   if(!lss_write(&mAUDIO_OUTPUT[2],sizeof(SBYTE),1,fp)) return 0;
   if(!lss_write(&mAUDIO_2_INTEGRATE_ENABLE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_2_WAVESHAPER,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mAUDIO_3_BKUP,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_3_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_3_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_3_LINKING,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_3_CURRENT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_3_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_3_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_3_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_3_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_3_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_3_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_3_VOLUME,sizeof(SBYTE),1,fp)) return 0;
   if(!lss_write(&mAUDIO_OUTPUT[3],sizeof(SBYTE),1,fp)) return 0;
   if(!lss_write(&mAUDIO_3_INTEGRATE_ENABLE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAUDIO_3_WAVESHAPER,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mSTEREO,sizeof(ULONG),1,fp)) return 0;

   //
   // Serial related variables
   //
   if(!lss_write(&mUART_RX_IRQ_ENABLE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mUART_TX_IRQ_ENABLE,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mUART_TX_COUNTDOWN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mUART_RX_COUNTDOWN,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mUART_SENDBREAK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mUART_TX_DATA,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mUART_RX_DATA,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mUART_RX_READY,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mUART_PARITY_ENABLE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mUART_PARITY_EVEN,sizeof(ULONG),1,fp)) return 0;

   return 1;
}

bool CMikie::ContextLoad(LSS_FILE *fp)
{
   TRACE_MIKIE0("ContextLoad()");

   char teststr[32]="XXXXXXXXXXXXXXXXXXX";
   if(!lss_read(teststr,sizeof(char),19,fp)) return 0;
   if(strcmp(teststr,"CMikie::ContextSave")!=0) return 0;

   if(!lss_read(&mDisplayAddress,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAudioInputComparator,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTimerStatusFlags,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTimerInterruptMask,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(mPalette,sizeof(TPALETTE),16,fp)) return 0;

   if(!lss_read(&mIODAT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mIODAT_REST_SIGNAL,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mIODIR,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mDISPCTL_DMAEnable,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mDISPCTL_Flip,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mDISPCTL_FourColour,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mDISPCTL_Colour,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mTIM_0_BKUP,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_0_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_0_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_0_LINKING,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_0_CURRENT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_0_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_0_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_0_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_0_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_0_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_0_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mTIM_1_BKUP,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_1_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_1_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_1_LINKING,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_1_CURRENT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_1_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_1_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_1_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_1_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_1_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_1_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mTIM_2_BKUP,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_2_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_2_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_2_LINKING,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_2_CURRENT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_2_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_2_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_2_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_2_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_2_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_2_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mTIM_3_BKUP,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_3_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_3_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_3_LINKING,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_3_CURRENT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_3_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_3_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_3_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_3_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_3_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_3_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mTIM_4_BKUP,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_4_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_4_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_4_LINKING,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_4_CURRENT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_4_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_4_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_4_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_4_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_4_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_4_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mTIM_5_BKUP,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_5_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_5_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_5_LINKING,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_5_CURRENT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_5_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_5_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_5_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_5_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_5_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_5_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mTIM_6_BKUP,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_6_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_6_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_6_LINKING,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_6_CURRENT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_6_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_6_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_6_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_6_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_6_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_6_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mTIM_7_BKUP,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_7_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_7_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_7_LINKING,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_7_CURRENT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_7_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_7_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_7_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_7_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_7_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mTIM_7_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mAUDIO_0_BKUP,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_0_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_0_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_0_LINKING,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_0_CURRENT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_0_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_0_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_0_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_0_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_0_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_0_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_0_VOLUME,sizeof(SBYTE),1,fp)) return 0;
   if(!lss_read(&mAUDIO_OUTPUT[0],sizeof(SBYTE),1,fp)) return 0;
   if(!lss_read(&mAUDIO_0_INTEGRATE_ENABLE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_0_WAVESHAPER,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mAUDIO_1_BKUP,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_1_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_1_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_1_LINKING,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_1_CURRENT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_1_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_1_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_1_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_1_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_1_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_1_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_1_VOLUME,sizeof(SBYTE),1,fp)) return 0;
   if(!lss_read(&mAUDIO_OUTPUT[1],sizeof(SBYTE),1,fp)) return 0;
   if(!lss_read(&mAUDIO_1_INTEGRATE_ENABLE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_1_WAVESHAPER,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mAUDIO_2_BKUP,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_2_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_2_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_2_LINKING,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_2_CURRENT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_2_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_2_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_2_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_2_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_2_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_2_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_2_VOLUME,sizeof(SBYTE),1,fp)) return 0;
   if(!lss_read(&mAUDIO_OUTPUT[2],sizeof(SBYTE),1,fp)) return 0;
   if(!lss_read(&mAUDIO_2_INTEGRATE_ENABLE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_2_WAVESHAPER,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mAUDIO_3_BKUP,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_3_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_3_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_3_LINKING,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_3_CURRENT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_3_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_3_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_3_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_3_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_3_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_3_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_3_VOLUME,sizeof(SBYTE),1,fp)) return 0;
   if(!lss_read(&mAUDIO_OUTPUT[3],sizeof(SBYTE),1,fp)) return 0;
   if(!lss_read(&mAUDIO_3_INTEGRATE_ENABLE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAUDIO_3_WAVESHAPER,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mSTEREO,sizeof(ULONG),1,fp)) return 0;

   //
   // Serial related variables
   //
   if(!lss_read(&mUART_RX_IRQ_ENABLE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mUART_TX_IRQ_ENABLE,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mUART_TX_COUNTDOWN,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mUART_RX_COUNTDOWN,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mUART_SENDBREAK,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mUART_TX_DATA,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mUART_RX_DATA,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mUART_RX_READY,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mUART_PARITY_ENABLE,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mUART_PARITY_EVEN,sizeof(ULONG),1,fp)) return 0;
   return 1;
}

void CMikie::PresetForHomebrew(void)
{
   TRACE_MIKIE0("PresetForHomebrew()");

   //
   // After all of that nice timer init we'll start timers running as some homebrew
   // i.e LR.O doesn't bother to setup the timers

   mTIM_0_BKUP=0x9e;
   mTIM_0_ENABLE_RELOAD=TRUE;
   mTIM_0_ENABLE_COUNT=TRUE;

   mTIM_2_BKUP=0x68;
   mTIM_2_ENABLE_RELOAD=TRUE;
   mTIM_2_ENABLE_COUNT=TRUE;
   mTIM_2_LINKING=7;

   mDISPCTL_DMAEnable=TRUE;
   mDISPCTL_Flip=FALSE;
   mDISPCTL_FourColour=0;
   mDISPCTL_Colour=TRUE;
}

void CMikie::ComLynxCable(int status)
{
   mUART_CABLE_PRESENT=status;
}

void CMikie::ComLynxRxData(int data)
{
   TRACE_MIKIE1("ComLynxRxData() - Received %04x",data);
   // Copy over the data
   if(mUART_Rx_waiting<UART_MAX_RX_QUEUE) {
      // Trigger incoming receive IF none waiting otherwise
      // we NEVER get to receive it!!!
      if(!mUART_Rx_waiting) mUART_RX_COUNTDOWN=UART_RX_TIME_PERIOD;

      // Receive the byte
      mUART_Rx_input_queue[mUART_Rx_input_ptr++]=data;
      mUART_Rx_input_ptr %= UART_MAX_RX_QUEUE;
      mUART_Rx_waiting++;
      TRACE_MIKIE2("ComLynxRxData() - input ptr=%02d waiting=%02d",mUART_Rx_input_ptr,mUART_Rx_waiting);
   } else {
      TRACE_MIKIE0("ComLynxRxData() - UART RX Overrun");
   }
}

void CMikie::ComLynxTxLoopback(int data)
{
   TRACE_MIKIE1("ComLynxTxLoopback() - Received %04x",data);

   if(mUART_Rx_waiting<UART_MAX_RX_QUEUE) {
      // Trigger incoming receive IF none waiting otherwise
      // we NEVER get to receive it!!!
      if(!mUART_Rx_waiting) mUART_RX_COUNTDOWN=UART_RX_TIME_PERIOD;

      // Receive the byte - INSERT into front of queue
      mUART_Rx_output_ptr--;
      mUART_Rx_output_ptr %= UART_MAX_RX_QUEUE;
      mUART_Rx_input_queue[mUART_Rx_output_ptr]=data;
      mUART_Rx_waiting++;
      TRACE_MIKIE2("ComLynxTxLoopback() - input ptr=%02d waiting=%02d",mUART_Rx_input_ptr,mUART_Rx_waiting);
   } else {
      TRACE_MIKIE0("ComLynxTxLoopback() - UART RX Overrun");
   }
}

void CMikie::ComLynxTxCallback(void (*function)(int data,ULONG objref),ULONG objref)
{
   mpUART_TX_CALLBACK=function;
   mUART_TX_CALLBACK_OBJECT=objref;
}


void CMikie::BuildPalette()
{
   //
   // Calculate the colour lookup tabes for the relevant mode
   //
   TPALETTE Spot;

   for(Spot.Index=0;Spot.Index<4096;Spot.Index++) {
      mColourMap[Spot.Index]=((Spot.Colours.Red<<12)&0xf000) | ((Spot.Colours.Red<<8)&0x0800);
      mColourMap[Spot.Index]|=((Spot.Colours.Green<<7)&0x0780) | ((Spot.Colours.Green<<3)&0x0060);
      mColourMap[Spot.Index]|=((Spot.Colours.Blue<<1)&0x001e) | ((Spot.Colours.Blue>>3)&0x0001);
   }

   if (mDisplayFormat == MIKIE_PIXEL_FORMAT_16BPP_565_BE) {
      for(int i=0;i<4096;i++) {
         mColourMap[i] = mColourMap[i] << 8 | mColourMap[i] >> 8;
      }
   }

   // Reset screen related counters/vars
   mTIM_0_CURRENT=0;
   mTIM_2_CURRENT=0;

   // Fix lastcount so that timer update will definitely occur
   mTIM_0_LAST_COUNT-=(1<<(4+mTIM_0_LINKING))+1;
   mTIM_2_LAST_COUNT-=(1<<(4+mTIM_2_LINKING))+1;

   // Force immediate timer update
   gNextTimerEvent=gSystemCycleCount;
}

inline void CMikie::ResetDisplayPtr()
{
   if (mDisplayRotate != mDisplayRotate_Pending)
   {
      mDisplayRotate = mDisplayRotate_Pending;
   }

   switch(mDisplayRotate)
   {
      case MIKIE_ROTATE_L:
         mpDisplayCurrent=gPrimaryFrameBuffer+(mDisplayPitch*(HANDY_SCREEN_WIDTH-1));
         break;
      case MIKIE_ROTATE_R:
         mpDisplayCurrent=gPrimaryFrameBuffer+mDisplayPitch-(((HANDY_SCREEN_WIDTH-HANDY_SCREEN_HEIGHT))*2)-2;
         break;
      default:
         mpDisplayCurrent=gPrimaryFrameBuffer;
         break;
   }
}

inline ULONG CMikie::DisplayRenderLine(void)
{
   UWORD *bitmap_tmp=NULL;
   ULONG source,loop;
   ULONG work_done=0;

   if(!gPrimaryFrameBuffer) return 0;
   if(!mpDisplayCurrent) return 0;
   if(!mDISPCTL_DMAEnable) return 0;

   //	if(mLynxLine&0x80000000) return 0;

   // Set the timer interrupt flag
   if(mTimerInterruptMask&0x01) {
      TRACE_MIKIE0("Update() - TIMER0 IRQ Triggered (Line Timer)");
      mTimerStatusFlags|=0x01;
      gSystemIRQ=TRUE;	// Added 19/09/06 fix for IRQ issue
   }

   // Logic says it should be 101 but testing on an actual lynx shows the rest
   // period is between lines 102,101,100 with the new line being latched at
   // the beginning of count==99 hence the code below !!

   // Emulate REST signal
   if(mLynxLine==mTIM_2_BKUP-2 || mLynxLine==mTIM_2_BKUP-3 || mLynxLine==mTIM_2_BKUP-4) mIODAT_REST_SIGNAL=TRUE;
   else mIODAT_REST_SIGNAL=FALSE;

   if(mLynxLine==(mTIM_2_BKUP-3)) {
      if(mDISPCTL_Flip) {
         mLynxAddr=mDisplayAddress&0xfffc;
         mLynxAddr+=3;
      } else {
         mLynxAddr=mDisplayAddress&0xfffc;
      }
      // Trigger line rending to start
      mLynxLineDMACounter=102;

      // Reset frame buffer pointer to top of screen
      ResetDisplayPtr();
   }

   // Decrement line counter logic
   if(mLynxLine) mLynxLine--;

   // Do 102 lines, nothing more, less is OK.
   if(mLynxLineDMACounter) {
      //		TRACE_MIKIE1("Update() - Screen DMA, line %03d",line_count);
      mLynxLineDMACounter--;

      // Cycle hit for a 80 RAM access in rendering a line
      work_done+=(80+100)*DMA_RDWR_CYC;

      if (!gRenderFrame) return work_done;

      // Mikie screen DMA can only see the system RAM....
      // (Step through bitmap, line at a time)

      // Assign the temporary pointer;
      bitmap_tmp=(UWORD*)mpDisplayCurrent;

		switch(mDisplayRotate)
		{
         case MIKIE_ROTATE_L:
            for(loop=0;loop<HANDY_SCREEN_WIDTH/2;loop++)
            {
               source=mpRamPointer[mLynxAddr];
               if(mDISPCTL_Flip)
               {
                  mLynxAddr--;
                  *(bitmap_tmp)=mColourMap[mPalette[source&0x0f].Index];
                  bitmap_tmp-=HANDY_SCREEN_WIDTH;
                  *(bitmap_tmp)=mColourMap[mPalette[source>>4].Index];
                  bitmap_tmp-=HANDY_SCREEN_WIDTH;
               }
               else
               {
                  mLynxAddr++;
                  if (bitmap_tmp >= (UWORD*)(gPrimaryFrameBuffer))
                  *(bitmap_tmp)=mColourMap[mPalette[source>>4].Index];
                  bitmap_tmp-=HANDY_SCREEN_WIDTH;
                  if (bitmap_tmp >= (UWORD*)(gPrimaryFrameBuffer))
                  *(bitmap_tmp)=mColourMap[mPalette[source&0x0f].Index];
                  bitmap_tmp-=HANDY_SCREEN_WIDTH;
               }
            }
            mpDisplayCurrent+=sizeof(UWORD);
				break;
			case MIKIE_ROTATE_R:
            for(loop=0;loop<HANDY_SCREEN_WIDTH/2;loop++)
            {
               source=mpRamPointer[mLynxAddr];
               if(mDISPCTL_Flip)
               {
                  mLynxAddr--;
                  *(bitmap_tmp)=mColourMap[mPalette[source&0x0f].Index];
                  bitmap_tmp+=HANDY_SCREEN_WIDTH;
                  *(bitmap_tmp)=mColourMap[mPalette[source>>4].Index];
                  bitmap_tmp+=HANDY_SCREEN_WIDTH;
               }
               else
               {
                  mLynxAddr++;
                  *(bitmap_tmp)=mColourMap[mPalette[source>>4].Index];
                  bitmap_tmp+=HANDY_SCREEN_WIDTH;
                  *(bitmap_tmp)=mColourMap[mPalette[source&0x0f].Index];
                  bitmap_tmp+=HANDY_SCREEN_WIDTH;
               }
            }
            mpDisplayCurrent-=sizeof(UWORD);
				break;
			default:
            for(loop=0;loop<HANDY_SCREEN_WIDTH/2;loop++)
            {
               source=mpRamPointer[mLynxAddr];
               if(mDISPCTL_Flip)
               {
                  mLynxAddr--;
                  *(bitmap_tmp++)=mColourMap[mPalette[source&0x0f].Index];
                  *(bitmap_tmp++)=mColourMap[mPalette[source>>4].Index];
               }
               else
               {
                  mLynxAddr++;
                  *(bitmap_tmp++)=mColourMap[mPalette[source>>4].Index];
                  *(bitmap_tmp++)=mColourMap[mPalette[source&0x0f].Index];
               }
            }
            mpDisplayCurrent+=mDisplayPitch;
            break;
		}
   }
   return work_done;
}

// Peek/Poke memory handlers

void CMikie::Poke(ULONG addr,UBYTE data)
{
   switch(addr&0xff) {
      case (TIM0BKUP&0xff):
         mTIM_0_BKUP=data;
         TRACE_MIKIE2("Poke(TIM0BKUP,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TIM1BKUP&0xff):
         mTIM_1_BKUP=data;
         TRACE_MIKIE2("Poke(TIM1BKUP,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TIM2BKUP&0xff):
         mTIM_2_BKUP=data;
         TRACE_MIKIE2("Poke(TIM2BKUP,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TIM3BKUP&0xff):
         mTIM_3_BKUP=data;
         TRACE_MIKIE2("Poke(TIM3BKUP,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TIM4BKUP&0xff):
         mTIM_4_BKUP=data;
         TRACE_MIKIE2("Poke(TIM4BKUP,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TIM5BKUP&0xff):
         mTIM_5_BKUP=data;
         TRACE_MIKIE2("Poke(TIM5BKUP,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TIM6BKUP&0xff):
         mTIM_6_BKUP=data;
         TRACE_MIKIE2("Poke(TIM6BKUP,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TIM7BKUP&0xff):
         mTIM_7_BKUP=data;
         TRACE_MIKIE2("Poke(TIM7BKUP,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;


      case (TIM0CTLA&0xff):
         mTimerInterruptMask&=(0x01^0xff);
         mTimerInterruptMask|=(data&0x80)?0x01:0x00;
         mTIM_0_ENABLE_RELOAD=data&0x10;
         mTIM_0_ENABLE_COUNT=data&0x08;
         mTIM_0_LINKING=data&0x07;
         if(data&0x40) mTIM_0_TIMER_DONE=0;
         if(data&0x48) {
            mTIM_0_LAST_COUNT=gSystemCycleCount;
            gNextTimerEvent=gSystemCycleCount;
         }
         TRACE_MIKIE2("Poke(TIM0CTLA,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TIM1CTLA&0xff):
         mTimerInterruptMask&=(0x02^0xff);
         mTimerInterruptMask|=(data&0x80)?0x02:0x00;
         mTIM_1_ENABLE_RELOAD=data&0x10;
         mTIM_1_ENABLE_COUNT=data&0x08;
         mTIM_1_LINKING=data&0x07;
         if(data&0x40) mTIM_1_TIMER_DONE=0;
         if(data&0x48) {
            mTIM_1_LAST_COUNT=gSystemCycleCount;
            gNextTimerEvent=gSystemCycleCount;
         }
         TRACE_MIKIE2("Poke(TIM1CTLA,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TIM2CTLA&0xff):
         mTimerInterruptMask&=(0x04^0xff);
         mTimerInterruptMask|=(data&0x80)?0x04:0x00;
         mTIM_2_ENABLE_RELOAD=data&0x10;
         mTIM_2_ENABLE_COUNT=data&0x08;
         mTIM_2_LINKING=data&0x07;
         if(data&0x40) mTIM_2_TIMER_DONE=0;
         if(data&0x48) {
            mTIM_2_LAST_COUNT=gSystemCycleCount;
            gNextTimerEvent=gSystemCycleCount;
         }
         TRACE_MIKIE2("Poke(TIM2CTLA,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TIM3CTLA&0xff):
         mTimerInterruptMask&=(0x08^0xff);
         mTimerInterruptMask|=(data&0x80)?0x08:0x00;
         mTIM_3_ENABLE_RELOAD=data&0x10;
         mTIM_3_ENABLE_COUNT=data&0x08;
         mTIM_3_LINKING=data&0x07;
         if(data&0x40) mTIM_3_TIMER_DONE=0;
         if(data&0x48) {
            mTIM_3_LAST_COUNT=gSystemCycleCount;
            gNextTimerEvent=gSystemCycleCount;
         }
         TRACE_MIKIE2("Poke(TIM3CTLA,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TIM4CTLA&0xff):
         // Timer 4 can never generate interrupts as its timer output is used
         // to drive the UART clock generator
         mTIM_4_ENABLE_RELOAD=data&0x10;
         mTIM_4_ENABLE_COUNT=data&0x08;
         mTIM_4_LINKING=data&0x07;
         if(data&0x40) mTIM_4_TIMER_DONE=0;
         if(data&0x48) {
            mTIM_4_LAST_COUNT=gSystemCycleCount;
            gNextTimerEvent=gSystemCycleCount;
         }
         TRACE_MIKIE2("Poke(TIM4CTLA,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TIM5CTLA&0xff):
         mTimerInterruptMask&=(0x20^0xff);
         mTimerInterruptMask|=(data&0x80)?0x20:0x00;
         mTIM_5_ENABLE_RELOAD=data&0x10;
         mTIM_5_ENABLE_COUNT=data&0x08;
         mTIM_5_LINKING=data&0x07;
         if(data&0x40) mTIM_5_TIMER_DONE=0;
         if(data&0x48) {
            mTIM_5_LAST_COUNT=gSystemCycleCount;
            gNextTimerEvent=gSystemCycleCount;
         }
         TRACE_MIKIE2("Poke(TIM5CTLA,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TIM6CTLA&0xff):
         mTimerInterruptMask&=(0x40^0xff);
         mTimerInterruptMask|=(data&0x80)?0x40:0x00;
         mTIM_6_ENABLE_RELOAD=data&0x10;
         mTIM_6_ENABLE_COUNT=data&0x08;
         mTIM_6_LINKING=data&0x07;
         if(data&0x40) mTIM_6_TIMER_DONE=0;
         if(data&0x48) {
            mTIM_6_LAST_COUNT=gSystemCycleCount;
            gNextTimerEvent=gSystemCycleCount;
         }
         TRACE_MIKIE2("Poke(TIM6CTLA,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TIM7CTLA&0xff):
         mTimerInterruptMask&=(0x80^0xff);
         mTimerInterruptMask|=(data&0x80)?0x80:0x00;
         mTIM_7_ENABLE_RELOAD=data&0x10;
         mTIM_7_ENABLE_COUNT=data&0x08;
         mTIM_7_LINKING=data&0x07;
         if(data&0x40) mTIM_7_TIMER_DONE=0;
         if(data&0x48) {
            mTIM_7_LAST_COUNT=gSystemCycleCount;
            gNextTimerEvent=gSystemCycleCount;
         }
         TRACE_MIKIE2("Poke(TIM7CTLA,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;


      case (TIM0CNT&0xff):
         mTIM_0_CURRENT=data;
         gNextTimerEvent=gSystemCycleCount;
         TRACE_MIKIE2("Poke(TIM0CNT ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TIM1CNT&0xff):
         mTIM_1_CURRENT=data;
         gNextTimerEvent=gSystemCycleCount;
         TRACE_MIKIE2("Poke(TIM1CNT ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TIM2CNT&0xff):
         mTIM_2_CURRENT=data;
         gNextTimerEvent=gSystemCycleCount;
         TRACE_MIKIE2("Poke(TIM2CNT ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TIM3CNT&0xff):
         mTIM_3_CURRENT=data;
         gNextTimerEvent=gSystemCycleCount;
         TRACE_MIKIE2("Poke(TIM3CNT ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TIM4CNT&0xff):
         mTIM_4_CURRENT=data;
         gNextTimerEvent=gSystemCycleCount;
         TRACE_MIKIE2("Poke(TIM4CNT ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TIM5CNT&0xff):
         mTIM_5_CURRENT=data;
         gNextTimerEvent=gSystemCycleCount;
         TRACE_MIKIE2("Poke(TIM5CNT ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TIM6CNT&0xff):
         mTIM_6_CURRENT=data;
         gNextTimerEvent=gSystemCycleCount;
         TRACE_MIKIE2("Poke(TIM6CNT ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (TIM7CNT&0xff):
         mTIM_7_CURRENT=data;
         gNextTimerEvent=gSystemCycleCount;
         TRACE_MIKIE2("Poke(TIM7CNT ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;

      case (TIM0CTLB&0xff):
         mTIM_0_TIMER_DONE=data&0x08;
         mTIM_0_LAST_CLOCK=data&0x04;
         mTIM_0_BORROW_IN=data&0x02;
         mTIM_0_BORROW_OUT=data&0x01;
         TRACE_MIKIE2("Poke(TIM0CTLB ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         //			BlowOut();
         break;
      case (TIM1CTLB&0xff):
         mTIM_1_TIMER_DONE=data&0x08;
         mTIM_1_LAST_CLOCK=data&0x04;
         mTIM_1_BORROW_IN=data&0x02;
         mTIM_1_BORROW_OUT=data&0x01;
         TRACE_MIKIE2("Poke(TIM1CTLB ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         //			BlowOut();
         break;
      case (TIM2CTLB&0xff):
         mTIM_2_TIMER_DONE=data&0x08;
         mTIM_2_LAST_CLOCK=data&0x04;
         mTIM_2_BORROW_IN=data&0x02;
         mTIM_2_BORROW_OUT=data&0x01;
         TRACE_MIKIE2("Poke(TIM2CTLB ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         //			BlowOut();
         break;
      case (TIM3CTLB&0xff):
         mTIM_3_TIMER_DONE=data&0x08;
         mTIM_3_LAST_CLOCK=data&0x04;
         mTIM_3_BORROW_IN=data&0x02;
         mTIM_3_BORROW_OUT=data&0x01;
         TRACE_MIKIE2("Poke(TIM3CTLB ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         //			BlowOut();
         break;
      case (TIM4CTLB&0xff):
         mTIM_4_TIMER_DONE=data&0x08;
         mTIM_4_LAST_CLOCK=data&0x04;
         mTIM_4_BORROW_IN=data&0x02;
         mTIM_4_BORROW_OUT=data&0x01;
         TRACE_MIKIE2("Poke(TIM4CTLB ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         //			BlowOut();
         break;
      case (TIM5CTLB&0xff):
         mTIM_5_TIMER_DONE=data&0x08;
         mTIM_5_LAST_CLOCK=data&0x04;
         mTIM_5_BORROW_IN=data&0x02;
         mTIM_5_BORROW_OUT=data&0x01;
         TRACE_MIKIE2("Poke(TIM5CTLB ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         //			BlowOut();
         break;
      case (TIM6CTLB&0xff):
         mTIM_6_TIMER_DONE=data&0x08;
         mTIM_6_LAST_CLOCK=data&0x04;
         mTIM_6_BORROW_IN=data&0x02;
         mTIM_6_BORROW_OUT=data&0x01;
         TRACE_MIKIE2("Poke(TIM6CTLB ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         //			BlowOut();
         break;
      case (TIM7CTLB&0xff):
         mTIM_7_TIMER_DONE=data&0x08;
         mTIM_7_LAST_CLOCK=data&0x04;
         mTIM_7_BORROW_IN=data&0x02;
         mTIM_7_BORROW_OUT=data&0x01;
         TRACE_MIKIE2("Poke(TIM7CTLB ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         //			BlowOut();
         break;

      case (AUD0VOL&0xff):
         mAUDIO_0_VOLUME=(SBYTE)data;
         TRACE_MIKIE2("Poke(AUD0VOL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD0SHFTFB&0xff):
         mAUDIO_0_WAVESHAPER&=0x001fff;
         mAUDIO_0_WAVESHAPER|=(ULONG)data<<13;
         TRACE_MIKIE2("Poke(AUD0SHFTB,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD0OUTVAL&0xff):
         mAUDIO_OUTPUT[0]=data;
         TRACE_MIKIE2("Poke(AUD0OUTVAL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD0L8SHFT&0xff):
         mAUDIO_0_WAVESHAPER&=0x1fff00;
         mAUDIO_0_WAVESHAPER|=data;
         TRACE_MIKIE2("Poke(AUD0L8SHFT,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD0TBACK&0xff):
         // Counter is disabled when backup is zero for optimisation
         // due to the fact that the output frequency will be above audio
         // range, we must update the last use position to stop problems
         if(!mAUDIO_0_BKUP && data) {
            mAUDIO_0_LAST_COUNT=gSystemCycleCount;
            gNextTimerEvent=gSystemCycleCount;
         }
         mAUDIO_0_BKUP=data;
         TRACE_MIKIE2("Poke(AUD0TBACK,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD0CTL&0xff):
         mAUDIO_0_ENABLE_RELOAD=data&0x10;
         mAUDIO_0_ENABLE_COUNT=data&0x08;
         mAUDIO_0_LINKING=data&0x07;
         mAUDIO_0_INTEGRATE_ENABLE=data&0x20;
         if(data&0x40) mAUDIO_0_TIMER_DONE=0;
         mAUDIO_0_WAVESHAPER&=0x1fefff;
         mAUDIO_0_WAVESHAPER|=(data&0x80)?0x001000:0x000000;
         if(data&0x48) {
            mAUDIO_0_LAST_COUNT=gSystemCycleCount;
            gNextTimerEvent=gSystemCycleCount;
         }
         TRACE_MIKIE2("Poke(AUD0CTL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD0COUNT&0xff):
         mAUDIO_0_CURRENT=data;
         TRACE_MIKIE2("Poke(AUD0COUNT,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD0MISC&0xff):
         mAUDIO_0_WAVESHAPER&=0x1ff0ff;
         mAUDIO_0_WAVESHAPER|=(data&0xf0)<<4;
         mAUDIO_0_BORROW_IN=data&0x02;
         mAUDIO_0_BORROW_OUT=data&0x01;
         mAUDIO_0_LAST_CLOCK=data&0x04;
         TRACE_MIKIE2("Poke(AUD0MISC,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;

      case (AUD1VOL&0xff):
         mAUDIO_1_VOLUME=(SBYTE)data;
         TRACE_MIKIE2("Poke(AUD1VOL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD1SHFTFB&0xff):
         mAUDIO_1_WAVESHAPER&=0x001fff;
         mAUDIO_1_WAVESHAPER|=(ULONG)data<<13;
         TRACE_MIKIE2("Poke(AUD1SHFTFB,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD1OUTVAL&0xff):
         mAUDIO_OUTPUT[1]=data;
         TRACE_MIKIE2("Poke(AUD1OUTVAL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD1L8SHFT&0xff):
         mAUDIO_1_WAVESHAPER&=0x1fff00;
         mAUDIO_1_WAVESHAPER|=data;
         TRACE_MIKIE2("Poke(AUD1L8SHFT,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD1TBACK&0xff):
         // Counter is disabled when backup is zero for optimisation
         // due to the fact that the output frequency will be above audio
         // range, we must update the last use position to stop problems
         if(!mAUDIO_1_BKUP && data) {
            mAUDIO_1_LAST_COUNT=gSystemCycleCount;
            gNextTimerEvent=gSystemCycleCount;
         }
         mAUDIO_1_BKUP=data;
         TRACE_MIKIE2("Poke(AUD1TBACK,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD1CTL&0xff):
         mAUDIO_1_ENABLE_RELOAD=data&0x10;
         mAUDIO_1_ENABLE_COUNT=data&0x08;
         mAUDIO_1_LINKING=data&0x07;
         mAUDIO_1_INTEGRATE_ENABLE=data&0x20;
         if(data&0x40) mAUDIO_1_TIMER_DONE=0;
         mAUDIO_1_WAVESHAPER&=0x1fefff;
         mAUDIO_1_WAVESHAPER|=(data&0x80)?0x001000:0x000000;
         if(data&0x48) {
            mAUDIO_1_LAST_COUNT=gSystemCycleCount;
            gNextTimerEvent=gSystemCycleCount;
         }
         TRACE_MIKIE2("Poke(AUD1CTL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD1COUNT&0xff):
         mAUDIO_1_CURRENT=data;
         TRACE_MIKIE2("Poke(AUD1COUNT,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD1MISC&0xff):
         mAUDIO_1_WAVESHAPER&=0x1ff0ff;
         mAUDIO_1_WAVESHAPER|=(data&0xf0)<<4;
         mAUDIO_1_BORROW_IN=data&0x02;
         mAUDIO_1_BORROW_OUT=data&0x01;
         mAUDIO_1_LAST_CLOCK=data&0x04;
         TRACE_MIKIE2("Poke(AUD1MISC,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;

      case (AUD2VOL&0xff):
         mAUDIO_2_VOLUME=(SBYTE)data;
         TRACE_MIKIE2("Poke(AUD2VOL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD2SHFTFB&0xff):
         mAUDIO_2_WAVESHAPER&=0x001fff;
         mAUDIO_2_WAVESHAPER|=(ULONG)data<<13;
         TRACE_MIKIE2("Poke(AUD2VSHFTFB,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD2OUTVAL&0xff):
         mAUDIO_OUTPUT[2]=data;
         TRACE_MIKIE2("Poke(AUD2OUTVAL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD2L8SHFT&0xff):
         mAUDIO_2_WAVESHAPER&=0x1fff00;
         mAUDIO_2_WAVESHAPER|=data;
         TRACE_MIKIE2("Poke(AUD2L8SHFT,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD2TBACK&0xff):
         // Counter is disabled when backup is zero for optimisation
         // due to the fact that the output frequency will be above audio
         // range, we must update the last use position to stop problems
         if(!mAUDIO_2_BKUP && data) {
            mAUDIO_2_LAST_COUNT=gSystemCycleCount;
            gNextTimerEvent=gSystemCycleCount;
         }
         mAUDIO_2_BKUP=data;
         TRACE_MIKIE2("Poke(AUD2TBACK,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD2CTL&0xff):
         mAUDIO_2_ENABLE_RELOAD=data&0x10;
         mAUDIO_2_ENABLE_COUNT=data&0x08;
         mAUDIO_2_LINKING=data&0x07;
         mAUDIO_2_INTEGRATE_ENABLE=data&0x20;
         if(data&0x40) mAUDIO_2_TIMER_DONE=0;
         mAUDIO_2_WAVESHAPER&=0x1fefff;
         mAUDIO_2_WAVESHAPER|=(data&0x80)?0x001000:0x000000;
         if(data&0x48) {
            mAUDIO_2_LAST_COUNT=gSystemCycleCount;
            gNextTimerEvent=gSystemCycleCount;
         }
         TRACE_MIKIE2("Poke(AUD2CTL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD2COUNT&0xff):
         mAUDIO_2_CURRENT=data;
         TRACE_MIKIE2("Poke(AUD2COUNT,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD2MISC&0xff):
         mAUDIO_2_WAVESHAPER&=0x1ff0ff;
         mAUDIO_2_WAVESHAPER|=(data&0xf0)<<4;
         mAUDIO_2_BORROW_IN=data&0x02;
         mAUDIO_2_BORROW_OUT=data&0x01;
         mAUDIO_2_LAST_CLOCK=data&0x04;
         TRACE_MIKIE2("Poke(AUD2MISC,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;

      case (AUD3VOL&0xff):
         mAUDIO_3_VOLUME=(SBYTE)data;
         TRACE_MIKIE2("Poke(AUD3VOL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD3SHFTFB&0xff):
         mAUDIO_3_WAVESHAPER&=0x001fff;
         mAUDIO_3_WAVESHAPER|=(ULONG)data<<13;
         TRACE_MIKIE2("Poke(AUD3SHFTFB,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD3OUTVAL&0xff):
         mAUDIO_OUTPUT[3]=data;
         TRACE_MIKIE2("Poke(AUD3OUTVAL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD3L8SHFT&0xff):
         mAUDIO_3_WAVESHAPER&=0x1fff00;
         mAUDIO_3_WAVESHAPER|=data;
         TRACE_MIKIE2("Poke(AUD3L8SHFT,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD3TBACK&0xff):
         // Counter is disabled when backup is zero for optimisation
         // due to the fact that the output frequency will be above audio
         // range, we must update the last use position to stop problems
         if(!mAUDIO_3_BKUP && data) {
            mAUDIO_3_LAST_COUNT=gSystemCycleCount;
            gNextTimerEvent=gSystemCycleCount;
         }
         mAUDIO_3_BKUP=data;
         TRACE_MIKIE2("Poke(AUD3TBACK,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD3CTL&0xff):
         mAUDIO_3_ENABLE_RELOAD=data&0x10;
         mAUDIO_3_ENABLE_COUNT=data&0x08;
         mAUDIO_3_LINKING=data&0x07;
         mAUDIO_3_INTEGRATE_ENABLE=data&0x20;
         if(data&0x40) mAUDIO_3_TIMER_DONE=0;
         mAUDIO_3_WAVESHAPER&=0x1fefff;
         mAUDIO_3_WAVESHAPER|=(data&0x80)?0x001000:0x000000;
         if(data&0x48) {
            mAUDIO_3_LAST_COUNT=gSystemCycleCount;
            gNextTimerEvent=gSystemCycleCount;
         }
         TRACE_MIKIE2("Poke(AUD3CTL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD3COUNT&0xff):
         mAUDIO_3_CURRENT=data;
         TRACE_MIKIE2("Poke(AUD3COUNT,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (AUD3MISC&0xff):
         mAUDIO_3_WAVESHAPER&=0x1ff0ff;
         mAUDIO_3_WAVESHAPER|=(data&0xf0)<<4;
         mAUDIO_3_BORROW_IN=data&0x02;
         mAUDIO_3_BORROW_OUT=data&0x01;
         mAUDIO_3_LAST_CLOCK=data&0x04;
         TRACE_MIKIE2("Poke(AUD3MISC,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;

      case (ATTEN_A&0xff):
         mAUDIO_ATTEN[0] = data;
         TRACE_MIKIE2("Poke(ATTEN_A ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (ATTEN_B&0xff):
         mAUDIO_ATTEN[1] = data;
         TRACE_MIKIE2("Poke(ATTEN_B ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (ATTEN_C&0xff):
         mAUDIO_ATTEN[2] = data;
         TRACE_MIKIE2("Poke(ATTEN_C ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (ATTEN_D&0xff):
         mAUDIO_ATTEN[3] = data;
         TRACE_MIKIE2("Poke(ATTEN_D ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (MPAN&0xff):
         TRACE_MIKIE2("Poke(MPAN ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         mPAN = data;
         break;

      case (MSTEREO&0xff):
         TRACE_MIKIE2("Poke(MSTEREO,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         mSTEREO=data;
         //			if(!(mSTEREO&0x11) && (data&0x11))
         //			{
         //				mAUDIO_0_LAST_COUNT=gSystemCycleCount;
         //				gNextTimerEvent=gSystemCycleCount;
         //			}
         //			if(!(mSTEREO&0x22) && (data&0x22))
         //			{
         //				mAUDIO_1_LAST_COUNT=gSystemCycleCount;
         //				gNextTimerEvent=gSystemCycleCount;
         //			}
         //			if(!(mSTEREO&0x44) && (data&0x44))
         //			{
         //				mAUDIO_2_LAST_COUNT=gSystemCycleCount;
         //				gNextTimerEvent=gSystemCycleCount;
         //			}
         //			if(!(mSTEREO&0x88) && (data&0x88))
         //			{
         //				mAUDIO_3_LAST_COUNT=gSystemCycleCount;
         //				gNextTimerEvent=gSystemCycleCount;
         //			}
         break;

      case (INTRST&0xff):
         data^=0xff;
         mTimerStatusFlags&=data;
         gNextTimerEvent=gSystemCycleCount;
         // 22/09/06 Fix to championship rally, IRQ not getting cleared with edge triggered system
         gSystemIRQ=(mTimerStatusFlags&mTimerInterruptMask)?TRUE:FALSE;
         // 22/09/06 Fix to championship rally, IRQ not getting cleared with edge triggered system
         TRACE_MIKIE2("Poke(INTRST  ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;

      case (INTSET&0xff):
         TRACE_MIKIE2("Poke(INTSET  ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         mTimerStatusFlags|=data;
         // 22/09/06 Fix to championship rally, IRQ not getting cleared with edge triggered system
         gSystemIRQ=(mTimerStatusFlags&mTimerInterruptMask)?TRUE:FALSE;
         // 22/09/06 Fix to championship rally, IRQ not getting cleared with edge triggered system
         gNextTimerEvent=gSystemCycleCount;
         break;

      case (SYSCTL1&0xff):
         TRACE_MIKIE2("Poke(SYSCTL1 ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         if(!(data&0x02)) {
            C6502_REGS regs;
            mSystem.mCpu->GetRegs(regs);
            log_printf("CMikie::Poke(SYSCTL1) - Lynx power down occurred at PC=$%04x.\nResetting system.\n", regs.PC);
            mSystem.Reset();
            gSystemHalt=TRUE;
         }
         mSystem.mCart->CartAddressStrobe((data&0x01)?TRUE:FALSE);
         if(mSystem.mEEPROM->Available()) mSystem.mEEPROM->ProcessEepromCounter(mSystem.mCart->GetCounterValue());
         break;

      case (MIKEYSREV&0xff):
         TRACE_MIKIE2("Poke(MIKEYSREV,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;

      case (IODIR&0xff):
         TRACE_MIKIE2("Poke(IODIR   ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         mIODIR=data;
         if(mSystem.mEEPROM->Available()) mSystem.mEEPROM->ProcessEepromIO(mIODIR,mIODAT);
         break;

      case (IODAT&0xff):
         TRACE_MIKIE2("Poke(IODAT   ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         mIODAT=data;
         mSystem.mCart->CartAddressData((mIODAT&0x02)?TRUE:FALSE);
         // Enable cart writes to BANK1 on AUDIN if AUDIN is set to output
         if(mIODIR&0x10) mSystem.mCart->mWriteEnableBank1=(mIODAT&0x10)?TRUE:FALSE;// there is no reason to use AUDIN as Write Enable or latch. private patch??? TODO
         if(mSystem.mEEPROM->Available()) mSystem.mEEPROM->ProcessEepromIO(mIODIR,mIODAT);
         break;

      case (SERCTL&0xff):
         TRACE_MIKIE2("Poke(SERCTL  ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         mUART_TX_IRQ_ENABLE=(data&0x80)?true:false;
         mUART_RX_IRQ_ENABLE=(data&0x40)?true:false;
         mUART_PARITY_ENABLE=(data&0x10)?true:false;
         mUART_SENDBREAK=data&0x02;
         mUART_PARITY_EVEN=data&0x01;

         // Reset all errors if required
         if(data&0x08) {
            mUART_Rx_overun_error=0;
            mUART_Rx_framing_error=0;
         }

         if(mUART_SENDBREAK) {
            // Trigger send break, it will self sustain as long as sendbreak is set
            mUART_TX_COUNTDOWN=UART_TX_TIME_PERIOD;
            // Loop back what we transmitted
            ComLynxTxLoopback(UART_BREAK_CODE);
         }
         break;

      case (SERDAT&0xff):
         TRACE_MIKIE2("Poke(SERDAT ,%04x) at PC=%04x",data,mSystem.mCpu->GetPC());
         //
         // Fake transmission, set counter to be decremented by Timer 4
         //
         // ComLynx only has one output pin, hence Rx & Tx are shorted
         // therefore any transmitted data will loopback
         //
         mUART_TX_DATA=data;
         // Calculate Parity data
         if(mUART_PARITY_ENABLE) {
            // Calc parity value
            // Leave at zero !!
         } else {
            // If disabled then the PAREVEN bit is sent
            if(mUART_PARITY_EVEN) data|=0x0100;
         }
         // Set countdown to transmission
         mUART_TX_COUNTDOWN=UART_TX_TIME_PERIOD;
         // Loop back what we transmitted
         ComLynxTxLoopback(mUART_TX_DATA);
         break;

      case (SDONEACK&0xff):
         TRACE_MIKIE2("Poke(SDONEACK,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;
      case (CPUSLEEP&0xff):
         TRACE_MIKIE2("Poke(CPUSLEEP,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         //
         // We must do "cycles_used" cycles of the system with the CPU sleeping
         // to compensate for the sprite painting, Mikie update will autowake the
         // CPU at the right time.
         //
         TRACE_MIKIE0("*********************************************************");
         TRACE_MIKIE0("****               CPU SLEEP STARTED                 ****");
         TRACE_MIKIE0("*********************************************************");
         gCPUWakeupTime=gSystemCycleCount+(SLONG)mSystem.mSusie->PaintSprites();
         gSystemCPUSleep=TRUE;
         TRACE_MIKIE2("Poke(CPUSLEEP,%02x) wakeup at cycle =%012d",data,gCPUWakeupTime);
         break;

      case (DISPCTL&0xff):
         TRACE_MIKIE2("Poke(DISPCTL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         mDISPCTL_DMAEnable=data&0x1;
         mDISPCTL_Flip=data&0x2;
         mDISPCTL_FourColour=data&0x4;
         mDISPCTL_Colour=data&0x8;
         break;
      case (PBKUP&0xff):
         TRACE_MIKIE2("Poke(PBKUP,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;

      case (DISPADRL&0xff):
         TRACE_MIKIE2("Poke(DISPADRL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         mDisplayAddress&=0xff00;
         mDisplayAddress+=data;
         break;

      case (DISPADRH&0xff):
         TRACE_MIKIE2("Poke(DISPADRH,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         mDisplayAddress&=0x00ff;
         mDisplayAddress+=(data<<8);
         break;

      case (Mtest0&0xff):
      case (Mtest1&0xff):
      case (Mtest2&0xff):
         // Test registers are unimplemented
         // lets hope no programs use them.
         TRACE_MIKIE2("Poke(MTEST0/1/2,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         break;

      case (0xfd97&0xff):
         // This code is to intercept calls to the fake ROM
         switch (mSystem.mCpu->GetPC()) {
            case 0xFE00+3: mSystem.HLE_BIOS_FE00(); break;
            case 0xFE19+3: mSystem.HLE_BIOS_FE19(); break;
            case 0xFE4A+3: mSystem.HLE_BIOS_FE4A(); break;
            case 0xFF80+3: mSystem.HLE_BIOS_FF80(); break;
            default: log_printf("BIOS: Missing function $%04X\n", mSystem.mCpu->GetPC());
         }
         break;
      case (GREEN0&0xff):
      case (GREEN1&0xff):
      case (GREEN2&0xff):
      case (GREEN3&0xff):
      case (GREEN4&0xff):
      case (GREEN5&0xff):
      case (GREEN6&0xff):
      case (GREEN7&0xff):
      case (GREEN8&0xff):
      case (GREEN9&0xff):
      case (GREENA&0xff):
      case (GREENB&0xff):
      case (GREENC&0xff):
      case (GREEND&0xff):
      case (GREENE&0xff):
      case (GREENF&0xff):
         TRACE_MIKIE2("Poke(GREENPAL0-F,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         mPalette[addr&0x0f].Colours.Green=data&0x0f;
         break;

      case (BLUERED0&0xff):
      case (BLUERED1&0xff):
      case (BLUERED2&0xff):
      case (BLUERED3&0xff):
      case (BLUERED4&0xff):
      case (BLUERED5&0xff):
      case (BLUERED6&0xff):
      case (BLUERED7&0xff):
      case (BLUERED8&0xff):
      case (BLUERED9&0xff):
      case (BLUEREDA&0xff):
      case (BLUEREDB&0xff):
      case (BLUEREDC&0xff):
      case (BLUEREDD&0xff):
      case (BLUEREDE&0xff):
      case (BLUEREDF&0xff):
         TRACE_MIKIE2("Poke(BLUEREDPAL0-F,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
         mPalette[addr&0x0f].Colours.Blue=(data&0xf0)>>4;
         mPalette[addr&0x0f].Colours.Red=data&0x0f;
         break;

         // Errors on read only register accesses

      case (MAGRDY0&0xff):
      case (MAGRDY1&0xff):
      case (AUDIN&0xff):
      case (MIKEYHREV&0xff):
         TRACE_MIKIE3("Poke(%04x,%02x) - Poke to read only register location at PC=%04x",addr,data,mSystem.mCpu->GetPC());
         break;

         // Errors on illegal location accesses

      default:
         TRACE_MIKIE3("Poke(%04x,%02x) - Poke to illegal location at PC=%04x",addr,data,mSystem.mCpu->GetPC());
         break;
   }

   if(addr >= AUD0VOL && addr <= MSTEREO)
	  UpdateSound();
}



UBYTE CMikie::Peek(ULONG addr)
{
   ULONG retval=0;

   switch(addr & 0xff) {
   // Timer control registers
      case (TIM0BKUP&0xff):
         TRACE_MIKIE2("Peek(TIM0KBUP ,%02x) at PC=%04x",mTIM_0_BKUP,mSystem.mCpu->GetPC());
         return (UBYTE)mTIM_0_BKUP;
      case (TIM1BKUP&0xff):
         TRACE_MIKIE2("Peek(TIM1KBUP ,%02x) at PC=%04x",mTIM_1_BKUP,mSystem.mCpu->GetPC());
         return (UBYTE)mTIM_1_BKUP;
      case (TIM2BKUP&0xff):
         TRACE_MIKIE2("Peek(TIM2KBUP ,%02x) at PC=%04x",mTIM_2_BKUP,mSystem.mCpu->GetPC());
         return (UBYTE)mTIM_2_BKUP;
      case (TIM3BKUP&0xff):
         TRACE_MIKIE2("Peek(TIM3KBUP ,%02x) at PC=%04x",mTIM_3_BKUP,mSystem.mCpu->GetPC());
         return (UBYTE)mTIM_3_BKUP;
      case (TIM4BKUP&0xff):
         TRACE_MIKIE2("Peek(TIM4KBUP ,%02x) at PC=%04x",mTIM_4_BKUP,mSystem.mCpu->GetPC());
         return (UBYTE)mTIM_4_BKUP;
      case (TIM5BKUP&0xff):
         TRACE_MIKIE2("Peek(TIM5KBUP ,%02x) at PC=%04x",mTIM_5_BKUP,mSystem.mCpu->GetPC());
         return (UBYTE)mTIM_5_BKUP;
      case (TIM6BKUP&0xff):
         TRACE_MIKIE2("Peek(TIM6KBUP ,%02x) at PC=%04x",mTIM_6_BKUP,mSystem.mCpu->GetPC());
         return (UBYTE)mTIM_6_BKUP;
      case (TIM7BKUP&0xff):
         TRACE_MIKIE2("Peek(TIM7KBUP ,%02x) at PC=%04x",mTIM_7_BKUP,mSystem.mCpu->GetPC());
         return (UBYTE)mTIM_7_BKUP;

      case (TIM0CTLA&0xff):
         retval|=(mTimerInterruptMask&0x01)?0x80:0x00;
         retval|=(mTIM_0_ENABLE_RELOAD)?0x10:0x00;
         retval|=(mTIM_0_ENABLE_COUNT)?0x08:0x00;
         retval|=mTIM_0_LINKING;
         TRACE_MIKIE2("Peek(TIM0CTLA ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (TIM1CTLA&0xff):
         retval|=(mTimerInterruptMask&0x02)?0x80:0x00;
         retval|=(mTIM_1_ENABLE_RELOAD)?0x10:0x00;
         retval|=(mTIM_1_ENABLE_COUNT)?0x08:0x00;
         retval|=mTIM_1_LINKING;
         TRACE_MIKIE2("Peek(TIM1CTLA ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (TIM2CTLA&0xff):
         retval|=(mTimerInterruptMask&0x04)?0x80:0x00;
         retval|=(mTIM_2_ENABLE_RELOAD)?0x10:0x00;
         retval|=(mTIM_2_ENABLE_COUNT)?0x08:0x00;
         retval|=mTIM_2_LINKING;
         TRACE_MIKIE2("Peek(TIM2CTLA ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (TIM3CTLA&0xff):
         retval|=(mTimerInterruptMask&0x08)?0x80:0x00;
         retval|=(mTIM_3_ENABLE_RELOAD)?0x10:0x00;
         retval|=(mTIM_3_ENABLE_COUNT)?0x08:0x00;
         retval|=mTIM_3_LINKING;
         TRACE_MIKIE2("Peek(TIM3CTLA ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (TIM4CTLA&0xff):
         retval|=(mTimerInterruptMask&0x10)?0x80:0x00;
         retval|=(mTIM_4_ENABLE_RELOAD)?0x10:0x00;
         retval|=(mTIM_4_ENABLE_COUNT)?0x08:0x00;
         retval|=mTIM_4_LINKING;
         TRACE_MIKIE2("Peek(TIM4CTLA ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (TIM5CTLA&0xff):
         retval|=(mTimerInterruptMask&0x20)?0x80:0x00;
         retval|=(mTIM_5_ENABLE_RELOAD)?0x10:0x00;
         retval|=(mTIM_5_ENABLE_COUNT)?0x08:0x00;
         retval|=mTIM_5_LINKING;
         TRACE_MIKIE2("Peek(TIM5CTLA ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (TIM6CTLA&0xff):
         retval|=(mTimerInterruptMask&0x40)?0x80:0x00;
         retval|=(mTIM_6_ENABLE_RELOAD)?0x10:0x00;
         retval|=(mTIM_6_ENABLE_COUNT)?0x08:0x00;
         retval|=mTIM_6_LINKING;
         TRACE_MIKIE2("Peek(TIM6CTLA ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (TIM7CTLA&0xff):
         retval|=(mTimerInterruptMask&0x80)?0x80:0x00;
         retval|=(mTIM_7_ENABLE_RELOAD)?0x10:0x00;
         retval|=(mTIM_7_ENABLE_COUNT)?0x08:0x00;
         retval|=mTIM_7_LINKING;
         TRACE_MIKIE2("Peek(TIM7CTLA ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (TIM0CNT&0xff):
         Update();
         TRACE_MIKIE2("Peek(TIM0CNT  ,%02x) at PC=%04x",mTIM_0_CURRENT,mSystem.mCpu->GetPC());
         return (UBYTE)mTIM_0_CURRENT;
      case (TIM1CNT&0xff):
         Update();
         TRACE_MIKIE2("Peek(TIM1CNT  ,%02x) at PC=%04x",mTIM_1_CURRENT,mSystem.mCpu->GetPC());
         return (UBYTE)mTIM_1_CURRENT;
      case (TIM2CNT&0xff):
         Update();
         TRACE_MIKIE2("Peek(TIM2CNT  ,%02x) at PC=%04x",mTIM_2_CURRENT,mSystem.mCpu->GetPC());
         return (UBYTE)mTIM_2_CURRENT;
      case (TIM3CNT&0xff):
         Update();
         TRACE_MIKIE2("Peek(TIM3CNT  ,%02x) at PC=%04x",mTIM_3_CURRENT,mSystem.mCpu->GetPC());
         return (UBYTE)mTIM_3_CURRENT;
      case (TIM4CNT&0xff):
         Update();
         TRACE_MIKIE2("Peek(TIM4CNT  ,%02x) at PC=%04x",mTIM_4_CURRENT,mSystem.mCpu->GetPC());
         return (UBYTE)mTIM_4_CURRENT;
      case (TIM5CNT&0xff):
         Update();
         TRACE_MIKIE2("Peek(TIM5CNT  ,%02x) at PC=%04x",mTIM_5_CURRENT,mSystem.mCpu->GetPC());
         return (UBYTE)mTIM_5_CURRENT;
      case (TIM6CNT&0xff):
         Update();
         TRACE_MIKIE2("Peek(TIM6CNT  ,%02x) at PC=%04x",mTIM_6_CURRENT,mSystem.mCpu->GetPC());
         return (UBYTE)mTIM_6_CURRENT;
      case (TIM7CNT&0xff):
         Update();
         TRACE_MIKIE2("Peek(TIM7CNT  ,%02x) at PC=%04x",mTIM_7_CURRENT,mSystem.mCpu->GetPC());
         return (UBYTE)mTIM_7_CURRENT;

      case (TIM0CTLB&0xff):
         retval|=(mTIM_0_TIMER_DONE)?0x08:0x00;
         retval|=(mTIM_0_LAST_CLOCK)?0x04:0x00;
         retval|=(mTIM_0_BORROW_IN)?0x02:0x00;
         retval|=(mTIM_0_BORROW_OUT)?0x01:0x00;
         TRACE_MIKIE2("Peek(TIM0CTLB ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (TIM1CTLB&0xff):
         retval|=(mTIM_1_TIMER_DONE)?0x08:0x00;
         retval|=(mTIM_1_LAST_CLOCK)?0x04:0x00;
         retval|=(mTIM_1_BORROW_IN)?0x02:0x00;
         retval|=(mTIM_1_BORROW_OUT)?0x01:0x00;
         TRACE_MIKIE2("Peek(TIM1CTLB ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (TIM2CTLB&0xff):
         retval|=(mTIM_2_TIMER_DONE)?0x08:0x00;
         retval|=(mTIM_2_LAST_CLOCK)?0x04:0x00;
         retval|=(mTIM_2_BORROW_IN)?0x02:0x00;
         retval|=(mTIM_2_BORROW_OUT)?0x01:0x00;
         TRACE_MIKIE2("Peek(TIM2CTLB ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (TIM3CTLB&0xff):
         retval|=(mTIM_3_TIMER_DONE)?0x08:0x00;
         retval|=(mTIM_3_LAST_CLOCK)?0x04:0x00;
         retval|=(mTIM_3_BORROW_IN)?0x02:0x00;
         retval|=(mTIM_3_BORROW_OUT)?0x01:0x00;
         TRACE_MIKIE2("Peek(TIM3CTLB ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (TIM4CTLB&0xff):
         retval|=(mTIM_4_TIMER_DONE)?0x08:0x00;
         retval|=(mTIM_4_LAST_CLOCK)?0x04:0x00;
         retval|=(mTIM_4_BORROW_IN)?0x02:0x00;
         retval|=(mTIM_4_BORROW_OUT)?0x01:0x00;
         TRACE_MIKIE2("Peek(TIM4CTLB ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (TIM5CTLB&0xff):
         retval|=(mTIM_5_TIMER_DONE)?0x08:0x00;
         retval|=(mTIM_5_LAST_CLOCK)?0x04:0x00;
         retval|=(mTIM_5_BORROW_IN)?0x02:0x00;
         retval|=(mTIM_5_BORROW_OUT)?0x01:0x00;
         TRACE_MIKIE2("Peek(TIM5CTLB ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (TIM6CTLB&0xff):
         retval|=(mTIM_6_TIMER_DONE)?0x08:0x00;
         retval|=(mTIM_6_LAST_CLOCK)?0x04:0x00;
         retval|=(mTIM_6_BORROW_IN)?0x02:0x00;
         retval|=(mTIM_6_BORROW_OUT)?0x01:0x00;
         TRACE_MIKIE2("Peek(TIM6CTLB ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (TIM7CTLB&0xff):
         retval|=(mTIM_7_TIMER_DONE)?0x08:0x00;
         retval|=(mTIM_7_LAST_CLOCK)?0x04:0x00;
         retval|=(mTIM_7_BORROW_IN)?0x02:0x00;
         retval|=(mTIM_7_BORROW_OUT)?0x01:0x00;
         TRACE_MIKIE2("Peek(TIM7CTLB ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      // Audio control registers

      case (AUD0VOL&0xff):
         TRACE_MIKIE2("Peek(AUD0VOL,%02x) at PC=%04x",(UBYTE)mAUDIO_0_VOLUME,mSystem.mCpu->GetPC());
         return (UBYTE)mAUDIO_0_VOLUME;
      case (AUD0SHFTFB&0xff):
         TRACE_MIKIE2("Peek(AUD0SHFTFB,%02x) at PC=%04x",(UBYTE)(mAUDIO_0_WAVESHAPER>>13)&0xff,mSystem.mCpu->GetPC());
         return (UBYTE)((mAUDIO_0_WAVESHAPER>>13)&0xff);
      case (AUD0OUTVAL&0xff):
         TRACE_MIKIE2("Peek(AUD0OUTVAL,%02x) at PC=%04x",(UBYTE)mAUDIO_OUTPUT[0],mSystem.mCpu->GetPC());
         return (UBYTE)mAUDIO_OUTPUT[0];
      case (AUD0L8SHFT&0xff):
         TRACE_MIKIE2("Peek(AUD0L8SHFT,%02x) at PC=%04x",(UBYTE)(mAUDIO_0_WAVESHAPER&0xff),mSystem.mCpu->GetPC());
         return (UBYTE)(mAUDIO_0_WAVESHAPER&0xff);
      case (AUD0TBACK&0xff):
         TRACE_MIKIE2("Peek(AUD0TBACK,%02x) at PC=%04x",(UBYTE)mAUDIO_0_BKUP,mSystem.mCpu->GetPC());
         return (UBYTE)mAUDIO_0_BKUP;
      case (AUD0CTL&0xff):
         retval|=(mAUDIO_0_INTEGRATE_ENABLE)?0x20:0x00;
         retval|=(mAUDIO_0_ENABLE_RELOAD)?0x10:0x00;
         retval|=(mAUDIO_0_ENABLE_COUNT)?0x08:0x00;
         retval|=(mAUDIO_0_WAVESHAPER&0x001000)?0x80:0x00;
         retval|=mAUDIO_0_LINKING;
         TRACE_MIKIE2("Peek(AUD0CTL,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (AUD0COUNT&0xff):
         TRACE_MIKIE2("Peek(AUD0COUNT,%02x) at PC=%04x",(UBYTE)mAUDIO_0_CURRENT,mSystem.mCpu->GetPC());
         return (UBYTE)mAUDIO_0_CURRENT;
      case (AUD0MISC&0xff):
         retval|=(mAUDIO_0_BORROW_OUT)?0x01:0x00;
         retval|=(mAUDIO_0_BORROW_IN)?0x02:0x00;
         retval|=(mAUDIO_0_LAST_CLOCK)?0x08:0x00;
         retval|=(mAUDIO_0_WAVESHAPER>>4)&0xf0;
         TRACE_MIKIE2("Peek(AUD0MISC,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (AUD1VOL&0xff):
         TRACE_MIKIE2("Peek(AUD1VOL,%02x) at PC=%04x",(UBYTE)mAUDIO_1_VOLUME,mSystem.mCpu->GetPC());
         return (UBYTE)mAUDIO_1_VOLUME;
      case (AUD1SHFTFB&0xff):
         TRACE_MIKIE2("Peek(AUD1SHFTFB,%02x) at PC=%04x",(UBYTE)(mAUDIO_1_WAVESHAPER>>13)&0xff,mSystem.mCpu->GetPC());
         return (UBYTE)((mAUDIO_1_WAVESHAPER>>13)&0xff);
      case (AUD1OUTVAL&0xff):
         TRACE_MIKIE2("Peek(AUD1OUTVAL,%02x) at PC=%04x",(UBYTE)mAUDIO_OUTPUT[1],mSystem.mCpu->GetPC());
         return (UBYTE)mAUDIO_OUTPUT[1];
      case (AUD1L8SHFT&0xff):
         TRACE_MIKIE2("Peek(AUD1L8SHFT,%02x) at PC=%04x",(UBYTE)(mAUDIO_1_WAVESHAPER&0xff),mSystem.mCpu->GetPC());
         return (UBYTE)(mAUDIO_1_WAVESHAPER&0xff);
      case (AUD1TBACK&0xff):
         TRACE_MIKIE2("Peek(AUD1TBACK,%02x) at PC=%04x",(UBYTE)mAUDIO_1_BKUP,mSystem.mCpu->GetPC());
         return (UBYTE)mAUDIO_1_BKUP;
      case (AUD1CTL&0xff):
         retval|=(mAUDIO_1_INTEGRATE_ENABLE)?0x20:0x00;
         retval|=(mAUDIO_1_ENABLE_RELOAD)?0x10:0x00;
         retval|=(mAUDIO_1_ENABLE_COUNT)?0x08:0x00;
         retval|=(mAUDIO_1_WAVESHAPER&0x001000)?0x80:0x00;
         retval|=mAUDIO_1_LINKING;
         TRACE_MIKIE2("Peek(AUD1CTL,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (AUD1COUNT&0xff):
         TRACE_MIKIE2("Peek(AUD1COUNT,%02x) at PC=%04x",(UBYTE)mAUDIO_1_CURRENT,mSystem.mCpu->GetPC());
         return (UBYTE)mAUDIO_1_CURRENT;
      case (AUD1MISC&0xff):
         retval|=(mAUDIO_1_BORROW_OUT)?0x01:0x00;
         retval|=(mAUDIO_1_BORROW_IN)?0x02:0x00;
         retval|=(mAUDIO_1_LAST_CLOCK)?0x08:0x00;
         retval|=(mAUDIO_1_WAVESHAPER>>4)&0xf0;
         TRACE_MIKIE2("Peek(AUD1MISC,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (AUD2VOL&0xff):
         TRACE_MIKIE2("Peek(AUD2VOL,%02x) at PC=%04x",(UBYTE)mAUDIO_2_VOLUME,mSystem.mCpu->GetPC());
         return (UBYTE)mAUDIO_2_VOLUME;
      case (AUD2SHFTFB&0xff):
         TRACE_MIKIE2("Peek(AUD2SHFTFB,%02x) at PC=%04x",(UBYTE)(mAUDIO_2_WAVESHAPER>>13)&0xff,mSystem.mCpu->GetPC());
         return (UBYTE)((mAUDIO_2_WAVESHAPER>>13)&0xff);
      case (AUD2OUTVAL&0xff):
         TRACE_MIKIE2("Peek(AUD2OUTVAL,%02x) at PC=%04x",(UBYTE)mAUDIO_OUTPUT[2],mSystem.mCpu->GetPC());
         return (UBYTE)mAUDIO_OUTPUT[2];
      case (AUD2L8SHFT&0xff):
         TRACE_MIKIE2("Peek(AUD2L8SHFT,%02x) at PC=%04x",(UBYTE)(mAUDIO_2_WAVESHAPER&0xff),mSystem.mCpu->GetPC());
         return (UBYTE)(mAUDIO_2_WAVESHAPER&0xff);
      case (AUD2TBACK&0xff):
         TRACE_MIKIE2("Peek(AUD2TBACK,%02x) at PC=%04x",(UBYTE)mAUDIO_2_BKUP,mSystem.mCpu->GetPC());
         return (UBYTE)mAUDIO_2_BKUP;
      case (AUD2CTL&0xff):
         retval|=(mAUDIO_2_INTEGRATE_ENABLE)?0x20:0x00;
         retval|=(mAUDIO_2_ENABLE_RELOAD)?0x10:0x00;
         retval|=(mAUDIO_2_ENABLE_COUNT)?0x08:0x00;
         retval|=(mAUDIO_2_WAVESHAPER&0x001000)?0x80:0x00;
         retval|=mAUDIO_2_LINKING;
         TRACE_MIKIE2("Peek(AUD2CTL,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (AUD2COUNT&0xff):
         TRACE_MIKIE2("Peek(AUD2COUNT,%02x) at PC=%04x",(UBYTE)mAUDIO_2_CURRENT,mSystem.mCpu->GetPC());
         return (UBYTE)mAUDIO_2_CURRENT;
      case (AUD2MISC&0xff):
         retval|=(mAUDIO_2_BORROW_OUT)?0x01:0x00;
         retval|=(mAUDIO_2_BORROW_IN)?0x02:0x00;
         retval|=(mAUDIO_2_LAST_CLOCK)?0x08:0x00;
         retval|=(mAUDIO_2_WAVESHAPER>>4)&0xf0;
         TRACE_MIKIE2("Peek(AUD2MISC,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (AUD3VOL&0xff):
         TRACE_MIKIE2("Peek(AUD3VOL,%02x) at PC=%04x",(UBYTE)mAUDIO_3_VOLUME,mSystem.mCpu->GetPC());
         return (UBYTE)mAUDIO_3_VOLUME;
      case (AUD3SHFTFB&0xff):
         TRACE_MIKIE2("Peek(AUD3SHFTFB,%02x) at PC=%04x",(UBYTE)(mAUDIO_3_WAVESHAPER>>13)&0xff,mSystem.mCpu->GetPC());
         return (UBYTE)((mAUDIO_3_WAVESHAPER>>13)&0xff);
      case (AUD3OUTVAL&0xff):
         TRACE_MIKIE2("Peek(AUD3OUTVAL,%02x) at PC=%04x",(UBYTE)mAUDIO_OUTPUT[3],mSystem.mCpu->GetPC());
         return (UBYTE)mAUDIO_OUTPUT[3];
      case (AUD3L8SHFT&0xff):
         TRACE_MIKIE2("Peek(AUD3L8SHFT,%02x) at PC=%04x",(UBYTE)(mAUDIO_3_WAVESHAPER&0xff),mSystem.mCpu->GetPC());
         return (UBYTE)(mAUDIO_3_WAVESHAPER&0xff);
      case (AUD3TBACK&0xff):
         TRACE_MIKIE2("Peek(AUD3TBACK,%02x) at PC=%04x",(UBYTE)mAUDIO_3_BKUP,mSystem.mCpu->GetPC());
         return (UBYTE)mAUDIO_3_BKUP;
      case (AUD3CTL&0xff):
         retval|=(mAUDIO_3_INTEGRATE_ENABLE)?0x20:0x00;
         retval|=(mAUDIO_3_ENABLE_RELOAD)?0x10:0x00;
         retval|=(mAUDIO_3_ENABLE_COUNT)?0x08:0x00;
         retval|=(mAUDIO_3_WAVESHAPER&0x001000)?0x80:0x00;
         retval|=mAUDIO_3_LINKING;
         TRACE_MIKIE2("Peek(AUD3CTL,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return retval;
      case (AUD3COUNT&0xff):
         TRACE_MIKIE2("Peek(AUD3COUNT,%02x) at PC=%04x",(UBYTE)mAUDIO_3_CURRENT,mSystem.mCpu->GetPC());
         return (UBYTE)mAUDIO_3_CURRENT;
      case (AUD3MISC&0xff):
         retval|=(mAUDIO_3_BORROW_OUT)?0x01:0x00;
         retval|=(mAUDIO_3_BORROW_IN)?0x02:0x00;
         retval|=(mAUDIO_3_LAST_CLOCK)?0x08:0x00;
         retval|=(mAUDIO_3_WAVESHAPER>>4)&0xf0;
         TRACE_MIKIE2("Peek(AUD3MISC,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return retval;

      case (ATTEN_A&0xff):
         TRACE_MIKIE1("Peek(ATTEN_A) at PC=%04x",mSystem.mCpu->GetPC());
         return (UBYTE) mAUDIO_ATTEN[0];
      case (ATTEN_B&0xff):
         TRACE_MIKIE1("Peek(ATTEN_B) at PC=%04x",mSystem.mCpu->GetPC());
         return (UBYTE) mAUDIO_ATTEN[1];
      case (ATTEN_C&0xff):
         TRACE_MIKIE1("Peek(ATTEN_C) at PC=%04x",mSystem.mCpu->GetPC());
         return (UBYTE) mAUDIO_ATTEN[2];
      case (ATTEN_D&0xff):
         TRACE_MIKIE1("Peek(ATTEN_D) at PC=%04x",mSystem.mCpu->GetPC());
         return (UBYTE) mAUDIO_ATTEN[3];
      case (MPAN&0xff):
         TRACE_MIKIE1("Peek(MPAN) at PC=%04x",mSystem.mCpu->GetPC());
         return (UBYTE) mPAN;

      case (MSTEREO&0xff):
         TRACE_MIKIE2("Peek(MSTEREO,%02x) at PC=%04x",(UBYTE)mSTEREO^0xff,mSystem.mCpu->GetPC());
         return (UBYTE) mSTEREO;

         // Miscellaneous registers

      case (SERCTL&0xff):
         retval|=(mUART_TX_COUNTDOWN&UART_TX_INACTIVE)?0xA0:0x00;	// Indicate TxDone & TxAllDone
         retval|=(mUART_RX_READY)?0x40:0x00;							// Indicate Rx data ready
         retval|=(mUART_Rx_overun_error)?0x08:0x0;					// Framing error
         retval|=(mUART_Rx_framing_error)?0x04:0x00;					// Rx overrun
         retval|=(mUART_RX_DATA&UART_BREAK_CODE)?0x02:0x00;			// Indicate break received
         retval|=(mUART_RX_DATA&0x0100)?0x01:0x00;					// Add parity bit
         TRACE_MIKIE2("Peek(SERCTL  ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return (UBYTE)retval;

      case (SERDAT&0xff):
         mUART_RX_READY=0;
         TRACE_MIKIE2("Peek(SERDAT  ,%02x) at PC=%04x",(UBYTE)mUART_RX_DATA,mSystem.mCpu->GetPC());
         return (UBYTE)(mUART_RX_DATA&0xff);

      case (IODAT&0xff):
         // IODIR  = output bit : input high (eeprom write done)
         if(mSystem.mEEPROM->Available()) {
            mSystem.mEEPROM->ProcessEepromBusy();
            retval|=(mIODIR&0x10)?mIODAT&0x10:(mSystem.mEEPROM->OutputBit()?0x10:0x00);
         } else {
            retval|=mIODAT&0x10;
         }
         retval|=(mIODIR&0x08)?(((mIODAT&0x08)&&mIODAT_REST_SIGNAL)?0x00:0x08):0x00;									// REST   = output bit : input low
         retval|=(mIODIR&0x04)?mIODAT&0x04:((mUART_CABLE_PRESENT)?0x04:0x00);	// NOEXP  = output bit : input low
         retval|=(mIODIR&0x02)?mIODAT&0x02:0x00;									// CARTAD = output bit : input low
         retval|=(mIODIR&0x01)?mIODAT&0x01:0x01;									// EXTPW  = output bit : input high (Power connected)
         TRACE_MIKIE2("Peek(IODAT   ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
         return (UBYTE)retval;

      case (INTRST&0xff):
      case (INTSET&0xff):
         TRACE_MIKIE2("Peek(INTSET  ,%02x) at PC=%04x",mTimerStatusFlags,mSystem.mCpu->GetPC());
         return (UBYTE)mTimerStatusFlags;
      case (MAGRDY0&0xff):
      case (MAGRDY1&0xff):
         TRACE_MIKIE2("Peek(MAGRDY0/1,%02x) at PC=%04x",0x00,mSystem.mCpu->GetPC());
         return 0x00;
      case (AUDIN&0xff):
         //			TRACE_MIKIE2("Peek(AUDIN,%02x) at PC=%04x",mAudioInputComparator?0x80:0x00,mSystem.mCpu->GetPC());
         //			if(mAudioInputComparator) return 0x80; else return 0x00;
         TRACE_MIKIE2("Peek(AUDIN,%02x) at PC=%04x",0x80,mSystem.mCpu->GetPC());
         return 0x80;
      case (MIKEYHREV&0xff):
         TRACE_MIKIE2("Peek(MIKEYHREV,%02x) at PC=%04x",0x01,mSystem.mCpu->GetPC());
         return 0x01;

      // Pallette registers

      case (GREEN0&0xff):
      case (GREEN1&0xff):
      case (GREEN2&0xff):
      case (GREEN3&0xff):
      case (GREEN4&0xff):
      case (GREEN5&0xff):
      case (GREEN6&0xff):
      case (GREEN7&0xff):
      case (GREEN8&0xff):
      case (GREEN9&0xff):
      case (GREENA&0xff):
      case (GREENB&0xff):
      case (GREENC&0xff):
      case (GREEND&0xff):
      case (GREENE&0xff):
      case (GREENF&0xff):
         TRACE_MIKIE2("Peek(GREENPAL0-F,%02x) at PC=%04x",mPalette[addr&0x0f].Colours.Green,mSystem.mCpu->GetPC());
         return mPalette[addr&0x0f].Colours.Green;
      case (BLUERED0&0xff):
      case (BLUERED1&0xff):
      case (BLUERED2&0xff):
      case (BLUERED3&0xff):
      case (BLUERED4&0xff):
      case (BLUERED5&0xff):
      case (BLUERED6&0xff):
      case (BLUERED7&0xff):
      case (BLUERED8&0xff):
      case (BLUERED9&0xff):
      case (BLUEREDA&0xff):
      case (BLUEREDB&0xff):
      case (BLUEREDC&0xff):
      case (BLUEREDD&0xff):
      case (BLUEREDE&0xff):
      case (BLUEREDF&0xff):
         TRACE_MIKIE2("Peek(BLUEREDPAL0-F,%02x) at PC=%04x",(mPalette[addr&0x0f].Colours.Red | (mPalette[addr&0x0f].Colours.Blue<<4)),mSystem.mCpu->GetPC());
         return (mPalette[addr&0x0f].Colours.Red | (mPalette[addr&0x0f].Colours.Blue<<4));

      // Errors on write only register accesses
      // For easier debugging

      case (DISPADRL&0xff):
         TRACE_MIKIE2("Peek(DISPADRL,%02x) at PC=%04x",(UBYTE)(mDisplayAddress&0xff),mSystem.mCpu->GetPC());
         return (UBYTE)(mDisplayAddress&0xff);
      case (DISPADRH&0xff):
         TRACE_MIKIE2("Peek(DISPADRH,%02x) at PC=%04x",(UBYTE)(mDisplayAddress>>8)&0xff,mSystem.mCpu->GetPC());
         return (UBYTE)(mDisplayAddress>>8)&0xff;

      case (DISPCTL&0xff):
      case (SYSCTL1&0xff):
      case (MIKEYSREV&0xff):
      case (IODIR&0xff):
      case (SDONEACK&0xff):
      case (CPUSLEEP&0xff):
      case (PBKUP&0xff):
      case (Mtest0&0xff):
      case (Mtest1&0xff):
      case (Mtest2&0xff):
         TRACE_MIKIE2("Peek(%04x) - Peek from write only register location at PC=$%04x",addr,mSystem.mCpu->GetPC());
         break;

      // Register to let programs know handy is running

      case (0xfd97&0xff):
         TRACE_MIKIE2("Peek(%04x) - **** HANDY DETECT ATTEMPTED **** at PC=$%04x",addr,mSystem.mCpu->GetPC());
         break;
         //return 0x42;
         // Errors on illegal location accesses

      default:
         TRACE_MIKIE2("Peek(%04x) - Peek from illegal location at PC=$%04x",addr,mSystem.mCpu->GetPC());
         break;
   }
   return 0xff;
}

inline void CMikie::Update(void)
{
   SLONG divide = 0;
   SLONG decval = 0;
   ULONG tmp = 0;
   ULONG mikie_work_done=0;

   //
   // To stop problems with cycle count wrap we will check and then correct the
   // cycle counter.
   //

   //			TRACE_MIKIE0("Update()");

   if(gSystemCycleCount>0xf0000000) {
      gSystemCycleCount-=0x80000000;
      gThrottleNextCycleCheckpoint-=0x80000000;
      gAudioLastUpdateCycle-=0x80000000;
      mTIM_0_LAST_COUNT-=0x80000000;
      mTIM_1_LAST_COUNT-=0x80000000;
      mTIM_2_LAST_COUNT-=0x80000000;
      mTIM_3_LAST_COUNT-=0x80000000;
      mTIM_4_LAST_COUNT-=0x80000000;
      mTIM_5_LAST_COUNT-=0x80000000;
      mTIM_6_LAST_COUNT-=0x80000000;
      mTIM_7_LAST_COUNT-=0x80000000;
      mAUDIO_0_LAST_COUNT-=0x80000000;
      mAUDIO_1_LAST_COUNT-=0x80000000;
      mAUDIO_2_LAST_COUNT-=0x80000000;
      mAUDIO_3_LAST_COUNT-=0x80000000;
      // Only correct if sleep is active
      if(gCPUWakeupTime) {
         gCPUWakeupTime-=0x80000000;
         gIRQEntryCycle-=0x80000000;
      }
   }

   gNextTimerEvent=0xffffffff;

   //
   // Check if the CPU needs to be woken up from sleep mode
   //
   if(gCPUWakeupTime) {
      if(gSystemCycleCount>=gCPUWakeupTime) {
         TRACE_MIKIE0("*********************************************************");
         TRACE_MIKIE0("****              CPU SLEEP COMPLETED                ****");
         TRACE_MIKIE0("*********************************************************");
         gSystemCPUSleep=FALSE;
         gSystemCPUSleep_Saved=FALSE;
         gCPUWakeupTime=0;
      } else {
         if(gCPUWakeupTime>gSystemCycleCount) gNextTimerEvent=gCPUWakeupTime;
      }
   }

   //	Timer updates, rolled out flat in group order
   //
   //	Group A:
   //	Timer 0 -> Timer 2 -> Timer 4.
   //
   //	Group B:
   //	Timer 1 -> Timer 3 -> Timer 5 -> Timer 7 -> Audio 0 -> Audio 1-> Audio 2 -> Audio 3 -> Timer 1.
   //

   //
   // Within each timer code block we will predict the cycle count number of
   // the next timer event
   //
   // We don't need to count linked timers as the timer they are linked
   // from will always generate earlier events.
   //
   // As Timer 4 (UART) will generate many events we will ignore it
   //
   // We set the next event to the end of time at first and let the timers
   // overload it. Any writes to timer controls will force next event to
   // be immediate and hence a new prediction will be done. The prediction
   // causes overflow as opposed to zero i.e. current+1
   // (In reality T0 line counter should always be running.)
   //


   //
   // Timer 0 of Group A
   //

   //
   // Optimisation, assume T0 (Line timer) is never in one-shot,
   // never placed in link mode
   //

   // KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..)
   //			if(mTIM_0_ENABLE_COUNT && (mTIM_0_ENABLE_RELOAD || !mTIM_0_TIMER_DONE))
   if(mTIM_0_ENABLE_COUNT) {
      // Timer 0 has no linking
      //				if(mTIM_0_LINKING!=0x07)
      {
         // Ordinary clocked mode as opposed to linked mode
         // 16MHz clock down to 1us == cyclecount >> 4
         divide=(4+mTIM_0_LINKING);
         decval=(gSystemCycleCount-mTIM_0_LAST_COUNT)>>divide;

         if(decval) {
            mTIM_0_LAST_COUNT+=decval<<divide;
            mTIM_0_CURRENT-=decval;

            if(mTIM_0_CURRENT&0x80000000) {
               // Set carry out
               mTIM_0_BORROW_OUT=TRUE;

               //							// Reload if necessary
               //							if(mTIM_0_ENABLE_RELOAD)
               //							{
               mTIM_0_CURRENT+=mTIM_0_BKUP+1;
               //							}
               //							else
               //							{
               //								mTIM_0_CURRENT=0;
               //							}

               mTIM_0_TIMER_DONE=TRUE;

               // Interupt flag setting code moved into DisplayRenderLine()

               // Line timer has expired, render a line, we cannot increment
               // the global counter at this point as it will screw the other timers
               // so we save under work done and inc at the end.
               mikie_work_done+=DisplayRenderLine();

            } else {
               mTIM_0_BORROW_OUT=FALSE;
            }
            // Set carry in as we did a count
            mTIM_0_BORROW_IN=TRUE;
         } else {
            // Clear carry in as we didn't count
            mTIM_0_BORROW_IN=FALSE;
            // Clear carry out
            mTIM_0_BORROW_OUT=FALSE;
         }
      }

      // Prediction for next timer event cycle number

      //				if(mTIM_0_LINKING!=7)
      {
         // Sometimes timeupdates can be >2x rollover in which case
         // then CURRENT may still be negative and we can use it to
         // calc the next timer value, we just want another update ASAP
         tmp=(mTIM_0_CURRENT&0x80000000)?1:((mTIM_0_CURRENT+1)<<divide);
         tmp+=gSystemCycleCount;
         if(tmp<gNextTimerEvent) {
            gNextTimerEvent=tmp;
            //						TRACE_MIKIE1("Update() - TIMER 0 Set NextTimerEvent = %012d",gNextTimerEvent);
         }
      }
      //				TRACE_MIKIE1("Update() - mTIM_0_CURRENT = %012d",mTIM_0_CURRENT);
      //				TRACE_MIKIE1("Update() - mTIM_0_BKUP    = %012d",mTIM_0_BKUP);
      //				TRACE_MIKIE1("Update() - mTIM_0_LASTCNT = %012d",mTIM_0_LAST_COUNT);
      //				TRACE_MIKIE1("Update() - mTIM_0_LINKING = %012d",mTIM_0_LINKING);
   }

   //
   // Timer 2 of Group A
   //

   //
   // Optimisation, assume T2 (Frame timer) is never in one-shot
   // always in linked mode i.e clocked by Line Timer
   //

   // KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..)
   //			if(mTIM_2_ENABLE_COUNT && (mTIM_2_ENABLE_RELOAD || !mTIM_2_TIMER_DONE))
   if(mTIM_2_ENABLE_COUNT) {
      decval=0;

      //				if(mTIM_2_LINKING==0x07)
      {
         if(mTIM_0_BORROW_OUT) decval=1;
         mTIM_2_LAST_LINK_CARRY=mTIM_0_BORROW_OUT;
      }
      //				else
      //				{
      //					// Ordinary clocked mode as opposed to linked mode
      //					// 16MHz clock down to 1us == cyclecount >> 4
      //					divide=(4+mTIM_2_LINKING);
      //					decval=(gSystemCycleCount-mTIM_2_LAST_COUNT)>>divide;
      //				}

      if(decval) {
         //					mTIM_2_LAST_COUNT+=decval<<divide;
         mTIM_2_CURRENT-=decval;
         if(mTIM_2_CURRENT&0x80000000) {
            // Set carry out
            mTIM_2_BORROW_OUT=TRUE;

            //						// Reload if necessary
            //						if(mTIM_2_ENABLE_RELOAD)
            //						{
            mTIM_2_CURRENT+=mTIM_2_BKUP+1;
            //						}
            //						else
            //						{
            //							mTIM_2_CURRENT=0;
            //						}
            mTIM_2_TIMER_DONE=TRUE;

            mLynxLineDMACounter=0;
            mLynxLine=mTIM_2_BKUP;

            if(gCPUWakeupTime) {
               gCPUWakeupTime = 0;
               gSystemCPUSleep=FALSE;
               gSystemCPUSleep_Saved=FALSE;
            }

            // Set the timer status flag
            if(mTimerInterruptMask&0x04) {
               TRACE_MIKIE0("Update() - TIMER2 IRQ Triggered (Frame Timer)");
               mTimerStatusFlags|=0x04;
               gSystemIRQ=TRUE;	// Added 19/09/06 fix for IRQ issue
            }

            TRACE_MIKIE0("Update() - Frame end");

            ResetDisplayPtr();

            gEndOfFrame = TRUE;

         } else {
            mTIM_2_BORROW_OUT=FALSE;
         }
         // Set carry in as we did a count
         mTIM_2_BORROW_IN=TRUE;
      } else {
         // Clear carry in as we didn't count
         mTIM_2_BORROW_IN=FALSE;
         // Clear carry out
         mTIM_2_BORROW_OUT=FALSE;
      }

      // Prediction for next timer event cycle number
      // We dont need to predict this as its the frame timer and will always
      // be beaten by the line timer on Timer 0
      //				if(mTIM_2_LINKING!=7)
      //				{
      //					tmp=gSystemCycleCount+((mTIM_2_CURRENT+1)<<divide);
      //					if(tmp<gNextTimerEvent)	gNextTimerEvent=tmp;
      //				}
      //				TRACE_MIKIE1("Update() - mTIM_2_CURRENT = %012d",mTIM_2_CURRENT);
      //				TRACE_MIKIE1("Update() - mTIM_2_BKUP    = %012d",mTIM_2_BKUP);
      //				TRACE_MIKIE1("Update() - mTIM_2_LASTCNT = %012d",mTIM_2_LAST_COUNT);
      //				TRACE_MIKIE1("Update() - mTIM_2_LINKING = %012d",mTIM_2_LINKING);
   }

   //
   // Timer 4 of Group A
   //
   // For the sake of speed it is assumed that Timer 4 (UART timer)
   // never uses one-shot mode, never uses linking, hence the code
   // is commented out. Timer 4 is at the end of a chain and seems
   // no reason to update its carry in-out variables
   //

   // KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..)
   //			if(mTIM_4_ENABLE_COUNT && (mTIM_4_ENABLE_RELOAD || !mTIM_4_TIMER_DONE))
   if(mTIM_4_ENABLE_COUNT) {
      decval=0;

      //				if(mTIM_4_LINKING==0x07)
      //				{
      ////				if(mTIM_2_BORROW_OUT && !mTIM_4_LAST_LINK_CARRY) decval=1;
      //					if(mTIM_2_BORROW_OUT) decval=1;
      //					mTIM_4_LAST_LINK_CARRY=mTIM_2_BORROW_OUT;
      //				}
      //				else
      {
         // Ordinary clocked mode as opposed to linked mode
         // 16MHz clock down to 1us == cyclecount >> 4
         // Additional /8 (+3) for 8 clocks per bit transmit
         divide=4+3+mTIM_4_LINKING;
         decval=(gSystemCycleCount-mTIM_4_LAST_COUNT)>>divide;
      }

      if(decval) {
         mTIM_4_LAST_COUNT+=decval<<divide;
         mTIM_4_CURRENT-=decval;
         if(mTIM_4_CURRENT&0x80000000) {
            // Set carry out
            mTIM_4_BORROW_OUT=TRUE;

            //
            // Update the UART counter models for Rx & Tx
            //

            //
            // According to the docs IRQ's are level triggered and hence will always assert
            // what a pain in the arse
            //
            // Rx & Tx are loopedback due to comlynx structure

            //
            // Receive
            //
            if(!mUART_RX_COUNTDOWN) {
               // Fetch a byte from the input queue
               if(mUART_Rx_waiting>0) {
                  mUART_RX_DATA = mUART_Rx_input_queue[mUART_Rx_output_ptr++];
                  mUART_Rx_output_ptr %= UART_MAX_RX_QUEUE;
                  mUART_Rx_waiting--;
                  TRACE_MIKIE2("Update() - RX Byte output ptr=%02d waiting=%02d",mUART_Rx_output_ptr,mUART_Rx_waiting);
               } else {
                  TRACE_MIKIE0("Update() - RX Byte but no data waiting ????");
               }

               // Retrigger input if more bytes waiting
               if(mUART_Rx_waiting>0) {
                  mUART_RX_COUNTDOWN=UART_RX_TIME_PERIOD+UART_RX_NEXT_DELAY;
                  TRACE_MIKIE1("Update() - RX Byte retriggered, %d waiting",mUART_Rx_waiting);
               } else {
                  mUART_RX_COUNTDOWN=UART_RX_INACTIVE;
                  TRACE_MIKIE0("Update() - RX Byte nothing waiting, deactivated");
               }

               // If RX_READY already set then we have an overrun
               // as previous byte hasn't been read
               if(mUART_RX_READY) mUART_Rx_overun_error=1;

               // Flag byte as being recvd
               mUART_RX_READY=1;
            } else if(!(mUART_RX_COUNTDOWN&UART_RX_INACTIVE)) {
               mUART_RX_COUNTDOWN--;
            }

            if(!mUART_TX_COUNTDOWN) {
               if(mUART_SENDBREAK) {
                  mUART_TX_DATA=UART_BREAK_CODE;
                  // Auto-Respawn new transmit
                  mUART_TX_COUNTDOWN=UART_TX_TIME_PERIOD;
                  // Loop back what we transmitted
                  ComLynxTxLoopback(mUART_TX_DATA);
               } else {
                  // Serial activity finished
                  mUART_TX_COUNTDOWN=UART_TX_INACTIVE;
               }

               // If a networking object is attached then use its callback to send the data byte.
               if(mpUART_TX_CALLBACK) {
                  TRACE_MIKIE0("Update() - UART_TX_CALLBACK");
                  (*mpUART_TX_CALLBACK)(mUART_TX_DATA,mUART_TX_CALLBACK_OBJECT);
               }

            } else if(!(mUART_TX_COUNTDOWN&UART_TX_INACTIVE)) {
               mUART_TX_COUNTDOWN--;
            }

            // Set the timer status flag
            // Timer 4 is the uart timer and doesn't generate IRQ's using this method

            // 16 Clocks = 1 bit transmission. Hold separate Rx & Tx counters

            // Reload if necessary
            //						if(mTIM_4_ENABLE_RELOAD)
            //						{
            mTIM_4_CURRENT+=mTIM_4_BKUP+1;
            // The low reload values on TIM4 coupled with a longer
            // timer service delay can sometimes cause
            // an underrun, check and fix
            if(mTIM_4_CURRENT&0x80000000) {
               mTIM_4_CURRENT=mTIM_4_BKUP;
               mTIM_4_LAST_COUNT=gSystemCycleCount;
            }
            //						}
            //						else
            //						{
            //							mTIM_4_CURRENT=0;
            //						}
            //						mTIM_4_TIMER_DONE=TRUE;
         }
         //					else
         //					{
         //						mTIM_4_BORROW_OUT=FALSE;
         //					}
         //					// Set carry in as we did a count
         //					mTIM_4_BORROW_IN=TRUE;
      }
      //				else
      //				{
      //					// Clear carry in as we didn't count
      //					mTIM_4_BORROW_IN=FALSE;
      //					// Clear carry out
      //					mTIM_4_BORROW_OUT=FALSE;
      //				}
      //
      //				// Prediction for next timer event cycle number
      //
      //				if(mTIM_4_LINKING!=7)
      //				{
      // Sometimes timeupdates can be >2x rollover in which case
      // then CURRENT may still be negative and we can use it to
      // calc the next timer value, we just want another update ASAP
      tmp=(mTIM_4_CURRENT&0x80000000)?1:((mTIM_4_CURRENT+1)<<divide);
      tmp+=gSystemCycleCount;
      if(tmp<gNextTimerEvent) {
         gNextTimerEvent=tmp;
         TRACE_MIKIE1("Update() - TIMER 4 Set NextTimerEvent = %012d",gNextTimerEvent);
      }
      //				}
      //				TRACE_MIKIE1("Update() - mTIM_4_CURRENT = %012d",mTIM_4_CURRENT);
      //				TRACE_MIKIE1("Update() - mTIM_4_BKUP    = %012d",mTIM_4_BKUP);
      //				TRACE_MIKIE1("Update() - mTIM_4_LASTCNT = %012d",mTIM_4_LAST_COUNT);
      //				TRACE_MIKIE1("Update() - mTIM_4_LINKING = %012d",mTIM_4_LINKING);
   }

   // Emulate the UART bug where UART IRQ is level sensitive
   // in that it will continue to generate interrupts as long
   // as they are enabled and the interrupt condition is true

   // If Tx is inactive i.e ready for a byte to eat and the
   // IRQ is enabled then generate it always
   if((mUART_TX_COUNTDOWN&UART_TX_INACTIVE) && mUART_TX_IRQ_ENABLE) {
      TRACE_MIKIE0("Update() - UART TX IRQ Triggered");
      mTimerStatusFlags|=0x10;
      gSystemIRQ=TRUE;	// Added 19/09/06 fix for IRQ issue
   }
   // Is data waiting and the interrupt enabled, if so then
   // what are we waiting for....
   if(mUART_RX_READY && mUART_RX_IRQ_ENABLE) {
      TRACE_MIKIE0("Update() - UART RX IRQ Triggered");
      mTimerStatusFlags|=0x10;
      gSystemIRQ=TRUE;	// Added 19/09/06 fix for IRQ issue
   }

   //
   // Timer 1 of Group B
   //
   // KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..)
   if(mTIM_1_ENABLE_COUNT && (mTIM_1_ENABLE_RELOAD || !mTIM_1_TIMER_DONE)) {
      if(mTIM_1_LINKING!=0x07) {
         // Ordinary clocked mode as opposed to linked mode
         // 16MHz clock down to 1us == cyclecount >> 4
         divide=(4+mTIM_1_LINKING);
         decval=(gSystemCycleCount-mTIM_1_LAST_COUNT)>>divide;

         if(decval) {
            mTIM_1_LAST_COUNT+=decval<<divide;
            mTIM_1_CURRENT-=decval;
            if(mTIM_1_CURRENT&0x80000000) {
               // Set carry out
               mTIM_1_BORROW_OUT=TRUE;

               // Set the timer status flag
               if(mTimerInterruptMask&0x02) {
                  TRACE_MIKIE0("Update() - TIMER1 IRQ Triggered");
                  mTimerStatusFlags|=0x02;
                  gSystemIRQ=TRUE;	// Added 19/09/06 fix for IRQ issue
               }

               // Reload if necessary
               if(mTIM_1_ENABLE_RELOAD) {
                  mTIM_1_CURRENT+=mTIM_1_BKUP+1;
               } else {
                  mTIM_1_CURRENT=0;
               }
               mTIM_1_TIMER_DONE=TRUE;
            } else {
               mTIM_1_BORROW_OUT=FALSE;
            }
            // Set carry in as we did a count
            mTIM_1_BORROW_IN=TRUE;
         } else {
            // Clear carry in as we didn't count
            mTIM_1_BORROW_IN=FALSE;
            // Clear carry out
            mTIM_1_BORROW_OUT=FALSE;
         }
      }

      // Prediction for next timer event cycle number

      if(mTIM_1_LINKING!=7) {
         // Sometimes timeupdates can be >2x rollover in which case
         // then CURRENT may still be negative and we can use it to
         // calc the next timer value, we just want another update ASAP
         tmp=(mTIM_1_CURRENT&0x80000000)?1:((mTIM_1_CURRENT+1)<<divide);
         tmp+=gSystemCycleCount;
         if(tmp<gNextTimerEvent) {
            gNextTimerEvent=tmp;
            TRACE_MIKIE1("Update() - TIMER 1 Set NextTimerEvent = %012d",gNextTimerEvent);
         }
      }
      //				TRACE_MIKIE1("Update() - mTIM_1_CURRENT = %012d",mTIM_1_CURRENT);
      //				TRACE_MIKIE1("Update() - mTIM_1_BKUP    = %012d",mTIM_1_BKUP);
      //				TRACE_MIKIE1("Update() - mTIM_1_LASTCNT = %012d",mTIM_1_LAST_COUNT);
      //				TRACE_MIKIE1("Update() - mTIM_1_LINKING = %012d",mTIM_1_LINKING);
   }

   //
   // Timer 3 of Group A
   //
   // KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..)
   if(mTIM_3_ENABLE_COUNT && (mTIM_3_ENABLE_RELOAD || !mTIM_3_TIMER_DONE)) {
      decval=0;

      if(mTIM_3_LINKING==0x07) {
         if(mTIM_1_BORROW_OUT) decval=1;
            mTIM_3_LAST_LINK_CARRY=mTIM_1_BORROW_OUT;
      } else {
         // Ordinary clocked mode as opposed to linked mode
         // 16MHz clock down to 1us == cyclecount >> 4
         divide=(4+mTIM_3_LINKING);
         decval=(gSystemCycleCount-mTIM_3_LAST_COUNT)>>divide;
      }

      if(decval) {
         mTIM_3_LAST_COUNT+=decval<<divide;
         mTIM_3_CURRENT-=decval;
         if(mTIM_3_CURRENT&0x80000000) {
            // Set carry out
            mTIM_3_BORROW_OUT=TRUE;

            // Set the timer status flag
            if(mTimerInterruptMask&0x08) {
               TRACE_MIKIE0("Update() - TIMER3 IRQ Triggered");
               mTimerStatusFlags|=0x08;
               gSystemIRQ=TRUE;	// Added 19/09/06 fix for IRQ issue
            }

            // Reload if necessary
            if(mTIM_3_ENABLE_RELOAD) {
               mTIM_3_CURRENT+=mTIM_3_BKUP+1;
            } else {
               mTIM_3_CURRENT=0;
            }
            mTIM_3_TIMER_DONE=TRUE;
         } else {
            mTIM_3_BORROW_OUT=FALSE;
         }
         // Set carry in as we did a count
         mTIM_3_BORROW_IN=TRUE;
      } else {
         // Clear carry in as we didn't count
         mTIM_3_BORROW_IN=FALSE;
         // Clear carry out
         mTIM_3_BORROW_OUT=FALSE;
      }

      // Prediction for next timer event cycle number

      if(mTIM_3_LINKING!=7) {
         // Sometimes timeupdates can be >2x rollover in which case
         // then CURRENT may still be negative and we can use it to
         // calc the next timer value, we just want another update ASAP
         tmp=(mTIM_3_CURRENT&0x80000000)?1:((mTIM_3_CURRENT+1)<<divide);
         tmp+=gSystemCycleCount;
         if(tmp<gNextTimerEvent) {
            gNextTimerEvent=tmp;
            TRACE_MIKIE1("Update() - TIMER 3 Set NextTimerEvent = %012d",gNextTimerEvent);
         }
      }
      //				TRACE_MIKIE1("Update() - mTIM_3_CURRENT = %012d",mTIM_3_CURRENT);
      //				TRACE_MIKIE1("Update() - mTIM_3_BKUP    = %012d",mTIM_3_BKUP);
      //				TRACE_MIKIE1("Update() - mTIM_3_LASTCNT = %012d",mTIM_3_LAST_COUNT);
      //				TRACE_MIKIE1("Update() - mTIM_3_LINKING = %012d",mTIM_3_LINKING);
   }

   //
   // Timer 5 of Group A
   //
   // KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..)
   if(mTIM_5_ENABLE_COUNT && (mTIM_5_ENABLE_RELOAD || !mTIM_5_TIMER_DONE)) {
      decval=0;

      if(mTIM_5_LINKING==0x07) {
         if(mTIM_3_BORROW_OUT) decval=1;
            mTIM_5_LAST_LINK_CARRY=mTIM_3_BORROW_OUT;
      } else {
         // Ordinary clocked mode as opposed to linked mode
         // 16MHz clock down to 1us == cyclecount >> 4
         divide=(4+mTIM_5_LINKING);
         decval=(gSystemCycleCount-mTIM_5_LAST_COUNT)>>divide;
      }

      if(decval) {
         mTIM_5_LAST_COUNT+=decval<<divide;
         mTIM_5_CURRENT-=decval;
         if(mTIM_5_CURRENT&0x80000000) {
            // Set carry out
            mTIM_5_BORROW_OUT=TRUE;

            // Set the timer status flag
            if(mTimerInterruptMask&0x20) {
               TRACE_MIKIE0("Update() - TIMER5 IRQ Triggered");
               mTimerStatusFlags|=0x20;
               gSystemIRQ=TRUE;	// Added 19/09/06 fix for IRQ issue
            }

            // Reload if necessary
            if(mTIM_5_ENABLE_RELOAD) {
               mTIM_5_CURRENT+=mTIM_5_BKUP+1;
            } else {
               mTIM_5_CURRENT=0;
            }
            mTIM_5_TIMER_DONE=TRUE;
         } else {
            mTIM_5_BORROW_OUT=FALSE;
         }
         // Set carry in as we did a count
         mTIM_5_BORROW_IN=TRUE;
      } else {
         // Clear carry in as we didn't count
         mTIM_5_BORROW_IN=FALSE;
         // Clear carry out
         mTIM_5_BORROW_OUT=FALSE;
      }

      // Prediction for next timer event cycle number

      if(mTIM_5_LINKING!=7) {
         // Sometimes timeupdates can be >2x rollover in which case
         // then CURRENT may still be negative and we can use it to
         // calc the next timer value, we just want another update ASAP
         tmp=(mTIM_5_CURRENT&0x80000000)?1:((mTIM_5_CURRENT+1)<<divide);
         tmp+=gSystemCycleCount;
         if(tmp<gNextTimerEvent) {
            gNextTimerEvent=tmp;
            TRACE_MIKIE1("Update() - TIMER 5 Set NextTimerEvent = %012d",gNextTimerEvent);
         }
      }
      //				TRACE_MIKIE1("Update() - mTIM_5_CURRENT = %012d",mTIM_5_CURRENT);
      //				TRACE_MIKIE1("Update() - mTIM_5_BKUP    = %012d",mTIM_5_BKUP);
      //				TRACE_MIKIE1("Update() - mTIM_5_LASTCNT = %012d",mTIM_5_LAST_COUNT);
      //				TRACE_MIKIE1("Update() - mTIM_5_LINKING = %012d",mTIM_5_LINKING);
   }

   //
   // Timer 7 of Group A
   //
   // KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..)
   if(mTIM_7_ENABLE_COUNT && (mTIM_7_ENABLE_RELOAD || !mTIM_7_TIMER_DONE)) {
      decval=0;

      if(mTIM_7_LINKING==0x07) {
         if(mTIM_5_BORROW_OUT) decval=1;
            mTIM_7_LAST_LINK_CARRY=mTIM_5_BORROW_OUT;
      } else {
          // Ordinary clocked mode as opposed to linked mode
          // 16MHz clock down to 1us == cyclecount >> 4
          divide=(4+mTIM_7_LINKING);
          decval=(gSystemCycleCount-mTIM_7_LAST_COUNT)>>divide;
      }

      if(decval) {
         mTIM_7_LAST_COUNT+=decval<<divide;
         mTIM_7_CURRENT-=decval;
         if(mTIM_7_CURRENT&0x80000000) {
            // Set carry out
            mTIM_7_BORROW_OUT=TRUE;

            // Set the timer status flag
            if(mTimerInterruptMask&0x80) {
               TRACE_MIKIE0("Update() - TIMER7 IRQ Triggered");
               mTimerStatusFlags|=0x80;
               gSystemIRQ=TRUE;	// Added 19/09/06 fix for IRQ issue
            }

            // Reload if necessary
            if(mTIM_7_ENABLE_RELOAD) {
               mTIM_7_CURRENT+=mTIM_7_BKUP+1;
            } else {
               mTIM_7_CURRENT=0;
            }
            mTIM_7_TIMER_DONE=TRUE;

         } else {
            mTIM_7_BORROW_OUT=FALSE;
         }
         // Set carry in as we did a count
         mTIM_7_BORROW_IN=TRUE;
      } else {
         // Clear carry in as we didn't count
         mTIM_7_BORROW_IN=FALSE;
         // Clear carry out
         mTIM_7_BORROW_OUT=FALSE;
      }

      // Prediction for next timer event cycle number

      if(mTIM_7_LINKING!=7) {
         // Sometimes timeupdates can be >2x rollover in which case
         // then CURRENT may still be negative and we can use it to
         // calc the next timer value, we just want another update ASAP
         tmp=(mTIM_7_CURRENT&0x80000000)?1:((mTIM_7_CURRENT+1)<<divide);
         tmp+=gSystemCycleCount;
         if(tmp<gNextTimerEvent) {
            gNextTimerEvent=tmp;
            TRACE_MIKIE1("Update() - TIMER 7 Set NextTimerEvent = %012d",gNextTimerEvent);
         }
      }
      //				TRACE_MIKIE1("Update() - mTIM_7_CURRENT = %012d",mTIM_7_CURRENT);
      //				TRACE_MIKIE1("Update() - mTIM_7_BKUP    = %012d",mTIM_7_BKUP);
      //				TRACE_MIKIE1("Update() - mTIM_7_LASTCNT = %012d",mTIM_7_LAST_COUNT);
      //				TRACE_MIKIE1("Update() - mTIM_7_LINKING = %012d",mTIM_7_LINKING);
   }

   //
   // Timer 6 has no group
   //
   // KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..)
   if(mTIM_6_ENABLE_COUNT && (mTIM_6_ENABLE_RELOAD || !mTIM_6_TIMER_DONE)) {
      //				if(mTIM_6_LINKING!=0x07)
      {
         // Ordinary clocked mode as opposed to linked mode
         // 16MHz clock down to 1us == cyclecount >> 4
         divide=(4+mTIM_6_LINKING);
         decval=(gSystemCycleCount-mTIM_6_LAST_COUNT)>>divide;

         if(decval) {
            mTIM_6_LAST_COUNT+=decval<<divide;
            mTIM_6_CURRENT-=decval;
            if(mTIM_6_CURRENT&0x80000000) {
               // Set carry out
               mTIM_6_BORROW_OUT=TRUE;

               // Set the timer status flag
               if(mTimerInterruptMask&0x40) {
                  TRACE_MIKIE0("Update() - TIMER6 IRQ Triggered");
                  mTimerStatusFlags|=0x40;
                  gSystemIRQ=TRUE;	// Added 19/09/06 fix for IRQ issue
               }

               // Reload if necessary
               if(mTIM_6_ENABLE_RELOAD) {
                  mTIM_6_CURRENT+=mTIM_6_BKUP+1;
               } else {
                  mTIM_6_CURRENT=0;
               }
               mTIM_6_TIMER_DONE=TRUE;
            } else {
               mTIM_6_BORROW_OUT=FALSE;
            }
            // Set carry in as we did a count
            mTIM_6_BORROW_IN=TRUE;
         } else {
            // Clear carry in as we didn't count
            mTIM_6_BORROW_IN=FALSE;
            // Clear carry out
            mTIM_6_BORROW_OUT=FALSE;
         }
      }

      // Prediction for next timer event cycle number
      // (Timer 6 doesn't support linking)

      //				if(mTIM_6_LINKING!=7)
      {
         // Sometimes timeupdates can be >2x rollover in which case
         // then CURRENT may still be negative and we can use it to
         // calc the next timer value, we just want another update ASAP
         tmp=(mTIM_6_CURRENT&0x80000000)?1:((mTIM_6_CURRENT+1)<<divide);
         tmp+=gSystemCycleCount;
         if(tmp<gNextTimerEvent) {
            gNextTimerEvent=tmp;
            TRACE_MIKIE1("Update() - TIMER 6 Set NextTimerEvent = %012d",gNextTimerEvent);
         }
      }
      //				TRACE_MIKIE1("Update() - mTIM_6_CURRENT = %012d",mTIM_6_CURRENT);
      //				TRACE_MIKIE1("Update() - mTIM_6_BKUP    = %012d",mTIM_6_BKUP);
      //				TRACE_MIKIE1("Update() - mTIM_6_LASTCNT = %012d",mTIM_6_LAST_COUNT);
      //				TRACE_MIKIE1("Update() - mTIM_6_LINKING = %012d",mTIM_6_LINKING);
   }

   //
   // If sound is enabled then update the sound subsystem
   //
   if(gAudioEnabled) {
      UpdateCalcSound();
      UpdateSound();
   }

   //			TRACE_MIKIE1("Update() - NextTimerEvent = %012d",gNextTimerEvent);

   // Update system IRQ status as a result of timer activity
   gSystemIRQ=(mTimerStatusFlags)?true:false;
   if(gSystemIRQ && gSystemCPUSleep) {gSystemCPUSleep=gSystemCPUSleep_Saved=FALSE;/*puts("ARLARM"); */ }
   //else if(gSuzieDoneTime) gSystemCPUSleep=TRUE;

   // Now all the timer updates are done we can increment the system
   // counter for any work done within the Update() function, gSystemCycleCounter
   // cannot be updated until this point otherwise it screws up the counters.
   gSystemCycleCount+=mikie_work_done;
}

inline void CMikie::UpdateCalcSound(void)
{
   SLONG divide = 0;
   SLONG decval = 0;
   ULONG tmp = 0;
   //
   // Audio 0
   //
   // if(mAUDIO_0_ENABLE_COUNT && (mAUDIO_0_ENABLE_RELOAD || !mAUDIO_0_TIMER_DONE))
   if(mAUDIO_0_ENABLE_COUNT && (mAUDIO_0_ENABLE_RELOAD || !mAUDIO_0_TIMER_DONE) && mAUDIO_0_VOLUME && mAUDIO_0_BKUP)
   {
      decval=0;

      if(mAUDIO_0_LINKING==0x07) {
         if(mTIM_7_BORROW_OUT) decval=1;
         mAUDIO_0_LAST_LINK_CARRY=mTIM_7_BORROW_OUT;
      } else {
         // Ordinary clocked mode as opposed to linked mode
         // 16MHz clock down to 1us == cyclecount >> 4
         divide=(4+mAUDIO_0_LINKING);
         decval=(gSystemCycleCount-mAUDIO_0_LAST_COUNT)>>divide;
      }

      if(decval) {
         mAUDIO_0_LAST_COUNT+=decval<<divide;
         mAUDIO_0_CURRENT-=decval;
         if(mAUDIO_0_CURRENT&0x80000000) {
            // Set carry out
            mAUDIO_0_BORROW_OUT=TRUE;

            // Reload if necessary
            if(mAUDIO_0_ENABLE_RELOAD) {
               mAUDIO_0_CURRENT+=mAUDIO_0_BKUP+1;
               if(mAUDIO_0_CURRENT&0x80000000) mAUDIO_0_CURRENT=0;
            } else {
               // Set timer done
               mAUDIO_0_TIMER_DONE=TRUE;
               mAUDIO_0_CURRENT=0;
            }

            //
            // Update audio circuitry
            //
            mAUDIO_0_WAVESHAPER=GetLfsrNext(mAUDIO_0_WAVESHAPER);

            if(mAUDIO_0_INTEGRATE_ENABLE) {
               SLONG temp=mAUDIO_OUTPUT[0];
               if(mAUDIO_0_WAVESHAPER&0x0001) temp+=mAUDIO_0_VOLUME;
               else temp-=mAUDIO_0_VOLUME;
               if(temp>127) temp=127;
               if(temp<-128) temp=-128;
               mAUDIO_OUTPUT[0]=(SBYTE)temp;
            } else {
               if(mAUDIO_0_WAVESHAPER&0x0001) mAUDIO_OUTPUT[0]=mAUDIO_0_VOLUME;
               else mAUDIO_OUTPUT[0]=-mAUDIO_0_VOLUME;
            }
         } else {
            mAUDIO_0_BORROW_OUT=FALSE;
         }
         // Set carry in as we did a count
         mAUDIO_0_BORROW_IN=TRUE;
      } else {
         // Clear carry in as we didn't count
         mAUDIO_0_BORROW_IN=FALSE;
         // Clear carry out
         mAUDIO_0_BORROW_OUT=FALSE;
      }

      // Prediction for next timer event cycle number

      if(mAUDIO_0_LINKING!=7) {
         // Sometimes timeupdates can be >2x rollover in which case
         // then CURRENT may still be negative and we can use it to
         // calc the next timer value, we just want another update ASAP
         tmp=(mAUDIO_0_CURRENT&0x80000000)?1:((mAUDIO_0_CURRENT+1)<<divide);
         tmp+=gSystemCycleCount;
         if(tmp<gNextTimerEvent) {
            gNextTimerEvent=tmp;
            TRACE_MIKIE1("Update() - AUDIO 0 Set NextTimerEvent = %012d",gNextTimerEvent);
         }
      }
      //					TRACE_MIKIE1("Update() - mAUDIO_0_CURRENT = %012d",mAUDIO_0_CURRENT);
      //					TRACE_MIKIE1("Update() - mAUDIO_0_BKUP    = %012d",mAUDIO_0_BKUP);
      //					TRACE_MIKIE1("Update() - mAUDIO_0_LASTCNT = %012d",mAUDIO_0_LAST_COUNT);
      //					TRACE_MIKIE1("Update() - mAUDIO_0_LINKING = %012d",mAUDIO_0_LINKING);
   }

   //
   // Audio 1
   //
   // if(mAUDIO_1_ENABLE_COUNT && (mAUDIO_1_ENABLE_RELOAD || !mAUDIO_1_TIMER_DONE)) {
   if(mAUDIO_1_ENABLE_COUNT && (mAUDIO_1_ENABLE_RELOAD || !mAUDIO_1_TIMER_DONE) && mAUDIO_1_VOLUME && mAUDIO_1_BKUP)
   {
      decval=0;

      if(mAUDIO_1_LINKING==0x07) {
         if(mAUDIO_0_BORROW_OUT) decval=1;
         mAUDIO_1_LAST_LINK_CARRY=mAUDIO_0_BORROW_OUT;
      } else {
         // Ordinary clocked mode as opposed to linked mode
         // 16MHz clock down to 1us == cyclecount >> 4
         divide=(4+mAUDIO_1_LINKING);
         decval=(gSystemCycleCount-mAUDIO_1_LAST_COUNT)>>divide;
      }

      if(decval) {
         mAUDIO_1_LAST_COUNT+=decval<<divide;
         mAUDIO_1_CURRENT-=decval;
         if(mAUDIO_1_CURRENT&0x80000000) {
            // Set carry out
            mAUDIO_1_BORROW_OUT=TRUE;

            // Reload if necessary
            if(mAUDIO_1_ENABLE_RELOAD) {
               mAUDIO_1_CURRENT+=mAUDIO_1_BKUP+1;
               if(mAUDIO_1_CURRENT&0x80000000) mAUDIO_1_CURRENT=0;
            } else {
               // Set timer done
               mAUDIO_1_TIMER_DONE=TRUE;
               mAUDIO_1_CURRENT=0;
            }

            //
            // Update audio circuitry
            //
            mAUDIO_1_WAVESHAPER=GetLfsrNext(mAUDIO_1_WAVESHAPER);

            if(mAUDIO_1_INTEGRATE_ENABLE) {
               SLONG temp=mAUDIO_OUTPUT[1];
               if(mAUDIO_1_WAVESHAPER&0x0001) temp+=mAUDIO_1_VOLUME;
               else temp-=mAUDIO_1_VOLUME;
               if(temp>127) temp=127;
               if(temp<-128) temp=-128;
               mAUDIO_OUTPUT[1]=(SBYTE)temp;
            } else {
               if(mAUDIO_1_WAVESHAPER&0x0001) mAUDIO_OUTPUT[1]=mAUDIO_1_VOLUME;
               else mAUDIO_OUTPUT[1]=-mAUDIO_1_VOLUME;
            }
         } else {
            mAUDIO_1_BORROW_OUT=FALSE;
         }
         // Set carry in as we did a count
         mAUDIO_1_BORROW_IN=TRUE;
      } else {
         // Clear carry in as we didn't count
         mAUDIO_1_BORROW_IN=FALSE;
         // Clear carry out
         mAUDIO_1_BORROW_OUT=FALSE;
      }

      // Prediction for next timer event cycle number

      if(mAUDIO_1_LINKING!=7) {
         // Sometimes timeupdates can be >2x rollover in which case
         // then CURRENT may still be negative and we can use it to
         // calc the next timer value, we just want another update ASAP
         tmp=(mAUDIO_1_CURRENT&0x80000000)?1:((mAUDIO_1_CURRENT+1)<<divide);
         tmp+=gSystemCycleCount;
         if(tmp<gNextTimerEvent) {
            gNextTimerEvent=tmp;
            TRACE_MIKIE1("Update() - AUDIO 1 Set NextTimerEvent = %012d",gNextTimerEvent);
         }
      }
      //					TRACE_MIKIE1("Update() - mAUDIO_1_CURRENT = %012d",mAUDIO_1_CURRENT);
      //					TRACE_MIKIE1("Update() - mAUDIO_1_BKUP    = %012d",mAUDIO_1_BKUP);
      //					TRACE_MIKIE1("Update() - mAUDIO_1_LASTCNT = %012d",mAUDIO_1_LAST_COUNT);
      //					TRACE_MIKIE1("Update() - mAUDIO_1_LINKING = %012d",mAUDIO_1_LINKING);
   }

   //
   // Audio 2
   //
   // if(mAUDIO_2_ENABLE_COUNT && (mAUDIO_2_ENABLE_RELOAD || !mAUDIO_2_TIMER_DONE))
   if(mAUDIO_2_ENABLE_COUNT && (mAUDIO_2_ENABLE_RELOAD || !mAUDIO_2_TIMER_DONE) && mAUDIO_2_VOLUME && mAUDIO_2_BKUP)
   {
      decval=0;

      if(mAUDIO_2_LINKING==0x07) {
         if(mAUDIO_1_BORROW_OUT) decval=1;
         mAUDIO_2_LAST_LINK_CARRY=mAUDIO_1_BORROW_OUT;
      } else {
         // Ordinary clocked mode as opposed to linked mode
         // 16MHz clock down to 1us == cyclecount >> 4
         divide=(4+mAUDIO_2_LINKING);
         decval=(gSystemCycleCount-mAUDIO_2_LAST_COUNT)>>divide;
      }

      if(decval) {
         mAUDIO_2_LAST_COUNT+=decval<<divide;
         mAUDIO_2_CURRENT-=decval;
         if(mAUDIO_2_CURRENT&0x80000000) {
            // Set carry out
            mAUDIO_2_BORROW_OUT=TRUE;

            // Reload if necessary
            if(mAUDIO_2_ENABLE_RELOAD) {
               mAUDIO_2_CURRENT+=mAUDIO_2_BKUP+1;
               if(mAUDIO_2_CURRENT&0x80000000) mAUDIO_2_CURRENT=0;
            } else {
               // Set timer done
               mAUDIO_2_TIMER_DONE=TRUE;
               mAUDIO_2_CURRENT=0;
            }

            //
            // Update audio circuitry
            //
            mAUDIO_2_WAVESHAPER=GetLfsrNext(mAUDIO_2_WAVESHAPER);

            if(mAUDIO_2_INTEGRATE_ENABLE) {
               SLONG temp=mAUDIO_OUTPUT[2];
               if(mAUDIO_2_WAVESHAPER&0x0001) temp+=mAUDIO_2_VOLUME;
               else temp-=mAUDIO_2_VOLUME;
               if(temp>127) temp=127;
               if(temp<-128) temp=-128;
               mAUDIO_OUTPUT[2]=(SBYTE)temp;
            } else {
               if(mAUDIO_2_WAVESHAPER&0x0001) mAUDIO_OUTPUT[2]=mAUDIO_2_VOLUME;
               else mAUDIO_OUTPUT[2]=-mAUDIO_2_VOLUME;
            }
         } else {
            mAUDIO_2_BORROW_OUT=FALSE;
         }
         // Set carry in as we did a count
         mAUDIO_2_BORROW_IN=TRUE;
      } else {
         // Clear carry in as we didn't count
         mAUDIO_2_BORROW_IN=FALSE;
         // Clear carry out
         mAUDIO_2_BORROW_OUT=FALSE;
      }

      // Prediction for next timer event cycle number

      if(mAUDIO_2_LINKING!=7) {
         // Sometimes timeupdates can be >2x rollover in which case
         // then CURRENT may still be negative and we can use it to
         // calc the next timer value, we just want another update ASAP
         tmp=(mAUDIO_2_CURRENT&0x80000000)?1:((mAUDIO_2_CURRENT+1)<<divide);
         tmp+=gSystemCycleCount;
         if(tmp<gNextTimerEvent) {
            gNextTimerEvent=tmp;
            TRACE_MIKIE1("Update() - AUDIO 2 Set NextTimerEvent = %012d",gNextTimerEvent);
         }
      }
      //					TRACE_MIKIE1("Update() - mAUDIO_2_CURRENT = %012d",mAUDIO_2_CURRENT);
      //					TRACE_MIKIE1("Update() - mAUDIO_2_BKUP    = %012d",mAUDIO_2_BKUP);
      //					TRACE_MIKIE1("Update() - mAUDIO_2_LASTCNT = %012d",mAUDIO_2_LAST_COUNT);
      //					TRACE_MIKIE1("Update() - mAUDIO_2_LINKING = %012d",mAUDIO_2_LINKING);
   }

   //
   // Audio 3
   //
   // if(mAUDIO_3_ENABLE_COUNT && (mAUDIO_3_ENABLE_RELOAD || !mAUDIO_3_TIMER_DONE))
   if(mAUDIO_3_ENABLE_COUNT && (mAUDIO_3_ENABLE_RELOAD || !mAUDIO_3_TIMER_DONE) && mAUDIO_3_VOLUME && mAUDIO_3_BKUP)
   {
      decval=0;

      if(mAUDIO_3_LINKING==0x07) {
         if(mAUDIO_2_BORROW_OUT) decval=1;
         mAUDIO_3_LAST_LINK_CARRY=mAUDIO_2_BORROW_OUT;
      } else {
         // Ordinary clocked mode as opposed to linked mode
         // 16MHz clock down to 1us == cyclecount >> 4
         divide=(4+mAUDIO_3_LINKING);
         decval=(gSystemCycleCount-mAUDIO_3_LAST_COUNT)>>divide;
      }

      if(decval) {
         mAUDIO_3_LAST_COUNT+=decval<<divide;
         mAUDIO_3_CURRENT-=decval;
         if(mAUDIO_3_CURRENT&0x80000000) {
            // Set carry out
            mAUDIO_3_BORROW_OUT=TRUE;

            // Reload if necessary
            if(mAUDIO_3_ENABLE_RELOAD) {
               mAUDIO_3_CURRENT+=mAUDIO_3_BKUP+1;
               if(mAUDIO_3_CURRENT&0x80000000) mAUDIO_3_CURRENT=0;
            } else {
               // Set timer done
               mAUDIO_3_TIMER_DONE=TRUE;
               mAUDIO_3_CURRENT=0;
            }

            //
            // Update audio circuitry
            //
            mAUDIO_3_WAVESHAPER=GetLfsrNext(mAUDIO_3_WAVESHAPER);

            if(mAUDIO_3_INTEGRATE_ENABLE) {
               SLONG temp=mAUDIO_OUTPUT[3];
               if(mAUDIO_3_WAVESHAPER&0x0001) temp+=mAUDIO_3_VOLUME;
               else temp-=mAUDIO_3_VOLUME;
               if(temp>127) temp=127;
               if(temp<-128) temp=-128;
               mAUDIO_OUTPUT[3]=(SBYTE)temp;
            } else {
               if(mAUDIO_3_WAVESHAPER&0x0001) mAUDIO_OUTPUT[3]=mAUDIO_3_VOLUME;
               else mAUDIO_OUTPUT[3]=-mAUDIO_3_VOLUME;
            }
         } else {
            mAUDIO_3_BORROW_OUT=FALSE;
         }
         // Set carry in as we did a count
         mAUDIO_3_BORROW_IN=TRUE;
      } else {
         // Clear carry in as we didn't count
         mAUDIO_3_BORROW_IN=FALSE;
         // Clear carry out
         mAUDIO_3_BORROW_OUT=FALSE;
      }

      // Prediction for next timer event cycle number

      if(mAUDIO_3_LINKING!=7) {
         // Sometimes timeupdates can be >2x rollover in which case
         // then CURRENT may still be negative and we can use it to
         // calc the next timer value, we just want another update ASAP
         tmp=(mAUDIO_3_CURRENT&0x80000000)?1:((mAUDIO_3_CURRENT+1)<<divide);
         tmp+=gSystemCycleCount;
         if(tmp<gNextTimerEvent) {
            gNextTimerEvent=tmp;
            TRACE_MIKIE1("Update() - AUDIO 3 Set NextTimerEvent = %012d",gNextTimerEvent);
         }
      }
      //					TRACE_MIKIE1("Update() - mAUDIO_3_CURRENT = %012d",mAUDIO_3_CURRENT);
      //					TRACE_MIKIE1("Update() - mAUDIO_3_BKUP    = %012d",mAUDIO_3_BKUP);
      //					TRACE_MIKIE1("Update() - mAUDIO_3_LASTCNT = %012d",mAUDIO_3_LAST_COUNT);
      //					TRACE_MIKIE1("Update() - mAUDIO_3_LINKING = %012d",mAUDIO_3_LINKING);
   }
}

inline void CMikie::UpdateSound(void)
{
   int samples = (gSystemCycleCount-gAudioLastUpdateCycle)/HANDY_AUDIO_SAMPLE_PERIOD;
   if (samples == 0) return;

   int cur_lsample = 0;
   int cur_rsample = 0;

   for(int x = 0; x < 4; x++){
      /// Assumption (seems there is no documentation for the Attenuation registers)
      /// a) they are linear from $0 to $f - checked!
      /// b) an attenuation of $0 is equal to channel OFF (bits in mSTEREO not set) - checked!
      /// c) an attenuation of $f is NOT equal to no attenuation (bits in PAN not set), $10 would be - checked!
      /// These assumptions can only checked with an oscilloscope... - done
      /// the values stored in mSTEREO are NOT bit-inverted ...
      /// mSTEREO was found to be set like that already (why?), but unused

      if(!(mSTEREO & (0x10 << x))) {
         if(mPAN & (0x10 << x))
            cur_lsample += (mAUDIO_OUTPUT[x]*(mAUDIO_ATTEN[x]&0xF0))/(16*16); /// NOT /15*16 see remark above
         else
            cur_lsample += mAUDIO_OUTPUT[x];
      }
      if(!(mSTEREO & (0x01 << x))) {
         if(mPAN & (0x01 << x))
            cur_rsample += (mAUDIO_OUTPUT[x]*(mAUDIO_ATTEN[x]&0x0F))/16; /// NOT /15 see remark above
         else
            cur_rsample += mAUDIO_OUTPUT[x];
      }
   }

   SWORD sample_l = (cur_lsample << 5);
   SWORD sample_r = (cur_rsample << 5);

   for(; samples > 0; --samples)
   {
      gAudioBuffer[gAudioBufferPointer++] = sample_l;
      gAudioBuffer[gAudioBufferPointer++] = sample_r;
      gAudioBufferPointer %= HANDY_AUDIO_BUFFER_LENGTH;
      gAudioLastUpdateCycle += HANDY_AUDIO_SAMPLE_PERIOD;
   }
}
