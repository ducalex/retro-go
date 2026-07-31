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
#include "rt_def.h"
#include "rt_view.h"
#include "z_zone.h"
#include "w_wad.h"
#include "lumpy.h"
#include "rt_util.h"
#include "rt_vid.h"
#include "rt_str.h"
#include "rt_menu.h"
#include "_rt_msg.h"
#include "rt_msg.h"
#include "rt_playr.h"
#include "isr.h"
#include "rt_main.h"
#include "rt_net.h"
#include "rt_com.h"

#ifdef DOS
#include <mem.h>
#endif

#include <stdlib.h>
//MED
#include "memcheck.h"


/*
=============================================================================

                               GLOBALS

=============================================================================
*/
messagetype Messages[MAXMSGS];

/*
=============================================================================

                               LOCAL

=============================================================================
*/

static int  UpdateMessageBackground;
static int  MessageSystemStarted=0;
static int  LastMessageTime;
static boolean EraseMessage[ MAXMSGS ];
static int     MessageOrder[ MAXMSGS ];
static int     TotalMessages = 0;
static int     MsgPos = 0;

boolean MessagesEnabled = true;

int StringLength (char *string)
{
	int length=0;

   while ((*string)!=0)
      {
      length++;
      string++;
      }

   length++;

   return length;
}

/*
====================
=
= ResetMessageTime
=
====================
*/

void ResetMessageTime ( void )
{
   LastMessageTime=GetTicCount();
}

/*
====================
=
= InitializeMessages
=
====================
*/
void InitializeMessages
   (
   void
   )

   {
   int i;
   boolean start;

   start = false;

   if ( MessageSystemStarted == 0 )
      {
      start = true;
      MessageSystemStarted = 1;
      memset( Messages, 0, sizeof( Messages ) );
      }

   for ( i = 0; i < MAXMSGS; i++ )
      {
      if ( Messages[ i ].active == 1 )
         {
         SafeFree( Messages[ i ].text );
         Messages[ i ].active  = 0;
         Messages[ i ].tictime = 0;
         Messages[ i ].flags   = 0;
         Messages[ i ].text    = NULL;
         }
      }

   MSG.messageon = false;

   LastMessageTime = 0;
   UpdateMessageBackground = 0;
   TotalMessages = 0;
   memset( EraseMessage,  0, sizeof( EraseMessage ) );
   memset( MessageOrder, -1, sizeof( MessageOrder ) );

   // Only print startup message if it's the first time in
   if ( start && !quiet )
      {
      printf( "RT_MSG: Message System Started\n" );
      }
   }


/*
====================
=
= GetMessageOrder
=
====================
*/
void GetMessageOrder
   (
   void
   )

   {
   int  i;
   int  lowest;
   int  lowesttime;
   byte done[ MAXMSGS ];
   boolean found;

   memset( &done[ 0 ],    0, sizeof( done ) );
   memset( MessageOrder, -1, sizeof( MessageOrder ) );

   for( TotalMessages = 0; TotalMessages < MAXMSGS; TotalMessages++ )
      {
      found = false;
      lowesttime = 1000;
      lowest = 0;

      for( i = 0; i < MAXMSGS; i++ )
         {
         if ( ( Messages[ i ].active == 1 ) && ( done[ i ] == 0 ) &&
            ( Messages[ i ].tictime < lowesttime ) )
            {
            lowesttime = Messages[ i ].tictime;
            lowest = i;
            found = true;
            }
         }

      if ( !found )
         {
         break;
         }

      done[ lowest ] = 1;
      MessageOrder[ TotalMessages ] = lowest;
      }
   }


/*
====================
=
= DeleteMessage
=
====================
*/
void DeleteMessage
   (
   int num
   )

   {
   int i;
   int msg;
   boolean found;

   found = false;
   for( i = 0; i < TotalMessages; i++ )
      {
      msg = MessageOrder[ i ];

      if ( msg == num )
         {
         found = true;
         }

      if ( found )
         {
         UpdateMessageBackground -= EraseMessage[ i ];
         UpdateMessageBackground += 3;
         EraseMessage[ i ] = 3;
         }
      }

   SafeFree( Messages[ num ].text );
   memset( &Messages[ num ], 0, sizeof( messagetype ) );

   GetMessageOrder();
   }


