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



/*
============================================================================

													 TIMER INTERRUPT

============================================================================
*/
#include "rt_def.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef DOS
#include <dos.h>
#include <mem.h>
#include <conio.h>
#endif

#include "task_man.h"
#include "isr.h"
#include "_isr.h"
#include "rt_in.h"
#include "rt_util.h"
#include "profile.h"
#include "develop.h"
#include "rt_main.h"

#if (DEVELOPMENT == 1)

#include "rt_vid.h"

#endif
//MED
#include "memcheck.h"

// Global Variables

static volatile boolean ExtendedKeyFlag;

#ifndef RETRO_GO
volatile int Keyboard[MAXKEYBOARDSCAN];
volatile int KeyboardQueue[KEYQMAX];
volatile int Keystate[MAXKEYBOARDSCAN];
#endif
volatile int Keyhead;
volatile int Keytail;

#ifdef DOS
volatile int ticcount;
volatile int fasttics;
#endif

volatile boolean PausePressed = false;
volatile boolean PanicPressed = false;
int KeyboardStarted=false;

const int ASCIINames[] =          // Unshifted ASCII for scan codes
{
//       0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
		  0  ,27 ,'1','2','3','4','5','6','7','8','9','0','-','=',8  ,9  ,        // 0
		  'q','w','e','r','t','y','u','i','o','p','[',']',13 ,0  ,'a','s',        // 1
		  'd','f','g','h','j','k','l',';',39 ,'`',0  ,92 ,'z','x','c','v',        // 2
		  'b','n','m',',','.','/',0  ,'*',0  ,' ',0  ,0  ,0  ,0  ,0  ,0  ,        // 3
		  0  ,0  ,0  ,0  ,0  ,0  ,0  ,'7','8','9','-','4','5','6','+','1',        // 4
		  '2','3','0',127,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,        // 5
		  0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,        // 6
		  0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0           // 7
};

const int ShiftNames[] =              // Shifted ASCII for scan codes
{
//       0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
		  0  ,27 ,'!','@','#','$','%','^','&','*','(',')','_','+',8  ,9  ,        // 0
		  'Q','W','E','R','T','Y','U','I','O','P','{','}',13 ,0  ,'A','S',        // 1
		  'D','F','G','H','J','K','L',':',34 ,'~',0  ,'|','Z','X','C','V',        // 2
		  'B','N','M','<','>','?',0  ,'*',0  ,' ',0  ,0  ,0  ,0  ,0  ,0  ,        // 3
		  0  ,0  ,0  ,0  ,0  ,0  ,0  ,'7','8','9','-','4','5','6','+','1',        // 4
		  '2','3','0',127,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,        // 5
		  0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,        // 6
		  0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0           // 7
};

#ifdef DOS

// Local Variables

static task * timertask;
#if (DEVELOPMENT == 1)
static task * fasttimertask;
#endif
static int TimerStarted=false;
static volatile int pausecount=0;
static struct dostime_t starttime;

void (__interrupt __far *oldtimerisr) ();
void (__interrupt __far *oldkeyboardisr) () = NULL;

static int LEDs;
static volatile int KBFlags;

/*
================
=
= I_TimerISR
=
================
*/

void __interrupt I_TimerISR (void)
{
// acknowledge the interrupt

		  OUTP(0x20,0x20);
		  ticcount++;
}

/*
================
=
= ISR_Timer
=
================
*/
#if 0
#if (DEVELOPMENT == 1)
static int time=0;
static int t1=0;
static int t2=0;
static int t3=0;
static int insettime=0;
#endif
#endif
/*
================
=
= ISR_SetTime
=
================
*/
void ISR_SetTime(int settime)
{
#if 0
#if (DEVELOPMENT == 1)
   int i;
   int t;
   int savetime;
#endif
#endif

   ticcount=settime;
#if 0
#if (DEVELOPMENT == 1)
      {
      insettime=1;
      savetime=time;
      t1=0;
      t2=0;
      t3=0;
      t=0;

      for (i=0;i<settime;i++)
         {
         t++;
         if (t==VBLCOUNTER)
            {
            t=0;
            t1++;
            if (t1==2)
               {
               t1=0;
               t2++;
               if (t2==2)
                  {
                  t2=0;
                  t3++;
                  if (t3==2)
                     t3=0;
                  }
               }
            }
         }
      time=t+(time-savetime);
      if (time>=VBLCOUNTER)
         {
         time-=VBLCOUNTER;
         t1++;
         if (t1==2)
            {
            t1=0;
            t2++;
            if (t2==2)
               {
               t2=0;
               t3++;
               if (t3==2)
                  t3=0;
               }
            }
         }
      insettime=0;
      }
#endif
#endif
}

