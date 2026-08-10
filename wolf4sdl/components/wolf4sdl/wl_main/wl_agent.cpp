// WL_AGENT.C

#include "wl_def.h"

/*
=============================================================================

                                LOCAL CONSTANTS

=============================================================================
*/

#define MAXMOUSETURN    10


#define MOVESCALE       150l
#define BACKMOVESCALE   100l
#define ANGLESCALE      20

/*
=============================================================================

                                GLOBAL VARIABLES

=============================================================================
*/



//
// player state info
//
int32_t         thrustspeed;

word            plux,pluy;          // player coordinates scaled to unsigned

short           anglefrac;

objtype        *LastAttacker;

/*
=============================================================================

                                                 LOCAL VARIABLES

=============================================================================
*/


void    T_Player (objtype *ob);
void    T_Attack (objtype *ob);

statetype   s_player = {false,0,0,(statefunc) T_Player,NULL,NULL};
statetype   s_attack = {false,0,0,(statefunc) T_Attack,NULL,NULL};

struct atkinf
{
    int8_t    tics,attack,frame;              // attack is 1 for gun, 2 for knife
} attackinfo[4][14] =
{
    { {6,0,1},{6,2,2},{6,0,3},{6,-1,4} },
    { {6,0,1},{6,1,2},{6,0,3},{6,-1,4} },
    { {6,0,1},{6,1,2},{6,3,3},{6,-1,4} },
    { {6,0,1},{6,1,2},{6,4,3},{6,-1,4} },
};

//===========================================================================

//----------

void Attack (void);
void Use (void);
void Search (objtype *ob);
void SelectWeapon (void);
void SelectItem (void);

//----------

boolean TryMove (objtype *ob);
void T_Player (objtype *ob);

void ClipMove (objtype *ob, int32_t xmove, int32_t ymove);

/*
=============================================================================

                                CONTROL STUFF

=============================================================================
*/

/*
======================
=
= CheckWeaponChange
=
= Keys 1-4 change weapons
=
======================
*/

void CheckWeaponChange (void)
{
    int newWeapon = -1;

    if(buttonstate[bt_nextweapon] && !buttonheld[bt_nextweapon])
    {
        newWeapon = gamestate.weapon;
        while(1)
        {
            newWeapon++;
            if(newWeapon > (int)gamestate.bestweapon) newWeapon = 0;
            if(newWeapon == (int)gamestate.weapon) break;
            
            if(newWeapon == wp_knife || gamestate.ammo > 0)
                break;
        }
    }
    else if(buttonstate[bt_prevweapon] && !buttonheld[bt_prevweapon])
    {
        newWeapon = gamestate.weapon;
        while(1)
        {
            newWeapon--;
            if(newWeapon < 0) newWeapon = (int)gamestate.bestweapon;
            if(newWeapon == (int)gamestate.weapon) break;

            if(newWeapon == wp_knife || gamestate.ammo > 0)
                break;
        }
    }
    else
    {
        for(int i = wp_knife; i <= (int)gamestate.bestweapon; i++)
        {
            if (buttonstate[bt_readyknife + i - wp_knife])
            {
                newWeapon = i;
                break;
            }
        }
    }

    if(newWeapon != -1 && (weapontype)newWeapon != gamestate.weapon)
    {
        gamestate.weapon = gamestate.chosenweapon = (weapontype) newWeapon;
        DrawWeapon();
    }
}


/*
=======================
=
= ControlMovement
=
= Takes controlx,controly, and buttonstate[bt_strafe]
=
= Changes the player's angle and position
=
= There is an angle hack because when going 70 fps, the roundoff becomes
= significant
=
=======================
*/

