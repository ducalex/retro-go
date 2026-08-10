// ID_CA.C

// this has been customized for WOLF

/*
=============================================================================

Id Software Caching Manager
---------------------------

Must be started BEFORE the memory manager, because it needs to get the headers
loaded into the data segment

=============================================================================
*/

#include <sys/types.h>
#if defined _WIN32
    #include <io.h>
#elif defined _arch_dreamcast
    #include <unistd.h>
#else
    #include "sys/uio.h"
    #include <unistd.h>
#endif

#include "wl_def.h"

#define THREEBYTEGRSTARTS

/*
=============================================================================

                             LOCAL CONSTANTS

=============================================================================
*/

typedef struct
{
    word bit0,bit1;       // 0-255 is a character, > is a pointer to a node
} huffnode;


typedef struct
{
    word RLEWtag;
    int32_t headeroffsets[100];
} mapfiletype;


/*
=============================================================================

                             GLOBAL VARIABLES

=============================================================================
*/

#define BUFFERSIZE 0x1000
static int32_t *bufferseg;

int     mapon;

word    **mapsegs;
static maptype* mapheaderseg[NUMMAPS];
byte    **audiosegs;
memptr  *grsegs;
int32_t  *grstarts;
huffnode *grhuffman;

word    RLEWtag;

int     numEpisodesMissing = 0;

/*
=============================================================================

                             LOCAL VARIABLES

=============================================================================
*/

char extension[5]; // Need a string, not constant to change cache files
char graphext[5];
char audioext[5];
static const char gheadname[] = "vgahead.";
static const char gfilename[] = "vgagraph.";
static const char gdictname[] = "vgadict.";
static const char mheadname[] = "maphead.";
static const char mfilename[] = "maptemp.";
static const char mfilecama[] = "gamemaps.";
static const char aheadname[] = "audiohed.";
static const char afilename[] = "audiot.";

void CA_CannotOpen(const char *string);

static int32_t* audiostarts; // array of offsets in audio / audiot

int    grhandle = -1;               // handle to EGAGRAPH
int    maphandle = -1;              // handle to MAPTEMP / GAMEMAPS
int    audiohandle = -1;            // handle to AUDIOT / AUDIO

int32_t   chunkcomplen,chunkexplen;

SDMode oldsoundmode;


static int32_t GRFILEPOS(const size_t idx)
{
	return grstarts[idx];
}

/*
=============================================================================

                            LOW LEVEL ROUTINES

=============================================================================
*/

void CAL_GetGrChunkLength (int chunk)
{
    __lseek(grhandle,GRFILEPOS(chunk),SEEK_SET);
    __read(grhandle,&chunkexplen,sizeof(chunkexplen));
    chunkcomplen = GRFILEPOS(chunk+1)-GRFILEPOS(chunk)-4;
}

boolean CA_WriteFile (const char *filename, void *ptr, int32_t length)
{
    const int handle =__open(filename, O_CREAT | O_WRONLY | O_BINARY, 0644);
    if (handle == -1)
        return false;

    if (!write (handle,ptr,length))
    {
        __close(handle);
        return false;
    }
    __close(handle);
    return true;
}

boolean CA_LoadFile (const char *filename, memptr *ptr)
{
    int32_t size;

    const int handle =__open(filename, O_RDONLY | O_BINARY);
    if (handle == -1)
        return false;

    size = __lseek(handle, 0, SEEK_END);
    __lseek(handle, 0, SEEK_SET);
    *ptr=malloc(size);
    CHECKMALLOCRESULT(*ptr);
    if (!read (handle,*ptr,size))
    {
        __close(handle);
        return false;
    }
    __close(handle);
    return true;
}

static void CAL_HuffExpand(byte *source, byte *dest, int32_t length, huffnode *hufftable)
{
    byte *end;
    huffnode *headptr, *huffptr;

    if(!length || !dest)
    {
        Quit("length or dest is null!");
        return;
    }

    headptr = hufftable+254;        // head node is always node 254

    end=dest+length;

    byte val = *source++;
    byte mask = 1;
    word nodeval;
    huffptr = headptr;
    while(1)
    {
        if(!(val & mask))
            nodeval = huffptr->bit0;
        else
            nodeval = huffptr->bit1;
        if(mask==0x80)
        {
            val = *source++;
            mask = 1;
        }
        else mask <<= 1;

        if(nodeval<256)
        {
            *dest++ = (byte) nodeval;
            huffptr = headptr;
            if(dest>=end) break;
        }
        else
        {
            huffptr = hufftable + (nodeval - 256);
        }
    }
}

