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

#include <netinet/in.h>

#include "dhcp6c.h"

typedef enum {IAS_ACTIVE, IAS_RENEW, IAS_REBIND} iastate_t;

TAILQ_HEAD(iaprefix_list, iaprefix);

struct iaprefix {
	TAILQ_ENTRY(iaprefix) link;

	struct dhcp6_prefix pa;
	time_t updatetime;
	struct dhcp6_timer *timer;
	struct iactl *ctl;

	union {
		TAILQ_HEAD(, dhcp6_ifprefix) ifprefix_list; /* interface prefixes */
		const struct dhcp6_if *dhcpif;
	} u;
};

struct ia {
	TAILQ_ENTRY(ia) link;

	/* back pointer to configuration */
	struct ia_conf *conf;

	/* common parameters of IA */
	uint32_t t1;				/* duration for renewal */
	uint32_t t2;				/* duration for rebind  */

	/* internal parameters for renewal/rebinding */
	iastate_t state;
	struct dhcp6_timer *timer;

	/* DHCP related parameters */
	struct dhcp6_if *ifp;		/* DHCP interface */
	struct dhcp6_vbuf serverid; /* the server ID that provided this IA */

	/* control information shared with each particular config routine */
	struct iactl *ctl;

	/* authentication parameters for transaction with servers on this IA */
	struct authparam *authparam;
};

struct iactl {
	struct ia *iactl_ia;		/* back pointer to IA */

	int type;

	struct pifc_list *pifc_head;
	struct iaprefix_list iaprefix_head;
};

struct dhcp6_ifprefix {
	TAILQ_ENTRY(dhcp6_ifprefix) plink;

	/* interface configuration */
	struct prefix_ifconf *ifconf;

	/* interface prefix parameters */
	struct in6_addr paddr;
	int plen;

	/* address assigned on the interface based on the prefix */
	struct in6_addr ifaddr;
};


void destruct_iadata(struct dhcp6_eventdata *evd);

static int prepare_iaevent(struct ia *, int);
static int reestablish_ia(struct ia *);
static void remove_ia(struct ia *);
static struct ia *get_ia(struct dhcp6_if *, struct ia_conf *,
    struct dhcp6_listval *, struct dhcp6_vbuf *);
static struct dhcp6_timer *ia_timo(void *);

#if defined CONFIG_UDHCP_DEBUG && CONFIG_UDHCP_DEBUG >= 1
static const char *iastatestr(iastate_t);
#endif

static int update_prefix6(struct ia *, struct dhcp6_listval *,
        struct pifc_list *, const struct dhcp6_if *, struct iactl **);
static struct iaprefix *find_iaprefix(struct iaprefix_list *,
    struct dhcp6_prefix *, int);
static int remove_iaprefix(struct iaprefix *, int);

static int iapref_renew(struct iactl *, struct dhcp6_ia *,
        struct dhcp6_eventdata *);
static ALWAYS_INLINE int pref_isvalid(struct iactl *);
static uint32_t pref_duration(struct iactl *);
static void pref_cleanup(struct iactl *);
static struct dhcp6_timer *iapref_timo(void *);
static int add_ifprefix(struct iaprefix *,
    struct dhcp6_prefix *, struct prefix_ifconf *);


#if ENABLE_FEATURE_DHCP6_AUTH
static struct authparam *copy_authparam(struct authparam *authparam) RETURNS_MALLOC;

static struct authparam *copy_authparam(struct authparam *authparam)
{
	struct authparam *dst = malloc_or_warn(sizeof(*dst));

	if (dst == NULL)
		return NULL;

	memcpy(dst, authparam, sizeof(*dst));

	return dst;
}

static int update_authparam(struct ia *ia, struct authparam *authparam)
{
	if (authparam == NULL)
		return 0;

	if (ia->authparam == NULL) {
		ia->authparam = copy_authparam(authparam);
		if (ia->authparam == NULL) {
			return -1;
		}
		return 0;
	}

	/* update the previous RD value and flags */
	ia->authparam->prevrd = authparam->prevrd;
	ia->authparam->flags = authparam->flags;

	return 0;
}
#endif /* ENABLE_FEATURE_DHCP6_AUTH */

