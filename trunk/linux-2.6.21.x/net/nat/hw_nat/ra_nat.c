/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

  Module Name:
  ra_nat.c

  Abstract:

  Revision History:
  Who         When            What
  --------    ----------      ----------------------------------------------
  Name        Date            Modification logs
  Steven Liu  2011-04-11      Support RT63365
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

#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netfilter.h>
#include <linux/netdevice.h>
#include <linux/types.h>
#include <asm/uaccess.h>
#include <asm/string.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <net/tcp.h>
#include <linux/udp.h>
#include <linux/in.h>
#include <linux/if_vlan.h>
#include <linux/if_ether.h>
#include <linux/if_pppox.h>
#include <linux/ppp_defs.h>
#include <asm/checksum.h>
#include <linux/pci.h>
#include <linux/etherdevice.h>

#include "ra_nat.h"
#include "foe_fdb.h"
#include "frame_engine.h"
#include "hwnat_ioctl.h"
#include "acl_ioctl.h"
#include "ac_ioctl.h"
#include "acl_policy.h"
#include "mtr_policy.h"
#include "ac_policy.h"
#include "util.h"

#if defined  (CONFIG_RA_HW_NAT_WIFI)
static int wifi_offload __read_mostly = 0;
module_param(wifi_offload, bool, S_IRUGO);
MODULE_PARM_DESC(wifi_offload, "Enable/Disable wifi PPE NAT Offload.");
#endif

MODULE_AUTHOR("Steven Liu/Kurtis Ke");
MODULE_LICENSE("Proprietary");
MODULE_DESCRIPTION("Ralink Hardware NAT v0.92\n");

#if !defined (CONFIG_RA_HW_NAT_MANUAL_BIND)
#define LAN_PORT_VLAN_ID	CONFIG_RA_HW_NAT_LAN_VLANID
#define WAN_PORT_VLAN_ID	CONFIG_RA_HW_NAT_WAN_VLANID
#endif

#if defined (CONFIG_RALINK_RT3052)
extern int rw_rf_reg(int write, int reg, int *data);
#endif

extern int (*ra_sw_nat_hook_rx) (struct sk_buff * skb);
extern int (*ra_sw_nat_hook_tx) (struct sk_buff * skb, int gmac_no);
extern unsigned char bind_dir;

struct FoeEntry    *PpeFoeBase;
dma_addr_t	    PpePhyFoeBase;
struct net_device  *DstPort[MAX_IF_NUM];
uint32_t           DscpReMarkerEbl=0;
#ifdef HWNAT_DEBUG
uint32_t	    DebugLevel=0;
#endif

uint16_t GLOBAL_PRE_ACL_STR  = DFL_PRE_ACL_STR;
uint16_t GLOBAL_PRE_ACL_END  = DFL_PRE_ACL_END;
uint16_t GLOBAL_PRE_MTR_STR  = DFL_PRE_MTR_STR;
uint16_t GLOBAL_PRE_MTR_END  = DFL_PRE_MTR_END;
uint16_t GLOBAL_PRE_AC_STR   = DFL_PRE_AC_STR;
uint16_t GLOBAL_PRE_AC_END   = DFL_PRE_AC_END;
uint16_t GLOBAL_POST_MTR_STR = DFL_POST_MTR_STR;
uint16_t GLOBAL_POST_MTR_END = DFL_POST_MTR_END;
uint16_t GLOBAL_POST_AC_STR  = DFL_POST_AC_STR;
uint16_t GLOBAL_POST_AC_END  = DFL_POST_AC_END;

#ifdef HWNAT_DEBUG
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
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,32)
                if(i==(unsigned int)sk->mac_header) printk("*");
#else
                if(i==(unsigned int)sk->mac.raw) printk("@m");
#endif
                printk("%02X-",*((unsigned char*)i));
        }
        printk("\n");
}
#endif

int RemoveVlanTag(struct sk_buff *skb)
{
    struct ethhdr *eth;
    struct vlan_ethhdr *veth;
    uint16_t VirIfIdx;

    /* get vlan header */
    veth = vlan_eth_hdr(skb);

    /* something wrong */
    if(veth->h_vlan_proto != htons(ETH_P_8021Q)) {
	printk("HNAT: Reentry packet is untagged frame?\n");
	return 65535;
    }

    /* get VirIfIdx */
    VirIfIdx = ntohs(veth->h_vlan_TCI);

    /* make skb writable */
    if (skb_cloned(skb) || skb_shared(skb)) {
	struct sk_buff *new_skb;
	new_skb = skb_copy(skb, GFP_ATOMIC);
	if (!new_skb) {
	    NAT_PRINT("HNAT: no mem for remove tag? (VirIfIdx=%d)\n", VirIfIdx);
	    return 65535;
	}
	kfree_skb(skb);
	skb = new_skb;
    }

    /* remove VLAN tag */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
    skb->data= skb->mac_header;
    skb->mac_header += VLAN_HLEN;
    memmove(skb->mac_header, skb->data, ETH_ALEN * 2);
#else
    skb->data= skb->mac.raw;
    skb->mac.raw += VLAN_HLEN;
    memmove(skb->mac.raw, skb->data, ETH_ALEN * 2);
#endif
    skb_pull(skb, VLAN_HLEN);
    skb->data += ETH_HLEN;  //pointer to layer3 header

    /* get ethernet header */
    eth = eth_hdr(skb);

    skb->protocol = eth->h_proto;

    return VirIfIdx;
}

static void FoeAllocTbl(uint32_t NumOfEntry)
{
    uint32_t FoeTblSize;

    FoeTblSize = NumOfEntry * sizeof(struct FoeEntry);
    PpeFoeBase = dma_alloc_coherent(NULL, FoeTblSize, &PpePhyFoeBase, GFP_KERNEL);
    if(PpeFoeBase == NULL) {
	    printk("FoeAllocTbl() failed to allocate foe mem.");
	    return;
    }
    RegWrite(PPE_FOE_BASE, PpePhyFoeBase);
    memset(PpeFoeBase, 0, FoeTblSize);
}

#if !defined (CONFIG_RA_HW_NAT_MANUAL_BIND) && defined (HWNAT_DEBUG)
static uint8_t *ShowCpuReason(struct sk_buff *skb)
{
    static uint8_t Buf[32];

    switch(FOE_AI(skb))
    {
    case TTL_0: /* 0x80 */
	return("TTL=0\n");
    case FOE_EBL_NOT_IPV4_HLEN5: /* 0x90 */
	return("FOE enable & not IPv4h5nf\n");
    case FOE_EBL_NOT_TCP_UDP_L4_READY: /* 0x91 */
	return("FOE enable & not TCP/UDP/L4_read\n");
    case TCP_SYN_FIN_RST: /* 0x92 */
	return("TCP SYN/FIN/RST\n");
    case UN_HIT: /* 0x93 */
	return("Un-hit\n");
    case HIT_UNBIND: /* 0x94 */
	return("Hit unbind\n");
    case HIT_UNBIND_RATE_REACH: /* 0x95 */
	return("Hit unbind & rate reach\n");
    case HIT_FIN:  /* 0x96 */
	return("Hit fin\n");
    case HIT_BIND_TTL_1: /* 0x97 */
	return("Hit bind & ttl=1 & ttl-1\n");
    case HIT_BIND_KEEPALIVE:  /* 0x98 */
	return("Hit bind & keep alive\n");
    case HIT_BIND_FORCE_TO_CPU: /* 0x99 */
	return("Hit bind & force to CPU\n");
    case ACL_FOE_TBL_ERR: /* 0x9A */
	return("acl link foe table error (!static & !unbind)\n");
    case ACL_TBL_TTL_1: /* 0x9B */
	return("acl link FOE table & TTL=1 & TTL-1\n");
    case ACL_ALERT_CPU: /* 0x9C */
	return("acl alert cpu\n");
    case NO_FORCE_DEST_PORT: /* 0xA0 */
	return("No force destination port\n");
    case ACL_FORCE_PRIORITY0: /* 0xA8 */
	return("ACL FORCE PRIORITY0\n");
    case ACL_FORCE_PRIORITY1: /* 0xA9 */
	return("ACL FORCE PRIORITY1\n");
    case ACL_FORCE_PRIORITY2: /* 0xAA */
	return("ACL FORCE PRIORITY2\n");
    case ACL_FORCE_PRIORITY3: /* 0xAB */
	return("ACL FORCE PRIORITY3\n");
    case ACL_FORCE_PRIORITY4: /* 0xAC */
	return("ACL FORCE PRIORITY4\n");
    case ACL_FORCE_PRIORITY5: /* 0xAD */
	return("ACL FORCE PRIORITY5\n");
    case ACL_FORCE_PRIORITY6: /* 0xAE */
	return("ACL FORCE PRIORITY6\n");
    case ACL_FORCE_PRIORITY7: /* 0xAF */
	return("ACL FORCE PRIORITY7\n");
    case EXCEED_MTU: /* 0xA1 */
	return("Exceed mtu\n");
    }

    sprintf(Buf,"CPU Reason Error - %X\n",FOE_AI(skb));
    return(Buf);
}


