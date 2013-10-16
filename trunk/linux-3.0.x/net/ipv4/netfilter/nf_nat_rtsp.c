/*
 * RTSP extension for TCP NAT alteration
 * (C) 2003 by Tom Marshall <tmarshall at real.com>
 *
 * 2013-03-04: Il'inykh Sergey <sergeyi at inango-sw.com>. Inango Systems Ltd
 *	- fixed rtcp nat mapping and other port mapping fixes
 *	- fixed system hard lock because of bug in the parser
 *	- codestyle fixes and less significant fixes
 *
 * based on ip_nat_irc.c
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 * Module load syntax:
 *	insmod nf_nat_rtsp.o ports=port1,port2,...port<MAX_PORTS>
 *	                     stunaddr=<address>
 *	                     destaction=[auto|strip|none]
 *
 * If no ports are specified, the default will be port 554 only.
 *
 * stunaddr specifies the address used to detect that a client is using STUN.
 * If this address is seen in the destination parameter, it is assumed that
 * the client has already punched a UDP hole in the firewall, so we don't
 * mangle the client_port.  If none is specified, it is autodetected.  It
 * only needs to be set if you have multiple levels of NAT.  It should be
 * set to the external address that the STUN clients detect.  Note that in
 * this case, it will not be possible for clients to use UDP with servers
 * between the NATs.
 *
 * If no destaction is specified, auto is used.
 *   destaction=auto:  strip destination parameter if it is not stunaddr.
 *   destaction=strip: always strip destination parameter (not recommended).
 *   destaction=none:  do not touch destination parameter (not recommended).
 */

#include <linux/module.h>
#include <linux/version.h>
#include <net/tcp.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
# include <net/netfilter/nf_nat.h>
#else
# include <net/netfilter/nf_nat_rule.h>
#endif
#include <net/netfilter/nf_nat_helper.h>
#include <linux/netfilter/nf_conntrack_rtsp.h>
#include <net/netfilter/nf_conntrack_expect.h>

#include <linux/inet.h>
#include <linux/ctype.h>
#define NF_NEED_STRNCASECMP
#define NF_NEED_STRTOU16
#include <linux/netfilter/netfilter_helpers.h>
#define NF_NEED_MIME_NEXTLINE
#include <linux/netfilter/netfilter_mime.h>

#define MAX_PORTS     8
#define DSTACT_AUTO   0
#define DSTACT_STRIP  1
#define DSTACT_NONE   2

static char* stunaddr = NULL;
static char* destaction = NULL;

static u_int32_t extip = 0;
static int       dstact = 0;

static void nf_nat_rtsp_expected(struct nf_conn* ct, struct nf_conntrack_expect *exp);

MODULE_AUTHOR("Tom Marshall <tmarshall at real.com>");
MODULE_DESCRIPTION("RTSP network address translation module");
MODULE_LICENSE("GPL");
module_param(stunaddr, charp, 0644);
MODULE_PARM_DESC(stunaddr, "Address for detecting STUN");
module_param(destaction, charp, 0644);
MODULE_PARM_DESC(destaction, "Action for destination parameter (auto/strip/none)");

#define SKIP_WSPACE(ptr,len,off) while(off < len && isspace(*(ptr+off))) { off++; }

/*** helper functions ***/