void update_ia(struct dhcp6_list *iahead, struct dhcp6_if *ifp,
	  struct dhcp6_vbuf *serverid
	  IF_FEATURE_DHCP6_AUTH(,struct authparam *authparam))
{
	struct ia *ia;
	struct ia_conf *iac;
	struct dhcp6_listval *iav, *siav;

	TAILQ_FOREACH(iav, iahead, link) {
		/* if we're not interested in this IA, ignore it. */
		iac = find_iaconf(&ifp->iaconf_list, iav->dh6optype, iav->val_ia.iaid);
		if (iac == NULL)
			continue;

		/* validate parameters */
		/*
		 * If a client receives an IA_NA with T1 greater than T2, and
		 * both T1 and T2 are greater than 0, the client discards the
		 * IA_NA option and processes the remainder of the message as
		 * though the server had not included the invalid IA_NA option.
		 * [RFC3315 22.4]
		 * We apply the same rule to IA_PD as well.
		 */
		if (iav->val_ia.t2 != 0 && iav->val_ia.t1 > iav->val_ia.t2) {
			bb_info_msg("invalid IA: T1(%u) > T2(%u)",
			    iav->val_ia.t1, iav->val_ia.t2);
			continue;
		}

		/* locate the local IA or make a new one */
		ia = get_ia(ifp, iac, iav, serverid);
		if (ia == NULL)
			continue;

#if ENABLE_FEATURE_DHCP6_AUTH
		/* update authentication parameters */
		if (update_authparam(ia, authparam)) {
			bb_info_msg("can't update authentication param for %s, ID: %u",
				dhcp6optstr(iac->type), iac->iaid);
			remove_ia(ia);
			continue;
		}
#endif

		/* update IA configuration information */
		TAILQ_FOREACH(siav, &iav->sublist, link) {
			char p[INET6_ADDRSTRLEN];

			switch (siav->lvtype) {
			case DHCP6_LISTVAL_PREFIX6:
				/* add or update the prefix */
				if (update_prefix6(ia, siav, &iac->iapd_pif_list, ifp, &ia->ctl)) {
					sprint_nip6(p, (const uint8_t *)&siav->val_prefix6.addr);
					bb_error_msg("can't update a prefix %s/%d",
					    p, siav->val_prefix6.plen);
				}
				break;
			case DHCP6_LISTVAL_STATEFULADDR6:
				if (update_prefix6(ia, siav, NULL, ifp, &ia->ctl)) {
					sprint_nip6(p, (const uint8_t *)&siav->val_statefuladdr6.addr);
					bb_error_msg("can't update an address %s", p);
				}
				break;
			case DHCP6_LISTVAL_NUM16:
				bb_info_msg("status code for %s-%u: %s",
				    dhcp6optstr(iav->dh6optype), iav->val_ia.iaid,
				    dhcp6stcodestr(siav->val_num16));
				if ((ia->state == IAS_RENEW || ia->state == IAS_REBIND) &&
				    siav->val_num16 == D6_OPT_STCODE_NOBINDING) {
					/*
					 * For each IA in the original Renew or
					 * Rebind message, the client
					 * sends a Request message if the IA
					 * contained a Status Code option
					 * with the NoBinding status.
					 * [RFC3315 18.1.8]
					 * XXX: what about the PD case?
					 */
					bb_info_msg("receive NoBinding against "
					    "renew/rebind for %s-%u",
					    dhcp6optstr(ia->conf->type), ia->conf->iaid);
					reestablish_ia(ia);
					goto nextia;
				}
				break;
			default:
				bb_error_msg("impossible subopt");
				goto nextia;
			}
		}

		/* see if this IA is still valid.  if not, remove it. */
		if (ia->ctl == NULL || !pref_isvalid(ia->ctl)) {
			log1("%s-%u invalidated",
				dhcp6optstr(ia->conf->type), ia->conf->iaid);
			remove_ia(ia);
			continue;
		}

		/* if T1 or T2 is 0, determine appropriate values locally. */
		if (ia->t1 == 0 || ia->t2 == 0) {
			uint32_t duration;

			if (ia->ctl)
				duration = pref_duration(ia->ctl);
			else
				duration = 1800; /* 30min. XXX: no rationale */

			if (ia->t1 == 0) {
				if (duration == DHCP6_DURATION_INFINITE)
					ia->t1 = DHCP6_DURATION_INFINITE;
				else
					ia->t1 = duration / 2;
			}
			if (ia->t2 == 0) {
				if (duration == DHCP6_DURATION_INFINITE)
					ia->t2 = DHCP6_DURATION_INFINITE;
				else
					ia->t2 = duration * 4 / 5;
			}

			/* make sure T1 <= T2 */
			if (ia->t1 > ia->t2)
				ia->t1 = ia->t2 * 5 / 8;

			bb_info_msg("T1(%u) and/or T2(%u) is locally determined",
						ia->t1, ia->t2);
		}

		/*
		 * Be proactive for too-small timeout values.  Note that
		 * the adjusted values may make some information expire
		 * without renewal.
		 */
		if (ia->t2 < DHCP6_DURATION_MIN) {
			bb_info_msg("T1 (%u) or T2 (%u) is too small", ia->t1, ia->t2);
			ia->t2 = DHCP6_DURATION_MIN;
			ia->t1 = ia->t2 * 5 / 8;
			bb_info_msg(" adjusted to %u and %u", ia->t1, ia->t2);
		}

		/* set up a timer for this IA. */
		if (ia->t1 == DHCP6_DURATION_INFINITE) {
			if (ia->timer) {
				dhcp6_timer_remove(&ia->timer);
			}
		} else {
			if (ia->timer == NULL) {
				ia->timer = dhcp6_timer_add(ia_timo, ia);
			}
			dhcp6_timer_set(ia->t1 * 1000ULL, ia->timer);
		}

		ia->state = IAS_ACTIVE;

	  nextia:
		;
	}
}

