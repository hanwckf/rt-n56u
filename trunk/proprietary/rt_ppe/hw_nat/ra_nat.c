/*
  Module Name:
  ra_nat.c

  Abstract:

  Revision History:
  Who         When            What
  --------    ----------      ----------------------------------------------
  Name        Date            Modification logs
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

#include "acl_ioctl.h"
#include "ac_ioctl.h"
#include "acl_policy.h"
#include "mtr_policy.h"
#include "ac_policy.h"

#if defined (CONFIG_RA_HW_NAT_WIFI) || defined (CONFIG_RA_HW_NAT_PCI)
static int wifi_offload __read_mostly = 0;
module_param(wifi_offload, int, S_IRUGO);
MODULE_PARM_DESC(wifi_offload, "Enable/Disable wifi/extif PPE NAT offload.");
#endif

extern int (*ra_sw_nat_hook_rx) (struct sk_buff * skb);
extern int (*ra_sw_nat_hook_tx) (struct sk_buff * skb, int gmac_no);
extern int (*ra_sw_nat_hook_rs) (struct net_device *dev, int hold);

extern uint32_t		DebugLevel;
extern uint16_t		wan_vid;
extern uint16_t		lan_vid;
extern int		udp_offload;
#if defined(CONFIG_RA_HW_NAT_IPV6)
extern int		ipv6_offload;
#endif
static int		ppe_udp_bug = 1;


struct FoeEntry		*PpeFoeBase = NULL;
uint32_t		PpeFoeTblSize = FOE_4TB_SIZ;
struct net_device	*DstPort[MAX_IF_NUM];

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

#ifdef HWNAT_DEBUG
static uint8_t *ShowCpuReason(struct sk_buff *skb)
{
	static uint8_t Buf[32];

	switch (FOE_AI(skb)) {
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
		NAT_PRINT("Information Block 2=%x\n", foe_entry->ipv4_hnapt.info_blk2);
	}
	else if (IS_IPV6_1T_ROUTE(foe_entry)) {
		NAT_PRINT("Information Block 1=%x\n", foe_entry->ipv6_1t_route.info_blk1);
		NAT_PRINT("Destination IPv6: %08X:%08X:%08X:%08X",
			  foe_entry->ipv6_1t_route.ipv6_dip3, foe_entry->ipv6_1t_route.ipv6_dip2,
			  foe_entry->ipv6_1t_route.ipv6_dip1, foe_entry->ipv6_1t_route.ipv6_dip0);
		NAT_PRINT("Information Block 2=%x\n", foe_entry->ipv6_1t_route.info_blk2);
	}
	else {
		NAT_PRINT("unknown Pkt_type=%d\n", foe_entry->bfib1.pkt_type);
	}

	NAT_PRINT("==================================\n");

	return 1;

}
#endif

#if defined  (CONFIG_RA_HW_NAT_WIFI) || defined (CONFIG_RA_HW_NAT_PCI)
/* push different VID for WiFi pseudo interface or USB external NIC */
uint32_t PpeExtIfRxHandler(struct sk_buff * skb)
{
	uint16_t VirIfIdx = 0;
	uint16_t eth_type;

#if defined(HWNAT_MCAST_BCAST_PPE)
	/* offload multicast/broadcast is not supported for extif */
	if (skb->pkt_type == PACKET_MULTICAST || skb->pkt_type == PACKET_BROADCAST)
		return 1;
#endif

	eth_type = ntohs(skb->protocol);

	/* offload packet types not support for extif:
	   1. VLAN tagged packets (avoid double tag issue).
	   2. PPPoE packets (PPPoE passthrough issue).
	   3. IPv6 1T routes. */
	if (eth_type != ETH_P_IP)
		return 1;

	/* check dst interface exist */
	if (skb->dev == NULL) {
#ifdef HWNAT_DEBUG
		NAT_PRINT("HNAT: RX: interface not exist, drop this packet.\n");
#endif
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
#if defined (HWNAT_MBSS_ALL)
	else if (skb->dev == DstPort[DP_RA2]) {
		VirIfIdx = DP_RA2;
	} else if (skb->dev == DstPort[DP_RA3]) {
		VirIfIdx = DP_RA3;
	} else if (skb->dev == DstPort[DP_RA4]) {
		VirIfIdx = DP_RA4;
	} else if (skb->dev == DstPort[DP_RA5]) {
		VirIfIdx = DP_RA5;
	} else if (skb->dev == DstPort[DP_RA6]) {
		VirIfIdx = DP_RA6;
	} else if (skb->dev == DstPort[DP_RA7]) {
		VirIfIdx = DP_RA7;
	}
#endif
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
#if defined (CONFIG_RTDEV_USB) || defined (CONFIG_RTDEV_PCI)
	else if (skb->dev == DstPort[DP_RAI0]) {
		VirIfIdx = DP_RAI0;
	}
#if defined (CONFIG_RT3090_AP_MBSS) || defined (CONFIG_RT5392_AP_MBSS) || \
    defined (CONFIG_RT3572_AP_MBSS) || defined (CONFIG_RT5572_AP_MBSS) || \
    defined (CONFIG_RT5592_AP_MBSS) || defined (CONFIG_RT3593_AP_MBSS)
	else if (skb->dev == DstPort[DP_RAI1]) {
		VirIfIdx = DP_RAI1;
	}
#if defined (HWNAT_MBSS_ALL)
	else if (skb->dev == DstPort[DP_RAI2]) {
		VirIfIdx = DP_RAI2;
	} else if (skb->dev == DstPort[DP_RAI3]) {
		VirIfIdx = DP_RAI3;
	} else if (skb->dev == DstPort[DP_RAI4]) {
		VirIfIdx = DP_RAI4;
	} else if (skb->dev == DstPort[DP_RAI5]) {
		VirIfIdx = DP_RAI5;
	} else if (skb->dev == DstPort[DP_RAI6]) {
		VirIfIdx = DP_RAI6;
	} else if (skb->dev == DstPort[DP_RAI7]) {
		VirIfIdx = DP_RAI7;
	}
#endif
#endif // CONFIG_RTDEV_AP_MBSS //
#endif // CONFIG_RTDEV_USB || CONFIG_RTDEV_PCI
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
	else if (skb->dev == DstPort[DP_PCI0]) {
		VirIfIdx = DP_PCI0;
	}
	else if (skb->dev == DstPort[DP_PCI1]) {
		VirIfIdx = DP_PCI1;
	}
#endif
	else {
#ifdef HWNAT_DEBUG
		NAT_PRINT("HNAT: The interface %s is unknown\n", skb->dev->name);
#endif
		return 1;
	}

	/* push vlan tag to stand for actual incoming interface,
	    so HNAT module can know the actual incoming interface from vlan id. */
	skb_reset_network_header(skb);
	skb_push(skb, ETH_HLEN);	//pointer to layer2 header before calling hard_start_xmit
	skb = __vlan_put_tag(skb, VirIfIdx);
	if (unlikely(!skb)) {
		NAT_PRINT("HNAT: not valid tag ? memleak ? (VirIfIdx=%d)\n", VirIfIdx);
		return 0;
	}

	/* redirect to PPE (check this in raeth) */
	FOE_MAGIC_TAG(skb) = FOE_MAGIC_PPE;
	DO_FILL_FOE_DESC(skb, 0);
	skb->dev = DstPort[DP_GMAC1];	//we use GMAC1 to send the packet to PPE
	dev_queue_xmit(skb);
	
	return 0;
}

uint32_t PpeExtIfPingPongHandler(struct sk_buff * skb)
{
	struct ethhdr *eth = NULL;
	uint16_t VirIfIdx;
	struct net_device *dev;
	struct vlan_ethhdr *veth;

	/* something wrong: proto must be 802.11q, interface index must be < MAX_IF_NUM and exist, 
		don`t touch this packets and return to normal path before corrupt in detag code
	*/
	if (skb->protocol != htons(ETH_P_8021Q))
		return 1;

	veth = (struct vlan_ethhdr *)LAYER2_HEADER(skb);

	VirIfIdx = ntohs(veth->h_vlan_TCI);
	if (VirIfIdx >= MAX_IF_NUM)
		return 1;

	dev = DstPort[VirIfIdx];
	if (!dev) {
#ifdef HWNAT_DEBUG
		NAT_PRINT("HNAT: Reentry packet interface (VirIfIdx=%d) not exist. Skip this packet!\n", VirIfIdx);
#endif
		return 1;
	}

	/* make skb writable */
	if (!skb_make_writable(skb, 0)) {
		NAT_PRINT("HNAT: No mem for remove tag or corrupted packet? (VirIfIdx=%d)\n", VirIfIdx);
		return 1;
	}

	/* remove vlan tag from current packet */
	skb->data = LAYER2_HEADER(skb);
	LAYER2_HEADER(skb) += VLAN_HLEN;
	memmove(LAYER2_HEADER(skb), skb->data, ETH_ALEN * 2);
	skb_pull(skb, VLAN_HLEN);
	skb->data += ETH_HLEN;	/* pointer to layer3 header */

	/* set correct skb protocol */
	eth = (struct ethhdr *)LAYER2_HEADER(skb);
	skb->protocol = eth->h_proto;

	/* set correct skb dev */
	skb->dev = dev;

	/* set correct skb pkt_type
	   note: eth_type_trans is already completed for GMAC1 (dev eth2),
	   so check only if pkt_type PACKET_OTHERHOST */
	if (skb->pkt_type == PACKET_OTHERHOST) {
		if (!compare_ether_addr(eth->h_dest, dev->dev_addr))
			skb->pkt_type = PACKET_HOST;
#ifdef CONFIG_RAETH_GMAC2
		else if (DstPort[DP_GMAC2] && !compare_ether_addr(eth->h_dest, DstPort[DP_GMAC2]->dev_addr))
			skb->pkt_type = PACKET_HOST;
#endif
	}

	return 1;
}
#endif

uint32_t PpeKeepAliveHandler(struct sk_buff* skb, struct FoeEntry* foe_entry)
{
	struct ethhdr *eth = (struct ethhdr *)(skb->data - ETH_HLEN);
	uint16_t eth_type = ntohs(skb->protocol);
	uint32_t vlan1_gap = 0;
	uint32_t vlan2_gap = 0;
	uint32_t pppoe_gap = 0;
	
	/*
	 * try to recover to original SMAC/DMAC, but we don't have such information.
	 * just use SMAC as DMAC and set Multicast address as SMAC.
	 */
	FoeGetMacInfo(eth->h_dest, eth->h_source);
	FoeGetMacInfo(eth->h_source, eth->h_dest);
	eth->h_source[0] = 0x1;	//change to multicast packet, make bridge not learn this packet

	if (eth_type == ETH_P_8021Q) {
		struct vlan_hdr *vh = (struct vlan_hdr *)skb->data;
		
		vlan1_gap = VLAN_HLEN;
		if (vh->h_vlan_TCI == htons(wan_vid)) {
			/* It make packet like coming from LAN port */
			vh->h_vlan_TCI = htons(lan_vid);
		} else {
			/* It make packet like coming from WAN port */
			vh->h_vlan_TCI = htons(wan_vid);
		}
		
		if (vh->h_vlan_encapsulated_proto == htons(ETH_P_PPP_SES)) {
			pppoe_gap = 8;
		} else if (vh->h_vlan_encapsulated_proto == htons(ETH_P_8021Q)) {
			vlan2_gap = VLAN_HLEN;
			vh = (struct vlan_hdr *)(skb->data + VLAN_HLEN);
			/* VLAN + VLAN + PPPoE */
			if (vh->h_vlan_encapsulated_proto == htons(ETH_P_PPP_SES)) {
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

	/* only Ipv4 NAT need KeepAlive Packet to refresh iptable */
	if (eth_type == ETH_P_IP) {
		struct iphdr *iph = (struct iphdr *)(skb->data + vlan1_gap + vlan2_gap + pppoe_gap);
		/* recover to original layer 4 header */
		if (iph->protocol == IPPROTO_TCP) {
			struct tcphdr *th = (struct tcphdr *)((uint8_t *)iph + iph->ihl * 4);
			FoeToOrgTcpHdr(foe_entry, iph, th);
		} else if (iph->protocol == IPPROTO_UDP) {
			struct udphdr *uh = (struct udphdr *)((uint8_t *)iph + iph->ihl * 4);
			if (!uh->check && ppe_udp_bug && foe_entry->ipv4_hnapt.bfib1.state == BIND) {
				/* no UDP checksum, force unbind session from PPE for workaround PPE UDP bug */
				foe_entry->ipv4_hnapt.udib1.state = UNBIND;
				foe_entry->ipv4_hnapt.udib1.time_stamp = RegRead(FOE_TS) & 0xFF;
			}
			FoeToOrgUdpHdr(foe_entry, iph, uh);
		}
		/* recover to original layer 3 header */
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

int PpeHitBindForceToCpuHandler(struct sk_buff *skb, struct FoeEntry *foe_entry)
{
	struct net_device *dev = DstPort[foe_entry->ipv4_hnapt.act_dp];
	if (!dev) {
#ifdef HWNAT_DEBUG
		NAT_PRINT("HNAT: PpeHitBindForceToCpuHandler, act_dp point to null!\n");
#endif
		return 1;
	}

	skb->dev = dev;
	skb_reset_network_header(skb);
	skb_push(skb, ETH_HLEN);	//pointer to layer2 header
	dev_queue_xmit(skb);

	return 0;
}

#if defined (CONFIG_RA_HW_NAT_ACL2UP_HELPER)
void PpeGetUpFromACLRule(struct sk_buff *skb)
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

	AclClassifyKey NewRateReach;
	eth = (struct ethhdr *)(skb->data - ETH_HLEN);

	memset(&NewRateReach, 0, sizeof(AclClassifyKey));
	memcpy(NewRateReach.Mac, eth->h_source, ETH_ALEN);
	NewRateReach.Ethertype = eth_type;	//Ethertype

	if (eth_type == ETH_P_8021Q) {
		vlan1_gap = VLAN_HLEN;
		vh = (struct vlan_hdr *)skb->data;
		NewRateReach.Vid = ntohs(vh->h_vlan_TCI);	//VID
		if (vh->h_vlan_encapsulated_proto == htons(ETH_P_PPP_SES)) {
			pppoe_gap = 8;
		} else if (vh->h_vlan_encapsulated_proto == htons(ETH_P_8021Q)) {
			vlan2_gap = VLAN_HLEN;
			vh = (struct vlan_hdr *)(skb->data + VLAN_HLEN);
			/* VLAN + VLAN + PPPoE */
			if (vh->h_vlan_encapsulated_proto == htons(ETH_P_PPP_SES)) {
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
		iph = (struct iphdr *)(skb->data + vlan1_gap + vlan2_gap + pppoe_gap);
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
	FOE_SP(skb) = AclClassify(&NewRateReach);
}
#endif

int32_t PpeRxHandler(struct sk_buff * skb)
{
	struct FoeEntry *foe_entry;

	/* return truncated packets to normal path */
	if (!skb || skb->len < ETH_HLEN) {
#ifdef HWNAT_DEBUG
		NAT_PRINT("HNAT: skb null or small len in rx path\n");
#endif
		return 1;
	}

#ifdef HWNAT_DEBUG
	if (DebugLevel >= 7) {
		FoeDumpPkt(skb);
	}
#endif

#if !defined(HWNAT_MCAST_BCAST_PPE)
	if (skb->pkt_type == PACKET_MULTICAST || skb->pkt_type == PACKET_BROADCAST)
		return 1;
#endif

	foe_entry = &PpeFoeBase[FOE_ENTRY_NUM(skb)];

	if (FOE_MAGIC_TAG(skb) == FOE_MAGIC_EXTIF) {
		/* the incoming packet is from PCI or WiFi interface */
#if defined (CONFIG_RA_HW_NAT_WIFI) || defined (CONFIG_RA_HW_NAT_PCI)
		if (wifi_offload) {
			/* add tag to pkts from external ifaces before send to PPE */
			return PpeExtIfRxHandler(skb);
		} else {
			return 1; /* wifi offload disabled */
		}
#else
		return 1; /* wifi offload not compiled */
#endif
	} else if ((FOE_AI(skb) == HIT_BIND_FORCE_TO_CPU)) {
		/* It means the flow is already in binding state, just transfer to output interface */
		return PpeHitBindForceToCpuHandler(skb, foe_entry);
	} else if (FOE_SP(skb) == 0 && (FOE_AI(skb) != HIT_BIND_KEEPALIVE)) {
		/* handle the incoming packet which came back from PPE */
#if defined (CONFIG_RA_HW_NAT_WIFI) || defined (CONFIG_RA_HW_NAT_PCI)
		if (wifi_offload) {
			/* detag pkts from external ifaces after PPE process */
			return PpeExtIfPingPongHandler(skb);
		} else {
			return 1; /* wifi offload disabled */
		}
#else
		return 1; /* wifi offload not compiled */
#endif
	} else if ((FOE_AI(skb) == HIT_BIND_KEEPALIVE) && (DFL_FOE_KA == 0)) {
		if (!FOE_ENTRY_VALID(skb)) {
#ifdef HWNAT_DEBUG
			NAT_PRINT("HNAT: hit bind keepalive is not valid FoE entry!\n");
#endif
			return 1;
		}
		if (PpeKeepAliveHandler(skb, foe_entry)) {
			return 1;
		}
	}
#if defined (CONFIG_RA_HW_NAT_ACL2UP_HELPER)
	else if ((FOE_AI(skb) == HIT_UNBIND_RATE_REACH)) {
		PpeGetUpFromACLRule(skb);
	}
#endif

	return 1;
}

#if defined (CONFIG_RA_HW_NAT_WIFI) || defined (CONFIG_RA_HW_NAT_PCI)
uint32_t PpeSetExtIfNum(struct sk_buff *skb, struct FoeEntry* foe_entry)
{
	uint32_t offset = 0;

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
#if defined (CONFIG_RTDEV_USB) || defined (CONFIG_RTDEV_PCI)
	if (strncmp(skb->dev->name, "rai", 3) == 0) {
#if defined (CONFIG_RT3090_AP_MESH) || defined (CONFIG_RT5392_AP_MESH) || \
    defined (CONFIG_RT3572_AP_MESH) || defined (CONFIG_RT5572_AP_MESH) || \
    defined (CONFIG_RT5592_AP_MESH) || defined (CONFIG_RT3593_AP_MESH)
		if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_MESH) {
			offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_MESH + DP_MESHI0);
		} else
#endif // CONFIG_RTDEV_AP_MESH //
#if defined (CONFIG_RT3090_AP_APCLI) || defined (CONFIG_RT5392_AP_APCLI) || \
    defined (CONFIG_RT3572_AP_APCLI) || defined (CONFIG_RT5572_AP_APCLI) || \
    defined (CONFIG_RT5592_AP_APCLI) || defined (CONFIG_RT3593_AP_APCLI)
		if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_APCLI) {
			offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_APCLI + DP_APCLII0);
		} else
#endif // CONFIG_RTDEV_AP_APCLI //
#if defined (CONFIG_RT3090_AP_WDS) || defined (CONFIG_RT5392_AP_WDS) || \
    defined (CONFIG_RT3572_AP_WDS) || defined (CONFIG_RT5572_AP_WDS) || \
    defined (CONFIG_RT5592_AP_WDS) || defined (CONFIG_RT3593_AP_WDS)
		if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_WDS) {
			offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_WDS + DP_WDSI0);
		} else
#endif // CONFIG_RTDEV_AP_WDS //
		{
			offset = RTMP_GET_PACKET_IF(skb) + DP_RAI0;
		}
	} else
#endif // CONFIG_RTDEV_USB || CONFIG_RTDEV_PCI

	if (strncmp(skb->dev->name, "ra", 2) == 0) {
#if defined (CONFIG_RT2860V2_AP_MESH)
		if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_MESH) {
			offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_MESH + DP_MESH0);
		} else
#endif // CONFIG_RT2860V2_AP_MESH //
#if defined (CONFIG_RT2860V2_AP_APCLI)
		if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_APCLI) {
			offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_APCLI + DP_APCLI0);
		} else
