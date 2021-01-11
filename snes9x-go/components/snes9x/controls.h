/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _CONTROLS_H_
#define _CONTROLS_H_

#define S9xNoMapping			0
#define S9xButtonJoypad			1
#define S9xButtonCommand		2

// These are automatically kicked out to the S9xHandlePortCommand function.
// If your port wants to define port-specific commands or whatever, use these values for the s9xcommand_t type field.

#define S9xButtonPort			251

#define S9xBadMapping			255
#define InvalidControlID		((uint32) -1)

typedef struct
{
	uint8	type;
	uint8	multi_press:2;
	uint8	button_norpt:1;

	union
	{
		union
		{
			struct
			{
				uint8	idx:3;				// Pad number 0-7
				uint8	toggle:1;			// If set, toggle turbo/sticky for the button
				uint8	turbo:1;			// If set, be a 'turbo' button
				uint8	sticky:1;			// If set, toggle button state (on/turbo or off) when pressed and do nothing on release
				uint16	buttons;			// Which buttons to actuate. Use SNES_*_MASK constants from snes9x.h
			}	joypad;

			int32	multi_idx;
			uint16	command;
		}	button;

		uint8	port[4];
	};
}	s9xcommand_t;

// Starting out...

void S9xUnmapAllControls (void);

// Setting which controllers are plugged in.

enum controllers
{
	CTL_NONE,		// all ids ignored
	CTL_JOYPAD,		// use id1 to specify 0-7
};

void S9xSetController (int port, enum controllers controller, int8 id1, int8 id2, int8 id3, int8 id4); // port=0-1

// Functions for translation s9xcommand_t's into strings, and vice versa.
// free() the returned string after you're done with it.

s9xcommand_t S9xGetCommandT (const char *name);

// Generic mapping functions

s9xcommand_t S9xGetMapping (uint32 id);
void S9xUnmapID (uint32 id);

// Button mapping functions.
// If a button is mapped with poll=TRUE, then S9xPollButton will be called whenever snes9x feels a need for that mapping.
// Otherwise, snes9x will assume you will call S9xReportButton() whenever the button state changes.
// S9xMapButton() will fail and return FALSE if mapping.type isn't an S9xButton* type.

bool S9xMapButton (uint32 id, s9xcommand_t mapping, bool poll);
void S9xReportButton (uint32 id, bool pressed);

// Do whatever the s9xcommand_t says to do.
// If cmd.type is a button type, data1 should be TRUE (non-0) or FALSE (0) to indicate whether the 'button' is pressed or released.
// If cmd.type is an axis, data1 holds the deflection value.
// If cmd.type is a pointer, data1 and data2 are the positions of the pointer.

void S9xApplyCommand (s9xcommand_t cmd, int16 data1, int16 data2);

//////////
// These functions are called by snes9x into your port, so each port should implement them.

// If something was mapped with poll=TRUE, these functions will be called when snes9x needs the button/axis/pointer state.
// Fill in the reference options as appropriate.

bool S9xPollButton (uint32 id, bool *pressed);

// These are called when snes9x tries to apply a command with a S9x*Port type.
// data1 and data2 are filled in like S9xApplyCommand.

void S9xHandlePortCommand (s9xcommand_t cmd, int16 data1, int16 data2);

// Called before already-read SNES joypad data is being used by the game if your port defines SNES_JOY_READ_CALLBACKS.

#ifdef SNES_JOY_READ_CALLBACKS
void S9xOnSNESPadRead (void);
#endif

// These are for your use.

s9xcommand_t S9xGetPortCommandT (const char *name);
char * S9xGetPortCommandName (s9xcommand_t command);
void S9xSetupDefaultKeymap (void);
bool8 S9xMapInput (const char *name, s9xcommand_t *cmd);

//////////
// These functions are called from snes9x into this subsystem. No need to use them from a port.

// Use when resetting snes9x.

void S9xControlsReset (void);

// Use when writing to $4016.

void S9xSetJoypadLatch (bool latch);

// Use when reading $4016/7 (JOYSER0 and JOYSER1).

uint8 S9xReadJOYSERn (int n);

// End-Of-Frame processing. Sets gun latch variables and tries to draw crosshairs

void S9xControlEOF (void);

// Functions and a structure for snapshot.

struct SControlSnapshot
{
	uint8	ver;
	uint8	port1_read_idx[2];
	uint8	dummy1[4];					// for future expansion
	uint8	port2_read_idx[2];
	uint8	dummy2[4];
	bool8	pad_read, pad_read_last;
	uint8	internal[60];				// yes, we need to save this!
	uint8   internal_macs[5];
};

void S9xControlPreSaveState (struct SControlSnapshot *s);
void S9xControlPostLoadState (struct SControlSnapshot *s);

#endif
