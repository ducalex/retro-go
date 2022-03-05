/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2002 by
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
 *-----------------------------------------------------------------------------*/

#include "z_zone.h"
#include "doomstat.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_sky.h"
#include "r_bsp.h"
#include "r_things.h"
#include "p_tick.h"
#include "i_system.h"
#include "r_draw.h"
#include "lprintf.h"
#include "r_patch.h"
#include <assert.h>

// posts are runs of non masked source pixels
typedef struct
{
  byte topdelta; // -1 is the last post in a column
  byte length;   // length data bytes follows
} post_t;

// column_t is a list of 0 or more post_t, (byte)-1 terminated
typedef post_t column_t;

//
// Patches.
// A patch holds one or more columns.
// Patches are used for sprites and all masked pictures,
// and we compose textures from the TEXTURE1/2 lists
// of patches.
//

typedef struct
{
  short width, height;  // bounding box size
  short leftoffset;     // pixels to the left of origin
  short topoffset;      // pixels below the origin
  int columnofs[8];     // only [width] used
} patch_t;

//---------------------------------------------------------------------------
// Re-engineered patch support
//---------------------------------------------------------------------------
static rpatch_t *patches = 0;
static rpatch_t *texture_composites = 0;

//---------------------------------------------------------------------------
void R_InitPatches(void)
{
  patches = Z_Calloc(numlumps, sizeof(*patches), PU_STATIC, 0);
  texture_composites = Z_Calloc(numtextures, sizeof(*patches), PU_STATIC, 0);
}

//---------------------------------------------------------------------------
int R_NumPatchWidth(int lump)
{
  const rpatch_t *patch = R_CachePatchNum(lump);
  int width = patch->width;
  R_UnlockPatchNum(lump);
  return width;
}

//---------------------------------------------------------------------------
int R_NumPatchHeight(int lump)
{
  const rpatch_t *patch = R_CachePatchNum(lump);
  int height = patch->height;
  R_UnlockPatchNum(lump);
  return height;
}

//---------------------------------------------------------------------------
static int getPatchIsNotTileable(const patch_t *patch) {
  int x=0, numPosts, lastColumnDelta = 0;
  const column_t *column;
  int cornerCount = 0;
  int hasAHole = 0;

  for (x=0; x<SHORT(patch->width); x++) {
    column = (const column_t *)((const byte *)patch + LONG(patch->columnofs[x]));
    if (!x) lastColumnDelta = column->topdelta;
    else if (lastColumnDelta != column->topdelta) hasAHole = 1;

    numPosts = 0;
    while (column->topdelta != 0xff) {
      // check to see if a corner pixel filled
      if (x == 0 && column->topdelta == 0) cornerCount++;
      else if (x == 0 && column->topdelta + column->length >= SHORT(patch->height)) cornerCount++;
      else if (x == SHORT(patch->width)-1 && column->topdelta == 0) cornerCount++;
      else if (x == SHORT(patch->width)-1 && column->topdelta + column->length >= SHORT(patch->height)) cornerCount++;

      if (numPosts++) hasAHole = 1;
      column = (const column_t *)((const byte *)column + column->length + 4);
    }
  }

  if (cornerCount == 4) return 0;
  return hasAHole;
}

//---------------------------------------------------------------------------
static int getIsSolidAtSpot(const column_t *column, int spot) {
  if (!column) return 0;
  while (column->topdelta != 0xff) {
    if (spot < column->topdelta) return 0;
    if ((spot >= column->topdelta) && (spot <= column->topdelta + column->length)) return 1;
    column = (const column_t*)((const byte*)column + 3 + column->length + 1);
  }
  return 0;
}

//---------------------------------------------------------------------------
// Used to determine whether a column edge (top or bottom) should slope
// up or down for smoothed masked edges - POPE
//---------------------------------------------------------------------------
static int getColumnEdgeSlope(const column_t *prevcolumn, const column_t *nextcolumn, int spot) {
  int holeToLeft = !getIsSolidAtSpot(prevcolumn, spot);
  int holeToRight = !getIsSolidAtSpot(nextcolumn, spot);

  if (holeToLeft && !holeToRight) return 1;
  if (!holeToLeft && holeToRight) return -1;
  return 0;
}

