/*
  Module Name:
  ra_nat.c

  Abstract:

  Revision History:
  Who         When            What
  --------    ----------      ----------------------------------------------
  Name        Date            Modification logs
  Steven Liu  2011-11-11      Support RT6352 Cache mechanism
  Steven Liu  2011-06-01      Support RT6352
  Steven Liu  2011-04-11      Support RT6855A
  Steven Liu  2011-02-08      Support IPv6 over PPPoE
  Steven Liu  2010-11-25      Fix double VLAN + PPPoE header bug
  Steven Liu  2010-11-24      Support upstream/downstream/bi-direction acceleration
  Steven Liu  2010-11-17      Support Linux 2.6.36 kernel
  Steven Liu  2010-07-13      Support DSCP to User Priority helper
  Steven Liu  2010-06-03      Support skb headroom/tailroom/cb to keep HNAT information
  Kurtis Ke   2010-03-30      Support HNAT parameter can be changed by application
  Steven Liu  2010-04-08      Support RT3883 + RT309x concurrent AP
  Steven Liu  2010-03-01      Support RT3352
  Steven Liu  2009-11-26      Support WiFi pseudo interface by using VLAN tag
  Steven Liu  2009-07-21      Support IPV6 Forwarding
  Steven Liu  2009-04-02      Support RT3883/RT3350
  Steven Liu  2008-03-19      Support RT3052
  Steven Liu  2007-09-25      Support RT2880 MP
  Steven Liu  2006-10-06      Initial version
 *
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/if_vlan.h>
#include <net/ipv6.h>
#include <net/ip.h>
#include <linux/if_pppox.h>
#include <linux/ppp_defs.h>
#include <linux/pci.h>

#include "ra_nat.h"
#include "foe_fdb.h"
#include "frame_engine.h"
#include "sys_rfrw.h"
#include "policy.h"
#include "util.h"

#if !defined (CONFIG_HNAT_V2)
#include "acl_ioctl.h"
#include "ac_ioctl.h"
#include "acl_policy.h"
#include "mtr_policy.h"
#include "ac_policy.h"
#endif

#if defined (CONFIG_RA_HW_NAT_WIFI) || defined (CONFIG_RA_HW_NAT_PCI)
static int wifi_offload __read_mostly = 0;
module_param(wifi_offload, bool, S_IRUGO);
MODULE_PARM_DESC(wifi_offload, "Enable/Disable wifi/external if PPE NAT Offload.");
#endif

#define LAN_PORT_VLAN_ID	CONFIG_RA_HW_NAT_LAN_VLANID
#define WAN_PORT_VLAN_ID	CONFIG_RA_HW_NAT_WAN_VLANID

extern int (*ra_sw_nat_hook_rx) (struct sk_buff * skb);
extern int (*ra_sw_nat_hook_tx) (struct sk_buff * skb, int gmac_no);
extern void (*ra_sw_nat_hook_rs) (uint32_t Ebl);

extern uint8_t		bind_dir;
extern uint32_t		DebugLevel;

struct FoeEntry		*PpeFoeBase;
dma_addr_t		PpePhyFoeBase;
struct net_device	*DstPort[MAX_IF_NUM] = {NULL};
PktParseResult		PpeParseResult;

#if 0
void skb_dump(struct sk_buff* sk) {
        unsigned int i;

        printk("\nskb_dump: from %s with len %d (%d) headroom=%d tailroom=%d\n",
                sk->dev?sk->dev->name:"ip stack",sk->len,sk->truesize,
                skb_headroom(sk),skb_tailroom(sk));

        for(i=(unsigned int)sk->head;i<(unsigned int)sk->tail;i++) {
                if((i % 16) == 0)
		    printk("\n");

                if(i==(unsigned int)sk->head) printk("@h");
                if(i==(unsigned int)sk->data) printk("@d");
                if(i==(unsigned int)LAYER2_HEADER(sk)) printk("*");
                printk("%02X-",*((unsigned char*)i));
        }
        printk("\n");
}
#endif

static void FoeAllocTbl(uint32_t NumOfEntry)
{
	uint32_t FoeTblSize;

	FoeTblSize = NumOfEntry * sizeof(struct FoeEntry);

	PpeFoeBase = dma_alloc_coherent(NULL, FoeTblSize, &PpePhyFoeBase, GFP_KERNEL);

	RegWrite(PPE_FOE_BASE, PpePhyFoeBase);
	memset(PpeFoeBase, 0, FoeTblSize);
}

#ifdef HWNAT_DEBUG
static uint8_t *ShowCpuReason(struct sk_buff *skb)
{
	static uint8_t Buf[32];

	switch (FOE_AI(skb)) {
#if defined (CONFIG_HNAT_V2)
	case TTL_0:
		return ("IPv4(IPv6) TTL(hop limit)\n");
	case HAS_OPTION_HEADER:
		return ("Ipv4(IPv6) has option(extension) header\n");
	case NO_FLOW_IS_ASSIGNED:
		return ("No flow is assigned\n");
	case IPV4_WITH_FRAGMENT:
		return ("IPv4 HNAT doesn't support IPv4 /w fragment\n");
	case IPV4_HNAPT_DSLITE_WITH_FRAGMENT:
		return ("IPv4 HNAPT/DS-Lite doesn't support IPv4 /w fragment\n");
	case IPV4_HNAPT_DSLITE_WITHOUT_TCP_UDP:
		return ("IPv4 HNAPT/DS-Lite can't find TCP/UDP sport/dport\n");
	case IPV6_5T_6RD_WITHOUT_TCP_UDP:
		return ("IPv6 5T-route/6RD can't find TCP/UDP sport/dport\n");
	case TCP_FIN_SYN_RST:
		return ("Ingress packet is TCP fin/syn/rst\n");
	case UN_HIT:
		return ("FOE Un-hit\n");
	case HIT_UNBIND:
		return ("FOE Hit unbind\n");
	case HIT_UNBIND_RATE_REACH:
		return ("FOE Hit unbind & rate reach\n");
	case HIT_BIND_TCP_FIN:
		return ("Hit bind PPE TCP FIN entry\n");
	case HIT_BIND_TTL_1:
		return ("Hit bind PPE entry and TTL(hop limit) = 1 and TTL(hot limit) - 1\n");
	case HIT_BIND_WITH_VLAN_VIOLATION:
		return ("Hit bind and VLAN replacement violation\n");
	case HIT_BIND_KEEPALIVE_UC_OLD_HDR:
		return ("Hit bind and keep alive with unicast old-header packet\n");
	case HIT_BIND_KEEPALIVE_MC_NEW_HDR:
		return ("Hit bind and keep alive with multicast new-header packet\n");
	case HIT_BIND_KEEPALIVE_DUP_OLD_HDR:
		return ("Hit bind and keep alive with duplicate old-header packet\n");
	case HIT_BIND_FORCE_TO_CPU:
		return ("FOE Hit bind & force to CPU\n");
	case HIT_BIND_EXCEED_MTU:
		return ("Hit bind and exceed MTU\n");
#else
	case TTL_0:		/* 0x80 */
		return ("TTL=0\n");
	case FOE_EBL_NOT_IPV4_HLEN5:	/* 0x90 */
		return ("FOE enable & not IPv4h5nf\n");
	case FOE_EBL_NOT_TCP_UDP_L4_READY:	/* 0x91 */
		return ("FOE enable & not TCP/UDP/L4_read\n");
	case TCP_SYN_FIN_RST:	/* 0x92 */
		return ("TCP SYN/FIN/RST\n");
	case UN_HIT:		/* 0x93 */
		return ("Un-hit\n");
	case HIT_UNBIND:	/* 0x94 */
		return ("Hit unbind\n");
	case HIT_UNBIND_RATE_REACH:	/* 0x95 */
		return ("Hit unbind & rate reach\n");
	case HIT_FIN:		/* 0x96 */
		return ("Hit fin\n");
	case HIT_BIND_TTL_1:	/* 0x97 */
		return ("Hit bind & ttl=1 & ttl-1\n");
	case HIT_BIND_KEEPALIVE:	/* 0x98 */
		return ("Hit bind & keep alive\n");
	case HIT_BIND_FORCE_TO_CPU:	/* 0x99 */
		return ("Hit bind & force to CPU\n");
	case ACL_FOE_TBL_ERR:	/* 0x9A */
		return ("acl link foe table error (!static & !unbind)\n");
	case ACL_TBL_TTL_1:	/* 0x9B */
		return ("acl link FOE table & TTL=1 & TTL-1\n");
	case ACL_ALERT_CPU:	/* 0x9C */
		return ("acl alert cpu\n");
	case NO_FORCE_DEST_PORT:	/* 0xA0 */
		return ("No force destination port\n");
	case ACL_FORCE_PRIORITY0:	/* 0xA8 */
		return ("ACL FORCE PRIORITY0\n");
	case ACL_FORCE_PRIORITY1:	/* 0xA9 */
		return ("ACL FORCE PRIORITY1\n");
	case ACL_FORCE_PRIORITY2:	/* 0xAA */
		return ("ACL FORCE PRIORITY2\n");
	case ACL_FORCE_PRIORITY3:	/* 0xAB */
		return ("ACL FORCE PRIORITY3\n");
	case ACL_FORCE_PRIORITY4:	/* 0xAC */
		return ("ACL FORCE PRIORITY4\n");
	case ACL_FORCE_PRIORITY5:	/* 0xAD */
		return ("ACL FORCE PRIORITY5\n");
	case ACL_FORCE_PRIORITY6:	/* 0xAE */
		return ("ACL FORCE PRIORITY6\n");
	case ACL_FORCE_PRIORITY7:	/* 0xAF */
		return ("ACL FORCE PRIORITY7\n");
	case EXCEED_MTU:	/* 0xA1 */
		return ("Exceed mtu\n");
#endif
	}

	sprintf(Buf, "CPU Reason Error - %X\n", FOE_AI(skb));
	return (Buf);
}

uint32_t FoeDumpPkt(struct sk_buff * skb)
{
	struct FoeEntry *foe_entry = &PpeFoeBase[FOE_ENTRY_NUM(skb)];

	NAT_PRINT("\nRx===<FOE_Entry=%d>=====\n", FOE_ENTRY_NUM(skb));
	NAT_PRINT("RcvIF=%s\n", skb->dev->name);
	NAT_PRINT("FOE_Entry=%d\n", FOE_ENTRY_NUM(skb));
	NAT_PRINT("CPU Reason=%s", ShowCpuReason(skb));
	NAT_PRINT("ALG=%d\n", FOE_ALG(skb));
	NAT_PRINT("SP=%d\n", FOE_SP(skb));

	/* PPE: IPv4 packet=IPV4_HNAT IPv6 packet=IPV6_ROUTE */
	if (IS_IPV4_GRP(foe_entry)) {
		NAT_PRINT("Information Block 1=%x\n", foe_entry->ipv4_hnapt.info_blk1);
		NAT_PRINT("SIP=%s\n", Ip2Str(foe_entry->ipv4_hnapt.sip));
		NAT_PRINT("DIP=%s\n", Ip2Str(foe_entry->ipv4_hnapt.dip));
		NAT_PRINT("SPORT=%d\n", foe_entry->ipv4_hnapt.sport);
		NAT_PRINT("DPORT=%d\n", foe_entry->ipv4_hnapt.dport);
		NAT_PRINT("Information Block 2=%x\n",
			  foe_entry->ipv4_hnapt.info_blk2);
	}
#if defined (CONFIG_HNAT_V2)
#if defined (CONFIG_RA_HW_NAT_IPV6)
	else if (IS_IPV6_GRP(foe_entry)) {
		NAT_PRINT("Information Block 1=%x\n", foe_entry->ipv6_5t_route.info_blk1);
		NAT_PRINT("IPv6_SIP=%08X:%08X:%08X:%08X\n",
			  foe_entry->ipv6_5t_route.ipv6_sip0,
			  foe_entry->ipv6_5t_route.ipv6_sip1,
			  foe_entry->ipv6_5t_route.ipv6_sip2,
			  foe_entry->ipv6_5t_route.ipv6_sip3);
		NAT_PRINT("IPv6_DIP=%08X:%08X:%08X:%08X\n",
			  foe_entry->ipv6_5t_route.ipv6_dip0,
			  foe_entry->ipv6_5t_route.ipv6_dip1,
			  foe_entry->ipv6_5t_route.ipv6_dip2,
			  foe_entry->ipv6_5t_route.ipv6_dip3);
		if(IS_IPV6_FLAB_EBL()) {
			NAT_PRINT("Flow Label=%08X\n", (foe_entry->ipv6_5t_route.sport << 16) | 
							(foe_entry->ipv6_5t_route.dport));
		} else {
			NAT_PRINT("SPORT=%d\n", foe_entry->ipv6_5t_route.sport);
			NAT_PRINT("DPORT=%d\n", foe_entry->ipv6_5t_route.dport);
		}
		NAT_PRINT("Information Block 2=%x\n",
			  foe_entry->ipv6_5t_route.info_blk2);
	}
#endif
#endif
	else {
		NAT_PRINT("unknown Pkt_type=%d\n", foe_entry->bfib1.pkt_type);
	}

	NAT_PRINT("==================================\n");

	return 1;

}
#endif

#if defined  (CONFIG_RA_HW_NAT_WIFI) || defined (CONFIG_RA_HW_NAT_PCI)
inline void RemoveVlanTag(struct sk_buff *skb)
{
	struct ethhdr *eth;

	/* remove VLAN tag */
	skb->data = LAYER2_HEADER(skb);
	LAYER2_HEADER(skb) += VLAN_HLEN;
	memmove(LAYER2_HEADER(skb), skb->data, ETH_ALEN * 2);

	skb_pull(skb, VLAN_HLEN);
	skb->data += ETH_HLEN;	/* pointer to layer3 header */
	eth = (struct ethhdr *)LAYER2_HEADER(skb);
	skb->protocol = eth->h_proto;
}

