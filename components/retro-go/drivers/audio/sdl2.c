#include "rg_system.h"
#include "rg_audio.h"

#if RG_AUDIO_USE_SDL2
#include <SDL2/SDL.h>

static SDL_AudioDeviceID audioDevice;
static int sampleRate;

static bool driver_init(int device, int _sampleRate)
{
    SDL_AudioSpec desired = {
        .freq = sampleRate,
        .format = AUDIO_S16,
        .channels = 2,
    };
    sampleRate = _sampleRate;
    audioDevice = SDL_OpenAudioDevice(NULL, 0, &desired, NULL, 0);
    return audioDevice != 0;
}

static bool driver_deinit(void)
{
    SDL_CloseAudioDevice(audioDevice);
    return true;
}

static bool driver_submit(const rg_audio_frame_t *frames, size_t count)
{
    SDL_QueueAudio(audioDevice, (void *)frames, count * 4);
    SDL_PauseAudioDevice(audioDevice, 0);
    // This is ugly, but we must emulate how it works on the ESP32, where the audio does the pacing!
    static int64_t frame_start = 0;
    int64_t frame_end = frame_start + (uint32_t)(count * (1000000.f / sampleRate)) - 500;
    if (frame_end > rg_system_timer())
        rg_usleep((frame_end - rg_system_timer()));
    frame_start = rg_system_timer();
    return true;
}

static bool driver_set_mute(bool mute)
{
    return true;
}

static bool driver_set_volume(int volume)
{
    return true;
}

static const char *driver_get_error(void)
{
    return SDL_GetError();
}

const rg_audio_driver_t rg_audio_driver_sdl2 = {
    .name = "sdl2",
    .init = driver_init,
    .deinit = driver_deinit,
    .submit = driver_submit,
    .set_mute = driver_set_mute,
    .set_volume = driver_set_volume,
    .set_sample_rate = NULL,
    .get_error = driver_get_error,
};

#endif // RG_AUDIO_USE_SDL2
