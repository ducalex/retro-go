/*
 * "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
 * Ken Silverman's official web site: "http://www.advsys.net/ken"
 * See the included license file "BUILDLIC.TXT" for license info.
 * This file has been modified from Ken Silverman's original release
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>

#include "mmulti_unstable.h"

#if (HAVE_ENET_ENET_H == 1)
	#include <enet/enet.h>
#else
//	#include "enet/include/enet.h"
#endif

//STL
//#include <vector>
//#include "buildqueue.h"

#include "network.h"

#include "fixedPoint_math.h"

#include "esp_attr.h"

#define MAXPLAYERS 16
#define BAKSIZ 16384
#define SIMULATEERRORS 0
#define SHOWSENDPACKETS 0
#define SHOWGETPACKETS 0
#define PRINTERRORS 0

#define updatecrc16(crc,dat) crc = (((crc<<8)&65535)^crctable[((((unsigned short)crc)>>8)&65535)^dat])

void Error (int errorType, char *error, ...);

static long incnt[MAXPLAYERS], outcntplc[MAXPLAYERS], outcntend[MAXPLAYERS];
static char errorgotnum[MAXPLAYERS];
static char errorfixnum[MAXPLAYERS];
static char errorresendnum[MAXPLAYERS];
#if (PRINTERRORS)
	static char lasterrorgotnum[MAXPLAYERS];
#endif

long crctable[256];
int tmpmax[8]; //addfaz variable addition (you could probs think of something better)
int itmp = 0; //addfaz router fix STUN

static char lastpacket[576], inlastpacket = 0;
static short lastpacketfrom, lastpacketleng;

extern long totalclock;  /* MUST EXTERN 1 ANNOYING VARIABLE FROM GAME */
static long timeoutcount = 60, resendagaincount = 4, lastsendtime[MAXPLAYERS];

extern unsigned short g_bStun;

static short bakpacketptr[MAXPLAYERS][256], bakpacketlen[MAXPLAYERS][256];
//EXT_RAM_ATTR 
static char *bakpacketbuf;//[BAKSIZ];
static long bakpacketplc = 0;

short myconnectindex, numplayers;
short connecthead, connectpoint2[MAXPLAYERS];
char syncstate = 0;

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
EXT_RAM_ATTR static gcomtype *gcom;

/*
//
// Packet queue data structs
//
typedef struct packet_wrapper
{
	unsigned long packetnumber;
	gcomtype packet;
} PACKETWRAPPER;

typedef PACKETWRAPPER packet_buffer;

unsigned long currentpacketnumber[MAXPLAYERS];

//
// Packet Buffer
//
packet_buffer pBuff[256];
*/


/*
typedef struct 
{
	unsigned char buffer[MAXPACKETSIZE];
}PACKET;
*/

//typedef std::vector<PACKET> PacketQueue;

enum ECommitCMDs
{
	COMMIT_CMD_SEND				= 1,
	COMMIT_CMD_GET              = 2,
	COMMIT_CMD_SENDTOALL        = 3,
	COMMIT_CMD_SENDTOALLOTHERS  = 4,
	COMMIT_CMD_SCORE            = 5,
};


// Queue of out going packets.
//PacketQueue outgoingPacketQueue;
//outgoingPacketQueue.reserve(128);

gcomtype *init_network_transport(char **ARGV, int argpos);
void deinit_network_transport(gcomtype *gcom);
//void unstable_callcommit(void);
void dosendpackets(long other);


void unstable_initcrc(void)
{
	long i, j, k, a;

	for(j=0;j<256;j++)      /* Calculate CRC table */
	{
		k = (j<<8); a = 0;
		for(i=7;i>=0;i--)
		{
			if (((k^a)&0x8000) > 0)
				a = ((a<<1)&65535) ^ 0x1021;   /* 0x1021 = genpoly */
			else
				a = ((a<<1)&65535);
			k = ((k<<1)&65535);
		}
		crctable[j] = (a&65535);
	}
}


long unstable_getcrc(char *buffer, short bufleng)
{
	long i, j;

	j = 0;
	for(i=bufleng-1;i>=0;i--) updatecrc16(j,buffer[i]);
	return(j&65535);
}

void unstable_initmultiplayers(char damultioption, char dacomrateoption, char dapriority)
{
    bakpacketbuf = malloc(sizeof(char)*BAKSIZ);
	long i;

	unstable_initcrc();
	for(i=0;i<MAXPLAYERS;i++)
	{
		incnt[i] = 0L;
		outcntplc[i] = 0L;
		outcntend[i] = 0L;
		bakpacketlen[i][255] = -1;
	}

	// clear out the packet ordering
//	memset(&currentpacketnumber, 0, sizeof(unsigned long) * MAXPLAYERS);

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

    gcom = init_network_transport(_argv, i+1);
    if (gcom == NULL)
	{
        Error(EXIT_SUCCESS, "Network transport initialization failed!\n"
							"Be sure you have closed *all* the previous instances of\n"
							"the xDuke and that no other application is using the\n"
							"same port# as Duke. If you can't solve this problem, try\n"
							"changing the port# or reboot.\n");
	}

	numplayers = gcom->numplayers;
	myconnectindex = gcom->myconnectindex-1;
#if (SIMULATEERRORS != 0)
	srand(myconnectindex*24572457+345356);
#endif
	connecthead = 0;
	for(i=0;i<numplayers-1;i++) connectpoint2[i] = i+1;
	connectpoint2[numplayers-1] = -1;

	for(i=0;i<numplayers;i++) lastsendtime[i] = totalclock;
}


