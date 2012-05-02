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

/*
	Dyn Dns update main implementation file 
	Author: narcis Ilisei
	Date: May 2003

	History:
		- first implemetnation
		- 18 May 2003 : cmd line option reading added - 
		- many options added
		- january 2005 - new format for the config file =Thanks to Jerome Benoit. 
        - january 30 2005 - new parser for config file -
*/
#define MODULE_TAG "CMD_OPTS: "

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
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
static RC_TYPE get_pidfile_name(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE set_silent_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE set_verbose_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_proxy_server_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_options_from_file_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE set_iterations_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE set_syslog_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE set_change_persona_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE print_version_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_exec_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
#ifndef USE_CACHE_FILE
static RC_TYPE get_cache_dir(CMD_DATA *p_cmd, int current_nr, void *p_context);
#else
static RC_TYPE get_cache_file(CMD_DATA *p_cmd, int current_nr, void *p_context);
#endif

static CMD_DESCRIPTION_TYPE cmd_options_table[] = 
{
	{"--help",		0,	{help_handler, NULL},	"help" },
	{"-h",			0,	{help_handler, NULL},	"help" },

	{"--username",	1,	{get_username_handler, NULL},	"your  membername/ hash"},
	{"-u",			1,	{get_username_handler, NULL},	"your  membername / hash"},

	{"--password",	1,	{get_password_handler, NULL},	"your password. Optional."},
	{"-p",			1,	{get_password_handler, NULL},	"your password"},

	{"--alias",		1,	{get_alias_handler, NULL},	"alias host name. this option can appear multiple times." },
	{"-a",			1,	{get_alias_handler, NULL},	"alias host name. this option can appear multiple times." },

	{DYNDNS_INPUT_FILE_OPT_STRING, 1, {get_options_from_file_handler, NULL}, "the file containing [further] inadyn options."
			"The default config file, '" DYNDNS_DEFAULT_CONFIG_FILE "' is used if inadyn is called without any cmd line options." },
	
	{"--ip_server_name",	2,	{get_ip_server_name_handler, NULL},
        "<srv_name[:port] local_url> - local IP is detected by parsing the response after returned by this server and URL. \n"
		"\t\tThe first IP in found in http response is considered 'my IP'. \n"
		"\t\tDefault value: 'checkip.dyndns.org /"},

	{"--dyndns_server_name", 1,	{get_dns_server_name_handler, NULL},	
            "[<NAME>[:port]] \n"
            "\t\tThe server that receives the update DNS request.  \n"
            "\t\tAllows the use of unknown DNS services that accept HTTP updates.\n"  
            "\t\tIf no proxy is wanted, then it is enough to set the dyndns system. The default servers will be taken."},

	{"--dyndns_server_url", 1, {get_dns_server_url_handler, NULL},	
            "<name>\n"
			"\tfull URL relative to DynDNS server root.\n"
			"\tEx: /some_script.php?hostname=\n"},	

	{"--dyndns_system",	1,	{get_dyndns_system_handler, NULL},	
            "[NAME] - optional DYNDNS service type. SHOULD be one of the following: \n"
            "\t\t-For dyndns.org: dyndns@dyndns.org OR statdns@dyndns.org OR custom@dyndns.org.\n"
            "\t\t-For freedns.afraid.org: default@freedns.afraid.org\n"
            "\t\t-For zoneedit.com: default@zoneedit.com\n"
            "\t\t-For no-ip.com: default@no-ip.com\n"
            "\t\t-For easydns.com: default@easydns.com\n"
            "\t\t-For tzo.com: default@tzo.com\n"
            "\t\t-For 3322.org: dyndns@3322.org\n"
            "\t\t-For dnsomatic.com: default@dnsomatic.com\n"
            "\t\t-For tunnelbroker.net: ipv6tb@he.net\n"
			"\t\t-For generic: custom@http_svr_basic_auth\n"
            "\t\tDEFAULT value is intended for default service at dyndns.org (most users): dyndns@dyndns.org"},

    {"--proxy_server", 1, {get_proxy_server_handler, NULL},
            "[NAME[:port]]  - the http proxy server name and port. Default is none."},
	{"--update_period",	1,	{get_update_period_handler, NULL},	
            "how often the IP is checked. The period is in [ms]. Default is about 1 min. Max is 10 days"},
	{"--update_period_sec",	1,	{get_update_period_sec_handler, NULL},	"how often the IP is checked. The period is in [sec]. Default is about 1 min. Max is 10 days"},
	{"--forced_update_period", 1,   {get_forced_update_period_handler, NULL},"how often the IP is updated even if it is not changed. [in sec]"},

	{"--log_file",	1,	{get_logfile_name, NULL},		"log file path abd name"},
	{"--pid_file",	1,	{get_pidfile_name, NULL},		"pid file path abd name"},
	{"--background", 0,	{set_silent_handler, NULL},		"run in background. output to log file or to syslog"},

	{"--verbose",	1,	{set_verbose_handler, NULL},	"set dbg level. 0 to 5"},

	{"--iterations",	1,	{set_iterations_handler, NULL},	"set the number of DNS updates. Default is 0, which means infinity."},
	{"--syslog",	0,	{set_syslog_handler, NULL},	"force logging to syslog . (e.g. /var/log/messages). Works on **NIX systems only."},
	{"--change_persona", 1, {set_change_persona_handler, NULL}, "after init switch to a new user/group. Parameters: <uid[:gid]> to change to. Works on **NIX systems only."},
	{"--version", 0, {print_version_handler, NULL}, "print the version number\n"},
	{"--exec", 1, {get_exec_handler, NULL}, "external command to exec after an IP update. Include the full path."},
#ifndef USE_CACHE_FILE
	{"--cache_dir", 1, {get_cache_dir, NULL}, "cache directory name. (e.g. /tmp/ddns). Defaults to /tmp on **NIX systems."},
#else
	{"--cache_file", 1, {get_cache_file, NULL}, "cache file name. (e.g. /tmp/ddns.cache). Defaults to /tmp/inadyn.cache."},
#endif
	{"--wildcard", 0, {get_wildcard_handler, NULL}, "enable domain wildcarding for dyndns.org, 3322.org, or easydns.com."},
	{NULL,		0,	{0, NULL},	NULL }
};


void print_help_page(void)
{
	printf("\n\n\n"
	"			INADYN Help\n\n"
	"	INADYN is a dynamic DNS client. That is, it maintains the IP address\n"
	"of a host name. It periodically checks whether the IP address of the current machine\n"
	"(the external visible IP address of the machine that runs INADYN) has changed.\n"
    "If yes it performs an update in the dynamic dns server.\n\n");
    printf("Typical usage: \n"	
    "\t-for dyndns.org system: \n"
    "\t\tinadyn -u username -p password -a my.registrated.name \n"
    "\t-for freedns.afraid.org:\n"
    "\t\t inadyn --dyndns_system default@freedns.afraid.org -a my.registrated.name,hash -a anothername,hash2\n"
    "\t\t\t 'hash' is extracted from the grab url batch file that is downloaded from freedns.afraid.org\n\n"
	"Parameters:\n");

	{
		CMD_DESCRIPTION_TYPE *it = cmd_options_table;
		
		while( it->p_option != NULL)
		{
			printf(
				"\t'%s': %s\n\r",
				it->p_option, it->p_description);
			++it;
		}
	}
	printf("\n\n\n");
}

static RC_TYPE help_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
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
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
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
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
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

static RC_TYPE get_pidfile_name(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	p_self->p_pidfilename = (char *)malloc(strlen(p_cmd->argv[current_nr]) + 1);
	if (p_self->p_pidfilename == NULL)
	{
		return  RC_OUT_OF_MEMORY;
	}
	strcpy(p_self->p_pidfilename, p_cmd->argv[current_nr]);
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

/**
    Parses alias,hash.
    Example: blabla.domain.com,hashahashshahah
    Action:
	-search by ',' and replace the ',' with 0
	-read hash and alias
*/
static RC_TYPE get_alias_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
        char *p_hash = NULL;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (p_self->info[curr_info].alias_count >= DYNDNS_MAX_ALIAS_NUMBER)
	{
		return RC_DYNDNS_TOO_MANY_ALIASES;
	}

	/*hash*/
        p_hash = strstr(p_cmd->argv[current_nr],",");
        if (p_hash)
	{
	    if (sizeof(p_self->info[curr_info].alias_info[p_self->info[curr_info].alias_count].hashes) < strlen(p_hash))
	    {
		return RC_DYNDNS_BUFFER_TOO_SMALL;
	    }
	    strcpy(p_self->info[curr_info].alias_info[p_self->info[curr_info].alias_count].hashes.str, p_hash);
	    *p_hash = '\0';
	}


	/*user*/
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

	if (sscanf(p_cmd->argv[current_nr], "%d", &p_self->sleep_sec) != 1)
	{
		return RC_DYNDNS_INVALID_OPTION;
	}
	
	p_self->sleep_sec = (p_self->sleep_sec < DYNDNS_MIN_SLEEP) ?  DYNDNS_MIN_SLEEP: p_self->sleep_sec;
	(p_self->sleep_sec > DYNDNS_MAX_SLEEP) ?  p_self->sleep_sec = DYNDNS_MAX_SLEEP: 1;	

	return RC_OK;
}

static RC_TYPE get_forced_update_period_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (sscanf(p_cmd->argv[current_nr], "%d", &p_self->forced_update_period_sec) != 1 ||
	    sscanf(p_cmd->argv[current_nr], "%d", &p_self->forced_update_period_sec_orig) != 1)
	{
		return RC_DYNDNS_INVALID_OPTION;
	}
	
	return RC_OK;
}

