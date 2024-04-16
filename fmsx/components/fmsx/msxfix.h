#ifndef _MSXFIX_H_
#define _MSXFIX_H_

#include <math.h>
#include <stdio.h>

// #define printf(x...) rg_system_log(RG_LOG_PRINT, NULL, x)

// Misc fixes
#define main(x, y) fmsx_main(x, y)
#define abs(x) __abs(x)
#define GenericSetVideo SetVideo

int fmsx_main(int argc, char *argv[]);


#ifdef ESP_PLATFORM
/**
 * esp-idf has no concept of a current working directory, which fMSX relies heavily on.
 * So we must emulate it. I've also added optimizations to bypass stat calls because
 * it makes using the built-in menu incredibly slow. It also bypass some fopen() only
 * during startup, to speed up boot.
 *
 * To make it work, add `-include msxfix.h` to fMSX's cflags
*/
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

DIR *msx_opendir(const char *);
struct dirent *msx_readdir(DIR *);
FILE *msx_fopen(const char *, const char *);
int msx_stat(const char *, struct stat *);
int msx_unlink(const char *);
int msx_rename(const char *, const char *);
int msx_chdir(const char *);
char *msx_getcwd(char *buf, size_t size);

#define chdir(x) msx_chdir(x)
#define getcwd(x, y) msx_getcwd(x, y)
#define opendir(x) msx_opendir(x)
#define readdir(x) msx_readdir(x)
#define fopen(x, y) msx_fopen(x, y)
#define stat(x, y) msx_stat(x, y)
#define unlink(x) msx_unlink(x)
#endif

#endif