void ControlMovement (objtype *ob)
{
    int32_t oldx,oldy;
    int     angle;
    int     angleunits;

    thrustspeed = 0;

    oldx = player->x;
    oldy = player->y;

    if(buttonstate[bt_strafeleft])
    {
        angle = ob->angle + ANGLES/4;
        if(angle >= ANGLES)
            angle -= ANGLES;
        if(buttonstate[bt_run])
            Thrust(angle, RUNMOVE * MOVESCALE * tics);
        else
            Thrust(angle, BASEMOVE * MOVESCALE * tics);
    }

    if(buttonstate[bt_straferight])
    {
        angle = ob->angle - ANGLES/4;
        if(angle < 0)
            angle += ANGLES;
        if(buttonstate[bt_run])
            Thrust(angle, RUNMOVE * MOVESCALE * tics );
        else
            Thrust(angle, BASEMOVE * MOVESCALE * tics);
    }

    //
    // side to side move
    //
    if (buttonstate[bt_strafe])
    {
        //
        // strafing
        //
        //
        if (controlx > 0)
        {
            angle = ob->angle - ANGLES/4;
            if (angle < 0)
                angle += ANGLES;
            Thrust (angle,controlx*MOVESCALE);      // move to left
        }
        else if (controlx < 0)
        {
            angle = ob->angle + ANGLES/4;
            if (angle >= ANGLES)
                angle -= ANGLES;
            Thrust (angle,-controlx*MOVESCALE);     // move to right
        }
    }
    else
    {
        //
        // not strafing
        //
        anglefrac += controlx;
        angleunits = anglefrac/ANGLESCALE;
        anglefrac -= angleunits*ANGLESCALE;
        ob->angle -= angleunits;

        if (ob->angle >= ANGLES)
            ob->angle -= ANGLES;
        if (ob->angle < 0)
            ob->angle += ANGLES;

    }

    //
    // forward/backwards move
    //
    if (controly < 0)
    {
        Thrust (ob->angle,-controly*MOVESCALE); // move forwards
    }
    else if (controly > 0)
    {
        angle = ob->angle + ANGLES/2;
        if (angle >= ANGLES)
            angle -= ANGLES;
        Thrust (angle,controly*BACKMOVESCALE);          // move backwards
    }

    if (gamestate.victoryflag)              // watching the BJ actor
        return;
}

/*
=============================================================================

                            STATUS WINDOW STUFF

=============================================================================
*/


/*
==================
=
= StatusDrawPic
=
==================
*/

void StatusDrawPic (unsigned x, unsigned y, unsigned picnum)
{
    LatchDrawPicScaledCoord ((screenWidth-scaleFactor*320)/16 + scaleFactor*x,
        screenHeight-scaleFactor*(STATUSLINES-y),picnum);
}

void StatusDrawFace(unsigned picnum)
{
    StatusDrawPic(17, 4, picnum);

#ifdef _arch_dreamcast
    DC_StatusDrawLCD(picnum);
#endif
}


/*
==================
=
= DrawFace
=
==================
*/

void DrawFace (void)
{
    if (SD_SoundPlaying() == GETGATLINGSND)
        StatusDrawFace(GOTGATLINGPIC);
    else if (gamestate.health)
    {
#ifdef SPEAR
        if (godmode)
            StatusDrawFace(GODMODEFACE1PIC+gamestate.faceframe);
        else
#endif
            StatusDrawFace(FACE1APIC+3*((100-gamestate.health)/16)+gamestate.faceframe);
    }
    else
    {
#ifndef SPEAR
        if (LastAttacker && LastAttacker->obclass == needleobj)
            StatusDrawFace(MUTANTBJPIC);
        else
#endif
            StatusDrawFace(FACE8APIC);
    }
}

/*
===============
=
= UpdateFace
=
= Calls draw face if time to change
=
===============
*/

int facecount = 0;
int facetimes = 0;

void UpdateFace (void)
{
    // don't make demo depend on sound playback
    if(demoplayback || demorecord)
    {
        if(facetimes > 0)
        {
            facetimes--;
            return;
        }
    }
    else if(SD_SoundPlaying() == GETGATLINGSND)
        return;

    facecount += tics;
    if (facecount > US_RndT())
    {
        gamestate.faceframe = (US_RndT()>>6);
        if (gamestate.faceframe==3)
            gamestate.faceframe = 1;

        facecount = 0;
        DrawFace ();
    }
}



