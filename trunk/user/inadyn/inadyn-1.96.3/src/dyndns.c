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
Copyright (C) 2003-2006 INAtech
Author: Narcis Ilisei

*/
/**
	Dyn Dns update main implementation file 
	Author: narcis Ilisei
	Date: May 2003

	History:
		- first implemetnation
		- 18 May 2003 : cmd line option reading added - 
        - Nov 2003 - new version
        - April 2004 - freedns.afraid.org system added.
        - October 2004 - Unix syslog capability added.
*/
#define MODULE_TAG 
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "dyndns.h"
#include "debug_if.h"
#include "base64.h"
#include "md5.h"
#include "get_cmd.h"

#include <nvram/bcmnvram.h>

#define MD5_DIGEST_BYTES (16)

/* DNS systems specific configurations*/

DYNDNS_ORG_SPECIFIC_DATA dyndns_org_dynamic = {"dyndns"};
DYNDNS_ORG_SPECIFIC_DATA dyndns_org_custom = {"custom"};
DYNDNS_ORG_SPECIFIC_DATA dyndns_org_static = {"statdns"};

static int get_req_for_dyndns_server(DYN_DNS_CLIENT *this, int infcnt, int alcnt, DYNDNS_SYSTEM *p_sys_info);
static int get_req_for_freedns_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt, DYNDNS_SYSTEM *p_sys_info);
static int get_req_for_generic_http_dns_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt, DYNDNS_SYSTEM *p_sys_info);
static int get_req_for_noip_http_dns_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt, DYNDNS_SYSTEM *p_sys_info);
static int get_req_for_zoneedit_http_dns_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt, DYNDNS_SYSTEM *p_sys_info);
static int get_req_for_easydns_http_dns_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt, DYNDNS_SYSTEM *p_sys_info);
static int get_req_for_tzo_http_dns_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt, DYNDNS_SYSTEM *p_sys_info);
static int get_req_for_he_ipv6tb_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt, DYNDNS_SYSTEM *p_sys_info);
static int get_req_for_asus_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt, DYNDNS_SYSTEM *p_sys_info);

static BOOL is_dyndns_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, int infcnt, char* p_ok_string);
static BOOL is_freedns_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, int infcnt, char* p_ok_string);
static BOOL is_generic_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, int infcnt, char* p_ok_string);
static BOOL is_zoneedit_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, int infcnt, char* p_ok_string);
static BOOL is_easydns_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, int infcnt, char* p_ok_string);
static BOOL is_tzo_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, int infcnt, char* p_ok_string);
static BOOL is_he_ipv6_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, int infcnt, char* p_ok_string);
static BOOL is_asus_server_register_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, int infcnt, char* p_ok_string);
static BOOL is_asus_server_update_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, int infcnt, char* p_ok_string);

