/*
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

#define MODULE_TAG ""

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>		/* sysconf() */
#include <pwd.h>		/* getpwnam_r() */
#include <grp.h>		/* getgrnam_r() */

#include "dyndns.h"
#include "debug_if.h"
#include "base64.h"
#include "get_cmd.h"

static int curr_info;

/* command line options */
#define DYNDNS_INPUT_FILE_OPT_STRING "--input_file"

static RC_TYPE help_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_wildcard_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_username_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_password_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_alias_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_dns_server_name_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_dns_server_url_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_ip_server_name_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_dyndns_system_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_update_period_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_update_period_sec_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_forced_update_period_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_logfile_name(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE set_silent_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE set_verbose_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_proxy_server_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_options_from_file_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE set_iterations_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE set_syslog_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE set_change_persona_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE set_bind_interface(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE set_check_interface(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE set_pidfile(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE set_cachefile(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE print_version_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_exec_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);

static CMD_DESCRIPTION_TYPE cmd_options_table[] =
{
	{"-a",			1,	{get_alias_handler, NULL},	"" },
	{"--alias",		1,	{get_alias_handler, NULL},	"<NAME>\n"
	 "\t\t\tAlias hostname. This option can appear multiple times." },

	{"-b",			0,	{set_silent_handler, NULL},	""},
	{"--background",	0,	{set_silent_handler, NULL},	"Run in background."},

	{"-B",			1,	{set_bind_interface, NULL}, ""},
	{"--bind",		1,	{set_bind_interface, NULL}, "<IFNAME>\n"
	 "\t\t\tSet interface to bind to, only on UNIX systems."},

	{"-c",			1,	{set_cachefile, NULL}, ""},
	{"--cachefile",		1,	{set_cachefile, NULL}, "<FILE>\n"
	 "\t\t\tSet cachefile, default " DYNDNS_DEFAULT_CACHE_FILE},

	{"-d",			1,	{set_change_persona_handler, NULL}, ""},
	{"--drop-privs", 	1,	{set_change_persona_handler, NULL}, "<USER[:GROUP]>\n"
	 "\t\t\tAfter init switch to a new user/group.\n"
	 "\t\t\tOnly on UNIX systems."},
	{"--change_persona", 	1,	{set_change_persona_handler, NULL}, NULL}, /* COMPAT */

	{"-e",			1,	{get_exec_handler, NULL}, ""},
	{"--exec",		1,	{get_exec_handler, NULL}, "Full path to external command to run after an IP update."},

	{"-f",			   1,   {get_forced_update_period_handler, NULL}, ""},
	{"--forced-update",        1,   {get_forced_update_period_handler, NULL}, "<SEC>\n"
	 "\t\t\tForced DDNS server update interval. Default: 1 week"},
	{"--forced_update_period", 1,   {get_forced_update_period_handler, NULL}, NULL}, /* COMPAT */

	{"-F",				1, {get_options_from_file_handler, NULL}, ""},
	{"--config",			1, {get_options_from_file_handler, NULL}, "<FILE>\n"
	 "\t\t\tConfiguration file, containing further options.  Default\n"
	 "\t\t\tconfig file: " DYNDNS_DEFAULT_CONFIG_FILE ", is used if inadyn is\n"
	 "\t\t\tcalled without any command line options." },
	{DYNDNS_INPUT_FILE_OPT_STRING,	1, {get_options_from_file_handler, NULL}, NULL},

	{"-H",			2, {get_ip_server_name_handler, NULL}, ""},
	{"--checkip-url",	2, {get_ip_server_name_handler, NULL}, "<NAME[:PORT] URL>\n"
	 "\t\t\tLocal IP is detected by parsing the response after\n"
	 "\t\t\treturned by this server and URL.  The first IP found\n"
	 "\t\t\tin the HTTP response is considered 'my IP'.\n"
	 "\t\t\tDefault value: 'checkip.dyndns.org /'"},
	{"--ip_server_name",	2, {get_ip_server_name_handler, NULL}, NULL},

	{"-n",			1,	{set_iterations_handler, NULL},	""},
	{"--iterations",	1,	{set_iterations_handler, NULL},	"<NUM>\n"
	 "\t\t\tSet the number of DNS updates. Default: 0 (forever)"},

	{"-i",			1,	{set_check_interface, NULL}, ""},
	{"--iface",		1,	{set_check_interface, NULL}, "<IFNAME>\n"
	 "\t\t\tSet interface to check for IP, only on UNIX systems.\n"
	 "\t\t\tExternal IP check is not performed."},

	{"-L",			1,	{get_logfile_name, NULL}, ""},
	{"--logfile",		1,	{get_logfile_name, NULL}, "<FILE>\n"
	 "\t\t\tFull path to log file"},
	{"--log_file",		1,	{get_logfile_name, NULL}, NULL},

	{"-N",		 	 1, {get_dns_server_name_handler, NULL}, ""},
	{"--server-name", 	 1, {get_dns_server_name_handler, NULL}, "[<NAME>[:port]]\n"
	 "\t\t\tThe server that receives the update DNS request.\n"
	 "\t\t\tAllows the use of unknown DNS services that accept HTTP\n"
	 "\t\t\tupdates.  If no proxy is wanted, then it is enough to\n"
	 "\t\t\tset the dyndns system.  Default servers will be taken."},
	{"--dyndns_server_name", 1, {get_dns_server_name_handler, NULL}, NULL},

	{"-U",		 	1, {get_dns_server_url_handler, NULL}, ""},
	{"--server-url", 	1, {get_dns_server_url_handler, NULL}, "<URL>\n"
	 "\t\t\tFull URL relative to DynDNS server root.\n"
	 "\t\t\tEx: /some_script.php?hostname=\n"},
	{"--dyndns_server_url", 1, {get_dns_server_url_handler, NULL}, NULL},

	{"-S",			1,	{get_dyndns_system_handler, NULL}, ""},
	{"--system",		1,	{get_dyndns_system_handler, NULL}, "<PROVIDER>\n"
	 "\t\t\tSelect DDNS service provider, one of the following.\n"
	 "\t\t\to For dyndns.org:         default@dyndns.org\n"
	 "\t\t\to For freedns.afraid.org: default@freedns.afraid.org\n"
	 "\t\t\to For zoneedit.com:       default@zoneedit.com\n"
	 "\t\t\to For no-ip.com:          default@no-ip.com\n"
	 "\t\t\to For easydns.com:        default@easydns.com\n"
	 "\t\t\to For tzo.com:            default@tzo.com\n"
	 "\t\t\to For 3322.org:           dyndns@3322.org\n"
	 "\t\t\to For dnsomatic.com:      default@dnsomatic.com\n"
	 "\t\t\to For tunnelbroker.net:   ipv6tb@he.net\n"
	 "\t\t\to For dns.he.net:         dyndns@he.net\n"
	 "\t\t\to For dynsip.org:         default@dynsip.org\n"
	 "\t\t\to For sitelutions.com:    default@sitelutions.com\n"
	 "\t\t\to For dnsexit.com:   	  default@dnsexit.com\n"
	 "\t\t\to For duckdns.org:   	  default@duckdns.org\n"
	 "\t\t\to For generic:            custom@http_svr_basic_auth\n\n"
	 "\t\t\tDefault value:            default@dyndns.org"},
	{"--dyndns_system",	1,	{get_dyndns_system_handler, NULL}, NULL},

	{"-x",			1,	{get_proxy_server_handler, NULL}, ""},
	{"--proxy-server",	1,	{get_proxy_server_handler, NULL}, "[NAME[:port]]\n"
	 "\t\t\tHTTP proxy server name, and optional port. Default: N/A"},
	{"--proxy_server",	1,	{get_proxy_server_handler, NULL}, NULL}, /* COMPAT */

	{"-T",			1,	{get_update_period_sec_handler, NULL},	""},
	{"--period",		1,	{get_update_period_sec_handler, NULL},	"<SEC>\n"
	 "\t\t\tIP change check interval.  Default: 2 min. Max: 10 days"},
	{"--update_period_sec",	1,	{get_update_period_sec_handler, NULL},	NULL},
	{"--update_period",	1,	{get_update_period_handler, NULL},      NULL},

	{"-P",			1,	{set_pidfile, NULL}, ""},
	{"--pidfile",		1,	{set_pidfile, NULL}, "<FILE>\n"
	 "\t\t\tSet pidfile, default " DYNDNS_DEFAULT_PIDFILE},

	{"-s",			0,	{set_syslog_handler, NULL}, ""},
	{"--syslog",		0,	{set_syslog_handler, NULL},
	 "Force logging to syslog, e.g., /var/log/messages, only on UNIX systems"},

	{"-u",			1,	{get_username_handler, NULL},	""},
	{"--username",		1,	{get_username_handler, NULL},	"<USERNAME>\n"
	 "\t\t\tYour DDNS user name, or hash"},

	{"-p",			1,	{get_password_handler, NULL},	""},
	{"--password",		1,	{get_password_handler, NULL},	"<PASSWORD>\n"
	 "\t\t\tYour DDNS user password."},

	{"-w",			0,	{get_wildcard_handler, NULL}, ""},
	{"--wildcard",		0,	{get_wildcard_handler, NULL}, "Enable domain wildcarding for easydns.com."},

	{"-h",			0,	{help_handler, NULL},	"" },
	{"--help",		0,	{help_handler, NULL},	"This online help." },

	{"-V",			1,	{set_verbose_handler, NULL},   ""},
	{"--verbose",		1,	{set_verbose_handler, NULL},   "Debug level: 0 - 5"},

	{"-v",			0,	{print_version_handler, NULL}, ""},
	{"--version",		0,	{print_version_handler, NULL}, "Show inadyn version"},

	{NULL,			0,	{0, NULL},	NULL }
};


void print_help_page(void)
{
	CMD_DESCRIPTION_TYPE *it;

	puts("Inadyn is a dynamic DNS (DDNS) client.  It does periodic and/or on-demand checks\n"
	     "of your externally visible IP address and updates the hostname to IP mapping at\n"
	     "your DDNS service provider when necessary.\n");
	puts("dyndns.org:\n"
	     "\tinadyn -u username -p password -a my.registrated.name\n");

	it = cmd_options_table;
	while (it->p_option != NULL)
	{
		if (it->p_description)
		{
			if (strlen(it->p_option) == 2)
				printf("  %s, ", it->p_option);
			else
				printf("%-16s  %s\n\r", it->p_option, it->p_description);
		}
		++it;
	}
}

static RC_TYPE help_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *)p_context;

	(void)p_cmd;
	(void)current_nr;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	p_self->abort = TRUE;
	print_help_page();

	return RC_OK;
}

static RC_TYPE get_wildcard_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *)p_context;

	(void)p_cmd;
	(void)current_nr;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	p_self->info[curr_info].wildcard = TRUE;

	return RC_OK;
}