/*
===============
=
= LatchNumber
=
= right justifies and pads with blanks
=
===============
*/

static void LatchNumber (int x, int y, unsigned width, int32_t number)
{
    unsigned length,c;
    char    str[20];

    ltoa (number,str,10);

    length = (unsigned) strlen (str);

    while (length<width)
    {
        StatusDrawPic (x,y,N_BLANKPIC);
        x++;
        width--;
    }

    c = length <= width ? 0 : length-width;

    while (c<length)
    {
        StatusDrawPic (x,y,str[c]-'0'+ N_0PIC);
        x++;
        c++;
    }
}


/*
===============
=
= DrawHealth
=
===============
*/

void DrawHealth (void)
{
    LatchNumber (21,16,3,gamestate.health);
}


/*
===============
=
= TakeDamage
=
===============
*/

void TakeDamage (int points,objtype *attacker)
{
    LastAttacker = attacker;

    if (gamestate.victoryflag)
        return;
    if (gamestate.difficulty==gd_baby)
        points>>=2;

    if (!godmode)
        gamestate.health -= points;

    if (gamestate.health<=0)
    {
        gamestate.health = 0;
        playstate = ex_died;
        killerobj = attacker;
    }

    if (godmode != 2)
        StartDamageFlash (points);

    DrawHealth ();
    DrawFace ();

    //
    // MAKE BJ'S EYES BUG IF MAJOR DAMAGE!
    //
#ifdef SPEAR
    if (points > 30 && gamestate.health!=0 && !godmode && viewsize != 21)
    {
        StatusDrawFace(BJOUCHPIC);
        facecount = 0;
    }
#endif
}

/*
===============
=
= HealSelf
=
===============
*/

void HealSelf (int points)
{
    gamestate.health += points;
    if (gamestate.health>100)
        gamestate.health = 100;

    DrawHealth ();
    DrawFace ();
}


//===========================================================================


/*
===============
=
= DrawLevel
=
===============
*/

void DrawLevel (void)
{
#ifdef SPEAR
    if (gamestate.mapon == 20)
        LatchNumber (2,16,2,18);
    else
#endif
        LatchNumber (2,16,2,gamestate.mapon+1);
}

//===========================================================================


/*
===============
=
= DrawLives
=
===============
*/

void DrawLives (void)
{
    LatchNumber (14,16,1,gamestate.lives);
}


/*
===============
=
= GiveExtraMan
=
===============
*/

void GiveExtraMan (void)
{
    if (gamestate.lives<9)
        gamestate.lives++;
    DrawLives ();
    SD_PlaySound (BONUS1UPSND);
}

//===========================================================================

/*
===============
=
= DrawScore
=
===============
*/

void DrawScore (void)
{
    LatchNumber (6,16,6,gamestate.score);
}

/*
===============
=
= GivePoints
=
===============
*/

void GivePoints (int32_t points)
{
    gamestate.score += points;
    while (gamestate.score >= gamestate.nextextra)
    {
        gamestate.nextextra += EXTRAPOINTS;
        GiveExtraMan ();
    }
    DrawScore ();
}

//===========================================================================

/*
==================
=
= DrawWeapon
=
==================
*/

void DrawWeapon (void)
{
    StatusDrawPic (32,8,KNIFEPIC + (int)gamestate.weapon);
}


/*
==================
=
= DrawKeys
=
==================
*/

void DrawKeys (void)
{
    if (gamestate.keys & 1)
        StatusDrawPic (30,4,GOLDKEYPIC);
    else
        StatusDrawPic (30,4,NOKEYPIC);

    if (gamestate.keys & 2)
        StatusDrawPic (30,20,SILVERKEYPIC);
    else
        StatusDrawPic (30,20,NOKEYPIC);
}

/*
==================
=
= GiveWeapon
=
==================
*/

