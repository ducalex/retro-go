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
// System object class                                                      //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class provides the glue to bind of of the emulation objects         //
// together via peek/poke handlers and pass thru interfaces to lower        //
// objects, all control of the emulator is done via this class. Update()    //
// does most of the work and each call emulates one CPU instruction and     //
// updates all of the relevant hardware if required. It must be remembered  //
// that if that instruction involves setting SPRGO then, it will cause a    //
// sprite painting operation and then a corresponding update of all of the  //
// hardware which will usually involve recursive calls to Update, see       //
// Mikey SPRGO code for more details.                                       //
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system.h"

//
// Define the global variable list
//
ULONG   gSystemCycleCount=0;
ULONG   gNextTimerEvent=0;
ULONG   gCPUWakeupTime=0;
ULONG   gIRQEntryCycle=0;
ULONG   gCPUBootAddress=0;
ULONG   gBreakpointHit=FALSE;
ULONG   gSingleStepMode=FALSE;
ULONG   gSingleStepModeSprites=FALSE;
ULONG   gSystemIRQ=FALSE;
ULONG   gSystemNMI=FALSE;
ULONG   gSystemCPUSleep=FALSE;
ULONG   gSystemCPUSleep_Saved=FALSE;
ULONG   gSystemHalt=FALSE;
ULONG   gThrottleMaxPercentage=100;
ULONG   gThrottleLastTimerCount=0;
ULONG   gThrottleNextCycleCheckpoint=0;
ULONG   gEndOfFrame=0;
ULONG   gTimerCount=0;
ULONG   gRenderFrame=1;

ULONG   gAudioEnabled=FALSE;
SWORD   *gAudioBuffer;//[HANDY_AUDIO_BUFFER_SIZE];
ULONG   gAudioBufferPointer=0;
ULONG   gAudioLastUpdateCycle=0;
UBYTE   *gPrimaryFrameBuffer=NULL;


extern void lynx_decrypt(unsigned char * result, const unsigned char * encrypted, const int length);

#if 0
int lss_read(void* dest, int varsize, int varcount, LSS_FILE *fp)
{
   ULONG copysize;
   copysize=varsize*varcount;
   if((fp->index + copysize) > fp->index_limit) copysize=fp->index_limit - fp->index;
   memcpy(dest,fp->memptr+fp->index,copysize);
   fp->index+=copysize;
   return copysize;
}

int lss_write(void* src, int varsize, int varcount, LSS_FILE *fp)
{
   ULONG copysize;
   copysize=varsize*varcount;
   //if((fp->index + copysize) > fp->index_limit) copysize=fp->index_limit - fp->index;
   memcpy(fp->memptr+fp->index,src,copysize);
   fp->index+=copysize;
   return copysize;
}

int lss_printf(LSS_FILE *fp, const char *str)
{
   ULONG copysize;
   copysize=strlen(str);
   memcpy(fp->memptr+fp->index,str,copysize);
   fp->index+=copysize;
   return copysize;
}
#endif


