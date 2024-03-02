/** EMULib Emulation Library *********************************/
/**                                                         **/
/**                         AY8910.c                        **/
/**                                                         **/
/** This file contains emulation for the AY8910 sound chip  **/
/** produced by General Instruments, Yamaha, etc. See       **/
/** AY8910.h for declarations.                              **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1996-2021                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/

#include "AY8910.h"
#include "Sound.h"
#include <string.h>

static const unsigned char Envelopes[16][32] =
{
  { 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0 },
  { 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 },
  { 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15 },
  { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 },
  { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15 },
  { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0 },
  { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }
};

static const int Volumes[16] =
{ 0,1,2,4,6,8,11,16,23,32,45,64,90,128,180,255 };
//{ 0,16,33,50,67,84,101,118,135,152,169,186,203,220,237,254 };

/** Reset8910() **********************************************/
/** Reset the sound chip and use sound channels from the    **/
/** one given in First.                                     **/
/*************************************************************/
void Reset8910(register AY8910 *D,int ClockHz,int First)
{
  static byte RegInit[16] =
  {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFD,
    0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00
  };
  int J;

  /* Reset state */
  memcpy(D->R,RegInit,sizeof(D->R));
  D->EPhase  = 0;
  D->Clock   = ClockHz>>4;
  D->First   = First;
  D->Sync    = AY8910_ASYNC;
  D->Changed = 0x00;
  D->EPeriod = -1;
  D->ECount  = 0;
  D->Latch   = 0x00;

  /* Set sound types */
  SetSound(0+First,SND_MELODIC);
  SetSound(1+First,SND_MELODIC);
  SetSound(2+First,SND_MELODIC);
  SetSound(3+First,SND_NOISE);
  SetSound(4+First,SND_NOISE);
  SetSound(5+First,SND_NOISE);

  /* Configure noise generator */
  SetNoise(0x10000,16,14);

  /* Silence all channels */
  for(J=0;J<AY8910_CHANNELS;J++)
  {
    D->Freq[J]=D->Volume[J]=0;
    Sound(J+First,0,0);
  }
}

/** WrCtrl8910() *********************************************/
/** Write a value V to the PSG Control Port.                **/
/*************************************************************/
void WrCtrl8910(AY8910 *D,byte V)
{
  D->Latch=V&0x0F;
}

/** WrData8910() *********************************************/
/** Write a value V to the PSG Data Port.                   **/
/*************************************************************/
void WrData8910(AY8910 *D,byte V)
{
  Write8910(D,D->Latch,V);
}

/** RdData8910() *********************************************/
/** Read a value from the PSG Data Port.                    **/
/*************************************************************/
byte RdData8910(AY8910 *D)
{
  return(D->R[D->Latch]);
}

/** Write8910() **********************************************/
/** Call this function to output a value V into the sound   **/
/** chip.                                                   **/
/*************************************************************/
void Write8910(register AY8910 *D,register byte R,register byte V)
{
  register int J;

  switch(R)
  {
    case 1:
    case 3:
    case 5:
      V&=0x0F;
      /* Fall through */
    case 0:
    case 2:
    case 4:
      if(V!=D->R[R])
      {
        /* Compute changed channels mask */
        D->Changed|=(1<<(R>>1))&~D->R[7];
        /* Write value */
        D->R[R]=V;
      }
      break;

    case 6:
      V&=0x1F;
      if(V!=D->R[R])
      {
        /* Compute changed channels mask */
        D->Changed|=0x38&~D->R[7];
        /* Write value */
        D->R[R]=V;
      }
      break;

    case 7:
      /* Compute changed channels mask */
      D->Changed|=(V^D->R[R])&0x3F;
      /* Write value */
      D->R[R]=V;
      break;
      
    case 8:
    case 9:
    case 10:
      V&=0x1F;
      if(V!=D->R[R])
      {
        /* Compute changed channels mask */
        D->Changed|=(0x09<<(R-8))&~D->R[7];
        /* Write value */
        D->R[R]=V;
      }
      break;

    case 11:
    case 12:
      if(V!=D->R[R])
      {
        /* Will need to recompute EPeriod next time */
        D->EPeriod = -1;
        /* Write value */
        D->R[R]=V;
        /* No channels changed */
      }
      return;

    case 13:
      /* Write value */
      D->R[R]=V&=0x0F;
      /* Reset envelope */
      D->ECount = 0;
      D->EPhase = 0;
      /* Compute changed channels mask */
      for(J=0;J<AY8910_CHANNELS/2;++J)
        if(D->R[J+8]&0x10) D->Changed|=(0x09<<J)&~D->R[7];
      break;

    case 14:
    case 15:
      /* Write value */
      D->R[R]=V;
      /* No channels changed */
      return;

    default:
      /* Wrong register, do nothing */
      return;
  }

  /* For asynchronous mode, make Sound() calls right away */
  if(!D->Sync&&D->Changed) Sync8910(D,AY8910_FLUSH);
}

