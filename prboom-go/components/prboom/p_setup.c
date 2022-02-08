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
 *  Do all the WAD I/O, get map description,
 *  set up initial state and misc. LUTs.
 *
 *-----------------------------------------------------------------------------*/

#include <math.h>

#include "doomstat.h"
#include "m_bbox.h"
#include "m_argv.h"
#include "g_game.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_things.h"
#include "p_maputl.h"
#include "p_map.h"
#include "p_setup.h"
#include "p_spec.h"
#include "p_tick.h"
#include "p_enemy.h"
#include "s_sound.h"
#include "lprintf.h" //jff 10/6/98 for debug outputs
#include "v_video.h"
#include "r_demo.h"
#include "r_fps.h"

//
// MAP related Lookup tables.
// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
//

int      numvertexes;
vertex_t *vertexes;

int      numsegs;
seg_t    *segs;

int      numsectors;
sector_t *sectors;

int      numsubsectors;
subsector_t *subsectors;

int      numnodes;
node_t   *nodes;

int      numlines;
line_t   *lines;

int      numsides;
side_t   *sides;


////////////////////////////////////////////////////////////////////////////////////////////
// figgi 08/21/00 -- constants and globals for glBsp support
#define gNd2            0x32644E67  // figgi -- suppport for new GL_VERT format v2.0
#define gNd3            0x33644E67
#define gNd4            0x34644E67
#define gNd5            0x35644E67
#define ZNOD            0x444F4E5A
#define ZGLN            0x4E4C475A
#define GL_VERT_OFFSET  4

static int     firstglvertex = 0;
static int     nodesVersion  = 0;

// figgi 08/21/00 -- glSegs
typedef struct
{
  unsigned short  v1;    // start vertex    (16 bit)
  unsigned short  v2;    // end vertex      (16 bit)
  unsigned short  linedef; // linedef, or -1 for minisegs
  short     side;  // side on linedef: 0 for right, 1 for left
  unsigned short  partner; // corresponding partner seg, or -1 on one-sided walls
} glseg_t;

// fixed 32 bit gl_vert format v2.0+ (glBsp 1.91)
typedef struct
{
  fixed_t x,y;
} mapglvertex_t;

enum
{
   ML_GL_LABEL=0,  // A separator name, GL_ExMx or GL_MAPxx
   ML_GL_VERTS,     // Extra Vertices
   ML_GL_SEGS,     // Segs, from linedefs & minisegs
   ML_GL_SSECT,    // SubSectors, list of segs
   ML_GL_NODES     // GL BSP nodes
};
////////////////////////////////////////////////////////////////////////////////////////////


// BLOCKMAP
// Created from axis aligned bounding box
// of the map, a rectangular array of
// blocks of size ...
// Used to speed up collision detection
// by spatial subdivision in 2D.
//
// Blockmap size.

int       bmapwidth, bmapheight;  // size in mapblocks

// killough 3/1/98: remove blockmap limit internally:
long      *blockmap;              // was short -- killough

// offsets in blockmap are from here
long      *blockmaplump;          // was short -- killough

fixed_t   bmaporgx, bmaporgy;     // origin of block map

mobj_t    **blocklinks;           // for thing chains

//
// REJECT
// For fast sight rejection.
// Speeds up enemy AI by skipping detailed
//  LineOf Sight calculation.
// Without the special effect, this could
// be used as a PVS lookup as well.
//

static int rejectlump = -1;// cph - store reject lump num if cached
const byte *rejectmatrix; // cph - const*

// Maintain single and multi player starting spots.

// 1/11/98 killough: Remove limit on deathmatch starts
mapthing_t *deathmatchstarts;      // killough
size_t     num_deathmatchstarts;   // killough

mapthing_t *deathmatch_p;
mapthing_t playerstarts[MAXPLAYERS];

//
// P_CheckForZDoomNodes
//

static boolean P_CheckForZDoomNodes(int lumpnum, int gl_lumpnum)
{
  const void *data;

  data = W_CacheLumpNum(lumpnum + ML_NODES);
  if (*(const int *)data == ZNOD)
    I_Error("P_CheckForZDoomNodes: ZDoom nodes not supported yet");

  data = W_CacheLumpNum(lumpnum + ML_SSECTORS);
  if (*(const int *)data == ZGLN)
    I_Error("P_CheckForZDoomNodes: ZDoom GL nodes not supported yet");

  return false;
}

//
// P_GetNodesVersion
//

static void P_GetNodesVersion(int lumpnum, int gl_lumpnum)
{
  const void *data;

  data = W_CacheLumpNum(gl_lumpnum+ML_GL_VERTS);
  if ( (gl_lumpnum > lumpnum) && (compatibility_level >= prboom_2_compatibility) ) {
    if (*(const int *)data == gNd2) {
      data = W_CacheLumpNum(gl_lumpnum+ML_GL_SEGS);
      if (*(const int *)data == gNd3) {
        nodesVersion = gNd3;
        lprintf(LO_DEBUG, "P_GetNodesVersion: found version 3 nodes\n");
        I_Error("P_GetNodesVersion: version 3 nodes not supported\n");
      } else {
        nodesVersion = gNd2;
        lprintf(LO_DEBUG, "P_GetNodesVersion: found version 2 nodes\n");
      }
    }
    if (*(const int *)data == gNd4) {
      nodesVersion = gNd4;
      lprintf(LO_DEBUG, "P_GetNodesVersion: found version 4 nodes\n");
      I_Error("P_GetNodesVersion: version 4 nodes not supported\n");
    }
    if (*(const int *)data == gNd5) {
      nodesVersion = gNd5;
      lprintf(LO_DEBUG, "P_GetNodesVersion: found version 5 nodes\n");
      I_Error("P_GetNodesVersion: version 5 nodes not supported\n");
    }
  } else {
    nodesVersion = 0;
    lprintf(LO_DEBUG,"P_GetNodesVersion: using normal BSP nodes\n");
    if (P_CheckForZDoomNodes(lumpnum, gl_lumpnum))
      I_Error("P_GetNodesVersion: ZDoom nodes not supported yet");
  }
}

//
// P_LoadVertexes
//
// killough 5/3/98: reformatted, cleaned up
//
static void P_LoadVertexes (int lump)
{
  const mapvertex_t *data; // cph - const
  int i;

  // Determine number of lumps:
  //  total lump length / vertex record length.
  numvertexes = W_LumpLength(lump) / sizeof(mapvertex_t);

  // Allocate zone memory for buffer.
  vertexes = Z_Malloc(numvertexes*sizeof(vertex_t),PU_LEVEL,0);

  // Load data into cache.
  // cph 2006/07/29 - cast to mapvertex_t here, making the loop below much neater
  data = (const mapvertex_t *)W_CacheLumpNum(lump);

  // Copy and convert vertex coordinates,
  // internal representation as fixed.
  for (i=0; i<numvertexes; i++)
    {
      vertexes[i].x = SHORT(data[i].x)<<FRACBITS;
      vertexes[i].y = SHORT(data[i].y)<<FRACBITS;
    }

  // Free buffer memory.
  W_UnlockLumpNum(lump);
}

/*******************************************
 * Name     : P_LoadVertexes2        *
 * modified : 09/18/00, adapted for PrBoom *
 * author   : figgi              *
 * what   : support for gl nodes       *
 *******************************************/

