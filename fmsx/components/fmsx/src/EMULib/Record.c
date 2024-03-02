/** EMULib Emulation Library *********************************/
/**                                                         **/
/**                        Record.c                         **/
/**                                                         **/
/** This file contains routines for gameplay recording and  **/
/** replay.                                                 **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 2013-2021                 **/
/**     The contents of this file are property of Marat     **/
/**     Fayzullin and should only be used as agreed with    **/
/**     him. The file is confidential. Absolutely no        **/
/**     distribution allowed.                               **/
/*************************************************************/
#include "Record.h"
#include "Console.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#ifndef DEFINE_ONCE
#define DEFINE_ONCE

#define RPL_RECSIZE  (RPL_STEP+1)
#define RPL_BUFSIZE  64
#define RPL_STEP     10
#define RPL_SIGNSIZE 12

#define READ_INT(Buf) \
  ((Buf)[0]+((int)(Buf)[1]<<8)+((int)(Buf)[2]<<16)+((int)(Buf)[3]<<24))

#define WRITE_INT(Buf,V) {   \
  (Buf)[0] = ((V)&0xFF);       \
  (Buf)[1] = (((V)>>8)&0xFF);  \
  (Buf)[2] = (((V)>>16)&0xFF); \
  (Buf)[3] = (((V)>>24)&0xFF); \
}

typedef struct
{
  unsigned char *State;
  unsigned int StateSize;
  unsigned int JoyState[RPL_RECSIZE];
  unsigned int Count[RPL_RECSIZE];
  unsigned char KeyState[RPL_RECSIZE][16];
} RPLState;

static RPLState RPLData[RPL_BUFSIZE] = {{0}};
static unsigned int StateSize = 0;
static unsigned int TimeLeft;
static int RPLRCount = -1;
static int RPLWCount = -1;
static int RPLUCount = -1;
static int RPtr1,RPtr2;
static int WPtr1,WPtr2;

static unsigned int (*SaveState)(unsigned char *,unsigned int) = 0;
static unsigned int (*LoadState)(unsigned char *,unsigned int) = 0;

/** RPLInit() ************************************************/
/** Initialize record/relay subsystem.                      **/
/*************************************************************/
void RPLInit(unsigned int (*SaveHandler)(unsigned char *,unsigned int),unsigned int (*LoadHandler)(unsigned char *,unsigned int),unsigned int MaxSize)
{
  RPLTrash();
  SaveState = SaveHandler;
  LoadState = LoadHandler;
  StateSize = MaxSize;
}

/** RPLTrash() ***********************************************/
/** Free all record/replay resources.                       **/
/*************************************************************/
void RPLTrash(void)
{
  /* Free recording buffers */
  RPLRecord(RPL_RESET);
  /* Disable both recording and playback */
  RPLRecord(RPL_OFF);
  RPLPlay(RPL_OFF);
}

/** RPLRecord() **********************************************/
/** Record emulation state and joystick input for replaying **/
/** it back later.                                          **/
/*************************************************************/
int RPLRecord(unsigned int Cmd) { return(RPLRecordKeys(Cmd,0,0)); }

