/* This file is part of Snes9x. See LICENSE file. */

#include "snes9x.h"
#include "dsp1.h"
#include "memmap.h"

#define INSIDE_DSP1_C
#include "dsp1emu.c"
#include "dsp2emu.c"

void (*SetDSP)(uint8_t, uint16_t) = &DSP1SetByte;
uint8_t(*GetDSP)(uint16_t) = &DSP1GetByte;

void S9xResetDSP1(void)
{
   DSP1.waiting4command = true;
   DSP1.in_count = 0;
   DSP1.out_count = 0;
   DSP1.in_index = 0;
   DSP1.out_index = 0;
   DSP1.first_parameter = true;
   if (!DSP1.output)
      DSP1.output = malloc(512);
   if (!DSP1.parameters)
      DSP1.parameters = malloc(512);
}

uint8_t S9xGetDSP(uint16_t address)
{
   return (*GetDSP)(address);
}

void S9xSetDSP(uint8_t byte, uint16_t address)
{
   (*SetDSP)(byte, address);
}

void DSP1SetByte(uint8_t byte, uint16_t address)
{
   if ((address & 0xf000) == 0x6000 || (address & 0x7fff) < 0x4000)
   {
      if ((DSP1.command == 0x0A || DSP1.command == 0x1A) && DSP1.out_count != 0)
      {
         DSP1.out_count--;
         DSP1.out_index++;
         return;
      }
      else if (DSP1.waiting4command)
      {
         DSP1.command = byte;
         DSP1.in_index = 0;
         DSP1.waiting4command = false;
         DSP1.first_parameter = true;
         switch (byte) /* Mario Kart uses 0x00, 0x02, 0x06, 0x0c, 0x28, 0x0a */
         {
         case 0x07:
         case 0x0a:
         case 0x0f:
         case 0x1f:
         case 0x27:
         case 0x2f:
            DSP1.in_count = 1;
            break;
         case 0x00:
         case 0x04:
         case 0x0e:
         case 0x10:
         case 0x1e:
         case 0x20:
         case 0x24:
         case 0x2e:
         case 0x30:
         case 0x3e:
            DSP1.in_count = 2;
            break;
         case 0x03:
         case 0x06:
         case 0x08:
         case 0x09:
         case 0x0b:
         case 0x0c:
         case 0x0d:
         case 0x13:
         case 0x16:
         case 0x19:
         case 0x1b:
         case 0x1d:
         case 0x23:
         case 0x26:
         case 0x28:
         case 0x29:
         case 0x2b:
         case 0x2c:
         case 0x2d:
         case 0x33:
         case 0x36:
         case 0x39:
         case 0x3b:
         case 0x3d:
            DSP1.in_count = 3;
            break;
         case 0x01:
         case 0x05:
         case 0x11:
         case 0x15:
         case 0x18:
         case 0x21:
         case 0x25:
         case 0x31:
         case 0x35:
         case 0x38:
            DSP1.in_count = 4;
            break;
         case 0x14:
         case 0x1c:
         case 0x34:
         case 0x3c:
            DSP1.in_count = 6;
            break;
         case 0x02:
         case 0x12:
         case 0x22:
         case 0x32:
            DSP1.in_count = 7;
            break;
         case 0x1a:
         case 0x2a:
         case 0x3a:
            DSP1.command = 0x1a;
            DSP1.in_count = 1;
            break;
         case 0x17:
         case 0x37:
         case 0x3F:
            DSP1.command = 0x1f;
            DSP1.in_count = 1;
            break;
         default:
            DSP1.in_count = 0;
            DSP1.waiting4command = true;
            DSP1.first_parameter = true;
            break;
         }
         DSP1.in_count <<= 1;
      }
      else
      {
         DSP1.parameters [DSP1.in_index] = byte;
         DSP1.first_parameter = false;
         DSP1.in_index++;
      }

      if (DSP1.waiting4command || (DSP1.first_parameter && byte == 0x80))
      {
         DSP1.waiting4command = true;
         DSP1.first_parameter = false;
      }
      else if (!(DSP1.first_parameter && (DSP1.in_count != 0 || (DSP1.in_count == 0 && DSP1.in_index == 0))))
      {
         if (DSP1.in_count)
         {
            if (--DSP1.in_count == 0)
            {
               DSP1.waiting4command = true;
               DSP1.out_index = 0;
               switch (DSP1.command)
               {
               case 0x1f:
                  DSP1.out_count = 2048;
                  break;
               case 0x00: /* Multiple */
                  Op00Multiplicand = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op00Multiplier = (int16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  DSPOp00();
                  DSP1.out_count = 2;
                  DSP1.output [0] = Op00Result & 0xFF;
                  DSP1.output [1] = (Op00Result >> 8) & 0xFF;
                  break;
               case 0x20: /* Multiple */
                  Op20Multiplicand = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op20Multiplier = (int16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  DSPOp20();
                  DSP1.out_count = 2;
                  DSP1.output [0] = Op20Result & 0xFF;
                  DSP1.output [1] = (Op20Result >> 8) & 0xFF;
                  break;
               case 0x30:
               case 0x10: /* Inverse */
                  Op10Coefficient = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op10Exponent = (int16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  DSPOp10();
                  DSP1.out_count = 4;
                  DSP1.output [0] = (uint8_t)(((int16_t) Op10CoefficientR) & 0xFF);
                  DSP1.output [1] = (uint8_t)((((int16_t) Op10CoefficientR) >> 8) & 0xFF);
                  DSP1.output [2] = (uint8_t)(((int16_t) Op10ExponentR) & 0xff);
                  DSP1.output [3] = (uint8_t)((((int16_t) Op10ExponentR) >> 8) & 0xff);
                  break;
               case 0x24:
               case 0x04: /* Sin and Cos of angle */
                  Op04Angle = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op04Radius = (uint16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  DSPOp04();
                  DSP1.out_count = 4;
                  DSP1.output [0] = (uint8_t)(Op04Sin & 0xFF);
                  DSP1.output [1] = (uint8_t)((Op04Sin >> 8) & 0xFF);
                  DSP1.output [2] = (uint8_t)(Op04Cos & 0xFF);
                  DSP1.output [3] = (uint8_t)((Op04Cos >> 8) & 0xFF);
                  break;
               case 0x08: /* Radius */
                  Op08X = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op08Y = (int16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  Op08Z = (int16_t)(DSP1.parameters [4] | (DSP1.parameters[5] << 8));
                  DSPOp08();
                  DSP1.out_count = 4;
                  DSP1.output [0] = (uint8_t)(((int16_t) Op08Ll) & 0xFF);
                  DSP1.output [1] = (uint8_t)((((int16_t) Op08Ll) >> 8) & 0xFF);
                  DSP1.output [2] = (uint8_t)(((int16_t) Op08Lh) & 0xFF);
                  DSP1.output [3] = (uint8_t)((((int16_t) Op08Lh) >> 8) & 0xFF);
                  break;
               case 0x18: /* Range */
                  Op18X = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op18Y = (int16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  Op18Z = (int16_t)(DSP1.parameters [4] | (DSP1.parameters[5] << 8));
                  Op18R = (int16_t)(DSP1.parameters [6] | (DSP1.parameters[7] << 8));
                  DSPOp18();
                  DSP1.out_count = 2;
                  DSP1.output [0] = (uint8_t)(Op18D & 0xFF);
                  DSP1.output [1] = (uint8_t)((Op18D >> 8) & 0xFF);
                  break;
               case 0x38: /* Range */
                  Op38X = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op38Y = (int16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  Op38Z = (int16_t)(DSP1.parameters [4] | (DSP1.parameters[5] << 8));
                  Op38R = (int16_t)(DSP1.parameters [6] | (DSP1.parameters[7] << 8));
                  DSPOp38();
                  DSP1.out_count = 2;
                  DSP1.output [0] = (uint8_t)(Op38D & 0xFF);
                  DSP1.output [1] = (uint8_t)((Op38D >> 8) & 0xFF);
                  break;
               case 0x28: /* Distance (vector length) */
                  Op28X = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op28Y = (int16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  Op28Z = (int16_t)(DSP1.parameters [4] | (DSP1.parameters[5] << 8));
                  DSPOp28();
                  DSP1.out_count = 2;
                  DSP1.output [0] = (uint8_t)(Op28R & 0xFF);
                  DSP1.output [1] = (uint8_t)((Op28R >> 8) & 0xFF);
                  break;
               case 0x2c:
               case 0x0c: /* Rotate (2D rotate) */
                  Op0CA = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op0CX1 = (int16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  Op0CY1 = (int16_t)(DSP1.parameters [4] | (DSP1.parameters[5] << 8));
                  DSPOp0C();
                  DSP1.out_count = 4;
                  DSP1.output [0] = (uint8_t)(Op0CX2 & 0xFF);
                  DSP1.output [1] = (uint8_t)((Op0CX2 >> 8) & 0xFF);
                  DSP1.output [2] = (uint8_t)(Op0CY2 & 0xFF);
                  DSP1.output [3] = (uint8_t)((Op0CY2 >> 8) & 0xFF);
                  break;
               case 0x3c:
               case 0x1c: /* Polar (3D rotate) */
                  Op1CZ = (DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op1CY = (DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  Op1CX = (DSP1.parameters [4] | (DSP1.parameters[5] << 8));
                  Op1CXBR = (DSP1.parameters [6] | (DSP1.parameters[7] << 8));
                  Op1CYBR = (DSP1.parameters [8] | (DSP1.parameters[9] << 8));
                  Op1CZBR = (DSP1.parameters [10] | (DSP1.parameters[11] << 8));
                  DSPOp1C();
                  DSP1.out_count = 6;
                  DSP1.output [0] = (uint8_t)(Op1CXAR & 0xFF);
                  DSP1.output [1] = (uint8_t)((Op1CXAR >> 8) & 0xFF);
                  DSP1.output [2] = (uint8_t)(Op1CYAR & 0xFF);
                  DSP1.output [3] = (uint8_t)((Op1CYAR >> 8) & 0xFF);
                  DSP1.output [4] = (uint8_t)(Op1CZAR & 0xFF);
                  DSP1.output [5] = (uint8_t)((Op1CZAR >> 8) & 0xFF);
                  break;
               case 0x32:
               case 0x22:
               case 0x12:
               case 0x02: /* Parameter (Projection) */
                  Op02FX = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op02FY = (int16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  Op02FZ = (int16_t)(DSP1.parameters [4] | (DSP1.parameters[5] << 8));
                  Op02LFE = (int16_t)(DSP1.parameters [6] | (DSP1.parameters[7] << 8));
                  Op02LES = (int16_t)(DSP1.parameters [8] | (DSP1.parameters[9] << 8));
                  Op02AAS = (uint16_t)(DSP1.parameters [10] | (DSP1.parameters[11] << 8));
                  Op02AZS = (uint16_t)(DSP1.parameters [12] | (DSP1.parameters[13] << 8));
                  DSPOp02();
                  DSP1.out_count = 8;
                  DSP1.output [0] = (uint8_t)(Op02VOF & 0xFF);
                  DSP1.output [1] = (uint8_t)((Op02VOF >> 8) & 0xFF);
                  DSP1.output [2] = (uint8_t)(Op02VVA & 0xFF);
                  DSP1.output [3] = (uint8_t)((Op02VVA >> 8) & 0xFF);
                  DSP1.output [4] = (uint8_t)(Op02CX & 0xFF);
                  DSP1.output [5] = (uint8_t)((Op02CX >> 8) & 0xFF);
                  DSP1.output [6] = (uint8_t)(Op02CY & 0xFF);
                  DSP1.output [7] = (uint8_t)((Op02CY >> 8) & 0xFF);
                  break;
               case 0x3a: /* 1a Mirror */
               case 0x2a: /* 1a Mirror */
               case 0x1a: /* Raster mode 7 matrix data */
               case 0x0a:
                  Op0AVS = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  DSPOp0A();
                  DSP1.out_count = 8;
                  DSP1.output [0] = (uint8_t)(Op0AA & 0xFF);
                  DSP1.output [2] = (uint8_t)(Op0AB & 0xFF);
                  DSP1.output [4] = (uint8_t)(Op0AC & 0xFF);
                  DSP1.output [6] = (uint8_t)(Op0AD & 0xFF);
                  DSP1.output [1] = (uint8_t)((Op0AA >> 8) & 0xFF);
                  DSP1.output [3] = (uint8_t)((Op0AB >> 8) & 0xFF);
                  DSP1.output [5] = (uint8_t)((Op0AC >> 8) & 0xFF);
                  DSP1.output [7] = (uint8_t)((Op0AD >> 8) & 0xFF);
                  DSP1.in_index = 0;
                  break;
               case 0x16:
               case 0x26:
               case 0x36:
               case 0x06: /* Project object */
                  Op06X = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op06Y = (int16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  Op06Z = (int16_t)(DSP1.parameters [4] | (DSP1.parameters[5] << 8));
                  DSPOp06();
                  DSP1.out_count = 6;
                  DSP1.output [0] = (uint8_t)(Op06H & 0xff);
                  DSP1.output [1] = (uint8_t)((Op06H >> 8) & 0xFF);
                  DSP1.output [2] = (uint8_t)(Op06V & 0xFF);
                  DSP1.output [3] = (uint8_t)((Op06V >> 8) & 0xFF);
                  DSP1.output [4] = (uint8_t)(Op06M & 0xFF);
                  DSP1.output [5] = (uint8_t)((Op06M >> 8) & 0xFF);
                  break;
               case 0x1e:
               case 0x2e:
               case 0x3e:
               case 0x0e: /* Target */
                  Op0EH = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op0EV = (int16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  DSPOp0E();
                  DSP1.out_count = 4;
                  DSP1.output [0] = (uint8_t)(Op0EX & 0xFF);
                  DSP1.output [1] = (uint8_t)((Op0EX >> 8) & 0xFF);
                  DSP1.output [2] = (uint8_t)(Op0EY & 0xFF);
                  DSP1.output [3] = (uint8_t)((Op0EY >> 8) & 0xFF);
                  break;
               case 0x05: /* Extra commands used by Pilot Wings */
               case 0x35:
               case 0x31:
               case 0x01: /* Set attitude matrix A */
                  Op01m = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op01Zr = (int16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  Op01Yr = (int16_t)(DSP1.parameters [4] | (DSP1.parameters[5] << 8));
                  Op01Xr = (int16_t)(DSP1.parameters [6] | (DSP1.parameters[7] << 8));
                  DSPOp01();
                  break;
               case 0x15:
               case 0x11: /* Set attitude matrix B */
                  Op11m = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op11Zr = (int16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  Op11Yr = (int16_t)(DSP1.parameters [4] | (DSP1.parameters[5] << 8));
                  Op11Xr = (int16_t)(DSP1.parameters [6] | (DSP1.parameters[7] << 8));
                  DSPOp11();
                  break;
               case 0x25:
               case 0x21: /* Set attitude matrix C */
                  Op21m = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op21Zr = (int16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  Op21Yr = (int16_t)(DSP1.parameters [4] | (DSP1.parameters[5] << 8));
                  Op21Xr = (int16_t)(DSP1.parameters [6] | (DSP1.parameters[7] << 8));
                  DSPOp21();
                  break;
               case 0x09:
               case 0x39:
               case 0x3d:
               case 0x0d: /* Objective matrix A */
                  Op0DX = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op0DY = (int16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  Op0DZ = (int16_t)(DSP1.parameters [4] | (DSP1.parameters[5] << 8));
                  DSPOp0D();
                  DSP1.out_count = 6;
                  DSP1.output [0] = (uint8_t)(Op0DF & 0xFF);
                  DSP1.output [1] = (uint8_t)((Op0DF >> 8) & 0xFF);
                  DSP1.output [2] = (uint8_t)(Op0DL & 0xFF);
                  DSP1.output [3] = (uint8_t)((Op0DL >> 8) & 0xFF);
                  DSP1.output [4] = (uint8_t)(Op0DU & 0xFF);
                  DSP1.output [5] = (uint8_t)((Op0DU >> 8) & 0xFF);
                  break;
               case 0x19:
               case 0x1d: /* Objective matrix B */
                  Op1DX = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op1DY = (int16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  Op1DZ = (int16_t)(DSP1.parameters [4] | (DSP1.parameters[5] << 8));
                  DSPOp1D();
                  DSP1.out_count = 6;
                  DSP1.output [0] = (uint8_t)(Op1DF & 0xFF);
                  DSP1.output [1] = (uint8_t)((Op1DF >> 8) & 0xFF);
                  DSP1.output [2] = (uint8_t)(Op1DL & 0xFF);
                  DSP1.output [3] = (uint8_t)((Op1DL >> 8) & 0xFF);
                  DSP1.output [4] = (uint8_t)(Op1DU & 0xFF);
                  DSP1.output [5] = (uint8_t)((Op1DU >> 8) & 0xFF);
                  break;
               case 0x29:
               case 0x2d: /* Objective matrix C */
                  Op2DX = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op2DY = (int16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  Op2DZ = (int16_t)(DSP1.parameters [4] | (DSP1.parameters[5] << 8));
                  DSPOp2D();
                  DSP1.out_count = 6;
                  DSP1.output [0] = (uint8_t)(Op2DF & 0xFF);
                  DSP1.output [1] = (uint8_t)((Op2DF >> 8) & 0xFF);
                  DSP1.output [2] = (uint8_t)(Op2DL & 0xFF);
                  DSP1.output [3] = (uint8_t)((Op2DL >> 8) & 0xFF);
                  DSP1.output [4] = (uint8_t)(Op2DU & 0xFF);
                  DSP1.output [5] = (uint8_t)((Op2DU >> 8) & 0xFF);
                  break;
               case 0x33:
               case 0x03: /* Subjective matrix A */
                  Op03F = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op03L = (int16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  Op03U = (int16_t)(DSP1.parameters [4] | (DSP1.parameters[5] << 8));
                  DSPOp03();
                  DSP1.out_count = 6;
                  DSP1.output [0] = (uint8_t)(Op03X & 0xFF);
                  DSP1.output [1] = (uint8_t)((Op03X >> 8) & 0xFF);
                  DSP1.output [2] = (uint8_t)(Op03Y & 0xFF);
                  DSP1.output [3] = (uint8_t)((Op03Y >> 8) & 0xFF);
                  DSP1.output [4] = (uint8_t)(Op03Z & 0xFF);
                  DSP1.output [5] = (uint8_t)((Op03Z >> 8) & 0xFF);
                  break;
               case 0x13: /* Subjective matrix B */
                  Op13F = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op13L = (int16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  Op13U = (int16_t)(DSP1.parameters [4] | (DSP1.parameters[5] << 8));
                  DSPOp13();
                  DSP1.out_count = 6;
                  DSP1.output [0] = (uint8_t)(Op13X & 0xFF);
                  DSP1.output [1] = (uint8_t)((Op13X >> 8) & 0xFF);
                  DSP1.output [2] = (uint8_t)(Op13Y & 0xFF);
                  DSP1.output [3] = (uint8_t)((Op13Y >> 8) & 0xFF);
                  DSP1.output [4] = (uint8_t)(Op13Z & 0xFF);
                  DSP1.output [5] = (uint8_t)((Op13Z >> 8) & 0xFF);
                  break;
               case 0x23: /* Subjective matrix C */
                  Op23F = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op23L = (int16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  Op23U = (int16_t)(DSP1.parameters [4] | (DSP1.parameters[5] << 8));
                  DSPOp23();
                  DSP1.out_count = 6;
                  DSP1.output [0] = (uint8_t)(Op23X & 0xFF);
                  DSP1.output [1] = (uint8_t)((Op23X >> 8) & 0xFF);
                  DSP1.output [2] = (uint8_t)(Op23Y & 0xFF);
                  DSP1.output [3] = (uint8_t)((Op23Y >> 8) & 0xFF);
                  DSP1.output [4] = (uint8_t)(Op23Z & 0xFF);
                  DSP1.output [5] = (uint8_t)((Op23Z >> 8) & 0xFF);
                  break;
               case 0x3b:
               case 0x0b:
                  Op0BX = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op0BY = (int16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  Op0BZ = (int16_t)(DSP1.parameters [4] | (DSP1.parameters[5] << 8));
                  DSPOp0B();
                  DSP1.out_count = 2;
                  DSP1.output [0] = (uint8_t)(Op0BS & 0xFF);
                  DSP1.output [1] = (uint8_t)((Op0BS >> 8) & 0xFF);
                  break;
               case 0x1b:
                  Op1BX = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op1BY = (int16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  Op1BZ = (int16_t)(DSP1.parameters [4] | (DSP1.parameters[5] << 8));
                  DSPOp1B();
                  DSP1.out_count = 2;
                  DSP1.output [0] = (uint8_t)(Op1BS & 0xFF);
                  DSP1.output [1] = (uint8_t)((Op1BS >> 8) & 0xFF);
                  break;
               case 0x2b:
                  Op2BX = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op2BY = (int16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  Op2BZ = (int16_t)(DSP1.parameters [4] | (DSP1.parameters[5] << 8));
                  DSPOp2B();
                  DSP1.out_count = 2;
                  DSP1.output [0] = (uint8_t)(Op2BS & 0xFF);
                  DSP1.output [1] = (uint8_t)((Op2BS >> 8) & 0xFF);
                  break;
               case 0x34:
               case 0x14:
                  Op14Zr = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  Op14Xr = (int16_t)(DSP1.parameters [2] | (DSP1.parameters[3] << 8));
                  Op14Yr = (int16_t)(DSP1.parameters [4] | (DSP1.parameters[5] << 8));
                  Op14U = (int16_t)(DSP1.parameters [6] | (DSP1.parameters[7] << 8));
                  Op14F = (int16_t)(DSP1.parameters [8] | (DSP1.parameters[9] << 8));
                  Op14L = (int16_t)(DSP1.parameters [10] | (DSP1.parameters[11] << 8));
                  DSPOp14();
                  DSP1.out_count = 6;
                  DSP1.output [0] = (uint8_t)(Op14Zrr & 0xFF);
                  DSP1.output [1] = (uint8_t)((Op14Zrr >> 8) & 0xFF);
                  DSP1.output [2] = (uint8_t)(Op14Xrr & 0xFF);
                  DSP1.output [3] = (uint8_t)((Op14Xrr >> 8) & 0xFF);
                  DSP1.output [4] = (uint8_t)(Op14Yrr & 0xFF);
                  DSP1.output [5] = (uint8_t)((Op14Yrr >> 8) & 0xFF);
                  break;
               case 0x27:
               case 0x2F:
                  Op2FUnknown = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  DSPOp2F();
                  DSP1.out_count = 2;
                  DSP1.output [0] = (uint8_t)(Op2FSize & 0xFF);
                  DSP1.output [1] = (uint8_t)((Op2FSize >> 8) & 0xFF);
                  break;
               case 0x07:
               case 0x0F:
                  Op0FRamsize = (int16_t)(DSP1.parameters [0] | (DSP1.parameters[1] << 8));
                  DSPOp0F();
                  DSP1.out_count = 2;
                  DSP1.output [0] = (uint8_t)(Op0FPass & 0xFF);
                  DSP1.output [1] = (uint8_t)((Op0FPass >> 8) & 0xFF);
                  break;
               default:
                  break;
               }
            }
         }
      }
   }
}

uint8_t DSP1GetByte(uint16_t address)
{
   uint8_t t;
   if ((address & 0xf000) == 0x6000 || (address & 0x7fff) < 0x4000)
   {
      if (DSP1.out_count)
      {
         t = (uint8_t) DSP1.output [DSP1.out_index];
         DSP1.out_index++;
         if (--DSP1.out_count == 0)
         {
            if (DSP1.command == 0x1a || DSP1.command == 0x0a)
            {
               DSPOp0A();
               DSP1.out_count = 8;
               DSP1.out_index = 0;
               DSP1.output [0] = (Op0AA & 0xFF);
               DSP1.output [1] = (Op0AA >> 8) & 0xFF;
               DSP1.output [2] = (Op0AB & 0xFF);
               DSP1.output [3] = (Op0AB >> 8) & 0xFF;
               DSP1.output [4] = (Op0AC & 0xFF);
               DSP1.output [5] = (Op0AC >> 8) & 0xFF;
               DSP1.output [6] = (Op0AD & 0xFF);
               DSP1.output [7] = (Op0AD >> 8) & 0xFF;
            }
            if (DSP1.command == 0x1f)
            {
               if ((DSP1.out_index % 2) != 0)
                  t = (uint8_t)DSP1ROM[DSP1.out_index >> 1];
               else
                  t = DSP1ROM[DSP1.out_index >> 1] >> 8;
            }
         }
         DSP1.waiting4command = true;
      }
      else
         t = 0xff;
   }
   else
      t = 0x80;
   return t;
}

void DSP2SetByte(uint8_t byte, uint16_t address)
{
#ifndef FAST_LSB_WORD_ACCESS
   uint32_t temp;
#endif

   if ((address & 0xf000) == 0x6000 || (address >= 0x8000 && address < 0xc000))
   {
      if (DSP1.waiting4command)
      {
         DSP1.command = byte;
         DSP1.in_index = 0;
         DSP1.waiting4command = false;
         switch (byte)
         {
         default:
            DSP1.in_count = 0;
            break;
         case 0x03:
         case 0x05:
         case 0x06:
            DSP1.in_count = 1;
            break;
         case 0x0d:
            DSP1.in_count = 2;
            break;
         case 0x09:
            DSP1.in_count = 4;
            break;
         case 0x01:
            DSP1.in_count = 32;
            break;
         }
      }
      else
      {
         DSP1.parameters [DSP1.in_index] = byte;
         DSP1.in_index++;
      }

      if (DSP1.in_count == DSP1.in_index)
      {
         DSP1.waiting4command = true;
         DSP1.out_index = 0;
         switch (DSP1.command)
         {
         case 0x0D:
            if (DSP2Op0DHasLen)
            {
               DSP2Op0DHasLen = false;
               DSP1.out_count = DSP2Op0DOutLen;
               DSP2_Op0D();
            }
            else
            {
               DSP2Op0DInLen = DSP1.parameters[0];
               DSP2Op0DOutLen = DSP1.parameters[1];
               DSP1.in_index = 0;
               DSP1.in_count = (DSP2Op0DInLen + 1) >> 1;
               DSP2Op0DHasLen = true;
               if (byte)
                  DSP1.waiting4command = false;
            }
            break;
         case 0x06:
            if (DSP2Op06HasLen)
            {
               DSP2Op06HasLen = false;
               DSP1.out_count = DSP2Op06Len;
               DSP2_Op06();
            }
            else
            {
               DSP2Op06Len = DSP1.parameters[0];
               DSP1.in_index = 0;
               DSP1.in_count = DSP2Op06Len;
               DSP2Op06HasLen = true;
               if (byte)
                  DSP1.waiting4command = false;
            }
            break;
         case 0x01:
            DSP1.out_count = 32;
            DSP2_Op01();
            break;
         case 0x09: /* Multiply - don't yet know if this is signed or unsigned */
            DSP2Op09Word1 = DSP1.parameters[0] | (DSP1.parameters[1] << 8);
            DSP2Op09Word2 = DSP1.parameters[2] | (DSP1.parameters[3] << 8);
            DSP1.out_count = 4;
#ifdef FAST_LSB_WORD_ACCESS
            *(uint32_t*)DSP1.output = DSP2Op09Word1 * DSP2Op09Word2;
#else
            temp = DSP2Op09Word1 * DSP2Op09Word2;
            DSP1.output[0] = temp & 0xFF;
            DSP1.output[1] = (temp >> 8) & 0xFF;
            DSP1.output[2] = (temp >> 16) & 0xFF;
            DSP1.output[3] = (temp >> 24) & 0xFF;
#endif
            break;
         case 0x05:
            if (DSP2Op05HasLen)
            {
               DSP2Op05HasLen = false;
               DSP1.out_count = DSP2Op05Len;
               DSP2_Op05();
            }
            else
            {
               DSP2Op05Len = DSP1.parameters[0];
               DSP1.in_index = 0;
               DSP1.in_count = 2 * DSP2Op05Len;
               DSP2Op05HasLen = true;
               if (byte)
                  DSP1.waiting4command = false;
            }
            break;

         case 0x03:
            DSP2Op05Transparent = DSP1.parameters[0];
            break;
         case 0x0f:
         default:
            break;
         }
      }
   }
}

uint8_t DSP2GetByte(uint16_t address)
{
   uint8_t t;
   if ((address & 0xf000) == 0x6000 || (address >= 0x8000 && address < 0xc000))
   {
      if (DSP1.out_count)
      {
         t = (uint8_t) DSP1.output [DSP1.out_index];
         DSP1.out_index++;
         if (DSP1.out_count == DSP1.out_index)
            DSP1.out_count = 0;
      }
      else
         t = 0xff;
   }
   else
      t = 0x80;
   return t;
}
