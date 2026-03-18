#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2022 NewRisingSun
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* BlazePro CPLD-based multicarts

   Unsolved issue: how is CHR RAM write-protection triggered?
    
   Known problems:
   
   Forever Duo of NES 852-in-1 (rev1):
   #A370 Time Lord: Hangs with glitchy status bar on NTSC and PAL but not Dendy
   #A133 Galactic Crusader: Wrong mirroring
   #A249 Mission Cobra: Wrong mirroring
   
   Legendary Games of NES 509-in-1:
   #189 Huang Di Battle of Zhuolu: Wrong mirroring during intro
   #227 Kid Niki Niki 2: Title screen animation flickers and looks strange
   #234 Klax: Screen in wrong position during options screen
   #365 Rocman X: Graphical garbage in waterfall (middle) level
   #403 Star Wars: Blank tiles due to lack of CHR RAM write-protection
   #404 The Empire Strikes Back: Blank tiles due to lack of CHR RAM write-protection
   #460 Twin Dragons: Wrong mirroring
   
   Unlicensed Collection 142-in-1:
   #59 Huang Di: Wrong mirroring during intro
   #84 Ms. Pac-Man: Wrong mirroring
   #102 Rocman X: Graphical garbage in waterfall (middle) level
   #132 Trolls on Treasure Island: Wrong mirroring during map
   
   Unreleased Collection 73-in-1 (v1.01):
   #38 Holy Diver: Wrong mirroring during first scene
*/

#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif
#include "mapinc.h"
#include "fceu-state.h"

static uint8 submapper;
static uint8 eeprom[16], eep_clock, state, command, output; /* Some strange serial EEPROM */
static uint8 *WRAM;
static uint32 WRAMSIZE;

static int prevSFEXINDEX;
extern int SFEXINDEX;
extern SFORMAT SFMDATA[64];

static uint8 mapper;      /* 5700 MSB >>4 OR'd with submapper <<4 */
static uint8 mapperFlags; /* 5700 LSB */
static uint8 misc;        /* 5601 */
static uint8 misc2;       /* 5702 */
static void (*sync)();

static uint16 prgOR;
static uint8  prgAND;

static uint8  regByte[16];
static int16  regWord[9];

#include "468_mmc1.h"
#include "468_mmc24.h"
#include "468_mmc3.h"
#include "468_vrc1.h"
#include "468_vrc24.h"
#include "468_vrc3.h"
#include "468_vrc6.h"
#include "468_vrc7.h"
#include "468_fme7.h"
#include "468_discrete.h"
#include "468_cnrom.h"
#include "468_if12.h"
#include "468_lf36.h"
#include "468_nanjing.h"

static SFORMAT stateRegs[] = {
	{ &mapper,          1, "SUP0" },
	{ &mapperFlags,     1, "SUP1" },
	{ &misc,            1, "SUP2" },
	{ &misc2,           1, "SUP3" },
	{ &prgOR,           2, "SUP4" },
	{ &prgAND,          1, "SUP5" },
	{  eeprom,          16,"EEPR" },
	{ &eep_clock,       1, "EEP0" },
	{ &state,           1, "EEP1" },
	{ &command,         1, "EEP2" },
	{ &output,          1, "EEP3" },	
	{ regByte,          16,"REGB" },
	{ regWord,          16,"REGW" },
	{ 0 }
};

static const uint16 lut509[512] ={ /* Strange look-up table, used only by Legendary Games of NES 509-in-1 */
   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,
  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,   0,   1,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,
  85,  86,  87,  88,  89,  90,   4,  91,  92,  93,  94,  95,  96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123,
 124, 125, 126, 127, 128, 129,   2,   3, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139,   5, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160,
 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200,
 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 238, 239, 240, 241,
 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 256,   6, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281,
 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321,
 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362,
 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401, 402,
 403, 404, 405, 406, 407, 408, 409, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 421, 422, 423, 424, 425, 426, 427, 428, 429, 430, 431, 432, 433, 434, 435, 436, 437, 438, 439, 440, 441, 442,
 443, 444, 445, 446, 447, 448, 449, 450, 451, 452, 453, 454, 455, 456, 457, 458, 459, 460, 461, 462, 463, 464, 465, 466, 467, 468, 469, 470, 471, 472, 473, 474, 475, 476, 477, 478, 479, 480, 481, 482,
 483, 484, 485, 486, 487, 488, 489, 490, 491, 492, 493, 494, 495, 496, 497, 498, 499, 500, 501, 502, 503, 504, 505, 506, 507, 508, 512, 513, 514, 515, 516, 517
};

void setPins(uint8 select, uint8 newClock, uint8 newData) { /* Serial EEPROM */
	if (select)
		state =0;
	else
	if (!eep_clock && !!newClock) {
		if (state <8) {
			command =command <<1 | !!(newData)*1;
			if (++state ==8 && (command &0xF0) !=0x50 && (command &0xF0) !=0xA0) state =0;
		} else {
			int mask =1 <<(15 -state);
			int address =command &0x0F;
			if ((command &0xF0) ==0xA0) {
				eeprom[address] =eeprom[address] &~mask | !!(newData)*mask;
				/* The "write" command also silently returns the content of a strange lookup table */
				output =!!(lut509[eeprom[0] | eeprom[1] | eeprom[2] <<8 &0x1FF] >>(address &1? 0: 8) &mask);
			} else
			if ((command &0xF0) ==0x50)
				output =!!(eeprom[address] &mask);
			
			if (++state ==16) state =0;
		}
	}
	eep_clock =newClock;
}

