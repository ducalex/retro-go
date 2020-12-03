#pragma once

#include <esp_attr.h>

// These two attributes are wrapped here to avoid any esp-specific
// code in the emulators.

#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

#ifndef DRAM_ATTR
#define DRAM_ATTR
#endif

// Retro-Go provided attributes

#ifndef ALWAYS_INLINE
#define ALWAYS_INLINE static inline __attribute__((always_inline))
#endif

#ifndef FLAT_INLINE
#define FLAT_INLINE static inline __attribute__((__always_inline__)) __attribute__((flatten))
#endif

#ifndef INLINE
#define INLINE static inline
#endif

#ifndef NO_PROFILING
#define NO_PROFILING __attribute((no_instrument_function))
#endif

#ifndef PACKED_STRUCT
#define PACKED_STRUCT __attribute__ ((__packed__)
#endif

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif
