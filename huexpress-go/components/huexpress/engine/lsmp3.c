/* lsmp3.c *************************************** UPDATED: 1999-04-08 23:38 TT
 *
 * Copyright   : (C) 1999 Tormod Tjaberg
 * Author      : Tormod Tjaberg (tjaberg@online.no)
 * Ripper      : Zeograd (zeograd@caramail.com)
 * Created     : 1999-02-14
 * Ripped      : around 1999-11-21
 * Description : Display the ID3 tag v1.0 & v1.1 of an MP3 file
 * Author      : Tormod Tjaberg 
 *
 * Note:
 * . Were shamelessly ripped from zeograd to know MP3 length for Hu-Go!
 *
 * . Genres were taken from Winamp 2.091 using doctored files :)
 *
 * . TAG code info is based on ID3-Tag Specification V1.1 by Michael Mutschler
 *   http://tick.informatik.uni-stuttgart.de/~mutschml/MP3ext/ID3-Tag.html
 *
 * . The MP3 header documentation is gleaned from several sources. 
 *   I could not find a definitive source which explained all versions.
 *
 *   . The "Private Life Of MP3 frames" 
 *     <http://www.id3.org>
 *
 *   . The Perl script "mp3tag.pm" by Peter D. Kovacs sent to me by Ole
 *     <http://www.egr.uri.edu/~kovacsp>
 *
 *   . A lot of the header decoder info is from Oliver Fromme's "mp3asm"
 *     <http://mpg.123.org>
 *
 * Changes:
 * 1999-04-08: 1.17: Exit properly after displaying "usage" text
 * 1999-03-07: 1.16: Cosmetic change
 * 1999-02-26: 1.15: Even more info, restructured layout
 * 1999-02-26: 1.10: Support for MP3 frame info
 * 1999-02-14: 1.00: Initial release, TAG's only
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#if defined(__linux__) || defined(__APPLE__)
#include <limits.h>
#endif
#include <assert.h>

#include "cleantypes.h"


#define PROGRAM_NAME "lsmp3"
#define PROGRAM_VER  "1.17"

#define ID3_TAG_LEN  128
#define TAG_LEN      3
#define SONGNAME_LEN 30
#define ARTIST_LEN   30
#define ALBUM_LEN    30
#define YEAR_LEN     4
#define COMMENT_LEN  30
#define NUM_GENRE    148
#define FRAME_SIZ    2048

/* Clever little option mimic wildcard expansion from
 * shell ONLY available with the Zortech compiler ver 3.0x and upwards
 */
#if defined(__ZTC__)
extern int _cdecl __wildcard;
int *__wild = &__wildcard;
#endif

/* Needs to be at least 4 bytes */
typedef unsigned long uint4;

/*
 *   1st index:  0 = "MPEG 1.0",   1 = "MPEG 2.0"
 *   2nd index:  0 unused,   1 = "Layer 3",   2 = "Layer 2",   3 = "Layer 1"
 *   3rd index:  BitRateIdx index from frame header
 */
int KbPerSecTab[2][4][16] = {
	{
	 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	 {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320},
	 {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384},
	 {0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448}
	 },
	{
	 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	 {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160},
	 {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160},
	 {0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256}
	 }
};

unsigned int FrqTab[] =
	{ 44100, 48000, 32000, 22050, 24000, 16000, 11025 };
char *EmphasisTbl[] = { "none", "50/15ms", "unknown", "CCITTj.17" };

char
fBufferEmpty(char *pBuffer)
{
	if (pBuffer == NULL)
		return 1;

	for (; isspace(*pBuffer) && *pBuffer != '\0'; pBuffer++);

	if (*pBuffer == '\0')
		return 1;
	else
		return 0;
}


