#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <odroid_system.h>
#include <osd.h>

#define AUDIO_SAMPLE_RATE   (22050)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 60 + 1)
#define AUDIO_CHANNELS 6

static short audioBuffer[AUDIO_BUFFER_LENGTH * 2];
static char  chanBuffer[AUDIO_CHANNELS][AUDIO_BUFFER_LENGTH * 2];


static void
audioTask(void *arg)
{
    printf("%s: STARTED\n", __func__);

    int lvol, rvol, lval, rval;

    while (1)
    {
        short *p = (short *)&audioBuffer;

        for (int i = 0; i < AUDIO_CHANNELS; i++)
        {
            psg_update(chanBuffer[i], i, AUDIO_BUFFER_LENGTH * 2);
        }

        lvol = (io.psg_volume >> 4);
        rvol = (io.psg_volume & 0x0F);

        for (int i = 0; i < AUDIO_BUFFER_LENGTH * 2; i += 2)
        {
            lval = rval = 0;

            for (int j = 0; j < AUDIO_CHANNELS; j++)
            {
                lval += chanBuffer[j][i];
                rval += chanBuffer[j][i + 1];
            }

            *p++ = (short)(lval * lvol);
            *p++ = (short)(rval * rvol);
        }

        odroid_audio_submit(audioBuffer, AUDIO_BUFFER_LENGTH);
    }

    vTaskDelete(NULL);
}

void osd_snd_init()
{
    host.sound.stereo = true;
    host.sound.freq = AUDIO_SAMPLE_RATE;
    host.sound.sample_size = 1;

    xTaskCreatePinnedToCore(&audioTask, "audioTask", 1024 * 2, NULL, 5, NULL, 1);
}

void osd_snd_shutdown(void)
{
    odroid_audio_stop();
}
