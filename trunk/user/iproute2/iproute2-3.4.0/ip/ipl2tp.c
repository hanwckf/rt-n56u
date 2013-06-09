/*
 * ipl2tp.c	       "ip l2tp"
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Original Author:	James Chapman <jchapman@katalix.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/ip.h>

#include <linux/genetlink.h>
#include <linux/l2tp.h>

#include "utils.h"
#include "ip_common.h"

enum {
	L2TP_ADD,
	L2TP_CHG,
	L2TP_DEL,
	L2TP_GET
};

struct l2tp_parm {
	uint32_t tunnel_id;
	uint32_t peer_tunnel_id;
	uint32_t session_id;
	uint32_t peer_session_id;
	uint32_t offset;
	uint32_t peer_offset;
	enum l2tp_encap_type encap;
	uint16_t local_udp_port;
	uint16_t peer_udp_port;
	int cookie_len;
	uint8_t cookie[8];
	int peer_cookie_len;
	uint8_t peer_cookie[8];
	struct in_addr local_ip;
	struct in_addr peer_ip;

	uint16_t pw_type;
	uint16_t mtu;
	int udp_csum:1;
	int recv_seq:1;
	int send_seq:1;
	int lns_mode:1;
	int data_seq:2;
	int tunnel:1;
	int session:1;
	int reorder_timeout;
	const char *ifname;
};

struct l2tp_stats {
	uint64_t data_rx_packets;
	uint64_t data_rx_bytes;
	uint64_t data_rx_errors;
	uint64_t data_rx_oos_packets;
	uint64_t data_rx_oos_discards;
	uint64_t data_tx_packets;
	uint64_t data_tx_bytes;
	uint64_t data_tx_errors;
};

struct l2tp_data {
	struct l2tp_parm config;
	struct l2tp_stats stats;
};

/* netlink socket */
static struct rtnl_handle genl_rth;
static int genl_family = -1;

/*****************************************************************************
 * Netlink actions
 *****************************************************************************/

static int create_tunnel(struct l2tp_parm *p)
{
	struct {
		struct nlmsghdr 	n;
		struct genlmsghdr	g;
		char   			buf[1024];
	} req;

	memset(&req, 0, sizeof(req));
	req.n.nlmsg_type = genl_family;
	req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
	req.g.cmd = L2TP_CMD_TUNNEL_CREATE;
	req.g.version = L2TP_GENL_VERSION;

	addattr32(&req.n, 1024, L2TP_ATTR_CONN_ID, p->tunnel_id);
	addattr32(&req.n, 1024, L2TP_ATTR_PEER_CONN_ID, p->peer_tunnel_id);
	addattr8(&req.n, 1024, L2TP_ATTR_PROTO_VERSION, 3);
	addattr16(&req.n, 1024, L2TP_ATTR_ENCAP_TYPE, p->encap);

	addattr32(&req.n, 1024, L2TP_ATTR_IP_SADDR, p->local_ip.s_addr);
	addattr32(&req.n, 1024, L2TP_ATTR_IP_DADDR, p->peer_ip.s_addr);
	if (p->encap == L2TP_ENCAPTYPE_UDP) {
		addattr16(&req.n, 1024, L2TP_ATTR_UDP_SPORT, p->local_udp_port);
		addattr16(&req.n, 1024, L2TP_ATTR_UDP_DPORT, p->peer_udp_port);
	}

	if (rtnl_talk(&genl_rth, &req.n, 0, 0, NULL) < 0)
		return -2;

	return 0;
}

static int delete_tunnel(struct l2tp_parm *p)
{
	struct {
		struct nlmsghdr 	n;
		struct genlmsghdr	g;
		char   			buf[128];
	} req;

	memset(&req, 0, sizeof(req));
	req.n.nlmsg_type = genl_family;
	req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
	req.g.cmd = L2TP_CMD_TUNNEL_DELETE;
	req.g.version = L2TP_GENL_VERSION;

	addattr32(&req.n, 128, L2TP_ATTR_CONN_ID, p->tunnel_id);

	if (rtnl_talk(&genl_rth, &req.n, 0, 0, NULL) < 0)
		return -2;

	return 0;
}

