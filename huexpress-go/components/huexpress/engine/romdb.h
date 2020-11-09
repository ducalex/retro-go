#ifndef _ROMFLAGS_H
#define _ROMFLAGS_H

#include <utils.h>

#define TWO_PART_ROM 0x0001
#define ONBOARD_RAM  0x0100
#define US_ENCODED   0x0010
#define USA          0x4000
#define JAP          0x8000

const struct {
	const uint32_t CRC;
	const char *Name;
	const uint32_t Flags;
} romFlags[] = {
	{0x00000000, "Unknown", JAP},
	{0xF0ED3094, "Blazing Lazers", USA | TWO_PART_ROM},
	{0xB4A1B0F6, "Blazing Lazers", USA | TWO_PART_ROM},
	{0x55E9630D, "Legend of Hero Tonma", USA | US_ENCODED},
	{0x083C956A, "Populous", JAP | ONBOARD_RAM},
	{0x0A9ADE99, "Populous", JAP | ONBOARD_RAM},
};

#define KNOWN_ROM_COUNT (sizeof(romFlags) / sizeof(romFlags[0]))

#endif /* _ROMFLAGS_H */
