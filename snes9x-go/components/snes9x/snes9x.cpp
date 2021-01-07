/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <ctype.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "snes9x.h"
#include "memmap.h"
#include "controls.h"
#include "crosshairs.h"
#include "display.h"
#ifdef NETPLAY_SUPPORT
#include "netplay.h"
#endif

#ifdef DEBUGGER
#include "debug.h"
extern FILE	*trace;
#endif

static bool parse_controller_spec (int, const char *);
static void parse_crosshair_spec (enum crosscontrols, const char *);


static bool parse_controller_spec (int port, const char *arg)
{
	if (!strcasecmp(arg, "none"))
		S9xSetController(port, CTL_NONE,       0, 0, 0, 0);
	else
	if (!strncasecmp(arg, "pad",   3) && arg[3] >= '1' && arg[3] <= '8' && arg[4] == '\0')
		S9xSetController(port, CTL_JOYPAD, arg[3] - '1', 0, 0, 0);
	else
	if (!strncasecmp(arg, "mouse", 5) && arg[5] >= '1' && arg[5] <= '2' && arg[6] == '\0')
		S9xSetController(port, CTL_MOUSE,  arg[5] - '1', 0, 0, 0);
	else
	if (!strcasecmp(arg, "superscope"))
		S9xSetController(port, CTL_SUPERSCOPE, 0, 0, 0, 0);
	else
	if (!strcasecmp(arg, "justifier"))
		S9xSetController(port, CTL_JUSTIFIER,  0, 0, 0, 0);
	else
	if (!strcasecmp(arg, "two-justifiers"))
		S9xSetController(port, CTL_JUSTIFIER,  1, 0, 0, 0);
	else
	if (!strcasecmp(arg, "macsrifle"))
		S9xSetController(port, CTL_MACSRIFLE,  0, 0, 0, 0);
	else
	if (!strncasecmp(arg, "mp5:", 4) && ((arg[4] >= '1' && arg[4] <= '8') || arg[4] == 'n') &&
										((arg[5] >= '1' && arg[5] <= '8') || arg[5] == 'n') &&
										((arg[6] >= '1' && arg[6] <= '8') || arg[6] == 'n') &&
										((arg[7] >= '1' && arg[7] <= '8') || arg[7] == 'n') && arg[8] == '\0')
		S9xSetController(port, CTL_MP5, (arg[4] == 'n') ? -1 : arg[4] - '1',
										(arg[5] == 'n') ? -1 : arg[5] - '1',
										(arg[6] == 'n') ? -1 : arg[6] - '1',
										(arg[7] == 'n') ? -1 : arg[7] - '1');
	else
		return (false);

	return (true);
}

static void parse_crosshair_spec (enum crosscontrols ctl, const char *spec)
{
	int			idx = -1, i;
	const char	*fg = NULL, *bg = NULL, *s = spec;

	if (s[0] == '"')
	{
		s++;
		for (i = 0; s[i] != '\0'; i++)
			if (s[i] == '"' && s[i - 1] != '\\')
				break;

		idx = 31 - ctl;

		std::string	fname(s, i);
		if (!S9xLoadCrosshairFile(idx, fname.c_str()))
			return;

		s += i + 1;
	}
	else
	{
		if (isdigit(*s))
		{
			idx = *s - '0';
			s++;
		}

		if (isdigit(*s))
		{
			idx = idx * 10 + *s - '0';
			s++;
		}

		if (idx > 31)
		{
			fprintf(stderr, "Invalid crosshair spec '%s'.\n", spec);
			return;
		}
	}

	while (*s != '\0' && isspace(*s))
		s++;

	if (*s != '\0')
	{
		fg = s;

		while (isalnum(*s))
			s++;

		if (*s != '/' || !isalnum(s[1]))
		{
			fprintf(stderr, "Invalid crosshair spec '%s.'\n", spec);
			return;
		}

		bg = ++s;

		while (isalnum(*s))
			s++;

		if (*s != '\0')
		{
			fprintf(stderr, "Invalid crosshair spec '%s'.\n", spec);
			return;
		}
	}

	S9xSetControllerCrosshair(ctl, idx, fg, bg);
}

