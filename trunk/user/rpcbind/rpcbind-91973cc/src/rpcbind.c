/*	$NetBSD: rpcbind.c,v 1.3 2002/11/08 00:16:40 fvdl Exp $	*/
/*	$FreeBSD: src/usr.sbin/rpcbind/rpcbind.c,v 1.12 2003/10/29 09:31:41 mbr Exp $ */

/*
 * Copyright (c) 2009, Sun Microsystems, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Sun Microsystems, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Copyright (c) 1984 - 1991 by Sun Microsystems, Inc.
 */



/*
 * rpcbind.c
 * Implements the program, version to address mapping for rpc.
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <rpc/rpc.h>
#include <rpc/rpc_com.h>
#ifdef PORTMAP
#include <netinet/in.h>
#endif
#include <arpa/inet.h>
#ifdef SYSTEMD
#include <systemd/sd-daemon.h>
#endif
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <netconfig.h>
#include <stdlib.h>
#include <unistd.h>
#include <grp.h>
#include <syslog.h>
#include <err.h>
#include <pwd.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_NSS_H
#include <nss.h>
#else
static inline void __nss_configure_lookup(const char *db, const char *s) {}
#endif
#include "rpcbind.h"
#include "xlog.h"

/*#define RPCBIND_DEBUG*/

/* Global variables */

int debugging = 0;	/* Tell me what's going on */
int doabort = 0;	/* When debugging, do an abort on errors */
int dofork = 1;		/* fork? */
int createdsocket = 0;  /* Did I create the socket or systemd did it for me? */

rpcblist_ptr list_rbl;	/* A list of version 3/4 rpcbind services */

#ifdef RPCBIND_USER
char *rpcbinduser = RPCBIND_USER;
#else
char *rpcbinduser = NULL;
#endif

#ifdef NSS_MODULES
char *nss_modules = NSS_MODULES;
#else
char *nss_modules = "files";
#endif

/* who to suid to if -s is given */
#define RUN_AS  "daemon"

#define RPCBINDDLOCK "/var/run/rpcbind.pid"

int runasdaemon = 0;
int insecure = 0;
int oldstyle_local = 0;
int verboselog = 0;

char **hosts = NULL;
int nhosts = 0;
int on = 1;
int rpcbindlockfd;

#ifdef LIB_SET_DEBUG
void  libtirpc_set_debug(char *name, int level, int use_stderr);
#endif
#ifdef WARMSTART
/* Local Variable */
static int warmstart = 0;	/* Grab an old copy of registrations. */
#endif

#ifdef PORTMAP
struct pmaplist *list_pml;	/* A list of version 2 rpcbind services */
char *udptrans;		/* Name of UDP transport */
char *tcptrans;		/* Name of TCP transport */
char *udp_uaddr;	/* Universal UDP address */
char *tcp_uaddr;	/* Universal TCP address */
#endif
static char servname[] = "rpcbind";
static char superuser[] = "superuser";

int main(int, char *[]);

static int init_transport(struct netconfig *);
static void rbllist_add(rpcprog_t, rpcvers_t, struct netconfig *,
			     struct netbuf *);
static void terminate(int);
static void parseargs(int, char *[]);

