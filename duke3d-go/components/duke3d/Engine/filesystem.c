//
//  filesystem.c
//  Duke3D
//
//  Created by fabien sanglard on 12-12-19.
//  Copyright (c) 2012 fabien sanglard. All rights reserved.
//

#include "filesystem.h"

#include "platform.h"
#include "cache.h"
#include "fixedPoint_math.h"
#include "global.h"
#include <strings.h>

#include "esp_attr.h"
#include "SDL.h"

#include "duke3d.h"

extern char game_dir[512];

//The multiplayer module in game.dll needs direct access to the crc32 (sic).
int32_t groupefil_crc32[MAXGROUPFILES];


// A typical GRP index entry:
//     - 12 bytes for filename
//     -  4 for filesize
typedef uint8_t grpIndexEntry_t[16]; 

typedef struct grpArchive_s{
    
    int32_t  numFiles             ;//Number of files in the archive.
    grpIndexEntry_t  *gfilelist   ;//Array containing the filenames.
    int32_t  *fileOffsets         ;//Array containing the file offsets.
    int32_t  *filesizes           ;//Array containing the file offsets.
    int fileDescriptor            ;//The fd used for open,read operations.
    uint32_t crc32                ;//Hash to recognize GRP: Duke Shareware, Duke plutonimum etc...
    
} grpArchive_t;

//All GRP opened are in this structure
typedef struct grpSet_s{
    grpArchive_t archives[MAXGROUPFILES];
    int32_t num;
} grpSet_t;

// Marking it static gurantee not only invisility outside module
// but also that the content will be set to 0.
EXT_RAM_ATTR static grpSet_t grpSet;


