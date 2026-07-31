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
#include "cin_def.h"
#include "cin_actr.h"
#include "cin_efct.h"
#include "modexlib.h"
//MED
#include "memcheck.h"

actortype * firstcinematicactor;
actortype * lastcinematicactor;


// LOCALS

boolean cinematicactorsystemstarted=false;
static int numcinematicactors;

/*
===============
=
= AddCinematicActor
=
===============
*/

void AddCinematicActor ( actortype * actor )
{
   if (!firstcinematicactor)
      {
      firstcinematicactor  = actor;
      }
   else
      {
      actor->prev = lastcinematicactor;
      lastcinematicactor->next = actor;
      }
   lastcinematicactor = actor;
}


/*
===============
=
= DeleteCinematicActor
=
===============
*/

void DeleteCinematicActor ( actortype * actor)
{
   if (actor == lastcinematicactor)
      {
      lastcinematicactor = actor->prev;
      }
   else
      {
      actor->next->prev = actor->prev;
      }

   if (actor == firstcinematicactor)
      {
      firstcinematicactor = actor->next;
      }
   else
      {
      actor->prev->next = actor->next;
      }

   actor->prev = NULL;
   actor->next = NULL;

   if (actor->effect != NULL)
      SafeFree(actor->effect);
   SafeFree(actor);
}

/*
===============
=
= GetNewCinematicActor
=
===============
*/

actortype * GetNewCinematicActor ( void )
{
   actortype * actor;

   numcinematicactors++;

   if ( numcinematicactors > MAXCINEMATICACTORS )
      Error ("Too many Cinematic actors\n");

   actor = SafeMalloc( sizeof (actortype) );

   actor->next=NULL;
   actor->prev=NULL;

   AddCinematicActor ( actor );

   return actor;
}

/*
===============
=
= StartupCinematicActors
=
===============
*/

void StartupCinematicActors ( void )
{
   if (cinematicactorsystemstarted==true)
      return;
   cinematicactorsystemstarted=true;
   firstcinematicactor = NULL;
   lastcinematicactor  = NULL;

   numcinematicactors=0;
}

/*
===============
=
= ShutdownCinematicActors
=
===============
*/

void ShutdownCinematicActors ( void )
{
   actortype * actor;
   if (cinematicactorsystemstarted==false)
      return;
   cinematicactorsystemstarted=false;

   actor=firstcinematicactor;
   while (actor != NULL)
      {
      actortype * nextactor;

      nextactor=actor->next;
      DeleteCinematicActor(actor);
      actor=nextactor;
      }
}

/*
===============
=
= SpawnCinematicActor
=
===============
*/

void SpawnCinematicActor ( enum_eventtype type, void * effect )
{
   actortype * actor;

   actor = GetNewCinematicActor ();
   actor->effecttype=type;
   actor->effect=effect;
}

/*
===============
=
= UpdateCinematicActors
=
===============
*/
void UpdateCinematicActors ( void )
{
   actortype * actor;

   for (actor=firstcinematicactor;actor != NULL;)
      {
      if (UpdateCinematicEffect ( actor->effecttype, actor->effect ) == false)
         {
         actortype * nextactor;

         nextactor=actor->next;
         DeleteCinematicActor(actor);
         actor=nextactor;
         }
      else
         actor=actor->next;
      }
}

/*
===============
=
= DrawCinematicActors
=
===============
*/
typedef enum {
        screenfunctions,
        background,
        backgroundsprites,
        backdrop,
        foregroundsprites,
        palettefunctions,
        numdrawphases
} enum_drawphases;

void DrawCinematicActors ( void )
{
   actortype * actor;
   actortype * nextactor;
   boolean draw;
   enum_drawphases sequence;
#if DUMP
   int numactors=0;
#endif
   boolean flippage=true;

   for (sequence=screenfunctions;sequence<numdrawphases;sequence++)
      {
      for (actor=firstcinematicactor;actor != NULL;)
         {
         draw=false;
         switch (actor->effecttype)
            {
            case fadeout:
            case blankscreen:
            case clearbuffer:
            case cinematicend:
            case flic:
               if (sequence==screenfunctions)
                  draw=true;
               flippage=false;
               break;
            case palette:
               if (sequence==palettefunctions)
                  draw=true;
               flippage=false;
               break;
            case background_noscrolling:
            case background_scrolling:
            case background_multi:
               if (sequence==background)
                  draw=true;
               break;
            case sprite_background:
               if (sequence==backgroundsprites)
                  draw=true;
               break;
            case backdrop_noscrolling:
            case backdrop_scrolling:
               if (sequence==backdrop)
                  draw=true;
               break;
            case sprite_foreground:
               if (sequence==foregroundsprites)
                  draw=true;
               break;
            }
         nextactor=actor->next;
         if (draw==true)
            {
#if DUMP
            printf("drawing type=%ld\n",actor->effecttype);
#endif
            if (DrawCinematicEffect ( actor->effecttype, actor->effect ) == false)
               {
               DeleteCinematicActor(actor);
               }
#if DUMP
            numactors++;
#endif
            }
         actor=nextactor;
         }
      }
      if (flippage==true)
         XFlipPage ();
#if DUMP
      printf("Total actors drawn=%ld\n",numactors);
#endif
}


