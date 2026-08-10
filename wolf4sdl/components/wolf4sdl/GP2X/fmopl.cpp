/*
** based on:
**
** File: fmopl.c - software implementation of FM sound generator
**                                            types OPL and OPL2
**
** Copyright (C) 2002,2003 Jarek Burczynski (bujar at mame dot net)
** Copyright (C) 1999,2000 Tatsuyuki Satoh , MultiArcadeMachineEmulator development
**
** Version 0.70
**
** from the dosbox 0.72 source
*/

#define LOG_MSG printf

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "fmopl.h"
#include "fmopl_940/fmopl_shared.h"

#ifndef PI
#define PI 3.14159265358979323846
#endif

/* output final shift */
#if (OPL_SAMPLE_BITS==16)
	#define FINAL_SH	(0)
	#define MAXOUT		(+32767)
	#define MINOUT		(-32768)
#else
	#define FINAL_SH	(8)
	#define MAXOUT		(+127)
	#define MINOUT		(-128)
#endif


#define FREQ_SH			16  /* 16.16 fixed point (frequency calculations) */
#define EG_SH			16  /* 16.16 fixed point (EG timing)              */
#define LFO_SH			24  /*  8.24 fixed point (LFO calculations)       */
#define TIMER_SH		16  /* 16.16 fixed point (timers calculations)    */

#define FREQ_MASK		((1<<FREQ_SH)-1)

/* envelope output entries */
#define ENV_BITS		10
#define ENV_LEN			(1<<ENV_BITS)
#define ENV_STEP		(128.0/ENV_LEN)

#define MAX_ATT_INDEX	((1<<(ENV_BITS-1))-1) /*511*/
#define MIN_ATT_INDEX	(0)

/* register number to channel number , slot offset */
#define SLOT1 0
#define SLOT2 1

/* Envelope Generator phases */

#define EG_ATT			4
#define EG_DEC			3
#define EG_SUS			2
#define EG_REL			1
#define EG_OFF			0

#define OPL_TYPE_WAVESEL   0x01  /* waveform select		*/
#define OPL_TYPE_ADPCM     0x02  /* DELTA-T ADPCM unit	*/
#define OPL_TYPE_KEYBOARD  0x04  /* keyboard interface	*/
#define OPL_TYPE_IO        0x08  /* I/O port			*/

/* ---------- Generic interface section ---------- */
#define OPL_TYPE_YM3526 (0)
#define OPL_TYPE_YM3812 (OPL_TYPE_WAVESEL)
#define OPL_TYPE_Y8950  (OPL_TYPE_ADPCM|OPL_TYPE_KEYBOARD|OPL_TYPE_IO)

#define WAIT_IF_MSG_BUF_FULL while( *NSubmittedMessages - *NExecutedMessages>= MSG_BUF_SIZE){};

#define WAIT_FOR_SYNC \
 while( (*NSubmittedMessages - *NExecutedMessages) % (int) MSG_BUF_SIZE !=0 ){};

#define ADD_MESSAGE(mtype, i,j,k) \
 { \
 int n = ((*NSubmittedMessages)+1) % ((int) MSG_BUF_SIZE); \
 MessageBuffer[n].type=mtype;  \
 MessageBuffer[n].data1=i;  \
 MessageBuffer[n].data2=j;  \
 MessageBuffer[n].data3=k; \
 (*NSubmittedMessages)++; \
 }
 //if((*NSubmittedMessages) % (int) 500 ==0)
// LOG_MSG("OPL2: %d %d %d\n",*NSubmittedMessages,*NExecutedMessages,*NSubmittedMessages-*NExecutedMessages);





