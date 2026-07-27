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
/**********************************************************************
   module: TASK_MAN.C

   author: James R. Dose
   date:   July 25, 1994

   Low level timer task scheduler.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#define TRUE  ( 1 == 1 )
#define FALSE ( !TRUE )

//#define USESTACK
#define LOCKMEMORY
#define NOINTS
#define USE_USRHOOKS

#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
#include "interrup.h"
#include "linklist.h"
#include "task_man.h"

#ifdef USESTACK
#include "dpmi.h"
#endif
#ifdef LOCKMEMORY
#include "dpmi.h"
#endif

#ifdef USE_USRHOOKS
#include "usrhooks.h"
#define FreeMem( ptr )   USRHOOKS_FreeMem( ( ptr ) )
#else
#define FreeMem( ptr )   free( ( ptr ) )
#endif

typedef struct
   {
   task *start;
   task *end;
   } tasklist;


/*---------------------------------------------------------------------
   Global variables
---------------------------------------------------------------------*/

#ifdef USESTACK

// adequate stack size
#define kStackSize 2048

static unsigned short StackSelector = NULL;
static unsigned long  StackPointer;

static unsigned short oldStackSelector;
static unsigned long  oldStackPointer;

#endif

static task HeadTask;
static task *TaskList = &HeadTask;

static void ( __interrupt __far *OldInt8 )( void );

static volatile long TaskServiceRate  = 0x10000L;
static volatile long TaskServiceCount = 0;

#ifndef NOINTS
static volatile int  TS_TimesInInterrupt;
#endif

static char TS_Installed = FALSE;

volatile int TS_InInterrupt = FALSE;

/*---------------------------------------------------------------------
   Function prototypes
---------------------------------------------------------------------*/

static void TS_FreeTaskList( void );
static void TS_SetClockSpeed( long speed );
static long TS_SetTimer( long TickBase );
static void TS_SetTimerToMaxTaskRate( void );
static void __interrupt __far TS_ServiceSchedule( void );
static void __interrupt __far TS_ServiceScheduleIntEnabled( void );
static void TS_AddTask( task *ptr );
static int  TS_Startup( void );
static void RestoreRealTimeClock( void );

// These declarations are necessary to use the inline assembly pragmas.

extern void GetStack(unsigned short *selptr,unsigned long *stackptr);
extern void SetStack(unsigned short selector,unsigned long stackptr);

// This function will get the current stack selector and pointer and save
// them off.
#pragma aux GetStack =	\
	"mov  [edi],esp"		\
	"mov	ax,ss"	 		\
	"mov  [esi],ax" 		\
	parm [esi] [edi]		\
	modify [eax esi edi];

// This function will set the stack selector and pointer to the specified
// values.
#pragma aux SetStack =	\
	"mov  ss,ax"			\
	"mov  esp,edx"			\
	parm [ax] [edx]		\
	modify [eax edx];


/**********************************************************************

   Memory locked functions:

**********************************************************************/


#define TS_LockStart TS_FreeTaskList


/*---------------------------------------------------------------------
   Function: TS_FreeTaskList

   Terminates all tasks and releases any memory used for control
   structures.
---------------------------------------------------------------------*/

static void TS_FreeTaskList
   (
   void
   )

   {
   task *node;
   task *next;
   unsigned flags;

   flags = DisableInterrupts();

   node = TaskList->next;
   while( node != TaskList )
      {
      next = node->next;
      FreeMem( node );
      node = next;
      }

   TaskList->next = TaskList;
   TaskList->prev = TaskList;

   RestoreInterrupts( flags );
   }


/*---------------------------------------------------------------------
   Function: TS_SetClockSpeed

   Sets the rate of the 8253 timer.
---------------------------------------------------------------------*/

static void TS_SetClockSpeed
   (
   long speed
   )

   {
   unsigned flags;

   flags = DisableInterrupts();

   if ( ( speed > 0 ) && ( speed < 0x10000L ) )
      {
      TaskServiceRate = speed;
      }
   else
      {
      TaskServiceRate = 0x10000L;
      }

   outp( 0x43, 0x36 );
   outp( 0x40, TaskServiceRate );
   outp( 0x40, TaskServiceRate >> 8 );

   RestoreInterrupts( flags );
   }


