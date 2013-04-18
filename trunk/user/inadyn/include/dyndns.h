/* Interface for main ddns functions
 *
 * Copyright (C) 2003-2004  Narcis Ilisei <inarcis2002@hotpop.com>
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

#ifndef _DYNDNS_INCLUDED
#define _DYNDNS_INCLUDED

#include "os.h"
#include "errorcode.h"
#include "http_client.h"
#include "debug_if.h"

#define DYNDNS_VERSION_STRING	"Inadyn version " VERSION_STRING " -- Dynamic DNS update client."
#define DYNDNS_AGENT_NAME	"inadyn/" VERSION_STRING
#define DYNDNS_EMAIL_ADDR	"admin@vampik.ru"

typedef enum
{
	DYNDNS_DEFAULT,
	FREEDNS_AFRAID_ORG_DEFAULT,
	ZONE_EDIT_DEFAULT,
	CUSTOM_HTTP_BASIC_AUTH,
	NOIP_DEFAULT,
	EASYDNS_DEFAULT,
	TZO_DEFAULT,
	DYNDNS_3322_DYNAMIC,
	SITELUTIONS_DOMAIN,
	DNSOMATIC_DEFAULT,
	DNSEXIT_DEFAULT,
	HE_IPV6TB,
	HE_DYNDNS,
	CHANGEIP_DEFAULT,
	DYNSIP_DEFAULT,
	ASUS_REGISTER,
	ASUS_UPDATE,
	LAST_DNS_SYSTEM = -1
} DYNDNS_SYSTEM_ID;

/*test values*/
#define DYNDNS_DEFAULT_DEBUG_LEVEL	1
#define DYNDNS_DEFAULT_CONFIG_FILE	"/etc/inadyn.conf"
#define DYNDNS_RUNTIME_DATA_DIR		"/var/run/inadyn"
#define DYNDNS_DEFAULT_CACHE_FILE	DYNDNS_RUNTIME_DATA_DIR"/inadyn.cache"
#define DYNDNS_DEFAULT_PIDFILE		DYNDNS_RUNTIME_DATA_DIR"/inadyn.pid"

#define DYNDNS_MY_IP_SERVER		"checkip.dyndns.org"
#define DYNDNS_MY_IP_SERVER_URL		"/"

/*REQ/RSP definitions*/

#define DYNDNS_DEFAULT_DNS_SYSTEM	DYNDNS_DEFAULT

/* Conversation with the IP server */
#define DYNDNS_GET_IP_HTTP_REQUEST  \
	"GET %s HTTP/1.0\r\n"						\
	"Host: %s\r\n"							\
	"User-Agent: " DYNDNS_AGENT_NAME " " DYNDNS_EMAIL_ADDR "\r\n\r\n"

#define GENERIC_HTTP_REQUEST                                      \
	"GET %s HTTP/1.0\r\n"						\
	"Host: %s\r\n"							\
	"User-Agent: " DYNDNS_AGENT_NAME " " DYNDNS_EMAIL_ADDR "\r\n\r\n"

#define GENERIC_AUTH_HTTP_REQUEST					\
	"GET %s HTTP/1.0\r\n"						\
	"Host: %s\r\n"							\
	"Authorization: Basic %s\r\n"					\
	"User-Agent: " DYNDNS_AGENT_NAME " " DYNDNS_EMAIL_ADDR "\r\n\r\n"

/* dyndns.org specific update address format
 * 3322.org has the same parameters ... */
#define DYNDNS_UPDATE_IP_HTTP_REQUEST					\
	"GET %s?"							\
	"hostname=%s&"							\
	"myip=%s "							\
	"HTTP/1.0\r\n"							\
	"Host: %s\r\n"							\
	"Authorization: Basic %s\r\n"					\
	"User-Agent: " DYNDNS_AGENT_NAME " " DYNDNS_EMAIL_ADDR "\r\n\r\n"

/* freedns.afraid.org specific update request format */
#define FREEDNS_UPDATE_IP_REQUEST					\
	"GET %s?"							\
	"%s&"								\
	"address=%s "							\
	"HTTP/1.0\r\n"							\
	"Host: %s\r\n"							\
	"User-Agent: "DYNDNS_AGENT_NAME " " DYNDNS_EMAIL_ADDR"\r\n\r\n"

