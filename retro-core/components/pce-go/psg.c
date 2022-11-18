// psg.c - Programmable Sound Generator
//
#include <stdlib.h>
#include <string.h>
#include "pce.h"
#include "psg.h"

static const uint8_t vol_tbl[32] = {
	100 >> 8, 451 >> 8, 508 >> 8, 573 >> 8, 646 >> 8, 728 >> 8, 821 >> 8, 925 >> 8,
	1043 >> 8, 1175 >> 8, 1325 >> 8, 1493 >> 8, 1683 >> 8, 1898 >> 8, 2139 >> 8, 2411 >> 8,
	2718 >> 8, 3064 >> 8, 3454 >> 8, 3893 >> 8, 4388 >> 8, 4947 >> 8, 5576 >> 8, 6285 >> 8,
	7085 >> 8, 7986 >> 8, 9002 >> 8, 10148 >> 8, 11439 >> 8, 12894 >> 8, 14535 >> 8, 16384 >> 8
};

// The buffer should be signed but it seems to sound better
// unsigned. I am still reviewing the implementation bellow.
// In some games it also sounds better in 8 bit than in 16...
// typedef uint8_t sample_t;
typedef int16_t sample_t;

static int samplerate = 22050;
static int stereo = true;


static inline void
psg_update_chan(sample_t *buf, int ch, size_t dwSize)
{
	psg_chan_t *chan = &PCE.PSG.chan[ch];
	int sample = 0;
	uint32_t Tp;
	sample_t *buf_end = buf + dwSize;

	/*
	* This gives us a volume level of (0...15).
	*/
	int lvol = (((chan->balance >> 4) * 1.1) * (chan->control & 0x1F)) / 32;
	int rvol = (((chan->balance & 0xF) * 1.1) * (chan->control & 0x1F)) / 32;

	if (!stereo) {
		lvol = (lvol + rvol) / 2;
	}

	// This isn't very accurate, we don't track how long each DA sample should play
	// but we call psg_update() often enough (10x per frame) that guessing should be good enough...
	if (chan->dda_count) {
		// Cycles per frame: 119318
		// Samples per frame: 368

		// Cycles per scanline: 454
		// Samples per scanline: ~1.4

		// One sample = 324 cycles

		// const int cycles_per_sample = CYCLES_PER_FRAME / (host.sound.sample_freq / 60);

		// float repeat = (float)elapsed / cycles_per_sample / chan->dda_count;
		// MESSAGE_INFO("%.2f\n", repeat);

		int start = (int)chan->dda_index - chan->dda_count;
		if (start < 0)
			start += 0x100;

		int repeat = 3; // MIN(2, (dwSize / 2) / chan->dda_count) + 1;

		lvol = vol_tbl[lvol << 1];
		rvol = vol_tbl[rvol << 1];

		while (buf < buf_end && (chan->dda_count || chan->control & PSG_DDA_ENABLE)) {
			if (chan->dda_count) {
				// sample = chan->dda_data[(start++) & 0x7F];
				if ((sample = (chan->dda_data[(start++) & 0xFF] - 16)) >= 0)
					sample++;
				chan->dda_count--;
			}

			for (int i = 0; i < repeat; i++) {
				*buf++ = (sample * lvol);

				if (stereo) {
					*buf++ = (sample * rvol);
				}
			}
		}
	}

	/*
	* Do nothing if there is no audio to be played on this channel.
	*/
	if (!(chan->control & PSG_CHAN_ENABLE)) {
		chan->wave_accum = 0;
	}
	/*
	* PSG Noise generation (it has priority over DDA and WAVE)
	*/
	else if ((ch == 4 || ch == 5) && (chan->noise_ctrl & PSG_NOISE_ENABLE)) {
		int Np = (chan->noise_ctrl & 0x1F);

		while (buf < buf_end) {
			chan->noise_accum += 3000 + Np * 512;

			if ((Tp = (chan->noise_accum / samplerate)) >= 1) {
				if (chan->noise_rand & 0x00080000) {
					chan->noise_rand = ((chan->noise_rand ^ 0x0004) << 1) + 1;
					chan->noise_level = -15;
				} else {
					chan->noise_rand <<= 1;
					chan->noise_level = 15;
				}
				chan->noise_accum -= samplerate * Tp;
			}

			*buf++ = (chan->noise_level * lvol);

			if (stereo) {
				*buf++ = (chan->noise_level * rvol);
			}
		}
	}
	/*
	* There is 'direct access' audio to be played.
	*/
	else if (chan->control & PSG_DDA_ENABLE) {

	}
	/*
	* PSG Wave generation.
	*/
	else if ((Tp = chan->freq_lsb + (chan->freq_msb << 8)) > 0) {
		/*
		 * Thank god for well commented code!  The original line of code read:
		 * fixed_inc = ((uint32_t) (3.2 * 1118608 / samplerate) << 16) / Tp;
		 * and had nary a comment to be found.  It took a little head scratching to get
		 * it figured out.  The 3.2 * 1118608 comes out to 3574595.6 which is obviously
		 * meant to represent the 3.58mhz cpu clock speed used in the pc engine to
		 * decrement the sound 'frequency'.  I haven't figured out why the original
		 * author had the two numbers multiplied together to get the odd value instead of
		 * just using 3580000.  I did some checking and the value will compute the same
		 * using either value divided by any standard soundcard samplerate.
		 *
		 * Taken from the PSG doc written by Paul Clifford (paul@plasma.demon.co.uk)
		 * <in reference to the 12 bit frequency value in PSG registers 2 and 3>
		 * "For waveform output, a copy of this value is, in effect, decremented 3,580,000
		 *  times a second until zero is reached.  When this happens the PSG advances an
		 *  internal pointer into the channel's waveform buffer by one."
		 *
		 * So all we need to do to emulate original pc engine behaviour is take our soundcard's
		 * sampling rate into consideration with regard to the 3580000 effective pc engine
		 * samplerate.  We use 16.16 fixed arithmetic for speed.
		 */
		uint32_t fixed_inc = ((CLOCK_PSG / samplerate) << 16) / Tp;

		while (buf < buf_end) {
			if ((sample = (chan->wave_data[chan->wave_index] - 16)) >= 0)
				sample++;

			*buf++ = (sample * lvol);

			if (stereo) {
				*buf++ = (sample * rvol);
			}

			chan->wave_accum += fixed_inc;
			chan->wave_accum &= 0x1FFFFF;	/* (31 << 16) + 0xFFFF */
			chan->wave_index = chan->wave_accum >> 16;
		}
	}

	if (buf < buf_end) {
		memset(buf, 0, (void*)buf_end - (void*)buf);
	}
}


int
psg_init(int _samplerate, bool _stereo)
{
	PCE.PSG.chan[4].noise_rand = 0x51F63101;
	PCE.PSG.chan[5].noise_rand = 0x1F631042;

	samplerate = _samplerate;
	stereo = _stereo;

	return 0;
}


void
psg_term(void)
{
	//
}


void
psg_update(int16_t *output, size_t length, bool downsample)
{
	int lvol = (PCE.PSG.volume >> 4);
	int rvol = (PCE.PSG.volume & 0x0F);

	if (stereo) {
		length *= 2;
	}

	memset(output, 0, length * sizeof(int16_t));

	for (int i = 0; i < PSG_CHANNELS; i++)
	{
		sample_t mix_buffer[length + 1];
		psg_update_chan(mix_buffer, i, length);

		if (downsample) {
			for (int j = 0; j < length; j += 2) {
				output[j] += (uint8_t)mix_buffer[j] * lvol;
				output[j + 1] += (uint8_t)mix_buffer[j + 1] * rvol;
			}
		} else {
			for (int j = 0; j < length; j += 2) {
				output[j] += mix_buffer[j] * lvol;
				output[j + 1] += mix_buffer[j + 1] * rvol;
			}
		}
	}
}