/*---------------------------------------------------------------------
   Function: TS_SetTimer

   Calculates the rate at which a task will occur and sets the clock
   speed if necessary.
---------------------------------------------------------------------*/

static long TS_SetTimer
   (
   long TickBase
   )

   {
   long speed;

   speed = 1192030L / TickBase;
   if ( speed < TaskServiceRate )
      {
      TS_SetClockSpeed( speed );
      }

   return( speed );
   }


/*---------------------------------------------------------------------
   Function: TS_SetTimerToMaxTaskRate

   Finds the fastest running task and sets the clock to operate at
   that speed.
---------------------------------------------------------------------*/

static void TS_SetTimerToMaxTaskRate
   (
   void
   )

   {
   task     *ptr;
   long      MaxServiceRate;
   unsigned  flags;

   flags = DisableInterrupts();

   MaxServiceRate = 0x10000L;

   ptr = TaskList->next;
   while( ptr != TaskList )
      {
      if ( ptr->rate < MaxServiceRate )
         {
         MaxServiceRate = ptr->rate;
         }

      ptr = ptr->next;
      }

   if ( TaskServiceRate != MaxServiceRate )
      {
      TS_SetClockSpeed( MaxServiceRate );
      }

   RestoreInterrupts( flags );
   }


#ifdef NOINTS
/*---------------------------------------------------------------------
   Function: TS_ServiceSchedule

   Interrupt service routine
---------------------------------------------------------------------*/

static void __interrupt __far TS_ServiceSchedule
   (
   void
   )

   {
   task *ptr;
   task *next;


   TS_InInterrupt = TRUE;

   #ifdef USESTACK
   // save stack
   GetStack( &oldStackSelector, &oldStackPointer );

   // set our stack
   SetStack( StackSelector, StackPointer );
   #endif

   ptr = TaskList->next;
   while( ptr != TaskList )
      {
      next = ptr->next;

      if ( ptr->active )
         {
         ptr->count += TaskServiceRate;
//JIM
//         if ( ptr->count >= ptr->rate )
         while( ptr->count >= ptr->rate )
            {
            ptr->count -= ptr->rate;
            ptr->TaskService( ptr );
            }
         }
      ptr = next;
      }

   #ifdef USESTACK
   // restore stack
   SetStack( oldStackSelector, oldStackPointer );
   #endif

   TaskServiceCount += TaskServiceRate;
   if ( TaskServiceCount > 0xffffL )
      {
      TaskServiceCount &= 0xffff;
      _chain_intr( OldInt8 );
      }

   outp( 0x20,0x20 );

   TS_InInterrupt = FALSE;
   }

#else

/*---------------------------------------------------------------------
   Function: TS_ServiceScheduleIntEnabled

   Interrupt service routine with interrupts enabled.
---------------------------------------------------------------------*/

static void __interrupt __far TS_ServiceScheduleIntEnabled
   (
   void
   )

   {
   task *ptr;
   task *next;

   TS_TimesInInterrupt++;
   TaskServiceCount += TaskServiceRate;
   if ( TaskServiceCount > 0xffffL )
      {
      TaskServiceCount &= 0xffff;
      _chain_intr( OldInt8 );
      }

   outp( 0x20,0x20 );

   if ( TS_InInterrupt )
      {
      return;
      }

   TS_InInterrupt = TRUE;
   _enable();

   #ifdef USESTACK
   // save stack
   GetStack( &oldStackSelector, &oldStackPointer );

   // set our stack
   SetStack( StackSelector, StackPointer );
   #endif

   while( TS_TimesInInterrupt )
      {
      ptr = TaskList->next ;
      while( ptr != TaskList )
         {
         next = ptr->next;

         if ( ptr->active )
            {
            ptr->count += TaskServiceRate;
            if ( ptr->count >= ptr->rate )
               {
               ptr->count -= ptr->rate;
               ptr->TaskService( ptr );
               }
            }
         ptr = next;
         }
      TS_TimesInInterrupt--;
      }

   _disable();

   #ifdef USESTACK
   // restore stack
   SetStack( oldStackSelector, oldStackPointer );
   #endif

   TS_InInterrupt = FALSE;
   }
