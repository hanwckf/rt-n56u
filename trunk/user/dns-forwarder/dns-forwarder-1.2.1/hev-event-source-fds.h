/*
 ============================================================================
 Name        : hev-event-source-fds.h
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2013 everyone.
 Description : A fds event source
 ============================================================================
 */

#include "hev-event-source.h"

#ifndef __HEV_EVENT_SOURCE_FDS_H__
#define __HEV_EVENT_SOURCE_FDS_H__

typedef struct _HevEventSourceFDs HevEventSourceFDs;
typedef bool (*HevEventSourceFDsFunc) (HevEventSourceFD *fd, void *data);

HevEventSource * hev_event_source_fds_new (void);

#endif /* __HEV_EVENT_SOURCE_FDS_H__ */