static RC_TYPE set_verbose_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (sscanf(p_cmd->argv[current_nr], "%d", &p_self->dbg.level) != 1)
	{
		return RC_DYNDNS_INVALID_OPTION;
	}
	return RC_OK;
}

static RC_TYPE set_iterations_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (sscanf(p_cmd->argv[current_nr], "%d", &p_self->total_iterations) != 1)
	{
		return RC_DYNDNS_INVALID_OPTION;
	}

	p_self->total_iterations = (p_self->sleep_sec < 0) ?  DYNDNS_DEFAULT_ITERATIONS : p_self->total_iterations;
	return RC_OK;
}

static RC_TYPE set_silent_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *)p_context;

	(void)p_cmd;
	(void)current_nr;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	p_self->run_in_background = TRUE;
	return RC_OK;
}

static RC_TYPE get_logfile_name(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (sizeof(p_self->dbg.p_logfilename) < strlen(p_cmd->argv[current_nr]))
	{
		return  RC_DYNDNS_BUFFER_TOO_SMALL;
	}
	strcpy(p_self->dbg.p_logfilename, p_cmd->argv[current_nr]);
	return RC_OK;
}

static RC_TYPE get_username_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	/*user*/
	if (sizeof(p_self->info[curr_info].credentials.my_username) < strlen(p_cmd->argv[current_nr]))
	{
		return  RC_DYNDNS_BUFFER_TOO_SMALL;
	}
	strcpy(p_self->info[curr_info].credentials.my_username, p_cmd->argv[current_nr]);

	return RC_OK;
}

