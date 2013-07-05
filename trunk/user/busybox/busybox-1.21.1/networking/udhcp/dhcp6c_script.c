/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2003 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "dhcp6c.h"

typedef struct {
	const char *name;
	int type;
} envp_list_t;

static const envp_list_t client6_envp_list[] = {
    { "new_sip_servers",		D6_OPT_SIP_SERVER_A   },
    { "new_sip_name",			D6_OPT_SIP_SERVER_D   },
    { "new_domain_name_servers",D6_OPT_DNS            },
    { "new_domain_name",		D6_OPT_DNSNAME        },
    { "new_sntp_servers",		D6_OPT_SNTP_SERVERS   },
    { "new_ntp_servers",		D6_OPT_NTP_SERVER     },
    { "new_nis_servers",		D6_OPT_NIS_SERVERS    },
    { "new_nis_name", 			D6_OPT_NIS_DOMAIN     },
    { "new_nisp_servers",		D6_OPT_NISP_SERVERS   },
    { "new_nisp_name",			D6_OPT_NISP_DOMAIN    },
};

char **fill_envp_client6(struct dhcp6_optinfo *optinfo, const char *ifname)
{
	int i, sz, envc, elen;
	char **envp, **curr;
	struct dhcp6_listval *v;
	int elens[ARRAY_SIZE(client6_envp_list)];

	envc = 3;    /* we at least include the interface, reason and the terminator */

	/* count the number of variables & total env. length */
	for (i=0; i < ARRAY_SIZE(client6_envp_list); i++) {
	    sz = 0;
	    v = TAILQ_FIRST(&optinfo->ad_list);
	    while (v) {
			if (v->dh6optype == client6_envp_list[i].type) {
			    switch (v->lvtype) {
				case DHCP6_LISTVAL_VBUF:
				    sz += v->val_vbuf.dv_len + 1;
				    break;
				case DHCP6_LISTVAL_ADDR6:
				    sz += INET6_ADDRSTRLEN + 1 + 1;
				    break;
				default:
				    break;
			    }
			}
			v = TAILQ_NEXT(v, link);
	    }
	    elens[i] = sz;
	    envc += (sz ? 1 : 0);
	}

	/* allocate an environments array */
	curr = envp = xzalloc(sizeof(char *) * envc);

	/*
	 * Copy the parameters as environment variables
	 */
	/* reason */
	*curr = xstrdup("REASON=NBI");
	putenv(*curr++);
	/* interface name */
	*curr = xasprintf("interface=%s", ifname);
	putenv(*curr++);

	/* "var=addr1 addr2 ... addrN" + null char for termination */
	for (i=0; i < ARRAY_SIZE(client6_envp_list); i++) {
	    if (elens[i] > 0) {
			char a[INET6_ADDRSTRLEN];

			elen = strlen(client6_envp_list[i].name) + 1 + elens[i];
			*curr = xzalloc(elen);
			sprintf(*curr, "%s=", client6_envp_list[i].name);

			TAILQ_FOREACH(v, &optinfo->ad_list, link) {
				if (v->dh6optype != client6_envp_list[i].type)
					continue;
				/* since we count total length above, it is safely to use strcat() */
				switch (v->lvtype) {
				    case DHCP6_LISTVAL_VBUF:
					    strcat(*curr, v->val_vbuf.dv_buf);
					    break;
				    case DHCP6_LISTVAL_ADDR6:
					    sprint_nip6(a, (const uint8_t *)&v->val_addr6);
					    strcat(*curr, a);
					    break;
				    default:
					    break;
				}
			    strcat(*curr, " ");
			}
			putenv(*curr++);
	    }
	}

	return envp;
}

int dhcp6_script(const char *scriptpath, char **envp)
{
	char *argv[2];

	/* if a script is not specified, do nothing */
	if (!scriptpath[0])
		return -2;

	/* launch the script */
	argv[0] = (char *)scriptpath;
	argv[1] = NULL;
	spawn_and_wait(argv);

	/* free env */
	if (envp != NULL) {
		char **curr;

		for (curr = envp; *curr; curr++) {
			bb_unsetenv_and_free(*curr);
		}
		free(envp);
	}
	return 0;
}
