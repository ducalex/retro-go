/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
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
 *      BSP traversal, handling of LineSegs for rendering.
 *
 *-----------------------------------------------------------------------------*/

#include "doomstat.h"
#include "m_bbox.h"
#include "r_main.h"
#include "r_segs.h"
#include "r_plane.h"
#include "r_things.h"
#include "r_bsp.h" // cph - sanity checking
#include "v_video.h"
#include "lprintf.h"

seg_t     *curline;
side_t    *sidedef;
line_t    *linedef;
sector_t  *frontsector;
sector_t  *backsector;
drawseg_t *ds_p;

// killough 4/7/98: indicates doors closed wrt automap bugfix:
// cph - replaced by linedef rendering flags - int      doorclosed;

// killough: New code which removes 2s linedef limit
drawseg_t *drawsegs;
unsigned  maxdrawsegs;
// drawseg_t drawsegs[MAXDRAWSEGS];       // old code -- killough

//
// R_ClearDrawSegs
//

void R_ClearDrawSegs(void)
{
  ds_p = drawsegs;
}

// CPhipps -
// Instead of clipsegs, let's try using an array with one entry for each column,
// indicating whether it's blocked by a solid wall yet or not.

byte solidcol[MAX_SCREENWIDTH];

// CPhipps -
// R_ClipWallSegment
//
// Replaces the old R_Clip*WallSegment functions. It draws bits of walls in those
// columns which aren't solid, and updates the solidcol[] array appropriately

static void R_ClipWallSegment(int first, int last, boolean solid)
{
  byte *p;
  while (first < last) {
    if (solidcol[first]) {
      if (!(p = memchr(solidcol+first, 0, last-first))) return; // All solid
      first = p - solidcol;
    } else {
      int to;
      if (!(p = memchr(solidcol+first, 1, last-first))) to = last;
      else to = p - solidcol;
      R_StoreWallRange(first, to-1);
      if (solid) {
  memset(solidcol+first,1,to-first);
      }
  first = to;
    }
  }
}

//
// R_ClearClipSegs
//

void R_ClearClipSegs (void)
{
  memset(solidcol, 0, SCREENWIDTH);
}

// killough 1/18/98 -- This function is used to fix the automap bug which
// showed lines behind closed doors simply because the door had a dropoff.
//
// cph - converted to R_RecalcLineFlags. This recalculates all the flags for
// a line, including closure and texture tiling.

