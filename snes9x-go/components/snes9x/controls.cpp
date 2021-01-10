/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <map>
#include <algorithm>
#include <string>
#include <assert.h>
#include <ctype.h>

#include "snes9x.h"
#include "memmap.h"
#include "apu/apu.h"
#include "snapshot.h"
#include "controls.h"
#include "display.h"

using namespace	std;

#define NONE					(-2)
#define JOYPAD0					0
#define JOYPAD1					1
#define JOYPAD2					2
#define JOYPAD3					3
#define JOYPAD4					4
#define JOYPAD5					5
#define JOYPAD6					6
#define JOYPAD7					7
#define NUMCTLS					8 // This must be LAST

#define MAP_UNKNOWN				(-1)
#define MAP_NONE				0
#define MAP_BUTTON				1

#define FLAG_IOBIT0				(Memory.FillRAM[0x4213] & 0x40)
#define FLAG_IOBIT1				(Memory.FillRAM[0x4213] & 0x80)
#define FLAG_IOBIT(n)			((n) ? (FLAG_IOBIT1) : (FLAG_IOBIT0))

bool8	pad_read = 0, pad_read_last = 0;
uint8	read_idx[2 /* ports */][2 /* per port */];

static struct
{
	uint16				buttons;
	uint16				turbos;
	uint16				toggleturbo;
	uint16				togglestick;
	uint8				turbo_ct;
}	joypad[8];

static map<uint32, s9xcommand_t>	keymap;
static int							turbo_time;
static int							FLAG_LATCH = FALSE;
static int							curcontrollers[2] = { NONE,    NONE };
static int							newcontrollers[2] = { JOYPAD0, NONE };
static char							buf[256];

// Note: these should be in asciibetical order!
#define THE_COMMANDS \
	S(ClipWindows), \
	S(Debugger), \
	S(DecEmuTurbo), \
	S(DecFrameRate), \
	S(DecFrameTime), \
	S(DecTurboSpeed), \
	S(EmuTurbo), \
	S(ExitEmu), \
	S(IncEmuTurbo), \
	S(IncFrameRate), \
	S(IncFrameTime), \
	S(IncTurboSpeed), \
	S(LoadFreezeFile), \
	S(Pause), \
	S(QuickLoad000), \
	S(QuickSave000), \
	S(Reset), \
	S(SaveFreezeFile), \
	S(SaveSPC), \
	S(SeekToFrame), \
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
	S(SwapJoypads), \
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

static void DisplayStateChange (const char *, bool8);

static int maptype (int t)
{
	switch (t)
	{
		case S9xNoMapping:
			return (MAP_NONE);

		case S9xButtonJoypad:
		case S9xButtonCommand:
		case S9xButtonPort:
			return (MAP_BUTTON);

		default:
			return (MAP_UNKNOWN);
	}
}

static bool strless (const char *a, const char *b)
{
	return (strcmp(a, b) < 0);
}

static int findstr (const char *needle, const char **haystack, int numstr)
{
	const char	**r;

	r = lower_bound(haystack, haystack + numstr, needle, strless);
	if (r >= haystack + numstr || strcmp(needle, *r))
		return (-1);

	return (r - haystack);
}

static const char * maptypename (int t)
{
	switch (t)
	{
		case MAP_NONE:		return ("unmapped");
		case MAP_BUTTON:	return ("button");
		default:			return ("unknown");
	}
}

static void DisplayStateChange (const char *str, bool8 on)
{
	snprintf(buf, sizeof(buf), "%s: %s", str, on ? "on":"off");
	S9xSetInfoString(buf);
}

void S9xControlsReset (void)
{
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			read_idx[i][j]=0;

	FLAG_LATCH = FALSE;

	curcontrollers[0] = newcontrollers[0];
	curcontrollers[1] = newcontrollers[1];
}

void S9xUnmapAllControls (void)
{
	S9xControlsReset();

	keymap.clear();

	for (int i = 0; i < 8; i++)
	{
		joypad[i].buttons  = 0;
		joypad[i].turbos   = 0;
		joypad[i].turbo_ct = 0;
	}

	turbo_time = 1;
}

