/* vi: set sw=4 ts=4: */
/*
 * DHCPv6 client
 *
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
 *
 * Original code taken from wide-dhcpv6-20080615 and converted into
 * a busybox applet by Leonid Lisovskiy <lly@sf.net>
 */

#include <net/if.h>
#include <syslog.h>

/* Override ENABLE_FEATURE_PIDFILE */
#define WANT_PIDFILE 1
#include "dhcp6c.h"
#include "dhcp6c_auth.h"

#define DHCP6C_CONF     "/etc/dhcp6c.conf"
#define DHCP6C_PIDFILE  "/var/run/dhcp6c.pid"
#define DUID_FILE       "/tmp/dhcp6c_duid"

struct globals {
	len_and_sockaddr *sa6_allagent;

	const char *pid_file;

	int csock;	/* inbound/outbound udp port */

	char rbuf[BUFSIZ], sbuf[BUFSIZ];

	struct dhcp6_vbuf client_duid;
	int duid_type;
} FIX_ALIASING;

void BUG_dhcp6c_globals_too_big(void);
#define G (*ptr_to_globals)
#define csock		(G.csock        )
#define INIT_G() do { \
	if (sizeof(struct client6_config_t) > COMMON_BUFSIZE / 2) \
		BUG_dhcp6c_globals_too_big(); \
	SET_PTR_TO_GLOBALS(xzalloc(sizeof(G))); \
	G.pid_file = DHCP6C_PIDFILE; \
	G.duid_type = 1; \
} while (0)


/* dhcp6c_ia.c */
extern void destruct_iadata(struct dhcp6_eventdata *evd);

static void if6init(struct dhcp6_if *ifp, const char *ifname);
static void dhcp6_remove_evdata(struct dhcp6_event *);
static void client6_startall(int isrestart);
static void client6_mainloop(void);
static void free_resources(int force);
static void check_exit(int force);
static struct dhcp6_serverinfo *find_server(struct dhcp6_event *,
						 struct dhcp6_vbuf *);
static struct dhcp6_serverinfo *select_server(struct dhcp6_event *);
static void client6_recv(void);
static int client6_recvadvert(const struct dhcp6_if *, struct dhcp6_event *,
				   struct dhcp6_optinfo *);
static int client6_recvreply(struct dhcp6_if *, struct dhcp6_event *,
				  struct dhcp6_optinfo *);
static struct dhcp6_event *find_event_withid(unsigned int ifid, uint32_t xid);
static int construct_confdata(const struct dhcp6_if *, struct dhcp6_event *);
static int construct_reqdata(const struct dhcp6_if *, struct dhcp6_optinfo *,
    struct dhcp6_event *);
static struct dhcp6_timer *client6_expire_refreshtime(void *);
static struct dhcp6_timer *client6_timo(void *arg);

#if ENABLE_FEATURE_DHCP6_AUTH
static int process_auth(struct authparam *, struct dhcp6 *dh6, ssize_t,
    struct dhcp6_optinfo *);
static int set_auth(struct dhcp6_event *, struct dhcp6_optinfo *);
#endif

enum {
        OPT_FOREGROUND = 1 << 0,
        OPT_i = 1 << 1,
        OPT_c = 1 << 2,
        OPT_D = 1 << 3,
        OPT_p = 1 << 4,
        OPT_VERBOSE = 1 << 5,
};

//usage:#define dhcp6c_trivial_usage
//usage:       "[-vfi] [-c FILE] [-D LL|LLT] [-p FILE] interface"
//usage:#define dhcp6c_full_usage "\n"
//usage:      "\n	-v         Print debugging messages(may be repeated)"
//usage:      "\n	-f         Foreground mode"
//usage:      "\n	-i         Info-req(information-only) mode"
//usage:      "\n	-c FILE    Use FILE as the configuration file"
//usage:      "\n	-D LL|LLT  Use LL or LLT to override default(LLT) DUID type generation"
//usage:      "\n	-p FILE    Use FILE to dump the process ID of dhcp6c"

int dhcp6c_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int dhcp6c_main(int argc UNUSED_PARAM, char **argv)
{
	int opt;
	const char *str_D;

	INIT_G();
	client6_config.conffile = DHCP6C_CONF;

	srandom(getpid());

	opt_complementary = "=1" IF_UDHCP_VERBOSE(":vv");
	opt = getopt32(argv, "fic:D:p:v",
			&client6_config.conffile, &str_D, &G.pid_file
			IF_UDHCP_VERBOSE(, &dhcp_verbose)
			);
	if (opt & OPT_D) {
		if (strcasecmp(str_D, "LL") == 0)
			G.duid_type = 3;
		else if (strcasecmp(str_D, "LLT") == 0)
			G.duid_type = 1;
	}

	if (!(opt & OPT_FOREGROUND) && !(opt & OPT_i)) {
		bb_daemonize_or_rexec(DAEMON_CLOSE_EXTRA_FDS, argv);
		openlog(applet_name, LOG_PID, LOG_DAEMON);
		logmode = LOGMODE_SYSLOG;
	}

	bb_signals(0 + (1 << SIGHUP) + (1 << SIGTERM) + (1 << SIGQUIT),
		  record_signo);

	if6init(&client6_config.dhcp6c_if, argv[optind]);

	if (!(opt & OPT_i) && (read_config6(client6_config.conffile)) != 0) {
		bb_error_msg_and_die("can't parse config");
	}

	bb_info_msg("started");
	/* get our DUID */
	if (get_duid(DUID_FILE, &G.client_duid, G.duid_type)) {
		bb_error_msg_and_die("can't get a DUID");
	}

	G.sa6_allagent = xhost_and_af2sockaddr(DH6ADDR_ALLAGENT, DH6PORT_UPSTREAM, AF_INET6);

	csock = sock6_init(NULL, xstr(DH6PORT_DOWNSTREAM));
	setsockopt_bindtodevice(csock, client6_config.dhcp6c_if.ifname);
	if (setsockopt_1(csock, IPPROTO_IPV6, IPV6_MULTICAST_LOOP) < 0) {
		bb_perror_msg_and_die("setsockopt(IPV6_MULTICAST_LOOP)");
	}

	LIST_INIT(&client6_config.timer_head);
	client6_config.tm_sentinel = ULLONG_MAX;
	TAILQ_INIT(&client6_config.event_list);

	/* dump current PID */
	write_pidfile(G.pid_file);

	client6_startall(0);
	client6_mainloop();
	return 0;
}


