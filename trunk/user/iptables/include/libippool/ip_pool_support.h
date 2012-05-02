/* support function prototypes for IP pool management (config file, mostly) */
#ifndef _IP_POOL_SUPPORT_H
#define _IP_POOL_SUPPORT_H

#include <iptables.h>
#include <linux/netfilter_ipv4/ip_conntrack.h>
#include <linux/netfilter_ipv4/ip_pool.h>

#ifndef IPPOOL_CONF
#define IPPOOL_CONF "/etc/ippool.conf"
#endif

/* called just to draw in this support .o */
void ip_pool_init(void);

/* given a pool name (or number), return pool index, possibly reading .conf */
ip_pool_t ip_pool_get_index(char *name);

/* given a pool index, and a buffer to store a name, search for the index
 * in the .conf file, and give the textual name, if present; if not, the
 * numeric index is returned. If numeric_flag == 1, the numeric index is
 * always returned
 */
char *ip_pool_get_name(char *buf, int size, ip_pool_t index, int numeric_flag);

#endif /*_IP_POOL_SUPPORT_H*/
