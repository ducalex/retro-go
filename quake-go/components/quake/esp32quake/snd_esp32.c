/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "quakedef.h"

#include "esp_attr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "rg_system.h"
#include "music_esp32.h"

// Forward declarations from common.c
byte *COM_LoadFile(char *path, int usehunk);

#define MAX_SFX 256

// Generation counter to invalidate channels across level changes
static int sound_generation = 0;

// SFX pool metadata. For original ESP32, keep in internal DRAM to save PSRAM address space.
#if defined(CONFIG_IDF_TARGET_ESP32)
static DRAM_ATTR sfx_t known_sfx[MAX_SFX];
#else
static EXT_RAM_BSS_ATTR sfx_t known_sfx[MAX_SFX];
#endif
// Channel state is touched by both the game and every mixer iteration.
DRAM_ATTR channel_t channels[MAX_CHANNELS];

static int num_sfx = 0;
static int permanent_sfx_count = -1;
static sfx_t *ambient_sfx[NUM_AMBIENTS];
static bool snd_ambient = 1;
static SemaphoreHandle_t sound_mutex = NULL;
static volatile TaskHandle_t audio_task_handle = NULL;

// sound.h visible stuff
cvar_t bgmvolume = {"bgmvolume", "0.7", true};
cvar_t volume = {"volume", "0.7", true};
cvar_t nosound = {"nosound", "0"};
cvar_t ambient_level = {"ambient_level", "0.3"};
cvar_t ambient_fade = {"ambient_fade", "100"};

int total_channels;

qboolean snd_initialized = false;

vec3_t listener_origin;
vec3_t listener_forward;
vec3_t listener_right;
vec3_t listener_up;
vec_t sound_nominal_clip_dist = 1000.0;

int paintedtime; // MUST be int
static int64_t hardware_sample_offset = 0;

// Keep mixer latency and the time spent holding sound_mutex bounded. At the
// 22,050 Hz output rate, 256 frames represent about 12 ms of audio.
#define AUDIO_BUFFER_SAMPLES 256

// Spatialization can double an 8-bit channel volume. Even with all 32 channels
// at that maximum, the worst-case sum is below 536 million and fits int32_t.
static DRAM_ATTR int32_t mix_buffer[AUDIO_BUFFER_SAMPLES * 2];
static DRAM_ATTR rg_audio_frame_t output_buffer[AUDIO_BUFFER_SAMPLES];

// forward declarations
static sfx_t *FindSfxName(char *name);

// LoadSound: loads a WAV file into Hunk memory
static bool LoadSound(sfx_t *s)
{
    char namebuffer[256];

    // load it in
    sprintf(namebuffer, "sound/%s", s->name);

    // Load sound into the engine hunk. 
    // We don't use mmap because fatfs doesn't support it directly.
    byte *data = COM_LoadFile(namebuffer, 1);
    
#if defined(CONFIG_IDF_TARGET_ESP32)
    // Feed watchdog during heavy loading
    vTaskDelay(1);
#endif

    if (!data)
    {
        return false;
    }

    wavinfo_t info = GetWavinfo(s->name, (byte *)data, com_filesize);

    if (info.channels != 1)
    {
        Con_Printf("%s is a stereo sample\n", s->name);
        return false;
    }

    assert(info.rate > 0);
    assert(info.width == 1 || info.width == 2);
    assert(info.loopstart >= -1);
    assert(info.samples > 0);

    s->cache.data = data + info.dataofs;
    s->cache.sampleRate = info.rate;
    s->cache.sampleWidth = info.width;
    s->cache.loopStart = info.loopstart;
    s->cache.sampleCount = info.samples;

    int outputRate = rg_audio_get_sample_rate();
    if (outputRate <= 0) outputRate = 22050; // default

    s->cache.stepFixedPoint = (uint32_t)((float)info.rate * ESP32_SOUND_STEP / outputRate);
    s->cache.effectiveLength = (s->cache.sampleCount * (int64_t)ESP32_SOUND_STEP) / s->cache.stepFixedPoint;

    return true;
}

static sfx_t *FindSfxName(char *name)
{
    if (name == NULL) return NULL;
    if (strlen(name) >= MAX_QPATH) return NULL;

    int i;
    for (i = 0; i < num_sfx; ++i)
        if (!strcmp(known_sfx[i].name, name))
            return &known_sfx[i];

    if (num_sfx == MAX_SFX) return NULL;

    sfx_t *sfx = &known_sfx[num_sfx++];
    memset(sfx, 0, sizeof(*sfx));
    strcpy(sfx->name, name);
    return sfx;
}