/** Loop8910() ***********************************************/
/** Call this function periodically to update volume        **/
/** envelopes. Use uSec to pass the time since the last     **/
/** call of Loop8910() in microseconds.                     **/
/*************************************************************/
void Loop8910(register AY8910 *D,int uSec)
{
  register int J;

  /* If envelope period was updated... */
  if(D->EPeriod<0)
  {
    /* Compute envelope period (why not <<4?) */
    J=((int)D->R[12]<<8)+D->R[11];
    D->EPeriod=(int)(1000000L*(J? J:0x10000)/D->Clock);
  }

  /* Exit if no envelope running */
  if(!D->EPeriod) return;

  /* Count microseconds */
  D->ECount += uSec;
  if(D->ECount<D->EPeriod) return;

  /* Count steps */
  J = D->ECount/D->EPeriod;
  D->ECount -= J*D->EPeriod;

  /* Count phases */
  D->EPhase += J;
  if(D->EPhase>31)
    D->EPhase = (D->R[13]&0x09)==0x08? (D->EPhase&0x1F):31;

  /* Compute changed channels mask */
  for(J=0;J<AY8910_CHANNELS/2;++J)
    if(D->R[J+8]&0x10) D->Changed|=(0x09<<J)&~D->R[7];

  /* For asynchronous mode, make Sound() calls right away */
  if(!D->Sync&&D->Changed) Sync8910(D,AY8910_FLUSH);
}

/** Sync8910() ***********************************************/
/** Flush all accumulated changes by issuing Sound() calls  **/
/** and set the synchronization on/off. The second argument **/
/** should be AY8910_SYNC/AY8910_ASYNC to set/reset sync,   **/
/** or AY8910_FLUSH to leave sync mode as it is. To emulate **/
/** noise channels with MIDI drums, OR second argument with **/
/** AY8910_DRUMS.                                           **/
/*************************************************************/
void Sync8910(register AY8910 *D,register byte Sync)
{
  register int J,I,K;
  int Freq,Volume,Drums;

  /* Update sync/async mode */
  I = Sync&~AY8910_DRUMS;
  if(I!=AY8910_FLUSH) D->Sync=I;

  /* If hitting drums, always compute noise channels */
  I = D->Changed|(Sync&AY8910_DRUMS? 0x38:0x00);

  /* For every changed channel... */
  for(J=0,Drums=0;I&&(J<AY8910_CHANNELS);++J,I>>=1)
  {
    if(I&1)
    {
      if(D->R[7]&(1<<J)) Freq=Volume=0;
      else
      {
        /* If it is a melodic channel... */
        if(J<AY8910_CHANNELS/2)
        {
          /* Compute melodic channel volume */
          K = D->R[J+8]&0x1F;
          K = Volumes[K&0x10? Envelopes[D->R[13]&0x0F][D->EPhase]:(K&0x0F)];
          Volume = K;
          /* Compute melodic channel frequency */
          K = J<<1;
          K = ((int)(D->R[K+1]&0x0F)<<8)+D->R[K];
          // Eggerland Mystery 1 (MSX) uses 0 to silence sound
          //Freq = D->Clock/(K? K:0x1000);
          Freq = K? D->Clock/K:0;
        }
        else
        {
          /* Compute noise channel volume */
          K = D->R[J+5]&0x1F;
          K = Volumes[K&0x10? Envelopes[D->R[13]&0x0F][D->EPhase]:(K&0x0F)];
          Volume = (K+1)>>1;
          Drums += Volume;
          /* Compute noise channel frequency */
          /* Shouldn't do <<2, but need to keep frequency down */
          K = D->R[6]&0x1F;
          Freq = D->Clock/((K&0x1F? (K&0x1F):0x20)<<2);
        }
      }

      /* Play sound */
      Sound(D->First+J,Freq,Volume);
    }
  }

  /* Hit MIDI drums for noise channels, if requested */
  if(Drums&&(Sync&AY8910_DRUMS)) Drum(DRM_MIDI|28,Drums>255? 255:Drums);

  /* Done with all the changes */
  D->Changed=0x00;
}
