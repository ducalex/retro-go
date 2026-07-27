// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.
// This file has been modified from Ken Silverman's original release

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"

extern long getcrc(char *buffer, short bufleng);
extern void processreservedmessage(short tempbufleng, char *datempbuf);
extern void initcrc(void);
extern int comon(void);
extern void comoff(void);
extern int neton(void);
extern void netoff(void);
extern void startcom(void);
extern int netinitconnection (long newconnectnum, char *newcompaddr);
extern void installbicomhandlers(void);
extern void uninstallbicomhandlers(void);

#define COMBUFSIZ 16384
#define COMCODEBYTES 384
#define COMCODEOFFS 14
#define NETCODEBYTES 384
#define MAXPLAYERS 16
#define ESC1 0x83
#define ESC2 0x8f
#define NETBACKPACKETS 4
#define MAXIPXSIZ 546

#define updatecrc16(crc,dat) crc = (((crc<<8)&65535)^crctable[((((unsigned short)crc)>>8)&65535)^dat])

char syncstate = 0, hangup = 1;
static char multioption = 0, comrateoption = 0;

	//COM & NET variables
short numplayers = 0, myconnectindex = 0;
short connecthead, connectpoint2[MAXPLAYERS];
char syncbuf[MAXIPXSIZ];
long syncbufleng, outbufindex[128], outcnt;
long myconnectnum, otherconnectnum, mypriority;
long crctable[256];

	//COM ONLY variables
long comnum, comvect, comspeed, comtemp, comi, comescape, comreset;
#ifdef PLATFORM_DOS   // !!! this is a real mess. --ryan.
static void interrupt far comhandler(void);
static unsigned short orig_pm_sel, orig_rm_seg, orig_rm_off;
static unsigned long orig_pm_off;
#endif
volatile unsigned char *inbuf, *outbuf, *comerror, *incnt, *comtype;
volatile unsigned char *comresend;
volatile short *inbufplc, *inbufend, *outbufplc, *outbufend, *comport;
#ifdef PLATFORM_DOS   // !!! this is a real mess. --ryan.
static char rmbuffer[COMCODEBYTES] =        //See realcom.asm
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x50,0x53,0x66,0x51,0x52,0x2e,
	0x8b,0x16,0x08,0x00,0x83,0xc2,0x02,0xec,0x24,0x07,
	0x8a,0xe0,0x80,0xfc,0x02,0x74,0x08,0x80,0xfc,0x04,
	0x74,0x62,0xe9,0x89,0x00,0x2e,0x8b,0x16,0x08,0x00,
	0x2e,0x8a,0x0e,0x0a,0x00,0x80,0xe9,0x01,0x78,0x7a,
	0x2e,0x80,0x3e,0x0c,0x00,0x01,0x7c,0x10,0x74,0x04,
	0xb0,0x83,0xeb,0x02,0xb0,0x8f,0xee,0x2e,0xfe,0x0e,
	0x0c,0x00,0xeb,0xe3,0x2e,0x80,0x3e,0x0b,0x00,0x01,
	0x7c,0x12,0x74,0x04,0xb0,0x83,0xeb,0x04,0x2e,0xa0,
	0x0d,0x00,0xee,0x2e,0xfe,0x0e,0x0b,0x00,0xeb,0xc9,
	0x2e,0x8b,0x1e,0x04,0x00,0x2e,0x3b,0x1e,0x06,0x00,
	0x74,0x3c,0x2e,0x8a,0x87,0x80,0x41,0xee,0x43,0x81,
	0xe3,0xff,0x3f,0x2e,0x89,0x1e,0x04,0x00,0xeb,0xab,
	0x2e,0x8b,0x16,0x08,0x00,0xec,0x2e,0x8b,0x1e,0x02,
	0x00,0x2e,0x88,0x87,0x80,0x01,0x43,0x81,0xe3,0xff,
	0x3f,0x2e,0x89,0x1e,0x02,0x00,0x2e,0x80,0x3e,0x0a,
	0x00,0x10,0x75,0x08,0x83,0xc2,0x05,0xec,0xa8,0x01,
	0x75,0xd6,0xf6,0xc4,0x01,0x0f,0x84,0x56,0xff,0xb0,
	0x20,0xe6,0x20,0x5a,0x66,0x59,0x5b,0x58,0xcf,
};
#endif

	//NET ONLY variables
short socket = 0x4949;
char compaddr[MAXPLAYERS][12], mycompaddr[12];
char netincnt[MAXPLAYERS], netoutcnt[MAXPLAYERS];
char getmess[MAXIPXSIZ];
char omessout[MAXPLAYERS][NETBACKPACKETS][MAXIPXSIZ];
short omessleng[MAXPLAYERS][NETBACKPACKETS];
short omessconnectindex[MAXPLAYERS][NETBACKPACKETS];
short omessnum[MAXPLAYERS];
long connectnum[MAXPLAYERS], rmoffset32, rmsegment16, neti;
volatile char *ecbget, *ecbput, *ipxin, *ipxout, *messin, *messout;
volatile char *tempinbuf, *tempoutbuf, *rmnethandler, *netinbuf;
volatile short *netinbufplc, *netinbufend;
static char rmnetbuffer[NETCODEBYTES] =
{
	0xfb,0x2e,0x8a,0x26,0x62,0x00,0x2e,0xa0,0x63,0x00,
	0x83,0xe8,0x1e,0x2e,0x8b,0x1e,0xe2,0x06,0x2e,0x88,
	0x87,0xe4,0x06,0x43,0x81,0xe3,0xff,0x3f,0x2e,0x88,
	0xa7,0xe4,0x06,0x43,0x81,0xe3,0xff,0x3f,0x33,0xf6,
	0x2e,0x8a,0x8c,0xa0,0x00,0x46,0x2e,0x88,0x8f,0xe4,
	0x06,0x43,0x81,0xe3,0xff,0x3f,0x3b,0xf0,0x72,0xec,
	0x2e,0x89,0x1e,0xe2,0x06,0xbb,0x04,0x00,0x8c,0xc8,
	0x8e,0xc0,0xbe,0x00,0x00,0xcd,0x7a,0xcb,
};
static long my7a = 0;