static void UpdateAmbientSounds(void)
{
    mleaf_t *l;
    float vol;
    int ambient_channel;
    channel_t *chan;

    if (!snd_initialized || !snd_ambient || !cl.worldmodel) return;

    l = Mod_PointInLeaf(listener_origin, cl.worldmodel);
    if (!l || !ambient_level.value)
    {
        for (ambient_channel = 0; ambient_channel < NUM_AMBIENTS; ambient_channel++)
            channels[ambient_channel].sfx = NULL;
        return;
    }

    for (ambient_channel = 0; ambient_channel < NUM_AMBIENTS; ambient_channel++)
    {
        chan = &channels[ambient_channel];
        chan->sfx = ambient_sfx[ambient_channel];

        vol = ambient_level.value * l->ambient_sound_level[ambient_channel];
        if (vol < 8) vol = 0;

        if (chan->master_vol < vol)
        {
            chan->master_vol += host_frametime * ambient_fade.value;
            if (chan->master_vol > vol) chan->master_vol = vol;
        }
        else if (chan->master_vol > vol)
        {
            chan->master_vol -= host_frametime * ambient_fade.value;
            if (chan->master_vol < vol) chan->master_vol = vol;
        }

        chan->leftvol = chan->rightvol = (int)chan->master_vol;
    }
}

channel_t *SND_PickChannel(int entnum, int entchannel)
{
    int ch_idx;
    int first_to_die = -1;
    int life_left = 0x7fffffff;

    for (ch_idx = NUM_AMBIENTS; ch_idx < NUM_AMBIENTS + MAX_DYNAMIC_CHANNELS; ++ch_idx)
    {
        if (entchannel != 0 && channels[ch_idx].entnum == entnum && (channels[ch_idx].entchannel == entchannel || entchannel == -1))
        {
            first_to_die = ch_idx;
            break;
        }
        if (channels[ch_idx].entnum == cl.viewentity && entnum != cl.viewentity && channels[ch_idx].sfx)
            continue;
        if (channels[ch_idx].end - paintedtime < life_left)
        {
            life_left = channels[ch_idx].end - paintedtime;
            first_to_die = ch_idx;
        }
    }

    if (first_to_die == -1) return NULL;
    channels[first_to_die].sfx = NULL;
    channels[first_to_die].generation = sound_generation;
    return &channels[first_to_die];
}

void SND_Spatialize(channel_t *ch)
{
    vec_t dot, dist, lscale, rscale, scale;
    vec3_t source_vec;

    if (ch->entnum == cl.viewentity)
    {
        ch->leftvol = ch->master_vol;
        ch->rightvol = ch->master_vol;
        return;
    }

    VectorSubtract(ch->origin, listener_origin, source_vec);
    dist = VectorNormalize(source_vec) * ch->dist_mult;
    dot = DotProduct(listener_right, source_vec);

    rscale = 1.0 + dot;
    lscale = 1.0 - dot;

    scale = (1.0 - dist) * rscale;
    ch->rightvol = (int)(ch->master_vol * scale);
    if (ch->rightvol < 0) ch->rightvol = 0;

    scale = (1.0 - dist) * lscale;
    ch->leftvol = (int)(ch->master_vol * scale);
    if (ch->leftvol < 0) ch->leftvol = 0;
}

