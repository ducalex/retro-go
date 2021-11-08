/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2006 by
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
 *  Dehacked file support
 *  New for the TeamTNT "Boom" engine
 * 
 *  Author: Ty Halderman, TeamTNT
 *
 *  Description: This file translates the #defined string constants
 *  to named variables to externalize them for deh/bex changes.
 *  Should be able to compile with D_FRENCH (for example) and still
 *  work (untested).
 *
 */

#ifndef __D_DEH__
#define __D_DEH__

void ProcessDehFile(const char *filename, const char *outfilename, int lumpnum);

//
//      Ty 03/22/98 - note that we are keeping the english versions and
//      comments in this file
//      New string names all start with an extra s_ to avoid conflicts,
//      but are otherwise identical to the original including uppercase.
//      This is partly to keep the changes simple and partly for easier
//      identification of the locations in which they're used.
//
//      Printed strings for translation
//

//
// D_Main.C
//
//#define D_DEVSTR      "Development mode ON.\n"
extern const char *s_D_DEVSTR; // = D_DEVSTR;
//#define D_CDROM       "CD-ROM Version: default.cfg from c:\\doomdata\n"
extern const char *s_D_CDROM; // = D_CDROM;

//
//      M_Menu.C
//
//#define PRESSKEY      "press a key."
extern const char *s_PRESSKEY; // = PRESSKEY;
//#define PRESSYN       "press y or n."
extern const char *s_PRESSYN; // = PRESSYN;
//#define QUITMSG       "are you sure you want to\nquit this great game?"
extern const char *s_QUITMSG; // = QUITMSG;
//#define LOADNET       "you can't do load while in a net game!\n\n"PRESSKEY
extern const char *s_LOADNET; // = LOADNET;
//#define QLOADNET      "you can't quickload during a netgame!\n\n"PRESSKEY
extern const char *s_QLOADNET; // = QLOADNET;
//#define QSAVESPOT     "you haven't picked a quicksave slot yet!\n\n"PRESSKEY
extern const char *s_QSAVESPOT; // = QSAVESPOT;
//#define SAVEDEAD      "you can't save if you aren't playing!\n\n"PRESSKEY
extern const char *s_SAVEDEAD; // = SAVEDEAD;
//#define QSPROMPT      "quicksave over your game named\n\n'%s'?\n\n"PRESSYN
extern const char *s_QSPROMPT; // = QSPROMPT;
//#define QLPROMPT      "do you want to quickload the game named\n\n'%s'?\n\n"PRESSYN
extern const char *s_QLPROMPT; // = QLPROMPT;

/*
#define NEWGAME       \
"you can't start a new game\n"\
"while in a network game.\n\n"PRESSKEY
*/
extern const char *s_NEWGAME; // = NEWGAME;

// CPhipps - message given when asked if to restart the level
extern const char *s_RESTARTLEVEL;

/*
#define NIGHTMARE     \
"are you sure? this skill level\n"\
"isn't even remotely fair.\n\n"PRESSYN
*/
extern const char *s_NIGHTMARE; // = NIGHTMARE;

/*
#define SWSTRING      \
"this is the shareware version of doom.\n\n"\
"you need to order the entire trilogy.\n\n"PRESSKEY
*/
extern const char *s_SWSTRING; // = SWSTRING;

//#define MSGOFF        "Messages OFF"
extern const char *s_MSGOFF; // = MSGOFF;
//#define MSGON         "Messages ON"
extern const char *s_MSGON; // = MSGON;
//#define NETEND        "you can't end a netgame!\n\n"PRESSKEY
extern const char *s_NETEND; // = NETEND;
//#define ENDGAME       "are you sure you want to end the game?\n\n"PRESSYN
extern const char *s_ENDGAME; // = ENDGAME;

//#define DOSY          "(press y to quit)"
extern const char *s_DOSY; // = DOSY;

//#define DETAILHI      "High detail"
extern const char *s_DETAILHI; // = DETAILHI;
//#define DETAILLO      "Low detail"
extern const char *s_DETAILLO; // = DETAILLO;
//#define GAMMALVL0     "Gamma correction OFF"
extern const char *s_GAMMALVL0; // = GAMMALVL0;
//#define GAMMALVL1     "Gamma correction level 1"
extern const char *s_GAMMALVL1; // = GAMMALVL1;
//#define GAMMALVL2     "Gamma correction level 2"
extern const char *s_GAMMALVL2; // = GAMMALVL2;
//#define GAMMALVL3     "Gamma correction level 3"
extern const char *s_GAMMALVL3; // = GAMMALVL3;
//#define GAMMALVL4     "Gamma correction level 4"
extern const char *s_GAMMALVL4; // = GAMMALVL4;
//#define EMPTYSTRING   "empty slot"
extern const char *s_EMPTYSTRING; // = EMPTYSTRING;

//
//      P_inter.C
//
//#define GOTARMOR      "Picked up the armor."
extern const char *s_GOTARMOR; // = GOTARMOR;
//#define GOTMEGA       "Picked up the MegaArmor!"
extern const char *s_GOTMEGA; // = GOTMEGA;
//#define GOTHTHBONUS   "Picked up a health bonus."
extern const char *s_GOTHTHBONUS; // = GOTHTHBONUS;
//#define GOTARMBONUS   "Picked up an armor bonus."
extern const char *s_GOTARMBONUS; // = GOTARMBONUS;
//#define GOTSTIM       "Picked up a stimpack."
extern const char *s_GOTSTIM; // = GOTSTIM;
//#define GOTMEDINEED   "Picked up a medikit that you REALLY need!"
extern const char *s_GOTMEDINEED; // = GOTMEDINEED;
//#define GOTMEDIKIT    "Picked up a medikit."
extern const char *s_GOTMEDIKIT; // = GOTMEDIKIT;
//#define GOTSUPER      "Supercharge!"
extern const char *s_GOTSUPER; // = GOTSUPER;

//#define GOTBLUECARD   "Picked up a blue keycard."
extern const char *s_GOTBLUECARD; // = GOTBLUECARD;
//#define GOTYELWCARD   "Picked up a yellow keycard."
extern const char *s_GOTYELWCARD; // = GOTYELWCARD;
//#define GOTREDCARD    "Picked up a red keycard."
extern const char *s_GOTREDCARD; // = GOTREDCARD;
//#define GOTBLUESKUL   "Picked up a blue skull key."
extern const char *s_GOTBLUESKUL; // = GOTBLUESKUL;
//#define GOTYELWSKUL   "Picked up a yellow skull key."
extern const char *s_GOTYELWSKUL; // = GOTYELWSKUL;
//#define GOTREDSKULL   "Picked up a red skull key."
extern const char *s_GOTREDSKULL; // = GOTREDSKULL;