static int prepare_iaevent(struct ia *ia, int dhcpstate)
{
	struct dhcp6_event *ev;
	struct dhcp6_ia iaparam;
	struct dhcp6_eventdata *evd;

	ev = dhcp6_create_event(ia->ifp, dhcpstate);

	if (dhcpstate == DHCP6S_RELEASE ||
	    dhcpstate == DHCP6S_REQUEST ||
	    dhcpstate == DHCP6S_RENEW
	   ) {
		dhcp6_vbuf_copy(&ev->serverid, &ia->serverid);
	}

	evd = xzalloc(sizeof(*evd));
	evd->event = ev;
	memset(&iaparam, 0, sizeof(iaparam));
	iaparam.iaid = ia->conf->iaid;
	/* XXX: should we set T1/T2 to 0 on release?  spec is silent on this. */
	iaparam.t1 = ia->t1;
	iaparam.t2 = ia->t2;

	if (ia->ctl && iapref_renew(ia->ctl, &iaparam, evd)) {
		bb_error_msg("can't prepare IA data");
			goto fail;
	}
	TAILQ_INSERT_TAIL(&ev->data_list, evd, link);

#if ENABLE_FEATURE_DHCP6_AUTH
	if (ia->authparam != NULL) {
		ev->authparam = copy_authparam(ia->authparam);
		if (ev->authparam == NULL) {
			goto fail;
		}
	}
#endif

	ev->timeouts = 0;
	dhcp6_set_timeoparam(ev);
	dhcp6_reset_timer(ev);

	client6_send(ev);
	return 0;

fail:
	if (ev)
		dhcp6_remove_event(ev);
	return -1;
}

static int reestablish_ia(struct ia *ia)
{

	log1("re-establishing %s-%u",
			dhcp6optstr(ia->conf->type), ia->conf->iaid);

	if (ia->state != IAS_RENEW && ia->state != IAS_REBIND) {
		bb_error_msg_and_die("invalid IA state (%d)", ia->state);
	}

	/* we don't need a timer for the IA (see comments in ia_timo()) */
	if (ia->timer)
		dhcp6_timer_remove(&ia->timer);

	return prepare_iaevent(ia, DHCP6S_REQUEST);
}

void release_all_ia(struct dhcp6_if *ifp)
{
	struct ia_conf *iac;
	struct ia *ia, *ia_next;

	TAILQ_FOREACH(iac, &ifp->iaconf_list, link) {
		for (ia = TAILQ_FIRST(&iac->iadata); ia; ia = ia_next) {
			ia_next = TAILQ_NEXT(ia, link);

			log1("release an %s-%u",
				dhcp6optstr(ia->conf->type), ia->conf->iaid);

			prepare_iaevent(ia, DHCP6S_RELEASE);

			/*
			 * The client MUST stop using all of the addresses
			 * being released as soon as the client begins the
			 * Release message exchange process.
			 * [RFC3315 Section 18.1.6]
			 */
			remove_ia(ia);
		}
	}
}

