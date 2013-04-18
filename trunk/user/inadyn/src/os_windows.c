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
#include <stdio.h>
#include "dyndns.h"

#ifdef _WIN32
void os_sleep_ms(int ms)
{
    Sleep(ms);
}

int  os_get_socket_error (void)
{
    return WSAGetLastError();
}

RC_TYPE os_ip_support_startup(void)
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD( 2, 2 );

    err = WSAStartup( wVersionRequested, &wsaData );
    if ( err != 0 ) {
	/* Tell the user that we could not find a usable */
	/* WinSock DLL.                                  */
	return RC_IP_OS_SOCKET_INIT_FAILED;
    }

    /* The WinSock DLL is acceptable. Proceed. */
    return RC_OK;
}

RC_TYPE os_ip_support_cleanup(void)
{
    WSACleanup();
    return RC_OK;
}


/* OS SIGNALS Support */
    static OS_SIGNAL_HANDLER_TYPE global_os_handler = { NULL, NULL};

typedef struct
{
    DWORD	original;
    int		translated;
} OS_EVENT_TYPE;

static const OS_EVENT_TYPE os_events_table [] =
{
    {CTRL_C_EVENT, OS_CTRL_C_SIGNAL},
    {CTRL_CLOSE_EVENT, OS_CTRL_CLOSE_SIGNAL},
    {CTRL_BREAK_EVENT, OS_CTRL_BREAK_SIGNAL},
    {CTRL_LOGOFF_EVENT, OS_CTRL_LOGOFF_SIGNAL},
    {CTRL_SHUTDOWN_EVENT, OS_CTRL_SHUTDOWN_SIGNAL},

    {-1, LAST_SIGNAL}
};

static int translate_os_signal(DWORD in)
{
    const OS_EVENT_TYPE *it = os_events_table;
    while (it->original != -1)
    {
	if (it->original == in)
	{
	    return it->translated;
	}
	++it;
    }
    return -1;
}

static BOOL WINAPI os_signal_wrapper_handler( DWORD dwCtrlType)
{
    OS_SIGNAL_TYPE signal;
    signal.signal = translate_os_signal(dwCtrlType);
    signal.p_in_data = NULL;
    signal.p_out_data = NULL;

    if (global_os_handler.p_func != NULL)
    {
	return global_os_handler.p_func(signal, global_os_handler.p_in_data) ? 1 : 0;
    }
    else
    {
	return 0;
    }
}

/**
	The actual signal handler for Windows.
	Does not respond on LOGOFF signal.
	Exits on shutdown ..., Ctl-C,...
*/
static int dyndns_win32_signal_handler_func(OS_SIGNAL_TYPE signal, void *p_in)
{
	int ret_flag = 0;

	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_in;
	if (p_self == NULL)
	{
		return 0;
	}

	switch (signal.signal)
	{
		case OS_CTRL_C_SIGNAL :
		case OS_CTRL_CLOSE_SIGNAL :
		case OS_CTRL_BREAK_SIGNAL :
		case OS_CTRL_SHUTDOWN_SIGNAL :
				logit(LOG_INFO, MODULE_TAG "Signal %d received. Sending shutdown command.", signal);
				ret_flag = 1;
				p_self->cmd = CMD_STOP;
			break;

		case OS_CTRL_LOGOFF_SIGNAL :
		default:
				logit(LOG_DEBUG, MODULE_TAG "Signal %d received. NO ACTION.", signal);
	}
	return ret_flag;
}

/* MAIN FUNCTION */
RC_TYPE os_install_signal_handler(void *p_dyndns)
{
    BOOL fSuccess;
    if (global_os_handler.p_func != NULL || p_dyndns == NULL)
    {
	return RC_OS_ERROR_INSTALLING_SIGNAL_HANDLER;
    }

    fSuccess = SetConsoleCtrlHandler(
	(PHANDLER_ROUTINE) os_signal_wrapper_handler,  /* handler function */
	TRUE);                           /* add to list */

    if (fSuccess)
    {
	global_os_handler.p_func = dyndns_win32_signal_handler_func;
		global_os_handler.p_in_data = p_dyndns;
	return RC_OK;
    }
    else
    {
	return RC_OS_ERROR_INSTALLING_SIGNAL_HANDLER;
    }
}

/*
    closes current console
    A rather bad function. I am pretty sure that there is
    another way to into background under Windows.

*/
RC_TYPE close_console_window(void)
{
    fclose(stdin);
    fclose(stderr);
    FreeConsole( );
    return RC_OK;
}

RC_TYPE os_syslog_open(const char *p_prg_name)
{
    return RC_OK;
}

RC_TYPE os_syslog_close(void)
{
    return RC_OK;
}

/*
    thanks pagedude
*/
RC_TYPE os_shell_execute(char * p_cmd)
{
	RC_TYPE rc = RC_OK;
	HANDLE hProcess = NULL;
	SHELLEXECUTEINFO shellInfo;

	ZeroMemory(&shellInfo, sizeof(shellInfo));
	shellInfo.cbSize = sizeof(shellInfo);
	shellInfo.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
	shellInfo.lpFile = p_cmd;
	/*shellInfo.lpParameters = args;*/
	if (!ShellExecuteEx(&shellInfo))
	{
		rc = RC_OS_FORK_FAILURE;
	}

	return rc;
}

static unsigned long os_get_inet_addr(char* addr)
{
    unsigned long b3;
    unsigned long b2;
    unsigned long b1;
    unsigned long b0;
    unsigned long ipa;
    int n;

    ipa = 0x0;
    n = sscanf(addr, "%d.%d.%d.%d", &b3, &b2, &b1, &b0);
    if (n == 4)
    {
	ipa = (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
    }
    ipa = htonl(ipa);
    return(ipa);
}

/* MAIN - Dyn DNS update entry point.*/
int main(int argc, char* argv[])
{
  return inadyn_main(argc, argv);
}

RC_TYPE os_change_persona(OS_USER_INFO *p_usr_info)
{
	return RC_OS_CHANGE_PERSONA_FAILURE;
}

#endif /*WIN32*/