#endif


#ifdef USESTACK

/*---------------------------------------------------------------------
   Function: allocateTimerStack

   Allocate a block of memory from conventional (low) memory and return
   the selector (which can go directly into a segment register) of the
   memory block or 0 if an error occured.
---------------------------------------------------------------------*/

static unsigned short allocateTimerStack
   (
   unsigned short size
   )

   {
   union REGS regs;

   // clear all registers
   memset( &regs, 0, sizeof( regs ) );

   // DPMI allocate conventional memory
   regs.w.ax = 0x100;

   // size in paragraphs
   regs.w.bx = ( size + 15 ) / 16;

   int386( 0x31, &regs, &regs );
   if (!regs.w.cflag)
      {
      // DPMI call returns selector in dx
      // (ax contains real mode segment
      // which is ignored here)

      return( regs.w.dx );
      }

   // Couldn't allocate memory.
   return( NULL );
   }


/*---------------------------------------------------------------------
   Function: deallocateTimerStack

   Deallocate a block of conventional (low) memory given a selector to
   it.  Assumes the block was allocated with DPMI function 0x100.
---------------------------------------------------------------------*/

static void deallocateTimerStack
   (
   unsigned short selector
   )

   {
	union REGS regs;

	if ( selector != NULL )
      {
      // clear all registers
      memset( &regs, 0, sizeof( regs ) );

      regs.w.ax = 0x101;
      regs.w.dx = selector;
      int386( 0x31, &regs, &regs );
      }
   }

#endif

/*---------------------------------------------------------------------
   Function: TS_Startup

   Sets up the task service routine.
---------------------------------------------------------------------*/

static int TS_Startup
   (
   void
   )

   {
   if ( !TS_Installed )
      {
#ifdef LOCKMEMORY

      int status;

      status = TS_LockMemory();
      if ( status != TASK_Ok )
         {
         TS_UnlockMemory();
         return( status );
         }

#endif

#ifdef USESTACK

	   StackSelector = allocateTimerStack( kStackSize );
      if ( StackSelector == NULL )
         {

#ifdef LOCKMEMORY

         TS_UnlockMemory();

#endif
         return( TASK_Error );
         }

      // Leave a little room at top of stack just for the hell of it...
      StackPointer = kStackSize - sizeof( long );

#endif

//static const task *TaskList = &HeadTask;
      TaskList->next = TaskList;
      TaskList->prev = TaskList;

      TaskServiceRate  = 0x10000L;
      TaskServiceCount = 0;

#ifndef NOINTS
      TS_TimesInInterrupt = 0;
#endif

      OldInt8 = _dos_getvect( 0x08 );
      #ifdef NOINTS
         _dos_setvect( 0x08, TS_ServiceSchedule );
      #else
         _dos_setvect( 0x08, TS_ServiceScheduleIntEnabled );
      #endif

      TS_Installed = TRUE;
      }

   return( TASK_Ok );
   }


/*---------------------------------------------------------------------
   Function: TS_Shutdown

   Ends processing of all tasks.
---------------------------------------------------------------------*/

void TS_Shutdown
   (
   void
   )

   {
   if ( TS_Installed )
      {
      TS_FreeTaskList();

      TS_SetClockSpeed( 0 );

      _dos_setvect( 0x08, OldInt8 );

#ifdef USESTACK

      deallocateTimerStack( StackSelector );
      StackSelector = NULL;

#endif

      // Set Date and Time from CMOS
//      RestoreRealTimeClock();

#ifdef LOCKMEMORY

      TS_UnlockMemory();

#endif
      TS_Installed = FALSE;
      }
   }


/*---------------------------------------------------------------------
   Function: TS_ScheduleTask

   Schedules a new task for processing.
---------------------------------------------------------------------*/

