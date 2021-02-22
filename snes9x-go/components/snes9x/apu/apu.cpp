/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <cmath>
#include "../snes9x.h"
#include "apu.h"
#include "../snapshot.h"
#include "resampler.h"

#include "bapu/snes/snes.hpp"

static const int APU_DEFAULT_INPUT_RATE = 31950; // ~59.94Hz
static const int APU_SAMPLE_BLOCK = 48;
static const int APU_NUMERATOR_NTSC = 15664;
static const int APU_DENOMINATOR_NTSC = 328125;
static const int APU_NUMERATOR_PAL = 34176;
static const int APU_DENOMINATOR_PAL = 709379;

// Max number of samples we'll ever generate before call to port API and
// moving the samples to the resampler.
// This is 535 sample frames, which corresponds to 1 video frame + some leeway
// for use with SoundSync, multiplied by 2, for left and right samples.
static const int MINIMUM_BUFFER_SIZE = 550 * 2;

namespace spc
{
	static apu_callback callback = NULL;
	static void *callback_data = NULL;

	static bool8 sound_in_sync = TRUE;
	static bool8 sound_enabled = FALSE;

	static Resampler *resampler = NULL;

	static int32 reference_time;
	static uint32 remainder;

	static const int timing_hack_numerator = 256;
	static int timing_hack_denominator = 256;
	/* Set these to NTSC for now. Will change to PAL in S9xAPUTimingSetSpeedup
   if necessary on game load. */
	static uint32 ratio_numerator = APU_NUMERATOR_NTSC;
	static uint32 ratio_denominator = APU_DENOMINATOR_NTSC;

	static double dynamic_rate_multiplier = 1.0;
} // namespace spc

bool8 S9xMixSamples(uint8 *dest, int sample_count)
{
#if 0
	int16 *out = (int16 *)dest;

	if (Settings.Mute)
	{
		memset(out, 0, sample_count << 1);
		S9xClearSamples();
	}
	else
	{
		if (spc::resampler->avail() >= sample_count)
		{
			spc::resampler->read((short *)out, sample_count);
		}
		else
		{
			memset(out, 0, sample_count << 1);
			return false;
		}
	}

	if (spc::resampler->space_empty() >= 535 * 2 || !Settings.SoundSync ||
		Settings.TurboMode || Settings.Mute)
		spc::sound_in_sync = true;
	else
		spc::sound_in_sync = false;
#endif

	return true;
}

int S9xGetSampleCount(void)
{
	if (spc::resampler)
		return spc::resampler->avail();
	else
		return -1;
}

void S9xClearSamples(void)
{
	if (spc::resampler)
		spc::resampler->clear();
}

void S9xLandSamples(void)
{
#if 0
	if (spc::callback != NULL)
		spc::callback(spc::callback_data);

	if (spc::resampler->space_empty() >= 535 * 2 || !Settings.SoundSync ||
		Settings.TurboMode || Settings.Mute)
		spc::sound_in_sync = true;
	else
		spc::sound_in_sync = false;
#endif
}

bool8 S9xSyncSound(void)
{
	if (!Settings.SoundSync || spc::sound_in_sync)
		return (TRUE);

	S9xLandSamples();

	return (spc::sound_in_sync);
}

void S9xSetSamplesAvailableCallback(apu_callback callback, void *data)
{
	spc::callback = callback;
	spc::callback_data = data;
}

static void UpdatePlaybackRate(void)
{
#if 0
	if (Settings.SoundInputRate == 0)
		Settings.SoundInputRate = APU_DEFAULT_INPUT_RATE;

	double time_ratio = (double)Settings.SoundInputRate * spc::timing_hack_numerator / (Settings.SoundPlaybackRate * spc::timing_hack_denominator);

	if (Settings.DynamicRateControl)
	{
		time_ratio *= spc::dynamic_rate_multiplier;
	}

	spc::resampler->time_ratio(time_ratio);
#endif
}

void S9xUpdateDynamicRate(int avail, int buffer_size)
{
	spc::dynamic_rate_multiplier = 1.0 + (Settings.DynamicRateLimit * (buffer_size - 2 * avail)) /
											 (double)(1000 * buffer_size);

	UpdatePlaybackRate();
}

