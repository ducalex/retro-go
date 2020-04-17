/*
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *	GNU Library General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


#include "iniconfig.h"

#include "utils.h"

static int default_joy_mapping[J_MAX] = {0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, -1, -1, -1, -1, -1, -1, -1, -1};

//! Filename of the cd system rom
char cdsystem_path[PCE_PATH_MAX];

char sCfgFileLine[BUFSIZ];
// line buffer for config reading

char *config_file;//[PATH_MAX];
char *config_file_tmp;//[PATH_MAX];
// name of the config file

const int config_ar_size_max = 2000;
// number max of variable read off the configuration file

int config_ar_index = 0;
// next entry where to insert variable / size of the config variable array

typedef struct
{
	char *section;
	char *variable;
	char *value;
} config_var;

config_var *config_ar;
// actual array toward the entries


int
config_var_cmp(const void *lhs, const void *rhs)
{
	int section_cmp
		= stricmp(((config_var *) lhs)->section, ((config_var *) rhs)->section);

	if (section_cmp != 0)
		return section_cmp;

	return stricmp(((config_var *) lhs)->variable,
		((config_var *) rhs)->variable);
}


void
set_config_file(const char *filename)
{
	strcpy (config_file_tmp, config_file);
	strcpy (config_file, filename);
}


void
set_config_file_back(void)
{
	strcpy (config_file, config_file_tmp);
}


int
init_config(void)
{
	FILE *FCfgFile = NULL;
	char *pWrd = NULL;
	char *pRet;
	char *pTmp;
	char *section = NULL;

	config_ar_index = 0;
	if ((config_ar
	= (config_var *) malloc (sizeof (config_var) * config_ar_size_max))
		== NULL)
	return 0;

	/* open config file for reading */
	if ((FCfgFile = fopen (config_file, "r")) != NULL) {
	do {
		memset (sCfgFileLine, '\0', BUFSIZ);
		/* note.. line must NOT be a comment */
		pRet = fgets (sCfgFileLine, BUFSIZ, FCfgFile);

		if (sCfgFileLine[0] == '#')
		continue;

		if (sCfgFileLine[0] == '[') {
		int section_size;
		pWrd = strrchr (sCfgFileLine, ']');
		if (pWrd == NULL)	/* Badly formed section line */
			continue;

		if (section != NULL)
			free (section);

		section_size = pWrd - sCfgFileLine;
		section = (char *) malloc (section_size);
		strncpy (section, sCfgFileLine + 1, section_size - 1);
		section[section_size - 1] = '\0';
		continue;
		}

		pWrd = strchr (sCfgFileLine, '=');
		if (pWrd == NULL)
		continue;

		pTmp = strchr (pWrd, '\n');
		if (pTmp != NULL)
		*pTmp = '\0';

		if (config_ar_index < config_ar_size_max) {
		config_ar[config_ar_index].section = (char *) strdup (section);

		*pWrd = '\0';
		pTmp = pWrd - 1;
		while (*pTmp == '\t' || *pTmp == ' ')
			*(pTmp--) = '\0';

		config_ar[config_ar_index].variable
			= (char *) strdup (sCfgFileLine);

		while (*pWrd == '\t' || *pWrd == ' ')
			pWrd++;

		config_ar[config_ar_index].value = (char *) strdup (pWrd + 1);

		config_ar_index++;
		}

	} while (pRet != NULL);

	fclose (FCfgFile);
	}

	if (section != NULL)
	free (section);

	qsort(config_ar, config_ar_index, sizeof (config_var), config_var_cmp);

	return 1;
}


void
dispose_config(void)
{
	int index;

	for (index = 0; index < config_ar_index; index++) {
	free (config_ar[index].section);
	free (config_ar[index].variable);
	free (config_ar[index].value);
	}

	free (config_ar);
}


int
get_config_var(char* section, char* variable, char* result)
{
	config_var key, *keyResult;

	key.section = section;
	key.variable = variable;

	keyResult = bsearch(&key, config_ar, config_ar_index,
		sizeof (config_var), config_var_cmp);

	if (keyResult != NULL) {
		strcpy(result, keyResult->value);
		return 1;
	}

	return 0;
}


