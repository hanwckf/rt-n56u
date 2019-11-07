/* 
 * Copyright (C) 1995, 1997-1999 Jeffrey A. Uphoff
 * Modified by Olaf Kirch, Oct. 1996.
 * Modified by H.J. Lu, 1998.
 * Modified by L. Hohberger of Mission Critical Linux, 2000.
 *
 * NSM for Linux.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/stat.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <rpcmisc.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <grp.h>

#include "statd.h"
#include "nfslib.h"
#include "nsm.h"

/* Socket operations */
#include <sys/types.h>
#include <sys/socket.h>

int	run_mode = 0;		/* foreground logging mode */

/* LH - I had these local to main, but it seemed silly to have 
 * two copies of each - one in main(), one static in log.c... 
 * It also eliminates the 256-char static in log.c */
static char *name_p = NULL;

/* PRC: a high-availability callout program can be specified with -H
 * When this is done, the program will receive callouts whenever clients
 * are added or deleted to the notify list */
char *ha_callout_prog = NULL;

static struct option longopts[] =
{
	{ "foreground", 0, 0, 'F' },
	{ "no-syslog", 0, 0, 'd' },
	{ "help", 0, 0, 'h' },
	{ "version", 0, 0, 'v' },
	{ "outgoing-port", 1, 0, 'o' },
	{ "port", 1, 0, 'p' },
	{ "name", 1, 0, 'n' },
	{ "state-directory-path", 1, 0, 'P' },
	{ "notify-mode", 0, 0, 'N' },
	{ "ha-callout", 1, 0, 'H' },
	{ "no-notify", 0, 0, 'L' },
	{ NULL, 0, 0, 0 }
};

extern void sm_prog_1 (struct svc_req *, register SVCXPRT *);

#ifdef SIMULATIONS
extern void simulator (int, char **);
#endif


#ifdef HAVE_TCP_WRAPPER 
#include "tcpwrapper.h"

static void 
sm_prog_1_wrapper (struct svc_req *rqstp, register SVCXPRT *transp)
{
	/* remote host authorization check */
	if (!check_default("statd", nfs_getrpccaller(transp), SM_PROG)) {
		svcerr_auth (transp, AUTH_FAILED);
		return;
	}

	sm_prog_1 (rqstp, transp);
}

#define sm_prog_1 sm_prog_1_wrapper
#endif

static void
statd_unregister(void) {
	nfs_svc_unregister(SM_PROG, SM_VERS);
}

/*
 * Signal handler.
 */
static void 
killer (int sig)
{
	statd_unregister ();
	xlog(D_GENERAL, "Caught signal %d, un-registering and exiting", sig);
	exit(0);
}

static void
sigusr (int sig)
{
	extern void my_svc_exit (void);
	xlog(D_GENERAL, "Caught signal %d, re-notifying (state %d)", sig,
								MY_STATE);
	my_svc_exit();
}

/*
 * Startup information.
 */
static void log_modes(void)
{
	char buf[128];		/* watch stack size... */

	/* No flags = no message */
	if (!run_mode) return;

	memset(buf,0,128);
	sprintf(buf,"Flags: ");
	if (run_mode & MODE_NODAEMON)
		strcat(buf,"No-Daemon ");
	if (run_mode & MODE_LOG_STDERR)
		strcat(buf,"Log-STDERR ");
#ifdef HAVE_LIBTIRPC
	strcat(buf, "TI-RPC ");
#endif

	xlog_warn(buf);
}

/*
 * Since we do more than standard statd stuff, we might need to
 * help the occasional admin. 
 */
static void 
usage(void)
{
	fprintf(stderr,"usage: %s [options]\n", name_p);
	fprintf(stderr,"      -h, -?, --help       Print this help screen.\n");
	fprintf(stderr,"      -F, --foreground     Foreground (no-daemon mode)\n");
	fprintf(stderr,"      -d, --no-syslog      Verbose logging to stderr.  Foreground mode only.\n");
	fprintf(stderr,"      -p, --port           Port to listen on\n");
	fprintf(stderr,"      -o, --outgoing-port  Port for outgoing connections\n");
	fprintf(stderr,"      -V, -v, --version    Display version information and exit.\n");
	fprintf(stderr,"      -n, --name           Specify a local hostname.\n");
	fprintf(stderr,"      -P                   State directory path.\n");
	fprintf(stderr,"      -N                   Run in notify only mode.\n");
	fprintf(stderr,"      -L, --no-notify      Do not perform any notification.\n");
	fprintf(stderr,"      -H                   Specify a high-availability callout program.\n");
}

static const char *pidfile = "/var/run/rpc.statd.pid";

