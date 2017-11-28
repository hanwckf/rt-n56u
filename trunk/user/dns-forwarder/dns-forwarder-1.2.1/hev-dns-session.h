/*
 ============================================================================
 Name        : hev-dns-session.h
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2014 everyone.
 Description : DNS session
 ============================================================================
 */

#ifndef __HEV_DNS_SESSION_H__
#define __HEV_DNS_SESSION_H__

#include <netinet/in.h>
#include <arpa/inet.h>

#include "hev-ring-buffer.h"
#include "hev-event-source.h"
#include "hev-event-source-fds.h"

typedef struct _HevDNSSession HevDNSSession;
typedef void (*HevDNSSessionCloseNotify) (HevDNSSession *self, void *data);

HevDNSSession * hev_dns_session_new (int fd, struct sockaddr_in *upstream,
			HevDNSSessionCloseNotify notify, void *notify_data);

HevDNSSession * hev_dns_session_ref (HevDNSSession *self);
void hev_dns_session_unref (HevDNSSession *self);

HevEventSource * hev_dns_session_get_source (HevDNSSession *self);
void hev_dns_session_start (HevDNSSession *self);

void hev_dns_session_set_idle (HevDNSSession *self);
bool hev_dns_session_get_idle (HevDNSSession *self);

#endif /* __HEV_DNS_SESSION_H__ */

