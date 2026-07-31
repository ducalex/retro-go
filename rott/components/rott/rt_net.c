#include "rt_def.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include <rg_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "rt_main.h"
#include "rt_net.h"
#include "rt_com.h"
#include "_rt_net.h"
#include "rt_actor.h"
#include "rt_playr.h"
#include "isr.h"
#include "z_zone.h"
#include "develop.h"
#include "rottnet.h"
#include "rt_msg.h"
#include "rt_sound.h"
#include "rt_menu.h"
#include "rt_util.h"
#include "rt_rand.h"
#include "rt_game.h"
#include "rt_draw.h"
#include "myprint.h"
#include "rt_debug.h"
#include "rt_view.h"
#include "rt_battl.h"
#include "rt_dmand.h"
//MED
#include "memcheck.h"

#if (SYNCCHECK == 1)
int            lastsynccheck;
COM_CheckSyncType PlayerSync[MAXPLAYERS];
#endif


CommandType * LocalCmds;
CommandType * ServerCmds;


int        controlupdatestartedtime=-1;
int        controlupdatetime=-1;
int        serverupdatetime=-1;
int        controlupdatestarted=0;
boolean    GamePaused=false;

boolean    modemgame;
boolean    networkgame;
int        numplayers;
int        server;
boolean    IsServer;
boolean    standalone;
boolean    restartgame=false;
boolean    respawnactive=false;
boolean    playerdead=false;
boolean    controlschanged=true;
boolean    battlegibs=false;
boolean    remoteridicule = false;
/*
=============================================================================

					LOCAL FUNCTION PROTOTYPES and VARIABLES

=============================================================================
*/
boolean  demorecord,
			demoplayback;
byte     *demoptr,
			*lastdemoptr,
         *demobuffer=NULL;
boolean  demodone = false;
int      predemo_violence = -1;
int oldmomx;
int oldmomy;
int oldspdang;

static boolean GameCommandsStarted=false;

static int oldcontrolbuf[3];
static int oldbuttonbits;
static CommandType * PlayerCmds[MAXPLAYERS];
static CommandType * ClientCmds[MAXPLAYERS];

static boolean GotPlayersDesc[MAXPLAYERS];
static boolean PlayersReady[MAXPLAYERS];
static int     LastCommandTime[MAXPLAYERS];

static CommandStatusType * CommandState[MAXPLAYERS+1];

static boolean InProcessServer=false;
static int lastcontrolupdatetime;
static int largesttime;
static int PlayerStatus[MAXPLAYERS];
//static int syncservertime;
//static boolean FixingPackets;
static int controldivisor=1;
static int nextupdatetime;
static boolean UpdateServer=true;

void CheckForPacket ( void );
void PrepareLocalPacket ( void );
void SendSyncCheckPacket ( void );
void AddModemSubPacket(void * incoming);
void SetPlayerDescription( void * pkt );
void UpdateDemoPlayback (int time);
int GetTypeSize (int type);
int MaxSpeedForCharacter(playertype *pstate);

/*
=============================================================================

						  Game Command Section

=============================================================================
*/

//****************************************************************************
//
// ComError ()
//
//****************************************************************************

#define ComError SoftError
#if 0
void ComError (char *error, ...)
{
#if 0
	va_list	argptr;
#endif

   SoftError(error);
#if 0
   if (standalone==true)
      {
	   va_start (argptr, error);
      vprintf (error, argptr);
	   va_end (argptr);
      }
#endif
}
#endif

//****************************************************************************
//
// ConsoleIsServer()
//
//****************************************************************************
boolean ConsoleIsServer ( void )
{
   if (modemgame==true)
      {
      if (networkgame==true)
         {
         if (rottcom->client==0)
            {
            return true;
            }
         }
      }
   return false;
}

//****************************************************************************
//
// GamePacketSize()
//
//****************************************************************************
int GamePacketSize( void )
{
   if ((remoteridicule == true) || (ConsoleIsServer() == true))
      {
      return GetTypeSize(COM_SOUNDANDDELTA);
      }
   else
      return GetTypeSize(COM_TEXT);
}

//****************************************************************************
//
// InitializeGameCommands()
//
//****************************************************************************
void InitializeGameCommands( void )
{
   int i;
   int j;

	// default to player 0

   if (GameCommandsStarted==true)
      return;

   GameCommandsStarted=true;

   if ((modemgame==true))
      controldivisor=rottcom->ticstep;

   standalone=false;
   IsServer=false;

   if (modemgame==true)
      {
		consoleplayer=rottcom->consoleplayer;

      if (networkgame==true)
         {
         if (rottcom->client==0)
            {
            IsServer=true;
            // turn it on absolutely for the server
            remoteridicule = true;
				if (consoleplayer==0)
               standalone=true;
            }
			if (consoleplayer>0)
				consoleplayer--; // playernumber fixup
         }
      }

   if (standalone==false)
      {
      int size;

      size = GamePacketSize ();

              for (i=0;i<numplayers;i++)
              {
              PlayerCmds[i]=(CommandType *)SafeLevelMalloc(sizeof(CommandType));
              byte *block = SafeLevelMalloc(MAXCMDS * size);
              for (j=0;j<MAXCMDS;j++)
              {
              PlayerCommand(i,j) = block + (j * size);
              }
              vTaskDelay(1);
              }

      }


   // allocate local commands

   LocalCmds=(CommandType *)SafeLevelMalloc(sizeof(CommandType));
   {
      int size = GamePacketSize();
      byte *block = SafeLevelMalloc(MAXCMDS * size);
      for (j=0;j<MAXCMDS;j++)
      {
      LocalCommand(j) = block + (j * size);
      memset(LocalCommand(j),COM_DELTANULL,size);
      }
   }
   vTaskDelay(1);
   CommandState[0]=(CommandStatusType *)SafeLevelMalloc(sizeof(CommandStatusType));

   if (modemgame==true)
      {
      for (i=0;i<numplayers;i++)
         {
         PlayerStatus[i]=player_ingame;
         }
      if (networkgame==true)
         {
         server=1;

         // initialize the Server

         if (IsServer==true)
            {
            server=0;
            ServerCmds=(CommandType *)SafeMalloc(sizeof(CommandType));
            for (j=0;j<MAXCMDS;j++)
               {
               int size;
               size=( (numplayers * GetTypeSize(COM_TEXT)) +
                      GetTypeSize(COM_SOUNDANDDELTA) +
                      sizeof(COM_ServerHeaderType) -
                      sizeof(byte)
                    );
               ServerCommand(j)=SafeMalloc( size );
               memset(ServerCommand(j),COM_DELTANULL,size);
               }
            for (i=1;i<=numplayers;i++)
               {
               CommandState[i]=(CommandStatusType *)
                               SafeMalloc(sizeof(CommandStatusType));
               }
				for (i=0;i<numplayers;i++)
               {
               ClientCmds[i]=(CommandType *)SafeMalloc(sizeof(CommandType));
               for (j=0;j<MAXCMDS;j++)
                  {
                  int size;

                  size=GetTypeSize(COM_SOUNDANDDELTA);
                  ClientCommand(i,j)=SafeMalloc(size);
                  memset(ClientCommand(i,j),COM_DELTANULL,size);
                  }
               }
            }
         }
		else // must be a two player game
         {
			server=consoleplayer^1;
         }
      }
#if 0
#if (DEVELOPMENT == 1)
      if (IsServer)
         ComError("I am the server\n");
      ComError("consoleplayer=%ld\n",consoleplayer);
      ComError("server=%ld mynumber=%ld\n",server,consoleplayer);
#endif
#endif
}


//****************************************************************************
//
// ShutdownGameCommands()
//
//****************************************************************************
void ShutdownGameCommands( void )
{
   int i;
   int j;

   if (GameCommandsStarted==false)
      return;

   GameCommandsStarted=false;

	// free up playercmds;
	if (standalone==false)
	{
	   for (i=0;i<numplayers;i++)
	   {
	   if ((i & 1) == 0) vTaskDelay(1);
	   if (PlayerCmds[i])

	    {
	    if (PlayerCommand(i,0))
	       {
	       SafeFree(PlayerCommand(i,0));
	       }
	    for (j=0;j<MAXCMDS;j++)
	       {
	       PlayerCommand(i,j)=NULL;
	       }
	    SafeFree( PlayerCmds[i] );
	    PlayerCmds[i]=NULL;
	    }
	 }
	}
   // free up command status

   SafeFree(CommandState[0]);
   CommandState[0]=NULL;

   if (modemgame==true)
      {

      // free up local commands

      if (LocalCmds)
         {
         if (LocalCommand(0))
            {
            SafeFree(LocalCommand(0));
            }
         for (j=0;j<MAXCMDS;j++)
            {
            LocalCommand(j)=NULL;
            }
         SafeFree(LocalCmds);
         LocalCmds=NULL;
         }


      // free up Server

      if (networkgame==true)
         {
         if (IsServer==true)
            {
            for (j=0;j<MAXCMDS;j++)
               {
               if (ServerCommand(j))
                  {
                  SafeFree(ServerCommand(j));
                  ServerCommand(j)=NULL;
                  }
               }
            SafeFree(ServerCmds);
            ServerCmds=NULL;
            for (i=1;i<=numplayers;i++)
               {
               SafeFree(CommandState[i]);
               CommandState[i]=NULL;
               }
				for (i=0;i<numplayers;i++)
               {
               for (j=0;j<MAXCMDS;j++)
                  {
                  if (ClientCommand(i,j))
                     {
                     SafeFree(ClientCommand(i,j));
                     ClientCommand(i,j)=NULL;
                     }
                  }
               SafeFree( ClientCmds[i] );
               ClientCmds[i]=NULL;
               }
            }
         }
      }
}




/*
=============================================================================

						  Client Controls Section

=============================================================================
*/


//****************************************************************************
//
// ShutdownClientControls ()
//
//****************************************************************************

void ShutdownClientControls ( void )
{
   int i;
#if (DEVELOPMENT == 1)
   SoftError ("LARGEST time difference=%ld\n",largesttime);
#endif
   controlupdatestarted=0;
   for (i=0;i<numplayers;i++)
       {
       if (PlayerStatus[i] == player_leftgame)
          PlayerStatus[i]=player_ingame;
       }
}