void S9xSetController (int port, enum controllers controller, int8 id1, int8 id2, int8 id3, int8 id4)
{
	if (port < 0 || port > 1)
		return;

	switch (controller)
	{
		case CTL_NONE:
			break;

		case CTL_JOYPAD:
			if (id1 < 0 || id1 > 7)
				break;

			newcontrollers[port] = JOYPAD0 + id1;
			return;

		default:
			fprintf(stderr, "Unknown controller type %d\n", controller);
			break;
	}

	newcontrollers[port] = NONE;
}

s9xcommand_t S9xGetCommandT (const char *name)
{
	s9xcommand_t	cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.type         = S9xBadMapping;
	cmd.multi_press  = 0;
	cmd.button_norpt = 0;

	if (!strcmp(name, "None"))
		cmd.type = S9xNoMapping;
	else
	if (!strncmp(name, "Joypad", 6))
	{
		if (name[6] < '1' || name[6] > '8' || name[7] != ' ')
			return (cmd);

		cmd.button.joypad.idx = name[6] - '1';
		const char	*s = name + 8;
		int i = 0;

		if ((cmd.button.joypad.toggle = strncmp(s, "Toggle", 6) ? 0 : 1))	s += i = 6;
		if ((cmd.button.joypad.sticky = strncmp(s, "Sticky", 6) ? 0 : 1))	s += i = 6;
		if ((cmd.button.joypad.turbo  = strncmp(s, "Turbo",  5) ? 0 : 1))	s += i = 5;

		if (cmd.button.joypad.toggle && !(cmd.button.joypad.sticky || cmd.button.joypad.turbo))
			return (cmd);

		if (i)
		{
			if (*s != ' ')
				return (cmd);
			s++;
		}

		i = 0;

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

		cmd.button.joypad.buttons = i;

		cmd.type = S9xButtonJoypad;
	}
	else
	{
		int i = findstr(name, command_names, LAST_COMMAND);
		if (i < 0)
			return (cmd);

		cmd.type = S9xButtonCommand;
		cmd.button.command = i;
	}

	return (cmd);
}

s9xcommand_t S9xGetMapping (uint32 id)
{
	if (keymap.count(id) == 0)
	{
		s9xcommand_t	cmd;
		cmd.type = S9xNoMapping;
		return (cmd);
	}
	else
		return (keymap[id]);
}

void S9xUnmapID (uint32 id)
{
	keymap.erase(id);
}

bool S9xMapButton (uint32 id, s9xcommand_t mapping, bool poll)
{
	int	t;

	if (id == InvalidControlID)
	{
		fprintf(stderr, "Cannot map InvalidControlID\n");
		return (false);
	}

	t = maptype(mapping.type);

	if (t == MAP_NONE)
	{
		S9xUnmapID(id);
		return (true);
	}

	if (t != MAP_BUTTON)
		return (false);

	t = maptype(S9xGetMapping(id).type);

	if (t != MAP_NONE && t != MAP_BUTTON)
		fprintf(stderr, "WARNING: Remapping ID 0x%08x from %s to button\n", id, maptypename(t));

	t = -1;

	if (poll)
	{
		//
	}

	S9xUnmapID(id);

	keymap[id] = mapping;

	return (true);
}

void S9xReportButton (uint32 id, bool pressed)
{
	if (keymap.count(id) == 0)
		return;

	if (keymap[id].type == S9xNoMapping)
		return;

	if (maptype(keymap[id].type) != MAP_BUTTON)
	{
		fprintf(stderr, "ERROR: S9xReportButton called on %s ID 0x%08x\n", maptypename(maptype(keymap[id].type)), id);
		return;
	}

	if (keymap[id].type == S9xButtonCommand)	// skips the "already-pressed check" unless it's a command, as a hack to work around the following problem:
		if (keymap[id].button_norpt == pressed)	// FIXME: this makes the controls "stick" after loading a savestate while recording a movie and holding any button
			return;

	keymap[id].button_norpt = pressed;

	S9xApplyCommand(keymap[id], pressed, 0);
}