#ifdef PLATFORM_DOS
#pragma aux koutp =\
	"out dx, al",\
	parm [edx][eax]\

#pragma aux kinp =\
	"in al, dx",\
	parm [edx]
#endif

long convalloc32 (long size)
{
#ifdef PLATFORM_DOS	
	union REGS r;

	r.x.eax = 0x0100;           //DPMI allocate DOS memory
	r.x.ebx = ((size+15)>>4);   //Number of paragraphs requested
	int386(0x31,&r,&r);

	if (r.x.cflag != 0) return ((long)0);   //Failed
	return ((long)((r.x.eax&0xffff)<<4));   //Returns full 32-bit offset
#else
	fprintf (stderr, "%s, line %d; convalloc32() called\n", __FILE__,
		__LINE__);
	return 0;
#endif	
}

#ifdef PLATFORM_DOS
#pragma aux fixregistersaftersimulate =\
	"cld",\
	"push ds",\
	"pop es",\

static struct rminfo
{
	long EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX;
	short flags, ES, DS, FS, GS, IP, CS, SP, SS;
} RMI;
#endif

long simulateint(char intnum, long daeax, long daebx, long daecx, long daedx, long daesi, long daedi)
{
#ifdef PLATFORM_DOS	
	union REGS regs;
	struct SREGS sregs;

	memset(&RMI,0,sizeof(RMI));    // Set up real-mode call structure
	memset(&sregs,0,sizeof(sregs));

	RMI.EAX = daeax;
	RMI.EBX = daebx;
	RMI.ECX = daecx;
	RMI.EDX = daedx;
	RMI.ESI = daesi-rmoffset32;
	RMI.EDI = daedi-rmoffset32;
	RMI.DS = rmsegment16;
	RMI.ES = rmsegment16;

	regs.w.ax = 0x0300;            // Use DMPI call 300h to issue the DOS interrupt
	regs.h.bl = intnum;
	regs.h.bh = 0;
	regs.w.cx = 0;
	sregs.es = FP_SEG(&RMI);
	regs.x.edi = FP_OFF(&RMI);
	int386x(0x31,&regs,&regs,&sregs);

	fixregistersaftersimulate();

	return(RMI.EAX);
#else
	fprintf(stderr, "%s line %d; simulateint() called\n",__FILE__,__LINE__);
	return 0;
#endif	
}

void initmultiplayers(char damultioption, char dacomrateoption, char dapriority)
{
	long i;

	multioption = damultioption;
	comrateoption = dacomrateoption;

	connecthead = 0;
	for(i=MAXPLAYERS-1;i>=0;i--)
		connectpoint2[i] = -1, connectnum[i] = 0x7fffffff;

	mypriority = dapriority;

	initcrc();

	if ((multioption >= 1) && (multioption <= 4))
	{
		comnum = multioption;
		switch(dacomrateoption&15)
		{
			case 0: comspeed = 2400; break;
			case 1: comspeed = 4800; break;
			case 2: comspeed = 9600; break;
			case 3: comspeed = 14400; break;
			case 4: comspeed = 19200; break;
			case 5: comspeed = 28800; break;
		}
		comon();
	}
	if (multioption >= 5)
	{
		if ((i = neton()) != 0)
		{
			if (i == -1) printf("IPX driver not found\n");
			if (i == -2) printf("Socket could not be opened\n");
			exit(0);
		}
	}
	numplayers = 1;
}

void uninitmultiplayers()
{
	if (numplayers > 0)
	{
		if ((multioption >= 1) && (multioption <= 4)) comoff();
		if (multioption >= 5) netoff();  //Uninstall before timer
	}
}

