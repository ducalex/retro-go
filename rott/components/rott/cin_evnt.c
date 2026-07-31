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
#include "cin_glob.h"
#include "cin_evnt.h"
#include "cin_efct.h"
#include "cin_actr.h"
#include "cin_def.h"
#include "scriplib.h"
#include "lumpy.h"
#include "w_wad.h"
#include "z_zone.h"
#include <string.h>
//MED
#include "memcheck.h"

eventtype * firstevent;
eventtype * lastevent;

// LOCALS

static int numevents=0;
static boolean eventsystemstarted=false;


/*
===============
=
= AddEvent
=
===============
*/

void AddEvent (eventtype * event)
{
   if (!firstevent)
      firstevent  = event;
   else
      {
      event->prev = lastevent;
      lastevent->next = event;
      }
   lastevent = event;
}


/*
===============
=
= DeleteEvent
=
===============
*/

void DeleteEvent(eventtype * event)
{
   if (event == lastevent)
      lastevent = event->prev;
   else
      event->next->prev = event->prev;

   if (event == firstevent)
      firstevent = event->next;
   else
      event->prev->next = event->next;

   event->prev = NULL;
   event->next = NULL;

   SafeFree ( event );
}

/*
===============
=
= GetNewEvent
=
===============
*/

eventtype * GetNewEvent ( void )
{
   eventtype * event;

   numevents++;

   if ( numevents > MAXCINEMATICEVENTS )
      Error ("Too many Cinematic events\n");

   event = SafeMalloc( sizeof (eventtype) );

   event->next=NULL;
   event->prev=NULL;

   return event;
}

/*
===============
=
= StartupEvents
=
===============
*/

void StartupEvents ( void )
{
   if (eventsystemstarted==true)
      return;
   eventsystemstarted=true;
   firstevent = NULL;
   lastevent  = NULL;

   numevents=0;
}

/*
===============
=
= ShutdownEvents
=
===============
*/

void ShutdownEvents ( void )
{
   eventtype * event;

   if (eventsystemstarted==false)
      return;
   eventsystemstarted=false;

   event=firstevent;
   while (event != NULL)
      {
      eventtype * nextevent;

      nextevent=event->next;
      DeleteEvent(event);
      event=nextevent;
      }
}

/*
===============
=
= CreateEvent
=
===============
*/
eventtype * CreateEvent ( int time, int type )
{
   eventtype * event;

   event = GetNewEvent ();

   event->time = time;
   event->effecttype = type;
   event->effect = NULL;

   AddEvent (event);

   return event;
}

/*
===============
=
= GetEventType
=
= Gets event type from token
=
===============
*/
enum_eventtype GetEventType ( void )
{
   // Get Event Token

   GetToken (false);

   if (!strcmpi (token,"BACKGROUND"))
      {
      GetToken (false);
      if (!strcmpi (token,"SCROLL"))
         {
         return background_scrolling;
         }
      else if (!strcmpi (token,"NOSCROLL"))
         {
         return background_noscrolling;
         }
      else if (!strcmpi (token,"MULTISCROLL"))
         {
         return background_multi;
         }
      else
         {
         Error("Illegal Background Scrolling token\n");
         }
      }
   else if (!strcmpi (token,"BACKDROP"))
      {
      GetToken (false);
      if (!strcmpi (token,"SCROLL"))
         {
         return backdrop_scrolling;
         }
      else if (!strcmpi (token,"NOSCROLL"))
         {
         return backdrop_noscrolling;
         }
      else
         {
         Error("Illegal Backdrop Scrolling token\n");
         }
      }
   else if (!strcmpi (token,"BACKGROUNDSPRITE"))
      {
      return sprite_background;
      }
   else if (!strcmpi (token,"FOREGROUNDSPRITE"))
      {
      return sprite_foreground;
      }
   else if (!strcmpi (token,"PALETTE"))
      {
      return palette;
      }
   else if (!strcmpi (token,"FADEOUT"))
      {
      return fadeout;
      }
   else if (!strcmpi (token,"FLIC"))
      {
      return flic;
      }
   else if (!strcmpi (token,"MOVIEEND"))
      {
      return cinematicend;
      }
   else if (!strcmpi (token,"BLANKSCREEN"))
      {
      return blankscreen;
      }
   else if (!strcmpi (token,"CLEARBUFFER"))
      {
      return clearbuffer;
      }
   else
      {
      Error("GetEventType: Illegal Token %s\n",token);
      }
   return -1;
}

