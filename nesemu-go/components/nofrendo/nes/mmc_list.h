#pragma once

#include "nes_mmc.h"

/* mapper interfaces */
extern mapintf_t map0_intf;
extern mapintf_t map1_intf;
extern mapintf_t map2_intf;
extern mapintf_t map3_intf;
extern mapintf_t map4_intf;
extern mapintf_t map5_intf;
extern mapintf_t map7_intf;
extern mapintf_t map8_intf;
extern mapintf_t map9_intf;
extern mapintf_t map10_intf;
extern mapintf_t map11_intf;
extern mapintf_t map15_intf;
extern mapintf_t map16_intf;
extern mapintf_t map18_intf;
extern mapintf_t map19_intf;
extern mapintf_t map21_intf;
extern mapintf_t map22_intf;
extern mapintf_t map23_intf;
extern mapintf_t map24_intf;
extern mapintf_t map25_intf;
extern mapintf_t map32_intf;
extern mapintf_t map33_intf;
extern mapintf_t map34_intf;
extern mapintf_t map40_intf;
extern mapintf_t map64_intf;
extern mapintf_t map65_intf;
extern mapintf_t map66_intf;
extern mapintf_t map70_intf;
extern mapintf_t map75_intf;
extern mapintf_t map78_intf;
extern mapintf_t map79_intf;
extern mapintf_t map85_intf;
extern mapintf_t map94_intf;
extern mapintf_t map99_intf;
extern mapintf_t map231_intf;

/* implemented mapper interfaces */
static const mapintf_t *mappers[] =
{
    &map0_intf,
    &map1_intf,
    &map2_intf,
    &map3_intf,
    &map4_intf,
    &map5_intf,
    &map7_intf,
    &map8_intf,
    &map9_intf,
    &map10_intf,
    &map11_intf,
    &map15_intf,
    &map16_intf,
    &map18_intf,
    &map19_intf,
    &map21_intf,
    &map22_intf,
    &map23_intf,
    &map24_intf,
    &map25_intf,
    &map32_intf,
    &map33_intf,
    &map34_intf,
    &map40_intf,
    &map64_intf,
    &map65_intf,
    &map66_intf,
    &map70_intf,
    &map75_intf,
    &map78_intf,
    &map79_intf,
    &map85_intf,
    &map94_intf,
    &map99_intf,
    &map231_intf,
    NULL
};
