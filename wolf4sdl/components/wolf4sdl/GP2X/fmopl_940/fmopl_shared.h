#include "memory_layout.h"

#define WRAPPED(x,y)   ((x)>=0?(x):(x)+(y))
#define MIN(x,y) ((x)<(y)?(x):(y))

typedef struct{
	UINT32  ar;             /* attack rate: AR<<2			*/
	UINT32  dr;             /* decay rate:  DR<<2			*/
	UINT32  rr;             /* release rate:RR<<2			*/
	UINT8   KSR;            /* key scale rate				*/
	UINT8   ksl;            /* keyscale level				*/
	UINT8   ksr;            /* key scale rate: kcode>>KSR	*/
	UINT8   mul;            /* multiple: mul_tab[ML]		*/

	/* Phase Generator */
	UINT32  Cnt;            /* frequency counter			*/
	UINT32  Incr;           /* frequency counter step		*/
	UINT8   FB;             /* feedback shift value			*/
	INT32   *connect1;      /* slot1 output pointer			*/
	INT32   op1_out[2];     /* slot1 output for feedback	*/
	UINT8   CON;            /* connection (algorithm) type	*/

	/* Envelope Generator */
	UINT8   eg_type;        /* percussive/non-percussive mode */
	UINT8   state;          /* phase type					*/
	UINT32  TL;             /* total level: TL << 2			*/
	INT32   TLL;            /* adjusted now TL				*/
	INT32   volume;         /* envelope counter				*/
	UINT32  sl;             /* sustain level: sl_tab[SL]	*/
	UINT8   eg_sh_ar;       /* (attack state)				*/
	UINT8   eg_sel_ar;      /* (attack state)				*/
	UINT8   eg_sh_dr;       /* (decay state)				*/
	UINT8   eg_sel_dr;      /* (decay state)				*/
	UINT8   eg_sh_rr;       /* (release state)				*/
	UINT8   eg_sel_rr;      /* (release state)				*/
	UINT32  key;            /* 0 = KEY OFF, >0 = KEY ON		*/

	/* LFO */
	UINT32  AMmask;         /* LFO Amplitude Modulation enable mask */
	UINT8   vib;            /* LFO Phase Modulation enable flag (active high)*/

	/* waveform select */
	unsigned int wavetable;
} OPL_SLOT;

typedef struct{
	OPL_SLOT SLOT[2];
	/* phase generator state */
	UINT32  block_fnum;     /* block+fnum					*/
	UINT32  fc;             /* Freq. Increment base			*/
	UINT32  ksl_base;       /* KeyScaleLevel Base step		*/
	UINT8   kcode;          /* key code (for key scaling)	*/
} OPL_CH;

/* OPL state */
typedef struct fm_opl_f {
	/* FM channel slots */
	OPL_CH  P_CH[9];               /* OPL/OPL2 chips have 9 channels*/

	UINT32  eg_cnt;                /* global envelope generator counter	*/
	UINT32  eg_timer;              /* global envelope generator counter works at frequency = chipclock/72 */
	UINT32  eg_timer_add;          /* step of eg_timer						*/
	UINT32  eg_timer_overflow;     /* envelope generator timer overlfows every 1 sample (on real chip) */

	UINT8   rhythm;                /* Rhythm mode					*/

	UINT32  *fn_tab;               /* fnumber->increment counter	*/

	/* LFO */
	UINT8   lfo_am_depth;
	UINT8   lfo_pm_depth_range;
	UINT32  lfo_am_cnt;
	UINT32  lfo_am_inc;
	UINT32  lfo_pm_cnt;
	UINT32  lfo_pm_inc;

	UINT32  noise_rng;             /* 23 bit noise shift register	*/
	UINT32  noise_p;               /* current noise 'phase'		*/
	UINT32  noise_f;               /* current noise period			*/

	UINT8   wavesel;               /* waveform select enable flag	*/

	int     T[2];                  /* timer counters				*/
	int     TC[2];
	UINT8   st[2];                 /* timer enable					*/

	/* external event callback handlers */

	UINT8   type;                  /* chip type					*/
	UINT8   address;               /* address register				*/
	UINT8   status;                /* status flag					*/
	UINT8   statusmask;            /* status mask					*/
	UINT8   mode;                  /* Reg.08 : CSM,notesel,etc.	*/

	int     clock;                 /* master clock  (Hz)			*/
	int     rate;                  /* sampling rate (Hz)			*/
} FM_OPL;

