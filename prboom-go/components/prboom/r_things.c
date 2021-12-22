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
 *  Refresh of things, i.e. objects represented by sprites.
 *
 *-----------------------------------------------------------------------------*/

#include "doomstat.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_bsp.h"
#include "r_segs.h"
#include "r_draw.h"
#include "r_things.h"
#include "r_fps.h"
#include "v_video.h"
#include "lprintf.h"

#define MINZ        (FRACUNIT*4)
#define BASEYCENTER 100

typedef struct {
  int x1;
  int x2;
  int column;
  int topclip;
  int bottomclip;
} maskdraw_t;

//
// Sprite rotation 0 is facing the viewer,
//  rotation 1 is one angle turn CLOCKWISE around the axis.
// This is not the same as the angle,
//  which increases counter clockwise (protractor).
// There was a lot of stuff grabbed wrong, so I changed it...
//

fixed_t pspritescale;
fixed_t pspriteiscale;
// proff 11/06/98: Added for high-res
fixed_t pspriteyscale;

// constant arrays
//  used for psprite clipping and initializing clipping

int negonearray[MAX_SCREENWIDTH];        // killough 2/8/98: // dropoff overflow
int screenheightarray[MAX_SCREENWIDTH];  // change to MAX_* // dropoff overflow

//
// INITIALIZATION FUNCTIONS
//

// variables used to look up and range check thing_t sprites patches

spritedef_t *sprites;
int numsprites;

#define MAX_SPRITE_FRAMES 29          /* Macroized -- killough 1/25/98 */

static spriteframe_t sprtemp[MAX_SPRITE_FRAMES];
static int maxframe;

//
// R_InstallSpriteLump
// Local function for R_InitSprites.
//

static void R_InstallSpriteLump(int lump, unsigned frame,
                                unsigned rotation, boolean flipped)
{
  if (frame >= MAX_SPRITE_FRAMES || rotation > 8)
    I_Error("R_InstallSpriteLump: Bad frame characters in lump %i", lump);

  if ((int) frame > maxframe)
    maxframe = frame;

  if (rotation == 0)
    {    // the lump should be used for all rotations
      int r;
      for (r=0 ; r<8 ; r++)
        if (sprtemp[frame].lump[r]==-1)
          {
            sprtemp[frame].lump[r] = lump - firstspritelump;
            sprtemp[frame].flip[r] = (byte) flipped;
            sprtemp[frame].rotate = false; //jff 4/24/98 if any subbed, rotless
          }
      return;
    }

  // the lump is only used for one rotation

  if (sprtemp[frame].lump[--rotation] == -1)
    {
      sprtemp[frame].lump[rotation] = lump - firstspritelump;
      sprtemp[frame].flip[rotation] = (byte) flipped;
      sprtemp[frame].rotate = true; //jff 4/24/98 only change if rot used
    }
}

//
// R_InitSpriteDefs
// Pass a null terminated list of sprite names
// (4 chars exactly) to be used.
//
// Builds the sprite rotation matrixes to account
// for horizontally flipped sprites.
//
// Will report an error if the lumps are inconsistent.
// Only called at startup.
//
// Sprite lump names are 4 characters for the actor,
//  a letter for the frame, and a number for the rotation.
//
// A sprite that is flippable will have an additional
//  letter/number appended.
//
// The rotation character can be 0 to signify no rotations.
//
// 1/25/98, 1/31/98 killough : Rewritten for performance
//
// Empirically verified to have excellent hash
// properties across standard Doom sprites:

#define R_SpriteNameHash(s) ((unsigned)((s)[0]-((s)[1]*3-(s)[3]*2-(s)[2])*2))

