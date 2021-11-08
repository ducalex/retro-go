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
 *  Functions to return random numbers.
 *
 *-----------------------------------------------------------------------------*/


#ifndef __M_RANDOM__
#define __M_RANDOM__

#include "doomtype.h"

// killough 1/19/98: rewritten to use to use a better random number generator
// in the new engine, although the old one is available for compatibility.

// killough 2/16/98:
//
// Make every random number generator local to each control-equivalent block.
// Critical for demo sync. Changing the order of this list breaks all previous
// versions' demos. The random number generators are made local to reduce the
// chances of sync problems. In Doom, if a single random number generator call
// was off, it would mess up all random number generators. This reduces the
// chances of it happening by making each RNG local to a control flow block.
//
// Notes to developers: if you want to reduce your demo sync hassles, follow
// this rule: for each call to P_Random you add, add a new class to the enum
// type below for each block of code which calls P_Random. If two calls to
// P_Random are not in "control-equivalent blocks", i.e. there are any cases
// where one is executed, and the other is not, put them in separate classes.
//
// Keep all current entries in this list the same, and in the order
// indicated by the #'s, because they're critical for preserving demo
// sync. Do not remove entries simply because they become unused later.

typedef enum {
  pr_skullfly,                // #1
  pr_damage,                  // #2
  pr_crush,                   // #3
  pr_genlift,                 // #4
  pr_killtics,                // #5
  pr_damagemobj,              // #6
  pr_painchance,              // #7
  pr_lights,                  // #8
  pr_explode,                 // #9
  pr_respawn,                 // #10
  pr_lastlook,                // #11
  pr_spawnthing,              // #12
  pr_spawnpuff,               // #13
  pr_spawnblood,              // #14
  pr_missile,                 // #15
  pr_shadow,                  // #16
  pr_plats,                   // #17
  pr_punch,                   // #18
  pr_punchangle,              // #19
  pr_saw,                     // #20
  pr_plasma,                  // #21
  pr_gunshot,                 // #22
  pr_misfire,                 // #23
  pr_shotgun,                 // #24
  pr_bfg,                     // #25
  pr_slimehurt,               // #26
  pr_dmspawn,                 // #27
  pr_missrange,               // #28
  pr_trywalk,                 // #29
  pr_newchase,                // #30
  pr_newchasedir,             // #31
  pr_see,                     // #32
  pr_facetarget,              // #33
  pr_posattack,               // #34
  pr_sposattack,              // #35
  pr_cposattack,              // #36
  pr_spidrefire,              // #37
  pr_troopattack,             // #38
  pr_sargattack,              // #39
  pr_headattack,              // #40
  pr_bruisattack,             // #41
  pr_tracer,                  // #42
  pr_skelfist,                // #43
  pr_scream,                  // #44
  pr_brainscream,             // #45
  pr_cposrefire,              // #46
  pr_brainexp,                // #47
  pr_spawnfly,                // #48
  pr_misc,                    // #49
  pr_all_in_one,              // #50
  /* CPhipps - new entries from MBF, mostly unused for now */
  pr_opendoor,                // #51
  pr_targetsearch,            // #52
  pr_friends,                 // #53
  pr_threshold,               // #54
  pr_skiptarget,              // #55
  pr_enemystrafe,             // #56
  pr_avoidcrush,              // #57
  pr_stayonlift,              // #58
  pr_helpfriend,              // #59
  pr_dropoff,                 // #60
  pr_randomjump,              // #61
  pr_defect,                  // #62  // Start new entries -- add new entries below

  // End of new entries
  NUMPRCLASS               // MUST be last item in list
} pr_class_t;

// The random number generator's state.
typedef struct {
  unsigned long seed[NUMPRCLASS];      // Each block's random seed
  int rndindex, prndindex;             // For compatibility support
} rng_t;

extern rng_t rng;                      // The rng's state

extern unsigned long rngseed;          // The starting seed (not part of state)

// As M_Random, but used by the play simulation.
int P_Random(pr_class_t DA(const char *, int));

#ifdef INSTRUMENTED
#define P_Random(a) (P_Random) (a, __FILE__,__LINE__)
#endif

// Returns a number from 0 to 255,
#define M_Random() P_Random(pr_misc)

// Fix randoms for demos.
void M_ClearRandom(void);

#endif