task *TS_ScheduleTask
   (
   void  ( *Function )( task * ),
   int   rate,
   int   priority,
   void *data
   )

   {
   task *ptr;

#ifdef USE_USRHOOKS
   int   status;

   ptr = NULL;

   status = USRHOOKS_GetMem( &ptr, sizeof( task ) );
   if ( status == USRHOOKS_Ok )
#else
   ptr = malloc( sizeof( task ) );
   if ( ptr != NULL )
#endif
      {
      if ( !TS_Installed )
         {
         status = TS_Startup();
         if ( status != TASK_Ok )
            {
            FreeMem( ptr );
            return( NULL );
            }
         }

      ptr->TaskService = Function;
      ptr->data = data;
      ptr->rate = TS_SetTimer( rate );
      ptr->count = 0;
      ptr->priority = priority;
      ptr->active = FALSE;

      TS_AddTask( ptr );
      }

   return( ptr );
   }


/*---------------------------------------------------------------------
   Function: TS_AddTask

   Adds a new task to our list of tasks.
---------------------------------------------------------------------*/

static void TS_AddTask
   (
   task *node
   )

   {
   LL_SortedInsertion( TaskList, node, next, prev, task, priority );
   }


/*---------------------------------------------------------------------
   Function: TS_Terminate

   Ends processing of a specific task.
---------------------------------------------------------------------*/

int TS_Terminate
   (
   task *NodeToRemove
   )

   {
   task *ptr;
   task *next;
   unsigned flags;

   flags = DisableInterrupts();

   ptr = TaskList->next;
   while( ptr != TaskList )
      {
      next = ptr->next;

      if ( ptr == NodeToRemove )
         {
         LL_RemoveNode( NodeToRemove, next, prev );
         NodeToRemove->next = NULL;
         NodeToRemove->prev = NULL;
         FreeMem( NodeToRemove );

         TS_SetTimerToMaxTaskRate();

         RestoreInterrupts( flags );

         return( TASK_Ok );
         }

      ptr = next;
      }

   RestoreInterrupts( flags );

   return( TASK_Warning );
   }


/*---------------------------------------------------------------------
   Function: TS_Dispatch

   Begins processing of all inactive tasks.
---------------------------------------------------------------------*/

void TS_Dispatch
   (
   void
   )

   {
   task *ptr;
   unsigned flags;

   flags = DisableInterrupts();

   ptr = TaskList->next;
   while( ptr != TaskList )
      {
      ptr->active = TRUE;
      ptr = ptr->next;
      }

   RestoreInterrupts( flags );
   }


/*---------------------------------------------------------------------
   Function: TS_SetTaskRate

   Sets the rate at which the specified task is serviced.
---------------------------------------------------------------------*/

void TS_SetTaskRate
   (
   task *Task,
   int rate
   )

   {
   unsigned flags;

   flags = DisableInterrupts();

   Task->rate = TS_SetTimer( rate );
   TS_SetTimerToMaxTaskRate();

   RestoreInterrupts( flags );
   }


#ifdef LOCKMEMORY

/*---------------------------------------------------------------------
   Function: TS_LockEnd

   Used for determining the length of the functions to lock in memory.
---------------------------------------------------------------------*/

static void TS_LockEnd
   (
   void
   )

   {
   }


/*---------------------------------------------------------------------
   Function: TS_UnlockMemory

   Unlocks all neccessary data.
---------------------------------------------------------------------*/

void TS_UnlockMemory
   (
   void
   )

   {
   DPMI_UnlockMemoryRegion( TS_LockStart, TS_LockEnd );
   DPMI_Unlock( TaskList );
   DPMI_Unlock( OldInt8 );
   DPMI_Unlock( TaskServiceRate );
   DPMI_Unlock( TaskServiceCount );
   DPMI_Unlock( TS_Installed );

#ifndef NOINTS
   DPMI_Unlock( TS_TimesInInterrupt );
#endif

#ifdef USESTACK
   DPMI_Unlock( StackSelector );
   DPMI_Unlock( StackPointer );
   DPMI_Unlock( oldStackSelector );
   DPMI_Unlock( oldStackPointer );
#endif
   }


