/*
 ============================================================================
 Name        : hev-event-source-signal.c
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2013 everyone.
 Description : A signal event source
 ============================================================================
 */

#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/epoll.h>
#if defined(ANDROID)
#include <fcntl.h>
#include <asm/unistd.h>
#else /* GENERIC */
#include <sys/signalfd.h>
#endif

#include "hev-event-source-signal.h"

static bool hev_event_source_signal_check (HevEventSource *source, HevEventSourceFD *fd);
static void hev_event_source_signal_finalize (HevEventSource *source);

struct _HevEventSourceSignal
{
	HevEventSource parent;

	int signal_fd;
};

#if defined(ANDROID)
#define SFD_NONBLOCK	O_NONBLOCK

struct signalfd_siginfo
{
  uint32_t ssi_signo;
  int32_t ssi_errno;
  int32_t ssi_code;
  uint32_t ssi_pid;
  uint32_t ssi_uid;
  int32_t ssi_fd;
  uint32_t ssi_tid;
  uint32_t ssi_band;
  uint32_t ssi_overrun;
  uint32_t ssi_trapno;
  int32_t ssi_status;
  int32_t ssi_int;
  uint64_t ssi_ptr;
  uint64_t ssi_utime;
  uint64_t ssi_stime;
  uint64_t ssi_addr;
  uint8_t __pad[48];
};
#endif

static HevEventSourceFuncs hev_event_source_signal_funcs =
{
	.prepare = NULL,
	.check = hev_event_source_signal_check,
	.dispatch = NULL,
	.finalize = hev_event_source_signal_finalize,
};

#if defined(ANDROID)
static int
signalfd (int fd, const sigset_t *mask, int flags)
{
	return syscall (__NR_signalfd4, fd, mask, _NSIG / 8, flags);
}
#endif

HevEventSource *
hev_event_source_signal_new (int signal)
{
	int fd = -1;
	HevEventSource *source = NULL;
	HevEventSourceSignal *self = NULL;
	sigset_t mask;

	sigemptyset (&mask);
	sigaddset (&mask, signal);
	sigprocmask(SIG_BLOCK, &mask, NULL);
	fd = signalfd (-1, &mask, SFD_NONBLOCK);
	if (-1 == fd)
	  return NULL;

	source = hev_event_source_new (&hev_event_source_signal_funcs,
				sizeof (HevEventSourceSignal));
	if (NULL == source) {
		close (fd);
		return NULL;
	}

	self = (HevEventSourceSignal *) source;
	self->signal_fd = fd;
	hev_event_source_add_fd (source, self->signal_fd, EPOLLIN | EPOLLET);

	return source;
}

static bool
hev_event_source_signal_check (HevEventSource *source, HevEventSourceFD *fd)
{
	HevEventSourceSignal *self = (HevEventSourceSignal *) source;
	if (EPOLLIN & fd->revents) {
		struct signalfd_siginfo siginfo;
		int size = read (self->signal_fd, &siginfo, sizeof (struct signalfd_siginfo));
		if (-1 == size) {
			if (EAGAIN == errno)
			  fd->revents &= ~EPOLLIN;
			return false;
		}
		return true;
	}

	return false;
}

static void
hev_event_source_signal_finalize (HevEventSource *source)
{
	HevEventSourceSignal *self = (HevEventSourceSignal *) source;
	close (self->signal_fd);
}