void S9xApplyCommand (s9xcommand_t cmd, int16 data1, int16 data2)
{
	int	i;

	switch (cmd.type)
	{
		case S9xNoMapping:
			return;

		case S9xButtonJoypad:
			if (cmd.button.joypad.toggle)
			{
				if (!data1)
					return;

				uint16	r = cmd.button.joypad.buttons;

				if (cmd.button.joypad.turbo)	joypad[cmd.button.joypad.idx].toggleturbo ^= r;
				if (cmd.button.joypad.sticky)	joypad[cmd.button.joypad.idx].togglestick ^= r;
			}
			else
			{
				uint16	r, s, t, st;

				r = cmd.button.joypad.buttons;
				st = r & joypad[cmd.button.joypad.idx].togglestick & joypad[cmd.button.joypad.idx].toggleturbo;
				r ^= st;
				t  = r & joypad[cmd.button.joypad.idx].toggleturbo;
				r ^= t;
				s  = r & joypad[cmd.button.joypad.idx].togglestick;
				r ^= s;

				if (cmd.button.joypad.turbo && cmd.button.joypad.sticky)
				{
					uint16	x = r; r = st; st = x;
					x = s; s = t; t = x;
				}
				else
				if (cmd.button.joypad.turbo)
				{
					uint16	x = r; r = t; t = x;
					x = s; s = st; st = x;
				}
				else
				if (cmd.button.joypad.sticky)
				{
					uint16	x = r; r = s; s = x;
					x = t; t = st; st = x;
				}

				if (data1)
				{
					joypad[cmd.button.joypad.idx].buttons |= r;
					joypad[cmd.button.joypad.idx].turbos  |= t;
					joypad[cmd.button.joypad.idx].buttons ^= s;
					joypad[cmd.button.joypad.idx].buttons &= ~(joypad[cmd.button.joypad.idx].turbos & st);
					joypad[cmd.button.joypad.idx].turbos  ^= st;
				}
				else
				{
					joypad[cmd.button.joypad.idx].buttons &= ~r;
					joypad[cmd.button.joypad.idx].buttons &= ~(joypad[cmd.button.joypad.idx].turbos & t);
					joypad[cmd.button.joypad.idx].turbos  &= ~t;
				}
			}

			return;

		case S9xButtonCommand:
			if (((enum command_numbers) cmd.button.command) >= LAST_COMMAND)
			{
				fprintf(stderr, "Unknown command %04x\n", cmd.button.command);
				return;
			}

			if (!data1)
			{
				switch (i = cmd.button.command)
				{
					case EmuTurbo:
						Settings.TurboMode = FALSE;
						break;
				}
			}
			else
			{
				switch ((enum command_numbers) (i = cmd.button.command))
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

					case ClipWindows:
						Settings.DisableGraphicWindows = !Settings.DisableGraphicWindows;
						DisplayStateChange("Graphic clip windows", !Settings.DisableGraphicWindows);
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
							S9xSetInfoString("Auto frame skip");
						else
						{
							sprintf(buf, "Frame skip: %d", Settings.SkipFrames - 1);
							S9xSetInfoString(buf);
						}

						break;

					case DecFrameRate:
						if (Settings.SkipFrames <= 1)
							Settings.SkipFrames = AUTO_FRAMERATE;
						else
						if (Settings.SkipFrames != AUTO_FRAMERATE)
							Settings.SkipFrames--;

						if (Settings.SkipFrames == AUTO_FRAMERATE)
							S9xSetInfoString("Auto frame skip");
						else
						{
							sprintf(buf, "Frame skip: %d", Settings.SkipFrames - 1);
							S9xSetInfoString(buf);
						}

						break;

					case IncEmuTurbo:
						if (Settings.TurboSkipFrames < 20)
							Settings.TurboSkipFrames += 1;
						else
						if (Settings.TurboSkipFrames < 200)
							Settings.TurboSkipFrames += 5;
						sprintf(buf, "Turbo frame skip: %d", Settings.TurboSkipFrames);
						S9xSetInfoString(buf);
						break;

					case DecEmuTurbo:
						if (Settings.TurboSkipFrames > 20)
							Settings.TurboSkipFrames -= 5;
						else
						if (Settings.TurboSkipFrames > 0)
							Settings.TurboSkipFrames -= 1;
						sprintf(buf, "Turbo frame skip: %d", Settings.TurboSkipFrames);
						S9xSetInfoString(buf);
						break;

					case IncFrameTime: // Increase emulated frame time by 1ms
						Settings.FrameTime += 1000;
						sprintf(buf, "Emulated frame time: %dms", Settings.FrameTime / 1000);
						S9xSetInfoString(buf);
						break;

					case DecFrameTime: // Decrease emulated frame time by 1ms
						if (Settings.FrameTime >= 1000)
							Settings.FrameTime -= 1000;
						sprintf(buf, "Emulated frame time: %dms", Settings.FrameTime / 1000);
						S9xSetInfoString(buf);
						break;

					case IncTurboSpeed:
						if (turbo_time >= 120)
							break;
						turbo_time++;
						sprintf(buf, "Turbo speed: %d", turbo_time);
						S9xSetInfoString(buf);
						break;

					case DecTurboSpeed:
						if (turbo_time <= 1)
							break;
						turbo_time--;
						sprintf(buf, "Turbo speed: %d", turbo_time);
						S9xSetInfoString(buf);
						break;

					case QuickLoad000:
					case LoadFreezeFile:
						S9xUnfreezeGame(S9xChooseFilename(TRUE));
						break;

					case QuickSave000:
					case SaveFreezeFile:
						S9xFreezeGame(S9xChooseFilename(FALSE));
						break;

					case Pause:
						Settings.Paused = !Settings.Paused;
						DisplayStateChange("Pause", Settings.Paused);
						break;

					case SaveSPC:
						S9xDumpSPCSnapshot();
						break;

					case SoundChannel0:
					case SoundChannel1:
					case SoundChannel2:
					case SoundChannel3:
					case SoundChannel4:
					case SoundChannel5:
					case SoundChannel6:
					case SoundChannel7:
						S9xToggleSoundChannel(i - SoundChannel0);
						sprintf(buf, "Sound channel %d toggled", i - SoundChannel0);
						S9xSetInfoString(buf);
						break;

					case SoundChannelsOn:
						S9xToggleSoundChannel(8);
						S9xSetInfoString("All sound channels on");
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

					case SwapJoypads:
						if ((curcontrollers[0] != NONE && !(curcontrollers[0] >= JOYPAD0 && curcontrollers[0] <= JOYPAD7)))
						{
							S9xSetInfoString("Cannot swap pads: port 1 is not a joypad");
							break;
						}

						if ((curcontrollers[1] != NONE && !(curcontrollers[1] >= JOYPAD0 && curcontrollers[1] <= JOYPAD7)))
						{
							S9xSetInfoString("Cannot swap pads: port 2 is not a joypad");
							break;
						}

						newcontrollers[1] = curcontrollers[0];
						newcontrollers[0] = curcontrollers[1];

						strcpy(buf, "Swap pads: P1=");
						i = 14;
						if (newcontrollers[0] == NONE)
						{
							strcpy(buf + i, "<none>");
							i += 6;
						}
						else
						{
							sprintf(buf + i, "Joypad%d", newcontrollers[0] - JOYPAD0 + 1);
							i += 7;
						}

						strcpy(buf + i, " P2=");
						i += 4;
						if (newcontrollers[1] == NONE)
							strcpy(buf + i, "<none>");
						else
							sprintf(buf + i, "Joypad%d", newcontrollers[1] - JOYPAD0 + 1);

						S9xSetInfoString(buf);
						break;

					case SeekToFrame:
						break;

					case LAST_COMMAND:
						break;
				}
			}

			return;

		case S9xButtonPort:
			S9xHandlePortCommand(cmd, data1, data2);
			return;

		default:
			fprintf(stderr, "WARNING: Unknown command type %d\n", cmd.type);
			return;
	}
}

