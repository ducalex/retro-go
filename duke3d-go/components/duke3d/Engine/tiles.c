//
//  tiles.c
//  Duke3D
//
//  Created by fabien sanglard on 12-12-22.
//  Copyright (c) 2012 fabien sanglard. All rights reserved.
//

#include "tiles.h"
#include "engine.h"
#include "draw.h"
#include "filesystem.h"

#include <rg_system.h>
#include "esp_attr.h"
#include "esp_heap_caps.h"
#include <lodepng.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

char  artfilename[20];

static EXT_RAM_BSS_ATTR tile_t tiles_static[MAXTILES];
tile_t *tiles = tiles_static;

int32_t numTiles;

int32_t artversion;

uint8_t  *pic = NULL;

EXT_RAM_BSS_ATTR uint8_t  gotpic[(MAXTILES+7)>>3];
EXT_RAM_BSS_ATTR static uint8_t missingPngTile[(MAXTILES+7)>>3];

#define TILE_OVERRIDE_NAME_MAX 128
#define MAX_ART_FILES_SCAN 1000
#define ART_MISS_STREAK_STOP 32
#define PNG_HEADER_READ_SIZE 33
#define OVERRIDE_PAL_CACHE_SLOTS 4096

typedef struct tile_override_s {
    short tileId;
    char fileName[TILE_OVERRIDE_NAME_MAX];
    struct tile_override_s *next;
} tile_override_t;

static tile_override_t *tileOverridesHead = NULL;

EXT_RAM_BSS_ATTR static uint32_t overridePalCacheKey[OVERRIDE_PAL_CACHE_SLOTS];
EXT_RAM_BSS_ATTR static uint8_t overridePalCacheValue[OVERRIDE_PAL_CACHE_SLOTS];
static int overridePalCacheInit = 0;

static int ci_starts_with(const char *s, const char *prefix)
{
    while (*prefix)
    {
        if (tolower((unsigned char)*s) != tolower((unsigned char)*prefix))
            return 0;
        s++;
        prefix++;
    }
    return 1;
}

static void clear_tile_overrides(void)
{
    tile_override_t *entry = tileOverridesHead;
    while (entry)
    {
        tile_override_t *next = entry->next;
        free(entry);
        entry = next;
    }
    tileOverridesHead = NULL;
}

static tile_override_t *find_tile_override(short tileId)
{
    tile_override_t *entry = tileOverridesHead;
    while (entry)
    {
        if (entry->tileId == tileId)
            return entry;
        entry = entry->next;
    }
    return NULL;
}

static int set_tile_override(short tileId, const char *fileName)
{
    tile_override_t *entry;

    if (!fileName || !fileName[0])
        return 0;

    entry = find_tile_override(tileId);
    if (!entry)
    {
        entry = (tile_override_t *)malloc(sizeof(tile_override_t));
        if (!entry)
            return 0;
        entry->tileId = tileId;
        entry->next = tileOverridesHead;
        tileOverridesHead = entry;
    }

    strncpy(entry->fileName, fileName, TILE_OVERRIDE_NAME_MAX - 1);
    entry->fileName[TILE_OVERRIDE_NAME_MAX - 1] = '\0';
    return 1;
}

static const char *get_tile_override_file(short tileId)
{
    tile_override_t *entry = find_tile_override(tileId);
    return entry ? entry->fileName : NULL;
}

static int find_keyword_ci(const char *start, const char *end, const char *keyword, const char **out)
{
    size_t kwlen = strlen(keyword);
    const char *p;

    if ((start == NULL) || (end == NULL) || (start >= end) || (kwlen == 0))
        return 0;

    for (p = start; p + kwlen <= end; p++)
    {
        if ((p > start) && (isalnum((unsigned char)p[-1]) || p[-1] == '_'))
            continue;

        if (ci_starts_with(p, keyword))
        {
            const char after = p[kwlen];
            if (!(isalnum((unsigned char)after) || after == '_'))
            {
                if (out) *out = p;
                return 1;
            }
        }
    }

    return 0;
}

static int parse_file_token_from_block(const char *blockStart, const char *blockEnd, char *outName, size_t outNameSize)
{
    const char *kw = NULL;
    const char *p;

    if (!find_keyword_ci(blockStart, blockEnd, "file", &kw))
        return 0;

    p = kw + 4;
    while ((p < blockEnd) && isspace((unsigned char)*p))
        p++;

    if (p >= blockEnd)
        return 0;

    if ((*p == '"') || (*p == '\''))
    {
        const char quote = *p++;
        const char *start = p;
        while ((p < blockEnd) && (*p != quote))
            p++;

        if ((p <= start) || (p > blockEnd))
            return 0;

        {
            size_t len = (size_t)(p - start);
            if (len >= outNameSize)
                len = outNameSize - 1;
            memcpy(outName, start, len);
            outName[len] = '\0';
            return (len > 0);
        }
    }
    else
    {
        const char *start = p;
        while ((p < blockEnd) && !isspace((unsigned char)*p) && (*p != '}'))
            p++;

        if (p <= start)
            return 0;

        {
            size_t len = (size_t)(p - start);
            if (len >= outNameSize)
                len = outNameSize - 1;
            memcpy(outName, start, len);
            outName[len] = '\0';
            return (len > 0);
        }
    }
}

