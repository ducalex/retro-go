/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "cheat.h"


void
fputw(uint16 value, FILE * F)
{
	fputc((int) (value & 0xFF), F);
	fputc((int) (value >> 8), F);
}

uint16
fgetw(FILE * F)
{
	return (uint16) (fgetc(F) + (fgetc(F) << 8));
}

freezed_value list_to_freeze[MAX_FREEZED_VALUE];
// List of all the value to freeze

uchar current_freezed_values;
// Current number of values to freeze

static uchar
bigindextobank(uint32 index)
{
	if (index < 0x8000)
		return 0;
	if (index < 0x18000)
		return ((index - 0x8000) >> 13) + 1;
	if (index < 0x48000)
		return ((index - 0x18000) >> 13) + 10;
	return 0;
}

uint16
bigtosmallindex(uint32 index)
{
	if (index < 0x8000)
		return (uint16) index;
	return (uint16) (index & 0x1FFF);
}

uchar
readindexedram(uint32 index)
{
	if (index < 0x8000)
		return RAM[index];
	if (index < 0x18000)
		return cd_extra_mem[index - 0x8000];
	if (index < 0x48000)
		return cd_extra_super_mem[index - 0x18000];

	return 0;

}

void
writeindexedram(uint32 index, uchar value)
{
	if (index < 0x8000)
		RAM[index] = value;
	else if (index < 0x18000)
		cd_extra_mem[index - 0x8000] = value;
	else if (index < 0x48000)
		cd_extra_super_mem[index - 0x18000] = value;

}


/*****************************************************************************

    Function: pokebyte

    Description: set a value in the ram to a specified value
    Parameters: none
    Return: 0

*****************************************************************************/

char
pokebyte()
{
	char tmp_str[10], new_val;
	uchar index = 0;
	unsigned addr;

	while (osd_keypressed())
		/*@-retvalother */
		osd_readkey();			// Flushing keys
	/*@=retvalother */
	while ((index < 10)
		   && ((tmp_str[index++] = (char) (osd_readkey() & 0xFF)) != 13));
	tmp_str[index - 1] = 0;
	addr = (unsigned) atoi(tmp_str);

	while (osd_keypressed())
		/*@-retvalother */
		osd_readkey();			// Flushing keys
	/*@=retvalother */
	index = 0;
	while ((index < 10)
		   && ((tmp_str[index++] = (char) (osd_readkey() & 0xFF)) != 13));
	tmp_str[index - 1] = 0;
	new_val = atoi(tmp_str);

	writeindexedram(addr, (uchar) new_val);

	{
		char *tmp_buf = (char *) malloc(100);
		snprintf(tmp_buf, 100, "Byte at %d set to %d", addr, new_val);
		osd_gfx_set_message(tmp_buf);
		message_delay = 180;
		free(tmp_buf);
	}


	return 0;
}

/*****************************************************************************

    Function: searchbyte

    Description: search the ram for a particuliar value
    Parameters: none
    Return: 1 on error

*****************************************************************************/
char
searchbyte()
{
	uint32 MAX_INDEX;
	char tmp_str[10];
	uint32 index = 0, tmp_word, last_index;
	uchar bank;
	char data_filename[80], old_filename[80];
	char first_research = 1;
	FILE *D, *O;
	Sint16 to_search;

	MAX_INDEX = (uint16) (CD_emulation ? 0x48000 : 0x8000);

	while (osd_keypressed())
		/*@-retvalother */
		osd_readkey();			// Flushing keys
	/*@=retvalother */

	while ((index < 10)
		   && ((tmp_str[index++] = (char) (osd_readkey() & 0xFF)) != 13));
	tmp_str[index - 1] = 0;

	to_search = atoi(tmp_str);

	strcpy(old_filename, short_cart_name);
	strcpy(data_filename, short_cart_name);
	strcat(data_filename, "FP1");

	O = fopen((char *) strcat(old_filename, "FP0"), "rb");
	if (O == NULL)
		first_research = 1;
/*  
  if (exists ((char *) strcat (old_filename, "FP0")))
    {
      if (!(O = fopen (old_filename, "rb")))
	return 1;
      first_research = 0;
    }
*/
	if (!(D = fopen(data_filename, "wb")))
		return 1;
	tmp_str[9] = 0;


	if ((tmp_str[0] != '-')
		&& (tmp_str[0] != '+')) {	/* non relative research */

		for (index = 0; index < MAX_INDEX; index++) {
			if (readindexedram(index) == (uchar) to_search) {

				if (first_research) {
					fputc(readindexedram(index), D);
					fputc(bigindextobank(index), D);
					fputw(bigtosmallindex(index), D);
				} else {
					while (!(feof(O))) {
						fgetc(O);
						bank = (uchar) fgetc(O);
						tmp_word = fgetw(O);

						if ((bank > bigindextobank(index))
							||
							((bank == bigindextobank(index))
							 && (tmp_word >= bigtosmallindex(index)))) {
							fseek(O, -4, SEEK_CUR);
							break;
						}
					}

					if ((bigtosmallindex(index) == (uint16) tmp_word) &&
						(bigindextobank(index) == bank)) {
						fputc(to_search, D);
						fputc(bigindextobank(index), D);
						fputw(bigtosmallindex(index), D);
						last_index = index;
					}
				}				//search for old occurences
			}
		}
	} else {					/* relative research */
		for (index = 0; index < MAX_INDEX; index++) {

			if (first_research) {
				fputc(readindexedram(index), D);
				fputc(bigindextobank(index), D);
				fputw(bigtosmallindex(index), D);
			} else {
				uchar old_value;

				{
					while (!(feof(O))) {
						fgetc(O);
						bank = (uchar) fgetc(O);
						tmp_word = fgetw(O);

						if ((bank > bigindextobank(index))
							||
							((bank == bigindextobank(index))
							 && (tmp_word >= bigtosmallindex(index)))) {
							fseek(O, -4, SEEK_CUR);
							break;
						}
					}

					if ((bigindextobank(index) == bank) &&
						(bigtosmallindex(index) == (uint16) tmp_word) &&
						(readindexedram(index) == old_value + to_search)) {
						fputc(readindexedram(index), D);
						fputc(bigindextobank(index), D);
						fputw(bigtosmallindex(index), D);
						last_index = index;
					}
				}				//search for old occurences
			}
		}

	}

	if (!first_research)
		fclose(O);
	fclose(D);

	rename(data_filename, old_filename);
	if (!file_size(old_filename)) {
		osd_gfx_set_message("Search failed");
		message_delay = 180;
		unlink(old_filename);
		return 1;
	}

	if (file_size(old_filename) == 4) {
		char *tmp_buf = (char *) malloc(100);
		snprintf(tmp_buf, 100, "Found at %d", last_index);
		osd_gfx_set_message(tmp_buf);
		message_delay = 60 * 5;
		free(tmp_buf);
	} else {
		osd_gfx_set_message("Still need to search");
		message_delay = 60 * 3;
	}

	while (osd_keypressed())
		/*@-retvalother */
		osd_readkey();			// Flushing keys
	/*@=retvalother */

	return 0;
}