// figgi -- FIXME: Automap showes wrong zoom boundaries when starting game
//           when P_LoadVertexes2 is used with classic BSP nodes.

static void P_LoadVertexes2(int lump, int gllump)
{
  const byte         *gldata;
  int                 i;
  const mapvertex_t*  ml;

  firstglvertex = W_LumpLength(lump) / sizeof(mapvertex_t);
  numvertexes   = W_LumpLength(lump) / sizeof(mapvertex_t);

  if (gllump >= 0)  // check for glVertices
  {
    gldata = W_CacheLumpNum(gllump);

    if (nodesVersion == gNd2) // 32 bit GL_VERT format (16.16 fixed)
    {
      const mapglvertex_t*  mgl;

      numvertexes += (W_LumpLength(gllump) - GL_VERT_OFFSET)/sizeof(mapglvertex_t);
      vertexes   = Z_Malloc (numvertexes*sizeof(vertex_t),PU_LEVEL,0);
      mgl      = (const mapglvertex_t *) (gldata + GL_VERT_OFFSET);

      for (i = firstglvertex; i < numvertexes; i++)
      {
        vertexes[i].x = mgl->x;
        vertexes[i].y = mgl->y;
        mgl++;
      }
    }
    else
    {
      numvertexes += W_LumpLength(gllump)/sizeof(mapvertex_t);
      vertexes     = Z_Malloc (numvertexes*sizeof(vertex_t),PU_LEVEL,0);
      ml       = (const mapvertex_t *)gldata;

      for (i = firstglvertex; i < numvertexes; i++)
      {
        vertexes[i].x = SHORT(ml->x)<<FRACBITS;
        vertexes[i].y = SHORT(ml->y)<<FRACBITS;
        ml++;
      }
    }
    W_UnlockLumpNum(gllump);
  }

  ml = (const mapvertex_t*) W_CacheLumpNum(lump);

  for (i=0; i < firstglvertex; i++)
  {
    vertexes[i].x = SHORT(ml->x)<<FRACBITS;
    vertexes[i].y = SHORT(ml->y)<<FRACBITS;
    ml++;
  }
  W_UnlockLumpNum(lump);
}


/*******************************************
 * created  : 08/13/00             *
 * modified : 09/18/00, adapted for PrBoom *
 * author   : figgi              *
 * what   : basic functions needed for   *
 *            computing  gl nodes      *
 *******************************************/

static int checkGLVertex(int num)
{
  if (num & 0x8000)
    num = (num&0x7FFF)+firstglvertex;
  return num;
}


// static float GetDistance(int dx, int dy)
// {
//   float fx = (float)(dx)/FRACUNIT, fy = (float)(dy)/FRACUNIT;
//   return (float)sqrt(fx*fx + fy*fy);
// }


static int GetOffset(vertex_t *v1, vertex_t *v2)
{
  float a, b;
  int r;
  a = (float)(v1->x - v2->x) / (float)FRACUNIT;
  b = (float)(v1->y - v2->y) / (float)FRACUNIT;
  r = (int)(sqrt(a*a+b*b) * (float)FRACUNIT);
  return r;
}



//
// P_LoadSegs
//
// killough 5/3/98: reformatted, cleaned up

static void P_LoadSegs (int lump)
{
  int  i;
  const mapseg_t *data; // cph - const

  numsegs = W_LumpLength(lump) / sizeof(mapseg_t);
  segs = Z_Calloc(numsegs,sizeof(seg_t),PU_LEVEL,0);
  data = (const mapseg_t *)W_CacheLumpNum(lump); // cph - wad lump handling updated

  if ((!data) || (!numsegs))
    I_Error("P_LoadSegs: no segs in level");

  for (i=0; i<numsegs; i++)
    {
      seg_t *li = segs+i;
      const mapseg_t *ml = data + i;
      unsigned short v1, v2;

      int side, linedef;
      line_t *ldef;

      v1 = (unsigned short)SHORT(ml->v1);
      v2 = (unsigned short)SHORT(ml->v2);
      li->v1 = &vertexes[v1];
      li->v2 = &vertexes[v2];

      li->miniseg = false; // figgi -- there are no minisegs in classic BSP nodes
      li->angle = (SHORT(ml->angle))<<16;
      li->offset =(SHORT(ml->offset))<<16;
      linedef = (unsigned short)SHORT(ml->linedef);
      ldef = &lines[linedef];
      li->linedef = ldef;
      side = SHORT(ml->side);
      li->sidedef = &sides[ldef->sidenum[side]];

      /* cph 2006/09/30 - our frontsector can be the second side of the
       * linedef, so must check for NO_INDEX in case we are incorrectly
       * referencing the back of a 1S line */
      if (ldef->sidenum[side] != NO_INDEX)
        li->frontsector = sides[ldef->sidenum[side]].sector;
      else {
        li->frontsector = 0;
        lprintf(LO_WARN, "P_LoadSegs: front of seg %i has no sidedef\n", i);
      }

      if (ldef->flags & ML_TWOSIDED && ldef->sidenum[side^1]!=NO_INDEX)
        li->backsector = sides[ldef->sidenum[side^1]].sector;
      else
        li->backsector = 0;
    }

  W_UnlockLumpNum(lump); // cph - release the data
}



/*******************************************
 * Name     : P_LoadGLSegs           *
 * created  : 08/13/00             *
 * modified : 09/18/00, adapted for PrBoom *
 * author   : figgi              *
 * what   : support for gl nodes       *
 *******************************************/
static void P_LoadGLSegs(int lump)
{
  int     i;
  const glseg_t   *ml;
  line_t    *ldef;

  numsegs = W_LumpLength(lump) / sizeof(glseg_t);
  segs = Z_Malloc(numsegs * sizeof(seg_t), PU_LEVEL, 0);
  memset(segs, 0, numsegs * sizeof(seg_t));
  ml = (const glseg_t*)W_CacheLumpNum(lump);

  if ((!ml) || (!numsegs))
    I_Error("P_LoadGLSegs: no glsegs in level");

  for(i = 0; i < numsegs; i++)
  {             // check for gl-vertices
    segs[i].v1 = &vertexes[checkGLVertex(SHORT(ml->v1))];
    segs[i].v2 = &vertexes[checkGLVertex(SHORT(ml->v2))];

    if(ml->linedef != (unsigned short)-1) // skip minisegs
    {
      ldef = &lines[ml->linedef];
      segs[i].linedef = ldef;
      segs[i].miniseg = false;
      segs[i].angle = R_PointToAngle2(segs[i].v1->x,segs[i].v1->y,segs[i].v2->x,segs[i].v2->y);

      segs[i].sidedef = &sides[ldef->sidenum[ml->side]];
      segs[i].frontsector = sides[ldef->sidenum[ml->side]].sector;
      if (ldef->flags & ML_TWOSIDED)
        segs[i].backsector = sides[ldef->sidenum[ml->side^1]].sector;
      else
        segs[i].backsector = 0;

      if (ml->side)
        segs[i].offset = GetOffset(segs[i].v1, ldef->v2);
      else
        segs[i].offset = GetOffset(segs[i].v1, ldef->v1);
    }
    else
    {
      segs[i].miniseg = true;
      segs[i].angle  = 0;
      segs[i].offset  = 0;
      segs[i].linedef = NULL;
      segs[i].sidedef = NULL;
      segs[i].frontsector = NULL;
      segs[i].backsector  = NULL;
    }
    ml++;
  }
  W_UnlockLumpNum(lump);
}