int32_t initgroupfile(const char  *filename)
{
	uint8_t         buf[16]                 ;
	int32_t         i, j, k                 ;
    grpArchive_t*   archive                 ;
    
    
    
	printf("Loading %s ...", filename);
    
	if (grpSet.num == MAXGROUPFILES){
        printf("Error: Unable to open an extra GRP archive <= No more slot available.\n");
        return(-1);
    }
    
    archive = &grpSet.archives[grpSet.num];
    
    //Init the slot
    memset(archive, 0, sizeof(grpArchive_t));
    
	//groupfil_memory[numgroupfiles] = NULL; // addresses of raw GRP files in memory
	//groupefil_crc32[numgroupfiles] = 0;
    SDL_LockDisplay();
	archive->fileDescriptor = open(filename,O_BINARY|O_RDONLY,S_IREAD);
    
    if (archive->fileDescriptor < 0){
        printf("Error: Unable to open file %s.\n",filename);
        getchar();
        exit(0);
    }
    
    
    read(archive->fileDescriptor,buf,16);

    //FCS   : The ".grp" file format is just a collection of a lot of files stored into 1 big one.
	//KS doc: I tried to make the format as simple as possible: The first 12 bytes contains my name,
	//"KenSilverman". The next 4 bytes is the number of files that were compacted into the
    //group file. Then for each file, there is a 16 byte structure, where the first 12
    //bytes are the filename, and the last 4 bytes are the file's size. The rest of the
    //group file is just the raw data packed one after the other in the same order as the list
    //of files. - ken
    
    // Check the magic number (12 bytes header).
    if ((buf[0] != 'K') || (buf[1] != 'e') || (buf[2] != 'n') ||
        (buf[3] != 'S') || (buf[4] != 'i') || (buf[5] != 'l') ||
        (buf[6] != 'v') || (buf[7] != 'e') || (buf[8] != 'r') ||
        (buf[9] != 'm') || (buf[10] != 'a') || (buf[11] != 'n')){
        printf("Error: File %s is not a GRP archive.\n",filename);
        return(-1);
    }
    
    
	
    
    // The next 4 bytes of the header feature the number of files in the GRP archive.
    archive->numFiles = BUILDSWAP_INTEL32(*((int32_t *)&buf[12]));
    
    
    archive->gfilelist = kmalloc(archive->numFiles * sizeof(grpIndexEntry_t));
    archive->fileOffsets = kmalloc(archive->numFiles * sizeof(int32_t));
    archive->filesizes = kmalloc(archive->numFiles * sizeof(int32_t));
    
    // Load the full index 16 bytes per file (12bytes for name + 4 bytes for the size).
    read(archive->fileDescriptor,archive->gfilelist, archive->numFiles * 16);

    //Initialize all file offset and pointers.
    j = 12 + 4 + archive->numFiles * sizeof(grpIndexEntry_t);
    for(i=0;i<archive->numFiles;i++){
        
        k = BUILDSWAP_INTEL32(*((int32_t *)&archive->gfilelist[i][12])); // get size
        
        // Now that the filesize has been read, we can replace it with '0' and hence have a
        // valid, null terminated character string that will be usable.
        archive->gfilelist[i][12] = '\0';
        archive->filesizes[i] = k;
        archive->fileOffsets[i] = j; // absolute offset list of all files.
        j += k;
    }
    //archive->fileOffsets[archive->numFiles-1] = j;
	

	// Compute CRC32 of the whole grp and implicitely caches the GRP in memory through windows caching service.
    // Rewind the fileDescriptor
	lseek(archive->fileDescriptor, 0, SEEK_SET);

	//i = 1000000;
	//groupfil_memory[numgroupfiles] = malloc(i);
    
    //Load the full GRP in RAM.
	//uint8_t         crcBuffer[ 1 << 20]     ;
	uint8_t *crcBuffer = malloc((1 << 20)*sizeof(uint8_t));
//	while((j=read(archive->fileDescriptor, crcBuffer, /*sizeof(crcBuffer)*/(1 << 20)*sizeof(uint8_t) ))){
/*		archive->crc32 = crc32_update(crcBuffer,j,archive->crc32);
		SDL_UnlockDisplay();
		SDL_LockDisplay();
		printf(".");
		fflush(stdout);
	}
*/	SDL_UnlockDisplay();
	printf("\n");
    free(crcBuffer);

// Instead of CRCs, use filesizes...
	int32_t length = filelength(archive->fileDescriptor);
	if(length == 44356548)
	{
		archive->crc32=CRC_BASE_GRP_ATOMIC_15;
		printf("Version: Atomic 1.5\n");
	} else if(length == 11035779)
	{
		archive->crc32=CRC_BASE_GRP_SHAREWARE_13;
		printf("Version: Shareware 1.3\n");
	} else {
		archive->crc32=CRC_BASE_GRP_ATOMIC_15;
	}
/*
	archive->crc32=CRC_BASE_GRP_SHAREWARE_13 
	archive->crc32=CRC_BASE_GRP_FULL_13 
	archive->crc32=CRC_BASE_GRP_PLUTONIUM_14 
	archive->crc32=CRC_BASE_GRP_ATOMIC_15 
*/

    // The game layer seems to absolutely need to access an array int[4] groupefil_crc32
    // so we need to store the crc32 in there too.
    groupefil_crc32[grpSet.num] = archive->crc32;
    
	//free(groupfil_memory[numgroupfiles]);
	//groupfil_memory[numgroupfiles] = 0;
    
    grpSet.num++;

	return(grpSet.num-1);
    
}

void uninitgroupfile(void)
{
	int i;
    
	for( i=0 ; i < grpSet.num ;i++){
        free(grpSet.archives[i].gfilelist);
        free(grpSet.archives[i].fileOffsets);
        free(grpSet.archives[i].filesizes);
        memset(&grpSet.archives[i], 0, sizeof(grpArchive_t));
    }
    
}

void crc32_table_gen(unsigned int* crc32_table) /* build CRC32 table */
{
    unsigned int crc, poly;
    int	i, j;
    
    poly = 0xEDB88320L;
    for (i = 0; i < 256; i++)
    {
		crc = i;
		for (j = 8; j > 0; j--)
		{
			if (crc & 1)
				crc = (crc >> 1) ^ poly;
			else
				crc >>= 1;
		}
		crc32_table[i] = crc;
    }
}

unsigned int crc32(uint8_t  *buf, unsigned int length)
{
	unsigned int initial_crc;
    
	initial_crc = 0;
	return(crc32_update(buf, length, initial_crc));
}

