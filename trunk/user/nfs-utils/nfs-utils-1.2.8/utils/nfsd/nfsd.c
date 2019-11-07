/*
 * nfsd
 *
 * This is the user level part of nfsd. This is very primitive, because
 * all the work is now done in the kernel module.
 *
 * Copyright (C) 1995, 1996 Olaf Kirch <okir@monad.swb.de>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "nfslib.h"
#include "nfssvc.h"
#include "xlog.h"

#ifndef NFSD_NPROC
#define NFSD_NPROC 8
#endif

static void	usage(const char *);

static struct option longopts[] =
{
	{ "host", 1, 0, 'H' },
	{ "help", 0, 0, 'h' },
	{ "no-nfs-version", 1, 0, 'N' },
	{ "nfs-version", 1, 0, 'V' },
	{ "no-tcp", 0, 0, 'T' },
	{ "no-udp", 0, 0, 'U' },
	{ "port", 1, 0, 'P' },
	{ "port", 1, 0, 'p' },
	{ "debug", 0, 0, 'd' },
	{ "syslog", 0, 0, 's' },
	{ NULL, 0, 0, 0 }
};

/* given a family and ctlbits, disable any that aren't listed in netconfig */
#ifdef HAVE_LIBTIRPC
static void
nfsd_enable_protos(unsigned int *proto4, unsigned int *proto6)
{
	struct netconfig *nconf;
	unsigned int *famproto;
	void *handle;

	xlog(D_GENERAL, "Checking netconfig for visible protocols.");

	handle = setnetconfig();
	while((nconf = getnetconfig(handle))) {
		if (!(nconf->nc_flag & NC_VISIBLE))
			continue;

		if (!strcmp(nconf->nc_protofmly, NC_INET))
			famproto = proto4;
		else if (!strcmp(nconf->nc_protofmly, NC_INET6))
			famproto = proto6;
		else
			continue;

		if (!strcmp(nconf->nc_proto, NC_TCP))
			NFSCTL_TCPSET(*famproto);
		else if (!strcmp(nconf->nc_proto, NC_UDP))
			NFSCTL_UDPSET(*famproto);

		xlog(D_GENERAL, "Enabling %s %s.", nconf->nc_protofmly,
			nconf->nc_proto);
	}
	endnetconfig(handle);
	return;
}
#else /* HAVE_LIBTIRPC */
static void
nfsd_enable_protos(unsigned int *proto4, unsigned int *proto6)
{
	/* Enable all IPv4 protocols if no TIRPC support */
	*proto4 = NFSCTL_ALLBITS;
	*proto6 = 0;
}
#endif /* HAVE_LIBTIRPC */

