#include "rt_def.h"
#include <rg_system.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Hack for boolean conflict
#undef false
#undef true
#define boolean rott_boolean

#include "z_zone.h"
#include "rt_error.h"
#include "rt_main.h"

extern void Error (const char *error, ...);

// Simple wrapper for Retro-Go memory management
// replacing the complex DOS zone allocator

void Z_Init (int size, int min)
{
    printf("ROTT Z_Init: Using Retro-Go rg_alloc for zone management\n");
}

void *Z_Malloc (int size, int tag, void *user)
{
    void *ptr = rg_alloc(size, MEM_SLOW);
    if (!ptr) {
        Error("Z_Malloc failed to allocate %d bytes", size);
    }
    if (user) {
        *(void **)user = ptr;
    }
    return ptr;
}

void *Z_LevelMalloc (int size, int tag, void *user)
{
    return Z_Malloc(size, tag, user);
}

void Z_Free (void *ptr)
{
    if (ptr) free(ptr);
}

void Z_FreeTags (int lowtag, int hightag)
{
    // Stubs for now
}

void Z_CheckHeap (void) {}
void Z_DumpHeap (int lowtag, int hightag) {}

int Z_FreeMemory (void) { return 4 * 1024 * 1024; }
int Z_AvailHeap (void) { return 4 * 1024 * 1024; }

void Z_CheckCard (void) {}

void *Z_TagMalloc (int size, int tag)
{
    return Z_Malloc(size, tag, NULL);
}

void Z_ChangeTag (void *ptr, int tag) {}

void Z_FreeAll (void) {}
