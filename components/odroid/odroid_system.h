#pragma once

#include <esp_system.h>
#include <esp_heap_caps.h>
#include <esp_timer.h>
#include <stdbool.h>

#if defined(ESP_IDF_VERSION_MAJOR) && ESP_IDF_VERSION_MAJOR >= 4
#include <esp32/rom/crc.h>
#else
#include <rom/crc.h>
#endif

#include "config.h"

#include "odroid_audio.h"
#include "odroid_display.h"
#include "odroid_input.h"
#include "odroid_netplay.h"
#include "odroid_overlay.h"
#include "odroid_profiler.h"
#include "odroid_sdcard.h"
#include "odroid_settings.h"

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
     ODROID_PATH_SAVE_STATE = 0,
     ODROID_PATH_SAVE_STATE_1,
     ODROID_PATH_SAVE_STATE_2,
     ODROID_PATH_SAVE_STATE_3,
     ODROID_PATH_SAVE_BACK,
     ODROID_PATH_SAVE_SRAM,
     ODROID_PATH_TEMP_FILE,
     ODROID_PATH_ROM_FILE,
     ODROID_PATH_ART_FILE,
     ODROID_PATH_CRC_CACHE,
} emu_path_type_t;

typedef enum
{
     SPI_LOCK_ANY     = 0,
     SPI_LOCK_SDCARD  = 1,
     SPI_LOCK_DISPLAY = 2,
} spi_lock_res_t;

typedef struct {
    short nsamples;
    short count;
    float avg;
    float last;
} avgr_t;

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
     odroid_battery_state_t battery;
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

typedef struct
{
     uint magicWord;
     uint errorCode;
     char message[128];
     char function[128];
     char file[128];
     uint backtrace[32];
} panic_trace_t;

#define PANIC_TRACE_MAGIC 0x12345678

void odroid_system_init(int app_id, int sampleRate);
char* odroid_system_get_path(emu_path_type_t type, const char *romPath);
void odroid_system_emu_init(state_handler_t load, state_handler_t save, netplay_callback_t netplay_cb);
bool odroid_system_emu_save_state(int slot);
bool odroid_system_emu_load_state(int slot);
void odroid_system_panic_dialog(const char *reason);
void odroid_system_panic(const char *reason, const char *file, const char *function) __attribute__((noreturn));
void odroid_system_halt() __attribute__((noreturn));
void odroid_system_sleep();
void odroid_system_switch_app(int app) __attribute__((noreturn));
void odroid_system_reload_app() __attribute__((noreturn));
void odroid_system_set_boot_app(int slot);
void odroid_system_set_led(int value);
void odroid_system_tick(uint skippedFrame, uint fullFrame, uint busyTime);
rg_app_desc_t* odroid_system_get_app();
runtime_stats_t odroid_system_get_stats();

void odroid_system_spi_lock_acquire(spi_lock_res_t);
void odroid_system_spi_lock_release(spi_lock_res_t);

/* helpers */

static inline uint get_frame_time(uint refresh_rate)
{
     // return (CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ * 1000000) / refresh_rate
     return 1000000 / refresh_rate;
}

static inline uint get_elapsed_time()
{
     // uint now = xthal_get_ccount();
     return (uint)esp_timer_get_time(); // uint is plenty resolution for us
}

static inline uint get_elapsed_time_since(uint start)
{
     // uint now = get_elapsed_time();
     // return ((now > start) ? now - start : ((uint64_t)now + (uint64_t)0xffffffff) - start);
     return get_elapsed_time() - start;
}

#undef MIN
#define MIN(a,b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a < _b ? _a : _b; })
#undef MAX
#define MAX(a,b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a > _b ? _a : _b; })

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

#define RG_PANIC(x) odroid_system_panic(x, __FUNCTION__, __FILE__)

#define MEM_ANY   0
#define MEM_SLOW  MALLOC_CAP_SPIRAM
#define MEM_FAST  MALLOC_CAP_INTERNAL
#define MEM_DMA   MALLOC_CAP_DMA
#define MEM_8BIT  MALLOC_CAP_8BIT
#define MEM_32BIT MALLOC_CAP_32BIT
// #define rg_alloc(...)  rg_alloc_(..., __FILE__, __FUNCTION__)

void *rg_alloc(size_t size, uint32_t caps);
void rg_free(void *ptr);
