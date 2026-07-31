#include "rt_def.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <rg_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/* ROTT expects these for its headers */
#define PLATFORM_UNIX 1

#include "rt_sound.h"
#include "rt_draw.h"
#include "rt_playr.h"
#include "rt_actor.h"
#include "rt_stat.h"
#include "rt_door.h"
#include "rt_game.h"
#include "isr.h"
#include "_rt_rand.h"
#include "sprites.h"
#include "lumpy.h"
#include "_rt_stat.h"
#include "rt_view.h"
#include "engine.h"
#include "rt_ted.h"
#include "rt_battl.h"
#include "rt_main.h"
#include "z_zone.h"
#undef boolean

#define MV_NumPanPositions 32
typedef struct
{
unsigned char left;
unsigned char right;
} Pan;

extern void Error(char *string,...);

// Memory tracking for ROTT tags
typedef struct memblock_s {
    void *ptr;
    int size;
    int tag;
    void **user;
    struct memblock_s *next;
} memblock_t;

typedef struct {
    memblock_t *block;
    uint32_t magic;
} zone_header_t;

#define ZONE_HEADER_MAGIC 0x524F5454u
#define ZONE_ALIGNMENT 16u
#define ZONE_HEADER_SIZE \
    ((sizeof(zone_header_t) + ZONE_ALIGNMENT - 1) & ~(ZONE_ALIGNMENT - 1))

#define MAX_MEM_BLOCKS 4096
static memblock_t *block_pool = NULL;
static memblock_t *free_blocks = NULL;
static memblock_t *first_block = NULL;


// Global pointers
byte * RotatedImage = NULL;
gamestorage_t *menu_game_storage = NULL;
word   (*tilemap)[MAPSIZE];
byte   (*spotvis)[MAPSIZE];
byte   (*mapseen)[MAPSIZE];
visobj_t *vislist;
objtype  **PLAYER;
playertype *PLAYERSTATE;
playertype *locplayerstate;
void     *(*actorat)[MAPSIZE];
statobj_t *(*sprites)[MAPSIZE];
statobj_t *FIRSTSTAT, *LASTSTAT;
statobj_t **DEADPLAYER;
statobj_t **BulletHoles;

volatile int *Keyboard;
volatile int *Keystate;
volatile int *KeyboardQueue;

const unsigned char *RandomTable;
elevator_t *ELEVATOR;
wall_t *switches;
animwall_t *animwalls;
int *numareatiles;
int *LightsInArea;
wallcast_t *posts;
basic_actor_sounds *BAS;
dirtype *opposite;
dirtype (*diagonal)[9];
awallinfo_t *animwallsinfo;
char (*mapnames)[23];
char (*mapfiles)[13];
int (*starthitpoints)[100];
word *mapplanes[3];
ROTTCHARS *characters;
byte *deathshapeoffset;
objtype  *new;
byte     *TRIGGER;
misc_stuff *MISCVARS;

Pan (*MV_PanTable)[ 63 + 1 ];

maskedwallobj_t **maskobjlist;

doorobj_t **doorobjlist;
pwallobj_t **pwallobjlist;
rott_boolean *areabyplayer;
int      *angletodir;
objtype  **firstareaactor;
objtype  **lastareaactor;
word     (*touchindices)[MAPSIZE];
touchplatetype **touchplate;
int      objcount;
_2Dpoint *SNAKEPATH;
byte     RANDOMACTORTYPE[100];
byte     *numactions;
touchplatetype **lastaction;
byte     (*areaconnect)[NUMAREAS];

short    *tantable;
int      *sintable;
short    *pixelangle;
int      *xstarts;
int      *ylookup;
fixed    *costable;

#ifndef NUMMAPS
#define NUMMAPS 100
#endif

extern void allocate_rott_memory(void);