CSystem::CSystem(const char* filename, long displayformat, long samplerate)
 : mCart(NULL),
   mRam(NULL),
   mCpu(NULL),
   mMikie(NULL),
   mSusie(NULL),
   mEEPROM(NULL)
{
   UBYTE *filedata = NULL;
   ULONG filesize = 0;
   FILE *fp;

   log_printf("Loading '%s'...\n", filename);

   if ((fp = fopen(filename, "rb"))) {
      fseek(fp, 0, SEEK_END);
      filesize = ftell(fp);
      filedata = (UBYTE*)malloc(filesize);
      fseek(fp, 0, SEEK_SET);
      if (!filedata) {
         log_printf("-> memory allocation failed (%d bytes)!\n", filesize);
      } else if (fread(filedata, filesize, 1, fp) != 1) {
         log_printf("-> fread failed (%d bytes)!\n", filesize);
      } else {
         // log_printf("-> read ok. size=%d, crc32=%08X\n", filesize, crc32_le(0, filedata, filesize));
         log_printf("-> read ok. size=%d\n", filesize);
      }
      fclose(fp);
   } else {
      log_printf("-> fopen failed!\n");
   }

   // Now try and determine the filetype we have opened
   if (!filedata || filesize < 16) {
      mFileType = HANDY_FILETYPE_ILLEGAL;
      mCart = new CCart(0, 0);
      mRam = new CRam(0, 0);
   } else if (!memcmp(filedata + 6, "BS93", 4)) {
      mFileType = HANDY_FILETYPE_HOMEBREW;
      mCart = new CCart(0, 0);
      mRam = new CRam(filedata, filesize);
   } else {
      mFileType = memcmp(filedata, "LYNX", 4) ? HANDY_FILETYPE_RAW : HANDY_FILETYPE_LNX;
      mCart = new CCart(filedata, filesize);
      mRam = new CRam(0, 0);

      // Setup BIOS
      memset(mBiosRom, 0x88, sizeof(mBiosRom));
      mBiosRom[0x00] = 0x8d;
      mBiosRom[0x01] = 0x97;
      mBiosRom[0x02] = 0xfd;
      mBiosRom[0x03] = 0x60; // RTS
      mBiosRom[0x19] = 0x8d;
      mBiosRom[0x20] = 0x97;
      mBiosRom[0x21] = 0xfd;
      mBiosRom[0x4A] = 0x8d;
      mBiosRom[0x4B] = 0x97;
      mBiosRom[0x4C] = 0xfd;
      mBiosRom[0x180] = 0x8d;
      mBiosRom[0x181] = 0x97;
      mBiosRom[0x182] = 0xfd;
   }

   // Vectors
   mBiosVectors[0] = 0x00;
   mBiosVectors[1] = 0x30;
   mBiosVectors[2] = 0x80;
   mBiosVectors[3] = 0xFF;
   mBiosVectors[4] = 0x80;
   mBiosVectors[5] = 0xFF;

   // Regain some memory before initializing the rest
   free(filedata);

   mRamPointer = mRam->GetRamPointer();
   mMemMapReg = 0x00;

   mCycleCountBreakpoint = 0xffffffff;
   mpDebugCallback = NULL;
   mDebugCallbackObject = 0;

   mMikie = new CMikie(*this, displayformat, samplerate);
   mSusie = new CSusie(*this);
   mCpu = new C65C02(*this);
   mEEPROM = new CEEPROM(mCart->CartGetEEPROMType());

   // Now init is complete do a reset, this will cause many things to be reset twice
   // but what the hell, who cares, I don't.....

   Reset();
}

void CSystem::SaveEEPROM(void)
{
   if(mEEPROM!=NULL) mEEPROM->Save();
}

CSystem::~CSystem()
{
   // Cleanup all our objects

   if(mEEPROM!=NULL) delete mEEPROM;
   if(mCart!=NULL) delete mCart;
   if(mRam!=NULL) delete mRam;
   if(mCpu!=NULL) delete mCpu;
   if(mMikie!=NULL) delete mMikie;
   if(mSusie!=NULL) delete mSusie;
}

void CSystem::HLE_BIOS_FE00(void)
{
    // Select Block in A
    C6502_REGS regs;
    mCpu->GetRegs(regs);
    mCart->SetShifterValue(regs.A);
    // we just put an RTS behind in fake ROM!
}

void CSystem::HLE_BIOS_FE19(void)
{
   // (not) initial jump from reset vector
   // Clear full 64k memory!
   memset(mRamPointer, 0x00, RAM_SIZE);

   // Set Load adresse to $200 ($05,$06)
   Poke_CPU(0x0005,0x00);
   Poke_CPU(0x0006,0x02);
   // Call to $FE00
   mCart->SetShifterValue(0);
   // Fallthrough $FE4A
   HLE_BIOS_FE4A();
}

void CSystem::HLE_BIOS_FE4A(void)
{
   UWORD addr=PeekW_CPU(0x0005);

   // Load from Cart (loader blocks)
   unsigned char buff[256];// maximum 5 blocks
   unsigned char res[256];

   buff[0]=mCart->Peek0();
   int blockcount = 0x100 -  buff[0];

   for (int i = 1; i < 1+51*blockcount; ++i) { // first encrypted loader
      buff[i] = mCart->Peek0();
   }

   lynx_decrypt(res, buff, 51);

   for (int i = 0; i < 50*blockcount; ++i) {
      Poke_CPU(addr++, res[i]);
   }

   // Load Block(s), decode to ($05,$06)
   // jmp $200

   C6502_REGS regs;
   mCpu->GetRegs(regs);
   regs.PC=0x0200;
   mCpu->SetRegs(regs);
}

