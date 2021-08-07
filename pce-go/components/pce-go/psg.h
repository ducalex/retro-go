#pragma once

#include <stdint.h>
#include <stddef.h>

int  psg_init(void);
void psg_term(void);
void psg_update(int16_t *output, size_t length);