unsigned int crc32_update(uint8_t  *buf, unsigned int length, unsigned int crc_to_update)
{
	unsigned int crc32_table[256];
    
	crc32_table_gen(crc32_table);
    
	crc_to_update ^= 0xFFFFFFFF;
    
	while (length--)
		crc_to_update = crc32_table[(crc_to_update ^ *buf++) & 0xFF] ^ (crc_to_update >> 8);
	
	return crc_to_update ^ 0xFFFFFFFF;
}


/*
 *                                      16   12   5
 * this is the CCITT CRC 16 polynomial X  + X  + X  + 1.
 * This is 0x1021 when x is 2, but the way the algorithm works
 * we use 0x8408 (the reverse of the bit pattern).  The high
 * bit is always assumed to be set, thus we only use 16 bits to
 * represent the 17 bit value.
 */

#define POLY 0x8408   /* 1021H bit reversed */

uint16_t crc16(uint8_t  *data_p, uint16_t length)
{
    uint8_t  i;
    unsigned int data;
    unsigned int crc = 0xffff;
    
    if (length == 0)
        return (~crc);
    do
    {
        for (i=0, data=(unsigned int)0xff & *data_p++;
             i < 8;
             i++, data >>= 1)
        {
            if ((crc & 0x0001) ^ (data & 0x0001))
                crc = (crc >> 1) ^ POLY;
            else  crc >>= 1;
        }
    } while (--length);
    
    crc = ~crc;
    data = crc;
    crc = (crc << 8) | (data >> 8 & 0xff);
    
    return (crc);
}

// The engine can open files transparently on the filesystem or on the GRPsystem
enum fileType_e{ SYSTEM_FILE, GRP_FILE} ;

//An entry in the array tracking open files
typedef struct openFile_s{
    enum fileType_e type ;
    int fd        ;  //Either the fileDescriptor or the fileIndex in a GRP depending on the type.
    int cursor    ;  //lseek cursor
    int grpID     ;  //GRP id
    int used      ;  //Marker 1=used
} openFile_t;


#define MAXOPENFILES 64
EXT_RAM_ATTR static openFile_t openFiles[MAXOPENFILES];

int32_t kopen4load(const char  *filename, int openOnlyFromGRP){
    //printf("File: %s\n", filename);
	int32_t     i, k;
    int32_t     newhandle;

    grpArchive_t* archive;
    
    //Search a free slot
	newhandle = MAXOPENFILES-1;
	while (openFiles[newhandle].used && newhandle >= 0)
		newhandle--;
	

    if (newhandle < 0)
        Error(EXIT_FAILURE, "Too Many files open!\n");
    
	SDL_LockDisplay();
    //Try to look in the filesystem first. In this case fd = filedescriptor.
    if(!openOnlyFromGRP){
        
        openFiles[newhandle].fd = open(filename,O_BINARY|O_RDONLY);
        
        if (openFiles[newhandle].fd != -1){
            openFiles[newhandle].type = SYSTEM_FILE;
            openFiles[newhandle].cursor = 0;
            openFiles[newhandle].used = 1;
			SDL_UnlockDisplay();
            return(newhandle); 
        }
    }
	SDL_UnlockDisplay();

    //Try to look in the GRP archives. In this case fd = index of the file in the GRP.
	for(k=grpSet.num-1;k>=0;k--)
	{
        archive = &grpSet.archives[k];
        
        for(i=archive->numFiles-1;i>=0;i--){
               
            if (!strncasecmp((char*)archive->gfilelist[i],filename,12)){
                
                openFiles[newhandle].type = GRP_FILE;
                openFiles[newhandle].used = 1;
                openFiles[newhandle].cursor = 0;
                openFiles[newhandle].fd = i;
                openFiles[newhandle].grpID = k;                
                return(newhandle);
            }
        }
	}
    
	return(-1);
    
}

int32_t kread(int32_t handle, void *buffer, int32_t leng){
    
    openFile_t      * openFile ;
    grpArchive_t    * archive  ;
    
    openFile = &openFiles[handle];
    
    if (!openFile->used){
        printf("Invalide handle. Unrecoverable error.\n");
        getchar();
        exit(0);
    }
    SDL_LockDisplay();
    //FILESYSTEM ? OS takes care of it !
    if (openFile->type == SYSTEM_FILE){
		int32_t ret = read(openFile->fd,buffer,leng);
		SDL_UnlockDisplay();
        return ret;
    }
    
    //File is actually in the GRP
    archive = & grpSet.archives[openFile->grpID];
        
    lseek(archive->fileDescriptor,
          archive->fileOffsets[openFile->fd] + openFile->cursor,
          SEEK_SET);
    
    //Adjust leng so we cannot read more than filesystem-cursor location.
    leng = min(leng,archive->filesizes[openFile->fd]-openFile->cursor);
    
    leng = read(archive->fileDescriptor,buffer,leng);
   
    SDL_UnlockDisplay();
    openFile->cursor += leng;
	
    return leng;
    
}