/*
====================
=
= DeletePriorityMessage
=
====================
*/
void DeletePriorityMessage ( int flags )
{
   int i;

   for (i=0;i<MAXMSGS;i++)
      {
      if (Messages[i].active==1)
         {
         if (Messages[i].flags==flags)
            DeleteMessage(i);
         }
      }
}


/*
====================
=
= GetFreeMessage
=
====================
*/
int GetFreeMessage
   (
   void
   )

   {
   int i;
   int found;

   for( i = 0; i < MAXMSGS; i++ )
      {
      if ( Messages[ i ].active == 0 )
         {
         return( i );
         }
      }

   found = -1;

   for( i = 0; i < MAXMSGS; i++ )
      {
      if ( Messages[ i ].tictime >= 0 )
         {
         if ( found == -1 )
            {
            found = i;
            }
         else
            {
            if ( Messages[ i ].tictime < Messages[ found ].tictime )
               {
               found = i;
               }
            }
         }
      }

   DeleteMessage( found );

   return( found );
   }


/*
====================
=
= SetMessage
=
====================
*/
void SetMessage
   (
   int   num,
   char *text,
   int   flags
   )

   {
   int i;
   int msg;
   int length;
   boolean found;

   if (iGLOBAL_SCREENWIDTH >= 640){
		CurrentFont = newfont1;//smallfont;
   }else{
		CurrentFont = smallfont;
   }


   length = StringLength( text );

   Messages[ num ].active = 1;
   Messages[ num ].flags  = flags;

   if ( PERMANENT_MSG( flags ) )
      {
      int l;

      l = COM_MAXTEXTSTRINGLENGTH + 1;
      Messages[ num ].text = SafeMalloc( l );
      memset( Messages[ num ].text, 0, l );

      // Hack so that we can place menu in certain order
      Messages[ num ].tictime = -100 + MsgPos;
      }
   else
      {
      Messages[ num ].text = SafeMalloc( length );

      memset( Messages[ num ].text, 0, length );
      Messages[ num ].tictime = MESSAGETIME;
      }

   memcpy( Messages[ num ].text, text, length );

   GetMessageOrder();
   found = false;
   for( i = 0; i < TotalMessages; i++ )
      {
      msg = MessageOrder[ i ];
      if ( msg == num )
         {
         found = true;
         }
      else if ( found )
         {
         UpdateMessageBackground -= EraseMessage[ i - 1 ];
         UpdateMessageBackground += 3;
         EraseMessage[ i - 1 ] = 3;
         }
      }
   }


/*
====================
=
= AddMessage
=
====================
*/
int AddMessage
   (
   char *text,
   int flags
   )

   {
   int new;

   if ( MessageSystemStarted == 0 )
      {
      Error( "Called AddMessage without starting Message system\n" );
      }

   if ( !( flags & MSG_NODELETE ) )
      {
      DeletePriorityMessage( flags );
      }

   new = GetFreeMessage();
   SetMessage( new, text, flags );

   return( new );
   }

/*
====================
=
= UpdateMessages
=
====================
*/
void UpdateMessages
   (
   void
   )

   {
   int messagetics;
   int i;

   messagetics = GetTicCount() - LastMessageTime;
   LastMessageTime = GetTicCount();

   if ( GamePaused == true )
      {
      return;
      }

   for( i = 0; i < MAXMSGS; i++ )
      {
      if ( ( Messages[ i ].active == 1 ) &&
         ( !PERMANENT_MSG( Messages[ i ].flags ) ) )
         {
         Messages[ i ].tictime -= messagetics;
         if ( Messages[ i ].tictime <= 0 )
            {
            DeleteMessage( i );
            }
         }
      }
   }


/*
====================
=
= DisplayMessage
=
====================
*/