#ifdef HWNAT_DEBUG
uint32_t FoeDumpPkt(struct sk_buff *skb)
{
//dump related info from packet
#ifdef HWNAT_DEBUG_PKT_PARSE
    struct ethhdr *eth = NULL;
    struct vlan_hdr *vh1 = NULL;
    struct vlan_hdr *vh2 = NULL;
    struct iphdr *iph = NULL;
    struct tcphdr *th = NULL;
    struct udphdr *uh = NULL;

    uint32_t vlan1_gap = 0;
    uint32_t vlan2_gap = 0;
    uint32_t pppoe_gap=0;
    uint16_t pppoe_sid = 0;
    uint16_t ppp_tag = 0;
    uint16_t eth_type=0;

    NAT_PRINT("\nRx===<FOE_Entry=%d>=====\n",FOE_ENTRY_NUM(skb)); 
    NAT_PRINT("RcvIF=%s\n", skb->dev->name);
    NAT_PRINT("FOE_Entry=%d\n",FOE_ENTRY_NUM(skb));
    NAT_PRINT("FVLD=%d\n",FOE_FVLD(skb));
    NAT_PRINT("CPU Reason=%s",ShowCpuReason(skb));
    NAT_PRINT("ALG=%d\n",FOE_ALG(skb));
    NAT_PRINT("SP=%d\n",FOE_SP(skb));
    NAT_PRINT("AIS=%d\n",FOE_AIS(skb));


    eth_type=ntohs(skb->protocol);

    // Layer 2
    if(eth_type==ETH_P_8021Q) {
	vlan1_gap = VLAN_HLEN;
	vh1 = (struct vlan_hdr *)(skb->data);

	/* VLAN + PPPoE */
	if(ntohs(vh1->h_vlan_encapsulated_proto)==ETH_P_PPP_SES){
	    pppoe_gap = 8;
	    if (GetPppoeSid(skb, vlan1_gap, &pppoe_sid, &ppp_tag, 0)) {
		return 0;
	    }
	    /* Double VLAN = VLAN + VLAN */
	}else if(ntohs(vh1->h_vlan_encapsulated_proto)==ETH_P_8021Q) {
	    vlan2_gap = VLAN_HLEN;
	    vh2 = (struct vlan_hdr *)(skb->data + VLAN_HLEN);

	    /* VLAN + VLAN + PPPoE */
	    if(ntohs(vh2->h_vlan_encapsulated_proto)==ETH_P_PPP_SES){
		pppoe_gap = 8;
		if (GetPppoeSid(skb, (vlan1_gap + vlan2_gap), &pppoe_sid, &ppp_tag, 0)) {
		    return 0;
		}
		/* VLAN + VLAN + IP */
	    }else if(ntohs(vh2->h_vlan_encapsulated_proto)!=ETH_P_IP) {
		return 0;
	    }
	    /* VLAN + IP */
	}else if(ntohs(vh1->h_vlan_encapsulated_proto)!=ETH_P_IP) {
	    return 0;
	}
    }else if(eth_type != ETH_P_IP) {
	return 0;
    }

    eth = (struct ethhdr *)(skb->data-ETH_HLEN) ; /* DA + SA + ETH_TYPE */

    // Layer 3
    iph = (struct iphdr *) (skb->data + vlan1_gap + vlan2_gap + pppoe_gap);


    // Layer 4
    if(iph->protocol==IPPROTO_TCP) {
	th = (struct tcphdr *) ((uint8_t *) iph + iph->ihl * 4);
    }else if(iph->protocol==IPPROTO_UDP) {
	uh = (struct udphdr *) ((uint8_t *) iph + iph->ihl * 4);
    }else { //Not TCP or UDP
	return 0;
    }

    if(vlan1_gap) {
	NAT_PRINT("VLAN1: %d\n",ntohs(vh1->h_vlan_TCI));
    }
    if(vlan2_gap) {
	NAT_PRINT("VLAN2: %d\n",ntohs(vh2->h_vlan_TCI));
    }
    if(pppoe_gap) {
	NAT_PRINT("PPPoE Session ID: %d\n", ntohs(pppoe_sid));
    }

    NAT_PRINT("----------------------------------\n");
    NAT_PRINT("SrcMac=%0X:%0X:%0X:%0X:%0X:%0X\n",MAC_ARG(eth->h_source));
    NAT_PRINT("DstMac=%0X:%0X:%0X:%0X:%0X:%0X\n",MAC_ARG(eth->h_dest));
    NAT_PRINT("SrcIp:%s\n",Ip2Str(ntohl(iph->saddr)));
    NAT_PRINT("DstIp:%s\n",Ip2Str(ntohl(iph->daddr)));
    if(th!=NULL) {
	NAT_PRINT("SrcPort:%d Dstport:%d\n",ntohs(th->source),ntohs(th->dest));
    }else {
	NAT_PRINT("SrcPort:%d Dstport:%d\n",ntohs(uh->source),ntohs(uh->dest));
    }

#else //dump related info from FoE table
    struct FoeEntry *foe_entry = &PpeFoeBase[FOE_ENTRY_NUM(skb)];

    NAT_PRINT("\nRx===<FOE_Entry=%d>=====\n",FOE_ENTRY_NUM(skb)); 
    NAT_PRINT("RcvIF=%s\n", skb->dev->name);
    NAT_PRINT("FOE_Entry=%d\n",FOE_ENTRY_NUM(skb));
    NAT_PRINT("FVLD=%d\n",FOE_FVLD(skb));
    NAT_PRINT("CPU Reason=%s",ShowCpuReason(skb));
    NAT_PRINT("ALG=%d\n",FOE_ALG(skb));
    NAT_PRINT("SP=%d\n",FOE_SP(skb));
    NAT_PRINT("AIS=%d\n",FOE_AIS(skb));

    NAT_PRINT("Information Block 1=%x\n",foe_entry->info_blk1);

    if(foe_entry->bfib1.fmt == IPV4_NAPT) {
	NAT_PRINT("SIP=%s\n",Ip2Str(foe_entry->sip));
	NAT_PRINT("DIP=%s\n",Ip2Str(foe_entry->dip));
	NAT_PRINT("SPORT=%d\n",foe_entry->sport);
	NAT_PRINT("DPORT=%d\n",foe_entry->dport);
    }else if(foe_entry->bfib1.fmt == IPV4_NAT) {
	NAT_PRINT("SIP=%s\n",Ip2Str(foe_entry->sip));
	NAT_PRINT("DIP=%s\n",Ip2Str(foe_entry->dip));
    }else if(foe_entry->bfib1.fmt == IPV6_ROUTING) {
	NAT_PRINT("IPv6_DIP0=%08X\n", foe_entry->ipv6_dip0);
	NAT_PRINT("IPv6_DIP1=%08X\n", foe_entry->ipv6_dip1);
	NAT_PRINT("IPv6_DIP2=%08X\n", foe_entry->ipv6_dip2);
	NAT_PRINT("IPv6_DIP3=%08X\n", foe_entry->ipv6_dip3);
    } else {
	NAT_PRINT("Wrong MFT value\n");
    }
#endif
    NAT_PRINT("==================================\n");

    return 1;

}
#endif
#endif

#if defined  (CONFIG_RA_HW_NAT_WIFI)
int32_t PpeRxWifiTag(struct sk_buff * skb)
{
	    struct ethhdr *eth=NULL;
	    uint16_t VirIfIdx=0;

	    /* check dst interface exist */
	    if (skb->dev == NULL) {
		NAT_PRINT("HNAT: RX: interface not exist\n");
		kfree_skb(skb);
		return 0;
	    }

#if defined (CONFIG_RALINK_RT3052) || defined(HWNAT_SPKIP_MCAST_BCAST)
	    /* skip bcast/mcast traffic PPE. WiFi bug ? */
	    eth = eth_hdr(skb);
	    if(is_multicast_ether_addr(eth->h_dest))
		return 1;
#endif

	    if(skb->dev == DstPort[DP_RA0]) { VirIfIdx=DP_RA0;}
#if defined (CONFIG_RT2860V2_AP_MBSS)
	    else if(skb->dev == DstPort[DP_RA1]) { VirIfIdx=DP_RA1; }
	    else if(skb->dev == DstPort[DP_RA2]) { VirIfIdx=DP_RA2; }
	    else if(skb->dev == DstPort[DP_RA3]) { VirIfIdx=DP_RA3; }
	    else if(skb->dev == DstPort[DP_RA4]) { VirIfIdx=DP_RA4; }
	    else if(skb->dev == DstPort[DP_RA5]) { VirIfIdx=DP_RA5; }
	    else if(skb->dev == DstPort[DP_RA6]) { VirIfIdx=DP_RA6; }
	    else if(skb->dev == DstPort[DP_RA7]) { VirIfIdx=DP_RA7; }
	    else if(skb->dev == DstPort[DP_RA8]) { VirIfIdx=DP_RA8; }
	    else if(skb->dev == DstPort[DP_RA9]) { VirIfIdx=DP_RA9; }
	    else if(skb->dev == DstPort[DP_RA10]) { VirIfIdx=DP_RA10; }
	    else if(skb->dev == DstPort[DP_RA11]) { VirIfIdx=DP_RA11; }
	    else if(skb->dev == DstPort[DP_RA12]) { VirIfIdx=DP_RA12; }
	    else if(skb->dev == DstPort[DP_RA13]) { VirIfIdx=DP_RA13; }
	    else if(skb->dev == DstPort[DP_RA14]) { VirIfIdx=DP_RA14; }
	    else if(skb->dev == DstPort[DP_RA15]) { VirIfIdx=DP_RA15; }
#endif // CONFIG_RT2860V2_AP_MBSS //
#if defined (CONFIG_RT2860V2_AP_WDS)
	    else if(skb->dev == DstPort[DP_WDS0]) { VirIfIdx=DP_WDS0; }
	    else if(skb->dev == DstPort[DP_WDS1]) { VirIfIdx=DP_WDS1; }
	    else if(skb->dev == DstPort[DP_WDS2]) { VirIfIdx=DP_WDS2; }
	    else if(skb->dev == DstPort[DP_WDS3]) { VirIfIdx=DP_WDS3; }
#endif
#if defined (CONFIG_RT2860V2_AP_APCLI)
	    else if(skb->dev == DstPort[DP_APCLI0]) { VirIfIdx=DP_APCLI0; }
#endif // CONFIG_RT2860V2_AP_APCLI //
#if defined (CONFIG_RT2860V2_AP_MESH)
	    else if(skb->dev == DstPort[DP_MESH0]) { VirIfIdx=DP_MESH0; }
#endif // CONFIG_RT2860V2_AP_MESH //
#if defined (CONFIG_RTDEV_MII) || defined (CONFIG_RTDEV_USB) || defined (CONFIG_RTDEV_PCI)
	    else if(skb->dev == DstPort[DP_RAI0]) { VirIfIdx=DP_RAI0; }
#if defined (CONFIG_RT3090_AP_MBSS) || defined (CONFIG_RT5392_AP_MBSS) || \
    defined (CONFIG_RT3572_AP_MBSS) || defined (CONFIG_RT5572_AP_MBSS)
	    else if(skb->dev == DstPort[DP_RAI1]) { VirIfIdx=DP_RAI1; }
	    else if(skb->dev == DstPort[DP_RAI2]) { VirIfIdx=DP_RAI2; }
	    else if(skb->dev == DstPort[DP_RAI3]) { VirIfIdx=DP_RAI3; }
	    else if(skb->dev == DstPort[DP_RAI4]) { VirIfIdx=DP_RAI4; }
	    else if(skb->dev == DstPort[DP_RAI5]) { VirIfIdx=DP_RAI5; }
	    else if(skb->dev == DstPort[DP_RAI6]) { VirIfIdx=DP_RAI6; }
	    else if(skb->dev == DstPort[DP_RAI7]) { VirIfIdx=DP_RAI7; }
	    else if(skb->dev == DstPort[DP_RAI8]) { VirIfIdx=DP_RAI8; }
	    else if(skb->dev == DstPort[DP_RAI9]) { VirIfIdx=DP_RAI9; }
	    else if(skb->dev == DstPort[DP_RAI10]) { VirIfIdx=DP_RAI10; }
	    else if(skb->dev == DstPort[DP_RAI11]) { VirIfIdx=DP_RAI11; }
	    else if(skb->dev == DstPort[DP_RAI12]) { VirIfIdx=DP_RAI12; }
	    else if(skb->dev == DstPort[DP_RAI13]) { VirIfIdx=DP_RAI13; }
	    else if(skb->dev == DstPort[DP_RAI14]) { VirIfIdx=DP_RAI14; }
	    else if(skb->dev == DstPort[DP_RAI15]) { VirIfIdx=DP_RAI15; }
#endif // CONFIG_RTDEV_AP_MBSS //
#endif // CONFIG_RTDEV_MII || CONFIG_RTDEV_USB || CONFIG_RTDEV_PCI
#if defined (CONFIG_RT3090_AP_APCLI) || defined (CONFIG_RT5392_AP_APCLI) || \
    defined (CONFIG_RT3572_AP_APCLI) || defined (CONFIG_RT5572_AP_APCLI)
	    else if(skb->dev == DstPort[DP_APCLII0]) { VirIfIdx=DP_APCLII0; }
#endif // CONFIG_RTDEV_AP_APCLI //
#if defined (CONFIG_RT3090_AP_MESH) || defined (CONFIG_RT5392_AP_MESH) || \
    defined (CONFIG_RT3572_AP_MESH) || defined (CONFIG_RT5572_AP_MESH)
	    else if(skb->dev == DstPort[DP_MESHI0]) { VirIfIdx=DP_MESHI0; }
#endif // CONFIG_RTDEV_AP_MESH //
	    else if(skb->dev == DstPort[DP_PCI]) { VirIfIdx=DP_PCI; }
	    else {
		NAT_PRINT("HNAT: The interface %s is unknown\n", skb->dev->name);
		return 1;
	    }

	    /* make skb writable */
	    if (skb_cloned(skb) || skb_shared(skb)) {
		struct sk_buff *new_skb;
		new_skb = skb_copy(skb, GFP_ATOMIC);
		if (!new_skb) {
		    NAT_PRINT("HNAT: no mem for add tag? (VirIfIdx=%d)\n", VirIfIdx);
		    return 1;
		}
		kfree_skb(skb);
		skb = new_skb;
	    }

	    //push vlan tag to stand for actual incoming interface,
	    //so HNAT module can know the actual incoming interface from vlan id.
	    skb_push(skb, ETH_HLEN); //pointer to layer2 header before calling hard_start_xmit
	    skb = __vlan_put_tag(skb, VirIfIdx);
	    if (!skb) {
		NAT_PRINT("HNAT: not valid tag ? memleak ? (VirIfIdx=%d)\n", VirIfIdx);
		return 0;
	    }

	    //redirect to PPE
	    FOE_AI(skb) = UN_HIT;
	    FOE_MAGIC_TAG(skb) = FOE_MAGIC_PPE;
	    skb->dev = DstPort[DP_GMAC]; //we use GMAC1 to send to packet to PPE
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,32)
	    skb->dev->netdev_ops->ndo_start_xmit(skb,skb->dev);
