#ifndef _PPU_H_
#define _PPU_H_

/* This file is part of Snes9x. See LICENSE file. */
#include <stdint.h>
#include <stdbool.h>

#define FIRST_VISIBLE_LINE 1

extern uint8_t  GetBank;

#define TILE_2BIT 0
#define TILE_4BIT 1
#define TILE_8BIT 2

#define MAX_2BIT_TILES 4096
#define MAX_4BIT_TILES 2048
#define MAX_8BIT_TILES 1024

#define PPU_H_BEAM_IRQ_SOURCE (1 << 0)
#define PPU_V_BEAM_IRQ_SOURCE (1 << 1)
#define GSU_IRQ_SOURCE        (1 << 2)
#define SA1_DMA_IRQ_SOURCE    (1 << 5)
#define SA1_IRQ_SOURCE        (1 << 7)

typedef struct
{
   uint32_t Count [6];
   uint32_t Left  [6][6];
   uint32_t Right [6][6];
} ClipData;

typedef struct
{
   bool     ColorsChanged;
   uint8_t  HDMA;
   bool     OBJChanged;
   bool     RenderThisFrame;
   uint32_t FrameCount;
   uint8_t* TileCache;
   uint8_t* TileCached;
   bool     FirstVRAMRead;
   bool     DoubleHeightPixels;
   bool     Interlace;
   bool     DoubleWidthPixels;
   bool     HalfWidthPixels;
   int32_t  RenderedScreenHeight;
   int32_t  RenderedScreenWidth;
   uint8_t  Red          [256];
   uint8_t  Green        [256];
   uint8_t  Blue         [256];
   const uint8_t* XB;
   uint16_t *ScreenColors; // [256];
   uint16_t *DirectColors; // [256 * 8];
   int32_t  PreviousLine;
   int32_t  CurrentLine;
   int32_t  Controller;
   uint32_t Joypads   [5];
   uint32_t SuperScope;
   uint32_t Mouse     [2];
   int32_t  PrevMouseX[2];
   int32_t  PrevMouseY[2];
   ClipData Clip      [2];
} InternalPPU;

typedef struct
{
   int16_t  HPos;
   uint16_t VPos;
   uint16_t Name;
   uint8_t  VFlip;
   uint8_t  HFlip;
   uint8_t  Priority;
   uint8_t  Palette;
   uint8_t  Size;
} SOBJ;

typedef struct
{
   uint8_t BGMode;
   uint8_t BG3Priority;
   uint8_t Brightness;

   struct
   {
      bool     High;
      uint8_t  Increment;
      uint16_t Address;
      uint16_t Mask1;
      uint16_t FullGraphicCount;
      uint16_t Shift;
   } VMA;

   struct
   {
      uint16_t SCBase;
      uint16_t VOffset;
      uint16_t HOffset;
      uint8_t  BGSize;
      uint16_t NameBase;
      uint16_t SCSize;
   } BG [4];

   bool     CGFLIP;
   uint16_t CGDATA [256];
   uint8_t  FirstSprite;
   uint8_t  UNUSED1;
   SOBJ     OBJ    [128];
   uint8_t  OAMPriorityRotation;
   uint16_t OAMAddr;
   uint8_t  RangeTimeOver;
   uint8_t  OAMFlip;
   uint16_t UNUSED2;
   uint16_t IRQVBeamPos;
   uint16_t IRQHBeamPos;
   uint16_t VBeamPosLatched;
   uint16_t HBeamPosLatched;
   uint8_t  HBeamFlip;
   uint8_t  VBeamFlip;
   uint8_t  UNUSED3;
   int16_t  MatrixA;
   int16_t  MatrixB;
   int16_t  MatrixC;
   int16_t  MatrixD;
   int16_t  CentreX;
   int16_t  CentreY;
   uint8_t  Joypad1ButtonReadPos;
   uint8_t  Joypad2ButtonReadPos;
   uint8_t  CGADD;
   uint8_t  FixedColourRed;
   uint8_t  FixedColourGreen;
   uint8_t  FixedColourBlue;
   uint16_t SavedOAMAddr;
   uint16_t ScreenHeight;
   uint32_t WRAM;
   uint8_t  UNUSED4;
   bool     ForcedBlanking;
   bool     UNUSED5;
   bool     UNUSED6;
   uint8_t  OBJSizeSelect;
   uint16_t OBJNameBase;
   bool     UNUSED7;
   uint8_t  UNUSED8;
   uint8_t  OAMData                [512 + 32];
   bool     VTimerEnabled;
   bool     HTimerEnabled;
   int16_t  HTimerPosition;
   uint8_t  Mosaic;
   bool     BGMosaic               [4];
   bool     Mode7HFlip;
   bool     Mode7VFlip;
   uint8_t  Mode7Repeat;
   uint8_t  Window1Left;
   uint8_t  Window1Right;
   uint8_t  Window2Left;
   uint8_t  Window2Right;
   uint8_t  UNUSED9                [6];
   uint8_t  ClipWindowOverlapLogic [6];
   uint8_t  ClipWindow1Enable      [6];
   uint8_t  ClipWindow2Enable      [6];
   bool     ClipWindow1Inside      [6];
   bool     ClipWindow2Inside      [6];
   bool     RecomputeClipWindows;
   bool     CGFLIPRead;
   uint16_t OBJNameSelect;
   bool     Need16x8Multiply;
   uint8_t  Joypad3ButtonReadPos;
   uint8_t  UNUSED10 [2];
   uint16_t OAMWriteRegister;
   uint8_t  BGnxOFSbyte;
   uint8_t  OpenBus1;
   uint8_t  OpenBus2;
} SPPU;