#endif // CONFIG_RT2860V2_AP_APCLI //
#if defined (CONFIG_RT2860V2_AP_WDS)
		if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_WDS) {
			offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_WDS + DP_WDS0);
		} else
#endif // CONFIG_RT2860V2_AP_WDS //
		{
			offset = RTMP_GET_PACKET_IF(skb) + DP_RA0;
		}
	}
#if defined (CONFIG_RA_HW_NAT_PCI)
	else if (skb->dev == DstPort[DP_PCI0]) {
		offset = DP_PCI0;
	}
	else if (skb->dev == DstPort[DP_PCI1]) {
		offset = DP_PCI1;
	}
#endif
	else if (skb->dev == DstPort[DP_GMAC1]) {
		offset = DP_GMAC1;
	}
#ifdef CONFIG_RAETH_GMAC2
	else if (skb->dev == DstPort[DP_GMAC2]) {
		offset = DP_GMAC2;
	}
#endif
	else {
#ifdef HWNAT_DEBUG
		NAT_PRINT("HNAT: unknow interface %s\n", skb->dev->name);
#endif
		return 1;
	}

	foe_entry->ipv4_hnapt.act_dp = offset;

	return 0;
}
#endif

int32_t FoeBindToPpe(struct sk_buff *skb, struct FoeEntry* foe_entry_ppe, int gmac_no)
{
	struct ethhdr *eth = (struct ethhdr *)skb->data;
	struct FoeEntry foe_entry;
	uint32_t vlan1_gap = 0;
	uint32_t vlan2_gap = 0;
	uint32_t pppoe_gap = 0;
	uint16_t ppp_tag = 0;
	uint16_t eth_type;

#if !defined(HWNAT_MCAST_BCAST_PPE)
	// we cannot speed up multicase packets because both wire and wireless PCs might join same multicast group.
	if(is_multicast_ether_addr(eth->h_dest))
		return 1;
#endif

	eth_type = ntohs(eth->h_proto);

	/* offload packet types not support for extif:
	   1. VLAN tagged packets (avoid double tag issue).
	   2. PPPoE packets (PPPoE passthrough issue).
	   3. IPv6 1T routes. */
	if (gmac_no == 0 && eth_type != ETH_P_IP)
		return 1;

	/* copy original FoE entry */
	memcpy(&foe_entry, foe_entry_ppe, sizeof(struct FoeEntry));

	/* if this entry is already in binding state, skip it */
	if (foe_entry.bfib1.state == BIND)
		return 1;

	/* reset L2 fields */
	foe_entry.ipv4_hnapt.vlan1 = 0;
	foe_entry.ipv4_hnapt.vlan2 = 0;
	foe_entry.ipv4_hnapt.pppoe_id = 0;

	/* PPPoE + IP (RT3883 with 2xGMAC) */
#if defined (CONFIG_RAETH_GMAC2)
	if (eth_type == ETH_P_PPP_SES) {
		struct pppoe_hdr *peh = (struct pppoe_hdr *)(skb->data + ETH_HLEN);
		if (peh->ver != 1 || peh->type != 1)
			return 1;
		pppoe_gap = 8;
		ppp_tag = ntohs(peh->tag[0].tag_type);
		foe_entry.ipv4_hnapt.pppoe_id = ntohs(peh->sid);
	}
	else
#endif
	if (eth_type == ETH_P_8021Q || vlan_tx_tag_present(skb)) {
		struct vlan_hdr *vh;
		
		if (eth_type == ETH_P_8021Q) {
			vlan1_gap = VLAN_HLEN;
			vh = (struct vlan_hdr *)(skb->data + ETH_HLEN);
			foe_entry.ipv4_hnapt.vlan1 = ntohs(vh->h_vlan_TCI);
			eth_type = ntohs(vh->h_vlan_encapsulated_proto);
		} else {
			foe_entry.ipv4_hnapt.vlan1 = vlan_tx_tag_get(skb);
		}
		
		/* VLAN + PPPoE */
		if (eth_type == ETH_P_PPP_SES) {
			struct pppoe_hdr *peh = (struct pppoe_hdr *)(skb->data + ETH_HLEN + vlan1_gap);
			if (peh->ver != 1 || peh->type != 1)
				return 1;
			pppoe_gap = 8;
			ppp_tag = ntohs(peh->tag[0].tag_type);
			foe_entry.ipv4_hnapt.pppoe_id = ntohs(peh->sid);
			
			/* Double VLAN = VLAN + VLAN */
		} else if (eth_type == ETH_P_8021Q) {
			vlan2_gap = VLAN_HLEN;
			vh = (struct vlan_hdr *)(skb->data + vlan1_gap + VLAN_HLEN);
			foe_entry.ipv4_hnapt.vlan2 = ntohs(vh->h_vlan_TCI);
			eth_type = ntohs(vh->h_vlan_encapsulated_proto);
			
			/* VLAN + VLAN + PPPoE */
			if (eth_type == ETH_P_PPP_SES) {
				struct pppoe_hdr *peh = (struct pppoe_hdr *)(skb->data + ETH_HLEN + vlan1_gap + VLAN_HLEN);
				if (peh->ver != 1 || peh->type != 1)
					return 1;
				pppoe_gap = 8;
				ppp_tag = ntohs(peh->tag[0].tag_type);
				foe_entry.ipv4_hnapt.pppoe_id = ntohs(peh->sid);
			}
		}
	}

	if ((eth_type == ETH_P_IP) ||
	    (eth_type == ETH_P_PPP_SES && ppp_tag == PPP_IP)) {
		struct iphdr *iph = (struct iphdr *)(skb->data + ETH_HLEN + vlan1_gap + vlan2_gap + pppoe_gap);
		if(iph->frag_off & htons(IP_MF|IP_OFFSET))
			return 1;
		foe_entry.ipv4_hnapt.new_sip = ntohl(iph->saddr);
		foe_entry.ipv4_hnapt.new_dip = ntohl(iph->daddr);
		foe_entry.ipv4_hnapt.iblk2.dscp = iph->tos;
		if (iph->protocol == IPPROTO_TCP) {
			struct tcphdr *th = (struct tcphdr *)((uint8_t *)iph + iph->ihl * 4);
			foe_entry.ipv4_hnapt.new_sport = ntohs(th->source);
			foe_entry.ipv4_hnapt.new_dport = ntohs(th->dest);
			foe_entry.ipv4_hnapt.bfib1.udp = TCP;
		} else if (iph->protocol == IPPROTO_UDP) {
			struct udphdr *uh;
			if (!udp_offload)
				return 1;
			uh = (struct udphdr *)((uint8_t *)iph + iph->ihl * 4);
			/* check PPE bug */
			if (!uh->check && ppe_udp_bug)
				return 1;
			foe_entry.ipv4_hnapt.new_sport = ntohs(uh->source);
			foe_entry.ipv4_hnapt.new_dport = ntohs(uh->dest);
			foe_entry.ipv4_hnapt.bfib1.udp = UDP;
		} else {
			/* packet format is not supported */
			return 1;
		}
#if defined (CONFIG_RA_HW_NAT_IPV6)
	} else if (eth_type == ETH_P_IPV6 ||
		  (eth_type == ETH_P_PPP_SES && ppp_tag == PPP_IPV6)) {
		if (!ipv6_offload)
			return 1;
#endif
	} else {
		return 1;
	}

	/******************** L2 ********************/

	/* Set MAC Info */
	FoeSetMacInfo(foe_entry.ipv4_hnapt.dmac_hi, eth->h_dest);
	FoeSetMacInfo(foe_entry.ipv4_hnapt.smac_hi, eth->h_source);

	/*
	 * PPE support SMART VLAN/PPPoE Tag Push/PoP feature
	 *
	 *         | MODIFY | INSERT | DELETE
	 * --------+--------+--------+----------
	 * Tagged  | modify | modify | delete
	 * Untagged| no act | insert | no act
	 *
	 */

	if (vlan1_gap || foe_entry.ipv4_hnapt.vlan1 > 0) // check HW_VLAN_TX
		foe_entry.bfib1.v1 = INSERT;
	else
		foe_entry.bfib1.v1 = DELETE;

	if (vlan2_gap)
		foe_entry.bfib1.v2 = INSERT;
	 else
		foe_entry.bfib1.v2 = DELETE;

	if (pppoe_gap)
		foe_entry.bfib1.pppoe = INSERT;
	else
		foe_entry.bfib1.pppoe = DELETE;

	/******************** DST ********************/

	foe_entry.ipv4_hnapt.iblk2.fd = 1;
	
#if defined (CONFIG_RA_HW_NAT_WIFI) || defined (CONFIG_RA_HW_NAT_PCI)
	/* CPU need to handle traffic between WLAN/PCI and GMAC port */
	if (gmac_no == 0) {
		foe_entry.ipv4_hnapt.iblk2.dp = 0;	/* -> CPU */
		/* set Pseudo Interface info in Foe entry */
		if (PpeSetExtIfNum(skb, &foe_entry))
			return 1;
	} else
#endif
	{
#if defined (CONFIG_RAETH_GMAC2)
		/* RT3883 with 2xGMAC - assuming GMAC2=WAN and GMAC1=LAN */
		if (gmac_no == 2)
			foe_entry.ipv4_hnapt.iblk2.dp = 2;	/* -> GMAC 2 */
		else
			foe_entry.ipv4_hnapt.iblk2.dp = 1;	/* -> GMAC 1 */
#elif defined (CONFIG_RALINK_RT2880) || defined (CONFIG_RALINK_RT3883)
		/* RT2880, RT3883 (1xGMAC mode), always send to GMAC1 */
		foe_entry.ipv4_hnapt.iblk2.dp = 1;	/* -> GMAC 1 */
#else
		/*  RT3052, RT335x */
		if ((foe_entry.ipv4_hnapt.vlan1 & VLAN_VID_MASK) == lan_vid)
			foe_entry.ipv4_hnapt.iblk2.dp = 1;	/* -> VirtualPort1 in GMAC1 */
		else if ((foe_entry.ipv4_hnapt.vlan1 & VLAN_VID_MASK) == wan_vid)
			foe_entry.ipv4_hnapt.iblk2.dp = 2;	/* -> VirtualPort2 in GMAC1 */
		else
			foe_entry.ipv4_hnapt.iblk2.dp = 1;	/* for one arm NAT test -> no vlan tag */
#endif
		foe_entry.ipv4_hnapt.act_dp = 0;	/* clear destination port for CPU */
	}

	/******************** BIND ********************/

	/* set current time to time_stamp field in information block 1 */
	foe_entry.bfib1.time_stamp = (uint16_t) (RegRead(FOE_TS) & 0xFFFF);

	/* Ipv4: TTL / Ipv6: Hop Limit filed */
	foe_entry.ipv4_hnapt.bfib1.ttl = DFL_FOE_TTL_REGEN;

#if defined (CONFIG_RA_HW_NAT_ACL2UP_HELPER)
	/* set user priority */
	foe_entry.ipv4_hnapt.iblk2.up = FOE_SP(skb);
	foe_entry.ipv4_hnapt.iblk2.fp = 1;
#endif

	/* change Foe Entry State to Binding State */
	foe_entry.bfib1.state = BIND;

	/* write entry to FoE table */
	memcpy(foe_entry_ppe, &foe_entry, sizeof(struct FoeEntry));
	dma_cache_sync(NULL, foe_entry_ppe, sizeof(struct FoeEntry), DMA_TO_DEVICE);

#ifdef HWNAT_DEBUG
	/* Dump Binding Entry */
	if (DebugLevel >= 2) {
		FoeDumpEntry(FOE_ENTRY_NUM(skb));
	}
#endif

	return 0;
}