static int create_session(struct l2tp_parm *p)
{
	struct {
		struct nlmsghdr 	n;
		struct genlmsghdr	g;
		char   			buf[1024];
	} req;

	memset(&req, 0, sizeof(req));
	req.n.nlmsg_type = genl_family;
	req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
	req.g.cmd = L2TP_CMD_SESSION_CREATE;
	req.g.version = L2TP_GENL_VERSION;

	addattr32(&req.n, 1024, L2TP_ATTR_CONN_ID, p->tunnel_id);
	addattr32(&req.n, 1024, L2TP_ATTR_PEER_CONN_ID, p->peer_tunnel_id);
	addattr32(&req.n, 1024, L2TP_ATTR_SESSION_ID, p->session_id);
	addattr32(&req.n, 1024, L2TP_ATTR_PEER_SESSION_ID, p->peer_session_id);
	addattr16(&req.n, 1024, L2TP_ATTR_PW_TYPE, p->pw_type);

	if (p->mtu)		addattr16(&req.n, 1024, L2TP_ATTR_MTU, p->mtu);
	if (p->recv_seq)	addattr(&req.n, 1024, L2TP_ATTR_RECV_SEQ);
	if (p->send_seq)	addattr(&req.n, 1024, L2TP_ATTR_SEND_SEQ);
	if (p->lns_mode)	addattr(&req.n, 1024, L2TP_ATTR_LNS_MODE);
	if (p->data_seq)	addattr8(&req.n, 1024, L2TP_ATTR_DATA_SEQ, p->data_seq);
	if (p->reorder_timeout) addattr64(&req.n, 1024, L2TP_ATTR_RECV_TIMEOUT,
					  p->reorder_timeout);
	if (p->offset)		addattr16(&req.n, 1024, L2TP_ATTR_OFFSET, p->offset);
	if (p->cookie_len)	addattr_l(&req.n, 1024, L2TP_ATTR_COOKIE,
					  p->cookie, p->cookie_len);
	if (p->peer_cookie_len) addattr_l(&req.n, 1024, L2TP_ATTR_PEER_COOKIE,
					  p->peer_cookie,  p->peer_cookie_len);
	if (p->ifname && p->ifname[0])
		addattrstrz(&req.n, 1024, L2TP_ATTR_IFNAME, p->ifname);

	if (rtnl_talk(&genl_rth, &req.n, 0, 0, NULL) < 0)
		return -2;

	return 0;
}

static int delete_session(struct l2tp_parm *p)
{
	struct {
		struct nlmsghdr 	n;
		struct genlmsghdr	g;
		char   			buf[128];
	} req;

	memset(&req, 0, sizeof(req));
	req.n.nlmsg_type = genl_family;
	req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
	req.g.cmd = L2TP_CMD_SESSION_DELETE;
	req.g.version = L2TP_GENL_VERSION;

	addattr32(&req.n, 1024, L2TP_ATTR_CONN_ID, p->tunnel_id);
	addattr32(&req.n, 1024, L2TP_ATTR_SESSION_ID, p->session_id);
	if (rtnl_talk(&genl_rth, &req.n, 0, 0, NULL) < 0)
		return -2;

	return 0;
}

static void print_cookie(char *name, const uint8_t *cookie, int len)
{
	printf("  %s %02x%02x%02x%02x", name,
	       cookie[0], cookie[1],
	       cookie[2], cookie[3]);
	if (len == 8)
		printf("%02x%02x%02x%02x",
		       cookie[4], cookie[5],
		       cookie[6], cookie[7]);
}

static void print_tunnel(const struct l2tp_data *data)
{
	const struct l2tp_parm *p = &data->config;

	printf("Tunnel %u, encap %s\n",
	       p->tunnel_id,
	       p->encap == L2TP_ENCAPTYPE_UDP ? "UDP" :
	       p->encap == L2TP_ENCAPTYPE_IP ? "IP" : "??");
	printf("  From %s ", inet_ntoa(p->local_ip));
	printf("to %s\n", inet_ntoa(p->peer_ip));
	printf("  Peer tunnel %u\n",
	       p->peer_tunnel_id);

	if (p->encap == L2TP_ENCAPTYPE_UDP)
		printf("  UDP source / dest ports: %hu/%hu\n",
		       p->local_udp_port, p->peer_udp_port);
}

