#include <string.h>
#include <stdlib.h>
#include "gnuboy.h"
#include "sound.h"
#include "hw.h"
#include "tables.h"

#define S1 (snd.ch[0])
#define S2 (snd.ch[1])
#define S3 (snd.ch[2])
#define S4 (snd.ch[3])

#define s1_freq() {int d = 2048 - (((R_NR14&7)<<8) + R_NR13); S1.freq = (snd.rate > (d<<4)) ? 0 : (snd.rate << 17)/d;}
#define s2_freq() {int d = 2048 - (((R_NR24&7)<<8) + R_NR23); S2.freq = (snd.rate > (d<<4)) ? 0 : (snd.rate << 17)/d;}
#define s3_freq() {int d = 2048 - (((R_NR34&7)<<8) + R_NR33); S3.freq = (snd.rate > (d<<3)) ? 0 : (snd.rate << 21)/d;}
#define s4_freq() {S4.freq = (freqtab[R_NR43&7] >> (R_NR43 >> 4)) * snd.rate; if (S4.freq >> 18) S4.freq = 1<<18;}

static gb_snd_t snd;


void sound_dirty(void)
{
	S1.swlen = ((R_NR10>>4) & 7) << 14;
	S1.len = (64-(R_NR11&63)) << 13;
	S1.envol = R_NR12 >> 4;
	S1.endir = (R_NR12>>3) & 1;
	S1.endir |= S1.endir - 1;
	S1.enlen = (R_NR12 & 7) << 15;
	s1_freq();

	S2.len = (64-(R_NR21&63)) << 13;
	S2.envol = R_NR22 >> 4;
	S2.endir = (R_NR22>>3) & 1;
	S2.endir |= S2.endir - 1;
	S2.enlen = (R_NR22 & 7) << 15;
	s2_freq();

	S3.len = (256-R_NR31) << 20;
	s3_freq();

	S4.len = (64-(R_NR41&63)) << 13;
	S4.envol = R_NR42 >> 4;
	S4.endir = (R_NR42>>3) & 1;
	S4.endir |= S4.endir - 1;
	S4.enlen = (R_NR42 & 7) << 15;
	s4_freq();
}

static void sound_off(void)
{
	memset(&S1, 0, sizeof S1);
	memset(&S2, 0, sizeof S2);
	memset(&S3, 0, sizeof S3);
	memset(&S4, 0, sizeof S4);
	R_NR10 = 0x80;
	R_NR11 = 0xBF;
	R_NR12 = 0xF3;
	R_NR14 = 0xBF;
	R_NR21 = 0x3F;
	R_NR22 = 0x00;
	R_NR24 = 0xBF;
	R_NR30 = 0x7F;
	R_NR31 = 0xFF;
	R_NR32 = 0x9F;
	R_NR34 = 0xBF;
	R_NR41 = 0xFF;
	R_NR42 = 0x00;
	R_NR43 = 0x00;
	R_NR44 = 0xBF;
	R_NR50 = 0x77;
	R_NR51 = 0xF3;
	R_NR52 = 0x70;
	sound_dirty();
}

gb_snd_t *sound_init(void)
{
	return &snd;
}

void sound_reset(bool hard)
{
	memset(&snd, 0, sizeof(snd));
	memcpy(snd.wave, hw.hwtype == GB_HW_CGB ? cgbwave : dmgwave, 16);
	memcpy(hw.ioregs + 0x30, snd.wave, 16);
	snd.rate = (int)(((1<<21) / (double)host.audio.samplerate) + 0.5);
	host.audio.pos = 0;
	sound_off();
	R_NR52 = 0xF1;
}

