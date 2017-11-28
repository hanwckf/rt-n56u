/*
 ============================================================================
 Name        : hev-event-source-timeout.h
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2013 everyone.
 Description : A timeout event source
 ============================================================================
 */

#include "hev-event-source.h"

#ifndef __HEV_EVENT_SOURCE_TIMEOUT_H__
#define __HEV_EVENT_SOURCE_TIMEOUT_H__

typedef struct _HevEventSourceTimeout HevEventSourceTimeout;

HevEventSource * hev_event_source_timeout_new (unsigned int interval);

#endif /* __HEV_EVENT_SOURCE_TIMEOUT_H__ */