//****************************************************************************
//
// StartupClientControls ()
//
//****************************************************************************

void StartupClientControls ( void )
{
   int i,j;

   if (controlupdatestarted==1)
      return;

   controlupdatestarted=1;

   memset(oldcontrolbuf,-1,sizeof(oldcontrolbuf));
   oldbuttonbits=-1;
   controlschanged=true;

   INL_GetMouseDelta(&i,&i);


   locplayerstate->dmomx = 0;
   locplayerstate->dmomy = 0;
   locplayerstate->angle = 0;
   locplayerstate->topspeed=MaxSpeedForCharacter(locplayerstate);


   CalcTics();
   CalcTics();

//   FixingPackets=false;

	memset (controlbuf, 0, sizeof (controlbuf));
   buttonbits = 0;
   lastpolltime=-1;
   IN_ClearKeyboardQueue ();

   if (modemgame==true)
      {
      controlupdatetime=controlsynctime+(VBLCOUNTER*2);
      SoftError("Controls started at %d\n",controlupdatetime);
      }
   else if (demoplayback || demorecord)
      {
      ISR_SetTime(20);
      oldtime = 20;
      controlupdatetime=20;
      }
   else
      controlupdatetime=GetTicCount();

   controlupdatetime-=(controlupdatetime%controldivisor);

   serverupdatetime=controlupdatetime;
   oldpolltime=controlupdatetime;
   nextupdatetime=oldpolltime;
#if (SYNCCHECK == 1)
   lastsynccheck=oldpolltime+CHECKSYNCTIME;
#endif
   controlupdatestartedtime=controlupdatetime;

   for( j = 0; j < numplayers; j++ )
      {
      memset( PLAYERSTATE[ j ].buttonheld, 0,
         sizeof( PLAYERSTATE[ j ].buttonheld ) );
      memset( PLAYERSTATE[ j ].buttonstate, 0,
         sizeof( PLAYERSTATE[ j ].buttonstate ) );
      }

   for (i=0;i<MAXCMDS;i++)
      {
      ServerCommandNumberStatus( i ) = cs_notarrived;
      }

   LastCommandTime[0]=controlupdatetime-controldivisor;
   if (IsServer==true)
      {
      int size;

      UpdateServer=true;
      size=( (numplayers * GetTypeSize(COM_TEXT)) +
               GetTypeSize(COM_SOUNDANDDELTA) +
               sizeof(COM_ServerHeaderType) -
               sizeof(byte)
            );

      for (j=0;j<numplayers;j++)
         {
	      for (i=0;i<MAXCMDS;i++)
            {
            ClientCommandNumberStatus( j , i ) = cs_notarrived;
            }
         LastCommandTime[j]=controlupdatetime-controldivisor;
         }
      for (i=0;i<MAXCMDS;i++)
         memset(ServerCommand(i),COM_DELTANULL,size);
      }
   else if (modemgame==true)
      {
      int nump;

      nump=numplayers;
      if (nump<2) nump=2;

      for (i=0;i<nump;i++)
         {
         LastCommandTime[i]=controlupdatetime-controldivisor;
         }
      }


#if (DEVELOPMENT == 1)
//   ComError("StartupClientControls: GetTicCount()=%ld oldtime=%ld controlupdatetime=%ld\n",GetTicCount(),oldtime,controlupdatetime);
#endif

   if ((demoplayback==false) && (standalone==false))
      {
      if (modemgame==true)
         {
         while (GetTicCount()<(controlupdatetime-10))
            {
            CalcTics();
            }
         }
      lastcontrolupdatetime=GetTicCount();
      largesttime=0;
      PollControls();
      }
   if (standalone==true)
      printf("Packet Server started\n");
}



//****************************************************************************
//
// UpdateClientControls ()
//
//****************************************************************************

static boolean InUCC=false;
void UpdateClientControls ( void )
{
   int time;
//   int delta;

   if (controlupdatestarted==0)
      return;

   if (InUCC)
      return;
   else
      InUCC = true;

   wami(6);

#if 0

   delta=GetTicCount()-lastcontrolupdatetime;
   if (delta>largesttime)
      {
      if (delta>10)
         largesttime=delta;
      largesttime=delta;
      }

#endif
   lastcontrolupdatetime=GetTicCount();

   if (standalone==false)
      {
      time=GetTicCount();

      // if we are a fixing the current packet stop update of deltas
      // in non-network games.
      if (
           (networkgame == false) &&
           (ServerCommandStatus(oldpolltime)==cs_fixing)
         )
         {
         time=controlupdatetime-controldivisor;
         }

      while (time>=controlupdatetime)
         {
         MoveType * Delta;
         boolean soundready;

         soundready = SD_SoundDataReady();

         if (demoplayback==true)
            {
            UpdateDemoPlayback(controlupdatetime);
            }
//         else
//            {
//            PollControls();
//            }

         if (
             (memcmp(&controlbuf[0],&oldcontrolbuf[0],sizeof(controlbuf))!=0) ||
             (buttonbits!=oldbuttonbits)
            )
            {
            controlschanged=true;
            memcpy(&oldcontrolbuf[0],&controlbuf[0],sizeof(controlbuf));
            oldbuttonbits=buttonbits;
            }
         else
            {
            controlschanged=false;
            }

         if ((controlschanged==false) && (soundready==false))
            {
            NullMoveType * NullDelta;

            NullDelta=(NullMoveType *)NextLocalCommand();
            NullDelta->type=COM_DELTANULL;
            }
         else
            {
            Delta=(MoveType *)NextLocalCommand();
            Delta->type=COM_DELTA;
            Delta->momx=(controlbuf[0]>>1);
            Delta->momy=(controlbuf[1]>>1);
            Delta->dangle=controlbuf[2]>>11;
            Delta->buttons=buttonbits;

            // See if we need to update sound packet

            if (soundready==true)
               {
               COM_SoundType * sndpkt;
               recordstate status;

               if (remoteridicule == false)
                  Error("Attempt to record Remote Ridicule without adequate storage");
               sndpkt=(COM_SoundType *)Delta->Sounddata;

               // Turn the packet into a COM_SOUNDANDDELTA packet

               Delta->type=COM_SOUNDANDDELTA;
               status = SD_GetSoundData ( &(sndpkt->data[0]),
                                          COM_SOUND_BUFFERSIZE );
               switch (status)
                  {
                  case rs_nodata:
                     Delta->type=COM_DELTA;
                     break;
                  case rs_newsound:
                     sndpkt->type=COM_SOUND_START_TRANSMISSION;
                     break;
                  case rs_endsound:
                     sndpkt->type=COM_SOUND_END_TRANSMISSION;
                     break;
                  case rs_data:
                     sndpkt->type=COM_SOUND_NORMAL_TRANSMISSION;
                     break;
                  default:
                     Error("Illegal return value for SD_GetSoundData");
                     break;
                  }
               }
            if (demorecord==true)
               RecordDemoCmd();
            }
         PrepareLocalPacket();

         if (
              (controlupdatetime != -1) &&
              (controlupdatetime > (lastpolltime+MAXPOLLTICS)) &&
              (demoplayback==false)
            )
            {
            controlbuf[0] = controlbuf[1] = controlbuf[2] = 0;
            }
         }
      }
   if (modemgame==true)
      {
      CheckForPacket ();
      }

   if ((standalone == false) && (IsServer==true) && (UpdateServer==true))
      ProcessServer();

// take out
   if (modemgame==true)
      {
//#if (DEVELOPMENT == 1)
      if (PanicPressed==true)
         {
         Error("Game Aborted. Scroll Lock pressed\n");
         }
//#endif
		if (Keyboard[sc_Insert] && Keyboard[sc_Q])
         Error("Game Aborted. Insert->Q pressed\n");
      }

   InUCC = false;

   waminot();
}

//****************************************************************************
//
// PlayerInGame()
//
//****************************************************************************
boolean PlayerInGame ( int p )
{
   if (PlayerStatus[p]!=player_ingame)
      return false;
   return true;
}

/*
=============================================================================

						  Packet Section

=============================================================================
*/

//****************************************************************************
//
// CheckForPacket()
//
//****************************************************************************
void CheckForPacket ( void )
{
   wami(7);
   while (ReadPacket()==true)
      {
      if (badpacket==0)
         {
         ProcessPacket(&ROTTpacket[0], rottcom->remotenode);
         #if (DEVELOPMENT == 1)
//         ComError("CheckForPacket: from=%ld\n",rottcom->remotenode);
         #endif
         }
      else
         RequestPacket (LastCommandTime[rottcom->remotenode]+controldivisor, rottcom->remotenode, controldivisor);
      }
   waminot();
}


//****************************************************************************
//
// AddRemoteRidiculeCommand()
//
//****************************************************************************
void AddRemoteRidiculeCommand ( int player, int towho, int num )
{
   ((COM_RemoteRidiculeType *)NextLocalCommand())->type=COM_REMRID;
   ((COM_RemoteRidiculeType *)NextLocalCommand())->num=num;
   ((COM_RemoteRidiculeType *)NextLocalCommand())->player=player;
   ((COM_RemoteRidiculeType *)NextLocalCommand())->towho=towho;

   PrepareLocalPacket();
}

//****************************************************************************
//
// ProcessRemoteRidicule()
//
//****************************************************************************
void ProcessRemoteRidicule ( void * pkt )
   {
   COM_RemoteRidiculeType * remrot;
   char name[ 50 ];
   int from;
   int who;

   remrot = (COM_RemoteRidiculeType *)pkt;
   from   = remrot->player;
   who    = remrot->towho;
   if ( ( who == consoleplayer ) || ( who == MSG_DIRECTED_TO_ALL ) ||
      ( ( who == MSG_DIRECTED_TO_TEAM ) && ( BATTLE_Team[ from ] ==
      BATTLE_Team[ consoleplayer ] ) ) )
      {
      strcpy( name, "(� RR from " );
      strcat( name, PLAYERSTATE[from].codename );
      strcat( name, ")" );
      AddMessage( name, MSG_REMOTERIDICULE );

      SD_Play( SD_REMOTEM1SND + remrot->num );
      }
   }

//****************************************************************************
//
// AddEndGameCommand()
//
//****************************************************************************
void AddEndGameCommand ( void )
{
   ((COM_EndGameType *)NextLocalCommand())->type=COM_ENDGAME;

   PrepareLocalPacket();
}