void GiveWeapon (int weapon)
{
    GiveAmmo (6);

    if (gamestate.bestweapon<weapon)
        gamestate.bestweapon = gamestate.weapon
        = gamestate.chosenweapon = (weapontype) weapon;

    DrawWeapon ();
}

//===========================================================================

/*
===============
=
= DrawAmmo
=
===============
*/

void DrawAmmo (void)
{
    LatchNumber (27,16,2,gamestate.ammo);
}

/*
===============
=
= GiveAmmo
=
===============
*/

void GiveAmmo (int ammo)
{
    if (!gamestate.ammo)                            // knife was out
    {
        if (!gamestate.attackframe)
        {
            gamestate.weapon = gamestate.chosenweapon;
            DrawWeapon ();
        }
    }
    gamestate.ammo += ammo;
    if (gamestate.ammo > 99)
        gamestate.ammo = 99;
    DrawAmmo ();
}

//===========================================================================

/*
==================
=
= GiveKey
=
==================
*/

void GiveKey (int key)
{
    gamestate.keys |= (1<<key);
    DrawKeys ();
}



/*
=============================================================================

                                MOVEMENT

=============================================================================
*/


/*
===================
=
= GetBonus
=
===================
*/
void GetBonus (statobj_t *check)
{
    switch (check->itemnumber)
    {
        case    bo_firstaid:
            if (gamestate.health == 100)
                return;

            SD_PlaySound (HEALTH2SND);
            HealSelf (25);
            break;

        case    bo_key1:
        case    bo_key2:
        case    bo_key3:
        case    bo_key4:
            GiveKey (check->itemnumber - bo_key1);
            SD_PlaySound (GETKEYSND);
            break;

        case    bo_cross:
            SD_PlaySound (BONUS1SND);
            GivePoints (100);
            gamestate.treasurecount++;
            break;
        case    bo_chalice:
            SD_PlaySound (BONUS2SND);
            GivePoints (500);
            gamestate.treasurecount++;
            break;
        case    bo_bible:
            SD_PlaySound (BONUS3SND);
            GivePoints (1000);
            gamestate.treasurecount++;
            break;
        case    bo_crown:
            SD_PlaySound (BONUS4SND);
            GivePoints (5000);
            gamestate.treasurecount++;
            break;

        case    bo_clip:
            if (gamestate.ammo == 99)
                return;

            SD_PlaySound (GETAMMOSND);
            GiveAmmo (8);
            break;
        case    bo_clip2:
            if (gamestate.ammo == 99)
                return;

            SD_PlaySound (GETAMMOSND);
            GiveAmmo (4);
            break;

#ifdef SPEAR
        case    bo_25clip:
            if (gamestate.ammo == 99)
                return;

            SD_PlaySound (GETAMMOBOXSND);
            GiveAmmo (25);
            break;
#endif

        case    bo_machinegun:
            SD_PlaySound (GETMACHINESND);
            GiveWeapon (wp_machinegun);
            break;
        case    bo_chaingun:
            SD_PlaySound (GETGATLINGSND);
            facetimes = 38;
            GiveWeapon (wp_chaingun);

            if(viewsize != 21)
                StatusDrawFace (GOTGATLINGPIC);
            facecount = 0;
            break;

        case    bo_fullheal:
            SD_PlaySound (BONUS1UPSND);
            HealSelf (99);
            GiveAmmo (25);
            GiveExtraMan ();
            gamestate.treasurecount++;
            break;

        case    bo_food:
            if (gamestate.health == 100)
                return;

            SD_PlaySound (HEALTH1SND);
            HealSelf (10);
            break;

        case    bo_alpo:
            if (gamestate.health == 100)
                return;

            SD_PlaySound (HEALTH1SND);
            HealSelf (4);
            break;

        case    bo_gibs:
            if (gamestate.health >10)
                return;

            SD_PlaySound (SLURPIESND);
            HealSelf (1);
            break;

#ifdef SPEAR
        case    bo_spear:
            spearflag = true;
            spearx = player->x;
            speary = player->y;
            spearangle = player->angle;
            playstate = ex_completed;
#endif
    }

    StartBonusFlash ();
    check->shapenum = -1;                   // remove from list
}

