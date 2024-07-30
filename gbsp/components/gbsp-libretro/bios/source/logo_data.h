
//{{BLOCK(logo_data)

//======================================================================
//
//	logo_data, 256x256@4, 
//	+ palette 16 entries, not compressed
//	+ 120 tiles (t|f|p reduced) lz77 compressed
//	+ regular map (in SBBs), lz77 compressed, 32x32 
//	Total size: 32 + 1720 + 496 = 2248
//
//	Time-stamp: 2013-08-26, 19:29:30
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.6
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_LOGO_DATA_H
#define GRIT_LOGO_DATA_H

#define logo_dataTilesLen 1720
extern const unsigned short logo_dataTiles[860];

#define logo_dataMapLen 496
extern const unsigned short logo_dataMap[248];

#define logo_dataPalLen 32
extern const unsigned short logo_dataPal[16];

#endif // GRIT_LOGO_DATA_H

//}}BLOCK(logo_data)
