//Wolf4SDL\DC
//dc_cd.cpp
//2009 - Cyle Terry

#if defined(_arch_dreamcast)

#include "dc/cdrom.h"

int DC_CheckDrive() {
    int disc_status;
    int disc_type;
    cdrom_get_status(&disc_status, &disc_type);
    return disc_status;
}

#endif
