#include "odroid_system.h"
#include "osd_snd.h"

#define AUDIO_SAMPLE_RATE 22050
#define AUDIO_BUFFER_SIZE 4096
#define AUDIO_CHANNELS 6

short *sbuf_mix[2];
char *sbuf[AUDIO_CHANNELS];


static void
audioTask_mode0(void *arg)
{
    uint8_t* param;
    printf("%s: STARTED\n", __func__);
    uint8_t buf = 0;

    while (1)
    {
        char *sbufp[AUDIO_CHANNELS];
        for (int i = 0;i < AUDIO_CHANNELS;i++)
        {
          sbufp[i] = sbuf[i];
          WriteBuffer((char*)sbuf[i], i, AUDIO_BUFFER_SIZE/2);
        }
        uchar lvol, rvol;
        lvol = (io.psg_volume >> 4) * 1.22;
        rvol = (io.psg_volume & 0x0F) * 1.22;

        short *p = sbuf_mix[buf];
        for (int i = 0;i < AUDIO_BUFFER_SIZE/2;i++)
        {
             short lval = 0;
             short rval = 0;
            for (int j = 0;j< AUDIO_CHANNELS;j++)
            {
                lval+=( short)sbufp[j][i];
                rval+=( short)sbufp[j][i+1];
            }
            //lval = lval/AUDIO_CHANNELS;
            //rval = rval/AUDIO_CHANNELS;
            lval = lval * lvol;
            rval = rval * rvol;
            *p = lval;
            *(p+1) = rval;
            p+=2;
        }

        odroid_audio_submit((short*)sbuf_mix[buf], AUDIO_BUFFER_SIZE/4);
        buf = buf?0:1;
    }

    vTaskDelete(NULL);
}

void
osd_snd_set_volume(uchar v)
{
}

int
osd_snd_init_sound()
{
    host.sound.stereo = true;
    host.sound.signed_sound = false;
    host.sound.freq = AUDIO_SAMPLE_RATE;
    host.sound.sample_size = 1;

    for (int i = 0;i < AUDIO_CHANNELS; i++)
    {
        sbuf[i] = rg_alloc(AUDIO_BUFFER_SIZE/2, MEM_SLOW);
    }
    sbuf_mix[0] = rg_alloc(AUDIO_BUFFER_SIZE, MEM_SLOW);
    sbuf_mix[1] = rg_alloc(AUDIO_BUFFER_SIZE, MEM_SLOW);

    xTaskCreatePinnedToCore(&audioTask_mode0, "audioTask", 1024 * 4, NULL, 5, NULL, 1);
}

void
osd_snd_trash_sound(void)
{
	odroid_audio_clear_buffer();
	odroid_audio_stop();
}
