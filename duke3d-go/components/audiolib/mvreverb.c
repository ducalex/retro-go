#include "multivoc.h"
#include "_multivc.h"

extern double *MV_FooBuffer;
extern int MV_BufferSize;
extern int MV_SampleSize;
extern int MV_MaxVolume;
extern int MV_ReverbDelay;
extern int MV_MixRate;
extern int MV_Channels;

static double * reverbBuffer = 0;
static int delay = 0, CurrAddr;

static	int FB_SRC_A, FB_SRC_B, IIR_DEST_A0, IIR_DEST_A1, ACC_SRC_A0, ACC_SRC_A1, ACC_SRC_B0,
		ACC_SRC_B1, IIR_SRC_A0, IIR_SRC_A1, IIR_DEST_B0, IIR_DEST_B1, ACC_SRC_C0,
		ACC_SRC_C1, ACC_SRC_D0, ACC_SRC_D1, IIR_SRC_B1, IIR_SRC_B0, MIX_DEST_A0,
		MIX_DEST_A1, MIX_DEST_B0, MIX_DEST_B1;
	
static	double IIR_ALPHA, ACC_COEF_A, ACC_COEF_B, ACC_COEF_C, ACC_COEF_D, IIR_COEF, FB_ALPHA, FB_X,
		IN_COEF_L, IN_COEF_R;

static  double iRVBLeft, iRVBRight;

static int cnv_offset(int src)
{
	int64_t temp = ((int64_t)src * (int64_t)MV_MixRate) / 22050;
	return (int)temp;
}

// static char err[256];

// extern __stdcall OutputDebugStringA(char *);

static void check_buffer()
{
	int new_delay = cnv_offset(MV_ReverbDelay);

	if (!delay || new_delay != delay)
	{
//		sprintf(err + strlen(err), ", new_delay: %d, MV_ReverbDelay: %d", new_delay, MV_ReverbDelay);
		FB_SRC_A = cnv_offset(0xE3);
		FB_SRC_B = cnv_offset(0xA9);
		IIR_DEST_A0 = cnv_offset(0xDFB);
		IIR_DEST_A1 = cnv_offset(0xB58);
		ACC_SRC_A0 = cnv_offset(0xD09);
		ACC_SRC_A1 = cnv_offset(0xA3C);
		ACC_SRC_B0 = cnv_offset(0xBD9);
		ACC_SRC_B1 = cnv_offset(0x973);
		IIR_SRC_A0 = cnv_offset(0xB59);
		IIR_SRC_A1 = cnv_offset(0x8DA);
		IIR_DEST_B0 = cnv_offset(0x8D9);
		IIR_DEST_B1 = cnv_offset(0x5E9);
		ACC_SRC_C0 = cnv_offset(0x7EC);
		ACC_SRC_C1 = cnv_offset(0x4B0);
		ACC_SRC_D0 = cnv_offset(0x6EF);
		ACC_SRC_D1 = cnv_offset(0x3D2);
		IIR_SRC_B1 = cnv_offset(0x5EA);
		IIR_SRC_B0 = cnv_offset(0x31D);
		MIX_DEST_A0 = cnv_offset(0x31C);
		MIX_DEST_A1 = cnv_offset(0x238);
		MIX_DEST_B0 = cnv_offset(0x154);
		MIX_DEST_B1 = cnv_offset(0xAA);
		IIR_ALPHA = 0.8701171875;
		ACC_COEF_A = 0.622314453125;
		ACC_COEF_B = -0.5244140625;
		ACC_COEF_C = 0.53955078125;
		ACC_COEF_D = -0.50830078125;
		IIR_COEF = -0.69921875;
		FB_ALPHA = 0.67578125;
		FB_X = 0.646484375;
		IN_COEF_L = -2.;
		IN_COEF_R = -2.;
		if (reverbBuffer) reverbBuffer = (double*) realloc(reverbBuffer, new_delay * sizeof(double));
		else reverbBuffer = (double*) malloc(new_delay * sizeof(double));
		memset(reverbBuffer, 0, new_delay * sizeof(double));
		delay = new_delay;
		CurrAddr = 0;
	}

}

double g_buffer(int iOff, double *ptr)                          // get_buffer content helper: takes care about wraps
{
	int correctDelay = delay;
	if(!correctDelay)
	{
		printf("Error! Reverb race on g_buffer\n");
		correctDelay = cnv_offset(14320);
	}

	iOff=(iOff*4)+CurrAddr;
	while(iOff>correctDelay-1)	
	{
		iOff=iOff-correctDelay;
	}
	while(iOff<0)
	{
		iOff=correctDelay-(0-iOff);
	}
	return (double)*(ptr+iOff);
}