int
main(int argc, char *argv[])
{
	struct netconfig *nconf;
	void *nc_handle;	/* Net config handle */
	struct rlimit rl;
	int maxrec = RPC_MAXDATASIZE;

	parseargs(argc, argv);

	/* Check that another rpcbind isn't already running. */
	if ((rpcbindlockfd = (open(RPCBINDDLOCK,
	    O_RDONLY|O_CREAT, 0444))) == -1)
		err(1, "%s", RPCBINDDLOCK);

	if(flock(rpcbindlockfd, LOCK_EX|LOCK_NB) == -1 && errno == EWOULDBLOCK)
		errx(1, "another rpcbind is already running. Aborting");

	getrlimit(RLIMIT_NOFILE, &rl);
	if (rl.rlim_cur < 128) {
		if (rl.rlim_max <= 128)
			rl.rlim_cur = rl.rlim_max;
		else
			rl.rlim_cur = 128;
		setrlimit(RLIMIT_NOFILE, &rl);
	}

	if (geteuid()) { /* This command allowed only to root */
		fprintf(stderr, "Sorry. You are not superuser\n");
		exit(1);
	}

	/*
	 * Make sure we use the local service file 
	 * for service lookkups
	 */
	__nss_configure_lookup("services", nss_modules);

	nc_handle = setnetconfig(); 	/* open netconfig file */
	if (nc_handle == NULL) {
		syslog(LOG_ERR, "could not read /etc/netconfig");
		exit(1);
	}

	nconf = getnetconfigent("local");
	if (nconf == NULL)
		nconf = getnetconfigent("unix");
	if (nconf == NULL) {
		syslog(LOG_ERR, "%s: can't find local transport\n", argv[0]);
		exit(1);
	}
	xlog_open("rpcbind");
	if (dofork) {
		xlog_syslog(TRUE);
		xlog_stderr(FALSE);
	} else {
		xlog_syslog(FALSE);
		xlog_stderr(TRUE);
	}
	if (debugging) {
		xlog_config(D_ALL, 1);
#ifdef LIB_SET_DEBUG
		libtirpc_set_debug("rpcbind", debugging, (dofork == 0));
#endif
	}
	rpc_control(RPC_SVC_CONNMAXREC_SET, &maxrec);

	init_transport(nconf);

	while ((nconf = getnetconfig(nc_handle))) {
		if (nconf->nc_flag & NC_VISIBLE)
			init_transport(nconf);
	}
	endnetconfig(nc_handle);

#ifdef PORTMAP
	if (!udptrans)
		udptrans = "";
	if (!tcptrans)
		tcptrans = "";
#endif

	/* catch the usual termination signals for graceful exit */
	(void) signal(SIGCHLD, reap);
	(void) signal(SIGINT, terminate);
	(void) signal(SIGTERM, terminate);
	(void) signal(SIGQUIT, terminate);
	/* ignore others that could get sent */
	(void) signal(SIGPIPE, SIG_IGN);
	(void) signal(SIGHUP, SIG_IGN);
	(void) signal(SIGUSR1, SIG_IGN);
	(void) signal(SIGUSR2, SIG_IGN);

#ifdef RPCBIND_DEBUG 
	if (debugging) {
		xlog(LOG_DEBUG, "debugging enabled.");
		if (doabort)
			xlog(LOG_DEBUG, "Will abort on errors!\n");
	}
#endif
	if (dofork) {
		if (daemon(0, 0))
        	err(1, "fork failed");
	}

	if (runasdaemon || rpcbinduser) {
		struct passwd *p;
		char *id = runasdaemon ? RUN_AS : rpcbinduser;

		/*
		 * Make sure we use the local password file
		 * for these lookups.
		 */
		__nss_configure_lookup("passwd", nss_modules);

		if((p = getpwnam(id)) == NULL) {
			syslog(LOG_ERR, "cannot get uid of '%s': %m", id);
			exit(1);
		}
#ifdef WARMSTART
		if (warmstart) {
			mkdir_warmstart(p->pw_uid);
		}
#endif
                if (setgid(p->pw_gid) == -1) {
                        syslog(LOG_ERR, "setgid to '%s' (%d) failed: %m", id, p->pw_gid);
                        exit(1);
                }
		if (setgroups(0, NULL) == -1) {
			syslog(LOG_ERR, "dropping supplemental groups failed: %m");
			exit(1);
		}
		if (setuid(p->pw_uid) == -1) {
			syslog(LOG_ERR, "setuid to '%s' (%d) failed: %m", id, p->pw_uid);
			exit(1);
		}
	}

#ifdef WARMSTART
	if (warmstart) {
		read_warmstart();
	}
#endif

	network_init();

#ifdef SYSTEMD
	/* Try to notify system of successful startup, regardless of whether we
	 * used systemd socket activation or not. When started from the command
	 * line, this should not hurt either.
	 */
	sd_notify(0, "READY=1");
#endif
	my_svc_run();
	syslog(LOG_ERR, "svc_run returned unexpectedly");
	rpcbind_abort();
	/* NOTREACHED */

	return 0;
}

/*
 * Normally systemd will open sockets in dual ipv4/ipv6 mode.
 * That won't work with netconfig and we'll only match
 * the ipv6 socket. Convert it to IPV6_V6ONLY and issue
 * a warning for the user to fix their systemd config.
 */