//****************************************************************************
//
// AddGameEndCommand()
//
//****************************************************************************
void AddGameEndCommand ( void )
{
   ((COM_GameEndType *)NextLocalCommand())->type=COM_GAMEEND;

   PrepareLocalPacket();
}

//****************************************************************************
//
// AddQuitCommand()
//
//****************************************************************************
void AddQuitCommand ( void )
{
   ((COM_QuitType *)NextLocalCommand())->type=COM_QUIT;
   PrepareLocalPacket();
}

//****************************************************************************
//
// AddExitCommand()
//
//****************************************************************************
void AddExitCommand ( void )
{
   ((COM_ExitType *)NextLocalCommand())->type=COM_EXIT;
   PrepareLocalPacket();
}

//****************************************************************************
//
// AddPauseStateCommand()
//
//****************************************************************************
void AddPauseStateCommand ( int type )
{
   ((COM_PauseType *)NextLocalCommand())->type=type;

   PrepareLocalPacket();
}


//****************************************************************************
//
// AddRespawnCommand()
//
//****************************************************************************
void AddRespawnCommand ( void )
{
   if (respawnactive==true)
      return;

   respawnactive=true;

   ((COM_RespawnType *)NextLocalCommand())->type=COM_RESPAWN;

   PrepareLocalPacket();
}


//****************************************************************************
//
// AddTextMessage()
//
//****************************************************************************
void AddTextMessage
   (
   char *message,
   int   length,
   int   towho
   )

   {
   COM_TextType *Text;

   Text = ( COM_TextType * )NextLocalCommand();

   Text->type = COM_TEXT;
   memset( &Text->string[ 0 ], 0, COM_MAXTEXTSTRINGLENGTH );

   if ( length >= COM_MAXTEXTSTRINGLENGTH )
      {
      length = COM_MAXTEXTSTRINGLENGTH - 1;
      }

   memcpy( &Text->string[ 0 ], message, length );

   Text->towho = towho;

   PrepareLocalPacket();
   }


//****************************************************************************
//
// PrepareLocalPacket
//
//****************************************************************************

void PrepareLocalPacket ( void )
{
   MoveType * pkt;

   wami(8);

   pkt=(MoveType *)NextLocalCommand();

   pkt->time=controlupdatetime;

   if (networkgame==false) // Whether it is a modem game or not we do this
      {
		AddClientPacket (pkt, consoleplayer);
      if (modemgame==false)
         {
         ServerCommandStatus ( controlupdatetime ) = cs_ready;
         }
      }

   if (modemgame==true)
      SendPacket (pkt, server);

#if (DEVELOPMENT == 1)
//   ComError("packet sent: realtime=%ld time=%ld type=%ld dest=%ld\n",GetTicCount(),pkt->time,pkt->type,server);
#endif

   controlupdatetime+=controldivisor;
   waminot();
}



//****************************************************************************
//
// GetPacketSize ()
//
//****************************************************************************

int GetPacketSize (void * pkt)
{
   int size;

   switch (((MoveType *)pkt)->type)
      {
      case COM_DELTA:
         size=sizeof(MoveType);
         break;
      case COM_DELTANULL:
         size=sizeof(NullMoveType);
         break;
      case COM_REQUEST:
         size=sizeof(COM_RequestType);
         break;
      case COM_FIXUP:
         size=sizeof(COM_FixupType);
         break;
      case COM_TEXT:
         size=sizeof(COM_TextType);
         break;
      case COM_PAUSE:
         size=sizeof(COM_PauseType);
         break;
      case COM_QUIT:
         size=sizeof(COM_QuitType);
         break;
      case COM_EXIT:
         size=sizeof(COM_ExitType);
         break;
      case COM_REMRID:
         size=sizeof(COM_RemoteRidiculeType);
         break;
      case COM_RESPAWN:
         size=sizeof(COM_RespawnType);
         break;
      case COM_UNPAUSE:
         size=sizeof(COM_UnPauseType);
         break;
      case COM_SERVER:
         size=sizeof(COM_ServerHeaderType);
         size-=sizeof(byte);
         break;
      case COM_GAMEDESC:
         size=sizeof(COM_GamePlayerType);
         break;
      case COM_GAMEEND:
         size=sizeof(COM_GameEndType);
         break;
      case COM_GAMEPLAY:
         size=DUMMYPACKETSIZE;
         break;
      case COM_GAMEACK:
         size=sizeof(COM_GameAckType);
         break;
      case COM_GAMEMASTER:
         size=sizeof(COM_GameMasterType);
         break;
      case COM_ENDGAME:
         size=sizeof(COM_EndGameType);
         break;
      case COM_SYNCTIME:
         size=sizeof(COM_SyncType);
         break;
#if (SYNCCHECK == 1)
      case COM_SYNCCHECK:
         size=sizeof(COM_CheckSyncType);
         break;
#endif
      case COM_SOUNDANDDELTA:
         size=sizeof(MoveType)+sizeof(COM_SoundType);
         break;
      default:
         Error("Unhandled packet type in GetPacketSize type=%d",((MoveType *)pkt)->type);
         break;
      }

   return size;
}

//****************************************************************************
//
// GetTypeSize ()
//
//****************************************************************************

int GetTypeSize (int type)
{
   byte pkt[2];

   pkt[0]=(byte)type;
   return ( GetPacketSize(&(pkt[0])) );
}

//****************************************************************************
//
// GetServerPacketSize ()
//
//****************************************************************************

int GetServerPacketSize (void * pkt)
{
   int i;
   byte * ptr;
   COM_ServerHeaderType * serverpkt;

   serverpkt=(COM_ServerHeaderType *)pkt;
   if (serverpkt->type==COM_SERVER)
      {
      ptr=&serverpkt->data;

      for (i=0;i<serverpkt->numpackets;i++)
         {
         ptr+=GetPacketSize(ptr);
         }
      return ((byte *)ptr-(byte *)pkt);
      }
   else
      return GetPacketSize(pkt);
}

//****************************************************************************
//
// SendPacket ()
//
//****************************************************************************

void SendPacket (void * pkt, int dest)
{
   if ((networkgame==false) && (PlayerStatus[dest]!=player_ingame))
      return;
   if ((IsServer==true) && (dest==server) && (standalone==false)) // must be client on top of server
      ProcessPacket(pkt,dest);
   else if ((IsServer==false) && (dest!=server) && (standalone==false)) // We shouldn't be sending as client to anyone else
      ComError("SendPacket:Problems\n");
   else
      WritePacket(pkt,GetPacketSize(pkt),dest);
#if (DEVELOPMENT == 1)
//   ComError( "SendPacket: time=%ld dest=%ld\n",((MoveType *)pkt)->time,dest);
#endif
}

//****************************************************************************
//
// ResetCurrentCommand ()
//
//****************************************************************************

void ResetCurrentCommand ( void )
{
   ServerCommandStatus(oldpolltime)=cs_notarrived;
}

//****************************************************************************
//
// BroadcastServerPacket ()
//
//****************************************************************************

void BroadcastServerPacket (void * pkt, int size)
{
   int i;


	for (i=0;i<numplayers;i++)
      {
      if (PlayerStatus[i]!=player_ingame)
         continue;
//      if ((standalone==false) && (i==consoleplayer))
//         ProcessPacket(pkt,i);
//      else
         WritePacket((byte *)pkt,size,i);
      }
}


//****************************************************************************
//
// ResendLocalPackets ()
//
//****************************************************************************

void ResendLocalPackets (int time, int dest, int numpackets)
{
   int cmd;
   MoveType * pkt;

   cmd = CommandAddress(time);

#if 0
   if (networkgame==false)
      {
      int nump;
      nump=controlupdatetime-time;
      if (nump>numpackets)
         numpackets=nump;
      }
#endif

   if (controlupdatetime<=time)
      return;

   pkt = (MoveType *)LocalCommand(cmd);

   if (pkt->time!=time)
      {
      Error( "CLIENT: Could not find packet to resend\ntime=%d packettime=%d controlupdatetime=%d\n",
             time, pkt->time, controlupdatetime);
      }
   else
      {
      byte * tempbuf;
      byte * tempptr;
      byte * tempstart;
      COM_FixupType * fixup;
      int i;
      int starti;
      int size;
      boolean done;

      // allocate some space

      tempbuf=SafeMalloc(MAXCOMBUFFERSIZE);

      fixup=(COM_FixupType *)tempbuf;

      fixup->type=COM_FIXUP;
      tempstart=&(fixup->data);

      done=false;
      i=0;
      while (done==false)
         {
         tempptr=tempstart;
         starti=i;
         fixup->time=( (MoveType *)LocalCommand(cmd) )->time;
         for (;i<numpackets;i++)
            {
            pkt = (MoveType *)LocalCommand(cmd);
            size=GetPacketSize(pkt);

            if (((tempptr+size)-tempbuf)>MAXCOMBUFFERSIZE)
               {
               break;
               }
            memcpy(tempptr,pkt,size);
            tempptr+=size;
            cmd = (cmd + controldivisor) & (MAXCMDS-1);
            }
         fixup->numpackets=i-starti;
         WritePacket(tempbuf,tempptr-tempbuf,dest);
         if (i==numpackets)
            done=true;
         }

      SafeFree(tempbuf);
      }
}

//****************************************************************************
//
// ResendServerPackets ()
//
//****************************************************************************

