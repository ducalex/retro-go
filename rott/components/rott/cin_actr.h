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
#ifndef _cin_actr_public
#define _cin_actr_public

#include "cin_glob.h"
#include "cin_def.h"

extern actortype * firstcinematicactor;
extern actortype * lastcinematicactor;

void AddCinematicActor ( actortype * actor );
void DeleteCinematicActor ( actortype * actor);

actortype * GetNewCinematicActor ( void );
void StartupCinematicActors ( void );
void ShutdownCinematicActors ( void );
void SpawnCinematicActor ( enum_eventtype type, void * effect );
void DrawCinematicActors ( void );
void UpdateCinematicActors ( void );

#endif