//#define GOTINVUL      "Invulnerability!"
extern const char *s_GOTINVUL; // = GOTINVUL;
//#define GOTBERSERK    "Berserk!"
extern const char *s_GOTBERSERK; // = GOTBERSERK;
//#define GOTINVIS      "Partial Invisibility"
extern const char *s_GOTINVIS; // = GOTINVIS;
//#define GOTSUIT       "Radiation Shielding Suit"
extern const char *s_GOTSUIT; // = GOTSUIT;
//#define GOTMAP        "Computer Area Map"
extern const char *s_GOTMAP; // = GOTMAP;
//#define GOTVISOR      "Light Amplification Visor"
extern const char *s_GOTVISOR; // = GOTVISOR;
//#define GOTMSPHERE    "MegaSphere!"
extern const char *s_GOTMSPHERE; // = GOTMSPHERE;

//#define GOTCLIP       "Picked up a clip."
extern const char *s_GOTCLIP; // = GOTCLIP;
//#define GOTCLIPBOX    "Picked up a box of bullets."
extern const char *s_GOTCLIPBOX; // = GOTCLIPBOX;
//#define GOTROCKET     "Picked up a rocket."
extern const char *s_GOTROCKET; // = GOTROCKET;
//#define GOTROCKBOX    "Picked up a box of rockets."
extern const char *s_GOTROCKBOX; // = GOTROCKBOX;
//#define GOTCELL       "Picked up an energy cell."
extern const char *s_GOTCELL; // = GOTCELL;
//#define GOTCELLBOX    "Picked up an energy cell pack."
extern const char *s_GOTCELLBOX; // = GOTCELLBOX;
//#define GOTSHELLS     "Picked up 4 shotgun shells."
extern const char *s_GOTSHELLS; // = GOTSHELLS;
//#define GOTSHELLBOX   "Picked up a box of shotgun shells."
extern const char *s_GOTSHELLBOX; // = GOTSHELLBOX;
//#define GOTBACKPACK   "Picked up a backpack full of ammo!"
extern const char *s_GOTBACKPACK; // = GOTBACKPACK;

//#define GOTBFG9000    "You got the BFG9000!  Oh, yes."
extern const char *s_GOTBFG9000; // = GOTBFG9000;
//#define GOTCHAINGUN   "You got the chaingun!"
extern const char *s_GOTCHAINGUN; // = GOTCHAINGUN;
//#define GOTCHAINSAW   "A chainsaw!  Find some meat!"
extern const char *s_GOTCHAINSAW; // = GOTCHAINSAW;
//#define GOTLAUNCHER   "You got the rocket launcher!"
extern const char *s_GOTLAUNCHER; // = GOTLAUNCHER;
//#define GOTPLASMA     "You got the plasma gun!"
extern const char *s_GOTPLASMA; // = GOTPLASMA;
//#define GOTSHOTGUN    "You got the shotgun!"
extern const char *s_GOTSHOTGUN; // = GOTSHOTGUN;
//#define GOTSHOTGUN2   "You got the super shotgun!"
extern const char *s_GOTSHOTGUN2; // = GOTSHOTGUN2;

//
// P_Doors.C
//
//#define PD_BLUEO      "You need a blue key to activate this object"
extern const char *s_PD_BLUEO; // = PD_BLUEO;
//#define PD_REDO       "You need a red key to activate this object"
extern const char *s_PD_REDO; // = PD_REDO;
//#define PD_YELLOWO    "You need a yellow key to activate this object"
extern const char *s_PD_YELLOWO; // = PD_YELLOWO;
//#define PD_BLUEK      "You need a blue key to open this door"
extern const char *s_PD_BLUEK; // = PD_BLUEK;
//#define PD_REDK       "You need a red key to open this door"
extern const char *s_PD_REDK; // = PD_REDK;
//#define PD_YELLOWK    "You need a yellow key to open this door"
extern const char *s_PD_YELLOWK; // = PD_YELLOWK;
//jff 02/05/98 Create messages specific to card and skull keys
//#define PD_BLUEC      "You need a blue card to open this door"
extern const char *s_PD_BLUEC; // = PD_BLUEC;
//#define PD_REDC       "You need a red card to open this door"
extern const char *s_PD_REDC; // = PD_REDC;
//#define PD_YELLOWC    "You need a yellow card to open this door"
extern const char *s_PD_YELLOWC; // = PD_YELLOWC;
//#define PD_BLUES      "You need a blue skull to open this door"
extern const char *s_PD_BLUES; // = PD_BLUES;
//#define PD_REDS       "You need a red skull to open this door"
extern const char *s_PD_REDS; // = PD_REDS;
//#define PD_YELLOWS    "You need a yellow skull to open this door"
extern const char *s_PD_YELLOWS; // = PD_YELLOWS;
//#define PD_ANY        "Any key will open this door"
extern const char *s_PD_ANY; // = PD_ANY;
//#define PD_ALL3 "You need all three keys to open this door"
extern const char *s_PD_ALL3; // = PD_ALL3;
//#define PD_ALL6 "You need all six keys to open this door"
extern const char *s_PD_ALL6; // = PD_ALL6;

//
//      G_game.C
//
//#define GGSAVED       "game saved."
extern const char *s_GGSAVED; // = GGSAVED;

//
//      HU_stuff.C
//
//#define HUSTR_MSGU    "[Message unsent]"
extern const char *s_HUSTR_MSGU; // = HUSTR_MSGU;

//#define HUSTR_E1M1    "E1M1: Hangar"
extern const char *s_HUSTR_E1M1; // = HUSTR_E1M1;
//#define HUSTR_E1M2    "E1M2: Nuclear Plant"
extern const char *s_HUSTR_E1M2; // = HUSTR_E1M2;
//#define HUSTR_E1M3    "E1M3: Toxin Refinery"
extern const char *s_HUSTR_E1M3; // = HUSTR_E1M3;
//#define HUSTR_E1M4    "E1M4: Command Control"
extern const char *s_HUSTR_E1M4; // = HUSTR_E1M4;
//#define HUSTR_E1M5    "E1M5: Phobos Lab"
extern const char *s_HUSTR_E1M5; // = HUSTR_E1M5;
//#define HUSTR_E1M6    "E1M6: Central Processing"
extern const char *s_HUSTR_E1M6; // = HUSTR_E1M6;
//#define HUSTR_E1M7    "E1M7: Computer Station"
extern const char *s_HUSTR_E1M7; // = HUSTR_E1M7;
//#define HUSTR_E1M8    "E1M8: Phobos Anomaly"
extern const char *s_HUSTR_E1M8; // = HUSTR_E1M8;
//#define HUSTR_E1M9    "E1M9: Military Base"
extern const char *s_HUSTR_E1M9; // = HUSTR_E1M9;

