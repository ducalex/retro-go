/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2004 by
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
 *  DOOM main program (D_DoomMain) and game loop (D_DoomLoop),
 *  plus functions to determine game mode (shareware, registered),
 *  parse command line parameters, configure game parameters (turbo),
 *  and call the startup functions.
 *
 *-----------------------------------------------------------------------------
 */


#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "doomdef.h"
#include "doomtype.h"
#include "doomstat.h"
#include "d_net.h"
#include "dstrings.h"
#include "sounds.h"
#include "z_zone.h"
#include "w_wad.h"
#include "s_sound.h"
#include "v_video.h"
#include "f_finale.h"
#include "f_wipe.h"
#include "m_argv.h"
#include "m_misc.h"
#include "m_menu.h"
#include "i_main.h"
#include "i_system.h"
#include "i_sound.h"
#include "i_video.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "wi_stuff.h"
#include "st_stuff.h"
#include "am_map.h"
#include "p_setup.h"
#include "r_draw.h"
#include "r_main.h"
#include "r_fps.h"
#include "d_main.h"
#include "d_deh.h"  // Ty 04/08/98 - Externalizations
#include "lprintf.h"  // jff 08/03/98 - declaration of lprintf
#include "am_map.h"

boolean devparm;        // started game with -devparm

// jff 1/24/98 add new versions of these variables to remember command line
boolean clnomonsters;   // checkparm of -nomonsters
boolean clrespawnparm;  // checkparm of -respawn
boolean clfastparm;     // checkparm of -fast
// jff 1/24/98 end definition of command line version of play mode switches

boolean nomonsters;     // working -nomonsters
boolean respawnparm;    // working -respawn
boolean fastparm;       // working -fast

boolean singletics = false; // debug flag to cancel adaptiveness

//jff 1/22/98 parms for disabling music and sound
boolean nosfxparm;
boolean nomusicparm;

//jff 4/18/98
extern boolean inhelpscreens;

skill_t startskill;
int     startepisode;
int     startmap;
boolean autostart;
int ffmap;

boolean advancedemo;

const char *basesavegame;

static int  demosequence;         // killough 5/2/98: made static
static int  pagetic;
static const char *pagename; // CPhipps - const

// wipegamestate can be set to -1 to force a wipe on the next draw
gamestate_t    wipegamestate = GS_DEMOSCREEN;
extern boolean setsizeneeded;
extern int     showMessages;

//jff 4/19/98 list of standard IWAD names
const char *standard_iwads[]=
{
  "doom.wad",     // Ultimate DOOM
  "plutonia.wad", // Final DOOM - The Plutonia Experiment
  "tnt.wad",      // Final Doom - TNT: Evilution
  "doom2.wad",    // DOOM2
  "doom2f.wad",   // DOOM2 French
  "doom1.wad",    // DOOM Shareware
  "freedoom.wad", /* wart@kobold.org:  added freedoom for Fedora Extras */
  NULL,
};

/*
 * D_PostEvent - Event handling
 *
 * Called by I/O functions when an event is received.
 * Try event handlers for each code area in turn.
 * cph - in the true spirit of the Boom source, let the
 *  short ciruit operator madness begin!
 */

void D_PostEvent(event_t *ev)
{
  /* cph - suppress all input events at game start
   * FIXME: This is a lousy kludge */
  if (gametic < 3) return;

  if (M_Responder(ev))
    return;

  if (gamestate == GS_LEVEL) {
    if (HU_Responder(ev))
        return;
    if (ST_Responder(ev))
        return;
    if (AM_Responder(ev))
        return;
  }

	G_Responder(ev);
}

//
// D_Wipe
//
// CPhipps - moved the screen wipe code from D_Display to here
// The screens to wipe between are already stored, this just does the timing
// and screen updating