#else
	    skb->dev->hard_start_xmit(skb, skb->dev);
#endif
    return 0;
}

int32_t PpeRxWifiDeTag(struct sk_buff * skb)
{
    /*
     * RT3883/RT3352/RT6855:
     * If FOE_AIS=1 and FOE_SP=0/6, it means this is reentry packet.
     * (WLAN->CPU->PPE->CPU or PCI->CPU->PPE->CPU)
     *
     *    Incoming  |   SP[2:0]   |  SP[2:0]
     *      Port    | EXT_SW_EN=1 | EXT_SW_EN=0
     *  ------------+-------------+------------
     *       P0	    |	  0	  |	1
     *       P1	    |	  1	  |	1
     *       P2	    |	  2	  |	1
     *       P3     |     3       |     1
     *	     P4     |     4       |     1
     *       P5     |     5*      |     1
     *      PDMA    |     6*      |     0
     *       GE1    |     N/A     |     1
     *       GE2    |     5*      |     2
     */
    struct ethhdr *eth=NULL;
    uint16_t VirIfIdx=0;
    uint32_t SrcPortNo=0;

#if defined(CONFIG_RALINK_RT3883) || defined(CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT6855)
    if(IS_EXT_SW_EN(RegRead(FE_COS_MAP))){
	SrcPortNo=6;
    }
#endif

    if((FOE_AIS(skb) == 1) && (FOE_SP(skb) == SrcPortNo)) {
	VirIfIdx = RemoveVlanTag(skb);

	/* recover to right incoming interface */
	if(VirIfIdx < MAX_IF_NUM) {
	    /* check dst interface exist */
	    if (DstPort[VirIfIdx] == NULL) {
		NAT_PRINT("HNAT: TX: interface (VirIfIdx=%d) not exist\n", VirIfIdx);
		kfree_skb(skb);
		return -1;
	    }

	    skb->dev=DstPort[VirIfIdx];

	    eth = eth_hdr(skb);
	    if (is_multicast_ether_addr(eth->h_dest)) {
		if (!compare_ether_addr(eth->h_dest, skb->dev->broadcast))
			skb->pkt_type = PACKET_BROADCAST;
		else
			skb->pkt_type = PACKET_MULTICAST;
	    } else {
		if (!compare_ether_addr(eth->h_dest, skb->dev->dev_addr))
			skb->pkt_type=PACKET_HOST;
		else
			skb->pkt_type=PACKET_OTHERHOST;
	    }

	    return 1;

	} else {
	    NAT_PRINT("HNAT: unknown interface (VirIfIdx=%d)\n", VirIfIdx);
	}
    }
    return 0;
}
#endif

int32_t PpeRxHandler(struct sk_buff * skb)
{
    struct ethhdr *eth=NULL;
    struct vlan_hdr *vh = NULL;
    struct iphdr *iph = NULL;
    struct tcphdr *th = NULL;
    struct udphdr *uh = NULL;
    struct FoeEntry *foe_entry=NULL;

    uint32_t vlan1_gap = 0;
    uint32_t vlan2_gap = 0;
    uint32_t pppoe_gap=0;
    uint16_t eth_type=0;
#if defined  (CONFIG_RA_HW_NAT_WIFI)
    int32_t ret=0;
#endif

    /* return trunclated packets to normal path */
    if (!skb || (skb->len < ETH_HLEN)) {
//	NAT_PRINT("HNAT: skb null or small len in rx path\n");
	return 1;
    }

    eth_type=ntohs(skb->protocol);

    /* PPE only can handle IPv4/VLAN/IPv6/PPP packets */
    if(eth_type != ETH_P_IP &&
#if defined(CONFIG_RA_HW_NAT_IPV6)
	eth_type != ETH_P_IPV6 &&
#endif
	eth_type != ETH_P_8021Q &&
	eth_type != ETH_P_PPP_SES &&
	eth_type != ETH_P_PPP_DISC) {
	return 1;
    }

    foe_entry=&PpeFoeBase[FOE_ENTRY_NUM(skb)];

#ifdef HWNAT_DEBUG
    if(DebugLevel==1) {
       FoeDumpPkt(skb);
    }
#endif

    if((FOE_MAGIC_TAG(skb) == FOE_MAGIC_PCI) || (FOE_MAGIC_TAG(skb) == FOE_MAGIC_WLAN)){
#if defined  (CONFIG_RA_HW_NAT_WIFI)
	/* check wifi offload enabled and prevent vlan double incap */
	if (wifi_offload && (eth_type != ETH_P_8021Q))
	    return PpeRxWifiTag(skb);
	else
	    return 1;
#else
	return 1;
#endif
    }

    /* It means the flow is already in binding state, just transfer to output interface 
     * rax<->raix binded traffic: HIT_BIND_FORCE_TO_CPU + FOE_AIS=1 + FOE_SP = 0 or 6
     */
    if((FOE_AI(skb)==HIT_BIND_FORCE_TO_CPU)) {
	    skb->dev = DstPort[foe_entry->act_dp];
	    skb_push(skb, ETH_HLEN); //pointer to layer2 header
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,32)
	    skb->dev->netdev_ops->ndo_start_xmit(skb,skb->dev);
#else
	    skb->dev->hard_start_xmit(skb, skb->dev);
#endif
	    return 0;
    }

#if defined  (CONFIG_RA_HW_NAT_WIFI)
    if(wifi_offload && (eth_type == ETH_P_8021Q) && (FOE_AI(skb)!=HIT_BIND_KEEPALIVE)) {
    /* PpeRxWifiDeTag return:
	-1 - iface exist error with kfree_skb
	 0 - no need detag
	 1 - detag ok and return to normal path
    */
	ret=PpeRxWifiDeTag(skb);
    if(ret == 1)
	return 1;	/* return to normal path */
    else if(ret == -1)	/* drop this packet */
	return 0;
    }
#endif

    if( (FOE_AI(skb)==HIT_BIND_KEEPALIVE) && (DFL_FOE_KA_ORG==0)){

#ifdef RELEASE_EXCLUDE
	    /* Notes:
	     *
	     *	 PPE_FOE_CFG->FOE_KA_ORG(bit12)
	     *
	     *	 Keep alive packet with original header
	     *	 1: Original header
	     *	 0: New header
	     *
	     *	 Either original or new header mode, all of the keepalive packets from 
	     *	 PPE to cpu will carry "keep alive with original header" in cpu reason field.
	     *
	     *	 If PPE in keepalive with new header mode:
	     *
	     *	 Step1: Recover to original packet and pass to cpu to refresh
	     *	 	uppler table (We have to recover SMAC/DMAC/SIP/DIP/SP/DP 
	     *	        and recalculate IP/TCP/UDP checksum)
	     *
	     *	 Step2: TxHandler have to drop this packet because PPE forwards
	     *	        packet to cpu and output port at the same time.
	     *
	     */
#endif
	  /* FIXME:
	   * Recover to original SMAC/DMAC, but we don't know that.
	   * just swap SMAC and DMAC to avoid "received packet with  own address as source address" error.
	   */
	    eth=(struct ethhdr *)(skb->data-ETH_HLEN);

	    FoeGetMacInfo(eth->h_dest, foe_entry->smac_hi);
	    FoeGetMacInfo(eth->h_source, foe_entry->dmac_hi);
	    eth->h_source[0]=0x1;//change to multicast packet, make bridge not learn this packet
	    if(eth_type==ETH_P_8021Q) {
		    vlan1_gap = VLAN_HLEN;
		    vh = (struct vlan_hdr *) skb->data;

#ifdef RELEASE_EXCLUDE
		    /*
		     * Recover to original vlan header
		     *
		     * LAN Ports VID=1, WAN Ports VID=2
		     * Packet from WAN to LAN: VLANID 2 --(FOE)--> VLANID 1
		     * Packet from LAN to WAN: VLANID 1 --(FOE)--> VLANID 2
		     *
		     * Keepalive in new header mode:
		     * FOE pass packet with new header to cpu after packet modified
		     *
		     * Packet from WAN to LAN: New VLANID = 1, change to VLANID=2 which mean
		     *              	       packet is coming from WAN port.
		     * Packet from LAN to WAN: New VLANID = 2, change VLANID=2 which mean
		     *              	       packet is coming from WAN port.
		     */
#endif
		    if(ntohs(vh->h_vlan_TCI)==LAN_PORT_VLAN_ID){
			    /* It make packet like coming from WAN port */
			    vh->h_vlan_TCI=htons(WAN_PORT_VLAN_ID);

		    } else {
			    /* It make packet like coming from LAN port */
			    vh->h_vlan_TCI=htons(LAN_PORT_VLAN_ID);
		    }

		    if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_PPP_SES) {
			    pppoe_gap = 8;
		    }else if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_8021Q) {
                            vlan2_gap = VLAN_HLEN;
                            vh = (struct vlan_hdr *)(skb->data + VLAN_HLEN);

                            /* VLAN + VLAN + PPPoE */
                            if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_PPP_SES){
                                pppoe_gap = 8;
                            }else {
                                /* VLAN + VLAN + IP */
                                eth_type = ntohs(vh->h_vlan_encapsulated_proto);
                            }
		    }else {
                            /* VLAN + IP */
                            eth_type = ntohs(vh->h_vlan_encapsulated_proto);
		    }
	    }

	    /* Only Ipv4 NAT need KeepAlive Packet to refresh iptable */
	    if(eth_type == ETH_P_IP) {
		iph = (struct iphdr *) (skb->data + vlan1_gap + vlan2_gap + pppoe_gap);

		//Recover to original layer 4 header
		if (iph->protocol == IPPROTO_TCP) {
		    th = (struct tcphdr *) ((uint8_t *) iph + iph->ihl * 4);
		    FoeToOrgTcpHdr(foe_entry, iph, th);

		} else if (iph->protocol == IPPROTO_UDP) {
		    uh = (struct udphdr *) ((uint8_t *) iph + iph->ihl * 4);
		    FoeToOrgUdpHdr(foe_entry, iph, uh);
		}

		//Recover to original layer 3 header
		FoeToOrgIpHdr(foe_entry,iph);
	    }else if(eth_type == ETH_P_IPV6) {
		/* Nothing to do */
	    }else {
		return 1;
	    }

	    /*
	     * Ethernet driver will call eth_type_trans() to set skb->pkt_type.
	     * If(destination mac != my mac)
	     *   skb->pkt_type=PACKET_OTHERHOST;
	     * In order to pass ip_rcv() check, we change pkt_type=PACKET_HOST here
	     */
	    skb->pkt_type=PACKET_HOST;
	    return 1;

    }

