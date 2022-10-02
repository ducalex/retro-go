#include <rg_system.h>
#include <string.h>

#include "rg_utils.h"

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
        if (relpath[0] == '/')
            path = relpath + 1;
        else if (relpath[0] == 0)
            path = relpath;
    }
    return path;
}

uint32_t rg_crc32(uint32_t crc, const uint8_t *buf, uint32_t len)
{
    // This is part of the ROM but finding the correct header is annoying as it differs per SOC...
    extern uint32_t crc32_le(uint32_t crc, const uint8_t * buf, uint32_t len);
    return crc32_le(crc, buf, len);
}