void sound_emulate(void)
{
	if (!snd.rate || snd.cycles < snd.rate)
		return;

	int16_t *output_buf = host.audio.buffer + host.audio.pos;
	int16_t *output_end = host.audio.buffer + host.audio.len;
	bool stereo = host.audio.stereo;

	for (; snd.cycles >= snd.rate; snd.cycles -= snd.rate)
	{
		int l = 0;
		int r = 0;

		if (S1.on)
		{
			int s = sqwave[R_NR11>>6][(S1.pos>>18)&7] & S1.envol;
			S1.pos += S1.freq;

			if ((R_NR14 & 64) && ((S1.cnt += snd.rate) >= S1.len))
				S1.on = 0;

			if (S1.enlen && (S1.encnt += snd.rate) >= S1.enlen)
			{
				S1.encnt -= S1.enlen;
				S1.envol += S1.endir;
				if (S1.envol < 0) S1.envol = 0;
				if (S1.envol > 15) S1.envol = 15;
			}

			if (S1.swlen && (S1.swcnt += snd.rate) >= S1.swlen)
			{
				S1.swcnt -= S1.swlen;
				int f = S1.swfreq;

				if (R_NR10 & 8)
					f -= (f >> (R_NR10 & 7));
				else
					f += (f >> (R_NR10 & 7));

				if (f > 2047)
					S1.on = 0;
				else
				{
					S1.swfreq = f;
					R_NR13 = f;
					R_NR14 = (R_NR14 & 0xF8) | (f>>8);
					s1_freq();
				}
			}
			s <<= 2;
			if (R_NR51 & 1) r += s;
			if (R_NR51 & 16) l += s;
		}

		if (S2.on)
		{
			int s = sqwave[R_NR21>>6][(S2.pos>>18)&7] & S2.envol;
			S2.pos += S2.freq;

			if ((R_NR24 & 64) && ((S2.cnt += snd.rate) >= S2.len))
				S2.on = 0;

			if (S2.enlen && (S2.encnt += snd.rate) >= S2.enlen)
			{
				S2.encnt -= S2.enlen;
				S2.envol += S2.endir;
				if (S2.envol < 0) S2.envol = 0;
				if (S2.envol > 15) S2.envol = 15;
			}
			s <<= 2;
			if (R_NR51 & 2) r += s;
			if (R_NR51 & 32) l += s;
		}

		if (S3.on)
		{
			int s = snd.wave[(S3.pos>>22) & 15];

			if (S3.pos & (1<<21))
				s &= 15;
			else
				s >>= 4;

			s -= 8;
			S3.pos += S3.freq;

			if ((R_NR34 & 64) && ((S3.cnt += snd.rate) >= S3.len))
				S3.on = 0;

			if (R_NR32 & 96)
				s <<= (3 - ((R_NR32>>5)&3));
			else
				s = 0;

			if (R_NR51 & 4) r += s;
			if (R_NR51 & 64) l += s;
		}

		if (S4.on)
		{
			int s;

			if (R_NR43 & 8)
				s = 1 & (noise7[(S4.pos>>20)&15] >> (7-((S4.pos>>17)&7)));
			else
				s = 1 & (noise15[(S4.pos>>20)&4095] >> (7-((S4.pos>>17)&7)));

			s = (-s) & S4.envol;
			S4.pos += S4.freq;

			if ((R_NR44 & 64) && ((S4.cnt += snd.rate) >= S4.len))
				S4.on = 0;

			if (S4.enlen && (S4.encnt += snd.rate) >= S4.enlen)
			{
				S4.encnt -= S4.enlen;
				S4.envol += S4.endir;
				if (S4.envol < 0) S4.envol = 0;
				if (S4.envol > 15) S4.envol = 15;
			}

			s += s << 1;

			if (R_NR51 & 8) r += s;
			if (R_NR51 & 128) l += s;
		}

		l *= (R_NR50 & 0x07);
		r *= ((R_NR50 & 0x70)>>4);

		l <<= 4;
		r <<= 4;

		if (!output_buf || output_buf >= output_end)
		{
			continue;
		}
		else if (stereo)
		{
			*output_buf++ = (int16_t)l; //+128;
			*output_buf++ = (int16_t)r; //+128;
		}
		else
		{
			*output_buf++ = (int16_t)((l+r)>>1); //+128;
		}
	}

	host.audio.pos = output_buf - host.audio.buffer;

	R_NR52 = (R_NR52&0xf0) | S1.on | (S2.on<<1) | (S3.on<<2) | (S4.on<<3);
}

