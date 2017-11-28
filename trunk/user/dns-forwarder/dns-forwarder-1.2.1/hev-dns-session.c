/*
 ============================================================================
 Name        : hev-dns-session.c
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2014 everyone.
 Description : DNS session
 ============================================================================
 */

#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include "hev-dns-session.h"

enum
{
	REMOTE_IN = (1 << 1),
	REMOTE_OUT = (1 << 0),
};

enum
{
	STEP_NULL,
	STEP_WAIT_CONNECT,
	STEP_WRITE_REQUEST,
	STEP_READ_RESPONSE,
	STEP_WRITE_RESPONSE,
	STEP_CLOSE_SESSION,
};

struct _HevDNSSession
{
	int cfd;
	int rfd;
	unsigned int ref_count;
	unsigned int step;
	bool idle;
	uint8_t revents;
	HevEventSourceFD *remote_fd;
	HevRingBuffer *forward_buffer;
	HevRingBuffer *backward_buffer;
	HevEventSource *source;
	HevDNSSessionCloseNotify notify;
	void *notify_data;
	struct sockaddr_in *upstream;
	struct sockaddr_in client_addr;
};

static int dns_read_request (HevDNSSession *self);
static void dns_do_connect (HevDNSSession *self);
static void dns_close_session (HevDNSSession *self);
static bool session_source_forward_handler (HevEventSourceFD *fd, void *data);

HevDNSSession *
hev_dns_session_new (int fd, struct sockaddr_in *upstream,
			HevDNSSessionCloseNotify notify, void *notify_data)
{
	HevDNSSession *self = HEV_MEMORY_ALLOCATOR_ALLOC (sizeof (HevDNSSession));
	if (self) {
		self->ref_count = 1;
		self->cfd = fd;
		self->rfd = -1;
		self->revents = 0;
		self->idle = false;
		self->remote_fd = NULL;
		self->forward_buffer = hev_ring_buffer_new (2000);
		self->backward_buffer = hev_ring_buffer_new (2000);
		self->source = NULL;
		self->step = STEP_NULL;
		self->notify = notify;
		self->upstream = upstream;
		self->notify_data = notify_data;
	}

	return self;
}

HevDNSSession *
hev_dns_session_ref (HevDNSSession *self)
{
	if (self)
	  self->ref_count ++;

	return self;
}

void
hev_dns_session_unref (HevDNSSession *self)
{
	if (self) {
		self->ref_count --;
		if (0 == self->ref_count) {
			if (-1 < self->rfd)
			  close (self->rfd);
			hev_ring_buffer_unref (self->forward_buffer);
			hev_ring_buffer_unref (self->backward_buffer);
			if (self->source)
			  hev_event_source_unref (self->source);
			HEV_MEMORY_ALLOCATOR_FREE (self);
		}
	}
}

HevEventSource *
hev_dns_session_get_source (HevDNSSession *self)
{
	if (self) {
		if (self->source)
		  return self->source;
		self->source = hev_event_source_fds_new ();
		if (self->source) {
			hev_event_source_set_callback (self->source,
						(HevEventSourceFunc) session_source_forward_handler, self, NULL);
		}
		return self->source;
	}

	return NULL;
}

void
hev_dns_session_start (HevDNSSession *self)
{
	if (self) {
		if (0 <= dns_read_request (self))
		  dns_do_connect (self);
	}
}

void
hev_dns_session_set_idle (HevDNSSession *self)
{
	if (self)
	  self->idle = true;
}

bool
hev_dns_session_get_idle (HevDNSSession *self)
{
	return self ? self->idle : false;
}

static size_t
iovec_size (struct iovec *iovec, size_t iovec_len)
{
	size_t i = 0, size = 0;

	for (i=0; i<iovec_len; i++)
	  size += iovec[i].iov_len;

	return size;
}

static ssize_t
read_data (int fd, HevRingBuffer *buffer, struct sockaddr_in *addr)
{
	struct msghdr mh;
	struct iovec iovec[2];
	size_t iovec_len = 0, inc_len = 0;
	ssize_t size = -2;

	iovec_len = hev_ring_buffer_writing (buffer, iovec);
	if (0 < iovec_len) {
		/* recv data */
		memset (&mh, 0, sizeof (mh));
		if (addr) {
			mh.msg_name = addr;
			mh.msg_namelen = sizeof (struct sockaddr_in);
		}
		mh.msg_iov = iovec;
		mh.msg_iovlen = iovec_len;
		size = recvmsg (fd, &mh, 0);
		inc_len = (0 > size) ? 0 : size;
		hev_ring_buffer_write_finish (buffer, inc_len);
	}

	return size;
}

static ssize_t
write_data (int fd, HevRingBuffer *buffer, struct sockaddr_in *addr)
{
	struct msghdr mh;
	struct iovec iovec[2];
	size_t iovec_len = 0, inc_len = 0;
	ssize_t size = -2;

	iovec_len = hev_ring_buffer_reading (buffer, iovec);
	if (0 < iovec_len) {
		/* send data */
		memset (&mh, 0, sizeof (mh));
		if (addr) {
			mh.msg_name = addr;
			mh.msg_namelen = sizeof (struct sockaddr_in);
		}
		mh.msg_iov = iovec;
		mh.msg_iovlen = iovec_len;
		size = sendmsg (fd, &mh, 0);
		inc_len = (0 > size) ? 0 : size;
		hev_ring_buffer_read_finish (buffer, inc_len);
	}

	return size;
}