#define OPL_SIZE 2048

/*	TL_TAB_LEN is calculated as:
*	12 - sinus amplitude bits     (Y axis)
*	2  - sinus sign bit           (Y axis)
*	TL_RES_LEN - sinus resolution (X axis)
*/

#define TL_RES_LEN	(256)	/* 8 bits addressing (real chip) */
#define TL_TAB_LEN 	(12*2*TL_RES_LEN)
static signed int *tl_tab;

#define SIN_BITS		10
#define SIN_LEN			(1<<SIN_BITS)
#define SIN_MASK		(SIN_LEN-1)

/* sin waveform table in 'decibel' scale */
/* four waveforms on OPL2 type chips */
/* sinwave entries */
static unsigned int *sin_tab;

enum CMESSAGE {
	INIT,
	READ,
	WRITE,
	UPDATE,
	TIMEROVER,
	RESET,
	SHUTDOWN
};

typedef struct {
	int type;
//	int no;
	int data1;
	int data2;
	int data3;
} CoreMessage;

#define MAX_OPL_CHIPS 2

#define MSG_BUF_SIZE 4*1024
volatile static CoreMessage* MessageBuffer;
volatile static int* NSubmittedMessages;
volatile static int* NExecutedMessages;

#define SHARED_BUF_SIZE 256
volatile static INT16* SharedBuffer[MAX_OPL_CHIPS];
volatile static int* BufWritePos[MAX_OPL_CHIPS];
volatile static int* BufReadPos[MAX_OPL_CHIPS];

#if (BUILD_YM3812)

static FM_OPL *OPL_YM3812[MAX_OPL_CHIPS];       /* array of pointers to the YM3812's */
static int *YM3812NumChips=0;                  /* number of chips */

#endif

#define BUFF_BASE_ADDRESS  OPL2_MSG_BASE
#define NSUB_OFFSET        0
#define NEX_OFFSET         (NSUB_OFFSET+sizeof(int))
#define MSG_BUF_OFFSET     (NEX_OFFSET+sizeof(int))
#define BUFPOS_OFFSET      (MSG_BUF_OFFSET+MSG_BUF_SIZE*sizeof(CoreMessage))
#define READPOS_OFFSET     (BUFPOS_OFFSET+MAX_OPL_CHIPS*sizeof(int))
#define DATA_OFFSET        (READPOS_OFFSET+MAX_OPL_CHIPS*sizeof(int))
#define TL_TAB_OFFSET      (DATA_OFFSET+MAX_OPL_CHIPS*SHARED_BUF_SIZE*sizeof(INT16))
#define SIN_TAB_OFFSET     (TL_TAB_OFFSET+TL_TAB_LEN*sizeof(signed int))
#define FNTAB_OFFSET       (SIN_TAB_OFFSET+4*SIN_LEN*sizeof(unsigned int))
#define END_OFFSET         (FNTAB_OFFSET+1024*MAX_OPL_CHIPS*sizeof(UINT32))

#define DATA_BASE_ADDRESS  OPL2_DAT_BASE
#define NUMCHIP_OFFSET     0
#define OPL_OFFSET         (NUMCHIP_OFFSET+sizeof(int))
#define END_OFFSET2        (OPL_OFFSET+OPL_SIZE*MAX_OPL_CHIPS)

static char* SharedData_ptr;
static char* SharedBuff_ptr;
