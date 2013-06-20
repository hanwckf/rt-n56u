/* DDNS client updater main implementation file
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

#define MODULE_TAG ""
#include <resolv.h>
#include <stdlib.h>
#include <string.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>

#include "dyndns.h"
#include "debug_if.h"
#include "base64.h"
#include "md5.h"
#include "sha1.h"
#include "get_cmd.h"

#include <nvram/bcmnvram.h>

#define MD5_DIGEST_BYTES (16)
#define SHA1_DIGEST_BYTES (20)

/* To preserve from being reset at SIGHUP */
static int cached_time_since_last_update = 0;
static int cached_num_iterations = 0;

static int get_req_for_dyndns_server(DYN_DNS_CLIENT *this, int infcnt, int alcnt);
static int get_req_for_freedns_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt);
static int get_req_for_generic_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt);
static int get_req_for_zoneedit_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt);
static int get_req_for_easydns_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt);
static int get_req_for_tzo_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt);
static int get_req_for_sitelutions_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt);
static int get_req_for_dnsexit_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt);
static int get_req_for_he_ipv6tb_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt);
static int get_req_for_changeip_server(DYN_DNS_CLIENT *this, int infcnt, int alcnt);
static int get_req_for_asus_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt);

static RC_TYPE is_dyndns_server_rsp_ok(DYN_DNS_CLIENT *p_self, HTTP_TRANSACTION *p_tr, int infcnt);
static RC_TYPE is_freedns_server_rsp_ok(DYN_DNS_CLIENT *p_self, HTTP_TRANSACTION *p_tr, int infcnt);
static RC_TYPE is_generic_server_rsp_ok(DYN_DNS_CLIENT *p_self, HTTP_TRANSACTION *p_tr, int infcnt);
static RC_TYPE is_zoneedit_server_rsp_ok(DYN_DNS_CLIENT *p_self, HTTP_TRANSACTION *p_tr, int infcnt);
static RC_TYPE is_easydns_server_rsp_ok(DYN_DNS_CLIENT *p_self, HTTP_TRANSACTION *p_tr, int infcnt);
static RC_TYPE is_tzo_server_rsp_ok(DYN_DNS_CLIENT *p_self, HTTP_TRANSACTION *p_tr, int infcnt);
static RC_TYPE is_sitelutions_server_rsp_ok(DYN_DNS_CLIENT *p_self, HTTP_TRANSACTION *p_tr, int infcnt);
static RC_TYPE is_dnsexit_server_rsp_ok(DYN_DNS_CLIENT *p_self, HTTP_TRANSACTION *p_tr, int infcnt);
static RC_TYPE is_he_ipv6_server_rsp_ok(DYN_DNS_CLIENT *p_self, HTTP_TRANSACTION *p_tr, int infcnt);
static RC_TYPE is_asus_server_register_rsp_ok( DYN_DNS_CLIENT *p_self, HTTP_TRANSACTION *p_tr, int infcnt);
static RC_TYPE is_asus_server_update_rsp_ok( DYN_DNS_CLIENT *p_self, HTTP_TRANSACTION *p_tr, int infcnt);

