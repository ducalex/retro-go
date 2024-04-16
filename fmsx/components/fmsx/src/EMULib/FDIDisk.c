/** EMULib Emulation Library *********************************/
/**                                                         **/
/**                        FDIDisk.c                        **/
/**                                                         **/
/** This file contains routines to load, save, and access   **/
/** disk images in various formats. The internal format is  **/
/** always .FDI. See FDIDisk.h for declarations.            **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 2007-2021                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#include "FDIDisk.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#ifdef ZLIB
#include <zlib.h>
#endif

#define IMAGE_SIZE(Fmt) \
  (Formats[Fmt].Sides*Formats[Fmt].Tracks*    \
   Formats[Fmt].Sectors*Formats[Fmt].SecSize)

#define FDI_SIDES(P)      ((P)[6]+((int)((P)[7])<<8))
#define FDI_TRACKS(P)     ((P)[4]+((int)((P)[5])<<8))
#define FDI_DIR(P)        ((P)+(P)[12]+((int)((P)[13])<<8)+14)
#define FDI_DATA(P)       ((P)+(P)[10]+((int)((P)[11])<<8))
#define FDI_INFO(P)       ((P)+(P)[8]+((int)((P)[9])<<8))
#define FDI_SECTORS(T)    ((T)[6])
#define FDI_TRACK(P,T)    (FDI_DATA(P)+(T)[0]+((int)((T)[1])<<8)+((int)((T)[2])<<16)+((int)((T)[3])<<24))
#define FDI_SECSIZE(S)    (SecSizes[(S)[3]<=4? (S)[3]:4])
#define FDI_SECTOR(P,T,S) (FDI_TRACK(P,T)+(S)[5]+((int)((S)[6])<<8))

static const struct { int Sides,Tracks,Sectors,SecSize; } Formats[] =
{
  { 2,80,16,256 }, /* Dummy format */
  { 2,80,10,512 }, /* FMT_IMG can be 256 */
  { 2,80,10,512 }, /* FMT_MGT can be 256 */
  { 2,80,16,256 }, /* FMT_TRD    - ZX Spectrum TRDOS disk */
  { 2,80,10,512 }, /* FMT_FDI    - Generic FDI image */
  { 2,80,16,256 }, /* FMT_SCL    - ZX Spectrum TRDOS disk */
  { 2,80,16,256 }, /* FMT_HOBETA - ZX Spectrum HoBeta disk */
  { 2,80,9,512 },  /* FMT_MSXDSK - MSX disk */
  { 2,80,9,512 },  /* FMT_CPCDSK - CPC disk */
  { 1,40,16,256 }, /* FMT_SF7000 - Sega SF-7000 disk */
  { 2,80,10,512 }, /* FMT_SAMDSK - Sam Coupe disk */
  { 1,40,8,512 },  /* FMT_ADMDSK - Coleco Adam disk */
  { 1,32,16,512 }, /* FMT_DDP    - Coleco Adam tape */
  { 2,80,10,512 }, /* FMT_SAD    - Sam Coupe disk */
  { 2,80,9,512 }   /* FMT_DSK    - Assuming 720kB MSX format */
};

static const int SecSizes[] =
{ 128,256,512,1024,4096,0 };

static const char FDIDiskLabel[] =
"Disk image created by EMULib (C)Marat Fayzullin";

static const byte TRDDiskInfo[] =
{
  0x01,0x16,0x00,0xF0,0x09,0x10,0x00,0x00,
  0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
  0x20,0x00,0x00,0x64,0x69,0x73,0x6B,0x6E,
  0x61,0x6D,0x65,0x00,0x00,0x00,0x46,0x55
};

static int stricmpn(const char *S1,const char *S2,int Limit)
{
  for(;*S1&&*S2&&Limit&&(toupper(*S1)==toupper(*S2));++S1,++S2,--Limit);
  return(Limit? toupper(*S1)-toupper(*S2):0);
}

/** InitFDI() ************************************************/
/** Clear all data structure fields.                        **/
/*************************************************************/
void InitFDI(FDIDisk *D)
{
  D->Format   = 0;
  D->Data     = 0;
  D->DataSize = 0;
  D->Sides    = 0;
  D->Tracks   = 0;
  D->Sectors  = 0;
  D->SecSize  = 0;
}

/** EjectFDI() ***********************************************/
/** Eject disk image. Free all allocated memory.            **/
/*************************************************************/
void EjectFDI(FDIDisk *D)
{
  if(D->Data) free(D->Data);
  InitFDI(D);
}

