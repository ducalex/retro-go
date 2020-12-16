#pragma once

#include <esp_system.h>
#include <esp_heap_caps.h>
#include <esp_timer.h>
#include <esp_attr.h>
#include <stdbool.h>

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
#include "rg_sdcard.h"
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
    int32_t startAction;
} rg_app_desc_t;

typedef enum
{
    EMU_PATH_SAVE_STATE = 0,
    EMU_PATH_SAVE_STATE_1,
    EMU_PATH_SAVE_STATE_2,
    EMU_PATH_SAVE_STATE_3,
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
    uint totalFrames;
    uint skippedFrames;
    uint fullFrames;
    uint busyTime;
    uint realTime;
    uint resetTime;
} runtime_counters_t;

typedef struct
{
    battery_state_t battery;
    float partialFPS;
    float skippedFPS;
    float totalFPS;
    float emulatedSpeed;
    float busyPercent;
    uint lastTickTime;
    uint freeMemoryInt;
    uint freeMemoryExt;
    uint freeBlockInt;
    uint freeBlockExt;
    uint idleTimeCPU0;
    uint idleTimeCPU1;
} runtime_stats_t;

void rg_system_init(int app_id, int sampleRate);
void rg_system_panic_dialog(const char *reason);
void rg_system_panic(const char *reason, const char *function, const char *file) __attribute__((noreturn));
void rg_system_halt() __attribute__((noreturn));
void rg_system_sleep() __attribute__((noreturn));
void rg_system_restart() __attribute__((noreturn));
void rg_system_switch_app(const char *app) __attribute__((noreturn));
void rg_system_set_boot_app(const char *app);
bool rg_system_find_app(const char *app);
void rg_system_set_led(int value);
void rg_system_tick(uint skippedFrame, uint fullFrame, uint busyTime);
rg_app_desc_t *rg_system_get_app();
runtime_stats_t rg_system_get_stats();

void rg_emu_init(state_handler_t load, state_handler_t save, netplay_callback_t netplay_cb);
char *rg_emu_get_path(emu_path_type_t type, const char *romPath);
bool rg_emu_save_state(int slot);
bool rg_emu_load_state(int slot);

void rg_spi_lock_acquire(spi_lock_res_t);
void rg_spi_lock_release(spi_lock_res_t);

void *rg_alloc(size_t size, uint32_t caps);
void rg_free(void *ptr);

#define MEM_ANY 0
#define MEM_SLOW MALLOC_CAP_SPIRAM
#define MEM_FAST MALLOC_CAP_INTERNAL
#define MEM_DMA MALLOC_CAP_DMA
#define MEM_8BIT MALLOC_CAP_8BIT
#define MEM_32BIT MALLOC_CAP_32BIT

/* Utilities */
#define get_frame_time(refresh_rate) (uint)(1000000 / (refresh_rate))
#define get_elapsed_time() (uint)esp_timer_get_time()
#define get_elapsed_time_since(start) (uint)(esp_timer_get_time() - (start))

#define RG_MIN(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a < _b ? _a : _b; })
#define RG_MAX(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a > _b ? _a : _b; })

// This should really support printf format...
#define RG_PANIC(x) rg_system_panic(x, __FUNCTION__, __FILE__)

// Attributes

#undef PATH_MAX
#define PATH_MAX 255

#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

#ifndef DRAM_ATTR
#define DRAM_ATTR
#endif

#ifndef NO_PROFILING
#define NO_PROFILING __attribute((no_instrument_function))
#endif