/*------------------------------------------------------------*/

static void if6init(struct dhcp6_if *ifp, const char *ifname)
{
	ifp->ifid = if_nametoindex(ifname);
	if (ifp->ifid == 0) {
		bb_perror_msg_and_die("invalid interface(%s)", ifp->ifname);
	}

	ifp->ifname = xstrdup(ifname);

	TAILQ_INIT(&ifp->reqopt_list);
	TAILQ_INIT(&ifp->iaconf_list);
#if ENABLE_FEATURE_DHCP6_AUTH
	ifp->authproto = -1;
	ifp->authalgorithm = -1;
	ifp->authrdm = -1;
#endif
}

struct dhcp6_event *dhcp6_create_event(const struct dhcp6_if *ifp, int state)
{
	struct dhcp6_event *ev;

	ev = xzalloc(sizeof(*ev));
	ev->ifp = ifp;
	ev->state = state;
	TAILQ_INIT(&ev->data_list);

	TAILQ_INSERT_TAIL(&client6_config.event_list, ev, link);

	ev->timer = dhcp6_timer_add(client6_timo, ev);

	return ev;
}

void dhcp6_remove_event(struct dhcp6_event *ev)
{
	struct dhcp6_serverinfo *sp, *sp_next;

	log1("removing an event on %s, state=%s",
	    ev->ifp->ifname, dhcp6_event_statestr(ev));

	if (ev->timer)
		dhcp6_timer_remove(&ev->timer);

	dhcp6_remove_evdata(ev);
	dhcp6_vbuf_free(&ev->serverid);
	TAILQ_REMOVE(&client6_config.event_list, ev, link);

	sp = ev->servers;
	while (sp) {
		sp_next = sp->next;

		log1("removing server (ID: %s)", duidstr(&sp->optinfo.serverID));
		dhcp6_clear_options(&sp->optinfo);
#if ENABLE_FEATURE_DHCP6_AUTH
		free(sp->authparam);
#endif
		free(sp);
		sp = sp_next;
	}

#if ENABLE_FEATURE_DHCP6_AUTH
	free(ev->authparam);
#endif
	free(ev);
}

static void dhcp6_remove_evdata(struct dhcp6_event *ev)
{
	struct dhcp6_eventdata *evd;

	while ((evd = TAILQ_FIRST(&ev->data_list)) != NULL) {
		TAILQ_REMOVE(&ev->data_list, evd, link);
		destruct_iadata(evd);
	}
}

int client6_start(struct dhcp6_if *ifp)
{
	struct dhcp6_event *ev;

	/* make sure that the interface does not have a timer */
	if (ifp->timer)
		dhcp6_timer_remove(&ifp->timer);

	/* create an event for the initial delay */
	ev = dhcp6_create_event(ifp, DHCP6S_INIT);

#if ENABLE_FEATURE_DHCP6_AUTH
	ev->authparam = new_authparam(ifp->authproto,
	    ifp->authalgorithm, ifp->authrdm);
#endif

	dhcp6_reset_timer(ev);

	return 0;
}

static void client6_startall(int isrestart)
{
	struct dhcp6_if *ifp = &client6_config.dhcp6c_if;

	if (isrestart) {
		/* reload the configuration file */
		if (read_config6(client6_config.conffile))
			bb_error_msg_and_die("can't parse config");

		ifp->ifid = if_nametoindex(ifp->ifname);
		if (ifp->ifid == 0)
			bb_error_msg_and_die("invalid interface(%s)", ifp->ifname);
	}

	if (client6_start(ifp)) {
		exit(1); /* initialization failure.  we give up. */
	}
}

static void free_resources(int force)
{
	struct dhcp6_event *ev, *ev_next;

	/* release all IAs as well as send RELEASE message(s) */
	release_all_ia(&client6_config.dhcp6c_if);

	/*
	 * Cancel all outstanding events for each interface except
	 * ones being released.
	 */
	for (ev = TAILQ_FIRST(&client6_config.event_list); ev; ev = ev_next) {
		ev_next = TAILQ_NEXT(ev, link);

		if (!force && ev->state == DHCP6S_RELEASE)
			continue; /* keep it for now */

		dhcp6_remove_event(ev);
	}
}