/* push different VID for WiFi pseudo interface or USB external NIC */
uint32_t PpeExtIfRxHandler(struct sk_buff * skb)
{
	uint16_t VirIfIdx = 0;

	/* check dst interface exist */
	if (skb->dev == NULL) {
	    NAT_PRINT("HNAT: RX: interface not exist drop this packet.\n");
	    kfree_skb(skb);
	    return 0;
	}

	if (skb->dev == DstPort[DP_RA0]) {
		VirIfIdx = DP_RA0;
	}
#if defined (CONFIG_RT2860V2_AP_MBSS)
	else if (skb->dev == DstPort[DP_RA1]) {
		VirIfIdx = DP_RA1;
	}
#endif // CONFIG_RT2860V2_AP_MBSS //
#if defined (CONFIG_RT2860V2_AP_WDS)
	else if (skb->dev == DstPort[DP_WDS0]) {
		VirIfIdx = DP_WDS0;
	} else if (skb->dev == DstPort[DP_WDS1]) {
		VirIfIdx = DP_WDS1;
	} else if (skb->dev == DstPort[DP_WDS2]) {
		VirIfIdx = DP_WDS2;
	} else if (skb->dev == DstPort[DP_WDS3]) {
		VirIfIdx = DP_WDS3;
	}
#endif
#if defined (CONFIG_RT2860V2_AP_APCLI)
	else if (skb->dev == DstPort[DP_APCLI0]) {
		VirIfIdx = DP_APCLI0;
	}
#endif // CONFIG_RT2860V2_AP_APCLI //
#if defined (CONFIG_RT2860V2_AP_MESH)
	else if (skb->dev == DstPort[DP_MESH0]) {
		VirIfIdx = DP_MESH0;
	}
#endif // CONFIG_RT2860V2_AP_MESH //
#if defined (CONFIG_RTDEV_MII) || defined (CONFIG_RTDEV_USB) || \
    defined (CONFIG_RTDEV_PCI) || defined (CONFIG_RTDEV)
	else if (skb->dev == DstPort[DP_RAI0]) {
		VirIfIdx = DP_RAI0;
	}
#if defined (CONFIG_RT3090_AP_MBSS) || defined (CONFIG_RT5392_AP_MBSS) || \
    defined (CONFIG_RT3572_AP_MBSS) || defined (CONFIG_RT5572_AP_MBSS) || \
    defined (CONFIG_RT5592_AP_MBSS) || defined (CONFIG_RT3593_AP_MBSS)
	else if (skb->dev == DstPort[DP_RAI1]) {
		VirIfIdx = DP_RAI1;
	}
#endif // CONFIG_RTDEV_AP_MBSS //
#endif // CONFIG_RTDEV_MII || CONFIG_RTDEV_USB || CONFIG_RTDEV_PCI
#if defined (CONFIG_RT3090_AP_APCLI) || defined (CONFIG_RT5392_AP_APCLI) || \
    defined (CONFIG_RT3572_AP_APCLI) || defined (CONFIG_RT5572_AP_APCLI) || \
    defined (CONFIG_RT5592_AP_APCLI) || defined (CONFIG_RT3593_AP_APCLI)
	else if (skb->dev == DstPort[DP_APCLII0]) {
		VirIfIdx = DP_APCLII0;
	}
#endif // CONFIG_RTDEV_AP_APCLI //
#if defined (CONFIG_RT3090_AP_MESH) || defined (CONFIG_RT5392_AP_MESH) || \
    defined (CONFIG_RT3572_AP_MESH) || defined (CONFIG_RT5572_AP_MESH) || \
    defined (CONFIG_RT5592_AP_MESH) || defined (CONFIG_RT3593_AP_MESH)
	else if (skb->dev == DstPort[DP_MESHI0]) {
		VirIfIdx = DP_MESHI0;
	}
#endif // CONFIG_RTDEV_AP_MESH //
#if defined (CONFIG_RA_HW_NAT_PCI)
	else if (skb->dev == DstPort[DP_PCI]) {
		VirIfIdx = DP_PCI;
	}
#endif
	else {
#ifdef HWNAT_DEBUG
		NAT_PRINT("HNAT: The interface %s is unknown\n", skb->dev->name);
#endif
		return 1;
	}

	/* make skb writable */
	if (!skb_make_writable(skb, 0)) {
		NAT_PRINT("HNAT: no mem or corrupted packet for add tag? (VirIfIdx=%d)\n", VirIfIdx);
		return 1;
	}

	/* push vlan tag to stand for actual incoming interface,
	    so HNAT module can know the actual incoming interface from vlan id. */
	LAYER3_HEADER(skb) = skb->data;
	skb_push(skb, ETH_HLEN);	//pointer to layer2 header before calling hard_start_xmit
	skb = __vlan_put_tag(skb, VirIfIdx);
	if (!skb) {
	    NAT_PRINT("HNAT: not valid tag ? memleak ? (VirIfIdx=%d)\n", VirIfIdx);
	    return 0;
	}

	/* redirect to PPE */
	FOE_AI(skb) = UN_HIT;
	FOE_MAGIC_TAG(skb) = FOE_MAGIC_PPE;
	skb->dev = DstPort[DP_GMAC];	//we use GMAC1 to send to packet to PPE
	dev_queue_xmit(skb);
	return 0;
}

uint32_t PpeExtIfPingPongHandler(struct sk_buff * skb)
{
	struct ethhdr *eth = NULL;
	uint16_t VirIfIdx = 0;
	struct net_device *dev;
	struct vlan_ethhdr *veth;

	veth = (struct vlan_ethhdr *)LAYER2_HEADER(skb);

	VirIfIdx = ntohs(veth->h_vlan_TCI);

	/* something wrong: 1) interface index must be < MAX_IF_NUM and exist, proto must be 802.11q
				don`t touch this packets and return to normal path before corrupt in detag code
			    2) UFO packets in this case keep only in if DstPort != LAN i.e not to LAN path
	*/
	if ((VirIfIdx >= MAX_IF_NUM) || (DstPort[VirIfIdx] == NULL) || (veth->h_vlan_proto != htons(ETH_P_8021Q))) {
#ifdef HWNAT_DEBUG
		if (DebugLevel >= 1)
		    NAT_PRINT("HNAT: Reentry packet for untagged frame, transit vlan or interface (VirIfIdx=%d) not exist. Skip this packet.\n", VirIfIdx);
#endif
		return 1;
	}

	/* make skb writable */
	if (!skb_make_writable(skb, 0)) {
	    NAT_PRINT("HNAT: No mem for remove tag or corrupted packet? (VirIfIdx=%d)\n", VirIfIdx);
	    return 1;
	}

	/* remove vlan tag from current packet */
	RemoveVlanTag(skb);

	/* set correct skb dest dev */
	skb->dev = DstPort[VirIfIdx];

	eth = (struct ethhdr *)LAYER2_HEADER(skb);

	if (is_multicast_ether_addr(eth->h_dest)) {
	    if (!compare_ether_addr(eth->h_dest, skb->dev->broadcast))
		skb->pkt_type = PACKET_BROADCAST;
	    else
		skb->pkt_type = PACKET_MULTICAST;
	} else {
	    skb->pkt_type=PACKET_OTHERHOST;
	    for(VirIfIdx=0; VirIfIdx < MAX_IF_NUM; VirIfIdx++) {
		    dev = DstPort[VirIfIdx];
		    if (dev !=NULL && !compare_ether_addr(eth->h_dest, dev->dev_addr)) {
			skb->pkt_type=PACKET_HOST;
			break;
		    }
	    }
	}

	return 1;
}
#endif

uint32_t PpeKeepAliveHandler(struct sk_buff * skb, struct FoeEntry * foe_entry)
{
	struct ethhdr *eth = NULL;
	uint16_t eth_type = ntohs(skb->protocol);
	uint32_t vlan1_gap = 0;
	uint32_t vlan2_gap = 0;
	uint32_t pppoe_gap = 0;
	struct vlan_hdr *vh;
	struct iphdr *iph = NULL;
	struct tcphdr *th = NULL;
	struct udphdr *uh = NULL;

	/*
	 * try to recover to original SMAC/DMAC, but we don't have such information.
	 * just use SMAC as DMAC and set Multicast address as SMAC.
	 */
	eth = (struct ethhdr *)(skb->data - ETH_HLEN);

	FoeGetMacInfo(eth->h_dest, eth->h_source);
	FoeGetMacInfo(eth->h_source, eth->h_dest);
	eth->h_source[0] = 0x1;	//change to multicast packet, make bridge not learn this packet
	if (eth_type == ETH_P_8021Q) {
		vlan1_gap = VLAN_HLEN;
		vh = (struct vlan_hdr *)skb->data;

		if (ntohs(vh->h_vlan_TCI) == LAN_PORT_VLAN_ID) {
			/* It make packet like coming from WAN port */
			vh->h_vlan_TCI = htons(WAN_PORT_VLAN_ID);

		} else {
			/* It make packet like coming from LAN port */
			vh->h_vlan_TCI = htons(LAN_PORT_VLAN_ID);
		}

		if (ntohs(vh->h_vlan_encapsulated_proto) == ETH_P_PPP_SES) {
			pppoe_gap = 8;
		} else if (ntohs(vh->h_vlan_encapsulated_proto) == ETH_P_8021Q) {
			vlan2_gap = VLAN_HLEN;
			vh = (struct vlan_hdr *)(skb->data + VLAN_HLEN);

			/* VLAN + VLAN + PPPoE */
			if (ntohs(vh->h_vlan_encapsulated_proto) ==
			    ETH_P_PPP_SES) {
				pppoe_gap = 8;
			} else {
				/* VLAN + VLAN + IP */
				eth_type = ntohs(vh->h_vlan_encapsulated_proto);
			}
		} else {
			/* VLAN + IP */
			eth_type = ntohs(vh->h_vlan_encapsulated_proto);
		}
	}

	/* Only Ipv4 NAT need KeepAlive Packet to refresh iptable */
	if (eth_type == ETH_P_IP) {
		iph =
		    (struct iphdr *)(skb->data + vlan1_gap + vlan2_gap +
				     pppoe_gap);

		//Recover to original layer 4 header 
		if (iph->protocol == IPPROTO_TCP) {
			th = (struct tcphdr *)((uint8_t *) iph + iph->ihl * 4);
			FoeToOrgTcpHdr(foe_entry, iph, th);

		} else if (iph->protocol == IPPROTO_UDP) {
			uh = (struct udphdr *)((uint8_t *) iph + iph->ihl * 4);
			FoeToOrgUdpHdr(foe_entry, iph, uh);
		}
		//Recover to original layer 3 header 
		FoeToOrgIpHdr(foe_entry, iph);
	} else if (eth_type == ETH_P_IPV6) {
		/* Nothing to do */
	} else {
		return 1;
	}

	/*
	 * Ethernet driver will call eth_type_trans() to update skb->pkt_type.
	 * If(destination mac != my mac) 
	 *   skb->pkt_type=PACKET_OTHERHOST;
	 *
	 * In order to pass ip_rcv() check, we change pkt_type to PACKET_HOST here
	 */
	skb->pkt_type = PACKET_HOST;
	return 1;

}

int
PpeHitBindForceToCpuHandler(struct sk_buff *skb, struct FoeEntry *foe_entry)
{

	if (IS_IPV4_HNAT(foe_entry) || IS_IPV4_HNAPT(foe_entry)) {
		skb->dev = DstPort[foe_entry->ipv4_hnapt.act_dp];
	}
#if defined (CONFIG_HNAT_V2)
#if defined (CONFIG_RA_HW_NAT_IPV6)
	else if (IS_IPV4_DSLITE(foe_entry)) {
		skb->dev = DstPort[foe_entry->ipv4_dslite.act_dp];
	} else if (IS_IPV6_3T_ROUTE(foe_entry)) {
		skb->dev = DstPort[foe_entry->ipv6_3t_route.act_dp];
	} else if (IS_IPV6_5T_ROUTE(foe_entry)) {
		skb->dev = DstPort[foe_entry->ipv6_5t_route.act_dp];
	} else if (IS_IPV6_6RD(foe_entry)) {
		skb->dev = DstPort[foe_entry->ipv6_6rd.act_dp];
	} else {
		return 1;
	}
#endif
#endif

	LAYER3_HEADER(skb) = skb->data;
	skb_push(skb, ETH_HLEN);	//pointer to layer2 header
	dev_queue_xmit(skb);
	return 0;
}

#if defined (CONFIG_RA_HW_NAT_ACL2UP_HELPER)
uint32_t PpeGetUpFromACLRule(struct sk_buff *skb)
{
	struct ethhdr *eth = NULL;
	uint16_t eth_type = 0;
	uint32_t vlan1_gap = 0;
	uint32_t vlan2_gap = 0;
	uint32_t pppoe_gap = 0;
	struct vlan_hdr *vh;
	struct iphdr *iph = NULL;
	struct tcphdr *th = NULL;
	struct udphdr *uh = NULL;

	AclClassifyKey NewRateReach;
	eth = (struct ethhdr *)(skb->data - ETH_HLEN);

	memset(&NewRateReach, 0, sizeof(AclClassifyKey));
	memcpy(NewRateReach.Mac, eth->h_source, ETH_ALEN);
	NewRateReach.Ethertype = eth_type;	//Ethertype
	if (eth_type == ETH_P_8021Q) {
		vlan1_gap = VLAN_HLEN;
		vh = (struct vlan_hdr *)skb->data;
		NewRateReach.Vid = ntohs(vh->h_vlan_TCI);	//VID
		if (ntohs(vh->h_vlan_encapsulated_proto) == ETH_P_PPP_SES) {
			pppoe_gap = 8;
		} else if (ntohs(vh->h_vlan_encapsulated_proto) == ETH_P_8021Q) {
			vlan2_gap = VLAN_HLEN;
			vh = (struct vlan_hdr *)(skb->data + VLAN_HLEN);

			/* VLAN + VLAN + PPPoE */
			if (ntohs(vh->h_vlan_encapsulated_proto) ==
			    ETH_P_PPP_SES) {
				pppoe_gap = 8;
			} else {
				/* VLAN + VLAN + IP */
				eth_type = ntohs(vh->h_vlan_encapsulated_proto);
			}
		} else {
			/* VLAN + IP */
			eth_type = ntohs(vh->h_vlan_encapsulated_proto);
		}
	}

	/*IPv4 */
	if (eth_type == ETH_P_IP) {
		iph =
		    (struct iphdr *)(skb->data + vlan1_gap + vlan2_gap +
				     pppoe_gap);

		NewRateReach.Sip = ntohl(iph->saddr);
		NewRateReach.Dip = ntohl(iph->daddr);
		NewRateReach.Tos = iph->tos;	//TOS
		if (iph->protocol == IPPROTO_TCP) {
			th = (struct tcphdr *)((uint8_t *) iph + iph->ihl * 4);
			NewRateReach.Sp = ntohs(th->source);
			NewRateReach.Dp = ntohs(th->dest);
			NewRateReach.Proto = ACL_PROTO_TCP;
		} else if (iph->protocol == IPPROTO_UDP) {
			uh = (struct udphdr *)((uint8_t *) iph + iph->ihl * 4);
			NewRateReach.Sp = ntohs(uh->source);
			NewRateReach.Dp = ntohs(uh->dest);
			NewRateReach.Proto = ACL_PROTO_UDP;

		}

	}

	/*classify user priority */
	return AclClassify(&NewRateReach);

}
#endif