static int parse_int_token_from_block(const char *blockStart, const char *blockEnd, const char *token, long *outValue)
{
    const char *kw = NULL;
    const char *p;
    const char *parseStart;
    long value;

    if (!outValue)
        return 0;

    if (!find_keyword_ci(blockStart, blockEnd, token, &kw))
        return 0;

    p = kw + strlen(token);
    while ((p < blockEnd) && isspace((unsigned char)*p))
        p++;

    parseStart = p;
    value = strtol(p, (char **)&p, 10);
    if (p == parseStart)
        return 0;

    *outValue = value;
    return 1;
}

static void set_tile_offsets(short tileId, long xoffset, long yoffset)
{
    uint32_t flags;
    int32_t xofs8 = (int32_t)xoffset;
    int32_t yofs8 = (int32_t)yoffset;

    if ((tileId < 0) || (tileId >= MAXTILES))
        return;

    if (xofs8 < -128) xofs8 = -128;
    if (xofs8 > 127) xofs8 = 127;
    if (yofs8 < -128) yofs8 = -128;
    if (yofs8 > 127) yofs8 = 127;

    flags = (uint32_t)tiles[tileId].animFlags;
    flags &= 0xFF0000FFu;
    flags |= ((uint32_t)((uint8_t)(int8_t)xofs8) << 8);
    flags |= ((uint32_t)((uint8_t)(int8_t)yofs8) << 16);
    tiles[tileId].animFlags = (int32_t)flags;
}

static void apply_animtilerange_to_tiles(long startTile, long endTile, long speed, long animation)
{
    long i;
    int32_t frameCount;
    uint32_t animTypeBits;
    uint32_t speedBits;

    if ((startTile < 0) || (endTile < startTile) || (endTile >= MAXTILES))
        return;

    if ((speed < 0) || (speed > 15) || (animation < 0) || (animation > 3))
        return;

    frameCount = (int32_t)(endTile - startTile);
    if (frameCount > 63)
        frameCount = 63;

    animTypeBits = ((uint32_t)animation & 3u) << 6;
    speedBits = ((uint32_t)speed & 15u) << 24;

    for (i = startTile; i <= endTile; i++)
    {
        uint32_t baseFlags = ((uint32_t)tiles[i].animFlags) & 0xF0FFFF00u;
        tiles[i].animFlags = (int32_t)(baseFlags | (uint32_t)frameCount | animTypeBits | speedBits);
    }
}

