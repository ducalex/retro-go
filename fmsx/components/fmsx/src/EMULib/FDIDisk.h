/** EMULib Emulation Library *********************************/
/**                                                         **/
/**                        FDIDisk.h                        **/
/**                                                         **/
/** This file declares routines to load, save, and access   **/
/** disk images in various formats. The internal format is  **/
/** always .FDI. See FDIDisk.c for the actual code.         **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 2007-2021                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifndef FDIDISK_H
#define FDIDISK_H
#ifdef __cplusplus
extern "C" {
#endif
                              /* SaveFDI() result:           */
#define FDI_SAVE_FAILED    0  /* Failed saving disk image    */
#define FDI_SAVE_TRUNCATED 1  /* Truncated data while saving */
#define FDI_SAVE_PADDED    2  /* Padded data while saving    */
#define FDI_SAVE_OK        3  /* Succeeded saving disk image */

                           /* Supported disk image formats:  */
#define FMT_AUTO   0       /* Determine format automatically */                   
#define FMT_IMG    1       /* ZX Spectrum disk               */             
#define FMT_MGT    2       /* ZX Spectrum disk, same as .DSK */             
#define FMT_TRD    3       /* ZX Spectrum TRDOS disk         */
#define FMT_FDI    4       /* Generic FDI image              */ 
#define FMT_SCL    5       /* ZX Spectrum TRDOS disk         */
#define FMT_HOBETA 6       /* ZX Spectrum HoBeta disk        */
#define FMT_MSXDSK 7       /* MSX disk                       */          
#define FMT_CPCDSK 8       /* CPC disk                       */          
#define FMT_SF7000 9       /* Sega SF-7000 disk              */ 
#define FMT_SAMDSK 10      /* Sam Coupe disk                 */    
#define FMT_ADMDSK 11      /* Coleco Adam disk               */  
#define FMT_DDP    12      /* Coleco Adam tape               */  
#define FMT_SAD    13      /* Sam Coupe disk                 */
#define FMT_DSK    14      /* Generic raw disk image         */

#define SEEK_DELETED (0x40000000)

#define DataFDI(D) ((D)->Data+(D)->Data[10]+((int)((D)->Data[11])<<8))

#ifndef BYTE_TYPE_DEFINED
#define BYTE_TYPE_DEFINED
typedef unsigned char byte;
#endif

/** FDIDisk **************************************************/
/** This structure contains all disk image information and  **/
/** also the result of the last SeekFDI() call.             **/
/*************************************************************/
typedef struct
{
  byte Format;     /* Original disk format (FMT_*) */
  int  Sides;      /* Sides per disk */
  int  Tracks;     /* Tracks per side */
  int  Sectors;    /* Sectors per track */
  int  SecSize;    /* Bytes per sector */

  byte *Data;      /* Disk data */
  int  DataSize;   /* Disk data size */

  byte Header[6];  /* Current header, result of SeekFDI() */
  byte Verbose;    /* 1: Print debugging messages */
} FDIDisk;

/** InitFDI() ************************************************/
/** Clear all data structure fields.                        **/
/*************************************************************/
void InitFDI(FDIDisk *D);

/** EjectFDI() ***********************************************/
/** Eject disk image. Free all allocated memory.            **/
/*************************************************************/
void EjectFDI(FDIDisk *D);

/** NewFDI() *************************************************/
/** Allocate memory and create new .FDI disk image of given **/
/** dimensions. Returns disk data pointer on success, 0 on  **/
/** failure.                                                **/
/*************************************************************/
byte *NewFDI(FDIDisk *D,int Sides,int Tracks,int Sectors,int SecSize);

/** FormatFDI() ***********************************************/
/** Allocate memory and create new standard disk image for a **/
/** given format. Returns disk data pointer on success, 0 on **/
/** failure.                                                 **/
/**************************************************************/
byte *FormatFDI(FDIDisk *D,int Format);

/** LoadFDI() ************************************************/
/** Load a disk image from a given file, in a given format  **/
/** (see FMT_* #defines). Guess format from the file name   **/
/** when Format=FMT_AUTO. Returns format ID on success or   **/
/** 0 on failure. When FileName=0, ejects the disk freeing  **/
/** memory and returns 0.                                   **/
/*************************************************************/
int LoadFDI(FDIDisk *D,const char *FileName,int Format);

/** SaveFDI() ************************************************/
/** Save a disk image to a given file, in a given format    **/
/** (see FMT_* #defines). Use the original format when      **/
/** when Format=FMT_AUTO. Returns FDI_SAVE_OK on success,   **/
/** FDI_SAVE_PADDED if any sectors were padded,             **/
/** FDI_SAVE_TRUNCATED if any sectors were truncated,       **/
/** FDI_SAVE_FAILED (0) if failed.                          **/
/*************************************************************/
int SaveFDI(FDIDisk *D,const char *FileName,int Format);

/** SeekFDI() ************************************************/
/** Seek to given side/track/sector. Returns sector address **/
/** on success or 0 on failure.                             **/
/*************************************************************/
byte *SeekFDI(FDIDisk *D,int Side,int Track,int SideID,int TrackID,int SectorID);

/** LinearFDI() **********************************************/
/** Seek to given sector by its linear number. Returns      **/
/** sector address on success or 0 on failure.              **/
/*************************************************************/
byte *LinearFDI(FDIDisk *D,int SectorN);

#ifdef __cplusplus
}
#endif
#endif /* FDIDISK_H */
