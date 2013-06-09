/*
    Module Name:
    policy.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2007-01-23      Initial version
*/

#ifndef _POLICY_WANTED
#define _POLICY_WANTED

#include <linux/version.h>
#include <asm/rt2880/rt_mmap.h>
#include "frame_engine.h"

/*
 * DEFINITIONS AND MACROS
 */
#define POLICY_TBL_BASE    (RALINK_FRAME_ENGINE_BASE + 0x1000)

enum L2RuleDir {
	OTHERS = 0,
	DMAC = 1,
	SMAC = 2,
	SDMAC = 3
};

enum L3RuleDir {
	IP_QOS = 0,
	DIP = 1,
	SIP = 2,
	SDIP = 3
};

enum L4RuleDir {
	DONT_CARE = 0,
	DPORT = 1,
	SPORT = 2,
	SDPORT = 3
};

enum RuleType {
	L2_RULE = 0,
	L3_RULE = 1,
	L4_RULE = 2,
	PT_RULE = 3
};

enum PortNum {
	PN_CPU = 0,
	PN_GE1 = 1,
	PN_DONT_CARE = 7
};

enum OpCode {
	AND = 0,
	OR = 1
};

enum TcpFlag {
	TCP_FIN = 1,
	TCP_SYN = 2,
	TCP_RESET = 4,
	TCP_PUSH = 8,
	TCP_ACK = 16,
	TCP_URGENT = 32
};

enum TcpFlagOp {
	EQUAL = 0,
	NOT_EQUAL = 1,
	IN_SET = 2,
	NOT_IN_SET = 3
};

enum L4Type {
	FLT_IP_PROT = 0,
	FLT_UDP = 1,
	FLT_TCP = 2,
	FLT_TCP_UDP = 3
};

enum FpnType {
	FPN_CPU = 0,
	FPN_GE1 = 1,
	FPN_GE2 = 2,
	FPN_FRC_PRI_ONLY = 5,
	FPN_ALLOW = 4,
	FPN_DROP = 7
};

/*
 * TYPEDEFS AND STRUCTURES
 */
struct common_field {

	union {
		struct {
			uint8_t ee:1;	/* entry end */
			uint8_t resv:6;
			uint8_t logic:1;	/* logic operation with next rule (AND or OR) */
		} ee_0;		/* entry end =0 */

		struct {
			uint8_t ee:1;	/* entry end */
			uint8_t dop:1;	/* drop out profile */
			uint8_t mg:6;	/* meter group */
		} mtr;		/* meter */

		struct {
			uint8_t ee:1;	/* entry end */
			uint8_t rsv:1;
			uint8_t ag:6;	/* accounting group */
		} ac;		/* account */

		struct {
			uint8_t ee:1;	/* entry end */
			uint8_t fpp:1;	/* Force Destination Port */
			uint8_t fpn:3;	/* Force Port Number */
			uint8_t up:3;	/* User Perority */
		} fpp;		/* force destination port & force new user priority */

		struct {
			uint8_t ee:1;	/* entry end */
			uint8_t foe:1;	/* is Foe entry */
			uint8_t foe_tb:6;	/* FoE Table Entry */
		} foe;
	};

	uint8_t match:1;	/* 0: non-equal, 1: equal */
	uint8_t pn:3;		/* port number, Pre: ingress port/Post: egress port */
	uint8_t rt:2;		/* Rule type */
	uint8_t dir:2;		/* Direction */
};

struct l2_rule {

	struct common_field com;

	union {
		uint8_t mac[6];
		struct {
			uint16_t v:1;	/* compare VIDX */
			uint16_t s:1;	/* compare special tag */
			uint16_t e:1;	/* compare ethernet type */
			uint16_t p:1;	/* compare pppoe session id */
			uint16_t vid:12;	/* vlan id */
			uint16_t etyp_pprot;	/* ethernet type(p=0) or pppoe protocol(p=1) */
			uint16_t pppoe_id;
		} others;
	};
} __attribute__ ((packed));

struct l3_rule {

	struct common_field com;

	union {

		/* ip boundary = ip ~ ip + (ip_rng_m << ip_rng_e) */
		struct {
			uint32_t ip;
			uint16_t ip_rng_m:8;	/* ip range mantissa part */
			uint16_t ip_rng_e:5;	/* ip range exponent part */
			uint16_t v4:1;	/* ipv4/ipv6 */
			uint16_t ip_seg:2;	/* segment for ipv */
		} ip;

		struct {
			uint32_t tos_s:8;	/* start of ipv4 tos range */
			uint32_t tos_e:8;	/* end of ipv4 tos range */
			uint32_t resv:16;
			uint16_t resv1:4;
			uint16_t foz:1;	/* IPv4 fragment offset zero */
			uint16_t mf:1;	/* Ipv4 more fragments flag */
			uint16_t fov:1;	/* IPv4 fragment offset valid */
			uint16_t mfv:1;	/*IPv4 more fragments flag valid */
			uint16_t rsv:5;
			uint16_t v4:1;	/* this rule is for ipv4 or ipv6 */
			uint16_t rsv1:2;
		} qos;
	};
} __attribute__ ((packed));

struct l4_rule {

	struct common_field com;

	uint16_t p_start;	/* start of port range */
	uint16_t p_end;		/* end of port range */

	union {
		struct {
			uint16_t tcp_fop:2;	/* TCP flag operations */
			uint16_t tcp_f:6;	/* Expected value of TCP flags (U/A/P/R/S/F) */
			uint16_t tcp_fm:6;	/* Mask of TCP flags */
			uint16_t tu:2;	/* 11:tcp/udp, 10:tcp, 01:udp, 00:ip_proto */
		} tcp;

		struct {
			uint16_t resv:14;
			uint16_t tu:2;	/* 11:tcp/udp, 10:tcp, 01:udp, 00:ip_proto */
		} udp;

		struct {
			uint16_t prot:8;	/* ip protocol field */
			uint16_t resv:6;
			uint16_t tu:2;	/* 11:tcp/udp, 10:tcp, 01:udp, 00:ip_proto */
		} ip;
	};
} __attribute__ ((packed));

#endif