/** generic update format for sites that perform the update
	with:
	http://some.address.domain/somesubdir
	?some_param_name=MY_ALIAS
	and then the normal http stuff and basic base64 encoded auth.
	The parameter here is the entire request but NOT including the alias.
*/
#define GENERIC_BASIC_AUTH_UPDATE_IP_REQUEST				\
	"GET %s"							\
	"%s "								\
	"HTTP/1.0\r\n"							\
	"Host: %s\r\n"							\
	"Authorization: Basic %s\r\n"					\
	"User-Agent: "DYNDNS_AGENT_NAME " " DYNDNS_EMAIL_ADDR"\r\n\r\n"

#define ZONEEDIT_UPDATE_IP_REQUEST					\
	"GET %s?"						\
	"host=%s&"							\
	"dnsto=%s "							\
	"HTTP/1.0\r\n"							\
	"Host: %s\r\n"							\
	"Authorization: Basic %s\r\n"					\
	"User-Agent: "DYNDNS_AGENT_NAME " " DYNDNS_EMAIL_ADDR"\r\n\r\n"

/* dont ask me why easydns is so picky
 * http://support.easydns.com/tutorials/dynamicUpdateSpecs.php */
#define EASYDNS_UPDATE_IP_REQUEST				\
	"GET %s?"							\
	"hostname=%s&"							\
	"myip=%s&"							\
	"wildcard=%s "							\
	"HTTP/1.0\r\n"							\
	"Host: %s\r\n"							\
	"Authorization: Basic %s\r\n"					\
	"User-Agent: "DYNDNS_AGENT_NAME " " DYNDNS_EMAIL_ADDR"\r\n\r\n"

/* tzo doesnt encode password */
#define TZO_UPDATE_IP_REQUEST						\
	"GET %s?"							\
	"tzoname=%s&"							\
	"email=%s&"							\
	"tzokey=%s&"							\
	"ipaddress=%s&"							\
	"system=tzodns&"							\
	"info=1 "							\
	"HTTP/1.0\r\n"							\
	"Host: %s\r\n"							\
	"User-Agent: "DYNDNS_AGENT_NAME " " DYNDNS_EMAIL_ADDR"\r\n\r\n"

/* sitelutions.com specific update address format */
#define SITELUTIONS_UPDATE_IP_HTTP_REQUEST				\
	"GET %s?"							\
	"user=%s&"							\
	"pass=%s&"							\
	"id=%s&"							\
	"ip=%s "							\
	"HTTP/1.0\r\n"							\
	"Host: %s\r\n"							\
	"User-Agent: "DYNDNS_AGENT_NAME " " DYNDNS_EMAIL_ADDR"\r\n\r\n"

#define DNSEXIT_UPDATE_IP_HTTP_REQUEST					\
	"GET %s?"							\
	"login=%s&"							\
	"password=%s&"							\
	"host=%s&"							\
	"myip=%s "							\
	"HTTP/1.0\r\n"							\
	"Host: %s\r\n"							\
	"User-Agent: "DYNDNS_AGENT_NAME " " DYNDNS_EMAIL_ADDR"\r\n\r\n"

/* HE tunnelbroker.com specific update request format */
#define HE_IPV6TB_UPDATE_IP_REQUEST					\
	"GET %s?"							\
	"username=%s&"							\
	"password=%s&"							\
	"hostname=%s&"							\
	"myip=%s "							\
	"HTTP/1.0\r\n"							\
	"Host: %s\r\n"							\
	"User-Agent: "DYNDNS_AGENT_NAME " " DYNDNS_EMAIL_ADDR"\r\n\r\n"

#define CHANGEIP_UPDATE_IP_HTTP_REQUEST					\
	"GET %s?"							\
	"system=dyndns&"						\
	"hostname=%s&"							\
	"myip=%s "							\
	"HTTP/1.0\r\n"							\
	"Host: %s\r\n"							\
	"Authorization: Basic %s\r\n"					\
	"User-Agent: " DYNDNS_AGENT_NAME " " DYNDNS_EMAIL_ADDR "\r\n\r\n"

