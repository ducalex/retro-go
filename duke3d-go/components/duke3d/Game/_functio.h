//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment

This file is part of Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
aint32_t with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------

// _functio.h

// file created by makehead.exe
// these headers contain default key assignments, as well as
// default button assignments and game function names
// axis defaults are also included


#ifndef _function_private_
#define _function_private_
#ifdef __cplusplus
extern "C" {
#endif
const char  * gamefunctions[] =
   {
   "Move_Forward",
   "Move_Backward",
   "Turn_Left",
   "Turn_Right",
   "Strafe",
   "Fire",
   "Open",
   "Run",
   "AutoRun",
   "Jump",
   "Crouch",
   "Look_Up",
   "Look_Down",
   "Look_Left",
   "Look_Right",
   "Strafe_Left",
   "Strafe_Right",
   "Aim_Up",
   "Aim_Down",
   "Weapon_1",
   "Weapon_2",
   "Weapon_3",
   "Weapon_4",
   "Weapon_5",
   "Weapon_6",
   "Weapon_7",
   "Weapon_8",
   "Weapon_9",
   "Weapon_10",
   "Inventory",
   "Inventory_Left",
   "Inventory_Right",
   "Holo_Duke",
   "Jetpack",
   "NightVision",
   "MedKit",
   "TurnAround",
   "SendMessage",
   "Map",
   "Shrink_Screen",
   "Enlarge_Screen",
   "Center_View",
   "Holster_Weapon",
   "Show_Opponents_Weapon",
   "Map_Follow_Mode",
   "See_Coop_View",
   "Mouse_Aiming",
   "Toggle_Crosshair",
   "Steroids",
   "Quick_Kick",
   "Next_Weapon",
   "Previous_Weapon",
   "Hide_Weapon",
   "Auto_Aim",
   "Console"
   };

	#define NUMKEYENTRIES 55  // Don't forget to change NUMGAMEFUNCTIONS as well

// FIX_00011: duke3d.cfg not needed anymore to start the game. Will create a default one
// if not found and use default keys.

struct keyEntry{
	char * entryKey;
	char * keyname1;
	char * keyname2;
} ;
    
    
const struct keyEntry    keydefaults[] = {
	{ "Move_Forward", 	"Up", 		"Kpad8" }, 
	{ "Move_Backward", 	"Down", 	"Kpad2" }, 
	{ "Turn_Left", 		"Left", 	"Kpad4" }, 
	{ "Turn_Right", 	"Right", 	"KPad6" }, 
	{ "Strafe", 		"LAlt", 	"RAlt" }, 
	{ "Fire", 			"LCtrl", 	"RCtrl" }, 
	{ "Open", 			"Space", 	"" }, 
	{ "Run", 			"LShift", 	"RShift" }, 
	{ "AutoRun", 		"CapLck", 	"" }, 
	{ "Jump", 			"A", 		"/" }, 
	{ "Crouch", 		"Z", 		"" }, 
	{ "Look_Up", 		"PgUp", 	"Kpad9" }, 
	{ "Look_Down", 		"PgDn", 	"Kpad3" }, 
	{ "Look_Left", 		"Insert", 	"Kpad0" }, 
	{ "Look_Right", 	"Delete", 	"Kpad." }, 
	{ "Strafe_Left", 	",", 		"" }, 
	{ "Strafe_Right", 	".", 		"" }, 
	{ "Aim_Up", 		"Home", 	"KPad7" }, 
	{ "Aim_Down", 		"End", 		"Kpad1" }, 
	{ "Weapon_1", 		"1", 		"" }, 
	{ "Weapon_2", 		"2", 		"" }, 
	{ "Weapon_3", 		"3", 		"" }, 
	{ "Weapon_4", 		"4", 		"" }, 
	{ "Weapon_5", 		"5", 		"" }, 
	{ "Weapon_6", 		"6", 		"" }, 
	{ "Weapon_7", 		"7", 		"" }, 
	{ "Weapon_8", 		"8", 		"" }, 
	{ "Weapon_9", 		"9", 		"" }, 
	{ "Weapon_10",		"0", 		"" }, 
	{ "Inventory", 		"Enter", 	"KpdEnt" }, 
	{ "Inventory_Left",	"[", 		"" }, 
	{ "Inventory_Right","]", 		"" }, 
	{ "Holo_Duke", 		"H", 		"" }, 
	{ "Jetpack", 		"J", 		"" }, 
	{ "NightVision", 	"N", 		"" }, 
	{ "MedKit", 		"M", "		" }, 
	{ "TurnAround", 	"BakSpc", 	"" }, 
	{ "SendMessage", 	"T", 		"" }, 
	{ "Map", 			"Tab", 		"" }, 
	{ "Shrink_Screen", 	"-", 		"Kpad-" }, 
	{ "Enlarge_Screen", "=", 		"Kpad+" }, 
	{ "Center_View", 	"KPad5", 	"" }, 
	{ "Holster_Weapon", "ScrLck", 	"" }, 
	{ "Show_Opponents_Weapon", "W", "" }, 
	{ "Map_Follow_Mode","F", 		"" }, 
	{ "See_Coop_View", 	"K", 		"" }, 
	{ "Mouse_Aiming", 	"U", 		"" }, 
	{ "Toggle_Crosshair", "I", 		"" }, 
	{ "Steroids", 		"R", 		"" }, 
	{ "Quick_Kick", 	"C", 		"" }, 
	{ "Next_Weapon", 	"'", 		"" }, 
	{ "Previous_Weapon",";", 		"" },
	{ "Hide_Weapon", 	"S", 		"" },
	{ "Auto_Aim", 		"V", 		"" },
	{ "Console", 		"`", 		"" }
};



#ifdef __cplusplus
};
#endif
#endif
