#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <rg_system.h>

#define AUDIO_SAMPLE_RATE   (32000)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 50 + 1)

extern rg_audio_sample_t audioBuffer[AUDIO_BUFFER_LENGTH];
extern rg_app_t *app;

extern uint8_t shared_memory_block_64K[0x10000];

void launcher_main();
void gbc_main();
void nes_main();
void pce_main();
void sms_main();
void gw_main();
void lynx_main();
void snes_main();
