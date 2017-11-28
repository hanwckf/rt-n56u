/*
 ============================================================================
 Name        : hev-event-source.c
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2013 everyone.
 Description : An event source
 ============================================================================
 */

#include <stdint.h>
#include <string.h>

#include "hev-event-source.h"

static bool hev_event_source_prepare_default (HevEventSource *source);
static bool hev_event_source_check_default (HevEventSource *source, HevEventSourceFD *fd);
static bool hev_event_source_dispatch_default (HevEventSource *source, HevEventSourceFD *fd,
			HevEventSourceFunc callback, void *data);
static void hev_event_source_finalize_default (HevEventSource *self);

HevEventSource *
hev_event_source_new (HevEventSourceFuncs *funcs, size_t struct_size)
{
	if (sizeof (HevEventSource) <= struct_size) {
		HevEventSource *self = (HevEventSource *) HEV_MEMORY_ALLOCATOR_ALLOC (struct_size);
		if (self) {
			self->name = NULL;
			self->priority = 0;
			self->ref_count = 1;
			if (funcs)
			  memcpy (&self->funcs, funcs, sizeof (HevEventSourceFuncs));
			if (!self->funcs.prepare)
			  self->funcs.prepare = hev_event_source_prepare_default;
			if (!self->funcs.check)
			  self->funcs.check = hev_event_source_check_default;
			if (!self->funcs.dispatch)
			  self->funcs.dispatch = hev_event_source_dispatch_default;
			if (!self->funcs.finalize)
			  self->funcs.finalize = hev_event_source_finalize_default;
			self->callback.data = NULL;
			self->callback.callback = NULL;
			self->callback.notify = NULL;
			self->fds = NULL;
			self->loop = NULL;
			return self;
		}
	}

	return NULL;
}

HevEventSource *
hev_event_source_ref (HevEventSource *self)
{
	if (self) {
		self->ref_count ++;
		return self;
	}

	return NULL;
}

void
hev_event_source_unref (HevEventSource *self)
{
	if (self) {
		self->ref_count --;
		if (0 == self->ref_count) {
			HevSList *list = NULL;
			self->funcs.finalize (self);
			if (self->name)
			  HEV_MEMORY_ALLOCATOR_FREE (self->name);
			if (self->callback.notify)
			  self->callback.notify (self->callback.data);
			for (list=self->fds; list; list=hev_slist_next (list)) {
				_hev_event_source_fd_clear_source (hev_slist_data (list));
				_hev_event_source_fd_unref (hev_slist_data (list));
			}
			hev_slist_free (self->fds);
			HEV_MEMORY_ALLOCATOR_FREE (self);
		}
	}
}

void
hev_event_source_set_name (HevEventSource *self, const char *name)
{
	if (self && name) {
		if (self->name)
		  HEV_MEMORY_ALLOCATOR_FREE (self->name);
		self->name = HEV_MEMORY_ALLOCATOR_ALLOC (strlen (name));
		strcpy (self->name, name);
	}
}

const char *
hev_event_source_get_name (HevEventSource *self)
{
	return self ? self->name : NULL;
}

void
hev_event_source_set_priority (HevEventSource *self, int priority)
{
	if (self)
	  self->priority = priority;
}

int
hev_event_source_get_priority (HevEventSource *self)
{
	return self ? self->priority : INT32_MAX;
}

void
hev_event_source_set_callback (HevEventSource *self, HevEventSourceFunc callback,
			void *data, HevDestroyNotify notify)
{
	if (self) {
		self->callback.data = data;
		self->callback.callback = callback;
		self->callback.notify = notify;
	}
}

HevEventSourceFD *
hev_event_source_add_fd (HevEventSource *self, int fd, uint32_t events)
{
	if (self) {
		HevSList *list = NULL;
		for (list=self->fds; list; list=hev_slist_next (list)) {
			HevEventSourceFD *efd = hev_slist_data (list);
			if (efd->fd == fd)
			  return NULL;
		}
		HevEventSourceFD *efd = _hev_event_source_fd_new (self, fd, events);
		if (efd) {
			self->fds = hev_slist_append (self->fds, efd);
			if (self->loop && !_hev_event_loop_add_fd (self->loop, efd)) {
				_hev_event_source_fd_unref (efd);
				efd = NULL;
			}
			return efd;
		}
	}

	return NULL;
}

bool
hev_event_source_del_fd (HevEventSource *self, int fd)
{
	if (self) {
		HevSList *list = NULL;
		HevEventSourceFD *rfd = NULL;
		for (list=self->fds; list; list=hev_slist_next (list)) {
			HevEventSourceFD *efd = hev_slist_data (list);
			if (efd->fd == fd) {
				rfd = efd;
				break;
			}
		}
		if (rfd) {
			bool res = false;
			self->fds = hev_slist_remove (self->fds, rfd);
			if (self->loop)
			  res = _hev_event_loop_del_fd (self->loop, rfd);
			_hev_event_source_fd_clear_source (rfd);
			_hev_event_source_fd_unref (rfd);
			return res;
		}
	}

	return false;
}

HevEventLoop *
hev_event_source_get_loop (HevEventSource *self)
{
	return self ? self->loop : NULL;
}

void
_hev_event_source_set_loop (HevEventSource *self, HevEventLoop *loop)
{
	if (self && (!self->loop) && loop)
	  self->loop = loop;
}

static bool
hev_event_source_prepare_default (HevEventSource *source)
{
	return true;
}

static bool
hev_event_source_check_default (HevEventSource *source, HevEventSourceFD *fd)
{
	return true;
}

static bool
hev_event_source_dispatch_default (HevEventSource *source, HevEventSourceFD *fd,
			HevEventSourceFunc callback, void *data)
{
	if (callback)
	  return callback (data);
	return true;
}

static void
hev_event_source_finalize_default (HevEventSource *self)
{
}

