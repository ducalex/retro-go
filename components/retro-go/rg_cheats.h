#pragma once

#include <stdint.h>

typedef enum
{
    RG_CHEAT_FORMAT_GAME_GENIE,
    RG_CHEAT_FORMAT_GAME_SHARK,
} rg_cheat_format_t;

typedef enum
{
    RG_CHEAT_TYPE_ROM,
    RG_CHEAT_TYPE_RAM_SINGLE,
    RG_CHEAT_TYPE_RAM_STICKY,
} rg_cheat_type_t;

typedef struct
{
    char name[32];
    char code[12];
    int type;
    int enabled;
    int address;
    int new_value;
    int old_value;
} rg_cheat_t;

void rg_cheats_init(void);
void rg_cheats_deinit(void);
void rg_cheats_load(void);
void rg_cheats_save(void);
void rg_cheats_apply(void);
void rg_cheats_reset(void);

int rg_cheats_add(int address, int new_value, int comp_value, rg_cheat_type_t type);
int rg_cheats_remove(int id);
int rg_cheats_add_code(const char *code, rg_cheat_format_t format);
int rg_cheats_remove_code(const char *code);
