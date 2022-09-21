#ifndef SDL_system_h_
#define SDL_system_h_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "SDL_stdinc.h"
#include "SDL.h"

#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"

typedef struct {
    Uint8 major;
    Uint8 minor;
    Uint8 patch;
} SDL_version;

#define SDL_MAJOR_VERSION   2
#define SDL_MINOR_VERSION   0
#define SDL_PATCHLEVEL      9

#define SDL_VERSION(x)                          \
{                                   \
    (x)->major = SDL_MAJOR_VERSION;                 \
    (x)->minor = SDL_MINOR_VERSION;                 \
    (x)->patch = SDL_PATCHLEVEL;                    \
}
const SDL_version* SDL_Linked_Version();

int SDL_Init(Uint32 flags);
void SDL_Quit(void);

void SDL_Delay(Uint32 ms);
void SDL_ClearError(void);
char *SDL_GetError(void);

struct SDL_mutex;
typedef struct SDL_mutex SDL_mutex;

char *** allocateTwoDimenArrayOnHeapUsingMalloc(int row, int col);

#define SDL_mutexP(mutex) SDL_LockMutex(mutex)
#define SDL_mutexV(mutex) SDL_UnlockMutex(mutex)

void SDL_DestroyMutex(SDL_mutex* mutex);
SDL_mutex* SDL_CreateMutex(void);
int SDL_LockMutex(SDL_mutex* mutex);
int SDL_UnlockMutex(SDL_mutex* mutex);

#endif