static void R_InitSpriteDefs(const char * const * namelist)
{
  size_t numentries = lastspritelump-firstspritelump+1;
  struct { short index, next; } *hash;
  int i;

  if (!numentries || !*namelist)
    return;

  // count the number of sprite names
  for (i=0; namelist[i]; i++)
    ;

  numsprites = i;

  sprites = Z_Malloc(numsprites * sizeof(*sprites), PU_STATIC, NULL);
  hash = Z_Malloc(numentries * sizeof(*hash), PU_STATIC, NULL);

  for (i=0; (size_t)i<numentries; i++)             // initialize hash table as empty
    hash[i].index = -1;

  for (i=0; (size_t)i<numentries; i++)             // Prepend each sprite to hash chain
    {                                      // prepend so that later ones win
      int j = R_SpriteNameHash(lumpinfo[i+firstspritelump].name) % numentries;
      hash[i].next = hash[j].index;
      hash[j].index = i;
    }

  // scan all the lump names for each of the names,
  //  noting the highest frame letter.

  for (i=0 ; i<numsprites ; i++)
    {
      const char *spritename = namelist[i];
      int j = hash[R_SpriteNameHash(spritename) % numentries].index;

      if (j >= 0)
        {
          memset(sprtemp, -1, sizeof(sprtemp));
          maxframe = -1;
          do
            {
              register lumpinfo_t *lump = lumpinfo + j + firstspritelump;

              // Fast portable comparison -- killough
              // (using int pointer cast is nonportable):

              if (!((lump->name[0] ^ spritename[0]) |
                    (lump->name[1] ^ spritename[1]) |
                    (lump->name[2] ^ spritename[2]) |
                    (lump->name[3] ^ spritename[3])))
                {
                  R_InstallSpriteLump(j+firstspritelump,
                                      lump->name[4] - 'A',
                                      lump->name[5] - '0',
                                      false);
                  if (lump->name[6])
                    R_InstallSpriteLump(j+firstspritelump,
                                        lump->name[6] - 'A',
                                        lump->name[7] - '0',
                                        true);
                }
            }
          while ((j = hash[j].next) >= 0);

          // check the frames that were found for completeness
          if ((sprites[i].numframes = ++maxframe))  // killough 1/31/98
            {
              int frame;
              for (frame = 0; frame < maxframe; frame++)
                switch ((int) sprtemp[frame].rotate)
                  {
                  case -1:
                    // no rotations were found for that frame at all
                    I_Error ("R_InitSprites: No patches found "
                             "for %.8s frame %c", namelist[i], frame+'A');
                    break;

                  case 0:
                    // only the first rotation is needed
                    break;

                  case 1:
                    // must have all 8 frames
                    {
                      int rotation;
                      for (rotation=0 ; rotation<8 ; rotation++)
                        if (sprtemp[frame].lump[rotation] == -1)
                          I_Error ("R_InitSprites: Sprite %.8s frame %c "
                                   "is missing rotations",
                                   namelist[i], frame+'A');
                      break;
                    }
                  }
              // allocate space for the frames present and copy sprtemp to it
              sprites[i].spriteframes =
                Z_Malloc (maxframe * sizeof(spriteframe_t), PU_STATIC, NULL);
              memcpy (sprites[i].spriteframes, sprtemp,
                      maxframe*sizeof(spriteframe_t));
            }
        }
    }
  free(hash);             // free hash table
}

//
// GAME FUNCTIONS
//

static vissprite_t *vissprites, **vissprite_ptrs;  // killough
static size_t num_vissprite, num_vissprite_alloc, num_vissprite_ptrs;

//
// R_InitSprites
// Called at program start.
//

void R_InitSprites(const char * const *namelist)
{
  int i;
  for (i=0; i<MAX_SCREENWIDTH; i++)    // killough 2/8/98
    negonearray[i] = -1;
  R_InitSpriteDefs(namelist);
}

//
// R_ClearSprites
// Called at frame start.
//

void R_ClearSprites (void)
{
  num_vissprite = 0;            // killough
}

//
// R_NewVisSprite
//

static vissprite_t *R_NewVisSprite(void)
{
  if (num_vissprite >= num_vissprite_alloc)             // killough
    {
      size_t num_vissprite_alloc_prev = num_vissprite_alloc;

      num_vissprite_alloc = num_vissprite_alloc ? num_vissprite_alloc*2 : 128;
      vissprites = realloc(vissprites,num_vissprite_alloc*sizeof(*vissprites));

      //e6y: set all fields to zero
      memset(vissprites + num_vissprite_alloc_prev, 0,
        (num_vissprite_alloc - num_vissprite_alloc_prev)*sizeof(*vissprites));
    }
 return vissprites + num_vissprite++;
}