void dosendpackets(long other)
{
	long i, j, k, messleng;
	unsigned short dacrc;

	if (outcntplc[other] == outcntend[other]) return;

#if (PRINTERRORS)
	if (errorgotnum[other] > lasterrorgotnum[other])
	{
		lasterrorgotnum[other]++;
		printf(" MeWant %ld",incnt[other]&255);
	}
#endif

	if (outcntplc[other]+1 == outcntend[other])
	{     /* Send 1 sub-packet */
		k = 0;
		gcom->buffer[k++] = (outcntplc[other]&255);
		gcom->buffer[k++] = (errorgotnum[other]&7)+((errorresendnum[other]&7)<<3);
		gcom->buffer[k++] = (incnt[other]&255);

		j = bakpacketptr[other][outcntplc[other]&255];
		messleng = bakpacketlen[other][outcntplc[other]&255];
		for(i=0;i<messleng;i++)
			gcom->buffer[k++] = bakpacketbuf[(i+j)&(BAKSIZ-1)];
		outcntplc[other]++;
	}
	else
	{     /* Send 2 sub-packets */
		k = 0;
		gcom->buffer[k++] = (outcntplc[other]&255);
		gcom->buffer[k++] = (errorgotnum[other]&7)+((errorresendnum[other]&7)<<3)+128;
		gcom->buffer[k++] = (incnt[other]&255);

			/* First half-packet */
		j = bakpacketptr[other][outcntplc[other]&255];
		messleng = bakpacketlen[other][outcntplc[other]&255];
		gcom->buffer[k++] = (char)(messleng&255);
		gcom->buffer[k++] = (char)(messleng>>8);
		for(i=0;i<messleng;i++)
			gcom->buffer[k++] = bakpacketbuf[(i+j)&(BAKSIZ-1)];
		outcntplc[other]++;

			/* Second half-packet */
		j = bakpacketptr[other][outcntplc[other]&255];
		messleng = bakpacketlen[other][outcntplc[other]&255];
		for(i=0;i<messleng;i++)
			gcom->buffer[k++] = bakpacketbuf[(i+j)&(BAKSIZ-1)];
		outcntplc[other]++;

	}

	dacrc = getcrc(gcom->buffer,(short)k);
	gcom->buffer[k++] = (dacrc&255);
	gcom->buffer[k++] = (dacrc>>8);

	gcom->other = other+1;
	gcom->numbytes = k;

#if (SHOWSENDPACKETS)
	printf("Send(%ld): ",gcom->other);
	for(i=0;i<gcom->numbytes;i++) printf("%2x ",gcom->buffer[i]);
	printf("\n");
#endif

#if (SIMULATEERRORS != 0)
	if (!(rand()&SIMULATEERRORS)) gcom->buffer[rand()%gcom->numbytes] = (rand()&255);
	if (rand()&SIMULATEERRORS)
#endif
		{ 
			gcom->command = COMMIT_CMD_SEND; 
			unstable_callcommit(); 
		}
}


void unstable_sendpacket(long other, char *bufptr, long messleng)
{
	long i = 0;
    long j = 0;

	if (numplayers < 2) return;

	i = 0;
	if (bakpacketlen[other][(outcntend[other]-1)&255] == messleng)
	{
		j = bakpacketptr[other][(outcntend[other]-1)&255];
		for(i=messleng-1;i>=0;i--)
			if (bakpacketbuf[(i+j)&(BAKSIZ-1)] != bufptr[i]) break;
	}
	bakpacketlen[other][outcntend[other]&255] = messleng;

	if (i < 0)   /* Point to last packet to save space on bakpacketbuf */
		bakpacketptr[other][outcntend[other]&255] = j;
	else
	{
		bakpacketptr[other][outcntend[other]&255] = bakpacketplc;
		for(i=0;i<messleng;i++)
			bakpacketbuf[(bakpacketplc+i)&(BAKSIZ-1)] = bufptr[i];
		bakpacketplc = ((bakpacketplc+messleng)&(BAKSIZ-1));
	}
	outcntend[other]++;

	lastsendtime[other] = totalclock;
	dosendpackets(other);
}


void unstable_setpackettimeout(long datimeoutcount, long daresendagaincount)
{
	// Don't do this it keeps '/f4' from working
	// Though /f4 feels weird on my mouse.... slugish is the word...
	/*
	long i;

	timeoutcount = datimeoutcount;
	resendagaincount = daresendagaincount;
	for(i=0;i<numplayers;i++) lastsendtime[i] = totalclock;
	*/
}


void unstable_uninitmultiplayers(void)
{
    deinit_network_transport(gcom);
    gcom = NULL;
    free(bakpacketbuf);
}

void unstable_sendlogon(void)
{
}

void unstable_sendlogoff(void)
{
	long i;
	char tempbuf[2];

	tempbuf[0] = 255;
	tempbuf[1] = myconnectindex;
	for(i=connecthead;i>=0;i=connectpoint2[i])
		if (i != myconnectindex)
			sendpacket(i,tempbuf,2L);
}

int unstable_getoutputcirclesize(void)
{
	return(0);
}

void unstable_setsocket(short newsocket)
{
}


