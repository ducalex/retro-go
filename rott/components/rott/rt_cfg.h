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
#ifndef _rt_cfg_public
#define _rt_cfg_public

//****************************************************************************
//
// Public header for RT_CFG.C
//
//****************************************************************************


//****************************************************************************
//
// GLOBALS
//
//****************************************************************************

extern int     FXMode;
extern int     MusicMode;
extern int     MUvolume;
extern int     FXvolume;
extern boolean mouseenabled;
extern boolean joystickenabled;
extern boolean joypadenabled;
extern int     joystickport;
extern int     mouseadjustment;
extern int     threshold;
extern int     NumVoices;
extern int     NumChannels;
extern int     NumBits;
#ifdef DOS
extern int     MidiAddress;
#endif
extern boolean stereoreversed;
extern boolean cybermanenabled;
extern boolean assassinenabled;
extern boolean spaceballenabled;
extern boolean AutoDetailOn;
extern int     DoubleClickSpeed;
extern int     fulllight;
extern boolean BobbinOn;
extern int     Menuflipspeed;
extern int     DetailLevel;
extern int     fandc;
extern int     blanktime;
extern char    CodeName[9];
#ifdef DOS
extern char   *ApogeePath;
#else
extern char   ApogeePath[256];
#endif

extern int     DefaultDifficulty;
extern int     DefaultPlayerCharacter;
extern int     DefaultPlayerColor;
extern byte    passwordstring[20];

typedef struct
{
   char path[41];
   boolean avail;
   char file[14];
} AlternateInformation;

extern AlternateInformation RemoteSounds;
extern AlternateInformation PlayerGraphics;
extern AlternateInformation GameLevels;
extern AlternateInformation BattleLevels;

#define MAXMACROLENGTH 32
#define MAXMACROS      10

typedef struct {
   byte avail;
   char macro[MAXMACROLENGTH+1];
} MacroList;

extern MacroList CommbatMacros[MAXMACROS];

//****************************************************************************
//
// PROTOTYPES
//
//****************************************************************************

void WriteBattleConfig(void);
void ReadScores (void);
void ReadInt (const char * s1, int * val);
void ReadBoolean (const char * s1, boolean * val);
void ReadConfig (void);
void WriteParameter (int file, const char * s1, int val);
void WriteScores (void);
void WriteConfig (void);
void ReadSETUPFiles (void);
void DeleteSoundFile ( void );
void CheckVendor (void);
void ConvertStringToPasswordString ( char * string );
void ConvertPasswordStringToPassword ( void );
void ConvertPasswordStringToString ( char * string );
void ConvertPasswordToPasswordString ( void );

#endif