static void R_RecalcLineFlags(void)
{
  linedef->r_validcount = gametic;

  /* First decide if the line is closed, normal, or invisible */
  if (!(linedef->flags & ML_TWOSIDED)
      || backsector->ceilingheight <= frontsector->floorheight
      || backsector->floorheight >= frontsector->ceilingheight
      || (
    // if door is closed because back is shut:
    backsector->ceilingheight <= backsector->floorheight

    // preserve a kind of transparent door/lift special effect:
    && (backsector->ceilingheight >= frontsector->ceilingheight ||
        curline->sidedef->toptexture)

    && (backsector->floorheight <= frontsector->floorheight ||
        curline->sidedef->bottomtexture)

    // properly render skies (consider door "open" if both ceilings are sky):
    && (backsector->ceilingpic !=skyflatnum ||
        frontsector->ceilingpic!=skyflatnum)
    )
      )
    linedef->r_flags = RF_CLOSED;
  else {
    // Reject empty lines used for triggers
    //  and special events.
    // Identical floor and ceiling on both sides,
    // identical light levels on both sides,
    // and no middle texture.
    // CPhipps - recode for speed, not certain if this is portable though
    if (backsector->ceilingheight != frontsector->ceilingheight
  || backsector->floorheight != frontsector->floorheight
  || curline->sidedef->midtexture
  || memcmp(&backsector->floor_xoffs, &frontsector->floor_xoffs,
      sizeof(frontsector->floor_xoffs) + sizeof(frontsector->floor_yoffs) +
      sizeof(frontsector->ceiling_xoffs) + sizeof(frontsector->ceiling_yoffs) +
      sizeof(frontsector->ceilingpic) + sizeof(frontsector->floorpic) +
      sizeof(frontsector->lightlevel) + sizeof(frontsector->floorlightsec) +
      sizeof(frontsector->ceilinglightsec))) {
      linedef->r_flags = 0; return;
    } else
      linedef->r_flags = RF_IGNORE;
  }

  /* cph - I'm too lazy to try and work with offsets in this */
  if (curline->sidedef->rowoffset) return;

  /* Now decide on texture tiling */
  if (linedef->flags & ML_TWOSIDED) {
    int c;

    /* Does top texture need tiling */
    if ((c = frontsector->ceilingheight - backsector->ceilingheight) > 0 &&
   (textureheight[texturetranslation[curline->sidedef->toptexture]] > c))
      linedef->r_flags |= RF_TOP_TILE;

    /* Does bottom texture need tiling */
    if ((c = frontsector->floorheight - backsector->floorheight) > 0 &&
   (textureheight[texturetranslation[curline->sidedef->bottomtexture]] > c))
      linedef->r_flags |= RF_BOT_TILE;
  } else {
    int c;
    /* Does middle texture need tiling */
    if ((c = frontsector->ceilingheight - frontsector->floorheight) > 0 &&
   (textureheight[texturetranslation[curline->sidedef->midtexture]] > c))
      linedef->r_flags |= RF_MID_TILE;
  }
}

//
// killough 3/7/98: Hack floor/ceiling heights for deep water etc.
//
// If player's view height is underneath fake floor, lower the
// drawn ceiling to be just under the floor height, and replace
// the drawn floor and ceiling textures, and light level, with
// the control sector's.
//
// Similar for ceiling, only reflected.
//
// killough 4/11/98, 4/13/98: fix bugs, add 'back' parameter
//