/*
===================
=
= TryMove
=
= returns true if move ok
= debug: use pointers to optimize
===================
*/

boolean TryMove (objtype *ob)
{
    int         xl,yl,xh,yh,x,y;
    objtype    *check;
    int32_t     deltax,deltay;

    xl = (ob->x-PLAYERSIZE) >>TILESHIFT;
    yl = (ob->y-PLAYERSIZE) >>TILESHIFT;

    xh = (ob->x+PLAYERSIZE) >>TILESHIFT;
    yh = (ob->y+PLAYERSIZE) >>TILESHIFT;

#define PUSHWALLMINDIST PLAYERSIZE

    //
    // check for solid walls
    //
    for (y=yl;y<=yh;y++)
    {
        for (x=xl;x<=xh;x++)
        {
            check = actorat[x][y];
            if (check && !ISPOINTER(check))
            {
                if(tilemap[x][y]==64 && x==pwallx && y==pwally)   // back of moving pushwall?
                {
                    switch(pwalldir)
                    {
                        case di_north:
                            if(ob->y-PUSHWALLMINDIST<=(pwally<<TILESHIFT)+((63-pwallpos)<<10))
                                return false;
                            break;
                        case di_west:
                            if(ob->x-PUSHWALLMINDIST<=(pwallx<<TILESHIFT)+((63-pwallpos)<<10))
                                return false;
                            break;
                        case di_east:
                            if(ob->x+PUSHWALLMINDIST>=(pwallx<<TILESHIFT)+(pwallpos<<10))
                                return false;
                            break;
                        case di_south:
                            if(ob->y+PUSHWALLMINDIST>=(pwally<<TILESHIFT)+(pwallpos<<10))
                                return false;
                            break;
                    }
                }
                else return false;
            }
        }
    }

    //
    // check for actors
    //
    if (yl>0)
        yl--;
    if (yh<MAPSIZE-1)
        yh++;
    if (xl>0)
        xl--;
    if (xh<MAPSIZE-1)
        xh++;

    for (y=yl;y<=yh;y++)
    {
        for (x=xl;x<=xh;x++)
        {
            check = actorat[x][y];
            if (ISPOINTER(check) && check != player && (check->flags & FL_SHOOTABLE) )
            {
                deltax = ob->x - check->x;
                if (deltax < -MINACTORDIST || deltax > MINACTORDIST)
                    continue;
                deltay = ob->y - check->y;
                if (deltay < -MINACTORDIST || deltay > MINACTORDIST)
                    continue;

                return false;
            }
        }
    }

    return true;
}


/*
===================
=
= ClipMove
=
===================
*/

void ClipMove (objtype *ob, int32_t xmove, int32_t ymove)
{
    int32_t    basex,basey;

    basex = ob->x;
    basey = ob->y;

    ob->x = basex+xmove;
    ob->y = basey+ymove;
    if (TryMove (ob))
        return;

#ifndef REMDEBUG
    if (noclip && ob->x > 2*TILEGLOBAL && ob->y > 2*TILEGLOBAL
        && ob->x < (((int32_t)(mapwidth-1))<<TILESHIFT)
        && ob->y < (((int32_t)(mapheight-1))<<TILESHIFT) )
        return;         // walk through walls
#endif

    if (!SD_SoundPlaying())
        SD_PlaySound (HITWALLSND);

    ob->x = basex+xmove;
    ob->y = basey;
    if (TryMove (ob))
        return;

    ob->x = basex;
    ob->y = basey+ymove;
    if (TryMove (ob))
        return;

    ob->x = basex;
    ob->y = basey;
}

//==========================================================================

/*
===================
=
= VictoryTile
=
===================
*/