int neton(void)
{
	long i, j;

	if ((simulateint(0x2f,(long)0x7a00,0L,0L,0L,0L,0L)&255) != 255) return(-1);
	if (*(long *)(0x7a<<2) == 0)
	{
#ifdef PLATFORM_DOS		
		printf("Faking int 0x7a to call IPX entry at: %4x:%4x\n",RMI.ES,RMI.EDI&65535);
		my7a = convalloc32(16L);
		*(short *)((0x7a<<2)+0) = (my7a&15);
		*(short *)((0x7a<<2)+2) = (my7a>>4);

		*(char *)(my7a+0) = 0x2e;               //call far ptr [L1]
		*(char *)(my7a+1) = 0x9a;
		*(long *)(my7a+2) = 7L;
		*(char *)(my7a+6) = 0xcf;               //iret
		*(short *)(my7a+7) = (RMI.EDI&65535);   //L1: ipxoff
		*(short *)(my7a+9) = RMI.ES;            //    ipxseg
#endif		
	}

		//Special stuff for WATCOM C
	if ((rmoffset32 = convalloc32(1380L+NETCODEBYTES+COMBUFSIZ)) == 0)
		{ printf("Can't allocate memory for IPX\n"); exit; }
	rmsegment16 = (rmoffset32>>4);

	i = rmoffset32;
	ecbget = (char *)i; i += 48;
	ecbput = (char *)i; i += 48;
	ipxin = (char *)i; i += 32;
	ipxout = (char *)i; i += 32;
	messin = (char *)i; i += 560;
	messout = (char *)i; i += 560;
	tempinbuf = (char *)i; i += 16;
	tempoutbuf = (char *)i; i += 80;
	rmnethandler = (char *)i; i += NETCODEBYTES;
	netinbufplc = (short *)i; i += 2;
	netinbufend = (short *)i; i += 2;
	netinbuf = (char *)i; i += COMBUFSIZ;
	memcpy((void *)rmnethandler,(void *)rmnetbuffer,NETCODEBYTES);

	simulateint(0x7a,0L,(long)0x1,0L,(long)socket,0L,0L);                             //Closesocket
	if ((simulateint(0x7a,(long)0xff,0L,0L,(long)socket,0L,0L)&255) != 0) return(-2); //Opensocket

	simulateint(0x7a,0L,9L,0L,0L,(long)tempoutbuf,0L);    //Getinternetworkaddress
	memcpy((void *)&mycompaddr[0],(void *)&tempoutbuf[0],10);
	mycompaddr[10] = (socket&255);
	mycompaddr[11] = (socket>>8);
	myconnectnum = ((long)tempoutbuf[6])+(((long)tempoutbuf[7])<<8)+(((long)(tempoutbuf[8]^tempoutbuf[9]))<<16)+(((long)mypriority)<<24);

	netinitconnection(myconnectnum,mycompaddr);

	ecbget[8] = 1; ecbput[8] = 0;
	*netinbufplc = 0; *netinbufend = 0;

	for(i=MAXPLAYERS-1;i>=0;i--) netincnt[i] = 0, netoutcnt[i] = 0;

	for(i=0;i<MAXPLAYERS;i++)
	{
		omessnum[i] = 0;
		for(j=NETBACKPACKETS-1;j>=0;j--)
		{
			omessleng[i][j] = 0;
			omessconnectindex[i][j] = 0;
		}
	}

		//Netlisten
	for(i=0;i<30;i++) ipxin[i] = 0;
	for(i=0;i<48;i++) ecbget[i] = 0;
	ecbget[4] = (char)(((long)rmnethandler-rmoffset32)&255), ecbget[5] = (char)(((long)rmnethandler-rmoffset32)>>8);
	ecbget[6] = (char)(rmsegment16&255), ecbget[7] = (char)(rmsegment16>>8);
	ecbget[10] = (socket&255), ecbget[11] = (socket>>8);
	ecbget[34] = 2, ecbget[35] = 0;
	ecbget[36] = (char)(((long)ipxin-rmoffset32)&255), ecbget[37] = (char)(((long)ipxin-rmoffset32)>>8);
	ecbget[38] = (char)(rmsegment16&255), ecbget[39] = (char)(rmsegment16>>8);
	ecbget[40] = 30, ecbget[41] = 0;
	ecbget[42] = (char)(((long)messin-rmoffset32)&255), ecbget[43] = (char)(((long)messin-rmoffset32)>>8);
	ecbget[44] = (char)(rmsegment16&255), ecbget[45] = (char)(rmsegment16>>8);
	ecbget[46] = (MAXIPXSIZ&255), ecbget[47] = (MAXIPXSIZ>>8);
	simulateint(0x7a,0L,(long)0x4,0L,0L,(long)ecbget,0L);               //Receivepacket

	return(0);
}

int comon()
{
	long divisor, cnt;
	short *ptr;

	if ((comnum < 1) || (comnum > 4)) return(-1);
	//comvect = 0xb+(comnum&1);
	comvect = ((comrateoption>>4)+0x8+2);
	installbicomhandlers();

	*incnt = 0; outcnt = 0;
	*inbufplc = 0; *inbufend = 0; *outbufplc = 0; *outbufend = 0;

	ptr = (short *)(0x400L+(long)((comnum-1)<<1));
	*comport = *ptr;
	if (*comport == 0)
	{
		switch(comnum)
		{
			case 1: *comport = 0x3f8; break;
			case 2: *comport = 0x2f8; break;
			case 3: *comport = 0x3e8; break;
			case 4: *comport = 0x2e8; break;
		}
		if ((inp((*comport)+5)&0x60) != 0x60) { *comport = 0; return(-1); }
	}
	if ((comspeed <= 0) || (comspeed > 115200)) return(-1);

	  // Baud-Setting,?,?,Parity O/E,Parity Off/On, Stop-1/2,Bits-5/6/7/8
	  // 0x0b is odd parity,1 stop bit, 8 bits

#ifdef PLATFORM_DOS	
	_disable();
#endif	
	koutp((*comport)+3,0x80);                  //enable latch registers
	divisor = 115200 / comspeed;
	koutp((*comport)+0,divisor&255);           //# = 115200 / bps
	koutp((*comport)+1,divisor>>8);
	koutp((*comport)+3,0x03);                  //0x03 = n,8,1

	koutp((*comport)+2,0x87);   //check for a 16550   0=1,64=4,128=8,192=14
	if ((kinp((*comport)+2)&0xf8) == 0xc0)
	{
		*comtype = 16;
	}
	else
	{
		*comtype = 1;
		koutp((*comport)+2,0);
	}
	cnt = *comtype;                        //Clear any junk already in FIFO
	while (((kinp((*comport)+5)&0x1) > 0) && (cnt > 0))
		{ kinp(*comport); cnt--; }

	koutp((*comport)+4,0x0b);             //setup for interrupts (modem control)
	koutp((*comport)+1,0);                //com interrupt disable
	koutp(0x21,kinp(0x21)&(255-(1<<(comvect&7))));          //Unmask vector
	kinp((*comport)+6);
	kinp((*comport)+5);
	kinp((*comport)+0);
	kinp((*comport)+2);
	koutp((*comport)+1,0x03);             //com interrupt enable
	koutp(0x20,0x20);

	comescape = 0; comreset = 0;
	*comerror = 0; *comresend = 0;
#ifdef PLATFORM_DOS	
	_enable();
#endif	

	syncbufleng = 0;

	return(0);
}

