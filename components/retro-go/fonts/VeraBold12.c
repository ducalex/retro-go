/*******************************************************************************
 * Size: 12 px
 * Bpp: 1
 * Opts: --bpp 1 --size 12 --no-compress --font Bitstream Vera Sans Bold.ttf --range 32-254 --format lvgl -o verabold12.c
 ******************************************************************************/

#include "../rg_gui.h"

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
uint8_t verabold12_glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xcf,

    /* U+0022 "\"" */
    0xb6, 0x80,

    /* U+0023 "#" */
    0x14, 0x51, 0xf9, 0x42, 0x9f, 0x8a, 0x28,

    /* U+0024 "$" */
    0x11, 0xe5, 0x1c, 0x7c, 0x71, 0xde, 0x10,

    /* U+0025 "%" */
    0x62, 0x49, 0x25, 0xc, 0x80, 0xb8, 0xd4, 0x4a,
    0x67,

    /* U+0026 "&" */
    0x3c, 0x64, 0x60, 0x30, 0xfa, 0xca, 0xce, 0x7e,

    /* U+0027 "'" */
    0xe0,

    /* U+0028 "(" */
    0x6f, 0x6d, 0xb3, 0x60,

    /* U+0029 ")" */
    0xd9, 0xb6, 0xde, 0xc0,

    /* U+002A "*" */
    0x25, 0x5d, 0xf2, 0x0,

    /* U+002B "+" */
    0x10, 0x4f, 0xc4, 0x10, 0x40,

    /* U+002C "," */
    0xf8,

    /* U+002D "-" */
    0xe0,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x32, 0x22, 0x44, 0x4c, 0x80,

    /* U+0030 "0" */
    0x7b, 0x2c, 0xf3, 0xcf, 0x3c, 0xde,

    /* U+0031 "1" */
    0xf7, 0x8c, 0x63, 0x18, 0xdf,

    /* U+0032 "2" */
    0xf4, 0xc6, 0x33, 0x33, 0x1f,

    /* U+0033 "3" */
    0x78, 0x30, 0xce, 0xc, 0x38, 0xfe,

    /* U+0034 "4" */
    0x38, 0xe7, 0x96, 0x9b, 0xf1, 0x86,

    /* U+0035 "5" */
    0x79, 0x86, 0x1e, 0xc, 0x38, 0xfe,

    /* U+0036 "6" */
    0x39, 0xc, 0x3e, 0xcf, 0x3c, 0xde,

    /* U+0037 "7" */
    0xf8, 0xc4, 0x63, 0x11, 0x8c,

    /* U+0038 "8" */
    0x7b, 0x3c, 0xdc, 0xcf, 0x3c, 0xde,

    /* U+0039 "9" */
    0x7b, 0x2c, 0xf3, 0x7c, 0x31, 0x9c,

    /* U+003A ":" */
    0xf0, 0xf0,

    /* U+003B ";" */
    0x6c, 0x6, 0xd0,

    /* U+003C "<" */
    0xe, 0xf3, 0x3, 0x81, 0xe0, 0x40,

    /* U+003D "=" */
    0xfc, 0x0, 0x3f,

    /* U+003E ">" */
    0xc0, 0x70, 0x39, 0xee, 0x10, 0x0,

    /* U+003F "?" */
    0xf0, 0xc6, 0x66, 0x1, 0x8c,

    /* U+0040 "@" */
    0x3e, 0x31, 0xb7, 0x34, 0x9a, 0x4d, 0x2f, 0x7c,
    0xc4, 0x3c, 0x0,

    /* U+0041 "A" */
    0x18, 0x38, 0x3c, 0x2c, 0x66, 0x7e, 0x46, 0xc3,

    /* U+0042 "B" */
    0xfb, 0x3c, 0xfe, 0xcf, 0x3c, 0xfe,

    /* U+0043 "C" */
    0x3d, 0x8c, 0x30, 0xc3, 0x6, 0xf,

    /* U+0044 "D" */
    0xf9, 0x9b, 0x1e, 0x3c, 0x78, 0xf3, 0x7c,

    /* U+0045 "E" */
    0xfe, 0x31, 0xfc, 0x63, 0x1f,

    /* U+0046 "F" */
    0xfe, 0x31, 0xfc, 0x63, 0x18,

    /* U+0047 "G" */
    0x3e, 0xc7, 0x6, 0xc, 0xf8, 0xd9, 0x9f,

    /* U+0048 "H" */
    0xc7, 0x8f, 0x1f, 0xfc, 0x78, 0xf1, 0xe3,

    /* U+0049 "I" */
    0xff, 0xff,

    /* U+004A "J" */
    0x33, 0x33, 0x33, 0x33, 0x3e,

    /* U+004B "K" */
    0xcd, 0xbb, 0xe7, 0x8f, 0x1b, 0x33, 0x67,

    /* U+004C "L" */
    0xc6, 0x31, 0x8c, 0x63, 0x1f,

    /* U+004D "M" */
    0xe7, 0xe7, 0xe7, 0xff, 0xdb, 0xdb, 0xc3, 0xc3,

    /* U+004E "N" */
    0xe7, 0xcf, 0xdf, 0xbd, 0xfb, 0xf3, 0xe7,

    /* U+004F "O" */
    0x3c, 0x66, 0xc3, 0xc3, 0xc3, 0xc3, 0x66, 0x3c,

    /* U+0050 "P" */
    0xfb, 0x3c, 0xf3, 0xfb, 0xc, 0x30,

    /* U+0051 "Q" */
    0x3c, 0x66, 0xc3, 0xc3, 0xc3, 0xc3, 0x66, 0x3c,
    0x6, 0x2,

    /* U+0052 "R" */
    0xf9, 0x9b, 0x36, 0x6f, 0x99, 0xb3, 0x63,

    /* U+0053 "S" */
    0x7b, 0xe, 0x3e, 0x7c, 0x38, 0xfe,

    /* U+0054 "T" */
    0xfe, 0x60, 0xc1, 0x83, 0x6, 0xc, 0x18,

    /* U+0055 "U" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x3c, 0xde,

    /* U+0056 "V" */
    0xc3, 0x46, 0x66, 0x66, 0x2c, 0x3c, 0x3c, 0x18,

    /* U+0057 "W" */
    0xce, 0x69, 0xc9, 0xab, 0x35, 0x66, 0xac, 0xf7,
    0x8e, 0x61, 0x8c,

    /* U+0058 "X" */
    0x66, 0x6c, 0x3c, 0x38, 0x38, 0x3c, 0x66, 0xc6,

    /* U+0059 "Y" */
    0x66, 0x66, 0x3c, 0x3c, 0x18, 0x18, 0x18, 0x18,

    /* U+005A "Z" */
    0xfc, 0x71, 0x8c, 0x31, 0x8e, 0x3f,

    /* U+005B "[" */
    0xfb, 0x6d, 0xb6, 0xe0,

    /* U+005C "\\" */
    0x8c, 0x44, 0x42, 0x22, 0x30,

    /* U+005D "]" */
    0xed, 0xb6, 0xdb, 0xe0,

    /* U+005E "^" */
    0x31, 0xec, 0x40,

    /* U+005F "_" */
    0xf8,

    /* U+0060 "`" */
    0x44,

    /* U+0061 "a" */
    0x78, 0x27, 0xb3, 0xcd, 0xf0,

    /* U+0062 "b" */
    0xc3, 0xf, 0xb3, 0xcf, 0x3c, 0xfe,

    /* U+0063 "c" */
    0x7e, 0x31, 0x8c, 0x3c,

    /* U+0064 "d" */
    0xc, 0x37, 0xf3, 0xcf, 0x3c, 0xdf,

    /* U+0065 "e" */
    0x7b, 0x3f, 0xf0, 0xc5, 0xf0,

    /* U+0066 "f" */
    0x76, 0xf6, 0x66, 0x66,

    /* U+0067 "g" */
    0x7f, 0x3c, 0xf3, 0xcd, 0xf0, 0xde,

    /* U+0068 "h" */
    0xc3, 0xf, 0xb3, 0xcf, 0x3c, 0xf3,

    /* U+0069 "i" */
    0xcf, 0xff,

    /* U+006A "j" */
    0x61, 0xb6, 0xdb, 0x78,

    /* U+006B "k" */
    0xc3, 0xc, 0xf6, 0xf3, 0xcd, 0xb3,

    /* U+006C "l" */
    0xff, 0xff,

    /* U+006D "m" */
    0xf6, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb,

    /* U+006E "n" */
    0xfb, 0x3c, 0xf3, 0xcf, 0x30,

    /* U+006F "o" */
    0x7b, 0x3c, 0xf3, 0xcd, 0xe0,

    /* U+0070 "p" */
    0xfb, 0x3c, 0xf3, 0xcf, 0xec, 0x30,

    /* U+0071 "q" */
    0x7f, 0x3c, 0xf3, 0xcd, 0xf0, 0xc3,

    /* U+0072 "r" */
    0xfc, 0xcc, 0xcc,

    /* U+0073 "s" */
    0x7e, 0x7c, 0xf1, 0xf8,

    /* U+0074 "t" */
    0x66, 0xf6, 0x66, 0x67,

    /* U+0075 "u" */
    0xcf, 0x3c, 0xf3, 0xcd, 0xf0,

    /* U+0076 "v" */
    0xcd, 0x36, 0x9e, 0x38, 0xc0,

    /* U+0077 "w" */
    0xc9, 0xa6, 0xdd, 0x4e, 0xe7, 0x71, 0xb8,

    /* U+0078 "x" */
    0x4d, 0xe3, 0x8e, 0x7b, 0x30,

    /* U+0079 "y" */
    0xcd, 0x36, 0x8e, 0x38, 0xc3, 0x18,

    /* U+007A "z" */
    0xf8, 0xcc, 0xcc, 0x7c,

    /* U+007B "{" */
    0x39, 0x8c, 0x6e, 0x18, 0xc6, 0x30, 0xc0,

    /* U+007C "|" */
    0xff, 0xc0,

    /* U+007D "}" */
    0xe6, 0x66, 0x36, 0x66, 0x6c,

    /* U+007E "~" */
    0x66, 0x60,

    /* U+00A0 " " */
    0x0,

    /* U+00A1 "¡" */
    0xf3, 0xff,

    /* U+00A2 "¢" */
    0x21, 0x1f, 0xce, 0x73, 0x8f, 0x20,

    /* U+00A3 "£" */
    0x3b, 0x18, 0xcf, 0xb1, 0x9f,

    /* U+00A4 "¤" */
    0x1, 0xe4, 0x92, 0x78, 0x0,

    /* U+00A5 "¥" */
    0xcd, 0x27, 0xbf, 0x33, 0xf3, 0xc,

    /* U+00A6 "¦" */
    0xf7, 0x80,

    /* U+00A7 "§" */
    0x72, 0x19, 0xfd, 0xfc, 0xc2, 0x70,

    /* U+00A8 "¨" */
    0xa0,

    /* U+00A9 "©" */
    0x38, 0x8a, 0x6d, 0x1a, 0x33, 0x51, 0x1c,

    /* U+00AA "ª" */
    0xf7, 0x9f, 0xf,

    /* U+00AB "«" */
    0x23, 0x7d, 0xa6, 0x80,

    /* U+00AC "¬" */
    0xfc, 0x10, 0x40,

    /* U+00AD "­" */
    0xe0,

    /* U+00AE "®" */
    0x38, 0x8a, 0xed, 0x5b, 0x35, 0x51, 0x1c,

    /* U+00AF "¯" */
    0xe0,

    /* U+00B0 "°" */
    0x56, 0xe0,

    /* U+00B1 "±" */
    0x10, 0x4f, 0xc4, 0x10, 0xf, 0xc0,

    /* U+00B2 "²" */
    0xe2, 0x4f,

    /* U+00B3 "³" */
    0xfc, 0xf0,

    /* U+00B4 "´" */
    0x50,

    /* U+00B5 "µ" */
    0xcd, 0x9b, 0x36, 0x6c, 0xdf, 0xf0, 0x60,

    /* U+00B6 "¶" */
    0x7f, 0x7b, 0xd2, 0x94, 0xa5, 0x28,

    /* U+00B7 "·" */
    0xf0,

    /* U+00B8 "¸" */
    0x70,

    /* U+00B9 "¹" */
    0xc9, 0x70,

    /* U+00BA "º" */
    0xe9, 0x9e, 0xf,

    /* U+00BB "»" */
    0x25, 0x9e, 0xbb, 0x0,

    /* U+00BC "¼" */
    0xc2, 0x23, 0x11, 0x1d, 0x20, 0xb0, 0xa8, 0x5e,
    0x42,

    /* U+00BD "½" */
    0xc2, 0x10, 0x84, 0x43, 0xbe, 0x8, 0x84, 0x21,
    0x10, 0x8f,

    /* U+00BE "¾" */
    0xe2, 0x72, 0x9, 0x1d, 0x20, 0xb0, 0xa8, 0xde,
    0x42,

    /* U+00BF "¿" */
    0x31, 0x80, 0x66, 0x63, 0x2f,

    /* U+00C0 "À" */
    0x10, 0x0, 0x18, 0x38, 0x3c, 0x2c, 0x66, 0x7e,
    0x46, 0xc3,

    /* U+00C1 "Á" */
    0x8, 0x10, 0x18, 0x38, 0x3c, 0x2c, 0x66, 0x7e,
    0x46, 0xc3,

    /* U+00C2 "Â" */
    0x18, 0x24, 0x18, 0x38, 0x3c, 0x2c, 0x66, 0x7e,
    0x46, 0xc3,

    /* U+00C3 "Ã" */
    0x38, 0x28, 0x18, 0x38, 0x3c, 0x2c, 0x66, 0x7e,
    0x46, 0xc3,

    /* U+00C4 "Ä" */
    0x28, 0x0, 0xe1, 0xc2, 0xcd, 0x9b, 0x3f, 0xc7,
    0x8c,

    /* U+00C5 "Å" */
    0x38, 0x50, 0xe1, 0xc6, 0xcd, 0x9b, 0x7f, 0xc7,
    0x8c,

    /* U+00C6 "Æ" */
    0x1f, 0xc5, 0x83, 0x60, 0xdf, 0x66, 0x1f, 0x84,
    0x63, 0x1f,

    /* U+00C7 "Ç" */
    0x3d, 0x8c, 0x30, 0xc3, 0x6, 0xf, 0x8, 0x60,

    /* U+00C8 "È" */
    0x61, 0x3f, 0x8c, 0x7f, 0x18, 0xc7, 0xc0,

    /* U+00C9 "É" */
    0x31, 0x3f, 0x8c, 0x7f, 0x18, 0xc7, 0xc0,

    /* U+00CA "Ê" */
    0x72, 0xbf, 0x8c, 0x7f, 0x18, 0xc7, 0xc0,

    /* U+00CB "Ë" */
    0x50, 0x3f, 0x8c, 0x7f, 0x18, 0xc7, 0xc0,

    /* U+00CC "Ì" */
    0x45, 0xb6, 0xdb, 0x6c,

    /* U+00CD "Í" */
    0x53, 0x6d, 0xb6, 0xd8,

    /* U+00CE "Î" */
    0x69, 0x66, 0x66, 0x66, 0x66,

    /* U+00CF "Ï" */
    0xa1, 0xb6, 0xdb, 0x6c,

    /* U+00D0 "Ð" */
    0x7c, 0x66, 0x63, 0x63, 0xf3, 0x63, 0x66, 0x7c,

    /* U+00D1 "Ñ" */
    0x28, 0xb3, 0x9f, 0x3f, 0x7e, 0xf7, 0xef, 0xcf,
    0x9c,

    /* U+00D2 "Ò" */
    0x10, 0x8, 0x3c, 0x66, 0xc3, 0xc3, 0xc3, 0xc3,
    0x66, 0x3c,

    /* U+00D3 "Ó" */
    0x8, 0x10, 0x3c, 0x66, 0xc3, 0xc3, 0xc3, 0xc3,
    0x66, 0x3c,

    /* U+00D4 "Ô" */
    0x18, 0x24, 0x3c, 0x66, 0xc3, 0xc3, 0xc3, 0xc3,
    0x66, 0x3c,

    /* U+00D5 "Õ" */
    0x3c, 0x0, 0x3c, 0x66, 0xc3, 0xc3, 0xc3, 0xc3,
    0x66, 0x3c,

    /* U+00D6 "Ö" */
    0x28, 0x0, 0x3c, 0x66, 0xc3, 0xc3, 0xc3, 0xc3,
    0x66, 0x3c,

    /* U+00D7 "×" */
    0x3, 0x37, 0x8c, 0x39, 0x30, 0x40,

    /* U+00D8 "Ø" */
    0x3f, 0x67, 0xcf, 0xcb, 0xd3, 0xf3, 0x66, 0xfc,

    /* U+00D9 "Ù" */
    0x20, 0xc, 0xf3, 0xcf, 0x3c, 0xf3, 0xcd, 0xe0,

    /* U+00DA "Ú" */
    0x10, 0x8c, 0xf3, 0xcf, 0x3c, 0xf3, 0xcd, 0xe0,

    /* U+00DB "Û" */
    0x31, 0xc, 0xf3, 0xcf, 0x3c, 0xf3, 0xcd, 0xe0,

    /* U+00DC "Ü" */
    0x30, 0xc, 0xf3, 0xcf, 0x3c, 0xf3, 0xcd, 0xe0,

    /* U+00DD "Ý" */
    0x8, 0x0, 0x66, 0x66, 0x3c, 0x3c, 0x18, 0x18,
    0x18, 0x18,

    /* U+00DE "Þ" */
    0xc3, 0xf, 0xb3, 0xcf, 0xec, 0x30,

    /* U+00DF "ß" */
    0x73, 0x2d, 0xbc, 0xdb, 0x3c, 0xf6,

    /* U+00E0 "à" */
    0x60, 0x81, 0x1e, 0x9, 0xec, 0xf3, 0x7c,

    /* U+00E1 "á" */
    0x8, 0x42, 0x1e, 0x9, 0xec, 0xf3, 0x7c,

    /* U+00E2 "â" */
    0x10, 0xc4, 0x9e, 0x9, 0xec, 0xf3, 0x7c,

    /* U+00E3 "ã" */
    0x30, 0x7, 0x82, 0x7b, 0x3d, 0xdb,

    /* U+00E4 "ä" */
    0x30, 0x7, 0x86, 0x7b, 0x7d, 0xdf,

    /* U+00E5 "å" */
    0x30, 0xc3, 0xc, 0x78, 0x27, 0xb3, 0xcd, 0xf0,

    /* U+00E6 "æ" */
    0x7f, 0x83, 0x27, 0xfb, 0x30, 0xcc, 0x5d, 0xf0,

    /* U+00E7 "ç" */
    0x7e, 0x31, 0x8c, 0x3c, 0x46,

    /* U+00E8 "è" */
    0x60, 0xc0, 0x1e, 0xcf, 0xfc, 0x31, 0x7c,

    /* U+00E9 "é" */
    0xc, 0x40, 0x1e, 0xcf, 0xfc, 0x31, 0x7c,

    /* U+00EA "ê" */
    0x10, 0xa0, 0x1e, 0xcf, 0xfc, 0x31, 0x7c,

    /* U+00EB "ë" */
    0x28, 0x7, 0xb3, 0xff, 0xe, 0x5f,

    /* U+00EC "ì" */
    0x88, 0xb6, 0xdb, 0x60,

    /* U+00ED "í" */
    0x2a, 0x6d, 0xb6, 0xc0,

    /* U+00EE "î" */
    0x66, 0x96, 0x66, 0x66, 0x60,

    /* U+00EF "ï" */
    0xa3, 0x6d, 0xb6,

    /* U+00F0 "ð" */
    0x39, 0x47, 0xb3, 0xcf, 0x3c, 0xde,

    /* U+00F1 "ñ" */
    0x78, 0xf, 0xb3, 0xcf, 0x3c, 0xf3,

    /* U+00F2 "ò" */
    0x60, 0x80, 0x1e, 0xcf, 0x3c, 0xf3, 0x78,

    /* U+00F3 "ó" */
    0x8, 0x40, 0x1e, 0xcf, 0x3c, 0xf3, 0x78,

    /* U+00F4 "ô" */
    0x10, 0xc0, 0x1e, 0xcf, 0x3c, 0xf3, 0x78,

    /* U+00F5 "õ" */
    0x38, 0x7, 0xb3, 0xcf, 0x3c, 0xde,

    /* U+00F6 "ö" */
    0x28, 0x7, 0xb3, 0xcf, 0x3c, 0xde,

    /* U+00F7 "÷" */
    0x30, 0x0, 0x3f, 0x0, 0xc0,

    /* U+00F8 "ø" */
    0x7f, 0x3d, 0xfb, 0xcf, 0xe0,

    /* U+00F9 "ù" */
    0x40, 0x81, 0x33, 0xcf, 0x3c, 0xf3, 0x7c,

    /* U+00FA "ú" */
    0x8, 0x42, 0x33, 0xcf, 0x3c, 0xf3, 0x7c,

    /* U+00FB "û" */
    0x10, 0xc4, 0xb3, 0xcf, 0x3c, 0xf3, 0x7c,

    /* U+00FC "ü" */
    0x50, 0xc, 0xf3, 0xcf, 0x3c, 0xdf,

    /* U+00FD "ý" */
    0x8, 0x40, 0x33, 0x4d, 0xa3, 0x8e, 0x30, 0xc6,
    0x0,

    /* U+00FE "þ" */
    0xc3, 0xf, 0xb3, 0xcf, 0x3c, 0xfe, 0xc3, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const rg_font_glyph_dsc_t verabold12_glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 56, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 73, .box_w = 2, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 83, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 5, .adv_w = 134, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 12, .adv_w = 111, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 19, .adv_w = 160, .box_w = 9, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 28, .adv_w = 140, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 36, .adv_w = 49, .box_w = 1, .box_h = 3, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 37, .adv_w = 73, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 41, .adv_w = 73, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 45, .adv_w = 84, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 49, .adv_w = 134, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 54, .adv_w = 61, .box_w = 2, .box_h = 3, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 55, .adv_w = 66, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 56, .adv_w = 61, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 57, .adv_w = 58, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 62, .adv_w = 111, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 68, .adv_w = 111, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 73, .adv_w = 111, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 78, .adv_w = 111, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 84, .adv_w = 111, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 90, .adv_w = 111, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 96, .adv_w = 111, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 102, .adv_w = 111, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 107, .adv_w = 111, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 113, .adv_w = 111, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 119, .adv_w = 64, .box_w = 2, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 121, .adv_w = 64, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 124, .adv_w = 134, .box_w = 7, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 130, .adv_w = 134, .box_w = 6, .box_h = 4, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 133, .adv_w = 134, .box_w = 7, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 139, .adv_w = 93, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 144, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 155, .adv_w = 124, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 163, .adv_w = 122, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 169, .adv_w = 117, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 175, .adv_w = 133, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 182, .adv_w = 109, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 187, .adv_w = 109, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 192, .adv_w = 131, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 199, .adv_w = 134, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 206, .adv_w = 60, .box_w = 2, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 208, .adv_w = 60, .box_w = 4, .box_h = 10, .ofs_x = -1, .ofs_y = -2},
    {.bitmap_index = 213, .adv_w = 124, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 220, .adv_w = 102, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 225, .adv_w = 159, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 233, .adv_w = 134, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 240, .adv_w = 136, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 248, .adv_w = 117, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 254, .adv_w = 136, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 264, .adv_w = 123, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 271, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 277, .adv_w = 109, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 284, .adv_w = 130, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 290, .adv_w = 124, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 298, .adv_w = 176, .box_w = 11, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 309, .adv_w = 123, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 317, .adv_w = 116, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 325, .adv_w = 116, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 331, .adv_w = 73, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 335, .adv_w = 58, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 340, .adv_w = 73, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 344, .adv_w = 134, .box_w = 6, .box_h = 3, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 347, .adv_w = 80, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 348, .adv_w = 80, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 349, .adv_w = 108, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 354, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 360, .adv_w = 95, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 364, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 370, .adv_w = 109, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 375, .adv_w = 70, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 379, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 385, .adv_w = 114, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 391, .adv_w = 55, .box_w = 2, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 393, .adv_w = 55, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 397, .adv_w = 106, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 403, .adv_w = 55, .box_w = 2, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 405, .adv_w = 167, .box_w = 8, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 411, .adv_w = 114, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 416, .adv_w = 110, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 421, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 427, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 433, .adv_w = 79, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 436, .adv_w = 95, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 440, .adv_w = 76, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 444, .adv_w = 114, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 449, .adv_w = 104, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 454, .adv_w = 148, .box_w = 9, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 461, .adv_w = 103, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 466, .adv_w = 104, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 472, .adv_w = 93, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 476, .adv_w = 114, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 483, .adv_w = 58, .box_w = 1, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 485, .adv_w = 114, .box_w = 4, .box_h = 10, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 490, .adv_w = 134, .box_w = 6, .box_h = 2, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 492, .adv_w = 111, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 493, .adv_w = 73, .box_w = 2, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 495, .adv_w = 111, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 501, .adv_w = 111, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 506, .adv_w = 102, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 511, .adv_w = 111, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 517, .adv_w = 58, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 519, .adv_w = 80, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 525, .adv_w = 80, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 526, .adv_w = 160, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 533, .adv_w = 90, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 536, .adv_w = 103, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 540, .adv_w = 134, .box_w = 6, .box_h = 3, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 543, .adv_w = 66, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 544, .adv_w = 160, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 551, .adv_w = 80, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 552, .adv_w = 80, .box_w = 3, .box_h = 4, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 554, .adv_w = 134, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 560, .adv_w = 70, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 562, .adv_w = 70, .box_w = 3, .box_h = 4, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 564, .adv_w = 80, .box_w = 3, .box_h = 2, .ofs_x = 2, .ofs_y = 7},
    {.bitmap_index = 565, .adv_w = 118, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 572, .adv_w = 102, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 578, .adv_w = 61, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 579, .adv_w = 80, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 580, .adv_w = 70, .box_w = 3, .box_h = 4, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 582, .adv_w = 90, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 585, .adv_w = 103, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 589, .adv_w = 166, .box_w = 9, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 598, .adv_w = 166, .box_w = 10, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 608, .adv_w = 166, .box_w = 9, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 617, .adv_w = 93, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 622, .adv_w = 124, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 632, .adv_w = 124, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 642, .adv_w = 124, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 652, .adv_w = 124, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 662, .adv_w = 124, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 671, .adv_w = 124, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 680, .adv_w = 174, .box_w = 10, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 690, .adv_w = 117, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 698, .adv_w = 109, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 705, .adv_w = 109, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 712, .adv_w = 109, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 719, .adv_w = 109, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 726, .adv_w = 60, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 730, .adv_w = 60, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 734, .adv_w = 60, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 739, .adv_w = 60, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 743, .adv_w = 134, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 751, .adv_w = 134, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 760, .adv_w = 136, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 770, .adv_w = 136, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 780, .adv_w = 136, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 790, .adv_w = 136, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 800, .adv_w = 136, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 810, .adv_w = 134, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 816, .adv_w = 136, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 824, .adv_w = 130, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 832, .adv_w = 130, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 840, .adv_w = 130, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 848, .adv_w = 130, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 856, .adv_w = 116, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 866, .adv_w = 118, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 872, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 878, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 885, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 892, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 899, .adv_w = 108, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 905, .adv_w = 108, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 911, .adv_w = 108, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 919, .adv_w = 168, .box_w = 10, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 927, .adv_w = 95, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 932, .adv_w = 109, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 939, .adv_w = 109, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 946, .adv_w = 109, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 953, .adv_w = 109, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 959, .adv_w = 55, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 963, .adv_w = 55, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 967, .adv_w = 55, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 972, .adv_w = 55, .box_w = 3, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 975, .adv_w = 110, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 981, .adv_w = 114, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 987, .adv_w = 110, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 994, .adv_w = 110, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1001, .adv_w = 110, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1008, .adv_w = 110, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1014, .adv_w = 110, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1020, .adv_w = 134, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 1025, .adv_w = 110, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1030, .adv_w = 114, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1037, .adv_w = 114, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1044, .adv_w = 114, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1051, .adv_w = 114, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1057, .adv_w = 104, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1066, .adv_w = 115, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = -2}
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

const rg_font_t font_verabold12 = {
    .bitmap_data = verabold12_glyph_bitmap,
    .glyph_dsc = verabold12_glyph_dsc,
    .name = "Verabold",
    .height = 14,
};