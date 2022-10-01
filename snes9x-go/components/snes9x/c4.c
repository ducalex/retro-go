/* This file is part of Snes9x. See LICENSE file. */

#include <math.h>
#include <stdlib.h>
#include "c4.h"
#include "port.h"

int16_t C4WFXVal;
int16_t C4WFYVal;
int16_t C4WFZVal;
int16_t C4WFX2Val;
int16_t C4WFY2Val;
int16_t C4WFDist;
int16_t C4WFScale;

int32_t tanval;
static int32_t c4x, c4y, c4z;
static int32_t c4x2, c4y2, c4z2;

const int16_t C4MulTable[256] =
{
   0x0000,  0x0003,  0x0006,  0x0009,  0x000c,  0x000f,  0x0012,  0x0015,
   0x0019,  0x001c,  0x001f,  0x0022,  0x0025,  0x0028,  0x002b,  0x002f,
   0x0032,  0x0035,  0x0038,  0x003b,  0x003e,  0x0041,  0x0045,  0x0048,
   0x004b,  0x004e,  0x0051,  0x0054,  0x0057,  0x005b,  0x005e,  0x0061,
   0x0064,  0x0067,  0x006a,  0x006d,  0x0071,  0x0074,  0x0077,  0x007a,
   0x007d,  0x0080,  0x0083,  0x0087,  0x008a,  0x008d,  0x0090,  0x0093,
   0x0096,  0x0099,  0x009d,  0x00a0,  0x00a3,  0x00a6,  0x00a9,  0x00ac,
   0x00af,  0x00b3,  0x00b6,  0x00b9,  0x00bc,  0x00bf,  0x00c2,  0x00c5,
   0x00c9,  0x00cc,  0x00cf,  0x00d2,  0x00d5,  0x00d8,  0x00db,  0x00df,
   0x00e2,  0x00e5,  0x00e8,  0x00eb,  0x00ee,  0x00f1,  0x00f5,  0x00f8,
   0x00fb,  0x00fe,  0x0101,  0x0104,  0x0107,  0x010b,  0x010e,  0x0111,
   0x0114,  0x0117,  0x011a,  0x011d,  0x0121,  0x0124,  0x0127,  0x012a,
   0x012d,  0x0130,  0x0133,  0x0137,  0x013a,  0x013d,  0x0140,  0x0143,
   0x0146,  0x0149,  0x014d,  0x0150,  0x0153,  0x0156,  0x0159,  0x015c,
   0x015f,  0x0163,  0x0166,  0x0169,  0x016c,  0x016f,  0x0172,  0x0175,
   0x0178,  0x017c,  0x017f,  0x0182,  0x0185,  0x0188,  0x018b,  0x018e,
   0x0192,  0x0195,  0x0198,  0x019b,  0x019e,  0x01a1,  0x01a4,  0x01a8,
   0x01ab,  0x01ae,  0x01b1,  0x01b4,  0x01b7,  0x01ba,  0x01be,  0x01c1,
   0x01c4,  0x01c7,  0x01ca,  0x01cd,  0x01d0,  0x01d4,  0x01d7,  0x01da,
   0x01dd,  0x01e0,  0x01e3,  0x01e6,  0x01ea,  0x01ed,  0x01f0,  0x01f3,
   0x01f6,  0x01f9,  0x01fc,  0x0200,  0x0203,  0x0206,  0x0209,  0x020c,
   0x020f,  0x0212,  0x0216,  0x0219,  0x021c,  0x021f,  0x0222,  0x0225,
   0x0228,  0x022c,  0x022f,  0x0232,  0x0235,  0x0238,  0x023b,  0x023e,
   0x0242,  0x0245,  0x0248,  0x024b,  0x024e,  0x0251,  0x0254,  0x0258,
   0x025b,  0x025e,  0x0261,  0x0264,  0x0267,  0x026a,  0x026e,  0x0271,
   0x0274,  0x0277,  0x027a,  0x027d,  0x0280,  0x0284,  0x0287,  0x028a,
   0x028d,  0x0290,  0x0293,  0x0296,  0x029a,  0x029d,  0x02a0,  0x02a3,
   0x02a6,  0x02a9,  0x02ac,  0x02b0,  0x02b3,  0x02b6,  0x02b9,  0x02bc,
   0x02bf,  0x02c2,  0x02c6,  0x02c9,  0x02cc,  0x02cf,  0x02d2,  0x02d5,
   0x02d8,  0x02db,  0x02df,  0x02e2,  0x02e5,  0x02e8,  0x02eb,  0x02ee,
   0x02f1,  0x02f5,  0x02f8,  0x02fb,  0x02fe,  0x0301,  0x0304,  0x0307,
   0x030b,  0x030e,  0x0311,  0x0314,  0x0317,  0x031a,  0x031d,  0x0321
};

