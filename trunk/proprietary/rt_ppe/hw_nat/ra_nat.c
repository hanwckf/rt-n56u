/*
  Module Name:
  ra_nat.c

  Abstract:

  Revision History:
  Who         When            What
  --------    ----------      ----------------------------------------------
  Name        Date            Modification logs
  Steven Liu  2011-11-11      Support MT7620 Cache mechanism
  Steven Liu  2011-06-01      Support MT7620
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
#include <linux/spinlock.h>
#include <linux/if_vlan.h>
#include <net/ipv6.h>
#include <net/ip.h>
#include <linux/if_pppox.h>
#include <linux/ppp_defs.h>
#include <linux/pci.h>

#include "ra_nat_compat.h"
#include "ra_nat.h"
#include "foe_fdb.h"
#include "frame_engine.h"
#include "policy.h"
#include "util.h"

#if defined (CONFIG_PPE_MCAST)
#include "mcast_tbl.h"
#endif

#if !defined (CONFIG_HNAT_V2)
#include "acl_ioctl.h"
#include "ac_ioctl.h"
#include "acl_policy.h"
#include "mtr_policy.h"
#include "ac_policy.h"
#endif

#define MAX_IF_HASH_NUM		256	/* must be 2^X */

static int wifi_offload __read_mostly = 0;
module_param(wifi_offload, int, S_IRUGO);
MODULE_PARM_DESC(wifi_offload, "PPE IPv4 NAT offload for wifi/extif");

static int udp_offload __read_mostly = DEFAULT_UDP_OFFLOAD;
module_param(udp_offload, int, S_IRUGO);
MODULE_PARM_DESC(udp_offload, "PPE IPv4 NAT offload for UDP proto");

int ipv6_offload __read_mostly = 0;
module_param(ipv6_offload, int, S_IRUGO);
MODULE_PARM_DESC(ipv6_offload, "PPE IPv6 routes offload");