void ResendServerPackets (int time, int dest, int numpackets)
{
   int cmd;
   COM_ServerHeaderType * serverpkt;


   cmd = CommandAddress(time);

   if (serverupdatetime<=time)
      return;

   serverpkt = (COM_ServerHeaderType *)ServerCommand(cmd);

   if (serverpkt->time!=time)
      {
      Error( "SERVER: Could not find packet to resend\ntime=%d packettime=%d serverupdatetime=%d\n",
             time, serverpkt->time,serverupdatetime);
      }
   else
      {
      byte * tempbuf;
      byte * tempptr;
      byte * tempstart;
      COM_FixupType * fixup;
      int i;
      int starti;
      int size;
      boolean done;

      // allocate some space

      tempbuf=SafeMalloc(MAXCOMBUFFERSIZE);

      fixup=(COM_FixupType *)tempbuf;

      fixup->type=COM_FIXUP;
      tempstart=&(fixup->data);

      done=false;
      i=0;
      while (done==false)
         {
         tempptr=tempstart;
         starti=i;
         fixup->time=( (MoveType *)ServerCommand(cmd) )->time;
         for (;i<numpackets;i++)
            {
            serverpkt = (COM_ServerHeaderType *)ServerCommand(cmd);
            size=GetServerPacketSize(serverpkt);

            if (((tempptr+size)-tempbuf)>MAXCOMBUFFERSIZE)
               {
               break;
               }
            memcpy(tempptr,serverpkt,size);
            tempptr+=size;
            cmd = (cmd + controldivisor) & (MAXCMDS-1);
            }
         fixup->numpackets=i-starti;
         WritePacket(tempbuf,tempptr-tempbuf,dest);
         if (i==numpackets)
            done=true;
         }

      SafeFree(tempbuf);
      }
}


//****************************************************************************
//
// ResendPacket (incoming packet, whoever requested it)
//
//****************************************************************************

void ResendPacket (void * pkt, int dest)
{
   int time;
   COM_RequestType * request;

   if ((networkgame==false) && (PlayerStatus[dest]!=player_ingame))
      return;

   request=(COM_RequestType * )pkt;
   time=request->time;

   ComError( "RESEND request received at %d\n packet time=%d dest=%d numpackets=%d\n",
             GetTicCount(), time, dest, request->numpackets);

   if (IsServer==true)
      {
      if ((dest==server) && (standalone==false))
         Error("Trying to resend packets to client on top of server\n");
      ComError( "RESEND SERVER serverupdatetime=%d\n",serverupdatetime);
      if (IsServerCommandReady ( time ) == true)
         ResendServerPackets(time,dest,request->numpackets);
      else
         ComError( "RESEND SERVER time=%d is not ready\n",time);
      }
   else
      {
      ResendLocalPackets(time,dest,request->numpackets);
      }
}

//****************************************************************************
//
// FixupPacket ()
//
//****************************************************************************

void FixupPacket (void * pkt, int src)
{
   COM_FixupType * fix;
   int i;
   int time;
   byte * ptr;

   fix=(COM_FixupType *)pkt;

   ComError( "Fixup received at %d, time=%d numpackets=%d\n", GetTicCount(), fix->time, fix->numpackets);
#if 0
   if (networkgame==false)
      FixingPackets=false;
#endif
   time=fix->time;
   ptr=&(fix->data);

   for (i=0;i<fix->numpackets;i++,time+=controldivisor)
      {
      if (time == (LastCommandTime[src]+controldivisor))
         LastCommandTime[src]=time;

      if (IsServer==true)
         {
         if (ClientCommandStatus(src, time)!=cs_fixing)
            {
            ComError("Server Received fixup with no bad packet time=%d from %d\n",time,src);
            }
         else
            {
            AddSubPacket(ptr, src);
            }
         ptr+=GetPacketSize(ptr);
         }
      else
         {
         if (ServerCommandStatus(time)!=cs_fixing)
            {
            ComError("Client Received fixup with no bad packet time=%d from %d\n",time,src);
            }
         else
            {
            if (networkgame==true)
               {
               AddServerSubPacket( (COM_ServerHeaderType *)ptr );
               }
            else
               {
               AddModemSubPacket(ptr);
               }
            }
         ptr+=GetServerPacketSize(ptr);
         }
      }
}

#if (SYNCCHECK == 1)
//****************************************************************************
//
// CheckForSyncCheck
//
//****************************************************************************

void CheckForSyncCheck ( void )
{
   int i;


   if (modemgame==true)
      {
      if (oldpolltime==lastsynccheck)
         {
         for (i=0;i<numplayers;i++)
            {
            PlayerSync[i].x=PLAYER[i]->x;
            PlayerSync[i].y=PLAYER[i]->y;
            PlayerSync[i].z=PLAYER[i]->z;
            PlayerSync[i].angle=PLAYER[i]->angle;
            }
         PlayerSync[0].randomindex=GetRNGindex();
         PlayerSync[0].synctime=lastsynccheck;
         SendSyncCheckPacket();
         lastsynccheck+=CHECKSYNCTIME;
         }
      if (oldpolltime>lastsynccheck)
         {
         Error("Missed a player sync check time=%d\n",oldpolltime);
         }
      }
}
#endif

//****************************************************************************
//
// ProcessSyncTimePacket
//
//****************************************************************************

void ProcessSyncTimePacket (void * pkt)
{
   COM_SyncType * sync;

   sync=(COM_SyncType *)pkt;
   ISR_SetTime(sync->synctime);
}

#if (SYNCCHECK == 1)
//****************************************************************************
//
// ProcessSyncCheckPacket
//
//****************************************************************************

void ProcessSyncCheckPacket (void * pkt, int src)
{
   COM_CheckSyncType * sync;

   sync=(COM_CheckSyncType *)pkt;
//   SoftError("Sync packet time=%ld\n",sync->synctime);
   if (sync->synctime!=PlayerSync[0].synctime)
      {
      SoftError("Old sync packet received\n");
      return;
      }
   if (sync->randomindex!=PlayerSync[0].randomindex)
      {
      Error("Player %d is unsynced localindex=%d remoteindex=%d\n"
            "Unsynced Player x=%x y=%x a=%d z=%d name=%s\n",
             src, PlayerSync[0].randomindex, sync->randomindex,
             PlayerSync[src].x, PlayerSync[src].y, PlayerSync[src].angle,
             PlayerSync[src].z,PLAYERSTATE[src].codename);
      }
   if (sync->x!=PlayerSync[src].x)
      {
      Error("Player %d is unsynced local x=%d remote x=%d\n"
            "Unsynced Player x=%x y=%x a=%d z=%d name=%s\n",
             src,PlayerSync[src].x,sync->x,
             PlayerSync[src].x, PlayerSync[src].y, PlayerSync[src].angle,
             PlayerSync[src].z,PLAYERSTATE[src].codename);
      }
   if (sync->y!=PlayerSync[src].y)
      {
      Error("Player %d is unsynced local y=%d remote y=%d\n"
            "Unsynced Player x=%x y=%x a=%d z=%d name=%s\n",
             src,PlayerSync[src].y,sync->y,
             PlayerSync[src].x, PlayerSync[src].y, PlayerSync[src].angle,
             PlayerSync[src].z,PLAYERSTATE[src].codename);
      }
   if (sync->z!=PlayerSync[src].z)
      {
      Error("Player %d is unsynced local z=%d remote z=%d\n"
            "Unsynced Player x=%x y=%x a=%d z=%d name=%s\n",
             src,PlayerSync[src].z,sync->z,
             PlayerSync[src].x, PlayerSync[src].y, PlayerSync[src].angle,
             PlayerSync[src].z,PLAYERSTATE[src].codename);
      }
   if (sync->angle!=PlayerSync[src].angle)
      {
      Error("Player %d is unsynced local angle=%d remote angle=%d\n"
            "Unsynced Player x=%x y=%x a=%d z=%d name=%s\n",
             src,PlayerSync[src].angle,sync->angle,
             PlayerSync[src].x, PlayerSync[src].y, PlayerSync[src].angle,
             PlayerSync[src].z,PLAYERSTATE[src].codename);
      }
}

//****************************************************************************
//
// SendSyncCheckPacket
//
//****************************************************************************

void SendSyncCheckPacket ( void )
{
   ((COM_CheckSyncType *)NextLocalCommand())->type=COM_SYNCCHECK;
   ((COM_CheckSyncType *)NextLocalCommand())->synctime=PlayerSync[0].synctime;
   ((COM_CheckSyncType *)NextLocalCommand())->x=PlayerSync[consoleplayer].x;
	((COM_CheckSyncType *)NextLocalCommand())->y=PlayerSync[consoleplayer].y;
	((COM_CheckSyncType *)NextLocalCommand())->z=PlayerSync[consoleplayer].z;
	((COM_CheckSyncType *)NextLocalCommand())->angle=PlayerSync[consoleplayer].angle;
	((COM_CheckSyncType *)NextLocalCommand())->randomindex=PlayerSync[0].randomindex;

   PrepareLocalPacket();
}
#endif

#if 0

//****************************************************************************
//
// CheckForSyncTime
//
//****************************************************************************

void CheckForSyncTime ( void )
{
   if ((modemgame==true) && (networkgame==false) && (consoleplayer==0))
      {
      if (controlupdatetime>=syncservertime)
         {
         SendSyncTimePacket();
         syncservertime+=MODEMSYNCSERVERTIME;
         }
      }
}
#endif

#if 0
//****************************************************************************
//
// SendSyncTimePacket
//
//****************************************************************************

void SendSyncTimePacket ( void )
{
   int i;
   COM_SyncType sync;

   return;

   sync.type=COM_SYNCTIME;

   if (networkgame==true)
      {
      for (i=0;i<numplayers;i++)
         {
         if ((PlayerStatus[i]!=player_ingame) || ( (i==consoleplayer) && (standalone==false) ) )
            continue;
         sync.synctime=GetTicCount()+GetTransitTime(i);
         WritePacket ( &sync.type, GetPacketSize(&sync.type), i);
         }
      }
   else
      {
      if (PlayerStatus[server]==player_ingame)
         {
         sync.synctime=GetTicCount()+GetTransitTime(server);
         WritePacket ( &sync.type, GetPacketSize(&sync.type), server);
         }
      }
}
#endif

//****************************************************************************
//
// ProcessSoundAndDeltaPacket
//
//****************************************************************************