static void D_Wipe(void)
{
  boolean done;
  int wipestart = I_GetTime () - 1;

  do
    {
      int nowtime, tics;
      do
        {
          I_uSleep(5000); // CPhipps - don't thrash cpu in this loop
          nowtime = I_GetTime();
          tics = nowtime - wipestart;
        }
      while (!tics);
      wipestart = nowtime;
      done = wipe_ScreenWipe(tics);
      I_UpdateNoBlit();
      M_Drawer();                   // menu is drawn even on top of wipes
      I_FinishUpdate();             // page flip or blit buffer
    }
  while (!done);
}

//
// D_PageDrawer
//
static void D_PageDrawer(void)
{
  // proff/nicolas 09/14/98 -- now stretchs bitmaps to fullscreen!
  // CPhipps - updated for new patch drawing
  // proff - added M_DrawCredits
  if (pagename)
    V_DrawNamePatch(0, 0, 0, pagename, CR_DEFAULT, VPT_STRETCH);
  else
    M_DrawCredits();
}

//
// D_Display
//  draw current display, possibly wiping it from the previous
//

void D_Display (void)
{
  static boolean isborderstate        = false;
  static boolean borderwillneedredraw = false;
  static gamestate_t oldgamestate = -1;
  boolean wipe = gamestate != wipegamestate;
  boolean viewactive = false, isborder = false;

  if (nodrawers)                    // for comparative timing / profiling
    return;

  if (!I_StartDisplay())
    return;

  // save the current screen if about to wipe
  if (wipe)
    wipe_StartScreen();

  if (gamestate != GS_LEVEL) { // Not a level
    switch (oldgamestate) {
    case -1:
    case GS_LEVEL:
      V_SetPalette(0); // cph - use default (basic) palette
    default:
      break;
    }

    switch (gamestate) {
    case GS_INTERMISSION:
      WI_Drawer();
      break;
    case GS_FINALE:
      F_Drawer();
      break;
    case GS_DEMOSCREEN:
      D_PageDrawer();
      break;
    default:
      break;
    }
  } else if (gametic != basetic) { // In a level
    boolean redrawborderstuff;

    HU_Erase();

    if (setsizeneeded) {               // change the view size if needed
      R_ExecuteSetViewSize();
      oldgamestate = -1;            // force background redraw
    }

    // Work out if the player view is visible, and if there is a border
    viewactive = (!(automapmode & am_active) || (automapmode & am_overlay)) && !inhelpscreens;
    isborder = viewactive ? (viewheight != SCREENHEIGHT) : (!inhelpscreens && (automapmode & am_active));

    if (oldgamestate != GS_LEVEL) {
      R_FillBackScreen ();    // draw the pattern into the back screen
      redrawborderstuff = isborder;
    } else {
      // CPhipps -
      // If there is a border, and either there was no border last time,
      // or the border might need refreshing, then redraw it.
      redrawborderstuff = isborder && (!isborderstate || borderwillneedredraw);
      // The border may need redrawing next time if the border surrounds the screen,
      // and there is a menu being displayed
      borderwillneedredraw = menuactive && isborder && viewactive && (viewwidth != SCREENWIDTH);
    }
    if (redrawborderstuff)
      R_DrawViewBorder();

    // Now do the drawing
    if (viewactive)
      R_RenderPlayerView (&players[displayplayer]);
    if (automapmode & am_active)
      AM_Drawer();
    ST_Drawer((viewheight != SCREENHEIGHT) || ((automapmode & am_active) && !(automapmode & am_overlay)), redrawborderstuff);
    R_DrawViewBorder();
    HU_Drawer();
  }

  isborderstate      = isborder;
  oldgamestate = wipegamestate = gamestate;

  // draw pause pic
  if (paused) {
    // Simplified the "logic" here and no need for x-coord caching - POPE
    V_DrawNamePatch((320 - V_NamePatchWidth("M_PAUSE"))/2, 4,
                    0, "M_PAUSE", CR_DEFAULT, VPT_STRETCH);
  }

  // menus go directly to the screen
  M_Drawer();          // menu is drawn even on top of everything
#ifdef HAVE_NET
  NetUpdate();         // send out any new accumulation
#else
  D_BuildNewTiccmds();
#endif

  // normal update
  if (!wipe)
    I_FinishUpdate ();              // page flip or blit buffer
  else {
    // wipe update
    wipe_EndScreen();
    D_Wipe();
  }

  I_EndDisplay();

  //e6y: don't thrash cpu during pausing
  if (paused) {
    I_uSleep(1000);
  }
}