static void audio_task(void *arg)
{
    (void)arg;
    while (snd_initialized)
    {
        if (!sound_mutex || xSemaphoreTake(sound_mutex, pdMS_TO_TICKS(100)) != pdTRUE)
            continue;

        memset(mix_buffer, 0, sizeof(mix_buffer));
        int volumeInt = (int)(volume.value * 256);
        int current_gen = sound_generation;

        for (int i = 0; i < total_channels; ++i)
        {
            channel_t *chan = &channels[i];
            
            // Immediately stop if this channel is from an old level generation
            if (chan->generation != current_gen) {
                chan->sfx = NULL;
                continue;
            }

            if (!chan->sfx || (!chan->leftvol && !chan->rightvol)) continue;

            sfxcache_t *cache = &chan->sfx->cache;
            if (!cache->data) continue;

            int pos = chan->pos;
            int length = cache->sampleCount * ESP32_SOUND_STEP;
            uint32_t step = cache->stepFixedPoint;
            int width = cache->sampleWidth;
            uint8_t *data = (uint8_t *)cache->data;
            int lvol = chan->leftvol;
            int rvol = chan->rightvol;

            for (int j = 0; j < AUDIO_BUFFER_SAMPLES; ++j)
            {
                if (pos >= length)
                {
                    if (cache->loopStart < 0) {
                        chan->sfx = NULL;
                        break;
                    }
                    pos = (cache->loopStart * ESP32_SOUND_STEP) + (pos % length);
                }

                int32_t sample;
                uint8_t *p = data + (pos >> 12) * width;
                if (width == 1)
                    sample = ((int32_t)*p - 128) << 8;
                else
                    sample = (int16_t)(p[0] | (p[1] << 8));

                mix_buffer[2 * j] += sample * lvol;
                mix_buffer[2 * j + 1] += sample * rvol;
                pos += step;
            }
            chan->pos = pos;
        }

        for (int i = 0; i < AUDIO_BUFFER_SAMPLES; ++i)
        {
            // Scale by 1/16 (>> 20) to prevent clipping with multiple sounds
            int32_t l = (int32_t)(((int64_t)mix_buffer[2 * i] * volumeInt) >> 20);
            int32_t r = (int32_t)(((int64_t)mix_buffer[2 * i + 1] * volumeInt) >> 20);
            
            if (l > 32767) l = 32767; else if (l < -32768) l = -32768;
            if (r > 32767) r = 32767; else if (r < -32768) r = -32768;
            output_buffer[i].left = (int16_t)l;
            output_buffer[i].right = (int16_t)r;
        }

        paintedtime += AUDIO_BUFFER_SAMPLES;
        xSemaphoreGive(sound_mutex);

        // Music has its own short buffer lock; never hold the effects lock
        // across music consumption, and never perform SD reads in this task.
        Music_Mix(output_buffer, AUDIO_BUFFER_SAMPLES);

        // The Retro-Go audio backend blocks as needed to pace the producer.
        rg_audio_submit(output_buffer, AUDIO_BUFFER_SAMPLES);
    }
    audio_task_handle = NULL;
    vTaskDelete(NULL);
}

void S_Init(void)
{
    Cvar_RegisterVariable(&bgmvolume);
    Cvar_RegisterVariable(&volume);
    Cvar_RegisterVariable(&nosound);
    Cvar_RegisterVariable(&ambient_level);
    Cvar_RegisterVariable(&ambient_fade);

    if (!sound_mutex) sound_mutex = xSemaphoreCreateMutex();

    total_channels = MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS;
    memset(channels, 0, sizeof(channels));
    memset(known_sfx, 0, sizeof(known_sfx));
    num_sfx = 0;
    permanent_sfx_count = -1;
    sound_generation = 1;

    snd_initialized = true;
    paintedtime = 0;
    hardware_sample_offset = rg_audio_get_counters().totalSamples;

#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32P4)
    int audio_core = 1;
#else
    int audio_core = tskNO_AFFINITY;
#endif

    TaskHandle_t task = NULL;
    if (xTaskCreatePinnedToCore(audio_task, "audio_task", 4096, NULL,
                                RG_TASK_PRIORITY_8, &task, audio_core) != pdPASS)
    {
        snd_initialized = false;
        vSemaphoreDelete(sound_mutex);
        sound_mutex = NULL;
        RG_LOGE("Failed to start Quake audio task");
        return;
    }
    audio_task_handle = task;
}

void S_AmbientOff(void) { snd_ambient = false; }
void S_AmbientOn(void) { snd_ambient = true; }

void S_Shutdown(void)
{
    if (!snd_initialized && audio_task_handle == NULL) {
        if (sound_mutex) {
            vSemaphoreDelete(sound_mutex);
            sound_mutex = NULL;
        }
        return;
    }
    snd_initialized = false;

    // rg_audio_submit() may still own the task briefly. Join it exactly before
    // Host_Shutdown releases sound state or Retro-Go deinitializes audio.
    while (audio_task_handle != NULL) {
        rg_task_yield();
    }

    if (sound_mutex) {
        vSemaphoreDelete(sound_mutex);
        sound_mutex = NULL;
    }
}

void S_StartSound(int entnum, int entchannel, sfx_t *sfx, vec3_t origin, float fvol, float attenuation)
{
    if (!snd_initialized || nosound.value || !sfx || !sound_mutex) return;

    if (xSemaphoreTake(sound_mutex, pdMS_TO_TICKS(10)) != pdTRUE) return;

    channel_t *target_chan = SND_PickChannel(entnum, entchannel);
    if (!target_chan)
    {
        xSemaphoreGive(sound_mutex);
        return;
    }

    memset(target_chan, 0, sizeof(*target_chan));
    VectorCopy(origin, target_chan->origin);
    target_chan->dist_mult = attenuation / sound_nominal_clip_dist;
    target_chan->master_vol = (int)(fvol * 255);
    target_chan->entnum = entnum;
    target_chan->entchannel = entchannel;
    target_chan->generation = sound_generation;
    SND_Spatialize(target_chan);

    if (target_chan->leftvol || target_chan->rightvol)
    {
        target_chan->sfx = sfx;
        target_chan->pos = 0;
        target_chan->end = paintedtime + sfx->cache.effectiveLength;
    }

    xSemaphoreGive(sound_mutex);
}

