/*
 ============================================================================
 Name        : hev-event-source-fds.c
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2013 everyone.
 Description : A fds event source
 ============================================================================
 */

#include "hev-event-source-fds.h"

static bool hev_event_source_fds_dispatch (HevEventSource *source, HevEventSourceFD *fd,
			HevEventSourceFunc callback, void *data);

static HevEventSourceFuncs hev_event_source_fds_funcs =
{
	.prepare = NULL,
	.check = NULL,
	.dispatch = hev_event_source_fds_dispatch,
	.finalize = NULL,
};

HevEventSource *
hev_event_source_fds_new (void)
{
	return hev_event_source_new (&hev_event_source_fds_funcs, sizeof (HevEventSource));
}

static bool
hev_event_source_fds_dispatch (HevEventSource *source, HevEventSourceFD *fd,
			HevEventSourceFunc callback, void *data)
{
	HevEventSourceFDsFunc _callback = (HevEventSourceFDsFunc) callback;
	return _callback (fd, data);
}