int kread16(int32_t handle, short *buffer){
    if (kread(handle, buffer, 2) != 2)
        return(0);
    
    *buffer = BUILDSWAP_INTEL16(*buffer);
    return(1);
}

int kread32(int32_t handle, int32_t *buffer){
    if (kread(handle, buffer, 4) != 4)
        return(0);
    
    *buffer = BUILDSWAP_INTEL32(*buffer);
    return(1);
}

int kread8(int32_t handle, uint8_t  *buffer){
    if (kread(handle, buffer, 1) != 1)
        return(0);
    
    return(1);
}

int32_t klseek(int32_t handle, int32_t offset, int whence){
    
    grpArchive_t* archive;
	
    if (!openFiles[handle].used){
        printf("Invalide handle. Unrecoverable error.\n");
        getchar();
        exit(0);
    }
    
    // FILESYSTEM ? OS will take care of it.
    if (openFiles[handle].type == SYSTEM_FILE){
		SDL_LockDisplay();
		int32_t ret = lseek(openFiles[handle].fd,offset,whence);
		SDL_UnlockDisplay();
        return ret;
    }
    
    archive = & grpSet.archives [   openFiles[handle].grpID ];
	
    switch(whence){
        case SEEK_SET: openFiles[handle].cursor = offset; break;
        case SEEK_END: openFiles[handle].cursor = archive->filesizes[openFiles[handle].fd]; break;
        case SEEK_CUR: openFiles[handle].cursor += offset; break;
    }
    
    return(openFiles[handle].cursor);
	
	
}

int32_t filelength(int32_t fd){
    struct stat stats;
	SDL_LockDisplay();
    fstat(fd, &stats);
	SDL_UnlockDisplay();
    return (int32_t )stats.st_size;
}

int32_t kfilelength(int32_t handle)
{
	openFile_t* openFile = &openFiles[handle];
    
    if (!openFile->used){
        printf("Invalide handle. Unrecoverable error.\n");
        getchar();
        exit(0);
    }
    
    if (openFile->type == SYSTEM_FILE){
        return(filelength(openFile->fd));
    }
    
    else{
        grpArchive_t* archive = &grpSet.archives[ openFile->grpID ];
        return archive->filesizes[openFile->fd];
    }
    
}

void kclose(int32_t handle)
{
    openFile_t* openFile = &openFiles[handle];
    
    //This is a typical handle for a non existing file.
    if (handle == -1)
        return;
    
    if (!openFile->used){
        printf("Invalide handle. Unrecoverable error.\n");
        getchar();
        exit(0);
    }
    SDL_LockDisplay();
    if (openFile->type == SYSTEM_FILE){
        close(openFile->fd);
    }
	SDL_UnlockDisplay();
    memset(openFile, 0, sizeof(openFile_t));
    
}




/* Internal LZW variables */
#define LZWSIZE 16384           /* Watch out for shorts! */
static uint8_t  *lzwbuf1, *lzwbuf4, *lzwbuf5;
static uint8_t  lzwbuflock[5];
static short *lzwbuf2, *lzwbuf3;



