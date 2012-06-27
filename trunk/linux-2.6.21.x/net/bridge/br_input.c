/*
 *	Handle incoming frames
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_input.c,v 1.2 2010-11-24 03:40:46 yy Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/netfilter_bridge.h>
#include "br_private.h"

#ifdef CONFIG_BRIDGE_IGMP_REPORT_NO_FLOODING
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/igmp.h>
#endif

/* Bridge group multicast address 802.1d (pg 51). */
const u8 br_group_address[ETH_ALEN] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x00 };

static int br_pass_frame_up(struct net_bridge *br, struct sk_buff *skb)
{
	struct net_device *indev;

	br->statistics.rx_packets++;
	br->statistics.rx_bytes += skb->len;

	indev = skb->dev;
	skb->dev = br->dev;

	return NF_HOOK(PF_BRIDGE, NF_BR_LOCAL_IN, skb, indev, NULL,
		netif_receive_skb);
}

/* note: already called with rcu_read_lock (preempt_disabled) */
int br_handle_frame_finish(struct sk_buff *skb)
{
	const unsigned char *dest = eth_hdr(skb)->h_dest;
	struct net_bridge_port *p = rcu_dereference(skb->dev->br_port);
	struct net_bridge *br;
	struct net_bridge_fdb_entry *dst;
        struct sk_buff *skb2;
#ifdef CONFIG_BRIDGE_IGMP_REPORT_NO_FLOODING
	struct ethhdr *eth;
	struct iphdr *ih_br;
	struct igmphdr *igmph;
#endif

	if (!p || p->state == BR_STATE_DISABLED)
		goto drop;

	/* insert into forwarding database after filtering to avoid spoofing */
	br = p->br;
	br_fdb_update(br, p, eth_hdr(skb)->h_source);

#ifdef CONFIG_BRIDGE_EAP
	if ((p->state == BR_STATE_LEARNING) && (skb->protocol != htons(ETH_P_PAE)))
#else
	if (p->state == BR_STATE_LEARNING)
#endif
		goto drop;

	/* The packet skb2 goes to the local host (NULL to skip). */
	skb2 = NULL;
	dst  = NULL;

	if (br->dev->flags & IFF_PROMISC)
		skb2 = skb;

#ifdef CONFIG_BRIDGE_EAP
	if (skb->protocol == htons(ETH_P_PAE)) {
		skb2 = skb;
		/* Do not forward 802.1x/EAP frames */
		skb = NULL;
	} else
#endif
	if (is_multicast_ether_addr(dest)) {
#ifdef CONFIG_BRIDGE_IGMP_REPORT_NO_FLOODING
		if (dest[0] != 0x01 || dest[1] != 0x00 || dest[2] != 0x5e || (dest[3] > 0x7f))
			goto no_igmp;

		eth = (struct ethhdr *)eth_hdr(skb);

		if (eth->h_proto == htons(ETH_P_IP)) {
			if (skb->len < (sizeof(struct iphdr) + sizeof(struct igmphdr)))
				goto no_igmp;

			ih_br = (struct iphdr *)skb->h.raw;
			if (ih_br->protocol != IPPROTO_IGMP)
				goto no_igmp;

			igmph = (struct igmphdr *)((unsigned char *)skb->h.raw + (ih_br->ihl<<2));
			if (igmph->type == IGMP_HOST_MEMBERSHIP_REPORT ||
			    igmph->type == IGMPV2_HOST_MEMBERSHIP_REPORT ||
			    igmph->type == IGMPV3_HOST_MEMBERSHIP_REPORT) {
			    if (skb)
				return br_pass_frame_up(br, skb);
			}
		}
no_igmp:
#endif
		br->statistics.multicast++;
		skb2 = skb;
#ifndef CONFIG_BRIDGE_FORWARD_CTRL
	} else if ((dst = __br_fdb_get(br, dest)) && dst->is_local) {
#else
	/* if set disable bridge forward flag process external packets as local */
	} else if ((dst = __br_fdb_get(br, dest)) && (dst->is_local || (atomic_read(&br->br_forward) == 0))) {
		/* if packet not local dst need full drop procedure */
		if (!dst->is_local) {
		    kfree_skb(skb);
		    br_fdb_put(dst);
		    goto out;
		}
#endif
		skb2 = skb;
		/* Do not forward the packet since it's local. */
		skb = NULL;
	}

	if (skb2 == skb)
	    skb2 = skb_clone(skb, GFP_ATOMIC);

	if (skb) {
		if (dst)
			br_forward(dst->dst, skb);
		else
			br_flood_forward(br, skb);
	}

	if (skb2)
	    return br_pass_frame_up(br, skb2);

out:
	return 0;
drop:
	kfree_skb(skb);
	goto out;
}

