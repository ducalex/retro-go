#ifndef _INCLUDE_SOUND_H
#define _INCLUDE_SOUND_H

int  snd_init(void);
void snd_term(void);
void snd_update(short *buffer, unsigned length);

#endif