int pidfd = -1;
static void create_pidfile(void)
{
	FILE *fp;

	unlink(pidfile);
	fp = fopen(pidfile, "w");
	if (!fp)
		xlog_err("Opening %s failed: %m\n", pidfile);
	fprintf(fp, "%d\n", getpid());
	pidfd = dup(fileno(fp));
	if (fclose(fp) < 0) {
		xlog_warn("Flushing pid file failed: errno %d (%m)\n",
			errno);
	}
}

static void truncate_pidfile(void)
{
	if (pidfd >= 0) {
		if (ftruncate(pidfd, 0) < 0) {
			xlog_warn("truncating pid file failed: errno %d (%m)\n",
				errno);
		}
	}
}

static void run_sm_notify(int outport)
{
	char op[20];
	char *av[6];
	int ac = 0;

	av[ac++] = "/usr/sbin/sm-notify";
	if (run_mode & MODE_NODAEMON)
		av[ac++] = "-d";
	if (outport) {
		sprintf(op, "-p%d", outport);
		av[ac++] = op;
	}
	if (run_mode & STATIC_HOSTNAME) {
		av[ac++] = "-v";
		av[ac++] = MY_NAME;
	}
	av[ac] = NULL;
	execv(av[0], av);
	fprintf(stderr, "%s: failed to run %s\n", name_p, av[0]);
	exit(2);

}
/* 
 * Entry routine/main loop.
 */
