/* Interface for main DDNS functions
 *
 * Copyright (C) 2003-2004  Narcis Ilisei <inarcis2002@hotpop.com>
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

#ifndef DYNDNS_H_
#define DYNDNS_H_

#include "os.h"
#include "error.h"
#include "http.h"
#include "debug.h"
#include "plugin.h"

#define VERSION_STRING	"Inadyn version " VERSION " -- Dynamic DNS update client."
#define AGENT_NAME	"inadyn/" VERSION
#define SUPPORT_ADDR	"troglobit@gmail.com"

/* Test values */
#define DEFAULT_CONFIG_FILE	"/etc/inadyn.conf"
#define RUNTIME_DATA_DIR	"/var/run"
#define DEFAULT_PIDFILE		RUNTIME_DATA_DIR "/inadyn.pid"

#define DYNDNS_MY_IP_SERVER	"checkip.dyndns.org"
#define DYNDNS_MY_CHECKIP_URL	"/"

#define DEFAULT_DNS_SYSTEM	"default@dyndns.org"

/* Conversation with the checkip server */
#define DYNDNS_CHECKIP_HTTP_REQUEST  					\
	"GET %s HTTP/1.0\r\n"						\
	"Host: %s\r\n"							\
	"User-Agent: " AGENT_NAME " " SUPPORT_ADDR "\r\n\r\n"

/* Some default configurations */
#define DYNDNS_DEFAULT_STARTUP_SLEEP      0       /* sec */
#define DYNDNS_DEFAULT_SLEEP              120     /* sec */
#define DYNDNS_MIN_SLEEP                  3       /* sec */
#define DYNDNS_MAX_SLEEP                  (10 * 24 * 3600)        /* 10 days in sec */
#define DYNDNS_ERROR_UPDATE_PERIOD        600     /* 10 min */
#define DYNDNS_FAILED_UPDATE_PERIOD       300     /* 5 min */
#define DYNDNS_FORCED_UPDATE_PERIOD       (10 * 24 * 3600)        /* 10 days in sec */
#define DYNDNS_DEFAULT_CMD_CHECK_PERIOD   1       /* sec */
#define DYNDNS_DEFAULT_ITERATIONS         0       /* Forever */
#define DYNDNS_HTTP_RESPONSE_BUFFER_SIZE  2500    /* Bytes */
#define DYNDNS_HTTP_REQUEST_BUFFER_SIZE   2500    /* Bytes */
#define DYNDNS_MAX_ALIAS_NUMBER           5       /* maximum number of aliases per server that can be maintained */
#define DYNDNS_MAX_SERVER_NUMBER          3       /* maximum number of servers that can be maintained */

/* local configs */
#define USERNAME_LEN                      50      /* chars */
#define PASSWORD_LEN                      50      /* chars */
#define SERVER_NAME_LEN                   256     /* chars */
#define SERVER_URL_LEN                    256     /* chars */
#ifdef INET6_ADDRSTRLEN
# define MAX_ADDRESS_LEN                  INET6_ADDRSTRLEN
#else
# define MAX_ADDRESS_LEN                  46
#endif

typedef enum {
	NO_CMD = 0,
	CMD_STOP,
	CMD_RESTART,
	CMD_FORCED_UPDATE,
	CMD_CHECK_NOW,
} ddns_cmd_t;

typedef struct {
	char           username[USERNAME_LEN];
	char           password[PASSWORD_LEN];
	char          *encoded_password;
	int            size;
	int            encoded;
} ddns_creds_t;

/* Server name and port */
typedef struct {
	char           name[SERVER_NAME_LEN];
	int            port;
} ddns_name_t;

typedef struct {
	int            ip_has_changed;
	char           address[MAX_ADDRESS_LEN];

	char           name[SERVER_NAME_LEN];
	int            update_required;
	time_t         last_update;
} ddns_alias_t;

typedef struct {
	int            id;

	ddns_creds_t   creds;
	ddns_system_t *system;

	/* Address of DDNS update service */
	ddns_name_t    server_name;
	char           server_url[SERVER_URL_LEN];

	/* Address of "What's my IP" checker */
	ddns_name_t    checkip_name;
	char           checkip_url[SERVER_URL_LEN];

	ddns_name_t    proxy_server_name;

	ddns_alias_t   alias[DYNDNS_MAX_ALIAS_NUMBER];
	int            alias_count;

	int            wildcard;

	int            ssl_enabled;
	int            append_myip; /* For custom setups! */
} ddns_info_t;

/* Client context */
typedef struct {
	char          *cfgfile;
	char          *external_command;

	ddns_dbg_t     dbg;

	ddns_cmd_t     cmd;
	int            startup_delay_sec;
	int            sleep_sec; /* time between 2 updates */
	int            normal_update_period_sec;
	int            error_update_period_sec;
	int            forced_update_period_sec;
	int            forced_update_fake_addr;
	int            cmd_check_period; /*time to wait for a command */
	int            total_iterations;
	int            num_iterations;
	char          *bind_interface;
	char          *check_interface;
	int            initialized;
	int            run_in_background;
	int            debug_to_syslog;
	int            change_persona;
	int            update_once;
	int            force_addr_update;
	int            use_proxy;
	int            abort;

	http_t         http_to_ip_server[DYNDNS_MAX_SERVER_NUMBER];
	http_t         http_to_dyndns[DYNDNS_MAX_SERVER_NUMBER];
	http_trans_t   http_transaction;

	char          *work_buf; /* for HTTP responses */
	int            work_buflen;

	char          *request_buf; /* for HTTP requests */
	int            request_buflen;

	ddns_user_t    sys_usr_info; /* info about the current account running inadyn */

	ddns_info_t    info[DYNDNS_MAX_SERVER_NUMBER]; /* servers, names, passwd */
	int            info_count;
} ddns_t;

int  ddns_main_loop    (ddns_t *ctx, int argc, char *argv[]);
int  get_config_data   (ddns_t *ctx, int argc, char *argv[]);

int common_request (ddns_t       *ctx,   ddns_info_t *info, ddns_alias_t *alias);
int common_response(http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias);

#endif /* DYNDNS_H_ */

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