//#define HUSTR_E2M1    "E2M1: Deimos Anomaly"
extern const char *s_HUSTR_E2M1; // = HUSTR_E2M1;
//#define HUSTR_E2M2    "E2M2: Containment Area"
extern const char *s_HUSTR_E2M2; // = HUSTR_E2M2;
//#define HUSTR_E2M3    "E2M3: Refinery"
extern const char *s_HUSTR_E2M3; // = HUSTR_E2M3;
//#define HUSTR_E2M4    "E2M4: Deimos Lab"
extern const char *s_HUSTR_E2M4; // = HUSTR_E2M4;
//#define HUSTR_E2M5    "E2M5: Command Center"
extern const char *s_HUSTR_E2M5; // = HUSTR_E2M5;
//#define HUSTR_E2M6    "E2M6: Halls of the Damned"
extern const char *s_HUSTR_E2M6; // = HUSTR_E2M6;
//#define HUSTR_E2M7    "E2M7: Spawning Vats"
extern const char *s_HUSTR_E2M7; // = HUSTR_E2M7;
//#define HUSTR_E2M8    "E2M8: Tower of Babel"
extern const char *s_HUSTR_E2M8; // = HUSTR_E2M8;
//#define HUSTR_E2M9    "E2M9: Fortress of Mystery"
extern const char *s_HUSTR_E2M9; // = HUSTR_E2M9;

//#define HUSTR_E3M1    "E3M1: Hell Keep"
extern const char *s_HUSTR_E3M1; // = HUSTR_E3M1;
//#define HUSTR_E3M2    "E3M2: Slough of Despair"
extern const char *s_HUSTR_E3M2; // = HUSTR_E3M2;
//#define HUSTR_E3M3    "E3M3: Pandemonium"
extern const char *s_HUSTR_E3M3; // = HUSTR_E3M3;
//#define HUSTR_E3M4    "E3M4: House of Pain"
extern const char *s_HUSTR_E3M4; // = HUSTR_E3M4;
//#define HUSTR_E3M5    "E3M5: Unholy Cathedral"
extern const char *s_HUSTR_E3M5; // = HUSTR_E3M5;
//#define HUSTR_E3M6    "E3M6: Mt. Erebus"
extern const char *s_HUSTR_E3M6; // = HUSTR_E3M6;
//#define HUSTR_E3M7    "E3M7: Limbo"
extern const char *s_HUSTR_E3M7; // = HUSTR_E3M7;
//#define HUSTR_E3M8    "E3M8: Dis"
extern const char *s_HUSTR_E3M8; // = HUSTR_E3M8;
//#define HUSTR_E3M9    "E3M9: Warrens"
extern const char *s_HUSTR_E3M9; // = HUSTR_E3M9;

//#define HUSTR_E4M1    "E4M1: Hell Beneath"
extern const char *s_HUSTR_E4M1; // = HUSTR_E4M1;
//#define HUSTR_E4M2    "E4M2: Perfect Hatred"
extern const char *s_HUSTR_E4M2; // = HUSTR_E4M2;
//#define HUSTR_E4M3    "E4M3: Sever The Wicked"
extern const char *s_HUSTR_E4M3; // = HUSTR_E4M3;
//#define HUSTR_E4M4    "E4M4: Unruly Evil"
extern const char *s_HUSTR_E4M4; // = HUSTR_E4M4;
//#define HUSTR_E4M5    "E4M5: They Will Repent"
extern const char *s_HUSTR_E4M5; // = HUSTR_E4M5;
//#define HUSTR_E4M6    "E4M6: Against Thee Wickedly"
extern const char *s_HUSTR_E4M6; // = HUSTR_E4M6;
//#define HUSTR_E4M7    "E4M7: And Hell Followed"
extern const char *s_HUSTR_E4M7; // = HUSTR_E4M7;
//#define HUSTR_E4M8    "E4M8: Unto The Cruel"
extern const char *s_HUSTR_E4M8; // = HUSTR_E4M8;
//#define HUSTR_E4M9    "E4M9: Fear"
extern const char *s_HUSTR_E4M9; // = HUSTR_E4M9;

//#define HUSTR_1       "level 1: entryway"
extern const char *s_HUSTR_1; // = HUSTR_1;
//#define HUSTR_2       "level 2: underhalls"
extern const char *s_HUSTR_2; // = HUSTR_2;
//#define HUSTR_3       "level 3: the gantlet"
extern const char *s_HUSTR_3; // = HUSTR_3;
//#define HUSTR_4       "level 4: the focus"
extern const char *s_HUSTR_4; // = HUSTR_4;
//#define HUSTR_5       "level 5: the waste tunnels"
extern const char *s_HUSTR_5; // = HUSTR_5;
//#define HUSTR_6       "level 6: the crusher"
extern const char *s_HUSTR_6; // = HUSTR_6;
//#define HUSTR_7       "level 7: dead simple"
extern const char *s_HUSTR_7; // = HUSTR_7;
//#define HUSTR_8       "level 8: tricks and traps"
extern const char *s_HUSTR_8; // = HUSTR_8;
//#define HUSTR_9       "level 9: the pit"
extern const char *s_HUSTR_9; // = HUSTR_9;
//#define HUSTR_10      "level 10: refueling base"
extern const char *s_HUSTR_10; // = HUSTR_10;
//#define HUSTR_11      "level 11: 'o' of destruction!"
extern const char *s_HUSTR_11; // = HUSTR_11;

//#define HUSTR_12      "level 12: the factory"
extern const char *s_HUSTR_12; // = HUSTR_12;
//#define HUSTR_13      "level 13: downtown"
extern const char *s_HUSTR_13; // = HUSTR_13;
//#define HUSTR_14      "level 14: the inmost dens"
extern const char *s_HUSTR_14; // = HUSTR_14;
//#define HUSTR_15      "level 15: industrial zone"
extern const char *s_HUSTR_15; // = HUSTR_15;
//#define HUSTR_16      "level 16: suburbs"
extern const char *s_HUSTR_16; // = HUSTR_16;
//#define HUSTR_17      "level 17: tenements"
extern const char *s_HUSTR_17; // = HUSTR_17;
//#define HUSTR_18      "level 18: the courtyard"
extern const char *s_HUSTR_18; // = HUSTR_18;
//#define HUSTR_19      "level 19: the citadel"
extern const char *s_HUSTR_19; // = HUSTR_19;
//#define HUSTR_20      "level 20: gotcha!"
extern const char *s_HUSTR_20; // = HUSTR_20;

