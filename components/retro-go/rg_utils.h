#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define RG_TIMER_INIT() int64_t _rgts_ = rg_system_timer(), _rgtl_ = rg_system_timer();
#define RG_TIMER_LAP(name)                                                                           \
    RG_LOGW("Lap %s: %dms   Total: %dms\n", name,                                                    \
            (int)((rg_system_timer() - _rgtl_) / 1000), (int)((rg_system_timer() - _rgts_) / 1000)); \
    _rgtl_ = rg_system_timer();

#define RG_MIN(a, b)            \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a < _b ? _a : _b;      \
    })
#define RG_MAX(a, b)            \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a > _b ? _a : _b;      \
    })
#define RG_COUNT(array) (sizeof(array) / sizeof((array)[0]))

#define PRINTF_BINARY_8 "%c%c%c%c%c%c%c%c"
#define PRINTF_BINVAL_8(i)\
    (((i) & 0x80) ? '1' : '0'), \
    (((i) & 0x40) ? '1' : '0'), \
    (((i) & 0x20) ? '1' : '0'), \
    (((i) & 0x10) ? '1' : '0'), \
    (((i) & 0x08) ? '1' : '0'), \
    (((i) & 0x04) ? '1' : '0'), \
    (((i) & 0x02) ? '1' : '0'), \
    (((i) & 0x01) ? '1' : '0')

#define PRINTF_BINARY_16 PRINTF_BINARY_8 " " PRINTF_BINARY_8
#define PRINTF_BINVAL_16(i) PRINTF_BINVAL_8((i) >> 8), PRINTF_BINVAL_8(i)
#define PRINTF_BINARY_32 PRINTF_BINARY_16 " " PRINTF_BINARY_16
#define PRINTF_BINVAL_32(i) PRINTF_BINVAL_16((i) >> 16), PRINTF_BINVAL_16(i)

/* String functions */

/**
 * both functions give you an allocation of strlen(str) + 1 valid for the lifetime of the application
 * they cannot be freed. unique avoids keeping multiple copies of an identical string (eg a path)
 * Things like unique_string("abc") == unique_string("abc") are guaranteed to be true
*/
const char *rg_const_string(const char *str);
const char *rg_unique_string(const char *str);
char *rg_strtolower(char *str);
char *rg_strtoupper(char *str);
char *rg_json_fixup(char *json);

/* Paths functions */
const char *rg_dirname(const char *path);
const char *rg_basename(const char *path);
const char *rg_extension(const char *filename);
bool rg_extension_match(const char *filename, const char *extensions);
const char *rg_relpath(const char *path);

/* Hashing */
uint32_t rg_crc32(uint32_t crc, const uint8_t *buf, size_t len);
uint32_t rg_hash(const char *buf, size_t len);

/* Misc */
void *rg_alloc(size_t size, uint32_t caps);
// rg_usleep behaves like usleep in libc: it will sleep for *at least* `us` microseconds, but possibly more
// due to scheduling. You should use rg_task_delay() if you don't need more than 10-15ms granularity.
void rg_usleep(uint32_t us);

#define MEM_ANY   (0)
#define MEM_SLOW  (1)
#define MEM_FAST  (2)
#define MEM_DMA   (4)
#define MEM_8BIT  (8)
#define MEM_32BIT (16)
#define MEM_EXEC  (32)
#define MEM_NOPANIC (64)

#define PTR_IN_SPIRAM(ptr) ((void *)(ptr) >= (void *)0x3F800000 && (void *)(ptr) < (void *)0x3FC00000)
