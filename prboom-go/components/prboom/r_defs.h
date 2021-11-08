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
 *      Refresh/rendering module, shared data struct definitions.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __R_DEFS__
#define __R_DEFS__

// Screenwidth.
#include "doomdef.h"

// Some more or less basic data types
// we depend on.
#include "m_fixed.h"

// We rely on the thinker data struct
// to handle sound origins in sectors.
#include "d_think.h"

// SECTORS do store MObjs anyway.
#include "p_mobj.h"

#ifdef __GNUG__
#pragma interface
#endif

// Silhouette, needed for clipping Segs (mainly)
// and sprites representing things.
#define SIL_NONE    0
#define SIL_BOTTOM  1
#define SIL_TOP     2
#define SIL_BOTH    3

#define MAXDRAWSEGS   256

//
// INTERNAL MAP TYPES
//  used by play and refresh
//

//
// Your plain vanilla vertex.
// Note: transformed values not buffered locally,
// like some DOOM-alikes ("wt", "WebView") do.
//
typedef struct
{
  fixed_t x, y;
} vertex_t;

// Each sector has a degenmobj_t in its center for sound origin purposes.
typedef struct
{
  thinker_t thinker;  // not used for anything
  fixed_t x, y, z;
} degenmobj_t;

//
// The SECTORS record, at runtime.
// Stores things/mobjs.
//

typedef struct
{
  int iSectorID; // proff 04/05/2000: needed for OpenGL and used in debugmode by the HUD to draw sectornum
  boolean no_toptextures;
  boolean no_bottomtextures;
  fixed_t floorheight;
  fixed_t ceilingheight;
  int nexttag,firsttag;  // killough 1/30/98: improves searches for tags.
  int soundtraversed;    // 0 = untraversed, 1,2 = sndlines-1
  mobj_t *soundtarget;   // thing that made a sound (or null)
  int blockbox[4];       // mapblock bounding box for height changes
  degenmobj_t soundorg;  // origin for any sounds played by the sector
  int validcount;        // if == validcount, already checked
  mobj_t *thinglist;     // list of mobjs in sector

  /* killough 8/28/98: friction is a sector property, not an mobj property.
   * these fields used to be in mobj_t, but presented performance problems
   * when processed as mobj properties. Fix is to make them sector properties.
   */
  int friction,movefactor;

  // thinker_t for reversable actions
  void *floordata;    // jff 2/22/98 make thinkers on
  void *ceilingdata;  // floors, ceilings, lighting,
  void *lightingdata; // independent of one another

  // jff 2/26/98 lockout machinery for stairbuilding
  int stairlock;   // -2 on first locked -1 after thinker done 0 normally
  int prevsec;     // -1 or number of sector for previous step
  int nextsec;     // -1 or number of next step sector

  // killough 3/7/98: support flat heights drawn at another sector's heights
  int heightsec;    // other sector, or -1 if no other sector

  int bottommap, midmap, topmap; // killough 4/4/98: dynamic colormaps

  // list of mobjs that are at least partially in the sector
  // thinglist is a subset of touching_thinglist
  struct msecnode_s *touching_thinglist;               // phares 3/14/98

  int linecount;
  struct line_s **lines;

  // killough 10/98: support skies coming from sidedefs. Allows scrolling
  // skies and other effects. No "level info" kind of lump is needed,
  // because you can use an arbitrary number of skies per level with this
  // method. This field only applies when skyflatnum is used for floorpic
  // or ceilingpic, because the rest of Doom needs to know which is sky
  // and which isn't, etc.

  int sky;

  // killough 3/7/98: floor and ceiling texture offsets
  fixed_t   floor_xoffs,   floor_yoffs;
  fixed_t ceiling_xoffs, ceiling_yoffs;

  // killough 4/11/98: support for lightlevels coming from another sector
  int floorlightsec, ceilinglightsec;

  short floorpic;
  short ceilingpic;
  short lightlevel;
  short special;
  short oldspecial;      //jff 2/16/98 remembers if sector WAS secret (automap)
  short tag;
} sector_t;

//
// The SideDef.
//

