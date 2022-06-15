/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "snes9x.h"
#include "memmap.h"
#include "apu/apu.h"
#include "snapshot.h"
#include "controls.h"

static s9xcommand_t keymap[MaxControlID + 1];
static int			FLAG_LATCH = FALSE;
static struct {
	uint32	read_idx;
	uint32	buttons;
} joypads[2];

// Note: these should be in asciibetical order!
#define THE_COMMANDS \
	S(Debugger), \
	S(DecFrameRate), \
	S(EmuTurbo), \
	S(ExitEmu), \
	S(IncFrameRate), \
	S(LoadFreezeFile), \
	S(Pause), \
	S(Reset), \
	S(SaveFreezeFile), \
	S(SoftReset), \
	S(SoundChannel0), \
	S(SoundChannel1), \
	S(SoundChannel2), \
	S(SoundChannel3), \
	S(SoundChannel4), \
	S(SoundChannel5), \
	S(SoundChannel6), \
	S(SoundChannel7), \
	S(SoundChannelsOn), \
	S(ToggleBG0), \
	S(ToggleBG1), \
	S(ToggleBG2), \
	S(ToggleBG3), \
	S(ToggleEmuTurbo), \
	S(ToggleSprites), \
	S(ToggleTransparency) \

#define S(x)	x

enum command_numbers
{
	THE_COMMANDS,
	LAST_COMMAND
};

#undef S
#define S(x)	#x

static const char	*command_names[LAST_COMMAND + 1] =
{
	THE_COMMANDS,
	NULL
};

#undef S
#undef THE_COMMANDS

static void DisplayStateChange (const char *str, bool8 on)
{
	snprintf(String, sizeof(String), "%s: %s", str, on ? "on":"off");
	S9xMessage(S9X_INFO, 0, String);
}

void S9xControlsReset (void)
{
	memset(&joypads, 0, sizeof(joypads));
	FLAG_LATCH = FALSE;
}

void S9xUnmapAllControls (void)
{
	memset(&keymap, 0, sizeof(keymap));
	S9xControlsReset();
}

s9xcommand_t S9xGetCommandT (const char *name)
{
	s9xcommand_t	cmd = {
		.type = S9xBadMapping,
		.button_norpt = 0,
		.command = 0,
	};

	if (!name)
		cmd.type = S9xBadMapping;
	else
	if (!strcmp(name, "None"))
		cmd.type = S9xNoMapping;
	else
	if (!strncmp(name, "Joypad", 6))
	{
		if (((int)name[6] - '1') <= 0)
			cmd.type = S9xButtonJoypad0;
		else
			cmd.type = S9xButtonJoypad1;

		const char	*s = name + 7;
		int i = 0;

		while (s[0] == ' ')
			s++;

		if (!strncmp(s, "Up",     2))	{ i |= SNES_UP_MASK;     s += 2; if (*s == '+') s++; }
		if (!strncmp(s, "Down",   4))	{ i |= SNES_DOWN_MASK;   s += 4; if (*s == '+') s++; }
		if (!strncmp(s, "Left",   4))	{ i |= SNES_LEFT_MASK;   s += 4; if (*s == '+') s++; }
		if (!strncmp(s, "Right",  5))	{ i |= SNES_RIGHT_MASK;  s += 5; if (*s == '+') s++; }

		if (*s == 'A')	{ i |= SNES_A_MASK;  s++; if (*s == '+') s++; }
		if (*s == 'B')	{ i |= SNES_B_MASK;  s++; if (*s == '+') s++; }
		if (*s == 'X')	{ i |= SNES_X_MASK;  s++; if (*s == '+') s++; }
		if (*s == 'Y')	{ i |= SNES_Y_MASK;  s++; if (*s == '+') s++; }
		if (*s == 'L')	{ i |= SNES_TL_MASK; s++; if (*s == '+') s++; }
		if (*s == 'R')	{ i |= SNES_TR_MASK; s++; if (*s == '+') s++; }

		if (!strncmp(s, "Start",  5))	{ i |= SNES_START_MASK;  s += 5; if (*s == '+') s++; }
		if (!strncmp(s, "Select", 6))	{ i |= SNES_SELECT_MASK; s += 6; }

		if (i == 0 || *s != 0 || *(s - 1) == '+')
			return (cmd);

		cmd.buttons = i;
	}
	else
	{
		for (int i = 0; i < LAST_COMMAND; i++)
		{
			if (strcasecmp(command_names[i], name) == 0)
			{
				cmd.type = S9xButtonCommand;
				cmd.command = i;
				break;
			}
		}
	}

	return (cmd);
}

