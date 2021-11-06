/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2002 WIDE Project.
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

#include <net/if.h>

#include "dhcp6c.h"
#include "dhcp6c_auth.h"

/* structures and definitions used in the config file parser */
struct cf_list {
	struct cf_list *next;
	struct cf_list *tail;

	int type;
	int line;		/* the line number of the config file */

	/* type dependent values: */
	char *ptr;
	struct cf_list *sublist;
};
#define MAKE_CFLIST(l, t, pp, pl) do { \
	(l) = (struct cf_list *)xzalloc(sizeof(*(l))); \
	l->line = lineno; \
	l->type = (t); \
	l->ptr  = (pp); \
	l->sublist = (pl); \
	l->tail = (l); \
	} while (0)

static void cleanup_cflist(struct cf_list *);


/* DHCPv6 authentication information */
struct authinfo {
	struct authinfo *next;

	char *name;		/* auth info name */

	int protocol;	/* authentication protocol */
	int algorithm;	/* authentication algorithm */
	int rdm;		/* random attack detection method */

	/* keys specific to this info? */
};

static int add_pd_pif(struct ia_conf *, struct cf_list *);
static int add_options(int, struct dhcp6_if *, struct cf_list *);
static int add_prefix6(struct dhcp6_list *, int type, int lvtype,
						struct dhcp6_prefix *);
static void clear_pd_pif(struct ia_conf *);
static void clear_iaconf(struct ia_conflist *);
#if ENABLE_FEATURE_DHCP6_AUTH
static void clear_keys(struct keyinfo *);
static void clear_authinfo(struct authinfo *);
#endif
static int get_default_ifid(struct prefix_ifconf *);
static char* FAST_FUNC qstrdup(char *);


static int configure_interface(void)
{
	struct cf_list *cnl;
	struct dhcp6_if *ifp = &client6_config.dhcp6c_if;

	for (cnl = client6_config.config_list; cnl; cnl = cnl->next) {
		struct cf_list *cfl;

		if (cnl->type != DECL_IFACE)
			continue;
		log3("configure iface '%s'", cnl->ptr);

		if (if_nametoindex(cnl->ptr) == 0) {
			bb_perror_msg("%s:%d invalid interface(%s)",
				client6_config.conffile, cnl->line,	cnl->ptr);
			return -1;
		}

		if (strcmp(ifp->ifname, cnl->ptr) != 0) {
			log1("Unknown interface(%s), ignoring", cnl->ptr);
			continue;
		}

		ifp->server_pref = -1;

		for (cfl = cnl->sublist; cfl; cfl = cfl->next) {
			switch (cfl->type) {
			case DECL_REQUEST:
			case DECL_SEND:
				if (add_options(cfl->type, ifp, cfl->sublist))
					return -1;
				break;
			case DECL_INFO_ONLY:
				ifp->send_flags |= DHCIFF_INFO_ONLY;
				break;
			case DECL_SCRIPT:
				if (ifp->scriptpath) {
					bb_info_msg("%s:%d duplicated script",
					    client6_config.conffile, cfl->line);
					return -1;
				}
				ifp->scriptpath = qstrdup(cfl->ptr);
				if (ifp->scriptpath == NULL)
					goto bad;
				if (ifp->scriptpath[0] != '/') {
					bb_info_msg("script must be an absolute path");
					return -1;
				}
				break;
			default:
 bad:
				bb_error_msg("%s:%d invalid configuration",
							client6_config.conffile, cfl->line);
				return -1;
			}
		} /* for(cnl->sublist) */
	} /* for(cf_list) */

	return 0;
}

static int configure_ia(void)
{
	struct cf_list *iap;
	struct ia_conf *iac = NULL;

	for (iap = client6_config.config_list; iap; iap = iap->next) {
		struct cf_list *cfl;

		if (iap->type != DHCPOPT_IA_PD && iap->type != DHCPOPT_IA_NA)
			continue;
		log3("configure %s '%s'", dhcp6optstr(iap->type), iap->ptr);

		iac = xzalloc(sizeof(struct ia_conf));
		/* common initialization */
		iac->type = iap->type;
		iac->iaid = (uint32_t)atoi(iap->ptr);
		TAILQ_INIT(&iac->iadata);
		TAILQ_INSERT_TAIL(&client6_config.ia_allconflist, iac, link);

		/* IA-type specific initialization */
		switch (iap->type) {
		case DHCPOPT_IA_PD:
			TAILQ_INIT(&iac->iapd_pif_list);
			/* Fall through */
		case DHCPOPT_IA_NA:
			TAILQ_INIT(&iac->prefix_list);
			break;
		default:
			/* not happen - for compiler only*/
			break;
		}

		/* set up parameters for the IA */
		for (cfl = iap->sublist; cfl; cfl = cfl->next) {

			switch (iap->type) {
			case DHCPOPT_IA_PD:
				switch (cfl->type) {
				case IACONF_PIF:
					if (add_pd_pif(iac, cfl))
						return -1;
					break;
				case IACONF_PREFIX:
					if (add_prefix6(&iac->prefix_list, iap->type,
						 DHCP6_LISTVAL_PREFIX6,
						 (struct dhcp6_prefix *)cfl->ptr))
						return -1;
					break;
				default:
					goto bad;
				}
				break;
			case DHCPOPT_IA_NA:
				switch (cfl->type) {
				case IACONF_ADDR:
					if (add_prefix6(&iac->prefix_list, iap->type,
						 DHCP6_LISTVAL_STATEFULADDR6,
						 (struct dhcp6_prefix *)cfl->ptr))
						return -1;
					break;
				default:
					goto bad;
				}
				break;
			default:
				/* not happen - for compiler only*/
 bad:
				bb_error_msg("%s:%d invalid configuration",
					    client6_config.conffile, cfl->line);
				return -1;
			}
		} /* for(iap->sublist) */
	} /* for(cf_list) */

	return 0;
}