/*
===============
=
= ParseBack
=
===============
*/
void ParseBack ( eventtype * event )
{
   char name[10];
   char name2[10];
   int duration;
   int yoffset;
   int width;
   int startx;
   int endx;

   GetToken (false);
   strcpy(&(name[0]),token);

   if (event->effecttype==background_multi)
      {
      GetToken (false);
      strcpy(&(name2[0]),token);
      }

   GetToken (false);
   duration=ParseNum(token);
   GetToken (false);
   yoffset=ParseNum(token);
   if (
       (event->effecttype==background_noscrolling) ||
       (event->effecttype==background_scrolling)
      )
      {
      lpic_t * lpic;

      lpic = (lpic_t *)W_CacheLumpName(name,PU_CACHE, Cvt_lpic_t, 1);
      width = lpic->width;
      }
   else if (event->effecttype!=background_multi)
      {
      patch_t * patch;

      patch = (patch_t *)W_CacheLumpName(name,PU_CACHE, Cvt_lpic_t, 1);
      width = patch->width;
      }

   startx=0;
   endx=0;
   if (
       (event->effecttype==backdrop_scrolling) ||
       (event->effecttype==background_scrolling) ||
       (event->effecttype==background_multi)
      )
      {
      GetToken (false);
      startx=ParseNum(token);
      GetToken (false);
      endx=ParseNum(token);
      }

   if (event->effecttype==background_multi)
      event->effect = SpawnCinematicMultiBack ( name, name2, duration, startx, endx, yoffset);
   else
      event->effect = SpawnCinematicBack ( name, duration, width, startx, endx, yoffset);
}

/*
===============
=
= ParseSprite
=
===============
*/
void ParseSprite ( eventtype * event )
{
   char name[10];
   int duration;
   int numframes;
   int framedelay;
   int x,y,scale;
   int endx,endy,endscale;

   GetToken (false);
   strcpy(&(name[0]),token);
   GetToken (false);
   duration=ParseNum(token);
   GetToken (false);
   numframes=ParseNum(token);
   GetToken (false);
   framedelay=ParseNum(token);
   GetToken (false);
   x=ParseNum(token);
   GetToken (false);
   y=ParseNum(token);
   GetToken (false);
   scale=ParseNum(token);
   GetToken (false);
   endx=ParseNum(token);
   GetToken (false);
   endy=ParseNum(token);
   GetToken (false);
   endscale=ParseNum(token);

   event->effect = SpawnCinematicSprite ( name, duration, numframes,
                                          framedelay, x, y, scale,
                                          endx, endy, endscale
                                        );
}

/*
===============
=
= ParseFlic
=
===============
*/
void ParseFlic ( eventtype * event )
{
   char name[10];
   boolean loop;
   boolean usefile;

   GetToken (false);
   strcpy(&(name[0]),token);

   GetToken (false);
   if (!strcmpi (token,"LOOP"))
      {
      loop = true;
      }
   else if (!strcmpi (token,"NOLOOP"))
      {
      loop = false;
      }
   else
      Error("ParseFlic: Illegal or missing flic loop token %s\n",token);

   GetToken (false);
   if (!strcmpi (token,"FILE"))
      {
      usefile=true;
      }
   else if (!strcmpi (token,"LUMP"))
      {
      usefile=false;
      }
   else
      Error("ParseFlic: Illegal or missing flic use token %s\n",token);

   event->effect = SpawnCinematicFlic ( name, loop, usefile );
}

/*
===============
=
= ParsePalette
=
===============
*/
void ParsePalette ( eventtype * event )
{
   char name[10];

   GetToken (false);
   strcpy(&(name[0]),token);

   event->effect = SpawnCinematicPalette ( name );
}

/*
===============
=
= ParseEvent
=
===============
*/
void ParseEvent ( int time )
{
   eventtype * event;

   event = CreateEvent ( time, GetEventType() );

   switch (event->effecttype)
      {
      case background_noscrolling:
      case background_scrolling:
      case background_multi:
      case backdrop_scrolling:
      case backdrop_noscrolling:
         ParseBack(event);
         break;
      case sprite_background:
      case sprite_foreground:
         ParseSprite(event);
         break;
      case palette:
         ParsePalette(event);
         break;
      case flic:
         ParseFlic(event);
         break;
      case fadeout:
      case blankscreen:
      case clearbuffer:
      case cinematicend:
         break;
      }
}

/*
===============
=
= UpdateCinematicEvents
=
===============
*/
void UpdateCinematicEvents ( int time )
{
   eventtype * event;

   for (event=firstevent;event != NULL;)
      {
      if (event->time==time)
         {
         eventtype * nextevent;

         nextevent=event->next;
         SpawnCinematicActor ( event->effecttype, event->effect );
         DeleteEvent(event);
         event=nextevent;
         }
      else if (event->time>time)
         break;
      else
         event=event->next;
      }
}

/*
===============
=
= PrecacheCinematic
=
===============
*/
void PrecacheCinematic ( void )
{
   eventtype * event;

   for (event=firstevent;event != NULL;)
      {
      PrecacheCinematicEffect ( event->effecttype, event->effect );
      event=event->next;
      }
}

