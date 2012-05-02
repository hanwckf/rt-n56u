#ifndef __LINUX_BRIDGE_NETFILTER_H
#define __LINUX_BRIDGE_NETFILTER_H

/* bridge-specific defines for netfilter. 
 */

#include <linux/netfilter.h>
#include <linux/if_ether.h>
#include <linux/if_pppox.h>
#include <linux/if_vlan.h>
#include <linux/if_pppox.h>

/* Bridge Hooks */
/* After promisc drops, checksum checks. */
#define NF_BR_PRE_ROUTING	0
/* If the packet is destined for this box. */
#define NF_BR_LOCAL_IN		1
/* If the packet is destined for another interface. */
#define NF_BR_FORWARD		2
/* Packets coming from a local process. */
#define NF_BR_LOCAL_OUT		3
/* Packets about to hit the wire. */
#define NF_BR_POST_ROUTING	4
/* Not really a hook, but used for the ebtables broute table */
#define NF_BR_BROUTING		5
#define NF_BR_NUMHOOKS		6

#ifdef __KERNEL__

#ifdef CONFIG_BRIDGE_NETFILTER

enum nf_br_hook_priorities {
	NF_BR_PRI_FIRST = INT_MIN,
	NF_BR_PRI_NAT_DST_BRIDGED = -300,
	NF_BR_PRI_FILTER_BRIDGED = -200,
	NF_BR_PRI_BRNF = 0,
	NF_BR_PRI_NAT_DST_OTHER = 100,
	NF_BR_PRI_FILTER_OTHER = 200,
	NF_BR_PRI_NAT_SRC = 300,
	NF_BR_PRI_LAST = INT_MAX,
};


#define BRNF_PKT_TYPE			0x01
#define BRNF_BRIDGED_DNAT		0x02
#define BRNF_DONT_TAKE_PARENT		0x04
#define BRNF_BRIDGED			0x08
#define BRNF_NF_BRIDGE_PREROUTING	0x10
#define BRNF_PPPoE                      0x20

static inline unsigned int nf_bridge_mtu_reduction(const struct sk_buff *skb)
{
	if (unlikely(skb->nf_bridge->mask & BRNF_PPPoE))
		return PPPOE_SES_HLEN;
	return 0;
}

/* This is called by the IP fragmenting code and it ensures there is
 * enough room for the encapsulating header (if there is one). */
static inline int nf_bridge_pad(const struct sk_buff *skb)
{
	int padding = 0;

	if (skb->nf_bridge && skb->protocol == htons(ETH_P_8021Q))
		padding = VLAN_HLEN;
	else if (skb->nf_bridge && skb->protocol == htons(ETH_P_PPP_SES))
		padding = PPPOE_SES_HLEN;

	return padding;
}

struct bridge_skb_cb {
	union {
		__be32 ipv4;
	} daddr;
};

#endif /* CONFIG_BRIDGE_NETFILTER */

#endif /* __KERNEL__ */
#endif
