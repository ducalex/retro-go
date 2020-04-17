/**************************************************************************/
/*                                                                        */
/*             'Hu-Go! Compact disk Definition' handling code             */
/*                                                                        */
/*            This is 'copyleft'. Use and abuse it as you want.           */
/*                                                                        */
/**************************************************************************/
/* Authors:                                                               */
/*		1999 - Zerograd?                                                  */
/*		2011 - Alexander von Gluck, kallisti5@unixzen.com                 */
/**************************************************************************/

#include "hcd.h"

#if 0

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "iniconfig.h"
#include "utils.h"


uint32 HCD_first_track;
uint32 HCD_last_track;

FILE *HCD_iso_FILE = 0;

uint32 HCD_current_subtitle = 0;
uint32 HCD_frame_at_beginning_of_track = 0;
uchar HCD_current_played_track = 0;


static int
get_HCD_path(char* hcdFile, char* result)
{
	// we write our own dirname to stay portable
	char fullPath[PATH_MAX + 1];
	realpath(hcdFile, fullPath);

	const char slash = '/';

	char *lastSlash = strrchr(fullPath, slash);
	if (lastSlash)
		lastSlash[1] = '\0';
	else
		snprintf(fullPath, 3, ".%s", slash);

	strncpy(result, fullPath, PATH_MAX);
}