static int add_pd_pif(struct ia_conf *iac, struct cf_list *cfl0)
{
	struct cf_list *cfl;
	struct prefix_ifconf *pif;

	/* duplication check */
	TAILQ_FOREACH(pif, &iac->iapd_pif_list, link) {
		if (strcmp(pif->ifname, cfl0->ptr) == 0) {
			bb_info_msg("%s:%d duplicated prefix interface %s",
			    client6_config.conffile, cfl0->line, cfl0->ptr);
			return 0; /* ignore it */
		}
	}

	pif = xzalloc(sizeof(*pif));

	/* validate and copy ifname */
	if (if_nametoindex(cfl0->ptr) == 0) {
		bb_perror_msg("%s:%d invalid interface(%s)",
		    client6_config.conffile, cfl0->line, cfl0->ptr);
		goto bad;
	}
	pif->ifname = xstrdup(cfl0->ptr);

	pif->ifid_len = IFID_LEN_DEFAULT;
	pif->sla_len = SLA_LEN_DEFAULT;
	if (get_default_ifid(pif)) {
		bb_error_msg("can't get IF ID for %s", pif->ifname);
		goto bad;
	}

	for (cfl = cfl0->sublist; cfl; cfl = cfl->next) {
		switch (cfl->type) {
		case IFPARAM_SLA_ID:
			pif->sla_id = xatou32(cfl->ptr);
			break;
		case IFPARAM_SLA_LEN:
			pif->sla_len = xatou(cfl->ptr);
			if (pif->sla_len < 0 || pif->sla_len > 128) {
				bb_error_msg("%s:%d invalid SLA length: %d",
				    client6_config.conffile, cfl->line, pif->sla_len);
				goto bad;
			}
			break;
		default:
			bb_error_msg("%s:%d invalid configuration",
			    client6_config.conffile, cfl->line);
			goto bad;
		}
	}

	TAILQ_INSERT_TAIL(&iac->iapd_pif_list, pif, link);
	return 0;

 bad:
	free(pif->ifname);
	free(pif);
	return -1;
}

#if ENABLE_FEATURE_DHCP6_AUTH
static int configure_keys(void)
{
	struct cf_list *key;
	struct keyinfo *kinfo;
	char *expire = NULL;

	for (key = client6_config.config_list; key; key = key->next) {
		struct cf_list *cfl;

		if (key->type != DECL_KEYINFO)
			continue;
		log3("configure keys '%s'", key->ptr);

		kinfo = xzalloc(sizeof(*kinfo));
		kinfo->next = client6_config.key_list;
		kinfo->name = xstrdup(key->ptr);
		kinfo->keyid = -1;
		client6_config.key_list = kinfo;

		for (cfl = key->sublist; cfl; cfl = cfl->next) {
			switch (cfl->type) {
			case KEYPARAM_REALM:
				if (kinfo->realm.dv_buf != NULL) {
					bb_info_msg("%s:%d duplicated %s for key %s (ignored)",
						client6_config.conffile, cfl->line, "realm", key->ptr);
					continue;
				}
				kinfo->realm.dv_buf = qstrdup(cfl->ptr);
				if (kinfo->realm.dv_buf == NULL)
					goto wrong;
				kinfo->realm.dv_len = strlen(kinfo->realm.dv_buf);
				break;
			case KEYPARAM_KEYID:
				if (kinfo->keyid != -1) {
					bb_info_msg("%s:%d duplicated %s for key %s (ignored)",
						client6_config.conffile, cfl->line, "key ID", key->ptr);
					continue;
				}
				kinfo->keyid = xatou32(cfl->ptr);
				break;
			case KEYPARAM_SECRET:
				/* duplicate check */
				if (kinfo->secret != NULL) {
					bb_info_msg("%s:%d duplicated %s for key %s (ignored)",
						client6_config.conffile, cfl->line, "secret", key->ptr);
					continue; /* ignored */
				}

				{
					int len;
					unsigned char *out_tail;
					const char *in_tail;

					/* convert base64 string to binary secret */
					len = strlen(cfl->ptr);
					len = (len >> 2) + (len >> 1) + 1; /* 3/4x + 1 */
					kinfo->secret = xmalloc(len);
					out_tail = kinfo->secret;
					in_tail = decode_base64((char **)&out_tail, cfl->ptr);
					if (*in_tail != '\0') {
						free(kinfo->secret);
						goto wrong;
					}
					kinfo->secretlen = out_tail - kinfo->secret;
				}
				break;
			case KEYPARAM_EXPIRE:
				if (expire != NULL) {
					bb_info_msg("%s:%d duplicated %s for key %s (ignored)",
						client6_config.conffile, cfl->line, "expire", key->ptr);
					continue;
				}
				expire = qstrdup(cfl->ptr);
				if (expire == NULL)
					goto wrong;
				break;
			default:
 wrong:
				bb_error_msg("%s:%d invalid parameter for %s",
				    client6_config.conffile, cfl->line, key->ptr);
				goto bad;
			}
		} /* for(key->sublist) */

		/* check for mandatory parameters or use default */
		if (kinfo->realm.dv_buf == NULL) {
			bb_error_msg("%s not specified for key %s",	"realm", key->ptr);
			goto bad;
		}
		if (kinfo->keyid == -1) {
			bb_error_msg("%s not specified for key %s",	"key ID", key->ptr);
			goto bad;
		}
		if (kinfo->secret == NULL) {
			bb_error_msg("%s not specified for key %s",	"secret", key->ptr);
			goto bad;
		}
		if (expire != NULL) {
			if (strcmp(expire, "forever") != 0) {
				time_t now, expire_time;
				struct tm lt;

				time(&now);
				parse_datestr(expire, &lt);
				expire_time = mktime(&lt);
				if (expire_time < now) {
					bb_error_msg("past expiration time specified: %s",
					    expire);
					goto bad;
				}

				kinfo->expire = expire_time;
			}
		}
	} /* for(cf_list) */

	return 0;

 bad:
	free(expire);
	return -1;
}