//
// R_DrawMaskedColumn
// Used for sprites and masked mid textures.
// Masked means: partly transparent, i.e. stored
//  in posts/runs of opaque pixels.
//

int   *mfloorclip;   // dropoff overflow
int   *mceilingclip; // dropoff overflow
fixed_t spryscale;
fixed_t sprtopscreen;

void R_DrawMaskedColumn(
  const rpatch_t *patch,
  R_DrawColumn_f colfunc,
  draw_column_vars_t *dcvars,
  const rcolumn_t *column,
  const rcolumn_t *prevcolumn,
  const rcolumn_t *nextcolumn
)
{
  int     i;
  int     topscreen;
  int     bottomscreen;
  fixed_t basetexturemid = dcvars->texturemid;

  dcvars->texheight = patch->height; // killough 11/98
  for (i=0; i<column->numPosts; i++) {
      const rpost_t *post = &column->posts[i];

      // calculate unclipped screen coordinates for post
      topscreen = sprtopscreen + spryscale*post->topdelta;
      bottomscreen = topscreen + spryscale*post->length;

      dcvars->yl = (topscreen+FRACUNIT-1)>>FRACBITS;
      dcvars->yh = (bottomscreen-1)>>FRACBITS;

      if (dcvars->yh >= mfloorclip[dcvars->x])
        dcvars->yh = mfloorclip[dcvars->x]-1;

      if (dcvars->yl <= mceilingclip[dcvars->x])
        dcvars->yl = mceilingclip[dcvars->x]+1;

      // killough 3/2/98, 3/27/98: Failsafe against overflow/crash:
      if (dcvars->yl <= dcvars->yh && dcvars->yh < viewheight)
        {
          dcvars->source = column->pixels + post->topdelta;
          dcvars->prevsource = prevcolumn->pixels + post->topdelta;
          dcvars->nextsource = nextcolumn->pixels + post->topdelta;

          dcvars->texturemid = basetexturemid - (post->topdelta<<FRACBITS);

          dcvars->edgeslope = post->slope;
          // Drawn by either R_DrawColumn
          //  or (SHADOW) R_DrawFuzzColumn.
          dcvars->drawingmasked = 1; // POPE
          colfunc (dcvars);
          dcvars->drawingmasked = 0; // POPE
        }
    }
  dcvars->texturemid = basetexturemid;
}

//
// R_DrawVisSprite
//  mfloorclip and mceilingclip should also be set.
//
// CPhipps - new wad lump handling, *'s to const*'s
static void R_DrawVisSprite(vissprite_t *vis, int x1, int x2)
{
  int      texturecolumn;
  fixed_t  frac;
  const rpatch_t *patch = R_CachePatchNum(vis->patch+firstspritelump);
  R_DrawColumn_f colfunc;
  draw_column_vars_t dcvars;
  enum draw_filter_type_e filter;
  enum draw_filter_type_e filterz;

  R_SetDefaultDrawColumnVars(&dcvars);
  if (vis->isplayersprite) {
    dcvars.edgetype = drawvars.patch_edges;
    filter = drawvars.filterpatch;
    filterz = RDRAW_FILTER_POINT;
  } else {
    dcvars.edgetype = drawvars.sprite_edges;
    filter = drawvars.filtersprite;
    filterz = drawvars.filterz;
  }

  dcvars.colormap = vis->colormap;
  dcvars.nextcolormap = dcvars.colormap; // for filtering -- POPE

  // killough 4/11/98: rearrange and handle translucent sprites
  // mixed with translucent/non-translucenct 2s normals

  if (!dcvars.colormap)   // NULL colormap = shadow draw
    colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_FUZZ, filter, filterz);    // killough 3/14/98
  else
    if (vis->mobjflags & MF_TRANSLATION)
      {
        colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_TRANSLATED, filter, filterz);
        dcvars.translation = translationtables - 256 +
          ((vis->mobjflags & MF_TRANSLATION) >> (MF_TRANSSHIFT-8) );
      }
    else
      if (vis->mobjflags & MF_TRANSLUCENT && general_translucency) // phares
        {
          colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_TRANSLUCENT, filter, filterz);
          tranmap = main_tranmap;       // killough 4/11/98
        }
      else
        colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_STANDARD, filter, filterz); // killough 3/14/98, 4/11/98