static void print_session(struct l2tp_data *data)
{
	struct l2tp_parm *p = &data->config;

	printf("Session %u in tunnel %u\n",
	       p->session_id, p->tunnel_id);
	printf("  Peer session %u, tunnel %u\n",
	       p->peer_session_id, p->peer_tunnel_id);

	if (p->ifname != NULL) {
		printf("  interface name: %s\n", p->ifname);
	}
	printf("  offset %u, peer offset %u\n",
	       p->offset, p->peer_offset);
	if (p->cookie_len > 0)
		print_cookie("cookie", p->cookie, p->cookie_len);
	if (p->peer_cookie_len > 0)
		print_cookie("peer cookie", p->peer_cookie, p->peer_cookie_len);

	if (p->reorder_timeout != 0) {
		printf("  reorder timeout: %u\n", p->reorder_timeout);
	}
}

static int get_response(struct nlmsghdr *n, void *arg)
{
	struct genlmsghdr *ghdr;
	struct l2tp_data *data = arg;
	struct l2tp_parm *p = &data->config;
	struct rtattr *attrs[L2TP_ATTR_MAX + 1];
	struct rtattr *nla_stats;
	int len;

	/* Validate message and parse attributes */
	if (n->nlmsg_type == NLMSG_ERROR)
		return -EBADMSG;

	ghdr = NLMSG_DATA(n);
	len = n->nlmsg_len - NLMSG_LENGTH(sizeof(*ghdr));
	if (len < 0)
		return -1;

	parse_rtattr(attrs, L2TP_ATTR_MAX, (void *)ghdr + GENL_HDRLEN, len);

	if (attrs[L2TP_ATTR_PW_TYPE])
		p->pw_type = rta_getattr_u16(attrs[L2TP_ATTR_PW_TYPE]);
	if (attrs[L2TP_ATTR_ENCAP_TYPE])
		p->encap = rta_getattr_u16(attrs[L2TP_ATTR_ENCAP_TYPE]);
	if (attrs[L2TP_ATTR_OFFSET])
		p->offset = rta_getattr_u16(attrs[L2TP_ATTR_OFFSET]);
	if (attrs[L2TP_ATTR_DATA_SEQ])
		p->data_seq = rta_getattr_u16(attrs[L2TP_ATTR_DATA_SEQ]);
	if (attrs[L2TP_ATTR_CONN_ID])
		p->tunnel_id = rta_getattr_u32(attrs[L2TP_ATTR_CONN_ID]);
	if (attrs[L2TP_ATTR_PEER_CONN_ID])
		p->peer_tunnel_id = rta_getattr_u32(attrs[L2TP_ATTR_PEER_CONN_ID]);
	if (attrs[L2TP_ATTR_SESSION_ID])
		p->session_id = rta_getattr_u32(attrs[L2TP_ATTR_SESSION_ID]);
	if (attrs[L2TP_ATTR_PEER_SESSION_ID])
		p->peer_session_id = rta_getattr_u32(attrs[L2TP_ATTR_PEER_SESSION_ID]);

	p->udp_csum = !!attrs[L2TP_ATTR_UDP_CSUM];
	if (attrs[L2TP_ATTR_COOKIE])
		memcpy(p->cookie, RTA_DATA(attrs[L2TP_ATTR_COOKIE]),
		       p->cookie_len = RTA_PAYLOAD(attrs[L2TP_ATTR_COOKIE]));

	if (attrs[L2TP_ATTR_PEER_COOKIE])
		memcpy(p->peer_cookie, RTA_DATA(attrs[L2TP_ATTR_PEER_COOKIE]),
		       p->peer_cookie_len = RTA_PAYLOAD(attrs[L2TP_ATTR_PEER_COOKIE]));

	p->recv_seq = !!attrs[L2TP_ATTR_RECV_SEQ];
	p->send_seq = !!attrs[L2TP_ATTR_SEND_SEQ];

	if (attrs[L2TP_ATTR_RECV_TIMEOUT])
		p->reorder_timeout = rta_getattr_u64(attrs[L2TP_ATTR_RECV_TIMEOUT]);
	if (attrs[L2TP_ATTR_IP_SADDR])
		p->local_ip.s_addr = rta_getattr_u32(attrs[L2TP_ATTR_IP_SADDR]);
	if (attrs[L2TP_ATTR_IP_DADDR])
		p->peer_ip.s_addr = rta_getattr_u32(attrs[L2TP_ATTR_IP_DADDR]);
	if (attrs[L2TP_ATTR_UDP_SPORT])
		p->local_udp_port = rta_getattr_u16(attrs[L2TP_ATTR_UDP_SPORT]);
	if (attrs[L2TP_ATTR_UDP_DPORT])
		p->peer_udp_port = rta_getattr_u16(attrs[L2TP_ATTR_UDP_DPORT]);
	if (attrs[L2TP_ATTR_MTU])
		p->mtu = rta_getattr_u16(attrs[L2TP_ATTR_MTU]);
	if (attrs[L2TP_ATTR_IFNAME])
		p->ifname = rta_getattr_str(attrs[L2TP_ATTR_IFNAME]);

	nla_stats = attrs[L2TP_ATTR_STATS];
	if (nla_stats) {
		struct rtattr *tb[L2TP_ATTR_STATS_MAX + 1];

		parse_rtattr_nested(tb, L2TP_ATTR_STATS_MAX, nla_stats);

		if (tb[L2TP_ATTR_TX_PACKETS])
			data->stats.data_tx_packets = rta_getattr_u64(tb[L2TP_ATTR_TX_PACKETS]);
		if (tb[L2TP_ATTR_TX_BYTES])
			data->stats.data_tx_bytes = rta_getattr_u64(tb[L2TP_ATTR_TX_BYTES]);
		if (tb[L2TP_ATTR_TX_ERRORS])
			data->stats.data_tx_errors = rta_getattr_u64(tb[L2TP_ATTR_TX_ERRORS]);
		if (tb[L2TP_ATTR_RX_PACKETS])
			data->stats.data_rx_packets = rta_getattr_u64(tb[L2TP_ATTR_RX_PACKETS]);
		if (tb[L2TP_ATTR_RX_BYTES])
			data->stats.data_rx_bytes = rta_getattr_u64(tb[L2TP_ATTR_RX_BYTES]);
		if (tb[L2TP_ATTR_RX_ERRORS])
			data->stats.data_rx_errors = rta_getattr_u64(tb[L2TP_ATTR_RX_ERRORS]);
		if (tb[L2TP_ATTR_RX_SEQ_DISCARDS])
			data->stats.data_rx_oos_discards = rta_getattr_u64(tb[L2TP_ATTR_RX_SEQ_DISCARDS]);
		if (tb[L2TP_ATTR_RX_OOS_PACKETS])
			data->stats.data_rx_oos_packets = rta_getattr_u64(tb[L2TP_ATTR_RX_OOS_PACKETS]);
	}

	return 0;
}

