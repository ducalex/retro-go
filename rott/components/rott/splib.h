/*
Copyright (C) 1994-1995 Apogee Software, Ltd.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#ifndef SPLIB_H
#define SPLIB_H

/* ======================================================================= *
 * Please Read "SpReadme.doc" for usage                                    *
 * ======================================================================= */

/* ======================================================================= *
 * Compiler & Memory Mode Wrappers.                                        *
 * ----------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER) || defined(__BORLANDC__)
#  if PLATFORM_DOS
#    define REALMODE
#  endif
#endif

#if defined(REALMODE)
#define FAR __far
#else
#define FAR
#endif

//---------------------------------------------------------------------------
// Constants

#define TSR_SERIAL_LENGTH		14
#define TSR_MAX_LENGTH			80

//---------------------------------------------------------------------------

typedef struct {
	char           copyright[TSR_MAX_LENGTH];	// driver copyright
	short          major;							// driver version number
	short				minor;
	short				count;							// # of available devices
} SpwDrvOpenPacket;



typedef struct {
	char           copyright[TSR_MAX_LENGTH];	// device copyright
	char           serial[TSR_SERIAL_LENGTH];	// device serial number
} SpwDevOpenPacket;



typedef struct {
	long				timestamp;						// time of event
	unsigned short	period;							// period since last MOVEMENT
	unsigned short button;							// button pressed mask
	short          tx;								// Translation X
	short          ty;								//					Y
	short          tz;								//					Z
	short          rx;								// Rotation X
	short          ry;								//				Y
	short          rz;								//				Z
} SpwForcePacket;



typedef struct {
	long           timestamp;						// time of event
	unsigned short period;							// period since last BUTTON
	unsigned short button;							// button pressed mask
} SpwButtonPacket;



typedef struct {
	unsigned long data;								// MUST be TSRCMD_DATA
} SpwCommandPacket;

#define TSRCMD_DATA	0xFF0000FF

//---------------------------------------------------------------------------

typedef union {
	char              padding[128];		/* Extra room for future expansion */

	SpwCommandPacket	command;

	SpwDrvOpenPacket	drvOpen;
	SpwDevOpenPacket	devOpen;

	SpwForcePacket		force;
	SpwButtonPacket	button;
} SpwPacket;



// TSR Interrupt Functions
#define TSR_DRIVER_CLOSE		 		0x0000
#define TSR_DRIVER_OPEN					0x8001
#define TSR_DEVICE_CLOSE				0x0002
#define TSR_DEVICE_OPEN					0x8003

#define TSR_DEVICE_DISABLE				0x0010
#define TSR_DEVICE_ENABLE				0x0011

#define TSR_DEVICE_GETFORCE			0x8020
#define TSR_DEVICE_GETBUTTONS			0x8021

/* ======================================================================= *
 * Function Prototypes                                                     *
 * ======================================================================= */

short SpwOpenDriver(SpwPacket FAR *packet);
short SpwCloseDriver(void);

short SpwOpenDevice(short device, SpwPacket FAR *packet);
short SpwCloseDevice(short device);

short SpwEnableDevice(short device);
short SpwDisableDevice(short device);

short SpwGetForce(short device, SpwPacket FAR *packet);
short SpwGetButton(short device, SpwPacket FAR *packet);

/* ======================================================================= *
 * Convience functions                                                     *
 * ======================================================================= */

/* ----------------------------------------------------------------------- *
 * The SpwSimple... functions are just convienence wrappers for the above  *
 * functions.                                                              *
 * ----------------------------------------------------------------------- */

#ifndef SPWSTRUCTS
#define SPWSTRUCTS

enum SpwDeviceType {
	SPW_AVENGER=1,
};



enum SpwEventType {
	SPW_NO_EVENT=0,
   SPW_BUTTON_HELD=1,
   SPW_BUTTON_DOWN=2,
   SPW_BUTTON_UP=4,
   SPW_MOTION=8
};



/* ----------------------------------------------------------------------- *
 * Data struct for handling library calls                                  *
 * ----------------------------------------------------------------------- */

typedef struct {
	short new;
   short cur;
   short old;
} SpwButtonRec;



typedef struct {
	short 		 tx;			/* Current Translation vector */
   short 		 ty;
   short 		 tz;
   short 		 rx;			/* Current Rotation vector    */
   short 		 ry;
   short        rz;
   SpwButtonRec buttons;   /* Current Button Record      */
   short			 newData;   /* An SpEventType mask of newData, 0 if none */
} SpwRawData;

#endif



short SpwSimpleGet(short devNum, SpwRawData FAR *splayer);
short SpwSimpleOpen(short devNum);
short SpwSimpleClose(short devNum);

/* ======================================================================= *
 * Compiler & Memory Mode Wrappers.                                        *
 * ======================================================================= *
 */

#ifdef __cplusplus
};
#endif

/* ======================================================================= */
#endif

