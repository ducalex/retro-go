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
// Lynx Cartridge Class                                                     //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class emulates the Lynx cartridge interface, given a filename it    //
// will contstruct a cartridge object via the constructor.                  //
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

#define CART_CPP

//#include <crtdbg.h>
//#define   TRACE_CART

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "system.h"
#include "cart.h"
//#include "zlib.h"
#include "scrc32.h"

CCart::CCart(UBYTE *gamedata,ULONG gamesize)
{
   int headersize=0
   TRACE_CART1("CCart() called with %s",gamefile);
   LYNX_HEADER	header;

   mWriteEnableBank0=FALSE;
   mWriteEnableBank1=FALSE;
   mCartRAM=FALSE;
   mHeaderLess=0;
   mEEPROMType=0;
   mCRC32=0;
   mCRC32=crc32(mCRC32,gamedata,gamesize);

   // Open up the file

   if(gamesize) {
      // Checkout the header bytes
      memcpy(&header,gamedata,sizeof(LYNX_HEADER));

#ifdef MSB_FIRST
      header.page_size_bank0 = ((header.page_size_bank0>>8) | (header.page_size_bank0<<8));
      header.page_size_bank1 = ((header.page_size_bank1>>8) | (header.page_size_bank1<<8));
      header.version         = ((header.version>>8) | (header.version<<8));
#endif
      // Sanity checks on the header

      if(header.magic[0]!='L' || header.magic[1]!='Y' || header.magic[2]!='N' || header.magic[3]!='X' || header.version!=1) {
        memset(&header,0,sizeof(LYNX_HEADER));
        fprintf(stderr, "Invalid cart (no header?).\nGuessing a ROM layout...\n");
        strncpy((char*)&header.cartname,"NO HEADER",32);
        strncpy((char*)&header.manufname,"HANDY",16);
        header.page_size_bank0=gamesize>>8;// Hard workaround...
      } else {
         headersize=sizeof(LYNX_HEADER);
      }

      // Setup name & manufacturer

      strncpy(mName,(char*)&header.cartname,32);
      strncpy(mManufacturer,(char*)&header.manufname,16);

      // Setup rotation

      mRotation=header.rotation;
      if(mRotation!=CART_NO_ROTATE && mRotation!=CART_ROTATE_LEFT && mRotation!=CART_ROTATE_RIGHT) mRotation=CART_NO_ROTATE;
      mAudinFlag=(header.aud_bits&0x01) ;
      mEEPROMType=header.eeprom;
   } else {
      header.page_size_bank0=0x000;
      header.page_size_bank1=0x000;

      // Setup name & manufacturer

      strcpy(mName,"<No cart loaded>");
      strcpy(mManufacturer,"<No cart loaded>");

      // Setup rotation

      mAudinFlag=false;
      mRotation=CART_NO_ROTATE;
   }


   // Set the filetypes

   CTYPE banktype0,banktype1;

   switch(header.page_size_bank0) {
      case 0x000:
         banktype0=UNUSED;
         mMaskBank0=0;
         mShiftCount0=0;
         mCountMask0=0;
         break;
      case 0x100:
         banktype0=C64K;
         mMaskBank0=0x00ffff;
         mShiftCount0=8;
         mCountMask0=0x0ff;
         break;
      case 0x200:
         banktype0=C128K;
         mMaskBank0=0x01ffff;
         mShiftCount0=9;
         mCountMask0=0x1ff;
         break;
      case 0x400:
         banktype0=C256K;
         mMaskBank0=0x03ffff;
         mShiftCount0=10;
         mCountMask0=0x3ff;
         break;
      case 0x800:
         banktype0=C512K;
         mMaskBank0=0x07ffff;
         mShiftCount0=11;
         mCountMask0=0x7ff;
         break;
      default:
         fprintf(stderr, "Invalid cart (bank0 size).\n");
         break;
   }
   TRACE_CART1("CCart() - Bank0 = $%06x",mMaskBank0);

   switch(header.page_size_bank1) {
      case 0x000:
         banktype1=UNUSED;
         mMaskBank1=0;
         mShiftCount1=0;
         mCountMask1=0;
         break;
      case 0x100:
         banktype1=C64K;
         mMaskBank1=0x00ffff;
         mShiftCount1=8;
         mCountMask1=0x0ff;
         break;
      case 0x200:
         banktype1=C128K;
         mMaskBank1=0x01ffff;
         mShiftCount1=9;
         mCountMask1=0x1ff;
         break;
      case 0x400:
         banktype1=C256K;
         mMaskBank1=0x03ffff;
         mShiftCount1=10;
         mCountMask1=0x3ff;
         break;
      case 0x800:
         banktype1=C512K;
         mMaskBank1=0x07ffff;
         mShiftCount1=11;
         mCountMask1=0x7ff;
         break;
      default:
         fprintf(stderr, "Invalid cart (bank1 size).\n");
         break;
   }
   TRACE_CART1("CCart() - Bank1 = $%06x",mMaskBank1);

   // Make some space for the new carts

   mCartBank0 = (UBYTE*) new UBYTE[mMaskBank0+1];
   mCartBank1 = (UBYTE*) new UBYTE[mMaskBank1+1];
   mCartBank0A = (UBYTE*) new UBYTE[mMaskBank0+1];
   mCartBank1A = (UBYTE*) new UBYTE[mMaskBank1+1];

   // Set default bank

   mBank=bank0;

   // Initialiase

   // TODO: the following code to read the banks is not very nice .. should be reworked
   // TODO: actually its dangerous, if more than one bank is used ... (only homebrews)
   int cartsize = __max(0, int(gamesize - headersize));
   int bank0size = __min(cartsize, (int)(mMaskBank0+1));
   int bank1size = __min(cartsize, (int)(mMaskBank1+1));

   memset(mCartBank0, DEFAULT_CART_CONTENTS, bank0size);
   memset(mCartBank1, DEFAULT_CART_CONTENTS, bank1size);
   memset(mCartBank0A, DEFAULT_CART_CONTENTS, bank0size);
   memset(mCartBank1A, DEFAULT_CART_CONTENTS, bank1size);
   if(bank0size==1) bank0size=0;// workaround ...
   if(bank1size==1) bank1size=0;// workaround ...
   
   memcpy(
         mCartBank0,
         gamedata+(headersize),
         bank0size);
   cartsize = cartsize < bank0size ? 0 : cartsize - bank0size;

   memcpy(
         mCartBank1,
         gamedata+(headersize + bank0size),
         cartsize < bank1size ? cartsize : bank1size);
   cartsize = cartsize < bank1size ? 0 : cartsize - bank1size;

   if(mAudinFlag){// TODO clean up code
      memcpy(
            mCartBank0A,
            gamedata+(headersize+ bank0size + bank1size),
            cartsize < bank0size ? cartsize : bank0size);
      cartsize = cartsize < bank0size ? 0 : cartsize - bank0size;

      memcpy(
            mCartBank1A,
            gamedata+(headersize + bank0size + bank1size + bank0size),
            cartsize < bank1size ? cartsize : bank1size);
      cartsize = cartsize < bank1size ? 0 : cartsize - bank1size;
   }

   if(bank0size==0) bank0size=1;// workaround ...
   if(bank1size==0) bank1size=1;// workaround ...

   // Copy the cart banks from the image
   if(gamesize) {
      // As this is a cartridge boot unset the boot address

      gCPUBootAddress=0;

      //
      // Check if this is a headerless cart, either 410 (EPYX_HEADER_NEW) or 512 (EPYX_HEADER_OLD) zeros
      //
      mHeaderLess=EPYX_HEADER_OLD;// old EPYX Type
      for(int loop=0; loop<EPYX_HEADER_OLD; loop++) {
         if(mCartBank0[loop&mMaskBank0]!=0x00) {
            if(loop<EPYX_HEADER_NEW) {
               mHeaderLess=0;// less than 410 zeros -> invalid
               break;
            } else {
               mHeaderLess=EPYX_HEADER_NEW;// at least 410 -> new EPYX type
               break;
            }
         }
      }
      TRACE_CART1("CCart() - mHeaderLess=%d",mHeaderLess);
   }

   // Dont allow an empty Bank1 - Use it for shadow SRAM/EEPROM
   if(banktype1==UNUSED) {
      // Delete the single byte allocated  earlier
      delete[] mCartBank1;
      // Allocate some new memory for us
      TRACE_CART0("CCart() - Bank1 being converted to 64K SRAM");
      banktype1=C64K;
      mMaskBank1=0x00ffff;
      mShiftCount1=8;
      mCountMask1=0x0ff;
      mCartBank1 = (UBYTE*) new UBYTE[mMaskBank1+1];
      memset(mCartBank1, DEFAULT_RAM_CONTENTS, mMaskBank1+1);
      mWriteEnableBank1=TRUE;
      mCartRAM=TRUE;
   }
}