short unstable_getpacket (short *other, char *bufptr)
{
	long i, messleng;
	unsigned short dacrc;

	if (numplayers < 2) return(0);

	for(i=connecthead;i>=0;i=connectpoint2[i])
		if (i != myconnectindex)
		{
			if (totalclock < lastsendtime[i]) lastsendtime[i] = totalclock;
			if (totalclock > lastsendtime[i]+timeoutcount)
			{
#if (PRINTERRORS)
					printf(" TimeOut!");
#endif
					errorgotnum[i] = errorfixnum[i]+1;

					if ((outcntplc[i] == outcntend[i]) && (outcntplc[i] > 0))
						{ outcntplc[i]--; lastsendtime[i] = totalclock; }
					else
						lastsendtime[i] += resendagaincount;
					dosendpackets(i);
				/* } */
			}
		}

	if (inlastpacket != 0)
	{
			/* 2ND half of good double-packet */
		inlastpacket = 0;
		*other = lastpacketfrom;
		memcpy(bufptr,lastpacket,lastpacketleng);
		return(lastpacketleng);
	}

	gcom->command = COMMIT_CMD_GET;
	unstable_callcommit();

#if (SHOWGETPACKETS)
	if (gcom->other != -1)
	{
		printf(" Get(%ld): ",gcom->other);
		for(i=0;i<gcom->numbytes;i++) printf("%2x ",gcom->buffer[i]);
		printf("\n");
	}
#endif

	if (gcom->other < 0) return(0);
	*other = gcom->other-1;

	messleng = gcom->numbytes;

	dacrc = ((unsigned short)gcom->buffer[messleng-2]);
	dacrc += (((unsigned short)gcom->buffer[messleng-1])<<8);
	if (dacrc != getcrc(gcom->buffer,(short)(messleng-2)))        /* CRC check */
	{
#if (PRINTERRORS)
		printf("\n%ld CRC",gcom->buffer[0]);
#endif
		errorgotnum[*other] = errorfixnum[*other]+1;
		return(0);
	}

	while ((errorfixnum[*other]&7) != ((gcom->buffer[1]>>3)&7))
		errorfixnum[*other]++;

	if ((gcom->buffer[1]&7) != (errorresendnum[*other]&7))
	{
		errorresendnum[*other]++;
		outcntplc[*other] = (outcntend[*other]&0xffffff00)+gcom->buffer[2];
		if (outcntplc[*other] > outcntend[*other]) outcntplc[*other] -= 256;
	}

	if (gcom->buffer[0] != (incnt[*other]&255))   /* CNT check */
	{
		if (((incnt[*other]-gcom->buffer[0])&255) > 32)
		{
			errorgotnum[*other] = errorfixnum[*other]+1;
#if (PRINTERRORS)
			printf("\n%ld CNT",gcom->buffer[0]);
#endif
		}
#if (PRINTERRORS)
		else
		{
			if (!(gcom->buffer[1]&128))           /* single else double packet */
				printf("\n%ld cnt",gcom->buffer[0]);
			else
			{
				if (((gcom->buffer[0]+1)&255) == (incnt[*other]&255))
				{
								 /* GOOD! Take second half of double packet */
#if (PRINTERRORS)
					printf("\n%ld-%ld .� ",gcom->buffer[0],(gcom->buffer[0]+1)&255);
#endif
					messleng = ((long)gcom->buffer[3]) + (((long)gcom->buffer[4])<<8);
					lastpacketleng = gcom->numbytes-7-messleng;
					memcpy(bufptr,&gcom->buffer[messleng+5],lastpacketleng);
					incnt[*other]++;
					return(lastpacketleng);
				}
				else
					printf("\n%ld-%ld cnt ",gcom->buffer[0],(gcom->buffer[0]+1)&255);
			}
		}
#endif
		return(0);
	}

		/* PACKET WAS GOOD! */
	if ((gcom->buffer[1]&128) == 0)           /* Single packet */
	{
#if (PRINTERRORS)
		printf("\n%ld �  ",gcom->buffer[0]);
#endif

		messleng = gcom->numbytes-5;

		memcpy(bufptr,&gcom->buffer[3],messleng);

		incnt[*other]++;
		return(messleng);
	}

														 /* Double packet */
#if (PRINTERRORS)
	printf("\n%ld-%ld �� ",gcom->buffer[0],(gcom->buffer[0]+1)&255);
#endif

	messleng = ((long)gcom->buffer[3]) + (((long)gcom->buffer[4])<<8);
	lastpacketleng = gcom->numbytes-7-messleng;
	inlastpacket = 1; lastpacketfrom = *other;

	memcpy(bufptr,&gcom->buffer[5],messleng);
	memcpy(lastpacket,&gcom->buffer[messleng+5],lastpacketleng);

	incnt[*other] += 2;
	return(messleng);
}

void unstable_flushpackets()
{
#if 0
	long i;

	if (numplayers < 2) return;

	do
	{
		gcom->command = COMMIT_CMD_GET;
		callcommit();
	} while (gcom->other >= 0);

	for(i=connecthead;i>=0;i=connectpoint2[i])
	{
		incnt[i] = 0L;
		outcntplc[i] = 0L;
		outcntend[i] = 0L;
		errorgotnum[i] = 0;
		errorfixnum[i] = 0;
		errorresendnum[i] = 0;
		lastsendtime[i] = totalclock;
	}
#endif
}

void unstable_genericmultifunction(long other, char *bufptr, long messleng, long command)
{
	if (numplayers < 2) return;

	gcom->command = command;
	gcom->numbytes = min(messleng,MAXPACKETSIZE);
	copybuf(bufptr,gcom->buffer,(gcom->numbytes+3)>>2);
	gcom->other = other+1;
	callcommit();
	
}