static int configure_authinfo(void)
{
	struct cf_list *auth;
	struct authinfo *ainfo;

	for (auth = client6_config.config_list; auth; auth = auth->next) {
		struct cf_list *cfl;

		if (auth->type != DECL_AUTHINFO)
			continue;
		log3("configure auth '%s'", auth->ptr);

		ainfo = xzalloc(sizeof(*ainfo));
		ainfo->next = client6_config.auth_list;
		client6_config.auth_list = ainfo;
		ainfo->protocol = -1;
		ainfo->algorithm = -1;
		ainfo->rdm = -1;
		ainfo->name = xstrdup(auth->ptr);

		for (cfl = auth->sublist; cfl; cfl = cfl->next) {
			switch (cfl->type) {
			case AUTHPARAM_PROTO:
				if (ainfo->protocol != -1) {
					bb_info_msg("%s:%d duplicated %s for auth %s (ignored)",
					    client6_config.conffile, cfl->line,
					    "protocol", auth->ptr);
					continue; /* ignored */
				}
				if (strcasecmp(cfl->ptr, "delayed") == 0)
					ainfo->protocol = DHCP6_AUTHPROTO_DELAYED;
#if 0 /* ENABLE_DHCP6S */
				else if (strcasecmp(cfl->ptr, "reconfig") == 0)
					ainfo->protocol = DHCP6_AUTHPROTO_RECONFIG;
#endif
				else
					goto wrong;
				break;
			case AUTHPARAM_ALG:
				if (ainfo->algorithm != -1) {
					bb_info_msg("%s:%d duplicated %s for auth %s (ignored)",
					    client6_config.conffile, cfl->line,
					    "algorithm", auth->ptr);
					continue; /* ignored */
				}
				if (strcasecmp(cfl->ptr, "hmac-md5") == 0 ||
				    strcasecmp(cfl->ptr, "hmacmd5") == 0)
					ainfo->algorithm = DHCP6_AUTHALG_HMACMD5;
				else
					goto wrong;
				break;
			case AUTHPARAM_RDM:
				if (ainfo->rdm != -1) {
					bb_info_msg("%s:%d duplicated %s for auth %s (ignored)",
					    client6_config.conffile, cfl->line,
					    "RDM", auth->ptr);
					continue; /* ignored */
				}
				if (strcasecmp(cfl->ptr, "monocounter") == 0)
					ainfo->rdm = DHCP6_AUTHRDM_MONOCOUNTER;
				else
					goto wrong;
				break;
			case AUTHPARAM_KEY:
				bb_info_msg("%s:%d auth info specific keys "
				    "are not supported",
				    client6_config.conffile, cfl->line);
				break;
			default:
 wrong:
				bb_error_msg("%s:%d invalid auth info parameter for %s",
				    client6_config.conffile, cfl->line, auth->ptr);
				goto bad;
			}
		} /* for(auth->sublist) */

		/* check for mandatory parameters and consistency */
		switch (ainfo->protocol) {
		case -1:
			bb_error_msg("auth protocol not specified for %s", auth->ptr);
			goto bad;
		case DHCP6_AUTHPROTO_DELAYED:
			break;
		case DHCP6_AUTHPROTO_RECONFIG:
			goto bad;
			break;
		}
		if (ainfo->algorithm == -1)
			ainfo->algorithm = DHCP6_AUTHALG_HMACMD5;
		if (ainfo->rdm == -1)
			ainfo->rdm = DHCP6_AUTHRDM_MONOCOUNTER;
	}

	return 0;

 bad:
	/* there is currently nothing special to recover the error */
	return -1;
}

static struct authinfo *find_authinfo(struct authinfo *head, char *name)
{
	struct authinfo *ainfo;

	for (ainfo = head; ainfo; ainfo = ainfo->next) {
		if (strcmp(ainfo->name, name) == 0)
			return ainfo;
	}

	return NULL;
}
#endif /* ENABLE_FEATURE_DHCP6_AUTH */

/* Strip quotes from string */
static char* FAST_FUNC qstrdup(char *qstr)
{
	size_t len;
	char *dst;

	len = strlen(qstr);
	if (qstr[0] != '"' || len < 2 || qstr[len - 1] != '"')
		return NULL;

	dst = xmemdup(qstr + 1, len - 1);
	dst[len - 2] = '\0';

	return dst;
}

/* we currently only construct EUI-64 based interface ID */
static int get_default_ifid(struct prefix_ifconf *pif)
{
	uint8_t buf[16];

	if (pif->ifid_len < 64) {
		bb_error_msg("ID length too short");
		return -1;
	}

	/* Such interface must have a mac */
	if (udhcp_read_interface(pif->ifname, NULL, NULL, buf, NULL))
		return -2;

	memset(pif->ifid, 0, sizeof(pif->ifid));
	pif->ifid[8]  = buf[0] ^ 0x02; /* reverse the u/l bit*/
	pif->ifid[9]  = buf[1];
	pif->ifid[10] = buf[2];
	pif->ifid[11] = 0xff;
	pif->ifid[12] = 0xfe;
	pif->ifid[13] = buf[3];
	pif->ifid[14] = buf[4];
	pif->ifid[15] = buf[5];

	return 0;
}

