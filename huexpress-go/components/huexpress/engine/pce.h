#ifndef _INCLUDE_PCE_H
#define _INCLUDE_PCE_H

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include <odroid_system.h>

#include "../config.h"
#include "utils.h"
#include "osd.h"

#include "hard_pce.h"
#include "h6280.h"
#include "sound.h"
#include "gfx.h"

int LoadState(char *name);
int SaveState(char *name);
void ResetPCE(bool);
void RunPCE(void);
void ShutdownPCE();
int InitPCE(const char *name);
int LoadCard(const char *name);

struct host_machine {
	bool paused;
	bool netplay;

	struct {
		int frameSkip;
		int bgEnabled;
		int fgEnabled;
	} options;

	struct {
		uint stereo;
		uint freq;
		uint sample_size;
	} sound;

	struct {
		bool splatterhouse;
	} hacks;
};

extern struct host_machine host;

typedef struct
{
	uint len;
	char key[16];
	void *ptr;
} svar_t;

#define SVAR_1(k, v) { 1, k, &v }
#define SVAR_2(k, v) { 2, k, &v }
#define SVAR_4(k, v) { 4, k, &v }
#define SVAR_A(k, v) { sizeof(v), k, &v }
#define SVAR_END { 0, "\0\0\0\0", 0 }

#endif