void S9xSetJoypadLatch (bool latch)
{
	if (!latch && FLAG_LATCH)
	{
		// 1 written, 'plug in' new controllers now
		curcontrollers[0] = newcontrollers[0];
		curcontrollers[1] = newcontrollers[1];
	}

	if (latch && !FLAG_LATCH)
	{
		memset(read_idx, 0, sizeof(read_idx));
	}

	FLAG_LATCH = latch;
}

// prevent read_idx from overflowing (only latching resets it)
// otherwise $4016/7 reads will start returning input data again
static inline uint8 IncreaseReadIdxPost(uint8 &var)
{
	uint8 oldval = var;
	if (var < 255)
		var++;
	return oldval;
}

uint8 S9xReadJOYSERn (int n)
{
	if (n > 1)
		n -= 0x4016;

	assert(n == 0 || n == 1);

	int bits = (OpenBus & ~3) | ((n == 1) ? 0x1c : 0);
	int i = curcontrollers[n];

	if (FLAG_LATCH)
	{
		switch (i)
		{
			case JOYPAD0:
			case JOYPAD1:
			case JOYPAD2:
			case JOYPAD3:
			case JOYPAD4:
			case JOYPAD5:
			case JOYPAD6:
			case JOYPAD7:
				return (bits | ((joypad[i - JOYPAD0].buttons & 0x8000) ? 1 : 0));

			default:
				return (bits);
		}
	}
	else
	{
		switch (i)
		{
			case JOYPAD0:
			case JOYPAD1:
			case JOYPAD2:
			case JOYPAD3:
			case JOYPAD4:
			case JOYPAD5:
			case JOYPAD6:
			case JOYPAD7:
				if (read_idx[n][0] >= 16)
				{
					IncreaseReadIdxPost(read_idx[n][0]);
					return (bits | 1);
				}
				else
					return (bits | ((joypad[i - JOYPAD0].buttons & (0x8000 >> IncreaseReadIdxPost(read_idx[n][0]))) ? 1 : 0));

			default:
				IncreaseReadIdxPost(read_idx[n][0]);
				return (bits);
		}
	}
}

