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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

typedef unsigned char   bool;   // 1204 ham

#include "notify_rc.h"

static void notify_rc_internal(const char *event_name, bool do_wait);


extern void notify_rc(const char *event_name)
{
	notify_rc_internal(event_name, 0);
}

extern void notify_rc_and_wait(const char *event_name)
{
	notify_rc_internal(event_name, 1);
}

static void notify_rc_internal(const char *event_name, bool do_wait)
{
	char *full_name;
	FILE *fp;
	int close_result;

	full_name = (char *)(malloc(strlen(event_name) + 100));
	if (full_name == NULL)
	{
		fprintf(stderr,
				"Error: Failed trying to allocate %lu bytes of memory for the "
				"full path of an rc notification marker file, while trying to "
				"notify rc of a `%s' event.\n",
				(unsigned long)(strlen(event_name) + 100), event_name);
		return;
	}

	sprintf(full_name, "/tmp/rc_notification/%s", event_name);
	fp = fopen(full_name, "w");
	if (fp == NULL)
	{
		fprintf(stderr,
				"Error: Failed trying to open file %s while trying to notify "
				"rc of an event: %s.\n", full_name, strerror(errno));
		free(full_name);
		return;
	}

	close_result = fclose(fp);
	if (close_result != 0)
	{
		fprintf(stderr,
				"Error: Failed trying to close file %s while trying to notify "
				"rc of an event: %s.\n", full_name, strerror(errno));
	}

	sprintf(full_name, "/tmp/rc_action_incomplete/%s", event_name);
	fp = fopen(full_name, "w");
	if (fp == NULL)
	{
		fprintf(stderr,
				"Error: Failed trying to open file %s while trying to notify "
				"rc of an event: %s.\n", full_name, strerror(errno));
		free(full_name);
		return;
	}

	close_result = fclose(fp);
	if (close_result != 0)
	{
		fprintf(stderr,
				"Error: Failed trying to close file %s while trying to notify "
				"rc of an event: %s.\n", full_name, strerror(errno));
	}

	sprintf(full_name, "/tmp/rc_notification/%s", event_name);
	fp = fopen(full_name, "w");
	if (fp == NULL)
	{
		fprintf(stderr,
				"Error: Failed trying to open file %s while trying to notify "
				"rc of an event: %s.\n", full_name, strerror(errno));
		free(full_name);
		return;
	}

	close_result = fclose(fp);
	if (close_result != 0)
	{
		fprintf(stderr,
				"Error: Failed trying to close file %s while trying to notify "
				"rc of an event: %s.\n", full_name, strerror(errno));
	}
	
	//wendebug
	int killproc = 1;
	//printf("notify_rc_internal : kill %d\n", killproc);
	kill(killproc, SIGUSR1);

	if (do_wait)
	{
		sprintf(full_name, "/tmp/rc_action_incomplete/%s", event_name);

		while (1)
		{
			fp = fopen(full_name, "r");
			if (fp == NULL)
				break;
			fclose(fp);
			sleep(1);
		}
	}

	free(full_name);
}
