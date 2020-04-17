#ifndef _INCLUDE_CONFIG_H
#define _INCLUDE_CONFIG_H

#include "pce.h"
#include "debug.h"

#include "interf.h"

#include <string.h>

#define VERSION_MAJOR 0
#define VERSION_MINOR 9
#define VERSION_UPDATE 0


#define SETTINGS_FILENAME "huexpress.cfg"

#if defined(SDL)

// #include "osd_machine_sdl.h"

#else // not SDL

#include "osd_machine.h"

#endif

int init_config();

void set_config_file (const char *filename);
void set_config_file_back (void);

void get_config_string(const char *section, const char *keyword, const char *default_value,
	char* result);
int get_config_int(const char *section, const char *keyword, int default_value);

void parse_INIfile();
/* check the configuration file for options
   also make some global initialisations */

int parse_commandline(int argc, char** argv);
/* check the command line for options */

extern uchar joy_mapping[5][16];

extern int32 smode,vmode;

extern char cdsystem_path[PCE_PATH_MAX];

#endif
