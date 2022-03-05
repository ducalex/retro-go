#pragma once

#include <stdbool.h>

int gfx_init(void);
void gfx_run(void);
void gfx_term(void);
void gfx_irq(int type);
void gfx_reset(bool hard);
void gfx_latch_context(int force);
