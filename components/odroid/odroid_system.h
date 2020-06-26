#pragma once

#include "odroid_audio.h"
#include "odroid_display.h"
#include "odroid_input.h"
#include "odroid_overlay.h"
#include "odroid_netplay.h"
#include "odroid_sdcard.h"
#include "odroid_settings.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "stdbool.h"

extern int8_t speedupEnabled;

typedef bool (*state_handler_t)(char *pathName);

typedef struct
{
     char *romPath;
     char *startAction;
     int8_t speedup;
     state_handler_t load;
     state_handler_t save;
} emu_state_t;

#define ODROID_BASE_PATH_ROMS      SD_BASE_PATH "/roms"
#define ODROID_BASE_PATH_SAVES     SD_BASE_PATH "/odroid/data"
#define ODROID_BASE_PATH_TEMP      SD_BASE_PATH "/odroid/data" // temp
#define ODROID_BASE_PATH_ROMART    SD_BASE_PATH "/romart"
#define ODROID_BASE_PATH_CRC_CACHE SD_BASE_PATH "/odroid/cache/crc"

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
     odroid_battery_state battery;
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

void odroid_system_emu_init(state_handler_t load, state_handler_t save, netplay_callback_t netplay_cb);
bool odroid_system_emu_save_state(int slot);
bool odroid_system_emu_load_state(int slot);
void odroid_system_init(int app_id, int sampleRate);
uint odroid_system_get_app_id();
void odroid_system_set_app_id(int appId);
uint odroid_system_get_game_id();
uint odroid_system_get_start_action();
const char* odroid_system_get_rom_path();
char* odroid_system_get_path(emu_path_type_t type, char *romPath);
void odroid_system_panic(const char *reason);
void odroid_system_halt();
void odroid_system_sleep();
void odroid_system_switch_app(int app);
void odroid_system_reload_app();
void odroid_system_set_boot_app(int slot);
void odroid_system_set_led(int value);
void odroid_system_tick(uint skippedFrame, uint fullFrame, uint busyTime);
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

#define MEM_ANY   0
#define MEM_SLOW  MALLOC_CAP_SPIRAM
#define MEM_FAST  MALLOC_CAP_INTERNAL
#define MEM_DMA   MALLOC_CAP_DMA
#define MEM_8BIT  MALLOC_CAP_8BIT
#define MEM_32BIT MALLOC_CAP_32BIT
// #define rg_alloc(...)  rg_alloc_(..., __FILE__, __FUNCTION__)

void *rg_alloc(size_t size, uint32_t caps);
void rg_free(void *ptr);

void rg_perf_init(void);
void rg_perf_reset(void);
void rg_perf_print(void);
void rg_perf_func_enter(void *func_ptr, char *func_name);
void rg_perf_func_leave(void);