int32_t PpeRxHandler(struct sk_buff * skb)
{
	struct FoeEntry *foe_entry;
	uint16_t eth_type=0;
#if defined (CONFIG_RA_HW_NAT_WIFI) || defined (CONFIG_RA_HW_NAT_PCI) || defined(HWNAT_SKIP_MCAST_BCAST)
	struct ethhdr *eth = NULL;
#endif

	/* return trunclated packets to normal path */
	if (!skb || (skb->len < ETH_HLEN)) {
#ifdef HWNAT_DEBUG
	    NAT_PRINT("HNAT: skb null or small len in rx path\n");
#endif
	    return 1;
	}

	/* PPE only can handle IPv4/VLAN/IPv6/PPP packets */
	eth_type=ntohs(skb->protocol);
	if(eth_type != ETH_P_IP &&
#ifdef CONFIG_RA_HW_NAT_IPV6
	    eth_type != ETH_P_IPV6 &&
#endif
	    eth_type != ETH_P_8021Q &&
	    eth_type != ETH_P_PPP_SES &&
	    eth_type != ETH_P_PPP_DISC) {
	    return 1;
	}

#if defined(HWNAT_SKIP_MCAST_BCAST)
	/* skip bcast/mcast traffic PPE. WiFi bug ? */
	eth = (struct ethhdr *)LAYER2_HEADER(skb);
	if(is_multicast_ether_addr(eth->h_dest))
	    return 1;
#endif

#ifdef HWNAT_DEBUG
	if (DebugLevel >= 3) {
		FoeDumpPkt(skb);
	}
#endif
	foe_entry = &PpeFoeBase[FOE_ENTRY_NUM(skb)];

	/* the incoming packet is from PCI or WiFi interface */
	if (((FOE_MAGIC_TAG(skb) == FOE_MAGIC_PCI) || (FOE_MAGIC_TAG(skb) == FOE_MAGIC_WLAN))) {
#if defined (CONFIG_RA_HW_NAT_WIFI) || defined (CONFIG_RA_HW_NAT_PCI)
	    if (wifi_offload && (eth_type != ETH_P_8021Q)) {
#if !defined(HWNAT_SKIP_MCAST_BCAST)
		eth = (struct ethhdr *)LAYER2_HEADER(skb);
		if(is_multicast_ether_addr(eth->h_dest))
			return 1; /* skip wifi offload for multicast/broadcast */
#endif
		/* add tag to pkts from external ifaces before send to PPE */
		return PpeExtIfRxHandler(skb);
	    } else {
		return 1; /* wifi offload disabled */
	    }
#else
		return 1; /* wifi offload not compiled */
#endif
	} else if ((FOE_AI(skb) == HIT_BIND_FORCE_TO_CPU)) {
		return PpeHitBindForceToCpuHandler(skb, foe_entry);
	/* handle the incoming packet which came back from PPE */
#if defined (CONFIG_HNAT_V2)
	} else if ((FOE_SP(skb) == 6) &&
			(FOE_AI(skb) != HIT_BIND_KEEPALIVE_UC_OLD_HDR) && 
			(FOE_AI(skb) != HIT_BIND_KEEPALIVE_MC_NEW_HDR) && 
			(FOE_AI(skb) != HIT_BIND_KEEPALIVE_DUP_OLD_HDR)) {
#else
	} else if (FOE_SP(skb) == 0 && (FOE_AI(skb) != HIT_BIND_KEEPALIVE)) {
#endif
#if defined (CONFIG_RA_HW_NAT_WIFI) || defined (CONFIG_RA_HW_NAT_PCI)
	    if (wifi_offload && (eth_type == ETH_P_8021Q)) {
		/* detag pkts from external ifaces after PPE process */
		return PpeExtIfPingPongHandler(skb);
	    } else {
		return 1; /* wifi offload disabled */
	    }
#else
		return 1; /* wifi offload not compiled */
#endif
#if defined (CONFIG_HNAT_V2)
	} else if (FOE_AI(skb) == HIT_BIND_KEEPALIVE_UC_OLD_HDR) {
		if (DebugLevel >= 2) {
			printk("Got HIT_BIND_KEEPALIVE_UC_OLD_HDR packet (hash index=%d)\n", FOE_ENTRY_NUM(skb));
		}
		return 1;
	} else if (FOE_AI(skb) == HIT_BIND_KEEPALIVE_MC_NEW_HDR) {
		if (DebugLevel >= 2) {
			printk("Got HIT_BIND_KEEPALIVE_MC_NEW_HDR packet (hash index=%d)\n", FOE_ENTRY_NUM(skb));
		}
		if (PpeKeepAliveHandler(skb, foe_entry)) {
			return 1;
		}
	} else if (FOE_AI(skb) == HIT_BIND_KEEPALIVE_DUP_OLD_HDR) {
		if (DebugLevel >= 2) {
			printk("Got HIT_BIND_KEEPALIVE_DUP_OLD_HDR packe (hash index=%d)\n", FOE_ENTRY_NUM(skb));
		}
		return 1;
#else
	} else if ((FOE_AI(skb) == HIT_BIND_KEEPALIVE) && (DFL_FOE_KA == 0)) {
		if (PpeKeepAliveHandler(skb, foe_entry)) {
			return 1;
		}
#endif
	}
#if defined (CONFIG_RA_HW_NAT_ACL2UP_HELPER)
	else if ((FOE_AI(skb) == HIT_UNBIND_RATE_REACH)) {
		FOE_SP(skb) = PpeGetUpFromACLRule(skb);
	}
#endif

	return 1;
}