sector_t *R_FakeFlat(sector_t *sec, sector_t *tempsec,
                     int *floorlightlevel, int *ceilinglightlevel,
                     boolean back)
{
  if (floorlightlevel)
    *floorlightlevel = sec->floorlightsec == -1 ?
      sec->lightlevel : sectors[sec->floorlightsec].lightlevel;

  if (ceilinglightlevel)
    *ceilinglightlevel = sec->ceilinglightsec == -1 ? // killough 4/11/98
      sec->lightlevel : sectors[sec->ceilinglightsec].lightlevel;

  if (sec->heightsec != -1)
    {
      const sector_t *s = &sectors[sec->heightsec];
      int heightsec = viewplayer->mo->subsector->sector->heightsec;
      int underwater = heightsec!=-1 && viewz<=sectors[heightsec].floorheight;

      // Replace sector being drawn, with a copy to be hacked
      *tempsec = *sec;

      // Replace floor and ceiling height with other sector's heights.
      tempsec->floorheight   = s->floorheight;
      tempsec->ceilingheight = s->ceilingheight;

      // killough 11/98: prevent sudden light changes from non-water sectors:
      if (underwater && (tempsec->  floorheight = sec->floorheight,
                          tempsec->ceilingheight = s->floorheight-1, !back))
        {                   // head-below-floor hack
          tempsec->floorpic    = s->floorpic;
          tempsec->floor_xoffs = s->floor_xoffs;
          tempsec->floor_yoffs = s->floor_yoffs;

          if (underwater) {
            if (s->ceilingpic == skyflatnum) {
		tempsec->floorheight   = tempsec->ceilingheight+1;
		tempsec->ceilingpic    = tempsec->floorpic;
                tempsec->ceiling_xoffs = tempsec->floor_xoffs;
                tempsec->ceiling_yoffs = tempsec->floor_yoffs;
	    } else {
		tempsec->ceilingpic    = s->ceilingpic;
		tempsec->ceiling_xoffs = s->ceiling_xoffs;
		tempsec->ceiling_yoffs = s->ceiling_yoffs;
	    }
	  }

          tempsec->lightlevel  = s->lightlevel;

          if (floorlightlevel)
            *floorlightlevel = s->floorlightsec == -1 ? s->lightlevel :
            sectors[s->floorlightsec].lightlevel; // killough 3/16/98

          if (ceilinglightlevel)
            *ceilinglightlevel = s->ceilinglightsec == -1 ? s->lightlevel :
            sectors[s->ceilinglightsec].lightlevel; // killough 4/11/98
        }
      else
        if (heightsec != -1 && viewz >= sectors[heightsec].ceilingheight &&
            sec->ceilingheight > s->ceilingheight)
          {   // Above-ceiling hack
            tempsec->ceilingheight = s->ceilingheight;
            tempsec->floorheight   = s->ceilingheight + 1;

            tempsec->floorpic    = tempsec->ceilingpic    = s->ceilingpic;
            tempsec->floor_xoffs = tempsec->ceiling_xoffs = s->ceiling_xoffs;
            tempsec->floor_yoffs = tempsec->ceiling_yoffs = s->ceiling_yoffs;

            if (s->floorpic != skyflatnum)
              {
                tempsec->ceilingheight = sec->ceilingheight;
                tempsec->floorpic      = s->floorpic;
                tempsec->floor_xoffs   = s->floor_xoffs;
                tempsec->floor_yoffs   = s->floor_yoffs;
              }

            tempsec->lightlevel  = s->lightlevel;

            if (floorlightlevel)
              *floorlightlevel = s->floorlightsec == -1 ? s->lightlevel :
              sectors[s->floorlightsec].lightlevel; // killough 3/16/98

            if (ceilinglightlevel)
              *ceilinglightlevel = s->ceilinglightsec == -1 ? s->lightlevel :
              sectors[s->ceilinglightsec].lightlevel; // killough 4/11/98
          }
      sec = tempsec;               // Use other sector
    }
  return sec;
}

//
// R_AddLine
// Clips the given segment
// and adds any visible pieces to the line list.
//

static void R_AddLine (seg_t *line)
{
  int      x1;
  int      x2;
  angle_t  angle1;
  angle_t  angle2;
  angle_t  span;
  angle_t  tspan;
  static sector_t tempsec;     // killough 3/8/98: ceiling/water hack

  curline = line;

  angle1 = R_PointToAngle (line->v1->x, line->v1->y);
  angle2 = R_PointToAngle (line->v2->x, line->v2->y);

  // Clip to view edges.
  span = angle1 - angle2;

  // Back side, i.e. backface culling
  if (span >= ANG180)
    return;

  // Global angle needed by segcalc.
  rw_angle1 = angle1;
  angle1 -= viewangle;
  angle2 -= viewangle;

  tspan = angle1 + clipangle;
  if (tspan > 2*clipangle)
    {
      tspan -= 2*clipangle;

      // Totally off the left edge?
      if (tspan >= span)
        return;

      angle1 = clipangle;
    }

  tspan = clipangle - angle2;
  if (tspan > 2*clipangle)
    {
      tspan -= 2*clipangle;

      // Totally off the left edge?
      if (tspan >= span)
        return;
      angle2 = 0-clipangle;
    }

  // The seg is in the view range,
  // but not necessarily visible.

  angle1 = (angle1+ANG90)>>ANGLETOFINESHIFT;
  angle2 = (angle2+ANG90)>>ANGLETOFINESHIFT;

  // killough 1/31/98: Here is where "slime trails" can SOMETIMES occur:
  x1 = viewangletox[angle1];
  x2 = viewangletox[angle2];

  // Does not cross a pixel?
  if (x1 >= x2)       // killough 1/31/98 -- change == to >= for robustness
    return;

  backsector = line->backsector;

  // Single sided line?
  if (backsector)
    // killough 3/8/98, 4/4/98: hack for invisible ceilings / deep water
    backsector = R_FakeFlat(backsector, &tempsec, NULL, NULL, true);

  /* cph - roll up linedef properties in flags */
  if ((linedef = curline->linedef)->r_validcount != gametic)
    R_RecalcLineFlags();

  if (linedef->r_flags & RF_IGNORE)
  {
    return;
  }
  else
    R_ClipWallSegment (x1, x2, linedef->r_flags & RF_CLOSED);
}

