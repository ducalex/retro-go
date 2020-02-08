#pragma once

#include "odroid_audio.h"
#include "odroid_display.h"
#include "odroid_input.h"
#include "odroid_overlay.h"
#include "odroid_sdcard.h"
#include "odroid_settings.h"
#include "stdbool.h"

void odroid_system_init(int app_id, int sample_rate, char **romPath, bool *reset);
void odroid_system_halt();
void odroid_system_sleep();
void odroid_system_application_set(int slot);
void odroid_system_led_set(int value);
void odroid_system_gpio_init();
