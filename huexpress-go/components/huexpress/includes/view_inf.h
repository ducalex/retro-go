#ifndef _DJGPP_INCLUDE_VIEW_INF_H
#define _DJGPP_INCLUDE_VIEW_INF_H

#define NB_CHAR_LINE 32

#define MAX_PAGES 10

#include "pce.h"
#include "cleantypes.h"

void view_info();
// Display pages of infos on hardware

void display_cd_var();
// Display info on CD bios variables

void display_reg();
// Display general info on reg values

void display_satb1();
// Display satb from 0 to 7

void display_satb2();
// Display 2th page of satb

void display_satb3();
// Display 3th page of satb

void display_satb4();
// Display 4th page of satb

void display_satb5();
// Display 5th page of satb

void display_satb6();
// Display 6th page of satb

void display_satb7();
// Display 7th page of satb

void display_satb8();
// Display 8th page of satb

void display_pattern();
// Display pattern in Vram

#endif
