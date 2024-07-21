#include "rg_system.h"
#include "rg_audio.h"

static int64_t busyUntil = 0;

static bool driver_init(int device, int sampleRate)
{
    busyUntil = 0;
    return true;
}

static bool driver_deinit(void)
{
    return true;
}

static bool driver_submit(const rg_audio_frame_t *frames, size_t count)
{
    // Wait until the previous submission is done "playing"
    if (busyUntil > rg_system_timer())
        rg_usleep(busyUntil - rg_system_timer());
    busyUntil = rg_system_timer() + (count * (1000000.f / rg_audio_get_sample_rate()));
    return true;
}

const rg_audio_driver_t rg_audio_driver_dummy = {
    .name = "dummy",
    .init = driver_init,
    .deinit = driver_deinit,
    .submit = driver_submit,
};
