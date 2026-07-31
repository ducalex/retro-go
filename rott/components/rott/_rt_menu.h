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
#ifndef _rt_menu_private
#define _rt_menu_private

//******************************************************************************
//
// Private Header for RT_MENU.C
//
//******************************************************************************

#include "rt_in.h"

//
// Specific Menu defines
//

#define BORDCOLOR       0x29
#define BORD2COLOR      0x23
#define DEACTIVE        0x2b
#define BKGDCOLOR       0x2d
#define STRIPE          0x2c


#define SENSITIVE       60
#define CENTER          SENSITIVE*2


#define MENU_X          32
#define MENU_Y          16
#define MENU_W          235
#define MENU_H          14*9+4


#define LSM_X           180      //32
#define LSM_Y           16
#define LSM_W           80       //176
#define LSM_H           10*14+8

#define LSA_X           96
#define LSA_Y           80
#define LSA_W           130
#define LSA_H           42


#define SM_X            32
#define SM_Y            52
#define SM_W            252
#define SM_H            35

#define SM_Y1           20
#define SM_H1           4*14-9
#define SM_Y2           SM_Y1+5*14
#define SM_H2           4*14-9
#define SM_Y3           SM_Y2+5*14
#define SM_H3           3*14-9


#define CTL_X           19
#define CTL_Y           32
#define CTL_W           284
#define CTL_H           105


#define CST_X           20
#define CST_Y           43
#define CST_START       60
#define CST_SPC         60


#define TUF_X           0
#define TUF_Y           32


#define HLP_X           34
#define HLP_Y           63
#define HLP_W           264
#define HLP_H           51


#define MU_X            32
#define MU_Y            22
#define MU_W            252
#define MU_H            118


#define FX_X            32
#define FX_Y            16
#define FX_W            252
#define FX_H            130


#define MP_X            55//32
#define MP_Y            60
#define MP_W            252
#define MP_H            35


#define X1_X            55
#define X1_Y            25
#define X1_W            252

#define X2_X            55
#define X2_Y            65
#define X2_W            252
#define X2_H            35

#define X3_X            55
#define X3_Y            65
#define X3_W            252
#define X3_H            35

#define LEVSEL_X            32
#define LEVSEL_Y            22
#define LEVSEL_W            252
#define LEVSEL_H            118
#define MAXCUSTOM       12

#define STARTITEM       newgame

//
// General defines
//
#define CP_SemiActive      4
#define CP_Highlight       5

#define OUTOFRANGE         100
#define ESCPRESSED         -1
#define PAGEUP             -2
#define PAGEDOWN           -3
#define NOTAVAILABLECOLOR  7
#define NEXTPAGECOLOR      16
#define NORMALCOLOR        21
#define DIMMEDCOLOR        24
#define ACTIVECOLOR        241
#define HIGHLIGHTCOLOR     1

#define MouseInt 0x33
#define GAMESVD      "There's already a game\n" \
                     "saved at this position.\n"\
                     "Overwrite?"
#define COLOR        235

#define CUSTOMX      76

#define NUMSAVEGAMES 14

#define PWORDX 58
#define PWORDY 46
//#define PBOXX  68
#define PBOXX  ( ( 288 - PBOXW ) / 2 )
#define PBOXY  71
#define PBOXW  115
#define PSTRW  110
#define PBOXH  12


#define QUICKSAVEBACKUP ("rottgamf.rot")
//******************************************************************************
//
// ENUMS
//
//******************************************************************************

typedef struct
{
   int allowed[4];
} CustomCtrls;

// FOR INPUT TYPES
enum {MOUSE, JOYSTICK, KEYBOARDBTNS, KEYBOARDMOVE, SPECIAL1, SPECIAL2};


//******************************************************************************
//
// GLOBALS
//
//******************************************************************************

static byte *ScanNames[] =    // Scan code names with single chars
{
   "?","?","1","2","3","4","5","6","7","8","9","0","-","+","?","?",
   "Q","W","E","R","T","Y","U","I","O","P","[","]","|","?","A","S",
   "D","F","G","H","J","K","L",";","\"","?","?","?","Z","X","C","V",
   "B","N","M",",",".","/","?","?","?","?","?","?","?","?","?","?",
   "?","?","?","?","?","?","?","?","\xf","?","-","\x15","5","\x11","+","?",
   "\x13","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?",
   "?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?",
   "?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?"
};


static byte ExtScanCodes[] = // Scan codes with >1 char names
{
   1,    0xe,  0xf,  0x1d, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e,
   0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x57, 0x58, 0x46, 0x1c, 0x36,
   0x37, 0x38, 0x47, 0x49, 0x4f, 0x51, 0x52, 0x53, 0x45, 0x48,
   0x50, 0x4b, 0x4d, 0x00
};

