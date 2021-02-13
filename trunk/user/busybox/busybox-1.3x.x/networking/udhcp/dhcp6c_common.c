/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 1998 and 1999 WIDE Project.
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
#include <netinet/in.h>
#include <netinet/if_ether.h>		// for hwid only
#include <limits.h>
#include <netdb.h>

#include "dhcp6c.h"


static int copyin_option(int, struct dhcp6opt *, struct dhcp6opt *,
    struct dhcp6_list *);
static int copy_option(uint16_t, uint16_t, void *, struct dhcp6opt **,
    struct dhcp6opt *, int *);

#if ENABLE_FEATURE_DHCP6_AUTH
static int copyout_auth(struct dhcp6_optinfo *optinfo,
		struct dhcp6opt **pp, struct dhcp6opt *bp, struct dhcp6opt *ep,
		int *totallenp);
# if defined CONFIG_UDHCP_DEBUG && CONFIG_UDHCP_DEBUG >= 2
static char *sprint_auth(struct dhcp6_optinfo *);
# endif
#endif

const struct dhcp6_option_typelen_t dhcp6_option_typelens[D6_OPT__MAX+1] = {
	[D6_OPT_CLIENTID]       = { DHCP6_LISTVAL_VBUF, 1                          },
	[D6_OPT_SERVERID]       = { DHCP6_LISTVAL_VBUF, 1                          },
	[D6_OPT_IA_NA]          = { DHCP6_LISTVAL_IA, sizeof(struct dhcp6opt_ia)-4 },
//	[D6_OPT_IA_TA]          = { 0, 0 },
	[D6_OPT_IAADDR]         = { DHCP6_LISTVAL_STATEFULADDR6, sizeof(struct dhcp6opt_ia_addr)-4 },
	[D6_OPT_ORO]            = { DHCP6_LISTVAL_NUM16, sizeof(uint16_t)          },
	[D6_OPT_PREFERENCE]     = { DHCP6_LISTVAL_NUM8, 1                          },
	[D6_OPT_ELAPSED_TIME]   = { DHCP6_LISTVAL_NUM16, sizeof(uint16_t)          },
	[D6_OPT_RELAY_MSG]      = { DHCP6_LISTVAL_VBUF, 1                          },
	[D6_OPT_AUTH]           = { 0, sizeof(struct dhcp6opt_auth)-4              },
//	[D6_OPT_UNICAST]		= { 0, 0 },
	[D6_OPT_STATUS_CODE]    = { DHCP6_LISTVAL_NUM16, sizeof(uint16_t)          },
	[D6_OPT_RAPID_COMMIT]   = { DHCP6_LISTVAL_NUM8, 0                          },
//	[D6_OPT_USER_CLASS]     = { 0, 0 },
//	[D6_OPT_VENDOR_CLASS]   = { 0, 0 },
//	[D6_OPT_VENDOR_OPTS]    = { 0, 0 },
	[D6_OPT_INTERFACE_ID]   = { DHCP6_LISTVAL_VBUF, 1                          },
//	[D6_OPT_RECONF_MSG]     = { 0, 0 },
	[D6_OPT_SIP_SERVER_D]   = { DHCP6_LISTVAL_VBUF, 1                          },
	[D6_OPT_SIP_SERVER_A]   = { DHCP6_LISTVAL_ADDR6, sizeof(struct in6_addr)   },
	[D6_OPT_DNS]            = { DHCP6_LISTVAL_ADDR6, sizeof(struct in6_addr)   },
	[D6_OPT_DNSNAME]        = { DHCP6_LISTVAL_VBUF, 1                          },
	[D6_OPT_IA_PD]          = { DHCP6_LISTVAL_IA, sizeof(struct dhcp6opt_ia)-4 },
	[D6_OPT_IA_PD_PREFIX]   = { DHCP6_LISTVAL_PREFIX6, sizeof(struct dhcp6opt_ia_pd_prefix)-4 },
	[D6_OPT_NIS_SERVERS]    = { DHCP6_LISTVAL_ADDR6, sizeof(struct in6_addr) },
	[D6_OPT_NISP_SERVERS]   = { DHCP6_LISTVAL_ADDR6, sizeof(struct in6_addr) },
	[D6_OPT_NIS_DOMAIN]     = { DHCP6_LISTVAL_VBUF, 1                          },
	[D6_OPT_NISP_DOMAIN]    = { DHCP6_LISTVAL_VBUF, 1                          },
	[D6_OPT_SNTP_SERVERS]   = { DHCP6_LISTVAL_ADDR6, sizeof(struct in6_addr)   },
	[D6_OPT_REFRESHTIME]    = { DHCP6_LISTVAL_NUM,	sizeof(uint32_t)           },
	[D6_OPT_BCMCS_SERVER_D] = { DHCP6_LISTVAL_VBUF, 1                          },
	[D6_OPT_BCMCS_SERVER_A] = { DHCP6_LISTVAL_ADDR6, sizeof(struct in6_addr)   },
//	[D6_OPT_GEOCONF_CIVIC]  = { 0, 0 },
//	[D6_OPT_REMOTE_ID]      = { 0, 0 },
//	[D6_OPT_SUBSCRIBER_ID]  = { 0, 0 },
//	[D6_OPT_CLIENT_FQDN]    = { 0, 0 },
	[D6_OPT_NTP_SERVER]     = { DHCP6_LISTVAL_ADDR6, sizeof(struct in6_addr)   },
};

int dhcp6_copy_list(struct dhcp6_list *dst, const struct dhcp6_list *src)
{
	struct dhcp6_listval *ent;

	TAILQ_FOREACH(ent, src, link) {
		if (dhcp6_add_listval(dst, ent->dh6optype, ent->lvtype,
		    &ent->uv, &ent->sublist) == NULL)
			return -1;
	}

	return 0;
}

void dhcp6_move_list(struct dhcp6_list *dst, struct dhcp6_list *src)
{
	struct dhcp6_listval *lv;

	while ((lv = TAILQ_FIRST(src)) != NULL) {
		TAILQ_REMOVE(src, lv, link);
		TAILQ_INSERT_TAIL(dst, lv, link);
	}
}

void dhcp6_clear_list(struct dhcp6_list *head)
{
	struct dhcp6_listval *lv;

	while ((lv = TAILQ_FIRST(head)) != NULL) {
		TAILQ_REMOVE(head, lv, link);
		dhcp6_clear_listval(lv);
	}
}

void dhcp6_clear_listval(struct dhcp6_listval *lv)
{
	dhcp6_clear_list(&lv->sublist);
	switch (lv->lvtype) {
	case DHCP6_LISTVAL_VBUF:
		dhcp6_vbuf_free(&lv->val_vbuf);
		break;
	default:		/* nothing to do */
		break;
	}
	free(lv);
}

/*
 * Note: this function only searches for the first entry that matches
 * VAL.  It also does not care about sublists.
 */
struct dhcp6_listval *dhcp6_find_listval(struct dhcp6_list *head,
		int type, const void *val, int option)
{
	struct dhcp6_listval *lv;

	TAILQ_FOREACH(lv, head, link) {
		if (option & MATCHLIST_DH6OPTYPE) {
			if (lv->dh6optype != type)
				continue;
		} else {
			if (lv->lvtype != type)
				continue;
		}

		switch (lv->lvtype) {
		case DHCP6_LISTVAL_NUM:
			if (lv->val_num == *(int32_t *)val)
				return lv;
			break;
		case DHCP6_LISTVAL_NUM16:
			if (lv->val_num16 == *(uint16_t *)val)
				return lv;
			break;
		case DHCP6_LISTVAL_NUM8:
			if (lv->val_num8 == *(uint8_t *)val)
				return lv;
			break;
		case DHCP6_LISTVAL_ADDR6:
			if (IN6_ARE_ADDR_EQUAL(&lv->val_addr6, (struct in6_addr *)val)) {
				return lv;
			}
			break;
		case DHCP6_LISTVAL_PREFIX6:
			if ((option & MATCHLIST_PREFIXLEN) &&
			    lv->val_prefix6.plen == ((struct dhcp6_prefix *)val)->plen) {
				return lv;
			} else if (IN6_ARE_ADDR_EQUAL(&lv->val_prefix6.addr,
			    &((struct dhcp6_prefix *)val)->addr) &&
			    lv->val_prefix6.plen == ((struct dhcp6_prefix *)val)->plen) {
				return lv;
			}
			break;
		case DHCP6_LISTVAL_STATEFULADDR6:
			if (IN6_ARE_ADDR_EQUAL(&lv->val_statefuladdr6.addr,
			    &((struct dhcp6_prefix *)val)->addr)) {
				return lv;
			}
			break;
		case DHCP6_LISTVAL_IA:
			if (lv->val_ia.iaid == ((struct dhcp6_ia *)val)->iaid) {
				return lv;
			}
			break;
		case DHCP6_LISTVAL_VBUF:
			if (dhcp6_vbuf_cmp(&lv->val_vbuf, (struct dhcp6_vbuf *)val) == 0) {
				return lv;
			}
			break;
		}
	}

	return NULL;
}

