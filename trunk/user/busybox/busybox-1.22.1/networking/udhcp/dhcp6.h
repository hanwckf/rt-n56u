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

#ifndef __DHCP6_H_DEFINED
#define __DHCP6_H_DEFINED

#include <sys/queue.h>

/* Message type */
#define DH6_SOLICIT		1
#define DH6_ADVERTISE	2
#define DH6_REQUEST		3
#define DH6_CONFIRM		4
#define DH6_RENEW		5
#define DH6_REBIND		6
#define DH6_REPLY		7
#define DH6_RELEASE		8
#define DH6_DECLINE		9
#define DH6_RECONFIGURE	10
#define DH6_INFORM_REQ	11
#define DH6_RELAY_FORW	12
#define DH6_RELAY_REPLY	13
#define DH6__MSG__MAX	DH6_RELAY_REPLY

/* Predefined addresses */
#define DH6ADDR_ALLAGENT	"ff02::1:2"
#define DH6ADDR_ALLSERVER	"ff05::1:3"
#define DH6PORT_DOWNSTREAM	546
#define DH6PORT_UPSTREAM	547

/* Protocol constants */

#define SOL_MAX_DELAY	1000	/* Solicit max delay, msec */

#define DHCP6_DURATION_INFINITE 0xffffffff
#define DHCP6_DURATION_MIN      30	/* 30 sec */

#define DHCP6_RELAY_MULTICAST_HOPS  32
#define DHCP6_RELAY_HOP_COUNT_LIMIT 32

#define DHCP6_IRT_DEFAULT	86400	/* 1 day */
#define DHCP6_IRT_MINIMUM	600

struct dhcp6_vbuf {		/* generic variable length buffer */
	int dv_len;
	caddr_t dv_buf;
};

/* option information */
struct dhcp6_ia {		/* identity association */
	uint32_t iaid;
	uint32_t t1;
	uint32_t t2;
};

struct dhcp6_prefix {		/* IA_PD */
	uint32_t pltime;
	uint32_t vltime;
	struct in6_addr addr;
	int plen;
};

struct dhcp6_statefuladdr {	/* IA_NA */
	uint32_t pltime;
	uint32_t vltime;
	struct in6_addr addr;
};

/* Internal data structure */
typedef enum {
	DHCP6_LISTVAL_NUM = 1, DHCP6_LISTVAL_NUM16, DHCP6_LISTVAL_NUM8,
	DHCP6_LISTVAL_IA,
	DHCP6_LISTVAL_ADDR6,
	DHCP6_LISTVAL_PREFIX6,
	DHCP6_LISTVAL_STATEFULADDR6,
	DHCP6_LISTVAL_VBUF
} dhcp6_listval_type_t;

TAILQ_HEAD(dhcp6_list, dhcp6_listval);
struct dhcp6_listval {
	TAILQ_ENTRY(dhcp6_listval) link;

	int dh6optype;					/* DHCPv6 option number (See D6_OPT_ ) */
	dhcp6_listval_type_t lvtype;

	union {
		int32_t  uv_num;
		uint16_t uv_num16;
		uint8_t  uv_num8;
		struct in6_addr uv_addr6;
		struct dhcp6_prefix uv_prefix6;
		struct dhcp6_statefuladdr uv_statefuladdr6;
		struct dhcp6_ia uv_ia;
		struct dhcp6_vbuf uv_vbuf;
	} uv;

	struct dhcp6_list sublist;
};
#define val_num           uv.uv_num
#define val_num16         uv.uv_num16
#define val_num8          uv.uv_num8
#define val_addr6         uv.uv_addr6
#define val_ia            uv.uv_ia
#define val_prefix6       uv.uv_prefix6
#define val_statefuladdr6 uv.uv_statefuladdr6
#define val_vbuf          uv.uv_vbuf

struct dhcp6_optinfo {
	struct dhcp6_vbuf clientID;		/* DUID */
	struct dhcp6_vbuf serverID;		/* DUID */

