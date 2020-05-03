#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <odroid_system.h>
#include "osd.h"

#define AUDIO_SAMPLE_RATE 22050
#define AUDIO_CHANNELS 6
#define AUDIO_SAMPLES_PER_FRAME (AUDIO_SAMPLE_RATE / 60 + 1)


static void
audioTask(void *arg)
{
    printf("%s: STARTED\n", __func__);

    short sbuf_mix[AUDIO_SAMPLES_PER_FRAME * 2];
    char sbuf[AUDIO_CHANNELS][AUDIO_SAMPLES_PER_FRAME * 2];

    int lvol, rvol, lval, rval, x;

    while (1)
    {
        short *p = sbuf_mix;

        for (int i = 0; i < AUDIO_CHANNELS; i++)
        {
            psg_update(sbuf[i], i, AUDIO_SAMPLES_PER_FRAME);
        }

        lvol = (io.psg_volume >> 4);
        rvol = (io.psg_volume & 0x0F);

        for (int i = 0; i < AUDIO_SAMPLES_PER_FRAME * 2; i += 2)
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

        odroid_audio_submit(sbuf_mix, AUDIO_SAMPLES_PER_FRAME / 2);
    }

    vTaskDelete(NULL);
}

void osd_snd_init()
{
    host.sound.stereo = true;
    host.sound.freq = AUDIO_SAMPLE_RATE;
    host.sound.sample_size = 1;

    xTaskCreatePinnedToCore(&audioTask, "audioTask", 1024 * 8, NULL, 5, NULL, 1);
}

void osd_snd_shutdown(void)
{
    odroid_audio_stop();
}