/*---------------------------------------------------------------------
   Function: TS_LockMemory

   Locks all neccessary data.
---------------------------------------------------------------------*/

int TS_LockMemory
   (
   void
   )

   {
   int status;

   status  = DPMI_LockMemoryRegion( TS_LockStart, TS_LockEnd );
   status |= DPMI_Lock( TaskList );
   status |= DPMI_Lock( OldInt8 );
   status |= DPMI_Lock( TaskServiceRate );
   status |= DPMI_Lock( TaskServiceCount );
   status |= DPMI_Lock( TS_Installed );

#ifndef NOINTS
   status |= DPMI_Lock( TS_TimesInInterrupt );
#endif

#ifdef USESTACK
   status |= DPMI_Lock( StackSelector );
   status |= DPMI_Lock( StackPointer );
   status |= DPMI_Lock( oldStackSelector );
   status |= DPMI_Lock( oldStackPointer );
#endif

   if ( status != DPMI_Ok )
      {
      TS_UnlockMemory();
      return( TASK_Error );
      }

   return( TASK_Ok );
   }

#endif

/*
// Converts a hex byte to an integer

static int btoi
   (
   unsigned char bcd
   )

   {
   unsigned b;
   unsigned c;
   unsigned d;

   b = bcd / 16;
   c = bcd - b * 16;
   d = b * 10 + c;
   return( d );
   }


static void RestoreRealTimeClock
   (
   void
   )

   {
   int read;
   int i;
   int hr;
   int min;
   int sec;
   int cent;
   int yr;
   int mo;
   int day;
   int year;
   union REGS inregs;

   // Read Real Time Clock Time.
   read = FALSE;
	inregs.h.ah = 0x02;
   for( i = 1; i <= 3; i++ )
      {
      int386( 0x1A, &inregs, &inregs );
      if ( inregs.x.cflag == 0 )
         {
         read = TRUE;
         }
      }

   if ( read )
      {
      //and convert BCD to integer format
      hr  = btoi( inregs.h.ch );
      min = btoi( inregs.h.cl );
      sec = btoi( inregs.h.dh );

      // Read Real Time Clock Date.
      inregs.h.ah = 0x04;
      int386( 0x1A, &inregs, &inregs );
      if ( inregs.x.cflag == 0 )
         {
         //and convert BCD to integer format
         cent = btoi( inregs.h.ch );
         yr   = btoi( inregs.h.cl );
         mo   = btoi( inregs.h.dh );
         day  = btoi( inregs.h.dl );
         year = cent * 100 + yr;

         // Set System Time.
         inregs.h.ch = hr;
         inregs.h.cl = min;
         inregs.h.dh = sec;
         inregs.h.dl = 0;
         inregs.h.ah = 0x2D;
         int386( 0x21, &inregs, &inregs );

         // Set System Date.
         inregs.w.cx = year;
         inregs.h.dh = mo;
         inregs.h.dl = day;
         inregs.h.ah = 0x2B;
         int386( 0x21, &inregs, &inregs );
         }
      }
   }
*/
/*
   struct dostime_t time;
   struct dosdate_t date;

   outp(0x70,0);
   time.second=inp(0x71);
   outp(0x70,2);
   time.minute=inp(0x71);
   outp(0x70,4);
   time.hour=inp(0x71);

   outp(0x70,7);
   date.day=inp(0x71);
   outp(0x70,8);
   date.month=inp(0x71);
   outp(0x70,9);
   date.year=inp(0x71);

   time.second=(time.second&0x0f)+((time.second>>4)*10);
   time.minute=(time.minute&0x0f)+((time.minute>>4)*10);
   time.hour=(time.hour&0x0f)+((time.hour>>4)*10);

   date.day=(date.day&0x0f)+((date.day>>4)*10);
   date.month=(date.month&0x0f)+((date.month>>4)*10);
   date.year=(date.year&0x0f)+((date.year>>4)*10);

   _dos_settime(&time);
   _dos_setdate(&date);

*/