static void
get_skb_tcpdata(struct sk_buff* skb, char** pptcpdata, uint* ptcpdatalen)
{
	struct iphdr*   iph  = ip_hdr(skb);
	struct tcphdr*  tcph = (void *)iph + ip_hdrlen(skb);

	*pptcpdata = (char*)tcph +  tcph->doff*4;
	*ptcpdatalen = ((char*)skb_transport_header(skb) + skb->len) - *pptcpdata;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
/* copy of sip_sprintf_addr */
static int rtsp_sprintf_addr(const struct nf_conn *ct, char *buffer,
			     const union nf_inet_addr *addr, bool delim)
{
	if (nf_ct_l3num(ct) == NFPROTO_IPV4) {
		return sprintf(buffer, "%pI4", &addr->ip);
	} else {
		if (delim)
			return sprintf(buffer, "[%pI6c]", &addr->ip6);
		else
			return sprintf(buffer, "%pI6c", &addr->ip6);
	}
}
#endif

/*** nat functions ***/

/*
 * Mangle the "Transport:" header:
 *   - Replace all occurences of "client_port=<spec>"
 *   - Handle destination parameter
 *
 * In:
 *   ct, ctinfo = conntrack context
 *   skb        = packet
 *   tranoff    = Transport header offset from TCP data
 *   tranlen    = Transport header length (incl. CRLF)
 *   rport_lo   = replacement low  port (host endian)
 *   rport_hi   = replacement high port (host endian)
 *
 * Returns packet size difference.
 *
 * Assumes that a complete transport header is present, ending with CR or LF
 */
static int
rtsp_mangle_tran(enum ip_conntrack_info ctinfo,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
		 unsigned int protoff,
#endif
		 struct nf_conntrack_expect* rtp_exp,
		 struct nf_conntrack_expect* rtcp_exp,
		 struct ip_ct_rtsp_expect* prtspexp,
		 struct sk_buff* skb, uint tranoff, uint tranlen)
{
	char*  ptcp;
	uint   tcplen;
	char*  ptran;
	char   rbuf1[16];	  /* Replacement buffer (one port) */
	uint   rbuf1len;	  /* Replacement len (one port) */
	char   rbufa[16];	  /* Replacement buffer (all ports) */
	uint   rbufalen;	  /* Replacement len (all ports) */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
	union nf_inet_addr newip;
#else
	u_int32_t  newip;
#endif
	u_int16_t loport, hiport;
	uint      off = 0;
	uint      diff;		   /* Number of bytes we removed */

	struct nf_conn *ct = rtp_exp->master;
	/* struct nf_conn *ct = nf_ct_get(skb, &ctinfo); */
	struct nf_conntrack_tuple *rtp_t;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
	char szextaddr[INET6_ADDRSTRLEN];
#else
	char szextaddr[INET_ADDRSTRLEN];
#endif
	uint extaddrlen;
	int  is_stun;

	get_skb_tcpdata(skb, &ptcp, &tcplen);
	ptran = ptcp+tranoff;

	if (tranoff+tranlen > tcplen || tcplen-tranoff < tranlen ||
	    tranlen < 10 || !iseol(ptran[tranlen-1]) ||
	    nf_strncasecmp(ptran, "Transport:", 10) != 0) {
		pr_info("sanity check failed\n");
		return 0;
	}
	off += 10;
	SKIP_WSPACE(ptcp+tranoff, tranlen, off);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
	newip = ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3;
	rtp_t = &rtp_exp->tuple;
	rtp_t->dst.u3 = newip;
	if (rtcp_exp) {
		rtcp_exp->tuple.dst.u3 = newip;
	}
	extaddrlen = rtsp_sprintf_addr(ct, szextaddr, &newip, true); // FIXME handle extip
	pr_debug("stunaddr=%s (auto)\n", szextaddr);
#else
	newip = ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.ip;
	rtp_t = &rtp_exp->tuple;
	rtp_t->dst.u3.ip = newip;
	if (rtcp_exp) {
		rtcp_exp->tuple.dst.u3.ip = newip;
	}
	extaddrlen = extip ? sprintf(szextaddr, "%pI4", &extip)
			   : sprintf(szextaddr, "%pI4", &newip);
	pr_debug("stunaddr=%s (%s)\n", szextaddr, (extip?"forced":"auto"));
#endif
	hiport = 0;
	rbuf1len = rbufalen = 0;
	switch (prtspexp->pbtype) {
	case pb_single:
		for (loport = prtspexp->loport; loport != 0; loport++) { /* XXX: improper wrap? */
			rtp_t->dst.u.udp.port = htons(loport);
			if (nf_ct_expect_related(rtp_exp) == 0) {
				pr_debug("using port %hu\n", loport);
				break;
			}
		}
		if (loport != 0) {
			rbuf1len = sprintf(rbuf1, "%hu", loport);
			rbufalen = sprintf(rbufa, "%hu", loport);
		}
		break;
	case pb_range:
		for (loport = prtspexp->loport; loport != 0; loport += 2) { /* XXX: improper wrap? */
			rtp_t->dst.u.udp.port = htons(loport);
			if (nf_ct_expect_related(rtp_exp) != 0) {
				continue;
			}
			hiport = loport + 1;
			rtcp_exp->tuple.dst.u.udp.port = htons(hiport);
			if (nf_ct_expect_related(rtcp_exp) != 0) {
				nf_ct_unexpect_related(rtp_exp);
				continue;
			}

			/* FIXME: invalid print in case of ipv6 */
			pr_debug("nat expect_related %pI4:%u-%u-%pI4:%u-%u\n",
				 &rtp_exp->tuple.src.u3.ip,
				 ntohs(rtp_exp->tuple.src.u.udp.port),
				 ntohs(rtcp_exp->tuple.src.u.udp.port),
				 &rtp_exp->tuple.dst.u3.ip,
				 ntohs(rtp_exp->tuple.dst.u.udp.port),
				 ntohs(rtcp_exp->tuple.dst.u.udp.port));
			break;
		}
		if (loport != 0) {
			rbuf1len = sprintf(rbuf1, "%hu", loport);
			rbufalen = sprintf(rbufa, "%hu-%hu", loport, hiport);
		}
		break;
	case pb_discon:
		for (loport = prtspexp->loport; loport != 0; loport++) { /* XXX: improper wrap? */
			rtp_t->dst.u.udp.port = htons(loport);
			if (nf_ct_expect_related(rtp_exp) == 0) {
				pr_debug("using port %hu (1 of 2)\n", loport);
				break;
			}
		}
		for (hiport = prtspexp->hiport; hiport != 0; hiport++) { /* XXX: improper wrap? */
			rtp_t->dst.u.udp.port = htons(hiport);
			if (nf_ct_expect_related(rtp_exp) == 0) {
				pr_debug("using port %hu (2 of 2)\n", hiport);
				break;
			}
		}
		if (loport != 0 && hiport != 0) {
			rbuf1len = sprintf(rbuf1, "%hu", loport);
			rbufalen = sprintf(rbufa, hiport == loport+1 ?
					   "%hu-%hu":"%hu/%hu", loport, hiport);
		}
		break;
	}

	if (rbuf1len == 0)
		return 0;   /* cannot get replacement port(s) */

	/* Transport: tran;field;field=val,tran;field;field=val,...
	   `off` is set to the start of Transport value from start of line
	*/
	while (off < tranlen) {
		uint        saveoff;
		const char* pparamend;
		uint        nextparamoff;

		pparamend = memchr(ptran+off, ',', tranlen-off);
		pparamend = (pparamend == NULL) ? ptran+tranlen : pparamend+1;
		nextparamoff = pparamend-ptran;

		/*
		 * We pass over each param twice.  On the first pass, we look for a
		 * destination= field.  It is handled by the security policy.  If it
		 * is present, allowed, and equal to our external address, we assume
		 * that STUN is being used and we leave the client_port= field alone.
		 */
		is_stun = 0;
		saveoff = off;
		while (off < nextparamoff) {
			const char* pfieldend;
			uint        nextfieldoff;

			pfieldend = memchr(ptran+off, ';', nextparamoff-off);
			nextfieldoff = (pfieldend == NULL) ? nextparamoff : pfieldend-ptran+1;

			if (dstact != DSTACT_NONE && strncmp(ptran+off, "destination=", 12) == 0) {
				if (strncmp(ptran+off+12, szextaddr, extaddrlen) == 0)
					is_stun = 1;

				if (dstact == DSTACT_STRIP || (dstact == DSTACT_AUTO && !is_stun)) {
					uint dstoff = (ptran-ptcp)+off;
					uint dstlen = nextfieldoff-off;
					char* pdstrep = NULL;
					uint dstreplen = 0;
					diff = dstlen;
					if (dstact == DSTACT_AUTO && !is_stun) {
						pr_debug("RTSP: replace dst addr\n");
						dstoff += 12;
						dstlen -= 13;
						pdstrep = szextaddr;
						dstreplen = extaddrlen;
						diff = nextfieldoff-off-13-extaddrlen;
					}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
					if (!nf_nat_mangle_tcp_packet(skb, ct, ctinfo, protoff,
								      dstoff, dstlen, pdstrep, dstreplen)) {
#else
					if (!nf_nat_mangle_tcp_packet(skb, ct, ctinfo,
								      dstoff, dstlen, pdstrep, dstreplen)) {
#endif
						/* mangle failed, all we can do is bail */
						nf_ct_unexpect_related(rtp_exp);
						if (rtcp_exp)
							nf_ct_unexpect_related(rtcp_exp);
						return 0;
					}
					get_skb_tcpdata(skb, &ptcp, &tcplen);
					ptran = ptcp+tranoff;
					tranlen -= diff;
					nextparamoff -= diff;
					nextfieldoff -= diff;
				}
			}

			off = nextfieldoff;
		}

		if (is_stun)
			continue;

		off = saveoff;
		while (off < nextparamoff) {
			const char* pfieldend;
			uint        nextfieldoff;

			pfieldend = memchr(ptran+off, ';', nextparamoff-off);
			nextfieldoff = (pfieldend == NULL) ? nextparamoff : pfieldend-ptran+1;

			if (strncmp(ptran+off, "client_port=", 12) == 0) {
				u_int16_t port;
				uint	  numlen;
				uint      origoff;
				uint      origlen;
				char*     rbuf = rbuf1;
				uint      rbuflen = rbuf1len;

				off += 12;
				origoff = (ptran-ptcp)+off;
				origlen = 0;
				numlen = nf_strtou16(ptran+off, &port);
				off += numlen;
				origlen += numlen;
				if (port != prtspexp->loport) {
					pr_debug("multiple ports found, port %hu ignored\n", port);
				} else {
					if (ptran[off] == '-' || ptran[off] == '/') {
						off++;
						origlen++;
						numlen = nf_strtou16(ptran+off, &port);
						off += numlen;
						origlen += numlen;
						rbuf = rbufa;
						rbuflen = rbufalen;
					}

					/*
					 * note we cannot just memcpy() if the sizes are the same.
					 * the mangle function does skb resizing, checks for a
					 * cloned skb, and updates the checksums.
					 *
					 * parameter 4 below is offset from start of tcp data.
					 */
					diff = origlen-rbuflen;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
					if (!nf_nat_mangle_tcp_packet(skb, ct, ctinfo, protoff,
								      origoff, origlen, rbuf, rbuflen)) {
#else
					if (!nf_nat_mangle_tcp_packet(skb, ct, ctinfo,
								      origoff, origlen, rbuf, rbuflen)) {
#endif
						/* mangle failed, all we can do is bail */
						nf_ct_unexpect_related(rtp_exp);
						if (rtcp_exp)
							nf_ct_unexpect_related(rtcp_exp);
						return 0;
					}
					get_skb_tcpdata(skb, &ptcp, &tcplen);
					ptran = ptcp+tranoff;
					tranlen -= diff;
					nextparamoff -= diff;
					nextfieldoff -= diff;
				}
			}

			off = nextfieldoff;
		}

		off = nextparamoff;
	}

	return 1;
}

static uint
help_out(struct sk_buff *skb, enum ip_conntrack_info ctinfo,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
	 unsigned int protoff,
#endif
	 unsigned int matchoff, unsigned int matchlen,
	 struct ip_ct_rtsp_expect* prtspexp,
	 struct nf_conntrack_expect* rtp_exp,
	 struct nf_conntrack_expect* rtcp_exp)
{
	char* ptcp;
	uint  tcplen;
	uint  hdrsoff;
	uint  hdrslen;
	uint  lineoff;
	uint  linelen;
	uint  off;
	int   dir = CTINFO2DIR(ctinfo);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
	union nf_inet_addr saddr = rtp_exp->master->tuplehash[dir].tuple.src.u3;
#else
	__be32 saddr = rtp_exp->master->tuplehash[dir].tuple.src.u3.ip;
#endif

	//struct iphdr* iph = (struct iphdr*)(*pskb)->nh.iph;
	//struct tcphdr* tcph = (struct tcphdr*)((void*)iph + iph->ihl*4);

	get_skb_tcpdata(skb, &ptcp, &tcplen);
	hdrsoff = matchoff;//exp->seq - ntohl(tcph->seq);
	hdrslen = matchlen;
	off = hdrsoff;
	pr_debug("NAT rtsp help_out\n");

	while (nf_mime_nextline(ptcp, hdrsoff+hdrslen, &off, &lineoff, &linelen)) {
		if (linelen == 0)
			break;

		if (off > hdrsoff+hdrslen) {
			pr_info("!! overrun !!");
			break;
		}
		pr_debug("hdr: len=%u, %.*s", linelen, (int)linelen, ptcp+lineoff);

		if (nf_strncasecmp(ptcp+lineoff, "Transport:", 10) == 0) {
			uint oldtcplen = tcplen;
			pr_debug("hdr: Transport\n");
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
			if (!rtsp_mangle_tran(ctinfo, protoff, rtp_exp, rtcp_exp,
					      prtspexp, skb, lineoff, linelen)) {
#else
			if (!rtsp_mangle_tran(ctinfo, rtp_exp, rtcp_exp, prtspexp,
					      skb, lineoff, linelen)) {
#endif
				pr_debug("hdr: Transport mangle failed");
				break;
			}
			rtp_exp->expectfn = nf_nat_rtsp_expected;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
			rtp_exp->saved_addr = saddr;
#else
			rtp_exp->saved_ip = saddr;
#endif
			rtp_exp->saved_proto.udp.port = htons(prtspexp->loport);
			rtp_exp->dir = !dir;
			if (rtcp_exp) {
				rtcp_exp->expectfn = nf_nat_rtsp_expected;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
				rtcp_exp->saved_addr = saddr;
#else
				rtcp_exp->saved_ip = saddr;
#endif
				rtcp_exp->saved_proto.udp.port = htons(prtspexp->hiport);
				rtcp_exp->dir = !dir;
			}
			get_skb_tcpdata(skb, &ptcp, &tcplen);
			hdrslen -= (oldtcplen-tcplen);
			off -= (oldtcplen-tcplen);
			lineoff -= (oldtcplen-tcplen);
			linelen -= (oldtcplen-tcplen);
			pr_debug("rep: len=%u, %.*s", linelen, (int)linelen, ptcp+lineoff);
		}
	}

	return NF_ACCEPT;
}

static unsigned int
nf_nat_rtsp(struct sk_buff *skb, enum ip_conntrack_info ctinfo,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
	    unsigned int protoff,
#endif
	    unsigned int matchoff, unsigned int matchlen,
	    struct ip_ct_rtsp_expect* prtspexp,
	    struct nf_conntrack_expect* rtp_exp,
	    struct nf_conntrack_expect* rtcp_exp)
{
	int dir = CTINFO2DIR(ctinfo);
	int rc = NF_ACCEPT;

	switch (dir) {
	case IP_CT_DIR_ORIGINAL:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
		rc = help_out(skb, ctinfo, protoff, matchoff, matchlen, prtspexp,
			      rtp_exp, rtcp_exp);
#else
		rc = help_out(skb, ctinfo, matchoff, matchlen, prtspexp,
			      rtp_exp, rtcp_exp);
#endif
		break;
	case IP_CT_DIR_REPLY:
		pr_debug("unmangle ! %u\n", ctinfo);
		/* XXX: unmangle */
		rc = NF_ACCEPT;
		break;
	}
	//UNLOCK_BH(&ip_rtsp_lock);

	return rc;
}

static void nf_nat_rtsp_expected(struct nf_conn* ct, struct nf_conntrack_expect *exp)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0) || LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
	struct nf_nat_range range;
#else
	struct nf_nat_ipv4_range range;
#endif

	/* This must be a fresh one. */
	BUG_ON(ct->status & IPS_NAT_DONE_MASK);

	/* For DST manip, map port here to where it's expected. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
	range.min_proto = range.max_proto = exp->saved_proto;
	range.min_addr = range.max_addr = exp->saved_addr;
#else
	range.min = range.max = exp->saved_proto;
	range.min_ip = range.max_ip = exp->saved_ip;
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
	range.flags = (NF_NAT_RANGE_MAP_IPS | NF_NAT_RANGE_PROTO_SPECIFIED);
	nf_nat_setup_info(ct, &range, NF_NAT_MANIP_DST);
#else
	range.flags = (IP_NAT_RANGE_MAP_IPS | IP_NAT_RANGE_PROTO_SPECIFIED);
	nf_nat_setup_info(ct, &range, IP_NAT_MANIP_DST);
#endif

	/* Change src to where master sends to, but only if the connection
	 * actually came from the same source. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
	if (nf_inet_addr_cmp(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3,
			     &ct->master->tuplehash[exp->dir].tuple.src.u3)) {
		range.min_addr = range.max_addr
			= ct->master->tuplehash[!exp->dir].tuple.dst.u3;
#else
	if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip ==
	    ct->master->tuplehash[exp->dir].tuple.src.u3.ip) {
		range.min_ip = range.max_ip
			= ct->master->tuplehash[!exp->dir].tuple.dst.u3.ip;
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
		range.flags = NF_NAT_RANGE_MAP_IPS;
		nf_nat_setup_info(ct, &range, NF_NAT_MANIP_SRC);
#else
		range.flags = IP_NAT_RANGE_MAP_IPS;
		nf_nat_setup_info(ct, &range, IP_NAT_MANIP_SRC);
#endif
	}
}


static void __exit fini(void)
{
	rcu_assign_pointer(nf_nat_rtsp_hook, NULL);
	synchronize_net();
}

static int __init init(void)
{
	printk("nf_nat_rtsp v" IP_NF_RTSP_VERSION " loading\n");

	BUG_ON(nf_nat_rtsp_hook);
	rcu_assign_pointer(nf_nat_rtsp_hook, nf_nat_rtsp);

	if (stunaddr != NULL)
		extip = in_aton(stunaddr);

	if (destaction != NULL) {
		if (strcmp(destaction, "auto") == 0)
			dstact = DSTACT_AUTO;

		if (strcmp(destaction, "strip") == 0)
			dstact = DSTACT_STRIP;

		if (strcmp(destaction, "none") == 0)
			dstact = DSTACT_NONE;
	}

	return 0;
}

module_init(init);
module_exit(fini);