static bool
remote_read (HevDNSSession *self)
{
	ssize_t size = read_data (self->remote_fd->fd, self->backward_buffer, NULL);
	if (-2 < size) {
		if (-1 == size) {
			if (EAGAIN == errno) {
				self->revents &= ~REMOTE_IN;
				self->remote_fd->revents &= ~EPOLLIN;
			} else {
				return false;
			}
		} else if (0 == size) {
			return false;
		}
	} else {
		self->remote_fd->revents &= ~EPOLLIN;
	}

	return true;
}

static bool
remote_write (HevDNSSession *self)
{
	ssize_t size = write_data (self->remote_fd->fd, self->forward_buffer, NULL);
	if (-2 < size) {
		if (-1 == size) {
			if (EAGAIN == errno) {
				self->revents &= ~REMOTE_OUT;
				self->remote_fd->revents &= ~EPOLLOUT;
			} else {
				return false;
			}
		}
	} else {
		self->remote_fd->revents &= ~EPOLLOUT;
	}

	return true;
}

static int
dns_read_request (HevDNSSession *self)
{
	ssize_t size;
	struct iovec iovec[2];
	unsigned short *plen;

	hev_ring_buffer_writing (self->forward_buffer, iovec);
	hev_ring_buffer_write_finish (self->forward_buffer, 2);
	size = read_data (self->cfd, self->forward_buffer, &self->client_addr);
	if (0 > size) {
		dns_close_session (self);
		return -1;
	}

	plen = iovec[0].iov_base;
	*plen = htons (size);

	return 0;
}

static void
dns_do_connect (HevDNSSession *self)
{
	int nonblock = 1;

	self->rfd = socket (AF_INET, SOCK_STREAM, 0);
	if (-1 == self->rfd) {
		dns_close_session (self);
		return;
	}
	ioctl (self->rfd, FIONBIO, (char *) &nonblock);
	/* add fd to source */
	if (self->source)
	  self->remote_fd = hev_event_source_add_fd (self->source,
				  self->rfd, EPOLLIN | EPOLLOUT | EPOLLET);
	/* connect to remote host */
	if (0 > connect (self->rfd, (struct sockaddr *) self->upstream, sizeof (struct sockaddr_in))) {
		if (EINPROGRESS != errno) {
			dns_close_session (self);
			return;
		}
	}
}

static bool
dns_wait_connect (HevDNSSession *self)
{
	if (!(REMOTE_OUT & self->revents))
	  return true;

	self->step = STEP_WRITE_REQUEST;

	return false;
}

static bool
dns_write_request (HevDNSSession *self)
{
	self->step = STEP_READ_RESPONSE;

	return false;
}

static bool
dns_read_response (HevDNSSession *self)
{
	struct iovec iovec[2];
	size_t iovec_len = 0, size = 0;
	unsigned short *plen, len;

	iovec_len = hev_ring_buffer_reading (self->backward_buffer, iovec);
	size = iovec_size (iovec, iovec_len);
	if (2 > size)
	  return true;
	plen = iovec[0].iov_base;
	len = ntohs (*plen);
	if ((len + 2) > size)
	  return true;

	self->step = STEP_WRITE_RESPONSE;
	hev_ring_buffer_read_finish (self->backward_buffer, 2);

	return false;
}

static bool
dns_write_response (HevDNSSession *self)
{
	write_data (self->cfd, self->backward_buffer, &self->client_addr);
	self->step = STEP_CLOSE_SESSION;

	return false;
}

static void
dns_close_session (HevDNSSession *self)
{
	if (self->notify)
	  self->notify (self, self->notify_data);
}

static int
handle_forward (HevDNSSession *self)
{
	bool wait = false;

	switch (self->step) {
	case STEP_NULL:
		self->step = STEP_WAIT_CONNECT;
	case STEP_WAIT_CONNECT:
		wait = dns_wait_connect (self);
		break;
	case STEP_WRITE_REQUEST:
		wait = dns_write_request (self);
		break;
	case STEP_READ_RESPONSE:
		wait = dns_read_response (self);
		break;
	case STEP_WRITE_RESPONSE:
		wait = dns_write_response (self);
		break;
	case STEP_CLOSE_SESSION:
	default:
		return -1;
	}

	return wait ? 1 : 0;
}
static bool
session_source_forward_handler (HevEventSourceFD *fd, void *data)
{
	HevDNSSession *self = data;
	int wait = -1;

	if ((EPOLLERR | EPOLLHUP) & fd->revents)
	  goto close_session;

	if (fd == self->remote_fd) {
		if (EPOLLIN & fd->revents)
		  self->revents |= REMOTE_IN;
		if (EPOLLOUT & fd->revents)
		  self->revents |= REMOTE_OUT;
	}

	do {
		if (REMOTE_OUT & self->revents) {
			if (!remote_write (self))
			  goto close_session;
		}
		if (REMOTE_IN & self->revents) {
			if (!remote_read (self))
			  goto close_session;
		}

		wait = handle_forward (self);
		if (-1 == wait)
		  goto close_session;
	} while (0 == wait);

	self->idle = false;

	return true;

close_session:
	dns_close_session (self);

	return true;
}