// Let's redefine the old allegro parsing function

int
get_config_int(const char *section, const char *keyword, int default_value)
{
	char buffer[BUFSIZ];

	if (!get_config_var(section, keyword, buffer))
		return default_value;

	return atoi(buffer);
}


void
get_config_string(const char *section, const char *keyword, const char *default_value,
	char* result)
{
	if (!get_config_var(section, keyword, result))
		strcpy(result, default_value);
}


void
read_joy_mapping(void)
{
	char tmp_str[10], tmp_str2[10], section_name[10];
	uchar x, y, z;
	unsigned short temp_val;

	Log ("--[ JOYPAD MAPPING ]-------------------------------\n");
	Log ("Loading default values\n");

	memset (tmp_str, 0, 10);

	strcpy (section_name, "CONFIG1");
	for (z = 0; z < 16; z++) {
	if (z < 10)
		section_name[6] = '0' + z;
	else
		section_name[6] = (z - 10) + 'a';

	Log (" * Looking for section %s\n", section_name);

	for (x = 0; x < 5; x++) {
		// for each player

		config[z].individual_config[x].joydev = 0;
		strcpy (tmp_str2, "joydev1");
		tmp_str2[6] = '0' + x;

		char buffer[BUFSIZ];
		get_config_string(section_name, tmp_str2, "0", buffer);
		strncpy(tmp_str, buffer, 10);

		config[z].individual_config[x].joydev = atoi (tmp_str);

		for (y = 0; y < J_MAX; y++) {
		strncpy (tmp_str, joymap_reverse[y], 10);
		tmp_str[strlen (tmp_str) + 1] = 0;
		tmp_str[strlen (tmp_str)] = '0' + x;
		temp_val = get_config_int (section_name, tmp_str, 0xffff);

		if (0xffff != temp_val) {
			config[z].individual_config[x].joy_mapping[y] = temp_val;
			Log ("		%s set to %d\n", joymap_reverse[y], temp_val);
		}
		}

	}
	}

	Log ("End of joypad mapping\n\n");
}


char
set_arg(char nb_arg, const char *val) {

	if (!val && (nb_arg == 'i' || nb_arg == 't' || nb_arg == 'w'
		|| nb_arg == 'c' || nb_arg == 'z')) {
		MESSAGE_ERROR("No value provided for %c arg\n", nb_arg);
		return 1;
	}

	switch (nb_arg)
	{
	case 'a':
		option.want_fullscreen_aspect = 1;
		MESSAGE_INFO("Option: Fullscreen aspect ratio enabled\n");
		return 0;

	case 'c':
		CD_emulation = atoi(val);
		MESSAGE_INFO("Option: Forcing CD emulation to mode %d\n", atoi(val));
		return 0;

	case 'd':
		debug_on_beginning = atoi(val);
		Log ("Option: Debug on beginning set to %d\n", debug_on_beginning);
		return 0;

	case 'e':
		use_eagle = atoi(val);
		Log ("Eagle mode set to %d\n", use_eagle);
		return 0;

	case 'f':
		option.want_fullscreen = 1;
		MESSAGE_INFO("Option: Fullscreen mode enabled\n");
		return 0;

	case 'i':
		strcpy (ISO_filename, val);
		Log ("ISO filename is %s\n", ISO_filename);
		return 0;

	case 'm':
		minimum_bios_hooking = atoi (val);
		Log ("Minimum Bios hooking set to %d\n", minimum_bios_hooking);
		return 0;

#if defined(ENABLE_NETPLAY)
	case 'n':
#warning hardcoding of netplay protocol
/*			 option.want_netplay = LAN_PROTOCOL; */
		option.want_netplay = INTERNET_PROTOCOL;
		strncpy(option.server_hostname, val, sizeof(option.server_hostname));
		Log ("Netplay mode enabled\nServer hostname set to %s\n",
		option.server_hostname);
		return 0;
#endif // NETPLAY

	case 'S':
		use_scanline = MIN(1, MAX(0, atoi(val)));
		Log ("Scanline mode set to %d\n", use_scanline);
		return 0;
	case 'u':
		US_encoded_card = atoi(val);
		Log ("US Card encoding set to %d\n", US_encoded_card);
		return 0;
	case 't':
		nb_max_track = atoi(val);
		Log ("Number of tracks set to %d\n", nb_max_track);
		return 0;
	case 'z':
		option.window_size = MIN(1, MAX(4, atoi(val)));
		return 0;
	case 'h':
		printf(
		"\nHuExpress, the multi-platform PCEngine emulator\n"
		"Version %d.%d.%d\n"
		"Copyright 2011-2013 Alexander von Gluck IV\n"
		"Copyright 2001-2005 Zeograd\n"
		"Copyright 1998 BERO\n"
		"Copyright 1996 Alex Krasivsky, Marat Fayzullin\n\n"
		"Usage: huexpress <GAME> [arguments]\n\n"
		"Where <GAME> is an pce|iso|zip\n"
		"Where [arguments] are:\n"
		"	-cX  Force CD Emulation mode X\n"
		"	-dX  Debug (0-1)\n"
		"	-eX  Eagle mode (0-1)\n"
		"	-f   Fullscreen mode\n"
		"	-SX  Scanline mode (0-1)\n"
		"	-zX  Zoom level X (1-4)\n"
		"\n", VERSION_MAJOR, VERSION_MINOR, VERSION_UPDATE);
		return 1;

	default:
		MESSAGE_ERROR("Unrecognized option : %c\n", nb_arg);
		return 1;
	}
}


