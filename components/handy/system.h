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
// System object header file                                                //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition and inline code for   //
// the system object, this object if what binds together all of the Handy   //
// hardware emulation objects, its the glue that holds the system together  //
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

#ifndef SYSTEM_H
#define SYSTEM_H

// #pragma inline_depth (255)
// #pragma inline_recursion (on)

#include <cstdint>

typedef int8_t SBYTE;
typedef uint8_t UBYTE;
typedef int16_t SWORD;
typedef uint16_t UWORD;
typedef int32_t SLONG;
typedef uint32_t ULONG;

#ifndef TRUE
#define TRUE	true
#endif

#ifndef FALSE
#define FALSE	false
#endif

#define HANDY_SYSTEM_FREQ                       16000000
#define HANDY_TIMER_FREQ                        20
#define HANDY_AUDIO_SAMPLE_FREQ                 24000 // 48000
#define HANDY_AUDIO_SAMPLE_PERIOD               (HANDY_SYSTEM_FREQ/HANDY_AUDIO_SAMPLE_FREQ)

// for a bit over one frame of 16bit stereo
#define HANDY_AUDIO_BUFFER_LENGTH               ((HANDY_AUDIO_SAMPLE_FREQ/60+25)*2)

#ifdef SDL_PATCH
//#define HANDY_AUDIO_BUFFER_SIZE               4096    // Needed for SDL 8bit MONO
//#define HANDY_AUDIO_BUFFER_SIZE               8192    // Needed for SDL STEREO 8bit
//#define HANDY_AUDIO_BUFFER_SIZE               16384   // Needed for SDL STEREO 16bit
#else
//#define HANDY_AUDIO_BUFFER_SIZE               (HANDY_AUDIO_SAMPLE_FREQ/4)
//#define HANDY_AUDIO_BUFFER_SIZE               (HANDY_AUDIO_SAMPLE_FREQ)
#endif


#define HANDY_FILETYPE_LNX      0
#define HANDY_FILETYPE_HOMEBREW 1
#define HANDY_FILETYPE_SNAPSHOT 2
#define HANDY_FILETYPE_ILLEGAL  3
#define HANDY_FILETYPE_RAW      4

#define HANDY_SCREEN_WIDTH   160
#define HANDY_SCREEN_HEIGHT  102
//
// Define the global variable list
//
extern ULONG    gSystemCycleCount;
extern ULONG    gNextTimerEvent;
extern ULONG    gCPUWakeupTime;
extern ULONG    gIRQEntryCycle;
extern ULONG    gCPUBootAddress;
extern ULONG    gBreakpointHit;
extern ULONG    gSingleStepMode;
extern ULONG    gSingleStepModeSprites;
extern ULONG    gSystemIRQ;
extern ULONG    gSystemNMI;
extern ULONG    gSystemCPUSleep;
extern ULONG    gSystemCPUSleep_Saved;
extern ULONG    gSystemHalt;
extern ULONG    gThrottleMaxPercentage;
extern ULONG    gThrottleLastTimerCount;
extern ULONG    gThrottleNextCycleCheckpoint;
extern ULONG    gEndOfFrame;
extern ULONG    gTimerCount;
extern ULONG    gRenderFrame;

extern ULONG    gAudioEnabled;
extern SWORD    *gAudioBuffer;
extern ULONG    gAudioBufferPointer;
extern ULONG    gAudioLastUpdateCycle;
extern UBYTE    *gPrimaryFrameBuffer;

// typedef struct lssfile
// {
//    UBYTE *memptr;
//    ULONG index;
//    ULONG index_limit;
// } LSS_FILE;

// int lss_read(void* dest, int varsize, int varcount, LSS_FILE *fp);
// int lss_write(void* src, int varsize, int varcount, LSS_FILE *fp);
// int lss_printf(LSS_FILE *fp, const char *str);

#define LSS_FILE FILE
#define lss_read(d, vs, vc, fp) (fread(d, vs, vc, fp) > 0)
#define lss_write(s, vs, vc, fp) (fwrite(s, vs, vc, fp) > 0)
#define lss_printf(fp, str) (fputs(str, fp) >= 0)

//
// Define logging functions
//
#ifdef RETRO_GO
#include <rg_system.h>
#define log_printf(x...) rg_system_log(RG_LOG_USER, NULL, x)
#define crc32_le(a, b, c) rg_crc32(a, b, c)
#else
#include <stdio.h>
#define log_printf(x...) printf(x)
#define crc32_le(a, b, c) (0)
#endif

//
// Define the interfaces before we start pulling in the classes
// as many classes look for articles from the interfaces to
// allow compilation

#include "sysbase.h"

class CSystem;

//
// Now pull in the parts that build the system
//
#include "lynxbase.h"
#include "ram.h"
#include "cart.h"
#include "eeprom.h"
#include "susie.h"
#include "mikie.h"
#include "c65c02.h"

#define TOP_START   0xfc00
#define TOP_MASK    0x03ff
#define TOP_SIZE    0x400
#define SYSTEM_SIZE 65536

#define LSS_VERSION     "LSS3"

class CSystem : public CSystemBase
{
   public:
      CSystem(const char* gamefile, long displayformat, long samplerate);
      ~CSystem();

   public:
      void HLE_BIOS_FE00(void);
      void HLE_BIOS_FE19(void);
      void HLE_BIOS_FE4A(void);
      void HLE_BIOS_FF80(void);
      void Reset(void);
      bool ContextSave(LSS_FILE *fp);
      bool ContextLoad(LSS_FILE *fp);
      void SaveEEPROM(void);

