//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment

This file is part of Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
aint32_t with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "types.h"
#include "develop.h"
#include "util_lib.h"
//#include "_animlib.h"
#include "animlib.h"

//****************************************************************************
//
// GLOBALS
//
//****************************************************************************

//****************************************************************************
//
// LOCALS
//
//****************************************************************************
anim_t * anim=NULL;
static boolean Anim_Started = false;

//****************************************************************************
//
//      CheckAnimStarted ()
//
//****************************************************************************

void CheckAnimStarted ( char  * funcname )
   {
   if (!Anim_Started)
      Error(EXIT_FAILURE, "ANIMLIB_%s: Anim has not been initialized\n",funcname);
   }
//****************************************************************************
//
//      findpage ()
//              - given a frame number return the large page number it resides in
//
//****************************************************************************

uint16 findpage (uint16 framenumber)
   {
   uint16 i;

   CheckAnimStarted ( "findpage" );
   for(i=0; i<anim->lpheader.nLps; i++)
      {
      if
         (
         anim->LpArray[i].baseRecord <= framenumber &&
         anim->LpArray[i].baseRecord + anim->LpArray[i].nRecords > framenumber
         )
         return(i);
      }
   return(i);
   }


//****************************************************************************
//
//      loadpage ()
//      - seek out and load in the large page specified
//
//****************************************************************************

void loadpage (uint16 pagenumber, uint16 *pagepointer)
   {
   int32 size;
   byte * buffer;

   CheckAnimStarted ( "loadpage" );
   buffer = anim->buffer;
   if (anim->curlpnum != pagenumber)
      {
      anim->curlpnum = pagenumber;
      buffer += 0xb00 + (pagenumber*0x10000);
      size = sizeof(lp_descriptor);
      memcpy(&anim->curlp,buffer,size);
      buffer += size + sizeof(uint16);
      memcpy(pagepointer,buffer,anim->curlp.nBytes+(anim->curlp.nRecords*2));
      }
   }


//****************************************************************************
//
//      CPlayRunSkipDump ()
//      - This version of the decompressor is here for portability to non PC's
//
//****************************************************************************

void CPlayRunSkipDump (byte *srcP, byte *dstP)
   {
   int8_t cnt;
   uint16 wordCnt;
   byte pixel;


nextOp:
   cnt = (int8_t ) *srcP++;
   if (cnt > 0)
      goto dump;
   if (cnt == 0)
      goto run;
   cnt -= 0x80;
   if (cnt == 0)
      goto longOp;
/* shortSkip */
   dstP += cnt;                    /* adding 7-bit count to 32-bit pointer */
   goto nextOp;
dump:
   do
      {
      *dstP++ = *srcP++;
      } while (--cnt);
   goto nextOp;
run:
   wordCnt = (byte)*srcP++;                /* 8-bit unsigned count */
   pixel = *srcP++;
   do
      {
      *dstP++ = pixel;
      } while (--wordCnt);

   goto nextOp;
longOp:
   wordCnt = *((uint16 *)srcP);
   srcP += sizeof(uint16);
   if ((int16)wordCnt <= 0)
      goto notLongSkip;       /* Do SIGNED test. */

/* longSkip. */
   dstP += wordCnt;
   goto nextOp;

notLongSkip:
   if (wordCnt == 0)
      goto stop;
   wordCnt -= 0x8000;              /* Remove sign bit. */
   if (wordCnt >= 0x4000)
      goto longRun;

/* longDump. */
   do
      {
      *dstP++ = *srcP++;
      } while (--wordCnt);
   goto nextOp;

longRun:
   wordCnt -= 0x4000;              /* Clear "longRun" bit. */
   pixel = *srcP++;
   do
      {
      *dstP++ = pixel;
      } while (--wordCnt);
   goto nextOp;

stop:   /* all done */
   ;
   }



//****************************************************************************
//
//      renderframe ()
//      - draw the frame sepcified from the large page in the buffer pointed to
//
//****************************************************************************

void renderframe (uint16 framenumber, uint16 *pagepointer)
{
   uint16 offset=0;
   uint16 i;
   uint16 destframe;
   byte *ppointer;

   CheckAnimStarted ( "renderframe" );
   destframe = framenumber - anim->curlp.baseRecord;

   for(i = 0; i < destframe; i++)
      offset += pagepointer[i];
      
   ppointer = (byte *)pagepointer;

   ppointer+=anim->curlp.nRecords*2+offset;
   if(ppointer[1])
      ppointer += (4 + (((uint16 *)ppointer)[1] + (((uint16 *)ppointer)[1] & 1)));
   else
      ppointer+=4;

   CPlayRunSkipDump (ppointer, anim->imagebuffer);
}


//****************************************************************************
//
//      drawframe ()
//      - high level frame draw routine
//
//****************************************************************************

void drawframe (uint16 framenumber)
   {
   CheckAnimStarted ( "drawframe" );
   loadpage(findpage(framenumber), anim->thepage);
   renderframe(framenumber, anim->thepage);
   }


//****************************************************************************
//
//      ANIM_LoadAnim ()
//
//****************************************************************************

void ANIM_LoadAnim (byte * buffer)
   {
   uint16 i;
   int32 size;

   if (!Anim_Started) Anim_Started = true;

   anim->buffer = buffer;
   anim->curlpnum = 0xffff;
   anim->currentframe = -1;
   size = sizeof(lpfileheader);
   memcpy(&anim->lpheader, buffer, size );
   buffer += size+128;
   // load the color palette
   for (i = 0; i < 768; i += 3)
      {
      anim->pal[i+2] = *buffer++;
      anim->pal[i+1] = *buffer++;
      anim->pal[i] = *buffer++;
      buffer++;
      }
        // read in large page descriptors
   size = sizeof(anim->LpArray);
   memcpy(&anim->LpArray,buffer,size);
   }

//****************************************************************************
//
//      ANIM_FreeAnim ()
//
//****************************************************************************

void ANIM_FreeAnim ( void )
   {
   if (Anim_Started)
      {
//      SafeFree(anim);
      Anim_Started = false;
      }
   }

//****************************************************************************
//
//      ANIM_NumFrames ()
//
//****************************************************************************

int32 ANIM_NumFrames ( void )
   {
   CheckAnimStarted ( "NumFrames" );
   return anim->lpheader.nRecords;
   }

//****************************************************************************
//
//      ANIM_DrawFrame ()
//
//****************************************************************************

byte * ANIM_DrawFrame (int32 framenumber)
   {
   int32 cnt;

   CheckAnimStarted ( "DrawFrame" );
   if ((anim->currentframe != -1) && (anim->currentframe<=framenumber))
      {
      for (cnt = anim->currentframe; cnt < framenumber; cnt++)
          drawframe (cnt);
      }
   else
      {
      for (cnt = 0; cnt < framenumber; cnt++)
         drawframe (cnt);
      }
   anim->currentframe = framenumber;
   return anim->imagebuffer;
   }

//****************************************************************************
//
//      ANIM_GetPalette ()
//
//****************************************************************************

byte * ANIM_GetPalette ( void )
   {
   CheckAnimStarted ( "GetPalette" );
   return anim->pal;
   }