typedef struct fm_opl_lite {
	int               T[2];            /* timer counters				*/
	int               TC[2];
	UINT8             st[2];           /* timer enable					*/

	UINT32            *fn_tab;

	/* external event callback handlers */
	OPL_TIMERHANDLER  TimerHandler;    /* TIMER handler				*/
	int               TimerParam;      /* TIMER parameter				*/
	OPL_IRQHANDLER    IRQHandler;      /* IRQ handler					*/
	int               IRQParam;        /* IRQ parameter				*/
	OPL_UPDATEHANDLER UpdateHandler;   /* stream update handler		*/
	int               UpdateParam;     /* stream update parameter		*/

	UINT8             type;            /* chip type					*/
	UINT8             address;         /* address register				*/
	UINT8             status;          /* status flag					*/
	UINT8             statusmask;      /* status mask					*/
	UINT8             mode;            /* Reg.08 : CSM,notesel,etc.	*/

	int               clock;           /* master clock  (Hz)			*/
	int               rate;            /* sampling rate (Hz)			*/
	double           freqbase;        /* frequency base				*/
	double           TimerBase;       /* Timer base time (==sampling time)*/
} FM_OPLlite;

/* status set and IRQ handling */
inline void OPL_STATUS_SET(FM_OPLlite *OPL,int flag)
{
	/* set status flag */
	OPL->status |= flag;
	if(!(OPL->status & 0x80))
	{
		if(OPL->status & OPL->statusmask)
		{	/* IRQ on */
			OPL->status |= 0x80;
			/* callback user interrupt handler (IRQ is OFF to ON) */
			if(OPL->IRQHandler) (OPL->IRQHandler)(OPL->IRQParam,1);
		}
	}
}

/* status reset and IRQ handling */
inline void OPL_STATUS_RESET(FM_OPLlite *OPL,int flag)
{
	/* reset status flag */
	OPL->status &=~flag;
	if((OPL->status & 0x80))
	{
		if (!(OPL->status & OPL->statusmask) )
		{
			OPL->status &= 0x7f;
			/* callback user interrupt handler (IRQ is ON to OFF) */
			if(OPL->IRQHandler) (OPL->IRQHandler)(OPL->IRQParam,0);
		}
	}
}

/* IRQ mask set */
inline void OPL_STATUSMASK_SET(FM_OPLlite *OPL,int flag)
{
	OPL->statusmask = flag;
	/* IRQ handling check */
	OPL_STATUS_SET(OPL,0);
	OPL_STATUS_RESET(OPL,0);
}

/* generic table initialize */
static int init_tables(void)
{
	signed int i,x;
	signed int n;
	double o,m;


	for (x=0; x<TL_RES_LEN; x++)
	{
		m = (1<<16) / pow(2.0, (x+1) * (ENV_STEP/4.0) / 8.0);
		m = floor(m);

		/* we never reach (1<<16) here due to the (x+1) */
		/* result fits within 16 bits at maximum */

		n = (int)m;		/* 16 bits here */
		n >>= 4;		/* 12 bits here */
		if (n&1)		/* round to nearest */
			n = (n>>1)+1;
		else
			n = n>>1;
						/* 11 bits here (rounded) */
		n <<= 1;		/* 12 bits here (as in real chip) */
		tl_tab[ x*2 + 0 ] = n;
		tl_tab[ x*2 + 1 ] = -tl_tab[ x*2 + 0 ];

		for (i=1; i<12; i++)
		{
			tl_tab[ x*2+0 + i*2*TL_RES_LEN ] =  tl_tab[ x*2+0 ]>>i;
			tl_tab[ x*2+1 + i*2*TL_RES_LEN ] = -tl_tab[ x*2+0 + i*2*TL_RES_LEN ];
		}
	}


	for (i=0; i<SIN_LEN; i++)
	{
		/* non-standard sinus */
		m = sin( ((i*2)+1) * PI / SIN_LEN ); /* checked against the real chip */

		/* we never reach zero here due to ((i*2)+1) */

		if (m>0.0)
			o = 8*log(1.0/m)/log(2.0);	/* convert to 'decibels' */
		else
			o = 8*log(-1.0/m)/log(2.0);	/* convert to 'decibels' */

		o = o / (ENV_STEP/4);

		n = (int)(2.0*o);
		if (n&1)						/* round to nearest */
			n = (n>>1)+1;
		else
			n = n>>1;

		sin_tab[ i ] = n*2 + (m>=0.0? 0: 1 );

	}

	for (i=0; i<SIN_LEN; i++)
	{
		/* waveform 1:  __      __     */
		/*             /  \____/  \____*/
		/* output only first half of the sinus waveform (positive one) */

		if (i & (1<<(SIN_BITS-1)) )
			sin_tab[1*SIN_LEN+i] = TL_TAB_LEN;
		else
			sin_tab[1*SIN_LEN+i] = sin_tab[i];

		/* waveform 2:  __  __  __  __ */
		/*             /  \/  \/  \/  \*/
		/* abs(sin) */

		sin_tab[2*SIN_LEN+i] = sin_tab[i & (SIN_MASK>>1) ];

		/* waveform 3:  _   _   _   _  */
		/*             / |_/ |_/ |_/ |_*/
		/* abs(output only first quarter of the sinus waveform) */

		if (i & (1<<(SIN_BITS-2)) )
			sin_tab[3*SIN_LEN+i] = TL_TAB_LEN;
		else
			sin_tab[3*SIN_LEN+i] = sin_tab[i & (SIN_MASK>>2)];
	}

	return 1;
}

