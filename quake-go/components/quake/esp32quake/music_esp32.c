/* Loose WAV streaming. Only the internal-stack reader owns the FILE.
 * The mixer consumes buffered PCM and never performs storage operations. */
#include "music_esp32.h"
#include "music_pcm.h"
#include "esp_attr.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "rg_system.h"
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>

#define MUSIC_RING_BYTES (16 * 1024)
#define MUSIC_READ_BYTES 4096

static DRAM_ATTR StaticSemaphore_t mutex_storage;
static DRAM_ATTR SemaphoreHandle_t music_mutex;
static DRAM_ATTR TaskHandle_t volatile reader_task;
// User preference survives Music_Init and is cheap to inspect in the mixer.
static DRAM_ATTR atomic_bool music_on = true;

/* Metadata/buffers use the application's ordinary PSRAM placement policy. */
static struct {
    music_ring_t ring;
    unsigned char *pool;
    char path[RG_PATH_MAX + 1];
    uint32_t generation;
    unsigned track, volume256;
    bool enabled, quit, loop, paused, menu_paused, ready, eof;
    unsigned underruns;
} music;

static void reader(void *arg)
{
    (void)arg;
    music_wav_t wav = {0};
    uint32_t generation = 0;
    char path[RG_PATH_MAX + 1];
    for (;;)
    {
        xSemaphoreTake(music_mutex, portMAX_DELAY);
        bool quit = music.quit;
        bool playback_enabled = Music_IsEnabled();
        bool changed = generation != music.generation;
        unsigned track = music.track;
        bool paused = music.paused || music.menu_paused;
        bool loop = music.loop;
        size_t available = music.ring.capacity - music.ring.used;
        if (changed) {
            generation = music.generation;
            memcpy(path, music.path, sizeof(path));
        }
        xSemaphoreGive(music_mutex);
        if (quit) break;

        if (changed)
        {
            music_wav_close(&wav);
            if (playback_enabled && track >= 2)
            {
                if (music_wav_open(&wav, path))
                    RG_LOGI("Music: playing track%02u (%s)", track, path);
                else
                    RG_LOGW("Music: unavailable or unsupported WAV: %s", path);
            }
            xSemaphoreTake(music_mutex, portMAX_DELAY);
            if (generation == music.generation) music.eof = wav.file == NULL;
            xSemaphoreGive(music_mutex);
        }

        if (!playback_enabled) {
            // Commands and shutdown wake us; Off must not poll or stream.
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            continue;
        }

        if (wav.file && !paused && available >= MUSIC_FRAME_BYTES)
        {
            size_t bytes = available < MUSIC_READ_BYTES ? available : MUSIC_READ_BYTES;
            unsigned char *staging = music.pool + MUSIC_RING_BYTES;
            size_t got = music_wav_read(&wav, staging, bytes, loop);
            bool ended = wav.failed || (!loop && !wav.remaining);
            xSemaphoreTake(music_mutex, portMAX_DELAY);
            // A stop/change may arrive during I/O: never publish stale samples.
            if (generation == music.generation && !music.quit)
            {
                music_ring_write(&music.ring, staging, got);
                if (music.ring.used >= MUSIC_READ_BYTES || ended) music.ready = true;
                music.eof = ended;
            }
            xSemaphoreGive(music_mutex);
            if (wav.failed) RG_LOGW("Music: read failed, stopping track%02u", track);
            if (ended) music_wav_close(&wav);
            // Fill ahead in SD-sized chunks, checking commands between reads.
            if (got) continue;
        }
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(10));
    }
    music_wav_close(&wav);
    unsigned free_stack = (unsigned)(uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));
    RG_LOGI("Music: reader stopped, minimum free stack=%u bytes", free_stack);
    reader_task = NULL;
    vTaskDelete(NULL);
}