static int ttl_regen __read_mostly = DFL_FOE_TTL_REGEN;
module_param(ttl_regen, int, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(ttl_regen, "TTL regeneration On/Off");

uint16_t lan_vid __read_mostly = CONFIG_RA_HW_NAT_LAN_VLANID;
module_param(lan_vid, ushort, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(lan_vid, "VLAN ID for LAN traffic");

uint16_t wan_vid __read_mostly = CONFIG_RA_HW_NAT_WAN_VLANID;
module_param(wan_vid, ushort, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(wan_vid, "VLAN ID for WAN traffic");

uint32_t DebugLevel = 1;

extern u32 ralink_asic_rev_id;

extern int (*ra_sw_nat_hook_rx) (struct sk_buff * skb);
extern int (*ra_sw_nat_hook_tx) (struct sk_buff * skb, int gmac_no);
extern int (*ra_sw_nat_hook_rs) (struct net_device *dev, int hold);
extern int (*ra_sw_nat_hook_ec) (int engine_init);

#if !defined (CONFIG_RALINK_MT7621)
static int			ppe_udp_bug = 0;
#endif

static struct FoeEntry		*PpeFoeBase = NULL;
static dma_addr_t		PpeFoeBasePhy = 0;
uint32_t			PpeFoeTblSize = FOE_4TB_SIZ;
static struct net_device	*DstPort[MAX_IF_NUM];
static uint8_t			DstPortHash[MAX_IF_HASH_NUM];
DEFINE_SPINLOCK(ppe_foe_lock);

struct FoeEntry *get_foe_entry(uint32_t foe_entry_num)
{
#if defined (CONFIG_HNAT_V2) && defined (CONFIG_RA_HW_NAT_IPV6)
	if (!ipv6_offload) {
		uint8_t *pEntry = (uint8_t *)PpeFoeBase + foe_entry_num * sizeof(struct FoeEntry64);
		return (struct FoeEntry *)pEntry;
	}
#endif

	return &PpeFoeBase[foe_entry_num];
}

#if defined (CONFIG_RA_HW_NAT_DEBUG)
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
	case HIT_BIND_MULTICAST_TO_CPU:
		return ("Hit bind multicast packet to CPU\n");
#if defined (CONFIG_RALINK_MT7621)
	case HIT_BIND_MULTICAST_TO_GMAC_CPU:
		return ("Hit bind multicast packet to GMAC & CPU\n");
	case HIT_PRE_BIND:
		return ("Pre bind\n");
#endif
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

static uint32_t FoeDumpPkt(struct sk_buff *skb)
{
	struct FoeEntry *foe_entry;
	uint32_t foe_entry_num;

	foe_entry_num = FOE_ENTRY_NUM(skb);
	if (foe_entry_num >= FOE_4TB_SIZ)
		return 1;

	foe_entry = get_foe_entry(foe_entry_num);

	NAT_PRINT("\nRx===<FOE_Entry=%d>=====\n", foe_entry_num);
	NAT_PRINT("RcvIF=%s\n", skb->dev->name);
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
#if defined (CONFIG_RA_HW_NAT_IPV6)
#if !defined (CONFIG_HNAT_V2)
	else if (IS_IPV6_1T_ROUTE(foe_entry)) {
		NAT_PRINT("Information Block 1=%x\n", foe_entry->ipv6_1t_route.info_blk1);
		NAT_PRINT("Destination IPv6: %08X:%08X:%08X:%08X",
			  foe_entry->ipv6_1t_route.ipv6_dip3, foe_entry->ipv6_1t_route.ipv6_dip2,
			  foe_entry->ipv6_1t_route.ipv6_dip1, foe_entry->ipv6_1t_route.ipv6_dip0);
		NAT_PRINT("Information Block 2=%x\n", foe_entry->ipv6_1t_route.info_blk2);
	}
#else
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
		if (IS_IPV6_FLAB_EBL()) {
			NAT_PRINT("Flow Label=%08X\n", (foe_entry->ipv6_5t_route.sport << 16) | 
							(foe_entry->ipv6_5t_route.dport));
		} else {
			NAT_PRINT("SPORT=%d\n", foe_entry->ipv6_5t_route.sport);
			NAT_PRINT("DPORT=%d\n", foe_entry->ipv6_5t_route.dport);
		}
		NAT_PRINT("Information Block 2=%x\n", foe_entry->ipv6_5t_route.info_blk2);
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

static inline void SetDstPortIndex(const struct net_device *dev, uint8_t dst_port)
{
	DstPortHash[(dev->ifindex & (MAX_IF_HASH_NUM - 1))] = dst_port;
}

static inline uint8_t GetDstPortIndex(const struct net_device *dev)
{
	return DstPortHash[(dev->ifindex & (MAX_IF_HASH_NUM - 1))];
}

static inline int RemoveVlanTag(struct sk_buff * skb)
{
	if (unlikely(!pskb_may_pull(skb, VLAN_HLEN))) {
		NAT_DEBUG("HNAT: %s, no mem for remove tag!\n", __FUNCTION__);
		return 1;
	}

	/* remove vlan tag from current packet */
	skb_pull(skb, VLAN_HLEN);
	memmove(skb->data - ETH_HLEN, skb->data - VLAN_ETH_HLEN, 2 * ETH_ALEN);
	skb->mac_header += VLAN_HLEN;

	/* set original skb protocol */
	skb->protocol = eth_hdr(skb)->h_proto;

	return 0;
}

#if defined  (CONFIG_RA_HW_NAT_WIFI) || defined (CONFIG_RA_HW_NAT_PCI)
/* push different VID for WiFi pseudo interface or USB external NIC */
static uint32_t PpeExtIfRxHandler(struct sk_buff * skb)
{
	uint16_t VirIfIdx;
#if defined (CONFIG_RAETH_HW_VLAN_TX)
#if defined (RAETH_HW_VLAN4K)
	int hwaccel_tx = 1;	// support full range VLAN 1..4095
#else
	int hwaccel_tx = 0;
#endif
#endif

	/* offload packet types not support for extif:
	   1. VLAN tagged packets (avoid double tag issue).
	   2. PPPoE packets (PPPoE passthrough issue).
	   3. IPv6 1T routes. */
	if ((skb->protocol != __constant_htons(ETH_P_IP))
#if defined (CONFIG_HNAT_V2)
	 && (skb->protocol != __constant_htons(ETH_P_IPV6))
#endif
	   )
		return 1;

	/* check dst interface exist */
	if (skb->dev == NULL) {
		NAT_DEBUG("HNAT: %s, interface not exist, drop this packet.\n", __FUNCTION__);
		kfree_skb(skb);
		return 0;
	}

	VirIfIdx = GetDstPortIndex(skb->dev);
	if (!DstPort[VirIfIdx]) {
		NAT_PRINT("HNAT: %s, unknown interface %s\n", __FUNCTION__, skb->dev->name);
		return 1;
	}

#if defined (CONFIG_RAETH_HW_VLAN_TX) && !defined (RAETH_HW_VLAN4K)
	switch (VirIfIdx) {
	case DP_RA0:
	case DP_RA1:
#if defined (HWNAT_USE_SECOND_IF)
	case DP_RAI0:
	case DP_RAI1:
#endif
	case DP_NIC0:
		hwaccel_tx = 1;
		break;
	}
#endif

	/* check GMAC1 interface exist and open */
	if (!DstPort[DP_GMAC1] || !(DstPort[DP_GMAC1]->flags & IFF_UP))
		return 1;

	/* set pointer to L2 header before call dev_queue_xmit */
	skb_reset_network_header(skb);
	skb_push(skb, ETH_HLEN);

	/* push vlan tag to stand for actual incoming interface,
	    so HNAT module can know the actual incoming interface from vlan id. */
#if defined (CONFIG_RAETH_HW_VLAN_TX)
	if (hwaccel_tx) {
		vlan_insert_tag_hwaccel(skb, __constant_htons(ETH_P_8021Q), VirIfIdx);
	} else
#endif
	{
		skb = vlan_insert_tag_set_proto(skb, __constant_htons(ETH_P_8021Q), VirIfIdx);
		if (unlikely(!skb)) {
			NAT_DEBUG("HNAT: %s, unable tagging skb, memleak ? (VirIfIdx=%d)\n", __FUNCTION__, VirIfIdx);
			return 0;
		}
	}

	/* redirect to PPE via GMAC1 (check FOE_MAGIC_PPE in raeth) */
	DO_FILL_FOE_DPORT_PPE(skb);
	skb->dev = DstPort[DP_GMAC1];
	dev_queue_xmit(skb);

	return 0;
}

static uint32_t PpeExtIfPingPongHandler(struct sk_buff * skb)
{
	struct net_device *dev;
	uint16_t VirIfIdx;

#if defined (CONFIG_RAETH_HW_VLAN_RX)
	if (skb_vlan_tag_present(skb))
		VirIfIdx = skb_vlan_tag_get(skb);
	else
#endif
	if (skb->protocol == __constant_htons(ETH_P_8021Q)) {
		struct vlan_ethhdr *veth = vlan_eth_hdr(skb);
		VirIfIdx = ntohs(veth->h_vlan_TCI);
	} else {
		/* something wrong: proto must be 802.11q, interface index must be < MAX_IF_NUM and exist, 
			don`t touch this packets and return to normal path before corrupt in detag code
		*/
		return 1;
	}

	if (VirIfIdx >= MAX_IF_NUM)
		return 1;

	dev = DstPort[VirIfIdx];
	if (!dev) {
		NAT_PRINT("HNAT: %s, reentry packet interface (VirIfIdx=%d) not exist!\n", __FUNCTION__, VirIfIdx);
		return 1;
	}

	/* remove vlan tag from current packet */
#if defined (CONFIG_RAETH_HW_VLAN_RX)
	if (skb_vlan_tag_present(skb))
		skb->vlan_tci = 0;
	else
#endif
	{
		if (RemoveVlanTag(skb))
			return 1;
	}

	/* set original skb dev */
	skb->dev = dev;

	/* set original skb pkt_type
	   note: eth_type_trans is already completed for GMAC1 (dev eth2),
	   so check only if pkt_type PACKET_OTHERHOST */
	if (skb->pkt_type == PACKET_OTHERHOST) {
		struct ethhdr *eth = eth_hdr(skb);
		if (ether_addr_equal(eth->h_dest, dev->dev_addr))
			skb->pkt_type = PACKET_HOST;
#if defined (CONFIG_RAETH_BOTH_GMAC)
		else if (DstPort[DP_GMAC2] && ether_addr_equal(eth->h_dest, DstPort[DP_GMAC2]->dev_addr))
			skb->pkt_type = PACKET_HOST;
#endif
	}

	return 1;
}
#endif

static void PpeKeepAliveHandler(struct sk_buff *skb, int recover_header)
{
	struct FoeEntry *foe_entry;
	struct ethhdr *eth;
	uint16_t eth_type;
	uint32_t vlan1_gap = 0;
	uint32_t vlan2_gap = 0;
	uint32_t pppoe_gap = 0;
	uint32_t foe_entry_num;

	foe_entry_num = FOE_ENTRY_NUM(skb);
	if (foe_entry_num >= FOE_4TB_SIZ || !FOE_ENTRY_VALID(skb)) {
		NAT_DEBUG("HNAT: %s, invalid FoE entry (%u)\n", __FUNCTION__, foe_entry_num);
		return;
	}

	eth = (struct ethhdr *)(skb->data - ETH_HLEN);
	eth_type = ntohs(skb->protocol);

	foe_entry = get_foe_entry(foe_entry_num);

	/*
	 * try to recover to original SMAC/DMAC, but we don't have such information.
	 * just use SMAC as DMAC and set Multicast address as SMAC.
	 */
	if (recover_header) {
		unsigned char tmp[8];
		
		memcpy(tmp, eth->h_dest, ETH_ALEN);
		memcpy(eth->h_dest, eth->h_source, ETH_ALEN);
		memcpy(eth->h_source, tmp, ETH_ALEN);
	}

	eth->h_source[0] = 0x1;	//change to multicast packet, make bridge not learn this packet

	if (eth_type == ETH_P_8021Q) {
		struct vlan_hdr *vh = (struct vlan_hdr *)skb->data;
		
		vlan1_gap = VLAN_HLEN;
		
		if (recover_header) {
			if (((ntohs(vh->h_vlan_TCI)) & VLAN_VID_MASK) == wan_vid) {
				/* It make packet like coming from LAN port */
				vh->h_vlan_TCI = htons(lan_vid);
			} else {
				/* It make packet like coming from WAN port */
				vh->h_vlan_TCI = htons(wan_vid);
			}
		}
		
		if (vh->h_vlan_encapsulated_proto == __constant_htons(ETH_P_PPP_SES)) {
			pppoe_gap = 8;
		} else if (vh->h_vlan_encapsulated_proto == __constant_htons(ETH_P_8021Q)) {
			vlan2_gap = VLAN_HLEN;
			vh = (struct vlan_hdr *)(skb->data + VLAN_HLEN);
			/* VLAN + VLAN + PPPoE */
			if (vh->h_vlan_encapsulated_proto == __constant_htons(ETH_P_PPP_SES)) {
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
			if (recover_header) {
				struct tcphdr *th = (struct tcphdr *)((uint8_t *)iph + iph->ihl * 4);
				FoeToOrgTcpHdr(foe_entry, iph, th);
			}
		} else if (iph->protocol == IPPROTO_UDP) {
			struct udphdr *uh = (struct udphdr *)((uint8_t *)iph + iph->ihl * 4);
#if !defined (CONFIG_RALINK_MT7621)
			if (!uh->check && ppe_udp_bug && foe_entry->bfib1.state == BIND) {
				/* no UDP checksum, force unbind session from PPE for workaround PPE UDP bug */
				spin_lock_bh(&ppe_foe_lock);
				foe_entry->udib1.state = UNBIND;
				foe_entry->udib1.time_stamp = RegRead(FOE_TS) & 0xFF;
#if defined (CONFIG_HNAT_V2)
				/* clear HWNAT cache */
				RegModifyBits(CAH_CTRL, 1, 9, 1);
				RegModifyBits(CAH_CTRL, 0, 9, 1);
#endif
				spin_unlock_bh(&ppe_foe_lock);
			}
#endif
			if (recover_header)
				FoeToOrgUdpHdr(foe_entry, iph, uh);
		}
		/* recover to original layer 3 header */
		if (recover_header)
			FoeToOrgIpHdr(foe_entry, iph);
	} else if (eth_type == ETH_P_IPV6) {
		/* Nothing to do */
	} else {
		return;
	}

	/*
	 * Ethernet driver will call eth_type_trans() to update skb->pkt_type.
	 * If(destination mac != my mac) 
	 *   skb->pkt_type=PACKET_OTHERHOST;
	 *
	 * In order to pass ip_rcv() check, we change pkt_type to PACKET_HOST here
	 */
	skb->pkt_type = PACKET_HOST;
}

static int PpeHitBindForceToCpuHandler(struct sk_buff *skb, uint32_t foe_entry_num)
{
	struct FoeEntry *foe_entry;
	struct net_device *dev = NULL;
	int act_dp;

	if (foe_entry_num >= FOE_4TB_SIZ) {
		NAT_DEBUG("HNAT: %s, invalid FoE entry (%u)\n", __FUNCTION__, foe_entry_num);
		return 1;
	}

	foe_entry = get_foe_entry(foe_entry_num);

#if !defined (CONFIG_HNAT_V2)
	act_dp = foe_entry->ipv4_hnapt.act_dp;			// act_dp: offset 13 dword for IPv4/IPv6
	dev = DstPort[act_dp];
#else
	if (IS_IPV4_GRP(foe_entry)) {
		act_dp = foe_entry->ipv4_hnapt.act_dp;		// act_dp: offset 11 dword for IPv4
		dev = DstPort[act_dp];
	}
#if defined (CONFIG_RA_HW_NAT_IPV6)
	else if (ipv6_offload && IS_IPV6_GRP(foe_entry)) {
		act_dp = foe_entry->ipv6_5t_route.act_dp;	// act_dp: offset 14 dword for IPv6
		dev = DstPort[act_dp];
	}
#endif
	else
		act_dp = -1;
#endif
	if (!dev) {
		NAT_PRINT("HNAT: %s, dest interface (dp: %d, pkt_type: %d) not exist!\n", __FUNCTION__, act_dp, foe_entry->bfib1.pkt_type);
		return 1;
	}

	if (!netif_running(dev)) {
		/* wifi/ext interface is down, simple drop skb */
		dev_kfree_skb(skb);
		return 0;
	}

#if defined (CONFIG_RALINK_MT7620)
	/* remove vlan tag from PPE packet (P7/P6 egress always tagged) */
	if (skb->protocol == __constant_htons(ETH_P_8021Q)) {
		if (RemoveVlanTag(skb))
			return 1;
	}
#endif

	skb->dev = dev;

	/* set pointer to L2 header before call dev_queue_xmit */
	skb_reset_network_header(skb);
	skb_push(skb, ETH_HLEN);
	dev_queue_xmit(skb);

	return 0;
}

#if defined (CONFIG_HNAT_V2)
static int PpeHitBindForceMcastToWiFiHandler(struct sk_buff *skb)
{
#if defined (CONFIG_RA_HW_NAT_MCAST)
#if defined (CONFIG_RA_HW_NAT_WIFI) || defined (CONFIG_RA_HW_NAT_PCI)
	int i;
	struct sk_buff *skb2;
	struct net_device *dev;

	/* remove vlan tag from PPE packet */
#if defined (CONFIG_RAETH_HW_VLAN_RX)
	if (skb_vlan_tag_present(skb))
		skb->vlan_tci = 0;
	else
#endif
	if (skb->protocol == __constant_htons(ETH_P_8021Q)
#if defined (CONFIG_RA_HW_NAT_QDMA)
	 || skb->protocol == 0x5678
#endif
	) {
		if (RemoveVlanTag(skb))
			return 1;
	}

	/* set pointer to L2 header before call dev_queue_xmit */
	skb_reset_network_header(skb);
	skb_push(skb, ETH_HLEN);

#if defined (CONFIG_RA_HW_NAT_WIFI)
	for (i = DP_RA0; i < MAX_WIFI_IF_NUM; i++) {
		dev = DstPort[i];
		if (dev && netif_running(dev)) {
			skb2 = skb_clone(skb, GFP_ATOMIC);
			if (skb2) {
				skb2->dev = dev;
				dev_queue_xmit(skb2);
			}
		}
	}
#elif defined (CONFIG_RA_HW_NAT_PCI)
	for (i= DP_NIC0; i <= DP_NIC1; i++) {
		dev = DstPort[i];
		if (dev && netif_running(dev)) {
			skb2 = skb_clone(skb, GFP_ATOMIC);
			if (skb2) {
				skb2->dev = dev;
				dev_queue_xmit(skb2);
			}
		}
	}
#endif
#endif
#endif /* CONFIG_RA_HW_NAT_MCAST */

	dev_kfree_skb(skb);

	return 0;
}
#endif

#if defined (CONFIG_RA_HW_NAT_ACL2UP_HELPER)
static void PpeGetUpFromACLRule(struct sk_buff *skb)
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
		if (vh->h_vlan_encapsulated_proto == __constant_htons(ETH_P_PPP_SES)) {
			pppoe_gap = 8;
		} else if (vh->h_vlan_encapsulated_proto == __constant_htons(ETH_P_8021Q)) {
			vlan2_gap = VLAN_HLEN;
			vh = (struct vlan_hdr *)(skb->data + VLAN_HLEN);
			/* VLAN + VLAN + PPPoE */
			if (vh->h_vlan_encapsulated_proto == __constant_htons(ETH_P_PPP_SES)) {
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
	uint32_t foe_ai, foe_sp;

	/* return truncated packets to normal path */
	if (!skb || skb->len < ETH_HLEN) {
		NAT_DEBUG("HNAT: %s, skb null or small len in RX path\n", __FUNCTION__);
		return 1;
	}

#if !defined (CONFIG_RA_HW_NAT_MCAST) || !defined (CONFIG_HNAT_V2)
	if (skb->pkt_type == PACKET_MULTICAST)
		return 1;
#endif
	if (skb->pkt_type == PACKET_BROADCAST)
		return 1;

	if (FOE_MAGIC_TAG(skb) == FOE_MAGIC_EXTIF) {
		/* the incoming packet is from PCI or WiFi interface */
#if defined (CONFIG_RA_HW_NAT_WIFI) || defined (CONFIG_RA_HW_NAT_PCI)
		if (wifi_offload)
			return PpeExtIfRxHandler(skb);
		else
#endif
			return 1;
	}

#if defined (CONFIG_RA_HW_NAT_DEBUG)
	if (DebugLevel >= 7)
		FoeDumpPkt(skb);
#endif

	foe_ai = FOE_AI(skb);
	foe_sp = FOE_SP(skb);

#if defined (CONFIG_RA_HW_NAT_QDMA)
	/* QDMA QoS remove CPU reason, we use special tag to identify force to CPU.
	 * Notes: CPU reason & Entry ID fileds are invalid at this moment
	 */
	if (foe_sp == 5 && skb->protocol == 0x5678) {
		uint32_t foe_entry_num = ntohs(vlan_eth_hdr(skb)->h_vlan_TCI);
		if (RemoveVlanTag(skb))
			return 1;
		return PpeHitBindForceToCpuHandler(skb, foe_entry_num);
	}
#endif

	if (foe_ai == HIT_BIND_FORCE_TO_CPU) {
		/* It means the flow is already in binding state, just transfer to output interface */
		if (FOE_ENTRY_VALID(skb))
			return PpeHitBindForceToCpuHandler(skb, FOE_ENTRY_NUM(skb));
#if defined (CONFIG_HNAT_V2)
#if defined (CONFIG_RALINK_MT7621)
	} else if (foe_ai == HIT_BIND_MULTICAST_TO_CPU || foe_ai == HIT_BIND_MULTICAST_TO_GMAC_CPU) {
#else
	} else if (foe_ai == HIT_BIND_MULTICAST_TO_CPU) {
#endif
		if (FOE_ENTRY_VALID(skb))
			return PpeHitBindForceMcastToWiFiHandler(skb);
	} else if (foe_ai == HIT_BIND_KEEPALIVE_DUP_OLD_HDR) {
		PpeKeepAliveHandler(skb, 0);
	} else if (foe_ai == HIT_BIND_KEEPALIVE_MC_NEW_HDR) {
		PpeKeepAliveHandler(skb, 1);
	} else if (foe_ai == HIT_BIND_KEEPALIVE_UC_OLD_HDR) {
		;
#if defined (CONFIG_RALINK_MT7621)
	} else if (foe_sp == 0 || foe_sp == 5) {
#else
	} else if (foe_sp == 6) {
#endif
		/* handle the incoming packet which came back from PPE */
#if defined (CONFIG_RA_HW_NAT_WIFI) || defined (CONFIG_RA_HW_NAT_PCI)
		if (wifi_offload)
			return PpeExtIfPingPongHandler(skb);
#endif
#else /* !CONFIG_HNAT_V2 */
	} else if (foe_ai == HIT_BIND_KEEPALIVE) {
#if (DFL_FOE_KA == 0)
		PpeKeepAliveHandler(skb, 1);
#endif
	} else if (foe_sp == 0) {
		/* handle the incoming packet which came back from PPE */
#if defined (CONFIG_RA_HW_NAT_WIFI) || defined (CONFIG_RA_HW_NAT_PCI)
		if (wifi_offload)
			return PpeExtIfPingPongHandler(skb);
#endif
#if defined (CONFIG_RA_HW_NAT_ACL2UP_HELPER)
	} else if (foe_ai == HIT_UNBIND_RATE_REACH) {
		PpeGetUpFromACLRule(skb);
#endif
#endif /* CONFIG_HNAT_V2 */
	}

	return 1;
}

#if defined (CONFIG_RA_HW_NAT_WIFI) || defined (CONFIG_RA_HW_NAT_PCI)
uint32_t PpeSetExtIfNum(struct sk_buff *skb, struct FoeEntry* foe_entry)
{
	uint32_t offset = GetDstPortIndex(skb->dev);

	/* This is ugly soultion to support WiFi pseudo interface (pre MT76x2).
	 * Please double check the definition is the same as include/rt_linux.h
	 */
#define CB_OFF  10
#define RTMP_GET_PACKET_IF(skb)		skb->cb[CB_OFF+6]
#define MIN_NET_DEVICE_FOR_WDS		0x10
#define MIN_NET_DEVICE_FOR_APCLI	0x20
#define MIN_NET_DEVICE_FOR_MESH		0x30

#if defined (HWNAT_USE_SECOND_IF)
	if (offset == DP_RAI0) {
#if defined (HWNAT_USE_SECOND_IF_CBOFF)
#if defined (HWNAT_USE_IF_MESH)
		if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_MESH)
			offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_MESH + DP_MESHI0);
		else
#endif
#if defined (HWNAT_USE_IF_APCLI)
		if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_APCLI)
			offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_APCLI + DP_APCLII0);
		else
#endif
#if defined (HWNAT_USE_IF_WDS)
		if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_WDS)
			offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_WDS + DP_WDSI0);
		else
#endif
#endif
			;
	} else
#endif
	if (offset == DP_RA0) {
#if defined (HWNAT_USE_FIRST_IF_CBOFF)
#if defined (HWNAT_USE_IF_MESH)
		if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_MESH)
			offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_MESH + DP_MESH0);
		else