static RC_TYPE get_password_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	/*password*/
	if (sizeof(p_self->info[curr_info].credentials.my_password) < strlen(p_cmd->argv[current_nr]))
	{
		return  RC_DYNDNS_BUFFER_TOO_SMALL;
	}

	strcpy(p_self->info[curr_info].credentials.my_password, (p_cmd->argv[current_nr]));
	return RC_OK;
}

static RC_TYPE get_alias_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (p_self->info[curr_info].alias_count >= DYNDNS_MAX_ALIAS_NUMBER)
	{
		return RC_DYNDNS_TOO_MANY_ALIASES;
	}

	if (sizeof(p_self->info[curr_info].alias_info[p_self->info[curr_info].alias_count].names) < strlen(p_cmd->argv[current_nr]))
	{
		return  RC_DYNDNS_BUFFER_TOO_SMALL;
	}
	strcpy(p_self->info[curr_info].alias_info[p_self->info[curr_info].alias_count].names.name, (p_cmd->argv[current_nr]));

	p_self->info[curr_info].alias_count++;
	return RC_OK;
}

static RC_TYPE get_name_and_port(char *p_src, char *p_dest_name, int *p_dest_port)
{
	const char *p_port = NULL;
	p_port = strstr(p_src,":");
	if (p_port)
	{
		int port_nr, len;
		int port_ok = sscanf(p_port + 1, "%d",&port_nr);
		if (port_ok != 1)
		{
			return RC_DYNDNS_INVALID_OPTION;
		}
		*p_dest_port = port_nr;
		len = p_port - p_src;
		memcpy(p_dest_name, p_src, len);
		p_dest_name[len] = 0;
	}
	else
	{
		strcpy(p_dest_name, p_src);
	}
	return RC_OK;
}