//
// P_LoadSubsectors
//
// killough 5/3/98: reformatted, cleaned up

static void P_LoadSubsectors (int lump)
{
  /* cph 2006/07/29 - make data a const mapsubsector_t *, so the loop below is simpler & gives no constness warnings */
  const mapsubsector_t *data;
  int  i;

  numsubsectors = W_LumpLength (lump) / sizeof(mapsubsector_t);
  subsectors = Z_Calloc(numsubsectors,sizeof(subsector_t),PU_LEVEL,0);
  data = (const mapsubsector_t *)W_CacheLumpNum(lump);

  if ((!data) || (!numsubsectors))
    I_Error("P_LoadSubsectors: no subsectors in level");

  for (i=0; i<numsubsectors; i++)
  {
    subsectors[i].numlines  = (unsigned short)SHORT(data[i].numsegs );
    subsectors[i].firstline = (unsigned short)SHORT(data[i].firstseg);
  }

  W_UnlockLumpNum(lump); // cph - release the data
}

//
// P_LoadSectors
//
// killough 5/3/98: reformatted, cleaned up

static void P_LoadSectors (int lump)
{
  const byte *data; // cph - const*
  int  i;

  numsectors = W_LumpLength (lump) / sizeof(mapsector_t);
  sectors = Z_Calloc (numsectors,sizeof(sector_t),PU_LEVEL,0);
  data = W_CacheLumpNum (lump); // cph - wad lump handling updated

  for (i=0; i<numsectors; i++)
    {
      sector_t *ss = sectors + i;
      const mapsector_t *ms = (const mapsector_t *) data + i;

      ss->floorheight = SHORT(ms->floorheight)<<FRACBITS;
      ss->ceilingheight = SHORT(ms->ceilingheight)<<FRACBITS;
      ss->floorpic = R_FlatNumForName(ms->floorpic);
      ss->ceilingpic = R_FlatNumForName(ms->ceilingpic);
      ss->lightlevel = SHORT(ms->lightlevel);
      ss->special = SHORT(ms->special);
      ss->oldspecial = SHORT(ms->special);
      ss->tag = SHORT(ms->tag);
      ss->thinglist = NULL;
      ss->touching_thinglist = NULL;            // phares 3/14/98

      ss->nextsec = -1; //jff 2/26/98 add fields to support locking out
      ss->prevsec = -1; // stair retriggering until build completes

      // killough 3/7/98:
      ss->floor_xoffs = 0;
      ss->floor_yoffs = 0;      // floor and ceiling flats offsets
      ss->ceiling_xoffs = 0;
      ss->ceiling_yoffs = 0;
      ss->heightsec = -1;       // sector used to get floor and ceiling height
      ss->floorlightsec = -1;   // sector used to get floor lighting
      // killough 3/7/98: end changes

      // killough 4/11/98 sector used to get ceiling lighting:
      ss->ceilinglightsec = -1;

      // killough 4/4/98: colormaps:
      ss->bottommap = ss->midmap = ss->topmap = 0;

      // killough 10/98: sky textures coming from sidedefs:
      ss->sky = 0;
    }

  W_UnlockLumpNum(lump); // cph - release the data
}


//
// P_LoadNodes
//
// killough 5/3/98: reformatted, cleaned up

static void P_LoadNodes (int lump)
{
  const byte *data; // cph - const*
  int  i;

  numnodes = W_LumpLength (lump) / sizeof(mapnode_t);
  nodes = Z_Malloc (numnodes*sizeof(node_t),PU_LEVEL,0);
  data = W_CacheLumpNum (lump); // cph - wad lump handling updated

  if ((!data) || (!numnodes))
  {
    // allow trivial maps
    if (numsubsectors == 1)
      lprintf(LO_INFO,
          "P_LoadNodes: trivial map (no nodes, one subsector)\n");
    else
      I_Error("P_LoadNodes: no nodes in level");
  }

  for (i=0; i<numnodes; i++)
    {
      node_t *no = nodes + i;
      const mapnode_t *mn = (const mapnode_t *) data + i;
      int j;

      no->x = SHORT(mn->x)<<FRACBITS;
      no->y = SHORT(mn->y)<<FRACBITS;
      no->dx = SHORT(mn->dx)<<FRACBITS;
      no->dy = SHORT(mn->dy)<<FRACBITS;

      for (j=0 ; j<2 ; j++)
        {
          int k;
          no->children[j] = SHORT(mn->children[j]);
          for (k=0 ; k<4 ; k++)
            no->bbox[j][k] = SHORT(mn->bbox[j][k])<<FRACBITS;
        }
    }

  W_UnlockLumpNum(lump); // cph - release the data
}


/*
 * P_LoadThings
 *
 * killough 5/3/98: reformatted, cleaned up
 * cph 2001/07/07 - don't write into the lump cache, especially non-idepotent
 * changes like byte order reversals. Take a copy to edit.
 */

static void P_LoadThings (int lump)
{
  int  i, numthings = W_LumpLength (lump) / sizeof(mapthing_t);
  const mapthing_t *data = W_CacheLumpNum (lump);

  if ((!data) || (!numthings))
    I_Error("P_LoadThings: no things in level");

  for (i=0; i<numthings; i++)
    {
      mapthing_t mt = data[i];

      mt.x = SHORT(mt.x);
      mt.y = SHORT(mt.y);
      mt.angle = SHORT(mt.angle);
      mt.type = SHORT(mt.type);
      mt.options = SHORT(mt.options);

      if (!P_IsDoomnumAllowed(mt.type))
        continue;

      // Do spawn all other stuff.
      P_SpawnMapThing(&mt);
    }

  W_UnlockLumpNum(lump); // cph - release the data
}

//
// P_LoadLineDefs
// Also counts secret lines for intermissions.
//        ^^^
// ??? killough ???
// Does this mean secrets used to be linedef-based, rather than sector-based?
//
// killough 4/4/98: split into two functions, to allow sidedef overloading
//
// killough 5/3/98: reformatted, cleaned up

