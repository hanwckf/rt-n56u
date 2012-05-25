/***********************************************************************
*
* main.c
*
* Main program of l2tp
*
* Copyright (C) 2002 by Roaring Penguin Software Inc.
*
* This software may be distributed under the terms of the GNU General
* Public License, Version 2, or (at your option) any later version.
*
* LIC: GPL
*
***********************************************************************/

static char const RCSID[] =
"$Id: main.c 3323 2011-09-21 18:45:48Z lly.dev $";

#include "l2tp.h"
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <syslog.h>

static void
usage(int argc, char *argv[], int exitcode)
{
    fprintf(stderr, "\nl2tpd Version %s Copyright 2002 Roaring Penguin Software Inc.\n", VERSION);
    fprintf(stderr, "http://www.roaringpenguin.com/\n\n");
    fprintf(stderr, "Usage: %s [options]\n", argv[0]);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "-d level               -- Set debugging to 'level'\n");
    fprintf(stderr, "-f                     -- Do not fork\n");
    fprintf(stderr, "-h                     -- Print usage\n");
    fprintf(stderr, "\nThis program is licensed under the terms of\nthe GNU General Public License, Version 2.\n");
    exit(exitcode);
}

static void
sighandler(int signum)
{
    static int count = 0;

    count++;
    fprintf(stderr, "Caught signal %d times\n", count);
    if (count < 5) {
	l2tp_cleanup();
    }
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    EventSelector *es = Event_CreateSelector();
    int i;
    int opt;
    int do_fork = 1;
    int debugmask = 0;

    while((opt = getopt(argc, argv, "d:fh")) != -1) {
	switch(opt) {
	case 'h':
	    usage(argc, argv, EXIT_SUCCESS);
	    break;
	case 'f':
	    do_fork = 0;
	    break;
	case 'd':
	    sscanf(optarg, "%d", &debugmask);
	    break;
	default:
	    usage(argc, argv, EXIT_FAILURE);
	}
    }

    openlog(argv[0], LOG_PID, LOG_DAEMON);
    l2tp_random_init();
    l2tp_tunnel_init(es);
    l2tp_peer_init();
    l2tp_debug_set_bitmask(debugmask);

    if (l2tp_parse_config_file(es, SYSCONFDIR"/l2tp/l2tp.conf") < 0) {
	l2tp_die();
    }

    if (l2tp_network_init(es) < 0) {
	l2tp_die();
    }

    /* Daemonize */
    if (do_fork) {
	i = fork();
	if (i < 0) {
	    perror("fork");
	    exit(EXIT_FAILURE);
	} else if (i != 0) {
	    /* Parent */
	    exit(EXIT_SUCCESS);
	}

	setsid();
	signal(SIGHUP, SIG_IGN);
	i = fork();
	if (i < 0) {
	    perror("fork");
	    exit(EXIT_FAILURE);
	} else if (i != 0) {
	    exit(EXIT_SUCCESS);
	}

	chdir("/");

	/* Point stdin/stdout/stderr to /dev/null */
	for (i=0; i<3; i++) {
	    close(i);
	}
	i = open("/dev/null", O_RDWR);
	if (i >= 0) {
	    dup2(i, 0);
	    dup2(i, 1);
	    dup2(i, 2);
	    if (i > 2) close(i);
	}
    }

    Event_HandleSignal(es, SIGINT, sighandler);
    Event_HandleSignal(es, SIGTERM, sighandler);

    while(1) {
	i = Event_HandleEvent(es);
	if (i < 0) {
	    fprintf(stderr, "Event_HandleEvent returned %d\n", i);
	    l2tp_cleanup();
	    exit(EXIT_FAILURE);
	}
    }
    return 0;
}