// proff 11/06/98: Changed for high-res
  dcvars.iscale = FixedDiv (FRACUNIT, vis->scale);
  dcvars.texturemid = vis->texturemid;
  frac = vis->startfrac;
  if (filter == RDRAW_FILTER_LINEAR)
    frac -= (FRACUNIT>>1);
  spryscale = vis->scale;
  sprtopscreen = centeryfrac - FixedMul(dcvars.texturemid,spryscale);

  for (dcvars.x=vis->x1 ; dcvars.x<=vis->x2 ; dcvars.x++, frac += vis->xiscale)
    {
      texturecolumn = frac>>FRACBITS;
      dcvars.texu = frac;

      R_DrawMaskedColumn(
        patch,
        colfunc,
        &dcvars,
        R_GetPatchColumnClamped(patch, texturecolumn),
        R_GetPatchColumnClamped(patch, texturecolumn-1),
        R_GetPatchColumnClamped(patch, texturecolumn+1)
      );
    }
  R_UnlockPatchNum(vis->patch+firstspritelump); // cph - release lump
}

//
// R_ProjectSprite
// Generates a vissprite for a thing if it might be visible.
//

static void R_ProjectSprite (mobj_t* thing, int lightlevel)
{
  fixed_t   gzt;               // killough 3/27/98
  fixed_t   tx;
  fixed_t   xscale;
  int       x1;
  int       x2;
  spritedef_t   *sprdef;
  spriteframe_t *sprframe;
  int       lump;
  boolean   flip;
  vissprite_t *vis;
  fixed_t   iscale;
  int heightsec;      // killough 3/27/98

  // transform the origin point
  fixed_t tr_x, tr_y;
  fixed_t fx, fy, fz;
  fixed_t gxt, gyt;
  fixed_t tz;
  int width;

  if (movement_smooth)
  {
    fx = thing->PrevX + FixedMul (tic_vars.frac, thing->x - thing->PrevX);
    fy = thing->PrevY + FixedMul (tic_vars.frac, thing->y - thing->PrevY);
    fz = thing->PrevZ + FixedMul (tic_vars.frac, thing->z - thing->PrevZ);
  }
  else
  {
    fx = thing->x;
    fy = thing->y;
    fz = thing->z;
  }
  tr_x = fx - viewx;
  tr_y = fy - viewy;

  gxt = FixedMul(tr_x,viewcos);
  gyt = -FixedMul(tr_y,viewsin);

  tz = gxt-gyt;

    // thing is behind view plane?
  if (tz < MINZ)
    return;

  xscale = FixedDiv(projection, tz);

  gxt = -FixedMul(tr_x,viewsin);
  gyt = FixedMul(tr_y,viewcos);
  tx = -(gyt+gxt);

  // too far off the side?
  if (D_abs(tx)>(tz<<2))
    return;

    // decide which patch to use for sprite relative to player
#ifdef RANGECHECK
  if ((unsigned) thing->sprite >= (unsigned)numsprites)
    I_Error ("R_ProjectSprite: Invalid sprite number %i", thing->sprite);
#endif

  sprdef = &sprites[thing->sprite];

#ifdef RANGECHECK
  if ((thing->frame&FF_FRAMEMASK) >= sprdef->numframes)
    I_Error ("R_ProjectSprite: Invalid sprite frame %i : %i", thing->sprite,
             thing->frame);
#endif

  if (!sprdef->spriteframes)
    I_Error ("R_ProjectSprite: Missing spriteframes %i : %i", thing->sprite,
             thing->frame);

  sprframe = &sprdef->spriteframes[thing->frame & FF_FRAMEMASK];

  if (sprframe->rotate)
    {
      // choose a different rotation based on player view
      angle_t ang = R_PointToAngle(fx, fy);
      unsigned rot = (ang-thing->angle+(unsigned)(ANG45/2)*9)>>29;
      lump = sprframe->lump[rot];
      flip = (boolean) sprframe->flip[rot];
    }
  else
    {
      // use single rotation for all views
      lump = sprframe->lump[0];
      flip = (boolean) sprframe->flip[0];
    }

  {
    const rpatch_t* patch = R_CachePatchNum(lump+firstspritelump);

    /* calculate edges of the shape
     * cph 2003/08/1 - fraggle points out that this offset must be flipped
     * if the sprite is flipped; e.g. FreeDoom imp is messed up by this. */
    if (flip) {
      tx -= (patch->width - patch->leftoffset) << FRACBITS;
    } else {
      tx -= patch->leftoffset << FRACBITS;
    }
    x1 = (centerxfrac + FixedMul(tx,xscale)) >> FRACBITS;

    tx += patch->width<<FRACBITS;
    x2 = ((centerxfrac + FixedMul (tx,xscale) ) >> FRACBITS) - 1;

    gzt = fz + (patch->topoffset << FRACBITS);
    width = patch->width;
    R_UnlockPatchNum(lump+firstspritelump);
  }

  // off the side?
  if (x1 > viewwidth || x2 < 0)
    return;

  // killough 4/9/98: clip things which are out of view due to height
  // e6y: fix of hanging decoration disappearing in Batman Doom MAP02
  // centeryfrac -> viewheightfrac
  if (fz  > viewz + FixedDiv(viewheightfrac, xscale) ||
      gzt < viewz - FixedDiv(viewheightfrac-viewheight, xscale))
    return;

    // killough 3/27/98: exclude things totally separated
    // from the viewer, by either water or fake ceilings
    // killough 4/11/98: improve sprite clipping for underwater/fake ceilings

  heightsec = thing->subsector->sector->heightsec;

  if (heightsec != -1)   // only clip things which are in special sectors
    {
      int phs = viewplayer->mo->subsector->sector->heightsec;
      if (phs != -1 && viewz < sectors[phs].floorheight ?
          fz >= sectors[heightsec].floorheight :
          gzt < sectors[heightsec].floorheight)
        return;
      if (phs != -1 && viewz > sectors[phs].ceilingheight ?
          gzt < sectors[heightsec].ceilingheight &&
          viewz >= sectors[heightsec].ceilingheight :
          fz >= sectors[heightsec].ceilingheight)
        return;
    }

  // store information in a vissprite
  vis = R_NewVisSprite ();

  // killough 3/27/98: save sector for special clipping later
  vis->heightsec = heightsec;

  vis->mobjflags = thing->flags;
// proff 11/06/98: Changed for high-res
  vis->scale = FixedDiv(projectiony, tz);
  vis->gx = fx;
  vis->gy = fy;
  vis->gz = fz;
  vis->gzt = gzt;                          // killough 3/27/98
  vis->texturemid = vis->gzt - viewz;
  vis->x1 = x1 < 0 ? 0 : x1;
  vis->x2 = x2 >= viewwidth ? viewwidth-1 : x2;
  iscale = FixedDiv (FRACUNIT, xscale);

  if (flip)
    {
      vis->startfrac = (width<<FRACBITS)-1;
      vis->xiscale = -iscale;
    }
  else
    {
      vis->startfrac = 0;
      vis->xiscale = iscale;
    }

  if (vis->x1 > x1)
    vis->startfrac += vis->xiscale*(vis->x1-x1);
  vis->patch = lump;

  // get light level
  if (thing->flags & MF_SHADOW)
      vis->colormap = NULL;             // shadow draw
  else if (fixedcolormap)
    vis->colormap = fixedcolormap;      // fixed map
  else if (thing->frame & FF_FULLBRIGHT)
    vis->colormap = fullcolormap;     // full bright  // killough 3/20/98
  else
    {      // diminished light
      vis->colormap = R_ColourMap(lightlevel,xscale);
    }
}

