#pragma once

#include <string.h>
#include <stddef.h>

#include "config.h"
#include "utils.h"
#include "osd.h"
#include "gfx.h"
#include "pce.h"
#include "psg.h"

#include <rg_system.h>

int LoadState(const char *name);
int SaveState(const char *name);
void ResetPCE(bool);
void RunPCE(void);
void ShutdownPCE();
int InitPCE(const char *name);
int LoadCard(const char *name);
void *PalettePCE(int colordepth);

typedef struct {
	bool paused;
	bool netplay;

	struct {
		int frameSkip;
		int bgEnabled;
		int fgEnabled;
	} options;

	struct {
		size_t sample_freq;
		bool sample_uint8;
		bool stereo;
	} sound;

	struct {
		bool splatterhouse;
	} hacks;
} host_machine_t;

extern host_machine_t host;