void CAL_CarmackExpand (byte *source, word *dest, int length)
{
    word ch,chhigh,count,offset;
    byte *inptr;
    word *copyptr, *outptr;

    length/=2;

    inptr = (byte *) source;
    outptr = dest;

    while (length>0)
    {
        ch = READWORD(inptr);
        chhigh = ch>>8;
        if (chhigh == 0xa7)
        {
            count = ch&0xff;
            if (!count)
            {
                ch |= *inptr++;
                *outptr++ = ch;
                length--;
            }
            else
            {
                offset = *inptr++;
                copyptr = outptr - offset;
                length -= count;
                if(length<0) return;
                while (count--)
                    *outptr++ = *copyptr++;
            }
        }
        else if (chhigh == 0xa8)
        {
            count = ch&0xff;
            if (!count)
            {
                ch |= *inptr++;
                *outptr++ = ch;
                length --;
            }
            else
            {
                offset = READWORD(inptr);
                copyptr = dest + offset;
                length -= count;
                if(length<0) return;
                while (count--)
                    *outptr++ = *copyptr++;
            }
        }
        else
        {
            *outptr++ = ch;
            length --;
        }
    }
}

int32_t CA_RLEWCompress (word *source, int32_t length, word *dest, word rlewtag)
{
    word value,count;
    unsigned i;
    word *start,*end;

    start = dest;
    end = source + (length+1)/2;

    do
    {
        count = 1;
        value = *source++;
        while (*source == value && source<end)
        {
            count++;
            source++;
        }
        if (count>3 || value == rlewtag)
        {
            *dest++ = rlewtag;
            *dest++ = count;
            *dest++ = value;
        }
        else
        {
            for (i=1;i<=count;i++)
                *dest++ = value;
        }

    } while (source<end);

    return (int32_t)(2*(dest-start));
}

void CA_RLEWexpand (word *source, word *dest, int32_t length, word rlewtag)
{
    word value,count,i;
    word *end=dest+length/2;

    do
    {
        value = *source++;
        if (value != rlewtag)
            *dest++=value;
        else
        {
            count = *source++;
            value = *source++;
            for (i=1;i<=count;i++)
                *dest++ = value;
        }
    } while (dest<end);
}

void CAL_SetupGrFile (void)
{
    char fname[300];
    int handle;
    byte *compseg;

    snprintf(fname, sizeof(fname), "%s%s%s", datadir, gdictname, graphext);

    handle =__open(fname, O_RDONLY | O_BINARY);
    if (handle == -1)
        CA_CannotOpen(fname);

    __read(handle, grhuffman, sizeof(huffnode) * 255);
    __close(handle);

    snprintf(fname, sizeof(fname), "%s%s%s", datadir, gheadname, graphext);

    handle =__open(fname, O_RDONLY | O_BINARY);
    if (handle == -1)
        CA_CannotOpen(fname);

    long headersize = __lseek(handle, 0, SEEK_END);
    __lseek(handle, 0, SEEK_SET);

    int actualChunks = headersize / 3;

    byte *data = (byte *) malloc(headersize);
    __read(handle, data, headersize);
    __close(handle);

    const byte* d = data;
    for (int i = 0; i < actualChunks; i++)
    {
        const int32_t val = d[0] | d[1] << 8 | d[2] << 16;
        grstarts[i] = (val == 0x00FFFFFF ? -1 : val);
        d += 3;
    }
    free(data);

    snprintf(fname, sizeof(fname), "%s%s%s", datadir, gfilename, graphext);

    grhandle =__open(fname, O_RDONLY | O_BINARY);
    if (grhandle == -1)
        CA_CannotOpen(fname);

    pictable=(pictabletype *) malloc(NUMPICS*sizeof(pictabletype));
    CHECKMALLOCRESULT(pictable);
    CAL_GetGrChunkLength(STRUCTPIC);
    compseg=(byte *) malloc(chunkcomplen);
    CHECKMALLOCRESULT(compseg);
    read (grhandle,compseg,chunkcomplen);
    CAL_HuffExpand(compseg, (byte*)pictable, NUMPICS * sizeof(pictabletype), grhuffman);
    free(compseg);
}

