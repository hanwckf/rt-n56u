/*
 ============================================================================
 Name        : hev-event-source-signal.h
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2013 everyone.
 Description : A signal event source
 ============================================================================
 */

#include "hev-event-source.h"

#ifndef __HEV_EVENT_SOURCE_SIGNAL_H__
#define __HEV_EVENT_SOURCE_SIGNAL_H__

typedef struct _HevEventSourceSignal HevEventSourceSignal;

HevEventSource * hev_event_source_signal_new (int signal);

#endif /* __HEV_EVENT_SOURCE_SIGNAL_H__ */

