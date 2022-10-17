   // Now render an individual destination line
   while(true)
   {
         ULONG tmp;

         if(!mLineRepeatCount)
         {
            // Normal sprites fetch their counts on a packet basis
            if(mLineType!=line_abs_literal)
            {
               MY_GET_BITS(tmp,1)
               if(tmp) mLineType=line_literal; else mLineType=line_packed;
            }

            // Pixel store is empty what should we do
            switch(mLineType)
            {
               case line_abs_literal:
                  // This means end of line for us
                  mLinePixel=LINE_END;
                  goto EndWhile;
               case line_literal:
                  MY_GET_BITS(mLineRepeatCount,4)
                  mLineRepeatCount++;
                  break;
               case line_packed:
                  //
                  // From reading in between the lines only a packed line with
                  // a zero size i.e 0b00000 as a header is allowable as a packet end
                  //
                  MY_GET_BITS(mLineRepeatCount,4)
                  if(!mLineRepeatCount)
                  {
                     mLinePixel=LINE_END;
                     mLineRepeatCount++;
                     goto EndWhile;
                  }
                  else
                  {
                     MY_GET_BITS(tmp,mSPRCTL0_PixelBits)
                     pixel=mPenIndex[tmp];
                  }
                  mLineRepeatCount++;
                  break;
               default:
                  pixel = 0;
                  goto LoopContinue;
            }

         }
      /*
         if(pixel==LINE_END)
         {
               printf("ERROR!\n");
               goto EndWhile;
         }
      */
            mLineRepeatCount--;

            switch(mLineType)
            {
               case line_abs_literal:
                  MY_GET_BITS(pixel,mSPRCTL0_PixelBits)
                  // Check the special case of a zero in the last pixel
                  if(!mLineRepeatCount && !pixel)
                  {
                     mLinePixel=LINE_END;
                     goto EndWhile;
                  }
                  else
                     pixel=mPenIndex[pixel];
                  break;
               case line_literal:
                  MY_GET_BITS(tmp,mSPRCTL0_PixelBits)
                  pixel=mPenIndex[tmp];
                  break;
               case line_packed:
                  break;
               default:
                  pixel=0;
                  goto LoopContinue;
            }

   LoopContinue:;

      // This is allowed to update every pixel
      mHSIZACUM.Word+=mSPRHSIZ.Word;
      pixel_width=mHSIZACUM.Byte.High;
      mHSIZACUM.Byte.High=0;

      for(int hloop=0;hloop<pixel_width;hloop++)
      {
         // Draw if onscreen but break loop on transition to offscreen
         if(hoff>=0 && hoff<HANDY_SCREEN_WIDTH)
         {
            //ProcessPixel(hoff,pixel);
            PROCESS_PIXEL
            onscreen=TRUE;
            everonscreen=TRUE;
         }
         else
         {
            if(onscreen) break;
         }
         hoff += hsign;
      }
   }
mLinePixel = pixel;
EndWhile:;
