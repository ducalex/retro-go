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

/////////////////////////////////////////////////////////////////////////////
//
//      ANIMLIB.H
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _animlib_public_
#define _animlib_public_
#ifdef __cplusplus
extern "C" {
#endif


// structure declarations for deluxe animate large page files */

typedef struct
   {
   uint32 id;                 // 4 character ID == "LPF " */
   uint16 maxLps;                  // max # largePages allowed. 256 FOR NOW.   */
   uint16 nLps;            // # largePages in this file. */
   uint32 nRecords;        // # records in this file.  65534 is current limit plus */
                        // one for last-to-first delta for looping the animation */
   uint16 maxRecsPerLp; // # records permitted in an lp. 256 FOR NOW.   */
   uint16 lpfTableOffset; // Absolute Seek position of lpfTable.  1280 FOR NOW.
                                         // The lpf Table is an array of 256 large page structures
                                         // that is used to facilitate finding records in an anim
                                         // file without having to seek through all of the Large
                                         // Pages to find which one a specific record lives in. */
   uint32 contentType;  // 4 character ID == "ANIM" */
   uint16 width;                   // Width of screen in pixels. */
   uint16 height;                  // Height of screen in pixels. */
   byte variant;              // 0==ANIM. */
   byte version;              // 0==frame rate is multiple of 18 cycles/sec.
                                        // 1==frame rate is multiple of 70 cycles/sec.  */
   byte hasLastDelta;   // 1==Last record is a delta from last-to-first frame. */
   byte lastDeltaValid; // 0==The last-to-first delta (if present) hasn't been
                                  // updated to match the current first&last frames,    so it
                                  // should be ignored. */
   byte pixelType;         //   /* 0==256 color. */
   byte CompressionType;//      /* 1==(RunSkipDump) Only one used FOR NOW. */
   byte otherRecsPerFrm;//      /* 0 FOR NOW. */
   byte bitmaptype;     //   /* 1==320x200, 256-color.  Only one implemented so far. */
   byte recordTypes[32];//      /* Not yet implemented. */
   uint32 nFrames;         //   /* In case future version adds other records at end of
                                //      file, we still know how many actual frames.
                                  //    NOTE: DOES include last-to-first delta when present. */
   uint16 framesPerSecond;      // Number of frames to play per second. */
   uint16 pad2[29];           // 58 bytes of filler to round up to 128 bytes total. */
   } lpfileheader;

// this is the format of a large page structure
typedef struct
   {
   uint16 baseRecord;   // Number of first record in this large page.
   uint16 nRecords;        // Number of records in lp.
                                                      // bit 15 of "nRecords" == "has continuation from previous lp".
                                              // bit 14 of "nRecords" == "final record continues on next lp".
   uint16 nBytes;                  // Total number of bytes of contents, excluding header.
   } lp_descriptor;

typedef struct
   {
   uint16 framecount;          // current frame of anim
   lpfileheader lpheader;           // file header will be loaded into this structure
   lp_descriptor LpArray[256]; // arrays of large page structs used to find frames
   uint16 curlpnum;               // initialize to an invalid Large page number
   lp_descriptor curlp;        // header of large page currently in memory
   uint16 thepage[0x8000];     // buffer where current large page is loaded
   byte imagebuffer[0x10000]; // buffer where anim frame is decoded
   byte * buffer;
   byte pal[768];
   int32  currentframe;
  } anim_t;

//****************************************************************************
//
//      ANIM_LoadAnim ()
//
// Setup internal anim data structure
//
//****************************************************************************

void ANIM_LoadAnim (byte * buffer);

//****************************************************************************
//
//      ANIM_FreeAnim ()
//
// Free up internal anim data structure
//
//****************************************************************************

void ANIM_FreeAnim ( void );

//****************************************************************************
//
//      ANIM_NumFrames ()
//
// returns the number of frames in the current anim
//
//****************************************************************************

int32 ANIM_NumFrames ( void );

//****************************************************************************
//
//      ANIM_DrawFrame ()
//
// Draw the frame to a returned buffer
//
//****************************************************************************

byte * ANIM_DrawFrame (int32 framenumber);

//****************************************************************************
//
//      ANIM_GetPalette ()
//
// return the palette of the anim
//****************************************************************************

byte * ANIM_GetPalette ( void );

extern anim_t * anim;

#ifdef __cplusplus
};
#endif
#endif