#endif
#if defined (HWNAT_USE_IF_APCLI)
		if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_APCLI)
			offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_APCLI + DP_APCLI0);
		else
#endif
#if defined (HWNAT_USE_IF_WDS)
		if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_WDS)
			offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_WDS + DP_WDS0);
		else
#endif
#endif
			;
	}

	if (!DstPort[offset]) {
		NAT_PRINT("HNAT: %s, unknown interface %s\n", __FUNCTION__, skb->dev->name);
		return 1;
	}

	/* Set actual output port info */
#if !defined (CONFIG_HNAT_V2)
	foe_entry->ipv4_hnapt.act_dp = offset;			// act_dp: offset 13 dword for IPv4/IPv6
#else
	if (IS_IPV4_GRP(foe_entry)) {
		foe_entry->ipv4_hnapt.act_dp = offset;		// act_dp: offset 11 dword for IPv4
	}
#if defined (CONFIG_RA_HW_NAT_IPV6)
	else if (ipv6_offload && IS_IPV6_GRP(foe_entry)) {
		foe_entry->ipv6_5t_route.act_dp = offset;	// act_dp: offset 14 dword for IPv6
	}
#endif
	else {
		return 1;
	}
#endif

	return 0;
}
#endif

#if defined (CONFIG_HNAT_V2)
static void PpeSetInfoBlk2(struct _info_blk2 *iblk2, uint32_t fpidx, uint32_t port_mg, uint32_t port_ag)
{
#if defined (CONFIG_RALINK_MT7621)
#if defined (CONFIG_RA_HW_NAT_QDMA)
	/* QDMA HW mode should goes to QoS */
	iblk2->qid = ((fpidx >> 8) & 0xf);
	iblk2->fqos = (fpidx & 0x8000) ? 1 : 0;
#else
	/* PDMA/QDMA SW mode should not goes to QoS */
	iblk2->fqos = 0;
#endif

	/* 0:PSE, 1:GSW, 2:GMAC, 4:PPE, 5:QDMA, 7:DROP */
	iblk2->dp = (fpidx & 0x7);

	/* need lookup another multicast table if this is multicast flow */
	iblk2->mcast = (fpidx & 0x80) ? 1 : 0;

#elif defined (CONFIG_RALINK_MT7620)
	/* 6: force to CPU, 8: no force port */
	iblk2->fpidx = fpidx;
#endif
	iblk2->port_mg = port_mg;
	iblk2->port_ag = port_ag;
}

#if defined (CONFIG_RA_HW_NAT_QDMA)
extern u8  M2Q_table[64];
extern int M2Q_wan_lan;

static u32 get_qid_ipv4(struct sk_buff *skb, int is_wan)
{
	u32 QID;
//#if defined (CONFIG_IMQ) || defined (CONFIG_IMQ_MODULE)
#if 1
	QID = M2Q_table[(skb->mark & 0x3f)];
	if (is_wan && M2Q_wan_lan && QID < 8)
		QID += 8;
#else
	const struct iphdr *iph = ip_hdr(skb);
	QID = (iph->tos & 0xff) >> 4; // 0xf0 -> 0x0f
#endif
	return QID;
}

static u32 get_qid_ipv6(struct sk_buff *skb, int is_wan)
{
	u32 QID;
//#if defined (CONFIG_IMQ) || defined (CONFIG_IMQ_MODULE)
#if 1
	QID = M2Q_table[(skb->mark & 0x3f)];
	if (is_wan && M2Q_wan_lan && QID < 8)
		QID += 8;
#else
	const struct ipv6hdr *ip6h = ipv6_hdr(skb);
	QID = ((ip6h->priority << 4) | (ip6h->flow_lbl[0] >> 4)) >> 4; // 0xf0 -> 0x0f
#endif
	return QID;
}
#endif

#if defined (CONFIG_RA_HW_NAT_IPV6)
static uint16_t PpeGetChkBase(struct iphdr *iph)
{
	uint16_t org_chksum = ntohs(iph->check);
	uint16_t org_tot_len = ntohs(iph->tot_len);
	uint16_t org_id = ntohs(iph->id);
	uint16_t chksum_tmp, tot_len_tmp, id_tmp;
	uint32_t tmp;

	chksum_tmp = ~(org_chksum);
	tot_len_tmp = ~(org_tot_len);
	id_tmp = ~(org_id);

	tmp = chksum_tmp + tot_len_tmp + id_tmp;
	tmp = ((tmp >> 16) & 0x7) + (tmp & 0xFFFF);
	tmp = ((tmp >> 16) & 0x7) + (tmp & 0xFFFF);

	return (uint16_t)(tmp & 0xFFFF);
}
#endif

