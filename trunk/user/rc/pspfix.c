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
 */
#include <sys/signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <nvram/bcmnvram.h>
#include <shutils.h>

#include "rc.h"

void psp_stopwps(int sig)
{
	if (nvram_match("wps_triggered", "0"))
	{
//		if (nvram_match("wps_band", "0"))
			stop_wsc();		// psp fix
//		else
			stop_wsc_2g();
		nvram_set("wps_enable", "0");	// psp fix
	}

	exit(0);
}

int pspfix_main(int argc, char *argv[])
{
	FILE *fp;

	/* write pid */
	if ((fp=fopen("/var/run/pspfix.pid", "w"))!=NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	nvram_set("wps_triggered", "0");
	signal(SIGALRM, psp_stopwps);
	alarm(120);

	/* listen for replies */
	while (1)
	{
		sleep(240);
	}
	
	return 0;
}

