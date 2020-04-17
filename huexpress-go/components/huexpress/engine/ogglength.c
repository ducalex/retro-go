#include "ogglength.h"

#if 0

//Inspired from ogginfo.c
// part of the vorbis-tools package of the OGG Vorbis project
int
OGG_length(const char *filename)
{
	FILE *fp;
	OggVorbis_File vf;
	int rc, i;
	double playtime;

	memset(&vf, 0, sizeof(OggVorbis_File));

	fp = fopen(filename, "r");
	if (!fp) {
		fprintf(stderr, "Unable to open \"%s\n", filename);
	}

	rc = ov_open(fp, &vf, NULL, 0);

	if (rc < 0) {
		fprintf(stderr, "Unable to understand \"%s\", errorcode=%d\n",
				filename, rc);
		return 0;
	}

	playtime = (double) ov_time_total(&vf, -1);

//  printf("length (seconds) =%f\n",playtime);
//  printf("length (samples) =%d\n",((int) (playtime * 75)));

	ov_clear(&vf);

	return (int) (playtime * 75.0);
}

#endif
