#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <string.h>
#include <stdio.h>

#include "odroid_system.h"
#include "odroid_profiler.h"

// Note this profiler might be inaccurate because of:
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=28205

// We also need to add multi-core support. Currently it's not an issue
// because all our profiled code runs on the same core.

static profile_t *profile;
static SemaphoreHandle_t lock;
static bool enabled = false;

#define LOCK_PROFILE()   xSemaphoreTake(lock, pdMS_TO_TICKS(10000)); // {while(lock);lock=true;}
#define UNLOCK_PROFILE() xSemaphoreGive(lock);                       // {lock = false;}

NO_PROFILE static inline profile_frame_t *find_frame(void *this_fn, void *call_site)
{
    for (int i = 0; i < 512; ++i)
    {
        profile_frame_t *frame = &profile->frames[i];

        if (frame->func_ptr == 0)
        {
            profile->total_frames = MAX(profile->total_frames, i + 1);
            frame->func_ptr = this_fn;
            // frame->caller_ptr = call_site;
        }

        if (frame->func_ptr == this_fn) // && frame->caller_ptr == call_site)
        {
            return frame;
        }
    }

    RG_PANIC("Profile memory exhausted!");
}

NO_PROFILE void rg_profiler_init(void)
{
    printf("%s: Allocating profile...\n", __func__);
    profile = rg_alloc(sizeof(profile_t), MEM_SLOW);
    lock = xSemaphoreCreateMutex();
}

NO_PROFILE void rg_profiler_start(void)
{
    LOCK_PROFILE();

    memset(profile, 0, sizeof(profile_t));
    profile->time_started = get_elapsed_time();
    enabled = true;

    UNLOCK_PROFILE();
}

NO_PROFILE void rg_profiler_stop(void)
{
    enabled = false;
}

NO_PROFILE void rg_profiler_print(void)
{
    if (!profile)
        return;

    LOCK_PROFILE();

    printf("RGD:PROF:BEGIN %d %d\n", profile->total_frames, get_elapsed_time_since(profile->time_started));

    for (int i = 0; i < profile->total_frames; ++i)
    {
        profile_frame_t *frame = &profile->frames[i];

        printf(
            "RGD:PROF:DATA %p\t%p\t%u\t%u\n",
            frame->caller_ptr,
            frame->func_ptr,
            frame->num_calls,
            frame->run_time
        );
    }

    printf("RGD:PROF:END\n");

    UNLOCK_PROFILE();
}

NO_PROFILE void rg_profiler_push(char *section_name)
{

}

NO_PROFILE void rg_profiler_pop(void)
{

}

NO_PROFILE void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
    if (!enabled)
        return;

    uint32_t now = get_elapsed_time();

    LOCK_PROFILE();

    profile_frame_t *fn = find_frame(this_fn, call_site);

    // Recursion will need a real stack, this is absurd
    if (fn->enter_time != 0)
        fn->run_time += now - fn->enter_time;

    fn->enter_time = get_elapsed_time(); // not now, because we delayed execution
    fn->num_calls++;

    UNLOCK_PROFILE();
}

NO_PROFILE void __cyg_profile_func_exit(void *this_fn, void *call_site)
{
    if (!enabled)
        return;

    uint32_t now = get_elapsed_time();

    LOCK_PROFILE();

    profile_frame_t *fn = find_frame(this_fn, call_site);

    // Ignore if profiler was disabled when function entered
    if (fn->enter_time != 0)
        fn->run_time += now - fn->enter_time;

    fn->enter_time = 0;

    UNLOCK_PROFILE();
}