static void P_LoadLineDefs (int lump)
{
  const byte *data; // cph - const*
  int  i;

  numlines = W_LumpLength (lump) / sizeof(maplinedef_t);
  lines = Z_Calloc (numlines,sizeof(line_t),PU_LEVEL,0);
  data = W_CacheLumpNum (lump); // cph - wad lump handling updated

  for (i=0; i<numlines; i++)
    {
      const maplinedef_t *mld = (const maplinedef_t *) data + i;
      line_t *ld = lines+i;
      vertex_t *v1, *v2;

      ld->flags = (unsigned short)SHORT(mld->flags);
      ld->special = SHORT(mld->special);
      ld->tag = SHORT(mld->tag);
      v1 = ld->v1 = &vertexes[(unsigned short)SHORT(mld->v1)];
      v2 = ld->v2 = &vertexes[(unsigned short)SHORT(mld->v2)];
      ld->dx = v2->x - v1->x;
      ld->dy = v2->y - v1->y;

      ld->tranlump = -1;   // killough 4/11/98: no translucency by default

      ld->slopetype = !ld->dx ? ST_VERTICAL : !ld->dy ? ST_HORIZONTAL :
        FixedDiv(ld->dy, ld->dx) > 0 ? ST_POSITIVE : ST_NEGATIVE;

      if (v1->x < v2->x)
        {
          ld->bbox[BOXLEFT] = v1->x;
          ld->bbox[BOXRIGHT] = v2->x;
        }
      else
        {
          ld->bbox[BOXLEFT] = v2->x;
          ld->bbox[BOXRIGHT] = v1->x;
        }
      if (v1->y < v2->y)
        {
          ld->bbox[BOXBOTTOM] = v1->y;
          ld->bbox[BOXTOP] = v2->y;
        }
      else
        {
          ld->bbox[BOXBOTTOM] = v2->y;
          ld->bbox[BOXTOP] = v1->y;
        }

      /* calculate sound origin of line to be its midpoint */
      //e6y: fix sound origin for large levels
      // no need for comp_sound test, these are only used when comp_sound = 0
      ld->soundorg.x = ld->bbox[BOXLEFT] / 2 + ld->bbox[BOXRIGHT] / 2;
      ld->soundorg.y = ld->bbox[BOXTOP] / 2 + ld->bbox[BOXBOTTOM] / 2;

      ld->sidenum[0] = SHORT(mld->sidenum[0]);
      ld->sidenum[1] = SHORT(mld->sidenum[1]);

      {
        /* cph 2006/09/30 - fix sidedef errors right away.
         * cph 2002/07/20 - these errors are fatal if not fixed, so apply them
         * in compatibility mode - a desync is better than a crash! */
        int j;

        for (j=0; j < 2; j++)
        {
          if (ld->sidenum[j] != NO_INDEX && ld->sidenum[j] >= numsides) {
            ld->sidenum[j] = NO_INDEX;
            lprintf(LO_WARN, "P_LoadLineDefs: linedef %d has out-of-range sidedef number\n",numlines-i-1);
          }
        }

        // killough 11/98: fix common wad errors (missing sidedefs):

        if (ld->sidenum[0] == NO_INDEX) {
          ld->sidenum[0] = 0;  // Substitute dummy sidedef for missing right side
          // cph - print a warning about the bug
          lprintf(LO_WARN, "P_LoadLineDefs: linedef %d missing first sidedef\n",numlines-i-1);
        }

        if ((ld->sidenum[1] == NO_INDEX) && (ld->flags & ML_TWOSIDED)) {
          ld->flags &= ~ML_TWOSIDED;  // Clear 2s flag for missing left side
          // cph - print a warning about the bug
          lprintf(LO_WARN, "P_LoadLineDefs: linedef %d has two-sided flag set, but no second sidedef\n",numlines-i-1);
        }
      }

      // killough 4/4/98: support special sidedef interpretation below
      if (ld->sidenum[0] != NO_INDEX && ld->special)
        sides[*ld->sidenum].special = ld->special;
    }

  W_UnlockLumpNum(lump); // cph - release the lump
}

// killough 4/4/98: delay using sidedefs until they are loaded
// killough 5/3/98: reformatted, cleaned up

static void P_LoadLineDefs2(int lump)
{
  int i = numlines;
  register line_t *ld = lines;
  for (;i--;ld++)
    {
      ld->frontsector = sides[ld->sidenum[0]].sector; //e6y: Can't be NO_INDEX here
      ld->backsector  = ld->sidenum[1]!=NO_INDEX ? sides[ld->sidenum[1]].sector : 0;
      switch (ld->special)
        {                       // killough 4/11/98: handle special types
          int lump, j;

        case 260:               // killough 4/11/98: translucent 2s textures
            lump = sides[*ld->sidenum].special; // translucency from sidedef
            if (!ld->tag)                       // if tag==0,
              ld->tranlump = lump;              // affect this linedef only
            else
              for (j=0;j<numlines;j++)          // if tag!=0,
                if (lines[j].tag == ld->tag)    // affect all matching linedefs
                  lines[j].tranlump = lump;
            break;
        }
    }
}

//
// P_LoadSideDefs
//
// killough 4/4/98: split into two functions

static void P_LoadSideDefs (int lump)
{
  numsides = W_LumpLength(lump) / sizeof(mapsidedef_t);
  sides = Z_Calloc(numsides,sizeof(side_t),PU_LEVEL,0);
}

// killough 4/4/98: delay using texture names until
// after linedefs are loaded, to allow overloading.
// killough 5/3/98: reformatted, cleaned up

static void P_LoadSideDefs2(int lump)
{
  const byte *data = W_CacheLumpNum(lump); // cph - const*, wad lump handling updated
  int  i;

  for (i=0; i<numsides; i++)
    {
      register const mapsidedef_t *msd = (const mapsidedef_t *) data + i;
      register side_t *sd = sides + i;
      register sector_t *sec;

      sd->textureoffset = SHORT(msd->textureoffset)<<FRACBITS;
      sd->rowoffset = SHORT(msd->rowoffset)<<FRACBITS;

      { /* cph 2006/09/30 - catch out-of-range sector numbers; use sector 0 instead */
        unsigned short sector_num = SHORT(msd->sector);
        if (sector_num >= numsectors) {
          lprintf(LO_WARN,"P_LoadSideDefs2: sidedef %i has out-of-range sector num %u\n", i, sector_num);
          sector_num = 0;
        }
        sd->sector = sec = &sectors[sector_num];
      }

      // killough 4/4/98: allow sidedef texture names to be overloaded
      // killough 4/11/98: refined to allow colormaps to work as wall
      // textures if invalid as colormaps but valid as textures.
      switch (sd->special)
        {
        case 242:                       // variable colormap via 242 linedef
          sd->bottomtexture =
            (sec->bottommap =   R_ColormapNumForName(msd->bottomtexture)) < 0 ?
            sec->bottommap = 0, R_TextureNumForName(msd->bottomtexture): 0 ;
          sd->midtexture =
            (sec->midmap =   R_ColormapNumForName(msd->midtexture)) < 0 ?
            sec->midmap = 0, R_TextureNumForName(msd->midtexture)  : 0 ;
          sd->toptexture =
            (sec->topmap =   R_ColormapNumForName(msd->toptexture)) < 0 ?
            sec->topmap = 0, R_TextureNumForName(msd->toptexture)  : 0 ;
          break;

        case 260: // killough 4/11/98: apply translucency to 2s normal texture
          sd->midtexture = strncasecmp("TRANMAP", msd->midtexture, 8) ?
            (sd->special = W_CheckNumForName(msd->midtexture)) < 0 ||
            W_LumpLength(sd->special) != 65536 ?
            sd->special=0, R_TextureNumForName(msd->midtexture) :
              (sd->special++, 0) : (sd->special=0);
          sd->toptexture = R_TextureNumForName(msd->toptexture);
          sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
          break;

        default:                        // normal cases
          sd->midtexture = R_SafeTextureNumForName(msd->midtexture, i);
          sd->toptexture = R_SafeTextureNumForName(msd->toptexture, i);
          sd->bottomtexture = R_SafeTextureNumForName(msd->bottomtexture, i);
          break;
        }
    }

  W_UnlockLumpNum(lump); // cph - release the lump
}

//
// jff 10/6/98
// New code added to speed up calculation of internal blockmap
// Algorithm is order of nlines*(ncols+nrows) not nlines*ncols*nrows
//

#define blkshift 7               /* places to shift rel position for cell num */
#define blkmask ((1<<blkshift)-1)/* mask for rel position within cell */
#define blkmargin 0              /* size guardband around map used */
                                 // jff 10/8/98 use guardband>0
                                 // jff 10/12/98 0 ok with + 1 in rows,cols