/** RPLRecordKeys() ******************************************/
/** Record emulation state, keys, and joystick input for    **/
/** replaying them back later.                              **/
/*************************************************************/
int RPLRecordKeys(unsigned int Cmd,const unsigned char *Keys,unsigned int KeySize)
{
  int J;

  /* Insure that recording is initialized */
  if(!SaveState || !LoadState || !StateSize) return(0);

  /* Toggle recording as necessary */
  if(Cmd==RPL_TOGGLE) Cmd=RPLWCount<0? RPL_ON:RPL_OFF;

  /* Reset recording as necessary */
  if(Cmd==RPL_RESET) { RPLWCount=-1;Cmd=RPL_ON; }

  /* Act on recording command */
  switch(Cmd)
  {
    case RPL_ON:
      /* If recording already on, do nothing */
      if(RPLWCount>=0) return(1);
      /* Clear current records */
      for(J=0;J<RPL_BUFSIZE;++J)
      {
        if(RPLData[J].State) { free(RPLData[J].State);RPLData[J].State=0; }
        RPLData[J].StateSize = 0;
        RPLData[J].Count[0]  = 0;
      }
      /* Reset recording and replay */
      RPLUCount = -1;
      RPLRCount = -1;
      RPLWCount = 0;
      RPtr1 = 0;
      WPtr1 = 0;
      RPtr2 = -1;
      WPtr2 = 0;
      /* Done */
      return(1);

    case RPL_OFF:
      /* Stop recording */
      RPLWCount=-1;
      return(0);

    case RPL_QUERY:
      /* Return current recording state */
      return(RPLWCount>=0);
  }

  /* If not recording or replaying, return immediately */
  if((RPLWCount<0) || (RPLRCount>=0)) return(0);

  /* If not creating a new state record yet... */
  if((++RPLWCount<RPL_STEP) && RPLData[WPtr1].Count[WPtr2])
  {
    /* If no keys, no changes to them */
    if(!Keys) J=0;
    else
    {
      /* Make sure we do not exceed KeyState[] size */
      if(KeySize>sizeof(RPLData[0].KeyState[0]))
        KeySize = sizeof(RPLData[0].KeyState[0]);
      /* Detect changes in keyboard state */
      J = memcmp(Keys,RPLData[WPtr1].KeyState[WPtr2],KeySize);
    }

    /* If same joystick and keyboard states, just increment counter */
    if(!J && (RPLData[WPtr1].JoyState[WPtr2]==Cmd))
    {
      ++RPLData[WPtr1].Count[WPtr2];
      return(1);
    }

    /* If empty input record or created new record... */
    if(!RPLData[WPtr1].Count[WPtr2] || (++WPtr2<RPL_RECSIZE))
    {
      /* Start filling up the new input record */
      RPLData[WPtr1].JoyState[WPtr2] = Cmd;
      RPLData[WPtr1].Count[WPtr2]    = 1;

      /* Save keyboard state if supplied */
      if(Keys) memcpy(RPLData[WPtr1].KeyState[WPtr2],Keys,KeySize);

      /* Terminate new input record */
      if(WPtr2<RPL_RECSIZE-1) RPLData[WPtr1].Count[WPtr2+1]=0;

      return(1);
    }
  }

  /*
   * Creating a new state record
   */

  /* Go to the next state slot */
  if(WPtr2 || RPLData[WPtr1].Count[WPtr2])
  {
    WPtr1 = (WPtr1+1)&(RPL_BUFSIZE-1);
    WPtr2 = 0;

    /* Bump the replay position */
    if(RPtr1==WPtr1)
    {
      RPtr1 = ((RPtr1+1)&(RPL_BUFSIZE-1));
      RPtr2 = -1;
    }
  }

  /* Allocate memory for the state buffer, if needed */
  if(!RPLData[WPtr1].State)
    RPLData[WPtr1].State = malloc(StateSize);

  /* If there is a state buffer, save emulation state */
  RPLData[WPtr1].StateSize =
    RPLData[WPtr1].State? SaveState(RPLData[WPtr1].State,StateSize):0;

  /* Start a new input record */
  RPLData[WPtr1].JoyState[WPtr2] = Cmd;
  RPLData[WPtr1].Count[WPtr2]    = 1;

  /* Save keyboard state if supplied */
  if(Keys) memcpy(RPLData[WPtr1].KeyState[WPtr2],Keys,KeySize);

  /* Terminate new input record */
  if(WPtr2<RPL_RECSIZE-1) RPLData[WPtr1].Count[WPtr2+1]=0;

  /* Start counting until the next SaveState() */
  RPLWCount = 0;

  return(1);
}

/** RPLPlay() ************************************************/
/** Replay gameplay saved with RPLRecord().                 **/
/*************************************************************/
unsigned int RPLPlay(int Cmd) { return(RPLPlayKeys(Cmd,0,0)); }