static void check_exit(int force)
{
	if (!force) {
		if (!bb_got_signal)
			return;

		/*
		 * Check if we have an outstanding event.  If we do, we cannot
		 * exit for now.
		 */
		if (!TAILQ_EMPTY(&client6_config.event_list))
			return;
	}

	/* We have no existing event.  Do exit. */
	bb_info_msg("exiting");
	free_resources(1);
	if (ENABLE_FEATURE_CLEAN_UP) {
		free(G.sa6_allagent);
		dhcp6_vbuf_free(&G.client_duid);
		clear_ifconf(1);
	}
	remove_pidfile(G.pid_file);

	exit(0);
}

static void client6_mainloop(void)
{
	struct timeval *w;
	int ret, maxsock;
	fd_set r;

	while (1) {
		if (bb_got_signal) {
			switch (bb_got_signal) {
			case SIGQUIT:
			case SIGTERM:
				check_exit(1);
				break;
			case SIGHUP:
				bb_info_msg("restarting");
				bb_got_signal = 0;
				free_resources(0);
				client6_startall(1);
				break;
			}
		}

		w = dhcp6_timer_check();

		FD_ZERO(&r);
		FD_SET(csock, &r);
		maxsock = csock;

		ret = select(maxsock + 1, &r, NULL, NULL, w);

		switch (ret) {
		case -1:
			if (errno != EINTR) {
				bb_perror_msg_and_die("select");
			}
			continue;
		case 0:	/* timeout */
			break;	/* dhcp6_timer_check() will treat the case */
		default:
			break;
		}
		if (FD_ISSET(csock, &r))
			client6_recv();
	}
}

static struct dhcp6_timer *client6_expire_refreshtime(void *arg)
{
	struct dhcp6_if *ifp = (struct dhcp6_if *)arg;

	log1("information refresh time on %s expired", ifp->ifname);

	dhcp6_timer_remove(&ifp->timer);
	client6_start(ifp);

	return NULL;
}

static struct dhcp6_timer *client6_timo(void *arg)
{
	struct dhcp6_event *ev = (struct dhcp6_event *)arg;
	const struct dhcp6_if *ifp;
	int state = ev->state;

	ifp = ev->ifp;
	ev->timeouts++;

	/*
	 * Unless MRC is zero, the message exchange fails once the client has
	 * transmitted the message MRC times.
	 * [RFC3315 14.]
	 */
	if (ev->max_retrans_cnt && ev->timeouts >= ev->max_retrans_cnt) {
		bb_info_msg("no responses were received");
		dhcp6_remove_event(ev);

		if (state == DHCP6S_RELEASE)
			check_exit(0);

		return NULL;
	}

	switch (ev->state) {
	case DHCP6S_INIT:
		ev->timeouts = 0; /* indicate to generate a new XID. */
		if ((ifp->send_flags & DHCIFF_INFO_ONLY) || (option_mask32 & OPT_i)) {
			ev->state = DHCP6S_INFOREQ;
		} else {
			ev->state = DHCP6S_SOLICIT;
			if (construct_confdata(ifp, ev)) {
				bb_error_msg_and_die("can't send solicit"); /* XXX */
			}
		}
		dhcp6_set_timeoparam(ev); /* XXX */
		/* fall through */
	case DHCP6S_REQUEST:
	case DHCP6S_RELEASE:
	case DHCP6S_INFOREQ:
		client6_send(ev);
		break;
	case DHCP6S_RENEW:
	case DHCP6S_REBIND:
		if (!TAILQ_EMPTY(&ev->data_list)) {
			client6_send(ev);
		} else {
			bb_info_msg("all information to be updated was canceled");
			dhcp6_remove_event(ev);
			return NULL;
		}
		break;
	case DHCP6S_SOLICIT:
		if (ev->servers) {
			/*
			 * Send a Request to the best server.
			 * Note that when we set Rapid-commit in Solicit,
			 * but a direct Reply has been delayed (very much),
			 * the transition to DHCP6S_REQUEST (and the change of
			 * transaction ID) will invalidate the reply even if it
			 * ever arrives.
			 */
			ev->current_server = select_server(ev);
			if (ev->current_server == NULL) {
				/* this should not happen! */
				bb_error_msg_and_die("can't find a server");
			}
			dhcp6_vbuf_copy(&ev->serverid, &ev->current_server->optinfo.serverID);
			ev->timeouts = 0;
			ev->state = DHCP6S_REQUEST;
			dhcp6_set_timeoparam(ev);
#if ENABLE_FEATURE_DHCP6_AUTH
			free(ev->authparam);
			ev->authparam = ev->current_server->authparam;
			ev->current_server->authparam = NULL;
#endif
			if (construct_reqdata(ifp, &ev->current_server->optinfo, ev)) {
				bb_info_msg("can't construct request");
				break;
			}
		}
		client6_send(ev);
		break;
	}

	dhcp6_reset_timer(ev);

	return ev->timer;
}