static int session_nlmsg(const struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	int ret = get_response(n, arg);

	if (ret == 0)
		print_session(arg);

	return ret;
}

static int get_session(struct l2tp_data *p)
{
	struct {
		struct nlmsghdr 	n;
		struct genlmsghdr	g;
		char buf[128];
	} req;

	memset(&req, 0, sizeof(req));
	req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
	req.n.nlmsg_type = genl_family;
	req.n.nlmsg_flags = NLM_F_ROOT|NLM_F_MATCH|NLM_F_REQUEST;
	req.n.nlmsg_seq = genl_rth.dump = ++genl_rth.seq;

	req.g.cmd = L2TP_CMD_SESSION_GET;
	req.g.version = L2TP_GENL_VERSION;

	if (p->config.tunnel_id && p->config.session_id) {
		addattr32(&req.n, 128, L2TP_ATTR_CONN_ID, p->config.tunnel_id);
		addattr32(&req.n, 128, L2TP_ATTR_SESSION_ID, p->config.session_id);
	}

	if (rtnl_send(&genl_rth, &req, req.n.nlmsg_len) < 0)
		return -2;

	if (rtnl_dump_filter(&genl_rth, session_nlmsg, p) < 0) {
		fprintf(stderr, "Dump terminated\n");
		exit(1);
	}

	return 0;
}

static int tunnel_nlmsg(const struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	int ret = get_response(n, arg);

	if (ret == 0)
		print_tunnel(arg);

	return ret;
}

