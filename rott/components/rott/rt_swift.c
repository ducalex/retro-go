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
//****************************************************************************
//
// RT_SWIFT.C
//
// SWIFT services module - for CYBERMAN use in ROTT.
//
//****************************************************************************


#include "rt_def.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DOS
#include <dos.h>
#endif

#include "rt_swift.h"
#include "_rt_swft.h"
//MED
#include "memcheck.h"


#ifdef DOS

//****************************************************************************
//
// SWIFT_Initialize ()
//
// Test for presence of SWIFT extensions and SWIFT device.
// Returns 1 (TRUE) if SWIFT features are available, 0 otherwise.
// Remember to call SWIFT_Terminate() if SWIFT_Initialize succeeds!
//
//****************************************************************************

int SWIFT_Initialize (void)
{
   SWIFT_StaticData sdBuf;
   int fSwift = 0;

   if (fActive)                     // SWIFT extensions already active
   {
#ifdef DEGUB
      SoftError( "SWIFT_Initialize: Already active.\n");
      SoftError( "SWIFT_Initialize: returns TRUE\n");
#endif
      return (1);
   }

   nAttached = SWIFT_DEV_NONE;


   if (_dos_getvect(0x33) == NULL)  // No mouse driver loaded
   {
#ifdef DBUG
      SoftError( "SWIFT_Initialize: No mouse driver loaded.\n");
      SoftError( "SWIFT_Initialize: returns FALSE\n");
#endif
      return (0);
   }


   // Reset the mouse and driver
   AX (regs) = 0;
   int386( 0x33, &regs, &regs);

   if (AX (regs) == 0)
   {                                // no mouse
#ifdef DBUG
      SoftError( "SWIFT_Initialize: No pointing device attached.\n");
      SoftError( "SWIFT_Initialize: returns FALSE\n");
#endif
      return (0);
   }

#ifdef DBUG
   AX (regs) = 36;   // Get Mouse Information
   BX (regs) = 0xffff;
   CX (regs) = 0xffff;
   DX (regs) = 0xffff;
   int386 (0x33, &regs, &regs);
   SoftError( "SWIFT_Initialize: driver version %d.%02d\n", regs.h.bh, regs.h.bl);
   SoftError( "SWIFT_Initialize: %s mouse using IRQ %d\n",
          (regs.h.ch==1) ? "bus" :
          (regs.h.ch==2) ? "serial" :
          (regs.h.ch==3) ? "inport" :
          (regs.h.ch==4) ? "PS/2" :
                           "unknown", regs.h.cl);
#endif


   // allocate a DOS real-mode buffer
   pdosmem = allocDOS(DOSMEMSIZE, &segment, &selector);
   if (!pdosmem)
   {
#ifdef DBUG
      SoftError( "SWIFT_Initialize: DOS Alloc failed!\n");
      SoftError( "SWIFT_Initialize: returns FALSE\n");
#endif
      return (0);
   }

//
// SWIFT device supported and attached
//
   if (SWIFT_GetStaticDeviceInfo (&sdBuf))
      fSwift = 1;


   if (!fSwift)
   {  // SWIFT functions not present
#ifdef DBUG
      SoftError( "SWIFT_Initialize: no SWIFT support in mouse driver.\n");
#endif
   }
   else
      if (sdBuf.deviceType == SWIFT_DEV_NONE)
      {
#ifdef DBUG
         SoftError( "SWIFT_Initialize: no SWIFT device connected.\n");
#endif
      }
      else
      {
         nAttached = sdBuf.deviceType;
#ifdef DBUG
         SoftError( "SWIFT_Initialize: ");

         switch (nAttached)
         {
            case SWIFT_DEV_CYBERMAN:
               SoftError( "CyberMan %d.%02d connected.\n",
                                sdBuf.majorVersion, sdBuf.minorVersion);
            break;

            default:
               SoftError( "Unknown SWIFT device (type %d) connected.\n",
                                nAttached);
            break;
         }
#endif
      fActive = 1;
   }

   if (!fActive)
   {                    // activation of SWIFT module failed for some reason
      if (pdosmem)
      {                 // if DOS buffer was allocated, free it
         freeDOS(selector);
         pdosmem = 0;
      }
   }

#ifdef DBUG
   SoftError( "SWIFT_Initialize: returns %s.\n", (fActive ? "TRUE" : "FALSE"));
#endif
   return fActive;
}



//****************************************************************************
//
// SWIFT_Terminate ()
//
// Free resources required for SWIFT support.  If SWIFT_Initialize has
// not been called, or returned FALSE, this function does nothing.
// SWIFT_Terminate should always be called at some time after a call to
// SWIFT_Initialize has returned TRUE.
//
//****************************************************************************

void SWIFT_Terminate (void)
{
#ifdef DBUG
   SoftError( "SWIFT_Terminate called.\n");
#endif

   if (fActive)
   {
     // free DOS buffer
      if (pdosmem)
      {
         freeDOS(selector);
         pdosmem = 0;
      }

      fActive = 0;
   }
}


//****************************************************************************
//
// SWIFT_GetAttachedDevice ()
//
// Returns the device-type code for the attached SWIFT device, if any.
//
//****************************************************************************

int SWIFT_GetAttachedDevice (void)
{
   return (nAttached);
}



