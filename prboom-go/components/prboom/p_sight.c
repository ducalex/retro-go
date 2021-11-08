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
 *      LineOfSight/Visibility checks, uses REJECT Lookup Table.
 *
 *-----------------------------------------------------------------------------*/

#include "doomstat.h"
#include "r_main.h"
#include "p_map.h"
#include "p_maputl.h"
#include "p_setup.h"
#include "m_bbox.h"
#include "lprintf.h"

//
// P_CheckSight
//
// killough 4/19/98:
// Convert LOS info to struct for reentrancy and efficiency of data locality

typedef struct {
  fixed_t sightzstart, t2x, t2y;   // eye z of looker
  divline_t strace;                // from t1 to t2
  fixed_t topslope, bottomslope;   // slopes to top and bottom of target
  fixed_t bbox[4];
  fixed_t maxz,minz;               // cph - z optimisations for 2sided lines
} los_t;

static los_t los; // cph - made static

//
// P_DivlineSide
// Returns side 0 (front), 1 (back), or 2 (on).
//
// killough 4/19/98: made static, cleaned up

inline static int P_DivlineSide(fixed_t x, fixed_t y, const divline_t *node)
{
  fixed_t left, right;
  return
    !node->dx ? x == node->x ? 2 : x <= node->x ? node->dy > 0 : node->dy < 0 :
    !node->dy ? ( compatibility_level < prboom_4_compatibility ? x : y) == node->y ? 2 : y <= node->y ? node->dx < 0 : node->dx > 0 :
    (right = ((y - node->y) >> FRACBITS) * (node->dx >> FRACBITS)) <
    (left  = ((x - node->x) >> FRACBITS) * (node->dy >> FRACBITS)) ? 0 :
    right == left ? 2 : 1;
}

//
// P_CrossSubsector
// Returns true
//  if strace crosses the given subsector successfully.
//
// killough 4/19/98: made static and cleaned up

static boolean P_CrossSubsector(int num)
{
  seg_t *seg = segs + subsectors[num].firstline;
  int count;
  fixed_t opentop = 0, openbottom = 0;
  const sector_t *front = NULL, *back = NULL;

#ifdef RANGECHECK
  if (num >= numsubsectors)
    I_Error("P_CrossSubsector: ss %i with numss = %i", num, numsubsectors);
#endif

  for (count = subsectors[num].numlines; --count >= 0; seg++) { // check lines
    line_t *line = seg->linedef;
    divline_t divl;

   if(!line) // figgi -- skip minisegs
     continue;

    // allready checked other side?
    if (line->validcount == validcount)
      continue;

    line->validcount = validcount;

    /* OPTIMIZE: killough 4/20/98: Added quick bounding-box rejection test
     * cph - this is causing demo desyncs on original Doom demos.
     *  Who knows why. Exclude test for those.
     */
    if (!demo_compatibility)
    if (line->bbox[BOXLEFT  ] > los.bbox[BOXRIGHT ] ||
  line->bbox[BOXRIGHT ] < los.bbox[BOXLEFT  ] ||
  line->bbox[BOXBOTTOM] > los.bbox[BOXTOP   ] ||
  line->bbox[BOXTOP]    < los.bbox[BOXBOTTOM])
      continue;

    // cph - do what we can before forced to check intersection
    if (line->flags & ML_TWOSIDED) {

      // no wall to block sight with?
      if ((front = seg->frontsector)->floorheight ==
    (back = seg->backsector)->floorheight   &&
    front->ceilingheight == back->ceilingheight)
  continue;

      // possible occluder
      // because of ceiling height differences
      opentop = front->ceilingheight < back->ceilingheight ?
  front->ceilingheight : back->ceilingheight ;

      // because of floor height differences
      openbottom = front->floorheight > back->floorheight ?
  front->floorheight : back->floorheight ;

      // cph - reject if does not intrude in the z-space of the possible LOS
      if ((opentop >= los.maxz) && (openbottom <= los.minz))
  continue;
    }

    { // Forget this line if it doesn't cross the line of sight
      const vertex_t *v1,*v2;

      v1 = line->v1;
      v2 = line->v2;

      if (P_DivlineSide(v1->x, v1->y, &los.strace) ==
          P_DivlineSide(v2->x, v2->y, &los.strace))
        continue;

      divl.dx = v2->x - (divl.x = v1->x);
      divl.dy = v2->y - (divl.y = v1->y);

      // line isn't crossed?
      if (P_DivlineSide(los.strace.x, los.strace.y, &divl) ==
    P_DivlineSide(los.t2x, los.t2y, &divl))
  continue;
    }

    // cph - if bottom >= top or top < minz or bottom > maxz then it must be
    // solid wrt this LOS
    if (!(line->flags & ML_TWOSIDED) || (openbottom >= opentop) ||
  (opentop < los.minz) || (openbottom > los.maxz))
  return false;

    { // crosses a two sided line
      /* cph 2006/07/15 - oops, we missed this in 2.4.0 & .1;
       *  use P_InterceptVector2 for those compat levels only. */ 
      fixed_t frac = (compatibility_level == prboom_5_compatibility || compatibility_level == prboom_6_compatibility) ?
		      P_InterceptVector2(&los.strace, &divl) : 
		      P_InterceptVector(&los.strace, &divl);

      if (front->floorheight != back->floorheight) {
        fixed_t slope = FixedDiv(openbottom - los.sightzstart , frac);
        if (slope > los.bottomslope)
            los.bottomslope = slope;
      }

      if (front->ceilingheight != back->ceilingheight)
        {
          fixed_t slope = FixedDiv(opentop - los.sightzstart , frac);
          if (slope < los.topslope)
            los.topslope = slope;
        }

      if (los.topslope <= los.bottomslope)
        return false;               // stop
    }
  }
  // passed the subsector ok
  return true;
}