void clear_ifconf(int destroy)
{
	struct dhcp6_if *ifp = &client6_config.dhcp6c_if;

	ifp->send_flags = 0;
	ifp->allow_flags = 0;

	ifp->server_pref = -1;
#if ENABLE_FEATURE_DHCP6_AUTH
	ifp->authproto = -1;
	ifp->authalgorithm = -1;
	ifp->authrdm = -1;
#endif
	dhcp6_clear_list(&ifp->reqopt_list);

	clear_iaconf(&ifp->iaconf_list);

	free(ifp->scriptpath);
	ifp->scriptpath = NULL;

	free(ifp->pool.name);
	ifp->pool.name = NULL;

	if (ENABLE_FEATURE_CLEAN_UP && destroy) {
		free(ifp->ifname);
	}
}

static void clear_pd_pif(struct ia_conf *iac)
{
	struct prefix_ifconf *pif, *pif_next;

	for (pif = TAILQ_FIRST(&iac->iapd_pif_list); pif; pif = pif_next) {
		pif_next = TAILQ_NEXT(pif, link);

		free(pif->ifname);
		free(pif);
	}

	dhcp6_clear_list(&iac->prefix_list);
}

static void clear_iaconf(struct ia_conflist *ialist)
{
	struct ia_conf *iac;

	while ((iac = TAILQ_FIRST(ialist)) != NULL) {
		TAILQ_REMOVE(ialist, iac, link);

		switch (iac->type) {
		case DHCPOPT_IA_PD:
			if (!TAILQ_EMPTY(&iac->iadata)) {
				bb_error_msg("iadata not empty");
			}
			clear_pd_pif(iac);
			break;
		case DHCPOPT_IA_NA:
			break;
		default:
			break;
		}
		free(iac);
	}
}

#if ENABLE_FEATURE_DHCP6_AUTH
static void clear_keys(struct keyinfo *klist)
{
	struct keyinfo *key = klist, *key_next;

	while (key) {
		key_next = key->next;

		free(key->name);
		dhcp6_vbuf_free(&key->realm);
		free(key->secret);
		free(key);

		key = key_next;
	}
}

static void clear_authinfo(struct authinfo *alist)
{
	struct authinfo *auth = alist, *auth_next;

	while (auth) {
		auth_next = auth->next;
		free(auth);

		auth = auth_next;
	}
}
#endif /* ENABLE_FEATURE_DHCP6_AUTH */

static int add_options(int opcode, struct dhcp6_if *ifp, struct cf_list *cfl0)
{
	struct cf_list *cfl;
	struct ia_conf *iac;
	uint32_t iaid;
#if ENABLE_FEATURE_DHCP6_AUTH
	struct authinfo *ainfo;
#endif

	for (cfl = cfl0; cfl; cfl = cfl->next) {
		switch (cfl->type) {
		case DHCPOPT_RAPID_COMMIT:
			switch (opcode) {
			case DECL_SEND:
				ifp->send_flags |= DHCIFF_RAPID_COMMIT;
				break;
#if 0 /* ENABLE_DHCP6S */
			case DECL_ALLOW:
				ifp->allow_flags |= DHCIFF_RAPID_COMMIT;
				break;
#endif
			default:
				goto inv_op;
			}
			break;
#if ENABLE_FEATURE_DHCP6_AUTH
		case DHCPOPT_AUTHINFO:
			if (opcode != DECL_SEND)
				goto inv_op;
			ainfo = find_authinfo(client6_config.auth_list, cfl->ptr);
			if (ainfo == NULL) {
				bb_error_msg("%s:%d auth info (%s) is not defined",
				    client6_config.conffile, cfl->line, cfl->ptr);
				return -1;
			}
			if (ifp->authproto != -1) {
				bb_error_msg("%s:%d duplicated auth info for %s",
				    client6_config.conffile, cfl->line, ifp->ifname);
				return -1;
			}
			ifp->authproto = ainfo->protocol;
			ifp->authalgorithm = ainfo->algorithm;
			ifp->authrdm = ainfo->rdm;
			break;
#endif /* ENABLE_FEATURE_DHCP6_AUTH */
		case DHCPOPT_IA_PD:
		case DHCPOPT_IA_NA:
			if (opcode != DECL_SEND)
				goto inv_op;
			iaid = xatou32(cfl->ptr);
			iac = find_iaconf(&ifp->iaconf_list, cfl->type, iaid);
			if (iac != NULL) {
				bb_error_msg("%s:%d duplicated %s (%u)",
					client6_config.conffile, cfl->line,
					dhcp6optstr(cfl->type), iaid);
				return -1;
			}
			iac = find_iaconf(&client6_config.ia_allconflist, cfl->type, iaid);
			if (iac == NULL) {
				bb_error_msg("%s:%d %s (%u) is not defined",
					client6_config.conffile, cfl->line,
					dhcp6optstr(cfl->type), iaid);
				return -1;
			}

			TAILQ_REMOVE(&client6_config.ia_allconflist, iac, link);
			TAILQ_INSERT_TAIL(&ifp->iaconf_list, iac, link);

			break;
		case DHCPOPT_SIP:
		case DHCPOPT_SIPNAME:
		case DHCPOPT_DNS:
		case DHCPOPT_DNSNAME:
		case DHCPOPT_SNTP:
		case DHCPOPT_NIS:
		case DHCPOPT_NISNAME:
		case DHCPOPT_NISP:
		case DHCPOPT_NISPNAME:
		case DHCPOPT_BCMCS:
		case DHCPOPT_BCMCSNAME:
		case DHCPOPT_REFRESHTIME:
			if (opcode != DECL_REQUEST)
				goto inv_op;
			if (dhcp6_find_listval(&ifp->reqopt_list,
					DHCP6_LISTVAL_NUM, &cfl->type, 0) != NULL) {
				bb_info_msg("duplicated request option (%s)",
				    dhcp6optstr(cfl->type));
				goto next; /* ignore it */
			}
			if (dhcp6_add_listval(&ifp->reqopt_list, 0,
					DHCP6_LISTVAL_NUM, &cfl->type, NULL) == NULL) {
				bb_error_msg("can't add an option (%s)",
					dhcp6optstr(cfl->type));
				return -1;
			}
			break;
		default:
			bb_error_msg("%s:%d unsupported option (%s)",
				client6_config.conffile, cfl->line,
				dhcp6optstr(cfl->type));
			return -1;
		}

 next:
		;
	}

	return 0;

 inv_op:
	bb_error_msg("%s:%d invalid operation (%d) for option (%s)",
		    client6_config.conffile, cfl->line,
		    opcode, dhcp6optstr(cfl->type));
	return -1;
}