int
parse_commandline(int argc, char **argv)
{
	char arg_value, i, arg_error = 0;

	for (i = 1; i < argc; i++) {
		// first option should always be our game
		if (i == 1 && argv[i][0] != '-') {
			strcpy (cart_name, argv[i]);
			int x;
			for (x = 0; x < strlen (cart_name); x++)
			if (cart_name[x] == '\\')
				cart_name[x] = '/';
		} else {
			if (argv[i][0] == '-') {
				// if argument
				if (strlen(argv[i]) > 2) {
					arg_error |= set_arg(argv[i][1], (char *) &argv[i][2]);
				} else {
					arg_error |= set_arg(argv[i][1], NULL);
				}
			} else {
				MESSAGE_ERROR("Unknown option: %s\n", argv[i]);
				arg_error = 1;
			}
		}
	}

	if (arg_error)
		return 1;

	video_driver = 0;

	if (use_eagle)
		video_driver = 1;
	else if (use_scanline)
		video_driver = 2;

	return 0;
}


void
parse_INIfile_raw ()
{
	Log ("Looking in %s\n", config_file);

	read_joy_mapping ();

	char buffer[BUFSIZ];
	get_config_string ("main", "rom_dir", ".", buffer);
	strcpy(initial_path, buffer);

	if ((initial_path[0]) && (initial_path[strlen (initial_path) - 1] != '/')
		&& (initial_path[strlen (initial_path) - 1] != '\\')) {
		strcat (initial_path, "/");
	}

	// rom_dir setting
	Log ("Setting initial path to %s\n", initial_path);

	current_config = get_config_int ("main", "config", 0);
	// choose input config
	Log ("Setting joypad config number to %d\n", current_config);

	smode = get_config_int ("main", "smode", -1);
	// sound mode setting
	Log ("Setting sound mode to %d\n", smode);

	use_eagle = get_config_int ("main", "eagle", 0);
	// do we use EAGLE ?
	Log ("Setting eagle mode to %d\n", use_eagle);

	use_scanline = get_config_int ("main", "scanline", 0);
	// do we use EAGLE ?
	Log ("Setting scanline mode to %d\n", use_scanline);

	option.want_snd_freq = get_config_int ("main", "snd_freq", 22050);
	// frequency of the sound generator
	Log ("Setting default frequency to %d\n", option.want_snd_freq);

	sbuf_size = get_config_int ("main", "buffer_size", 512);
	// size of the sound buffer
	Log ("Setting sound buffer size to %d bytes\n", sbuf_size);

	gamepad_driver = get_config_int ("main", "joy_type", -1);
	Log ("Setting joy type to %d\n", gamepad_driver);

	sound_driver = get_config_int ("main", "sound_driver", 1);
	Log ("Setting sound driver to %d\n", sound_driver);

	synchro = get_config_int ("main", "limit_fps", 0);
	Log ("Setting fps limitation to %d\n", synchro);

	option.want_fullscreen = get_config_int ("main", "start_fullscreen", 0);
	Log ("Setting start in fullscreen mode to %d\n", option.want_fullscreen);

	option.want_fullscreen_aspect
		= get_config_int ("main", "use_fullscreen_aspect", 0);
	Log ("Setting fullscreen aspect to %d\n", option.want_fullscreen_aspect);

	option.window_size = get_config_int ("main", "window_size", 2);
	Log ("Setting window size to %d\n", option.window_size);

	option.wanted_hardware_format
		= get_config_int ("main", "hardware_format", 0);

	Log ("Setting wanted hardware format to %x\n",
		option.wanted_hardware_format);

	minimum_bios_hooking = get_config_int ("main", "minimum_bios_hooking", 0);

	Log ("Minimum Bios hooking set to %d\n", minimum_bios_hooking);

	memset(buffer, 0, BUFSIZ * sizeof(char));
	get_config_string("main", "cdsystem_path", "", buffer);
	strcpy (cdsystem_path, buffer);

	Log ("CD system path set to %d\n", cdsystem_path);

	memset(buffer, 0, BUFSIZ * sizeof(char));
	get_config_string("main", "cd_path", "", buffer);
	strcpy(ISO_filename, buffer);
	Log ("CD path set to %s\n", ISO_filename);

	option.want_arcade_card_emulation
		= get_config_int("main", "arcade_card", 1);
	Log ("Arcade card emulation set to %d\n",
		option.want_arcade_card_emulation);

	option.want_supergraphx_emulation
		= get_config_int("main", "supergraphx", 1);
	Log ("SuperGraphX emulation set to %d\n",
		option.want_supergraphx_emulation);

	option.want_television_size_emulation
		= get_config_int("main", "tv_size", 0);
	Log ("Limiting graphics size to emulate tv output set to %d\n",
		option.want_television_size_emulation);

	memset(buffer, 0, BUFSIZ * sizeof(char));
	get_config_string("main", "resource_path", "/usr/share/huexpress", buffer);
	strcpy(option.resource_location, buffer);
	Log ("Resource path set to %s\n", buffer);

	Log ("End of parsing INI file\n\n");
}