static int get_tunnel(struct l2tp_data *p)
{
	struct {
		struct nlmsghdr 	n;
		struct genlmsghdr	g;
		char buf[1024];
	} req;

	memset(&req, 0, sizeof(req));
	req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
	req.n.nlmsg_type = genl_family;
	req.n.nlmsg_flags = NLM_F_ROOT|NLM_F_MATCH|NLM_F_REQUEST;
	req.n.nlmsg_seq = genl_rth.dump = ++genl_rth.seq;

	req.g.cmd = L2TP_CMD_TUNNEL_GET;
	req.g.version = L2TP_GENL_VERSION;

	if (p->config.tunnel_id)
		addattr32(&req.n, 1024, L2TP_ATTR_CONN_ID, p->config.tunnel_id);

	if (rtnl_send(&genl_rth, &req, req.n.nlmsg_len) < 0)
		return -2;

	if (rtnl_dump_filter(&genl_rth, tunnel_nlmsg, p) < 0) {
		fprintf(stderr, "Dump terminated\n");
		exit(1);
	}

	return 0;
}

/*****************************************************************************
 * Command parser
 *****************************************************************************/

static int hex(char ch)
{
	if ((ch >= 'a') && (ch <= 'f'))
		return ch - 'a' + 10;
	if ((ch >= '0') && (ch <= '9'))
		return ch - '0';
	if ((ch >= 'A') && (ch <= 'F'))
		return ch - 'A' + 10;
	return -1;
}

static int hex2mem(const char *buf, uint8_t *mem, int count)
{
	int i, j;
	int c;

	for (i = 0, j = 0; i < count; i++, j += 2) {
		c = hex(buf[j]);
		if (c < 0)
			goto err;

		mem[i] = c << 4;

		c = hex(buf[j + 1]);
		if (c < 0)
			goto err;

		mem[i] |= c;
	}

	return 0;

err:
	return -1;
}

static void usage(void) __attribute__((noreturn));

static void usage(void)
{
	fprintf(stderr, "Usage: ip l2tp add tunnel\n");
	fprintf(stderr, "          remote ADDR local ADDR\n");
	fprintf(stderr, "          tunnel_id ID peer_tunnel_id ID\n");
	fprintf(stderr, "          [ encap { ip | udp } ]\n");
	fprintf(stderr, "          [ udp_sport PORT ] [ udp_dport PORT ]\n");
	fprintf(stderr, "Usage: ip l2tp add session [ name NAME ]\n");
	fprintf(stderr, "          tunnel_id ID\n");
	fprintf(stderr, "          session_id ID peer_session_id ID\n");
	fprintf(stderr, "          [ cookie HEXSTR ] [ peer_cookie HEXSTR ]\n");
	fprintf(stderr, "          [ offset OFFSET ] [ peer_offset OFFSET ]\n");
	fprintf(stderr, "       ip l2tp del tunnel tunnel_id ID\n");
	fprintf(stderr, "       ip l2tp del session tunnel_id ID session_id ID\n");
	fprintf(stderr, "       ip l2tp show tunnel [ tunnel_id ID ]\n");
	fprintf(stderr, "       ip l2tp show session [ tunnel_id ID ] [ session_id ID ]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Where: NAME   := STRING\n");
	fprintf(stderr, "       ADDR   := { IP_ADDRESS | any }\n");
	fprintf(stderr, "       PORT   := { 0..65535 }\n");
	fprintf(stderr, "       ID     := { 1..4294967295 }\n");
	fprintf(stderr, "       HEXSTR := { 8 or 16 hex digits (4 / 8 bytes) }\n");
	exit(-1);
}

