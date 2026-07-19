//============================================================================
//
//   SSSS    tt          lll  lll
//  SS  SS   tt           ll   ll
//  SS     tttttt  eeee   ll   ll   aaaa
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2024 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// This file has been modified by Dave Bernazzani (wavemotion-dave)
// for optimized execution on the DS/DSi platform. Please seek the
// official Stella source distribution which is far cleaner, newer,
// and better maintained.
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

#include <nds.h>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "Console.hxx"
#include "Control.hxx"
#include "M6502.hxx"
#include "System.hxx"
#include "TIA.hxx"
#include "TIASound.hxx"
#include "Cart.hxx"

#define HBLANK 68       // Standard HBLANK for both NTSC and PAL TVs

//#define TIA_HMOVE_DEBUG

// ---------------------------------------------------------------------------------------------------------
// All of this used to be in the TIA class but for maximum speed, this is moved it out into fast memory...
// ---------------------------------------------------------------------------------------------------------
uint32  myBlendBk                    
 = 0;
uInt32  myPF                         
;
uInt32  myColor[4]                   
;
uInt32  myStartDisplayOffset         
;
uInt32  myStopDisplayOffset          
;
Int32   myVSYNCFinishClock           
;
uInt8*  myCurrentFrameBuffer[2]      
;
uInt8*  myFramePointer               
;
extern uInt8* myDSFramePointer;
Int32   myLastHMOVEClock            ;
uInt32  (*ourPlayfieldTable)[160]   = NULL;
Int32   myClockWhenFrameStarted      
;
Int32   myCyclesWhenFrameStarted     
;
Int32   myClockStartDisplay          
;
Int32   myClockStopDisplay           
;
Int32   myClockAtLastUpdate          
;
Int32   myClocksToEndOfScanLine      
;
uInt8*  myCurrentBLMask              
;
uInt8*  myCurrentM0Mask              
;
uInt8*  myCurrentM1Mask              
;
uInt8*  myCurrentP0Mask              
;
uInt8*  myCurrentP1Mask              
;
uInt32* myCurrentPFMask              
;
uInt16* ourCollisionTable           = NULL;
uInt16  myCollision                  
;
Int16   myPOSP0                      
;
Int16   myPOSP1                      
;
Int16   myPOSM0                      
;
Int16   myPOSM1                      
;
Int16   myPOSBL                      
;
uInt8   myM0CosmicArkCounter         
;
uInt8   myM1CosmicArkCounter         
;
uInt8   myCurrentFrame               
;
uInt8   myNUSIZ0                     
;
uInt8   myNUSIZ1                     
;
uInt32  myPlayfieldPriorityAndScore  
;
uInt8   (*myPriorityEncoder)[256]   = NULL;
uInt8   myCTRLPF                     
;
uInt8   myREFP0                      
;
uInt8   myREFP1                      
;
uInt8   myGRP0                       
;
uInt8   myGRP1                       
;
uInt8   myDGRP0                      
;
uInt8   myDGRP1                      
;
uInt8   myENAM0                      
;
uInt8   myENAM1                      
;
uInt8   myENABL                      
;
uInt8   myDENABL                     
;
Int8    myHMP0                       
;
Int8    myHMP1                       
;
Int8    myHMM0                       
;
Int8    myHMM1                       
;
Int8    myHMBL                       
;
uInt8   myVDELP0                     
;
uInt8   myVDELP1                     
;
uInt8   myVDELBL                     
;
uInt8   myRESMP0                     
;
uInt8   myRESMP1                     
;
uInt8   myEnabledObjects             
;
uInt32  myCurrentGRP0                
;
uInt32  myCurrentGRP1                
;
uInt32  myVSYNC                      
;
uInt32  myVBLANK                     
;
uInt8   myHMOVEBlankEnabled          
;
uInt8   myM0CosmicArkMotionEnabled   
;
uInt8   myM1CosmicArkMotionEnabled   
;
uInt32  lastTiaPokeCycles            
 = 0;
uInt8   bWaveDirectSound             
 = 0;
uInt8   bFrameSkipCDFJ               
 = 0;
uInt8   bNoCollisionDetection        
 = 0;
uInt8*  ourPlayerReflectTable       = NULL;
uInt8*  ourPokeDelayTable           = NULL;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8* delay_tab                    = NULL;
uInt32  color_repeat_table[256];

// -----------------------------------------------------------
#define MYCOLUBK  0
#define MYCOLUPF  1
#define MYCOLUP0  2
#define MYCOLUP1  3