static byte *ExtScanNames[] = // Names corresponding to ExtScanCodes
{
   "Esc","BkSp","Tab","Ctrl","Space","CapLk","F1","F2","F3","F4",
   "F5","F6","F7","F8","F9","F10","F11","F12","SclLk","Enter","Shift",
   "PrtSc","Alt","Home","PgUp","End","PgDn","Ins","Del","NumLk","Up",
   "Down","Left","Right",""
};


//******************************************************************************
//
// PROTOTYPES
//
//******************************************************************************

void MouseSensitivity (void);
void DoThreshold (void);
byte * IN_GetScanName (ScanCode scan);
void DisplayInfo (int which);

void PrintLSEntry (int w);

void DrawHalfStep (int x, int y);

void DrawCtlScreen (void);
void DrawCtlButtons (void);
void DrawSoundMenu (void);
void DrawCustomScreen (void);
void DrawLoadSaveScreen (int loadsave);
void DrawLoadSaveScreenAlt (int loadsave);
void DrawMusicMenu (void);
void DrawNewGame (void);
void DrawNewGameDiff (int w);
void DrawNewPlayerDiff (int w);
void DrawFXMenu (void);
void DrawFXButtons (void);
void DrawCustomJoystick (void);
void DrawCustomMouse (void);
void DrawCustomSpecial (void);
void DrawCustomMenu (void);
void DrawPlayerMenu (void);

void DoMainMenu (void);

boolean CP_DisplayMsg (char *s, int number);
void CP_EndGame (void);
int CP_SaveGame (void);
void CP_Control (void);
void CP_Sound (void);
void CP_ChangeView (void);
void CP_DoubleClickSpeed( void );
void CP_ErrorMsg( char *title, char *error, int font );
void CP_Quit (int which);
void CP_NewGame (void);
void CP_Music (void);
void CP_FX (void);
void CP_MCports (void);
void CP_FXMenu (void);
void CP_Custom (void);
void CP_Keyboard (void);
void CP_Mouse (void);
void CP_Joystick (void);
void CP_Special (void);
void CP_OrderInfo( void );

void DefineMouseBtns1 (void);
void DefineMouseBtns2 (void);
void DefineKeyBtns1 (void);
void DefineKeyBtns2 (void);
void DefineKeyMove1 (void);
void DefineKeyMove2 (void);
void DefineJoyBtns1 (void);
void DefineJoyBtns2 (void);
void DefineSpecialBtns1 (void);
void DefineSpecialBtns2 (void);

void DrawSTMenuBuf (int x, int y, int w, int h, boolean up);

void MusicVolume (void);
void FXVolume (void);

void DefineKey( void );
void DefineJoyBtn( void );
void DefineMouseBtn( void );

void DrawControlMenu (void);
void CP_ControlMenu (void);
void DrawOptionsMenu (void);
void DrawExtOptionsMenu (void);
void CP_ExtOptionsMenu (void);
void CP_OptionsMenu (void);
void DrawOptionsButtons (void);
void DrawExtOptionsButtons (void);
void MenuFlipSpeed (void);
void DrawDetailMenu (void);
void CP_DetailMenu (void);
void DrawBattleMenu (void);
void CP_BattleMenu (void);
void MCERROR (void);

void DrawKeyboardMenu (void);
void CP_KeyboardMenu (void);
boolean SliderMenu( int *number, int upperbound, int lowerbound, int erasex,
   int erasey, int erasew, int numadjust, char *blockname,
   void (*routine) (int w), char *title, char *left, char *right );

void DrawF1Help (void);
void CP_F1Help (void);
void CP_ScreenSize( void );

void CP_ViolenceMenu (void);
void DrawViolenceLevelMenu (void);
void DrawViolenceLevelPWord (void);

void DrawViolenceLevel (void);
void CP_ViolenceLevel (void);

void DrawPWMenu (void);
void CP_PWMenu (void);

void DrawBattleModes (void);
void CP_BattleModes (void);

void DrawBattleOptions (void);
void CP_BattleOptions (void);

void DrawGravityMenu (void);
void CP_GravityOptions (void);
void DrawSpeedMenu (void);
void CP_SpeedOptions (void);
void DrawAmmoPerWeaponMenu (void);
void CP_AmmoPerWeaponOptions (void);
void DrawHitPointsMenu (void);
void CP_HitPointsOptions (void);
void DrawSpawnControlMenu (void);
void DrawSpawnControlButtons (void);
void CP_SpawnControlOptions (void);
void DrawLightLevelMenu (void);
void CP_LightLevelOptions (void);
void DrawPointGoalMenu (void);
void CP_PointGoalOptions (void);
void DrawDangerMenu (void);
void CP_DangerOptions (void);
void DrawTimeLimitMenu (void);
void CP_TimeLimitOptions (void);
void PrintBattleOption( boolean inmenu, int x, int y, char *text );

void CP_OnePlayerWarningMessage( void );

void DrawMultiPageCustomMenu( char *title, void ( *redrawfunc )( void ) );

#endif
