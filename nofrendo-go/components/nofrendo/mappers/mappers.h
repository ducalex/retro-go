#pragma once

#include <mmc.h>

/* mapper interfaces */
extern mapintf_t map0_intf;
extern mapintf_t map1_intf;
extern mapintf_t map2_intf;
extern mapintf_t map3_intf;
extern mapintf_t map4_intf;
extern mapintf_t map5_intf;
extern mapintf_t map7_intf;
extern mapintf_t map9_intf;
extern mapintf_t map10_intf;
extern mapintf_t map11_intf;
extern mapintf_t map15_intf;
extern mapintf_t map16_intf;
extern mapintf_t map18_intf;
extern mapintf_t map19_intf;
extern mapintf_t map20_intf;
extern mapintf_t map21_intf;
extern mapintf_t map22_intf;
extern mapintf_t map23_intf;
extern mapintf_t map24_intf;
extern mapintf_t map25_intf;
extern mapintf_t map30_intf;
extern mapintf_t map31_intf;
extern mapintf_t map32_intf;
extern mapintf_t map33_intf;
extern mapintf_t map34_intf;
extern mapintf_t map40_intf;
extern mapintf_t map41_intf;
extern mapintf_t map42_intf;
extern mapintf_t map46_intf;
extern mapintf_t map50_intf;
extern mapintf_t map64_intf;
extern mapintf_t map65_intf;
extern mapintf_t map66_intf;
extern mapintf_t map70_intf;
extern mapintf_t map71_intf;
extern mapintf_t map73_intf;
extern mapintf_t map74_intf;
extern mapintf_t map75_intf;
extern mapintf_t map76_intf;
extern mapintf_t map78_intf;
extern mapintf_t map79_intf;
extern mapintf_t map85_intf;
extern mapintf_t map87_intf;
extern mapintf_t map93_intf;
extern mapintf_t map94_intf;
extern mapintf_t map119_intf;
extern mapintf_t map160_intf;
extern mapintf_t map162_intf;
extern mapintf_t map163_intf;
extern mapintf_t map176_intf;
extern mapintf_t map185_intf;
extern mapintf_t map191_intf;
extern mapintf_t map192_intf;
extern mapintf_t map193_intf;
extern mapintf_t map194_intf;
extern mapintf_t map195_intf;
extern mapintf_t map206_intf;
extern mapintf_t map228_intf;
extern mapintf_t map229_intf;
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
    &map9_intf,
    &map10_intf,
    &map11_intf,
    &map15_intf,
    &map16_intf,
    &map18_intf,
    &map19_intf,
    &map20_intf,
    &map21_intf,
    &map22_intf,
    &map23_intf,
    &map24_intf,
    &map25_intf,
    &map30_intf,
    &map31_intf,
    &map32_intf,
    &map33_intf,
    &map34_intf,
    &map40_intf,
    &map41_intf,
    &map42_intf,
    &map46_intf,
    &map50_intf,
    &map64_intf,
    &map65_intf,
    &map66_intf,
    &map70_intf,
    &map71_intf,
    &map73_intf,
    &map74_intf,
    &map75_intf,
    &map76_intf,
    &map78_intf,
    &map79_intf,
    &map85_intf,
    &map87_intf,
    &map93_intf,
    &map94_intf,
    &map119_intf,
    &map160_intf,
    &map162_intf,
    &map163_intf,
    &map176_intf,
    &map185_intf,
    &map191_intf,
    &map192_intf,
    &map193_intf,
    &map194_intf,
    &map195_intf,
    &map206_intf,
    &map228_intf,
    &map229_intf,
    &map231_intf,
    NULL
};