/** NewFDI() *************************************************/
/** Allocate memory and create new .FDI disk image of given **/
/** dimensions. Returns disk data pointer on success, 0 on  **/
/** failure.                                                **/
/*************************************************************/
byte *NewFDI(FDIDisk *D,int Sides,int Tracks,int Sectors,int SecSize)
{
  byte *P,*DDir;
  int I,J,K,L,N;

  /* Find sector size code */
  for(L=0;SecSizes[L]&&(SecSizes[L]!=SecSize);++L);
  if(!SecSizes[L]) return(0);

  /* Allocate memory */
  K = Sides*Tracks*Sectors*SecSize+sizeof(FDIDiskLabel);
  I = Sides*Tracks*(Sectors+1)*7+14;
  if(!(P=(byte *)malloc(I+K))) return(0);
  memset(P,0x00,I+K);

  /* Eject previous disk image */
  EjectFDI(D);

  /* Set disk dimensions according to format */
  D->Format   = FMT_FDI;
  D->Data     = P;
  D->DataSize = I+K;
  D->Sides    = Sides;
  D->Tracks   = Tracks;
  D->Sectors  = Sectors;
  D->SecSize  = SecSize;

  /* .FDI magic number */
  memcpy(P,"FDI",3);
  /* Disk description */
  memcpy(P+I,FDIDiskLabel,sizeof(FDIDiskLabel));
  /* Write protection (1=ON) */
  P[3]  = 0;
  P[4]  = Tracks&0xFF;
  P[5]  = Tracks>>8;
  P[6]  = Sides&0xFF;
  P[7]  = Sides>>8;
  /* Disk description offset */
  P[8]  = I&0xFF;
  P[9]  = I>>8;
  I    += sizeof(FDIDiskLabel);
  /* Sector data offset */
  P[10] = I&0xFF;
  P[11] = I>>8;
  /* Track directory offset */
  P[12] = 0;
  P[13] = 0;

  /* Create track directory */
  for(J=K=0,DDir=P+14;J<Sides*Tracks;++J,K+=Sectors*SecSize)
  {
    /* Create track entry */
    DDir[0] = K&0xFF;
    DDir[1] = (K>>8)&0xFF;
    DDir[2] = (K>>16)&0xFF;
    DDir[3] = (K>>24)&0xFF;
    /* Reserved bytes */
    DDir[4] = 0;
    DDir[5] = 0;
    DDir[6] = Sectors;
    /* For all sectors on a track... */
    for(I=N=0,DDir+=7;I<Sectors;++I,DDir+=7,N+=SecSize)
    {
      /* Create sector entry */
      DDir[0] = J/Sides;
      DDir[1] = J%Sides;
      DDir[2] = I+1;
      DDir[3] = L;
      /* CRC marks and "deleted" bit (D00CCCCC) */
      DDir[4] = (1<<L);
      DDir[5] = N&0xFF;
      DDir[6] = N>>8;
    }
  }

  /* Done */
  return(FDI_DATA(P));
}

#ifdef ZLIB
#define fopen(N,M)      (FILE *)gzopen(N,M)
#define fclose(F)       gzclose((gzFile)(F))
#define fread(B,L,N,F)  gzread((gzFile)(F),B,(L)*(N))
#define fwrite(B,L,N,F) gzwrite((gzFile)(F),B,(L)*(N))
#define fgets(B,L,F)    gzgets((gzFile)(F),B,L)
#define fseek(F,O,W)    gzseek((gzFile)(F),O,W)
#define rewind(F)       gzrewind((gzFile)(F))
#define fgetc(F)        gzgetc((gzFile)(F))
#define ftell(F)        gztell((gzFile)(F))
#endif

