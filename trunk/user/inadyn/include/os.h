/*
 * Copyright (C) 2003-2004  Narcis Ilisei
 * Copyright (C) 2006       Steve Horbachuk
 * Copyright (C) 2010-2014  Joachim Nilsson <troglobit@gmail.com>
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
 * along with this program; if not, visit the Free Software Foundation
 * website at http://www.gnu.org/licenses/gpl-2.0.html or write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#ifndef _OS_H_INCLUDED
#define _OS_H_INCLUDED

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

#include "error.h"

typedef struct {
	uid_t uid;
	gid_t gid;
} ddns_user_t;

/* Blocks a thread the specified number of milliseconds*/
void os_sleep_ms(int ms);

/* returns the last error that happend in the os ip_support */
int os_get_socket_error(void);

/* NETWORK SUPPORT*/

/* Socket system start */
int os_ip_support_startup(void);

/* Socket system stop*/
int os_ip_support_cleanup(void);

/* OS SIGNALS */
int os_install_signal_handler(void *);

/* console */
int close_console_window(void);

enum {
	DBG_STD_LOG = 0,	/* stdout */
	DBG_SYS_LOG,	/* syslog */
	DBG_FILE_LOG	/* file output */
};

/**
 * Opens the dbg output for the required destination.
 *
 */
int os_open_dbg_output(int dest, const char *name, const char *logfile);

/**
 * Closes the dbg output device.
 */
int os_close_dbg_output(void);

/**
 * Opens the system's syslog. The prg name is what will be printed before every message.
 */
int os_syslog_open(const char *name);
int os_syslog_close(void);

/**
 * Drop privileges to the given user
 */
int os_change_persona (ddns_user_t *user);

/**
 * Check file system permissions and create pidfile
 */
int os_check_perms(void *);

/**
 * Execute command on successful update.
 */
int os_shell_execute(char *cmd, char *ip, char *hostname, char *iface);

#endif				/* _OS_H_INCLUDED */
/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
