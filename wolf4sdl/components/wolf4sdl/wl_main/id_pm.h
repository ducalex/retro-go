#ifndef __ID_PM__
#define __ID_PM__

#ifdef USE_HIRES
#define PMPageSize 16384
#else
#define PMPageSize 4096
#endif

extern int ChunksInFile;
extern int PMSpriteStart;
extern int PMSoundStart;

extern bool PMSoundInfoPagePadded;

// ChunksInFile+1 pointers to page starts.
// The last pointer points one byte after the last page.
extern uint8_t **PMPages;

void PM_Startup();
void PM_Shutdown();

static inline uint32_t PM_GetPageSize(int page)
{
    if(page < 0 || page >= ChunksInFile)
        Quit("PM_GetPageSize: Tried to access illegal page: %i", page);
    return (uint32_t) (PMPages[page + 1] - PMPages[page]);
}

static inline uint8_t *PM_GetPage(int page)
{
    if(page < 0 || page >= ChunksInFile)
        Quit("PM_GetPage: Tried to access illegal page: %i", page);
    return PMPages[page];
}

static inline uint8_t *PM_GetEnd()
{
    return PMPages[ChunksInFile];
}

static inline byte *PM_GetTexture(int wallpic)
{
    return PM_GetPage(wallpic);
}

static inline uint16_t *PM_GetSprite(int shapenum)
{
    // correct alignment is enforced by PM_Startup()
    return (uint16_t *) (void *) PM_GetPage(PMSpriteStart + shapenum);
}

static inline byte *PM_GetSound(int soundpagenum)
{
    return PM_GetPage(PMSoundStart + soundpagenum);
}

#endif