	int rapidcommit;				/* bool */
	int pref;						/* server preference */
	int32_t elapsed_time;			/* elapsed time (from client to server only) */
	int64_t refreshtime;			/* info refresh time for stateless options */

	struct dhcp6_list ia_list;		/* list of IA_PD/IA_NA */
	struct dhcp6_list reqopt_list;	/* options in option request */
	struct dhcp6_list stcode_list;	/* status code */
	struct dhcp6_list ad_list;		/* SIP/DNS/NTP/NIS/NIS+/BCMS servers/domain list */

	struct dhcp6_vbuf relay_msg;	/* relay message */
#define relaymsg_len relay_msg.dv_len
#define relaymsg_msg relay_msg.dv_buf

	struct dhcp6_vbuf ifidopt;		/* Interface-id */
#define ifidopt_len ifidopt.dv_len
#define ifidopt_id  ifidopt.dv_buf

	uint16_t authflags;
#define DHCP6OPT_AUTHFLAG_NOINFO	0x1
	int authproto;
	int authalgorithm;
	int authrdm;
	/* the followings are effective only when NOINFO is unset */
	uint64_t authrd;
	union {
		struct {
			uint32_t keyid;
			struct dhcp6_vbuf realm;
			int offset;			/* offset to the HMAC field */
		} aiu_delayed;
		struct {
			int type;
			int offset;			/* offset to the HMAC field */
			char val[16];		/* key value */
		} aiu_reconfig;
	} authinfo;
#define delayedauth_keyid    authinfo.aiu_delayed.keyid
#define delayedauth_realm    authinfo.aiu_delayed.realm
#define delayedauth_realmlen authinfo.aiu_delayed.realm.dv_len
#define delayedauth_realmval authinfo.aiu_delayed.realm.dv_buf
#define delayedauth_offset   authinfo.aiu_delayed.offset
#define reconfigauth_type    authinfo.aiu_reconfig.type
#define reconfigauth_offset  authinfo.aiu_reconfig.offset
#define reconfigauth_val     authinfo.aiu_reconfig.val
};

/* DHCP6 base packet format */
struct dhcp6 {
	union {
		uint8_t m;
		uint32_t x;
	} dh6_msgtypexid;
	/* options follow */
} __attribute__ ((__packed__));
#define dh6_msgtype	dh6_msgtypexid.m
#define dh6_xid		dh6_msgtypexid.x
#define DH6_XIDMASK	0x00ffffff

/* DHCPv6 relay messages */
struct dhcp6_relay {
	uint8_t dh6relay_msgtype;
	uint8_t dh6relay_hcnt;
	struct in6_addr dh6relay_linkaddr; /* XXX: badly aligned */
	struct in6_addr dh6relay_peeraddr; /* ditto */
	/* options follow */
} __attribute__ ((__packed__));

/* options */
#define D6_OPT_CLIENTID				1			/* RFC3315 */
#define D6_OPT_SERVERID				2			/* RFC3315 */
#define D6_OPT_IA_NA				3			/* RFC3315 */
#define D6_OPT_IA_TA				4			/* RFC3315 */
#define D6_OPT_IAADDR				5			/* RFC3315 */
#define D6_OPT_ORO					6			/* RFC3315 */
#define D6_OPT_PREFERENCE			7			/* RFC3315 */
#  define D6_OPT_PREF_MAX               255
#define D6_OPT_ELAPSED_TIME			8			/* RFC3315 */
#define D6_OPT_RELAY_MSG			9			/* RFC3315 */
/* #define D6_OPT_10				10: deprecated */
#define D6_OPT_AUTH					11			/* RFC3315 */
#  define D6_OPT_AUTH_PROTO_DELAYED     2
#  define D6_OPT_AUTH_RRECONFIGURE      3
#  define D6_OPT_AUTH_ALG_HMACMD5       1
#define D6_OPT_UNICAST				12			/* RFC3315 */
#define D6_OPT_STATUS_CODE			13			/* RFC3315 */
#  define D6_OPT_STCODE_SUCCESS         0
#  define D6_OPT_STCODE_UNSPECFAIL      1
#  define D6_OPT_STCODE_NOADDRSAVAIL    2
#  define D6_OPT_STCODE_NOBINDING       3
#  define D6_OPT_STCODE_NOTONLINK       4
#  define D6_OPT_STCODE_USEMULTICAST    5
#  define D6_OPT_STCODE_NOPREFIXAVAIL   6
#  define D6_OPT_STCODE__MAX            D6_OPT_STCODE_NOPREFIXAVAIL