int main (int argc, char **argv)
{
	extern char *optarg;
	int pid;
	int arg;
	int port = 0, out_port = 0;
	struct rlimit rlim;

	int pipefds[2] = { -1, -1};
	char status;

	/* Default: daemon mode, no other options */
	run_mode = 0;
	xlog_stderr(0);
	xlog_syslog(1);

	/* Set the basename */
	if ((name_p = strrchr(argv[0],'/')) != NULL) {
		name_p ++;
	} else {
		name_p = argv[0];
	}

	/* Set hostname */
	MY_NAME = NULL;

	/* Process command line switches */
	while ((arg = getopt_long(argc, argv, "h?vVFNH:dn:p:o:P:L", longopts, NULL)) != EOF) {
		switch (arg) {
		case 'V':	/* Version */
		case 'v':
			printf("%s version " VERSION "\n",name_p);
			exit(0);
		case 'F':	/* Foreground/nodaemon mode */
			run_mode |= MODE_NODAEMON;
			break;
		case 'N':
			run_mode |= MODE_NOTIFY_ONLY;
			break;
		case 'L': /* Listen only */
			run_mode |= MODE_NO_NOTIFY;
			break;
		case 'd':	/* No daemon only - log to stderr */
			run_mode |= MODE_LOG_STDERR;
			break;
		case 'o':
			out_port = atoi(optarg);
			if (out_port < 1 || out_port > 65535) {
				fprintf(stderr, "%s: bad port number: %s\n",
					argv[0], optarg);
				usage();
				exit(1);
			}
			break;
		case 'p':
			port = atoi(optarg);
			if (port < 1 || port > 65535) {
				fprintf(stderr, "%s: bad port number: %s\n",
					argv[0], optarg);
				usage();
				exit(1);
			}
			break;
		case 'n':	/* Specify local hostname */
			run_mode |= STATIC_HOSTNAME;
			MY_NAME = xstrdup(optarg);
			break;
		case 'P':
			if (!nsm_setup_pathnames(argv[0], optarg))
				exit(1);
			break;
		case 'H': /* PRC: specify the ha-callout program */
			if ((ha_callout_prog = xstrdup(optarg)) == NULL) {
				fprintf(stderr, "%s: xstrdup(%s) failed!\n",
					argv[0], optarg);
				exit(1);
			}
			break;
		case '?':	/* heeeeeelllllllpppp? heh */
		case 'h':
			usage();
			exit (0);
		default:	/* oh dear ... heh */
			usage();
			exit(-1);
		}
	}

	if (port == out_port && port != 0) {
		fprintf(stderr, "Listening and outgoing ports cannot be the same!\n");
		exit(-1);
	}

	if (run_mode & MODE_NOTIFY_ONLY) {
		fprintf(stderr, "%s: -N deprecated, consider using /usr/sbin/sm-notify directly\n",
			name_p);
		run_sm_notify(out_port);
	}

	if (!(run_mode & MODE_NODAEMON)) {
		run_mode &= ~MODE_LOG_STDERR;	/* Never log to console in
						   daemon mode. */
	}

	if (getrlimit (RLIMIT_NOFILE, &rlim) != 0)
		fprintf(stderr, "%s: getrlimit (RLIMIT_NOFILE) failed: %s\n",
				argv [0], strerror(errno));
	else {
		/* glibc sunrpc code dies if getdtablesize > FD_SETSIZE */
		if (rlim.rlim_cur > FD_SETSIZE) {
			rlim.rlim_cur = FD_SETSIZE;

			if (setrlimit (RLIMIT_NOFILE, &rlim) != 0) {
				fprintf(stderr, "%s: setrlimit (RLIMIT_NOFILE) failed: %s\n",
					argv [0], strerror(errno));
			}
		}
	}

#ifdef SIMULATIONS
	if (argc > 1)
		/* LH - I _really_ need to update simulator... */
		simulator (--argc, ++argv);	/* simulator() does exit() */
#endif
	
	if (!(run_mode & MODE_NODAEMON)) {
		int tempfd;

		if (pipe(pipefds)<0) {
			perror("statd: unable to create pipe");
			exit(1);
		}
		if ((pid = fork ()) < 0) {
			perror ("statd: Could not fork");
			exit (1);
		} else if (pid != 0) {
			/* Parent.
			 * Wait for status from child.
			 */
			close(pipefds[1]);
			if (read(pipefds[0], &status, 1) != 1)
				exit(1);
			exit (0);
		}
		/* Child.	*/
		close(pipefds[0]);
		setsid ();

		while (pipefds[1] <= 2) {
			pipefds[1] = dup(pipefds[1]);
			if (pipefds[1]<0) {
				perror("statd: dup");
				exit(1);
			}
		}
		tempfd = open("/dev/null", O_RDWR);
		dup2(tempfd, 0);
		dup2(tempfd, 1);
		dup2(tempfd, 2);
		dup2(pipefds[1], 3);
		pipefds[1] = 3;
		closeall(4);
	}

	/* Child. */

	if (run_mode & MODE_LOG_STDERR) {
		xlog_syslog(0);
		xlog_stderr(1);
		xlog_config(D_ALL, 1);
	}
	xlog_open(name_p);
	xlog(L_NOTICE, "Version " VERSION " starting");

	log_modes();

	signal (SIGHUP, killer);
	signal (SIGINT, killer);
	signal (SIGTERM, killer);
	/* PRC: trap SIGUSR1 to re-read notify list from disk */
	signal(SIGUSR1, sigusr);
	/* WARNING: the following works on Linux and SysV, but not BSD! */
	signal(SIGCHLD, SIG_IGN);
	/*
	 * Ignore SIGPIPE to avoid statd dying when peers close their
	 * TCP connection while we're trying to reply to them.
	 */
	signal(SIGPIPE, SIG_IGN);

	create_pidfile();
	atexit(truncate_pidfile);

	if (! (run_mode & MODE_NO_NOTIFY))
		switch (pid = fork()) {
		case 0:
			run_sm_notify(out_port);
			break;
		case -1:
			break;
		default:
			waitpid(pid, NULL, 0);
		}

	/* Make sure we have a privilege port for calling into the kernel */
	if (statd_get_socket() < 0)
		exit(1);

	/* If sm-notify didn't take all the state files, load
	 * state information into our notify-list so we can
	 * pass on any SM_NOTIFY that arrives
	 */
	load_state();

	MY_STATE = nsm_get_state(0);
	if (MY_STATE == 0)
		exit(1);
	xlog(D_GENERAL, "Local NSM state number: %d", MY_STATE);
	nsm_update_kernel_state(MY_STATE);

	/*
	 * ORDER
	 * Clear old listeners while still root, to override any
	 * permission checking done by rpcbind.
	 */
	statd_unregister();

	/*
	 * ORDER
	 */
	if (!nsm_drop_privileges(pidfd))
		exit(1);

	/*
	 * ORDER
	 * Create RPC listeners after dropping privileges.  This permits
	 * statd to unregister its own listeners when it exits.
	 */
	if (nfs_svc_create("statd", SM_PROG, SM_VERS, sm_prog_1, port) == 0) {
		xlog(L_ERROR, "failed to create RPC listeners, exiting");
		exit(1);
	}
	atexit(statd_unregister);

	/* If we got this far, we have successfully started, so notify parent */
	if (pipefds[1] > 0) {
		status = 0;
		if (write(pipefds[1], &status, 1) != 1) {
			xlog_warn("writing to parent pipe failed: errno %d (%s)\n",
				errno, strerror(errno));
		}
		close(pipefds[1]);
		pipefds[1] = -1;
	}

	for (;;) {
		/*
		 * Handle incoming requests:  SM_NOTIFY socket requests, as
		 * well as callbacks from lockd.
		 */
		my_svc_run();	/* I rolled my own, Olaf made it better... */

		/* Only get here when simulating a crash so we should probably
		 * start sm-notify running again.  As we have already dropped
		 * privileges, this might not work, but I don't think
		 * responding to SM_SIMU_CRASH is an important use cases to
		 * get perfect.
		 */
		if (! (run_mode & MODE_NO_NOTIFY))
			switch (pid = fork()) {
			case 0:
				run_sm_notify(out_port);
				break;
			case -1:
				break;
			default:
				waitpid(pid, NULL, 0);
			}

	}
	return 0;
}