static int construct_confdata(const struct dhcp6_if *ifp, struct dhcp6_event *ev)
{
	struct ia_conf *iac;
	struct dhcp6_eventdata *evd = NULL;
	struct dhcp6_list *ial = NULL;
	struct dhcp6_ia iaparam;

	TAILQ_FOREACH(iac, &ifp->iaconf_list, link) {
		/* ignore IA config currently used */
		if (!TAILQ_EMPTY(&iac->iadata))
			continue;

		memset(&iaparam, 0, sizeof(iaparam));
		iaparam.iaid = iac->iaid;

		switch (iac->type) {
		case D6_OPT_IA_PD:
		case D6_OPT_IA_NA:
			ial = xmalloc(sizeof(*ial));
			TAILQ_INIT(ial);

			if (dhcp6_add_listval(ial, iac->type, DHCP6_LISTVAL_IA,
			    &iaparam, &iac->prefix_list) == NULL) {
				goto fail;
			}
			break;
		default:
			bb_error_msg_and_die("impossible IA type %d", iac->type);
		}

		evd = xzalloc(sizeof(*evd));
		evd->type = iac->type;
		evd->data = ial;
		evd->event = ev;
		TAILQ_INSERT_TAIL(&ev->data_list, evd, link);
	}

	return 0;

  fail:
	free(evd);
	free(ial);
	dhcp6_remove_event(ev);	/* XXX */

	return -1;
}

static int construct_reqdata(const struct dhcp6_if *ifp, struct dhcp6_optinfo *optinfo,
		  struct dhcp6_event *ev)
{
	struct ia_conf *iac;
	struct dhcp6_eventdata *evd = NULL;
	struct dhcp6_list *ial = NULL;
	struct dhcp6_ia iaparam;

	/* discard previous event data */
	dhcp6_remove_evdata(ev);

	if (optinfo == NULL)
		return 0;

	TAILQ_FOREACH(iac, &ifp->iaconf_list, link) {
		struct dhcp6_listval *lv;

		/* ignore IA config currently used */
		if (!TAILQ_EMPTY(&iac->iadata))
			continue;

		memset(&iaparam, 0, sizeof(iaparam));
		iaparam.iaid = iac->iaid;

		switch (iac->type) {
		case D6_OPT_IA_PD:
		case D6_OPT_IA_NA:
			lv = dhcp6_find_listval(&optinfo->ia_list,
			    iac->type, &iaparam, MATCHLIST_DH6OPTYPE);
			if (lv == NULL)
				continue;

			ial = xmalloc(sizeof(*ial));
			TAILQ_INIT(ial);

			if (dhcp6_add_listval(ial, iac->type, DHCP6_LISTVAL_IA,
			    &iaparam, &lv->sublist) == NULL) {
				goto fail;
			}
			break;
		default:
			bb_error_msg_and_die("impossible IA type %d", iac->type);
		}

		evd = xzalloc(sizeof(*evd));
		evd->type = iac->type;
		evd->data = ial;
		evd->event = ev;
		TAILQ_INSERT_TAIL(&ev->data_list, evd, link);
	}

	return 0;

  fail:
	free(evd);
	free(ial);
	dhcp6_remove_event(ev);	/* XXX */

	return -1;
}

static struct dhcp6_serverinfo *select_server(struct dhcp6_event *ev)
{
	struct dhcp6_serverinfo *s = ev->servers;

	/*
	 * pick the best server according to RFC3315 Section 17.1.3.
	 * XXX: we currently just choose the one that is active and has the
	 * highest preference.
	 */
	while (s) {
		if (s->active) {
			log1("picked a server (ID: %s)", duidstr(&s->optinfo.serverID));
			return s;
		}
		s = s->next;
	}

	return NULL;
}

static struct dhcp6_serverinfo *find_server(struct dhcp6_event *ev,
					 struct dhcp6_vbuf *duid)
{
	struct dhcp6_serverinfo *s = ev->servers;

	while (s) {
		if (dhcp6_vbuf_cmp(&s->optinfo.serverID, duid) == 0)
			return s;

		s = s->next;
	}

	return NULL;
}