//#define HUSTR_21      "level 21: nirvana"
extern const char *s_HUSTR_21; // = HUSTR_21;
//#define HUSTR_22      "level 22: the catacombs"
extern const char *s_HUSTR_22; // = HUSTR_22;
//#define HUSTR_23      "level 23: barrels o' fun"
extern const char *s_HUSTR_23; // = HUSTR_23;
//#define HUSTR_24      "level 24: the chasm"
extern const char *s_HUSTR_24; // = HUSTR_24;
//#define HUSTR_25      "level 25: bloodfalls"
extern const char *s_HUSTR_25; // = HUSTR_25;
//#define HUSTR_26      "level 26: the abandoned mines"
extern const char *s_HUSTR_26; // = HUSTR_26;
//#define HUSTR_27      "level 27: monster condo"
extern const char *s_HUSTR_27; // = HUSTR_27;
//#define HUSTR_28      "level 28: the spirit world"
extern const char *s_HUSTR_28; // = HUSTR_28;
//#define HUSTR_29      "level 29: the living end"
extern const char *s_HUSTR_29; // = HUSTR_29;
//#define HUSTR_30      "level 30: icon of sin"
extern const char *s_HUSTR_30; // = HUSTR_30;

//#define HUSTR_31      "level 31: wolfenstein"
extern const char *s_HUSTR_31; // = HUSTR_31;
//#define HUSTR_32      "level 32: grosse"
extern const char *s_HUSTR_32; // = HUSTR_32;

//#define PHUSTR_1      "level 1: congo"
extern const char *s_PHUSTR_1; // = PHUSTR_1;
//#define PHUSTR_2      "level 2: well of souls"
extern const char *s_PHUSTR_2; // = PHUSTR_2;
//#define PHUSTR_3      "level 3: aztec"
extern const char *s_PHUSTR_3; // = PHUSTR_3;
//#define PHUSTR_4      "level 4: caged"
extern const char *s_PHUSTR_4; // = PHUSTR_4;
//#define PHUSTR_5      "level 5: ghost town"
extern const char *s_PHUSTR_5; // = PHUSTR_5;
//#define PHUSTR_6      "level 6: baron's lair"
extern const char *s_PHUSTR_6; // = PHUSTR_6;
//#define PHUSTR_7      "level 7: caughtyard"
extern const char *s_PHUSTR_7; // = PHUSTR_7;
//#define PHUSTR_8      "level 8: realm"
extern const char *s_PHUSTR_8; // = PHUSTR_8;
//#define PHUSTR_9      "level 9: abattoire"
extern const char *s_PHUSTR_9; // = PHUSTR_9;
//#define PHUSTR_10     "level 10: onslaught"
extern const char *s_PHUSTR_10; // = PHUSTR_10;
//#define PHUSTR_11     "level 11: hunted"
extern const char *s_PHUSTR_11; // = PHUSTR_11;

//#define PHUSTR_12     "level 12: speed"
extern const char *s_PHUSTR_12; // = PHUSTR_12;
//#define PHUSTR_13     "level 13: the crypt"
extern const char *s_PHUSTR_13; // = PHUSTR_13;
//#define PHUSTR_14     "level 14: genesis"
extern const char *s_PHUSTR_14; // = PHUSTR_14;
//#define PHUSTR_15     "level 15: the twilight"
extern const char *s_PHUSTR_15; // = PHUSTR_15;
//#define PHUSTR_16     "level 16: the omen"
extern const char *s_PHUSTR_16; // = PHUSTR_16;
//#define PHUSTR_17     "level 17: compound"
extern const char *s_PHUSTR_17; // = PHUSTR_17;
//#define PHUSTR_18     "level 18: neurosphere"
extern const char *s_PHUSTR_18; // = PHUSTR_18;
//#define PHUSTR_19     "level 19: nme"
extern const char *s_PHUSTR_19; // = PHUSTR_19;
//#define PHUSTR_20     "level 20: the death domain"
extern const char *s_PHUSTR_20; // = PHUSTR_20;

//#define PHUSTR_21     "level 21: slayer"
extern const char *s_PHUSTR_21; // = PHUSTR_21;
//#define PHUSTR_22     "level 22: impossible mission"
extern const char *s_PHUSTR_22; // = PHUSTR_22;
//#define PHUSTR_23     "level 23: tombstone"
extern const char *s_PHUSTR_23; // = PHUSTR_23;
//#define PHUSTR_24     "level 24: the final frontier"
extern const char *s_PHUSTR_24; // = PHUSTR_24;
//#define PHUSTR_25     "level 25: the temple of darkness"
extern const char *s_PHUSTR_25; // = PHUSTR_25;
//#define PHUSTR_26     "level 26: bunker"
extern const char *s_PHUSTR_26; // = PHUSTR_26;
//#define PHUSTR_27     "level 27: anti-christ"
extern const char *s_PHUSTR_27; // = PHUSTR_27;
//#define PHUSTR_28     "level 28: the sewers"
extern const char *s_PHUSTR_28; // = PHUSTR_28;
//#define PHUSTR_29     "level 29: odyssey of noises"
extern const char *s_PHUSTR_29; // = PHUSTR_29;
//#define PHUSTR_30     "level 30: the gateway of hell"
extern const char *s_PHUSTR_30; // = PHUSTR_30;

//#define PHUSTR_31     "level 31: cyberden"
extern const char *s_PHUSTR_31; // = PHUSTR_31;
//#define PHUSTR_32     "level 32: go 2 it"
extern const char *s_PHUSTR_32; // = PHUSTR_32;

//#define THUSTR_1      "level 1: system control"
extern const char *s_THUSTR_1; // = THUSTR_1;
//#define THUSTR_2      "level 2: human bbq"
extern const char *s_THUSTR_2; // = THUSTR_2;
//#define THUSTR_3      "level 3: power control"
extern const char *s_THUSTR_3; // = THUSTR_3;
//#define THUSTR_4      "level 4: wormhole"
extern const char *s_THUSTR_4; // = THUSTR_4;
//#define THUSTR_5      "level 5: hanger"
extern const char *s_THUSTR_5; // = THUSTR_5;
//#define THUSTR_6      "level 6: open season"
extern const char *s_THUSTR_6; // = THUSTR_6;
//#define THUSTR_7      "level 7: prison"
extern const char *s_THUSTR_7; // = THUSTR_7;
//#define THUSTR_8      "level 8: metal"
extern const char *s_THUSTR_8; // = THUSTR_8;
//#define THUSTR_9      "level 9: stronghold"
extern const char *s_THUSTR_9; // = THUSTR_9;
//#define THUSTR_10     "level 10: redemption"
extern const char *s_THUSTR_10; // = THUSTR_10;
//#define THUSTR_11     "level 11: storage facility"
extern const char *s_THUSTR_11; // = THUSTR_11;

