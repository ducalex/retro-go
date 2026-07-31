#include "rt_def.h"
#include <stdlib.h>
#include <string.h>

#include "interrup.h"
#include "linklist.h"
#include "task_man.h"

#ifdef ESP_PLATFORM
#define __far
#define __interrupt
#define TS_Ok 0
#endif

#ifndef ESP_PLATFORM
static void ( __interrupt __far *OldInt8 )( void );
#endif

static int TS_Installed = 0;

void TS_SetClockSpeed( unsigned short speed )
{
#ifndef ESP_PLATFORM
   outp( 0x43, 0x36 );
   outp( 0x40, speed & 0xff );
   outp( 0x40, speed >> 8 );
#endif
}

#ifndef ESP_PLATFORM
static void __interrupt __far TS_ServiceSchedule( void )
{
}
#endif

int TS_Startup( void )
{
   if ( TS_Installed ) return TS_Ok;
   TS_Installed = 1;
   return TS_Ok;
}

void TS_Shutdown( void )
{
   if ( !TS_Installed ) return;
   TS_Installed = 0;
}

void TS_AddQueuedTask( task *task ) {}
void TS_AddFixedTask( task *task ) {}
void TS_RemoveTask( task *task ) {}

#ifndef ESP_PLATFORM
int TS_LockMemory( void ) { return 0; }
int TS_UnlockMemory( void ) { return 0; }
#endif