void client6_send(struct dhcp6_event *ev)
{
	const struct dhcp6_if *ifp = ev->ifp;
	struct dhcp6 *dh6 = (struct dhcp6 *)G.sbuf;
	struct sockaddr_in6 dst;
	struct dhcp6_optinfo optinfo;
	ssize_t optlen, len;
	struct dhcp6_eventdata *evd;

	len = sizeof(struct dhcp6);
	memset(dh6, 0, len);
	dh6->dh6_msgtype = ev->state;

	if (ev->timeouts == 0) {
		/*
		 * A client SHOULD generate a random number that cannot easily
		 * be guessed or predicted to use as the transaction ID for
		 * each new message it sends.
		 *
		 * A client MUST leave the transaction-ID unchanged in
		 * retransmissions of a message. [RFC3315 15.1]
		 */
		ev->xid = random() & DH6_XIDMASK;
		log1("a new XID (%x) is generated",
		    ev->xid);
	}
	dh6->dh6_xid &= ~ntohl(DH6_XIDMASK);
	dh6->dh6_xid |= htonl(ev->xid);

	/*
	 * construct options
	 */
	dhcp6_init_options(&optinfo);

	/* server ID */
	switch (ev->state) {
	case DHCP6S_REQUEST:
	case DHCP6S_RENEW:
	case DHCP6S_RELEASE:
		dhcp6_vbuf_copy(&optinfo.serverID, &ev->serverid);
		break;
	}

	/* client ID */
	dhcp6_vbuf_copy(&optinfo.clientID, &G.client_duid);

	/* rapid commit (in Solicit only) */
	if (ev->state == DHCP6S_SOLICIT && (ifp->send_flags & DHCIFF_RAPID_COMMIT)) {
		optinfo.rapidcommit = 1;
	}

	/* elapsed time */
	if (ev->timeouts == 0) {
		ev->t_start = monotonic_ms();
		optinfo.elapsed_time = 0;
	} else {
		long long t_diff;

		t_diff = (monotonic_ms() - ev->t_start) / 10LL;
		/*
		 * The client uses the value 0xffff to represent any elapsed
		 * time values greater than the largest time value that can be
		 * represented in the Elapsed Time option (hundredths of a second).
		 * [RFC3315 22.9.]
		 */
		if (t_diff > MAX_ELAPSED_TIME) {
			/*
			 * Perhaps we are nervous too much, but without this
			 * additional check, we would see an overflow in 248
			 * days (of no responses).
			 */
			t_diff = MAX_ELAPSED_TIME;
		}
		optinfo.elapsed_time = (int32_t )t_diff;
	}

	/* option request options */
	if (ev->state != DHCP6S_RELEASE &&
	    dhcp6_copy_list(&optinfo.reqopt_list, &ifp->reqopt_list)) {
		bb_error_msg("can't copy requested options");
		goto end;
	}

	/* configuration information specified as event data */
	evd = TAILQ_FIRST(&ev->data_list);
	while (evd) {
		if (dhcp6_copy_list(&optinfo.ia_list, (struct dhcp6_list *)evd->data)) {
			bb_info_msg("can't add IA");
			goto end;
		}
		evd = TAILQ_NEXT(evd, link);
	}

#if ENABLE_FEATURE_DHCP6_AUTH
	/* authentication information */
	if (set_auth(ev, &optinfo)) {
		goto end;
	}
#endif

	/* set options in the message */
	optlen = dhcp6_set_options(dh6->dh6_msgtype,
	    (struct dhcp6opt *)(dh6 + 1),
	    (struct dhcp6opt *)(G.sbuf + sizeof(G.sbuf)), &optinfo);
	if (optlen < 0) {
		bb_error_msg("can't construct options");
		goto end;
	}
	len += optlen;

#if ENABLE_FEATURE_DHCP6_AUTH
	/* calculate MAC if necessary, and put it to the message */
	if (ev->authparam != NULL) {
		switch (ev->authparam->authproto) {
		case DHCP6_AUTHPROTO_DELAYED:
			if (ev->authparam->key == NULL)
				break;

			if (dhcp6_calc_mac((char *)dh6, len,
					optinfo.authproto, optinfo.authalgorithm,
					optinfo.delayedauth_offset + sizeof(*dh6),
					ev->authparam->key)) {
				bb_error_msg("can't calculate MAC");
				goto end;
			}
			break;
		default:
			break;	/* do nothing */
		}
	}
#endif

	/*
	 * Unless otherwise specified in this document or in a document that
	 * describes how IPv6 is carried over a specific type of link (for link
	 * types that do not support multicast), a client sends DHCP messages
	 * to the All_DHCP_Relay_Agents_and_Servers.
	 * [RFC3315 Section 13.]
	 */
	dst = G.sa6_allagent->u.sin6;
	dst.sin6_scope_id = ifp->ifid;

	if (sendto(csock, G.sbuf, len, 0, (struct sockaddr *)&dst,
			sizeof(dst)) == -1) {
		bb_perror_msg("sendto");
		goto end;
	}

#if defined CONFIG_UDHCP_DEBUG && CONFIG_UDHCP_DEBUG >= 1
	{
		char *a = xmalloc_sockaddr2dotted_noport((struct sockaddr *)&dst);
		log1("send %s to %s", dhcp6msgstr(dh6->dh6_msgtype), a);
		free(a);
	}
#endif

 end:
	dhcp6_clear_options(&optinfo);
}

static void client6_recv(void)
{
	struct sockaddr from;
	struct sockaddr_in6 to;
	struct dhcp6_if *ifp = &client6_config.dhcp6c_if;
	struct dhcp6_optinfo optinfo;
	ssize_t len;
	const struct dhcp6 *dh6 = (struct dhcp6 *)G.rbuf;
	struct dhcp6_event *ev;
	char *a;

	len = recv_from_to(csock, G.rbuf, sizeof(G.rbuf), 0,
			&from, (struct sockaddr *)&to, sizeof(struct sockaddr_in6));
	if (len < sizeof(struct dhcp6)) {
		if (len >= 0)
			bb_info_msg("short packet (%d bytes)", (int )len);
		return;
	}

	if (ifp->ifid != to.sin6_scope_id) {
		char ifname[IFNAMSIZ];

		bb_info_msg("unexpected interface (%s)",
				if_indextoname(to.sin6_scope_id, ifname));
		return;
	}

	a = xmalloc_sockaddr2dotted_noport(&from);
	log1("receive %s from %s on %s", dhcp6msgstr(dh6->dh6_msgtype),
		a, ifp->ifname);

	if (dh6->dh6_msgtype != DH6_ADVERTISE && dh6->dh6_msgtype != DH6_REPLY) {
		bb_info_msg("received an unexpected message (%s) from %s",
			dhcp6msgstr(dh6->dh6_msgtype), a);
		free(a);
		return;
	}

	/* get options */
	dhcp6_init_options(&optinfo);
	if (dhcp6_get_options((struct dhcp6opt *)(dh6 + 1),
	    (struct dhcp6opt *)((char *)dh6 + len), &optinfo) < 0) {
		bb_info_msg("can't parse options");
		goto fail;
	}

	/* find the corresponding event based on the received xid */
	ev = find_event_withid(ifp->ifid, ntohl(dh6->dh6_xid) & DH6_XIDMASK);
	if (ev == NULL) {
		bb_error_msg("XID mismatch");
		goto fail;
	}

	/* packet validation based on Section 15.3 of RFC3315. */
	if (optinfo.serverID.dv_len == 0) {
		bb_error_msg("no server ID");
		goto fail;
	}

	if (optinfo.clientID.dv_len == 0) {
		bb_error_msg("no client ID");
		goto fail;
	}
	/*
	 * DUID in the Client ID option (which must be contained for our
	 * client implementation) must match ours.
	 */
	if (dhcp6_vbuf_cmp(&optinfo.clientID, &G.client_duid)) {
		bb_error_msg("client DUID mismatch");
		goto fail;
	}

#if ENABLE_FEATURE_DHCP6_AUTH
	/* validate authentication */
	if (process_auth(ev->authparam, dh6, len, &optinfo)) {
		bb_error_msg("auth fail");
		goto fail;
	}
#endif

	switch (dh6->dh6_msgtype) {
	case DH6_ADVERTISE:
		client6_recvadvert(ifp, ev, &optinfo);
		break;
	case DH6_REPLY:
		client6_recvreply(ifp, ev, &optinfo);
		break;
	}

fail:
	dhcp6_clear_options(&optinfo);
	free(a);
	return;
}