struct dhcp6_listval *dhcp6_add_listval(struct dhcp6_list *head,
		int dh6optype, dhcp6_listval_type_t lvtype,
		const void *val, struct dhcp6_list *sublist)
{
	struct dhcp6_listval *lv;

	lv = xzalloc(sizeof(*lv));
	lv->dh6optype = dh6optype;
	lv->lvtype = lvtype;
	TAILQ_INIT(&lv->sublist);

	switch (lvtype) {
	case DHCP6_LISTVAL_NUM:
		lv->val_num = *(int *)val;
		break;
	case DHCP6_LISTVAL_NUM16:
		lv->val_num16 = *(uint16_t *)val;
		break;
	case DHCP6_LISTVAL_NUM8:
		lv->val_num8 = *(uint8_t *)val;
		break;
	case DHCP6_LISTVAL_ADDR6:
		lv->val_addr6 = *(struct in6_addr *)val;
		break;
	case DHCP6_LISTVAL_PREFIX6:
		lv->val_prefix6 = *(struct dhcp6_prefix *)val;
		break;
	case DHCP6_LISTVAL_STATEFULADDR6:
		lv->val_statefuladdr6 = *(struct dhcp6_statefuladdr *)val;
		break;
	case DHCP6_LISTVAL_IA:
		lv->val_ia = *(struct dhcp6_ia *)val;
		break;
	case DHCP6_LISTVAL_VBUF:
		dhcp6_vbuf_copy(&lv->val_vbuf, (struct dhcp6_vbuf *)val);
		break;
	default:
		bb_error_msg("option '%s': unexpected list value type (%d)",
				dhcp6optstr(dh6optype), lvtype);
		free(lv);
		return NULL;
	}

	if (sublist)
		dhcp6_copy_list(&lv->sublist, sublist);

	TAILQ_INSERT_TAIL(head, lv, link);
	return lv;
}

void dhcp6_vbuf_copy(struct dhcp6_vbuf *dst, struct dhcp6_vbuf *src)
{
	dst->dv_len = src->dv_len;
	if (dst->dv_len == 0)
		return;

	dst->dv_buf = xmemdup(src->dv_buf, dst->dv_len);
}

void dhcp6_vbuf_free(struct dhcp6_vbuf *vbuf)
{
	free(vbuf->dv_buf);

	vbuf->dv_len = 0;
	vbuf->dv_buf = NULL;
}

struct dhcp6_timer *dhcp6_timer_add(struct dhcp6_timer *(*timeout)(void *),
				    void *timeodata)
{
	struct dhcp6_timer *newtimer;

	if (timeout == NULL) {
		log1("timeout function unspecified");
		return NULL;
	}

	newtimer = xzalloc(sizeof(*newtimer));
	newtimer->expire = timeout;
	newtimer->expire_data = timeodata;
	newtimer->t = ULLONG_MAX;

	LIST_INSERT_HEAD(&client6_config.timer_head, newtimer, link);

	return newtimer;
}

void dhcp6_timer_remove(struct dhcp6_timer **timer)
{
	LIST_REMOVE(*timer, link);
	free(*timer);
	*timer = NULL;
}

void dhcp6_timer_set(unsigned long long t, struct dhcp6_timer *timer)
{
	timer->t = monotonic_ms() + t;

	/* update the next expiration time */
	if (timer->t < client6_config.tm_sentinel)
		client6_config.tm_sentinel = timer->t;
}

/*
 * Check expiration for each timer. If a timer is expired,
 * call the expire function for the timer and update the timer.
 * Return the next interval for select() call.
 */
struct timeval *dhcp6_timer_check(void)
{
	unsigned long long now = monotonic_ms();
	struct dhcp6_timer *tm, *tm_next;

	client6_config.tm_sentinel = ULLONG_MAX;
	for (tm = LIST_FIRST(&client6_config.timer_head); tm; tm = tm_next) {
		tm_next = LIST_NEXT(tm, link);

		if ((long long)(now - tm->t) >= 0) {
			if ((*tm->expire)(tm->expire_data) == NULL)
				continue; /* timer has been freed */
		}

		if (tm->t < client6_config.tm_sentinel)
			client6_config.tm_sentinel = tm->t;
	}

	if (ULLONG_MAX == client6_config.tm_sentinel) {
		/* no need to timeout */
		return NULL;
	} else if (client6_config.tm_sentinel < now) {
		/* this may occur when the interval is too small */
		client6_config.tm_check.tv_sec = client6_config.tm_check.tv_usec = 0;
	} else {
		client6_config.tm_check.tv_sec =
			(client6_config.tm_sentinel - now) / 1000ULL;
		client6_config.tm_check.tv_usec =
			((client6_config.tm_sentinel - now) % 1000ULL) * 1000ULL;
	}
	return (&client6_config.tm_check);
}

unsigned long long dhcp6_timer_rest(struct dhcp6_timer *timer)
{
	unsigned long long now = monotonic_ms();

	if (timer->t - now <= 0) {
		log2("a timer must be expired, but not yet");
		return 0;
	} else {
		return (timer->t - now);
	}
}

/* Domain names stored in RFC1035 format */
#if ENABLE_FEATURE_UDHCP_RFC3397
static int dhcp6_get_domain(int optlen, void *cp, int dh6optype,
			    struct dhcp6_list *list)
{
	char *names, *p, *n;

	names = dname_dec(cp, optlen, "");
	if (names == NULL) {
		bb_error_msg("can't decode %s", dhcp6optstr(dh6optype));
		return -1;
	}

	log2("  domains: '%s'", names);

	p = names;
	while (*p) {
		struct dhcp6_vbuf vb;

		n = strchrnul(p, ' ');
		vb.dv_buf = p;
		vb.dv_len = n - p + 1;
		if (*n)
			*(n++) = '\0';

		if (dhcp6_add_listval(list, dh6optype, DHCP6_LISTVAL_VBUF,
				&vb, NULL) == NULL) {
			free(names);
			return -2;
		}
		p = n;
	}
	free(names);

	return 0;
}
#endif /* FEATURE_UDHCP_RFC3397 */

#if ENABLE_FEATURE_DHCP6_AUTH
struct authparam *new_authparam(int proto, int alg, int rdm)
{
	struct authparam *authparam;

	authparam = xzalloc(sizeof(*authparam));

	authparam->authproto = proto;
	authparam->authalgorithm = alg;
	authparam->authrdm = rdm;
	authparam->key = NULL;
	authparam->flags |= AUTHPARAM_FLAGS_NOPREVRD;
	authparam->prevrd = 0;

	return authparam;
}

