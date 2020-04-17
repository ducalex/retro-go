/*
 * Copyright 2013, Alexander von Gluck, All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *              Alexander von Gluck IV <kallisti5@unixzen.com>
 */
#ifndef ZIPMGR_H
#define ZIPMGR_H


#include "cleantypes.h"

#include <string.h>


#define ZIP_ERROR	0x0000
#define ZIP_HAS_NONE	0x0001
#define ZIP_HAS_PCE	0x0002
#define ZIP_HAS_ISO	0x0004
#define ZIP_HAS_HCD	0x0008

#define PATH_SLASH "/"

uint32 zipmgr_probe_file(char* zipFilename, char* foundGameFile);
uint32 zipmgr_extract_to_disk(char* zipFilename, char* destination);
char* zipmgr_extract_to_memory(char* zipFilename, char* cartFilename,
	size_t* cartSize);

#endif /* ZIPMGR_H */
