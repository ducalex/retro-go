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
#include "rg_image.h"
#include "rg_gui.h"
#include "rg_profiler.h"
#include "rg_settings.h"
#include "rg_cheats.h"

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

enum {
    RG_LOG_PRINT = 0,
    RG_LOG_ERROR,
    RG_LOG_WARN,
    RG_LOG_INFO,
    RG_LOG_DEBUG,
};

#define RG_STRUCT_MAGIC 0x12345678

typedef bool (*rg_state_handler_t)(const char *filename);
typedef bool (*rg_reset_handler_t)(bool hard);
typedef bool (*rg_message_handler_t)(int msg, void *arg);
typedef bool (*rg_screenshot_handler_t)(const char *filename, int width, int height);
typedef int  (*rg_mem_read_handler_t)(int addr);
typedef bool (*rg_mem_write_handler_t)(int addr, int value);

typedef struct
{
    rg_state_handler_t loadState;
    rg_state_handler_t saveState;
    rg_reset_handler_t reset;
    rg_message_handler_t message;
    rg_netplay_handler_t netplay;
    rg_screenshot_handler_t screenshot;
    rg_mem_read_handler_t memRead;    // Used by for cheats and debugging
    rg_mem_write_handler_t memWrite;  // Used by for cheats and debugging
} rg_emu_proc_t;

// TO DO: Make it an abstract ring buffer implementation?
#define LOG_BUFFER_SIZE 2048
typedef struct
{
    char buffer[LOG_BUFFER_SIZE];
    size_t cursor;
} log_buffer_t;

typedef struct
{
    const char *name;
    const char *version;
    const char *buildDate;
    const char *buildTime;
    const char *buildUser;
    int speedupEnabled;
    int refreshRate;
    int sampleRate;
    int startAction;
    int logLevel;
    int isLauncher;
    const char *romPath;
    void *mainTaskHandle;
    rg_emu_proc_t handlers;
    log_buffer_t log;
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
void rg_system_log(int level, const char *context, const char *format, ...);
void rg_system_write_log(log_buffer_t *log, FILE *fp);
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
#define RG_ASSERT(cond, msg) while (!(cond)) { RG_PANIC("Assertion failed: `" #cond "` : " msg); }

#define RG_LOGX(x, ...) rg_system_log(RG_LOG_PRINT, __func__, x, ## __VA_ARGS__)
#define RG_LOGE(x, ...) rg_system_log(RG_LOG_ERROR, __func__, x, ## __VA_ARGS__)
#define RG_LOGW(x, ...) rg_system_log(RG_LOG_WARN, __func__, x, ## __VA_ARGS__)
#define RG_LOGI(x, ...) rg_system_log(RG_LOG_INFO, __func__, x, ## __VA_ARGS__)
#define RG_LOGD(x, ...) rg_system_log(RG_LOG_DEBUG, __func__, x, ## __VA_ARGS__)

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