#if defined (CONFIG_RA_HW_NAT_ACL2UP_HELPER)
    if( (FOE_AI(skb)==HIT_UNBIND_RATE_REACH) )
    {
        AclClassifyKey NewRateReach;
	eth=(struct ethhdr *)(skb->data-ETH_HLEN);

	memset(&NewRateReach, 0, sizeof(AclClassifyKey));
	memcpy(NewRateReach.Mac, eth->h_source,ETH_ALEN);
        NewRateReach.Ethertype = eth_type; //Ethertype
	if(eth_type==ETH_P_8021Q)
	{
	    vlan1_gap = VLAN_HLEN;
	    vh = (struct vlan_hdr *) skb->data;
            NewRateReach.Vid = ntohs(vh->h_vlan_TCI); //VID
	    if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_PPP_SES) {
		pppoe_gap = 8;
	    }else if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_8021Q) {
		vlan2_gap = VLAN_HLEN;
		vh = (struct vlan_hdr *)(skb->data + VLAN_HLEN);

		/* VLAN + VLAN + PPPoE */
		if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_PPP_SES){
		    pppoe_gap = 8;
		}else {
		    /* VLAN + VLAN + IP */
		    eth_type = ntohs(vh->h_vlan_encapsulated_proto);
		}
	    }else {
		/* VLAN + IP */
		eth_type = ntohs(vh->h_vlan_encapsulated_proto);
	    }
	}

	/*IPv4*/
	if(eth_type == ETH_P_IP)
	{
	    iph = (struct iphdr *) (skb->data + vlan1_gap + vlan2_gap + pppoe_gap);

	    NewRateReach.Sip = ntohl(iph->saddr);
	    NewRateReach.Dip = ntohl(iph->daddr);
            NewRateReach.Tos = iph->tos; //TOS
	    if (iph->protocol == IPPROTO_TCP)
	    {
		th = (struct tcphdr *) ((uint8_t *) iph + iph->ihl * 4);
		NewRateReach.Sp = ntohs(th->source);
		NewRateReach.Dp = ntohs(th->dest);
		NewRateReach.Proto = ACL_PROTO_TCP;
	    }
	    else if (iph->protocol == IPPROTO_UDP) 
	    {
		uh = (struct udphdr *) ((uint8_t *) iph + iph->ihl * 4);
		NewRateReach.Sp = ntohs(uh->source);
		NewRateReach.Dp = ntohs(uh->dest);
		NewRateReach.Proto = ACL_PROTO_UDP;

	    }

	}

	/*classify user priority*/
	FOE_SP(skb)= AclClassify(&NewRateReach);

	return 1;

    }
#endif

    return 1;
}

/* is_in = 1 --> in  */
/* is_in = 0 --> out */
int32_t GetPppoeSid(struct sk_buff *skb, uint32_t vlan_gap, 
		uint16_t *sid, uint16_t *ppp_tag, uint32_t is_in)
{
	struct pppoe_hdr *peh = NULL;
	uint32_t offset = 0;

	if(!is_in) {
		offset = ETH_HLEN;
	}

	peh = (struct pppoe_hdr *) (skb->data + offset + vlan_gap);
#ifdef HWNAT_DEBUG
	if(DebugLevel==1) { 
		NAT_PRINT("\n==============\n");
		NAT_PRINT(" Ver=%d\n",peh->ver);
		NAT_PRINT(" Type=%d\n",peh->type);
		NAT_PRINT(" Code=%d\n",peh->code);
		NAT_PRINT(" sid=%x\n",ntohs(peh->sid));
		NAT_PRINT(" Len=%d\n",ntohs(peh->length));
		NAT_PRINT(" tag_type=%x\n",ntohs(peh->tag[0].tag_type));
		NAT_PRINT(" tag_len=%d\n",ntohs(peh->tag[0].tag_len));
		NAT_PRINT("=================\n");
	}
#endif
	*ppp_tag = ntohs(peh->tag[0].tag_type);
#if defined (CONFIG_RA_HW_NAT_IPV6)
	if (peh->ver != 1 || peh->type != 1 || (*ppp_tag != PPP_IP && *ppp_tag != PPP_IPV6) ) {
#else
	if (peh->ver != 1 || peh->type != 1 || *ppp_tag != PPP_IP ) {
#endif
		return 1;
	}

	*sid = peh->sid;
	return 0;
}

int32_t PpeTxHandler(struct sk_buff *skb, int gmac_no)
{
	struct vlan_hdr *vh = NULL;
	struct iphdr *iph = NULL;
	struct tcphdr *th = NULL;
#if defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT63365) || defined (CONFIG_RALINK_RT3352)
	struct udphdr *uh = NULL;
#elif defined (CONFIG_RALINK_RT3052)
	struct udphdr *uh = NULL;
	uint32_t phy_val;
#endif
	struct ethhdr *eth = NULL;
	uint32_t vlan1_gap = 0;
	uint32_t vlan2_gap = 0;
	uint32_t pppoe_gap = 0;
	uint16_t pppoe_sid = 0;
	uint16_t ppp_tag = 0;
	struct FoeEntry *foe_entry;
	uint32_t current_time;
	struct FoeEntry entry;
	uint16_t eth_type=0;
	uint32_t offset=0;
#if defined (CONFIG_RA_HW_NAT_SEMIAUTO_BIND)
	uint32_t now=0;
#endif

	if (!skb) {
//	    NAT_PRINT("HNAT: skb is null ?\n");
	    return 1;
	}

	/* return trunclated packets to normal path with padding */
	if (skb->len < ETH_HLEN) {
	    memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
//	    NAT_PRINT("HNAT: skb null or small len in tx path\n");
	    return 1;
	}

	/*
	 * Packet is interested by ALG?
	 * Yes: Don't enter binind state
	 * No: If flow rate exceed binding threshold, enter binding state.
	 */
	if(IS_MAGIC_TAG_VALID(skb) && (FOE_AI(skb)==HIT_UNBIND_RATE_REACH) && (FOE_ALG(skb)==0))
	{
		eth = (struct ethhdr *) skb->data;
		eth_type=ntohs(eth->h_proto);
		foe_entry=&PpeFoeBase[FOE_ENTRY_NUM(skb)];

#if defined (CONFIG_RA_HW_NAT_SEMIAUTO_BIND)
		// It's ready for becoming binding state in semi-auto
		// bind mode, so there is no need to update any
		// information within refresh interval.
#define SEMIAUTO_REFRESH_INTERVAL	30
		now = RegRead(FOE_TS)&0xFFFF;
		if(time_before((unsigned long)now, 
			    (unsigned long)foe_entry->tmp_buf.time_stamp 
			    + SEMIAUTO_REFRESH_INTERVAL)) {
		    memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
		    return 1;
		}
#endif
		//if this entry is already in binding state, skip it
		if(foe_entry->bfib1.state == BIND) {
			memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
			return 1;
		}

		/* Get original setting */
		memcpy(&entry, foe_entry, sizeof(entry));


		/* Set Layer2 Info - DMAC, SMAC */
		FoeSetMacInfo(entry.dmac_hi,eth->h_dest);
		FoeSetMacInfo(entry.smac_hi,eth->h_source);

		/* Set VLAN Info - VLAN1/VLAN2 */
		if(eth_type==ETH_P_8021Q) {
			vlan1_gap = VLAN_HLEN;
			vh = (struct vlan_hdr *)(skb->data + ETH_HLEN);
			entry.vlan1 = ntohs(vh->h_vlan_TCI);

			/* VLAN + PPPoE */
			if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_PPP_SES){
				pppoe_gap = 8;
				if (GetPppoeSid(skb, vlan1_gap, &pppoe_sid, &ppp_tag, 0)) {
					memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
					return 1;
				}
				entry.pppoe_id = ntohs(pppoe_sid);
				eth_type = ntohs(vh->h_vlan_encapsulated_proto);
			/* Double VLAN = VLAN + VLAN */
			}else if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_8021Q) {
			    vlan2_gap = VLAN_HLEN;
			    vh = (struct vlan_hdr *)(skb->data + ETH_HLEN + VLAN_HLEN);
			    entry.vlan2 = ntohs(vh->h_vlan_TCI);

			    /* VLAN + VLAN + PPPoE */
			    if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_PPP_SES){
				pppoe_gap = 8;
				if (GetPppoeSid(skb, (vlan1_gap + vlan2_gap), &pppoe_sid, &ppp_tag, 0)) {
					memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
					return 1;
				}
				entry.pppoe_id = ntohs(pppoe_sid);
				eth_type = ntohs(vh->h_vlan_encapsulated_proto);
			    }else {
				/* VLAN + VLAN + IP */
				eth_type = ntohs(vh->h_vlan_encapsulated_proto);
			    }
			}else {
			    /* VLAN + IP */
			    eth_type = ntohs(vh->h_vlan_encapsulated_proto);
			}
		}

		/*
		 * PPE support SMART VLAN/PPPoE Tag Push/PoP feature
		 *
		 *         | MODIFY | INSERT | DELETE
		 * --------+--------+--------+----------
		 * Tagged  | modify | modify | delete
		 * Untagged| no act | insert | no act
		 *
		 */

		if(vlan1_gap) {
		    entry.bfib1.v1=INSERT;
                } else {
		    entry.bfib1.v1 = DELETE ;
                }

                if(vlan2_gap) {
		    entry.bfib1.v2=INSERT;
                } else {
		    entry.bfib1.v2 = DELETE ;
                }

		if(pppoe_gap) { 
			entry.bfib1.pppoe = INSERT ;
		} else { 
			entry.bfib1.pppoe = DELETE ;
		}

		/* Set Layer3 Info */
		/* IPv4 or IPv4 over PPPoE */
		if( (eth_type == ETH_P_IP) || (eth_type == ETH_P_PPP_SES && ppp_tag == PPP_IP) ) {
		    iph = (struct iphdr *) (skb->data + ETH_HLEN + vlan1_gap + vlan2_gap + pppoe_gap);
		    entry.new_sip = ntohl(iph->saddr);
		    entry.new_dip = ntohl(iph->daddr);
		    entry.iblk2.rmdscp = DscpReMarkerEbl;
		    entry.iblk2.dscp = iph->tos;


		    /* Set Layer4 Info - NEW_SPORT, NEW_DPORT */
		    if (iph->protocol == IPPROTO_TCP) {
			th = (struct tcphdr *) ((uint8_t *) iph + iph->ihl * 4);
			entry.new_sport = ntohs(th->source);
			entry.new_dport = ntohs(th->dest);
			entry.bfib1.t_u = TCP;
		    } else if (iph->protocol == IPPROTO_UDP) {
#if defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT63365)
			uh = (struct udphdr *) ((uint8_t *) iph + iph->ihl * 4);
			entry.new_sport = ntohs(uh->source);
			entry.new_dport = ntohs(uh->dest);
			entry.bfib1.t_u = UDP;
#elif defined (CONFIG_RALINK_RT3352)
			if(RegRead(0xB000000C)> 0x0104) {
			    uh = (struct udphdr *) ((uint8_t *) iph + iph->ihl * 4);
			    entry.new_sport = ntohs(uh->source);
			    entry.new_dport = ntohs(uh->dest);
			    entry.bfib1.t_u = UDP;
			}else {
			    memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
			    return 1;
			}
#elif defined (CONFIG_RALINK_RT3052)
			rw_rf_reg(0, 0, &phy_val);
			phy_val = phy_val & 0xFF;

			if(phy_val > 0x53) {
			    uh = (struct udphdr *) ((uint8_t *) iph + iph->ihl * 4);
			    entry.new_sport = ntohs(uh->source);
			    entry.new_dport = ntohs(uh->dest);
			    entry.bfib1.t_u = UDP;
			} else {
			    memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
			    return 1;
			}
#else
			/* if udp check is zero, it cannot be accelerated by HNAT */
			/* we found the application is possible to use udp checksum=0 at first stage,
			 * then use non-zero checksum in the same session later, so we disable HNAT acceleration
			 * for all UDP traffic */
			memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
			return 1;
#endif
		    }else {
			/* we support IPv4 NAT mode */
#if defined (HWNAT_FIX_GRE)
			/* gre will fail in this case.*/
			if (eth_type == IPPROTO_GRE) {
			    memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
			    return 1;
			}
#else
			;
#endif
		    }
#if defined (CONFIG_RA_HW_NAT_IPV6)
		/* IPv6 or IPv6 over PPPoE */
		} else if (eth_type == ETH_P_IPV6 || (eth_type == ETH_P_PPP_SES && ppp_tag == PPP_IPV6) ) {
		    /* Nothing to do */
		    ;
#endif
		} else {
		    memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
		    return 1;
		}

		/* Set Current time to time_stamp field in information block 1 */
		current_time =RegRead(FOE_TS)&0xFFFF;
		entry.bfib1.time_stamp=(uint16_t)current_time;