int32_t PpeTxHandler(struct sk_buff *skb, int gmac_no)
{
	struct FoeEntry *foe_entry;

	/* check traffic from WiFi/ExtIf (gmac_no = 0) */
	if (gmac_no == 0) {
#if defined (CONFIG_RA_HW_NAT_WIFI) || defined (CONFIG_RA_HW_NAT_PCI)
		if (!wifi_offload)
			return 1;
#else
		return 1;
#endif
	}

	/* return truncated packets to normal path with padding */
	if (!skb || skb->len < ETH_HLEN)
		return 1;

	/* check FoE packet tag */
	if (!IS_SPACE_AVAILABLED(skb) || !IS_MAGIC_TAG_VALID(skb))
		return 1;

	/* check FoE AI for local traffic */
	if (FOE_AI(skb) == UN_HIT)
		return 1;

	foe_entry = &PpeFoeBase[FOE_ENTRY_NUM(skb)];

	/*
	 * Packet is interested by ALG?
	 * Yes: Don't enter bindind state
	 * No: If flow rate exceed binding threshold, enter binding state.
	 *     IPV6_1T require binding for all time (binding threshold=0).
	 */
	if (FOE_ALG(skb) == 0 && ((FOE_AI(skb) == HIT_UNBIND_RATE_REACH)
#if defined (CONFIG_RA_HW_NAT_IPV6)
			       || (FOE_AI(skb) == HIT_UNBIND && IS_IPV6_1T_ROUTE(foe_entry))
#endif
	  )) {
		if (FoeBindToPpe(skb, foe_entry, gmac_no)) {
			if (gmac_no != 0)
				FOE_AI(skb) = UN_HIT;
			return 1;
		}
		
	} else if ((FOE_AI(skb) == HIT_BIND_KEEPALIVE) && (DFL_FOE_KA == 0)) {
		/* check duplicate packet in keepalive new header mode, just drop it */
		FOE_AI(skb) = UN_HIT;
		return 0;
#ifdef HWNAT_DEBUG
	} else if (FOE_AI(skb) == HIT_UNBIND_RATE_REACH && FOE_ALG(skb) == 1) {
		if (DebugLevel >= 2) {
			NAT_PRINT ("FOE_ALG=1 (Entry=%d)\n", FOE_ENTRY_NUM(skb));
		}
#endif
	}

	return 1;
}

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