DYNDNS_SYSTEM_INFO dns_system_table[] = 
{ 
    {DYNDNS_DEFAULT, 
        {"default@dyndns.org", &dyndns_org_dynamic, 
            (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_dyndns_server_rsp_ok, 
            (DNS_SYSTEM_REQUEST_FUNC)get_req_for_dyndns_server,
             DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
			DYNDNS_MY_DNS_SERVER, DYNDNS_MY_DNS_SERVER_URL, NULL}},
    {DYNDNS_DYNAMIC, 
        {"dyndns@dyndns.org", &dyndns_org_dynamic,  
            (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_dyndns_server_rsp_ok, 
            (DNS_SYSTEM_REQUEST_FUNC) get_req_for_dyndns_server,
             DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
			DYNDNS_MY_DNS_SERVER, DYNDNS_MY_DNS_SERVER_URL, NULL}},    
    {DYNDNS_CUSTOM, 
        {"custom@dyndns.org", &dyndns_org_custom,  
            (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_dyndns_server_rsp_ok, 
            (DNS_SYSTEM_REQUEST_FUNC) get_req_for_dyndns_server,
             DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
			DYNDNS_MY_DNS_SERVER, DYNDNS_MY_DNS_SERVER_URL, NULL}},
    {DYNDNS_STATIC, 
        {"statdns@dyndns.org", &dyndns_org_static,  
            (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_dyndns_server_rsp_ok, 
            (DNS_SYSTEM_REQUEST_FUNC) get_req_for_dyndns_server,
             DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
				DYNDNS_MY_DNS_SERVER, DYNDNS_MY_DNS_SERVER_URL, NULL}},

    {FREEDNS_AFRAID_ORG_DEFAULT, 
        {"default@freedns.afraid.org", NULL,  
            (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_freedns_server_rsp_ok, 
            (DNS_SYSTEM_REQUEST_FUNC) get_req_for_freedns_server,
            DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
			"freedns.afraid.org", "/dynamic/update.php?", NULL}},

    {ZONE_EDIT_DEFAULT, 
        {"default@zoneedit.com", NULL,  
            (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_zoneedit_server_rsp_ok, 
            (DNS_SYSTEM_REQUEST_FUNC) get_req_for_zoneedit_http_dns_server,
            "dynamic.zoneedit.com", "/checkip.html", 
			"dynamic.zoneedit.com", "/auth/dynamic.html?host=", ""}},

    {NOIP_DEFAULT, 
        {"default@no-ip.com", NULL,  
            (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_dyndns_server_rsp_ok, 
            (DNS_SYSTEM_REQUEST_FUNC) get_req_for_noip_http_dns_server,
            "ip1.dynupdate.no-ip.com", "/", 
			"dynupdate.no-ip.com", "/nic/update?hostname=", ""}},

    {EASYDNS_DEFAULT, 
        {"default@easydns.com", NULL,  
            (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_easydns_server_rsp_ok, 
            (DNS_SYSTEM_REQUEST_FUNC) get_req_for_easydns_http_dns_server,
            DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL, 
			"members.easydns.com", "/dyn/dyndns.php?hostname=", ""}},

    {TZO_DEFAULT,
	{"default@tzo.com", NULL,
	    (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_tzo_server_rsp_ok,
	    (DNS_SYSTEM_REQUEST_FUNC) get_req_for_tzo_http_dns_server,
	    DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
			"cgi.tzo.com", "/webclient/signedon.html?TZOName=", NULL}},

    {DYNDNS_3322_DYNAMIC, 
        {"dyndns@3322.org", &dyndns_org_dynamic,  
            (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_dyndns_server_rsp_ok, 
            (DNS_SYSTEM_REQUEST_FUNC) get_req_for_dyndns_server,
             DYNDNS_3322_MY_IP_SERVER, DYNDNS_3322_MY_IP_SERVER_URL,
			DYNDNS_3322_MY_DNS_SERVER, DYNDNS_3322_MY_DNS_SERVER_URL, NULL}},

    {DNSOMATIC_DEFAULT,
	{"default@dnsomatic.com", &dyndns_org_dynamic,
	    (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_dyndns_server_rsp_ok,
	    (DNS_SYSTEM_REQUEST_FUNC) get_req_for_dyndns_server,
	    DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
			"updates.dnsomatic.com", "/nic/update?", NULL}},

    {HE_IPV6TB,
	{"ipv6tb@he.net", NULL,
	    (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_he_ipv6_server_rsp_ok,
	    (DNS_SYSTEM_REQUEST_FUNC) get_req_for_he_ipv6tb_server,
	    DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
			"ipv4.tunnelbroker.net", "/nic/update?", NULL}},
    {HE_DYNDNS,
	{"dyndns@he.net", NULL,
	    (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC) is_dyndns_server_rsp_ok,
	    (DNS_SYSTEM_REQUEST_FUNC) get_req_for_noip_http_dns_server,
	    DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
			"dyn.dns.he.net", "/nic/update?hostname=", NULL}},

    {ASUS_REGISTER,
	{"register@asus.com", NULL,
	    (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_asus_server_register_rsp_ok,
	    (DNS_SYSTEM_REQUEST_FUNC) get_req_for_asus_server,
	    DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
			ASUS_MY_DNS_SERVER, ASUS_MY_DNS_REGISTER_URL, ASUS_MY_DNS_REGISTER_URL}},
    {ASUS_UPDATE,
	{"update@asus.com", NULL,
	    (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_asus_server_update_rsp_ok,
	    (DNS_SYSTEM_REQUEST_FUNC) get_req_for_asus_server,
	    DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
			ASUS_MY_DNS_SERVER, ASUS_MY_DNS_UPDATE_URL, ASUS_MY_DNS_UPDATE_URL}},

    {CUSTOM_HTTP_BASIC_AUTH, 
        {"custom@http_svr_basic_auth", NULL,  
            (DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_generic_server_rsp_ok, 
            (DNS_SYSTEM_REQUEST_FUNC) get_req_for_generic_http_dns_server,
            GENERIC_DNS_IP_SERVER_NAME, DYNDNS_MY_IP_SERVER_URL,
			"", "", "OK"}},

    {LAST_DNS_SYSTEM, {NULL, NULL, NULL, NULL, NULL, NULL}}
};

static DYNDNS_SYSTEM* get_dns_system_by_id(DYNDNS_SYSTEM_ID id)
{
    {
        DYNDNS_SYSTEM_INFO *it;
        for (it = dns_system_table; it->id != LAST_DNS_SYSTEM; ++it)
        {
            if (it->id == id)
            {
                return &it->system;
            }
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
	int counter = p_self->sleep_sec / p_self->cmd_check_period;
	DYN_DNS_CMD old_cmd = p_self->cmd;

	if (old_cmd != NO_CMD)
	{
		return RC_OK;
	}

	while( counter --)
	{
		if (p_self->cmd != old_cmd)
		{
			break;
		}
		os_sleep_ms(p_self->cmd_check_period * 1000);
	}
	return RC_OK;
}

static int get_req_for_dyndns_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt, DYNDNS_SYSTEM *p_sys_info)
{	
    DYNDNS_ORG_SPECIFIC_DATA *p_dyndns_specific = 
		(DYNDNS_ORG_SPECIFIC_DATA*) p_sys_info->p_specific_data;
	return sprintf(p_self->p_req_buffer, DYNDNS_GET_MY_IP_HTTP_REQUEST_FORMAT,
		p_self->info[infcnt].dyndns_server_url,
		p_dyndns_specific->p_system,
		p_self->info[infcnt].alias_info[alcnt].names.name,
		p_self->info[infcnt].my_ip_address.name,
		p_self->info[infcnt].wildcard ? "ON" : "OFF",
		p_self->info[infcnt].alias_info[alcnt].names.name,
		p_self->info[infcnt].dyndns_server_name.name,
		p_self->info[infcnt].credentials.p_enc_usr_passwd_buffer
		);
}

static int get_req_for_freedns_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt, DYNDNS_SYSTEM *p_sys_info)
{
	(void)p_sys_info;
	return sprintf(p_self->p_req_buffer, FREEDNS_UPDATE_MY_IP_REQUEST_FORMAT,
		p_self->info[infcnt].dyndns_server_url,
		p_self->info[infcnt].alias_info[alcnt].hashes.str,
		p_self->info[infcnt].dyndns_server_name.name);
}


static int get_req_for_generic_http_dns_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt, DYNDNS_SYSTEM *p_sys_info)
{
	(void)p_sys_info;
	return sprintf(p_self->p_req_buffer, GENERIC_DNS_BASIC_AUTH_MY_IP_REQUEST_FORMAT,
		p_self->info[infcnt].dyndns_server_url,
		p_self->info[infcnt].alias_info[alcnt].names.name,
		p_self->info[infcnt].credentials.p_enc_usr_passwd_buffer,
		p_self->info[infcnt].dyndns_server_name.name);
}
static int get_req_for_noip_http_dns_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt,  DYNDNS_SYSTEM *p_sys_info)
{
	(void)p_sys_info;
	return sprintf(p_self->p_req_buffer, GENERIC_NOIP_AUTH_MY_IP_REQUEST_FORMAT,
		p_self->info[infcnt].dyndns_server_url,
		p_self->info[infcnt].alias_info[alcnt].names.name,
		p_self->info[infcnt].my_ip_address.name,
		p_self->info[infcnt].credentials.p_enc_usr_passwd_buffer,
		p_self->info[infcnt].dyndns_server_name.name
		);
}

static int get_req_for_zoneedit_http_dns_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt,  DYNDNS_SYSTEM *p_sys_info)
{
	(void)p_sys_info;
	return sprintf(p_self->p_req_buffer, GENERIC_ZONEEDIT_AUTH_MY_IP_REQUEST_FORMAT,
		p_self->info[infcnt].dyndns_server_url,
		p_self->info[infcnt].alias_info[alcnt].names.name,
		p_self->info[infcnt].my_ip_address.name,
		p_self->info[infcnt].credentials.p_enc_usr_passwd_buffer,
		p_self->info[infcnt].dyndns_server_name.name
		);
}

static int get_req_for_easydns_http_dns_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt, DYNDNS_SYSTEM *p_sys_info)
{
	(void)p_sys_info;
	return sprintf(p_self->p_req_buffer, GENERIC_EASYDNS_AUTH_MY_IP_REQUEST_FORMAT,
		p_self->info[infcnt].dyndns_server_url,
		p_self->info[infcnt].alias_info[alcnt].names.name,
		p_self->info[infcnt].my_ip_address.name,
		p_self->info[infcnt].wildcard ? "ON" : "OFF",
		p_self->info[infcnt].credentials.p_enc_usr_passwd_buffer,
		p_self->info[infcnt].dyndns_server_name.name
		);
}

static int get_req_for_tzo_http_dns_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt, DYNDNS_SYSTEM *p_sys_info)
{
	(void)p_sys_info;
	return sprintf(p_self->p_req_buffer, GENERIC_TZO_AUTH_MY_IP_REQUEST_FORMAT,
	    p_self->info[infcnt].dyndns_server_url,
	    p_self->info[infcnt].alias_info[alcnt].names.name,
	    p_self->info[infcnt].credentials.my_username,
	    p_self->info[infcnt].credentials.my_password,
	    p_self->info[infcnt].my_ip_address.name,
	    p_self->info[infcnt].dyndns_server_name.name);
}

static int get_req_for_he_ipv6tb_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt, DYNDNS_SYSTEM *p_sys_info)
{
	unsigned char digestbuf[MD5_DIGEST_BYTES];
	char digeststr[MD5_DIGEST_BYTES*2+1];
	int i;

	(void)p_sys_info;
	md5_buffer(p_self->info[infcnt].credentials.my_password,
		strlen(p_self->info[infcnt].credentials.my_password), digestbuf);
	for(i = 0; i < MD5_DIGEST_BYTES; i++)
		sprintf(&digeststr[i*2], "%02x", digestbuf[i]);
	return sprintf(p_self->p_req_buffer, HE_IPV6TB_UPDATE_MY_IP_REQUEST_FORMAT,
	    p_self->info[infcnt].dyndns_server_url,
	    p_self->info[infcnt].my_ip_address.name,
	    p_self->info[infcnt].credentials.my_username,
	    digeststr,
	    p_self->info[infcnt].alias_info[alcnt].names.name,
	    p_self->info[infcnt].dyndns_server_name.name);
}

static int get_req_for_asus_server(DYN_DNS_CLIENT *p_self, int infcnt, int alcnt, DYNDNS_SYSTEM *p_sys_info)
{
	unsigned char digest[MD5_DIGEST_BYTES];
	char auth[6*2+1+MD5_DIGEST_BYTES*2+1];
	char *p_tmp, *p_auth = auth;
	int i, size;

	(void)p_sys_info;

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
	if (p_self->info[infcnt].credentials.p_enc_usr_passwd_buffer != NULL)
		free(p_self->info[infcnt].credentials.p_enc_usr_passwd_buffer);
	p_self->info[infcnt].credentials.p_enc_usr_passwd_buffer = b64encode(auth);
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

static int get_req_for_ip_server(DYN_DNS_CLIENT *p_self, int infcnt, void *p_specific_data)
{
    return sprintf(p_self->p_req_buffer, DYNDNS_GET_MY_IP_HTTP_REQUEST,
        p_self->info[infcnt].ip_server_name.name, p_self->info[infcnt].ip_server_url);
}

/* 
	Send req to IP server and get the response
*/
static RC_TYPE do_ip_server_transaction(DYN_DNS_CLIENT *p_self, int servernum)
{
	RC_TYPE rc = RC_OK;
	HTTP_CLIENT *p_http;

	p_http = &p_self->http_to_ip_server[servernum];

	rc = http_client_init(p_http);
	if (rc != RC_OK)
	{
		return rc;
	}

	do
	{
		/*prepare request for IP server*/
		{
			HTTP_TRANSACTION *p_tr = &p_self->http_tr;
			p_tr->req_len = get_req_for_ip_server((DYN_DNS_CLIENT*) p_self,
				servernum,
				p_self->info[servernum].p_dns_system->p_specific_data);
			if (p_self->dbg.level > 2) 
			{
				DBG_PRINTF((LOG_DEBUG, "The request for IP server:\n%s\n",p_self->p_req_buffer));
			}
			p_tr->p_req = (char*) p_self->p_req_buffer;
			p_tr->p_rsp = (char*) p_self->p_work_buffer;
			p_tr->max_rsp_len = p_self->work_buffer_size - 1;/*save place for a \0 at the end*/
			p_tr->rsp_len = 0;
			
			rc = http_client_transaction(p_http, &p_self->http_tr);
			p_self->p_work_buffer[p_tr->rsp_len] = 0;
		}
	}
	while(0);

	/*close*/
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
	char *p_ip;
	char *p_current_str = p_self->http_tr.p_rsp;
	BOOL found;
    char new_ip_str[IP_V4_MAX_LENGTH];

	if (p_self->http_tr.rsp_len <= 0 || 
		p_self->http_tr.p_rsp == NULL)
	{
		return RC_INVALID_POINTER;
	}

	found = FALSE;
	do
	{
		/*try to find first decimal number (begin of IP)*/
		p_ip = strpbrk(p_current_str, DYNDNS_ALL_DIGITS);
		if (p_ip != NULL)
		{
			/*maybe I found it*/
			count = sscanf(p_ip, DYNDNS_IP_ADDR_FORMAT,
							&ip1, &ip2, &ip3, &ip4);
			if (count != 4 ||
				ip1 <= 0 || ip1 > 255 ||
				ip2 < 0 || ip2 > 255 ||
				ip3 < 0 || ip3 > 255 ||
				ip4 < 0 || ip4 > 255 )
			{
				p_current_str = p_ip + 1;
			}
			else
			{
				/* FIRST occurence of a valid IP found*/
				found = TRUE;
				break;
			}
		}
	}
	while(p_ip != NULL);
		   

	if (found)
	{        
		i = 0;
		do
		{
        sprintf(new_ip_str, DYNDNS_IP_ADDR_FORMAT, ip1, ip2, ip3, ip4);
			p_self->info[i].my_ip_has_changed = (strcmp(new_ip_str, p_self->info[i].my_ip_address.name) != 0);
			strcpy(p_self->info[i].my_ip_address.name, new_ip_str);
		}
		while(++i < p_self->info_count);
		return RC_OK;
	}
	else
	{
		return RC_DYNDNS_INVALID_RSP_FROM_IP_SERVER;
	}	
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

	/*uses fix test if ip of server 0 has changed */
	/*that should be ok even if changes dyn_dns_update_ip to */
	/*iterate over servernum, but not if it's fix set to =! 0 */
	if (p_self->info[0].my_ip_has_changed ||
        p_self->force_addr_update ||
	(p_self->times_since_last_update > p_self->forced_update_times)
       )
    {
		for (i = 0; i < p_self->info_count; i++)
	    {
			for (j = 0; j < p_self->info[i].alias_count; j++)
			{
				p_self->info[i].alias_info[j].update_required = TRUE;
			{
				DBG_PRINTF((LOG_WARNING,"I:" MODULE_TAG "IP address for alias '%s' needs update to '%s'\n",
						p_self->info[i].alias_info[j].names.name,
						p_self->info[i].my_ip_address.name));
				}
			}
        }
    }
    return RC_OK;
}


/* DynDNS org.specific response validator.
    'good' or 'nochange' are the good answers,
*/
static BOOL is_dyndns_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, int infnr, char* p_ok_string)
{
	(void) p_ok_string;
    return ( (strstr(p_rsp, DYNDNS_OK_RESPONSE) != NULL) ||
             (strstr(p_rsp, DYNDNS_OK_NOCHANGE) != NULL) );
}

/* Freedns afraid.org.specific response validator.
    ok blabla and n.n.n.n
    fail blabla and n.n.n.n
    are the good answers. We search our own IP address in response and that's enough.
*/
static BOOL is_freedns_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, int infnr, char* p_ok_string)
{
	(void) p_ok_string;
    return (strstr(p_rsp, p_self->info[infnr].my_ip_address.name) != NULL);
}

/** generic http dns server ok parser 
	parses a given string. If found is ok,
	Example : 'SUCCESS CODE='
*/
static BOOL is_generic_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, int infnr, char* p_ok_string)
{
	if (p_ok_string == NULL)
	{
		return FALSE;
	}
    return (strstr(p_rsp, p_ok_string) != NULL);
}

/**
	the OK codes are:
		CODE=200,2001
		CODE=707, for duplicated updates
*/
static BOOL is_zoneedit_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, int infnr, char* p_ok_string)
{
	return ((strstr(p_rsp, "CODE=\"200\"") != NULL) ||
		(strstr(p_rsp, "CODE=\"201\"") != NULL) ||
		(strstr(p_rsp, "CODE=\"707\"") != NULL));
}

/**
	NOERROR is the OK code here
*/
static BOOL is_easydns_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, int infnr, char* p_ok_string)
{
	return (strstr(p_rsp, "NOERROR") != NULL);
}

/* TZO specific response validator.
    If we have an HTTP 302 the update wasn't good and we're being redirected 
*/
static BOOL is_tzo_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, int infnr, char* p_ok_string)
{
	(void) p_ok_string;
	return (strstr(p_rsp, " HTTP/1.%*c 302") == NULL);
}

/* HE ipv6 tunnelbroker specific response validator.
    own IP address and 'already in use' are the good answers.
*/
static BOOL is_he_ipv6_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, int infnr, char* p_ok_string)
{
	(void) p_ok_string;
	return ((strstr(p_rsp, p_self->info[infnr].my_ip_address.name) != NULL) ||
		(strstr(p_rsp, "already") != NULL));
}

static BOOL is_asus_server_register_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, int infnr, char* p_ok_string)
{
	int ret;
	char *p, *ret_key, ret_buf[64], domain[256] = "";
	(void) p_ok_string;

	ret_key = "ddns_return_code";

	if (sscanf(p_rsp, "HTTP/1.%*c %3d", &ret) != 1) {
		snprintf(ret_buf, sizeof(ret_buf), "%s,%d", "register", -1);
		nvram_set(ret_key, ret_buf);
		return FALSE;
	}

	if ((p = strchr(p_rsp, '|')) && (p = strchr(++p, '|')))
		sscanf(p, "|%255[^|\r\n]", domain);

	if (ret >= 500)		/* shutdown */
		return FALSE;

	snprintf(ret_buf, sizeof(ret_buf), "%s,%d", "register", ret);
	nvram_set(ret_key, ret_buf);

	switch (ret) {
	case 200:		/* registration success */
	case 220:		/* registration same domain success*/
		return TRUE;
	case 203:		/* registration failed */
		DBG_PRINTF((LOG_WARNING,"W: Registration failed, suggested domain '%s'\n", domain));
		return FALSE;
	case 230:		/* registration new domain success */
		DBG_PRINTF((LOG_WARNING,"W: Registration success, previous domain '%s'\n", domain));
		return TRUE;
	case 233:		/* registration failed */
		DBG_PRINTF((LOG_WARNING,"W: Registration failed, previous domain '%s'\n", domain));
		return FALSE;
	case 297:		/* invalid hostname */
	case 298:		/* invalid domain name */
	case 299:		/* invalid ip format */
	case 401:		/* authentication failure */
	case 407:		/* proxy authentication required */
		return FALSE;
	}

	if (ret >= 500)		/* shutdown */
		nvram_set(ret_key, "unknown_error");
	else			/* unknown error */
		nvram_set(ret_key, "time_out");

	return FALSE;
}

static BOOL is_asus_server_update_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, int infnr, char* p_ok_string)
{
	int ret;
	(void) p_ok_string;
	static int z = 0;

	if (sscanf(p_rsp, "HTTP/1.%*c %3d", &ret) != 1)
		return FALSE;

	switch (ret) {
	case 200:		/* update success */
	case 220:		/* update same domain success*/
		return TRUE;
	case 297:		/* invalid hostname */
	case 298:		/* invalid domain name */
	case 299:		/* invalid ip format */
	case 401:		/* authentication failure */
	case 407:		/* proxy authentication required */
		return FALSE;
	}

	if (ret >= 500)		/* shutdown */
		return FALSE;

	return FALSE;		/* unknown error */
}

static RC_TYPE do_update_alias_table(DYN_DNS_CLIENT *p_self)
{
	int i, j;
	RC_TYPE rc = RC_OK;
	FILE *fp;
	
	do 
	{
		for (i = 0; i < p_self->info_count; i++)
		{
			for (j = 0; j < p_self->info[i].alias_count; j++)
		{
				if (p_self->info[i].alias_info[j].update_required != TRUE)
			{
				continue;
			}
			
			rc = http_client_init(&p_self->http_to_dyndns[i]);
			if (rc != RC_OK)
			{
				break;
			}
			
			/*build dyndns transaction*/
			{
				HTTP_TRANSACTION http_tr;
					http_tr.req_len = p_self->info[i].p_dns_system->p_dns_update_req_func(
						(struct _DYN_DNS_CLIENT*) p_self, i, j,
						(struct DYNDNS_SYSTEM*) p_self->info[i].p_dns_system);
				http_tr.p_req = (char*) p_self->p_req_buffer;
				http_tr.p_rsp = (char*) p_self->p_work_buffer;
				http_tr.max_rsp_len = p_self->work_buffer_size - 1;/*save place for a \0 at the end*/
				http_tr.rsp_len = 0;
				p_self->p_work_buffer[http_tr.rsp_len+1] = 0;
				
				/*send it*/
				rc = http_client_transaction(&p_self->http_to_dyndns[i], &http_tr);

				if (p_self->dbg.level > 2)
				{
					p_self->p_req_buffer[http_tr.req_len] = 0;
					DBG_PRINTF((LOG_DEBUG,"DYNDNS my Request:\n%s\n", p_self->p_req_buffer));
				}

				if (rc == RC_OK)
				{
					BOOL update_ok = 
							p_self->info[i].p_dns_system->p_rsp_ok_func((struct _DYN_DNS_CLIENT*)p_self, 
							http_tr.p_rsp, 
							i,
							p_self->info[i].p_dns_system->p_success_string);
					if (update_ok)
					{
						p_self->info[i].alias_info[j].update_required = FALSE;
						
						DBG_PRINTF((LOG_WARNING,"I:" MODULE_TAG "Alias '%s' to IP '%s' updated successful.\n", 
								p_self->info[i].alias_info[j].names.name,
								p_self->info[i].my_ip_address.name));
						p_self->times_since_last_update = 0;
						/*recalc forced update period*/
						p_self->forced_update_period_sec = p_self->forced_update_period_sec_orig;
						p_self->forced_update_times = p_self->forced_update_period_sec / p_self->sleep_sec;
					#ifndef USE_CACHE_FILE
						if ((fp=fopen(p_self->ip_cache, "w")))
						{
							fprintf(fp,"%s", p_self->info[i].my_ip_address.name);
							fclose(fp);
						}
						if ((fp=fopen(p_self->time_cache, "w")))
						{
							fprintf(fp,"%ld", time (NULL));
							fclose(fp);
						}
					#else
						if ((fp=fopen(p_self->file_cache, "w")))
						{
							fprintf(fp,"%ld,%s\n", time (NULL), p_self->info[i].my_ip_address.name);
							fclose(fp);
						}
					#endif
						if (strlen(p_self->external_command) > 0)
							os_shell_execute(p_self->external_command);
					}
					else
					{
						DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "Error validating DYNDNS svr answer. Check usr,pass,hostname,abuse...!\n", http_tr.p_rsp));
						rc = RC_DYNDNS_RSP_NOTOK;
					}
					if (p_self->dbg.level > 2 || !update_ok)
					{
						http_tr.p_rsp[http_tr.rsp_len] = 0;
						DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "DYNDNS Server response:\n%s\n", http_tr.p_rsp));
					}
				}
			}
			
			{
				RC_TYPE rc2 = http_client_shutdown(&p_self->http_to_dyndns[i]);
				if (rc == RC_OK)
				{
					rc = rc2;
				}
			}
			if (rc != RC_OK)
			{
				break;
			}
			os_sleep_ms(1000);
		}
		}
		if (rc != RC_OK)
		{
			break;
		}
	}
	while(0);
	return rc;
}


