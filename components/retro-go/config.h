#if defined(RG_TARGET_ODROID_GO)
#include "targets/odroid-go/config.h"
#elif defined(RG_TARGET_MRGC_G32)
#include "targets/mrgc-g32/config.h"
#elif defined(RG_TARGET_QTPY_GAMER)
#include "targets/qtpy-gamer/config.h"
#elif defined(RG_TARGET_RETRO_ESP32)
#include "targets/retro-esp32/config.h"
#elif defined(RG_TARGET_SDL2)
#include "targets/sdl2/config.h"
#elif defined(RG_TARGET_MRGC_GBM)
#include "targets/mrgc-gbm/config.h"
#elif defined(RG_TARGET_ESPLAY_MICRO)
#include "targets/esplay-micro/config.h"
#elif defined(RG_TARGET_ESPLAY_S3)
#include "targets/esplay-s3/config.h"
#elif defined(RG_TARGET_ESP32S3_DEVKIT_C)
#include "targets/esp32s3-devkit-c/config.h"
#elif defined(RG_TARGET_FRI3D_2024)
#include "targets/fri3d-2024/config.h"
#else
#warning "No target defined. Defaulting to ODROID-GO."
#include "targets/odroid-go/config.h"
#define RG_TARGET_ODROID_GO
#endif

// #ifndef RG_ENABLE_NETPLAY
// #define RG_ENABLE_NETPLAY 0
// #endif

// #ifndef RG_ENABLE_PROFILING
// #define RG_ENABLE_PROFILING 0
// #endif

#ifndef RG_APP_LAUNCHER
#define RG_APP_LAUNCHER "launcher"
#endif

#ifndef RG_APP_FACTORY
#define RG_APP_FACTORY NULL
#endif

#ifndef RG_APP_UPDATER
#define RG_APP_UPDATER RG_APP_FACTORY
// #define RG_APP_UPDATER "updater"
#endif

#ifndef RG_PATH_MAX
#define RG_PATH_MAX 255
#endif

#ifndef RG_PROJECT_NAME
#define RG_PROJECT_NAME "Retro-Go"
#endif

#ifndef RG_PROJECT_APP
#define RG_PROJECT_APP "unknown"
#endif

#ifndef RG_PROJECT_VERSION
#define RG_PROJECT_VERSION RG_BUILD_VERSION
#endif

#ifndef RG_PROJECT_CREDITS
#define RG_PROJECT_CREDITS \
    "Retro-Go: ducalex\n"
    // TODO: Decide which additional credits should be included here?
    // Maybe the main author for each emulator? Or should that credit be only when seeing `about` inside said app?
    // What about libraries and fonts (lodepng, etc)?
    // What about targets/ports? Should we have a RG_TARGET_AUTHOR to append here?
#endif

#ifndef RG_PROJECT_WEBSITE
#define RG_PROJECT_WEBSITE "https://github.com/ducalex/retro-go"
#endif

#ifndef RG_PROJECT_RELEASES_URL
#define RG_PROJECT_RELEASES_URL "https://api.github.com/repos/ducalex/retro-go/releases"
#endif

#ifndef RG_BUILD_VERSION
#define RG_BUILD_VERSION "unknown"
#endif

#ifndef RG_BUILD_TIME
// 2020-01-31 00:00:00, first retro-go commit :)
#define RG_BUILD_TIME 1580446800
#endif

#ifndef RG_BUILD_DATE
#define RG_BUILD_DATE __DATE__ " " __TIME__
#endif

#ifndef RG_BUILD_TOOL
#define RG_BUILD_TOOL "unknown"
#endif

#ifndef RG_RECOVERY_BTN
#define RG_RECOVERY_BTN RG_KEY_ANY
#endif

#ifndef RG_BATTERY_CALC_PERCENT
#define RG_BATTERY_CALC_PERCENT(raw) (100)
#endif

#ifndef RG_BATTERY_CALC_VOLTAGE
#define RG_BATTERY_CALC_VOLTAGE(raw) (0)
#endif

#ifndef RG_BATTERY_UPDATE_THRESHOLD
#define RG_BATTERY_UPDATE_THRESHOLD 1.0f
#endif

#ifndef RG_BATTERY_UPDATE_THRESHOLD_VOLT
#define RG_BATTERY_UPDATE_THRESHOLD_VOLT 0.010f
#endif

#ifndef RG_LOG_COLORS
#define RG_LOG_COLORS (1)
#endif

#ifndef RG_TICK_RATE
#ifdef ESP_PLATFORM
#define RG_TICK_RATE CONFIG_FREERTOS_HZ
#else
#define RG_TICK_RATE 1000
#endif
#endif

#ifdef ESP_PLATFORM
#define RG_ZIP_SUPPORT 1
#else
#define RG_ZIP_SUPPORT 0
#endif
