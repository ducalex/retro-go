#include "odroid_system.h"
#include "osd_snd.h"

#define AUDIO_SAMPLE_RATE 22050
#define AUDIO_BUFFER_SIZE 4096
#define AUDIO_CHANNELS 6

short *sbuf_mix;
char *sbuf[AUDIO_CHANNELS];

static void
audioTask_mode0(void *arg)
{
    printf("%s: STARTED\n", __func__);

    while (1)
    {
        short *p = sbuf_mix;

        for (int i = 0; i < AUDIO_CHANNELS; i++)
        {
            WriteBuffer(sbuf[i], i, AUDIO_BUFFER_SIZE / 2);
        }

        uchar lvol, rvol;

        lvol = (io.psg_volume >> 4) * 1.22;
        rvol = (io.psg_volume & 0x0F) * 1.22;

        for (int i = 0; i < AUDIO_BUFFER_SIZE / 2; i++)
        {
            short lval = 0;
            short rval = 0;
            for (int j = 0; j < AUDIO_CHANNELS; j++)
            {
                lval += (short)sbuf[j][i];
                rval += (short)sbuf[j][i + 1];
            }
            //lval = lval/AUDIO_CHANNELS;
            //rval = rval/AUDIO_CHANNELS;
            lval = lval * lvol;
            rval = rval * rvol;
            *p = lval;
            *(p + 1) = rval;
            p += 2;
        }

        odroid_audio_submit(sbuf_mix, AUDIO_BUFFER_SIZE / 4);
    }

    vTaskDelete(NULL);
}

void osd_snd_set_volume(uchar v)
{
}

int osd_snd_init_sound()
{
    host.sound.stereo = true;
    host.sound.signed_sound = false;
    host.sound.freq = AUDIO_SAMPLE_RATE;
    host.sound.sample_size = 1;

    for (int i = 0; i < AUDIO_CHANNELS; i++)
    {
        sbuf[i] = rg_alloc(AUDIO_BUFFER_SIZE / 2, MEM_SLOW);
    }
    sbuf_mix = rg_alloc(AUDIO_BUFFER_SIZE, MEM_SLOW);

    xTaskCreatePinnedToCore(&audioTask_mode0, "audioTask", 1024 * 4, NULL, 5, NULL, 1);
}

void osd_snd_trash_sound(void)
{
    odroid_audio_clear_buffer();
    odroid_audio_stop();
}
