#ifndef esp32_compat_h_
#define esp32_compat_h_
    
    #include "stdint.h"
    #include <assert.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <time.h>
    #include <fcntl.h>
    #include <sys/stat.h>
    //#include <sys/types.h>
    #include <sys/uio.h>
    #include <unistd.h>
    #include <sys/dirent.h>
    #define HAVE_ASSERT_H 1
    #define PLATFORM_SUPPORTS_SDL
    #define kkfree(x) free(x)
    #define kkmalloc(x) malloc(x)
    #define kmalloc(x) malloc(x)
    #define strcmpi(x, y) strcasecmp(x, y)
    #define stricmp strcasecmp
    #define __int64 int64_t
    #define min(x,y) (((x) < (y)) ? (x) : (y))
    #define max(x,y) (((x) > (y)) ? (x) : (y))
    #define FP_OFF(x) ((int32_t) (x))
    #define O_BINARY 0

    #define BYTE_ORDER 1234
    #define HAVE_ENET_ENET_H 1
    #define UDP_NETWORKING 1

    #define	SOL_SOCKET	0xffff
    #define SOL_IP SOL_SOCKET

    #define	SO_BROADCAST	0x0020
    #define IP_RECVERR  SO_BROADCAST

    #define USER_DUMMY_NETWORK 1

    #define STUBBED(x) fprintf(stderr,"STUB: %s (%s, %s:%d)\n",x,__FUNCTION__,__FILE__,__LINE__)

    #define getch getchar
    #define printchrasm(x,y,ch) printf("%c", (uint8_t ) (ch & 0xFF))

    #define Z_AvailHeap() ((4 * 1024) * 1024)

    #define mkdir(X) mkdir(X,0777)
    //#define PLATFORM_UNIX 1
    #define PLATFORM_ESP32 1

    #define MAX_PATH 255

    struct find_t
    {
        DIR *dir;
        char  pattern[MAX_PATH];
        char  name[MAX_PATH];
    };
    int _dos_findfirst(char  *filename, int x, struct find_t *f);
    int _dos_findnext(struct find_t *f);

    struct dosdate_t
    {
        uint8_t  day;
        uint8_t  month;
        unsigned int year;
        uint8_t  dayofweek;
    };

    void _dos_getdate(struct dosdate_t *date);

    #define PATH_SEP_CHAR '/'
    #define PATH_SEP_STR  "/"
    #define ROOTDIR       "/"
    #define CURDIR        "./"
#endif