static int client6_recvadvert(const struct dhcp6_if *ifp, struct dhcp6_event *ev,
				struct dhcp6_optinfo *optinfo)
{
	struct dhcp6_serverinfo *newserver, **sp;
	struct dhcp6_eventdata *evd;
	int have_ia = -1;

	/*
	 * The requesting router MUST ignore any Advertise message that
	 * includes a Status Code option containing the value NoPrefixAvail
	 * [RFC3633 Section 11.1].
	 * Likewise, the client MUST ignore any Advertise message that includes
	 * a Status Code option containing the value NoAddrsAvail.
	 * [RFC3315 Section 17.1.3].
	 * We only apply this when we are going to request an address or
	 * a prefix.
	 */
	TAILQ_FOREACH(evd, &ev->data_list, link) {
		struct dhcp6_listval *lv, *slv;
		dhcp6_listval_type_t lvtype;
		uint16_t stcode;

		switch (evd->type) {
		case D6_OPT_IA_PD:
			stcode = D6_OPT_STCODE_NOPREFIXAVAIL;
			lvtype = DHCP6_LISTVAL_PREFIX6;
			break;
		case D6_OPT_IA_NA:
			stcode = D6_OPT_STCODE_NOADDRSAVAIL;
			lvtype = DHCP6_LISTVAL_STATEFULADDR6;
			break;
		default:
			continue;
		}
		if (dhcp6_find_listval(&optinfo->stcode_list,
		    DHCP6_LISTVAL_NUM16, &stcode, 0)) {
			bb_error_msg("advertise contains %s status", dhcp6stcodestr(stcode));
			return -1;
		}

		if (have_ia > 0 || TAILQ_EMPTY((struct dhcp6_list *)evd->data))
			continue;
		have_ia = 0;
		TAILQ_FOREACH(lv, (struct dhcp6_list *)evd->data, link) {
			slv = dhcp6_find_listval(&optinfo->ia_list,
			    evd->type, &lv->val_ia, MATCHLIST_DH6OPTYPE);
			if (slv == NULL)
				continue;
			TAILQ_FOREACH(slv, &slv->sublist, link) {
				if (slv->lvtype == lvtype) {
					have_ia = 1;
					break;
				}
			}
		}
	}

	/* Ignore message with none of requested addresses and/or
	 * a prefixes as if NoAddrsAvail/NoPrefixAvail Status Code
	 * was included.
	 */
	if (have_ia == 0) {
		bb_info_msg("advertise contains no address/prefix");
		return -1;
	}

	if (ev->state != DHCP6S_SOLICIT ||
	    (ifp->send_flags & DHCIFF_RAPID_COMMIT) || (option_mask32 & OPT_i)) {
		/*
		 * We expected a reply message, but do actually receive an
		 * Advertise message.  The server should be configured not to
		 * allow the Rapid Commit option.
		 * We process the message as if we expected the Advertise.
		 * [RFC3315 Section 17.1.4]
		 */
		bb_info_msg("unexpected advertise");
		/* proceed anyway */
	}

	/* ignore the server if it is known */
	if (find_server(ev, &optinfo->serverID)) {
		bb_info_msg("duplicated server (ID: %s)", duidstr(&optinfo->serverID));
		return -1;
	}

	/* keep the server */
	newserver = xzalloc(sizeof(*newserver));

#if ENABLE_FEATURE_DHCP6_AUTH
	/* remember authentication parameters */
	newserver->authparam = ev->authparam;

	/* allocate new authentication parameter for the soliciting event */
	ev->authparam = new_authparam(ev->authparam->authproto,
		ev->authparam->authalgorithm, ev->authparam->authrdm);
#endif

	/* copy options */
	dhcp6_init_options(&newserver->optinfo);
	dhcp6_copy_options(&newserver->optinfo, optinfo);
	if (optinfo->pref != -1)
		newserver->pref = optinfo->pref;
	newserver->active = 1;
	for (sp = &ev->servers; *sp; sp = &(*sp)->next) {
		if ((*sp)->pref != D6_OPT_PREF_MAX && (*sp)->pref < newserver->pref) {
			break;
		}
	}
	newserver->next = *sp;
	*sp = newserver;