typedef struct linelist_t        // type used to list lines in each block
{
  long num;
  struct linelist_t *next;
} linelist_t;

//
// Subroutine to add a line number to a block list
// It simply returns if the line is already in the block
//

static void AddBlockLine(linelist_t **lists, int *count, int *done, int blockno, long lineno)
{
  if (done[blockno])
    return;

  linelist_t *l = Z_Malloc(sizeof(linelist_t), PU_STATIC, 0);
  l->num = lineno;
  l->next = lists[blockno];
  lists[blockno] = l;
  count[blockno]++;
  done[blockno] = 1;
}

//
// Actually construct the blockmap lump from the level data
//
// This finds the intersection of each linedef with the column and
// row lines at the left and bottom of each blockmap cell. It then
// adds the line to all block lists touching the intersection.
//

static void P_CreateBlockMap(void)
{
  int xorg,yorg;                 // blockmap origin (lower left)
  int nrows,ncols;               // blockmap dimensions
  linelist_t **blocklists=NULL;  // array of pointers to lists of lines
  int *blockcount=NULL;          // array of counters of line lists
  int *blockdone=NULL;           // array keeping track of blocks/line
  int NBlocks;                   // number of cells = nrows*ncols
  long linetotal=0;              // total length of all blocklists
  int i,j;
  int map_minx=INT_MAX;          // init for map limits search
  int map_miny=INT_MAX;
  int map_maxx=INT_MIN;
  int map_maxy=INT_MIN;

  // scan for map limits, which the blockmap must enclose

  for (i=0;i<numvertexes;i++)
  {
    fixed_t t;

    if ((t=vertexes[i].x) < map_minx)
      map_minx = t;
    else if (t > map_maxx)
      map_maxx = t;
    if ((t=vertexes[i].y) < map_miny)
      map_miny = t;
    else if (t > map_maxy)
      map_maxy = t;
  }
  map_minx >>= FRACBITS;    // work in map coords, not fixed_t
  map_maxx >>= FRACBITS;
  map_miny >>= FRACBITS;
  map_maxy >>= FRACBITS;

  // set up blockmap area to enclose level plus margin

  xorg = map_minx-blkmargin;
  yorg = map_miny-blkmargin;
  ncols = (map_maxx+blkmargin-xorg+1+blkmask)>>blkshift;  //jff 10/12/98
  nrows = (map_maxy+blkmargin-yorg+1+blkmask)>>blkshift;  //+1 needed for
  NBlocks = ncols*nrows;                                  //map exactly 1 cell

  // create the array of pointers on NBlocks to blocklists
  // also create an array of linelist counts on NBlocks
  // finally make an array in which we can mark blocks done per line
  blocklists = Z_Calloc(NBlocks, sizeof(linelist_t *), PU_STATIC, 0);
  blockcount = Z_Calloc(NBlocks, sizeof(int), PU_STATIC, 0);
  blockdone = Z_Calloc(NBlocks, sizeof(int), PU_STATIC, 0);

  // initialize each blocklist, and enter the trailing -1 in all blocklists
  // note the linked list of lines grows backwards

  for (i=0;i<NBlocks;i++)
  {
    blocklists[i] = Z_Malloc(sizeof(linelist_t), PU_STATIC, 0);
    blocklists[i]->num = -1;
    blocklists[i]->next = NULL;
    blockcount[i]++;
  }

  // For each linedef in the wad, determine all blockmap blocks it touches,
  // and add the linedef number to the blocklists for those blocks

  for (i=0;i<numlines;i++)
  {
    int x1 = lines[i].v1->x>>FRACBITS;         // lines[i] map coords
    int y1 = lines[i].v1->y>>FRACBITS;
    int x2 = lines[i].v2->x>>FRACBITS;
    int y2 = lines[i].v2->y>>FRACBITS;
    int dx = x2-x1;
    int dy = y2-y1;
    int vert = !dx;                            // lines[i] slopetype
    int horiz = !dy;
    int spos = (dx^dy) > 0;
    int sneg = (dx^dy) < 0;
    int bx,by;                                 // block cell coords
    int minx = x1>x2? x2 : x1;                 // extremal lines[i] coords
    int maxx = x1>x2? x1 : x2;
    int miny = y1>y2? y2 : y1;
    int maxy = y1>y2? y1 : y2;

    // no blocks done for this linedef yet

    memset(blockdone,0,NBlocks*sizeof(int));

    // The line always belongs to the blocks containing its endpoints

    bx = (x1-xorg)>>blkshift;
    by = (y1-yorg)>>blkshift;
    AddBlockLine(blocklists,blockcount,blockdone,by*ncols+bx,i);
    bx = (x2-xorg)>>blkshift;
    by = (y2-yorg)>>blkshift;
    AddBlockLine(blocklists,blockcount,blockdone,by*ncols+bx,i);


    // For each column, see where the line along its left edge, which
    // it contains, intersects the Linedef i. Add i to each corresponding
    // blocklist.

    if (!vert)    // don't interesect vertical lines with columns
    {
      for (j=0;j<ncols;j++)
      {
        // intersection of Linedef with x=xorg+(j<<blkshift)
        // (y-y1)*dx = dy*(x-x1)
        // y = dy*(x-x1)+y1*dx;

        int x = xorg+(j<<blkshift);       // (x,y) is intersection
        int y = (dy*(x-x1))/dx+y1;
        int yb = (y-yorg)>>blkshift;      // block row number
        int yp = (y-yorg)&blkmask;        // y position within block

        if (yb<0 || yb>nrows-1)     // outside blockmap, continue
          continue;

        if (x<minx || x>maxx)       // line doesn't touch column
          continue;

        // The cell that contains the intersection point is always added

        AddBlockLine(blocklists,blockcount,blockdone,ncols*yb+j,i);

        // if the intersection is at a corner it depends on the slope
        // (and whether the line extends past the intersection) which
        // blocks are hit

        if (yp==0)        // intersection at a corner
        {
          if (sneg)       //   \ - blocks x,y-, x-,y
          {
            if (yb>0 && miny<y)
              AddBlockLine(blocklists,blockcount,blockdone,ncols*(yb-1)+j,i);
            if (j>0 && minx<x)
              AddBlockLine(blocklists,blockcount,blockdone,ncols*yb+j-1,i);
          }
          else if (spos)  //   / - block x-,y-
          {
            if (yb>0 && j>0 && minx<x)
              AddBlockLine(blocklists,blockcount,blockdone,ncols*(yb-1)+j-1,i);
          }
          else if (horiz) //   - - block x-,y
          {
            if (j>0 && minx<x)
              AddBlockLine(blocklists,blockcount,blockdone,ncols*yb+j-1,i);
          }
        }
        else if (j>0 && minx<x) // else not at corner: x-,y
          AddBlockLine(blocklists,blockcount,blockdone,ncols*yb+j-1,i);
      }
    }

    // For each row, see where the line along its bottom edge, which
    // it contains, intersects the Linedef i. Add i to all the corresponding
    // blocklists.

    if (!horiz)
    {
      for (j=0;j<nrows;j++)
      {
        // intersection of Linedef with y=yorg+(j<<blkshift)
        // (x,y) on Linedef i satisfies: (y-y1)*dx = dy*(x-x1)
        // x = dx*(y-y1)/dy+x1;

        int y = yorg+(j<<blkshift);       // (x,y) is intersection
        int x = (dx*(y-y1))/dy+x1;
        int xb = (x-xorg)>>blkshift;      // block column number
        int xp = (x-xorg)&blkmask;        // x position within block

        if (xb<0 || xb>ncols-1)   // outside blockmap, continue
          continue;

        if (y<miny || y>maxy)     // line doesn't touch row
          continue;

        // The cell that contains the intersection point is always added

        AddBlockLine(blocklists,blockcount,blockdone,ncols*j+xb,i);

        // if the intersection is at a corner it depends on the slope
        // (and whether the line extends past the intersection) which
        // blocks are hit

        if (xp==0)        // intersection at a corner
        {
          if (sneg)       //   \ - blocks x,y-, x-,y
          {
            if (j>0 && miny<y)
              AddBlockLine(blocklists,blockcount,blockdone,ncols*(j-1)+xb,i);
            if (xb>0 && minx<x)
              AddBlockLine(blocklists,blockcount,blockdone,ncols*j+xb-1,i);
          }
          else if (vert)  //   | - block x,y-
          {
            if (j>0 && miny<y)
              AddBlockLine(blocklists,blockcount,blockdone,ncols*(j-1)+xb,i);
          }
          else if (spos)  //   / - block x-,y-
          {
            if (xb>0 && j>0 && miny<y)
              AddBlockLine(blocklists,blockcount,blockdone,ncols*(j-1)+xb-1,i);
          }
        }
        else if (j>0 && miny<y) // else not on a corner: x,y-
          AddBlockLine(blocklists,blockcount,blockdone,ncols*(j-1)+xb,i);
      }
    }
  }

  // Add initial 0 to all blocklists
  // count the total number of lines (and 0's and -1's)

  memset(blockdone,0,NBlocks*sizeof(int));
  for (i=0,linetotal=0;i<NBlocks;i++)
  {
    AddBlockLine(blocklists,blockcount,blockdone,i,0);
    linetotal += blockcount[i];
  }

  // Create the blockmap lump

  blockmaplump = Z_Malloc(sizeof(*blockmaplump) * (4+NBlocks+linetotal),
                          PU_LEVEL, 0);
  // blockmap header

  blockmaplump[0] = bmaporgx = xorg << FRACBITS;
  blockmaplump[1] = bmaporgy = yorg << FRACBITS;
  blockmaplump[2] = bmapwidth  = ncols;
  blockmaplump[3] = bmapheight = nrows;

  // offsets to lists and block lists

  for (i=0;i<NBlocks;i++)
  {
    linelist_t *bl = blocklists[i];
    long offs = blockmaplump[4+i] =   // set offset to block's list
      (i? blockmaplump[4+i-1] : 4+NBlocks) + (i? blockcount[i-1] : 0);

    // add the lines in each block's list to the blockmaplump
    // delete each list node as we go

    while (bl)
    {
      linelist_t *tmp = bl->next;
      blockmaplump[offs++] = bl->num;
      free(bl);
      bl = tmp;
    }
  }

  // free all temporary storage

  free (blocklists);
  free (blockcount);
  free (blockdone);
}