static void ISR_Timer (task *Task)
{
//	(*(int *)(Task->data))=((*(int *)(Task->data))+1)&0xffff;
	(*(int *)(Task->data))++;

#if 0
#if (DEVELOPMENT == 1)
      {
      if (Task==timertask)
         {
         time++;
         if ((time==VBLCOUNTER) && (insettime==0))
            {
            time=0;
            VL_SetColor(0,(t1<<5)+20,(t2<<5)+20,(t3<<5)+20);
            t1++;
            if (t1==2)
               {
               t1=0;
               t2++;
               if (t2==2)
                  {
                  t2=0;
                  t3++;
                  if (t3==2)
                     t3=0;
                  }
               }
            }
         }
      }
#endif
#endif
}


/*
=====================
=
= I_SetTimer0
=
= Sets system timer 0 to the specified speed
=
=====================
*/

void I_SetTimer0(int speed)
{
	unsigned s;
		  if (!((speed > 0 && speed < 150)))
					 Error ("INT_SetTimer0: %i is a bad value",speed);
		  s=1192030U/(unsigned)speed;
		  OUTP(0x43,0x36);                            // Change timer 0
		  OUTP(0x40,s);
		  OUTP(0x40,s >> 8);
}

/*
================
=
= I_Delay
=
================
*/

void I_Delay ( int delay )
{
   int time;

   delay=(VBLCOUNTER*delay)/10;
   IN_ClearKeysDown();
   time=ticcount;
   while (ticcount<time+delay)
      {
      if (LastScan)
         break;
      }
}

/*
===============
=
= I_StartupTimer
=
===============
*/

void I_StartupTimer (void)
{
#if PROFILE
   return;
#else
   struct dostime_t cmostime;

   if (TimerStarted==true)
      return;
   TimerStarted=true;

   I_GetCMOSTime ( &cmostime );
   _dos_settime  ( &cmostime );

//      I_SetTimer0(VBLCOUNTER);
//      oldtimerisr = _dos_getvect(TIMERINT);
//      _dos_setvect (TIMERINT, I_TimerISR);

   timertask=TS_ScheduleTask( &ISR_Timer, VBLCOUNTER, 10, &ticcount);
#if (DEVELOPMENT == 1)
   fasttimertask=TS_ScheduleTask( &ISR_Timer, VBLCOUNTER*4, 10, &fasttics);
#endif
   TS_Dispatch();
   I_GetCMOSTime ( &cmostime );
   memcpy(&starttime,&cmostime,sizeof(starttime));
   ticcount=0;
   if (!quiet)
      printf("I_StartupTimer: Timer Started\n");
#endif
}

void I_ShutdownTimer (void)
{
#if PROFILE
   return;
#else
   struct dostime_t dostime;
   struct dostime_t cmostime;
#if (DEVELOPMENT == 1)
   int totaltime;
#endif

   if (TimerStarted==false)
      return;
   TimerStarted=false;

//      OUTP(0x43,0x36);                            // Change timer 0
//      OUTP(0x40,0);
//      OUTP(0x40,0);
//      _dos_setvect (TIMERINT, oldtimerisr);

   I_GetCMOSTime ( &cmostime );
   _dos_gettime  ( &dostime  );

#if (DEVELOPMENT == 1)
   SoftError("Time difference in seconds (DOS-CMOS) %ld\n",dostime.second-cmostime.second);
   SoftError("Time difference in minutes (DOS-CMOS) %ld\n",dostime.minute-cmostime.minute);
   SoftError("Time difference in hour (DOS-CMOS) %ld\n",dostime.hour-cmostime.hour);
   totaltime=( ((cmostime.hour-starttime.hour)*3600) +
               ((cmostime.minute-starttime.minute)*60) +
                (cmostime.second-starttime.second)
             );
   SoftError("Total seconds = %ld Total Tics = %ld Game Tics = %ld\n",totaltime,totaltime*VBLCOUNTER,ticcount);
#endif

   TS_Terminate( timertask );
#if (DEVELOPMENT == 1)
   TS_Terminate( fasttimertask );
#endif
   TS_Shutdown();
//   TS_Halt();
/*
   if (oldtimerisr)
      {
      OUTP(0x43,0x36);                            // Change timer 0
      OUTP(0x40,0);
      OUTP(0x40,0);
      _dos_setvect (TIMERINT, oldtimerisr);
      // Set Date and Time from CMOS

      OUTP(0x70,0);
      time.second=inp(0x71);
      OUTP(0x70,2);
      time.minute=inp(0x71);
      OUTP(0x70,4);
      time.hour=inp(0x71);
      time.second=(time.second&0x0f)+((time.second>>4)*10);
      time.minute=(time.minute&0x0f)+((time.minute>>4)*10);
      time.hour=(time.hour&0x0f)+((time.hour>>4)*10);
      _dos_settime(&time);

      OUTP(0x70,7);
      date.day=inp(0x71);
      OUTP(0x70,8);
      date.month=inp(0x71);
      OUTP(0x70,9);
      date.year=inp(0x71);
      date.day=(date.day&0x0f)+((date.day>>4)*10);
      date.month=(date.month&0x0f)+((date.month>>4)*10);
      date.year=(date.year&0x0f)+((date.year>>4)*10);
      _dos_setdate(&date);
//      }
*/
#endif
}

