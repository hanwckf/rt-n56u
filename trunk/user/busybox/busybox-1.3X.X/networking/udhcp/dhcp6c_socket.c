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

#include "dhcp6c.h"

#ifndef SO_REUSEPORT
#define SO_REUSEPORT SO_REUSEADDR
#endif

int sock6_init(const char *boundaddr, const char *port)
{
	struct addrinfo hints, *res;
	int err, sock;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;
	err = getaddrinfo(boundaddr, port, &hints, &res);
	if (err) {
		bb_error_msg_and_die("getaddrinfo: %s", gai_strerror(err));
	}
	sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sock < 0) {
		bb_perror_msg_and_die("socket");
	}
	if (setsockopt_SOL_SOCKET_1(sock, SO_REUSEPORT) < 0) {
		bb_perror_msg_and_die("setsockopt(SO_REUSEPORT)");
	}
	socket_want_pktinfo(sock);
#ifdef IPV6_V6ONLY
	if (setsockopt_1(sock, IPPROTO_IPV6, IPV6_V6ONLY) < 0) {
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


#ifdef __linux__
/* from /usr/include/linux/ipv6.h */

struct in6_ifreq {
	struct in6_addr ifr6_addr;
	uint32_t ifr6_prefixlen;
	unsigned int ifr6_ifindex;
};
#endif /* __linux__ */

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
