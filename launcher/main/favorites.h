#include "emulators.h"

typedef struct {
    char path[168];
    bool removed;
    retro_emulator_file_t file;
} favorite_t;

void favorites_init();
void favorites_load();
void favorites_save();
bool favorite_add(retro_emulator_file_t *file);
bool favorite_remove(retro_emulator_file_t *file);
favorite_t *favorite_find(retro_emulator_file_t *file);