int32_t compress(uint8_t  *lzwinbuf, int32_t uncompleng, uint8_t  *lzwoutbuf)
{
	int32_t i, addr, newaddr, addrcnt, zx, *longptr;
	int32_t bytecnt1, bitcnt, numbits, oneupnumbits;
	short *shortptr;
    
	for(i=255;i>=0;i--) { lzwbuf1[i] = (uint8_t ) i; lzwbuf3[i] = (short) ((i+1)&255); }
	clearbuf((void *) (lzwbuf2),256>>1,0xffffffff);
	clearbuf((void *) (lzwoutbuf),((uncompleng+15)+3)>>2,0L);
    
	addrcnt = 256; bytecnt1 = 0; bitcnt = (4<<3);
	numbits = 8; oneupnumbits = (1<<8);
	do
	{
		addr = lzwinbuf[bytecnt1];
		do
		{
			bytecnt1++;
			if (bytecnt1 == uncompleng) break;
			if (lzwbuf2[addr] < 0) {lzwbuf2[addr] = (short) addrcnt; break;}
			newaddr = lzwbuf2[addr];
			while (lzwbuf1[newaddr] != lzwinbuf[bytecnt1])
			{
				zx = lzwbuf3[newaddr];
				if (zx < 0) {lzwbuf3[newaddr] = (short) addrcnt; break;}
				newaddr = zx;
			}
			if (lzwbuf3[newaddr] == addrcnt) break;
			addr = newaddr;
		} while (addr >= 0);
		lzwbuf1[addrcnt] = lzwinbuf[bytecnt1];
		lzwbuf2[addrcnt] = -1;
		lzwbuf3[addrcnt] = -1;
        
		longptr = (int32_t *)&lzwoutbuf[bitcnt>>3];
		longptr[0] |= (addr<<(bitcnt&7));
		bitcnt += numbits;
		if ((addr&((oneupnumbits>>1)-1)) > ((addrcnt-1)&((oneupnumbits>>1)-1)))
			bitcnt--;
        
		addrcnt++;
		if (addrcnt > oneupnumbits) { numbits++; oneupnumbits <<= 1; }
	} while ((bytecnt1 < uncompleng) && (bitcnt < (uncompleng<<3)));
    
	longptr = (int32_t *)&lzwoutbuf[bitcnt>>3];
	longptr[0] |= (addr<<(bitcnt&7));
	bitcnt += numbits;
	if ((addr&((oneupnumbits>>1)-1)) > ((addrcnt-1)&((oneupnumbits>>1)-1)))
		bitcnt--;
    
	shortptr = (short *)lzwoutbuf;
	shortptr[0] = (short)uncompleng;
	if (((bitcnt+7)>>3) < uncompleng)
	{
		shortptr[1] = (short)addrcnt;
		return((bitcnt+7)>>3);
	}
	shortptr[1] = (short)0;
	for(i=0;i<uncompleng;i++) lzwoutbuf[i+4] = lzwinbuf[i];
	return(uncompleng+4);
}

int32_t uncompress(uint8_t  *lzwinbuf, int32_t compleng, uint8_t  *lzwoutbuf)
{
	int32_t strtot, currstr, numbits, oneupnumbits;
	int32_t i, dat, leng, bitcnt, outbytecnt, *longptr;
	short *shortptr;
    
	shortptr = (short *)lzwinbuf;
	strtot = (int32_t )shortptr[1];
	if (strtot == 0)
	{
		copybuf((void *)((lzwinbuf)+4),(void *)((lzwoutbuf)),((compleng-4)+3)>>2);
		return((int32_t )shortptr[0]); /* uncompleng */
	}
	for(i=255;i>=0;i--) { lzwbuf2[i] = (short) i; lzwbuf3[i] = (short) i; }
	currstr = 256; bitcnt = (4<<3); outbytecnt = 0;
	numbits = 8; oneupnumbits = (1<<8);
	do
	{
		longptr = (int32_t *)&lzwinbuf[bitcnt>>3];
		dat = ((longptr[0]>>(bitcnt&7)) & (oneupnumbits-1));
		bitcnt += numbits;
		if ((dat&((oneupnumbits>>1)-1)) > ((currstr-1)&((oneupnumbits>>1)-1)))
        { dat &= ((oneupnumbits>>1)-1); bitcnt--; }
        
		lzwbuf3[currstr] = (short) dat;
        
		for(leng=0;dat>=256;leng++,dat=lzwbuf3[dat])
			lzwbuf1[leng] = (uint8_t ) lzwbuf2[dat];
        
		lzwoutbuf[outbytecnt++] = (uint8_t ) dat;
		for(i=leng-1;i>=0;i--) lzwoutbuf[outbytecnt++] = lzwbuf1[i];
        
		lzwbuf2[currstr-1] = (short) dat; lzwbuf2[currstr] = (short) dat;
		currstr++;
		if (currstr > oneupnumbits) { numbits++; oneupnumbits <<= 1; }
	} while (currstr < strtot);
	return((int32_t )shortptr[0]); /* uncompleng */
}