int
main(int argc, char **argv)
{
	int	count = NFSD_NPROC, c, error = 0, portnum = 0, fd, found_one;
	char *p, *progname, *port;
	char *haddr = NULL;
	int	socket_up = 0;
	int minorvers41 = 0;	/* nfsv4 minor version */
	unsigned int versbits = NFSCTL_ALLBITS;
	unsigned int protobits = NFSCTL_ALLBITS;
	unsigned int proto4 = 0;
	unsigned int proto6 = 0;

	progname = strdup(basename(argv[0]));
	if (!progname) {
		fprintf(stderr, "%s: unable to allocate memory.\n", argv[0]);
		exit(1);
	}

	port = strdup("nfs");
	if (!port) {
		fprintf(stderr, "%s: unable to allocate memory.\n", progname);
		exit(1);
	}

	xlog_syslog(0);
	xlog_stderr(1);

	while ((c = getopt_long(argc, argv, "dH:hN:V:p:P:sTU", longopts, NULL)) != EOF) {
		switch(c) {
		case 'd':
			xlog_config(D_ALL, 1);
			break;
		case 'H':
			/*
			 * for now, this only handles one -H option. Use the
			 * last one specified.
			 */
			free(haddr);
			haddr = strdup(optarg);
			if (!haddr) {
				fprintf(stderr, "%s: unable to allocate "
					"memory.\n", progname);
				exit(1);
			}
			break;
		case 'P':	/* XXX for nfs-server compatibility */
		case 'p':
			/* only the last -p option has any effect */
			portnum = atoi(optarg);
			if (portnum <= 0 || portnum > 65535) {
				fprintf(stderr, "%s: bad port number: %s\n",
					progname, optarg);
				usage(progname);
			}
			free(port);
			port = strdup(optarg);
			if (!port) {
				fprintf(stderr, "%s: unable to allocate "
						"memory.\n", progname);
				exit(1);
			}
			break;
		case 'N':
			switch((c = strtol(optarg, &p, 0))) {
			case 4:
				if (*p == '.') {
					int i = atoi(p+1);
					if (i != 1) {
						fprintf(stderr, "%s: unsupported minor version\n", optarg);
						exit(1);
					}
					minorvers41 = -1;
					break;
				}
			case 3:
			case 2:
				NFSCTL_VERUNSET(versbits, c);
				break;
			default:
				fprintf(stderr, "%s: Unsupported version\n", optarg);
				exit(1);
			}
			break;
		case 'V':
			switch((c = strtol(optarg, &p, 0))) {
			case 4:
				if (*p == '.') {
					int i = atoi(p+1);
					if (i != 1) {
						fprintf(stderr, "%s: unsupported minor version\n", optarg);
						exit(1);
					}
					minorvers41 = 1;
					break;
				}
			case 3:
			case 2:
				NFSCTL_VERSET(versbits, c);
				break;
			default:
				fprintf(stderr, "%s: Unsupported version\n", optarg);
				exit(1);
			}
			break;
		case 's':
			xlog_syslog(1);
			xlog_stderr(0);
			break;
		case 'T':
			NFSCTL_TCPUNSET(protobits);
			break;
		case 'U':
			NFSCTL_UDPUNSET(protobits);
			break;
		default:
			fprintf(stderr, "Invalid argument: '%c'\n", c);
		case 'h':
			usage(progname);
		}
	}

	if (optind < argc) {
		if ((count = atoi(argv[optind])) < 0) {
			/* insane # of servers */
			fprintf(stderr,
				"%s: invalid server count (%d), using 1\n",
				argv[0], count);
			count = 1;
		} else if (count == 0) {
			/*
			 * don't bother setting anything else if the threads
			 * are coming down anyway.
			 */
			socket_up = 1;
			goto set_threads;
		}
	}

	xlog_open(progname);

	nfsd_enable_protos(&proto4, &proto6);

	if (!NFSCTL_TCPISSET(protobits)) {
		NFSCTL_TCPUNSET(proto4);
		NFSCTL_TCPUNSET(proto6);
	}

	if (!NFSCTL_UDPISSET(protobits)) {
		NFSCTL_UDPUNSET(proto4);
		NFSCTL_UDPUNSET(proto6);
	}

	/* make sure that at least one version is enabled */
	found_one = 0;
	for (c = NFSD_MINVERS; c <= NFSD_MAXVERS; c++) {
		if (NFSCTL_VERISSET(versbits, c))
			found_one = 1;
	}
	if (!found_one) {
		xlog(L_ERROR, "no version specified");
		exit(1);
	}			

	if (NFSCTL_VERISSET(versbits, 4) &&
	    !NFSCTL_TCPISSET(proto4) &&
	    !NFSCTL_TCPISSET(proto6)) {
		xlog(L_ERROR, "version 4 requires the TCP protocol");
		exit(1);
	}

	if (chdir(NFS_STATEDIR)) {
		xlog(L_ERROR, "chdir(%s) failed: %m", NFS_STATEDIR);
		exit(1);
	}

	/* make sure nfsdfs is mounted if it's available */
	nfssvc_mount_nfsdfs(progname);

	/* can only change number of threads if nfsd is already up */
	if (nfssvc_inuse()) {
		socket_up = 1;
		goto set_threads;
	}

	/*
	 * must set versions before the fd's so that the right versions get
	 * registered with rpcbind. Note that on older kernels w/o the right
	 * interfaces, these are a no-op.
	 */
	nfssvc_setvers(versbits, minorvers41);
 
	error = nfssvc_set_sockets(AF_INET, proto4, haddr, port);
	if (!error)
		socket_up = 1;

#ifdef IPV6_SUPPORTED
	error = nfssvc_set_sockets(AF_INET6, proto6, haddr, port);
	if (!error)
		socket_up = 1;
#endif /* IPV6_SUPPORTED */

set_threads:
	/* don't start any threads if unable to hand off any sockets */
	if (!socket_up) {
		xlog(L_ERROR, "unable to set any sockets for nfsd");
		goto out;
	}
	error = 0;

	/*
	 * KLUDGE ALERT:
	 * Some kernels let nfsd kernel threads inherit open files
	 * from the program that spawns them (i.e. us).  So close
	 * everything before spawning kernel threads.  --Chip
	 */
	fd = open("/dev/null", O_RDWR);
	if (fd == -1)
		xlog(L_ERROR, "Unable to open /dev/null: %m");
	else {
		/* switch xlog output to syslog since stderr is being closed */
		xlog_syslog(1);
		xlog_stderr(0);
		(void) dup2(fd, 0);
		(void) dup2(fd, 1);
		(void) dup2(fd, 2);
	}
	closeall(3);

	if ((error = nfssvc_threads(portnum, count)) < 0)
		xlog(L_ERROR, "error starting threads: errno %d (%m)", errno);
out:
	free(port);
	free(haddr);
	free(progname);
	return (error != 0);
}

static void
usage(const char *prog)
{
	fprintf(stderr, "Usage:\n"
		"%s [-d|--debug] [-H hostname] [-p|-P|--port port] [-N|--no-nfs-version version] [-V|--nfs-version version] [-s|--syslog] [-T|--no-tcp] [-U|--no-udp] nrservs\n", 
		prog);
	exit(2);
}