/** RPLPlayKeys() ********************************************/
/** Replay gameplay saved with RPLRecordKeys().             **/
/*************************************************************/
unsigned int RPLPlayKeys(int Cmd,unsigned char *Keys,unsigned int KeySize)
{
  /* Insure that recording is initialized */
  if(!SaveState || !LoadState || !StateSize) return(Cmd==RPL_NEXT? RPL_ENDED:0);

  /* Toggle replay as necessary */
  if(Cmd==RPL_TOGGLE) Cmd=RPLRCount<0? RPL_ON:RPL_OFF;

  /* Reset replay as necessary */
  if(Cmd==RPL_RESET) { RPLRCount=-1;Cmd=RPL_ON; }

  /* Act on replay command */
  switch(Cmd)
  {
    case RPL_ON:
      /* If replay already on, do nothing */
      if(RPLRCount>=0) return(1);
      /* Look for the oldest valid state to replay */
      for(RPtr1=(WPtr1+1)&(RPL_BUFSIZE-1);RPtr1!=WPtr1;RPtr1=(RPtr1+1)&(RPL_BUFSIZE-1))
        if(RPLData[RPtr1].State && RPLData[RPtr1].StateSize && RPLData[RPtr1].Count[0])
        {
          /* State found, replay from that state */
          RPLRCount = 0;
          RPtr2     = -1;
          TimeLeft  = RPLCount();
          return(1);
        }
      /* State not found */
      return(0);

    case RPL_NEXT:
      /* This will play the next record */
      if(RPLRCount<0) return(RPL_ENDED);
      break;

    case RPL_OFF:
      /* If terminating playback while recording... */
      if((RPLRCount>=0) && (RPLWCount>=0))
      {
        /* Set recording to the spot where playback ends */
        WPtr1 = RPtr1;
        WPtr2 = RPtr2;

        /* Truncate last input record */
        RPLData[WPtr1].Count[WPtr2]-=RPLRCount;

        /* Terminate truncated input record */
        if(WPtr2<RPL_RECSIZE-1) RPLData[WPtr1].Count[WPtr2+1]=0;
      }
      /* Playback now off */
      RPLUCount = -1;
      RPLRCount = -1;
      RPLUCount = -1;
      return(0);

    case RPL_QUERY:
    default:
      /* Return current replay state */
      return(RPLRCount>=0);
  }

  /* If finished repeating previous joystick state... */
  if(!RPLRCount)
  {
    /* Stop at the current recording position */
    if((RPtr1==WPtr1) && (RPtr2==WPtr2)) { RPLPlay(RPL_OFF);return(RPL_ENDED); } 

    /* Go to the next input record inside a state slot */
    if((RPtr2<0) || (++RPtr2>=RPL_RECSIZE) || !RPLData[RPtr1].Count[RPtr2])
    {
      /* Go to the next state slot */
      RPtr1 = RPtr2>=0? ((RPtr1+1)&(RPL_BUFSIZE-1)) : RPtr1;

      /* Exit when done with replay */
      if(!RPLData[RPtr1].Count[0]) { RPLPlay(RPL_OFF);return(RPL_ENDED); }

      /* Load next emulation state, if present */
      if(RPLData[RPtr1].State && RPLData[RPtr1].StateSize)
        if(!LoadState(RPLData[RPtr1].State,RPLData[RPtr1].StateSize))
        { RPLPlay(RPL_OFF);return(RPL_ENDED); }

      /* Go to the first record */
      RPtr2 = 0;
    }

    /* New joystick state will be valid for this many frames */
    RPLRCount = RPLData[RPtr1].Count[RPtr2];
    if(!RPLRCount) { RPLPlay(RPL_OFF);return(RPL_ENDED); }
  }

  /* Load keyboard state if supplied */
  if(Keys)
  {
    if(KeySize>sizeof(RPLData[0].KeyState[0]))
      KeySize = sizeof(RPLData[0].KeyState[0]);
    memcpy(Keys,RPLData[RPtr1].KeyState[RPtr2],KeySize);
  }

  /* Return joystick state */
  --RPLRCount;
  if(TimeLeft) --TimeLeft;
  return(RPLData[RPtr1].JoyState[RPtr2]);
}

