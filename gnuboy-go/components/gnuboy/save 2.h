#pragma once

#include <stdbool.h>

int sram_load(const char *file);
int sram_save(const char *file, bool quick_save);
int state_load(const char *file);
int state_save(const char *file);
