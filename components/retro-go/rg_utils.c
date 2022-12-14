#include "rg_system.h"
#include "rg_utils.h"

#include <stdlib.h>
#include <string.h>

#ifndef RG_TARGET_SDL2
#include <esp_heap_caps.h>
#endif

char *rg_strtolower(char *str)
{
    if (!str)
        return NULL;

    for (char *c = str; *c; c++)
        if (*c >= 'A' && *c <= 'Z')
            *c += 32;

    return str;
}

char *rg_strtoupper(char *str)
{
    if (!str)
        return NULL;

    for (char *c = str; *c; c++)
        if (*c >= 'a' && *c <= 'z')
            *c -= 32;

    return str;
}

const char *rg_dirname(const char *path)
{
    static char buffer[100];
    const char *basename = strrchr(path, '/');
    ptrdiff_t length = basename - path;

    if (!path || !basename)
        return ".";

    if (path[0] == '/' && path[1] == 0)
        return "/";

    RG_ASSERT(length < 100, "to do: use heap");

    strncpy(buffer, path, length);
    buffer[length] = 0;

    return buffer;
}

const char *rg_basename(const char *path)
{
    if (!path)
        return ".";

    const char *name = strrchr(path, '/');
    return name ? name + 1 : path;
}

const char *rg_extension(const char *path)
{
    if (!path)
        return NULL;

    const char *ext = strrchr(rg_basename(path), '.');
    return ext ? ext + 1 : NULL;
}

const char *rg_relpath(const char *path)
{
    if (!path)
        return NULL;

    if (strncmp(path, RG_STORAGE_ROOT, strlen(RG_STORAGE_ROOT)) == 0)
    {
        const char *relpath = path + strlen(RG_STORAGE_ROOT);
        if (relpath[0] == '/' || relpath[0] == 0)
            path = relpath;
    }
    return path;
}

uint32_t rg_crc32(uint32_t crc, const uint8_t *buf, uint32_t len)
{
#ifndef RG_TARGET_SDL2
    // This is part of the ROM but finding the correct header is annoying as it differs per SOC...
    extern uint32_t crc32_le(uint32_t crc, const uint8_t *buf, uint32_t len);
    return crc32_le(crc, buf, len);
#else
    // Derived from: http://www.hackersdelight.org/hdcodetxt/crc.c.txt
    crc = ~crc;
    for (size_t i = 0; i < len; ++i)
    {
        crc = crc ^ buf[i];
        for (int j = 7; j >= 0; j--) // Do eight times.
        {
            uint32_t mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
    }
    return ~crc;
#endif
}

const char *const_string(const char *str)
{
    static const char **strings = NULL;
    static size_t strings_count = 0;

    if (!str)
        return NULL;

    // To do : use hashmap or something faster
    for (int i = 0; i < strings_count; i++)
        if (strcmp(strings[i], str) == 0)
            return strings[i];

    str = strdup(str);

    strings = realloc(strings, (strings_count + 1) * sizeof(char *));
    RG_ASSERT(strings && str, "alloc failed");

    strings[strings_count++] = str;

    return str;
}

// Note: You should use calloc/malloc everywhere possible. This function is used to ensure
// that some memory is put in specific regions for performance or hardware reasons.
// Memory from this function should be freed with free()
void *rg_alloc(size_t size, uint32_t caps)
{
    char caps_list[36] = "";
    size_t available = 0;
    void *ptr;

    if (caps & MEM_SLOW)
        strcat(caps_list, "SPIRAM|");
    if (caps & MEM_FAST)
        strcat(caps_list, "INTERNAL|");
    if (caps & MEM_DMA)
        strcat(caps_list, "DMA|");
    if (caps & MEM_EXEC)
        strcat(caps_list, "IRAM|");
    strcat(caps_list, (caps & MEM_32BIT) ? "32BIT" : "8BIT");

#ifndef RG_TARGET_SDL2
    uint32_t esp_caps = 0;
    esp_caps |= (caps & MEM_SLOW ? MALLOC_CAP_SPIRAM : (caps & MEM_FAST ? MALLOC_CAP_INTERNAL : 0));
    esp_caps |= (caps & MEM_DMA ? MALLOC_CAP_DMA : 0);
    esp_caps |= (caps & MEM_EXEC ? MALLOC_CAP_EXEC : 0);
    esp_caps |= (caps & MEM_32BIT ? MALLOC_CAP_32BIT : MALLOC_CAP_8BIT);

    if (!(ptr = heap_caps_calloc(1, size, esp_caps)))
    {
        available = heap_caps_get_largest_free_block(esp_caps);
        // Loosen the caps and try again
        if ((ptr = heap_caps_calloc(1, size, esp_caps & ~(MALLOC_CAP_SPIRAM | MALLOC_CAP_INTERNAL))))
        {
            RG_LOGW("SIZE=%u, CAPS=%s, PTR=%p << CAPS not fully met! (available: %d)\n", size, caps_list, ptr,
                    available);
            return ptr;
        }
    }
#else
    ptr = calloc(1, size);
#endif

    if (!ptr)
    {
        RG_LOGE("SIZE=%u, CAPS=%s << FAILED! (available: %d)\n", size, caps_list, available);
        RG_PANIC("Memory allocation failed!");
    }

    RG_LOGI("SIZE=%u, CAPS=%s, PTR=%p\n", size, caps_list, ptr);
    return ptr;
}
