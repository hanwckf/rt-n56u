/*
 * Copyright (C) 2003-2004  Narcis Ilisei
 * Copyright (C) 2006  Steve Horbachuk
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#define MODULE_TAG ""
#include "debug_if.h"

#include "os.h"
#include "dyndns.h"

#ifdef UNIX_OS
#include <unistd.h>
#include <sys/stat.h>

#include "debug_if.h"

void os_sleep_ms(int ms)
{
    usleep(ms*1000);
}

int  os_get_socket_error (void)
{
    return errno;
}

RC_TYPE os_ip_support_startup(void)
{
    return RC_OK;
}

RC_TYPE os_ip_support_cleanup(void)
{
    return RC_OK;
}

RC_TYPE os_shell_execute(char * p_cmd)
{
	RC_TYPE rc = RC_OK;
	int child;

	child = vfork();
	switch (child)
	{
		case 0: /* child */
			execl("/bin/sh", "sh", "-c", p_cmd, (char *) 0);
			exit(1);
			break;

		case -1:
			rc = RC_OS_FORK_FAILURE;
			break;

		default: /* parent */
			break;
	}

	return rc;
}

/* storage for the parameter needed by the handler */
static void *global_p_signal_handler_param = NULL;

/**
 * unix_signal_handler - The actual handler
 * @signo: Signal number
 *
 * Handler for registered/known signals. Most others will terminate the daemon.
 *
 * NOTE:
 * Since printf() is one of the possible back-ends of logit(), and printf() is not one
 * of the safe syscalls to be used, according to POSIX signal(7). The calls are commented,
 * since they are most likely also only needed for debugging.
 */
static void unix_signal_handler(int signo)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *)global_p_signal_handler_param;

	if (p_self == NULL)
	{
//		logit(LOG_WARNING, MODULE_TAG "Signal %d received. But handler is not installed correctly.", signo);
		return;
	}

	switch (signo)
	{
		case SIGHUP:
//			logit(LOG_DEBUG, MODULE_TAG "Signal %d received. Sending restart command.", signo);
			p_self->cmd = CMD_RESTART;
			break;

		case SIGINT:
		case SIGQUIT:
		case SIGALRM:
		case SIGTERM:
//			logit(LOG_DEBUG, MODULE_TAG "Signal %d received. Sending shutdown command.", signo);
			p_self->cmd = CMD_STOP;
			break;

		default:
//			logit(LOG_DEBUG, MODULE_TAG "Signal %d received, ignoring.", signo);
			break;
	}
	return;
}

/**
	install handler for SIGALRM and HUP, INT, QUIT, TERM.
	avoid receiving HUP, INT, QUIT during ALRM and TERM.

*/
RC_TYPE os_install_signal_handler(void *p_dyndns)
{
	RC_TYPE rc;
	struct sigaction    newact;
	newact.sa_handler = unix_signal_handler;
	newact.sa_flags   = 0;

	rc = sigemptyset(&newact.sa_mask)            ||
		sigaddset(&newact.sa_mask, SIGHUP)   ||
		sigaddset(&newact.sa_mask, SIGINT)   ||
		sigaddset(&newact.sa_mask, SIGQUIT)  ||
		sigaddset(&newact.sa_mask, SIGTERM)  ||
		sigaction(SIGALRM, &newact, NULL)    ||
		sigemptyset(&newact.sa_mask)         ||
		sigaddset(&newact.sa_mask, SIGALRM)  ||
		sigaction(SIGHUP, &newact, NULL)     ||
		sigaction(SIGINT, &newact, NULL)     ||
		sigaction(SIGQUIT, &newact, NULL)    ||
		sigaction(SIGTERM, &newact, NULL);
 	if (rc == RC_OK)
 	{
		global_p_signal_handler_param = p_dyndns;
	}

	return rc;
}

/*
    closes current console

  July 5th, 2004 - Krev
  ***
  This function is used to close the console window to start the
  software as a service on Windows. On Unix, closing the console window
  isn't used for a daemon, but rather it forks. Modified this function
  to fork into a daemon mode under Unix-compatible systems.

  Actions:
    - for child:
	- close in and err console
	- become session leader
	- change working directory
	- clear the file mode creation mask
    - for parent
	just exit
*/
RC_TYPE close_console_window(void)
{
    pid_t pid = fork();

    if (pid < 0)
    {
	return RC_OS_FORK_FAILURE;
    }

    if (pid == 0)		/* child */
    {
	fclose(stdin);
	fclose(stderr);
	setsid();
	if (-1 == chdir("/"))
	{
		logit(LOG_WARNING, MODULE_TAG "Failed changing cwd to /: %s", strerror(errno));
	}
	umask(0);

	return RC_OK;
    }

    exit(0);

    return RC_OK;		/* Never reached. */
}

/* MAIN - Dyn DNS update entry point.*/
int main(int argc, char* argv[])
{
    return inadyn_main(argc, argv);
}

RC_TYPE os_syslog_open(const char *p_prg_name)
{
    openlog(p_prg_name, LOG_PID, LOG_USER);
    return RC_OK;
}

RC_TYPE os_syslog_close(void)
{
    closelog();
    return RC_OK;
}

RC_TYPE os_change_persona(OS_USER_INFO *p_usr_info)
{
	int rc;

	do
	{
		if (p_usr_info->gid != getgid())
		{
			if ((rc = setgid(p_usr_info->gid)) != 0)
			{
				break;
			}
		}

		if (p_usr_info->uid != getuid())
		{
			if ((rc = setuid(p_usr_info->uid)) != 0)
			{
				break;
			}
		}
	}
	while(0);

	if (rc != 0)
	{
		logit(LOG_WARNING, MODULE_TAG "Failed dropping privileges: %s", strerror(errno));
		return RC_OS_CHANGE_PERSONA_FAILURE;
	}

	return RC_OK;
}
#endif

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "ellemtel"
 *  c-basic-offset: 8
 * End:
 */