/** Returns the svr name and port if the format is :
 * name[:port] url.
 */
static RC_TYPE get_ip_server_name_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	RC_TYPE rc;
	int port = -1;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	/*ip_server_name*/
	if (sizeof(p_self->info[curr_info].ip_server_name) < strlen(p_cmd->argv[current_nr]) + 1)
	{
		return  RC_DYNDNS_BUFFER_TOO_SMALL;
	}

	p_self->info[curr_info].ip_server_name.port = HTTP_DEFAULT_PORT;
	rc = get_name_and_port(p_cmd->argv[current_nr], p_self->info[curr_info].ip_server_name.name, &port);
	if (rc == RC_OK && port != -1)
	{
		p_self->info[curr_info].ip_server_name.port = port;
	}

	if (sizeof(p_self->info[curr_info].ip_server_url) < strlen(p_cmd->argv[current_nr + 1]) + 1)
	{
		return  RC_DYNDNS_BUFFER_TOO_SMALL;
	}
	strcpy(p_self->info[curr_info].ip_server_url, p_cmd->argv[current_nr + 1]);

	return rc;
}
static RC_TYPE get_dns_server_name_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	RC_TYPE rc;
	int port = -1;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	/*dyndns_server_name*/
	if (sizeof(p_self->info[curr_info].dyndns_server_name) < strlen(p_cmd->argv[current_nr]))
	{
		return  RC_DYNDNS_BUFFER_TOO_SMALL;
	}

	p_self->info[curr_info].dyndns_server_name.port = HTTP_DEFAULT_PORT;
	rc = get_name_and_port(p_cmd->argv[current_nr], p_self->info[curr_info].dyndns_server_name.name, &port);
	if (rc == RC_OK && port != -1)
	{
		p_self->info[curr_info].dyndns_server_name.port = port;
	}
	return rc;
}
RC_TYPE get_dns_server_url_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	/*dyndns_server_url*/
	if (sizeof(p_self->info[curr_info].dyndns_server_url) < strlen(p_cmd->argv[current_nr]))
	{
		return  RC_DYNDNS_BUFFER_TOO_SMALL;
	}
	strcpy(p_self->info[curr_info].dyndns_server_url, p_cmd->argv[current_nr]);
	return RC_OK;
}

/* returns the proxy server nme and port
 */
static RC_TYPE get_proxy_server_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	RC_TYPE rc;
	int port = -1;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	/*proxy_server_name*/
	if (sizeof(p_self->info[curr_info].proxy_server_name) < strlen(p_cmd->argv[current_nr]))
	{
		return  RC_DYNDNS_BUFFER_TOO_SMALL;
	}

	p_self->info[curr_info].proxy_server_name.port = HTTP_DEFAULT_PORT;
	rc = get_name_and_port(p_cmd->argv[current_nr], p_self->info[curr_info].proxy_server_name.name, &port);
	if (rc == RC_OK && port != -1)
	{
		p_self->info[curr_info].proxy_server_name.port = port;
	}
	return rc;
}
/* Read the dyndnds name update period.
   and impose the max and min limits
*/
static RC_TYPE get_update_period_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (sscanf(p_cmd->argv[current_nr], "%d", &p_self->sleep_sec) != 1)
	{
		return RC_DYNDNS_INVALID_OPTION;
	}

	p_self->sleep_sec /= 1000;
	p_self->sleep_sec = (p_self->sleep_sec < DYNDNS_MIN_SLEEP) ?  DYNDNS_MIN_SLEEP: p_self->sleep_sec;
	(p_self->sleep_sec > DYNDNS_MAX_SLEEP) ?  p_self->sleep_sec = DYNDNS_MAX_SLEEP: 1;

	return RC_OK;
}