void PpeSetFoeEbl(uint32_t FoeEbl)
{
	uint32_t PpeFlowSet = 0;

	PpeFlowSet = RegRead(PPE_FLOW_SET);

	/* FOE engine need to handle unicast/multicast/broadcast flow */
	if (FoeEbl == 1) {
		PpeFlowSet |= (BIT_IPV4_NAPT_EN | BIT_IPV4_NAT_EN);
		PpeFlowSet |= (BIT_FUC_FOE);
#if defined(HWNAT_MCAST_BCAST_PPE)
		PpeFlowSet |= (BIT_FMC_FOE | BIT_FBC_FOE);
#endif
#if defined(CONFIG_RA_HW_NAT_IPV6)
		if (ipv6_offload)
			PpeFlowSet |= (BIT_IPV6_FOE_EN);
#endif
	} else {
		PpeFlowSet &= ~(BIT_IPV4_NAPT_EN | BIT_IPV4_NAT_EN);
		PpeFlowSet &= ~(BIT_FUC_FOE | BIT_FMC_FOE | BIT_FBC_FOE);
		PpeFlowSet &= ~(BIT_IPV6_FOE_EN);
	}

	RegWrite(PPE_FLOW_SET, PpeFlowSet);
}

static int PpeSetFoeHashMode(uint32_t HashMode)
{
	dma_addr_t PpeFoeBasePhy = 0;

	/* Get allocated FoE table from raeth */
	PpeFoeBase = get_foe_table(&PpeFoeBasePhy, &PpeFoeTblSize);
	if (!PpeFoeBase)
		return -ENOMEM;

	memset(PpeFoeBase, 0, PpeFoeTblSize * sizeof(struct FoeEntry));

	RegWrite(PPE_FOE_BASE, PpeFoeBasePhy);

	switch (PpeFoeTblSize) {
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
	RegModifyBits(PPE_FOE_CFG, HashMode, 3, 1);

	/* Set action for FOE search miss */
	RegModifyBits(PPE_FOE_CFG, FWD_CPU_BUILD_ENTRY, 4, 2);

	return 0;
}

static void PpeSetAgeOut(void)
{
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

	/* set Delta time for aging out an bind TCP FIN FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE1, DFL_FOE_FIN_DLTA, 16, 16);

	/* set Delta time for aging out an bind TCP FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE1, DFL_FOE_TCP_DLTA, 0, 16);
}

static void PpeSetFoeKa(void)
{
	/* set Keep alive packet with new/org header */
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_KA, 12, 1);

	/* set Keep alive enable */
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_KA_EN, 13, 1);

	/* Keep alive timer value */
	RegModifyBits(PPE_FOE_KA, DFL_FOE_KA_T, 0, 16);

	/* Keep alive time for bind FOE TCP entry */
	RegModifyBits(PPE_FOE_KA, DFL_FOE_TCP_KA, 16, 8);

	/* Keep alive timer for bind FOE UDP entry */
	RegModifyBits(PPE_FOE_KA, DFL_FOE_UDP_KA, 24, 8);
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
	if (Ebl == 1) {
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
#if 0
		/* Disable switch port 6 flow control if HNAT QoS is needed */
		RegModifyBits(RALINK_ETH_SW_BASE+0xC8, 0x0, 8, 2);
#endif
#if defined (CONFIG_RAETH_SPECIAL_TAG)
		/* Set GDMA1 GDM1_TCI_81xx */
		RegModifyBits(FE_GDMA1_FWD_CFG, 0x1, 24, 1);
		/* Set GDMA2 GDM2_TCI_81xx */
		RegModifyBits(FE_GDMA2_FWD_CFG, 0x1, 24, 1);
		/* Set EXT_SW_EN = 1 */
		RegModifyBits(FE_COS_MAP, 0x1, 30, 1);
#endif

	} else {
		/* PPE Engine Disable */
		RegModifyBits(PPE_GLO_CFG, 0, 0, 1);

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
}

