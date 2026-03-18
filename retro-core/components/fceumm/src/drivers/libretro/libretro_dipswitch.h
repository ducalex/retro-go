#ifndef __VSUNI_DIPSWITCH__
#define __VSUNI_DIPSWITCH__

#include <libretro.h>

void set_dipswitch_variables(unsigned current_index, struct retro_core_option_v2_definition *vars);
void update_dipswitch(void);
void DPSW_Cleanup(void);

extern retro_environment_t environ_cb;

#endif /* __VSUNI_DIPSWITCH__ */