int32_t
GetPppoeSid(struct sk_buff * skb, uint32_t vlan_gap,
	    uint16_t * sid, uint16_t * ppp_tag)
{
	struct pppoe_hdr *peh = NULL;

	peh = (struct pppoe_hdr *)(skb->data + ETH_HLEN + vlan_gap);

#ifdef HWNAT_DEBUG
	if (DebugLevel >= 6) {
		NAT_PRINT("\n==============\n");
		NAT_PRINT(" Ver=%d\n", peh->ver);
		NAT_PRINT(" Type=%d\n", peh->type);
		NAT_PRINT(" Code=%d\n", peh->code);
		NAT_PRINT(" sid=%x\n", ntohs(peh->sid));
		NAT_PRINT(" Len=%d\n", ntohs(peh->length));
		NAT_PRINT(" tag_type=%x\n", ntohs(peh->tag[0].tag_type));
		NAT_PRINT(" tag_len=%d\n", ntohs(peh->tag[0].tag_len));
		NAT_PRINT("=================\n");
	}
#endif
	*ppp_tag = peh->tag[0].tag_type;
#if defined (CONFIG_RA_HW_NAT_IPV6)
	if (peh->ver != 1 || peh->type != 1
	    || (*ppp_tag != htons(PPP_IP) && *ppp_tag != htons(PPP_IPV6))) {
#else
	if (peh->ver != 1 || peh->type != 1 || *ppp_tag != htons(PPP_IP)) {
#endif
		return 1;
	}

	*sid = peh->sid;
	return 0;
}

/* HNAT_V2 can push special tag */
int32_t isSpecialTag(uint16_t eth_type)
{
	/* Please modify this function to speed up the packet with special tag
	 * Ex: 
	 *    Ralink switch = 0x81xx
	 *    Realtek switch = 0x8899 
	 */
#if defined (CONFIG_HNAT_V2)
	if ((eth_type && 0x00FF) == htons(ETH_P_8021Q)) { //Ralink Special Tag: 0x81xx
		PpeParseResult.vlan_tag = eth_type;
		return 1;
	} else {
		return 0;
	}
#else
	return 0;
#endif
}

int32_t is8021Q(uint16_t eth_type)
{
	if (eth_type == htons(ETH_P_8021Q)) {
		PpeParseResult.vlan_tag = eth_type;
		return 1;
	} else {
		return 0;
	}
}


int32_t PpeParseLayerInfo(struct sk_buff * skb)
{

	struct vlan_hdr *vh = NULL;
	struct ethhdr *eth = NULL;
	struct iphdr *iph = NULL;
	struct ipv6hdr *ip6h = NULL;
	struct tcphdr *th = NULL;
	struct udphdr *uh = NULL;
	
	memset(&PpeParseResult, 0, sizeof(PpeParseResult));

	eth = (struct ethhdr *)skb->data;
	memcpy(PpeParseResult.dmac, eth->h_dest, ETH_ALEN);
	memcpy(PpeParseResult.smac, eth->h_source, ETH_ALEN);
	PpeParseResult.eth_type = eth->h_proto;

	/* PPPoE (RT3883 with 2xGMAC) */
#if defined (CONFIG_RAETH_GMAC2)
	if (PpeParseResult.eth_type == htons(ETH_P_PPP_SES))
	{
		PpeParseResult.pppoe_gap = 8;
		if (GetPppoeSid(skb, 0,
				&PpeParseResult.pppoe_sid,
				&PpeParseResult.ppp_tag)) {
			return 1;
		}
	}
	else
#endif
	if (is8021Q(PpeParseResult.eth_type) || isSpecialTag(PpeParseResult.eth_type)) {
		PpeParseResult.vlan1_gap = VLAN_HLEN;
		PpeParseResult.vlan_layer++;
		vh = (struct vlan_hdr *)(skb->data + ETH_HLEN);
		PpeParseResult.vlan1 = vh->h_vlan_TCI;

		/* VLAN + PPPoE */
		if (ntohs(vh->h_vlan_encapsulated_proto) == ETH_P_PPP_SES) {
			PpeParseResult.pppoe_gap = 8;
			if (GetPppoeSid(skb, PpeParseResult.vlan1_gap,
					&PpeParseResult.pppoe_sid,
					&PpeParseResult.ppp_tag)) {
				return 1;
			}
			PpeParseResult.eth_type = vh->h_vlan_encapsulated_proto;
			/* Double VLAN = VLAN + VLAN */
		} else if ( is8021Q(vh->h_vlan_encapsulated_proto) || 
			isSpecialTag(vh->h_vlan_encapsulated_proto)) {
			PpeParseResult.vlan2_gap = VLAN_HLEN;
			PpeParseResult.vlan_layer++;
			vh = (struct vlan_hdr *)(skb->data + ETH_HLEN + VLAN_HLEN);
			PpeParseResult.vlan2 = vh->h_vlan_TCI;

			/* VLAN + VLAN + PPPoE */
			if (ntohs(vh->h_vlan_encapsulated_proto) == ETH_P_PPP_SES) {
				PpeParseResult.pppoe_gap = 8;
				if (GetPppoeSid
				    (skb,
				     (PpeParseResult.vlan1_gap + PpeParseResult.vlan2_gap),
				     &PpeParseResult.pppoe_sid, &PpeParseResult.ppp_tag)) {
					return 1;
				}
				PpeParseResult.eth_type = vh->h_vlan_encapsulated_proto;
#if defined (CONFIG_HNAT_V2)
			} else if (is8021Q(vh->h_vlan_encapsulated_proto)) {
				/* VLAN + VLAN + VLAN */
				PpeParseResult.vlan_layer++;
				vh = (struct vlan_hdr *)(skb->data + ETH_HLEN + VLAN_HLEN + VLAN_HLEN);

				/* VLAN + VLAN + VLAN */
				if (is8021Q(vh->h_vlan_encapsulated_proto)) {
					PpeParseResult.vlan_layer++;
				}
#endif
			} else {
				/* VLAN + VLAN + IP */
				PpeParseResult.eth_type = vh->h_vlan_encapsulated_proto;
			}
		} else {
			/* VLAN + IP */
			PpeParseResult.eth_type = vh->h_vlan_encapsulated_proto;
		}
	}

	/* set layer2 start addr */
	LAYER2_HEADER(skb) = skb->data;

	/* set layer3 start addr */
	LAYER3_HEADER(skb) =
	    (skb->data + ETH_HLEN + PpeParseResult.vlan1_gap +
	     PpeParseResult.vlan2_gap + PpeParseResult.pppoe_gap);

	/* set layer4 start addr */
	if ((PpeParseResult.eth_type == htons(ETH_P_IP)) || (PpeParseResult.eth_type == htons(ETH_P_PPP_SES)
		&& PpeParseResult.ppp_tag == htons(PPP_IP))) {
		iph = (struct iphdr *)LAYER3_HEADER(skb);

		//prepare layer3/layer4 info
		memcpy(&PpeParseResult.iph, iph, sizeof(struct iphdr));
		if (iph->protocol == IPPROTO_TCP) {
			LAYER4_HEADER(skb) = ((uint8_t *) iph + (iph->ihl * 4));
			th = (struct tcphdr *)LAYER4_HEADER(skb);
			memcpy(&PpeParseResult.th, th, sizeof(struct tcphdr));
			PpeParseResult.pkt_type = IPV4_HNAPT;

			//More fragment bit = 1 (IP fragment packet with TCP header)
			if(ntohs(iph->frag_off) & IP_MF) {
				return 1;
			}
		} else if (iph->protocol == IPPROTO_UDP) {
			LAYER4_HEADER(skb) = ((uint8_t *) iph + iph->ihl * 4);
			uh = (struct udphdr *)LAYER4_HEADER(skb);
			memcpy(&PpeParseResult.uh, uh, sizeof(struct udphdr));
			PpeParseResult.pkt_type = IPV4_HNAPT;
			
			//More fragment bit = 1 (IP fragment packet with UDP header)
			if(ntohs(iph->frag_off) & IP_MF) {
				return 1;
			}
		}
#if defined (CONFIG_HNAT_V2)
		else if (iph->protocol == IPPROTO_GRE) {
			/* do nothing */
		}
#if defined (CONFIG_RA_HW_NAT_IPV6)
		else if (iph->protocol == IPPROTO_IPV6) {
			ip6h = (struct ipv6hdr *)((uint8_t *) iph + iph->ihl * 4);
			memcpy(&PpeParseResult.ip6h, ip6h, sizeof(struct ipv6hdr));

			if (ip6h->nexthdr == NEXTHDR_TCP) {
				LAYER4_HEADER(skb) = ((uint8_t *) ip6h + sizeof(struct ipv6hdr));
				th = (struct tcphdr *)LAYER4_HEADER(skb);
				memcpy(&PpeParseResult.th.source, &th->source, sizeof(th->source));
				memcpy(&PpeParseResult.th.dest, &th->dest, sizeof(th->dest));
			} else if (ip6h->nexthdr == NEXTHDR_UDP) {
				LAYER4_HEADER(skb) = ((uint8_t *) ip6h + sizeof(struct ipv6hdr));
				uh = (struct udphdr *)LAYER4_HEADER(skb);
				memcpy(&PpeParseResult.uh.source, &uh->source, sizeof(uh->source));
				memcpy(&PpeParseResult.uh.dest, &uh->dest, sizeof(uh->dest));
			}
			PpeParseResult.pkt_type = IPV6_6RD;
		}
#endif
#endif
		else {
			/* Packet format is not supported */
			return 1;
		}

	} else if (PpeParseResult.eth_type == htons(ETH_P_IPV6) || 
			(PpeParseResult.eth_type == htons(ETH_P_PPP_SES) &&
		        PpeParseResult.ppp_tag == htons(PPP_IPV6))) {
		ip6h = (struct ipv6hdr *)LAYER3_HEADER(skb);
		memcpy(&PpeParseResult.ip6h, ip6h, sizeof(struct ipv6hdr));

		if (ip6h->nexthdr == NEXTHDR_TCP) {
			LAYER4_HEADER(skb) = ((uint8_t *) ip6h + sizeof(struct ipv6hdr));
			th = (struct tcphdr *)LAYER4_HEADER(skb);
			memcpy(&PpeParseResult.th, th, sizeof(struct tcphdr));
			PpeParseResult.pkt_type = IPV6_5T_ROUTE;
		} else if (ip6h->nexthdr == NEXTHDR_UDP) {
			LAYER4_HEADER(skb) = ((uint8_t *) ip6h + sizeof(struct ipv6hdr));
			uh = (struct udphdr *)LAYER4_HEADER(skb);
			memcpy(&PpeParseResult.uh, uh, sizeof(struct udphdr));
			PpeParseResult.pkt_type = IPV6_5T_ROUTE;
		}
		else if (ip6h->nexthdr == NEXTHDR_IPIP) {
			memcpy(&PpeParseResult.iph, ip6h + sizeof(struct ipv6hdr),
			       sizeof(struct iphdr));
			PpeParseResult.pkt_type = IPV4_DSLITE;
		} else {
#if defined (CONFIG_HNAT_V2)
			PpeParseResult.pkt_type = IPV6_3T_ROUTE;
#else
			PpeParseResult.pkt_type = IPV6_1T_ROUTE;
#endif
		}

	} else {
		return 1;
	}

#ifdef HWNAT_DEBUG
	if (DebugLevel >= 6) {
		printk("--------------\n");
		printk("DMAC:%02X:%02X:%02X:%02X:%02X:%02X\n",
		       PpeParseResult.dmac[0], PpeParseResult.dmac[1],
		       PpeParseResult.dmac[2], PpeParseResult.dmac[3],
		       PpeParseResult.dmac[4], PpeParseResult.dmac[5]);
		printk("SMAC:%02X:%02X:%02X:%02X:%02X:%02X\n",
		       PpeParseResult.smac[0], PpeParseResult.smac[1],
		       PpeParseResult.smac[2], PpeParseResult.smac[3],
		       PpeParseResult.smac[4], PpeParseResult.smac[5]);
		printk("Eth_Type=%x\n", PpeParseResult.eth_type);
		if (PpeParseResult.vlan1_gap > 0) {
			printk("VLAN1 ID=%x\n", ntohs(PpeParseResult.vlan1));
		}

		if (PpeParseResult.vlan2_gap > 0) {
			printk("VLAN2 ID=%x\n", ntohs(PpeParseResult.vlan2));
		}

		if (PpeParseResult.pppoe_gap > 0) {
			printk("PPPOE Session ID=%x\n",
			       PpeParseResult.pppoe_sid);
			printk("PPP Tag=%x\n", ntohs(PpeParseResult.ppp_tag));
		}
#if defined (CONFIG_HNAT_V2)
		printk("PKT_TYPE=%s\n",
		       PpeParseResult.pkt_type ==
		       0 ? "IPV4_HNAT" : PpeParseResult.pkt_type ==
		       1 ? "IPV4_HNAPT" : PpeParseResult.pkt_type ==
		       3 ? "IPV4_DSLITE" : PpeParseResult.pkt_type ==
		       4 ? "IPV6_ROUTE" : PpeParseResult.pkt_type ==
		       5 ? "IPV6_6RD" : "Unknown");
#else
		printk("PKT_TYPE=%s\n",
		       PpeParseResult.pkt_type ==
		       0 ? "IPV4_HNAPT" : PpeParseResult.pkt_type ==
		       1 ? "IPV4_HNAT" : PpeParseResult.pkt_type ==
		       2 ? "IPV6_ROUTE" : "Unknown");
#endif

		if (PpeParseResult.pkt_type == IPV4_HNAT) {
			printk("SIP=%s\n",
			       Ip2Str(ntohl(PpeParseResult.iph.saddr)));
			printk("DIP=%s\n",
			       Ip2Str(ntohl(PpeParseResult.iph.daddr)));
			printk("TOS=%x\n", ntohs(PpeParseResult.iph.tos));
		} else if (PpeParseResult.pkt_type == IPV4_HNAPT) {
			printk("SIP=%s\n",
			       Ip2Str(ntohl(PpeParseResult.iph.saddr)));
			printk("DIP=%s\n",
			       Ip2Str(ntohl(PpeParseResult.iph.daddr)));
			printk("TOS=%x\n", ntohs(PpeParseResult.iph.tos));
			
			if (PpeParseResult.iph.protocol == IPPROTO_TCP) {
			    printk("TCP SPORT=%d\n", ntohs(PpeParseResult.th.source));
			    printk("TCP DPORT=%d\n", ntohs(PpeParseResult.th.dest));
			}else if(PpeParseResult.iph.protocol == IPPROTO_UDP) {
			    printk("UDP SPORT=%d\n", ntohs(PpeParseResult.uh.source));
			    printk("UDP DPORT=%d\n", ntohs(PpeParseResult.uh.dest));
			}
		}
#if defined (CONFIG_HNAT_V2)
		else if (PpeParseResult.pkt_type == IPV6_6RD) {
			/* fill in ipv4 6rd entry */
			printk("SIP=%s\n",
			       Ip2Str(ntohl(PpeParseResult.iph.saddr)));
			printk("DIP=%s\n",
			       Ip2Str(ntohl(PpeParseResult.iph.daddr)));
			printk("Checksum=%x\n",
			       ntohs(PpeParseResult.iph.check));
			printk("Flag=%x\n", ntohs(PpeParseResult.iph.frag_off) >> 13);
			printk("TTL=%x\n", PpeParseResult.iph.ttl);
			printk("TOS=%x\n", PpeParseResult.iph.tos);
		}
#endif
	}
#endif

	return 0;
}

int32_t PpeFillInL2Info(struct sk_buff * skb, struct FoeEntry * foe_entry)
{
	//if this entry is already in binding state, skip it 
	if (foe_entry->bfib1.state == BIND) {
		return 1;
	}

	/* Set VLAN Info - VLAN1/VLAN2 */
#if defined (CONFIG_HNAT_V2)
	/* Set Layer2 Info - DMAC, SMAC */
	if ((PpeParseResult.pkt_type == IPV4_HNAT) || (PpeParseResult.pkt_type == IPV4_HNAPT)) {

		if(foe_entry->ipv4_hnapt.bfib1.pkt_type == IPV4_DSLITE) { //DS-Lite WAN->LAN
#if defined (CONFIG_RA_HW_NAT_IPV6)
			FoeSetMacInfo(foe_entry->ipv4_dslite.dmac_hi, PpeParseResult.dmac);
			FoeSetMacInfo(foe_entry->ipv4_dslite.smac_hi, PpeParseResult.smac);
			foe_entry->ipv4_dslite.vlan1 = ntohs(PpeParseResult.vlan1);
			foe_entry->ipv4_dslite.pppoe_id = ntohs(PpeParseResult.pppoe_sid);
			foe_entry->ipv4_dslite.vlan2 = ntohs(PpeParseResult.vlan2);
			foe_entry->ipv4_dslite.etype = ntohs(PpeParseResult.vlan_tag);
#else
			return 1;
#endif
		}else { //IPv4 WAN<->LAN
			FoeSetMacInfo(foe_entry->ipv4_hnapt.dmac_hi, PpeParseResult.dmac);
			FoeSetMacInfo(foe_entry->ipv4_hnapt.smac_hi, PpeParseResult.smac);
			foe_entry->ipv4_hnapt.vlan1 = ntohs(PpeParseResult.vlan1);
#ifdef VPRI_REMARK_TEST
			//VPRI=0x7
			foe_entry->ipv4_hnapt.vlan1 |= (7 << 13);
#endif
			foe_entry->ipv4_hnapt.pppoe_id = ntohs(PpeParseResult.pppoe_sid);
			foe_entry->ipv4_hnapt.vlan2 = ntohs(PpeParseResult.vlan2);
			foe_entry->ipv4_hnapt.etype = ntohs(PpeParseResult.vlan_tag);
		}
	} 
	else {
#if defined (CONFIG_RA_HW_NAT_IPV6)
		FoeSetMacInfo(foe_entry->ipv6_5t_route.dmac_hi, PpeParseResult.dmac);
		FoeSetMacInfo(foe_entry->ipv6_5t_route.smac_hi, PpeParseResult.smac);
		foe_entry->ipv6_5t_route.vlan1 = ntohs(PpeParseResult.vlan1);
		foe_entry->ipv6_5t_route.pppoe_id = ntohs(PpeParseResult.pppoe_sid);
		foe_entry->ipv6_5t_route.vlan2 = ntohs(PpeParseResult.vlan2);
		foe_entry->ipv6_5t_route.etype = ntohs(PpeParseResult.vlan_tag);
#else
		return 1;
#endif
	}

	/* 
	 * VLAN Layer:
	 * 0: outgoing packet is untagged packet
	 * 1: outgoing packet is tagged packet
	 * 2: outgoing packet is double tagged packet
	 * 3: outgoing packet is triple tagged packet
	 * 4: outgoing packet is fourfold tagged packet
	 */
	foe_entry->bfib1.vlan_layer = PpeParseResult.vlan_layer;

#ifdef VLAN_LAYER_TEST
	//outgoing packet is triple tagged packet
	foe_entry->bfib1.vlan_layer = 3;
	foe_entry->ipv4_hnapt.vlan1 = 2;
	foe_entry->ipv4_hnapt.vlan2 = 1;
#endif
	if (PpeParseResult.pppoe_gap) {
		foe_entry->bfib1.psn = 1;
	} else {
		foe_entry->bfib1.psn = 0;
	}

#ifdef FORCE_UP_TEST	
	foe_entry->ipv4_hnapt.bfib1.dvp = 0; //let switch decide VPRI
#else
	/* we set VID and VPRI in foe entry already, so we have to inform switch of keeping VPRI */
	foe_entry->ipv4_hnapt.bfib1.dvp = 1;
#endif
#else
	FoeSetMacInfo(foe_entry->ipv4_hnapt.dmac_hi, PpeParseResult.dmac);
	FoeSetMacInfo(foe_entry->ipv4_hnapt.smac_hi, PpeParseResult.smac);
	foe_entry->ipv4_hnapt.vlan1 = ntohs(PpeParseResult.vlan1);
	foe_entry->ipv4_hnapt.pppoe_id = ntohs(PpeParseResult.pppoe_sid);
	foe_entry->ipv4_hnapt.vlan2 = ntohs(PpeParseResult.vlan2);
	
	/* 
	 * PPE support SMART VLAN/PPPoE Tag Push/PoP feature
	 *
	 *         | MODIFY | INSERT | DELETE
	 * --------+--------+--------+----------
	 * Tagged  | modify | modify | delete
	 * Untagged| no act | insert | no act
	 *
	 */

	if (PpeParseResult.vlan1_gap) {
		foe_entry->bfib1.v1 = INSERT;
	} else {
		foe_entry->bfib1.v1 = DELETE;
	}

	if (PpeParseResult.vlan2_gap) {
		foe_entry->bfib1.v2 = INSERT;
	} else {
		foe_entry->bfib1.v2 = DELETE;
	}

	if (PpeParseResult.pppoe_gap) {
		foe_entry->bfib1.pppoe = INSERT;
	} else {
		foe_entry->bfib1.pppoe = DELETE;
	}
#endif

	return 0;
}

#if defined (CONFIG_HNAT_V2)
#if defined (CONFIG_RA_HW_NAT_IPV6)
static uint16_t PpeGetChkBase(struct iphdr *iph)
{
	uint16_t org_chksum = ntohs(iph->check);
	uint16_t org_tot_len = ntohs(iph->tot_len);
	uint32_t tmp = 0;
	uint16_t chksum_base = 0;

	tmp = ~(org_chksum) + ~(org_tot_len);
	tmp = ((tmp >> 16) && 0x7) + (tmp & 0xFFFF);
	tmp = ((tmp >> 16) && 0x7) + (tmp & 0xFFFF);
	chksum_base = tmp & 0xFFFF;

	return chksum_base;
}
#endif
#endif

int32_t PpeFillInL3Info(struct sk_buff * skb, struct FoeEntry * foe_entry)
{
	/* IPv4 or IPv4 over PPPoE */
	if ((PpeParseResult.eth_type == htons(ETH_P_IP)) || 
		(PpeParseResult.eth_type == htons(ETH_P_PPP_SES) && 
		 PpeParseResult.ppp_tag == htons(PPP_IP))) {

		if ((PpeParseResult.pkt_type == IPV4_HNAT) || (PpeParseResult.pkt_type == IPV4_HNAPT)) {

			if(foe_entry->ipv4_hnapt.bfib1.pkt_type == IPV4_DSLITE) { //DS-Lite WAN->LAN
#if defined (CONFIG_HNAT_V2)
#if defined (CONFIG_RA_HW_NAT_IPV6)
				foe_entry->ipv4_dslite.bfib1.drm = 1;	//switch will keep dscp
				foe_entry->ipv4_dslite.bfib1.rmt = 1; //remove outer IPv6 header
				foe_entry->ipv4_dslite.iblk2.dscp = PpeParseResult.iph.tos;
#endif
#endif

			} else {
#if defined (CONFIG_HNAT_V2)
				foe_entry->ipv4_hnapt.bfib1.drm = 1;	//switch will keep dscp
#endif
				foe_entry->ipv4_hnapt.new_sip = ntohl(PpeParseResult.iph.saddr);
				foe_entry->ipv4_hnapt.new_dip = ntohl(PpeParseResult.iph.daddr);
				foe_entry->ipv4_hnapt.iblk2.dscp = PpeParseResult.iph.tos;
#ifdef DSCP_REMARK_TEST
				foe_entry->ipv4_hnapt.iblk2.dscp = 0xff;
#endif
			}
		}
#if defined (CONFIG_HNAT_V2)
#if defined (CONFIG_RA_HW_NAT_IPV6)
		else if (PpeParseResult.pkt_type == IPV6_6RD) {
			/* fill in ipv4 6rd entry */
			foe_entry->ipv6_6rd.tunnel_sipv4 = ntohl(PpeParseResult.iph.saddr);
			foe_entry->ipv6_6rd.tunnel_dipv4 = ntohl(PpeParseResult.iph.daddr);
			foe_entry->ipv6_6rd.hdr_chksum = PpeGetChkBase(&PpeParseResult.iph);
			foe_entry->ipv6_6rd.flag = (ntohs(PpeParseResult.iph.frag_off) >> 13);
			foe_entry->ipv6_6rd.ttl = PpeParseResult.iph.ttl;
			foe_entry->ipv6_6rd.dscp = PpeParseResult.iph.tos;

			/* IPv4 DS-Lite and IPv6 6RD shall be turn on by SW during initialization */
			foe_entry->bfib1.pkt_type = IPV6_6RD;
		}
#endif
#endif
	}
#if defined (CONFIG_RA_HW_NAT_IPV6)
	/* IPv6 or IPv6 over PPPoE */
	else if (PpeParseResult.eth_type == htons(ETH_P_IPV6) || 
		(PpeParseResult.eth_type == htons(ETH_P_PPP_SES) && 
		 PpeParseResult.ppp_tag == htons(PPP_IPV6))) {
#if defined (CONFIG_HNAT_V2)
		if (PpeParseResult.pkt_type == IPV6_3T_ROUTE || PpeParseResult.pkt_type == IPV6_5T_ROUTE) {
			
			// incoming packet is 6RD and need to remove outer IPv4 header
			if(foe_entry->bfib1.pkt_type == IPV6_6RD) {
				foe_entry->ipv6_3t_route.bfib1.drm = 1;	//switch will keep dscp
				foe_entry->ipv6_3t_route.bfib1.rmt = 1;
				foe_entry->ipv6_3t_route.iblk2.dscp = (PpeParseResult.ip6h.priority << 4 | (PpeParseResult.ip6h.flow_lbl[0]>>4));
			} else {
				/* fill in ipv6 routing entry */
				foe_entry->ipv6_3t_route.bfib1.drm = 1;	//switch will keep dscp
				foe_entry->ipv6_3t_route.ipv6_sip0 = ntohl(PpeParseResult.ip6h.saddr.s6_addr32[0]);
				foe_entry->ipv6_3t_route.ipv6_sip1 = ntohl(PpeParseResult.ip6h.saddr.s6_addr32[1]);
				foe_entry->ipv6_3t_route.ipv6_sip2 = ntohl(PpeParseResult.ip6h.saddr.s6_addr32[2]);
				foe_entry->ipv6_3t_route.ipv6_sip3 = ntohl(PpeParseResult.ip6h.saddr.s6_addr32[3]);

				foe_entry->ipv6_3t_route.ipv6_dip0 = ntohl(PpeParseResult.ip6h.daddr.s6_addr32[0]);
				foe_entry->ipv6_3t_route.ipv6_dip1 = ntohl(PpeParseResult.ip6h.daddr.s6_addr32[1]);
				foe_entry->ipv6_3t_route.ipv6_dip2 = ntohl(PpeParseResult.ip6h.daddr.s6_addr32[2]);
				foe_entry->ipv6_3t_route.ipv6_dip3 = ntohl(PpeParseResult.ip6h.daddr.s6_addr32[3]);
				foe_entry->ipv6_3t_route.iblk2.dscp = (PpeParseResult.ip6h.priority << 4 | (PpeParseResult.ip6h.flow_lbl[0]>>4));
			}

		} else if (PpeParseResult.pkt_type == IPV4_DSLITE) {
			/* fill in DSLite entry */
			foe_entry->ipv4_dslite.tunnel_sipv6_0 = ntohl(PpeParseResult.ip6h.saddr.s6_addr32[0]);
			foe_entry->ipv4_dslite.tunnel_sipv6_1 = ntohl(PpeParseResult.ip6h.saddr.s6_addr32[1]);
			foe_entry->ipv4_dslite.tunnel_sipv6_2 = ntohl(PpeParseResult.ip6h.saddr.s6_addr32[2]);
			foe_entry->ipv4_dslite.tunnel_sipv6_3 = ntohl(PpeParseResult.ip6h.saddr.s6_addr32[3]);

			foe_entry->ipv4_dslite.tunnel_dipv6_0 = ntohl(PpeParseResult.ip6h.daddr.s6_addr32[0]);
			foe_entry->ipv4_dslite.tunnel_dipv6_1 = ntohl(PpeParseResult.ip6h.daddr.s6_addr32[1]);
			foe_entry->ipv4_dslite.tunnel_dipv6_2 = ntohl(PpeParseResult.ip6h.daddr.s6_addr32[2]);
			foe_entry->ipv4_dslite.tunnel_dipv6_3 = ntohl(PpeParseResult.ip6h.daddr.s6_addr32[3]);

			memcpy(foe_entry->ipv4_dslite.flow_lbl, PpeParseResult.ip6h.flow_lbl, sizeof(PpeParseResult.ip6h.flow_lbl));
			foe_entry->ipv4_dslite.priority = PpeParseResult.ip6h.priority;
			foe_entry->ipv4_dslite.hop_limit = PpeParseResult.ip6h.hop_limit;
			/* IPv4 DS-Lite and IPv6 6RD shall be turn on by SW during initialization */
			foe_entry->bfib1.pkt_type = IPV4_DSLITE;
		}
#else
		/* Nothing to do */
		;
#endif
	}
#endif // CONFIG_RA_HW_NAT_IPV6 //
	else {
		return 1;
	}

	return 0;
}

int32_t PpeFillInL4Info(struct sk_buff * skb, struct FoeEntry * foe_entry)
{

#if defined (CONFIG_RALINK_RT3052)
	uint32_t phy_val;
#endif

	if (PpeParseResult.pkt_type == IPV4_HNAPT) {
#if defined (CONFIG_HNAT_V2)
		// DS-LIte WAN->LAN
		if(foe_entry->ipv4_hnapt.bfib1.pkt_type == IPV4_DSLITE) {
			return 0;
		}
#endif

		/* Set Layer4 Info - NEW_SPORT, NEW_DPORT */
		if (PpeParseResult.iph.protocol == IPPROTO_TCP) {
			foe_entry->ipv4_hnapt.new_sport = ntohs(PpeParseResult.th.source);
			foe_entry->ipv4_hnapt.new_dport = ntohs(PpeParseResult.th.dest);
			foe_entry->ipv4_hnapt.bfib1.udp = TCP;
		} else if (PpeParseResult.iph.protocol == IPPROTO_UDP) {
#if defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6855A) || defined (CONFIG_RALINK_RT6352)
			foe_entry->ipv4_hnapt.new_sport = ntohs(PpeParseResult.uh.source);
			foe_entry->ipv4_hnapt.new_dport = ntohs(PpeParseResult.uh.dest);
			foe_entry->ipv4_hnapt.bfib1.udp = UDP;
#elif defined (CONFIG_RALINK_RT3352)
			if (RegRead(0xB000000C) > 0x0104) {
				foe_entry->ipv4_hnapt.new_sport = ntohs(PpeParseResult.uh.source);
				foe_entry->ipv4_hnapt.new_dport = ntohs(PpeParseResult.uh.dest);
				foe_entry->ipv4_hnapt.bfib1.udp = UDP;
			} else {
				memset(FOE_INFO_START_ADDR(skb), 0,
				       FOE_INFO_LEN);
				return 1;
			}
#elif defined (CONFIG_RALINK_RT3052)
			rw_rf_reg(0, 0, &phy_val);
			phy_val = phy_val & 0xFF;

			if (phy_val > 0x53) {
				foe_entry->ipv4_hnapt.new_sport = ntohs(PpeParseResult.uh.source);
				foe_entry->ipv4_hnapt.new_dport = ntohs(PpeParseResult.uh.dest);
				foe_entry->ipv4_hnapt.bfib1.udp = UDP;
			} else {
				return 1;
			}
#else
			/* if udp checksum is zero, it cannot be accelerated by HNAT */
			/* we found the application is possible to use udp checksum=0 at first stage, 
			 * then use non-zero checksum in the same session later, so we disable HNAT acceleration
			 * for all UDP flows */
			return 1;
#endif
		}
	} else if (PpeParseResult.pkt_type == IPV4_HNAT) {
		/* do nothing */
	} else if (PpeParseResult.pkt_type == IPV6_1T_ROUTE) {
		/* do nothing */
#if defined (CONFIG_HNAT_V2)
#if defined (CONFIG_RA_HW_NAT_IPV6)
	} else if (PpeParseResult.pkt_type == IPV6_3T_ROUTE) {
		/* do nothing */
	} else if (PpeParseResult.pkt_type == IPV6_5T_ROUTE) {
		/* do nothing */
#endif
#endif
	}

	return 0;
}

#if defined(CONFIG_HNAT_V2)
int32_t PpeSetAGInfo(uint16_t index, uint16_t vlan_id)
{
	struct l2_rule L2Rule;
	uint32_t *p = (uint32_t *) & L2Rule;

	memset(&L2Rule, 0, sizeof(L2Rule));

	L2Rule.others.vid = vlan_id;
	L2Rule.others.v = 1;

	L2Rule.com.rt = L2_RULE;
	L2Rule.com.pn = PN_DONT_CARE;
	L2Rule.com.match = 1;

	L2Rule.com.ac.ee = 1;
	L2Rule.com.ac.ag = index;

	L2Rule.com.dir = OTHERS;
	RegWrite(POLICY_TBL_BASE + index * 8, *p);	/* Low bytes */
	RegWrite(POLICY_TBL_BASE + index * 8 + 4, *(p + 1));	/* High bytes */

	return 1;
}

/* token_rate: unit= KB/S */
int32_t PpeSetMtrByteInfo(uint16_t MgrIdx, uint32_t TokenRate, uint32_t MaxBkSize)
{
	uint32_t MtrEntry = 0;

        MtrEntry = ((TokenRate << 3) | (MaxBkSize << 1));
        
	RegWrite(METER_BASE + MgrIdx * 4, MtrEntry);

	printk("Meter Table Base=%08X Offset=%d\n", METER_BASE, MgrIdx * 4);
        printk("%08X: %08X\n", METER_BASE + MgrIdx * 4, MtrEntry);
	
	return 1;
}

int32_t PpeSetMtrPktInfo(uint16_t MgrIdx, uint32_t MtrIntval, uint32_t MaxBkSize)
{
	uint32_t MtrEntry = 0;

	MtrEntry = ((MtrIntval << 8) | (MaxBkSize << 1) | 1);

        RegWrite(METER_BASE + MgrIdx * 4, MtrEntry);

	printk("Meter Table Base=%08X Offset=%d\n", METER_BASE, MgrIdx * 4);
        printk("%08X: %08X\n", METER_BASE + MgrIdx * 4, MtrEntry);

	return 1;
}

static void PpeSetInfoBlk2(struct _info_blk2 *iblk2, uint32_t fpidx, uint32_t port_mg, uint32_t port_ag)
{
#ifdef FORCE_UP_TEST	
	uint32_t reg;
	
	iblk2->fp = 1;
	iblk2->up = 7;

	//Replace 802.Q priority by user priority
	reg = RegRead(RALINK_ETH_SW_BASE + 0x2704);
	reg |= (0x1 << 11);
	RegWrite(RALINK_ETH_SW_BASE + 0x2704, reg);
#endif

	iblk2->fpidx = fpidx;
	iblk2->port_mg = port_mg;
	iblk2->port_ag = port_ag;
}
#endif

/* Set force port info */
int32_t
PpeSetForcePortInfo(struct sk_buff * skb,
		    struct FoeEntry * foe_entry, int gmac_no)
{
#if !defined(CONFIG_HNAT_V2)
	foe_entry->ipv4_hnapt.iblk2.fd = 1;
#endif
	/* CPU need to handle traffic between WLAN/PCI and GMAC port */
	if ((strncmp(skb->dev->name, "ra", 2) == 0)
	    || (strncmp(skb->dev->name, "wds", 3) == 0)
	    || (strncmp(skb->dev->name, "mesh", 4) == 0)
	    || (strncmp(skb->dev->name, "apcli", 5) == 0)
#if defined (CONFIG_RA_HW_NAT_PCI)
	    || ((skb->dev != NULL) && (skb->dev == DstPort[DP_PCI]))
#endif
	) {
#if defined (CONFIG_RA_HW_NAT_WIFI) || defined (CONFIG_RA_HW_NAT_PCI)
	    if (wifi_offload) { /* wifi offload enabled */
#if defined (CONFIG_HNAT_V2)
 		if (IS_IPV4_GRP(foe_entry)) {
		    PpeSetInfoBlk2(&foe_entry->ipv4_hnapt.iblk2, 6, 0x3F, 0x3F);
		}
#if defined (CONFIG_RA_HW_NAT_IPV6)
		else if (IS_IPV6_GRP(foe_entry)) {
		    PpeSetInfoBlk2(&foe_entry->ipv6_3t_route.iblk2, 6, 0x3F, 0x3F);
		}
#endif
#else
	    foe_entry->ipv4_hnapt.iblk2.dp = 0;	/* cpu */
#endif
	    } else { /* wifi offload disabled */
		return 1;
	    }
#else
		return 1;
#endif // CONFIG_RA_HW_NAT_WIFI || CONFIG_RA_HW_NAT_PCI //

	} else {
		/* RT3883 with 2xGMAC - Assuming GMAC2=WAN  and GMAC1=LAN */
#if defined (CONFIG_RAETH_GMAC2)
		if (gmac_no == 1) {
			if ((bind_dir == DOWNSTREAM_ONLY)
			    || (bind_dir == BIDIRECTION)) {
				foe_entry->ipv4_hnapt.iblk2.dp = 1;
			} else {
				return 1;
			}
		} else if (gmac_no == 2) {
			if ((bind_dir == UPSTREAM_ONLY)
			    || (bind_dir == BIDIRECTION)) {
				foe_entry->ipv4_hnapt.iblk2.dp = 2;
			} else {
				return 1;
			}
		}

		/* RT2880, RT3883 */
#elif defined (CONFIG_RALINK_RT2880) || defined (CONFIG_RALINK_RT3883)
		if ((foe_entry->ipv4_hnapt.vlan1 & VLAN_VID_MASK) == LAN_PORT_VLAN_ID) {
			if ((bind_dir == DOWNSTREAM_ONLY)
			    || (bind_dir == BIDIRECTION)) {
				foe_entry->ipv4_hnapt.iblk2.dp = 1;
			} else {
				return 1;
			}
		} else if ((foe_entry->ipv4_hnapt.vlan1 & VLAN_VID_MASK) ==
			   WAN_PORT_VLAN_ID) {
			if ((bind_dir == UPSTREAM_ONLY)
			    || (bind_dir == BIDIRECTION)) {
				foe_entry->ipv4_hnapt.iblk2.dp = 1;
			} else {
				return 1;
			}
		}

#elif defined (CONFIG_HNAT_V2)
		if(IS_IPV4_GRP(foe_entry)) {
			if ((foe_entry->ipv4_hnapt.vlan1 & VLAN_VID_MASK) == LAN_PORT_VLAN_ID) {
				if ((bind_dir == DOWNSTREAM_ONLY) || (bind_dir == BIDIRECTION)) {
					PpeSetInfoBlk2(&foe_entry->ipv4_hnapt.iblk2, 8, 0x3F, 2);
				} else {
					return 1;
				}
			} else if ((foe_entry->ipv4_hnapt.vlan1 & VLAN_VID_MASK) == WAN_PORT_VLAN_ID) {
				if ((bind_dir == UPSTREAM_ONLY) || (bind_dir == BIDIRECTION)) {
					PpeSetInfoBlk2(&foe_entry->ipv4_hnapt.iblk2, 8, 0x3F, 1);
				} else {
					return 1;
				}
			} else { //one-arm
				PpeSetInfoBlk2(&foe_entry->ipv4_hnapt.iblk2, 8, 0x3F, 1);
			}
		} 
		
#if defined (CONFIG_RA_HW_NAT_IPV6)
		else if(IS_IPV6_GRP(foe_entry)) {
			if ((foe_entry->ipv6_5t_route.vlan1 & VLAN_VID_MASK) == WAN_PORT_VLAN_ID) {
				if ((bind_dir == DOWNSTREAM_ONLY) || (bind_dir == BIDIRECTION)) {
					PpeSetInfoBlk2(&foe_entry->ipv6_5t_route.iblk2, 8, 0x3F, 2);
				} else {
					return 1;
				}
			} else if ((foe_entry->ipv6_5t_route.vlan1 & VLAN_VID_MASK) == WAN_PORT_VLAN_ID) {
				if ((bind_dir == UPSTREAM_ONLY) || (bind_dir == BIDIRECTION)) {
					PpeSetInfoBlk2(&foe_entry->ipv6_5t_route.iblk2, 8, 0x3F, 1);
				} else {
					return 1;
				}
			} else { //one-arm
				PpeSetInfoBlk2(&foe_entry->ipv6_5t_route.iblk2, 8, 0x3F, 1);
			}
		}
#endif // CONFIG_RA_HW_NAT_IPV6 //

#else
		/*  RT3052, RT335x */
		if ((foe_entry->ipv4_hnapt.vlan1 & VLAN_VID_MASK) == LAN_PORT_VLAN_ID) {
			if ((bind_dir == DOWNSTREAM_ONLY) || (bind_dir == BIDIRECTION)) {
				foe_entry->ipv4_hnapt.iblk2.dp = 1;	/* LAN traffic use VirtualPort1 in GMAC1 */
			} else {
				return 1;
			}
		} else if ((foe_entry->ipv4_hnapt.vlan1 & VLAN_VID_MASK) == WAN_PORT_VLAN_ID) {
			if ((bind_dir == UPSTREAM_ONLY) || (bind_dir == BIDIRECTION)) {
				foe_entry->ipv4_hnapt.iblk2.dp = 2;	/* WAN traffic use VirtualPort2 in GMAC1 */
			} else {
				return 1;
			}
		} else {
			/* for one arm NAT test -> no vlan tag */
			foe_entry->ipv4_hnapt.iblk2.dp = 1;
		}
#endif
	}

	return 0;
}

uint32_t PpeSetExtIfNum(struct sk_buff * skb, struct FoeEntry * foe_entry)
{
#if defined  (CONFIG_RA_HW_NAT_WIFI) || defined  (CONFIG_RA_HW_NAT_PCI)
	uint32_t offset = 0;

	if (!wifi_offload)
	    return 0;

	/* This is ugly soultion to support WiFi pseudo interface.
	 * Please double check the definition is the same as include/rt_linux.h 
	 */
#define CB_OFF  10
#define RTMP_GET_PACKET_IF(skb)                 skb->cb[CB_OFF+6]
#define MIN_NET_DEVICE_FOR_MBSSID               0x00
#define MIN_NET_DEVICE_FOR_WDS                  0x10
#define MIN_NET_DEVICE_FOR_APCLI                0x20
#define MIN_NET_DEVICE_FOR_MESH                 0x30

	/* Set actual output port info */
#if defined (CONFIG_RTDEV_MII) || defined (CONFIG_RTDEV_USB) || \
    defined (CONFIG_RTDEV_PCI) || defined (CONFIG_RTDEV)
	if (strncmp(skb->dev->name, "rai", 3) == 0) {
#if defined (CONFIG_RT3090_AP_MESH) || defined (CONFIG_RT5392_AP_MESH) || \
    defined (CONFIG_RT3572_AP_MESH) || defined (CONFIG_RT5572_AP_MESH) || \
    defined (CONFIG_RT5592_AP_MESH) || defined (CONFIG_RT3593_AP_MESH)
		if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_MESH) {
			offset =
			    (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_MESH +
			     DP_MESHI0);
		} else
#endif // CONFIG_RTDEV_AP_MESH //

#if defined (CONFIG_RT3090_AP_APCLI) || defined (CONFIG_RT5392_AP_APCLI) || \
    defined (CONFIG_RT3572_AP_APCLI) || defined (CONFIG_RT5572_AP_APCLI) || \
    defined (CONFIG_RT5592_AP_APCLI) || defined (CONFIG_RT3593_AP_APCLI)

		if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_APCLI) {
			offset =
			    (RTMP_GET_PACKET_IF(skb) -
			     MIN_NET_DEVICE_FOR_APCLI + DP_APCLII0);
		} else
#endif // CONFIG_RTDEV_AP_APCLI //
#if defined (CONFIG_RT3090_AP_WDS) || defined (CONFIG_RT5392_AP_WDS) || \
    defined (CONFIG_RT3572_AP_WDS) || defined (CONFIG_RT5572_AP_WDS) || \
    defined (CONFIG_RT5592_AP_WDS) || defined (CONFIG_RT3593_AP_WDS)
		if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_WDS) {
			offset =
			    (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_WDS +
			     DP_WDSI0);
		} else