int32_t FoeBindToPpe(struct sk_buff *skb, struct FoeEntry* foe_entry, int gmac_no)
{
	struct ethhdr *eth = (struct ethhdr *)skb->data;
	uint32_t vlan_layer = 0;
	uint32_t vlan1_gap = 0;
	uint32_t vlan2_gap = 0;
	uint32_t pppoe_gap = 0;
	uint16_t ppp_tag = 0;
	uint16_t ppp_sid = 0;
	uint16_t vlan1_id = 0;
	uint16_t vlan2_id = 0;
	uint16_t vlan_tag = 0;
	uint16_t eth_type;
	uint32_t pkt_type;
	int __maybe_unused drm_keep = 1;
	int rmt_header = 0;
	int is_mcast = 0;

	if (is_multicast_ether_addr(eth->h_dest))
#if defined (CONFIG_RA_HW_NAT_MCAST)
		is_mcast = 1;
#else
		return 1;
#endif

	eth_type = ntohs(eth->h_proto);

	/* offload packet types not support for extif:
	   1. VLAN tagged packets (avoid double tag issue).
	   2. PPPoE packets (PPPoE passthrough issue). */
	if ((gmac_no == 0) && (eth_type != ETH_P_IP) && (eth_type != ETH_P_IPV6))
		return 1;

	/* if this entry is already in binding state, skip it */
	if (foe_entry->bfib1.state == BIND)
		return 1;

	if (eth_type == ETH_P_8021Q || skb_vlan_tag_present(skb)
#if defined (CONFIG_RAETH_SPECIAL_TAG)
	 || ((eth_type & 0xFF00) == ETH_P_8021Q)
#endif
	   ) {
		struct vlan_hdr *vh;
		
		vlan_layer++;
		if (skb_vlan_tag_present(skb)) {
			vlan_tag = ETH_P_8021Q;
			vlan1_id = skb_vlan_tag_get(skb);
		} else {
			vh = (struct vlan_hdr *)(skb->data + ETH_HLEN);
			vlan_tag = eth_type;
			vlan1_gap = VLAN_HLEN;
			vlan1_id = ntohs(vh->h_vlan_TCI);
			eth_type = ntohs(vh->h_vlan_encapsulated_proto);
		}
		
		/* VLAN + PPPoE */
		if (eth_type == ETH_P_PPP_SES) {
			struct pppoe_hdr *peh = (struct pppoe_hdr *)(skb->data + ETH_HLEN + vlan1_gap);
			if (peh->ver != 1 || peh->type != 1)
				return 1;
			pppoe_gap = 8;
			ppp_tag = ntohs(peh->tag[0].tag_type);
			ppp_sid = ntohs(peh->sid);
			
			/* Double VLAN = VLAN + VLAN */
		} else if (eth_type == ETH_P_8021Q) {
			vh = (struct vlan_hdr *)(skb->data + vlan1_gap + VLAN_HLEN);
			vlan_layer++;
			vlan2_gap = VLAN_HLEN;
			vlan2_id = ntohs(vh->h_vlan_TCI);
			eth_type = ntohs(vh->h_vlan_encapsulated_proto);
			
			/* VLAN + VLAN + PPPoE */
			if (eth_type == ETH_P_PPP_SES) {
				struct pppoe_hdr *peh = (struct pppoe_hdr *)(skb->data + ETH_HLEN + vlan1_gap + VLAN_HLEN);
				if (peh->ver != 1 || peh->type != 1)
					return 1;
				pppoe_gap = 8;
				ppp_tag = ntohs(peh->tag[0].tag_type);
				ppp_sid = ntohs(peh->sid);
				
				/* VLAN + VLAN + VLAN */
			} else if (eth_type == ETH_P_8021Q) {
				vh = (struct vlan_hdr *)(skb->data + ETH_HLEN + vlan1_gap + VLAN_HLEN);
				vlan_layer++;
				/* VLAN + VLAN + VLAN + VLAN */
				if (vh->h_vlan_encapsulated_proto == __constant_htons(ETH_P_8021Q))
					vlan_layer++;
			}
		}
	}
#if defined (CONFIG_RAETH_BOTH_GMAC)
	/* PPPoE + IP (MT7621 with 2xGMAC) */
	else if (eth_type == ETH_P_PPP_SES) {
		struct pppoe_hdr *peh = (struct pppoe_hdr *)(skb->data + ETH_HLEN);
		if (peh->ver != 1 || peh->type != 1)
			return 1;
		pppoe_gap = 8;
		ppp_tag = ntohs(peh->tag[0].tag_type);
		ppp_sid = ntohs(peh->sid);
	}
#endif

	pkt_type = foe_entry->bfib1.pkt_type;

	if ((eth_type == ETH_P_IP) || (eth_type == ETH_P_PPP_SES && ppp_tag == PPP_IP)) {
		struct iphdr *iph = (struct iphdr *)(skb->data + ETH_HLEN + vlan1_gap + vlan2_gap + pppoe_gap);
		
		if (iph->protocol == IPPROTO_TCP) {
			struct tcphdr *th;
			if (iph->frag_off & htons(IP_MF|IP_OFFSET))
				return 1;
			
			if (pkt_type != IPV4_DSLITE) {
				/* fill L4 info */
				th = (struct tcphdr *)((uint8_t *)iph + iph->ihl * 4);
				foe_entry->ipv4_hnapt.new_sport = ntohs(th->source);
				foe_entry->ipv4_hnapt.new_dport = ntohs(th->dest);
				foe_entry->ipv4_hnapt.bfib1.udp = TCP;
			}
		} else if (iph->protocol == IPPROTO_UDP) {
			struct udphdr *uh;
			if (iph->frag_off & htons(IP_MF|IP_OFFSET))
				return 1;
			
			if (pkt_type != IPV4_DSLITE) {
				if (!udp_offload)
					return 1;
				
				uh = (struct udphdr *)((uint8_t *)iph + iph->ihl * 4);
#if !defined (CONFIG_RALINK_MT7621)
				/* check PPE bug */
				if (ppe_udp_bug) {
					if (!uh->check)
						return 1;
					if (uh->dest == __constant_htons(500) ||	// IPSec IKE
					    uh->dest == __constant_htons(4500) ||	// IPSec NAT-T
					    uh->dest == __constant_htons(1701))		// L2TP
						return 1;
				}
#endif
				/* fill L4 info */
				foe_entry->ipv4_hnapt.new_sport = ntohs(uh->source);
				foe_entry->ipv4_hnapt.new_dport = ntohs(uh->dest);
				foe_entry->ipv4_hnapt.bfib1.udp = UDP;
			}
#if defined (CONFIG_RA_HW_NAT_IPV6)
		} else if (iph->protocol == IPPROTO_IPV6) {
				if (!ipv6_offload)
					return 1;
				
				/* fill 6rd entry */
				foe_entry->ipv6_6rd.tunnel_sipv4 = ntohl(iph->saddr);
				foe_entry->ipv6_6rd.tunnel_dipv4 = ntohl(iph->daddr);
				foe_entry->ipv6_6rd.hdr_chksum = PpeGetChkBase(iph);
				foe_entry->ipv6_6rd.flag = (ntohs(iph->frag_off) >> 13);
				foe_entry->ipv6_6rd.ttl = iph->ttl;
				foe_entry->ipv6_6rd.dscp = iph->tos;
				
				/* IPv6 6rd shall be turn on by SW during initialization */
				foe_entry->bfib1.pkt_type = IPV6_6RD;
				
				drm_keep = 0;
#endif
		} else if (iph->protocol == IPPROTO_GRE) {
			/* do nothing */
			return 1;
		} else {
			/* packet format is not supported */
			return 1;
		}
		
		if (pkt_type == IPV4_DSLITE) {
#if defined (CONFIG_RA_HW_NAT_IPV6)
			if (!ipv6_offload)
				return 1;
			
			rmt_header = 1;				// remove outer IPv4 header
			
			foe_entry->ipv4_dslite.iblk2.dscp = iph->tos;
			
			/* fill L2 info (80B) */
			FoeSetMacHiInfo(foe_entry->ipv4_dslite.dmac_hi, eth->h_dest);
			FoeSetMacLoInfo(foe_entry->ipv4_dslite.dmac_lo, eth->h_dest);
			FoeSetMacHiInfo(foe_entry->ipv4_dslite.smac_hi, eth->h_source);
			FoeSetMacLoInfo(foe_entry->ipv4_dslite.smac_lo, eth->h_source);
			foe_entry->ipv4_dslite.pppoe_id = ppp_sid;
			foe_entry->ipv4_dslite.vlan1 = vlan1_id;
			foe_entry->ipv4_dslite.vlan2 = vlan2_id;
			foe_entry->ipv4_dslite.etype = vlan_tag;
#else
			return 1;
#endif
		} else {
#if defined (CONFIG_RA_HW_NAT_IPV6)
			if (iph->protocol == IPPROTO_IPV6) {
				/* fill L2 info (80B) */
				FoeSetMacHiInfo(foe_entry->ipv6_6rd.dmac_hi, eth->h_dest);
				FoeSetMacLoInfo(foe_entry->ipv6_6rd.dmac_lo, eth->h_dest);
				FoeSetMacHiInfo(foe_entry->ipv6_6rd.smac_hi, eth->h_source);
				FoeSetMacLoInfo(foe_entry->ipv6_6rd.smac_lo, eth->h_source);
				foe_entry->ipv6_6rd.pppoe_id = ppp_sid;
				foe_entry->ipv6_6rd.vlan1 = vlan1_id;
				foe_entry->ipv6_6rd.vlan2 = vlan2_id;
				foe_entry->ipv6_6rd.etype = vlan_tag;
			} else
#endif
			{
				/* fill L3 info */
				foe_entry->ipv4_hnapt.new_sip = ntohl(iph->saddr);
				foe_entry->ipv4_hnapt.new_dip = ntohl(iph->daddr);
				foe_entry->ipv4_hnapt.iblk2.dscp = iph->tos;
				
				/* fill L2 info (64B) */
				FoeSetMacHiInfo(foe_entry->ipv4_hnapt.dmac_hi, eth->h_dest);
				FoeSetMacLoInfo(foe_entry->ipv4_hnapt.dmac_lo, eth->h_dest);
				FoeSetMacHiInfo(foe_entry->ipv4_hnapt.smac_hi, eth->h_source);
				FoeSetMacLoInfo(foe_entry->ipv4_hnapt.smac_lo, eth->h_source);
				foe_entry->ipv4_hnapt.pppoe_id = ppp_sid;
				foe_entry->ipv4_hnapt.vlan1 = vlan1_id;
				foe_entry->ipv4_hnapt.vlan2 = vlan2_id;
				foe_entry->ipv4_hnapt.etype = vlan_tag;
			}
		}
		
#if defined (CONFIG_RA_HW_NAT_IPV6)
	} else if (eth_type == ETH_P_IPV6 || (eth_type == ETH_P_PPP_SES && ppp_tag == PPP_IPV6)) {
		struct ipv6hdr *ip6h;
		if (!ipv6_offload)
			return 1;
		
		ip6h = (struct ipv6hdr *)(skb->data + ETH_HLEN + vlan1_gap + vlan2_gap + pppoe_gap);
		if (ip6h->nexthdr == NEXTHDR_IPIP) {
			/* fill in DSLite entry */
			foe_entry->ipv4_dslite.tunnel_sipv6_0 = ntohl(ip6h->saddr.s6_addr32[0]);
			foe_entry->ipv4_dslite.tunnel_sipv6_1 = ntohl(ip6h->saddr.s6_addr32[1]);
			foe_entry->ipv4_dslite.tunnel_sipv6_2 = ntohl(ip6h->saddr.s6_addr32[2]);
			foe_entry->ipv4_dslite.tunnel_sipv6_3 = ntohl(ip6h->saddr.s6_addr32[3]);
			
			foe_entry->ipv4_dslite.tunnel_dipv6_0 = ntohl(ip6h->daddr.s6_addr32[0]);
			foe_entry->ipv4_dslite.tunnel_dipv6_1 = ntohl(ip6h->daddr.s6_addr32[1]);
			foe_entry->ipv4_dslite.tunnel_dipv6_2 = ntohl(ip6h->daddr.s6_addr32[2]);
			foe_entry->ipv4_dslite.tunnel_dipv6_3 = ntohl(ip6h->daddr.s6_addr32[3]);
			
			memcpy(foe_entry->ipv4_dslite.flow_lbl, ip6h->flow_lbl, 3);
			foe_entry->ipv4_dslite.priority = ip6h->priority;
			foe_entry->ipv4_dslite.hop_limit = ip6h->hop_limit;
			
			/* IPv4 DS-Lite shall be turn on by SW during initialization */
			foe_entry->bfib1.pkt_type = IPV4_DSLITE;
			
			drm_keep = 0;
		} else {
			foe_entry->ipv6_5t_route.iblk2.dscp = ((ip6h->priority << 4) | (ip6h->flow_lbl[0]>>4));
			
			if (pkt_type != IPV6_6RD) {
				/* fill in ipv6 (3T/5T) routing entry */
				foe_entry->ipv6_5t_route.ipv6_sip0 = ntohl(ip6h->saddr.s6_addr32[0]);
				foe_entry->ipv6_5t_route.ipv6_sip1 = ntohl(ip6h->saddr.s6_addr32[1]);
				foe_entry->ipv6_5t_route.ipv6_sip2 = ntohl(ip6h->saddr.s6_addr32[2]);
				foe_entry->ipv6_5t_route.ipv6_sip3 = ntohl(ip6h->saddr.s6_addr32[3]);
				
				foe_entry->ipv6_5t_route.ipv6_dip0 = ntohl(ip6h->daddr.s6_addr32[0]);
				foe_entry->ipv6_5t_route.ipv6_dip1 = ntohl(ip6h->daddr.s6_addr32[1]);
				foe_entry->ipv6_5t_route.ipv6_dip2 = ntohl(ip6h->daddr.s6_addr32[2]);
				foe_entry->ipv6_5t_route.ipv6_dip3 = ntohl(ip6h->daddr.s6_addr32[3]);
			} else {
				rmt_header = 1;			// remove outer IPv6 header
			}
		}
		
		/* fill L2 info (80B) */
		FoeSetMacHiInfo(foe_entry->ipv6_5t_route.dmac_hi, eth->h_dest);
		FoeSetMacLoInfo(foe_entry->ipv6_5t_route.dmac_lo, eth->h_dest);
		FoeSetMacHiInfo(foe_entry->ipv6_5t_route.smac_hi, eth->h_source);
		FoeSetMacLoInfo(foe_entry->ipv6_5t_route.smac_lo, eth->h_source);
		foe_entry->ipv6_5t_route.pppoe_id = ppp_sid;
		foe_entry->ipv6_5t_route.vlan1 = vlan1_id;
		foe_entry->ipv6_5t_route.vlan2 = vlan2_id;
		foe_entry->ipv6_5t_route.etype = vlan_tag;
#endif
	} else {
		/* packet format is not supported */
		return 1;
	}

	/******************** L2 ********************/

	/*
	 * VLAN Layer:
	 * 0: outgoing packet is untagged packet
	 * 1: outgoing packet is tagged packet
	 * 2: outgoing packet is double tagged packet
	 * 3: outgoing packet is triple tagged packet
	 * 4: outgoing packet is fourfold tagged packet
	 */
	foe_entry->bfib1.vlan_layer = vlan_layer;

	foe_entry->bfib1.psn = (pppoe_gap) ? 1 : 0;
	foe_entry->bfib1.rmt = (rmt_header) ? 1 : 0;

#if defined (CONFIG_RALINK_MT7621)
	/* VLAN etype 0x8100 */
	foe_entry->bfib1.vpm = 1;
#else
	/* we set VID and VPRI in foe entry already, so we have to inform switch of keeping VPRI */
	foe_entry->bfib1.dvp = 1;

	/* inform switch of keeping DSCP */
	foe_entry->bfib1.drm = (drm_keep) ? 1 : 0;
#endif

	/******************** DST ********************/

#if defined (CONFIG_RA_HW_NAT_WIFI) || defined (CONFIG_RA_HW_NAT_PCI)
	/* CPU need to handle traffic between WLAN/PCI and GMAC port */
	if (gmac_no == 0) {
#if defined (CONFIG_RALINK_MT7621)
#if defined (CONFIG_RAETH_QDMATX_QDMARX)
		uint32_t fpidx = 5;	/* 5: to CPU (QDMA) */
#else
		uint32_t fpidx = 0;	/* 0: to CPU (PDMA) */
#endif
		if (is_mcast)
			fpidx |= 0x80;
#endif
#if defined (CONFIG_RALINK_MT7620)
		uint32_t fpidx = 6;	/* 6: force to P6 */
		if (is_mcast)
			fpidx = 8;	/* 8: no force port */
#endif
		if (IS_IPV4_GRP(foe_entry)) {
#if defined (CONFIG_RA_HW_NAT_QDMA)
			u32 QID = get_qid_ipv4(skb, 0);
			
			if (vlan_layer < 3) {
				foe_entry->bfib1.vpm = 0; /* etype remark (0x5678) */
				foe_entry->bfib1.vlan_layer = vlan_layer + 1;
				foe_entry->ipv4_hnapt.etype = __constant_ntohs(0x5678);
				if (vlan_layer > 0)
					foe_entry->ipv4_hnapt.vlan2 = vlan1_id;
				foe_entry->ipv4_hnapt.vlan1 = FOE_ENTRY_NUM(skb);
			}
			fpidx |= 0x8000;
			fpidx |= (QID << 8);
#endif
			PpeSetInfoBlk2(&foe_entry->ipv4_hnapt.iblk2, fpidx, 0x3f, 0x3f);
		}
#if defined (CONFIG_RA_HW_NAT_IPV6)
		else if (ipv6_offload && IS_IPV6_GRP(foe_entry)) {
#if defined (CONFIG_RA_HW_NAT_QDMA)
			u32 QID = get_qid_ipv6(skb, 0);
			
			if (vlan_layer < 3) {
				foe_entry->bfib1.vpm = 0; /* etype remark (0x5678) */
				foe_entry->bfib1.vlan_layer = vlan_layer + 1;
				foe_entry->ipv6_5t_route.etype = __constant_ntohs(0x5678);
				if (vlan_layer > 0)
					foe_entry->ipv6_5t_route.vlan2 = vlan1_id;
				foe_entry->ipv6_5t_route.vlan1 = FOE_ENTRY_NUM(skb);
			}
			fpidx |= 0x8000;
			fpidx |= (QID << 8);
#endif
			PpeSetInfoBlk2(&foe_entry->ipv6_5t_route.iblk2, fpidx, 0x3f, 0x3f);
		}
#endif
		/* set Pseudo Interface destination port in Foe entry */
		if (PpeSetExtIfNum(skb, foe_entry))
			return 1;
	} else
#endif
	{
		uint32_t port_ag = 1;	/* account group #1 */
#if defined (CONFIG_RALINK_MT7621)
		uint32_t fpidx = 1;	/* 1: to GE1 (GSW P6) */
#if defined (CONFIG_RAETH_GMAC2)
		if (gmac_no == 2) {
			fpidx = 2;	/* 2: to GE2 */
			port_ag = 2;	/* account group #2 (WAN) */
		}
#endif
		if (is_mcast)
			fpidx |= 0x80;
#endif
#if defined (CONFIG_RALINK_MT7620)
#if defined (CONFIG_RAETH_HAS_PORT5) && !defined (CONFIG_RAETH_HAS_PORT4) && !defined (CONFIG_RAETH_ESW)
		uint32_t fpidx = 5;	/* 5: force P5 */
#elif defined (CONFIG_RAETH_HAS_PORT4) && !defined (CONFIG_RAETH_HAS_PORT5) && !defined (CONFIG_RAETH_ESW)
		uint32_t fpidx = 4;	/* 4: force P4 */
#else
		uint32_t fpidx = 8;	/* 8: no force port (use DA) */
#endif
#endif
#if !defined (CONFIG_RAETH_BOTH_GMAC)
		if ((vlan1_id & VLAN_VID_MASK) != lan_vid)
			port_ag = 2;	/* account group #2 (WAN) */
#endif
		if (IS_IPV4_GRP(foe_entry)) {
#if defined (CONFIG_RA_HW_NAT_QDMA)
			if (FOE_SP(skb) != 5) {
				u32 QID = get_qid_ipv4(skb, (port_ag == 2));
				fpidx |= 0x8000;
				fpidx |= (QID << 8);
			}
#endif
			PpeSetInfoBlk2(&foe_entry->ipv4_hnapt.iblk2, fpidx, 0x3f, port_ag);
			foe_entry->ipv4_hnapt.act_dp = 0;	/* clear destination port for CPU */
		}
#if defined (CONFIG_RA_HW_NAT_IPV6)
		else if (ipv6_offload && IS_IPV6_GRP(foe_entry)) {
#if defined (CONFIG_RA_HW_NAT_QDMA)
			if (FOE_SP(skb) != 5) {
				u32 QID = get_qid_ipv6(skb, (port_ag == 2));
				fpidx |= 0x8000;
				fpidx |= (QID << 8);
			}
#endif
			PpeSetInfoBlk2(&foe_entry->ipv6_5t_route.iblk2, fpidx, 0x3f, port_ag);
			foe_entry->ipv6_5t_route.act_dp = 0;	/* clear destination port for CPU */
		}
#endif
	}

#if defined (CONFIG_PPE_MCAST) && defined (CONFIG_RA_HW_NAT_QDMA)
	if (is_mcast)
		foe_mcast_entry_qid(vlan1_id, eth->h_dest, M2Q_table[(skb->mark & 0x3f)]);
#endif

	/******************** BIND ********************/

	/* set current time to time_stamp field in information block 1 */
	foe_entry->bfib1.time_stamp = (uint16_t) (RegRead(FOE_TS) & 0xFFFF);

	/* Ipv4: TTL / Ipv6: Hop Limit filed */
	foe_entry->bfib1.ttl = (ttl_regen) ? 1 : 0;

	/* enable cache by default */
	foe_entry->bfib1.cah = 1;

	/* change FoE entry state to (pre)binding state */
#if defined (CONFIG_RA_HW_NAT_PREBIND)
	foe_entry->udib1.preb = 1;
#else
	foe_entry->bfib1.state = BIND;
#endif

#if !defined (CONFIG_RA_HW_NAT_PREBIND)
#if defined (CONFIG_RA_HW_NAT_DEBUG)
	/* Dump Binding Entry */
	if (DebugLevel >= 3) {
		FoeDumpEntry(FOE_ENTRY_NUM(skb));
	}
#endif
#endif

	return 0;
}
#else
int32_t FoeBindToPpe(struct sk_buff *skb, struct FoeEntry* foe_entry, int gmac_no)
{
	struct ethhdr *eth = (struct ethhdr *)skb->data;
	uint32_t vlan1_gap = 0;
	uint32_t vlan2_gap = 0;
	uint32_t pppoe_gap = 0;
	uint16_t ppp_tag = 0;
	uint16_t ppp_sid = 0;
	uint16_t vlan1_id = 0;
	uint16_t vlan2_id = 0;
	uint16_t eth_type;

	/* we cannot speed up multicase packets because both wire and wireless
	   PCs might join same multicast group. */
	if (is_multicast_ether_addr(eth->h_dest))
		return 1;

	eth_type = ntohs(eth->h_proto);

	/* offload packet types not support for extif:
	   1. VLAN tagged packets (avoid double tag issue).
	   2. PPPoE packets (PPPoE passthrough issue).
	   3. IPv6 1T routes. */
	if (gmac_no == 0 && eth_type != ETH_P_IP)
		return 1;

	/* if this entry is already in binding state, skip it */
	if (foe_entry->bfib1.state == BIND)
		return 1;

	if (eth_type == ETH_P_8021Q || skb_vlan_tag_present(skb)) {
		struct vlan_hdr *vh;
		
		if (eth_type == ETH_P_8021Q) {
			vh = (struct vlan_hdr *)(skb->data + ETH_HLEN);
			vlan1_gap = VLAN_HLEN;
			vlan1_id = ntohs(vh->h_vlan_TCI);
			eth_type = ntohs(vh->h_vlan_encapsulated_proto);
		} else {
			vlan1_id = skb_vlan_tag_get(skb);
		}
		
		/* VLAN + PPPoE */
		if (eth_type == ETH_P_PPP_SES) {
			struct pppoe_hdr *peh = (struct pppoe_hdr *)(skb->data + ETH_HLEN + vlan1_gap);
			if (peh->ver != 1 || peh->type != 1)
				return 1;
			pppoe_gap = 8;
			ppp_tag = ntohs(peh->tag[0].tag_type);
			ppp_sid = ntohs(peh->sid);
			
			/* Double VLAN = VLAN + VLAN */
		} else if (eth_type == ETH_P_8021Q) {
			vh = (struct vlan_hdr *)(skb->data + vlan1_gap + VLAN_HLEN);
			vlan2_gap = VLAN_HLEN;
			vlan2_id = ntohs(vh->h_vlan_TCI);
			eth_type = ntohs(vh->h_vlan_encapsulated_proto);
			
			/* VLAN + VLAN + PPPoE */
			if (eth_type == ETH_P_PPP_SES) {
				struct pppoe_hdr *peh = (struct pppoe_hdr *)(skb->data + ETH_HLEN + vlan1_gap + VLAN_HLEN);
				if (peh->ver != 1 || peh->type != 1)
					return 1;
				pppoe_gap = 8;
				ppp_tag = ntohs(peh->tag[0].tag_type);
				ppp_sid = ntohs(peh->sid);
			}
		}
	}
#if defined (CONFIG_RAETH_BOTH_GMAC)
	/* PPPoE + IP (RT3883 with 2xGMAC) */
	else if (eth_type == ETH_P_PPP_SES) {
		struct pppoe_hdr *peh = (struct pppoe_hdr *)(skb->data + ETH_HLEN);
		if (peh->ver != 1 || peh->type != 1)
			return 1;
		pppoe_gap = 8;
		ppp_tag = ntohs(peh->tag[0].tag_type);
		ppp_sid = ntohs(peh->sid);
	}
#endif

	if ((eth_type == ETH_P_IP) || (eth_type == ETH_P_PPP_SES && ppp_tag == PPP_IP)) {
		struct iphdr *iph = (struct iphdr *)(skb->data + ETH_HLEN + vlan1_gap + vlan2_gap + pppoe_gap);
		if(iph->frag_off & htons(IP_MF|IP_OFFSET))
			return 1;
		
		if (iph->protocol == IPPROTO_TCP) {
			/* fill L4 info */
			struct tcphdr *th = (struct tcphdr *)((uint8_t *)iph + iph->ihl * 4);
			foe_entry->ipv4_hnapt.new_sport = ntohs(th->source);
			foe_entry->ipv4_hnapt.new_dport = ntohs(th->dest);
			foe_entry->ipv4_hnapt.bfib1.udp = TCP;
		} else if (iph->protocol == IPPROTO_UDP) {
			struct udphdr *uh;
			if (!udp_offload)
				return 1;
			
			uh = (struct udphdr *)((uint8_t *)iph + iph->ihl * 4);
			/* check PPE bug */
			if (ppe_udp_bug) {
				if (!uh->check)
					return 1;
				if (uh->dest == __constant_htons(500) ||	// IPSec IKE
				    uh->dest == __constant_htons(4500) ||	// IPSec NAT-T
				    uh->dest == __constant_htons(1701))		// L2TP
					return 1;
			}
			
			/* fill L4 info */
			foe_entry->ipv4_hnapt.new_sport = ntohs(uh->source);
			foe_entry->ipv4_hnapt.new_dport = ntohs(uh->dest);
			foe_entry->ipv4_hnapt.bfib1.udp = UDP;
		} else {
			/* packet format is not supported */
			return 1;
		}
		
		/* fill L3 info */
		foe_entry->ipv4_hnapt.new_sip = ntohl(iph->saddr);
		foe_entry->ipv4_hnapt.new_dip = ntohl(iph->daddr);
		foe_entry->ipv4_hnapt.iblk2.dscp = iph->tos;
		
#if defined (CONFIG_RA_HW_NAT_IPV6)
	} else if (eth_type == ETH_P_IPV6 || (eth_type == ETH_P_PPP_SES && ppp_tag == PPP_IPV6)) {
		if (!ipv6_offload)
			return 1;
#endif
	} else {
		return 1;
	}

	/******************** L2 ********************/

	/* Set MAC Info */
	FoeSetMacHiInfo(foe_entry->ipv4_hnapt.dmac_hi, eth->h_dest);
	FoeSetMacLoInfo(foe_entry->ipv4_hnapt.dmac_lo, eth->h_dest);
	FoeSetMacHiInfo(foe_entry->ipv4_hnapt.smac_hi, eth->h_source);
	FoeSetMacLoInfo(foe_entry->ipv4_hnapt.smac_lo, eth->h_source);

	foe_entry->ipv4_hnapt.vlan1 = vlan1_id;
	foe_entry->ipv4_hnapt.pppoe_id = ppp_sid;
	foe_entry->ipv4_hnapt.vlan2 = vlan2_id;

	/*
	 * PPE support SMART VLAN/PPPoE Tag Push/PoP feature
	 *
	 *         | MODIFY | INSERT | DELETE
	 * --------+--------+--------+----------
	 * Tagged  | modify | modify | delete
	 * Untagged| no act | insert | no act
	 *
	 */
	if (vlan1_gap || vlan1_id > 0) // check HW_VLAN_TX
		foe_entry->bfib1.v1 = INSERT;
	else
		foe_entry->bfib1.v1 = DELETE;

	if (vlan2_gap)
		foe_entry->bfib1.v2 = INSERT;
	else
		foe_entry->bfib1.v2 = DELETE;

	if (pppoe_gap)
		foe_entry->bfib1.pppoe = INSERT;
	else
		foe_entry->bfib1.pppoe = DELETE;

	/******************** DST ********************/

	foe_entry->ipv4_hnapt.iblk2.fd = 1;

#if defined (CONFIG_RA_HW_NAT_WIFI) || defined (CONFIG_RA_HW_NAT_PCI)
	/* CPU need to handle traffic between WLAN/PCI and GMAC port */
	if (gmac_no == 0) {
		foe_entry->ipv4_hnapt.iblk2.dp = 0;		/* -> CPU */
		
		/* set Pseudo Interface destination port in Foe entry */
		if (PpeSetExtIfNum(skb, foe_entry))
			return 1;
	} else
#endif
	{
#if defined (CONFIG_RAETH_GMAC2)
		/* RT3883 with 2xGMAC - assuming GMAC2=WAN and GMAC1=LAN */
		foe_entry->ipv4_hnapt.iblk2.dp = (gmac_no == 2) ? 2 : 1;
#elif defined (CONFIG_RALINK_RT3883)
		/* RT3883 (1xGMAC mode), always send to GMAC1 */
		foe_entry->ipv4_hnapt.iblk2.dp = 1;		/* -> GMAC 1 */
#else
		/* RT3052, RT335x */
		if ((vlan1_id & VLAN_VID_MASK) != lan_vid)
			foe_entry->ipv4_hnapt.iblk2.dp = 2;	/* -> VirtualPort2 in GMAC1 */
		else
			foe_entry->ipv4_hnapt.iblk2.dp = 1;	/* -> VirtualPort1 in GMAC1 */
#endif
		foe_entry->ipv4_hnapt.act_dp = 0;	/* clear destination port for CPU */
	}

	/******************** BIND ********************/

	/* set current time to time_stamp field in information block 1 */
	foe_entry->bfib1.time_stamp = (uint16_t) (RegRead(FOE_TS) & 0xFFFF);

	/* Ipv4: TTL / Ipv6: Hop Limit filed */
	foe_entry->bfib1.ttl = (ttl_regen) ? 1 : 0;

#if defined (CONFIG_RA_HW_NAT_ACL2UP_HELPER)
	/* set user priority */
	foe_entry->ipv4_hnapt.iblk2.up = FOE_SP(skb);
	foe_entry->ipv4_hnapt.iblk2.fp = 1;
#endif

	/* change FoE entry state to binding state */
	foe_entry->bfib1.state = BIND;

#if defined (CONFIG_RA_HW_NAT_DEBUG)
	/* Dump Binding Entry */
	if (DebugLevel >= 3) {
		FoeDumpEntry(FOE_ENTRY_NUM(skb));
	}
#endif

	return 0;
}
#endif