static int copyout_auth(struct dhcp6_optinfo *optinfo,
		struct dhcp6opt **pp, struct dhcp6opt *bp, struct dhcp6opt *ep,
		int *totallenp)
{
	struct dhcp6opt_auth *auth;
	int authlen;

	authlen = sizeof(struct dhcp6opt_auth);
	if (!(optinfo->authflags & DHCP6OPT_AUTHFLAG_NOINFO)) {
		switch (optinfo->authproto) {
		case DHCP6_AUTHPROTO_DELAYED:
			/* Realm + key ID + HMAC-MD5 */
			authlen += optinfo->delayedauth_realmlen +
			    sizeof(optinfo->delayedauth_keyid) + 16;
			break;
#ifdef notyet
		case DHCP6_AUTHPROTO_RECONFIG:
			/* type + key-or-HAMC */
			authlen += 17;
			break;
#endif
		default:
			bb_error_msg("unsupported authentication protocol: %d",
				    optinfo->authproto);
			goto fail;
		}
	}

	if (ep - (*pp) < authlen)
		return -1;

	/* copy_option will take care of type and len later */
	auth = (struct dhcp6opt_auth *)(*pp);
	auth->dh6_proto = (uint8_t)optinfo->authproto;
	auth->dh6_alg = (uint8_t)optinfo->authalgorithm;
	auth->dh6_rdm = (uint8_t)optinfo->authrdm;
	memcpy(auth->dh6_rdinfo, &optinfo->authrd, sizeof(auth->dh6_rdinfo));

	if (!(optinfo->authflags & DHCP6OPT_AUTHFLAG_NOINFO)) {
		char *authinfo;

		switch (optinfo->authproto) {
		case DHCP6_AUTHPROTO_DELAYED:
			authinfo = (char *)(auth + 1);

			/* copy realm */
			memcpy(authinfo, optinfo->delayedauth_realmval,
			    optinfo->delayedauth_realmlen);
			authinfo += optinfo->delayedauth_realmlen;

			/* copy key ID (unaligned!) */
			move_to_unaligned32(authinfo, htonl(optinfo->delayedauth_keyid));

			/*
			 * Set the offset so that the caller can
			 * calculate the HMAC.
			 */
			optinfo->delayedauth_offset = ((char *)(*pp) - (char *)bp)
										+ authlen - 16;

			log1("key ID %x, offset %d", optinfo->delayedauth_keyid,
			    optinfo->delayedauth_offset);
			break;
#ifdef notyet
		case DHCP6_AUTHPROTO_RECONFIG:
#endif
		default:
			bb_error_msg("unsupported authentication protocol: %d",
				    optinfo->authproto);
			free(auth);
			goto fail;
		}
	}

	if (copy_option(D6_OPT_AUTH, authlen - 4, NULL, pp, ep, totallenp) != 0) {
		goto fail;
	}
	return 0;

 fail:
	return -1;
}
#endif /* ENABLE_FEATURE_DHCP6_AUTH */

static int sa6_plen2mask(struct sockaddr_in6 *sa6, int plen)
{
	uint8_t *cp;

	if (plen < 0 || plen > 128)
		return -1;

	memset(sa6, 0, sizeof(*sa6));
	sa6->sin6_family = AF_INET6;
#ifdef HAVE_SA_LEN
	sa6->sin6_len = sizeof(*sa6);
#endif

	for (cp = (uint8_t *)&sa6->sin6_addr; plen > 7; plen -= 8)
		*cp++ = 0xff;
	*cp = 0xff << (8 - plen);

	return 0;
}

int prefix6_mask(struct in6_addr *in6, int plen)
{
	struct sockaddr_in6 mask6;
	int i;

	if (sa6_plen2mask(&mask6, plen))
		return -1;

	for (i = 0; i < 16; i++)
		in6->s6_addr[i] &= mask6.sin6_addr.s6_addr[i];

	return 0;
}

/* TODO! migrate to libbb functions
  Get hardware id(MAC) of first ethernet interface for DUID
 */
static ssize_t gethwid(char *buf, uint16_t *hwtype)
{
	int n, numreqs = 30;
	struct ifconf ifc;
	struct ifreq *ifr;
	int skfd;
	ssize_t l = 0;

	ifc.ifc_buf = NULL;
	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd < 0)
		return -1;

	/* query all available interfaces */
	for (;;) {
		ifc.ifc_len = sizeof(struct ifreq) * numreqs;
		ifc.ifc_buf = xrealloc(ifc.ifc_buf, ifc.ifc_len);

		if (ioctl_or_warn(skfd, SIOCGIFCONF, &ifc) < 0) {
			goto out;
		}
		if (ifc.ifc_len == (int)(sizeof(struct ifreq) * numreqs)) {
			/* assume it overflowed and try again */
			numreqs += 10;
			continue;
		}
		break;
	}

	/* search for acceptable hwaddr */
	ifr = ifc.ifc_req;
	for (n = 0; n < ifc.ifc_len; n += sizeof(struct ifreq), ifr++) {
		if (ioctl(skfd, SIOCGIFHWADDR, ifr) != 0)
			continue;
		switch (ifr->ifr_hwaddr.sa_family) {
			case ARPHRD_ETHER:
			case ARPHRD_EETHER:
			case ARPHRD_IEEE802:
				memcpy(buf, ifr->ifr_hwaddr.sa_data, 8);
				*hwtype = ifr->ifr_hwaddr.sa_family;
				l = ETH_ALEN;
				break;
			default:
				continue;
		}
		break;
	}
	if (l > 0)
		log1("found an interface %s hwtype %d for DUID", ifr->ifr_name, *hwtype);

 out:
	close(skfd);
	free(ifc.ifc_buf);
	return l;
}

int get_duid(const char *idfile, struct dhcp6_vbuf *duid, int duid_type)
{
	int fd;
	uint16_t len = 0, hwtype = 0;

	fd = open(idfile, O_RDONLY);
	if (fd < 0 && errno != ENOENT)
		bb_error_msg("can't open DUID file: %s", idfile);

	if (fd > 0) {
		/* decode length */
		if (safe_read(fd, &len, sizeof(len)) != sizeof(len)) {
			bb_error_msg("DUID file corrupted");
			goto fail;
		}
		duid->dv_len = len;
		duid->dv_buf = xmalloc(len);

		if (safe_read(fd, duid->dv_buf, len) != len) {
			bb_error_msg("DUID file corrupted");
			goto fail;
		}

		log1("extracted an existing DUID from %s: %s",
		    idfile, duidstr(duid));
	} else {
		int hwlen;
		struct iovec iov[2];
		union dhcp6opt_duid_type *dp;
		char tmpbuf[64];	/* HWID should be no more than 64 bytes */

		hwlen = gethwid(tmpbuf, &hwtype);
		if (hwlen < 0) {
			bb_error_msg("can't get hardware address");
			goto fail;
		}
		duid->dv_buf = xmalloc(hwlen + sizeof(union dhcp6opt_duid_type));

		dp = (union dhcp6opt_duid_type *)duid->dv_buf;

		/* we only support the types 1,3 DUID */
		switch (duid_type) {
			case 1: {
				uint64_t t64;

				len = sizeof(struct dhcp6opt_duid_type1);
				dp->d1.hwtype = htons(hwtype);
				/* time is Jan 1, 2000 (UTC), modulo 2^32 */
				t64 = (uint64_t)(time(NULL) - 946684800);
				dp->d1.time = htonl((uint32_t )t64);
				break;
				}
			case 3:
				len = sizeof(struct dhcp6opt_duid_type3);
				dp->d3.hwtype = htons(hwtype);
				break;
		}
		dp->type = htons(duid_type);
		memcpy((char *)dp + len, tmpbuf, hwlen);
		len += hwlen;
		duid->dv_len = len;

		log1("generated a new DUID: %s", duidstr(duid));

		/* save the (new) ID to the file for next time */
		fd = open(idfile, O_WRONLY|O_CREAT|O_TRUNC);
		if (fd < 0) {
			/* Not fatal */
			return 0;
		}
		iov[0].iov_base = &len;
		iov[0].iov_len  = sizeof(len);
		iov[1].iov_base = duid->dv_buf;
		iov[1].iov_len  = duid->dv_len;
		if (writev(fd, iov, 2) < 0) {
			bb_error_msg("can't write DUID file");
			goto fail;
		}
	}
	if (fd > 0)
		close(fd);
	return 0;

  fail:
	if (fd > 0)
		close(fd);
	dhcp6_vbuf_free(duid);
	return -1;
}

void dhcp6_init_options(struct dhcp6_optinfo *optinfo)
{
	memset(optinfo, 0, sizeof(*optinfo));

	optinfo->pref = -1;
	optinfo->elapsed_time = -1;
	optinfo->refreshtime = -1;

	TAILQ_INIT(&optinfo->ia_list);
	TAILQ_INIT(&optinfo->reqopt_list);
	TAILQ_INIT(&optinfo->stcode_list);
	TAILQ_INIT(&optinfo->ad_list);

	optinfo->authproto = -1;
	optinfo->authalgorithm = -1;
	optinfo->authrdm = -1;
}

void dhcp6_clear_options(struct dhcp6_optinfo *optinfo)
{
	switch (optinfo->authproto) {
	case DHCP6_AUTHPROTO_DELAYED:
		dhcp6_vbuf_free(&optinfo->authinfo.aiu_delayed.realm);
		break;
	}

	dhcp6_vbuf_free(&optinfo->clientID);
	dhcp6_vbuf_free(&optinfo->serverID);

	dhcp6_clear_list(&optinfo->ia_list);
	dhcp6_clear_list(&optinfo->reqopt_list);
	dhcp6_clear_list(&optinfo->stcode_list);
	dhcp6_clear_list(&optinfo->ad_list);

	dhcp6_vbuf_free(&optinfo->relay_msg);
	dhcp6_vbuf_free(&optinfo->ifidopt);

	dhcp6_init_options(optinfo);
}