#define CLIP_OR 0
#define CLIP_AND 1
#define CLIP_XOR 2
#define CLIP_XNOR 3

typedef struct
{
   bool     TransferDirection;
   bool     AAddressFixed;
   bool     AAddressDecrement;
   uint8_t  TransferMode;
   uint8_t  ABank;
   uint16_t AAddress;
   uint16_t Address;
   uint8_t  BAddress;

   /* General DMA only: */
   uint16_t TransferBytes;

   /* H-DMA only: */
   bool     HDMAIndirectAddressing;
   uint16_t IndirectAddress;
   uint8_t  IndirectBank;
   bool     Repeat;
   uint8_t  LineCount;
   bool     FirstLine;
} SDMA;

void S9xUpdateScreen(void);
void S9xResetPPU(void);
void S9xSoftResetPPU(void);
void S9xFixColourBrightness(void);
void S9xUpdateJoypads(void);
void S9xProcessMouse(int32_t which1);
void S9xSuperFXExec(void);

void S9xSetPPU(uint8_t Byte, uint16_t Address);
uint8_t S9xGetPPU(uint16_t Address);
void S9xSetCPU(uint8_t Byte, uint16_t Address);
uint8_t S9xGetCPU(uint16_t Address);

void S9xInitC4(void);
void S9xSetC4(uint8_t Byte, uint16_t Address);
uint8_t S9xGetC4(uint16_t Address);
void S9xSetC4RAM(uint8_t Byte, uint16_t Address);
uint8_t S9xGetC4RAM(uint16_t Address);

extern SPPU PPU;
extern SDMA DMA [8];
extern InternalPPU IPPU;

#include "memmap.h"

#define SNES_5C77 1
#define SNES_5C78 3
#define SNES_5A22 4
// #define SNES_5C77 2
// #define SNES_5C78 4
// #define SNES_5A22 3

#define MAX_5C77_VERSION 0x01
#define MAX_5A22_VERSION 0x02
#define MAX_5C78_VERSION 0x03

/* Platform specific input functions used by PPU.C */
void JustifierButtons(uint32_t*);
bool JustifierOffscreen(void);

static INLINE void FLUSH_REDRAW(void)
{
   if (IPPU.PreviousLine != IPPU.CurrentLine)
      S9xUpdateScreen();
}

