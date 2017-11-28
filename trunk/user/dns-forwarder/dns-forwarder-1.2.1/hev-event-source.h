/*
 ============================================================================
 Name        : hev-event-source.h
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2013 everyone.
 Description : An event source
 ============================================================================
 */

#include "hev-event-loop.h"

#ifndef __HEV_EVENT_SOURCE_H__
#define __HEV_EVENT_SOURCE_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "hev-memory-allocator.h"
#include "hev-slist.h"

typedef struct _HevEventSource HevEventSource;
typedef struct _HevEventSourceFuncs HevEventSourceFuncs;
typedef bool (*HevEventSourceFunc) (void *data);

#include "hev-event-source-fd.h"

struct _HevEventSourceFuncs
{
	bool (*prepare) (HevEventSource *self);
	bool (*check) (HevEventSource *self, HevEventSourceFD *fd);
	bool (*dispatch) (HevEventSource *self, HevEventSourceFD *fd,
				HevEventSourceFunc callback, void *data);
	void (*finalize) (HevEventSource *self);
};

struct _HevEventSource
{
	char *name;
	int priority;
	unsigned int ref_count;

	HevEventSourceFuncs funcs;
	struct {
		void *data;
		HevEventSourceFunc callback;
		HevDestroyNotify notify;
	} callback;

	HevSList *fds;
	HevEventLoop *loop;
};

HevEventSource * hev_event_source_new (HevEventSourceFuncs *funcs, size_t struct_size);

HevEventSource * hev_event_source_ref (HevEventSource *self);
void hev_event_source_unref (HevEventSource *self);

void hev_event_source_set_name (HevEventSource *self, const char *name);
const char * hev_event_source_get_name (HevEventSource *self);

void hev_event_source_set_priority (HevEventSource *self, int priority);
int hev_event_source_get_priority (HevEventSource *self);

void hev_event_source_set_callback (HevEventSource *self, HevEventSourceFunc callback,
			void *data, HevDestroyNotify notify);

HevEventSourceFD * hev_event_source_add_fd (HevEventSource *self, int fd, uint32_t events);
bool hev_event_source_del_fd (HevEventSource *self, int fd);

HevEventLoop * hev_event_source_get_loop (HevEventSource *self);
void _hev_event_source_set_loop (HevEventSource *self, HevEventLoop *loop);

#endif /* __HEV_EVENT_SOURCE_H__ */