      inline void Update(void)
      {
         //
         // Only update if there is a predicted timer event
         //
         if(gSystemCycleCount>=gNextTimerEvent)
         {
            mMikie->Update();
         }
         //
         // Step the processor through 1 instruction
         //
         mCpu->Update();

#ifdef _LYNXDBG
         // Check breakpoint
         static ULONG lastcycle=0;
         if(lastcycle<mCycleCountBreakpoint && gSystemCycleCount>=mCycleCountBreakpoint) gBreakpointHit=TRUE;
         lastcycle=gSystemCycleCount;

         // Check single step mode
         if(gSingleStepMode) gBreakpointHit=TRUE;
#endif

         //
         // If the CPU is asleep then skip to the next timer event
         //
         if(gSystemCPUSleep)
         {
            gSystemCycleCount=gNextTimerEvent;
         }
      }

      inline void UpdateFrame(bool draw)
      {
         gEndOfFrame = FALSE;
         gRenderFrame = draw;

         while(gEndOfFrame != TRUE)
         {
            if(gSystemCycleCount>=gNextTimerEvent)
            {
               mMikie->Update();
            }

            mCpu->Update();

         #ifdef _LYNXDBG
                  // Check breakpoint
                  static ULONG lastcycle=0;
                  if(lastcycle<mCycleCountBreakpoint && gSystemCycleCount>=mCycleCountBreakpoint) gBreakpointHit=TRUE;
                  lastcycle=gSystemCycleCount;

                  // Check single step mode
                  if(gSingleStepMode) gBreakpointHit=TRUE;
         #endif

            if(gSystemCPUSleep)
            {
               gSystemCycleCount=gNextTimerEvent;
            }
         }
      }

      //
      // CPU
      //
      inline void  Poke_CPU(ULONG addr, UBYTE data) {
         if (addr < 0xFC00) {             // 0000-FBFF Always RAM
            // mRamPointer[addr] = data;
            // return;
         }
         else if (addr == 0xFFF9) {       // MMU
            mMemMapReg = data;
            return;
         }
         else if ((addr >> 8) == 0xFC) {  // FC00-FCFF Susie area
            if ((mMemMapReg & 0x1) == 0) {
               mSusie->Poke(addr, data);
               return;
            }
         }
         else if ((addr >> 8) == 0xFD) {  // FD00-FDFF Mikie area
            if ((mMemMapReg & 0x2) == 0) {
               mMikie->Poke(addr, data);
               return;
            }
         }
         else if (addr < 0xFFF8) {        // FE00-FFF7 Bios ROM
             if ((mMemMapReg & 0x4) == 0)
               return;
         }
         else {                           // FFFA-FFFF Vector area
            if ((mMemMapReg & 0x8) == 0)
               return;
         }

         mRamPointer[addr] = data;
      };
      inline UBYTE Peek_CPU(ULONG addr) {
         if (addr < 0xFC00) {             // 0000-FBFF Always RAM
            // return mRamPointer[addr];
         }
         else if (addr == 0xFFF9) {
            return mMemMapReg;
         }
         else if ((addr >> 8) == 0xFC) {  // FC00-FCFF Susie area
            if ((mMemMapReg & 0x1) == 0)
               return mSusie->Peek(addr);
         }
         else if ((addr >> 8) == 0xFD) {  // FD00-FDFF Mikie area
            if ((mMemMapReg & 0x2) == 0)
               return mMikie->Peek(addr);
         }
         else if (addr < 0xFFF8) {        // FE00-FFF7 Bios ROM
            if ((mMemMapReg & 0x4) == 0)
               return mBiosRom[addr & 0x1FF];
         }
         else {                           // FFFA-FFFF Vector area
            if ((mMemMapReg & 0x8) == 0)
               return mBiosVectors[addr - 0xFFFA];
         }

         return mRamPointer[addr];
      };
      inline void  PokeW_CPU(ULONG addr,UWORD data) { Poke_CPU(addr, data&0xff); Poke_CPU(addr + 1, data >> 8); };
      inline UWORD PeekW_CPU(ULONG addr) { return ((Peek_CPU(addr))+(Peek_CPU(addr+1)<<8)); };

      // Mikey system interfacing

      void   ComLynxCable(int status) { mMikie->ComLynxCable(status); };
      void   ComLynxRxData(int data)  { mMikie->ComLynxRxData(data); };
      void   ComLynxTxCallback(void (*function)(int data,ULONG objref),ULONG objref) { mMikie->ComLynxTxCallback(function,objref); };

      // Miscellaneous

      void   SetButtonData(ULONG data) {mSusie->SetButtonData(data);};
      ULONG  GetButtonData(void) {return mSusie->GetButtonData();};
      void   SetCycleBreakpoint(ULONG breakpoint) {mCycleCountBreakpoint=breakpoint;};
      UBYTE* GetRamPointer(void) {return mRam->GetRamPointer();};

      // Debugging

      void   DebugTrace(int address);
      void   DebugSetCallback(void (*function)(ULONG objref, char *message),ULONG objref);
      void   (*mpDebugCallback)(ULONG objref, char *message);
      ULONG  mDebugCallbackObject;

   public:
      ULONG         mCycleCountBreakpoint;
      CCart         *mCart;
      CRam          *mRam;
      C65C02        *mCpu;
      CMikie        *mMikie;
      CSusie        *mSusie;
      CEEPROM       *mEEPROM;

      ULONG         mFileType;

      UBYTE         mMemMapReg;
      UBYTE         *mRamPointer;
      UBYTE         mBiosVectors[0x8];
      UBYTE         mBiosRom[0x200];
};

#endif
