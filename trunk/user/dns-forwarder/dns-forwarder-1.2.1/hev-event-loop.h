/*
 ============================================================================
 Name        : hev-event-loop.h
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2013 everyone.
 Description : An event loop
 ============================================================================
 */

#ifndef __HEV_EVENT_LOOP_H__
#define __HEV_EVENT_LOOP_H__

typedef struct _HevEventLoop HevEventLoop;

#include "hev-event-source.h"

HevEventLoop * hev_event_loop_new (void);

HevEventLoop * hev_event_loop_ref (HevEventLoop *self);
void hev_event_loop_unref (HevEventLoop *self);

void hev_event_loop_run (HevEventLoop *self);
void hev_event_loop_quit (HevEventLoop *self);

bool hev_event_loop_add_source (HevEventLoop *self, HevEventSource *source);
bool hev_event_loop_del_source (HevEventLoop *self, HevEventSource *source);

bool _hev_event_loop_add_fd (HevEventLoop *self, HevEventSourceFD *fd);
bool _hev_event_loop_del_fd (HevEventLoop *self, HevEventSourceFD *fd);

#endif /* __HEV_EVENT_LOOP_H__ */

