/** EMULib Emulation Library *********************************/
/**                                                         **/
/**                          MCF.h                          **/
/**                                                         **/
/** This file contains declarations for the .MCF cheat file **/
/** support. See MCF.c for implementation.                  **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 2017                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifndef MCF_H
#define MCF_H

/** MCFEntry *************************************************/
/** Definition of a single MCF entry representing a cheat.  **/
/*************************************************************/
typedef struct
{
  unsigned int Addr;   // Address to apply cheat to
  unsigned int Data;   // Data to write to Addr
  unsigned char Size;  // Size of Data in bytes (1/2/4)
  char Note[116];      // Description of the cheat
} MCFEntry;

/** LoadFileMCF() ********************************************/
/** Load cheats from .MCF file. Returns number of loaded    **/
/** cheat entries or 0 on failure.                          **/
/*************************************************************/
int LoadFileMCF(const char *Name,MCFEntry *Cheats,int MaxCheats);

/** SaveFileMCF() ********************************************/
/** Save cheats to .MCF file. Returns number of loaded      **/
/** cheat entries or 0 on failure.                          **/
/*************************************************************/
int SaveFileMCF(const char *Name,const MCFEntry *Cheats,int CheatCount);

#endif /* MCF_H */
