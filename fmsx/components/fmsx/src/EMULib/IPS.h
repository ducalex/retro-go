/** EMULib Emulation Library *********************************/
/**                                                         **/
/**                          IPS.h                          **/
/**                                                         **/
/** This file contains declarations for the .IPS patch file **/
/** support. See IPS.c for implementation.                  **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1996-2021                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifndef IPS_H
#define IPS_H

/** ApplyIPS() ***********************************************/
/** Loads patches from an .IPS file and applies them to the **/
/** given data buffer. Returns number of patches applied.   **/
/*************************************************************/
unsigned int ApplyIPS(const char *FileName,unsigned char *Data,unsigned int Size);

/** MeasureIPS() *********************************************/
/** Find total data size assumed by a given .IPS file.      **/
/*************************************************************/
unsigned int MeasureIPS(const char *FileName);

#endif /* IPS_H */