#if defined (CONFIG_RA_HW_NAT_ACL2UP_HELPER)
		/*set user priority*/
		entry.iblk2.up = FOE_SP(skb);
		entry.iblk2.fp = 1;
#endif
		/* Set Information block 2 */
		entry.iblk2.fd=1;
		/* CPU need to handle traffic between WLAN/PCI and GMAC port */
		if( (strncmp(skb->dev->name,"ra",2)==0) ||
		    (strncmp(skb->dev->name,"wds",3)==0) ||
		    (strncmp(skb->dev->name,"mesh",4)==0) ||
		    (strncmp(skb->dev->name,"apcli",5)==0) ||
		    (skb->dev == DstPort[DP_PCI])) {
#if defined  (CONFIG_RA_HW_NAT_WIFI)
			if (!wifi_offload) {
			    memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
			    return 1;
			} else
			    entry.iblk2.dp=0; /* cpu */
#else
			memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
			return 1;
#endif // CONFIG_RA_HW_NAT_WIFI //

		}else {
/* RT3883 with 2xGMAC - Assuming GMAC2=WAN  and GMAC1=LAN */
#if defined (CONFIG_RAETH_GMAC2)
			if(gmac_no==1) {
			    if((bind_dir == DOWNSTREAM_ONLY) || (bind_dir == BIDIRECTION)) {
				entry.iblk2.dp=1;
			    }else {
				return 1;
			    }
			}else if(gmac_no==2) {
			    if((bind_dir == UPSTREAM_ONLY) || (bind_dir == BIDIRECTION)) {
				entry.iblk2.dp=2;
			    }else {
				return 1;
			    }
			}

/* RT2880, RT3883 */
#elif defined (CONFIG_RALINK_RT2880) || defined (CONFIG_RALINK_RT3883)
			if((entry.vlan1 & VLAN_VID_MASK)==LAN_PORT_VLAN_ID) {
			    if((bind_dir == DOWNSTREAM_ONLY) || (bind_dir == BIDIRECTION)) {
				entry.iblk2.dp=1;
			    }else {
				return 1;
			    }
			}else if((entry.vlan1 & VLAN_VID_MASK)==WAN_PORT_VLAN_ID) {
			    if((bind_dir == UPSTREAM_ONLY) || (bind_dir == BIDIRECTION)) {
				entry.iblk2.dp=1;
			    }else {
				return 1;
			    }
			}
/*  RT3052, RT335x */
#else

			if((entry.vlan1 & VLAN_VID_MASK)==LAN_PORT_VLAN_ID) {
			    if((bind_dir == DOWNSTREAM_ONLY) || (bind_dir == BIDIRECTION)) {
				entry.iblk2.dp=1; /* LAN traffic use VirtualPort1 in GMAC1*/
			    }else {
				return 1;
			    }
			}else if((entry.vlan1 & VLAN_VID_MASK)==WAN_PORT_VLAN_ID) {
			    if((bind_dir == UPSTREAM_ONLY) || (bind_dir == BIDIRECTION)) {
				entry.iblk2.dp=2; /* WAN traffic use VirtualPort2 in GMAC1*/
			    }else {
				return 1;
			    }
			}else {
			    /* for one arm NAT test -> no vlan tag */
			    entry.iblk2.dp=1;
			}
#endif
		}

#if defined (CONFIG_RA_HW_NAT_ACL2UP_HELPER)
		if(IS_FORCE_ACL_TO_UP(skb))
		{
			entry.iblk2.up=(GET_ACL_TO_UP(skb)); /* new user priority */
			entry.iblk2.fp=1; /* enable force user priority */
		}
#endif

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
#if defined (CONFIG_RTDEV_MII) || defined (CONFIG_RTDEV_USB) || defined (CONFIG_RTDEV_PCI)
		if(strncmp(skb->dev->name, "rai", 3)==0) {
#if defined (CONFIG_RT3090_AP_MESH) || defined (CONFIG_RT5392_AP_MESH) || \
    defined (CONFIG_RT3572_AP_MESH) || defined (CONFIG_RT5572_AP_MESH)
		    if(RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_MESH) {
			offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_MESH + DP_MESHI0);
		    }else
#endif // CONFIG_RTDEV_AP_MESH //

#if defined (CONFIG_RT3090_AP_APCLI) || defined (CONFIG_RT5392_AP_APCLI) || \
    defined (CONFIG_RT3572_AP_APCLI) || defined (CONFIG_RT5572_AP_APCLI)
		    if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_APCLI) {
			offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_APCLI + DP_APCLII0);
		    }else
#endif // CONFIG_RTDEV_AP_APCLI //
#if defined (CONFIG_RT3090_AP_WDS) || defined (CONFIG_RT5392_AP_WDS) || \
    defined (CONFIG_RT3572_AP_WDS) || defined (CONFIG_RT5572_AP_WDS)
	   	    if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_WDS) {
			offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_WDS + DP_WDSI0);
		    }else
#endif // CONFIG_RTDEV_AP_WDS //
		    {
			offset = RTMP_GET_PACKET_IF(skb) + DP_RAI0;
		    }
		}else
#endif // CONFIG_RTDEV_MII || CONFIG_RTDEV_USB || CONFIG_RTDEV_PCI

		if(strncmp(skb->dev->name, "ra", 2)==0) {
#if defined (CONFIG_RT2860V2_AP_MESH)
		    if(RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_MESH) {
			offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_MESH + DP_MESH0);
		    }else
#endif // CONFIG_RT2860V2_AP_MESH //
#if defined (CONFIG_RT2860V2_AP_APCLI)
		    if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_APCLI) {
			offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_APCLI + DP_APCLI0);
		    }else
#endif  // CONFIG_RT2860V2_AP_APCLI //
#if defined (CONFIG_RT2860V2_AP_WDS)
		    if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_WDS) {
			offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_WDS + DP_WDS0);
		    }else
#endif // CONFIG_RT2860V2_AP_WDS //
		    {
			offset = RTMP_GET_PACKET_IF(skb) + DP_RA0;
		    }
		}else if(strncmp(skb->dev->name, "eth2", 4)==0) {
			offset = DP_GMAC; //for debugging purpose
#ifdef CONFIG_RAETH_GMAC2
		}else if(strncmp(skb->dev->name, "eth3", 4)==0) {
			offset = DP_GMAC2; //for debugging purpose
#endif
		}else if(strncmp(skb->dev->name, "eth0", 4)==0) {
			offset = DP_PCI; //for debugging purpose
		}else {
		    printk("HNAT: unknown interface %s\n",skb->dev->name);
		}

		entry.act_dp = offset;

		/* Ipv4: TTL / Ipv6: Hot Limit filed */
		entry.bfib1.ttl = DFL_FOE_TTL_REGEN;

		/* Change Foe Entry State to Binding State*/
#if defined (CONFIG_RA_HW_NAT_AUTO_BIND)
		entry.bfib1.state = BIND;
#elif defined (CONFIG_RA_HW_NAT_SEMIAUTO_BIND)
		/* Don't bind this flow until user wants to bind it. */
		memcpy(&entry.tmp_buf, &entry.bfib1 , sizeof(entry.bfib1));
#endif
		memcpy(foe_entry, &entry, sizeof(entry));
#ifdef HWNAT_DEBUG
		if(DebugLevel==7)
			FoeDumpEntry(FOE_ENTRY_NUM(skb));
#endif
	}else if(IS_MAGIC_TAG_VALID(skb) && (FOE_AI(skb)==HIT_BIND_KEEPALIVE) && (DFL_FOE_KA_ORG==0)){
		/* this is duplicate packet in keepalive new header mode,
		 * just drop it */
		memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
		return 0;
	}
#ifdef HWNAT_DEBUG
	else if(IS_MAGIC_TAG_VALID(skb) && (FOE_AI(skb)==HIT_UNBIND_RATE_REACH)&& FOE_ALG(skb)==1) {
		if(DebugLevel==1) {
		    NAT_PRINT("%s: I cannot bind it becuase of FOE_ALG=1\n",__FUNCTION__);
		}

	}
#endif
	return 1;
}

void  PpeSetFoeEbl(uint32_t FoeEbl)
{
	uint32_t PpeFlowSet=0;

	PpeFlowSet = RegRead(PPE_FLOW_SET);

	/* FOE engine need to handle unicast/multicast/broadcast flow */
	if(FoeEbl==1) {
#if defined(HWNAT_SPKIP_MCAST_BCAST)
		PpeFlowSet = (BIT_FUC_FOE);
#else
		PpeFlowSet = (BIT_FUC_FOE | BIT_FMC_FOE | BIT_FBC_FOE);
#endif
		PpeFlowSet|= (BIT_IPV4_NAPT_EN | BIT_IPV4_NAT_EN);

#if defined(CONFIG_RA_HW_NAT_IPV6)
		PpeFlowSet |= (BIT_IPV6_FOE_EN);
#endif
	} else {
		PpeFlowSet &= ~(BIT_FUC_FOE | BIT_FMC_FOE | BIT_FBC_FOE);
		PpeFlowSet &= ~(BIT_IPV4_NAPT_EN | BIT_IPV4_NAT_EN);
#if defined(CONFIG_RA_HW_NAT_IPV6)
		PpeFlowSet &= ~(BIT_IPV6_FOE_EN);
#endif
	}

	RegWrite( PPE_FLOW_SET, PpeFlowSet);
}


