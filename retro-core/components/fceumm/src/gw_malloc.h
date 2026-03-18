#ifndef GW_MALLOC_H
#define GW_MALLOC_H

#include "rg_utils.h"
#include <stdlib.h>
#include <string.h>


/**
 * Compatibility layer for Retro-Go (ESP32-S3)
 * These functions were originally for the Game & Watch port of FCEUMM.
 * On Retro-Go, we map them to silver heap allocations (PSRAM).
 */

static inline void *ahb_calloc(size_t nmemb, size_t size) {
  return rg_alloc(nmemb * size, MEM_SLOW);
}

static inline void *itc_calloc(size_t nmemb, size_t size) {
  return rg_alloc(nmemb * size, MEM_SLOW);
}

static inline void *ahb_malloc(size_t size) { return rg_alloc(size, MEM_SLOW); }

static inline void *itc_malloc(size_t size) { return rg_alloc(size, MEM_SLOW); }

static inline void itc_free(void *ptr) { free(ptr); }

static inline void ahb_free(void *ptr) { free(ptr); }

#endif