static void parse_tile_overrides_from_def(void)
{
    int32_t defHandle;

    clear_tile_overrides();

    defHandle = kopen4load("duke3d.def", 1);
    if (defHandle == -1)
        return;

    {
        int32_t defSize = kfilelength(defHandle);
        char *defText;
        const char *scan;
        const char *end;

        if (defSize <= 0)
        {
            kclose(defHandle);
            return;
        }

        defText = (char *)malloc((size_t)defSize + 1);
        if (defText == NULL)
        {
            kclose(defHandle);
            return;
        }

        if (kread(defHandle, defText, defSize) != defSize)
        {
            free(defText);
            kclose(defHandle);
            return;
        }

        defText[defSize] = '\0';
        kclose(defHandle);

        scan = defText;
        end = defText + defSize;

        while (scan < end)
        {
            const char *kw = NULL;
            long tileId;
            const char *p;
            const char *braceOpen;
            const char *braceClose;
            char pngName[TILE_OVERRIDE_NAME_MAX];
            long parsedXoff = 0;
            long parsedYoff = 0;
            int hasXoff = 0;
            int hasYoff = 0;

            if (!find_keyword_ci(scan, end, "tilefromtexture", &kw))
                break;

            p = kw + strlen("tilefromtexture");
            while ((p < end) && isspace((unsigned char)*p))
                p++;

            if (p >= end)
                break;

            tileId = strtol(p, (char **)&p, 10);

            if ((tileId < 0) || (tileId >= MAXTILES))
            {
                scan = kw + 1;
                continue;
            }

            while ((p < end) && (*p != '{'))
                p++;

            if ((p >= end) || (*p != '{'))
            {
                scan = kw + 1;
                continue;
            }

            braceOpen = p + 1;
            braceClose = braceOpen;
            while ((braceClose < end) && (*braceClose != '}'))
                braceClose++;

            if (braceClose >= end)
                break;

            if (parse_file_token_from_block(braceOpen, braceClose, pngName, sizeof(pngName)))
                set_tile_override((short)tileId, pngName);

            hasXoff = parse_int_token_from_block(braceOpen, braceClose, "xoffset", &parsedXoff);
            hasYoff = parse_int_token_from_block(braceOpen, braceClose, "yoffset", &parsedYoff);
            if (hasXoff || hasYoff)
            {
                const int32_t currentXoff = (int32_t)(int8_t)((tiles[tileId].animFlags >> 8) & 255);
                const int32_t currentYoff = (int32_t)(int8_t)((tiles[tileId].animFlags >> 16) & 255);
                const long finalXoff = hasXoff ? parsedXoff : currentXoff;
                const long finalYoff = hasYoff ? parsedYoff : currentYoff;
                set_tile_offsets((short)tileId, finalXoff, finalYoff);

                if (tileId == 2544)
                {
                    printf("tilefromtexture: tile 2544 parsed offsets x=%ld y=%ld (raw x token=%s y token=%s)\n",
                           finalXoff, finalYoff,
                           hasXoff ? "present" : "missing",
                           hasYoff ? "present" : "missing");
                }
            }

            scan = braceClose + 1;
        }

        scan = defText;
        while (scan < end)
        {
            const char *kw = NULL;
            const char *p;
            const char *parseStart;
            long startTile;
            long endTile;
            long speed;
            long animation;

            if (!find_keyword_ci(scan, end, "animtilerange", &kw))
                break;

            p = kw + strlen("animtilerange");

            while ((p < end) && isspace((unsigned char)*p))
                p++;
            parseStart = p;
            startTile = strtol(p, (char **)&p, 10);
            if (p == parseStart)
            {
                scan = kw + 1;
                continue;
            }

            while ((p < end) && isspace((unsigned char)*p))
                p++;
            parseStart = p;
            endTile = strtol(p, (char **)&p, 10);
            if (p == parseStart)
            {
                scan = kw + 1;
                continue;
            }

            while ((p < end) && isspace((unsigned char)*p))
                p++;
            parseStart = p;
            speed = strtol(p, (char **)&p, 10);
            if (p == parseStart)
            {
                scan = kw + 1;
                continue;
            }

            while ((p < end) && isspace((unsigned char)*p))
                p++;
            parseStart = p;
            animation = strtol(p, (char **)&p, 10);
            if (p == parseStart)
            {
                scan = kw + 1;
                continue;
            }

            apply_animtilerange_to_tiles(startTile, endTile, speed, animation);
            scan = p;
        }

        free(defText);
    }
}

static uint32_t read_be_u32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) |
           ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) |
           ((uint32_t)p[3] << 0);
}

static int read_png_dimensions(const char *fileName, short *outWidth, short *outHeight)
{
    int32_t fileHandle;
    uint8_t header[PNG_HEADER_READ_SIZE];
    uint32_t width, height;

    if (!fileName || !outWidth || !outHeight)
        return 0;

    fileHandle = TCkopen4load(fileName, 0);
    if (fileHandle == -1)
        return 0;

    if (kread(fileHandle, header, PNG_HEADER_READ_SIZE) != PNG_HEADER_READ_SIZE)
    {
        kclose(fileHandle);
        return 0;
    }
    kclose(fileHandle);

    if (memcmp(header, "\x89PNG\r\n\x1a\n", 8) != 0)
        return 0;

    if (read_be_u32(&header[8]) != 13 || memcmp(&header[12], "IHDR", 4) != 0)
        return 0;

    width = read_be_u32(&header[16]);
    height = read_be_u32(&header[20]);

    if ((width == 0) || (height == 0) || (width > 32767) || (height > 32767))
        return 0;

    *outWidth = (short)width;
    *outHeight = (short)height;
    return 1;
}

static void prime_tile_override_metadata_from_png_headers(void)
{
    tile_override_t *entry;
    int32_t okCount = 0;
    int32_t failCount = 0;
    int32_t mismatchCount = 0;
    int32_t logBudget = 32;

    for (entry = tileOverridesHead; entry != NULL; entry = entry->next)
    {
        short w = 0, h = 0;

        if (read_png_dimensions(entry->fileName, &w, &h))
        {
            if (tiles[entry->tileId].dim.width <= 0 || tiles[entry->tileId].dim.height <= 0)
            {
                int j;
                tiles[entry->tileId].dim.width = w;
                tiles[entry->tileId].dim.height = h;

                j = 15;
                while ((j > 1) && (pow2long[j] > (int32_t)w))
                    j--;
                picsiz[entry->tileId] = (uint8_t)j;

                j = 15;
                while ((j > 1) && (pow2long[j] > (int32_t)h))
                    j--;
                picsiz[entry->tileId] += (uint8_t)(j << 4);
                okCount++;
            }
            else
            {
                if ((tiles[entry->tileId].dim.width != w) || (tiles[entry->tileId].dim.height != h))
                {
                    mismatchCount++;
                    if (logBudget > 0)
                    {
                        logBudget--;
                        printf("loadpics: override tile %d PNG=%dx%d differs from tile metadata=%dx%d (will scale at decode)\n",
                               (int)entry->tileId, (int)w, (int)h,
                               (int)tiles[entry->tileId].dim.width,
                               (int)tiles[entry->tileId].dim.height);
                    }
                }
            }
        }
        else
        {
            failCount++;
            if (logBudget > 0)
            {
                logBudget--;
                printf("loadpics: override metadata failed for tile %d file '%s'\n",
                       (int)entry->tileId, entry->fileName);
            }
        }
    }

    printf("loadpics: override metadata primed from PNG headers: new=%" PRId32 ", fail=%" PRId32 ", size-mismatch=%" PRId32 "\n",
           okCount, failCount, mismatchCount);
}