//
// R_CheckBBox
// Checks BSP node/subtree bounding box.
// Returns true
//  if some part of the bbox might be visible.
//

static const int checkcoord[12][4] = // killough -- static const
{
  {3,0,2,1},
  {3,0,2,0},
  {3,1,2,0},
  {0},
  {2,0,2,1},
  {0,0,0,0},
  {3,1,3,0},
  {0},
  {2,0,3,1},
  {2,1,3,1},
  {2,1,3,0}
};

// killough 1/28/98: static // CPhipps - const parameter, reformatted
static boolean R_CheckBBox(const fixed_t *bspcoord)
{
  angle_t angle1, angle2;

  {
    int        boxpos;
    const int* check;

    // Find the corners of the box
    // that define the edges from current viewpoint.
    boxpos = (viewx <= bspcoord[BOXLEFT] ? 0 : viewx < bspcoord[BOXRIGHT ] ? 1 : 2) +
      (viewy >= bspcoord[BOXTOP ] ? 0 : viewy > bspcoord[BOXBOTTOM] ? 4 : 8);

    if (boxpos == 5)
      return true;

    check = checkcoord[boxpos];
    angle1 = R_PointToAngle (bspcoord[check[0]], bspcoord[check[1]]) - viewangle;
    angle2 = R_PointToAngle (bspcoord[check[2]], bspcoord[check[3]]) - viewangle;
  }

  // cph - replaced old code, which was unclear and badly commented
  // Much more efficient code now
  if ((signed)angle1 < (signed)angle2) { /* it's "behind" us */
    /* Either angle1 or angle2 is behind us, so it doesn't matter if we
     * change it to the corect sign
     */
    if ((angle1 >= ANG180) && (angle1 < ANG270))
      angle1 = INT_MAX; /* which is ANG180-1 */
    else
      angle2 = INT_MIN;
  }

  if ((signed)angle2 >= (signed)clipangle) return false; // Both off left edge
  if ((signed)angle1 <= -(signed)clipangle) return false; // Both off right edge
  if ((signed)angle1 >= (signed)clipangle) angle1 = clipangle; // Clip at left edge
  if ((signed)angle2 <= -(signed)clipangle) angle2 = 0-clipangle; // Clip at right edge

  // Find the first clippost
  //  that touches the source post
  //  (adjacent pixels are touching).
  angle1 = (angle1+ANG90)>>ANGLETOFINESHIFT;
  angle2 = (angle2+ANG90)>>ANGLETOFINESHIFT;
  {
    int sx1 = viewangletox[angle1];
    int sx2 = viewangletox[angle2];
    //    const cliprange_t *start;

    // Does not cross a pixel.
    if (sx1 == sx2)
      return false;

    if (!memchr(solidcol+sx1, 0, sx2-sx1)) return false;
    // All columns it covers are already solidly covered
  }

  return true;
}

//
// R_Subsector
// Determine floor/ceiling planes.
// Add sprites of things in sector.
// Draw one or more line segments.
//
// killough 1/31/98 -- made static, polished