#if STUB_NETWORKING
gcomtype *init_network_transport(char **ARGV, int argpos)
{
    printf("No networking support built in.\n");
    return NULL;
} /* init_network_transport */

void deinit_network_transport(gcomtype *gcom)
{
}

void callcommit(void)
{
}

#elif (defined PLATFORM_DOS)
gcomtype *init_network_transport(char **ARGV, int argpos)
{
    /*
     * How to talk to COMMIT is passed as a pointer to a block of memory
     *  that COMMIT.EXE configures...
     */
	return((gcomtype *)atol(ARGV[argpos]));  /* UGH!  --ryan. */
} /* init_network_transport */

static union REGS regs;

#pragma aux longcall =\
	"call eax",\
	parm [eax]

void callcommit(void)
{
	if (gcom->intnum&0xff00)
		longcall(gcom->longcalladdress);
	else
		int386(gcom->intnum,&regs,&regs);
}

void deinit_network_transport(gcomtype *gcom)
{
    /* no-op, apparently. */
}


#elif UDP_NETWORKING

#if PLATFORM_WIN32
#  include <winsock.h>
#  define EAGAIN WSAEWOULDBLOCK
#  define EWOULDBLOCK WSAEWOULDBLOCK
#  define ECONNREFUSED WSAECONNRESET
#  define socklen_t size_t
#  define netstrerror() win32netstrerror()
#  define neterrno() WSAGetLastError()
#  define sockettype SOCKET
#  define socketclose(x) closesocket(x)
#else
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <netdb.h>
#  include <sys/uio.h>
#  include <sys/ioctl.h>
#  include <sys/time.h>
#  include <errno.h>
#  include <fcntl.h>
#  include <time.h>
#  define netstrerror() strerror(errno)
#  define neterrno() errno
#  define sockettype int
#  define socketclose(x) close(x)
#  ifndef MSG_ERRQUEUE  /* legacy glibc header workaround... */
#    define MSG_ERRQUEUE 0x2000
#  endif
#endif

#define SOCKET_SHUTDOWN_BOTH 2

#include <signal.h>
#include "filesystem.h"  /* kopen4load for cfg file. */
#include "display.h"  /* getticks */

#define IPSEG1(ip) ((((unsigned int) ip) & 0xFF000000) >> 24)
#define IPSEG2(ip) ((((unsigned int) ip) & 0x00FF0000) >> 16)
#define IPSEG3(ip) ((((unsigned int) ip) & 0x0000FF00) >>  8)
#define IPSEG4(ip) ((((unsigned int) ip) & 0x000000FF)      )

#define MAX_PLAYERS 16
#define BUILD_DEFAULT_UDP_PORT 1635  /* eh...why not? */
#define CLIENT_POLL_DELAY 3000  /* ms between pings at peer-to-peer startup. */
#define HEADER_PEER_GREETING 245

static sockettype udpsocket = -1;
static short udpport = BUILD_DEFAULT_UDP_PORT;

static struct {
  int host;
  short port;
} allowed_addresses[MAX_PLAYERS];  /* only respond to these IPs. */

volatile int ctrlc_pressed = 0;
static void siginthandler(int sigint)
{
    ctrlc_pressed = 1;
}

#if PLATFORM_WIN32
/*
 * Figure out what the last failing Win32 API call was, and
 *  generate a human-readable string for the error message.
 *
 * The return value is a static buffer that is overwritten with
 *  each call to this function.
 *
 * Code lifted from PhysicsFS: http://icculus.org/physfs/
 */
static const char *win32netstrerror(void)
{
    static TCHAR msgbuf[255];
    TCHAR *ptr = msgbuf;

    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        WSAGetLastError(), /*GetLastError(),*/
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), /* Default language */
        msgbuf,
        sizeof (msgbuf) / sizeof (TCHAR),
        NULL 
    );

        /* chop off newlines. */
    for (ptr = msgbuf; *ptr; ptr++)
    {
        if ((*ptr == '\n') || (*ptr == '\r'))
        {
            *ptr = ' ';
            break;
        } /* if */
    } /* for */

    return((const char *) msgbuf);
} /* win32strerror */
#endif


typedef enum
{
    udpmode_peer,
    udpmode_server,
    udpmode_client
} udpmodes;
static udpmodes udpmode = udpmode_peer;


static char *static_ipstring(int ip)
{
    static char s[16];
    sprintf(s, "%u.%u.%u.%u", IPSEG1(ip), IPSEG2(ip), IPSEG3(ip), IPSEG4(ip));
    return(s);
}


static int send_udp_packet(int ip, short port, void *pkt, size_t pktsize)
{
    /* !!! FIXME: See if this would ever block. */
    /* !!! FIXME: See if this would send a partial packet. */
    struct sockaddr_in addr;
    int rc;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(ip);
	addr.sin_port = htons(port);

    rc = sendto(udpsocket, pkt, pktsize, 0,
                (struct sockaddr *) &addr, sizeof (addr));

    if (rc != (int) pktsize)
    {
        printf("sendto err rc==%d when sending %d to %s:%d [%s].\n",
                rc, (int) pktsize, static_ipstring(ip), (int) port,
                netstrerror());
        return(0);
    }

/*printf("Sent %d byte packet to %s:%d\n", (int) pktsize, static_ipstring(ip), (int) port);*/

    return(1);
}


static void process_udp_send_queue(void)
{
    /* Fill this in if the non-blocking socket becomes an issue. */
}