static void init_override_palette_cache(void)
{
    int i;
    for (i = 0; i < OVERRIDE_PAL_CACHE_SLOTS; i++)
        overridePalCacheKey[i] = 0xffffffffu;
    overridePalCacheInit = 1;
}

static uint8_t nearest_palette_index(uint8_t r, uint8_t g, uint8_t b)
{
    int bestIdx = 0;
    int bestDist = 0x7fffffff;
    int i;

    for (i = 0; i < 256; i++)
    {
        const int pr = palette[i * 3 + 0] << 2;
        const int pg = palette[i * 3 + 1] << 2;
        const int pb = palette[i * 3 + 2] << 2;
        const int dr = (int)r - pr;
        const int dg = (int)g - pg;
        const int db = (int)b - pb;
        const int dist = dr * dr + dg * dg + db * db;

        if (dist < bestDist)
        {
            bestDist = dist;
            bestIdx = i;
            if (dist == 0)
                break;
        }
    }

    return (uint8_t)bestIdx;
}

static uint8_t nearest_palette_index_cached(uint8_t r, uint8_t g, uint8_t b)
{
    const uint32_t key = ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
    const uint32_t slot = (key * 2654435761u) >> (32 - 12);

    if (!overridePalCacheInit)
        init_override_palette_cache();

    if (overridePalCacheKey[slot] == key)
        return overridePalCacheValue[slot];

    overridePalCacheKey[slot] = key;
    overridePalCacheValue[slot] = nearest_palette_index(r, g, b);
    return overridePalCacheValue[slot];
}