/** LoadFDI() ************************************************/
/** Load a disk image from a given file, in a given format  **/
/** (see FMT_* #defines). Guess format from the file name   **/
/** when Format=FMT_AUTO. Returns format ID on success or   **/
/** 0 on failure. When FileName=0, ejects the disk freeing  **/
/** memory and returns 0.                                   **/
/*************************************************************/
int LoadFDI(FDIDisk *D,const char *FileName,int Format)
{
  byte Buf[256],*P,*DDir;
  const char *T;
  int J,I,K,L,N;
  FILE *F;

  /* If just ejecting a disk, drop out */
  if(!FileName) { EjectFDI(D);return(0); }

  /* If requested automatic format recognition... */
  if(!Format)
  {
    /* Recognize disk image format */
    T = strrchr(FileName,'\\');
    T = T? T:strrchr(FileName,'/');
    T = T? T+1:FileName;
    T = strchr(T,'.');
    Format = !T? 0
           : !stricmpn(T,".FDI",4)? FMT_FDI
           : !stricmpn(T,".IMG",4)? FMT_IMG
           : !stricmpn(T,".MGT",4)? FMT_MGT
           : !stricmpn(T,".TRD",4)? FMT_TRD
           : !stricmpn(T,".SCL",4)? FMT_SCL
           : !stricmpn(T,".DSK",4)? FMT_DSK
           : !stricmpn(T,".DDP",4)? FMT_DDP
           : !stricmpn(T,".SAD",4)? FMT_SAD
           : !stricmpn(T,".$",2)?   FMT_HOBETA
           : 0;

    /* Try loading by extension, ignore generic raw images for now */
    if(Format&&(Format!=FMT_DSK)&&(Format!=FMT_MGT)&&(J=LoadFDI(D,FileName,Format)))
      return(J);

    /* Try loading by magic number... */

    /* Starts with "FDI" */
    if(LoadFDI(D,FileName,FMT_FDI)) return(FMT_FDI);

    /* Starts with "SINCLAIR" */
    if(LoadFDI(D,FileName,FMT_SCL)) return(FMT_SCL);

    /* Starts with "Aley's disk backup" */
    if(LoadFDI(D,FileName,FMT_SAD)) return(FMT_SAD);

    /* Starts with "MV - CPC" or "EXTENDED CPC DSK File" */
    if(LoadFDI(D,FileName,FMT_CPCDSK)) return(FMT_CPCDSK);

    /* Starts with 0xE9 or 0xEB, with some other constraints */
    if(LoadFDI(D,FileName,FMT_MSXDSK)) return(FMT_MSXDSK);

    /* Try loading as a generic raw disk image */
    return(LoadFDI(D,FileName,FMT_DSK));
  }

  /* Open file and find its size */
  if(!(F=fopen(FileName,"rb"))) return(0);
#ifdef ZLIB
  for(J=0;(I=fread(Buf,1,sizeof(Buf),F));J+=I);
#else
  if(fseek(F,0,SEEK_END)<0) { fclose(F);return(0); }
  if((J=ftell(F))<=0)       { fclose(F);return(0); }
#endif
  rewind(F);

  switch(Format)
  {
    case FMT_FDI: /* If .FDI format... */
      /* Allocate memory and read file */
      if(!(P=(byte *)malloc(J))) { fclose(F);return(0); }
      if(fread(P,1,J,F)!=J)      { free(P);fclose(F);return(0); }
      /* Verify .FDI format tag */
      if(memcmp(P,"FDI",3))      { free(P);fclose(F);return(0); }
      /* Eject current disk image */
      EjectFDI(D);
      /* Read disk dimensions */
      D->Sides   = FDI_SIDES(P);
      D->Tracks  = FDI_TRACKS(P);
      D->Sectors = 0;
      D->SecSize = 0;
      /* Check number of sectors and sector size */
      for(J=FDI_SIDES(P)*FDI_TRACKS(P),DDir=FDI_DIR(P);J;--J)
      {
        /* Get number of sectors */
        I=FDI_SECTORS(DDir);
        /* Check that all tracks have the same number of sectors */
        if(!D->Sectors) D->Sectors=I; else if(D->Sectors!=I) break;
        /* Check that all sectors have the same size */
        for(DDir+=7;I;--I,DDir+=7)
          if(!D->SecSize) D->SecSize=FDI_SECSIZE(DDir);
          else if(D->SecSize!=FDI_SECSIZE(DDir)) break;
        /* Drop out if the sector size is not uniform */
        if(I) break;
      }
      /* If no uniform sectors or sector size, set them to zeros */
      if(J) D->Sectors=D->SecSize=0;
      break;

    case FMT_DSK: /* If generic raw disk image... */
    case FMT_MGT: /* ZX Spectrum .MGT is similar to .DSK */
      /* Try a few standard geometries first */
      I = J==IMAGE_SIZE(FMT_MSXDSK)? FMT_MSXDSK /* 737280 bytes */
        : J==IMAGE_SIZE(FMT_ADMDSK)? FMT_ADMDSK /* 163840 bytes */
        : J==IMAGE_SIZE(FMT_SAMDSK)? FMT_SAMDSK /* 819200 bytes */
        : J==IMAGE_SIZE(FMT_TRD)?    FMT_TRD    /* 655360 bytes */
        : J==IMAGE_SIZE(FMT_DDP)?    FMT_DDP    /* 262144 bytes */
        : J==IMAGE_SIZE(FMT_SF7000)? FMT_SF7000 /* 163840 bytes (!) */
        : J==IMAGE_SIZE(FMT_MGT)?    FMT_MGT    /* 819200 bytes (!) */
        : 0;
      /* If a standard geometry found... */
      if(I)
      {
        /* Create a new disk image */
        P = FormatFDI(D,Format=I);
        if(!P) { fclose(F);return(0); }
        /* Read disk image file (ignore short reads!) */
        fread(P,1,IMAGE_SIZE(I),F);
        /* Done */
        P = D->Data;
        break;
      }
      /* Try finding matching geometry */
      for(K=1,P=0;!P&&(K<=2);K<<=1)
        for(I=40;!P&&(I<=80);I<<=1)
          for(N=8;!P&&(N<=16);++N)
            for(L=256;!P&&(L<=512);L<<=1)
              if(J==K*I*N*L)
              {
                /* Create a new disk image */
                P = NewFDI(D,K,I,N,L);
                if(!P) { fclose(F);return(0); }
                /* Read disk image file (ignore short reads!) */
                fread(P,1,J,F);
                /* Done */
                P = D->Data;
              }
      break;

    case FMT_MSXDSK: /* If MSX .DSK format... */
      /* Read header */
      if(fread(Buf,1,32,F)!=32) { fclose(F);return(0); }
      /* Check magic number */
      if((Buf[0]!=0xE9)&&(Buf[0]!=0xEB)) { fclose(F);return(0); }
      /* Check media descriptor */
      if(Buf[21]<0xF8) { fclose(F);return(0); }
      /* Compute disk geometry */
      K = Buf[26]+((int)Buf[27]<<8);       /* Heads   */
      N = Buf[24]+((int)Buf[25]<<8);       /* Sectors */
      L = Buf[11]+((int)Buf[12]<<8);       /* SecSize */
      I = Buf[19]+((int)Buf[20]<<8);       /* Total S.*/
      I = K&&N? I/K/N:0;                   /* Tracks  */
      /* Number of heads CAN BE WRONG */
      K = I&&N&&L? J/I/N/L:0;
      /* Create a new disk image */
      P = NewFDI(D,K,I,N,L);
      if(!P) { fclose(F);return(0); }
      /* Make sure we do not read too much data */
      I = K*I*N*L;
      J = J>I? I:J;
      /* Read disk image file (ignore short reads!) */
      rewind(F);
      fread(P,1,J,F);
      /* Done */
      P = D->Data;
      break;

    case FMT_CPCDSK: /* If Amstrad CPC .DSK format... */
      /* Read header (first track is next) */
      if(fread(Buf,1,256,F)!=256) { fclose(F);return(0); }
      /* Check magic string */
      if(memcmp(Buf,"MV - CPC",8)&&memcmp(Buf,"EXTENDED CPC DSK File",21))
      { fclose(F);return(0); }
      /* Compute disk geometry */
      I = Buf[48];                   /* Tracks  */
      K = Buf[49];                   /* Heads   */
      N = Formats[Format].Sectors;   /* Maximal number of sectors */
      L = Buf[50]+((int)Buf[51]<<8); /* Track size + 0x100 */
      /* Extended CPC .DSK format lists sizes by track */
      if(!L)
        for(J=0;J<I;++J)
          if(L<((int)Buf[J+52]<<8)) L=(int)Buf[J+52]<<8;
      /* Maximal sector size */  
      L = (L-0x100+N-1)/N;
      /* Round up to the next power of two */
      for(J=1;J<L;J<<=1);
//printf("Tracks=%d, Heads=%d, Sectors=%d, SectorSize=%d<%d\n",I,K,N,L,J);
      if(D->Verbose && (L!=J))
        printf("LoadFDI(): Adjusted %d-byte CPC disk sectors to %d bytes.\n",L,J);
      L = J;
      /* Check geometry */
      if(!K||!N||!L||!I) { fclose(F);return(0); }
      /* Create a new disk image */
      if(!NewFDI(D,K,I,N,L)) { fclose(F);return(0); }
      /* Sectors-per-track and bytes-per-sector may vary */
      D->Sectors = 0;
      D->SecSize = 0;
      /* We are rewriting .FDI directory and data */
      DDir = FDI_DIR(D->Data);
      P    = FDI_DATA(D->Data);
      /* Skip to the first track info block */
      fseek(F,0x100,SEEK_SET);
      /* Read tracks */
      for(I*=K;I;--I)
      {
        /* Read track header */
        if(fread(Buf,1,0x18,F)!=0x18) break;
        /* Check magic string */
        if(memcmp(Buf,"Track-Info\r\n",12)) break;
        /* Compute track geometry */
        N = Buf[21];             /* Sectors */
        L = Buf[20];             /* SecSize */
        J = P-FDI_DATA(D->Data); /* Data offset */
        /* Create .FDI track entry */
        DDir[0] = J&0xFF;
        DDir[1] = (J>>8)&0xFF;
        DDir[2] = (J>>16)&0xFF;
        DDir[3] = (J>>24)&0xFF;
        DDir[4] = 0;
        DDir[5] = 0;
        DDir[6] = N;
        /* Read sector headers */
        for(DDir+=7,J=N,K=0;J&&(fread(Buf,8,1,F)==8);DDir+=7,--J,K+=SecSizes[L])
        {
          /* Create .FDI sector entry */
          DDir[0] = Buf[0];
          DDir[1] = Buf[1];
          DDir[2] = Buf[2];
          DDir[3] = Buf[3];
//          DDir[4] = (1<<L)|(~Buf[4]&0x80);
          DDir[4] = (1<<L)|((Buf[5]&0x40)<<1);
          DDir[5] = K&0xFF;
          DDir[6] = K>>8;
        }
        /* Seek to the track data */
        if(fseek(F,0x100-0x18-8*N,SEEK_CUR)<0) break;
        /* Read track data */
        if(fread(P,1,K,F)!=K) break; else P+=K;
      }
      /* Done */
      P = D->Data;
      break;

    case FMT_SAD: /* If Sam Coupe .SAD format... */
      /* Read header */
      if(fread(Buf,1,22,F)!=22) { fclose(F);return(0); }
      /* Check magic string */
      if(memcmp(Buf,"Aley's disk backup",18)) { fclose(F);return(0); }
      /* Compute disk geometry */
      K = Buf[18];     /* Heads       */
      I = Buf[19];     /* Tracks      */
      N = Buf[20];     /* Sectors     */
      L = Buf[21]*64;  /* Sector size */
      /* Check geometry */
      if(!K||!N||!L||!I) { fclose(F);return(0); }
      /* Create a new disk image */
      P = NewFDI(D,K,I,N,L);
      if(!P) { fclose(F);return(0); }
      /* Make sure we do not read too much data */
      I = K*I*N*L;
      J = J-22;
      J = J>I? I:J;
      /* Read disk image file (ignore short reads!) */
      fread(P,1,J,F);
      /* Done */
      P = D->Data;
      break;

    case FMT_IMG: /* If .IMG format... */
      /* Create a new disk image */
      P = FormatFDI(D,Format);
      if(!P) { fclose(F);return(0); }
      /* Read disk image file track-by-track */
      K = Formats[Format].Tracks;
      L = Formats[Format].Sectors*Formats[Format].SecSize;
      I = Formats[Format].Tracks*Formats[Format].Sides;
      for(J=0;J<I;++J)
        if(fread(P+L*2*(J%K)+(J>=K? L:0),1,L,F)!=L) break;
      /* Done */
      P = D->Data;
      break;

    case FMT_SCL: /* If .SCL format... */
      /* @@@ NEED TO CHECK CHECKSUM AT THE END */
      /* Read header */
      if(fread(Buf,1,9,F)!=9) { fclose(F);return(0); }
      /* Verify .SCL format tag and the number of files */
      if(memcmp(Buf,"SINCLAIR",8)||(Buf[8]>128)) { fclose(F);return(0); }
      /* Create a new disk image */
      P = FormatFDI(D,Format);
      if(!P) { fclose(F);return(0); }
      /* Compute the number of free sectors */
      I = D->Sides*D->Tracks*D->Sectors;
      /* Build directory, until we run out of disk space */
      for(J=0,K=D->Sectors,DDir=P;(J<Buf[8])&&(K<I);++J)
      {
        /* Read .SCL directory entry */
        if(fread(DDir,1,14,F)!=14) break;
        /* Compute sector and track */
        DDir[14] = K%D->Sectors;
        DDir[15] = K/D->Sectors;
        /* Next entry */
        K       += DDir[13];
        DDir    += 16;
      }
      /* Skip over remaining directory entries */
      if(J<Buf[8]) fseek(F,(Buf[8]-J)*14,SEEK_CUR);
      /* Build disk information */
      memset(P+J*16,0,D->Sectors*D->SecSize-J*16);
      memcpy(P+0x08E2,TRDDiskInfo,sizeof(TRDDiskInfo));
      strncpy((char *)P+0x08F5,"SPECCY",8);
      P[0x8E1] = K%D->Sectors;  /* First free sector */
      P[0x8E2] = K/D->Sectors;  /* Track it belongs to */
      P[0x8E3] = 0x16 + (D->Tracks>40? 0:1) + (D->Sides>1? 0:2);
      P[0x8E4] = J;             /* Number of files */
      P[0x8E5] = (I-K)&0xFF;    /* Number of free sectors */
      P[0x8E6] = (I-K)>>8;
      /* Read data */
      for(DDir=P;J;--J,DDir+=16)
      {
        /* Determine data offset and size */
        I = (DDir[15]*D->Sectors+DDir[14])*D->SecSize;
        N = DDir[13]*D->SecSize;
        /* Read .SCL data (ignore short reads!) */
        fread(P+I,1,N,F);
      }
      /* Done */
      P = D->Data;
      break;

    case FMT_HOBETA: /* If .$* format... */
      /* Create a new disk image */
      P = FormatFDI(D,Format);
      if(!P) { fclose(F);return(0); }
      /* Read header */
      if(fread(P,1,17,F)!=17) { fclose(F);return(0); }
      /* Determine data offset and size */
      I = D->Sectors*D->SecSize;
      N = P[13]+((int)P[14]<<8);
      /* Build disk information */
      memset(P+16,0,I-16);
      memcpy(P+0x08E2,TRDDiskInfo,sizeof(TRDDiskInfo));
      strncpy((char *)P+0x08F5,"SPECCY",8);
      K        = D->Sectors+N;
      J        = D->Sectors*D->Tracks*D->Sides-K;
      P[0x8E1] = K%D->Sectors;  /* First free sector */
      P[0x8E2] = K/D->Sectors;  /* Track it belongs to */
      P[0x8E3] = 0x16 + (D->Tracks>40? 0:1) + (D->Sides>1? 0:2);
      P[0x8E4] = 1;             /* Number of files */
      P[0x8E5] = J&0xFF;        /* Number of free sectors */
      P[0x8E6] = J>>8;
      N        = N*D->SecSize;  /* N is now in bytes */
      /* Read data (ignore short reads!) */
      fread(P+I,1,N,F);
      /* Compute and check checksum */
      for(L=I=0;I<15;++I) L+=P[I];
      L = ((L*257+105)&0xFFFF)-P[15]-((int)P[16]<<8);
      if(L) { /* @@@ DO SOMETHING BAD! */ }
      /* Place file at track #1 sector #0, limit its size to 255 sectors */
      P[13] = P[14]? 255:P[13];
      P[14] = 0;
      P[15] = 1;
      P[16] = 0;
      /* Done */
      P = D->Data;
      break;

    case FMT_SF7000: /* If SF-7000 .SF format...  */
    case FMT_SAMDSK: /* If Sam Coupe .DSK format... */
    case FMT_ADMDSK: /* If Coleco Adam .DSK format... */
    case FMT_DDP:    /* If Coleco Adam .DDP format... */
    case FMT_TRD:    /* If ZX Spectrum .TRD format... */
      /* Must have exact size, unless it is a .TRD */
      if((Format!=FMT_TRD) && (J!=IMAGE_SIZE(Format))) { fclose(F);return(0); }
      /* Create a new disk image */
      P = FormatFDI(D,Format);
      if(!P) { fclose(F);return(0); }
      /* Read disk image file (ignore short reads!) */
      fread(P,1,IMAGE_SIZE(Format),F);
      /* Done */
      P = D->Data;
      break;

    default:
      /* Format not recognized */
      return(0);
  }

  if(D->Verbose)
    printf(
      "LoadFDI(): Loaded '%s', %d sides x %d tracks x %d sectors x %d bytes\n",
      FileName,D->Sides,D->Tracks,D->Sectors,D->SecSize
    );

  /* Done */
  fclose(F);
  D->Data   = P;
  D->Format = Format;
  return(Format);
}

