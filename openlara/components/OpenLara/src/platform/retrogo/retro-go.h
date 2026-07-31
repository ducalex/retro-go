#ifndef RETROGO_H
#define RETROGO_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <rg_system.h>
#include <rg_display.h>
#include <rg_input.h>
#include <rg_audio.h>
#include <rg_storage.h>
#include <rg_utils.h>

#ifdef __cplusplus
extern "C" {
#endif

// Define GBA-like memory regions or just placeholders if needed
#define MEM_IWRAM
#define MEM_EWRAM
#define IWRAM_DATA
#define EWRAM_DATA
#define EWRAM_BSS
#define IWRAM_CODE
#define EWRAM_CODE

#define rg_free free

extern rg_surface_t *update;
extern uint8_t *fb;

#ifdef __cplusplus
}
#endif

void osSetPalette(const uint16_t* palette);
int osGetSystemTimeMS();
bool osSaveSettings();
bool osLoadSettings();
bool osCheckSave();
bool osSaveGame();
bool osLoadGame();
void osJoyVibrate(int32_t index, int32_t L, int32_t R);
const void* osLoadScreen(int32_t id);
const void* osLoadLevel(int32_t id);

#endif