void CAL_SetupMapFile (void)
{
    int     i;
    int handle;
    int32_t length,pos;
    char fname[300];

    snprintf(fname, sizeof(fname), "%s%s%s", datadir, mheadname, extension);

    handle =__open(fname, O_RDONLY | O_BINARY);
    if (handle == -1)
        CA_CannotOpen(fname);

    length = NUMMAPS*4+2;
    mapfiletype *tinf=(mapfiletype *) malloc(sizeof(mapfiletype));
    CHECKMALLOCRESULT(tinf);
    __read(handle, tinf, length);
    __close(handle);

    RLEWtag=tinf->RLEWtag;

#ifdef CARMACIZED
    snprintf(fname, sizeof(fname), "%s%s%s", datadir, mfilecama, extension);
#else
    snprintf(fname, sizeof(fname), "%s%s%s", datadir, mfilename, extension);
#endif

    maphandle =__open(fname, O_RDONLY | O_BINARY);
    if (maphandle == -1)
        CA_CannotOpen(fname);

    for (i=0;i<NUMMAPS;i++)
    {
        pos = tinf->headeroffsets[i];
        if (pos<0)
            continue;

        mapheaderseg[i]=(maptype *) malloc(sizeof(maptype));
        CHECKMALLOCRESULT(mapheaderseg[i]);
        __lseek(maphandle,pos,SEEK_SET);
        read (maphandle,(memptr)mapheaderseg[i],sizeof(maptype));
    }

    free(tinf);

    for (i=0;i<MAPPLANES;i++)
    {
        mapsegs[i]=(word *) malloc(maparea*2);
        CHECKMALLOCRESULT(mapsegs[i]);
    }
}

void CAL_SetupAudioFile (void)
{
    char fname[300];

    snprintf(fname, sizeof(fname), "%s%s%s", datadir, aheadname, audioext);

    void* ptr;
    if (!CA_LoadFile(fname, &ptr))
        CA_CannotOpen(fname);
    audiostarts = (int32_t*)ptr;

    snprintf(fname, sizeof(fname), "%s%s%s", datadir, afilename, audioext);

    audiohandle =__open(fname, O_RDONLY | O_BINARY);
    if (audiohandle == -1)
        CA_CannotOpen(fname);
}

void CA_Startup (void)
{
    CAL_SetupMapFile ();
    CAL_SetupGrFile ();
    CAL_SetupAudioFile ();
    mapon = -1;
}

void CA_Shutdown (void)
{
    if(maphandle != -1) __close(maphandle);
    if(grhandle != -1) __close(grhandle);
    if(audiohandle != -1) __close(audiohandle);

    for(int i=0; i<NUMCHUNKS; i++)
        UNCACHEGRCHUNK(i);
    free(pictable);

    int start = (oldsoundmode == sdm_PC) ? STARTPCSOUNDS : STARTADLIBSOUNDS;
    if(oldsoundmode != sdm_Off)
    {
        for(int i=0; i<NUMSOUNDS; i++,start++)
            UNCACHEAUDIOCHUNK(start);
    }
}

IRAM_ATTR int32_t CA_CacheAudioChunk (int chunk)
{
    int32_t pos = audiostarts[chunk];
    int32_t size = audiostarts[chunk+1]-pos;

    if (audiosegs[chunk])
        return size;

    audiosegs[chunk]=(byte *) malloc(size);
    CHECKMALLOCRESULT(audiosegs[chunk]);

    __lseek(audiohandle,pos,SEEK_SET);
    __read(audiohandle,audiosegs[chunk],size);

    return size;
}