#ifdef ZLIB
#undef fopen
#undef fclose
#undef fread
#undef fwrite
#undef fseek
#undef ftell
#undef rewind
#endif

/** SaveDSKData() ********************************************/
/** Save uniform disk data, truncating or adding zeros as   **/
/** needed. Returns FDI_SAVE_OK on success, FDI_SAVE_PADDED **/
/** if any sectors were padded, FDI_SAVE_TRUNCATED if any   **/
/** sectors were truncated, FDI_SAVE_FAILED if failed.      **/
/*************************************************************/
static int SaveDSKData(FDIDisk *D,FILE *F,int Sides,int Tracks,int Sectors,int SecSize)
{
  int J,I,K,Result;

  Result = FDI_SAVE_OK;

  /* Scan through all tracks, sides, sectors */
  for(J=0;J<Tracks;++J)
    for(I=0;I<Sides;++I)
      for(K=0;K<Sectors;++K)
      {
        /* Seek to sector and determine actual sector size */
        byte *P = SeekFDI(D,I,J,I,J,K+1);
        int   L = D->SecSize<SecSize? D->SecSize:SecSize;
        /* Write sector */
        if(!P||!L||(fwrite(P,1,L,F)!=L)) return(FDI_SAVE_FAILED);
        /* Pad sector to SecSize, if needed */
        if((SecSize>L)&&fseek(F,SecSize-L,SEEK_CUR)) return(FDI_SAVE_FAILED);
        /* Update result */
        L = SecSize>L? FDI_SAVE_PADDED:SecSize<L? FDI_SAVE_TRUNCATED:FDI_SAVE_OK;
        if(L<Result) Result=L;
      }

  /* Done */
  return(Result);
}