static int try_loadtile_from_override_png(short tilenume)
{
    int32_t fileHandle;
    int32_t fileSize;
    uint8_t *pngBytes = NULL;
    unsigned char *rgba = NULL;
    unsigned width = 0;
    unsigned height = 0;
    unsigned err;
    uint8_t *dst;
    int32_t pixelCount;
    int32_t x, y;
    int32_t targetWidth;
    int32_t targetHeight;

    const char *overrideFile;
    const uint8_t missing_mask = (uint8_t)(1u << ((unsigned)tilenume & 7u));
    const uint32_t missing_idx = (uint32_t)tilenume >> 3;

    if ((tilenume < 0) || (tilenume >= MAXTILES))
        return 0;

    if (missingPngTile[missing_idx] & missing_mask)
    {
        /* Retry roughly once per second at 120 ticks/s. */
        if ((((uint32_t)totalclock ^ (uint32_t)(uint16_t)tilenume) & 127u) != 0)
            return 0;
        missingPngTile[missing_idx] &= (uint8_t)~missing_mask;
    }

    if (tiles[tilenume].dim.width <= 0 || tiles[tilenume].dim.height <= 0)
        return 0;

    overrideFile = get_tile_override_file(tilenume);
    if (!overrideFile)
        return 0;

    if (tiles[tilenume].data != NULL)
        return 1;

    fileHandle = TCkopen4load(overrideFile, 0);
    if (fileHandle == -1)
        return 0;

    fileSize = kfilelength(fileHandle);
    if (fileSize <= 0)
    {
        missingPngTile[missing_idx] |= missing_mask;
        kclose(fileHandle);
        return 0;
    }

    pngBytes = (uint8_t *)malloc((size_t)fileSize);
    if (pngBytes == NULL)
    {
        missingPngTile[missing_idx] |= missing_mask;
        kclose(fileHandle);
        return 0;
    }

    if (kread(fileHandle, pngBytes, fileSize) != fileSize)
    {
        missingPngTile[missing_idx] |= missing_mask;
        free(pngBytes);
        kclose(fileHandle);
        return 0;
    }
    kclose(fileHandle);

    err = lodepng_decode32(&rgba, &width, &height, pngBytes, (size_t)fileSize);
    free(pngBytes);
    if (err != 0 || rgba == NULL)
    {
        missingPngTile[missing_idx] |= missing_mask;
        return 0;
    }

    if ((width == 0) || (height == 0) || (width > 32767) || (height > 32767) ||
        ((uint64_t)width * (uint64_t)height > 0x7fffffffULL))
    {
        missingPngTile[missing_idx] |= missing_mask;
        free(rgba);
        return 0;
    }

    targetWidth = (tiles[tilenume].dim.width > 0) ? (int32_t)tiles[tilenume].dim.width : (int32_t)width;
    targetHeight = (tiles[tilenume].dim.height > 0) ? (int32_t)tiles[tilenume].dim.height : (int32_t)height;

    if ((targetWidth <= 0) || (targetHeight <= 0) ||
        ((int64_t)targetWidth * (int64_t)targetHeight > 0x7fffffffLL))
    {
        missingPngTile[missing_idx] |= missing_mask;
        free(rgba);
        return 0;
    }

    if ((targetWidth != (int32_t)width) || (targetHeight != (int32_t)height))
    {
        static int overrideSizeWarnBudget = 128;
        if (overrideSizeWarnBudget > 0)
        {
            overrideSizeWarnBudget--;
            printf("loadtile: tile %d override '%s' PNG=%ux%u, using metadata size=%" PRId32 "x%" PRId32 " (scaled)\n",
                   (int)tilenume, overrideFile, width, height, targetWidth, targetHeight);
        }
    }

    tiles[tilenume].dim.width = (short)targetWidth;
    tiles[tilenume].dim.height = (short)targetHeight;

    pixelCount = targetWidth * targetHeight;

    {
        int j;
        j = 15;
        while ((j > 1) && (pow2long[j] > targetWidth))
            j--;
        picsiz[tilenume] = (uint8_t)j;

        j = 15;
        while ((j > 1) && (pow2long[j] > targetHeight))
            j--;
        picsiz[tilenume] += (uint8_t)(j << 4);
    }

    if (tiles[tilenume].data == NULL)
    {
        tiles[tilenume].lock = 199;
        allocache(&tiles[tilenume].data, pixelCount, (uint8_t *)&tiles[tilenume].lock);
        if (tiles[tilenume].data == NULL)
        {
            free(rgba);
            missingPngTile[missing_idx] |= missing_mask;
            return 0;
        }
    }

    dst = tiles[tilenume].data;
    for (y = 0; y < targetHeight; y++)
    {
        for (x = 0; x < targetWidth; x++)
        {
            const int32_t srcX = (x * (int32_t)width) / targetWidth;
            const int32_t srcY = (y * (int32_t)height) / targetHeight;
            const int32_t srcIndex = (srcY * (int32_t)width + srcX) * 4;
            const int32_t dstIndex = x * targetHeight + y;
            const uint8_t r = rgba[srcIndex + 0];
            const uint8_t g = rgba[srcIndex + 1];
            const uint8_t b = rgba[srcIndex + 2];
            const uint8_t a = rgba[srcIndex + 3];
            dst[dstIndex] = (a < 128) ? 255 : nearest_palette_index_cached(r, g, b);
        }
    }

    {
        static int overrideDecodeLogBudget = 128;
        if (overrideDecodeLogBudget > 0)
        {
            overrideDecodeLogBudget--;
            RG_LOGD("loadtile: decoded override tile %d from '%s' (png=%ux%u -> tile=%" PRId32 "x%" PRId32 ", %" PRId32 " px)",
                   (int)tilenume, overrideFile, width, height, targetWidth, targetHeight, pixelCount);
        }
    }

    free(rgba);
    missingPngTile[missing_idx] &= (uint8_t)~missing_mask;
    return 1;
}

void setviewtotile(short tilenume, int32_t tileWidth, int32_t tileHeight)
{
    int32_t i, j;

    /* DRAWROOMS TO TILE BACKUP&SET CODE */
    tiles[tilenume].dim.width = tileWidth;
    tiles[tilenume].dim.height = tileHeight;
    bakxsiz[setviewcnt] = max(xdimen, tileWidth);
    bakysiz[setviewcnt] = max(ydimen, tileHeight);
    bakvidoption[setviewcnt] = vidoption;
    vidoption = 2;
    bakframeplace[setviewcnt] = frameplace;
    frameplace = tiles[tilenume].data;
    bakwindowx1[setviewcnt] = windowx1;
    bakwindowy1[setviewcnt] = windowy1;
    bakwindowx2[setviewcnt] = windowx2;
    bakwindowy2[setviewcnt] = windowy2;
    copybufbyte(&startumost[windowx1],&bakumost[windowx1],(windowx2-windowx1+1)*sizeof(bakumost[0]));
    copybufbyte(&startdmost[windowx1],&bakdmost[windowx1],(windowx2-windowx1+1)*sizeof(bakdmost[0]));
    setview(0,0,tileHeight-1,tileWidth-1);
    setaspect(65536,65536);
    j = 0;
    for(i=0; i<=tileWidth; i++) {
        ylookup[i] = j;
        j += tileHeight;
    }
    bakbytesperline[setviewcnt] = bytesperline;
    setBytesPerLine(tileHeight);
    setviewcnt++;
}




