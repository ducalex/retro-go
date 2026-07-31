/*
Copyright (C) 1994-1995 Apogee Software, Ltd.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// rottnet.h
#ifndef rottnet_public
#define rottnet_public

#include "develop.h"

#ifdef DOS
#define PEL_WRITE_ADR   0x3c8
#define PEL_DATA        0x3c9

#define I_ColorBlack(r,g,b) {outp(PEL_WRITE_ADR,0);outp(PEL_DATA,r);outp(PEL_DATA,g);outp(PEL_DATA,b);};
#endif

#define	MAXNETNODES		14			// max computers in a game

#if ( SHAREWARE == 1 )
   #define  MAXPLAYERS     5        // 5 players max + drones
#else
   #define	MAXPLAYERS		11			// 11 players max + drones
#endif

#define	CMD_SEND	   1
#define	CMD_GET		2
#define  CMD_OUTQUEBUFFERSIZE 3
#define  CMD_INQUEBUFFERSIZE  4

#define	ROTTCOM_ID		0x12345678l

#define  MAXPACKETSIZE 2048
#define	MAXCOMBUFFERSIZE 2048

#if __WATCOMC__
#pragma pack (1)
#endif

typedef struct
{
	short	intnum;			// ROTT executes an int to send commands

// communication between ROTT and the driver
	short	command;	    	// CMD_SEND or CMD_GET
	short	remotenode;		// dest for send, set by get (-1 = no packet)
	short	datalength;		// bytes in rottdata to be sent / bytes read

// info specific to this node
	short	consoleplayer;	// 0-3 = player number
	short	numplayers;		// 1-4
   short   client;         // 0 = server 1 = client
   short   gametype;       // 0 = modem  1 = network
   short   ticstep;        // 1 for every tic 2 for every other tic ...
   short   remoteridicule; // 0 = remote ridicule is off 1= rr is on

// packet data to be sent
	char	data[MAXPACKETSIZE];
} rottcom_t;

#if __WATCOMC__
#pragma pack (4)
#endif

#define  MODEM_GAME   0
#define	NETWORK_GAME 1

#define	ROTTLAUNCHER ("ROTT.EXE")

#if defined(DOS) && (__WATCOMC__ == 0)

extern   rottcom_t   rottcom;
extern   boolean     pause;

void ShutdownROTTCOM ( void );
int  CheckParm (char *check);
void LaunchROTT (void);
void NetISR (void);
long GetVector (void);

#else

extern   rottcom_t   * rottcom;

#endif

#endif
