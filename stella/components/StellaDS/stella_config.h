// =====================================================================================================
// Stella DSi - Improved Version by Dave Bernazzani (wavemotion)
//
// See readme.txt for a list of everything that has changed in the baseline 1.0 code.
// =====================================================================================================
#ifndef __STELLA_CONFIG_H
#define __STELLA_CONFIG_H

#include <nds.h>
#include "Cart.hxx"

// ---------------------------
// Config handling...
// ---------------------------
#define CONFIG_VER  0x000C

#define MAX_CONFIGS 1300

typedef struct
{
    uInt16                  config_ver;
    struct CartInfo         cart[MAX_CONFIGS];
    struct GlobalCartInfo   global;
    uInt32                  crc32;
} __attribute__((aligned(4))) AllConfig_t;

extern AllConfig_t *allConfigs_ptr;
#define allConfigs (*allConfigs_ptr)

void LoadConfig(void);
void ShowConfig(void);
void SaveConfig(bool bShow);
void LoadFavorites(void);

#endif
