/* Trace settings for debug purposes */
#define ENABLE_TRACING 0
#define ENABLE_TRACING_AUDIO 0
#define ENABLE_TRACING_GFX 0
#define ENABLE_TRACING_DEEP_GFX 0
#define ENABLE_TRACING_SPRITE 0
#define ENABLE_TRACING_SND 0

/* defined if user wants netplay support */
/* #undef ENABLE_NETPLAY */

//#define OPCODE_LOGGING 1
#undef OPCODE_LOGGING

/* defined if user wants a 'clean' binary ( = not for hugo developpers) */
#define FINAL_RELEASE 1
//#undef FINAL_RELEASE

/* for hugo developers working on the input subsystem */
#undef INPUT_DEBUG

/* for hugo developers working on hu6280 emulation */
#undef KERNEL_DEBUG
//#define KERNEL_DEBUG

/* for hugo developers working on netplay emulation */
#undef NETPLAY_DEBUG

/* Define to empty if `const' does not conform to ANSI C. */
// #undef const

#define PCE_PATH_MAX 192

#define INLINE static inline __attribute__((__always_inline__))
