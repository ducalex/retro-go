#ifndef _HCD_H
#define _HCD_H


#include "cleantypes.h"
#include "pce.h"
//#include "lsmp3.h"
//#include "ogglength.h"
//#include "osd_sdl_music.h"

extern uint32 HCD_first_track;
/* first track of emulated cd */

extern uint32 HCD_last_track;
/* last track of emulated cd */

extern uint32 HCD_frame_at_beginning_of_track;
/* nb of frame when this track started,
 * used to make subtitle
 */

extern FILE *HCD_iso_FILE;


int fill_HCD_info(char *name);
void HCD_play_sectors(int begin_sect, int sect_len, char repeat);
void HCD_pause_playing();
void HCD_play_track(uchar track, char repeat);
void HCD_shutdown();
void HCD_handle_subtitle();
void HCD_iso_read_sector(uchar * p, uint32 dum, uint32 result);
void HCD_cd_read_sector(uchar * p, uint32 dum, uint32 result);


#endif /* _HCD_H */