int16_t C4_Sin(int16_t Angle)
{
   int32_t S;
   int16_t AngleS7;

   if (Angle < 0)
   {
      if (Angle == -32768)
         return 0;
      return -C4_Sin(-Angle);
   }

   AngleS7 = Angle >> 7;
   S       = C4SinTable[AngleS7] + (C4MulTable[Angle & 0xff] * C4SinTable[0x80 + AngleS7] >> 15);

   if (S > 32767)
      S = 32767;
   return (int16_t) S;
}

int16_t C4_Cos(int16_t Angle)
{
   int32_t S;
   int16_t AngleS7;

   if (Angle < 0)
   {
      if (Angle == -32768)
         return -32768;
      Angle = -Angle;
   }
   AngleS7 = Angle >> 7;
   S       = C4SinTable[0x80 + AngleS7] - (C4MulTable[Angle & 0xff] * C4SinTable[AngleS7] >> 15);
   if (S < -32768)
      S = -32767;
   return (int16_t) S;
}

const int16_t atantbl[256] = {
   0,   1,   1,   2,   3,   3,   4,   4,   5,   6,   6,   7,   8,   8,   9,   10,
   10,  11,  11,  12,  13,  13,  14,  15,  15,  16,  16,  17,  18,  18,  19,  20,
   20,  21,  21,  22,  23,  23,  24,  25,  25,  26,  26,  27,  28,  28,  29,  29,
   30,  31,  31,  32,  33,  33,  34,  34,  35,  36,  36,  37,  37,  38,  39,  39,
   40,  40,  41,  42,  42,  43,  43,  44,  44,  45,  46,  46,  47,  47,  48,  49,
   49,  50,  50,  51,  51,  52,  53,  53,  54,  54,  55,  55,  56,  57,  57,  58,
   58,  59,  59,  60,  60,  61,  62,  62,  63,  63,  64,  64,  65,  65,  66,  66,
   67,  67,  68,  69,  69,  70,  70,  71,  71,  72,  72,  73,  73,  74,  74,  75,
   75,  76,  76,  77,  77,  78,  78,  79,  79,  80,  80,  81,  81,  82,  82,  83,
   83,  84,  84,  85,  85,  86,  86,  86,  87,  87,  88,  88,  89,  89,  90,  90,
   91,  91,  92,  92,  92,  93,  93,  94,  94,  95,  95,  96,  96,  96,  97,  97,
   98,  98,  99,  99,  99,  100, 100, 101, 101, 101, 102, 102, 103, 103, 104, 104,
   104, 105, 105, 106, 106, 106, 107, 107, 108, 108, 108, 109, 109, 109, 110, 110,
   111, 111, 111, 112, 112, 113, 113, 113, 114, 114, 114, 115, 115, 115, 116, 116,
   117, 117, 117, 118, 118, 118, 119, 119, 119, 120, 120, 120, 121, 121, 121, 122,
   122, 122, 123, 123, 123, 124, 124, 124, 125, 125, 125, 126, 126, 126, 127, 127
};

int16_t _atan2(int16_t x, int16_t y)
{
   int32_t absAtan;
   int32_t x1, y1;
   if (x == 0)
      return 0;

   x1 = ABS(x);
   y1 = ABS(y);

   if (x1 > y1)
      absAtan = atantbl[(uint8_t)((y1 << 8) / x1)];
   else
      absAtan = atantbl[(uint8_t)((x1 << 8) / y1)];

   if ((x >= 0) ^ (y >= 0))
      return -absAtan;

   return absAtan;
}