static void remove_ia(struct ia *ia)
{
	struct ia_conf *iac = ia->conf;
	struct dhcp6_if *ifp = ia->ifp;

	log1("remove an %s-%u", dhcp6optstr(ia->conf->type), ia->conf->iaid);

	TAILQ_REMOVE(&iac->iadata, ia, link);

	dhcp6_vbuf_free(&ia->serverid);

	if (ia->timer)
		dhcp6_timer_remove(&ia->timer);

	if (ia->ctl)
		pref_cleanup(ia->ctl);

	free(ia->authparam);
	free(ia);

	client6_start(ifp);
}

static struct dhcp6_timer *ia_timo(void *arg)
{
	struct ia *ia = (struct ia *)arg;
	int dhcpstate;

	log1("timeout for %s-%u, state=%s",
	    dhcp6optstr(ia->conf->type), ia->conf->iaid, iastatestr(ia->state));

	switch (ia->state) {
	case IAS_ACTIVE:
		ia->state = IAS_RENEW;
		dhcpstate = DHCP6S_RENEW;
		dhcp6_timer_set((ia->t1 < ia->t2) ? (ia->t2 - ia->t1) * 1000ULL : 0,
						ia->timer);
		break;
	case IAS_RENEW:
		ia->state = IAS_REBIND;
		dhcpstate = DHCP6S_REBIND;

		/*
		 * We need keep DUID for sending Release in this state.
		 * But we don't need a timer for the IA.  We'll just wait for a
		 * reply for the REBIND until all associated configuration
		 * parameters for this IA expire.
		 */
		dhcp6_timer_remove(&ia->timer);
		break;
	default:
		bb_error_msg("invalid IA state (%d)", ia->state);
		return NULL;	/* XXX */
	}

	if (prepare_iaevent(ia, dhcpstate) == 0)
		return ia->timer;
	else
		return NULL;
}

static struct ia *get_ia(struct dhcp6_if *ifp,
	struct ia_conf *iac, struct dhcp6_listval *iaparam,
	struct dhcp6_vbuf *serverid)
{
	struct ia *ia;
	IF_UDHCP_VERBOSE(bool create = false;)

	TAILQ_FOREACH(ia, &iac->iadata, link) {
		if (ia->conf->type == iaparam->dh6optype &&
		    ia->conf->iaid == iaparam->val_ia.iaid)
			break;
	}

	if (ia == NULL) {
		ia = xzalloc(sizeof(*ia));
		ia->state = IAS_ACTIVE;

		TAILQ_INSERT_TAIL(&iac->iadata, ia, link);
		ia->conf = iac;

		IF_UDHCP_VERBOSE(create = true;)
	} else {
		dhcp6_vbuf_free(&ia->serverid);
	}

	ia->t1 = iaparam->val_ia.t1;
	ia->t2 = iaparam->val_ia.t2;
	ia->ifp = ifp;
	dhcp6_vbuf_copy(&ia->serverid, serverid);

	log1("%s an %s-%u", create ? "make" : "update",
	    dhcp6optstr(iaparam->dh6optype), ia->conf->iaid);

	return ia;
}


void destruct_iadata(struct dhcp6_eventdata *evd)
{
	struct dhcp6_list *ial;

	if (evd->type != D6_OPT_IA_PD && evd->type != D6_OPT_IA_NA) {
		bb_error_msg_and_die("impossible IA type %d", evd->type);
	}

	ial = (struct dhcp6_list *)evd->data;
	dhcp6_clear_list(ial);
	free(ial);

	free(evd);
}

#if defined CONFIG_UDHCP_DEBUG && CONFIG_UDHCP_DEBUG >= 1
static const char *iastatestr(iastate_t state)
{
	switch (state) {
	case IAS_ACTIVE:
		return "ACTIVE";
	case IAS_RENEW:
		return "RENEW";
	case IAS_REBIND:
		return "REBIND";
	default:
		return "???";	/* should be a bug */
	}
}
#endif