static int add_prefix6(struct dhcp6_list *head, int type, int lvtype,
		   struct dhcp6_prefix *prefix)
{
	struct in6_addr oldaddr;
	const char *name = dhcp6optstr(type);
	char p[INET6_ADDRSTRLEN];

	oldaddr = prefix->addr;

	/* additional validation of parameters */
	if (prefix->plen < 4 || prefix->plen > 128) {
		bb_error_msg("invalid prefix length: %d", prefix->plen);
		return -1;
	}
	/* clear trailing bits */
	prefix6_mask(&prefix->addr, prefix->plen);
	sprint_nip6(p, (const uint8_t *)&prefix->addr);
	if (!IN6_ARE_ADDR_EQUAL(&oldaddr, &prefix->addr)) {
		char p0[INET6_ADDRSTRLEN];

		sprint_nip6(p0, (const uint8_t *)&oldaddr);
		bb_info_msg("prefix %s/%d for %s "
		    "has a trailing garbage. It should be %s/%d",
		    p0, prefix->plen, name, p, prefix->plen);
		/* ignore the error */
	}

	/* avoid invalid prefix addresses */
	if (IN6_IS_ADDR_MULTICAST(&prefix->addr) ||
	    IN6_IS_ADDR_LINKLOCAL(&prefix->addr) ||
	    IN6_IS_ADDR_SITELOCAL(&prefix->addr)) {
		bb_error_msg("invalid prefix address: %s", p);
		return -1;
	}

	/* prefix duplication check */
	if (dhcp6_find_listval(head, lvtype, prefix, 0)) {
		if (lvtype == DHCP6_LISTVAL_PREFIX6) {
			bb_info_msg("duplicated prefix: %s/%d for %s",
			    p, prefix->plen, name);
		} else {
			bb_info_msg("duplicated address: %s for %s", p, name);
		}
		return -1;
	}

	/* validation about relationship of pltime and vltime */
	if (prefix->vltime != DHCP6_DURATION_INFINITE &&
		 (prefix->pltime == DHCP6_DURATION_INFINITE ||
		 prefix->pltime > prefix->vltime)) {
		bb_info_msg("%s/%d has larger preferred lifetime than valid lifetime",
				p, (lvtype == DHCP6_LISTVAL_PREFIX6) ? prefix->plen : 128);
		return -1;
	}

	/* insert the new prefix to the chain */
	if (dhcp6_add_listval(head, type, lvtype, prefix, NULL) == NULL) {
		return -1;
	}

	return 0;
}

struct ia_conf *find_iaconf(const struct ia_conflist *head, dh6cnfopts_t type,
			uint32_t iaid)
{
	struct ia_conf *iac;

	TAILQ_FOREACH(iac, head, link) {
		if (iac->type == type && iac->iaid == iaid)
			return iac;
	}

	return NULL;
}

struct dhcp6_prefix *find_prefix6(struct dhcp6_list *list,
				struct dhcp6_prefix *prefix)
{
	struct dhcp6_listval *lv;

	TAILQ_FOREACH(lv, list, link) {
		if (lv->val_prefix6.plen == prefix->plen &&
		    IN6_ARE_ADDR_EQUAL(&lv->val_prefix6.addr, &prefix->addr)) {
			return (&lv->val_prefix6);
		}
	}
	return NULL;
}

#if ENABLE_FEATURE_DHCP6_AUTH
struct keyinfo *find_key(struct dhcp6_vbuf *realm, uint32_t id)
{
	struct keyinfo *key;

	for (key = client6_config.key_list; key; key = key->next) {
		if (key->keyid == id && dhcp6_vbuf_cmp(&key->realm, realm) == 0) {
			return key;
		}
	}

	return NULL;
}
#endif

////////////////////////////////////////////////////////////////////////


/* supplement routines for configuration */
static int add_cflevel0(dh6cnfopts_t type, const char *name, int lineno)
{
	struct cf_list *new;
	struct cf_list *n = client6_config.config_list;

	MAKE_CFLIST(new, type, xstrdup(name), NULL);

	/* check for duplicated configuration */
	while (n) {
		if (n->type == new->type && strcmp(n->ptr, new->ptr) == 0) {
			bb_info_msg("%s:%d duplicated %s name: %s (ignored)",
				client6_config.conffile, lineno, dhcp6optstr(type), name);
			cleanup_cflist(new);
			return 0;
		}
		n = n->next;
	}

	/* insert at first pos of list */
	new->next = client6_config.config_list;
	if (client6_config.config_list)
		new->tail = client6_config.config_list->tail;
	client6_config.config_list = new;

	return 0;
}

