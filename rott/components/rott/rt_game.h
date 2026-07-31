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
#ifndef _rt_game_public
#define _rt_game_public

//***************************************************************************
//
// Public header for RT_GAME.C
//
//***************************************************************************

#include "rt_actor.h"
#include "lumpy.h"
#include "rt_cfg.h"
#include "rt_playr.h"

//***************************************************************************
//
// DEFINES
//
//***************************************************************************

#define  MaxHighName 57
#define  MaxScores   7


//***************************************************************************
//
// TYPEDEFS
//
//***************************************************************************

typedef struct
{
   char  message[30];
   byte  episode;
   byte  area;
   byte  version;
   byte  picture[16000];
   word  mapcrc;
   AlternateInformation info;
} gamestorage_t;

typedef  struct
{
	char  name[MaxHighName + 1];
	long  score;
	word  completed,episode;
} HighScore;


//***************************************************************************
//
// GLOBALS
//
//***************************************************************************

extern int PlayerSnds[5];

//extern int SHAKETICS;
extern unsigned short SHAKETICS;//bna++
extern int damagecount;

extern HighScore   Scores[MaxScores];
extern int SaveTime;

//***************************************************************************
//
// PROTOTYPES
//
//***************************************************************************

void SetupPlayScreen (void);
void SD_PreCache (void);
void GameMemToScreen( pic_t *source, int x, int y, int bufferofsonly );
void DrawPlayScreen (boolean bufferofsonly);

void DrawKills (boolean bufferofsonly);
void DrawPlayers ( void );
void DrawGameString (int x, int y, const char * str, boolean bufferofsonly);
void DrawNumber (int x, int y, int width, int which, boolean bufferofsonly);
void TakeDamage (int points, objtype *attacker);
void HealPlayer (int points, objtype * ob);
void DrawLives (boolean bufferofsonly);
void GiveExtraMan (void);
void DrawScore (boolean bufferofsonly);
void GivePoints (long points);
void DrawKeys (boolean bufferofsonly);
void GiveKey (int key);
void GiveWeapon (objtype * ob, int weapon);
void GiveMissileWeapon(objtype * ob, int which);
void GiveLives (int newlives);

void UpdateScore (unsigned int num);
void UpdateLives (int num);

void DrawTimeXY( int x, int y, int sec, boolean bufferofsonly );
void DrawTime (boolean bufferofsonly);

boolean SaveTheGame (int num, gamestorage_t * game);
boolean LoadTheGame (int num, gamestorage_t * game);
void GetSavedMessage (int num, char * message);
void GetSavedHeader (int num, gamestorage_t * game);

void DrawHighScores (void);
void CheckHighScore (long score, word other, boolean INMENU);
void LevelCompleted ( exit_t playstate );
void BattleLevelCompleted ( int localplayer );
void Died (void);
void ScreenShake (void);
void UpdateTriads (objtype * ob, int num);
void DrawTriads (boolean bufferofsonly);
void DrawStats (void);
void DrawBarHealth (boolean bufferonly);
void DrawBarAmmo (boolean bufferonly);
void GM_DrawBonus (int which);

void DrawEpisodeLevel ( int x, int y );
void DoBorderShifts (void);
void GM_UpdateBonus (int time, int powerup);
void DoLoadGameAction (void);
int GetLevel (int episode, int mapon);
void DrawPause (void);
void DrawPauseXY (int x, int y);

void DrawColoredMPPic (int xpos, int ypos, int width, int height, int heightmod, byte *src, boolean bufferofsonly, int color);
void StatusDrawColoredPic (unsigned x, unsigned y, pic_t *nums, boolean bufferofsonly, int color);

void ClearTriads (playertype * pstate);
#endif