static void R_Subsector(int num)
{
  int         count;
  seg_t       *line;
  subsector_t *sub;
  sector_t    tempsec;              // killough 3/7/98: deep water hack
  int         floorlightlevel;      // killough 3/16/98: set floor lightlevel
  int         ceilinglightlevel;    // killough 4/11/98

#ifdef RANGECHECK
  if (num>=numsubsectors)
    I_Error ("R_Subsector: ss %i with numss = %i", num, numsubsectors);
#endif

  sub = &subsectors[num];
  frontsector = sub->sector;
  count = sub->numlines;
  line = &segs[sub->firstline];

  // killough 3/8/98, 4/4/98: Deep water / fake ceiling effect
  frontsector = R_FakeFlat(frontsector, &tempsec, &floorlightlevel,
                           &ceilinglightlevel, false);   // killough 4/11/98

  // killough 3/7/98: Add (x,y) offsets to flats, add deep water check
  // killough 3/16/98: add floorlightlevel
  // killough 10/98: add support for skies transferred from sidedefs

  floorplane = frontsector->floorheight < viewz || // killough 3/7/98
    (frontsector->heightsec != -1 &&
     sectors[frontsector->heightsec].ceilingpic == skyflatnum) ?
    R_FindPlane(frontsector->floorheight,
                frontsector->floorpic == skyflatnum &&  // kilough 10/98
                frontsector->sky & PL_SKYFLAT ? frontsector->sky :
                frontsector->floorpic,
                floorlightlevel,                // killough 3/16/98
                frontsector->floor_xoffs,       // killough 3/7/98
                frontsector->floor_yoffs
                ) : NULL;

  ceilingplane = frontsector->ceilingheight > viewz ||
    frontsector->ceilingpic == skyflatnum ||
    (frontsector->heightsec != -1 &&
     sectors[frontsector->heightsec].floorpic == skyflatnum) ?
    R_FindPlane(frontsector->ceilingheight,     // killough 3/8/98
                frontsector->ceilingpic == skyflatnum &&  // kilough 10/98
                frontsector->sky & PL_SKYFLAT ? frontsector->sky :
                frontsector->ceilingpic,
                ceilinglightlevel,              // killough 4/11/98
                frontsector->ceiling_xoffs,     // killough 3/7/98
                frontsector->ceiling_yoffs
                ) : NULL;

  // killough 9/18/98: Fix underwater slowdown, by passing real sector
  // instead of fake one. Improve sprite lighting by basing sprite
  // lightlevels on floor & ceiling lightlevels in the surrounding area.
  //
  // 10/98 killough:
  //
  // NOTE: TeamTNT fixed this bug incorrectly, messing up sprite lighting!!!
  // That is part of the 242 effect!!!  If you simply pass sub->sector to
  // the old code you will not get correct lighting for underwater sprites!!!
  // Either you must pass the fake sector and handle validcount here, on the
  // real sector, or you must account for the lighting in some other way,
  // like passing it as an argument.

  R_AddSprites(sub, (floorlightlevel+ceilinglightlevel)/2);
  while (count--)
  {
    if (line->miniseg == false)
      R_AddLine (line);
    line++;
    curline = NULL; /* cph 2001/11/18 - must clear curline now we're done with it, so R_ColourMap doesn't try using it for other things */
  }
}

//
// RenderBSPNode
// Renders all subsectors below a given node,
//  traversing subtree recursively.
// Just call with BSP root.
//
// killough 5/2/98: reformatted, removed tail recursion

void R_RenderBSPNode(int bspnum)
{
  while (!(bspnum & NF_SUBSECTOR))  // Found a subsector?
    {
      const node_t *bsp = &nodes[bspnum];

      // Decide which side the view point is on.
      int side = R_PointOnSide(viewx, viewy, bsp);
      // Recursively divide front space.
      R_RenderBSPNode(bsp->children[side]);

      // Possibly divide back space.

      if (!R_CheckBBox(bsp->bbox[side^1]))
        return;

      bspnum = bsp->children[side^1];
    }
  R_Subsector(bspnum == -1 ? 0 : bspnum & ~NF_SUBSECTOR);
}
