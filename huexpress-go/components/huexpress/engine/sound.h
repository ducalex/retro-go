#ifndef _INCLUDE_SOUND_H
#define _INCLUDE_SOUND_H

#include "pce.h"

int  snd_init();
void snd_term();
void psg_update(char *buf, int ch, unsigned dwSize);

#endif