static int update_prefix6(struct ia *ia, struct dhcp6_listval *lv,
		struct pifc_list *pifc,	const struct dhcp6_if * dhcpifp,
		struct iactl **ctlp)
{
	struct iactl *iac = (struct iactl *)(*ctlp);
	struct iaprefix *sp;
	struct dhcp6_prefix pinfo;
	struct prefix_ifconf *pif;
	int spcreate = 0;
	char p[INET6_ADDRSTRLEN];

	switch (lv->lvtype) {
		case DHCP6_LISTVAL_PREFIX6:
			memcpy(&pinfo, &lv->uv.uv_prefix6, sizeof(pinfo));
			break;
		case DHCP6_LISTVAL_STATEFULADDR6:
			memcpy(&pinfo, &lv->uv.uv_statefuladdr6, sizeof(struct dhcp6_statefuladdr));
			pinfo.plen = 128;
			break;
		default:
			break;
	}

	/*
	 * A client discards any addresses for which the preferred
         * lifetime is greater than the valid lifetime.
	 * [RFC3315 22.6]
	 */
	sprint_nip6(p, (const uint8_t *)&pinfo.addr);
	if (pinfo.vltime != DHCP6_DURATION_INFINITE &&
	    (pinfo.pltime == DHCP6_DURATION_INFINITE ||
	    pinfo.pltime > pinfo.vltime)) {
		bb_info_msg("%s/%d has larger preferred lifetime than valid lifetime",
				p, pinfo.plen);
		return -1;
	}

	if (iac == NULL) {
		iac = xzalloc(sizeof(*iac));

		iac->iactl_ia = ia;
		iac->type = (lv->lvtype == DHCP6_LISTVAL_PREFIX6) ?
					D6_OPT_IA_PD : D6_OPT_IA_NA;

		iac->pifc_head = pifc;
		TAILQ_INIT(&iac->iaprefix_head);
		*ctlp = (struct iactl *)iac;
	}

	/* search for the given prefix, and make a new one if it fails */
	sp = find_iaprefix(&iac->iaprefix_head, &pinfo,
					(lv->lvtype==DHCP6_LISTVAL_PREFIX6));
	if (sp == NULL) {
		sp = xzalloc(sizeof(*sp));
		sp->pa.addr = pinfo.addr;
		sp->pa.plen = pinfo.plen;
		sp->ctl = iac;

		if (lv->lvtype == DHCP6_LISTVAL_PREFIX6)
			TAILQ_INIT(&sp->u.ifprefix_list);

		TAILQ_INSERT_TAIL(&iac->iaprefix_head, sp, link);

		spcreate = 1;
	}

	/* update the timestamp of update */
	sp->updatetime = time(NULL);

	/* update the prefix according to pinfo */
	sp->pa.pltime = pinfo.pltime;
	sp->pa.vltime = pinfo.vltime;
	if (lv->lvtype == DHCP6_LISTVAL_STATEFULADDR6)
		sp->u.dhcpif = dhcpifp;

	log2("%s a address %s/%d pltime=%u, vltime=%u",
	    spcreate ? "create" : "update",
	    p, pinfo.plen, pinfo.pltime, pinfo.vltime);

	/* update prefix interfaces if necessary */
	if (sp->pa.vltime != 0) {
		if (lv->lvtype == DHCP6_LISTVAL_PREFIX6 && spcreate) {
			TAILQ_FOREACH(pif, iac->pifc_head, link) {
			/*
			 * The requesting router MUST NOT assign any delegated
			 * prefixes or subnets from the delegated prefix(es) to
			 * the link through which it received the DHCP message
			 * from the delegating router.
			 * [RFC3633 Section 12.1]
			 */
				if (strcmp(pif->ifname, dhcpifp->ifname) == 0) {
					bb_info_msg("skip %s as a prefix interface",
					    dhcpifp->ifname);
					continue;
				}

				add_ifprefix(sp, &pinfo, pif);
			}
		}
		else if (lv->lvtype == DHCP6_LISTVAL_STATEFULADDR6) {
			if (ifaddrconf(IFADDRCONF_ADD, sp->u.dhcpif->ifname, &sp->pa.addr,
				 128, sp->pa.pltime, sp->pa.vltime) < 0)
				return -1;
		}
	}

