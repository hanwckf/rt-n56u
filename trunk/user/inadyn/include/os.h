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

#ifndef _OS_H_INCLUDED
#define _OS_H_INCLUDED

/* Include the correct headers for each OS.
   TODO Note: This is a very thin and hacked way of supporting multiple OSes.
   Must be another way of doing it.
*/
#define OS_DEF_OK 0

#ifdef _WIN32
	#include <Windows.h>
    #undef OS_DEF_OK
    #define OS_DEF_OK  1
    #ifndef BOOL
	#define BOOL WINBOOL
	#define HAVE_OS_BOOL 1
    #endif
#endif

#ifdef RMC_DEBUG_VERSION
    #undef OS_DEF_OK
    #define OS_DEF_OK 1
    #define PSOS_OS 1
    #include <rmctypes.h>
    #include <rmcmacros.h>

    #define HAVE_OS_BOOL 1
    #include <psos.h>
    #include "psos_net.h"
#endif

#if OS_DEF_OK == 0
  /*default to Unix*/
    #define UNIX_OS  1
    #define HAVE_OS_SYSLOG 1

    #include <unistd.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <errno.h>
    #include <string.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <sys/wait.h>
    #include <signal.h>
    #include <sys/socket.h>
    #include <netdb.h>
    #include <syslog.h>

    typedef int SOCKET;
    #define closesocket close

    #define FAR
#endif

#ifndef IN_PRIVATE
#define IN_PRIVATE(addr) (((addr & IN_CLASSA_NET) == 0x0a000000) ||  \
                          ((addr & 0xfff00000)    == 0xac100000) ||  \
                          ((addr & IN_CLASSB_NET) == 0xc0a80000))
#endif

#ifndef IN_LINKLOCAL
#define IN_LINKLOCALNETNUM	0xa9fe0000
#define IN_LINKLOCAL(addr) ((addr & IN_CLASSB_NET) == IN_LINKLOCALNETNUM)
#endif

#ifndef IN_LOOPBACK
#define IN_LOOPBACK(addr) ((addr & IN_CLASSA_NET) == 0x7f000000)
#endif

#ifndef IN_ZERONET
#define IN_ZERONET(addr) ((addr & IN_CLASSA_NET) == 0)
#endif

#ifndef __TIMESTAMP__
    #define __TIMESTAMP__   "0"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*real begin */
#include "errorcode.h"

#ifndef FALSE
    #define FALSE (1 == 0)
    #define TRUE  !FALSE
#endif

#ifndef HAVE_OS_BOOL
    typedef int BOOL;
#endif

#ifndef UNIX_OS
    typedef unsigned int uid_t;
    typedef unsigned int gid_t;
#endif


/* general defines */
#ifndef SOCKET_ERROR
    #define SOCKET_ERROR    (-1)
#endif
#define IP_V4_IP_ADDR_FORMAT "%u.%u.%u.%u"

/** Syslog support */
#ifndef HAVE_OS_SYSLOG
    #define LOG_EMERG       0       /* system is unusable */
    #define LOG_ALERT       1       /* action must be taken immediately */
    #define LOG_CRIT        2       /* critical conditions */
    #define LOG_ERR         3       /* error conditions */
    #define LOG_WARNING     4       /* warning conditions */
    #define LOG_NOTICE      5       /* normal but significant condition */
    #define LOG_INFO        6       /* informational */
    #define LOG_DEBUG       7       /* debug-level messages */
#endif

typedef enum
{
		OS_CTRL_C_SIGNAL = 0,
		OS_CTRL_CLOSE_SIGNAL = 1,
		OS_CTRL_BREAK_SIGNAL = 2,
		OS_CTRL_LOGOFF_SIGNAL = 3,
		OS_CTRL_SHUTDOWN_SIGNAL = 4,
		LAST_SIGNAL = -1
} OS_SIGNALS;

typedef struct
{
	OS_SIGNALS signal;
	void *p_in_data;
	void *p_out_data;
} OS_SIGNAL_TYPE;

typedef int (*OS_SIGNAL_HANDLER_FUNC) (OS_SIGNAL_TYPE, void*);
typedef struct
{
	OS_SIGNAL_HANDLER_FUNC p_func;
	void *p_in_data;
} OS_SIGNAL_HANDLER_TYPE;

typedef struct
{
	uid_t uid;
	gid_t gid;
} OS_USER_INFO;

RC_TYPE os_change_persona(OS_USER_INFO *p_usr_info);

/* Blocks a thread the specified number of milliseconds*/
void os_sleep_ms(int ms);

/* returns the last error that happend in the os ip_support */
int  os_get_socket_error (void);

/* NETWORK SUPPORT*/

/* Socket system start */
RC_TYPE os_ip_support_startup(void);

/* Socket system stop*/
RC_TYPE os_ip_support_cleanup(void);

/* OS SIGNALS */
RC_TYPE os_install_signal_handler(void*);

/* MAIN - Dyn DNS update entry point.*/
int inadyn_main(int argc, char* argv[]);

/* Threads */
typedef struct
{
	void (*p_func) (void*);
	void *p_params;
	int priority;
	unsigned int nStackSize;
	unsigned int dwCreateFlags;
} OS_THREAD_TYPE;


int thread_start(OS_THREAD_TYPE *p_thread);

/* console */
RC_TYPE close_console_window(void);

typedef enum
{
    DBG_STD_LOG, /*stdout*/
    DBG_SYS_LOG, /*syslog, for Unix only*/
    DBG_FILE_LOG /*file output*/
} DBG_DEST;

/**
 * Opens the dbg output for the required destination.
 *
 */
RC_TYPE os_open_dbg_output(DBG_DEST dest, const char *p_prg_name, const char *p_logfile_name);

/**
 * Closes the dbg output device.
 */
RC_TYPE os_close_dbg_output(void);


/**
 * Opens the system's syslog. The prg name is what will be printed before every message.
 */
RC_TYPE os_syslog_open(const char *p_prg_name);
RC_TYPE os_syslog_close(void);

/**
 * Execute command on successful update.
 */
RC_TYPE os_shell_execute(char *p_cmd);

#ifdef __cplusplus
}
#endif

#endif /* _OS_H_INCLUDED */