void dhcp6_copy_options(struct dhcp6_optinfo *dst, struct dhcp6_optinfo *src)
{
	dhcp6_vbuf_copy(&dst->clientID, &src->clientID);
	dhcp6_vbuf_copy(&dst->serverID, &src->serverID);

	dhcp6_copy_list(&dst->ia_list, &src->ia_list);
	dhcp6_copy_list(&dst->reqopt_list, &src->reqopt_list);
	dhcp6_copy_list(&dst->stcode_list, &src->stcode_list);
	dhcp6_copy_list(&dst->ad_list, &src->ad_list);

	dst->rapidcommit = src->rapidcommit;
	dst->elapsed_time = src->elapsed_time;
	dst->refreshtime = src->refreshtime;
	dst->pref = src->pref;

	if (src->relaymsg_msg != NULL) {
		dhcp6_vbuf_copy(&dst->relay_msg, &src->relay_msg);
	}

	if (src->ifidopt_id != NULL) {
		dhcp6_vbuf_copy(&dst->ifidopt, &src->ifidopt);
	}

	dst->authflags = src->authflags;
	dst->authproto = src->authproto;
	dst->authalgorithm = src->authalgorithm;
	dst->authrdm = src->authrdm;
	dst->authrd = src->authrd;

	switch (src->authproto) {
	case DHCP6_AUTHPROTO_DELAYED:
		dst->delayedauth_keyid = src->delayedauth_keyid;
		dst->delayedauth_offset = src->delayedauth_offset;
		dhcp6_vbuf_copy(&dst->authinfo.aiu_delayed.realm,
				&src->authinfo.aiu_delayed.realm);
		break;
#ifdef notyet
	case DHCP6_AUTHPROTO_RECONFIG:
		dst->reconfigauth_type = src->reconfigauth_type;
		dst->reconfigauth_offset = src->reconfigauth_offset;
		memcpy(dst->reconfigauth_val, src->reconfigauth_val,
		    sizeof(dst->reconfigauth_val));
		break;
#endif
	}
}

static int copy_option(uint16_t type, uint16_t len, void *val,
		struct dhcp6opt **optp, struct dhcp6opt *ep, int *totallenp)
{
	struct dhcp6opt *opt = *optp, opth;

	if ((char *)ep - (char *)opt < len + sizeof(struct dhcp6opt)) {
		log1("option buffer short for %s", dhcp6optstr(type));
		return -1;
	}

	opth.dh6opt_type = htons(type);
	opth.dh6opt_len = htons(len);
	memcpy(opt, &opth, sizeof(opth));
	if (len != 0 && val)
		memcpy(opt + 1, val, len);

	(*optp) = (struct dhcp6opt *)((char *)(opt + 1) + len);
	(*totallenp) += sizeof(struct dhcp6opt) + len;

	log2("set %s (len %d)", dhcp6optstr(type), len);

	return 0;
}

int dhcp6_get_options(struct dhcp6opt *p, struct dhcp6opt *ep,
			struct dhcp6_optinfo *optinfo)
{
	struct dhcp6opt *np;
	int opt, optlen, tlen;
	char *cp, *val;
	union {
		struct dhcp6opt h;
		struct dhcp6opt_ia optia;
	} u ALIGN4;
	union {
		uint16_t val16;
		uint32_t val32;
		struct dhcp6_ia ia;
		struct dhcp6_vbuf duid0;
		struct in6_addr valaddr;
	} u1 ALIGN4;
	struct dhcp6_list sublist;
#if ENABLE_FEATURE_DHCP6_AUTH
	char *bp = (char *)p;
	int authinfolen;
#endif

	TAILQ_INIT(&sublist);