static int
handle_ipv6_socket(int fd)
{
	int opt;
	socklen_t len = sizeof(opt);

	if (getsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &opt, &len)) {
		syslog(LOG_ERR, "failed to get ipv6 socket opts: %m");
		return -1;
	}

	if (opt) /* socket is already in V6ONLY mode */
		return 0;

	syslog(LOG_ERR, "systemd has passed an IPv4/IPv6 dual-mode socket.");
	syslog(LOG_ERR, "Please fix your systemd config by specifying IPv4 and IPv6 sockets separately and using BindIPv6Only=ipv6-only.");
	return -1;
}

/*
 * Adds the entry into the rpcbind database.
 * If PORTMAP, then for UDP and TCP, it adds the entries for version 2 also
 * Returns 0 if succeeds, else fails
 */
static int
init_transport(struct netconfig *nconf)
{
	int fd = -1;
	struct t_bind taddr;
	struct addrinfo hints, *res;
	struct __rpc_sockinfo si;
	SVCXPRT	*my_xprt = NULL;
	int status;	/* bound checking ? */
	int aicode;
	int addrlen = 0;
	int nhostsbak;
	int checkbind;
	int on = 1;
	struct sockaddr *sa = NULL;
	u_int32_t host_addr[4];  /* IPv4 or IPv6 */
	struct sockaddr_un sun;
	mode_t oldmask;
	int n;
        res = NULL;

	if ((nconf->nc_semantics != NC_TPI_CLTS) &&
		(nconf->nc_semantics != NC_TPI_COTS) &&
		(nconf->nc_semantics != NC_TPI_COTS_ORD))
		return (1);	/* not my type */
#ifdef RPCBIND_DEBUG
	if (debugging) {
		int i;
		char **s;

		(void) xlog(LOG_DEBUG, "%s: %ld lookup routines :\n",
			nconf->nc_netid, nconf->nc_nlookups);
		for (i = 0, s = nconf->nc_lookups; i < nconf->nc_nlookups;
		     i++, s++)
			xlog(LOG_DEBUG, "[%d] - %s\n", i, *s);
	}
#endif
	if (!__rpc_nconf2sockinfo(nconf, &si)) {
		syslog(LOG_ERR, "cannot get information for %s",
		    nconf->nc_netid);
		return (1);
	}

#ifdef SYSTEMD
	n = sd_listen_fds(0);
	if (n < 0) {
		syslog(LOG_ERR, "failed to acquire systemd sockets: %s", strerror(-n));
		return 1;
	}

	/* Try to find if one of the systemd sockets we were given match
	 * our netconfig structure. */

	for (fd = SD_LISTEN_FDS_START; fd < SD_LISTEN_FDS_START + n; fd++) {
		struct __rpc_sockinfo si_other;
		union {
			struct sockaddr sa;
			struct sockaddr_un un;
			struct sockaddr_in in4;
			struct sockaddr_in6 in6;
			struct sockaddr_storage storage;
		} sa;
		socklen_t addrlen = sizeof(sa);

		if (!__rpc_fd2sockinfo(fd, &si_other)) {
			syslog(LOG_ERR, "cannot get information for fd %i", fd);
			return 1;
		}

		if (si.si_af != si_other.si_af ||
                    si.si_socktype != si_other.si_socktype ||
                    si.si_proto != si_other.si_proto)
			continue;

		if (getsockname(fd, &sa.sa, &addrlen) < 0) {
			syslog(LOG_ERR, "failed to query socket name: %s",
                               strerror(errno));
			goto error;
		}

		if (sa.sa.sa_family == AF_INET6 && handle_ipv6_socket(fd))
		        goto error;

		/* Copy the address */
		taddr.addr.maxlen = taddr.addr.len = addrlen;
		taddr.addr.buf = malloc(addrlen);
		if (taddr.addr.buf == NULL) {
			syslog(LOG_ERR,
                               "cannot allocate memory for %s address",
                               nconf->nc_netid);
			goto error;
		}
		memcpy(taddr.addr.buf, &sa, addrlen);

		my_xprt = (SVCXPRT *)svc_tli_create(fd, nconf, &taddr,
                          RPC_MAXDATASIZE, RPC_MAXDATASIZE);
		if (my_xprt == (SVCXPRT *)NULL) {
			syslog(LOG_ERR, "%s: could not create service",
                               nconf->nc_netid);
			goto error;
		}
	}

	/*
	 * If none of the systemd sockets matched, we set up the socket in
	 * the normal way:
	 */
#endif
	if (my_xprt != NULL)
		goto got_socket;

	if ((strcmp(nconf->nc_netid, "local") == 0) ||
	    (strcmp(nconf->nc_netid, "unix") == 0)) {
		memset(&sun, 0, sizeof sun);
		sun.sun_family = AF_LOCAL;
		unlink(_PATH_RPCBINDSOCK);
		createdsocket = 1; /* We are now in the process of creating the unix socket */
		strcpy(sun.sun_path, _PATH_RPCBINDSOCK);
		addrlen = SUN_LEN(&sun);
		sa = (struct sockaddr *)&sun;
	} else {
		/* Get rpcbind's address on this transport */

		memset(&hints, 0, sizeof hints);
		hints.ai_flags = AI_PASSIVE;
		hints.ai_family = si.si_af;
		hints.ai_socktype = si.si_socktype;
		hints.ai_protocol = si.si_proto;
	}
	if (nconf->nc_semantics == NC_TPI_CLTS) {
		/*
		 * If no hosts were specified, just bind to INADDR_ANY.  Otherwise
		 * make sure 127.0.0.1 is added to the list.
		 */
		nhostsbak = nhosts;
		nhostsbak++;
		hosts = realloc(hosts, nhostsbak * sizeof(char *));
		if (nhostsbak == 1)
			hosts[0] = "*";
		else {
			if (hints.ai_family == AF_INET) {
				hosts[nhostsbak - 1] = "127.0.0.1";
			} else if (hints.ai_family == AF_INET6) {
				hosts[nhostsbak - 1] = "::1";
			} else
				return 1;
		}

	       /*
		* Bind to specific IPs if asked to
		*/
		checkbind = 0;
		while (nhostsbak > 0) {
			--nhostsbak;
			/*
			 * XXX - using RPC library internal functions.
			 */
			if ((fd = __rpc_nconf2fd(nconf)) < 0) {
				syslog(LOG_ERR, "cannot create socket for %s",
				    nconf->nc_netid);
				return (1);
			}

			hints.ai_flags &= ~AI_NUMERICHOST;
			switch (hints.ai_family) {
			case AF_INET:
				if (inet_pton(AF_INET, hosts[nhostsbak],
				    host_addr) == 1) {
					hints.ai_flags |= AI_NUMERICHOST;
				} else {
					/*
					 * Skip if we have an AF_INET6 adress.
					 */
					if (inet_pton(AF_INET6,
					    hosts[nhostsbak], host_addr) == 1)
						continue;
				}
				break;
			case AF_INET6:
				if (inet_pton(AF_INET6, hosts[nhostsbak],
				    host_addr) == 1) {
					hints.ai_flags |= AI_NUMERICHOST;
				} else {
					/*
					 * Skip if we have an AF_INET adress.
					 */
					if (inet_pton(AF_INET, hosts[nhostsbak],
					    host_addr) == 1)
						continue;
				}
	        		break;
			default:
				break;
			}

			/*
			 * If no hosts were specified, just bind to INADDR_ANY
			 */
			if (strcmp("*", hosts[nhostsbak]) == 0)
				hosts[nhostsbak] = NULL;

			if ((aicode = getaddrinfo(hosts[nhostsbak],
			    servname, &hints, &res)) != 0) {
			  if ((aicode = getaddrinfo(hosts[nhostsbak],
						    "portmapper", &hints, &res)) != 0) {
				syslog(LOG_ERR,
				    "cannot get local address for %s: %s",
				    nconf->nc_netid, gai_strerror(aicode));
				continue;
			  }
			}
			addrlen = res->ai_addrlen;
			sa = (struct sockaddr *)res->ai_addr;
			oldmask = umask(S_IXUSR|S_IXGRP|S_IXOTH);
                        if (bind(fd, sa, addrlen) != 0) {
				syslog(LOG_ERR, "cannot bind %s on %s: %m",
					(hosts[nhostsbak] == NULL) ? "*" :
					hosts[nhostsbak], nconf->nc_netid);
				if (res != NULL)
					freeaddrinfo(res);
				continue;
			} else
				checkbind++;
			(void) umask(oldmask);

			/* Copy the address */
			taddr.addr.maxlen = taddr.addr.len = addrlen;
			taddr.addr.buf = malloc(addrlen);
			if (taddr.addr.buf == NULL) {
				syslog(LOG_ERR,
				    "cannot allocate memory for %s address",
				    nconf->nc_netid);
				if (res != NULL)
					freeaddrinfo(res);
				return 1;
			}
			memcpy(taddr.addr.buf, sa, addrlen);
#ifdef RPCBIND_DEBUG
			if (debugging) {
				/*
				 * for debugging print out our universal
				 * address
				 */
				char *uaddr;
				struct netbuf nb;
				int sa_size = 0;

				nb.buf = sa;
				switch( sa->sa_family){
				case AF_INET:
				  sa_size = sizeof (struct sockaddr_in);
				  break;
				case AF_INET6:
				  sa_size = sizeof (struct sockaddr_in6);				 
				  break;
				}
				nb.len = nb.maxlen = sa_size;
				uaddr = taddr2uaddr(nconf, &nb);
				(void) xlog(LOG_DEBUG,
				    "rpcbind : my address is %s\n", uaddr);
				(void) free(uaddr);
			}
#endif
			my_xprt = (SVCXPRT *)svc_tli_create(fd, nconf, &taddr, 
                                RPC_MAXDATASIZE, RPC_MAXDATASIZE);
			if (my_xprt == (SVCXPRT *)NULL) {
				syslog(LOG_ERR, "%s: could not create service", 
                                        nconf->nc_netid);
				goto error;
			}
		}
		if (!checkbind)
			return 1;
	} else {	/* NC_TPI_COTS */
		if ((fd = __rpc_nconf2fd(nconf)) < 0) {
			syslog(LOG_ERR, "cannot create socket for %s",
			    nconf->nc_netid);
			return (1);
		}

		if ((strcmp(nconf->nc_netid, "local") != 0) &&
		    (strcmp(nconf->nc_netid, "unix") != 0)) {
			if ((aicode = getaddrinfo(NULL, servname, &hints, &res))!= 0) {
			  if ((aicode = getaddrinfo(NULL, "portmapper", &hints, &res))!= 0) {
			  printf("cannot get local address for %s: %s",  nconf->nc_netid, gai_strerror(aicode));
			  syslog(LOG_ERR,
				    "cannot get local address for %s: %s",
				    nconf->nc_netid, gai_strerror(aicode));
				return 1;
			  }
			}
			addrlen = res->ai_addrlen;
			sa = (struct sockaddr *)res->ai_addr;
		}
		oldmask = umask(S_IXUSR|S_IXGRP|S_IXOTH);
		__rpc_fd2sockinfo(fd, &si);
		if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on,
				sizeof(on)) != 0) {
			syslog(LOG_ERR, "cannot set SO_REUSEADDR on %s",
				nconf->nc_netid);
			if (res != NULL)
				freeaddrinfo(res);
			return 1;
		}
		if (bind(fd, sa, addrlen) < 0) {
			syslog(LOG_ERR, "cannot bind %s: %m", nconf->nc_netid);
			if (res != NULL)
				freeaddrinfo(res);
			return 1;
		}
		(void) umask(oldmask);

		/* Copy the address */
		taddr.addr.len = taddr.addr.maxlen = addrlen;
		taddr.addr.buf = malloc(addrlen);
		if (taddr.addr.buf == NULL) {
			syslog(LOG_ERR, "cannot allocate memory for %s address",
			    nconf->nc_netid);
			if (res != NULL)
				freeaddrinfo(res);
			return 1;
		}
		memcpy(taddr.addr.buf, sa, addrlen);
