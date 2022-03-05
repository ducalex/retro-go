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
// Mikey class header file                                                  //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition and some of the code  //
// for the Mikey chip within the Lynx. The most crucial code is the         //
// Update() function which as you can probably guess updates all of the     //
// Mikey hardware counters and screen DMA from the prevous time it was      //
// called. Yes I know how to spell Mikey but I cant be bothered to change   //
// it everywhere.                                                           //
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

#ifndef MIKIE_H
#define MIKIE_H

//#define	TRACE_MIKIE

#ifdef TRACE_MIKIE

#define TRACE_MIKIE0(msg)					_RPT1(_CRT_WARN,"CMikie::"msg" (Time=%012d)\n",gSystemCycleCount)
#define TRACE_MIKIE1(msg,arg1)				_RPT2(_CRT_WARN,"CMikie::"msg" (Time=%012d)\n",arg1,gSystemCycleCount)
#define TRACE_MIKIE2(msg,arg1,arg2)			_RPT3(_CRT_WARN,"CMikie::"msg" (Time=%012d)\n",arg1,arg2,gSystemCycleCount)
#define TRACE_MIKIE3(msg,arg1,arg2,arg3)	_RPT4(_CRT_WARN,"CMikie::"msg" (Time=%012d)\n",arg1,arg2,arg3,gSystemCycleCount)

#else

#define TRACE_MIKIE0(msg)
#define TRACE_MIKIE1(msg,arg1)
#define TRACE_MIKIE2(msg,arg1,arg2)
#define TRACE_MIKIE3(msg,arg1,arg2,arg3)

#endif

class CSystem;

#define MIKIE_START	0xfd00
#define MIKIE_SIZE	0x100

#define DMA_RDWR_CYC	4

//
// Define counter types and defines
//

#define CTRL_A_IRQEN	0x80
#define CTRL_A_RTD		0x40
#define CTRL_A_RELOAD	0x10
#define CTRL_A_COUNT	0x08
#define CTRL_A_DIVIDE	0x07

#define CTRL_B_TDONE	0x08
#define CTRL_B_LASTCK	0x04
#define CTRL_B_CIN		0x02
#define CTRL_B_COUT		0x01

#define LINE_TIMER		0x00
#define SCREEN_TIMER	0x02

#define LINE_WIDTH		160
#define LINE_SIZE		80

#define UART_TX_INACTIVE	0x80000000
#define UART_RX_INACTIVE	0x80000000
#define UART_BREAK_CODE		0x00008000
#define UART_MAX_RX_QUEUE	32
#define UART_TX_TIME_PERIOD	(11)
#define UART_RX_TIME_PERIOD	(11)
#define UART_RX_NEXT_DELAY	(44)

typedef struct
{
   union
   {
      struct
      {
#ifdef MSB_FIRST
         UBYTE unused:8;
         UBYTE unused2:8;
         UBYTE unused3:4;
         UBYTE Blue:4;
         UBYTE Red:4;
         UBYTE Green:4;
#else
         UBYTE Green:4;
         UBYTE Red:4;
         UBYTE Blue:4;
#endif
      }Colours;
      ULONG     Index;
   };
}TPALETTE;


//
// Enumerated types for possible mikie windows independant modes
//
enum
{
   MIKIE_BAD_MODE=0,
   MIKIE_NO_ROTATE,
   MIKIE_ROTATE_L,
   MIKIE_ROTATE_R
};

enum
{
   MIKIE_PIXEL_FORMAT_16BPP_565=0,
   MIKIE_PIXEL_FORMAT_16BPP_565_BE
};

class CMikie : public CLynxBase
{
   public:
      CMikie(CSystem& parent, ULONG displayformat, ULONG samplerate);
      ~CMikie();

      bool	ContextSave(LSS_FILE *fp);
      bool	ContextLoad(LSS_FILE *fp);
      void	Reset(void);

