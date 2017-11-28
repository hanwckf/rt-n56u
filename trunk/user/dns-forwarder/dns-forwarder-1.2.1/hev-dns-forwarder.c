/*
 ============================================================================
 Name        : hev-dns-forwarder.c
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2014 everyone.
 Description : DNS Forwarder
 ============================================================================
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "hev-dns-forwarder.h"
#include "hev-dns-session.h"

#define TIMEOUT		(10 * 1000)

struct _HevDNSForwarder
{
	int listen_fd;
	unsigned int ref_count;
	HevEventSource *listener_source;
	HevEventSource *timeout_source;
	HevSList *session_list;

	HevEventLoop *loop;
	struct sockaddr_in upstream;
};

static bool listener_source_handler (HevEventSourceFD *fd, void *data);
static bool timeout_source_handler (void *data);
static void session_close_handler (HevDNSSession *session, void *data);
static void remove_all_sessions (HevDNSForwarder *self);

HevDNSForwarder *
hev_dns_forwarder_new (HevEventLoop *loop, const char *addr, const char *port,
			const char *upstream, const char *upstream_port)
{
	HevDNSForwarder *self = HEV_MEMORY_ALLOCATOR_ALLOC (sizeof (HevDNSForwarder));
	if (self) {
		int r, nonblock = 1, reuseaddr = 1;
		struct addrinfo hints;
		struct addrinfo *addr_ip;

		/* listen socket */
		self->listen_fd = socket (AF_INET, SOCK_DGRAM, 0);
		if (0 > self->listen_fd) {
			HEV_MEMORY_ALLOCATOR_FREE (self);
			fprintf (stderr, "socket error\n");
			return NULL;
		}
		ioctl (self->listen_fd, FIONBIO, (char *) &nonblock);
		setsockopt (self->listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof (reuseaddr));
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		if (0 != (r = getaddrinfo(addr, port, &hints, &addr_ip))) {
			fprintf(stderr, "%s:%s:%s\n", gai_strerror(r), addr, port);
			return NULL;
		}
		if (0 != bind(self->listen_fd, addr_ip->ai_addr, addr_ip->ai_addrlen)) {
			close (self->listen_fd);
			HEV_MEMORY_ALLOCATOR_FREE (self);
			fprintf (stderr, "Can't bind address %s:%s\n", addr, port);
			return NULL;
		}
		freeaddrinfo(addr_ip);

		/* event source fds for listener */
		self->listener_source = hev_event_source_fds_new ();
		hev_event_source_set_priority (self->listener_source, 1);
		hev_event_source_add_fd (self->listener_source, self->listen_fd, EPOLLIN | EPOLLET);
		hev_event_source_set_callback (self->listener_source,
					(HevEventSourceFunc) listener_source_handler, self, NULL);
		hev_event_loop_add_source (loop, self->listener_source);
		hev_event_source_unref (self->listener_source);

		/* event source timeout */
		self->timeout_source = hev_event_source_timeout_new (TIMEOUT);
		hev_event_source_set_priority (self->timeout_source, -1);
		hev_event_source_set_callback (self->timeout_source, timeout_source_handler, self, NULL);
		hev_event_loop_add_source (loop, self->timeout_source);
		hev_event_source_unref (self->timeout_source);

		self->ref_count = 1;
		self->session_list = NULL;
		self->loop = loop;

		/* upstream address */
		memset (&self->upstream, 0, sizeof (self->upstream));
		self->upstream.sin_family = AF_INET;
		if (0 == inet_aton (upstream, &self->upstream.sin_addr)) {
			fprintf (stderr, "invalid upstream %s\n", upstream);
			return NULL;
		}
		self->upstream.sin_port = htons (atoi (upstream_port));
	}

	return self;
}

HevDNSForwarder *
hev_dns_forwarder_ref (HevDNSForwarder *self)
{
	if (self) {
		self->ref_count ++;
		return self;
	}

	return NULL;
}

void
hev_dns_forwarder_unref (HevDNSForwarder *self)
{
	if (self) {
		self->ref_count --;
		if (0 == self->ref_count) {
			hev_event_loop_del_source (self->loop, self->listener_source);
			hev_event_loop_del_source (self->loop, self->timeout_source);
			close (self->listen_fd);
			remove_all_sessions (self);
			HEV_MEMORY_ALLOCATOR_FREE (self);
		}
	}
}

static bool
listener_source_handler (HevEventSourceFD *fd, void *data)
{
	HevDNSForwarder *self = data;
	HevDNSSession *session = NULL;
	HevEventSource *source = NULL;
	ssize_t size;

	size = recvfrom (fd->fd, NULL, 0, MSG_PEEK, NULL, NULL);
	if (0 > size) {
		if (EAGAIN == errno)
		  fd->revents &= ~EPOLLIN;
	} else {
		session = hev_dns_session_new (fd->fd, &self->upstream, session_close_handler, self);
		source = hev_dns_session_get_source (session);
		hev_event_loop_add_source (self->loop, source);
		/* printf ("New session %p\n", session); */
		self->session_list = hev_slist_append (self->session_list, session);
		hev_dns_session_start (session);
	}

	return true;
}

static bool
timeout_source_handler (void *data)
{
	HevDNSForwarder *self = data;
	HevSList *list = NULL;
	for (list=self->session_list; list; list=hev_slist_next (list)) {
		HevDNSSession *session = hev_slist_data (list);
		if (hev_dns_session_get_idle (session)) {
			/* printf ("Remove timeout session %p\n", session); */
			hev_event_loop_del_source (self->loop,
						hev_dns_session_get_source (session));
			hev_dns_session_unref (session);
			hev_slist_set_data (list, NULL);
		} else {
			hev_dns_session_set_idle (session);
		}
	}
	self->session_list = hev_slist_remove_all (self->session_list, NULL);

	return true;
}

static void
session_close_handler (HevDNSSession *session, void *data)
{
	HevDNSForwarder *self = data;

	/* printf ("Remove session %p\n", session); */
	hev_event_loop_del_source (self->loop,
				hev_dns_session_get_source (session));
	hev_dns_session_unref (session);
	self->session_list = hev_slist_remove (self->session_list, session);
}

static void
remove_all_sessions (HevDNSForwarder *self)
{
	HevSList *list = NULL;
	for (list=self->session_list; list; list=hev_slist_next (list)) {
		HevDNSSession *session = hev_slist_data (list);
		/* printf ("Remove session %p\n", session); */
		hev_event_loop_del_source (self->loop,
					hev_dns_session_get_source (session));
		hev_dns_session_unref (session);
	}
	hev_slist_free (self->session_list);
}