void DisplayMessage   (int num,int position)
   {
   PrintX = 1;
   if (iGLOBAL_SCREENWIDTH > 320){
		PrintY = 2 + ( position * (9*2) );
   }else{
		PrintY = 2 + ( position * (9*1) );
   }


   if ( SHOW_TOP_STATUS_BAR() )
      {
      PrintY += 16;
      }
   if ( !MessagesEnabled )
      {
      switch ( Messages[ num ].flags )
         {
         case MSG_QUIT:
         case MSG_MACRO:
         case MSG_MODEM:
         case MSG_NAMEMENU:
         case MSG_MSGSYSTEM:
            break;

         case MSG_REMOTERIDICULE:
         case MSG_REMOTE:
         case MSG_GAME:
         case MSG_DOOR:
         case MSG_BONUS:
         case MSG_BONUS1:
         case MSG_CHEAT:
         case MSG_SYSTEM:
         default :
            DeleteMessage( num );
            return;
         }
      }

   switch ( Messages[ num ].flags )
      {
      case MSG_REMOTERIDICULE:
      case MSG_REMOTE:
         fontcolor = egacolor[ WHITE ];
         break;

      case MSG_MODEM:
         fontcolor = egacolor[ LIGHTBLUE ];
         DrawIString( PrintX, PrintY, "Message>", Messages[ num ].flags );
		 if ( iGLOBAL_SCREENWIDTH == 320) {
			PrintX += 8 * 8;
		 }else if ( iGLOBAL_SCREENWIDTH == 640) {
			PrintX += 8 * 8*2;
		 }else if ( iGLOBAL_SCREENWIDTH == 800) {
			PrintX += 8 * 8*2;
		 }

         fontcolor = egacolor[ LIGHTGRAY ];
         break;

      case MSG_GAME:
      case MSG_DOOR:
      case MSG_BONUS:
      case MSG_BONUS1:
      case MSG_NAMEMENU:
         fontcolor = egacolor[ GREEN ];
         break;

      case MSG_CHEAT:
         fontcolor = egacolor[ YELLOW ];
         break;
      case MSG_MSGSYSTEM:
      case MSG_SYSTEM:
      case MSG_QUIT:
      case MSG_MACRO:
         fontcolor = egacolor[ RED ];
         break;

      default :
#if ((DEVELOPMENT == 1))
         Error( "DisplayMessage called with invalid priority number." );
#else
         fontcolor = egacolor[ LIGHTGREEN ];
#endif
      }

   DrawIString( PrintX, PrintY, Messages[ num ].text, Messages[ num ].flags );
   }



/*
====================
=
= RestoreMessageBackground
=
====================
*/
void RestoreMessageBackground
   (
   void
   )

   {
   pic_t *shape;
   int i;
   int y;

   if ( UpdateMessageBackground > 0 )
      {
      y = 18;
      for( i = 0; i < MAXMSGS; i++ )
         {
         if ( EraseMessage[ i ] )
            {
            UpdateMessageBackground--;
            EraseMessage[ i ]--;
            if ( viewsize < 15 )
               {
               shape =  ( pic_t * )W_CacheLumpName( "backtile", PU_CACHE, Cvt_pic_t, 1 );//w=32 h=8
			   	   //SetTextMode (  );
               //DrawTiledRegion( 0, y, 320, 9, 0, y, shape );KILLS_HEIGHT bna--
				DrawTiledRegion( 0, y, iGLOBAL_SCREENWIDTH, 9, 0, y, shape );
				DrawTiledRegion( 0, y+8, iGLOBAL_SCREENWIDTH, 9, 0, y, shape );
				DrawTiledRegion( 0, y+16, iGLOBAL_SCREENWIDTH, 9, 0, y, shape );

               //DrawTiledRegion( 0, y, iGLOBAL_SCREENWIDTH, 212, 0, y, shape );
               }
            if ( viewsize == 0 )
               {
               if ( ( y + 9 > YOURCPUSUCKS_Y ) &&
                  ( y < ( YOURCPUSUCKS_Y + YOURCPUSUCKS_HEIGHT ) ) )
                  {
                  DrawCPUJape();
                  }
               }
            }

         y += 9;
         }
      }
   }


