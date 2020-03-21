#pragma once

#include "odroid_audio.h"
#include "odroid_display.h"
#include "odroid_input.h"
#include "odroid_overlay.h"
#include "odroid_netplay.h"
#include "odroid_sdcard.h"
#include "odroid_settings.h"
#include "esp_system.h"
#include "stdbool.h"

void odroid_system_init(int app_id, int sampleRate);
void odroid_system_emu_init(char **romPath, int8_t *startAction);
void odroid_system_set_app_id(int appId);
void odroid_system_quit_app(bool save);
bool odroid_system_save_state(int slot);
bool odroid_system_load_state(int slot);
void odroid_system_panic(const char *reason);
void odroid_system_halt();
void odroid_system_sleep();
void odroid_system_set_boot_app(int slot);
void odroid_system_set_led(int value);
void odroid_system_gpio_init();

extern int8_t speedupEnabled;

extern bool SaveState(char *pathName);
extern bool LoadState(char *pathName);

inline uint get_elapsed_time_since(uint start)
{
     uint now = xthal_get_ccount();
     return ((now > start) ? now - start : ((uint64_t)now + (uint64_t)0xffffffff) - start);
}
