#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <odroid_system.h>
#include "osd.h"

#define AUDIO_SAMPLE_RATE 22050
#define AUDIO_BUFFER_SIZE 2048
#define AUDIO_CHANNELS 6

static short *sbuf_mix;
static char *sbuf[AUDIO_CHANNELS];

QueueHandle_t audioSyncQueue;


static void
audioTask(void *arg)
{
    printf("%s: STARTED\n", __func__);

    audioSyncQueue = xQueueCreate(1, sizeof(void*));

    int samples_per_frame = AUDIO_SAMPLE_RATE / 60 + 1;

    int lvol, rvol, lval, rval, x;

    while (1)
    {
        short *p = sbuf_mix;

        for (int i = 0; i < AUDIO_CHANNELS; i++)
        {
            WriteBuffer(sbuf[i], i, samples_per_frame);
        }

        lvol = (io.psg_volume >> 4);
        rvol = (io.psg_volume & 0x0F);

        for (int i = 0; i < samples_per_frame * 2; i += 2)
        {
            lval = rval = 0;

            for (int j = 0; j < AUDIO_CHANNELS; j++)
            {
                lval += sbuf[j][i];
                rval += sbuf[j][i + 1];
            }

            *p++ = (short)(lval * lvol);
            *p++ = (short)(rval * rvol);
        }

        odroid_audio_submit(sbuf_mix, samples_per_frame / 2);
    }

    vTaskDelete(NULL);
}

void osd_snd_set_volume(uchar v)
{
}

int osd_snd_init_sound()
{
    host.sound.stereo = true;
    host.sound.freq = AUDIO_SAMPLE_RATE;
    host.sound.sample_size = 1;

    for (int i = 0; i < AUDIO_CHANNELS; i++)
    {
        sbuf[i] = rg_alloc(AUDIO_BUFFER_SIZE / 2, MEM_SLOW);
    }
    sbuf_mix = rg_alloc(AUDIO_BUFFER_SIZE, MEM_SLOW);

    xTaskCreatePinnedToCore(&audioTask, "audioTask", 1024 * 4, NULL, 5, NULL, 1);
}

void osd_snd_trash_sound(void)
{
    odroid_audio_stop();
}