/*
===============
=
= I_GetCMOSTime
=
===============
*/
void I_GetCMOSTime ( struct dostime_t * cmostime )
{
   OUTP(0x70,0);
   cmostime->second=inp(0x71);
   OUTP(0x70,2);
   cmostime->minute=inp(0x71);
   OUTP(0x70,4);
   cmostime->hour=inp(0x71);
   cmostime->second=(cmostime->second&0x0f)+((cmostime->second>>4)*10);
   cmostime->minute=(cmostime->minute&0x0f)+((cmostime->minute>>4)*10);
   cmostime->hour=(cmostime->hour&0x0f)+((cmostime->hour>>4)*10);
}

/*
============================================================================

																KEYBOARD

============================================================================
*/
#define ena_kbd           0xae
#define caps_state        0x40
#define num_state         0x20
#define scroll_state      0x10

#define kb_resend         0xfe
#define kb_ack            0xfa
#define kb_pr_led         0x40
#define kb_error          0x80
#define kb_fe             0x20
#define kb_fa             0x10

#define porta             0x60
#define kb_status_port    0x64
#define input_buffer_full 0x02
#define led_cmd           0xed
#define kb_enable         0xf4
#define leds_off          0
#define caps_led_on       0x04
#define num_led_on        0x02
#define scroll_led_on     0x01

/*
================
=
= I_SendKeyboardData
=
================
*/

void I_SendKeyboardData
   (
   int val
   )

   {
   int retry;
   volatile int count;

   _disable();

   KBFlags &= ~( kb_fe | kb_fa );

   count = 0xffff;
   while( count-- )
      {
      if ( !( inp( kb_status_port ) & input_buffer_full ) )
         {
         break;
         }
      }

   outp( porta, val );

   _enable();

   retry = 3;
   while( retry-- )
      {
      count = 0x1a00;
      while( count-- )
         {
         if ( KBFlags & kb_fe )
            {
            break;
            }

         if ( KBFlags & kb_fa )
            {
            return;
            }
         }
      }

   KBFlags |= kb_error;
   }

/*
================
=
= I_SetKeyboardLEDs
=
================
*/

void I_SetKeyboardLEDs
   (
   int which,
   boolean val
   )

   {
   int mask;
   int count;

   _disable();
   KBFlags |= kb_pr_led;

   switch( which )
      {
      case scroll_lock :
         mask = scroll_led_on;
         break;

      case num_lock :
         mask = num_led_on;
         break;

      case caps_lock :
         mask = caps_led_on;
         break;

      default :
         mask = 0;
         break;
      }

   if ( val )
      {
      LEDs |= mask;
      }
   else
      {
      LEDs &= ~mask;
      }

   count = 0;
   do
      {
      if ( count > 3 )
         {
         break;
         }

      I_SendKeyboardData( led_cmd );
      _disable();
      I_SendKeyboardData( LEDs );
      _disable();
      I_SendKeyboardData( kb_enable );
      _disable();
      count++;
      }
   while( KBFlags & kb_error );

   _enable();
   KBFlags &= ~(kb_pr_led|kb_error);
   }


