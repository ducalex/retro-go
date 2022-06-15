/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "snes9x.h"
#include "memmap.h"
#include "controls.h"

struct SSettings Settings;
char String[513];


void S9xInitSettings(void)
{
	memset(&Settings, 0, sizeof(Settings));

	// Sound
	Settings.SoundSync                  =  true;
	Settings.Stereo                     =  true;
	Settings.SoundPlaybackRate          =  32000;
	Settings.SoundInputRate             =  31950;
	Settings.Mute                       =  false;
	Settings.DynamicRateLimit           =  5;

	// Display
	Settings.Transparency               =  true;
	Settings.InitialInfoStringTimeout   =  120;

	// Settings
	Settings.TurboMode                  =  false;
	Settings.SkipFrames                 = AUTO_FRAMERATE;

	// Hack
	Settings.DisableGameSpecificHacks   = false;
}