	for (; p + 1 <= ep; p = np) {
		/*
		 * get the option header.  XXX: since there is no guarantee
		 * about the header alignment, we need to make a local copy.
		 */
		memcpy(&u.h, p, sizeof(struct dhcp6opt));
		optlen = ntohs(u.h.dh6opt_len);
		opt = ntohs(u.h.dh6opt_type);

		cp = (char *)(p + 1);
		np = (struct dhcp6opt *)(cp + optlen);

		log2("get DHCP6 option %s, len %d", dhcp6optstr(opt), optlen);

		/* option length field overrun */
		if (opt > D6_OPT__MAX)
			goto unexpected;
		tlen = dhcp6_option_typelens[opt].minlen;
		if (np > ep || optlen < tlen)
			goto malformed;

		switch (opt) {

		case D6_OPT_CLIENTID:
			u1.duid0.dv_len = optlen;
			u1.duid0.dv_buf = cp;
			log2("  DUID: %s", duidstr(&u1.duid0));
			dhcp6_vbuf_copy(&optinfo->clientID, &u1.duid0);
			break;

		case D6_OPT_SERVERID:
			u1.duid0.dv_len = optlen;
			u1.duid0.dv_buf = cp;
			log2("  DUID: %s", duidstr(&u1.duid0));
			dhcp6_vbuf_copy(&optinfo->serverID, &u1.duid0);
			break;

		case D6_OPT_STATUS_CODE:
			move_from_unaligned16(u1.val16, cp);
			u1.val16 = ntohs(u1.val16);
			log2("  status code: %s", dhcp6stcodestr(u1.val16));

			/* need to check duplication? */
			if (dhcp6_add_listval(&optinfo->stcode_list, opt,
			    DHCP6_LISTVAL_NUM16, &u1, NULL) == NULL) {
				goto fail;
			}
			break;

		case D6_OPT_ORO:
			if ((optlen & 0x1) != 0) /* must be multiple by sizeof(uint16_t) */
				goto malformed;

			for (val = cp; val < (char *)np; val += sizeof(uint16_t)) {

				move_from_unaligned16(u1.val16, val);
				u1.val16 = ntohs(u1.val16);
				log2("  requested option: %s", dhcp6optstr(u1.val16));

				if (dhcp6_find_listval(&optinfo->reqopt_list,
				    DHCP6_LISTVAL_NUM16, &u1, 0)) {
					bb_error_msg("duplicated %s", dhcp6optstr(u1.val16));
					continue;
				}

				if (dhcp6_add_listval(&optinfo->reqopt_list, opt,
				    DHCP6_LISTVAL_NUM16, &u1, NULL) == NULL) {
					goto fail;
				}
			}
			break;

		case D6_OPT_PREFERENCE:
			log2("  preference: %d", (int)*(uint8_t *)cp);
			if (optinfo->pref != -1) {
				bb_error_msg("duplicated %s", dhcp6optstr(opt));
			} else
				optinfo->pref = (int)*(uint8_t *)cp;
			break;

		case D6_OPT_ELAPSED_TIME:
			move_from_unaligned16(u1.val16, cp);
			u1.val16 = ntohs(u1.val16);

			log2("  elapsed time: %u", (uint32_t)u1.val16);
			if (optinfo->elapsed_time != -1) {
				bb_error_msg("duplicated %s", dhcp6optstr(opt));
			} else
				optinfo->elapsed_time = u1.val16;
			break;

		case D6_OPT_RELAY_MSG:
			optinfo->relaymsg_msg = xmemdup(cp, optlen);
			optinfo->relaymsg_len = optlen;
			break;

#if ENABLE_FEATURE_DHCP6_AUTH
		case D6_OPT_AUTH:
			/*
			 * Any DHCP message that includes more than one
			 * authentication option MUST be discarded.
			 * [RFC3315 Section 21.4.2]
			 */
			if (optinfo->authproto != -1) {
				bb_error_msg("duplicated %s", dhcp6optstr(opt));
				goto fail;
			}

			optinfo->authproto = *(uint8_t *)cp++;
			optinfo->authalgorithm = *(uint8_t *)cp++;
			optinfo->authrdm = *(uint8_t *)cp++;
			memcpy(&optinfo->authrd, cp, sizeof(optinfo->authrd));
			cp += sizeof(optinfo->authrd);

			log2("  %s", sprint_auth(optinfo));

			authinfolen = optlen - tlen;
			switch (optinfo->authproto) {
			case DHCP6_AUTHPROTO_DELAYED:
				if (authinfolen == 0) {
					optinfo->authflags |= DHCP6OPT_AUTHFLAG_NOINFO;
					break;
				}
				/* XXX: should we reject an empty realm? */
				if (authinfolen < sizeof(optinfo->delayedauth_keyid) + 16) {
					goto malformed;
				}

				optinfo->delayedauth_realmlen = authinfolen -
				    (sizeof(optinfo->delayedauth_keyid) + 16);
				optinfo->delayedauth_realmval =
				    malloc_or_warn(optinfo->delayedauth_realmlen);
				if (optinfo->delayedauth_realmval == NULL)
					goto fail;
				memcpy(optinfo->delayedauth_realmval, cp,
				    optinfo->delayedauth_realmlen);
				cp += optinfo->delayedauth_realmlen;

				memcpy(&optinfo->delayedauth_keyid, cp,
				    sizeof(optinfo->delayedauth_keyid));
				optinfo->delayedauth_keyid = ntohl(optinfo->delayedauth_keyid);
				cp += sizeof(optinfo->delayedauth_keyid);

				optinfo->delayedauth_offset = cp - bp;
				cp += 16;

				log2("  auth key ID: %x, offset=%d, realmlen=%d",
				    optinfo->delayedauth_keyid,
				    optinfo->delayedauth_offset,
				    optinfo->delayedauth_realmlen);
				break;
#ifdef notyet
			case DHCP6_AUTHPROTO_RECONFIG:
				break;
#endif
			default:
				bb_error_msg("unsupported authentication protocol: %d", *cp);
				goto fail;
			}
			break;
#endif /* ENABLE_FEATURE_DHCP6_AUTH */

		case D6_OPT_RAPID_COMMIT:
			if (optlen != 0)
				goto malformed;
			optinfo->rapidcommit = 1;
			break;

		case D6_OPT_INTERFACE_ID:
			optinfo->ifidopt_id = xmemdup(cp, optlen);
			optinfo->ifidopt_len = optlen;
			break;

		case D6_OPT_SIP_SERVER_D:
		case D6_OPT_DNSNAME:
		case D6_OPT_NIS_DOMAIN:
		case D6_OPT_NISP_DOMAIN:
		case D6_OPT_BCMCS_SERVER_D:
#if ENABLE_FEATURE_UDHCP_RFC3397
			if (dhcp6_get_domain(optlen, cp, opt, &optinfo->ad_list) == -1)
				goto fail;
#endif
			break;

		case D6_OPT_SIP_SERVER_A:
		case D6_OPT_DNS:
		case D6_OPT_NIS_SERVERS:
		case D6_OPT_NISP_SERVERS:
		case D6_OPT_BCMCS_SERVER_A:
		case D6_OPT_SNTP_SERVERS:
		case D6_OPT_NTP_SERVER:
			if ((optlen % sizeof(struct in6_addr)) != 0)
				goto malformed;
			for (val = cp; val < (char *)np; val += sizeof(struct in6_addr)) {
				char a[INET6_ADDRSTRLEN];

				memcpy(&u1.valaddr, val, sizeof(struct in6_addr));
#if defined CONFIG_UDHCP_DEBUG && CONFIG_UDHCP_DEBUG >= 2
				sprint_nip6(a, (const uint8_t *)&u1.valaddr);
				log2("  address: %s", a);
#endif

				if (dhcp6_find_listval(&optinfo->ad_list, opt, &u1.valaddr,
					 MATCHLIST_DH6OPTYPE)) {
					sprint_nip6(a, (const uint8_t *)&u1.valaddr);
					bb_error_msg("duplicated %s address (%s)",
								dhcp6optstr(opt), a);
					continue;
				}

				if (dhcp6_add_listval(&optinfo->ad_list, opt, DHCP6_LISTVAL_ADDR6,
					 &u1, NULL) == NULL) {
					goto fail;
				}
			}
			break;

		case D6_OPT_IA_PD:
		case D6_OPT_IA_NA:
			memcpy(((char *)&u) + sizeof(struct dhcp6opt), cp, tlen);
			u1.ia.iaid = ntohl(u.optia.dh6_iaid);
			u1.ia.t1 = ntohl(u.optia.dh6_t1);
			u1.ia.t2 = ntohl(u.optia.dh6_t2);

			log2("  %s: ID=%u, T1=%u, T2=%u", dhcp6optstr(opt),
				u1.ia.iaid, u1.ia.t1, u1.ia.t2);

			/* duplication check */
			if (dhcp6_find_listval(&optinfo->ia_list,
			    opt, &u1, MATCHLIST_DH6OPTYPE)) {
				bb_error_msg("duplicated %s %u",	dhcp6optstr(opt), u1.ia.iaid);
				break; /* ignore this IA */
			}

			/* take care of sub-options */
			if (copyin_option(opt, (struct dhcp6opt *)(cp + tlen),
				 (struct dhcp6opt *)(cp + optlen), &sublist)) {
				goto fail;
			}

			/* link this option set */
			if (dhcp6_add_listval(&optinfo->ia_list, opt,
			    DHCP6_LISTVAL_IA, &u1, &sublist) == NULL) {
				goto fail;
			}
			dhcp6_clear_list(&sublist);
			break;

		case D6_OPT_REFRESHTIME:
			move_from_unaligned32(u1.val32, cp);
			u1.val32 = ntohl(u1.val32);
			log2("   information refresh time: %u", u1.val32);
			if (u1.val32 < DHCP6_IRT_MINIMUM) {
				/*
				 * A client MUST use the refresh time
				 * IRT_MINIMUM if it receives the option with a
				 * value less than IRT_MINIMUM.
				 * [draft-ietf-dhc-lifetime-02.txt,
				 *  Section 3.2]
				 */
				bb_error_msg("refresh time is too small (%d), adjusted",	u1.val32);
				u1.val32 = DHCP6_IRT_MINIMUM;
			}
			if (optinfo->refreshtime != -1) {
				bb_error_msg("duplicated %s", dhcp6optstr(opt));
			} else
				optinfo->refreshtime = (int64_t)u1.val32;
			break;

		case D6_OPT_RECONF_ACCEPT:
			/* Silently ignore unsupported feature */
			break;

		default:
 unexpected:
			/* no option specific behavior */
			bb_error_msg("unexpected DHCP6 option %s, len %d",
				dhcp6optstr(opt), optlen);
			break;
		}
	}

	return 0;

 malformed:
	bb_error_msg("malformed DHCP6 option: type %d, len %d", opt, optlen);
 fail:
	dhcp6_clear_list(&sublist);
	dhcp6_clear_options(optinfo);
	return -1;
}

static int copyin_option(int type, struct dhcp6opt *p, struct dhcp6opt *ep,
			 struct dhcp6_list *list)
{
	int opt, optlen, tlen;
	char *cp;
	struct dhcp6opt *np;
	union {
		struct dhcp6opt h;
		struct dhcp6opt_stcode stcode;
		struct dhcp6opt_ia_pd_prefix prefix;
		struct dhcp6opt_ia_addr addr;
	} u ALIGN4;
	union {
		uint16_t dh6_stcode;
		struct dhcp6_prefix ia_prefix;
		struct dhcp6_statefuladdr ia_addr;
	} u1 ALIGN4;
	struct dhcp6_list sublist;
	dhcp6_listval_type_t lvtype;
	char a[INET6_ADDRSTRLEN];

	TAILQ_INIT(&sublist);