/*
====================
=
= DrawMessages
=
====================
*/
void DrawMessages
   (
   void
   )

   {
   int i;

   if ( TotalMessages > 0 )
      {
      IFont = ( cfont_t * )W_CacheLumpName( "ifnt", PU_CACHE, Cvt_cfont_t, 1 );

      for( i = 0; i < TotalMessages; i++ )
         {
         DisplayMessage( MessageOrder[ i ], i );
         }
      }
   UpdateMessages();
   }


/*
====================
=
= UpdateModemMessage
=
====================
*/
void UpdateModemMessage
   (
   int num,
   char c
   )

   {
   int i;

   Messages[ num ].text[ MSG.length - 1 ] = ( byte )c;
   Messages[ num ].text[ MSG.length ]     = ( byte )'_';
   MSG.length++;

   for( i = 0; i < TotalMessages; i++ )
      {
      if ( MessageOrder[ i ] == num )
         {
         UpdateMessageBackground -= EraseMessage[ i ];
         UpdateMessageBackground += 3;
         EraseMessage[ i ] = 3;
         break;
         }
      }
   }


/*
====================
=
= ModemMessageDeleteChar
=
====================
*/
void ModemMessageDeleteChar
   (
   int num
   )

   {
   int i;

   MSG.length--;
   Messages[ num ].text[ MSG.length ]     = ( byte )0;
   Messages[ num ].text[ MSG.length - 1 ] = ( byte )'_';

   for( i = 0; i < TotalMessages; i++ )
      {
      if ( MessageOrder[ i ] == num )
         {
         UpdateMessageBackground -= EraseMessage[ i ];
         UpdateMessageBackground += 3;
         EraseMessage[ i ] = 3;
         break;
         }
      }
   }


/*
====================
=
= DrawPlayerSelectionMenu
=
====================
*/

void DrawPlayerSelectionMenu
   (
   void
   )

   {
   int i;
   int p;
   char str[ 20 ];

   p = 1;
   MsgPos = 1;
   AddMessage( "Press a key from 0 to 9 to select", MSG_NAMEMENU );
   MsgPos++;
   AddMessage( "who to send your message to:", MSG_NAMEMENU );
   MsgPos++;

   for( i = 0; i < numplayers; i++ )
      {
      if ( i != consoleplayer )
         {
         strcpy( str, "0 - " );
         strcat( str, PLAYERSTATE[ i ].codename );
         str[ 0 ] = '0' + p;
         p++;
         if ( p > 9 )
            {
            p = 0;
            }

         AddMessage( str, MSG_NAMEMENU );
         MsgPos++;
         }
      }

   if ( ( MsgPos < MAXMSGS - 1 ) && ( gamestate.teamplay ) )
      {
      AddMessage( "T - All team members", MSG_NAMEMENU );
      MsgPos++;
      }

   if ( MsgPos < MAXMSGS - 1 )
      {
      AddMessage( "A - All players", MSG_NAMEMENU );
      }

   MsgPos = 0;
   }


/*
====================
=
= FinishModemMessage
=
====================
*/
void FinishModemMessage
   (
   int num,
   boolean send
   )
   {
   if ( ( !MSG.inmenu ) && ( MSG.length > 0 ) )
      {
      Messages[ num ].text[ MSG.length - 1 ] = ( byte )0;
      MSG.length--;
      }

   if ( ( send == true ) && ( ( MSG.length > 0 ) ||
      ( MSG.remoteridicule != -1 ) ) )
      {
      if ( ( MSG.directed ) && ( !MSG.inmenu ) )
         {
         DrawPlayerSelectionMenu();
         MSG.messageon = true;
         MSG.inmenu = true;
         return;
         }

      MSG.messageon = false;
      if ( MSG.remoteridicule != -1 )
         {
         AddRemoteRidiculeCommand( consoleplayer, MSG.towho,
            MSG.remoteridicule );
         }
      if ( MSG.length > 0 )
         {
         AddTextMessage( Messages[ num ].text, MSG.length, MSG.towho );
         }
      }

   if ( MSG.inmenu )
      {
      DeletePriorityMessage( MSG_NAMEMENU );
      }

   DeleteMessage( num );
   }
