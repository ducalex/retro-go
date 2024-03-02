/** EMULib Emulation Library *********************************/
/**                                                         **/
/**                          MCF.c                          **/
/**                                                         **/
/** This file contains support for the .MCF cheat file      **/
/** format. See MCF.h for declarations.                     **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 2017                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#include "MCF.h"

#include <stdio.h>
#include <string.h>

/** LoadFileMCF() ********************************************/
/** Load cheats from .MCF file. Returns number of loaded    **/
/** cheat entries or 0 on failure.                          **/
/*************************************************************/
int LoadFileMCF(const char *Name,MCFEntry *Cheats,int MaxCheats)
{
  char Buf[256],Note[256];
  unsigned int Arg0,Addr,Data,Arg3;
  int J;
  FILE *F;

  /* Open .MCF text file with cheats */
  F = fopen(Name,"rb");
  if(!F) return(0);

  /* Load cheats from file */
  for(J=0;!feof(F)&&(J<MaxCheats);)
    if(fgets(Buf,sizeof(Buf),F) && (sscanf(Buf,"%u,%u,%u,%u,%255s",&Arg0,&Addr,&Data,&Arg3,Note)==5))
    {
      Cheats[J].Addr = Addr;
      Cheats[J].Data = Data;
      Cheats[J].Size = Data>0xFFFF? 4:Data>0xFF? 2:1;
      strncpy(Cheats[J].Note,Note,sizeof(Cheats[J].Note));
      Cheats[J].Note[sizeof(Cheats[J].Note)-1] = '\0';
      ++J;
    }

  /* Done with the file */
  fclose(F);

  /* Done */
  return(J);
}

/** SaveFileMCF() ********************************************/
/** Save cheats to .MCF file. Returns number of loaded      **/
/** cheat entries or 0 on failure.                          **/
/*************************************************************/
int SaveFileMCF(const char *Name,const MCFEntry *Cheats,int CheatCount)
{
  FILE *F;
  int J;

  /* Open .MCF text file with cheats */
  F = fopen(Name,"wb");
  if(!F) return(0);

  /* Save cheats */
  for(J=0;J<CheatCount;++J)
    fprintf(F,"%d,%d,%d,%d,%s\n",0,Cheats[J].Addr,Cheats[J].Data,0,Cheats[J].Note);

  /* Done */
  fclose(F);
  return(CheatCount);
}