/** SaveIMGData() ********************************************/
/** Save uniform disk data, truncating or adding zeros as   **/
/** needed. Returns FDI_SAVE_OK on success, FDI_SAVE_PADDED **/
/** if any sectors were padded, FDI_SAVE_TRUNCATED if any   **/
/** sectors were truncated, FDI_SAVE_FAILED if failed.      **/
/*************************************************************/
static int SaveIMGData(FDIDisk *D,FILE *F,int Sides,int Tracks,int Sectors,int SecSize)
{
  int J,I,K,Result;

  Result = FDI_SAVE_OK;

  /* Scan through all sides, tracks, sectors */
  for(I=0;I<Sides;++I)
    for(J=0;J<Tracks;++J)
      for(K=0;K<Sectors;++K)
      {
        /* Seek to sector and determine actual sector size */
        byte *P = SeekFDI(D,I,J,I,J,K+1);
        int   L = D->SecSize<SecSize? D->SecSize:SecSize;
        /* Write sector */
        if(!P||!L||(fwrite(P,1,L,F)!=L)) return(FDI_SAVE_FAILED);
        /* Pad sector to SecSize, if needed */
        if((SecSize>L)&&fseek(F,SecSize-L,SEEK_CUR)) return(FDI_SAVE_FAILED);
        /* Update result */
        L = SecSize>L? FDI_SAVE_PADDED:SecSize<L? FDI_SAVE_TRUNCATED:FDI_SAVE_OK;
        if(L<Result) Result=L;
      }

  /* Done */
  return(Result);
}