void CA_CacheAdlibSoundChunk (int chunk)
{
    int32_t pos = audiostarts[chunk];
    int32_t size = audiostarts[chunk+1]-pos;

    if (audiosegs[chunk])
        return;

    __lseek(audiohandle, pos, SEEK_SET);
    __read(audiohandle, bufferseg, ORIG_ADLIBSOUND_SIZE - 1);

    AdLibSound *sound = (AdLibSound *) malloc(size + sizeof(AdLibSound) - ORIG_ADLIBSOUND_SIZE);
    CHECKMALLOCRESULT(sound);

    byte *ptr = (byte *) bufferseg;
    sound->common.length = READLONGWORD(ptr);
    sound->common.priority = READWORD(ptr);
    sound->inst.mChar = *ptr++;
    sound->inst.cChar = *ptr++;
    sound->inst.mScale = *ptr++;
    sound->inst.cScale = *ptr++;
    sound->inst.mAttack = *ptr++;
    sound->inst.cAttack = *ptr++;
    sound->inst.mSus = *ptr++;
    sound->inst.cSus = *ptr++;
    sound->inst.mWave = *ptr++;
    sound->inst.cWave = *ptr++;
    sound->inst.nConn = *ptr++;
    sound->inst.voice = *ptr++;
    sound->inst.mode = *ptr++;
    sound->inst.unused[0] = *ptr++;
    sound->inst.unused[1] = *ptr++;
    sound->inst.unused[2] = *ptr++;
    sound->block = *ptr++;

    __read(audiohandle, sound->data, size - ORIG_ADLIBSOUND_SIZE + 1);

    audiosegs[chunk]=(byte *) sound;
}

void CA_LoadAllSounds (void)
{
    unsigned start = 0,i;

    if (oldsoundmode != sdm_Off)
    {
        start = (oldsoundmode == sdm_PC) ? STARTPCSOUNDS : STARTADLIBSOUNDS;
        for (i=0;i<NUMSOUNDS;i++,start++)
            UNCACHEAUDIOCHUNK(start);
    }

    oldsoundmode = SoundMode;

    switch (SoundMode)
    {
        case sdm_Off:
            start = STARTADLIBSOUNDS;
            break;
        case sdm_PC:
            start = STARTPCSOUNDS;
            break;
        case sdm_AdLib:
            start = STARTADLIBSOUNDS;
            break;
    }

    if(start == STARTADLIBSOUNDS)
    {
        for (i=0;i<NUMSOUNDS;i++,start++)
            CA_CacheAdlibSoundChunk(start);
    }
    else
    {
        for (i=0;i<NUMSOUNDS;i++,start++)
            CA_CacheAudioChunk(start);
    }
}

void CAL_ExpandGrChunk (int chunk, int32_t *source)
{
    int32_t    expanded;

    if (chunk >= STARTTILE8 && chunk < STARTEXTERNS)
    {
#define BLOCK           64
#define MASKBLOCK       128

        if (chunk<STARTTILE8M)
            expanded = BLOCK*NUMTILE8;
        else if (chunk<STARTTILE16)
            expanded = MASKBLOCK*NUMTILE8M;
        else if (chunk<STARTTILE16M)
            expanded = BLOCK*4;
        else if (chunk<STARTTILE32)
            expanded = MASKBLOCK*4;
        else if (chunk<STARTTILE32M)
            expanded = BLOCK*16;
        else
            expanded = MASKBLOCK*16;
    }
    else
    {
        expanded = *source++;
    }

    grsegs[chunk]=(byte *) malloc(expanded);
    CHECKMALLOCRESULT(grsegs[chunk]);
    CAL_HuffExpand((byte *) source, (byte *) grsegs[chunk], expanded, grhuffman);
}

void CA_CacheGrChunk (int chunk)
{
    int32_t pos,compressed;
    int32_t *source;
    int  next;

    if (grsegs[chunk])
        return;

    pos = GRFILEPOS(chunk);
    if (pos<0)
        return;

    next = chunk +1;
    while (GRFILEPOS(next) == -1)
        next++;

    compressed = GRFILEPOS(next)-pos;

    __lseek(grhandle,pos,SEEK_SET);

    if (compressed<=BUFFERSIZE)
    {
        __read(grhandle,bufferseg,compressed);
        source = bufferseg;
    }
    else
    {
        source = (int32_t *) malloc(compressed);
        CHECKMALLOCRESULT(source);
        __read(grhandle,source,compressed);
    }

    CAL_ExpandGrChunk (chunk,source);

    if (compressed>BUFFERSIZE)
        free(source);
}