int
fill_HCD_info(char *name)
{
	int current_track;

	set_config_file(name);

	init_config();
	HCD_first_track = get_config_int("main", "first_track", 1);
	HCD_last_track = get_config_int("main", "last_track", 22);

	current_track = get_config_int("main", "minimum_bios", -1);

	if (current_track != -1)
		minimum_bios_hooking = current_track;

	for (current_track = HCD_first_track;
		 current_track <= HCD_last_track; current_track++) {

		char *section_name = (char *) alloca(100);
		char *tmp_buf = (char *) alloca(100);

		sprintf(section_name, "track%d", current_track);

// ----- Init patch section -----

		CD_track[current_track].patch_number = 0;
		CD_track[current_track].patch = NULL;

// ----- find beginning ---------

		get_config_string(section_name, "begin", "", tmp_buf);

		if (strcasestr(tmp_buf, "MSF")) {
			int min = (tmp_buf[4] - '0') * 10 + (tmp_buf[5] - '0');
			int sec = (tmp_buf[7] - '0') * 10 + (tmp_buf[8] - '0');
			int fra = (tmp_buf[10] - '0') * 10 + (tmp_buf[11] - '0');

			CD_track[current_track].beg_lsn = fra + 75 * (sec + 60 * min);
			CD_track[current_track].beg_lsn -= 150;
		} else if (strcasestr(tmp_buf, "LSN")) {
			CD_track[current_track].beg_lsn = atoi(&tmp_buf[4]) - 150;
		} else if (strcasestr(tmp_buf, "LBA")) {
			CD_track[current_track].beg_lsn = atoi(&tmp_buf[4]);
		} else {
			CD_track[current_track].beg_lsn = 0;
		}

		get_config_string(section_name, "type", "AUDIO", tmp_buf);

		if (strcmp(tmp_buf, "CODE") == 0) {
			// FOUND: CODE TRACK
			char ISO_path[PATH_MAX + 1];
			get_HCD_path(name, ISO_path);
			// find where ISO's live

			CD_track[current_track].type = 4;
			// emulated track will use a regular file

			CD_track[current_track].source_type = HCD_SOURCE_REGULAR_FILE;

			strcpy(tmp_buf, ISO_path);
			// search filename of ISO

			char buffer[BUFSIZ];
			get_config_string(section_name, "filename", "", buffer);
			strcat(tmp_buf, buffer);
			strcpy(CD_track[current_track].filename, tmp_buf);

			struct stat rom_file_buf;

			if (stat(tmp_buf, &rom_file_buf) != 0) {
				MESSAGE_ERROR("Error : %X when stating %s\n", errno,
							  tmp_buf);
				return 0;
			}

			CD_track[current_track].length = file_size(tmp_buf) / 2048;
			// Determine file length of track

			CD_track[current_track].patch_number
				= get_config_int(section_name, "patch_number", 0);
			// Search for patch

			if (CD_track[current_track].patch_number) {
				uint32 i;
				char tmp_str[256];

				CD_track[current_track].patch
					= (PatchEntry *) malloc(sizeof(PatchEntry)
											*
											CD_track[current_track].
											patch_number);

				for (i = 0; i < CD_track[current_track].patch_number; i++) {
					sprintf(tmp_str, "patch%d", i);
					get_config_string(section_name, tmp_str,
									  "0XFFFFFFFF, 0XFF", tmp_str);
					sscanf(tmp_str, "%X,%X",
						   &CD_track[current_track].patch[i].offset,
						   &CD_track[current_track].patch[i].new_val);
					MESSAGE_INFO
						("Track #%d - Patch - replaced bytes at offset"
						 " 0x%X with 0x%X\n", i,
						 CD_track[current_track].patch[i].offset,
						 CD_track[current_track].patch[i].new_val);
				}
			}

			MESSAGE_INFO("Track #%d - Program Code - %s\n",
						 current_track, CD_track[current_track].filename);

		} else if (strcmp(tmp_buf, "CD") == 0) {
			// FOUND: HARDWARE CD
			int min, sec, fra, control;
			int min_end, sec_end, fra_end, control_end;

			CD_track[current_track].source_type = HCD_SOURCE_CD_TRACK;

			get_config_string(section_name, "drive", "0", tmp_buf);
			osd_cd_init(tmp_buf);

			//------- looking for the index of the track to use for emulation ----

			CD_track[current_track].filename[0]
				= get_config_int(section_name, "track", current_track);

			//------- looking for the control byte and deducing type ------

			osd_cd_track_info(CD_track[current_track].filename[0],
							  &min, &sec, &fra, &control);

			/* TODO : add support if track is in last position */

			osd_cd_track_info(CD_track[current_track].filename[0] + 1,
							  &min_end, &sec_end, &fra_end, &control_end);

			CD_track[current_track].length = Time2Frame(min_end,
														sec_end,
														fra_end) -
				Time2Frame(min, sec, fra);

			CD_track[current_track].type = (control & 4);
		} else {
			// FOUND: OTHER (AUDIO)
			char audio_path[PATH_MAX];
			get_HCD_path(name, audio_path);
			//find where audio tracks live

			CD_track[current_track].type = 0;	// Audio track
			CD_track[current_track].source_type = HCD_SOURCE_REGULAR_FILE;

			strcpy(tmp_buf, audio_path);
			char fileNameBuffer[PATH_MAX];
			get_config_string(section_name, "filename", "",
				fileNameBuffer);
			strcat(tmp_buf, fileNameBuffer);
			strcpy(CD_track[current_track].filename, tmp_buf);

			// START Search for subtitles
			CD_track[current_track].subtitle_synchro_type
				= get_config_int(section_name, "synchro_mode", 0);

			CD_track[current_track].subtitle_number
				= get_config_int(section_name, "subtitle_number", 0);

			if (CD_track[current_track].subtitle_number) {
				uint32 i;
				char tmp_str[256];

				CD_track[current_track].subtitle
					= (SubtitleEntry *) malloc(sizeof(SubtitleEntry)
											   *
											   CD_track[current_track].
											   subtitle_number);

				for (i = 0; i < CD_track[current_track].subtitle_number;
					 i++) {
					sprintf(tmp_str, "subtitle%d", i);
					get_config_string(section_name, tmp_str, "0,0,",
									  tmp_str);

					sscanf(tmp_str, "%d,%d",
						   &CD_track[current_track].subtitle[i].StartTime,
						   &CD_track[current_track].subtitle[i].Duration);

					if (strrchr(tmp_str, ',')) {
						memset(CD_track[current_track].subtitle[i].data, 0,
							   32);
						strncpy(CD_track[current_track].subtitle[i].data,
								strrchr(tmp_str, ',') + 1, 31);
					}

					MESSAGE_INFO
						("Track #%d - Subtitle #%d - %s begins at %d, "
						 "lasts %d ms\n", current_track, i,
						 CD_track[current_track].subtitle[i].data,
						 CD_track[current_track].subtitle[i].StartTime,
						 CD_track[current_track].subtitle[i].Duration);
				}
			}
			// END Search for Subtitles

			//----- use Tormod's work to guess MP3 length ---------

			if (strcasestr(CD_track[current_track].filename, ".mp3")) {
				CD_track[current_track].length
					=
					(int) (MP3_length(CD_track[current_track].filename) *
						   75.0);

				MESSAGE_INFO
					("Track #%d - MP3 Audio - %s is %d 75th of a second long\n",
					 current_track, CD_track[current_track].filename,
					 CD_track[current_track].length);
			}
			// littletux: use ogginfo hack to guess OGG length
			if (strcasestr(CD_track[current_track].filename, ".ogg")) {
				CD_track[current_track].length
					= (int) (OGG_length(CD_track[current_track].filename));

				MESSAGE_INFO
					("Track #%d - OGG Audio - %s is %d 75th of a second long\n",
					 current_track, CD_track[current_track].filename,
					 CD_track[current_track].length);
			}

			if (CD_track[current_track].length == 0)
				CD_track[current_track].length = 30 * 75;	// 30 sec track
		}


		// ----- if begin hasn't been specified ---------
		if (!CD_track[current_track].beg_lsn) {
			if (current_track == 1)
				CD_track[current_track].beg_lsn = 0;
			else
				CD_track[current_track].beg_lsn
					= CD_track[current_track - 1].beg_lsn
					+ CD_track[current_track - 1].length;
		}
		// ----- convert beginning in PCE msf format ---------
		{
			int min, sec, fra;
			Frame2Time(CD_track[current_track].beg_lsn + 150, &min, &sec,
					   &fra);

			CD_track[current_track].beg_min = binbcd[min];
			CD_track[current_track].beg_sec = binbcd[sec];
			CD_track[current_track].beg_fra = binbcd[fra];
		}

	}

	// restores right file for hugo config
	set_config_file_back();

	return 1;
}


