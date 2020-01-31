/*
** Nofrendo (c) 1998-2000 Matthew Conte (matt@conte.com)
**
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of version 2 of the GNU Library General 
** Public License as published by the Free Software Foundation.
**
** This program is distributed in the hope that it will be useful, 
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
** Library General Public License for more details.  To obtain a 
** copy of the GNU Library General Public License, write to the Free 
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
**
**
** log.c
**
** Error logging functions
** $Id: log.c,v 1.2 2001/04/27 14:37:11 neil Exp $
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <noftypes.h>
#include <log.h>


//static FILE *errorlog = NULL;
static int (*log_func)(const char *string) = NULL;

/* first up: debug versions of calls */
#ifdef NOFRENDO_DEBUG
int log_init(void)
{
//   errorlog = fopen("errorlog.txt", "wt");
//   if (NULL == errorlog)
//      return (-1);

   return 0;
}

void log_shutdown(void)
{
   /* Snoop around for unallocated blocks */
   mem_checkblocks();
   mem_checkleaks();
   mem_cleanup();

//   if (NULL != errorlog)
//      fclose(errorlog);
}

int log_print(const char *string)
{
   /* if we have a custom logging function, use that */
   if (NULL != log_func)
      log_func(string);
   
   /* Log it to disk, as well */
//   fputs(string, errorlog);
//	printf("%s\n", string);

   return 0;
}

int log_printf(const char *format, ... )
{
   /* don't allocate on stack every call */
   static char buffer[1024 + 1];
   va_list arg;

   va_start(arg, format);

   if (NULL != log_func)
   {
      vsprintf(buffer, format, arg);
      log_func(buffer);
   }

//   vfprintf(errorlog, format, arg);
   va_end(arg);

   return 0; /* should be number of chars written */
}

#else /* !NOFRENDO_DEBUG */

int log_init(void)
{
   return 0;
}

void log_shutdown(void)
{
}

int log_print(const char *string)
{
   UNUSED(string);

   return 0;
}

int log_printf(const char *format, ... )
{
   UNUSED(format);

   return 0; /* should be number of chars written */
}
#endif /* !NOFRENDO_DEBUG */

void log_chain_logfunc(int (*func)(const char *string))
{
   log_func = func;
}

void log_assert(int expr, int line, const char *file, char *msg)
{
   if (expr)
      return;

   if (NULL != msg)
      log_printf("ASSERT: line %d of %s, %s\n", line, file, msg);
   else
      log_printf("ASSERT: line %d of %s\n", line, file);

   asm("break.n 1");
//   exit(-1);
}


/*
** $Log: log.c,v $
** Revision 1.2  2001/04/27 14:37:11  neil
** wheeee
**
** Revision 1.1.1.1  2001/04/27 07:03:54  neil
** initial
**
** Revision 1.14  2000/11/13 00:56:17  matt
** doesn't look as nasty now
**
** Revision 1.13  2000/11/06 02:15:07  matt
** more robust logging routines
**
** Revision 1.12  2000/10/15 13:28:12  matt
** need stdlib.h for exit()
**
** Revision 1.11  2000/10/10 13:13:13  matt
** dumb bug in log_chain_logfunc
**
** Revision 1.10  2000/10/10 13:03:54  matt
** Mr. Clean makes a guest appearance
**
** Revision 1.9  2000/08/28 01:47:10  matt
** quelled fricking compiler complaints
**
** Revision 1.8  2000/07/31 04:28:46  matt
** one million cleanups
**
** Revision 1.7  2000/07/17 01:52:27  matt
** made sure last line of all source files is a newline
**
** Revision 1.6  2000/07/06 17:20:52  matt
** block manager space itself wasn't being freed - d'oh!
**
** Revision 1.5  2000/06/26 04:55:33  matt
** minor change
**
** Revision 1.4  2000/06/09 15:12:25  matt
** initial revision
**
*/