float
MP3_length(char *argv)
{
	FILE *fp;
	char *pFileSpec;
	long FilePos;
	char TagStr[TAG_LEN + 1];
	char SongNameStr[SONGNAME_LEN + 1];
	char ArtistStr[ARTIST_LEN + 1];
	char AlbumStr[ALBUM_LEN + 1];
	char YearStr[YEAR_LEN + 1];
	char CommentStr[COMMENT_LEN + 1];
	uchar *pBuf;
	uchar *pBufTrav;
	uchar *pBufEnd;
	int Layer;
	int BitRateIdx;
	int Frq;
	int MajorVer;
	int MinorVer = 0;
	int KbPerSec = 0;
	int Mode;
	int Emphasis;
	int fCRC = 0;
	int fPadding = 0;
	int fPrivate = 0;
	int fCopyright = 0;
	int fOriginal = 0;
	uint4 Head;
	long n;
/*   long Length; */
	long TotTime = 0;
	long TotLength = 0;
	long TotNum = 0;
	int i;

	/* Add a few extra bytes so we may overstep our buffer */
	if ((pBuf = (uchar *) malloc(FRAME_SIZ + 8)) == NULL)
		return 0;

	{
		pFileSpec = argv;

		if ((fp = fopen(pFileSpec, "rb")) == NULL)
			return 0;

		/* Determine if this is an MP3 */
		n = fread(pBuf, sizeof(char), FRAME_SIZ, fp);

		if (n <= 128) {
			/* Too short */
			fclose(fp);
			return 0;
		}

		/* Use KbPerSec as a sentinel */
		for (KbPerSec = -1, pBufTrav = pBuf, pBufEnd = pBuf + n;
			 pBufTrav != pBufEnd; pBufTrav++) {
			/* Search for the sync 12 or 11 bits set !, I'm assuming that
			 * the alignment could be anything.
			 */
			if (pBufTrav[0] == 0xff && (pBufTrav[1] & 0xe0) == 0xe0) {
				Head = (uint4) pBufTrav[0] << 24 |
					(uint4) pBufTrav[1] << 16 |
					(uint4) pBufTrav[2] << 8 | (uint4) pBufTrav[3];

				/* Test code from mp3asm */
				if ((Head & 0x0c00) == 0x0c00 || !(Head & 0x060000))
					return 0;

				if ((BitRateIdx = (0x0f & Head >> 12)) == 0)
					return 0;

				if ((Layer = 4 - (0x03 & Head >> 17)) > 3)
					return 0;

				if (Head & 0x00100000) {
					if (Head & 0x00080000) {	/* MPEG 1.0 */
						MajorVer = 1;
						Frq = FrqTab[0x03 & (Head >> 10)];
					} else {	/* MPEG 2.0 */
						MajorVer = 2;
						Frq = FrqTab[3 + (0x03 & (Head >> 10))];
					}
					MinorVer = 0;
				} else {		/* "MPEG 2.5" */
					Frq = FrqTab[6];
					MajorVer = 2;
					MinorVer = 5;
				}

				KbPerSec =
					KbPerSecTab[MajorVer - 1][4 - Layer][BitRateIdx];

				Mode = 0x03 & (Head >> 6);

				/* If the protecion bit is NOT set then a 16bit CRC follows
				 * the frame header
				 */
				if (!(Head & 0x10000))
					fCRC = 1;

				if (Head & 0x0200)
					fPadding = 1;

				if (Head & 0x100)
					fPrivate = 1;

				if (Head & 0x08)
					fCopyright = 1;

				if (Head & 0x04)
					fOriginal = 1;

				Emphasis = Head & 0x3;

				break;
			}
		}

		/* Is this a valid MP3 file ? */
		if (KbPerSec == -1) {
			fclose(fp);
			return 0;
		}

		/* Seek to the end of the file */
		fseek(fp, 0, SEEK_END);
		FilePos = ftell(fp);

		return (float) FilePos / ((long) KbPerSec * 125);

/*
      TotTime += Length;
      TotLength += FilePos;
      TotNum++;


      printf("File   : %s (%ldK, %d:%02ds)\n",
              pFileSpec, FilePos / 1024, (int) (Length / 60), (int) (Length - ((Length / 60) * 60)));
      printf("Info   : MPEG ver %d.%d layer %d, freq %uHz, KBit/s %d, %s\n", 
              MajorVer, MinorVer, Layer, Frq, KbPerSec, Mode == 3 ? "Mono" : "Stereo");
*/
#if 0
		/* Just in case you need it :) */
		printf
			("Details: Private=%s CRC's=%s Copyright=%s Original=%s Emphasis=%s\n",
			 fPrivate ? "Yes" : "No", fCRC ? "Yes" : "No",
			 fCopyright ? "Yes" : "No", fOriginal ? "Yes" : "No",
			 EmphasisTbl[Emphasis]);
#endif

		if (pBuf[0] == 'I' && pBuf[1] == 'D' && pBuf[2] == '3') {
			fclose(fp);
			return 0;
		}

		/* Look for the TAG ID go backwards */
		if (FilePos < ID3_TAG_LEN) {
			fclose(fp);
			return 0;
		}

		/* Position ourselves in front of the TAG */
		fseek(fp, -ID3_TAG_LEN, SEEK_END);

		/* Start with a clean slate */
		TagStr[0] = 0;
		if (fread(TagStr, sizeof(char), TAG_LEN, fp) != TAG_LEN) {
			fclose(fp);
			return 0;
		}

		if (!(TagStr[0] == 'T' && TagStr[1] == 'A' && TagStr[2] == 'G')) {
			fclose(fp);
			return 0;
		}

		/* Read in all the fields and null terminate them */
		fread(SongNameStr, sizeof(char), SONGNAME_LEN, fp);
		SongNameStr[SONGNAME_LEN] = 0;
		fread(ArtistStr, sizeof(char), ARTIST_LEN, fp);
		ArtistStr[ARTIST_LEN] = 0;
		fread(AlbumStr, sizeof(char), ALBUM_LEN, fp);
		AlbumStr[ALBUM_LEN] = 0;
		fread(YearStr, sizeof(char), YEAR_LEN, fp);
		YearStr[YEAR_LEN] = 0;
		fread(CommentStr, sizeof(char), COMMENT_LEN, fp);
		CommentStr[COMMENT_LEN] = 0;

		/* Finally get the Genre */
		i = getc(fp);

		fclose(fp);

		if (i == EOF)
			return 0;

		/* All stuff is read in, display it, now instead of storing
		 * 0's a lot of people hve stored space, handle it
		 */
/*
      if (!fBufferEmpty(ArtistStr))
         printf("Artist : %s\n", ArtistStr);

      if (!fBufferEmpty(SongNameStr))
         printf("Title  : %s\n", SongNameStr);

      if (!fBufferEmpty(AlbumStr))
         printf("Album  : %s\n", AlbumStr);

      if (!fBufferEmpty(YearStr))
         printf("Year   : %s\n", YearStr);

      if (!fBufferEmpty(CommentStr))
         printf("Comment: %s\n", CommentStr);
*/

		/* Special ID3 ver 1.1 addition */

/*
      if (CommentStr[COMMENT_LEN - 2] == 0 && 
          CommentStr[COMMENT_LEN - 1] != 0)
         printf("Track  : %d\n", CommentStr[COMMENT_LEN - 1]);
*/

		/* 255 means unused */

/*
      if (i != 255)
      {
         if (i >= 0 && i < NUM_GENRE)
            printf("Genre  : %s\n", pGenreTbl[i]);
         else
            printf("Genre  : unknown genre\n");
      }
*/
		putchar('\n');
	}

	if (pBuf != NULL)
		free(pBuf);

	if (TotNum > 1)
		printf("Total  : %ld files %ldK %d:%02ds\n", TotNum,
			   TotLength / 1024, (int) (TotTime / 60),
			   (int) (TotTime - ((TotTime / 60) * 60)));

	return 0;
}
