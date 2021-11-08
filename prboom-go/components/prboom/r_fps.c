/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze, Andrey Budko
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      Uncapped framerate stuff
 *
 *---------------------------------------------------------------------
 */

#include "doomstat.h"
#include "r_defs.h"
#include "r_state.h"
#include "p_spec.h"
#include "r_demo.h"
#include "r_fps.h"

int movement_smooth = false;

typedef enum
{
  INTERP_SectorFloor,
  INTERP_SectorCeiling,
  INTERP_Vertex,
  INTERP_WallPanning,
  INTERP_FloorPanning,
  INTERP_CeilingPanning
} interpolation_type_e;

typedef struct
{
  interpolation_type_e type;
  void *address;
} interpolation_t;

static int numinterpolations = 0;

tic_vars_t tic_vars;

view_vars_t original_view_vars;

extern int realtic_clock_rate;
void D_Display(void);

void R_InitInterpolation(void)
{
  tic_vars.msec = realtic_clock_rate * TICRATE / 100000.0f;
}

typedef fixed_t fixed2_t[2];
static fixed2_t *oldipos;
static fixed2_t *bakipos;
static interpolation_t *curipos;

static boolean NoInterpolateView;
static boolean didInterp;
boolean WasRenderedInTryRunTics;

void R_InterpolateView (player_t *player, fixed_t frac)
{
  if (movement_smooth)
  {
    if (NoInterpolateView)
    {
      NoInterpolateView = false;
      original_view_vars.viewx = player->mo->x;
      original_view_vars.viewy = player->mo->y;
      original_view_vars.viewz = player->viewz;

      original_view_vars.viewangle = player->mo->angle + viewangleoffset;
    }

    viewx = original_view_vars.viewx + FixedMul (frac, player->mo->x - original_view_vars.viewx);
    viewy = original_view_vars.viewy + FixedMul (frac, player->mo->y - original_view_vars.viewy);
    viewz = original_view_vars.viewz + FixedMul (frac, player->viewz - original_view_vars.viewz);

    viewangle = original_view_vars.viewangle + FixedMul (frac, R_SmoothPlaying_Get(player->mo->angle) + viewangleoffset - original_view_vars.viewangle);
  }
  else
  {
    viewx = player->mo->x;
    viewy = player->mo->y;
    viewz = player->viewz;
    viewangle = R_SmoothPlaying_Get(player->mo->angle);
  }
}

void R_ResetViewInterpolation ()
{
  NoInterpolateView = true;
}

static void R_CopyInterpToOld (int i)
{
  switch (curipos[i].type)
  {
  case INTERP_SectorFloor:
    oldipos[i][0] = ((sector_t*)curipos[i].address)->floorheight;
    break;
  case INTERP_SectorCeiling:
    oldipos[i][0] = ((sector_t*)curipos[i].address)->ceilingheight;
    break;
  case INTERP_Vertex:
    oldipos[i][0] = ((vertex_t*)curipos[i].address)->x;
    oldipos[i][1] = ((vertex_t*)curipos[i].address)->y;
    break;
  case INTERP_WallPanning:
    oldipos[i][0] = ((side_t*)curipos[i].address)->rowoffset;
    oldipos[i][1] = ((side_t*)curipos[i].address)->textureoffset;
    break;
  case INTERP_FloorPanning:
    oldipos[i][0] = ((sector_t*)curipos[i].address)->floor_xoffs;
    oldipos[i][1] = ((sector_t*)curipos[i].address)->floor_yoffs;
    break;
  case INTERP_CeilingPanning:
    oldipos[i][0] = ((sector_t*)curipos[i].address)->ceiling_xoffs;
    oldipos[i][1] = ((sector_t*)curipos[i].address)->ceiling_yoffs;
    break;
  }
}