#endif // CONFIG_RTDEV_AP_WDS //
		{
			offset = RTMP_GET_PACKET_IF(skb) + DP_RAI0;
		}
	} else
#endif // CONFIG_RTDEV_MII || CONFIG_RTDEV_USB || CONFIG_RTDEV_PCI || CONFIG_RTDEV

	if (strncmp(skb->dev->name, "ra", 2) == 0) {
#if defined (CONFIG_RT2860V2_AP_MESH)
		if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_MESH) {
			offset =
			    (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_MESH +
			     DP_MESH0);
		} else
#endif // CONFIG_RT2860V2_AP_MESH //
#if defined (CONFIG_RT2860V2_AP_APCLI)
		if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_APCLI) {
			offset =
			    (RTMP_GET_PACKET_IF(skb) -
			     MIN_NET_DEVICE_FOR_APCLI + DP_APCLI0);
		} else
#endif // CONFIG_RT2860V2_AP_APCLI //
#if defined (CONFIG_RT2860V2_AP_WDS)
		if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_WDS) {
			offset =
			    (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_WDS +
			     DP_WDS0);
		} else
#endif // CONFIG_RT2860V2_AP_WDS //
		{
			offset = RTMP_GET_PACKET_IF(skb) + DP_RA0;
		}
	}
