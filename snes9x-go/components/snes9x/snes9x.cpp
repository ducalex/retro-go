/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "snes9x.h"
#include "memory.h"
#include "controls.h"
#include "display.h"

struct SSettings Settings;
char String[513];


void S9xInitSettings(void)
{
	memset(&Settings, 0, sizeof(Settings));

	// ROM
	Settings.ForcePAL                   = false;
	Settings.ForceNTSC                  = false;

	// Sound
	Settings.SoundSync                  =  true;
	Settings.Stereo                     =  true;
	Settings.SoundPlaybackRate          =  48000;
	Settings.SoundInputRate             =  31950;
	Settings.Mute                       =  false;
	Settings.DynamicRateControl         =  false;
	Settings.DynamicRateLimit           =  5;
	Settings.InterpolationMethod        =  2;

	// Display
	Settings.Transparency               =  true;
	Settings.DisableGraphicWindows      =  false;
	Settings.InitialInfoStringTimeout   =  120;

	// Settings
	Settings.TurboMode                  =  false;
	Settings.TurboSkipFrames            =  15;
	Settings.AutoSaveDelay              =  0;
	Settings.SkipFrames                 = AUTO_FRAMERATE;

	// Hack
	Settings.DisableGameSpecificHacks   = false;
}