static RC_TYPE set_syslog_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
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
#ifndef _WIN32
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	{
		gid_t gid = getuid();
		uid_t uid = getgid();

		char *p_gid = strstr(p_cmd->argv[current_nr],":");
		if (p_gid)
		{
			if ((strlen(p_gid + 1) > 0) &&  /* if something is present after :*/
				sscanf(p_gid + 1, "%u",&gid) != 1)
			{
				return RC_DYNDNS_INVALID_OPTION;
			}
		}
		if (sscanf(p_cmd->argv[current_nr], "%u",&uid) != 1)
		{
			return RC_DYNDNS_INVALID_OPTION;
		}

		p_self->change_persona = TRUE;
		p_self->sys_usr_info.gid = gid;
		p_self->sys_usr_info.uid = uid;
	}
#endif
	return RC_OK;
}

RC_TYPE print_version_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	DBG_PRINTF((LOG_INFO, "Version: %s\n", DYNDNS_VERSION_STRING));
	p_self->abort = TRUE;
	return RC_OK;
}

static RC_TYPE get_exec_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (sizeof(p_self->external_command) < strlen(p_cmd->argv[current_nr]))
	{
		return  RC_DYNDNS_BUFFER_TOO_SMALL;
	}
	strcpy(p_self->external_command, p_cmd->argv[current_nr]);
	return RC_OK;
}

