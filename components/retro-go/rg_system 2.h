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
#elif defined(RG_TARGET_QTPY_GAMER)
    #include "targets/qtpy-gamer.h"
#else
    #warning "No target defined. Defaulting to ODROID-GO."
    #include "targets/odroid-go.h"
    #define RG_TARGET_ODROID_GO
#endif

#include "rg_audio.h"
#include "rg_display.h"
#include "rg_input.h"
#include "rg_netplay.h"
#include "rg_storage.h"
#include "rg_gui.h"
#include "rg_i2c.h"
#include "rg_profiler.h"
#include "rg_printf.h"

typedef enum
{
    RG_PATH_SAVE_STATE = 0x100,
    RG_PATH_SAVE_SRAM  = 0x200,
    RG_PATH_SCREENSHOT = 0x300,
    RG_PATH_ROM_FILE   = 0x400,
    RG_PATH_CACHE_FILE = 0x500,
} rg_path_type_t;

enum
{
    RG_BOOT_NORMAL = 0x0,
    RG_BOOT_RESUME = 0x1,
    RG_BOOT_ONCE   = 0x2,
    RG_BOOT_RESET  = 0x4,
    RG_BOOT_NETPLAY= 0x8,
};

enum
{
    RG_LOG_PRINT = 0,
    RG_LOG_USER,
    RG_LOG_ERROR,
    RG_LOG_WARN,
    RG_LOG_INFO,
    RG_LOG_DEBUG,
    RG_LOG_MAX,
};

typedef enum
{
    RG_OK    = 0,
    RG_FAIL  = 1,
    RG_NOMEM = 2,
} rg_err_t;

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
} rg_handlers_t;

// TO DO: Make it an abstract ring buffer implementation?
#define RG_LOGBUF_SIZE 2048
typedef struct
{
    char buffer[RG_LOGBUF_SIZE];
    size_t cursor;
} rg_logbuf_t;

typedef struct
{
    uint32_t totalFrames;
    uint32_t skippedFrames;
    uint32_t fullFrames;
    uint32_t busyTime;
} rg_counters_t;

typedef struct
{
    // const char *partition;
    const char *name;
    const char *version;
    const char *buildDate;
    const char *buildTime;
    const char *buildUser;
    const char *configNs;
    int bootFlags;
    int speedupEnabled;
    int refreshRate;
    int sampleRate;
    int logLevel;
    int isLauncher;
    const char *romPath;
    void *mainTaskHandle;
    const rg_gui_option_t *options;
    rg_handlers_t handlers;
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

rg_app_t *rg_system_init(int sampleRate, const rg_handlers_t *handlers, const rg_gui_option_t *options);
void rg_system_panic(const char *context, const char *message) __attribute__((noreturn));
void rg_system_shutdown(void) __attribute__((noreturn));
void rg_system_sleep(void) __attribute__((noreturn));
void rg_system_restart(void) __attribute__((noreturn));
void rg_system_start_app(const char *app, const char *name, const char *args, int flags) __attribute__((noreturn));
void rg_system_set_boot_app(const char *app);
bool rg_system_find_app(const char *app);
void rg_system_set_led(int value);
int  rg_system_get_led(void);
void rg_system_tick(int busyTime);
void rg_system_vlog(int level, const char *context, const char *format, va_list va);
void rg_system_log(int level, const char *context, const char *format, ...) __attribute__((format(printf,3,4)));
bool rg_system_save_trace(const char *filename, bool append);
void rg_system_event(rg_event_t event, void *arg);
char *rg_system_get_path(char *buffer, rg_path_type_t type, const char *filename);
rg_app_t *rg_system_get_app(void);
rg_stats_t rg_system_get_stats(void);

bool rg_emu_save_state(int slot);
bool rg_emu_load_state(int slot);
bool rg_emu_reset(int hard);
bool rg_emu_screenshot(const char *filename, int width, int height);

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
    RG_LOGX("Lap %s: %.2f   Total: %.2f\n", #name, get_elapsed_time_since(_rgtl_) / 1000.f, \
            get_elapsed_time_since(_rgts_) / 1000.f); _rgtl_ = get_elapsed_time();

#define RG_MIN(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a < _b ? _a : _b; })
#define RG_MAX(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a > _b ? _a : _b; })
#define RG_COUNT(array) (sizeof(array) / sizeof((array)[0]))

// This should really support printf format...
#define RG_PANIC(x) rg_system_panic(__func__, x)
#define RG_ASSERT(cond, msg) while (!(cond)) { RG_PANIC("Assertion failed: `" #cond "` : " msg); }

#ifndef RG_LOG_TAG
#define RG_LOG_TAG __func__
#endif

#define RG_LOGX(x, ...) rg_system_log(RG_LOG_PRINT, RG_LOG_TAG, x, ## __VA_ARGS__)
#define RG_LOGE(x, ...) rg_system_log(RG_LOG_ERROR, RG_LOG_TAG, x, ## __VA_ARGS__)
#define RG_LOGW(x, ...) rg_system_log(RG_LOG_WARN, RG_LOG_TAG, x, ## __VA_ARGS__)
#define RG_LOGI(x, ...) rg_system_log(RG_LOG_INFO, RG_LOG_TAG, x, ## __VA_ARGS__)
#define RG_LOGD(x, ...) rg_system_log(RG_LOG_DEBUG, RG_LOG_TAG, x, ## __VA_ARGS__)

#define RG_DUMP(...) {}

#define RG_PATH_MAX 255

// Attributes

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