static void PpeSetFoeHashMode(uint32_t HashMode)
{

	/* Allocate FOE table base */
	FoeAllocTbl(FOE_4TB_SIZ);

	switch(FOE_4TB_SIZ){
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
	RegModifyBits(PPE_FOE_CFG, HashMode , 3, 1);

	/* Set action for FOE search miss */
#if defined (CONFIG_RA_HW_NAT_AUTO_BIND) || defined (CONFIG_RA_HW_NAT_SEMIAUTO_BIND)
	RegModifyBits(PPE_FOE_CFG, FWD_CPU_BUILD_ENTRY, 4, 2);
#elif defined (CONFIG_RA_HW_NAT_MANUAL_BIND)
	RegModifyBits(PPE_FOE_CFG, ONLY_FWD_CPU, 4, 2);
#else
	#error "Please Choice Action for FoE search miss"
#endif
}

static void PpeSetAgeOut(void)
{
	/* set Unbind State Age Enable */
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_UNB_AGE, 8, 1);

	/* set min threshold of packet count for aging out at unbind state */
	RegModifyBits(PPE_FOE_UNB_AGE, DFL_FOE_UNB_MNP, 16, 16);

	/* set Delta time for aging out an unbind FOE entry */
	RegModifyBits(PPE_FOE_UNB_AGE, DFL_FOE_UNB_DLTA, 0, 8);

#if defined (CONFIG_RA_HW_NAT_AUTO_BIND) || defined (CONFIG_RA_HW_NAT_SEMIAUTO_BIND)
	/* set Bind TCP Age Enable */
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_TCP_AGE, 9, 1);

	/* set Bind UDP Age Enable */
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_UDP_AGE, 10, 1);

	/* set Bind TCP FIN Age Enable */
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_FIN_AGE, 11, 1);

	/* Delta time for aging out an ACL link to FOE entry */
	//RegModifyBits(PPE_FOE_UNB_AGE, DFL_FOE_ACL_DLTA, 8, 8);

	/* set Delta time for aging out an bind UDP FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE1, DFL_FOE_UDP_DLTA, 0, 16);

	/* set Delta time for aging out an bind TCP FIN FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE2, DFL_FOE_FIN_DLTA, 16, 16);

	/* set Delta time for aging out an bind TCP FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE2, DFL_FOE_TCP_DLTA, 0, 16);
#elif defined (CONFIG_RA_HW_NAT_MANUAL_BIND)
	/* fix TCP last ACK issue */
	/* Only need to enable Bind TCP FIN aging out function */
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_FIN_AGE, 11, 1);

	/* set Delta time for aging out an bind TCP FIN FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE2, DFL_FOE_FIN_DLTA, 16, 16);
#endif
}

static void PpeSetFoeKa(void)
{
	/* set Keep alive packet with new/org header */
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_KA_ORG, 12, 1);

	/* set Keep alive enable */
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_KA_EN, 13, 1);

	/* ACL link to FOE age enable */
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_ACL_AGE, 14, 1);

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
	if(Ebl==1) {
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
#if defined (CONFIG_RALINK_RT3052)
		/* Disable switch port 6 flow control to fix rt3052 tx/rx FC wired issue*/
		RegModifyBits(RALINK_ETH_SW_BASE+0xC8, 0x0, 8, 2);

		/* Set switch scheduler to SPQ */
		RegModifyBits(RALINK_ETH_SW_BASE+0x10, 0x0, 0, 16);
#elif defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT63365)

#if defined (CONFIG_RAETH_SPECIAL_TAG)
		/* Set GDMA1 GDM1_TCI_81xx */
		RegModifyBits(FE_GDMA1_FWD_CFG, 0x1, 24, 1);
		/* Set GDMA2 GDM2_TCI_81xx */
		RegModifyBits(FE_GDMA2_FWD_CFG, 0x1, 24, 1);
		/* Set EXT_SW_EN = 1 */
		RegModifyBits(FE_COS_MAP, 0x1, 30, 1);
#endif
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

#ifndef CONFIG_RALINK_RT3052_MP2
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
static void  PpeSetUserPriority(void)
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

#ifdef HWNAT_USER_AUTO
    /* Set boundary and range of auto user priority */
    RegModifyBits(AUTO_UP_CFG1, DFL_ATUP_BND1, 16, 14);
    RegModifyBits(AUTO_UP_CFG2, DFL_ATUP_BND2, 0, 14);
    RegModifyBits(AUTO_UP_CFG2, DFL_ATUP_BND3, 16, 14);
    RegModifyBits(AUTO_UP_CFG1, DFL_ATUP_R1_UP, 0, 3);
    RegModifyBits(AUTO_UP_CFG1, DFL_ATUP_R2_UP, 4, 3);
    RegModifyBits(AUTO_UP_CFG1, DFL_ATUP_R3_UP, 8, 3);
    RegModifyBits(AUTO_UP_CFG1, DFL_ATUP_R4_UP, 12, 3);
#endif

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

static void FoeFreeTbl(uint32_t NumOfEntry)
{
	uint32_t FoeTblSize;

	FoeTblSize = NumOfEntry * sizeof(struct FoeEntry);
	dma_free_coherent(NULL, FoeTblSize, PpeFoeBase, PpePhyFoeBase);
	RegWrite( PPE_FOE_BASE, 0);
}

static int32_t PpeEngStart(void)
{
	/* Set PPE Flow Set */
	PpeSetFoeEbl(1);

	/* Set PPE FOE Hash Mode */
	PpeSetFoeHashMode(DFL_FOE_HASH_MODE);

	/* Set default index in policy table */
#ifndef CONFIG_RA_HW_NAT_MINIMAL
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

#ifndef CONFIG_RALINK_RT3052_MP2
	/* Set User Priority related register */
	PpeSetUserPriority();
#endif
	return 0;
}

static int32_t PpeEngStop(void)
{
	/* Set PPE FOE ENABLE */
	PpeSetFoeGloCfgEbl(0);

	/* Set PPE Flow Set */
	PpeSetFoeEbl(0);

	/* Set default index in policy table */
#ifndef CONFIG_RA_HW_NAT_MINIMAL
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
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,32)
    return dev_get_by_name(&init_net, name);
#else
    return dev_get_by_name(name);
#endif
}