      UBYTE	Peek(ULONG addr);
      void	Poke(ULONG addr,UBYTE data);
      ULONG	ObjectSize(void) {return MIKIE_SIZE;};
      void	PresetForHomebrew(void);

      void	ComLynxCable(int status);
      void	ComLynxRxData(int data);
      void	ComLynxTxLoopback(int data);
      void	ComLynxTxCallback(void (*function)(int data,ULONG objref),ULONG objref);

      void	BuildPalette(void);

      void Update(void);
      void SetRotation(UBYTE data) {mDisplayRotate_Pending = data;};
      inline bool SwitchAudInDir(void){ return(mIODIR&0x10);};
      inline bool SwitchAudInValue(void){ return (mIODAT&0x10);};

   private:
      inline void UpdateSound(void);
      inline void UpdateCalcSound(void);
      inline void ResetDisplayPtr();
      ULONG	DisplayRenderLine(void);
      void	BlowOut(void);

   private:
      CSystem		&mSystem;

      // Hardware storage

      ULONG		mDisplayAddress;
      ULONG		mAudioInputComparator;
      ULONG		mTimerStatusFlags;
      ULONG		mTimerInterruptMask;
      UBYTE    mDisplayRotate;
      UBYTE    mDisplayRotate_Pending;

      TPALETTE	mPalette[16];
      UWORD		mColourMap[4096];

      ULONG		mIODAT;
      ULONG		mIODIR;
      ULONG		mIODAT_REST_SIGNAL;

      ULONG		mDISPCTL_DMAEnable;
      ULONG		mDISPCTL_Flip;
      ULONG		mDISPCTL_FourColour;
      ULONG		mDISPCTL_Colour;

      ULONG		mTIM_0_BKUP;
      ULONG		mTIM_0_ENABLE_RELOAD;
      ULONG		mTIM_0_ENABLE_COUNT;
      ULONG		mTIM_0_LINKING;
      ULONG		mTIM_0_CURRENT;
      ULONG		mTIM_0_TIMER_DONE;
      ULONG		mTIM_0_LAST_CLOCK;
      ULONG		mTIM_0_BORROW_IN;
      ULONG		mTIM_0_BORROW_OUT;
      ULONG		mTIM_0_LAST_LINK_CARRY;
      ULONG		mTIM_0_LAST_COUNT;

      ULONG		mTIM_1_BKUP;
      ULONG		mTIM_1_ENABLE_RELOAD;
      ULONG		mTIM_1_ENABLE_COUNT;
      ULONG		mTIM_1_LINKING;
      ULONG		mTIM_1_CURRENT;
      ULONG		mTIM_1_TIMER_DONE;
      ULONG		mTIM_1_LAST_CLOCK;
      ULONG		mTIM_1_BORROW_IN;
      ULONG		mTIM_1_BORROW_OUT;
      ULONG		mTIM_1_LAST_LINK_CARRY;
      ULONG		mTIM_1_LAST_COUNT;

      ULONG		mTIM_2_BKUP;
      ULONG		mTIM_2_ENABLE_RELOAD;
      ULONG		mTIM_2_ENABLE_COUNT;
      ULONG		mTIM_2_LINKING;
      ULONG		mTIM_2_CURRENT;
      ULONG		mTIM_2_TIMER_DONE;
      ULONG		mTIM_2_LAST_CLOCK;
      ULONG		mTIM_2_BORROW_IN;
      ULONG		mTIM_2_BORROW_OUT;
      ULONG		mTIM_2_LAST_LINK_CARRY;
      ULONG		mTIM_2_LAST_COUNT;

      ULONG		mTIM_3_BKUP;
      ULONG		mTIM_3_ENABLE_RELOAD;
      ULONG		mTIM_3_ENABLE_COUNT;
      ULONG		mTIM_3_LINKING;
      ULONG		mTIM_3_CURRENT;
      ULONG		mTIM_3_TIMER_DONE;
      ULONG		mTIM_3_LAST_CLOCK;
      ULONG		mTIM_3_BORROW_IN;
      ULONG		mTIM_3_BORROW_OUT;
      ULONG		mTIM_3_LAST_LINK_CARRY;
      ULONG		mTIM_3_LAST_COUNT;