CCart::~CCart()
{
   TRACE_CART0("~CCart()");
   delete[] mCartBank0;
   delete[] mCartBank1;
   delete[] mCartBank0A;
   delete[] mCartBank1A;
}


void CCart::Reset(void)
{
   TRACE_CART0("Reset()");
   mCounter=0;
   mShifter=0;
   mAddrData=0;
   mStrobe=0;
}

bool CCart::ContextSave(LSS_FILE *fp)
{
   TRACE_CART0("ContextSave()");
   if(!lss_printf(fp,"CCart::ContextSave")) return 0;
   if(!lss_write(&mCounter,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mShifter,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mAddrData,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mStrobe,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mShiftCount0,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mCountMask0,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mShiftCount1,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mCountMask1,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mBank,sizeof(EMMODE),1,fp)) return 0;
   if(!lss_write(&mWriteEnableBank0,sizeof(ULONG),1,fp)) return 0;
   if(!lss_write(&mWriteEnableBank1,sizeof(ULONG),1,fp)) return 0;

   if(!lss_write(&mCartRAM,sizeof(ULONG),1,fp)) return 0;
   if(mCartRAM) {
      if(!lss_write(&mMaskBank1,sizeof(ULONG),1,fp)) return 0;
      if(!lss_write(mCartBank1,sizeof(UBYTE),mMaskBank1+1,fp)) return 0;
   }
   return 1;
}

