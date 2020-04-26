#pragma once

#include <odroid_system.h>
#include "esp_system.h"
#include "../../main/odroid_debug.h"

#define MY_EXCLUDE

//#define ODROID_DEBUG_PERF_CPU_ALL_INSTR

#define MY_INLINE_bank_set
#define MY_VDC_VARS // Pos
// -------------------------------------------------


#define USE_INSTR_SWITCH // ***
#define MY_INLINE_h6280_opcodes //***
#define MY_h6280_exe_go // ***
#define MY_INLINE // ***

//#define MY_PCENGINE_LOGGING
#define MY_LOG_CPU_NOT_INLINED // Slower without?!


extern uint skipFrames;