/* note: already called with rcu_read_lock (preempt_disabled) */
static int br_handle_local_finish(struct sk_buff *skb)
{
	struct net_bridge_port *p = rcu_dereference(skb->dev->br_port);

	if (p)
		br_fdb_update(p->br, p, eth_hdr(skb)->h_source);
	return 0;	 /* process further */
}

/* Does address match the link local multicast address.
 * 01:80:c2:00:00:0X
 */
static inline int is_link_local(const unsigned char *dest)
{
        __be16 *a = (__be16 *)dest;
        static const __be16 *b = (const __be16 *)br_group_address;
        static const __be16 m = __constant_cpu_to_be16(0xfff0);

        return ((a[0] ^ b[0]) | (a[1] ^ b[1]) | ((a[2] ^ b[2]) & m)) == 0;
}

/*
 * Called via br_handle_frame_hook.
 * Return NULL if skb is handled
 * note: already called with rcu_read_lock (preempt_disabled)
 */
struct sk_buff *br_handle_frame(struct net_bridge_port *p, struct sk_buff *skb)
{
    const unsigned char *dest = eth_hdr(skb)->h_dest;

    if (unlikely(skb->pkt_type == PACKET_LOOPBACK))
	return skb;

    if (!is_valid_ether_addr(eth_hdr(skb)->h_source))
	goto drop;

    skb = skb_share_check(skb, GFP_ATOMIC);
    if (!skb)
	return NULL;

    if (unlikely(is_link_local(dest))) {
	/*
	 * See IEEE 802.1D Table 7-10 Reserved addresses
	 *
	 * Assignment		 		Value
	 * Bridge Group Address		01-80-C2-00-00-00
	 * (MAC Control) 802.3		01-80-C2-00-00-01
	 * (Link Aggregation) 802.3	01-80-C2-00-00-02
	 * 802.1X PAE address		01-80-C2-00-00-03
	 *
	 * 802.1AB LLDP 		01-80-C2-00-00-0E
	 *
	 * Others reserved for future standardization
	 */
	switch (dest[5]) {
	case 0x00:	/* Bridge Group Address */
		/* If STP is turned off,
		   then must forward to keep loop detection */
		if (!p->br->stp_enabled)
			goto forward;
		break;

	case 0x01:	/* IEEE MAC (Pause) */
		goto drop;

	default:
		/* Allow selective forwarding for most other protocols */
		if (p->br->group_fwd_mask & (1u << dest[5]))
			goto forward;
	}

	/* Deliver packet to local host only */
	if (NF_HOOK(PF_BRIDGE, NF_BR_LOCAL_IN, skb, skb->dev,
			    NULL, br_handle_local_finish))
			return NULL;	/* frame consumed by filter */
		else
			return skb;	/* continue processing */
    }

forward:
    switch (p->state) {
    case BR_STATE_FORWARDING:

	if (br_should_route_hook) {
	    if (br_should_route_hook(&skb))
		return skb;
	    dest = eth_hdr(skb)->h_dest;
	}
	/* fall through */
    case BR_STATE_LEARNING:
	if (!compare_ether_addr(p->br->dev->dev_addr, dest))
	    skb->pkt_type = PACKET_HOST;

	NF_HOOK(PF_BRIDGE, NF_BR_PRE_ROUTING, skb, skb->dev, NULL,
	    br_handle_frame_finish);
	break;
    default:
drop:
	kfree_skb(skb);
    }
    return NULL;
}
