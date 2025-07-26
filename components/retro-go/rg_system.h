#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "config.h"

#ifdef ESP_PLATFORM
#include <esp_idf_version.h>
#include <esp_heap_caps.h>
#include <esp_attr.h>
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(4, 4, 0)
#error "Retro-Go requires ESP-IDF version 4.4.0 or newer!"
#endif
#else
#define IRAM_ATTR
#define RTC_NOINIT_ATTR
#endif

#include "rg_localization.h"
#include "rg_audio.h"
#include "rg_display.h"
#include "rg_input.h"
#include "rg_storage.h"
#include "rg_settings.h"
#include "rg_network.h"
#include "rg_gui.h"
#include "rg_i2c.h"
#include "rg_utils.h"

#ifdef RG_ENABLE_NETPLAY
#include "rg_netplay.h"
#endif

typedef enum
{
    RG_PATH_SAVE_STATE = 0x100,
    RG_PATH_SAVE_SRAM  = 0x200,
    RG_PATH_SCREENSHOT = 0x300,
    RG_PATH_ROM_FILE   = 0x400,
    RG_PATH_CACHE_FILE = 0x500,
    RG_PATH_GAME_CONFIG= 0x600,
} rg_path_type_t;

typedef enum
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
} rg_boot_flags_t;

// RG_TASK_PRIORITY_1 is the same as the main task's. Anything
// higher will run even if main task never yields
typedef enum
{
    RG_TASK_PRIORITY_1 = 1,
    RG_TASK_PRIORITY_2,
    RG_TASK_PRIORITY_3,
    RG_TASK_PRIORITY_4,
    RG_TASK_PRIORITY_5,
    RG_TASK_PRIORITY_6,
    RG_TASK_PRIORITY_7,
    RG_TASK_PRIORITY_8,
} rg_task_priority_t;

typedef enum
{
    RG_LOG_PRINTF = 0,
    RG_LOG_ERROR,
    RG_LOG_WARN,
    RG_LOG_INFO,
    RG_LOG_DEBUG,
    RG_LOG_VERBOSE,
    RG_LOG_MAX,
} rg_log_level_t;

typedef enum
{
    // 0-3: Power indicators
    RG_INDICATOR_POWER_NORMAL = 0,
    RG_INDICATOR_POWER_LOW,
    RG_INDICATOR_POWER_CHARGING,
    // 4-15: Activity indicators
    RG_INDICATOR_ACTIVITY_DISK = 4,
    RG_INDICATOR_ACTIVITY_NETWORK,
    RG_INDICATOR_ACTIVITY_SYSTEM,
    RG_INDICATOR_ACTIVITY_USER,
    // 30-31: Critical indicators
    RG_INDICATOR_CRITICAL = 30,
} rg_indicator_t;

typedef enum
{
    /* Types and masks */
    RG_EVENT_TYPE_SYSTEM  = 0xF10000,
    RG_EVENT_TYPE_POWER   = 0xF20000,
    RG_EVENT_TYPE_NETWORK = 0xF30000,
    RG_EVENT_TYPE_NETPLAY = 0xF40000,
    RG_EVENT_TYPE_MENU    = 0xF50000,
    RG_EVENT_TYPE_MASK    = 0xFF0000,

    /* Events */
    RG_EVENT_UNRESPONSIVE = RG_EVENT_TYPE_SYSTEM | 1,
    RG_EVENT_LOWMEMORY    = RG_EVENT_TYPE_SYSTEM | 2,
    RG_EVENT_REDRAW       = RG_EVENT_TYPE_SYSTEM | 3,
    RG_EVENT_SPEEDUP      = RG_EVENT_TYPE_SYSTEM | 4,
    RG_EVENT_SCREENSHOT   = RG_EVENT_TYPE_SYSTEM | 5,
    RG_EVENT_SHUTDOWN     = RG_EVENT_TYPE_POWER | 1,
    RG_EVENT_SLEEP        = RG_EVENT_TYPE_POWER | 2,
} rg_event_t;

