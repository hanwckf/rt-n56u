#ifndef __IP_SET_PORTMAP_H
#define __IP_SET_PORTMAP_H

#include <linux/netfilter_ipv4/ip_set.h>

#define SETTYPE_NAME	"portmap"
#define MAX_RANGE	0x0000FFFF
#define INVALID_PORT	(MAX_RANGE + 1)

struct ip_set_portmap {
	void *members;			/* the portmap proper */
	ip_set_ip_t first_port;		/* host byte order, included in range */
	ip_set_ip_t last_port;		/* host byte order, included in range */
};

struct ip_set_req_portmap_create {
	ip_set_ip_t from;
	ip_set_ip_t to;
};

struct ip_set_req_portmap {
	ip_set_ip_t port;
};

#endif /* __IP_SET_PORTMAP_H */