static void PpeSetDstPort(uint32_t Ebl)
{
    if(Ebl) {
	DstPort[DP_RA0]=ra_dev_get_by_name("ra0");
#if defined (CONFIG_RT2860V2_AP_MBSS)
	DstPort[DP_RA1]=ra_dev_get_by_name("ra1");
	DstPort[DP_RA2]=ra_dev_get_by_name("ra2");
	DstPort[DP_RA3]=ra_dev_get_by_name("ra3");
	DstPort[DP_RA4]=ra_dev_get_by_name("ra4");
	DstPort[DP_RA5]=ra_dev_get_by_name("ra5");
	DstPort[DP_RA6]=ra_dev_get_by_name("ra6");
	DstPort[DP_RA7]=ra_dev_get_by_name("ra7");
#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT63365)
	DstPort[DP_RA8]=ra_dev_get_by_name("ra8");
	DstPort[DP_RA9]=ra_dev_get_by_name("ra9");
	DstPort[DP_RA10]=ra_dev_get_by_name("ra10");
	DstPort[DP_RA11]=ra_dev_get_by_name("ra11");
	DstPort[DP_RA12]=ra_dev_get_by_name("ra12");
	DstPort[DP_RA13]=ra_dev_get_by_name("ra13");
	DstPort[DP_RA14]=ra_dev_get_by_name("ra14");
	DstPort[DP_RA15]=ra_dev_get_by_name("ra15");
#endif
#endif
#if defined (CONFIG_RT2860V2_AP_WDS)
	DstPort[DP_WDS0]=ra_dev_get_by_name("wds0");
	DstPort[DP_WDS1]=ra_dev_get_by_name("wds1");
	DstPort[DP_WDS2]=ra_dev_get_by_name("wds2");
	DstPort[DP_WDS3]=ra_dev_get_by_name("wds3");
#endif
#if defined (CONFIG_RT2860V2_AP_APCLI)
	DstPort[DP_APCLI0]=ra_dev_get_by_name("apcli0");
#endif
#if defined (CONFIG_RT2860V2_AP_MESH)
	DstPort[DP_MESH0]=ra_dev_get_by_name("mesh0");
#endif
#if defined (CONFIG_RTDEV_MII) || defined (CONFIG_RTDEV_USB) || defined (CONFIG_RTDEV_PCI)
	DstPort[DP_RAI0]=ra_dev_get_by_name("rai0");
#if defined (CONFIG_RT3090_AP_MBSS) || defined (CONFIG_RT5392_AP_MBSS) || \
    defined (CONFIG_RT3572_AP_MBSS) || defined (CONFIG_RT5572_AP_MBSS)
	DstPort[DP_RAI1]=ra_dev_get_by_name("rai1");
	DstPort[DP_RAI2]=ra_dev_get_by_name("rai2");
	DstPort[DP_RAI3]=ra_dev_get_by_name("rai3");
	DstPort[DP_RAI4]=ra_dev_get_by_name("rai4");
	DstPort[DP_RAI5]=ra_dev_get_by_name("rai5");
	DstPort[DP_RAI6]=ra_dev_get_by_name("rai6");
	DstPort[DP_RAI7]=ra_dev_get_by_name("rai7");
	DstPort[DP_RAI8]=ra_dev_get_by_name("rai8");
	DstPort[DP_RAI9]=ra_dev_get_by_name("rai9");
	DstPort[DP_RAI10]=ra_dev_get_by_name("rai10");
	DstPort[DP_RAI11]=ra_dev_get_by_name("rai11");
	DstPort[DP_RAI12]=ra_dev_get_by_name("rai12");
	DstPort[DP_RAI13]=ra_dev_get_by_name("rai13");
	DstPort[DP_RAI14]=ra_dev_get_by_name("rai14");
	DstPort[DP_RAI15]=ra_dev_get_by_name("rai15");
#endif // CONFIG_RTDEV_AP_MBSS //
#endif // CONFIG_RTDEV_MII || CONFIG_RTDEV_USB || CONFIG_RTDEV_PCI
#if defined (CONFIG_RT3090_AP_APCLI) || defined (CONFIG_RT5392_AP_APCLI) || \
    defined (CONFIG_RT3572_AP_APCLI) || defined (CONFIG_RT5572_AP_APCLI)
	DstPort[DP_APCLII0]=ra_dev_get_by_name("apclii0");
#endif // CONFIG_RTDEV_AP_APCLI //
#if defined (CONFIG_RT3090_AP_MESH) || defined (CONFIG_RT5392_AP_MESH) || \
    defined (CONFIG_RT3572_AP_MESH) || defined (CONFIG_RT5572_AP_MESH)
	DstPort[DP_MESHI0]=ra_dev_get_by_name("meshi0");
#endif // CONFIG_RTDEV_AP_MESH //
	DstPort[DP_GMAC]=ra_dev_get_by_name("eth2");
#ifdef CONFIG_RAETH_GMAC2
	DstPort[DP_GMAC2]=ra_dev_get_by_name("eth3");
#endif
	DstPort[DP_PCI]=ra_dev_get_by_name("eth0"); // PCI interface name
    }else {
	if(DstPort[DP_RA0]!=NULL) { dev_put(DstPort[DP_RA0]); }
#if defined (CONFIG_RT2860V2_AP_MBSS)
	if(DstPort[DP_RA1]!=NULL) { dev_put(DstPort[DP_RA1]); }
	if(DstPort[DP_RA2]!=NULL) { dev_put(DstPort[DP_RA2]); }
	if(DstPort[DP_RA3]!=NULL) { dev_put(DstPort[DP_RA3]); }
	if(DstPort[DP_RA4]!=NULL) { dev_put(DstPort[DP_RA4]); }
	if(DstPort[DP_RA5]!=NULL) { dev_put(DstPort[DP_RA5]); }
	if(DstPort[DP_RA6]!=NULL) { dev_put(DstPort[DP_RA6]); }
	if(DstPort[DP_RA7]!=NULL) { dev_put(DstPort[DP_RA7]); }
#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT63365)
	if(DstPort[DP_RA8]!=NULL) { dev_put(DstPort[DP_RA8]); }
	if(DstPort[DP_RA9]!=NULL) { dev_put(DstPort[DP_RA9]); }
	if(DstPort[DP_RA10]!=NULL) { dev_put(DstPort[DP_RA10]); }
	if(DstPort[DP_RA11]!=NULL) { dev_put(DstPort[DP_RA11]); }
	if(DstPort[DP_RA12]!=NULL) { dev_put(DstPort[DP_RA12]); }
	if(DstPort[DP_RA13]!=NULL) { dev_put(DstPort[DP_RA13]); }
	if(DstPort[DP_RA14]!=NULL) { dev_put(DstPort[DP_RA14]); }
	if(DstPort[DP_RA15]!=NULL) { dev_put(DstPort[DP_RA15]); }
#endif
#endif
#if defined (CONFIG_RT2860V2_AP_WDS)
	if(DstPort[DP_WDS0]!=NULL) { dev_put(DstPort[DP_WDS0]); }
	if(DstPort[DP_WDS1]!=NULL) { dev_put(DstPort[DP_WDS1]); }
	if(DstPort[DP_WDS2]!=NULL) { dev_put(DstPort[DP_WDS2]); }
	if(DstPort[DP_WDS3]!=NULL) { dev_put(DstPort[DP_WDS3]); }
#endif
#if defined (CONFIG_RT2860V2_AP_APCLI)
	if(DstPort[DP_APCLI0]!=NULL) { dev_put(DstPort[DP_APCLI0]); }
#endif
#if defined (CONFIG_RT2860V2_AP_MESH)
	if(DstPort[DP_MESH0]!=NULL) { dev_put(DstPort[DP_MESH0]); }
#endif
#if defined (CONFIG_RTDEV_MII) || defined (CONFIG_RTDEV_USB) || defined (CONFIG_RTDEV_PCI)
	if(DstPort[DP_RAI0]!=NULL) { dev_put(DstPort[DP_RAI0]); }
#if defined (CONFIG_RT3090_AP_MBSS) || defined (CONFIG_RT5392_AP_MBSS) || \
    defined (CONFIG_RT3572_AP_MBSS) || defined (CONFIG_RT5572_AP_MBSS)
	if(DstPort[DP_RAI1]!=NULL) { dev_put(DstPort[DP_RAI1]); }
	if(DstPort[DP_RAI2]!=NULL) { dev_put(DstPort[DP_RAI2]); }
	if(DstPort[DP_RAI3]!=NULL) { dev_put(DstPort[DP_RAI3]); }
	if(DstPort[DP_RAI4]!=NULL) { dev_put(DstPort[DP_RAI4]); }
	if(DstPort[DP_RAI5]!=NULL) { dev_put(DstPort[DP_RAI5]); }
	if(DstPort[DP_RAI6]!=NULL) { dev_put(DstPort[DP_RAI6]); }
	if(DstPort[DP_RAI7]!=NULL) { dev_put(DstPort[DP_RAI7]); }
	if(DstPort[DP_RAI8]!=NULL) { dev_put(DstPort[DP_RAI8]); }
	if(DstPort[DP_RAI9]!=NULL) { dev_put(DstPort[DP_RAI9]); }
	if(DstPort[DP_RAI10]!=NULL) { dev_put(DstPort[DP_RAI10]); }
	if(DstPort[DP_RAI11]!=NULL) { dev_put(DstPort[DP_RAI11]); }
	if(DstPort[DP_RAI12]!=NULL) { dev_put(DstPort[DP_RAI12]); }
	if(DstPort[DP_RAI13]!=NULL) { dev_put(DstPort[DP_RAI13]); }
	if(DstPort[DP_RAI14]!=NULL) { dev_put(DstPort[DP_RAI14]); }
	if(DstPort[DP_RAI15]!=NULL) { dev_put(DstPort[DP_RAI15]); }
#endif // CONFIG_RTDEV_AP_MBSS //
#endif // CONFIG_RTDEV_MII || CONFIG_RTDEV_USB || CONFIG_RTDEV_PCI
#if defined (CONFIG_RT3090_AP_APCLI) || defined (CONFIG_RT5392_AP_APCLI) || \
    defined (CONFIG_RT3572_AP_APCLI) || defined (CONFIG_RT5572_AP_APCLI)
	if(DstPort[DP_APCLII0]!=NULL) { dev_put(DstPort[DP_APCLII0]); }
#endif // CONFIG_RTDEV_AP_APCLI //
#if defined (CONFIG_RT3090_AP_MESH) || defined (CONFIG_RT5392_AP_MESH) || \
    defined (CONFIG_RT3572_AP_MESH) || defined (CONFIG_RT5572_AP_MESH)
	if(DstPort[DP_MESHI0]!=NULL) { dev_put(DstPort[DP_MESHI0]); }
#endif // CONFIG_RTDEV_AP_MESH //
	if(DstPort[DP_GMAC]!=NULL) { dev_put(DstPort[DP_GMAC]); }
#ifdef CONFIG_RAETH_GMAC2
	if(DstPort[DP_GMAC2]!=NULL) { dev_put(DstPort[DP_GMAC2]); }
#endif
	if(DstPort[DP_PCI]!=NULL) { dev_put(DstPort[DP_PCI]); }
    }

}