static void OPL_initalize(FM_OPLlite *OPL, FM_OPL *OPLs)
{
	int i;

	/* frequency base */
	OPL->freqbase  = (OPL->rate) ? ((double)OPL->clock / 72.0) / OPL->rate  : 0;

	/* Timer base time */
	OPL->TimerBase = 1.0 / ((double)OPL->clock / 72.0 );

	/* make fnumber -> increment counter table */
	for( i=0 ; i < 1024 ; i++ )
	{
		/* opn phase increment counter = 20bit */
		OPL->fn_tab[i] = (UINT32)( (double)i * 64 * OPL->freqbase * (1<<(FREQ_SH-10)) );
		/* -10 because chip works with 10.10 fixed point, while we use 16.16 */
	}

	/* Amplitude modulation: 27 output levels (triangle waveform); 1 level takes one of: 192, 256 or 448 samples */
	/* One entry from LFO_AM_TABLE lasts for 64 samples */
	OPLs->lfo_am_inc = (UINT32)((1.0 / 64.0 ) * (1<<LFO_SH) * OPL->freqbase);

	/* Vibrato: 8 output levels (triangle waveform); 1 level takes 1024 samples */
	OPLs->lfo_pm_inc = (UINT32)((1.0 / 1024.0) * (1<<LFO_SH) * OPL->freqbase);

	/* Noise generator: a step takes 1 sample */
	OPLs->noise_f = (UINT32)((1.0 / 1.0) * (1<<FREQ_SH) * OPL->freqbase);

	OPLs->eg_timer_add  = (UINT32)((1<<EG_SH)  * OPL->freqbase);
	OPLs->eg_timer_overflow = ( 1 ) * (1<<EG_SH);

}