//---------------------------------------------------------------------------
static void createPatch(int id) {
  rpatch_t *patch;
  const int patchNum = id;
  const patch_t *oldPatch = (const patch_t*)W_CacheLumpNum(patchNum);
  const column_t *oldColumn, *oldPrevColumn, *oldNextColumn;
  int x, y;
  int pixelDataSize;
  int columnsDataSize;
  int postsDataSize;
  int dataSize;
  const byte *oldColumnPixelData;
  int numPostsUsedSoFar;
  int edgeSlope;

#ifdef RANGECHECK
  if (id >= numlumps)
    I_Error("createPatch: %i >= numlumps", id);
#endif

  patch = &patches[id];
  // proff - 2003-02-16 What about endianess?
  patch->width = SHORT(oldPatch->width);
  patch->widthmask = 0;
  patch->height = SHORT(oldPatch->height);
  patch->leftoffset = SHORT(oldPatch->leftoffset);
  patch->topoffset = SHORT(oldPatch->topoffset);
  patch->isNotTileable = getPatchIsNotTileable(oldPatch);

  // work out how much memory we need to allocate for this patch's data
  pixelDataSize = (patch->width * patch->height + 4) & ~3;
  columnsDataSize = sizeof(rcolumn_t) * patch->width;

  // count the number of posts in each column
  int numPostsInColumn[patch->width];
  int numPostsTotal = 0;

  for (x=0; x<patch->width; x++) {
    oldColumn = (const column_t *)((const byte *)oldPatch + LONG(oldPatch->columnofs[x]));
    numPostsInColumn[x] = 0;
    while (oldColumn->topdelta != 0xff) {
      numPostsInColumn[x]++;
      numPostsTotal++;
      oldColumn = (const column_t *)((const byte *)oldColumn + oldColumn->length + 4);
    }
  }

  postsDataSize = numPostsTotal * sizeof(rpost_t);

  // allocate our data chunk
  dataSize = pixelDataSize + columnsDataSize + postsDataSize;
  patch->data = Z_Malloc(dataSize, PU_CACHE, (void **)&patch->data);

  // set out pixel, column, and post pointers into our data array
  patch->pixels = patch->data;
  patch->columns = (rcolumn_t*)((unsigned char*)patch->pixels + pixelDataSize);
  patch->posts = (rpost_t*)((unsigned char*)patch->columns + columnsDataSize);

  memset(patch->pixels, 0xff, pixelDataSize);
  memset(patch->columns, 0x00, columnsDataSize);
  memset(patch->posts, 0x00, postsDataSize);

  // fill in the pixels, posts, and columns
  numPostsUsedSoFar = 0;
  for (x=0; x<patch->width; x++) {

    oldColumn = (const column_t *)((const byte *)oldPatch + LONG(oldPatch->columnofs[x]));

    if (patch->isNotTileable) {
      // non-tiling
      if (x == 0) oldPrevColumn = 0;
      else oldPrevColumn = (const column_t *)((const byte *)oldPatch + LONG(oldPatch->columnofs[x-1]));
      if (x == patch->width-1) oldNextColumn = 0;
      else oldNextColumn = (const column_t *)((const byte *)oldPatch + LONG(oldPatch->columnofs[x+1]));
    }
    else {
      // tiling
      int prevColumnIndex = x-1;
      int nextColumnIndex = x+1;
      while (prevColumnIndex < 0) prevColumnIndex += patch->width;
      while (nextColumnIndex >= patch->width) nextColumnIndex -= patch->width;
      oldPrevColumn = (const column_t *)((const byte *)oldPatch + LONG(oldPatch->columnofs[prevColumnIndex]));
      oldNextColumn = (const column_t *)((const byte *)oldPatch + LONG(oldPatch->columnofs[nextColumnIndex]));
    }

    // setup the column's data
    patch->columns[x].pixels = patch->pixels + (x*patch->height) + 0;
    patch->columns[x].numPosts = numPostsInColumn[x];
    patch->columns[x].posts = patch->posts + numPostsUsedSoFar;

    while (oldColumn->topdelta != 0xff) {
      // set up the post's data
      patch->posts[numPostsUsedSoFar].topdelta = oldColumn->topdelta;
      patch->posts[numPostsUsedSoFar].length = oldColumn->length;
      patch->posts[numPostsUsedSoFar].slope = 0;

      edgeSlope = getColumnEdgeSlope(oldPrevColumn, oldNextColumn, oldColumn->topdelta);
      if (edgeSlope == 1) patch->posts[numPostsUsedSoFar].slope |= RDRAW_EDGESLOPE_TOP_UP;
      else if (edgeSlope == -1) patch->posts[numPostsUsedSoFar].slope |= RDRAW_EDGESLOPE_TOP_DOWN;

      edgeSlope = getColumnEdgeSlope(oldPrevColumn, oldNextColumn, oldColumn->topdelta+oldColumn->length);
      if (edgeSlope == 1) patch->posts[numPostsUsedSoFar].slope |= RDRAW_EDGESLOPE_BOT_UP;
      else if (edgeSlope == -1) patch->posts[numPostsUsedSoFar].slope |= RDRAW_EDGESLOPE_BOT_DOWN;

      // fill in the post's pixels
      oldColumnPixelData = (const byte *)oldColumn + 3;
      for (y=0; y<oldColumn->length; y++) {
        patch->pixels[x * patch->height + oldColumn->topdelta + y] = oldColumnPixelData[y];
      }

      oldColumn = (const column_t *)((const byte *)oldColumn + oldColumn->length + 4);
      numPostsUsedSoFar++;
    }
  }

  if (1 || patch->isNotTileable) {
    const rcolumn_t *column, *prevColumn;

    // copy the patch image down and to the right where there are
    // holes to eliminate the black halo from bilinear filtering
    for (x=0; x<patch->width; x++) {
      //oldColumn = (const column_t *)((const byte *)oldPatch + oldPatch->columnofs[x]);

      column = R_GetPatchColumnClamped(patch, x);
      prevColumn = R_GetPatchColumnClamped(patch, x-1);

      if (column->pixels[0] == 0xff) {
        // force the first pixel (which is a hole), to use
        // the color from the next solid spot in the column
        for (y=0; y<patch->height; y++) {
          if (column->pixels[y] != 0xff) {
            column->pixels[0] = column->pixels[y];
            break;
          }
        }
      }

      // copy from above or to the left
      for (y=1; y<patch->height; y++) {
        //if (getIsSolidAtSpot(oldColumn, y)) continue;
        if (column->pixels[y] != 0xff) continue;

        // this pixel is a hole

        if (x && prevColumn->pixels[y-1] != 0xff) {
          // copy the color from the left
          column->pixels[y] = prevColumn->pixels[y];
        }
        else {
          // copy the color from above
          column->pixels[y] = column->pixels[y-1];
        }
      }
    }

    // verify that the patch truly is non-rectangular since
    // this determines tiling later on
  }

  W_UnlockLumpNum(patchNum);
}