void
parse_INIfile ()
{
	Log ("--[ PARSING INI FILE ]------------------------------\n");

#ifndef LINUX
	sprintf(config_file, "%s/%s", config_basepath, SETTINGS_FILENAME);
#else
	{

		char tmp_home[256];
		FILE *f;

		sprintf(tmp_home, "%s/%s", config_basepath, SETTINGS_FILENAME);

		f = fopen(tmp_home, "rb");

		if (f != NULL) {
			strcpy (config_file, tmp_home);
			fclose (f);
		} else {
			sprintf(tmp_home, "/etc/%s", SETTINGS_FILENAME);
			strcpy(config_file, tmp_home);
		}
	}
#endif

	init_config();

	parse_INIfile_raw();

	dispose_config();
}


void
set_config_var_str(const char *section, const char *name, char *value)
{
	config_var key, *result;

#if !defined(FINAL_RELEASE)
	printf ("Setting [%s] %s to %s\n", section, name, value);
#endif

	key.section = section;
	key.variable = name;

	result = bsearch(&key, config_ar, config_ar_index,
		sizeof (config_var), config_var_cmp);

	if (result != NULL) {
		free (result->value);
		result->value = strdup (value);
		return;
	}

	if (config_ar_index < config_ar_size_max) {
			config_ar[config_ar_index].section = strdup (section);
			config_ar[config_ar_index].variable = strdup (name);
			config_ar[config_ar_index].value = strdup (value);

			config_ar_index++;

	} else {
		Log ("Couldn't set [%s]%s to %sn bit enough internal space\n", section,
			name, value);
	}
}


void
set_config_var_int(const char *section, const char *name, int value)
{
	char temp_string[10];

	snprintf (temp_string, 10, "%d", value);

	set_config_var_str (section, name, temp_string);
}