/* write a value v to register r on OPL chip */
static void OPLWriteReg(FM_OPLlite *OPL, int r, int v)
{
	/* adjust bus to 8 bits */
	r &= 0xff;
	v &= 0xff;

	switch(r&0xe0)
	{
	case 0x00:	/* 00-1f:control */
		switch(r&0x1f)
		{
		case 0x01:	/* waveform select enable */
			break;
		case 0x02:	/* Timer 1 */
			OPL->T[0] = (256-v)*4;
			break;
		case 0x03:	/* Timer 2 */
			OPL->T[1] = (256-v)*16;
			break;
		case 0x04:	/* IRQ clear / mask and Timer enable */
			if(v&0x80)
			{	/* IRQ flag clear */
				OPL_STATUS_RESET(OPL,0x7f);
			}
			else
			{	/* set IRQ mask ,timer enable*/
				OPL->st[0] = v&1;
				OPL->st[1] = (v>>1)&1;

				/* IRQRST,T1MSK,t2MSK,EOSMSK,BRMSK,x,ST2,ST1 */
				OPL_STATUS_RESET(OPL, v & 0x78 );
				OPL_STATUSMASK_SET(OPL, (~v) & 0x78 );

				/* timer 1 */
				if(OPL->st[0])
				{
					OPL->TC[0]=OPL->T[0]*20;
					double interval = (double)OPL->T[0]*OPL->TimerBase;
					if (OPL->TimerHandler) (OPL->TimerHandler)(OPL->TimerParam+0,interval);
				}
				/* timer 2 */
				if(OPL->st[1])
				{
					OPL->TC[1]=OPL->T[1]*20;
					double interval =(double)OPL->T[1]*OPL->TimerBase;
					if (OPL->TimerHandler) (OPL->TimerHandler)(OPL->TimerParam+1,interval);
				}
			}
			break;
		case 0x08:	/* MODE,DELTA-T control 2 : CSM,NOTESEL,x,x,smpl,da/ad,64k,rom */
			OPL->mode = v;
			break;

		default:
			//logerror("FMOPL.C: write to unknown register: %02x\n",r);
			break;
		}
		break;
	case 0x20:	/* am ON, vib ON, ksr, eg_type, mul */
		break;
	case 0x40:
		break;
	case 0x60:
		break;
	case 0x80:
		break;
	case 0xa0:
		break;
	case 0xc0:
		break;
	case 0xe0: /* waveform select */
		break;
	}
}

static void OPLResetChip(FM_OPLlite *OPL)
{
	int c,s;
	int i;

	OPL->mode   = 0;	/* normal mode */
	OPL_STATUS_RESET(OPL,0x7f);

	/* reset with register write */
	OPLWriteReg(OPL,0x01,0); /* wavesel disable */
	OPLWriteReg(OPL,0x02,0); /* Timer1 */
	OPLWriteReg(OPL,0x03,0); /* Timer2 */
	OPLWriteReg(OPL,0x04,0); /* IRQ mask clear */
	for(i = 0xff ; i >= 0x20 ; i-- ) OPLWriteReg(OPL,i,0);
}

/* Create one of virtual YM3812/YM3526/Y8950 */
/* 'clock' is chip clock in Hz  */
/* 'rate'  is sampling rate  */
void OPLCreate(int type, int clock, int rate, FM_OPLlite* OPL, FM_OPL* OPLs)
{
	OPL->type  = type;
	OPL->clock = clock;
	OPL->rate  = rate;

	/* init global tables */
	OPL_initalize(OPL,OPLs);
}

/* Destroy one of virtual YM3812 */
static void OPLDestroy(FM_OPLlite *OPL)
{
	free(OPL);
}

/* Optional handlers */

static void OPLSetTimerHandler(FM_OPLlite *OPL,OPL_TIMERHANDLER TimerHandler,int channelOffset)
{
	OPL->TimerHandler = TimerHandler;
	OPL->TimerParam   = channelOffset;
}
static void OPLSetIRQHandler(FM_OPLlite *OPL,OPL_IRQHANDLER IRQHandler,int param)
{
	OPL->IRQHandler = IRQHandler;
	OPL->IRQParam   = param;
}
static void OPLSetUpdateHandler(FM_OPLlite *OPL,OPL_UPDATEHANDLER UpdateHandler,int param)
{
	OPL->UpdateHandler = UpdateHandler;
	OPL->UpdateParam   = param;
}

static int OPLWrite(FM_OPLlite *OPL,int a,int v)
{
	if( !(a&1) )
	{	/* address port */
		OPL->address = v & 0xff;
	}
	else
	{	/* data port */
		if(OPL->UpdateHandler) OPL->UpdateHandler(OPL->UpdateParam,0);
		OPLWriteReg(OPL,OPL->address,v);
	}
	return OPL->status>>7;
}