typedef struct
{
  fixed_t textureoffset; // add this to the calculated texture column
  fixed_t rowoffset;     // add this to the calculated texture top
  short toptexture;      // Texture indices. We do not maintain names here.
  short bottomtexture;
  short midtexture;
  sector_t* sector;      // Sector the SideDef is facing.

  // killough 4/4/98, 4/11/98: highest referencing special linedef's type,
  // or lump number of special effect. Allows texture names to be overloaded
  // for other functions.

  int special;

} side_t;

//
// Move clipping aid for LineDefs.
//
typedef enum
{
  ST_HORIZONTAL,
  ST_VERTICAL,
  ST_POSITIVE,
  ST_NEGATIVE
} slopetype_t;

typedef struct line_s
{
  int iLineID;           // proff 04/05/2000: needed for OpenGL
  vertex_t *v1, *v2;     // Vertices, from v1 to v2.
  fixed_t dx, dy;        // Precalculated v2 - v1 for side checking.
  unsigned short flags;           // Animation related.
  short special;
  short tag;
  unsigned short sidenum[2];        // Visual appearance: SideDefs.
  fixed_t bbox[4];       // A bounding box, for the linedef's extent
  slopetype_t slopetype; // To aid move clipping.
  sector_t *frontsector; // Front and back sector.
  sector_t *backsector;
  int validcount;        // if == validcount, already checked
  void *specialdata;     // thinker_t for reversable actions
  int tranlump;          // killough 4/11/98: translucency filter, -1 == none
  int firsttag,nexttag;  // killough 4/17/98: improves searches for tags.
  int r_validcount;      // cph: if == gametic, r_flags already done
  enum {                 // cph:
    RF_TOP_TILE  = 1,     // Upper texture needs tiling
    RF_MID_TILE = 2,     // Mid texture needs tiling
    RF_BOT_TILE = 4,     // Lower texture needs tiling
    RF_IGNORE   = 8,     // Renderer can skip this line
    RF_CLOSED   =16,     // Line blocks view
  } r_flags;
  degenmobj_t soundorg;  // sound origin for switches/buttons
} line_t;


// phares 3/14/98
//
// Sector list node showing all sectors an object appears in.
//
// There are two threads that flow through these nodes. The first thread
// starts at touching_thinglist in a sector_t and flows through the m_snext
// links to find all mobjs that are entirely or partially in the sector.
// The second thread starts at touching_sectorlist in an mobj_t and flows
// through the m_tnext links to find all sectors a thing touches. This is
// useful when applying friction or push effects to sectors. These effects
// can be done as thinkers that act upon all objects touching their sectors.
// As an mobj moves through the world, these nodes are created and
// destroyed, with the links changed appropriately.
//
// For the links, NULL means top or end of list.

typedef struct msecnode_s
{
  sector_t          *m_sector; // a sector containing this object
  struct mobj_s     *m_thing;  // this object
  struct msecnode_s *m_tprev;  // prev msecnode_t for this thing
  struct msecnode_s *m_tnext;  // next msecnode_t for this thing
  struct msecnode_s *m_sprev;  // prev msecnode_t for this sector
  struct msecnode_s *m_snext;  // next msecnode_t for this sector
  boolean visited; // killough 4/4/98, 4/7/98: used in search algorithms
} msecnode_t;

//
// The LineSeg.
//
typedef struct
{
  vertex_t *v1, *v2;
  fixed_t offset;
  angle_t angle;
  side_t* sidedef;
  line_t* linedef;

  int iSegID; // proff 11/05/2000: needed for OpenGL
  // figgi -- needed for glnodes
  float     length;
  boolean   miniseg;


  // Sector references.
  // Could be retrieved from linedef, too
  // (but that would be slower -- killough)
  // backsector is NULL for one sided lines

  sector_t *frontsector, *backsector;
} seg_t;


//
// A SubSector.
// References a Sector.
// Basically, this is a list of LineSegs,
//  indicating the visible walls that define
//  (all or some) sides of a convex BSP leaf.
//

typedef struct subsector_s
{
  sector_t *sector;
  unsigned short numlines, firstline;
} subsector_t;