void
dump_config(char *filename)
{
	FILE *output_file;
	int local_index;
	char *last_section_name;

	last_section_name = "";

	output_file = fopen (filename, "wt");
	if (output_file == NULL)
		{
			Log ("Couldn't save configuration to %s\n", filename);
			return;
		}

	Log ("Saving %d entries in the configuration file\n", config_ar_index);

	for (local_index = 0; local_index < config_ar_index; local_index++) {
		if (strcmp (last_section_name, config_ar[local_index].section)) {
			last_section_name = config_ar[local_index].section;
			fprintf (output_file, "[%s]\n", last_section_name);
		}
		fprintf (output_file, "%s=%s\n", config_ar[local_index].variable,
			config_ar[local_index].value);
	}
	fclose (output_file);

}


//! makes the configuration changes permanent
void
save_config (void)
{

	char config_name[PATH_MAX];
	uchar input_config_number, input_config_button, input_config_player;

	// Reads all variables in the ini file
	init_config();

	set_config_var_str("main", "rom_dir", initial_path);
	set_config_var_int("main", "config", current_config);
	set_config_var_int("main", "smode", smode);
	set_config_var_int("main", "eagle", use_eagle);
	set_config_var_int("main", "scanline", use_scanline);
	set_config_var_int("main", "snd_freq", option.want_snd_freq);
	set_config_var_int("main", "buffer_size", sbuf_size);
	set_config_var_int("main", "joy_type", gamepad_driver);
	set_config_var_int("main", "sound_driver", sound_driver);
	set_config_var_int("main", "start_fullscreen", option.want_fullscreen);
	set_config_var_int("main", "use_fullscreen_aspect",
				option.want_fullscreen_aspect);
	set_config_var_int("main", "minimum_bios_hooking", minimum_bios_hooking);
	set_config_var_str("main", "cdsystem_path", cdsystem_path);
	set_config_var_str("main", "cd_path", ISO_filename);
	set_config_var_int("main", "window_size", option.window_size);
	set_config_var_int("main", "arcade_card",
				option.want_arcade_card_emulation);
	set_config_var_int("main", "supergraphx",
				option.want_supergraphx_emulation);
	set_config_var_int("main", "tv_size",
				option.want_television_size_emulation);
	set_config_var_int("main", "hardware_format",
				option.wanted_hardware_format);

	// For each input configuration ...
	for (input_config_number = 0; input_config_number < 16;
		input_config_number++) {
		char section_name[] = "CONFIG0";

		if (input_config_number < 10)
			section_name[6] = '0' + input_config_number;
		else
			section_name[6] = 'a' + input_config_number - 10;

		// For each player configuration ...
		for (input_config_player = 0; input_config_player < 5;
			input_config_player++) {
			char input_name[8];
			char input_type_name[10];

			// If there's a joypad, dump it
			if (config[input_config_number]
				.individual_config[input_config_player].joydev) {
				snprintf (input_name, 8, "joydev%1d", input_config_player);
				snprintf (input_type_name, 10, "%d",
					config[input_config_number]
						.individual_config[input_config_player].joydev);
				set_config_var_str (section_name, input_name, input_type_name);
			}
		// For each button configuration ...
			for (input_config_button = 0; input_config_button < J_MAX;
				input_config_button++) {
				char temp_joy_str[15];

				// Skip empty entries in joypad mapping
				if (config[input_config_number].individual_config[input_config_player]
					.joy_mapping[input_config_button]
					== default_joy_mapping[input_config_button]) {
					continue;
				}

				if ((0 == config[input_config_number].individual_config[input_config_player].joydev)
					&& (input_config_button >= J_PAD_START)) {
					// If it is a joystick button/axis and disabled, we skip it
					continue;
				}

				snprintf (temp_joy_str, 15, "%s%1d",
				joymap_reverse[input_config_button], input_config_player);

				set_config_var_int(section_name, temp_joy_str,
					config[input_config_number].individual_config[input_config_player].
					joy_mapping[input_config_button]);
			}
		}
	}

	// Sorts the configuration array
	qsort (config_ar, config_ar_index, sizeof (config_var), config_var_cmp);

	// Dump the configuration into a file
	sprintf (config_name, "%s/huexpress.ini", config_basepath);
	dump_config (config_name);

	dispose_config ();
}
