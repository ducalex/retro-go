// Enable debugging messages (hardware faults, fix-me, etc)
#define DEBUG_ENABLED          0

// Enable low-level tracing
#define ENABLE_CPU_TRACING     0
#define ENABLE_GFX_TRACING     0
#define ENABLE_GFX2_TRACING    0
#define ENABLE_SPR_TRACING     0
#define ENABLE_SND_TRACING     0
#define ENABLE_IO_TRACING      0

#define USE_MEM_MACROS         1

#define PCE_PATH_MAX           192

#define INLINE static inline __attribute__((__always_inline__))
#define FAST_INLINE static inline __attribute__((__always_inline__)) // __attribute__((flatten))
