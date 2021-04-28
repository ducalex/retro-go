#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <esp_attr.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "config.h"

#include "rg_audio.h"
#include "rg_display.h"
#include "rg_input.h"
#include "rg_netplay.h"
#include "rg_vfs.h"
#include "rg_gui.h"
#include "rg_profiler.h"
#include "rg_settings.h"

typedef enum
{
    RG_MSG_SHUTDOWN,
    RG_MSG_RESTART,
    RG_MSG_RESET,
    RG_MSG_REDRAW,
    RG_MSG_NETPLAY,
    RG_MSG_SAVE_STATE,
    RG_MSG_LOAD_STATE,
    RG_MSG_SAVE_SRAM,
    RG_MSG_LOAD_SRAM,
} rg_msg_t;

typedef enum
{
    RG_START_ACTION_RESUME = 0,
    RG_START_ACTION_NEWGAME,
    RG_START_ACTION_NETPLAY
} rg_start_action_t;

typedef enum
{
    RG_PATH_SAVE_STATE = 0,
    RG_PATH_SAVE_STATE_1,
    RG_PATH_SAVE_STATE_2,
    RG_PATH_SAVE_STATE_3,
    RG_PATH_SCREENSHOT,
    RG_PATH_SAVE_SRAM,
    RG_PATH_TEMP_FILE,
    RG_PATH_ROM_FILE,
    RG_PATH_ART_FILE,
} rg_path_type_t;

typedef enum
{
    SPI_LOCK_ANY = 0,
    SPI_LOCK_SDCARD = 1,
    SPI_LOCK_DISPLAY = 2,
    SPI_LOCK_OTHER = 3,
} spi_lock_res_t;

typedef bool (*state_handler_t)(const char *filename);
typedef bool (*reset_handler_t)(bool hard);
typedef bool (*message_handler_t)(int msg, void *arg);
typedef bool (*screenshot_handler_t)(const char *filename, int width, int height);

typedef struct
{
    state_handler_t loadState;
    state_handler_t saveState;
    reset_handler_t reset;
    message_handler_t message;
    netplay_handler_t netplay;
    screenshot_handler_t screenshot;
} rg_emu_proc_t;

typedef struct
{
    const char *name;
    const char *version;
    const char *buildDate;
    const char *buildTime;
    int speedupEnabled;
    int refreshRate;
    int sampleRate;
    int startAction;
    const char *romPath;
    void *mainTaskHandle;
    rg_emu_proc_t handlers;
} rg_app_desc_t;

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
    uint32_t freeStackMain;
    uint32_t magicWord;
} runtime_stats_t;

rg_app_desc_t *rg_system_init(int sampleRate, const rg_emu_proc_t *handlers);
void rg_system_panic(const char *reason, const char *context) __attribute__((noreturn));
void rg_system_halt() __attribute__((noreturn));
void rg_system_sleep() __attribute__((noreturn));
void rg_system_restart() __attribute__((noreturn));
void rg_system_switch_app(const char *app) __attribute__((noreturn));
void rg_system_set_boot_app(const char *app);
bool rg_system_find_app(const char *app);
void rg_system_set_led(int value);
int  rg_system_get_led(void);
void rg_system_tick(bool skippedFrame, bool fullFrame, int busyTime);
rg_app_desc_t *rg_system_get_app();
runtime_stats_t rg_system_get_stats();

void rg_system_time_init();
void rg_system_time_save();

char *rg_emu_get_path(rg_path_type_t type, const char *romPath);
bool rg_emu_save_state(int slot);
bool rg_emu_load_state(int slot);
bool rg_emu_reset(int hard);
bool rg_emu_notify(int msg, void *arg);
bool rg_emu_screenshot(const char *file, int width, int height);
void rg_emu_start_game(const char *emulator, const char *romPath, rg_start_action_t action);

int32_t rg_system_get_startup_app(void);
void rg_system_set_startup_app(int32_t value);

void rg_spi_lock_acquire(spi_lock_res_t);
void rg_spi_lock_release(spi_lock_res_t);

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
extern uint32_t crc32_le(uint32_t crc, const uint8_t * buf, uint32_t len);

// long microseconds
#define get_frame_time(refresh_rate) (1000000 / (refresh_rate))
// int64_t microseconds
#define get_elapsed_time() esp_timer_get_time()
// int64_t microseconds
#define get_elapsed_time_since(start) (esp_timer_get_time() - (start))

#define RG_MIN(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a < _b ? _a : _b; })
#define RG_MAX(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a > _b ? _a : _b; })

// This should really support printf format...
#define RG_PANIC(x) rg_system_panic(x, __FUNCTION__)
#define RG_ASSERT(cond, x) while (!(cond)) { RG_PANIC(x); }

#define RG_LOGX(x, ...) printf(x, ## __VA_ARGS__)
#define RG_LOGE(x, ...) printf("!! %s: " x, __func__, ## __VA_ARGS__)
#define RG_LOGW(x, ...) printf(" ! %s: " x, __func__, ## __VA_ARGS__)
#define RG_LOGI(x, ...) printf("%s: " x, __func__, ## __VA_ARGS__)
//#define RG_LOGD(x, ...) printf("> %s: " x, __func__, ## __VA_ARGS__)
#define RG_LOGD(x, ...) {}

#define RG_DUMP(...) {}

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