void
HCD_pause_playing()
{
	if (Mix_PlayingMusic())
		Mix_HaltMusic();
};


void
HCD_play_track(uchar track, char repeat)
{
	switch (CD_track[track].subtitle_synchro_type) {
	case 0:					// frame synchronisation
		HCD_frame_at_beginning_of_track = (uint32) frame;
		break;
	case 1:					// timer synchronisation
		HCD_frame_at_beginning_of_track = timer_60;
		break;
	}

	HCD_current_played_track = (uchar) track;
	HCD_current_subtitle = 0;

	switch (CD_track[track].source_type) {
	case HCD_SOURCE_CD_TRACK:
		osd_cd_play_audio_track((uchar) (CD_track[track].filename[0]));
		break;
	case HCD_SOURCE_REGULAR_FILE:
		if ((strcasestr(CD_track[track].filename, ".mp3"))
			|| (strcasestr(CD_track[track].filename, ".ogg"))
			|| (strcasestr(CD_track[track].filename, ".wav"))) {
			Mix_PlayMusic(sdlmixmusic[track], repeat);
		}
		break;
	}
};


void
HCD_play_sectors(int begin_sect, int sect_len, char repeat)
{
	int result;

	for (result = nb_max_track; result; result--) {
		if (((uint32) begin_sect >= CD_track[result].beg_lsn)
			&& ((unsigned) begin_sect <= CD_track[result].beg_lsn
				+ CD_track[result].length))
			break;
	}

	if (CD_track[result].source_type == HCD_SOURCE_CD_TRACK) {
		uint32 min_from, sec_from, fra_from;
		uint32 min_to, sec_to, fra_to;
		uint32 min_real, sec_real, fra_real, dummy;
		begin_sect -= CD_track[result].beg_lsn;

		/* begin_sect is now relative to the begin of the track to play */
		Frame2Time((unsigned) begin_sect, (int *) &min_from,
				   (int *) &sec_from, (int *) &fra_from);

		sect_len += begin_sect;

		/* sect_len is now also relative to the begin of the track to play */
		Frame2Time((unsigned) sect_len, (int *) &min_to, (int *) &sec_to,
				   (int *) &fra_to);

		osd_cd_track_info((uchar) (CD_track[result].filename[0]),
						  (int *) &min_real, (int *) &sec_real,
						  (int *) &fra_real, (int *) &dummy);

		min_from += min_real;
		sec_from += sec_real;
		fra_from += fra_real;

		min_to += min_real;
		sec_to += sec_real;
		fra_to += fra_real;

		if (fra_to > 75) {
			fra_to -= 75;
			sec_to++;
		}

		if (fra_from > 75) {
			fra_from -= 75;
			sec_from++;
		}

		if (sec_to > 60) {
			sec_to -= 60;
			min_to++;
		}

		if (sec_from > 60) {
			sec_from -= 60;
			min_from++;
		}

		osd_cd_play_audio_range(min_from, sec_from, fra_from,
								min_to, sec_to, fra_to);
	}

	if (CD_track[result].source_type == HCD_SOURCE_REGULAR_FILE) {
		if (strcasestr(CD_track[result].filename, ".mp3")
			|| strcasestr(CD_track[result].filename, ".ogg")) {
			if (-150 < begin_sect - CD_track[result].beg_lsn
				&& begin_sect - CD_track[result].beg_lsn < 150)
				Mix_PlayMusic(sdlmixmusic[result], repeat);
			else				/* can't yet easily repeat "inside" a track */
				Mix_PlayMusic(sdlmixmusic[result], 0);

			Mix_RewindMusic();
			if (Mix_SetMusicPosition
				((begin_sect - CD_track[result].beg_lsn)
				 / 75) == -1) {
				printf("Mix_SetMusicPosition(): %s\n", Mix_GetError());
			}

			HCD_frame_at_beginning_of_track = frame
				- (begin_sect - CD_track[result].beg_lsn) / 75.0 * 60.0;

			/* estimate the number of cycle that should have elapsed since
			   the beginning of the track */
			HCD_current_played_track = result;
			HCD_current_subtitle = 0;
		} else if (strcasestr(CD_track[result].filename, ".WAV")) {
			// TODO: There was only an MSDOS case here? :(
		}
	}
};


