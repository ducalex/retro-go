#ifndef _INCLUDE_SOUND_H
#define _INCLUDE_SOUND_H

int  snd_init(void);
void snd_term(void);
void psg_update(signed char *buf, int ch, unsigned dwSize);

#endif