void VictoryTile (void)
{
#ifndef SPEAR
    SpawnBJVictory ();
#endif

    gamestate.victoryflag = true;
}

/*
===================
=
= Thrust
=
===================
*/

// For player movement in demos exactly as in the original Wolf3D v1.4 source code
static fixed FixedByFracOrig(fixed a, fixed b)
{
    int sign = 0;
    if(b == 65536) b = 65535;
    else if(b == -65536) b = 65535, sign = 1;
    else if(b < 0) b = (-b), sign = 1;

    if(a < 0)
    {
        a = -a;
        sign = !sign;
    }
    fixed res = (fixed)(((int64_t) a * b) >> 16);
    if(sign)
        res = -res;
    return res;
}

void Thrust (int angle, int32_t speed)
{
    int32_t xmove,ymove;
    unsigned offset;


    //
    // ZERO FUNNY COUNTER IF MOVED!
    //
#ifdef SPEAR
    if (speed)
        funnyticount = 0;
#endif

    thrustspeed += speed;
    //
    // moving bounds speed
    //
    if (speed >= MINDIST*2)
        speed = MINDIST*2-1;

    xmove = DEMOCHOOSE_ORIG_SDL(
                FixedByFracOrig(speed, costable[angle]),
                FixedMul(speed,costable[angle]));
    ymove = DEMOCHOOSE_ORIG_SDL(
                -FixedByFracOrig(speed, sintable[angle]),
                -FixedMul(speed,sintable[angle]));

    ClipMove(player,xmove,ymove);

    player->tilex = (short)(player->x >> TILESHIFT);                // scale to tile values
    player->tiley = (short)(player->y >> TILESHIFT);

    offset = (player->tiley<<mapshift)+player->tilex;
    player->areanumber = *(mapsegs[0] + offset) -AREATILE;

    if (*(mapsegs[1] + offset) == EXITTILE)
        VictoryTile ();
}


/*
=============================================================================

                                ACTIONS

=============================================================================
*/


/*
===============
=
= Cmd_Fire
=
===============
*/

void Cmd_Fire (void)
{
    buttonheld[bt_attack] = true;

    gamestate.weaponframe = 0;

    player->state = &s_attack;

    gamestate.attackframe = 0;
    gamestate.attackcount =
        attackinfo[gamestate.weapon][gamestate.attackframe].tics;
    gamestate.weaponframe =
        attackinfo[gamestate.weapon][gamestate.attackframe].frame;
}

//===========================================================================

/*
===============
=
= Cmd_Use
=
===============
*/

void Cmd_Use (void)
{
    int     checkx,checky,doornum,dir;
    boolean elevatorok;

    //
    // find which cardinal direction the player is facing
    //
    if (player->angle < ANGLES/8 || player->angle > 7*ANGLES/8)
    {
        checkx = player->tilex + 1;
        checky = player->tiley;
        dir = di_east;
        elevatorok = true;
    }
    else if (player->angle < 3*ANGLES/8)
    {
        checkx = player->tilex;
        checky = player->tiley-1;
        dir = di_north;
        elevatorok = false;
    }
    else if (player->angle < 5*ANGLES/8)
    {
        checkx = player->tilex - 1;
        checky = player->tiley;
        dir = di_west;
        elevatorok = true;
    }
    else
    {
        checkx = player->tilex;
        checky = player->tiley + 1;
        dir = di_south;
        elevatorok = false;
    }

    doornum = tilemap[checkx][checky];
    if (*(mapsegs[1]+(checky<<mapshift)+checkx) == PUSHABLETILE)
    {
        //
        // pushable wall
        //

        PushWall (checkx,checky,dir);
        return;
    }
    if (!buttonheld[bt_use] && doornum == ELEVATORTILE && elevatorok)
    {
        //
        // use elevator
        //
        buttonheld[bt_use] = true;

        tilemap[checkx][checky]++;              // flip switch
        if (*(mapsegs[0]+(player->tiley<<mapshift)+player->tilex) == ALTELEVATORTILE)
            playstate = ex_secretlevel;
        else
            playstate = ex_completed;
        SD_PlaySound (LEVELDONESND);
        SD_WaitSoundDone();
    }
    else if (!buttonheld[bt_use] && doornum & 0x80)
    {
        buttonheld[bt_use] = true;
        OperateDoor (doornum & ~0x80);
    }
    else
        SD_PlaySound (DONOTHINGSND);
}