void netoff()
{
	if (my7a) *(long *)(0x7a<<2) = 0L;
	simulateint(0x7a,0L,(long)0x1,0L,(long)socket,0L,0L);               //Closesocket
}

void comoff()
{
	long i;

	i = 1048576;
	while ((*outbufplc != *outbufend) && (i >= 0))
	{
		startcom();
		i--;
	}

#ifdef PLATFORM_DOS	
	_disable();
#endif	
	koutp(0x21,kinp(0x21)|(1<<(comvect&7)));          //Mask vector
	if (hangup != 0)
	{
		koutp((*comport)+1,0);
		koutp((*comport)+4,0);
	}
#ifdef PLATFORM_DOS	
	_enable();
#endif	
	uninstallbicomhandlers();
}

void netsend (short otherconnectindex, short messleng)
{
	long i;

	i = 32767;
	while ((ecbput[8] != 0) && (i > 0)) i--;
	for(i=0;i<30;i++) ipxout[i] = 0;
	for(i=0;i<48;i++) ecbput[i] = 0;
	ipxout[5] = 4;
	if (otherconnectindex < 0)
	{
		memcpy((void *)&ipxout[6],(void *)&compaddr[0][0],4);
		ipxout[10] = 0xff, ipxout[11] = 0xff, ipxout[12] = 0xff;
		ipxout[13] = 0xff, ipxout[14] = 0xff, ipxout[15] = 0xff;
		ipxout[16] = (socket&255), ipxout[17] = (socket>>8);
	}
	else
	{
		memcpy((void *)&ipxout[6],(void *)&compaddr[otherconnectindex][0],12);
	}

	ecbput[10] = (socket&255), ecbput[11] = (socket>>8);
	if (otherconnectindex < 0)
	{
		ecbput[28] = 0xff, ecbput[29] = 0xff, ecbput[30] = 0xff;
		ecbput[31] = 0xff, ecbput[32] = 0xff, ecbput[33] = 0xff;
	}
	else
	{
		memcpy((void *)&ecbput[28],(void *)&compaddr[otherconnectindex][4],6);
	}

	ecbput[34] = 2, ecbput[35] = 0;
	ecbput[36] = (char)(((long)ipxout-rmoffset32)&255), ecbput[37] = (char)(((long)ipxout-rmoffset32)>>8);
	ecbput[38] = (char)(rmsegment16&255), ecbput[39] = (char)(rmsegment16>>8);
	ecbput[40] = 30, ecbput[41] = 0;
	ecbput[42] = (char)(((long)messout-rmoffset32)&255), ecbput[43] = (char)(((long)messout-rmoffset32)>>8);
	ecbput[44] = (char)(rmsegment16&255), ecbput[45] = (char)(rmsegment16>>8);
	ecbput[46] = (char)(messleng&255), ecbput[47] = (char)(messleng>>8);
	simulateint(0x7a,0L,(long)0x3,0L,0L,(long)ecbput,0L);               //Sendpacket
}

void comsend(char ch)
{
	if (ch == ESC1)
	{
		outbuf[*outbufend] = ESC1; *outbufend = (((*outbufend)+1)&(COMBUFSIZ-1));
		ch = 128;
	}
	else if (ch == ESC2)
	{
		outbuf[*outbufend] = ESC1; *outbufend = (((*outbufend)+1)&(COMBUFSIZ-1));
		ch = 129;
	}
	outbuf[*outbufend] = ch; *outbufend = (((*outbufend)+1)&(COMBUFSIZ-1));
}

void startcom()
{
	if ((kinp((*comport)+5)&0x40) == 0) return;

	if (*comresend != 0)
	{
		if (*comresend == 2) koutp(*comport,ESC1);
		if (*comresend == 1) koutp(*comport,ESC2);
		*comresend = (*comresend) - 1;
	}
	else if (*comerror != 0)
	{
		if (*comerror == 2) koutp(*comport,ESC1);
		if (*comerror == 1) koutp(*comport,*incnt);
		*comerror = (*comerror) - 1;
	}
	else if (*outbufplc != *outbufend)
	{
		koutp(*comport,(long)outbuf[*outbufplc]);
		*outbufplc = (((*outbufplc)+1)&(COMBUFSIZ-1));
	}
}