int32_t PpeTxHandler(struct sk_buff *skb, int gmac_no)
{
	struct FoeEntry *foe_entry;
	uint32_t foe_ai, foe_entry_num;

	/* check traffic from WiFi/ExtIf (gmac_no = 0) */
	if (gmac_no == 0) {
#if defined (CONFIG_RA_HW_NAT_WIFI) || defined (CONFIG_RA_HW_NAT_PCI)
		if (!wifi_offload)
#endif
			return 1;
	}

	/* return truncated packets to normal path with padding */
	if (!skb || skb->len < ETH_HLEN)
		return 1;

	/* check FoE packet tag from GE */
	if (!IS_SPACE_AVAILABLED(skb) || !IS_MAGIC_TAG_VALID(skb))
		return 1;

	foe_ai = FOE_AI(skb);

	/* check FoE AI for local traffic */
	if (foe_ai == UN_HIT)
		return 1;

	foe_entry_num = FOE_ENTRY_NUM(skb);
	if (foe_entry_num >= FOE_4TB_SIZ || !FOE_ENTRY_VALID(skb))
		return 1;

	foe_entry = get_foe_entry(foe_entry_num);

	/*
	 * Packet is interested by ALG?
	 * Yes: Don't enter bindind state
	 * No: If flow rate exceed binding threshold, enter binding state.
	 *     IPV6_1T require binding for all time (binding threshold=0).
	 */
	if (FOE_ALG(skb) == 0 && ((foe_ai == HIT_UNBIND_RATE_REACH)
#if !defined (CONFIG_HNAT_V2) && defined (CONFIG_RA_HW_NAT_IPV6)
			       || (foe_ai == HIT_UNBIND && IS_IPV6_1T_ROUTE(foe_entry))
#endif
	  )) {
		spin_lock_bh(&ppe_foe_lock);
		if (FoeBindToPpe(skb, foe_entry, gmac_no)) {
			FOE_AI(skb) = UN_HIT;
		}
		spin_unlock_bh(&ppe_foe_lock);
		
#if defined (CONFIG_HNAT_V2)
	} else if (foe_ai == HIT_BIND_KEEPALIVE_MC_NEW_HDR || foe_ai == HIT_BIND_KEEPALIVE_DUP_OLD_HDR) {
#else
	} else if (foe_ai == HIT_BIND_KEEPALIVE && DFL_FOE_KA == 0) {
#endif
		/* check duplicate packet in keepalive new header mode, just drop it */
		DO_FAST_CLEAR_FOE(skb);
		return 0;
#if defined (CONFIG_RA_HW_NAT_DEBUG)
	} else if (foe_ai == HIT_UNBIND_RATE_REACH && FOE_ALG(skb) == 1) {
		if (DebugLevel >= 4) {
			NAT_DEBUG("FOE_ALG=1 (Entry=%d)\n", foe_entry_num);
		}
#endif
#if defined (CONFIG_RA_HW_NAT_PREBIND)
	} else if (foe_ai == HIT_PRE_BIND) {
		spin_lock_bh(&ppe_foe_lock);
		if (foe_entry->udib1.preb && foe_entry->bfib1.state != BIND) {
			foe_entry->bfib1.state = BIND;
			foe_entry->udib1.preb = 0;
			spin_unlock_bh(&ppe_foe_lock);
		} else {
			/* drop duplicate prebind notify packet */
			DO_FAST_CLEAR_FOE(skb);
			spin_unlock_bh(&ppe_foe_lock);
			return 0;
		}
#if defined (CONFIG_RA_HW_NAT_DEBUG)
		/* Dump Binding Entry */
		if (DebugLevel >= 3) {
			FoeDumpEntry(foe_entry_num);
		}
#endif
#endif
	}

	return 1;
}

