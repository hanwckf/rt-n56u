/*
 ============================================================================
 Name        : hev-main.c
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2014 everyone.
 Description : Main
 ============================================================================
 */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include "hev-main.h"
#include "hev-dns-forwarder.h"
#include "hev-event-source-signal.h"

static const char *default_dns_servers = "8.8.8.8:53";
static const char *default_dns_port = "53";
static const char *default_listen_addr = "0.0.0.0";
static const char *default_listen_port = "5300";

static void
usage (const char *app)
{
	printf ("\
usage: %s [-h] [-b BIND_ADDR] [-p BIND_PORT] [-s DNS]\n\
Forwarding DNS queries on TCP transport.\n\
\n\
  -b BIND_ADDR          address that listens, default: 0.0.0.0\n\
  -p BIND_PORT          port that listens, default: 5300\n\
  -s DNS:[PORT]         DNS servers to use, default: 8.8.8.8:53\n\
  -h                    show this help message and exit\n", app);
}

static bool
signal_handler (void *data)
{
	HevEventLoop *loop = data;
	hev_event_loop_quit (loop);
	return false;
}

int
main (int argc, char **argv)
{
	HevEventLoop *loop = NULL;
	HevEventSource *source = NULL;
	HevDNSForwarder *forwarder = NULL;

	int ch;
	char *listen_addr = NULL;
	char *listen_port = NULL;
	char *dns_servers = NULL;
	char *dns_port = NULL;

	while ((ch = getopt(argc, argv, "hb:p:s:")) != -1) {
		switch (ch) {
			case 'h':
				usage(argv[0]);
				exit(0);
			case 'b':
				listen_addr = strdup(optarg);
				break;
			case 'p':
				listen_port = strdup(optarg);
				break;
			case 's':
				dns_servers = strdup(optarg);
				break;
		}
	}

	if (dns_servers == NULL) {
		dns_servers = strdup(default_dns_servers);
	}
	dns_port = strpbrk(dns_servers, ":#");
	if (dns_port == NULL) {
		dns_port = strdup(default_dns_port);
	} else {
		*dns_port++ = '\0';
	}
	if (listen_addr == NULL) {
		listen_addr = strdup(default_listen_addr);
	}
	if (listen_port == NULL) {
		listen_port = strdup(default_listen_port);
	}

	loop = hev_event_loop_new ();

	signal (SIGPIPE, SIG_IGN);

	source = hev_event_source_signal_new (SIGINT);
	hev_event_source_set_priority (source, 3);
	hev_event_source_set_callback (source, signal_handler, loop, NULL);
	hev_event_loop_add_source (loop, source);
	hev_event_source_unref (source);

	forwarder = hev_dns_forwarder_new (loop, listen_addr, listen_port, dns_servers, dns_port);
	if (forwarder) {
		hev_event_loop_run (loop);
		hev_dns_forwarder_unref (forwarder);
	}

	hev_event_loop_unref (loop);

	return 0;
}

