#include "shared.h"

void snes_main(void)
{
    // Currently a separate app, see snes9x-go in project's root
    rg_system_switch_app("snes9x-go", "snes", 0, 0);
}