bool CCart::ContextLoad(LSS_FILE *fp)
{
   TRACE_CART0("ContextLoad()");
   char teststr[100]="XXXXXXXXXXXXXXXXXX";
   if(!lss_read(teststr,sizeof(char),18,fp)) return 0;
   if(strcmp(teststr,"CCart::ContextSave")!=0) return 0;
   if(!lss_read(&mCounter,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mShifter,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAddrData,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mStrobe,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mShiftCount0,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mCountMask0,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mShiftCount1,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mCountMask1,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mBank,sizeof(EMMODE),1,fp)) return 0;
   if(!lss_read(&mWriteEnableBank0,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mWriteEnableBank1,sizeof(ULONG),1,fp)) return 0;

   if(!lss_read(&mCartRAM,sizeof(ULONG),1,fp)) return 0;
   if(mCartRAM) {
      if(!lss_read(&mMaskBank1,sizeof(ULONG),1,fp)) return 0;
      delete[] mCartBank1;
      mCartBank1 = new UBYTE[mMaskBank1+1];
      if(!lss_read(mCartBank1,sizeof(UBYTE),mMaskBank1+1,fp)) return 0;
   }
   return 1;
}

bool CCart::ContextLoadLegacy(LSS_FILE *fp)
{
   TRACE_CART0("ContextLoadLegacy()");
   strcpy(mName,"<** IMAGE **>");
   strcpy(mManufacturer,"<** RESTORED **>");
   char teststr[100]="XXXXXXXXXXXXXXXXXX";
   if(!lss_read(teststr,sizeof(char),18,fp)) return 0;
   if(strcmp(teststr,"CCart::ContextSave")!=0) return 0;
   if(!lss_read(&mRotation,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mHeaderLess,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mCounter,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mShifter,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mAddrData,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mStrobe,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mShiftCount0,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mCountMask0,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mShiftCount1,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mCountMask1,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mBank,sizeof(EMMODE),1,fp)) return 0;

   if(!lss_read(&mMaskBank0,sizeof(ULONG),1,fp)) return 0;
   if(!lss_read(&mMaskBank1,sizeof(ULONG),1,fp)) return 0;

   delete[] mCartBank0;
   delete[] mCartBank1;
   mCartBank0 = new UBYTE[mMaskBank0+1];
   mCartBank1 = new UBYTE[mMaskBank1+1];
   if(!lss_read(mCartBank0,sizeof(UBYTE),mMaskBank0+1,fp)) return 0;
   if(!lss_read(mCartBank1,sizeof(UBYTE),mMaskBank1+1,fp)) return 0;
   return 1;
}

