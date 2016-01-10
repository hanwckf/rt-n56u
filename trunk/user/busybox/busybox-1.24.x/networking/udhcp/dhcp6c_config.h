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

PUSH_AND_SET_FUNCTION_VISIBILITY_TO_HIDDEN

typedef enum {
	DHCPOPT_IA_NA			= D6_OPT_IA_NA,
	DHCPOPT_AUTHINFO		= D6_OPT_AUTH,
	DHCPOPT_RAPID_COMMIT	= D6_OPT_RAPID_COMMIT,
	DHCPOPT_SIPNAME			= D6_OPT_SIP_SERVER_D,
	DHCPOPT_SIP				= D6_OPT_SIP_SERVER_A,
	DHCPOPT_DNS				= D6_OPT_DNS,
	DHCPOPT_DNSNAME			= D6_OPT_DNSNAME,
	DHCPOPT_IA_PD			= D6_OPT_IA_PD,
	DHCPOPT_NIS				= D6_OPT_NIS_SERVERS,
	DHCPOPT_NISP			= D6_OPT_NISP_SERVERS,
	DHCPOPT_NISNAME			= D6_OPT_NIS_DOMAIN,
	DHCPOPT_NISPNAME		= D6_OPT_NISP_DOMAIN,
	DHCPOPT_SNTP			= D6_OPT_SNTP_SERVERS,
	DHCPOPT_REFRESHTIME		= D6_OPT_REFRESHTIME,
	DHCPOPT_BCMCSNAME		= D6_OPT_BCMCS_SERVER_D,
	DHCPOPT_BCMCS			= D6_OPT_BCMCS_SERVER_A,

	DECL_IFACE = 100, DECL_AUTHINFO, DECL_KEYINFO,
	DECL_SEND, DECL_REQUEST, DECL_SCRIPT, DECL_INFO_ONLY,
/* server only start */
	DECL_ALLOW, DECL_DUID, DECL_PREFIX, DECL_PREFERENCE,
	DECL_DELAYEDKEY, DECL_ADDRESS, DECL_RANGE, DECL_ADDRESSPOOL,
/* server only  end  */
	IACONF_PIF, IACONF_PREFIX, IACONF_ADDR,
	IFPARAM_SLA_ID, IFPARAM_SLA_LEN,
	AUTHPARAM_PROTO, AUTHPARAM_ALG, AUTHPARAM_RDM, AUTHPARAM_KEY,
	KEYPARAM_REALM, KEYPARAM_KEYID, KEYPARAM_SECRET, KEYPARAM_EXPIRE
} dh6cnfopts_t;

/* definitions of tail-queue types */
TAILQ_HEAD(ia_conflist, ia_conf);
TAILQ_HEAD(pifc_list, prefix_ifconf);

struct dhcp6_poolspec {
	char *name;
	uint32_t pltime;
	uint32_t vltime;
};

struct dhcp6_range {
	struct in6_addr min;
	struct in6_addr max;
};

struct pool_conf {
	struct pool_conf *next;

	char *name;

	struct in6_addr min;
	struct in6_addr max;
};

/* per-interface information */
struct dhcp6_if {
/*	struct dhcp6_if *next;*/

	/* timer for the interface */
	struct dhcp6_timer *timer;

	/* static parameters of the interface */
	char *ifname;
	unsigned int ifid;
	/* multiple global address configuration is not supported now */
	struct in6_addr addr; 		/* global address (server only) */

	/* configuration parameters */
	uint32_t send_flags;
	uint32_t allow_flags;
#define DHCIFF_INFO_ONLY    0x1
#define DHCIFF_RAPID_COMMIT 0x2

	int server_pref;			/* server preference (server only) */
	struct dhcp6_poolspec pool;	/* address pool (server only) */
	char *scriptpath;			/* path to config script (client only) */

	struct dhcp6_list reqopt_list;
	struct ia_conflist iaconf_list;

#if ENABLE_FEATURE_DHCP6_AUTH
	/* authentication information */
	int authproto;				/* protocol */
	/* the followings are valid only if authproto is not UNDEF */
	int authalgorithm;			/* algorithm */
	int authrdm;				/* replay attack detection method */
#endif
};

