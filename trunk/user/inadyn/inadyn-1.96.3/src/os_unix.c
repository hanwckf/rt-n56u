/*
Copyright (C) 2003-2004 Narcis Ilisei
Modifications by Steve Horbachuk
Copyright (C) 2006 Steve Horbachuk

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#define MODULE_TAG "OS_UNIX:"
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
	int kid;
	switch((kid=vfork()))
	{
		case 0:
		/* child */
			execl("/bin/sh", "sh", "-c", p_cmd, (char *) 0);
			exit(1);
			
		break;
		case -1:
			rc = RC_OS_FORK_FAILURE;
		break;
		default:
		/* parent */
		break;
	}
	return rc;
}

/* storage fot the parameter needed by the handler */
static void *global_p_signal_handler_param = NULL;

/** The actual handler 
	Stops on almost everything .
*/
static void unix_signal_handler(int signo)
{	
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) global_p_signal_handler_param;
	if (p_self == NULL)
	{
		DBG_PRINTF((LOG_WARNING,MODULE_TAG "Signal '0x%x' received. But handler not correctly installed.\n", signo));		
		return;
	}
	switch(signo)
	{
		case SIGHUP:
		case SIGINT:
		case SIGQUIT:
		case SIGALRM:
			DBG_PRINTF((LOG_DEBUG,MODULE_TAG "Signal '0x%x' received. Sending 'Shutdown cmd'.\n", signo));
			p_self->cmd = CMD_STOP;			
		break;
	
		default:
			DBG_PRINTF((LOG_DEBUG,MODULE_TAG "Signal '0x%x' received. Ignored.\n", signo));
		break;
	}
	return;
} 

/**
	install handler for SIGALRM and HUP, INT, QUIT.
	avoid receiving HIP,INT, QUIT during ALRM and 
	
*/
RC_TYPE os_install_signal_handler(void *p_dyndns)
{
	RC_TYPE rc;
    struct sigaction    newact;
	newact.sa_handler = unix_signal_handler;
	newact.sa_flags   = 0;

    rc = sigemptyset(&newact.sa_mask)              ||
	         sigaddset(&newact.sa_mask, SIGHUP)   ||
	         sigaddset(&newact.sa_mask, SIGINT)   ||
	         sigaddset(&newact.sa_mask, SIGQUIT)  ||
             sigaction(SIGALRM, &newact, NULL)    ||
             sigemptyset(&newact.sa_mask)          ||
	         sigaddset(&newact.sa_mask, SIGALRM)  ||
             sigaction(SIGHUP, &newact, NULL)     ||
             sigaction(SIGINT, &newact, NULL)     ||
             sigaction(SIGQUIT, &newact, NULL);
 	if (rc != RC_OK)
 	{
 		DBG_PRINTF((LOG_WARNING,"Error '%s' (0x%x) installing OS signal handler\n", rc));
	}
	else
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
    
    if(pid < 0)
    {
        return RC_OS_FORK_FAILURE;
    }
    
    if (pid == 0)
    { /* child */
        fclose(stdin);
        fclose(stderr);
        setsid();  
        chdir("/");
        umask(0);  
        return RC_OK;
    }
    else 
    {
        exit(0);
    }
    return RC_OK;
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
		DBG_PRINTF((LOG_WARNING, "Error changing uid/gid: %s\n", strerror(errno)));
		return RC_OS_CHANGE_PERSONA_FAILURE;
	}
	return RC_OK;
}
#endif