static const unsigned char StaticRandomTable[ 2048 ] =
   {
   107,   65,  179,   81,  212,    1,   34,  230,
   167,  142,   82,   27,   62,   88,  140,  119,
   222,  252,  254,  160,   26,   33,   30,  234,
   162,  216,  126,   42,   20,  108,  245,  150,
   167,  145,  215,  226,  153,  184,  251,  141,
   116,  225,  201,  114,  111,   92,  223,  185,
   199,  160,  244,  190,  113,   77,  217,  239,
    15,  239,  129,  243,   21,  242,  202,  183,
    49,  151,   38,   38,  147,  179,  234,  130,
   138,  110,  228,  118,   93,   81,  253,   98,
   246,   44,   75,  161,  189,   86,   85,  204,
   169,   95,  199,  178,  186,  123,  200,  250,
   118,  242,   86,   48,    7,  205,   71,  132,
   185,  214,  192,   68,  191,  236,  175,  197,
   199,  177,  163,   57,  220,  139,  133,  182,
    91,  196,  246,   29,  177,   82,  184,  226,
   209,  151,  206,  250,  195,  119,  193,  235,
   144,  146,   58,   61,  245,   83,  204,  214,
   249,  164,  212,  172,   90,  199,  242,  182,
   228,  159,  127,   37,  209,  165,   89,  122,
    87,  254,  222,   43,  148,  205,  155,  230,
    74,  127,  238,  181,  154,  170,  232,   47,
   105,   31,   96,  166,  208,    5,  201,   73,
   244,   67,   55,  168,   84,  221,  251,   85,
    44,  198,    8,   35,  229,  122,  229,   80,
   137,   28,  202,  135,  211,   69,  100,  250,
   224,  156,   75,  128,  176,   53,  207,  157,
   241,  216,  210,  124,  163,  248,  223,  174,
   241,  235,   97,  120,   25,    3,  218,  102,
   143,  187,  202,  116,  209,  253,  227,  151,
   203,  254,  223,   65,  146,  255,  231,  237,
   162,  187,  194,   90,  198,   81,  219,   94,
    70,  158,  106,  175,   81,  239,  181,   79,
    62,  184,   21,   30,   98,  245,  233,    8,
    57,   54,  133,   95,   43,  125,  161,  109,
   167,  130,   32,   46,   72,    3,   42,   34,
   147,  139,   19,  122,   49,   23,  253,  192,
   231,  116,  165,    8,   15,   16,  223,  140,
    29,  136,    2,  114,  248,  219,  214,  168,
    36,  241,  210,   63,  230,   42,  197,  100,
   134,  159,  179,  149,   17,   96,  152,   45,
    93,  151,   26,  202,   35,   58,  189,  220,
    74,  173,  250,   76,   57,   52,   55,   14,
   205,   80,  213,  190,   14,  108,   60,   89,
   236,   41,   53,   59,  249,  237,   88,  217,
   129,  252,  169,  123,   87,   23,   91,   79,
    36,  164,   89,   24,  102,  111,  104,  115,
   116,  221,  212,   82,  166,   16,  244,   18,
    68,   73,   64,  194,   82,  115,  121,  216,
   133,   47,   85,  124,  242,  144,    1,   65,
     0,   75,   91,   83,  137,  183,  132,   15,
    66,  227,   67,   98,  186,  100,  208,  150,
    74,   61,   69,  130,  120,   56,   39,   21,
    44,  166,  180,  145,   97,  101,   68,   19,
   243,  162,  222,  228,    9,  160,  185,   10,
   168,   64,  109,  198,  113,   75,   48,   52,
   119,  163,  176,  201,   12,  246,   20,   92,
    31,  175,   33,    4,   70,  169,  218,   86,
    240,  127,    7,   38,  102,   94,  112,   90,
   114,   63,  139,  119,   45,    5,  183,  224,
   155,   13,  141,   90,   93,  177,  118,   67,
   211,  117,   58,  158,  135,   83,  236,  194,
    72,   59,  131,   40,  196,  232,  247,  132,
    18,   73,   37,  207,   50,  187,   32,  174,
    13,   34,  135,   39,   46,  192,   51,  203,
   216,    3,  188,   99,  245,  112,   61,  206,
   127,  118,  136,  186,  153,  148,  140,  106,
    55,  201,   19,    9,   27,   97,   84,   24,
    48,  227,  108,  202,    5,  190,    6,  235,
   160,  146,  123,  222,  212,  147,   27,   28,
    50,  226,  200,  221,  180,  244,   34,  146,
    41,  104,   37,  217,   10,  238,   52,  198,
     4,   29,  126,   16,   45,  124,   64,   25,
    40,   20,  138,  225,   71,  247,  128,   62,
    62,  178,   31,  251,  197,  215,  226,  193,
    28,   25,   82,   15,   21,   43,   30,   86,
    50,   99,  199,  191,   80,  171,  250,    4,
   149,   61,  163,  128,  143,   95,   76,   56,
    14,  109,   54,   84,  137,   43,   77,  255,
   204,  151,  129,    8,   20,   26,  193,   38,
   205,  125,  203,   22,   78,   47,  101,   37,
   121,  113,  141,   36,   33,   23,  131,   55,
    78,   96,  123,  228,   69,  213,   51,  147,
   195,  195,   85,   54,  225,   65,  143,   13,
   152,   60,  110,   29,  146,  179,  164,   17,
    96,   67,  156,    7,    3,   92,   26,  104,
   157,  220,  211,  154,  103,    2,  174,  136,
    83,  229,   18,  116,   66,   87,    0,   74,
    98,   58,   75,  162,  254,  177,   89,  173,
   169,  107,   12,   90,   71,  193,   53,  255,
   138,    2,  233,  163,   11,  138,  183,  209,
   119,  210,  249,  112,  113,   30,  185,  180,
    11,  171,  164,   77,    5,   99,   35,  218,
    11,  159,  221,  148,  170,  137,  129,  188,
   231,  172,  191,  135,  176,   33,  189,   68,
   190,  152,    7,  220,  246,  203,  181,   63,
   199,   91,  251,  208,  217,   39,  170,  111,
   182,  209,  216,  247,  240,  165,  153,  118,
   238,  233,  204,   12,  126,  105,  168,  234,
    38,  184,  132,  110,  145,  227,  165,   92,
   241,   24,  251,  231,  213,  126,  240,  214,
    31,   11,  223,   46,    0,   35,  218,  242,
   211,  191,   48,  110,  253,   94,  161,  139,
    42,  148,  198,  246,  239,  107,   57,    5,
   117,  252,    9,   12,   73,   77,   58,  167,
     6,  142,   32,   34,   72,   31,  243,  170,
     0,   93,  182,   56,  254,  158,  250,   23,
    77,   81,   45,   69,  142,  237,  225,   44,
   232,  142,  209,  115,   61,   38,  224,   70,
    53,   82,   10,  229,  214,  224,  101,   42,
    100,  121,  133,   85,   67,  150,  116,   91,
    79,   79,  235,   22,    1,  244,   16,  101,
    88,   25,   43,  242,   53,  106,  128,   40,
    18,    2,  196,   22,   47,   96,  130,  249,
    49,  117,  178,  171,  132,  189,   95,   66,
   153,  200,   71,   71,  215,  113,  102,  131,
   172,  235,  192,  204,  107,  127,   60,   52,
   254,   84,   20,   27,  107,   44,   72,  149,
   230,   87,   64,  144,  140,  186,  154,   49,
    76,  156,  162,  219,    9,  146,   41,   80,
    59,  142,  147,  184,  115,  158,  103,  140,
     8,  234,  157,   17,  112,   22,  163,  187,
    47,  178,  207,   63,  125,  202,  156,   30,
    97,  193,  171,  173,  204,   28,    4,  195,
   109,  211,  105,  103,  179,  221,  203,  138,
   139,  172,   14,   44,  119,  150,  185,  180,
   205,  255,  152,   94,  205,  173,   40,  134,
    13,   65,  194,  196,   68,  124,   26,  134,
    85,  228,   50,  216,  159,  254,  237,  206,
   103,   69,   90,  187,  182,  183,    2,  114,
     3,  201,    1,  129,  212,  232,   81,   33,
    56,  182,  133,  176,   19,    6,  144,   84,
    145,   24,  243,  234,   35,   25,   89,  150,
    54,  143,  207,  104,  248,  102,   41,  253,
    46,   44,  197,  208,  200,   52,  219,  238,
   223,  241,   60,  230,  196,   42,   88,   51,
   170,   25,   40,  114,   39,   45,   57,   29,
   186,   54,   36,  167,   51,  249,  194,  108,
   250,  199,   67,  212,  123,  151,   92,   32,
    46,   12,  248,  165,  223,   24,  143,  189,
   120,   57,   38,  222,  169,   17,  188,   27,
   209,  176,  215,  108,  177,   96,   50,  154,
    41,   15,   32,  181,   99,  120,   73,  225,
    49,  200,   43,  177,   72,  202,   97,   92,
   240,  110,  252,   21,   28,  192,   70,  128,
    85,   60,  127,   63,  235,  100,  155,   65,
   241,  119,   34,   80,   75,  117,  148,  102,
    15,   90,  121,  206,  104,  164,  132,   98,
    36,  108,  214,    0,   84,   94,  107,  154,
    14,   26,   23,   93,  195,  224,  162,  171,
   245,   76,  125,  158,  120,   66,   80,  130,
   183,  120,  179,  118,   31,  176,  106,  184,
   164,  111,   59,  253,  118,   62,  114,  215,
   156,  161,  127,  210,  112,  122,  204,   86,
    29,   69,  187,  218,  233,   47,  188,   91,
   105,  153,  236,  116,  135,   19,   83,  175,
    88,  166,  156,  238,  245,  227,  248,  161,
    10,  180,   22,  230,  157,  205,    6,   37,
     3,  234,  155,   49,  239,  229,  122,  223,
   234,  206,   82,  135,  190,   77,  130,  197,
   144,  226,  123,  181,  143,  208,  169,    8,
     1,  174,   97,  177,  232,  201,  210,  193,
   192,   62,  211,   28,  252,   19,  189,  229,
   191,  246,    7,  202,   35,   20,   28,   30,
    70,  221,   55,  159,  237,   10,   15,  134,
   124,  103,   73,  172,   26,  201,   13,  117,
    35,  131,  136,   66,   33,  245,  159,  207,
   225,  147,   18,  248,  126,  131,   33,  153,
   101,   60,  186,  216,  250,   11,   30,   50,
    57,   58,  151,  178,  181,  227,  128,  253,
    10,    5,  224,   48,  160,   34,    7,  111,
   124,   27,  160,   75,  137,   17,   53,   39,
   243,  236,   39,   81,  217,  122,   79,   32,
    13,  115,  191,  167,  137,   87,  228,   91,
    86,   78,   18,   22,   40,   76,   94,  129,
   119,   87,   63,  197,  203,   21,  218,  185,
   166,   49,  161,  213,   38,  111,  209,  154,
    43,  152,  134,  132,    4,  133,  108,   83,
   145,   41,   97,   74,  251,  173,   42,  213,
    46,  118,   73,   68,   21,  244,  106,  158,
    11,   90,  252,  140,   44,  157,  194,   66,
   229,   58,  215,  251,  144,  125,  146,  172,
   165,  136,  105,  102,  173,   95,  150,  169,
    78,  230,  111,  160,  196,  112,  120,  231,
   168,  152,  149,  174,  128,  232,  194,  122,
     3,   74,  231,  105,   12,  239,  180,  239,
    89,  235,   95,  157,  149,  187,    6,  203,
   188,   71,  110,    8,  126,  242,  188,  249,
    71,  123,  162,   93,   59,   65,   16,  222,
   192,   24,   27,  142,  168,  114,   89,  220,
   226,  182,  207,   94,  195,  207,  174,  228,
   142,  130,  115,  175,  154,  106,  206,   48,
   185,  147,  155,  224,  239,   64,   88,   83,
    54,  247,  113,  248,  137,   96,    5,  110,
   240,   51,    1,  208,  166,  200,  233,  167,
   230,   14,   20,  198,   36,   13,    0,  220,
   165,  104,   85,  122,    1,  255,  212,  161,
   190,   37,   86,  226,  195,  105,   78,  244,
   236,  168,   48,  211,  121,  216,   25,  144,
    16,  237,    7,   75,   12,   99,   56,  109,
   218,   87,   35,  204,  155,   61,  191,   41,
    61,   57,  121,  236,   62,   22,  187,   95,
    92,   69,  210,  163,  205,  214,   34,   84,
    73,  184,  109,  124,  231,  103,  217,  241,
    77,  175,  183,  232,  198,  164,   14,  125,
   103,  197,  249,  241,  176,   59,   48,   50,
    25,  222,  134,  199,  252,  196,   23,   19,
    78,    0,   59,  120,   91,  121,  117,  228,
    52,   68,  113,  127,  141,   67,   65,  130,
   132,  243,  160,  138,  138,  116,   10,   98,
   170,   70,  246,  206,  219,   28,   94,  226,
    29,   58,  255,   31,  141,   82,  134,  152,
   251,   84,   52,   23,  154,  139,   74,   55,
    47,  198,  167,  162,    4,   16,  220,   53,
    37,  158,  135,  136,  190,   86,  151,  178,
   238,  161,   88,  171,  148,  169,  137,  212,
   181,  146,  189,   70,   49,   79,  142,  131,
   195,  126,  149,  156,   93,   55,  164,  174,
    87,  243,   66,   92,  207,    9,  126,  123,
    40,   51,  133,  143,    9,   55,  193,  173,
    56,   64,  210,  159,    6,  178,  145,  153,
   115,  215,  185,  238,  101,  210,   98,  157,
    95,  171,  235,  217,  240,  232,  107,   53,
    54,  211,  111,  236,  100,   37,  147,   64,
    60,  215,  174,  136,  128,   98,   79,  234,
   145,   76,  197,  165,  191,  141,  114,  109,
   148,  214,   93,   12,  163,  125,  222,   96,
    71,  220,    8,  225,  200,   14,  139,  166,
   155,    9,  237,    9,  172,    6,  229,  179,
   247,   76,   24,  213,   26,  248,   56,  247,
   106,  221,   63,   20,  206,  231,  168,  196,
   208,   36,   31,  175,  178,  255,  124,  156,
   129,  186,   45,  201,    7,  112,  189,   68,
   225,   64,  150,   74,    2,  218,   62,  159,
     4,   11,  213,   40,   23,   15,   67,   52,
   182,  203,  246,  194,   72,   33,   89,  104,
    16,  131,   60,  150,  243,   21,   59,  175,
   219,  149,   56,  224,   32,  105,  153,   99,
    80,  148,  107,   99,  108,   43,  106,   41,
    32,  244,    4,  186,  253,   27,  233,  104,
   245,    5,    2,  252,  170,    2,  101,   39,
    17,  166,  221,   11,  227,  129,  103,   45,
   144,   79,   17,  139,   70,  237,  176,    6,
    76,  134,   54,   30,   69,   13,  100,  149,
   141,  227,  255,   22,  179,   61,   88,   97,
   121,   29,   24,   77,  170,  190,  242,  109,
   152,   47,   81,  140,   45,   46,  249,   78,
   180,  143,  102,   19,  208,  177,   72,   72,
   247,   51,  115,    3,  141,  117,   39,  188,
   180,  171,   83,    1,    0,   51,  100,  112,
   172,   18,  219,  136,  155,  125,  200,  131,
   157,  217,   78,   63,  213,  238,  193,  173,
   145,   46,  101,  158,   42,  140,  110,   66,
   183,  181,  233,  188,   10,  133,   36,  135,
    17,  254,  138,  233,  117,  240,  184,   80,
   113,   18,   99,  219,  240,   50,  192,  247
   };