/** SaveFDI() ************************************************/
/** Save a disk image to a given file, in a given format    **/
/** (see FMT_* #defines). Use the original format when      **/
/** when Format=FMT_AUTO. Returns FDI_SAVE_OK on success,   **/
/** FDI_SAVE_PADDED if any sectors were padded,             **/
/** FDI_SAVE_TRUNCATED if any sectors were truncated,       **/
/** FDI_SAVE_FAILED (0) if failed.                          **/
/*************************************************************/
int SaveFDI(FDIDisk *D,const char *FileName,int Format)
{
  byte S[256];
  int I,J,K,C,L,Result;
  FILE *F;
  byte *P,*T;

  /* Must have a disk to save */
  if(!D->Data) return(0);

  /* Use original format if requested */
  if(!Format) Format=D->Format;

  /* Open file for writing */
  if(!(F=fopen(FileName,"wb"))) return(0);

  /* Assume success */
  Result = FDI_SAVE_OK;

  /* Depending on the format... */
  switch(Format)
  {
    case FMT_FDI:
      /* This is the native format in which data is stored in memory */
      if(fwrite(D->Data,1,D->DataSize,F)!=D->DataSize)
      { fclose(F);unlink(FileName);return(0); }
      break;

    case FMT_IMG:
      /* Check the number of tracks and sides */
      if((FDI_TRACKS(D->Data)!=Formats[Format].Tracks)||(FDI_SIDES(D->Data)!=Formats[Format].Sides))
      { fclose(F);unlink(FileName);return(0); }
      /* Write out the data, in sides/tracks/sectors order */
      Result = SaveIMGData(D,F,Formats[Format].Sides,Formats[Format].Tracks,Formats[Format].Sectors,Formats[Format].SecSize);
      if(!Result) { fclose(F);unlink(FileName);return(0); }
      break;

    case FMT_SF7000:
    case FMT_SAMDSK:
    case FMT_ADMDSK:
    case FMT_DDP:
    case FMT_TRD:
      /* Check the number of tracks and sides */
      if((FDI_TRACKS(D->Data)!=Formats[Format].Tracks)||(FDI_SIDES(D->Data)!=Formats[Format].Sides))
      { fclose(F);unlink(FileName);return(0); }
      /* Write out the data, in tracks/sides/sectors order */
      Result = SaveDSKData(D,F,Formats[Format].Sides,Formats[Format].Tracks,Formats[Format].Sectors,Formats[Format].SecSize);
      if(!Result) { fclose(F);unlink(FileName);return(0); }
      break;

    case FMT_MSXDSK:
    case FMT_DSK:
    case FMT_MGT:
      /* Must have uniform tracks */
      if(!D->Sectors || !D->SecSize) { fclose(F);unlink(FileName);return(0); }
      /* Write out the data, in tracks/sides/sectors order */
      Result = SaveDSKData(D,F,FDI_SIDES(D->Data),FDI_TRACKS(D->Data),D->Sectors,D->SecSize);
      if(!Result) { fclose(F);unlink(FileName);return(0); }
      break;

    case FMT_SAD:
      /* Must have uniform tracks with "even" sector size */
      if(!D->Sectors || !D->SecSize || (D->SecSize&0x3F))
      { fclose(F);unlink(FileName);return(0); }
      /* Fill header */
      memset(S,0,sizeof(S));
      strcpy((char *)S,"Aley's disk backup");
      S[18] = FDI_SIDES(D->Data);
      S[19] = FDI_TRACKS(D->Data);
      S[20] = D->Sectors;
      S[21] = D->SecSize>>6;
      /* Write header */
      if(fwrite(S,1,22,F)!=22) { fclose(F);unlink(FileName);return(0); }
      /* Write out the data, in tracks/sides/sectors order */
      Result = SaveDSKData(D,F,S[18],S[19],S[20],S[21]*64);
      if(!Result) { fclose(F);unlink(FileName);return(0); }
      break;

    case FMT_SCL:
      /* Get data pointer */
      T=FDI_DATA(D->Data);
      /* Check tracks, sides, sectors, and the TR-DOS magic number */
      if((FDI_SIDES(D->Data)!=Formats[Format].Sides)
       ||(FDI_TRACKS(D->Data)!=Formats[Format].Tracks)
       ||(D->Sectors!=Formats[Format].Sectors)
       ||(D->SecSize!=Formats[Format].SecSize)
       ||(T[0x8E3]!=0x16)
      ) { fclose(F);unlink(FileName);return(0); }
      /* Write header */
      strcpy((char *)S,"SINCLAIR");
      S[8]=T[0x8E4];
      if(fwrite(S,1,9,F)!=9) { fclose(F);unlink(FileName);return(0); }
      for(C=I=0;I<9;++I) C+=S[I];
      /* Write directory entries */
      for(J=0,P=T;J<T[0x8E4];++J,P+=16)
      {
        if(fwrite(P,1,14,F)!=14) { fclose(F);unlink(FileName);return(0); }
        for(I=0;I<14;++I) C+=P[I];
      }
      /* Write files */
      for(J=0,P=T;J<T[0x8E4];++J,P+=16)
      {
        /* Determine data offset and size */
        K = (P[15]*D->Sectors+P[14])*D->SecSize;
        I = P[13]*D->SecSize;
        /* Write data */
        if(fwrite(T+K,1,I,F)!=I)
        { fclose(F);unlink(FileName);return(0); }
        /* Compute checksum */
        for(L=K,I+=K;L<I;++L) C+=T[L];
      }
      /* Write checksum */
      S[0] = C&0xFF;
      S[1] = (C>>8)&0xFF;
      S[2] = (C>>16)&0xFF;
      S[3] = (C>>24)&0xFF;
      if(fwrite(S,1,4,F)!=4) { fclose(F);unlink(FileName);return(0); }
      /* Done */
      break;

    case FMT_HOBETA:
      /* Get data pointer */
      T=FDI_DATA(D->Data);
      /* Check tracks, sides, sectors, and the TR-DOS magic number */
      if((FDI_SIDES(D->Data)!=Formats[Format].Sides)
       ||(FDI_TRACKS(D->Data)!=Formats[Format].Tracks)
       ||(D->Sectors!=Formats[Format].Sectors)
       ||(D->SecSize!=Formats[Format].SecSize)
       ||(T[0x8E3]!=0x16)
      ) { fclose(F);unlink(FileName);return(0); }
      /* Look for the first file */
      for(J=0,P=T;(J<T[0x8E4])&&!P[0];++J,P+=16);
      /* If not found, drop out */
      if(J>=T[0x8E4]) { fclose(F);unlink(FileName);return(0); }      
      /* Copy header */
      memcpy(S,P,14);
      /* Get single file address and size */
      I = P[13]*D->SecSize;
      P = T+(P[14]+D->Sectors*P[15])*D->SecSize;
      /* Compute checksum and build header */
      for(C=J=0;J<14;++J) C+=P[J];
      S[14] = 0;
      C     = (C+S[14])*257+105;
      S[15] = C&0xFF;
      S[16] = (C>>8)&0xFF;
      /* Write header */
      if(fwrite(S,1,17,F)!=17) { fclose(F);unlink(FileName);return(0); }
      /* Write file data */
      if(fwrite(P,1,I,F)!=I) { fclose(F);unlink(FileName);return(0); }
      /* Done */
      break;

    default:
      /* Can't save this format for now */
      fclose(F);
      unlink(FileName);
      return(0);
  }

  /* Done */
  fclose(F);
  return(Result);
}