void s_buffer(int iOff,double iVal, double *ptr)                // set_buffer content helper: takes care about wraps and clipping
{
	int correctDelay = delay;
	if(!correctDelay)
	{
		printf("Error! Reverb race on s_buffer\n");
		correctDelay = cnv_offset(14320);
	}

	iOff=(iOff*4)+CurrAddr;
	while(iOff>correctDelay-1)
	{
		iOff=iOff-correctDelay;
	}
	while(iOff<0)
	{
		iOff=correctDelay-(0-iOff);
	}
	*(ptr+iOff)=iVal;
}

void s_buffer1(int iOff,double iVal, double *ptr)                // set_buffer (+1 sample) content helper: takes care about wraps and clipping
{
	int correctDelay = delay;
	if(!correctDelay)
	{
		printf("Error! Reverb race on s_buffer1\n");
		correctDelay = cnv_offset(14320);
	}

	iOff=(iOff*4)+CurrAddr+1;
	while(iOff>correctDelay-1)	
	{
		iOff=iOff-correctDelay;
	}
	while(iOff<0)			
	{
		iOff=correctDelay-(0-iOff);
	}
	*(ptr+iOff)=iVal;
}

double MixREVERBLeft(double INPUT_SAMPLE_L, double INPUT_SAMPLE_R, double *ptr)
{
	double ACC0,ACC1,FB_A0,FB_A1,FB_B0,FB_B1;
	
	const double IIR_INPUT_A0 = (g_buffer(IIR_SRC_A0, ptr) * IIR_COEF) + (INPUT_SAMPLE_L * IN_COEF_L);
	const double IIR_INPUT_A1 = (g_buffer(IIR_SRC_A1, ptr) * IIR_COEF) + (INPUT_SAMPLE_R * IN_COEF_R);
	const double IIR_INPUT_B0 = (g_buffer(IIR_SRC_B0, ptr) * IIR_COEF) + (INPUT_SAMPLE_L * IN_COEF_L);
	const double IIR_INPUT_B1 = (g_buffer(IIR_SRC_B1, ptr) * IIR_COEF) + (INPUT_SAMPLE_R * IN_COEF_R);
	
	const double IIR_A0 = (IIR_INPUT_A0 * IIR_ALPHA) + (g_buffer(IIR_DEST_A0, ptr) * (1.f - IIR_ALPHA));
	const double IIR_A1 = (IIR_INPUT_A1 * IIR_ALPHA) + (g_buffer(IIR_DEST_A1, ptr) * (1.f - IIR_ALPHA));
	const double IIR_B0 = (IIR_INPUT_B0 * IIR_ALPHA) + (g_buffer(IIR_DEST_B0, ptr) * (1.f - IIR_ALPHA));
	const double IIR_B1 = (IIR_INPUT_B1 * IIR_ALPHA) + (g_buffer(IIR_DEST_B1, ptr) * (1.f - IIR_ALPHA));
	
	s_buffer1(IIR_DEST_A0, IIR_A0, ptr);
	s_buffer1(IIR_DEST_A1, IIR_A1, ptr);
	s_buffer1(IIR_DEST_B0, IIR_B0, ptr);
	s_buffer1(IIR_DEST_B1, IIR_B1, ptr);
	
	ACC0 = (g_buffer(ACC_SRC_A0, ptr) * ACC_COEF_A) +
		(g_buffer(ACC_SRC_B0, ptr) * ACC_COEF_B) +
		(g_buffer(ACC_SRC_C0, ptr) * ACC_COEF_C) +
		(g_buffer(ACC_SRC_D0, ptr) * ACC_COEF_D);
	ACC1 = (g_buffer(ACC_SRC_A1, ptr) * ACC_COEF_A) +
		(g_buffer(ACC_SRC_B1, ptr) * ACC_COEF_B) +
		(g_buffer(ACC_SRC_C1, ptr) * ACC_COEF_C) +
		(g_buffer(ACC_SRC_D1, ptr) * ACC_COEF_D);
	
	FB_A0 = g_buffer(MIX_DEST_A0 - FB_SRC_A, ptr);
	FB_A1 = g_buffer(MIX_DEST_A1 - FB_SRC_A, ptr);
	FB_B0 = g_buffer(MIX_DEST_B0 - FB_SRC_B, ptr);
	FB_B1 = g_buffer(MIX_DEST_B1 - FB_SRC_B, ptr);
	
	s_buffer(MIX_DEST_A0, ACC0 - (FB_A0 * FB_ALPHA), ptr);
	s_buffer(MIX_DEST_A1, ACC1 - (FB_A1 * FB_ALPHA), ptr);
	
	s_buffer(MIX_DEST_B0, (FB_ALPHA * ACC0) - (FB_A0 * (FB_ALPHA - 1.f)) - (FB_B0 * FB_X), ptr);
	s_buffer(MIX_DEST_B1, (FB_ALPHA * ACC1) - (FB_A1 * (FB_ALPHA - 1.f)) - (FB_B1 * FB_X), ptr);
	
	iRVBLeft  = (g_buffer(MIX_DEST_A0, ptr)+g_buffer(MIX_DEST_B0, ptr))/3.f;
	iRVBRight = (g_buffer(MIX_DEST_A1, ptr)+g_buffer(MIX_DEST_B1, ptr))/3.f;
	
	CurrAddr++;
	if(CurrAddr>delay-1) CurrAddr=0;
	
	return (double)iRVBLeft;
}