void kdfread(void *buffer, size_t dasizeof, size_t count, int32_t fil)
{
	size_t i, j;
	int32_t k, kgoal;
	short leng;
	uint8_t  *ptr;
    
	lzwbuflock[0] = lzwbuflock[1] = lzwbuflock[2] = lzwbuflock[3] = lzwbuflock[4] = 200;
	if (lzwbuf1 == NULL) allocache(&lzwbuf1,LZWSIZE+(LZWSIZE>>4),&lzwbuflock[0]);
	if (lzwbuf2 == NULL) allocache((uint8_t**)&lzwbuf2,(LZWSIZE+(LZWSIZE>>4))*2,&lzwbuflock[1]);
	if (lzwbuf3 == NULL) allocache((uint8_t**)&lzwbuf3,(LZWSIZE+(LZWSIZE>>4))*2,&lzwbuflock[2]);
	if (lzwbuf4 == NULL) allocache(&lzwbuf4,LZWSIZE,&lzwbuflock[3]);
	if (lzwbuf5 == NULL) allocache(&lzwbuf5,LZWSIZE+(LZWSIZE>>4),&lzwbuflock[4]);
    
	if (dasizeof > LZWSIZE) { count *= dasizeof; dasizeof = 1; }
	ptr = (uint8_t  *)buffer;
    
	kread(fil,&leng,2); kread(fil,lzwbuf5,(int32_t )leng);
	k = 0;
	kgoal = uncompress(lzwbuf5,leng,lzwbuf4);
    
	copybufbyte(lzwbuf4,ptr,(int32_t )dasizeof);
	k += (int32_t )dasizeof;
    
	for(i=1;i<count;i++)
	{
		if (k >= kgoal)
		{
			kread(fil,&leng,2); kread(fil,lzwbuf5,(int32_t )leng);
			k = 0; kgoal = uncompress(lzwbuf5,(int32_t )leng,lzwbuf4);
		}
		for(j=0;j<dasizeof;j++) ptr[j+dasizeof] = (uint8_t ) ((ptr[j]+lzwbuf4[j+k])&255);
		k += dasizeof;
		ptr += dasizeof;
	}
	lzwbuflock[0] = lzwbuflock[1] = lzwbuflock[2] = lzwbuflock[3] = lzwbuflock[4] = 1;
}

void dfread(void *buffer, size_t dasizeof, size_t count, FILE *fil)
{
	size_t i, j;
	int32_t k, kgoal;
	short leng;
	uint8_t  *ptr;
    
	lzwbuflock[0] = lzwbuflock[1] = lzwbuflock[2] = lzwbuflock[3] = lzwbuflock[4] = 200;
	if (lzwbuf1 == NULL) allocache(&lzwbuf1,LZWSIZE+(LZWSIZE>>4),&lzwbuflock[0]);
	if (lzwbuf2 == NULL) allocache((uint8_t**)&lzwbuf2,(LZWSIZE+(LZWSIZE>>4))*2,&lzwbuflock[1]);
	if (lzwbuf3 == NULL) allocache((uint8_t**)&lzwbuf3,(LZWSIZE+(LZWSIZE>>4))*2,&lzwbuflock[2]);
	if (lzwbuf4 == NULL) allocache(&lzwbuf4,LZWSIZE,&lzwbuflock[3]);
	if (lzwbuf5 == NULL) allocache(&lzwbuf5,LZWSIZE+(LZWSIZE>>4),&lzwbuflock[4]);
    
	if (dasizeof > LZWSIZE) {
        count *= dasizeof;
        dasizeof = 1;
    }
    
	ptr = (uint8_t  *)buffer;
    SDL_LockDisplay();
	fread(&leng,2,1,fil);
    fread(lzwbuf5,(int32_t )leng,1,fil);
    
	k = 0;
    kgoal = uncompress(lzwbuf5,(int32_t )leng,lzwbuf4);
    
	copybufbyte(lzwbuf4,ptr,(int32_t )dasizeof);
	k += (int32_t )dasizeof;
    
	for(i=1;i<count;i++)
	{
		if (k >= kgoal)
		{
			fread(&leng,2,1,fil); fread(lzwbuf5,(int32_t )leng,1,fil);
			k = 0; kgoal = uncompress(lzwbuf5,(int32_t )leng,lzwbuf4);
		}
		for(j=0;j<dasizeof;j++) ptr[j+dasizeof] = (uint8_t ) ((ptr[j]+lzwbuf4[j+k])&255);
		k += dasizeof;
		ptr += dasizeof;
	}
	lzwbuflock[0] = lzwbuflock[1] = lzwbuflock[2] = lzwbuflock[3] = lzwbuflock[4] = 1;
	SDL_UnlockDisplay();
}

