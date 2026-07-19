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
#ifndef CARTRIDGE_HXX
#define CARTRIDGE_HXX

class Cartridge;
class System;

#include "bspf.hxx"
#include "Device.hxx"
#include "System.hxx"

extern uInt8* fast_cart_buffer;

extern PageAccess page_access;
extern uInt32 myCurrentOffset;
extern uInt8 cartDriver;

extern uInt32 myTops[8];
extern uInt32 myBottoms[8];
extern uInt32 myCounters[8];
extern uInt8 myFlags[8];
extern uInt8 myRandomNumber;
extern uInt8 myMusicMode[3];
extern uInt32 myMusicCycles; 
extern uInt32 myCurrentOffset32;
extern uInt16 myCurrentBank;
extern uInt8 bSaveStateXL;
extern uInt8* xl_ram_buffer;

extern char my_filename[];

#define NTSC    0
#define PAL     1


// The following is a simple table mapping games to type's using MD5 values
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
struct CartInfo
{
  char  md5[33];
  char  gameID[7];
  uInt8 banking;
  uInt8 controllerType;
  uInt8 special;
  uInt8 frame_mode;
  uInt8 vblankZero;
  uInt8 hBlankZero;
  uInt8 analogSensitivity;      // 10=1.0
  uInt8 tv_type;                // NTSC or PAL
  uInt8 displayStartScanline;   
  uInt8 displayNumScalines;  
  uInt8 yScale;                // 100 = 100% (smaller numbers squish screen to fit)
  Int8  xOffset;
  Int8  yOffset;
  uInt8 aButton;
  uInt8 bButton;
  uInt8 xButton;
  uInt8 yButton;
  uInt8 soundQuality;
  uInt8 left_difficulty;
  uInt8 right_difficulty;
  uInt8 thumbOptimize;
  uInt8 palette_type;
  uInt8 bus_driver;
  uInt8 clearRAM;
  uInt8 xStretch;
  uInt8 spare5_0;
  uInt8 spare6_0;
  uInt8 spare7_0;
  uInt8 spare8_0;
  uInt8 spare1_1;
  uInt8 spare2_1;
  uInt8 spare3_1;
  uInt8 spare1_FF;
  uInt8 spare2_FF;
  uInt8 spare3_FF;
} __attribute__((aligned(4)));

extern CartInfo myCartInfo;

struct GlobalCartInfo
{
    uInt8                   palette;
    uInt8                   sound;
    uInt8                   global1;
    uInt8                   global2;
    uInt8                   global3;
    uInt8                   global4;
    uInt8                   global5;
    uInt8                   global6;
    uInt8                   global7;
    uInt8                   global8;
};

extern GlobalCartInfo myGlobalCartInfo;

#define MAX_CART_FILE_SIZE   (1024 * 512)            // ROMs can be up to 512K in size. This is equivalent to the Harmony Encore and ensures good future-proofing.
extern uInt8* cart_buffer;

// Difficulty Switch defines
#define DIFF_B           0
#define DIFF_A           1

// Controller defines
#define CTR_LJOY         0     // Left Joystick is used for player 1 (default) + Save Key in Right Jack
#define CTR_RJOY         1     // Right Joystick is used for player 1
#define CTR_PADDLE0      2     // For Paddle Games like Breakout and Kaboom
#define CTR_PADDLE1      3     // A few odd games use the OTHER paddle...sigh...
#define CTR_PADDLE2      4     // Paddle in right port
#define CTR_PADDLE3      5     // Tac-Scan uses the second set of paddles and the other paddle on that side. Double sigh.
#define CTR_DRIVING      6     // For Driving Controller games like Indy500
#define CTR_KEYBOARD0    7     // For keyboard games like Codebreaker
#define CTR_KEYBOARD1    8     // For both keyboards showing at same time
#define CTR_BOOSTER      9     // Omega Race and Thrust+
#define CTR_GENESIS     10     // For Genesis 2-button Controller support + Save Key on right port
#define CTR_QUADTARI    11     // For QuadTari two sticks on left port and Save Key on right port
#define CTR_RAIDERS     12     // Special 2 joystick setup for Raiders of the Lost Ark
#define CTR_STARRAID    13     // Star raiders has Left Joystick and Right Keypad
#define CTR_STARGATE    14     // Defender II, Stargate and Defender Arcade (Hack) need both joysticks
#define CTR_SOLARIS     15     // For Solaris - needs button on 2nd controller
#define CTR_MCA         16     // For missile command arcade - 3 button support
#define CTR_BUMPBASH    17     // For bumper bash (both paddle buttons used)
#define CTR_TWINSTICK   18     // For twin-stick games like Rail Slider