static void R_CopyBakToInterp (int i)
{
  switch (curipos[i].type)
  {
  case INTERP_SectorFloor:
    ((sector_t*)curipos[i].address)->floorheight = bakipos[i][0];
    break;
  case INTERP_SectorCeiling:
    ((sector_t*)curipos[i].address)->ceilingheight = bakipos[i][0];
    break;
  case INTERP_Vertex:
    ((vertex_t*)curipos[i].address)->x = bakipos[i][0];
    ((vertex_t*)curipos[i].address)->y = bakipos[i][1];
    break;
  case INTERP_WallPanning:
    ((side_t*)curipos[i].address)->rowoffset = bakipos[i][0];
    ((side_t*)curipos[i].address)->textureoffset = bakipos[i][1];
    break;
  case INTERP_FloorPanning:
    ((sector_t*)curipos[i].address)->floor_xoffs = bakipos[i][0];
    ((sector_t*)curipos[i].address)->floor_yoffs = bakipos[i][1];
    break;
  case INTERP_CeilingPanning:
    ((sector_t*)curipos[i].address)->ceiling_xoffs = bakipos[i][0];
    ((sector_t*)curipos[i].address)->ceiling_yoffs = bakipos[i][1];
    break;
  }
}

static void R_DoAnInterpolation (int i, fixed_t smoothratio)
{
  fixed_t pos;
  fixed_t *adr1 = NULL;
  fixed_t *adr2 = NULL;

  switch (curipos[i].type)
  {
  case INTERP_SectorFloor:
    adr1 = &((sector_t*)curipos[i].address)->floorheight;
    break;
  case INTERP_SectorCeiling:
    adr1 = &((sector_t*)curipos[i].address)->ceilingheight;
    break;
  case INTERP_Vertex:
    adr1 = &((vertex_t*)curipos[i].address)->x;
////    adr2 = &((vertex_t*)curipos[i].Address)->y;
    break;
  case INTERP_WallPanning:
    adr1 = &((side_t*)curipos[i].address)->rowoffset;
    adr2 = &((side_t*)curipos[i].address)->textureoffset;
    break;
  case INTERP_FloorPanning:
    adr1 = &((sector_t*)curipos[i].address)->floor_xoffs;
    adr2 = &((sector_t*)curipos[i].address)->floor_yoffs;
    break;
  case INTERP_CeilingPanning:
    adr1 = &((sector_t*)curipos[i].address)->ceiling_xoffs;
    adr2 = &((sector_t*)curipos[i].address)->ceiling_yoffs;
    break;

 default:
    return;
  }

  if (adr1)
  {
    pos = bakipos[i][0] = *adr1;
    *adr1 = oldipos[i][0] + FixedMul (pos - oldipos[i][0], smoothratio);
  }

  if (adr2)
  {
    pos = bakipos[i][1] = *adr2;
    *adr2 = oldipos[i][1] + FixedMul (pos - oldipos[i][1], smoothratio);
  }
}

void R_UpdateInterpolations()
{
  int i;
  if (!movement_smooth)
    return;
  for (i = numinterpolations-1; i >= 0; --i)
    R_CopyInterpToOld (i);
}

int interpolations_max = 0;

static void R_SetInterpolation(interpolation_type_e type, void *posptr)
{
  int i;
  if (!movement_smooth)
    return;
  
  if (numinterpolations >= interpolations_max) {
    interpolations_max = interpolations_max ? interpolations_max * 2 : 256;
    
    oldipos = (fixed2_t*)realloc(oldipos, sizeof(*oldipos) * interpolations_max);
    bakipos = (fixed2_t*)realloc(bakipos, sizeof(*bakipos) * interpolations_max);
    curipos = (interpolation_t*)realloc(curipos, sizeof(*curipos) * interpolations_max);
  }
  
  for(i = numinterpolations-1; i >= 0; i--)
    if (curipos[i].address == posptr && curipos[i].type == type)
      return;

  curipos[numinterpolations].address = posptr;
  curipos[numinterpolations].type = type;
  R_CopyInterpToOld (numinterpolations);
  numinterpolations++;
} 

static void R_StopInterpolation(interpolation_type_e type, void *posptr)
{
  int i;

  if (!movement_smooth)
    return;

  for(i=numinterpolations-1; i>= 0; --i)
  {
    if (curipos[i].address == posptr && curipos[i].type == type)
    {
      numinterpolations--;
      oldipos[i][0] = oldipos[numinterpolations][0];
      oldipos[i][1] = oldipos[numinterpolations][1];
      bakipos[i][0] = bakipos[numinterpolations][0];
      bakipos[i][1] = bakipos[numinterpolations][1];
      curipos[i] = curipos[numinterpolations];
      break;
    }
  }
}