RC_TYPE get_default_config_data(DYN_DNS_CLIENT *p_self)
{
	RC_TYPE rc = RC_OK;

	do
	{
		p_self->info[0].p_dns_system = get_dns_system_by_id(DYNDNS_MY_DNS_SYSTEM);	
		if (p_self->info[0].p_dns_system == NULL)
        {
            rc = RC_DYNDNS_INVALID_DNS_SYSTEM_DEFAULT;
            break;
        }
						
		/*forced update period*/
		p_self->forced_update_period_sec = DYNDNS_MY_FORCED_UPDATE_PERIOD_S;
		p_self->forced_update_period_sec_orig = DYNDNS_MY_FORCED_UPDATE_PERIOD_S;
#ifdef UNIX_OS
	#ifndef USE_CACHE_FILE
		sprintf(p_self->ip_cache, "%s%s", DYNDNS_DEFAULT_CACHE_PREFIX, DYNDNS_DEFAULT_IP_FILE);
		sprintf(p_self->time_cache, "%s%s", DYNDNS_DEFAULT_CACHE_PREFIX, DYNDNS_DEFAULT_TIME_FILE);
	#else
		sprintf(p_self->file_cache, "%s%s", DYNDNS_DEFAULT_CACHE_PREFIX, DYNDNS_DEFAULT_CACHE_FILE);
        #endif
#endif
#ifdef _WIN32
	#ifndef USE_CACHE_FILE
		sprintf(p_self->ip_cache, "%s", DYNDNS_DEFAULT_IP_FILE);
		sprintf(p_self->time_cache, "%s", DYNDNS_DEFAULT_TIME_FILE);
	#else
		sprintf(p_self->file_cache, "%s", DYNDNS_DEFAULT_CACHE_FILE);
	#endif
#endif
		/*update period*/
		p_self->sleep_sec = DYNDNS_DEFAULT_SLEEP;
	}
	while(0);
	
	return rc;
}