/*
=============================================================================

                                PLAYER CONTROL

=============================================================================
*/



/*
===============
=
= SpawnPlayer
=
===============
*/

void SpawnPlayer (int tilex, int tiley, int dir)
{
    player->obclass = playerobj;
    player->active = ac_yes;
    player->tilex = tilex;
    player->tiley = tiley;
    player->areanumber = (byte) *(mapsegs[0]+(player->tiley<<mapshift)+player->tilex);
    player->x = ((int32_t)tilex<<TILESHIFT)+TILEGLOBAL/2;
    player->y = ((int32_t)tiley<<TILESHIFT)+TILEGLOBAL/2;
    player->state = &s_player;
    player->angle = (1-dir)*90;
    if (player->angle<0)
        player->angle += ANGLES;
    player->flags = FL_NEVERMARK;
    Thrust (0,0);                           // set some variables

    InitAreas ();
}


//===========================================================================

/*
===============
=
= T_KnifeAttack
=
= Update player hands, and try to do damage when the proper frame is reached
=
===============
*/

void    KnifeAttack (objtype *ob)
{
    objtype *check,*closest;
    int32_t  dist;

    SD_PlaySound (ATKKNIFESND);
    // actually fire
    dist = 0x7fffffff;
    closest = NULL;
    for (check=ob->next; check; check=check->next)
    {
        if ( (check->flags & FL_SHOOTABLE) && (check->flags & FL_VISABLE)
            && abs(check->viewx-centerx) < shootdelta)
        {
            if (check->transx < dist)
            {
                dist = check->transx;
                closest = check;
            }
        }
    }

    if (!closest || dist > 0x18000l)
    {
        // missed
        return;
    }

    // hit something
    DamageActor (closest,US_RndT() >> 4);
}



void    GunAttack (objtype *ob)
{
    objtype *check,*closest,*oldclosest;
    int      damage;
    int      dx,dy,dist;
    int32_t  viewdist;

    switch (gamestate.weapon)
    {
        case wp_pistol:
            SD_PlaySound (ATKPISTOLSND);
            break;
        case wp_machinegun:
            SD_PlaySound (ATKMACHINEGUNSND);
            break;
        case wp_chaingun:
            SD_PlaySound (ATKGATLINGSND);
            break;
        default:
            break;
    }

    madnoise = true;

    //
    // find potential targets
    //
    viewdist = 0x7fffffffl;
    closest = NULL;

    while (1)
    {
        oldclosest = closest;

        for (check=ob->next ; check ; check=check->next)
        {
            if ((check->flags & FL_SHOOTABLE) && (check->flags & FL_VISABLE)
                && abs(check->viewx-centerx) < shootdelta)
            {
                if (check->transx < viewdist)
                {
                    viewdist = check->transx;
                    closest = check;
                }
            }
        }

        if (closest == oldclosest)
            return;                                         // no more targets, all missed

        //
        // trace a line from player to enemey
        //
        if (CheckLine(closest))
            break;
    }

    //
    // hit something
    //
    dx = ABS(closest->tilex - player->tilex);
    dy = ABS(closest->tiley - player->tiley);
    dist = dx>dy ? dx:dy;
    if (dist<2)
        damage = US_RndT() / 4;
    else if (dist<4)
        damage = US_RndT() / 6;
    else
    {
        if ( (US_RndT() / 12) < dist)           // missed
            return;
        damage = US_RndT() / 6;
    }
    DamageActor (closest,damage);
}

//===========================================================================

/*
===============
=
= VictorySpin
=
===============
*/