/** RPLCount() ***********************************************/
/** Compute the number of remaining replay records.         **/
/*************************************************************/
unsigned int RPLCount(void)
{
  unsigned int Count;
  int I,J;

  /* Playback must be on */
  if((RPLRCount<0) || (RPtr1<0)) return(0);

  /* Start with the current count */
  Count = RPLRCount;

  /* Go through all states, iterating over available records */
  for(J=RPtr1 ; (J!=WPtr1) && RPLData[J].Count[0] ; J=(J+1)&(RPL_BUFSIZE-1))
    for(I=0 ; (I<RPL_RECSIZE) && RPLData[J].Count[I] ; ++I)
      Count+=RPLData[J].Count[I];

  /* For the last state, only go until the record being written */
  if(J==WPtr1)
    for(I=0 ; (I<RPL_RECSIZE) && RPLData[J].Count[I] && (I<=WPtr2) ; ++I)
      Count+=RPLData[J].Count[I];

  /* Subtract current state records that have been already played */
  if(RPtr2>=0)
    for(I=0;I<=RPtr2;++I)
      Count-=RPLData[RPtr1].Count[I];

  /* Done */
  return(Count);
}

/** SaveRPL() ************************************************/
/** Save gameplay recording into given file.                **/
/*************************************************************/
int SaveRPL(const char *FileName)
{
  static unsigned char Header[16] = "RPL\032\001\0\0\0\0\0\0\0\0\0\0\0";
  unsigned char Buf[16];
  FILE *F;
  int J,K;

  /* Look for the oldest valid state to replay */
  for(J=(WPtr1+1)&(RPL_BUFSIZE-1);J!=WPtr1;J=(J+1)&(RPL_BUFSIZE-1))
    if(RPLData[J].State && RPLData[J].StateSize && RPLData[J].Count[0]) break;

  /* If state not found, drop out */
  if(J==WPtr1) return(0);

  /* Open file */
  F = fopen(FileName,"wb");
  if(!F) return(0);

  /* Fill and write header */
  WRITE_INT(Header+5,RPLData[J].StateSize);
  if(fwrite(Header,1,sizeof(Header),F)!=sizeof(Header))
  { fclose(F);unlink(FileName);return(0); }

  /* Write initial state */
  if(fwrite(RPLData[J].State,1,RPLData[J].StateSize,F)!=RPLData[J].StateSize)
  { fclose(F);unlink(FileName);return(0); }

  /* Write input records */
  for(;J!=WPtr1;J=(J+1)&(RPL_BUFSIZE-1))
    for(K=0;(K<RPL_RECSIZE)&&RPLData[J].Count[K];++K)
    {
      WRITE_INT(Buf,  RPLData[J].Count[K]);
      WRITE_INT(Buf+4,RPLData[J].JoyState[K]);

      if(fwrite(Buf,1,8,F)!=8)
      { fclose(F);unlink(FileName);return(0); }
    }

  /* Write terminating record */
  memset(Buf,0x00,8);
  if(fwrite(Buf,1,8,F)!=8)
  { fclose(F);unlink(FileName);return(0); }

  /* Done */
  fclose(F);
  return(1);
}

