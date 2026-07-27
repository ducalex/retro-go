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
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "myprint.h"

static unsigned short disp_offset = 160 * 24;

void DrawText
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
         DrawText( x, y, ch, foreground, background );
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
         DrawText( x, y1, type, foreground, background );
         DrawText( x, y2, type, foreground, background );
         }
      for( y = y1 + 1; y < y2; y++ )
         {
         DrawText( x1, y, type, foreground, background );
         DrawText( x2, y, type, foreground, background );
         }
      }
   if ( type == SINGLE_FRAME )
      {
      DrawText( x1, y1, 'Ú', foreground, background );
      DrawText( x2, y1, '¿', foreground, background );
      DrawText( x1, y2, 'À', foreground, background );
      DrawText( x2, y2, 'Ù', foreground, background );
      for( x = x1 + 1; x < x2; x++ )
         {
         DrawText( x, y1, 'Ä', foreground, background );
         DrawText( x, y2, 'Ä', foreground, background );
         }
      for( y = y1 + 1; y < y2; y++ )
         {
         DrawText( x1, y, '³', foreground, background );
         DrawText( x2, y, '³', foreground, background );
         }
      }
   if ( type == DOUBLE_FRAME )
      {
      DrawText( x1, y1, 'É', foreground, background );
      DrawText( x2, y1, '»', foreground, background );
      DrawText( x1, y2, 'È', foreground, background );
      DrawText( x2, y2, '¼', foreground, background );
      for( x = x1 + 1; x < x2; x++ )
         {
         DrawText( x, y1, 'Í', foreground, background );
         DrawText( x, y2, 'Í', foreground, background );
         }
      for( y = y1 + 1; y < y2; y++ )
         {
         DrawText( x1, y, 'º', foreground, background );
         DrawText( x2, y, 'º', foreground, background );
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

   return( 0 );

   // DEBUG
   mysetxy( 0, 0 );

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