static uint32_t SetGdmaFwd(uint32_t Ebl)
{
	uint32_t data=0;

	data=RegRead(FE_GDMA1_FWD_CFG);

	if(Ebl) {
	    //Uni-cast frames forward to PPE
	    data |= GDM1_UFRC_P_PPE;
	    //Broad-cast MAC address frames forward to PPE
	    data |= GDM1_BFRC_P_PPE;
	    //Multi-cast MAC address frames forward to PPE
	    data |= GDM1_MFRC_P_PPE;
	    //Other MAC address frames forward to PPE
	    data |= GDM1_OFRC_P_PPE;

	}else {
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
	data=RegRead(FE_GDMA2_FWD_CFG);

	if(Ebl) {
	    //Uni-cast frames forward to PPE
	    data |= GDM1_UFRC_P_PPE;
	    //Broad-cast MAC address frames forward to PPE
	    data |= GDM1_BFRC_P_PPE;
	    //Multi-cast MAC address frames forward to PPE
	    data |= GDM1_MFRC_P_PPE;
	    //Other MAC address frames forward to PPE
	    data |= GDM1_OFRC_P_PPE;

	}else {
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
static int32_t PpeInitMod(void)
{

    //Get net_device structure of Dest Port
    PpeSetDstPort(1);

    /* Register ioctl handler */
    PpeRegIoctlHandler();
#ifndef CONFIG_RA_HW_NAT_MINIMAL
    AclRegIoctlHandler();
    AcRegIoctlHandler();
    MtrRegIoctlHandler();
#endif

    /* Initialize PPE related register */
    PpeEngStart();

#if ! defined (CONFIG_RA_HW_NAT_MANUAL_BIND)
    /* In manual mode, PPE always reports UN-HIT CPU reason, so we don't need to process it */
    /* Register RX/TX hook point */
    ra_sw_nat_hook_tx = PpeTxHandler;
    ra_sw_nat_hook_rx = PpeRxHandler;
#endif

    /* Set GMAC fowrards packet to PPE */
    SetGdmaFwd(1);

    NAT_PRINT("Ralink HW NAT Module Load\n");

    return 0;
}

static void PpeCleanupMod(void)
{
    /* Set GMAC fowrards packet to CPU */
    SetGdmaFwd(0);

#if ! defined (CONFIG_RA_HW_NAT_MANUAL_BIND)
    /* Unregister RX/TX hook point */
    ra_sw_nat_hook_rx = NULL;
    ra_sw_nat_hook_tx = NULL;
#endif

    /* Restore PPE related register */
    PpeEngStop();

    /* Unregister ioctl handler */
    PpeUnRegIoctlHandler();
#ifndef CONFIG_RA_HW_NAT_MINIMAL
    AclUnRegIoctlHandler();
    AcUnRegIoctlHandler();
    MtrUnRegIoctlHandler();
#endif

    //Release net_device structure of Dest Port
    PpeSetDstPort(0);

    NAT_PRINT("Ralink HW NAT Module Unload\n");
}

/*HNAT QOS*/
int PpeSetDscpRemarkEbl(uint32_t enable)
{
#if defined (CONFIG_RALINK_RT6855)
    /* Re-generate DSCP per flow */
    DscpReMarkerEbl=enable;
#else
    /* Re-generate DSCP */
    RegModifyBits(PPE_GLO_CFG, enable, 11, 1);
#endif
    return HWNAT_SUCCESS;
}

int PpeSetVpriRemarkEbl(uint32_t enable)
{
    /* Re-generate VLAN Priority */
    RegModifyBits(PPE_GLO_CFG, enable, 10, 1);
    return HWNAT_SUCCESS;
}

int PpeSetWeightFOE(uint32_t weight)
{
    /* Set weight of decision in resolution */
    RegModifyBits(UP_RES, weight, FUP_WT_OFFSET, 3);
    return HWNAT_SUCCESS;
}

int PpeSetWeightACL(uint32_t weight)
{
    /* Set weight of decision in resolution */
    RegModifyBits(UP_RES, weight, AUP_WT_OFFSET, 3);
    return HWNAT_SUCCESS;
}

int PpeSetWeightDSCP(uint32_t weight)
{
    RegModifyBits(UP_RES, weight, DUP_WT_OFFSET, 3);
    return HWNAT_SUCCESS;
}

int PpeSetWeightVPRI(uint32_t weight)
{
    /* Set weight of decision in resolution */
    RegModifyBits(UP_RES, weight, VUP_WT_OFFSET, 3);
    return HWNAT_SUCCESS;
}

int PpeSetDSCP_UP(uint32_t DSCP_SET, unsigned char UP)
{
    int DSCP_UP;

    DSCP_UP = ((UP<<0) | (UP<<4) | (UP<<8) | (UP<<12)\
	    | (UP<<16) | (UP<<20) | (UP<<24) | (UP<<28));
    /* Set DSCP to User priority mapping table */
    switch(DSCP_SET)
    {
    case 0:
	RegWrite(DSCP0_7_MAP_UP, DSCP_UP);
	break;
    case 1:
	RegWrite(DSCP8_15_MAP_UP, DSCP_UP);
	break;
    case 2:
	RegWrite(DSCP16_23_MAP_UP, DSCP_UP);
	break;
    case 3:
	RegWrite(DSCP24_31_MAP_UP, DSCP_UP);
	break;
    case 4:
	RegWrite(DSCP32_39_MAP_UP, DSCP_UP);
	break;
    case 5:
	RegWrite(DSCP40_47_MAP_UP, DSCP_UP);
	break;
    case 6:
	RegWrite(DSCP48_55_MAP_UP, DSCP_UP);
	break;
    case 7:
	RegWrite(DSCP56_63_MAP_UP, DSCP_UP);
	break;
    default:

	break;
    }
    return HWNAT_SUCCESS;
}

int PpeSetUP_IDSCP(uint32_t UP, uint32_t IDSCP)
{
    /* Set mapping table of user priority to in-profile DSCP */
    switch(UP)
    {
    case 0:
	RegModifyBits(UP0_3_MAP_IDSCP, IDSCP, 0, 6);
	break;
    case 1:
	RegModifyBits(UP0_3_MAP_IDSCP, IDSCP, 8, 6);
	break;
    case 2:
	RegModifyBits(UP0_3_MAP_IDSCP, IDSCP, 16, 6);
	break;
    case 3:
	RegModifyBits(UP0_3_MAP_IDSCP, IDSCP, 24, 6);
	break;
    case 4:
	RegModifyBits(UP4_7_MAP_IDSCP, IDSCP, 0, 6);
	break;
    case 5:
	RegModifyBits(UP4_7_MAP_IDSCP, IDSCP, 8, 6);
	break;
    case 6:
	RegModifyBits(UP4_7_MAP_IDSCP, IDSCP, 16, 6);
	break;
    case 7:
	RegModifyBits(UP4_7_MAP_IDSCP, IDSCP, 24, 6);
	break;
    default:
	break;
    }
    return HWNAT_SUCCESS;
}
int PpeSetUP_ODSCP(uint32_t UP, uint32_t ODSCP)
{
    /* Set mapping table of user priority to out-profile DSCP */
    switch(UP)
    {
    case 0:
	RegModifyBits(UP0_3_MAP_ODSCP, ODSCP, 0, 6);
	break;
    case 1:
	RegModifyBits(UP0_3_MAP_ODSCP, ODSCP, 8, 6);
	break;
    case 2:
	RegModifyBits(UP0_3_MAP_ODSCP, ODSCP, 16, 6);
	break;
    case 3:
	RegModifyBits(UP0_3_MAP_ODSCP, ODSCP, 24, 6);
	break;
    case 4:
	RegModifyBits(UP4_7_MAP_ODSCP, ODSCP, 0, 6);
	break;
    case 5:
	RegModifyBits(UP4_7_MAP_ODSCP, ODSCP, 8, 6);
	break;
    case 6:
	RegModifyBits(UP4_7_MAP_ODSCP, ODSCP, 16, 6);
	break;
    case 7:
	RegModifyBits(UP4_7_MAP_ODSCP, ODSCP, 24, 6);
	break;
    default:
	break;
    }
    return HWNAT_SUCCESS;
}

int PpeSetUP_VPRI(uint32_t UP, uint32_t VPRI)
{
    /* Set mapping table of user priority to vlan priority */
    switch(UP)
    {
    case 0:
	RegModifyBits(UP_MAP_VPRI, VPRI, 0, 3);
	break;
    case 1:
	RegModifyBits(UP_MAP_VPRI, VPRI, 4, 3);
	break;
    case 2:
	RegModifyBits(UP_MAP_VPRI, VPRI, 8, 3);
	break;
    case 3:
	RegModifyBits(UP_MAP_VPRI, VPRI, 12, 3);
	break;
    case 4:
	RegModifyBits(UP_MAP_VPRI, VPRI, 16, 3);
	break;
    case 5:
	RegModifyBits(UP_MAP_VPRI, VPRI, 20, 3);
	break;
    case 6:
	RegModifyBits(UP_MAP_VPRI, VPRI, 24, 3);
	break;
    case 7:
	RegModifyBits(UP_MAP_VPRI, VPRI, 28, 3);
	break;
    default:
	break;
    }
    return HWNAT_SUCCESS;
}

int PpeSetUP_AC(uint32_t UP, uint32_t AC)
{
    /* Set mapping table of user priority to access category */
    switch(UP)
    {
    case 0:
	RegModifyBits(UP_MAP_AC, AC, 0, 2);
	break;
    case 1:
	RegModifyBits(UP_MAP_AC, AC, 2, 2);
	break;
    case 2:
	RegModifyBits(UP_MAP_AC, AC, 4, 2);
	break;
    case 3:
	RegModifyBits(UP_MAP_AC, AC, 6, 2);
	break;
    case 4:
	RegModifyBits(UP_MAP_AC, AC, 8, 2);
	break;
    case 5:
	RegModifyBits(UP_MAP_AC, AC, 10, 2);
	break;
    case 6:
	RegModifyBits(UP_MAP_AC, AC, 12, 2);
	break;
    case 7:
	RegModifyBits(UP_MAP_AC, AC, 14, 2);
	break;
    default:
	break;
    }
    return HWNAT_SUCCESS;
}

int PpeSetSchMode(uint32_t policy)
{
    /* Set GDMA1&2 Schduling Mode */
    RegModifyBits(FE_GDMA1_SCH_CFG, policy, 24, 2);
    RegModifyBits(FE_GDMA2_SCH_CFG, policy, 24, 2);

    return HWNAT_SUCCESS;
}

/* In general case, we only need 1/2/4/8 weight */
int PpeWeightRemap(uint8_t W)
{
    switch(W)
    {
    case 8:
#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3883)
	return 3;
#else
	return 7;
#endif
    case 4:
#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3883)
	return 2;
#else
	return 3;
#endif
    case 2:
	return 1;
    case 1:
	return 0;
    default:
	/* invalid value */
	return -1;
    }
}

int PpeSetSchWeight(uint8_t W0, uint8_t W1, uint8_t W2, uint8_t W3)
{
    int32_t _W0, _W1, _W2, _W3;

    _W0=PpeWeightRemap(W0);
    _W1=PpeWeightRemap(W1);
    _W2=PpeWeightRemap(W2);
    _W3=PpeWeightRemap(W3);

    if((_W0==-1) || (_W1==-1) || (_W2==-1) || (_W3==-1)) {
	return HWNAT_FAIL;
    }

    /* Set GDMA1 Schduling Weight */
    RegModifyBits(FE_GDMA1_SCH_CFG, _W0, 0, 3);
    RegModifyBits(FE_GDMA1_SCH_CFG, _W1, 4, 3);
    RegModifyBits(FE_GDMA1_SCH_CFG, _W2, 8, 3);
    RegModifyBits(FE_GDMA1_SCH_CFG, _W3, 12, 3);

    /* Set GDMA2 Schduling Weight */
    RegModifyBits(FE_GDMA2_SCH_CFG, _W0, 0, 3);
    RegModifyBits(FE_GDMA2_SCH_CFG, _W1, 4, 3);
    RegModifyBits(FE_GDMA2_SCH_CFG, _W2, 8, 3);
    RegModifyBits(FE_GDMA2_SCH_CFG, _W3, 12, 3);

    return HWNAT_SUCCESS;
}


int PpeSetBindThreshold(uint32_t threshold)
{
   /* Set reach bind rate for unbind state */
    RegWrite(PPE_FOE_BNDR, threshold);

    return HWNAT_SUCCESS;
}

int PpeSetMaxEntryLimit(uint32_t full, uint32_t half, uint32_t qurt)
{
	/* Allowed max entries to be build during a time stamp unit */

	/* smaller than 1/4 of total entries */
	RegModifyBits(PPE_FOE_LMT1, qurt, 0, 14);

	/* between 1/2 and 1/4 of total entries */
	RegModifyBits(PPE_FOE_LMT1, half, 16, 14);

	/* between full and 1/2 of total entries */
	RegModifyBits(PPE_FOE_LMT2, full, 0, 14);

    return HWNAT_SUCCESS;
}

int PpeSetRuleSize(uint16_t pre_acl, uint16_t pre_meter, uint16_t pre_ac, uint16_t post_meter, uint16_t post_ac)
{

/* Pre Access Control List Rule Start Index */
	GLOBAL_PRE_ACL_STR = 0;

    /* Pre Access Control List Rule End Index */
	GLOBAL_PRE_ACL_END = GLOBAL_PRE_ACL_STR;

    /* Pre Meter Rule Start Index */
	GLOBAL_PRE_MTR_STR = GLOBAL_PRE_ACL_STR + pre_acl;
     /* Pre Meter Rule End Index */
	GLOBAL_PRE_MTR_END = GLOBAL_PRE_MTR_STR;

    /* Pre Accounting Rule Start Index */
	GLOBAL_PRE_AC_STR = GLOBAL_PRE_MTR_STR + pre_meter;

    /* Pre Accounting Rule End Index */
	GLOBAL_PRE_AC_END = GLOBAL_PRE_AC_STR;

    /* Post Meter Rule Start Index */
	GLOBAL_POST_MTR_STR = GLOBAL_PRE_AC_STR + pre_ac;

    /* Post Meter Rule End Index */
	GLOBAL_POST_MTR_END = GLOBAL_POST_MTR_STR;

    /* Post Accounting Rule Start Index */
	GLOBAL_POST_AC_STR = GLOBAL_POST_MTR_STR + post_meter;

    /* Post Accounting Rule End Index */
	GLOBAL_POST_AC_END = GLOBAL_POST_AC_STR;




    /* Set Pre ACL Table */
    RegModifyBits(PPE_PRE_ACL, GLOBAL_PRE_ACL_STR, 0, 9);
    RegModifyBits(PPE_PRE_ACL, GLOBAL_PRE_ACL_END, 16, 9);
    /* Set Pre AC Table */
    RegModifyBits(PPE_PRE_AC, GLOBAL_PRE_AC_STR, 0, 9);
    RegModifyBits(PPE_PRE_AC, GLOBAL_PRE_AC_END, 16, 9);

    /* Set Post AC Table */
    RegModifyBits(PPE_POST_AC, GLOBAL_POST_AC_STR, 0, 9);
    RegModifyBits(PPE_POST_AC, GLOBAL_POST_AC_END, 16, 9);
    /* Set Pre MTR Table */
    RegModifyBits(PPE_PRE_MTR, GLOBAL_PRE_MTR_STR, 0, 9);
    RegModifyBits(PPE_PRE_MTR, GLOBAL_PRE_MTR_END, 16, 9);

    /* Set Post MTR Table */
    RegModifyBits(PPE_POST_MTR, GLOBAL_POST_MTR_STR, 0, 9);
    RegModifyBits(PPE_POST_MTR, GLOBAL_POST_MTR_END, 16, 9);



    return HWNAT_SUCCESS;
}

int PpeSetKaInterval(uint8_t tcp_ka, uint8_t udp_ka)
{
	/* Keep alive time for bind FOE TCP entry */
	RegModifyBits(PPE_FOE_KA, tcp_ka, 16, 8);

	/* Keep alive timer for bind FOE UDP entry */
	RegModifyBits(PPE_FOE_KA, udp_ka, 24, 8);


    return HWNAT_SUCCESS;
}

int PpeSetUnbindLifeTime(uint8_t lifetime)
{
	/* set Delta time for aging out an unbind FOE entry */
	RegModifyBits(PPE_FOE_UNB_AGE, lifetime, 0, 8);

    return HWNAT_SUCCESS;
}

int PpeSetBindLifetime(uint16_t tcp_life, uint16_t udp_life, uint16_t fin_life)
{

	/* set Delta time for aging out an bind UDP FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE1, udp_life, 0, 16);

	/* set Delta time for aging out an bind TCP FIN FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE2, fin_life, 16, 16);

	/* set Delta time for aging out an bind TCP FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE2, tcp_life, 0, 16);




    return HWNAT_SUCCESS;
}


module_init(PpeInitMod);
module_exit(PpeCleanupMod);