#if defined (CONFIG_RA_HW_NAT_PCI)
	else if (strncmp(skb->dev->name, "eth0", 4) == 0) {
		offset = DP_PCI;
	}
#endif // CONFIG_RA_HW_NAT_PCI //
	else if (strncmp(skb->dev->name, "eth2", 4) == 0) {
		offset = DP_GMAC;
	}
#ifdef CONFIG_RAETH_GMAC2
	else if (strncmp(skb->dev->name, "eth3", 4) == 0) {
		offset = DP_GMAC2;
	}
#endif
	else {
		printk("HNAT: unknow interface %s\n", skb->dev->name);
		return 1;
	}

	if (IS_IPV4_HNAT(foe_entry) || IS_IPV4_HNAPT(foe_entry)) {
		foe_entry->ipv4_hnapt.act_dp = offset;
	}

#if defined (CONFIG_HNAT_V2)
#if defined (CONFIG_RA_HW_NAT_IPV6)
	else if (IS_IPV4_DSLITE(foe_entry)) {
		foe_entry->ipv4_dslite.act_dp = offset;
	} else if (IS_IPV6_3T_ROUTE(foe_entry)) {
		foe_entry->ipv6_3t_route.act_dp = offset;
	} else if (IS_IPV6_5T_ROUTE(foe_entry)) {
		foe_entry->ipv6_5t_route.act_dp = offset;
	} else if (IS_IPV6_6RD(foe_entry)) {
		foe_entry->ipv6_6rd.act_dp = offset;
	} else {
		return 1;
	}
#endif // CONFIG_RA_HW_NAT_IPV6 //
#endif // CONFIG_HNAT_V2 //
#endif // CONFIG_RA_HW_NAT_WIFI || CONFIG_RA_HW_NAT_PCI //

	return 0;
}

void PpeSetEntryBind(struct sk_buff *skb, struct FoeEntry *foe_entry)
{

	uint32_t current_time;
	/* Set Current time to time_stamp field in information block 1 */
	current_time = RegRead(FOE_TS) & 0xFFFF;
	foe_entry->bfib1.time_stamp = (uint16_t) current_time;

	/* Ipv4: TTL / Ipv6: Hot Limit filed */
	foe_entry->ipv4_hnapt.bfib1.ttl = DFL_FOE_TTL_REGEN;
#if defined (CONFIG_HNAT_V2)
	/* enable cache by default */
	foe_entry->ipv4_hnapt.bfib1.cah = 1;
#endif

#if defined (CONFIG_RA_HW_NAT_ACL2UP_HELPER)
	/*set user priority */
	foe_entry->ipv4_hnapt.iblk2.up = FOE_SP(skb);
	foe_entry->ipv4_hnapt.iblk2.fp = 1;
#endif

	/* Change Foe Entry State to Binding State */
	foe_entry->bfib1.state = BIND;

}

int32_t PpeTxHandler(struct sk_buff *skb, int gmac_no)
{
	struct FoeEntry *foe_entry = &PpeFoeBase[FOE_ENTRY_NUM(skb)];

	if (!skb) {
#ifdef HWNAT_DEBUG
	    NAT_PRINT("HNAT: skb is null ?\n");
#endif
	    return 1;
	}

	/* return trunclated packets to normal path with padding */
	if (skb->len < ETH_HLEN) {
	    memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
#ifdef HWNAT_DEBUG
	    NAT_PRINT("HNAT: skb null or small len in tx path\n");
#endif
	    return 1;
	}

	/*
	 * Packet is interested by ALG?
	 * Yes: Don't enter binind state
	 * No: If flow rate exceed binding threshold, enter binding state.
	 */

	/* FIXME: why 6RD WAN->LAN path needs ALG */
#if defined (CONFIG_HNAT_V2) && defined(CONFIG_RA_HW_NAT_IPV6)
	if (IS_MAGIC_TAG_VALID(skb) && (FOE_AI(skb) == HIT_UNBIND_RATE_REACH) && ((FOE_ALG(skb) == 0) || IS_IPV6_6RD(foe_entry)) ) {
#else
	if (IS_MAGIC_TAG_VALID(skb) && (FOE_AI(skb) == HIT_UNBIND_RATE_REACH) && (FOE_ALG(skb) == 0) ) {
#endif

		/* get start addr for each layer */
		if (PpeParseLayerInfo(skb)) {
			memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
			return 1;
		}

		/* Set Layer2 Info */
		if (PpeFillInL2Info(skb, foe_entry)) {
			memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
			return 1;
		}

		/* Set Layer3 Info */
		if (PpeFillInL3Info(skb, foe_entry)) {
			memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
			return 1;
		}

		/* Set Layer4 Info */
		if (PpeFillInL4Info(skb, foe_entry)) {
			memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
			return 1;
		}

		/* Set force port info */
		if (PpeSetForcePortInfo(skb, foe_entry, gmac_no)) {
			memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
			return 1;
		}

		/* Set Pseudo Interface info in Foe entry */
		if (PpeSetExtIfNum(skb, foe_entry)) {
			memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
			return 1;
		}

		/* Enter binding state */
		PpeSetEntryBind(skb, foe_entry);

#ifdef HWNAT_DEBUG
		/* Dump Binding Entry */
		if (DebugLevel >= 1) {
			FoeDumpEntry(FOE_ENTRY_NUM(skb));
		}
#endif
#if defined (CONFIG_HNAT_V2)
	} else if (IS_MAGIC_TAG_VALID(skb)
		   && (FOE_AI(skb) == HIT_BIND_KEEPALIVE_MC_NEW_HDR
		       || (FOE_AI(skb) == HIT_BIND_KEEPALIVE_DUP_OLD_HDR))) {
#else
	} else if (IS_MAGIC_TAG_VALID(skb)
		   && (FOE_AI(skb) == HIT_BIND_KEEPALIVE)
		   && (DFL_FOE_KA == 0)) {
#endif
		/* this is duplicate packet in keepalive new header mode, 
		 * just drop it */
		memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
		return 0;
#if defined (CONFIG_HNAT_V2) && defined(CONFIG_RA_HW_NAT_IPV6)
	} else if (IS_MAGIC_TAG_VALID(skb) && (FOE_AI(skb) == HIT_UNBIND_RATE_REACH)
		   && ((FOE_ALG(skb) == 1) && !IS_IPV6_6RD(foe_entry))) {
#else
	} else if (IS_MAGIC_TAG_VALID(skb) && (FOE_AI(skb) == HIT_UNBIND_RATE_REACH) && (FOE_ALG(skb) == 1)) {
#endif
#ifdef HWNAT_DEBUG
		if (DebugLevel >= 2) {
			NAT_PRINT ("FOE_ALG=1 (Entry=%d)\n", FOE_ENTRY_NUM(skb));
		}
#endif
	}

	return 1;
}

void PpeSetFoeEbl(uint32_t FoeEbl)
{
	uint32_t PpeFlowSet = 0;

	PpeFlowSet = RegRead(PPE_FLOW_SET);

	/* FOE engine need to handle unicast/multicast/broadcast flow */
	if (FoeEbl == 1) {
		PpeFlowSet |= (BIT_IPV4_NAPT_EN | BIT_IPV4_NAT_EN);
#if defined(HWNAT_SKIP_MCAST_BCAST)
		PpeFlowSet |= (BIT_FUC_FOE);
#else
		PpeFlowSet |= (BIT_FUC_FOE | BIT_FMC_FOE | BIT_FBC_FOE);
#endif
#if defined (CONFIG_HNAT_V2)
		PpeFlowSet |= (BIT_IPV4_NAT_FRAG_EN); //ip fragment
#if defined(CONFIG_RA_HW_NAT_IPV6)
		PpeFlowSet |= (BIT_IPV4_DSL_EN | BIT_IPV6_6RD_EN | BIT_IPV6_3T_ROUTE_EN | BIT_IPV6_5T_ROUTE_EN);
//		PpeFlowSet |= (BIT_IPV6_HASH_FLAB); // flow label
#endif

#else
#if defined(CONFIG_RA_HW_NAT_IPV6)
		PpeFlowSet |= (BIT_IPV6_FOE_EN);
#endif

#endif
	} else {
		PpeFlowSet &= ~(BIT_IPV4_NAPT_EN | BIT_IPV4_NAT_EN);
		PpeFlowSet &= ~(BIT_FUC_FOE | BIT_FMC_FOE | BIT_FBC_FOE);
#if defined (CONFIG_HNAT_V2)
		PpeFlowSet &= ~(BIT_IPV4_NAT_FRAG_EN);
#if defined(CONFIG_RA_HW_NAT_IPV6)
		PpeFlowSet &= ~(BIT_IPV4_DSL_EN | BIT_IPV6_6RD_EN | BIT_IPV6_3T_ROUTE_EN | BIT_IPV6_5T_ROUTE_EN);
//		PpeFlowSet &= ~(BIT_IPV6_HASH_FLAB);
#else
#if defined(CONFIG_RA_HW_NAT_IPV6)
		PpeFlowSet &= ~(BIT_IPV6_FOE_EN);
#endif

#endif
#endif
	}

	RegWrite(PPE_FLOW_SET, PpeFlowSet);
}

static void PpeSetFoeHashMode(uint32_t HashMode)
{

	/* Allocate FOE table base */
	FoeAllocTbl(FOE_4TB_SIZ);

	switch (FOE_4TB_SIZ) {
	case 1024:
		RegModifyBits(PPE_FOE_CFG, FoeTblSize_1K, 0, 3);
		break;
	case 2048:
		RegModifyBits(PPE_FOE_CFG, FoeTblSize_2K, 0, 3);
		break;
	case 4096:
		RegModifyBits(PPE_FOE_CFG, FoeTblSize_4K, 0, 3);
		break;
	case 8192:
		RegModifyBits(PPE_FOE_CFG, FoeTblSize_8K, 0, 3);
		break;
	case 16384:
		RegModifyBits(PPE_FOE_CFG, FoeTblSize_16K, 0, 3);
		break;
	}

	/* Set Hash Mode */
#if defined (CONFIG_HNAT_V2)
	RegModifyBits(PPE_FOE_CFG, HashMode, 14, 2);
	RegWrite(PPE_HASH_SEED, HASH_SEED);
#if defined (CONFIG_RA_HW_NAT_IPV6)
	RegModifyBits(PPE_FOE_CFG, 1, 3, 1);	//entry size = 80bytes
#else
	RegModifyBits(PPE_FOE_CFG, 0, 3, 1);	//entry size = 64bytes
#endif
#else
	RegModifyBits(PPE_FOE_CFG, HashMode, 3, 1);
#endif

	/* Set action for FOE search miss */
	RegModifyBits(PPE_FOE_CFG, FWD_CPU_BUILD_ENTRY, 4, 2);
}

static void PpeSetAgeOut(void)
{
#if defined (CONFIG_HNAT_V2)
	/* set Bind Non-TCP/UDP Age Enable */
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_NTU_AGE, 7, 1);
#endif

	/* set Unbind State Age Enable */
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_UNB_AGE, 8, 1);

	/* set min threshold of packet count for aging out at unbind state */
	RegModifyBits(PPE_FOE_UNB_AGE, DFL_FOE_UNB_MNP, 16, 16);

	/* set Delta time for aging out an unbind FOE entry */
	RegModifyBits(PPE_FOE_UNB_AGE, DFL_FOE_UNB_DLTA, 0, 8);

	/* set Bind TCP Age Enable */
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_TCP_AGE, 9, 1);

	/* set Bind UDP Age Enable */
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_UDP_AGE, 10, 1);


	/* set Bind TCP FIN Age Enable */
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_FIN_AGE, 11, 1);

	/* set Delta time for aging out an bind UDP FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE0, DFL_FOE_UDP_DLTA, 0, 16);
	
#if defined (CONFIG_HNAT_V2)
	/* set Delta time for aging out an bind Non-TCP/UDP FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE0, DFL_FOE_NTU_DLTA, 16, 16);
#endif

	/* set Delta time for aging out an bind TCP FIN FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE1, DFL_FOE_FIN_DLTA, 16, 16);

	/* set Delta time for aging out an bind TCP FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE1, DFL_FOE_TCP_DLTA, 0, 16);
}

static void PpeSetFoeKa(void)
{
	/* set Keep alive packet with new/org header */
