/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */
 
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <nvram/bcmnvram.h>
#include <shutils.h>
#include <stdarg.h>

#include "rc.h"


static int server_idx = 0;
static char servers[32];

static void catch_sig(int sig)
{
	if (sig == SIGTSTP)
	{
		if (server_idx)
			strcpy(servers, nvram_safe_get("ntp_server1"));
		else
			strcpy(servers, nvram_safe_get("ntp_server0"));
		
		nvram_set("ntp_server_tried", servers);
		
		server_idx = (server_idx + 1) % 2;
	}
	else if (sig == SIGTERM)
	{
		remove("/var/run/ntp.pid");
		exit(0);
	}
}

int ntp_main(int argc, char **argv)
{
	FILE *fp;
	int ret;

	strcpy(servers, nvram_safe_get("ntp_server0"));
	nvram_set("ntp_server_tried", servers);

	if (!strlen(servers))
		return 0;
	
	fp = fopen("/var/run/ntp.pid","w");
	if (fp == NULL)
		exit(0);
	fprintf(fp, "%d", getpid());
	fclose(fp);

	dbg("starting ntp...\n");

	signal(SIGTSTP, catch_sig);

	while (1)
	{
		ret = doSystem("/usr/sbin/ntpclient -h %s -i 3 -l -s", servers);
		pause();
	}
	
	return 0;
}
