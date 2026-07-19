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

#include <assert.h>
#include <string.h>
#include "Cart.hxx"
#include "Cart2K.hxx"
#include "Cart3E.hxx"
#include "Cart3EPlus.hxx"
#include "Cart3F.hxx"
#include "Cart4K.hxx"
#include "CartAR.hxx"
#include "CartCDF.hxx"
#include "CartDPC.hxx"
#include "CartDPCPlus.hxx"
#include "CartE0.hxx"
#include "CartE7.hxx"
#include "CartF4.hxx"
#include "CartF4SC.hxx"
#include "CartF6.hxx"
#include "CartF6SC.hxx"
#include "CartF8.hxx"
#include "CartWF8.hxx"
#include "CartF8SC.hxx"
#include "CartJANE.hxx"
#include "CartFASC.hxx"
#include "CartFE.hxx"
#include "CartMB.hxx"
#include "CartCV.hxx"
#include "CartUA.hxx"
#include "CartWD.hxx"
#include "CartEF.hxx"
#include "CartEFSC.hxx"
#include "CartBF.hxx"
#include "CartBFSC.hxx"
#include "CartDF.hxx"
#include "CartDFSC.hxx"
#include "CartSB.hxx"
#include "CartFA2.hxx"
#include "CartTV.hxx"
#include "Cart0840.hxx"
#include "Cart03E0.hxx"
#include "Cart0FA0.hxx"
#include "CartX07.hxx"
#include "CartCTY.hxx"
#include "MD5.hxx"
#include "M6502.hxx"
#include "../stella_config.h"

extern void dsWarnIncompatibileCart(void);
extern void dsPrintCartType(char *, int);

extern uInt8 tv_type_requested;
uInt8 original_banking_detect = 0;
uInt8 bFoundInDAT = false;
uInt8 bElevatorAgent = false;
uInt8 bSaveStateXL = false;

// The counter registers for the data fetchers

uInt32 myCurrentOffset32    ;
uInt32 myCounters[8]        ;
CartInfo myCartInfo         ;
PageAccess page_access      ;
uInt32 myCurrentOffset      ;
uInt16 myCurrentBank         = 0;

GlobalCartInfo myGlobalCartInfo;

// We can store up to 8k in the fast DTCM memory to give a speed boost... This helps 2k/4k and 8k carts... plus Starpath Supercharger BANK_AR carts
uInt8* fast_cart_buffer = NULL;

// Our cart buffer memory - this can store game ROMs up to 512k
uInt8* cart_buffer = NULL; // Will be allocated in Cartridge::create

// For the few carts that need a big chunk of RAM. This is mostly CDFJ+ and E3/E3+ carts.
uInt8* xl_ram_buffer = NULL; // Will be allocated in Cartridge::create

#define VB 1        // Vertical Blank (1=zero the vertical blank... 0 or !VB is faster but may graphically cause glitching)
#define HB 1        // Horizontal Blank (1=zero the horizontal blank... 0 or !HB is faster but may graphically cause glitching)

const CartInfo table[] =
{
    {"DefaultCart_NTSCxxxxxxxxxxxxxxxx",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Default Cart is 4k, full-scale, L-Joy and nothing special...
    {"DefaultCart_PALxxxxxxxxxxxxxxxxx",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Default PAL Cart is 4k, full-scale, L-Joy and nothing special...
    {"2662d1a4bfdbd234f23957436c4fd5c0",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  5},    // 1775 (v2) (David Weavil).bin
    {"941500699285bb60f782fa82ec7df078",  "1942xx", BANK_F4SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    94,   0,  2},    // 1942 (32k).bin
    {"0db4f4150fecf77e4ce72ca4d04c052f",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // 3-D Tic-Tac-Toe (1980).bin
    {"e3600be9eb98146adafdc12d91323d0f",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   33,    245,    69,   0,  8},    // 3-D Tic-Tac-Toe (1980)(PAL).bin
    {"7b5207e68ee85b16998bea861987c690",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,    93,   0,  8},    // 3-D Tic-Tac-Toe (1980)(PAL).bin
    {"cac9928a84e1001817b223f0cecaa3f2",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // 3-D Genesis (Prototype).bin
    {"fcbdf405f0fc2027b0ea45bb5af94c1a",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // 3-D Ghost Attack (Prototype).bin
    {"60cd61a2dfccb0e2736434f9792c1672",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // 3-D Havoc (Prototype).bin
    {"50c7edc9f9dc0369abcdab3b4efeb5e9",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // 3-D Zapper (Prototype).bin
    {"8dc78243950eaf1b26fe42a732ea0c85",  "CELERY", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0, -5},    // 50celery67.bin
    {"c50fbee08681f15d2d40dbc693d3a837",  "A-STAR", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // A Star.bin
    {"cd5af682685cfecbc25a983e16b9d833",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // A-Team (AKA Saboteur) (1984).bin
    {"c00734a2233ef683d9b6e622ac97a5c8",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // A-Team (AKA Saboteur) (1984).bin
    {"82ee056af81203af58092ff2e6cf079d",  "AARDVK", BANK_F4SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    89,   0,  1},    // Aardvark (32k).bin
    {"b6d13da9e641b95352f090090e440ce4",  "AARDVK", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Aardvark.bin
    {"09274c3fc1c43bf1e362fda436651fd8",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  22,    225,    71,   0,  0},    // Acid Drop (1992) (NTSC by TJ).bin
    {"17ee23e5da931be82f733917adcb6386",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,    51,   0, 16},    // Acid Drop (1992) (PAL).bin
    {"a1bcbe0bfe6570da2661fc4de2f74e8a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Actionauts (1984).bin
    {"525f2dfc8b21b0186cff2568e0509bfc",  "??????", BANK_FE,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Activision Decathlon (1983) [fixed].bin
    {"ac7c2260378975614192ca2bc3d20e0b",  "??????", BANK_FE,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Activision Decathlon (1983).bin
    {"883258dcd68cefc6cd4d40b1185116dc",  "??????", BANK_FE,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   59,    245,   100,   3,  0},    // Activision Decathlon (1983) (PAL).bin
    {"bf52327c2197d9d2c4544be053caded1",  "??????", BANK_FE,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,   100,   3,  0},    // Activision Decathlon (1983) (PAL).bin
    {"157bddb7192754a45372be196797f284",  "ADVENT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Adventure (1980).bin
    {"e2846af3e4d172b251ab77cbdd01761e",  "ADVENT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Adventure+ (Hack).bin
    {"c58d70eb7f67412aa5fb555a5be41c12",  "ADVENT", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Adventure Enhanced 8K (Hack).bin
    {"988c6401afc0a2cd27a6809cfcb30c2c",  "ADVENT", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Adventure - MisAdventure (Hack).bin
    {"5357037076d437e4016ff842a019918f",  "ADVENT", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Adventure - MisAdventure Flip (Hack).bin
    {"4b27f5397c442d25f0c418ccdacf1926",  "ADVENT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   59,    245,   100,   0,  1},    // Adventure (1980) (PAL).bin
    {"ca4f8c5b4d6fb9d608bb96bc7ebd26c7",  "ADTRON", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  5},    // Adventures of TRON (1982).bin
    {"06cfd57f0559f38b9293adae9128ff88",  "ADTRON", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  PAL,   63,    245,   100,   2,  0},    // Adventures of TRON (1982) (PAL).bin
    {"35be55426c1fec32dfb503b4f0651572",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   42,    245,    70,   0,  1},    // Air Raid (PAL).bin
    {"a9cb638cd2cb2e8e0643d7a67db4281c",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  5},    // Air Raiders (1982).bin
    {"4dbd7e8b30e715efc8d71d215aec7fe7",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  5},    // Air Raiders (1982).bin
    {"cf3a9ada2692bb42f81192897752b912",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   46,    245,   100,   4,  4},    // Air Raiders (1982) (PAL).bin
    {"b438a6aa9d4b9b8f0b2ddb51323b21e4",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   46,    245,   100,   4,  4},    // Air Raiders (1982) (PAL).bin
    {"e0b24c3f40a46cda52e29835ab7ad660",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   46,    245,   100,   4,  4},    // Air Raiders (1982) (PAL).bin
    {"16cb43492987d2f32b423817cdaaf7c4",  "??????", BANK_2K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Air-Sea Battle (1977).bin
    {"98e5e4d5c4dd9a986d30fd62bd2f75ae",  "??????", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Air-Sea Battle (1977).bin
    {"1d1d2603ec139867c1d1f5ddf83093f1",  "??????", BANK_2K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Air-Sea Battle (sears).bin
    {"0c7926d660f903a2d6910c254660c32c",  "??????", BANK_2K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   41,    245,    51,   0,  0},    // Air-Sea Battle (pal).bin
    {"8aad33da907bed78b76b87fceaa838c1",  "??????", BANK_2K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   41,    245,    51,   0,  0},    // Air-Sea Battle (pal).bin
    {"4d77f291dca1518d7d8e47838695f54b",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Airlock (1982).bin
    {"62899430338e0538ee93397867d85957",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,    75,   0,  9},    // Airlock (1982) (PAL).bin
    {"f1a0a23e6464d954e3a9579c4ccd01c8",  "ALIEN0", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Alien (1982).bin
    {"956496f81775de0b69a116a0d1ad41cc",  "ALIEN0", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Alien (1982).bin
    {"7c98626ca4502747b6f95e38d687106a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Alien Greed.bin
    {"4ca94fcd35a70cf1ee573d5a26be56a2",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    95,   0,  4},    // Alien Greed 2.bin
    {"f82e1558c956a9e7037531dd81103111",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    95,   0,  3},    // Alien Greed 3.bin
    {"297a7c0ade563a910bea068fab650925",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    93,   0,  0},    // Alien Greed 4.bin
    {"a36824dcc3290746c0c205966e750b30",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  37,    210,    95,   0,  1},    // AlienRevenge2.bin
    {"a4f741910a8dade004e61be5d5ef5d8b",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // AlienAttack4-8-20.bin
    {"e1a51690792838c5c687da80cd764d78",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Alligator People (1983).bin
    {"3aa47765f6184e64c41d1fa2b0b16ddc",  "??????", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Alligator32.bin
    {"9e01f7f95cb8596765e03b9a36e8e33c",  "??????", BANK_F8,   CTR_KEYBOARD0, SPEC_ALPHABM,   MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Alpha Beam with Ernie (1983).bin
    {"8f5ac5139419c5d49bacc296e342a247",  "??????", BANK_F8,   CTR_KEYBOARD0, SPEC_ALPHABM,   MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Alpha Beam with Ernie (1983).bin
    {"a5d82e469a499a41cfb6e459eba89ad0",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Alpha Beam With Ernie (joystick).bin
    {"df95e4af466c809619299f49ece92365",  "??????", BANK_F8,   CTR_KEYBOARD0, SPEC_ALPHABM,   MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Alpha Beam with Ernie (1983) (PAL).bin
    {"f2d40c70cf3e1d03bc112796315888d9",  "??????", BANK_F8,   CTR_KEYBOARD0, SPEC_ALPHABM,   MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Alpha Beam with Ernie (1983) (PAL).bin
    {"acb7750b4d0c4bd34969802a7deb2990",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Amidar (1982).bin
    {"80dcbe1b55f12be731a224a53ee4ad5f",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Amidar (1982).bin
    {"93b9229fc0ea4fb959d604f83f8f603c",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Amidar (DS Fast Hack 1).bin
    {"6354f9c7588a27109c66905b0405825b",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Amidar (DS Fast Hack 2).bin
    {"709910c2e83361bc4bf8cd0c20c34fbf",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Amidar (1982) (PAL).bin
    {"056f5d886a4e7e6fdd83650554997d0d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Amidar (1982) (PAL).bin
    {"539f3c42c4e15f450ed93cb96ce93af5",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    91,   0,  1},    // Amoeba Jump v1.3.bin
    {"0866e22f6f56f92ea1a14c8d8d01d29c",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // AndroMan on the Moon (1984).bin
    {"428428097f85f74dc5cf0dbe07cf16e0",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Anguna-rc3.bin
    {"35a1d649438d58c438e7da57c2ddefaf",  "??????", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Apollyon_NTSC_0.41.bas.bin
    {"e73838c43040bcbc83e4204a3e72eef4",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Apples and Dolls (CCE).bin
    {"053684f9b444f63c0b26ff181cde341d",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    97,   0,  7},    // ApshaiTEST_v4.bin
    {"f69d4fcf76942fcd9bdf3fd8fde790fb",  "AQUAVE", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    92,   0,  1},    // Aquaventure (CCE).bin
    {"038e1e79c3d4410defde4bfe0b99cc32",  "AQUAVE", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    92,   0,  1},    // Aquaventure (CCE).bin
    {"9aaaadc8220029a6f8d1a87f20f9ee77",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0, -3},    // Arctic Landtran (2008) (lord_mike).bin
    {"c122ffc90865ff306d112853bd035de0",  "??????", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    210,   100,   0, -2},    // AroachInSpaceRC2.bin
    {"2434102f30eeb47792cf0825e368229b",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Arkyology.bin
    {"92b383c6f0f76a619dfafa284f1ec45c",  "??????", BANK_F4SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  38,    210,    96,   0,  0},    // Armageddon_Complex_v2.bas.bin
    {"a7b584937911d60c120677fe0d47f36f",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Armor Ambush (1982).bin
    {"d0af33865512e9b6900714c26db5fa23",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  8},    // Armor Ambush (1982) (PAL).bin
    {"c77c35a6fc3c0f12bf9e8bae48cba54b",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  30,    210,   100,   0,  0},    // Artillery Duel (1983).bin
    {"589c73bbcd77db798cb92a992b4c06c3",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   28,    245,   100,   0, -8},    // Artillery Duel (1983) (PAL).bin
    {"de78b3a064d374390ac0710f95edde92",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    91,   0,  0},    // Assault (AKA Sky Alien) (1983).bin
    {"a428068d3e51498907d97cec40000515",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    91,   0,  0},    // Assault (AKA Sky Alien) (1983).bin
    {"c31a17942d162b80962cb1f7571cd1d5",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   68,    245,    91,   0,  2},    // Assault (AKA Sky Alien) (1983) (PAL).bin
    {"01e60a109a6a67c70d3c0528381d0187",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   68,    245,    91,   0,  2},    // Assault (AKA Sky Alien) (1983) (PAL).bin
    {"89a68746eff7f266bbf08de2483abe55",  "ASTRIX", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Asterix (AKA Taz) (1983).bin
    {"faebcb2ef1f3831b2fc1dbd39d36517c",  "ASTRIX", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   64,    245,   100,   0,  2},    // Asterix (AKA Taz) (1983) (PAL).bin
    {"ccbd36746ed4525821a8083b0d6d2c2c",  "ASTERD", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Asteroids (1981) [no copyright].bin
    {"dd7884b4f93cab423ac471aa1935e3df",  "ASTERD", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Asteroids (1981).bin
    {"ca128e861647bb4c3eec51db5d093761",  "ASTERD", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Asteroids Arcade (hack).bin
    {"3e4b1137433cc1e617b5508619e13063",  "ASTERD", BANK_F8,   CTR_GENESIS,   SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Asteroids (2 button hack).bin
    {"bb5049e4558daade0f87fed69a244c59",  "ASTERD", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   45,    245,    91,   0,  0},    // Asteroids (1981) [no copyright] (PAL).bin
    {"8cf0d333bbe85b9549b1e6b1e2390b8d",  "ASTERD", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   45,    245,    91,   0,  0},    // Asteroids (1981) (PAL).bin
    {"170e7589a48739cfb9cc782cbb0fe25a",  "ASTROB", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  NTSC,  31,    200,   100,   0,  0},    // Astroblast (1982).bin
    {"75169c08b56e4e6c36681e599c4d8cc5",  "ASTROB", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  NTSC,  31,    200,   100,   0,  0},    // Astroblast (1982).bin
    {"4d5f6db55f7f44fd0253258e810bde21",  "ASTROB", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  NTSC,  31,    200,   100,   0,  0},    // Astroblast (BetterBlast).bin
    {"46e9428848c9ea71a4d8f91ff81ac9cc",  "ASTROB", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  PAL,   64,    245,   100,   0,  0},    // Astroblast (PAL).bin
    {"7d24f3054be9950aa263e005581c4786",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    87,   0,  2},    // Astro Chase (prototype).bin
    {"7a27eb22570fd1af0b0fec5cab48db61",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // AstronomerNTSC.bin
    {"8f53a3b925f0fd961d9b8c4d46ee6755",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Astrowar (Unknown).bin
    {"317a4cdbab090dcc996833d07cb40165",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   61,    245,    96,   0,  5},    // Astrowar (Unknown) (PAL).bin
    {"6522717cfd75d1dba252cbde76992090",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   67,    245,    98,   0,  5},    // Astrowar (Unknown) (PAL).bin
    {"f0631c6675033428238408885d7e4fde",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Atari 2600 Test Cart.bin
    {"4edb251f5f287c22efc64b3a2d095504",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Atari VCS Point-of-Purchase ROM (1982).bin
    {"3f540a30fdee0b20aed7288e4a5ea528",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    96,   0,  3},    // Atari Video Cube (1982).bin
    {"9ad36e699ef6f45d9eb6c4cf90475c9f",  "ATLANT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  4},    // Atlantis (1982).bin
    {"0b33252b680b65001e91a411e56e72e9",  "ATLANT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  4},    // Atlantis (1982).bin
    {"41818738ab1745e879024a17784d71f5",  "ATLANT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  4},    // Atlantis (1982).bin
    {"71b193f46c88fb234329855452dfac5b",  "ATLANT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  4},    // Atlantis (1982).bin
    {"3aad0ef62885736a5b8c6ccac0dbe00c",  "ATLANT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  4},    // Atlantis (1982).bin
    {"a1403fef01641dcd3980cac9f24d63f9",  "ATLANT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  4},    // Atlantis (1982).bin
    {"18b476a34ce5e6db2c032029873ac39b",  "ATLANT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  4},    // Atlantis (1982).bin
    {"4cabc895ea546022c2ecaa5129036634",  "ATLANT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  4},    // Atlantis (1982).bin
    {"826481f6fc53ea47c9f272f7050eedf7",  "ATLANT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  4},    // Atlantis II (1982).bin
    {"3d2367b2b09c28f1659c082bb46a7334",  "ATLANT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   65,    245,   100,   0,  0},    // Atlantis (1982) (PAL).bin
    {"071f84d10b343c7c05ce3e32af631687",  "ATLANT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   43,    245,   100,   0,  0},    // Atlantis (1982) (PAL).bin
    {"f240ba9f8092d2e8a4c7d82c554bf509",  "ATLANT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   65,    245,   100,   0,  0},    // Atlantis (1982) (PAL).bin
    {"6cea35ded079863a846159c3a1101cc7",  "ATLANT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   59,    245,   100,   0,  0},    // Atlantis (1982) (PAL).bin
    {"c4bbbb0c8fe203cbd3be2e318e55bcc0",  "ATLANT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   65,    245,   100,   0,  0},    // Atlantis (1982) (PAL).bin
    {"53e49ebd522c299739b8e22c4de7ea36",  "??????", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Atomic Meltdown.bas.bin
    {"2cc3049b7feb8e92f1870f1972629757",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Atom Smasher (1983).bin
    {"6ceb7d6a54e9a5e62d26874d1cc88dbc",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Atom Smasher (1983).bin
    {"5f791d93ac95bdd8a691a65d665fb436",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Atom Smasher (1983).bin
    {"14d8bf013eed9edd76e55b86a27709d8",  "??????", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    210,   100,   0,  0},    // Ature (2010) (beoran).bin
    {"4eb7b733de3e61184341f46a24f8e489",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  36,    210,   100,   0,  0},    // Avalanche.bin
    {"b2294cc6bd6f2118633e0bf6d4bebd18",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  36,    210,   100,   0,  0},    // Avalanche_demo_final_NTSC.bin
    {"cb81972e2cd9b175ded45d7f0892da42",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // AVCSTec Challenge.bin
    {"ddd2e0aa26a1c3f3240d55682b245f95",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // bBlocks.bin
    {"5b124850de9eea66781a50b2e9837000",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  25,    210,    83,   0,  0},    // Bachelor Party (1982).bin
    {"274d17ccd825ef9c728d68394b4569d2",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  25,    210,    83,   0,  0},    // Bachelorette Party (1982).bin
    {"0f34f9158b4b85707d465a06d9b270bf",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  32,    210,   100,   0,  1},    // BackFire.bin
    {"8556b42aa05f94bc29ff39c39b11bff4",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Backgammon (1979).bin
    {"85b1bca93e69f13905107cc802a02470",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   51,    245,    73,   0,  0},    // Backgammon (1979) (PAL).bin
    {"19a15ccf2fc4f81f6223150078978e0a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Balloon TripV3.1.bin
    {"00ce0bdd43aed84a983bef38fe7f5ee3",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Bank Heist (1983).bin
    {"a6ed8d72ed691fd3aad5b6974fa17978",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Bank Heist (1983).bin
    {"83b8c01c72306d60dd9b753332ebd276",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  7},    // Bank Heist (1983) (PAL).bin
    {"e9c71f8cdba6037521c9a3c70819d171",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  7},    // Bank Heist (1983) (PAL).bin
    {"f8240e62d8c0a64a61e19388414e3104",  "BARNST", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  3},    // Barnstorming (1982).bin
    {"5ae73916fa1da8d38ceff674fa25a78a",  "BARNST", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  3},    // Barnstorming (1982).bin
    {"e7dd8c2e6c100044002c1086d02b366e",  "BARNST", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   68,    245,   100,   4,  2},    // Barnstorming (1982) (PAL).bin
    {"9ad362179c2eea4ea115c7640b4b003e",  "BARNST", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   68,    245,   100,   4, 12},    // Barnstorming (1982) (PAL).bin
    {"819aeeb9a2e11deb54e6de334f843894",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Basic Math (1977).bin
    {"a41450333f8dd0e96e5e9f0af3770ae9",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Basic Math (1977) (PAL).bin
    {"5f46d1ff6d7cdeb4b09c39d04dfd50a1",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Basic Math (1977) (PAL).bin
    {"819aeeb9a2e11deb54e6de334f843894",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Basic Math (1977) (PAL).bin
    {"9f48eeb47836cf145a15771775f0767a",  "??????", BANK_4K,   CTR_KEYBOARD1, SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // BASIC Programming (1979).bin
    {"b061e98a4c854a672aadefa233236e51",  "??????", BANK_4K,   CTR_KEYBOARD1, SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // BASIC Programming (1979) (PAL).bin
    {"ab4ac994865fb16ebb85738316309457",  "??????", BANK_2K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Basketball (1978).bin
    {"6588d192d9a8afce27b44271a2072325",  "??????", BANK_2K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Basketball (1978).bin
    {"218c0fe53dfaaa37f3c823f66eafd3fc",  "??????", BANK_2K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Basketball (1978) (PAL).bin
    {"77be57d872e3f5b7ecf8d19d97f73281",  "??????", BANK_2K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Basketball (1978) (PAL).bin
    {"1228c01cd3c4b9c477540c5adb306d2a",  "??????", BANK_2K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Basketball (1978) (PAL).bin
    {"41f252a66c6301f1e8ab3612c19bc5d4",  "BAZONE", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   4,  0},    // Battlezone (1983).bin
    {"fbe554aa8f759226d251ba6b64a9cce4",  "BAZONE", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   4, 13},    // Battlezone (1983) (PAL).bin
    {"79ab4123a83dc11d468fb2108ea09e2e",  "BEAMRI", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  0},    // Beamrider (1984).bin
    {"05f24a2df386b8881850e22b7f921258",  "BEAMRI", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  2},    // Beamrider Reimagined (Hack).bin
    {"fec0c2e2ab0588ed20c750b58cf3baa3",  "BEAMRI", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   4,  0},    // Beamrider (1984) (PAL).bin
    {"d0b9df57bfea66378c0418ec68cfe37f",  "BEANYB", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Beany Bopper (1982).bin
    {"6a9e0c72fab92df70084eccd9061fdbd",  "BEANYB", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Beany Bopper (1982).bin
    {"e30f3a37032da52d7815b5a409f6d4b4",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Bear Game Demo.bin
    {"e5340edb08a5c76e9b6d8c36dd845e31",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Beast (Release 21060501)
    {"8cbb790efcb98010d60207d29fa503ec",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Beast (Release 21082901)
    {"59e96de9628e8373d1c685f5e57dcf10",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Beat 'Em & Eat 'Em (1982).bin
    {"6c25f58fd184632ca76020f589bb3767",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Beat 'Em & Eat 'Em (1982).bin
    {"ca54de69f7cdf4d7996e86f347129892",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Beat 'Em & Eat 'Em (1982).bin
    {"b4f31ea8a6cc9f1fd4d5585a87c3b487",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0, 11},    // Beat 'Em & Eat 'Em (1982) (PAL).bin
    {"0d22edcb05e5e52f45ddfe171e3c4b50",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Bee Ball.bin
    {"d655f3b4e327d2713f52b2e342bf25c6",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Bell Hopper (2011) (Tjoppen) (NTSC).bin
    {"ee6665683ebdb539e89ba620981cb0f6",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Berenstain Bears (1983).bin
    {"b8ed78afdb1e6cfe44ef6e3428789d5f",  "BERMUD", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Bermuda Triangle (1982).bin
    {"36c993dc328933e4dd6374a8ffe224f4",  "BERMUD", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   47,    245,    51,   0,  0},    // Bermuda Triangle (1982) (PAL).bin
    {"136f75c4dd02c29283752b7e5799f978",  "BERZRK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0, -1},    // Berzerk (1982).bin
    {"fac28963307b6e85082ccd77c88325e7",  "BERZRK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Berzerk (1982).bin
    {"1bd6e676ba7a066d433f7179014acf11",  "BERZRK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Berzerk (Arcade Enhanced Hack).bin
    {"91750d2ce72f22ae70c8826d67c6aecb",  "BERZRK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0, -1},    // Berzerk (Dungeon Of The Mad Mages Hack).bin
    {"68597f3efc332bc141e21077b68cad34",  "BERZRK", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Berzerk (Frosty 2019 Hack).bin
    {"c887d31fd62c32f20ce74c8dc6db6be3",  "BERZRK", BANK_F6,   CTR_LJOY,      SPEC_WAVEDIRC,  MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Berzerk (Arcade Enhanced VE Hack).bin
    {"cd51012a994540075e24809a4f1d259e",  "BERZRK", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Berzerk (Diagonal Shots).bin
    {"8b80a6c398724ba0ca6321e15b27c7c0",  "BERZRK", BANK_F4,   CTR_LJOY,      SPEC_WAVEDIRC,  MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Berzerk Enhanded - World.bin
    {"0805366f1b165a64b6d4df20d2c39d25",  "BERZRK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  6},    // Berzerk (1982) (PAL).bin
    {"490e3cc59d82f85fae817cdf767ea7a0",  "BERZRK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  PAL,   57,    245,   100,   0,  6},    // Berzerk (1982) (PAL).bin
    {"4b205ef73a5779acc5759bde3f6d33ed",  "BERZRK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  PAL,   59,    245,   100,   0,  6},    // Berzerk (1982) (PAL).bin
    {"be41463cd918daef107d249f8cde3409",  "BERZRK", BANK_F6,   CTR_LJOY,      SPEC_WAVEDIRC,  MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Berzerk-VE.bin
    {"3264674dd39b20abb3eb508973ea2d9b",  "BERZRK", BANK_F4,   CTR_LJOY,      SPEC_WAVEDIRC,  MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Doctor Who vs Daleks (Berzerk Hack)
    {"1badbf0d3cb5abf7cf29233120dc14cc",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // BiFrost (RC).bin
    {"1802cc46b879b229272501998c5de04f",  "??????", BANK_F8,   CTR_KEYBOARD0, SPEC_BIGBIRD,   MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Big Bird's Egg Catch (1983).bin
    {"0b17ed42984000da8b727ca46143f87a",  "??????", BANK_F8,   CTR_KEYBOARD0, SPEC_BIGBIRD,   MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Big Bird's Egg Catch (1983).bin
    {"151fa3218d8d7600114eb5bcd79c85cb",  "??????", BANK_F8,   CTR_KEYBOARD0, SPEC_BIGBIRD,   MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Big Bird's Egg Catch (1983).bin
    {"d22a401f0b20808196f538cc43786459",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Big Bird's Egg Catch (joystick).bin
    {"f283cc294ece520c2badf9da20cfc025",  "??????", BANK_F8,   CTR_KEYBOARD0, SPEC_BIGBIRD,   MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Big Bird's Egg Catch (1983) (PAL).bin
    {"f0541d2f7cda5ec7bab6d62b6128b823",  "??????", BANK_F8,   CTR_KEYBOARD1, SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Bionic Breakthrough (Prototype).bin
    {"ffcb629e39e0455b37626ca1cf360db2",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // BirdandbeansV2.1.bin
    {"a6239810564638de7e4c54e66b3014e4",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Birthday Mania.bin
    {"0a981c03204ac2b278ba392674682560",  "??????", BANK_2K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Blackjack (1977).bin
    {"ff7627207e8aa03730c35c735a82c26c",  "??????", BANK_2K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   66,    245,   100,   0,  0},    // Blackjack (1977) (PAL).bin
    {"ff3bd0c684f7144aeaa18758d8281a78",  "??????", BANK_2K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Blackjack (1977) (PAL).bin
    {"575c0fb61e66a31d982c95c9dea6865c",  "??????", BANK_2K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Blackjack (1977) (PAL).bin
    {"42dda991eff238d26669fd33e353346d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Blinky Goes Up.bin
    {"ae4adeb98fa92abaf8ac8d3cf1ddddcf",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // BLiP Football.bin
    {"33d68c3cd74e5bc4cf0df3716c5848bc",  "BLUEPR", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    96,   0,  0},    // Blueprint (1983).bin
    {"2432f33fd278dea5fe6ae94073627fcc",  "BLUEPR", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,    96,   0,  0},    // Blueprint (1983) (PAL).bin
    {"968efc79d500dce52a906870a97358ab",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // BMX Air Master (1990).bin
    {"4f89b897444e7c3b36aed469b8836839",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,    97,   4, 11},    // BMX Air Master (1990) (PAL).bin
    {"7c757bb151269b2a626c907a22f5dae7",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,    97,   4, 11},    // BMX Air Master (1990) (PAL).bin
    {"521f4dd1eb84a09b2b19959a41839aad",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  30,    210,    94,   0,  0},    // Bobby Is Going Home (1983).bin
    {"ebcb084a91d41865b2c1915779001ca7",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    210,    95,   0,  0},    // Bobby Is Going Home (1983).bin
    {"f2f59629d7341c97644405daeac08845",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    210,    95,   0,  0},    // Bobby Is Going Home (1983).bin
    {"2f2f9061398a74c80420b99ddecf6448",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    210,    95,   0,  0},    // Bobby Is Going Home (1983).bin
    {"075069ad80cde15eca69e3c98bd66714",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    210,    95,   0,  0},    // Bobby Is Going Home (1983).bin
    {"afe776db50e3378cd6f29c7cdd79104a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  23,    220,    78,   0,  0},    // Bobby Is Going Home (1983) (NTSC by TJ).bin
    {"48e5c4ae4f2d3b62b35a87bca18dc9f5",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   40,    245,    52,   0,  7},    // Bobby Is Going Home (1983) (PAL).bin
    {"2823364702595feea24a3fbee138a243",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   40,    245,    52,   0,  7},    // Bobby Is Going Home (1983) (PAL).bin
    {"3cbdf71bb9fd261fbc433717f547d738",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   41,    245,    52,   0,  7},    // Bobby Is Going Home (1983) (PAL).bin
    {"ce243747bf34a2de366f846b3f4ca772",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   40,    245,    52,   0,  7},    // Bobby Is Going Home (1983) (PAL).bin
    {"80e1410ec98089e0733cc09e584dba4b",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   40,    245,    52,   0,  7},    // Bobby Is Going Home (1983) (PAL).bin
    {"c59633dbebd926c150fb6d30b0576405",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  5},    // Bogey Blaster (1989).bin
    {"a5855d73d304d83ef07dde03e379619f",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  26,    210,    90,   0,  0},    // Boggle (1978).bin
    {"14c2548712099c220964d7f044c59fd9",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Boing! (1983).bin
    {"c471b97446a85304bbac021c57c2cb49",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   45,    245,   100,   0,  0},    // Boing! (1983) (PAL).bin
    {"1eabc038854452bd3f963cb47673f3ac",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Bomb City (final).bin
    {"0ff357ea06b7f4f542cbfdd953eb159e",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  4},    // Bombs Away! (2010) (Steve Engelhardt).bin
    {"ab48c4af46c8b34c3613d210e1206132",  "??????", BANK_3E,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  37,    210,   100,   0,  0},    // Boulder Dash Demo.bin
    {"96662271ae50b6859017bffbdda75525",  "??????", BANK_3E,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  37,    210,   100,   0,  0},    // Boulder Dash Full.bin
    {"594dbc80b93fa5804e0f1368c037331d",  "??????", BANK_FE,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  10},   // Bouncin' Baby Bunnies (Prototype).bin
    {"c9b7afad3bfd922e006a6bfc1d4f3fe7",  "BOWLNG", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Bowling (1979).bin
    {"32ecb5a652eb73d287e883eea751d99c",  "BOWLNG", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Bowling (1979).bin
    {"4d06f72cc3d8934579c11ff8f375c260",  "BOWLNG", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Bowling (1979).bin
    {"969b968383d9f0e9d8ffd1056bcaef49",  "BOWLNG", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,   100,   0,  0},    // Bowling (1979) (PAL).bin
    {"11e7e0d9437ec98fa085284cf16d0eb4",  "BOWLNG", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   60,    245,   100,   0,  0},    // Bowling (1979) (PAL).bin
    {"f69bb58b815a6bdca548fa4d5e0d5a75",  "BOWLNG", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   59,    245,   100,   0,  0},    // Bowling (1979) (PAL).bin
    {"c3ef5c4653212088eda54dc91d787870",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    98,   0,  2},    // Boxing (1980).bin
    {"a8b3ea6836b99bea77c8f603cf1ea187",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    98,   0,  2},    // Boxing (1980).bin
    {"cfb3260c603b0341d49ddfc94051ec10",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    98,   0,  2},    // Boxing (1980).bin
    {"25f2e760cd7f56b88aac88d63757d41b",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,    98,   3, 16},    // Boxing (1980) (PAL).bin
    {"fd6e507b5df68beeeddeaf696b6828fa",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   47,    245,    98,   3, 16},    // Boxing (1980) (PAL).bin
    {"2c45c3eb819a797237820a1816c532eb",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   45,    245,    98,   3, 16},    // Boxing (1980) (PAL).bin
    {"7f07cd2e89dda5a3a90d3ab064bfd1f6",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   46,    245,    98,   3, 16},    // Boxing (1980) (PAL).bin
    {"2c6caeb16c3102308015ca9f0a400365",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_1,  NTSC,  34,    210,   100,   0,  5},    // Bot&Tom2 (NTSC).bin
    {"b966ad0c6252344162add8a3b1f8e3b4",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_1,  PAL,   34,    210,   100,   0,  5},    // Bot&Tom2 (PAL).bin
    {"1cca2197d95c5a41f2add49a13738055",  "??????", BANK_2K,   CTR_KEYBOARD0, SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Brain Games (1978).bin
    {"f280976d69d6e27a48506bd6bad11dcd",  "??????", BANK_2K,   CTR_KEYBOARD0, SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Brain Games (1978) (PAL).bin
    {"f34f08e5eb96e500e851a80be3277a56",  "BRKOUT", BANK_2K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_1,  NTSC,  34,    200,   100,   0,  0},    // Breakout (1978).bin
    {"4df6124093ccb4f0b6c26a719f4b7706",  "BRKOUT", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_1,  NTSC,  34,    200,   100,   0,  0},    // Breakout (1978).bin
    {"9a25b3cfe2bbb847b66a97282200cca2",  "BRKOUT", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_1,  NTSC,  34,    200,   100,   0,  0},    // Breakout (1978).bin
    {"c5fe45f2734afd47e27ca3b04a90213c",  "BRKOUT", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_1,  NTSC,  34,    200,   100,   0,  0},    // Breakout (1978).bin
    {"6c76fe09aa8b39ee52035e0da6d0808b",  "BRKOUT", BANK_2K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_1,  PAL,   52,    245,    60,   0,  0},    // Breakout (1978) (PAL).bin
    {"413c925c5fdcea62842a63a4c671a5f2",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Bridge [fixed] (1980).bin
    {"cfd6a8b23d12b0462baf6a05ef347cd8",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Bridge (1980).bin
    {"f8b2a6a4d73ebff10d805a9b59041986",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  9},    // Bridge (1980) (PAL).bin
    {"18a970bea7ac4d29707c8d5cd559d03a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   54,    245,   100,   0,  9},    // Bridge (1980) (PAL).bin
    {"1cf59fc7b11cdbcefe931e41641772f6",  "BUCKRO", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  5},    // Buck Rogers (1983).bin
    {"cd88ef1736497288c4533bcca339f881",  "BUCKRO", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   70,    245,    93,   0,  0},    // Buck Rogers (1983) (PAL).bin
    {"68597264c8e57ada93be3a5be4565096",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Bugs (1982).bin
    {"e61210293b14c9c4ecc91705072c6a7e",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   46,    245,    51,   0,  0},    // Bugs (1982) (PAL).bin
    {"68597264c8e57ada93be3a5be4565096",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   46,    245,    51,   0,  0},    // Bugs (1982) (PAL).bin
    {"fa4404fabc094e3a31fcd7b559cdd029",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  5},    // Bugs Bunny (1983).bin
    {"a3486c0b8110d9d4b1db5d8a280723c6",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  5},    // Bugs Bunny (1983).bin
    {"76f53abbbf39a0063f24036d6ee0968a",  "??????", BANK_E7,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Bump 'n' Jump (1983) [alt].bin
    {"ab2cfcaad3daaf673b2b14fdbb8dac33",  "??????", BANK_E7,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Bump 'n' Jump (1983).bin
    {"d6d7e829221a452792937acd9fda8b81",  "??????", BANK_E7,   CTR_GENESIS,   SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Bump 'n' Jump (2 Button Hack).bin
    {"9295570a141cdec18074c55dc7229d08",  "??????", BANK_E7,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  4},    // Bump 'n' Jump (1983) (PAL).bin
    {"aa1c41f86ec44c0a44eb64c332ce08af",  "??????", BANK_4K,   CTR_BUMPBASH,  SPEC_BUMPBASH,  MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   1,  0},    // Bumper Bash (1983).bin
    {"16ee443c990215f61f7dd1e55a0d2256",  "??????", BANK_4K,   CTR_BUMPBASH,  SPEC_BUMPBASH,  MODE_NO,    VB,   HB,  ANA1_0,  PAL,   81,    245,   100,   1,  0},    // Bumper Bash (1983) (PAL).bin
    {"1bf503c724001b09be79c515ecfcbd03",  "??????", BANK_4K,   CTR_BUMPBASH,  SPEC_BUMPBASH,  MODE_NO,    VB,   HB,  ANA1_0,  PAL,   79,    245,   100,   1,  0},    // Bumper Bash (1983) (PAL).bin
    {"0443cfa9872cdb49069186413275fa21",  "??????", BANK_E7,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    210,   100,   0,  0},    // BurgerTime (1983).bin
    {"19d6956ff17a959c48fcd8f4706a848d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  25,    210,    95,   0,  1},    // Burning Desire (1982).bin
    {"b42df8d92e3118dc594cecd575f515d7",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,    95,   0,  5},    // Burning Desire (1982) (PAL).bin
    {"66fcf7643d554f5e15d4d06bab59fe70",  "CABAGE", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  6},    // Cabbage Patch Kids (1984).bin
    {"f6b5ebb65cbb2981af4d546c470629d7",  "CABAGE", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  6},    // Cabbage Patch Kids (1984).bin
    {"b7e0d5042dad1bb0e30050c414cde1f8",  "CABAGE", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  7},    // Cabbage Patch Kids (Athletic Land Hack).bin
    {"455b91372a858f2e7513babbbaf847ed",  "CABAGE", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  7},    // Cabbage Patch Kids (Improved Hack).bin
    {"3a67bdeb7252a573123110167435e3bb",  "CABAGE", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  7},    // Cabbage Patch Kids (Red Hair Hack).bin
    {"7f6533386644c7d6358f871666c86e79",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Cakewalk (1983).bin
    {"0060a89b4c956b9c703a59b181cb3018",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  8},    // Cakewalk (1983) (PAL).bin
    {"9ab72d3fd2cc1a0c9adb504502579037",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  4},    // California Games (1988).bin
    {"8068e07b484dfd661158b3771d6621ca",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   4,  9},    // California Games (1988) (PAL).bin
    {"9d652c50a0f101d8ea071753bddb8f53",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Candy Catcher (2011) (Grant Thienemann).bin
    {"79e151d8ca4fb96d46dfbc685323c0fe",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Cannonhead Clash.bin
    {"feedcc20bc3ca34851cd5d9e38aa2ca6",  "??????", BANK_2K,   CTR_PADDLE1,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Canyon Bomber (1979).bin
    {"457f4ad2cda5f4803f122508bfbde3f5",  "??????", BANK_2K,   CTR_PADDLE1,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   68,    245,   100,   0,  2},    // Canyon Bomber (1979) (PAL).bin
    {"151c33a71b99e6bcffb34b43c6f0ec23",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Care Bears (1983).bin
    {"028024fb8e5e5f18ea586652f9799c96",  "CARNIV", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  26,    214,    80,   0,  0},    // Carnival (1982).bin
    {"7dd9c5284422f729066ab22a284c8283",  "CARNIV", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  26,    214,    80,   0,  0},    // Carnival (1982).bin
    {"3d6fc7a19be76d808aa233415cb583fc",  "CARNIV", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  26,    214,    80,   0,  0},    // Carnival (1982).bin
    {"de29e46dbea003c3c09c892d668b9413",  "CARNIV", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   48,    245,    74,   0,  0},    // Carnival (1982) (PAL).bin
    {"540075f657d4b244a1f74da1b9e4bf92",  "CARNIV", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   50,    245,    75,   0, -1},    // Carnival (1982) (PAL).bin
    {"2516f4f4b811ede4ecf6fbeb5d54a299",  "CARNIV", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   48,    245,    74,   0,  0},    // Carnival (1982) (PAL).bin
    {"8ed5a746c59571feb255eaa7d6d0cf98",  "CARNIV", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   50,    245,    79,   0,  0},    // Carnival (1982) (PAL).bin
    {"e3dccd660e5715bab3ca6c647f20e993",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Carrot Kingdom (8a fix).bin
    {"b816296311019ab69a21cb9e9e235d12",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA0_7,  NTSC,  34,    210,   100,   0,  4},    // Casino (1979).bin
    {"2bc26619e31710a9884c110d8430c1da",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  7},    // Casino (1979) (PAL).bin
    {"1d88b15eb93fdb9f3c3c4b77f30d5241",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  26,    210,    87,   0,  5},    // Castle54.bin
    {"76f66ce3b83d7a104a899b4b3354a2f2",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  33,    210,    92,   0,  0},    // Cat Trax (1983).bin
    {"d071d2ec86b9d52b585cc0382480b351",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  33,    210,    92,   0,  0},    // Cat Trax (1983).bin
    {"2fab2840daac5ea2abd989d1f46fe192",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,    92,   0,  0},    // Catch_A_Coin_v4.bin
    {"9e192601829f5f5c2d3b51f8ae25dbe5",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    95,   0,  9},    // Cathouse Blues (1982).bin
    {"3002e64a33a744487272be26d6069b3a",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  5},    // Cave 1K (2004) (Thomas Jentzsch).bin
    {"2f21515ea4c7eb39a4b4ca7b25d759ba",  "??????", BANK_F4SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  39,    210,    96,   0, -3},    // Cave In.bin
    {"1cedebe83d781cc22e396383e028241a",  "??????", BANK_F4SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  5},    // Cave In.bin
    {"049b33b03e7928af596c9d683f587475",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Caverns.bin
    {"4bd38b84e9f46f39367dae0af849b13d",  "??????", BANK_F4SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    94,   0,  5},    // Caverun_update2.bin
    {"91c2098e88a6b13f977af8c003e0bca5",  "CENTIP", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Centipede (1982).bin
    {"126f7f64b7b00e25dcf5e3710b4cf8b8",  "CENTIP", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Centipede (1982).bin
    {"17d000a2882f9fdaa8b4a391ad367f00",  "CENTIP", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   61,    245,   100,   0,  5},    // Centipede (1982) (PAL).bin
    {"ce09df4f125e49a8239c954e22fe8adb",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Chakdust - NTSC Cart.bin
    {"73158ea51d77bf521e1369311d26c27b",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,    95,   0,  3},    // Challenge (Zellers).bin
    {"4311a4115fb7bc68477c96cf44cebacf",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,    95,   0,  3},    // Challenge (Zellers).bin
    {"9905f9f4706223dadee84f6867ede8e3",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  PAL,   52,    245,    95,   0,  0},    // Challenge (HES) (PAL).bin
    {"1e0ef01e330e5b91387f75f700ccaf8f",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  PAL,   48,    245,    89,   0,  0},    // Challenge (Quelle) (PAL).bin
    {"5d799bfa9e1e7b6224877162accada0d",  "CNEXAR", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Challenge of.... Nexar (1982).bin
    {"45c4413dd703b9cfea49a13709d560eb",  "CNEXAR", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Challenge of.... Nexar (1982).bin
    {"1da2da7974d2ca73a823523f82f517b3",  "CNEXAR", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  9},    // Challenge of.... Nexar (1982) (PAL).bin
    {"3d9c2fccf8b11630762ff00811c19277",  "CNEXAR", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  9},    // Challenge of.... Nexar (1982) (PAL).bin
    {"c745487828a1a6a743488ecebc55ad44",  "CNEXAR", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  9},    // Challenge of.... Nexar (1982) (PAL).bin
    {"ea7e25ade3fe68f5b786ee0aa82b1fe5",  "CNEXAR", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  9},    // Challenge of.... Nexar (1982) (PAL).bin
    {"690a6049db78b9400c13521646708e9c",  "CNEXAR", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  9},    // Challenge of.... Nexar (1982) (PAL).bin
    {"ace319dc4f76548659876741a6690d57",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Championship Soccer (1980).bin
    {"7a09299f473105ae1ef3ad6f9f2cd807",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   43,    245,    71,   0,  0},    // Championship Soccer (1980) (PAL).bin
    {"1160721a4a98bc0ca5a7b61a91cb8e9a",  "??????", BANK_DPCP, CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  32,    205,   100,   0,  1},    // Chaotic Grill - 2021
    {"78a916081b350b9678255824b9c9ffaa",  "??????", BANK_DPCP, CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  32,    205,   100,   0,  2},    // Chaotic Grill - 2022
    {"b34b470a41ef971832f05586f0e329ce",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Charge_rev1.bin
    {"3e33ac10dcf2dff014bc1decf8a9aea4",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    94,   0,  0},    // Chase the Chuckwagon (1983).bin
    {"3f5a43602f960ede330cd2f43a25139e",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   6,  2},    // Checkers (1980).bin
    {"3d7749fb9c2f91a276dfe494495234c5",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   6,  2},    // Checkers (1980).bin
    {"191ac4eec767358ee3ec3756c120423a",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,  10,  4},    // Checkers (1980) (PAL).bin
    {"bce93984b920e9b56cf24064f740fe78",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,  10,  4},    // Checkers (1980) (PAL).bin
    {"bfbdeafbe085d23d2b143580014a469d",  "CHETRY", BANK_CTY,  CTR_LJOY,      SPEC_WAVEDIRC,  MODE_NO,   !VB,  !HB,  ANA1_0,  NTSC,  34,    210,    82,   0,  1},    // Chetiry_NOEEPROM_NTSC.bin
    {"3b48bc3cec29b63b2f4429f6263d55e8",  "CHETRY", BANK_CTY,  CTR_LJOY,      SPEC_WAVEDIRC,  MODE_NO,   !VB,  !HB,  ANA1_0,  NTSC,  34,    210,    82,   0,  1},    // Chetiry_NTSC.bin
    {"9784290f422e7aeeab4d542318bd9a1f",  "CHETRY", BANK_CTY,  CTR_LJOY,      SPEC_WAVEDIRC,  MODE_NO,   !VB,  !HB,  ANA1_0,  NTSC,  34,    210,    82,   0,  1},    // Chetiry_NTSC_Stella.bin
    {"749fec9918160921576f850b2375b516",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  31,    210,    80,   0,  0},    // China Syndrome (1982).bin
    {"e150f0d14f013a104b032305c0ce23ef",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   50,    245,    80,   0,  2},    // China Syndrome (1982) (PAL).bin
    {"c1cb228470a87beb5f36e90ac745da26",  "CHOPER", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    197,   100,   4,  5},    // Chopper Command (1982).bin
    {"ffdc0eb3543404eb4c353fbdddfa33b6",  "CHOPER", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    197,   100,   4,  5},    // Chopper Command (1982).bin
    {"85a4133f6dcf4180e36e70ad0fca0921",  "CHOPER", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    197,   100,   4,  5},    // Chopper Command (1982).bin
    {"da66d75e4b47fab99733529743f86f4f",  "CHOPER", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    197,   100,   4,  5},    // Chopper Command (1982).bin
    {"1cad3b56cc0e6e858554e46d08952861",  "CHOPER", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    197,   100,   4,  5},    // Chopper Command (1982).bin
    {"f8811d45a9935cca90c62f924712f8e6",  "CHOPER", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    197,   100,   4,  5},    // Chopper Command (1982).bin
    {"3c72ddaf41158fdd66e4f1cb90d4fd29",  "CHOPER", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    197,   100,   4,  5},    // Chopper Command (1982).bin
    {"983f0b27ce796bf1175c5ec70b47a072",  "CHOPER", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  5},    // Chopper Command (Tron Solar Sailor Hack).bin
    {"5c0520c00163915a4336e481ca4e7ef4",  "CHOPER", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    244,   100,   0, 10},    // Chopper Command (1982) (PAL).bin
    {"114c599454d32f74c728a6e1f71012ba",  "CHOPER", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    244,   100,   3, 10},    // Chopper Command (1982) (PAL).bin
    {"2a360bc85bf22de438651cf92ffda1de",  "CHOPER", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    244,   100,   0, 10},    // Chopper Command (1982) (PAL).bin
    {"37fd7fa52d358f66984948999f1213c5",  "CHOPER", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   65,    244,   100,   0,  0},    // Chopper Command (1982) (PAL).bin
    {"6fc0176ccf53d7bce249aeb56d59d414",  "CHOPER", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   65,    244,   100,   0,  0},    // Chopper Command (1982) (PAL).bin
    {"3f58f972276d1e4e0e09582521ed7a5b",  "CHUCKN", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  5},    // Chuck Norris Superkicks (1983).bin
    {"e5d72ff8bab4450be57785cc9e83f3c0",  "CHUCKN", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,   100,   0,  8},    // Chuck Norris Superkicks (1983) (PAL).bin
    {"645bf7f9146f0e4811ff9c7898f5cd93",  "CHUCKN", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,   100,   0,  8},    // Chuck Norris Superkicks (1983) (PAL).bin
    {"3523c3858f5d10c768cd1441469df2bb",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    92,   0,  0},    // Chunkout2600.v1.0.bin
    {"a7b96a8150600b3e800a4689c3ec60a2",  "CIRCUS", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA0_7,  NTSC,  34,    210,   100,   0,  2},    // Circus Atari (1980).bin
    {"efffafc17b7cb01b9ca35324aa767364",  "CIRCUS", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA0_7,  NTSC,  34,    210,   100,   0,  2},    // Circus Atari (1980).bin
    {"a30ece6dc4787e474fbc4090512838dc",  "CIRCUS", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA0_7,  NTSC,  34,    210,   100,   0,  2},    // Circus Atari (1980).bin (Zellers Joystick)
    {"2aa30b26866483986ee4a345ad57f106",  "CIRCUS", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA0_7,  NTSC,  34,    210,   100,   0,  5},    // Circus Atari (Exidy Circus Hack).bin
    {"30e0ab8be713208ae9a978b34e9e8e8c",  "CIRCUS", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA0_7,  PAL,   44,    245,    53,   0,  1},    // Circus Atari (1980) (PAL).bin
    {"feba8686fd0376015258d1152923958a",  "CIRCUS", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA0_7,  PAL,   61,    245,    53,   0,  1},    // Circus Atari (1980) (PAL).bin
    {"f3dfae774f3bd005a026e29894db40d3",  "CIRCUS", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA0_7,  PAL,   67,    245,    99,   0,  0},    // Circus Atari (1980) (PAL).bin
    {"466c9d4a93668ab6a052c268856cf4a5",  "??????", BANK_F4SC, CTR_PADDLE0,   SPEC_NONE,      MODE_FF,    VB,   HB,  ANA0_7,  NTSC,  34,    210,    90,   0,  3},    // Circus Atari Age PRGE Demo.bin
    {"5f383ce70c30c5059f13e89933b05c4a",  "CIRCAT", BANK_F8,   CTR_PADDLE0,   SPEC_NONE,      MODE_FF,    VB,   HB,  ANA0_7,  NTSC,  34,    210,    90,   0,  3},    // Circus Atariage 2020.bin
    {"d85d93c7db2790b805c3d9805b3d8b69",  "??????", BANK_DPCP, CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // City Defence 20200331.bin
    {"19b1f085068e43db89f0dc28a74f7447",  "??????", BANK_DPCP, CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // City Defence 2.4.bin
    {"13a11a95c9a9fb0465e419e4e2dbd50a",  "CLIMB5", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Climber 5.bin
    {"1e587ca91518a47753a28217cd4fd586",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  35,    210,    98,   0,  0},    // Coco Nuts (1982).bin
    {"5846b1d34c296bf7afc2fa05bbc16e98",  "??????", BANK_2K,   CTR_KEYBOARD0, SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    210,   100,   0,  0},    // Codebreaker (1978).bin
    {"a47e26096de6f6487bf5dd2d1cced294",  "??????", BANK_2K,   CTR_KEYBOARD0, SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   53,    245,   100,   0,  3},    // Codebreaker (1978) (PAL).bin
    {"02dc11425eb84af137ff783071470b0f",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  30,    210,   100,   0,  0},    // Cold War Fixed (2008) (David Weavil).bin
    {"e3aa9681fab887a5cb964a55b4edd5d7",  "COLONY", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Colony 7.bin
    {"c48ab8184e5816c5704a16161f8a3ca6",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  35,    210,   100,   0,  4},    // Col'n (NTSC Fix by TJ).bin
    {"4c8832ed387bbafc055320c05205bc08",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    213,    89,   0,  3},    // Combat (1977).bin
    {"0ef64cdbecccb7049752a3de0b7ade14",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,    51,   0,  5},    // Combat (1977) (PAL).bin
    {"e8aa36e3d49e9bfa654c25dcc19c74e6",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,    51,   0,  5},    // Combat (1977) (PAL).bin
    {"b0c9cf89a6d4e612524f4fd48b5bb562",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Combat Two Prototype (1982).bin
    {"5d2cc33ca798783dee435eb29debf6d6",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   5,  4},    // Commando (1988).bin
    {"61631c2f96221527e7da9802b4704f93",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   5,  4},    // Commando (1988).bin
    {"de1e9fb700baf8d2e5ae242bffe2dbda",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   63,    245,   100,   5,  0},    // Commando (1988) (PAL).bin
    {"f457674cef449cfd85f21db2b4f631a7",  "COMRAI", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Commando Raid (1982).bin
    {"ce5524bb18e3bd8e092273ef22d36cb9",  "COMRAI", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   49,    245,   100,   0,  0},    // Commando Raid (1982) (PAL).bin
    {"ec407a206b718a0a9f69b03e920a0185",  "COMRAI", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   49,    245,   100,   0,  0},    // Commando Raid (1982) (PAL).bin
    {"5864cab0bc21a60be3853b6bcd50c59f",  "COMRAI", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   55,    245,   100,   0,  0},    // Commando Raid (1982) (PAL).bin
    {"5f316973ffd107f7ab9117e93f50e4bd",  "COMRAI", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   55,    245,   100,   0,  0},    // Commando Raid (1982) (PAL).bin
    {"2c8835aed7f52a0da9ade5226ee5aa75",  "COMMIE", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  31,    200,    95,   4,  0},    // Communist Mutants from Space (1982).bin
    {"e2c89f270f72cd256ed667507fa038a2",  "COMMIE", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_FF,    VB,   HB,  ANA1_0,  PAL,   68,    240,   100,   4,  0},    // Communist Mutants from Space (1982) (PAL).bin
    {"b98cc2c6f7a0f05176f74f0f62c45488",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // CompuMate.bin
    {"e7f005ddb6902c648de098511f6ae2e5",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // CompuMate (PAL).bin
    {"6a2c68f7a77736ba02c0f21a6ba0985b",  "??????", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  9},    // Computer Chess (1978).bin
    {"1f21666b8f78b65051b7a609f1d48608",  "CONDOR", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    90,   0,  0},    // Condor Attack (1982).bin
    {"e505bd8e59e31aaed20718d47b15c61b",  "CONDOR", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   65,    245,    90,   0,  0},    // Condor Attack (1982) (PAL).bin
    {"b49331b237c8f11d5f36fe2054a7b92b",  "CONDOR", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   65,    245,    90,   0,  0},    // Condor Attack (1982) (PAL).bin
    {"13a991bc9c2ff03753aeb322d3e3e2e5",  "CONDOR", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   69,    245,    90,   0,  0},    // Condor Attack (1982) (PAL).bin
    {"b00a8bc9d7fe7080980a514005cbad13",  "CONDOR", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   68,    245,    90,   0,  0},    // Condor Attack (1982) (PAL).bin
    {"522c9cf684ecd72db2f85053e6f6f720",  "CONDOR", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   68,    245,    90,   0,  0},    // Condor Attack (1982) (PAL).bin
    {"f965cc981cbb0822f955641f8d84e774",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  33,    210,    80,   0,  1},    // Confrontation (1983).bin
    {"00b7b4cbec81570642283e7fc1ef17af",  "CONGOB", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  9},    // Congo Bongo (1983).bin
    {"d078d25873c5b99f78fa267245a2af02",  "CONGOB", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  9},    // Congo Bongo (1983).bin
    {"335a7c5cfa6fee0f35f5824d1fa09aed",  "CONGOB", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   70,    245,   100,   0,  0},    // Congo Bongo (1983) (PAL).bin
    {"50dd164c77c4df579843baf838327469",  "CONMAR", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  7},    // Conquest of Mars (v1).bin
    {"0f604cd4c9d2795cf5746e8af7948064",  "CONMAR", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  7},    // Conquest of Mars (v2).bin
    {"b9f64a784b0e271526887ecfb67d36ae",  "CONMAR", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  7},    // Conquest-of-Mars_demo_final_NTSC.bin
    {"57c5b351d4de021785cf8ed8191a195c",  "??????", BANK_F8,   CTR_KEYBOARD0, SPEC_COOKIEM,   MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Cookie Monster Munch (1983).bin
    {"ba403a44966fd8307aac08214ac73bdd",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Cookie Monster Munch (joystick).bin
    {"798b8921276eec9e332dfcb47a2dbb17",  "??????", BANK_F8,   CTR_KEYBOARD0, SPEC_COOKIEM,   MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Cookie Monster Munch (1983) (PAL).bin
    {"a0297c4788f9e91d43e522f4c561b4ad",  "??????", BANK_F8,   CTR_KEYBOARD0, SPEC_COOKIEM,   MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Cookie Monster Munch (1983) (PAL).bin
    {"7d903411807704e725cf3fafbeb97255",  "COSARK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Cosmic Ark (1982) [selectable starfield].bin
    {"ab5bf1ef5e463ad1cbb11b6a33797228",  "COSARK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Cosmic Ark (1982).bin
    {"69df0411d4d176e558017f961f5c5849",  "COSARK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Cosmic Ark (1982).bin
    {"98ef1593624b409b9fb83a1c272a0aa7",  "COSARK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Cosmic Ark (1982).bin
    {"5c19f6da638c4c7c1f98d09e63df43e4",  "COSARK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Cosmic Ark (1982).bin
    {"0fd72a13b3b6103fc825a692c71963b4",  "COSARK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   65,    245,   100,   0,  1},    // Cosmic Ark (1982) [selectable starfield] (PAL).bin
    {"c5124e7d7a8c768e5a18bde8b54aeb1d",  "COSARK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   65,    245,   100,   0,  1},    // Cosmic Ark (1982) (PAL).bin
    {"282a77841cb3d33af5b56151acba770e",  "COSARK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   65,    245,   100,   0,  1},    // Cosmic Ark (1982) (PAL).bin
    {"72d0acb5de0db662de0360a6fc59334d",  "COSARK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   65,    245,   100,   0,  1},    // Cosmic Ark (1982) (PAL).bin
    {"133b56de011d562cbab665968bde352b",  "COSCOM", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   0,  0},    // Cosmic Commuter (1984).bin
    {"5f1b7d5fa73aa071ba0a3c2819511505",  "COSCOM", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   0,  0},    // Cosmic Commuter (1984).bin
    {"f367e58667a30e7482175809e3cec4d4",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0, -1},    // Cosmic Corridor (Space Tunnel) (1983).bin
    {"df2745d585238780101df812d00b49f4",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  36,    210,   100,   0,  2},    // Cosmic Corridor (Space Tunnel) (1983).bin
    {"be3f0e827e2f748819dac2a22d6ac823",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  36,    210,   100,   0,  0},    // Cosmic Corridor (Space Tunnel) (1983).bin
    {"7fcd5fb59e88fc7b8473c641f44226c3",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  36,    210,   100,   0,  0},    // Cosmic Corridor (Space Tunnel) (1983).bin
    {"d73ad614f1c2357997c88f37e75b18fe",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   39,    245,    75,   0,  0},    // Cosmic Corridor  (Space Tunnel) (1983) (PAL).bin
    {"c5387fc1aa71f11d2fa82459e189a5f0",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   38,    245,    89,   0,  0},    // Cosmic Corridor  (Space Tunnel) (1983) (PAL).bin
    {"4981cefe5493ea512284e7f9f27d1e54",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Cosmic Corridor  (Space Tunnel) (1983) (PAL).bin
    {"3c853d864a1d5534ed0d4b325347f131",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Cosmic Creeps (1982).bin
    {"e2ca84a2bb63d1a210ebb659929747a9",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  PAL,   63,    245,    84,   0,  0},    // Cosmic Creeps (1982) (PAL).bin
    {"e5f17b3e62a21d0df1ca9aee1aa8c7c5",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Cosmic Swarm (1982).bin
    {"8af58a9b90b25907da0251ec0facf3b8",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  31,    210,   100,   0,  0},    // Cosmic Swarm (1982).bin
    {"2a2f46b3f4000495239cbdad70f17c59",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   55,    245,    90,   0,  1},    // Cosmic Swarm (1982) (PAL).bin
    {"afe4eefc7d885c277fc0649507fbcd84",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   55,    245,    90,   0,  1},    // Cosmic Swarm (1982) (PAL).bin
    {"fe67087f9c22655ce519616fc6c6ef4d",  "??????", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   3,  1},    // Crack'ed (1988).bin
    {"a184846d8904396830951217b47d13d9",  "CRAPOT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    198,   100,   4,  7},    // Crackpots (1983).bin
    {"e5359cbbbff9c6d7fe8aeff5fb471b46",  "CRAPOT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    198,   100,   4,  7},    // Crackpots (1983).bin
    {"a2aae759e4e76f85c8afec3b86529317",  "CRAPOT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    198,   100,   4,  7},    // Crackpots (1983).bin
    {"3091af0ef1a61e801f4867783c21d45c",  "CRAPOT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    198,   100,   4,  7},    // Crackpots (1983).bin
    {"3f3ad2765c874ca13c015ca6a44a40a1",  "CRAPOT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    198,   100,   4,  7},    // Crackpots (1983).bin
    {"606c2c1753051e03c1f1ac096c9d2832",  "CRAPOT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    198,   100,   4,  7},    // Crackpots (1983).bin
    {"13448eb5ba575e8d7b8d5b280ea6788f",  "CRAPOT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    198,   100,   4,  7},    // Crackpots (1983).bin
    {"73aa02458b413091ac940c0489301710",  "CRAPOT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   51,    245,   100,   4, 17},    // Crackpots (1983) (PAL).bin
    {"7f54fa6aa824001af415503c313262f2",  "CRAPOT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   51,    245,   100,   4, 17},    // Crackpots (1983) (PAL).bin
    {"2825f4d068feba6973e61c84649489fe",  "CRAPOT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   51,    245,   100,   4, 17},    // Crackpots (1983) (PAL).bin
    {"f3c431930e035a457fe370ed4d230659",  "CRAPOT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   51,    245,   100,   4, 17},    // Crackpots (1983) (PAL).bin
    {"fb88c400d602fe759ae74ef1716ee84e",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,    93,   0,  0},    // Crash Dive (1983).bin
    {"0cebb0bb45a856b23f56d21ce7d1bc34",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,   100,   0,  0},    // Crash Dive (1983) (PAL).bin
    {"9c53b60a7b439a01efa46ae1effa348e",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Crazy Ballon.bin
    {"55ef7b65066428367844342ed59f956c",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Crazy Climber (1982).bin
    {"f91df5b8c095701838fe2ab42b3c697b",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  43,    210,   100,   0,  0},    // Crazy Tunes (RC2) NTSC.bin
    {"4a7eee19c2dfb6aeb4d9d0a01d37e127",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Crazy Valet.bin
    {"76a9bf05a6de8418a3ebc7fc254b71b4",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Color Bar Generator.bin
    {"c17bdc7d14a36e10837d039f43ee5fa3",  "CROSSF", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Cross Force (1982).bin
    {"8f88309afad108936ca70f8b2b084718",  "CROSSF", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,    94,   0,  0},    // Cross Force (1982) (PAL).bin
    {"8372eec01a08c60dbed063c5524cdfb1",  "CROSSF", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,    94,   0,  0},    // Cross Force (1982) (PAL).bin
    {"8cd26dcf249456fe4aeb8db42d49df74",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  0},    // Crossbow (1987).bin
    {"7e4783a59972ae2cd8384f231757ea0b",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   55,    245,   100,   4,  0},    // Crossbow (1987) (PAL).bin
    {"74f623833429d35341b7a84bc09793c0",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Cruise Missile (1987).bin
    {"384f5fbf57b5e92ed708935ebf8a8610",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Crypts of Chaos (1982).bin
    {"1c6eb740d3c485766cade566abab8208",  "??????", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Crystal Castles (1984).bin
    {"f12afbffa080dd3b2801dd14d4837cf6",  "??????", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Crystal Castles (1984).bin
    {"ca7abc774a2fa95014688bc0849eee47",  "??????", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Crystal Castles (1984) (PAL).bin
    {"6fa0ac6943e33637d8e77df14962fbfc",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    220,   100,   0,  2},    // Cubicolor (1982) (Imagic, Rob Fulop) (Prototype) ~.bin
    {"64ca518905311d2d9aeb56273f6caa04",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Cubicolor (1982).bin
    {"d2c957dd7746521b51bb09fde25c5774",  "??????", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Cubis (6K) (1997) (Eckhard Stolberg).bin
    {"58513bae774360b96866a07ca0e8fd8e",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Custer's Revenge (1982).bin
    {"50200f697aeef38a3ce31c4f49739551",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   39,    245,   100,   0, -2},    // Custer's Revenge (1982) (PAL).bin
    {"7e464186ba384069582d9f0c141f7491",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   62,    245,   100,   0,  0},    // Custer's Revenge (1982) (PAL).bin
    {"d47387658ed450db77c3f189b969cc00",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   62,    245,   100,   0,  0},    // Custer's Revenge (1982) (PAL).bin
    {"b25841173f058380b1771aacd5e7cdf3",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Dancing Plate (1982).bin
    {"ece463abde92e8b89bcd867ec71751b8",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Dancing Plate (1982) (PAL).bin
    {"f48735115ec302ba8bb2d2f3a442e814",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Dancing Plate (1982) (PAL).bin
    {"929e8a84ed50601d9af8c49b0425c7ea",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Dancing Plate (1982) (PAL).bin
    {"203b1efc6101d4b9d83bb6cc1c71f67f",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Dancing Plate (1982) (PAL).bin
    {"4f1dd311816cca397ff318bcbbcd4646",  "??????", BANK_DPCP, CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,  !HB,  ANA1_0,  NTSC,  34,    204,   100,   0,  0},    // Dare Devil RC1.bin
    {"a422194290c64ef9d444da9d6a207807",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   3,  3},    // Dark Cavern (1982).bin
    {"bd39598f067a1193ae81bd6182e756d1",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   59,    245,    97,   3,  1},    // Dark Cavern (Night Stalker) (1982) (PAL).bin
    {"106855474c69d08c8ffa308d47337269",  "??????", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Dark Chambers (1988).bin
    {"951e8cec7a1a1d6c01fd649e7ff7743a",  "??????", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   54,    245,   100,   0, -1},    // Dark Chambers (1988) (PAL).bin
    {"0d5af65ad3f19558e6f8e29bf2a9d0f8",  "??????", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Dark Chambers (1988) (PAL).bin
    {"6333ef5b5cbb77acd47f558c8b7a95d3",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  5},    // Dark Mage (8K) (PD).bin
    {"e4c00beb17fdc5881757855f2838c816",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Deadly Duck (1982).bin
    {"4e15ddfd48bca4f0bf999240c47b49f5",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    85,   0,  2},    // Death Trap (1983).bin
    {"0f643c34e40e3f1daafd9c524d3ffe64",  "DEFEND", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Defender (1982).bin
    {"e1029676edb3d35b76ca943da7434da8",  "DEFEND", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Defender (1982).bin
    {"808c3b1e60ee0e7c65205fa4bd772221",  "DEFEND", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Defender (1982).bin
    {"9af615951e9719df2244bc77fc50cb95",  "DEFEND", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Defender (1982).bin
    {"6596b3737ae4b976e4aadb68d836c5c7",  "DEFEND", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Defender (1982).bin
    {"dd1842ba0f3f9d94dccb21eaa0f069b7",  "DEFEND", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Defender (1982).bin
    {"aa5cfe3b20395aba1d479135943ad85c",  "DEFEND", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Defender (1982).bin
    {"6982854657a2cc87d712f718e402bf85",  "DEFEND", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Defender (1982).bin
    {"3c4223316c835ceaad619651e25df0f9",  "DEFEND", BANK_4K,   CTR_GENESIS,   SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Defender (2 Button Hack).bin
    {"e4bff1d5df70163c0428a1ead309c22d",  "DEFEND", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  PAL,   47,    245,    68,   0,  1},    // Defender (1982) (PAL).bin
    {"bbb4ed29300848f579f730962d277700",  "DEFEND", BANK_F8SC, CTR_STARGATE,  SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Defender Arcade (1982).bin
    {"3a771876e4b61d42e3a3892ad885d889",  "??????", BANK_F8SC, CTR_STARGATE,  SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Defender II (Stargate) (1987).bin
    {"ce82a675c773ff21e0ffc0a4d1c90a71",  "??????", BANK_F8SC, CTR_GENESIS,   SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Defender II (Stargate) (2 Button Hack).bin
    {"5f786b67e05fb9985b77d4beb35e06ee",  "??????", BANK_F8SC, CTR_STARGATE,  SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   61,    245,    99,   0,  0},    // Defender II (Stargate) (1987) (PAL).bin
    {"acce65decf11f89afef6496afaf15277",  "??????", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Demios Lander.bin
    {"d09935802d6760ae58253685ff649268",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Demolition Herby (1983).bin
    {"7dfd100bda9abb0f3744361bc7112681",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   62,    245,   100,   0,  0},    // Demolition Herby (1983) (PAL).bin
    {"4a6be79310f86f0bebc7dfcba4d74161",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   61,    245,   100,   0,  0},    // Demolition Herby (1983) (PAL).bin
    {"f0e0addc07971561ab80d9abe1b8d333",  "DEMONA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Demon Attack (1981) [alt].bin
    {"b12a7f63787a6bb08e683837a8ed3f18",  "DEMONA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Demon Attack (1981) [fixed].bin
    {"bcb2967b6a9254bcccaf906468a22241",  "DEMONA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  6},    // Demon Attack (1981).bin
    {"b24f6a5820a4b7763a3d547e3e07441d",  "DEMONA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Demon Attack (1981).bin
    {"10c8cfd8c37522f11d47540ff024e5f9",  "DEMONA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  5},    // Demon Attack (1981).bin
    {"bac28d06dfc03d3d2f4a7c13383e84ee",  "DEMONA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  5},    // Demon Attack (1981).bin
    {"4901c05068512828367fde3fb22199fe",  "DEMONA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   45,    245,    54,   0,  0},    // Demon Attack (1981) (PAL).bin
    {"442602713cb45b9321ee93c6ea28a5d0",  "DEMONA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   56,    245,    94,   0,  0},    // Demon Attack (1981) (PAL).bin
    {"f91fb8da3223b79f1c9a07b77ebfa0b2",  "DEMDIA", BANK_4K,   CTR_PADDLE1,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA0_9,  NTSC,  34,    210,   100,   0,  0},    // Demons to Diamonds (1982).bin
    {"bf84f528de44225dd733c0e6a8e400a0",  "DEMDIA", BANK_4K,   CTR_PADDLE1,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA0_9,  NTSC,  34,    210,   100,   0,  0},    // Demons to Diamonds (1982).bin
    {"d62283aed0f4199adb2333de4c263e9c",  "DEMDIA", BANK_4K,   CTR_PADDLE1,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA0_9,  PAL,   44,    245,    59,   0,  2},    // Demons to Diamonds (1982) (PAL).bin
    {"519f007c0e14fb90208dbb5199dfb604",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Depth Charge (1983) (Amiga - Video Soft).bin
    {"fd4f5536fd80f35c64d365df85873418",  "DESFAL", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  30,    210,   100,   0,  0},    // Desert Falcon (1987).bin
    {"e9e6ad30549a6e2cd89fe93b7691d447",  "DESFAL", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   33,    245,   100,   0,  0},    // Desert Falcon (1987) (PAL).bin
    {"d4806775693fcaaa24cf00fc00edcdf3",  "DESFAL", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   33,    245,   100,   0,  0},    // Desert Falcon (1987) (PAL).bin
    {"38bd172da8b2a3a176e517c213fcd5a6",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Diagnostic Test Cartridge 2.6.bin
    {"93e276172b521c4491097f8b1393eea7",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Diagnostic Test Cartridge 4.2.bin
    {"9222b25a0875022b412e8da37e7f6887",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  31,    210,   100,   0,  0},    // Dice Puzzle (1983).bin
    {"e02156294393818ff872d4314fc2f38e",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   41,    210,    78,   0,  0},    // Dice Puzzle (1983) (PAL).bin
    {"84535afb9a69712ec0af4947329e08b8",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   45,    245,    85,   0,  0},    // Dice Puzzle (1983) (PAL).bin
    {"6dda84fb8e442ecf34241ac0d1d91d69",  "DIGDUG", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Dig Dug (1983).bin
    {"977294ae6526c31c7f9a166ee00964ad",  "DIGDUG", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   61,    245,   100,   0,  3},    // Dig Dug (1983) (PAL).bin
    {"6ee5ee59262035d2ad1d77725489b8a9",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // DigiBeatz_RC1.bin
    {"939ce554f5c0e74cc6e4e62810ec2111",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  6},    // Dishaster (1983).bin
    {"c3472fa98c3b452fa2fd37d1c219fb6f",  "DODGEM", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  8},    // Dodge 'Em (1980).bin
    {"83bdc819980db99bf89a7f2ed6a2de59",  "DODGEM", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  8},    // Dodge 'Em (1980) [fixed].bin
    {"a5e9ed3033fb2836e80aa7a420376788",  "DODGEM", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   74,    245,   100,   0,  0},    // Dodge 'Em (1980) (PAL).bin
    {"10f62443f1ae087dc588a77f9e8f43e9",  "DODGEM", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   74,    245,   100,   0,  0},    // Dodge 'Em (1980) [fixed] (PAL).bin
    {"d28afe0517a046265c418181fa9dd9a1",  "DODGEM", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   74,    245,   100,   0,  0},    // Dodge 'Em (1980) (PAL).bin
    {"77e7d82235b3d62342cca24140e1c23b",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  25,    210,   100,   0,  8},    // Dodge It.bin
    {"c6124a6c82c5b965f6afcf01f2790697",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  26,    210,    88,   0,  8},    // Doggone_It!.bin
    {"ef9acf3d796070782ebcf1ccfdd6754c",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,   100,   0,  0},    // Dogwalk_v1.0_NTSC.bas.bin
    {"ca09fa7406b7d2aea10d969b6fc90195",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  5},    // Dolphin (1983).bin
    {"3889351c6c2100b9f3aef817a7e17a7a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  5},    // Dolphin (1983).bin
    {"e17699a54c90f3a56ae4820f779f72c4",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   40,    245,   100,   4,  0},    // Dolphin (1983) (PAL).bin
    {"df3e6a9b6927cf59b7afb626f6fd7eea",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   4,  0},    // Dolphin (1983) (PAL).bin
    {"937736d899337036de818391a87271e0",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  2},    // Donald Duck's Speedboat (1983).bin
    {"fa7ce62e7fd77e02b3e2198d70742f80",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   62,    245,   100,   4,  0},    // Donald Duck's Speedboat (1983) (PAL).bin
    {"36b20c427975760cb9cf4a47e41369e4",  "DOKONG", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Donkey Kong (1982).bin
    {"5f4ebf8a1e5f5f7b9ff3e3c6affff3e6",  "DOKONG", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Donkey Kong (1982).bin
    {"8b5b1e3a434ebbdc2c2a49dc68f46360",  "DOKONG", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   64,    245,   100,   0,  0},    // Donkey Kong (1982) (PAL).bin
    {"b59417d083b0be2d49a7d93769880a4b",  "DOKONG", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   66,    245,   100,   0,  0},    // Donkey Kong (1982) (PAL).bin
    {"7511c34518a9a124ea773f5b0b5c9a48",  "DOKONG", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   66,    245,   100,   0,  0},    // Donkey Kong (1982) (PAL).bin
    {"9faace2db60d04685e58857c9d9205d0",  "DOKONG", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Donkey Kong 4 Levels (rev2).bin
    {"c8fa5d69d9e555eb16068ef87b1c9c45",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Donkey Kong Junior (1983).bin
    {"5a6febb9554483d8c71c86a84a0aa74e",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Donkey Kong Junior (1983).bin
    {"2880c6b59bd54b153174676e465167c7",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Donkey Kong Junior (1983).bin
    {"2091af29b4e7b86914d79d9aaa4cbd20",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   66,    245,    97,   0,  0},    // Donkey Kong Junior (1983) (PAL).bin
    {"494cda91cc640551b4898c82be058dd9",  "??????", BANK_F4SC, CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Donkey Kong VCS (PAL).bin
    {"f1ae6305fa33a948e36deb0ef12af852",  "??????", BANK_F4SC, CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Donkey Kong VCS.bin
    {"78c1759931d0f9ca877fc5eb2a25b61e",  "DKDPCP", BANK_DPCP, CTR_LJOY,      SPEC_NONE,      MODE_FF,   !VB,  !HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Donkey Kong DPC+.bin
    {"33c843d1f2fce61582e8a7885c20bf2a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Dorothy X (2006) (Cameron Aubuchon,Tarick F. Khalaf, Daniel Stensland).bin
    {"7386004f9a5a7daf7e50ac8547088337",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // DOT.bin
    {"7e2fe40a788e56765fe56a3576019968",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  6},    // Double Dragon (1989).bin
    {"4026ad38ba5ce486e88383dc27d7a46f",  "??????", BANK_F6,   CTR_GENESIS,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  6},    // Double Dragon (2 Button Hack).bin
    {"3624e5568368929fabb55d7f9df1022e",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   67,    245,   100,   4,  0},    // Double Dragon (1989) (PAL).bin
    {"4999b45be0ab5a85bac1b7c0e551542b",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   67,    245,   100,   4,  0},    // Double Dragon (1989) (PAL).bin
    {"368d88a6c071caba60b4f778615aae94",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Double Dunk (Super Basketball) (1989).bin
    {"cfc226d04d7490b69e155abd7741e98c",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   63,    245,   100,   0,  0},    // Double Dunk (Super Basketball) (1989) (PAL).bin
    {"6ed5012793f5ddf4353a48c11ea9b8d3",  "??????", BANK_AR,   CTR_PADDLE0,   SPEC_AR,        MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Down on the Line, Handcar.bin
    {"24d9a55d8f0633e886a1b33ee1e0e797",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  30,    210,    75,   0,  0},    // Dragon Defender.bin
    {"95e542a7467c94b1e4ab24a3ebe907f1",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   48,    245,    59,   0,  1},    // Dragon Defender (PAL).bin
    {"6a882fb1413912d2ce5cf5fa62cf3875",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   48,    245,    60,   0,  1},    // Dragon Defender (PAL).bin
    {"41810dd94bd0de1110bedc5092bef5b0",  "DRFIRE", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Dragonfire (1982).bin
    {"6fc394dbf21cf541a60e3b3631b817f1",  "DRFIRE", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   62,    245,   100,   0,  0},    // Dragonfire (1982) (PAL).bin
    {"1267e3c6ca951ff1df6f222c8f813d97",  "DRFIRE", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   63,    245,   100,   0,  0},    // Dragonfire (1982) (PAL).bin
    {"05ccf96247af12eef59698f1a060a54f",  "DRFIRE", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   59,    245,   100,   0,  0},    // Dragonfire (1982) (PAL).bin
    {"90ccf4f30a5ad8c801090b388ddd5613",  "DSTOMP", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   3,  0},    // Dragonstomper (Excalibur) (1982).bin
    {"8b04e9d132b8e30d447acaa6bd049c32",  "DSTOMP", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    240,    94,   3, 10},    // Dragonstomper (Excalibur) (1982) (PAL).bin
    {"77057d9d14b99e465ea9e29783af0ae3",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    92,   0,  0},    // Dragster (1980).bin
    {"0c54811cf3b1f1573c9164d5f19eca65",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   49,    245,    89,   0,  0},    // Dragster (1980) (PAL).bin
    {"c216b91f5db21a093ded6a5aaec85709",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   49,    245,    89,   0,  0},    // Dragster (1980) (PAL).bin
    {"5de972bc653d2a83645ce0fc49e3f05c",  "??????", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Duck Attack.bin
    {"51de328e79d919d7234cf19c1cd77fbc",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Dukes of Hazzard (1983).bin
    {"3897744dd3c756ea4b1542e5e181e02a",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Dumbo's Flying Circus (1983).bin
    {"15a0d59304dece2c7d0580f3ea3527f0",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Dumbo's Flying Circus (1983).bin (prototype)
    {"1f773a94d919b2a3c647172bbb97f6b4",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,    87,   0,  0},    // Dumbo's Flying Circus (1983) (PAL).bin
    {"31fcbce1cfa6ec9f5b6de318e1f57647",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   71,    245,    89,   0,  0},    // Dumbo's Flying Circus (1983) (PAL).bin
    {"469473ff6fed8cc8d65f3c334f963aab",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  27,    210,    97,   0,  0},    // Dune (1984).bin
    {"e2e514c3176b8215dd1fbc8fa83fd8f2",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Dungeon II.bin
    {"d4f23c92392474b8bc79184f1deb1b4d",  "DUNGEN", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Dungeon.bin
    {"615a3bf251a38eb6638cdc7ffbde5480",  "EXTRAT", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // E.T. - The Extra-Terrestrial (1982).bin
    {"c82ec00335cbb4b74494aecf31608fa1",  "EXTRAT", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // E.T. - The Extra-Terrestrial (1982).bin
    {"8febdd9142960d084ab6eeb1d3e88969",  "EXTRAT", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   62,    245,   100,   0,  0},    // E.T. - The Extra-Terrestrial (1982) (PAL).bin
    {"033e21521e0bf4e54e8816873943406d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    210,    88,   0,  0},    // Earth Dies Screaming (1983).bin
    {"2c0dc885d5ede94aa664bf3081add34e",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   53,    245,    88,   0,  3},    // Earth Dies Screaming (1983) (PAL).bin
    {"e1d75bcfbbb4e4d5e8d5af62e25ffba2",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Edtris (1995).bin
    {"db41f3ffc90cbb22a289386a85c524fe",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Egg Drop (2011) (Mr. Podoboo).bin
    {"42b2c3b4545f1499a083cfbc4a3b7640",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  8},    // Eggomania (1982).bin
    {"5f560837396387455c9dcb05cdd4b053",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  8},    // Eggomania (1982).bin
    {"2b1589c7e1f394ae6a1c046944f06688",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Eggomania (1982) (PAL).bin
    {"ae881ea7a216955e72ceae78266bf613",  "??????", BANK_DPCP, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // EggVenture_2600_(NTSC)(2021_ScumSoftware_RC4).bin
    {"8008ec6756631b095b25fd71691aca1d",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // ElectroBall_Beta_2[NTSC].bin
    {"b4a4f8af361d842beec37ba818911225",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // ElectroBall_Beta_3[NTSC].bin
    {"edfdff829f3a50fd66d61c6128c9ab5f",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // ElectroBall_Beta_4[NTSC].bin
    {"cd43a9cc395d8899f85be4e5c5177fc2",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // ElectroBallV1.0[NTSC].bin
    {"71f8bacfbdca019113f3f0801849057e",  "??????", BANK_F8SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Elevator Action (1983).bin
    {"7657b6373fcc9ad69850a687bee48aa1",  "ELEVAT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,  !HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  1},    // Elevators Amiss.bin
    {"7d483b702c44ee65cd2df22cbcc8b7ed",  "ELFADV", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  1},    // Elf Adventure (05-25-83) (Atari, Warren Robinett) (Prototype) ~.bin
    {"eed92cd796494529e3a1834bceb77ddd",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // ElfDashNTSC_011308.bin.bin
    {"b6812eaf87127f043e78f91f2028f9f4",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Eli's Ladder (1982).bin
    {"7eafc9827e8d5b1336905939e097aae7",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Elk Attack (1987).bin
    {"1cb5d322d1f95fd53a62c438505fa8d2",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   57,    210,   100,   0,  0},    // Encaved (2007) (Simon Quernhorst) (PAL).bin
    {"dbc8829ef6f12db8f463e30f60af209f",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  36,    210,    93,   0,  0},    // Encounter at L-5 (1982).bin
    {"5188fee071d3c5ef0d66fb45c123e4a5",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   44,    245,    93,   0,  0},    // Encounter at L-5 (1982) (PAL).bin
    {"94b92a882f6dbaa6993a46e2dcc58402",  "ENDURO", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  33,    198,   100,   4,  0},    // Enduro (1983).bin
    {"de62f8a30298e2325249fe112ecb5c10",  "ENDURO", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  33,    198,   100,   4,  0},    // Enduro (1983).bin
    {"360c0dcb11506e73bd0b77207c81bc62",  "ENDURO", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  33,    198,   100,   4,  0},    // Enduro (1983).bin
    {"e1efe2ef7664bb6758b1a22ff8ea16a1",  "ENDURO", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  33,    198,   100,   4,  0},    // Enduro (1983).bin
    {"07f84db31e97ef8d08dc9fa8a5250755",  "ENDURO", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  33,    198,   100,   4,  0},    // Enduro (1983).bin
    {"1323c45d660f5a5b6d5ea45c6c4cbe4a",  "ENDURO", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  33,    198,   100,   4,  0},    // Enduro (1983).bin
    {"31c5fd55a39db5ff30a0da065f86c140",  "ENDURO", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  33,    198,   100,   4,  0},    // Enduro (1983).bin
    {"5df559a36347d8572f9a6e8075a31322",  "ENDURO", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  33,    198,   100,   4,  0},    // Enduro (1983).bin
    {"2bb0a1f1dee5226de648eb5f1c97f067",  "ENDURO", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  33,    198,   100,   4,  0},    // Enduro (1983).bin
    {"e9e646f730b8400cd5da08c849ef3e3b",  "ENDURO", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  33,    198,   100,   4,  0},    // Enduro (1983).bin
    {"4279485e922b34f127a88904b31ce9fa",  "ENDURO", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  33,    198,   100,   4,  0},    // Enduro (1983).bin
    {"9e437229136f1c5e6ef4c5f36178ed18",  "ENDURO", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  33,    198,   100,   4,  0},    // Enduro (1983).bin
    {"85502d69fe46b7f54ef2598225678b47",  "ENDURO", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  33,    198,   100,   4,  0},    // Enduro (1983).bin
    {"724613effaf7743cbcd695fab469c2a8",  "ENDURO", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  33,    198,   100,   4,  0},    // Enduro (1983).bin
    {"a8e49d7e24ce293629ca29614862821b",  "ENDURO", BANK_4K,   CTR_GENESIS,   SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  33,    198,   100,   4,  0},    // Enduro (2 Button Hack).bin
    {"61719a8bdafbd8dab3ca9ce7b171b9e2",  "ENDURO", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   56,    245,   100,   4,  0},    // Enduro (1983) (PAL).bin
    {"6a82b8ecc663f371b19076d99f46c598",  "ENDURO", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   4, 17},    // Enduro (1983) (PAL).bin
    {"638cc82ea96f67674595ba9ae05da6c6",  "ENDURO", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   4,  0},    // Enduro (1983) (PAL).bin
    {"2b27eb194e13f3b38d23c879cc1e3abf",  "ENDURO", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   63,    245,   100,   4,  0},    // Enduro (1983) (PAL).bin
    {"9f5096a6f1a5049df87798eb59707583",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    96,   0,  0},    // Entity (1983).bin
    {"6b683be69f92958abe0e2a9945157ad5",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    94,   0,  2},    // Entombed (1982).bin
    {"7e6a1375ee356f5a682f643bb8b7090c",  "??????", BANK_DPCP, CTR_LJOY,      SPEC_OLDDPCP,   MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    205,   100,   0,  0},    // Epic Adventure v28.bin
    {"81f4f0285f651399a12ff2e2f35bab77",  "MINDMA", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   2,  0},    // Escape from the Mindmaster (1982).bin
    {"c9e721eb29c940c2e743485b044c0a3f",  "MINDMA", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_NO,   !VB,   HB,  ANA1_0,  PAL,   58,    230,   100,   0,  0},    // Escape from the Mindmaster (1982) (PAL).bin
    {"8334075902fa9f3471905a30fc84e706",  "??????", BANK_F8SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,   100,   0,  0},    // Escape It (2009) (Alan W. Smith).bin
    {"f344ac1279152157d63e64aa39479599",  "??????", BANK_3F,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Espial (1984).bin
    {"f7a138eed69665b5cd1bfa796a550b01",  "??????", BANK_3F,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Espial (1984) (PAL).bin
    {"63685b372bf33967a8aa2be0bb01d1bd",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Espionage (2007) (Jessica Raboud, Matthew Remmele, Richard Shemaka).bin
    {"6205855cc848d1f6c4551391b9bfa279",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Euchre.bin
    {"62d1f50219edf9a429a9f004c19f31b3",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   34,    210,   100,   0,  0},    // Euro Gen (PAL).bin
    {"033d4d269f5f7053e3c9863fae9afbf5",  "??????", BANK_DPCP, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    205,   100,   0,  0},    // Evil Magician Returns II.bin
    {"e04c8ecae485b6970d680c202e58f843",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Evil Magician Returns.bin
    {"6362396c8344eec3e86731a700b13abf",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Exocet (1983).bin
    {"74f623833429d35341b7a84bc09793c0",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Exocet (1983).bin
    {"7ac4f4fb425db38288fa07fb8ff4b21d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   35,    245,    63,   0,  0},    // Exocet (1983) (PAL).bin
    {"ebd2488dcace40474c1a78fa53ebfadf",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Extra Terrestrials.bin
    {"8f459512fa21f8af59c3df8020be5b9d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  29,    210,   100,   0,  0},    // F-4 Supercharger (2006) (David Weavil).bin
    {"76181e047c0507b2779b4bcbf032c9d5",  "FALLDN", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Fall Down.bin
    {"78f788e8d0288069b97a887990e2123f",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  29,    210,   100,   0,  0},    // Fallout (Chain Reaction) (2005) (Colman Bryant, Jill Donnely, April Headen).bin
    {"b80d50ecee73919a507498d0a4d922ae",  "FANVOY", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Fantastic Voyage (1982).bin
    {"fab7b04b9f42df761eb6f2bc445eaa99",  "FANVOY", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Fantastic Voyage (1982).bin
    {"0f24ca5668b4ab5dfaf217933c505926",  "FANVOY", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  3},    // Fantastic Voyage (1982) (PAL).bin
    {"f7e07080ed8396b68f2e5788a5c245e2",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  54,    210,   100,   0,  1},    // Farmyard Fun (1983).bin
    {"d85f1e35c5445ac898746719a3d93f09",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,   100,   0,  9},    // Farmyard Fun (1983) (PAL).bin
    {"9de0d45731f90a0a922ab09228510393",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Fast Eddie (1982).bin
    {"a97733b0852ee3096300102cb0689175",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Fast Eddie (1982).bin
    {"83e1b9f22f29259679e1018bc04cc018",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Fast Eddie (1982).bin
    {"665b8f8ead0eef220ed53886fbd61ec9",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Fast Food (1982).bin
    {"6b7e1c11448c4d3f28160d2de884ebc8",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Fast Food (1982).bin
    {"48411c9ef7e2cef1d6b2bee0e6055c27",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0, 11},    // Fast Food (1982) (PAL).bin
    {"313243fc41e49ef6bd3aa9ebc0d372dd",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   58,    245,   100,   0,  1},    // Fast Food (1982) (PAL).bin
    {"a1f9159121142d42e63e6fb807d337aa",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   58,    245,   100,   0,  1},    // Fast Food (1982) (PAL).bin
    {"a1ca372388b6465a693e4626cc98b865",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   65,    245,   100,   0,  1},    // Fast Food (1982) (PAL).bin
    {"85470dcb7989e5e856f36b962d815537",  "FATRUN", BANK_F4SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  0},    // Fatal Run (1991).bin
    {"074ec425ec20579e64a7ded592155d48",  "FATRUN", BANK_F4SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   4,  6},    // Fatal Run (1991) (PAL).bin
    {"0b55399cf640a2a00ba72dd155a0c140",  "FATHOM", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Fathom (1983).bin
    {"68ac69b8e1ba83af8792f693f5ae7783",  "FATHOM", BANK_UA,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Fathom (1983).bin
    {"47cd61f83457a0890de381e478f5cf5f",  "FATHOM", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   63,    245,   100,   0,  0},    // Fathom (1983) (PAL).bin
    {"9addbd7959e47fafa4eddc43a77f292a",  "??????", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // FearoftheDark0.64_NTSC.bas.bin
    {"e4b12deaafd1dbf5ac31afe4b8e9c233",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Fellowship of the Ring.bin
    {"211fbbdbbca1102dc5b43dc8157c09b3",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Final Approach (1982).bin
    {"554fd5775ca6d544818c96825032cf0d",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Firefox Prototype (1983).bin
    {"c8e90fc944596718c84c82b55139b065",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Firefox Prototype (1983).bin
    {"1e31b3a48865ba98d4d1aa5205115983",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Firefox Prototype (1983).bin
    {"d09f1830fb316515b90694c45728d702",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Fire Fighter (1982).bin
    {"8e737a88a566cc94bd50174c2d019593",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   62,    245,   100,   0,  0},    // Fire Fighter (1982) (PAL).bin
    {"90d77e966793754ab4312c47b42900b1",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   64,    245,   100,   0,  0},    // Fire Fighter (1982) (PAL).bin
    {"20dca534b997bf607d658e77fbb3c0ee",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  5},    // Fire Fly (1983).bin
    {"d3171407c3a8bb401a3a62eb578f48fb",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Fire Spinner.bin
    {"386ff28ac5e254ba1b1bac6916bcc93a",  "??????", BANK_AR,   CTR_PADDLE0,   SPEC_AR,        MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Fireball (Frantic) (1982).bin
    {"5438e84b90e50a5362f01cc843b358d4",  "??????", BANK_AR,   CTR_PADDLE0,   SPEC_AR,        MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Fireball (Frantic) (1982).bin (AR-4300) (Prototype)
    {"cdb81bf33d830ee4ee0606ee99e84dba",  "??????", BANK_AR,   CTR_PADDLE0,   SPEC_AR,        MODE_FF,    VB,   HB,  ANA1_0,  PAL,   59,    235,   100,   0,  0},    // Fireball (Frantic) (1982) (PAL).bin
    {"2213bfaf13ca90ce7dfca2c63797e462",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  40,    210,   100,   0,  0},    // FishFightfinal.bin
    {"3fe43915e5655cf69485364e9f464097",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  35,    210,   100,   0,  9},    // Fisher Price (1983).bin
    {"b8865f05676e64f3bec72b9defdacfa7",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    199,   100,   0,  4},    // Fishing Derby (1980).bin
    {"13ccc692f111d52fec75d83df16192e2",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    199,   100,   0,  4},    // Fishing Derby (1980).bin
    {"dea0ade296f7093e71185e802b500db8",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    199,   100,   0,  4},    // Fishing Derby (1980).bin
    {"74023e0f2e739fc5a9ba7caaeeee8b6b",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    199,   100,   0,  4},    // Fishing Derby (1980).bin
    {"571c6d9bc71cb97617422851f787f8fe",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   54,    230,   100,   0,  7},    // Fishing Derby (1980) (PAL).bin
    {"6672de8f82c4f7b8f7f1ef8b6b4f614d",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    230,   100,   0,  7},    // Fishing Derby (1980) (PAL).bin
    {"db112399ab6d6402cc2b34f18ef449da",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  27,    210,   100,   0,  0},    // Fixit.bin
    {"30512e0e83903fc05541d2f6a6a62654",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Flag Capture (1978).bin
    {"da7a17dcdaa62d6971393c0a6faf202a",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   62,    245,   100,   0,  0},    // Flag Capture (1978) (PAL).bin
    {"f5445b52999e229e3789c39e7ee99947",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   62,    245,   100,   0,  0},    // Flag Capture (1978) (PAL).bin
    {"163ff70346c5f4ce4048453d3a2381db",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // FlapPing.bin
    {"8786c1e56ef221d946c64f6b65b697e9",  "FLASHG", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    94,   0,  0},    // Flash Gordon (1983).bin
    {"a2a384d3a16d5be50afd12906f146827",  "FLASHG", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    94,   0,  0},    // Flash Gordon (1983).bin
    {"4ae8c76cd6f24a2e181ae874d4d2aa3d",  "FLASHG", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   61,    245,   100,   0,  0},    // Flash Gordon (1983) (PAL).bin
    {"e549f1178e038fa88dc6d657dc441146",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Football (1979).bin
    {"cfe2185f84ce8501933beb5c5e1fd053",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Football (1979)(PAL).bin
    {"7608abdfd9b26f4a0ecec18b232bea54",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0, 12},    // Football (1979)(PAL).bin
    {"e275cbe7d4e11e62c3bfcfb38fca3d49",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    92,   4,  2},    // Football (AKA Super Challenge Football) (1989).bin
    {"5f9b62350b31be8bd270d9a241cbd50e",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   65,    245,    92,   4,  0},    // Football (AKA Super Challenge Football) (1989) (PAL).bin
    {"dd1422ffd538e2e33b339ebeef4f259d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Football Demo.bin
    {"5926ab1a9d1d34fb7c3bfd5afff6bc80",  "FOURPL", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Four Play.bin
    {"213e5e82ecb42af237cfed8612c128ac",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   59,    245,   95,    3,  0},    // Forest (1983) (PAL).bin
    {"15dd21c2608e0d7d9f54c0d3f08cca1f",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Frankenstein's Monster (1983).bin
    {"7d0b49ea4fe3a5f1e119a6d14843db17",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   46,    245,    80,   0,  0},    // Frankenstein's Monster (1983) (PAL).bin
    {"16c909c16ce47e2429fe6e005551178d",  "??????", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Frantic.bin
    {"8e0ab801b1705a740b476b7f588c6d16",  "FREWAY", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    202,   100,   5,  2},    // Freeway (1981).bin
    {"5dae540347cf0a559962d62604ecf750",  "FREWAY", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    202,   100,   5,  2},    // Freeway (1981).bin
    {"eddef10fdc0029301064115ae0cd41d4",  "FREWAY", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    202,   100,   5,  2},    // Freeway (1981).bin
    {"7d5c3b7b908752b98e30690e2a3322c2",  "FREWAY", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    202,   100,   5,  2},    // Freeway (1981).bin
    {"7b7b4ac05232490c28f9b680c72998f9",  "FREWAY", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    202,   100,   5,  2},    // Freeway (1981).bin
    {"4f7a6ba1baac8d25ce524eb660cca78d",  "FREWAY", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  60,    205,    91,   4,  0},    // Freeway (Turtle Race - BitCorp).bin
    {"2ec6b045cfd7bc52d9cdfd1b1447d1e5",  "FREWAY", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    230,   100,   5,  6},    // Freeway (1981) (PAL).bin
    {"e80a4026d29777c3c7993fbfaee8920f",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  35,    210,   100,   0,  1},    // Frisco (Unknown).bin
    {"cb4a7b507372c24f8b9390d22d54a918",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   57,    245,   100,   0,  0},    // Frisco (Home Vision) (PAL).bin
    {"056ff67dd9715fafa91fb8b0ddcc4a46",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   57,    245,   100,   0,  0},    // Frisco (Home Vision) (PAL).bin
    {"f67181b3a01b9c9159840b15449b87b0",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Frog Pond (1982).bin
    {"081e2c114c9c20b61acf25fc95c71bf4",  "FROGGR", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  31,    210,   100,   0,  0},    // Frogger (1982).bin
    {"02ced7ea2b7cb509748db6bfa227ebec",  "FROGGR", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  PAL,   44,    245,    74,   0,  1},    // Frogger (1982) (PAL).bin
    {"27c6a2ca16ad7d814626ceea62fa8fb4",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  5},    // Frogger II - Threeedeep! (1984).bin
    {"fb91dfc36cddaa54b09924ae8fd96199",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   54,    245,    88,   0,  0},    // Frogger II - Threeedeep! (1984) (PAL).bin
    {"dcc2956c7a39fdbf1e861fc5c595da0d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  7},    // Frogs and Flies (1982).bin
    {"4a196713a21ef07a3f74cf51784c6b12",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  7},    // Frogs and Flies (1982).bin
    {"1b8c3c0bfb815b2a1010bba95998b66e",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   60,    245,   100,   0,  7},    // Frogs and Flies (1982) (PAL).bin
    {"834a2273e97aec3181ee127917b4b269",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   60,    245,   100,   0,  7},    // Frogs and Flies (1982) (PAL).bin
    {"e556e07cc06c803f2955986f53ef63ed",  "FRONTL", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Front Line (1984).bin
    {"4c8832ed387bbafc055320c05205bc08",  "FRONTL", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Front Line (1984).bin
    {"4ca73eb959299471788f0b685c3ba0b5",  "FROSTB", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  38,    195,   100,   4,  1},    // Frostbite (1983).bin
    {"f4469178cd8998cb437fa110a228eaca",  "FROSTB", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  38,    195,   100,   4,  1},    // Frostbite (1983).bin
    {"c225379e7c4fb6f886ef9c8c522275b4",  "FROSTB", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  38,    195,   100,   4,  1},    // Frostbite (1983).bin
    {"c6ae21caceaad734987cb24243793bd5",  "FROSTB", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  38,    195,   100,   4,  1},    // Frostbite (1983).bin
    {"adb79f9ac1a633cdd44954e2eac14774",  "FROSTB", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  38,    195,   100,   4,  1},    // Frostbite (1983).bin
    {"6a9b30ca46b0dba9e719f4cbd340e01c",  "FROSTB", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   6, 13},    // Frostbite (BitCorp 1983).bin
    {"5a93265095146458df2baf2162014889",  "FROSTB", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   4,  5},    // Frostbite (1983) (PAL).bin
    {"25a21c47afe925a3ca0806876a2b4f3f",  "FROSTB", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   58,    245,   100,   4,  5},    // Frostbite (1983) (PAL).bin
    {"819aeeb9a2e11deb54e6de334f843894",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Fun with Numbers (1980).bin
    {"d816fea559b47f9a672604df06f9d2e3",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Fun with Numbers (1980) (PAL).bin
    {"819aeeb9a2e11deb54e6de334f843894",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Fun with Numbers (1980) (PAL).bin
    {"5f46d1ff6d7cdeb4b09c39d04dfd50a1",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Fun with Numbers (1980) (PAL).bin
    {"d3bb42228a6cd452c111c1932503cc03",  "??????", BANK_UA,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Funky Fish (1983).bin
    {"c1fdd44efda916414be3527a47752c75",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_GIJOE,     MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    94,   0,  0},    // G.I. Joe - Cobra Strike.bin
    {"b9f6fa399b8cd386c235983ec45e4355",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_GIJOE,     MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,    94,   0,  0},    // G.I. Joe - Cobra Strike (PAL).bin
    {"f38210ca3955a098c06a1e1c0004ef39",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Galactopus20141115.bin
    {"476d8d236085f8b1a6892dad3a898a62",  "??????", BANK_CDFJ, CTR_LJOY,      SPEC_DPCPOPT,   MODE_NO,   !VB,  !HB,  ANA1_0,  NTSC,  34,    205,    88,   0,  0},    // Galaga V2 (demo).bin
    {"211774f4c5739042618be8ff67351177",  "GALAXY", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Galaxian (1983).bin
    {"590ac71fa5f71d3eb29c41023b09ade9",  "GALAXY", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    98,   0,  2},    // Galaxian (1983).bin
    {"803393ed29a9e9346569dd1bf209907b",  "GALAXY", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Galaxian (1983).bin
    {"d65028524761ef52fbbdebab46f79d0f",  "GALAXY", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Galaxian (1983).bin
    {"93c8d9d24f9c5f1f570694848d087df7",  "GALAXY", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Galaxian (1983).bin
    {"13a37cf8170a3a34ce311b89bde82032",  "GALAXY", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   46,    245,   100,   0,  0},    // Galaxian (1983) (PAL).bin
    {"e67cc9ded34cf94b8f145df13ab11a60",  "GALAXY", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  7},    // Galaxian Arcade (Hack).bin
    {"8bd346383ac9ef486d1eab29661a7d37",  "GALAXY", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  7},    // Galaxian Expanded (Hack).bin
    {"102672bbd7e25cd79f4384dd7214c32b",  "??????", BANK_2K,   CTR_KEYBOARD0, SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Game of Concentration (1980).bin
    {"31f4692ee2ca07a7ce1f7a6a1dab4ac9",  "??????", BANK_4K,   CTR_KEYBOARD0, SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Game of Concentration (1980).bin
    {"5be03a1fe7b2c114725150be04b38704",  "??????", BANK_4K,   CTR_KEYBOARD0, SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Game of Concentration (1980) (PAL).bin
    {"003cadf805e346a2a99efe830a745226",  "??????", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  27,    210,    89,   0, 13},    // Game Catcher_finalbuild_033012.bas.bin
    {"db971b6afc9d243f614ebf380af0ac60",  "??????", BANK_2K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0, 13},    // Gamma-Attack (1983).bin
    {"20edcc3aa6c189259fa7e2f044a99c49",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    90,   0,  2},    // Gangster Alley (1982) [fixed].bin
    {"f16ef574d2042ed8fe877d6541f4dba4",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    90,   0,  2},    // Gangster Alley (1982).bin
    {"0c35806ff0019a270a7acae68de89d28",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    90,   0,  2},    // Gangster Alley (1982).bin
    {"bae66907c3200bc63592efe5a9a69dbb",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   53,    245,    90,   0,  8},    // Gangster Alley (1982) (PAL).bin
    {"47aad247cce2534fd70c412cb483c7e0",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   54,    245,    90,   0,  8},    // Gangster Alley (1982) (PAL).bin
    {"b17b9cc4103844dcda54f77f44acc93a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   53,    245,    88,   0,  8},    // Gangster Alley (1982) (PAL).bin
    {"dc13df8420ec69841a7c51e41b9fbba5",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Garfield (1984).bin
    {"5cbd7c31443fb9c308e9f0b54d94a395",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Gas Hog (1983) [fixed].bin
    {"728152f5ae6fdd0d3a9b88709bee6c7a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Gas Hog (1983).bin
    {"b1486e12de717013376447ac6f7f3a80",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   64,    245,   100,   0,  0},    // Gas Hog (1983) (PAL).bin
    {"89494fbf3377909027ac45fabed0e85b",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // GateRacer_Final.bin
    {"e9ecb4102242f662acbf6ea6e77fa940",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // GateRacerII.bin
    {"e64a8008812327853877a37befeb6465",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Gauntlet (1983).bin
    {"78e88aaa3bbbc747acbe6b4868d18b1f",  "??????", BANK_F4SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Gauntlet (1985 Hack).bin
    {"e05d25c5e330384b243675e3f4638263",  "??????", BANK_F4SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Gauntlet (1985 Hack - Scanline Fix 2P).bin
    {"5079bfbc7b8f5770f84215ed2e3bdd1b",  "??????", BANK_4K,   CTR_GENESIS,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Genesis Button Tester.bin
    {"2bee7f226d506c217163bad4ab1768c0",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    210,   100,   0,  0},    // Ghost Manor (1983).bin
    {"40d8ed6a5106245aa79f05642a961485",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   31,    245,   100,   0,  4},    // Ghost Manor (1983) (PAL).bin
    {"e314b42761cd13c03def744b4afc7b1b",  "GHOSTB", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  6},    // Ghostbusters (1985).bin
    {"c560a3ecb7b751021953819efcfe5b41",  "GHOSTB", BANK_F8,   CTR_GENESIS,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  6},    // Ghostbusters (2 Button Hack).bin
    {"f7d6592dcb773c81c278140ed4d01669",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   3, 15},    // Ghostbusters (1985) (PAL).bin
    {"718ee85ea7ec27d5bea60d11f6d40030",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  36,    210,   100,   0,  0},    // Ghostbusters II (1992).bin
    {"643e6451eb6b8ab793eb60ba9c02e000",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   58,    245,   100,   0,  0},    // Ghostbusters II (1992) (PAL).bin
    {"c2b5c50ccb59816867036d7cf730bf75",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   58,    245,   100,   0,  0},    // Ghostbusters II (1992) (PAL).bin
    {"1c8c42d1aee5010b30e7f1992d69216e",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  9},    // Gigolo (1982).bin
    {"f303630a2d7316787aecd67fff6b2e33",  "??????", BANK_UA,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Gingerbread Man (2007).bin
    {"37e828675d556775ae8285c0caf7d11c",  "??????", BANK_UA,   CTR_GENESIS,   SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Gingerbread Man (2007).bin (Sega Controller)
    {"4359452274f008a339be7622efd68fe3",  "??????", BANK_DPCP, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    205,   100,   0,  0},    // Gizzle WAP
    {"6d8b4f40a6e5ac9518f33927da52e135",  "??????", BANK_DPCP, CTR_LJOY,      SPEC_DPCPOPT,   MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    205,   100,   0,  0},    // Gizzle Wizard
    {"5e0c37f534ab5ccc4661768e2ddf0162",  "GLAPAT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Glacier Patrol (1989).bin
    {"67cdde4176e0447fc45a71e0a1cdd288",  "GLAPAT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  7},    // Glacier Patrol (1989) (PAL).bin
    {"2d9e5d8d083b6367eda880e80dfdfaeb",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Glib - Video Word Game (1983).bin
    {"cfb83a3b0513acaf8be4cae1512281dc",  "??????", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Going-Up (AR) (Prototype).bin
    {"0c4e5087d748c67a81d114b3a9b5a112",  "GOFISH", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Go Fish - Extended.bin
    {"787a2faebadc670a887a0e2483b8f034",  "GOFISH", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Go Fish.bin
    {"4093382187f8387e6d011883e8ea519b",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  5},    // Go Go Home Monster (Unknown).bin
    {"103f1756d9dc0dd2b16b53ad0f0f1859",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   66,    245,   100,   0,  0},    // Go Go Home Monster (Home Vision) (PAL).bin
    {"8178db8efd4675d57ec8adfa0503b332",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    95,   0,  0},    // Go Sub 2.bin
    {"c92cbd7a55a4c5f0289afeffa55d03b9",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    95,   0,  0},    // Go Sub.bin
    {"2e663eaa0d6b723b645e643750b942fd",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   3,  0},    // Golf (1980).bin
    {"bb756aa98b847dddc8fc170bc79f92b2",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   64,    245,   100,   3,  0},    // Golf (1980) (PAL).bin
    {"95351b46fa9c45471d852d28b9b4e00b",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   3,  0},    // Golf (1980) (PAL).bin
    {"9d522a3759aa855668e75962c84546f7",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   42,    245,    60,   3,  0},    // Golf (1980) (PAL).bin
    {"32ae78abbb5e677e2aabae5cc86cec29",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   3,  1},    // Good Luck, Charlie Brown (04-18-1984).bin
    {"c16c79aad6272baffb8aae9a7fff0864",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  36,    210,   100,   0,  0},    // Gopher (1982).bin
    {"c47b7389e76974fd0de3f088fea35576",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Gopher (1982).bin
    {"8f90590dba143d783df5a6cff2000e4d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0, 11},    // Gopher (1982) (PAL).bin
    {"06db908011065e5ebb37f4e253c2a0b0",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   61,    245,   100,   0, 11},    // Gopher (1982) (PAL).bin
    {"a56b642a3d3ab9bbeee63cd44eb73216",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   54,    245,   100,   0, 11},    // Gopher (1982) (PAL).bin
    {"31df1c50c4351e144c9a378adb8c10ba",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   61,    245,   100,   0, 11},    // Gopher (1982) (PAL).bin
    {"81b3bf17cf01039d311b4cd738ae608e",  "GORFxx", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Gorf (1982).bin
    {"c1f9f70ae527093b6af3b6531860e75d",  "GORFxx", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Gorf (1982).bin (prototype)
    {"3e03086da53ecc29d855d8edf10962cb",  "GORFxx", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   42,    245,    88,   0,  1},    // Gorf (1982) (PAL).bin
    {"5a9685c4d51a6c1d6a9544946d9e8dc3",  "??????", BANK_UA,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  6},    // Grandmas Revenge (UA Ltd. Bankswitching) (2010) (Fred Quimby).bin
    {"2903896d88a341511586d69fcfc20f7d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  6},    // Grand Prix (1982).bin
    {"e5f84930aa468db33c0d0f7b26dd8293",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  6},    // Grand Prix (1982).bin
    {"41c4e3d45a06df9d21b7aae6ae7e9912",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  6},    // Grand Prix (1982).bin
    {"d5d2d44fb73785996ccc24ae3a0f5cef",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  6},    // Grand Prix (1982).bin
    {"757f529026696e13838364dea382a4ed",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0, 10},    // Grand Prix (1982) (PAL).bin
    {"0f738dc44437557624eb277ed7ad91c9",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0, 17},    // Grand Prix (1982) (PAL).bin
    {"b4f87ce75f7329c18301a2505fe59cd3",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   63,    245,   100,   3,  0},    // Grand Prix (1982) (PAL).bin
    {"7146dd477e019f81eac654a79be96cb5",  "GRAVIT", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  35,    210,   100,   0,  0},    // Gravitar (1983) [alt].bin
    {"8ac18076d01a6b63acf6e2cab4968940",  "GRAVIT", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  35,    210,   100,   0,  0},    // Gravitar (1983).bin
    {"a81697b0c8bbc338ae4d0046ede0646b",  "GRAVIT", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  35,    210,   100,   0,  0},    // Gravitar (1983).bin
    {"9245a84e9851565d565cb6c9fac5802b",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  26,    215,    76,   0,  1},    // Great Escape (1983).bin
    {"7576dd46c2f8d8ab159d97e3a3f2052f",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   49,    245,    74,   0,  0},    // Great Escape (1983) (PAL).bin
    {"18f299edb5ba709a64c80c8c9cec24f2",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   29,    245,    74,   0,  0},    // Great Escape (1983) (PAL).bin
    {"01cb3e8dfab7203a9c62ba3b94b4e59f",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Gremlins (1984).bin
    {"66b89ba44e7ae0b51f9ef000ebba1eb7",  "??????", BANK_F8,   CTR_KEYBOARD0, SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Grover's Music Maker (1983).bin
    {"4ac9f40ddfcf194bd8732a75b3f2f214",  "??????", BANK_F8,   CTR_KEYBOARD0, SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Grover's Music Maker (1983).bin
    {"7ab2f190d4e59e8742e76a6e870b567e",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Guardian (1982).bin
    {"6a07836c382195dd5305ce61d992aaa6",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Guardian (1982).bin
    {"f750b5d613796963acecab1690f554ae",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Gunfight (Manuel Polik).bin
    {"b311ab95e85bc0162308390728a7361d",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  39,    210,   100,   0,  0},    // Gyruss (1984).bin
    {"e600f5e98a20fafa47676198efe6834d",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,    98,   0, 11},    // Gyruss (1984) (PAL).bin
    {"378054896d57cf0a87f8012378466eb1",  "GYVOLV", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Gyvolver_ntsc_2019y_05m_17d_1753t.bin
    {"fca4a5be1251927027f2c24774a02160",  "HEROAV", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    205,   100,   0,  8},    // H.E.R.O. (1984).bin
    {"1d284d6a3f850bafb25635a12b316f3d",  "HEROAV", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    205,   100,   0,  8},    // H.E.R.O. (1984).bin
    {"bdf1996e2dd64baf8eff5511811ca6ca",  "HEROAV", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    205,   100,   0,  8},    // H.E.R.O. (1984).bin
    {"abdad00a325e4374f76ac9a7dbcf61c0",  "HEROAV", BANK_F8,   CTR_GENESIS,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    205,   100,   0,  8},    // H.E.R.O. (2 Button Hack).bin
    {"d9b49f0678776e04916fa5478685a819",  "HEROAV", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   49,    245,   100,   3, 16},    // H.E.R.O. (1984) (PAL).bin
    {"09195a823d673e8698705542049385c5",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  40,    210,    95,   0,  8},    // Hack'em_HanglyMan.bin
    {"30516cfbaa1bc3b5335ee53ad811f17a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Halloween (1983).bin
    {"e04f1c1e4401d584d3f4343410a5bcc4",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Halloween (1983).bin
    {"6e179eee3d4631a7434d40cf7aeea6e8",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Halloween (1983).bin
    {"4afa7f377eae1cafb4265c68f73f2718",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  41,    210,    94,   0, -3},    // Halo2600_Final.bin
    {"f16c709df0a6c52f47ff52b9d95b7d8d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Hangman (1978).bin
    {"378c118b3bda502c73e76190ca089eef",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   45,    245,    86,   0,  0},    // Hangman (1978) (PAL).bin
    {"700a786471c8a91ec09e2f8e47f14a04",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  6},    // Hard Head (Activision Prototype).bin
    {"a34560841e0878c7b14cc65f79f6967d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  28,    210,    97,   0,  0},    // Harem (1982).bin
    {"610c5f8a514f540410671ed3cb3ccf41",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Haunted Adventure I.bin
    {"2f09da6a4011bbc25ec120d9ee0824e2",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Haunted Adventure II.bin
    {"f0a6e99f5875891246c3dbecbf2d2cea",  "HAUNTH", BANK_4K,   CTR_LJOY,      SPEC_HAUNTED,   MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Haunted House (1982).bin
    {"4857f8bb88bb63c640d3ea5aac7f5d6d",  "HAUNTH", BANK_F8,   CTR_LJOY,      SPEC_HAUNTED,   MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Haunted House (1982 - Prototype).bin
    {"3ca51b5c08f5a0ecfb17d0c1ec6d0942",  "HAUNTH", BANK_4K,   CTR_LJOY,      SPEC_HAUNTED,   MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Haunted House (1982).bin
    {"09e1ecf9bd2a3030d5670dba7a65e78d",  "HAUNTH", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Haunted House (1982) (PAL).bin
    {"2cc640f904e1034bf438075a139548d3",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Heartbreak.bin
    {"e4e173eae89f847ffa3610ede9035b55",  "??????", BANK_CDFJ, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // HEIST.bin
    {"260c787e8925bf3649c8aeae5b97dcc0",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Hell Driver (ITT Family Games, Thomas Jentzsch) (NTSC).bin
    {"aab840db22075aa0f6a6b83a597f8890",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   61,    245,   100,   0,  1},    // Hell Driver (ITT Family Games) (PAL).bin
    {"129d6c4cda0cd883faac4ae20499e0c2",  "HELWAY", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    97,   0,  1},    // Hellway (Final)
    {"8b0218362531b11a33b305dfc0437f9d",  "??????", BANK_UA,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  28,    212,    90,   0, 10},    // Hobo (UA).bin
    {"c52d9bbdc5530e1ef8e8ba7be692b01e",  "??????", BANK_F8,   CTR_KEYBOARD0, SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Holey Moley (1984).bin
    {"078e08c1ddf839067b7bc7b88dc04ac1",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Holey Moley (joystick) Prototype.bin
    {"0bfabf1e98bdb180643f35f2165995d0",  "??????", BANK_2K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Home Run - Baseball (1978).bin
    {"328949872e454181223a80389d03c122",  "??????", BANK_2K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   3, 13},    // Home Run - Baseball (1978) (PAL).bin
    {"ca7aaebd861a9ef47967d31c5a6c4555",  "??????", BANK_2K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   3, 13},    // Home Run - Baseball (1978) (PAL).bin
    {"9f901509f0474bf9760e6ebd80e629cd",  "??????", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Home Run - Baseball (Sears 4K).bin
    {"a6fe2dd4b829ad267b2b3183e1650228",  "HUGOHU", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Hugo Hunt stellaDemo8k_ntsc.bin
    {"7972e5101fa548b952d852db24ad6060",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   3,  0},    // Human Cannonball (1979).bin
    {"11330eaa5dd2629052fac37cfe1a0b7d",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   41,    245,    92,   3,  0},    // Human Cannonball (1979) (PAL).bin
    {"10a3cd14e5dcfdde6ff216a14ce7b7dd",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   41,    245,    92,   3,  0},    // Human Cannonball (1979) (PAL).bin
    {"ad42e3ca3144e2159e26be123471bffc",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   41,    245,    92,   3,  0},    // Human Cannonball (1979) (PAL).bin
    {"ecdcd9bd0949bad0b5e371aefa3f6352",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Hunchy 1K (2005) (Chris Walton).bin
    {"9fa0c664b157a0c27d10319dbbca812c",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  35,    210,   100,   0,  0},    // Hunchy II (2005) (Chris Walton).bin
    {"102672bbd7e25cd79f4384dd7214c32b",  "??????", BANK_2K,   CTR_KEYBOARD0, SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Hunt & Score (1978).bin
    {"f6a282374441012b01714e19699fc62a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // I Want My Mommy (1983).bin
    {"4b9581c3100a1ef05eac1535d25385aa",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // I.Q. 180.bin
    {"a4c08c4994eb9d24fb78be1793e82e26",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Ice Hockey (1981).bin
    {"6ed6bda5c42b2eb7a21c54e5b3ace3e3",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Ice Hockey (1981).bin
    {"47711c44723da5d67047990157dcb5dd",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Ice Hockey (1981).bin
    {"a3f2a0fcf74bbc5fa763b0ee979b05b1",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,    65,   0, 10},    // Ice Hockey (1981) (PAL).bin
    {"ac9adbd6de786a242e19d4bec527982b",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   49,    245,    70,   0,  0},    // Ice Hockey (1981) (PAL).bin
    {"c7d5819b26b480a49eb26aeb63cc831e",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   38,    245,    65,   0, 10},    // Ice Hockey (1981) (PAL).bin
    {"9a21fba9ee9794e0fadd7c7eb6be4e12",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Ikari Warriors (1991).bin
    {"3f251c50aa7237e61a38ab42315ebed4",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Ikari Warriors (TJ)(1991).bin
    {"321c3451129357af42a375d12afd4450",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   60,    245,   100,   3,  2},    // Ikari Warriors (1991) (PAL).bin
    {"c4bc8c2e130d76346ebf8eb544991b46",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Imagic Selector.bin
    {"75a303fd46ad12457ed8e853016815a0",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Immies & Aggies (1983).bin
    {"f4c6621f1a0b4d27081123c08d7d1497",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Immies & Aggies (1983).bin
    {"9b21d8fc78cc4308990d99a4d906ec52",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Immies & Aggies (1983).bin
    {"faffd84f3a8eceee2fa5ea5b0a3e6678",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   49,    245,   100,   0,  0},    // Immies & Aggies (1983) (PAL).bin
    {"47abfb993ff14f502f88cf988092e055",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  29,    210,    90,   0,  0},    // Inca Gold (AKA Spider Kong).bin
    {"de5aab22e5aba5edcb29a3e7491ff319",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  29,    210,    90,   0,  0},    // Inca Gold (AKA Spider Kong).bin
    {"936ef1d6f8a57b9ff575dc195ee36b80",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  26,    210,    90,   0, -2},    // Inca Gold (AKA Spider Kong).bin
    {"21299c8c3ac1d54f8289d88702a738fd",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  29,    210,    90,   0,  0},    // Inca Gold (AKA Spider Kong).bin
    {"ae465044dfba287d344ba468820995d7",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,    90,   0,  0},    // Inca Gold (AKA Spider Kong) (PAL).bin
    {"88d300a38bdd7cab9edad271c18cd02b",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,    90,   0,  0},    // Inca Gold (AKA Spider Kong) (PAL).bin
    {"d223bc6f13358642f02ddacfaf4a90c9",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,    90,   0,  0},    // Inca Gold (AKA Spider Kong) (PAL).bin
    {"f14d5e96ec3380aef57a4b70132c6677",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,    90,   0,  0},    // Inca Gold (AKA Spider Kong) (PAL).bin
    {"672012d40336b403edea4a98ce70c76d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,    90,   0,  0},    // Inca Gold (AKA Spider Kong) (PAL).bin
    {"37b98344c8e0746c486caf5aaeec892a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,    90,   0,  0},    // Inca Gold (AKA Spider Kong) (PAL).bin
    {"d39e29b03af3c28641084dd1528aae05",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,    90,   0,  0},    // Inca Gold (AKA Spider Kong) (PAL).bin
    {"ad859c8d0c9513b47861f38d03bdead7",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Incoming! [alt].bin
    {"331e3c95f7c5d2e7869301d070b7de76",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Incoming!.bin
    {"c5301f549d0722049bb0add6b10d1e09",  "??????", BANK_2K,   CTR_DRIVING,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    88,   0,  0},    // Indy 500 (1977).bin
    {"81591a221419024060b890665beb0fb8",  "??????", BANK_2K,   CTR_DRIVING,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,    88,   0,  4},    // Indy 500 (1977) (PAL).bin
    {"afe88aae81d99e0947c0cfb687b16251",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    96,   0,  5},    // Infiltrate (1981).bin
    {"3b69f8929373598e1752f43f8da61aa4",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,    91,   0,  2},    // Infiltrate (1981) (PAL).bin
    {"b4030c38a720dd84b84178b6ce1fc749",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // International Soccer (1982).bin
    {"3f6dbf448f25e2bd06dea44248eb122d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // International Soccer (1982).bin
    {"ce904c0ae58d36d085cd506989116b0b",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   62,    245,   100,   1,  1},    // International Soccer (1982) (PAL).bin
    {"a0185c06297b2818f786d11a3f9e42c3",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   58,    245,   100,   1,  1},    // International Soccer (1982) (PAL).bin
    {"cd568d6acb2f14477ebf7e59fb382292",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   62,    245,   100,   1,  1},    // International Soccer (1982) (PAL).bin
    {"4868a81e1b6031ed66ecd60547e6ec85",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Inv (V2.1) (1-3-98).bin
    {"9ea8ed9dec03082973244a080941e58a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // INV+.bin
    {"ab301d3d7f2f4fe3fdd8a3540b7a74f5",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // IQ-180.bin
    {"4b9581c3100a1ef05eac1535d25385aa",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // IQ-180.bin
    {"dc33479d66615a3b09670775de4c2a38",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,   100,   0,  0},    // IQ-180 (PAL).bin
    {"47db521cc84aaa1a0a8eef9464685dc9",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // IRan_final.bin
    {"2f0546c4d238551c7d64d884b618100c",  "IXION_", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Ixion (1984).bin
    {"e51030251e440cffaab1ac63438b44ae",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // James Bond 007 (1983).bin
    {"04dfb4acac1d0909e4c360fd2ac04480",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  26,    210,    91,   0,  1},    // Jammed.bin
    {"58a82e1da64a692fd727c25faef2ecc9",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Jawbreaker (1982).bin
    {"f40e437a9ebf0bdfe26204152f74f868",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Jawbreaker (1982).bin
    {"a406d2f6d84e61d842f4cb13b2b1cfa7",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   54,    245,   100,   0,  3},    // Jawbreaker (1982) (PAL).bin
    {"97327d6962f8c64e6f926f79cd01c6b9",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  3},    // Jawbreaker (1982) (PAL).bin
    {"718ae62c70af4e5fd8e932fee216948a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Journey Escape (1982).bin
    {"6b4eb5b3df80995b8d9117cb7e9aeb3c",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   45,    245,    91,   0,  0},    // Journey Escape (1982) (PAL).bin
    {"3276c777cbe97cdd2b4a63ffc16b7151",  "JOUST_", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Joust (1983).bin
    {"640a08e9ca019172d612df22a9190afb",  "JOUST_", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Joust (1983) (PAL).bin
    {"ec40d4b995a795650cf5979726da67df",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Joust Pong.bin
    {"89a923f6f6bec64f9b6fa6ff8ea37510",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Joyride.bin
    {"36c29ceee2c151b23a1ad7aa04bd529d",  "JRPACM", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  29,    210,   100,   0,  0},    // Jr. Pac-Man (1984).bin
    {"297c405afd01f3ac48cdb67b00d273fe",  "JRPACM", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   50,    245,   100,   0,  0},    // Jr. Pac-Man (1984) (PAL).bin
    {"4364680de59e650a02348b901ca7202f",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Jump.bin
    {"3f5bbfba280bb98c32371f0642aabbed",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Jumper99rc1.bin
    {"2cccc079c15e9af94246f867ffc7e9bf",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  22,    210,    81,   0,  3},    // Jungle Fever (1982).bin
    {"2bb9f4686f7e08c5fcc69ec1a1c66fe7",  "JUNGLE", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  1},    // Jungle Hunt (1983).bin
    {"2496d404bfc561a40a80bea6a69695c3",  "JUNGLE", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  1},    // Jungle Hunt (1983).bin
    {"88a6c9c88cb329ee5fa7d168bd6c7c63",  "JUNGLE", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  1},    // Jungle Hunt (1983).bin
    {"9fc2d1627dcdd8925f4c042e38eb0bc9",  "JUNGLE", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   4, 13},    // Jungle Hunt (1983) (PAL).bin
    {"7f30147d787265228f1eaef585f03b03",  "JUNGLE", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  2},    // Jungle Hunt (Bugfix Hack).bin
    {"90b647bfb6b18af35fcf613573ad2eec",  "JUNOST", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Juno First.bin
    {"b9d1e3be30b131324482345959aed5e5",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  4},    // Kabobber (1983).bin
    {"5428cdfada281c569c74c7308c7f2c26",  "KABOOM", BANK_2K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Kaboom! (1981).bin
    {"dbdaf82f4f0c415a94d1030271a9ef44",  "KABOOM", BANK_2K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Kaboom! (1981).bin
    {"af6ab88d3d7c7417db2b3b3c70b0da0a",  "KABOOM", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Kaboom! (1981).bin
    {"f9e99596345a84358bc5d1fbe877134b",  "KABOOM", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   1, 13},    // Kaboom! (1981) (PAL).bin
    {"7b43c32e3d4ff5932f39afcb4c551627",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Kamikaze Saucers (1983).bin
    {"4326edb70ff20d0ee5ba58fa5cb09d60",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Kangaroo (1983).bin
    {"ce1cbe159b9ae5992dacf09371de5e13",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Kangaroo (1983).bin
    {"6d8a04ee15951480cb7c466e5951eee0",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Kangaroo (1983).bin
    {"6fe67f525c39200a798985e419431805",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  7},    // Kangaroo (1983) (PAL).bin
    {"cedbd67d1ff321c996051eec843f8716",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  28,    210,    86,   0,  0},    // Karate (1982).bin
    {"10eae73a07b3da044b72473d8d366267",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Karate (1982) (PAL).bin
    {"dd17711a30ad60109c8beace0d4a76e8",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Karate (1982) (PAL).bin
    {"6805734a0b7bcc8925d9305b071bf147",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Karate (1982) (PAL).bin
    {"be929419902e21bd7830a7a7d746195d",  "KAPERS", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   3,  3},    // Keystone Kapers (1983).bin
    {"572d0a4633d6a9407d3ba83083536e0f",  "KAPERS", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   3,  3},    // Keystone Kapers (1983).bin
    {"e77f332b71f13884c84771e7a121182d",  "KAPERS", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   3,  3},    // Keystone Kapers (1983).bin
    {"a1770ef47146ab7b12e2c4beccd68806",  "KAPERS", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   3,  3},    // Keystone Kapers (1983).bin
    {"810d8952af5a6036fca8d0c4e1b23db6",  "KAPERS", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   3,  3},    // Keystone Kapers (1983).bin
    {"25f879ff678130fea615ac418e7943f1",  "KAPERS", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   3,  3},    // Keystone Kapers (1983).bin
    {"f28c07767b3e90a2689ade5b5e305874",  "KAPERS", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   3,  3},    // Keystone Kapers (1983).bin
    {"1351c67b42770c1bd758c3e42f553fea",  "KAPERS", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   3,  3},    // Keystone Kapers (1983).bin
    {"f5a3e051730d45fea518f2e8b926565b",  "KAPERS", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   3,  3},    // Keystone Kapers (1983).bin
    {"e558be88eef569f33716e8e330d2f5bc",  "KAPERS", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   3,  3},    // Keystone Kapers (1983).bin
    {"8db152458abaef3cfa7a4e420ddbda59",  "KAPERS", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   3,  3},    // Keystone Kapers (1983).bin
    {"e28113d10c0c14cc3b5f430b0d142fcb",  "KAPERS", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   3,  3},    // Keystone Kapers (1983).bin
    {"4fbe0f10a6327a76f83f83958c3cbeff",  "KAPERS", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   3,  3},    // Keystone Kapers (1983).bin
    {"7187118674ff3c0bb932e049d9dbb379",  "KAPERS", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   3,  3},    // Keystone Kapers (1983).bin
    {"17bbe288c3855c235950fea91c9504e9",  "KAPERS", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   3,  3},    // Keystone Kapers (1983).bin
    {"05aedf04803c43eb5e09dfd098d3fd01",  "KAPERS", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   3,  7},    // Keystone Kapers (1983) (PAL).bin
    {"6c1f3f2e359dbf55df462ccbcdd2f6bf",  "KAPERS", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   3,  7},    // Keystone Kapers (1983) (PAL).bin
    {"1c5796d277d9e4df3f6648f7012884c4",  "KAPERS", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   3,  7},    // Keystone Kapers (1983) (PAL).bin
    {"248668b364514de590382a7eda2c9834",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Kick-Man (Prototype).bin
    {"7a7f6ab9215a3a6b5940b8737f116359",  "KILLER", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    195,   100,   0,  0},    // Killer Satellites (1983).bin
    {"75e276ba12dc4504659481c31345703a",  "KILLER", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_NO,    VB,   HB,  ANA1_0,  PAL,   53,    230,   100,   0, 11},    // Killer Satellites (1983) (PAL).bin
    {"e21ee3541ebd2c23e817ffb449939c37",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  28,    210,    81,   0,  0},    // King Kong (1982).bin
    {"0b1056f1091cfdc5eb0e2301f47ac6c3",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   30,    245,    75,   0,  0},    // King Kong (1982) (PAL).bin
    {"b3ff7cef2f04711ee683d040da5276fc",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   30,    245,    75,   0,  0},    // KITE! (2009) (Simon Quernhorst) (PAL).bin
    {"0dd4c69b5f9a7ae96a7a08329496779a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   54,    245,    79,   0,  0},    // King Kong (1982) (PAL).bin
    {"2c29182edf0965a7f56fe0897d2f84ba",  "KLAX90", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Klax (1990).bin
    {"eed9eaf1a0b6a2b9bc4c8032cb43e3fb",  "KLAX90", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   48,    245,   100,   0,  0},    // Klax (1990) (PAL).bin
    {"7fcd1766de75c614a3ccc31b25dd5b7a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Knight on the Town (1982).bin
    {"ed0451010d022b96a464febcba70b9c4",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  3},    // Knight on the Town (1982) (PAL).bin
    {"53888a549cd732abed06a3bcd7318710",  "KOCRUZ", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // KO Cruiser - 2009-09-14 - NTSC.bin
    {"534e23210dd1993c828d944c6ac4d9fb",  "??????", BANK_4K,   CTR_LJOY,      SPEC_KOOLAID,   MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Kool-Aid Man (1983).bin
    {"a2eb84cfeed55acd7fece7fefdc83fbb",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Kool-Aid Man (1983) - collision fix.bin
    {"4a7fad7a9a5015e09c3cd00db619b9b6",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Kool-Aid Man (Supercharger compatible - bugfixed).bin
    {"dad7d548930ebc2c52f15c16d2c4eaec",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  29,    210,   100,   0,  0},    // Kraken Attack.bin
    {"4baada22435320d185c95b7dd2bcdb24",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Krull (1983).bin
    {"00dc28b881989c39a6cf87a892bd3c6b",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Krull (1983).bin
    {"5b92a93b23523ff16e2789b820e2a4c5",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  3},    // Kung-Fu Master (1987).bin
    {"0b4e793c9425175498f5a65a3e960086",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  3},    // Kung-Fu Master (1987).bin
    {"4474b3ad3bf6aabe719a2d7f1d1fb4cc",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   5,  5},    // Kung-Fu Master (1987) (PAL).bin
    {"7ad782952e5147b88b65a25cadcdf9e0",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    98,   0,  5},    // Kwibble (1983).bin
    {"b86552198f52cfce721bafb496363099",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Kyphus (1982).bin
    {"adfbd2e8a38f96e03751717f7422851d",  "LADBUG", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0, 10},    // Lady Bug.bin
    {"5b45b4ad247c37600dd0faf269bff993",  "LADBUG", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0, 10},    // Lady-Bug_demo_final_NTSC.bin
    {"f1489e27a4539a0c6c8529262f9f7e18",  "LADBUG", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  PAL,   52,    245,    80,   0,  0},    // Lady Bug PAL.bin
    {"95a89d1bf767d7cc9d0d5093d579ba61",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Lady in Wading (1982).bin
    {"d06b7fcc7640735d36c0b8d44fb5e765",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Lander (2008) (Bastian Framke).bin
    {"8c103a79b007a2fd5af602334937b4e1",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  32,    240,    60,   0, 13},    // Laser Base World End (NTSC by TJ).bin
    {"85564dd0665aa0a1359037aef1a48d58",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   28,    245,    51,   0, 13},    // Laser Base World End (PAL).bin
    {"130c5742cd6cbe4877704d733d5b08ca",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   28,    245,    51,   0, 13},    // Laser Base World End (PAL).bin
    {"931b91a8ea2d39fe4dca1a23832b591a",  "LASRBL", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Laser Blast (1981).bin
    {"d078674afdf24a4547b4b32890fdc614",  "LASRBL", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Laser Blast (1981).bin
    {"303242c239474f2d7763b843de58c1c3",  "LASRBL", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,    88,   0,  0},    // Laser Blast (1981).bin
    {"0d1b3abf681a2fc9a6aa31a9b0e8b445",  "LASRBL", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   2,  6},    // Laser Blast (1981) (PAL).bin
    {"8a8e401369e2b63a13e18a4d685387c6",  "LASRBL", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   59,    245,   100,   2,  6},    // Laser Blast (1981) (PAL).bin
    {"1fa58679d4a39052bd9db059e8cda4ad",  "LASERG", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  29,    210,   100,   0,  0},    // Laser Gates (1983).bin
    {"b59fd465abf76f64c85652ff29d5952d",  "LASERG", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  29,    210,   100,   0,  2},    // Laser Gates (1983).bin (Innerspace)
    {"68760b82fc5dcf3fedf84376a4944bf9",  "LASERG", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  29,    210,   100,   0,  0},    // Laser Gates (1983).bin
    {"48287a9323a0ae6ab15e671ac2a87598",  "LASERG", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  29,    210,   100,   0,  0},    // Laser Gates (1983).bin
    {"cd4ded1ede63c4dd09f3dd01bda7458c",  "LASERG", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   25,    245,   100,   0,  0},    // Laser Gates (1983) (PAL).bin
    {"8e4cd60d93fcde8065c1a2b972a26377",  "LASERG", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   35,    245,   100,   0,  0},    // Laser Gates (1983) (PAL).bin
    {"bce4c291d0007f16997faa5c4db0a6b8",  "LASERG", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   35,    245,   100,   0,  0},    // Laser Gates (1983) (PAL).bin
    {"105918c03562a262c56c145fa736465d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Laserblast (2007) (Nick Poirer, David Poore, Jenny Rainwater).bin
    {"1fab68fd67fe5a86b2c0a9227a59bb95",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    210,    82,   0,  0},    // Lasercade (1983).bin
    {"8a171b3376d1711bdda68df0f304e776",  "LASERG", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,   100,   0,  0},    // Last Mobile Space Battalion (2008) (Phillip Sassano).bin
    {"31e518debba46df6226b535fa8bd2543",  "SOLARI", BANK_F6,   CTR_LJOY,      CTR_SOLARIS,    MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  5},    // Last Starfighter (1984).bin
    {"f8692dc7fb9dddc76f066ede765f048d",  "SOLARI", BANK_F6,   CTR_GENESIS,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  5},    // Last Starfighter (2 Button Hack).bin
    {"7507f70476746d1d9dd8ec0d6b1ca05f",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  30,    210,   100,   0,  0},    // Lawn Mow (2007) (Laura DeMichelis, Daniel Fuller).bin
    {"021dbeb7417cac4e5f8e867393e742d6",  "LEAD01", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Lead (16k).bin
    {"c98ff002205095c8a40f7a537a7e8f01",  "LEAD01", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Lead (8k).bin
    {"a55e0773d40de38b67d80d2852d7b538",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Legendary Spear.bin
    {"5fd4239051c76563eed086f512b4cd48",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  28,    210,    90,   4,  0},    // Lili, Artkaris (Argentina) (NTSC).bin
    {"3947eb7305b0c904256cdbc5c5956c0f",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  53,    210,    81,   4,  0},    // Lilly Adventure.bin
    {"d0cdafcb000b9ae04ac465f17788ad11",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   70,    245,   100,   4,  0},    // Lilly Adventure - Alices Adventure (PAL).bin
    {"ab10f2974dee73dab4579f0cab35fca6",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   70,    245,   100,   4,  0},    // Lilly Adventure (1983) (PAL).bin
    {"86128001e69ab049937f265911ce7e8a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  9},    // Lochjaw (1981).bin
    {"d46a55662cbbab137dc81e724e385c9a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  9},    // Lochjaw (1981).bin
    {"71464c54da46adae9447926fdbfc1abe",  "LOCKNC", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  5},    // Lock 'n' Chase (1982).bin
    {"51c1ddc9d6d597f71fb7efb56012abec",  "LOCKNC", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  5},    // Lock 'n' Chase (1982).bin
    {"493e90602a4434b117c91c95e73828d1",  "LOCKNC", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   62,    245,   100,   0,  1},    // Lock 'n' Chase (1982) (PAL).bin
    {"e88340f5bd2f03e2e9ce5ecfa9c644f5",  "LOCKNC", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   62,    245,   100,   0,  1},    // Lock 'n' Chase (1982) (PAL).bin
    {"3ff5165378213dab531ffa4f1a41ae45",  "LOCKNC", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  9},    // Lock 'n' Chase (1982) (PAL).bin
    {"c43fc3242c1b36bf36e065bab851ab5f",  "??????", BANK_CDFJ, CTR_LJOY,      SPEC_DPCPOPT,   MODE_NO,   !VB,  !HB,  ANA1_0,  NTSC,  34,    205,    97,   0,  0},    // LodeRunner2600-Demo-NTSC.bin
    {"b4e2fd27d3180f0f4eb1065afc0d7fc9",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // London Blitz (1983).bin
    {"5babe0cad3ec99d76b0aa1d36a695d2f",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  29,    210,   100,   0,  0},    // Looping (1983).bin
    {"e24d7d879281ffec0641e9c3f52e505a",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    94,   0,  0},    // Lord of the Rings (1983).bin
    {"c6d7fe7a46dc46f962fe8413c6f53fc9",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    94,   0,  0},    // Lord of the Rings (1983).bin
    {"7c1a6f8163cad8a5ce3c872793e8d79c",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  31,    210,   100,   0,  0},    // Lost Cat (V11).bin
    {"2d76c5d1aad506442b9e9fb67765e051",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Lost Luggage (1981) [no opening scene].bin
    {"7c00e7a205d3fda98eb20da7c9c50a55",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Lost Luggage (1981).bin
    {"d0b26e908370683ad99bc6b52137a784",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   4,  4},    // Lost Luggage (1981) (PAL).bin
    {"a0f9a076ef9ca7555b051835b6e59970",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  5},    // LowResCarJoyRC1.bin
    {"d66e72d4b623551a3779779f33730d36",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Lunokhod (b8).bin
    {"593268a34f1e4d78d94729e1d7ee8367",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Lunokhod.bin
    {"393e41ca8bdd35b52bf6256a968a9b89",  "M.A.D.", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  5},    // M.A.D. (Missile Intercept) (1982).bin
    {"090f0a7ef8a3f885048d213faa59b2f8",  "M.A.D.", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // M.A.D. (Missile Intercept) (1982) (PAL).bin
    {"adf1afac3bdd7b36d2eda5949f1a0fa3",  "M.A.D.", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // M.A.D. (Missile Intercept) (1982) (PAL).bin
    {"835759ff95c2cdc2324d7c1e7c5fa237",  "MASH00", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    97,   4,  1},    // M.A.S.H (1983).bin
    {"1423f560062c4f3c669d55891a2bcbe7",  "MASH00", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    97,   4,  1},    // M.A.S.H (1983).bin
    {"cf63ffac9da89ef09c6c973083061a47",  "MASH00", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    98,   4,  1},    // M.A.S.H (1983).bin
    {"43c6cfffeddab6b3787357fed9d44529",  "MASH00", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   45,    245,    54,   4,  1},    // M.A.S.H (1983) (PAL).bin
    {"e97eafd0635651d3999cece953c06bd5",  "MASH00", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   60,    245,    97,   4,  1},    // M.A.S.H (1983) (PAL).bin
    {"65ba1a4c643d1ab44481bdddeb403827",  "MASH00", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,    95,   4, 10},    // M.A.S.H (1983) (PAL).bin
    {"cddabfd68363a76cd30bee4e8094c646",  "??????", BANK_CV,   CTR_KEYBOARD1, SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    90,   0,  0},    // MagiCard (1981).bin
    {"ccb5fa954fb76f09caae9a8c66462190",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Malagai (1983).bin
    {"402d876ec4a73f9e3133f8f7f7992a1e",  "MANGOS", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  6},    // Man Goes Down.bin
    {"54a1c1255ed45eb8f71414dadb1cf669",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Mangia' (1983).bin
    {"d8295eff5dcc43360afa87221ea6021f",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,    89,   0, 14},    // Mangia' (1983) (PAL).bin
    {"13895ef15610af0d0f89d588f376b3fe",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Marauder (1982).bin
    {"cc03c68b8348b62331964d7a3dbec381",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Marauder (1982).bin
    {"512e874a240731d7378586a05f28aec6",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   65,    245,   100,   0,  0},    // Marauder (1982) (PAL).bin
    {"09a03e0c85e667695bcd6c6394e47e5f",  "??????", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Marble Craze ntsc_7_26_3.bin
    {"b00e8217633e870bf39d948662a52aac",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Marine Wars (1983).bin
    {"cade123747426df69570a2bc871d3baf",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Marine Wars (1983) (PAL).bin
    {"e908611d99890733be31733a979c62d8",  "MARIOB", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Mario Bros. (1983).bin
    {"9a165c39af3f050fdee6583fdfcdc9be",  "MARIOB", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Mario Bros. (1983).bin
    {"b1d1e083dc9e7d9a5dc1627869d2ade7",  "MARIOB", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Mario Bros. (1983).bin
    {"c49fe437800ad7fd9302f3a90a38fb7d",  "MARIOB", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Mario Bros. (1983) (PAL).bin
    {"ae4be3a36b285c1a1dff202157e2155d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Master Builder (1983).bin
    {"36e47ed74968c365121eab60f48c6517",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   72,    245,   100,   0,  0},    // Master Builder (1983) (PAL).bin
    {"677f45b569096ecebf069696bfdacad2",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Master Mind Deluxe (2006) (Cye Freeman).bin
    {"3b76242691730b2dd22ec0ceab351bc6",  "??????", BANK_E7,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Masters of the Universe (1983).bin
    {"470878b9917ea0348d64b5750af149aa",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Math Gran Prix (1982).bin
    {"5e2495d43b981010304af55efed1e798",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Math Gran Prix (1982).bin
    {"01e5c81258860dd82f77339d58bc5f5c",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Math Gran Prix (1982).bin
    {"7996b8d07462a19259baa4c811c2b4b4",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   66,    245,   100,   0,  0},    // Math Gran Prix (1982) (PAL).bin
    {"45beef9da1a7e45f37f3f445f769a0b3",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   66,    245,   100,   0,  0},    // Math Gran Prix (1982) (PAL).bin
    {"f825c538481f9a7a46d1e9bc06200aaf",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Maze Craze (1980).bin
    {"8108ad2679bd055afec0a35a1dca46a4",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Maze Craze (1980).bin
    {"0fbf618be43d4396856d4244126fe7dc",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   65,    245,   100,   0,  0},    // Maze Craze (1980) (PAL).bin
    {"ed2218b3075d15eaa34e3356025ccca3",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   65,    245,   100,   0,  0},    // Maze Craze (1980) (PAL).bin
    {"0e27164de87e1ea190d3fb39b9a09613",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Maze Demo.bin
    {"afab14643c8a2689e2c6d7f6e41f7b44",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Minotaur Version 7 Demo.bin
    {"35b43b54e83403bb3d71f519739a9549",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // McDonald's - Golden Arches (1983).bin
    {"f7fac15cf54b55c5597718b6742dbec2",  "MAYHAM", BANK_F4,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    90,   0,  0},    // Medieval Mayhem.bin
    {"b65d4a38d6047735824ee99684f3515e",  "??????", BANK_MB,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    225,   100,   0,  6},    // MegaBoy.bin
    {"bf7f1925434226fad356b479c5f89bc5",  "??????", BANK_X07,  CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   34,    210,   100,   0,  6},    // MegaBoy.bin (X07 Conversion)
    {"daeb54957875c50198a7e616f9cc8144",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Mega Force (1982).bin
    {"eb503cc64c3560cd78b7051188b7ba56",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Mega Force (1982).bin
    {"bdbaeff1f7132358ea64c7be9e46c1ac",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   63,    245,   100,   0,  0},    // Mega Force (1982) (PAL).bin
    {"ecf51385384b468834611d44a8429c03",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   63,    245,   100,   0,  0},    // Mega Force (1982) (PAL).bin
    {"318a9d6dda791268df92d72679914ac3",  "MEGAMA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    198,   100,   3,  4},    // MegaMania (1982).bin
    {"6bb22efa892b89b69b9bf5ea547e62b8",  "MEGAMA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    198,   100,   3,  4},    // MegaMania (1982).bin
    {"d5618464dbdc2981f6aa8b955828eeb4",  "MEGAMA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    198,   100,   3,  4},    // MegaMania (1982).bin
    {"12937db3d4a80da5c4452b752891252d",  "MEGAMA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    198,   100,   3,  4},    // MegaMania (1982).bin
    {"3278158e5c1f7eb5c5d28ccfd7285250",  "MEGAMA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    198,   100,   3,  4},    // MegaMania (1982).bin
    {"cd8bd5af3e0e2ebd7930b456dc3babad",  "MEGAMA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    198,   100,   3,  4},    // MegaMania (Atarimania hack).bin
    {"39da69ff9833f8c143f03b6e0e7a996b",  "MEGAMA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    198,   100,   3,  4},    // MegaMania (Ventrra Invaders 2002 hack).bin
    {"3d934bb980e2e63e1ead3e7756928ccd",  "MEGAMA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   3,  4},    // MegaMania (1982) (PAL).bin
    {"1a624e236526c4c8f31175e9c89b2a22",  "MEGAMA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   36,    245,   100,   3,  4},    // MegaMania (1982) (PAL).bin
    {"345769d085113d57937198262af52298",  "MEGAMA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   48,    245,   100,   3,  4},    // MegaMania (1982) (PAL).bin
    {"6604f72a966ca6b2df6a94ee4a68eb82",  "MEGAMA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   56,    245,   100,   3,  4},    // MegaMania (1982) (PAL).bin
    {"049626cbfb1a5f7a5dc885a0c4bb758e",  "MEGAMA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   56,    245,   100,   3,  4},    // MegaMania (1982) (PAL).bin
    {"96e798995af6ed9d8601166d4350f276",  "MELTDN", BANK_4K,   CTR_LJOY,      SPEC_MELTDOWN,  MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  33,    210,    91,   0, -1},    // Meltdown (Prototype) (1983).bin
    {"712924a2c7b692f6e7b009285c2169a7",  "??????", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Meteoroids (1982).bin
    {"909d01739a22579556b740e933915fd8",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  35,    210,    90,   0,  0},    // Mid Space (v43).bin
    {"f1554569321dc933c87981cf5c239c43",  "MIDMAG", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_FF,   !VB,   HB,  ANA1_0,  NTSC,  38,    200,   100,   0,  0},    // Midnight Magic (1984).bin
    {"da732c57697ad7d7af414998fa527e75",  "MIDMAG", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_FF,   !VB,   HB,  ANA1_0,  PAL,   63,    230,   100,   0,  1},    // Midnight Magic (1984) (PAL).bin
    {"3c57748c8286cf9e821ecd064f21aaa9",  "MIPEDE", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  1},    // Millipede (1984).bin
    {"11bcf5c752088b5aaf86d6c7a6a11e8d",  "MIPEDE", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  1},    // Millipede (1984).bin
    {"0bf19e40d5cd8aa5afb33b16569313e6",  "MIPEDE", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  1},    // Millipede (1984).bin
    {"a7673809068062106db8e9d10b56a5b3",  "MIPEDE", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   66,    245,   100,   0,  0},    // Millipede (1984) (PAL).bin
    {"efd387430a35a659ff569a9a0ec22209",  "MIPEDE", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   66,    245,   100,   0,  0},    // Millipede (1984) (PAL).bin
    {"0e224ea74310da4e7e2103400eb1b4bf",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Mind Maze (1984).bin
    {"3b040ed7d1ef8acb4efdeebebdaa2052",  "??????", BANK_3F,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  31,    210,    79,  -1,  0},    // Miner 2049er (1982) [fixed].bin
    {"fa0570561aa80896f0ead05c46351389",  "??????", BANK_3F,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  31,    210,    79,  -1,  0},    // Miner 2049er (1982).bin
    {"598a4e6e12f8238b7e7555f5a7777b46",  "??????", BANK_3F,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  31,    210,    55,   0,  1},    // Miner 2049er (1982).bin
    {"c64f5f9faef02a4323482abbe7296aa4",  "??????", BANK_3F,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  24,    210,    75,   0, -2},    // Miner 2049er (Rob Hack).bin
    {"c103fb01b4b922df0c04078c48774bb0",  "??????", BANK_3F,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  24,    210,    75,   0, -2},    // Miner 2049er (Bob Hack).bin
    {"c517144e3d3ac5c06f2f682ebf212dd7",  "??????", BANK_3F,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   50,    245,    79,  -1,  4},    // Miner 2049er (1982) (PAL).bin
    {"2a1b454a5c3832b0240111e7fd73de8a",  "??????", BANK_3F,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  28,    210,    79,   0,  0},    // Miner 2049er Vol II (1983).bin
    {"468f2dec984f3d4114ea84f05edf82b6",  "??????", BANK_3F,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   61,    245,    80,   0,  0},    // Miner 2049er Vol II (1983) (PAL).bin
    {"4543b7691914dfd69c3755a5287a95e1",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Mines of Minos (1982).bin
    {"73cb1f1666f3fd30b52b4f3d760c928f",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   43,    245,    64,   0,  0},    // Mines of Minos (1982) (PAL).bin
    {"b5cb9cf6e668ea3f4cc2be00ea70ec3c",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   43,    245,    64,   0,  0},    // Mines of Minos (1982) (PAL).bin
    {"df62a658496ac98a3aa4a6ee5719c251",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    90,   0,  6},    // Miniature Golf (1979).bin
    {"bdc6a304623b3f80500f344666ca075f",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    95,   0,  6},    // Miniature Golf+ (2002).bin
    {"ed5ccfc93ad4561075436ee42a15438a",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   45,    245,    51,   0,  0},    // Miniature Golf (1979) (PAL).bin
    {"73521c6b9fed6a243d9b7b161a0fb793",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   45,    245,    51,   0,  0},    // Miniature Golf (1979) (PAL).bin
    {"bb6fc47aed82b3f65c4938bf668de767",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  31,    210,   100,   0,  0},    // Minotaur (2006) (Michael Biggs).bin
    {"4181087389a79c7f59611fb51c263137",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  25,    210,    93,   0,  0},    // Miss Piggy's Wedding (1983).bin
    {"25e73efb9a6edf119114718bd2f646ba",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  25,    210,    93,   0,  0},    // Miss Piggy's Wedding (1983).bin
    {"855a42078b14714bcfd490d2cf57e68d",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  25,    210,    93,   0,  0},    // Miss Piggy's Wedding (1983).bin
    {"1a8204a2bcd793f539168773d9ad6230",  "MISCOM", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  NTSC,  34,    205,    94,   0,  4},    // Missile Command (1981) [no initials].bin
    {"3a2e2d0c6892aa14544083dfb7762782",  "MISCOM", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  NTSC,  34,    205,    94,   0,  4},    // Missile Command (1981).bin
    {"7cedffa0db65d610568b90aeca705ac6",  "MISCOM", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  NTSC,  34,    205,    91,   0,  2},    // Missile Command (1981).bin
    {"5ced13931c21ef4fc77d3fe801a1cbfa",  "MISCOM", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  NTSC,  34,    205,    94,   0,  4},    // Missile Command (1981).bin
    {"d1ca47b262f952413c1234117c4e4e21",  "MISCOM", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  NTSC,  34,    205,    94,   0,  4},    // Missile Command (1981).bin
    {"9364ad51c321e0f15c96a8c0aff47ceb",  "MISCOM", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  PAL,   47,    245,    51,   0,  0},    // Missile Command (1981) (PAL).bin
    {"6e51fe2c2007687308d47ea9ce45432e",  "MISCOM", BANK_F8,   CTR_MCA,       SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    91,   0,  5},    // Missile_command_arcade.bin
    {"e6e5bb0e4f4350da573023256268313d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  35,    210,   100,   0, 12},    // Missile Control (AKA Raketen-Angriff) (Ariola, Thomas Jentzsch).bin
    {"0b577e63b0c64f9779f315dca8967587",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   64,    245,   100,   0,  0},    // Missile Control (AKA Raketen-Angriff) (PAL).bin
    {"cb24210dc86d92df97b38cf2a51782da",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   68,    245,   100,   0,  0},    // Missile Control (AKA Raketen-Angriff) (PAL).bin
    {"4c6afb8a44adf8e28f49164c84144bfe",  "MI3000", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Mission 3000 A.D. (1983).bin
    {"b83579c4450fcbdf2b108903731fa734",  "MI3000", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Mission 3000 A.D. (1983) (PAL).bin
    {"cfad2b9ca8b8fec7fb1611d656cc765b",  "MI3000", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Mission 3000 A.D. (1983) (PAL).bin
    {"6efe876168e2d45d4719b6a61355e5fe",  "MI3000", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Mission 3000 A.D. (1983) (PAL).bin
    {"b676a9b7094e0345a76ef027091d916b",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  38,    210,   100,   0, -1},    // Mission Survive (1983) (Video Gems)(NTSC).bin
    {"6a5ae550cce93d03a58ba7cf546d1cd8",  "??????", BANK_4K,   CTR_GENESIS,   SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  38,    210,   100,   0, -1},    // Mission Survive (1983) (Video Gems)(NTSC 2 Button Hack).bin
    {"b5cdbab514ea726a14383cff6db40e26",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  6},    // Mission Survive (1983) (Video Gems)(PAL).bin
    {"cf9069f92a43f719974ee712c50cd932",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   57,    245,   100,   0,  0},    // Mission Survive (1983) (Video Gems)(PAL).bin
    {"e13818a5c0cb2f84dd84368070e9f099",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   2,  1},    // Misterious Thief (1983).bin
    {"5d9592756425192ec621d2613d0e683d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   2,  1},    // Misterious Thief (1983).bin
    {"07973be3ecfd55235bf59aa56bdef28c",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   2,  0},    // Misterious Thief (1983) (PAL).bin
    {"5c554f7984684a1704bed282300fc7c1",  "??????", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // MMC2005.bin
    {"bbe5df7f206fcb7f626c995b14df79c2",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    210,   100,   0,  0},    // mmsbcII.bin
    {"3d48b8b586a09bdbf49f1a016bf4d29a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,    84,   0,  0},    // Mole Hunter (AKA Topy).bin
    {"01abcc1d2d3cba87a3aa0eb97a9d7b9c",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  29,    210,    84,   0,  0},    // Mole Hunter (AKA Topy).bin
    {"f802fa61011dd9eb6f80b271bac479d0",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   59,    245,    84,   3,  1},    // Mole Hunter (AKA Topy) (PAL).bin
    {"7af40c1485ce9f29b1a7b069a2eb04a7",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   4,  0},    // Mogul Maniac (1983).bin
    {"2a043ced6586381d9e04bc10ac5cbf2d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  27,    210,    80,   4,  0},    // MonkeyKing_NTSC_rc6.bas.bin
    {"6913c90002636c1487538d4004f7cac2",  "??????", BANK_F8,   CTR_KEYBOARD0, SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Monstercise.bin
    {"3347a6dd59049b15a38394aa2dafa585",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,  !HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  0},    // Montezuma's Revenge (1984).bin
    {"515046e3061b7b18aa3a551c3ae12673",  "MOONPA", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Moon Patrol (1983).bin
    {"7b8a481e0c5aa78150b5555dff01f64e",  "MOONPA", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Moon Patrol (1983).bin
    {"ac3dd22dd945724be705ddd2785487c2",  "MOONPA", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Moon Patrol (1983).bin
    {"e2c1b60eaa8eda131632d73e4e0c146b",  "MOONPA", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Moon Patrol (1983).bin
    {"5256f68d1491986aae5cfdff539bfeb5",  "MOONPA", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Moon Patrol (1983).bin
    {"94ff6b7489ed401dcaaf952fece10f67",  "MOONPA", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Moon Patrol (1983).bin
    {"6de924c2297c8733524952448d54a33c",  "MOONPA", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Moon Patrol (1983).bin
    {"2854e5dfb84173fafc5bf485c3e69d5a",  "MOONPA", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Moon Patrol (1983).bin
    {"1b22a3d79ddd79335b69c94dd9b3e44e",  "MOONPA", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Moon Patrol (1983).bin
    {"5643ee916f7dc760148fca4db3aa7d10",  "MOONPA", BANK_F8,   CTR_GENESIS,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Moon Patrol (2 Button Hack).bin
    {"44e9c4a047c348dbeb7ace60f45484b4",  "MOONPA", BANK_F8,   CTR_GENESIS,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Moon Patrol Arcade (2 Button Hack).bin
    {"65490d61922f3e3883ee1d583ce10855",  "MOONPA", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   54,    245,   100,   0,  0},    // Moon Patrol (1983) (PAL).bin
    {"203abb713c00b0884206dcc656caa48f",  "MOONSW", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Moonsweeper (1983).bin
    {"b06050f686c6b857d0df1b79fea47bb4",  "MOONSW", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Moonsweeper (1983).bin
    {"42249ec8043a9a0203dde0b5bb46d8c4",  "MOONSW", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Moonsweeper (1983).bin
    {"4af4103759d603c82b1c9c5acd2d8faf",  "MOONSW", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   43,    245,    61,   0,  0},    // Moonsweeper (1983) (PAL).bin
    {"7d1034bcb38c9b746ea2c0ae37d9dff2",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Morse Code Tutor.bin
    {"9c20e20fa4c5fad038b3e5cee3706c0b",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Moth 7.2.bin
    {"5641c0ff707630d2dd829b26a9f2e98f",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Motocross Racer (1990).bin
    {"de0173ed6be9de6fd049803811e5f1a8",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Motocross Racer (1990).bin
    {"f5a2f6efa33a3e5541bc680e9dc31d5b",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   67,    245,    68,   0,  0},    // Motocross (1983) (PAL).bin
    {"a20b7abbcdf90fbc29ac0fafa195bd12",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   67,    245,    68,   0,  0},    // Motocross (1983) (PAL).bin
    {"378a62af6e9c12a760795ff4fc939656",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // MotoRodeo (1990).bin
    {"b1e2d5dc1353af6d56cd2fe7cfe75254",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   45,    245,   100,   0,  0},    // MotoRodeo (1990) (PAL).bin
    {"db4eb44bc5d652d9192451383d3249fc",  "??????", BANK_FASC, CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Mountain King (1983).bin
    {"5678ebaa09ca3b699516dba4671643ed",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Mouse Trap (1982).bin
    {"35156407e54f67eb1f625450d5c093e1",  "??????", BANK_4K,   CTR_GENESIS,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Mouse Trap (2 Button Hack).bin
    {"23d445ea19a18fb78d5035878d9fb649",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   61,    245,   100,   0,  0},    // Mouse Trap (1982) (PAL).bin
    {"0164f26f6b38a34208cd4a2d0212afc3",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  31,    210,   100,   0,  0},    // Mr. Do! (1983).bin
    {"aa7bb54d2c189a31bb1fa20099e42859",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   58,    245,   100,   0,  0},    // Mr. Do! (1983) (PAL).bin
    {"b7a7e34e304e4b7bc565ec01ba33ea27",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Mr. Do!'s Castle (1984) [alt].bin
    {"97184b263722748757cfdc41107ca5c0",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  5},    // Mr. Do!'s Castle (1984).bin
    {"b8e715223ba65cf716b3620a90ca3ec1",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  5},    // Mr. Do!'s Castle (1984).bin
    {"8644352b806985efde499ae6fc7b0fec",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Mr. Postman (1983).bin
    {"f0daaa966199ef2b49403e9a29d12c50",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Mr. Postman (1983).bin
    {"0ecdb07bf9b36ef18f3780ef48e6c709",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Mr. Postman (1983).bin
    {"2327456f86d7e0deda94758c518d05b3",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Mr. Postman (1983).bin
    {"2d16a8b59a225ea551667be45f554652",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   55,    245,   100,   0,  0},    // Mr. Postman (1983) (PAL).bin
    {"603c7a0d12c935df5810f400f3971b67",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   55,    245,   100,   0,  0},    // Mr. Postman (1983) (PAL).bin
    {"b54be87652cd9af6095f7d7f154762c6",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Mr_Yo-Yo_NTSC_FINAL_DIGITAL.bin
    {"87e79cd41ce136fd4f72cc6e2c161bee",  "MSPACM", BANK_F8,   CTR_LJOY,      SPEC_WAVEDIRC,  MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,    98,   0,  0},    // Ms. Pac-Man (1982).bin
    {"ccd92a269a4c2bd64d58cf2c0114423c",  "MSPACM", BANK_F8,   CTR_LJOY,      SPEC_WAVEDIRC,  MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,    98,   0,  0},    // Ms. Pac-Man (1982).bin
    {"1ea980574416bfd504f62575ba524005",  "MSPACM", BANK_F8,   CTR_LJOY,      SPEC_WAVEDIRC,  MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,    98,   0,  0},    // Ms. Pac-Man (1982).bin
    {"9469d18238345d87768e8965f9f4a6b2",  "MSPACM", BANK_F8,   CTR_LJOY,      SPEC_WAVEDIRC,  MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,    98,   0,  0},    // Ms. Pac-Man (1982).bin
    {"1ee9c1ba95cef2cf987d63f176c54ac3",  "MSPACM", BANK_F8,   CTR_LJOY,      SPEC_WAVEDIRC,  MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,    98,   0, 11},    // Ms. Pac-Man (1982) (PAL).bin
    {"391764720140c432aec454a468f77a40",  "MSPACM", BANK_F8,   CTR_LJOY,      SPEC_WAVEDIRC,  MODE_NO,    VB,   HB,  ANA1_0,  PAL,   49,    245,    94,   0, 11},    // Ms. Pac-Man (1982) (PAL).bin
    {"cd7ee8477d7b42aba3700262761eaff8",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Msgalactopus-FIX2.bin
    {"4dff6094568a5467ff9b1181df34a716",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  30,    210,   100,   0,  0},    // MSJoystick2.3.bin
    {"ddf72763f88afa541f6b52f366c90e3a",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    96,   0,  1},    // Muncher.bin
    {"aaac0d277eda054861e613c59c2e4ff2",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Music Demo.bin
    {"65b106eba3e45f3dab72ea907f39f8b4",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Music Machine (1983).bin
    {"0546f4e6b946f38956799dd00caab3b1",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  5},    // My Golf (HES, Thomas Jentzsch) (NTSC).bin
    {"dfad86dd85a11c80259f3ddb6151f48f",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // My Golf (1990) (PAL).bin
    {"04fccc7735155a6c1373d453b110c640",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // My Golf (1990) (PAL).bin
    {"ee6cbedf6c0aac90faa0a8dbc093ffbe",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // My Golf (1990) (PAL).bin
    {"fcbbd0a407d3ff7bf857b8a399280ea1",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Mysterious Thief (1983).bin
    {"48f18d69799a5f5451a5f0d17876acef",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Mysterious Thief (1983).bin
    {"36306070f0c90a72461551a7a4f3a209",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Name This Game (1982).bin
    {"f98d2276d4a25b286135566255aea9d0",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Name This Game (1982).bin
    {"0fee596b974c9d3e70b367a3671599b6",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Name This Game (1982).bin
    {"2f7949f71076db42480d3f5036b4a332",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  9},    // Name This Game (1982) (PAL).bin
    {"b392964e8b1c9c2bed12246f228011b2",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  9},    // Name This Game (1982) (PAL).bin
    {"45cb0f41774b78def53331e4c3bf3362",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  9},    // Name This Game (1982) (PAL).bin
    {"6ac169c723525f061f12b39f3f3f7bb1",  "??????", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // NewPeril-RC1.bin
    {"392f00fd1a074a3c15bc96b0a57d52a1",  "??????", BANK_2K,   CTR_PADDLE0,   SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Night Driver (1980).bin
    {"feec54aac911887940b47fe8c9f80b11",  "??????", BANK_2K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   22,    245,    56,   0, 18},    // Night Driver (1980) (PAL).bin
    {"27f9e2e1b92af9dc17c6155605c38e49",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Nightmare (CCE).bin
    {"ed0ab909cf7b30aff6fc28c3a4660b8e",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Nightmare.bin
    {"b1339c56a9ea63122232fe4328373ac5",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   83,    245,    80,   0,  0},    // Nightmare (PAL).bin
    {"ead60451c28635b55ca8fea198444e16",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   65,    245,    83,   0, 18},    // Nightmare (PAL).bin
    {"63ccbde4fbff7c3dbb52f78f3edbb01b",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Nightrider02NTSC.bin
    {"2df8ea51bcc9f1b3b4c61a141b5a1405",  "NINJAG", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // NinjishGuy_prerelease.bin
    {"9e25d77f17008847d789d68bd1948ce1",  "NINJAG", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // NinjishGuy_prerelease.bin
    {"1c11997364ae24a2e10f929b898226e2",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  30,    210,   100,   0,  0},    // Nite Bear.bin
    {"b6d52a0cf53ad4216feb04147301f87d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // No Escape! (1982).bin
    {"dc81c4805bf23959fcf2c649700b82bf",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // No Escape! (1982) (PAL).bin
    {"de7a64108074098ba333cc0c70eef18a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Nuts (Unknown).bin
    {"e3c35eac234537396a865d23bafb1c84",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   49,    245,    89,   0,  0},    // Nuts (TechnoVision) (PAL).bin
    {"9ed0f2aa226c34d4f55f661442e8f22a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   64,    245,   100,   0,  0},    // Nuts (TechnoVision) (PAL).bin
    {"76ee917d817ef9a654bc4783e0273ac4",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   64,    245,   100,   0,  0},    // Nuts (TechnoVision) (PAL).bin
    {"133a4234512e8c4e9e8c5651469d4a09",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Obelix (1983).bin
    {"a189f280521f4e5224d345efb4e75506",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  36,    210,   100,   0,  4},    // Obelix (Atari, Thomas Jentzsch) (NTSC).bin
    {"19e739c2764a5ab9ed08f9095aa2af0b",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   62,    250,   100,   0,  0},    // Obelix (Atari) (PAL).bin
    {"67afbf02142c66861a1d682e4c0ea46e",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Odin-joystick.bin
    {"e20fc6ba44176e9149ef6d3f99221ac5",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Odin.bin
    {"98f63949e656ff309cefa672146dc1b8",  "OFFWAL", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Off the Wall (1989).bin
    {"36edef446ab4c2395666efc672b92ed0",  "OFFWAL", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   56,    245,   100,   0,  0},    // Off the Wall (1989) (PAL).bin
    {"b6166f15720fdf192932f1f76df5b65d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    93,   0,  0},    // Off Your Rocker (1983).bin
    {"c73ae5ba5a0a3f3ac77f0a9e14770e73",  "OFFROG", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    200,    95,   0,  0},    // Official Frogger (1983).bin
    {"a74689a08746a667a299b0507e1e6dd9",  "OFFROG", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_NO,    VB,   HB,  ANA1_0,  PAL,   57,    225,    95,   0,  0},    // Official Frogger (1983) (PAL).bin
    {"c9c25fc536de9a7cdc5b9a916c459110",  "OINK!!", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  5},    // Oink! (1982).bin
    {"f8648d0c6ad1266434f6c485ff69ec40",  "OINK!!", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  5},    // Oink! (1982).bin
    {"853c11c4d07050c22ef3e0721533e0c5",  "OINK!!", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   4, 12},    // Oink! (1982) (PAL).bin
    {"06b6c5031b8353f3a424a5b86b8fe409",  "OINK!!", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   4, 12},    // Oink! (1982) (PAL).bin
    {"0de53160a8b54c3aa5aed8d68c970b62",  "OINK!!", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   4, 12},    // Oink! (1982) (PAL).bin
    {"cca33ae30a58f39e3fc5d80f94dc0362",  "??????", BANK_2K,   CTR_DRIVING,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Okie dokie (pd).bin
    {"9947f1ebabb56fd075a96c6d37351efa",  "??????", BANK_FASC, CTR_BOOSTER,   SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0, 12},    // Omega Race (1983).bin
    {"336ea20d38f98926919d4b4651d1a03f",  "??????", BANK_FASC, CTR_GENESIS,   SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0, 12},    // Omega Race (2 Button Hack).bin
    {"bc593f2284c67b7d8716d110f541953f",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Omicron 16k.bin
    {"2148917316ca5ce1672f6c49c0f89d0b",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Omicron 2k.bin
    {"eb19444ab669335b00a35aa0a8d67c93",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  27,    210,   100,   0,  1},    // Ooze0.85.bas.bin
    {"52385334ac9e9b713e13ffa4cc5cb940",  "OPENSE", BANK_4K,   CTR_LJOY,      SPEC_WAVEDIRC,  MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    96,   0,  0},    // Open, Sesame! (1983).bin
    {"e73838c43040bcbc83e4204a3e72eef4",  "OPENSE", BANK_4K,   CTR_LJOY,      SPEC_WAVEDIRC,  MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    96,   0,  0},    // Open, Sesame! (1983).bin
    {"f6a282374441012b01714e19699fc62a",  "OPENSE", BANK_4K,   CTR_LJOY,      SPEC_WAVEDIRC,  MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    96,   0,  0},    // Open, Sesame! (1983).bin
    {"54836a8f23913e9a77c7f2665baf36ac",  "OPENSE", BANK_4K,   CTR_LJOY,      SPEC_WAVEDIRC,  MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    96,   0,  0},    // Open, Sesame! (1983).bin
    {"8786f4609a66fbea2cd9aa48ca7aa11c",  "OPENSE", BANK_4K,   CTR_LJOY,      SPEC_WAVEDIRC,  MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,    96,   0,  3},    // Open, Sesame! (1983) (PAL).bin
    {"90578a63441de4520be5324e8f015352",  "OPENSE", BANK_4K,   CTR_LJOY,      SPEC_WAVEDIRC,  MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,    96,   0,  3},    // Open, Sesame! (1983) (PAL).bin
    {"28d5df3ed036ed63d33a31d0d8b85c47",  "OPENSE", BANK_4K,   CTR_LJOY,      SPEC_WAVEDIRC,  MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,    96,   0,  3},    // Open, Sesame! (1983) (PAL).bin
    {"8c2fa33048f055f38358d51eefe417db",  "OPENSE", BANK_4K,   CTR_LJOY,      SPEC_WAVEDIRC,  MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,    96,   0,  3},    // Open, Sesame! (1983) (PAL).bin
    {"fa1b060fd8e0bca0c2a097dcffce93d3",  "??????", BANK_F8,   CTR_KEYBOARD1, SPEC_OSCAR,     MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Oscar's Trash Race (1983).bin
    {"cc7138202cd8f6776212ebfc3a820ecc",  "??????", BANK_F8,   CTR_KEYBOARD1, SPEC_OSCAR,     MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Oscar's Trash Race (1983).bin
    {"4b94fd272785d7ec6c95fb7279d0f522",  "??????", BANK_F8,   CTR_KEYBOARD1, SPEC_OSCAR,     MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Oscar's Trash Race (1983).bin   
    {"47911752bf113a2496dbb66c70c9e70c",  "??????", BANK_F8,   CTR_KEYBOARD1, SPEC_OSCAR,     MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Oscar's Trash Race (1983) (PAL).bin
    {"113cd09c9771ac278544b7e90efe7df2",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    93,   0,  2},    // Othello (1981) [no grid markers].bin
    {"55949cb7884f9db0f8dfcf8707c7e5cb",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    93,   0,  2},    // Othello (1981).bin
    {"2c3b9c171e214e9e46bbaa12bdf8977e",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    93,   0,  2},    // Othello (1981).bin
    {"00e19ebf9d0817ccfb057e262be1e5af",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   57,    245,    74,   0,  1},    // Othello (1981) [no grid markers] (PAL).bin
    {"a0e2d310e3e98646268200c8f0f08f46",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   57,    245,    74,   0,  1},    // Othello (1981) (PAL).bin
    {"6468d744be9984f2a39ca9285443a2b2",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   57,    245,    74,   0,  1},    // Othello (1981) (PAL).bin
    {"f97dee1aa2629911f30f225ca31789d4",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0, 13},    // Out of Control (1983).bin
    {"890c13590e0d8d5d6149737d930e4d95",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Outlaw (1978).bin
    {"22675cacd9b71dea21800cbf8597f000",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   59,    245,   100,   0,  2},    // Outlaw (1978) (PAL).bin
    {"2e3728f3086dc3e71047ffd6b2d9f015",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   59,    245,   100,   0,  2},    // Outlaw (1978) (PAL).bin
    {"a7523db9a33e9417637be0e71fa4377c",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   59,    245,   100,   0,  2},    // Outlaw (1978) (PAL).bin
    {"91f0a708eeb93c133e9672ad2c8e0429",  "OYSTRO", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Oystron (V2.9) (PD).bin
    {"6e372f076fb9586aff416144f5cfe1cb",  "PACMAN", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  NTSC,  32,    210,   100,   1,  0},    // Pac-Man (1982).bin
    {"651d2b6743a3a18b426bce2c881af212",  "PACMAN", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  NTSC,  32,    210,   100,   1,  0},    // Pac-Man (1982).bin
    {"ca53fc8fd8b3c4a7df89ac86b222eba0",  "PACMAN", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  NTSC,  32,    210,   100,   1,  0},    // Pac-Man (1982).bin
    {"b36040a2f9ecafa73d835d804a572dbf",  "PACMAN", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  NTSC,  32,    210,   100,   1,  0},    // Pac-Man (1982).bin
    {"fc2233fc116faef0d3c31541717ca2db",  "PACMAN", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  PAL,   52,    245,   97,    1,  0},    // Pac-Man (1982) (PAL).bin
    {"c2410d03820e0ff0a449fa6170f51211",  "PACMAN", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   1,  0},    // Pac-Man (1982) (PAL).bin
    {"880e45b99c785e9910450b88e69c49eb",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Pac-Man 4k.bin
    {"98d41ef327c58812ecc75bf1611ddced",  "PACM8K", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    204,    92,   0,  1},    // Pac-Man 8k.bin
    {"f3ce4c5e0a01a6960583adf9a60beed6",  "PACM8K", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    204,    98,   0,  1},    // Pac-Man 8k - fast corner fix.bin
    {"6e88da2b704916eb04a998fed9e23a3e",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Pac-Man_4k (debro).bin
    {"82bf0dff20cee6a1ed4bb834b00074e6",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,    80,   4,  1},    // Panda Chase (PAL).bin
    {"0e713d4e272ea7322c5b27d645f56dd0",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,    80,   4,  1},    // Panda Chase (PAL).bin
    {"f8582bc6ca7046adb8e18164e8cecdbc",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   36,    245,    80,   4,  1},    // Panda Chase (PAL).bin
    {"fb833ed50c865a9a505a125fc9d79a7e",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,    80,   4,  1},    // Panda Chase (PAL).bin
    {"0fcff6fe3b0769ad5d0cf82814d2a6d9",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,    80,   4,  1},    // Panda Chase (PAL).bin
    {"bf9a2045952d40e08711aa232a92eb78",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Panky the Panda [alt].bin
    {"25f69569b1defffcb64cb431edf3e093",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Panky the Panda 1.02 (NTSC).bin
    {"245f07c8603077a0caf5f83ee6cf8b43",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  41,    210,    74,   0, -1},    // Parachute (HomeVision, Thomas Jentzsch) (NTSC).bin
    {"714e13c08508ee9a7785ceac908ae831",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   64,    245,    95,   0,  2},    // Parachute (HomeVision) (PAL).bin
    {"8108162bc88b5a14adc3e031cf4175ad",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,    79,   0,  2},    // Parachute (HomeVision) (PAL).bin
    {"012b8e6ef3b5fd5aabc94075c527709d",  "??????", BANK_AR,   CTR_PADDLE0,   SPEC_AR,        MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  31,    210,    97,   0,  0},    // Party Mix (1983).bin
    {"7ef3ca08abde439c6ccca84693839c57",  "??????", BANK_AR,   CTR_PADDLE0,   SPEC_AR,        MODE_NO,   !VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  4},    // Party Mix (1983) (PAL).bin
    {"36c31bb5daeb103f488c66de67ac5075",  "??????", BANK_AR,   CTR_PADDLE0,   SPEC_AR,        MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  32,    210,   100,   0,  0},    // Party Mix - Bop a Buggy.bin
    {"b7300f7c94c8838b6b9a55e8d67f9fba",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Passthrough 2600 (2010) (Cliff Friedel).bin
    {"e40a818dac4dd851f3b4aafbe2f1e0c1",  "??????", BANK_4K,   CTR_KEYBOARD0, SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Peek-A-Boo (1984).bin
    {"04014d563b094e79ac8974366f616308",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Pengo (1984).bin
    {"87b6a17132fc32f576bc49ea18729506",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Pengo (1984) (PAL).bin
    {"212d0b200ed8b45d8795ad899734d7d7",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    90,   0,  1},    // Pepsi Invaders (1983).bin
    {"09388bf390cd9a86dc0849697b96c7dc",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Pete Rose Baseball (1988).bin
    {"2d63b452f897818c52b3fceeb080a4d0",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   65,    210,   100,   0,  0},    // Pete Rose Baseball (1988) (PAL).bin
    {"e959b5a2c882ccaacb43c32790957c2d",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Phantom II, Pirate (2006) (David Weavil).bin
    {"e9034b41741dcee64ab6605aba9de455",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  33,    210,    91,   0,  0},    // Phantom Tank (Digivision).bin
    {"6a222c26bcece3a510ddda21398f72c6",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  33,    210,    91,   0,  0},    // Phantom Tank (Digivision) [alt].bin
    {"1a613ce60fc834d4970e1e674b9196b3",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   54,    245,    69,   0,  0},    // Phantom Tank (Digivision) (PAL).bin
    {"b29359f7de62fed6e6ad4c948f699df8",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   54,    245,    69,   0,  0},    // Phantom Tank (Digivision) (PAL).bin
    {"7454786af7126ccc7a0c31fcf5af40f1",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   54,    245,    69,   0,  0},    // Phantom Tank (Digivision) (PAL).bin
    {"6b1fc959e28bd71aed7b89014574bdc2",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   41,    245,    62,   0,  1},    // Phantom Tank (Digivision) (PAL).bin
    {"5a9d188245aff829efde816fcade0b16",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   54,    245,    69,   0,  0},    // Phantom Tank (Digivision) (PAL).bin
    {"2ae700c9dba843a68dfdca40d7d86bd6",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  44,    230,    58,   0,  0},    // Pharaoh's Curse (1983) (NTSC by TJ).bin
    {"62f74a2736841191135514422b20382d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  44,    230,    58,   0,  0},    // Pharaoh's Curse (1983) (NTSC conv).bin
    {"3577e19714921912685bb0e32ddf943c",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   47,    245,    58,   0,  1},    // Pharaoh's Curse (1983) (PAL).bin
    {"a69f5b1761a8a11c98e706ec7204937f",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   47,    250,    58,   0,  1},    // Pharaoh's Curse (1983) (PAL).bin
    {"7dcbfd2acc013e817f011309c7504daa",  "PHASER", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    192,   100,   0,  0},    // Phaser Patrol (1982).bin
    {"72305c997f2cec414fe6f8c946172f83",  "PHASER", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_NO,    VB,   HB,  ANA1_0,  PAL,   40,    230,   100,   0,  0},    // Phaser Patrol (1982) (PAL).bin
    {"7e52a95074a66640fcfde124fffd491a",  "PHOENX", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Phoenix (1982).bin
    {"a00ec89d22fcc0c1a85bb542ddcb1178",  "PHOENX", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Phoenix (1982).bin
    {"00e55b27fe2e96354cd21b8b698d1e31",  "PHOENX", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Phoenix (1982).bin
    {"a8633050a686270fcf6c0cc4dcbad630",  "PHOENX", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Phoenix (1982).bin
    {"79fcdee6d71f23f6cf3d01258236c3b9",  "PHOENX", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,    91,   0,  1},    // Phoenix (1982) (PAL).bin
    {"51e390424f20e468d2b480030ce95d7b",  "PHOENX", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   61,    245,   100,   0,  1},    // Phoenix (1982) (PAL).bin
    {"1d4e0a034ad1275bc4d75165ae236105",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  36,    215,   100,   0,  5},    // Pick Up (1983).bin
    {"c4060a31d61ba857e756430a0a15ed2e",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  27,    210,    90,   0,  0},    // Pick'n Pile (NTSC by Thomas Jentzsch).bin
    {"da79aad11572c80a96e261e4ac6392d0",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   56,    245,    90,   0, 13},    // Pick'n Pile (PAL).bin
    {"17c0a63f9a680e7a61beba81692d9297",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  37,    210,   100,   0,  0},    // Picnic (1982).bin
    {"24544ee5d76f579992d9522e9b238955",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   62,    245,   100,   0,  0},    // Picnic (1982) (PAL).bin
    {"d3423d7600879174c038f53e5ebbf9d3",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  42,    210,   100,   0,  0},    // Piece o' Cake (1982).bin
    {"8e4fa8c6ad8d8dce0db8c991c166cdaa",  "PIGSIN", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Pigs in Space (1983).bin
    {"f04ee80011d95798006378643650aaa7",  "PIGSIN", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Pigs in Space (1983).bin
    {"95e1d834c57cdd525dd0bd6048a57f7b",  "PIGSIN", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   44,    245,    76,   0,  0},    // Pigs in Space (1983) (PAL).bin
    {"4bcfde5f9dbd07f8145a409d9fdd6f60",  "PINATA", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Pinata 2014
    {"56210a3b9ea6d5dd8f417a357ed8ca92",  "??????", BANK_WD,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  4},    // Pursuit of the Pink Panther Prototype (1983 - bad dump).bin
    {"62ee2b8f59e9cd6285bbdb674a952e8b",  "??????", BANK_WD,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  4},    // Pursuit of the Pink Panther Prototype (1983).bin
    {"aff90d7fb05e8f43937fc655bfffe2ea",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Pirate (2006) (David Weavil).bin
    {"9b54c77b530582d013f0fa4334d785b7",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Pirate Special Edition (2007) (David Weavil).bin
    {"3e90cf23106f2e08b2781e41299de556",  "PITFAL", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    197,   100,   4,  4},    // Pitfall! (1982).bin
    {"f73d2d0eff548e8fc66996f27acf2b4b",  "PITFAL", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    197,   100,   4,  4},    // Pitfall (1983) (CCE) (C-813).bin
    {"d20e61c86ed729780feca162166912ca",  "PITFAL", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    197,   100,   4,  4},    // Pitfall! (1982).bin
    {"2d405da70af82b20a6b3ecc3d1d2c4ec",  "PITFAL", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    197,   100,   4,  4},    // Pitfall! (1982).bin
    {"5a272012a62becabcd52920348c7c60b",  "PITFAL", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    197,   100,   4,  4},    // Pitfall! (1982).bin
    {"b86a12e53ab107b6caedd4e0272aa034",  "PITFAL", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    197,   100,   4,  4},    // Pitfall! (1982).bin
    {"791bc8aceb6b0f4d9990d6062b30adfa",  "PITFAL", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   4, 12},    // Pitfall! (1982) (PAL).bin
    {"de61a0b171e909a5a4cfcf81d146dbcb",  "PITFAL", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   4, 12},    // Pitfall! (1982) (PAL).bin
    {"55ef6ab2321ca0c3d369e63d59c059c8",  "PITFAL", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   34,    210,   100,   4, 30},    // Pitfall! (1982) (PAL).bin
    {"90369a64007a375bac633d85f1e02fcc",  "PITFAL", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  6},    // Pitfall 256 v1.20 (2017) (Thomas Jentzsch) (NTSC).bin
    {"6d842c96d5a01967be9680080dd5be54",  "PITFA2", BANK_DPC,  CTR_LJOY,      SPEC_WAVEDIRC,  MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    197,   100,   5,  2},    // Pitfall II - Lost Caverns (1983).bin
    {"490eed07d4691b27f473953fbea6541a",  "PITFA2", BANK_DPC,  CTR_LJOY,      SPEC_WAVEDIRC,  MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    197,   100,   5,  2},    // Pitfall II - Lost Caverns (1983).bin
    {"e34c236630c945089fcdef088c4b6e06",  "PITFA2", BANK_DPC,  CTR_LJOY,      SPEC_WAVEDIRC,  MODE_NO,   !VB,   HB,  ANA1_0,  PAL,   61,    240,   100,   5,  0},    // Pitfall II - Lost Caverns (1983) (PAL).bin
    {"17e5bddc16d520b62ea465b5c1057755",  "??????", BANK_E7,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,  !HB,  ANA1_0,  NTSC,  31,    202,   100,   0,  0},    // PITKATNTS.bin
    {"d9fbf1113114fb3a3c97550a0689f10f",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  39,    210,    97,   0,  3},    // Pizza Chef (1983).bin
    {"82efe7984783e23a7c55266a5125c68e",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Pizza Chef (1983).bin
    {"860ae9177b882f3324ed561f7b797940",  "??????", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // PlagueFinalNTSC 1.09.bas.bin
    {"9efb4e1a15a6cdd286e4bcd7cd94b7b8",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Planet of the Apes (1983).bin
    {"043f165f384fbea3ea89393597951512",  "PLAPAT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    97,   0, 10},    // Planet Patrol (1982).bin
    {"1c3f3133a3e5b023c77ecba94fd65995",  "PLAPAT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    97,   0, 10},    // Planet Patrol (1982).bin
    {"79004f84bdeee78d142e445057883169",  "PLAPAT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    97,   0, 10},    // Planet Patrol (1982).bin
    {"bb745c893999b0efc96ea9029e3c62ca",  "PLAPAT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   44,    245,    97,   0,  0},    // Planet Patrol (1982) (PAL).bin
    {"d6acff6aed0f04690fe4024d58ff4ce3",  "PLAPAT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   69,    245,    93,   0,  0},    // Planet Patrol (1982) (PAL).bin
    {"69fac82cd2312dd9ce5d90e22e2f070a",  "PLAPAT", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   69,    245,    93,   0,  0},    // Planet Patrol (1982) (PAL).bin
    {"da4e3396aa2db3bd667f83a1cb9e4a36",  "PLAQUE", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Plaque Attack (1983).bin
    {"b64426e787f04ff23ee629182c168603",  "PLAQUE", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Plaque Attack (1983).bin
    {"de24f700fd28d5b8381de13abd091db9",  "PLAQUE", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Plaque Attack (1983).bin
    {"8b8789c6669a4cee86c579a65332f852",  "PLAQUE", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Plaque Attack (1983).bin
    {"9afdfe1cff7f37f1c971fe3f0c900606",  "PLAQUE", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Plaque Attack (1983).bin
    {"e2904748da63dfefc8816652b924b642",  "PLAQUE", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Plaque Attack (1983).bin
    {"f7424985bac41067502b4a05b64cb75a",  "PLAQUE", BANK_4K,   CTR_GENESIS,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Plaque Attack (2 Button Hack).bin
    {"f6f1b27efc247a0e8d473ddb4269ff9e",  "PLAQUE", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   42,    245,   100,   0,  0},    // Plaque Attack (1983) (PAL).bin
    {"78821ef76ebc3934850d1bc1b9e4f4b0",  "PLAQUE", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   42,    245,   100,   0,  0},    // Plaque Attack (1983) (PAL).bin
    {"3eccf9f363f5c5de0c8b174a535dc83b",  "PLAQUE", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   65,    245,   100,   0,  0},    // Plaque Attack (1983) (PAL).bin
    {"7ced6709f091e79a2ab9575d3516a4ac",  "PLAQUE", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Plaque Attack (1983) (PAL).bin
    {"8bbfd951c89cc09c148bfabdefa08bec",  "??????", BANK_UA,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  28,    210,    87,   0,  4},    // Pleiades (1983).bin
    {"2b20a85ce8ea7dddd772c3c1cd3961bf",  "??????", BANK_UA,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  28,    210,    87,   0,  4},    // Pleiades (1983).bin
    {"8c136e97c0a4af66da4a249561ed17db",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Poker_squares.bin
    {"44f71e70b89dcc7cf39dfd622cfb9a27",  "POLARI", BANK_3F,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Polaris (1983).bin
    {"87bea777a34278d29b3b6029833c5422",  "POLARI", BANK_3F,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  38,    210,   100,   0,  0},    // Polaris (1983) (TJ).bin
    {"203049f4d8290bb4521cc4402415e737",  "POLARI", BANK_3F,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  5},    // Polaris (1983) (PAL).bin
    {"a4ff39d513b993159911efe01ac12eba",  "POLEPO", BANK_F8,   CTR_LJOY,      SPEC_POLEPOS,   MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Pole Position (1983).bin
    {"5f39353f7c6925779b0169a87ff86f1e",  "POLEPO", BANK_F8,   CTR_LJOY,      SPEC_POLEPOS,   MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  1},    // Pole Position (1983)[alt].bin
    {"650df778c6ce22d3fd1a7c33c565bcc3",  "POLEPO", BANK_F8,   CTR_GENESIS,   SPEC_POLEPOS,   MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  1},    // Pole Position (2 Button Hack).bin
    {"3225676f5c0c577aeccfaa7e6bedd765",  "POLEPO", BANK_F8,   CTR_LJOY,      SPEC_POLEPOS,   MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Pole Position (1983).bin
    {"5da8fd0b5ed33a360bff37f8b5d0cd58",  "POLEPO", BANK_F8,   CTR_LJOY,      SPEC_POLEPOS,   MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Pole Position (1983).bin
    {"b56264f738b2eb2c8f7cf5a2a75e5fdc",  "POLEPO", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   3,  0},    // Pole Position (1983) (PAL).bin
    {"ee28424af389a7f3672182009472500c",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Polo (1978).bin
    {"a83b070b485cf1fb4d5a48da153fdf1a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  42,    210,   100,   0,  2},    // Pompeii (1983).bin
    {"4799a40b6e889370b7ee55c17ba65141",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  27,    210,    89,   0,  0},    // Pooyan (1983).bin
    {"668dc528b7ea9345140f4fcfbecf7066",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   44,    245,    77,   0,  0},    // Pooyan (1983) (PAL).bin
    {"89afff4a10807093c105740c73e9b544",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   44,    245,    77,   0,  0},    // Pooyan (1983) (PAL).bin
    {"72876fd7c7435f41d571f1101fc456ea",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   26,    245,    77,   0,  0},    // Pooyan (1983) (PAL).bin
    {"c7f13ef38f61ee2367ada94fdcc6d206",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    96,   0,  8},    // Popeye (1983).bin
    {"e9cb18770a41a16de63b124c1e8bd493",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   51,    245,    96,   0,  0},    // Popeye (1983) (PAL).bin
    {"f93d7fee92717e161e6763a88a293ffa",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  39,    210,   100,   0,  4},    // Porky's (1983).bin
    {"3ad3dc799211ccd424d7c6d454401436",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Power Lords (1983) (Probe 2000 - NAP).bin
    {"97d079315c09796ff6d95a06e4b70171",  "PRCOOK", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   3,  5},    // Pressure Cooker (1983).bin
    {"525ea747d746f3e80e3027720e1fa7ac",  "PRCOOK", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   61,    230,   100,   3,  5},    // Pressure Cooker (1983) (PAL).bin
    {"027a59a575b78860aed780b2ae7d001d",  "PRCOOK", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   3,  0},    // Pressure Cooker (1983) (PAL).bin
    {"de1a636d098349be11bbc2d090f4e9cf",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Pressure gauge.bin
    {"8e9544bbcf9b874f7c8b0c3f09925952",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Primate14.txt.bin
    {"104468e44898b8e9fa4a1500fde8d4cb",  "PRIRES", BANK_F4,   CTR_GENESIS,   SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  30,    210,   100,   0,  0},    // PrincessRescue_Final_NTSC.bin
    {"ef3a4f64b6494ba770862768caf04b86",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Private Eye (1983).bin
    {"f9cef637ea8e905a10e324e582dd39c2",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Private Eye (1983).bin
    {"1266b3fd632c981f3ef9bdbf9f86ce9a",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   65,    245,   100,   3,  0},    // Private Eye (1983) (PAL).bin
    {"9c0554b6d355be247883255e0a17a4b4",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  6},    // Pro Tennis 2.0
    {"dbda26ffc809e072cc734afe8dd89fe2",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  30,    210,    80,   0,  0},    // Pumuckl I (Zoo Fun) (1983) (NTSC).bin
    {"484b0076816a104875e00467d431c2d2",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    96,   0, 10},    // Q-bert (1983).bin
    {"8b40a9ca1cfcd14822e2547eaa9df5c1",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   48,    245,    96,   0,  3},    // Q-bert (1983) (PAL).bin
    {"eb6d6e22a16f30687ade526d7a6f05c5",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL ,  48,    245,    96,   0,  0},    // Q-bert (1983) (PAL).bin
    {"dcb406b54f1b69017227cfbe4052005e",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  10},   // Q-bert Arcade Hack.bin
    {"517592e6e0c71731019c0cebc2ce044f",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    93,  -1,  7},    // Q-bert's Qubes (1984).bin
    {"ac53b83e1b57a601eeae9d3ce1b4a458",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // qb (2.15) (ntsc).bin
    {"34e37eaffc0d34e05e40ed883f848b40",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // qb (2.15) (stella).bin
    {"024365007a87f213cbe8ef5f2e8e1333",  "QUARUN", BANK_F8,   CTR_LJOY,      SPEC_WAVEDIRC,  MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Quadrun (1983).bin
    {"a0675883f9b09a3595ddd66a6f5d3498",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  36,    210,   100,   0,  1},    // Quest for Quintana Roo (1984).bin
    {"eaacfcdc1d4ee1258429b7ae7f084125",  "??????", BANK_F8,   CTR_GENESIS,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  36,    210,   100,   0,  1},    // Quest for Quintana Roo (2 Button Hack).bin
    {"f736864442164b29235e8872013180cd",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   60,    245,   100,   0,  0},    // Quest for Quintana Roo (1984) (PAL).bin
    {"7eba20c2291a982214cc7cbe8d0b47cd",  "QUICKS", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Quick Step! (1983).bin
    {"e72ee2d6e501f07ec5e8a0efbe520bee",  "QUICKS", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   61,    245,   100,   0,  0},    // Quick Step! (1983) (PAL).bin
    {"c7490a9f6c3ddfbf7cbb020aa24f1366",  "??????", BANK_UA,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    98,   0,  4},    // R2D-Tank (UA).bin
    {"fb4ca865abc02d66e39651bd9ade140a",  "RABTRA", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  1},    // Rabbit Transit (1983).bin
    {"7481f0771bff13885b2ff2570cf90d7b",  "RABTRA", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_FF,    VB,   HB,  ANA1_0,  PAL,   57,    230,   100,   4,  0},    // Rabbit Transit (1983) (PAL).bin
    {"664d9bfda6f32511f6b4aa0159fd87f5",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Racer Prototype (1982).bin
    {"3c7a7b3a0a7e6319b2fa0f923ef6c9af",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Racer Prototype (1982).bin
    {"4df9d7352a56a458abb7961bf10aba4e",  "??????", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  35,    210,   100,   0,  1},    // Racing Car (Unknown).bin
    {"aab840db22075aa0f6a6b83a597f8890",  "??????", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Racing Car (Home Vision) (PAL).bin
    {"a20d931a8fddcd6f6116ed21ff5c4832",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  1},    // Racquetball (1981).bin
    {"cbced209dd0575a27212d3eee6aee3bc",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  1},    // Racquetball (1981).bin
    {"f0d393dbf4164a688b2346770c9bbd12",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  1},    // Racquetball (1981).bin
    {"4f7b07ec2bef5ccffe06403a142f80db",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   3,  5},    // Racquetball (1981) (PAL).bin
    {"97933c9f20873446e4c1f8a4da21575f",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   3,  5},    // Racquetball (1981) (PAL).bin
    {"8fe00172e7fff4c1878dabcf11bb8dce",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   3,  5},    // Racquetball (1981) (PAL).bin
    {"247fa1a29ad90e64069ee13d96fea6d6",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    98,   0,  2},    // Radar (1983).bin
    {"baf4ce885aa281fd31711da9b9795485",  "RADARL", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   2,  1},    // Radar Lock (1989).bin
    {"a6c521d3645c874f2580891a99a1e3bc",  "RADARL", BANK_F6SC, CTR_GENESIS,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   2,  1},    // Radar Lock (2 Button Hack).bin
    {"04856e3006a4f5f7b4638da71dad3d88",  "RADARL", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   75,    245,   100,   3,  0},    // Radar Lock (1989) (PAL).bin
    {"92a1a605b7ad56d863a56373a866761b",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Raft Rider (1982).bin
    {"438968a26b7cfe14a499f5bbbbf844db",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   67,    245,   100,   0,  0},    // Raft Rider (1982) (PAL).bin
    {"1e750000af77cc76232f4d040f4ab060",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Raft Rider (1982).bin
    {"f724d3dd2471ed4cf5f191dbb724b69f",  "RAIDER", BANK_F8,   CTR_RAIDERS,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Raiders of the Lost Ark (1982).bin
    {"1cafa9f3f9a2fce4af6e4b85a2bbd254",  "RAIDER", BANK_F8,   CTR_RAIDERS,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   62,    245,   100,   0,  0},    // Raiders of the Lost Ark (PAL).bin
    {"8cc06f4ba468f764f54feb1773a295f0",  "??????", BANK_DPCP, CTR_TWINSTICK, SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Rail Slider (Twin Stick shooter)
    {"cb96b0cf90ab7777a2f6f05e8ad3f694",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Rainbow Invaders NTSC.bin
    {"7096a198531d3f16a99d518ac0d7519a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    93,   0,  1},    // Ram It (1982).bin
    {"63e42d576800086488679490a833e097",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   47,    245,    93,   0,  8},    // Ram It (1982) (PAL).bin
    {"f2f2cb35fdef063c966c1f5481050ea2",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   32,    245,    97,   0,  8},    // Ram It (1982) (PAL).bin
    {"ccd6ce508eee4b3fca67212833edcd85",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,    93,   0,  8},    // Ram It (1982) (PAL).bin
    {"5e1b4629426f4992cf3b2905a696e1a7",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Rampage! (1989).bin
    {"a11099b6ec24e4b00b8795744fb12005",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,    92,   0,  1},    // Rampage! (1989) (PAL).bin
    {"9f8fad4badcd7be61bbd2bcaeef3c58f",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Reactor (1982).bin
    {"4904a2550759b9b4570e886374f9d092",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   66,    245,   100,   0,  2},    // Reactor (1982) (PAL).bin
    {"eb634650c3912132092b7aee540bbce3",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    96,   0,  0},    // RealSports Baseball (1982).bin
    {"1d6ed6fe9dfbde32708e8353548cbb80",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    96,   0,  0},    // RealSports Baseball (1982).bin
    {"5524718a19107a04ec3265c93136a7b5",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  38,    210,   100,   0,  0},    // RealSports Basketball (1983) (NTSC by TJ).bin
    {"8a183b6357987db5170c5cf9f4a113e5",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   48,    245,   100,   0,  0},    // RealSports Basketball (1983) (PAL).bin
    {"7e9f088e15b2af9ff3411991393e6b1f",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   48,    245,   100,   0,  0},    // RealSports Basketball (1983) (PAL).bin
    {"3177cc5c04c1a4080a927dfa4099482b",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // RealSports Boxing (1987).bin
    {"1c85c0fc480bbd69dc301591b6ecb422",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // RealSports Boxing (1987).bin
    {"4abb4c87a4c5f5d0c14ead2bb36251be",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   55,    245,   100,   0,  0},    // RealSports Boxing (1987) (PAL).bin
    {"7ad257833190bc60277c1ca475057051",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // RealSports Football (1982).bin
    {"08f853e8e01e711919e734d85349220d",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // RealSports Soccer (1983).bin
    {"f7856e324bc56f45b9c8e6ff062ec033",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // RealSports Soccer (1983).bin
    {"2447e17a4e18e6b609de498fe4ab52ba",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // RealSports Soccer (1983).bin
    {"0e7e73421606873b544e858c59dc283e",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // RealSports Soccer (1983).bin
    {"b9336ed6d94a5cc81a16483b0a946a73",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   54,    245,    72,   0, 10},    // RealSports Soccer (1983) (PAL).bin
    {"dac5c0fe74531f077c105b396874a9f1",  "REALTE", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // RealSports Tennis (1983).bin
    {"3e7d10d0a911afc4b492d06c99863e65",  "REALTE", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // RealSports Tennis (1983).bin
    {"53b66f11f67c3b53b2995e0e02017bd7",  "REALTE", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // RealSports Tennis (1983).bin
    {"517923e655755086a3b72c0b17b430e6",  "REALTE", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // RealSports Tennis (1983).bin
    {"dac5c0fe74531f077c105b396874a9f1",  "REALTE", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // RealSports Tennis (1983).bin
    {"4e66c8e7c670532569c70d205f615dad",  "REALTE", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   62,    245,    94,   0,  0},    // RealSports Tennis (1983) (PAL).bin
    {"aed0b7bd64cc384f85fdea33e28daf3b",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // RealSports Volleyball (1982).bin
    {"cbc373fbcb1653b4c56bfabba33ea50d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // RealSports Volleyball (1982).bin
    {"6c128bc950fcbdbcaf0d99935da70156",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // RealSports Volleyball (1982).bin
    {"5faffe1c4c57430978dec5ced32b9f4a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // RealSports Volleyball (1982).bin
    {"42b3ab3cf661929bdc77b621a8c37574",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // RealSports Volleyball (1982).bin
    {"4ca0959f846d2beada18ecf29efe137e",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   48,    245,    72,   3,  0},    // RealSports Volleyball (1982) (PAL).bin
    {"4d8396deeabb40b5e8578276eb5a8b6d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   65,    245,   100,   0,  0},    // RealSports Volleyball (1982) (PAL).bin
    {"4eb4fd544805babafc375dcdb8c2a597",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  6},    // Red Sea Crossing.bin
    {"2395021c633bb1890b546c6e23b6c3df",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Refraction_Beta_1.bin
    {"8e512ad4506800458f99dec084fc2c64",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  6},    // ReinderRescue.bin
    {"60a61da9b2f43dd7e13a5093ec41a53d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Rescue Terra I (1982).bin
    {"42249ec8043a9a0203dde0b5bb46d8c4",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Resgate Espacial.bin
    {"4f64d6d0694d9b7a1ed7b0cb0b83e759",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Revenge of the Beefsteak Tomatoes (1982).bin
    {"0b01909ba84512fdaf224d3c3fd0cf8d",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // RevengeOfTheApes.bin
    {"a995b6cbdb1f0433abc74050808590e6",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Riddle of the Sphinx (1982).bin
    {"083e7cae41a874b2f9b61736c37d2ffe",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Riddle of the Sphinx (1982) (PAL).bin
    {"31512cdfadfd82bfb6f196e3b0fd83cd",  "??????", BANK_3F,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // River Patrol (1984).bin
    {"393948436d1f4cc3192410bb918f9724",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    195,   100,   0,  0},    // River Raid (1982).bin
    {"b9232c1de494875efe1858fc8390616d",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    195,   100,   0,  0},    // River Raid (1982).bin
    {"d5e17022d1ecc20fd9b53dc464c302f1",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    195,   100,   0,  0},    // River Raid (1982).bin
    {"39d36366ae7e6dfd53393fb9ebab02a0",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    195,   100,   0,  0},    // River Raid (1982).bin
    {"59f596285d174233c84597dee6f34f1f",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    195,   100,   0,  0},    // River Raid (1982).bin
    {"da5096000db5fdaa8d02db57d9367998",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    195,   100,   0,  0},    // River Raid (1982).bin
    {"01b09872dcd9556427761f0ed64aa42a",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    195,   100,   0,  0},    // River Raid (1982).bin
    {"8c8b15b3259e60757987ed13cdd74d41",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    195,   100,   0,  0},    // River Raid (1982).bin
    {"cd4423bd9f0763409bae9111f888f7c2",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    195,   100,   0,  0},    // River Raid (1982).bin
    {"f11cfab087fcbd930ab8b0becc5b2e5a",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    195,   100,   0,  0},    // River Raid (1982).bin
    {"8c941fa32c7718a10061d8c328909577",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    195,   100,   0,  0},    // River Raid (1982).bin
    {"a94528ae05dd051894e945d4d2349b3b",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    195,   100,   0,  0},    // River Raid (1982).bin
    {"c29d17eef6b0784db4586c12cb5fd454",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    195,   100,   0,  0},    // River Raid (1982).bin
    {"6ce2110ac5dd89ab398d9452891752ab",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    195,   100,   0,  0},    // River Raid (1982).bin
    {"39fe316952134b1277b6a81af8e05776",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    195,   100,   0,  0},    // River Raid (1982).bin
    {"d5e5b3ec074fff8976017ef121d26129",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    195,   100,   0,  0},    // River Raid (1982).bin
    {"90f502cbf4438a95f69f848cef36eb64",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    195,   100,   0,  6},    // River Raid (1982) II (Digitel).bin
    {"f2d4d6187903cac2d5ea8ed90dad120d",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    195,   100,   0,  6},    // River Raid (1982) II (Digimax).bin
    {"cdeb99d17b82373f2fe17606e4eed769",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4, -2},    // River Raid Night Stealth (Hack).bin
    {"927d422d6335018da469a9a07cd80390",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   40,    245,   100,   3, 17},    // River Raid (1982) (PAL).bin
    {"fadb89f9b23beb4d43a7895c532757e2",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   40,    245,   100,   3, 17},    // River Raid (1982) (PAL).bin
    {"eb46e99ec15858f8cd8c91cef384ce09",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   41,    245,   100,   3, 17},    // River Raid (1982) (PAL).bin
    {"073d7aff37b7601431e4f742c36c0dc1",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   3, -8},    // River Raid (1982) (PAL).bin
    {"cf3c2725f736d4bcb84ad6f42de62a41",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   3,  6},    // River Raid (1982) (PAL).bin
    {"bcef7880828a391cf6b50d5a6dcef719",  "RVRAID", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   3,  6},    // River Raid (1982) (PAL).bin
    {"ab56f1b2542a05bebc4fbccfc4803a38",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // River Raid II (1988).bin
    {"b049fc8ac50be7c2f28418817979c637",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   1,  5},    // River Raid II (1988) (PAL).bin
    {"a2fbb071591073a19c7f901df74f1df7",  "RVRAID", BANK_F8SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   2,  5},    // River Raid 3 (v4).bin
    {"4e86866d9cde738d1630e2e35d7288ce",  "RVRAID", BANK_F8SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   2,  7},    // River Raid III (AKA River Raid) (Supergame).bin
    {"2bd00beefdb424fa39931a75e890695d",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  27,    210,   100,   0,  0},    // Road Runner (1989) [alt].bin
    {"ce5cc62608be2cd3ed8abd844efb8919",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  27,    210,   100,   0,  0},    // Road Runner (1989).bin
    {"7d3cdde63b16fa637c4484e716839c94",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  27,    210,   100,   0,  0},    // Road Runner (1989).bin
    {"c3a9550f6345f4c25b372c42dc865703",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   53,    245,    94,   0,  0},    // Road Runner (1989) (PAL).bin
    {"72a46e0c21f825518b7261c267ab886e",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    210,    90,   0,  0},    // Robin Hood (1983).bin
    {"dd7598b8bcb81590428900f71b720efb",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   31,    245,    90,   0,  0},    // Robin Hood (1983) (PAL).bin
    {"0f8043715d66a4bbed394ef801d99862",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   31,    245,    90,   0,  0},    // Robin Hood (1983) (PAL).bin
    {"fef7a035eb3a6ad8f658a339d5f1df0e",  "??????", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // RoboMechanik_WIP4__NTSC.bin
    {"ec44dcf2ddb4319962fc43b725a902e8",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  31,    210,    96,   0,  0},    // Robot City (RC8).bin
    {"4f618c2429138e0280969193ed6c107e",  "ROTANK", BANK_FE,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  7},    // Robot Tank (1983).bin
    {"f2edec8d3dbfb19261caf85ff4d975d6",  "ROTAN8", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  7},    // Robot Tank (1983).bin [F8]
    {"594437a35603c3e857b5af75b9718b61",  "ROTANK", BANK_FE,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Robot Tank (1983) (PAL).bin
    {"f687ec4b69611a7f78bd69b8a567937a",  "ROTANK", BANK_FE,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   3, 13},    // Robot Tank (1983) (PAL).bin
    {"65bd29e8ab1b847309775b0de6b2e4fe",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,    91,   0,  0},    // Roc 'n Rope (1984).bin
    {"0173675d40a8d975763ee493377ca87d",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   53,    245,    53,   0,  1},    // Roc 'n Rope (1984) (PAL).bin
    {"cf92ce2324c57a6a5738333a5b0a26f1",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Rocket_Command.bin
    {"d97fd5e6e1daacd909559a71f189f14b",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Rocky & Bullwinkle (1983).bin
    {"67931b0d37dc99af250dd06f1c095e8d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  29,    210,   100,   0,  0},    // Room of Doom (1982).bin
    {"a936d80083e99d48752ad15c2b5f7c96",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   37,    245,    87,   0,  5},    // Room of Doom (1982) (PAL).bin
    {"685e9668dc270b6deeb9cfbfd4d633c3",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   37,    245,    87,   0,  5},    // Room of Doom (1982) (PAL).bin
    {"1f2ae0c70a04c980c838c2cdc412cf45",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    94,   0,  1},    // Rubik's Cube (1984).bin
    {"40b1832177c63ebf81e6c5b61aaffd3a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // RubiksCube3D.bin
    {"f3cd0f886201d1376f3abab2df53b1b9",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    78,   0,  0},    // Rush Hour (1983).bin
    {"aad61898633f470ce528e3d7ef3d0adb",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    78,   0,  0},    // Rush Hour (1983).bin
    {"ebf2dff78a08733251bf3838f02f7938",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  28,    210,    78,   0, -1},    // Rush Hour (1983).bin
    {"b9b4612358a0b2c1b4d66bb146767306",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    78,   0,  0},    // Rush Hour (1983).bin
    {"715dbf2e39ba8a52c5fe5cdd927b37e0",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // S.A.C. Alert (Joyboard) (1983) (Amiga) (Prototype) ~.bin
    {"a4ecb54f877cd94515527b11e698608c",  "SABOTR", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Saboteur (1983).bin
    {"64fab9d15df937915b1c392fc119b83b",  "SABOTR", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Saboteur (1983).bin
    {"350e0f7b562ec5e457b3f5af013648db",  "SABOTR", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Saboteur (1983).bin
    {"4e01d9072c500331e65bb87c24020d3f",  "SABOTR", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Saboteur (1983).bin
    {"1ec57bbd27bdbd08b60c391c4895c1cf",  "SABOTR", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Saboteur (1983).bin
    {"4d502d6fb5b992ee0591569144128f99",  "SAVMAR", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Save Mary! (1989).bin
    {"7bb286cb659d146af3966d699b51f509",  "SAVMAR", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Save Mary! (1989).bin
    {"4884b1297500bd1243659e43c7e7579e",  "SAVMAR", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0, 12},    // Save Mary! (1989) (PAL).bin
    {"49571b26f46620a85f93448359324c28",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    88,   0,  6},    // Save Our Ship (Unknown).bin
    {"ed1a784875538c7871d035b7a98c2433",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Save Our Ship.bin
    {"01297d9b450455dd716db9658efb2fae",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,    89,   0,  0},    // Save Our Ship (Technovision) (PAL).bin
    {"e377c3af4f54a51b85efe37d4b7029e6",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  23,    210,    82,   0,  3},    // Save the Whales (1983).bin
    {"fe641247a4ab9bee970e19ab55f23b25",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    90,   0,  2},    // Save the Whales (1983).bin
    {"e9f25c7af4f27c9e1b5b8f6fe6141e8c",  "SCRAMB", BANK_DPCP, CTR_LJOY,      SPEC_DPCPNOC,   MODE_NO,   !VB,  !HB,  ANA1_0,  NTSC,  32,    205,   100,   0,  0},    // Scramble (final).bin
    {"4f556cd013528aed2aebc715b4e49ac8",  "SCRAMB", BANK_DPCP, CTR_LJOY,      SPEC_DPCPNOC,   MODE_NO,   !VB,  !HB,  ANA1_0,  NTSC,  32,    205,   100,   0,  0},    // Scramble_demo_final_v2_NTSC.bin
    {"bee094c9d3d1feac3c63ed81b4881617",  "SCRAMB", BANK_DPCP, CTR_LJOY,      SPEC_DPCPNOC,   MODE_NO,   !VB,  !HB,  ANA1_0,  NTSC,  32,    205,   100,   0,  0},    // Scramble_demo_final_CG_NTSC.bin
    {"19e761e53e5ec8e9f2fceea62715ca06",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  41,    210,    97,   0,  2},    // Scuba Diver (1983).bin
    {"3fe43915e5655cf69485364e9f464097",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  41,    210,    97,   0,  2},    // Scuba Diver (1983).bin
    {"5dccf215fdb9bbf5d4a6d0139e5e8bcb",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  41,    210,    97,   0,  2},    // Scuba Diver (1983).bin
    {"5dccf215fdb9bbf5d4a6d0139e5e8bcb",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  41,    210,    97,   0,  2},    // Scuba Diver (1983).bin
    {"1bc2427ac9b032a52fe527c7b26ce22c",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Sea Battle (1983).bin
    {"624e0a77f9ec67d628211aaf24d8aea6",  "SEAHWK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   2,  2},    // Sea Hawk (1983).bin
    {"07f42847a79e4f5ae55cc03304b18c25",  "SEAHWK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   2,  2},    // Sea Hawk (1983).bin
    {"3fd53bfeee39064c945a769f17815a7f",  "SEAHWK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   2,  2},    // Sea Hawk (1983).bin
    {"07f42847a79e4f5ae55cc03304b18c25",  "SEAHWK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   2,  2},    // Sea Hawk (1983).bin
    {"8fa47e5242776e841df7e708b12eb998",  "SEAHWK", BANK_4K,   CTR_GENESIS,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   2,  2},    // Sea Hawk (2 Button Hack).bin
    {"74d072e8a34560c36cacbc57b2462360",  "SEAHWK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   38,    245,    85,   2,  0},    // Sea Hawk (1983) (PAL).bin
    {"1b8d35d93697450ea26ebf7ff17bd4d1",  "SEAHWK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   38,    245,    85,   2,  0},    // Sea Hawk (1983) (PAL).bin
    {"5dccf215fdb9bbf5d4a6d0139e5e8bcb",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  9},    // Sea Hunt (1987).bin
    {"d8acaa980cda94b65066568dd04d9eb0",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  9},    // Sea Hunt (1987).bin
    {"340f546d59e72fb358c49ac2ca8482bb",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,    64,   0,  5},    // Sea Hunt (1987) (PAL).bin
    {"1278f74ca1dfaa9122df3eca3c5bcaad",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,    64,   0,  5},    // Sea Hunt (1987) (PAL).bin
    {"7cd900e9eccbb240fe9c37fa28f917b5",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,    64,   0,  5},    // Sea Hunt (1987) (PAL).bin
    {"dd45e370aceff765f1e72c619efd4399",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  26,    210,    76,   0,  1},    // Sea Monster (1982) (Bit Corporation) (PG201).bin
    {"a4b9423877a0b86ca35b52ca3c994ac5",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  26,    210,    76,   0,  1},    // Sea Monster (1982) (Bit Corporation) (PG201).bin
    {"df6a46714960a3e39b57b3c3983801b5",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   53,    245,    76,   0,  0},    // Sea Monster (1982) (Bit Corporation) (PAL).bin
    {"2124cf92978c46684b6c39ccc2e33713",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,    76,   0,  0},    // Sea Monster (1982) (Bit Corporation) (PAL).bin
    {"68489e60268a5e6e052bad9c62681635",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,    76,   0,  0},    // Sea Monster (1982) (Bit Corporation) (PAL).bin
    {"dde55d9868911407fe8b3fefef396f00",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Sea Wolf.bin
    {"a8c48b4e0bf35fe97cc84fdd2c507f78",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  30,    210,    84,   0,  0},    // Seamonster (1982).bin
    {"240bfbac5163af4df5ae713985386f92",  "SEAQST", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Seaquest (1983).bin
    {"79c27f90591e3fdc7d2ed020ecbedeb3",  "SEAQST", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Seaquest (1983).bin
    {"0b24658714f8dff110a693a2052cc207",  "SEAQST", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Seaquest (1983).bin
    {"bc33c685e6ffced83abe7a43f30df7f9",  "SEAQST", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Seaquest (1983).bin
    {"94d90f63678e086f6b6d5e1bc6c4c8c2",  "SEAQST", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Seaquest (1983).bin
    {"10af8728f975aa35a99d0965de8f714c",  "SEAQST", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Seaquest (1983).bin
    {"6141c095d0aee4e734bebfaac939030a",  "SEAQST", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   65,    245,   100,   0,  0},    // Seaquest (1983) (PAL).bin
    {"5b6f5bcbbde42fc77d0bdb3146693565",  "SEAQST", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   65,    245,   100,   0,  0},    // Seaquest (1983) (PAL).bin
    {"fd0e5148162e8ec6719445d559f018a9",  "SEAQST", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   3, 19},    // Seaquest (1983) (PAL).bin
    {"3034532daf80997f752aee680d2e7fc3",  "??????", BANK_F4SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Seaweed Assault.bin
    {"605fd59bfef88901c8c4794193a4cbad",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    79,   0,  5},    // Secret Agent (1983).bin
    {"fc24a94d4371c69bc58f5245ada43c44",  "SQUEST", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Secret Quest (1989).bin
    {"2d2c5f0761e609e3c5228766f446f7f8",  "SQUEST", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   55,    245,   100,   0, -2},    // Secret Quest (1989) (PAL).bin
    {"8da51e0c4b6b46f7619425119c7d018e",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   3,  8},    // Sentinel (1990).bin
    {"c880c659cdc0f84c4a66bc818f89618e",  "OPENSE", BANK_4K,   CTR_LJOY,      SPEC_WAVEDIRC,  MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  30,    210,    82,   0,  0},    // Sesam, Oeffne Dich (AKA Open Sesame) (Bitcorp, TJ).bin
    {"258f8f1a6d9af8fc1980b22361738678",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  32,    210,   100,   0,  0},    // Shadow Reflex (Beta 10-26-2020).bin
    {"54f7efa6428f14b9f610ad0ca757e26c",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   1,  6},    // Shark Attack (1982).bin
    {"5069fecbe4706371f17737b0357cfa68",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   1,  0},    // Shark Attack (1982) (PAL).bin
    {"97734e9e037574674749b9f8cadc392a",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // ShatteredEarth20210606.bin
    {"8debebc61916692e2d66f2edf4bed29c",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Sheep It Up_ntsc.bin
    {"0248a33b414282ba2addf7dcb2a2e696",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    98,   0,  3},    // Shield Shifter (2009) (John A. Reder).bin
    {"b5a1a189601a785bdb2f02a424080412",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  31,    210,    93,   0,  0},    // Shootin' Gallery (1982).bin
    {"15c11ab6e4502b2010b18366133fc322",  "??????", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Shooting Arcade (1989).bin
    {"557e893616648c37a27aab5a47acbf10",  "??????", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,   100,   0,  0},    // Shooting Arcade (1990) (PAL).bin
    {"25b6dc012cdba63704ea9535c6987beb",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Shuttle Orbiter (1983).bin
    {"f332b43a5c9ae93aef60cf76e8f71a67",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Silhouette 2007-04-10.bin
    {"1e85f8bccb4b866d4daa9fcf89306474",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Sinistar (1984).bin
    {"4c8970f6c294a0a54c9c45e5e8445f93",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Sir Lancelot (1983).bin
    {"dd0cbe5351551a538414fb9e37fc56e8",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Sir Lancelot (1983) (PAL).bin
    {"f847fb8dba6c6d66d13724dbe5d95c4d",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    94,   0,  0},    // Skate Boardin' (1987).bin
    {"abe40542e4ff2d1c51aa2bb033f09984",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,    79,   2,  0},    // Skate Boardin' (1987) (PAL).bin
    {"39c78d682516d79130b379fa9deb8d1c",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Skeet Shoot (1981).bin
    {"f1d3af1489002d811440517e399753ec",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Skeet Shoot (1981).bin
    {"0832fb2ee654bf9382bc57d2b16d2ffc",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Skeet Shoot (1981) (PAL).bin
    {"eafe8b40313a65792e88ff9f2fe2655c",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    78,   0,  0},    // SkelPlus.bin
    {"75723c16a975afb9da92022583fa802d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  51,    232,    76,   4,  0},    // Ski Hunt (aka Snow Hunter).bin
    {"a23fb1710f3115d7b15e9c656f9a00bc",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  29,    220,    80,   0,  0},    // Ski Hunt (alt).bin
    {"8654d7f0fb351960016e06646f639b02",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   36,    245,   100,   3,  1},    // Ski Hunt (PAL).bin
    {"6c1553ca90b413bf762dfc65f2b881c7",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,   100,   3,  1},    // Ski Hunt (PAL).bin
    {"5305f69fbf772fac4760cdcf87f1ab1f",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  36,    210,   100,   0,  4},    // Ski Run (2600 Screen Search Console).bin
    {"268067a42751aa4695e01d0160273270",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  35,    210,   100,   0,  0},    // Ski Run (NTSC by TJ).bin
    {"f10e3f45fb01416c87e5835ab270b53a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  2},    // Ski Run (PAL).bin
    {"b76fbadc8ffb1f83e2ca08b6fb4d6c9f",  "SKIING", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   1,  1},    // Skiing (1980).bin
    {"c118854d670289a8b5d5156aa74b0c49",  "SKIING", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   1,  1},    // Skiing (1980).bin
    {"40d9f5709877ecf3dd1184f9791dd35e",  "SKIING", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   1,  1},    // Skiing (1980).bin
    {"0e4b2b6e014a93ef8be896823da0d4ec",  "SKIING", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   58,    245,   100,   1,  0},    // Skiing (1980) (PAL).bin
    {"367411b78119299234772c08df10e134",  "SKIING", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   58,    245,   100,   1,  0},    // Skiing (1980) (PAL).bin
    {"eec61cc4250df70939d48fe02d7122ac",  "SKIING", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   1, 17},    // Skiing (1980) (PAL).bin
    {"fa2bf0c76104050013086e085b2dbbad",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0, -3},    // Sky Destroyer.bin
    {"46c021a3e9e2fd00919ca3dd1a6b76d8",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Sky Diver (1979).bin
    {"5a81ad4e184050851e63c8e16e3dac77",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Sky Diver (1979).bin
    {"756ca07a65a4fbbedeb5f0ddfc04d0be",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   43,    245,    60,   0,  0},    // Sky Diver (1979) (PAL).bin
    {"3f75a5da3e40d486b21dfc1c8517adc0",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   44,    245,    59,   0,  0},    // Sky Diver (1979) (PAL).bin
    {"8190b403d67bf9792fe22fa5d22f3556",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   59,    245,   100,   0,  0},    // Sky Diver (1979) (PAL).bin
    {"2a0ba55e56e7a596146fa729acf0e109",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  3},    // Sky Jinks (1982).bin
    {"3750f2375252b6a20e4628692e94e8b1",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  3},    // Sky Jinks (1982).bin
    {"93dc15d15e77a7b23162467f95a5f22d",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  3},    // Sky Jinks (1982).bin
    {"f992a39b46aa48188fab12ad3809ae4a",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   51,    245,   100,   4, 11},    // Sky Jinks (1982) (PAL).bin
    {"50a410a5ded0fc9aa6576be45a04f215",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   51,    245,   100,   4, 11},    // Sky Jinks (1982) (PAL).bin
    {"4c9307de724c36fd487af6c99ca078f2",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Sky Patrol (1982).bin
    {"3b91c347d8e6427edbe942a7a405290d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  6},    // Sky Skipper (1983).bin
    {"514f911ecff2be5eeff2f39c49a9725c",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   39,    245,    88,   0,  0},    // Sky Skipper (1983) (PAL).bin
    {"024e97dabc0b083c31ea52a83cca4e01",  "SLIDEB", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Slideboy (final).bin
    {"6151575ffb5ceddd26173f709336776b",  "??????", BANK_CDFJ, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Slime.bin
    {"f90b5da189f24d7e1a2117d8c8abc952",  "??????", BANK_2K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Slot Machine (1979).bin
    {"705fe719179e65b0af328644f3a04900",  "??????", BANK_2K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Slot Machine (1979).bin
    {"81254ebce88fa46c4ff5a2f4d2bad538",  "??????", BANK_2K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Slot Machine (1979).bin
    {"fc6052438f339aea373bbc999433388a",  "??????", BANK_2K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   56,    245,    89,   0,  0},    // Slot Machine (1979) (PAL).bin
    {"75ea128ba96ac6db8edf54b071027c4e",  "??????", BANK_2K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Slot Machine (1979) (PAL).bin
    {"aed82052f7589df05a3f417bb4e45f0c",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    92,   0,  0},    // Slot Racers (1978).bin
    {"a7ed7dc5cbc901388afa59030fb11d26",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   62,    210,    92,   0,  0},    // Slot Racers (1978) (PAL).bin
    {"d1d704a7146e95709b57b6d4cac3f788",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   60,    210,    92,   0,  0},    // Slot Racers (1978) (PAL).bin
    {"08916d8d9bd249270bc0424251fb5bcc",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Smiley022707.bas.bin
    {"3d1e83afdb4265fa2fb84819c9cfd39c",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0, 10},    // Smurf - Rescue in Gargamel's Castle (1982).bin
    {"6eb10fd23c7161751d18b9e8484c0004",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0, 10},    // Smurf - Rescue in Gargamel's Castle (1982).bin
    {"9f96c10c3272c12ff60d65421fae2271",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0, 10},    // Smurf - Rescue in Gargamel's Castle (button hack).bin
    {"73c545db2afd5783d37c46004e4024c2",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   56,    245,   100,   0,  0},    // Smurf - Rescue in Gargamel's Castle (1982) (PAL).bin
    {"a204cd4fb1944c86e800120706512a64",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Smurfs Save the Day (1983).bin
    {"4d1b86afde65f0d124977cb80018ea6a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Snappy.bin
    {"9c6faa4ff7f2ae549bbcb14f582b70e4",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  30,    210,   100,   1,  0},    // Sneak 'n Peek (1982).bin
    {"f21813aa050437f0dbc8479864acec6d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   81,    245,   100,   1,  0},    // Sneak 'n Peek (1982) (PAL).bin
    {"3a10562937a766cbbb77203d029b00e1",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   81,    245,   100,   1,  0},    // Sneak 'n Peek (1982) (PAL).bin
    {"b979e5b87a8ff44a5d5017631848b303",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Sniper 1.0.bin
    {"57939b326df86b74ca6404f64f89fce9",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  0},    // Snoopy and the Red Baron (1983).bin
    {"45a095645696a217e416e4bd2baea723",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  0},    // Snoopy and the Red Baron (1983).bin
    {"d2deddb77c8b823e4be9c57cb3c69adc",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  0},    // Snoopy and the Red Baron (1983).bin
    {"c5d2834bf98e90245e545573eb7e6bbc",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  0},    // Snoopy and the Red Baron (1983).bin
    {"01293bd90a4579abb7aed2f7d440681f",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   46,    245,    66,   4,  0},    // Snoopy and the Red Baron (1983) (PAL).bin
    {"f844f4c6f3baaaf5322657442d6f29eb",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   55,    245,    73,   4,  0},    // Snoopy and the Red Baron (1983) (PAL).bin
    {"75ee371ccfc4f43e7d9b8f24e1266b55",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Snow White (1982) [alt].bin
    {"75028162bfc4cc8e74b04e320f9e6a3f",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Snow White (1982).bin
    {"3f6dbf448f25e2bd06dea44248eb122d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   3,  2},    // Soccer (1989).bin
    {"947317a89af38a49c4864d6bdd6a91fb",  "SOLARF", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    90,   0,  2},    // Solar Fox (1983).bin
    {"e03b0b091bea5bc9d3f14ee0221e714d",  "SOLARF", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Solar Fox (1983) (PAL).bin
    {"b5be87b87fd38c61b1628e8e2d469cb5",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  32,    210,   100,   0,  0},    // Solar Plexus.bin
    {"97842fe847e8eb71263d6f92f7e122bd",  "SOLSTM", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  5},    // Solar Storm (1983).bin
    {"e6de4ef9ab62e2196962aa6b0dedac59",  "SOLSTM", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   60,    245,   100,   0,  0},    // Solar Storm (1983) (PAL).bin
    {"e72eb8d4410152bdcb69e7fba327b420",  "SOLARI", BANK_F6,   CTR_SOLARIS,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Solaris (1986).bin
    {"46ca1d37d9b9cb2324f33c69ef8ed4ef",  "SOLARI", BANK_F6,   CTR_GENESIS,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Solaris (2 Button Hack).bin
    {"bc4cf38a4bee45752dc466c98ed7ad09",  "SOLARI", BANK_F6,   CTR_SOLARIS,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   63,    245,   100,   3,  0},    // Solaris (1986) (PAL).bin
    {"f753c0799f8ed2c767d3241112b96408",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Snark (Prototype).bin
    {"0e2f1011c993d9181f76faab4b3514db",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Sonar (Prototype).bin
    {"d2c4f8a4a98a905a9deef3ba7380ed64",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  50,    210,   100,   3,  0},    // Sorcerer (1983).bin
    {"5f7ae9a7f8d79a3b37e8fc841f65643a",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Sorcerer's Apprentice (1983).bin
    {"d214c7a734e133a5c18e93229435b57a",  "??????", BANK_UASW, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Sorcerer's Apprentice (1983).bin (UA Swapped Banks)
    {"2e82a1628ef6c735c0ab8fa92927e9b0",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Sorcerer's Apprentice (1983) (PAL).bin
    {"25c97848ae6499e569b832b686a84bb2",  "SPPLUS", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    92,   0,  2},    // Sp+.bin
    {"17badbb3f54d1fc01ee68726882f26a6",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Space Attack (1982).bin
    {"abb741c83f665d73c86d90a7d9292a9b",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Space Attack (1982) (PAL).bin
    {"9f81edee8b4b5afbde0e49a6fe8da0de",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  5},    // Space Battle (NTSC).bin
    {"0efc91e45f61023cda9d086a7d3c402f",  "??????", BANK_DPCP, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    205,   100,   0,  0},    // Space Cactus Canyon FINAL.bin
    {"df6a28a89600affe36d94394ef597214",  "SPACAV", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Space Cavern (1981).bin
    {"559317712f989f097ea464517f1a8318",  "SPACAV", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Space Cavern (1981).bin
    {"d9548ad44e67edec202d1b8b325e5adf",  "SPACAV", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Space Cavern (1981) (PAL).bin
    {"72ffbef6504b75e69ee1045af9075f66",  "SPAINV", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  6},    // Space Invaders (1980).bin
    {"61dbe94f110f30ca4ec524ae5ce2d026",  "SPAINV", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  6},    // Space Invaders (1980).bin
    {"15b9f5e2439bfaa08874b5184261c777",  "SPAINV", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  6},    // Space Invaders (1980).bin
    {"8747ba79cd39fa83a529bb26010db21b",  "SPAINV", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  PAL,   50,    245,    73,   0,  0},    // Space Invaders (1980) (PAL).bin
    {"7cc77f6745e1f2b20df4a4327d350545",  "SPAINV", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  PAL,   50,    245,    73,   0,  0},    // Space Invaders (1980) (PAL).bin
    {"f1b7edff81ceef5af7ae1fa76c8590fc",  "SPAINV", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  PAL,   50,    245,    73,   0,  0},    // Space Invaders (1980) (PAL).bin
    {"b2a6f31636b699aeda900f07152bab6e",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    91,   0,  2},    // Space Invaders Deluxe.a26
    {"e074af84dcd5bd21fb48ca7f36845e61",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  6},    // Space Invaders Deluxe.a26
    {"e10bf1af6bf3b4a253c5bef6577fe923",  "SPARCD", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  6},    // Space Invaders Arcade (needs TIA Bus handling)
    {"6f2aaffaaf53d23a28bf6677b86ac0e3",  "SPJOCK", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Space Jockey (1982).bin
    {"457e7d4fcd56ebc47f5925dbea3ee427",  "SPJOCK", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   89,    245,   100,   0,  4},    // Space Jockey (1982) (PAL).bin
    {"e1d79e4e7c150f3861256c541ec715a1",  "SPJOCK", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   81,    245,   100,   0,  4},    // Space Jockey (1982) (PAL).bin
    {"822a950f27ff0122870558a89a49cad3",  "SPJOCK", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   64,    245,   100,   0,  4},    // Space Jockey (1982) (PAL).bin
    {"5db9e5bf663cad6bf159bc395f6ead53",  "SPJOCK", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   89,    245,   100,   0,  4},    // Space Jockey (1982) (PAL).bin
    {"00eaee22034aff602f899b684c107d77",  "SPJOCK", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   89,    245,   100,   0,  4},    // Space Jockey (1982) (PAL).bin
    {"6bb09bc915a7411fe160d0b2e4d66047",  "SPJOCK", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   81,    245,   100,   0,  4},    // Space Jockey (1982) (PAL).bin
    {"e784a9d26707cfcd170a4c1c60422a72",  "SPJOCK", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   89,    245,   100,   0,  4},    // Space Jockey (1982) (PAL).bin
    {"cb3a9b32a01746621f5c268db48833b2",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Space Raid.bin
    {"3dfb7c1803f937fadc652a3e95ff7dc6",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Space Robot (Dimax - Sinmax).bin
    {"82e7aab602c378cffdd8186a099e807e",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Space Robot (Dimax - Sinmax).bin
    {"1bef389e3dd2d4ca4f2f60d42c932509",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   57,    245,   100,   0,  2},    // Space Robot (Dimax - Sinmax) (PAL).bin
    {"c4d888bcf532e7c9c5fdeafbb145266a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   59,    245,   100,   0,  0},    // Space Robot (Dimax - Sinmax) (PAL).bin
    {"03db2942cfbea51633726853c81a7b17",  "SPAROC", BANK_DPCP, CTR_LJOY,      SPEC_DPCPNOC,   MODE_FF,   !VB,  !HB,  ANA1_0,  NTSC,  34,    202,   100,   0,  8},    // Space Rocks - Encore.bin
    {"6fce528556f11a1721db8cfc95d5547a",  "SPAROC", BANK_DPCP, CTR_LJOY,      SPEC_DPCPNOC,   MODE_NO,   !VB,  !HB,  ANA1_0,  NTSC,  34,    202,   100,   0,  8},    // Space Rocks - TE Encore.bin
    {"fe395b292e802ea16b3b5782b21ee686",  "SPAROC", BANK_DPCP, CTR_LJOY,      SPEC_DPCPNOC,   MODE_FF,   !VB,  !HB,  ANA1_0,  NTSC,  34,    202,   100,   0,  8},    // Space Rocks.bin
    {"898143773824663efe88d0a3a0bb1ba4",  "??????", BANK_FE,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    92,   0,  1},    // Space Shuttle (1983) [FE Bankswitching].bin
    {"5894c9c0c1e7e29f3ab86c6d3f673361",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    92,   0,  1},    // Space Shuttle (1983).bin
    {"4f6702c3ba6e0ee2e2868d054b00c064",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,    92,   0,  0},    // Space Shuttle (1983) (PAL).bin
    {"9e135f5dce61e3435314f5cddb33752f",  "SPACET", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Space Treat Deluxe.bin
    {"6c9a32ad83bcfde3774536e52be1cce7",  "SPACET", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Space Treat.bin
    {"df2745d585238780101df812d00b49f4",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Space Tunnel (1982) (Bit Corporation) (PG202).bin
    {"be3f0e827e2f748819dac2a22d6ac823",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Space Tunnel (1982).bin
    {"a7ef44ccb5b9000caf02df3e6da71a92",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Space War (1978).bin
    {"f9677b2ec8728a703eb710274474613d",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   58,    245,   100,   0,  0},    // Space War (1978) (PAL).bin
    {"63d6247f35902ba32aa49e7660b0ecaa",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   61,    245,   100,   0,  0},    // Space War (1978) (PAL).bin
    {"b702641d698c60bcdc922dbd8c9dd49c",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   61,    245,   100,   0,  0},    // Space War (1978) (PAL).bin
    {"1d566002bbc51e5eee73de4c595fd545",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // SpaceBattleFinal4N.bin
    {"ec5c861b487a5075876ab01155e74c6c",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  37,    210,   100,   0,  5},    // Spacechase (1981)bin
    {"89eaba47a59cbfd26e74aad32f553cd7",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Spacechase (1981) (PAL).bin
    {"94255d5c05601723a58df61726bc2615",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // SpaceGame 2K.bin
    {"44ca1a88274ff55787ed1763296b8456",  "SPGAME", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  5},    // SpaceGame-Final.bin
    {"45040679d72b101189c298a864a5b5ba",  "SPACX7", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // SpaceMaster X-7 (1983).bin
    {"bd551ff1264f5c367a3ad7cf0d2f266c",  "SPACX7", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // SpaceMaster X-7 (1983).bin
    {"e14feddeb82f5160ed5cf9ca4078e58d",  "SPACX7", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   60,    245,   100,   0,  3},    // SpaceMaster X-7 (1983) (PAL).bin
    {"24d018c4a6de7e5bd19a36f2b879b335",  "SPIDFI", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Spider Fighter (1982).bin
    {"0fc161704c46e16f7483f92b06c1558d",  "SPIDFI", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Spider Fighter (1982).bin
    {"c41e7735f6701dd50e84ee71d3ed1d8f",  "SPIDFI", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Spider Fighter (1982).bin
    {"92e72f7cc569584c44c9530d645ae04e",  "SPIDFI", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Spider Fighter (1982).bin
    {"b40dea357d41c5408546e4e4d5f27779",  "SPIDFI", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Spider Fighter (1982).bin
    {"cfd5518c71552b8bb853b0e461e328d7",  "SPIDFI", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Spider Fighter (1982).bin
    {"8786f229b974c393222874f73a9f3206",  "SPIDFI", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   42,    245,   100,   0, 11},    // Spider Fighter (1982) (PAL).bin
    {"4d38e1105c3a5f0b3119a805f261fcb5",  "SPIDFI", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   40,    245,   100,   0, 11},    // Spider Fighter (1982) (PAL).bin
    {"21299c8c3ac1d54f8289d88702a738fd",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  30,    210,    95,   0,  0},    // Spider Maze (1982).bin
    {"199eb0b8dce1408f3f7d46411b715ca9",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    96,   0,  0},    // Spider-Man (1982).bin
    {"e77ec259e1387bc308b0534647a89198",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   56,    245,   100,   0,  3},    // Spider-Man (1982) (PAL).bin
    {"f7af41a87533524d9a478575b0d873d0",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Spider-Man (1982) (PAL).bin
    {"8454ed9787c9d8211748ccddb673e920",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Spiderdroid (1987).bin
    {"cc2dde4d1990ad79cd25065e307d0832",  "??????", BANK_UA,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    87,   0,  5},    // Spiders (UA).bin
    {"aecdc8da1d67ad5fd520582750e19938",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Spies in the Night.bin
    {"a45be00760b87d6d490d27170c3b7dc3",  "??????", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  26,    210,   100,   0,  0},    // Spies in the Night2-Beta9.bin
    {"a4e885726af9d97b12bb5a36792eab63",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,    94,   0, 11},    // Spike's Peak (1983).bin
    {"b37f0fe822b92ca8f5e330bf62d56ea9",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   62,    245,    89,   0,  4},    // Spike's Peak (1983) (PAL).bin
    {"d3171407c3a8bb401a3a62eb578f48fb",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Spinning Fireball (1983).bin
    {"542c6dd5f7280179b51917a4cba4faff",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Spinning Fireball (1983).bin
    {"6c85098518d3f94f7622c42fd1d819ac",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   65,    245,   100,   0,  0},    // Spinning Fireball (1983) (PAL).bin
    {"98555b95cb38e0e0b22b482b2b60a5b6",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   65,    245,   100,   0,  0},    // Spinning Fireball (1983) (PAL).bin
    {"cef2287d5fd80216b2200fb2ef1adfa8",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Spitfire Attack (1983).bin
    {"6216bef66edceb8a24841e6065bf233e",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    210,    92,   0,  0},    // Splatform 2600 (v1.01).bin
    {"4cd796b5911ed3f1062e805a3df33d98",  "??????", BANK_3F,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    92,   0,  2},    // Springer (1982).bin
    {"133456269a03e3fdae6cddd65754c50d",  "??????", BANK_3F,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   70,    245,    92,   0,  2},    // Springer (1982) (PAL).bin
    {"5a8afe5422abbfb0a342fb15afd7415f",  "SPRINT", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  25,    200,   100,   0,  0},    // Sprint Master (1988).bin
    {"b2d5d200f0af8485413fad957828582a",  "SPRINT", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   56,    245,   100,   0,  0},    // Sprint Master (1988)(PAL).bin
    {"3105967f7222cc36a5ac6e5f6e89a0b4",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Spy Hunter (1984).bin
    {"22f749ea4d37db82f7cb84e2c6c9f4eb",  "??????", BANK_F8SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Squareraid_1.0Beta1.bin
    {"ba257438f8a78862a9e014d831143690",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Squeeze Box (1982).bin
    {"68878250e106eb6c7754bc2519d780a0",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    215,    75,   0,  0},    // Squirrel Against Snail (1983).bin
    {"0be9ad4729ecd50cadece9200f324a43",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  26,    216,    71,   0,  0},    // Squirrel Against Snail (1983).bin
    {"ac26d7d37248d1d8eac5eccacdbef8db",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,    86,   0,  0},    // Squirrel Against Snail (1983) (PAL).bin
    {"898b5467551d32af48a604802407b6e8",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   48,    245,    77,   0,  0},    // Squirrel Against Snail (1983) (PAL).bin
    {"aa8c75d6f99548309949916ad6cf33bc",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Squish-Em.bin
    {"34c808ad6577dbfa46169b73171585a3",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Squoosh (1983).bin
    {"22abbdcb094d014388d529352abe9b4b",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Squoosh (1983).bin
    {"21a96301bb0df27fde2e7eefa49e0397",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  39,    210,    80,   0,  0},    // Sssnake (1982).bin
    {"b3203e383b435f7e43f9492893c7469f",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   48,    245,    77,   0,  0},    // Sssnake (1982) (PAL).bin
    {"21d7334e406c2407e69dbddd7cec3583",  "STAMPE", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   0,  2},    // Stampede (1981).bin
    {"43adf60ebdd6b5a0fae21594ecf17154",  "STAMPE", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   0,  2},    // Stampede (1981).bin
    {"f77f5fc3893da5d00198e4cd96544aad",  "STAMPE", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   0,  2},    // Stampede (1981).bin
    {"1ea1abcd2d3d3d628f59a99a9d41b13b",  "STAMPE", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   0,  2},    // Stampede (1981).bin
    {"c9196e28367e46f8a55e04c27743148f",  "STAMPE", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  7},    // Stampede (1981) (PAL).bin
    {"869abe0426e6e9fcb6d75a3c2d6e05d1",  "STAMPE", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  7},    // Stampede (1981) (PAL).bin
    {"75511bb694662301c9e71df645f4b5a7",  "STAMPE", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   1, 14},    // Stampede (1981) (PAL).bin
    {"3f96eb711928a6fac667c04ecd41f59f",  "STAMPE", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  7},    // Stampede (1981) (PAL).bin
    {"bdecc81f740200780db04a107c3a1eba",  "STAMPE", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  7},    // Stampede (1981) (PAL).bin
    {"d9c9cece2e769c7985494b1403a25721",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Star Castle.bin
    {"2418fa5ecfb685f7bb70bc4527e9d966",  "??????", BANK_FA2,  CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Star Castle Arcade (Stella).bin
    {"6ea02420947c4e1727b064f89c533a61",  "??????", BANK_FA2,  CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Star Castle Arcade (Harmony).bin
    {"0c06e364b8af0f83b1cfbbe516c7ccb9",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // StarfighterV1_06.bin
    {"fb88baa01afd34e0e4b601e1d29bc806",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Star Fire.bin
    {"f526d0c519f5001adb1fc7948bfbb3ce",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   6,  9},    // Star Fox (1983).bin
    {"cbd981a23c592fb9ab979223bb368cd5",  "STRAID", BANK_F8,   CTR_STARRAID,  SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Star Raiders (1982).bin
    {"c1a83f44137ea914b495fc6ac036c493",  "STRAID", BANK_F8,   CTR_STARRAID,  SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   59,    245,   100,   0, 16},    // Star Raiders (PAL 1982).bin
    {"e363e467f605537f3777ad33e74e113a",  "??????", BANK_2K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Star Ship (1977).bin
    {"7b938c7ddf18e8362949b62c7eaa660a",  "??????", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Star Ship (1977).bin
    {"79e5338dbfa6b64008bb0d72a3179d3c",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Star Strike (1983).bin
    {"405f8591b6941cff56c9b392c2d5e4e5",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Star Strike (1983) (PAL).bin
    {"03c3f7ba4585e349dd12bfa7b34b7729",  "STARTR", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Star Trek - Strategic Operations Simulator (1983).bin
    {"30f0b49661cfcfd4ec63395fab837dc3",  "STARTR", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   34,    245,   100,   0,  0},    // Star Trek - Strategic Operations Simulator (1983) (PAL).bin
    {"ab8d318da4addd39c65b7f9c408df2a6",  "STARTR", BANK_F8,   CTR_GENESIS,   SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Star Trek - Strategic Operations Simulator (2 Button Hack).bin
    {"813985a940aa739cc28df19e0edd4722",  "STAVOY", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Star Voyager (1982).bin
    {"2e7e9c6dcfcceaffc6fa73f0d08a402a",  "STAVOY", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Star Voyager (1982).bin
    {"d912312349d90e9d41a9db0d5cd3db70",  "STAVOY", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Star Voyager (1982).bin
    {"0956285e24a18efa10c68a33846ca84d",  "STAVOY", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Star Voyager (1982).bin
    {"0aceb7c3bd13fe048b77a1928ed4267d",  "STAVOY", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  9},    // Star Voyager (1982) (PAL).bin
    {"5336f86f6b982cc925532f2e80aa1e17",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Star Wars - Death Star Battle (1983).bin
    {"cb9b2e9806a7fbab3d819cfe15f0f05a",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   55,    245,   100,   0,  0},    // Star Wars - Death Star Battle (1983) (PAL).bin
    {"d44d90e7c389165f5034b5844077777f",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    96,   0,  8},    // Star Wars - Ewok Adventure (1983).bin
    {"6dfad2dd2c7c16ac0fa257b6ce0be2f0",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,    96,   0,  0},    // Star Wars - Ewok Adventure (1983) (PAL).bin
    {"c9f6e521a49a2d15dac56b6ddb3fb4c7",  "??????", BANK_4K,   CTR_PADDLE1,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    98,   0,  0},    // Star Wars - Jedi Arena (1983).bin
    {"05b45ba09c05befa75ac70476829eda0",  "??????", BANK_4K,   CTR_PADDLE1,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   57,    245,    98,   0,  0},    // Star Wars - Jedi Arena (1983) (PAL).bin
    {"6339d28c9a7f92054e70029eb0375837",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Star Wars - The Arcade Game (1984).bin
    {"6cf054cd23a02e09298d2c6f787eb21d",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   55,    245,   100,   0,  2},    // Star Wars - The Arcade Game (1984) (PAL).bin
    {"6651e2791d38edc02c5a5fd7b47a1627",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Star Wars - The Arcade Game (1984 Prototype).bin
    {"3c8e57a246742fa5d59e517134c0b4e6",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Star Wars - The Empire Strikes Back (1982).bin
    {"be060a704803446c02e6f039ab12eb91",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   34,    245,    59,   0,  0},    // Star Wars - The Empire Strikes Back (1982) (PAL).bin
    {"0c48e820301251fbb6bcdc89bd3555d9",  "??????", BANK_F8SC, CTR_STARGATE,  SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Stargate (1984).bin
    {"493de059b32f84ab29cde6213964aeee",  "??????", BANK_F8SC, CTR_STARGATE,  SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   63,    245,   100,   0,  0},    // Stargate (1984) (PAL).bin
    {"a3c1c70024d7aabb41381adbfb6d3b25",  "STAGUN", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0, 12},    // Stargunner (1982).bin
    {"d69559f9c9dc6ef528d841bf9d91b275",  "STARMA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // StarMaster (1982).bin
    {"e5bacf526036d3c8c99db5b030cf00e7",  "STARMA", BANK_4K,   CTR_GENESIS,   SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // StarMaster (2 Button Hack).bin
    {"d62d7d1a974c31c5803f96a8c1552510",  "STARMA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // StarMaster (1982) (PAL).bin
    {"348615ffa30fab3cec1441b5a76e9460",  "STARMA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // StarMaster (1982) (PAL).bin
    {"73c839aff6a055643044d2ce16b3aaf7",  "STARMA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // StarMaster (1982) (PAL).bin
    {"00ce76ad69cdc2fa36ada01ae092d5a6",  "STARMA", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // StarMaster (1982) (PAL).bin
    {"c5bab953ac13dbb2cba03cd0684fb125",  "FROSTY", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    96,   0,  0},    // StayFrosty.bin
    {"541cac55ebcf7891d9d51c415922303f",  "FROST2", BANK_DPCP, CTR_LJOY,      SPEC_DPCPOPT,   MODE_NO,   !VB,  !HB,  ANA1_0,  NTSC,  34,    205,    91,   0,  3},    // StayFrosty2.bin
    {"f2564445e395cf1bebb76f92c5810095",  "FROST2", BANK_DPCP, CTR_LJOY,      SPEC_DPCPOPT,   MODE_NO,   !VB,  !HB,  ANA1_0,  NTSC,  34,    205,    91,   0,  3},    // StayFrosty2 - harmony fix.bin
    {"656dc247db2871766dffd978c71da80c",  "??????", BANK_2K,   CTR_PADDLE0,   SPEC_NONE,      MODE_FF,    VB,   HB,  ANA2_5,  NTSC,  34,    210,   100,   0,  5},    // Steeplechase (1980).bin
    {"1619bc27632f9148d8480cd813aa74c3",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA2_5,  NTSC,  27,    235,    93,   1,  1},    // Steeplechase (1983 Video Gems) (NTSC by TJ).bin
    {"f1eeeccc4bba6999345a2575ae96508e",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA2_5,  PAL,   52,    245,    82,   0, 10},    // Steeplechase (1983 Video Gems) (PAL).bin
    {"afcf38bda03206f29200bbea73257930",  "SSTOCK", BANK_X07,  CTR_LJOY,      SPEC_WAVEDIRC,  MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  6},    // Stella's Stocking
    {"0b8d3002d8f744a753ba434a4d39249a",  "STELTR", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Stellar Track (1980).bin
    {"23fad5a125bcd4463701c8ad8a0043a9",  "??????", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  31,    210,   100,   0,  0},    // Stone Age (1983).bin
    {"9333172e3c4992ecf548d3ac1f2553eb",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,   100,   0,  0},    // Strategy X (1983).bin
    {"b7345220a0c587f3b0c47af33ebe533c",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   55,    245,   100,   0,  0},    // Strategy X (1983) (PAL).bin
    {"9333172e3c4992ecf548d3ac1f2553eb",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   55,    245,   100,   0,  0},    // Strategy X (1983) (PAL).bin
    {"ef76ea05655a0b62cb1018c92b9b4b7d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   65,    245,   100,   0,  0},    // Strategy X (1983) (PAL).bin
    {"807a8ff6216b00d52aba2dfea5d8d860",  "??????", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // StratOGemsDeluxe.bin
    {"42946b04e3dc90df3e9d61e3a9636326",  "??????", BANK_DPCP, CTR_LJOY,      SPEC_NONE,      MODE_FF,   !VB,   HB,  ANA1_0,  NTSC,  34,    205,   100,   0,  0},    // Stratovox RC1.bin
    {"e10d2c785aadb42c06390fae0d92f282",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  30,    210,    72,   0,  0},    // Strawberry Shortcake (1983).bin
    {"516ffd008057a1d78d007c851e6eff37",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   51,    245,    72,   0,  0},    // Strawberry Shortcake (1983) (PAL).bin
    {"396f7bc90ab4fa4975f8c74abe4e81f0",  "??????", BANK_2K,   CTR_PADDLE1,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Street Racer (1977).bin
    {"e12e32dee68201b6765fcd0ed54d6646",  "??????", BANK_2K,   CTR_PADDLE1,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   36,    245,    51,   0,  0},    // Street Racer (1977) (PAL).bin
    {"8b2926c0716ecf062c27275467130573",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Strip Off (2009) (John A. Reder).bin
    {"7b3cf0256e1fa0fdc538caf3d5d86337",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  32,    210,   100,   0,  0},    // Stronghold (1983).bin
    {"c3bbc673acf2701b5275e85d9372facf",  "??????", BANK_2K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Stunt Cycle (1980).bin
    {"5af9cd346266a1f2515e1fbc86f5186a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  36,    210,   100,   0,  8},    // Sub-Scan (1982).bin
    {"b095009004df341386d22b2a3fae3c81",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  52,    245,   100,   0,  0},    // Sub-Scan (1982) (PAL).bin
    {"f3f5f72bfdd67f3d0e45d097e11b8091",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Submarine Commander (1982).bin
    {"40eb4e263581b3dfec6dd8920b68e00f",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   44,    245,   100,   0,  0},    // Submarine Commander (1982) (PAL).bin
    {"93c52141d3c4e1b5574d072f1afde6cd",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Subterranea (1983).bin
    {"38de7b68379770b9bd3f7bf000136eb0",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   57,    245,   100,   0,  0},    // Subterranea (1983) (PAL).bin
    {"cff578e5c60de8caecbee7f2c9bbb57b",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Suicide Adventure.bin
    {"e4c666ca0c36928b95b13d33474dbb44",  "??????", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_FF,   !VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  0},    // Suicide Mission (1982).bin
    {"08bf437d012db07b05ff57a0c745c49e",  "??????", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_FF,   !VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  0},    // Suicide Mission - Meteroids (1982).bin
    {"eb92193f06b645df0b2a15d077ce435f",  "??????", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_FF,   !VB,   HB,  ANA1_0,  PAL,   57,    230,   100,   0,  8},    // Suicide Mission (1982) (PAL).bin
    {"45027dde2be5bdd0cab522b80632717d",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,   100,   0,  0},    // Summer Games (1987).bin
    {"12bca8305d5ab8ea51fe1cfd95d7ab0e",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   56,    245,   100,   0, -1},    // Summer Games (1987) (PAL).bin
    {"7adbcf78399b19596671edbffc3d34aa",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    94,   0,  0},    // Super Baseball (1988).bin
    {"faed2ef6b44894f8c83f2b50891c35c6",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    94,   0,  0},    // Super Baseball (1988).bin
    {"0751f342ee4cf28f2c9a6e8467c901be",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   56,    245,    94,   0,  0},    // Super Baseball (1988) (PAL).bin
    {"8885d0ce11c5b40c3a8a8d9ed28cefef",  "SUPBRK", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    195,   100,   0,  0},    // Super Breakout (1982).bin
    {"0ad9a358e361256b94f3fb4f2fa5a3b1",  "SUPBRK", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    195,   100,   0,  0},    // Super Breakout (1982).bin
    {"ee4c186123d31a279ed7a84d3578df23",  "SUPBRK", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   36,    245,    52,   0, -2},    // Super Breakout (1982) (PAL).bin
    {"9d37a1be4a6e898026414b8fee2fc826",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   4,  4},    // Super Challenge Baseball (1982).bin
    {"034c1434280b0f2c9f229777d790d1e1",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   4, 11},    // Super Challenge Baseball (1982) (PAL).bin
    {"dab844deed4c752632b5e786b0f47999",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   4, 10},    // Super Challenge Baseball (1982) (PAL).bin
    {"e275cbe7d4e11e62c3bfcfb38fca3d49",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    92,   4,  2},    // Super Challenge Football (1982).bin
    {"d7b2259f6bb57bf37eac82365c1f8ad6",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Super Cobra (Prototype).bin
    {"c29f8db680990cb45ef7fef6ab57a2c2",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  6},    // Super Cobra (1982).bin
    {"5f7de62a408b9de3a1168898298fd31d",  "??????", BANK_E0,   CTR_GENESIS,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  6},    // Super Cobra (2 Button Hack).bin
    {"d326db524d93fa2897ab69c42d6fb698",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   49,    245,   100,   0,  0},    // Super Cobra (1982) (PAL).bin
    {"841057f83ce3731e6bbfda1707cbca58",  "??????", BANK_CDFJ, CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,  !HB,  ANA1_0,  NTSC,  32,    205,   100,   0,  0},    // Super Cobra Arcade (final).bin
    {"724613effaf7743cbcd695fab469c2a8",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Super Ferrari (Quelle).bin
    {"09abfe9a312ce7c9f661582fdf12eab6",  "SUPFBL", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Super Football (1988).bin
    {"2f0a8bb4e18839f9b1dcaa2f5d02fd1d",  "SUPFBL", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Super Football (1988).bin
    {"262ccb882ff617d9b4b51f24aee02cbe",  "SUPFBL", BANK_F6SC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   65,    245,   100,   3,  0},    // Super Football (1988) (PAL).bin
    {"e88eda5c9e0bd2f98d52a1721e4b229f",  "??????", BANK_FASC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // SuperCharger_SpaceInvaders_AFP_BETA4.bin
    {"5de8803a59c36725888346fdc6e7429d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Superman (1979) [fixed].bin
    {"a9531c763077464307086ec9a1fd057d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Superman (1979).bin
    {"149b543c917c180a1b02d33c12415206",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Superman (1979).bin
    {"dbb10b904242fcfb8428f372e00c01af",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   59,    245,   100,   0,  0},    // Superman (1979) (PAL).bin
    {"fd10915633aea4f9cd8b518a25d62b55",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,   100,   0,  0},    // Superman (1979) (PAL).bin
    {"6fac680fc9a72e0e54255567c72afe34",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,   100,   0,  0},    // Superman (1979) (PAL).bin
    {"a623b94f26e71ff180ff49583b51ba9f",  "??????", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  28,    210,   100,   0,  0},    // Super Mario Bros. SE.bin
    {"59b70658f9dd0e2075770b07be1a35cf",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  37,    215,    90,   0,  2},    // Surfer's Paradise - But Danger Below! (1983) (NTSC by TJ).bin
    {"c20f15282a1aa8724d70c117e5c9709e",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,   100,   0,  0},    // Surfer's Paradise - But Danger Below! (1983) (PAL).bin
    {"aec9b885d0e8b24e871925630884095c",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Surf's Up (1983).bin
    {"a2170318a8ef4b50a1b1d38567c220d6",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Surf's Up (1983).bin
    {"fd78f186bdff83fbad7f97cb583812fe",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Surf's Up (1983).bin
    {"4d7517ae69f95cfbc053be01312b7dba",  "??????", BANK_2K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    80,   0,  3},    // Surround (1977).bin
    {"31d08cb465965f80d3541a57ec82c625",  "??????", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Surround (4k).bin
    {"c370c3268ad95b3266d6e36ff23d1f0c",  "??????", BANK_2K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,    63,   0,  0},    // Surround (PAL).bin
    {"a60598ad7ee9c5ccad42d5b0df1570a1",  "??????", BANK_2K,   CTR_RJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,    63,   0,  0},    // Surround (PAL).bin
    {"045035f995272eb2deb8820111745a07",  "??????", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    207,   100,   4,  9},    // Survival Island (1983).bin
    {"84db818cd4111542a15c2a795369a256",  "??????", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    235,   100,   4,  0},    // Survival Island (1983) (PAL).bin
    {"85e564dae5687e431955056fbda10978",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  30,    215,    77,   0,  3},    // Survival Run (1983).bin
    {"59e53894b3899ee164c91cfa7842da66",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0, 10},    // Survival Run (Data Age).bin
    {"e51c23389e43ab328ccfb05be7d451da",  "??????", BANK_AR,   CTR_RJOY,      SPEC_AR,        MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Sweat! - The Decathlon (1983).bin
    {"c7600d72247c5dfa1ec1a88d23e6c85e",  "??????", BANK_AR,   CTR_RJOY,      SPEC_AR,        MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Sweat! - The Decathlon (1983).bin
    {"278f14887d601b5e5b620f1870bc09f6",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  30,    210,    72,   0,  0},    // Swoops! (v0.96).bin
    {"528400fad9a77fd5ad7fc5fdc2b7d69d",  "??????", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Sword of Saros (1983).bin
    {"8b7ca29a55432f886cee3d452fb00481",  "??????", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,    89,   0,  4},    // Sword of Saros (1983) (PAL).bin
    {"37f8ad4cbd23abf4fe8cbb499554c233",  "SSURTR", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Sword of Surtr.bin
    {"87662815bc4f3c3c86071dc994e3f30e",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Swordfight (1983).bin
    {"5aea9974b975a6a844e6df10d2b861c4",  "EARTHW", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // SwordQuest - EarthWorld (1982).bin
    {"a875f0a919129b4f1b5103ddd200d2fe",  "EARTHW", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   47,    245,    85,   0,  0},    // SwordQuest - EarthWorld (1982) (PAL).bin
    {"f9d51a4e5f8b48f68770c89ffd495ed1",  "FIREWO", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  6},    // SwordQuest - FireWorld (1982).bin
    {"112afb9c6777d652968f31328a327a01",  "FIREWO", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  6},    // SwordQuest - FireWorld (Finalist Cart).bin
    {"bf976cf80bcf52c5f164c1d45f2b316b",  "FIREWO", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   67,    245,   100,   0,  0},    // SwordQuest - FireWorld (1982) (PAL).bin
    {"bc5389839857612cfabeb810ba7effdc",  "WATERW", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    94,   0,  6},    // SwordQuest - WaterWorld (1983).bin
    {"c0eee10f8868de535c9ac0995ef3f6dc",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Sync.bin
    {"2c2aea31b01c6126c1a43e10cacbfd58",  "??????", BANK_F8,   CTR_KEYBOARD1, SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Synth Cart.bin
    {"d0b9f705aa5f61f47a748a66009ae2d2",  "??????", BANK_F8,   CTR_KEYBOARD1, SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Synth Cart.bin
    {"d45ebf130ed9070ea8ebd56176e48a38",  "TACSCN", BANK_4K,   CTR_PADDLE3,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_6,  NTSC,  50,    210,   100,   0,  0},    // Tac-Scan (1982).bin
    {"4892b85c248131d6a42c66a4163a40d0",  "TACSCN", BANK_4K,   CTR_PADDLE3,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_6,  NTSC,  50,    210,   100,   0,  0},    // Tac-Scan (1982).bin
    {"6aca52e11b597ab84b33d5252e1cd9d1",  "TACSCN", BANK_4K,   CTR_PADDLE3,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_6,  NTSC,  50,    210,   100,   0,  0},    // Tac-Scan (1982).bin
    {"06e5dc181a8eda1c31cc7c581c68b6ef",  "TACSCN", BANK_4K,   CTR_PADDLE1,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_6,  PAL,   59,    245,   100,   0,  0},    // Tac-Scan (1982) (PAL).bin
    {"e0ca5b3062453c9ae9b3cec18a585e03",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Tank Blitz (Prototype).bin
    {"c77d3b47f2293e69419b92522c6f6647",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  33,    210,    81,   0,  0},    // Tank Brigade (1983).bin
    {"fa6fe97a10efb9e74c0b5a816e6e1958",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  33,    210,    80,   0,  0},    // Tanks But No Tanks (1983).bin
    {"de3d0e37729d85afcb25a8d052a6e236",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  35,    210,    85,   0,  2},    // Tapeworm (1982).bin
    {"33cac5e767a534c95d292b04f439dc37",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  35,    210,    85,   0,  2},    // Tapeworm (1982).bin
    {"8ed73106e2f42f91447fb90b6f0ea4a4",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  68,    245,   100,   0,  0},    // Tapeworm (1982) (PAL).bin
    {"c0d2434348de72fa6edcc6d8e40f28d7",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  30,    210,    72,   0,  0},    // Tapper (1984).bin
    {"2d6741cda3000230f6bbdd5e31941c01",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Targ (1983).bin
    {"6cab04277e7cd552a3e40b3c0e6e1e3d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Targ (Universal Chaos Beta).bin
    {"0c35806ff0019a270a7acae68de89d28",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Task Force (1987).bin
    {"a1ead9c181d67859aa93c44e40f1709c",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Tax Avoiders (1982).bin
    {"7574480ae2ab0d282c887e9015fdb54c",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Taz (1983).bin
    {"360ba640f6810ec902b01a09cc8ab556",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   66,    245,   100,   0,  0},    // Taz (1983) (PAL).bin
    {"3d7aad37c55692814211c8b590a0334c",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    90,   3,  2},    // Telepathy (1983).bin
    {"c830f6ae7ee58bcc2a6712fb33e92d55",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Tempest (1984).bin
    {"42cdd6a9e42a3639e190722b8ea3fc51",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  33,    197,   100,   0,  0},    // Tennis (1981).bin
    {"04b488d4eef622d022a0021375e7e339",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  33,    197,   100,   0,  0},    // Tennis (1981) (4K).bin
    {"73efa9f3cbe197f26e0fb87132829232",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  33,    197,   100,   0,  0},    // Tennis (1981) (4K).bin
    {"cfce5596a7e8ca13529e9804cad693ef",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  33,    197,   100,   0,  0},    // Tennis (1981) (4K).bin
    {"a3873d7c544af459f40d58dfcfb78887",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  33,    197,   100,   0,  0},    // Tennis (1981).bin
    {"30685b9b6ebd9ba71536dd7632a1e3b6",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  33,    197,   100,   0,  0},    // Tennis (1981).bin
    {"04cf9e6898007024622ed6a0b295961f",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  33,    197,   100,   0,  0},    // Tennis (1981).bin
    {"61e0f5e1cc207e98704d0758c68df317",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  33,    197,   100,   0,  0},    // Tennis (1981).bin
    {"a3873d7c544af459f40d58dfcfb78887",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  33,    197,   100,   0,  0},    // Tennis (1981).bin
    {"a5c96b046d5f8b7c96daaa12f925bef8",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,   100,   4,  9},    // Tennis (1981) (PAL).bin
    {"053d1e12003c6a5155a1a4b4c8d199b2",  "??????", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,   100,   0, -4},    // Terrance and Phillip - Asses Of Fire (2008) (Chris Read).bin
    {"5eeb81292992e057b290a5cd196f155d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    98,   4,  6},    // Texas Chainsaw Massacre (1983).bin
    {"2d15b092e8350912ec4b2e5e750fa1c6",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    98,   4,  6},    // Texas Chainsaw Massacre, The (Prototype).bin
    {"f7ebf3dfbd6a3ff5ebc2709c4139a53a",  "??????", BANK_DPCP, CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,  !HB,  ANA1_0,  NTSC,  34,    204,   100,   0,  2},    // The End NTSC - RC6.bin
    {"7d3730b7c3199a49cb4189926179653e",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0, -3},    // TheHorde.bin
    {"32ee2063bbec93a159d99b64db2285f9",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // The Stacks (2011) (Mike Mika, Kevin Wilson).bin
    {"1d7636a157d5aa1880d9cbd88516f45e",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,   100,   0,  4},    // Thewickedfather-270514.bin
    {"4b90d7d0467269f7b3ee128db491b318",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // The Wicked Father Remix (2012) (Juno).bin
    {"5fb71cc60e293fe10a5023f11c734e55",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // This Planet Sucks.bin
    {"b7d0aae399781b3c18679debda6d32b1",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  35,    210,    93,   0,  0},    // Three.s (v1.02).bin
    {"e63a87c231ee9a506f9599aa4ef7dfb9",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  22,    210,    73,   0,  0},    // Threshold (1982).bin
    {"85b6071974aca39e980d48064ae5e847",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  22,    210,    73,   0,  0},    // Threshold (1982).bin
    {"67684a1d18c85ffa5d82dab48fd1cb51",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   49,    245,    82,   0,  0},    // Threshold (1982) (PAL).bin
    {"de7bca4e569ad9d3fd08ff1395e53d2d",  "??????", BANK_F6,   CTR_BOOSTER,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Thrust (V1.22) (Thomas Jentzsch) (Booster Grip).bin
    {"943798452ceba9357e2c56303cadb4f7",  "??????", BANK_F6,   CTR_GENESIS,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Thrust (V1.28) (Thomas Jentzsch) (Genesis Controller).bin
    {"7ded20e88b17c8149b4de0d55c795d37",  "??????", BANK_F6,   CTR_BOOSTER,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Thrust v1.26 (PD).bin
    {"cf507910d6e74568a68ac949537bccf9",  "THUNGR", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Thunderground (1983).bin
    {"ad8072675109d13fdd31a2e0403d5cff",  "THUNGR", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Thunderground (1983).bin
    {"bcb31f22856b0028c00d12f0e4c0a952",  "THUNGR", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Thunderground (1983).bin
    {"08bd4c1dcc843f6a0b563d9fd80b3b11",  "THUNGR", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   65,    245,   100,   0,  0},    // Thunderground (1983) (PAL).bin
    {"1428029e762797069ad795ce7c6a1a93",  "THUNGR", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   57,    245,   100,   0,  0},    // Thunderground (1983) (PAL).bin
    {"c032c2bd7017fdfbba9a105ec50f800e",  "??????", BANK_FE,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   3,  6},    // Thwocker (1984).bin
    {"1414328eb3e3fdd4225ac779bdc11d05",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   3,  5},    // Thwockerv2Hi-DiffDemo.bin
    {"7228299994ddf3782e491a78770e9a79",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   3,  5},    // Thwockerv01fix.bin
    {"fc2104dd2dadf9a6176c1c1c8f87ced9",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Time Pilot (1983).bin
    {"49f2cef5269fd06218be9f9474c74f8d",  "??????", BANK_UA,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Time Pilot (1983).bin
    {"b879e13fd99382e09bcaf1d87ad84add",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Time Warp (1982).bin
    {"bc3057a35319aae3a5cd87a203736abe",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Time Warp (1982).bin
    {"619de46281eb2e0adbb98255732483b4",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Time Warp (1982).bin
    {"71f09f128e76eb14e244be8f44848759",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   47,    245,   100,   0,  0},    // Time Warp (1982) (PAL).bin
    {"d6d1ddd21e9d17ea5f325fa09305069c",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   46,    245,   100,   0,  0},    // Time Warp (1982) (PAL).bin
    {"6d9afd70e9369c2a6bff96c4964413b7",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   46,    245,   100,   0,  0},    // Time Warp (1982) (PAL).bin
    {"40742a6770f0fdccceb9ffc7af32a1d7",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Tinkernut World (2010) (Tinkernut).bin
    {"953c45c3dd128a4bd5b78db956c455bb",  "??????", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // TitanAxe.bin
    {"12123b534bdee79ed7563b9ad74f1cbd",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  36,    210,   100,   2,  3},    // Title Match Pro Wrestling (1987).bin
    {"153f40e335e5cb90f5ce02e54934ab62",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   65,    245,   100,   0,  0},    // Title Match Pro Wrestling (1987) (PAL).bin
    {"8bc0d2052b4f259e7a50a7c771b45241",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  26,    210,    88,   0,  0},    // Tomarc the Barbarian (1983) [alt].bin
    {"32dcd1b535f564ee38143a70a8146efe",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  26,    210,    88,   0,  0},    // Tomarc the Barbarian (1983).bin
    {"e68e28752d3c54edd3ccda42c27e320c",  "??????", BANK_F8,   CTR_GENESIS,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,   100,   0,  0},    // Tomarc the Barbarian (2 Button Hack).bin
    {"447215c0e4fe0d0ebeb3aab09024af5c",  "??????", BANK_F6,   CTR_GENESIS,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0, -1},    // Tombstones Beta 3.bin
    {"3ac6c50a8e62d4ce71595134cbd8035e",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Tomcat - F-14 Fighter (1988).bin
    {"2ac3a08cfbf1942ba169c3e9e6c47e09",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   2,  2},    // Tomcat - F-14 Fighter (1988) (PAL).bin
    {"fa2be8125c3c60ab83e1c0fe56922fcb",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    210,    75,   0,  0},    // Tooth Protectors (1983).bin
    {"01abcc1d2d3cba87a3aa0eb97a9d7b9c",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  30,    210,   100,   4,  0},    // Topy (unknown).bin
    {"d45ba53d74811b7a12fcff4427777fbc",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  2},    // Tower of Rubble 2600-NTSC-v1.1.bin
    {"0aa208060d7c140f20571e3341f5a3f8",  "??????", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  32,    210,   100,   0,  0},    // Towering Inferno (1982).bin
    {"0c7bd935d9a7f2522155e48315f44fa0",  "??????", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   57,    245,    96,   0,  3},    // Towering Inferno (1982) (PAL).bin
    {"15fe28d0c8893be9223e8cb2d032e557",  "??????", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   73,    245,    96,   0,  8},    // Towering Inferno (1982) (PAL).bin
    {"f39e4bc99845edd8621b0f3c7b8c4fd9",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,    81,   0,  0},    // ToyshopTrouble.bin (F8 banking)
    {"fef9a1e86cf744ce06406e29f3aa6f2a",  "??????", BANK_0840, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,    81,   0,  0},    // ToyshopTrouble.bin (original 0840 Banking)
    {"6ae4dc6d7351dacd1012749ca82f9a56",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    97,   0,  4},    // Track and Field (1984).bin
    {"192aa2e8c795c9e10a7913e5d41feb81",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,    90,   0,  6},    // Track and Field (1984) (PAL).bin
    {"5f9e3d323a21bbd566b56605c9cb6a69",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    210,   100,   0,  0},    // TRASH_16k_Garbage_Day.bin
    {"24d468d81ac91c521d01c05904dcad95",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0, -2},    // Trashmania - Remix (2012) (Jonathon Bont).bin
    {"81414174f1816d5c1e583af427ac89fc",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  37,    210,    82,   2,  2},    // Treasure Below (Video Gems, Thomas Jentzsch) (NTSC).bin
    {"66706459e62514d0c39c3797cbf73ff1",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   50,    245,    82,   2,  0},    // Treasure Below (Video Gems) (PAL).bin
    {"4717908b53804e634db39c9d98b30fcf",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   5,  6},    // Treasure Discovery (Treasure Island) (Quelle) (NTSC by TJ).bin
    {"1bb91bae919ddbd655fa25c54ea6f532",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   64,    245,   100,   3,  0},    // Treasure Discovery (Treasure Island) (Quelle) (PAL).bin
    {"6c878b6983558ce03facee8b37ea3f21",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,   100,   0,  0},    // Treasure Hunt (2012).bin
    {"24df052902aa9de21c2b2525eb84a255",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Trick Shot (1982).bin
    {"03fbcee0bc80e31f27254aea3d920510",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Trick Shot (1982).bin
    {"93c9f9239a4e5c956663dd7affa70da2",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   62,    245,   100,   0,  0},    // Trick Shot (1982) (PAL).bin
    {"097936b07e0e0117b9026ae6835eb168",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   61,    245,   100,   0,  0},    // Trick Shot (1982) (PAL).bin
    {"fb27afe896e7c928089307b32e5642ee",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   1,  1},    // TRON - Deadly Discs (1982).bin
    {"c1f209d80f0624dada5866ce05dd3399",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   63,    245,   100,   1,  0},    // TRON - Deadly Discs (1982) (PAL).bin
    {"aaea37b65db9e492798f0105a6915e96",  "??????", BANK_AR,   CTR_PADDLE0,   SPEC_AR,        MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Tug of War, Wizard's Keep.bin
    {"6c2c0902a50b9578432bdeb329ccbcf2",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Tumble Temple (V2.5NTSC).bin
    {"b2737034f974535f5c0c6431ab8caf73",  "??????", BANK_FASC, CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  56,    210,   100,   0,  0},    // Tunnel Runner (1983).bin
    {"807841df228ee8aab0a06ee639ce5a8a",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,   100,   0, -1},    // Turbo (1982) (Coleco) Prototype.bin
    {"71edab64f54783132536bef7c141a63f",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  32,    210,   100,   0, -1},    // Turbo (1982) (Coleco) Prototype.bin
    {"7a5463545dfb2dcfdafa6074b2f2c15e",  "TURMOI", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  7},    // Turmoil (1982).bin
    {"46258bd92b1f66f4cb47864d7654f542",  "TURMOI", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  7},    // Turmoil (1982).bin
    {"67631ea5cfe44066a1e76ddcb6bcb512",  "TURMOI", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   72,    245,   100,   0,  5},    // Turmoil (1982) (PAL).bin
    {"4f7a6ba1baac8d25ce524eb660cca78d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   60,    210,    91,   0,  5},    // Turtle Race (Bit Corporation).bin
    {"085322bae40d904f53bdcc56df0593fc",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  42,    210,   100,   0,  2},    // Tutankham (1983).bin
    {"66c2380c71709efa7b166621e5bb4558",  "??????", BANK_E0,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   54,    245,   100,   0,  0},    // Tutankham (1983) (PAL).bin
    {"00655e50cf62004b1127cfc42a74cac6",  "??????", BANK_AR,   CTR_LJOY,      SPEC_AR,        MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  29,    210,   100,   0,  0},    // Twisty Passages.bin
    {"d4a18df0f55a1a5090318c75fce9ff7f",  "??????", BANK_DPCP, CTR_LJOY,      SPEC_NONE,      MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    204,   100,   0,  0},    // Tyre_Trax_ntsc_v273.bin
    {"137373599e9b7bf2cf162a102eb5927f",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Ultra SCSICide.bin
    {"81a010abdba1a640f7adf7f84e13d307",  "UCHAOS", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BACKG, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Universal Chaos (1989).bin
    {"e020f612255e266a8a6a9795a4df0c0f",  "UCHAOS", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   58,    245,   100,   0,  0},    // Universal Chaos (1989) (PAL).bin
    {"73e66e82ac22b305eb4d9578e866236e",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  26,    210,   100,   0, -5},    // Unknown Datatech Game.bin
    {"5f950a2d1eb331a1276819520705df94",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    210,    94,   0,  2},    // Heart Like a Wheel Prototype (1983).bin
    {"ee681f566aad6c07c61bbbfc66d74a27",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   3,  2},    // UnknownActivision1_NTSC.bin
    {"841b7bc1cad05f5408302308777d49dc",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   3,  2},    // UnknownActivision1_NTSC.bin
    {"a499d720e7ee35c62424de882a3351b6",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    87,   2, -1},    // Up 'n Down (1984).bin
    {"a128b1a092e3d16c4f7793e5ee50e67a",  "UPPLUS", BANK_F4,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // UPP+
    {"b16cb64a306a56f2bc491cbe5fb50295",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Uppa Creek! (2010) (Jason Santuci).bin
    {"c6556e082aac04260596b4045bc122de",  "VANGRD", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Vanguard (1982).bin
    {"88d7b6b3967de0db24cdae1c7f7181bd",  "VANGRD", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Vanguard (1982).bin
    {"7ef74879d7cb9fa0ef161b91ad55b3bb",  "VANGRD", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Vanguard (1982).bin
    {"d15655fe355fa57dd541487dc5725145",  "VANGRD", BANK_UA,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Vanguard (1982).bin
    {"f9660ebed66fee8bdfdf07b4faa22941",  "VANGRD", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Vanguard (1982).bin
    {"3caa902ac0ce4509308990645876426a",  "VANGRD", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   63,    245,   100,   0,  1},    // Vanguard (1982) (PAL).bin
    {"bf7389cbfee666b33b8a88cc6c830edd",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Vault (TE).bin
    {"787ebc2609a31eb5c57c4a18837d1aee",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Vault Assault.bin
    {"19b3b807507653516985ba95da92499d",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // VCS Draw Demo.bin
    {"d08fccfbebaa531c4a4fa7359393a0a9",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  9},    // Venetian Blinds Demo (1982).bin
    {"3e899eba0ca8cd2972da1ae5479b4f0d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Venture (1982).bin
    {"c63a98ca404aa5ee9fcff1de488c3f43",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Venture (1987).bin
    {"345758747b893e4c9bdde8877de47788",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   55,    245,   100,   0,  0},    // Venture (1983) (PAL).bin
    {"7412f6788087d7e912c33ba03b36dd1b",  "VENREL", BANK_F4SC, CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Venture Reloaded (RC3).bin
    {"0956285e24a18efa10c68a33846ca84d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Viagem Espacial.bin
    {"b9ad59fc4694c0e63dd03933ceb3c5c0",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Video Cartridge Bicycle Trainer (Prototype).bin
    {"539d26b6e9df0da8e7465f0f5ad863b7",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  1},    // Video Checkers (1980).bin
    {"193f060553ba0a2a2676f91d9ec0c555",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   48,    245,    61,   0, -3},    // Video Checkers (1980) (PAL).bin
    {"f0b7db930ca0e548c41a97160b9f6275",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Video Chess (1979).bin
    {"3ef9573536730dcd6d9c20b6822dbdc4",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Video Chess (1979) (PAL).bin
    {"ed1492d4cafd7ebf064f0c933249f5b0",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Video Cube (CCE).bin
    {"4191b671bcd8237fc8e297b4947f2990",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  4},    // Video Jogger (1983).bin
    {"4209e9dcdf05614e290167a1c033cfd2",  "??????", BANK_CV,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Video Life (1981).bin
    {"3ec12372ca3e870b11ca70edc7ec26a4",  "??????", BANK_CV,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Video Life (1981).bin
    {"497f3d2970c43e5224be99f75e97cbbb",  "??????", BANK_CV,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Video Life (1981).bin
    {"14d365bbfaac3d20c6119591f57acca4",  "??????", BANK_CV,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Video Life (1981).bin
    {"60e0ea3cbe0913d39803477945e9e5ec",  "??????", BANK_2K,   CTR_PADDLE1,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    210,    88,   0,  0},    // Video Olympics (1977).bin
    {"77d0a577636e1c9212aeccde9d0baa4b",  "??????", BANK_2K,   CTR_PADDLE1,   SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   56,    245,    92,   0,  0},    // Video Olympics (1977) (PAL).bin
    {"107cc025334211e6d29da0b6be46aec7",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Video Pinball (1981).bin
    {"6e59dd52f88c00d5060eac56c1a0b0d3",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  PAL,   56,    240,    75,   0,  2},    // Video Pinball (1981) (PAL).bin
    {"a2424c1a0c783d7585d701b1c71b5fdc",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  PAL,   56,    240,   100,   0,  0},    // Video Pinball (1981) (PAL).bin
    {"5a2f2dcd775207536d9299e768bcd2df",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_BLACK, VB,   HB,  ANA1_0,  PAL,   56,    240,   100,   0,  0},    // Video Pinball (1981) (PAL).bin
    {"ee659ae50e9df886ac4f8d7ad10d046a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Video Reflex (1983).bin
    {"297236cb9156be35679f83c4e38ee169",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Video Reflex (1983).bin
    {"6c128bc950fcbdbcaf0d99935da70156",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  3},    // Volleyball (1983).bin
    {"df9384d371588cd0400b29b03350eb62",  "VROOM!", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  28,    210,    88,   0,  0},    // VROOM! (v1.0) (PlusROM) (NTSC).bin
    {"6041f400b45511aa3a69fab4b8fc8f41",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Wabbit (1982).bin
    {"45abcf993b65a9a1bbb49a019d5556dc",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  29,    215,    73,   0,  0},    // Walker (NTSC by TJ).bin
    {"d175258b2973b917a05b46df4e1cf15d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   40,    240,    79,   0,  0},    // Walker (1983) (PAL).bin
    {"7ff53f6922708119e7bf478d7d618c86",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   40,    240,    79,   0,  0},    // Walker (1983) (PAL).bin
    {"d3456b4cf1bd1a7b8fb907af1a80ee15",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},    // Wall Ball (1983).bin
    {"5da448a2e1a785d56bf4f04709678156",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Wall Jump Ninja.bin
    {"c16fbfdbfdf5590cc8179e4b0f5f5aeb",  "WALDEF", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  33,    210,    78,   0,  0},    // Wall-Defender (1983).bin
    {"03ff9e8a7af437f16447fe88cea3226c",  "WALDEF", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  31,    210,    85,   0, -1},    // Wall-Defender (1983)[alt].bin
    {"eae6a5510055341d3abeb45667bb3e9b",  "WALDEF", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,    78,   0,  0},    // Wall-Defender (1983) (PAL).bin
    {"372bddf113d088bc572f94e98d8249f5",  "WALDEF", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,    78,   0,  0},    // Wall-Defender (1983) (PAL).bin
    {"cbe5a166550a8129a5e6d374901dffad",  "WARLOR", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_FF,    VB,   HB,  ANA0_8,  NTSC,  34,    210,   100,   0,  0},    // Warlords (1981).bin
    {"0c80751f6f7a3b370cc9e9f39ad533a7",  "WARLOR", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_FF,    VB,   HB,  ANA0_8,  PAL,   39,    245,    61,   0,  1},    // Warlords (1981) (PAL).bin
    {"679e910b27406c6a2072f9569ae35fc8",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Warplock (1982) [alt].bin
    {"a20bc456c3b5fd9335e110df6e000e12",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Warplock (1982).bin
    {"d1c3520b57c348bc21d543699bc88e7e",  "??????", BANK_4K,   CTR_PADDLE0,   SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   49,    245,   100,   0,  0},    // Warplock (1982) (PAL).bin
    {"7e7c4c59d55494e66eef5e04ec1c6157",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Warring Worms.bin
    {"3b7d5daca420319bf13aeec42a54be3e",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    96,   0,  6},    // Water Diver (R3).bin
    {"6d1748c000a1c424a635436e66c45c71",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  31,    210,   100,   0,  0},    // White Water Madness.bin
    {"0cdd9cc692e8b04ba8eb31fc31d72e5e",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  25,    210,   100,   0, 11},    // Wing War (Imagic, Thomas Jentzsch) (NTSC).bin
    {"4e02880beeb8dbd4da724a3f33f0971f",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   57,    250,   100,   0,  1},    // Wing War (Imagic) (PAL).bin
    {"8e48ea6ea53709b98e6f4bd8aa018908",  "??????", BANK_FASC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  9},    // Wings (1983).bin
    {"827a22b9dffee24e93ed0df09ff8414a",  "??????", BANK_FASC, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Wings (1983) (PAL).bin
    {"83fafd7bd12e3335166c6314b3bde528",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Winter Games (1987).bin
    {"8c36ed2352801031516695d1eeefe617",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   61,    245,   100,   0,  0},    // Winter Games (1987) (PAL).bin
    {"7b24bfe1b61864e758ada1fe9adaa098",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Wizard (1980).bin
    {"c43bd363e1f128e73ba5f0380b6fd7e3",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Wizard (1980).bin
    {"7e8aa18bc9502eb57daaf5e7c1e94da7",  "WIZWOR", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  0},    // Wizard of Wor (1982).bin
    {"3545eb3b8b1e7dc19f87d231ab0b1d4c",  "WIZWOR", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  0},    // Wizard of Wor (1982).bin
    {"913631e0367bdae33a13613a263adfb9",  "WIZWOR", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  0},    // Wizard of Wor - Arena (1982).bin
    {"4a16174fd9ca19a11f65645aff0ab1ee",  "WIZWOR", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  0},    // Wizard of Wor - Bunkers (1982).bin
    {"c1f3203638cb287b605b6d2f367d02ed",  "WIZWOR", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  0},    // Wizard of Wor - Caverns (1982).bin
    {"ad10970adbe82c4604188e767db3d582",  "WIZWOR", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  0},    // Wizard of Wor - Columns (1982).bin
    {"85debc8cb66de5d8be3fe5cca24cc3ac",  "WIZWOR", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  0},    // Wizard of Wor - Depths (1982).bin
    {"55c88eea3522945325a1ad5b59864bd3",  "WIZWOR", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  0},    // Wizard of Wor - Dungeons (1982).bin
    {"c1dbcbc9ff88567f4e12596c601a216c",  "WIZWOR", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  0},    // Wizard of Wor - Lords (1982).bin
    {"ec9f5e944727f0770fc1f045b97490ce",  "WIZWOR", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  0},    // Wizard of Wor - Magic (1982).bin
    {"92cb8d1938c2e5855d401c8ea14d0fa7",  "WIZWOR", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  0},    // Wizard of Wor - Masters (1982).bin
    {"9eeb80df5f07f1ec1b550c2bf1527089",  "WIZWOR", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  0},    // Wizard of Wor - Pit (1982).bin
    {"f05f8bc7ba4bf265162e3d237d8faee8",  "WIZWOR", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    200,   100,   0,  0},    // Wizard of Wor - World (1982).bin
    {"663ef22eb399504d5204c543b8a86bcd",  "WIZWOR", BANK_4K,   CTR_RJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   70,    245,   100,   0,  0},    // Wizard of Wor (PAL).bin
    {"620a6db9cfd11ef5f2249e6e358dbaba",  "WIZWOR", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0, -2},    // Wizard of Wor (Arcade - F6 Hack)
    {"7c9c8c03c563e5972083927f721be86f",  "WIZWOR", BANK_F6,   CTR_RJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0, -2},    // Wizard of Wor (Arcade - F6 DSB Hack)
    {"af4f98bca8e01d6c50942dc047487213",  "WIZWOR", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0, -2},    // Wizard of Wor (Arcade - F6 Hack - Earlier Ver)
    {"dfbb975a2700b115d1bab244fb7bad6d",  "WIZWOR", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0, -2},    // Wizard of Wor (Arcade - F8 Hack)
    {"442b96451cc60263ffeb457b175348c9",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Wolfenstein VCS.bin
    {"6c39fce12a64c06ad878645df301fde0",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Wolfenstein-TheNextMission.bin
    {"ec3beb6d8b5689e867bafb5d5f507491",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Word Zapper (1982).bin
    {"e1143b72a30d4d3fee385eec38b4aa4d",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Word Zapper (1982).bin
    {"3a53963f053b22599db6ac9686f7722f",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Word Zapper (1982) (PAL).bin
    {"37527966823ee9243d34c7da8302774f",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Word Zapper (1982) (PAL).bin
    {"1b59db76ba41dab70813813f189bc32c",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  30,    210,   100,   0,  0},    // Words Attack.bin
    {"2facd460a6828e0e476d3ac4b8c5f4f7",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   55,    245,    76,   0, -1},    // Words Attack (PAL).bin
    {"e62e60a3e6cb5563f72982fcd83de25a",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  44,    230,    60,   0,  0},    // World End (unknown).bin
    {"745820765bd159c5bf5222fb859769d8",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  44,    230,    60,   0,  0},    // World End / Laser Blaster (BitCorp).bin
    {"87f020daa98d0132e98e43db7d8fea7e",  "WORMW1", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Worm War I (1982).bin
    {"007d18dedc1f0565f09c42aa61a6f585",  "WORMW1", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Worm War I (1982).bin
    {"fb531febf8e155328ec0cd39ef77a122",  "WORMW1", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   55,    245,   100,   0, -9},    // Worm War I (1982) (PAL).bin
    {"52b448757081fd9fabf859f4e2f91f6b",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   55,    245,   100,   0, -9},    // Worm War I (1982) (PAL).bin
    {"2244a5ff3ffb288da513dd66b8bee10e",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Wumpus Hunt 2600 (2006) (Bob Montgomery).bin
    {"5961d259115e99c30b64fe7058256bcf",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    91,   0,  0},    // X-Man (1983).bin
    {"5e201d6bfc520424a28f129ee5e56835",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   54,    245,    91,   0,  0},    // X-Man (1983) (PAL).bin
    {"eaf744185d5e8def899950ba7c6e7bb5",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Xenophobe (1990).bin
    {"f02ba8b5292bf3017d10553c9b7b2861",  "??????", BANK_F6,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   62,    245,   100,   0,  0},    // Xenophobe (1991) (PAL).bin
    {"c6688781f4ab844852f4e3352772289b",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    97,   0,  2},    // Xevious (1983).bin
    {"24385ba7f5109fbe76aadc0a375de573",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    97,   0,  2},    // Xevious (1983).bin
    {"af6f3e9718bccfcd8afb421f96561a34",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    97,   0,  2},    // Xevious (prototype).bin
    {"4cd29f2c0edfe0f74f42272b39a9620a",  "??????", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    90,   0,  0},    // X'Mission (1983) (NTSC by TJ).bin
    {"ca50cc4b21b0155255e066fcd6396331",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // X'Mission (1983) (PAL).bin
    {"ca50cc4b21b0155255e066fcd6396331",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // X'Mission (1983) (PAL).bin
    {"332f01fd18e99c6584f61aa45ee7791e",  "??????", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // X'Mission (1983) (PAL).bin
    {"c5930d0e8cdae3e037349bfa08e871be",  "YARSRE", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   0,  0},    // Yars' Revenge (1982).bin
    {"ee8027d554d14c8d0b86f94737d2fdcc",  "YARSRE", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   0,  0},    // Yars' Revenge (1982).bin
    {"8e7241bfc8380aac3c0ef1b6881cdded",  "YARSRE", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   0,  0},    // Yars' Revenge (Time Freeze) (09-01-81) (Atari) (CX2655 - 49-75167) (Prototype).bin
    {"2f7772879a1ed04f660aa9d77a86a4bd",  "YARSRE", BANK_4K,   CTR_GENESIS,   SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  34,    196,   100,   0,  0},    // Yars' Revenge (2 Button Hack).bin
    {"e91d2ecf8803ae52b55bbf105af04d4b",  "YARSRE", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,   100,   0,  0},    // Yars' Revenge (1982) (PAL).bin
    {"c1e6e4e7ef5f146388a090f1c469a2fa",  "Z-TACK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    90,   0,  0},    // Z-Tack (AKA Base Attack) (1983) (Bomb - Onbase).bin
    {"c469151655e333793472777052013f4f",  "Z-TACK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  NTSC,  32,    210,    90,   0,  0},    // Z-Tack (AKA Base Attack) (1983) (Bomb - Onbase).bin
    {"d6dc9b4508da407e2437bfa4de53d1b2",  "Z-TACK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,    90,   0, 19},    // Z-Tack (AKA Base Attack) (1983) (Bomb - Onbase) (PAL).bin
    {"6c91ac51421cb9fc72c9833c4f440d65",  "Z-TACK", BANK_4K,   CTR_LJOY,      SPEC_NONE,      MODE_FF,    VB,   HB,  ANA1_0,  PAL,   52,    245,    90,   0, 19},    // Z-Tack (AKA Base Attack) (1983) (Bomb - Onbase) (PAL).bin
    {"eea0da9b987d661264cce69a7c13c3bd",  "ZAXXON", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    92,   0,  8},    // Zaxxon (1982).bin
    {"25bb080457351be724aac8a02021aa92",  "ZAXXON", BANK_F8,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   57,    245,    92,   0,  3},    // Zaxxon (1982) (PAL).bin
    {"66caf2dc4a9ea1a4534b2ea169909cbd",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Zirconium 2005-10-28 (Fred Quimby).bin
    {"9168f9f4881eff94cc2f657e3539e2d4",  "??????", BANK_CDFJ, CTR_LJOY,      SPEC_DPCPNOC,   MODE_NO,   !VB,   HB,  ANA1_0,  NTSC,  34,    205,   100,   0,  2},    // zeviouz00_NTSC.bin
    {"65a6f1255fe22468a8bf84ff28a4d289",  "??????", BANK_TV,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    220,   100,   1, 30},    // TV Boy
    {"1b5a8da0622bffcee4c5b42aed4e0ef0",  "??????", BANK_TV,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    220,   100,   1, 30},    // TV Boy2
    {"f7ec2f2bdbe8fbea048c0d5fa6503b0b",  "??????", BANK_TV,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  PAL,   52,    230,   100,   1, 17},    // TV Boy (PAL)

    {"494c0fb944d8d0d6b13c6b4b50ccbd11",  "ZAXXON", BANK_WF8,  CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,    92,   0,  8},   // Zaxxon (1982).bin (write F8 banking)
    {"7b0ebb6bc1d700927f6efe34bac2ecd2",  "??????", BANK_WF8,  CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0, 10},   // Smurf - Rescue in Gargamel's Castle (1982).bin (write F8 banking)
    {"468dc062a58499d081c59cf5fd08f655",  "??????", BANK_JANE, CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  33,    210,   100,   0,  0},   // Tarzan - 16K Prototype with unique banking

    {"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Snake Oil
    {"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Snake Oil
    {"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Snake Oil
    {"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Snake Oil
    {"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",  "??????", BANK_2K,   CTR_LJOY,      SPEC_NONE,      MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0},    // Snake Oil

    {"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",  "??????", BANK_2K,   CTR_LJOY,      99,             MODE_NO,    VB,   HB,  ANA1_0,  NTSC,  34,    210,   100,   0,  0}     // End of list...
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Cartridge* Cartridge::create(const uInt8* image, uInt32 size)
{
  if (cart_buffer == NULL)   cart_buffer = (uInt8 *)rg_alloc(MAX_CART_FILE_SIZE, MEM_SLOW);
  if (xl_ram_buffer == NULL) xl_ram_buffer = (uInt8 *)rg_alloc(32768, MEM_SLOW);
  if (fast_cart_buffer == NULL) fast_cart_buffer = (uInt8 *)rg_alloc(8 * 1024, MEM_SLOW);

  Cartridge* cartridge = 0;

   uInt8 banking = autodetectType(image, size);

  // We should know the cart's type by now so let's create it
  if (banking == BANK_2K)
    cartridge = new Cartridge2K(image);
  else if (banking == BANK_3EPLUS)
    cartridge = new Cartridge3EPlus(image, size);
  else if (banking == BANK_3E)
    cartridge = new Cartridge3E(image, size);
  else if (banking == BANK_3F)
    cartridge = new Cartridge3F(image, size);
  else if (banking == BANK_4K)
    cartridge = new Cartridge4K(image);
  else if (banking == BANK_AR)
    cartridge = new CartridgeAR(image, size);
  else if (banking == BANK_DPC)
    cartridge = new CartridgeDPC(image, size);
  else if (banking == BANK_E0)
    cartridge = new CartridgeE0(image);
  else if (banking == BANK_E7)
    cartridge = new CartridgeE7(image, size);
  else if (banking == BANK_F4)
    cartridge = new CartridgeF4(image);
  else if (banking == BANK_F4SC)
    cartridge = new CartridgeF4SC(image);
  else if (banking == BANK_F6)
    cartridge = new CartridgeF6(image);
  else if (banking == BANK_F6SC)
    cartridge = new CartridgeF6SC(image);
  else if (banking == BANK_F8)
    cartridge = new CartridgeF8(image);
  else if (banking == BANK_WF8)
    cartridge = new CartridgeWF8(image);
  else if (banking == BANK_F8SC)
    cartridge = new CartridgeF8SC(image);
  else if (banking == BANK_JANE)
    cartridge = new CartridgeJANE(image);
  else if (banking == BANK_FASC)
    cartridge = new CartridgeFASC(image);
  else if (banking == BANK_FE)
    cartridge = new CartridgeFE(image);
  else if (banking == BANK_MB)
    cartridge = new CartridgeMB(image);
  else if (banking == BANK_CV)
    cartridge = new CartridgeCV(image, size);
  else if (banking == BANK_UA)
    cartridge = new CartridgeUA(image, 0);
  else if (banking == BANK_UASW)
    cartridge = new CartridgeUA(image, 1);
  else if (banking == BANK_WD)
    cartridge = new CartridgeWD(image, size);
  else if (banking == BANK_EF)
    cartridge = new CartridgeEF(image);
  else if (banking == BANK_EFSC)
    cartridge = new CartridgeEFSC(image);
  else if (banking == BANK_BF)
    cartridge = new CartridgeBF(image);
  else if (banking == BANK_BFSC)
    cartridge = new CartridgeBFSC(image);
  else if (banking == BANK_DF)
    cartridge = new CartridgeDF(image);
  else if (banking == BANK_DFSC)
    cartridge = new CartridgeDFSC(image);
  else if (banking == BANK_SB)
    cartridge = new CartridgeSB(image, size);
  else if (banking == BANK_FA2)
    cartridge = new CartridgeFA2(image, size);
  else if (banking == BANK_TV)
    cartridge = new CartridgeTV(image);
  else if (banking == BANK_0840)
    cartridge = new Cartridge0840(image);
  else if (banking == BANK_03E0)
    cartridge = new Cartridge03E0(image);
  else if (banking == BANK_0FA0)
    cartridge = new Cartridge0FA0(image);
  else if (banking == BANK_X07)
    cartridge = new CartridgeX07(image);
  else if (banking == BANK_CTY)
    cartridge = new CartridgeCTY(image, size);
  else if (banking == BANK_DPCP)
      cartridge = new CartridgeDPCPlus(image, size);
  else if (banking == BANK_CDFJ)
      cartridge = new CartridgeCDF(image, size);
  else
  {
      cartridge = new Cartridge4K(image); // It's gonna fail anyway...
      dsWarnIncompatibileCart();
  }

  return cartridge;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Cartridge::Cartridge()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Cartridge::~Cartridge()
{
}

void SetOtherDatabaseFieldDefaults(void)
{
  myCartInfo.soundQuality = myGlobalCartInfo.sound;
  myCartInfo.palette_type = myGlobalCartInfo.palette;

  // A few of the DPC+ games need reduced sound to be playable (11025 Hz)
  if (strcmp(myCartInfo.gameID, "SPAROC") == 0)   myCartInfo.soundQuality = SOUND_10KHZ;
  if (strcmp(myCartInfo.gameID, "SCRAMB") == 0)   myCartInfo.soundQuality = SOUND_10KHZ;
  if (strcmp(myCartInfo.gameID, "FROST2") == 0)   myCartInfo.soundQuality = SOUND_WAVE; // Because this game already has SPEC_DPCPOPT we can't also set SPEC_WAVEDIRC
  if (myCartInfo.special == SPEC_WAVEDIRC)        myCartInfo.soundQuality = SOUND_WAVE;
  
  myCartInfo.thumbOptimize = 0;
  if (myCartInfo.special == SPEC_DPCPOPT) myCartInfo.thumbOptimize = 1;
  if (myCartInfo.special == SPEC_DPCPNOC) myCartInfo.thumbOptimize = 2;

  myCartInfo.aButton = BUTTON_FIRE;
  myCartInfo.bButton = BUTTON_FIRE;
  myCartInfo.xButton = BUTTON_FIRE;
  myCartInfo.yButton = BUTTON_FIRE;

  myCartInfo.bus_driver = (isDSiMode() ? 1:0);
  myCartInfo.clearRAM = (myCartInfo.special == SPEC_AR) ? 1:0;  // Supercharger AR games generally want RAM CLEAR
  myCartInfo.xStretch = 0;
  myCartInfo.spare5_0 = 0;
  myCartInfo.spare6_0 = 0;
  myCartInfo.spare7_0 = 0;
  myCartInfo.spare8_0 = 0;
  myCartInfo.spare1_1 = 1;
  myCartInfo.spare2_1 = 1;
  myCartInfo.spare3_1 = 1;
  myCartInfo.spare1_FF = 0xFF;
  myCartInfo.spare2_FF = 0xFF;
  myCartInfo.spare3_FF = 0xFF;

  if (strcmp(myCartInfo.gameID, "ELFADV") == 0) myCartInfo.clearRAM = 1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static char rom_md5[33];
uInt8 Cartridge::autodetectType(const uInt8* image, uInt32 size)
{
  uInt8 bFound = false;

  // Get the MD5 message-digest for the ROM image
  strcpy(rom_md5, MD5(image, size).c_str());

  // Defaults for the selected cart... this may change up below...
  if (tv_type_requested == PAL)
      myCartInfo = table[1];
  else
      myCartInfo = table[0];

  // These are the fields not set directly in the Database Entry
  SetOtherDatabaseFieldDefaults();

  // -----------------------------------------------------------------------
  // Take a closer look at the ROM image and try to figure out its type
  // -----------------------------------------------------------------------
  bFoundInDAT = false;

  // Try finding it in the external configuration database...
  for (int idx = 0; idx < MAX_CONFIGS; idx++)
  {
    if (strcmp(allConfigs.cart[idx].md5, rom_md5) == 0)   // String compare...
    {
        myCartInfo = allConfigs.cart[idx];
        bFound = true;
        bFoundInDAT = true;
        break;
    }
  }

  if (!bFound)   // Try finding it in the internal database...
  {
      // First we'll see if its type is listed in the table above
      for(const CartInfo* entry = table; (entry->special != 99); ++entry)
      {
        if (strcmp(entry->md5, rom_md5) == 0)   // String compare...
        {
            myCartInfo = *entry;

            // These are the fields not set directly in the Database Entry
            SetOtherDatabaseFieldDefaults();

            bFound = true;
            break;
        }
      }
  }
  else  // Even if the entry was found in the config file, we search the internal database for the gameID and special entries
  {
      // First we'll see if its type is listed in the table above
      for(const CartInfo* entry = table; (entry->special != 99); ++entry)
      {
        if (strcmp(entry->md5, rom_md5) == 0)   // String compare...
        {
            // Since these two can change and aren't configurable, always get the latest from the internal database...
            strcpy(myCartInfo.gameID, entry->gameID);
            myCartInfo.special = entry->special;
            break;
        }
      }
  }

  if (!isDSiMode()) // For older DS/DS-LITE, we turn off Flicker Free by default... except for some popular games that can handle it!
  {
      if (!bFoundInDAT)
      {
          myCartInfo.frame_mode = MODE_NO;

          if (strcmp(myCartInfo.gameID, "ASTERD") == 0) myCartInfo.frame_mode = MODE_FF;
          if (strcmp(myCartInfo.gameID, "CENTIP") == 0) myCartInfo.frame_mode = MODE_FF;
          if (strcmp(myCartInfo.gameID, "DEMONA") == 0) myCartInfo.frame_mode = MODE_FF;
          if (strcmp(myCartInfo.gameID, "DEFEND") == 0) myCartInfo.frame_mode = MODE_FF;
          if (strcmp(myCartInfo.gameID, "GORFxx") == 0) myCartInfo.frame_mode = MODE_FF;
          if (strcmp(myCartInfo.gameID, "JUNOST") == 0) myCartInfo.frame_mode = MODE_FF;
          if (strcmp(myCartInfo.gameID, "MIPEDE") == 0) myCartInfo.frame_mode = MODE_FF;
          if (strcmp(myCartInfo.gameID, "MISCOM") == 0) myCartInfo.frame_mode = MODE_BACKG;
          if (strcmp(myCartInfo.gameID, "STARTR") == 0) myCartInfo.frame_mode = MODE_FF;
          if (strcmp(myCartInfo.gameID, "YARSRE") == 0) myCartInfo.frame_mode = MODE_FF;
          if (strcmp(myCartInfo.gameID, "PACMAN") == 0) myCartInfo.frame_mode = MODE_FF;
          if (strcmp(myCartInfo.gameID, "ATLANT") == 0) myCartInfo.frame_mode = MODE_FF;
          if (strcmp(myCartInfo.gameID, "SPAINV") == 0) myCartInfo.frame_mode = MODE_FF;
          if (strcmp(myCartInfo.gameID, "SPIDFI") == 0) myCartInfo.frame_mode = MODE_FF;
          if (strcmp(myCartInfo.gameID, "WIZWOR") == 0) myCartInfo.frame_mode = MODE_FF;
          if (strcmp(myCartInfo.gameID, "STELTR") == 0) myCartInfo.frame_mode = MODE_FF;
          if (strcmp(myCartInfo.gameID, "CARNIV") == 0) myCartInfo.frame_mode = MODE_FF;
          if (strcmp(myCartInfo.gameID, "HAUNTH") == 0) myCartInfo.frame_mode = MODE_FF;
          if (strcmp(myCartInfo.gameID, "GLAPAT") == 0) myCartInfo.frame_mode = MODE_FF;
          if (strcmp(myCartInfo.gameID, "QUARUN") == 0) myCartInfo.frame_mode = MODE_FF;
          if (strcmp(myCartInfo.gameID, "MELTDN") == 0) myCartInfo.frame_mode = MODE_FF;
          if (strcmp(myCartInfo.gameID, "FROGGR") == 0) myCartInfo.frame_mode = MODE_BLACK;
      }
  }
  
  // Meltdown won't look right in anything but Flicker-Free due to the hacked Player/Missile table in use
  if (myCartInfo.special == SPEC_MELTDOWN)
  {
      myCartInfo.frame_mode = MODE_FF;
  }

  // Patch Haunted House to avoid original bug (technically isn't needed anymore with improved emulation... but whatevs...)
  if (myCartInfo.special == SPEC_HAUNTED)
  {
      uInt8* imageOffset = (uInt8*)(image+1103);
      // Haunted House needs a fix to work... original programming bug.
      if (*imageOffset == 0xE5) *imageOffset = 0xE9;
  }

  // Patch Kool-Aid to avoid collision problems (this is easier than hacking the HMOVE timing)
  if (myCartInfo.special == SPEC_KOOLAID)
  {
      uInt8* imagePatch = (uInt8*)(image);
      imagePatch[0x5B] = 0xAC;
      imagePatch[0x5C] = 0x00;
      imagePatch[0x5D] = 0xF0;
      imagePatch[0x5E] = 0xEA;
      imagePatch[0x5F] = 0x88;
      imagePatch[0x60] = 0xD0;
      imagePatch[0x61] = 0xFC;
      imagePatch[0x62] = 0xA9;
      imagePatch[0x63] = 0x00;
      imagePatch[0x64] = 0x85;
      imagePatch[0x65] = 0x20;
      imagePatch[0x66] = 0xA9;
      imagePatch[0x67] = 0x10;
      imagePatch[0x68] = 0x85;
      imagePatch[0x69] = 0x21;
  }

  // If we didn't find the type in the table then guess it based on size
  if(!bFound)
  {
    strcpy(myCartInfo.md5, rom_md5);
    if((size % 8448) == 0)
    {
      myCartInfo.banking = BANK_AR;
      myCartInfo.special= SPEC_AR;
    }
    else if((size <= 2048) || (size == 4096 && memcmp(image, image + 2048, 2048) == 0))
    {
      myCartInfo.banking = BANK_2K;
    }
    else if(size == 4096)
    {
      myCartInfo.banking = BANK_4K;
    }
    else if(size == 8192)  // 8K
    {
      if(isProbablySC(image, size))
        myCartInfo.banking = BANK_F8SC;
      else if(memcmp(image, image + 4096, 4096) == 0)
        myCartInfo.banking = BANK_4K;
      else if(isProbablyE0(image, size))
        myCartInfo.banking = BANK_E0;
      else if(isProbably3F(image, size))
        myCartInfo.banking = isProbably3E(image, size) ? BANK_3E : BANK_3F;
      else if(isProbablyUA(image, size))
        myCartInfo.banking = BANK_UA;
      else if(isProbablyFE(image, size))
        myCartInfo.banking = BANK_FE;
      else if (isProbably0840(image, size))
          myCartInfo.banking = BANK_0840;
      else if (isProbably03E0(image, size))
          myCartInfo.banking = BANK_03E0;
      else if (isProbably0FA0(image, size))
          myCartInfo.banking = BANK_0FA0;
      else
        myCartInfo.banking = BANK_F8;
    }
    else if((size == 10495) || (size == 10496) || (size == 10240))  // 10K - Pitfall2
    {
      myCartInfo.banking = BANK_DPC;
    }
    else if(size == 12288)
    {
      // For now, we assume that all carts of 12K are CBS RAM Plus/FASC.
      myCartInfo.banking = BANK_FASC;
    }
    else if(size == 16384)  // 16K
    {
      if(isProbablySC(image, size))
        myCartInfo.banking = BANK_F6SC;
      else if(isProbablyE7(image, size))
        myCartInfo.banking = BANK_E7;
      else if(isProbably3EPlus(image, size))
        myCartInfo.banking = BANK_3EPLUS;
      else if(isProbably3F(image, size))
        myCartInfo.banking = isProbably3E(image, size) ? BANK_3E : BANK_3F;
      else
        myCartInfo.banking = BANK_F6;
    }
    else if(size == 28672) // 28K
    {
        myCartInfo.banking = BANK_FA2;  // 28K is probably FA2 (Star Castle)
    }
    else if(size == 32*1024)  // 32K
    {
      if(isProbablySC(image, size))
        myCartInfo.banking = BANK_F4SC;
      else if (isProbablyDPCplus(image, size))
        myCartInfo.banking = BANK_DPCP;
      else if (isProbablyCDF(image, size))
        myCartInfo.banking = BANK_CDFJ;
      else if(isProbably3EPlus(image, size))
        myCartInfo.banking = BANK_3EPLUS;
      else if(isProbably3F(image, size))
        myCartInfo.banking = isProbably3E(image, size) ? BANK_3E : BANK_3F;
      else if (isProbablyFA2(image, size))
        myCartInfo.banking = BANK_FA2;
      else
        myCartInfo.banking = BANK_F4;
    }
    else if (size == 64*1024) // 64K
    {
      if (isProbablyCDF(image, size))
        myCartInfo.banking = BANK_CDFJ;
      else if(isProbably3EPlus(image, size))
        myCartInfo.banking = BANK_3EPLUS;
      else if(isProbably3F(image, size))
        myCartInfo.banking = isProbably3E(image, size) ? BANK_3E : BANK_3F;
      else if (isProbablyEF(image, size))
          myCartInfo.banking = BANK_EF;
      else if (isProbablyEFSC(image, size))
          myCartInfo.banking = BANK_EFSC;
      else myCartInfo.banking = BANK_EF;        // Could be F0/MegaBoy but this is growing in popularity...
    }
    else if(size == (128*1024)) // 128K
    {
        if (isProbablyCDF(image, size)) myCartInfo.banking = BANK_CDFJ;
        else if (isProbablyDFSC(image, size)) myCartInfo.banking = BANK_DFSC;
        else myCartInfo.banking = ((isProbablyDF(image, size)) ? BANK_DF : BANK_SB);          // 128K games generally use either DFSC or, more commonly SuperBanking
    }
    else if(size == (256*1024)) // 256K
    {
        if (isProbablyCDF(image, size)) myCartInfo.banking = BANK_CDFJ;
        else if (isProbablyBF(image,size)) myCartInfo.banking = BANK_BF;
        else myCartInfo.banking = ((isProbablyBFSC(image,size)) ? BANK_BFSC : BANK_SB);       // 256K games are either BFSC or, more commonly SuperBanking
    }
    else if(size == (480*1024)) // 480K
    {
        myCartInfo.banking = BANK_3E;                                                       // Most games at 480K are 3E
    }
    else // 512K or some odd-size...
    {
      if (isProbablyCDF(image, size)) myCartInfo.banking = BANK_CDFJ;
      else if(isProbably3EPlus(image, size))
        myCartInfo.banking = BANK_3EPLUS;
      else if(isProbably3F(image, size))
        myCartInfo.banking = isProbably3E(image, size) ? BANK_3E : BANK_3F;                 // Many games > 256K are 3E or 3F
      else
        myCartInfo.banking = BANK_3F;  // Best guess...
    }
  }

  bElevatorAgent = false;
  bSaveStateXL = false;
  memset(xl_ram_buffer, 0x00, 32768);

  // ----------------------------------------------------------------
  // For Starpath Supercharger games, we must track distinct memory
  // access. This takes time so we don't do it for other game types...
  // ----------------------------------------------------------------
  if (myCartInfo.special == SPEC_AR)
  {
      cartDriver = 5;   // AR carts must use the special driver

      if ((strcmp(myCartInfo.gameID, "MINDMA") == 0))
      {
          myCartInfo.soundQuality = SOUND_15KHZ;
      }
  }
  else if (myCartInfo.banking == BANK_DPC)
  {
      cartDriver = 12;  // DPC carts must use the special driver
  }
  else if (myCartInfo.banking == BANK_CTY)
  {
      cartDriver = 13;  // CTY carts can use the special driver
  }
  else if (myCartInfo.banking == BANK_DPCP)
  {
      isCDFJPlus = false;
      cartDriver = 8;   // DPC+ carts must use the special driver

      if (!bFoundInDAT)
      {
          if (strstr(my_filename, "scramble") != 0)
          {
              myCartInfo.hBlankZero = 0;
              myCartInfo.vblankZero = 0;
              myCartInfo.displayStartScanline = 30;
              myCartInfo.displayNumScalines = 212;
              myCartInfo.yOffset = 18;
              myCartInfo.xButton = BUTTON_SHIFT_UP;
              myCartInfo.special = SPEC_DPCPNOC;
          }
      }
  }
  else if (myCartInfo.banking == BANK_CDFJ)
  {
      isCDFJPlus = (searchForBytes5(image, size, 'P', 'L', 'U', 'S', 'C') ? true:false);

      cartDriver = (isCDFJPlus ? 10:9); // The isCDFJPlus flag is set in isProbablyCDF()

      // ----------------------------------------------------------------------------------
      // These carts need a little extra oomph from the CPU so we have a special optmized
      // driver that requires the Atari 6502 native code to be no more than 2 banks (8K).
      // ----------------------------------------------------------------------------------
      if (strstr(my_filename, "turbo") != 0)     cartDriver = 11;  //CDFJ++
      if (strstr(my_filename, "elevator") != 0) {cartDriver = 11; bElevatorAgent = true;} //CDFJ++
      if (strstr(my_filename, "gorf") != 0)      cartDriver = 11;  //CDFJ++
      if (strstr(my_filename, "tutankham") != 0) cartDriver = 11;  //CDFJ++
      if (strstr(my_filename, "spiders") != 0)   cartDriver = 11;  //CDFJ++

      // For the CDF/CDFJ banking we need all the power we can get... turn on a reasonable level of optmization and minimal sound
      if (!bFoundInDAT)
      {
          if (myCartInfo.thumbOptimize < 2) myCartInfo.thumbOptimize = 2;
          if (myCartInfo.soundQuality != SOUND_WAVE)  myCartInfo.soundQuality = SOUND_10KHZ;

          // -----------------------------------------------------------------------
          // The special CHAMP GAMES handler ... Since these ROMs are unique to
          // a buyer, we will just handle them based on filename. This allows us
          // to set some sensible defaults for these cutting-edge games...
          // -----------------------------------------------------------------------
          if (strstr(my_filename, "mappy") != 0)
          {
              myCartInfo.soundQuality = SOUND_WAVE;
              myCartInfo.frame_mode = MODE_FF;
              myCartInfo.hBlankZero = 0;
              myCartInfo.displayStartScanline = 30;
              myCartInfo.yOffset = 14;
              myCartInfo.xButton = BUTTON_SHIFT_UP;
          }
          else
          if (strstr(my_filename, "qyx") != 0)
          {
              myCartInfo.soundQuality = SOUND_15KHZ;
              myCartInfo.frame_mode = MODE_FF;
              myCartInfo.displayStartScanline = 28;
              myCartInfo.yOffset = 16;
              myCartInfo.xButton = BUTTON_SHIFT_UP;
          }
          else
          if (strstr(my_filename, "lady") != 0)
          {
              myCartInfo.hBlankZero = 0;
              myCartInfo.vblankZero = 0;
              myCartInfo.yOffset = 15;
              myCartInfo.xButton = BUTTON_SHIFT_UP;
          }
          else
          if (strstr(my_filename, "zoo") != 0)
          {
              myCartInfo.frame_mode = MODE_FF;
              myCartInfo.hBlankZero = 0;
              myCartInfo.vblankZero = 0;
              myCartInfo.displayStartScanline = 32;
              myCartInfo.displayNumScalines = 210;
              myCartInfo.yOffset = 19;
              myCartInfo.xButton = BUTTON_SHIFT_UP;
          }
          else
          if (strstr(my_filename, "gorf") != 0)
          {
              myCartInfo.hBlankZero = 0;
              myCartInfo.vblankZero = 0;
              myCartInfo.displayStartScanline = 32;
              myCartInfo.displayNumScalines = 211;
              myCartInfo.yOffset = 17;
              myCartInfo.xButton = BUTTON_SHIFT_UP;
          }
          else
          if (strstr(my_filename, "cobra") != 0)
          {
              myCartInfo.frame_mode = MODE_FF;
              myCartInfo.hBlankZero = 0;
              myCartInfo.vblankZero = 0;
              myCartInfo.displayStartScanline = 30;
              myCartInfo.displayNumScalines = 212;
              myCartInfo.yOffset = 18;
              myCartInfo.xButton = BUTTON_SHIFT_UP;
          }
          else
          if (strstr(my_filename, "galagon") != 0)
          {
              myCartInfo.hBlankZero = 0;
              myCartInfo.vblankZero = 0;
              myCartInfo.yOffset = 12;
              myCartInfo.xButton = BUTTON_SHIFT_UP;
          }
          else
          if (strstr(my_filename, "robot") != 0)
          {
              myCartInfo.frame_mode = MODE_FF;
              myCartInfo.controllerType = CTR_QUADTARI;
              myCartInfo.hBlankZero = 0;
              myCartInfo.vblankZero = 0;
              myCartInfo.displayStartScanline = 28;
              myCartInfo.displayNumScalines = 205;
              myCartInfo.yScale = 87;
          }
          else
          if (strstr(my_filename, "wizard") != 0)
          {
              myCartInfo.frame_mode = MODE_FF;
              myCartInfo.soundQuality = SOUND_15KHZ;
              myCartInfo.hBlankZero = 0;
              myCartInfo.vblankZero = 0;
              myCartInfo.yOffset = -9;
              myCartInfo.xButton = BUTTON_SHIFT_DN;
          }
          else
          if (strstr(my_filename, "turbo") != 0)
          {
              myCartInfo.hBlankZero = 0;
              myCartInfo.vblankZero = 0;
              myCartInfo.yOffset = 10;
              myCartInfo.displayNumScalines = 206;
              myCartInfo.xButton = BUTTON_SHIFT_UP;
          }
          else
          if (strstr(my_filename, "elevator") != 0)
          {
              myCartInfo.thumbOptimize = 3; // This one needs frame skip
              myCartInfo.controllerType = CTR_GENESIS;
              myCartInfo.hBlankZero = 0;
              myCartInfo.vblankZero = 0;
              myCartInfo.displayStartScanline = 34;
              myCartInfo.displayNumScalines = 208;
              myCartInfo.yOffset = 14;
              myCartInfo.xButton = BUTTON_SHIFT_UP;
          }
          else
          if (strstr(my_filename, "tutankham") != 0)
          {
              myCartInfo.controllerType = CTR_GENESIS;
              myCartInfo.vblankZero = 0;
              myCartInfo.displayStartScanline = 33;
              myCartInfo.displayNumScalines = 208;
              myCartInfo.yOffset = 14;
              myCartInfo.xButton = BUTTON_SHIFT_UP;
          }
          else
          if (strstr(my_filename, "spiders") != 0)
          {
              myCartInfo.vblankZero = 0;
              myCartInfo.displayStartScanline = 33;
              myCartInfo.displayNumScalines = 210;
              myCartInfo.yOffset = 15;
              myCartInfo.xButton = BUTTON_SHIFT_UP;
          }
          else
          if (strstr(my_filename, "frantic") != 0)
          {
              myCartInfo.hBlankZero = 0;
              myCartInfo.vblankZero = 0;
              myCartInfo.displayStartScanline = 34;
              myCartInfo.displayNumScalines = 210;
              myCartInfo.yOffset = -1;
              myCartInfo.xButton = BUTTON_SHIFT_DN;
          }
          else
          if (strstr(my_filename, "ruby") != 0)
          {
              myCartInfo.frame_mode = MODE_FF;
              myCartInfo.hBlankZero = 1;
              myCartInfo.vblankZero = 0;
              myCartInfo.displayStartScanline = 30;
              myCartInfo.displayNumScalines = 205;
              myCartInfo.yScale = 94;
          }
          else
          if (strstr(my_filename, "boom") != 0)
          {
              myCartInfo.soundQuality = SOUND_WAVE;
              myCartInfo.hBlankZero = 0;
              myCartInfo.vblankZero = 0;
              myCartInfo.displayNumScalines = 205;
          }
          else
          if (strstr(my_filename, "draconian") != 0)
          {
              myCartInfo.thumbOptimize = 3; // This one needs frame skip
              myCartInfo.soundQuality = SOUND_WAVE;
              myCartInfo.hBlankZero = 1;
              myCartInfo.vblankZero = 0;
              myCartInfo.displayStartScanline = 34;
              myCartInfo.displayNumScalines = 204;
          }
      }

  }
  else if (((myCartInfo.banking == BANK_4K) || (myCartInfo.banking == BANK_2K)))
  {
      cartDriver = 1;   // Assume we can use the optmized driver until proven otherwise

      if (strcmp(myCartInfo.gameID, "SPACX7") == 0) cartDriver = 0;  // Spacemaster X-7 tries to write ROM... can't use the faster driver
      if (strcmp(myCartInfo.gameID, "SPPLUS") == 0) cartDriver = 0;  // SP+ requires the normal driver
      if (strcmp(myCartInfo.gameID, "MELTDN") == 0) cartDriver = 0;  // Meltdown requires the normal driver
      if (strcmp(myCartInfo.gameID, "FLASHG") == 0) cartDriver = 0;  // Flash Gordon requires the normal driver
      if (strcmp(myCartInfo.gameID, "STAGUN") == 0) cartDriver = 0;  // Star Gunner requires the normal driver
      if (strcmp(myCartInfo.gameID, "WARLOR") == 0) cartDriver = 0;  // Worlords requires the normal driver
      if (strcmp(myCartInfo.gameID, "WORMW1") == 0) cartDriver = 0;  // Worm War I requires the normal driver
      if (strcmp(myCartInfo.gameID, "A-STAR") == 0) cartDriver = 0;  // A-Star requires the normal driver
      if (strcmp(myCartInfo.gameID, "HUGOHU") == 0) cartDriver = 0;  // Hugo Hunt requires the normal driver
      if (strcmp(myCartInfo.gameID, "OYSTRO") == 0) cartDriver = 0;  // Oystron requires the normal driver
      if (strcmp(myCartInfo.gameID, "ALIEN0") == 0) cartDriver = 0;  // Alien requires the normal driver
      if (strcmp(myCartInfo.gameID, "BERZRK") == 0) cartDriver = 0;  // Berzerk requires the normal driver

      if (myCartInfo.bus_driver) cartDriver = 0;    // If the user has chosen the 'accurate' driver force it to be used...
  }
  else if (myCartInfo.banking == BANK_F8)
  {
      cartDriver = 0; // Assume we must use the non-optmized driver but switch to the highly optmized driver for some games...

      // ---------------------------------------------------------------------------------
      // For a handful of F8 games, we can turn on special speed-hack banking handling...
      // ---------------------------------------------------------------------------------
      if (strcmp(myCartInfo.gameID, "AQUAVE") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "ASTERD") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "ASTRIX") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "BAZONE") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "BEAMRI") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "BLUEPR") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "BUCKRO") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "CABAGE") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "CENTIP") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "CHUCKN") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "CIRCAT") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "COLONY") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "FALLDN") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "FATHOM") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "FRONTL") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "GALAXY") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "GHOSTB") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "GOFISH") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "GRAVIT") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "GYVOLV") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "HEROAV") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "ROTAN8") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "IXION_") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "JOUST_") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "JUNGLE") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "LEAD01") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "MARIOB") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "MOONPA") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "MOONSW") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "MSPACM") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "PACM8K") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "PHOENX") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "PIGSIN") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "POLEPO") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "PRCOOK") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "STARTR") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "VANGRD") == 0) cartDriver = 2;
      if (strcmp(myCartInfo.gameID, "ZAXXON") == 0) cartDriver = 2;

      if ((strcmp(myCartInfo.gameID, "JUNGLE") == 0) || (strcmp(myCartInfo.gameID, "GALAXY") == 0) || (strcmp(myCartInfo.gameID, "BAZONE") == 0) || (strcmp(myCartInfo.gameID, "HEROAV") == 0))
      {
          if (!isDSiMode())
          {
              // Small speed-up
              myCartInfo.hBlankZero = 0;
              myCartInfo.vblankZero = 0;
          }
      }

      if (myCartInfo.bus_driver) cartDriver = 0;    // If the user has chosen the 'accurate' driver force it to be used...
  }
  else if ((myCartInfo.banking == BANK_F6))
  {
      // ------------------------------------------------------------------------------------
      // For F6 games we will utilize a special optmized F6 driver directly in M6502Low.cpp
      // ------------------------------------------------------------------------------------
      cartDriver = 3;

      if (!bFoundInDAT)
      {
          if (strstr(my_filename, "mars") != 0)
          {
              strcpy(myCartInfo.gameID, "CONMAR");
              myCartInfo.displayStartScanline = 34;
              myCartInfo.displayNumScalines = 210;
              myCartInfo.yOffset = 7;
          }

          if ((strstr(my_filename, "lady") != 0) && (strstr(my_filename, "bug") != 0))
          {
              strcpy(myCartInfo.gameID, "LADBUG");
              myCartInfo.frame_mode = isDSiMode() ? MODE_BLACK : MODE_NO;
              myCartInfo.displayStartScanline = 34;
              myCartInfo.displayNumScalines = 210;
              myCartInfo.yOffset = 10;
              myCartInfo.xButton = BUTTON_SHIFT_UP;
          }
      }

      // A few games just need a tiny bit more... ooomff!
      if ((strcmp(myCartInfo.gameID, "CONMAR") == 0) || (strcmp(myCartInfo.gameID, "MANGOS") == 0) || (strcmp(myCartInfo.gameID, "FROSTY") == 0))
      {
          if (!isDSiMode())
          {
              // Small speed-up
              myCartInfo.hBlankZero = 0;
              myCartInfo.vblankZero = 0;
          }
      }

      if (myCartInfo.bus_driver) cartDriver = 0;    // If the user has chosen the 'accurate' driver force it to be used...
  }
  else if ((myCartInfo.banking == BANK_F4))
  {
      // ------------------------------------------------------------------------------------
      // For F4 games we will utilize a special optmized F4 driver directly in M6502Low.cpp
      // ------------------------------------------------------------------------------------
      cartDriver = 0;

      if (strcmp(myCartInfo.gameID, "JUNOST") == 0) cartDriver = 4;
      if (strcmp(myCartInfo.gameID, "SLIDEB") == 0) cartDriver = 4;
      if (strcmp(myCartInfo.gameID, "NINJAG") == 0) cartDriver = 4;
      if (strcmp(myCartInfo.gameID, "SPGAME") == 0) cartDriver = 4;
      if (strcmp(myCartInfo.gameID, "DUNGEN") == 0) cartDriver = 4;
      if (strcmp(myCartInfo.gameID, "KOCRUZ") == 0) cartDriver = 4;
      if (strcmp(myCartInfo.gameID, "MAYHAM") == 0) cartDriver = 4;
      if (strcmp(myCartInfo.gameID, "PINATA") == 0) cartDriver = 4;
      if (strcmp(myCartInfo.gameID, "UPPLUS") == 0) cartDriver = 4;
      if (strcmp(myCartInfo.gameID, "PRIRES") == 0) cartDriver = 4;
      if (strcmp(myCartInfo.gameID, "CELERY") == 0) cartDriver = 4;

      // A few games just need a tiny bit more... ooomff!
      if ((strcmp(myCartInfo.gameID, "JUNOST") == 0) || (strcmp(myCartInfo.gameID, "SLIDEB") == 0) || (strcmp(myCartInfo.gameID, "NINJAG") == 0) ||
          (strcmp(myCartInfo.gameID, "SPGAME") == 0) || (strcmp(myCartInfo.gameID, "DUNGEN") == 0) || (strcmp(myCartInfo.gameID, "KOCRUZ") == 0) ||
          (strcmp(myCartInfo.gameID, "UPPLUS") == 0) || (strcmp(myCartInfo.gameID, "PRIRES") == 0))
      {
          if (!isDSiMode())
          {
              // Small speed-up
              myCartInfo.hBlankZero = 0;
              myCartInfo.vblankZero = 0;
          }
      }

      if (myCartInfo.bus_driver) cartDriver = 0;    // If the user has chosen the 'accurate' driver force it to be used...
  }
  else if ((myCartInfo.banking == BANK_F8SC))
  {
      cartDriver = 6;
      if (myCartInfo.bus_driver) cartDriver = 0;    // If the user has chosen the 'accurate' driver force it to be used...
  }
  else if ((myCartInfo.banking == BANK_F6SC))
  {
      cartDriver = 0;

      if (strcmp(myCartInfo.gameID, "DESFAL") == 0) cartDriver = 7;
      if (strcmp(myCartInfo.gameID, "DIGDUG") == 0) cartDriver = 7;
      if (strcmp(myCartInfo.gameID, "JRPACM") == 0) cartDriver = 7;
      if (strcmp(myCartInfo.gameID, "KLAX90") == 0) cartDriver = 7;
      if (strcmp(myCartInfo.gameID, "MIPEDE") == 0) cartDriver = 7;
      if (strcmp(myCartInfo.gameID, "RADARL") == 0) cartDriver = 7;
      if (strcmp(myCartInfo.gameID, "SAVMAR") == 0) cartDriver = 7;
      if (strcmp(myCartInfo.gameID, "SQUEST") == 0) cartDriver = 7;
      if (strcmp(myCartInfo.gameID, "SPRINT") == 0) cartDriver = 7;
      if (strcmp(myCartInfo.gameID, "SUPFBL") == 0) cartDriver = 7;

      // A few games just need a tiny bit more... ooomff!
      if ((strcmp(myCartInfo.gameID, "DIGDUG") == 0) || (strcmp(myCartInfo.gameID, "JRPACM") == 0) || (strcmp(myCartInfo.gameID, "MIPEDE") == 0) ||
          (strcmp(myCartInfo.gameID, "SPRINT") == 0))
      {
          if (!isDSiMode())
          {
              // Small speed-up
              myCartInfo.hBlankZero = 0;
              myCartInfo.vblankZero = 0;
          }
      }
      if (myCartInfo.bus_driver) cartDriver = 0;    // If the user has chosen the 'accurate' driver force it to be used...
  }
  else // Use the normal driver....
  {
      cartDriver = 0;
  }

  // --------------------------------------------------------------------------------------------------------
  // And a few games that struggle to keep up 60FPS even on the DSi so we use the optimized drivers always
  // --------------------------------------------------------------------------------------------------------
  if (strcmp(myCartInfo.gameID, "REALTE") == 0) cartDriver = 2;     // Real Sports Tennis is F8
  if (strcmp(myCartInfo.gameID, "MIDMAG") == 0) cartDriver = 3;     // Midnight Magic is F6

  // Conquest of Mars has a glitch unless the unused TIA pins are driven exactly so...
  if (strcmp(myCartInfo.gameID, "CONMAR") == 0) {cartDriver = 3; myDataBusState = 0x02;}

  original_banking_detect = myCartInfo.banking;     // In case the user wants to restore defaults - this brings back the original "guessed" banking scheme
  return myCartInfo.banking;
}

int Cartridge::searchForBytes(const uInt8* image, uInt32 size, uInt8 byte1, uInt8 byte2)
{
  uInt32 count = 0;
  for(uInt32 i = 0; i < size - 1; ++i)
  {
    if((image[i] == byte1) && (image[i + 1] == byte2))
    {
      ++count;
    }
  }

  return count;
}

int Cartridge::searchForBytes3(const uInt8* image, uInt32 size, uInt8 byte1, uInt8 byte2, uInt8 byte3)
{
  uInt32 count = 0;
  for(uInt32 i = 0; i < size - 2; ++i)
  {
    if((image[i] == byte1) && (image[i + 1] == byte2) && (image[i + 2] == byte3))
    {
      ++count;
    }
  }

  return count;
}

int Cartridge::searchForBytes4(const uInt8* image, uInt32 size, uInt8 byte1, uInt8 byte2, uInt8 byte3, uInt8 byte4)
{
  uInt32 count = 0;
  for(uInt32 i = 0; i < size - 3; ++i)
  {
    if((image[i] == byte1) && (image[i + 1] == byte2) && (image[i + 2] == byte3) && (image[i + 3] == byte4))
    {
      ++count;
    }
  }

  return count;
}

int Cartridge::searchForBytes5(const uInt8* image, uInt32 size, uInt8 byte1, uInt8 byte2, uInt8 byte3, uInt8 byte4, uInt8 byte5)
{
  uInt32 count = 0;
  for(uInt32 i = 0; i < size - 4; ++i)
  {
    if((image[i] == byte1) && (image[i + 1] == byte2) && (image[i + 2] == byte3) && (image[i + 3] == byte4) && (image[i + 4] == byte5))
    {
      ++count;
    }
  }

  return count;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbablyFE(const uInt8* image, uInt32 size)
{
  // FE bankswitching is very weird, but always seems to include a
  // 'JSR $xxxx'
  // These signatures are attributed to the MESS project
  uInt8 signature[4][5] = {
    { 0x20, 0x00, 0xD0, 0xC6, 0xC5 },  // JSR $D000; DEC $C5
    { 0x20, 0xC3, 0xF8, 0xA5, 0x82 },  // JSR $F8C3; LDA $82
    { 0xD0, 0xFB, 0x20, 0x73, 0xFE },  // BNE $FB; JSR $FE73
    { 0x20, 0x00, 0xF0, 0x84, 0xD6 }   // JSR $F000; STY $D6
  };
  for(uInt32 i = 0; i < 4; ++i)
    if(searchForBytes5(image, size, signature[i][0], signature[i][1], signature[i][2], signature[i][3], signature[i][4]))
      return true;

  return false;
}



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbablyUA(const uInt8* image, uInt32 size)
{
  // UA cart bankswitching switches to bank 1 by accessing address 0x240 using STA 0x240 or LDS 0x240
  uInt8 signature[7][3] = {
    { 0x8D, 0x40, 0x02 },  // STA $240 (Funky Fish, Pleiades)
    { 0xAD, 0x40, 0x02 },  // LDA $240 (???)
    { 0xBD, 0x1F, 0x02 },  // LDA $21F,X (Gingerbread Man)
    { 0x2C, 0xC0, 0x02 },  // BIT $2C0 (Time Pilot)
    { 0x8D, 0xC0, 0x02 },  // STA $2C0 (Fathom, Vanguard)
    { 0xAD, 0xC0, 0x02 },  // LDA $2C0 (Mickey)
    { 0x2C, 0xC0, 0x0F }   // BIT $FC0 (H.E.R.O., Kung-Fu Master)
  };

  for(uInt32 i = 0; i < 7; ++i)
  {
    if (searchForBytes3(image, size, signature[i][0], signature[i][1], signature[i][2]))
      return true;
  }
  return false;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbablySC(const uInt8* image, uInt32 size)
{
  // We assume a Superchip cart contains the same bytes for its entire
  // RAM area; obviously this test will fail if it doesn't
  // The RAM area will be the first 256 bytes of each 4K bank
  uInt32 banks = size / 4096;
  for(uInt32 i = 0; i < banks; ++i)
  {
    uInt8 first = image[i*4096];
    for(uInt32 j = 0; j < 256; ++j)
    {
      if(image[i*4096+j] != first)
        return false;
    }
  }
  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbably3F(const uInt8* image, uInt32 size)
{
  uInt32 count = 0;
  for(uInt32 i = 0; i < size - 1; ++i)
  {
    if((image[i] == 0x85) && (image[i + 1] == 0x3F))
    {
      ++count;
    }
  }

  return (count > 2);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbably3E(const uInt8* image, uInt32 size)
{
  return (searchForBytes(image, size, 0x85, 0x3E) > 2);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbably3EPlus(const uInt8* image, uInt32 size)
{
  return (searchForBytes4(image, size, 'T', 'J', '3', 'E'));    // Signature: $54 $4A $33 $45 (TJ3E)
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbablyE0(const uInt8* image, uInt32 size)
{
  // E0 cart bankswitching is triggered by accessing addresses
  // $FE0 to $FF9 using absolute non-indexed addressing
  // To eliminate false positives (and speed up processing), we
  // search for only certain known signatures
  // Thanks to "stella@casperkitty.com" for this advice
  // These signatures are attributed to the MESS project
  uInt8 signature[8][3] = {
    { 0x8D, 0xE0, 0x1F },  // STA $1FE0
    { 0x8D, 0xE0, 0x5F },  // STA $5FE0
    { 0x8D, 0xE9, 0xFF },  // STA $FFE9
    { 0x0C, 0xE0, 0x1F },  // NOP $1FE0
    { 0xAD, 0xE0, 0x1F },  // LDA $1FE0
    { 0xAD, 0xE9, 0xFF },  // LDA $FFE9
    { 0xAD, 0xED, 0xFF },  // LDA $FFED
    { 0xAD, 0xF3, 0xBF }   // LDA $BFF3
  };
  for(uInt32 i = 0; i < 8; ++i)
    if(searchForBytes3(image, size, signature[i][0], signature[i][1], signature[i][2]))
      return true;

  return false;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbablyE7(const uInt8* image, uInt32 size)
{
  // E7 cart bankswitching is triggered by accessing addresses
  // $FE0 to $FE6 using absolute non-indexed addressing
  // To eliminate false positives (and speed up processing), we
  // search for only certain known signatures
  // Thanks to "stella@casperkitty.com" for this advice
  // These signatures are attributed to the MESS project
  uInt8 signature[7][3] = {
    { 0xAD, 0xE2, 0xFF },  // LDA $FFE2
    { 0xAD, 0xE5, 0xFF },  // LDA $FFE5
    { 0xAD, 0xE5, 0x1F },  // LDA $1FE5
    { 0xAD, 0xE7, 0x1F },  // LDA $1FE7
    { 0x0C, 0xE7, 0x1F },  // NOP $1FE7
    { 0x8D, 0xE7, 0xFF },  // STA $FFE7
    { 0x8D, 0xE7, 0x1F }   // STA $1FE7
  };
  for(uInt32 i = 0; i < 7; ++i)
    if(searchForBytes3(image, size, signature[i][0], signature[i][1], signature[i][2]))
      return true;

  return false;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbably0840(const uInt8* image, uInt32 size)
{
  // 0840 cart bankswitching is triggered by accessing addresses 0x0800
  // or 0x0840 at least twice
  uInt8 signature1[3][3] = {
    { 0xAD, 0x00, 0x08 },  // LDA $0800
    { 0xAD, 0x40, 0x08 },  // LDA $0840
    { 0x2C, 0x00, 0x08 }   // BIT $0800
  };
  for(uInt32 i = 0; i < 3; ++i)
    if(searchForBytes3(image, size, signature1[i][0], signature1[i][1], signature1[i][2]))
      return true;

  uInt8 signature2[2][4] = {
    { 0x0C, 0x00, 0x08, 0x4C },  // NOP $0800; JMP ...
    { 0x0C, 0xFF, 0x0F, 0x4C }   // NOP $0FFF; JMP ...
  };
  for(uInt32 i = 0; i < 2; ++i)
    if(searchForBytes4(image, size, signature2[i][0], signature2[i][1], signature2[i][2], signature2[i][3]))
      return true;

  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbably03E0(const uInt8* image, uInt32 size)
{
  // 03E0 cart bankswitching for Brazilian Parker Bros ROMs, switches segment
  // 0 into bank 0 by accessing address 0x3E0 using 'LDA $3E0' or 'ORA $3E0'.
  uInt8 signature[2][4] = {
    { 0x0D, 0xE0, 0x03, 0x0D },  // ORA $3E0, ORA (Popeye)
    { 0xAD, 0xE0, 0x03, 0xAD }   // LDA $3E0, ORA (Montezuma's Revenge)
  };
  for(uInt32 i = 0; i < 2; ++i)
    if(searchForBytes4(image, size, signature[i][0], signature[i][1], signature[i][2], signature[i][3]))
      return true;

  return false;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbably0FA0(const uInt8* image, uInt32 size)
{
    // Other Brazilian (Fotomania) ROM's bankswitching switches to bank 1 by
    // accessing address 0xFC0 using 'BIT $FC0', 'BIT $FC0' or 'STA $FC0'
    // Also a game (Motocross) using 'BIT $EFC0' has been found
    static constexpr uInt8 signature[4][3] = {
      { 0x2C, 0xC0, 0x0F },  // BIT $FC0  (H.E.R.O., Kung-Fu Master)
      { 0x8D, 0xC0, 0x0F },  // STA $FC0  (Pole Position, Subterranea)
      { 0xAD, 0xC0, 0x0F },  // LDA $FC0  (Front Line, Zaxxon)
      { 0x2C, 0xC0, 0xEF }   // BIT $EFC0 (Motocross)
    };  for(uInt32 i = 0; i < 4; ++i)

    if(searchForBytes3(image, size, signature[i][0], signature[i][1], signature[i][2]))
        return true;
    return false;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbablyFA2(const uInt8* image, uInt32 size)
{
  // This currently tests only the 32K version of FA2; the 24 and 28K
  // versions are easy, in that they're the only possibility with those
  // file sizes

  // 32K version has all zeros in 29K-32K area
  for(size_t i = (29*1024); i < (32*1024); ++i)
    if(image[i] != 0)
      return false;

  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbablyDPCplus(const uInt8* image, uInt32 size)
{
  // DPC+ ARM code has 2 occurrences of the string DPC+
  // Note: all Harmony/Melody custom drivers also contain the value
  // 0x10adab1e (LOADABLE) if needed for future improvement
  return searchForBytes4(image, size, 'D', 'P', 'C', '+');
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbablyCDF(const uInt8* image, uInt32 size)
{
  isCDFJPlus = (searchForBytes5(image, size, 'P', 'L', 'U', 'S', 'C') ? true:false);

  // DPC+ ARM code has occurrences of the string CDF (or CDFJ)
  return searchForBytes3(image, size, 'C', 'D', 'F');
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbablyEF(const uInt8* image, uInt32 size)
{
  // EFSC carts have 'EFEF' in the binary
  return searchForBytes4(image, size, 'E', 'F', 'E', 'F');
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbablyEFSC(const uInt8* image, uInt32 size)
{
  // EFSC carts have 'EFSC' in the binary
  return searchForBytes4(image, size, 'E', 'F', 'S', 'C');
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbablyDF(const uInt8* image, uInt32 size)
{
  // DF carts have 'DFDF' in the binary
  return searchForBytes4(image, size, 'D', 'F', 'D', 'F');
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbablyDFSC(const uInt8* image, uInt32 size)
{
  // DFSC carts have 'DFSC' in the binary
  return searchForBytes4(image, size, 'D', 'F', 'S', 'C');
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbablyBF(const uInt8* image, uInt32 size)
{
  // BFSC carts have 'BFBF' in the binary
  return searchForBytes4(image, size, 'B', 'F', 'B', 'F');
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cartridge::isProbablyBFSC(const uInt8* image, uInt32 size)
{
  // BFSC carts have 'BFSC' in the binary
  return searchForBytes4(image, size, 'B', 'F', 'S', 'C');
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Cartridge::Cartridge(const Cartridge&)
{
  assert(false);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Cartridge& Cartridge::operator = (const Cartridge&)
{
  assert(false);
  return *this;
}


void CartSetDefaultFromInternalDatabase(void)
{
    bool bFound = false;
    char rom_md5[33];
    strcpy(rom_md5, myCartInfo.md5);
    myCartInfo = table[(tv_type_requested == PAL) ? 1:0];   // Default
    // First we'll see if its type is listed in the table above
    for(const CartInfo* entry = table; (entry->special != 99); ++entry)
    {
        if (strcmp(entry->md5, rom_md5) == 0)   // String compare...
        {
            myCartInfo = *entry;
            bFound = true;
            break;
        }
    }

    if (!bFound)
    {
        myCartInfo.banking = original_banking_detect;
    }

    if (!isDSiMode()) // For older DS/DS-LITE, we turn off Flicker Free by default... For a few games we will re-enable
    {
        myCartInfo.frame_mode = MODE_NO;
    }

    // -----------------------------------------------------------
    // These are always defaulted as such from internal database
    // -----------------------------------------------------------
    SetOtherDatabaseFieldDefaults();
}
