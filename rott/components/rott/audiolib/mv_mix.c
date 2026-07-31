#include "rt_def.h"
#include "multivoc.h"
#include "_multivc.h"

extern char  *MV_MixDestination;
extern unsigned long MV_MixPosition;

extern int MV_LeftVolume;
extern int MV_RightVolume;

extern unsigned char *MV_HarshClipTable;

extern int MV_RightChannelOffset;
extern int MV_SampleSize;

static inline int MV_Scale8BitSample(int sample, int volume)
{
	return ((sample - 128) * volume) / MV_MaxVolume;
}

static inline int MV_Scale16BitSample(int sample, int volume)
{
	return ((sample * 256 - 32768) * volume) / MV_MaxVolume;
}

// Saturation mix formula for 8-bit unsigned:
// Centered at 128. Accumulate relative differences.
#define SATURATE_MIX_8BIT(current, delta) do { \
    int val = (int)(current) + (int)(delta); \
    if (val < 0) val = 0; \
    if (val > 255) val = 255; \
    (current) = (unsigned char)val; \
} while(0)

void MV_Mix8BitMono( unsigned long position, unsigned long rate,
   const char *start, unsigned long length )
{
	const unsigned char *src;
	unsigned char *dest;
	unsigned int i;

	src = (const unsigned char *)start;
	dest = (unsigned char *)MV_MixDestination;

	for (i = 0; i < length; i++) {
		int s = src[position >> 16];
        
        // dest is unsigned byte (128 is silence).
        SATURATE_MIX_8BIT(*dest, MV_Scale8BitSample(s, MV_LeftVolume));
		
		position += rate;
		dest += MV_SampleSize;
	}
	
	MV_MixPosition = position;
	MV_MixDestination = (char *)dest;
}

void MV_Mix8BitStereo( unsigned long position,
   unsigned long rate, const char *start, unsigned long length )
{
	const unsigned char *src;
	unsigned char *dest;
	unsigned int i;
	
	src = (const unsigned char *)start;
	dest = (unsigned char *)MV_MixDestination;

	for (i = 0; i < length; i++) {
		int s = src[(position >> 16)];
		
		SATURATE_MIX_8BIT(dest[0], MV_Scale8BitSample(s, MV_LeftVolume));
		SATURATE_MIX_8BIT(dest[MV_RightChannelOffset], MV_Scale8BitSample(s, MV_RightVolume));
		
		position += rate;
		dest += MV_SampleSize;
	}
	
	MV_MixPosition = position;
	MV_MixDestination = (char *)dest;
}

void MV_Mix16BitMono( unsigned long position,
   unsigned long rate, const char *start, unsigned long length )
{
	const unsigned char *src;
	short *dest;
	unsigned int i;

	src = (const unsigned char *)start;
	dest = (short *)MV_MixDestination;
	
	for (i = 0; i < length; i++) {
		int s = src[position >> 16];
		int d = dest[0];
		
		s = MV_Scale16BitSample(s, MV_LeftVolume);
		s += d;
		
		if (s < -32768) s = -32768;
		if (s >  32767) s =  32767;
		
		*dest = (short) s;
		
		position += rate;
		dest += MV_SampleSize/2;
	}
	
	MV_MixPosition = position;
	MV_MixDestination = (char *)dest;
}

void MV_Mix16BitStereo( unsigned long position,
   unsigned long rate, const char *start, unsigned long length )
{
	const unsigned char *src;
	short *dest;
	unsigned int i;

	src = (unsigned char *)start;
	dest = (short *)MV_MixDestination;
	
	for (i = 0; i < length; i++) {
		int s = src[position >> 16];
		int dl = dest[0];
		int dr = dest[MV_RightChannelOffset/2];
		
		dl += MV_Scale16BitSample(s, MV_LeftVolume);
		dr += MV_Scale16BitSample(s, MV_RightVolume);
		
		if (dl < -32768) dl = -32768;
		if (dl >  32767) dl =  32767;
		if (dr < -32768) dr = -32768;
		if (dr >  32767) dr =  32767;
		
		dest[0] = (short) dl;
		dest[MV_RightChannelOffset/2] = (short) dr;
		
		position += rate;
		dest += MV_SampleSize/2;
	}
	
	MV_MixPosition = position;
	MV_MixDestination = (char *)dest;
}

void MV_Mix8BitMono16( unsigned long position, unsigned long rate,
   const char *start, unsigned long length )
{
	const unsigned char *src;
	unsigned char *dest;
	unsigned int i;

	src = (const unsigned char *)start;
	dest = (unsigned char *)MV_MixDestination;

	for (i = 0; i < length; i++) {
		int s = src[(position >> 16) * 2 + 1];
        SATURATE_MIX_8BIT(*dest, MV_Scale8BitSample(s, MV_LeftVolume));
		
		position += rate;
		dest += MV_SampleSize;
	}
	
	MV_MixPosition = position;
	MV_MixDestination = (char *)dest;
}

void MV_Mix8BitStereo16( unsigned long position,
   unsigned long rate, const char *start, unsigned long length )
{
	const unsigned char *src;
	unsigned char *dest;
	unsigned int i;
	
	src = (const unsigned char *)start;
	dest = (unsigned char *)MV_MixDestination;

	for (i = 0; i < length; i++) {
		int s = src[(position >> 16) * 2 + 1];
		
		SATURATE_MIX_8BIT(dest[0], MV_Scale8BitSample(s, MV_LeftVolume));
		SATURATE_MIX_8BIT(dest[MV_RightChannelOffset], MV_Scale8BitSample(s, MV_RightVolume));
		
		position += rate;
		dest += MV_SampleSize;
	}
	
	MV_MixPosition = position;
	MV_MixDestination = (char *)dest;
}

void MV_Mix16BitMono16( unsigned long position,
   unsigned long rate, const char *start, unsigned long length )
{
	const short *src;
	short *dest;
	unsigned int i;
	
	src = (const short *)start;
	dest = (short *)MV_MixDestination;
	
	for (i = 0; i < length; i++) {
		int s = src[position >> 16];
        int idx = (s >> 8) + 128;
		int d = *dest;
		
		s = MV_Scale16BitSample(idx & 0xff, MV_LeftVolume);
		d = s + d;
		
		if (d < -32768) d = -32768;
		if (d >  32767) d =  32767;
		
		*dest = (short) d;
		
		position += rate;
		dest += MV_SampleSize/2;
	}
	
	MV_MixPosition = position;
	MV_MixDestination = (char *)dest;
}

void MV_Mix16BitStereo16( unsigned long position,
   unsigned long rate, const char *start, unsigned long length )
{
	const short *src;
	short *dest;
	unsigned int i;
	
	src = (const short *)start;
	dest = (short *)MV_MixDestination;
	
	for (i = 0; i < length; i++) {
		int s = src[position >> 16];
        int idx = (s >> 8) + 128;
		
		int dl = dest[0];
		int dr = dest[MV_RightChannelOffset/2];
		
		int sl = MV_Scale16BitSample(idx & 0xff, MV_LeftVolume);
		int sr = MV_Scale16BitSample(idx & 0xff, MV_RightVolume);
		
		dl = sl + dl;
		dr = sr + dr;
		
		if (dl < -32768) dl = -32768;
		if (dl >  32767) dl =  32767;
		if (dr < -32768) dr = -32768;
		if (dr >  32767) dr =  32767;
		
		dest[0] = (short) dl;
		dest[MV_RightChannelOffset/2] = (short) dr;
		
		position += rate;
		dest += MV_SampleSize/2;
	}
	
	MV_MixPosition = position;
	MV_MixDestination = (char *)dest;
}
