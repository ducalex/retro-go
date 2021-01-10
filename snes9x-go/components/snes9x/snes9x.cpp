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
#include "display.h"

#ifdef DEBUGGER
#include "debug.h"
extern FILE	*trace;
#endif

static bool parse_controller_spec (int port, const char *arg)
{
	if (!strcasecmp(arg, "none"))
		S9xSetController(port, CTL_NONE,       0, 0, 0, 0);
	else
	if (!strncasecmp(arg, "pad",   3) && arg[3] >= '1' && arg[3] <= '8' && arg[4] == '\0')
		S9xSetController(port, CTL_JOYPAD, arg[3] - '1', 0, 0, 0);
	else
		return (false);

	return (true);
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

	// Sound

	Settings.SoundSync                  =  true;
	Settings.SixteenBitSound            =  true;
	Settings.Stereo                     =  true;
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
	Settings.AutoDisplayMessages        =  true;
	Settings.InitialInfoStringTimeout   =  120;

	// Settings

	Settings.TurboMode                  =  false;
	Settings.TurboSkipFrames            =  15;
	Settings.AutoSaveDelay              =  0;

	Settings.FrameTimePAL = 20000;
	Settings.FrameTimeNTSC = 16667;

	Settings.SkipFrames = AUTO_FRAMERATE;

	// Controls
	parse_controller_spec(0, "pad1");
	parse_controller_spec(1, "none");

	// Hack
	Settings.DisableGameSpecificHacks       = false;
	Settings.HDMATimingHack                 = 100;
	Settings.MaxSpriteTilesPerLine          = 34;

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
}