typedef struct
{
    bool (*loadState)(const char *filename);                         // rg_emu_load_state() handler
    bool (*saveState)(const char *filename);                         // rg_emu_save_state() handler
    bool (*reset)(bool hard);                                        // rg_emu_reset() handler
    bool (*screenshot)(const char *filename, int width, int height); // rg_emu_screenshot() handler
    void (*event)(int event, void *data);                            // listen to retro-go system events
    int (*memRead)(int addr);                                        // Used by for cheats and debugging
    int (*memWrite)(int addr, int value);                            // Used by for cheats and debugging
    void (*options)(rg_gui_option_t *dest);                          // Add extra options to rg_gui_options_menu()
    void (*about)(rg_gui_option_t *dest);                            // Add extra options to rg_gui_about_menu()
} rg_handlers_t;

typedef struct
{
    uint8_t id;
    bool is_used;
    bool is_lastused;
    size_t size;
    time_t mtime;
    char preview[RG_PATH_MAX];
    char file[RG_PATH_MAX];
} rg_emu_slot_t;

typedef struct
{
    size_t total;
    size_t used;
    rg_emu_slot_t *lastused;
    rg_emu_slot_t *latest;
    rg_emu_slot_t slots[];
} rg_emu_states_t;

typedef struct
{
    const char *name;
    const char *version;
    const char *buildDate;
    const char *buildInfo;
    const char *configNs;
    const char *bootArgs;
    uint32_t bootFlags;
    uint32_t indicatorsMask;
    float speed;
    int sampleRate;
    int tickRate;
    int frameTime;
    int frameskip;
    int overclock;
    int tickTimeout;
    bool lowMemoryMode;
    bool enWatchdog;
    bool isColdBoot;
    bool isLauncher;
    // bool isOfficial;
    bool isRelease;
    int logLevel;
    int saveSlot;
    const char *romPath;
    rg_handlers_t handlers;
    bool initialized;
} rg_app_t;

typedef struct
{
    float skippedFPS;
    float partialFPS;
    float fullFPS;
    float totalFPS;
    float busyPercent;
    int64_t busyTime;
    int64_t lastTick;
    int ticks;
    int uptime;
    int totalMemoryInt;
    int totalMemoryExt;
    int freeMemoryInt;
    int freeMemoryExt;
    int freeBlockInt;
    int freeBlockExt;
    int freeStackMain;
} rg_stats_t;

rg_app_t *rg_system_init(int sampleRate, const rg_handlers_t *handlers, void *_unused);
rg_app_t *rg_system_reinit(int sampleRate, const rg_handlers_t *handlers, void *_unused);
void rg_system_panic(const char *context, const char *message) __attribute__((noreturn));
void rg_system_shutdown(void) __attribute__((noreturn));
void rg_system_sleep(void) __attribute__((noreturn));
void rg_system_restart(void) __attribute__((noreturn));
void rg_system_exit(void) __attribute__((noreturn));
void rg_system_switch_app(const char *part, const char *name, const char *args, uint32_t flags) __attribute__((noreturn));
bool rg_system_have_app(const char *app);
void rg_system_set_indicator(rg_indicator_t indicator, bool on);
bool rg_system_get_indicator(rg_indicator_t indicator);
void rg_system_set_indicator_mask(rg_indicator_t indicator, bool on);
bool rg_system_get_indicator_mask(rg_indicator_t indicator);
void rg_system_set_tick_rate(int tickRate);
int rg_system_get_tick_rate(void);
void rg_system_set_overclock(int level);
int  rg_system_get_overclock(void);
void rg_system_set_log_level(rg_log_level_t level);
int  rg_system_get_log_level(void);
void rg_system_tick(int busyTime);
void rg_system_vlog(int level, const char *context, const char *format, va_list va);
void rg_system_log(int level, const char *context, const char *format, ...) __attribute__((format(printf,3,4)));
bool rg_system_save_trace(const char *filename, bool append);
void rg_system_event(int event, void *data);
int64_t rg_system_timer(void);
rg_app_t *rg_system_get_app(void);
rg_stats_t rg_system_get_counters(void);

// RTC and time-related functions
void rg_system_set_timezone(const char *TZ);
char *rg_system_get_timezone(void);
void rg_system_load_time(void);
void rg_system_save_time(void);