bool8 S9xInitSound(int buffer_ms)
{
#if 0
	// The resampler and spc unit use samples (16-bit short) as arguments.
	int buffer_size_samples = MINIMUM_BUFFER_SIZE;
	int requested_buffer_size_samples = Settings.SoundPlaybackRate * buffer_ms * 2 / 1000;

	if (requested_buffer_size_samples > buffer_size_samples)
		buffer_size_samples = requested_buffer_size_samples;

	if (!spc::resampler)
	{
		spc::resampler = new Resampler(buffer_size_samples);
		if (!spc::resampler)
			return (FALSE);
	}
	else
		spc::resampler->resize(buffer_size_samples);

	SNES::dsp.spc_dsp.set_output(spc::resampler);
#endif

	UpdatePlaybackRate();

	return TRUE;
}

void S9xSetSoundControl(uint8 voice_switch)
{
#if 0
	SNES::dsp.spc_dsp.set_stereo_switch(voice_switch << 8 | voice_switch);
#endif
}

void S9xSetSoundMute(bool8 mute)
{
	Settings.Mute = mute || !spc::sound_enabled;
}

void S9xToggleSoundChannel(int c)
{
	uint8 sound_switch = SNES::dsp.spc_dsp.stereo_switch;

	if (c == 8)
		sound_switch = 255;
	else
		sound_switch ^= 1 << c;

	S9xSetSoundControl(sound_switch);
}

bool8 S9xInitAPU(void)
{
	spc::resampler = NULL;

	return (TRUE);
}

void S9xDeinitAPU(void)
{
	if (spc::resampler)
	{
		delete spc::resampler;
		spc::resampler = NULL;
	}
}

void S9xAPUExecute(void)
{
	int cycles = (spc::ratio_numerator * (CPU.Cycles - spc::reference_time) + spc::remainder);
	SNES::smp.execute(cycles / spc::ratio_denominator);
	spc::remainder = (cycles % spc::ratio_denominator);
	spc::reference_time = CPU.Cycles;
}

uint8 S9xAPUReadPort(uint32 port)
{
	S9xAPUExecute();
	return ((uint8)SNES::smp.apuram[0xf4 + (port & 3)]);
}

void S9xAPUWritePort(uint32 port, uint8 byte)
{
	S9xAPUExecute();
	SNES::smp.registers[port & 3] = byte;
}

void S9xAPUSetReferenceTime(int32 cpucycles)
{
	spc::reference_time = cpucycles;
}

void S9xAPUEndScanline(void)
{
	S9xAPUExecute();

#if 0
	SNES::dsp.synchronize();

	if (spc::resampler->space_filled() >= APU_SAMPLE_BLOCK || !spc::sound_in_sync)
		S9xLandSamples();
#endif
}

void S9xAPUTimingSetSpeedup(int ticks)
{
	if (ticks != 0)
		printf("APU speedup hack: %d\n", ticks);

	spc::timing_hack_denominator = 256 - ticks;

	spc::ratio_numerator = Settings.PAL ? APU_NUMERATOR_PAL : APU_NUMERATOR_NTSC;
	spc::ratio_denominator = Settings.PAL ? APU_DENOMINATOR_PAL : APU_DENOMINATOR_NTSC;
	spc::ratio_denominator = spc::ratio_denominator * spc::timing_hack_denominator / spc::timing_hack_numerator;

	UpdatePlaybackRate();
}

void S9xResetAPU(void)
{
	spc::reference_time = 0;
	spc::remainder = 0;

	SNES::smp.power();
	SNES::dsp.power();

	S9xClearSamples();
}

void S9xSoftResetAPU(void)
{
	spc::reference_time = 0;
	spc::remainder = 0;

	SNES::smp.reset();
	SNES::dsp.reset();

	S9xClearSamples();
}

void S9xAPUSaveState(uint8 *block)
{
	uint8 *ptr = block;

	SNES::smp.save_state(&ptr);
	SNES::dsp.save_state(&ptr);

	SNES::set_le32(ptr, spc::reference_time);
	ptr += sizeof(int32);
	SNES::set_le32(ptr, spc::remainder);
	ptr += sizeof(int32);
	SNES::set_le32(ptr, SNES::dsp.clock);
	ptr += sizeof(int32);
	memcpy(ptr, SNES::smp.registers, 4);
	ptr += sizeof(int32);

	memset(ptr, 0, SPC_SAVE_STATE_BLOCK_SIZE - (ptr - block));
}

void S9xAPULoadState(uint8 *block)
{
	uint8 *ptr = block;

	SNES::smp.load_state(&ptr);
	SNES::dsp.load_state(&ptr);

	spc::reference_time = SNES::get_le32(ptr);
	ptr += sizeof(int32);
	spc::remainder = SNES::get_le32(ptr);
	ptr += sizeof(int32);
	SNES::dsp.clock = SNES::get_le32(ptr);
	ptr += sizeof(int32);
	memcpy(SNES::smp.registers, ptr, 4);
}
