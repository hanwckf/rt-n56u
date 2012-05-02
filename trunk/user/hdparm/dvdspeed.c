#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>
#include "hdparm.h"
/*
 * dvdspeed - use SET STREAMING command to set the speed of DVD-drives
 *
 * Copyright (c) 2004	Thomas Fritzsche <tf@noto.de>
 * A bit mangled in 2006 and 2008 by Thomas Orgis <thomas@orgis.org>
 *
 */

int set_dvdspeed(int fd, int speed)
{
	struct cdrom_generic_command cgc;
	struct request_sense sense;
	unsigned char buffer[28];
	unsigned long rwsize = 177 * speed;

	memset(&cgc, 0, sizeof(cgc));
	memset(&sense, 0, sizeof(sense));
	memset(&buffer, 0, sizeof(buffer));

	cgc.cmd[0] = 0xb6;	// SET_STREAMING

	cgc.cmd[10] = 28;	// parameter list length (28 bytes)
	cgc.sense   = &sense;
	cgc.buffer  = buffer;
	cgc.buflen  = sizeof(buffer);
	cgc.data_direction = CGC_DATA_WRITE;

	if (speed == 0)		// reset to default speed?
		buffer[0] = 4;

	buffer[ 8] = 0xff;
	buffer[ 9] = 0xff;
	buffer[10] = 0xff;
	buffer[11] = 0xff;

	// read size:
	buffer[12] = rwsize >> 24;
	buffer[13] = rwsize >> 16;
	buffer[14] = rwsize >>  8;
	buffer[15] = rwsize;

	// read time = 1 second:
	buffer[18] = 0x03;
	buffer[19] = 0xE8;

	// write size:
	buffer[20] = rwsize >> 24;
	buffer[21] = rwsize >> 16;
	buffer[22] = rwsize >>  8;
	buffer[23] = rwsize;

	// write time = 1 second:
	buffer[26] = 0x03;
	buffer[27] = 0xE8;

	return ioctl(fd, CDROM_SEND_PACKET, &cgc);
}