s9xcommand_t S9xGetMapping (uint32 id)
{
	if (id > MaxControlID)
	{
		s9xcommand_t dummy = {0};
		return dummy;
	}

	return (keymap[id]);
}

void S9xUnmapButton (uint32 id)
{
	if (id > MaxControlID)
	{
		printf("WARNING: Invalid Control ID %d, should be between 0 and %d\n", id, MaxControlID);
		return;
	}

	memset(&keymap[id], 0, sizeof(s9xcommand_t));
}

bool S9xMapButtonT (uint32 id, const char *command)
{
	s9xcommand_t cmd = S9xGetCommandT(command);

	if (cmd.type == S9xBadMapping)
	{
		printf("WARNING: Invalid command '%s'\n", command);
		return (false);
	}

	return S9xMapButton(id, cmd);
}

bool S9xMapButton (uint32 id, s9xcommand_t mapping)
{
	if (id > MaxControlID)
	{
		printf("WARNING: Invalid Control ID %d, should be between 0 and %d\n", id, MaxControlID);
		return (false);
	}

	if (mapping.type == S9xNoMapping)
	{
		S9xUnmapButton(id);
		return (true);
	}

	if (keymap[id].type != S9xNoMapping)
		fprintf(stderr, "WARNING: Remapping ID %d\n", id);

	keymap[id] = mapping;

	return (true);
}

void S9xReportButton (uint32 id, bool pressed)
{
	if (id > MaxControlID)
		return;

	// skips the "already-pressed check" unless it's a command, as a hack to work around the following problem:
	// FIXME: this makes the controls "stick" after loading a savestate while recording a movie and holding any button
	if (keymap[id].type == S9xButtonCommand && keymap[id].button_norpt)
		return;

	keymap[id].button_norpt = pressed;

	S9xApplyCommand(keymap[id], pressed);
}

