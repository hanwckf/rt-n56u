/*
 ============================================================================
 Name        : hev-dns-forwarder.h
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2014 everyone.
 Description : DNS Forwarder
 ============================================================================
 */

#ifndef __HEV_DNS_FORWARDER_H__
#define __HEV_DNS_FORWARDER_H__

#include "hev-dns-session.h"
#include "hev-event-source-timeout.h"

typedef struct _HevDNSForwarder HevDNSForwarder;

HevDNSForwarder * hev_dns_forwarder_new (HevEventLoop *loop,
			const char *addr, const char *port,
			const char *upstream, const char *upstream_port);

HevDNSForwarder * hev_dns_forwarder_ref (HevDNSForwarder *self);
void hev_dns_forwarder_unref (HevDNSForwarder *self);

#endif /* __HEV_DNS_FORWARDER_H__ */