#define D6_OPT_RAPID_COMMIT			14			/* RFC3315 */
#define D6_OPT_USER_CLASS			15			/* RFC3315 */
#define D6_OPT_VENDOR_CLASS			16			/* RFC3315 */
#define D6_OPT_VENDOR_OPTS			17			/* RFC3315 */
#define D6_OPT_INTERFACE_ID			18			/* RFC3315 */
#define D6_OPT_RECONF_MSG			19			/* RFC3315 */
#define D6_OPT_RECONF_ACCEPT		20			/* RFC3315 */

#define D6_OPT_SIP_SERVER_D			21			/* RFC3319 */
#define D6_OPT_SIP_SERVER_A			22			/* RFC3319 */
#define D6_OPT_DNS					23			/* RFC3646 */
#define D6_OPT_DNSNAME				24			/* RFC3646 */
#define D6_OPT_IA_PD				25			/* RFC3633 */
#define D6_OPT_IA_PD_PREFIX			26			/* RFC3633 */
#define D6_OPT_NIS_SERVERS			27			/* RFC3898 */
#define D6_OPT_NISP_SERVERS			28			/* RFC3898 */
#define D6_OPT_NIS_DOMAIN			29			/* RFC3898 */
#define D6_OPT_NISP_DOMAIN			30			/* RFC3898 */
#define D6_OPT_SNTP_SERVERS			31			/* RFC4075 */
#define D6_OPT_REFRESHTIME			32			/* RFC4242 */
#define D6_OPT_BCMCS_SERVER_D		33			/* RFC4280 */
#define D6_OPT_BCMCS_SERVER_A		34			/* RFC4280 */
/* #define D6_OPT_35				35: unassigned */
#define D6_OPT_GEOCONF_CIVIC		36			/* RFC4776 */
#define D6_OPT_REMOTE_ID			37			/* RFC4649 */
#define D6_OPT_SUBSCRIBER_ID		38			/* RFC4580 */
#define D6_OPT_CLIENT_FQDN			39			/* RFC4704 */
#define D6_OPT_PANA_AGENT			40			/* RFC5192 */
#define D6_OPT_NEW_POSIX_TIMEZONE	41			/* RFC4833 */
#define D6_OPT_NEW_TZDB_TIMEZONE	42			/* RFC4833 */
#define D6_OPT_ERO					43			/* RFC4994 */
#define D6_OPT_LQ_QUERY				44			/* RFC5007 */
#define D6_OPT_CLIENT_DATA			45			/* RFC5007 */
#define D6_OPT_CLT_TIME				46			/* RFC5007 */
#define D6_OPT_LQ_RELAY_DATA		47			/* RFC5007 */
#define D6_OPT_LQ_CLIENT_LINK		48			/* RFC5007 */
#define D6_OPT_MIP6_HNINF			49			/* RFC-ietf-mip6-hiopt-17 */
#define D6_OPT_MIP6_RELAY			50			/* RFC-ietf-mip6-hiopt-17 */
#define D6_OPT_V6_LOST				51			/* RFC5223 */
#define D6_OPT_CAPWAP_AC_V6			52			/* RFC5417 */
#define D6_OPT_RELAY_ID				53			/* RFC5460 */
#define D6_OPT_IPv6_Address_MoS		54			/* RFC5678 */
#define D6_OPT_IPv6_FQDN_MoS		55			/* RFC5678 */
#define D6_OPT_NTP_SERVER			56			/* RFC5908 */
#define D6_OPT_V6_ACCESS_DOMAIN		57			/* RFC5986 */
#define D6_OPT_SIP_UA_CS_LIST		58			/* RFC6011 */
#define D6_OPT_BOOTFILE_URL			59			/* RFC5970 */
#define D6_OPT_BOOTFILE_PARAM		60			/* RFC5970 */
#define D6_OPT_CLIENT_ARCH_TYPE		61			/* RFC5970 */
#define D6_OPT_NII					62			/* RFC5970 */
#define D6_OPT__MAX					D6_OPT_NII