static void PpeSetHNATProtoType(void)
{
#ifndef CONFIG_RALINK_RT3052_MP
	/* TODO: we should add exceptional case to register to point out the HNAT case here */
#endif
}

static int32_t PpeEngStart(void)
{
	/* Set PPE Flow Set */
	PpeSetFoeEbl(1);

	/* Set PPE FOE Hash Mode */
	PpeSetFoeHashMode(DFL_FOE_HASH_MODE);

	/* Set default index in policy table */
	PpeSetPreAclEbl(0);
	PpeSetPreMtrEbl(0);
	PpeSetPostMtrEbl(0);
	PpeSetPreAcEbl(0);
	PpeSetPostAcEbl(0);

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

	/* Set default index in policy table */
	PpeSetPreAclEbl(0);
	PpeSetPreMtrEbl(0);
	PpeSetPostMtrEbl(0);
	PpeSetPreAcEbl(0);
	PpeSetPostAcEbl(0);

	/* Unbind FOE table */
	RegWrite(PPE_FOE_BASE, 0);

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

int PpeRsHandler(struct net_device *dev, int hold)
{
#ifdef CONFIG_RA_HW_NAT_PCI
	if (!dev)
		return -1;
	
	if (hold) {
		if (DstPort[DP_PCI0] == dev || DstPort[DP_PCI1] == dev)
			return 1;
		if (!DstPort[DP_PCI0]) {
			dev_hold(dev);
			DstPort[DP_PCI0] = dev;
			return 0;
		}
		else if (!DstPort[DP_PCI1]) {
			dev_hold(dev);
			DstPort[DP_PCI1] = dev;
			return 0;
		}
	} else {
		if (DstPort[DP_PCI0] == dev) {
			DstPort[DP_PCI0] = NULL;
			dev_put(dev);
			return 0;
		}
		else if (DstPort[DP_PCI1] == dev) {
			DstPort[DP_PCI1] = NULL;
			dev_put(dev);
			return 0;
		}
	}
#endif
	return 1;
}

void PpeSetDstPort(uint32_t Ebl)
{
	int i;
	if (Ebl) {
		DstPort[DP_RA0] = ra_dev_get_by_name("ra0");
#if defined (CONFIG_RT2860V2_AP_MBSS)
		DstPort[DP_RA1] = ra_dev_get_by_name("ra1");
#if defined (HWNAT_MBSS_ALL)
		DstPort[DP_RA2] = ra_dev_get_by_name("ra2");
		DstPort[DP_RA3] = ra_dev_get_by_name("ra3");
		DstPort[DP_RA4] = ra_dev_get_by_name("ra4");
		DstPort[DP_RA5] = ra_dev_get_by_name("ra5");
		DstPort[DP_RA6] = ra_dev_get_by_name("ra6");
		DstPort[DP_RA7] = ra_dev_get_by_name("ra7");
#endif
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
#if defined (CONFIG_RTDEV_USB) || defined (CONFIG_RTDEV_PCI)
		DstPort[DP_RAI0] = ra_dev_get_by_name("rai0");
#if defined (CONFIG_RT3090_AP_MBSS) || defined (CONFIG_RT5392_AP_MBSS) || \
    defined (CONFIG_RT3572_AP_MBSS) || defined (CONFIG_RT5572_AP_MBSS) || \
    defined (CONFIG_RT5592_AP_MBSS) || defined (CONFIG_RT3593_AP_MBSS)
		DstPort[DP_RAI1] = ra_dev_get_by_name("rai1");
#if defined (HWNAT_MBSS_ALL)
		DstPort[DP_RAI2] = ra_dev_get_by_name("rai2");
		DstPort[DP_RAI3] = ra_dev_get_by_name("rai3");
		DstPort[DP_RAI4] = ra_dev_get_by_name("rai4");
		DstPort[DP_RAI5] = ra_dev_get_by_name("rai5");
		DstPort[DP_RAI6] = ra_dev_get_by_name("rai6");
		DstPort[DP_RAI7] = ra_dev_get_by_name("rai7");
#endif
#endif // CONFIG_RTDEV_AP_MBSS //
#endif // CONFIG_RTDEV_USB || CONFIG_RTDEV_PCI
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
		DstPort[DP_GMAC1] = ra_dev_get_by_name("eth2");
#ifdef CONFIG_RAETH_GMAC2
		DstPort[DP_GMAC2] = ra_dev_get_by_name("eth3");
#endif
#ifdef CONFIG_RA_HW_NAT_PCI
		i = DP_PCI0;
		DstPort[i] = ra_dev_get_by_name("weth0");	// USB interface name
		if (DstPort[i]) i = DP_PCI1;
		DstPort[i] = ra_dev_get_by_name("wwan0");	// USB WWAN interface name
#endif
	} else {
		for (i = 0; i < MAX_IF_NUM; i++) {
			if (DstPort[i]) {
				dev_put(DstPort[i]);
				DstPort[i] = NULL;
			}
		}
	}
}

uint32_t SetGdmaFwd(uint32_t Ebl)
{
	uint32_t data = 0;

	data = RegRead(FE_GDMA1_FWD_CFG);

	if (Ebl) {
		//Uni-cast frames forward to PPE
		data |= GDM1_UFRC_P_PPE;
#if defined(HWNAT_MCAST_BCAST_PPE)
		//Broad-cast MAC address frames forward to PPE
		data |= GDM1_BFRC_P_PPE;
		//Multi-cast MAC address frames forward to PPE
		data |= GDM1_MFRC_P_PPE;
#endif
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
#if defined(HWNAT_MCAST_BCAST_PPE)
		//Broad-cast MAC address frames forward to PPE
		data |= GDM1_BFRC_P_PPE;
		//Multi-cast MAC address frames forward to PPE
		data |= GDM1_MFRC_P_PPE;
#endif
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

	return 0;
}

/*
 * PPE Enabled: GMAC<->PPE<->CPU
 * PPE Disabled: GMAC<->CPU
 */
static int __init PpeInitMod(void)
{
	char chip_id[8];
	uint32_t rev_id;
	
	*(uint32_t *)&chip_id[0] = RegRead(CHIPID);
	*(uint32_t *)&chip_id[4] = RegRead(CHIPID + 0x4);
	chip_id[6] = '\0';
	rev_id = RegRead(REVID);
	
#if defined (CONFIG_RALINK_RT3052)
	/* RT3052 with RF_REG0 > 0x53 has no bug UDP w/o checksum */
	uint32_t phy_val = 0;
	rw_rf_reg(0, 0, &phy_val);
	ppe_udp_bug = ((phy_val & 0xFF) > 0x53) ? 0 : 1;
#elif defined (CONFIG_RALINK_RT3352)
	/* RT3352 rev 0105 has no bug UDP w/o checksum */
	ppe_udp_bug = (rev_id > 0x0104) ? 0 : 1;
#else
	/* RT3883 and RT3662 at least rev 0105 has bug UDP w/o checksum :-( */
	ppe_udp_bug = 1;
#endif

	// Get net_device structure of Dest Port 
	memset(DstPort, 0, sizeof(DstPort));
	PpeSetDstPort(1);

	/* Register ioctl handler */
	PpeRegIoctlHandler();

	PpeSetRuleSize(PRE_ACL_SIZE, PRE_MTR_SIZE, PRE_AC_SIZE,
		       POST_MTR_SIZE, POST_AC_SIZE);
	AclRegIoctlHandler();
	AcRegIoctlHandler();
	MtrRegIoctlHandler();
	
	/* 0~63 Accounting group */
	PpeSetAGInfo(1, lan_vid);	// AG Index1=VLAN1
	PpeSetAGInfo(2, wan_vid);	// AG Index2=VLAN2

	/* Initialize PPE related register */
	PpeEngStart();

	/* Register RX/TX hook point */
	ra_sw_nat_hook_tx = PpeTxHandler;
	ra_sw_nat_hook_rx = PpeRxHandler;
	ra_sw_nat_hook_rs = PpeRsHandler;

	/* Set GMAC fowrards packet to PPE */
	SetGdmaFwd(1);

	NAT_PRINT("Ralink HW NAT %s Module Enabled, ASIC: %s, REV: %04X, FoE Size: %d\n", HW_NAT_MODULE_VER, chip_id, rev_id, PpeFoeTblSize);

	return 0;
}

static void __exit PpeCleanupMod(void)
{
	NAT_PRINT("Ralink HW NAT %s Module Disabled\n", HW_NAT_MODULE_VER);

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
	AclUnRegIoctlHandler();
	AcUnRegIoctlHandler();
	MtrUnRegIoctlHandler();

	//Release net_device structure of Dest Port 
	PpeSetDstPort(0);

#if 0
	/* Restore switch port 6 flow control to default on */
	RegModifyBits(RALINK_ETH_SW_BASE + 0xC8, 0x3, 8, 2);
#endif
}

module_init(PpeInitMod);
module_exit(PpeCleanupMod);

MODULE_AUTHOR("Steven Liu/Kurtis Ke");
MODULE_LICENSE("Proprietary");
MODULE_DESCRIPTION("Ralink Hardware NAT");
