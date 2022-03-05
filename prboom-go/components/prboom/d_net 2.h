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
 *   Networking stuff.
 *
 *-----------------------------------------------------------------------------*/


#ifndef __D_NET__
#define __D_NET__

#include "d_player.h"


#ifdef __GNUG__
#pragma interface
#endif


//
// Network play related stuff.
// There is a data struct that stores network
//  communication related stuff, and another
//  one that defines the actual packets to
//  be transmitted.
//

#define DOOMCOM_ID              0x12345678l

// Max computers/players in a game.
#define MAXNETNODES             8


typedef enum
{
    CMD_SEND    = 1,
    CMD_GET     = 2

} command_t;


//
// Network packet data.
//
typedef struct
{
    // High bit is retransmit request.
    unsigned            checksum;
    // Only valid if NCMD_RETRANSMIT.
    byte                retransmitfrom;

    byte                starttic;
    byte                player;
    byte                numtics;
    ticcmd_t            cmds[BACKUPTICS];

} doomdata_t;

//
// Startup packet difference
// SG: 4/12/98
// Added so we can send more startup data to synch things like
// bobbing, recoil, etc.
// this is just mapped over the ticcmd_t array when setup packet is sent
//
// Note: the original code takes care of startskill, deathmatch, nomonsters
//       respawn, startepisode, startmap
// Note: for phase 1 we need to add monsters_remember, variable_friction,
//       weapon_recoil, allow_pushers, over_under, player_bobbing,
//       fastparm, demo_insurance, and the rngseed
//Stick all options into bytes so we don't need to mess with bitfields
//WARNING: make sure this doesn't exceed the size of the ticcmds area!
//sizeof(ticcmd_t)*BACKUPTICS
//This is the current length of our extra stuff
//
//killough 5/2/98: this should all be replaced by calls to G_WriteOptions()
//and G_ReadOptions(), which were specifically designed to set up packets.
//By creating a separate struct and functions to read/write the options,
//you now have two functions and data to maintain instead of just one.
//If the array in g_game.c which G_WriteOptions()/G_ReadOptions() operates
//on, is too large (more than sizeof(ticcmd_t)*BACKUPTICS), it can
//either be shortened, or the net code needs to divide it up
//automatically into packets. The STARTUPLEN below is non-portable.
//There's a portable way to do it without having to know the sizes.

#define STARTUPLEN 12
typedef struct
{
  byte monsters_remember;
  byte variable_friction;
  byte weapon_recoil;
  byte allow_pushers;
  byte over_under;
  byte player_bobbing;
  byte fastparm;
  byte demo_insurance;
  unsigned long rngseed;
  char filler[sizeof(ticcmd_t)*BACKUPTICS-STARTUPLEN];
} startup_t;

typedef enum {
  // Leave space, so low values corresponding to normal netgame setup packets can be ignored
  nm_plcolour = 3,
  nm_savegamename = 4,
} netmisctype_t;

typedef struct
{
  netmisctype_t type;
  size_t len;
  byte value[sizeof(ticcmd_t)*BACKUPTICS - sizeof(netmisctype_t) - sizeof(size_t)];
} netmisc_t;

typedef struct
{
    // Supposed to be DOOMCOM_ID?
    long                id;

    // DOOM executes an int to execute commands.
    short               intnum;
    // Communication between DOOM and the driver.
    // Is CMD_SEND or CMD_GET.
    short               command;
    // Is dest for send, set by get (-1 = no packet).
    short               remotenode;

    // Number of bytes in doomdata to be sent
    short               datalength;

    // Info common to all nodes.
    // Console is allways node 0.
    short               numnodes;
    // Flag: 1 = no duplication, 2-5 = dup for slow nets.
    short               ticdup;
    // Flag: 1 = send a backup tic in every packet.
    short               extratics;
    // Flag: 1 = deathmatch.
    short               deathmatch;
    // Flag: -1 = new game, 0-5 = load savegame
    short               savegame;
    short               episode;        // 1-3
    short               map;            // 1-9
    short               skill;          // 1-5

    // Info specific to this node.
    short               consoleplayer;
    short               numplayers;

    // These are related to the 3-display mode,
    //  in which two drones looking left and right
    //  were used to render two additional views
    //  on two additional computers.
    // Probably not operational anymore.
    // 1 = left, 0 = center, -1 = right
    short               angleoffset;
    // 1 = drone
    short               drone;

    // The packet data to be sent.
    doomdata_t          data;

} doomcom_t;

// Create any new ticcmds and broadcast to other players.
#ifdef HAVE_NET
void NetUpdate (void);
#else
void D_BuildNewTiccmds(void);
#endif

//? how many ticks to run?
void TryRunTics (void);

// CPhipps - move to header file
void D_InitNetGame (void); // This does the setup
void D_CheckNetGame(void); // This waits for game start

// CPhipps - misc info broadcast
void D_NetSendMisc(netmisctype_t type, size_t len, void* data);

// CPhipps - ask server for a wad file we need
boolean D_NetGetWad(const char* name);

// Netgame stuff (buffers and pointers, i.e. indices).
extern  doomcom_t  *doomcom;
extern  doomdata_t *netbuffer;  // This points inside doomcom.

#endif