#ifdef RPCBIND_DEBUG
		if (debugging) {
			/* for debugging print out our universal address */
			char *uaddr;
			struct netbuf nb;
		        int sa_size2 = 0;

			nb.buf = sa;
			switch( sa->sa_family){
			case AF_INET:
			  sa_size2 = sizeof (struct sockaddr_in);
			  break;
			case AF_INET6:
			  sa_size2 = sizeof (struct sockaddr_in6);				 
			  break;
			}
			nb.len = nb.maxlen = sa_size2;
			uaddr = taddr2uaddr(nconf, &nb);
			(void) xlog(LOG_DEBUG, "rpcbind : my address is %s\n",
			    uaddr);
			(void) free(uaddr);
		}
#endif

		listen(fd, SOMAXCONN);

		my_xprt = (SVCXPRT *)svc_tli_create(fd, nconf, &taddr, RPC_MAXDATASIZE, RPC_MAXDATASIZE);
		if (my_xprt == (SVCXPRT *)NULL) {
			syslog(LOG_ERR, "%s: could not create service",
					nconf->nc_netid);
			goto error;
		}
	}
got_socket:

#ifdef PORTMAP
	/*
	 * Register both the versions for tcp/ip, udp/ip.
	 */
	if (si.si_af == AF_INET &&
	    (si.si_proto == IPPROTO_TCP || si.si_proto == IPPROTO_UDP)) {
		struct pmaplist *pml;

		pml = malloc(sizeof (struct pmaplist));
		if (pml == NULL) {
			syslog(LOG_ERR, "no memory!");
			exit(1);
		}
		pml->pml_map.pm_prog = PMAPPROG;
		pml->pml_map.pm_vers = PMAPVERS;
		pml->pml_map.pm_port = PMAPPORT;
		pml->pml_map.pm_prot = si.si_proto;

		switch (si.si_proto) {
		case IPPROTO_TCP:
			tcptrans = strdup(nconf->nc_netid);
			break;
		case IPPROTO_UDP:
			udptrans = strdup(nconf->nc_netid);
			break;
		} 
		pml->pml_next = list_pml;
		list_pml = pml;

		/* Add version 3 information */
		pml = malloc(sizeof (struct pmaplist));
		if (pml == NULL) {
			syslog(LOG_ERR, "no memory!");
			exit(1);
		}
		pml->pml_map = list_pml->pml_map;
		pml->pml_map.pm_vers = RPCBVERS;
		pml->pml_next = list_pml;
		list_pml = pml;

		/* Add version 4 information */
		pml = malloc (sizeof (struct pmaplist));
		if (pml == NULL) {
			syslog(LOG_ERR, "no memory!");
			exit(1);
		}
		pml->pml_map = list_pml->pml_map;
		pml->pml_map.pm_vers = RPCBVERS4;
		pml->pml_next = list_pml;
		list_pml = pml;

		/* Also add version 2 stuff to rpcbind list */
		rbllist_add(PMAPPROG, PMAPVERS, nconf, &taddr.addr);
	}

	/* We need to support portmap over IPv4. It makes sense to
	 * support it over AF_LOCAL as well, because that allows
	 * rpcbind to identify the owner of a socket much better
	 * than by relying on privileged ports to tell root from
	 * non-root users. */
	if (si.si_af == AF_INET || si.si_af == AF_LOCAL) {
		if (!svc_register(my_xprt, PMAPPROG, PMAPVERS, pmap_service, 0)) {
			syslog(LOG_ERR, "could not register on %s",
					nconf->nc_netid);
			goto error;
		}
	}