	if (newserver->pref == D6_OPT_PREF_MAX) {
		/*
		 * If the client receives an Advertise message that includes a
		 * Preference option with a preference value of 255, the client
		 * immediately begins a client-initiated message exchange.
		 * [RFC3315 Section 17.1.2]
		 */
		ev->current_server = newserver;
		dhcp6_vbuf_copy(&ev->serverid, &ev->current_server->optinfo.serverID);
		if (construct_reqdata(ifp, &ev->current_server->optinfo, ev)) {
			bb_info_msg("can't construct request");
			return -1; /* XXX */
		}

		ev->timeouts = 0;
		ev->state = DHCP6S_REQUEST;
#if ENABLE_FEATURE_DHCP6_AUTH
		free(ev->authparam);
		ev->authparam = newserver->authparam;
		newserver->authparam = NULL;
#endif
		client6_send(ev);

		dhcp6_set_timeoparam(ev);
		dhcp6_reset_timer(ev);
	} else if (ev->servers->next == NULL) {
		unsigned long long elapsed, timo;

		/*
		 * If this is the first advertise, adjust the timer so that
		 * the client can collect other servers until IRT elapses.
		 */
		elapsed = ev->retrans - dhcp6_timer_rest(ev->timer);
		if (elapsed <= ev->init_retrans)
			timo = ev->init_retrans - elapsed;
		else
			timo = 0;

		log1("reset timer for %s to %d.%03d",
		    ifp->ifname, (int )(timo/1000ULL), (int )(timo%1000ULL));

		dhcp6_timer_set(timo, ev->timer);
	}

	return 0;
}

static int client6_recvreply(struct dhcp6_if *ifp, struct dhcp6_event *ev,
				struct dhcp6_optinfo *optinfo)
{
	struct dhcp6_listval *lv;
	int state;

	state = ev->state;
	if (state != DHCP6S_INFOREQ &&
	    state != DHCP6S_REQUEST &&
	    state != DHCP6S_RENEW &&
	    state != DHCP6S_REBIND &&
	    state != DHCP6S_RELEASE &&
	    (state != DHCP6S_SOLICIT || !(ifp->send_flags & DHCIFF_RAPID_COMMIT))
	   ) {
		bb_info_msg("unexpected reply");
		return -1;
	}

	/*
	 * If the client included a Rapid Commit option in the Solicit message,
	 * the client discards any Reply messages it receives that do not
	 * include a Rapid Commit option.
	 * (should we keep the server otherwise?)
	 * [RFC3315 Section 17.1.4]
	 */
	if (state == DHCP6S_SOLICIT &&
	    (ifp->send_flags & DHCIFF_RAPID_COMMIT) && !optinfo->rapidcommit) {
		bb_info_msg("no rapid commit");
		return -1;
	}

	/*
	 * The client MAY choose to report any status code or message from the
	 * status code option in the Reply message.
	 * [RFC3315 Section 18.1.8]
	 */
	TAILQ_FOREACH(lv, &optinfo->stcode_list, link) {
		bb_info_msg("status code: %s", dhcp6stcodestr(lv->val_num16));
	}

	/*
	 * Set refresh timer for configuration information specified in
	 * information-request.  If the timer value is specified by the server
	 * in an information refresh time option, use it; use the protocol
	 * default otherwise.
	 */
	if (state == DHCP6S_INFOREQ) {
		unsigned long long refreshtime = DHCP6_IRT_DEFAULT;

		if (optinfo->refreshtime != -1)
			refreshtime = optinfo->refreshtime;

		ifp->timer = dhcp6_timer_add(client6_expire_refreshtime, ifp);

		if (refreshtime > 0x7fffffffULL) {
			/*
			 * XXX: tv_sec can overflow for an
			 * unsigned 32bit value.
			 */
			bb_info_msg("refresh time is too large: %llu", refreshtime);
			refreshtime = 0x7fffffffULL;
		}
		refreshtime *= 1000ULL;
		dhcp6_timer_set(refreshtime, ifp->timer);

	} else if (optinfo->refreshtime != -1) {
		/*
		 * draft-ietf-dhc-lifetime-02 clarifies that refresh time
		 * is only used for information-request and reply exchanges.
		 */
		bb_info_msg("unexpected information refresh time option (ignored)");
	}

	/* update stateful configuration information */
	if (state != DHCP6S_RELEASE) {
		update_ia(&optinfo->ia_list, ifp, &optinfo->serverID IF_FEATURE_DHCP6_AUTH(, ev->authparam));
	}

	/*
	 * Call the configuration script, if specified, to handle various
	 * configuration parameters.
	 */
	if (ifp->scriptpath != NULL) {
		log1("executes %s", ifp->scriptpath);
		dhcp6_script(ifp->scriptpath, fill_envp_client6(optinfo, ifp->ifname));
	}

	dhcp6_remove_event(ev);

	if (state == DHCP6S_RELEASE) {
		/*
		 * When the client receives a valid Reply message in response
		 * to a Release message, the client considers the Release event
		 * completed, regardless of the Status Code option(s) returned
		 * by the server.
		 * [RFC3315 Section 18.1.8]
		 */
		check_exit(0);
	}

	log1("got an expected reply, sleeping");

	if (option_mask32 & OPT_i) {
		check_exit(1);
	}
	return 0;
}

static struct dhcp6_event *find_event_withid(unsigned int ifid, uint32_t xid)
{
	struct dhcp6_event *ev;

	TAILQ_FOREACH(ev, &client6_config.event_list, link) {
		if (ev->xid == xid && ev->ifp->ifid == ifid)
			return ev;
	}

