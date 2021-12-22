/* Uncomment this to exhaustively run memory checks while the game is running
   (this is EXTREMELY slow). */
/* #undef CHECKHEAP */

/* Define for support for MBF helper dogs */
//#define DOGS 0

/* Uncomment this to cause heap dumps to be generated. Only useful if
   INSTRUMENTED is also defined. */
/* #undef HEAPDUMP */

/* Define on targets supporting 386 assembly */
/* #undef I386_ASM */

/* Define this to see real-time memory allocation statistics, and enable extra
   debugging features */
/* #undef INSTRUMENTED */

/* If your platform has a fast version of max, define MAX to it */
/* #undef MAX */

/* If your platform has a fast version of min, define MIN to it */
/* #undef MIN */

/* Name of package */
#define PACKAGE "prboom-go"

/* Set to the attribute to apply to struct definitions to make them packed */
#define PACKEDATTR __attribute__((packed))

/* Define to enable internal range checking */
/* #undef RANGECHECK */

/* When defined this causes quick checks which only impose significant
   overhead if a posible error is detected. */
#define SIMPLECHECKS

/* Defining this causes time stamps to be created each time a lump is locked,
   and lumps locked for long periods of time are reported */
/* #undef TIMEDIAG */

/* Version number of package */
#define VERSION "2.5.0"

/* Define to remove DEH support */
#define NODEHSUPPORT

/* Define to remove true color support (keep 8bit palette only) */
#define NOTRUECOLOR

/* Define to bundle prboom.wad (minus the trig tables, which we always include) */
#define PRBOOMWAD