static INLINE void REGISTER_2104(uint8_t byte)
{
   if (PPU.OAMAddr & 0x100)
   {
      int32_t addr = ((PPU.OAMAddr & 0x10f) << 1) + (PPU.OAMFlip & 1);
      if (byte != PPU.OAMData [addr])
      {
         uint32_t SignExtend[2] = {0x00, 0xff00};

         SOBJ* pObj;
         FLUSH_REDRAW();
         PPU.OAMData [addr] = byte;
         IPPU.OBJChanged = true;

         /* X position high bit, and sprite size (x4) */
         pObj = &PPU.OBJ [(addr & 0x1f) * 4];

         pObj->HPos = (pObj->HPos & 0xFF) | SignExtend[(byte >> 0) & 1];
         pObj++->Size = byte & 2;
         pObj->HPos = (pObj->HPos & 0xFF) | SignExtend[(byte >> 2) & 1];
         pObj++->Size = byte & 8;
         pObj->HPos = (pObj->HPos & 0xFF) | SignExtend[(byte >> 4) & 1];
         pObj++->Size = byte & 32;
         pObj->HPos = (pObj->HPos & 0xFF) | SignExtend[(byte >> 6) & 1];
         pObj->Size = byte & 128;
      }
      PPU.OAMFlip ^= 1;
      if (!(PPU.OAMFlip & 1))
      {
         ++PPU.OAMAddr;
         PPU.OAMAddr &= 0x1ff;
         if (PPU.OAMPriorityRotation && PPU.FirstSprite != (PPU.OAMAddr >> 1))
         {
            PPU.FirstSprite = (PPU.OAMAddr & 0xFE) >> 1;
            IPPU.OBJChanged = true;
         }
      }
      else if (PPU.OAMPriorityRotation && (PPU.OAMAddr & 1))
         IPPU.OBJChanged = true;
   }
   else if (!(PPU.OAMFlip & 1))
   {
      PPU.OAMWriteRegister &= 0xff00;
      PPU.OAMWriteRegister |= byte;
      PPU.OAMFlip |= 1;
      if (PPU.OAMPriorityRotation && (PPU.OAMAddr & 1))
         IPPU.OBJChanged = true;
   }
   else
   {
      int32_t addr;
      uint8_t lowbyte, highbyte;
      PPU.OAMWriteRegister &= 0x00ff;
      lowbyte = (uint8_t)(PPU.OAMWriteRegister);
      highbyte = byte;
      PPU.OAMWriteRegister |= byte << 8;
      addr = (PPU.OAMAddr << 1);

      if (lowbyte != PPU.OAMData [addr] || highbyte != PPU.OAMData [addr + 1])
      {
         FLUSH_REDRAW();
         PPU.OAMData [addr] = lowbyte;
         PPU.OAMData [addr + 1] = highbyte;
         IPPU.OBJChanged = true;
         if (addr & 2)
         {
            /* Tile */
            PPU.OBJ[addr = PPU.OAMAddr >> 1].Name = PPU.OAMWriteRegister & 0x1ff;
            /* priority, h and v flip. */
            PPU.OBJ[addr].Palette = (highbyte >> 1) & 7;
            PPU.OBJ[addr].Priority = (highbyte >> 4) & 3;
            PPU.OBJ[addr].HFlip = (highbyte >> 6) & 1;
            PPU.OBJ[addr].VFlip = (highbyte >> 7) & 1;
         }
         else
         {
            /* X position (low) */
            PPU.OBJ[addr = PPU.OAMAddr >> 1].HPos &= 0xFF00;
            PPU.OBJ[addr].HPos |= lowbyte;

            /* Sprite Y position */
            PPU.OBJ[addr].VPos = highbyte;
         }
      }
      PPU.OAMFlip &= ~1;
      ++PPU.OAMAddr;
      if (PPU.OAMPriorityRotation && PPU.FirstSprite != (PPU.OAMAddr >> 1))
      {
         PPU.FirstSprite = (PPU.OAMAddr & 0xFE) >> 1;
         IPPU.OBJChanged = true;
      }
   }

   Memory.FillRAM [0x2104] = byte;
}

static INLINE void REGISTER_2118(uint8_t Byte)
{
   uint32_t address;
   if (PPU.VMA.FullGraphicCount)
   {
      uint32_t rem = PPU.VMA.Address & PPU.VMA.Mask1;
      address = (((PPU.VMA.Address & ~PPU.VMA.Mask1) + (rem >> PPU.VMA.Shift) + ((rem & (PPU.VMA.FullGraphicCount - 1)) << 3)) << 1) & 0xffff;
      Memory.VRAM [address] = Byte;
   }
   else
      Memory.VRAM[address = (PPU.VMA.Address << 1) & 0xFFFF] = Byte;
   IPPU.TileCached[address >> 4] = false;
   IPPU.TileCached[address >> 5] = false;
   IPPU.TileCached[address >> 6] = false;
   if (!PPU.VMA.High)
      PPU.VMA.Address += PPU.VMA.Increment;
}

static INLINE void REGISTER_2118_tile(uint8_t Byte)
{
   uint32_t address;
   uint32_t rem = PPU.VMA.Address & PPU.VMA.Mask1;
   address = (((PPU.VMA.Address & ~PPU.VMA.Mask1) + (rem >> PPU.VMA.Shift) + ((rem & (PPU.VMA.FullGraphicCount - 1)) << 3)) << 1) & 0xffff;
   Memory.VRAM [address] = Byte;
   IPPU.TileCached[address >> 4] = false;
   IPPU.TileCached[address >> 5] = false;
   IPPU.TileCached[address >> 6] = false;
   if (!PPU.VMA.High)
      PPU.VMA.Address += PPU.VMA.Increment;
}

static INLINE void REGISTER_2118_linear(uint8_t Byte)
{
   uint32_t address = (PPU.VMA.Address << 1) & 0xFFFF;
   Memory.VRAM[address] = Byte;
   IPPU.TileCached[address >> 4] = false;
   IPPU.TileCached[address >> 5] = false;
   IPPU.TileCached[address >> 6] = false;
   if (!PPU.VMA.High)
      PPU.VMA.Address += PPU.VMA.Increment;
}

