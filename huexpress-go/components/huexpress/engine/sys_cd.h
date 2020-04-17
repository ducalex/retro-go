#ifndef _INCLUDE_SYS_CD_H
#define _INCLUDE_SYS_CD_H

#ifndef LINUX
#define CDROM_AUDIO_PLAY 0x11
#define CDROM_AUDIO_PAUSED 0x12
#endif

/*
 * CD acces section
 * 
 * No need to implement them if you don't have a CD machine, don't want
 * or can't add PC Engine CD support
 */

  /*
   * osd_cd_init
   * 
   * Try to initialize the cd drive
   * The char* in input is the iso_filename, i.e. the name specified by the
   * -i command line parameter or "" if not specified (use default for your
   * system then)
   * return 0 on success else non zero value on error
   */
int osd_cd_init(char *);

  /*
   * osd_cd_close
   * 
   * Releases the resources allocated by osd_cd_init
   */
void osd_cd_close(void);


  /*
   * osd_cd_read
   * 
   * Read a 2048 cooked sector from the cd
   * Returns the read data in the uchar* variable
   * Data are located at sector in the uint32 variable
   * return 0 on success else non zero value on error
   */
void osd_cd_read(uchar *, uint32);

  /*
   * osd_cd_stop_audio
   * 
   * Stops the audio playing from cd is any
   */
void osd_cd_stop_audio(void);

  /*
   * osd_cd_track_info
   * 
   * Return the msf location of the beginning of the given track
   * plus the control byte of it
   */
void osd_cd_track_info(uchar track,
					   int *min, int *sec, int *fra, int *control);

  /*
   * osd_cd_nb_tracks
   * 
   * Return the index of the first and last track of the cd
   */
void osd_cd_nb_tracks(int *first, int *last);

  /*
   * osd_cd_length
   *
   * Return the total length of the cd
   */
void osd_cd_length(int *min, int *sec, int *fra);


  /*
   * osd_cd_play_audio_track
   * 
   * Plays the selected audio track
   */
void osd_cd_play_audio_track(uchar track);

  /*
   * osd_cd_play_audio_range
   * 
   * Plays audio range specified in msf mode
   */
void osd_cd_play_audio_range(uchar min_from,
							 uchar sec_from,
							 uchar fra_from,
							 uchar min_to, uchar sec_to, uchar fra_to);
	/*
	 * osd_cd_pause
	 *
	 * Pauses the cdda playing
	 */
void osd_cd_pause();

	/*
	 * osd_cd_status
	 *
	 * Get status about the current cdda playing
	 */
void osd_cd_status(int *status);

	/*
	 * osd_cd_subchannel_info
	 *
	 * Fills the pce buffer given as param with info about the subchannel
	 */
void osd_cd_subchannel_info(uint16 offset);

	/*
	 * osd_cd_resume
	 *
	 * Resumes paused cdda playback
	 */
void osd_cd_resume(void);
#endif
