#include "emulators.h"

typedef struct {
    char name[64];
    char path[168];
    int  removed;
    retro_emulator_file_t file;
} favorite_t;

void favorites_init();
bool favorite_add(retro_emulator_file_t *file);
bool favorite_remove(retro_emulator_file_t *file);
favorite_t *favorite_find(retro_emulator_file_t *file);
