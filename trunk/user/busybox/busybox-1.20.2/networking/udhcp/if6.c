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
#include <netinet/in.h>
#include <netinet/if_ether.h>

#include "common.h"
#include "dhcp6.h"
#include "config6.h"
#include "common6.h"

#ifndef SO_REUSEPORT
#define SO_REUSEPORT SO_REUSEADDR
#endif

#ifdef __linux__
/* from /usr/include/linux/ipv6.h */

struct in6_ifreq {
	struct in6_addr ifr6_addr;
	uint32_t ifr6_prefixlen;
	unsigned int ifr6_ifindex;
};
#endif /* __linux__ */


void if6init(struct dhcp6_if *ifp, const char *ifname)
{
	ifp->ifid = if_nametoindex(ifname);
	if (ifp->ifid == 0) {
		bb_perror_msg_and_die("invalid interface(%s)", ifp->ifname);
	}

	ifp->ifname = xstrdup(ifname);

	TAILQ_INIT(&ifp->reqopt_list);
	TAILQ_INIT(&ifp->iaconf_list);

	ifp->authproto = DHCP6_AUTHPROTO_UNDEF;
	ifp->authalgorithm = DHCP6_AUTHALG_UNDEF;
	ifp->authrdm = DHCP6_AUTHRDM_UNDEF;
}

int sock6_init(struct addrinfo *res)
{
	int sock;

	sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sock < 0) {
		bb_perror_msg_and_die("socket");
	}
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &const_int_1,
			sizeof(const_int_1)) < 0) {
		bb_perror_msg_and_die("setsockopt(SO_REUSEPORT)");
	}
	socket_want_pktinfo(sock);
#ifdef IPV6_V6ONLY
	if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &const_int_1,
			sizeof(const_int_1)) < 0) {
		bb_perror_msg_and_die("setsockopt(IPV6_V6ONLY)");
	}
#endif

	/*
	 * According RFC3315 2.2, only the incoming port should be bound to UDP
	 * port 546.  However, to have an interoperability with some servers,
	 * the outgoing port is also bound to the DH6PORT_DOWNSTREAM.
	 */
	xbind(sock, res->ai_addr, res->ai_addrlen);
	freeaddrinfo(res);

	return sock;
}

/* TODO! we can't use udp_io.c:recv_from_to() since it not return ipi6_ifindex */
ssize_t FAST_FUNC recv_from6(int fd, void *buf, size_t len,
				struct sockaddr_in6 *from, unsigned int *to_ifindex)
{
	struct msghdr mhdr;
	struct cmsghdr *cm;
	struct iovec iov[1];
	char cmsgbuf[CMSG_SPACE(sizeof(struct in6_pktinfo))];
	ssize_t recv_length;

	memset(&mhdr, 0, sizeof(mhdr));
	iov[0].iov_base = buf;
	iov[0].iov_len  = len;
	mhdr.msg_name       = from;
	mhdr.msg_namelen    = sizeof(*from);
	mhdr.msg_control    = cmsgbuf;
	mhdr.msg_controllen = sizeof(cmsgbuf);
	mhdr.msg_iov        = iov;
	mhdr.msg_iovlen     = 1;

	recv_length = recvmsg(fd, &mhdr, 0);
	if (recv_length < 0) {
		bb_perror_msg("recvmsg");
		return -EIO;
	}

	if (((struct sockaddr *)from)->sa_family != AF_INET6) {
		bb_info_msg("non-IPv6 packet is received (AF %d)",
				((struct sockaddr *)from)->sa_family);
		return -EPROTOTYPE;
	}

	/* detect receiving interface */
	*to_ifindex = 0;
	for (cm = (struct cmsghdr *)CMSG_FIRSTHDR(&mhdr); cm;
	     cm = (struct cmsghdr *)CMSG_NXTHDR(&mhdr, cm)) {
		if (cm->cmsg_level == IPPROTO_IPV6 &&
		    (cm->cmsg_type == IPV6_PKTINFO 
#if defined(IPV6_2292PKTINFO) && defined(IPV6_RECVPKTINFO)
			 || cm->cmsg_type == IPV6_2292PKTINFO
#endif
		    ) && cm->cmsg_len == CMSG_LEN(sizeof(struct in6_pktinfo))
		) {
			const int IPI6_IFINDEX_OFF = offsetof(struct in6_pktinfo, ipi6_ifindex);
			move_from_unaligned_int(*to_ifindex, (char*)(CMSG_DATA(cm)) + IPI6_IFINDEX_OFF);
			break;
		}
	}

	if (recv_length < sizeof(struct dhcp6)) {
		bb_info_msg("short packet (%d bytes)", (int )recv_length);
		return -ENODATA;
	}

	return recv_length;
}

/* Get hardware id(MAC) of first ethernet interface for DUID */
ssize_t gethwid(char *buf, uint16_t *hwtype)
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

int ifaddrconf(ifaddrconf_cmd_t cmd, const char *ifname, struct in6_addr *addr,
	   int plen, int pltime UNUSED_PARAM, int vltime UNUSED_PARAM)
{
/*
 * TODO: Should be rewritten to Netlink calls in order to support
 *       preferred/valid lifetimes
 */
	struct in6_ifreq req;
	struct ifreq ifr;
	const char *cmdstr[] = {
		[IFADDRCONF_ADD] "add",
		[IFADDRCONF_REMOVE] "remove"
	};
	unsigned long ioctl_cmd;
	struct sockaddr_in6 sin6;
	int sk;
	char *a;

	switch (cmd) {
	case IFADDRCONF_ADD:
		ioctl_cmd = SIOCSIFADDR;
		break;
	case IFADDRCONF_REMOVE:
		ioctl_cmd = SIOCDIFADDR;
		break;
	default:
		return -1;
	}

	sk = xsocket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	memset(&req, 0, sizeof(req));
    memset(&sin6, 0, sizeof(sin6));
    sin6.sin6_family = AF_INET6;
#ifdef HAVE_SA_LEN
    sin6.sin6_len = sizeof(struct sockaddr_in6);
#endif
    sin6.sin6_addr = *addr;

	memset(&ifr, 0, sizeof(ifr));
	strncpy_IFNAMSIZ(ifr.ifr_name, ifname);
	xioctl(sk, SIOGIFINDEX, &ifr);
	memcpy(&req.ifr6_addr, addr, sizeof(struct in6_addr));
	req.ifr6_prefixlen = plen;
	req.ifr6_ifindex = ifr.ifr_ifindex;

	if (ioctl(sk, ioctl_cmd, &req)) {
		if (errno == EEXIST) /* Ignore address duplicate */
			goto out;
		bb_perror_msg("can't %s address on %s", cmdstr[cmd], ifname);
		close(sk);
		return -1;
	}

	a = xmalloc_sockaddr2dotted_noport((struct sockaddr *)&sin6);
	bb_info_msg("%s address %s/%d on %s", cmdstr[cmd], a, plen, ifname);
	free(a);

 out:
	close(sk);
	return 0;
}