//
// BSP node.
//
typedef struct
{
  fixed_t  x,  y, dx, dy;        // Partition line.
  fixed_t bbox[2][4];            // Bounding box for each child.
  unsigned short children[2];    // If NF_SUBSECTOR its a subsector.
} node_t;

//
// OTHER TYPES
//

// This could be wider for >8 bit display.
// Indeed, true color support is posibble
// precalculating 24bpp lightmap/colormap LUT.
// from darkening PLAYPAL to all black.
// Could use even more than 32 levels.

typedef byte  lighttable_t;

//
// Masked 2s linedefs
//

typedef struct drawseg_s
{
  seg_t *curline;
  int x1, x2;
  fixed_t scale1, scale2, scalestep;
  int silhouette;                       // 0=none, 1=bottom, 2=top, 3=both
  fixed_t bsilheight;                   // do not clip sprites above this
  fixed_t tsilheight;                   // do not clip sprites below this

  // Added for filtering (fractional texture u coord) support - POPE
  fixed_t rw_offset, rw_distance, rw_centerangle; 
  
  // Pointers to lists for sprite clipping,
  // all three adjusted so [x1] is first value.

  int *sprtopclip, *sprbottomclip, *maskedtexturecol; // dropoff overflow
} drawseg_t;

// proff: Added for OpenGL
typedef struct
{
  int width,height;
  int leftoffset,topoffset;
  int lumpnum;
} patchnum_t;

//
// A vissprite_t is a thing that will be drawn during a refresh.
// i.e. a sprite object that is partly visible.
//

typedef struct vissprite_s
{
  mobj_t *thing;
  boolean flip;
  int x1, x2;
  fixed_t gx, gy;              // for line side calculation
  fixed_t gz, gzt;             // global bottom / top for silhouette clipping
  fixed_t startfrac;           // horizontal position of x1
  fixed_t scale;
  fixed_t xiscale;             // negative if flipped
  fixed_t texturemid;
  int patch;
  uint_64_t mobjflags;

  // for color translation and shadow draw, maxbright frames as well
  const lighttable_t *colormap;

  // killough 3/27/98: height sector for underwater/fake ceiling support
  int heightsec;

  boolean isplayersprite;
} vissprite_t;

//
// Sprites are patches with a special naming convention
//  so they can be recognized by R_InitSprites.
// The base name is NNNNFx or NNNNFxFx, with
//  x indicating the rotation, x = 0, 1-7.
// The sprite and frame specified by a thing_t
//  is range checked at run time.
// A sprite is a patch_t that is assumed to represent
//  a three dimensional object and may have multiple
//  rotations pre drawn.
// Horizontal flipping is used to save space,
//  thus NNNNF2F5 defines a mirrored patch.
// Some sprites will only have one picture used
// for all views: NNNNF0
//

typedef struct
{
  // If false use 0 for any position.
  // Note: as eight entries are available,
  //  we might as well insert the same name eight times.
  boolean rotate;

  // Lump to use for view angles 0-7.
  short lump[8];

  // Flip bit (1 = flip) to use for view angles 0-7.
  byte  flip[8];

} spriteframe_t;

//
// A sprite definition:
//  a number of animation frames.
//

typedef struct
{
  int numframes;
  spriteframe_t *spriteframes;
} spritedef_t;

//
// Now what is a visplane, anyway?
//
// Go to http://classicgaming.com/doom/editing/ to find out -- killough
//

typedef struct visplane
{
  struct visplane *next;        // Next visplane in hash chain -- killough
  int picnum, lightlevel, minx, maxx;
  fixed_t height;
  fixed_t xoffs, yoffs;         // killough 2/28/98: Support scrolling flats
  unsigned int pad1;          // leave pads for [minx-1]/[maxx+1]
  unsigned int top[MAX_SCREENWIDTH];
  unsigned int pad2, pad3;    // killough 2/8/98, 4/25/98
  unsigned int bottom[MAX_SCREENWIDTH];
  unsigned int pad4; // dropoff overflow
} visplane_t;

#endif
