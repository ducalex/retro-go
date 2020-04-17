#include <stdio.h>

#include "romdb.h"

int
main(int argc, char* argv[])
{
	char* filename = argv[1];
	uint32 CRC = CRC_file(filename);
	int32 romindex = -1;

	int index;
	for (index = 0; index < KNOWN_ROM_COUNT; index++) {
		if (CRC == kKnownRoms[index].CRC)
			romindex = index;
	}

	if (romindex == -1)
		printf("%s; CRC=%lx; Unknown\n", filename, CRC);
	else
		printf("%s; CRC=%lx; %s\n", filename, CRC, kKnownRoms[romindex].Name);

	return 0;
}
