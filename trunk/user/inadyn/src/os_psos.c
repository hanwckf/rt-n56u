/*
Copyright (C) 2003-2004 Narcis Ilisei

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
#include "os.h"
#include "dyndns.h"
#include <stdio.h>

#ifdef PSOS_OS

	void os_sleep_ms(int ms)
	{
		tm_wkafter(MS2TICKS(ms));
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


/* MAIN FUNCTION */
RC_TYPE os_install_signal_handler(void* p_dyndns)
{
    return RC_OK;
}

/*
	closes current console 
*/
RC_TYPE close_console_window(void)
{
    return RC_OK;
}

/*
     dummy. 
*/
struct hostent FAR *gethostbyname(const char FAR *name )
{
    return NULL;
}

RC_TYPE os_syslog_open(const char *p_prg_name)
{
    return RC_OK;
}

RC_TYPE os_syslog_close(void)
{
    return RC_OK;
}

RC_TYPE os_change_persona(OS_USER_INFO *p_usr_info)
{
	return RC_OS_CHANGE_PERSONA_FAILURE;
}

#endif 