/* run-time authentication parameters */
struct authparam {
	int authproto;
	int authalgorithm;
	int authrdm;
	struct keyinfo *key;
	int flags;
#define AUTHPARAM_FLAGS_NOPREVRD	0x1

	uint64_t prevrd;			/* previous RD value provided by the peer */
};

struct dhcp6_event {
	TAILQ_ENTRY(dhcp6_event) link;

	const struct dhcp6_if *ifp;
	struct dhcp6_timer *timer;

	struct dhcp6_vbuf serverid; /* server DUID */

	unsigned long long t_start; /* timestamp(ms) when the 1st msg is sent */

	/* internal timer parameters */
	unsigned long retrans;
	unsigned long init_retrans;
	long max_retrans_cnt;
	long max_retrans_time;
	long max_retrans_dur;
	int timeouts;				/* number of timeouts */

	uint32_t xid;				/* current transaction ID */
	int state;

	/* list of known servers */
	struct dhcp6_serverinfo *current_server;
	struct dhcp6_serverinfo *servers;

#if ENABLE_FEATURE_DHCP6_AUTH
	/* authentication parameters */
	struct authparam *authparam;
#endif
	TAILQ_HEAD(, dhcp6_eventdata) data_list;
};

struct dhcp6_eventdata {
	TAILQ_ENTRY(dhcp6_eventdata) link;

	struct dhcp6_event *event;
	int type;
	void *data;
};

struct dhcp6_serverinfo {
	struct dhcp6_serverinfo *next;

	/* option information provided in the advertisement */
	struct dhcp6_optinfo optinfo;

	int pref;					/* preference */
	int active;					/* bool; if this server is active or not */

#if ENABLE_FEATURE_DHCP6_AUTH
	struct authparam *authparam; /* authentication parameters */
#endif
	/* TODO: remember available information from the server */
};

struct prefix_ifconf {
	TAILQ_ENTRY(prefix_ifconf) link;

	char *ifname;		/* interface name such as ne0 */
	int sla_len;		/* SLA ID length in bits */
	uint32_t sla_id;	/* need more than 32bits? */
	int ifid_len;		/* interface ID length in bits */
	int ifid_type;		/* EUI-64 and manual (unused?) */
	char ifid[16];		/* Interface ID, up to 128bits */
};
#define IFID_LEN_DEFAULT 64
#define SLA_LEN_DEFAULT  16

struct ia_conf {
	TAILQ_ENTRY(ia_conf) link;

	int type;
	uint32_t iaid;

	TAILQ_HEAD(, ia) iadata; /* struct ia is an opaque type */

	/* type dependent values follow */
	struct dhcp6_list prefix_list;
	struct pifc_list iapd_pif_list;
};

struct cf_list;
struct authinfo;

struct client6_config_t {
	const char *conffile;

	struct dhcp6_if dhcp6c_if;

	struct ia_conflist ia_allconflist;
#if ENABLE_FEATURE_DHCP6_AUTH
	struct keyinfo *key_list;
	struct authinfo *auth_list;
#endif

	/* config parser internal tree */
	struct cf_list *config_list;

	/* timer */
	LIST_HEAD(, dhcp6_timer) timer_head;
	unsigned long long tm_sentinel;
	struct timeval tm_check;

	/* event queue */
	TAILQ_HEAD(, dhcp6_event) event_list;
} FIX_ALIASING;

#define client6_config (*(struct client6_config_t*)(&bb_common_bufsiz1))

void clear_ifconf(int destroy);

struct prefix_ifconf *find_prefixifconf(char *);
struct dhcp6_prefix *find_prefix6(struct dhcp6_list *, struct dhcp6_prefix *);
struct ia_conf *find_iaconf(const struct ia_conflist *, dh6cnfopts_t, uint32_t);
#if ENABLE_FEATURE_DHCP6_AUTH
struct keyinfo *find_key(struct dhcp6_vbuf *, uint32_t);
#endif

int read_config6(const char *file) FAST_FUNC;

POP_SAVED_FUNCTION_VISIBILITY
