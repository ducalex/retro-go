/*****************************************************
 * HuExpress, the PC Engine emulator
 *
 * Copyright 2011-2013 Alexander von Gluck IV
 * Released under the terms of the GPLv2 license.
 *
 * Original HuGO! Authors:
 *	Copyright 2001-2005 Zeograd
 *  Copyright 1998 BERO
 *  Copyright 1996 Alex Krasivsky, Marat Fayzullin
 */

#include <sys/stat.h>

#include "PCEngine.h"


PCEngine::PCEngine()
	:
	fReady(0),
	fOptions(&option)
{
	fOptions->want_fullscreen = 0;
	fOptions->want_fullscreen_aspect = 0;
	fOptions->configure_joypads = 0;

	// 2 is a good default multiplier for modern desktops
	fOptions->window_size = 2;

#if defined(ENABLE_NETPLAY)
	fOptions->local_input_mapping[0] = 0;
	fOptions->local_input_mapping[1] = 1;
	fOptions->local_input_mapping[2] = -1;
	fOptions->local_input_mapping[3] = -1;
	fOptions->local_input_mapping[4] = -1;
	strcpy(fOptions.server_hostname, "localhost");
#endif

	/*
	 * Most sound cards won't like this value but we'll accept whatever SDL
	 * gives us.  This frequency will get us really close to the original
	 * pc engine sound behaviour.
	 */
	fOptions->want_snd_freq = 21963;

	// Create the log file
	init_log_file();

	// Init the random seed
	srand((unsigned int) time(NULL));

	// Read configuration in ini file
	parse_INIfile();

	// Initialise the host machine
	if (!osd_init_machine()) {
		MESSAGE_ERROR("Unable to initialize the host machine!\n");
		return;
	}

	// Initialize the input subsystem
	if (osd_init_input() != 0) {
		MESSAGE_ERROR("Unable to initialize input subsystem!\n");
		return;
	}

	InitPaths();

	fReady = 1;
}


PCEngine::~PCEngine()
{
	osd_shutdown_input();

	// Deinitialise the host machine
	osd_shut_machine();
}


void
PCEngine::InitPaths()
{
	// Populate paths for emulation engine

	if (getenv("HOME")) {
		#ifdef __HAIKU__
		snprintf(config_basepath, PATH_MAX, "%s/config/settings/huexpress", getenv("HOME"));
		#else
		snprintf(config_basepath, PATH_MAX, "%s/.huexpress", getenv("HOME"));
		#endif
	} else
		sprintf(config_basepath, "./");

	// Create directory if not existing
	mkdir(config_basepath, 0777);

	// Set the log file
	sprintf(log_filename,"%s/%s", config_basepath, "huexpress.log");

	// Set a temporary path per user (should it be by process ?)
	sprintf(tmp_basepath, "%s/tmp", config_basepath);
	mkdir(tmp_basepath, 0777);

	// Set the saved game directory
	sprintf (sav_basepath, "%s/sav/", config_basepath);
	mkdir(sav_basepath, 0777);

	// Set the video output directory
	sprintf (video_path, "%s/video/", config_basepath);
	mkdir(video_path, 0777);
}


int
PCEngine::LoadFile(char* file)
{
	if (InitPCE(file) != 0) {
		MESSAGE_ERROR("Unable to load file %s!\n", file);
		return 1;
	}
	MESSAGE_INFO("Loaded file %s\n", file);
	return 0;
}


void
PCEngine::Run()
{
	if (!(*osd_gfx_driver_list[video_driver].init) ())
		MESSAGE_ERROR("Can't set graphic resolution\n");

	if (!osd_snd_init_sound())
		MESSAGE_ERROR("Couldn't open any sound hardware on host machine.\n");
	else
		MESSAGE_INFO("Initilized audio, SDL compatible soundcard. %dHz\n",
			host.sound.freq);

#if defined(ENABLE_NETPLAY)
	osd_init_netplay();
#endif

	// Run the emulation
	RunPCE();

#if defined(ENABLE_NETPLAY)
	osd_shutdown_netplay();
#endif

	osd_snd_trash_sound();

	(*osd_gfx_driver_list[video_driver].shut) ();

	// Free the target machine (pce)
	TrashPCE();
}