typedef struct {
  unsigned short patches;
  unsigned short posts;
  unsigned short posts_used;
} count_t;

static void switchPosts(rpost_t *post1, rpost_t *post2) {
  rpost_t dummy;

  dummy.topdelta = post1->topdelta;
  dummy.length = post1->length;
  dummy.slope = post1->slope;
  post1->topdelta = post2->topdelta;
  post1->length = post2->length;
  post1->slope = post2->slope;
  post2->topdelta = dummy.topdelta;
  post2->length = dummy.length;
  post2->slope = dummy.slope;
}

static void removePostFromColumn(rcolumn_t *column, int post) {
  int i;
#ifdef RANGECHECK
  if (post >= column->numPosts)
    I_Error("removePostFromColumn: invalid post index");
#endif
  if (post < column->numPosts)
    for (i=post; i<(column->numPosts-1); i++) {
      rpost_t *post1 = &column->posts[i];
      rpost_t *post2 = &column->posts[i+1];
      post1->topdelta = post2->topdelta;
      post1->length = post2->length;
      post1->slope = post2->slope;
    }
  column->numPosts--;
}

//---------------------------------------------------------------------------
static void createTextureCompositePatch(int id) {
  rpatch_t *composite_patch;
  texture_t *texture;
  texpatch_t *texpatch;
  int patchNum;
  const patch_t *oldPatch;
  const column_t *oldColumn, *oldPrevColumn, *oldNextColumn;
  int i, x, y;
  int oy, count;
  int pixelDataSize;
  int columnsDataSize;
  int postsDataSize;
  int dataSize;
  const byte *oldColumnPixelData;
  int numPostsUsedSoFar;
  int edgeSlope;

#ifdef RANGECHECK
  if (id >= numtextures)
    I_Error("createTextureCompositePatch: %i >= numtextures", id);
#endif

  composite_patch = &texture_composites[id];

  texture = textures[id];

  composite_patch->width = texture->width;
  composite_patch->height = texture->height;
  composite_patch->widthmask = texture->widthmask;
  composite_patch->leftoffset = 0;
  composite_patch->topoffset = 0;
  composite_patch->isNotTileable = 0;

  // work out how much memory we need to allocate for this patch's data
  pixelDataSize = (composite_patch->width * composite_patch->height + 4) & ~3;
  columnsDataSize = sizeof(rcolumn_t) * composite_patch->width;

  // count the number of posts in each column
  count_t countsInColumn[composite_patch->width];
  size_t numPostsTotal = 0;

  memset(countsInColumn, 0, sizeof(countsInColumn));

  for (i=0; i<texture->patchcount; i++) {
    texpatch = &texture->patches[i];
    patchNum = texpatch->patch;
    oldPatch = (const patch_t*)W_CacheLumpNum(patchNum);

    for (x=0; x<SHORT(oldPatch->width); x++) {
      int tx = texpatch->originx + x;

      if (tx < 0)
        continue;
      if (tx >= composite_patch->width)
        break;

      countsInColumn[tx].patches++;

      oldColumn = (const column_t *)((const byte *)oldPatch + LONG(oldPatch->columnofs[x]));
      while (oldColumn->topdelta != 0xff) {
        countsInColumn[tx].posts++;
        numPostsTotal++;
        oldColumn = (const column_t *)((const byte *)oldColumn + oldColumn->length + 4);
      }
    }

    W_UnlockLumpNum(patchNum);
  }

  postsDataSize = numPostsTotal * sizeof(rpost_t);

  // allocate our data chunk
  dataSize = pixelDataSize + columnsDataSize + postsDataSize;
  composite_patch->data = Z_Malloc(dataSize, PU_STATIC, (void **)&composite_patch->data);

  // set out pixel, column, and post pointers into our data array
  composite_patch->pixels = composite_patch->data;
  composite_patch->columns = (rcolumn_t*)((unsigned char*)composite_patch->pixels + pixelDataSize);
  composite_patch->posts = (rpost_t*)((unsigned char*)composite_patch->columns + columnsDataSize);

  memset(composite_patch->pixels, 0xff, pixelDataSize);
  memset(composite_patch->columns, 0x00, columnsDataSize);
  memset(composite_patch->posts, 0x00, postsDataSize);

  numPostsUsedSoFar = 0;

  for (x=0; x<texture->width; x++) {
      // setup the column's data
      composite_patch->columns[x].pixels = composite_patch->pixels + (x*composite_patch->height);
      composite_patch->columns[x].numPosts = countsInColumn[x].posts;
      composite_patch->columns[x].posts = composite_patch->posts + numPostsUsedSoFar;
      numPostsUsedSoFar += countsInColumn[x].posts;
  }

  // fill in the pixels, posts, and columns
  for (i=0; i<texture->patchcount; i++) {
    texpatch = &texture->patches[i];
    patchNum = texpatch->patch;
    oldPatch = (const patch_t*)W_CacheLumpNum(patchNum);

    for (x=0; x<SHORT(oldPatch->width); x++) {
      int tx = texpatch->originx + x;

      if (tx < 0)
        continue;
      if (tx >= composite_patch->width)
        break;

      oldColumn = (const column_t *)((const byte *)oldPatch + LONG(oldPatch->columnofs[x]));

      {
        // tiling
        int prevColumnIndex = x-1;
        int nextColumnIndex = x+1;
        while (prevColumnIndex < 0) prevColumnIndex += SHORT(oldPatch->width);
        while (nextColumnIndex >= SHORT(oldPatch->width)) nextColumnIndex -= SHORT(oldPatch->width);
        oldPrevColumn = (const column_t *)((const byte *)oldPatch + LONG(oldPatch->columnofs[prevColumnIndex]));
        oldNextColumn = (const column_t *)((const byte *)oldPatch + LONG(oldPatch->columnofs[nextColumnIndex]));
      }

      while (oldColumn->topdelta != 0xff) {
        rpost_t *post = &composite_patch->columns[tx].posts[countsInColumn[tx].posts_used];
        oldColumnPixelData = (const byte *)oldColumn + 3;
        oy = texpatch->originy;
        count = oldColumn->length;
        // the original renderer had several bugs which we reproduce here
        if (countsInColumn[tx].patches > 1) {
          // when there are multiple patches, then we need to handle the
          // column differently
          if (i == 0) {
            // draw first patch at original position, it will be partly
            // overdrawn below
            for (y=0; y<count; y++) {
              int ty = oy + oldColumn->topdelta + y;
              if (ty < 0)
                continue;
              if (ty >= composite_patch->height)
                break;
              composite_patch->pixels[tx * composite_patch->height + ty] = oldColumnPixelData[y];
            }
          }
          // do the buggy clipping
          if ((oy + oldColumn->topdelta) < 0) {
            count += oy;
            oy = 0;
          }
        } else {
          // with a single patch only negative y origins are wrong
          oy = 0;
        }
        // set up the post's data
        post->topdelta = oldColumn->topdelta + oy;
        post->length = count;
        if ((post->topdelta + post->length) > composite_patch->height) {
          if (post->topdelta > composite_patch->height)
            post->length = 0;
          else
            post->length = composite_patch->height - post->topdelta;
        }
        if (post->topdelta < 0) {
          if ((post->topdelta + post->length) <= 0)
            post->length = 0;
          else
            post->length -= post->topdelta;
          post->topdelta = 0;
        }
        post->slope = 0;

        edgeSlope = getColumnEdgeSlope(oldPrevColumn, oldNextColumn, oldColumn->topdelta);
        if (edgeSlope == 1) post->slope |= RDRAW_EDGESLOPE_TOP_UP;
        else if (edgeSlope == -1) post->slope |= RDRAW_EDGESLOPE_TOP_DOWN;

        edgeSlope = getColumnEdgeSlope(oldPrevColumn, oldNextColumn, oldColumn->topdelta+count);
        if (edgeSlope == 1) post->slope |= RDRAW_EDGESLOPE_BOT_UP;
        else if (edgeSlope == -1) post->slope |= RDRAW_EDGESLOPE_BOT_DOWN;

        // fill in the post's pixels
        for (y=0; y<count; y++) {
          int ty = oy + oldColumn->topdelta + y;
          if (ty < 0)
            continue;
          if (ty >= composite_patch->height)
            break;
          composite_patch->pixels[tx * composite_patch->height + ty] = oldColumnPixelData[y];
        }

        oldColumn = (const column_t *)((const byte *)oldColumn + oldColumn->length + 4);
        countsInColumn[tx].posts_used++;
        assert(countsInColumn[tx].posts_used <= countsInColumn[tx].posts);
      }
    }

    W_UnlockLumpNum(patchNum);
  }

  for (x=0; x<texture->width; x++) {
    rcolumn_t *column;

    if (countsInColumn[x].patches <= 1)
      continue;

    // cleanup posts on multipatch columns
    column = &composite_patch->columns[x];

    i = 0;
    while (i<(column->numPosts-1)) {
      rpost_t *post1 = &column->posts[i];
      rpost_t *post2 = &column->posts[i+1];
      int length;

      if ((post2->topdelta - post1->topdelta) < 0)
        switchPosts(post1, post2);

      if ((post1->topdelta + post1->length) >= post2->topdelta) {
        length = (post1->length + post2->length) - ((post1->topdelta + post1->length) - post2->topdelta);
        if (post1->length < length) {
          post1->slope = post2->slope;
          post1->length = length;
        }
        removePostFromColumn(column, i+1);
        i = 0;
        continue;
      }
      i++;
    }
  }

  if (1 || composite_patch->isNotTileable) {
    const rcolumn_t *column, *prevColumn;

    // copy the patch image down and to the right where there are
    // holes to eliminate the black halo from bilinear filtering
    for (x=0; x<composite_patch->width; x++) {
      //oldColumn = (const column_t *)((const byte *)oldPatch + oldPatch->columnofs[x]);

      column = R_GetPatchColumnClamped(composite_patch, x);
      prevColumn = R_GetPatchColumnClamped(composite_patch, x-1);

      if (column->pixels[0] == 0xff) {
        // force the first pixel (which is a hole), to use
        // the color from the next solid spot in the column
        for (y=0; y<composite_patch->height; y++) {
          if (column->pixels[y] != 0xff) {
            column->pixels[0] = column->pixels[y];
            break;
          }
        }
      }

      // copy from above or to the left
      for (y=1; y<composite_patch->height; y++) {
        //if (getIsSolidAtSpot(oldColumn, y)) continue;
        if (column->pixels[y] != 0xff) continue;

        // this pixel is a hole

        if (x && prevColumn->pixels[y-1] != 0xff) {
          // copy the color from the left
          column->pixels[y] = prevColumn->pixels[y];
        }
        else {
          // copy the color from above
          column->pixels[y] = column->pixels[y-1];
        }
      }
    }

    // verify that the patch truly is non-rectangular since
    // this determines tiling later on
  }
}