// jff 10/6/98
// End new code added to speed up calculation of internal blockmap

//
// P_LoadBlockMap
//
// killough 3/1/98: substantially modified to work
// towards removing blockmap limit (a wad limitation)
//
// killough 3/30/98: Rewritten to remove blockmap limit,
// though current algorithm is brute-force and unoptimal.
//

static void P_LoadBlockMap (int lump)
{
  long count;

  if (M_CheckParm("-blockmap") || W_LumpLength(lump)<8 || (count = W_LumpLength(lump)/2) >= 0x10000) //e6y
    P_CreateBlockMap();
  else
    {
      long i;
      // cph - const*, wad lump handling updated
      const short *wadblockmaplump = W_CacheLumpNum(lump);
      blockmaplump = Z_Malloc(sizeof(*blockmaplump) * count, PU_LEVEL, 0);

      // killough 3/1/98: Expand wad blockmap into larger internal one,
      // by treating all offsets except -1 as unsigned and zero-extending
      // them. This potentially doubles the size of blockmaps allowed,
      // because Doom originally considered the offsets as always signed.

      blockmaplump[0] = SHORT(wadblockmaplump[0]);
      blockmaplump[1] = SHORT(wadblockmaplump[1]);
      blockmaplump[2] = (long)(SHORT(wadblockmaplump[2])) & 0xffff;
      blockmaplump[3] = (long)(SHORT(wadblockmaplump[3])) & 0xffff;

      for (i=4 ; i<count ; i++)
        {
          short t = SHORT(wadblockmaplump[i]);          // killough 3/1/98
          blockmaplump[i] = t == -1 ? -1l : (long) t & 0xffff;
        }

      W_UnlockLumpNum(lump); // cph - unlock the lump

      bmaporgx = blockmaplump[0]<<FRACBITS;
      bmaporgy = blockmaplump[1]<<FRACBITS;
      bmapwidth = blockmaplump[2];
      bmapheight = blockmaplump[3];
    }

  blocklinks = Z_Calloc(bmapwidth*bmapheight, sizeof(*blocklinks), PU_LEVEL, 0);
  blockmap = blockmaplump+4;
}

//
// P_LoadReject - load the reject table, padding it if it is too short
// totallines must be the number returned by P_GroupLines()
// an underflow will be padded with zeroes, or a doom.exe z_zone header
//
// this function incorporates e6y's RejectOverrunAddInt code:
// e6y: REJECT overrun emulation code
// It's emulated successfully if the size of overflow no more than 16 bytes.
// No more desync on teeth-32.wad\teeth-32.lmp.
// http://www.doomworld.com/vb/showthread.php?s=&threadid=35214

static void P_LoadReject(int lumpnum, int totallines)
{
  unsigned int length, required;
  byte *newreject;

  // dump any old cached reject lump, then cache the new one
  if (rejectlump != -1)
    W_UnlockLumpNum(rejectlump);
  rejectlump = lumpnum + ML_REJECT;
  rejectmatrix = W_CacheLumpNum(rejectlump);

  required = (numsectors * numsectors + 7) / 8;
  length = W_LumpLength(rejectlump);

  if (length >= required)
    return; // nothing to do

  // allocate a new block and copy the reject table into it; zero the rest
  // PU_LEVEL => will be freed on level exit
  newreject = Z_Malloc(required, PU_LEVEL, NULL);
  rejectmatrix = (const byte *)memmove(newreject, rejectmatrix, length);
  memset(newreject + length, 0, required - length);
  // unlock the original lump, it is no longer needed
  W_UnlockLumpNum(rejectlump);
  rejectlump = -1;

  if (demo_compatibility)
  {
    // merged in RejectOverrunAddInt(), and the 4 calls to it, here
    unsigned int rejectpad[4] = {
      0,        // size, will be filled in using totallines
      0,        // part of the header of a doom.exe z_zone block
      50,       // DOOM_CONST_PU_LEVEL
      0x1d4a11  // DOOM_CONST_ZONEID
    };
    unsigned int i, pad = 0, *src = rejectpad;
    byte *dest = newreject + length;

    rejectpad[0] = ((totallines*4+3)&~3)+24; // doom.exe zone header size

    // copy at most 16 bytes from rejectpad
    // emulating a 32-bit, little-endian architecture (can't memmove)
    for (i = 0; i < required - length && i < 16; i++) { // 16 hard-coded
      if (!(i&3)) // get the next 4 bytes to copy when i=0,4,8,12
        pad = *src++;
      *dest++ = pad & 0xff; // store lowest-significant byte
      pad >>= 8; // rotate the next byte down
    }
  }
  lprintf(LO_WARN, "P_LoadReject: REJECT too short (%u<%u) - padded\n",
          length, required);
}