// Various special attributes of games
#define SPEC_NONE        0     // Nothing special to do with this game...
#define SPEC_HAUNTED     1     // Haunted House - fix bug by patching offset 1103's E5 to E9
#define SPEC_AR          4     // AR Carts we must track distinct memory access
#define SPEC_MELTDOWN    5     // Meltdown requires NUSIZ0/1 changes
#define SPEC_BUMPBASH    6     // Bumper Bash requires NUSIZ0/1 changes
#define SPEC_KOOLAID     7     // Patch to avoid collisions (2002 original patch)
#define SPEC_BIGBIRD     8     // Patched for joystick use.
#define SPEC_ALPHABM     9     // Patched for joystick use.
#define SPEC_COOKIEM    10     // Patched for joystick use.
#define SPEC_OLDDPCP    11     // For special "older" DPC+ handling
#define SPEC_DPCPOPT    12     // Optmized ARM Thumb DPC+ handling
#define SPEC_DPCPNOC    13     // Optmized ARM Thumb DPC+ handling with No Collision Handling (Space Rocks, Scramble)
#define SPEC_POLEPOS    14     // Pole Position requires NUSIZ0/1 changes
#define SPEC_WAVEDIRC   15     // Slow down the Wave Direct (for games like Quadrun or Berzerk VE)
#define SPEC_GIJOE      16     // GI Joe needs graphical tweaks to make it look right
#define SPEC_OSCAR      17     // Patched for joystick use

// Various output modes for the LCD
#define MODE_NO          0     // Normal Mode - fastest no blend mode
#define MODE_FF          1     // Flicker Free Mode (blend last 2 frames every other frame which is fairly fast)
#define MODE_BACKG       2     // Flicker Reduce (try using background color grab - helps with Missile Command, Astroblast etc. with shifting backgrounds)
#define MODE_BLACK       3     // Ficker Reduce (using Black background improvement only)

// All of the supported bankswitching schemes
#define BANK_2K          0
#define BANK_4K          1
#define BANK_F4          2
#define BANK_F4SC        3
#define BANK_F6          4
#define BANK_F6SC        5
#define BANK_F8          6
#define BANK_F8SC        7
#define BANK_AR          8
#define BANK_DPC         9
#define BANK_DPCP       10
#define BANK_3E         11
#define BANK_3F         12
#define BANK_E0         13
#define BANK_E7         14
#define BANK_FASC       15
#define BANK_FE         16
#define BANK_CDFJ       17
#define BANK_MB         18
#define BANK_CV         19
#define BANK_UA         20
#define BANK_WD         21
#define BANK_EF         22
#define BANK_EFSC       23
#define BANK_BF         24
#define BANK_BFSC       25
#define BANK_DF         26
#define BANK_DFSC       27
#define BANK_SB         28
#define BANK_FA2        29
#define BANK_TV         30
#define BANK_UASW       31
#define BANK_0840       32
#define BANK_X07        33
#define BANK_CTY        34
#define BANK_3EPLUS     35
#define BANK_WF8        36
#define BANK_JANE       37
#define BANK_03E0       38
#define BANK_0FA0       39

// Analog Sensitivity... 10 = 1.0 and normal... 1.1 is faster and 0.9 is slower
#define ANA0_7        7
#define ANA0_8        8
#define ANA0_9        9
#define ANA1_0       10
#define ANA1_1       11
#define ANA1_2       12
#define ANA1_3       13
#define ANA1_4       14
#define ANA1_5       15
#define ANA1_6       16
#define ANA1_7       17
#define ANA1_8       18
#define ANA1_9       19
#define ANA2_0       20
#define ANA2_5       25


// Some button mapping defines...
#define BUTTON_FIRE         0
#define BUTTON_JOY_UP       1
#define BUTTON_JOY_DOWN     2
#define BUTTON_JOY_LEFT     3
#define BUTTON_JOY_RIGHT    4
#define BUTTON_AUTOFIRE     5
#define BUTTON_SHIFT_UP     6
#define BUTTON_SHIFT_DN     7

// Sound settings
#define SOUND_MUTE          0
#define SOUND_10KHZ         1
#define SOUND_15KHZ         2
#define SOUND_20KHZ         3
#define SOUND_30KHZ         4
#define SOUND_WAVE          5


/**
  A cartridge is a device which contains the machine code for a 
  game and handles any bankswitching performed by the cartridge.
 
  @author  Bradford W. Mott
  @version $Id: Cart.hxx,v 1.2 2003/02/17 04:59:54 bwmott Exp $
*/
class Cartridge : public Device
{
  public:
    /**
      Create a new cartridge object allocated on the heap.  The
      type of cartridge created depends on the properties object.

      @param image A pointer to the ROM image
      @param size The size of the ROM image 
      @param properties The properties associated with the game
      @return Pointer to the new cartridge object allocated on the heap
    */
    static Cartridge* create(const uInt8* image, uInt32 size);

