#include <rg_system.h>
#include <malloc.h>
#include <string.h>

#include "utils.h"

static const char **strings;
static size_t strings_count;

const char *const_string(const char *str)
{
    if (!str)
        return NULL;

    // To do : use hashmap or something faster
    for (int i = 0; i < strings_count; i++)
        if (strcmp(strings[i], str) == 0)
            return strings[i];

    str = strdup(str);

    strings = realloc(strings, (strings_count + 1) * sizeof(char*));
    RG_ASSERT(strings && str, "alloc failed");

    strings[strings_count++] = str;

    return str;
}