void S9xApplyCommand (s9xcommand_t cmd, int data1)
{
	switch (cmd.type)
	{
		case S9xNoMapping:
			return;

		case S9xButtonJoypad0:
			if (data1)
				joypads[0].buttons |= cmd.buttons;
			else
				joypads[0].buttons &= ~cmd.buttons;
			return;

		case S9xButtonJoypad1:
			if (data1)
				joypads[1].buttons |= cmd.buttons;
			else
				joypads[1].buttons &= ~cmd.buttons;

			return;

		case S9xButtonCommand:
			if (!data1)
			{
				switch (cmd.command)
				{
					case EmuTurbo:
						Settings.TurboMode = FALSE;
						break;
					default:
						fprintf(stderr, "Unknown command %04x\n", cmd.command);
				}
			}
			else
			{
				switch (cmd.command)
				{
					case ExitEmu:
						S9xExit();
						break;

					case Reset:
						S9xReset();
						break;

					case SoftReset:
						S9xSoftReset();
						break;

					case EmuTurbo:
						Settings.TurboMode = TRUE;
						break;

					case ToggleEmuTurbo:
						Settings.TurboMode = !Settings.TurboMode;
						DisplayStateChange("Turbo mode", Settings.TurboMode);
						break;

					case Debugger:
					#ifdef DEBUGGER
						CPU.Flags |= DEBUG_MODE_FLAG;
					#endif
						break;

					case IncFrameRate:
						if (Settings.SkipFrames == AUTO_FRAMERATE)
							Settings.SkipFrames = 1;
						else
						if (Settings.SkipFrames < 10)
							Settings.SkipFrames++;

						if (Settings.SkipFrames == AUTO_FRAMERATE)
							S9xMessage(S9X_INFO, 0, "Auto frame skip");
						else
						{
							sprintf(String, "Frame skip: %d", Settings.SkipFrames - 1);
							S9xMessage(S9X_INFO, 0, String);
						}

						break;

					case DecFrameRate:
						if (Settings.SkipFrames <= 1)
							Settings.SkipFrames = AUTO_FRAMERATE;
						else
						if (Settings.SkipFrames != AUTO_FRAMERATE)
							Settings.SkipFrames--;

						if (Settings.SkipFrames == AUTO_FRAMERATE)
							S9xMessage(S9X_INFO, 0, "Auto frame skip");
						else
						{
							sprintf(String, "Frame skip: %d", Settings.SkipFrames - 1);
							S9xMessage(S9X_INFO, 0, String);
						}

						break;

					case LoadFreezeFile:
						// S9xUnfreezeGame(S9xChooseFilename(TRUE));
						break;

					case SaveFreezeFile:
						// S9xFreezeGame(S9xChooseFilename(FALSE));
						break;

					case Pause:
						Settings.Paused = !Settings.Paused;
						DisplayStateChange("Pause", Settings.Paused);
						break;

					case SoundChannel0:
					case SoundChannel1:
					case SoundChannel2:
					case SoundChannel3:
					case SoundChannel4:
					case SoundChannel5:
					case SoundChannel6:
					case SoundChannel7:
						S9xToggleSoundChannel((int)cmd.command - SoundChannel0);
						sprintf(String, "Sound channel %d toggled", (int)cmd.command - SoundChannel0);
						S9xMessage(S9X_INFO, 0, String);
						break;

					case SoundChannelsOn:
						S9xToggleSoundChannel(8);
						S9xMessage(S9X_INFO, 0, "All sound channels on");
						break;

					case ToggleBG0:
						Settings.BG_Forced ^= 1;
						DisplayStateChange("BG#0", !(Settings.BG_Forced & 1));
						break;

					case ToggleBG1:
						Settings.BG_Forced ^= 2;
						DisplayStateChange("BG#1", !(Settings.BG_Forced & 2));
						break;

					case ToggleBG2:
						Settings.BG_Forced ^= 4;
						DisplayStateChange("BG#2", !(Settings.BG_Forced & 4));
						break;

					case ToggleBG3:
						Settings.BG_Forced ^= 8;
						DisplayStateChange("BG#3", !(Settings.BG_Forced & 8));
						break;

					case ToggleSprites:
						Settings.BG_Forced ^= 16;
						DisplayStateChange("Sprites", !(Settings.BG_Forced & 16));
						break;

					case ToggleTransparency:
						Settings.Transparency = !Settings.Transparency;
						DisplayStateChange("Transparency effects", Settings.Transparency);
						break;

					case LAST_COMMAND:
						break;

					default:
						fprintf(stderr, "Unknown command %04x\n", cmd.command);
				}
			}

			return;

		default:
			fprintf(stderr, "WARNING: Unknown command type %d\n", cmd.type);
			return;
	}
}

void S9xSetJoypadLatch (bool latch)
{
	if (latch && !FLAG_LATCH)
	{
		joypads[0].read_idx = 0;
		joypads[1].read_idx = 0;
	}

	FLAG_LATCH = latch;
}

uint8 S9xReadJOYSERn (int n)
{
	n &= 1;

	int bits = (OpenBus & ~3) | ((n == 1) ? 0x1c : 0);

	if (FLAG_LATCH)
	{
		return (bits | ((joypads[n].buttons >> 15) & 1));
	}
	else
	{
		if (joypads[n].read_idx < 16)
		{
			return (bits | ((joypads[n].buttons >> (15 - joypads[n].read_idx++)) & 1));
		}
		else
		{
			return (bits | 1);
		}
	}
}

void S9xDoAutoJoypad (void)
{
	S9xSetJoypadLatch(1);
	S9xSetJoypadLatch(0);

	for (int n = 0; n < 2; n++)
	{
		joypads[n].read_idx = 16;
		WRITE_WORD(Memory.CPU_IO + 0x218 + n * 2, joypads[n].buttons);
		WRITE_WORD(Memory.CPU_IO + 0x21c + n * 2, 0);
	}
}
