/*****************************************************
 * HuExpress, the PC Engine emulator
 *
 * Copyright 2011-2013 Alexander von Gluck IV
 * Released under the terms of the GPLv2 license.
 *
 * Original HuGO! Authors:
 *  Copyright 2001-2005 Zeograd
 *  Copyright 1998 BERO
 *  Copyright 1996 Alex Krasivsky, Marat Fayzullin
 */
#ifndef _PCENGINE_H
#define _PCENGINE_H

extern "C" {
#include "debug.h"
#include "pce.h"
#include "iniconfig.h"
}


class PCEngine {

public:
						PCEngine();
						~PCEngine();

	int					LoadFile(char* file);
	void				Run();

	int					isReady() { return fReady; };

private:

	void				InitPaths();

	int					fReady;

	struct host_machine	fHost;
	struct hugo_options*	fOptions;
};


#endif