void S9xInitSettings(void)
{
	memset(&Settings, 0, sizeof(Settings));

	// ROM
	Settings.ForceInterleaved2          =  false;
	Settings.ForceInterleaveGD24        =  false;
	Settings.NoPatch                    = !true;
	Settings.IgnorePatchChecksum        =  false;

	Settings.ForceLoROM = false;
	Settings.ForceHiROM = false;

	Settings.ForcePAL   = false;
	Settings.ForceNTSC  = false;

	Settings.ForceHeader = false;
	Settings.ForceNoHeader = false;

	Settings.ForceInterleaved = false;
	Settings.ForceNotInterleaved = false;

	Settings.InitialSnapshotFilename[0] = '\0';

	// Sound

	Settings.SoundSync                  =  true;
	Settings.SixteenBitSound            =  true;
	Settings.Stereo                     =  true;
	Settings.ReverseStereo              =  false;
	Settings.SoundPlaybackRate          =  48000;
	Settings.SoundInputRate             =  31950;
	Settings.Mute                       =  false;
	Settings.DynamicRateControl         =  false;
	Settings.DynamicRateLimit           =  5;
	Settings.InterpolationMethod        =  2;

	// Display

	Settings.Transparency               =  true;
	Settings.DisableGraphicWindows      = !true;
	Settings.DisplayFrameRate           =  false;
	Settings.DisplayWatchedAddresses    =  false;
	Settings.AutoDisplayMessages        =  true;
	Settings.InitialInfoStringTimeout   =  120;
	Settings.BilinearFilter             =  false;

	// Settings

	Settings.TurboMode                  =  false;
	Settings.TurboSkipFrames            =  15;
	Settings.AutoSaveDelay              =  0;

	Settings.FrameTimePAL = 20000;
	Settings.FrameTimeNTSC = 16667;

	Settings.SkipFrames = AUTO_FRAMERATE;

	// Controls

	Settings.MouseMaster                =  true;
	Settings.SuperScopeMaster           =  true;
	Settings.JustifierMaster            =  true;
	Settings.MacsRifleMaster            =  true;
	Settings.MultiPlayer5Master         =  true;
	Settings.UpAndDown                  =  false;


	parse_controller_spec(0, "pad1");
	parse_controller_spec(1, "none");

	parse_crosshair_spec(X_MOUSE1,     "1 White/Black");
	parse_crosshair_spec(X_MOUSE2,     "1 White/Black");
	parse_crosshair_spec(X_SUPERSCOPE, "2 White/Black");
	parse_crosshair_spec(X_JUSTIFIER1, "4 Blue/Black");
	parse_crosshair_spec(X_JUSTIFIER2, "4 MagicPink/Black");

	// Hack
	Settings.SuperFXClockMultiplier         = 100;
    Settings.OverclockMode                  = 0;
	Settings.DisableGameSpecificHacks       = false;
	Settings.BlockInvalidVRAMAccessMaster   = true;
	Settings.HDMATimingHack                 = 100;
	Settings.MaxSpriteTilesPerLine          = 34;

	// Netplay
#ifdef NETPLAY_SUPPORT
	Settings.NetPlay = false;
	Settings.Port = NP_DEFAULT_PORT;
	Settings.ServerName[0] = '\0';
#endif

	// Debug
#ifdef DEBUGGER
	if (conf.GetBool("DEBUG::Debugger", false))
		CPU.Flags |= DEBUG_MODE_FLAG;

	if (conf.GetBool("DEBUG::Trace", false))
	{
		ENSURE_TRACE_OPEN(trace,"trace.log","wb")
		CPU.Flags |= TRACE_FLAG;
	}
	Settings.TraceSMP = FALSE;
#endif

	S9xVerifyControllers();
}
