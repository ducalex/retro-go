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

#include "odroid_audio.h"
#include "odroid_display.h"
#include "odroid_input.h"
#include "odroid_overlay.h"
#include "odroid_netplay.h"
#include "odroid_sdcard.h"
#include "odroid_settings.h"

// GPIO Allocations
#define ODROID_PIN_LED            GPIO_NUM_2
#define ODROID_PIN_DAC1           GPIO_NUM_25
#define ODROID_PIN_DAC2           GPIO_NUM_26
#define ODROID_PIN_I2S_DAC_BCK    GPIO_NUM_4
#define ODROID_PIN_I2S_DAC_WS     GPIO_NUM_12
#define ODROID_PIN_I2S_DAC_DATA   GPIO_NUM_15
#define ODROID_PIN_GAMEPAD_X      ADC1_CHANNEL_6
#define ODROID_PIN_GAMEPAD_Y      ADC1_CHANNEL_7
#define ODROID_PIN_GAMEPAD_SELECT GPIO_NUM_27
#define ODROID_PIN_GAMEPAD_START  GPIO_NUM_39
#define ODROID_PIN_GAMEPAD_A      GPIO_NUM_32
#define ODROID_PIN_GAMEPAD_B      GPIO_NUM_33
#define ODROID_PIN_GAMEPAD_MENU   GPIO_NUM_13
#define ODROID_PIN_GAMEPAD_VOLUME GPIO_NUM_0
#define ODROID_PIN_SPI_MISO       GPIO_NUM_19
#define ODROID_PIN_SPI_MOSI       GPIO_NUM_23
#define ODROID_PIN_SPI_CLK        GPIO_NUM_18
#define ODROID_PIN_LCD_MISO       ODROID_PIN_SPI_MISO
#define ODROID_PIN_LCD_MOSI       ODROID_PIN_SPI_MOSI
#define ODROID_PIN_LCD_CLK        ODROID_PIN_SPI_CLK
#define ODROID_PIN_LCD_CS         GPIO_NUM_5
#define ODROID_PIN_LCD_DC         GPIO_NUM_21
#define ODROID_PIN_LCD_BCKL       GPIO_NUM_14
#define ODROID_PIN_SD_MISO        ODROID_PIN_SPI_MISO
#define ODROID_PIN_SD_MOSI        ODROID_PIN_SPI_MOSI
#define ODROID_PIN_SD_CLK         ODROID_PIN_SPI_CLK
#define ODROID_PIN_SD_CS          GPIO_NUM_22
#define ODROID_PIN_NES_LATCH      GPIO_NUM_15
#define ODROID_PIN_NES_CLOCK      GPIO_NUM_12
#define ODROID_PIN_NES_DATA       GPIO_NUM_4

// SD Card Paths
#define ODROID_BASE_PATH           "/sd"
#define ODROID_BASE_PATH_ROMS      ODROID_BASE_PATH "/roms"
#define ODROID_BASE_PATH_SAVES     ODROID_BASE_PATH "/odroid/data"
#define ODROID_BASE_PATH_TEMP      ODROID_BASE_PATH "/odroid/data" // temp
#define ODROID_BASE_PATH_ROMART    ODROID_BASE_PATH "/romart"
#define ODROID_BASE_PATH_CRC_CACHE ODROID_BASE_PATH "/odroid/cache/crc"

typedef bool (*state_handler_t)(char *pathName);

typedef struct
{
     char *romPath;
     char *startAction;
     int8_t speedup;
     state_handler_t load;
     state_handler_t save;
} emu_state_t;

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

void odroid_system_emu_init(state_handler_t load, state_handler_t save, netplay_callback_t netplay_cb);
bool odroid_system_emu_save_state(int slot);
bool odroid_system_emu_load_state(int slot);
void odroid_system_init(int app_id, int sampleRate);
uint odroid_system_get_app_id();
void odroid_system_set_app_id(int appId);
uint odroid_system_get_game_id();
uint odroid_system_get_start_action();
const char* odroid_system_get_rom_path();
char* odroid_system_get_path(emu_path_type_t type, const char *romPath);
void odroid_system_panic_dialog(const char *reason);
void odroid_system_panic(const char *reason, const char *file, const char *function) __attribute__((noreturn));
void odroid_system_halt() __attribute__((noreturn));
void odroid_system_sleep();
void odroid_system_switch_app(int app) __attribute__((noreturn));
void odroid_system_reload_app() __attribute__((noreturn));
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

void rg_perf_init(void);
void rg_perf_reset(void);
void rg_perf_print(void);
void rg_perf_func_enter(void *func_ptr, char *func_name);
void rg_perf_func_leave(void);