void
HCD_shutdown()
{
	int current_track;

	for (current_track = (int) HCD_first_track;
		 current_track <= (int) HCD_last_track; current_track++) {
		free(CD_track[current_track].patch);
		free(CD_track[current_track].subtitle);
	}

};

void
HCD_handle_subtitle()
{

	if (HCD_current_subtitle >=
		CD_track[HCD_current_played_track].subtitle_number)
		return;

	switch (CD_track[HCD_current_played_track].subtitle_synchro_type) {
	case 0:
		if (frame - HCD_frame_at_beginning_of_track >=
			(int) (CD_track[HCD_current_played_track].subtitle
				   [HCD_current_subtitle].StartTime)) {
			osd_gfx_set_message(CD_track[HCD_current_played_track].
								subtitle[HCD_current_subtitle].data);
			message_delay =
				CD_track[HCD_current_played_track].
				subtitle[HCD_current_subtitle].Duration;
			HCD_current_subtitle++;
		}
		break;

	case 1:

		if (timer_60 - HCD_frame_at_beginning_of_track >=
			CD_track[HCD_current_played_track].subtitle
			[HCD_current_subtitle].StartTime)
		{

			osd_gfx_set_message(CD_track[HCD_current_played_track].subtitle
								[HCD_current_subtitle].data);

			message_delay =
				CD_track[HCD_current_played_track].subtitle
				[HCD_current_subtitle].Duration;

			HCD_current_subtitle++;

		}

		break;
	}

}

