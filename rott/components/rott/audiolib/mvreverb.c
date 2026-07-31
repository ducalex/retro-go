#include "rt_def.h"
#include "multivoc.h"
#include "_multivc.h"

static inline int MV_Scale8BitSample(int sample, int volume)
{
	return ((sample - 128) * volume) / MV_MaxVolume;
}

static inline int MV_Scale16BitSample(int sample, int volume)
{
	return ((sample * 256 - 32768) * volume) / MV_MaxVolume;
}

void MV_16BitReverb( const char *src, char *dest, int volume, int count )
{
	int i;

	short *pdest = (short *)dest;
	const unsigned char *source = (const unsigned char *)src;
	
	for (i = 0; i < count; i++) {
		int sl = source[i*2+0];
		int sh = source[i*2+1] ^ 0x80;
		
		sl = MV_Scale16BitSample(sl, volume) >> 8;
		sh = MV_Scale16BitSample(sh, volume);
		
		pdest[i] = (short)(sl + sh + 0x80);
	}
}

void MV_8BitReverb( const signed char *src, signed char *dest, int volume, int count )
{
	int i;

	for (i = 0; i < count; i++) {
		unsigned char s = (unsigned char) src[i];
		
		s = MV_Scale8BitSample(s, volume) & 0xff;
		
		dest[i] = (char)(s + 0x80);
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