//---------------------------------------------------------------------------
const rpatch_t *R_CachePatchNum(int id) {
  const int locks = 1;

  if (!patches)
    I_Error("R_CachePatchNum: Patches not initialized");

#ifdef RANGECHECK
  if (id >= numlumps)
    I_Error("createPatch: %i >= numlumps", id);
#endif

  if (!patches[id].data)
    createPatch(id);

  /* cph - if wasn't locked but now is, tell z_zone to hold it */
  if (!patches[id].locks && locks) {
    Z_ChangeTag(patches[id].data,PU_STATIC);
#ifdef TIMEDIAG
    patches[id].locktic = gametic;
#endif
  }
  patches[id].locks += locks;

#ifdef SIMPLECHECKS
  if (!((patches[id].locks+1) & 0xf))
    lprintf(LO_DEBUG, "R_CachePatchNum: High lock on %8s (%d)\n",
	    lumpinfo[id].name, patches[id].locks);
#endif

  return &patches[id];
}

void R_UnlockPatchNum(int id)
{
  const int unlocks = 1;
#ifdef SIMPLECHECKS
  if ((signed short)patches[id].locks < unlocks)
    lprintf(LO_DEBUG, "R_UnlockPatchNum: Excess unlocks on %8s (%d-%d)\n",
	    lumpinfo[id].name, patches[id].locks, unlocks);
#endif
  patches[id].locks -= unlocks;
  /* cph - Note: must only tell z_zone to make purgeable if currently locked,
   * else it might already have been purged
   */
  if (unlocks && !patches[id].locks)
    Z_ChangeTag(patches[id].data, PU_CACHE);
}

