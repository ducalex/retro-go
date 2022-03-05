/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _CONTROLS_H_
#define _CONTROLS_H_

enum
{
	S9xNoMapping,
	S9xButtonJoypad0,
	S9xButtonJoypad1,
	S9xButtonCommand,
	S9xBadMapping,
};

#define MaxControlID	63

typedef struct
{
	uint8	type;
	uint8	button_norpt;
	union
	{
		uint16	buttons;			// Which buttons to actuate. Use SNES_*_MASK constants from snes9x.h
		uint16	command;
	};
}	s9xcommand_t;

void S9xUnmapAllControls (void);
s9xcommand_t S9xGetCommandT (const char *name);
s9xcommand_t S9xGetMapping (uint32 id);
bool S9xMapButton (uint32 id, s9xcommand_t mapping);
bool S9xMapButtonT (uint32 id, const char *command);
void S9xReportButton (uint32 id, bool pressed);
void S9xUnmapButton (uint32 id);
void S9xApplyCommand (s9xcommand_t cmd, int data);

//////////
// These functions are called from snes9x into this subsystem. No need to use them from a port.

// Use when resetting snes9x.

void S9xControlsReset (void);

// Use when writing to $4016.

void S9xSetJoypadLatch (bool latch);

// Use when reading $4016/7 (JOYSER0 and JOYSER1).

uint8 S9xReadJOYSERn (int n);

#endif