#define ASUS_PROCESS_MY_IP_REQUEST_FORMAT \
	"GET %s?" \
	"hostname=%s&" \
	"myip=%s " \
	 "HTTP/1.0\r\n" \
	"Authorization: Basic %s\r\n" \
	"Host: %s\r\n" \
	"User-Agent: "DYNDNS_AGENT_NAME " " DYNDNS_EMAIL_ADDR"\r\n\r\n"

/* Some default configurations */
#define DYNDNS_DEFAULT_SLEEP			(120) /* sec */
#define DYNDNS_MIN_SLEEP			(3)   /* sec */
#define DYNDNS_MAX_SLEEP			(10 * 24 * 3600) /* 10 days in sec */
#define DYNDNS_ERROR_UPDATE_PERIOD		(600) /* 10 min */
#define DYNDNS_FORCED_UPDATE_PERIOD		(7 * 24 * 3600)  /* 7 days in sec */
#define DYNDNS_DEFAULT_CMD_CHECK_PERIOD		(1)    /* sec */
#define DYNDNS_DEFAULT_ITERATIONS		0      /* Forever */
#define DYNDNS_HTTP_RESPONSE_BUFFER_SIZE	(2500) /* Bytes */
#define DYNDNS_HTTP_REQUEST_BUFFER_SIZE		(2500) /* Bytes */
#define DYNDNS_MAX_ALIAS_NUMBER			10 /* maximum number of aliases per server that can be maintained */
#define DYNDNS_MAX_SERVER_NUMBER		5  /* maximum number of servers that can be maintained */

/* local configs */
#define DYNDNS_MY_IP_ADDRESS_LENGTH		20  /* chars */
#define DYNDNS_MY_USERNAME_LENGTH		50  /* chars */
#define DYNDNS_MY_PASSWORD_LENGTH		50  /* chars */
#define DYNDNS_SERVER_NAME_LENGTH		256 /* chars */
#define DYNDNS_SERVER_URL_LENGTH		256 /* chars */
#define IP_V4_MAX_LENGTH			16  /* chars: nnn.nnn.nnn.nnn\0 */


/* typedefs */
struct _DYN_DNS_CLIENT;
struct DYNDNS_SYSTEM;

/* Types used for DNS system specific configuration */
/* Function to prepare DNS system specific server requests */
typedef int (*DNS_SYSTEM_REQUEST_FUNC)(struct _DYN_DNS_CLIENT *this, int infnr, int alnr);
typedef RC_TYPE (*DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)(struct _DYN_DNS_CLIENT *this, HTTP_TRANSACTION *p_tr, int infnr);
typedef struct
{
	const char* p_key;
	DNS_SYSTEM_SRV_RESPONSE_OK_FUNC p_rsp_ok_func;
	DNS_SYSTEM_REQUEST_FUNC p_dns_update_req_func;
	const char *p_ip_server_name;
	const char *p_ip_server_url;
	const char *p_dyndns_server_name;
	const char *p_dyndns_server_url;
} DYNDNS_SYSTEM;

typedef struct
{
	DYNDNS_SYSTEM_ID id;
	DYNDNS_SYSTEM system;
} DYNDNS_SYSTEM_INFO;

typedef enum
{
	NO_CMD = 0,
	CMD_STOP = 1,
	CMD_RESTART = 2,
} DYN_DNS_CMD;

typedef struct
{
	char my_username[DYNDNS_MY_USERNAME_LENGTH];
	char my_password[DYNDNS_MY_PASSWORD_LENGTH];
	char *p_enc_usr_passwd_buffer;
	int size;
	BOOL encoded;
} DYNDNS_CREDENTIALS;

typedef struct
{
	char name[DYNDNS_SERVER_NAME_LENGTH];
	int port;
} DYNDNS_SERVER_NAME;


typedef struct
{
	DYNDNS_SERVER_NAME names;
	int update_required;
} DYNDNS_ALIAS_INFO;