void squarerotatetile(short tilenume)
{
    int32_t i, j, k;
    uint8_t  *ptr1, *ptr2;

    dimensions_t tileDim;

    tileDim.width = tiles[tilenume].dim.width;
    tileDim.height = tiles[tilenume].dim.height;

    /* supports square tiles only for rotation part */
    if (tileDim.width == tileDim.height)
    {
        k = (tileDim.width<<1);
        for(i=tileDim.width-1; i>=0; i--)
        {
            ptr1 = tiles[tilenume].data+i*(tileDim.width+1);
            ptr2 = ptr1;
            if ((i&1) != 0) {
                ptr1--;
                ptr2 -= tileDim.width;
                swapchar(ptr1,ptr2);
            }
            for(j=(i>>1)-1; j>=0; j--)
            {
                ptr1 -= 2;
                ptr2 -= k;
                swapchar2(ptr1,ptr2,tileDim.width);
            }
        }
    }
}



//1. Lock a picture in the cache system.
//2. Mark it as used in the bitvector tracker.
IRAM_ATTR void setgotpic(int32_t tilenume)
{
    if (tiles[tilenume].lock < 200)
        tiles[tilenume].lock = 199;

    gotpic[tilenume>>3] |= pow2char[tilenume&7];
}





void loadtile(short tilenume)
{
    uint8_t  *ptr;
    int32_t i, tileFilesize;




    if ((uint32_t)tilenume >= (uint32_t)MAXTILES)
        return;

    if (try_loadtile_from_override_png(tilenume))
        return;

    tileFilesize = tiles[tilenume].dim.width * tiles[tilenume].dim.height;

    if (tileFilesize <= 0)
        return;

    i = tilefilenum[tilenume];
    if (i != artfilnum){
        if (artfil != -1)
            kclose(artfil);
        artfilnum = i;
        artfilplc = 0L;

        artfilename[7] = (i%10)+48;
        artfilename[6] = ((i/10)%10)+48;
        artfilename[5] = ((i/100)%10)+48;
        artfil = TCkopen4load(artfilename,0);

        if (artfil == -1){
            RG_LOGW("loadtile: missing artfile '%s' (or PNG override) for tile %d (w=%d h=%d); synthesizing transparent fallback",
                    artfilename, (int)tilenume,
                    (int)tiles[tilenume].dim.width,
                    (int)tiles[tilenume].dim.height);

            artfilnum = -1;  // Prevent subsequent tiles from using stale -1 handle

            if (tiles[tilenume].data == NULL)
            {
                tiles[tilenume].lock = 199;
                allocache(&tiles[tilenume].data, tileFilesize, (uint8_t  *) &tiles[tilenume].lock);
            }

            if (tiles[tilenume].data != NULL)
            {
                memset(tiles[tilenume].data, 255, (size_t)tileFilesize);
            }
            else
            {
                RG_LOGW("loadtile: tile %d fallback alloc failed (%" PRId32 " bytes), leaving tile empty",
                        (int)tilenume, tileFilesize);
            }
            return;
        }

        faketimerhandler();
    }

    if (tiles[tilenume].data == NULL){
        tiles[tilenume].lock = 199;
        // printf("loadtile: %d, sizing %d, calling allocache\n", tilenume, tileFilesize);
        allocache(&tiles[tilenume].data,tileFilesize,(uint8_t  *) &tiles[tilenume].lock);
        if (tiles[tilenume].data == NULL) {
            printf("loadtile: %d, allocache FAILED! tileFilesize=%"PRId32"\n", (int)tilenume, tileFilesize);
            RG_PANIC("Tile allocation failed!");
        }

        if (artfilplc != tilefileoffs[tilenume])
        {
            klseek(artfil,tilefileoffs[tilenume]-artfilplc,SEEK_CUR);
            faketimerhandler();
        }
        ptr = tiles[tilenume].data;

        if (ptr == NULL) {
            printf("loadtile: %d, ptr is NULL after successful allocache?!\n", (int)tilenume);
            RG_PANIC("Tile pointer is NULL!");
        }

        // printf("loadtile: %d, reading %d bytes to %p\n", tilenume, tileFilesize, ptr);
        if (kread(artfil,ptr,tileFilesize) != tileFilesize) {
            printf("loadtile: %d, kread FAILED! Expected %"PRId32" bytes\n", (int)tilenume, tileFilesize);
            RG_PANIC("Tile read failed!");
        }
        faketimerhandler();
        artfilplc = tilefileoffs[tilenume]+tileFilesize;
    }
}