	/*
	 * If the new vltime is 0, this prefix immediately expires.
	 * Otherwise, set up or update the associated timer.
	 */
	switch (sp->pa.vltime) {
	case 0:
		remove_iaprefix(sp, (lv->lvtype == DHCP6_LISTVAL_PREFIX6) ?
				     D6_OPT_IA_PD : D6_OPT_IA_NA);
		break;
	case DHCP6_DURATION_INFINITE:
		if (sp->timer)
			dhcp6_timer_remove(&sp->timer);
		break;
	default:
		if (sp->timer == NULL) {
			sp->timer = dhcp6_timer_add(iapref_timo, sp);
		}
		/* update the timer */
		dhcp6_timer_set(sp->pa.vltime * 1000ULL, sp->timer);
		break;
	}

	return 0;
}

static struct iaprefix *find_iaprefix(struct iaprefix_list *head, struct dhcp6_prefix *addr,
		int match_plen)
{
	struct iaprefix *sp;

	TAILQ_FOREACH(sp, head, link) {
		if (!IN6_ARE_ADDR_EQUAL(&sp->pa.addr, &addr->addr))
			continue;
		if (match_plen == 0 || sp->pa.plen == addr->plen)
			return sp;
	}

	return NULL;
}

static int remove_iaprefix(struct iaprefix *sp, int type)
{
	int ret = 0;
#if defined CONFIG_UDHCP_DEBUG && CONFIG_UDHCP_DEBUG >= 1
	char a[INET6_ADDRSTRLEN];

	sprint_nip6(a, (const uint8_t *)&sp->pa.addr);
	log2("remove an address %s/%d", a, sp->pa.plen);
#endif

	if (sp->timer)
		dhcp6_timer_remove(&sp->timer);

	if (type == D6_OPT_IA_PD) {
		struct dhcp6_ifprefix *ip;

		/* remove all interface prefixes */
		while ((ip = TAILQ_FIRST(&sp->u.ifprefix_list)) != NULL) {
			TAILQ_REMOVE(&sp->u.ifprefix_list, ip, plink);
			ifaddrconf(IFADDRCONF_REMOVE, ip->ifconf->ifname,
						&ip->ifaddr, ip->plen,
						DHCP6_DURATION_INFINITE, DHCP6_DURATION_INFINITE);
			free(ip);
		}
	} else {
		ret = ifaddrconf(IFADDRCONF_REMOVE, sp->u.dhcpif->ifname, &sp->pa.addr,
						128, sp->pa.pltime, sp->pa.vltime);
	}

	TAILQ_REMOVE(&sp->ctl->iaprefix_head, sp, link);
	free(sp);
	return ret;
}

static ALWAYS_INLINE int pref_isvalid(struct iactl *iac)
{
	if (TAILQ_EMPTY(&iac->iaprefix_head))
		return 0;	/* this IA is invalid */
	return 1;
}

static uint32_t pref_duration(struct iactl *iac)
{
	struct iaprefix *sp;
	uint32_t base = DHCP6_DURATION_INFINITE, pltime, passed;
	time_t now;

	/* Determine the smallest period until pltime expires. */
	now = time(NULL);
	TAILQ_FOREACH(sp, &iac->iaprefix_head, link) {
		passed = now > sp->updatetime ? (uint32_t)(now - sp->updatetime) : 0;
		pltime = sp->pa.pltime > passed ? sp->pa.pltime - passed : 0;

		if (base == DHCP6_DURATION_INFINITE || pltime < base)
			base = pltime;
	}

	return base;
}

static void pref_cleanup(struct iactl *iac)
{
	struct iaprefix *sp;

	while ((sp = TAILQ_FIRST(&iac->iaprefix_head)) != NULL) {
		TAILQ_REMOVE(&iac->iaprefix_head, sp, link);
		remove_iaprefix(sp, iac->type);
	}

	free(iac);
}

static int iapref_renew(struct iactl *iac, struct dhcp6_ia *iaparam,
		struct dhcp6_eventdata *evd)
{
	struct iaprefix *sp;
	struct dhcp6_list *ial = NULL, pl;