/*
================
=
= I_KeyboardISR
=
================
*/
void __interrupt I_KeyboardISR (void)
{
	int k;
   int temp;
   int keyon;
   int strippedkey;

   // Get the scan code
   k = inp( 0x60 );

   // Tell the XT keyboard controller to clear the key
	temp = inp( 0x61 );
	OUTP ( 0x61, temp | 0x80 );
	OUTP ( 0x61, temp );

   if ( KBFlags & kb_pr_led )
      {
      if ( k == kb_resend )
         {
         // Handle resend
         KBFlags |= kb_fe;
         }
      else if (k == kb_ack)
         {
         // Handle ack
         KBFlags |= kb_fa;
         }
      }
   else if ( pausecount )
      {
      pausecount--;
      }
   else if ( k == 0xe1 )         // Handle Pause key
      {
      PausePressed = true;
      pausecount = 5;
      }
   else if ( k == 0x46 )         // Handle Panic key (Scroll Lock)
      {
      PanicPressed = true;
      }
   else
      {
      if ( k == 0xE0 )
         {
         ExtendedKeyFlag = true;
         }
      else
         {
         keyon = k & 0x80;
         strippedkey = k & 0x7f;

         if ( ExtendedKeyFlag )
            {
            if ( ( strippedkey == sc_LShift ) ||
               ( strippedkey == sc_RShift ) )
               {
               k = sc_None;
               }
/*
            else
               {
               if ( strippedkey == sc_Alt )
                  {
                  k = sc_RightAlt | keyon;
                  }
               if ( strippedkey == sc_Control )
                  {
                  k = sc_RightCtrl | keyon;
                  }
               }
*/
            }

         if ( k != sc_None )
            {
            if ( strippedkey == sc_LShift )
               {
               k = sc_RShift | keyon;
               }

            if ( !keyon )
               {
               LastScan = k;
               }

            if (k & 0x80)        // Up event
               {
               Keystate[k&0x7f]=0;
               }
            else                 // Down event
               {
               Keystate[k]=1;
               }

            KeyboardQueue[ Keytail ] = k;
            Keytail = ( Keytail + 1 )&( KEYQMAX - 1 );
            }

         ExtendedKeyFlag = false;
         }
      }

   // acknowledge the interrupt
   OUTP ( 0x20, 0x20 );
   }


/*
===============
=
= I_StartupKeyboard
=
===============
*/

void I_StartupKeyboard (void)
{
   if (KeyboardStarted==true)
      return;
   KeyboardStarted=true;

   LEDs = 0;
   KBFlags = 0;
   ExtendedKeyFlag = false;

	oldkeyboardisr = _dos_getvect(KEYBOARDINT);
	_dos_setvect (0x8000 | KEYBOARDINT, I_KeyboardISR);

//   I_SetKeyboardLEDs( scroll_lock, 0 );

   Keyhead = Keytail = 0;
   memset((void *)Keystate, 0, MAXKEYBOARDSCAN * sizeof(int));
   if (!quiet)
      printf("I_StartupKeyboard: Keyboard Started\n");
}

void I_ShutdownKeyboard (void)
{
   if (KeyboardStarted==false)
      return;
   KeyboardStarted=false;

   // Clear LEDS
//   *( (byte *)0x417 ) &= ~0x70;

	_dos_setvect (KEYBOARDINT, oldkeyboardisr);
	*(short *)0x41c = *(short *)0x41a;      // clear bios key buffer
}
#else

#include "SDL.h"

static int ticoffset;    /* offset for SDL_GetTicks() */
static int ticbase;      /* game-supplied base */

int GetTicCount (void)
{
	return ((SDL_GetTicks() - ticoffset) * VBLCOUNTER) / 1000 + ticbase;
}

/*
================
=
= ISR_SetTime
=
================
*/
void ISR_SetTime(int settime)
{
	ticoffset = SDL_GetTicks();
	ticbase = settime;
}

/* developer-only */

int GetFastTics (void)
{
	/* STUB_FUNCTION; */
	
	return 0;
}

void SetFastTics (int settime)
{
	/* STUB_FUNCTION; */
}

/*
================
=
= I_Delay
=
================
*/

void I_Delay ( int delay )
{
   int time;

   delay=(VBLCOUNTER*delay)/10;
   IN_ClearKeysDown();
   time=GetTicCount();
   while (!LastScan && !IN_GetMouseButtons() && GetTicCount()<time+delay)
      {
      	IN_UpdateKeyboard();
      }
}

/*
===============
=
= I_StartupTimer
=
===============
*/

void I_StartupTimer (void)
{
}

void I_ShutdownTimer (void)
{
}

/*
===============
=
= I_StartupKeyboard
=
===============
*/

void I_StartupKeyboard (void)
{
}


void I_ShutdownKeyboard (void)
{
}
#endif
