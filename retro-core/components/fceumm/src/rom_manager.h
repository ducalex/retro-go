#ifndef ROM_MANAGER_H
#define ROM_MANAGER_H

#include <stddef.h>
#include "fceu-types.h"

typedef struct {
} rom_system_t;

typedef struct {
  uint8 *address;
} retro_emulator_file_t;

typedef struct {
} rom_manager_t;

extern rom_manager_t rom_mgr;

static inline void *rom_manager_system(rom_manager_t *mgr, const char *name) {
  return NULL;
}

static inline void *rom_manager_get_file(const rom_system_t *system,
                                         const char *name) {
  return NULL;
}

#endif