static int add_cfparam(struct cf_list *new)
{
	struct cf_list *headp = client6_config.config_list;

	if (headp->sublist == NULL) {
		new->tail = new;
		new->next = NULL;
		headp->sublist = new;
	} else {
		/* add to the end of list */
		headp->sublist->tail->next = new;
		headp->sublist->tail = new->tail;
	}
	return 0;
}

static int add_cfsubparam(struct cf_list *new, struct cf_list *parent)
{
	if (parent->sublist == NULL) {
		new->tail = new;
		new->next = NULL;
		parent->sublist = new;
	} else {
		/* add to the end of list */
		parent->sublist->tail->next = new;
		parent->sublist->tail = new->tail;
	}
	return 0;
}

static void cleanup_cflist(struct cf_list *cfl)
{
	struct cf_list *cfl_next;

	while (cfl) {
		cfl_next = cfl->next;
		free(cfl->ptr);
		if (cfl->sublist)
			cleanup_cflist(cfl->sublist);
		free(cfl);

		cfl = cfl_next;
	}
}

struct dhcp_cf_options {
	const char *optname;
	const int optval;
	const uint8_t paramt; /* 0-none, 1-string, 2-integer */
};

static const struct dhcp_cf_options dhcp_option_list[] = {
    { "rapid-commit",             DHCPOPT_RAPID_COMMIT,  0 },
    { "ia-pd",                    DHCPOPT_IA_PD,         2 },
    { "ia-na",                    DHCPOPT_IA_NA,         2 },
    { "authentication",           DHCPOPT_AUTHINFO,      1 },
    { "domain-name-servers",      DHCPOPT_DNS,           0 },
    { "ntp-servers",              DHCPOPT_SNTP,          0 },
    { "sip-server-address",       DHCPOPT_SIP,           0 },
    { "nis-server-address",       DHCPOPT_NIS,           0 },
    { "nisp-server-address",      DHCPOPT_NISP,          0 },
    { "bcmcs-server-address",     DHCPOPT_BCMCS,         0 },
#if ENABLE_FEATURE_UDHCP_RFC3397
    { "domain-name",              DHCPOPT_DNSNAME,       0 },
    { "sip-server-domain-name",   DHCPOPT_SIPNAME,       0 },
    { "nis-domain-name",          DHCPOPT_NISNAME,       0 },
    { "nisp-domain-name",         DHCPOPT_NISPNAME,      0 },
    { "bcmcs-server-domain-name", DHCPOPT_BCMCSNAME,     0 },
#endif
};

static int FAST_FUNC create_iface(int lineno, int kw_ind UNUSED_PARAM, char **varr, int cnt)
{
	if (cnt < 2)
		return -2;

	if (add_cflevel0(DECL_IFACE, varr[1], lineno))
		return -1;

	return 0;
}

static int FAST_FUNC create_assoc(int lineno, int kw_ind UNUSED_PARAM, char **varr, int cnt)
{
	const char *id;
	dh6cnfopts_t iatype;

	if (cnt < 2)
		return -2;

	if (cnt >=3 && strcmp(varr[2], "{"))
		id = varr[2];
	else
		id = "0";

	if (strcasecmp(varr[1], "pd") == 0)
		iatype = DHCPOPT_IA_PD;
	else if (strcasecmp(varr[1], "na") == 0)
		iatype = DHCPOPT_IA_NA;
	else
		return -3;

	if (add_cflevel0(iatype, id, lineno))
		return -1;

	return 0;
}

static int FAST_FUNC parse_iface(int lineno, int kw_ind, char **varr, int cnt)
{
	struct cf_list *l;
	int i, j;

	switch (kw_ind) {
	case 0: /* send */
		MAKE_CFLIST(l, DECL_SEND, NULL, NULL);
		add_cfparam(l);
		break;
	case 1: /* request */
		MAKE_CFLIST(l, DECL_REQUEST, NULL, NULL);
		add_cfparam(l);
		break;
	case 2: /* script */
		if (cnt < 2)
			return -2;
		MAKE_CFLIST(l, DECL_SCRIPT, xstrdup(varr[1]), NULL);
		add_cfparam(l);
		return 0;
	case 3: /* information-only */
		MAKE_CFLIST(l, DECL_INFO_ONLY, NULL, NULL);
		add_cfparam(l);
		return 0;
	}

	/* add subparams */
	for (i = 1; i < cnt; i++) {

		for (j = 0; j < ARRAY_SIZE(dhcp_option_list); j++) {
			if (strcasecmp(dhcp_option_list[j].optname, varr[i]) == 0) {

			MAKE_CFLIST(l, dhcp_option_list[j].optval, xstrdup(varr[i+1]), NULL);
			if (dhcp_option_list[j].paramt && (i < cnt - 1))
				i++;
			if (add_cfsubparam(l, client6_config.config_list->sublist->tail))
				return -1;
			break;

			}
		}

		if (j >= ARRAY_SIZE(dhcp_option_list))
		return -3;

	}

	return 0;
}

static int FAST_FUNC create_prefiff(int lineno, int kw_ind UNUSED_PARAM, char **varr, int cnt)
{
	struct cf_list *ifl;

	if (cnt < 2)
		return -2;

	MAKE_CFLIST(ifl, IACONF_PIF, xstrdup(varr[1]), NULL);

	if (add_cfparam(ifl))
		return -1;

	return 0;
}

