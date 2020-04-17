#ifndef _INCLUDE_SYS_SND_H
#define _INCLUDE_SYS_SND_H

/*
 * Snd section
 * 
 * This section gathers all sound replaying related function on the emulating machine.
 * We have to handle raw waveform (generated from hu-go! and internal pce state) and
 * if possible external format to add support for them in .hcd.
 */

/*
   * osd_snd_set_volume
   *
   * Changes the current global volume
   */
void osd_snd_set_volume(uchar);

	/*
	 * osd_snd_init_sound
	 *
	 * Allocates ressources to output sound
	 * returns 0 on error else non zero value
	 */
int osd_snd_init_sound();

	/*
	 * osd_snd_trash_sound
	 *
	 * Frees all sound ressources allocated in osd_snd_init_sound
	 */
void osd_snd_trash_sound();
#endif