DYNDNS_SYSTEM_INFO dns_system_table[] =
{
	{DYNDNS_DEFAULT,
	 {"default@dyndns.org",
	  (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_dyndns_server_rsp_ok,
	  (DNS_SYSTEM_REQUEST_FUNC)get_req_for_dyndns_server,
	  DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
	  "members.dyndns.org", "/nic/update"}},

	{FREEDNS_AFRAID_ORG_DEFAULT,
	 {"default@freedns.afraid.org",
	  (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_freedns_server_rsp_ok,
	  (DNS_SYSTEM_REQUEST_FUNC) get_req_for_freedns_server,
	  DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
	  "freedns.afraid.org", "/dynamic/update.php"}},

	{ZONE_EDIT_DEFAULT,
	 {"default@zoneedit.com",
	  (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_zoneedit_server_rsp_ok,
	  (DNS_SYSTEM_REQUEST_FUNC) get_req_for_zoneedit_server,
	  "dynamic.zoneedit.com", "/checkip.html",
	  "dynamic.zoneedit.com", "/auth/dynamic.html"}},

	{NOIP_DEFAULT,
	 {"default@no-ip.com",
	  (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_dyndns_server_rsp_ok,
	  (DNS_SYSTEM_REQUEST_FUNC) get_req_for_dyndns_server,
	  "ip1.dynupdate.no-ip.com", "/",
	  "dynupdate.no-ip.com", "/nic/update"}},

	{EASYDNS_DEFAULT,
	 {"default@easydns.com",
	  (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_easydns_server_rsp_ok,
	  (DNS_SYSTEM_REQUEST_FUNC) get_req_for_easydns_server,
	  DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
	  "members.easydns.com", "/dyn/dyndns.php"}},
	
	{TZO_DEFAULT,
	 {"default@tzo.com",
	  (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_tzo_server_rsp_ok,
	  (DNS_SYSTEM_REQUEST_FUNC) get_req_for_tzo_server,
	  "echo.tzo.com", "/",
	  "rh.tzo.com", "/webclient/tzoperl.html"}},

	{DYNDNS_3322_DYNAMIC,
	 {"dyndns@3322.org",
	  (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_dyndns_server_rsp_ok,
	  (DNS_SYSTEM_REQUEST_FUNC) get_req_for_dyndns_server,
	  "bliao.com", "/ip.phtml",
	  "members.3322.org", "/dyndns/update"}},

	{SITELUTIONS_DOMAIN,
	 {"default@sitelutions.com",
	  (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_sitelutions_server_rsp_ok,
	  (DNS_SYSTEM_REQUEST_FUNC)get_req_for_sitelutions_server,
	  DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
	  "www.sitelutions.com", "/dnsup"}},

	{DNSOMATIC_DEFAULT,
	 {"default@dnsomatic.com",
	  (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_dyndns_server_rsp_ok,
	  (DNS_SYSTEM_REQUEST_FUNC) get_req_for_dyndns_server,
	  "myip.dnsomatic.com", "/",
	  "updates.dnsomatic.com", "/nic/update"}},

	{DNSEXIT_DEFAULT,
	 {"default@dnsexit.com",
	  (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_dnsexit_server_rsp_ok,
	  (DNS_SYSTEM_REQUEST_FUNC) get_req_for_dnsexit_server,
	  "ip.dnsexit.com", "/",
	  "update.dnsexit.com", "/RemoteUpdate.sv"}},

	{HE_IPV6TB,
	 {"ipv6tb@he.net",
	  (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_he_ipv6_server_rsp_ok,
	  (DNS_SYSTEM_REQUEST_FUNC) get_req_for_he_ipv6tb_server,
	  "checkip.dns.he.net", "/",
	  "ipv4.tunnelbroker.net", "/nic/update"}},

	{HE_DYNDNS,
	 {"dyndns@he.net",
	  (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC) is_dyndns_server_rsp_ok,
	  (DNS_SYSTEM_REQUEST_FUNC) get_req_for_dyndns_server,
	  "checkip.dns.he.net", "/",
	  "dyn.dns.he.net", "/nic/update"}},

	{CHANGEIP_DEFAULT,
	 {"default@changeip.com",
	  (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC) is_dyndns_server_rsp_ok,
	  (DNS_SYSTEM_REQUEST_FUNC) get_req_for_changeip_server,
	  "ip.changeip.com", "/",
	  "nic.changeip.com", "/nic/update"}},

	/* Support for dynsip.org by milkfish, from DD-WRT */
	{DYNSIP_DEFAULT,
	 {"default@dynsip.org",
	  (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_dyndns_server_rsp_ok,
	  (DNS_SYSTEM_REQUEST_FUNC)get_req_for_dyndns_server,
	  DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
	  "dynsip.org", "/nic/update"}},

	{ASUS_REGISTER,
	{"register@asus.com",
	 (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_asus_server_register_rsp_ok,
	  (DNS_SYSTEM_REQUEST_FUNC) get_req_for_asus_server,
	  DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
	  "ns1.asuscomm.com", "/ddns/register.jsp"}},

	{ASUS_UPDATE,
	{"update@asus.com",
	 (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_asus_server_update_rsp_ok,
	  (DNS_SYSTEM_REQUEST_FUNC) get_req_for_asus_server,
	  DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
	  "ns1.asuscomm.com", "/ddns/update.jsp"}},

	{CUSTOM_HTTP_BASIC_AUTH,
	 {"custom@http_srv_basic_auth",
	  (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_generic_server_rsp_ok,
	  (DNS_SYSTEM_REQUEST_FUNC) get_req_for_generic_server,
	  DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
	  "", ""}},

	{LAST_DNS_SYSTEM, {NULL, NULL, NULL, NULL, NULL, NULL, NULL}}
};

static DYNDNS_SYSTEM* get_dns_system_by_id(DYNDNS_SYSTEM_ID id)
{
	DYNDNS_SYSTEM_INFO *it;

	for (it = dns_system_table; it->id != LAST_DNS_SYSTEM; ++it)
	{
		if (it->id == id)
		{
			return &it->system;
		}
	}

	return NULL;
}

DYNDNS_SYSTEM_INFO* get_dyndns_system_table(void)
{
	return dns_system_table;
}

/*************PRIVATE FUNCTIONS ******************/
static RC_TYPE dyn_dns_wait_for_cmd(DYN_DNS_CLIENT *p_self)
{
	int counter;
	DYN_DNS_CMD old_cmd;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	old_cmd = p_self->cmd;
	if (old_cmd != NO_CMD)
	{
		return RC_OK;
	}

	counter = p_self->sleep_sec / p_self->cmd_check_period;
	while (counter --)
	{
		if (p_self->cmd != old_cmd)
		{
			break;
		}
		os_sleep_ms(p_self->cmd_check_period * 1000);
	}

	return RC_OK;
}

static RC_TYPE is_http_status_code_ok(int status)
{
	if (status == 200)
		return RC_OK;
	else if (status >= 500 && status < 600)
		return RC_DYNDNS_RSP_RETRY_LATER;
	else
		return RC_DYNDNS_RSP_NOTOK;
}

static int get_req_for_dyndns_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt)
{
	if (p_self == NULL)
	{
		/* 0 == "No characters written" */
		return 0;
	}
	
	return sprintf(p_self->p_req_buffer, DYNDNS_UPDATE_IP_HTTP_REQUEST,
		       p_self->info[infcnt].dyndns_server_url,
		       p_self->info[infcnt].alias_info[alcnt].names.name,
		       p_self->info[infcnt].my_ip_address.name,
		       p_self->info[infcnt].dyndns_server_name.name,
		       p_self->info[infcnt].credentials.p_enc_usr_passwd_buffer);
}

static int get_req_for_freedns_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt)
{
	RC_TYPE rc = RC_OK, rc2;
	HTTP_CLIENT http_to_dyndns;
	HTTP_TRANSACTION http_tr;
	
	char buffer[256];

	unsigned char digestbuf[SHA1_DIGEST_BYTES];
	char digeststr[SHA1_DIGEST_BYTES*2+1];
	int i;
	
	char *buf, *tmp, *line;
	char host[256], updateurl[256];
	char *hash = NULL;

	if (p_self == NULL)
	{
		/* 0 == "No characters written" */
		return 0;
	}

	// I know it's ugly, http client needs redesign.

	do
	{
		if ((rc = http_client_construct(&http_to_dyndns)) != RC_OK)
			break;
		
		http_client_set_port(&http_to_dyndns, p_self->info[infcnt].dyndns_server_name.port);
		http_client_set_remote_name(&http_to_dyndns, p_self->info[infcnt].dyndns_server_name.name);
		http_client_set_bind_iface(&http_to_dyndns, p_self->bind_interface);
	
		if ((rc = http_client_init(&http_to_dyndns, (p_self->dbg.level > 0) ? "Sending update URL query" : NULL)) != RC_OK)
			break;
	
		snprintf(buffer, sizeof(buffer), "%s|%s",
		         p_self->info[infcnt].credentials.my_username,
		         p_self->info[infcnt].credentials.my_password);
		sha1(buffer, strlen(buffer), digestbuf);
		for (i = 0; i < SHA1_DIGEST_BYTES; i++)
			sprintf(&digeststr[i*2], "%02x", digestbuf[i]);
	
		snprintf(buffer, sizeof(buffer), "/api/?action=getdyndns&sha=%s", digeststr);
	
		http_tr.req_len = sprintf(p_self->p_req_buffer, GENERIC_HTTP_REQUEST,
			buffer, p_self->info[infcnt].dyndns_server_name.name);
		http_tr.p_req = (char*) p_self->p_req_buffer;
		http_tr.p_rsp = (char*) p_self->p_work_buffer;
		http_tr.max_rsp_len = p_self->work_buffer_size - 1; /* Save place for a \0 at the end */
		http_tr.rsp_len = 0;
	
		rc = http_client_transaction(&http_to_dyndns, &http_tr);
		http_tr.p_rsp[http_tr.rsp_len] = 0;
		
		rc2 = http_client_shutdown(&http_to_dyndns);
		
		http_client_destruct(&http_to_dyndns, 1);
		
		if (rc != RC_OK || rc2 != RC_OK)
			break;
		
		if ((rc = is_http_status_code_ok(http_tr.status)) != RC_OK)
			break;
		
		tmp = buf = strdup(http_tr.p_rsp_body);
		
		for (line = strsep(&tmp, "\n"); line; line = strsep(&tmp, "\n")) {
			if (*line &&
			    sscanf(line, "%255[^|\r\n]|%*[^|\r\n]|%255[^|\r\n]", host, updateurl) == 2 &&
			    !strcmp(host, p_self->info[infcnt].alias_info[alcnt].names.name)) {
				hash = strstr(updateurl, "?");
				break;
			}
		}

		free(buf);

		if (!hash)
			rc = RC_DYNDNS_RSP_NOTOK;
	}
	while (0);
	
	if (rc != RC_OK)
	{
		logit(LOG_INFO, MODULE_TAG "Update URL query failed");
		return 0;
	}

	return sprintf(p_self->p_req_buffer, FREEDNS_UPDATE_IP_REQUEST,
		       p_self->info[infcnt].dyndns_server_url,
		       hash,
		       p_self->info[infcnt].my_ip_address.name,
		       p_self->info[infcnt].dyndns_server_name.name);
}


static int get_req_for_generic_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt)
{
	if (p_self == NULL)
	{
		/* 0 == "No characters written" */
		return 0;
	}

	return sprintf(p_self->p_req_buffer, GENERIC_BASIC_AUTH_UPDATE_IP_REQUEST,
		       p_self->info[infcnt].dyndns_server_url,
		       p_self->info[infcnt].alias_info[alcnt].names.name,
		       p_self->info[infcnt].dyndns_server_name.name,
		       p_self->info[infcnt].credentials.p_enc_usr_passwd_buffer);
}

static int get_req_for_zoneedit_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt)
{
	if (p_self == NULL)
	{
		/* 0 == "No characters written" */
		return 0;
	}

	return sprintf(p_self->p_req_buffer, ZONEEDIT_UPDATE_IP_REQUEST,
		       p_self->info[infcnt].dyndns_server_url,
		       p_self->info[infcnt].alias_info[alcnt].names.name,
		       p_self->info[infcnt].my_ip_address.name,
		       p_self->info[infcnt].dyndns_server_name.name,
		       p_self->info[infcnt].credentials.p_enc_usr_passwd_buffer);
}

static int get_req_for_easydns_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt)
{
	if (p_self == NULL)
	{
		/* 0 == "No characters written" */
		return 0;
	}

	return sprintf(p_self->p_req_buffer, EASYDNS_UPDATE_IP_REQUEST,
		       p_self->info[infcnt].dyndns_server_url,
		       p_self->info[infcnt].alias_info[alcnt].names.name,
		       p_self->info[infcnt].my_ip_address.name,
		       p_self->info[infcnt].wildcard ? "ON" : "OFF",
		       p_self->info[infcnt].dyndns_server_name.name,
		       p_self->info[infcnt].credentials.p_enc_usr_passwd_buffer);
}

static int get_req_for_tzo_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt)
{
	if (p_self == NULL)
	{
		/* 0 == "No characters written" */
		return 0;
	}

	return sprintf(p_self->p_req_buffer, TZO_UPDATE_IP_REQUEST,
		       p_self->info[infcnt].dyndns_server_url,
		       p_self->info[infcnt].alias_info[alcnt].names.name,
		       p_self->info[infcnt].credentials.my_username,
		       p_self->info[infcnt].credentials.my_password,
		       p_self->info[infcnt].my_ip_address.name,
		       p_self->info[infcnt].dyndns_server_name.name);
}

static int get_req_for_sitelutions_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt)
{
	if (p_self == NULL)
	{
		/* 0 == "No characters written" */
		return 0;
	}

	return sprintf(p_self->p_req_buffer, SITELUTIONS_UPDATE_IP_HTTP_REQUEST,
		       p_self->info[infcnt].dyndns_server_url,
		       p_self->info[infcnt].credentials.my_username,
		       p_self->info[infcnt].credentials.my_password,
		       p_self->info[infcnt].alias_info[alcnt].names.name,
		       p_self->info[infcnt].my_ip_address.name,
		       p_self->info[infcnt].dyndns_server_name.name);
}

static int get_req_for_dnsexit_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt)
{
	if (p_self == NULL)
	{
		/* 0 == "No characters written" */
		return 0;
	}

	return sprintf(p_self->p_req_buffer, DNSEXIT_UPDATE_IP_HTTP_REQUEST,
		       p_self->info[infcnt].dyndns_server_url,
		       p_self->info[infcnt].credentials.my_username,
		       p_self->info[infcnt].credentials.my_password,
		       p_self->info[infcnt].alias_info[alcnt].names.name,
		       p_self->info[infcnt].my_ip_address.name,
		       p_self->info[infcnt].dyndns_server_name.name);
}

static int get_req_for_he_ipv6tb_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt)
{
	if (p_self == NULL)
	{
		/* 0 == "No characters written" */
		return 0;
	}

	return sprintf(p_self->p_req_buffer, HE_IPV6TB_UPDATE_IP_REQUEST,
		       p_self->info[infcnt].dyndns_server_url,
		       p_self->info[infcnt].credentials.my_username,
		       p_self->info[infcnt].credentials.my_password,
		       p_self->info[infcnt].alias_info[alcnt].names.name,
		       p_self->info[infcnt].my_ip_address.name,
		       p_self->info[infcnt].dyndns_server_name.name);
}

static int get_req_for_changeip_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt)
{
	if (p_self == NULL)
	{
		/* 0 == "No characters written" */
		return 0;
	}
	
	return sprintf(p_self->p_req_buffer, CHANGEIP_UPDATE_IP_HTTP_REQUEST,
		       p_self->info[infcnt].dyndns_server_url,
		       p_self->info[infcnt].alias_info[alcnt].names.name,
		       p_self->info[infcnt].my_ip_address.name,
		       p_self->info[infcnt].dyndns_server_name.name,
		       p_self->info[infcnt].credentials.p_enc_usr_passwd_buffer);
}

/*
	Get the IP address from interface
*/
static RC_TYPE do_ip_check_interface(DYN_DNS_CLIENT *p_self)
{
	struct ifreq ifr;
	in_addr_t new_ip;
	char *new_ip_str;
	int i;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (p_self->check_interface)
	{
		if (p_self->dbg.level > 0)
			logit(LOG_INFO, MODULE_TAG "Checking for IP# change, querying interface %s",
			      p_self->check_interface);

		int sd = socket(PF_INET, SOCK_DGRAM, 0);

		if (sd < 0)
		{
			int code = os_get_socket_error();

			logit(LOG_WARNING, MODULE_TAG "Failed opening network socket: %s", strerror(code));
			return RC_IP_OS_SOCKET_INIT_FAILED;
		}

		memset(&ifr, 0, sizeof(struct ifreq));
		ifr.ifr_addr.sa_family = AF_INET;
		snprintf(ifr.ifr_name, IFNAMSIZ, p_self->check_interface);
		if (ioctl(sd, SIOCGIFADDR, &ifr) != -1)
		{	
			new_ip = ntohl(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr);
			new_ip_str = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
		}
		else
		{
			int code = os_get_socket_error();

			logit(LOG_ERR, MODULE_TAG "Failed reading IP address of interface %s: %s",
			      p_self->check_interface, strerror(code));
			return RC_ERROR;
		}
		close(sd);
	}
	else
	{
		return RC_ERROR;
	}

	if (IN_ZERONET(new_ip) ||
	    IN_LOOPBACK(new_ip) ||
	    IN_LINKLOCAL(new_ip) ||
	    IN_MULTICAST(new_ip) ||
	    IN_EXPERIMENTAL(new_ip))
	{
		logit(LOG_WARNING, MODULE_TAG "Interface %s has invalid IP# %s",
		      p_self->check_interface, new_ip_str);
		return RC_ERROR;
	}

	int anychange = 0;

	for (i = 0; i < p_self->info_count; i++)
	{
		DYNDNS_INFO_TYPE *info = &p_self->info[i];

		info->my_ip_has_changed = strcmp(info->my_ip_address.name, new_ip_str) != 0;
		if (info->my_ip_has_changed)
		{
			anychange++;
			strcpy(info->my_ip_address.name, new_ip_str);
		}
	}

	if (!anychange)
	{
		if (p_self->dbg.level > 0)
			logit(LOG_INFO, MODULE_TAG "No IP# change detected, still at %s", new_ip_str);
	}

	return RC_OK;
}

static int get_req_for_asus_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt)
{
	unsigned char digest[MD5_DIGEST_BYTES];
	char auth[6*2+1+MD5_DIGEST_BYTES*2+1];
	char *p_tmp, *p_auth = auth;
	char *p_b64_buff = NULL;
	size_t dlen = 0;
	int i;

	if (p_self == NULL)
	{
		/* 0 == "No characters written" */
		return 0;
	}

	/* prepare username */
	p_tmp = p_self->info[infcnt].credentials.my_username;
	for (i = 0; i < 6*2; i++) {
		while (*p_tmp && !isxdigit(*p_tmp)) p_tmp++;
		*p_auth++ = *p_tmp ? toupper(*p_tmp++) : '0';
	}

	/* split username and password */
	*p_auth++ = ':';

	/* prepare password, reuse p_req_buffer */
	sprintf(p_self->p_req_buffer, "%s%s",
		p_self->info[infcnt].alias_info[alcnt].names.name,
		p_self->info[infcnt].my_ip_address.name);
	hmac_md5_buffer(p_self->p_req_buffer, strlen(p_self->p_req_buffer),
			p_self->info[infcnt].credentials.my_password,
			strlen(p_self->info[infcnt].credentials.my_password), digest);
	for (i = 0; i < MD5_DIGEST_BYTES; i++)
		p_auth += sprintf(p_auth, "%02X", digest[i]);

	/*encode*/
	base64_encode(NULL, &dlen, auth, strlen(auth));
	p_b64_buff = (char *) malloc(dlen);
	if (p_b64_buff)
		base64_encode(p_b64_buff, &dlen, auth, strlen(auth));

	if (p_self->info[infcnt].credentials.p_enc_usr_passwd_buffer != NULL)
		free(p_self->info[infcnt].credentials.p_enc_usr_passwd_buffer);
	p_self->info[infcnt].credentials.p_enc_usr_passwd_buffer = p_b64_buff;
	p_self->info[infcnt].credentials.encoded =
		(p_self->info[infcnt].credentials.p_enc_usr_passwd_buffer != NULL);
	p_self->info[infcnt].credentials.size =
		strlen(p_self->info[infcnt].credentials.p_enc_usr_passwd_buffer);

	return sprintf(p_self->p_req_buffer, ASUS_PROCESS_MY_IP_REQUEST_FORMAT,
		       p_self->info[infcnt].dyndns_server_url,
		       p_self->info[infcnt].alias_info[alcnt].names.name,
		       p_self->info[infcnt].my_ip_address.name,
		       p_self->info[infcnt].credentials.p_enc_usr_passwd_buffer,
		       p_self->info[infcnt].dyndns_server_name.name);
}

static int get_req_for_ip_server(DYN_DNS_CLIENT *p_self, int infcnt)
{
	if (p_self == NULL)
	{
		/* 0 == "No characters written" */
		return 0;
	}

	return sprintf(p_self->p_req_buffer, DYNDNS_GET_IP_HTTP_REQUEST,
		       p_self->info[infcnt].ip_server_url, p_self->info[infcnt].ip_server_name.name);
}

/*
	Send req to IP server and get the response
*/
static RC_TYPE do_ip_server_transaction(DYN_DNS_CLIENT *p_self, int servernum)
{
	RC_TYPE rc = RC_OK;
	HTTP_CLIENT *p_http;
	HTTP_TRANSACTION *p_tr;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}
	p_http = &p_self->http_to_ip_server[servernum];

	rc = http_client_init(p_http, (p_self->dbg.level > 0) ? "Checking for IP# change" : NULL);
	if (rc != RC_OK)
	{
		return rc;
	}

	/* Prepare request for IP server */
	p_tr = &p_self->http_tr;
	p_tr->req_len = get_req_for_ip_server(p_self, servernum);
	if (p_self->dbg.level > 1)
	{
		logit(LOG_DEBUG, MODULE_TAG "Querying DDNS server for my public IP#:");
		logit(LOG_DEBUG, MODULE_TAG "%s", p_self->p_req_buffer);
	}
	p_tr->p_req = (char*) p_self->p_req_buffer;
	p_tr->p_rsp = (char*) p_self->p_work_buffer;
	p_tr->max_rsp_len = p_self->work_buffer_size - 1; /* Save place for terminating \0 in string. */
	p_tr->rsp_len = 0;

	rc = http_client_transaction(p_http, &p_self->http_tr);
	p_tr->p_rsp[p_tr->rsp_len] = 0;
	
	if (p_tr->status != 200)
		rc = RC_DYNDNS_INVALID_RSP_FROM_IP_SERVER;

	http_client_shutdown(p_http);

	return rc;
}

/*
  Read in 4 integers the ip addr numbers.
  construct then the IP address from those numbers.
  Note:
  it updates the flag: info->'my_ip_has_changed' if the old address was different
*/
static RC_TYPE do_parse_my_ip_address(DYN_DNS_CLIENT *p_self, int servernum)
{
	int ip1 = 0, ip2 = 0, ip3 = 0, ip4 = 0;
	int count, i;
	char *p_ip, *p_current_str;
	BOOL found;
	char new_ip_str[IP_V4_MAX_LENGTH];

	(void)servernum;

	if (p_self == NULL || p_self->http_tr.rsp_len <= 0 || p_self->http_tr.p_rsp == NULL)
	{
		return RC_INVALID_POINTER;
	}

	p_current_str = p_self->http_tr.p_rsp_body;

	found = FALSE;
	do
	{
		/* Try to find first decimal number (begin of IP) */
		p_ip = strpbrk(p_current_str, "0123456789");
		if (p_ip != NULL)
		{
			/* Maybe I found it? */
			count = sscanf(p_ip, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4);
			if (count != 4 ||
			    ip1 <= 0 || ip1 > 255 ||
			    ip2 < 0  || ip2 > 255 ||
			    ip3 < 0  || ip3 > 255 ||
			    ip4 < 0  || ip4 > 255)
			{
				p_current_str = p_ip + 1;
			}
			else
			{
				/* FIRST occurence of a valid IP found */
				found = TRUE;
				break;
			}
		}
	}
	while (p_ip != NULL);

	if (found)
	{
		int anychange = 0;

		for (i = 0; i < p_self->info_count; i++)
		{
			DYNDNS_INFO_TYPE *info = &p_self->info[i];

			sprintf(new_ip_str, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);

			info->my_ip_has_changed = strcmp(info->my_ip_address.name, new_ip_str) != 0;
			if (info->my_ip_has_changed)
			{
				anychange++;
				strcpy(info->my_ip_address.name, new_ip_str);
			}
		}

		if (!anychange)
		{
			if (p_self->dbg.level > 0)
				logit(LOG_INFO, MODULE_TAG "No IP# change detected, still at %s", new_ip_str);
		}

		return RC_OK;
	}

	return RC_DYNDNS_INVALID_RSP_FROM_IP_SERVER;
}

/*
  Updates for every maintained name the property: 'update_required'.
  The property will be checked in another function and updates performed.

  Action:
  Check if my IP address has changed. -> ALL names have to be updated.
  Nothing else.
  Note: In the update function the property will set to false if update was successful.
*/
static RC_TYPE do_check_alias_update_table(DYN_DNS_CLIENT *p_self)
{
	int i, j;

	/* Uses fix test if ip of server 0 has changed.
	 * That should be OK even if changes dyn_dns_update_ip to
	 * iterate over servernum, but not if it's fix set to =! 0 */
	if (p_self->info[0].my_ip_has_changed || p_self->force_addr_update ||
	    (p_self->time_since_last_update > p_self->forced_update_period_sec))
	{
		for (i = 0; i < p_self->info_count; i++)
		{
			DYNDNS_INFO_TYPE *info = &p_self->info[i];

			for (j = 0; j < info->alias_count; j++)
			{
				info->alias_info[j].update_required = TRUE;
				logit(LOG_WARNING, MODULE_TAG "Update needed for alias %s, new IP# %s",
				      info->alias_info[j].names.name, info->my_ip_address.name);
			}
		}
	}

	return RC_OK;
}

/* DynDNS org.specific response validator.
   'good' or 'nochg' are the good answers,
*/
static RC_TYPE is_dyndns_server_rsp_ok(DYN_DNS_CLIENT *p_self, HTTP_TRANSACTION *p_tr, int infnr)
{
	(void)p_self;
	char *p_rsp = p_tr->p_rsp_body;
	(void)infnr;

	RC_TYPE rc;
	if ((rc = is_http_status_code_ok(p_tr->status)) != RC_OK)
		return rc;

	if (strstr(p_rsp, "good") != NULL ||
		strstr(p_rsp, "nochg") != NULL)
		return RC_OK;
	else if (strstr(p_rsp, "dnserr") != NULL ||
		strstr(p_rsp, "911") != NULL)
		return RC_DYNDNS_RSP_RETRY_LATER;
	else
		return RC_DYNDNS_RSP_NOTOK;
}

/* Freedns afraid.org.specific response validator.
   ok blabla and n.n.n.n
    fail blabla and n.n.n.n
    are the good answers. We search our own IP address in response and that's enough.
*/
static RC_TYPE is_freedns_server_rsp_ok(DYN_DNS_CLIENT *p_self, HTTP_TRANSACTION *p_tr, int infnr)
{
	char *p_rsp = p_tr->p_rsp_body;

	RC_TYPE rc;
	if ((rc = is_http_status_code_ok(p_tr->status)) != RC_OK)
		return rc;

	if (strstr(p_rsp, p_self->info[infnr].my_ip_address.name) != NULL)
		return RC_OK;
	else
		return RC_DYNDNS_RSP_NOTOK;
}

/** generic http dns server ok parser
    parses a given string. If found is ok,
    Example : 'SUCCESS CODE='
*/
static RC_TYPE is_generic_server_rsp_ok(DYN_DNS_CLIENT *p_self, HTTP_TRANSACTION *p_tr, int infnr)
{
	(void)p_self;
	char *p_rsp = p_tr->p_rsp_body;
	(void)infnr;

	RC_TYPE rc;
	if ((rc = is_http_status_code_ok(p_tr->status)) != RC_OK)
		return rc;

	if (strstr(p_rsp, "OK") != NULL)
		return RC_OK;
	else
		return RC_DYNDNS_RSP_NOTOK;
}

/**
   the OK codes are:
   CODE=200, 201
   CODE=707, for duplicated updates
*/
static RC_TYPE is_zoneedit_server_rsp_ok(DYN_DNS_CLIENT *p_self, HTTP_TRANSACTION *p_tr, int infnr)
{
	(void)p_self;
	(void)infnr;

	RC_TYPE rc;
	if ((rc = is_http_status_code_ok(p_tr->status)) != RC_OK)
		return rc;

	int code = -1;
	sscanf(p_tr->p_rsp_body, "%*s CODE=\"%d\" ", &code);
	
	switch (code) {
		case 200:
		case 201:
		case 707:
			// is 707 really ok??
			return RC_OK;
		default:
			return RC_DYNDNS_RSP_NOTOK;
	}
}

/**
   NOERROR is the OK code here
*/
static RC_TYPE is_easydns_server_rsp_ok(DYN_DNS_CLIENT *p_self, HTTP_TRANSACTION *p_tr, int infnr)
{
	(void)p_self;
	char *p_rsp = p_tr->p_rsp_body;
	(void)infnr;

	RC_TYPE rc;
	if ((rc = is_http_status_code_ok(p_tr->status)) != RC_OK)
		return rc;

	if (strstr(p_rsp, "NOERROR") != NULL)
		return RC_OK;
	else if (strstr(p_rsp, "TOOSOON") != NULL)
		return RC_DYNDNS_RSP_RETRY_LATER;
	else
		return RC_DYNDNS_RSP_NOTOK;
}

/* TZO specific response validator. */
static RC_TYPE is_tzo_server_rsp_ok(DYN_DNS_CLIENT *p_self, HTTP_TRANSACTION *p_tr, int infnr)
{
	(void)p_self;
	(void)infnr;

	RC_TYPE rc;
	if ((rc = is_http_status_code_ok(p_tr->status)) != RC_OK)
		return rc;

	int code = -1;
	sscanf(p_tr->p_rsp_body, "%d ", &code);

	switch (code) {
		case 200:
		case 304:
			return RC_OK;
		case 414:
		case 500:
			return RC_DYNDNS_RSP_RETRY_LATER;
		default:
			return RC_DYNDNS_RSP_NOTOK;
	}
}

static RC_TYPE is_sitelutions_server_rsp_ok(DYN_DNS_CLIENT *p_self, HTTP_TRANSACTION *p_tr, int infnr)
{
	(void)p_self;
	char *p_rsp = p_tr->p_rsp_body;
	(void)infnr;
	
	RC_TYPE rc;
	if ((rc = is_http_status_code_ok(p_tr->status)) != RC_OK)
		return rc;

	if (strstr(p_rsp, "success") != NULL)
		return RC_OK;
	else if (strstr(p_rsp, "dberror") != NULL)
		return RC_DYNDNS_RSP_RETRY_LATER;
	else
		return RC_DYNDNS_RSP_NOTOK;
}

static RC_TYPE is_dnsexit_server_rsp_ok(DYN_DNS_CLIENT *p_self, HTTP_TRANSACTION *p_tr, int infnr)
{
	(void)p_self;
	(void)infnr;

	RC_TYPE rc;
	if ((rc = is_http_status_code_ok(p_tr->status)) != RC_OK)
		return rc;

	int code = -1;
	char *tmp;
	if ((tmp = strstr(p_tr->p_rsp_body, "\n")) != NULL)
		sscanf(++tmp, "%d=", &code);
	
	switch (code) {
		case 0:
		case 1:
			return RC_OK;
		case 4:
		case 11:
			return RC_DYNDNS_RSP_RETRY_LATER;
		default:
			return RC_DYNDNS_RSP_NOTOK;
	}
}

/* HE ipv6 tunnelbroker specific response validator.
   own IP address and 'already in use' are the good answers.
*/
static RC_TYPE is_he_ipv6_server_rsp_ok(DYN_DNS_CLIENT *p_self, HTTP_TRANSACTION *p_tr, int infnr)
{
	char *p_rsp = p_tr->p_rsp_body;

	RC_TYPE rc;
	if ((rc = is_http_status_code_ok(p_tr->status)) != RC_OK)
		return rc;

	if (strstr(p_rsp, p_self->info[infnr].my_ip_address.name) != NULL ||
		strstr(p_rsp, "already") != NULL)
//		strstr(p_rsp, "-ERROR: This tunnel is already associated with this IP address.") != NULL)
		return RC_OK;
	else
		return RC_DYNDNS_RSP_NOTOK;
}

static RC_TYPE is_asus_server_register_rsp_ok(DYN_DNS_CLIENT *p_self, HTTP_TRANSACTION *p_tr, int infnr)
{
	char *p, *ret_key, ret_buf[64], domain[256] = "";

	(void)p_self;
	char *p_rsp = p_tr->p_rsp;
	(void)infnr;

	ret_key = "ddns_return_code";

	if ((p = strchr(p_rsp, '|')) && (p = strchr(++p, '|')))
		sscanf(p, "|%255[^|\r\n]", domain);

	if (p_tr->status >= 500 && p_tr->status < 600) {
		nvram_set(ret_key, "unknown_error");
		return RC_DYNDNS_RSP_RETRY_LATER;
	}

	snprintf(ret_buf, sizeof(ret_buf), "%s,%d", "register", p_tr->status);
	nvram_set(ret_key, ret_buf);

	switch (p_tr->status) {
	case 200:		/* registration success */
	case 220:		/* registration same domain success*/
		return RC_OK;
	case 203:		/* registration failed */
		logit(LOG_WARNING, MODULE_TAG "Domain already in use, suggested domain '%s'", domain);
		return RC_DYNDNS_RSP_NOTOK;
	case 230:		/* registration new domain success */
		logit(LOG_WARNING, MODULE_TAG "Registration success, previous domain '%s'", domain);
		return RC_OK;
	case 233:		/* registration failed */
		logit(LOG_WARNING, MODULE_TAG "Domain already in use, current domain '%s'", domain);
		return RC_DYNDNS_RSP_NOTOK;
	case 297:		/* invalid hostname */
	case 298:		/* invalid domain name */
	case 299:		/* invalid ip format */
	case 401:		/* authentication failure */
	case 407:		/* proxy authentication required */
		return RC_DYNDNS_RSP_NOTOK;
	}

	if (p_tr->status < 500)		/* shutdown */
		nvram_set(ret_key, "time_out");

	return RC_DYNDNS_RSP_NOTOK;
}

static RC_TYPE is_asus_server_update_rsp_ok(DYN_DNS_CLIENT *p_self, HTTP_TRANSACTION *p_tr, int infnr)
{
	(void)p_self;
	char *p_rsp = p_tr->p_rsp;
	(void)infnr;

	char *p, domain[256] = "";
	if ((p = strchr(p_rsp, '|')) && (p = strchr(++p, '|')))
		sscanf(p, "|%255[^|\r\n]", domain);

	switch (p_tr->status) {
		case 200:		/* update success */
		case 220:		/* update same domain success -- unused?? */
			return RC_OK;
		case 203:		/* update failed */
			logit(LOG_WARNING, MODULE_TAG "Domain already in use, suggested domain '%s'", domain);
			return RC_DYNDNS_RSP_NOTOK;
		case 233:		/* update failed */
			logit(LOG_WARNING, MODULE_TAG "Domain already in use, current domain '%s'", domain);
			return RC_DYNDNS_RSP_NOTOK;
		case 297:		/* invalid hostname */
		case 298:		/* invalid domain name */
		case 299:		/* invalid ip format */
		case 401:		/* authentication failure */
		case 407:		/* proxy authentication required */
			return RC_DYNDNS_RSP_NOTOK;
	}

	if (p_tr->status >= 500 && p_tr->status < 600)
		return RC_DYNDNS_RSP_RETRY_LATER;

	return RC_DYNDNS_RSP_NOTOK;
}

static RC_TYPE do_update_alias_table(DYN_DNS_CLIENT *p_self)
{
	int i, j;
	RC_TYPE rc = RC_OK, rc2;
	HTTP_TRANSACTION http_tr;
	int anychange = 0;

	for (i = 0; i < p_self->info_count; i++)
	{
		DYNDNS_INFO_TYPE *info = &p_self->info[i];

		for (j = 0; j < info->alias_count; j++)
		{
			if (info->alias_info[j].update_required != TRUE)
			{
				continue;
			}

			rc = http_client_init(&p_self->http_to_dyndns[i], "Sending IP# update to DDNS server");
			if (rc != RC_OK)
			{
				break;
			}

			/* Build dyndns transaction */
			http_tr.req_len = info->p_dns_system->p_dns_update_req_func(
				(struct _DYN_DNS_CLIENT*) p_self, i, j);
			http_tr.p_req = (char*) p_self->p_req_buffer;
			http_tr.p_rsp = (char*) p_self->p_work_buffer;
			http_tr.max_rsp_len = p_self->work_buffer_size - 1; /* Save place for a \0 at the end */
			http_tr.rsp_len = 0;

			rc = http_client_transaction(&p_self->http_to_dyndns[i], &http_tr);
			http_tr.p_rsp[http_tr.rsp_len] = 0;

			if (p_self->dbg.level > 1)
			{
				p_self->p_req_buffer[http_tr.req_len] = 0;
				logit(LOG_DEBUG, MODULE_TAG "Sending alias table update to DDNS server:");
				logit(LOG_DEBUG, MODULE_TAG "%s", p_self->p_req_buffer);
			}

			if (rc == RC_OK)
			{
				rc = info->p_dns_system->p_rsp_ok_func((struct _DYN_DNS_CLIENT*)p_self,
										&http_tr, i);
				if (rc == RC_OK)
				{
					info->alias_info[j].update_required = FALSE;
					logit(LOG_INFO, MODULE_TAG "Successful alias table update for %s => new IP# %s",
					      info->alias_info[j].names.name, info->my_ip_address.name);
					p_self->time_since_last_update = 0;
					anychange++; /* Adjust forced update period on success */
				}
				else
				{
					logit(LOG_WARNING, MODULE_TAG "%s error in DDNS server response:",
						  rc == RC_DYNDNS_RSP_RETRY_LATER ? "Temporary" : "Fatal");
					logit(LOG_WARNING, MODULE_TAG "[%d %s] %s",
					      http_tr.status, http_tr.status_desc,
					      http_tr.p_rsp_body != http_tr.p_rsp ? http_tr.p_rsp_body : "");
				}

				if (p_self->dbg.level > 1)
				{
					logit(LOG_DEBUG, MODULE_TAG "DDNS server response:");
					logit(LOG_DEBUG, MODULE_TAG "%s", http_tr.p_rsp);
				}
			}

			rc2 = http_client_shutdown(&p_self->http_to_dyndns[i]);
			if (rc == RC_OK)
			{
				/* Only overwrite rc with of http_client_shutdown() rc if previous call, in
				 * e.g., http_client_transaction() or the p_rsp_ok_func() callback was OK. */
				rc = rc2;
			}
			if (rc != RC_OK)
			{
				break;
			}
			os_sleep_ms(1000);
		}
	}

	/* Successful change or when cache file does not yet exist! */
	if (anychange || access(p_self->cachefile, F_OK))
	{
		FILE *fp;

		/* Update cache with new IP */
		fp = fopen(p_self->cachefile, "w"); 
		if (fp)
		{
			fprintf(fp, "%s", p_self->info[0].my_ip_address.name);
			fclose(fp);
		}

		if (anychange && p_self->external_command)
		{
			/* Run external command hook on update. */
			os_shell_execute(p_self->external_command);
		}
	}

	return rc;
}


RC_TYPE get_default_config_data(DYN_DNS_CLIENT *p_self)
{
	RC_TYPE rc = RC_OK;
	int i;

	do
	{
		p_self->info[0].p_dns_system = get_dns_system_by_id(DYNDNS_DEFAULT_DNS_SYSTEM);
		if (p_self->info[0].p_dns_system == NULL)
		{
			rc = RC_DYNDNS_INVALID_DNS_SYSTEM_DEFAULT;
			break;
		}

		/* forced update period */
		p_self->forced_update_period_sec = DYNDNS_FORCED_UPDATE_PERIOD;

		/* non-fatal error update period */
		p_self->error_update_period_sec = DYNDNS_ERROR_UPDATE_PERIOD;

		/* normal update period */
		p_self->normal_update_period_sec = DYNDNS_DEFAULT_SLEEP;
		p_self->sleep_sec = DYNDNS_DEFAULT_SLEEP;

		/* Domain wildcarding disabled by default */
		for (i = 0; i < DYNDNS_MAX_SERVER_NUMBER; i++)
			p_self->info[i].wildcard = FALSE;

		/* pidfile */
		p_self->pidfile = strdup(DYNDNS_DEFAULT_PIDFILE);

		/* cachefile */
		p_self->cachefile = strdup(DYNDNS_DEFAULT_CACHE_FILE);
	}
	while (0);

	return rc;
}


static RC_TYPE get_encoded_user_passwd(DYN_DNS_CLIENT *p_self)
{
	RC_TYPE rc = RC_OK;
	const char* format = "%s:%s";
	char *p_tmp_buff = NULL;
	int size, actual_len;
	int i = 0;
	char *p_b64_buff = NULL;
	size_t dlen = 0;

	do
	{
		DYNDNS_INFO_TYPE *info = &p_self->info[i];

		size = strlen(info->credentials.my_password) +
			strlen(info->credentials.my_username) +
			strlen(format) + 1;

		p_tmp_buff = (char *) malloc(size);
		if (p_tmp_buff == NULL)
		{
			rc = RC_OUT_OF_MEMORY;
			break;
		}

		actual_len = sprintf(p_tmp_buff, format,
				     info->credentials.my_username,
				     info->credentials.my_password);
		if (actual_len >= size)
		{
			rc = RC_OUT_BUFFER_OVERFLOW;
			break;
		}

		/*encode*/

		base64_encode(NULL, &dlen, p_tmp_buff, strlen(p_tmp_buff));
		p_b64_buff = (char *) malloc(dlen);
		if (p_b64_buff == NULL)
		{
			rc = RC_OUT_OF_MEMORY;
			break;
		}
		base64_encode(p_b64_buff, &dlen, p_tmp_buff, strlen(p_tmp_buff));
		
		info->credentials.p_enc_usr_passwd_buffer = p_b64_buff;
		info->credentials.encoded =
			(info->credentials.p_enc_usr_passwd_buffer != NULL);
		info->credentials.size = strlen(info->credentials.p_enc_usr_passwd_buffer);

		free(p_tmp_buff);
		p_tmp_buff = NULL;
	}
	while (++i < p_self->info_count);

	free(p_tmp_buff);
	p_tmp_buff = NULL;

	return rc;
}

void dyn_dns_print_hello(void)
{
	logit(LOG_INFO, "%s", DYNDNS_VERSION_STRING);
}

/**
   basic resource allocations for the dyn_dns object
*/
RC_TYPE dyn_dns_construct(DYN_DNS_CLIENT **pp_self)
{
	RC_TYPE rc;
	DYN_DNS_CLIENT *p_self;
	BOOL http_to_dyndns_constructed = FALSE;
	BOOL http_to_ip_constructed = FALSE;
	int i;

	if (pp_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	*pp_self = (DYN_DNS_CLIENT *) malloc(sizeof(DYN_DNS_CLIENT));
	if (*pp_self == NULL)
	{
		return RC_OUT_OF_MEMORY;
	}

	do
	{
		p_self = *pp_self;
		memset(p_self, 0, sizeof(DYN_DNS_CLIENT));

		/*alloc space for http_to_ip_server data*/
		p_self->work_buffer_size = DYNDNS_HTTP_RESPONSE_BUFFER_SIZE;
		p_self->p_work_buffer = (char*) malloc(p_self->work_buffer_size);
		if (p_self->p_work_buffer == NULL)
		{
			rc = RC_OUT_OF_MEMORY;
			break;
   		}

		/*alloc space for request data*/
		p_self->req_buffer_size = DYNDNS_HTTP_REQUEST_BUFFER_SIZE;
		p_self->p_req_buffer = (char*) malloc(p_self->req_buffer_size);
		if (p_self->p_req_buffer == NULL)
		{
			rc = RC_OUT_OF_MEMORY;
			break;
		}

		i = 0;
		while (i < DYNDNS_MAX_SERVER_NUMBER)
		{
			rc = http_client_construct(&p_self->http_to_ip_server[i++]);
			if (rc != RC_OK)
			{
				rc = RC_OUT_OF_MEMORY;
				break;
			}
		}
		http_to_ip_constructed = TRUE;

		i = 0;
		while (i < DYNDNS_MAX_SERVER_NUMBER)
		{
			rc = http_client_construct(&p_self->http_to_dyndns[i++]);
			if (rc != RC_OK)
			{
				rc = RC_OUT_OF_MEMORY;
				break;
			}
		}
		http_to_dyndns_constructed = TRUE;

		(p_self)->cmd = NO_CMD;
		(p_self)->normal_update_period_sec = DYNDNS_DEFAULT_SLEEP;
		(p_self)->sleep_sec = DYNDNS_DEFAULT_SLEEP;
		(p_self)->total_iterations = DYNDNS_DEFAULT_ITERATIONS;
		(p_self)->initialized = FALSE;

		i = 0;
		while (i < DYNDNS_MAX_SERVER_NUMBER)
		{
			p_self->info[i++].credentials.p_enc_usr_passwd_buffer = NULL;
		}
	}
	while (0);

	if (rc != RC_OK)
	{

		free(*pp_self);

		free(p_self->p_work_buffer);

		free (p_self->p_work_buffer);

		if (http_to_dyndns_constructed)
		{
			http_client_destruct(p_self->http_to_dyndns, DYNDNS_MAX_SERVER_NUMBER);
		}
		if (http_to_ip_constructed)
		{
			http_client_destruct(p_self->http_to_ip_server, DYNDNS_MAX_SERVER_NUMBER);
		}
	}

	return RC_OK;
}


/**
   Resource free.
*/
RC_TYPE dyn_dns_destruct(DYN_DNS_CLIENT *p_self)
{
	int i;
	RC_TYPE rc;

	if (p_self == NULL)
	{
		return RC_OK;
	}

	if (p_self->initialized == TRUE)
	{
		dyn_dns_shutdown(p_self);
	}

	rc = http_client_destruct(p_self->http_to_ip_server, DYNDNS_MAX_SERVER_NUMBER);
	if (rc != RC_OK)
	{
		/* XXX */
	}

	rc = http_client_destruct(p_self->http_to_dyndns, DYNDNS_MAX_SERVER_NUMBER);
	if (rc != RC_OK)
	{
		/* XXX */
	}

	free(p_self->p_work_buffer);
	p_self->p_work_buffer = NULL;

	free(p_self->p_req_buffer);
	p_self->p_req_buffer = NULL;

	i = 0;
	while (i < DYNDNS_MAX_SERVER_NUMBER)
	{
		DYNDNS_INFO_TYPE *info = &p_self->info[i];

		free(info->credentials.p_enc_usr_passwd_buffer);
		info->credentials.p_enc_usr_passwd_buffer = NULL;

		i++;
	}

	free(p_self->cfgfile);
	p_self->cfgfile = NULL;

	free(p_self->pidfile);
	p_self->pidfile = NULL;

	free(p_self->cachefile);
	p_self->cachefile = NULL;

	free(p_self->external_command);
	p_self->external_command = NULL;

	free(p_self->bind_interface);
	p_self->bind_interface = NULL;

	free(p_self->check_interface);
	p_self->check_interface = NULL;

	/* Save old value, if restarted by SIGHUP */
	cached_time_since_last_update = p_self->time_since_last_update;
	cached_num_iterations = p_self->num_iterations;

	free(p_self);
	p_self = NULL;

	return RC_OK;
}

/**
   Sets up the object.
   - sets the IPs of the DYN DNS server
   - if proxy server is set use it!
   - ...
*/
RC_TYPE dyn_dns_init(DYN_DNS_CLIENT *p_self)
{
	int i = 0;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (p_self->initialized == TRUE)
	{
		return RC_OK;
	}

	p_self->abort_on_network_errors = FALSE;
	p_self->force_addr_update = (p_self->total_iterations == 1) ? TRUE : FALSE;

	do
	{
		DYNDNS_INFO_TYPE *info = &p_self->info[i];

		if (strlen(info->proxy_server_name.name) > 0)
		{
			http_client_set_port(&p_self->http_to_ip_server[i], info->proxy_server_name.port);
			http_client_set_remote_name(&p_self->http_to_ip_server[i], info->proxy_server_name.name);

			http_client_set_port(&p_self->http_to_dyndns[i], info->proxy_server_name.port);
			http_client_set_remote_name(&p_self->http_to_dyndns[i], info->proxy_server_name.name);
		}
		else
		{
			http_client_set_port(&p_self->http_to_ip_server[i], info->ip_server_name.port);
			http_client_set_remote_name(&p_self->http_to_ip_server[i], info->ip_server_name.name);

			http_client_set_port(&p_self->http_to_dyndns[i], info->dyndns_server_name.port);
			http_client_set_remote_name(&p_self->http_to_dyndns[i], info->dyndns_server_name.name);
		}

		http_client_set_bind_iface(&p_self->http_to_dyndns[i], p_self->bind_interface);
		http_client_set_bind_iface(&p_self->http_to_ip_server[i], p_self->bind_interface);
	}
	while (++i < p_self->info_count);

	p_self->cmd = NO_CMD;
	if (p_self->cmd_check_period == 0)
	{
		p_self->cmd_check_period = DYNDNS_DEFAULT_CMD_CHECK_PERIOD;
	}

	/* Reset to old value, if restarted by SIGHUP */
	p_self->time_since_last_update = cached_time_since_last_update;
	p_self->num_iterations = cached_num_iterations;

	p_self->initialized = TRUE;

	return RC_OK;
}

/**
   Disconnect and some other clean up.
*/
RC_TYPE dyn_dns_shutdown(DYN_DNS_CLIENT *p_self)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (p_self->initialized == FALSE)
	{
		return RC_OK;
	}

	return RC_OK;
}

/** the real action:
    - increment the forced update times counter
    - detect current IP
    - connect to an HTTP server
    - parse the response for IP addr

    - for all the names that have to be maintained
    - get the current DYN DNS address from DYN DNS server
    - compare and update if neccessary
*/
RC_TYPE dyn_dns_update_ip(DYN_DNS_CLIENT *p_self)
{
	int servernum = 0; /* server to use for requesting IP */
		     /*use server 0 by default, should be always exist */
	RC_TYPE rc;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	do
	{
		if (p_self->check_interface)
		{
			rc = do_ip_check_interface(p_self);
			if (rc != RC_OK)
			{
				break;
			}
		}
		else
		{
			/* Ask IP server something so he will respond and give me my IP */
			rc = do_ip_server_transaction(p_self, servernum);
			if (rc != RC_OK)
			{
				break;
			}
			if (p_self->dbg.level > 1)
			{
				logit(LOG_DEBUG, MODULE_TAG "IP server response:");
				logit(LOG_DEBUG, MODULE_TAG "%s", p_self->p_work_buffer);
			}
			
			/* Extract our IP, check if different than previous one */
			rc = do_parse_my_ip_address(p_self, servernum);
			if (rc != RC_OK)
			{
				break;
			}
			
			if (p_self->dbg.level > 1)
			{
				logit(LOG_INFO, MODULE_TAG "Current public IP# %s", p_self->info[servernum].my_ip_address.name);
			}
		}
		/* Step through aliases list, resolve them and check if they point to my IP */
		rc = do_check_alias_update_table(p_self);
		if (rc != RC_OK)
		{
			break;
		}
		
		/* Update IPs marked as not identical with my IP */
		rc = do_update_alias_table(p_self);
		if (rc != RC_OK)
		{
			break;
		}
	}
	while (0);

	return rc;
}


/** MAIN - Dyn DNS update entry point
    Actions:
    - read the configuration options
    - perform various init actions as specified in the options
    - create and init dyn_dns object.
    - launch the IP update action loop
*/
int dyn_dns_main(DYN_DNS_CLIENT *p_dyndns, int argc, char* argv[])
{
	FILE *fp;
	RC_TYPE rc = RC_OK;
	int i, s;
	int iterations_err = 0;
	char name[DYNDNS_SERVER_NAME_LENGTH];

	if (p_dyndns == NULL)
	{
		return RC_INVALID_POINTER;
	}

	/* Create pid and cache file repository. */
	mkdir(DYNDNS_RUNTIME_DATA_DIR, 0755);

	/* read cmd line options and set object properties */
	rc = get_config_data(p_dyndns, argc, argv);
	if (rc != RC_OK || p_dyndns->abort)
	{
		return rc;
	}

	if (p_dyndns->change_persona)
	{
		OS_USER_INFO os_usr_info;

		memset(&os_usr_info, 0, sizeof(os_usr_info));
		os_usr_info.gid = p_dyndns->sys_usr_info.gid;
		os_usr_info.uid = p_dyndns->sys_usr_info.uid;
		rc = os_change_persona(&os_usr_info);
		if (rc != RC_OK)
		{
			return rc;
		}
	}

	/* if logfile provided, redirect output to log file */
	if (strlen(p_dyndns->dbg.p_logfilename) != 0)
	{
		rc = os_open_dbg_output(DBG_FILE_LOG, "", p_dyndns->dbg.p_logfilename);
		if (rc != RC_OK)
		{
			return rc;
		}
	}

	if (p_dyndns->debug_to_syslog == TRUE || (p_dyndns->run_in_background == TRUE))
	{
		if (get_dbg_dest() == DBG_STD_LOG) /* avoid file and syslog output */
		{
			rc = os_open_dbg_output(DBG_SYS_LOG, "inadyn", NULL);
			if (rc != RC_OK)
			{
				return rc;
			}
		}
	}

	/* if silent required, close console window */
	if (p_dyndns->run_in_background == TRUE)
	{
		rc = close_console_window();
		if (rc != RC_OK)
		{
			return rc;
		}
		if (get_dbg_dest() == DBG_SYS_LOG)
		{
			fclose(stdout);
		}
	}

	/* Create files with permissions 0644 */
	umask(S_IWGRP | S_IWOTH);

	/* write pid file */
	fp = fopen(p_dyndns->pidfile, "w");
	if (!fp)
	{
		logit(LOG_ERR, MODULE_TAG "Failed opening pidfile %s for writing: %s", p_dyndns->pidfile, strerror(errno));
		return RC_ERROR;
	}
	fprintf(fp, "%u", getpid());
	fclose(fp);

	dyn_dns_print_hello();

	/* At boot, or when restarting inadyn at runtime, the memory struct holding
	 * our current IP# is empty.  We want to avoid unnecessary updates of our
	 * DDNS server record, since we might get locked out for abuse, so we "seed"
	 * each of the DDNS records of our struct with the cached IP# from our cache
	 * file, or from a regular DNS query. */
	fp = fopen(p_dyndns->cachefile, "r");
	if (!fp)
	{
		struct addrinfo hints;
		struct addrinfo *result;

		/* Clear DNS cache before querying for the IP below. */
		res_init();

		/* Try a DNS lookup of our last known IP#. */
		for (i = 0; i < p_dyndns->info_count; i++)
		{
			if (p_dyndns->info[i].alias_count &&
			    /* exception for tunnelbroker.net - no name to lookup */
			    strcmp(p_dyndns->info[i].p_dns_system->p_key, "ipv6tb@he.net"))
			{
				/* DNS Lookup */
				memset(&hints, 0, sizeof(struct addrinfo));
				hints.ai_family = AF_INET;    /* IPv4 */
				hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
				hints.ai_flags = 0;
				hints.ai_protocol = 0;          /* Any protocol */

				if (!(s = getaddrinfo(p_dyndns->info[i].alias_info[0].names.name, NULL, &hints, &result)))
				{
					/* DNS reply for alias found, convert to IP# */
					if (!getnameinfo(result->ai_addr, result->ai_addrlen, name, sizeof(name), NULL, 0, NI_NUMERICHOST))
					{
						/* Update local record for next checkip call. */
						strncpy(p_dyndns->info[i].my_ip_address.name, name, sizeof(p_dyndns->info[i].my_ip_address.name));
						if (p_dyndns->dbg.level > 0)
							logit(LOG_INFO, MODULE_TAG "Resolving hostname %s => IP# %s", p_dyndns->info[i].alias_info[0].names.name, name);
					}
					freeaddrinfo(result);
				}
				else
					logit(LOG_WARNING, MODULE_TAG "Failed resolving hostname %s: %s", p_dyndns->info[i].alias_info[0].names.name, gai_strerror(s));
			}
		}
	}
	else
	{
		/* Read cached IP# from inadyn cache file. */
		if (fgets(name, sizeof(name), fp))
		{
			if (p_dyndns->dbg.level > 0)
				logit(LOG_INFO, MODULE_TAG "Cached IP# %s from previous invocation.", name);
			
			/* Update local record for next checkip call. */
			for (i = 0; i < p_dyndns->info_count; i++)
			{
				strncpy(p_dyndns->info[i].my_ip_address.name, name, sizeof(p_dyndns->info[i].my_ip_address.name));
			}
		}
		fclose(fp);
	}

	do
	{
		rc = dyn_dns_init(p_dyndns);
		if (rc != RC_OK)
		{
			break;
		}

		rc = get_encoded_user_passwd(p_dyndns);
		if (rc != RC_OK)
		{
			break;
		}

		/* DDNS client main loop */
		while (1)
		{
			rc = dyn_dns_update_ip(p_dyndns);
			if (rc != RC_OK)
			{
				if (p_dyndns->cmd == CMD_RESTART)
				{
					logit(LOG_DEBUG, "RESTART command received. Restarting.");
					rc = RC_RESTART;
					break;
				}
				
				if (rc == RC_DYNDNS_RSP_NOTOK && p_dyndns->total_iterations == 1)
				{
					logit(LOG_ERR, MODULE_TAG "Error response from DDNS server, exiting!");
					break;
				}
				
				iterations_err++;
			}
			else
			{
				/* count only the successful iterations */
				p_dyndns->num_iterations++;
				iterations_err = 0;
			}

			/* check if the user wants us to stop */
			if (p_dyndns->total_iterations != 0 && p_dyndns->num_iterations >= p_dyndns->total_iterations)
				break;

			if (p_dyndns->total_iterations != 1)
			{
				if (rc == RC_OK)
					p_dyndns->sleep_sec = p_dyndns->normal_update_period_sec;
				else if (rc == RC_DYNDNS_RSP_RETRY_LATER || rc == RC_DYNDNS_RSP_NOTOK)
					p_dyndns->sleep_sec = p_dyndns->error_update_period_sec;
				else
					p_dyndns->sleep_sec = DYNDNS_FAILED_UPDATE_PERIOD;
			}
			else
				p_dyndns->sleep_sec = DYNDNS_MIN_SLEEP;

			if (rc != RC_OK)
			{
				/* dyn_dns_update_ip() failed above, and we've not reached MAX iterations. 
				 * Time to inform the user the (network) error is not fatal and that we
				 * will try again in a short while. */
				logit(LOG_WARNING, MODULE_TAG "Will retry again in %d sec...", p_dyndns->sleep_sec);
			}

			/* Now sleep a while. Using the time set in sleep_sec data member */
			dyn_dns_wait_for_cmd(p_dyndns);
			if (p_dyndns->cmd == CMD_STOP)
			{
				if (p_dyndns->dbg.level > 1)
					logit(LOG_DEBUG, MODULE_TAG "STOP command received, exiting.");
				rc = RC_OK;
				break;
			}
			else if (p_dyndns->cmd == CMD_RESTART)
			{
				if (p_dyndns->dbg.level > 1)
					logit(LOG_DEBUG, "RESTART command received, restarting.");
				rc = RC_RESTART;
				break;
			}

			if (p_dyndns->total_iterations > 0 && iterations_err > p_dyndns->total_iterations)
			{
				rc = RC_OK;
				break;
			}

			if (p_dyndns->dbg.level > 1)
			{
				logit(LOG_DEBUG, ".");
//				logit(LOG_DEBUG, "Time since last update: %d", p_dyndns->time_since_last_update);
			}
			p_dyndns->time_since_last_update += p_dyndns->sleep_sec;
		}
	}
	while (0);

	/* if everything ok here we should exit. End of program */
	if (rc == RC_OK)
	{
	    rc = dyn_dns_shutdown(p_dyndns);
	}

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