static int FAST_FUNC parse_prefiff(int lineno, int kw_ind, char **varr, int cnt)
{
	struct cf_list *l;

	if (cnt < 2)
		return -2;

	switch (kw_ind) {
	case 0: /* sla-id */
		MAKE_CFLIST(l, IFPARAM_SLA_ID, xstrdup(varr[1]), NULL);
		break;
	case 1: /* sla-len */
		MAKE_CFLIST(l, IFPARAM_SLA_LEN, xstrdup(varr[1]), NULL);
		break;
	default:
		return -1;
	}
	if (add_cfsubparam(l, client6_config.config_list->sublist->tail))
		return -1;

	return 0;
}

static int FAST_FUNC parse_assoc(int lineno, int kw_ind, char **varr, int cnt)
{
	struct cf_list *l;
	struct dhcp6_prefix *pconf;

	if (cnt < 2)
		return -2;

	switch (kw_ind) {
		case 0: /* address */
			MAKE_CFLIST(l, IACONF_ADDR, NULL, NULL);
			break;
		case 1: /* prefix */
			MAKE_CFLIST(l, IACONF_PREFIX, NULL, NULL);
			break;
		default:
			return -1;
	}
	pconf = xzalloc(sizeof(*pconf));
	if (inet_pton(AF_INET6, varr[1], &pconf->addr) != 1) {
		bb_error_msg("invalid IPv6 address: %s", varr[1]);
				free(pconf);
				return -1;
	}
	/* validate other parameters later */
	pconf->plen = 128; /* XXX this field is ignored */
	if (cnt < 3) {
		pconf->pltime = DHCP6_DURATION_INFINITE;
	} else {
		pconf->pltime = xatoul(varr[2]);
		if (cnt < 4)
			pconf->vltime = DHCP6_DURATION_INFINITE;
		else
			pconf->vltime = xatoul(varr[3]);
	}
	l->ptr = (void *)pconf;

	if (add_cfparam(l))
		return -1;

	return 0;
}

#if ENABLE_FEATURE_DHCP6_AUTH
static int FAST_FUNC create_auth(int lineno, int kw_ind UNUSED_PARAM, char **varr, int cnt)
{
	if (cnt < 2)
		return -2;

	if (add_cflevel0(DECL_AUTHINFO, varr[1], lineno))
		return -1;

	return 0;
}

static int FAST_FUNC create_keyinfo(int lineno, int kw_ind UNUSED_PARAM, char **varr, int cnt)
{
	if (cnt < 2)
		return -2;

	if (add_cflevel0(DECL_KEYINFO, varr[1], lineno))
		return -1;

	return 0;
}

static int FAST_FUNC parse_auth(int lineno, int kw_ind, char **varr, int cnt)
{
	struct cf_list *l;

	if (cnt < 2)
		return -2;

	switch (kw_ind) {
		case 0: /* protocol */
			MAKE_CFLIST(l, AUTHPARAM_PROTO, xstrdup(varr[1]), NULL);
			break;
		case 1: /* algorithm */
			MAKE_CFLIST(l, AUTHPARAM_ALG, xstrdup(varr[1]), NULL);
			break;
		case 2: /* rdm */
			MAKE_CFLIST(l, AUTHPARAM_RDM, xstrdup(varr[1]), NULL);
			break;
		default:
			return -1;
	}
	if (add_cfparam(l))
		return -1;

	return 0;
}

static int FAST_FUNC parse_keyinfo(int lineno, int kw_ind, char **varr, int cnt)
{
	struct cf_list *l;

	if (cnt < 2)
		return -2;

	switch (kw_ind) {
		case 0: /* realm */
			MAKE_CFLIST(l, KEYPARAM_REALM, xstrdup(varr[1]), NULL);
			break;
		case 1: /* keyid */
			MAKE_CFLIST(l, KEYPARAM_KEYID, xstrdup(varr[1]), NULL);
			break;
		case 2: /* secret */
			MAKE_CFLIST(l, KEYPARAM_SECRET, qstrdup(varr[1]), NULL);
			break;
		case 3: /* expire */
			MAKE_CFLIST(l, KEYPARAM_EXPIRE, xstrdup(varr[1]), NULL);
			break;
		default:
			return -1;
	}
	if (add_cfparam(l))
		return -1;

	return 0;
}
#endif /* ENABLE_FEATURE_DHCP6_AUTH */

struct config_keyword;

struct config_keyword {
	const char *keyword;
	const struct config_keyword *next_level;

	int (*handler)(int lineno, int kw_ind, char **varr, int cnt) FAST_FUNC;
};

static const struct config_keyword interface_kw[] = {
 { "send",             NULL,          &parse_iface    },
 { "request",          NULL,          &parse_iface    },
 { "script",           NULL,          &parse_iface    },
 { "information-only", NULL,          &parse_iface    },
 { NULL,               NULL,          NULL },
};

static const struct config_keyword prefiff_kw[] = {
 { "sla-id",           NULL,          &parse_prefiff  },
 { "sla-len",          NULL,          &parse_prefiff  },
 { NULL,               NULL,          NULL },
};

static const struct config_keyword assoc_kw[] = {
 { "address",          NULL,          &parse_assoc    },
 { "prefix",           NULL,          &parse_assoc    },
 { "prefix-interface", prefiff_kw,    &create_prefiff },
 { NULL,               NULL,          NULL },
};

