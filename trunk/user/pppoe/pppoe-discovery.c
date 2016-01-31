/***********************************************************************
*
* pppoe-discovery.c
*
* Implementation of user-space PPPoE discovery for Linux.
*
* Copyright (C) 2000-2015 by Roaring Penguin Software Inc.
*
* This program may be distributed according to the terms of the GNU
* General Public License, version 2 or (at your option) any later version.
*
* LIC: GPL
*
***********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define _GNU_SOURCE 1
#include "pppoe.h"

#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/* Default interface if no -I option given */
#if defined (USE_SINGLE_MAC)
#define DEFAULT_IF	"eth2.2"
#else
#define DEFAULT_IF	"eth3"
#endif

void rp_fatal(char const *str)
{
    fprintf(stderr, "%s\n", str);
    exit(1);
}

void fatalSys(char const *str)
{
    perror(str);
    exit(1);
}

void sysErr(char const *str)
{
    rp_fatal(str);
}

char *xstrdup(const char *s)
{
    register char *ret = strdup(s);
    if (!ret)
	sysErr("strdup");
    return ret;
}

void usage(void)
{
    fprintf(stderr, "Usage: pppoe-discovery [options]\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "   -I if_name     -- Specify interface (default %s).\n", DEFAULT_IF);
#ifdef DEBUGGING_ENABLED
    fprintf(stderr, "   -D filename    -- Log debugging information in filename.\n");
#endif
    fprintf(stderr,
        "   -S name        -- Set desired service name.\n"
        "   -C name        -- Set desired access concentrator name.\n"
        "   -U             -- Use Host-Unique to allow multiple PPPoE sessions.\n"
        "   -t timeout     -- Initial timeout for discovery packets in seconds.\n"
        "   -h             -- Print usage information.\n\n"
    );

    fprintf(stderr, "PPPoE Version %s, Copyright (C) 2001-2015 Roaring Penguin Software Inc.\n", RP_VERSION);
}

int main(int argc, char *argv[])
{
    int opt;
    PPPoEConnection conn;

    memset(&conn, 0, sizeof(PPPoEConnection));

    conn.discoverySocket = -1;
    conn.sessionSocket = -1;
    conn.printACNames = 1;
    conn.discoveryTimeout = PADI_TIMEOUT;

    while ((opt = getopt(argc, argv, "I:D:VUAS:C:t:h")) > 0) {
	switch(opt) {
	case 't':
	    conn.discoveryTimeout = atoi(optarg);
	    if (conn.discoveryTimeout < 1)
		conn.discoveryTimeout = 1;
	    break;
	case 'S':
	    conn.serviceName = xstrdup(optarg);
	    break;
	case 'C':
	    conn.acName = xstrdup(optarg);
	    break;
	case 'U':
	    conn.useHostUniq = 1;
	    break;
#ifdef DEBUGGING_ENABLED
	case 'D':
	    conn.debugFile = fopen(optarg, "w");
	    if (!conn.debugFile) {
		fprintf(stderr, "Could not open %s: %s\n",
			optarg, strerror(errno));
		exit(1);
	    }
	    fprintf(conn.debugFile, "pppoe-discovery %s\n", RP_VERSION);
	    break;
#endif
	case 'I':
	    conn.ifName = xstrdup(optarg);
	    break;
	case 'A':
	    /* this is the default */
	    break;
	case 'V':
	    printf("PPPoE Version %s, Copyright (C) 2001-2015 Roaring Penguin Software Inc.\n", RP_VERSION);
	    exit(0);
	    break;
	case 'h':
	    usage();
	    exit(0);
	default:
	    usage();
	    exit(1);
	}
    }

    /* default interface name */
    if (!conn.ifName)
        conn.ifName = xstrdup(DEFAULT_IF);

    conn.discoverySocket = openInterface(conn.ifName, Eth_PPPOE_Discovery, conn.myEth, NULL);

    discovery(&conn);

    if (conn.serviceName)
        free(conn.serviceName);
    if (conn.acName)
        free(conn.acName);
    if (conn.ifName)
        free(conn.ifName);
#ifdef DEBUGGING_ENABLED
    if (conn.debugFile)
        fclose(conn.debugFile);
#endif
    if (conn.discoverySocket != -1)
        close(conn.discoverySocket);

    exit(0);
}