      ULONG		mTIM_4_BKUP;
      ULONG		mTIM_4_ENABLE_RELOAD;
      ULONG		mTIM_4_ENABLE_COUNT;
      ULONG		mTIM_4_LINKING;
      ULONG		mTIM_4_CURRENT;
      ULONG		mTIM_4_TIMER_DONE;
      ULONG		mTIM_4_LAST_CLOCK;
      ULONG		mTIM_4_BORROW_IN;
      ULONG		mTIM_4_BORROW_OUT;
      ULONG		mTIM_4_LAST_LINK_CARRY;
      ULONG		mTIM_4_LAST_COUNT;

      ULONG		mTIM_5_BKUP;
      ULONG		mTIM_5_ENABLE_RELOAD;
      ULONG		mTIM_5_ENABLE_COUNT;
      ULONG		mTIM_5_LINKING;
      ULONG		mTIM_5_CURRENT;
      ULONG		mTIM_5_TIMER_DONE;
      ULONG		mTIM_5_LAST_CLOCK;
      ULONG		mTIM_5_BORROW_IN;
      ULONG		mTIM_5_BORROW_OUT;
      ULONG		mTIM_5_LAST_LINK_CARRY;
      ULONG		mTIM_5_LAST_COUNT;

      ULONG		mTIM_6_BKUP;
      ULONG		mTIM_6_ENABLE_RELOAD;
      ULONG		mTIM_6_ENABLE_COUNT;
      ULONG		mTIM_6_LINKING;
      ULONG		mTIM_6_CURRENT;
      ULONG		mTIM_6_TIMER_DONE;
      ULONG		mTIM_6_LAST_CLOCK;
      ULONG		mTIM_6_BORROW_IN;
      ULONG		mTIM_6_BORROW_OUT;
      ULONG		mTIM_6_LAST_LINK_CARRY;
      ULONG		mTIM_6_LAST_COUNT;

      ULONG		mTIM_7_BKUP;
      ULONG		mTIM_7_ENABLE_RELOAD;
      ULONG		mTIM_7_ENABLE_COUNT;
      ULONG		mTIM_7_LINKING;
      ULONG		mTIM_7_CURRENT;
      ULONG		mTIM_7_TIMER_DONE;
      ULONG		mTIM_7_LAST_CLOCK;
      ULONG		mTIM_7_BORROW_IN;
      ULONG		mTIM_7_BORROW_OUT;
      ULONG		mTIM_7_LAST_LINK_CARRY;
      ULONG		mTIM_7_LAST_COUNT;

      ULONG		mAUDIO_0_BKUP;
      ULONG		mAUDIO_0_ENABLE_RELOAD;
      ULONG		mAUDIO_0_ENABLE_COUNT;
      ULONG		mAUDIO_0_LINKING;
      ULONG		mAUDIO_0_CURRENT;
      ULONG		mAUDIO_0_TIMER_DONE;
      ULONG		mAUDIO_0_LAST_CLOCK;
      ULONG		mAUDIO_0_BORROW_IN;
      ULONG		mAUDIO_0_BORROW_OUT;
      ULONG		mAUDIO_0_LAST_LINK_CARRY;
      ULONG		mAUDIO_0_LAST_COUNT;
      SBYTE		mAUDIO_0_VOLUME;
      ULONG		mAUDIO_0_INTEGRATE_ENABLE;
      ULONG		mAUDIO_0_WAVESHAPER;