#if ENABLE_FEATURE_DHCP6_AUTH
static const struct config_keyword auth_kw[] = {
 { "protocol",         NULL,          &parse_auth     },
 { "algorithm",        NULL,          &parse_auth     },
 { "rdm",              NULL,          &parse_auth     },
 { NULL,               NULL,          NULL },
};

static const struct config_keyword keyinfo_kw[] = {
 { "realm",            NULL,          &parse_keyinfo  },
 { "keyid",            NULL,          &parse_keyinfo  },
 { "secret",           NULL,          &parse_keyinfo  },
 { "expire",           NULL,          &parse_keyinfo  },
 { NULL,               NULL,          NULL },
};
#endif /* ENABLE_FEATURE_DHCP6_AUTH */

static const struct config_keyword level0_kw[] = {
 { "interface",        interface_kw,  &create_iface   },
 { "id-assoc",         assoc_kw,      &create_assoc   },
#if ENABLE_FEATURE_DHCP6_AUTH
 { "authentication",   auth_kw,       &create_auth    },
 { "keyinfo",          keyinfo_kw,    &create_keyinfo },
#endif
 { NULL,               NULL,          NULL            },
};

TAILQ_HEAD(keyword6_list, keywords6);
struct keywords6 {
	TAILQ_ENTRY(keywords6) link;

	const struct config_keyword *kw;
};


int FAST_FUNC read_config6(const char *file)
{
	parser_t *parser;
	struct keyword6_list kwq_head;
	struct keywords6 *kl;
	unsigned i = 0, j, m, n;
#define MAX_TOKENS	15
	char *token[MAX_TOKENS];
	const struct config_keyword *req_next_level = NULL;
	int ret = 0;

	/* cleanup config int. structures */
	cleanup_cflist(client6_config.config_list);
	client6_config.config_list = NULL;
	TAILQ_INIT(&client6_config.ia_allconflist);
#if ENABLE_FEATURE_DHCP6_AUTH
	clear_keys(client6_config.key_list);
	client6_config.key_list = NULL;
	clear_authinfo(client6_config.auth_list);
	client6_config.auth_list = NULL;
#endif
	clear_ifconf(0);

	/* Phase I - parse & fill-up config int. structures */
	TAILQ_INIT(&kwq_head);
	kl = xzalloc(sizeof(*kl));
	kl->kw = level0_kw;
	TAILQ_INSERT_TAIL(&kwq_head, kl, link);
	parser = config_open(file);
	while ((n = config_read(parser, token, MAX_TOKENS, 1, "# \t,", PARSE_NORMAL))) {
		const struct config_keyword *k;
		int was_found;

		kl = TAILQ_LAST(&kwq_head, keyword6_list);
		i = 0;
 restart_scan:
		if (kl == NULL)
			goto scan_fail;

		k = kl->kw;
		j = 0;
		was_found = 0;
		/* Just skip alone ';' */
		if (strcmp(token[i], ";") == 0) {
			i++;
		}
		/* scan for ';' statement trailer */
		for (m = i; m < n; m++) {
			char *lc;

			if (strcmp(token[m], "{") == 0 || strcmp(token[m], ";") == 0)
				break;
			lc = last_char_is(token[m], ';');
			if (lc) {
				*lc = '\0';
				m++;
				break;
			}
		}
		/* m contains index of last parameter token */
		while (i < m && k[j].keyword != NULL) {
			if (strcasecmp(token[i], k[j].keyword) == 0) {
				if (k[j].handler != NULL) {
					log3("parse line(%d): token '%s' nargs=%d", parser->lineno, token[i], m-i);
					if ((*k[j].handler)(parser->lineno, j, &token[i], m-i) < 0)
						goto scan_fail;

					i = m;
				} else {
					i++;
				}
				req_next_level = k[j].next_level;
				was_found = 1;
				break;
			}
			j++;
		} /* while(keywords scan) */

		if (i >= n)	// EOL
			continue;

		if (req_next_level != NULL) {
			if (strcmp(token[i], "{") != 0) {
				goto scan_fail;
			}
			kl = xzalloc(sizeof(*kl));
			kl->kw = req_next_level;
			TAILQ_INSERT_TAIL(&kwq_head, kl, link);
			req_next_level = NULL;
			if (++i >= n)	// EOL
				continue;
			goto restart_scan;
		}
		if (strncmp(token[i], "}", 1) == 0) {
			TAILQ_REMOVE(&kwq_head, kl, link);
			free(kl);
			kl = TAILQ_LAST(&kwq_head, keyword6_list);
			if (++i >= n)	// EOL
				continue;
			goto restart_scan;
		}
		if (was_found)
			goto restart_scan;

 scan_fail:
		bb_error_msg("%s: can't parse line %u near token '%s'", file, parser->lineno, token[i]);
		break;
	} /* while(config_read) */
	config_close(parser);
	while ((kl = TAILQ_FIRST(&kwq_head)) != NULL) {
		TAILQ_REMOVE(&kwq_head, kl, link);
		free(kl);
	}

	/* Phase II - validations & cross-checks */
#if ENABLE_FEATURE_DHCP6_AUTH
	if (configure_keys())
		goto cf_fail;

	if (configure_authinfo())
		goto cf_fail;
#endif
	if (configure_ia())
		goto cf_fail;

	if (configure_interface())
		goto cf_fail;

	/* unused IA configurations */
	if (!TAILQ_EMPTY(&client6_config.ia_allconflist)) {
		bb_info_msg("some IA defined but not used!");
	}

 cf_ret:
	clear_iaconf(&client6_config.ia_allconflist);
	cleanup_cflist(client6_config.config_list);
	client6_config.config_list = NULL;
	return ret;

 cf_fail:
	ret = -1;
	goto cf_ret;
}