//****************************************************************************
//
// SWIFT_GetStaticDeviceInfo ()
//
// Reads static device data.
//
//****************************************************************************

int SWIFT_GetStaticDeviceInfo (SWIFT_StaticData far *psd)
{
   memset (&RMI, 0, sizeof (RMI));
   RMI.ax = 0x53C1;                       // SWIFT: Get Static Device Data
   RMI.es = segment;                      // DOS buffer real-mode segment
   RMI.dx = 0;                            //  "    "      "   "   offset
   MouseInt (&RMI);                       // get data into DOS buffer

   *psd = *(SWIFT_StaticData *)pdosmem;   // then copy into caller's buffer
   return (RMI.ax == 1);                  // return success
}



//****************************************************************************
//
// SWIFT_Get3DStatus ()
//
// Read the current input state of the device.
//
//****************************************************************************


void SWIFT_Get3DStatus (SWIFT_3DStatus far *pstat)
{
#ifdef DBUG
   if (!fActive)
   {
      SoftError( "SWIFT_Get3DStatus: SWIFT module not active!\n");
   }
#endif

   memset (&RMI, 0, sizeof (RMI));
   RMI.ax = 0x5301;
   RMI.es = segment;
   RMI.dx = 0;
   MouseInt(&RMI);
   *pstat = *(SWIFT_3DStatus *)pdosmem;
}




//****************************************************************************
//
// SWIFT_TactileFeedback ()
//
// Generates tactile feedback to user.
// d   = duration of tactile burst, in milliseconds.
// on  = motor on-time per cycle, in milliseconds.
// off = motor off-time per cycle, in milliseconds.
//
//****************************************************************************

void SWIFT_TactileFeedback (int d, int on, int off)
{
  // Use DPMI call 300h to issue mouse interrupt
   memset (&RMI, 0, sizeof(RMI));
   RMI.ax = 0x5330;                    // SWIFT: Get Position & Buttons
   RMI.bx = (on / 5) << 8 + (off / 5);
   RMI.cx = d / 40;
   MouseInt (&RMI);

#ifdef DBUG
   SoftError( "SWIFT_TactileFeedback (dur=%d ms, on=%d ms, off=%d ms)\n",
                   d / 40 * 40, on/5*5, off/5*5);
#endif
}



//****************************************************************************
//
// SWIFT_GetDynamicDeviceData ()
//
// Returns Dynamic Device Data word - see SDD_* above
//
//****************************************************************************

unsigned SWIFT_GetDynamicDeviceData (void)
{
   memset (&RMI, 0, sizeof(RMI));
   RMI.ax = 0x53C2;                       // SWIFT: Get Dynamic Device Data
   MouseInt (&RMI);
   return ((unsigned)RMI.ax);
}


//****************************************************************************
//
// MouseInt ()
//
// Generate a call to the mouse driver (interrupt 33h) in real mode,
// using the DPMI function 'Simulate Real-Mode Interrupt'.
//
//****************************************************************************

void MouseInt (struct rminfo *prmi)
{
   memset (&sregs, 0, sizeof (sregs));
   AX (regs) = 0x0300;                    // DPMI: simulate interrupt
   BX (regs) = MOUSE_INT;
   CX (regs) = 0;
   DI (regs) = FP_OFF (prmi);
   sregs.es = FP_SEG (prmi);
   int386x( DPMI_INT, &regs, &regs, &sregs );
}


//****************************************************************************
//
// freeDOS ()
//
// Release real-mode DOS memory block via DPMI
//
//****************************************************************************

void freeDOS (short sel)
{
   AX(regs) = 0x0101;      // DPMI free DOS memory
   DX(regs) = sel;

   int386( DPMI_INT, &regs, &regs);
}


//****************************************************************************
//
// allocDOS ()
//
// Allocate a real-mode DOS memory block via DPMI
//
//****************************************************************************

void far *allocDOS (unsigned nbytes, short *pseg, short *psel)
{
   unsigned npara = (nbytes + 15) / 16;
   void far *pprot;
   pprot = NULL;
   *pseg = 0;        // assume will fail
   *psel = 0;

   // DPMI call 100h allocates DOS memory
   segread (&sregs);
   AX (regs) = 0x0100;        // DPMI: Allocate DOS Memory
   BX (regs) = npara;         // number of paragraphs to alloc
   int386( DPMI_INT, &regs, &regs);

   if (regs.w.cflag == 0)
   {
      *pseg = AX (regs);      // the real-mode segment
      *psel = DX (regs);      // equivalent protected-mode selector
      // pprot is the protected mode address of the same allocated block.
      // The Rational extender maps the 1 MB physical DOS memory into
      // the bottom of our virtual address space.
      pprot = (void far *) ((unsigned)*pseg << 4);
   }
   return pprot;
}

#else

/* This isn't of much use in Linux. */

int SWIFT_Initialize (void)
{
	STUB_FUNCTION;
	
	return 0;
}

void SWIFT_Terminate (void)
{
	STUB_FUNCTION;
}

void SWIFT_Get3DStatus (SWIFT_3DStatus far *pstat)
{
	STUB_FUNCTION;
}

void SWIFT_TactileFeedback (int d, int on, int off)
{
	STUB_FUNCTION;
}

unsigned SWIFT_GetDynamicDeviceData (void)
{
	STUB_FUNCTION;
	
	return 0;
}

#endif
