#include "osd_cd.h"

int osd_cd_init(char *device)
{
}


void osd_cd_stop_audio()
{
}


void osd_cd_close()
{
}


void osd_cd_read(uchar *p, uint32 sector)
{
}


void osd_cd_subchannel_info(unsigned short offset)
{
}


void osd_cd_status(int *status)
{
}


void osd_cd_track_info(uchar track, int *min, int *sec, int *fra, int *control)
{
}


void osd_cd_nb_tracks(int *first, int *last)
{
}


void osd_cd_length(int *min, int *sec, int *fra)
{
}


void osd_cd_pause(void)
{
}


void osd_cd_resume(void)
{
}


/* TODO : check for last track asked */
void osd_cd_play_audio_track(uchar track)
{
}


void osd_cd_play_audio_range(uchar min_from, uchar sec_from, uchar fra_from,
                             uchar min_to, uchar sec_to, uchar fra_to)
{
}
