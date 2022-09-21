#include "SDL_system.h"
#include <unistd.h>

struct SDL_mutex
{
    pthread_mutex_t id;
    SemaphoreHandle_t handle;
#if FAKE_RECURSIVE_MUTEX
    int recursive;
    pthread_t owner;
#endif
};

void SDL_ClearError(void)
{

}

void SDL_Delay(Uint32 ms)
{
    usleep(ms * 1000);
}

char *SDL_GetError(void)
{
    return (char *)"";
}

int SDL_Init(Uint32 flags)
{
    if(flags == SDL_INIT_VIDEO)
        SDL_InitSubSystem(flags);
    return 0;
}

void SDL_Quit(void)
{

}

const SDL_version* SDL_Linked_Version()
{
    SDL_version *vers = malloc(sizeof(SDL_version));
    vers->major = SDL_MAJOR_VERSION;
    vers->minor = SDL_MINOR_VERSION;
    vers->patch = SDL_PATCHLEVEL;
    return vers;
}

char *** allocateTwoDimenArrayOnHeapUsingMalloc(int row, int col)
{
	char ***ptr = malloc(row * sizeof(*ptr) + row * (col * sizeof **ptr) );

	int * const data = ptr + row;
	for(int i = 0; i < row; i++)
		ptr[i] = data + i * col;

	return ptr;
}

void SDL_DestroyMutex(SDL_mutex* mutex)
{

}

SDL_mutex* SDL_CreateMutex(void)
{
    SDL_mutex* mut = malloc(sizeof(SDL_mutex));
    mut->handle = xSemaphoreCreateMutex();
    //mut->handle = xSemaphoreCreateBinary();
    return mut;
}

int SDL_LockMutex(SDL_mutex* mutex)
{
    xSemaphoreTake(mutex->handle, 1000 / portTICK_RATE_MS);
    return 0;
}

int SDL_UnlockMutex(SDL_mutex* mutex)
{
    xSemaphoreGive(mutex->handle);
    return 0;
}