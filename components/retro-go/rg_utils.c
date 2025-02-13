#include "rg_system.h"
#include "rg_utils.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

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

char *rg_json_fixup(char *json)
{
    // Strip trailing commas, eg [,1,2,3] {"a":1,}
    for (char *ptr = json, *prev = ptr; ptr && *ptr; ++ptr)
    {
        if ((*ptr == '}' || *ptr == ']' || *ptr == '{' || *ptr == '[' || *ptr == ',') && *prev == ',')
        {
            RG_LOGW("Found trailing comma at pos %d", (int)(ptr - json));
            *prev = ' ';
        }
        if (*ptr != '\t' && *ptr != '\n'  && *ptr != '\r' && *ptr != ' ')
            prev = ptr;
    }

    // TODO: We should also strip C-style comments!

    return json;
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

const char *rg_extension(const char *filename)
{
    if (!filename)
        return NULL;
    const char *ptr = filename + strlen(filename) - 1;
    while (ptr > filename && *ptr != '/')
    {
        if (*ptr == '.')
            return ptr + 1;
        ptr--;
    }
    return NULL;
}

bool rg_extension_match(const char *filename, const char *extensions)
{
    const char *ext = rg_extension(filename);
    if (!ext || !extensions)
        return false;

    const char *haystack = extensions;
    while (*haystack)
    {
        while (*haystack == ' ')
            haystack++;

        const char *needle = ext;
        while (*needle && *haystack && tolower((unsigned char)*needle) == tolower((unsigned char)*haystack))
        {
            needle++;
            haystack++;
        }
        if (*needle == 0 && (*haystack == 0 || *haystack == ' '))
            return true;

        // Fast forward to next extension
        while (*haystack && *haystack != ' ')
            haystack++;
    }

    return false;
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

uint32_t rg_crc32(uint32_t crc, const uint8_t *buf, size_t len)
{
#ifdef ESP_PLATFORM
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

/**
 * This function is the SuperFastHash from:
 *  http://www.azillionmonkeys.com/qed/hash.html
*/
IRAM_ATTR uint32_t rg_hash(const char *data, size_t len)
{
    #define get16bits(d) (*((const uint16_t *)(d)))

    if (len <= 0 || data == NULL)
        return 0;

    uint32_t hash = len, tmp;
    int rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (; len > 0; len--)
    {
        hash += get16bits(data);
        tmp = (get16bits(data + 2) << 11) ^ hash;
        hash = (hash << 16) ^ tmp;
        data += 2 * sizeof(uint16_t);
        hash += hash >> 11;
    }

    /* Handle end cases */
    switch (rem)
    {
    case 3:
        hash += get16bits(data);
        hash ^= hash << 16;
        hash ^= ((signed char)data[sizeof(uint16_t)]) << 18;
        hash += hash >> 11;
        break;
    case 2:
        hash += get16bits(data);
        hash ^= hash << 11;
        hash += hash >> 17;
        break;
    case 1:
        hash += (signed char)*data;
        hash ^= hash << 10;
        hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    #undef get16bits
    return hash;
}

typedef struct
{
    uint16_t length;
    uint16_t unused;
    char data[];
} unique_string_t;

const char *rg_unique_string(const char *str)
{
    static const unique_string_t **strings = NULL;
    static size_t strings_count = 0;

    if (!str)
        return NULL;

    size_t len = strlen(str);

    for (int i = 0; i < strings_count; i++)
    {
        if (strings[i]->length != len)
            continue;
        if (memcmp(strings[i]->data, str, len + 1) == 0)
            return strings[i]->data;
    }

    unique_string_t *obj = malloc(sizeof(unique_string_t) + len + 1);

    strings = realloc(strings, (strings_count + 1) * sizeof(unique_string_t *));
    RG_ASSERT(strings && obj, "alloc failed");

    memcpy(obj->data, str, len + 1);
    obj->length = len;

    strings[strings_count++] = obj;

    return obj->data;
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

#ifdef ESP_PLATFORM
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
            RG_LOGW("SIZE=%d, CAPS=%s, PTR=%p << CAPS not fully met! (available: %d)\n",
                    (int)size, caps_list, ptr, (int)available);
            return ptr;
        }
    }
#else
    ptr = calloc(1, size);
#endif

    if (!ptr)
    {
        RG_LOGE("SIZE=%d, CAPS=%s << FAILED! (available: %d)\n", (int)size, caps_list, (int)available);
        if (caps & MEM_NOPANIC)
            return NULL;
        RG_PANIC("Memory allocation failed!");
    }

    RG_LOGI("SIZE=%d, CAPS=%s, PTR=%p\n", (int)size, caps_list, ptr);
    return ptr;
}

void rg_usleep(uint32_t us)
{
    int64_t goal = rg_system_timer() + us;
    // Only yield if it's for more than one tick duration, otherwise the delay will overshoot
    if (us >= 1000000 / RG_TICK_RATE)
        rg_task_delay(us / 1000);
    // Then we busy wait, which is fine as it's a short delay
    while (rg_system_timer() < goal)
        continue;
}