typedef struct
{
	BOOL                my_ip_has_changed;
	DYNDNS_SERVER_NAME  my_ip_address;
	DYNDNS_CREDENTIALS  credentials;
	DYNDNS_SYSTEM      *p_dns_system;
	DYNDNS_SERVER_NAME  dyndns_server_name;
	char                dyndns_server_url[DYNDNS_SERVER_URL_LENGTH];
	DYNDNS_SERVER_NAME  ip_server_name;
	char                ip_server_url[DYNDNS_SERVER_URL_LENGTH];
	DYNDNS_SERVER_NAME  proxy_server_name;
	DYNDNS_ALIAS_INFO   alias_info[DYNDNS_MAX_ALIAS_NUMBER];
	int                 alias_count;
	BOOL wildcard;
} DYNDNS_INFO_TYPE;

typedef struct
{
	uid_t uid;
	gid_t gid;
} USER_INFO;

typedef struct DYN_DNS_CLIENT
{
	char *cfgfile;
	char *cachefile;
	char *pidfile;
	char *external_command;

	DYN_DNS_CMD  cmd;
	int          sleep_sec; /* time between 2 updates*/
	int          normal_update_period_sec;
	int          error_update_period_sec;
	int          forced_update_period_sec;
	int          time_since_last_update;
	int          cmd_check_period; /*time to wait for a command*/
	int          total_iterations;
	int          num_iterations;
	char        *bind_interface;
	char        *check_interface;
	BOOL         initialized;
	BOOL         run_in_background;
	BOOL         debug_to_syslog;
	BOOL         change_persona;

	HTTP_CLIENT       http_to_ip_server[DYNDNS_MAX_SERVER_NUMBER];
	HTTP_CLIENT       http_to_dyndns[DYNDNS_MAX_SERVER_NUMBER];
	HTTP_TRANSACTION  http_tr;
	char             *p_work_buffer; /* for HTTP responses*/
	int               work_buffer_size;
	char             *p_req_buffer; /* for HTTP requests*/
	int               req_buffer_size;

	USER_INFO         sys_usr_info; /* info about the current account running inadyn */
	DYNDNS_INFO_TYPE  info[DYNDNS_MAX_SERVER_NUMBER]; /* servers, names, passwd */
	int               info_count;

	BOOL abort_on_network_errors;
	BOOL force_addr_update;
	BOOL use_proxy;
	BOOL abort;

	DBG_TYPE dbg;
} DYN_DNS_CLIENT;
/*public functions*/
/** Returns the table of supported dyndns systems
*/
DYNDNS_SYSTEM_INFO* get_dyndns_system_table(void);

/**
 *  Returns the default DYNDNS client config data.
*/
RC_TYPE get_default_config_data(DYN_DNS_CLIENT *p_self);

/*
	Set up all details:
		- ip server name
		- dns server name
		- username, passwd
		- ...
	Implementation:
		- load defaults
		- parse cmd line
	- assign settings that may change due to cmd line options
		- check data
*/
RC_TYPE get_config_data(DYN_DNS_CLIENT *p_self, int argc, char** argv);

/*
	printout of version
*/
void dyn_dns_print_hello(void);

/*
	 basic resource allocations for the dyn_dns object
*/
RC_TYPE dyn_dns_construct(DYN_DNS_CLIENT **pp_self);

/*
	Resource free.
*/
RC_TYPE dyn_dns_destruct(DYN_DNS_CLIENT *p_self);

/*
	Sets up the object.
	- sets the IPs of the DYN DNS server
	- ...
*/
RC_TYPE dyn_dns_init(DYN_DNS_CLIENT *p_self);

/*
	Disconnect and some other clean up.
*/
RC_TYPE dyn_dns_shutdown(DYN_DNS_CLIENT *p_self);



/* the real action:
	- detect current IP
		- connect to an HTTP server
		- parse the response for IP addr

	- for all the names that have to be maintained
		- get the current DYN DNS address from DYN DNS server
		- compare and update if neccessary
*/
RC_TYPE dyn_dns_update_ip(DYN_DNS_CLIENT *p_self);

/* MAIN - Dyn DNS update entry point.*/

/*
	Actions:
		- read the configuration options
		- create and init dyn_dns object.
		- launch the IP update action
*/
int dyn_dns_main(DYN_DNS_CLIENT *p_self, int argc, char* argv[]);


/*
	help.
*/
void print_help_page(void);

/*
    main entry point
*/
int inadyn_main(int argc, char* argv[]);

#endif /*_DYNDNS_INCLUDED*/

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "ellemtel"
 *  c-basic-offset: 8
 * End:
 */
