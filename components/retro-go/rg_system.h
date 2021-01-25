#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <esp_idf_version.h>
#include <esp_attr.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#if defined(ESP_IDF_VERSION_MAJOR) && ESP_IDF_VERSION_MAJOR >= 4
#include <esp32/rom/crc.h>
#else
#include <rom/crc.h>
#endif

#include "config.h"

#include "rg_audio.h"
#include "rg_display.h"
#include "rg_input.h"
#include "rg_netplay.h"
#include "rg_gui.h"
#include "rg_profiler.h"
#include "rg_settings.h"

typedef bool (*state_handler_t)(char *pathName);

typedef struct
{
    uint32_t id;
    uint32_t gameId;
    const char *romPath;
    state_handler_t loadState;
    state_handler_t saveState;
    int32_t speedupEnabled;
    int32_t refreshRate;
    int32_t sampleRate;
    int32_t startAction;
} rg_app_desc_t;

typedef enum
{
    EMU_PATH_SAVE_STATE = 0,
    EMU_PATH_SAVE_STATE_1,
    EMU_PATH_SAVE_STATE_2,
    EMU_PATH_SAVE_STATE_3,
    EMU_PATH_SCREENSHOT,
    EMU_PATH_SAVE_BACK,
    EMU_PATH_SAVE_SRAM,
    EMU_PATH_TEMP_FILE,
    EMU_PATH_ROM_FILE,
    EMU_PATH_ART_FILE,
    EMU_PATH_CRC_CACHE,
} emu_path_type_t;

typedef enum
{
    SPI_LOCK_ANY = 0,
    SPI_LOCK_SDCARD = 1,
    SPI_LOCK_DISPLAY = 2,
} spi_lock_res_t;

typedef struct
{
    uint32_t totalFrames;
    uint32_t skippedFrames;
    uint32_t fullFrames;
    uint32_t busyTime;
    uint64_t resetTime;
    uint32_t ticks;
} runtime_counters_t;

typedef struct
{
    battery_state_t battery;
    float partialFPS;
    float skippedFPS;
    float totalFPS;
    float busyPercent;
    uint32_t freeMemoryInt;
    uint32_t freeMemoryExt;
    uint32_t freeBlockInt;
    uint32_t freeBlockExt;
} runtime_stats_t;

void rg_system_init(int app_id, int sampleRate);
void rg_system_panic(const char *reason, const char *function, const char *file) __attribute__((noreturn));
void rg_system_halt() __attribute__((noreturn));
void rg_system_sleep() __attribute__((noreturn));
void rg_system_restart() __attribute__((noreturn));
void rg_system_switch_app(const char *app) __attribute__((noreturn));
void rg_system_set_boot_app(const char *app);
bool rg_system_find_app(const char *app);
void rg_system_set_led(int value);
void rg_system_tick(bool skippedFrame, bool fullFrame, int busyTime);
rg_app_desc_t *rg_system_get_app();
runtime_stats_t rg_system_get_stats();

void rg_system_time_init();
void rg_system_time_save();

void rg_emu_init(state_handler_t load, state_handler_t save, netplay_callback_t netplay_cb);
char *rg_emu_get_path(emu_path_type_t type, const char *romPath);
bool rg_emu_save_state(int slot);
bool rg_emu_load_state(int slot);

void rg_spi_lock_acquire(spi_lock_res_t);
void rg_spi_lock_release(spi_lock_res_t);

bool rg_sdcard_mount();
bool rg_sdcard_unmount();

FILE *rg_fopen(const char *filename, const char *mode);
int rg_fclose(FILE *fp);
bool rg_mkdir(const char *path);
bool rg_unlink(const char* path);
bool rg_readdir(const char* path, char **out_files, size_t *out_count);
long rg_filesize(const char *path);

const char* rg_get_filename(const char *path);
const char* rg_get_extension(const char *path);

void *rg_alloc(size_t size, uint32_t caps);
void rg_free(void *ptr);

#define MEM_ANY   (0)
#define MEM_SLOW  (1)
#define MEM_FAST  (2)
#define MEM_DMA   (4)
#define MEM_8BIT  (8)
#define MEM_32BIT (16)

/* Utilities */

// Functions from esp-idf, we don't include their header but they will be linked
extern int64_t esp_timer_get_time();
extern void heap_caps_malloc_extmem_enable(size_t limit);

// long microseconds
#define get_frame_time(refresh_rate) (1000000 / (refresh_rate))
// int64_t microseconds
#define get_elapsed_time() esp_timer_get_time()
// int64_t microseconds
#define get_elapsed_time_since(start) (esp_timer_get_time() - (start))

#define RG_MIN(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a < _b ? _a : _b; })
#define RG_MAX(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a > _b ? _a : _b; })

// This should really support printf format...
#define RG_PANIC(x) rg_system_panic(x, __FUNCTION__, __FILE__)
#define RG_ASSERT(cond, x) do { if (!(cond)) rg_system_panic(x, __FUNCTION__, __FILE__); } while(0);

// Attributes

#undef PATH_MAX
#define PATH_MAX 255

#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

#ifndef DRAM_ATTR
#define DRAM_ATTR
#endif

#ifdef __cplusplus
}
#endif