void ProcessSoundAndDeltaPacket (void * pkt, int src)
{
   MoveType * packet;
   COM_SoundType * sndpkt;
   byte oldtype;

   packet = (MoveType *)pkt;

   // Trick packet into being a normal delta packet

   oldtype=packet->type;
   packet->type=COM_DELTA;
   AddClientPacket (pkt,src);
   packet->type=oldtype;

   // Don't process sound if it is from us
   if (src==consoleplayer)
      return;

   sndpkt = (COM_SoundType *) (packet->Sounddata);

   if (sndpkt->type==COM_SOUND_START_TRANSMISSION)
      {
      SD_StartIncomingSound ();
      }
   if (sndpkt->type==COM_SOUND_END_TRANSMISSION)
      {
      SD_StopIncomingSound();
      }
   else
      {
      SD_UpdateIncomingSound (&(sndpkt->data[0]), COM_SOUND_BUFFERSIZE);
      }
}
//****************************************************************************
//
// SyncToServer
//
//****************************************************************************
#define NETWORKTIMEAHEADOFSERVER (1)
#define MODEMTIMEAHEADOFSERVER (2)
void SyncToServer( void )
{
   int diff;

   if ((networkgame==false) && (consoleplayer==0))
      return;
   if (IsServer==true)
      return;
//   if (networkgame==true)
//      {
//      diff = (GetTicCount()-controldivisor-LastCommandTime[0])/controldivisor;
//      SoftError("diff=%ld\n",diff);
//      if (abs(diff)>1)
//         ISR_SetTime(GetTicCount()-diff);
#if 0
      diff = controlupdatetime-LastCommandTime[0];
      if (diff>3)
         {
         ISR_SetTime(GetTicCount()-1);
         }
      else if (diff<-3)
         {
         ISR_SetTime(GetTicCount()+1);
         }
#endif
//      }
//   else
//      {
      diff = (GetTicCount()-controldivisor-LastCommandTime[server])/controldivisor;
      if (abs(diff)>0)
         ISR_SetTime(GetTicCount()-diff);
//      }
}

//****************************************************************************
//
// ProcessPacket
//
//****************************************************************************

void ProcessPacket (void * pkt, int src)
{
   switch (((MoveType *)pkt)->type)
      {
      case COM_DELTA:
      case COM_DELTANULL:
      case COM_TEXT:
      case COM_PAUSE:
      case COM_QUIT:
      case COM_EXIT:
      case COM_REMRID:
      case COM_RESPAWN:
      case COM_UNPAUSE:
      case COM_ENDGAME:
#if (SYNCCHECK == 1)
      case COM_SYNCCHECK:
#endif
//         if (FixingPackets==false)
         AddPacket(pkt,src);
         break;
      case COM_SOUNDANDDELTA:
         if (remoteridicule == false )
            {
            ((MoveType *)pkt)->type = COM_DELTA;
            }
         AddPacket(pkt,src);
         break;
      case COM_SERVER:
         AddServerPacket(pkt,src);
         break;

      case COM_REQUEST:
         ResendPacket(pkt, src);
         break;

      case COM_FIXUP:
         FixupPacket(pkt, src);
         break;

      case COM_SYNCTIME:
         ProcessSyncTimePacket(pkt);
         break;

      case COM_GAMEEND:
      case COM_GAMEDESC:
      case COM_GAMEACK:
      case COM_GAMEMASTER:
         if (standalone==true)
            restartgame=true;
         break;

      case COM_START:
         break;

      default:
         Error("ProcessPacket: Unknown packet type=%d\n",((MoveType *)pkt)->type);
      }
}


//****************************************************************************
//
// AddServerSubPacket
//
//****************************************************************************

void AddServerSubPacket(COM_ServerHeaderType * serverpkt)
{
   byte * pkt;
   int i;

   ServerCommandStatus(serverpkt->time)=cs_ready;

   pkt=&serverpkt->data;
   for (i=0;i<serverpkt->numpackets;i++)
      {
      AddClientPacket(pkt,i);
      pkt+=GetPacketSize(pkt);
      }
}

//****************************************************************************
//
// AddModemSubPacket
//
//****************************************************************************

void AddModemSubPacket(void * incoming)
{
   MoveType * pkt;

   pkt=(MoveType *)incoming;
   ServerCommandStatus(pkt->time)=cs_ready;

   AddClientPacket(incoming,server);
}

//****************************************************************************
//
// AddServerPacket
//
//****************************************************************************

void AddServerPacket(void * pkt, int src)
{
   COM_ServerHeaderType * serverpkt;

   // The server uses the client's lgts for communicating

   // Last good time can be set even for the client/server combo

   if (standalone==true)
      {
      Error("standalone should not be here\n");
      }

   if (src!=server)
      {
      Error("Received server packet from non-server src=%d\n",src);
      }

   serverpkt=(COM_ServerHeaderType *)pkt;

//   if (networkgame==false)
//      SyncToServer(serverpkt->time);

   LastCommandTime[src]+=controldivisor;

   if (serverpkt->time != LastCommandTime[src])
      {
      int numpackets;

      numpackets=serverpkt->time-LastCommandTime[src];
      if (ServerCommandStatus(LastCommandTime[src])!=cs_fixing)
         {
         RequestPacket ( LastCommandTime[src] , src , numpackets );

         ComError("AddServerPacket: Request packet time=%d lct=%d numpackets=%d\n",
                  serverpkt->time, LastCommandTime[src], numpackets
                 );
         }

      LastCommandTime[src]+=numpackets;
      }

   AddServerSubPacket( serverpkt );
}

//****************************************************************************
//
// AddClientPacket
//
//****************************************************************************

void AddClientPacket (void * pkt, int src)
{
   int size;
   MoveType * packet;

   packet=(MoveType *)pkt;

   switch (packet->type)
      {
      case COM_DELTA:
      case COM_DELTANULL:
      case COM_TEXT:
      case COM_REMRID:
      case COM_PAUSE:
      case COM_QUIT:
      case COM_EXIT:
      case COM_RESPAWN:
      case COM_UNPAUSE:
#if (SYNCCHECK == 1)
      case COM_SYNCCHECK:
#endif
      case COM_ENDGAME:
         size=GetPacketSize(packet);
         memcpy(PlayerCommand(src,CommandAddress(packet->time)),packet,size);
         break;
      case COM_SOUNDANDDELTA:
         ProcessSoundAndDeltaPacket(packet, src);
         break;
      default:
         Error("AddClientPacket: Unknown packet type = %d\n",packet->type);
      }
}

//****************************************************************************
//
// AddSubPacket
//
//****************************************************************************

void AddSubPacket (void * pkt, int src)
{
   MoveType * packet;

   if (networkgame==false)
      Error("Modem game should not be here in AddSubPacket\n");

   packet = (MoveType *) pkt;

   ClientCommandStatus(src, packet->time)=cs_ready;

   memcpy (
          ClientTimeCommand(src,packet->time),
          pkt,
          GetPacketSize(packet)
          );
}

//****************************************************************************
//
// AddPacket
//
//****************************************************************************

void AddPacket (void * pkt, int src)
{
   MoveType * packet;

   // should only be called by server in network game
   // in modem game we fall through the first condition
   // all packets should be sequential

   if ((IsServer==true) && (PlayerStatus[src]!=player_ingame))
      return;
   packet = (MoveType *) pkt;

//   if ((networkgame==false) && (consoleplayer!=0))
//      SyncToServer();

   if (!((src==server) && (standalone==false) && (IsServer==true)))
      {
      LastCommandTime[src]+=controldivisor;

      if (packet->time != LastCommandTime[src])
         {
         int numpackets;

         numpackets=packet->time-LastCommandTime[src];
         if ( ( (networkgame==false) &&
               (ServerCommandStatus(LastCommandTime[src])!=cs_fixing)
            )
            ||
            ( (networkgame==true) &&
               (ClientCommandStatus(src,LastCommandTime[src])!=cs_fixing)
            )
            )
            {
            RequestPacket ( LastCommandTime[src] , src , numpackets );

            ComError("AddPacket: Request packet time=%d lct=%d numpackets=%d\n",
                     packet->time, LastCommandTime[src], numpackets
                     );
            }

         LastCommandTime[src]+=numpackets;
         }
      }

   if (networkgame==true)
      {
      AddSubPacket ( packet, src );
      }
   else
      {
      AddModemSubPacket(packet);
      }
}


//****************************************************************************
//
// RequestPacket ( int time, int dest )
//
//****************************************************************************

void RequestPacket (int time, int dest, int numpackets)
{
   COM_RequestType request;
   int i;


#if (DEVELOPMENT == 1)
   if (modemgame==false)
      Error("Called Request Packet outside of modem game\n");
#endif

   request.type=COM_REQUEST;
   request.time=time;
   request.numpackets=numpackets/controldivisor;

   if (IsServer==true)
      {
      if ((dest==server) && (standalone==false))
         {
         Error("Requesting packet from client on top of server\n");
         }
      if (PlayerStatus[dest]!=player_ingame)
         return;
      for (i=0;i<numpackets;i+=controldivisor)
         {
         ClientCommandStatus( dest , (time+i) ) = cs_fixing;
         }
      }
   else
      {
      if ((networkgame==false) && (PlayerStatus[dest]!=player_ingame))
         return;
      for (i=0;i<numpackets;i+=controldivisor)
         {
         ServerCommandStatus( (time+i) ) = cs_fixing;
         }
      }
//   if (networkgame==false)
//      FixingPackets=true;

   // send out the packet

   WritePacket (&request, GetPacketSize(&request), dest);

#if (DEVELOPMENT == 1)
//   ComError( "BADPKT, request sent at %ld lgt=%ld dest=%ld\n",GetTicCount(),time,dest);
#endif
}

//****************************************************************************
//
// IsServerCommandReady ()
//
//****************************************************************************
boolean IsServerCommandReady ( int time )
{

   if (
       (
        (COM_ServerHeaderType *)
         ServerCommand(CommandAddress (time) ) )->time==time)
      return true;
   else
      {
      return false;
      }
}

//****************************************************************************
//
// AreClientsReady ()
//
//****************************************************************************
boolean AreClientsReady ( void )
{
   int i;
   int timeindex;
   int status;

   timeindex=CommandAddress(serverupdatetime);

	for (i=0;i<numplayers;i++)
      {
      if (PlayerStatus[i]!=player_ingame)
         continue;
      status=ClientCommandStatus(i, serverupdatetime);
      if (status==cs_notarrived)
         return false;
      else if (status==cs_fixing)
         {
//         RequestPacket ( serverupdatetime , i , controldivisor );
         return false;
         }
      else if (((MoveType *)ClientCommand(i, timeindex))->time != serverupdatetime)
         return false;
      }
   return true;
}

//****************************************************************************
//
// IsPlayerCommandReady ()
//
//****************************************************************************
boolean IsPlayerCommandReady (int num, int time)
{
   MoveType * cmd;

   cmd=(MoveType *)PlayerCommand(num,CommandAddress(time));

   if (cmd->time==time)
      return true;
   else
      return false;
}