      ULONG		mAUDIO_1_BKUP;
      ULONG		mAUDIO_1_ENABLE_RELOAD;
      ULONG		mAUDIO_1_ENABLE_COUNT;
      ULONG		mAUDIO_1_LINKING;
      ULONG		mAUDIO_1_CURRENT;
      ULONG		mAUDIO_1_TIMER_DONE;
      ULONG		mAUDIO_1_LAST_CLOCK;
      ULONG		mAUDIO_1_BORROW_IN;
      ULONG		mAUDIO_1_BORROW_OUT;
      ULONG		mAUDIO_1_LAST_LINK_CARRY;
      ULONG		mAUDIO_1_LAST_COUNT;
      SBYTE		mAUDIO_1_VOLUME;
      ULONG		mAUDIO_1_INTEGRATE_ENABLE;
      ULONG		mAUDIO_1_WAVESHAPER;

      ULONG		mAUDIO_2_BKUP;
      ULONG		mAUDIO_2_ENABLE_RELOAD;
      ULONG		mAUDIO_2_ENABLE_COUNT;
      ULONG		mAUDIO_2_LINKING;
      ULONG		mAUDIO_2_CURRENT;
      ULONG		mAUDIO_2_TIMER_DONE;
      ULONG		mAUDIO_2_LAST_CLOCK;
      ULONG		mAUDIO_2_BORROW_IN;
      ULONG		mAUDIO_2_BORROW_OUT;
      ULONG		mAUDIO_2_LAST_LINK_CARRY;
      ULONG		mAUDIO_2_LAST_COUNT;
      SBYTE		mAUDIO_2_VOLUME;
      ULONG		mAUDIO_2_INTEGRATE_ENABLE;
      ULONG		mAUDIO_2_WAVESHAPER;

      ULONG		mAUDIO_3_BKUP;
      ULONG		mAUDIO_3_ENABLE_RELOAD;
      ULONG		mAUDIO_3_ENABLE_COUNT;
      ULONG		mAUDIO_3_LINKING;
      ULONG		mAUDIO_3_CURRENT;
      ULONG		mAUDIO_3_TIMER_DONE;
      ULONG		mAUDIO_3_LAST_CLOCK;
      ULONG		mAUDIO_3_BORROW_IN;
      ULONG		mAUDIO_3_BORROW_OUT;
      ULONG		mAUDIO_3_LAST_LINK_CARRY;
      ULONG		mAUDIO_3_LAST_COUNT;
      SBYTE		mAUDIO_3_VOLUME;
      ULONG		mAUDIO_3_INTEGRATE_ENABLE;
      ULONG		mAUDIO_3_WAVESHAPER;

      SBYTE		mAUDIO_OUTPUT[4];
      UBYTE    mAUDIO_ATTEN[4];
      ULONG		mSTEREO;
      ULONG		mPAN;

      //
      // Serial related variables
      //
      ULONG		mUART_RX_IRQ_ENABLE;
      ULONG		mUART_TX_IRQ_ENABLE;

      ULONG		mUART_RX_COUNTDOWN;
      ULONG		mUART_TX_COUNTDOWN;

      ULONG		mUART_SENDBREAK;
      ULONG		mUART_TX_DATA;
      ULONG		mUART_RX_DATA;
      ULONG		mUART_RX_READY;

      ULONG		mUART_PARITY_ENABLE;
      ULONG		mUART_PARITY_EVEN;

      SLONG		mUART_CABLE_PRESENT;
      void		(*mpUART_TX_CALLBACK)(int data,ULONG objref);
      ULONG		mUART_TX_CALLBACK_OBJECT;

      SLONG		mUART_Rx_input_queue[UART_MAX_RX_QUEUE];
      ULONG 	mUART_Rx_input_ptr;
      ULONG 	mUART_Rx_output_ptr;
      SLONG		mUART_Rx_waiting;
      SLONG		mUART_Rx_framing_error;
      SLONG		mUART_Rx_overun_error;

      //
      // Screen related
      //

      UBYTE		*mpDisplayCurrent;
      UBYTE		*mpRamPointer;
      ULONG		mLynxLine;
      ULONG		mLynxLineDMACounter;
      ULONG		mLynxAddr;

      ULONG		mAudioSampleRate;
      ULONG		mDisplayFormat;
      ULONG		mDisplayPitch;
};


#endif
