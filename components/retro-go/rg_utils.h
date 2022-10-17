#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define RG_TIMER_INIT() int64_t _rgts_ = rg_system_timer(), _rgtl_ = rg_system_timer();
#define RG_TIMER_LAP(name) \
    RG_LOGX("Lap %s: %.2f   Total: %.2f\n", #name, (rg_system_timer() - _rgtl_) / 1000.f, \
            (rg_system_timer() - _rgts_) / 1000.f); _rgtl_ = rg_system_timer();

#define RG_MIN(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a < _b ? _a : _b; })
#define RG_MAX(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a > _b ? _a : _b; })
#define RG_COUNT(array) (sizeof(array) / sizeof((array)[0]))

char *rg_strtolower(char *str);
char *rg_strtoupper(char *str);
size_t rg_strlcpy(char *dst, const char *src, size_t size);

bool rg_token_in_list(const char *list, const char *token);

const char *rg_dirname(const char *path);
const char *rg_basename(const char *path);
const char *rg_extension(const char *path);
const char *rg_relpath(const char *path);

uint32_t rg_crc32(uint32_t crc, const uint8_t* buf, uint32_t len);