//
//  D_DoomLoop()
//
// Not a globally visible function,
//  just included for source reference,
//  called by D_DoomMain, never exits.
// Manages timing and IO,
//  calls all ?_Responder, ?_Ticker, and ?_Drawer,
//  calls I_GetTime, I_StartFrame, and I_StartTic
//

static void D_DoomLoop(void)
{
  for (;;)
    {
      WasRenderedInTryRunTics = false;
      // frame syncronous IO operations
      I_StartFrame ();

      if (ffmap == gamemap) ffmap = 0;

      // process one or more tics
      if (singletics)
        {
          I_StartTic ();
          G_BuildTiccmd (&netcmds[consoleplayer][maketic%BACKUPTICS]);
          if (advancedemo)
            D_DoAdvanceDemo ();
          M_Ticker ();
          G_Ticker ();
          gametic++;
          maketic++;
        }
      else
        TryRunTics (); // will run at least one tic

      // killough 3/16/98: change consoleplayer to displayplayer
      if (players[displayplayer].mo) // cph 2002/08/10
        S_UpdateSounds(players[displayplayer].mo);// move positional sounds

      // Update display, next frame, with current state.
      if (!movement_smooth || !WasRenderedInTryRunTics || gamestate != wipegamestate)
        D_Display();
    }
}

//
//  DEMO LOOP
//

//
// D_PageTicker
// Handles timing for warped projection
//
void D_PageTicker(void)
{
  if (--pagetic < 0)
    D_AdvanceDemo();
}

//
// D_AdvanceDemo
// Called after each demo or intro demosequence finishes
//
void D_AdvanceDemo (void)
{
  advancedemo = true;
}

/* killough 11/98: functions to perform demo sequences
 * cphipps 10/99: constness fixes
 */

static void D_SetPageName(const char *name)
{
  pagename = name;
}

static void D_DrawTitle1(const char *name)
{
  S_StartMusic(mus_intro);
  pagetic = (TICRATE*170)/35;
  D_SetPageName(name);
}

static void D_DrawTitle2(const char *name)
{
  S_StartMusic(mus_dm2ttl);
  D_SetPageName(name);
}

/* killough 11/98: tabulate demo sequences
 */

static struct
{
  void (*func)(const char *);
  const char *name;
} const demostates[][4] =
  {
    {
      {D_DrawTitle1, "TITLEPIC"},
      {D_DrawTitle1, "TITLEPIC"},
      {D_DrawTitle2, "TITLEPIC"},
      {D_DrawTitle1, "TITLEPIC"},
    },

    {
      {G_DeferedPlayDemo, "demo1"},
      {G_DeferedPlayDemo, "demo1"},
      {G_DeferedPlayDemo, "demo1"},
      {G_DeferedPlayDemo, "demo1"},
    },
    {
      {D_SetPageName, NULL},
      {D_SetPageName, NULL},
      {D_SetPageName, NULL},
      {D_SetPageName, NULL},
    },

    {
      {G_DeferedPlayDemo, "demo2"},
      {G_DeferedPlayDemo, "demo2"},
      {G_DeferedPlayDemo, "demo2"},
      {G_DeferedPlayDemo, "demo2"},
    },

    {
/*
      {D_SetPageName, "HELP2"},
      {D_SetPageName, "HELP2"},
*/
      {D_SetPageName, NULL},
      {D_SetPageName, NULL},
      {D_SetPageName, "CREDIT"},
      {D_DrawTitle1,  "TITLEPIC"},
    },

    {
      {G_DeferedPlayDemo, "demo3"},
      {G_DeferedPlayDemo, "demo3"},
      {G_DeferedPlayDemo, "demo3"},
      {G_DeferedPlayDemo, "demo3"},
    },

    {
      {NULL},
      {NULL},
      {NULL},
      {D_SetPageName, "CREDIT"},
    },

    {
      {NULL},
      {NULL},
      {NULL},
      {G_DeferedPlayDemo, "demo4"},
    },

    {
      {NULL},
      {NULL},
      {NULL},
      {NULL},
    }
  };

