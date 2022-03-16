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
// RAM object header file                                                   //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition for the RAM class     //
// that emulates the Handy system RAM (64K)                                 //
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

#ifndef RAM_H
#define RAM_H

#define RAM_SIZE				   65536

typedef struct
{
   UWORD   jump;
   UWORD   load_address;
   UWORD   size;
   UBYTE   magic[4];
}HOME_HEADER;

static UBYTE STATIC_RAM[RAM_SIZE];

class CRam : public CLynxBase
{
   // Function members

   public:

      CRam(UBYTE *filedata, ULONG filesize)
      {
         mRamData = (UBYTE*)&STATIC_RAM;
         if (filedata && filesize > 64 && memcmp(filedata + 6, "BS93", 4) == 0) {
            #ifdef MSB_FIRST
            mHomebrewAddr = filedata[2] << 8 | filedata[3];
            mHomebrewSize = filedata[4] << 8 | filedata[5];
            #else
            mHomebrewAddr = filedata[3] << 8 | filedata[2];
            mHomebrewSize = filedata[5] << 8 | filedata[4];
            #endif
            mHomebrewSize = filesize > mHomebrewSize ? mHomebrewSize : filesize;
            mHomebrewAddr -= 10;
            mHomebrewData = new UBYTE[mHomebrewSize];
            memcpy(mHomebrewData, filedata, mHomebrewSize);
            log_printf("Homebrew found: size=%d, addr=0x%04X\n", mHomebrewSize, mHomebrewAddr);
         } else {
            mHomebrewData = NULL;
            mHomebrewAddr = 0;
            mHomebrewSize = 0;
         }
         Reset();
      }

      ~CRam()
      {
         if (mHomebrewData) {
            delete[] mHomebrewData;
            mHomebrewData=NULL;
         }
      }

      void Reset(void)
      {
         if (mHomebrewData) {
            // Load the cart into RAM
            memset(mRamData, 0x00, RAM_SIZE);
            memcpy(mRamData+mHomebrewAddr, mHomebrewData, mHomebrewSize);
            gCPUBootAddress = mHomebrewAddr;
         } else {
            memset(mRamData, 0xFF, RAM_SIZE);
         }
      }

      void Clear(void)
      {
         memset(mRamData, 0, RAM_SIZE);
      }

      bool ContextSave(LSS_FILE *fp)
      {
         if(!lss_printf(fp,"CRam::ContextSave")) return 0;
         if(!lss_write(mRamData,sizeof(UBYTE),RAM_SIZE,fp)) return 0;
         return 1;
      }

      bool ContextLoad(LSS_FILE *fp)
      {
         char teststr[32]="XXXXXXXXXXXXXXXXX";
         if(!lss_read(teststr,sizeof(char),17,fp)) return 0;
         if(strcmp(teststr,"CRam::ContextSave")!=0) return 0;
         if(!lss_read(mRamData,sizeof(UBYTE),RAM_SIZE,fp)) return 0;
         return 1;
      }

      void   Poke(ULONG addr, UBYTE data) {mRamData[addr] = data;};
      UBYTE  Peek(ULONG addr) {return mRamData[addr];};
      ULONG  ObjectSize(void) {return RAM_SIZE;};
      UBYTE* GetRamPointer(void) {return mRamData;};

   // Data members

   private:
      UBYTE	*mRamData;
      UBYTE	*mHomebrewData;
      ULONG	mHomebrewSize;
      ULONG mHomebrewAddr;
};

#endif
