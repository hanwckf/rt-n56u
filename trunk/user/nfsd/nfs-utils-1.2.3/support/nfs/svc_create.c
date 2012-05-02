/*
 * Copyright 2009 Oracle.  All rights reserved.
 *
 * This file is part of nfs-utils.
 *
 * nfs-utils is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * nfs-utils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with nfs-utils.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>

#include <netinet/in.h>

#include <sys/socket.h>
#include <sys/resource.h>

#include <rpc/rpc.h>
#include <rpc/svc.h>

#ifdef HAVE_TCP_WRAPPER
#include "tcpwrapper.h"
#endif

#include "rpcmisc.h"
#include "xlog.h"

#ifdef HAVE_LIBTIRPC

/*
 * Set up an appropriate bind address, given @port and @nconf.
 *
 * Returns getaddrinfo(3) results if successful.  Caller must
 * invoke freeaddrinfo(3) on these results.
 *
 * Otherwise NULL is returned if an error occurs.
 */
__attribute_malloc__
static struct addrinfo *
svc_create_bindaddr(struct netconfig *nconf, const uint16_t port)
{
	struct addrinfo *ai = NULL;
	struct addrinfo hint = {
		.ai_flags	= AI_PASSIVE | AI_NUMERICSERV,
	};
	char buf[8];
	int error;

	if (strcmp(nconf->nc_protofmly, NC_INET) == 0)
		hint.ai_family = AF_INET;
#ifdef IPV6_SUPPORTED
	else if (strcmp(nconf->nc_protofmly, NC_INET6) == 0)
		hint.ai_family = AF_INET6;
#endif	/* IPV6_SUPPORTED */
	else {
		xlog(D_GENERAL, "Unrecognized bind address family: %s",
			nconf->nc_protofmly);
		return NULL;
	}

	if (strcmp(nconf->nc_proto, NC_UDP) == 0)
		hint.ai_protocol = (int)IPPROTO_UDP;
	else if (strcmp(nconf->nc_proto, NC_TCP) == 0)
		hint.ai_protocol = (int)IPPROTO_TCP;
	else {
		xlog(D_GENERAL, "Unrecognized bind address protocol: %s",
			nconf->nc_proto);
		return NULL;
	}

	(void)snprintf(buf, sizeof(buf), "%u", port);
	error = getaddrinfo(NULL, buf, &hint, &ai);
	if (error != 0) {
		xlog(L_ERROR, "Failed to construct bind address: %s",
			gai_strerror(error));
		return NULL;
	}

	return ai;
}

static unsigned int
svc_create_nconf(const char *name, const rpcprog_t program,
		const rpcvers_t version,
		void (*dispatch)(struct svc_req *, SVCXPRT *),
		const uint16_t port, struct netconfig *nconf)
{
	struct t_bind bindaddr;
	struct addrinfo *ai;
	SVCXPRT	*xprt;

	ai = svc_create_bindaddr(nconf, port);
	if (ai == NULL)
		return 0;

	bindaddr.addr.buf = ai->ai_addr;
	bindaddr.qlen = SOMAXCONN;

	xprt = svc_tli_create(RPC_ANYFD, nconf, &bindaddr, 0, 0);
	freeaddrinfo(ai);
	if (xprt == NULL) {
		xlog(D_GENERAL, "Failed to create listener xprt "
				"(%s, %u, %s)", name, version, nconf->nc_netid);
		return 0;
	}

	if (!svc_reg(xprt, program, version, dispatch, nconf)) {
		/* svc_reg(3) destroys @xprt in this case */
		xlog(D_GENERAL, "Failed to register (%s, %u, %s)",
				name, version, nconf->nc_netid);
		return 0;
	}

	return 1;
}

/**
 * nfs_svc_create - start up RPC svc listeners
 * @name: C string containing name of new service
 * @program: RPC program number to register
 * @version: RPC version number to register
 * @dispatch: address of function that handles incoming RPC requests
 * @port: if not zero, transport listens on this port
 *
 * Sets up network transports for receiving RPC requests, and starts
 * the RPC dispatcher.  Returns the number of started network transports.
 */
unsigned int
nfs_svc_create(__attribute__((unused)) char *name,
		const rpcprog_t program, const rpcvers_t version,
		void (*dispatch)(struct svc_req *, SVCXPRT *),
		const uint16_t port)
{
	const struct sigaction create_sigaction = {
		.sa_handler	= SIG_IGN,
	};
	unsigned int visible, up;
	struct netconfig *nconf;
	void *handlep;

	/*
	 * Ignore SIGPIPE to avoid exiting sideways when peers
	 * close their TCP connection while we're trying to reply
	 * to them.
	 */
	(void)sigaction(SIGPIPE, &create_sigaction, NULL);

	handlep = setnetconfig();
	if (handlep == NULL) {
		xlog(L_ERROR, "Failed to access local netconfig database: %s",
			nc_sperror());
		return 0;
	}

	visible = 0;
	up = 0;
	while ((nconf = getnetconfig(handlep)) != NULL) {
		if (!(nconf->nc_flag & NC_VISIBLE))
			continue;
		visible++;
		up += svc_create_nconf(name, program, version, dispatch,
						port, nconf);
	}

	if (visible == 0)
		xlog(L_ERROR, "Failed to find any visible netconfig entries");

	if (endnetconfig(handlep) == -1)
		xlog(L_ERROR, "Failed to close local netconfig database: %s",
			nc_sperror());

	return up;
}

/**
 * nfs_svc_unregister - remove service registrations from local rpcbind database
 * @program: RPC program number to unregister
 * @version: RPC version number to unregister
 *
 * Removes all registrations for [ @program, @version ] .
 */
void
nfs_svc_unregister(const rpcprog_t program, const rpcvers_t version)
{
	if (rpcb_unset(program, version, NULL) == FALSE)
		xlog(D_GENERAL, "Failed to unregister program %lu, version %lu",
			(unsigned long)program, (unsigned long)version);
}

#else	/* !HAVE_LIBTIRPC */

/**
 * nfs_svc_create - start up RPC svc listeners
 * @name: C string containing name of new service
 * @program: RPC program number to register
 * @version: RPC version number to register
 * @dispatch: address of function that handles incoming RPC requests
 * @port: if not zero, transport listens on this port
 *
 * Sets up network transports for receiving RPC requests, and starts
 * the RPC dispatcher.  Returns the number of started network transports.
 */
unsigned int
nfs_svc_create(char *name, const rpcprog_t program, const rpcvers_t version,
		void (*dispatch)(struct svc_req *, SVCXPRT *),
		const uint16_t port)
{
	rpc_init(name, (int)program, (int)version, dispatch, (int)port);
	return 1;
}

/**
 * nfs_svc_unregister - remove service registrations from local rpcbind database
 * @program: RPC program number to unregister
 * @version: RPC version number to unregister
 *
 * Removes all registrations for [ @program, @version ] .
 */
void
nfs_svc_unregister(const rpcprog_t program, const rpcvers_t version)
{
	if (pmap_unset((unsigned long)program, (unsigned long)version) == FALSE)
		xlog(D_GENERAL, "Failed to unregister program %lu, version %lu",
			(unsigned long)program, (unsigned long)version);
}

#endif	/* !HAVE_LIBTIRPC */