	TAILQ_INIT(&pl);
	TAILQ_FOREACH(sp, &iac->iaprefix_head, link) {
		if (dhcp6_add_listval(&pl, iac->type,
		    (iac->type == D6_OPT_IA_PD) ? DHCP6_LISTVAL_PREFIX6 : DHCP6_LISTVAL_STATEFULADDR6,
		    &sp->pa, NULL) == NULL)
			goto fail;
	}

	ial = xmalloc(sizeof(*ial));
	TAILQ_INIT(ial);
	if (dhcp6_add_listval(ial, iac->type, DHCP6_LISTVAL_IA, iaparam, &pl) == NULL)
		goto fail;
	dhcp6_clear_list(&pl);

	evd->type = iac->type;
	evd->data = ial;

	return 0;

  fail:
	dhcp6_clear_list(&pl);
	free(ial);
	return -1;
}

static struct dhcp6_timer *iapref_timo(void *arg)
{
	struct iaprefix *sp = (struct iaprefix *)arg;
	struct ia *ia = sp->ctl->iactl_ia;
#if defined CONFIG_UDHCP_DEBUG && CONFIG_UDHCP_DEBUG >= 1
	char a[INET6_ADDRSTRLEN];

	sprint_nip6(a, (const uint8_t *)&sp->pa.addr);
	log1("address timeout for %s/%d", a, sp->pa.plen);
#endif

	if (sp->timer)
		dhcp6_timer_remove(&sp->timer);

	remove_iaprefix(sp, sp->ctl->type);

	/* see if this IA is still valid.  if not, remove it. */
	if (ia->ctl == NULL || !pref_isvalid(ia->ctl)) {
		log1("%s-%u is invalidated",
		    dhcp6optstr(ia->conf->type), ia->conf->iaid);
		remove_ia(ia);
	}

	return NULL;
}

static int add_ifprefix(struct iaprefix *iaprefix,
		struct dhcp6_prefix *prefix, struct prefix_ifconf *pconf)
{
	struct dhcp6_ifprefix *ifpfx = NULL;
	uint32_t sla_id;
	char *sp;
	int b, i;

	ifpfx = xzalloc(sizeof(*ifpfx));

	ifpfx->ifconf = pconf;

	ifpfx->paddr = prefix->addr;
	ifpfx->plen = prefix->plen + pconf->sla_len;
	/*
	 * XXX: our current implementation assumes ifid len is a multiple of 8
	 */
	if ((pconf->ifid_len % 8) != 0) {
		bb_error_msg("length of interface ID must be multiple of 8");
		goto bad;
	}
	if (ifpfx->plen + pconf->ifid_len < 0 ||
	    ifpfx->plen + pconf->ifid_len > 128) {
		bb_info_msg("invalid prefix length: %d + %d + %d",
			prefix->plen, pconf->sla_len, pconf->ifid_len);
		goto bad;
	}

	/* copy prefix and SLA ID */
	b = prefix->plen;
	for (i = 0, b = prefix->plen; b > 0; b -= 8, i++) {
		ifpfx->paddr.s6_addr[i] = prefix->addr.s6_addr[i];
	}
	sla_id = htonl(pconf->sla_id);
	sp = ((char *)&sla_id + 3);
	i = (128 - pconf->ifid_len) / 8;
	for (b = pconf->sla_len; b > 7; b -= 8, sp--) {
		ifpfx->paddr.s6_addr[--i] = *sp;
	}
	if (b) {
		ifpfx->paddr.s6_addr[--i] |= *sp;
	}

	/* configure the corresponding address */
	ifpfx->ifaddr = ifpfx->paddr;
	for (i = 15; i >= pconf->ifid_len / 8; i--) {
		ifpfx->ifaddr.s6_addr[i] = pconf->ifid[i];
	}
	if (ifaddrconf(IFADDRCONF_ADD, ifpfx->ifconf->ifname, &ifpfx->ifaddr,
		 ifpfx->plen, DHCP6_DURATION_INFINITE, DHCP6_DURATION_INFINITE))
		goto bad;

	/* TODO: send a control message for other processes */

	TAILQ_INSERT_TAIL(&iaprefix->u.ifprefix_list, ifpfx, plink);

	return 0;

  bad:
	free(ifpfx);
	return -1;
}