	for (; p + 1 <= ep; p = np) {
		memcpy(&u.h, p, sizeof(struct dhcp6opt));
		optlen = ntohs(u.h.dh6opt_len);
		opt = ntohs(u.h.dh6opt_type);

		cp = (char *)(p + 1);
		np = (struct dhcp6opt *)(cp + optlen);

		log2("get DHCP6 option %s, len %d", dhcp6optstr(opt), optlen);

		/* check option context & length */
		switch (opt) {
		case D6_OPT_IA_PD_PREFIX:
			if (type != D6_OPT_IA_PD) {
				bb_error_msg("%s is an invalid position for %s",
				    dhcp6optstr(type), dhcp6optstr(opt));
				goto fail;
			}
			break;
		case D6_OPT_IAADDR:
			if (type != D6_OPT_IA_NA) {
				bb_error_msg("%s is an invalid position for %s",
				    dhcp6optstr(type), dhcp6optstr(opt));
				goto fail;
			}
			break;
		case D6_OPT_STATUS_CODE:
			break;
		default:
			bb_error_msg("unexpected DHCP6 option %s, len %d",
			    dhcp6optstr(opt), optlen);
			goto nextoption;
		} /* switch1(opt) */
		lvtype = dhcp6_option_typelens[opt].lvtype;
		tlen = dhcp6_option_typelens[opt].minlen;
		if (np > ep || optlen < tlen) {
				bb_error_msg("malformed DHCP6 option: type %d, len %d",
					opt, optlen);
				goto fail;
		}
		/* copy option main body (fixup unaligned access) */
		memcpy(((char *)&u) + sizeof(struct dhcp6opt), cp, tlen);

		switch (opt) {
		case D6_OPT_IA_PD_PREFIX:
			/* convert option values */
			u1.ia_prefix.plen = u.prefix.dh6_prefix_len;
			u1.ia_prefix.pltime = ntohl(u.prefix.dh6_preferred_time);
			u1.ia_prefix.vltime = ntohl(u.prefix.dh6_valid_time);
			memcpy(&u1.ia_prefix.addr, &u.prefix.dh6_prefix_addr, sizeof(u1.ia_prefix.addr));
			if (u1.ia_prefix.plen < 4 || u1.ia_prefix.plen > 128) {
				bb_error_msg("invalid prefix length: %d", u1.ia_prefix.plen);
				goto fail;
			}
			/* clear padding bits in the prefix address */
			prefix6_mask(&u1.ia_prefix.addr, u1.ia_prefix.plen);

			sprint_nip6(a, (const uint8_t *)&u1.ia_prefix.addr);
			log2("  IA_PD prefix: %s/%d pltime=%u vltime=%u", a,
				u1.ia_prefix.plen, u1.ia_prefix.pltime, u1.ia_prefix.vltime);

			if (dhcp6_find_listval(list, lvtype, &u1, 0)) {
				bb_error_msg("duplicated IA_PD %s/%d", a, u1.ia_prefix.plen );
				goto nextoption;
			}

			/* take care of sub-options */
			if (copyin_option(opt, (struct dhcp6opt *)(cp + tlen), np,
				 &sublist)) {
				goto fail;
			}
			break;

		case D6_OPT_IAADDR:
			/* convert option values */
			u1.ia_addr.pltime = ntohl(u.addr.dh6_preferred_time);
			u1.ia_addr.vltime = ntohl(u.addr.dh6_valid_time);
			memcpy(&u1.ia_addr.addr, &u.addr.dh6_addr, sizeof(u1.ia_addr.addr));

			sprint_nip6(a, (const uint8_t *)&u1.ia_addr.addr);
			log2("  IA_NA address: %s pltime=%u vltime=%u", a,
				u1.ia_addr.pltime, u1.ia_addr.vltime);

			if (dhcp6_find_listval(list, lvtype, &u1, 0)) {
				bb_error_msg("duplicated IA_NA %s", a);
				goto nextoption;
			}

			/* take care of sub-options */
			if (copyin_option(opt, (struct dhcp6opt *)(cp + tlen), np,
				 &sublist)) {
				goto fail;
			}
			break;

		case D6_OPT_STATUS_CODE:
			/* convert option values */
			u1.dh6_stcode = ntohs(u.stcode.dh6_stcode);

			log2("  status code: %s", dhcp6stcodestr(u1.dh6_stcode));

			/* duplication check */
			if (dhcp6_find_listval(list, lvtype, &u1, 0)) {
				bb_error_msg("duplicated status code (%d)", u1.dh6_stcode);
				goto nextoption;
			}
			break;
		} /* switch2(opt) */

		/* copy-in option */
		if (dhcp6_add_listval(list, opt, lvtype, &u1,
			 (opt == D6_OPT_STATUS_CODE) ? NULL : &sublist) == NULL)
			goto fail;
		dhcp6_clear_list(&sublist);

 nextoption:
		;
	} /* for(p) */

	return 0;

 fail:
	dhcp6_clear_list(&sublist);
	return -1;
}

/*
 * Construct a DHCPv6 option along with sub-options in the wire format.
 * If the packet buffer is NULL, just calculate the length of the option
 * (and sub-options) so that the caller can allocate a buffer to store the
 * option(s).
 * This function basically assumes that the caller prepares enough buffer to
 * store all the options.  However, it also takes the buffer end and checks
 * the possibility of overrun for safety.
 */
static int copyout_option(void *p, void *ep, struct dhcp6_listval *optval)
{
	union {
		struct dhcp6opt opt;
		struct dhcp6opt_stcode stcodeopt;
		struct dhcp6opt_ia ia;
		struct dhcp6opt_ia_pd_prefix pd_prefix;
		struct dhcp6opt_ia_addr ia_addr;
	} u ALIGN4;
	void *subp;
	struct dhcp6_listval *subov;
	int opt_len, headlen, sublen, opttype;

	/* check invariant for safety */
	if (ep <= p)
		return -1;

	/* first, detect the length of the option head */
	switch (optval->lvtype) {
	case DHCP6_LISTVAL_IA:
		headlen = sizeof(u.ia);
		opttype = optval->dh6optype;
		break;
	case DHCP6_LISTVAL_ADDR6:
		headlen = sizeof(u.pd_prefix);
		opttype = D6_OPT_IA_PD_PREFIX;
		break;
	case DHCP6_LISTVAL_PREFIX6:
		headlen = sizeof(u.pd_prefix);
		opttype = D6_OPT_IA_PD_PREFIX;
		break;
	case DHCP6_LISTVAL_STATEFULADDR6:
		headlen = sizeof(u.ia_addr);
		opttype = D6_OPT_IAADDR;
		break;
	case DHCP6_LISTVAL_NUM16:
		headlen = sizeof(u.stcodeopt);
		opttype = D6_OPT_STATUS_CODE;
		break;
	default:
		/*
		 * we encounter an unknown option.  this should be an internal
		 * error.
		 */
		bb_error_msg("unknown listval type %d", optval->lvtype);
		return -2;
	}

	/* then, calculate the length of and fill in the sub-options */
	sublen = 0;
	subp = p + headlen;
	TAILQ_FOREACH(subov, &optval->sublist, link) {
		int s = copyout_option(subp, ep, subov);

		if (s < 0)
			return s;
		subp += s;
		sublen += s;
	}

	/* finally, deal with the head part again */
	opt_len = headlen + sublen;

	log2("set %s [lvtype %d] (len %d)", dhcp6optstr(opttype),
		optval->lvtype, opt_len);
	if (ep - p < headlen) /* check it just in case */
		return -1;

	/* fill in the common part */
	memset(&u, 0, sizeof(u));
	u.opt.dh6opt_type = htons(opttype);
	u.opt.dh6opt_len  = htons(opt_len - sizeof(struct dhcp6opt));

	/* fill in type specific fields */
	switch (optval->lvtype) {

	case DHCP6_LISTVAL_IA:
		u.ia.dh6_iaid = htonl(optval->val_ia.iaid);
		u.ia.dh6_t1 = htonl(optval->val_ia.t1);
		u.ia.dh6_t2 = htonl(optval->val_ia.t2);
		break;

	case DHCP6_LISTVAL_PREFIX6:
		u.pd_prefix.dh6_preferred_time = htonl(optval->val_prefix6.pltime);
		u.pd_prefix.dh6_valid_time = htonl(optval->val_prefix6.vltime);
		u.pd_prefix.dh6_prefix_len = optval->val_prefix6.plen;
		/* XXX: prefix_addr is badly aligned, so we need memcpy */
		memcpy(&u.pd_prefix.dh6_prefix_addr,
		    &optval->val_prefix6.addr, sizeof(struct in6_addr));
		break;

	case DHCP6_LISTVAL_STATEFULADDR6:
		u.ia_addr.dh6_preferred_time = htonl(optval->val_statefuladdr6.pltime);
		u.ia_addr.dh6_valid_time = htonl(optval->val_statefuladdr6.vltime);
		u.ia_addr.dh6_addr = optval->val_statefuladdr6.addr;
		break;

	case DHCP6_LISTVAL_NUM16:
		u.stcodeopt.dh6_stcode = htons(optval->val_num16);
		break;

	default:
		/*
		 * XXX: this case should be rejected at the beginning of this
		 * function.
		 */
		return -2;
	}
	/* copyout the data */
	memcpy(p, &u, headlen);

	return opt_len;
}

static int dhcp6_set_iaoptions(int type, struct dhcp6_list *head,
		struct dhcp6opt **pp, struct dhcp6opt *ep, int *totallenp)
{
	struct dhcp6_listval *lv;

	TAILQ_FOREACH(lv, head, link) {
		int opt_len1;

		if (lv->dh6optype != type)
			continue;

		opt_len1 = copyout_option(*pp, ep, lv);
		if (opt_len1 < 0) {
			bb_error_msg("can't construct an %s option", dhcp6optstr(type));
			return opt_len1;
		}
		(*pp) = (struct dhcp6opt *)((char *)(*pp) + opt_len1);
		(*totallenp) += opt_len1;
	}
	return 0;
}

