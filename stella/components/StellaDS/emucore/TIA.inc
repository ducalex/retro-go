
    case (myPFBit | myBLBit):  // Playfield and Ball only are enabled (with our without myPlayfieldPriorityAndScore)
    {
        if (!(myPlayfieldPriorityAndScore & ScoreBit))
        {
             uInt32* mPF = &myCurrentPFMask[hpos];
             uInt8* mBL = &myCurrentBLMask[hpos];

             while(mfp < ending)
             {
                 if(0 /* disabled fast path */)
                 {
                     *(uInt32*)mfp = (myPF & *mPF) ? myColor[MYCOLUPF] : myColor[MYCOLUBK];
                     mPF += 4; mBL += 4; mfp += 4;
                 }
                 else
                 {
                     *mfp = ((myPF & *mPF) || *mBL) ? myColor[MYCOLUPF] : myColor[MYCOLUBK];
#ifndef COLLISIONS_OFF
                     if((myPF & *mPF) && *mBL) myCollision |= ourCollisionTable[myPFBit | myBLBit];
#endif                     
                     ++mPF; ++mBL; ++mfp;
                 }
            }
        }
        else    // Do this the hard way...
        {
             uInt32*mPF = &myCurrentPFMask[hpos];
             uInt8* mBL = &myCurrentBLMask[hpos];
             for(; mfp < ending; ++mfp)
             {
               uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
               if(*mBL++)                enabled |= myBLBit;
               HANDLE_COLOR_AND_COLLISIONS;
             }
        }
     }
     break;
    
    case (myPFBit | myBLBit | myP1Bit | myP0Bit):   // Playfield and Ball plus Player 1 and Player 0 enabled...
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(*mBL++)                        enabled |= myBLBit;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
     }
     break;
    
    case (myPFBit | myBLBit | myP0Bit):     // Playfield and Ball plus Player 0 enabled...
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(*mBL++)                        enabled |= myBLBit;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
     }
     break;
        
    case (myPFBit | myBLBit | myM0Bit):     // Playfield and Ball plus Missile 0 enabled... (Elevators Amiss)
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(*mBL++)                        enabled |= myBLBit;
           if(*mM0++)                        enabled |= myM0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myPFBit | myBLBit | myM1Bit):     // Playfield and Ball plus Missile 1 enabled...
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(*mBL++)                        enabled |= myBLBit;
           if(*mM1++)                        enabled |= myM1Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myPFBit | myBLBit | myP1Bit | myM0Bit):   // Playfield and Ball plus Player 1 and Missile 0 enabled...
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(*mBL++)                        enabled |= myBLBit;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           if(*mM0++)                        enabled |= myM0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
    
    case (myPFBit | myBLBit | myM1Bit | myP1Bit): // Playfield and Ball plus Missle 1 and Player 1
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(*mBL++)                        enabled |= myBLBit;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           if(*mM1++)                        enabled |= myM1Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
       
    case (myPFBit | myBLBit | myP0Bit | myM0Bit | myM1Bit): // Playfield and Ball plus Player 0 and Missile 0 plus Missile 1 enabled...
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(*mBL++)                        enabled |= myBLBit;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           if(*mM0++)                        enabled |= myM0Bit;
           if(*mM1++)                        enabled |= myM1Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myPFBit | myBLBit | myM1Bit | myP1Bit | myM0Bit):
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(*mBL++)                        enabled |= myBLBit;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           if(*mM1++)                        enabled |= myM1Bit;
           if(*mM0++)                        enabled |= myM0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;

    case (myPFBit | myBLBit | myP1Bit | myM0Bit | myP0Bit): // Playfield and Ball plus P1, M0 and P0
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(*mBL++)                        enabled |= myBLBit;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           if(*mM0++)                        enabled |= myM0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;

    case (myPFBit | myBLBit | myP0Bit | myM0Bit):   // Playfield and Ball plus Player 0 and Missile 0 enabled...
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(*mBL++)                        enabled |= myBLBit;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           if(*mM0++)                        enabled |= myM0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myPFBit | myBLBit | myP1Bit): // Playfield and Ball plus Player 1 enabled...
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(*mBL++)                        enabled |= myBLBit;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;

    case (myPFBit | myBLBit | myM0Bit | myM1Bit): // Playfield and Ball plus Missile 0 and Missile 1 enabled...
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(*mBL++)                        enabled |= myBLBit;
           if(*mM0++)                        enabled |= myM0Bit;
           if(*mM1++)                        enabled |= myM1Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;

    case (myPFBit | myBLBit | myM1Bit | myP0Bit): // Playfield and Ball plus Missle 1 and Player 0
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(*mBL++)                        enabled |= myBLBit;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           if(*mM1++)                        enabled |= myM1Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myPFBit | myBLBit | myM1Bit | myP1Bit | myM0Bit | myP0Bit): // Everything is enbaled...
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(*mBL++)                        enabled |= myBLBit;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           if(*mM1++)                        enabled |= myM1Bit;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           if(*mM0++)                        enabled |= myM0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;        
        
    case (myP0Bit | myP1Bit): // Player 0 and 1 is enabled only...
    {
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mP1 = &myCurrentP1Mask[hpos];

         while(mfp < ending)
         {
           if(0 /* disabled fast path */)
           {
             *(uInt32*)mfp = myColor[MYCOLUBK];
             mP0 += 4; mP1 += 4; mfp += 4;
           }
           else
           {
             *mfp = (myCurrentGRP0 & *mP0) ? 
                 myColor[MYCOLUP0] : ((myCurrentGRP1 & *mP1) ? myColor[MYCOLUP1] : myColor[MYCOLUBK]);

#ifndef COLLISIONS_OFF
             if((myCurrentGRP0 & *mP0) && (myCurrentGRP1 & *mP1))
               myCollision |= ourCollisionTable[myP0Bit | myP1Bit];
#endif
             ++mP0; ++mP1; ++mfp;
           }
         }
    }
    break;
        
    case (myP0Bit | myP1Bit | myPFBit):   // Player 0 and 1 plus Playfield
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myP0Bit | myP1Bit | myPFBit | myM1Bit): // Player 0 and 1 plus Playfield plus Missile 1
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           if(*mM1++)                        enabled |= myM1Bit;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myP0Bit | myP1Bit | myBLBit | myM1Bit): // Player 0 and 1 plus Ball plus Missile 1
    {
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];              
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = myPlayfieldPriorityAndScore;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           if(*mM1++)                        enabled |= myM1Bit;
           if(*mBL++)                        enabled |= myBLBit;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myP0Bit | myP1Bit | myM1Bit): // Player 0 and 1 plus Missile 1
    {
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = myPlayfieldPriorityAndScore;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           if(*mM1++)                        enabled |= myM1Bit;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myP0Bit | myP1Bit | myPFBit | myM0Bit): // // Player 0 and 1 plus Playfield plus Missile 0
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           if(*mM0++)                        enabled |= myM0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myP0Bit | myP1Bit | myM0Bit | myM1Bit | myBLBit): // Player 0/1 plus Missile 0/1 plus Ball enabled...
    {
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = myPlayfieldPriorityAndScore;
           if(*mBL++)                        enabled |= myBLBit;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           if(*mM1++)                        enabled |= myM1Bit;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           if(*mM0++)                        enabled |= myM0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myP0Bit | myP1Bit | myBLBit): // // Player 0 and 1 plus Ball
    {
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = myPlayfieldPriorityAndScore;
           if(*mBL++)                        enabled |= myBLBit;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myP0Bit | myP1Bit | myPFBit | myM0Bit | myM1Bit): // // Playfield plus M0, M1, P0, P1
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(*mM0++)                        enabled |= myM0Bit;
           if(*mM1++)                        enabled |= myM1Bit;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myP0Bit | myM0Bit | myP1Bit): // Player 0 and Missile 0 and Missile 1 are enabled
    {
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];            
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = myPlayfieldPriorityAndScore;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           if(*mM0++)                        enabled |= myM0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myPFBit): // Playfield bit set... with or without score
    {
           uInt32* mask = &myCurrentPFMask[hpos];
           if (myPlayfieldPriorityAndScore & PriorityBit)   // Priority bit overrides the Score bit
           {
             // Update a uInt8 at a time until reaching a uInt32 boundary
             for(; ((uintptr_t)mfp & 0x03); ++mfp, ++mask)
             {
               *mfp = (myPF & *mask) ? myColor[MYCOLUPF] : myColor[MYCOLUBK];
             }

             // Now, update a uInt32 at a time
             for(; mfp < ending; mfp++, mask++)
             {
               *mfp = (myPF & *mask) ? myColor[MYCOLUPF] : myColor[MYCOLUBK];
             }
           }
           else if (myPlayfieldPriorityAndScore & ScoreBit)   // Playfield is enabled and the score bit is set 
           {
             // Update a uInt8 at a time until reaching a uInt32 boundary
             for(; ((uintptr_t)mfp & 0x03); ++mfp, ++mask, ++hpos)
             {
               *mfp = (myPF & *mask) ? (hpos < 80 ? myColor[MYCOLUP0] : myColor[MYCOLUP1]) : myColor[MYCOLUBK];
             }

             // Now, update a uInt32 at a time
             for(; mfp < ending; mfp++, mask++, hpos++)
             {
               *mfp = (myPF & *mask) ? (hpos < 80 ? myColor[MYCOLUP0] : myColor[MYCOLUP1]) : myColor[MYCOLUBK];
             }
           }          
           else  // Playfield is enabled and the score bit is not set and priority clear...
           {
             // Update a uInt8 at a time until reaching a uInt32 boundary
             for(; ((uintptr_t)mfp & 0x03); ++mfp, ++mask)
             {
               *mfp = (myPF & *mask) ? myColor[MYCOLUPF] : myColor[MYCOLUBK];
             }

             // Now, update a uInt32 at a time
             for(; mfp < ending; mfp++, mask++)
             {
               *mfp = (myPF & *mask) ? myColor[MYCOLUPF] : myColor[MYCOLUBK];
             }
           }
    }
    break;
           
    case (myM0Bit | myPFBit): // Playfield + Missile 0
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         uInt8 meo3 = (myEnabledObjects & myM0Bit) ? 0xFF:0x00;
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(meo3 & *mM0++)                enabled |= myM0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myPFBit | myP0Bit): // Playfield and Player 0 are enabled
    {
           if (myPlayfieldPriorityAndScore & PriorityBit) // Priority set
           {
             uInt32* mPF = &myCurrentPFMask[hpos];
             uInt8* mP0 = &myCurrentP0Mask[hpos];

             while(mfp < ending)
             {
               if(0 /* disabled fast path */)
               {
                 *(uInt32*)mfp = (myPF & *mPF) ? myColor[MYCOLUPF] : myColor[MYCOLUBK];
                 mPF += 4; mP0 += 4; mfp += 4;
               }
               else
               {
                 *mfp = (myPF & *mPF) ? myColor[MYCOLUPF] : 
                     ((myCurrentGRP0 & *mP0) ? myColor[MYCOLUP0] : myColor[MYCOLUBK]);
#ifndef COLLISIONS_OFF
                 if((myPF & *mPF) && (myCurrentGRP0 & *mP0))
                   myCollision |= ourCollisionTable[myPFBit | myP0Bit];
#endif
                 ++mPF; ++mP0; ++mfp;
               }
             }
           }
           else if (!(myPlayfieldPriorityAndScore & ScoreBit)) // Priority not set and Score not set
           {
             uInt32* mPF = &myCurrentPFMask[hpos];
             uInt8* mP0 = &myCurrentP0Mask[hpos];

             while(mfp < ending)
             {
               if(0 /* disabled fast path */)
               {
                 *(uInt32*)mfp = (myPF & *mPF) ? myColor[MYCOLUPF] : myColor[MYCOLUBK];
                 mPF += 4; mP0 += 4; mfp += 4;
               }
               else
               {
                 *mfp = (myCurrentGRP0 & *mP0) ? 
                       myColor[MYCOLUP0] : ((myPF & *mPF) ? myColor[MYCOLUPF] : myColor[MYCOLUBK]);
#ifndef COLLISIONS_OFF
                 if((myPF & *mPF) && (myCurrentGRP0 & *mP0))
                   myCollision |= ourCollisionTable[myPFBit | myP0Bit];
#endif
                 ++mPF; ++mP0; ++mfp;
               }
             }
           }
           else // Need to do this the hard way...
           {             
             uInt32*mPF = &myCurrentPFMask[hpos];
             uInt8* mP0 = &myCurrentP0Mask[hpos];
             for(; mfp < ending; ++mfp)
             {
               uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
               if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
               HANDLE_COLOR_AND_COLLISIONS;
             }
           }
    }
    break;
        
    case (myPFBit | myP1Bit): // Playfield and Player 1 are enabled 
    {
           if (myPlayfieldPriorityAndScore & PriorityBit) // Priority set
           {
             uInt32* mPF = &myCurrentPFMask[hpos];
             uInt8* mP1 = &myCurrentP1Mask[hpos];

             while(mfp < ending)
             {
               if(0 /* disabled fast path */)
               {
                 *(uInt32*)mfp = (myPF & *mPF) ? myColor[MYCOLUPF] : myColor[MYCOLUBK];
                 mPF += 4; mP1 += 4; mfp += 4;
               }
               else
               {
                 *mfp = (myPF & *mPF) ? myColor[MYCOLUPF] : 
                     ((myCurrentGRP1 & *mP1) ? myColor[MYCOLUP1] : myColor[MYCOLUBK]);
#ifndef COLLISIONS_OFF
                 if((myPF & *mPF) && (myCurrentGRP1 & *mP1))
                   myCollision |= ourCollisionTable[myPFBit | myP1Bit];
#endif
                 ++mPF; ++mP1; ++mfp;
               }
             }
           }
           else if (!(myPlayfieldPriorityAndScore & ScoreBit)) // Priority not set and Score not set
           {
             uInt32* mPF = &myCurrentPFMask[hpos];
             uInt8* mP1 = &myCurrentP1Mask[hpos];

             while(mfp < ending)
             {
               if(0 /* disabled fast path */)
               {
                 *(uInt32*)mfp = (myPF & *mPF) ? myColor[MYCOLUPF] : myColor[MYCOLUBK];
                 mPF += 4; mP1 += 4; mfp += 4;
               }
               else
               {
                 *mfp = (myCurrentGRP1 & *mP1) ? 
                       myColor[MYCOLUP1] : ((myPF & *mPF) ? myColor[MYCOLUPF] : myColor[MYCOLUBK]);
#ifndef COLLISIONS_OFF
                 if((myPF & *mPF) && (myCurrentGRP1 & *mP1))
                   myCollision |= ourCollisionTable[myPFBit | myP1Bit];
#endif
                 ++mPF; ++mP1; ++mfp;
               }
             }
           }
           else // Need to do this the hard way...
           {
             uInt32*mPF = &myCurrentPFMask[hpos];
             uInt8* mP1 = &myCurrentP1Mask[hpos];
             for(; mfp < ending; ++mfp)
             {
               uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
               if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
               HANDLE_COLOR_AND_COLLISIONS;
             }
           }
    }
    break;
        
    case (myPFBit | myP1Bit | myM0Bit): // Playfield and Player 1 and Missile 0 are enabled 
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           if(*mM0++)                        enabled |= myM0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myPFBit | myP0Bit | myM0Bit): // Playfield and Player 0 and Missile 0 are enabled 
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           if(*mM0++)                        enabled |= myM0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myPFBit | myM1Bit | myM0Bit): // Playfield, Missile 1, Missile 0
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(*mM1++)                        enabled |= myM1Bit;
           if(*mM0++)                        enabled |= myM0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myPFBit | myM1Bit): // Playfield, Missile 1
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(*mM1++)                        enabled |= myM1Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myPFBit | myM1Bit | myP1Bit): // Playfield, Missile 1, Player 1
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(*mM1++)                        enabled |= myM1Bit;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
    
    case (myPFBit | myM1Bit | myP0Bit): // Playfield, Missile 1, Player 0
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(*mM1++)                        enabled |= myM1Bit;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myPFBit | myM1Bit | myM0Bit | myP0Bit): // Playfield, Missile 0, Missile 1, Player 1
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(*mM1++)                        enabled |= myM1Bit;
           if(*mM0++)                        enabled |= myM0Bit;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myPFBit | myM1Bit | myM0Bit | myP1Bit): // Playfield, Missile 0, Missile 1, Player 1
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(*mM1++)                        enabled |= myM1Bit;
           if(*mM0++)                        enabled |= myM0Bit;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;

    case (myP0Bit | myP1Bit | myM1Bit | myM0Bit): // Player 0, Player 1, Missile 0, Missile 1
    {
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = myPlayfieldPriorityAndScore;
           if(*mM1++)                        enabled |= myM1Bit;
           if(*mM0++)                        enabled |= myM0Bit;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;

    case (myBLBit): // Ball is enabled (Official Frogger)
    {
         uInt8* mBL = &myCurrentBLMask[hpos];

         while(mfp < ending)
         {
           if(0 /* disabled fast path */)
           {
             *(uInt32*)mfp = myColor[MYCOLUBK];
             mBL += 4; mfp += 4;
           }
           else
           {
             *mfp = *mBL ? myColor[MYCOLUPF] : myColor[MYCOLUBK];
             ++mBL; ++mfp;
           }
         }
    }
    break;
        
    case (myBLBit | myP0Bit): // Ball + Player 0 (freeway)
    {
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = myPlayfieldPriorityAndScore;
           if(*mBL++)                        enabled |= myBLBit;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myBLBit | myP0Bit | myM0Bit): // Ball plus Player 0 and Missile 0 are enabled
    {
         uInt8* mBL = &myCurrentBLMask[hpos];
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = myPlayfieldPriorityAndScore;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           if(*mM0++)                        enabled |= myM0Bit;
           if(*mBL++)                        enabled |= myBLBit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myBLBit | myM0Bit): // Ball and Missle 0 are enabled
    {
           if (myPlayfieldPriorityAndScore & PriorityBit) // Priority set
           {
             uInt8* mBL = &myCurrentBLMask[hpos];
             uInt8* mM0 = &myCurrentM0Mask[hpos];

             while(mfp < ending)
             {
               if(0 /* disabled fast path */)
               {
                 *(uInt32*)mfp = myColor[MYCOLUBK];
                 mBL += 4; mM0 += 4; mfp += 4;
               }
               else
               {
                 *mfp = (*mBL ? myColor[MYCOLUPF] : (*mM0 ? myColor[MYCOLUP0] : myColor[MYCOLUBK]));
#ifndef COLLISIONS_OFF
                 if(*mBL && *mM0)
                   myCollision |= ourCollisionTable[myBLBit | myM0Bit];
#endif
                 ++mBL; ++mM0; ++mfp;
               }
             }
           }
           else if (!(myPlayfieldPriorityAndScore & ScoreBit)) // Priority not set and Score not set
           {
             uInt8* mBL = &myCurrentBLMask[hpos];
             uInt8* mM0 = &myCurrentM0Mask[hpos];

             while(mfp < ending)
             {
               if(0 /* disabled fast path */)
               {
                 *(uInt32*)mfp = myColor[MYCOLUBK];
                 mBL += 4; mM0 += 4; mfp += 4;
               }
               else
               {
                 *mfp = (*mM0 ? myColor[MYCOLUP0] : (*mBL ? myColor[MYCOLUPF] : myColor[MYCOLUBK]));
#ifndef COLLISIONS_OFF
                 if(*mBL && *mM0)
                   myCollision |= ourCollisionTable[myBLBit | myM0Bit];
#endif
                 ++mBL; ++mM0; ++mfp;
               }
             }
           }
           else // Need to do this the hard way...
           {
             uInt32*mPF = &myCurrentPFMask[hpos];
             uInt8* mM0 = &myCurrentM0Mask[hpos];
             uInt8* mBL = &myCurrentBLMask[hpos];
             uInt8 meo1 = (myEnabledObjects & myBLBit) ? 0xFF:0x00;
             uInt8 meo2 = (myEnabledObjects & myM0Bit) ? 0xFF:0x00;
             for(; mfp < ending; ++mfp)
             {
               uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
               if(meo1 & *mBL++)                enabled |= myBLBit;
               if(meo2 & *mM0++)                enabled |= myM0Bit;
               HANDLE_COLOR_AND_COLLISIONS;
             }
           }
    }
    break;
        
    case (myBLBit | myM1Bit): // Ball and Missle 1 are enabled
    {
           if (myPlayfieldPriorityAndScore & PriorityBit) // Priority set
           {
             uInt8* mBL = &myCurrentBLMask[hpos];
             uInt8* mM1 = &myCurrentM1Mask[hpos];

             while(mfp < ending)
             {
               if(0 /* disabled fast path */)
               {
                 *(uInt32*)mfp = myColor[MYCOLUBK];
                 mBL += 4; mM1 += 4; mfp += 4;
               }
               else
               {
                 *mfp = (*mBL ? myColor[MYCOLUPF] : (*mM1 ? myColor[MYCOLUP1] : myColor[MYCOLUBK]));
#ifndef COLLISIONS_OFF
                 if(*mBL && *mM1)
                   myCollision |= ourCollisionTable[myBLBit | myM1Bit];
#endif
                 ++mBL; ++mM1; ++mfp;
               }
             }
           }
           else if (!(myPlayfieldPriorityAndScore & ScoreBit)) // Priority not set and Score not set
           {
             uInt8* mBL = &myCurrentBLMask[hpos];
             uInt8* mM1 = &myCurrentM1Mask[hpos];

             while(mfp < ending)
             {
               if(0 /* disabled fast path */)
               {
                 *(uInt32*)mfp = myColor[MYCOLUBK];
                 mBL += 4; mM1 += 4; mfp += 4;
               }
               else
               {
                 *mfp = (*mM1 ? myColor[MYCOLUP1] : (*mBL ? myColor[MYCOLUPF] : myColor[MYCOLUBK]));
#ifndef COLLISIONS_OFF
                 if(*mBL && *mM1)
                   myCollision |= ourCollisionTable[myBLBit | myM1Bit];
#endif
                 ++mBL; ++mM1; ++mfp;
               }
             }
           }
           else // Need to do this the hard way...
           {
             uInt32*mPF = &myCurrentPFMask[hpos];
             uInt8* mM1 = &myCurrentM1Mask[hpos];
             uInt8* mBL = &myCurrentBLMask[hpos];
             uInt8 meo1 = (myEnabledObjects & myBLBit) ? 0xFF:0x00;
             uInt8 meo2 = (myEnabledObjects & myM1Bit) ? 0xFF:0x00;
             for(; mfp < ending; ++mfp)
             {
               uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
               if(meo1 & *mBL++)                enabled |= myBLBit;
               if(meo2 & *mM1++)                enabled |= myM1Bit;
               HANDLE_COLOR_AND_COLLISIONS;
             }
           }
    }
    break;
        
    case (myBLBit | myP1Bit): // Ball and Player 1 are enabled
    {
           if (myPlayfieldPriorityAndScore & PriorityBit) // Priority set
           {
             uInt8* mBL = &myCurrentBLMask[hpos];
             uInt8* mP1 = &myCurrentP1Mask[hpos];

             while(mfp < ending)
             {
               if(0 /* disabled fast path */)
               {
                 *(uInt32*)mfp = myColor[MYCOLUBK];
                 mBL += 4; mP1 += 4; mfp += 4;
               }
               else
               {
                 *mfp = *mBL ? myColor[MYCOLUPF] : 
                     ((myCurrentGRP1 & *mP1) ? myColor[MYCOLUP1] : myColor[MYCOLUBK]);
#ifndef COLLISIONS_OFF
                 if(*mBL && (myCurrentGRP1 & *mP1))
                   myCollision |= ourCollisionTable[myBLBit | myP1Bit];
#endif
                 ++mBL; ++mP1; ++mfp;
               }
             }
           }
           else if (!(myPlayfieldPriorityAndScore & ScoreBit)) // Priority not set and Score not set
           {
             uInt8* mBL = &myCurrentBLMask[hpos];
             uInt8* mP1 = &myCurrentP1Mask[hpos];

             while(mfp < ending)
             {
               if(0 /* disabled fast path */)
               {
                 *(uInt32*)mfp = myColor[MYCOLUBK];
                 mBL += 4; mP1 += 4; mfp += 4;
               }
               else
               {
                 *mfp = (myCurrentGRP1 & *mP1) ? myColor[MYCOLUP1] : 
                     (*mBL ? myColor[MYCOLUPF] : myColor[MYCOLUBK]);
#ifndef COLLISIONS_OFF
                 if(*mBL && (myCurrentGRP1 & *mP1))
                   myCollision |= ourCollisionTable[myBLBit | myP1Bit];
#endif
                 ++mBL; ++mP1; ++mfp;
               }
             }
           }
           else // Need to do this the hard way...
           {
             uInt32*mPF = &myCurrentPFMask[hpos];
             uInt8* mP1 = &myCurrentP1Mask[hpos];
             uInt8* mBL = &myCurrentBLMask[hpos];
             uInt8 meo1 = (myEnabledObjects & myBLBit) ? 0xFF:0x00;
             for(; mfp < ending; ++mfp)
             {
               uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
               if(meo1 & *mBL++)                enabled |= myBLBit;
               if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
               HANDLE_COLOR_AND_COLLISIONS;
             }
           }
    }
    break;
        
    case (myBLBit | myP1Bit | myM1Bit): // Ball, Player 1, Missile 1
    {
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = myPlayfieldPriorityAndScore;
           if(*mBL++)                        enabled |= myBLBit;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           if(*mM1++)                        enabled |= myM1Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;

    case (myP0Bit | myM1Bit): // Player 0, Missile 1
    {
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = myPlayfieldPriorityAndScore;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           if(*mM1++)                        enabled |= myM1Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
    
    case (myBLBit | myP0Bit | myM1Bit): // Ball, Player 0, Missile 1
    {
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = myPlayfieldPriorityAndScore;
           if(*mBL++)                        enabled |= myBLBit;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           if(*mM1++)                        enabled |= myM1Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;

    case (myP0Bit | myM0Bit | myM1Bit): // Player 0, Missile 0, Missile 1
    {
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = myPlayfieldPriorityAndScore;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           if(*mM0++)                        enabled |= myM0Bit;
           if(*mM1++)                        enabled |= myM1Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
    case (myBLBit | myP0Bit | myM0Bit | myM1Bit): // Ball, Player 0, Missile 0, Missile 1
    {
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = myPlayfieldPriorityAndScore;
           if(*mBL++)                        enabled |= myBLBit;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           if(*mM0++)                        enabled |= myM0Bit;
           if(*mM1++)                        enabled |= myM1Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;

    case (myP1Bit | myM0Bit ): // Player 1, Missile 0
    {
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = myPlayfieldPriorityAndScore;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           if(*mM0++)                        enabled |= myM0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;

    case (myBLBit | myP1Bit | myM0Bit | myM1Bit): // Ball, Player 1, Missile 0, Missile 1
    {
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = myPlayfieldPriorityAndScore;
           if(*mBL++)                        enabled |= myBLBit;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           if(*mM0++)                        enabled |= myM0Bit;
           if(*mM1++)                        enabled |= myM1Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;

    case (myBLBit | myP1Bit | myP0Bit | myM0Bit): // Ball, Player 0, Player 1, Missile 0
    {
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = myPlayfieldPriorityAndScore;
           if(*mBL++)                        enabled |= myBLBit;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           if(*mM0++)                        enabled |= myM0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;

    case (myBLBit | myP1Bit | myM0Bit): // Ball, Player 1, Missile 0
    {
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = myPlayfieldPriorityAndScore;
           if(*mBL++)                        enabled |= myBLBit;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           if(*mM0++)                        enabled |= myM0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;

    case (myBLBit | myM0Bit | myM1Bit): // Ball + Missile 0/1
    {
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = myPlayfieldPriorityAndScore;
           if(*mBL++)                enabled |= myBLBit;
           if(*mM1++)                enabled |= myM1Bit;
           if(*mM0++)                enabled |= myM0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myP0Bit): // Player 0 is enabled
    {
         uInt8* mP0 = &myCurrentP0Mask[hpos];

         while(mfp < ending)
         {
           if(0 /* disabled fast path */)
           {
             *(uInt32*)mfp = myColor[MYCOLUBK];
             mP0 += 4; mfp += 4;
           }
           else
           {
             *mfp = (myCurrentGRP0 & *mP0) ? myColor[MYCOLUP0] : myColor[MYCOLUBK];
             ++mP0; ++mfp;
           }
         }
    }
    break;
        
    case (myP1Bit): // Player 1 is enabled
    {
         uInt8* mP1 = &myCurrentP1Mask[hpos];

         while(mfp < ending)
         {
           if(0 /* disabled fast path */)
           {
             *(uInt32*)mfp = myColor[MYCOLUBK];
             mP1 += 4; mfp += 4;
           }
           else
           {
             *mfp = (myCurrentGRP1 & *mP1) ? myColor[MYCOLUP1] : myColor[MYCOLUBK];
             ++mP1; ++mfp;
           }
         }
    }
    break;
        
    case (myM0Bit): // Missile 0 is enabled
    {
         uInt8* mM0 = &myCurrentM0Mask[hpos];

         while(mfp < ending)
         {
           if(0 /* disabled fast path */)
           {
             *(uInt32*)mfp = myColor[MYCOLUBK];
             mM0 += 4; mfp += 4;
           }
           else
           {
             *mfp = *mM0 ? myColor[MYCOLUP0] : myColor[MYCOLUBK];
             ++mM0; ++mfp;
           }
         }
    }
    break;
        
    case (myM1Bit): // Missile 1 is enabled
    {
         uInt8* mM1 = &myCurrentM1Mask[hpos];

         while(mfp < ending)
         {
           if(0 /* disabled fast path */)
           {
             *(uInt32*)mfp = myColor[MYCOLUBK];
             mM1 += 4; mfp += 4;
           }
           else
           {
             *mfp = *mM1 ? myColor[MYCOLUP1] : myColor[MYCOLUBK];
             ++mM1; ++mfp;
           }
         }
    }
    break;
        
    case (myP0Bit | myM0Bit): // Player 0 and Missile 0 are enabled (Super Breakout!!)
    {
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = myPlayfieldPriorityAndScore;
           if(myCurrentGRP0 & *mP0++)        enabled |= myP0Bit;
           if(*mM0++)                        enabled |= myM0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    case (myM0Bit | myM1Bit): // Missile 0 and 1 is enabled
    {
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];

         while(mfp < ending)
         {
           if(0 /* disabled fast path */)
           {
             *(uInt32*)mfp = myColor[MYCOLUBK];
             mM0 += 4; mM1 += 4; mfp += 4;
           }
           else
           {
             *mfp = *mM0 ? myColor[MYCOLUP0] : (*mM1 ? myColor[MYCOLUP1] : myColor[MYCOLUBK]);
#ifndef COLLISIONS_OFF
             if(*mM0 && *mM1)
               myCollision |= ourCollisionTable[myM0Bit | myM1Bit];
#endif
             ++mM0; ++mM1; ++mfp;
           }
         }
    }
    break;
        
    case (myP1Bit | myM1Bit): // Player 1, Missile 1
    {
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = myPlayfieldPriorityAndScore;
           if(myCurrentGRP1 & *mP1++)        enabled |= myP1Bit;
           if(*mM1++)                        enabled |= myM1Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;
        
    default:
    {
         uInt32*mPF = &myCurrentPFMask[hpos];
         uInt8* mP1 = &myCurrentP1Mask[hpos];
         uInt8* mP0 = &myCurrentP0Mask[hpos];
         uInt8* mM0 = &myCurrentM0Mask[hpos];
         uInt8* mM1 = &myCurrentM1Mask[hpos];
         uInt8* mBL = &myCurrentBLMask[hpos];
         uInt8 meo1 = (myEnabledObjects & myBLBit) ? 0xFF:0x00;
         uInt8 meo2 = (myEnabledObjects & myM1Bit) ? 0xFF:0x00;
         uInt8 meo3 = (myEnabledObjects & myM0Bit) ? 0xFF:0x00;
         for(; mfp < ending; ++mfp)
         {
           uInt8 enabled = (myPF & *mPF++) ? (myPFBit| myPlayfieldPriorityAndScore) : myPlayfieldPriorityAndScore;
           if(meo1 & *mBL++)                enabled |= myBLBit;
           if(myCurrentGRP1 & *mP1++)       enabled |= myP1Bit;
           if(meo2 & *mM1++)                enabled |= myM1Bit;
           if(myCurrentGRP0 & *mP0++)       enabled |= myP0Bit;
           if(meo3 & *mM0++)                enabled |= myM0Bit;
           HANDLE_COLOR_AND_COLLISIONS;
         }
    }
    break;