static int get_udp_packet(int *ip, short *_port, void *pkt, size_t pktsize)
{
    int err = 0;
    struct sockaddr_in addr;
    short port;
    socklen_t fromlen = sizeof (addr);
    int valid = 0;
    int i;

    /* FIXME: Will this ever receive a partial packet? */
    int rc = recvfrom(udpsocket, pkt, pktsize, 0,
                      (struct sockaddr *) &addr, (int *)&fromlen);
	
	if (rc == -1)
        err = neterrno();

#if !PLATFORM_WIN32
	/* !!! FIXME: Linux specific? */
    if (rc == -1)  /* fill in the addr structure on error... */
    {
        socklen_t l = sizeof (addr);
        recvfrom(udpsocket, NULL, 0, MSG_ERRQUEUE,
                 (struct sockaddr *) &addr, &l);
    }
#endif

    *ip = ntohl(addr.sin_addr.s_addr);
    port = ntohs(addr.sin_port);
    if (_port)
        *_port = port;

    /*
     * Reject packets from unallowed IPs. Prevents (ha) DoS attacks and
     *  other confusion...
     */
    if (gcom == NULL)
	{
		if(g_bStun) //if stun is enabled
		{
			if ((allowed_addresses[itmp].host == *ip)
			&& (allowed_addresses[itmp].port == port))
			{
				valid = 1; //only accept packets from the current player (itmp) and no one else
			}
			else
			{
				return 0; //reject packets from other players
			}
		}
		else
        {
			valid = 1;
		}
	}
    else
    {
        for (i = 1; i <= gcom->numplayers; i++)
        {
            if ((allowed_addresses[i].host == *ip)
				&& (allowed_addresses[i].port == port))

            {
				valid = i;

				//printf ("Packet received from %s:%d\n",static_ipstring(*ip), (int) port);
				
                break;
            }

		}
    }

    /*if (!valid)
    {
        
        static int unallowed_ip_spam = 0;
        if (unallowed_ip_spam <= 100)
        {
            printf("Packet received from unallowed IP %s:%d\n",
                    static_ipstring(*ip), (int) port);
            if (unallowed_ip_spam == 100)
                printf("(Disabling further unallowed IP spam.)\n");
            unallowed_ip_spam++;
        }
        
        return(0);
    } */

    if (rc == -1)
    {
        if ((err == EAGAIN) || (err == EWOULDBLOCK))
            rc = 0;

        else if (err == ECONNREFUSED)   //"connection reset by peer" in winsock 
        {
            
             //  This means that we sent a packet to an unopened port, and
             //  it responded by telling us to piss off. Take them out of the
             //  allowed list. We check gcom so that we don't worry about this
             //  during detection when game might still be loading elsewhere.
              
            if (gcom != NULL)
            {
                allowed_addresses[valid].host = 0;
                printf("%s:%d refused packets. He either crashed or quited abruptly. Please restart.\n",
                        static_ipstring(*ip), (int) port);
            }
            // !!! FIXME: Actually boot player, too. 
        }

        else
        {
            printf("recvfrom err rc==%d when getting %d from %s:%d [%s].\n",
                    rc, (int) pktsize, static_ipstring(*ip), (int) port,
                    netstrerror());
        }
    } 
//else printf("Got %d byte packet from %s:%d\n", (int) rc, static_ipstring(*ip), (int) port);
	//printf( "IP from client %d", *ip);
    return(rc);
}


static char *read_whole_file(const char *cfgfile)
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

static char *get_token(char **ptr)
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

static int set_socket_blockmode(int onOrOff)
{
    unsigned long flags;
    int rc = 0;

    /* set socket to be (non-)blocking. */

#if PLATFORM_WIN32
    flags = (onOrOff) ? 0 : 1;
    rc = (ioctlsocket(udpsocket, FIONBIO, &flags) == 0);
#else
    flags = fcntl(udpsocket, F_GETFL, 0);
    if (flags != -1)
    {
        if (onOrOff)
            flags &= ~O_NONBLOCK;
        else
    	    flags |= O_NONBLOCK;
	    rc = (fcntl(udpsocket, F_SETFL, flags) == 0);
    }
#endif

    if (!rc)
    {
        printf("set socket %sblocking failed: %s\n",
            ((onOrOff) ? "" : "non-"), netstrerror());
    }

    return(rc);
}


static int set_socket_broadcast(int onOrOff)
{
    int f = (onOrOff) ? 1 : 0;
    int rc;

    /* give socket clearance to broadcast. */
    rc = setsockopt(udpsocket, SOL_SOCKET, SO_BROADCAST, (char *)(&f), sizeof (f)) == 0;
    if (!rc)
    {
        printf("%sset SO_BROADCAST failed: %s\n",
            ((onOrOff) ? "" : "un"), netstrerror());
    }

    return(rc);
}


