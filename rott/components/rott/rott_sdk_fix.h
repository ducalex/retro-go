#ifndef ROTT_SDK_FIX_H
#define ROTT_SDK_FIX_H

// Force-included to resolve SDK v5.5.1 header conflicts
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Provide a macro that doesn't rely on implicit declarations
#ifndef assert
#define assert(x) ((void)0)
#endif

// Declare __assert_func just in case something else uses it
void __assert_func(const char *file, int line, const char *func, const char *failedexpr);

#ifdef __cplusplus
}
#endif

#endif