inline void CCart::Poke(ULONG addr, UBYTE data)
{
   if(mBank==bank0) {
      if(mWriteEnableBank0) mCartBank0[addr&mMaskBank0]=data;
   } else {
      if(mWriteEnableBank1) mCartBank1[addr&mMaskBank1]=data;
   }
}


inline UBYTE CCart::Peek(ULONG addr)
{
   if(mBank==bank0) {
      return(mCartBank0[addr&mMaskBank0]);
   } else {
      return(mCartBank1[addr&mMaskBank1]);
   }
}


void CCart::CartAddressStrobe(bool strobe)
{
   static int last_strobe=0;

   mStrobe=strobe;

   if(mStrobe) mCounter=0;

   //
   // Either of the two below seem to work OK.
   //
   // if(!strobe && last_strobe)
   //
   if(mStrobe && !last_strobe) {
      // Clock a bit into the shifter
      mShifter=mShifter<<1;
      mShifter+=mAddrData?1:0;
      mShifter&=0xff;
   }
   last_strobe=mStrobe;
   TRACE_CART2("CartAddressStrobe(strobe=%d) mShifter=$%06x",strobe,mShifter);
}

void CCart::CartAddressData(bool data)
{
   TRACE_CART1("CartAddressData($%02x)",data);
   mAddrData=data;
}


void CCart::Poke0(UBYTE data)
{
   if(mWriteEnableBank0) {
      ULONG address=(mShifter<<mShiftCount0)+(mCounter&mCountMask0);
      mCartBank0[address&mMaskBank0]=data;
   }
   if(!mStrobe) {
      mCounter++;
      mCounter&=0x07ff;
   }
}

void CCart::Poke0A(UBYTE data)
{
	if(mWriteEnableBank0) {
       ULONG address=(mShifter<<mShiftCount0)+(mCounter&mCountMask0);
       mCartBank0A[address&mMaskBank0]=data;		
	}
	if(!mStrobe) {
       mCounter++;
       mCounter&=0x07ff;
	}
}

void CCart::Poke1(UBYTE data)
{
   if(mWriteEnableBank1) {
      ULONG address=(mShifter<<mShiftCount1)+(mCounter&mCountMask1);
      mCartBank1[address&mMaskBank1]=data;
   }
   if(!mStrobe) {
      mCounter++;
      mCounter&=0x07ff;
   }
}

void CCart::Poke1A(UBYTE data)
{
	if(mWriteEnableBank1) {
       ULONG address=(mShifter<<mShiftCount1)+(mCounter&mCountMask1);
       mCartBank1A[address&mMaskBank1]=data;		
	}
	if(!mStrobe) {
       mCounter++;
       mCounter&=0x07ff;
	}
}

UBYTE CCart::Peek0(void)
{
   ULONG address=(mShifter<<mShiftCount0)+(mCounter&mCountMask0);
   UBYTE data=mCartBank0[address&mMaskBank0];

   if(!mStrobe) {
      mCounter++;
      mCounter&=0x07ff;
   }

   return data;
}

UBYTE CCart::Peek0A(void)
{
	ULONG address=(mShifter<<mShiftCount0)+(mCounter&mCountMask0);
	UBYTE data=mCartBank0A[address&mMaskBank0];		

	if(!mStrobe) {
       mCounter++;
       mCounter&=0x07ff;
	}

	return data;
}

UBYTE CCart::Peek1(void)
{
   ULONG address=(mShifter<<mShiftCount1)+(mCounter&mCountMask1);
   UBYTE data=mCartBank1[address&mMaskBank1];

   if(!mStrobe) {
      mCounter++;
      mCounter&=0x07ff;
   }

   return data;
}

UBYTE CCart::Peek1A(void)
{
	ULONG address=(mShifter<<mShiftCount1)+(mCounter&mCountMask1);
	UBYTE data=mCartBank1A[address&mMaskBank1];		

	if(!mStrobe) {
       mCounter++;
       mCounter&=0x07ff;
	}

	return data;
}