#if defined (CONFIG_HNAT_V2)
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_KA, 12, 2);
#else
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_KA, 12, 1);

	/* set Keep alive enable */
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_KA_EN, 13, 1);
#endif

	/* Keep alive timer value */
	RegModifyBits(PPE_FOE_KA, DFL_FOE_KA_T, 0, 16);

	/* Keep alive time for bind FOE TCP entry */
	RegModifyBits(PPE_FOE_KA, DFL_FOE_TCP_KA, 16, 8);

	/* Keep alive timer for bind FOE UDP entry */
	RegModifyBits(PPE_FOE_KA, DFL_FOE_UDP_KA, 24, 8);
	
#if defined (CONFIG_HNAT_V2)
	/* Keep alive timer for bind Non-TCP/UDP entry */
	RegModifyBits(PPE_BIND_LMT_1, DFL_FOE_NTU_KA, 16, 8);
#endif
}

static void PpeSetFoeBindRate(uint32_t FoeBindRate)
{
	/* Allowed max entries to be build during a time stamp unit */

	/* smaller than 1/4 of total entries */
	RegModifyBits(PPE_FOE_LMT1, DFL_FOE_QURT_LMT, 0, 14);

	/* between 1/2 and 1/4 of total entries */
	RegModifyBits(PPE_FOE_LMT1, DFL_FOE_HALF_LMT, 16, 14);

	/* between full and 1/2 of total entries */
	RegModifyBits(PPE_FOE_LMT2, DFL_FOE_FULL_LMT, 0, 14);

	/* Set reach bind rate for unbind state */
	RegWrite(PPE_FOE_BNDR, FoeBindRate);
}

static void PpeSetFoeGloCfgEbl(uint32_t Ebl)
{
#if defined (CONFIG_HNAT_V2)
	uint32_t tpf = 0;
#endif

	if (Ebl == 1) {
#if defined (CONFIG_HNAT_V2)
		/* 1. Remove P7 on forwarding ports */
		/* It's chip default setting */

		/* 2. PPE Forward Control Register: PPE_PORT=Port7 */
		RegModifyBits(PFC, 7, 0, 3);
	    	
		/* 3. Select P7 as PPE port (PPE_EN=1) */
		RegModifyBits(PFC, 1, 3, 1);
	
		/* TO_PPE Forwarding Register */	
		tpf = IPV4_PPE_MYUC | IPV4_PPE_MC | IPV4_PPE_IPM | IPV4_PPE_UC | IPV4_PPE_UN;
	
#if defined (CONFIG_RA_HW_NAT_IPV6)
		tpf |= (IPV6_PPE_MYUC | IPV6_PPE_MC | IPV6_PPE_IPM | IPV6_PPE_UC | IPV6_PPE_UN);
#endif

		RegWrite(TPF0, tpf);
		RegWrite(TPF1, tpf);
		RegWrite(TPF2, tpf);
		RegWrite(TPF3, tpf);
		RegWrite(TPF4, tpf);
		RegWrite(TPF5, tpf);

		/* Forced Port7 link up, 1Gbps, and Full duplex  */
		RegWrite(PMCR_P7, 0x5e33b);

		/* Disable SA Learning */
		RegModifyBits(PSC_P7, 1, 4, 1);

		/* Use default values on P7 */
#else
		/* PPE Engine Enable */
		RegModifyBits(PPE_GLO_CFG, 1, 0, 1);
		
		/* PPE Packet with TTL=0 */
		RegModifyBits(PPE_GLO_CFG, DFL_TTL0_DRP, 4, 1);

		/* Use VLAN priority tag as priority decision */
		RegModifyBits(PPE_GLO_CFG, DFL_VPRI_EN, 8, 1);

		/* Use DSCP as priority decision */
		RegModifyBits(PPE_GLO_CFG, DFL_DPRI_EN, 9, 1);

		/* Re-generate VLAN priority tag */
		RegModifyBits(PPE_GLO_CFG, DFL_REG_VPRI, 10, 1);

		/* Re-generate DSCP */
		RegModifyBits(PPE_GLO_CFG, DFL_REG_DSCP, 11, 1);

		/* Random early drop mode */
		RegModifyBits(PPE_GLO_CFG, DFL_RED_MODE, 12, 2);

		/* Enable use ACL force priority for hit unbind 
		 * and rate reach packet in CPU reason */
		RegModifyBits(PPE_GLO_CFG, DFL_ACL_PRI_EN, 14, 1);

		/* Disable switch port 6 flow control if HNAT QoS is needed */
		//RegModifyBits(RALINK_ETH_SW_BASE+0xC8, 0x0, 8, 2);
#endif // CONFIG_HNAT_V2 //

		/* PPE Packet with TTL=0 */
		RegModifyBits(PPE_GLO_CFG, DFL_TTL0_DRP, 4, 1);

#if defined (CONFIG_RAETH_SPECIAL_TAG)
		/*TODO: for Ralink GSW */

		/* Set GDMA1 GDM1_TCI_81xx */
		RegModifyBits(FE_GDMA1_FWD_CFG, 0x1, 24, 1);
		/* Set GDMA2 GDM2_TCI_81xx */
		RegModifyBits(FE_GDMA2_FWD_CFG, 0x1, 24, 1);
		/* Set EXT_SW_EN = 1 */
		RegModifyBits(FE_COS_MAP, 0x1, 30, 1);
#endif

	} else {
#if defined (CONFIG_HNAT_V2)
	    	/* 1. Select P7 as PPE port (PPE_EN=1) */
		RegModifyBits(PFC, 0, 3, 1);
	    
		/* TO_PPE Forwarding Register */	
		RegWrite(TPF0, 0);
		RegWrite(TPF1, 0);
		RegWrite(TPF2, 0);
		RegWrite(TPF3, 0);
		RegWrite(TPF4, 0);
		RegWrite(TPF5, 0);
		
		/* Forced Port7 link down */
		RegWrite(PMCR_P7, 0x5e330);

		/* Enable SA Learning */
		RegModifyBits(PSC_P7, 0, 4, 1);
#else
		/* PPE Engine Disable */
		RegModifyBits(PPE_GLO_CFG, 0, 0, 1);
#endif

#if defined (CONFIG_RAETH_SPECIAL_TAG)
		/* Remove GDMA1 GDM1_TCI_81xx */
		RegModifyBits(FE_GDMA1_FWD_CFG, 0x0, 24, 1);
		/* Remove GDMA2 GDM2_TCI_81xx */
		RegModifyBits(FE_GDMA2_FWD_CFG, 0x0, 24, 1);
		/* Remove EXT_SW_EN = 1 */
		RegModifyBits(FE_COS_MAP, 0x0, 30, 1);
#endif
	}

}

/*
 * - VLAN->UP: Incoming VLAN Priority to User Priority (Fixed)
 * - DSCP->UP: Incoming DSCP to User Priority
 * - UP->xxx : User Priority to VLAN/InDSCP/OutDSCP/AC Priority Mapping
 *
 * VLAN | DSCP |  UP |VLAN Pri|In-DSCP |Out-DSCP| AC | WMM_AC
 * -----+------+-----+--------+--------+--------+----+-------
 *   0	| 00-07|  0  |   0    |  0x00  |  0x00	|  0 |  BE
 *   3	| 24-31|  3  |   3    |  0x18  |  0x10	|  0 |  BE
 *   1	| 08-15|  1  |   1    |  0x08  |  0x00	|  0 |  BG
 *   2  | 16-23|  2  |   2    |  0x10  |  0x08	|  0 |  BG
 *   4	| 32-39|  4  |   4    |  0x20  |  0x18	|  1 |  VI
 *   5  | 40-47|  5  |   5    |  0x28  |  0x20	|  1 |  VI
 *   6  | 48-55|  6  |   6    |  0x30  |  0x28	|  2 |  VO
 *   7	| 56-63|  7  |   7    |  0x38  |  0x30	|  2 |  VO
 * -----+------+-----+--------+--------+--------+----+--------
 *
 */
static void PpeSetUserPriority(void)
{
#if !defined (CONFIG_HNAT_V2)
	/* Set weight of decision in resolution */
	RegWrite(UP_RES, DFL_UP_RES);

	/* Set DSCP to User priority mapping table */
	RegWrite(DSCP0_7_MAP_UP, DFL_DSCP0_7_UP);
	RegWrite(DSCP24_31_MAP_UP, DFL_DSCP24_31_UP);
	RegWrite(DSCP8_15_MAP_UP, DFL_DSCP8_15_UP);
	RegWrite(DSCP16_23_MAP_UP, DFL_DSCP16_23_UP);
	RegWrite(DSCP32_39_MAP_UP, DFL_DSCP32_39_UP);
	RegWrite(DSCP40_47_MAP_UP, DFL_DSCP40_47_UP);
	RegWrite(DSCP48_55_MAP_UP, DFL_DSCP48_55_UP);
	RegWrite(DSCP56_63_MAP_UP, DFL_DSCP56_63_UP);

	/* Set mapping table of user priority to vlan priority */
	RegModifyBits(UP_MAP_VPRI, DFL_UP0_VPRI, 0, 3);
	RegModifyBits(UP_MAP_VPRI, DFL_UP1_VPRI, 4, 3);
	RegModifyBits(UP_MAP_VPRI, DFL_UP2_VPRI, 8, 3);
	RegModifyBits(UP_MAP_VPRI, DFL_UP3_VPRI, 12, 3);
	RegModifyBits(UP_MAP_VPRI, DFL_UP4_VPRI, 16, 3);
	RegModifyBits(UP_MAP_VPRI, DFL_UP5_VPRI, 20, 3);
	RegModifyBits(UP_MAP_VPRI, DFL_UP6_VPRI, 24, 3);
	RegModifyBits(UP_MAP_VPRI, DFL_UP7_VPRI, 28, 3);

	/* Set mapping table of user priority to in-profile DSCP */
	RegModifyBits(UP0_3_MAP_IDSCP, DFL_UP0_IDSCP, 0, 6);
	RegModifyBits(UP0_3_MAP_IDSCP, DFL_UP1_IDSCP, 8, 6);
	RegModifyBits(UP0_3_MAP_IDSCP, DFL_UP2_IDSCP, 16, 6);
	RegModifyBits(UP0_3_MAP_IDSCP, DFL_UP3_IDSCP, 24, 6);
	RegModifyBits(UP4_7_MAP_IDSCP, DFL_UP4_IDSCP, 0, 6);
	RegModifyBits(UP4_7_MAP_IDSCP, DFL_UP5_IDSCP, 8, 6);
	RegModifyBits(UP4_7_MAP_IDSCP, DFL_UP6_IDSCP, 16, 6);
	RegModifyBits(UP4_7_MAP_IDSCP, DFL_UP7_IDSCP, 24, 6);

	/* Set mapping table of user priority to out-profile DSCP */
	RegModifyBits(UP0_3_MAP_ODSCP, DFL_UP0_ODSCP, 0, 6);
	RegModifyBits(UP0_3_MAP_ODSCP, DFL_UP1_ODSCP, 8, 6);
	RegModifyBits(UP0_3_MAP_ODSCP, DFL_UP2_ODSCP, 16, 6);
	RegModifyBits(UP0_3_MAP_ODSCP, DFL_UP3_ODSCP, 24, 6);
	RegModifyBits(UP4_7_MAP_ODSCP, DFL_UP4_ODSCP, 0, 6);
	RegModifyBits(UP4_7_MAP_ODSCP, DFL_UP5_ODSCP, 8, 6);
	RegModifyBits(UP4_7_MAP_ODSCP, DFL_UP6_ODSCP, 16, 6);
	RegModifyBits(UP4_7_MAP_ODSCP, DFL_UP7_ODSCP, 24, 6);

	/* Set mapping table of user priority to access category */
	RegModifyBits(UP_MAP_AC, DFL_UP0_AC, 0, 2);
	RegModifyBits(UP_MAP_AC, DFL_UP1_AC, 2, 2);
	RegModifyBits(UP_MAP_AC, DFL_UP2_AC, 4, 2);
	RegModifyBits(UP_MAP_AC, DFL_UP3_AC, 6, 2);
	RegModifyBits(UP_MAP_AC, DFL_UP4_AC, 8, 2);
	RegModifyBits(UP_MAP_AC, DFL_UP5_AC, 10, 2);
	RegModifyBits(UP_MAP_AC, DFL_UP6_AC, 12, 2);
	RegModifyBits(UP_MAP_AC, DFL_UP7_AC, 14, 2);
#endif
}

static void PpeSetHNATProtoType(void)
{
#ifndef CONFIG_RALINK_RT3052_MP2
	/* TODO: we should add exceptional case to register to point out the HNAT case here */
#endif
}

static void FoeFreeTbl(uint32_t NumOfEntry)
{
	uint32_t FoeTblSize;

	FoeTblSize = NumOfEntry * sizeof(struct FoeEntry);
	dma_free_coherent(NULL, FoeTblSize, PpeFoeBase, PpePhyFoeBase);
	RegWrite(PPE_FOE_BASE, 0);
}

static int32_t PpeEngStart(void)
{
	/* Set PPE Flow Set */
	PpeSetFoeEbl(1);

	/* Set PPE FOE Hash Mode */
	PpeSetFoeHashMode(DFL_FOE_HASH_MODE);

#if !defined (CONFIG_HNAT_V2)
	/* Set default index in policy table */
	PpeSetPreAclEbl(0);
	PpeSetPreMtrEbl(0);
	PpeSetPostMtrEbl(0);
	PpeSetPreAcEbl(0);
	PpeSetPostAcEbl(0);
#endif

	/* Set Auto Age-Out Function */
	PpeSetAgeOut();

	/* Set PPE FOE KEEPALIVE TIMER */
	PpeSetFoeKa();

	/* Set PPE FOE Bind Rate */
	PpeSetFoeBindRate(DFL_FOE_BNDR);

	/* Set PPE Global Configuration */
	PpeSetFoeGloCfgEbl(1);

	/* Set User Priority related register */
	PpeSetUserPriority();

	/* which protocol type should be handle by HNAT not HNAPT */
	PpeSetHNATProtoType();
	return 0;
}

static int32_t PpeEngStop(void)
{
	/* Set PPE FOE ENABLE */
	PpeSetFoeGloCfgEbl(0);

	/* Set PPE Flow Set */
	PpeSetFoeEbl(0);

#if !defined (CONFIG_HNAT_V2)
	/* Set default index in policy table */
	PpeSetPreAclEbl(0);
	PpeSetPreMtrEbl(0);
	PpeSetPostMtrEbl(0);
	PpeSetPreAcEbl(0);
	PpeSetPostAcEbl(0);
#endif

	/* Free FOE table */
	FoeFreeTbl(FOE_4TB_SIZ);

	return 0;
}

