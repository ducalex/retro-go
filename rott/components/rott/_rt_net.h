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
#ifndef _rt_net_private
#define _rt_net_private

#define DEMOBUFFSIZE 50000

#define FASTSPEED (0xB000)

#define SETUPTIME     15

#define PlayerCommand(player,command)    \
        (PlayerCmds[(player)]->Commands[(command)])

#define ClientCommand(player,command)    \
        (ClientCmds[(player)]->Commands[(command)])

#define LocalCommand(command)    \
        (LocalCmds->Commands[(command)])

#define ServerCommand(command)    \
        (ServerCmds->Commands[(command)])

#define CommandAddress(time)    \
        (((time)-controlupdatestartedtime) & (MAXCMDS-1))

#define NextLocalCommand()    \
        (LocalCommand(CommandAddress(controlupdatetime)))

#define NextServerCommand()    \
        (ServerCommand(CommandAddress(serverupdatetime)))

#define ClientTimeCommand(which,whichtime)    \
        (ClientCmds[(which)]->Commands[(CommandAddress((whichtime)))])

#define ServerCommandStatus(whichtime)    \
        (CommandState[0]->CommandStates[(CommandAddress((whichtime)))])

#define ServerCommandNumberStatus(command)    \
        (CommandState[0]->CommandStates[(command)])

#define ClientCommandStatus(which, whichtime)    \
        (CommandState[(which+1)]->CommandStates[(CommandAddress((whichtime)))])

#define ClientCommandNumberStatus(which, command)    \
        (CommandState[(which+1)]->CommandStates[(command)])

/*
#define PacketAddress(time)    \
        ((time) & (MAXCMDS-1))
*/

#define PacketAddress(time)    \
        (time)


#define NETWORKTIMEOUT  (VBLCOUNTER/3)
#define MODEMTIMEOUT    (VBLCOUNTER/2)
#define SERVERTIMEOUT    (VBLCOUNTER<<3)

#define MAXPOLLTICS 3


typedef enum {
   scfp_nodata,
   scfp_gameready,
   scfp_data,
   scfp_done
} setupcheckforpacketstate;

typedef enum {
   cs_ready,
   cs_notarrived,
   cs_fixing
} en_CommandStatus;

typedef enum {
   player_ingame,
   player_quitgame,
   player_leftgame
} en_playerstatus;

void PreparePacket (MoveType * pkt);
int  GetPacketSize (void * pkt);
void SendPacket (void * pkt, int dest);
void GetRemotePacket (int from, int delay);
void ResendLocalPackets  (int time, int dest, int numpackets);
void ResendServerPackets (int time, int dest, int numpackets);
void ResendPacket (void * pkt, int dest);
void AddClientDelta (void * pkt, int src);
void FixupPacket (void * pkt, int src);
void ProcessPacket (void * pkt, int src);
void AddServerPacket(void * pkt, int src);
void AddClientPacket (void * pkt, int src);
void AddPacket (void * pkt, int src);
void RequestPacket (int time, int dest, int numpackets);
boolean AllPlayersReady ( void );
boolean AreClientsReady ( void );
boolean IsServerCommandReady ( int time );
void UpdatePlayerObj ( int player );
void AddServerSubPacket(COM_ServerHeaderType * serverpkt);
void AddSubPacket (void * pkt, int src);


#endif