static unsigned char OPLRead(FM_OPLlite *OPL,int a)
{
	if( !(a&1) )
	{
		/* status port */

		if (OPL->st[0]) {
			if (OPL->TC[0]) OPL->TC[0]--;
			else {
				OPL->TC[0]=OPL->T[0]*20;
				OPL_STATUS_SET(OPL,0x40);
			}
		}
		if (OPL->st[1]) {
			if (OPL->TC[1]) OPL->TC[1]--;
			else {
				OPL->TC[1]=OPL->T[1]*20;
				OPL_STATUS_SET(OPL,0x40);
			}
		}
		return OPL->status & (OPL->statusmask|0x80);
	}
	return 0xff;
}

static int OPLTimerOver(FM_OPLlite *OPL,int c)
{
	if( c )
	{	/* Timer B */
		OPL_STATUS_SET(OPL,0x20);
	}
	else
	{	/* Timer A */
		OPL_STATUS_SET(OPL,0x40);
		/* CSM mode key,TL controll */
		if( OPL->mode & 0x80 )
		{	/* CSM mode total level latch and auto key on */
			int ch;
			if(OPL->UpdateHandler) OPL->UpdateHandler(OPL->UpdateParam,0);
		}
	}
	return OPL->status>>7;
}


#define MAX_OPL_CHIPS 2

#if (BUILD_YM3812)

static FM_OPLlite *OPLlite_YM3812[MAX_OPL_CHIPS];

extern "C" {
	static int Status940=0;
	static int g_hMemory=0;
	static volatile unsigned short *g_pusRegs;
	static unsigned char *g_pSharedMemory = 0;
	void UpdateThreadEntry(void);
	void Pause940(int n);
	void Reset940(int yes);
	void Startup940();
	void Shutdown940();
	void CleanUp(void);
	void InitSharedMemory();
}

void Pause940(int n)
{
	if(n)
		g_pusRegs[0x0904>>1] &= 0xFFFE;
	else
		g_pusRegs[0x0904>>1] |= 1;
}

void Reset940(int yes)
{
	g_pusRegs[0x3B48>>1] = ((yes&1) << 7) | (0x03);
}

void Startup940()
{
	int nLen, nRead;
	FILE *fp;
	unsigned char ucData[1000];

	Reset940(1);
	Pause940(1);
	g_pusRegs[0x3B40>>1] = 0;
	g_pusRegs[0x3B42>>1] = 0;
	g_pusRegs[0x3B44>>1] = 0xffff;
	g_pusRegs[0x3B46>>1] = 0xffff;

	// load code940.bin
	nLen = 0;
	fp = __fopen("code940.bin", "r");
	if(!fp) {
		LOG_MSG("no 940 core found\n");
		return;
	} else
	{
		LOG_MSG("940 core found\n");
	}
	while(1)
	{
		nRead = __fread(ucData, 1, 1000, fp);
		if(nRead <= 0)
			break;
		memcpy(g_pSharedMemory + nLen, ucData, nRead);
		nLen += nRead;
	}
	__fclose(fp);

	Reset940(0);
	Pause940(0);

	usleep(10000);
}

void Shutdown940()
{
	Reset940(1);
	Pause940(1);
}

void CleanUp(void)
{
	Status940--;
	//if(Status940>0) return;

	//if(g_pSharedMemory)
	//	munmap(g_pSharedMemory, 0xF80000);
	g_pSharedMemory  = 0;
	Shutdown940();
	close(g_hMemory);
	printf("Core shutdown\n");
}

void InitSharedMemory()
{
	if(g_hMemory) return;
	LOG_MSG("Once?\n");
	g_hMemory =__open("/dev/mem", O_RDWR);
	g_pusRegs = (unsigned short *) mmap(0, 0x10000,
			PROT_READ|PROT_WRITE, MAP_SHARED, g_hMemory, 0xc0000000);

	g_pSharedMemory = (unsigned char *) mmap(0, 0xF80000,
			PROT_READ|PROT_WRITE, MAP_SHARED, g_hMemory, 0x3000000);
	memset(g_pSharedMemory,0,0x400000);
}

void UpdateThreadEntry(void)
{
	Status940++;
	if(Status940==1) Startup940();
}