// We need this because we moved them to pointers
void allocate_rott_memory() {
    static bool initialized = false;
    if (initialized) {
        printf("allocate_rott_memory already initialized, skipping.\n");
        return;
    }
    initialized = true;

    printf("Allocating ROTT memory structures in PSRAM via Pointer Strategy...\n");

    // Initialize the memblock_t pool in PSRAM (80KB)
    block_pool = rg_alloc(MAX_MEM_BLOCKS * sizeof(memblock_t), MEM_SLOW);
    memset(block_pool, 0, MAX_MEM_BLOCKS * sizeof(memblock_t));
    for (int i = 0; i < MAX_MEM_BLOCKS - 1; i++) {
        block_pool[i].next = &block_pool[i+1];
    }
    free_blocks = &block_pool[0];
    first_block = NULL;

    // Pre-allocate RotatedImage in PSRAM (128KB)
    RotatedImage = rg_alloc(131072, MEM_SLOW);
    memset(RotatedImage, 0, 131072);

    menu_game_storage = rg_alloc(sizeof(gamestorage_t), MEM_SLOW);
    memset(menu_game_storage, 0, sizeof(gamestorage_t));

    tilemap = rg_alloc(MAPSIZE * MAPSIZE * sizeof(word), MEM_SLOW);
    memset(tilemap, 0, MAPSIZE * MAPSIZE * sizeof(word));

    spotvis = rg_alloc(MAPSIZE * MAPSIZE * sizeof(byte), MEM_SLOW);
    memset(spotvis, 0, MAPSIZE * MAPSIZE * sizeof(byte));

    mapseen = rg_alloc(MAPSIZE * MAPSIZE * sizeof(byte), MEM_SLOW);
    memset(mapseen, 0, MAPSIZE * MAPSIZE * sizeof(byte));

    vislist = rg_alloc(MAXVISIBLE * sizeof(visobj_t), MEM_SLOW);
    memset(vislist, 0, MAXVISIBLE * sizeof(visobj_t));

    PLAYER = rg_alloc(MAXPLAYERS * sizeof(objtype *), MEM_SLOW);
    memset(PLAYER, 0, MAXPLAYERS * sizeof(objtype *));

    PLAYERSTATE = rg_alloc(MAXPLAYERS * sizeof(playertype), MEM_SLOW);
    memset(PLAYERSTATE, 0, MAXPLAYERS * sizeof(playertype));

    actorat = rg_alloc(MAPSIZE * MAPSIZE * sizeof(void *), MEM_SLOW);
    memset(actorat, 0, MAPSIZE * MAPSIZE * sizeof(void *));

    sprites = rg_alloc(MAPSIZE * MAPSIZE * sizeof(statobj_t *), MEM_SLOW);
    memset(sprites, 0, MAPSIZE * MAPSIZE * sizeof(statobj_t *));

    DEADPLAYER = rg_alloc(MAXDEAD * sizeof(statobj_t *), MEM_SLOW);
    memset(DEADPLAYER, 0, MAXDEAD * sizeof(statobj_t *));

    BulletHoles = rg_alloc(MAXBULLETS * sizeof(statobj_t *), MEM_SLOW);
    memset(BulletHoles, 0, MAXBULLETS * sizeof(statobj_t *));

    Keyboard = rg_alloc(MAXKEYBOARDSCAN * sizeof(int), MEM_SLOW);
    memset((void*)Keyboard, 0, MAXKEYBOARDSCAN * sizeof(int));

    Keystate = rg_alloc(MAXKEYBOARDSCAN * sizeof(int), MEM_SLOW);
    memset((void*)Keystate, 0, MAXKEYBOARDSCAN * sizeof(int));

    KeyboardQueue = rg_alloc(KEYQMAX * sizeof(int), MEM_SLOW);
    memset((void*)KeyboardQueue, 0, KEYQMAX * sizeof(int));

    tantable = rg_alloc(FINEANGLES * sizeof(short), MEM_FAST | MEM_NOPANIC);
    if (!tantable) tantable = rg_alloc(FINEANGLES * sizeof(short), MEM_SLOW);
    memset(tantable, 0, FINEANGLES * sizeof(short));

    sintable = rg_alloc((FINEANGLES + FINEANGLEQUAD + 1) * sizeof(int), MEM_FAST | MEM_NOPANIC);
    if (!sintable) sintable = rg_alloc((FINEANGLES + FINEANGLEQUAD + 1) * sizeof(int), MEM_SLOW);
    memset(sintable, 0, (FINEANGLES + FINEANGLEQUAD + 1) * sizeof(int));

    costable = sintable + (FINEANGLES / 4);

    pixelangle = rg_alloc(800 * sizeof(short), MEM_SLOW);
    memset(pixelangle, 0, 800 * sizeof(short));

    xstarts = rg_alloc(600 * sizeof(int), MEM_SLOW);
    memset(xstarts, 0, 600 * sizeof(int));

    ylookup = rg_alloc(600 * sizeof(int), MEM_FAST | MEM_NOPANIC);
    if (!ylookup) ylookup = rg_alloc(600 * sizeof(int), MEM_SLOW);
    memset(ylookup, 0, 600 * sizeof(int));

    // Initializing the static data in PSRAM
    unsigned char *rt = rg_alloc(2048, MEM_SLOW);
    memcpy(rt, StaticRandomTable, 2048);
    RandomTable = rt;

    BAS = rg_alloc((NUMCLASSES + 3) * sizeof(basic_actor_sounds), MEM_SLOW);
    memset(BAS, 0, (NUMCLASSES + 3) * sizeof(basic_actor_sounds));

    opposite = rg_alloc(9 * sizeof(dirtype), MEM_SLOW);
    dirtype opp_static[9] = {west,southwest,south,southeast,east,northeast,north,northwest,nodir};
    memcpy(opposite, opp_static, 9 * sizeof(dirtype));

    diagonal = rg_alloc(9 * 9 * sizeof(dirtype), MEM_SLOW);
    dirtype diag_static[9][9] =
        { {nodir,nodir,northeast,nodir,nodir,nodir,southeast,nodir,nodir},
                        {nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir},
                        {northeast,nodir,nodir,nodir,northwest,nodir,nodir,nodir,nodir},
                        {nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir},
                        {nodir,nodir,northwest,nodir,nodir,nodir,southwest,nodir,nodir},
                        {nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir},
                        {southeast,nodir,nodir,nodir,southwest,nodir,nodir,nodir,nodir},
                        {nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir},
                        {nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir} };
    memcpy(diagonal, diag_static, 9 * 9 * sizeof(dirtype));

    animwallsinfo = rg_alloc(MAXANIMWALLS * sizeof(awallinfo_t), MEM_SLOW);
    awallinfo_t static_awinfo[MAXANIMWALLS] = {
        {3,4,"FPLACE1\0"}, {3,6,"ANIMY1\0"}, {3,6,"ANIMR1\0"}, {40,4,"ANIMFAC1\0"},
        {3,4,"ANIMONE1\0"}, {3,4,"ANIMTWO1\0"}, {3,4,"ANIMTHR1\0"}, {3,4,"ANIMFOR1\0"},
        {3,6,"ANIMGW1\0"}, {3,6,"ANIMYOU1\0"}, {3,6,"ANIMBW1\0"}, {3,6,"ANIMBP1\0"},
        {3,6,"ANIMCHN1\0"}, {3,6,"ANIMFW1\0"}, {3,6,"ANIMLAT1\0"}, {3,6,"ANIMST1\0"},
        {3,6,"ANIMRP1\0"}
    };
    memcpy(animwallsinfo, static_awinfo, sizeof(static_awinfo));

    mapnames = rg_alloc(NUMMAPS * 23, MEM_SLOW);
    memset(mapnames, 0, NUMMAPS * 23);

    mapfiles = rg_alloc(NUMMAPS * 13, MEM_SLOW);
    memset(mapfiles, 0, NUMMAPS * 13);

    starthitpoints = rg_alloc(4 * 100 * sizeof(int), MEM_SLOW);
    int shp_static[4][100] = {
        {100,100,25,35,40,45,45,50,60,60,70,100,100,100,100,100,100,100,100,100},
        {100,100,35,45,50,55,55,60,70,70,80,100,100,100,100,100,100,100,100,100},
        {100,100,50,60,65,70,70,75,85,85,95,100,100,100,100,100,100,100,100,100},
        {100,100,75,85,90,95,95,100,110,110,120,100,100,100,100,100,100,100,100,100}
    };
    memcpy(starthitpoints, shp_static, 4 * 100 * sizeof(int));

    numareatiles = rg_alloc((NUMAREAS + 1) * sizeof(int), MEM_SLOW);
    memset(numareatiles, 0, (NUMAREAS + 1) * sizeof(int));

    LightsInArea = rg_alloc((NUMAREAS + 1) * sizeof(int), MEM_SLOW);
    memset(LightsInArea, 0, (NUMAREAS + 1) * sizeof(int));

    // This structure is touched by wall casting, plane clipping, and sprite
    // occlusion on every rendered frame. Keep the fixed 320-wide working set
    // internal; the former 800-entry PSRAM allocation was a desktop remnant.
    posts = rg_alloc(ROTT_POST_COUNT * sizeof(wallcast_t), MEM_FAST);
    memset(posts, 0, ROTT_POST_COUNT * sizeof(wallcast_t));

    animwalls = rg_alloc(MAXANIMWALLS * sizeof(animwall_t), MEM_SLOW);
    memset(animwalls, 0, MAXANIMWALLS * sizeof(animwall_t));

    switches = rg_alloc(MAXSWITCHES * sizeof(wall_t), MEM_SLOW);
    memset(switches, 0, MAXSWITCHES * sizeof(wall_t));

    ELEVATOR = rg_alloc(MAXELEVATORS * sizeof(elevator_t), MEM_SLOW);
    memset(ELEVATOR, 0, MAXELEVATORS * sizeof(elevator_t));

    deathshapeoffset = rg_alloc(8, MEM_SLOW);
    byte ds_init[] = {0,7,7,8,8,9,8,7};
    memcpy(deathshapeoffset, ds_init, 8);

    characters = rg_alloc(5 * sizeof(ROTTCHARS), MEM_SLOW);
    ROTTCHARS chars_init[5] = {
       {0x2100,0x4800,100,2,25},  // Taradino Cassatt
       {0x2200,0x5200,85,3,32},   // Thi Barrett
       {0x1f00,0x4000,150,3,20},  // Doug Wendt
       {0x2300,0x5500,70,2,33},   // Lorelei Ni
       {0x2000,0x4400,120,3,25}   // Ian Paul Freeley
    };
    memcpy(characters, chars_init, 5 * sizeof(ROTTCHARS));

    TRIGGER = rg_alloc(MAXTOUCHPLATES, MEM_SLOW);
    memset(TRIGGER, 0, MAXTOUCHPLATES);

    MISCVARS = rg_alloc(sizeof(misc_stuff), MEM_SLOW);
    memset(MISCVARS, 0, sizeof(misc_stuff));

    MV_PanTable = rg_alloc(MV_NumPanPositions * 64 * sizeof(Pan), MEM_SLOW);
    memset(MV_PanTable, 0, MV_NumPanPositions * 64 * sizeof(Pan));

    maskobjlist = rg_alloc(MAXMASKED * sizeof(maskedwallobj_t *), MEM_SLOW);
    memset(maskobjlist, 0, MAXMASKED * sizeof(maskedwallobj_t *));

    doorobjlist = rg_alloc(MAXDOORS * sizeof(doorobj_t *), MEM_SLOW);
    memset(doorobjlist, 0, MAXDOORS * sizeof(doorobj_t *));

    pwallobjlist = rg_alloc(MAXPWALLS * sizeof(pwallobj_t *), MEM_SLOW);
    memset(pwallobjlist, 0, MAXPWALLS * sizeof(pwallobj_t *));

    areabyplayer = rg_alloc(NUMAREAS * sizeof(rott_boolean), MEM_SLOW);
    memset(areabyplayer, 0, NUMAREAS * sizeof(rott_boolean));

    angletodir = rg_alloc(ANGLES * sizeof(int), MEM_SLOW);
    memset(angletodir, 0, ANGLES * sizeof(int));

    firstareaactor = rg_alloc((NUMAREAS + 1) * sizeof(objtype *), MEM_SLOW);
    memset(firstareaactor, 0, (NUMAREAS + 1) * sizeof(objtype *));

    lastareaactor = rg_alloc((NUMAREAS + 1) * sizeof(objtype *), MEM_SLOW);
    memset(lastareaactor, 0, (NUMAREAS + 1) * sizeof(objtype *));

    touchindices = rg_alloc(MAPSIZE * MAPSIZE * sizeof(word), MEM_SLOW);
    memset(touchindices, 0, MAPSIZE * MAPSIZE * sizeof(word));

    touchplate = rg_alloc(MAXTOUCHPLATES * sizeof(touchplatetype *), MEM_SLOW);
    memset(touchplate, 0, MAXTOUCHPLATES * sizeof(touchplatetype *));

    SNAKEPATH = rg_alloc(512 * sizeof(_2Dpoint), MEM_SLOW);
    memset(SNAKEPATH, 0, 512 * sizeof(_2Dpoint));

    numactions = rg_alloc(MAXTOUCHPLATES, MEM_SLOW);
    memset(numactions, 0, MAXTOUCHPLATES);

    lastaction = rg_alloc(MAXTOUCHPLATES * sizeof(touchplatetype *), MEM_SLOW);
    memset(lastaction, 0, MAXTOUCHPLATES * sizeof(touchplatetype *));

    areaconnect = rg_alloc(NUMAREAS * NUMAREAS, MEM_SLOW);
    memset(areaconnect, 0, NUMAREAS * NUMAREAS);

    printf("Memory allocation complete.\n");
}

