#ifndef __SAVESTATE_H
#define __SAVESTATE_H

#include <nds.h>

#include "Console.hxx"

extern void dsSaveStateHandler(void);
extern void SaveState(const char *filename);
extern void LoadState(const char *filename);

#endif
