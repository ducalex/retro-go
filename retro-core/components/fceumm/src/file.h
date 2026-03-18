#ifndef _FCEU_FILE_H
#define _FCEU_FILE_H

typedef struct {
   uint8 *data_int;
   const uint8 *data;
   uint32 size;
   uint32 location;
} MEMWRAP;

typedef struct {
   MEMWRAP *fp;
} FCEUFILE;

FCEUFILE *FCEU_fopen(const char *path, const uint8 *buffer, size_t bufsize);
int FCEU_fclose(FCEUFILE*);
uint64 FCEU_fread(void *ptr, size_t size, size_t nmemb, FCEUFILE*);
int FCEU_fseek(FCEUFILE*, long offset, int whence);
uint64 FCEU_ftell(FCEUFILE*);
int FCEU_read32le(uint32 *Bufo, FCEUFILE*);
int FCEU_fgetc(FCEUFILE*);
uint64 FCEU_fgetsize(FCEUFILE*);

#endif