//****************************************************************************
//
// ResetClientCommands ()
//
//****************************************************************************
void ResetClientCommands ( int player )
{
   int j;

   for (j=0;j<MAXCMDS;j++)
      {
      memset(ClientCommand(player,j),COM_DELTA,GamePacketSize());
      }
}

//****************************************************************************
//
// SendFullServerPacket ()
//
//****************************************************************************
void SendFullServerPacket ( void )
{
   int i;
   int size;
   byte * pkt;
   COM_ServerHeaderType * spkt;
   int timeindex;
   int playerstatus[MAXPLAYERS];

   timeindex=CommandAddress(serverupdatetime);

   spkt=(COM_ServerHeaderType *)ServerCommand(timeindex);

   pkt=&spkt->data;
   spkt->time=serverupdatetime;
   spkt->type=COM_SERVER;
	spkt->numpackets=numplayers;


   memset(playerstatus,-1,sizeof(playerstatus));
	for (i=0;i<numplayers;i++)
      {
      size=GetPacketSize(ClientCommand(i,timeindex));
      if (((MoveType *)ClientCommand(i,timeindex))->type == COM_QUIT)
         {
         playerstatus[i]=player_quitgame;
         }
      if (((MoveType *)ClientCommand(i,timeindex))->type == COM_ENDGAME)
         {
         playerstatus[i]=player_leftgame;
         }
      memcpy(pkt,
            ClientCommand(i,timeindex),
            size
            );
      pkt+=size;
      ClientCommandNumberStatus(i,timeindex)=cs_notarrived;
      }
   BroadcastServerPacket((void *)spkt,(pkt-(byte *)spkt));
   serverupdatetime+=controldivisor;

   for (i=0;i<numplayers;i++)
      {
      if (playerstatus[i]!=-1)
         {
         if ((standalone==false) && (consoleplayer==i))
            {
            UpdateServer=false;
            }
         else
            {
            ResetClientCommands(i);
            PlayerStatus[i]=playerstatus[i];
            }
         }
      }
}



//****************************************************************************
//
// ProcessServer ()
//
//****************************************************************************

void ProcessServer ( void )
{
   boolean done;
   boolean exit;
   int i;
   int time;
   int quittime;

   if (InProcessServer==true)
      return;

   InProcessServer=true;

   if (GetTicCount()<serverupdatetime)
      goto exitProcessServer;

   time=GetTicCount();
   quittime=GetTicCount()+SERVERTIMEOUT;
   exit=false;

   while (time>=serverupdatetime)
      {
      int savetime;

      savetime=GetTicCount()+NETWORKTIMEOUT;
      done = false;
		while (done == false)
         {
         if (standalone==true)
            AbortCheck("GameServer aborted\n");

         done = AreClientsReady ();

         if ( (standalone==false) && (serverupdatetime>=(controlupdatetime-controldivisor)) && (done==false) )
            break;

         CheckForPacket ();

         if (standalone==false)
            UpdateClientControls();

         if (restartgame==true)
            break;
         if (GetTicCount()>savetime)
            {
            for (i=0;i<numplayers;i++)
               {
               int val;

               val=ClientCommandStatus(i, serverupdatetime);
               if ((val!=cs_ready) && (PlayerStatus[i]==player_ingame))
                  {
                  SoftError("Server timeout\n");
                  RequestPacket(serverupdatetime, i , controldivisor);
                  }
               }
            savetime=GetTicCount()+NETWORKTIMEOUT;
            }
//         if (GetTicCount()>quittime)
//            {
//            Error("Server aborting after %ld seconds\n",SERVERTIMEOUT/VBLCOUNTER);
//            }
         if ((standalone==false) && (done==false))
            {
            exit=true;
            done=true;
            }
         }
      if (exit==true)
         break;
      if ( (serverupdatetime>=(controlupdatetime-controldivisor)) && (standalone==false))
         break;
      if (restartgame==true)
         break;
      SendFullServerPacket();
#if 0
      if (serverupdatetime>=syncservertime)
         {
         SendSyncTimePacket();
         syncservertime+=NETSYNCSERVERTIME;
         }
#endif
      }
exitProcessServer:
   InProcessServer=false;
}


//****************************************************************************
//
// SetupCheckForPacket()
//
//****************************************************************************
int SetupCheckForPacket ( void )
{
   int retval=scfp_nodata;

   if ((ReadPacket()==true) && (badpacket==0))
      {
      MoveType * pkt;

      retval=scfp_data;
      pkt=(MoveType *)&ROTTpacket[0];
      if ((IsServer==true) && (standalone==true))
         {
         switch (pkt->type)
            {
            case COM_GAMEEND:
               break;
            case COM_GAMEDESC:
               if (standalone==true)
                  printf("Received GameDescription from player#%ld\n",(long int)rottcom->remotenode);
               WritePacket(&ROTTpacket[0],GetPacketSize(pkt),0); // Send to player 0
               break;
            case COM_GAMEACK:
               if (standalone==true)
                  printf("Received GameAcknowledgement from player#%ld\n",(long int)rottcom->remotenode);
               WritePacket(&ROTTpacket[0],GetPacketSize(pkt),0); // Send to player 0
               break;
            case COM_GAMEMASTER:
               if (standalone==true)
                  printf("Received GameMasterPacket from player#%ld\n",(long int)rottcom->remotenode);
               BroadcastServerPacket(&ROTTpacket[0],GetPacketSize(pkt)); // Send to all
               break;
            case COM_GAMEPLAY:
               if (standalone==true)
                  printf("Received StartGamePacket from player#%ld\n",(long int)rottcom->remotenode);
               BroadcastServerPacket(&ROTTpacket[0],GetPacketSize(pkt)); // Send to all
               retval=scfp_done;
               break;
            default:
               ComError("Server received unknown packet in Game preamble\n");
               break;
            }
         }
      else
         {
         switch (pkt->type)
            {
            case COM_GAMEPLAY:
                retval=scfp_done;
                break;
            case COM_GAMEMASTER:
                SetGameDescription(pkt);
                retval=scfp_gameready;
                break;
            case COM_GAMEACK:
					 PlayersReady[((COM_GameAckType *)pkt)->player]=true;
                break;
            case COM_GAMEDESC:
					 GotPlayersDesc[((COM_GamePlayerType *)pkt)->player]=true;
                SetPlayerDescription(pkt);
                break;
            }
         }
      }
   return retval;
}


//****************************************************************************
//
// ServerLoop ()
//
//****************************************************************************
void ServerLoop( void )
{
   boolean done;

   while (1)
      {
      ShutdownClientControls();
      restartgame=false;

      done=false;
      while (done==false)
         {
         AbortCheck("SetupGameServer aborted\n");

         if (SetupCheckForPacket()==scfp_done)
            done=true;
         }
      ComSetTime();
      StartupClientControls();
      while(1)
         {
         ProcessServer();
#if (DEVELOPMENT == 1)
         Z_CheckHeap();
#endif
         CalcTics();
         if (restartgame==true)
            break;
         }
      }
}

//****************************************************************************
//
// ProcessPlayerCommand()
//
//****************************************************************************
void ProcessPlayerCommand( int player )
{
   MoveType * cmd;

   cmd=(MoveType *)PlayerCommand(player,CommandAddress(oldpolltime));

	if (cmd->type==COM_DELTA)
      {
		UpdatePlayerObj(player);
      }
	else if (cmd->type==COM_RESPAWN)
      {
		if (player==consoleplayer) // reset spawn state
         respawnactive=false;
      RespawnPlayerobj(PLAYER[player]);
      }
	else if (cmd->type==COM_ENDGAME)
      {
      playstate = ex_battledone;
      }
   else if (cmd->type==COM_QUIT)
      {
      if (player==consoleplayer)
         QuitGame();
      else
         {
         char str[50]="Player #";
         char str2[10];

         strcat(str,itoa(player+1,str2,10));
         strcat(str,", ");
         strcat(str,PLAYERSTATE[player].codename);
         strcat(str," has left the game.");
         AddMessage(str,MSG_REMOTE);
         PlayerStatus[player]=player_quitgame;
         }
      }
   else if (cmd->type==COM_EXIT)
      {
      QuitGame();
      }
   else if (cmd->type==COM_REMRID)
      {
      ProcessRemoteRidicule (cmd);
      }
   else if (cmd->type==COM_TEXT)
      {
      int who;

      who = ( ( COM_TextType * )cmd )->towho;
      if ( ( who == consoleplayer ) ||
         ( who == MSG_DIRECTED_TO_ALL ) ||
         ( ( who == MSG_DIRECTED_TO_TEAM ) &&
         ( BATTLE_Team[ player ] == BATTLE_Team[ consoleplayer ] ) ) )
         {
         char string[ 50 ];

         strcpy( string, "\\N9" );
         strcat( string, PLAYERSTATE[player].codename );
         strcat( string, ":\\NF" );
         strcat( string, ((COM_TextType *)cmd)->string );
         SD_PlayPitchedSound ( SD_ENDBONUS1SND, 255, 1200 );

         AddMessage( string, MSG_REMOTE );
         }
      }
#if (SYNCCHECK == 1)
	else if (cmd->type==COM_SYNCCHECK)
      {
      ProcessSyncCheckPacket(cmd, player);
      }
#endif
	else if (cmd->type==COM_PAUSE)
      {
      MUSIC_Pause();
      GamePaused=true;
      pausedstartedticcount = oldpolltime;
      }
	else if (cmd->type==COM_UNPAUSE)
      {
      GamePaused=false;
      MUSIC_Continue ();
      if (RefreshPause == false)       // screen is blanked
         {
         ShutdownScreenSaver();
         SetupScreen (true);
         RefreshPause = true;
         }
      }
}

//****************************************************************************
//
// CheckUnPause ()
//
//****************************************************************************
void CheckUnPause ( void )
{
   if (oldpolltime==nextupdatetime)
      {
      nextupdatetime=oldpolltime+controldivisor;
      while (1)
         {
         if (ServerCommandStatus(oldpolltime)==cs_ready)
            {
            int j;

            for (j=0;j<numplayers;j++)
               {
               if (PlayerStatus[j]==player_ingame)
                  ProcessPlayerCommand( j );
               }
            break;
            }
         else
            {
            UpdateClientControls();
            }
         }
      }
}