/** LoadRPL() ************************************************/
/** Load gameplay recording from given file.                **/
/*************************************************************/
int LoadRPL(const char *FileName)
{
  unsigned char Header[16];
  unsigned char Buf[16],*P;
  int J,K;
  FILE *F;

  /* Open file */
  F = fopen(FileName,"rb");
  if(!F) return(0);

  /* Read and verify header */
  if(fread(Header,1,sizeof(Header),F)!=sizeof(Header)) { fclose(F);return(0); }
  if(memcmp(Header,"RPL\032\001",5)) { fclose(F);return(0); }

  /* Allocate state buffer */
  J = READ_INT(Header+5);
  P = malloc(J);
  if(!P) { fclose(F);return(0); }

  /* Read state */
  if(fread(P,1,J,F)!=J) { fclose(F);free(P);return(0); }

  /* State loaded, move it in */
  RPLTrash();
  RPLData[0].State     = P;
  RPLData[0].StateSize = J;

  /* Read input records */
  for(RPtr1=J=K=0;(K>=0)&&(J<RPL_BUFSIZE);J=(J+1)&(RPL_BUFSIZE-1))
    for(K=0;K<RPL_RECSIZE;++K)
    {
      /* Read next input record from file */
      if(fread(Buf,1,8,F)!=8)
      {
        RPLData[J].Count[K]    = 0;
        RPLData[J].JoyState[K] = 0;
      }
      else
      {
        RPLData[J].Count[K]    = READ_INT(Buf);
        RPLData[J].JoyState[K] = READ_INT(Buf+4);
      }

      /* If read null-record, or incomplete record, finish */
      if(!RPLData[J].Count[K]) { K=-1;break; }
    }

  /* Done */
  WPtr1 = J;
  fclose(F);
  return(1);
}

/** RPLControls() ********************************************/
/** Let user browse through replay states with directional  **/
/** buttons: LEFT:REW, RIGHT:FWD, DOWN:STOP, UP:CONTINUE.   **/
/*************************************************************/
unsigned int RPLControls(unsigned int Buttons)
{
  int J;

  /* Do nothing when replay is inactive */
  if(RPLRCount<0) return(Buttons);

  /* If user controls counter has not expired... */
  if(RPLUCount)
  {
    /* Keep playing until controls counter runs out */
    if(RPLUCount>0) --RPLUCount;
    else if(Buttons&BTN_UP)  RPLUCount=4;
    else if(Buttons&~BTN_UP) RPLPlay(RPL_OFF);
    /* Swallowing UP button */
    return(Buttons&~BTN_UP);
  }

  /* Wait for a button to be pressed, then act on it */
  switch(WaitJoystick(BTN_ALL))
  {
    case BTN_LEFT:
      /* Go to the previous state as needed */
      J         = (RPtr1-1)&(RPL_BUFSIZE-1);
      RPtr1     = (J!=WPtr1)&&RPLData[J].Count[0]? J:RPtr1;
      RPtr2     = -1;
      RPLRCount = 0;
      TimeLeft  = RPLCount();
      RPLUCount = 4;
      break;
  
    case BTN_RIGHT:
      /* Go to the previous state as needed */
      J         = (RPtr1+1)&(RPL_BUFSIZE-1);
      RPtr1     = (RPtr1!=WPtr1)&&((J!=WPtr1)||(WPtr2>0))&&RPLData[J].Count[0]? J:RPtr1;
      RPtr2     = -1;
      RPLRCount = 0;
      TimeLeft  = RPLCount();
      RPLUCount = 4;
      break;
  
    case BTN_UP:
      /* Continue replay */
      RPLUCount = -1;
      break;
  
    default:
      /* Stop replay */
      RPLPlay(RPL_OFF);
      RPLUCount = -1;
      break;
  }

  /* Make sure all buttons are up */
  while(VideoImg&&(GetJoystick()&~(BTN_LEFT|BTN_RIGHT)));

  /* Swallowing all buttons */
  return(0);
}

#endif /* DEFINE_ONCE */

#if defined(BPP8) || defined(BPP16) || defined(BPP24) || defined(BPP32)
#define CLR_FG PIXEL(255,255,255)
#define CLR_BG PIXEL(0,0,0)