static RC_TYPE get_update_period_sec_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (sscanf(p_cmd->argv[current_nr], "%d", &p_self->normal_update_period_sec) != 1)
	{
		return RC_DYNDNS_INVALID_OPTION;
	}

	p_self->normal_update_period_sec = (p_self->normal_update_period_sec < DYNDNS_MIN_SLEEP) ?  DYNDNS_MIN_SLEEP: p_self->normal_update_period_sec;
	(p_self->normal_update_period_sec > DYNDNS_MAX_SLEEP) ?  p_self->normal_update_period_sec = DYNDNS_MAX_SLEEP: 1;

	return RC_OK;
}

static RC_TYPE get_forced_update_period_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (sscanf(p_cmd->argv[current_nr], "%d", &p_self->forced_update_period_sec) != 1)
	{
		return RC_DYNDNS_INVALID_OPTION;
	}

	return RC_OK;
}

static RC_TYPE set_syslog_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *)p_context;

	(void)p_cmd;
	(void)current_nr;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}
	p_self->debug_to_syslog = TRUE;

	return RC_OK;
}

/**
 * Reads the params for change persona. Format:
 * <uid[:gid]>
 */
static RC_TYPE set_change_persona_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	int s, result = RC_OK;
	char *arg, *buf, *p_gid;
	ssize_t bufsize;
	gid_t gid;
	uid_t uid;
	struct passwd pwd, *pwd_res;
	long login_len_max;
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	/* Determine max length of a username */
	login_len_max = sysconf(_SC_LOGIN_NAME_MAX);
	if (login_len_max <= 0)
	{
		login_len_max = 32;
	}

	arg = p_cmd->argv[current_nr];
	{
		char groupname[33] = ""; /* MAX 32 chars + '\0', groupadd(8) */
		char username[login_len_max+1];
		char fmt[65]; /* Conversion string for username */

		uid = getuid();
		gid = getgid();

		p_gid = strstr(arg, ":");
		if (p_gid)
		{
			if ((strlen(p_gid + 1) > 0) &&  /* if something is present after : */
			    sscanf(p_gid + 1, "%32[a-zA-Z-]", groupname) != 1)
			{
				return RC_DYNDNS_INVALID_OPTION;
			}
		}

		snprintf(fmt, sizeof(fmt), "%%%ld[a-zA-Z-]", login_len_max);
		if (sscanf(arg, fmt, username) != 1)
		{
			return RC_DYNDNS_INVALID_OPTION;
		}

		/* Get uid and gid by their names */
		if (strlen(groupname) > 0)
		{
			struct group grp;
			struct group *grp_res;

			bufsize = sysconf(_SC_GETGR_R_SIZE_MAX);
			if (bufsize == -1)
			{
				bufsize = 16384;        /* Should be more than enough */
			}

			buf = malloc(bufsize);
			if (buf == NULL)
			{
				return RC_OUT_OF_MEMORY;
			}

			s = getgrnam_r(groupname, &grp, buf, bufsize, &grp_res);
			if (grp_res != NULL)
			{
				gid = grp.gr_gid;
			}
			else
			{
				if (s == 0)
				{
					logit(LOG_ERR, MODULE_TAG "Cannot find GROUP %s", groupname);
					result = RC_OS_INVALID_GID;
				}
				else
				{
					result = RC_ERROR;
				}
			}
			free(buf);

			if (RC_OK != result)
			{
				return result;
			}
		}

		bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
		if (bufsize == -1)          /* Value was indeterminate */
		{
			bufsize = 16384;        /* Should be more than enough */
		}

		buf = malloc(bufsize);
		if (buf == NULL)
		{
			return RC_OUT_OF_MEMORY;
		}

		s = getpwnam_r(username, &pwd, buf, bufsize, &pwd_res);
		if (pwd_res != NULL)
		{
			uid = pwd.pw_uid;
			if (gid == getgid())
			{
				gid = pwd.pw_gid;
			}
		}
		else
		{
			if (s == 0)
			{
				logit(LOG_ERR, MODULE_TAG "Cannot find USER %s", username);
				result = RC_OS_INVALID_UID;
			}
			else
			{
				result = RC_ERROR;
			}
		}
		free(buf);

		if (RC_OK != result)
		{
			return result;
		}

		p_self->change_persona = TRUE;
		p_self->sys_usr_info.gid = gid;
		p_self->sys_usr_info.uid = uid;
	}

	return RC_OK;
}

