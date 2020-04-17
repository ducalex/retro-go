#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include <odroid_system.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pce.h"

#include "./odroid_debug.h"

#define APP_ID 40

#define AUDIO_SAMPLE_RATE 22050
#define AUDIO_BUFFER_SIZE 4096
#define AUDIO_CHANNELS 6

extern uchar *SPM_raw, *SPM;
extern uint8_t* framebuffers[2];


void *my_special_alloc(unsigned char speed, unsigned char bytes, unsigned long size)
{
    uint32_t caps = (speed?MALLOC_CAP_INTERNAL:MALLOC_CAP_SPIRAM) |
      ( bytes==1?MALLOC_CAP_8BIT:MALLOC_CAP_32BIT);
      /*
    if (speed) {
        uint32_t max = heap_caps_get_largest_free_block(caps);
        if (max < size) {
            printf("ALLOC: Size: %u; Max FREE for internal is '%u'. Allocating in SPI RAM\n", (unsigned int)size, max);
            caps = MALLOC_CAP_SPIRAM | ( size==1?MALLOC_CAP_8BIT:MALLOC_CAP_32BIT);
        }
    } else {
      caps = MALLOC_CAP_SPIRAM | MALLOC_CAP_32BIT;
    }
    */
    if (!speed) caps = MALLOC_CAP_SPIRAM | MALLOC_CAP_32BIT;
    //if (!speed || size!=65536) caps = MALLOC_CAP_SPIRAM | MALLOC_CAP_32BIT; // only RAM
    void *rc = heap_caps_calloc(1, size, caps);
    printf("ALLOC: Size: %-10u; SPI: %u; 32BIT: %u; RC: %p\n", (unsigned int)size, (caps&MALLOC_CAP_SPIRAM)!=0, (caps&MALLOC_CAP_32BIT)!=0, rc);
    if (!rc) { abort(); }
    return rc;
}


#ifdef MY_SND_AS_TASK
QueueHandle_t audioQueue;
volatile bool audioTaskIsRunning = false;

short *sbuf_mix[2];
char *sbuf[AUDIO_CHANNELS];
void audioTask_mode0(void *arg) {
    uint8_t* param;
    audioTaskIsRunning = true;
    printf("%s: STARTED\n", __func__);
    uint8_t buf = 0;

    while(1)
    {
        if (xQueuePeek(audioQueue, &param, 0) == pdTRUE)
        {
            if (param == (void*)1)
                break;
            xQueueReceive(audioQueue, &param, portMAX_DELAY);
        }
        //memset(sbuf[buf], 0, AUDIO_BUFFER_SIZE);
        usleep(10*1000);
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
    xQueueReceive(audioQueue, &param, portMAX_DELAY);
    audioTaskIsRunning = false;
    printf("%s: FINISHED\n", __func__);
    vTaskDelete(NULL);
    while (1) {}
}
#endif


void app_main(void)
{
    printf("huexpress (%s-%s).\n", COMPILEDATE, GITREV);

    odroid_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    odroid_system_emu_init(NULL, NULL, NULL);

    char *romFile = odroid_system_get_path(NULL, ODROID_PATH_ROM_FILE);

    framebuffers[0] = calloc(1, XBUF_WIDTH * XBUF_HEIGHT);
    framebuffers[1] = calloc(1, XBUF_WIDTH * XBUF_HEIGHT);
    SPM_raw         = calloc(1, XBUF_WIDTH * XBUF_HEIGHT);
    SPM = SPM_raw + XBUF_WIDTH * 64 + 32;

    assert(framebuffers[0] && framebuffers[1] && SPM_raw);

    strcpy(config_basepath, "/sd/odroid/data/pce");
    strcpy(tmp_basepath, "");
    strcpy(video_path, "");
    strcpy(ISO_filename, "");
    strcpy(sav_basepath,"/sd/odroid/data");
    strcpy(sav_path,"pce");
    strcpy(log_filename,"");

    InitPCE(romFile);

    osd_init_machine();
    host.sound.stereo = true;
    host.sound.signed_sound = false;
    host.sound.freq = AUDIO_SAMPLE_RATE;
    host.sound.sample_size = 1;

    for (int i = 0;i < AUDIO_CHANNELS; i++)
    {
        sbuf[i] = my_special_alloc(false, 1, AUDIO_BUFFER_SIZE/2);
    }
    sbuf_mix[0] = my_special_alloc(false, 1, AUDIO_BUFFER_SIZE);
    sbuf_mix[1] = my_special_alloc(false, 1, AUDIO_BUFFER_SIZE);

#ifdef MY_SND_AS_TASK
    audioQueue = xQueueCreate(1, sizeof(uint16_t*));
    xTaskCreatePinnedToCore(&audioTask_mode0, "audioTask", 1024 * 4, NULL, 5, NULL, 1);
#endif

   RunPCE();

   printf("Huexpress died.\n");
   abort();
}
