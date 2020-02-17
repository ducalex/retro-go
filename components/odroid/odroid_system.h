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

void odroid_system_init(int app_id, int sampleRate, char **romPath);
void odroid_system_halt();
void odroid_system_sleep();
void odroid_system_application_set(int slot);
void odroid_system_led_set(int value);
void odroid_system_gpio_init();

extern ODROID_START_ACTION startAction;
extern ODROID_SCALING scalingMode;
extern int8_t forceRedraw;
extern int8_t speedupEnabled;

inline uint get_elapsed_time_since(uint start)
{
     uint now = xthal_get_ccount();
     return ((now > start) ? now - start : ((uint64_t)now + (uint64_t)0xffffffff) - start);
}