void C4TransfWireFrame()
{
   c4x = C4WFXVal;
   c4y = C4WFYVal;
   c4z = C4WFZVal - 0x95;

   /* Rotate X */
   tanval = -C4WFX2Val << 9;
   c4y2 = (c4y * C4_Cos(tanval) - c4z * C4_Sin(tanval)) >> 15;
   c4z2 = (c4y * C4_Sin(tanval) + c4z * C4_Cos(tanval)) >> 15;

   /* Rotate Y */
   tanval = -C4WFY2Val << 9;
   c4x2 = (c4x * C4_Cos(tanval) + c4z2 * C4_Sin(tanval)) >> 15;
   c4z = (c4x * -C4_Sin(tanval) + c4z2 * C4_Cos(tanval)) >> 15;

   /* Rotate Z */
   tanval = -C4WFDist << 9;
   c4x = (c4x2 * C4_Cos(tanval) - c4y2 * C4_Sin(tanval)) >> 15;
   c4y = (c4x2 * C4_Sin(tanval) + c4y2 * C4_Cos(tanval)) >> 15;

   /* Scale */
   C4WFXVal = (int16_t)(((int32_t)c4x * C4WFScale * 0x95) / (0x90 * (c4z + 0x95)));
   C4WFYVal = (int16_t)(((int32_t)c4y * C4WFScale * 0x95) / (0x90 * (c4z + 0x95)));
}

void C4TransfWireFrame2()
{
   c4x = C4WFXVal;
   c4y = C4WFYVal;
   c4z = C4WFZVal;

   /* Rotate X */
   tanval = -C4WFX2Val << 9;
   c4y2 = (c4y * C4_Cos(tanval) - c4z * C4_Sin(tanval)) >> 15;
   c4z2 = (c4y * C4_Sin(tanval) + c4z * C4_Cos(tanval)) >> 15;

   /* Rotate Y */
   tanval = -C4WFY2Val << 9;
   c4x2 = (c4x * C4_Cos(tanval) + c4z2 * C4_Sin(tanval)) >> 15;
   c4z = (c4x * -C4_Sin(tanval) + c4z2 * C4_Cos(tanval)) >> 15;

   /* Rotate Z */
   tanval = -C4WFDist << 9;
   c4x = (c4x2 * C4_Cos(tanval) - c4y2 * C4_Sin(tanval)) >> 15;
   c4y = (c4x2 * C4_Sin(tanval) + c4y2 * C4_Cos(tanval)) >> 15;

   /* Scale */
   C4WFXVal = (int16_t)(((int32_t)c4x * C4WFScale) / 0x100);
   C4WFYVal = (int16_t)(((int32_t)c4y * C4WFScale) / 0x100);
}

void C4CalcWireFrame()
{
   C4WFXVal = C4WFX2Val - C4WFXVal;
   C4WFYVal = C4WFY2Val - C4WFYVal;
   if (ABS(C4WFXVal) > ABS(C4WFYVal))
   {
      C4WFDist = ABS(C4WFXVal) + 1;
      C4WFYVal = (int16_t)(((int32_t)C4WFYVal << 8) / ABS(C4WFXVal));
      if (C4WFXVal < 0)
         C4WFXVal = -256;
      else
         C4WFXVal = 256;
   }
   else
   {
      if (C4WFYVal != 0)
      {
         C4WFDist = ABS(C4WFYVal) + 1;
         C4WFXVal = (int16_t)(((int32_t)C4WFXVal << 8) / ABS(C4WFYVal));
         if (C4WFYVal < 0)
            C4WFYVal = -256;
         else
            C4WFYVal = 256;
      }
      else
         C4WFDist = 0;
   }
}

int16_t C41FXVal;
int16_t C41FYVal;
int16_t C41FAngleRes;
int16_t C41FDist;
int16_t C41FDistVal;