static int copyout_reqoptions(int type, struct dhcp6_list *head,
		struct dhcp6opt **pp, struct dhcp6opt *ep, int *totallenp)
{
	struct dhcp6_listval *lv;
	uint16_t *valp;
	int opt_len = 0;

	TAILQ_FOREACH(lv, head, link) {
		opt_len += sizeof(uint16_t);
	}
	if (opt_len == 0)
		return 0;

	valp = (uint16_t *)((char *)(*pp)+ sizeof(struct dhcp6opt));
	if (ep - (struct dhcp6opt *)valp < opt_len)
		return -1;

	TAILQ_FOREACH(lv, head, link) {
		/*
		 * Information request option can only be specified
		 * in information-request messages.
		 * [draft-ietf-dhc-lifetime-02.txt, Section 3.2]
		 */
		if (lv->val_num == D6_OPT_REFRESHTIME && type != DH6_INFORM_REQ) {
			log1("refresh time option is not requested for %s",
				 dhcp6msgstr(type));
		}

		move_to_unaligned16(valp++, htons((uint16_t)lv->val_num));
	}
	if (copy_option(D6_OPT_ORO, opt_len, NULL, pp, ep, totallenp) != 0) {
		return -1;
	}

	return 0;
}

int dhcp6_set_options(int type, struct dhcp6opt *optbp, struct dhcp6opt *optep,
		struct dhcp6_optinfo *optinfo)
{
	struct dhcp6opt *p = optbp;
	struct dhcp6_listval *lv;
	int len = 0;

	if (optinfo->clientID.dv_len) {
		if (copy_option(D6_OPT_CLIENTID, optinfo->clientID.dv_len,
		    optinfo->clientID.dv_buf, &p, optep, &len) != 0)
			goto fail;
	}

	if (optinfo->serverID.dv_len) {
		if (copy_option(D6_OPT_SERVERID, optinfo->serverID.dv_len,
		    optinfo->serverID.dv_buf, &p, optep, &len) != 0)
			goto fail;
	}

	if (dhcp6_set_iaoptions(D6_OPT_IA_PD, &optinfo->ia_list, &p, optep, &len) < 0)
		goto fail;

	if (optinfo->rapidcommit) {
		if (copy_option(D6_OPT_RAPID_COMMIT, 0, NULL, &p,
		    optep, &len) != 0)
			goto fail;
	}

	if (optinfo->pref != -1) {
		uint8_t p8 = (uint8_t)optinfo->pref;

		if (copy_option(D6_OPT_PREFERENCE, sizeof(p8), &p8, &p,
		    optep, &len) != 0)
			goto fail;
	}

	if (optinfo->elapsed_time != -1) {
		uint16_t p16 = htons((uint16_t)optinfo->elapsed_time);

		if (copy_option(D6_OPT_ELAPSED_TIME, sizeof(p16), &p16, &p,
		    optep, &len) != 0)
			goto fail;
	}

	TAILQ_FOREACH(lv, &optinfo->stcode_list, link) {
		uint16_t code = htons(lv->val_num16);

		if (copy_option(D6_OPT_STATUS_CODE, sizeof(code), &code, &p,
		    optep, &len) != 0)
			goto fail;
	}

	if (!TAILQ_EMPTY(&optinfo->reqopt_list)) {
		if (copyout_reqoptions(type, &optinfo->reqopt_list, &p,
		    optep, &len) != 0)
			goto fail;
	}

	if (dhcp6_set_iaoptions(D6_OPT_IA_NA, &optinfo->ia_list, &p, optep, &len) < 0)
		goto fail;

	if (optinfo->relaymsg_len) {
		if (copy_option(D6_OPT_RELAY_MSG, optinfo->relaymsg_len,
		    optinfo->relaymsg_msg, &p, optep, &len) != 0)
			goto fail;
	}

	if (optinfo->ifidopt_id) {
		if (copy_option(D6_OPT_INTERFACE_ID, optinfo->ifidopt_len,
		    optinfo->ifidopt_id, &p, optep, &len) != 0)
			goto fail;
	}

	if (optinfo->refreshtime != -1) {
		uint32_t p32 = htonl((uint32_t)optinfo->refreshtime);

		if (copy_option(D6_OPT_REFRESHTIME, sizeof(p32), &p32, &p,
		    optep, &len) != 0)
			goto fail;
	}

#if ENABLE_FEATURE_DHCP6_AUTH
	if (optinfo->authproto != -1) {
		if (copyout_auth(optinfo, &p, optbp, optep, &len) != 0)
			goto fail;
	}
#endif /* ENABLE_FEATURE_DHCP6_AUTH */

	return len;

  fail:
	return -1;
}

/* timer parameters (msec, unless explicitly commented) */
void dhcp6_set_timeoparam(struct dhcp6_event *ev)
{
	ev->retrans = 0;
	ev->max_retrans_cnt = 0;
	ev->max_retrans_dur = 0;
	ev->max_retrans_time = 0;

	switch (ev->state) {
	case DHCP6S_SOLICIT:
		ev->init_retrans = 1000;
		ev->max_retrans_time = 120000;
		break;
	case DHCP6S_INFOREQ:
		ev->init_retrans = 1000;
		ev->max_retrans_time = 120000;
		break;
	case DHCP6S_REQUEST:
		ev->init_retrans = 1000;
		ev->max_retrans_time = 30000;
		ev->max_retrans_cnt = 10;	/* Max Request retry attempts */
		break;
	case DHCP6S_RENEW:
		ev->init_retrans = 10000;	/* 10secs */
		ev->max_retrans_time = 600000;	/* 600secs */
		break;
	case DHCP6S_REBIND:
		ev->init_retrans = 10000;	/* 10secs */
		ev->max_retrans_time = 600000;	/* 600secs */
		break;
	case DHCP6S_RELEASE:
		ev->init_retrans = 1000;
		ev->max_retrans_cnt = 5;
		break;
	default:
		bb_error_msg_and_die("unexpected event state %d on %s",
		    ev->state, ev->ifp->ifname);
	}
}

void dhcp6_reset_timer(struct dhcp6_event *ev)
{
	double n, r;

	switch (ev->state) {
	case DHCP6S_INIT:
		/*
		 * The first Solicit message from the client on the interface
		 * MUST be delayed by a random amount of time between
		 * 0 and SOL_MAX_DELAY.
		 * [RFC3315 17.1.2]
		 * XXX: a random delay is also necessary before the first
		 * information-request message.  Fortunately, the parameters
		 * and the algorithm for these two cases are the same.
		 * [RFC3315 18.1.5]
		 */
		ev->retrans = (random() % (SOL_MAX_DELAY));
		break;

	default:
		if (ev->state == DHCP6S_SOLICIT && ev->timeouts == 0) {
			/*
			 * The first RT MUST be selected to be strictly
			 * greater than IRT by choosing RAND to be strictly
			 * greater than 0.
			 * [RFC3315 17.1.2]
			 */
			r = (double)((random() % 1000) + 1) / 10000;
			n = ev->init_retrans + r * ev->init_retrans;
		} else {
			r = (double)((random() % 2000) - 1000) / 10000;

			if (ev->timeouts == 0) {
				n = ev->init_retrans + r * ev->init_retrans;
			} else {
				n = 2 * ev->retrans + r * ev->retrans;
			}
		}
		if (ev->max_retrans_time && n > ev->max_retrans_time)
			n = ev->max_retrans_time + r * ev->max_retrans_time;

		ev->retrans = (long)n;
		break;
	}

	dhcp6_timer_set(ev->retrans, ev->timer);

	log1("reset a timer on %s, state=%s, timeo=%d, retrans=%ld",
		ev->ifp->ifname, dhcp6_event_statestr(ev), ev->timeouts, ev->retrans);
}

#if ENABLE_FEATURE_DHCP6_AUTH
/*
 * Provide an NTP-format timestamp as a replay detection counter
 * as mentioned in RFC3315.
 */
#define JAN_1970        2208988800UL        /* 1970 - 1900 in seconds */
int get_rdvalue(int rdm, void *rdvalue)
{
	struct timeval tv;
	uint32_t pack[2];

	if (rdm != DHCP6_AUTHRDM_MONOCOUNTER) {
		bb_error_msg("unsupported replay detection method (%d)", rdm);
		return -1;
	}

	gettimeofday(&tv, NULL);

	pack[0] = htonl((uint32_t)tv.tv_sec + JAN_1970);
	pack[1] = htonl((uint32_t)tv.tv_usec);

	memcpy(rdvalue, pack, sizeof(uint64_t));

	return 0;
}
#endif /* ENABLE_FEATURE_DHCP6_AUTH */