void R_StopAllInterpolations(void)
{
  int i;
  
  if (!movement_smooth)
    return;

  for(i=numinterpolations-1; i>= 0; --i)
  {
    numinterpolations--;
    oldipos[i][0] = oldipos[numinterpolations][0];
    oldipos[i][1] = oldipos[numinterpolations][1];
    bakipos[i][0] = bakipos[numinterpolations][0];
    bakipos[i][1] = bakipos[numinterpolations][1];
    curipos[i] = curipos[numinterpolations];
  }
}

void R_DoInterpolations(fixed_t smoothratio)
{
  int i;
  if (!movement_smooth)
    return;

  if (smoothratio == FRACUNIT)
  {
    didInterp = false;
    return;
  }

  didInterp = true;

  for (i = numinterpolations-1; i >= 0; --i)
  {
    R_DoAnInterpolation (i, smoothratio);
  }
}

void R_RestoreInterpolations()
{
  int i;
  
  if (!movement_smooth)
    return;

  if (didInterp)
  {
    didInterp = false;
    for (i = numinterpolations-1; i >= 0; --i)
    {
      R_CopyBakToInterp (i);
    }
  }
}

void R_ActivateSectorInterpolations()
{
  int i;
  sector_t     *sec;

  if (!movement_smooth)
    return;

  for (i=0, sec = sectors ; i<numsectors ; i++,sec++)
  {
    if (sec->floordata)
      R_SetInterpolation (INTERP_SectorFloor, sec);
    if (sec->ceilingdata)
      R_SetInterpolation (INTERP_SectorCeiling, sec);
  }
}

static void R_InterpolationGetData(thinker_t *th,
  interpolation_type_e *type1, interpolation_type_e *type2,
  void **posptr1, void **posptr2)
{
  *posptr1 = NULL;
  *posptr2 = NULL;

  if (th->function == T_MoveFloor)
  {
    *type1 = INTERP_SectorFloor;
    *posptr1 = ((floormove_t *)th)->sector;
  }
  else
  if (th->function == T_PlatRaise)
  {
    *type1 = INTERP_SectorFloor;
    *posptr1 = ((plat_t *)th)->sector;
  }
  else
  if (th->function == T_MoveCeiling)
  {
    *type1 = INTERP_SectorCeiling;
    *posptr1 = ((ceiling_t *)th)->sector;
  }
  else
  if (th->function == T_VerticalDoor)
  {
    *type1 = INTERP_SectorCeiling;
    *posptr1 = ((vldoor_t *)th)->sector;
  }
  else
  if (th->function == T_MoveElevator)
  {
    *type1 = INTERP_SectorFloor;
    *posptr1 = ((elevator_t *)th)->sector;
    *type2 = INTERP_SectorCeiling;
    *posptr2 = ((elevator_t *)th)->sector;
  }
  else
  if (th->function == T_Scroll)
  {
    switch (((scroll_t *)th)->type)
    {
      case sc_side:
        *type1 = INTERP_WallPanning;
        *posptr1 = sides + ((scroll_t *)th)->affectee;
        break;
      case sc_floor:
        *type1 = INTERP_FloorPanning;
        *posptr1 = sectors + ((scroll_t *)th)->affectee;
        break;
      case sc_ceiling:
        *type1 = INTERP_CeilingPanning;
        *posptr1 = sectors + ((scroll_t *)th)->affectee;
        break;
      default: ;
    }
  }
}

void R_ActivateThinkerInterpolations(thinker_t *th)
{
  void *posptr1;
  void *posptr2;
  interpolation_type_e type1, type2;

  if (!movement_smooth)
    return;

  R_InterpolationGetData(th, &type1, &type2, &posptr1, &posptr2);

  if(posptr1)
  {
    R_SetInterpolation (type1, posptr1);
    
    if(posptr2)
      R_SetInterpolation (type2, posptr2);
  }
}

void R_StopInterpolationIfNeeded(thinker_t *th)
{
  void *posptr1;
  void *posptr2;
  interpolation_type_e type1, type2;

  if (!movement_smooth)
    return;

  R_InterpolationGetData(th, &type1, &type2, &posptr1, &posptr2);

  if(posptr1)
  {
    R_StopInterpolation (type1, posptr1);
    if(posptr2)
      R_StopInterpolation (type2, posptr2);
  }
}