bool Music_Init(void)
{
    if (!music_mutex) music_mutex = xSemaphoreCreateMutexStatic(&mutex_storage);
    if (!music_mutex) return false;
    xSemaphoreTake(music_mutex, portMAX_DELAY);
    if (music.enabled) {
        xSemaphoreGive(music_mutex);
        return true;
    }
    memset(&music, 0, sizeof(music));
    // No internal-memory fallback or panic if optional music cannot fit.
    music.pool = heap_caps_malloc(MUSIC_RING_BYTES + MUSIC_READ_BYTES,
                                 MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    music_ring_init(&music.ring, music.pool, music.pool ? MUSIC_RING_BYTES : 0);
    music.volume256 = 256;
    xSemaphoreGive(music_mutex);
    if (!music.pool) {
        RG_LOGW("Music disabled: cannot allocate 20 KiB PSRAM buffer");
        return false;
    }

    TaskHandle_t task = NULL;
    // IDF allocates normal task stacks internally. Plain stdio is safe here;
    // the Quake owner-only FATFS proxy remains unchanged.
    if (xTaskCreatePinnedToCore(reader, "quake_music", 4096, NULL,
                               RG_TASK_PRIORITY_5, &task, tskNO_AFFINITY) != pdPASS)
    {
        xSemaphoreTake(music_mutex, portMAX_DELAY);
        free(music.pool);
        music.pool = NULL;
        music_ring_init(&music.ring, NULL, 0);
        xSemaphoreGive(music_mutex);
        RG_LOGW("Music disabled: cannot create reader task");
        return false;
    }
    reader_task = task;
    xSemaphoreTake(music_mutex, portMAX_DELAY);
    music.enabled = true;
    xSemaphoreGive(music_mutex);
    RG_LOGI("Music: WAV streaming enabled, %u Hz stereo PCM16, 16 KiB ring", MUSIC_SAMPLE_RATE);
    return true;
}

bool Music_IsEnabled(void)
{
    return atomic_load_explicit(&music_on, memory_order_relaxed);
}

void Music_SetEnabled(bool enabled)
{
    if (music_mutex) xSemaphoreTake(music_mutex, portMAX_DELAY);
    bool changed = Music_IsEnabled() != enabled;
    atomic_store_explicit(&music_on, enabled, memory_order_relaxed);
    if (changed && music_mutex) {
        // Preserve the requested track and pause flags. In-flight reads are
        // discarded; the reader closes/reopens its FILE on this generation.
        music.generation++;
        music.ready = music.eof = false;
        music_ring_init(&music.ring, music.pool, music.pool ? MUSIC_RING_BYTES : 0);
    }
    if (music_mutex) xSemaphoreGive(music_mutex);
    if (changed) {
        if (reader_task) xTaskNotifyGive(reader_task);
        RG_LOGI("Music: %s", enabled ? "On" : "Off");
    }
}

void Music_Play(const char *path, unsigned track, bool loop)
{
    if (!music_mutex) return;
    xSemaphoreTake(music_mutex, portMAX_DELAY);
    if (music.enabled && (track != music.track || loop != music.loop ||
                          (music.eof && !music.ring.used) ||
                          strcmp(path, music.path)))
    {
        snprintf(music.path, sizeof(music.path), "%s", path);
        music.track = track;
        music.loop = loop;
        music.paused = false;
        music.ready = music.eof = false;
        music.generation++;
        music_ring_init(&music.ring, music.pool, MUSIC_RING_BYTES);
    }
    xSemaphoreGive(music_mutex);
    if (reader_task) xTaskNotifyGive(reader_task);
}

void Music_Stop(void)
{
    if (!music_mutex) return;
    xSemaphoreTake(music_mutex, portMAX_DELAY);
    music.track = 0;
    music.path[0] = 0;
    music.generation++;
    music.ready = false;
    music.paused = false;
    music_ring_init(&music.ring, music.pool, music.pool ? MUSIC_RING_BYTES : 0);
    xSemaphoreGive(music_mutex);
    if (reader_task) xTaskNotifyGive(reader_task);
}

void Music_Pause(bool paused)
{
    if (!music_mutex) return;
    xSemaphoreTake(music_mutex, portMAX_DELAY);
    music.paused = paused;
    xSemaphoreGive(music_mutex);
}

void Music_MenuPause(bool paused)
{
    if (!music_mutex) return;
    xSemaphoreTake(music_mutex, portMAX_DELAY);
    music.menu_paused = paused;
    xSemaphoreGive(music_mutex);
}

void Music_SetVolume(float volume)
{
    if (!music_mutex) return;
    // Also handles NaN without converting it to an integer.
    unsigned gain = !(volume > 0) ? 0 : (volume >= 1 ? 256 : (unsigned)(volume * 256));
    xSemaphoreTake(music_mutex, portMAX_DELAY);
    music.volume256 = gain;
    xSemaphoreGive(music_mutex);
}

void Music_Mix(rg_audio_frame_t *frames, size_t count)
{
    // Every holder performs only bounded in-memory work. Wait for that short
    // section rather than dropping an audible block on a refill collision.
    if (!Music_IsEnabled() || !music_mutex ||
        xSemaphoreTake(music_mutex, portMAX_DELAY) != pdTRUE) return;
    if (Music_IsEnabled() && music.enabled && music.ready && !music.paused && !music.menu_paused)
    {
        unsigned rate = rg_audio_get_sample_rate();
        bool starved = false;
        for (size_t i = 0; i < count; i++)
        {
            int left, right;
            if (!music_ring_next(&music.ring, rate, &left, &right)) starved = true;
            frames[i].left = music_mix_sample(frames[i].left, left, music.volume256);
            frames[i].right = music_mix_sample(frames[i].right, right, music.volume256);
        }
        if (starved && !music.eof) {
            music.underruns++;
            music.ready = false; // Refill ahead before resuming after slow I/O.
        }
    }
    xSemaphoreGive(music_mutex);
}

void Music_Shutdown(void)
{
    if (!music_mutex) return;
    xSemaphoreTake(music_mutex, portMAX_DELAY);
    music.enabled = false;
    music.quit = true;
    xSemaphoreGive(music_mutex);
    if (reader_task) xTaskNotifyGive(reader_task);
    while (reader_task) vTaskDelay(1);
    xSemaphoreTake(music_mutex, portMAX_DELAY);
    unsigned underruns = music.underruns;
    free(music.pool);
    music.pool = NULL;
    music_ring_init(&music.ring, NULL, 0);
    xSemaphoreGive(music_mutex);
    // Retain the static mutex: the audio task may still call Music_Mix until
    // S_Shutdown joins it. Disabled/null-ring state makes those calls harmless.
    RG_LOGI("Music: shutdown, %u buffer underruns", underruns);
}