#endif

	/* version 3 registration */
	if (!svc_reg(my_xprt, RPCBPROG, RPCBVERS, rpcb_service_3, NULL)) {
		syslog(LOG_ERR, "could not register %s version 3",
				nconf->nc_netid);
		goto error;
	}
	rbllist_add(RPCBPROG, RPCBVERS, nconf, &taddr.addr);

	/* version 4 registration */
	if (!svc_reg(my_xprt, RPCBPROG, RPCBVERS4, rpcb_service_4, NULL)) {
		syslog(LOG_ERR, "could not register %s version 4",
				nconf->nc_netid);
		goto error;
	}
	rbllist_add(RPCBPROG, RPCBVERS4, nconf, &taddr.addr);

	/* decide if bound checking works for this transport */
	status = add_bndlist(nconf, &taddr.addr);
#ifdef RPCBIND_DEBUG
	if (debugging) {
		if (status < 0) {
			xlog(LOG_DEBUG, "Error in finding bind status for %s\n",
				nconf->nc_netid);
		} else if (status == 0) {
			xlog(LOG_DEBUG, "check binding for %s\n",
				nconf->nc_netid);
		} else if (status > 0) {
			xlog(LOG_DEBUG, "No check binding for %s\n",
				nconf->nc_netid);
		}
	}
