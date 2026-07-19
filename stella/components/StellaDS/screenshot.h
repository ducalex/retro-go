// Borrowed from GodMode9i from Rocket Robz
#ifndef SCREENSHOT_H
#define SCREENSHOT_H
#include <nds/ndstypes.h>

extern bool screenshot(void);

typedef struct {
    u16 type;                       /* Magic identifier            */
    u32 size;                       /* File size in bytes          */
    u16 reserved1, reserved2;
    u32 offset;                     /* Offset to image data, bytes */
} PACKED HEADER;

typedef struct {
    u32 size;                       /* Header size in bytes      */
    u32 width, height;              /* Width and height of image */
    u16 planes;                     /* Number of colour planes   */
    u16 bits;                       /* Bits per pixel            */
    u32 compression;                /* Compression type          */
    u32 imagesize;                  /* Image size in bytes       */
    u32 xresolution, yresolution;   /* Pixels per meter          */
    u32 ncolours;                   /* Number of colours         */
    u32 importantcolours;           /* Important colours         */
    u32 redBitmask;                 /* Red bitmask               */
    u32 greenBitmask;               /* Green bitmask             */
    u32 blueBitmask;                /* Blue bitmask              */
    u32 reserved;
} PACKED INFOHEADER;

#endif // SCREENSHOT_H