static int parse_args(int argc, char **argv, int cmd, struct l2tp_parm *p)
{
	memset(p, 0, sizeof(*p));

	if (argc == 0)
		usage();

	while (argc > 0) {
		if (strcmp(*argv, "encap") == 0) {
			NEXT_ARG();
			if (strcmp(*argv, "ip") == 0) {
				p->encap = L2TP_ENCAPTYPE_IP;
			} else if (strcmp(*argv, "udp") == 0) {
				p->encap = L2TP_ENCAPTYPE_UDP;
			} else {
				fprintf(stderr, "Unknown tunnel encapsulation.\n");
				exit(-1);
			}
		} else if (strcmp(*argv, "name") == 0) {
			NEXT_ARG();
			p->ifname = *argv;
		} else if (strcmp(*argv, "remote") == 0) {
			NEXT_ARG();
			p->peer_ip.s_addr = get_addr32(*argv);
		} else if (strcmp(*argv, "local") == 0) {
			NEXT_ARG();
			p->local_ip.s_addr = get_addr32(*argv);
		} else if ((strcmp(*argv, "tunnel_id") == 0) ||
			   (strcmp(*argv, "tid") == 0)) {
			__u32 uval;
			NEXT_ARG();
			if (get_u32(&uval, *argv, 0))
				invarg("invalid ID\n", *argv);
			p->tunnel_id = uval;
		} else if ((strcmp(*argv, "peer_tunnel_id") == 0) ||
			   (strcmp(*argv, "ptid") == 0)) {
			__u32 uval;
			NEXT_ARG();
			if (get_u32(&uval, *argv, 0))
				invarg("invalid ID\n", *argv);
			p->peer_tunnel_id = uval;
		} else if ((strcmp(*argv, "session_id") == 0) ||
			   (strcmp(*argv, "sid") == 0)) {
			__u32 uval;
			NEXT_ARG();
			if (get_u32(&uval, *argv, 0))
				invarg("invalid ID\n", *argv);
			p->session_id = uval;
		} else if ((strcmp(*argv, "peer_session_id") == 0) ||
			   (strcmp(*argv, "psid") == 0)) {
			__u32 uval;
			NEXT_ARG();
			if (get_u32(&uval, *argv, 0))
				invarg("invalid ID\n", *argv);
			p->peer_session_id = uval;
		} else if (strcmp(*argv, "udp_sport") == 0) {
			__u16 uval;
			NEXT_ARG();
			if (get_u16(&uval, *argv, 0))
				invarg("invalid port\n", *argv);
			p->local_udp_port = uval;
		} else if (strcmp(*argv, "udp_dport") == 0) {
			__u16 uval;
			NEXT_ARG();
			if (get_u16(&uval, *argv, 0))
				invarg("invalid port\n", *argv);
			p->peer_udp_port = uval;
		} else if (strcmp(*argv, "offset") == 0) {
			__u8 uval;
			NEXT_ARG();
			if (get_u8(&uval, *argv, 0))
				invarg("invalid offset\n", *argv);
			p->offset = uval;
		} else if (strcmp(*argv, "peer_offset") == 0) {
			__u8 uval;
			NEXT_ARG();
			if (get_u8(&uval, *argv, 0))
				invarg("invalid offset\n", *argv);
			p->peer_offset = uval;
		} else if (strcmp(*argv, "cookie") == 0) {
			int slen;
			NEXT_ARG();
			slen = strlen(*argv);
			if ((slen != 8) && (slen != 16))
				invarg("cookie must be either 8 or 16 hex digits\n", *argv);

			p->cookie_len = slen / 2;
			if (hex2mem(*argv, p->cookie, p->cookie_len) < 0)
				invarg("cookie must be a hex string\n", *argv);
		} else if (strcmp(*argv, "peer_cookie") == 0) {
			int slen;
			NEXT_ARG();
			slen = strlen(*argv);
			if ((slen != 8) && (slen != 16))
				invarg("cookie must be either 8 or 16 hex digits\n", *argv);

			p->peer_cookie_len = slen / 2;
			if (hex2mem(*argv, p->peer_cookie, p->peer_cookie_len) < 0)
				invarg("cookie must be a hex string\n", *argv);
		} else if (strcmp(*argv, "tunnel") == 0) {
			p->tunnel = 1;
		} else if (strcmp(*argv, "session") == 0) {
			p->session = 1;
		} else if (matches(*argv, "help") == 0) {
			usage();
		} else {
			fprintf(stderr, "Unknown command: %s\n", *argv);
			usage();
		}

		argc--; argv++;
	}

	return 0;
}


static int do_add(int argc, char **argv)
{
	struct l2tp_parm p;
	int ret = 0;

	if (parse_args(argc, argv, L2TP_ADD, &p) < 0)
		return -1;

	if (!p.tunnel && !p.session)
		missarg("tunnel or session");

	if (p.tunnel_id == 0)
		missarg("tunnel_id");

	/* session_id and peer_session_id must be provided for sessions */
	if ((p.session) && (p.peer_session_id == 0))
		missarg("peer_session_id");
	if ((p.session) && (p.session_id == 0))
		missarg("session_id");

	/* peer_tunnel_id is needed for tunnels */
	if ((p.tunnel) && (p.peer_tunnel_id == 0))
		missarg("peer_tunnel_id");

	if (p.tunnel) {
		if (p.local_ip.s_addr == 0)
			missarg("local");

		if (p.peer_ip.s_addr == 0)
			missarg("remote");

		if (p.encap == L2TP_ENCAPTYPE_UDP) {
			if (p.local_udp_port == 0)
				missarg("udp_sport");
			if (p.peer_udp_port == 0)
				missarg("udp_dport");
		}

		ret = create_tunnel(&p);
	}

	if (p.session) {
		/* Only ethernet pseudowires supported */
		p.pw_type = L2TP_PWTYPE_ETH;

		ret = create_session(&p);
	}

	return ret;
}