void interrupt far comhandler(void)
{
	do
	{
		comtemp = (kinp((*comport)+2)&7);
		if (comtemp == 2)
		{
			for(comi=(*comtype);comi>0;comi--)
			{
				if (*comresend != 0)
				{
					if (*comresend == 2) koutp(*comport,ESC1);
					if (*comresend == 1) koutp(*comport,ESC2);
					*comresend = (*comresend) - 1;
					continue;
				}
				if (*comerror != 0)
				{
					if (*comerror == 2) koutp(*comport,ESC1);
					if (*comerror == 1) koutp(*comport,*incnt);
					*comerror = (*comerror) - 1;
					continue;
				}
				if (*outbufplc != *outbufend)
				{
					koutp(*comport,(long)outbuf[*outbufplc]);
					*outbufplc = (((*outbufplc)+1)&(COMBUFSIZ-1));
					continue;
				}
				break;
			}
		}
		else if (comtemp == 4)
		{
			do
			{
				//comtemp = (rand()&255);
				//if (comtemp == 17)
				//   inbuf[*inbufend] = 17;
				//else
					inbuf[*inbufend] = (char)kinp(*comport);

				//if (comtemp != 11) *inbufend = (((*inbufend)+1)&(COMBUFSIZ-1));
				//if (comtemp == 24)
				//{
				//   inbuf[*inbufend] = 17;
					*inbufend = (((*inbufend)+1)&(COMBUFSIZ-1));
				//}
				//comtemp = 4;

			} while ((*comtype == 16) && ((kinp((*comport)+5)&1) > 0));
		}
	}
	while ((comtemp&1) == 0);
	koutp(0x20,0x20);
}

int netinitconnection (long newconnectnum, char *newcompaddr)
{
	long i, j, k, newindex, templong;
	char tempchar;

		//Check to see if connection number already initialized
	for(i=0;i<MAXPLAYERS;i++)
		if (connectnum[i] == newconnectnum) return(-1);

		//Find blank place to put new connection number
	newindex = 0;
	while (connectnum[newindex] != 0x7fffffff)
	{
		newindex++;
		if (newindex >= MAXPLAYERS) return(-1);  //Out of space! (more than 16 players)
	}

		//Insert connection number on connection number list
	numplayers++;
	connectnum[newindex] = newconnectnum;

		//Getinternetworkaddress
	memcpy((void *)&compaddr[newindex][0],(void *)newcompaddr,10);
	compaddr[newindex][10] = (socket&255);
	compaddr[newindex][11] = (socket>>8);

		//Sort connection numbers
	for(i=1;i<MAXPLAYERS;i++)
		for(j=0;j<i;j++)
			if (connectnum[i] < connectnum[j])
			{
				templong = connectnum[i], connectnum[i] = connectnum[j], connectnum[j] = templong;
				for(k=0;k<12;k++) tempchar = compaddr[i][k], compaddr[i][k] = compaddr[j][k], compaddr[j][k] = tempchar;
			}

		//Rebuild linked list, MAKING SURE that the linked list goes through
		//   the players in the same order on all computers!
	connecthead = 0;
	for(i=0;i<numplayers-1;i++) connectpoint2[i] = i+1;
	connectpoint2[numplayers-1] = -1;

	for(i=0;i<numplayers;i++)
		if (connectnum[i] == myconnectnum) myconnectindex = i;

	return(1);
}

void netuninitconnection(short goneindex)
{
	long i, j, k=0;

	connectnum[goneindex] = 0x7fffffff; numplayers--;

	j = 0;
	for(i=0;i<MAXPLAYERS;i++)
		if (connectnum[i] != 0x7fffffff)
		{
			if (j == 0) connecthead = i; else connectpoint2[k] = i;
			k = i; j++;
		}
	connectpoint2[k] = -1;
}

void sendpacket (short otherconnectindex, unsigned char *bufptr, short messleng)
{
	long i, j, k, l;

	if (multioption <= 0) return;
	if (multioption < 5)
	{
			//Allow initial incnt/outcnt syncing
		if ((bufptr[0] == 253) || (bufptr[0] == 254)) outcnt = 0;

		outbufindex[outcnt] = *outbufend;

		comsend(((messleng&1)<<7)+outcnt);
		for(i=0;i<messleng;i++) comsend(bufptr[i]);
		if ((comrateoption&15) > 0)
		{
			i = getcrc(bufptr,messleng);
			updatecrc16(i,(((messleng&1)<<7)+outcnt));
			comsend(i&255); comsend(i>>8);
		}
		outbuf[*outbufend] = ESC2, *outbufend = (((*outbufend)+1)&(COMBUFSIZ-1));  //Not raw

		startcom();
		outcnt = ((outcnt+1)&127);
	}
	else
	{
		i = 262144;       //Wait for last packet to be sent
		while ((i > 0) && (ecbput[8] != 0)) i--;

		messout[0] = myconnectindex; j = 1;

		if ((unsigned char) bufptr[0] >= 200)
		{
				//Allow initial incnt/outcnt syncing
			for(i=0;i<MAXPLAYERS;i++) netoutcnt[i] = 0;
			messout[j++] = 0xfe;
			for(i=0;i<messleng;i++) messout[j++] = bufptr[i];
			netsend(otherconnectindex,j);
			return;
		}

			//Copy new packet into omess fifo
		if (otherconnectindex < 0)
		{
			for(k=connecthead;k>=0;k=connectpoint2[k])
				if (k != myconnectindex)
				{
					omessconnectindex[k][omessnum[k]] = -1;
					omessleng[k][omessnum[k]] = messleng;
					for(i=0;i<messleng;i++) omessout[k][omessnum[k]][i] = bufptr[i];
				}
		}
		else
		{
			omessconnectindex[otherconnectindex][omessnum[otherconnectindex]] = otherconnectindex;
			omessleng[otherconnectindex][omessnum[otherconnectindex]] = messleng;
			for(i=0;i<messleng;i++) omessout[otherconnectindex][omessnum[otherconnectindex]][i] = bufptr[i];
		}

			//Put last 4 packets into 1 big packet
		for(l=0;l<NETBACKPACKETS;l++)
		{
				//k = omess index
			k = ((omessnum[otherconnectindex]-l)&(NETBACKPACKETS-1));

			if (omessconnectindex[otherconnectindex][k] < 0)
			{
				messout[j++] = 255;
				for(i=connecthead;i>=0;i=connectpoint2[i])
					if (i != myconnectindex)
						messout[j++] = ((netoutcnt[i]-l)&255);
			}
			else
			{
				messout[j++] = otherconnectindex;
				messout[j++] = ((netoutcnt[otherconnectindex]-l)&255);
			}
			messout[j++] = (omessleng[otherconnectindex][k]&255);
			messout[j++] = ((omessleng[otherconnectindex][k]>>8)&255);
			for(i=0;i<omessleng[otherconnectindex][k];i++)
				messout[j++] = omessout[otherconnectindex][k][i];
		}

			//SEND!!!
		netsend(otherconnectindex,j);


			//Increment outcnt and omessnum counters
		if (otherconnectindex < 0)
		{
			for(i=connecthead;i>=0;i=connectpoint2[i])
				if (i != myconnectindex)
				{
					netoutcnt[i]++;
					omessnum[i] = ((omessnum[i]+1)&(NETBACKPACKETS-1));
				}
		}
		else
		{
			netoutcnt[otherconnectindex]++;
			omessnum[otherconnectindex] = ((omessnum[otherconnectindex]+1)&(NETBACKPACKETS-1));
		}
	}
}