/*
 * This cycles through the demo sequences.
 * killough 11/98: made table-driven
 */

void D_DoAdvanceDemo(void)
{
  players[consoleplayer].playerstate = PST_LIVE;  /* not reborn */
  advancedemo = usergame = paused = false;
  gameaction = ga_nothing;

  pagetic = TICRATE * 11;         /* killough 11/98: default behavior */
  gamestate = GS_DEMOSCREEN;

  if (netgame && !demoplayback)
    demosequence = 0;
  else if (!demostates[++demosequence][gamemode].func)
    demosequence = 0;
  demostates[demosequence][gamemode].func
    (demostates[demosequence][gamemode].name);
}

//
// D_StartTitle
//
void D_StartTitle (void)
{
  gameaction = ga_nothing;
  demosequence = -1;
  D_AdvanceDemo();
}

//
// D_AddFile
//
// Add files to be loaded later by W_Init()
//
bool D_AddFile(const char *file)
{
  char relpath[PATH_MAX + 1];

  if (numwadfiles == MAX_WAD_FILES)
    I_Error("D_AddFile: Can't load more than %d WADs\n", MAX_WAD_FILES);

  snprintf(relpath, PATH_MAX, "%s/%s", I_DoomExeDir(), file);

  if (access(relpath, R_OK) == 0)
    file = relpath;
  else if (access(file, R_OK) != 0) {
    lprintf(LO_WARN, "D_AddFile: %s not found\n", file);
    return false;
  }

  wadfiles[numwadfiles++] = (wadfile_info_t){
      .name = strdup(file),
      .handle = NULL,
      .data = NULL,
      .size = 0,
  };

  return true;
}

//
// CheckIWAD
//
// Verify a file is indeed tagged as an IWAD
// Scan its lumps for levelnames and return gamemode as indicated
// Detect missing wolf levels in DOOM II
//
// The filename to check is passed in iwadname, the gamemode detected is
// returned in gmode, hassec returns the presence of secret levels
//
// jff 4/19/98 Add routine to test IWAD for validity and determine
// the gamemode from it. Also note if DOOM II, whether secret levels exist
// CPhipps - const char* for iwadname, made static
static void CheckIWAD(const char *iwadname,GameMode_t *gmode,boolean *hassec)
{
    int ud=0,rg=0,sw=0,cm=0,sc=0;
    wadinfo_t header;
    FILE *fp;

    if (!(fp = fopen(iwadname, "rb")))
      I_Error("CheckIWAD: Can't open IWAD %s", iwadname);

    fread(&header, sizeof(header), 1, fp);
    // read IWAD header
    if (!strncmp(header.identification, "IWAD", 4))
    {
      size_t length;
      filelump_t *fileinfo;

      // read IWAD directory
      header.numlumps = LONG(header.numlumps);
      header.infotableofs = LONG(header.infotableofs);
      length = header.numlumps;
      fileinfo = calloc(sizeof(filelump_t), length);
      fseek(fp, header.infotableofs, SEEK_SET);
      fread(fileinfo, sizeof(filelump_t), length, fp);


      // scan directory for levelname lumps
      while (length--)
        if (fileinfo[length].name[0] == 'E' &&
            fileinfo[length].name[2] == 'M' &&
            fileinfo[length].name[4] == 0)
        {
          if (fileinfo[length].name[1] == '4')
            ++ud;
          else if (fileinfo[length].name[1] == '3')
            ++rg;
          else if (fileinfo[length].name[1] == '2')
            ++rg;
          else if (fileinfo[length].name[1] == '1')
            ++sw;
        }
        else if (fileinfo[length].name[0] == 'M' &&
                  fileinfo[length].name[1] == 'A' &&
                  fileinfo[length].name[2] == 'P' &&
                  fileinfo[length].name[5] == 0)
        {
          ++cm;
          if (fileinfo[length].name[3] == '3')
            if (fileinfo[length].name[4] == '1' ||
                fileinfo[length].name[4] == '2')
              ++sc;
        }

      free(fileinfo);
    }
    else // missing IWAD tag in header
      I_Error("CheckIWAD: IWAD tag %s not present", iwadname);

    // Determine game mode from levels present
    // Must be a full set for whichever mode is present
    // Lack of wolf-3d levels also detected here

    *gmode = indetermined;
    *hassec = false;
    if (cm>=30)
    {
      *gmode = commercial;
      *hassec = sc>=2;
    }
    else if (ud>=9)
      *gmode = retail;
    else if (rg>=18)
      *gmode = registered;
    else if (sw>=9)
      *gmode = shareware;
}


