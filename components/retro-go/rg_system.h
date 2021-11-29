#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <esp_attr.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#if defined(RG_TARGET_ODROID_GO)
    #include "targets/odroid-go.h"
#elif defined(RG_TARGET_MRGC_G32)
    #include "targets/mrgc-g32.h"
#else
    #warning "No target defined. Defaulting to ODROID-GO."
    #include "targets/odroid-go.h"
    #define RG_TARGET_ODROID_GO
#endif

#include "rg_audio.h"
#include "rg_display.h"
#include "rg_input.h"
#include "rg_netplay.h"
#include "rg_sdcard.h"
#include "rg_gui.h"
#include "rg_i2c.h"
#include "rg_profiler.h"
#include "rg_settings.h"

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
    RG_PATH_ROM_FILE,
    RG_PATH_ART_FILE,
} rg_path_type_t;

enum
{
    RG_LOG_PRINT = 0,
    RG_LOG_ERROR,
    RG_LOG_WARN,
    RG_LOG_INFO,
    RG_LOG_DEBUG,
};

typedef enum
{
    RG_EVENT_SHUTDOWN,
    RG_EVENT_SLEEP,
    RG_EVENT_UNRESPONSIVE,
    RG_EVENT_LOWMEMORY,
    RG_EVENT_REDRAW,
} rg_event_t;

#define RG_APP_LAUNCHER "launcher"
#define RG_APP_FACTORY  NULL

typedef bool (*rg_state_handler_t)(const char *filename);
typedef bool (*rg_reset_handler_t)(bool hard);
typedef void (*rg_event_handler_t)(int event, void *arg);
typedef bool (*rg_screenshot_handler_t)(const char *filename, int width, int height);
typedef int  (*rg_mem_read_handler_t)(int addr);
typedef int  (*rg_mem_write_handler_t)(int addr, int value);
typedef void (*rg_settings_handler_t)(void);

typedef struct
{
    rg_state_handler_t loadState;       // rg_emu_load_state() handler
    rg_state_handler_t saveState;       // rg_emu_save_state() handler
    rg_reset_handler_t reset;           // rg_emu_reset() handler
    rg_screenshot_handler_t screenshot; // rg_emu_screenshot() handler
    rg_event_handler_t event;           // listen to retro-go system events
    rg_netplay_handler_t netplay;       // netplay handler
    rg_mem_read_handler_t memRead;      // Used by for cheats and debugging
    rg_mem_write_handler_t memWrite;    // Used by for cheats and debugging
    rg_settings_handler_t settings;     // Called by "More..." in rg_gui_settings_menu()
} rg_emu_proc_t;

// TO DO: Make it an abstract ring buffer implementation?
#define LOG_BUFFER_SIZE 2048
typedef struct
{
    char buffer[LOG_BUFFER_SIZE];
    size_t cursor;
} rg_logbuf_t;

typedef struct
{
    uint32_t totalFrames;
    uint32_t skippedFrames;
    uint32_t fullFrames;
    uint32_t busyTime;
    uint64_t resetTime;
} rg_counters_t;

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
    int inputTimeout;
    const char *romPath;
    void *mainTaskHandle;
    rg_emu_proc_t handlers;
    rg_logbuf_t log;
} rg_app_t;

typedef struct
{
    float batteryPercent;
    float batteryVoltage;
    float partialFPS;
    float skippedFPS;
    float totalFPS;
    float busyPercent;
    uint32_t freeMemoryInt;
    uint32_t freeMemoryExt;
    uint32_t freeBlockInt;
    uint32_t freeBlockExt;
    uint32_t freeStackMain;
} rg_stats_t;

rg_app_t *rg_system_init(int sampleRate, const rg_emu_proc_t *handlers);
void rg_system_panic(const char *reason, const char *context) __attribute__((noreturn));
void rg_system_shutdown() __attribute__((noreturn));
void rg_system_sleep() __attribute__((noreturn));
void rg_system_restart() __attribute__((noreturn));
void rg_system_switch_app(const char *app) __attribute__((noreturn));
void rg_system_set_boot_app(const char *app);
bool rg_system_find_app(const char *app);
void rg_system_set_led(int value);
int  rg_system_get_led(void);
void rg_system_tick(int busyTime);
void rg_system_log(int level, const char *context, const char *format, ...);
bool rg_system_save_trace(const char *filename, bool append);
void rg_system_event(rg_event_t event, void *arg);
rg_app_t *rg_system_get_app();
rg_stats_t rg_system_get_stats();

void rg_system_time_init();
void rg_system_time_save();

char *rg_emu_get_path(rg_path_type_t type, const char *romPath);
bool rg_emu_save_state(int slot);
bool rg_emu_load_state(int slot);
bool rg_emu_reset(int hard);
bool rg_emu_screenshot(const char *filename, int width, int height);
void rg_emu_start_game(const char *emulator, const char *romPath, rg_start_action_t action);

int32_t rg_system_get_startup_app(void);
void rg_system_set_startup_app(int32_t value);

void *rg_alloc(size_t size, uint32_t caps);

#define MEM_ANY   (0)
#define MEM_SLOW  (1)
#define MEM_FAST  (2)
#define MEM_DMA   (4)
#define MEM_8BIT  (8)
#define MEM_32BIT (16)
#define MEM_EXEC  (32)

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

#define RG_TIMER_INIT() int _rgts_ = get_elapsed_time(), _rgtl_ = get_elapsed_time();
#define RG_TIMER_LAP(name) \
    printf("Lap %s: %.2f   Total: %.2f\n", #name, get_elapsed_time_since(_rgtl_) / 1000.f, \
            get_elapsed_time_since(_rgts_) / 1000.f); _rgtl_ = get_elapsed_time();

#define RG_MIN(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a < _b ? _a : _b; })
#define RG_MAX(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a > _b ? _a : _b; })
#define RG_COUNT(array) (sizeof(array) / sizeof((array)[0]))

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

#define autofree __attribute__(cleanup(free))

#ifdef __cplusplus
}
#endif