//
// R_AddSprites
// During BSP traversal, this adds sprites by sector.
//
// killough 9/18/98: add lightlevel as parameter, fixing underwater lighting
void R_AddSprites(subsector_t* subsec, int lightlevel)
{
  sector_t* sec=subsec->sector;
  mobj_t *thing;

  // BSP is traversed by subsector.
  // A sector might have been split into several
  //  subsectors during BSP building.
  // Thus we check whether its already added.

  if (sec->validcount == validcount)
    return;

  // Well, now it will be done.
  sec->validcount = validcount;

  // Handle all things in sector.

  for (thing = sec->thinglist; thing; thing = thing->snext)
    R_ProjectSprite(thing, lightlevel);
}

//
// R_DrawPSprite
//

static void R_DrawPSprite (pspdef_t *psp, int lightlevel)
{
  int           x1, x2;
  spritedef_t   *sprdef;
  spriteframe_t *sprframe;
  int           lump;
  boolean       flip;
  vissprite_t   *vis;
  vissprite_t   avis;
  int           width;
  fixed_t       topoffset;

  avis.isplayersprite = true;

  // decide which patch to use

#ifdef RANGECHECK
  if ( (unsigned)psp->state->sprite >= (unsigned)numsprites)
    I_Error ("R_ProjectSprite: Invalid sprite number %i", psp->state->sprite);
#endif

  sprdef = &sprites[psp->state->sprite];

#ifdef RANGECHECK
  if ( (psp->state->frame & FF_FRAMEMASK)  >= sprdef->numframes)
    I_Error ("R_ProjectSprite: Invalid sprite frame %i : %li",
             psp->state->sprite, psp->state->frame);
#endif

  sprframe = &sprdef->spriteframes[psp->state->frame & FF_FRAMEMASK];

  lump = sprframe->lump[0];
  flip = (boolean) sprframe->flip[0];

  {
    const rpatch_t* patch = R_CachePatchNum(lump+firstspritelump);
    // calculate edges of the shape
    fixed_t       tx;
    tx = psp->sx-160*FRACUNIT;

    tx -= patch->leftoffset<<FRACBITS;
    x1 = (centerxfrac + FixedMul (tx,pspritescale))>>FRACBITS;

    tx += patch->width<<FRACBITS;
    x2 = ((centerxfrac + FixedMul (tx, pspritescale) ) >>FRACBITS) - 1;

    width = patch->width;
    topoffset = patch->topoffset<<FRACBITS;
    R_UnlockPatchNum(lump+firstspritelump);
  }

  // off the side
  if (x2 < 0 || x1 > viewwidth)
    return;

  // store information in a vissprite
  vis = &avis;
  vis->mobjflags = 0;
   // killough 12/98: fix psprite positioning problem
  vis->texturemid = (BASEYCENTER<<FRACBITS) /* +  FRACUNIT/2 */ -
                    (psp->sy-topoffset);
  vis->x1 = x1 < 0 ? 0 : x1;
  vis->x2 = x2 >= viewwidth ? viewwidth-1 : x2;
// proff 11/06/98: Added for high-res
  vis->scale = pspriteyscale;

  if (flip)
    {
      vis->xiscale = -pspriteiscale;
      vis->startfrac = (width<<FRACBITS)-1;
    }
  else
    {
      vis->xiscale = pspriteiscale;
      vis->startfrac = 0;
    }

  if (vis->x1 > x1)
    vis->startfrac += vis->xiscale*(vis->x1-x1);

  vis->patch = lump;

  if (viewplayer->powers[pw_invisibility] > 4*32
      || viewplayer->powers[pw_invisibility] & 8)
    vis->colormap = NULL;                    // shadow draw
  else if (fixedcolormap)
    vis->colormap = fixedcolormap;           // fixed color
  else if (psp->state->frame & FF_FULLBRIGHT)
    vis->colormap = fullcolormap;            // full bright // killough 3/20/98
  else
    // add a fudge factor to better match the original game
    vis->colormap = R_ColourMap(lightlevel,
        FixedMul(pspritescale, 0x2b000));  // local light

  R_DrawVisSprite(vis, vis->x1, vis->x2);
}

