#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    void *func_ptr;
    void *caller_ptr;
    uint32_t enter_time;
    uint32_t num_calls;
    uint32_t run_time;
} rg_profile_frame_t;

typedef struct
{
    int64_t time_started;
    int64_t time_stopped;
    int32_t total_frames;
    int32_t locked;
    rg_profile_frame_t frames[512];
} rg_profile_t;

void rg_profiler_init(void);
void rg_profiler_free(void);
void rg_profiler_start(void);
void rg_profiler_stop(void);
void rg_profiler_print(void);
void rg_profiler_push(char *section_name);
void rg_profiler_pop(void);

#ifdef __cplusplus
extern "C" {
#endif

    void __cyg_profile_func_enter(void *this_fn, void *call_site);
    void __cyg_profile_func_exit(void *this_fn, void *call_site);

#ifdef __cplusplus
}
#endif

#define NO_PROFILE __attribute((no_instrument_function))