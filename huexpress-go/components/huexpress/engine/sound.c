// sound.c - Sound emulation
//
#include "sound.h"
#include "pce.h"

static const uint8_t vol_tbl[32] = {
    100 >> 8, 451 >> 8, 508 >> 8, 573 >> 8, 646 >> 8, 728 >> 8, 821 >> 8, 925 >> 8,
    1043 >> 8, 1175 >> 8, 1325 >> 8, 1493 >> 8, 1683 >> 8, 1898 >> 8, 2139 >> 8, 2411 >> 8,
    2718 >> 8, 3064 >> 8, 3454 >> 8, 3893 >> 8, 4388 >> 8, 4947 >> 8, 5576 >> 8, 6285 >> 8,
    7085 >> 8, 7986 >> 8, 9002 >> 8, 10148 >> 8, 11439 >> 8, 12894 >> 8, 14535 >> 8, 16384 >> 8
};

static uint32_t noise_rand[PSG_CHANNELS];
static int32_t noise_level[PSG_CHANNELS];

// The buffer should be signed but it seems to sound better
// unsigned. I am still reviewing the implementation bellow.
static int8_t mix_buffer[44100 / 60 * 2];
// static uint8_t mix_buffer[44100 / 60 * 2];

static int last_cycles = 0;


static inline void
psg_update(int8_t *buf, int ch, size_t dwSize)
{
    psg_chan_t *chan = &PCE.PSG.chan[ch];
    int vol, sample = 0;
    uint32_t Tp;
    int8_t *buf_end = buf + dwSize;

    /*
    * We multiply the 4-bit balance values by 1.1 to get a result from (0..16.5).
    * This multiplied by the 5-bit channel volume (0..31) gives us a result of
    * (0..511).
    */
    int lbal = ((chan->balance >> 4) * 1.1) * (chan->control & 0x1F);
    int rbal = ((chan->balance & 0xF) * 1.1) * (chan->control & 0x1F);

    if (!host.sound.stereo) {
        lbal = (lbal + rbal) / 2;
    }

    // This isn't very accurate, we don't track how long each DA sample should play
    // but we call psg_update() often enough (10x per frame) that guessing should be good enough...
    if (chan->dda_count) {
        // Cycles per frame: 119318
        // Samples per frame: 368

        // Cycles per scanline: 454
        // Samples per scanline: ~1.4

        // One sample = 324 cycles

        int elapsed = Cycles - last_cycles;
        const int cycles_per_sample = CYCLES_PER_FRAME / (host.sound.sample_freq / 60);

        // float repeat = (float)elapsed / cycles_per_sample / chan->dda_count;
        // printf("%.2f\n", repeat);

        int start = (int)chan->dda_index - chan->dda_count;
        if (start < 0)
            start += 0x80;

        int repeat = 3; // MIN(2, (dwSize / 2) / chan->dda_count) + 1;

        while (buf < buf_end && (chan->dda_count || chan->control & PSG_DDA_ENABLE)) {
            if (chan->dda_count) {
                // sample = chan->dda_data[(start++) & 0x7F];
                if ((sample = (chan->dda_data[(start++) & 0x7F] - 16)) >= 0)
                    sample++;
                chan->dda_count--;
            }

            for (int i = 0; i < repeat; i++) {
                *buf++ = (int8_t) (sample * lbal / 50);  // 64

                if (host.sound.stereo) {
                    *buf++ = (int8_t) (sample * rbal / 50);  // 64
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

        vol = MAX((PCE.PSG.volume >> 3) & 0x1E, (PCE.PSG.volume << 1) & 0x1E) +
              (chan->control & PSG_CHAN_VOLUME) +
              MAX((chan->balance >> 3) & 0x1E, (chan->balance << 1) & 0x1E);

        vol = vol_tbl[MAX(0, vol - 60)];

        while (buf < buf_end) {
            chan->noise_accum += 3000 + Np * 512;

            if ((Tp = (chan->noise_accum / host.sound.sample_freq)) >= 1) {
                if (noise_rand[ch] & 0x00080000) {
                    noise_rand[ch] = ((noise_rand[ch] ^ 0x0004) << 1) + 1;
                    noise_level[ch] = -10 * 702;
                } else {
                    noise_rand[ch] <<= 1;
                    noise_level[ch] = 10 * 702;
                }
                chan->noise_accum -= host.sound.sample_freq * Tp;
            }

            *buf++ = (int8_t) (noise_level[ch] * vol / 4096);
        }
    }
    /*
    * There is 'direct access' audio to be played.
    */
    else if (chan->control & PSG_DDA_ENABLE) {
        #if 0
        int start = (int)chan->dda_index - chan->dda_count;
        if (start < 0)
            start += 0x80;

        while (buf < buf_end) {
            if (chan->dda_count) {
                sample = chan->dda_data[start++];
                start &= 0x7F;
                chan->dda_count--;
            }

            *buf++ = (int8_t) (sample * lbal / 64);

            if (host.sound.stereo) {
                *buf++ = (int8_t) (sample * rbal / 64);
            }

            *buf++ = (int8_t) (sample * lbal / 64);

            if (host.sound.stereo) {
                *buf++ = (int8_t) (sample * rbal / 64);
            }

            *buf++ = (int8_t) (sample * lbal / 64);

            if (host.sound.stereo) {
                *buf++ = (int8_t) (sample * rbal / 64);
            }
        }
#endif
    }
    /*
    * PSG Wave generation.
    */
    else if ((Tp = chan->freq_lsb + (chan->freq_msb << 8)) > 0) {
        /*
         * Thank god for well commented code!  The original line of code read:
         * fixed_inc = ((uint32_t) (3.2 * 1118608 / host.sound.sample_freq) << 16) / Tp;
         * and had nary a comment to be found.  It took a little head scratching to get
         * it figured out.  The 3.2 * 1118608 comes out to 3574595.6 which is obviously
         * meant to represent the 3.58mhz cpu clock speed used in the pc engine to
         * decrement the sound 'frequency'.  I haven't figured out why the original
         * author had the two numbers multiplied together to get the odd value instead of
         * just using 3580000.  I did some checking and the value will compute the same
         * using either value divided by any standard soundcard samplerate.  The
         * host.sound.sample_freq is our soundcard's samplerate which is quite a bit slower than
         * the pce's cpu (3580000 vs. 22050/44100 typically).
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
        uint32_t fixed_inc = ((CLOCK_PSG / host.sound.sample_freq) << 16) / Tp;

        while (buf < buf_end) {
            if ((sample = (chan->wave_data[chan->wave_index] - 16)) >= 0)
                sample++;

            *buf++ = (int8_t) (sample * lbal / 100); // 64

            if (host.sound.stereo) {
                *buf++ = (int8_t) (sample * rbal / 100); // 64
            }

            chan->wave_accum += fixed_inc;
            chan->wave_accum &= 0x1FFFFF;    /* (31 << 16) + 0xFFFF */
            chan->wave_index = chan->wave_accum >> 16;
        }
    }

pad_and_return:
    if (buf < buf_end) {
        memset(buf, 0, buf_end - buf);
    }
}


int
snd_init(void)
{
    noise_rand[4] = 0x51F63101;
    noise_rand[5] = 0x1F631042;

    osd_snd_init();

    return 0;
}


void
snd_term(void)
{
    osd_snd_shutdown();
}


void
snd_update(short *buffer, size_t length)
{
    int lvol = (PCE.PSG.volume >> 4);
    int rvol = (PCE.PSG.volume & 0x0F);

    if (host.sound.stereo) {
        length *= 2;
    }

    memset(buffer, 0, length * 2);

    for (int i = 0; i < PSG_CHANNELS; i++)
    {
        psg_update((void*)mix_buffer, i, length);

        for (int j = 0; j < length; j += 2)
        {
            buffer[j] += mix_buffer[j] * lvol;
            buffer[j + 1] += mix_buffer[j + 1] * rvol;
        }
    }

    last_cycles = Cycles;
}