//
// P_GroupLines
// Builds sector line lists and subsector sector numbers.
// Finds block bounding boxes for sectors.
//
// killough 5/3/98: reformatted, cleaned up
// cph 18/8/99: rewritten to avoid O(numlines * numsectors) section
// It makes things more complicated, but saves seconds on big levels
// figgi 09/18/00 -- adapted for gl-nodes

// cph - convenient sub-function
static void P_AddLineToSector(line_t* li, sector_t* sector)
{
  fixed_t *bbox = (void*)sector->blockbox;

  sector->lines[sector->linecount++] = li;
  M_AddToBox (bbox, li->v1->x, li->v1->y);
  M_AddToBox (bbox, li->v2->x, li->v2->y);
}

// modified to return totallines (needed by P_LoadReject)
static int P_GroupLines (void)
{
  register line_t *li;
  register sector_t *sector;
  int i,j, total = numlines;

  // figgi
  for (i=0 ; i<numsubsectors ; i++)
  {
    seg_t *seg = &segs[subsectors[i].firstline];
    subsectors[i].sector = NULL;
    for(j=0; j<subsectors[i].numlines; j++)
    {
      if(seg->sidedef)
      {
        subsectors[i].sector = seg->sidedef->sector;
        break;
      }
      seg++;
    }
    if(subsectors[i].sector == NULL)
      I_Error("P_GroupLines: Subsector a part of no sector!\n");
  }

  // count number of lines in each sector
  for (i=0,li=lines; i<numlines; i++, li++)
    {
      li->frontsector->linecount++;
      if (li->backsector && li->backsector != li->frontsector)
        {
          li->backsector->linecount++;
          total++;
        }
    }

  {  // allocate line tables for each sector
    line_t **linebuffer = Z_Malloc(total*sizeof(line_t *), PU_LEVEL, 0);

    // e6y: REJECT overrun emulation code
    // moved to P_LoadReject

    for (i=0, sector = sectors; i<numsectors; i++, sector++)
    {
      sector->lines = linebuffer;
      linebuffer += sector->linecount;
      sector->linecount = 0;
      M_ClearBox(sector->blockbox);
    }
  }

  // Enter those lines
  for (i=0,li=lines; i<numlines; i++, li++)
  {
    P_AddLineToSector(li, li->frontsector);
    if (li->backsector && li->backsector != li->frontsector)
      P_AddLineToSector(li, li->backsector);
  }

  for (i=0, sector = sectors; i<numsectors; i++, sector++)
  {
    fixed_t *bbox = (void*)sector->blockbox; // cph - For convenience, so
                                  // I can sue the old code unchanged
    int block;

    // set the degenmobj_t to the middle of the bounding box
    if (comp[comp_sound])
    {
      sector->soundorg.x = (bbox[BOXRIGHT]+bbox[BOXLEFT])/2;
      sector->soundorg.y = (bbox[BOXTOP]+bbox[BOXBOTTOM])/2;
    }
    else
    {
      //e6y: fix sound origin for large levels
      sector->soundorg.x = bbox[BOXRIGHT]/2+bbox[BOXLEFT]/2;
      sector->soundorg.y = bbox[BOXTOP]/2+bbox[BOXBOTTOM]/2;
    }

    // adjust bounding box to map blocks
    block = (bbox[BOXTOP]-bmaporgy+MAXRADIUS)>>MAPBLOCKSHIFT;
    block = block >= bmapheight ? bmapheight-1 : block;
    sector->blockbox[BOXTOP]=block;

    block = (bbox[BOXBOTTOM]-bmaporgy-MAXRADIUS)>>MAPBLOCKSHIFT;
    block = block < 0 ? 0 : block;
    sector->blockbox[BOXBOTTOM]=block;

    block = (bbox[BOXRIGHT]-bmaporgx+MAXRADIUS)>>MAPBLOCKSHIFT;
    block = block >= bmapwidth ? bmapwidth-1 : block;
    sector->blockbox[BOXRIGHT]=block;

    block = (bbox[BOXLEFT]-bmaporgx-MAXRADIUS)>>MAPBLOCKSHIFT;
    block = block < 0 ? 0 : block;
    sector->blockbox[BOXLEFT]=block;
  }

  return total; // this value is needed by the reject overrun emulation code
}

//
// killough 10/98
//
// Remove slime trails.
//
// Slime trails are inherent to Doom's coordinate system -- i.e. there is
// nothing that a node builder can do to prevent slime trails ALL of the time,
// because it's a product of the integer coodinate system, and just because
// two lines pass through exact integer coordinates, doesn't necessarily mean
// that they will intersect at integer coordinates. Thus we must allow for
// fractional coordinates if we are to be able to split segs with node lines,
// as a node builder must do when creating a BSP tree.
//
// A wad file does not allow fractional coordinates, so node builders are out
// of luck except that they can try to limit the number of splits (they might
// also be able to detect the degree of roundoff error and try to avoid splits
// with a high degree of roundoff error). But we can use fractional coordinates
// here, inside the engine. It's like the difference between square inches and
// square miles, in terms of granularity.
//
// For each vertex of every seg, check to see whether it's also a vertex of
// the linedef associated with the seg (i.e, it's an endpoint). If it's not
// an endpoint, and it wasn't already moved, move the vertex towards the
// linedef by projecting it using the law of cosines. Formula:
//
//      2        2                         2        2
//    dx  x0 + dy  x1 + dx dy (y0 - y1)  dy  y0 + dx  y1 + dx dy (x0 - x1)
//   {---------------------------------, ---------------------------------}
//                  2     2                            2     2
//                dx  + dy                           dx  + dy
//
// (x0,y0) is the vertex being moved, and (x1,y1)-(x1+dx,y1+dy) is the
// reference linedef.
//
// Segs corresponding to orthogonal linedefs (exactly vertical or horizontal
// linedefs), which comprise at least half of all linedefs in most wads, don't
// need to be considered, because they almost never contribute to slime trails
// (because then any roundoff error is parallel to the linedef, which doesn't
// cause slime). Skipping simple orthogonal lines lets the code finish quicker.
//
// Please note: This section of code is not interchangable with TeamTNT's
// code which attempts to fix the same problem.
//
// Firelines (TM) is a Rezistered Trademark of MBF Productions
//

static void P_RemoveSlimeTrails(void)         // killough 10/98
{
  byte *hit = calloc(1, numvertexes);         // Hitlist for vertices
  int i;
  for (i=0; i<numsegs; i++)                   // Go through each seg
  {
    const line_t *l;

    if (segs[i].miniseg == true)        //figgi -- skip minisegs
      return;

    l = segs[i].linedef;            // The parent linedef
    if (l->dx && l->dy)                     // We can ignore orthogonal lines
    {
    vertex_t *v = segs[i].v1;
    do
      if (!hit[v - vertexes])           // If we haven't processed vertex
        {
    hit[v - vertexes] = 1;        // Mark this vertex as processed
    if (v != l->v1 && v != l->v2) // Exclude endpoints of linedefs
      { // Project the vertex back onto the parent linedef
        int_64_t dx2 = (l->dx >> FRACBITS) * (l->dx >> FRACBITS);
        int_64_t dy2 = (l->dy >> FRACBITS) * (l->dy >> FRACBITS);
        int_64_t dxy = (l->dx >> FRACBITS) * (l->dy >> FRACBITS);
        int_64_t s = dx2 + dy2;
        int x0 = v->x, y0 = v->y, x1 = l->v1->x, y1 = l->v1->y;
        v->x = (int)((dx2 * x0 + dy2 * x1 + dxy * (y0 - y1)) / s);
        v->y = (int)((dy2 * y0 + dx2 * y1 + dxy * (x0 - x1)) / s);
      }
        }  // Obsfucated C contest entry:   :)
    while ((v != segs[i].v2) && (v = segs[i].v2));
  }
    }
  free(hit);
}