static RC_TYPE get_encoded_user_passwd(DYN_DNS_CLIENT *p_self)
{
	RC_TYPE rc = RC_OK;
	const char* format = "%s:%s";
	char *p_tmp_buff = NULL;
	int size, actual_len;
	int i = 0;

	do
	{
		size = strlen(p_self->info[i].credentials.my_password) + 
			strlen(p_self->info[i].credentials.my_username) + 
			strlen(format) + 1;

		p_tmp_buff = (char *) malloc(size);
		if (p_tmp_buff == NULL)
		{
			rc = RC_OUT_OF_MEMORY;
			break;
		}

		actual_len = sprintf(p_tmp_buff, format, 
				p_self->info[i].credentials.my_username, 
				p_self->info[i].credentials.my_password);
		if (actual_len >= size)
		{
			rc = RC_OUT_BUFFER_OVERFLOW;
			break;
		}

		/*encode*/
		p_self->info[i].credentials.p_enc_usr_passwd_buffer = b64encode(p_tmp_buff);	
		p_self->info[i].credentials.encoded = 
			(p_self->info[i].credentials.p_enc_usr_passwd_buffer != NULL);
		p_self->info[i].credentials.size = strlen(p_self->info[i].credentials.p_enc_usr_passwd_buffer);

		if (p_tmp_buff != NULL)
		{
			free(p_tmp_buff);
			p_tmp_buff = NULL;
	}
	}
	while(++i < p_self->info_count);

	if (p_tmp_buff != NULL)
	{
		free(p_tmp_buff);
		p_tmp_buff = NULL;
	}
	return rc;	
}