void CA_CacheScreen (int chunk)
{
    int32_t    pos,compressed,expanded;
    memptr  bigbufferseg;
    int32_t    *source;
    int             next;

    pos = GRFILEPOS(chunk);
    next = chunk +1;
    while (GRFILEPOS(next) == -1)
        next++;
    compressed = GRFILEPOS(next)-pos;

    __lseek(grhandle,pos,SEEK_SET);

    bigbufferseg=malloc(compressed);
    CHECKMALLOCRESULT(bigbufferseg);
    __read(grhandle,bigbufferseg,compressed);
    source = (int32_t *) bigbufferseg;

    expanded = *source++;

    byte *pic = (byte *) malloc(64000);
    CHECKMALLOCRESULT(pic);
    CAL_HuffExpand((byte *) source, pic, expanded, grhuffman);

    byte *vbuf = LOCK();
    int yoffset = (screenHeight / scaleFactor - 200) / 2 * scaleFactor;
    if(yoffset < 0) yoffset = 0;

    if(screenHeight > 200 * scaleFactor)
        SDL_FillRect(curSurface, NULL, 0);

    for(int y = 0, scy = yoffset; y < 200; y++, scy += scaleFactor)
    {
        for(int x = 0, scx = 0; x < 320; x++, scx += scaleFactor)
        {
            byte col = pic[(y * 80 + (x >> 2)) + (x & 3) * 80 * 200];
            for(unsigned i = 0; i < scaleFactor; i++)
                for(unsigned j = 0; j < scaleFactor; j++)
                    vbuf[(scy + i) * curPitch + scx + j] = col;
        }
    }
    UNLOCK();
    free(pic);
    free(bigbufferseg);
}

void CA_CacheMap (int mapnum)
{
    int32_t   pos,compressed;
    int       plane;
    word     *dest;
    memptr    bigbufferseg;
    unsigned  size;
    word     *source;
#ifdef CARMACIZED
    word     *buffer2seg;
    int32_t   expanded;
#endif

    mapon = mapnum;
    size = maparea*2;

    for (plane = 0; plane<MAPPLANES; plane++)
    {
        pos = mapheaderseg[mapnum]->planestart[plane];
        compressed = mapheaderseg[mapnum]->planelength[plane];

        dest = mapsegs[plane];

        __lseek(maphandle,pos,SEEK_SET);
        if (compressed<=BUFFERSIZE)
            source = (word *) bufferseg;
        else
        {
            bigbufferseg=malloc(compressed);
            CHECKMALLOCRESULT(bigbufferseg);
            source = (word *) bigbufferseg;
        }

        __read(maphandle,source,compressed);
#ifdef CARMACIZED
        expanded = *source;
        source++;
        buffer2seg = (word *) malloc(expanded);
        CHECKMALLOCRESULT(buffer2seg);
        CAL_CarmackExpand((byte *) source, buffer2seg,expanded);
        CA_RLEWexpand(buffer2seg+1,dest,size,RLEWtag);
        free(buffer2seg);
#else
        CA_RLEWexpand (source+1,dest,size,RLEWtag);
#endif

        if (compressed>BUFFERSIZE)
            free(bigbufferseg);
    }
}

void CA_CannotOpen(const char *string)
{
    char str[30];
    strcpy(str,"Can't open ");
    strcat(str,string);
    strcat(str,"!\n");
    Quit (str);
}

void AllocGlobals_CA(void)
{
    // Resource decompression is dominated by storage and occurs outside the
    // gameplay hot path, so its scratch buffer does not need internal RAM.
    bufferseg = (int32_t *) rg_alloc(BUFFERSIZE, MEM_SLOW);
    grsegs = (memptr *) rg_alloc(NUMCHUNKS * sizeof(memptr), MEM_SLOW);
    mapsegs = (word **) rg_alloc(MAPPLANES * sizeof(word *), MEM_SLOW);
    audiosegs = (byte **) rg_alloc(NUMSNDCHUNKS * sizeof(byte *), MEM_SLOW);
    grstarts = (int32_t *) rg_alloc((NUMCHUNKS + 1) * sizeof(int32_t), MEM_SLOW);
    grhuffman = (huffnode *) rg_alloc(255 * sizeof(huffnode), MEM_SLOW);
}