#endif
	/*
	 * rmtcall only supported on CLTS transports for now.
	 */
	if (nconf->nc_semantics == NC_TPI_CLTS) {
		status = create_rmtcall_fd(nconf);

#ifdef RPCBIND_DEBUG
		if (debugging) {
			if (status < 0) {
				xlog(LOG_DEBUG,
				    "Could not create rmtcall fd for %s\n",
					nconf->nc_netid);
			} else {
				xlog(LOG_DEBUG, "rmtcall fd for %s is %d\n",
					nconf->nc_netid, status);
			}
		}
#endif
	}
	return (0);
error:
	close(fd);
	return (1);
}

static void
rbllist_add(rpcprog_t prog, rpcvers_t vers, struct netconfig *nconf,
	    struct netbuf *addr)
{
	rpcblist_ptr rbl;

	rbl = malloc(sizeof (rpcblist));
	if (rbl == NULL) {
		syslog(LOG_ERR, "no memory!");
		exit(1);
	}
#ifdef RPCBIND_DEBUG	
	if (debugging){
	  xlog(LOG_DEBUG,"FUNCTION rbllist_add");
	  xlog(LOG_DEBUG,"Add the prog %lu vers %lu to the rpcbind list",
                  (ulong)prog, (ulong)vers);
	}
#endif	
        rbl->rpcb_map.r_prog = prog;
	rbl->rpcb_map.r_vers = vers;
	rbl->rpcb_map.r_netid = strdup(nconf->nc_netid);
	rbl->rpcb_map.r_addr = taddr2uaddr(nconf, addr);
	rbl->rpcb_map.r_owner = strdup(superuser);
	rbl->rpcb_next = list_rbl;	/* Attach to global list */
	list_rbl = rbl;
}