static int do_del(int argc, char **argv)
{
	struct l2tp_parm p;

	if (parse_args(argc, argv, L2TP_DEL, &p) < 0)
		return -1;

	if (!p.tunnel && !p.session)
		missarg("tunnel or session");

	if ((p.tunnel) && (p.tunnel_id == 0))
		missarg("tunnel_id");
	if ((p.session) && (p.session_id == 0))
		missarg("session_id");

	if (p.session_id)
		return delete_session(&p);
	else
		return delete_tunnel(&p);

	return -1;
}

static int do_show(int argc, char **argv)
{
	struct l2tp_data data;
	struct l2tp_parm *p = &data.config;

	if (parse_args(argc, argv, L2TP_GET, p) < 0)
		return -1;

	if (!p->tunnel && !p->session)
		missarg("tunnel or session");

	if (p->session)
		get_session(&data);
	else
		get_tunnel(&data);

	return 0;
}

static int genl_parse_getfamily(struct nlmsghdr *nlh)
{
	struct rtattr *tb[CTRL_ATTR_MAX + 1];
	struct genlmsghdr *ghdr = NLMSG_DATA(nlh);
	int len = nlh->nlmsg_len;
	struct rtattr *attrs;

	if (nlh->nlmsg_type != GENL_ID_CTRL) {
		fprintf(stderr, "Not a controller message, nlmsg_len=%d "
			"nlmsg_type=0x%x\n", nlh->nlmsg_len, nlh->nlmsg_type);
		return -1;
	}

	if (ghdr->cmd != CTRL_CMD_NEWFAMILY) {
		fprintf(stderr, "Unknown controller command %d\n", ghdr->cmd);
		return -1;
	}

	len -= NLMSG_LENGTH(GENL_HDRLEN);

	if (len < 0) {
		fprintf(stderr, "wrong controller message len %d\n", len);
		return -1;
	}

	attrs = (struct rtattr *) ((char *) ghdr + GENL_HDRLEN);
	parse_rtattr(tb, CTRL_ATTR_MAX, attrs, len);

	if (tb[CTRL_ATTR_FAMILY_ID] == NULL) {
		fprintf(stderr, "Missing family id TLV\n");
		return -1;
	}

	return rta_getattr_u16(tb[CTRL_ATTR_FAMILY_ID]);
}

int genl_ctrl_resolve_family(const char *family)
{
	struct {
		struct nlmsghdr         n;
		struct genlmsghdr	g;
		char                    buf[1024];
	} req;

	memset(&req, 0, sizeof(req));
	req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_type = GENL_ID_CTRL;
	req.g.cmd = CTRL_CMD_GETFAMILY;

	addattr_l(&req.n, 1024, CTRL_ATTR_FAMILY_NAME,
		  family, strlen(family) + 1);

	if (rtnl_talk(&genl_rth, &req.n, 0, 0, &req.n) < 0) {
		fprintf(stderr, "Error talking to the kernel\n");
		return -2;
	}

	return genl_parse_getfamily(&req.n);
}

int do_ipl2tp(int argc, char **argv)
{
	if (genl_family < 0) {
		if (rtnl_open_byproto(&genl_rth, 0, NETLINK_GENERIC) < 0) {
			fprintf(stderr, "Cannot open generic netlink socket\n");
			exit(1);
		}

		genl_family = genl_ctrl_resolve_family(L2TP_GENL_NAME);
		if (genl_family < 0)
			exit(1);
	}

	if (argc < 1)
		usage();

	if (matches(*argv, "add") == 0)
		return do_add(argc-1, argv+1);
	if (matches(*argv, "del") == 0)
		return do_del(argc-1, argv+1);
	if (matches(*argv, "show") == 0 ||
	    matches(*argv, "lst") == 0 ||
	    matches(*argv, "list") == 0)
		return do_show(argc-1, argv+1);
	if (matches(*argv, "help") == 0)
		usage();

	fprintf(stderr, "Command \"%s\" is unknown, try \"ip l2tp help\".\n", *argv);
	exit(-1);
}