#ifndef USE_CACHE_FILE
static RC_TYPE get_cache_dir(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (sizeof(p_self->time_cache) < strlen(p_cmd->argv[current_nr]))
	{
		return  RC_DYNDNS_BUFFER_TOO_SMALL;
	}
	sprintf(p_self->ip_cache, "%s/%s", p_cmd->argv[current_nr], DYNDNS_DEFAULT_IP_FILE);
	sprintf(p_self->time_cache, "%s/%s", p_cmd->argv[current_nr], DYNDNS_DEFAULT_TIME_FILE);
	return RC_OK;
}
#else
static RC_TYPE get_cache_file(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (sizeof(p_self->file_cache) < strlen(p_cmd->argv[current_nr]))
	{
		return  RC_DYNDNS_BUFFER_TOO_SMALL;
	}
	sprintf(p_self->file_cache, "%s", p_cmd->argv[current_nr]);
	return RC_OK;
}
#endif

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
    
	while(!parse_end)
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
			DBG_PRINTF((LOG_ERR, "W:" MODULE_TAG "Cannot open cfg file:%s\n", p_cmd->argv[current_nr]));
	  		rc = RC_FILE_IO_OPEN_ERROR;
	  		break;
	  	}

        if ((rc = parser_init(&parser, p_file)) != RC_OK)
        {
            break;
        }

    	while(!feof(p_file))
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
	while(0);
	
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
	FILE *fp;
#ifndef USE_CACHE_FILE
	char cached_time[80];
#endif
	int dif;
	
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
			while( it->p_option != NULL)
			{
				it->p_handler.p_context = (void*) p_self;
				++it;
			}
		}
		
		/* in case of no options, assume the default cfg file may be present */
		if (argc == 1)
		{
			char* custom_argv[] = {"", DYNDNS_INPUT_FILE_OPT_STRING, DYNDNS_DEFAULT_CONFIG_FILE};
			int custom_argc = sizeof(custom_argv) / sizeof(char*);
			if (p_self->dbg.level > 0)
			{
            	DBG_PRINTF((LOG_NOTICE,"I:" MODULE_TAG "Using default config file %s\n", DYNDNS_DEFAULT_CONFIG_FILE));
			}
			rc = get_cmd_parse_data(custom_argv, custom_argc, cmd_options_table);	
		}
		else
		{
			rc = get_cmd_parse_data(argv, argc, cmd_options_table);
		}

		if (rc != RC_OK ||
			p_self->abort)
		{
			break;
		}	

        /*settings that may change due to cmd line options*/
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

		/*check if the neccessary params have been provided*/
		if ( 
			(p_self->info_count == 0) ||
			(p_self->info[0].alias_count == 0) ||
			(strlen(p_self->info[0].dyndns_server_name.name) == 0)  ||
			(strlen(p_self->info[0].ip_server_name.name) == 0)
			)
		{
			rc = RC_DYNDNS_INVALID_OR_MISSING_PARAMETERS;
			break;
		}
		/*forced update*/
		#ifndef USE_CACHE_FILE
		if ((fp=fopen(p_self->time_cache, "r")))
		{
			if (!fgets(cached_time, sizeof (cached_time), fp))
			{
				DBG_PRINTF((LOG_WARNING,"I: Could not read cached time\n"));
			}
			fclose(fp);
			dif = time(NULL) - atoi(cached_time);
			p_self->forced_update_period_sec -= dif;
		}
		#else
		if ((fp=fopen(p_self->file_cache, "r")))
		{
			if (fscanf(fp, "%ld,", &dif) < 1)
			{
				DBG_PRINTF((LOG_WARNING,"I: Could not read cached time\n"));
			}
			fclose(fp);
			dif = time(NULL) - dif;
			p_self->forced_update_period_sec -= dif;
		}
		#endif
		p_self->times_since_last_update = 0;
		p_self->forced_update_times = p_self->forced_update_period_sec / p_self->sleep_sec;

	}
	while(0);

	return rc;
}



