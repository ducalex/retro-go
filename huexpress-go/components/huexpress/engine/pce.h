#ifndef _INCLUDE_PCE_H
#define _INCLUDE_PCE_H

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "../config.h"
#include "utils.h"
#include "osd.h"
#include "hard_pce.h"
#include "h6280.h"
#include "psg.h"
#include "gfx.h"

#include <rg_system.h>

int LoadState(const char *name);
int SaveState(const char *name);
void ResetPCE(bool);
void RunPCE(void);
void ShutdownPCE();
int InitPCE(const char *name);
int LoadCard(const char *name);

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

typedef struct
{
	size_t len;
	char key[16];
	void *ptr;
} svar_t;

#define SVAR_1(k, v) { 1, k, &v }
#define SVAR_2(k, v) { 2, k, &v }
#define SVAR_4(k, v) { 4, k, &v }
#define SVAR_A(k, v) { sizeof(v), k, &v }
#define SVAR_N(k, v, n) { n, k, &v }
#define SVAR_END { 0, "\0\0\0\0", 0 }

#endif
