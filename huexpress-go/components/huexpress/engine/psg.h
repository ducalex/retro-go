#ifndef _INCLUDE_PSG_H
#define _INCLUDE_PSG_H

#include <stdint.h>
#include <stddef.h>

int  psg_init(void);
void psg_term(void);
void psg_update(short *buf, int chan, size_t dwSize);
void psg_mix(short *buffer, size_t length);

#endif