/* The followings are KAME specific. */

/* DUID type 1 (DUID-LLT) */
struct dhcp6opt_duid_type1 {
	uint16_t type;
	uint16_t hwtype;
	uint32_t time;
	/* link-layer address follows */
} __attribute__ ((__packed__));

/* DUID type 2 (DUID-EN) */
struct dhcp6opt_duid_type2 {
	uint16_t type;
	uint32_t enterprise_number;
	/* identifier follows */
} __attribute__ ((__packed__));

/* DUID type 3 (DUID-LL) */
struct dhcp6opt_duid_type3 {
	uint16_t type;
	uint16_t hwtype;
	/* link-layer address follows */
} __attribute__ ((__packed__));

union dhcp6opt_duid_type {
	uint16_t type;
	struct dhcp6opt_duid_type1	d1;
	struct dhcp6opt_duid_type2	d2;
	struct dhcp6opt_duid_type3	d3;
};

struct dhcp6opt {
	uint16_t dh6opt_type;
	uint16_t dh6opt_len;
	/* type-dependent data follows */
} __attribute__ ((__packed__));

#define __D6_OPT_COMMON(opt_prefix) \
    struct dhcp6opt opt_prefix##opt

/* Status Code */
struct dhcp6opt_stcode {
	__D6_OPT_COMMON (stcode_);
	uint16_t dh6_stcode;
} __attribute__ ((__packed__));

/*
 * General format of Identity Association.
 * This format applies to Prefix Delegation (IA_PD) and Non-temporary Addresses
 * (IA_NA)
 */
struct dhcp6opt_ia {
	__D6_OPT_COMMON (ia_);
	uint32_t dh6_iaid;
	uint32_t dh6_t1;
	uint32_t dh6_t2;
	/* sub options follow */
} __attribute__ ((__packed__));

/* IA Addr */
struct dhcp6opt_ia_addr {
	__D6_OPT_COMMON (ia_addr_);
	struct in6_addr dh6_addr;
	uint32_t dh6_preferred_time;
	uint32_t dh6_valid_time;
} __attribute__ ((__packed__));

/* IA_PD Prefix */
struct dhcp6opt_ia_pd_prefix {
	__D6_OPT_COMMON (iapd_prefix_);
	uint32_t dh6_preferred_time;
	uint32_t dh6_valid_time;
	uint8_t dh6_prefix_len;
	struct in6_addr dh6_prefix_addr;
} __attribute__ ((__packed__));

/* Authentication */
struct dhcp6opt_auth {
	__D6_OPT_COMMON (auth_);
	uint8_t dh6_proto;
	uint8_t dh6_alg;
	uint8_t dh6_rdm;
	uint8_t dh6_rdinfo[8];
	/* authentication information follows */
} __attribute__ ((__packed__));
#undef __D6_OPT_COMMON


enum {
	DHCP6_AUTHPROTO_DELAYED  = 2,
	DHCP6_AUTHPROTO_RECONFIG = 3
};
enum {
	DHCP6_AUTHALG_HMACMD5 = 1
};
enum {
	DHCP6_AUTHRDM_MONOCOUNTER = 0
};

#endif /* __DHCP6_H_DEFINED */