void
read_sector_HCD(uchar * p, uint32 dum)
{
	int result;

	for (result = (int) HCD_last_track; result > 0x01; result--) {
		if ((pce_cd_sectoraddy >= CD_track[result].beg_lsn) &&
			(pce_cd_sectoraddy <= CD_track[result].beg_lsn +
			 CD_track[result].length))
			break;
	}

	if (CD_track[result].source_type == HCD_SOURCE_REGULAR_FILE)
		HCD_iso_read_sector(p, dum, (uint32) result);
	else if (CD_track[result].source_type == HCD_SOURCE_CD_TRACK)
		HCD_cd_read_sector(p, dum, (uint32) result);

}

void
HCD_iso_read_sector(uchar * p, uint32 dum, uint32 result)
{

	static int current_position = 0;
	static int current_file = 0;
	static int current_type = 0;

#ifndef FINAL_RELEASE
	fprintf(stderr, "Loading sector nø%d.\n", pce_cd_sectoraddy);
	Log("Loading sector nø%d.\n", pce_cd_sectoraddy);
	Log("HCD : seek at %d\n",
		(pce_cd_sectoraddy - CD_track[result].beg_lsn) * 2048);
	Log("Track nø%d begin at %d\n", result, CD_track[result].beg_lsn);
	Log("Track nø2 begin at %d\n", CD_track[2].beg_lsn);
#endif

  label_are_bad:

	if (result == current_file) {
		if (current_type == 1)	// non compacted
		{
			fseek(HCD_iso_FILE,
				  (long) ((pce_cd_sectoraddy - CD_track[result].beg_lsn)
						  * 2048), SEEK_SET);

			if (fread(p, 2048, 1, HCD_iso_FILE) != 2048)
				Log("Error when reading sector %d in hcd\n",
					pce_cd_sectoraddy);

			current_position = ftell(HCD_iso_FILE);	// not useful

		} else {
			Log("Open mode in HCD read function incorrect\n");
			exit(-2);
		}

	}							// file well opened

	else
	{							// must open file

		if (strcasestr(CD_track[result].filename, ".ISO"))
		{

			HCD_iso_FILE = fopen(CD_track[result].filename, "rb");



#ifndef FINAL_RELEASE

			Log("File tried to open\n");

#endif



			if (!HCD_iso_FILE)
			{



				Log("ISO file not found : %s\nUsed for track %d\n",
					CD_track[result].filename, result);



				exit(-3);



			}



			current_file = result;

			current_type = 1;

			current_position = 0;

		}
		goto label_are_bad;
	}

	{
		int dummy;
		for (dummy = 0; dummy < (int) (CD_track[result].patch_number);
			 dummy++) {
			if ((CD_track[result].patch[dummy].offset >> 11) ==
				pce_cd_sectoraddy - CD_track[result].beg_lsn)
				p[CD_track[result].patch[dummy].offset & 2047] =
					CD_track[result].patch[dummy].new_val;

		}

	}

}

void
HCD_cd_read_sector(uchar * p, uint32 dum, uint32 result)
{
	uint32 min, sec, fra, control;

	osd_cd_track_info((uchar) CD_track[result].filename[0],
					  (int *) &min,
					  (int *) &sec, (int *) &fra, (int *) &control);



	osd_cd_read(p,
				dum - CD_track[result].beg_lsn + Time2HSG((int) min,
														  (int) sec,
														  (int) fra));

}

#else

uint32 HCD_first_track;
uint32 HCD_last_track;

int
fill_HCD_info(char *name)
{
    return 0;
}

void
HCD_play_track(uchar track, char repeat)
{
}

void
HCD_play_sectors(int begin_sect, int sect_len, char repeat)
{
}

void
HCD_pause_playing()
{
}

void
read_sector_HCD(uchar * p, uint32 dum)
{
}
void
HCD_handle_subtitle()
{
}
#endif