static int open_udp_socket(int ip, int port)
{
    struct sockaddr_in addr;

    printf("Setting up UDP interface %s:%d...\n", static_ipstring(ip), port);
	printf("Stun is currently %s\n", (g_bStun) ? "Enabled":"Disabled");

    udpsocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpsocket == -1)
    {
        printf("socket creation failed: %s\n", netstrerror());
        return(0);
    }

    if (!set_socket_blockmode(0))
        return(0);

    #if !PLATFORM_WIN32
    {
        /* !!! FIXME: Might be Linux (not Unix, not BSD, not WinSock) specific. */
        int flags = 1;
        setsockopt(udpsocket, SOL_IP, IP_RECVERR, &flags, sizeof (flags));
    }
    #endif

    memset(&addr, '\0', sizeof (addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(ip);
    addr.sin_port = htons((unsigned short)port);
    if (bind(udpsocket, (struct sockaddr *) &addr, sizeof (addr)) == -1)
    {
        printf("socket binding failed: %s\n", netstrerror());
        return(0);
    }

    return(1);
}

/* server init. */
static int wait_for_other_players(gcomtype *gcom, int myip)
{
    printf("Server code NOT implemented!\n");
    return(0);
}

/* client init. */
static int connect_to_server(gcomtype *gcom, int myip)
{
    printf("Client code NOT implemented!\n");
    return(0);
}

typedef struct
{
    unsigned char dummy1;   /* so these don't confuse game after load. */
    unsigned char dummy2;   /* so these don't confuse game after load. */
    unsigned char dummy3;   /* so these don't confuse game after load. */
    unsigned char header;   /* always HEADER_PEER_GREETING (245). */
    unsigned short id;
} PacketPeerGreeting;


static void send_peer_greeting(int ip, short port, short myid)
{
    PacketPeerGreeting packet;
	memset(&packet, '\0', sizeof (packet));
    packet.header = HEADER_PEER_GREETING;
    packet.id = BUILDSWAP_INTEL16(myid);
    send_udp_packet(ip, port, &packet, sizeof (packet));
}


/* peer to peer init. */
static int connect_to_everyone(gcomtype *gcom, int myip, int bcast)
{
    PacketPeerGreeting packet;
    unsigned short my_id = 0;
    int i;
    int rc;
    int ip;
    short port;
    int first_send = 1;
    unsigned short heard_from[MAX_PLAYERS];
    unsigned long resendat;
    int max;
    int remaining;

    printf("peer-to-peer init. CTRL-C to abort...\n");

    if (bcast)
    {
        if (gcom->numplayers > 1)
        {
            printf("ERROR: Can't do both 'broadcast' and 'allow'.\n");
            return(0);
        }

        if (!set_socket_broadcast(1))
            return(0);

        gcom->numplayers = bcast + 1;
    }

    memset(heard_from, '\0', sizeof (heard_from));

    while (my_id == 0)  /* player number is based on id, low to high. */
        my_id = (unsigned short) rand();

    printf("(This client's ID for this round is 0x%X.)\n\n", my_id);

    resendat = getticks();
    remaining = max = gcom->numplayers - 1;

    printf("Waiting for %d player%s...\n", remaining, remaining==1 ? "":"s");
    if (remaining == 0)
    {
        printf("Hmmm...don't have time to play with myself.\n");
        return(0);
    }

    while ((remaining) && (!ctrlc_pressed))
    {
        if (resendat <= getticks())
        {
            if (bcast)
            {
                printf("%sroadcasting greeting...\n", first_send ? "B":"Reb");
                /* !!! FIXME: This is...probably not right. */
                send_peer_greeting(0xFFFFFFFF, udpport, my_id);
            }
            else
            {
                for (i = 0; (i < max) || g_bStun ; i++)
                {
				
					//only checking one player at a time works
					//this is where special formatting of allow lines comes in    
					if(g_bStun)
					{
						i = itmp; //addfaz router fix
					}
	
					if (!heard_from[i])
					{
	                   printf("%sending greeting to %s:%d...\n",
                                first_send ? "S" : "Res",
                                static_ipstring(allowed_addresses[i].host),
                                allowed_addresses[i].port);
    
					   send_peer_greeting(allowed_addresses[i].host,
                                           allowed_addresses[i].port,
                                           my_id);
                    }

					// If this is stun-enabled then don't loop.
					if(g_bStun)
					{
						break;
					}
                }
            }
            first_send = 0;
            resendat += CLIENT_POLL_DELAY;
        }

        _idle();
        process_udp_send_queue();

        rc = get_udp_packet(&ip, &port, &packet, sizeof (packet));

		//this is so we don't get unexpected packet errors from players already heard from
		
		if(g_bStun)	
		{
			//addfaz router/stun addition *Start*
			//this is so we are not dealing with players already heard from.
			for (i = 0; i < max; i++)
			{
				if (ip == allowed_addresses[i].host &&
				port == allowed_addresses[i].port)
				{
					if (heard_from[i] != 0) //if we've heard from player already.
					{
						rc = 0;
					}	     
				}
			}
			//addfaz router/stun addition *End*
		}


        if ( (rc > 0) && (ip) && ((ip != myip) || (port != udpport)) )
        {
            char *ipstr = static_ipstring(ip);

            for (i = 0; i < max; i++)
            {

                ////addfaz NAT addition *START*////
				if(!g_bStun)
				{
					if(tmpmax[i] != 1)
					{					
						if (allowed_addresses[i].host == ip)
						{
							if(allowed_addresses[i].port != port)
							{	
								printf("Different player Port Number detected. %s:%i\n",ipstr,
								allowed_addresses[i].port);
								printf("Changed to %s:%i, player may be behind a firewall.\n", ipstr, port); //addfaz NAT addition
								allowed_addresses[i].port = port;
							}					
						}
					}
				}
				////addfaz NAT addition *END*////

				if ((ip == allowed_addresses[i].host) &&
                    (port == allowed_addresses[i].port))  //addfaz NAT line addition
                {
                    break;
                }

                if ((bcast) && (allowed_addresses[i].host == 0))
                    break;  /* select this slot. */
				
            }

            if (i == max)
                printf("%s:%d is not an allowed player.\n", ipstr, port);

            else if (rc != sizeof (packet))
                printf("Missized packet/packet fragment from %s:%i\n", ipstr, port);

            else if (packet.header != HEADER_PEER_GREETING)
                printf("Unexpected packet type from %s:%i\n", ipstr, port);
			
            else if (heard_from[i] == 0)
            {
                packet.id = BUILDSWAP_INTEL16(packet.id);
                heard_from[i] = packet.id;
                allowed_addresses[i].host = ip;   /* bcast needs this. */
                allowed_addresses[i].port = port;
                remaining--;

                printf("Heard from %s:%i (id 0x%X). %d player%s to go.\n",
                        ipstr, port ,(int) packet.id,
                        remaining, remaining == 1 ? "" : "s");

                /* make sure they've heard from us at all... */
                /* !!! FIXME: Could be fatal if packet is dropped... */
                send_peer_greeting(allowed_addresses[i].host,
                                   allowed_addresses[i].port,
                                   my_id);
		
				if(g_bStun)
				{
					itmp++; //addfaz router/stun addition (goto next player)
				}
				else
				{
					tmpmax[i] = 1; //addfaz line addition
				}
            }
        }
    }
	
    if (ctrlc_pressed)
    {
        printf("Connection attempt aborted.\n");
        return(0);
    }

    /* ok, now everyone is talking to you. Sort them into player numbers... */

    heard_from[max] = my_id; /* so we sort, too... */
    allowed_addresses[max].host = myip;
    allowed_addresses[max].port = udpport;

    do
    {
        remaining = 0;
        for (i = 0; i < max; i++)
        {
            if (heard_from[i] == heard_from[i+1])  /* blah. */
            {
                printf("ERROR: Two players have the same random ID!\n");
                printf("ERROR: Please restart the game to generate new IDs.\n");
                return(0);
            }

            else if (heard_from[i] > heard_from[i+1])
            {
                int tmpi;
                short tmps;

                tmps = heard_from[i];
                heard_from[i] = heard_from[i+1];
                heard_from[i+1] = tmps;

                tmpi = allowed_addresses[i].host;
                allowed_addresses[i].host = allowed_addresses[i+1].host;
                allowed_addresses[i+1].host = tmpi;

                tmps = allowed_addresses[i].port;
                allowed_addresses[i].port = allowed_addresses[i+1].port;
                allowed_addresses[i+1].port = tmps;

                remaining = 1;  /* yay for bubble sorting! */
            }
        }
    } while (remaining);

    /*
     * Now we're sorted. But, the local player is referred to by both his
     *  player number and player index ZERO, so bump everyone up one to
     *  their actual index and fill in local player as item zero.
     */

    memmove(&allowed_addresses[1], &allowed_addresses[0],
            sizeof (allowed_addresses) - sizeof (allowed_addresses[0]));
    allowed_addresses[0].host = myip;

    gcom->myconnectindex = 0;
    for (i = 1; i <= gcom->numplayers; i++)
    {
        ip = (allowed_addresses[i].host);


        if (ip == myip)
		{
            if (udpport == allowed_addresses[i].port)
				gcom->myconnectindex = i;
		}

        printf("%s:%i is player #%i.\n", static_ipstring(ip),allowed_addresses[i].port,i);
    }
    assert(gcom->myconnectindex);

    printf("Everyone ready! We are player #%i\n", gcom->myconnectindex);

    /*
     * Ok, we should have specific IPs and ports for all players, and
     *  therefore shouldn't broadcast anymore. Disable permission to do so,
     *  just in case, so we aren't flooding the LAN with broadcasted packets.
     */
    set_socket_broadcast(0);

    return(1);
}

static int parse_ip(const char *str, int *ip)
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

static int parse_interface(char *str, int *ip, short *udpport)
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

static int initialize_sockets(void)
{
#if PLATFORM_WIN32
    int rc;
    WSADATA data;
    printf("initializing WinSock...\n");
    rc = WSAStartup(0x0101, &data);
    if (rc != 0)
    {
        printf("WinSock failed to initialize! [err==%d].\n", rc);
        return(0);
    }
    else
    {
        printf("WinSock initialized.\n");
        printf("  - Caller uses version %d.%d, highest supported is %d.%d.\n",
                data.wVersion >> 8, data.wVersion & 0xFF,
                data.wHighVersion >> 8, data.wHighVersion & 0xFF);
        printf("  - Implementation description: [%s].\n", data.szDescription);
        printf("  - System status: [%s].\n", data.szSystemStatus);
        printf("  - Max sockets: %d.\n", data.iMaxSockets);
        printf("  - Max UDP datagram size: %d.\n", data.iMaxUdpDg);
    }
#endif

    return(1);
}

static void deinitialize_sockets(void)
{
#if PLATFORM_WIN32
    WSACleanup();
#endif
}

static int parse_udp_config(const char *cfgfile, gcomtype *gcom)
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
                    allowed_addresses[gcom->numplayers].host = host;
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

    if (open_udp_socket(ip, udpport))
    {
        gcom->numplayers++;  /* that's you. */
        if (udpmode == udpmode_server)
            return(wait_for_other_players(gcom, ip));
        else if (udpmode == udpmode_client)
            return(connect_to_server(gcom, ip));
        else if (udpmode == udpmode_peer)
            return(connect_to_everyone(gcom, ip, bcast));

        printf("wtf?!");  /* Should be handled by a udpmode above... */
        assert(0);
    }

    return(0);
}