static DECLFR(readReg);
static DECLFW(writeReg);
static void setMapper(uint8 clearRegs) {
	int i;
	if (clearRegs) {
		for (i =0; i <16; i++) regByte[i] =0;
		for (i =0; i < 8; i++) regWord[i] =0;
		X6502_IRQEnd(FCEU_IQEXT);
	}
	SetReadHandler(0x5000, 0x5FFF, readReg);
	SetReadHandler(0x6000, 0xFFFF, CartBR);
	SetWriteHandler(0x5000, 0x5FFF, writeReg);
	SetWriteHandler(0x6000, 0xFFFF, CartBW);
	MapIRQHook = NULL;
	PPU_hook = NULL;
	GameHBIRQHook = NULL;
	setprg8r(0x10, 0x6000, 0);

	switch(mapper) { /* 5700 MSB >>4 OR'd with submapper <<4 */
	case 0x00: case 0x01: case 0x32:            MMC1_reset(clearRegs); break;
	case 0x0A:                                  MMC2_reset(clearRegs); break;
	case 0x10: case 0x11: case 0x12:            MMC3_reset(clearRegs); break;
	case 0x08:                                  MMC4_reset(clearRegs); break;
	case 0x40:                                  VRC1_reset(clearRegs); break;
	case 0x20: case 0x21: case 0x22: case 0x23: VRC24_reset(clearRegs); break;
	case 0x44:                                  VRC3_reset(clearRegs); break;
	case 0x30: case 0x31:                       VRC6_reset(clearRegs); break;
	case 0x41:                                  VRC7_reset(clearRegs); break;
	case 0x07:                                  LF36_reset(clearRegs); break;
	case 0x50:                                  FME7_reset(clearRegs); break;
	case 0x0E: case 0x1E:                       NANJING_reset(clearRegs); break;
	case 0x09: case 0x0B: case 0x17: case 0x37: UNROM_IF12_reset(clearRegs); break;
	case 0x04: case 0x06: case 0x14: case 0x16: ANROM_BNROM_reset(clearRegs); break;
	case 0x05: case 0x15:                       CNROM_BF9097_reset(clearRegs); break;
	case 0x0C: case 0x0D: case 0x1C: case 0x1D: GNROM_reset(clearRegs); break;
	default:                                    break;
	}
	sync();
}

static DECLFR(readReg) {
	switch(A) {
	case 0x5301: case 0x5601:
		return output? 0x80: 0x00;
	default:
		return 0xFF;
	}
}

static DECLFW(writeReg) {
	switch(A) {
	case 0x5301:
		if (submapper ==0) setPins(!!(V &0x04), !!(V &0x02), !!(V &0x01));
		break;
	case 0x5601:
		if (~misc &0x80) {
			misc =V;
			if (submapper !=1) {
				prgOR =prgOR &~0x2000 | V <<9 &0x2000;
				sync();
			}
		}
		if (submapper ==1) setPins(!!(V &0x10), !!(V &0x02), !!(V &0x01));
		break;
	case 0x5700:
		mapper =V >>4 | submapper <<4;
		mapperFlags =V &0xF;
		prgOR =prgOR &~0x0010 | V <<4 &0x0010;
		setMapper(1);
		break;
	case 0x5701:
		prgOR =prgOR &~0x1FE0 | V <<5 &0x1FE0;
		sync();
		break;
	case 0x5702:
		if (submapper ==1) {
			misc2 =V;
			prgOR =prgOR &~0x2000 | V <<9 &0x2000;
			setMapper(0); /* The misc2 value is required for prgAND by MMC3 and UNROM */
		}
		break;
	}
}

static void reset(void) {
	mapper =submapper <<4;
	mapperFlags =0x0F;
	misc =0;
	misc2 =0;
	prgOR =0x7FF0;
	eep_clock =command =output =1;
	command =state =0;
	setMapper(1);
}

static void power(void) {
	int i;
	for (i =0; i <16; i++) eeprom[i] =0;
	reset();
}

static void close(void) {
#ifndef FCEU_NO_MALLOC
	if (WRAM)
		FCEU_gfree(WRAM);
#endif
	WRAM = NULL;
}

static void stateRestore(int version) {
	setMapper(0);
}

void Mapper468_Init(CartInfo *info) {
	submapper =info->submapper;
	info->Reset =reset;
	info->Power =power;
	info->Close =close;
	GameStateRestore =stateRestore;

	WRAMSIZE =8192;
#ifndef FCEU_NO_MALLOC
	WRAM =(uint8*) FCEU_gmalloc(WRAMSIZE);
#else
	WRAM =(uint8*) ahb_calloc(1, WRAMSIZE);
#endif
	SetupCartPRGMapping(0x10, WRAM, WRAMSIZE, 1);
	AddExState(WRAM, WRAMSIZE, 0, "WRAM");
	AddExState(stateRegs, ~0, 0, 0);
	prevSFEXINDEX =SFEXINDEX;
}
#endif