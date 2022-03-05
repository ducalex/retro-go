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
#define RAM_ADDR_MASK			0xffff
#define DEFAULT_RAM_CONTENTS	0xff

#ifndef __min
#define __min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _b : _a; })
#endif

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

      CRam(UBYTE *filememory,ULONG filesize)
      {
         mRamData = (UBYTE*)&STATIC_RAM;
         mFileData = NULL;

         if (filesize >= sizeof(HOME_HEADER)) {
            // Sanity checks on the header
            memcpy(&mFileHeader, filememory, sizeof(HOME_HEADER));
            if(mFileHeader.magic[0]!='B' || mFileHeader.magic[1]!='S' ||
               mFileHeader.magic[2]!='9' || mFileHeader.magic[3]!='3') {
               fprintf(stderr, "Invalid cart.\n");
            } else {
            #ifndef MSB_FIRST
               mFileHeader.load_address = mFileHeader.load_address<<8 | mFileHeader.load_address>>8;
               mFileHeader.size = mFileHeader.size<<8 | mFileHeader.size>>8;
            #endif
               mFileHeader.load_address-=10;
            }

            // Take a copy of the ram data
            mFileSize = filesize;
            mFileData = new UBYTE[mFileSize];
            memcpy(mFileData, filememory, mFileSize);
         }

         Reset();
      }

      ~CRam()
      {
         if (mFileData) delete[] mFileData;
         mFileData=NULL;
      }

      void Reset(void)
      {
         if (mFileData) {
            // Load the cart into RAM
            int data_size = __min(int(mFileHeader.size), (int)(mFileSize));
            memset(mRamData, 0x00, mFileHeader.load_address);
            memcpy(mRamData+mFileHeader.load_address, mFileData, data_size);
            memset(mRamData+mFileHeader.load_address+data_size, 0x00, RAM_SIZE-mFileHeader.load_address-data_size);
            gCPUBootAddress=mFileHeader.load_address;
         } else {
            memset(mRamData, DEFAULT_RAM_CONTENTS, RAM_SIZE);
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
      UBYTE	*mFileData;
      ULONG	mFileSize;
      HOME_HEADER mFileHeader;
};

#endif