static RC_TYPE set_bind_interface(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *)p_context;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	p_self->bind_interface = strdup(p_cmd->argv[current_nr]);

	return RC_OK;
}

static RC_TYPE set_check_interface(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *)p_context;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	p_self->check_interface = strdup(p_cmd->argv[current_nr]);

	return RC_OK;
}

static RC_TYPE set_pidfile(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *)p_context;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	p_self->pidfile = strdup(p_cmd->argv[current_nr]);

	return RC_OK;
}

static RC_TYPE set_cachefile(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *)p_context;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	p_self->cachefile = strdup(p_cmd->argv[current_nr]);

	return RC_OK;
}

RC_TYPE print_version_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *)p_context;

	(void)p_cmd;
	(void)current_nr;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	printf("%s\n", DYNDNS_VERSION_STRING);
	p_self->abort = TRUE;

	return RC_OK;
}

static RC_TYPE get_exec_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *)p_context;

	(void)p_cmd;
	(void)current_nr;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	p_self->external_command = strdup(p_cmd->argv[current_nr]);

	return RC_OK;
}

/**
   Searches the DYNDNS system by the argument.
   Input is like: system@server.name
   system=statdns|custom|dyndns|default
   server name = dyndns.org | freedns.afraid.org
   The result is a pointer in the table of DNS systems.
*/
static RC_TYPE get_dyndns_system_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYNDNS_SYSTEM *p_dns_system = NULL;
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	{
		DYNDNS_SYSTEM_INFO *it = get_dyndns_system_table();
		for (; it != NULL && it->id != LAST_DNS_SYSTEM; ++it)
		{
			if (strcmp(it->system.p_key, p_cmd->argv[current_nr]) == 0)
			{
				p_dns_system = &it->system;
			}
		}
	}

	if (p_dns_system == NULL)
	{
		return RC_DYNDNS_INVALID_OPTION;
	}

	for (curr_info = 0; curr_info < p_self->info_count &&
		     curr_info < DYNDNS_MAX_SERVER_NUMBER &&
		     p_self->info[curr_info].p_dns_system != p_dns_system; curr_info++)
	{
	}
	if (curr_info >= p_self->info_count)
	{
		if (curr_info < DYNDNS_MAX_SERVER_NUMBER)
		{
			p_self->info_count++;
			p_self->info[curr_info].p_dns_system = p_dns_system;
		}
	   	else
			return RC_DYNDNS_BUFFER_TOO_SMALL;
	}

	return RC_OK;
}
static RC_TYPE push_in_buffer(char* p_src, int src_len, char *p_buffer, int* p_act_len, int max_len)
{
	if (*p_act_len + src_len > max_len)
	{
		return RC_FILE_IO_OUT_OF_BUFFER;
	}
	memcpy(p_buffer + *p_act_len,p_src, src_len);
	*p_act_len += src_len;
	return RC_OK;
}

typedef enum
{
	NEW_LINE,
	COMMENT,
	DATA,
	SPACE,
	ESCAPE
} PARSER_STATE;

typedef struct
{
	FILE *p_file;
	PARSER_STATE state;
} OPTION_FILE_PARSER;

static RC_TYPE parser_init(OPTION_FILE_PARSER *p_cfg, FILE *p_file)
{
	memset(p_cfg, 0, sizeof(*p_cfg));
	p_cfg->state = NEW_LINE;
	p_cfg->p_file = p_file;
	return RC_OK;
}

/** Read one single option from file into the given buffer.
    When the first separator is encountered it returns.
    Actions:
    - read chars while not eof
    - skip comments (parts beginning with '#' and ending with '\n')
    - switch to DATA STATE if non space char is encountered
    - assume first name in lines to be a long option name by adding '--' if necesssary
    - add data to buffer
    - do not forget a 0 at the end
    * States:
    * NEW_LINE - wait here until some option. Add '--' if not already there
    * SPACE - between options. Like NEW_LINE but no additions
    * DATA - real data. Stop on space.
    * COMMENT - everything beginning with # until EOLine
    * ESCAPE - everything that is otherwise (incl. spaces). Next char is raw copied.
    */
