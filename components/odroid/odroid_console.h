#pragma once

#include "odroid_audio.h"
#include "odroid_display.h"
#include "odroid_input.h"
#include "odroid_overlay.h"
#include "odroid_sdcard.h"
#include "odroid_settings.h"
#include "odroid_system.h"
#include "stdbool.h"

void odroid_console_init(char **romPath, bool *reset, int sample_rate);