gcomtype *init_network_transport(char **ARGV, int argpos)
{
    gcomtype *retval;

    printf("UDP NETWORK TRANSPORT INITIALIZING...\n");

    ctrlc_pressed = 0;

    if (!initialize_sockets())
        return(NULL);

    retval = (gcomtype *)malloc(sizeof (gcomtype));
    if (retval != NULL)
    {
        int rc;
        char *cfgfile = ARGV[argpos];
        void (*oldsigint)(int);

        memset(retval, '\0', sizeof (gcomtype));
        memset(allowed_addresses, '\0', sizeof (allowed_addresses));
        udpsocket = -1;
        udpport = BUILD_DEFAULT_UDP_PORT;
        udpmode = udpmode_peer;

        oldsigint = signal(SIGINT, siginthandler);
        rc = parse_udp_config(cfgfile, retval);
        signal(SIGINT, oldsigint);

        if (!rc)
        {
            free(retval);
            deinit_network_transport(NULL);
            return(NULL);
        }
        retval->gametype = 3;  /* gametype: 1-serial,2-modem,3-net */
    }

    return(retval);
}


void deinit_network_transport(gcomtype *gcom)
{
    printf("UDP NETWORK TRANSPORT DEINITIALIZING...\n");

    if (gcom != NULL)
    {
        printf("  ...freeing gcom structure...\n");
        free(gcom);
    }

    if (udpsocket != -1)
    {
        printf("  ...closing socket...\n");
        set_socket_blockmode(1);  /* block while socket drains. */
        shutdown(udpsocket, SOCKET_SHUTDOWN_BOTH);
        socketclose(udpsocket);
        udpsocket = -1;
    }

    deinitialize_sockets();

    printf("UDP net deinitialized successfully.\n");
}


