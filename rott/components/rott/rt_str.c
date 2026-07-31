/*
Copyright (C) 1994-1995 Apogee Software, Ltd.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
//******************************************************************************
//
// RT_STR.C
//    Contains the menu stuff!
//
//******************************************************************************

#include "rt_def.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <string.h>
#include <ctype.h>
#include "rt_menu.h"
#include "rt_util.h"
#include "rt_vid.h"
#include "rt_build.h"
#include "lumpy.h"
#include "rt_str.h"
#include "_rt_str.h"
#include "isr.h"
#include "rt_in.h"
#include "rt_menu.h"
#include "rt_view.h"
#include "w_wad.h"
#include "z_zone.h"
#include "modexlib.h"
#include "rt_main.h"
#include "rt_msg.h"
#include "rt_playr.h"
#include "rt_sound.h"
#include "myprint.h"
//MED
#include "memcheck.h"


//******************************************************************************
//
// GLOBALS
//
//******************************************************************************

int fontcolor;

//******************************************************************************
//
// LOCALS
//
//******************************************************************************

static int BKw;
static int BKh;
   
static char strbuf[MaxString];

//******************************************************************************
//
// VW_DrawClippedString ()
//
// Draws a string at x, y to bufferofs
//
//******************************************************************************

void VW_DrawClippedString (int x, int y, const char *string)
{
   int   width,height,ht;
   byte  *source;
   int   ch;
   int   oy;

   ht = CurrentFont->height;

   oy=y;

   while ((ch = (unsigned char)*string++)!=0)
      {
      ch -= 31;
      width = CurrentFont->width[ch];
      source = ((byte *)CurrentFont)+CurrentFont->charofs[ch];
      while (width--)
         {
         if ((x>=0) && (x<iGLOBAL_SCREENWIDTH))
            {
            y=oy;
            VGAWRITEMAP(x&3);
            height = ht;
            while (height--)
               {
               if ((y>=0) && (y<iGLOBAL_SCREENHEIGHT))
                  {
                  if (*source>0)
#ifdef DOS
                     *((byte *)(bufferofs+ylookup[y]+(x>>2))) = *source;
#else
                     *((byte *)(bufferofs+ylookup[y]+x)) = *source;
#endif
                  }
               source++;
               y++;
               }
            }
         x++;
         }
      }
}

//******************************************************************************
//
// US_ClippedPrint() - Prints a string in bufferofs. Newlines are supported.
//
//******************************************************************************

void US_ClippedPrint (int x, int y, const char *string)
{
   char  c,
         *se;
   char  *s;
   int   startx;

   strcpy(strbuf, string);
   s = strbuf;
   
   startx=x;
   while (*s)
   {
      se = s;
      while ((c = *se) && (c != '\n'))
         se++;
      *se = '\0';

      VW_DrawClippedString ( x, y, s);

      s = se;
      if (c)
         {
         *se = c;
         s++;
         y += CurrentFont->height;
         }
   }
}


//******************************************************************************
//
// VW_DrawPropString ()
//
// Draws a string at px, py to bufferofs
//
//******************************************************************************

void VW_DrawPropString (const char *string)
{
#ifdef DOS
   byte  pix;
   int   width,step,height,ht;
   byte  *source, *dest, *origdest;
   int   ch,mask;


   ht = CurrentFont->height;
   dest = origdest = (byte *)(bufferofs+ylookup[py]+(px>>2));


   mask = 1<<(px&3);


   while ((ch = *string++)!=0)
   {
      ch -= 31;
      width = step = CurrentFont->width[ch];
      source = ((byte *)CurrentFont)+CurrentFont->charofs[ch];
      while (width--)
      {
         VGAMAPMASK(mask);

         height = ht;
         while (height--)
         {
            pix = *source;
            if (pix)
               *dest = pix;

            source++;
            dest += linewidth;
         }

         px++;
         mask <<= 1;
         if (mask == 16)
         {
            mask = 1;
            origdest++;
         }
         dest = origdest;
      }
   }
   bufferheight = ht;
   bufferwidth = ((dest+1)-origdest)*4;
#else
   byte  pix;
   int   width,step,height,ht;
   byte  *source, *dest, *origdest;
   int   ch,mask;

   ht = CurrentFont->height;
   dest = origdest = (byte *)(bufferofs+ylookup[py]+px);

   while ((ch = (unsigned char)*string++)!=0)
   {
      ch -= 31;
      width = step = CurrentFont->width[ch];
      source = ((byte *)CurrentFont)+CurrentFont->charofs[ch];
      while (width--)
      {
         height = ht;
         while (height--)
         {
            pix = *source;
            if (pix)
               *dest = pix;

            source++;
            dest += linewidth;
         }

         px++;
	 origdest++;
         dest = origdest;
      }
   }
   bufferheight = ht;
   bufferwidth = ((dest+1)-origdest);
#endif
}



//******************************************************************************
//
// VWB_DrawPropString ()
//
// Calls VW_DrawPropString then updates the mark block.
//
//******************************************************************************

void VWB_DrawPropString  (const char *string)
{
   int x;
   x = px;
   VW_DrawPropString (string);
   VW_MarkUpdateBlock (x, py, px-1, py+bufferheight-1);
}



//******************************************************************************
//
// VW_DrawIPropString ()
//
// Draws a string at px, py to bufferofs
//
//******************************************************************************

void VW_DrawIPropString (const char *string)
{
   byte  pix;
   int   width,step,height,ht;
   byte  *source, *dest, *origdest;
   int   ch,mask;


   ht = CurrentFont->height;
#ifdef DOS
   dest = origdest = (byte *)(bufferofs+ylookup[py]+(px>>2));
#else
   dest = origdest = (byte *)(bufferofs+ylookup[py]+px);
#endif


   mask = 1<<(px&3);


   while ((ch = (unsigned char)*string++)!=0)
   {
      ch -= 31;
      width = step = CurrentFont->width[ch];
      source = ((byte *)CurrentFont)+CurrentFont->charofs[ch];
      while (width--)
      {
         VGAMAPMASK(mask);

         height = ht;
         while (height--)
         {
            pix = *source;
            if (pix)
               *dest = pix;

            source++;
            dest += linewidth;
         }

         px++;
#ifdef DOS
         mask <<= 1;
         if (mask == 16)
         {
            mask = 1;
            origdest++;
         }
#else
	 origdest++;
#endif
         dest = origdest;
      }
   }
   bufferheight = ht;
#ifdef DOS
   bufferwidth = ((dest+1)-origdest)*4;
#else
   bufferwidth = ((dest+1)-origdest);
#endif

}



//******************************************************************************
//
// VWB_DrawIPropString ()
//
// Calls VW_DrawIPropString then updates the mark block.
//
//******************************************************************************

void VWB_DrawIPropString  (const char *string)
{
   int x;
   x = px;
   VW_DrawIPropString (string);
   VW_MarkUpdateBlock (x, py, px-1, py+bufferheight-1);
}



//******************************************************************************
//
// VWL_MeasureString ()
//
//******************************************************************************

void VWL_MeasureString (const char *s, int *width, int *height, const font_t *font)
{
   *height = font->height;

   for (*width = 0; *s; s++)
      *width += font->width[(*((byte *)s))-31];   // proportional width
}

//******************************************************************************
//
// VWL_MeasureIntensityString ()
//
//******************************************************************************

void VWL_MeasureIntensityString (const char *s, int *width, int *height, const cfont_t *font)
{
   *height = font->height;

   for (*width = 0; *s; s++)
      *width += font->width[(*((byte *)s))-31];   // proportional width
}

//******************************************************************************
//
// VW_MeasureIntensityPropString ()
//
//******************************************************************************

void VW_MeasureIntensityPropString (const char *string, int *width, int *height)
{
   VWL_MeasureIntensityString (string, width, height, IFont);
}

//******************************************************************************
//
// VW_MeasurePropString ()
//
//******************************************************************************

void VW_MeasurePropString (const char *string, int *width, int *height)
{
   VWL_MeasureString (string, width, height, CurrentFont);
}


//******************************************************************************
//
// US_MeasureStr ()
//
//******************************************************************************

void US_MeasureStr (int *width, int *height, const char * s, ...)
{
   char  c,
         *se,
         *ss;
   int   w,h;
   va_list strptr;
   char buf[300];

   *width  = 0;
   *height = 0;

   memset (&buf[0], 0, sizeof (buf));
   va_start (strptr, s);
   vsprintf (&buf[0], s, strptr);
   va_end (strptr);

   ss = &buf[0];

   while (*ss)
   {
      se = ss;
      while ((c = *se) && (c != '\n'))
         se++;
      *se = '\0';

      VWL_MeasureString (ss, &w, &h, CurrentFont);

      *height += h;

      if (w > *width)
         *width = w;

      ss = se;
      if (c)
      {
         *se = c;
         ss++;
      }
   }
}


//******************************************************************************
//
// US_SetPrintRoutines() - Sets the routines used to measure and print
//    from within the User Mgr. Primarily provided to allow switching
//    between masked and non-masked fonts
//
//******************************************************************************

void US_SetPrintRoutines (void (*measure)(const char *, int *, int *, font_t *),
                          void (*print)(const char *))
{
   USL_MeasureString = measure;
   USL_DrawString    = print;
}


//******************************************************************************
//
// US_Print() - Prints a string in the current window. Newlines are
//    supported.
//
//******************************************************************************

void US_Print (const char *string)
{
   char  c,
         *se,
         *s;
   int   w,h;

   strcpy(strbuf, string);
   s = strbuf;
   
   while (*s)
   {
      se = s;
      while ((c = *se) && (c != '\n'))
         se++;
      *se = '\0';

      USL_MeasureString (s, &w, &h, CurrentFont);
      px = PrintX;
      py = PrintY;
      USL_DrawString (s);

      s = se;
      if (c)
      {
         *se = c;
         s++;

         PrintX = WindowX;
         PrintY += h;
      }
      else
         PrintX += w;
   }
}

//******************************************************************************
//
// US_BufPrint() - Prints a string in bufferofs. Newlines are supported.
//
//******************************************************************************

void US_BufPrint (const char *string)
{
   char  c,
         *se,
         *s;
   int   startx;

   strcpy(strbuf, string);
   s = strbuf;

   startx=PrintX;
   while (*s)
   {
      se = s;
      while ((c = *se) && (c != '\n'))
         se++;
      *se = '\0';

      px = PrintX;
      py = PrintY;
      USL_DrawString (s);

      PrintY = py;
      PrintX = px;

      s = se;
      if (c)
         {
         *se = c;
         s++;
         PrintY += CurrentFont->height;
         PrintX = startx;
         }
   }
}


//******************************************************************************
//
// US_PrintUnsigned () - Prints an unsigned long int
//
//******************************************************************************

void US_PrintUnsigned (unsigned long int n)
{
   char  buffer[32];

   US_Print (ultoa (n, buffer, 10));
}

//******************************************************************************
//
// US_PrintSigned() - Prints a signed long
//
//******************************************************************************

void US_PrintSigned (long int n)
{
   char  buffer[32];

   US_Print (ltoa (n, buffer, 10));
}

//******************************************************************************
//
// USL_PrintInCenter() - Prints a string in the center of the given rect
//
//******************************************************************************

void USL_PrintInCenter (const char *s, Rect r)
{
   int   w,h,
         rw,rh;

   USL_MeasureString (s,&w,&h, CurrentFont);
   rw = r.lr.x - r.ul.x;
   rh = r.lr.y - r.ul.y;

   px = r.ul.x + ((rw - w) / 2);
   py = r.ul.y + ((rh - h) / 2);
   USL_DrawString (s);
}

//******************************************************************************
//
// US_PrintCentered() - Prints a string centered in the current window.
//
//******************************************************************************

void US_PrintCentered (const char *s)
{
   Rect  r;

   r.ul.x = WindowX;
   r.ul.y = WindowY;
   r.lr.x = r.ul.x + WindowW;
   r.lr.y = r.ul.y + WindowH;

   USL_PrintInCenter (s, r);
}

//******************************************************************************
//
// US_CPrintLine() - Prints a string centered on the current line and
//    advances to the next line. Newlines are not supported.
//
//******************************************************************************

void US_CPrintLine (const char *s)
{
   int w, h;

   USL_MeasureString (s, &w, &h, CurrentFont);

   if (w > WindowW)
      Error("US_CPrintLine() - String exceeds width");

   px = WindowX + ((WindowW - w) / 2);
   py = PrintY;
   USL_DrawString (s);
   PrintY += h;
}

//******************************************************************************
//
// US_CPrint() - Prints a string in the current window. Newlines are
//    supported.
//
//******************************************************************************

void US_CPrint (const char *string)
{
   char  c,
         *se,
         *s;

   strcpy(strbuf, string);
   s = strbuf;

   while (*s)
   {
      se = s;
      while ((c = *se) && (c != '\n'))
         se++;
      *se = '\0';

      US_CPrintLine (s);

      s = se;
      if (c)
      {
         *se = c;
         s++;
      }
   }
}



//
//
// Text Input routines
//
//
//



//******************************************************************************
//
// USL_XORICursor() - XORs the I-bar text cursor. Used by  US_LineInput()
//
//******************************************************************************

static void USL_XORICursor (int x, int y, const char *s, int cursor, int color)
{
   static   boolean  status;     // VGA doesn't XOR...
   char     buf[MaxString];

   int      w,h;
   int      oldx = px;
   int      oldy = py;

   strcpy (buf,s);
   buf[cursor] = '\0';
   USL_MeasureString (buf, &w, &h, CurrentFont);


   if (status^=1)
   {
      px = x + w;
      py = y;
      if (color)
         USL_DrawString ("\x80");
      else
         DrawMenuBufPropString (px, py, "\x80");
   }
   else
   {
      if (color)
      {
         VWB_Bar (px, py, BKw, BKh, color);
         USL_DrawString (s);
      }
      else
      {
         EraseMenuBufRegion (px, py, BKw, BKh);
//         EraseMenuBufRegion (px, py+1, BKw, BKh-2);
         DrawMenuBufPropString (px, py, s);
      }
   }
   px = oldx;
   py = oldy;
}






//******************************************************************************
//
// US_LineInput() - Gets a line of user input at (x,y), the string defaults
//    to whatever is pointed at by def. Input is restricted to maxchars
//    chars or maxwidth pixels wide. If the user hits escape (and escok is
//    true), nothing is copied into buf, and false is returned. If the
//    user hits return, the current string is copied into buf, and true is
//    returned
//
///******************************************************************************

extern byte * IN_GetScanName (ScanCode scan);

boolean US_LineInput (int x, int y, char *buf, const char *def, boolean escok,
                      int maxchars, int maxwidth, int color)
{
   boolean  redraw,
            cursorvis,
            cursormoved,
            done,
            result;
   char     s[MaxString],
            olds[MaxString];
   int      i,
            cursor,
            w,h,
            len;

   int      lasttime;


   int      lastkey;
   int      cursorwidth;

   cursorwidth = CurrentFont->width[80-31];

   memset (s, 0, MaxString);
   memset (olds, 0, MaxString);
   IN_ClearKeyboardQueue ();

   BKw = maxwidth;
   BKh = CurrentFont->height;


   if (def)
      strcpy (s, def);
   else
      *s = '\0';

   *olds = '\0';

   cursor      = strlen (s);
   cursormoved = redraw = true;
   cursorvis   = done   = false;

   lasttime  = GetTicCount();


   lastkey = getASCII ();

   while (!done)
   {
//      if (GameEscaped==true)
//         PauseLoop ();

      IN_PumpEvents();

      if (cursorvis)
         USL_XORICursor (x, y, s, cursor, color);

      LastScan = IN_InputUpdateKeyboard ();
      if (Keyboard[sc_LShift] || Keyboard[sc_RShift])
         lastkey = ShiftNames[LastScan];
      else
         lastkey = ASCIINames[LastScan];


      switch (LastScan)
      {
      case sc_LeftArrow:

         if (cursor)
            {
            cursor--;
            cursormoved = true;
            MN_PlayMenuSnd (SD_MOVECURSORSND);
            }
         lastkey = key_None;
         Keyboard[sc_LeftArrow] = 0;
         break;

      case sc_RightArrow:

         if (s[cursor])
            {
            cursor++;
            cursormoved = true;
            MN_PlayMenuSnd (SD_MOVECURSORSND);
            }
         lastkey = key_None;
         Keyboard[sc_RightArrow] = 0;
         break;

      case sc_Home:

         if ( cursor )
            {
            cursor = 0;
            cursormoved = true;
            MN_PlayMenuSnd (SD_MOVECURSORSND);
            }
         Keyboard[sc_Home] = 0;
         lastkey = key_None;
         break;

      case sc_End:

         if ( cursor != (int)strlen (s) )
            {
            cursor = strlen (s);
            cursormoved = true;
            MN_PlayMenuSnd (SD_MOVECURSORSND);
            }
         lastkey = key_None;
         Keyboard[sc_End] = 0;
         break;


      case sc_Return:
         strcpy (buf,s);
         done = true;
         result = true;
         lastkey = key_None;
         MN_PlayMenuSnd (SD_SELECTSND);
         break;

      case sc_Escape:
         if (escok)
         {
            done = true;
            result = false;
            MN_PlayMenuSnd (SD_ESCPRESSEDSND);
         }
         lastkey = key_None;
         break;

      case sc_BackSpace:

         if (cursor)
            {
            strcpy (s + cursor - 1,s + cursor);
            cursor--;
            redraw = true;
            cursormoved = true;
            MN_PlayMenuSnd (SD_MOVECURSORSND);
            }
         lastkey = key_None;
         Keyboard[sc_BackSpace] = 0;
         IN_ClearKeyboardQueue ();
         break;

      case sc_Delete:

         if (s[cursor])
         {
            memmove (s + cursor, s + cursor + 1, strlen(s + cursor + 1) + 1);
            redraw = true;
            cursormoved = true;
            MN_PlayMenuSnd (SD_MOVECURSORSND);
         }
         lastkey = key_None;
         Keyboard[sc_Delete] = 0;
         IN_ClearKeyboardQueue ();
         break;

      case 0x4c:  // Keypad 5
      case sc_UpArrow:
      case sc_DownArrow:
      case sc_PgUp:
      case sc_PgDn:
      case sc_Insert:
         lastkey = key_None;
         break;
      }

//      if (GameEscaped==true)
//         PauseLoop ();

      if (lastkey)
      {
         len = strlen (s);
         USL_MeasureString (s, &w, &h, CurrentFont);

         if
         (
            isprint(lastkey)
         && (len < MaxString - 1)
         && ((!maxchars) || (len < maxchars))
         && ((!maxwidth) || ((w+2) < (maxwidth-cursorwidth-2)))
         )
         {
            int ls;
            int rs;

            for (i = len + 1;i > cursor;i--)
               s[i] = s[i - 1];
            s[cursor++] = lastkey;
            redraw = true;

            ls = Keyboard[sc_LShift];
            rs = Keyboard[sc_RShift];
            memset ((void*)Keyboard, 0, 127*sizeof(int));       // Clear printable keys
            Keyboard[sc_LShift] = ls;
            Keyboard[sc_RShift] = rs;

            MN_PlayMenuSnd (SD_MOVECURSORSND);
         }
      }

//      if (GameEscaped==true)
//         PauseLoop ();

      if (redraw)
      {
         if (color)
            VWB_Bar (x, y, BKw, BKh, color);
         else
            EraseMenuBufRegion (x, y, BKw, BKh);

         strcpy (olds, s);

         px = x;
         py = y;
         if (color)
            USL_DrawString (s);
         else
            DrawMenuBufPropString (px, py, s);
         px = x;
         py = y;

         redraw = false;
      }

      if (cursormoved)
      {
         cursorvis = false;
         lasttime = GetTicCount() - VBLCOUNTER;

         cursormoved = false;
      }
      if (GetTicCount() - lasttime > VBLCOUNTER / 2)
      {
         lasttime = GetTicCount();

         cursorvis ^= true;
      }
      if (cursorvis)
         USL_XORICursor (x, y, s, cursor, color);

//      if (GameEscaped==true)
//         PauseLoop ();

      if (color)
         VW_UpdateScreen ();
      else
         RefreshMenuBuf (0);
   }

   if (cursorvis)
      USL_XORICursor (x, y, s, cursor, color);

   if (!result)
   {
      px = x;
      py = y;
      if (color)
         USL_DrawString (olds);
      else
         DrawMenuBufPropString (px, py, olds);
   }

//   if (GameEscaped==true)
//      PauseLoop ();

   if (color)
      VW_UpdateScreen ();
   else
      RefreshMenuBuf (0);

   IN_ClearKeyboardQueue ();
   return (result);
}


//******************************************************************************
//
// US_lineinput() - Gets a line of user input at (x,y), the string defaults
//    to whatever is pointed at by def. Input is restricted to maxchars
//    chars or maxwidth pixels wide. If the user hits escape (and escok is
//    true), nothing is copied into buf, and false is returned. If the
//    user hits return, the current string is copied into buf, and true is
//    returned - PASSWORD INPUT
//
///******************************************************************************

boolean US_lineinput (int x, int y, char *buf, const char *def, boolean escok,
                      int maxchars, int maxwidth, int color)
{
   boolean  redraw,
            cursorvis,
            cursormoved,
            done,
            result;
   char     s[MaxString],
            xx[MaxString],
            olds[MaxString];
   int      i,
            cursor,
            w,h,
            len;

   int      lasttime;


   int      lastkey;
   int      cursorwidth;

   cursorwidth = CurrentFont->width[80-31];

   memset (s, 0, MaxString);
   memset (xx, 0, MaxString);
   memset (olds, 0, MaxString);
   IN_ClearKeyboardQueue ();

   BKw = maxwidth;
   BKh = CurrentFont->height;


   if (def)
      strcpy (s, def);
   else
      *s = '\0';

   *olds = '\0';

   cursor      = strlen (s);
   cursormoved = redraw = true;
   cursorvis   = done   = false;

   lasttime  = GetTicCount();


   lastkey = getASCII ();

   while (!done)
   {
//      if (GameEscaped == true)
//         PauseLoop ();

      IN_PumpEvents();
      
      if (cursorvis)
         USL_XORICursor (x, y, xx, cursor, color);

      LastScan = IN_InputUpdateKeyboard ();
      if (Keyboard[sc_LShift] || Keyboard[sc_RShift])
         lastkey = ShiftNames[LastScan];
      else
         lastkey = ASCIINames[LastScan];


      switch (LastScan)
      {
      case sc_LeftArrow:

         if (cursor)
            {
            cursor--;
            cursormoved = true;
            MN_PlayMenuSnd (SD_MOVECURSORSND);
            }
         lastkey = key_None;
         Keyboard[sc_LeftArrow] = 0;
         break;

      case sc_RightArrow:

         if (s[cursor])
            {
            cursor++;
            cursormoved = true;
            MN_PlayMenuSnd (SD_MOVECURSORSND);
            }
         lastkey = key_None;
         Keyboard[sc_RightArrow] = 0;
         break;

      case sc_Home:

         if ( cursor != 0 )
            {
            cursor = 0;
            cursormoved = true;
            MN_PlayMenuSnd (SD_MOVECURSORSND);
            }
         Keyboard[sc_Home] = 0;
         lastkey = key_None;
         break;

      case sc_End:

         if ( cursor != (int)strlen( s ) )
            {
            cursor = strlen (s);
            cursormoved = true;
            MN_PlayMenuSnd (SD_MOVECURSORSND);
            }
         lastkey = key_None;
         Keyboard[sc_End] = 0;
         break;

      case sc_Return:
         strcpy (buf,s);
         done = true;
         result = true;
         lastkey = key_None;
         MN_PlayMenuSnd (SD_SELECTSND);
         break;

      case sc_Escape:
         if (escok)
         {
            done = true;
            result = false;
            MN_PlayMenuSnd (SD_ESCPRESSEDSND);
         }
         lastkey = key_None;
         break;

      case sc_BackSpace:

         if (cursor)
            {
            strcpy (s + cursor - 1,s + cursor);
            strcpy (xx + cursor - 1,xx + cursor);
            cursor--;
            redraw = true;
            MN_PlayMenuSnd (SD_MOVECURSORSND);
            cursormoved = true;
            }
         lastkey = key_None;
         Keyboard[sc_BackSpace] = 0;
         IN_ClearKeyboardQueue ();
         break;

      case sc_Delete:

         if (s[cursor])
            {
            memmove (s + cursor, s + cursor + 1, strlen(s + cursor + 1) + 1);
            memmove (xx + cursor, xx + cursor + 1, strlen(xx + cursor + 1) + 1);
            redraw = true;
            cursormoved = true;
            MN_PlayMenuSnd (SD_MOVECURSORSND);
            }
         lastkey = key_None;
         Keyboard[sc_Delete] = 0;
         IN_ClearKeyboardQueue ();
         break;

      case 0x4c:  // Keypad 5
      case sc_UpArrow:
      case sc_DownArrow:
      case sc_PgUp:
      case sc_PgDn:
      case sc_Insert:
         lastkey = key_None;
         break;
      }

//      if (GameEscaped==true)
//         PauseLoop ();

      if (lastkey)
      {
         len = strlen (s);
         USL_MeasureString (xx, &w, &h, CurrentFont);

         if
         (
            isprint(lastkey)
         && (len < MaxString - 1)
         && ((!maxchars) || (len < maxchars))
         && ((!maxwidth) || ((w+2) < (maxwidth-cursorwidth-2)))
         )
         {
            int ls;
            int rs;

            for (i = len + 1;i > cursor;i--)
               s[i] = s[i - 1];
            s[cursor]   = lastkey;
            xx[cursor++] = '*';
            redraw = true;

            ls = Keyboard[sc_LShift];
            rs = Keyboard[sc_RShift];
            memset ((void*)Keyboard, 0, 127*sizeof(int));       // Clear printable keys
            Keyboard[sc_LShift] = ls;
            Keyboard[sc_RShift] = rs;
            MN_PlayMenuSnd (SD_MOVECURSORSND);
         }
      }

//      if (GameEscaped==true)
//         PauseLoop ();

      if (redraw)
      {
         if (color)
            VWB_Bar (x, y, BKw, BKh, color);
         else
            EraseMenuBufRegion (x, y, BKw, BKh);

         strcpy (olds, s);

         px = x;
         py = y;
         if (color)
            USL_DrawString (xx);
         else
            DrawMenuBufPropString (px, py, xx);
         px = x;
         py = y;

         redraw = false;
      }

      if (cursormoved)
      {
         cursorvis = false;
         lasttime = GetTicCount() - VBLCOUNTER;

         cursormoved = false;
      }
      if (GetTicCount() - lasttime > VBLCOUNTER / 2)
      {
         lasttime = GetTicCount();

         cursorvis ^= true;
      }
      if (cursorvis)
         USL_XORICursor (x, y, xx, cursor, color);

      if (color)
         VW_UpdateScreen ();
      else
         RefreshMenuBuf (0);
   }

   if (cursorvis)
      USL_XORICursor (x, y, xx, cursor, color);

   if (!result)
   {
      px = x;
      py = y;
      if (color)
         USL_DrawString (xx);
      else
         DrawMenuBufPropString (px, py, xx);
   }

//   if (GameEscaped==true)
//      PauseLoop ();

   if (color)
      VW_UpdateScreen ();
   else
      RefreshMenuBuf (0);

   IN_ClearKeyboardQueue ();
   return (result);
}



//******************************************************************************
//******************************************************************************
//
// WINDOWING ROUTINES
//
//******************************************************************************
//******************************************************************************


//******************************************************************************
//
// US_ClearWindow() - Clears the current window to white and homes the
//    cursor
//
//******************************************************************************

void US_ClearWindow (void)
{
   VWB_Bar (WindowX, WindowY, WindowW, WindowH, 13);
   PrintX = WindowX;
   PrintY = WindowY;
}




//******************************************************************************
//
// US_DrawWindow() - Draws a frame and sets the current window parms
//
//******************************************************************************

void US_DrawWindow (int x, int y, int w, int h)
{
   int  i,
        sx,
        sy,
        sw,
        sh;
   byte * shape;

   pic_t *Win1;
   pic_t *Win2;
   pic_t *Win3;
   pic_t *Win4;
   pic_t *Win5;
   pic_t *Win6;
   pic_t *Win7;
   pic_t *Win8;
   pic_t *Win9;

	// Cache in windowing shapes
	shape = W_CacheLumpNum (W_GetNumForName ("window1"), PU_CACHE, Cvt_pic_t, 1);
	Win1 = (pic_t *) shape;
	shape = W_CacheLumpNum (W_GetNumForName ("window2"), PU_CACHE, Cvt_pic_t, 1);
	Win2 = (pic_t *) shape;
	shape = W_CacheLumpNum (W_GetNumForName ("window3"), PU_CACHE, Cvt_pic_t, 1);
	Win3 = (pic_t *) shape;
	shape = W_CacheLumpNum (W_GetNumForName ("window4"), PU_CACHE, Cvt_pic_t, 1);
	Win4 = (pic_t *) shape;
	shape = W_CacheLumpNum (W_GetNumForName ("window5"), PU_CACHE, Cvt_pic_t, 1);
	Win5 = (pic_t *) shape;
	shape = W_CacheLumpNum (W_GetNumForName ("window6"), PU_CACHE, Cvt_pic_t, 1);
	Win6 = (pic_t *) shape;
	shape = W_CacheLumpNum (W_GetNumForName ("window7"), PU_CACHE, Cvt_pic_t, 1);
	Win7 = (pic_t *) shape;
	shape = W_CacheLumpNum (W_GetNumForName ("window8"), PU_CACHE, Cvt_pic_t, 1);
	Win8 = (pic_t *) shape;
	shape = W_CacheLumpNum (W_GetNumForName ("window9"), PU_CACHE, Cvt_pic_t, 1);
	Win9 = (pic_t *) shape;

   WindowX = x * 8;
   WindowY = y * 8;
   WindowW = w * 8;
   WindowH = h * 8;

   PrintX = WindowX;
   PrintY = WindowY;

   sx = (x - 1) * 8;
   sy = (y - 1) * 8;
   sw = (w + 1) * 8;
   sh = (h + 1) * 8;

   US_ClearWindow ();


   VWB_DrawPic (sx, sy, Win1);

   VWB_DrawPic (sx, sy + sh, Win7);

   for (i = sx + 8;i <= sx + sw - 8; i += 8)
   {
      VWB_DrawPic (i, sy, Win2);
      VWB_DrawPic (i, sy + sh, Win8);
   }

   VWB_DrawPic (i, sy, Win3);
   VWB_DrawPic (i, sy + sh, Win9);

   for (i = sy + 8;i <= sy + sh - 8; i += 8)
   {
      VWB_DrawPic (sx, i, Win4);
      VWB_DrawPic (sx + sw, i, Win6);
   }
}



//******************************************************************************
//
// US_CenterWindow() - Generates a window of a given width & height in the
//    middle of the screen
//
//******************************************************************************

void US_CenterWindow (int w, int h)
{
   //HDG US_DrawWindow (((MaxX / 8) - w) / 2,  ((MaxY / 8) - h) / 2, w, h);
   US_DrawWindow (((iGLOBAL_SCREENWIDTH / 8) - w) / 2,((iGLOBAL_SCREENHEIGHT / 8) - h) / 2, w, h);
}



//==============================================================================
//
// Intensity Font stuff
//
// TEXT FORMATTING COMMANDS - (Use EGA colors ONLY!)
// -------------------------------------------------
// /<hex digit> - Change the following word to <hex digit> color
// `            - Highlights the following word with lighter color of fontcolor
// /N<hex digit> - Change the fontcolor to a certain color
//
//==============================================================================

//******************************************************************************
//
// GetIntensityColor ()
//
//******************************************************************************

byte GetIntensityColor (byte pix)
{
   if ((fontcolor<0) || (fontcolor>255))
      Error("Intensity Color out of range\n");
   return ((byte) intensitytable[(pix<<8)+fontcolor]);
}


//******************************************************************************
//
// DrawIntensityChar ()
//
// Draws an intensity character at px, py
//
//******************************************************************************

void DrawIntensityChar  ( char ch )
   {

   byte  pix;
   int   px1,py1;
   int   width,w1;
   int   height,h1;
   int   ht;
   byte  *source,*src1;
   byte  *dest;
   byte  *origdest,*orgdst1;
   int   mask;

   px1 = px;py1 = py;

   ht = IFont->height;

   origdest = ( byte * )( bufferofs + ylookup[ py ] + px );

   dest = origdest;

   ch -= 31;
   width = IFont->width[ (unsigned char)ch ];
   source = ( ( byte * )IFont ) + IFont->charofs[ (unsigned char)ch ];

   mask = 1 << ( px & 3 );

   if ((iGLOBAL_SCREENWIDTH <= 320)||(StretchScreen == true)){
	   while( width-- )
	   {
		  VGAMAPMASK( mask );

		  height = ht;
		  while( height-- )
			 {
			 pix = *source;
			 if ( pix != 0xFE )
				{
				*dest = GetIntensityColor( pix );
				}

			 source++;
			 dest += linewidth;
			 }

		  px++;
		  origdest++;
		  dest = origdest;
	   }
	}else{//strech letter in x any direction
	   w1 = width;
	   h1 = ht;
	   orgdst1 = origdest;
	   src1 = source;
	   while( width-- )
	   {
		  VGAMAPMASK( mask );

		  height = ht;
		  while( height-- )
			 {
			 pix = *source;
			 if ( pix != 0xFE )
				{
				*dest = GetIntensityColor( pix );
				*(dest+iGLOBAL_SCREENWIDTH) = GetIntensityColor( pix );

				*(dest+1) = GetIntensityColor( pix );
				*(dest+1+iGLOBAL_SCREENWIDTH) = GetIntensityColor( pix );
				}

			 source++;
			 dest += linewidth*2;
			 }

		  px++;px++;
		  origdest++;origdest++;
		  dest = origdest;
	   }

	}

}


//******************************************************************************
//
// GetColor ()
//
//******************************************************************************

int GetColor (int num)
{
   int returnval;

   if ((num >= '0') && (num <= '9'))
      returnval = egacolor[num - '0'];
   else
      if ((num >= 'A') && (num <= 'F'))
         returnval = egacolor[((num - 'A') + 10)];

   return (returnval);
}

//******************************************************************************
//
// DrawIString ()
//
//******************************************************************************

static int oldfontcolor = 0;
static boolean highlight = false;

void DrawIString (unsigned short int x, unsigned short int y, const char *string, int flags)
{
   char ch;
   char temp;

   px = x;
   py = y;

   while ((ch = *string++) != 0)
   {
      if ( !PERMANENT_MSG( flags ) )
      {
         // Highlighting is done only for 1 word - if we get a "space"
         //  and highlight is on ...., reset variables.
         //
         if ((ch == ' ') && (highlight == true))
         {
            highlight = false;
            fontcolor = oldfontcolor;
            DrawIntensityChar (ch);
         }
         else
            // '\\' is color change to a specific EGA color (ie. egacolor)
            //
            if (ch == '\\')
            {
               temp = *string++;
               temp = toupper (temp);

               // Force fontcolor to a specific color egacolor[ RED ];
               if (temp == 'N')
               {
                  temp         = *string++;
                  fontcolor    = GetColor (temp);
                  oldfontcolor = fontcolor;
               }
			   //bna added
               else if (temp == 'X')
               {
				  temp         = *string;
                  fontcolor    = egacolor[ RED ];
                  oldfontcolor = fontcolor;
               }
               else if (temp == 'Y')
               {
				   temp         = *string;
                  fontcolor    = egacolor[ YELLOW ];
                  oldfontcolor = fontcolor;
               }
               else if (temp == 'Z')
               {
				   temp         = *string;
                  fontcolor    = egacolor[ GREEN ];
                  oldfontcolor = fontcolor;
               }
			   //bna added end
               // Restore fontcolor to a previous color
               else if (temp == 'O')
               {
                  fontcolor    = oldfontcolor;
               }
               else
               {
                  oldfontcolor = fontcolor;           // save off old font color
                  highlight    = true;                // set highlight
                  fontcolor    = GetColor (temp);
               }
            }
            else
               // '`' is highlight the current fontcolor
               //
               if (ch == '`')
               {
                  oldfontcolor = fontcolor;        // save off old font color
                  highlight    = true;             // set highlight
                  if (fontcolor < 8)               // only highlight the
                     fontcolor    = fontcolor-10;  //  lower colors
               }
               else
                  DrawIntensityChar (ch);
      }
      else
         DrawIntensityChar (ch);
   }

   if (highlight == true)
   {
      highlight = false;
      fontcolor = oldfontcolor;
   }
}


//******************************************************************************
//
// DrawIntensityString ()
//
//******************************************************************************

void DrawIntensityString (unsigned short int x, unsigned short int y, const char *string, int color)
{
   char ch;

   px = x;
   py = y;

   fontcolor=color;

   while ((ch = *string++) != 0)
      {
      DrawIntensityChar (ch);
      }
}









































static unsigned short disp_offset = 160 * 24;

void DrawRottText
   (
   int x,
   int y,
   int ch,
   int foreground,
   int background
   )

   {
   char *vid;

   vid  = ( char * )( 0xb0000 );
   vid += y * 160;
   vid += x * 2;

   if ( ch != NONE )
      {
      *vid = ch;
      }
   vid++;
   *vid = ( ( background & 0x0f ) << 4 ) | ( foreground & 0x0f );
   }

void TextBox
   (
   int  x1,
   int  y1,
   int  x2,
   int  y2,
   int ch,
   int  foreground,
   int  background
   )

   {
   int x;
   int y;

   for( x = x1; x <= x2; x++ )
      {
      for( y = y1; y <= y2; y++ )
         {
         DrawRottText( x, y, ch, foreground, background );
         }
      }
   }

void TextFrame
   (
   int x1,
   int y1,
   int x2,
   int y2,
   int type,
   int foreground,
   int background
   )

   {
   int x;
   int y;

   if ( type == 0 )
      {
      for( x = x1 + 1; x < x2; x++ )
         {
         DrawRottText( x, y1, type, foreground, background );
         DrawRottText( x, y2, type, foreground, background );
         }
      for( y = y1 + 1; y < y2; y++ )
         {
         DrawRottText( x1, y, type, foreground, background );
         DrawRottText( x2, y, type, foreground, background );
         }
      }
   if ( type == SINGLE_FRAME )
      {
      DrawRottText( x1, y1, '�', foreground, background );
      DrawRottText( x2, y1, '�', foreground, background );
      DrawRottText( x1, y2, '�', foreground, background );
      DrawRottText( x2, y2, '�', foreground, background );
      for( x = x1 + 1; x < x2; x++ )
         {
         DrawRottText( x, y1, '�', foreground, background );
         DrawRottText( x, y2, '�', foreground, background );
         }
      for( y = y1 + 1; y < y2; y++ )
         {
         DrawRottText( x1, y, '�', foreground, background );
         DrawRottText( x2, y, '�', foreground, background );
         }
      }
   if ( type == DOUBLE_FRAME )
      {
      DrawRottText( x1, y1, '�', foreground, background );
      DrawRottText( x2, y1, '�', foreground, background );
      DrawRottText( x1, y2, '�', foreground, background );
      DrawRottText( x2, y2, '�', foreground, background );
      for( x = x1 + 1; x < x2; x++ )
         {
         DrawRottText( x, y1, '�', foreground, background );
         DrawRottText( x, y2, '�', foreground, background );
         }
      for( y = y1 + 1; y < y2; y++ )
         {
         DrawRottText( x1, y, '�', foreground, background );
         DrawRottText( x2, y, '�', foreground, background );
         }
      }
   }

void mysetxy
   (
   int x,
   int y
   )

   {
   disp_offset = ( x * 2 ) + ( y * 160 );
   }

void myputch
   (
   char ch
   )

   {
   int j;
   char *disp_start = (char *)( 0xb0000 );

   if ( disp_offset >= 160 * 24 )
      {
      for ( j = 160; j < 160 * 24; j += 2 )
         {
         *( disp_start + j - 160 ) = *( disp_start + j );
         }

      disp_offset = 160 * 23;

      for ( j = disp_offset; j < ( 160 * 24 ); j += 2 )
         {
         *( disp_start + j ) = ' ';
         }
      }

   if ( ch >= 32 )
      {
      *( disp_start + disp_offset ) = ch;
      disp_offset = disp_offset + 2;
      }

   if ( ch == '\r' )
      {
      disp_offset = disp_offset / 160;
      disp_offset = disp_offset * 160;
      }

   if ( ch == '\n' )
      {
      disp_offset = disp_offset + 160;
      if ( disp_offset < 160 * 24 )
         {
         for ( j = disp_offset; j < ( ( ( disp_offset / 160 ) + 1 ) *
            160 ); j += 2 )
            {
            *( disp_start + j ) = ' ';
            }
         }
      }
   }

int printstring
   (
   char *string
   )

   {
   int count;
   char *ptr;

   ptr = string;
   count = 0;

   while ( *ptr )
      {
      myputch( *ptr );
      count++;
      ptr++;
      }

   return( count );
   }


int printnum
   (
   int number
   )

   {
   char string[ 100 ];
   int  count;

   itoa( number, string, 10 );
   count = printstring( string );

   return( count );
   }

int printunsigned
   (
   unsigned long number,
   int radix
   )

   {
   char string[ 100 ];
   int  count;

   ultoa( number, string, radix );
   count = printstring( string );

   return( count );
   }

int myprintf
   (
   char *fmt,
   ...
   )

   {
   va_list argptr;
   int     count;
   char    *ptr;
   if (MONOPRESENT==false)
      {
      Debug("%s", fmt);
      return 0;
      }
   va_start( argptr, fmt );
   ptr = fmt;
   count = 0;

   while( *ptr != 0 )
      {
      if ( *ptr == '%' )
         {
         ptr++;
         switch( *ptr )
            {
            case 0 :
               return( EOF );
               break;
            case 'l' :
               count += printnum( va_arg( argptr, int ) );
               ptr++;
               break;
            case 'd' :
               count += printnum( va_arg( argptr, int ) );
               break;
            case 's' :
               count += printstring( va_arg( argptr, char * ) );
               break;
            case 'u' :
               count += printunsigned( va_arg( argptr, int ), 10 );
               break;
            case 'x' :
            case 'X' :
               count += printunsigned( va_arg( argptr, int ), 16 );
               break;
            }
         ptr++;
         }
      else
         {
         myputch( *ptr );
         count++;
         ptr++;
         }
      }

   va_end( argptr );

   return( count );
   }