//
// P_CrossBSPNode
// Returns true
//  if strace crosses the given node successfully.
//
// killough 4/20/98: rewritten to remove tail recursion, clean up, and optimize
// cph - Made to use R_PointOnSide instead of P_DivlineSide, since the latter
//  could return 2 which was ambigous, and the former is
//  better optimised; also removes two casts :-)

static boolean P_CrossBSPNode_LxDoom(int bspnum)
{
  while (!(bspnum & NF_SUBSECTOR))
    {
      register const node_t *bsp = nodes + bspnum;
      int side,side2;
      side = R_PointOnSide(los.strace.x, los.strace.y, bsp);
      side2 = R_PointOnSide(los.t2x, los.t2y, bsp);
      if (side == side2)
         bspnum = bsp->children[side]; // doesn't touch the other side
      else         // the partition plane is crossed here
        if (!P_CrossBSPNode_LxDoom(bsp->children[side]))
          return 0;  // cross the starting side
        else
          bspnum = bsp->children[side^1];  // cross the ending side
    }
  return P_CrossSubsector(bspnum == -1 ? 0 : bspnum & ~NF_SUBSECTOR);
}

static boolean P_CrossBSPNode_PrBoom(int bspnum)
{
  while (!(bspnum & NF_SUBSECTOR))
    {
      register const node_t *bsp = nodes + bspnum;
      int side,side2;
      side = P_DivlineSide(los.strace.x,los.strace.y,(const divline_t *)bsp)&1;
      side2= P_DivlineSide(los.t2x, los.t2y, (const divline_t *) bsp);
      if (side == side2)
         bspnum = bsp->children[side]; // doesn't touch the other side
      else         // the partition plane is crossed here
        if (!P_CrossBSPNode_PrBoom(bsp->children[side]))
          return 0;  // cross the starting side
        else
          bspnum = bsp->children[side^1];  // cross the ending side
    }
  return P_CrossSubsector(bspnum == -1 ? 0 : bspnum & ~NF_SUBSECTOR);
}

