#include "SDL_event.h"
#include <rg_system.h>
#include <string.h>

int keyMode = 1;

Uint8 SDL_EventState(Uint32 type, int state)
{
    return 0;
}

extern int SDL_RG_PollEvent(SDL_Event *event);

int SDL_PollEvent(SDL_Event *event)
{
    return SDL_RG_PollEvent(event);
}

int SDL_WaitEvent(SDL_Event *event)
{
    while (!SDL_PollEvent(event)) {
        rg_task_yield();
    }
    return 1;
}

Uint32 SDL_GetMouseState(int *x, int *y)
{
    if (x) *x = 0;
    if (y) *y = 0;
    return 0;
}

int SDL_ShowCursor(int toggle)
{
    return 0;
}