  public:
    /**
      Create a new cartridge
    */
    Cartridge();
 
    /**
      Destructor
    */
    virtual ~Cartridge();
    
  private:
    /**
      Try to auto-detect the bankswitching type of the cartridge

      @param image A pointer to the ROM image
      @param size The size of the ROM image 
      @return The "best guess" for the cartridge type
    */
    static uInt8 autodetectType(const uInt8* image, uInt32 size);

    /**
      Utility method used by isProbably3F and isProbably3E
    */
    static int searchForBytes(const uInt8* image, uInt32 size, uInt8 byte1, uInt8 byte2);
    static int searchForBytes3(const uInt8* image, uInt32 size, uInt8 byte1, uInt8 byte2, uInt8 byte3);    
    static int searchForBytes4(const uInt8* image, uInt32 size, uInt8 byte1, uInt8 byte2, uInt8 byte3, uInt8 byte4);
    static int searchForBytes5(const uInt8* image, uInt32 size, uInt8 byte1, uInt8 byte2, uInt8 byte3, uInt8 byte4, uInt8 byte5);
    
    /**
      Returns true if the image is probably a SuperChip (256 bytes RAM)
    */
    static bool isProbablySC(const uInt8* image, uInt32 size);
    
    // Returns true if probalby FE cart
    static bool isProbablyFE(const uInt8* image, uInt32 size);

    // Returns true if probably UA cart
    static bool isProbablyUA(const uInt8* image, uInt32 size);    

    /**
      Returns true if the image is probably a 3F bankswitching cartridge
    */
    static bool isProbably3F(const uInt8* image, uInt32 size);

    /**
      Returns true if the image is probably a 3E bankswitching cartridge
    */
    static bool isProbably3E(const uInt8* image, uInt32 size);
    
    /**
      Returns true if the image is probably a 3E=+ bankswitching cartridge
    */
    static bool isProbably3EPlus(const uInt8* image, uInt32 size);    
    
    /**
      Returns true if the image is probably a DPC+ bankswitching cartridge
    */
    static bool isProbablyDPCplus(const uInt8* image, uInt32 size);
    
    /**
      Returns true if the image is probably a CDF/CDFJ/CDFJ+ bankswitching cartridge
    */
    static bool isProbablyCDF(const uInt8* image, uInt32 size);
    
    /**
      Returns true if the image is probably a EF cartridge
    */
    static bool isProbablyEF(const uInt8* image, uInt32 size);

    /**
      Returns true if the image is probably a EFSC cartridge
    */
    static bool isProbablyEFSC(const uInt8* image, uInt32 size);

    /**
      Returns true if the image is probably a DFSC cartridge
    */
    static bool isProbablyDFSC(const uInt8* image, uInt32 size);
    
    /**
      Returns true if the image is probably a DF cartridge
    */
    static bool isProbablyDF(const uInt8* image, uInt32 size);
    
    /**
      Returns true if the image is probably a BFSC cartridge
    */
    static bool isProbablyBFSC(const uInt8* image, uInt32 size);

    /**
      Returns true if the image is probably a BF cartridge
    */
    static bool isProbablyBF(const uInt8* image, uInt32 size);

    /**
      Returns true if the image is probably a E0 bankswitching cartridge
    */
    static bool isProbablyE0(const uInt8* image, uInt32 size);

    /**
      Returns true if the image is probably a E7 bankswitching cartridge
    */
    static bool isProbablyE7(const uInt8* image, uInt32 size);
    
    /**
      Returns true if the image is probably a 0840 Econobanking cartridge
    */
    static bool isProbably0840(const uInt8* image, uInt32 size);

    /**
      Returns true if the image is probably a 0FA0 Brazillian cartridge
    */
    static bool isProbably0FA0(const uInt8* image, uInt32 size);

    /**
      Returns true if the image is probably a 03E0 Brazillian cartridge
    */
    static bool isProbably03E0(const uInt8* image, uInt32 size);

    /**
      Returns true if the image is probably an FA2 cartridge (32K only)
    */
    static bool isProbablyFA2(const uInt8* image, uInt32 size);
    
  private:
    // Copy constructor isn't supported by cartridges so make it private
    Cartridge(const Cartridge&);

    // Assignment operator isn't supported by cartridges so make it private
    Cartridge& operator = (const Cartridge&);
};
#endif