// Wrappers for the OS' task/thread creation API. It also keeps track of handles for debugging purposes...
typedef struct rg_task_s rg_task_t;
typedef struct
{
    int type; // Negative values are reserved
    union
    {
        const void *dataPtr;
        uint32_t dataInt;
    };
} rg_task_msg_t;
#define RG_TASK_MSG_STOP -1
rg_task_t *rg_task_create(const char *name, void (*taskFunc)(void *arg), void *arg, size_t stackSize, int priority, int affinity);
rg_task_t *rg_task_find(const char *name);
rg_task_t *rg_task_current(void);
bool rg_task_send(rg_task_t *task, const rg_task_msg_t *msg);
bool rg_task_peek(rg_task_msg_t *out);
bool rg_task_receive(rg_task_msg_t *out);
bool rg_task_is_blocked(rg_task_t *task);
size_t rg_task_messages_waiting(rg_task_t *task);
// The main difference between rg_task_delay and rg_usleep is that rg_task_delay will yield
// to other tasks and will not busy wait time smaller than a tick. Meaning rg_usleep
// is more accurate but rg_task_delay is more multitasking-friendly.
void rg_task_delay(uint32_t ms);
void rg_task_yield(void);

typedef void rg_mutex_t;
rg_mutex_t *rg_mutex_create(void);
void rg_mutex_free(rg_mutex_t *mutex);
bool rg_mutex_give(rg_mutex_t *mutex);
bool rg_mutex_take(rg_mutex_t *mutex, int timeoutMS);

char *rg_emu_get_path(rg_path_type_t type, const char *arg);
bool rg_emu_save_state(uint8_t slot);
bool rg_emu_load_state(uint8_t slot);
bool rg_emu_reset(bool hard);
bool rg_emu_screenshot(const char *filename, int width, int height);
rg_emu_states_t *rg_emu_get_states(const char *romPath, size_t slots);
uint8_t rg_emu_get_last_used_slot(const char *romPath);
void rg_emu_set_speed(float speed);
float rg_emu_get_speed(void);

/* Utilities */

// #define gpio_set_level(num, level) (((num) & I2C) ? rg_gpio_set_level((num) & ~I2C) : (gpio_set_level)(num, level) == ESP_OK)
// #define gpio_get_level(num, level) (((num) & I2C) ? rg_gpio_set_level((num) & ~I2C) : (gpio_get_level)(num, level))
// #define gpio_set_direction(num, mode) (((num) & I2C) ? rg_gpio_set_direction((num) & ~I2C) : (gpio_set_direction)(num, level) == ESP_OK)

// This should really support printf format...
#define RG_PANIC(x) rg_system_panic(__func__, x)
#define RG_ASSERT(cond, msg) while (!(cond)) { RG_PANIC("Assertion failed: `" #cond "` : " msg); }
#define RG_ASSERT_ARG(cond) while (!(cond)) { RG_PANIC("Invalid function argument"); }

#ifndef RG_LOG_TAG
#define RG_LOG_TAG __func__
#endif

#define RG_LOGE(x, ...) rg_system_log(RG_LOG_ERROR, RG_LOG_TAG, x, ## __VA_ARGS__)
#define RG_LOGW(x, ...) rg_system_log(RG_LOG_WARN, RG_LOG_TAG, x, ## __VA_ARGS__)
#define RG_LOGI(x, ...) rg_system_log(RG_LOG_INFO, RG_LOG_TAG, x, ## __VA_ARGS__)
#if RG_BUILD_RELEASE
#define RG_LOGD(x, ...)
#define RG_LOGV(x, ...)
#else
#define RG_LOGD(x, ...) rg_system_log(RG_LOG_DEBUG, RG_LOG_TAG, x, ## __VA_ARGS__)
#define RG_LOGV(x, ...) rg_system_log(RG_LOG_VERBOSE, RG_LOG_TAG, x, ## __VA_ARGS__)
#endif

#ifdef RG_ENABLE_PROFILING
void __cyg_profile_func_enter(void *this_fn, void *call_site);
void __cyg_profile_func_exit(void *this_fn, void *call_site);
#define NO_PROFILE __attribute((no_instrument_function))
#else
#define NO_PROFILE
#endif

#ifdef __cplusplus
}
#endif