uint8_t* allocatepermanenttile(short tilenume, int32_t width, int32_t height)
{
    int32_t j;
    uint32_t tileDataSize;

    //Check dimensions are correct.
    if ((width <= 0) || (height <= 0) || ((uint32_t)tilenume >= (uint32_t)MAXTILES))
        return(0);

    tileDataSize = width * height;

    tiles[tilenume].lock = 255;
    allocache(&tiles[tilenume].data,tileDataSize,(uint8_t  *) &tiles[tilenume].lock);

    tiles[tilenume].dim.width = width;
    tiles[tilenume].dim.height = height;
    tiles[tilenume].animFlags = 0;

    j = 15;
    while ((j > 1) && (pow2long[j] > width))
        j--;
    picsiz[tilenume] = ((uint8_t )j);

    j = 15;
    while ((j > 1) && (pow2long[j] > height))
        j--;
    picsiz[tilenume] += ((uint8_t )(j<<4));

    return(tiles[tilenume].data);
}



int loadpics(char  *filename, char * gamedir)

{
    int32_t offscount, localtilestart, localtileend, dasiz;
    short fil, i, j;
    int32_t k;
    int32_t missingStreak = 0;
    int32_t seenAnyArt = 0;


    strcpy(artfilename,filename);

    for(i=0; i<MAXTILES; i++)
    {
        tiles[i].dim.width = 0;
        tiles[i].dim.height = 0;
        tiles[i].animFlags = 0L;
    }

    artsize = 0L;

    numtilefiles = 0;
    for (k = 0; k < MAX_ART_FILES_SCAN; k++)
    {
        artfilename[7] = (k%10)+48;
        artfilename[6] = ((k/10)%10)+48;
        artfilename[5] = ((k/100)%10)+48;

        fil = TCkopen4load(artfilename,0);
        if (fil == -1)
        {
            if (seenAnyArt)
            {
                missingStreak++;
                if (missingStreak == 1)
                    printf("loadpics: missing '%s', continuing scan for sparse ART sets...\n", artfilename);
                if (missingStreak >= ART_MISS_STREAK_STOP)
                    break;
            }
            if (!seenAnyArt && k >= ART_MISS_STREAK_STOP)
                break;
            continue;
        }

        seenAnyArt = 1;
        missingStreak = 0;
        RG_LOGD("loadpics: Loading '%s'...", artfilename);
        kread32(fil,&artversion);
        if (artversion != 1) return(-1);

        kread32(fil,&numTiles);
        kread32(fil,&localtilestart);
        kread32(fil,&localtileend);

        /*kread(fil,&tilesizx[localtilestart],(localtileend-localtilestart+1)<<1);*/
        for (i = localtilestart; i <= localtileend; i++)
            kread16(fil,&tiles[i].dim.width);

        /*kread(fil,&tilesizy[localtilestart],(localtileend-localtilestart+1)<<1);*/
        for (i = localtilestart; i <= localtileend; i++)
            kread16(fil,&tiles[i].dim.height);

        /*kread(fil,&picanm[localtilestart],(localtileend-localtilestart+1)<<2);*/
        for (i = localtilestart; i <= localtileend; i++)
            kread32(fil,&tiles[i].animFlags);

        offscount = 4+4+4+4+((localtileend-localtilestart+1)<<3);
        for(i=localtilestart; i<=localtileend; i++)
        {
            tilefilenum[i] = (short)k;
            tilefileoffs[i] = offscount;
            dasiz = tiles[i].dim.width*tiles[i].dim.height;
            offscount += dasiz;
            artsize += ((dasiz+15)&0xfffffff0);
        }
        kclose(fil);

        numtilefiles++;
    }

    printf("Art files loaded: %" PRId32 " file(s) found by sparse scan\n", numtilefiles);
    if (!seenAnyArt)
        printf("loadpics: no ART files found; relying on DEF PNG overrides and header metadata\n");

    parse_tile_overrides_from_def();
    prime_tile_override_metadata_from_png_headers();

    clearbuf(gotpic,(MAXTILES+31)>>5,0L);
    clearbuf(missingPngTile,(MAXTILES+31)>>5,0L);

    // Size the tile cache depending on the amount of available RAM
    const size_t ext_free = heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    const size_t ext_largest = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    // Reserve space for things other than tile cache;
    // Not reserving enough will result in some textures failing to load, so they will be black, but still quite playable.
    // Reserving too much space will result in little space left over for texture cache, making the game laggy.
    const int32_t reserve_bytes = 640 * 1024; // 500KB has missing textures, 750KB is fine, 640KB seems to be the sweet spot
    const int32_t dynamic_size = (int32_t)ext_largest - reserve_bytes;
    int32_t min_cache_size = dynamic_size;

    if (dynamic_size < 64 * 1024) {
        RG_LOGW("loadpics: cache target ultra low, setting to minimum of 64KB");
        min_cache_size = 64 * 1024;
    } else if (dynamic_size < 512 * 1024) {
        RG_LOGW("loadpics: cache target is low and might result in the game being slow due to frequent tile cache misses");
    } else if (dynamic_size > 1536 * 1024) {
        RG_LOGI("loadpics: cache target very high, setting to maximum of 1.5MB");
        min_cache_size = 1536 * 1024;
    }

    RG_LOGI("loadpics: dynamic cache computed=%" PRId32 "KB, adjusted target=%" PRId32 "KB (ext free=%uKB largest=%uKB reserve=%" PRId32 "KB)",
            (int32_t)(dynamic_size / 1024),
            (int32_t)(min_cache_size / 1024),
            (unsigned)(ext_free / 1024),
            (unsigned)(ext_largest / 1024),
            (int32_t)(reserve_bytes / 1024));

    cachesize = max(artsize, min_cache_size);
    cachesize = (cachesize + 15) & ~15; // Align size to 16 bytes

#ifdef CONFIG_IDF_TARGET_ESP32
    // The dynamic calculation above has already reserved 640 KiB for the
    // rest of the port. Honor its 1.5 MiB target instead of cutting the cache
    // back to 1 MiB, which causes tiles and sound effects to evict each other.
    if (cachesize > 1536 * 1024) {
        cachesize = 1536 * 1024;
    }
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
    if (cachesize > 4 * 1024 * 1024) {
        cachesize = 4 * 1024 * 1024;
    }
#elif defined(CONFIG_IDF_TARGET_ESP32P4)
    if (cachesize > 24 * 1024 * 1024) {
        cachesize = 24 * 1024 * 1024;
    }
#endif

    pic = (uint8_t  *)rg_alloc(cachesize + 16, MEM_SLOW | MEM_NOPANIC);
    while (pic == NULL)
    {
        cachesize -= 65536L;
        if (cachesize < 65536) return(-1);
        pic = (uint8_t  *)rg_alloc(cachesize + 16, MEM_SLOW | MEM_NOPANIC);
    }
    uint8_t *aligned_pic = (uint8_t *)(((uintptr_t)pic + 15) & ~15);
    initcache(aligned_pic,cachesize);

    for(i=0; i<MAXTILES; i++)
    {
        j = 15;
        while ((j > 1) && (pow2long[j] > tiles[i].dim.width))
            j--;

        picsiz[i] = ((uint8_t )j);
        j = 15;

        while ((j > 1) && (pow2long[j] > tiles[i].dim.height))
            j--;

        picsiz[i] += ((uint8_t )(j<<4));
    }

    artfil = -1;
    artfilnum = -1;
    artfilplc = 0L;

    return(0);
}