void CSystem::HLE_BIOS_FF80(void)
{
    // initial jump from reset vector ... calls FE19
    HLE_BIOS_FE19();
}

void CSystem::Reset(void)
{
   gSystemCycleCount=0;
   gNextTimerEvent=0;
   gCPUBootAddress=0;
   gBreakpointHit=FALSE;
   gSingleStepMode=FALSE;
   gSingleStepModeSprites=FALSE;
   gSystemIRQ=FALSE;
   gSystemNMI=FALSE;
   gSystemCPUSleep=FALSE;
   gSystemHalt=FALSE;

   gThrottleLastTimerCount=0;
   gThrottleNextCycleCheckpoint=0;

   gTimerCount=0;

   gAudioBufferPointer=0;
   gAudioLastUpdateCycle=0;
   //	memset(gAudioBuffer,128,HANDY_AUDIO_BUFFER_SIZE); // only for unsigned 8bit
   // memset(gAudioBuffer,0,HANDY_AUDIO_BUFFER_SIZE); // for unsigned 8/16 bit

#ifdef _LYNXDBG
   gSystemHalt=TRUE;
#endif

   mCart->Reset();
   mEEPROM->Reset();
   mRam->Reset();
   mMikie->Reset();
   mSusie->Reset();
   mCpu->Reset();

   // Homebrew hashup
   if(mFileType==HANDY_FILETYPE_HOMEBREW) {
      mMikie->PresetForHomebrew();
      C6502_REGS regs;
      mCpu->GetRegs(regs);
      regs.PC=(UWORD)gCPUBootAddress;
      mCpu->SetRegs(regs);
   }
}

bool CSystem::ContextSave(LSS_FILE *fp)
{
   bool status=1;

   // fp->index = 0;
   if(!lss_printf(fp, LSS_VERSION)) status=0;

   // Save ROM CRC
   ULONG checksum=mCart->CRC32();
   if(!lss_write(&checksum,sizeof(ULONG),1,fp)) status=0;

   if(!lss_printf(fp, "CSystem::ContextSave")) status=0;

   if(!lss_write(&mCycleCountBreakpoint,sizeof(ULONG),1,fp)) status=0;
   if(!lss_write(&gSystemCycleCount,sizeof(ULONG),1,fp)) status=0;
   if(!lss_write(&gNextTimerEvent,sizeof(ULONG),1,fp)) status=0;
   if(!lss_write(&gCPUWakeupTime,sizeof(ULONG),1,fp)) status=0;
   if(!lss_write(&gCPUBootAddress,sizeof(ULONG),1,fp)) status=0;
   if(!lss_write(&gIRQEntryCycle,sizeof(ULONG),1,fp)) status=0;
   if(!lss_write(&gBreakpointHit,sizeof(ULONG),1,fp)) status=0;
   if(!lss_write(&gSingleStepMode,sizeof(ULONG),1,fp)) status=0;
   if(!lss_write(&gSystemIRQ,sizeof(ULONG),1,fp)) status=0;
   if(!lss_write(&gSystemNMI,sizeof(ULONG),1,fp)) status=0;
   if(!lss_write(&gSystemCPUSleep,sizeof(ULONG),1,fp)) status=0;
   if(!lss_write(&gSystemCPUSleep_Saved,sizeof(ULONG),1,fp)) status=0;
   if(!lss_write(&gSystemHalt,sizeof(ULONG),1,fp)) status=0;
   if(!lss_write(&gThrottleMaxPercentage,sizeof(ULONG),1,fp)) status=0;
   if(!lss_write(&gThrottleLastTimerCount,sizeof(ULONG),1,fp)) status=0;
   if(!lss_write(&gThrottleNextCycleCheckpoint,sizeof(ULONG),1,fp)) status=0;

   if(!lss_write(&gTimerCount,sizeof(ULONG),1,fp)) status=0;

   if(!lss_write(&gAudioLastUpdateCycle,sizeof(ULONG),1,fp)) status=0;

   if(!lss_write(&mMemMapReg,sizeof(UBYTE),1,fp)) status=0;

   // Save other device contexts
   if(!mCart->ContextSave(fp)) status=0;
   if(!mRam->ContextSave(fp)) status=0;
   if(!mMikie->ContextSave(fp)) status=0;
   if(!mSusie->ContextSave(fp)) status=0;
   if(!mCpu->ContextSave(fp)) status=0;
   if(!mEEPROM->ContextSave(fp)) status=0;

   return status;
}