short getpacket (short *otherconnectindex, char *bufptr)
{
	char toindex, bad, totbad;
	short i, j=0, k, messleng, submessleng;

	if (multioption <= 0) return(0);
	if (multioption < 5)
	{
		*otherconnectindex = (myconnectindex^1);

		bad = 0;
		while (*inbufplc != *inbufend)
		{
			i = (short)inbuf[*inbufplc], *inbufplc = (((*inbufplc)+1)&(COMBUFSIZ-1));

			if (i != ESC2)
			{
				if (i == ESC1) { comescape++; continue; }
				if (comescape != 0)
				{
					comescape--;
					if ((i < 128) && (*comresend == 0) && (((i-outcnt)&127) > 4))
					{
						*comresend = 2;
						*outbufplc = outbufindex[i];
						startcom();
						continue;
					}
					if (syncbufleng < MAXIPXSIZ)
					{
						if (i == 128) { syncbuf[syncbufleng++] = ESC1; continue; }
						if (i == 129) { syncbuf[syncbufleng++] = ESC2; continue; }
					}
				}
				if (syncbufleng < MAXIPXSIZ) syncbuf[syncbufleng++] = i;
				continue;
			}

			if (comescape != 0)
			{
				comescape = 0; comreset = 0; *comerror = 0;
				syncbufleng = 0;
				continue;
			}

			messleng = syncbufleng-3+(((comrateoption&15)==0)<<1);
			if ((syncbuf[0]&127) != *incnt)
			{
				bad |= 1;      //Packetcnt error
				if ((*incnt == 1) && (syncbuf[1] == 254))  //Prevent 2 Masters!
					myconnectindex = (myconnectnum<otherconnectnum);
			}
			if (((syncbuf[0]&128)>>7) != (messleng&1)) bad |= 2;   //messleng error
			for(i=0;i<messleng;i++) bufptr[i] = syncbuf[i+1];
			if ((comrateoption&15) > 0)
			{
				i = getcrc(bufptr,messleng);
				updatecrc16(i,syncbuf[0]);
				if (((unsigned short)i) != ((long)syncbuf[syncbufleng-2])+((long)syncbuf[syncbufleng-1]<<8))
					bad |= 2;   //CRC error
			}

			syncbufleng = 0;
			if (bad != 0)
			{
					//Don't send reset again if outbufplc is not before incnt!
				if ((bad == 1) && ((((syncbuf[0]&127)-(*incnt))&127) >= 124))
				{
					bad = 0;
					continue;
				}

				bad = 0;
				if (comreset != 0) comreset--;
				if (((*comerror)|comreset) == 0)
				{
					*comerror = 2; comreset = 2;
					startcom();
				}
				continue;
			}

			*incnt = (((*incnt)+1)&127);
			if ((messleng > 0) && (bufptr[0] >= 200))  //200-255 are messages for engine's use only
				processreservedmessage(messleng,bufptr);

			comescape = 0; comreset = 0; *comerror = 0;
			return(messleng);
		}
		return(0);
	}
	else
	{
		if (*netinbufplc == *netinbufend) return(0);

		messleng = (short)netinbuf[*netinbufplc] + (((short)netinbuf[((*netinbufplc)+1)&(COMBUFSIZ-1)])<<8);
		for(i=0;i<messleng;i++)
			getmess[i] = netinbuf[((*netinbufplc)+i+2)&(COMBUFSIZ-1)];

		k = 0; *otherconnectindex = getmess[k++];

		for(totbad=0;totbad<NETBACKPACKETS;totbad++)   //Number of sub-packets per packet
		{
			toindex = getmess[k++];
			if (toindex == 0xfe)
			{
				netincnt[*otherconnectindex] = 0;  // (>= 200) && ( <= 254)
				submessleng = messleng-2;          // Submessleng not necessary
			}
			else
			{
				if (toindex == 0xff)
				{
					for(i=connecthead;i>=0;i=connectpoint2[i])
						if (i != *otherconnectindex)
						{
							if (i == myconnectindex) j = getmess[k];
							k++;
						}
				}
				else
					j = getmess[k++];

				if (j != netincnt[*otherconnectindex])
				{
					submessleng = (short)getmess[k]+(((short)getmess[k+1])<<8);
					k += submessleng+2;
					continue;
				}
				netincnt[*otherconnectindex]++;

				submessleng = (short)getmess[k]+(((short)getmess[k+1])<<8); k += 2;
			}
			for(i=0;i<submessleng;i++) bufptr[i] = getmess[k++];

			if (totbad == 0)
			{
					//Increment inbufplc only if first sub-message is read
				*netinbufplc = (((*netinbufplc)+messleng+2)&(COMBUFSIZ-1));
				if ((submessleng > 0) && (bufptr[0] >= 200)) //200-255 are messages for engine's use only
				{
					if (bufptr[0] >= 253)
					{
						processreservedmessage(submessleng,bufptr);
						if ((bufptr[0] == 253) || (bufptr[0] == 254)) return(0);
					}
					return(submessleng);
				}
			}
			if (*otherconnectindex == myconnectindex) return(0);
			return(submessleng);   //Got good packet
		}

		syncstate++;   //DON'T WANT TO GET HERE!!!
			//Increment inbufplc to make it not continuously screw up!
		*netinbufplc = (((*netinbufplc)+messleng+2)&(COMBUFSIZ-1));
	}
	return(0);
}