/*****************************************************************************

    Function: loadgame

    Description: load a saved game using allegro packing functions
    Parameters: none
    Return: 1 on error, else 0

*****************************************************************************/
int
loadgame()
{
	FILE *saved_file;

	saved_file = fopen(sav_path, "rb");

	if (saved_file == NULL)
		return 1;

	if (fread(hard_pce, 1, sizeof(struct_hard_pce), saved_file) !=
		sizeof(struct_hard_pce)) {
		fclose(saved_file);
		return 1;
	}

	{
		int mmr_index;

		for (mmr_index = 0; mmr_index < 8; mmr_index++) {
			bank_set((uchar) mmr_index, mmr[mmr_index]);
			printf("Setting bank %d to 0x%02x\n", mmr_index,
				   mmr[mmr_index]);
		}
	}

	fclose(saved_file);

	return 0;
}

/*****************************************************************************

    Function: savegame

    Description: save a game using allegro packing functions
    Parameters: none
    Return: 1 on error, else 0

*****************************************************************************/


int
savegame()
{
	FILE *saved_file;

	saved_file = fopen(sav_path, "wb");

	if (saved_file == NULL)
		return 1;

	if (fwrite(hard_pce, 1, sizeof(struct_hard_pce), saved_file) !=
		sizeof(struct_hard_pce)) {
		fclose(saved_file);
		return 1;
	}

	fclose(saved_file);

	return 0;
}

/*****************************************************************************

    Function:  freeze_value

    Description: set or unset a value to freeze
    Parameters:none
    Return: 0 if unset an old freezed value or can't set a value anymore
 	         1 if new was created,
				may modify 'list_freezed_value'

*****************************************************************************/
int
freeze_value(void)
{
	char tmp_str[10];
	uchar index = 0;
	unsigned where;

	while (osd_keypressed())
		/*@-retvalother */
		osd_readkey();			// Flushing keys
	/*@=retvalother */

	while ((index < 10)
		   && ((tmp_str[index++] = (char) (osd_readkey() & 0xFF)) != 13));
	tmp_str[index - 1] = 0;

	where = (unsigned) atoi(tmp_str);

	for (index = 0; index < current_freezed_values; index++)
		if (list_to_freeze[index].position == (unsigned short) where) {
			// We entered an already freezed offset

			memcpy(&list_to_freeze[index], &list_to_freeze[index + 1],
				   (current_freezed_values - index +
					1) * sizeof(freezed_value));
			// We erase the current struct letting no hole...

			current_freezed_values--;
			// And we got one less value

			return 0;
		}

	if (current_freezed_values < MAX_FREEZED_VALUE) {
		list_to_freeze[current_freezed_values].position =
			(unsigned short) where;

		while ((index < 10)
			   && ((tmp_str[index++] = (char) (osd_readkey() & 0xFF)) !=
				   13));
		tmp_str[index - 1] = 0;

		list_to_freeze[current_freezed_values++].value =
			(unsigned) atoi(tmp_str);

		return 1;
	} else
		return 0;
}