void S9xDoAutoJoypad (void)
{
	S9xSetJoypadLatch(1);
	S9xSetJoypadLatch(0);

	for (int n = 0; n < 2; n++)
	{
		int i = curcontrollers[n];
		switch (i)
		{
			case JOYPAD0:
			case JOYPAD1:
			case JOYPAD2:
			case JOYPAD3:
			case JOYPAD4:
			case JOYPAD5:
			case JOYPAD6:
			case JOYPAD7:
				read_idx[n][0] = 16;
				WRITE_WORD(Memory.FillRAM + 0x4218 + n * 2, joypad[i - JOYPAD0].buttons);
				WRITE_WORD(Memory.FillRAM + 0x421c + n * 2, 0);
				break;

			default:
				WRITE_WORD(Memory.FillRAM + 0x4218 + n * 2, 0);
				WRITE_WORD(Memory.FillRAM + 0x421c + n * 2, 0);
				break;
		}
	}
}

void S9xControlEOF (void)
{
	PPU.GunVLatch = 1000; // i.e., never latch
	PPU.GunHLatch = 0;

	for (int n = 0; n < 2; n++)
	{
		int i = curcontrollers[n];
		switch (i)
		{
			case JOYPAD0:
			case JOYPAD1:
			case JOYPAD2:
			case JOYPAD3:
			case JOYPAD4:
			case JOYPAD5:
			case JOYPAD6:
			case JOYPAD7:
				if (++joypad[i - JOYPAD0].turbo_ct >= turbo_time)
				{
					joypad[i - JOYPAD0].turbo_ct = 0;
					joypad[i - JOYPAD0].buttons ^= joypad[i - JOYPAD0].turbos;
				}

				break;

			default:
				break;
		}
	}

	pad_read_last = pad_read;
	pad_read      = false;
}

void S9xControlPreSaveState (struct SControlSnapshot *s)
{
	memset(s, 0, sizeof(*s));
	s->ver = 4;

	for (int j = 0; j < 2; j++)
	{
		s->port1_read_idx[j] = read_idx[0][j];
		s->port2_read_idx[j] = read_idx[1][j];
	}

#define COPY(x)	{ memcpy((char *) s->internal + i, &(x), sizeof(x)); i += sizeof(x); }

	int	i = 0;

	for (int j = 0; j < 8; j++)
		COPY(joypad[j].buttons);

	assert(i == sizeof(s->internal) + sizeof(s->internal_macs));

#undef COPY

	s->pad_read      = pad_read;
	s->pad_read_last = pad_read_last;
}

void S9xControlPostLoadState (struct SControlSnapshot *s)
{
	for (int j = 0; j < 2; j++)
	{
		read_idx[0][j] = s->port1_read_idx[j];
		read_idx[1][j] = s->port2_read_idx[j];
	}

	FLAG_LATCH = (Memory.FillRAM[0x4016] & 1) == 1;

	if (s->ver > 1)
	{
	#define COPY(x)	{ memcpy(&(x), (char *) s->internal + i, sizeof(x)); i += sizeof(x); }

		int	i = 0;

		for (int j = 0; j < 8; j++)
			COPY(joypad[j].buttons);

	#undef COPY
	}

	if (s->ver > 2)
	{
		pad_read      = s->pad_read;
		pad_read_last = s->pad_read_last;
	}
}