void initcrc(void)
{
	long i, j, k, a;

	for(j=0;j<256;j++)      //Calculate CRC table
	{
		k = (j<<8); a = 0;
		for(i=7;i>=0;i--)
		{
			if (((k^a)&0x8000) > 0)
				a = ((a<<1)&65535) ^ 0x1021;   //0x1021 = genpoly
			else
				a = ((a<<1)&65535);
			k = ((k<<1)&65535);
		}
		crctable[j] = (a&65535);
	}
}

long getcrc(char *buffer, short bufleng)
{
	long i, j;

	j = 0;
	for(i=bufleng-1;i>=0;i--) updatecrc16(j,buffer[i]);
	return(j&65535);
}

void installbicomhandlers(void)
{
#ifdef PLATFORM_DOS	
	union REGS r;
	struct SREGS sr;
	long lowp;
	void far *fh;

		//Get old protected mode handler
	r.x.eax = 0x3500+comvect;   /* DOS get vector (INT 0Ch) */
	sr.ds = sr.es = 0;
	int386x(0x21,&r,&r,&sr);
	orig_pm_sel = (unsigned short)sr.es;
	orig_pm_off = r.x.ebx;

		//Get old real mode handler
	r.x.eax = 0x0200;   /* DPMI get real mode vector */
	r.h.bl = comvect;
	int386(0x31,&r,&r);
	orig_rm_seg = (unsigned short)r.x.ecx;
	orig_rm_off = (unsigned short)r.x.edx;

		//Allocate memory in low memory to store real mode handler
	if ((lowp = convalloc32(COMCODEBYTES+(COMBUFSIZ<<1))) == 0)
		{ printf("Can't allocate conventional memory.\n"); exit; }

	inbufplc = (short *)(lowp+0);
	inbufend = (short *)(lowp+2);
	outbufplc = (short *)(lowp+4);
	outbufend = (short *)(lowp+6);
	comport = (short *)(lowp+8);
	comtype = (char *)(lowp+10);
	comerror = (char *)(lowp+11);
	comresend = (char *)(lowp+12);
	incnt = (char *)(lowp+13);
	inbuf = (char *)(lowp+COMCODEBYTES);
	outbuf = (char *)(lowp+COMCODEBYTES+COMBUFSIZ);

	memcpy((void *)lowp,(void *)rmbuffer,COMCODEBYTES);

		//Set new protected mode handler
	r.x.eax = 0x2500+comvect;   /* DOS set vector (INT 0Ch) */
	fh = (void far *)comhandler;
	r.x.edx = FP_OFF(fh);
	sr.ds = FP_SEG(fh);      //DS:EDX == &handler
	sr.es = 0;
	int386x(0x21,&r,&r,&sr);

		//Set new real mode handler (must be after setting protected mode)
	r.x.eax = 0x0201;
	r.h.bl = comvect;              //CX:DX == real mode &handler
	r.x.ecx = ((lowp>>4)&0xffff);  //D32realseg
	r.x.edx = COMCODEOFFS;         //D32realoff
	int386(0x31,&r,&r);
#else
	fprintf (stderr,"%s, line %d; installbicomhandlers() called\n",
		__FILE__, __LINE__);
#endif	
}

void uninstallbicomhandlers(void)
{
#ifdef PLATFORM_DOS	
	union REGS r;
	struct SREGS sr;

		//restore old protected mode handler
	r.x.eax = 0x2500+comvect;   /* DOS set vector (INT 0Ch) */
	r.x.edx = orig_pm_off;
	sr.ds = orig_pm_sel;    /* DS:EDX == &handler */
	sr.es = 0;
	int386x(0x21,&r,&r,&sr);

		//restore old real mode handler
	r.x.eax = 0x0201;   /* DPMI set real mode vector */
	r.h.bl = comvect;
	r.x.ecx = (unsigned long)orig_rm_seg;     //CX:DX == real mode &handler
	r.x.edx = (unsigned long)orig_rm_off;
	int386(0x31,&r,&r);
#else
	fprintf (stderr, "%s line %d; uninstallbicomhandlers() called\n",
		__FILE__, __LINE__);	
#endif	
}