void VictorySpin (void)
{
    int32_t    desty;

    if (player->angle > 270)
    {
        player->angle -= (short)(tics * 3);
        if (player->angle < 270)
            player->angle = 270;
    }
    else if (player->angle < 270)
    {
        player->angle += (short)(tics * 3);
        if (player->angle > 270)
            player->angle = 270;
    }

    desty = (((int32_t)player->tiley-5)<<TILESHIFT)-0x3000;

    if (player->y > desty)
    {
        player->y -= tics*4096;
        if (player->y < desty)
            player->y = desty;
    }
}


//===========================================================================

/*
===============
=
= T_Attack
=
===============
*/

void    T_Attack (objtype *ob)
{
    struct  atkinf  *cur;

    UpdateFace ();

    if (gamestate.victoryflag)              // watching the BJ actor
    {
        VictorySpin ();
        return;
    }

    if ( buttonstate[bt_use] && !buttonheld[bt_use] )
        buttonstate[bt_use] = false;

    if ( buttonstate[bt_attack] && !buttonheld[bt_attack])
        buttonstate[bt_attack] = false;

    ControlMovement (ob);
    if (gamestate.victoryflag)              // watching the BJ actor
        return;

    plux = (word) (player->x >> UNSIGNEDSHIFT);                     // scale to fit in unsigned
    pluy = (word) (player->y >> UNSIGNEDSHIFT);
    player->tilex = (short)(player->x >> TILESHIFT);                // scale to tile values
    player->tiley = (short)(player->y >> TILESHIFT);

    //
    // change frame and fire
    //
    gamestate.attackcount -= (short) tics;
    while (gamestate.attackcount <= 0)
    {
        cur = &attackinfo[gamestate.weapon][gamestate.attackframe];
        switch (cur->attack)
        {
            case -1:
                ob->state = &s_player;
                if (!gamestate.ammo)
                {
                    gamestate.weapon = wp_knife;
                    DrawWeapon ();
                }
                else
                {
                    if (gamestate.weapon != gamestate.chosenweapon)
                    {
                        gamestate.weapon = gamestate.chosenweapon;
                        DrawWeapon ();
                    }
                }
                gamestate.attackframe = gamestate.weaponframe = 0;
                return;

            case 4:
                if (!gamestate.ammo)
                    break;
                if (buttonstate[bt_attack])
                    gamestate.attackframe -= 2;
            case 1:
                if (!gamestate.ammo)
                {       // can only happen with chain gun
                    gamestate.attackframe++;
                    break;
                }
                GunAttack (ob);
                if (!ammocheat)
                    gamestate.ammo--;
                DrawAmmo ();
                break;

            case 2:
                KnifeAttack (ob);
                break;

            case 3:
                if (gamestate.ammo && buttonstate[bt_attack])
                    gamestate.attackframe -= 2;
                break;
        }

        gamestate.attackcount += cur->tics;
        gamestate.attackframe++;
        gamestate.weaponframe =
            attackinfo[gamestate.weapon][gamestate.attackframe].frame;
    }
}



//===========================================================================

/*
===============
=
= T_Player
=
===============
*/

void    T_Player (objtype *ob)
{
    if (gamestate.victoryflag)              // watching the BJ actor
    {
        VictorySpin ();
        return;
    }

    UpdateFace ();
    CheckWeaponChange ();

    if ( buttonstate[bt_use] )
        Cmd_Use ();

    if ( buttonstate[bt_attack] && !buttonheld[bt_attack])
        Cmd_Fire ();

    ControlMovement (ob);
    if (gamestate.victoryflag)              // watching the BJ actor
        return;

    plux = (word) (player->x >> UNSIGNEDSHIFT);                     // scale to fit in unsigned
    pluy = (word) (player->y >> UNSIGNEDSHIFT);
    player->tilex = (short)(player->x >> TILESHIFT);                // scale to tile values
    player->tiley = (short)(player->y >> TILESHIFT);
}