void Z_Realloc(void **ptr, int size) {
    if (*ptr == NULL) {
        *ptr = Z_LevelMalloc(size, PU_STATIC, NULL);
        return;
    }

    memblock_t *curr = first_block;
    while (curr) {
        if (curr->ptr == *ptr) {
            int alloc_size = (size > 0) ? size : 1;
            size_t total_size = ZONE_HEADER_SIZE + (size_t)alloc_size;
            zone_header_t *header =
                (zone_header_t *)((byte *)curr->ptr - ZONE_HEADER_SIZE);
            if (header->magic != ZONE_HEADER_MAGIC || header->block != curr)
                Error("Z_Realloc: invalid zone allocation header");

            zone_header_t *new_header = heap_caps_realloc(
                header, total_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

            if (new_header) {
                void *newptr = (byte *)new_header + ZONE_HEADER_SIZE;
                new_header->block = curr;
                new_header->magic = ZONE_HEADER_MAGIC;
                curr->ptr = newptr;
                curr->size = alloc_size;
                if (curr->user)
                    *curr->user = newptr;
                *ptr = newptr;
            }
            return;
        }
        curr = curr->next;
    }

    // If not found in our tracking, just try to realloc normally
    void *tmp = heap_caps_realloc(*ptr, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (tmp) {
        *ptr = tmp;
    }
}

void *Z_Malloc (int size, int tag, void *user)
{
    static int alloc_count = 0;
    if ((++alloc_count & 31) == 0) vTaskDelay(1);

    int alloc_size = (size > 0) ? size : 1;
    size_t total_size = ZONE_HEADER_SIZE + (size_t)alloc_size;

    zone_header_t *header =
        heap_caps_calloc(1, total_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!header) {
        Z_FreeTags(100, 255);
        header =
            heap_caps_calloc(1, total_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    }
    if (!header) {
        size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
        printf("Z_Malloc FAILED: size=%d tag=%d (PSRAM free: %d)\n", 
               alloc_size, tag, (int)free_psram);
        Error("Z_Malloc failed to allocate %d bytes", alloc_size);
    }

    memblock_t *block = NULL;
    if (free_blocks) {
        block = free_blocks;
        free_blocks = block->next;
    } else {
        block = malloc(sizeof(memblock_t));
    }

    if (!block) {
        free(header);
        Error("Z_Malloc: failed to allocate tracking block");
    }

    void *ptr = (byte *)header + ZONE_HEADER_SIZE;
    header->block = block;
    header->magic = ZONE_HEADER_MAGIC;
    block->ptr = ptr;
    block->size = alloc_size;
    block->tag = tag;
    block->user = (void **)user;
    block->next = first_block;
    first_block = block;

    if (user) *(void **)user = ptr;
    return ptr;
}

static void Z_Free_Internal(memblock_t *block)
{
    zone_header_t *header =
        (zone_header_t *)((byte *)block->ptr - ZONE_HEADER_SIZE);

    if (header->magic != ZONE_HEADER_MAGIC || header->block != block)
        Error("Z_Free: invalid zone allocation header");

    if (block->user) *(block->user) = NULL;
    header->magic = 0;
    header->block = NULL;
    free(header);
    
    if (block >= block_pool && block < block_pool + MAX_MEM_BLOCKS) {
        block->next = free_blocks;
        free_blocks = block;
    } else {
        free(block);
    }
}

void Z_Free (void *ptr)
{
    if (!ptr) return;

    memblock_t *prev = NULL;
    memblock_t *curr = first_block;
    while (curr) {
        if (curr->ptr == ptr) {
            if (prev) prev->next = curr->next;
            else first_block = curr->next;
            Z_Free_Internal(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

void Z_FreeTags (int lowtag, int hightag)
{
    memblock_t *prev = NULL;
    memblock_t *curr = first_block;
    int count = 0;
    while (curr) {
        if (curr->tag >= lowtag && curr->tag <= hightag) {
            memblock_t *next = curr->next;
            if (prev) prev->next = next;
            else first_block = next;
            Z_Free_Internal(curr);
            curr = next;
        } else {
            prev = curr;
            curr = curr->next;
        }
        if ((++count & 63) == 0) vTaskDelay(1);
    }
}

void Z_FreeAll (void)
{
    memblock_t *curr = first_block;
    int count = 0;
    while (curr) {
        memblock_t *next = curr->next;
        Z_Free_Internal(curr);
        curr = next;
        if ((++count & 63) == 0) vTaskDelay(1);
    }
    first_block = NULL;
}

void Z_ShutDown (void) {
    Z_FreeAll();
}

int  Z_HeapSize(void) { return 4 * 1024 * 1024; }
int  Z_UsedHeap(void) {
    int total = 0;
    memblock_t *curr = first_block;
    while (curr) {
        total += curr->size;
        curr = curr->next;
    }
    return total;
}
int  Z_UsedLevelHeap(void) { return 512 * 1024; }
int  lowmemory = 0;
int  zonememorystarted = 1;
void DivisionErrorClear (void) {}

void *Z_TagMalloc (int size, int tag)
{
    return Z_Malloc(size, tag, NULL);
}

void Z_CheckCard (void) {}

void *Z_LevelMalloc (int size, int tag, void *user)
{
    return Z_Malloc(size, tag, user);
}

void Z_ChangeTag (void *ptr, int tag)
{
    if (!ptr)
        return;

    zone_header_t *header =
        (zone_header_t *)((byte *)ptr - ZONE_HEADER_SIZE);
    memblock_t *block = header->block;

    if (header->magic != ZONE_HEADER_MAGIC || !block || block->ptr != ptr)
        Error("Z_ChangeTag: pointer is not a zone allocation");

    block->tag = tag;
}

void Z_Init (int size, int min)
{
}