#if !defined(CONFIG_HNAT_V2)
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
#else
/* token_rate: unit= KB/S */
int32_t PpeSetMtrByteInfo(uint16_t MgrIdx, uint32_t TokenRate, uint32_t MaxBkSize)
{
	uint32_t MtrEntry = 0;

	MtrEntry = ((TokenRate << 3) | (MaxBkSize << 1));
#if defined (CONFIG_RALINK_MT7620)
	RegWrite(METER_BASE + MgrIdx * 4, MtrEntry);
	NAT_DEBUG("Meter Table Base=%08X Offset=%d\n", METER_BASE, MgrIdx * 4);
	NAT_DEBUG("%08X: %08X\n", METER_BASE + MgrIdx * 4, MtrEntry);
#elif defined (CONFIG_RALINK_MT7621)
	RegWrite(METER_BASE + MgrIdx * 16 + 12, MtrEntry);
	NAT_DEBUG("Meter Table Base=%08X Offset=%d\n", METER_BASE, MgrIdx * 16 + 12);
	NAT_DEBUG("%08X: %08X\n", METER_BASE + MgrIdx * 12, MtrEntry);
#endif

	return 1;
}

/*
 * MtrIntval:
 * 0x0 = 1
 * 0x1 = 10
 * 0x2 = 50
 * 0x3 = 100
 * 0x4 = 500
 * 0x5 = 1000
 * 0x6 = 5000
 * 0x7 = 10000
 */
int32_t PpeSetMtrPktInfo(uint16_t MgrIdx, uint32_t MtrIntval, uint32_t MaxBkSize)
{
	uint32_t MtrEntry = 0;

	MtrEntry = ((MtrIntval << 8) | (MaxBkSize << 1) | 1);
	RegWrite(METER_BASE + MgrIdx * 4, MtrEntry);

	NAT_DEBUG("Meter Table Base=%08X Offset=%d\n", METER_BASE, MgrIdx * 4);
	NAT_DEBUG("%08X: %08X\n", METER_BASE + MgrIdx * 4, MtrEntry);

	return 1;
}
#endif

void PpeSetFoeEbl(uint32_t FoeEbl)
{
	uint32_t PpeFlowSet = 0;

	PpeFlowSet = RegRead(PPE_FLOW_SET);

	/* FOE engine need to handle unicast/multicast/broadcast flow */
	if (FoeEbl == 1) {
		PpeFlowSet |= (BIT_IPV4_NAPT_EN | BIT_IPV4_NAT_EN);
#if defined (CONFIG_HNAT_V2)
		PpeFlowSet |= (BIT_IPV4_NAT_FRAG_EN); //ip fragment
		PpeFlowSet |= (BIT_IPV4_HASH_GREK);
#if defined (CONFIG_RA_HW_NAT_IPV6)
		if (ipv6_offload) {
			PpeFlowSet |= (BIT_IPV4_DSL_EN | BIT_IPV6_6RD_EN | BIT_IPV6_3T_ROUTE_EN | BIT_IPV6_5T_ROUTE_EN);
//			PpeFlowSet |= (BIT_IPV6_HASH_FLAB); // flow label
			PpeFlowSet |= (BIT_IPV6_HASH_GREK);
		}
#endif
#else
		PpeFlowSet |= (BIT_FUC_FOE);
#if defined (CONFIG_RA_HW_NAT_IPV6)
		if (ipv6_offload)
			PpeFlowSet |= (BIT_IPV6_FOE_EN);
#endif
#endif
	} else {
		PpeFlowSet &= ~(BIT_IPV4_NAPT_EN | BIT_IPV4_NAT_EN);
#if defined (CONFIG_HNAT_V2)
		PpeFlowSet &= ~(BIT_IPV4_NAT_FRAG_EN);
		PpeFlowSet &= ~(BIT_IPV4_DSL_EN | BIT_IPV6_6RD_EN | BIT_IPV6_3T_ROUTE_EN | BIT_IPV6_5T_ROUTE_EN);
		PpeFlowSet &= ~(BIT_IPV6_HASH_FLAB);
		PpeFlowSet &= ~(BIT_IPV6_HASH_GREK);
#else
		PpeFlowSet &= ~(BIT_FUC_FOE | BIT_FMC_FOE | BIT_FBC_FOE);
		PpeFlowSet &= ~(BIT_IPV6_FOE_EN);
#endif
	}

	RegWrite(PPE_FLOW_SET, PpeFlowSet);
}

