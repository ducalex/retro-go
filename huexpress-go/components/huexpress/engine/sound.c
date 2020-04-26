#include "osd_machine.h"
#include "sound.h"

extern int BaseClock;

static inline int
mseq(uint32 * rand_val)
{
    if (*rand_val & 0x00080000) {
        *rand_val = ((*rand_val ^ 0x0004) << 1) + 1;
        return 1;
    } else {
        *rand_val <<= 1;
        return 0;
    }
}


void
WriteBuffer(char *buf, int ch, unsigned dwSize)
{
    static uint32 fixed_n[6] = { 0, 0, 0, 0, 0, 0 };
    uint32 fixed_inc;
    static uint32 k[6] = { 0, 0, 0, 0, 0, 0 };
    static uint32 t;            // used to know how much we got to advance in the ring buffer
    static uint32 r[6];
    static uint32 rand_val[6] = { 0, 0, 0, 0, 0x51F631E4, 0x51F631E4 }; // random seed for 'noise' generation
    uint16 dwPos = 0;
    int32 vol;
    uint32 Tp;
    static signed char vol_tbl[32] = {
        /*
         * Funky stuff everywhere!  I'm quite sure there was a reason to use an array
         * of constant values divided by constant values and having the host machine figure
         * it all out . . . that's why I'm leaving the original formula here within the
         * comment.
         *    100 / 256, 451 / 256, 508 / 256, 573 / 256, 646 / 256, 728 / 256,
         *    821 / 256, 925 / 256,
         *    1043 / 256, 1175 / 256, 1325 / 256, 1493 / 256, 1683 / 256, 1898 / 256,
         *    2139 / 256, 2411 / 256,
         *    2718 / 256, 3064 / 256, 3454 / 256, 3893 / 256, 4388 / 256, 4947 / 256,
         *    5576 / 256, 6285 / 256,
         *    7085 / 256, 7986 / 256, 9002 / 256, 10148 / 256, 11439 / 256, 12894 / 256,
         *    14535 / 256, 16384 / 256
         */
        0, 1, 1, 2, 2, 2, 3, 3, 4, 4, 5, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17,
            19, 21, 24, 27, 31, 35, 39, 44, 50, 56, 64
    };
    uint16 lbal, rbal;
    signed char sample;

    if (!(io.PSG[ch][PSG_DDA_REG] & PSG_DDA_ENABLE)
        || io.psg_channel_disabled[ch]) {
        /*
         * There is no audio to be played on this channel.
         */
        fixed_n[ch] = 0;
        memset(buf, 0, dwSize * host.sound.sample_size);
        return;
    }

    if ((io.PSG[ch][PSG_DDA_REG] & PSG_DDA_DIRECT_ACCESS)
        || io.psg_da_count[ch]) {
        /*
         * There is 'direct access' audio to be played.
         */
        static uint32 da_index[6] = { 0, 0, 0, 0, 0, 0 };
        uint16 index = da_index[ch] >> 16;

        /*
         * For this direct audio stuff there is no frequency provided via PSG registers 3
         * and 4.  I'm not sure if this is normal behaviour or if it's something wrong in
         * the emulation but I'm leaning toward the former.
         *
         * The 0x1FF divisor is completely arbitrary.  I adjusted it by listening to the voices
         * in Street Fighter 2 CE.  If anyone has information to improve my "seat of the pants"
         * calculations then by all means *does finger quotes* "throw me a frikkin` bone here".
         *
         * See the big comment in the final else clause for an explanation of this value
         * to the best of my knowledge.
         */
        fixed_inc = ((uint32) (3580000 / host.sound.freq) << 16) / 0x1FF;

        /*
         * Volume handling changed 2-24-03.
         * I believe io.psg_volume should only be used to compute the final sample
         * volume after all the buffers have been mixed together.  Alright, it's what
         * other people have already stated, and I believe them :)
         */

        if (host.sound.stereo) {
            /*
             * We multiply the 4-bit balance values by 1.1 to get a result from (0..16.5).
             * This multiplied by the 5-bit channel volume (0..31) gives us a result of
             * (0..511).
             */
            lbal =
                ((io.PSG[ch][5] >> 4) * 1.1) *
                (io.PSG[ch][4] & PSG_DDA_VOICE_VOLUME);
            rbal =
                ((io.PSG[ch][5] & 0x0F) * 1.1) *
                (io.PSG[ch][4] & PSG_DDA_VOICE_VOLUME);
        } else {
            /*
             * Use an average of the two channels for mono.
             */
            lbal =
                ((((io.PSG[ch][5] >> 4) * 1.1) *
                  (io.PSG[ch][4] & PSG_DDA_VOICE_VOLUME)) +
                 (((io.PSG[ch][5] & 0x0F) * 1.1) *
                  (io.PSG[ch][4] & PSG_DDA_VOICE_VOLUME))) / 2;
        }

        while ((dwPos < dwSize) && io.psg_da_count[ch]) {
            /*
             * Make our sample data signed (-16..15) and then increment a non-negative
             * result otherwise a sample with a value of 10000b will not be reproduced,
             * which I do not believe is the correct behaviour.  Plus the increment
             * insures matching values on both sides of the wave.
             */
            if ((sample = io.psg_da_data[ch][index] - 16) >= 0)
                sample++;

            /*
             * Left channel, or main channel in mono mode.  Multiply our sample value
             * (-16..16) by our balance (0..511) and then divide by 64 to get a final
             * 8-bit output sample of (-127..127)
             */

//#define MY_SOUND_1(va_) *buf++ = (char) ((int32) (sample * va_) >> 6);
#define MY_SOUND_1(va_) *buf++ = (char) ((int32) (sample * va_) >> 6);
            //
            MY_SOUND_1(lbal)

            if (host.sound.stereo) {
                /*
                 * Same as above but for right channel.
                 */
                MY_SOUND_1(rbal)
                dwPos += 2;
            } else {
                dwPos++;
            }

            da_index[ch] += fixed_inc;
            da_index[ch] &= 0x3FFFFFF;  /* (1023 << 16) + 0xFFFF */
            if ((da_index[ch] >> 16) != index) {
                index = da_index[ch] >> 16;
                io.psg_da_count[ch]--;
            }
        }

        if ((dwPos != dwSize)
            && (io.PSG[ch][PSG_DDA_REG] & PSG_DDA_DIRECT_ACCESS)) {
            memset(buf, 0, (dwSize - dwPos)*host.sound.sample_size);
            return;
        }
    }

    if ((ch > 3) && (io.PSG[ch][7] & 0x80)) {
        uint32 Np = (io.PSG[ch][7] & 0x1F);

        /*
         * PSG Noise generation, for nifty little effects like space ships taking off or blowing up.
         * Only available to PSG channels 5 and 6.
         */
//                      if (ds_nChannels == 2) // STEREO DISABLED
//                      {
//                              lvol = ((io.psg_volume>>3)&0x1E) + (io.PSG[ch][4] & PSG_DDA_VOICE_VOLUME) + ((io.PSG[ch][5]>>3)&0x1E);
//                              lvol = lvol-60;
//                              if (lvol < 0) lvol = 0;
//                              lvol = vol_tbl[lvol];
//                              rvol = ((io.psg_volume<<1)&0x1E) + (io.PSG[ch][4] & PSG_DDA_VOICE_VOLUME) + ((io.PSG[ch][5]<<1)&0x1E);
//                              rvol = rvol-60;
//                              if (rvol < 0) rvol = 0;
//                              rvol = vol_tbl[rvol];
//                              for (dwPos = 0; dwPos < dwSize; dwPos += 2)
//                              {
//                                      k[ch] += 3000+Np*512;
//                                      t = k[ch] / (DWORD) host.sound.freq;
//                                      if (t >= 1)
//                                      {
//                                              r[ch] = mseq(&rand_val[ch]);
//                                              k[ch] -= host.sound.freq * t;
//                                      }
//                                      *buf++ = (WORD)((r[ch] ? 10*702 : -10*702)*lvol/64);
//                                      *buf++ = (WORD)((r[ch] ? 10*702 : -10*702)*rvol/64);
//                              }
//                      }
//                      else  // MONO

        vol =
            MAX((io.psg_volume >> 3) & 0x1E,
                (io.psg_volume << 1) & 0x1E) +
            (io.PSG[ch][4] & PSG_DDA_VOICE_VOLUME) +
            MAX((io.PSG[ch][5] >> 3) & 0x1E, (io.PSG[ch][5] << 1) & 0x1E);
        //average sound level

        if ((vol -= 60) < 0)
            vol = 0;

        vol = vol_tbl[vol];
        // get cooked volume

        while (dwPos < dwSize) {
            k[ch] += 3000 + Np * 512;

            if ((t = (k[ch] / (uint32) host.sound.freq)) >= 1) {
                r[ch] = mseq(&rand_val[ch]);
                k[ch] -= host.sound.freq * t;
            }

            //*buf++ = (signed char) ((r[ch] ? 10 * 702 : -10 * 702) * vol / 256 / 16);   // Level 0
            *buf++ = (signed char) ((r[ch] ? 10 * 702 : -10 * 702) * vol / 256 / 16);   // Level 0

            //sbuf[ch][dum++] = (WORD)((r[ch] ? 10*702 : -10*702)*lvol/64/256);
            //*buf++ = (r[ch] ? 32 : -32) * lvol / 24;
            dwPos++;
        }
    } else
        if ((Tp =
             (io.PSG[ch][PSG_FREQ_LSB_REG] +
              (io.PSG[ch][PSG_FREQ_MSB_REG] << 8))) == 0) {
        /*
         * 12-bit pseudo frequency value stored in PSG registers 2 (all 8 bits) and 3
         * (lower nibble).  If we get to this point and the value is 0 then there's no
         * sound to be played.
         *
         * dwPos will either be 0 as initialized at the beginning of the function or a value
         * left over from the direct audio stuff.  If left over then buf will already be at
         * (buf + dwPos) from the beginning of the function.
         */
        memset(buf, 0, dwSize * host.sound.sample_size);
    } else {
        /*
         * Thank god for well commented code!  The original line of code read:
         * fixed_inc = ((uint32) (3.2 * 1118608 / host.sound.freq) << 16) / Tp;
         * and had nary a comment to be found.  It took a little head scratching to get
         * it figured out.  The 3.2 * 1118608 comes out to 3574595.6 which is obviously
         * meant to represent the 3.58mhz cpu clock speed used in the pc engine to
         * decrement the sound 'frequency'.  I haven't figured out why the original
         * author had the two numbers multiplied together to get the odd value instead of
         * just using 3580000.  I did some checking and the value will compute the same
         * using either value divided by any standard soundcard samplerate.  The
         * host.sound.freq is our soundcard's samplerate which is quite a bit slower than
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
        fixed_inc = ((uint32) (3580000 / host.sound.freq) << 16) / Tp;

        if (host.sound.stereo) {
            /*
             * See the direct audio code above if you're curious why we're multiplying by 1.1
             */
            lbal =
                ((io.PSG[ch][5] >> 4) * 1.1) *
                (io.PSG[ch][4] & PSG_DDA_VOICE_VOLUME);
            rbal =
                ((io.PSG[ch][5] & 0x0F) * 1.1) *
                (io.PSG[ch][4] & PSG_DDA_VOICE_VOLUME);
        } else {
            lbal =
                ((((io.PSG[ch][5] >> 4) * 1.1) *
                  (io.PSG[ch][4] & PSG_DDA_VOICE_VOLUME)) +
                 (((io.PSG[ch][5] & 0x0F) * 1.1) *
                  (io.PSG[ch][4] & PSG_DDA_VOICE_VOLUME))) / 2;
        }

        while (dwPos < dwSize) {
            /*
             * See the direct audio stuff a little above for an explanation of everything
             * within this loop.
             */
            if ((sample =
                 (io.wave[ch][io.PSG[ch][PSG_DATA_INDEX_REG]] - 16)) >= 0)
                sample++;
//#define MY_SOUND_2(va_) *buf++ = (char) ((Sint16) (sample * va_) >> 6);
#define MY_SOUND_2(va_) *buf++ = (char) ((Sint16) (sample * va_) >> 6);
            MY_SOUND_2(lbal)

            if (host.sound.stereo) {
                MY_SOUND_2(rbal)
                dwPos += 2;
            } else {
                dwPos++;
            }

            fixed_n[ch] += fixed_inc;
            fixed_n[ch] &= 0x1FFFFF;    /* (31 << 16) + 0xFFFF */
            io.PSG[ch][PSG_DATA_INDEX_REG] = fixed_n[ch] >> 16;
        }
    }
}