void dfwrite(void *buffer, size_t dasizeof, size_t count, FILE *fil)
{
	size_t i, j, k;
	short leng;
	uint8_t  *ptr;
    
	lzwbuflock[0] = lzwbuflock[1] = lzwbuflock[2] = lzwbuflock[3] = lzwbuflock[4] = 200;
	if (lzwbuf1 == NULL) allocache(&lzwbuf1,LZWSIZE+(LZWSIZE>>4),&lzwbuflock[0]);
	if (lzwbuf2 == NULL) allocache((uint8_t**)&lzwbuf2,(LZWSIZE+(LZWSIZE>>4))*2,&lzwbuflock[1]);
	if (lzwbuf3 == NULL) allocache((uint8_t**)&lzwbuf3,(LZWSIZE+(LZWSIZE>>4))*2,&lzwbuflock[2]);
	if (lzwbuf4 == NULL) allocache(&lzwbuf4,LZWSIZE,&lzwbuflock[3]);
	if (lzwbuf5 == NULL) allocache(&lzwbuf5,LZWSIZE+(LZWSIZE>>4),&lzwbuflock[4]);
    
	if (dasizeof > LZWSIZE) { count *= dasizeof; dasizeof = 1; }
	ptr = (uint8_t  *)buffer;
    
	copybufbyte(ptr,lzwbuf4,(int32_t )dasizeof);
	k = dasizeof;
    SDL_LockDisplay();
	if (k > LZWSIZE-dasizeof)
	{
		leng = (short)compress(lzwbuf4,k,lzwbuf5); k = 0;
		fwrite(&leng,2,1,fil); fwrite(lzwbuf5,(int32_t )leng,1,fil);
	}
    
	for(i=1;i<count;i++)
	{
		for(j=0;j<dasizeof;j++) lzwbuf4[j+k] = (uint8_t ) ((ptr[j+dasizeof]-ptr[j])&255);
		k += dasizeof;
		if (k > LZWSIZE-dasizeof)
		{
			leng = (short)compress(lzwbuf4,k,lzwbuf5); k = 0;
			fwrite(&leng,2,1,fil); fwrite(lzwbuf5,(int32_t )leng,1,fil);
		}
		ptr += dasizeof;
	}
	if (k > 0)
	{
		leng = (short)compress(lzwbuf4,k,lzwbuf5);
		fwrite(&leng,2,1,fil); fwrite(lzwbuf5,(int32_t )leng,1,fil);
	}
	lzwbuflock[0] = lzwbuflock[1] = lzwbuflock[2] = lzwbuflock[3] = lzwbuflock[4] = 1;
	SDL_UnlockDisplay();
}



//int SafeFileExists ( const char  * _filename );
int32_t TCkopen4load(const char  *filename, int readfromGRP)
{
	char  fullfilename[512];
	int32_t result = 0;
    
	if(game_dir[0] != '\0' && !readfromGRP)
	{
		sprintf(fullfilename, "%s\\%s", game_dir, filename);
		if (!SafeFileExists(fullfilename)) // try root
			sprintf(fullfilename, "%s", filename);
	}
	else
	{
		sprintf(fullfilename, "%s", filename);
	}
    
	result = kopen4load(fullfilename, readfromGRP);
    
	return result;
}


void   setGameDir(char* gameDir){
    if (gameDir == NULL)
        return;
    
    strncpy(game_dir,gameDir,sizeof(game_dir));
}

char*  getGameDir(void){
    return game_dir;
}

int      getGRPcrc32(int grpID){
    
    return grpSet.archives[grpID].crc32;
}