//
// R_DrawPlayerSprites
//

void R_DrawPlayerSprites(void)
{
  int i, lightlevel = viewplayer->mo->subsector->sector->lightlevel;
  pspdef_t *psp;

  // clip to screen bounds
  mfloorclip = screenheightarray;
  mceilingclip = negonearray;

  // add all active psprites
  for (i=0, psp=viewplayer->psprites; i<NUMPSPRITES; i++,psp++)
    if (psp->state)
      R_DrawPSprite (psp, lightlevel);
}

//
// R_SortVisSprites
//
// Rewritten by Lee Killough to avoid using unnecessary
// linked lists, and to use faster sorting algorithm.
//

#ifdef DJGPP

// killough 9/22/98: inlined memcpy of pointer arrays
// CPhipps - added memory as modified
#define bcopyp(d, s, n) asm(" cld; rep; movsl;" :: "D"(d), "S"(s), "c"(n) : "%cc", "%esi", "%edi", "%ecx", "memory")

#else

#define bcopyp(d, s, n) memcpy(d, s, (n) * sizeof(void *))

#endif

// killough 9/2/98: merge sort

static void msort(vissprite_t **s, vissprite_t **t, int n)
{
  if (n >= 16)
    {
      int n1 = n/2, n2 = n - n1;
      vissprite_t **s1 = s, **s2 = s + n1, **d = t;

      msort(s1, t, n1);
      msort(s2, t, n2);

      while ((*s1)->scale > (*s2)->scale ?
             (*d++ = *s1++, --n1) : (*d++ = *s2++, --n2));

      if (n2)
        bcopyp(d, s2, n2);
      else
        bcopyp(d, s1, n1);

      bcopyp(s, t, n);
    }
  else
    {
      int i;
      for (i = 1; i < n; i++)
        {
          vissprite_t *temp = s[i];
          if (s[i-1]->scale < temp->scale)
            {
              int j = i;
              while ((s[j] = s[j-1])->scale < temp->scale && --j);
              s[j] = temp;
            }
        }
    }
}