void sound_write(byte r, byte b)
{
	if (!(R_NR52 & 128) && r != RI_NR52)
		return;

	if (snd.cycles >= snd.rate)
		sound_emulate();

	switch (r)
	{
	/* Square 1 */
	case RI_NR10:
		R_NR10 = b;
		S1.swlen = ((R_NR10>>4) & 7) << 14;
		S1.swfreq = ((R_NR14&7)<<8) + R_NR13;
		break;
	case RI_NR11:
		R_NR11 = b;
		S1.len = (64-(R_NR11&63)) << 13;
		break;
	case RI_NR12:
		R_NR12 = b;
		S1.envol = R_NR12 >> 4;
		S1.endir = (R_NR12>>3) & 1;
		S1.endir |= S1.endir - 1;
		S1.enlen = (R_NR12 & 7) << 15;
		break;
	case RI_NR13:
		R_NR13 = b;
		s1_freq();
		break;
	case RI_NR14:
		R_NR14 = b;
		s1_freq();
		if (b & 0x80) // Trigger
		{
			S1.swcnt = 0;
			S1.swfreq = ((R_NR14&7)<<8) + R_NR13;
			S1.envol = R_NR12 >> 4;
			S1.endir = (R_NR12>>3) & 1;
			S1.endir |= S1.endir - 1;
			S1.enlen = (R_NR12 & 7) << 15;
			S1.cnt = S1.encnt = 0;
			if (!S1.on)
				S1.on = 1, S1.pos = 0;
		}
		break;

	/* Square 2 */
	case RI_NR21:
		R_NR21 = b;
		S2.len = (64-(R_NR21&63)) << 13;
		break;
	case RI_NR22:
		R_NR22 = b;
		S2.envol = R_NR22 >> 4;
		S2.endir = (R_NR22>>3) & 1;
		S2.endir |= S2.endir - 1;
		S2.enlen = (R_NR22 & 7) << 15;
		break;
	case RI_NR23:
		R_NR23 = b;
		s2_freq();
		break;
	case RI_NR24:
		R_NR24 = b;
		s2_freq();
		if (b & 0x80) // Trigger
		{
			S2.envol = R_NR22 >> 4;
			S2.endir = (R_NR22>>3) & 1;
			S2.endir |= S2.endir - 1;
			S2.enlen = (R_NR22 & 7) << 15;
			S2.cnt = S2.encnt = 0;
			if (!S2.on)
				S2.on = 1, S2.pos = 0;
		}
		break;

	/* Wave */
	case RI_NR30:
		R_NR30 = b;
		if (!(b & 0x80)) S3.on = 0;
		break;
	case RI_NR31:
		R_NR31 = b;
		S3.len = (256-R_NR31) << 13;
		break;
	case RI_NR32:
		R_NR32 = b;
		break;
	case RI_NR33:
		R_NR33 = b;
		s3_freq();
		break;
	case RI_NR34:
		R_NR34 = b;
		s3_freq();
		if (b & 0x80) // Trigger
		{
			if (!S3.on) S3.pos = 0;
			S3.cnt = 0;
			S3.on = R_NR30 >> 7;
			if (!S3.on) return;
			for (int i = 0; i < 16; i++)
				REG(i+0x30) = 0x13 ^ REG(i+0x31);
		}
		break;

	/* Noise */
	case RI_NR41:
		R_NR41 = b;
		S4.len = (64-(R_NR41&63)) << 13;
		break;
	case RI_NR42:
		R_NR42 = b;
		S4.envol = R_NR42 >> 4;
		S4.endir = (R_NR42>>3) & 1;
		S4.endir |= S4.endir - 1;
		S4.enlen = (R_NR42 & 7) << 15;
		break;
	case RI_NR43:
		R_NR43 = b;
		s4_freq();
		break;
	case RI_NR44:
		R_NR44 = b;
		if (b & 0x80) // Trigger
		{
			S4.envol = R_NR42 >> 4;
			S4.endir = (R_NR42>>3) & 1;
			S4.endir |= S4.endir - 1;
			S4.enlen = (R_NR42 & 7) << 15;
			S4.cnt = S4.encnt = 0;
			S4.on = 1, S4.pos = 0;
		}
		break;

	/* Control */
	case RI_NR50:
		R_NR50 = b;
		break;
	case RI_NR51:
		R_NR51 = b;
		break;
	case RI_NR52:
		R_NR52 = b;
		if (!(R_NR52 & 128))
			sound_off();
		break;

	/* Wave Table */
	case 0x30:
	case 0x31:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	case 0x36:
	case 0x37:
	case 0x38:
	case 0x39:
	case 0x3A:
	case 0x3B:
	case 0x3C:
	case 0x3D:
	case 0x3E:
	case 0x3F:
		if (!S3.on) // Wave table writes only go through when the channel is disabled
			snd.wave[r & 0xF] = REG(r) = b;
		break;

	default:
		MESSAGE_DEBUG("Invalid sound register: %02X %02X\n", r, b);
		break;
	}
}