/* proff - Moved the compatibility check outside the functions
 * this gives a slight speedup
 */
static boolean P_CrossBSPNode(int bspnum)
{
  /* cph - LxDoom used some R_* funcs here */
  if (compatibility_level == lxdoom_1_compatibility)
    return P_CrossBSPNode_LxDoom(bspnum);
  else
    return P_CrossBSPNode_PrBoom(bspnum);
}

//
// P_CheckSight
// Returns true
//  if a straight line between t1 and t2 is unobstructed.
// Uses REJECT.
//
// killough 4/20/98: cleaned up, made to use new LOS struct

boolean P_CheckSight(mobj_t *t1, mobj_t *t2)
{
  const sector_t *s1 = t1->subsector->sector;
  const sector_t *s2 = t2->subsector->sector;
  int pnum = (s1-sectors)*numsectors + (s2-sectors);

  // First check for trivial rejection.
  // Determine subsector entries in REJECT table.
  //
  // Check in REJECT table.

  if (rejectmatrix[pnum>>3] & (1 << (pnum&7)))   // can't possibly be connected
    return false;

  // killough 4/19/98: make fake floors and ceilings block monster view

  if ((s1->heightsec != -1 &&
       ((t1->z + t1->height <= sectors[s1->heightsec].floorheight &&
         t2->z >= sectors[s1->heightsec].floorheight) ||
        (t1->z >= sectors[s1->heightsec].ceilingheight &&
         t2->z + t1->height <= sectors[s1->heightsec].ceilingheight)))
      ||
      (s2->heightsec != -1 &&
       ((t2->z + t2->height <= sectors[s2->heightsec].floorheight &&
         t1->z >= sectors[s2->heightsec].floorheight) ||
        (t2->z >= sectors[s2->heightsec].ceilingheight &&
         t1->z + t2->height <= sectors[s2->heightsec].ceilingheight))))
    return false;

  /* killough 11/98: shortcut for melee situations
   * same subsector? obviously visible
   * cph - compatibility optioned for demo sync, cf HR06-UV.LMP */
  if ((t1->subsector == t2->subsector) &&
      (compatibility_level >= mbf_compatibility))
    return true;

  // An unobstructed LOS is possible.
  // Now look from eyes of t1 to any part of t2.

  validcount++;

  los.topslope = (los.bottomslope = t2->z - (los.sightzstart =
                                             t1->z + t1->height -
                                             (t1->height>>2))) + t2->height;
  los.strace.dx = (los.t2x = t2->x) - (los.strace.x = t1->x);
  los.strace.dy = (los.t2y = t2->y) - (los.strace.y = t1->y);

  if (t1->x > t2->x)
    los.bbox[BOXRIGHT] = t1->x, los.bbox[BOXLEFT] = t2->x;
  else
    los.bbox[BOXRIGHT] = t2->x, los.bbox[BOXLEFT] = t1->x;

  if (t1->y > t2->y)
    los.bbox[BOXTOP] = t1->y, los.bbox[BOXBOTTOM] = t2->y;
  else
    los.bbox[BOXTOP] = t2->y, los.bbox[BOXBOTTOM] = t1->y;

  /* cph - calculate min and max z of the potential line of sight
   * For old demos, we disable this optimisation by setting them to
   * the extremes */
  switch (compatibility_level) {
  case lxdoom_1_compatibility:
    if (los.sightzstart < t2->z) {
      los.maxz = t2->z + t2->height; los.minz = los.sightzstart;
    } else if (los.sightzstart > t2->z + t2->height) {
      los.maxz = los.sightzstart; los.minz = t2->z;
    } else {
      los.maxz = t2->z + t2->height; los.minz = t2->z;
    }
    break;
  default:
    los.maxz = INT_MAX; los.minz = INT_MIN;
  }

  // the head node is the last node output
  return P_CrossBSPNode(numnodes-1);
}
