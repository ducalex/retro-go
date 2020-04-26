#pragma once

#include <odroid_system.h>
#include "esp_system.h"

#define ODROID_DEBUG_PERF_INIT
#define ODROID_DEBUG_PERF_START()
#define ODROID_DEBUG_PERF_START2(var_name)
#define ODROID_DEBUG_PERF_INCR(call)
#define ODROID_DEBUG_PERF_INCR2(var_name, call)
#define ODROID_DEBUG_PERF_LOG()

#define USE_INSTR_SWITCH // ***
#define MY_INLINE_h6280_opcodes //***
#define MY_INLINE // ***

extern uint skipFrames;