/** SeekFDI() ************************************************/
/** Seek to given side/track/sector. Returns sector address **/
/** on success or 0 on failure.                             **/
/*************************************************************/
byte *SeekFDI(FDIDisk *D,int Side,int Track,int SideID,int TrackID,int SectorID)
{
  byte *P,*T;
  int J,Deleted;

  /* Have to have disk mounted */
  if(!D||!D->Data) return(0);

  /* May need to search for deleted sectors */
  Deleted = (SectorID>=0) && (SectorID&SEEK_DELETED)? 0x80:0x00;
  if(Deleted) SectorID&=~SEEK_DELETED;

  switch(D->Format)
  {
    case FMT_TRD:
    case FMT_DSK:
    case FMT_SCL:
    case FMT_FDI:
    case FMT_MGT:
    case FMT_IMG:
    case FMT_DDP:
    case FMT_SAD:
    case FMT_CPCDSK:
    case FMT_SAMDSK:
    case FMT_ADMDSK:
    case FMT_MSXDSK:
    case FMT_SF7000:
      /* Track directory */
      P = FDI_DIR(D->Data);
      /* Find current track entry */
      for(J=Track*D->Sides+Side%D->Sides;J;--J) P+=(FDI_SECTORS(P)+1)*7;
      /* Find sector entry */
      for(J=FDI_SECTORS(P),T=P+7;J;--J,T+=7)
        if((T[0]==TrackID)||(TrackID<0))
          if((T[1]==SideID)||(SideID<0))
            if(((T[2]==SectorID)&&((T[4]&0x80)==Deleted))||(SectorID<0))
              break;
      /* Fall out if not found */
      if(!J) return(0);
      /* FDI stores a header for each sector */
      D->Header[0] = T[0];
      D->Header[1] = T[1];
      D->Header[2] = T[2];
      D->Header[3] = T[3]<=3? T[3]:3;
      D->Header[4] = T[4];
      D->Header[5] = 0x00;
      /* FDI has variable sector numbers and sizes */
      D->Sectors   = FDI_SECTORS(P);
      D->SecSize   = FDI_SECSIZE(T);
      return(FDI_SECTOR(D->Data,P,T));
  }

  /* Unknown format */
  return(0);
}

/** LinearFDI() **********************************************/
/** Seek to given sector by its linear number. Returns      **/
/** sector address on success or 0 on failure.              **/
/*************************************************************/
byte *LinearFDI(FDIDisk *D,int SectorN)
{
  if(!D->Sectors || !D->Sides || (SectorN<0)) return(0);
  else
  {
    int Sector = SectorN % D->Sectors;
    int Track  = SectorN / D->Sectors / D->Sides;
    int Side   = (SectorN / D->Sectors) % D->Sides;
    return(SeekFDI(D,Side,Track,Side,Track,Sector+1));
  }
}

/** FormatFDI() ***********************************************/
/** Allocate memory and create new standard disk image for a **/
/** given format. Returns disk data pointer on success, 0 on **/
/** failure.                                                 **/
/**************************************************************/
byte *FormatFDI(FDIDisk *D,int Format)
{
  if((Format<0) || (Format>=sizeof(Formats)/sizeof(Formats[0]))) return(0);
  
  return(NewFDI(D,
    Formats[Format].Sides,
    Formats[Format].Tracks,
    Formats[Format].Sectors,
    Formats[Format].SecSize
  ));
}