static INLINE void REGISTER_2119(uint8_t Byte)
{
   uint32_t address;
   if (PPU.VMA.FullGraphicCount)
   {
      uint32_t rem = PPU.VMA.Address & PPU.VMA.Mask1;
      address = ((((PPU.VMA.Address & ~PPU.VMA.Mask1) + (rem >> PPU.VMA.Shift) + ((rem & (PPU.VMA.FullGraphicCount - 1)) << 3)) << 1) + 1) & 0xFFFF;
      Memory.VRAM [address] = Byte;
   }
   else
      Memory.VRAM[address = ((PPU.VMA.Address << 1) + 1) & 0xFFFF] = Byte;
   IPPU.TileCached[address >> 4] = false;
   IPPU.TileCached[address >> 5] = false;
   IPPU.TileCached[address >> 6] = false;
   if (PPU.VMA.High)
      PPU.VMA.Address += PPU.VMA.Increment;
}

static INLINE void REGISTER_2119_tile(uint8_t Byte)
{
   uint32_t rem = PPU.VMA.Address & PPU.VMA.Mask1;
   uint32_t address = ((((PPU.VMA.Address & ~PPU.VMA.Mask1) + (rem >> PPU.VMA.Shift) + ((rem & (PPU.VMA.FullGraphicCount - 1)) << 3)) << 1) + 1) & 0xFFFF;
   Memory.VRAM [address] = Byte;
   IPPU.TileCached[address >> 4] = false;
   IPPU.TileCached[address >> 5] = false;
   IPPU.TileCached[address >> 6] = false;
   if (PPU.VMA.High)
      PPU.VMA.Address += PPU.VMA.Increment;
}

static INLINE void REGISTER_2119_linear(uint8_t Byte)
{
   uint32_t address;
   Memory.VRAM[address = ((PPU.VMA.Address << 1) + 1) & 0xFFFF] = Byte;
   IPPU.TileCached[address >> 4] = false;
   IPPU.TileCached[address >> 5] = false;
   IPPU.TileCached[address >> 6] = false;
   if (PPU.VMA.High)
      PPU.VMA.Address += PPU.VMA.Increment;
}

static INLINE void REGISTER_2122(uint8_t Byte)
{
   if (PPU.CGFLIP)
   {
      if ((Byte & 0x7f) != (PPU.CGDATA[PPU.CGADD] >> 8))
      {
         FLUSH_REDRAW();
         PPU.CGDATA[PPU.CGADD] &= 0x00FF;
         PPU.CGDATA[PPU.CGADD] |= (Byte & 0x7f) << 8;
         IPPU.ColorsChanged = true;
         IPPU.Blue [PPU.CGADD] = IPPU.XB [(Byte >> 2) & 0x1f];
         IPPU.Green [PPU.CGADD] = IPPU.XB [(PPU.CGDATA[PPU.CGADD] >> 5) & 0x1f];
         IPPU.ScreenColors [PPU.CGADD] = (uint16_t) BUILD_PIXEL(IPPU.Red [PPU.CGADD], IPPU.Green [PPU.CGADD], IPPU.Blue [PPU.CGADD]);
      }
      PPU.CGADD++;
   }
   else if (Byte != (uint8_t)(PPU.CGDATA[PPU.CGADD] & 0xff))
   {
      FLUSH_REDRAW();
      PPU.CGDATA[PPU.CGADD] &= 0x7F00;
      PPU.CGDATA[PPU.CGADD] |= Byte;
      IPPU.ColorsChanged = true;
      IPPU.Red [PPU.CGADD] = IPPU.XB [Byte & 0x1f];
      IPPU.Green [PPU.CGADD] = IPPU.XB [(PPU.CGDATA[PPU.CGADD] >> 5) & 0x1f];
      IPPU.ScreenColors [PPU.CGADD] = (uint16_t) BUILD_PIXEL(IPPU.Red [PPU.CGADD], IPPU.Green [PPU.CGADD], IPPU.Blue [PPU.CGADD]);
   }
   PPU.CGFLIP = !PPU.CGFLIP;
}

static INLINE void REGISTER_2180(uint8_t Byte)
{
   Memory.RAM[PPU.WRAM++] = Byte;
   PPU.WRAM &= 0x1FFFF;
   Memory.FillRAM [0x2180] = Byte;
}

static INLINE uint8_t REGISTER_4212(void)
{
   uint8_t GetBank = 0;
   if (CPU.V_Counter >= PPU.ScreenHeight + FIRST_VISIBLE_LINE && CPU.V_Counter < PPU.ScreenHeight + FIRST_VISIBLE_LINE + 3)
      GetBank = 1;

   GetBank |= CPU.Cycles >= Settings.HBlankStart ? 0x40 : 0;
   if (CPU.V_Counter >= PPU.ScreenHeight + FIRST_VISIBLE_LINE)
      GetBank |= 0x80; /* XXX: 0x80 or 0xc0 ? */

   return GetBank;
}
#endif