#if ENABLE_FEATURE_DHCP6_AUTH
# if defined CONFIG_UDHCP_DEBUG && CONFIG_UDHCP_DEBUG >= 2
static char *sprint_auth(struct dhcp6_optinfo *optinfo)
{
	char *ret;
	const char *proto, *alg, *rdm;
	char proto0[16], alg0[16], rdm0[16];

	switch (optinfo->authproto) {
	case DHCP6_AUTHPROTO_DELAYED:
		proto = "delayed";
		break;
#ifdef notyet
	case DHCP6_AUTHPROTO_RECONFIG:
		proto = "reconfig";
		break;
#endif
	default:
		snprintf(proto0, sizeof(proto0), "unknown(%d)",
		    optinfo->authproto & 0xff);
		proto = proto0;
		break;
	}

	switch (optinfo->authalgorithm) {
	case DHCP6_AUTHALG_HMACMD5:
		alg = "HMAC-MD5";
		break;
	default:
		snprintf(alg0, sizeof(alg0), "unknown(%d)",
		    optinfo->authalgorithm & 0xff);
		alg = alg0;
		break;
	}

	switch (optinfo->authrdm) {
	case DHCP6_AUTHRDM_MONOCOUNTER:
		rdm = "mono counter";
		break;
	default:
		snprintf(rdm0, sizeof(rdm0), "unknown(%d)", optinfo->authrdm);
		rdm = rdm0;
	}

	ret = xasprintf("proto: %s, alg: %s, RDM: %s, RD: %llx",
	    proto, alg, rdm, SWAP_BE64(optinfo->authrd));

	return ret;
}
# endif /* CONFIG_UDHCP_DEBUG */
#endif /* ENABLE_FEATURE_DHCP6_AUTH */

static const char *dh6_opts_strings[D6_OPT__MAX+1] = {
 [D6_OPT_CLIENTID]			= "client ID",
 [D6_OPT_SERVERID]			= "server ID",
 [D6_OPT_IA_NA]				= "identity association",
 [D6_OPT_IA_TA]				= "IA for temporary",
 [D6_OPT_IAADDR]			= "IA address",
 [D6_OPT_ORO]				= "option request",
 [D6_OPT_PREFERENCE]		= "preference",
 [D6_OPT_ELAPSED_TIME]		= "elapsed time",
 [D6_OPT_RELAY_MSG]			= "relay message",
 [D6_OPT_AUTH]				= "authentication",
 [D6_OPT_UNICAST]			= "server unicast",
 [D6_OPT_STATUS_CODE]		= "status code",
 [D6_OPT_RAPID_COMMIT]		= "rapid commit",
 [D6_OPT_USER_CLASS]		= "user class",
 [D6_OPT_VENDOR_CLASS]		= "vendor class",
 [D6_OPT_VENDOR_OPTS]		= "vendor specific info",
 [D6_OPT_INTERFACE_ID]		= "interface ID",
 [D6_OPT_RECONF_MSG]		= "reconfigure message",
 [D6_OPT_RECONF_ACCEPT]		= "reconfigure accept",
 [D6_OPT_SIP_SERVER_D]		= "SIP domain name",
 [D6_OPT_SIP_SERVER_A]		= "SIP server address",
 [D6_OPT_DNS]				= "DNS",
 [D6_OPT_DNSNAME]			= "domain search list",
 [D6_OPT_IA_PD]				= "IA_PD",
 [D6_OPT_IA_PD_PREFIX]		= "IA_PD prefix",
 [D6_OPT_NIS_SERVERS]		= "NIS servers",
 [D6_OPT_NISP_SERVERS]		= "NIS+ servers",
 [D6_OPT_NIS_DOMAIN]		= "NIS domain name",
 [D6_OPT_NISP_DOMAIN]		= "NIS+ domain name",
 [D6_OPT_SNTP_SERVERS]		= "SNTP server",
 [D6_OPT_REFRESHTIME]		= "information refresh time",
 [D6_OPT_BCMCS_SERVER_D]	= "BCMCS domain name",
 [D6_OPT_BCMCS_SERVER_A]	= "BCMCS server address",
 [D6_OPT_REMOTE_ID]			= "remote ID",
 [D6_OPT_SUBSCRIBER_ID]		= "subscriber ID",
 [D6_OPT_ERO]				= "Relay agent echo request",
 [D6_OPT_NTP_SERVER]		= "NTP server",
};

#define genstr        (&bb_common_bufsiz1[COMMON_BUFSIZE / 2])
#define sizeof_genstr (COMMON_BUFSIZE / 2 - 1)
const char *dhcp6optstr(int type)
{
	if (type > 65535)
		return "INVALID option";
	if (type > D6_OPT__MAX || dh6_opts_strings[type] == NULL)
		return utoa_to_buf(type, genstr, sizeof_genstr);

	return dh6_opts_strings[type];
}

static const char *dh6_msgs_strings[DH6__MSG__MAX+1] = {
 [0]				= "???",
 [DH6_SOLICIT]		= "solicit",
 [DH6_ADVERTISE]	= "advertise",
 [DH6_REQUEST]		= "request",
 [DH6_CONFIRM]		= "confirm",
 [DH6_RENEW]		= "renew",
 [DH6_REBIND]		= "rebind",
 [DH6_REPLY]		= "reply",
 [DH6_RELEASE]		= "release",
 [DH6_DECLINE]		= "decline",
 [DH6_RECONFIGURE]	= "reconfigure",
 [DH6_INFORM_REQ]	= "information request",
 [DH6_RELAY_FORW]	= "relay-forward",
 [DH6_RELAY_REPLY]	= "relay-reply",
};

const char *dhcp6msgstr(int type)
{
	if (type > 255)
		return "INVALID msg";
	if (type > DH6__MSG__MAX) {
		return utoa_to_buf(type, genstr+16, sizeof_genstr-16);
	}

	return dh6_msgs_strings[type];
}

static const char *dh6_stcodes_strings[D6_OPT_STCODE__MAX+1] = {
 [D6_OPT_STCODE_SUCCESS]		= "success",
 [D6_OPT_STCODE_UNSPECFAIL]		= "unspec failure",
 [D6_OPT_STCODE_NOADDRSAVAIL]	= "no addresses",
 [D6_OPT_STCODE_NOBINDING]		= "no binding",
 [D6_OPT_STCODE_NOTONLINK]		= "not on-link",
 [D6_OPT_STCODE_USEMULTICAST]	= "use multicast",
 [D6_OPT_STCODE_NOPREFIXAVAIL]	= "no prefixes",
};

const char *dhcp6stcodestr(uint16_t code)
{
	if (code > 255)
		return "INVALID code";
	if (code > D6_OPT_STCODE__MAX) {
		return utoa_to_buf(code, genstr+32, sizeof_genstr-32);
	}

	return dh6_stcodes_strings[code];
}

void BUG_dhcp6c_globals_too_big(void);

char *duidstr(struct dhcp6_vbuf *duid)
{
	int i, len = 0;
#define	duid_buf	(genstr + 48)
	if (sizeof("xx:") * 32 + sizeof("...") > sizeof_genstr - 48)
		BUG_dhcp6c_globals_too_big();

	for (i = 0; i < duid->dv_len && i < 32; i++) {
		if (i > 0)
			duid_buf[len++] = ':';
		bin2hex(duid_buf + len, (const char * )&duid->dv_buf[i], 1);
		len += 2;
	}
	if (i < duid->dv_len) {
		strcpy(duid_buf + len, "...");
		len += 3;
	}
	duid_buf[len] = '\0';

	return duid_buf;
#undef  duid_buf
}
#undef	genstr
#undef	sizeof_genstr

#if defined CONFIG_UDHCP_DEBUG && CONFIG_UDHCP_DEBUG >= 1
const char *dhcp6_event_statestr(struct dhcp6_event *ev)
{
	switch (ev->state) {
	case DHCP6S_INIT:
		return "INIT";
	case DHCP6S_SOLICIT:
		return "SOLICIT";
	case DHCP6S_INFOREQ:
		return "INFOREQ";
	case DHCP6S_REQUEST:
		return "REQUEST";
	case DHCP6S_RENEW:
		return "RENEW";
	case DHCP6S_REBIND:
		return "REBIND";
	case DHCP6S_RELEASE:
		return "RELEASE";
	default:
		return "???"; /* XXX */
	}
}
#endif
