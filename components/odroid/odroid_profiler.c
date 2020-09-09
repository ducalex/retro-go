#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <stdio.h>

#include "odroid_system.h"
#include "odroid_profiler.h"

// Note this profiler might be inaccurate because of:
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=28205

static profile_t *profile;
static volatile bool profile_lock;

NO_PROFILE static inline profile_frame_t *find_frame(void *this_fn, void *call_site)
{
    for (int i = 0; i < 512; ++i)
    {
        profile_frame_t *frame = &profile->frames[i];

        if (frame->func_ptr == 0)
        {
            profile->total_frames = MAX(profile->total_frames, i + 1);
            frame->func_ptr = this_fn;
            frame->caller_ptr = call_site;
        }

        if (frame->func_ptr == this_fn && frame->caller_ptr == call_site)
        {
            return frame;
        }
    }

    RG_PANIC("Profile memory exhausted!");
}

NO_PROFILE void rg_profiler_init(void)
{
    printf("%s: Initializing profile...\n", __func__);
    profile = rg_alloc(sizeof(profile_t), MEM_SLOW);
    profile->time_started = get_elapsed_time();
}

NO_PROFILE void rg_profiler_free(void)
{
    // free(call_stack);
    free(profile);
    profile = NULL;
}

NO_PROFILE static int list_comparator(const void *p, const void *q)
{
    return ((profile_frame_t*)q)->run_time - ((profile_frame_t*)p)->run_time;
}

NO_PROFILE void rg_profiler_print(void)
{
    if (!profile)
        return;

    // xSemaphoreTake(profile_lock, pdMS_TO_TICKS(10000));
    profile_lock = true;

    uint32_t time_running = get_elapsed_time_since(profile->time_started);

    // probably should use a mutex here
    qsort(profile->frames, profile->total_frames, sizeof(profile_frame_t), list_comparator);

    for (int i = 0; i < profile->total_frames; ++i)
    {
        profile_frame_t *frame = &profile->frames[i];

        printf(
            "%p\t%p\t%d\t%d\t%.0f%%\n",
            frame->caller_ptr,
            frame->func_ptr,
            frame->num_calls,
            frame->run_time,
            (float)frame->run_time / time_running * 100
        );
    }

    printf("Profile frames: %d, total time: %d\n", profile->total_frames, time_running);

    profile_lock = false;
}

NO_PROFILE void rg_profiler_push(char *section_name)
{

}

NO_PROFILE void rg_profiler_pop(void)
{

}

NO_PROFILE void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
    if (!profile)
        return;

    while (profile_lock);

    profile_frame_t *fn = find_frame(this_fn, call_site);

    // Recursion
    if (fn->enter_time != 0)
        fn->run_time += get_elapsed_time_since(fn->enter_time);

    fn->enter_time = get_elapsed_time();
    fn->num_calls++;
}

NO_PROFILE void __cyg_profile_func_exit(void *this_fn, void *call_site)
{
    if (!profile)
        return;

    while (profile_lock);

    profile_frame_t *fn = find_frame(this_fn, call_site);

    fn->run_time += get_elapsed_time_since(fn->enter_time);
    fn->enter_time = 0;
}