void R_SortVisSprites (void)
{
  if (num_vissprite)
    {
      int i = num_vissprite;

      // If we need to allocate more pointers for the vissprites,
      // allocate as many as were allocated for sprites -- killough
      // killough 9/22/98: allocate twice as many

      if (num_vissprite_ptrs < num_vissprite*2)
        {
          free(vissprite_ptrs);  // better than realloc -- no preserving needed
          vissprite_ptrs = malloc((num_vissprite_ptrs = num_vissprite_alloc*2)
                                  * sizeof *vissprite_ptrs);
        }

      while (--i>=0)
        vissprite_ptrs[i] = vissprites+i;

      // killough 9/22/98: replace qsort with merge sort, since the keys
      // are roughly in order to begin with, due to BSP rendering.

      msort(vissprite_ptrs, vissprite_ptrs + num_vissprite, num_vissprite);
    }
}

//
// R_DrawSprite
//

static void R_DrawSprite (vissprite_t* spr)
{
  drawseg_t *ds;
  int     clipbot[MAX_SCREENWIDTH]; // killough 2/8/98: // dropoff overflow
  int     cliptop[MAX_SCREENWIDTH]; // change to MAX_*  // dropoff overflow
  int     x;
  int     r1;
  int     r2;
  fixed_t scale;
  fixed_t lowscale;

  for (x = spr->x1 ; x<=spr->x2 ; x++)
    clipbot[x] = cliptop[x] = -2;

  // Scan drawsegs from end to start for obscuring segs.
  // The first drawseg that has a greater scale is the clip seg.

  // Modified by Lee Killough:
  // (pointer check was originally nonportable
  // and buggy, by going past LEFT end of array):

  //    for (ds=ds_p-1 ; ds >= drawsegs ; ds--)    old buggy code

  for (ds=ds_p ; ds-- > drawsegs ; )  // new -- killough
    {      // determine if the drawseg obscures the sprite
      if (ds->x1 > spr->x2 || ds->x2 < spr->x1 ||
          (!ds->silhouette && !ds->maskedtexturecol))
        continue;      // does not cover sprite

      r1 = ds->x1 < spr->x1 ? spr->x1 : ds->x1;
      r2 = ds->x2 > spr->x2 ? spr->x2 : ds->x2;

      if (ds->scale1 > ds->scale2)
        {
          lowscale = ds->scale2;
          scale = ds->scale1;
        }
      else
        {
          lowscale = ds->scale1;
          scale = ds->scale2;
        }

      if (scale < spr->scale || (lowscale < spr->scale &&
                    !R_PointOnSegSide (spr->gx, spr->gy, ds->curline)))
        {
          if (ds->maskedtexturecol)       // masked mid texture?
            R_RenderMaskedSegRange(ds, r1, r2);
          continue;               // seg is behind sprite
        }

      // clip this piece of the sprite
      // killough 3/27/98: optimized and made much shorter

      if (ds->silhouette&SIL_BOTTOM && spr->gz < ds->bsilheight) //bottom sil
        for (x=r1 ; x<=r2 ; x++)
          if (clipbot[x] == -2)
            clipbot[x] = ds->sprbottomclip[x];

      if (ds->silhouette&SIL_TOP && spr->gzt > ds->tsilheight)   // top sil
        for (x=r1 ; x<=r2 ; x++)
          if (cliptop[x] == -2)
            cliptop[x] = ds->sprtopclip[x];
    }

  // killough 3/27/98:
  // Clip the sprite against deep water and/or fake ceilings.
  // killough 4/9/98: optimize by adding mh
  // killough 4/11/98: improve sprite clipping for underwater/fake ceilings
  // killough 11/98: fix disappearing sprites

  if (spr->heightsec != -1)  // only things in specially marked sectors
    {
      fixed_t h,mh;
      int phs = viewplayer->mo->subsector->sector->heightsec;
      if ((mh = sectors[spr->heightsec].floorheight) > spr->gz &&
          (h = centeryfrac - FixedMul(mh-=viewz, spr->scale)) >= 0 &&
          (h >>= FRACBITS) < viewheight) {
        if (mh <= 0 || (phs != -1 && viewz > sectors[phs].floorheight))
          {                          // clip bottom
            for (x=spr->x1 ; x<=spr->x2 ; x++)
              if (clipbot[x] == -2 || h < clipbot[x])
                clipbot[x] = h;
          }
        else                        // clip top
    if (phs != -1 && viewz <= sectors[phs].floorheight) // killough 11/98
      for (x=spr->x1 ; x<=spr->x2 ; x++)
        if (cliptop[x] == -2 || h > cliptop[x])
    cliptop[x] = h;
      }

      if ((mh = sectors[spr->heightsec].ceilingheight) < spr->gzt &&
          (h = centeryfrac - FixedMul(mh-viewz, spr->scale)) >= 0 &&
          (h >>= FRACBITS) < viewheight) {
        if (phs != -1 && viewz >= sectors[phs].ceilingheight)
          {                         // clip bottom
            for (x=spr->x1 ; x<=spr->x2 ; x++)
              if (clipbot[x] == -2 || h < clipbot[x])
                clipbot[x] = h;
          }
        else                       // clip top
          for (x=spr->x1 ; x<=spr->x2 ; x++)
            if (cliptop[x] == -2 || h > cliptop[x])
              cliptop[x] = h;
      }
    }
  // killough 3/27/98: end special clipping for deep water / fake ceilings

  // all clipping has been performed, so draw the sprite
  // check for unclipped columns

  for (x = spr->x1 ; x<=spr->x2 ; x++) {
    if (clipbot[x] == -2)
      clipbot[x] = viewheight;

    if (cliptop[x] == -2)
      cliptop[x] = -1;
  }

  mfloorclip = clipbot;
  mceilingclip = cliptop;
  R_DrawVisSprite (spr, spr->x1, spr->x2);
}

//
// R_DrawMasked
//

void R_DrawMasked(void)
{
  int i;
  drawseg_t *ds;

  R_SortVisSprites();

  // draw all vissprites back to front

  rendered_vissprites = num_vissprite;
  for (i = num_vissprite ;--i>=0; )
    R_DrawSprite(vissprite_ptrs[i]);         // killough

  // render any remaining masked mid textures

  // Modified by Lee Killough:
  // (pointer check was originally nonportable
  // and buggy, by going past LEFT end of array):

  //    for (ds=ds_p-1 ; ds >= drawsegs ; ds--)    old buggy code

  for (ds=ds_p ; ds-- > drawsegs ; )  // new -- killough
    if (ds->maskedtexturecol)
      R_RenderMaskedSegRange(ds, ds->x1, ds->x2);

  // draw the psprites on top of everything
  //  but does not draw on side views
  if (!viewangleoffset)
    R_DrawPlayerSprites ();
}
