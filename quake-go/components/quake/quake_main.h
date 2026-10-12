#pragma once

#include <stdint.h>
#include <stdbool.h>

#define QUAKE_MAX_PATH 128

void quake_main(const char *basedir, const char *game, const char *pak0Path,
                const char *pak1Path,
                uint32_t pakSize, const void *pakMmap);
void quake_request_exit(void);
void quake_shutdown(void);
void quake_redraw(void);
bool quake_screenshot(const char *filename, int width, int height);