//****************************************************************************
//
// ControlPlayerObj ()
//
//****************************************************************************
void ControlPlayerObj (objtype * ob)
{
	playertype * pstate;
   int num;
   int savetime;
//   boolean asked;

//   if (GamePaused==true)
//      return;

   M_LINKSTATE(ob,pstate);

	// get player number

   num=ob->dirchoosetime;

   memcpy (pstate->buttonheld, pstate->buttonstate, sizeof(pstate->buttonstate));

   if (oldpolltime==nextupdatetime)
      {
      if (num==numplayers-1)
         nextupdatetime=oldpolltime+controldivisor;
      if (networkgame==true)
         savetime=GetTicCount()+NETWORKTIMEOUT;
      else
         savetime=GetTicCount()+MODEMTIMEOUT;

      if (PlayerStatus[num]!=player_ingame)
         return;

      //   asked=false;

      // copy previous state of buttons


      while (1)
         {
         if (ServerCommandStatus(oldpolltime)==cs_ready)
            {
            ProcessPlayerCommand (num);
            if (demoplayback||demorecord) {
               SoftError("x=%4x y=%4x a=%4x time=%5d\n",player->x,player->y,player->angle,oldpolltime);
	    }
            break;
            }
   //      else if ((ServerCommandStatus(oldpolltime)==cs_fixing) &&
   //               (networkgame==false) &&
   //               (asked==false)
   //              )
   //         {
   //         asked=true;
   //         RequestPacket(oldpolltime, server, controldivisor);
   //         }
         else
            {
            UpdateClientControls();
            }

         if (GetTicCount()>savetime)
            {
            SoftError("Client timeout oldpolltime=%d\n",oldpolltime);
            if (IsServer==false)
               RequestPacket(oldpolltime, server, controldivisor);
            if (networkgame==true)
               savetime=GetTicCount()+NETWORKTIMEOUT;
            else
               savetime=GetTicCount()+MODEMTIMEOUT;
            }
         }
      }

   if (!(ob->flags&FL_DYING))
      {
      if (ob->flags&FL_PUSHED)
         {
         ob->flags&=~FL_PUSHED;
#if 0
         if (abs(ob->momentumx)>0)
            {
            if (abs(ob->momentumx+pstate->dmomx)>=abs(ob->momentumx))
               {
               ob->momentumx += pstate->dmomx;
               ob->momentumy += pstate->dmomy;
               }
            }
         else if (abs(ob->momentumy+pstate->dmomy)>=abs(ob->momentumy))
            {
            ob->momentumx += pstate->dmomx;
            ob->momentumy += pstate->dmomy;
            }
#endif
         if (abs(ob->momentumx+pstate->dmomx)>=abs(ob->momentumx))
            {
            ob->momentumx += pstate->dmomx;
            }
         if (abs(ob->momentumy+pstate->dmomy)>=abs(ob->momentumy))
            {
            ob->momentumy += pstate->dmomy;
            }
         }
      else
         {
         ob->momentumx += pstate->dmomx;
         ob->momentumy += pstate->dmomy;
         }
      }
}

//****************************************************************************
//
// MaxSpeedForCharacter ()
//
//****************************************************************************

int MaxSpeedForCharacter(playertype*pstate)
{
   if (BATTLEMODE && (gamestate.BattleOptions.Speed == bo_fast_speed))
      {
      return( FASTSPEED );
      }
   else
      {
      if (pstate->buttonstate[bt_run])
         return (characters[pstate->player].toprunspeed);
      else
         return (characters[pstate->player].topspeed);
      }
}

//****************************************************************************
//
// UpdatePlayerObj ()
//
//****************************************************************************

void UpdatePlayerObj ( int player )
{
	int i, buttonbits;
	playertype * pstate;
	MoveType * MoveCmd;

   MoveCmd=(MoveType *)PlayerCommand(player,CommandAddress(oldpolltime));

	pstate=&PLAYERSTATE[player];

   buttonbits = MoveCmd->buttons;
	for (i = 0; i < NUMTXBUTTONS; i++)
		{
		pstate->buttonstate[i] = buttonbits & 1;
		buttonbits   >>= 1;
		}

   pstate->dmomx = (int)(MoveCmd->momx)<<1;
   pstate->dmomy = (int)(MoveCmd->momy)<<1;
	pstate->angle = MoveCmd->dangle;
	pstate->angle <<= 11;
   pstate->topspeed=MaxSpeedForCharacter(pstate);

   if (demoplayback||demorecord) {
      SoftError("  dmx=%4x dmy=%4x da=%4x time=%5d\n",pstate->dmomx,pstate->dmomy,pstate->angle>>11,oldpolltime);
   }
#if 0
#if (DEVELOPMENT == 1)
         if ((modemgame==true) || (demoplayback==true) || (demorecord==true))
		   {
			ComError( "player#%2ld\n",player);
		   ComError( "momx = %6ld\n", PLAYER[player]->momentumx);
		   ComError( "momy = %6ld\n", PLAYER[player]->momentumy);
		   ComError( "   x = %6ld\n", PLAYER[player]->x);
		   ComError( "   y = %6ld\n", PLAYER[player]->y);
		   ComError( "   z = %6ld\n", PLAYER[player]->z);
		   ComError( "   a = %6ld\n", PLAYER[player]->angle);
		   if (pstate->buttonstate[bt_attack])
			   ComError( "FIRING\n");
         }
#endif
#endif
}


//****************************************************************************
//
// SendPlayerDescription ()
//
//****************************************************************************

void SendPlayerDescription( void )
{
   byte * temp;
   COM_GamePlayerType * desc;
   int length;

   length=sizeof(COM_GamePlayerType);
   temp=SafeMalloc(length);

   memset(temp,0,length);

   desc=(COM_GamePlayerType *)temp;
   desc->type=(byte)COM_GAMEDESC;
	desc->player=consoleplayer;
   desc->violence=gamestate.violence;
   desc->Version = gamestate.Version;
   desc->Product = gamestate.Product;
   desc->playerdescription.character=locplayerstate->player;
   desc->playerdescription.uniformcolor=locplayerstate->uniformcolor;
   strcpy(&(desc->playerdescription.codename[0]),
      &locplayerstate->codename[0]);

   WritePacket(temp,length,server);

   SafeFree(temp);
}

//****************************************************************************
//
// SendGameDescription ()
//
//****************************************************************************

void SendGameDescription( void )
{
   byte * temp;
   COM_GameMasterType * desc;
   int length;
   int i;

   length=sizeof(COM_GameMasterType);
   temp=SafeMalloc(length);

   memset(temp,0,length);

   desc=(COM_GameMasterType *)temp;
   desc->type=(byte)COM_GAMEMASTER;
   desc->level=gamestate.mapon;
   desc->mapcrc=GetMapCRC (gamestate.mapon);
   desc->mode=gamestate.battlemode;
   desc->violence=gamestate.violence;
   desc->Version = gamestate.Version;
   desc->Product = gamestate.Product;
   desc->teamplay = gamestate.teamplay;
   memcpy( &desc->SpecialsTimes, &gamestate.SpecialsTimes, sizeof( specials ) );
   BATTLE_GetOptions( &( desc->options ) );
   GetMapFileName( &(desc->battlefilename[0]) );
   desc->randomseed=GetRNGindex ( );
   gamestate.randomseed=desc->randomseed;
   desc->ludicrousgibs=battlegibs;
   ludicrousgibs=battlegibs;
//   SetRNGindex ( gamestate.randomseed );
	for (i=0;i<numplayers;i++)
      {
      if (gamestate.Product == ROTT_SHAREWARE)
         PLAYERSTATE[i].player = 0;
		desc->players[i].character    =PLAYERSTATE[i].player;
		desc->players[i].uniformcolor =PLAYERSTATE[i].uniformcolor;
      strcpy ( &(desc->players[i].codename[0]),&(PLAYERSTATE[i].codename[0]));
      }

   if (!networkgame)
      AssignTeams();

   if (IsServer==false)
      {
      WritePacket(temp,length,server);
      }
   else
      {
      BroadcastServerPacket(temp,length); // Send to all
      }

   SafeFree(temp);
}

//****************************************************************************
//
// SetGameDescription ()
//
//****************************************************************************

void SetGameDescription( void * pkt )
{
   COM_GameMasterType * desc;
   word localcrc;
   int i;

   desc=(COM_GameMasterType *)pkt;
   gamestate.mapon=desc->level;
   gamestate.battlemode=desc->mode;
   gamestate.violence=desc->violence;
   gamestate.Version = desc->Version;
   gamestate.Product = desc->Product;
   gamestate.teamplay = desc->teamplay;
   memcpy( &gamestate.SpecialsTimes, &desc->SpecialsTimes, sizeof( specials ) );
   BATTLE_SetOptions( &( desc->options ) );
   gamestate.randomseed=desc->randomseed;
   SetRNGindex ( gamestate.randomseed );
   SetBattleMapFileName( &(desc->battlefilename[0]) );
   localcrc=GetMapCRC (gamestate.mapon);
   ludicrousgibs=desc->ludicrousgibs;
   if (localcrc!=desc->mapcrc)
      Error("You have different maps on your system\n");
   for (i=0;i<numplayers;i++)
      {
		PLAYERSTATE[i].player=desc->players[i].character;
		PLAYERSTATE[i].uniformcolor=desc->players[i].uniformcolor;
      strcpy ( &(PLAYERSTATE[i].codename[0]),
               &(desc->players[i].codename[0])
             );
      }
   AssignTeams();
}

//****************************************************************************
//
// SetPlayerDescription ()
//
//****************************************************************************

void SetPlayerDescription( void * pkt )
{
   COM_GamePlayerType * desc;

   desc=(COM_GamePlayerType *)pkt;
	PLAYERSTATE[desc->player].player=desc->playerdescription.character;
   PLAYERSTATE[desc->player].uniformcolor=desc->playerdescription.uniformcolor;
   strcpy ( &(PLAYERSTATE[desc->player].codename[0]),
            &(desc->playerdescription.codename[0])
          );
   if ( gamestate.Version != desc->Version )
      {
      Error("Player %s is using a different version of ROTT\n",PLAYERSTATE[desc->player].codename);
//      gamestate.Version = desc->Version;
      }

   if ( gamestate.violence > desc->violence )
      {
      gamestate.violence = desc->violence;
      }

   if ( gamestate.Product > desc->Product )
      {
      gamestate.Product = desc->Product;
      }
}