//#define THUSTR_12     "level 12: crater"
extern const char *s_THUSTR_12; // = THUSTR_12;
//#define THUSTR_13     "level 13: nukage processing"
extern const char *s_THUSTR_13; // = THUSTR_13;
//#define THUSTR_14     "level 14: steel works"
extern const char *s_THUSTR_14; // = THUSTR_14;
//#define THUSTR_15     "level 15: dead zone"
extern const char *s_THUSTR_15; // = THUSTR_15;
//#define THUSTR_16     "level 16: deepest reaches"
extern const char *s_THUSTR_16; // = THUSTR_16;
//#define THUSTR_17     "level 17: processing area"
extern const char *s_THUSTR_17; // = THUSTR_17;
//#define THUSTR_18     "level 18: mill"
extern const char *s_THUSTR_18; // = THUSTR_18;
//#define THUSTR_19     "level 19: shipping/respawning"
extern const char *s_THUSTR_19; // = THUSTR_19;
//#define THUSTR_20     "level 20: central processing"
extern const char *s_THUSTR_20; // = THUSTR_20;

//#define THUSTR_21     "level 21: administration center"
extern const char *s_THUSTR_21; // = THUSTR_21;
//#define THUSTR_22     "level 22: habitat"
extern const char *s_THUSTR_22; // = THUSTR_22;
//#define THUSTR_23     "level 23: lunar mining project"
extern const char *s_THUSTR_23; // = THUSTR_23;
//#define THUSTR_24     "level 24: quarry"
extern const char *s_THUSTR_24; // = THUSTR_24;
//#define THUSTR_25     "level 25: baron's den"
extern const char *s_THUSTR_25; // = THUSTR_25;
//#define THUSTR_26     "level 26: ballistyx"
extern const char *s_THUSTR_26; // = THUSTR_26;
//#define THUSTR_27     "level 27: mount pain"
extern const char *s_THUSTR_27; // = THUSTR_27;
//#define THUSTR_28     "level 28: heck"
extern const char *s_THUSTR_28; // = THUSTR_28;
//#define THUSTR_29     "level 29: river styx"
extern const char *s_THUSTR_29; // = THUSTR_29;
//#define THUSTR_30     "level 30: last call"
extern const char *s_THUSTR_30; // = THUSTR_30;

//#define THUSTR_31     "level 31: pharaoh"
extern const char *s_THUSTR_31; // = THUSTR_31;
//#define THUSTR_32     "level 32: caribbean"
extern const char *s_THUSTR_32; // = THUSTR_32;

//#define HUSTR_CHATMACRO1      "I'm ready to kick butt!"
extern const char *s_HUSTR_CHATMACRO1; // = HUSTR_CHATMACRO1;
//#define HUSTR_CHATMACRO2      "I'm OK."
extern const char *s_HUSTR_CHATMACRO2; // = HUSTR_CHATMACRO2;
//#define HUSTR_CHATMACRO3      "I'm not looking too good!"
extern const char *s_HUSTR_CHATMACRO3; // = HUSTR_CHATMACRO3;
//#define HUSTR_CHATMACRO4      "Help!"
extern const char *s_HUSTR_CHATMACRO4; // = HUSTR_CHATMACRO4;
//#define HUSTR_CHATMACRO5      "You suck!"
extern const char *s_HUSTR_CHATMACRO5; // = HUSTR_CHATMACRO5;
//#define HUSTR_CHATMACRO6      "Next time, scumbag..."
extern const char *s_HUSTR_CHATMACRO6; // = HUSTR_CHATMACRO6;
//#define HUSTR_CHATMACRO7      "Come here!"
extern const char *s_HUSTR_CHATMACRO7; // = HUSTR_CHATMACRO7;
//#define HUSTR_CHATMACRO8      "I'll take care of it."
extern const char *s_HUSTR_CHATMACRO8; // = HUSTR_CHATMACRO8;
//#define HUSTR_CHATMACRO9      "Yes"
extern const char *s_HUSTR_CHATMACRO9; // = HUSTR_CHATMACRO9;
//#define HUSTR_CHATMACRO0      "No"
extern const char *s_HUSTR_CHATMACRO0; // = HUSTR_CHATMACRO0;

//#define HUSTR_TALKTOSELF1     "You mumble to yourself"
extern const char *s_HUSTR_TALKTOSELF1; // = HUSTR_TALKTOSELF1;
//#define HUSTR_TALKTOSELF2     "Who's there?"
extern const char *s_HUSTR_TALKTOSELF2; // = HUSTR_TALKTOSELF2;
//#define HUSTR_TALKTOSELF3     "You scare yourself"
extern const char *s_HUSTR_TALKTOSELF3; // = HUSTR_TALKTOSELF3;
//#define HUSTR_TALKTOSELF4     "You start to rave"
extern const char *s_HUSTR_TALKTOSELF4; // = HUSTR_TALKTOSELF4;
//#define HUSTR_TALKTOSELF5     "You've lost it..."
extern const char *s_HUSTR_TALKTOSELF5; // = HUSTR_TALKTOSELF5;

//#define HUSTR_MESSAGESENT     "[Message Sent]"
extern const char *s_HUSTR_MESSAGESENT; // = HUSTR_MESSAGESENT;

// The following should NOT be changed unless it seems
// just AWFULLY necessary

//#define HUSTR_PLRGREEN        "Green: "
extern const char *s_HUSTR_PLRGREEN; // = HUSTR_PLRGREEN;
//#define HUSTR_PLRINDIGO       "Indigo: "
extern const char *s_HUSTR_PLRINDIGO; // = HUSTR_PLRINDIGO;
//#define HUSTR_PLRBROWN        "Brown: "
extern const char *s_HUSTR_PLRBROWN; // = HUSTR_PLRBROWN;
//#define HUSTR_PLRRED          "Red: "
extern const char *s_HUSTR_PLRRED; // = HUSTR_PLRRED;

//
//      AM_map.C
//

//#define AMSTR_FOLLOWON        "Follow Mode ON"
extern const char* s_AMSTR_FOLLOWON; // = AMSTR_FOLLOWON;
//#define AMSTR_FOLLOWOFF       "Follow Mode OFF"
extern const char* s_AMSTR_FOLLOWOFF; // = AMSTR_FOLLOWOFF;

//#define AMSTR_GRIDON  "Grid ON"
extern const char* s_AMSTR_GRIDON; // = AMSTR_GRIDON;
//#define AMSTR_GRIDOFF "Grid OFF"
extern const char* s_AMSTR_GRIDOFF; // = AMSTR_GRIDOFF;