//
// P_SetupLevel
//
// killough 5/3/98: reformatted, cleaned up

void P_SetupLevel(int episode, int map, int playermask, skill_t skill)
{
  int   i;
  char  lumpname[9];
  int   lumpnum;

  char  gl_lumpname[9];
  int   gl_lumpnum;

  R_StopAllInterpolations();

  totallive = totalkills = totalitems = totalsecret = wminfo.maxfrags = 0;
  wminfo.partime = 180;

  for (i=0; i<MAXPLAYERS; i++)
    players[i].killcount = players[i].secretcount = players[i].itemcount = 0;

  // Initial height of PointOfView will be set by player think.
  players[consoleplayer].viewz = 1;

  // Make sure all sounds are stopped before Z_FreeTags.
  S_Start();

  Z_FreeTags(PU_LEVEL, PU_PURGELEVEL);
  if (rejectlump != -1) { // cph - unlock the reject table
    W_UnlockLumpNum(rejectlump);
    rejectlump = -1;
  }

  P_InitThinkers();

  // if working with a devlopment map, reload it
  //    W_Reload ();     killough 1/31/98: W_Reload obsolete

  // find map name
  if (gamemode == commercial)
  {
    sprintf(lumpname, "map%02d", map);           // killough 1/24/98: simplify
    sprintf(gl_lumpname, "gl_map%02d", map);    // figgi
  }
  else
  {
    sprintf(lumpname, "E%dM%d", episode, map);   // killough 1/24/98: simplify
    sprintf(gl_lumpname, "GL_E%iM%i", episode, map); // figgi
  }

  lumpnum = W_GetNumForName(lumpname);
  gl_lumpnum = W_CheckNumForName(gl_lumpname); // figgi

  leveltime = 0; totallive = 0;

  // note: most of this ordering is important

  // killough 3/1/98: P_LoadBlockMap call moved down to below
  // killough 4/4/98: split load of sidedefs into two parts,
  // to allow texture names to be used in special linedefs

  // refuse to load Hexen-format maps, avoid segfaults
  if ((i = lumpnum + ML_BLOCKMAP + 1) < numlumps
      && !strncasecmp(lumpinfo[i].name, "BEHAVIOR", 8))
    I_Error("P_SetupLevel: %s: Hexen format not supported", lumpname);

#if 1
  // figgi 10/19/00 -- check for gl lumps and load them
  P_GetNodesVersion(lumpnum,gl_lumpnum);

  if (nodesVersion > 0)
    P_LoadVertexes2 (lumpnum+ML_VERTEXES,gl_lumpnum+ML_GL_VERTS);
  else
    P_LoadVertexes  (lumpnum+ML_VERTEXES);
  P_LoadSectors   (lumpnum+ML_SECTORS);
  P_LoadSideDefs  (lumpnum+ML_SIDEDEFS);
  P_LoadLineDefs  (lumpnum+ML_LINEDEFS);
  P_LoadSideDefs2 (lumpnum+ML_SIDEDEFS);
  P_LoadLineDefs2 (lumpnum+ML_LINEDEFS);
  P_LoadBlockMap  (lumpnum+ML_BLOCKMAP);

  if (nodesVersion > 0)
  {
    P_LoadSubsectors(gl_lumpnum + ML_GL_SSECT);
    P_LoadNodes(gl_lumpnum + ML_GL_NODES);
    P_LoadGLSegs(gl_lumpnum + ML_GL_SEGS);
  }
  else
  {
    P_LoadSubsectors(lumpnum + ML_SSECTORS);
    P_LoadNodes(lumpnum + ML_NODES);
    P_LoadSegs(lumpnum + ML_SEGS);
  }

#else

  P_LoadVertexes  (lumpnum+ML_VERTEXES);
  P_LoadSectors   (lumpnum+ML_SECTORS);
  P_LoadSideDefs  (lumpnum+ML_SIDEDEFS);             // killough 4/4/98
  P_LoadLineDefs  (lumpnum+ML_LINEDEFS);             //       |
  P_LoadSideDefs2 (lumpnum+ML_SIDEDEFS);             //       |
  P_LoadLineDefs2 (lumpnum+ML_LINEDEFS);             // killough 4/4/98
  P_LoadBlockMap  (lumpnum+ML_BLOCKMAP);             // killough 3/1/98
  P_LoadSubsectors(lumpnum+ML_SSECTORS);
  P_LoadNodes     (lumpnum+ML_NODES);
  P_LoadSegs      (lumpnum+ML_SEGS);

#endif

  // reject loading and underflow padding separated out into new function
  // P_GroupLines modified to return a number the underflow padding needs
  P_LoadReject(lumpnum, P_GroupLines());

  // e6y
  // Correction of desync on dv04-423.lmp/dv.wad
  // http://www.doomworld.com/vb/showthread.php?s=&postid=627257#post627257
  if (compatibility_level>=lxdoom_1_compatibility || M_CheckParm("-force_remove_slime_trails") > 0)
    P_RemoveSlimeTrails();    // killough 10/98: remove slime trails from wad

  // Note: you don't need to clear player queue slots --
  // a much simpler fix is in g_game.c -- killough 10/98

  bodyqueslot = 0;

  /* cph - reset all multiplayer starts */
  memset(playerstarts,0,sizeof(playerstarts));
  deathmatch_p = deathmatchstarts;
  for (i = 0; i < MAXPLAYERS; i++)
    players[i].mo = NULL;

  P_MapStart();

  P_LoadThings(lumpnum+ML_THINGS);

  // if deathmatch, randomly spawn the active players
  if (deathmatch)
  {
    for (i=0; i<MAXPLAYERS; i++)
      if (playeringame[i])
        {
          players[i].mo = NULL; // not needed? - done before P_LoadThings
          G_DeathMatchSpawnPlayer(i);
        }
  }
  else // if !deathmatch, check all necessary player starts actually exist
  {
    for (i=0; i<MAXPLAYERS; i++)
      if (playeringame[i] && !players[i].mo)
        I_Error("P_SetupLevel: missing player %d start\n", i+1);
  }

  // killough 3/26/98: Spawn icon landings:
  if (gamemode==commercial)
    P_SpawnBrainTargets();

  // clear special respawning que
  iquehead = iquetail = 0;

  // set up world state
  P_SpawnSpecials();

  P_MapEnd();

  // preload graphics
  if (precache)
    R_PrecacheLevel();

  R_SmoothPlaying_Reset(NULL); // e6y
}

//
// P_Init
//
void P_Init (void)
{
  P_InitSwitchList();
  P_InitPicAnims();
  R_InitSprites(sprnames);
}