/*************PUBLIC FUNCTIONS ******************/

/*
	printout
*/
void dyn_dns_print_hello(void*p)
{
	(void) p;

    DBG_PRINTF((LOG_INFO, MODULE_TAG "Started 'INADYN version %s' - dynamic DNS updater.\n", DYNDNS_VERSION_STRING));
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
	/*alloc space for me*/
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
		while(i < DYNDNS_MAX_SERVER_NUMBER)
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
		while(i < DYNDNS_MAX_SERVER_NUMBER)
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
		(p_self)->sleep_sec = DYNDNS_DEFAULT_SLEEP;
		(p_self)->total_iterations = DYNDNS_DEFAULT_ITERATIONS;	
		(p_self)->initialized = FALSE;
	
		i = 0;		
		while(i < DYNDNS_MAX_SERVER_NUMBER)
		{
			p_self->info[i++].credentials.p_enc_usr_passwd_buffer = NULL;
		}
	}
	while(0);

    if (rc != RC_OK)
    {
        if (*pp_self)
        {
            free(*pp_self);
        }
        if (p_self->p_work_buffer != NULL)
        {
            free(p_self->p_work_buffer);
        }
        if (p_self->p_req_buffer != NULL)
        {
            free (p_self->p_work_buffer);
        }
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
		
	}

	rc = http_client_destruct(p_self->http_to_dyndns, DYNDNS_MAX_SERVER_NUMBER);
	if (rc != RC_OK)
	{	
		
	}

	if (p_self->p_work_buffer != NULL)
	{
		free(p_self->p_work_buffer);
		p_self->p_work_buffer = NULL;
	}

	if (p_self->p_req_buffer != NULL)
	{
		free(p_self->p_req_buffer);
		p_self->p_req_buffer = NULL;
	}

	i = 0;		
	while(i < DYNDNS_MAX_SERVER_NUMBER)
	{
		if (p_self->info[i].credentials.p_enc_usr_passwd_buffer != NULL)
	{
			free(p_self->info[i].credentials.p_enc_usr_passwd_buffer);
			p_self->info[i].credentials.p_enc_usr_passwd_buffer = NULL;
		}
		i++;
	}

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
	p_self->force_addr_update = FALSE;

	do
    {
		if (strlen(p_self->info[i].proxy_server_name.name) > 0)
		{
			http_client_set_port(&p_self->http_to_ip_server[i], p_self->info[i].proxy_server_name.port);
			http_client_set_remote_name(&p_self->http_to_ip_server[i], p_self->info[i].proxy_server_name.name);

			http_client_set_port(&p_self->http_to_dyndns[i], p_self->info[i].proxy_server_name.port);
			http_client_set_remote_name(&p_self->http_to_dyndns[i], p_self->info[i].proxy_server_name.name);
    }
    else
    {
			http_client_set_port(&p_self->http_to_ip_server[i], p_self->info[i].ip_server_name.port);
			http_client_set_remote_name(&p_self->http_to_ip_server[i], p_self->info[i].ip_server_name.name);

			http_client_set_port(&p_self->http_to_dyndns[i], p_self->info[i].dyndns_server_name.port);
			http_client_set_remote_name(&p_self->http_to_dyndns[i], p_self->info[i].dyndns_server_name.name);    
		}
    }
	while(++i < p_self->info_count);

	p_self->cmd = NO_CMD;
    if (p_self->cmd_check_period == 0)
    {
	    p_self->cmd_check_period = DYNDNS_DEFAULT_CMD_CHECK_PERIOD;
    }


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
		/*ask IP server something so he will respond and give me my IP */
		rc = do_ip_server_transaction(p_self, servernum);
		if (rc != RC_OK)
		{
			DBG_PRINTF((LOG_WARNING,"W: Error '%s' (0x%x) when talking to IP server\n",
				errorcode_get_name(rc), rc));
			break;
		}
		if (p_self->dbg.level > 1)
		{
			DBG_PRINTF((LOG_DEBUG,"IP server response: %s\n", p_self->p_work_buffer));
		}

		/*extract my IP, check if different than previous one*/
		rc = do_parse_my_ip_address(p_self, servernum);
		if (rc != RC_OK)
		{
			break;
		}
		
		if (p_self->dbg.level > 1)
		{
			DBG_PRINTF((LOG_WARNING,"W: My IP address: %s\n", p_self->info[servernum].my_ip_address.name));	
		}

		/*step through aliases list, resolve them and check if they point to my IP*/
		rc = do_check_alias_update_table(p_self);
		if (rc != RC_OK)
		{
			break;
		}

		/*update IPs marked as not identical with my IP*/
		rc = do_update_alias_table(p_self);
		if (rc != RC_OK)
		{
			break;
		}
	}
	while(0);

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
	RC_TYPE rc = RC_OK;
	int iterations = 0;
	int iterations_err = 0;
	BOOL quit_flag = FALSE;
	BOOL init_flag;
	BOOL os_handler_installed = FALSE;
	FILE *fp;

	if (p_dyndns == NULL)
	{
		return RC_INVALID_POINTER;
	}

	/* read cmd line options and set object properties*/
	rc = get_config_data(p_dyndns, argc, argv);
	if (rc != RC_OK || p_dyndns->abort)
	{
		return rc;
	}

    /*if logfile provided, redirect output to log file*/
    if (strlen(p_dyndns->dbg.p_logfilename) != 0)
    {
        rc = os_open_dbg_output(DBG_FILE_LOG, "", p_dyndns->dbg.p_logfilename);
        if (rc != RC_OK)
        {
            return rc;
        }
    }

	if (p_dyndns->debug_to_syslog == TRUE ||
       (p_dyndns->run_in_background == TRUE))
	{
		if (get_dbg_dest() == DBG_STD_LOG) /*avoid file and syslog output */
        {
            rc = os_open_dbg_output(DBG_SYS_LOG, "INADYN", NULL);
            if (rc != RC_OK)
            {
                return rc;
            }
        }
	}

    /*if silent required, close console window*/
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

	/*if pid-file wanted, create it*/
	if (p_dyndns->p_pidfilename && strlen(p_dyndns->p_pidfilename) != 0)
	{
		FILE *fp;
		int obj = 0;
		char pid[7];
		if ((fp = fopen(p_dyndns->p_pidfilename, "w")))
		{
			sprintf(pid, "%d\n", getpid());
			obj = fwrite(pid, strlen(pid), 1, fp);
			fclose(fp);
		}
		free(p_dyndns->p_pidfilename);
		p_dyndns->p_pidfilename = NULL;
		if (!fp || obj != 1)
		{
			return RC_FILE_IO_OPEN_ERROR;
		}
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

    dyn_dns_print_hello(NULL);

#ifndef USE_CACHE_FILE
    if ((fp=fopen(p_dyndns->ip_cache, "r")))
    {
	if (!fgets (p_dyndns->info[0].my_ip_address.name, sizeof (p_dyndns->info[0].my_ip_address.name),fp)) {
		DBG_PRINTF((LOG_WARNING,"Error reading IP from cache\n"));
	}
	fclose(fp);
	DBG_PRINTF((LOG_INFO, MODULE_TAG "IP read from cache file is '%s'. No update required.\n", p_dyndns->info[0].my_ip_address.name));
    }
#else
    if ((fp=fopen(p_dyndns->file_cache, "r")))
    {
	int dif;

	if (fscanf(fp, "%ld,%16s", &dif, p_dyndns->info[0].my_ip_address.name) < 2) {
		DBG_PRINTF((LOG_WARNING,"Error reading IP from cache\n"));
	}
	fclose(fp);
	DBG_PRINTF((LOG_INFO, MODULE_TAG "IP read from cache file is '%s'. No update required.\n", p_dyndns->info[0].my_ip_address.name));
    }
#endif

	/* the real work here */
	do
	{
		/* init object */
		init_flag = FALSE;
		rc = dyn_dns_init(p_dyndns);
		if (rc != RC_OK)
		{
			break;
		}
		init_flag = TRUE;

		rc = get_encoded_user_passwd(p_dyndns);
		if (rc != RC_OK)
		{
			break;
		}

		if (!os_handler_installed)
		{
			rc = os_install_signal_handler(p_dyndns);
			if (rc != RC_OK)
			{
				DBG_PRINTF((LOG_WARNING,"Error '%s' (0x%x) installing OS signal handler\n",
					errorcode_get_name(rc), rc));
				break;
			}
			os_handler_installed = TRUE;
		}
		
		/*update IP address in a loop*/
		while(1)
		{
			rc = dyn_dns_update_ip(p_dyndns);
			if (rc != RC_OK)
			{
				DBG_PRINTF((LOG_WARNING,"W:'%s' (0x%x) updating the IPs. (it %d)\n",
					errorcode_get_name(rc), rc, iterations));
				if (rc == RC_DYNDNS_RSP_NOTOK)
				{
					iterations_err++;
					DBG_PRINTF((LOG_ERR,"E: The response of DYNDNS svr was an error! Aborting.\n"));
					break;
				}
			}
			else /*count only the successful iterations */
			{
				++iterations;
				iterations_err = 0;
			}
			
			/* check if the user wants us to stop */
			if (iterations >= p_dyndns->total_iterations &&
				p_dyndns->total_iterations != 0)
			{
				break;
			}
			
			/* also sleep the time set in the ->sleep_sec data memeber*/
			dyn_dns_wait_for_cmd(p_dyndns);
			if (p_dyndns->cmd == CMD_STOP)
			{
				DBG_PRINTF((LOG_DEBUG,"STOP command received. Exiting.\n"));
				rc = RC_OK;
				break;
			}
			
			if (rc == RC_OK)
			{
				if (p_dyndns->dbg.level > 0)
				{
					DBG_PRINTF((LOG_DEBUG,"."));
				}
				p_dyndns->times_since_last_update ++;
			}
			else
			{
				dyn_dns_shutdown(p_dyndns);
				init_flag = FALSE;
				break;
			}
		}
		/*if everything ok here we should exit. End of program*/
		if (rc == RC_OK)
		{
			break;
		}
		
		if (iterations_err > 0)
		{
			if (p_dyndns->total_iterations != 0 && iterations_err > (p_dyndns->total_iterations + 1))
				break;
			
			sleep(1);
		}
	}
	while(quit_flag == FALSE);

	if (init_flag == TRUE)
	{
	    /* dyn_dns_shutdown object */
	    rc = dyn_dns_shutdown(p_dyndns);
	}
	
	return rc;
}
