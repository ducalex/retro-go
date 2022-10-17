#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <rg_system.h>

#define AUDIO_SAMPLE_RATE   (32000)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 50 + 1)

extern rg_audio_sample_t audioBuffer[AUDIO_BUFFER_LENGTH];

extern rg_video_update_t updates[2];
extern rg_video_update_t *currentUpdate;
extern rg_video_update_t *previousUpdate;

extern rg_app_t *app;

extern uint8_t shared_memory_block_64K[0x10000];

extern void gbc_main();
extern void nes_main();
extern void pce_main();
extern void gw_main();
extern void lnx_main();