/*
 * Catch the signal and die
 */
static void
terminate(int dummy /*__unused*/)
{
	close(rpcbindlockfd);
	if(createdsocket)
		unlink(_PATH_RPCBINDSOCK);
	unlink(RPCBINDDLOCK);
#ifdef WARMSTART
	write_warmstart();	/* Dump yourself */
#endif
	exit(0); /* exit gracefully */
}

void
rpcbind_abort()
{
#ifdef WARMSTART
	write_warmstart();	/* Dump yourself */
#endif
	abort();
}

/* get command line options */
static void
parseargs(int argc, char *argv[])
{
	int c;
	oldstyle_local = 1;
	while ((c = getopt(argc, argv, "adh:ilswf")) != -1) {
		switch (c) {
		case 'a':
			doabort = 1;	/* when debugging, do an abort on */
			break;		/* errors; for rpcbind developers */
					/* only! */
		case 'd':
			debugging++;
			break;
		case 'h':
			++nhosts;
			hosts = realloc(hosts, nhosts * sizeof(char *));
			if (hosts == NULL)
				errx(1, "Out of memory");
			hosts[nhosts - 1] = strdup(optarg);
			if (hosts[nhosts - 1] == NULL)
				errx(1, "Out of memory");
			break;
		case 'i':
			insecure = 1;
			break;
		case 'l':
			verboselog = 1;
			break;
		case 's':
			runasdaemon = 1;
			break;
		case 'f':
			dofork = 0;
			break;
#ifdef WARMSTART
		case 'w':
			warmstart = 1;
			break;
#endif
		default:	/* error */
			fprintf(stderr,	"usage: rpcbind [-adhilswf]\n");
			exit (1);
		}
	}
	if (doabort && !debugging) {
	    fprintf(stderr,
		"-a (abort) specified without -d (debugging) -- ignored.\n");
	    doabort = 0;
	}
}

void
reap(int dummy /*__unused*/)
{
	int save_errno = errno;
 
	while (wait3(NULL, WNOHANG, NULL) > 0)
		;       
	errno = save_errno;
}

void
toggle_verboselog(int dummy /*__unused*/)
{
	verboselog = !verboselog;
}
