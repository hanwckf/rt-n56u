/*
 *	Forwarding decision
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_forward.c,v 1.1.1.1 2007-05-25 06:50:00 bruce Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/ip.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <linux/netfilter_bridge.h>
#include "br_private.h"

/* Don't forward packets to originating port or forwarding diasabled */
static inline int should_deliver(struct net_bridge_port *p, 
				 const struct sk_buff *skb)
{
 	if ((skb->dev == p->dev) || (p->state != BR_STATE_FORWARDING))
 		return 0;

	return 1;
}

static inline unsigned packet_length(const struct sk_buff *skb)
{
	return skb->len - (skb->protocol == htons(ETH_P_8021Q) ? VLAN_HLEN : 0);
}

/*
 * When forwarding bridge frames, we save a copy of the original
 * header before processing.
 */
#ifdef CONFIG_BRIDGE_NETFILTER
static inline int nf_bridge_copy_header(struct sk_buff *skb)
{
	int err;
	int header_size = ETH_HLEN;

	if (skb->protocol == htons(ETH_P_8021Q))
		header_size += VLAN_HLEN;

	err = skb_cow(skb, header_size);
	if (err)
		return err;

	memcpy(skb->data - header_size, skb->nf_bridge->data, header_size);

	if (skb->protocol == htons(ETH_P_8021Q))
		__skb_push(skb, VLAN_HLEN);
	return 0;
}

static inline int nf_bridge_maybe_copy_header(struct sk_buff *skb)
{
	if (skb->nf_bridge &&
            skb->nf_bridge->mask & (BRNF_BRIDGED | BRNF_BRIDGED_DNAT))
		return nf_bridge_copy_header(skb);
  	return 0;
}
#endif

inline int br_dev_queue_push_xmit(struct sk_buff *skb)
{
	/* drop mtu oversized packets except gso */
#ifdef CONFIG_W7_LOGO
	/* for W7/Vista Logo Test (DTM) */
	/* in case it drop the packet whose lenth is >= 1504 if packet is tagged */
	if (packet_length(skb) > (skb->dev->mtu + ((skb->protocol == ETH_P_8021Q) ? 4 : 0)) && !skb_is_gso(skb))
#else
	if (packet_length(skb) > skb->dev->mtu && !skb_is_gso(skb))
#endif
		kfree_skb(skb);
	else {
#ifdef CONFIG_BRIDGE_NETFILTER
		/* ip_refrag calls ip_fragment, doesn't copy the MAC header. */
		if (nf_bridge_maybe_copy_header(skb))
			kfree_skb(skb);
		else {
#endif
			skb_push(skb, ETH_HLEN);
			dev_queue_xmit(skb);
#ifdef CONFIG_BRIDGE_NETFILTER
		}
#endif
	}

	return 0;
}

int br_forward_finish(struct sk_buff *skb)
{
	return NF_HOOK(PF_BRIDGE, NF_BR_POST_ROUTING, skb, NULL, skb->dev,
		       br_dev_queue_push_xmit);

}

static void __br_deliver(const struct net_bridge_port *to, struct sk_buff *skb)
{
	skb->dev = to->dev;
	NF_HOOK(PF_BRIDGE, NF_BR_LOCAL_OUT, skb, NULL, skb->dev,
			br_forward_finish);
}

static void __br_forward(const struct net_bridge_port *to, struct sk_buff *skb)
{
	struct net_device *indev;

	indev = skb->dev;
	skb->dev = to->dev;
	skb->ip_summed = CHECKSUM_NONE;

	NF_HOOK(PF_BRIDGE, NF_BR_FORWARD, skb, indev, skb->dev,
			br_forward_finish);
}

/* called with rcu_read_lock */
void br_deliver(struct net_bridge_port *to, struct sk_buff *skb)
{
	if (should_deliver(to, skb)) {
		__br_deliver(to, skb);
		return;
	}

	kfree_skb(skb);
}

/* called with rcu_read_lock */
void br_forward(struct net_bridge_port *to, struct sk_buff *skb)
{
	if (should_deliver(to, skb)) {
		__br_forward(to, skb);
		return;
	}

	kfree_skb(skb);
}

/* called under bridge lock */
static void br_flood(struct net_bridge *br, struct sk_buff *skb,
	void (*__packet_hook)(const struct net_bridge_port *p,
			      struct sk_buff *skb))
{
	struct net_bridge_port *p;
	struct net_bridge_port *prev;

	prev = NULL;

	list_for_each_entry_rcu(p, &br->port_list, list) {
		if (should_deliver(p, skb)) {
			if (prev != NULL) {
				struct sk_buff *skb2;

				if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
					br->statistics.tx_dropped++;
					kfree_skb(skb);
					return;
				}

				__packet_hook(prev, skb2);
			}

			prev = p;
		}
	}

	if (prev != NULL) {
		__packet_hook(prev, skb);
		return;
	}

	kfree_skb(skb);
}


/* called with rcu_read_lock */
void br_flood_deliver(struct net_bridge *br, struct sk_buff *skb)
{
	br_flood(br, skb, __br_deliver);
}

/* called under bridge lock */
void br_flood_forward(struct net_bridge *br, struct sk_buff *skb)
{
	br_flood(br, skb, __br_forward);
}
