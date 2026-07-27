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
#include "debugio.h"

static unsigned short disp_offset = 160 * 24;
static void myutoa( unsigned num, char *string, int radix );
static void myitoa( int num, char *string, int radix );

void DB_SetXY
   (
   int x,
   int y
   )

   {
   disp_offset = ( x * 2 ) + ( y * 160 );
   }

void DB_PutChar
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

int DB_PrintString
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
      DB_PutChar( *ptr );
      count++;
      ptr++;
      }

   return( count );
   }

static void myutoa
   (
   unsigned num,
   char *string,
   int radix
   )

   {
   int val;
   int length;
   int pos;
   char temp[ 100 ];

   length = 0;
   do
      {
      val = num % radix;
      if ( val < 10 )
         {
         temp[ length ] = '0' + val;
         }
      else
         {
         temp[ length ] = 'A' + val - 10;
         }
      num /= radix;
      length++;
      }
   while( num > 0 );

   pos = 0;
   while( length > 0 )
      {
      length--;
      string[ length ] = temp[ pos ];
      pos++;
      }
   string[ pos ] = 0;
   }

static void myitoa
   (
   int num,
   char *string,
   int radix
   )

   {
   if ( num < 0 )
      {
      *string++ = '-';
      num = -num;
      }

   myutoa( num, string, radix );
   }

int DB_PrintNum
   (
   int number
   )

   {
   char string[ 100 ];
   int  count;

   myitoa( number, &string[ 0 ], 10 );
   count = DB_PrintString( &string[ 0 ] );

   return( count );
   }

int DB_PrintUnsigned
   (
   unsigned long number,
   int radix
   )

   {
   char string[ 100 ];
   int  count;

   myutoa( number, &string[ 0 ], radix );
   count = DB_PrintString( &string[ 0 ] );

   return( count );
   }

int DB_printf
   (
   char *fmt,
   ...
   )

   {
   va_list argptr;
   int     count;
   char    *ptr;

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
               count += DB_PrintNum( va_arg( argptr, int ) );
               break;
            case 's' :
               count += DB_PrintString( va_arg( argptr, char * ) );
               break;
            case 'u' :
               count += DB_PrintUnsigned( va_arg( argptr, int ), 10 );
               break;
            case 'x' :
            case 'X' :
               count += DB_PrintUnsigned( va_arg( argptr, int ), 16 );
               break;
            }
         ptr++;
         }
      else
         {
         DB_PutChar( *ptr );
         count++;
         ptr++;
         }
      }

   va_end( argptr );

   return( count );
   }