static void PpeSetFoeHashMode(uint32_t HashMode)
{
	memset(PpeFoeBase, 0, PpeFoeTblSize * sizeof(struct FoeEntry));

#if defined (CONFIG_RA_HW_NAT_IPV6) && defined (CONFIG_HNAT_V2)
	if (ipv6_offload) {
		/* these entries are bad every 128 entries */
		int boundary_entry_offset[7] = {12, 25, 38, 51, 76, 89, 102};
		int entry_base = 0, bad_entry, i, j;
		
		for (i = 0; entry_base < FOE_4TB_SIZ; i++) {
			/* set bad entries as static */
			for (j = 0; j < 7; j++) {
				bad_entry = entry_base + boundary_entry_offset[j];
				PpeFoeBase[bad_entry].udib1.sta = 1;
			}
			entry_base = (i+1)*128;
		}
	}
#endif

	wmb();

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
#if defined (CONFIG_HNAT_V2)
	RegModifyBits(PPE_FOE_CFG, HashMode, 14, 2);
	RegWrite(PPE_HASH_SEED, HASH_SEED);
#if defined (CONFIG_RA_HW_NAT_IPV6)
	if (ipv6_offload)
		RegModifyBits(PPE_FOE_CFG, 1, 3, 1);	// entry size = 80bytes
	else
#endif
		RegModifyBits(PPE_FOE_CFG, 0, 3, 1);	// entry size = 64bytes

#if defined (CONFIG_RA_HW_NAT_PREBIND)
	RegModifyBits(PPE_FOE_CFG, 1, 6, 1);	// pre-bind age enable
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

#if defined (CONFIG_RA_HW_NAT_PREBIND)
	RegModifyBits(PPE_BIND_LMT_1, DFL_PBND_RD_LMT, 24, 8);
#endif
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
	RegModifyBits(PPE_FOE_BNDR, FoeBindRate, 0, 16);

#if defined (CONFIG_RA_HW_NAT_PREBIND)
	RegModifyBits(PPE_FOE_BNDR, DFL_PBND_RD_PRD, 16, 16);
#endif
}

static void PpeSetFoeGloCfgEbl(uint32_t Ebl)
{
	if (Ebl == 1) {
#if defined (CONFIG_RALINK_MT7620)
		uint32_t tpf = 0;
		
		/* 1. Remove P7 on forwarding ports */
		/* It's chip default setting */

		/* 2. PPE Forward Control Register: PPE_PORT=Port7 */
		RegModifyBits(PFC, 7, 0, 3);

		/* 3. Select P7 as PPE port (PPE_EN=1) */
		RegModifyBits(PFC, 1, 3, 1);

		/* TO_PPE Forwarding Register (exclude broadcast) */
		tpf = IPV4_PPE_MYUC | IPV4_PPE_UC | IPV4_PPE_UN;
#if defined (CONFIG_RA_HW_NAT_IPV6)
		if (ipv6_offload)
			tpf |= (IPV6_PPE_MYUC | IPV6_PPE_UC | IPV6_PPE_UN);
#endif
#if defined (CONFIG_RA_HW_NAT_MCAST)
		tpf |= (IPV4_PPE_MC | IPV4_PPE_IPM);
#if defined (CONFIG_RA_HW_NAT_IPV6)
		if (ipv6_offload)
			tpf |= (IPV6_PPE_MC | IPV6_PPE_IPM);
#endif
#endif

#if defined (CONFIG_RAETH_ESW)
		RegWrite(TPF0, tpf);
		RegWrite(TPF1, tpf);
		RegWrite(TPF2, tpf);
		RegWrite(TPF3, tpf);
#endif
#if defined (CONFIG_RAETH_HAS_PORT4) || defined (CONFIG_RAETH_ESW)
		RegWrite(TPF4, tpf);
#endif
#if defined (CONFIG_RAETH_HAS_PORT5)
		RegWrite(TPF5, tpf);
#endif
		/* Forced Port7 link up, 1Gbps, and Full duplex  */
		RegWrite(PMCR_P7, 0x5e33b);

		/* Disable SA Learning */
		RegModifyBits(PSC_P7, 1, 4, 1);

		/* Use default values on P7 */
#elif defined (CONFIG_RALINK_MT7621)
		/* PPE Engine Enable */
		RegModifyBits(PPE_GLO_CFG, 1, 0, 1);

#if defined (CONFIG_RA_HW_NAT_MCAST)
		/* Enable multicast table lookup */
		RegModifyBits(PPE_GLO_CFG, 1, 7, 1);
		RegModifyBits(PPE_GLO_CFG, 1, 12, 2);    // Reserve 16 entry for multicast packet
		RegModifyBits(PPE_MCAST_PPSE, 1, 4, 4);  // multicast port1 map to GMAC1
		RegModifyBits(PPE_MCAST_PPSE, 2, 8, 4);  // multicast port2 map to GMAC2
#if defined (CONFIG_RAETH_QDMATX_QDMARX)
		RegModifyBits(PPE_MCAST_PPSE, 5, 12, 4); // multicast port3 map to QDMA
#else
		RegModifyBits(PPE_MCAST_PPSE, 0, 0, 4);  // multicast port0 map to PDMA
#endif
#endif /* CONFIG_RA_HW_NAT_MCAST */
#if defined (CONFIG_RAETH_QDMATX_QDMARX)
		/* default CPU port is port5 (QDMA) */
		RegWrite(PPE_DFT_CPORT, 0x55555555);
#else
		/* default CPU port is port0 (PDMA) */
		RegWrite(PPE_DFT_CPORT, 0);
#endif
#else /* !CONFIG_RALINK_MT7621 */
		/* PPE Engine Enable */
		RegModifyBits(PPE_GLO_CFG, 1, 0, 1);

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
#endif
		/* PPE Packet with TTL=0 */
		RegModifyBits(PPE_GLO_CFG, DFL_TTL0_DRP, 4, 1);
	} else {
#if defined (CONFIG_RALINK_MT7620)
		/* 1. Select P7 as PPE port (PPE_EN=0) */
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
#else
		/* PPE Engine Disable */
		RegModifyBits(PPE_GLO_CFG, 0, 0, 1);
#endif
	}
}

#if !defined (CONFIG_HNAT_V2)
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
#endif

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

#if !defined (CONFIG_HNAT_V2)
	/* Set User Priority related register */
	PpeSetUserPriority();
#endif

	/* which protocol type should be handle by HNAT not HNAPT */
	PpeSetHNATProtoType();

#if defined (CONFIG_RALINK_MT7620)
	/* Turn On UDP Control */
	if ((ralink_asic_rev_id & 0xF) >= 5) {
		uint32_t reg = RegRead(RALINK_PPE_BASE + 0x380);
		reg &= ~(0x1 << 30);
		RegWrite(RALINK_PPE_BASE + 0x380, reg);
	}
#endif

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

	wmb();

	/* Unbind FOE table */
	RegWrite(PPE_FOE_BASE, 0);

	return 0;
}

