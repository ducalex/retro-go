#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "config.h"

#ifndef RG_TARGET_SDL2
#include <esp_idf_version.h>
#include <esp_attr.h>
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(4, 3, 0)
#define SPI_DMA_CH_AUTO 1
#endif
#else
#define EXT_RAM_ATTR
#define IRAM_ATTR
#define RTC_NOINIT_ATTR
#endif

#include "rg_audio.h"
#include "rg_display.h"
#include "rg_input.h"
#include "rg_storage.h"
#include "rg_settings.h"
#include "rg_network.h"
#include "rg_gui.h"
#include "rg_i2c.h"
#include "rg_printf.h"
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
    RG_OK = 0,
    RG_FAIL,
    RG_NOMEM,
} rg_err_t;

typedef enum
{
    RG_RST_POWERON = 0, // Cold boot
    RG_RST_RESTART,     // Warm boot
    RG_RST_PANIC,       // Crash
} rg_reset_reason_t;

typedef enum
{
    RG_EVENT_TYPE_SYSTEM  = 0xF1000000,
    RG_EVENT_TYPE_POWER   = 0xF2000000,
    RG_EVENT_TYPE_NETWORK = 0xF3000000,
    RG_EVENT_TYPE_NETPLAY = 0xF4000000,
    RG_EVENT_TYPE_MASK    = 0xFF000000,
} rg_event_type_t;

enum
{
    RG_EVENT_UNRESPONSIVE = RG_EVENT_TYPE_SYSTEM | 1,
    RG_EVENT_LOWMEMORY    = RG_EVENT_TYPE_SYSTEM | 2,
    RG_EVENT_REDRAW       = RG_EVENT_TYPE_SYSTEM | 3,
    RG_EVENT_SHUTDOWN     = RG_EVENT_TYPE_POWER | 1,
    RG_EVENT_SLEEP        = RG_EVENT_TYPE_POWER | 2,
};

typedef bool (*rg_state_handler_t)(const char *filename);
typedef bool (*rg_reset_handler_t)(bool hard);
typedef void (*rg_event_handler_t)(int event, void *data);
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
    rg_mem_read_handler_t memRead;      // Used by for cheats and debugging
    rg_mem_write_handler_t memWrite;    // Used by for cheats and debugging
} rg_handlers_t;

typedef struct
{
    uint8_t id;
    bool exists;
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
} rg_emu_state_t;

typedef struct
{
    const char *name;
    const char *version;
    const char *buildDate;
    const char *buildTime;
    const char *buildUser;
    const char *toolchain;
    const char *configNs;
    const char *bootArgs;
    uint32_t bootFlags;
    uint32_t bootType;
    float speed;
    int refreshRate;
    int sampleRate;
    int logLevel;
    int isLauncher;
    int saveSlot;
    const char *romPath;
    const rg_gui_option_t *options;
    rg_handlers_t handlers;
    bool initialized;
} rg_app_t;

typedef struct
{
    float skippedFPS;
    float fullFPS;
    float totalFPS;
    float busyPercent;
    int64_t busyTime;
    int ticks;
    int totalMemoryInt;
    int totalMemoryExt;
    int freeMemoryInt;
    int freeMemoryExt;
    int freeBlockInt;
    int freeBlockExt;
    int freeStackMain;
} rg_stats_t;

rg_app_t *rg_system_init(int sampleRate, const rg_handlers_t *handlers, const rg_gui_option_t *options);
rg_app_t *rg_system_reinit(int sampleRate, const rg_handlers_t *handlers, const rg_gui_option_t *options);
void rg_system_panic(const char *context, const char *message) __attribute__((noreturn));
void rg_system_shutdown(void) __attribute__((noreturn));
void rg_system_sleep(void) __attribute__((noreturn));
void rg_system_restart(void) __attribute__((noreturn));
void rg_system_switch_app(const char *part, const char *name, const char *args, uint32_t flags) __attribute__((noreturn));
bool rg_system_have_app(const char *app);
void rg_system_set_led(int value);
int  rg_system_get_led(void);
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
void rg_system_load_time(void);
void rg_system_save_time(void);

// Wrappers for the OS' task/thread creation API. It also keeps track of handles for debugging purposes...
bool rg_task_create(const char *name, void (*taskFunc)(void *data), void *data, size_t stackSize, int priority, int affinity);
bool rg_task_delete(const char *name);
void rg_task_delay(int ms);

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

// #define gpio_set_level(num, level) (((num) & I2C) ? rg_gpio_set_level((num) & ~I2C) : (gpio_set_level)(num, level) == ESP_OK)
// #define gpio_get_level(num, level) (((num) & I2C) ? rg_gpio_set_level((num) & ~I2C) : (gpio_get_level)(num, level))
// #define gpio_set_direction(num, mode) (((num) & I2C) ? rg_gpio_set_direction((num) & ~I2C) : (gpio_set_direction)(num, level) == ESP_OK)

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

void __cyg_profile_func_enter(void *this_fn, void *call_site);
void __cyg_profile_func_exit(void *this_fn, void *call_site);
#define NO_PROFILE __attribute((no_instrument_function))

#ifdef __cplusplus
}
#endif