static RC_TYPE parser_read_option(OPTION_FILE_PARSER *p_cfg, char *p_buffer, int maxlen)
{
	RC_TYPE rc = RC_OK;
	BOOL parse_end = FALSE;
	int count = 0;
	*p_buffer = 0;

	while (!parse_end)
	{
		char ch;
		{
			int n;
			if ((n = fscanf(p_cfg->p_file, "%c", &ch)) < 0)
			{
				if (feof(p_cfg->p_file))
				{
					break;
				}
				rc = RC_FILE_IO_READ_ERROR;
				break;
			}
		}

		switch (p_cfg->state)
		{
			case NEW_LINE:
				if (ch == '\\')
				{
					p_cfg->state = ESCAPE;
					break;
				}
				if (ch == '#') /*comment*/
				{
					p_cfg->state = COMMENT;
					break;
				}
				if (!isspace(ch))
				{
					if (ch != '-')/*add '--' to first word in line*/
					{
						if ((rc = push_in_buffer("--", 2, p_buffer, &count, maxlen)) != RC_OK)
						{
							break;
						}
					}
					if ((rc = push_in_buffer(&ch, 1, p_buffer, &count, maxlen)) != RC_OK)
					{
						break;
					}
					p_cfg->state = DATA;
					break;
				}
				/*skip actual leading  spaces*/
				break;

			case SPACE:
				if (ch == '\\')
				{
					p_cfg->state = ESCAPE;
					break;
				}
				if (ch == '#') /*comment*/
				{
					p_cfg->state = COMMENT;
					break;
				}
				if (ch == '\n' || ch == '\r')
				{
					p_cfg->state = NEW_LINE;
					break;
				}
				if (!isspace(ch))
				{
					if ((rc = push_in_buffer(&ch, 1, p_buffer, &count, maxlen)) != RC_OK)
					{
						break;
					}
					p_cfg->state = DATA;
					break;
				}
				break;

			case COMMENT:
				if (ch == '\n' || ch == '\r')
				{
					p_cfg->state = NEW_LINE;
				}
				/*skip comments*/
				break;

			case DATA:
				if (ch == '\\')
				{
					p_cfg->state = ESCAPE;
					break;
				}
				if (ch == '#')
				{
					p_cfg->state = COMMENT;
					break;
				}
				if (ch == '\n' || ch == '\r')
				{
					p_cfg->state = NEW_LINE;
					parse_end = TRUE;
					break;
				}
				if (isspace(ch))
				{
					p_cfg->state = SPACE;
					parse_end = TRUE;
					break;
				}
				/*actual data*/
				if ((rc = push_in_buffer(&ch, 1, p_buffer, &count, maxlen)) != RC_OK)
				{
					break;
				}
				break;
			case ESCAPE:
				if ((rc = push_in_buffer(&ch, 1, p_buffer, &count, maxlen)) != RC_OK)
				{
					break;
				}
				p_cfg->state = DATA;
				break;

			default:
				rc = RC_CMD_PARSER_INVALID_OPTION;
		}
		if (rc != RC_OK)
		{
			break;
		}
	}
	if (rc == RC_OK)
	{
		char ch = 0;
		rc = push_in_buffer(&ch, 1, p_buffer, &count, maxlen);
	}
	return rc;
}


/**
   This handler reads the data in the passed file name.
   Then appends the words in the table (cutting all spaces) to the existing cmd line options.
   It adds to the CMD_DATA struct.
   Actions:
   - open file
   - read characters and cut spaces away
   - add values one by one to the existing p_cmd data
*/
static RC_TYPE get_options_from_file_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	RC_TYPE rc = RC_OK;
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	FILE *p_file = NULL;
	char *p_tmp_buffer = NULL;
	const int buffer_size = DYNDNS_SERVER_NAME_LENGTH;
	OPTION_FILE_PARSER parser;

	if (!p_self || !p_cmd)
	{
		return RC_INVALID_POINTER;
	}

	do
	{
 		p_tmp_buffer = malloc(buffer_size);
   		if (!p_tmp_buffer)
	 	{
	 		rc = RC_OUT_OF_MEMORY;
	 		break;
	  	}
	  	p_file = fopen(p_cmd->argv[current_nr], "r");
	  	if (!p_file)
	  	{
			logit(LOG_ERR, MODULE_TAG "Cannot open config file %s: %s", p_cmd->argv[current_nr], strerror(errno));
	  		rc = RC_FILE_IO_OPEN_ERROR;
	  		break;
	  	}

		/* Save for later... */
		if (p_self->cfgfile)
			free(p_self->cfgfile);
		p_self->cfgfile = strdup(p_cmd->argv[current_nr]);

		if ((rc = parser_init(&parser, p_file)) != RC_OK)
		{
			break;
		}

		while (!feof(p_file))
		{
			rc = parser_read_option(&parser,p_tmp_buffer, buffer_size);
			if (rc != RC_OK)
			{
				break;
			}

			if (!strlen(p_tmp_buffer))
			{
				break;
			}

			rc = cmd_add_val(p_cmd, p_tmp_buffer);
			if (rc != RC_OK)
			{
				break;
			}
   		}
	}
	while (0);

	if (p_file)
	{
		fclose(p_file);
	}
	if (p_tmp_buffer)
	{
		free(p_tmp_buffer);
	}

	return rc;
}

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
  Note:
  - if no argument is specified tries to call the cmd line parser
  with the default cfg file path.