int PpeRsHandler(struct net_device *dev, int hold)
{
#if defined (CONFIG_RA_HW_NAT_PCI)
	if (!dev)
		return -1;

	if (hold) {
		if (DstPort[DP_NIC0] == dev || DstPort[DP_NIC1] == dev)
			return 1;
		if (!DstPort[DP_NIC0]) {
			dev_hold(dev);
			DstPort[DP_NIC0] = dev;
			SetDstPortIndex(dev, DP_NIC0);
			return 0;
		}
		else if (!DstPort[DP_NIC1]) {
			dev_hold(dev);
			DstPort[DP_NIC1] = dev;
			SetDstPortIndex(dev, DP_NIC1);
			return 0;
		}
	} else {
		if (DstPort[DP_NIC0] == dev) {
			SetDstPortIndex(dev, 0);
			DstPort[DP_NIC0] = NULL;
			dev_put(dev);
			return 0;
		}
		else if (DstPort[DP_NIC1] == dev) {
			SetDstPortIndex(dev, 0);
			DstPort[DP_NIC1] = NULL;
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
#if defined (HWNAT_USE_IF_MBSS)
		DstPort[DP_RA1] = ra_dev_get_by_name("ra1");
		DstPort[DP_RA2] = ra_dev_get_by_name("ra2");
		DstPort[DP_RA3] = ra_dev_get_by_name("ra3");
		DstPort[DP_RA4] = ra_dev_get_by_name("ra4");
		DstPort[DP_RA5] = ra_dev_get_by_name("ra5");
		DstPort[DP_RA6] = ra_dev_get_by_name("ra6");
		DstPort[DP_RA7] = ra_dev_get_by_name("ra7");
#endif
#if defined (HWNAT_USE_IF_WDS)
		DstPort[DP_WDS0] = ra_dev_get_by_name("wds0");
		DstPort[DP_WDS1] = ra_dev_get_by_name("wds1");
		DstPort[DP_WDS2] = ra_dev_get_by_name("wds2");
		DstPort[DP_WDS3] = ra_dev_get_by_name("wds3");
#endif
#if defined (HWNAT_USE_IF_APCLI)
		DstPort[DP_APCLI0] = ra_dev_get_by_name("apcli0");
#endif
#if defined (HWNAT_USE_IF_MESH)
		DstPort[DP_MESH0] = ra_dev_get_by_name("mesh0");
#endif

#if defined (HWNAT_USE_SECOND_IF)
		DstPort[DP_RAI0] = ra_dev_get_by_name("rai0");
#if defined (HWNAT_USE_IF_MBSS)
		DstPort[DP_RAI1] = ra_dev_get_by_name("rai1");
		DstPort[DP_RAI2] = ra_dev_get_by_name("rai2");
		DstPort[DP_RAI3] = ra_dev_get_by_name("rai3");
		DstPort[DP_RAI4] = ra_dev_get_by_name("rai4");
		DstPort[DP_RAI5] = ra_dev_get_by_name("rai5");
		DstPort[DP_RAI6] = ra_dev_get_by_name("rai6");
		DstPort[DP_RAI7] = ra_dev_get_by_name("rai7");
#endif
#if defined (HWNAT_USE_IF_WDS)
		DstPort[DP_WDSI0] = ra_dev_get_by_name("wdsi0");
		DstPort[DP_WDSI1] = ra_dev_get_by_name("wdsi1");
		DstPort[DP_WDSI2] = ra_dev_get_by_name("wdsi2");
		DstPort[DP_WDSI3] = ra_dev_get_by_name("wdsi3");
#endif
#if defined (HWNAT_USE_IF_APCLI)
		DstPort[DP_APCLII0] = ra_dev_get_by_name("apclii0");
#endif
#if defined (HWNAT_USE_IF_MESH)
		DstPort[DP_MESHI0] = ra_dev_get_by_name("meshi0");
#endif
#endif

		DstPort[DP_GMAC1] = ra_dev_get_by_name("eth2");
#if defined (CONFIG_RAETH_BOTH_GMAC)
		DstPort[DP_GMAC2] = ra_dev_get_by_name("eth3");
#endif
#if defined (CONFIG_RA_HW_NAT_PCI)
		i = DP_NIC0;
		DstPort[i] = ra_dev_get_by_name("weth0");	// USB interface name
		if (DstPort[i]) i = DP_NIC1;
		DstPort[i] = ra_dev_get_by_name("wwan0");	// USB WWAN interface name
#endif
		for (i = 0; i < MAX_IF_NUM; i++) {
			if (DstPort[i])
				SetDstPortIndex(DstPort[i], i);
		}
	} else {
		for (i = 0; i < MAX_IF_NUM; i++) {
			if (DstPort[i]) {
				SetDstPortIndex(DstPort[i], 0);
				dev_put(DstPort[i]);
				DstPort[i] = NULL;
			}
		}
	}
}

uint32_t SetGdmaFwd(uint32_t Ebl)
{
	uint32_t data;

#if defined (CONFIG_RALINK_MT7620)
	data = RegRead(GDM2_FWD_CFG);
	data &= ~0x7777;
	if (Ebl) {
		/* GDM2 U/B/M/O frames forward to PPE */
		data |= GDM2_UFRC_P_PPE;
		data |= GDM2_BFRC_P_PPE;
		data |= GDM2_MFRC_P_PPE;
		data |= GDM2_OFRC_P_PPE;
	} else {
		/* GDM2 U/B/M/O frames discard */
		data |=  0x7777;
	}
	RegWrite(GDM2_FWD_CFG, data);
#else
#if !defined (CONFIG_GE1_RGMII_NONE)
	data = RegRead(FE_GDMA1_FWD_CFG);
	data &= ~0x7777;
	if (Ebl) {
		/* Uni-cast frames forward to PPE */
		data |= GDM1_UFRC_P_PPE;
		/* Broad-cast MAC address frames forward to CPU */
		data |= GDM1_BFRC_P_CPU;
#if defined (CONFIG_RA_HW_NAT_MCAST) && defined (CONFIG_HNAT_V2)
		/* Multi-cast MAC address frames forward to PPE */
		data |= GDM1_MFRC_P_PPE;
#else
		/* Multi-cast MAC address frames forward to CPU */
		data |= GDM1_MFRC_P_CPU;
#endif
		/* Other MAC address frames forward to PPE */
		data |= GDM1_OFRC_P_PPE;
	} else {
		/* Uni-cast frames forward to CPU */
		data |= GDM1_UFRC_P_CPU;
		/* Broad-cast MAC address frames forward to CPU */
		data |= GDM1_BFRC_P_CPU;
		/* Multi-cast MAC address frames forward to CPU */
		data |= GDM1_MFRC_P_CPU;
		/* Other MAC address frames forward to CPU */
		data |= GDM1_OFRC_P_CPU;
	}
	RegWrite(FE_GDMA1_FWD_CFG, data);
#endif

#if defined (CONFIG_RAETH_GMAC2)
	data = RegRead(FE_GDMA2_FWD_CFG);
	data &= ~0x7777;
	if (Ebl) {
		/* Uni-cast frames forward to PPE */
		data |= GDM1_UFRC_P_PPE;
		/* Broad-cast MAC address frames forward to CPU */
		data |= GDM1_BFRC_P_CPU;
#if defined (CONFIG_RA_HW_NAT_MCAST) && defined (CONFIG_HNAT_V2)
		/* Multi-cast MAC address frames forward to PPE */
		data |= GDM1_MFRC_P_PPE;
#else
		/* Multi-cast MAC address frames forward to CPU */
		data |= GDM1_MFRC_P_CPU;
#endif
		/* Other MAC address frames forward to PPE */
		data |= GDM1_OFRC_P_PPE;
	} else {
		/* Uni-cast frames forward to CPU */
		data |= GDM1_UFRC_P_CPU;
		/* Broad-cast MAC address frames forward to CPU */
		data |= GDM1_BFRC_P_CPU;
		/* Multi-cast MAC address frames forward to CPU */
		data |= GDM1_MFRC_P_CPU;
		/* Other MAC address frames forward to CPU */
		data |= GDM1_OFRC_P_CPU;
	}
	RegWrite(FE_GDMA2_FWD_CFG, data);
#endif
#endif
	return 0;
}

#if defined (CONFIG_HNAT_V2)
static void PpeSetCacheEbl(void)
{
	/* clear cache table before enabling cache */
	RegModifyBits(CAH_CTRL, 1, 9, 1);
	RegModifyBits(CAH_CTRL, 0, 9, 1);

	/* Cache enable */
	RegModifyBits(CAH_CTRL, 1, 0, 1);
}

static void PpeSetSwitchVlanChk(int Ebl)
{
#if defined (CONFIG_RALINK_MT7620)
#if defined (CONFIG_RAETH_ESW) || (defined (CONFIG_RAETH_HAS_PORT5) && defined (CONFIG_RAETH_HAS_PORT4))
	uint32_t reg_p6, reg_p7;

	reg_p6 = RegRead(RALINK_ETH_SW_BASE + 0x2604);

	/* check P6 in Port Matrix Mode (dumb switch) */
	if ((reg_p6 & 0x03) == 0x0)
		return;

	reg_p7 = RegRead(RALINK_ETH_SW_BASE + 0x2704);

	reg_p6 &= ~0xff0003;
	reg_p7 &= ~0xff0003;

	/* port6&7: fall back mode / same port matrix group */
	if (Ebl) {
		reg_p6 |= 0xc00003;
		reg_p7 |= 0xc00003;
#if defined (CONFIG_RAETH_ESW)
		reg_p6 |= 0x0f0000;
		reg_p7 |= 0x0f0000;
#endif
#if defined (CONFIG_RAETH_HAS_PORT4) || defined (CONFIG_RAETH_ESW)
		reg_p6 |= 0x100000;
		reg_p7 |= 0x100000;
#endif
#if defined (CONFIG_RAETH_HAS_PORT5)
		reg_p6 |= 0x200000;
		reg_p7 |= 0x200000;
#endif
	} else {
		reg_p6 |= 0xc00001;
		reg_p7 |= 0xc00001;
	}

	RegWrite(RALINK_ETH_SW_BASE + 0x2604, reg_p6);
	RegWrite(RALINK_ETH_SW_BASE + 0x2704, reg_p7);
#endif
#endif
}

static void PpeSetFpBMAP(void)
{
#if defined (CONFIG_RALINK_MT7620)
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
#endif
}

static void PpeSetIpProt(void)
{
	/* IP Protocol Field for IPv4 NAT or IPv6 3-tuple flow */
	/* Don't forget to turn on related bits in PPE_IP_PROT_CHK register if you want to support 
	 * another IP protocol. 
	 */
	/* FIXME: enable it to support IP fragement */
	RegWrite(PPE_IP_PROT_CHK, 0xFFFFFFFF); //IPV4_NXTH_CHK and IPV6_NXTH_CHK
//	RegModifyBits(PPE_IP_PROT_0, IPPROTO_GRE, 0, 8);
//	RegModifyBits(PPE_IP_PROT_0, IPPROTO_TCP, 8, 8);
//	RegModifyBits(PPE_IP_PROT_0, IPPROTO_UDP, 16, 8);
//	RegModifyBits(PPE_IP_PROT_0, IPPROTO_IPV6, 24, 8);
}
#endif

int PpeEcHandler(int engine_init)
{
	if (engine_init) {
		/* Set PPE FOE Hash Mode */
		PpeSetFoeHashMode(DFL_FOE_HASH_MODE);
		
#if !defined (CONFIG_HNAT_V2)
		PpeSetRuleSize(PRE_ACL_SIZE, PRE_MTR_SIZE, PRE_AC_SIZE, POST_MTR_SIZE, POST_AC_SIZE);
		
		/* 0~63 Accounting group */
		PpeSetAGInfo(1, lan_vid);	// AG Index1=VLAN1
		PpeSetAGInfo(2, wan_vid);	// AG Index2=VLAN2
#else
		PpeSetFpBMAP();
		PpeSetIpProt();
		PpeSetCacheEbl();
		PpeSetSwitchVlanChk(0);
		
		/* 0~63 Metering group */
//		PpeSetMtrByteInfo(1, 500, 3);	// TokenRate=500=500KB/s, MaxBkSize= 3 (32K-1B)
//		PpeSetMtrPktInfo(1, 5, 3);	// 1 pkts/sec, MaxBkSize=3 (32K-1B)
#endif
		/* Initialize PPE related register */
		PpeEngStart();
		
		/* Register RX/TX hook point */
		ra_sw_nat_hook_tx = PpeTxHandler;
		ra_sw_nat_hook_rx = PpeRxHandler;
		
		/* Set GMAC forwards packet to PPE */
		SetGdmaFwd(1);
	} else {
		/* Set GMAC forwards packet to CPU */
		SetGdmaFwd(0);
		
		/* Unregister RX/TX hook point */
		ra_sw_nat_hook_rx = NULL;
		ra_sw_nat_hook_tx = NULL;
		
		/* Restore PPE related register */
		PpeEngStop();
		
#if defined (CONFIG_HNAT_V2)
		PpeSetSwitchVlanChk(1);
#endif
#if defined (CONFIG_PPE_MCAST)
		foe_mcast_entry_del_all();
#endif
	}

	return 0;
}

/*
 * PPE Enabled: GMAC<->PPE<->CPU
 * PPE Disabled: GMAC<->CPU
 */
static int __init PpeInitMod(void)
{
#if defined (CONFIG_RALINK_RT3052)
	/* RT3052 with RF_REG0 > 0x53 has no bug UDP w/o checksum */
	uint32_t phy_val = 0;
	extern int rw_rf_reg(int write, int reg, int *data);

	rw_rf_reg(0, 0, &phy_val);
	ppe_udp_bug = ((phy_val & 0xFF) > 0x53) ? 0 : 1;
#elif defined (CONFIG_RALINK_RT3352)
	/* RT3352 rev 0105 has no bug UDP w/o checksum */
	ppe_udp_bug = ((ralink_asic_rev_id & 0xFFFF) < 0x0105) ? 1 : 0;
#elif defined (CONFIG_RALINK_RT3883)
	/* RT3883/RT3662 at least rev 0105 has bug UDP w/o checksum :-( */
	ppe_udp_bug = 1;
#elif defined (CONFIG_RALINK_MT7620)
	/* MT7620 rev 0205 has no bug UDP w/o checksum */
	ppe_udp_bug = ((ralink_asic_rev_id & 0xF) < 5) ? 1 : 0;
#endif

	memset(DstPort, 0, sizeof(DstPort));
	memset(DstPortHash, 0, sizeof(DstPortHash));

	/* Get allocated FoE table from raeth */
	PpeFoeBase = get_foe_table(&PpeFoeBasePhy, &PpeFoeTblSize);
	if (!PpeFoeBase)
		return -ENOMEM;

	/* Hold net_device structure of Dest Port */
	PpeSetDstPort(1);

	/* Engine Start */
	PpeEcHandler(1);

	/* Register ioctl handler */
	PpeRegIoctlHandler();
#if !defined (CONFIG_HNAT_V2)
	AclRegIoctlHandler();
	AcRegIoctlHandler();
	MtrRegIoctlHandler();
#endif

	/* Register control hook points */
	ra_sw_nat_hook_rs = PpeRsHandler;
	ra_sw_nat_hook_ec = PpeEcHandler;

	printk("Ralink HW NAT %s Module Enabled, FoE Size: %d\n",
		HW_NAT_MODULE_VER, PpeFoeTblSize);

	return 0;
}

static void __exit PpeCleanupMod(void)
{
	printk("Ralink HW NAT %s Module Disabled\n", HW_NAT_MODULE_VER);

	/* Unregister control hook points */
	ra_sw_nat_hook_rs = NULL;
	ra_sw_nat_hook_ec = NULL;

	/* Engine Stop */
	PpeEcHandler(0);

	/* Unregister ioctl handler */
	PpeUnRegIoctlHandler();
#if !defined (CONFIG_HNAT_V2)
	AclUnRegIoctlHandler();
	AcUnRegIoctlHandler();
	MtrUnRegIoctlHandler();
#endif

	/* Release net_device structure of Dest Port */
	PpeSetDstPort(0);
}

module_init(PpeInitMod);
module_exit(PpeCleanupMod);

MODULE_AUTHOR("Steven Liu/Kurtis Ke");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Ralink Hardware NAT");

