#ifndef _INCLUDE_HAIKU_OSD_CD_H
#define _INCLUDE_HAIKU_OSD_CD_H

//#include <device/scsi.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
//#include <sys/ioctl.h>

#include <errno.h>

#include "sys_dep.h"
#include "cd.h"

#include "debug.h"


typedef struct {
uint8   reserved;
	uint8   adr_control;    // bytes 0-3 are control, 4-7 are ADR
	uint8   track_number;
	uint8   reserved2;

	uint8   reserved3;
	uint8   min;
	uint8   sec;
	uint8   frame;
} TrackDescriptor;


#endif /* _INCLUDE_HAIKU_OSD_CD_H */