void S_StaticSound(sfx_t *sfx, vec3_t origin, float vol, float attenuation)
{
    if (!snd_initialized || !sfx || !sound_mutex || total_channels == MAX_CHANNELS || sfx->cache.loopStart == -1) return;

    if (xSemaphoreTake(sound_mutex, pdMS_TO_TICKS(10)) != pdTRUE) return;

    channel_t *ss = &channels[total_channels++];
    memset(ss, 0, sizeof(*ss));
    ss->sfx = sfx;
    VectorCopy(origin, ss->origin);
    ss->master_vol = (int)vol;
    ss->dist_mult = (attenuation / 64) / sound_nominal_clip_dist;
    ss->end = paintedtime + sfx->cache.effectiveLength;
    ss->generation = sound_generation;
    SND_Spatialize(ss);

    xSemaphoreGive(sound_mutex);
}

void S_LocalSound(char *name)
{
    sfx_t *sfx = S_PrecacheSound(name);
    if (sfx) S_StartSound(cl.viewentity, -1, sfx, vec3_origin, 1, 1);
}

void S_StopSound(int entnum, int entchannel)
{
    if (!snd_initialized || !sound_mutex) return;
    if (xSemaphoreTake(sound_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        for (int i = 0; i < MAX_CHANNELS; ++i)
            if (channels[i].entnum == entnum && channels[i].entchannel == entchannel)
                channels[i].sfx = NULL;
        xSemaphoreGive(sound_mutex);
    }
}

sfx_t *S_PrecacheSound(char *name)
{
    sfx_t *sfx = FindSfxName(name);
    if (!sfx || (sfx->cache.data == NULL && !LoadSound(sfx))) return NULL;
    return sfx;
}

// The first server-info pass only touches names. Loading here would read and
// allocate every map sound a second time before the real precache pass.
void S_TouchSound(char *name) { FindSfxName(name); }

void S_Update(vec3_t origin, vec3_t forward, vec3_t right, vec3_t up)
{
    if (!snd_initialized || !sound_mutex) return;

    VectorCopy(origin, listener_origin);
    VectorCopy(forward, listener_forward);
    VectorCopy(right, listener_right);
    VectorCopy(up, listener_up);

    int soundtime_now = (int)(rg_audio_get_counters().totalSamples - hardware_sample_offset);

    if (xSemaphoreTake(sound_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        UpdateAmbientSounds();
        for (int i = NUM_AMBIENTS; i < total_channels; ++i)
        {
            if (!channels[i].sfx) continue;
            if (channels[i].end <= soundtime_now) {
                channels[i].sfx = NULL;
                continue;
            }
            SND_Spatialize(&channels[i]);
        }
        xSemaphoreGive(sound_mutex);
    }
}

void S_StopAllSounds(qboolean clear)
{
    if (!snd_initialized || !sound_mutex) return;
    if (xSemaphoreTake(sound_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        total_channels = MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS;
        memset(channels, 0, MAX_CHANNELS * sizeof(channel_t));
        sound_generation++; // Increment generation to invalidate audio task's current work
        paintedtime = 0;
        hardware_sample_offset = rg_audio_get_counters().totalSamples;
        xSemaphoreGive(sound_mutex);
    }
}

void S_ClearBuffer(void) {}

void S_BeginPrecaching(void) 
{
    if (xSemaphoreTake(sound_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Sounds loaded during engine initialization live below the host hunk
        // mark and remain valid across maps. Touched map entries follow them
        // and have no data yet on the first pass.
        if (permanent_sfx_count < 0) {
            permanent_sfx_count = 0;
            while (permanent_sfx_count < num_sfx &&
                   known_sfx[permanent_sfx_count].cache.data != NULL) {
                permanent_sfx_count++;
            }
        }

        num_sfx = permanent_sfx_count;
        memset(&known_sfx[num_sfx], 0,
               (MAX_SFX - num_sfx) * sizeof(known_sfx[0]));
        sound_generation++;
        xSemaphoreGive(sound_mutex);
    }
}

void S_EndPrecaching(void) {}
void S_ClearPrecache(void) {}
void S_ExtraUpdate(void) {}

void S_PrecacheAmbients(void)
{
    // Ambients will be loaded by the engine via S_PrecacheSound during loading
    ambient_sfx[AMBIENT_WATER] = S_PrecacheSound("ambience/water1.wav");
    ambient_sfx[AMBIENT_SKY] = S_PrecacheSound("ambience/wind2.wav");
}
