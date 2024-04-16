/* This file is part of Snes9x. See LICENSE file. */

#ifdef INSIDE_DSP1_C
uint16_t DSP2Op09Word1 = 0;
uint16_t DSP2Op09Word2 = 0;
bool DSP2Op05HasLen = false;
int32_t DSP2Op05Len = 0;
bool DSP2Op06HasLen = false;
int32_t DSP2Op06Len = 0;
uint8_t DSP2Op05Transparent = 0;

void DSP2_Op05(void)
{
   uint8_t color;
   /* Overlay bitmap with transparency.
    * Input:
    *
    *   Bitmap 1:  i[0] <=> i[size-1]
    *   Bitmap 2:  i[size] <=> i[2*size-1]
    *
    * Output:
    *
    *   Bitmap 3:  o[0] <=> o[size-1]
    *
    * Processing:
    *
    *   Process all 4-bit pixels (nibbles) in the bitmap
    *
    *   if ( BM2_pixel == transparent_color )
    *      pixelout = BM1_pixel
    *   else
    *      pixelout = BM2_pixel

    * The max size bitmap is limited to 255 because the size parameter is a byte
    * I think size=0 is an error.  The behavior of the chip on size=0 is to
    * return the last value written to DR if you read DR on Op05 with
    * size = 0.  I don't think it's worth implementing this quirk unless it's
    * proven necessary.
    */

   int32_t n;
   uint8_t c1;
   uint8_t c2;
   uint8_t* p1 = DSP1.parameters;
   uint8_t* p2 = &DSP1.parameters[DSP2Op05Len];
   uint8_t* p3 = DSP1.output;

   color = DSP2Op05Transparent & 0x0f;

   for (n = 0; n < DSP2Op05Len; n++)
   {
      c1 = *p1++;
      c2 = *p2++;
      *p3++ = (((c2 >> 4) == color) ? c1 & 0xf0 : c2 & 0xf0) | (((c2 & 0x0f) == color) ? c1 & 0x0f : c2 & 0x0f);
   }
}

void DSP2_Op01(void)
{
   /* Op01 size is always 32 bytes input and output.
    * The hardware does strange things if you vary the size. */
   int32_t j;
   uint8_t c0, c1, c2, c3;
   uint8_t* p1 = DSP1.parameters;
   uint8_t* p2a = DSP1.output;
   uint8_t* p2b = &DSP1.output[16]; /* halfway */

   /* Process 8 blocks of 4 bytes each */
   for (j = 0; j < 8; j++)
   {
      c0 = *p1++;
      c1 = *p1++;
      c2 = *p1++;
      c3 = *p1++;
      *p2a++ = (c0 & 0x10) << 3 | (c0 & 0x01) << 6 | (c1 & 0x10) << 1 | (c1 & 0x01) << 4 | (c2 & 0x10) >> 1 | (c2 & 0x01) << 2 | (c3 & 0x10) >> 3 | (c3 & 0x01);
      *p2a++ = (c0 & 0x20) << 2 | (c0 & 0x02) << 5 | (c1 & 0x20)      | (c1 & 0x02) << 3 | (c2 & 0x20) >> 2 | (c2 & 0x02) << 1 | (c3 & 0x20) >> 4 | (c3 & 0x02) >> 1;
      *p2b++ = (c0 & 0x40) << 1 | (c0 & 0x04) << 4 | (c1 & 0x40) >> 1 | (c1 & 0x04) << 2 | (c2 & 0x40) >> 3 | (c2 & 0x04)      | (c3 & 0x40) >> 5 | (c3 & 0x04) >> 2;
      *p2b++ = (c0 & 0x80)      | (c0 & 0x08) << 3 | (c1 & 0x80) >> 2 | (c1 & 0x08) << 1 | (c2 & 0x80) >> 4 | (c2 & 0x08) >> 1 | (c3 & 0x80) >> 6 | (c3 & 0x08) >> 3;
   }
}

void DSP2_Op06(void)
{
   /* Input:
    *    size
    *    bitmap
    */

   int32_t i, j;

   for (i = 0, j = DSP2Op06Len - 1; i < DSP2Op06Len; i++, j--)
      DSP1.output[j] = (DSP1.parameters[i] << 4) | (DSP1.parameters[i] >> 4);
}

bool DSP2Op0DHasLen = false;
int32_t DSP2Op0DOutLen = 0;
int32_t DSP2Op0DInLen = 0;

/* Scale bitmap based on input length out output length */
void DSP2_Op0D(void)
{
   /* (Modified) Overload's algorithm */
   int32_t i;
   for(i = 0 ; i < DSP2Op0DOutLen ; i++)
   {
      int32_t j = i << 1;
      int32_t pixel_offset_low = ((j * DSP2Op0DInLen) / DSP2Op0DOutLen) >> 1;
      int32_t pixel_offset_high = (((j + 1) * DSP2Op0DInLen) / DSP2Op0DOutLen) >> 1;
      uint8_t pixel_low = DSP1.parameters[pixel_offset_low] >> 4;
      uint8_t pixel_high = DSP1.parameters[pixel_offset_high] & 0x0f;
      DSP1.output[i] = (pixel_low << 4) | pixel_high;
   }
}
#endif