struct net_device *ra_dev_get_by_name(const char *name)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	return dev_get_by_name(&init_net, name);
#else
	return dev_get_by_name(name);
#endif
}

void PpeSetDstPort(uint32_t Ebl)
{
	if (Ebl) {
		DstPort[DP_RA0] = ra_dev_get_by_name("ra0");
#if defined (CONFIG_RT2860V2_AP_MBSS)
		DstPort[DP_RA1] = ra_dev_get_by_name("ra1");
#endif
#if defined (CONFIG_RT2860V2_AP_WDS)
		DstPort[DP_WDS0] = ra_dev_get_by_name("wds0");
		DstPort[DP_WDS1] = ra_dev_get_by_name("wds1");
		DstPort[DP_WDS2] = ra_dev_get_by_name("wds2");
		DstPort[DP_WDS3] = ra_dev_get_by_name("wds3");
#endif
#if defined (CONFIG_RT2860V2_AP_APCLI)
		DstPort[DP_APCLI0] = ra_dev_get_by_name("apcli0");
#endif
#if defined (CONFIG_RT2860V2_AP_MESH)
		DstPort[DP_MESH0] = ra_dev_get_by_name("mesh0");
#endif
#if defined (CONFIG_RTDEV_MII) || defined (CONFIG_RTDEV_USB) || \
    defined (CONFIG_RTDEV_PCI) || defined (CONFIG_RTDEV)
		DstPort[DP_RAI0] = ra_dev_get_by_name("rai0");
#if defined (CONFIG_RT3090_AP_MBSS) || defined (CONFIG_RT5392_AP_MBSS) || \
    defined (CONFIG_RT3572_AP_MBSS) || defined (CONFIG_RT5572_AP_MBSS) || \
    defined (CONFIG_RT5592_AP_MBSS) || defined (CONFIG_RT3593_AP_MBSS)
		DstPort[DP_RAI1] = ra_dev_get_by_name("rai1");
#endif // CONFIG_RTDEV_AP_MBSS //
#endif // CONFIG_RTDEV_MII || CONFIG_RTDEV_USB || CONFIG_RTDEV_PCI
#if defined (CONFIG_RT3090_AP_APCLI) || defined (CONFIG_RT5392_AP_APCLI) || \
    defined (CONFIG_RT3572_AP_APCLI) || defined (CONFIG_RT5572_AP_APCLI) || \
    defined (CONFIG_RT5592_AP_APCLI) || defined (CONFIG_RT3593_AP_APCLI)	
		DstPort[DP_APCLII0] = ra_dev_get_by_name("apclii0");
#endif // CONFIG_RTDEV_AP_APCLI //
#if defined (CONFIG_RT3090_AP_MESH) || defined (CONFIG_RT5392_AP_MESH) || \
    defined (CONFIG_RT3572_AP_MESH) || defined (CONFIG_RT5572_AP_MESH) || \
    defined (CONFIG_RT5592_AP_MESH) || defined (CONFIG_RT3593_AP_MESH)
		DstPort[DP_MESHI0] = ra_dev_get_by_name("meshi0");
#endif // CONFIG_RTDEV_AP_MESH //
		DstPort[DP_GMAC] = ra_dev_get_by_name("eth2");
#ifdef CONFIG_RAETH_GMAC2
		DstPort[DP_GMAC2] = ra_dev_get_by_name("eth3");
#endif
#ifdef CONFIG_RA_HW_NAT_PCI
		DstPort[DP_PCI] = ra_dev_get_by_name("eth0");	// PCI interface name
#endif
	} else {
		if (DstPort[DP_RA0] != NULL) {
			dev_put(DstPort[DP_RA0]);
		}
#if defined (CONFIG_RT2860V2_AP_MBSS)
		if (DstPort[DP_RA1] != NULL) {
			dev_put(DstPort[DP_RA1]);
		}
#endif
#if defined (CONFIG_RT2860V2_AP_WDS)
		if (DstPort[DP_WDS0] != NULL) {
			dev_put(DstPort[DP_WDS0]);
		}
		if (DstPort[DP_WDS1] != NULL) {
			dev_put(DstPort[DP_WDS1]);
		}
		if (DstPort[DP_WDS2] != NULL) {
			dev_put(DstPort[DP_WDS2]);
		}
		if (DstPort[DP_WDS3] != NULL) {
			dev_put(DstPort[DP_WDS3]);
		}
#endif
#if defined (CONFIG_RT2860V2_AP_APCLI)
		if (DstPort[DP_APCLI0] != NULL) {
			dev_put(DstPort[DP_APCLI0]);
		}
#endif
#if defined (CONFIG_RT2860V2_AP_MESH)
		if (DstPort[DP_MESH0] != NULL) {
			dev_put(DstPort[DP_MESH0]);
		}
#endif
#if defined (CONFIG_RTDEV_MII) || defined (CONFIG_RTDEV_USB) || \
    defined (CONFIG_RTDEV_PCI) || defined (CONFIG_RTDEV)
		if (DstPort[DP_RAI0] != NULL) {
			dev_put(DstPort[DP_RAI0]);
		}
#if defined (CONFIG_RT3090_AP_MBSS) || defined (CONFIG_RT5392_AP_MBSS) || \
    defined (CONFIG_RT3572_AP_MBSS) || defined (CONFIG_RT5572_AP_MBSS) || \
    defined (CONFIG_RT5592_AP_MBSS) || defined (CONFIG_RT3593_AP_MBSS)
		if (DstPort[DP_RAI1] != NULL) {
			dev_put(DstPort[DP_RAI1]);
		}
#endif // CONFIG_RTDEV_AP_MBSS //
#endif // CONFIG_RTDEV_MII || CONFIG_RTDEV_USB || CONFIG_RTDEV_PCI
#if defined (CONFIG_RT3090_AP_APCLI) || defined (CONFIG_RT5392_AP_APCLI) || \
    defined (CONFIG_RT3572_AP_APCLI) || defined (CONFIG_RT5572_AP_APCLI) || \
    defined (CONFIG_RT5592_AP_APCLI) || defined (CONFIG_RT3593_AP_APCLI)
		if (DstPort[DP_APCLII0] != NULL) {
			dev_put(DstPort[DP_APCLII0]);
		}
#endif // CONFIG_RTDEV_AP_APCLI //
#if defined (CONFIG_RT3090_AP_MESH) || defined (CONFIG_RT5392_AP_MESH) || \
    defined (CONFIG_RT3572_AP_MESH) || defined (CONFIG_RT5572_AP_MESH) || \
    defined (CONFIG_RT5592_AP_MESH) || defined (CONFIG_RT3593_AP_MESH)
		if (DstPort[DP_MESHI0] != NULL) {
			dev_put(DstPort[DP_MESHI0]);
		}
#endif // CONFIG_RTDEV_AP_MESH //
		if (DstPort[DP_GMAC] != NULL) {
			dev_put(DstPort[DP_GMAC]);
		}
#ifdef CONFIG_RAETH_GMAC2
		if (DstPort[DP_GMAC2] != NULL) {
			dev_put(DstPort[DP_GMAC2]);
		}
#endif
#ifdef CONFIG_RA_HW_NAT_PCI
		if (DstPort[DP_PCI] != NULL) {
			dev_put(DstPort[DP_PCI]);
		}
#endif
	}
}

uint32_t SetGdmaFwd(uint32_t Ebl)
{
#if defined (CONFIG_HNAT_V2)
	uint32_t data = 0;

	data = RegRead(GDM2_FWD_CFG);

	if (Ebl) {
		data &= ~0x7777;
		data |=  GDM1_OFRC_P_CPU;
		data |=  GDM1_MFRC_P_CPU;
		data |=  GDM1_BFRC_P_CPU;
		data |=  GDM1_UFRC_P_CPU;
	}else {
		data |= 0x7777;
	}

	RegWrite(GDM2_FWD_CFG, data);
#else
	uint32_t data = 0;

	data = RegRead(FE_GDMA1_FWD_CFG);

	if (Ebl) {
		//Uni-cast frames forward to PPE
		data |= GDM1_UFRC_P_PPE;
		//Broad-cast MAC address frames forward to PPE
		data |= GDM1_BFRC_P_PPE;
		//Multi-cast MAC address frames forward to PPE
		data |= GDM1_MFRC_P_PPE;
		//Other MAC address frames forward to PPE
		data |= GDM1_OFRC_P_PPE;

	} else {
		//Uni-cast frames forward to CPU
		data &= ~GDM1_UFRC_P_PPE;
		//Broad-cast MAC address frames forward to CPU
		data &= ~GDM1_BFRC_P_PPE;
		//Multi-cast MAC address frames forward to CPU
		data &= ~GDM1_MFRC_P_PPE;
		//Other MAC address frames forward to CPU
		data &= ~GDM1_OFRC_P_PPE;

	}

	RegWrite(FE_GDMA1_FWD_CFG, data);

#ifdef CONFIG_RAETH_GMAC2
	data = RegRead(FE_GDMA2_FWD_CFG);

	if (Ebl) {
		//Uni-cast frames forward to PPE
		data |= GDM1_UFRC_P_PPE;
		//Broad-cast MAC address frames forward to PPE
		data |= GDM1_BFRC_P_PPE;
		//Multi-cast MAC address frames forward to PPE
		data |= GDM1_MFRC_P_PPE;
		//Other MAC address frames forward to PPE
		data |= GDM1_OFRC_P_PPE;

	} else {
		//Uni-cast frames forward to CPU
		data &= ~GDM1_UFRC_P_PPE;
		//Broad-cast MAC address frames forward to CPU
		data &= ~GDM1_BFRC_P_PPE;
		//Multi-cast MAC address frames forward to CPU
		data &= ~GDM1_MFRC_P_PPE;
		//Other MAC address frames forward to CPU
		data &= ~GDM1_OFRC_P_PPE;

	}
	RegWrite(FE_GDMA2_FWD_CFG, data);
#endif
#endif

	return 0;
}

#if defined (CONFIG_HNAT_V2)
static void PpeSetCacheEbl(void)
{
	/* Cache enable */
	RegModifyBits(CAH_CTRL, 1, 0, 1);
}

static void PpeSetFpBMAP(void)
{
	/* index 0 = force port 0 
	 * index 1 = force port 1 
	 * ...........
	 * index 7 = force port 7
	 * index 8 = no force port 
	 * index 9 = force to all ports
	 */
	RegWrite(PPE_FP_BMAP_0, 0x00020001);
	RegWrite(PPE_FP_BMAP_1, 0x00080004);
	RegWrite(PPE_FP_BMAP_2, 0x00200010);
	RegWrite(PPE_FP_BMAP_3, 0x00800040);
	RegWrite(PPE_FP_BMAP_4, 0x003F0000);
}

static void PpeSetIpProt(void)
{
	/* IP Protocol Field for IPv4 NAT or IPv6 3-tuple flow */
	/* Don't forget to turn on related bits in PPE_IP_PROT_CHK register if you want to support 
	 * another IP protocol. 
	 */
	/* FIXME: enable it to support IP fragement */
	RegWrite(PPE_IP_PROT_CHK, 0xFFFFFFFF); //IPV4_NXTH_CHK and IPV6_NXTH_CHK
	RegModifyBits(PPE_IP_PROT_0, IPPROTO_GRE, 0, 8);
//	RegModifyBits(PPE_IP_PROT_0, IPPROTO_TCP, 8, 8);
//	RegModifyBits(PPE_IP_PROT_0, IPPROTO_UDP, 16, 8);
//	RegModifyBits(PPE_IP_PROT_0, IPPROTO_IPV6, 24, 8);
}
#endif

/*
 * PPE Enabled: GMAC<->PPE<->CPU
 * PPE Disabled: GMAC<->CPU
 */
static int32_t PpeInitMod(void)
{
	NAT_PRINT("Ralink HW NAT Module Enabled\n");

	//Get net_device structure of Dest Port 
	PpeSetDstPort(1);

	/* Register ioctl handler */
	PpeRegIoctlHandler();

#if !defined (CONFIG_HNAT_V2)
	PpeSetRuleSize(PRE_ACL_SIZE, PRE_MTR_SIZE, PRE_AC_SIZE,
		       POST_MTR_SIZE, POST_AC_SIZE);
	AclRegIoctlHandler();
	AcRegIoctlHandler();
	MtrRegIoctlHandler();
#else
	PpeSetFpBMAP();
	PpeSetIpProt();
	PpeSetCacheEbl();

	/* 0~63 Accounting group */
	PpeSetAGInfo(1, 1);	//AG Index1=VLAN1
	PpeSetAGInfo(2, 2);	//AG Index2=VLAN2

	/* 0~63 Metering group */
	PpeSetMtrByteInfo(1, 500, 3); //TokenRate=500=500KB/s, MaxBkSize= 3 (32K-1B)
	PpeSetMtrPktInfo(2, 1, 3);  //100 pkts/sec, MaxBkSize=3 (32K-1B)
#endif


	/* Initialize PPE related register */
	PpeEngStart();

	/* In manual mode, PPE always reports UN-HIT CPU reason, so we don't need to process it */
	/* Register RX/TX hook point */
	ra_sw_nat_hook_tx = PpeTxHandler;
	ra_sw_nat_hook_rx = PpeRxHandler;
	ra_sw_nat_hook_rs = PpeSetDstPort;

	/* Set GMAC fowrards packet to PPE */
	SetGdmaFwd(1);

#if 0				//we cannot use GPL-only symbol
	hnat_class = class_create(THIS_MODULE, "hnat");
	class_device_create(hnat_class, NULL, MKDEV(220, 0), NULL, "hwnat0");
	class_device_create(hnat_class, NULL, MKDEV(230, 0), NULL, "acl0");
	class_device_create(hnat_class, NULL, MKDEV(240, 0), NULL, "ac0");
	class_device_create(hnat_class, NULL, MKDEV(250, 0), NULL, "mtr0");
#endif

	return 0;
}

static void PpeCleanupMod(void)
{
	NAT_PRINT("Ralink HW NAT Module Disabled\n");

	/* Set GMAC fowrards packet to CPU */
	SetGdmaFwd(0);

	/* Unregister RX/TX hook point */
	ra_sw_nat_hook_rx = NULL;
	ra_sw_nat_hook_tx = NULL;
	ra_sw_nat_hook_rs = NULL;

	/* Restore PPE related register */
	PpeEngStop();

	/* Unregister ioctl handler */
	PpeUnRegIoctlHandler();
#if !defined (CONFIG_HNAT_V2)
	AclUnRegIoctlHandler();
	AcUnRegIoctlHandler();
	MtrUnRegIoctlHandler();
#endif

	//Release net_device structure of Dest Port 
	PpeSetDstPort(0);

#if 0
	/* Restore switch port 6 flow control to default on */
	RegModifyBits(RALINK_ETH_SW_BASE + 0xC8, 0x3, 8, 2);
#endif

#if 0	//we cannot use GPL-only symbol
	class_device_destroy(hnat_class, MKDEV(220, 0));
	class_device_destroy(hnat_class, MKDEV(230, 0));
	class_device_destroy(hnat_class, MKDEV(240, 0));
	class_device_destroy(hnat_class, MKDEV(250, 0));
	class_destroy(hnat_class);
#endif
}

module_init(PpeInitMod);
module_exit(PpeCleanupMod);

MODULE_AUTHOR("Steven Liu/Kurtis Ke");
MODULE_LICENSE("Proprietary");
MODULE_DESCRIPTION("Ralink Hardware NAT v2.01\n");