	return NULL;
}

#if ENABLE_FEATURE_DHCP6_AUTH
static int
process_auth(struct authparam *authparam, struct dhcp6 *dh6, ssize_t len,
	     struct dhcp6_optinfo *optinfo)
{
	struct keyinfo *key = NULL;
	int authenticated = 0;

	switch (optinfo->authproto) {
	case -1:
		/* server did not provide authentication option */
		break;
	case DHCP6_AUTHPROTO_DELAYED:
		if ((optinfo->authflags & DHCP6OPT_AUTHFLAG_NOINFO)) {
			bb_info_msg("server did not include authentication information");
			break;
		}

		if (optinfo->authalgorithm != DHCP6_AUTHALG_HMACMD5) {
			bb_info_msg("unknown authentication algorithm: %d",
				 optinfo->authalgorithm);
			break;
		}

		if (optinfo->authrdm != DHCP6_AUTHRDM_MONOCOUNTER) {
			bb_info_msg("unsupported replay detection method (%d)",
				optinfo->authrdm);
			break;
		}

		/*
		 * Replay protection.  If we do not know the previous RD value,
		 * we accept the message anyway (XXX).
		 */
		if ((authparam->flags & AUTHPARAM_FLAGS_NOPREVRD)) {
			bb_info_msg("previous RD value is unknown (accept it)");
		} else {
			if (dhcp6_auth_replaycheck(optinfo->authrdm,
			    authparam->prevrd, optinfo->authrd)) {
				bb_info_msg("possible replay attack detected");
				break;
			}
		}

		/* identify the secret key */
		key = authparam->key;
		if (key != NULL) {
			/*
			 * If we already know a key, its identification should
			 * match that contained in the received option.
			 * (from Section 21.4.5.1 of RFC3315)
			 */
			if (optinfo->delayedauth_keyid != key->keyid ||
				dhcp6_vbuf_cmp(&optinfo->authinfo.aiu_delayed.realm,
							   &key->realm) != 0) {
				bb_error_msg("authentication key mismatch");
				break;
			}
		} else {
			key = find_key(&optinfo->authinfo.aiu_delayed.realm,
			    optinfo->delayedauth_keyid);
			if (key == NULL) {
				bb_error_msg("can't find server key (ID: %x)",
				    optinfo->delayedauth_keyid);
				break;
			} else {
				log1("found key for authentication: %s", key->name);
			}
			authparam->key = key;
		}

		/* check for the key lifetime */
		if (dhcp6_validate_key(key)) {
			bb_error_msg("auth key %s has expired", key->name);
			break;
		}

		/* validate MAC */
		if (dhcp6_verify_mac((char *)dh6, len, optinfo->authproto,
		    optinfo->authalgorithm,
		    optinfo->delayedauth_offset + sizeof(*dh6), key) == 0) {
			log1("message authentication validated");
			authenticated = 1;
		} else {
			bb_error_msg("invalid message authentication");
		}

		break;
	default:
		bb_error_msg("unsupported authentication protocol: %d",
			 optinfo->authproto);
		break;
	}

	if (authenticated == 0) {
		if (authparam->authproto != -1) {
			bb_error_msg("message not authenticated "
			    "while authentication required");

			/*
			 * Right now, we simply discard unauthenticated
			 * messages.
			 */
			return -1;
		}
	} else {
		/* if authenticated, update the "previous" RD value */
		authparam->prevrd = optinfo->authrd;
		authparam->flags &= ~AUTHPARAM_FLAGS_NOPREVRD;
	}

	return 0;
}

static int set_auth(struct dhcp6_event *ev, struct dhcp6_optinfo *optinfo)
{
	struct authparam *authparam = ev->authparam;

	if (authparam == NULL)
		return 0;

	optinfo->authproto = authparam->authproto;
	optinfo->authalgorithm = authparam->authalgorithm;
	optinfo->authrdm = authparam->authrdm;

	switch (authparam->authproto) {
	case -1: /* we simply do not need authentication */
		return 0;
	case DHCP6_AUTHPROTO_DELAYED:
		if (ev->state == DHCP6S_INFOREQ) {
			/*
			 * In the current implementation, delayed
			 * authentication for Information-request and Reply
			 * exchanges doesn't work.  Specification is also
			 * unclear on this usage.
			 */
			bb_info_msg("delayed authentication "
			    "cannot be used for Information-request yet");
			return -1;
		}

		if (ev->state == DHCP6S_SOLICIT) {
			optinfo->authflags |= DHCP6OPT_AUTHFLAG_NOINFO;
			return 0; /* no auth information is needed */
		}

		if (authparam->key == NULL) {
			bb_error_msg("no authentication key");
			return -1;
		}

		if (dhcp6_validate_key(authparam->key)) {
			bb_error_msg("auth key %s is invalid", authparam->key->name);
			return -1;
		}

		if (get_rdvalue(optinfo->authrdm, &optinfo->authrd)) {
			return -1;
		}

		optinfo->delayedauth_keyid = authparam->key->keyid;
		dhcp6_vbuf_copy(&optinfo->authinfo.aiu_delayed.realm,
			&authparam->key->realm);
		break;
	default:
		bb_error_msg("unsupported authentication protocol: %d",
		    authparam->authproto);
		return -1;
	}

	return 0;
}
#endif /* ENABLE_FEATURE_DHCP6_AUTH*/