//****************************************************************************
//
// SendGameAck ()
//
//****************************************************************************

void SendGameAck( void )
{
   byte * temp;
   int length;
   COM_GameAckType * desc;

   length=sizeof(COM_GameAckType);
   temp=SafeMalloc(length);
   desc=(COM_GameAckType *)temp;
   desc->type=COM_GAMEACK;
	desc->player=consoleplayer;

   WritePacket(temp,length,server);

   SafeFree(temp);
}

//****************************************************************************
//
// SendGameStart ()
//
//****************************************************************************

void SendGameStart( void )
{
   byte * temp;
   int length;

   length=DUMMYPACKETSIZE;
   temp=SafeMalloc(length);
   *(temp)=(byte)COM_GAMEPLAY;

   if (IsServer==false)
      {
      WritePacket(temp,length,server);
      }
   else
      {
      BroadcastServerPacket(temp,length); // Send to all
      }

   SafeFree(temp);
}

//****************************************************************************
//
// SetupGamePlayer ()
//
//****************************************************************************
void SetupGamePlayer ( void )
{
   int savetime;
   boolean done;
   boolean gameready;

   savetime=GetTicCount();

   done=false;
   gameready=false;

   while (done==false)
      {
		// Setup individual player
      AbortCheck("SetupGamePlayer aborted\n");

      // send Player Description
		if (GetTicCount() >= savetime)
         {
         savetime=GetTicCount()+SETUPTIME;
         if (gameready==false)
            SendPlayerDescription();
         else
            SendGameAck();
         }
      switch (SetupCheckForPacket())
         {
         case scfp_done:
            done=true;
            break;
         case scfp_gameready:
            gameready=true;
            break;
         }
      }
   savetime=GetTicCount()+(VBLCOUNTER/2);

   while (GetTicCount()<savetime)
      {
      SetupCheckForPacket ();
      }
}

//****************************************************************************
//
// AllPlayersReady ()
//
//****************************************************************************
boolean AllPlayersReady ( void )
{
   int i;

	for (i=0;i<numplayers;i++)
      if ((PlayersReady[i]==false) && (PlayerStatus[i]==player_ingame))
         return false;

   return true;
}

//****************************************************************************
//
// GotAllPlayerDescriptions ()
//
//****************************************************************************
boolean GotAllPlayerDescriptions ( void )
{
   int i;

	for (i=0;i<numplayers;i++)
      if ((GotPlayersDesc[i]==false) && (PlayerStatus[i]==player_ingame))
         return false;

   return true;
}

//****************************************************************************
//
// SetupGameMaster ()
//
//****************************************************************************
void SetupGameMaster ( void )
{
   int savetime;
   boolean done;

   memset(GotPlayersDesc,false,sizeof(GotPlayersDesc));
	GotPlayersDesc[consoleplayer]=true;

   memset(PlayersReady,false,sizeof(PlayersReady));
	PlayersReady[consoleplayer]=true;

   savetime=GetTicCount();

   done=false;

   InitializeRNG ();

   while (done==false)
      {
		// Setup individual player

      AbortCheck("SetupGameMaster aborted\n");

      // send Game Description
		if (GetTicCount() >= savetime)
         {
         savetime=GetTicCount()+SETUPTIME;
			if (GotAllPlayerDescriptions()==true)
            SendGameDescription();
         }
      if (AllPlayersReady ()==true)
         {
         SendGameStart();
         SendGameStart();
         done=true;
         }
      SetupCheckForPacket();
      }
   savetime=GetTicCount()+(VBLCOUNTER/2);

   while (GetTicCount()<savetime)
      {
      SetupCheckForPacket ();
      }
}










/*
=============================================================================

   DEMO CODE

=============================================================================
*/
//****************************************************************************
//
// GetDemoFilename ()
//
//****************************************************************************

void GetDemoFilename (int demonumber, char * filename)
{
   strcpy(filename,DATADIR "DEMO0_0.DMO\0");

   filename[4 + strlen(DATADIR)] = (char)('0' + (byte)demonumber);
   filename[6 + strlen(DATADIR)] = (char)('0' + (byte)gamestate.violence);
   FixFilePath(filename);
}
//****************************************************************************
//
// DemoExists ()
//
//****************************************************************************

boolean DemoExists (int demonumber)
{
   char demo[20 + sizeof(DATADIR)];

   GetDemoFilename (demonumber, &demo[0]);
   if (access (demo, F_OK) == 0)
      return true;
   else
   {
      /* Saves the users violence level, only do this once, otherwise
         we might override the saved level with one already modified by us */
      if (predemo_violence == -1)
         predemo_violence = gamestate.violence;
      /* The demos distributed with rott are all for a violence level of 3 */
      gamestate.violence = 3;
      GetDemoFilename (demonumber, &demo[0]);
      if (access (demo, F_OK) == 0)
         return true;
      else
         return false;
   }
}

//****************************************************************************
//
// SaveDemo ()
//
//****************************************************************************

void SaveDemo (int demonumber)
{
   char demo[20 + sizeof(DATADIR)];

   RecordDemoCmd ();
   GetDemoFilename (demonumber, &demo[0]);
   SaveFile (demo, demobuffer, (demoptr-demobuffer));
   FreeDemo();
}

//****************************************************************************
//
// LoadDemo ()
//
//****************************************************************************

void LoadDemo (int demonumber)
{
   char demo[20 + sizeof(DATADIR)];
   int size;

   GetDemoFilename (demonumber, demo);
   if (demobuffer!=NULL)
      FreeDemo();
   size = LoadFile (demo, (void **)&demobuffer);
   playstate = ex_demoplayback;
   demoptr = demobuffer;
   lastdemoptr = (demoptr+size);
   locplayerstate->player=0;
   InitializeWeapons(locplayerstate);
   ResetPlayerstate(locplayerstate);
   InitCharacter();
}

//****************************************************************************
//
// RecordDemo ()
//
//****************************************************************************

void RecordDemo ( void )
{
   DemoHeaderType * DemoHeader;
   int level;

   if (demobuffer!=NULL)
      FreeDemo();
   godmode=0;
   demobuffer = SafeMalloc (DEMOBUFFSIZE);
   demoptr = demobuffer;
   lastdemoptr = demobuffer+DEMOBUFFSIZE;

   // Save off level number

   DemoHeader=(DemoHeaderType *)demoptr;
   demoptr+=sizeof(gamestate);
   memcpy(&(DemoHeader->demostate),&gamestate,sizeof(gamestate));
   demorecord = true;
   locplayerstate->player=0;
   InitializeWeapons(locplayerstate);
   ResetPlayerstate(locplayerstate);
   level=gamestate.mapon;
   InitCharacter();
   gamestate.mapon=level;
   SoftError(">>>>>>>>>>>>Start demo record\n");
}

//****************************************************************************
//
// SetupDemo ()
//
//****************************************************************************

void SetupDemo ( void )
{
   DemoHeaderType * DemoHeader;

   demoplayback = true;
   godmode=0;

   DemoHeader=(DemoHeaderType *)demoptr;
   demoptr+=sizeof(gamestate);
//   if (gamestate.violence!=DemoHeader->demostate.violence)
//      Error ("This demo has a different difficulty level than your current settings\n");
   memcpy(&gamestate,&(DemoHeader->demostate),sizeof(gamestate));
   SoftError(">>>>>>>>>>>>Start demo playback\n");
}

//****************************************************************************
//
// FreeDemo ()
//
//****************************************************************************

void FreeDemo ( void )
{
   demoplayback = false;
   demorecord = false;
   SafeFree (demobuffer);
   demobuffer=NULL;
}

//****************************************************************************
//
// CheckForDemoDone ()
//
//****************************************************************************

void CheckForDemoDone ( void )
{
	if ((demoplayback==true) && (demoptr >= lastdemoptr))
		{
      FreeDemo();
      playstate = ex_demodone;
		}
}

//****************************************************************************
//
// CheckForDemoOverflowed ()
//
//****************************************************************************

void CheckForDemoOverflowed ( void )
{
	if (demoptr >= (lastdemoptr-sizeof(DemoType)))
      {
      playstate = ex_completed;     // demo is done
      EndDemo();
      }
}

//****************************************************************************
//
// RecordDemoCmd ()
//
//****************************************************************************

void RecordDemoCmd (void)
{
   DemoType * dtime;

   SoftError("Demo command recorded at %d\n",controlupdatetime);
   dtime=(DemoType *)demoptr;
   dtime->time = controlupdatetime;
   dtime->momx = (controlbuf[0]>>1);
   dtime->momy = (controlbuf[1]>>1);
   dtime->dangle = controlbuf[2]>>11;
   dtime->buttons = buttonbits;

   demoptr+=sizeof(DemoType);

   CheckForDemoOverflowed();
}

//****************************************************************************
//
// AddDemoCmd ()
//
//****************************************************************************

void AddDemoCmd (void)
{
   DemoType * dtime;

   //
   // get info from demo buffer
   //

   SoftError("Demo command played at %d\n",controlupdatetime);
   if (demoplayback==true)
      {
      dtime=(DemoType *)demoptr;
      controlbuf[0]=dtime->momx<<1;
      controlbuf[1]=dtime->momy<<1;
      controlbuf[2]=dtime->dangle<<11;
      buttonbits   =dtime->buttons;
      demoptr+=sizeof(DemoType);
		}
}

//****************************************************************************
//
// GetNextDemoTime ()
//
//****************************************************************************

int GetNextDemoTime (void)
{
   DemoType * dtime;

   CheckForDemoDone();
   dtime=(DemoType *)demoptr;
   if (demoplayback)
      return dtime->time;
   else
      return -1;
}

//****************************************************************************
//
// UpdateDemoPlayback ()
//
//****************************************************************************

void UpdateDemoPlayback (int time)
{
   if (demoplayback)
      {
      if (GetNextDemoTime()==time)
         AddDemoCmd();
      }
}