bool CSystem::ContextLoad(LSS_FILE *fp)
{
   bool status=1;

   // fp->index=0;

   char teststr[32];
   // Check identifier
   if(!lss_read(teststr,sizeof(char),4,fp)) status=0;
   teststr[4]=0;

   if(strcmp(teststr,LSS_VERSION)==0) {
      ULONG checksum;
      // Read CRC32 and check against the CART for a match
      lss_read(&checksum,sizeof(ULONG),1,fp);
      if(mCart->CRC32()!=checksum) {
         log_printf("CSystem::ContextLoad() LSS Snapshot CRC does not match the loaded cartridge image...\n");
         // return 0;
      }

      // Check our block header
      if(!lss_read(teststr,sizeof(char),20,fp)) status=0;
      teststr[20]=0;
      if(strcmp(teststr,"CSystem::ContextSave")!=0) status=0;

      if(!lss_read(&mCycleCountBreakpoint,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gSystemCycleCount,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gNextTimerEvent,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gCPUWakeupTime,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gCPUBootAddress,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gIRQEntryCycle,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gBreakpointHit,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gSingleStepMode,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gSystemIRQ,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gSystemNMI,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gSystemCPUSleep,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gSystemCPUSleep_Saved,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gSystemHalt,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gThrottleMaxPercentage,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gThrottleLastTimerCount,sizeof(ULONG),1,fp)) status=0;
      if(!lss_read(&gThrottleNextCycleCheckpoint,sizeof(ULONG),1,fp)) status=0;

      if(!lss_read(&gTimerCount,sizeof(ULONG),1,fp)) status=0;

      if(!lss_read(&gAudioLastUpdateCycle,sizeof(ULONG),1,fp)) status=0;

      if(!lss_read(&mMemMapReg,sizeof(UBYTE),1,fp)) status=0;

      if(!mCart->ContextLoad(fp)) status=0;
      if(!mRam->ContextLoad(fp)) status=0;
      if(!mMikie->ContextLoad(fp)) status=0;
      if(!mSusie->ContextLoad(fp)) status=0;
      if(!mCpu->ContextLoad(fp)) status=0;
      if(!mEEPROM->ContextLoad(fp)) status=0;

      gAudioBufferPointer = 0;
   } else {
      log_printf("CSystem::ContextLoad() Not a recognised LSS file!\n");
   }

   return status;
}

#ifdef _LYNXDBG

void CSystem::DebugTrace(int address)
{
   char message[1024+1];
   int count=0;

   log_printf(message,"%08x - DebugTrace(): ",gSystemCycleCount);
   count=strlen(message);

   if(address) {
      if(address==0xffff) {
         C6502_REGS regs;
         char linetext[1024];
         // Register dump
         mCpu->GetRegs(regs);
         log_printf(linetext,"PC=$%04x SP=$%02x PS=0x%02x A=0x%02x X=0x%02x Y=0x%02x",regs.PC,regs.SP, regs.PS,regs.A,regs.X,regs.Y);
         strcat(message,linetext);
         count=strlen(message);
      } else {
         // The RAM address contents should be dumped to an open debug file in this function
         do {
            message[count++]=Peek_RAM(address);
         } while(count<1024 && Peek_RAM(address++)!=0);
      }
   } else {
      strcat(message,"CPU Breakpoint");
      count=strlen(message);
   }
   message[count]=0;

   // Callback to dump the message
   if(mpDebugCallback) {
      (*mpDebugCallback)(mDebugCallbackObject,message);
   }
}

void CSystem::DebugSetCallback(void (*function)(ULONG objref,char *message),ULONG objref)
{
   mDebugCallbackObject=objref;
   mpDebugCallback=function;
}

#endif