// TIA globals are now pointers to allow PSRAM/DRAM placement during install()
// Pointer initialization handled in TIA::install()

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TIA::TIA()
{
  myConsole = NULL;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TIA::~TIA()
{
    // Frame Buffers are static now... nothing to delete...
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* TIA::name() const
{
  return "TIA";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TIA::reset()
{
  // Clear frame buffers
  for(uInt32 i = 0; i < 160 * 245; ++i)
  {
    myCurrentFrameBuffer[0][i] = 0;
    // videoBuf1 points to the same memory as videoBuf0 now
  }
  myCurrentFrame = 0;

  // Bumper Bash requires shorter NUSIZx delay
  if (myCartInfo.special == SPEC_BUMPBASH)
  {
      ourPokeDelayTable[NUSIZ0] = 6;
      ourPokeDelayTable[NUSIZ1] = 6;
  }
  // Pole Position requires shorter NUSIZx delay
  else if (myCartInfo.special == SPEC_POLEPOS)
  {
      ourPokeDelayTable[NUSIZ0] = 1;
      ourPokeDelayTable[NUSIZ1] = 1;
  }
  else // The default delay... this is enough to render the sprite output before a change in NUSIZ takes place. Not perfect emulation - the new Stella TIA core handles it better.
  {
      ourPokeDelayTable[NUSIZ0] = 8;
      ourPokeDelayTable[NUSIZ1] = 8;
  }
  
  // Due to the hack to make MELTDOWN playable, we need to recompute this table on TIA reset...
  computePlayerMaskTable();

  // Reset pixel pointer and drawing flag
  myFramePointer = myCurrentFrameBuffer[0];
  myDSFramePointer = BG_GFX;

  // Calculate color clock offsets for starting and stoping frame drawing
  if (myCartInfo.displayNumScalines > 245) myCartInfo.displayNumScalines = 245;
  myStartDisplayOffset = 228 * myCartInfo.displayStartScanline;                              // Allow for some underscan lines on a per-cart basis
  myStopDisplayOffset = myStartDisplayOffset + (228 * (myCartInfo.displayNumScalines));      // Allow for some overscan lines on a per-cart basis

  // Reasonable values to start and stop the current frame drawing
  myCyclesWhenFrameStarted = gSystemCycles;
  myClockWhenFrameStarted = gSystemCycles * 3;
  myClockStartDisplay = myClockWhenFrameStarted + myStartDisplayOffset;
  myClockStopDisplay = myClockWhenFrameStarted + myStopDisplayOffset;
  myClockAtLastUpdate = myClockWhenFrameStarted;
  myClocksToEndOfScanLine = 228;
  myVSYNCFinishClock = 0x7FFFFFFF;

  // Currently no objects are enabled
  myEnabledObjects = 0;

  // Some default values for the registers
  myVSYNC = 0;
  myVBLANK = 0;
  myNUSIZ0 = 0;
  myNUSIZ1 = 0;
  myPlayfieldPriorityAndScore = 0;
  myColor[MYCOLUP0] = 0;
  myColor[MYCOLUP1] = 0;
  myColor[MYCOLUPF] = 0;
  myColor[MYCOLUBK] = 0;
  myCTRLPF = 0;
  myREFP0 = false;
  myREFP1 = false;
  myPF = 0;
  myGRP0 = 0;
  myGRP1 = 0;
  myDGRP0 = 0;
  myDGRP1 = 0;
  myENAM0 = false;
  myENAM1 = false;
  myENABL = false;
  myDENABL = false;
  myHMP0 = 0;
  myHMP1 = 0;
  myHMM0 = 0;
  myHMM1 = 0;
  myHMBL = 0;
  myVDELP0 = false;
  myVDELP1 = false;
  myVDELBL = false;
  myRESMP0 = false;
  myRESMP1 = false;
  myCollision = 0;
  myPOSP0 = 0;
  myPOSP1 = 0;
  myPOSM0 = 0;
  myPOSM1 = 0;
  myPOSBL = 0;


  // Some default values for the "current" variables
  myCurrentGRP0 = 0;
  myCurrentGRP1 = 0;
  myCurrentBLMask = ourBallMaskTable[0][0];
  myCurrentM0Mask = ourMissleMaskTable[0][0][0];
  myCurrentM1Mask = ourMissleMaskTable[0][0][0];
  myCurrentP0Mask = ourPlayerMaskTable[0][0][0];
  myCurrentP1Mask = ourPlayerMaskTable[0][0][0];
  myCurrentPFMask = ourPlayfieldTable[0];

  myLastHMOVEClock = 0;
  myHMOVEBlankEnabled = false;
  myM0CosmicArkMotionEnabled = false;
  myM1CosmicArkMotionEnabled = false;
  myM0CosmicArkCounter = 0;
  myM1CosmicArkCounter = 0;

  myDumpEnabled = false;
  myDumpDisabledCycle = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TIA::systemCyclesReset()
{
  // Get the current system cycle
  uInt32 cycles = gSystemCycles;

  // Adjust the dump cycle
  myDumpDisabledCycle -= cycles;

  // Get the current color clock the system is using
  uInt32 clocks = cycles * 3;

  // Adjust the clocks by this amount since we're reseting the clock to zero
  myCyclesWhenFrameStarted -= cycles;
  myClockWhenFrameStarted -= clocks;
  myClockStartDisplay -= clocks;
  myClockStopDisplay -= clocks;
  myClockAtLastUpdate -= clocks;
  myVSYNCFinishClock -= clocks;
  myLastHMOVEClock -= clocks;

  // This one just goes back to zero...
  lastTiaPokeCycles = 0;
}

extern "C" uInt8* videoBuf0;
extern "C" uInt8* videoBuf1;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TIA::install(System& system)
{
  if (ourBallMaskTable == NULL) ourBallMaskTable = (uInt8 (*)[4][320])rg_alloc(4 * 4 * 320 * sizeof(uInt8), MEM_SLOW);
  if (ourDisabledMaskTable == NULL) ourDisabledMaskTable = (uInt8 *)rg_alloc(640 * sizeof(uInt8), MEM_SLOW);
  if (ourMissleMaskTable == NULL) ourMissleMaskTable = (uInt8 (*)[8][4][320])rg_alloc(4 * 8 * 4 * 320 * sizeof(uInt8), MEM_SLOW);
  if (ourPlayerMaskTable == NULL) ourPlayerMaskTable = (uInt8 (*)[2][8][320])rg_alloc(4 * 2 * 8 * 320 * sizeof(uInt8), MEM_SLOW);
  if (ourPlayerPositionResetWhenTable == NULL) ourPlayerPositionResetWhenTable = (Int8 (*)[160][160])rg_alloc(8 * 160 * 160 * sizeof(Int8), MEM_SLOW);
  if (ourPlayfieldTable == NULL) ourPlayfieldTable = (uInt32 (*)[160])rg_alloc(2 * 160 * sizeof(uInt32), MEM_SLOW);
  if (ourCollisionTable == NULL) ourCollisionTable = (uInt16 *)rg_alloc(256 * sizeof(uInt16), MEM_SLOW);
  if (myPriorityEncoder == NULL) myPriorityEncoder = (uInt8 (*)[256])rg_alloc(2 * 256 * sizeof(uInt8), MEM_SLOW);
  if (ourPlayerReflectTable == NULL) ourPlayerReflectTable = (uInt8 *)rg_alloc(256 * sizeof(uInt8), MEM_SLOW);
  if (ourPokeDelayTable == NULL) {
    ourPokeDelayTable = (uInt8 *)rg_alloc(64 * sizeof(uInt8), MEM_SLOW);
    const uInt8 defaults[64] = {
      0, 1, 0, 0, 8, 8, 0, 0, 0, 0, 0, 1, 1, 0x80, 0x80, 0x80,
      0, 0, 8, 8, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1
    };
    memcpy(ourPokeDelayTable, defaults, 64);
  }
  if (delay_tab == NULL) {
    delay_tab = (uInt8 *)rg_alloc(256 * sizeof(uInt8), MEM_SLOW);
    const uInt8 d_defaults[] = {
        4, 4, 4, 5, 5, 5, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 2, 2, 2, 3, 3, 3,
        4, 4, 4, 5, 5, 5, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 2, 2, 2, 3, 3, 3,
        4, 4, 4, 5, 5, 5, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 2, 2, 2, 3, 3, 3,
        4, 4, 4, 5, 5, 5, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 2, 2, 2, 3, 3, 3,
        4, 4, 4, 5, 5, 5, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 2, 2, 2, 3, 3, 3
    };
    memcpy(delay_tab, d_defaults, sizeof(d_defaults));
  }

  if (videoBuf0 == NULL) videoBuf0 = (uInt8 *)rg_alloc(160 * 245, MEM_FAST);
  if (videoBuf1 == NULL) videoBuf1 = videoBuf0;

  memset(videoBuf0, 0x00, 160 * 245);
  myCurrentFrameBuffer[0] = videoBuf0;
  myCurrentFrameBuffer[1] = videoBuf1;

  for(uInt16 x = 0; x < 2; ++x) {
    for(uInt16 enabled = 0; enabled < 256; ++enabled) {
      uInt8 color = 0;
      if(enabled & PriorityBit) {
        if((enabled & (myP1Bit | myM1Bit)) != 0) color = 3;
        if((enabled & (myP0Bit | myM0Bit)) != 0) color = 2;
        if((enabled & myBLBit) != 0)             color = 1;
        if((enabled & myPFBit) != 0)             color = 1;
      } else {
        if((enabled & myBLBit) != 0)             color = 1;
        if((enabled & myPFBit) != 0)             color = (enabled & ScoreBit) ? ((x == 0) ? 2 : 3) : 1;
        if((enabled & (myP1Bit | myM1Bit)) != 0) color = 3;
        if((enabled & (myP0Bit | myM0Bit)) != 0) color = 2;
      }
      myPriorityEncoder[x][enabled] = color;
    }
  }
  for(uInt32 i = 0; i < 640; ++i) ourDisabledMaskTable[i] = 0;
  for (uInt32 i=0; i<256; i += 2)
  {
      color_repeat_table[i]   = (uInt32)(i<<24) | (uInt32)(i<<16) | (uInt32)(i<<8) | (uInt32)i;
      color_repeat_table[i+1] = (uInt32)(i<<24) | (uInt32)(i<<16) | (uInt32)(i<<8) | (uInt32)i;
  }

  computeBallMaskTable();
  computeCollisionTable();
  computeMissleMaskTable();
  computePlayerMaskTable();
  computePlayerPositionResetWhenTable();
  computePlayerReflectTable();
  computePlayfieldMaskTable();

  // Remember which system I'm installed in
  mySystem = &system;

  uInt16 shift = mySystem->pageShift();
  mySystem->resetCycles();


  // All accesses are to this device
  PageAccess access;
  access.directPeekBase = 0;
  access.directPokeBase = 0;
  access.device = this;

  // We're installing in a 2600 system
  for(uInt32 i = 0; i < 8192; i += (1 << shift))
  {
    if((i & 0x1080) == 0x0000)
    {
      mySystem->setPageAccess(i >> shift, access);
    }
  }
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ITCM_CODE void TIA::update()
{
  // We have processed another frame... used for true FPS indication
  gAtariFrames++;
  gTotalAtariFrames++;

  // Remember the number of clocks which have passed on the current scanline
  // so that we can adjust the frame's starting clock by this amount.  This
  // is necessary since some games position objects during VSYNC and the
  // TIA's internal counters are not reset by VSYNC.
  uInt32 clocks = ((gSystemCycles * 3) - myClockWhenFrameStarted) % 228;

  // Ask the system to reset the cycle count so it doesn't overflow
  mySystem->resetCycles();

  // Setup clocks that'll be used for drawing this frame
  myCyclesWhenFrameStarted = -(clocks/3);
  myClockWhenFrameStarted = -clocks;
  myClockStartDisplay = myClockWhenFrameStarted + myStartDisplayOffset;
  myClockStopDisplay = myClockWhenFrameStarted + myStopDisplayOffset;
  myClockAtLastUpdate = myClockStartDisplay;
  myClocksToEndOfScanLine = 228;

  // Reset frame buffer pointer
  myCurrentFrame = (myCurrentFrame + 1) % 2;
  myFramePointer = myCurrentFrameBuffer[myCurrentFrame];
  myDSFramePointer = BG_GFX;

  bWaveDirectSound = (myCartInfo.soundQuality == SOUND_WAVE);
  
  bNoCollisionDetection = (myCartInfo.thumbOptimize & 2) ? 1:0;
  

  // If the cart requires frame skipping (mostly for Turbo Arcade and Draconian)
  // then we set the skip flag for every other pair of frames
  if (myCartInfo.thumbOptimize == 3)
  {
      extern uInt8 bElevatorAgent;
      if (bElevatorAgent)
        bFrameSkipCDFJ = ((gTotalAtariFrames & 0x02) ? 1:0);    // Heavy Frameskip (2 on, 2 off)
      else
        bFrameSkipCDFJ = ((gTotalAtariFrames & 0x03) ? 0:1);    // Moderate Frameskip (3 on, 1 off)
  }
  else bFrameSkipCDFJ = 0;

  // --------------------------------------------------------------------
  // Execute instructions until frame is finished
  // --------------------------------------------------------------------
  // For games that can be specially executed for speed... do so here.
  // --------------------------------------------------------------------
  switch (cartDriver)
  {
      case 1: mySystem->m6502().execute_4K();            break;   // If we are 2K or 4K (non-banked), we can run faster here...
      case 2: mySystem->m6502().execute_F8();            break;   // If we are F8, we can run faster here...
      case 3: mySystem->m6502().execute_F6();            break;   // If we are F6, we can run faster here...
      case 4: mySystem->m6502().execute_F4();            break;   // If we are F4, we can run faster here...
      case 5: mySystem->m6502().execute_AR();            break;   // If we are AR, we can run faster here...
      case 6: mySystem->m6502().execute_F8SC();          break;   // If we are F8SC, we can run faster here...
      case 7: mySystem->m6502().execute_F6SC();          break;   // If we are F6SC, we can run faster here...
      case 8: mySystem->m6502().execute_DPCP();          break;   // If we are DPC+, we can run faster here...
      case 9: mySystem->m6502().execute_CDFJ();          break;   // If we are CDF/CDFJ, we can run faster here...
      case 10: mySystem->m6502().execute_CDFJPlus();     break;   // If we are CDFJ+, we can run faster here...
      case 11: mySystem->m6502().execute_CDFJPlusPlus(); break;   // If we are CDFJ+ PLUS!!, we can run faster here... This is non-standard but highly optmized for games like Elevator Agent
      case 12: mySystem->m6502().execute_DPC();          break;   // If we are DPC (Pitfall II), we can run faster here...
      case 13: mySystem->m6502().execute_CTY();          break;   // If we are CTY (Cherity), we can run faster here...
      default: mySystem->m6502().execute();              break;   // Otherwise the normal execute driver
  }
  stack_executionStatus = (volatile int*)&debug[19];   // In theory not needed but in case someone tries to access this outside of the emulation driver
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const uInt32* TIA::palette() const
{
    if (myCartInfo.tv_type == PAL)
    {
        if (myCartInfo.palette_type == 0) return ourPALPaletteDS;
        if (myCartInfo.palette_type == 1) return ourPALPalette;
        if (myCartInfo.palette_type == 2) return ourPALPaletteZ26;
        return ourPALPalette;
    }
    else
    {
        if (myCartInfo.palette_type == 0) return ourNTSCPaletteDS;
        if (myCartInfo.palette_type == 1) return ourNTSCPalette;
        if (myCartInfo.palette_type == 2) return ourNTSCPaletteZ26;
        return ourNTSCPalette;
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 TIA::width() const
{
  return 160;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// height() defined in TIA.hxx


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TIA::computeBallMaskTable()
{
  // First, calculate masks for alignment 0
  for(Int32 size = 0; size < 4; ++size)
  {
    Int32 x;

    // Set all of the masks to false to start with
    for(x = 0; x < 160; ++x)
    {
      ourBallMaskTable[0][size][x] = false;
    }

    // Set the necessary fields true
    for(x = 0; x < 160 + 8; ++x)
    {
      if((x >= 0) && (x < (1 << size)))
      {
        ourBallMaskTable[0][size][x % 160] = true;
      }
    }

    // Copy fields into the wrap-around area of the mask
    for(x = 0; x < 160; ++x)
    {
      ourBallMaskTable[0][size][x + 160] = ourBallMaskTable[0][size][x];
    }
  }

  // Now, copy data for alignments of 1, 2 and 3
  for(uInt32 align = 1; align < 4; ++align)
  {
    for(uInt32 size = 0; size < 4; ++size)
    {
      for(uInt32 x = 0; x < 320; ++x)
      {
        ourBallMaskTable[align][size][x] =
            ourBallMaskTable[0][size][(x + 320 - align) % 320];
      }
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TIA::computeCollisionTable()
{
  for(uInt8 i = 0; i < 64; ++i)
  {
    ourCollisionTable[i] = 0;

    if((i & myM0Bit) && (i & myP1Bit))    // M0-P1
      ourCollisionTable[i] |= 0x0001;

    if((i & myM0Bit) && (i & myP0Bit))    // M0-P0
      ourCollisionTable[i] |= 0x0002;

    if((i & myM1Bit) && (i & myP0Bit))    // M1-P0
      ourCollisionTable[i] |= 0x0004;

    if((i & myM1Bit) && (i & myP1Bit))    // M1-P1
      ourCollisionTable[i] |= 0x0008;

    if((i & myP0Bit) && (i & myPFBit))    // P0-PF
      ourCollisionTable[i] |= 0x0010;

    if((i & myP0Bit) && (i & myBLBit))    // P0-BL
      ourCollisionTable[i] |= 0x0020;

    if((i & myP1Bit) && (i & myPFBit))    // P1-PF
      ourCollisionTable[i] |= 0x0040;

    if((i & myP1Bit) && (i & myBLBit))    // P1-BL
      ourCollisionTable[i] |= 0x0080;

    if((i & myM0Bit) && (i & myPFBit))    // M0-PF
      ourCollisionTable[i] |= 0x0100;

    if((i & myM0Bit) && (i & myBLBit))    // M0-BL
      ourCollisionTable[i] |= 0x0200;

    if((i & myM1Bit) && (i & myPFBit))    // M1-PF
      ourCollisionTable[i] |= 0x0400;

    if((i & myM1Bit) && (i & myBLBit))    // M1-BL
      ourCollisionTable[i] |= 0x0800;

    if((i & myBLBit) && (i & myPFBit))    // BL-PF
      ourCollisionTable[i] |= 0x1000;

    if((i & myP0Bit) && (i & myP1Bit))    // P0-P1
      ourCollisionTable[i] |= 0x2000;

    if((i & myM0Bit) && (i & myM1Bit))    // M0-M1
      ourCollisionTable[i] |= 0x4000;
  }

  for(int i = 64; i < 256; i++)
  {
        ourCollisionTable[i] = ourCollisionTable[i%64];
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TIA::computeMissleMaskTable()
{
  // First, calculate masks for alignment 0
  Int32 x, size, number;

  // Clear the missle table to start with
  for(number = 0; number < 8; ++number)
    for(size = 0; size < 4; ++size)
      for(x = 0; x < 160; ++x)
        ourMissleMaskTable[0][number][size][x] = false;

  for(number = 0; number < 8; ++number)
  {
    for(size = 0; size < 4; ++size)
    {
      for(x = 0; x < 160 + 72; ++x)
      {
        // Only one copy of the missle
        if((number == 0x00) || (number == 0x05) || (number == 0x07))
        {
          if((x >= 0) && (x < (1 << size)))
            ourMissleMaskTable[0][number][size][x % 160] = true;
        }
        // Two copies - close
        else if(number == 0x01)
        {
          if((x >= 0) && (x < (1 << size)))
            ourMissleMaskTable[0][number][size][x % 160] = true;
          else if(((x - 16) >= 0) && ((x - 16) < (1 << size)))
            ourMissleMaskTable[0][number][size][x % 160] = true;
        }
        // Two copies - medium
        else if(number == 0x02)
        {
          if((x >= 0) && (x < (1 << size)))
            ourMissleMaskTable[0][number][size][x % 160] = true;
          else if(((x - 32) >= 0) && ((x - 32) < (1 << size)))
            ourMissleMaskTable[0][number][size][x % 160] = true;
        }
        // Three copies - close
        else if(number == 0x03)
        {
          if((x >= 0) && (x < (1 << size)))
            ourMissleMaskTable[0][number][size][x % 160] = true;
          else if(((x - 16) >= 0) && ((x - 16) < (1 << size)))
            ourMissleMaskTable[0][number][size][x % 160] = true;
          else if(((x - 32) >= 0) && ((x - 32) < (1 << size)))
            ourMissleMaskTable[0][number][size][x % 160] = true;
        }
        // Two copies - wide
        else if(number == 0x04)
        {
          if((x >= 0) && (x < (1 << size)))
            ourMissleMaskTable[0][number][size][x % 160] = true;
          else if(((x - 64) >= 0) && ((x - 64) < (1 << size)))
            ourMissleMaskTable[0][number][size][x % 160] = true;
        }
        // Three copies - medium
        else if(number == 0x06)
        {
          if((x >= 0) && (x < (1 << size)))
            ourMissleMaskTable[0][number][size][x % 160] = true;
          else if(((x - 32) >= 0) && ((x - 32) < (1 << size)))
            ourMissleMaskTable[0][number][size][x % 160] = true;
          else if(((x - 64) >= 0) && ((x - 64) < (1 << size)))
            ourMissleMaskTable[0][number][size][x % 160] = true;
        }
      }

      // Copy data into wrap-around area
      for(x = 0; x < 160; ++x)
        ourMissleMaskTable[0][number][size][x + 160] =
          ourMissleMaskTable[0][number][size][x];
    }
  }

  // Now, copy data for alignments of 1, 2 and 3
  for(uInt32 align = 1; align < 4; ++align)
  {
    for(number = 0; number < 8; ++number)
    {
      for(size = 0; size < 4; ++size)
      {
        for(x = 0; x < 320; ++x)
        {
          ourMissleMaskTable[align][number][size][x] =
            ourMissleMaskTable[0][number][size][(x + 320 - align) % 320];
        }
      }
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TIA::computePlayerMaskTable()
{
  // First, calculate masks for alignment 0
  Int32 x, enable, mode;

  // Set the player mask table to all zeros
  for(enable = 0; enable < 2; ++enable)
    for(mode = 0; mode < 8; ++mode)
      for(x = 0; x < 160; ++x)
        ourPlayerMaskTable[0][enable][mode][x] = 0x00;

  // Now, compute the player mask table
  for(enable = 0; enable < 2; ++enable)
  {
    for(mode = 0; mode < 8; ++mode)
    {
      for(x = 0; x < 160 + 72; ++x)
      {
        if(mode == 0x00)
        {
          if((enable == 0) && (x >= 0) && (x < 8))
            ourPlayerMaskTable[0][enable][mode][x % 160] = 0x80 >> x;
        }
        else if(mode == 0x01)
        {
          if((enable == 0) && (x >= 0) && (x < 8))
            ourPlayerMaskTable[0][enable][mode][x % 160] = 0x80 >> x;
          else if(((x - 16) >= 0) && ((x - 16) < 8))
            ourPlayerMaskTable[0][enable][mode][x % 160] = 0x80 >> (x - 16);
        }
        else if(mode == 0x02)
        {
          if((enable == 0) && (x >= 0) && (x < 8))
            ourPlayerMaskTable[0][enable][mode][x % 160] = 0x80 >> x;
          else if(((x - 32) >= 0) && ((x - 32) < 8))
            ourPlayerMaskTable[0][enable][mode][x % 160] = 0x80 >> (x - 32);
        }
        else if(mode == 0x03)
        {
          if((enable == 0) && (x >= 0) && (x < 8))
            ourPlayerMaskTable[0][enable][mode][x % 160] = 0x80 >> x;
          else if(((x - 16) >= 0) && ((x - 16) < 8))
            ourPlayerMaskTable[0][enable][mode][x % 160] = 0x80 >> (x - 16);
          else if(((x - 32) >= 0) && ((x - 32) < 8))
            ourPlayerMaskTable[0][enable][mode][x % 160] = 0x80 >> (x - 32);
        }
        else if(mode == 0x04)
        {
          if (myCartInfo.special == SPEC_MELTDOWN) // Hack for Meltdown to compensate for NUSIZ hit during Player Draw - the effect will show 2 player sprites at double-size 
          {
              if((enable == 0) && (x >= 0) && (x <= 16))
                ourPlayerMaskTable[0][enable][mode][x % 160] = 0x80 >> x / 2;
              else if(((x - 64) >= 0) && ((x - 64) <= 16))
                ourPlayerMaskTable[0][enable][mode][x % 160] = 0x80 >> (x - 64) / 2;
          }
          else
          {
              if((enable == 0) && (x >= 0) && (x < 8))
                ourPlayerMaskTable[0][enable][mode][x % 160] = 0x80 >> x;
              else if(((x - 64) >= 0) && ((x - 64) < 8))
                ourPlayerMaskTable[0][enable][mode][x % 160] = 0x80 >> (x - 64);
          }
        }
        else if(mode == 0x05)
        {
          if (myCartInfo.special == SPEC_MELTDOWN) // Hack for Meltdown to compensate for NUSIZ hit during Player Draw - the effect will show 2 player sprites at double-size 
          {
              if((enable == 0) && (x >= 0) && (x <= 16))
                ourPlayerMaskTable[0][enable][mode][x % 160] = 0x80 >> x / 2;
              else if(((x - 64) >= 0) && ((x - 64) <= 16))
                ourPlayerMaskTable[0][enable][mode][x % 160] = 0x80 >> (x - 64) / 2;
          }
          else
          {
              // For some reason in double size mode the player's output
              // is delayed by one pixel thus we use > instead of >=
              if((enable == 0) && (x > 0) && (x <= 16))
                ourPlayerMaskTable[0][enable][mode][x % 160] = 0x80 >> ((x - 1)/2);
          }
        }
        else if(mode == 0x06)
        {
          if((enable == 0) && (x >= 0) && (x < 8))
            ourPlayerMaskTable[0][enable][mode][x % 160] = 0x80 >> x;
          else if(((x - 32) >= 0) && ((x - 32) < 8))
            ourPlayerMaskTable[0][enable][mode][x % 160] = 0x80 >> (x - 32);
          else if(((x - 64) >= 0) && ((x - 64) < 8))
            ourPlayerMaskTable[0][enable][mode][x % 160] = 0x80 >> (x - 64);
        }
        else if(mode == 0x07)
        {
          // For some reason in quad size mode the player's output
          // is delayed by one pixel thus we use > instead of >=
          if((enable == 0) && (x > 0) && (x <= 32))
            ourPlayerMaskTable[0][enable][mode][x % 160] = 0x80 >> ((x - 1)/4);
        }
      }

      // Copy data into wrap-around area
      for(x = 0; x < 160; ++x)
      {
        ourPlayerMaskTable[0][enable][mode][x + 160] =
            ourPlayerMaskTable[0][enable][mode][x];
      }
    }
  }

  // Now, copy data for alignments of 1, 2 and 3
  for(uInt32 align = 1; align < 4; ++align)
  {
    for(enable = 0; enable < 2; ++enable)
    {
      for(mode = 0; mode < 8; ++mode)
      {
        for(x = 0; x < 320; ++x)
        {
          ourPlayerMaskTable[align][enable][mode][x] =
              ourPlayerMaskTable[0][enable][mode][(x + 320 - align) % 320];
        }
      }
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TIA::computePlayerPositionResetWhenTable()
{
  uInt32 mode, oldx, newx;

  // Loop through all player modes, all old player positions, and all new
  // player positions and determine where the new position is located:
  // 1 means the new position is within the display of an old copy of the
  // player, -1 means the new position is within the delay portion of an
  // old copy of the player, and 0 means it's neither of these two
  for(mode = 0; mode < 8; ++mode)
  {
    for(oldx = 0; oldx < 160; ++oldx)
    {
      // Set everything to 0 for non-delay/non-display section
      for(newx = 0; newx < 160; ++newx)
      {
        ourPlayerPositionResetWhenTable[mode][oldx][newx] = 0;
      }

      // Now, we'll set the entries for non-delay/non-display section
      for(newx = 0; newx < 160 + 72 + 5; ++newx)
      {
        if(mode == 0x00)
        {
          if((newx >= oldx) && (newx < (oldx + 4)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = -1;

          if((newx >= oldx + 4) && (newx < (oldx + 4 + 8)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = 1;
        }
        else if(mode == 0x01)
        {
          if((newx >= oldx) && (newx < (oldx + 4)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = -1;
          else if((newx >= (oldx + 16)) && (newx < (oldx + 16 + 4)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = -1;

          if((newx >= oldx + 4) && (newx < (oldx + 4 + 8)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = 1;
          else if((newx >= oldx + 16 + 4) && (newx < (oldx + 16 + 4 + 8)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = 1;
        }
        else if(mode == 0x02)
        {
          if((newx >= oldx) && (newx < (oldx + 4)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = -1;
          else if((newx >= (oldx + 32)) && (newx < (oldx + 32 + 4)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = -1;

          if((newx >= oldx + 4) && (newx < (oldx + 4 + 8)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = 1;
          else if((newx >= oldx + 32 + 4) && (newx < (oldx + 32 + 4 + 8)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = 1;
        }
        else if(mode == 0x03)
        {
          if((newx >= oldx) && (newx < (oldx + 4)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = -1;
          else if((newx >= (oldx + 16)) && (newx < (oldx + 16 + 4)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = -1;
          else if((newx >= (oldx + 32)) && (newx < (oldx + 32 + 4)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = -1;

          if((newx >= oldx + 4) && (newx < (oldx + 4 + 8)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = 1;
          else if((newx >= oldx + 16 + 4) && (newx < (oldx + 16 + 4 + 8)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = 1;
          else if((newx >= oldx + 32 + 4) && (newx < (oldx + 32 + 4 + 8)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = 1;
        }
        else if(mode == 0x04)
        {
          if((newx >= oldx) && (newx < (oldx + 4)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = -1;
          else if((newx >= (oldx + 64)) && (newx < (oldx + 64 + 4)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = -1;

          if((newx >= oldx + 4) && (newx < (oldx + 4 + 8)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = 1;
          else if((newx >= oldx + 64 + 4) && (newx < (oldx + 64 + 4 + 8)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = 1;
        }
        else if(mode == 0x05)
        {
          if((newx >= oldx) && (newx < (oldx + 4)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = -1;

          if((newx >= oldx + 4) && (newx < (oldx + 4 + 16)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = 1;
        }
        else if(mode == 0x06)
        {
          if((newx >= oldx) && (newx < (oldx + 4)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = -1;
          else if((newx >= (oldx + 32)) && (newx < (oldx + 32 + 4)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = -1;
          else if((newx >= (oldx + 64)) && (newx < (oldx + 64 + 4)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = -1;

          if((newx >= oldx + 4) && (newx < (oldx + 4 + 8)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = 1;
          else if((newx >= oldx + 32 + 4) && (newx < (oldx + 32 + 4 + 8)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = 1;
          else if((newx >= oldx + 64 + 4) && (newx < (oldx + 64 + 4 + 8)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = 1;
        }
        else if(mode == 0x07)
        {
          if((newx >= oldx) && (newx < (oldx + 4)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = -1;

          if((newx >= oldx + 4) && (newx < (oldx + 4 + 32)))
            ourPlayerPositionResetWhenTable[mode][oldx][newx % 160] = 1;
        }
      }

      // Let's do a sanity check on our table entries
      uInt32 s1 = 0, s2 = 0;
      for(newx = 0; newx < 160; ++newx)
      {
        if(ourPlayerPositionResetWhenTable[mode][oldx][newx] == -1)
          ++s1;
        if(ourPlayerPositionResetWhenTable[mode][oldx][newx] == 1)
          ++s2;
      }
      assert((s1 % 4 == 0) && (s2 % 8 == 0));
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TIA::computePlayerReflectTable()
{
  for(uInt16 i = 0; i < 256; ++i)
  {
    uInt8 r = 0;

    for(uInt16 t = 1; t <= 128; t *= 2)
    {
      r = (r << 1) | ((i & t) ? 0x01 : 0x00);
    }

    ourPlayerReflectTable[i] = r;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TIA::computePlayfieldMaskTable()
{
  Int32 x;

  // Compute playfield mask table for non-reflected mode
  for(x = 0; x < 160; ++x)
  {
    if(x < 16)
    {
      int tmp = 0x00001 << (x / 4);
      ourPlayfieldTable[0][x] = tmp;
      ourPlayfieldTable[1][x] = tmp;
    }
    else if(x < 48)
    {
      int tmp = 0x00800 >> ((x - 16) / 4);
      ourPlayfieldTable[0][x] = tmp;
      ourPlayfieldTable[1][x] = tmp;
    }
    else if(x < 80)
    {
      int tmp = 0x01000 << ((x - 48) / 4);
      ourPlayfieldTable[0][x] = tmp;
      ourPlayfieldTable[1][x] = tmp;
    }
    else if(x < 96)
    {
      ourPlayfieldTable[0][x] = 0x00001 << ((x - 80) / 4);
    }
    else if(x < 128)
    {
      ourPlayfieldTable[0][x] = 0x00800 >> ((x - 96) / 4);
    }
    else if(x < 160)
    {
      ourPlayfieldTable[0][x] = 0x01000 << ((x - 128) / 4);
    }
  }

  // Compute playfield mask table for reflected mode
  for(x = 80; x < 160; ++x)
  {
     if(x < 112)
      ourPlayfieldTable[1][x] = 0x80000 >> ((x - 80) / 4);
    else if(x < 144)
      ourPlayfieldTable[1][x] = 0x00010 << ((x - 112) / 4);
    else if(x < 160)
      ourPlayfieldTable[1][x] = 0x00008 >> ((x - 144) / 4);
  }
}

// -----------------------------------------------------------------------
// Magic! This helps speedup rendering as it assumes the last color
// is likely to be the next color drawn - and corrects if that's
// not true. We keep track of the last enabled bits and if those
// bits have not changed, there is no reason to update the myCollision
// register and we can just blast out the last known color.
// -----------------------------------------------------------------------
#define HANDLE_COLOR_AND_COLLISIONS  \
              if (enabled == last_enabled)  \
              {  \
                  *mfp = last_color;  \
                  if (hpos < 80) {if (++hpos == 80) last_enabled=255;}  \
              }  \
              else  \
              {  \
                  myCollision |= ourCollisionTable[enabled];  \
                  if (hpos < 80)  \
                  {  \
                      last_color = myColor[myPriorityEncoder[0][enabled]];  \
                      hpos++;  \
                  }  \
                  else  \
                  {  \
                      last_color = myColor[myPriorityEncoder[1][enabled]];  \
                  }  \
                  *mfp = last_color;  \
                  last_enabled = enabled;  \
              }

// -----------------------------------------------------------------------
// We spent a LOT of time in here... so we've done our best to keep this
// as streamlined as possible. We could reduce this to about 10 lines of
// code if source-code / memory was at a premium. But it's not - instead
// we are after speed of execution and are willing to trade off these
// large if-then-else blocks to help with code execution to get as many
// games running at full frame rate as possible...
// -----------------------------------------------------------------------
void TIA::handleObjectsAndCollisions(Int32 clocksToUpdate, Int32 hpos)
{
    uInt8 last_color=0;
    uInt8 last_enabled=255;
    uInt8* ending = myFramePointer + clocksToUpdate;  // Calculate the ending frame pointer value
    
    uint8* mfp = myFramePointer;

    switch (myEnabledObjects)
    {
    #include "TIA.inc"
    }
    myFramePointer = ending;
}

// -----------------------------------------------------------------------
// For some of the DPC+ and many of the CDF/CDFJ we don't need to handle
// any special color or collisions so we can do this the fast way..
// Note that we are purposely ignoring the ScoreBit handling here which
// provides a bit more speed and nobody is using that for the ARM-Assisted
// games that also don't provide any collision usage. It's not a safe
// way to do this... but nothing has broken it and we need the speed.
// -----------------------------------------------------------------------
#undef HANDLE_COLOR_AND_COLLISIONS
#define HANDLE_COLOR_AND_COLLISIONS  *mfp = myColor[myPriorityEncoder[0][enabled]];

void TIA::handleObjectsNoCollisions(Int32 clocksToUpdate, Int32 hpos)
{
    uInt8* ending = myFramePointer + clocksToUpdate;  // Calculate the ending frame pointer value
    uint8* mfp = myFramePointer;
    
    #define COLLISIONS_OFF

    switch (myEnabledObjects)
    {
    #include "TIA.inc"
    }
    myFramePointer = ending;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ITCM_CODE void TIA::updateFrame(Int32 clock)
{
  // -------------------------------------------------------------------------------------
  // Games like Elevator Agent are highly demanding and require us to skip frames
  // to even have a chance of keeping up... Check that here and render 2 frames,
  // then skip 2 frames, etc. It causes minor flicker but we're desperate!
  // -------------------------------------------------------------------------------------
  if (bFrameSkipCDFJ) return;

  // ---------------------------------------------------------------
  // See if we're in the nondisplayable portion of the screen or if
  // we've already updated this portion of the screen
  // We do these in the order of "most likely"
  // ---------------------------------------------------------------
  if (clock < myClockStartDisplay)               return;
  if (myClockAtLastUpdate >= myClockStopDisplay) return;
  if (myClockAtLastUpdate >= clock)              return;

  // Truncate the number of cycles to update to the stop display point
  if (unlikely(clock > myClockStopDisplay))
  {
    clock = myClockStopDisplay;
  }

  // Update frame one scanline at a time
  do
  {
    // Safety check: ensure we don't draw outside our buffer
    if (myFramePointer > (myCurrentFrameBuffer[myCurrentFrame] + (160 * 240))) break;

    // Compute the number of clocks we're going to update
    Int32 clocksToUpdate = clock - myClockAtLastUpdate;

    // Remember how many clocks we are from the left side of the screen
    Int32 clocksFromStartOfScanLine = 228 - myClocksToEndOfScanLine;

    // See if we're updating more than the current scanline
    if(clock > (myClockAtLastUpdate + myClocksToEndOfScanLine))
    {
      // Yes, we have more than one scanline to update so finish current one
      clocksToUpdate = myClocksToEndOfScanLine;
      myClocksToEndOfScanLine = 228;
      myClockAtLastUpdate += clocksToUpdate;
    }
    else
    {
      // No, so do as much of the current scanline as possible
      myClocksToEndOfScanLine -= clocksToUpdate;
      myClockAtLastUpdate = clock;
    }

    // Skip over as many horizontal blank clocks as we can
    if(clocksFromStartOfScanLine < HBLANK)
    {
      uInt32 tmp;

      if((HBLANK - clocksFromStartOfScanLine) < clocksToUpdate)
        tmp = HBLANK - clocksFromStartOfScanLine;
      else
        tmp = clocksToUpdate;

      clocksFromStartOfScanLine += tmp;
      clocksToUpdate -= tmp;
    }


    // Remember frame pointer in case HMOVE blanks need to be handled
    uInt8* oldFramePointer = myFramePointer;

    // ----------------------------------------------------------------
    // Update as much of the scanline as we can. This is slow so
    // this routine has been optimized to handle objects in a
    // sort of "best" order...
    // ----------------------------------------------------------------

    if (clocksToUpdate)
    {
        // If we are updating, make sure the Enabled Objects register is up to date...
        if(myPF != 0)
          myEnabledObjects |= myPFBit;
        else
          myEnabledObjects &= ~myPFBit;

        // See if we're in the vertical blank region
        if(myVBLANK & 0x02)
        {
            // -------------------------------------------------------------------------------------------
            // Some games present a fairly static screen from frame to frame and so there is really no
            // reason to blank the memory which can be time consuming... so check the flag for the cart
            // currently being emulated...
            // -------------------------------------------------------------------------------------------
            if (myCartInfo.vblankZero || !(gAtariFrames & 0x0E))    // Every 16 frames we will do two frames proper vBlank despite the cartInfo (two is needed as some games display different data on alternating frames)
            {
                memset(myFramePointer, 0x00, clocksToUpdate);
            }
            myFramePointer += clocksToUpdate;
        }
        else if (myEnabledObjects == 0x00)  // Background handling...
        {
            memset(myFramePointer, myColor[MYCOLUBK], clocksToUpdate);
            myFramePointer += clocksToUpdate;
        }
        else  // All other possibilities... this is expensive CPU-wise
        {
            Int32 hpos = clocksFromStartOfScanLine - HBLANK;
            if (bNoCollisionDetection)  // If we are Optimizing the ARM Thumb with NO collisions...
            {
                if ((myEnabledObjects|myPlayfieldPriorityAndScore) == myPFBit) // Playfield bit set... without priority/score (common in CDF/J/+ games)
                {
                       uInt8* ending = myFramePointer + clocksToUpdate;  // Calculate the ending frame pointer value
                       uInt32* mask = &myCurrentPFMask[hpos];
                       // Simplified to byte-by-byte for ESP32 unaligned safety and buffer protection
                       for(; myFramePointer < ending; myFramePointer++, mask++)
                       {
                         *myFramePointer = myColor[(myPF & *mask) ? MYCOLUPF:MYCOLUBK];
                       }
                       myFramePointer = ending;

                }
                else
                {
                    handleObjectsNoCollisions(clocksToUpdate, hpos);
                }
            }
            else    // Normal handling...
            {
                handleObjectsAndCollisions(clocksToUpdate, hpos);
            }
        }
    }

    // Handle HMOVE blanks if they are enabled
    if(myHMOVEBlankEnabled)
    {
        Int32 blanks = (HBLANK + 8) - clocksFromStartOfScanLine;
        if (blanks > 0)
        {
            memset(oldFramePointer, 0, blanks);

            if((clocksToUpdate + clocksFromStartOfScanLine) >= (HBLANK + 8))
            {
                myHMOVEBlankEnabled = false;
            }
        }
    }

    // ------------------------------------------------------------------------
    // If we are mid-scanline... we record the background color for blending
    // ------------------------------------------------------------------------
    if (myClocksToEndOfScanLine < 190)
    {
        myBlendBk = myColor[MYCOLUBK];
    }
    else
    // See if we're at the end of a scanline
    if(myClocksToEndOfScanLine == 228)
    {
      // Yes, so set PF mask based on current CTRLPF reflection state
      myCurrentPFMask = ourPlayfieldTable[myCTRLPF & 0x01];

      // TODO: These should be reset right after the first copy of the player
      // has passed.  However, for now we'll just reset at the end of the
      // scanline since the other way would be to slow (01/21/99).
      myCurrentP0Mask = &ourPlayerMaskTable[myPOSP0 & 0x03][0][myNUSIZ0 & 0x07][160 - (myPOSP0 & 0xFC)];
      myCurrentP1Mask = &ourPlayerMaskTable[myPOSP1 & 0x03][0][myNUSIZ1 & 0x07][160 - (myPOSP1 & 0xFC)];

      // Handle the "Cosmic Ark" TIA bug if it's enabled
      if(myM0CosmicArkMotionEnabled)
      {
        // Movement table associated with the bug
        static uInt16 m[4] = {18, 33, 0, 17};

        myM0CosmicArkCounter = (myM0CosmicArkCounter + 1) & 3;
        myPOSM0 -= m[myM0CosmicArkCounter];

        if(myPOSM0 >= 160)
          myPOSM0 -= 160;
        else if(myPOSM0 < 0)
          myPOSM0 += 160;

        if(myM0CosmicArkCounter == 1)
        {
          // Stretch this missle so it's at least 2 pixels wide
          myCurrentM0Mask = &ourMissleMaskTable[myPOSM0 & 0x03][myNUSIZ0 & 0x07][((myNUSIZ0 & 0x30) >> 4) | 0x01][160 - (myPOSM0 & 0xFC)];
        }
        else if(myM0CosmicArkCounter == 2)
        {
          // Missle is disabled on this line
          myCurrentM0Mask = &ourDisabledMaskTable[0];
        }
        else
        {
          myCurrentM0Mask = &ourMissleMaskTable[myPOSM0 & 0x03][myNUSIZ0 & 0x07][(myNUSIZ0 & 0x30) >> 4][160 - (myPOSM0 & 0xFC)];
        }
      }

      // Handle the "Cosmic Ark" TIA bug if it's enabled
      if(myM1CosmicArkMotionEnabled)
      {
        // Movement table associated with the bug
        static uInt16 m[4] = {18, 33, 0, 17};

        myM1CosmicArkCounter = (myM1CosmicArkCounter + 1) & 3;
        myPOSM1 -= m[myM1CosmicArkCounter];

        if(myPOSM1 >= 160)
          myPOSM1 -= 160;
        else if(myPOSM1 < 0)
          myPOSM1 += 160;

        if(myM1CosmicArkCounter == 1)
        {
          // Stretch this missle so it's at least 2 pixels wide
          myCurrentM1Mask = &ourMissleMaskTable[myPOSM1 & 0x03]
              [myNUSIZ1 & 0x07][((myNUSIZ1 & 0x30) >> 4) | 0x01]
              [160 - (myPOSM1 & 0xFC)];
        }
        else if(myM1CosmicArkCounter == 2)
        {
          // Missle is disabled on this line
          myCurrentM1Mask = &ourDisabledMaskTable[0];
        }
        else
        {
          myCurrentM1Mask = &ourMissleMaskTable[myPOSM1 & 0x03][myNUSIZ1 & 0x07][(myNUSIZ1 & 0x30) >> 4][160 - (myPOSM1 & 0xFC)];
        }
      }
    }
  }
  while(myClockAtLastUpdate < clock);
}

// ---------------------------------------------------------------------
// Only handle basic controller input and skip accurate bus "noise"
// Useful for CDFJ/+ games which don't bother with collision detection.
// ---------------------------------------------------------------------
ITCM_CODE uInt8 TIA::peek_minimal(uInt8 addr)
{
    addr &= 0x0F;

    switch (addr)
    {
    case 0x08:    // INPT0
      return (myConsole->controller(Controller::Left).read(Controller::Nine) == Controller::minimumResistance) ? 0x80 : 0x00;

    case 0x09:    // INPT1
      return (myConsole->controller(Controller::Left).read(Controller::Five) == Controller::minimumResistance) ? 0x80 : 0x00;

    case 0x0C:    // INPT4
      return myConsole->controller(Controller::Left).read(Controller::Six) ? 0x80 : 0x00;

    case 0x0D:    // INPT5
      return myConsole->controller(Controller::Right).read(Controller::Six) ? 0x80 : 0x00;

    default:
      return 0x00;
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ITCM_CODE uInt8 TIA::peek(uInt16 addr)
{
    // ----------------------------------------------------------------
    // The undriven bits of the TIA are usually what's last on the bus
    // so we do a poor-man's emulation of that by setting the noise to
    // what we last read/wrote on the data bus. It's not perfect.
    // For Conquest of Mars, this will always be set to 0x02 to avoid
    // a collision detect "bug" in the game (works on real HW)
    // ----------------------------------------------------------------
    uInt8 noise = (myDataBusState & 0x3F);

    addr &= 0x000F;

    if (addr < 8)
    {
        // Update frame to current color clock before we look at anything!
        updateFrame((3*gSystemCycles));
        if (!myCollision) {return noise;}
        switch(addr)
        {
        case 0x00:    // CXM0P
          if (myCollision & 0x0001) noise |= 0x80;
          if (myCollision & 0x0002) noise |= 0x40;
          return noise;
        case 0x01:    // CXM1P
          if (myCollision & 0x0004) noise |= 0x80;
          if (myCollision & 0x0008) noise |= 0x40;
          return noise;
        case 0x02:    // CXP0FB
          if (myCollision & 0x0010) noise |= 0x80;
          if (myCollision & 0x0020) noise |= 0x40;
          return noise;
        case 0x03:    // CXP1FB
          if (myCollision & 0x0040) noise |= 0x80;
          if (myCollision & 0x0080) noise |= 0x40;
          return noise;
        case 0x04:    // CXM0FB
          if (myCollision & 0x0100) noise |= 0x80;
          if (myCollision & 0x0200) noise |= 0x40;
          return noise;
        case 0x05:    // CXM1FB
          if (myCollision & 0x0400) noise |= 0x80;
          if (myCollision & 0x0800) noise |= 0x40;
          return noise;
        case 0x06:    // CXBLPF
          if (myCollision & 0x1000) noise |= 0x80;
          return noise;
        case 0x07:    // CXPPMM
          if (myCollision & 0x2000) noise |= 0x80;
          if (myCollision & 0x4000) noise |= 0x40;
          return noise;
        }
    }

    // ----------------------------------------------------------------
    // Otherwise it's an input read... don't need to update frame yet.
    // ----------------------------------------------------------------
    switch (addr)
    {
    case 0x08:    // INPT0
    {
      Int32 r = myConsole->controller(Controller::Left).read(Controller::Nine);
      if(r == Controller::minimumResistance)
      {
        return 0x80 | noise;
      }
      else if((r == Controller::maximumResistance) || myDumpEnabled)
      {
        return noise;
      }
      else
      {
        uInt32 needed = (r*10) / 525; //52.52 actually
        if((uInt32)gSystemCycles > (myDumpDisabledCycle + needed))
        {
          return 0x80 | noise;
        }
        else
        {
          return noise;
        }
      }
    }

    case 0x09:    // INPT1
    {
      Int32 r = myConsole->controller(Controller::Left).read(Controller::Five);
      if(r == Controller::minimumResistance)
      {
        return 0x80 | noise;
      }
      else if((r == Controller::maximumResistance) || myDumpEnabled)
      {
        return noise;
      }
      else
      {
        uInt32 needed = (r*10) / 525; //52.52 actually
        if((uInt32)gSystemCycles > (myDumpDisabledCycle + needed))
        {
          return 0x80 | noise;
        }
        else
        {
          return noise;
        }
      }
    }

    case 0x0A:    // INPT2
    {
      Int32 r = myConsole->controller(Controller::Right).read(Controller::Nine);
      if(r == Controller::minimumResistance)
      {
        return 0x80 | noise;
      }
      else if((r == Controller::maximumResistance) || myDumpEnabled)
      {
        return noise;
      }
      else
      {
        uInt32 needed = (r*10) / 525; //52.52 actually
        if((uInt32)gSystemCycles > (myDumpDisabledCycle + needed))
        {
          return 0x80 | noise;
        }
        else
        {
          return noise;
        }
      }
    }

    case 0x0B:    // INPT3
    {
      Int32 r = myConsole->controller(Controller::Right).read(Controller::Five);
      if(r == Controller::minimumResistance)
      {
        return 0x80 | noise;
      }
      else if((r == Controller::maximumResistance) || myDumpEnabled)
      {
        return noise;
      }
      else
      {
        uInt32 needed = (r*10) / 525; //52.52 actually
        if((uInt32)gSystemCycles > (myDumpDisabledCycle + needed))
        {
          return 0x80 | noise;
        }
        else
        {
          return noise;
        }
      }
    }

    case 0x0C:    // INPT4
      return myConsole->controller(Controller::Left).read(Controller::Six) ? (0x80 | noise) : noise;

    case 0x0D:    // INPT5
      return myConsole->controller(Controller::Right).read(Controller::Six) ? (0x80 | noise) : noise;

    default:
      return noise;
    }
}

uInt8 poke_needs_update_display[]  
 =
{
    1,1,0,1,1,1,1,1,   1,1,1,1,1,1,1,1,
    1,1,1,1,1,0,0,0,   0,0,0,1,1,1,1,1,
    
    1,1,1,1,1,1,1,1,   1,1,1,1,0,0,0,0,
    0,0,0,0,0,0,0,0,   0,0,0,0,0,0,0,0
};

uInt8 player_reset_pos[] =
{
    3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
    3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
    3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
    3,   3,   3,   3,   3,   3,   3,   3,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,
   17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,
   37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,
   57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,
   77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96,
   97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116,
  117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136,
  137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156,
  157, 158, 159,   0,   1,   2,   3,   4
};

void __attribute__ ((noinline))  ResetBallEarlyHack(Int32 clock, uInt8 hpos)
{
  // This is a special hack for Escape from the Mindmaster (AR) and Mission Survive (4K)
  if((clock - myLastHMOVEClock) == (18 * 3))
  {
    if ((hpos == 60) || (hpos == 69))  myPOSBL = 10; // Escape from the Mindmaster (01/09/99)
    else if (hpos == 63)
    {
          if (myCartInfo.special == SPEC_AR) myPOSBL = 9; // Labyrinth AR (prototype)
          else  myPOSBL = 7; // Mission Survive (04/11/08)
    }
  }
  // This is a special hack for Escape from the Mindmaster 
  else if(((clock - myLastHMOVEClock) == (15 * 3)) && (hpos == 60))
  {
    myPOSBL = 10;
  }
  // This is a special hack for Decathlon 
  else if(((clock - myLastHMOVEClock) == (3 * 3)) && (hpos == 18))
  {
    myPOSBL = 3;
  }
  // This is a special hack for Robot Tank 
  else if(((clock - myLastHMOVEClock) == (7 * 3)) && (hpos == 30))
  {
    myPOSBL = 6;
  }
  // This is a special hack for Hole Hunter
  else if(((clock - myLastHMOVEClock) == (6 * 3)) && (hpos == 27))
  {
    myPOSBL = 5;
  }
  // This is a special hack for Swoops! 
  else if(((clock - myLastHMOVEClock) == (9 * 3)) && (hpos == 36))
  {
    myPOSBL = 7;
  }
  // This is a special hack for Solaris 
  else if(((clock - myLastHMOVEClock) == (12 * 3)) && (hpos == 45))
  {
    myPOSBL = 8;
  }
  // This is a special hack for Pole Position
  else if(((clock - myLastHMOVEClock) == (21 * 3)) && (hpos == 72))
  {
    myPOSBL = 8;
  }
  // This is a special hack for Moon Patrol
  else if(((clock - myLastHMOVEClock) == (20 * 3)) && (hpos == 69))
  {
      myPOSBL = 8;
  }          
#ifdef TIA_HMOVE_DEBUG
  else if ((clock - myLastHMOVEClock) <= (23 * 3))
  {
      debug[8] = (clock - myLastHMOVEClock)/3;
      debug[9] = hpos;
  }
#endif      
}

// Hack to GI Joe for early hits of Missile Position during HMOVE... this helps to fix the massive graphical glitches though it's not perfect
void HackGIJoe(Int32 clock)
{
    if ((clock - myLastHMOVEClock) == (8 * 3))
    {
      myHMP1 = (myHMP1 + 1) % 160;
    }
    else if ((clock - myLastHMOVEClock) == (21 * 3))
    {
      myHMP1 = (myHMP1 + 2) % 160;
    }
}

#ifndef TIA_HMOVE_DEBUG
ITCM_CODE 
#endif
void TIA::poke(uInt16 addr, uInt8 value)
{
  Int32 clock;
  Int32 delta_clock;
  addr = addr & 0x003f;

  // Update frame to current CPU cycle before we make any changes!
  if (poke_needs_update_display[addr])
  {
      if (bFrameSkipCDFJ)
      {
          if (addr > 0x02) return;
      }

      clock = (3*gSystemCycles);
      delta_clock = (clock - myClockWhenFrameStarted);
      uInt8 delay = ourPokeDelayTable[addr];
      // See if this is a poke to a PF register
      if (delay & 0x80)
      {
        delay = delay_tab[delta_clock % 228];
      }
      updateFrame(clock + delay);
  }
  else
  {
      clock = 0;
      delta_clock = 0;
  }

  if (unlikely(bWaveDirectSound))
  {
      while ((gSystemCycles - lastTiaPokeCycles) >= 76)
      {
          lastTiaPokeCycles += 76;
          Tia_process();
      }
  }

  switch(addr)
  {
    case 0x00:    // Vertical sync set-clear
    {
      myVSYNC = value;

      if(myVSYNC & 0x02)
      {
        // Indicate when VSYNC should be finished.  This should really
        // be 3 * 228 according to Atari's documentation, however, some
        // games don't supply the full 3 scanlines of VSYNC.
        myVSYNCFinishClock = clock + 228;
      }
      else if(clock >= myVSYNCFinishClock)
      {
        // We're no longer interested in myVSYNCFinishClock
        myVSYNCFinishClock = 0x7FFFFFFF;

        // Since we're finished with the frame tell the processor to halt
        mySystem->m6502().stop();
      }
      break;
    }

    case 0x01:    // Vertical blank set-clear
    {
      // Is the dump to ground path being set for I0, I1, I2, and I3?
      if(!(myVBLANK & 0x80) && (value & 0x80))
      {
        myDumpEnabled = true;
      }
      else
      // Is the dump to ground path being removed from I0, I1, I2, and I3?
      if((myVBLANK & 0x80) && !(value & 0x80))
      {
        myDumpEnabled = false;
        myDumpDisabledCycle = gSystemCycles;
      }

      myVBLANK = value;
      break;
    }

    case 0x02:    // Wait for leading edge of HBLANK (WSYNC)
    {
        // Tell the cpu to waste the necessary amount of time
        uInt32 cyclesToEndOfLine = 76 - ((gSystemCycles - myCyclesWhenFrameStarted) % 76);

        if (cyclesToEndOfLine < 76)
        {
            gSystemCycles += cyclesToEndOfLine;
        }
        break;
    }

    case 0x03:    // Reset horizontal sync counter (RSYNC)
    {
        uInt32 cyclesToEndOfLine = 76 - ((gSystemCycles - myCyclesWhenFrameStarted) % 76);
        gSystemCycles += (cyclesToEndOfLine-1);
        break;
    }

    case 0x04:    // Number-size of player-missile 0 (NUSIZ0)
    {
        myNUSIZ0 = value;
        myCurrentP0Mask = &ourPlayerMaskTable[myPOSP0 & 0x03][0][myNUSIZ0 & 0x07][160 - (myPOSP0 & 0xFC)];
        myCurrentM0Mask = &ourMissleMaskTable[myPOSM0 & 0x03][myNUSIZ0 & 0x07][(myNUSIZ0 & 0x30) >> 4][160 - (myPOSM0 & 0xFC)];
        break;
    }

    case 0x05:    // Number-size of player-missile 1 (NUSIZ1)
    {
        myNUSIZ1 = value;
        myCurrentP1Mask = &ourPlayerMaskTable[myPOSP1 & 0x03][0][myNUSIZ1 & 0x07][160 - (myPOSP1 & 0xFC)];
        myCurrentM1Mask = &ourMissleMaskTable[myPOSM1 & 0x03][myNUSIZ1 & 0x07][(myNUSIZ1 & 0x30) >> 4][160 - (myPOSM1 & 0xFC)];
        break;
    }

    case 0x06:    // Color-Luminance Player 0
    {
      myColor[MYCOLUP0] = color_repeat_table[value];
      break;
    }

    case 0x07:    // Color-Luminance Player 1
    {
      myColor[MYCOLUP1] = color_repeat_table[value];
      break;
    }

    case 0x08:    // Color-Luminance Playfield
    {
      myColor[MYCOLUPF] = color_repeat_table[value];
      break;
    }

    case 0x09:    // Color-Luminance Background
    {
      myColor[MYCOLUBK] = color_repeat_table[value];
      break;
    }

    case 0x0A:    // Control Playfield, Ball size, Collisions
    {
        if (myCTRLPF != value)
        {
          myCTRLPF = value;

          // The playfield priority and score bits from the control register
          // are accessed when the frame is being drawn.  We precompute the
          // necessary value here so we can save time while drawing.
          myPlayfieldPriorityAndScore = ((myCTRLPF & 0x06) << 5);

          myCurrentBLMask = &ourBallMaskTable[myPOSBL & 0x03]
              [(myCTRLPF & 0x30) >> 4][160 - (myPOSBL & 0xFC)];
        }

        // Update the playfield mask based on reflection state if
        // we're still on the left hand side of the playfield
        if(((delta_clock) % 228) < (HBLANK + 79))
        {
            myCurrentPFMask = ourPlayfieldTable[myCTRLPF & 0x01];
        }
      break;
    }

    case 0x0B:    // Reflect Player 0
    {
      // See if the reflection state of the player is being changed
      if ((value ^ myREFP0) & 0x08)
      {
        myREFP0 = (value & 0x08);
        myCurrentGRP0 = ourPlayerReflectTable[myCurrentGRP0];
      }
      break;
    }

    case 0x0C:    // Reflect Player 1
    {
      // See if the reflection state of the player is being changed
      if ((value ^ myREFP1) & 0x08)
      {
        myREFP1 = (value & 0x08);
        myCurrentGRP1 = ourPlayerReflectTable[myCurrentGRP1];
      }
      break;
    }

    case 0x0D:    // Playfield register byte 0
    {
      myPF = (myPF & 0x000FFFF0) | ((value >> 4) & 0x0F);
      break;
    }

    case 0x0E:    // Playfield register byte 1
    {
      myPF = (myPF & 0x000FF00F) | ((uInt32)value << 4);
      break;
    }

    case 0x0F:    // Playfield register byte 2
    {
      myPF = (myPF & 0x00000FFF) | ((uInt32)value << 12);
      break;
    }

    case 0x10:    // Reset Player 0
    {
      uInt8 hpos = (delta_clock) % 228;
      uInt8 newx = player_reset_pos[hpos];  //hpos < HBLANK ? 3 : (((hpos - HBLANK) + 5) % 160);

      // This is a special hack for Grand Prix
      if ((clock - myLastHMOVEClock) == (21 * 3) && (hpos==72))
      {
          newx = 11;
      }
      // This is a special hack for Space Rocks
      else if ((clock - myLastHMOVEClock) == (23 * 3) && (hpos==69))
      {
          newx = 11;
      }
      // This is a special hack for Draconian
      else if ((clock - myLastHMOVEClock) == (20 * 3) && (hpos==69))
      {
          newx = 11;
      }
      // This is a special hack for Double Dragon (Activision)
      else if ((clock - myLastHMOVEClock) == (19 * 3) && (hpos==66))
      {
          newx = 11;
      }
#ifdef TIA_HMOVE_DEBUG
      else if ((clock - myLastHMOVEClock) <= (23 * 3))
      {
          debug[0] = (clock - myLastHMOVEClock)/3;
          debug[1] = hpos;
      }
#endif      

      // Find out under what condition the player is being reset
      Int8 when = ourPlayerPositionResetWhenTable[myNUSIZ0 & 7][myPOSP0][newx];

      // Player is being reset in neither the delay nor display section
      if (when == 0)
      {
        myPOSP0 = newx;

        // So we setup the mask to skip the first copy of the player
        myCurrentP0Mask = &ourPlayerMaskTable[myPOSP0 & 0x03][1][myNUSIZ0 & 0x07][160 - (myPOSP0 & 0xFC)];
      }
      // Player is being reset during the display of one of its copies
      else if (when == 1)
      {
        // So we go ahead and update the display before moving the player
        // TODO: The 11 should depend on how much of the player has already
        // been displayed.  Probably change table to return the amount to
        // delay by instead of just 11 (01/21/99).
        updateFrame(clock + 11);

        myPOSP0 = newx;

        // Setup the mask to skip the first copy of the player
        myCurrentP0Mask = &ourPlayerMaskTable[myPOSP0 & 0x03][1][myNUSIZ0 & 0x07][160 - (myPOSP0 & 0xFC)];
      }
      // Player is being reset during the delay section of one of its copies
      else
      {
        myPOSP0 = newx;

        // So we setup the mask to display all copies of the player
        myCurrentP0Mask = &ourPlayerMaskTable[myPOSP0 & 0x03][0][myNUSIZ0 & 0x07][160 - (myPOSP0 & 0xFC)];
      }
      break;
    }

    case 0x11:    // Reset Player 1
    {
      uInt8 hpos = (delta_clock) % 228;
      uInt8 newx = player_reset_pos[hpos];  //hpos < HBLANK ? 3 : (((hpos - HBLANK) + 5) % 160);

      if (hpos == 69)
      {
          // This is a special hack for Space Rocks
          if ((clock - myLastHMOVEClock) == (23 * 3))
          {
              newx = 11;
          }
          // This is a special hack for Draconian, Masters of the Universe, Rabbit Transit, Jungle Hunt, and Dragon Stomper (Excalibur) by StarPath/Arcadia
          else if ((clock - myLastHMOVEClock) == (20 * 3))
          {
              newx = 11;
          }
#ifdef TIA_HMOVE_DEBUG      
          else if ((clock - myLastHMOVEClock) <= (23 * 3))
          {
              debug[2] = (clock - myLastHMOVEClock)/3;
              debug[3] = hpos;
          }
#endif
      }
#ifdef TIA_HMOVE_DEBUG      
      else if ((clock - myLastHMOVEClock) <= (23 * 3))
      {
          debug[2] = (clock - myLastHMOVEClock)/3;
          debug[3] = hpos;
      }
#endif

      // Find out under what condition the player is being reset
      Int8 when = ourPlayerPositionResetWhenTable[myNUSIZ1 & 7][myPOSP1][newx];

      // Player is being reset in neither the delay nor display section
      if (when == 0)
      {
        myPOSP1 = newx;

        // So we setup the mask to skip the first copy of the player
        myCurrentP1Mask = &ourPlayerMaskTable[myPOSP1 & 0x03][1][myNUSIZ1 & 0x07][160 - (myPOSP1 & 0xFC)];
      }
      // Player is being reset during the display of one of its copies
      else if (when == 1)
      {
        // So we go ahead and update the display before moving the player
        // TODO: The 11 should depend on how much of the player has already
        // been displayed.  Probably change table to return the amount to
        // delay by instead of just 11 (01/21/99).
        updateFrame(clock + 11);

        myPOSP1 = newx;

        // Setup the mask to skip the first copy of the player
        myCurrentP1Mask = &ourPlayerMaskTable[myPOSP1 & 0x03][1][myNUSIZ1 & 0x07][160 - (myPOSP1 & 0xFC)];
      }
      // Player is being reset during the delay section of one of its copies
      else
      {
        myPOSP1 = newx;

        // So we setup the mask to display all copies of the player
        myCurrentP1Mask = &ourPlayerMaskTable[myPOSP1 & 0x03][0][myNUSIZ1 & 0x07][160 - (myPOSP1 & 0xFC)];
      }
      break;
    }

    case 0x12:    // Reset Missle 0
    {
      uInt8 hpos = (delta_clock) % 228;
      myPOSM0 = hpos < HBLANK ? 2 : (((hpos - HBLANK) + 4) % 160);
      
      // This is a special hack for Pole Position
      if(((clock - myLastHMOVEClock) == (12 * 3)) && (hpos == 45))
      {
          myPOSM0 = 9;
      }
      // This is a special hack for Dolphin
      else if(((clock - myLastHMOVEClock) == (20 * 3)) && (hpos == 69))
      {
          myPOSM0 = 8;
      }
      // This is a special hack for Solaris 
      else if(((clock - myLastHMOVEClock) == (9 * 3)) && (hpos == 36))
      {
          myPOSM0 = 8;
      }
#ifdef TIA_HMOVE_DEBUG      
      else if ((clock - myLastHMOVEClock) <= (23 * 3))
      {
          debug[4] = (clock - myLastHMOVEClock)/3;
          debug[5] = hpos;
      }
#endif
      myCurrentM0Mask = &ourMissleMaskTable[myPOSM0 & 0x03][myNUSIZ0 & 0x07][(myNUSIZ0 & 0x30) >> 4][160 - (myPOSM0 & 0xFC)];
      break;
    }

    case 0x13:    // Reset Missle 1
    {
      uInt8 hpos = (delta_clock) % 228;
      myPOSM1 = hpos < HBLANK ? 2 : (((hpos - HBLANK) + 4) % 160);

      // This is a special hack for Pole Position
      if(((clock - myLastHMOVEClock) == (15 * 3)) && (hpos == 54))
      {
          myPOSM1 = 9;
      }
      // This is a special hack for Pitfall II 
      else if(((clock - myLastHMOVEClock) == (3 * 3)) && (hpos == 18))
      {
          myPOSM1 = 3;
      }
      // This is a special hack for Moon Patrol
      else if(((clock - myLastHMOVEClock) == (20 * 3)) && (hpos == 69))
      {
          myPOSM1 = 9;
      }
      // This is a special hack for Ski Hunt
      else if(((clock - myLastHMOVEClock) == (21 * 3)) && (hpos == 72))
      {
          myPOSM1 = 9;
      }
      // This is a special hack for Bumper Bash
      else if (((clock - myLastHMOVEClock) == (130 * 3)) && (hpos == 171))
      {
          if (myCartInfo.special == SPEC_BUMPBASH) myPOSM1 += 4;    // Fixes right bumper position
      }
#ifdef TIA_HMOVE_DEBUG
      else if ((clock - myLastHMOVEClock) <= (23 * 3))
      {
          debug[6] = (clock - myLastHMOVEClock)/3;
          debug[7] = hpos;
      }
#endif

      myCurrentM1Mask = &ourMissleMaskTable[myPOSM1 & 0x03][myNUSIZ1 & 0x07][(myNUSIZ1 & 0x30) >> 4][160 - (myPOSM1 & 0xFC)];
      break;
    }

    case 0x14:    // Reset Ball
    {
      uInt8 hpos = (delta_clock) % 228 ;
      myPOSBL = hpos < HBLANK ? 2 : (((hpos - HBLANK) + 4) % 160);

      // -----------------------------------------------------------------------------------------
      // If the reset comes "too soon" after the last HMove, the TIA does some strange things.
      // Modern versions of Stella are cycle accurate and handle this perfectly - but here
      // we have the old Stella core and need to adjust a few special cases...
      // -----------------------------------------------------------------------------------------
      if ((clock - myLastHMOVEClock) <= (23*3))
      {
          ResetBallEarlyHack(clock, hpos);
      }
      
      myCurrentBLMask = &ourBallMaskTable[myPOSBL & 0x03][(myCTRLPF & 0x30) >> 4][160 - (myPOSBL & 0xFC)];
      break;
    }

    case 0x15:    // Audio control 0
          AUDC[0] = value & 0x0f;
          Update_tia_sound_0();
          break;
    case 0x16:    // Audio control 1
          AUDC[1] = value & 0x0f;
          Update_tia_sound_1();
          break;

    case 0x17:    // Audio frequency 0
          AUDF[0] = value & 0x1f;
          Update_tia_sound_0();
          break;
    case 0x18:    // Audio frequency 1
          AUDF[1] = value & 0x1f;
          Update_tia_sound_1();
          break;

    case 0x19:    // Audio volume 0
          AUDV[0] = (value & 0x0f) << 3;
          Update_tia_sound_0();
          break;

    case 0x1A:    // Audio volume 1
          AUDV[1] = (value & 0x0f) << 3;
          Update_tia_sound_1();
          break;

    case 0x1B: // Graphics Player 0
    {
      // Set player 0 graphics
      myGRP0 = value;

      // Copy player 1 graphics into its delayed register
      myDGRP1 = myGRP1;

      // Get the "current" data for GRP0 base on delay register and reflect
      uInt8 grp0 = myVDELP0 ? myDGRP0 : myGRP0;
      myCurrentGRP0 = myREFP0 ? ourPlayerReflectTable[grp0] : grp0;

      // Get the "current" data for GRP1 base on delay register and reflect
      uInt8 grp1 = myVDELP1 ? myDGRP1 : myGRP1;
      myCurrentGRP1 = myREFP1 ? ourPlayerReflectTable[grp1] : grp1;

      // Set enabled object bits
      if(myCurrentGRP0 != 0)
        myEnabledObjects |= myP0Bit;
      else
        myEnabledObjects &= ~myP0Bit;

      if(myCurrentGRP1 != 0)
        myEnabledObjects |= myP1Bit;
      else
        myEnabledObjects &= ~myP1Bit;
      break;
    }

    case 0x1C: // Graphics Player 1
    {
      // Set player 1 graphics
      myGRP1 = value;

      // Copy player 0 graphics into its delayed register
      myDGRP0 = myGRP0;

      // Copy ball graphics into its delayed register
      myDENABL = myENABL;

      // Get the "current" data for GRP0 base on delay register
      uInt8 grp0 = myVDELP0 ? myDGRP0 : myGRP0;
      myCurrentGRP0 = myREFP0 ? ourPlayerReflectTable[grp0] : grp0;

      // Get the "current" data for GRP1 base on delay register
      uInt8 grp1 = myVDELP1 ? myDGRP1 : myGRP1;
      myCurrentGRP1 = myREFP1 ? ourPlayerReflectTable[grp1] : grp1;

      // Set enabled object bits
      if(myCurrentGRP0 != 0)
        myEnabledObjects |= myP0Bit;
      else
        myEnabledObjects &= ~myP0Bit;

      if(myCurrentGRP1 != 0)
        myEnabledObjects |= myP1Bit;
      else
        myEnabledObjects &= ~myP1Bit;

      if(myVDELBL ? myDENABL : myENABL)
        myEnabledObjects |= myBLBit;
      else
        myEnabledObjects &= ~myBLBit;
      break;
    }

    case 0x1D:    // Enable Missle 0 graphics
    {
      myENAM0 = value & 0x02;

      if(myENAM0 && !myRESMP0)
        myEnabledObjects |= myM0Bit;
      else
        myEnabledObjects &= ~myM0Bit;
      break;
    }

    case 0x1E:    // Enable Missle 1 graphics
    {
      myENAM1 = value & 0x02;

      if(myENAM1 && !myRESMP1)
        myEnabledObjects |= myM1Bit;
      else
        myEnabledObjects &= ~myM1Bit;
      break;
    }

    case 0x1F:    // Enable Ball graphics
    {
      myENABL = value & 0x02;

      if(myVDELBL ? myDENABL : myENABL)
        myEnabledObjects |= myBLBit;
      else
        myEnabledObjects &= ~myBLBit;

      break;
    }

    case 0x20:    // Horizontal Motion Player 0
    {
      myHMP0 = value >> 4;
#ifdef TIA_HMOVE_DEBUG
      if ((clock - myLastHMOVEClock) <= (23 * 3))
      {
          uInt8 hpos = (delta_clock) % 228;
          debug[10] = (clock - myLastHMOVEClock)/3;
          debug[11] = hpos;
      }
#endif
      break;
    }

    case 0x21:    // Horizontal Motion Player 1
    {
      myHMP1 = value >> 4;
      if (myCartInfo.special == SPEC_GIJOE)
      {
          HackGIJoe(clock);
      }
#ifdef TIA_HMOVE_DEBUG
      if ((clock - myLastHMOVEClock) <= (23 * 3))
      {
          uInt8 hpos = (delta_clock) % 228;
          debug[12] = (clock - myLastHMOVEClock)/3;
          debug[13] = hpos;
      }
#endif
      break;
    }

    case 0x22:    // Horizontal Motion Missle 0
    {
      Int8 tmp = value >> 4;

      // Should we enabled TIA M0 "bug" used for stars in Cosmic Ark?
      if((clock == (myLastHMOVEClock + 21 * 3)) && (myHMM0 == 7) && (tmp == 6))
      {
        myM0CosmicArkMotionEnabled = true;
        myM0CosmicArkCounter = 0;
      }
#ifdef TIA_HMOVE_DEBUG
      if ((clock - myLastHMOVEClock) <= (23 * 3))
      {
          uInt8 hpos = (delta_clock) % 228;
          debug[14] = (clock - myLastHMOVEClock)/3;
          debug[15] = hpos;
      }
#endif
      myHMM0 = tmp;
      break;
    }

    case 0x23:    // Horizontal Motion Missle 1
    {
      Int8 tmp = value >> 4;

      // Should we enabled TIA M1 "bug" used for stars in Stay Frosty?
      if((clock == (myLastHMOVEClock + 21 * 3)) && (myHMM1 == 7) && (tmp == 6))
      {
        myM1CosmicArkMotionEnabled = true;
        myM1CosmicArkCounter = 0;
      }
#ifdef TIA_HMOVE_DEBUG
      if ((clock - myLastHMOVEClock) <= (23 * 3))
      {
          uInt8 hpos = (delta_clock) % 228;
          debug[16] = (clock - myLastHMOVEClock)/3;
          debug[20] = hpos;
      }
#endif
      myHMM1 = tmp;
      break;
    }

    case 0x24:    // Horizontal Motion Ball
    {
      myHMBL = value >> 4;
      break;
    }

    case 0x25:    // Vertical Delay Player 0
    {
      myVDELP0 = value & 0x01;

      uInt8 grp0 = myVDELP0 ? myDGRP0 : myGRP0;
      myCurrentGRP0 = myREFP0 ? ourPlayerReflectTable[grp0] : grp0;

      if(myCurrentGRP0 != 0)
        myEnabledObjects |= myP0Bit;
      else
        myEnabledObjects &= ~myP0Bit;
      break;
    }

    case 0x26:    // Vertical Delay Player 1
    {
      myVDELP1 = value & 0x01;

      uInt8 grp1 = myVDELP1 ? myDGRP1 : myGRP1;
      myCurrentGRP1 = myREFP1 ? ourPlayerReflectTable[grp1] : grp1;

      if(myCurrentGRP1 != 0)
        myEnabledObjects |= myP1Bit;
      else
        myEnabledObjects &= ~myP1Bit;
      break;
    }

    case 0x27:    // Vertical Delay Ball
    {
      myVDELBL = value & 0x01;

      if(myVDELBL ? myDENABL : myENABL)
        myEnabledObjects |= myBLBit;
      else
        myEnabledObjects &= ~myBLBit;
      break;
    }

    case 0x28:    // Reset missle 0 to player 0
    {
      if(myRESMP0 && !(value & 0x02))
      {
        uInt16 middle = 4;

        if((myNUSIZ0 & 0x07) == 0x05)
          middle = 8;
        else if((myNUSIZ0 & 0x07) == 0x07)
          middle = 16;

        myPOSM0 = (myPOSP0 + middle) % 160;
        myCurrentM0Mask = &ourMissleMaskTable[myPOSM0 & 0x03][myNUSIZ0 & 0x07][(myNUSIZ0 & 0x30) >> 4][160 - (myPOSM0 & 0xFC)];
      }

      myRESMP0 = value & 0x02;

      if(myENAM0 && !myRESMP0)
        myEnabledObjects |= myM0Bit;
      else
        myEnabledObjects &= ~myM0Bit;

      break;
    }

    case 0x29:    // Reset missle 1 to player 1
    {
      if(myRESMP1 && !(value & 0x02))
      {
        uInt16 middle = 4;

        if((myNUSIZ1 & 0x07) == 0x05)
          middle = 8;
        else if((myNUSIZ1 & 0x07) == 0x07)
          middle = 16;

        myPOSM1 = (myPOSP1 + middle) % 160;
        myCurrentM1Mask = &ourMissleMaskTable[myPOSM1 & 0x03][myNUSIZ1 & 0x07][(myNUSIZ1 & 0x30) >> 4][160 - (myPOSM1 & 0xFC)];
      }

      myRESMP1 = value & 0x02;

      if(myENAM1 && !myRESMP1)
        myEnabledObjects |= myM1Bit;
      else
        myEnabledObjects &= ~myM1Bit;
      break;
    }

    case 0x2A:    // Apply horizontal motion (HMOVE)
    {
      // Figure out what cycle we're at
      //Int32 x = ((clock - myClockWhenFrameStarted) % 228) / 3;
      Int32 x = (gSystemCycles - myCyclesWhenFrameStarted) % 76;

      // See if we need to enable the HMOVE blank bug
      if(myCartInfo.hBlankZero)
      {
          myHMOVEBlankEnabled = ourHMOVEBlankEnableCycles[x];
      }

      Int8 motion = ourCompleteMotionTable[x][myHMP0];
      if (motion != 0)
      {
          myPOSP0 += motion;
          if(myPOSP0 >= 160) myPOSP0 -= 160;
          else if(myPOSP0 < 0) myPOSP0 += 160;
          myCurrentP0Mask = &ourPlayerMaskTable[myPOSP0 & 0x03][0][myNUSIZ0 & 0x07][160 - (myPOSP0 & 0xFC)];
      }

      motion = ourCompleteMotionTable[x][myHMP1];
      if (motion != 0)
      {
          myPOSP1 += motion;
          if(myPOSP1 >= 160) myPOSP1 -= 160;
          else if(myPOSP1 < 0) myPOSP1 += 160;
          myCurrentP1Mask = &ourPlayerMaskTable[myPOSP1 & 0x03][0][myNUSIZ1 & 0x07][160 - (myPOSP1 & 0xFC)];
      }

      motion = ourCompleteMotionTable[x][myHMM0];
      if (motion != 0)
      {
          myPOSM0 += motion;
          if(myPOSM0 >= 160) myPOSM0 -= 160;
          else if(myPOSM0 < 0) myPOSM0 += 160;
          myCurrentM0Mask = &ourMissleMaskTable[myPOSM0 & 0x03][myNUSIZ0 & 0x07][(myNUSIZ0 & 0x30) >> 4][160 - (myPOSM0 & 0xFC)];
          // Disable TIA M0 "bug" used for stars in Cosmic ark
          myM0CosmicArkMotionEnabled = false;
      }

      motion = ourCompleteMotionTable[x][myHMM1];
      if (motion != 0)
      {
          myPOSM1 += motion;
          if(myPOSM1 >= 160) myPOSM1 -= 160;
          else if(myPOSM1 < 0) myPOSM1 += 160;
          myCurrentM1Mask = &ourMissleMaskTable[myPOSM1 & 0x03][myNUSIZ1 & 0x07][(myNUSIZ1 & 0x30) >> 4][160 - (myPOSM1 & 0xFC)];
      }

      motion = ourCompleteMotionTable[x][myHMBL];
      if (motion != 0)
      {
          myPOSBL += motion;
          if(myPOSBL >= 160) myPOSBL -= 160;
          else if(myPOSBL < 0) myPOSBL += 160;
          myCurrentBLMask = &ourBallMaskTable[myPOSBL & 0x03][(myCTRLPF & 0x30) >> 4][160 - (myPOSBL & 0xFC)];
      }

      // Remember what clock HMOVE occured at
      myLastHMOVEClock = clock;

      break;
    }

    case 0x2b:    // Clear horizontal motion registers
    {
      // Should we enabled TIA M0 "bug" used for stars in Rabbit Transit?
      if((clock == (myLastHMOVEClock + 20 * 3)) && (myHMM0 == 7))
      {
        myM0CosmicArkMotionEnabled = true;
        myM0CosmicArkCounter = 0;
      }
      myHMP0 = 0;
      myHMP1 = 0;
      myHMM0 = 0;
      myHMM1 = 0;
      myHMBL = 0;
      break;
    }

    case 0x2c:    // Clear collision latches
    {
      myCollision = 0;
      break;
    }

    default:
    {
      break;
    }
  }
}

// End of TIA.cpp

uInt8 (*TIA::ourBallMaskTable)[4][320] = NULL;
uInt8 *TIA::ourDisabledMaskTable = NULL;
uInt8 (*TIA::ourMissleMaskTable)[8][4][320] = NULL;
uInt8 (*TIA::ourPlayerMaskTable)[2][8][320] = NULL;
Int8 (*TIA::ourPlayerPositionResetWhenTable)[160][160] = NULL;
const uInt8 TIA::ourHMOVEBlankEnableCycles[76] = {
  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,   // 00
  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,   // 10
  true,  false, false, false, false, false, false, false, false, false,  // 20
  false, false, false, false, false, false, false, false, false, false,  // 30
  false, false, false, false, false, false, false, false, false, false,  // 40
  false, false, false, false, false, false, false, false, false, false,  // 50
  false, false, false, false, false, false, false, false, false, false,  // 60
  false, false, false, false, false, true                                // 70
};

const Int8 TIA::ourCompleteMotionTable[76][16] = {
  { 0, -1, -2, -3, -4, -5, -6, -7,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -3, -4, -5, -6, -7,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -3, -4, -5, -6, -7,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -3, -4, -5, -6, -7,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -3, -4, -5, -6, -6,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -3, -4, -5, -5, -5,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -3, -4, -5, -5, -5,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -3, -4, -4, -4, -4,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -3, -3, -3, -3, -3,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -2, -2, -2, -2, -2,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -2, -2, -2, -2, -2,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -1, -1, -1, -1, -1, -1,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0,  0,  0,  0,  0,  0,  0,  0,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 1,  1,  1,  1,  1,  1,  1,  1,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 1,  1,  1,  1,  1,  1,  1,  1,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 2,  2,  2,  2,  2,  2,  2,  2,  8,  7,  6,  5,  4,  3,  2,  2}, // HBLANK
  { 3,  3,  3,  3,  3,  3,  3,  3,  8,  7,  6,  5,  4,  3,  3,  3}, // HBLANK
  { 4,  4,  4,  4,  4,  4,  4,  4,  8,  7,  6,  5,  4,  4,  4,  4}, // HBLANK
  { 4,  4,  4,  4,  4,  4,  4,  4,  8,  7,  6,  5,  4,  4,  4,  4}, // HBLANK
  { 5,  5,  5,  5,  5,  5,  5,  5,  8,  7,  6,  5,  5,  5,  5,  5}, // HBLANK
  { 6,  6,  6,  6,  6,  6,  6,  6,  8,  7,  6,  6,  6,  6,  6,  6}, // HBLANK
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0, -1, -2,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0, -1, -2, -3,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0, -1, -2, -3,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0, -1, -2, -3, -4,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0, -1, -2, -3, -4, -5,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0, -1, -2, -3, -4, -5, -6,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0, -1, -2, -3, -4, -5, -6,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0, -1, -2, -3, -4, -5, -6, -7,  0,  0,  0,  0,  0,  0,  0,  0},
  {-1, -2, -3, -4, -5, -6, -7, -8,  0,  0,  0,  0,  0,  0,  0,  0},
  {-2, -3, -4, -5, -6, -7, -8, -9,  0,  0,  0,  0,  0,  0,  0, -1},
  {-2, -3, -4, -5, -6, -7, -8, -9,  0,  0,  0,  0,  0,  0,  0, -1},
  {-3, -4, -5, -6, -7, -8, -9,-10,  0,  0,  0,  0,  0,  0, -1, -2},
  {-4, -5, -6, -7, -8, -9,-10,-11,  0,  0,  0,  0,  0, -1, -2, -3},
  {-5, -6, -7, -8, -9,-10,-11,-12,  0,  0,  0,  0, -1, -2, -3, -4},
  {-5, -6, -7, -8, -9,-10,-11,-12,  0,  0,  0,  0, -1, -2, -3, -4},
  {-6, -7, -8, -9,-10,-11,-12,-13,  0,  0,  0, -1, -2, -3, -4, -5},
  {-7, -8, -9,-10,-11,-12,-13,-14,  0,  0, -1, -2, -3, -4, -5, -6},
  {-8, -9,-10,-11,-12,-13,-14,-15,  0, -1, -2, -3, -4, -5, -6, -7},
  {-8, -9,-10,-11,-12,-13,-14,-15,  0, -1, -2, -3, -4, -5, -6, -7},
  { 0, -1, -2, -3, -4, -5, -6, -7,  8,  7,  6,  5,  4,  3,  2,  1}  // HBLANK
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const uInt32 TIA::ourNTSCPalette[256] =
{
  0x000000, 0, 0x4a4a4a, 0, 0x6f6f6f, 0, 0x8e8e8e, 0,
  0xaaaaaa, 0, 0xc0c0c0, 0, 0xd6d6d6, 0, 0xececec, 0,
  0x484800, 0, 0x69690f, 0, 0x86861d, 0, 0xa2a22a, 0,
  0xbbbb35, 0, 0xd2d240, 0, 0xe8e84a, 0, 0xfcfc54, 0,
  0x7c2c00, 0, 0x904811, 0, 0xa26221, 0, 0xb47a30, 0,
  0xc3903d, 0, 0xd2a44a, 0, 0xdfb755, 0, 0xecc860, 0,
  0x901c00, 0, 0xa33915, 0, 0xb55328, 0, 0xc66c3a, 0,
  0xd5824a, 0, 0xe39759, 0, 0xf0aa67, 0, 0xfcbc74, 0,
  0x940000, 0, 0xa71a1a, 0, 0xb83232, 0, 0xc84848, 0,
  0xd65c5c, 0, 0xe46f6f, 0, 0xf08080, 0, 0xfc9090, 0,
  0x840064, 0, 0x97197a, 0, 0xa8308f, 0, 0xb846a2, 0,
  0xc659b3, 0, 0xd46cc3, 0, 0xe07cd2, 0, 0xec8ce0, 0,
  0x500084, 0, 0x68199a, 0, 0x7d30ad, 0, 0x9246c0, 0,
  0xa459d0, 0, 0xb56ce0, 0, 0xc57cee, 0, 0xd48cfc, 0,
  0x140090, 0, 0x331aa3, 0, 0x4e32b5, 0, 0x6848c6, 0,
  0x7f5cd5, 0, 0x956fe3, 0, 0xa980f0, 0, 0xbc90fc, 0,
  0x000094, 0, 0x181aa7, 0, 0x2d32b8, 0, 0x4248c8, 0,
  0x545cd6, 0, 0x656fe4, 0, 0x7580f0, 0, 0x8490fc, 0,
  0x001c88, 0, 0x183b9d, 0, 0x2d57b0, 0, 0x4272c2, 0,
  0x548ad2, 0, 0x65a0e1, 0, 0x75b5ef, 0, 0x84c8fc, 0,
  0x003064, 0, 0x185080, 0, 0x2d6d98, 0, 0x4288b0, 0,
  0x54a0c5, 0, 0x65b7d9, 0, 0x75cceb, 0, 0x84e0fc, 0,
  0x004030, 0, 0x18624e, 0, 0x2d8169, 0, 0x429e82, 0,
  0x54b899, 0, 0x65d1ae, 0, 0x75e7c2, 0, 0x84fcd4, 0,
  0x004400, 0, 0x1a661a, 0, 0x328432, 0, 0x48a048, 0,
  0x5cba5c, 0, 0x6fd26f, 0, 0x80e880, 0, 0x90fc90, 0,
  0x143c00, 0, 0x355f18, 0, 0x527e2d, 0, 0x6e9c42, 0,
  0x87b754, 0, 0x9ed065, 0, 0xb4e775, 0, 0xc8fc84, 0,
  0x303800, 0, 0x505916, 0, 0x6d762b, 0, 0x88923e, 0,
  0xa0ab4f, 0, 0xb7c25f, 0, 0xccd86e, 0, 0xe0ec7c, 0,
  0x482c00, 0, 0x694d14, 0, 0x866a26, 0, 0xa28638, 0,
  0xbb9f47, 0, 0xd2b656, 0, 0xe8cc63, 0, 0xfce070, 0
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const uInt32 TIA::ourPALPalette[256] = {
  0x000000, 0, 0x121212, 0, 0x242424, 0, 0x484848, 0,
  0x6c6c6c, 0, 0x909090, 0, 0xb4b4b4, 0, 0xd8d8d8, 0,
  0x000000, 0, 0x121212, 0, 0x242424, 0, 0x484848, 0,
  0x6c6c6c, 0, 0x909090, 0, 0xb4b4b4, 0, 0xd8d8d8, 0,
  0x1d0f00, 0, 0x3f2700, 0, 0x614900, 0, 0x836b01, 0,
  0xa58d23, 0, 0xc7af45, 0, 0xe9d167, 0, 0xffe789, 0,
  0x002400, 0, 0x004600, 0, 0x216800, 0, 0x438a07, 0,
  0x65ac29, 0, 0x87ce4b, 0, 0xa9f06d, 0, 0xcbff8f, 0,
  0x340000, 0, 0x561400, 0, 0x783602, 0, 0x9a5824, 0,
  0xbc7a46, 0, 0xde9c68, 0, 0xffbe8a, 0, 0xffd0ad, 0,
  0x002700, 0, 0x004900, 0, 0x0c6b0c, 0, 0x2e8d2e, 0,
  0x50af50, 0, 0x72d172, 0, 0x94f394, 0, 0xb6ffb6, 0,
  0x3d0008, 0, 0x610511, 0, 0x832733, 0, 0xa54955, 0,
  0xc76b77, 0, 0xe98d99, 0, 0xffafbb, 0, 0xffd1d7, 0,
  0x001e12, 0, 0x004228, 0, 0x046540, 0, 0x268762, 0,
  0x48a984, 0, 0x6acba6, 0, 0x8cedc8, 0, 0xafffe0, 0,
  0x300025, 0, 0x5f0047, 0, 0x811e69, 0, 0xa3408b, 0,
  0xc562ad, 0, 0xe784cf, 0, 0xffa8ea, 0, 0xffc9f2, 0,
  0x001431, 0, 0x003653, 0, 0x0a5875, 0, 0x2c7a97, 0,
  0x4e9cb9, 0, 0x70bedb, 0, 0x92e0fd, 0, 0xb4ffff, 0,
  0x2c0052, 0, 0x4e0074, 0, 0x701d96, 0, 0x923fb8, 0,
  0xb461da, 0, 0xd683fc, 0, 0xe2a5ff, 0, 0xeec9ff, 0,
  0x001759, 0, 0x00247c, 0, 0x1d469e, 0, 0x3f68c0, 0,
  0x618ae2, 0, 0x83acff, 0, 0xa5ceff, 0, 0xc7f0ff, 0,
  0x12006d, 0, 0x34038f, 0, 0x5625b1, 0, 0x7847d3, 0,
  0x9a69f5, 0, 0xb48cff, 0, 0xc9adff, 0, 0xe1d1ff, 0,
  0x000070, 0, 0x161292, 0, 0x3834b4, 0, 0x5a56d6, 0,
  0x7c78f8, 0, 0x9e9aff, 0, 0xc0bcff, 0, 0xe2deff, 0,
  0x000000, 0, 0x121212, 0, 0x242424, 0, 0x484848, 0,
  0x6c6c6c, 0, 0x909090, 0, 0xb4b4b4, 0, 0xd8d8d8, 0,
  0x000000, 0, 0x121212, 0, 0x242424, 0, 0x484848, 0,
  0x6c6c6c, 0, 0x909090, 0, 0xb4b4b4, 0, 0xd8d8d8, 0,
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const uInt32 TIA::ourNTSCPaletteZ26[256] = {
  0x000000, 0, 0x505050, 0, 0x646464, 0, 0x787878, 0,
  0x8c8c8c, 0, 0xa0a0a0, 0, 0xb4b4b4, 0, 0xc8c8c8, 0,
  0x445400, 0, 0x586800, 0, 0x6c7c00, 0, 0x809000, 0,
  0x94a414, 0, 0xa8b828, 0, 0xbccc3c, 0, 0xd0e050, 0,
  0x673900, 0, 0x7b4d00, 0, 0x8f6100, 0, 0xa37513, 0,
  0xb78927, 0, 0xcb9d3b, 0, 0xdfb14f, 0, 0xf3c563, 0,
  0x7b2504, 0, 0x8f3918, 0, 0xa34d2c, 0, 0xb76140, 0,
  0xcb7554, 0, 0xdf8968, 0, 0xf39d7c, 0, 0xffb190, 0,
  0x7d122c, 0, 0x912640, 0, 0xa53a54, 0, 0xb94e68, 0,
  0xcd627c, 0, 0xe17690, 0, 0xf58aa4, 0, 0xff9eb8, 0,
  0x730871, 0, 0x871c85, 0, 0x9b3099, 0, 0xaf44ad, 0,
  0xc358c1, 0, 0xd76cd5, 0, 0xeb80e9, 0, 0xff94fd, 0,
  0x5d0b92, 0, 0x711fa6, 0, 0x8533ba, 0, 0x9947ce, 0,
  0xad5be2, 0, 0xc16ff6, 0, 0xd583ff, 0, 0xe997ff, 0,
  0x401599, 0, 0x5429ad, 0, 0x683dc1, 0, 0x7c51d5, 0,
  0x9065e9, 0, 0xa479fd, 0, 0xb88dff, 0, 0xcca1ff, 0,
  0x252593, 0, 0x3939a7, 0, 0x4d4dbb, 0, 0x6161cf, 0,
  0x7575e3, 0, 0x8989f7, 0, 0x9d9dff, 0, 0xb1b1ff, 0,
  0x0f3480, 0, 0x234894, 0, 0x375ca8, 0, 0x4b70bc, 0,
  0x5f84d0, 0, 0x7398e4, 0, 0x87acf8, 0, 0x9bc0ff, 0,
  0x04425a, 0, 0x18566e, 0, 0x2c6a82, 0, 0x407e96, 0,
  0x5492aa, 0, 0x68a6be, 0, 0x7cbad2, 0, 0x90cee6, 0,
  0x044f30, 0, 0x186344, 0, 0x2c7758, 0, 0x408b6c, 0,
  0x549f80, 0, 0x68b394, 0, 0x7cc7a8, 0, 0x90dbbc, 0,
  0x0f550a, 0, 0x23691e, 0, 0x377d32, 0, 0x4b9146, 0,
  0x5fa55a, 0, 0x73b96e, 0, 0x87cd82, 0, 0x9be196, 0,
  0x1f5100, 0, 0x336505, 0, 0x477919, 0, 0x5b8d2d, 0,
  0x6fa141, 0, 0x83b555, 0, 0x97c969, 0, 0xabdd7d, 0,
  0x344600, 0, 0x485a00, 0, 0x5c6e14, 0, 0x708228, 0,
  0x84963c, 0, 0x98aa50, 0, 0xacbe64, 0, 0xc0d278, 0,
  0x463e00, 0, 0x5a5205, 0, 0x6e6619, 0, 0x827a2d, 0,
  0x968e41, 0, 0xaaa255, 0, 0xbeb669, 0, 0xd2ca7d, 0
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const uInt32 TIA::ourPALPaletteZ26[256] = {
  0x000000, 0, 0x4c4c4c, 0, 0x606060, 0, 0x747474, 0,
  0x888888, 0, 0x9c9c9c, 0, 0xb0b0b0, 0, 0xc4c4c4, 0,
  0x000000, 0, 0x4c4c4c, 0, 0x606060, 0, 0x747474, 0,
  0x888888, 0, 0x9c9c9c, 0, 0xb0b0b0, 0, 0xc4c4c4, 0,
  0x533a00, 0, 0x674e00, 0, 0x7b6203, 0, 0x8f7617, 0,
  0xa38a2b, 0, 0xb79e3f, 0, 0xcbb253, 0, 0xdfc667, 0,
  0x1b5800, 0, 0x2f6c00, 0, 0x438001, 0, 0x579415, 0,
  0x6ba829, 0, 0x7fbc3d, 0, 0x93d051, 0, 0xa7e465, 0,
  0x6a2900, 0, 0x7e3d12, 0, 0x925126, 0, 0xa6653a, 0,
  0xba794e, 0, 0xce8d62, 0, 0xe2a176, 0, 0xf6b58a, 0,
  0x075b00, 0, 0x1b6f11, 0, 0x2f8325, 0, 0x439739, 0,
  0x57ab4d, 0, 0x6bbf61, 0, 0x7fd375, 0, 0x93e789, 0,
  0x741b2f, 0, 0x882f43, 0, 0x9c4357, 0, 0xb0576b, 0,
  0xc46b7f, 0, 0xd87f93, 0, 0xec93a7, 0, 0xffa7bb, 0,
  0x00572e, 0, 0x106b42, 0, 0x247f56, 0, 0x38936a, 0,
  0x4ca77e, 0, 0x60bb92, 0, 0x74cfa6, 0, 0x88e3ba, 0,
  0x6d165f, 0, 0x812a73, 0, 0x953e87, 0, 0xa9529b, 0,
  0xbd66af, 0, 0xd17ac3, 0, 0xe58ed7, 0, 0xf9a2eb, 0,
  0x014c5e, 0, 0x156072, 0, 0x297486, 0, 0x3d889a, 0,
  0x519cae, 0, 0x65b0c2, 0, 0x79c4d6, 0, 0x8dd8ea, 0,
  0x5f1588, 0, 0x73299c, 0, 0x873db0, 0, 0x9b51c4, 0,
  0xaf65d8, 0, 0xc379ec, 0, 0xd78dff, 0, 0xeba1ff, 0,
  0x123b87, 0, 0x264f9b, 0, 0x3a63af, 0, 0x4e77c3, 0,
  0x628bd7, 0, 0x769feb, 0, 0x8ab3ff, 0, 0x9ec7ff, 0,
  0x451e9d, 0, 0x5932b1, 0, 0x6d46c5, 0, 0x815ad9, 0,
  0x956eed, 0, 0xa982ff, 0, 0xbd96ff, 0, 0xd1aaff, 0,
  0x2a2b9e, 0, 0x3e3fb2, 0, 0x5253c6, 0, 0x6667da, 0,
  0x7a7bee, 0, 0x8e8fff, 0, 0xa2a3ff, 0, 0xb6b7ff, 0,
  0x000000, 0, 0x4c4c4c, 0, 0x606060, 0, 0x747474, 0,
  0x888888, 0, 0x9c9c9c, 0, 0xb0b0b0, 0, 0xc4c4c4, 0,
  0x000000, 0, 0x4c4c4c, 0, 0x606060, 0, 0x747474, 0,
  0x888888, 0, 0x9c9c9c, 0, 0xb0b0b0, 0, 0xc4c4c4, 0
};

const uInt32 TIA::ourNTSCPaletteDS[256] = {
  0x000000, 0x000000, 0x393939, 0x000000,           // 0x
  0x797979, 0x000000, 0xababab, 0x000000,
  0xcdcdcd, 0x000000, 0xe6e6e6, 0x000000,
  0xf2f2f2, 0x000000, 0xffffff, 0x000000,
  0x391701, 0x000000, 0x833008, 0x000000,           // 1x
  0xc85f24, 0x000000, 0xff911d, 0x000000,
  0xffc51d, 0x000000, 0xffd84c, 0x000000,
  0xfff456, 0x000000, 0xffff98, 0x000000,
  0x451904, 0x000000, 0x9f241e, 0x000000,           // 2x
  0xc85122, 0x000000, 0xff811e, 0x000000,
  0xff982c, 0x000000, 0xffc545, 0x000000,
  0xffc66d, 0x000000, 0xffe4a1, 0x000000,
  0x4a1704, 0x000000, 0xb21d17, 0x000000,           // 3x
  0xdf251c, 0x000000, 0xfa5255, 0x000000,
  0xff706e, 0x000000, 0xff8f8f, 0x000000,
  0xffabad, 0x000000, 0xffc7ce, 0x000000,
  0x940000, 0x000000, 0xa71a1a, 0x000000,           // 4x
  0xb83232, 0x000000, 0xc84848, 0x000000,
  0xd65c5c, 0x000000, 0xe46f6f, 0x000000,
  0xf08080, 0x000000, 0xfc9090, 0x000000,
  0x280479, 0x000000, 0x590f90, 0x000000,           // 5x
  0x8839aa, 0x000000, 0xc04adc, 0x000000,
  0xe05eff, 0x000000, 0xf27cff, 0x000000,
  0xff98ff, 0x000000, 0xfeabff, 0x000000,
  0x35088a, 0x000000, 0x500cd0, 0x000000,           // 6x
  0x7945d0, 0x000000, 0xa251d9, 0x000000,
  0xbe60ff, 0x000000, 0xcc77ff, 0x000000,
  0xd790ff, 0x000000, 0xdfaaff, 0x000000,
  0x051e81, 0x000000, 0x082fca, 0x000000,           // 7x
  0x444cde, 0x000000, 0x5a68ff, 0x000000,
  0x7183ff, 0x000000, 0x90a0ff, 0x000000,
  0x9fb2ff, 0x000000, 0xc0cbff, 0x000000,
  0x0c048b, 0x000000, 0x382db5, 0x000000,           // 8x
  0x584fda, 0x000000, 0x6b64ff, 0x000000,
  0x8a84ff, 0x000000, 0x9998ff, 0x000000,
  0xb1aeff, 0x000000, 0xc0c2ff, 0x000000,
  0x1d295a, 0x000000, 0x1d4892, 0x000000,           // 9x
  0x1c71c6, 0x000000, 0x489bd9, 0x000000,
  0x55b6ff, 0x000000, 0x8cd8ff, 0x000000,
  0x9bdfff, 0x000000, 0xc3e9ff, 0x000000,
  0x2f4302, 0x000000, 0x446103, 0x000000,           // Ax
  0x3e9421, 0x000000, 0x57ab3b, 0x000000,
  0x61d070, 0x000000, 0x72f584, 0x000000,
  0x87ff97, 0x000000, 0xadffb6, 0x000000,
  0x0a4108, 0x000000, 0x10680d, 0x000000,           // Bx
  0x169212, 0x000000, 0x1cb917, 0x000000,
  0x21d91b, 0x000000, 0x6ef040, 0x000000,
  0x83ff5b, 0x000000, 0xb2ff9a, 0x000000,
  0x04410b, 0x000000, 0x066611, 0x000000,           // Cx
  0x088817, 0x000000, 0x0baf1d, 0x000000,
  0x86d922, 0x000000, 0x99f927, 0x000000,
  0xb7ff5b, 0x000000, 0xdcff81, 0x000000,
  0x02350f, 0x000000, 0x0c4a1c, 0x000000,           // Dx
  0x4f7420, 0x000000, 0x649228, 0x000000,
  0xa1b034, 0x000000, 0xb2d241, 0x000000,
  0xd6e149, 0x000000, 0xf2ff53, 0x000000,
  0x263001, 0x000000, 0x234005, 0x000000,           // Ex
  0x806931, 0x000000, 0xaf993a, 0x000000,
  0xd5b543, 0x000000, 0xe1cb38, 0x000000,
  0xe3e534, 0x000000, 0xfbff7d, 0x000000,
  0x401a02, 0x000000, 0x702408, 0x000000,           // Fx
  0xab511f, 0x000000, 0xbf7730, 0x000000,
  0xe19344, 0x000000, 0xf9ad58, 0x000000,
  0xffc160, 0x000000, 0xffcb83, 0x000000
};

const uInt32 TIA::ourPALPaletteDS[256] = {
  0x000000, 0x000000, 0x242424, 0x000000,
  0x484848, 0x000000, 0x6d6d6d, 0x000000,
  0x919191, 0x000000, 0xb6b6b6, 0x000000,
  0xdadada, 0x000000, 0xffffff, 0x000000,
  0x000000, 0x000000, 0x242424, 0x000000,
  0x484848, 0x000000, 0x6d6d6d, 0x000000,
  0x919191, 0x000000, 0xb6b6b6, 0x000000,
  0xdadada, 0x000000, 0xffffff, 0x000000,
  0x4a3700, 0x000000, 0x705813, 0x000000,
  0x8c732a, 0x000000, 0xa68d46, 0x000000,
  0xbea767, 0x000000, 0xd4c18b, 0x000000,
  0xeadcb3, 0x000000, 0xfff6de, 0x000000,
  0x284a00, 0x000000, 0x44700f, 0x000000,
  0x5c8c21, 0x000000, 0x74a638, 0x000000,
  0x8cbe51, 0x000000, 0xa6d46e, 0x000000,
  0xc0ea8e, 0x000000, 0xdbffb0, 0x000000,
  0x4a1300, 0x000000, 0x70280f, 0x000000,
  0x8c3d21, 0x000000, 0xa65438, 0x000000,
  0xbe6d51, 0x000000, 0xd4886e, 0x000000,
  0xeaa58e, 0x000000, 0xffc4b0, 0x000000,
  0x004a22, 0x000000, 0x0f703b, 0x000000,
  0x218c52, 0x000000, 0x38a66a, 0x000000,
  0x51be83, 0x000000, 0x6ed49d, 0x000000,
  0x8eeab8, 0x000000, 0xb0ffd4, 0x000000,
  0x4a0028, 0x000000, 0x700f44, 0x000000,
  0x8c215c, 0x000000, 0xa63874, 0x000000,
  0xbe518c, 0x000000, 0xd46ea6, 0x000000,
  0xea8ec0, 0x000000, 0xffb0db, 0x000000,
  0x00404a, 0x000000, 0x0f6370, 0x000000,
  0x217e8c, 0x000000, 0x3897a6, 0x000000,
  0x51afbe, 0x000000, 0x6ec7d4, 0x000000,
  0x8edeea, 0x000000, 0xb0f4ff, 0x000000,
  0x43002c, 0x000000, 0x650f4b, 0x000000,
  0x7e2165, 0x000000, 0x953880, 0x000000,
  0xa6519a, 0x000000, 0xbf6eb7, 0x000000,
  0xd38ed3, 0x000000, 0xe5b0f1, 0x000000,
  0x001d4a, 0x000000, 0x0f3870, 0x000000,
  0x21538c, 0x000000, 0x386ea6, 0x000000,
  0x518dbe, 0x000000, 0x6ea8d4, 0x000000,
  0x8ec8ea, 0x000000, 0xb0e9ff, 0x000000,
  0x37004a, 0x000000, 0x570f70, 0x000000,
  0x70218c, 0x000000, 0x8938a6, 0x000000,
  0xa151be, 0x000000, 0xba6ed4, 0x000000,
  0xd28eea, 0x000000, 0xeab0ff, 0x000000,
  0x00184a, 0x000000, 0x0f2e70, 0x000000,
  0x21448c, 0x000000, 0x385ba6, 0x000000,
  0x5174be, 0x000000, 0x6e8fd4, 0x000000,
  0x8eabea, 0x000000, 0xb0c9ff, 0x000000,
  0x13004a, 0x000000, 0x280f70, 0x000000,
  0x3d218c, 0x000000, 0x5438a6, 0x000000,
  0x6d51be, 0x000000, 0x886ed4, 0x000000,
  0xa58eea, 0x000000, 0xc4b0ff, 0x000000,
  0x00014a, 0x000000, 0x0f1170, 0x000000,
  0x21248c, 0x000000, 0x383aa6, 0x000000,
  0x5153be, 0x000000, 0x6e70d4, 0x000000,
  0x8e8fea, 0x000000, 0xb0b2ff, 0x000000,
  0x000000, 0x000000, 0x242424, 0x000000,
  0x484848, 0x000000, 0x6d6d6d, 0x000000,
  0x919191, 0x000000, 0xb6b6b6, 0x000000,
  0xdadada, 0x000000, 0xffffff, 0x000000,
  0x000000, 0x000000, 0x242424, 0x000000,
  0x484848, 0x000000, 0x6d6d6d, 0x000000,
  0x919191, 0x000000, 0xb6b6b6, 0x000000,
  0xdadada, 0x000000, 0xffffff, 0x000000
};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TIA::TIA(const TIA& c)
    : myConsole(c.myConsole)
{
  assert(false);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TIA& TIA::operator = (const TIA&)
{
  assert(false);

  return *this;
}
