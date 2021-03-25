/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <cmath>
#include "../snes9x.h"
#include "apu.h"
#include "../snapshot.h"

namespace SNES
{
	#include "smp.hpp"
} // namespace SNES

static const int APU_NUMERATOR_NTSC = 15664;
static const int APU_DENOMINATOR_NTSC = 328125;
static const int APU_NUMERATOR_PAL = 34176;
static const int APU_DENOMINATOR_PAL = 709379;

namespace spc
{
	static apu_callback callback = NULL;
	static void *callback_data = NULL;

	static bool8 sound_in_sync = TRUE;
	static bool8 sound_enabled = FALSE;

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

extern "C" {

bool8 S9xMixSamples(uint8 *dest, int sample_count)
{
	return true;
}

int S9xGetSampleCount(void)
{
	return -1;
}

void S9xClearSamples(void)
{

}

void S9xLandSamples(void)
{

}

bool8 S9xSoundSync(void)
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

}

void S9xUpdateDynamicRate(int avail, int buffer_size)
{
	spc::dynamic_rate_multiplier = 1.0 + (Settings.DynamicRateLimit * (buffer_size - 2 * avail)) /
											 (double)(1000 * buffer_size);

	UpdatePlaybackRate();
}

bool8 S9xSoundInit(int buffer_ms)
{
	if (!S9xInitAPU())
		return FALSE;

	UpdatePlaybackRate();

	return TRUE;
}

void S9xSetSoundControl(uint8 voice_switch)
{

}

void S9xSetSoundMute(bool8 mute)
{
	Settings.Mute = mute || !spc::sound_enabled;
}

void S9xToggleSoundChannel(int c)
{
	uint8 sound_switch = 0;

	if (c == 8)
		sound_switch = 255;
	else
		sound_switch ^= 1 << c;

	S9xSetSoundControl(sound_switch);
}

bool8 S9xInitAPU(void)
{
	return (TRUE);
}

void S9xDeinitAPU(void)
{

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
}

void S9xAPUTimingSetSpeedup(int ticks)
{
	if (ticks != 0)
		printf("APU speedup hack: %d\n", ticks);

	spc::timing_hack_denominator = 256 - ticks;

	spc::ratio_numerator = (Settings.Region == S9X_PAL) ? APU_NUMERATOR_PAL : APU_NUMERATOR_NTSC;
	spc::ratio_denominator = (Settings.Region == S9X_PAL) ? APU_DENOMINATOR_PAL : APU_DENOMINATOR_NTSC;
	spc::ratio_denominator = spc::ratio_denominator * spc::timing_hack_denominator / spc::timing_hack_numerator;

	UpdatePlaybackRate();
}

void S9xResetAPU(void)
{
	spc::reference_time = 0;
	spc::remainder = 0;

	SNES::smp.power();

	S9xClearSamples();
}

void S9xSoftResetAPU(void)
{
	spc::reference_time = 0;
	spc::remainder = 0;

	SNES::smp.reset();

	S9xClearSamples();
}

void S9xAPUSaveState(uint8 *block)
{
	uint8 *ptr = block;

	SNES::smp.save_state(&ptr);
	// SNES::dsp.save_state(&ptr);

	SET_LE32(ptr, spc::reference_time);
	ptr += sizeof(int32);
	SET_LE32(ptr, spc::remainder);
	ptr += sizeof(int32);
	// SET_LE32(ptr, SNES::dsp.clock);
	ptr += sizeof(int32);
	memcpy(ptr, SNES::smp.registers, 4);
	ptr += sizeof(int32);

	memset(ptr, 0, SPC_SAVE_STATE_BLOCK_SIZE - (ptr - block));
}

void S9xAPULoadState(uint8 *block)
{
	uint8 *ptr = block;

	SNES::smp.load_state(&ptr);
	// SNES::dsp.load_state(&ptr);

	spc::reference_time = GET_LE32(ptr);
	ptr += sizeof(int32);
	spc::remainder = GET_LE32(ptr);
	ptr += sizeof(int32);
	// SNES::dsp.clock = GET_LE32(ptr);
	ptr += sizeof(int32);
	memcpy(SNES::smp.registers, ptr, 4);
}

}