//#define AMSTR_MARKEDSPOT      "Marked Spot"
extern const char* s_AMSTR_MARKEDSPOT; // = AMSTR_MARKEDSPOT;
//#define AMSTR_MARKSCLEARED    "All Marks Cleared"
extern const char* s_AMSTR_MARKSCLEARED; // = AMSTR_MARKSCLEARED;

// CPhipps - automap rotate & overlay
extern const char* s_AMSTR_ROTATEON;
extern const char* s_AMSTR_ROTATEOFF;
extern const char* s_AMSTR_OVERLAYON;
extern const char* s_AMSTR_OVERLAYOFF;

//
//      ST_stuff.C
//

//#define STSTR_MUS             "Music Change"
extern const char* s_STSTR_MUS; // = STSTR_MUS;
//#define STSTR_NOMUS           "IMPOSSIBLE SELECTION"
extern const char* s_STSTR_NOMUS; // = STSTR_NOMUS;
//#define STSTR_DQDON           "Degreelessness Mode On"
extern const char* s_STSTR_DQDON; // = STSTR_DQDON;
//#define STSTR_DQDOFF  "Degreelessness Mode Off"
extern const char* s_STSTR_DQDOFF; // = STSTR_DQDOFF;

//#define STSTR_KFAADDED        "Very Happy Ammo Added"
extern const char* s_STSTR_KFAADDED; // = STSTR_KFAADDED;
//#define STSTR_FAADDED "Ammo (no keys) Added"
extern const char* s_STSTR_FAADDED; // = STSTR_FAADDED;

//#define STSTR_NCON            "No Clipping Mode ON"
extern const char* s_STSTR_NCON; // = STSTR_NCON;
//#define STSTR_NCOFF           "No Clipping Mode OFF"
extern const char* s_STSTR_NCOFF; // = STSTR_NCOFF;

//#define STSTR_BEHOLD  "inVuln, Str, Inviso, Rad, Allmap, or Lite-amp"
extern const char* s_STSTR_BEHOLD; // = STSTR_BEHOLD;
//#define STSTR_BEHOLDX "Power-up Toggled"
extern const char* s_STSTR_BEHOLDX; // = STSTR_BEHOLDX;

//#define STSTR_CHOPPERS        "... doesn't suck - GM"
extern const char* s_STSTR_CHOPPERS; // = STSTR_CHOPPERS;
//#define STSTR_CLEV            "Changing Level..."
extern const char* s_STSTR_CLEV; // = STSTR_CLEV;

//
//      F_Finale.C
//
/*
#define E1TEXT \
"Once you beat the big badasses and\n"\
"clean out the moon base you're supposed\n"\
"to win, aren't you? Aren't you? Where's\n"\
"your fat reward and ticket home? What\n"\
"the hell is this? It's not supposed to\n"\
"end this way!\n"\
"\n" \
"It stinks like rotten meat, but looks\n"\
"like the lost Deimos base.  Looks like\n"\
"you're stuck on The Shores of Hell.\n"\
"The only way out is through.\n"\
"\n"\
"To continue the DOOM experience, play\n"\
"The Shores of Hell and its amazing\n"\
"sequel, Inferno!\n"
*/
extern const char* s_E1TEXT; // = E1TEXT;


/*
#define E2TEXT \
"You've done it! The hideous cyber-\n"\
"demon lord that ruled the lost Deimos\n"\
"moon base has been slain and you\n"\
"are triumphant! But ... where are\n"\
"you? You clamber to the edge of the\n"\
"moon and look down to see the awful\n"\
"truth.\n" \
"\n"\
"Deimos floats above Hell itself!\n"\
"You've never heard of anyone escaping\n"\
"from Hell, but you'll make the bastards\n"\
"sorry they ever heard of you! Quickly,\n"\
"you rappel down to  the surface of\n"\
"Hell.\n"\
"\n" \
"Now, it's on to the final chapter of\n"\
"DOOM! -- Inferno."
*/
extern const char* s_E2TEXT; // = E2TEXT;


/*
#define E3TEXT \
"The loathsome spiderdemon that\n"\
"masterminded the invasion of the moon\n"\
"bases and caused so much death has had\n"\
"its ass kicked for all time.\n"\
"\n"\
"A hidden doorway opens and you enter.\n"\
"You've proven too tough for Hell to\n"\
"contain, and now Hell at last plays\n"\
"fair -- for you emerge from the door\n"\
"to see the green fields of Earth!\n"\
"Home at last.\n" \
"\n"\
"You wonder what's been happening on\n"\
"Earth while you were battling evil\n"\
"unleashed. It's good that no Hell-\n"\
"spawn could have come through that\n"\
"door with you ..."
*/
extern const char* s_E3TEXT; // = E3TEXT;


/*
#define E4TEXT \
"the spider mastermind must have sent forth\n"\
"its legions of hellspawn before your\n"\
"final confrontation with that terrible\n"\
"beast from hell.  but you stepped forward\n"\
"and brought forth eternal damnation and\n"\
"suffering upon the horde as a true hero\n"\
"would in the face of something so evil.\n"\
"\n"\
"besides, someone was gonna pay for what\n"\
"happened to daisy, your pet rabbit.\n"\
"\n"\
"but now, you see spread before you more\n"\
"potential pain and gibbitude as a nation\n"\
"of demons run amok among our cities.\n"\
"\n"\
"next stop, hell on earth!"
*/
extern const char* s_E4TEXT; // = E4TEXT;


// after level 6, put this:

/*
#define C1TEXT \
"YOU HAVE ENTERED DEEPLY INTO THE INFESTED\n" \
"STARPORT. BUT SOMETHING IS WRONG. THE\n" \
"MONSTERS HAVE BROUGHT THEIR OWN REALITY\n" \
"WITH THEM, AND THE STARPORT'S TECHNOLOGY\n" \
"IS BEING SUBVERTED BY THEIR PRESENCE.\n" \
"\n"\
"AHEAD, YOU SEE AN OUTPOST OF HELL, A\n" \
"FORTIFIED ZONE. IF YOU CAN GET PAST IT,\n" \
"YOU CAN PENETRATE INTO THE HAUNTED HEART\n" \
"OF THE STARBASE AND FIND THE CONTROLLING\n" \
"SWITCH WHICH HOLDS EARTH'S POPULATION\n" \
"HOSTAGE."
*/
extern const char* s_C1TEXT; // = C1TEXT;

// After level 11, put this:

/*
#define C2TEXT \
"YOU HAVE WON! YOUR VICTORY HAS ENABLED\n" \
"HUMANKIND TO EVACUATE EARTH AND ESCAPE\n"\
"THE NIGHTMARE.  NOW YOU ARE THE ONLY\n"\
"HUMAN LEFT ON THE FACE OF THE PLANET.\n"\
"CANNIBAL MUTATIONS, CARNIVOROUS ALIENS,\n"\
"AND EVIL SPIRITS ARE YOUR ONLY NEIGHBORS.\n"\
"YOU SIT BACK AND WAIT FOR DEATH, CONTENT\n"\
"THAT YOU HAVE SAVED YOUR SPECIES.\n"\
"\n"\
"BUT THEN, EARTH CONTROL BEAMS DOWN A\n"\
"MESSAGE FROM SPACE: \"SENSORS HAVE LOCATED\n"\
"THE SOURCE OF THE ALIEN INVASION. IF YOU\n"\
"GO THERE, YOU MAY BE ABLE TO BLOCK THEIR\n"\
"ENTRY.  THE ALIEN BASE IS IN THE HEART OF\n"\
"YOUR OWN HOME CITY, NOT FAR FROM THE\n"\
"STARPORT.\" SLOWLY AND PAINFULLY YOU GET\n"\
"UP AND RETURN TO THE FRAY."
*/
extern const char* s_C2TEXT; // = C2TEXT;


// After level 20, put this:

/*
#define C3TEXT \
"YOU ARE AT THE CORRUPT HEART OF THE CITY,\n"\
"SURROUNDED BY THE CORPSES OF YOUR ENEMIES.\n"\
"YOU SEE NO WAY TO DESTROY THE CREATURES'\n"\
"ENTRYWAY ON THIS SIDE, SO YOU CLENCH YOUR\n"\
"TEETH AND PLUNGE THROUGH IT.\n"\
"\n"\
"THERE MUST BE A WAY TO CLOSE IT ON THE\n"\
"OTHER SIDE. WHAT DO YOU CARE IF YOU'VE\n"\
"GOT TO GO THROUGH HELL TO GET TO IT?"
*/
extern const char* s_C3TEXT; // = C3TEXT;


// After level 29, put this:

/*
#define C4TEXT \
"THE HORRENDOUS VISAGE OF THE BIGGEST\n"\
"DEMON YOU'VE EVER SEEN CRUMBLES BEFORE\n"\
"YOU, AFTER YOU PUMP YOUR ROCKETS INTO\n"\
"HIS EXPOSED BRAIN. THE MONSTER SHRIVELS\n"\
"UP AND DIES, ITS THRASHING LIMBS\n"\
"DEVASTATING UNTOLD MILES OF HELL'S\n"\
"SURFACE.\n"\
"\n"\
"YOU'VE DONE IT. THE INVASION IS OVER.\n"\
"EARTH IS SAVED. HELL IS A WRECK. YOU\n"\
"WONDER WHERE BAD FOLKS WILL GO WHEN THEY\n"\
"DIE, NOW. WIPING THE SWEAT FROM YOUR\n"\
"FOREHEAD YOU BEGIN THE LONG TREK BACK\n"\
"HOME. REBUILDING EARTH OUGHT TO BE A\n"\
"LOT MORE FUN THAN RUINING IT WAS.\n"
*/
extern const char* s_C4TEXT; // = C4TEXT;



// Before level 31, put this:

/*
#define C5TEXT \
"CONGRATULATIONS, YOU'VE FOUND THE SECRET\n"\
"LEVEL! LOOKS LIKE IT'S BEEN BUILT BY\n"\
"HUMANS, RATHER THAN DEMONS. YOU WONDER\n"\
"WHO THE INMATES OF THIS CORNER OF HELL\n"\
"WILL BE."
*/
extern const char* s_C5TEXT; // = C5TEXT;


// Before level 32, put this:

/*
#define C6TEXT \
"CONGRATULATIONS, YOU'VE FOUND THE\n"\
"SUPER SECRET LEVEL!  YOU'D BETTER\n"\
"BLAZE THROUGH THIS ONE!\n"
*/
extern const char* s_C6TEXT; // = C6TEXT;


// after map 06

/*
#define P1TEXT  \
"You gloat over the steaming carcass of the\n"\
"Guardian.  With its death, you've wrested\n"\
"the Accelerator from the stinking claws\n"\
"of Hell.  You relax and glance around the\n"\
"room.  Damn!  There was supposed to be at\n"\
"least one working prototype, but you can't\n"\
"see it. The demons must have taken it.\n"\
"\n"\
"You must find the prototype, or all your\n"\
"struggles will have been wasted. Keep\n"\
"moving, keep fighting, keep killing.\n"\
"Oh yes, keep living, too."
*/
extern const char* s_P1TEXT; // = P1TEXT;


// after map 11

/*
#define P2TEXT \
"Even the deadly Arch-Vile labyrinth could\n"\
"not stop you, and you've gotten to the\n"\
"prototype Accelerator which is soon\n"\
"efficiently and permanently deactivated.\n"\
"\n"\
"You're good at that kind of thing."
*/
extern const char* s_P2TEXT; // = P2TEXT;


// after map 20

/*
#define P3TEXT \
"You've bashed and battered your way into\n"\
"the heart of the devil-hive.  Time for a\n"\
"Search-and-Destroy mission, aimed at the\n"\
"Gatekeeper, whose foul offspring is\n"\
"cascading to Earth.  Yeah, he's bad. But\n"\
"you know who's worse!\n"\
"\n"\
"Grinning evilly, you check your gear, and\n"\
"get ready to give the bastard a little Hell\n"\
"of your own making!"
*/
extern const char* s_P3TEXT; // = P3TEXT;

// after map 30

/*
#define P4TEXT \
"The Gatekeeper's evil face is splattered\n"\
"all over the place.  As its tattered corpse\n"\
"collapses, an inverted Gate forms and\n"\
"sucks down the shards of the last\n"\
"prototype Accelerator, not to mention the\n"\
"few remaining demons.  You're done. Hell\n"\
"has gone back to pounding bad dead folks \n"\
"instead of good live ones.  Remember to\n"\
"tell your grandkids to put a rocket\n"\
"launcher in your coffin. If you go to Hell\n"\
"when you die, you'll need it for some\n"\
"final cleaning-up ..."
*/
extern const char* s_P4TEXT; // = P4TEXT;

// before map 31

/*
#define P5TEXT \
"You've found the second-hardest level we\n"\
"got. Hope you have a saved game a level or\n"\
"two previous.  If not, be prepared to die\n"\
"aplenty. For master marines only."
*/
extern const char* s_P5TEXT; // = P5TEXT;

// before map 32

/*
#define P6TEXT \
"Betcha wondered just what WAS the hardest\n"\
"level we had ready for ya?  Now you know.\n"\
"No one gets out alive."
*/
extern const char* s_P6TEXT; // = P6TEXT;


