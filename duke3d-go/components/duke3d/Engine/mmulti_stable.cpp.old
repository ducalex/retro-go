
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "enet.h"

//#include "buildqueue.h"

#include <vector>


//#define _DEBUG_NETWORKING_
extern "C"
{

#include "platform.h"
#include "fixedPoint_math.h"
#include "signal.h"
#include "mmulti_stable.h"
#include "filesystem.h"

#define MAXPLAYERS 16
#define BAKSIZ 16384
#define SIMULATEERRORS 0
#define SHOWSENDPACKETS 0
#define SHOWGETPACKETS 0
#define PRINTERRORS 0

#define MAX_PLAYERS 16
#define BUILD_DEFAULT_UDP_PORT 1635  /* eh...why not? */
#define CLIENT_POLL_DELAY 5000  /* ms between pings at peer-to-peer startup. */
#define HEADER_PEER_GREETING 245
#define HEADER_PEER_READY 244

static int udpsocket = -1;
static short udpport = BUILD_DEFAULT_UDP_PORT;

#define updatecrc16(crc,dat) crc = (((crc<<8)&65535)^crctable[((((unsigned short)crc)>>8)&65535)^dat])

static long incnt[MAXPLAYERS], outcntplc[MAXPLAYERS], outcntend[MAXPLAYERS];
static char errorgotnum[MAXPLAYERS];
static char errorfixnum[MAXPLAYERS];
static char errorresendnum[MAXPLAYERS];
#if (PRINTERRORS)
	static char lasterrorgotnum[MAXPLAYERS];
#endif

long crctable[256];
int tmpmax[8]; //addfaz variable addition (you could probs think of something better)
unsigned int g_bAllPlayersFound = 0;

static char lastpacket[576], inlastpacket = 0;
static short lastpacketfrom, lastpacketleng;

extern long totalclock;  /* MUST EXTERN 1 ANNOYING VARIABLE FROM GAME */
static long timeoutcount = 60, resendagaincount = 4, lastsendtime[MAXPLAYERS];

static short bakpacketptr[MAXPLAYERS][256], bakpacketlen[MAXPLAYERS][256];
static char bakpacketbuf[BAKSIZ];
static long bakpacketplc = 0;

short myconnectindex, numplayers;
short connecthead, connectpoint2[MAXPLAYERS];
//char syncstate = 0;

unsigned char g_bWaitingForAllReady = 0;

extern int _argc;
extern char **_argv;

#define MAXPACKETSIZE 2048
typedef struct
{
	short intnum;                /* communication between Game and the driver */
	short command;               /* 1-send, 2-get */
	short other;                 /* dest for send, set by get (-1 = no packet) */
	short numbytes;
	short myconnectindex;
	short numplayers;
	short gametype;              /* gametype: 1-serial,2-modem,3-net */
	short filler;
	char buffer[MAXPACKETSIZE];
	long longcalladdress;
} gcomtype;
static gcomtype *gcom;

gcomtype g_LastPersonalPacket;

typedef struct 
{
	unsigned short other;
	unsigned int bufferSize;
	unsigned char buffer[MAXPACKETSIZE];
}PACKET;

typedef std::vector<PACKET> PacketQueue;
PacketQueue incommingPacketQueue;

//typedef std::vector<PACKET> PacketQueue;

enum ECommitCMDs
{
	COMMIT_CMD_SEND				= 1,
	COMMIT_CMD_GET              = 2,
	COMMIT_CMD_SENDTOALL        = 3,
	COMMIT_CMD_SENDTOALLOTHERS  = 4,
	COMMIT_CMD_SCORE            = 5,
};

typedef enum
{
    udpmode_peer,
    udpmode_server,
    udpmode_client
} udpmodes;
static udpmodes udpmode = udpmode_peer;

enum EConnectionMode
{
	CONN_MODE_CONNECTING	= 0,
	CONN_MODE_GREETING		= 1,
	CONN_MODE_WAITFORREADY	= 2,
	CONN_MODE_CONNECTED		= 3,
	CONN_MODE_DISCONNECTED	= 4,
};
EConnectionMode g_ConnMode = CONN_MODE_CONNECTING;

typedef struct {
  unsigned int host;
  short port;
  unsigned short id;
  unsigned short peer_idx;
  unsigned short bHeardFrom;
  unsigned char  bReady;
} ADDRESS_STRUCT;
ADDRESS_STRUCT	allowed_addresses[MAX_PLAYERS];  /* only respond to these IPs. */


volatile int sb_ctrlc_pressed = 0;
static void sb_siginthandler(int sigint)
{
    sb_ctrlc_pressed = 1;
}

typedef struct
{
    unsigned char dummy1;   /* so these don't confuse game after load. */
    unsigned char dummy2;   /* so these don't confuse game after load. */
    unsigned char dummy3;   /* so these don't confuse game after load. */
    unsigned char header;   /* always HEADER_PEER_GREETING (245). */
    unsigned short id;
} PacketPeerGreeting;

// unique id for determining "myconnectindex"
unsigned short my_id = 0;

unsigned short g_nPlayerIDList[MAX_PLAYERS];

#define CONNECTION_DELAY 3000
#define INITIAL_CONNECTION_DELAY 50
#define INGAME_CONNECTION_DELAY 0
#define POLL_DELAY 1000

//ENetAddress		address;
ENetHost*		g_Server = 0;
//ENetEvent		event;
ENetPeer*		g_Peers[MAX_PLAYERS];

short *g_other;
char *g_bufptr;
short g_nMessageLen;
unsigned char g_bAllGreetingsIn = 0;

// Prototypes
int CreateServer(char* ip, int nPort, int nMaxPlayers);

char *static_ipstring(int ip);
char *read_whole_file(const char *cfgfile);
char *get_token(char **ptr);
int parse_ip(const char *str, int *ip);
int parse_interface(char *str, int *ip, short *udpport);
int parse_udp_config(const char *cfgfile, gcomtype *gcom);

int connect_to_everyone();
void HandleEvent(ENetEvent *pEvent);
unsigned int GetPeerIndex(ENetPeer* peer);
unsigned int GetOtherIndex(ENetPeer* peer);
void ServiceNetwork();
void Send_Peer_Gretting();
void Wait_For_Ready();

void cleanup(void);


	void stable_callcommit(void)
	{
	}
	
	void stable_initcrc(void)
	{
	}

	// Get CRC
	long stable_getcrc(char *buffer, short bufleng)
	{
		long i, j;

		j = 0;
		for(i=bufleng-1;i>=0;i--) updatecrc16(j,buffer[i]);
		return(j&65535);
	}

	void stable_initmultiplayers(char damultioption, char dacomrateoption, char dapriority)
	{
		long i;
		gcomtype *retval;
		int k;

		// DO NOT DO THIS - it screws up the networking
		//srand(enet_time_get_raw());

		for (i = _argc - 1; i > 0; i--)
		{
			const char *arg = _argv[i];
			char ch = *arg;
			if ((ch == '-') || (ch == '/'))
			{
				if (stricmp(arg + 1, "net") == 0)
					break;
			}
		}

		if ((i == 0) || (i+1 == _argc))
		{
			numplayers = 1; myconnectindex = 0;
			connecthead = 0; connectpoint2[0] = -1;
			return;
		}

		// Zero out the peers buffer
		//memset(g_Peers, 0, MAX_PLAYERS);
				
		/*
		for(k = 0; k < MAX_PLAYERS-1; ++k)
		{
			g_Peers[k] = NULL;
		}
		*/
		
		

		if(enet_initialize() == -1)
		{
			printf("Error initializing ENet\n");
		}

		atexit(cleanup);

		retval = (gcomtype *)malloc(sizeof (gcomtype));
		if (retval != NULL)
		{
			int rc;
			char *cfgfile = _argv[i+1];
			void (*oldsigint)(int);

			memset(retval, '\0', sizeof (gcomtype));
			memset(allowed_addresses, '\0', sizeof (allowed_addresses));
			udpsocket = -1;
			udpport = BUILD_DEFAULT_UDP_PORT;
			udpmode = udpmode_peer;

			oldsigint = signal(SIGINT, sb_siginthandler);
			rc = parse_udp_config(cfgfile, retval);
			signal(SIGINT, oldsigint);

			if(!rc)
			{
				printf("Network transport initialization error!\n");
			}

			gcom = retval;
		}
		else
		{
				printf("Error allocating gcomtype!\n");
		}

		numplayers = gcom->numplayers;
//		myconnectindex = gcom->myconnectindex;//numplayers-1;
		#if (SIMULATEERRORS != 0)
			srand(myconnectindex*24572457+345356);
		#endif

		//g_Peers = (ENetPeer**)malloc(sizeof(ENetPeer*) * gcom->numplayers);
		connect_to_everyone();

		connecthead = 0;
		for(i=0;i<numplayers-1;i++)
		{ 
			connectpoint2[i] = i+1;
		}
		connectpoint2[numplayers-1] = -1;

		for(i=0;i<numplayers;i++) 
		{
			lastsendtime[i] = totalclock;
		}



		// Set our connection index
		myconnectindex = gcom->myconnectindex;//numplayers-1;

	}

	void stable_sendpacket(long other, char *bufptr, long messleng)
	{

		if(other == (myconnectindex))
		{
			#ifdef _DEBUG_NETWORKING_
				printf("Send Packet to myself %d : type: %d len: %d\n", other, bufptr[0], messleng);
			#endif
			memcpy(g_LastPersonalPacket.buffer, bufptr, messleng);
			g_LastPersonalPacket.numbytes = (short)messleng;
			g_LastPersonalPacket.command = 1;
		}
		else
		{

			#ifdef _DEBUG_NETWORKING_
				printf("Send Packet to peer %d : type: %d len: %d\n", other, bufptr[0], messleng);
			#endif

			ENetPacket * packet = enet_packet_create (bufptr, sizeof(char) * messleng, ENET_PACKET_FLAG_RELIABLE);//ENET_PACKET_FLAG_RELIABLE
			//enet_peer_send (g_Peers[other], 0, packet);
			enet_peer_send (g_Peers[allowed_addresses[other].peer_idx], 0, packet);
			enet_host_flush(g_Server);

		}
	}

	void stable_setpackettimeout(long datimeoutcount, long daresendagaincount)
	{
		//NOT USED for anything other than '/f4'
	}

	void stable_uninitmultiplayers(void)
	{
		//kill networking

		/*
		if(g_Peers)
		{
			free(g_Peers);
		}
		*/
		incommingPacketQueue.clear();

		enet_deinitialize();
	}

	void cleanup(void)
	{
		stable_uninitmultiplayers();
	}

	void stable_sendlogon(void)
	{
	}

	void stable_sendlogoff(void)
	{
		long i;
		char tempbuf[2];

		tempbuf[0] = 255;
		tempbuf[1] = myconnectindex;
		for(i=connecthead;i>=0;i=connectpoint2[i])
			if (i != myconnectindex)
				stable_sendpacket(i,tempbuf,2L);
	}

	int stable_getoutputcirclesize(void)
	{
		return 0;
	}

	void stable_setsocket(short newsocket)
	{
	}


//-------------------------------------------------
//
//	GetPacket
//
//-------------------------------------------------
	short stable_getpacket(short *other, char *bufptr)
	{	
		ENetEvent event;
		g_nMessageLen = 0;

		
		//clear out the early packet buffer first
		if(incommingPacketQueue.size() > 0)
		{
			PacketQueue::iterator iter = incommingPacketQueue.begin();

			if(iter != incommingPacketQueue.end() )
			{
				g_nMessageLen = (*iter).bufferSize;
				*other = (*iter).other;
				memcpy(bufptr , (*iter).buffer, g_nMessageLen);

				// Delete this entry now that we're done with it.
				incommingPacketQueue.erase(iter);
			}
		}
		else			
		if (enet_host_service (g_Server, & event, INGAME_CONNECTION_DELAY) > 0) 
		{
			// setup the pointers.
			g_other = other;

			HandleEvent(&event);

			if(event.type == ENET_EVENT_TYPE_RECEIVE)
			{
				memcpy(bufptr, &lastpacket[0], g_nMessageLen);
			}

		}else // check to see if we have a packet of our own to deliver to ourselves.
		{
			if(g_LastPersonalPacket.command == 1)
			{
				*other = gcom->numplayers -1;//myconnectindex;
				memcpy(bufptr, &g_LastPersonalPacket.buffer[0], g_nMessageLen);
				
				//reset it
				g_LastPersonalPacket.command = 0;

				return g_LastPersonalPacket.numbytes;
			}
		}

		return g_nMessageLen;
	}

	void stable_flushpackets(void)
	{
		//STUB
	}

	void stable_genericmultifunction(long other, char *bufptr, long messleng, long command)
	{

	}

//
//
//
int connect_to_everyone()
{
	ENetAddress		address;
    ENetEvent		event;
	int i;
	int bWaiting = 1;
	int bCreatedPeers = 0;

	while(bWaiting)
	{
		printf( (g_bAllPlayersFound) ? "." : "Waiting for connections...\n");

		//wait for conencts to/from them
		if (enet_host_service (g_Server, & event, (bCreatedPeers == 1) ? CONNECTION_DELAY : INITIAL_CONNECTION_DELAY) > 0) 
		{
			HandleEvent(&event);
		}

		//Create peer and connect to it
		//enet_address_set_host (& address, m_szAddr);

		if(bCreatedPeers == 0)
		{
			for(i = 0; i < gcom->numplayers-1; ++i)
			{

				char szHostName[64];
	
				address.host = allowed_addresses[i].host; //ip;
				address.port = allowed_addresses[i].port; //m_nPort;

				enet_address_get_host(&address, szHostName, 64);
				printf("Creating peer: %s:%d\n", szHostName, address.port);
	
				g_Peers[i] = enet_host_connect (g_Server, & address, 2); 
	
				if(g_Peers[i] == NULL)
				{
					printf("Error creating peer! \n");
					//return 1;
				}else
				{
					allowed_addresses[i].peer_idx = i;
				}

			}
				bCreatedPeers = 1;
		}

		if(g_bAllPlayersFound == 1)
		{
			bWaiting = 0;
		}

	}

	printf("Negotiating connection order...\n");
	Send_Peer_Gretting();
	Wait_For_Ready();

	return 0;
}

void Send_Peer_Gretting()
{
	int i;

	g_ConnMode = CONN_MODE_GREETING;

    while (my_id == 0)  /* player number is based on id, low to high. */
	{
		my_id = (unsigned short)enet_time_get_raw();//(unsigned short) rand();
	}

	printf("My client id is %d\n", my_id);

	for(i = 0; i < MAX_PLAYERS; ++i)
	{
		allowed_addresses[i].id = 0;
	}

    PacketPeerGreeting greetpacket;
	memset(&greetpacket, '\0', sizeof (greetpacket));
    greetpacket.header = HEADER_PEER_GREETING;
    greetpacket.id = BUILDSWAP_INTEL16(my_id);

	// Create the greeting packet
	ENetPacket * packet = enet_packet_create (&greetpacket, sizeof(PacketPeerGreeting), ENET_PACKET_FLAG_RELIABLE);//ENET_PACKET_FLAG_RELIABLE

	printf("Broadcasting Greating...\n");

	// Broadcast it to all the peers
	enet_host_broadcast(g_Server, 0, packet);

	// Flush the send buffer
	enet_host_flush(g_Server);

	while(CONN_MODE_GREETING == g_ConnMode)
	{
		ENetEvent event;
		if (enet_host_service (g_Server, & event, CONNECTION_DELAY) > 0) 
		{
			int nAllIDsIn = 1;

			
			HandleEvent(&event);	
			
			// are all the id's in yet?
			for(i = 0; i < (gcom->numplayers-1); ++i)
			{
				if(allowed_addresses[i].id == 0)
				{
					nAllIDsIn = 0;
					break;
				}
			}

			// add our ID to the list for sorting
			allowed_addresses[gcom->numplayers-1].id = my_id;

			//check the validity of the ID and sort them.
			if(nAllIDsIn == 1)
			{
				int iteration = 0;
				int k = 0;
				unsigned short nCurrentHigh = 0;

				printf("Sorting player IDs...\n");

				for(iteration = 0; iteration < gcom->numplayers; ++iteration)
				{					
					//g_nPlayerIDList[i]
					for(k = iteration+1; k < gcom->numplayers; ++k)
					{
						if(allowed_addresses[iteration].id == allowed_addresses[k].id)
						{
							printf("ERROR!!!!! Two players with the same Unique ID found, please restart...\n");
						}
						else
						{
							// if it's valid, then goto sort it
							if(allowed_addresses[k].id > allowed_addresses[iteration].id)
							{
								//swap'm
								//unsigned short nTemp = allowed_addresses[iteration];
								ADDRESS_STRUCT tempAddress;

								// Swap the positions
								memcpy(&tempAddress, &allowed_addresses[iteration], sizeof(ADDRESS_STRUCT));								
								memcpy(&allowed_addresses[iteration], &allowed_addresses[k], sizeof(ADDRESS_STRUCT));
								memcpy(&allowed_addresses[k], &tempAddress, sizeof(ADDRESS_STRUCT));
							}
						}
					}
					
				}

				// Find our slot	
				printf("Finding our player index...\n");

				for(i = 0; i < (gcom->numplayers); ++i)
				{	
					printf("Index[%d] = %d\n", i, allowed_addresses[i].id);


					if(allowed_addresses[i].id == my_id)
					{
						gcom->myconnectindex = i;
						printf("You are player #%d\n", i);

						// We're all greated, switch to waiting for all ready
						g_ConnMode = CONN_MODE_WAITFORREADY;
						break;
					}
					
				}		
			}
		}
	}
	
}

void Wait_For_Ready()
{

	g_bWaitingForAllReady = gcom->numplayers-1;

	// Create the greeting packet
	unsigned char message = HEADER_PEER_READY;

	ENetPacket * packet = enet_packet_create (&message, sizeof(unsigned char), ENET_PACKET_FLAG_RELIABLE);//ENET_PACKET_FLAG_RELIABLE

	printf("Broadcasting Ready Packet...\n");

	// Broadcast it to all the peers
	enet_host_broadcast(g_Server, 0, packet);

	// Flush the send buffer
	enet_host_flush(g_Server);

	//g_ConnMode = CONN_MODE_CONNECTED;//CONN_MODE_WAITFORREADY;

	while(g_bWaitingForAllReady > 0)
	{
		ENetEvent event;
		if (enet_host_service (g_Server, & event, CONNECTION_DELAY) > 0) 
		{
			int i;
			
			HandleEvent(&event);

			g_bWaitingForAllReady = gcom->numplayers-1;
			for(i = 0; i < gcom->numplayers; ++i)
			{
				if(allowed_addresses[i].bReady == 1)
				{
					--g_bWaitingForAllReady;
				}
			}

			// Check to make sure we didn't subtract 1 from 0 to make 255. (unsigned char)
			if(g_bWaitingForAllReady > gcom->numplayers)
			{
				printf("Error: we have a problem with the waiting for ready packets...\n");
			}

		}
	}

	printf("All players are ready. Start sending game data...\n");

	g_ConnMode = CONN_MODE_CONNECTED;
}

void HandleEvent(ENetEvent *pEvent)
{
	switch(pEvent->type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				{
					//bServerConnected = true;
					ENetAddress address;
					int i;

					address.host = pEvent->peer->address.host; //ip;
					address.port = pEvent->peer->address.port; //m_nPort;
					char szHostName[64];
					enet_address_get_host(&address, szHostName, 64);
					
					printf("Connection Established with: (%s)\n", szHostName);

					for(i = 0; i < gcom->numplayers-1; ++i)
					{
						if(allowed_addresses[i].host == address.host)
						{
							allowed_addresses[i].bHeardFrom = 1;
						}
					}

					for(i = 0; i < gcom->numplayers-1; ++i)
					{
						if(allowed_addresses[i].bHeardFrom == 0)
						{
							return;
						}
					}

					// All players have been found... YAY!
					g_bAllPlayersFound = 1;
					printf("All Players Connected...\n");

				}
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				{
						g_nMessageLen = (short)pEvent->packet->dataLength;


						
						switch(g_ConnMode)
						{

						case CONN_MODE_GREETING:
							{
								PacketPeerGreeting packet;
								unsigned int nPeerIndex;


								if(pEvent->packet->data[0] != HEADER_PEER_GREETING)
								{
									printf("Invalid greeting!!!!\n");
								}

								printf("Received greeting from (%x)...\n", pEvent->peer->address.host);

								memcpy(&packet, pEvent->packet->data, g_nMessageLen);
								if(packet.header == HEADER_PEER_GREETING)
								{
									// Find the peer's index
									nPeerIndex = GetPeerIndex(pEvent->peer);
		
									// Set the id for the peer
									//g_nPlayerIDList[nPeerIndex] = packet.id;
									allowed_addresses[nPeerIndex].id = packet.id;

								}
							}
							break;
						case CONN_MODE_WAITFORREADY:
							{
								if(pEvent->packet->data[0] == HEADER_PEER_READY)
								{
									allowed_addresses[GetOtherIndex(pEvent->peer)].bReady = 1;
								}
								else
								{
									printf("Invalid READY packet!!!\n");
								}
									

							}
							break;
						case CONN_MODE_CONNECTED:
						default:
							{
								if(g_ConnMode != CONN_MODE_CONNECTED)
								{
									PACKET packet;
									packet.other = GetOtherIndex(pEvent->peer);
									packet.bufferSize = g_nMessageLen;
									memcpy(packet.buffer, pEvent->packet->data, g_nMessageLen);
									incommingPacketQueue.push_back(packet);
									printf("Saving early packet...\n");
									break;
								}

								if(pEvent->packet->data[0] == 16)
								{
									printf("PACKET 16: len:[%d]\n", g_nMessageLen);
								}

								#ifdef _DEBUG_NETWORKING_LEVEL2_
									printf("RECEIVE: type[%d] len:[%d]\n", pEvent->packet->data[0], g_nMessageLen);
								#endif						
								memcpy(&lastpacket[0], pEvent->packet->data, g_nMessageLen);
								*g_other = GetOtherIndex(pEvent->peer);
							}
							break;
						}
						
						/*if(g_nMessageLen > 0)
						{
							switch(pEvent->packet->data[0])
							{
								case HEADER_PEER_GREETING:
								{
									PacketPeerGreeting packet;
									unsigned int nPeerIndex;
									int i;

									printf("Received greeting from (%x)...\n", pEvent->peer->address.host);

									memcpy(&packet, pEvent->packet->data, g_nMessageLen);
									if(packet.header == HEADER_PEER_GREETING)
									{
										// Find the peer's index in the g_Peers[] array
										nPeerIndex = GetPeerIndex(pEvent->peer);
			
										// Set the id for the peer
										//g_nPlayerIDList[nPeerIndex] = packet.id;
										allowed_addresses[nPeerIndex].id = packet.id;

									}
								}
								break;
								case HEADER_PEER_READY:
								{
									unsigned int nOtherIndex;

									printf("Received ready from (%x)...\n", pEvent->peer->address.host);
									nOtherIndex = GetOtherIndex(pEvent->peer);

									allowed_addresses[nOtherIndex].bReady = 1;

								}
								break;


								default:
									{
										
										if((g_bWaitingForAllReady) || (g_ConnMode == CONN_MODE_GREETING))
										{
											PACKET packet;
											packet.other = GetOtherIndex(pEvent->peer);
											packet.bufferSize = g_nMessageLen;
											memcpy(packet.buffer, pEvent->packet->data, g_nMessageLen);
											incommingPacketQueue.push_back(packet);
											printf("Saving early packet...\n");
											break;
										}
										

										#ifdef _DEBUG_NETWORKING_LEVEL2_
											printf("RECEIVE: type[%d] len:[%d]\n", pEvent->packet->data[0], g_nMessageLen);
										#endif						
										memcpy(&lastpacket[0], pEvent->packet->data, g_nMessageLen);

										// find the correct index in the allowed_addresses[] array.
										*g_other = GetOtherIndex(pEvent->peer);
									}
									break;
							}
						}
						else
						{
							printf("Error: we received a Zero length packet!\n");
						}*/

						// Destroy the packet now that we're done with it.
						enet_packet_destroy (pEvent->packet);
				}
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				{
					printf("DISCONNECT: someone left!\n");
				}
				break;
			default:
				{
					printf("Error: unknown event! : %d\n", pEvent->type);
				}
				break;
			}
}

unsigned int GetPeerIndex(ENetPeer* peer)
{
	int i;
	for(i = 0; i < gcom->numplayers; ++i)
	{
		if(peer->address.host == allowed_addresses[i].host)
		{
			return allowed_addresses[i].peer_idx;
			//return i;
		}
	}

	printf("Error: GetPeerIndex failed to find the corrent index!\n");
	return 0;
}

unsigned int GetOtherIndex(ENetPeer* peer)
{
	int i;
	for(i = 0; i < gcom->numplayers; ++i)
	{
		if(peer->address.host == allowed_addresses[i].host)
		{
			return i;//allowed_addresses[i].peer_idx;
			//return i;
		}
	}

	printf("Error: GetOtherIndex failed to find the corrent index!\n");
	return 0;
}

void ServiceNetwork()
{
	ENetEvent event;
	if (enet_host_service (g_Server, & event, INGAME_CONNECTION_DELAY) > 0) 
	{
		HandleEvent(&event);
	}
}

//**************************************************************
//* Network Transport Functions
//**************************************************************
int CreateServer(char* ip, int nPort, int nMaxPlayers)
{

	ENetAddress address;

	printf("Creating server of %d players on port %d.\n", nMaxPlayers, nPort);

    /* Bind the server to the default localhost.
     * A specific host address can be specified by
     * enet_address_set_host (& address, "x.x.x.x");
     */
    address.host = enet_address_set_host (& address, ip);//nIp;//ENET_HOST_ANY;
    /* Bind the server to port 1234. */
    address.port = nPort;

    g_Server = enet_host_create (& address /* the address to bind the server host to */, 
                nMaxPlayers /* allow up to 32 clients and/or outgoing connections */,
                0 /* assume any amount of incoming bandwidth */,
                0 /* assume any amount of outgoing bandwidth */);
   
	if (g_Server == NULL)
    {
		printf("Error creating server!\n");
		return 1;
	}

	return 0;
}

//**************************************************************
//* Network Config File Functions
//**************************************************************

#include "filesystem.h"  /* kopen4load for cfg file. */
#include "display.h"  /* getticks */

#define IPSEG1(ip) ((((unsigned int) ip) & 0xFF000000) >> 24)
#define IPSEG2(ip) ((((unsigned int) ip) & 0x00FF0000) >> 16)
#define IPSEG3(ip) ((((unsigned int) ip) & 0x0000FF00) >>  8)
#define IPSEG4(ip) ((((unsigned int) ip) & 0x000000FF)      )

	char *static_ipstring(int ip)
	{
		static char s[16];
		sprintf(s, "%u.%u.%u.%u", IPSEG1(ip), IPSEG2(ip), IPSEG3(ip), IPSEG4(ip));
		return(s);
	}

	char *read_whole_file(const char *cfgfile)
	{
		char *buf;
		long len, rc;
		long handle;

		if (cfgfile == NULL)
			return(NULL);

		handle = kopen4load(cfgfile,0);
		if (handle == -1)
		{
			printf("ERROR: Failed to open config file [%s].\n", cfgfile);
			return(NULL);
		}

		len = kfilelength(handle);
		buf = (char *) malloc(len + 2);
		if (!buf)
		{
			kclose(handle);
			return(NULL);
		}

		rc = kread(handle, buf, len);
		kclose(handle);
		if (rc != len)
		{
			free(buf);
			return(NULL);
		}

		buf[len] = '\0';
		buf[len+1] = '\0';
		return(buf);
	}

	char *get_token(char **ptr)
	{
		char *retval;
		char *p = *ptr;
		if (*p == '\0')
			return(NULL);

		while ((*p != '\0') && (isspace(*p)))
			p++;

		if (*p == '\0')  /* nothing but whitespace. */
			return(NULL);

		retval = p;
		while ((*p != '\0') && (!isspace(*p)))
			p++;

		*p = '\0';
		*ptr = p + 1;

		/*printf("Got token [%s].\n", retval);*/
		return(retval);
	}

	int parse_ip(const char *str, int *ip)
	{
		int ip1, ip2, ip3, ip4;

		if (stricmp(str, "any") == 0)
		{
			*ip = 0;
			return(1);
		}

		if (sscanf(str, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4) != 4)
		{
			printf("\"%s\" is not a valid IP address.\n", str);
			return(0);
		}

		/* we _should_ check that 0 <= ip? <= 255, but it'll fail later anyhow. */

		*ip = ( ((ip1 & 0xFF) << 24) |
				((ip2 & 0xFF) << 16) |
				((ip3 & 0xFF) <<  8) |
				((ip4 & 0xFF)      ) );

		return(1);
	}

	int parse_interface(char *str, int *ip, short *udpport)
	{
		char *ptr = strchr(str, ':');
		if (ptr) /* portnum specified? */
			*ptr = '\0';

		if (!parse_ip(str, ip))
			return(0);

		*udpport = BUILD_DEFAULT_UDP_PORT;
		if (ptr != NULL)  /* port specified? */
		{
			ptr++;
			if (stricmp(ptr, "any") == 0)
				*udpport = 0;
			else
				*udpport = (short) atoi(ptr);
		}

		return(1);
	}

	int parse_udp_config(const char *cfgfile, gcomtype *gcom)
	{
		char *buf;
		char *tok;
		char *ptr;
		int ip = 0;  /* interface */
		int bcast = 0;

		buf = read_whole_file(cfgfile);  /* we must free this. */
		if (buf == NULL)
			return(0);

		ptr = buf;
		while ((tok = get_token(&ptr)) != NULL)
		{
			int bogus = 1;

			if (stricmp(tok, "interface") == 0)
			{
				if ( (tok = get_token(&ptr)) &&
					 (parse_interface(tok, &ip, &udpport)) )
				{
					bogus = 0;
				}
				printf("Interface %s:%d chosen.\n",
						static_ipstring(ip), (int) udpport);
			}

			else if (stricmp(tok, "mode") == 0)
			{
				if ((tok = get_token(&ptr)) != NULL)
				{
					bogus = 0;
					if (stricmp(tok, "server") == 0)
						udpmode = udpmode_server;
					else if (stricmp(tok, "client") == 0)
						udpmode = udpmode_client;
					else if (stricmp(tok, "peer") == 0)
						udpmode = udpmode_peer;
					else
						bogus = 1;

					if (!bogus)
						printf("You want to be in [%s] mode\n", tok);
				}
			}

			else if (stricmp(tok, "broadcast") == 0)
			{
				if ((tok = get_token(&ptr)) != NULL)
				{
					bcast = atoi(tok);
					if (bcast > MAX_PLAYERS - 1)
					{
						printf("WARNING: Too many broadcast players.\n");
						bcast = MAX_PLAYERS - 1;
					}

					bogus = 0;
				}
			}

			else if (stricmp(tok, "allow") == 0)
			{
				int host;
				short port=BUILD_DEFAULT_UDP_PORT;
				if ((tok = get_token(&ptr)) != NULL)
				{
					if (gcom->numplayers >= MAX_PLAYERS - 1)
						printf("WARNING: Too many allowed IP addresses.\n");

					else if (parse_interface(tok, &host, &port))
					{
						ENetAddress address;
						enet_address_set_host(&address, static_ipstring(host));
						printf("Adding: %s:%d to the list of allowed addresses.\n", static_ipstring(host), port);
						allowed_addresses[gcom->numplayers].host = address.host;
						allowed_addresses[gcom->numplayers].port = port;
						gcom->numplayers++;
						bogus = 0;
					}
				}
			}

			if (bogus)
				printf("bogus token! [%s]\n", tok);
		}

		free(buf);

		// Create the server
		int ret = CreateServer(static_ipstring(ip), udpport, gcom->numplayers);
		gcom->numplayers++; //that's you

		if(ret == 0)
		{
			return 1;
		}

		return(0);
	}




} // end extern "C"

/* end of mmulti.cpp ... */