void processreservedmessage(short tempbufleng, char *datempbuf)
{
	long i, j, k, daotherconnectnum, templong;

	switch(datempbuf[0])
	{
		//[253] (login, if myconnectnum's lowest, then respond with packet type 254)
		case 253:
			if (multioption < 5)
			{
				otherconnectnum = ((long)datempbuf[1])+(((long)datempbuf[2])<<8)+(((long)datempbuf[3])<<16)+(((long)datempbuf[4])<<24);

				datempbuf[0] = 254;
				sendpacket(-1,datempbuf,1);

				myconnectindex = 0;
				connecthead = 0; connectpoint2[0] = 1; connectpoint2[1] = -1;
				numplayers = 2;
			}
			else if (multioption >= 5)
			{
				daotherconnectnum = ((long)datempbuf[1])+((long)(datempbuf[2]<<8))+((long)(datempbuf[3]<<16))+((long)(datempbuf[4]<<24));
				if (daotherconnectnum != myconnectnum)
				{
					netinitconnection(daotherconnectnum,&datempbuf[5]);

					if ((myconnectindex == connecthead) || ((connectnum[connecthead] == daotherconnectnum) && (myconnectindex == connectpoint2[connecthead])))
					{
						datempbuf[0] = 254;
						j = 1;
						for(i=0;i<MAXPLAYERS;i++)
							if ((connectnum[i] != 0x7fffffff) && (connectnum[i] != daotherconnectnum))
							{
								datempbuf[j++] = (connectnum[i]&255);
								datempbuf[j++] = ((connectnum[i]>>8)&255);
								datempbuf[j++] = ((connectnum[i]>>16)&255);
								datempbuf[j++] = ((connectnum[i]>>24)&255);

								for(k=0;k<10;k++)
									datempbuf[j++] = compaddr[i][k];
							}

							//While this doesn't have to be a broadcast, sending
							//this info again makes good error correction
						sendpacket(-1,datempbuf,j);

						for(i=0;i<MAXPLAYERS;i++)
							if (connectnum[i] == daotherconnectnum)
							{
								sendpacket((short)i,datempbuf,j);
								break;
							}
					}
				}
			}
			break;
		case 254:  //[254][connectnum][connectnum]...(Packet type 253 response)
			if (multioption < 5)
			{
				myconnectindex = 1;
				connecthead = 0; connectpoint2[0] = 1; connectpoint2[1] = -1;
				numplayers = 2;
			}
			else if (multioption >= 5)
			{
				j = 1;
				while (j < tempbufleng)
				{
					templong = ((long)datempbuf[j])+((long)(datempbuf[j+1]<<8))+((long)(datempbuf[j+2]<<16))+((long)(datempbuf[j+3]<<24));
					netinitconnection(templong,&datempbuf[j+4]);
					j += 14;
				}
			}
			break;
		case 255:
			if (multioption >= 5)
				netuninitconnection(datempbuf[1]);
			break;
	}
}

void sendlogon(void)
{
	long i;
	char tempbuf[16];

	if (multioption <= 0)
		return;

	tempbuf[0] = 253;
	if (multioption < 5)
	{
		tempbuf[1] = kinp(0x40);
		tempbuf[2] = kinp(0x40);
		tempbuf[3] = kinp(0x40);
		tempbuf[4] = mypriority;
		myconnectnum = ((long)tempbuf[1])+(((long)tempbuf[2])<<8)+(((long)tempbuf[3])<<16)+(((long)mypriority)<<24);
		sendpacket(-1,tempbuf,5);
	}
	else
	{
		tempbuf[1] = (myconnectnum&255);
		tempbuf[2] = ((myconnectnum>>8)&255);
		tempbuf[3] = ((myconnectnum>>16)&255);
		tempbuf[4] = ((myconnectnum>>24)&255);
		for(i=0;i<10;i++)
			tempbuf[i+5] = mycompaddr[i];

		sendpacket(-1,tempbuf,15);
	}
}

void sendlogoff(void)
{
	char tempbuf[16];
	long i;

	if ((numplayers <= 1) || (multioption <= 0)) return;

	tempbuf[0] = 255;
	if (multioption < 5)
	{
		sendpacket(-1,tempbuf,1);
	}
	else
	{
		tempbuf[1] = myconnectindex;
		for(i=connecthead;i>=0;i=connectpoint2[i])
			if (i != myconnectindex)
				sendpacket(i,tempbuf,2);
	}
}

int getoutputcirclesize(void)
{
	if ((multioption >= 1) && (multioption <= 4))
	{
		startcom();
		return(((*outbufend)-(*outbufplc)+COMBUFSIZ)&(COMBUFSIZ-1));
	}
	return(0);
}

int setsocket(short newsocket)
{
	long i;

	if (multioption < 5)
	{
		socket = newsocket;
		return(0);
	}

	simulateint(0x7a,0L,(long)0x1,0L,(long)socket,0L,0L);                             //Closesocket

	socket = newsocket;

	simulateint(0x7a,0L,(long)0x1,0L,(long)socket,0L,0L);                             //Closesocket
	if ((simulateint(0x7a,(long)0xff,0L,0L,(long)socket,0L,0L)&255) != 0) return(-2); //Opensocket
	mycompaddr[10] = (socket&255);
	mycompaddr[11] = (socket>>8);
	ecbget[10] = (socket&255);
	ecbget[11] = (socket>>8);
	for(i=0;i<MAXPLAYERS;i++)
	{
		compaddr[i][10] = (socket&255);
		compaddr[i][11] = (socket>>8);
	}
	return(0);
}