/*
#define T1TEXT \
"You've fought your way out of the infested\n"\
"experimental labs.   It seems that UAC has\n"\
"once again gulped it down.  With their\n"\
"high turnover, it must be hard for poor\n"\
"old UAC to buy corporate health insurance\n"\
"nowadays..\n"\
"\n"\
"Ahead lies the military complex, now\n"\
"swarming with diseased horrors hot to get\n"\
"their teeth into you. With luck, the\n"\
"complex still has some warlike ordnance\n"\
"laying around."
*/
extern const char* s_T1TEXT; // = T1TEXT;


/*
#define T2TEXT \
"You hear the grinding of heavy machinery\n"\
"ahead.  You sure hope they're not stamping\n"\
"out new hellspawn, but you're ready to\n"\
"ream out a whole herd if you have to.\n"\
"They might be planning a blood feast, but\n"\
"you feel about as mean as two thousand\n"\
"maniacs packed into one mad killer.\n"\
"\n"\
"You don't plan to go down easy."
*/
extern const char* s_T2TEXT; // = T2TEXT;


/*
#define T3TEXT \
"The vista opening ahead looks real damn\n"\
"familiar. Smells familiar, too -- like\n"\
"fried excrement. You didn't like this\n"\
"place before, and you sure as hell ain't\n"\
"planning to like it now. The more you\n"\
"brood on it, the madder you get.\n"\
"Hefting your gun, an evil grin trickles\n"\
"onto your face. Time to take some names."
*/
extern const char* s_T3TEXT; // = T3TEXT;

/*
#define T4TEXT \
"Suddenly, all is silent, from one horizon\n"\
"to the other. The agonizing echo of Hell\n"\
"fades away, the nightmare sky turns to\n"\
"blue, the heaps of monster corpses start \n"\
"to evaporate along with the evil stench \n"\
"that filled the air. Jeeze, maybe you've\n"\
"done it. Have you really won?\n"\
"\n"\
"Something rumbles in the distance.\n"\
"A blue light begins to glow inside the\n"\
"ruined skull of the demon-spitter."
*/
extern const char* s_T4TEXT; // = T4TEXT;


/*
#define T5TEXT \
"What now? Looks totally different. Kind\n"\
"of like King Tut's condo. Well,\n"\
"whatever's here can't be any worse\n"\
"than usual. Can it?  Or maybe it's best\n"\
"to let sleeping gods lie.."
*/
extern const char* s_T5TEXT; // = T5TEXT;


/*
#define T6TEXT \
"Time for a vacation. You've burst the\n"\
"bowels of hell and by golly you're ready\n"\
"for a break. You mutter to yourself,\n"\
"Maybe someone else can kick Hell's ass\n"\
"next time around. Ahead lies a quiet town,\n"\
"with peaceful flowing water, quaint\n"\
"buildings, and presumably no Hellspawn.\n"\
"\n"\
"As you step off the transport, you hear\n"\
"the stomp of a cyberdemon's iron shoe."
*/
extern const char* s_T6TEXT; // = T6TEXT;

//
// Character cast strings F_FINALE.C
//
//#define CC_ZOMBIE     "ZOMBIEMAN"
extern const char* s_CC_ZOMBIE; // = CC_ZOMBIE;
//#define CC_SHOTGUN    "SHOTGUN GUY"
extern const char* s_CC_SHOTGUN; // = CC_SHOTGUN;
//#define CC_HEAVY      "HEAVY WEAPON DUDE"
extern const char* s_CC_HEAVY; // = CC_HEAVY;
//#define CC_IMP        "IMP"
extern const char* s_CC_IMP; // = CC_IMP;
//#define CC_DEMON      "DEMON"
extern const char* s_CC_DEMON; // = CC_DEMON;
//#define CC_LOST       "LOST SOUL"
extern const char* s_CC_LOST; // = CC_LOST;
//#define CC_CACO       "CACODEMON"
extern const char* s_CC_CACO; // = CC_CACO;
//#define CC_HELL       "HELL KNIGHT"
extern const char* s_CC_HELL; // = CC_HELL;
//#define CC_BARON      "BARON OF HELL"
extern const char* s_CC_BARON; // = CC_BARON;
//#define CC_ARACH      "ARACHNOTRON"
extern const char* s_CC_ARACH; // = CC_ARACH;
//#define CC_PAIN       "PAIN ELEMENTAL"
extern const char* s_CC_PAIN; // = CC_PAIN;
//#define CC_REVEN      "REVENANT"
extern const char* s_CC_REVEN; // = CC_REVEN;
//#define CC_MANCU      "MANCUBUS"
extern const char* s_CC_MANCU; // = CC_MANCU;
//#define CC_ARCH       "ARCH-VILE"
extern const char* s_CC_ARCH; // = CC_ARCH;
//#define CC_SPIDER     "THE SPIDER MASTERMIND"
extern const char* s_CC_SPIDER; // = CC_SPIDER;
//#define CC_CYBER      "THE CYBERDEMON"
extern const char* s_CC_CYBER; // = CC_CYBER;
//#define CC_HERO       "OUR HERO"
extern const char* s_CC_HERO; // = CC_HERO;

// Ty 03/30/98 - new substitutions for background textures during int screens
// char*        bgflatE1 = "FLOOR4_8";
extern const char* bgflatE1;
// char*        bgflatE2 = "SFLR6_1";
extern const char* bgflatE2;
// char*        bgflatE3 = "MFLR8_4";
extern const char* bgflatE3;
// char*        bgflatE4 = "MFLR8_3";
extern const char* bgflatE4;

// char*        bgflat06 = "SLIME16";
extern const char* bgflat06;
// char*        bgflat11 = "RROCK14";
extern const char* bgflat11;
// char*        bgflat20 = "RROCK07";
extern const char* bgflat20;
// char*        bgflat30 = "RROCK17";
extern const char* bgflat30;
// char*        bgflat15 = "RROCK13";
extern const char* bgflat15;
// char*        bgflat31 = "RROCK19";
extern const char* bgflat31;

// char*        bgcastcall = "BOSSBACK"; // panel behind cast call
extern const char* bgcastcall;

// ignored if blank, general purpose startup announcements
// char*        startup1 = "";
extern const char* startup1;
// char*        startup2 = "";
extern const char* startup2;
// char*        startup3 = "";
extern const char* startup3;
// char*        startup4 = "";
extern const char* startup4;
// char*        startup5 = "";
extern const char* startup5;

// from g_game.c, prefix for savegame name like "boomsav"
extern const char* savegamename;

void D_BuildBEXTables(void);

#endif