static void L_SetupConsoleMasks(void) {
  int p;
  int i;
  const char *cena="ICWEFDA",*pos;  //jff 9/3/98 use this for parsing console masks // CPhipps - const char*'s

  //jff 9/3/98 get mask for console output filter
  if ((p = M_CheckParm ("-cout"))) {
    lprintf(LO_DEBUG, "mask for console output: ");
    if (++p != myargc && *myargv[p] != '-')
      for (i=0,cons_output_mask=0;(size_t)i<strlen(myargv[p]);i++)
        if ((pos = strchr(cena,toupper(myargv[p][i])))) {
          cons_output_mask |= (1<<(pos-cena));
          lprintf(LO_DEBUG, "%c", toupper(myargv[p][i]));
        }
    lprintf(LO_DEBUG, "\n");
  }
}

//
// D_DoomMainSetup
//
// CPhipps - the old contents of D_DoomMain, but moved out of the main
//  line of execution so its stack space can be freed

static void D_DoomMainSetup(void)
{
  const char *doomverstr;
  const char *iwad;
  int p,slot;

  L_SetupConsoleMasks();
  setbuf(stdout,NULL);

	states = memcpy(malloc(sizeof(rostates)), rostates, sizeof(rostates));
  modifiedgame = false;
  numwadfiles = 0;

  lprintf(LO_INFO, "M_LoadDefaults: Load system defaults.\n");
  M_LoadDefaults(); // load before initing other systems

  // Try loading iwad specified as parameter
  if ((p = M_CheckParm("-iwad")) && (++p < myargc))
    D_AddFile(myargv[p]);

  // If that fails then try known standard iwad names
  for (int i = 0; !numwadfiles && standard_iwads[i]; i++)
    D_AddFile(standard_iwads[i]);

  if (!numwadfiles)
    I_Error("IWAD not found\n");

  iwad = wadfiles[numwadfiles-1].name;

  CheckIWAD(iwad, &gamemode, &haswolflevels);
  lprintf(LO_CONFIRM, "IWAD found: %s\n", iwad);

  switch (gamemode)
  {
  case retail:
    doomverstr = "The Ultimate DOOM";
    gamemission = doom;
    break;
  case shareware:
    doomverstr = "DOOM Shareware";
    gamemission = doom;
    break;
  case registered:
    doomverstr = "DOOM Registered";
    gamemission = doom;
    break;
  case commercial:  // Ty 08/27/98 - fixed gamemode vs gamemission
      p = strlen(iwad);
      if (p>=7 && !strncasecmp(iwad+p-7,"tnt.wad",7)) {
        doomverstr = "DOOM 2: TNT - Evilution";
        gamemission = pack_tnt;
      } else if (p>=12 && !strncasecmp(iwad+p-12,"plutonia.wad",12)) {
        doomverstr = "DOOM 2: Plutonia Experiment";
        gamemission = pack_plut;
      } else {
        doomverstr = "DOOM 2: Hell on Earth";
        gamemission = doom2;
      }
    break;
  default:
    doomverstr = "Public DOOM";
    gamemission = none;
    break;
  }

  /* cphipps - the main display. This shows the build date, copyright, and game type */
  lprintf(LO_ALWAYS, "%s %s (built %s), playing: %s\n"
    "PrBoom is released under the GNU General Public license v2.0.\n"
    "You are welcome to redistribute it under certain conditions.\n"
    "It comes with ABSOLUTELY NO WARRANTY. See the file COPYING for details.\n",
    PACKAGE, VERSION, version_date, doomverstr);

  // Add prboom.wad at the very beginning so it can be overriden by mods
#ifdef PRBOOMWAD
  #include "prboom_wad.h"
  wadfiles[numwadfiles++] = (wadfile_info_t){
      .name = "prboom.wad",
      .data = prboom_wad,
      .size = prboom_wad_size,
      .handle = NULL,
  };
#else
  if (!D_AddFile("prboom.wad"))
    I_Error("PRBOOM.WAD not found\n");
#endif

  if ((devparm = M_CheckParm("-devparm")))
    lprintf(LO_CONFIRM, "%s", s_D_DEVSTR);

  // set save path to -save parm or current dir
  basesavegame = I_DoomExeDir();
  if ((p=M_CheckParm("-save")) && p<myargc-1) //jff 3/24/98 if -save present
  {
    struct stat sbuf; //jff 3/24/98 used to test save path for existence
    if (!stat(myargv[p+1], &sbuf) && S_ISDIR(sbuf.st_mode))
      basesavegame = strdup(myargv[p+1]);
    else
      lprintf(LO_ERROR,"Error: -save path does not exist!\n");
  }
  lprintf(LO_CONFIRM,"Save path set to: %s\n", basesavegame);

  // jff 1/24/98 set both working and command line value of play parms
  nomonsters = clnomonsters = M_CheckParm ("-nomonsters");
  respawnparm = clrespawnparm = M_CheckParm ("-respawn");
  fastparm = clfastparm = M_CheckParm ("-fast");
  // jff 1/24/98 end of set to both working and command line value

  if (M_CheckParm ("-altdeath"))
    deathmatch = 2;
  else if (M_CheckParm ("-deathmatch"))
    deathmatch = 1;

  if ((devparm = M_CheckParm("-devparm")))
    lprintf(LO_CONFIRM,"%s",s_D_DEVSTR);

  // turbo option
  if ((p=M_CheckParm ("-turbo")))
    {
      int scale = 200;
      extern int forwardmove[2];
      extern int sidemove[2];

      if (p<myargc-1)
        scale = atoi(myargv[p+1]);
      if (scale < 10)
        scale = 10;
      if (scale > 400)
        scale = 400;
      //jff 9/3/98 use logical output routine
      lprintf (LO_CONFIRM,"turbo scale: %i%%\n",scale);
      forwardmove[0] = forwardmove[0]*scale/100;
      forwardmove[1] = forwardmove[1]*scale/100;
      sidemove[0] = sidemove[0]*scale/100;
      sidemove[1] = sidemove[1]*scale/100;
    }

  // get skill / episode / map from parms

  startskill = sk_none; // jff 3/24/98 was sk_medium, just note not picked
  startepisode = 1;
  startmap = 1;
  autostart = false;

  if ((p = M_CheckParm ("-skill")) && p < myargc-1)
    {
      startskill = myargv[p+1][0]-'1';
      autostart = true;
    }

  if ((p = M_CheckParm ("-episode")) && p < myargc-1)
    {
      startepisode = myargv[p+1][0]-'0';
      startmap = 1;
      autostart = true;
    }

  if ((p = M_CheckParm ("-timer")) && p < myargc-1 && deathmatch)
    {
      int time = atoi(myargv[p+1]);
      //jff 9/3/98 use logical output routine
      lprintf(LO_CONFIRM,"Levels will end after %d minute%s.\n", time, time>1 ? "s" : "");
    }

  if ((p = M_CheckParm ("-warp")) ||      // killough 5/2/98
       (p = M_CheckParm ("-wart")))
       // Ty 08/29/98 - moved this check later so we can have -warp alone: && p < myargc-1)
  {
    startmap = 0; // Ty 08/29/98 - allow "-warp x" to go to first map in wad(s)
    autostart = true; // Ty 08/29/98 - move outside the decision tree
    if (gamemode == commercial)
    {
      if (p < myargc-1)
        startmap = atoi(myargv[p+1]);   // Ty 08/29/98 - add test if last parm
    }
    else    // 1/25/98 killough: fix -warp xxx from crashing Doom 1 / UD
    {
      if (p < myargc-2)
      {
        startepisode = atoi(myargv[++p]);
        startmap = atoi(myargv[p+1]);
      }
    }
  }
  // Ty 08/29/98 - later we'll check for startmap=0 and autostart=true
  // as a special case that -warp * was used.  Actually -warp with any
  // non-numeric will do that but we'll only document "*"

  nomusicparm = M_CheckParm("-nosound") || M_CheckParm("-nomusic");
  nosfxparm   = M_CheckParm("-nosound") || M_CheckParm("-nosfx");
  nodrawers = M_CheckParm ("-nodraw");
  noblit = M_CheckParm ("-noblit");

  //proff 11/22/98: Added setting of viewangleoffset
  if ((p = M_CheckParm("-viewangle")))
  {
    viewangleoffset = atoi(myargv[p+1]);
    viewangleoffset = viewangleoffset<0 ? 0 : (viewangleoffset>7 ? 7 : viewangleoffset);
    viewangleoffset = (8-viewangleoffset) * ANG45;
  }

  // init subsystems

  G_ReloadDefaults();    // killough 3/4/98: set defaults just loaded.
  // jff 3/24/98 this sets startskill if it was -1

#ifndef NODEHSUPPORT
  // ty 03/09/98 do dehacked stuff
  // Note: do this before any other since it is expected by
  // the deh patch author that this is actually part of the EXE itself
  // Using -deh in BOOM, others use -dehacked.
  // Ty 03/18/98 also allow .bex extension.  .bex overrides if both exist.

  if ((p = M_CheckParm ("-deh")))
    {
      char file[PATH_MAX+1];      // cph - localised
      // the parms after p are deh/bex file names,
      // until end of parms or another - preceded parm
      // Ty 04/11/98 - Allow multiple -deh files in a row

      while (++p != myargc && *myargv[p] != '-')
        {
          AddDefaultExtension(strcpy(file, myargv[p]), ".bex");
          if (access(file, F_OK))  // nope
            {
              AddDefaultExtension(strcpy(file, myargv[p]), ".deh");
              if (access(file, F_OK))  // still nope
                I_Error("D_DoomMainSetup: Cannot find .deh or .bex file named %s",myargv[p]);
            }
          // during the beta we have debug output to dehout.txt
          D_ProcessDehFile(file,D_dehout(),0);
        }
    }
  // ty 03/09/98 end of do dehacked stuff
#endif

  // add any files specified on the command line with -file wadfile
  // to the wad list
  if ((p = M_CheckParm ("-file")))
    {
      // the parms after p are wadfile/lump names,
      // until end of parms or another - preceded parm
      while (++p != myargc && *myargv[p] != '-')
        if (D_AddFile(myargv[p]))
          modifiedgame = true;
    }

  if (!(p = M_CheckParm("-playdemo")) || p >= myargc-1) {   /* killough */
    if ((p = M_CheckParm ("-fastdemo")) && p < myargc-1)    /* killough */
      fastdemo = true;             // run at fastest speed possible
    else
      p = M_CheckParm ("-timedemo");
  }

  if (p && p < myargc-1)
    {
      char file[PATH_MAX+1];      // cph - localised
      strcpy(file,myargv[p+1]);
      AddDefaultExtension(file,".lmp");     // killough
      D_AddFile (file);
      //jff 9/3/98 use logical output routine
      lprintf(LO_CONFIRM,"Playing demo %s\n",file);
      if ((p = M_CheckParm ("-ffmap")) && p < myargc-1) {
        ffmap = atoi(myargv[p+1]);
      }
    }

  // internal translucency set to config file value               // phares
  general_translucency = default_translucency;                    // phares

  lprintf(LO_INFO, "I_Init: Setting up machine state.\n");
  I_Init();

  // CPhipps - move up netgame init
  lprintf(LO_INFO, "D_InitNetGame: Checking for network game.\n");
  D_InitNetGame();

  lprintf(LO_INFO, "W_Init: Init WADfiles.\n");
  W_Init(); // CPhipps - handling of wadfiles init changed

  lprintf(LO_INFO, "V_Init: Setting up video.\n");
  V_Init(SCREENWIDTH, SCREENHEIGHT, default_videomode);

#ifndef NODEHSUPPORT
  // e6y: option to disable automatic loading of dehacked-in-wad lump
  if (!M_CheckParm ("-nodeh"))
    if ((p = W_CheckNumForName("DEHACKED")) != -1) // cph - add dehacked-in-a-wad support
      D_ProcessDehFile(NULL, D_dehout(), p);
#endif

  lprintf(LO_INFO, "M_Init: Init miscellaneous info.\n");
  M_Init();

#ifdef HAVE_NET
  // CPhipps - now wait for netgame start
  D_CheckNetGame();
#endif

  lprintf(LO_INFO, "R_Init: Init DOOM refresh daemon:\n");
  R_Init();

  lprintf(LO_INFO, "P_Init: Init Playloop state.\n");
  P_Init();

  lprintf(LO_INFO, "S_Init: Setting up sound.\n");
  S_Init(snd_SfxVolume, snd_MusicVolume);

  lprintf(LO_INFO, "HU_Init: Setting up heads up display.\n");
  HU_Init();

  lprintf(LO_INFO, "ST_Init: Init status bar.\n");
  ST_Init();

  // Show hacked startup messages, if any
  if (*startup1 || *startup2 || *startup3 || *startup4 || *startup5)
  {
    if (*startup1) lprintf(LO_INFO, "%s", startup1);
    if (*startup2) lprintf(LO_INFO, "%s", startup2);
    if (*startup3) lprintf(LO_INFO, "%s", startup3);
    if (*startup4) lprintf(LO_INFO, "%s", startup4);
    if (*startup5) lprintf(LO_INFO, "%s", startup5);
  }

  lprintf(LO_INFO, "\n");

  idmusnum = -1; //jff 3/17/98 insure idmus number is blank

  // start the apropriate game based on parms

  // killough 12/98:
  // Support -loadgame with -record and reimplement -recordfrom.

  if ((slot = M_CheckParm("-recordfrom")) && (p = slot+2) < myargc)
    G_RecordDemo(myargv[p]);
  else
  {
    slot = M_CheckParm("-loadgame");
    if ((p = M_CheckParm("-record")) && ++p < myargc)
    {
      autostart = true;
      G_RecordDemo(myargv[p]);
    }
  }

  if ((p = M_CheckParm ("-fastdemo")) && ++p < myargc)
  {                                 // killough
    fastdemo = true;                // run at fastest speed possible
    timingdemo = true;              // show stats after quit
    G_DeferedPlayDemo(myargv[p]);
    singledemo = true;              // quit after one demo
  }
  else if ((p = M_CheckParm("-timedemo")) && ++p < myargc)
  {
    singletics = true;
    timingdemo = true;            // show stats after quit
    G_DeferedPlayDemo(myargv[p]);
    singledemo = true;            // quit after one demo
  }
  else if ((p = M_CheckParm("-playdemo")) && ++p < myargc)
  {
    G_DeferedPlayDemo(myargv[p]);
    singledemo = true;          // quit after one demo
  }

  if (slot && ++slot < myargc)
  {
    slot = atoi(myargv[slot]);        // killough 3/16/98: add slot info
    G_LoadGame(slot, true);           // killough 5/15/98: add command flag // cph - no filename
  }
  else if (!singledemo)
  {
    if (autostart || netgame)
    {
      G_InitNew(startskill, startepisode, startmap);
      if (demorecording)
        G_BeginRecording();
    }
    else
      D_StartTitle();                 // start up intro loop
  }
}

//
// D_DoomMain
//

void D_DoomMain(void)
{
  D_DoomMainSetup(); // CPhipps - setup out of main execution stack
  D_DoomLoop();      // never returns
}