//---------------------------------------------------------------------------
const rpatch_t *R_CacheTextureCompositePatchNum(int id) {
  const int locks = 1;

  if (!texture_composites)
    I_Error("R_CacheTextureCompositePatchNum: Composite patches not initialized");

#ifdef RANGECHECK
  if (id >= numtextures)
    I_Error("createTextureCompositePatch: %i >= numtextures", id);
#endif

  if (!texture_composites[id].data)
    createTextureCompositePatch(id);

  /* cph - if wasn't locked but now is, tell z_zone to hold it */
  if (!texture_composites[id].locks && locks) {
    Z_ChangeTag(texture_composites[id].data,PU_STATIC);
#ifdef TIMEDIAG
    texture_composites[id].locktic = gametic;
#endif
  }
  texture_composites[id].locks += locks;

#ifdef SIMPLECHECKS
  if (!((texture_composites[id].locks+1) & 0xf))
    lprintf(LO_DEBUG, "R_CacheTextureCompositePatchNum: High lock on %8s (%d)\n",
	    textures[id]->name, texture_composites[id].locks);
#endif

  return &texture_composites[id];

}

void R_UnlockTextureCompositePatchNum(int id)
{
  const int unlocks = 1;
#ifdef SIMPLECHECKS
  if ((signed short)texture_composites[id].locks < unlocks)
    lprintf(LO_DEBUG, "R_UnlockTextureCompositePatchNum: Excess unlocks on %8s (%d-%d)\n",
	    textures[id]->name, texture_composites[id].locks, unlocks);
#endif
  texture_composites[id].locks -= unlocks;
  /* cph - Note: must only tell z_zone to make purgeable if currently locked,
   * else it might already have been purged
   */
  if (unlocks && !texture_composites[id].locks)
    Z_ChangeTag(texture_composites[id].data, PU_CACHE);
}

//---------------------------------------------------------------------------
const rcolumn_t *R_GetPatchColumnWrapped(const rpatch_t *patch, int columnIndex) {
  while (columnIndex < 0) columnIndex += patch->width;
  columnIndex %= patch->width;
  return &patch->columns[columnIndex];
}

//---------------------------------------------------------------------------
const rcolumn_t *R_GetPatchColumnClamped(const rpatch_t *patch, int columnIndex) {
  if (columnIndex < 0) columnIndex = 0;
  if (columnIndex >= patch->width) columnIndex = patch->width-1;
  return &patch->columns[columnIndex];
}

//---------------------------------------------------------------------------
const rcolumn_t *R_GetPatchColumn(const rpatch_t *patch, int columnIndex) {
  if (patch->isNotTileable) return R_GetPatchColumnClamped(patch, columnIndex);
  else return R_GetPatchColumnWrapped(patch, columnIndex);
}