*/
RC_TYPE get_config_data(DYN_DNS_CLIENT *p_self, int argc, char** argv)
{
	int i;
	RC_TYPE rc = RC_OK;

	do
	{
		/*load default data */
		rc = get_default_config_data(p_self);
		if (rc != RC_OK)
		{
			break;
		}
		/*set up the context pointers */
		{
			CMD_DESCRIPTION_TYPE *it = cmd_options_table;
			while (it->p_option != NULL)
			{
				it->p_handler.p_context = (void*) p_self;
				++it;
			}
		}

		/* in case of no options, assume the default cfg file may be present */
		if (argc == 1)
		{
			char *custom_argv[] = {"", DYNDNS_INPUT_FILE_OPT_STRING, DYNDNS_DEFAULT_CONFIG_FILE};
			int custom_argc = sizeof(custom_argv) / sizeof(char*);

			if (p_self->dbg.level > 1)
			{
				logit(LOG_NOTICE, MODULE_TAG "Using default config file %s", DYNDNS_DEFAULT_CONFIG_FILE);
			}

			if (p_self->cfgfile)
				free(p_self->cfgfile);
			p_self->cfgfile = strdup(DYNDNS_DEFAULT_CONFIG_FILE);
			rc = get_cmd_parse_data(custom_argv, custom_argc, cmd_options_table);
		}
		else
		{
			rc = get_cmd_parse_data(argv, argc, cmd_options_table);
		}

		if (rc != RC_OK || p_self->abort)
		{
			break;
		}

		/* settings that may change due to cmd line options */
		i = 0;
		do
		{
			/*ip server*/
			if (strlen(p_self->info[i].ip_server_name.name) == 0)
			{
				if (sizeof(p_self->info[i].ip_server_name.name) < strlen(p_self->info[i].p_dns_system->p_ip_server_name))
				{
					rc = RC_DYNDNS_BUFFER_TOO_SMALL;
					break;
				}
				strcpy(p_self->info[i].ip_server_name.name, p_self->info[i].p_dns_system->p_ip_server_name);

				if (sizeof(p_self->info[i].ip_server_url) < strlen(p_self->info[i].p_dns_system->p_ip_server_url))
				{
					rc = RC_DYNDNS_BUFFER_TOO_SMALL;
					break;
				}
				strcpy(p_self->info[i].ip_server_url, p_self->info[i].p_dns_system->p_ip_server_url);
			}

			/*dyndns server*/
			if (strlen(p_self->info[i].dyndns_server_name.name) == 0)
			{
				if (sizeof(p_self->info[i].dyndns_server_name.name) < strlen(p_self->info[i].p_dns_system->p_dyndns_server_name))
				{
					rc = RC_DYNDNS_BUFFER_TOO_SMALL;
					break;
				}
				strcpy(p_self->info[i].dyndns_server_name.name, p_self->info[i].p_dns_system->p_dyndns_server_name);

				if (sizeof(p_self->info[i].dyndns_server_url) < strlen(p_self->info[i].p_dns_system->p_dyndns_server_url))
				{
					rc = RC_DYNDNS_BUFFER_TOO_SMALL;
					break;
				}
				strcpy(p_self->info[i].dyndns_server_url, p_self->info[i].p_dns_system->p_dyndns_server_url);
			}
		}
		while(++i < p_self->info_count);

		/* Check if the neccessary params have been provided */
		if ((p_self->info_count == 0) ||
		    (p_self->info[0].alias_count == 0) ||
		    (strlen(p_self->info[0].dyndns_server_name.name) == 0)  ||
		    (strlen(p_self->info[0].ip_server_name.name) == 0))
		{
			rc = RC_DYNDNS_INVALID_OR_MISSING_PARAMETERS;
			break;
		}
	}
	while (0);

	return rc;
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "ellemtel"
 *  c-basic-offset: 8
 * End:
 */