double MixREVERBRight(void)
{
	return (double)iRVBRight;
}

IRAM_ATTR void MV_FPReverb(int volume)
{
	int i, count = MV_BufferSize / MV_SampleSize * MV_Channels;

//	sprintf(err, "count: %d, old_delay: %d", count, delay);
	//EnterCriticalSection(&reverbCS);
	SDL_mutexP(reverbMutex);

	check_buffer();

	// DAVE
	if(delay == 0)
	{
		//get out now!!!
		printf("Error! MV_FPReverb() delay==0\n");
		return;
	}

//	OutputDebugStringA(err);
	
	if (MV_Channels == 1)
	{
		for (i = 0; i < count; i++)
		{
			double temp = MV_FooBuffer[i];
			MV_FooBuffer[i] += ((MixREVERBLeft(temp, temp, reverbBuffer) + MixREVERBRight()) * .5) * (double)volume / (double)MV_MaxVolume;
		}
	}
	else
	{
		count >>= 1;
		for (i = 0; i < count; i++)
		{
			double left = MV_FooBuffer[i*2];
			double right = MV_FooBuffer[i*2+1];
			double scale = (double)volume / (double)MV_MaxVolume;
			left += MixREVERBLeft(left, right, reverbBuffer) * scale;
			right += MixREVERBRight() * scale;
			MV_FooBuffer[i*2] = left;
			MV_FooBuffer[i*2+1] = right;
		}
	}

	//LeaveCriticalSection(&reverbCS);
	SDL_mutexV(reverbMutex);
}

void MV_FPReverbFree(void)
{
	SDL_mutexP(reverbMutex);
	//EnterCriticalSection(&reverbCS);
	delay = 0;
	if (reverbBuffer)
	{
		free(reverbBuffer);
		reverbBuffer = 0;
	}
	//LeaveCriticalSection(&reverbCS);
	SDL_mutexV(reverbMutex);
}

void MV_16BitDownmix(char *dest, int count)
{
	int i;

	short *pdest = (short *)dest;

	for (i = 0; i < count; i++)
	{
		int out = (int)((MV_FooBuffer[i] * (double)0x8000));
		if (out < -32768) pdest[i] = -32768;
		else if (out > 32767) pdest[i] = 32767;
		else pdest[i] = out;
	}
}

void MV_8BitDownmix(char *dest, int count)
{
	int i;

	for (i = 0; i < count; i++)
	{
		int out = ((int)((MV_FooBuffer[i] * (double)0x80)));
		if (out < -128) dest[i] = 0;
		else if (out > 127) dest[i] = 255;
		else dest[i] = out + 0x80;
	}
}

void MV_16BitReverbFast( const char *src, char *dest, int count, int shift )
{
	int i;

	short *pdest = (short *)dest;
	const short *psrc = (const short *)src;
	
	for (i = 0; i < count; i++) {
		pdest[i] = psrc[i] >> shift;
	}
}

void MV_8BitReverbFast( const signed char *src, signed char *dest, int count, int shift )
{
	int i;

	unsigned char sh = 0x80 - (0x80 >> shift);
	
	for (i = 0; i < count; i++) {
		unsigned char a = ((unsigned char) src[i]) >> shift;
		unsigned char c = (((unsigned char) src[i]) ^ 0x80) >> 7;
		
		dest[i] = (signed char) (a + sh + c);
	}
}