void unstable_callcommit(void)
{
    int ip, i, rc;
    short port;

    if (udpsocket == -1)
        return;

    process_udp_send_queue();

    switch (gcom->command)
    {
        case COMMIT_CMD_GET:
            rc = get_udp_packet(&ip, &port, gcom->buffer, sizeof(gcom->buffer));
            if (rc > 0)
            {
                gcom->numbytes = rc;  /* size of new packet. */
                for (i = 1; i <= gcom->numplayers; i++)
                {
                    if ( (allowed_addresses[i].host == ip) &&
                         (allowed_addresses[i].port == port) )
                    {
                        gcom->other = i;
                        return;
                    }
                }
                /* if you manage to hit this, it'll report no packet avail. */
            }

            gcom->numbytes = 0;
            gcom->other = -1;  /* no packet available. */
            break;

        case COMMIT_CMD_SEND:
            if ((gcom->other < 0) || (gcom->other > gcom->numplayers))
            {
                printf("NET TRANSPORT ERROR: send to player out of range\n");
                return;
            }

            ip = allowed_addresses[gcom->other].host;
            if (ip == 0)  /* dropped player? */
                return;

            port = allowed_addresses[gcom->other].port;

            if (!send_udp_packet(ip, port, gcom->buffer, gcom->numbytes))
            {
                printf("NET TRANSPORT ERROR: send failed to %s:%d\n",
                        static_ipstring(ip), (int) port);
            }
            break;

        case COMMIT_CMD_SENDTOALL:
            /* skip player zero, 'cause that's a duplicate of local IP. */
            for (i = 1; i <= gcom->numplayers; i++)
            {
                ip = allowed_addresses[i].host;
                if (ip == 0)  /* dropped player? */
                    continue;

                port = allowed_addresses[i].port;

                if (!send_udp_packet(ip, port, gcom->buffer, gcom->numbytes))
                {
                    printf("NET TRANSPORT ERROR: send failed to %s:%d\n",
                            static_ipstring(ip), (int) port);
                }
            }
            break;

        case COMMIT_CMD_SENDTOALLOTHERS:
            /* skip player zero, 'cause that's a duplicate of local IP. */
            for (i = 1; i <= gcom->numplayers; i++)
            {
                if (i == gcom->myconnectindex)  /* local player. */
                    continue;

                ip = allowed_addresses[i].host;
                if (ip == 0)  /* dropped player? */
                    continue;

                port = allowed_addresses[i].port;

                if (!send_udp_packet(ip, port, gcom->buffer, gcom->numbytes))
                {
                    printf("NET TRANSPORT ERROR: send failed to %s:%d\n",
                            static_ipstring(ip), (int) port);
                }
            }
            break;


        case COMMIT_CMD_SCORE:
			// FIX_00008: minor protocol error after frags (NET TRANSPORT ERROR: Unknown command 5)

			// Leave it on, since the sendscore() function will trigger that case.
			// This will avoid to fall in the default case on each frag and flood
			// the DOS console with NET TRANSPORT ERROR: Unknown command error messages.
			// sendscore() seems to be an obsolete function, and was probably used
			// in an old netcode architecture that 3drealms forgot to remove.
			// We leave sendscore() in the code anyway since it doesnt seem to break 
			// anything. It's not needed to fix what's not broken...
			gcom->other = -1;
            break;

        default:
            printf("NET TRANSPORT ERROR: Unknown command %d\n", gcom->command);
            gcom->other = -1;  /* oh well. */
            break;
    }
}

#else
#error Please define a network transport for your platform.
#endif

/* end of mmulti.c ... */

