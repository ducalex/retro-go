#ifndef H_LEVEL
#define H_LEVEL

#include "common.h"
#include "stream.h"

extern Level level;

#ifdef __32X__
    extern uint8 gLightmap[256 * 32]; // SDRAM 8k at 0x6000000
#else
    #ifndef MODEHW
        extern uint8 gLightmap[256 * 32]; // IWRAM 8k
    #endif
#endif

extern ItemObj *items;

#ifdef ROM_READ
extern Texture *textures;
extern Sprite *sprites;
extern FixedCamera *cameras;
extern Box *boxes;
#endif

extern Room *rooms;
extern Model *models;
extern const Mesh** meshes;
extern StaticMesh *staticMeshes;

extern int32 gBrightness;

#if (USE_FMT & (LVL_FMT_PHD | LVL_FMT_PSX))
    extern uint8 *gLevelData;
#endif

#if (USE_FMT & LVL_FMT_PKD)
    #include "fmt/pkd.h"
#endif

#if (USE_FMT & LVL_FMT_PHD)
    #include "fmt/phd.h"
#endif

#if (USE_FMT & LVL_FMT_PSX)
    #include "fmt/psx.h"
#endif

bool readLevelStream(DataStream& f)
{
#ifdef CPU_BIG_ENDIAN
    f.bigEndian = true;
#endif

#if (USE_FMT & LVL_FMT_PKD)
    if (read_PKD(f))
        return true;
#endif

#if (USE_FMT & LVL_FMT_PHD)
    if (read_PHD(f))
        return true;
#endif

#if (USE_FMT & LVL_FMT_PSX)
    if (read_PSX(f))
        return true;
#endif

    return false;
}

TrackID getAmbientTrack()
{
    return gLevelInfo[gLevelID].track;
}

bool isCutsceneLevel()
{
    return (gLevelID == LVL_TR1_CUT_1) ||
           (gLevelID == LVL_TR1_CUT_2) ||
           (gLevelID == LVL_TR1_CUT_3) ||
           (gLevelID == LVL_TR1_CUT_4);
}

void updateLevel(int32 frames)
{
    // Placeholder or copy from engine if needed
}

#endif
