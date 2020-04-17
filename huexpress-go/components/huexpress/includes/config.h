/* modified bsd pce cd hardware support (old one) */
#define BSD_CD_HARDWARE_SUPPORT 1

/* Trace settings for debug purposes */
#define ENABLE_TRACING 0
#define ENABLE_TRACING_AUDIO 0
#define ENABLE_TRACING_BIOS 0
#define ENABLE_TRACING_CD 0
#define ENABLE_TRACING_CD_2 0
#define ENABLE_TRACING_GFX 0
#define ENABLE_TRACING_SDL 0
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

/* defined if inlined accessors should be used */
#undef INLINED_ACCESSORS

/* for hugo developers working on the input subsystem */
#undef INPUT_DEBUG

/* for hugo developers working on hu6280 emulation */
#undef KERNEL_DEBUG
//#define KERNEL_DEBUG

/* for hugo developers working on netplay emulation */
#undef NETPLAY_DEBUG

/* Name of package */
#undef PACKAGE

/* Define to the address where bug reports for this package should be sent. */
#undef PACKAGE_BUGREPORT

/* directory in which data are located */
#undef PACKAGE_DATA_DIR

/* Define to the full name of this package. */
#undef PACKAGE_NAME

/* Define to the full name and version of this package. */
#undef PACKAGE_STRING

/* Define to the one symbol short name of this package. */
#undef PACKAGE_TARNAME

/* defined if user wants SDL as library */
//#define SDL 1

/* defined if user wants SDL_mixer as library */
//#define SDL_mixer 1

/* defined if user wants open memory openness */
#undef SHARED_MEMORY

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* defined if rom are expected to be patched to run from bank 0x68 */
#undef TEST_ROM_RELOCATED

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
#if defined(__POWERPC__)
#define WORDS_BIGENDIAN 1
#else
#undef WORDS_BIGENDIAN
#endif
// #define WORDS_BIGENDIAN 1

/* Define to empty if `const' does not conform to ANSI C. */
#undef const

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#undef inline
#endif