void TILE_MakeAvailable(short picID){
    if (tiles[picID].data == NULL)
        loadtile(picID);

}

void copytilepiece(int32_t tilenume1, int32_t sx1, int32_t sy1, int32_t xsiz, int32_t ysiz,
                   int32_t tilenume2, int32_t sx2, int32_t sy2)
{
    uint8_t  *ptr1, *ptr2, dat;
    int32_t xsiz1, ysiz1, xsiz2, ysiz2, i, j, x1, y1, x2, y2;

    xsiz1 = tiles[tilenume1].dim.width;
    ysiz1 = tiles[tilenume1].dim.height;

    xsiz2 = tiles[tilenume2].dim.width;
    ysiz2 = tiles[tilenume2].dim.height;


    if ((xsiz1 > 0) && (ysiz1 > 0) && (xsiz2 > 0) && (ysiz2 > 0))
    {
        TILE_MakeAvailable(tilenume1);
        TILE_MakeAvailable(tilenume2);

        x1 = sx1;
        for(i=0; i<xsiz; i++)
        {
            y1 = sy1;
            for(j=0; j<ysiz; j++)
            {
                x2 = sx2+i;
                y2 = sy2+j;
                if ((x2 >= 0) && (y2 >= 0) && (x2 < xsiz2) && (y2 < ysiz2))
                {
                    ptr1 = tiles[tilenume1].data + x1*ysiz1 + y1;
                    ptr2 = tiles[tilenume2].data + x2*ysiz2 + y2;
                    dat = *ptr1;


                    if (dat != 255)
                        *ptr2 = *ptr1;
                }

                y1++;
                if (y1 >= ysiz1) y1 = 0;
            }
            x1++;
            if (x1 >= xsiz1) x1 = 0;
        }
    }
}



/*
 FCS:   If a texture is animated, this will return the offset to add to tilenum
 in order to retrieve the texture to display.
 */
int animateoffs(int16_t tilenum)
{
    int32_t i, k, offs;

    offs = 0;

    i = (totalclocklock>>((tiles[tilenum].animFlags>>24)&15));

    if ((tiles[tilenum].animFlags&63) > 0){
        switch(tiles[tilenum].animFlags&192)
        {
            case 64:
                k = (i%((tiles[tilenum].animFlags&63)<<1));
                if (k < (tiles[tilenum].animFlags&63))
                    offs = k;
                else
                    offs = (((tiles[tilenum].animFlags&63)<<1)-k);
                break;
            case 128:
                offs = (i%((tiles[tilenum].animFlags&63)+1));
                break;
            case 192:
                offs = -(i%((tiles[tilenum].animFlags&63)+1));
        }
    }

    return(offs);
}
