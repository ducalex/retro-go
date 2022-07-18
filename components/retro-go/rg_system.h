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

#define RG_APP_LAUNCHER "launcher"
#define RG_APP_FACTORY  NULL

#define RG_PATH_MAX 255

#include "rg_audio.h"
#include "rg_display.h"
#include "rg_input.h"
#include "rg_netplay.h"
#include "rg_storage.h"
#include "rg_settings.h"
#include "rg_kvs.h"
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
{   // bits 0-3: Mode
    RG_BOOT_NORMAL    = 0x00,
    RG_BOOT_RESUME    = 0x01,
    RG_BOOT_ONCE      = 0x02,
    RG_BOOT_RESET     = 0x04,
    RG_BOOT_NETPLAY   = 0x08,
    RG_BOOT_MODE_MASK = 0x0F,
    // bits 4-7: slot
    RG_BOOT_SLOT0     = 0x00,
    RG_BOOT_SLOT1     = 0x10,
    RG_BOOT_SLOT2     = 0x20,
    RG_BOOT_SLOT3     = 0x30,
    RG_BOOT_SLOT_MASK = 0xF0,
    // bits 8-31: unused...
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

typedef struct
{
    char preview[RG_PATH_MAX];
    char file[RG_PATH_MAX];
    time_t mtime;
    bool exists;
    bool latest;
} rg_emu_state_t;

// TO DO: Make it an abstract ring buffer implementation?
#define RG_LOGBUF_SIZE 2048
typedef struct
{
    char buffer[RG_LOGBUF_SIZE];
    size_t cursor;
} rg_logbuf_t;

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
    float speed;
    int refreshRate;
    int sampleRate;
    int logLevel;
    int isLauncher;
    int saveSlot;
    const char *romPath;
    void *mainTaskHandle;
    const rg_gui_option_t *options;
    rg_handlers_t handlers;
} rg_app_t;

typedef struct
{
    float batteryPercent;
    float batteryVoltage;
    float skippedFPS;
    float fullFPS;
    float totalFPS;
    float busyPercent;
    uint32_t totalFrames;
    uint32_t busyTime;
    uint32_t ticks;
    uint32_t totalMemoryInt;
    uint32_t totalMemoryExt;
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
int64_t rg_system_timer(void);
rg_app_t *rg_system_get_app(void);
rg_stats_t rg_system_get_stats(void);

char *rg_emu_get_path(rg_path_type_t type, const char *arg);
bool rg_emu_save_state(uint8_t slot);
bool rg_emu_load_state(uint8_t slot);
bool rg_emu_reset(bool hard);
bool rg_emu_screenshot(const char *filename, int width, int height);
rg_emu_state_t *rg_emu_get_states(const char *romPath, size_t slots);

void *rg_alloc(size_t size, uint32_t caps);

#define MEM_ANY   (0)
#define MEM_SLOW  (1)
#define MEM_FAST  (2)
#define MEM_DMA   (4)
#define MEM_8BIT  (8)
#define MEM_32BIT (16)
#define MEM_EXEC  (32)

#define PTR_IN_SPIRAM(ptr) ((void*)(ptr) >= (void*)0x3F800000 && (void*)(ptr) < (void*)0x3FC00000)

/* Utilities */

// Functions from esp-idf, we don't include their header but they will be linked
extern uint32_t crc32_le(uint32_t crc, const uint8_t * buf, uint32_t len);

// These older macros do not guarantee a time unit in case retro-go was ported to a platform where
// divisions were too slow or overflows were likely. However at this time it doesn't matter and much
// of the code assumes microseconds so I will replace them by rg_system_timer() for clarity.
#define get_frame_time(refresh_rate) (1000000 / (refresh_rate))
#define get_elapsed_time() (rg_system_timer())
#define get_elapsed_time_since(start) (get_elapsed_time() - (start))

#define RG_TIMER_INIT() int64_t _rgts_ = rg_system_timer(), _rgtl_ = rg_system_timer();
#define RG_TIMER_LAP(name) \
    RG_LOGX("Lap %s: %.2f   Total: %.2f\n", #name, (rg_system_timer() - _rgtl_) / 1000.f, \
            (rg_system_timer() - _rgts_) / 1000.f); _rgtl_ = rg_system_timer();

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