/** RPLShow() ************************************************/
/** Draw replay icon when active.                           **/
/*************************************************************/
void RPLShow(Image *Img,int X,int Y)
{
  pixel *P;
  int J,I;

  /* Draw nothing when record/replay is inactive */
  if((RPLWCount<0) && (RPLRCount<0)) return;

  /* Starting address to draw the arrow */
  P = (pixel *)Img->Data + Img->L*Y + X;

  /* If replay active... */
  if(RPLRCount>=0)
  {
    /* If replay is playing... */
    if(RPLUCount<0)
    {
      ShadowPrintXY(Img,"^ TO PAUSE",(Img->W-8*10)>>1,Img->H-Y-8,CLR_FG,CLR_BG);

      for(J=1;J<=RPL_SIGNSIZE/2;++J,P+=Img->L)
      {
        P[0] = CLR_BG;
        for(I=1;I<J;++I) P[I]=CLR_FG;
        P[J] = CLR_BG;
      }

      for(J-=2;J;--J,P+=Img->L)
      {
        P[0] = CLR_BG;
        for(I=1;I<J;++I) P[I]=CLR_FG;
        P[J] = CLR_BG;
      }
    }
    else
    {
      ShadowPrintXY(Img,"^ TO RESUME",(Img->W-8*11)>>1,Img->H-Y-8,CLR_FG,CLR_BG);
      ShadowPrintXY(Img,"<",X,(Img->H-8)>>1,CLR_FG,CLR_BG);
      ShadowPrintXY(Img,">",Img->W-X-8,(Img->H-8)>>1,CLR_FG,CLR_BG);

      for(J=0;J<7;++J) P[J]=CLR_BG;
      for(J=1,P+=Img->L;J<RPL_SIGNSIZE-3;++J,P+=Img->L)
      {
        P[1] = P[2] = P[4] = P[5] = CLR_FG;
        P[0] = P[3] = P[6] = CLR_BG;
      }
      for(J=0;J<7;++J) P[J]=CLR_BG;
    }

    if(TimeLeft>0)
    {
      char S[16];
      J = TimeLeft*100/60;
      sprintf(S,"%d:%02d",J/100,J%100);
      ShadowPrintXY(Img,S,X+RPL_SIGNSIZE/2+2,Y+(RPL_SIGNSIZE-8)/2-1,CLR_FG,CLR_BG);
//      PrintXY(Img,S,X+RPL_SIGNSIZE/2+2,Y+(RPL_SIGNSIZE-8)/2,CLR_FG,-1);
    }
  }
  else
  {
    /* Recording is active */
    for(J=0;J<=RPL_SIGNSIZE/2;++J,P+=Img->L)
    {
      I    = RPL_SIGNSIZE/2-J;
      P[I] = CLR_BG;
      while(I<RPL_SIGNSIZE/2+J) P[++I]=CLR_FG;
      P[I] = CLR_BG;
    }

    for(J-=2;J>=0;--J,P+=Img->L)
    {
      I    = RPL_SIGNSIZE/2-J;
      P[I] = CLR_BG;
      while(I<RPL_SIGNSIZE/2+J) P[++I]=CLR_FG;
      P[I] = CLR_BG;
    }
  }
}

#undef CLR_FG
#undef CLR_BG
#else /* !BPP8 && !BPP16 && !BPP24 && !BPP32 */

#define BPP8
#define pixel unsigned char
#define RPLShow RPLShow_8
#include "Record.c"
#undef pixel
#undef RPLShow
#undef BPP8

#define BPP16
#define pixel unsigned short
#define RPLShow RPLShow_16
#include "Record.c"
#undef pixel
#undef RPLShow
#undef BPP16

#define BPP32
#define pixel unsigned int
#define RPLShow RPLShow_32
#include "Record.c"
#undef pixel
#undef RPLShow
#undef BPP32

void RPLShow(Image *Img,int X,int Y)
{
  switch(Img->D)
  {
    case 8:  RPLShow_8(Img,X,Y);break;
    case 16: RPLShow_16(Img,X,Y);break;
    case 24:
    case 32: RPLShow_32(Img,X,Y);break;
  }
}

#endif /* !BPP8 && !BPP16 && !BPP24 && !BPP32 */