static void InitMemory()
{
	SharedBuff_ptr = (char *) (g_pSharedMemory + BUFF_BASE_ADDRESS);
	SharedData_ptr = (char *) (g_pSharedMemory + DATA_BASE_ADDRESS);
	memset(SharedBuff_ptr,0, END_OFFSET);
	memset(SharedData_ptr,0, END_OFFSET2);
}

int YM3812Init(int num, int clock, int rate)
{
	int i;
	char *ptr;

	if (YM3812NumChips)
		return -1;	/* duplicate init. */

	if(END_OFFSET>OPL2_MSG_SIZE ||
			END_OFFSET2>OPL2_DAT_SIZE) {
		LOG_MSG("OPL2 memory data error\n");
		return -1;
	}

	InitSharedMemory();
	InitMemory();

	LOG_MSG("OPL2 reports\n");
	LOG_MSG("OPL2 mem: %d %d %d %d\n", sizeof(OPL_SLOT),
		sizeof(OPL_CH),sizeof(FM_OPL),OPL_SIZE);


	ptr=(SharedData_ptr + NUMCHIP_OFFSET);
	YM3812NumChips=(int *) ptr;
	*YM3812NumChips = num;

	ptr=(SharedBuff_ptr + NSUB_OFFSET);
	NSubmittedMessages=(int *) ptr;
	*NSubmittedMessages=-1;

	ptr=(SharedBuff_ptr + NEX_OFFSET);
	NExecutedMessages=(int *) ptr;
	*NExecutedMessages=-1;

	ptr=(SharedBuff_ptr + MSG_BUF_OFFSET);
	MessageBuffer=(CoreMessage *) ptr;

	ptr=(SharedBuff_ptr + TL_TAB_OFFSET);
	tl_tab=(signed int *) ptr;
	ptr=(SharedBuff_ptr + SIN_TAB_OFFSET);
	sin_tab=(unsigned int *) ptr;

	init_tables();

	for (i = 0;i < *YM3812NumChips; i++)
	{
		ptr=(SharedBuff_ptr + BUFPOS_OFFSET+ i*sizeof(int));
		Bu__fwrite(Pos[i]=(int *) ptr;
		*Bu__fwrite(Pos[i]=0;

		ptr=(SharedBuff_ptr + READPOS_OFFSET +i*sizeof(int));
		BufReadPos[i]=(int *) ptr;
		*BufReadPos[i]=0;

		ptr=(SharedBuff_ptr + DATA_OFFSET + i * SHARED_BUF_SIZE * sizeof(INT16));
		SharedBuffer[i]=(INT16 *) ptr;

		ptr=(SharedData_ptr + OPL_OFFSET + i*OPL_SIZE);
		OPL_YM3812[i] = (FM_OPL*) ptr;

		ptr = (char *) malloc(sizeof(FM_OPLlite));
		memset(ptr , 0, sizeof(FM_OPLlite));
		OPLlite_YM3812[i] = (FM_OPLlite *) ptr;

		ptr=(SharedBuff_ptr + FNTAB_OFFSET+i*1024*sizeof(UINT32));
		OPLlite_YM3812[i]->fn_tab=(UINT32 *) ptr;

		OPLCreate(OPL_TYPE_YM3812,clock,rate,OPLlite_YM3812[i],OPL_YM3812[i]);
	}

	UpdateThreadEntry();

	ADD_MESSAGE(INIT ,num ,clock ,rate );

	for (i = 0;i < *YM3812NumChips; i++)
	{
		YM3812ResetChip(i);
	}


	return 0;
}

void YM3812Shutdown(void)
{
	int i;

	LOG_MSG("OPL2 ...\n");
	ADD_MESSAGE(SHUTDOWN,0,0,0);
	WAIT_FOR_SYNC;
	LOG_MSG("OPL2 end\n");
	for (i = 0;i < *YM3812NumChips; i++)
	{
		/* emulator shutdown */
		OPLDestroy(OPLlite_YM3812[i]);
		OPL_YM3812[i] = NULL;
		OPLlite_YM3812[i] = NULL;
	}
	*YM3812NumChips = 0;
	CleanUp();
}
void YM3812ResetChip(int which)
{
	ADD_MESSAGE(RESET,which,0,0);
	OPLResetChip(OPLlite_YM3812[which]);
}

int YM3812Write(int which, int a, int v)
{
	ADD_MESSAGE(WRITE,which,0,a);
	ADD_MESSAGE(WRITE,which,1,v);
	OPLWriteReg(OPLlite_YM3812[which], a, v);
	return (OPLlite_YM3812[which]->status>>7);
}

unsigned char YM3812Read(int which, int a)
{
	ADD_MESSAGE(READ,which,a,0);
	/* YM3812 always returns bit2 and bit1 in HIGH state */
	return OPLRead(OPLlite_YM3812[which], a) | 0x06 ;
}

int YM3812TimerOver(int which, int c)
{
	ADD_MESSAGE(TIMEROVER,which,c,0);
	return OPLTimerOver(OPLlite_YM3812[which], c);
}

void YM3812SetTimerHandler(int which, OPL_TIMERHANDLER TimerHandler, int channelOffset)
{
	OPLSetTimerHandler(OPLlite_YM3812[which], TimerHandler, channelOffset);
}
void YM3812SetIRQHandler(int which,OPL_IRQHANDLER IRQHandler,int param)
{
	OPLSetIRQHandler(OPLlite_YM3812[which], IRQHandler, param);
}
void YM3812SetUpdateHandler(int which,OPL_UPDATEHANDLER UpdateHandler,int param)
{
	OPLSetUpdateHandler(OPLlite_YM3812[which], UpdateHandler, param);
}

void YM3812UpdateOne(int which, INT16 *buffer, int length)
{
	int i,ncopy,nfree,nbuff,d,bufpos;
	INT16 lt;
	static int warn=1;

	d=*NSubmittedMessages-*NExecutedMessages;
	if(warn && d>MSG_BUF_SIZE)
	{
		LOG_MSG("OPL2: buffer running full");
		warn=0;
	}
	else
	{
		if(d<MSG_BUF_SIZE) warn=1;
	}

	bufpos=*BufReadPos[which];
	d=*Bu__fwrite(Pos[which]- bufpos;
	nbuff=WRAPPED(d,SHARED_BUF_SIZE);
	ncopy=MIN(length, nbuff);
	nfree=SHARED_BUF_SIZE - bufpos;
	if(ncopy < nfree)
	{
		for(i=0;i<ncopy;i++)
		{
			lt=Amp( SharedBuffer[which][ bufpos+i ] );

			buffer[2*i]=lt;
			buffer[2*i+1]=lt; //for stereo
		}
		(*BufReadPos[which])+=ncopy;
	}
	else
	{
		for(i=0;i<nfree;i++)
		{
			lt=Amp( SharedBuffer[which][ bufpos+i ] );
			buffer[2*i]=lt;
			buffer[2*i+1]=lt;
		}

		for(i=0;i<ncopy-nfree;i++)
		{
			lt=Amp( SharedBuffer[which][i] );
			buffer[2*i+2*nfree]=lt;
			buffer[2*i+2*nfree+1]=lt;
		}
		*BufReadPos[which]=ncopy-nfree;
	}

	if(ncopy < length)
	{
		bufpos=*BufReadPos[which];
		lt=Amp( SharedBuffer[which][WRAPPED(bufpos-1,SHARED_BUF_SIZE)] );
		for(i=ncopy;i<length;i++)
		{
			buffer[2*i]=lt;
			buffer[2*i+1]=lt;
		}
	}

	ADD_MESSAGE(UPDATE, which, length, 0);
}

INT16 Amp( INT16 value )
{
	INT32 temp;
	int offset = 2;

	temp = value;
	temp <<= offset;

	if( temp > MAXOUT ) temp = MAXOUT;
	if( temp < MINOUT ) temp = MINOUT;

	return (INT16)temp;
}

#endif /* BUILD_YM3812 */
