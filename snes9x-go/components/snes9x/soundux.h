/* This file is part of Snes9x. See LICENSE file. */

#ifndef USE_BLARGG_APU

#ifndef _SOUND_H_
#define _SOUND_H_

enum
{
   SOUND_SAMPLE = 0,
   SOUND_NOISE,
   SOUND_EXTRA_NOISE,
   SOUND_MUTE
};

enum
{
   SOUND_SILENT, SOUND_ATTACK, SOUND_DECAY, SOUND_SUSTAIN,
   SOUND_RELEASE, SOUND_GAIN, SOUND_INCREASE_LINEAR,
   SOUND_INCREASE_BENT_LINE, SOUND_DECREASE_LINEAR,
   SOUND_DECREASE_EXPONENTIAL
};

enum
{
   MODE_NONE = SOUND_SILENT, MODE_ADSR, MODE_RELEASE = SOUND_RELEASE,
   MODE_GAIN, MODE_INCREASE_LINEAR, MODE_INCREASE_BENT_LINE,
   MODE_DECREASE_LINEAR, MODE_DECREASE_EXPONENTIAL
};

#define MAX_ENVELOPE_HEIGHT 127
#define ENVELOPE_SHIFT 7
#define MAX_VOLUME 127
#define VOLUME_SHIFT 7
#define SOUND_DECODE_LENGTH 16

#define NUM_CHANNELS    8
#define SOUND_BUFFER_SIZE (2 * 33040 / 50)
#define MAX_BUFFER_SIZE SOUND_BUFFER_SIZE

#define SOUND_BUFS      4

typedef struct
{
   int32_t  playback_rate;
   int32_t  noise_gen;
   uint32_t freqbase; /* notaz */
   bool     mute_sound;
} SoundStatus;

extern SoundStatus so;

typedef struct
{
   int32_t  state;
   int32_t  type;
   int16_t  volume_left;
   int16_t  volume_right;
   uint32_t hertz;
   uint32_t frequency;
   uint32_t count;
   bool     loop;
   int32_t  envx;
   int16_t  left_vol_level;
   int16_t  right_vol_level;
   int16_t  envx_target;
   uint32_t env_error;
   uint32_t erate;
   int32_t  direction;
   uint32_t attack_rate;
   uint32_t decay_rate;
   uint32_t sustain_rate;
   uint32_t release_rate;
   uint32_t sustain_level;
   int16_t  sample;
   int16_t  decoded [16];
   int16_t* block;
   uint16_t sample_number;
   bool     last_block;
   bool     needs_decode;
   uint32_t block_pointer;
   uint32_t sample_pointer;
   int32_t* echo_buf_ptr;
   int32_t  mode;
   int32_t  envxx;
   int16_t  next_sample;
   int32_t  interpolate;
   int32_t  previous [2];
   uint8_t  env_ind_attack;
   uint8_t  env_ind_decay;
   uint8_t  env_ind_sustain;
} Channel;

typedef struct
{
   int32_t  echo_enable;
   int32_t  echo_feedback; /* range is -128 .. 127 */
   int32_t  echo_ptr;
   int32_t  echo_buffer_size;
   int32_t  echo_write_enabled;
   int32_t  pitch_mod;
   Channel  channels [NUM_CHANNELS];
   int16_t  master_volume [2]; /* range is -128 .. 127 */
   int16_t  echo_volume   [2]; /* range is -128 .. 127 */
   int32_t  noise_hertz;
} SSoundData;

extern SSoundData SoundData;

void S9xSetSoundVolume(int32_t channel, int16_t volume_left, int16_t volume_right);
void S9xSetSoundFrequency(int32_t channel, int32_t hertz);
void S9xSetSoundHertz(int32_t channel, int32_t hertz);
void S9xSetSoundType(int32_t channel, int32_t type_of_sound);
void S9xSetMasterVolume(int16_t master_volume_left, int16_t master_volume_right);
void S9xSetEchoVolume(int16_t echo_volume_left, int16_t echo_volume_right);
void S9xSetEnvelopeHeight(int32_t channel, int32_t height);
void S9xSetSoundADSR(int32_t channel, int32_t attack, int32_t decay, int32_t sustain, int32_t sustain_level, int32_t release);
void S9xSetSoundKeyOff(int32_t channel);
void S9xSetSoundDecayMode(int32_t channel);
void S9xSetSoundAttachMode(int32_t channel);
void S9xSoundStartEnvelope(Channel*);
void S9xSetEchoFeedback(int32_t echo_feedback);
void S9xSetEchoEnable(uint8_t byte);
void S9xSetEchoDelay(int32_t byte);
void S9xSetEchoWriteEnable(uint8_t byte);
void S9xSetFilterCoefficient(int32_t tap, int32_t value);
void S9xSetFrequencyModulationEnable(uint8_t byte);
void S9xSetEnvelopeRate(int32_t channel, uint32_t rate, int32_t direction, int32_t target, uint32_t  mode);
bool S9xSetSoundMode(int32_t channel, int32_t mode);
void S9xResetSound(bool full);
void S9xFixSoundAfterSnapshotLoad();
void S9xPlaybackSoundSetting(int32_t channel);
void S9xPlaySample(int32_t channel);
void S9xFixEnvelope(int32_t channel, uint8_t gain, uint8_t adsr1, uint8_t adsr2);
void S9xStartSample(int32_t channel);

void S9xMixSamples(int16_t* buffer, int32_t sample_count);
void S9xMixSamplesLowPass(int16_t* buffer, int32_t sample_count, int32_t low_pass_range);
void S9xSetPlaybackRate(uint32_t rate);
#endif
#endif
