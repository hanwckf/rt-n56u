#include <asm/io.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/net.h>
#include <linux/in.h>
#include "ra_qos.h"
#include "raether.h"
#include "ra2882ethreg.h"

#include <asm/types.h>
#include <net/sock.h>
#include <linux/socket.h>
#include <linux/skbuff.h>
#include <linux/net.h>
#include <linux/if_vlan.h>
#include <linux/ip.h>


#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
#include "../../../net/nat/hw_nat/ra_nat.h"
#endif

#define CONTI_TX_SEND_MAX_SIZE	1440

/* 
 * set tx queue # to descriptor
 */
void rt3052_tx_queue_init(unsigned long data)
{
	/* define qos p */
	
}

void rt3052_pse_port0_fc_clear(unsigned long data)
{
	/* clear FE_INT_STATUS.PSE_P0_FC */
	
}

inline int get_tx_ctx_idx(unsigned int ring_no, unsigned long *idx)
{
	switch (ring_no) {
		case RING0:
			*idx = *(unsigned long*)TX_CTX_IDX0;
			break;
		case RING1:
			*idx = *(unsigned long*)TX_CTX_IDX1;
			break;
		case RING2:
			*idx = *(unsigned long*)TX_CTX_IDX2;
			break;
		case RING3:
			*idx = *(unsigned long*)TX_CTX_IDX3;
			break;
		default:
			printk("set_tx_ctx_idex error\n");
			return -1;
	};
	return 0;
}

inline int set_tx_ctx_idx(unsigned int ring_no, unsigned int idx)
{
	switch (ring_no ) {
		case RING0:
			*(unsigned long*)TX_CTX_IDX0 = cpu_to_le32((u32)idx);
			break;
		case RING1:
			*(unsigned long*)TX_CTX_IDX1 = cpu_to_le32((u32)idx);
			break;
		case RING2:
			*(unsigned long*)TX_CTX_IDX2 = cpu_to_le32((u32)idx);
			break;
		case RING3:
			*(unsigned long*)TX_CTX_IDX3 = cpu_to_le32((u32)idx);
			break;
		default:
			printk("set_tx_ctx_idex error\n");
			return -1;
	};

	return 1;
}

void get_tx_desc_and_dtx_idx(END_DEVICE* ei_local, int ring_no, unsigned long *tx_dtx_idx, struct PDMA_txdesc **tx_desc)
{
	switch (ring_no) {
		case RING0:
			*tx_desc = ei_local->tx_ring0;
			*tx_dtx_idx 	 = *(unsigned long*)TX_DTX_IDX0;
			break;
		case RING1:
			*tx_desc = ei_local->tx_ring1;
			*tx_dtx_idx 	 = *(unsigned long*)TX_DTX_IDX1;
			break;
		case RING2:
			*tx_desc = ei_local->tx_ring2;
			*tx_dtx_idx 	 = *(unsigned long*)TX_DTX_IDX2;
			break;
		case RING3:
			*tx_desc = ei_local->tx_ring3;
			*tx_dtx_idx 	 = *(unsigned long*)TX_DTX_IDX3;
			break;
		default:
			printk("ring_no input error... %d\n", ring_no);
	};
}

int fe_qos_packet_send(struct net_device *dev, struct sk_buff* skb, unsigned int ring_no, unsigned int qn, unsigned pn)
{
	END_DEVICE* ei_local = netdev_priv(dev);
	struct PDMA_txdesc* tx_desc;
	unsigned int tx_cpu_owner_idx, tx_dtx_idx;

	unsigned int	length=skb->len;
	int ret;
	unsigned long flags;

	//printk("fe_qos_packet_send: ring_no=%d qn=%d pn=%d\n", ring_no, qn, pn);

	switch ( ring_no ) {
		case 0:
			tx_desc = ei_local->tx_ring0;
			tx_cpu_owner_idx = *(unsigned long*)TX_CTX_IDX0;
			tx_dtx_idx 	 = *(unsigned long*)TX_DTX_IDX0;
			break;
		case 1:
			tx_desc = ei_local->tx_ring1;
			tx_cpu_owner_idx = *(unsigned long*)TX_CTX_IDX1;
			tx_dtx_idx 	 = *(unsigned long*)TX_DTX_IDX1;
			break;
		case 2:
			tx_desc = ei_local->tx_ring2;
			tx_cpu_owner_idx = *(unsigned long*)TX_CTX_IDX2;
			tx_dtx_idx 	 = *(unsigned long*)TX_DTX_IDX2;
			break;
		case 3:
			tx_desc = ei_local->tx_ring3;
			tx_cpu_owner_idx = *(unsigned long*)TX_CTX_IDX3;
			tx_dtx_idx 	 = *(unsigned long*)TX_DTX_IDX3;
			break;
		default:
			printk("ring_no input error... %d\n", ring_no);
			return -1;
	};

	//printk("tx_cpu_owner_idx=%d tx_dtx_idx=%d\n", tx_cpu_owner_idx, tx_dtx_idx);

	if(tx_desc == NULL) {
		printk("%s : txdesc is NULL\n", dev->name);
		return -1;
	}

	tx_desc[tx_cpu_owner_idx].txd_info1.SDP0 = virt_to_phys(skb->data);
	tx_desc[tx_cpu_owner_idx].txd_info2.SDL0 = length;
	tx_desc[tx_cpu_owner_idx].txd_info2.DDONE_bit = 0;
	tx_desc[tx_cpu_owner_idx].txd_info4.PN = pn;
	tx_desc[tx_cpu_owner_idx].txd_info4.QN = qn;

#ifdef CONFIG_RAETH_CHECKSUM_OFFLOAD
	ei_local->tx_ring0[tx_cpu_owner_idx].txd_info4.TCO = 1; 
	ei_local->tx_ring0[tx_cpu_owner_idx].txd_info4.UCO = 1; 
	ei_local->tx_ring0[tx_cpu_owner_idx].txd_info4.ICO = 1; 
#endif

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE) 
	if(FOE_MAGIC_TAG(skb) == FOE_MAGIC_PPE) {
	    tx_desc[tx_cpu_owner_idx].txd_info4.PN = 6; /* PPE */
	} else {
	    tx_desc[tx_cpu_owner_idx].txd_info4.PN = pn; 
	}
	
#endif

	spin_lock_irqsave(&ei_local->page_lock, flags);
	ei_local->skb_free[ring_no][tx_cpu_owner_idx] = skb;
	tx_cpu_owner_idx = (tx_cpu_owner_idx +1) % NUM_TX_DESC;
	ret = set_tx_ctx_idx(ring_no, tx_cpu_owner_idx);
	spin_unlock_irqrestore(&ei_local->page_lock, flags);

	ei_local->stat.tx_packets++;
	ei_local->stat.tx_bytes += length;

#ifdef CONFIG_RAETH_NAPI
	switch ( ring_no ) {
		case 0:
			if ( ei_local->tx0_full == 1) {
				ei_local->tx0_full = 0;
				netif_wake_queue(dev);
			}
			break;
		case 1:
			if ( ei_local->tx1_full == 1) {
				ei_local->tx1_full = 0;
				netif_wake_queue(dev);
			}
			break;
		case 2:
			if ( ei_local->tx2_full == 1) {
				ei_local->tx2_full = 0;
				netif_wake_queue(dev);
			}
			break;
		case 3:
			if ( ei_local->tx3_full == 1) {
				ei_local->tx3_full = 0;
				netif_wake_queue(dev);
			}
			break;
		default :
			printk("ring_no input error %d\n", ring_no);
	};
#endif
	return length;
}

int fe_tx_desc_init(struct net_device *dev, unsigned int ring_no, unsigned int qn, unsigned int pn)
{
	END_DEVICE* ei_local = netdev_priv(dev);
	struct PDMA_txdesc *tx_desc;
	unsigned int tx_cpu_owner_idx = 0;
	int i;
	unsigned int phy_tx_ring;

	// sanity check
	if ( ring_no > 3 ){
		printk("%s : ring_no - %d, please under 4...\n", dev->name, ring_no);
		return 0;
	}

	if ( pn > 2 ){
		printk("%s : pn - %d, please under 2...\n", dev->name, pn);
		return 0;
	}

	tx_desc = dma_alloc_coherent(NULL, NUM_TX_DESC*sizeof(struct PDMA_txdesc), &phy_tx_ring);
	ei_local->tx_cpu_owner_idx0 = tx_cpu_owner_idx;
	
	switch (ring_no) {
		case 0:
			ei_local->tx_ring0 = tx_desc;
			ei_local->phy_tx_ring0 = phy_tx_ring;
			break;
		case 1:
			ei_local->phy_tx_ring1 = phy_tx_ring;
			ei_local->tx_ring1 = tx_desc;
			break;
		case 2:
			ei_local->phy_tx_ring2 = phy_tx_ring;
			ei_local->tx_ring2 = tx_desc;
			break;
		case 3:
			ei_local->phy_tx_ring3 = phy_tx_ring;
			ei_local->tx_ring3 = tx_desc;
			break;
		default:
			printk("ring_no input error! %d\n", ring_no);
			dma_free_coherent(NULL, NUM_TX_DESC*sizeof(struct PDMA_txdesc), tx_desc, phy_tx_ring);
			return 0;
	};	

	if ( tx_desc == NULL)
	{
		printk("tx desc allocation failed!\n");
		return 0;
	}

	for( i = 0; i < NUM_TX_DESC; i++) {
		memset( &tx_desc[i], 0, sizeof(struct PDMA_txdesc));
		tx_desc[i].txd_info2.LS0_bit = 1;
		tx_desc[i].txd_info2.DDONE_bit = 1;
		tx_desc[i].txd_info4.PN = pn;
		tx_desc[i].txd_info4.QN = qn;
	}

	switch ( ring_no ) {
		case 0 :
			*(unsigned long*)TX_BASE_PTR0 = phys_to_bus((u32) phy_tx_ring);
			*(unsigned long*)TX_MAX_CNT0  = cpu_to_le32((u32)NUM_TX_DESC);
			*(unsigned long*)TX_CTX_IDX0  = cpu_to_le32((u32) tx_cpu_owner_idx);
			sysRegWrite(PDMA_RST_CFG, PST_DTX_IDX0);
			break;
		case 1 :
			*(unsigned long*)TX_BASE_PTR1 = phys_to_bus((u32) phy_tx_ring);
			*(unsigned long*)TX_MAX_CNT1  = cpu_to_le32((u32)NUM_TX_DESC);
			*(unsigned long*)TX_CTX_IDX1  = cpu_to_le32((u32) tx_cpu_owner_idx);
			sysRegWrite(PDMA_RST_CFG, PST_DTX_IDX1);
			break;
		case 2 :
			*(unsigned long*)TX_BASE_PTR2 = phys_to_bus((u32) phy_tx_ring);
			*(unsigned long*)TX_MAX_CNT2  = cpu_to_le32((u32)NUM_TX_DESC);
			*(unsigned long*)TX_CTX_IDX2  = cpu_to_le32((u32) tx_cpu_owner_idx);
			sysRegWrite(PDMA_RST_CFG, PST_DTX_IDX2);
			break;
		case 3 :
			*(unsigned long*)TX_BASE_PTR3 = phys_to_bus((u32) phy_tx_ring);
			*(unsigned long*)TX_MAX_CNT3  = cpu_to_le32((u32)NUM_TX_DESC);
			*(unsigned long*)TX_CTX_IDX3  = cpu_to_le32((u32) tx_cpu_owner_idx);
			sysRegWrite(PDMA_RST_CFG, PST_DTX_IDX3);
			break;
		default :
			printk("tx descriptor init failed %d\n", ring_no);
			return 0;
	};
	return 1;
}

/*
   DSCP | AC | WMM_AC (Access Category)
   ------+----+--------
   00-07|  1 |  BE
   24-31|  1 |  BE
   08-15|  0 |  BG
   16-23|  0 |  BG
   32-39|  2 |  VI
   40-47|  2 |  VI
   48-55|  3 |  VO
   56-63|  3 |  VO 

          |    TOS    |
     DSCP |(bit5~bit7)|  WMM  
   -------+-----------+-------
    0x00  |    000    |   BE
    0x18  |    011    |   BE
    0x08  |    001    |   BG
    0x10  |    010    |   BG
    0x20  |    100    |   VI
    0x28  |    101    |   VI
    0x30  |    110    |   VO
    0x38  |    111    |   VO

    Notes: BE should be mapped to AC1, but mapped to AC0 in linux kernel.

 */

int  pkt_classifier(struct sk_buff *skb,int gmac_no, int *ring_no, int *queue_no, int *port_no)
{
#if defined(CONFIG_RALINK_RT2880)
    /* RT2880 -- Assume using 1 Ring (Ring0), Queue 0, and Port 0 */
    *port_no 	= 0;
    *ring_no 	= 0;
    *queue_no 	= 0;
#else
    unsigned int ac=0;
    unsigned int bridge_traffic=0, lan_traffic=0;
    struct iphdr *iph=NULL;
    struct vlan_ethhdr *veth=NULL;
    unsigned int vlan_id=0;
#if defined (CONFIG_RAETH_QOS_DSCP_BASED)
    static char DscpToAcMap[8]={1,0,0,1,2,2,3,3};
#elif defined (CONFIG_RAETH_QOS_VPRI_BASED)
    static char VlanPriToAcMap[8]={1,0,0,1,2,2,3,3};
#endif

    /* Bridge:: {BG,BE,VI,VO} */
    /* GateWay:: WAN: {BG,BE,VI,VO}, LAN: {BG,BE,VI,VO} */
#if defined (CONFIG_RALINK_RT3883) && defined (CONFIG_RAETH_GMAC2)
    /* 
     * 1) Bridge: 
     *    1.1) GMAC1 ONLY:
     *                 VO/VI->Ring3, BG/BE->Ring2 
     *    1.2) GMAC1+GMAC2: 
     *                 GMAC1:: VO/VI->Ring3, BG/BE->Ring2 
     *                 GMAC2:: VO/VI->Ring1, BG/BE->Ring0 
     * 2) GateWay:
     *    2.1) GMAC1 ONLY:
     *	       GMAC1:: LAN:VI/VO->Ring2, BE/BK->Ring2
     *	               WAN:VI/VO->Ring3, BE/BK->Ring3
     *    2.2)GMAC1+GMAC2: 
     *	       GMAC1:: LAN:VI/VO/BE/BK->Ring2, WAN:VI/VO/BE/BK->Ring3
     *	       GMAC2:: VI/VO->Ring1, BE/BK->Ring0
     */
    static unsigned char AcToRing_BridgeMap[4] = {2, 2, 3, 3}; 
    static unsigned char AcToRing_GE1Map[2][4] = {{3, 3, 3, 3},{2, 2, 2, 2}}; 
    static unsigned char AcToRing_GE2Map[4] = {0, 0, 1, 1};
#elif defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT2883) || \
      defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || \
      defined (CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_RT63365) || \
     (defined (CONFIG_RALINK_RT3883) && !defined(CONFIG_RAETH_GMAC2))
    /* 
     * 1) Bridge: VO->Ring3, VI->Ring2, BG->Ring1, BE->Ring0 
     * 2) GateWay:
     *    2.1) GMAC1:: LAN:VI/VO->Ring1, BE/BK->Ring0
     *	               WAN:VI/VO->Ring3, BE/BK->Ring2
     */ 
    static unsigned char AcToRing_BridgeMap[4] = {0, 1, 2, 3}; 
    static unsigned char AcToRing_GE1Map[2][4] = {{2, 2, 3, 3},{0, 0, 1, 1}}; 
#endif  // CONFIG_RALINK_RT2883

    /* 
     * Set queue no - QN field in TX Descriptor
     * always use queue 3 for the packet from CPU to GMAC 
     */
    *queue_no = 3; 

    /* Get access category */
    veth = (struct vlan_ethhdr *)(skb->data);
    if(veth->h_vlan_proto == htons(ETH_P_8021Q)) { // VLAN traffic
	iph= (struct iphdr *)(skb->data + VLAN_ETH_HLEN); 

	vlan_id = ntohs(veth->h_vlan_TCI & VLAN_VID_MASK);
	if(vlan_id==1) { //LAN
	    lan_traffic = 1;
	} else { //WAN
	    lan_traffic = 0;
	}

	if (veth->h_vlan_encapsulated_proto == htons(ETH_P_IP)) { //IPv4 
#if defined (CONFIG_RAETH_QOS_DSCP_BASED)
	    ac = DscpToAcMap[(iph->tos & 0xe0) >> 5];
#elif defined (CONFIG_RAETH_QOS_VPRI_BASED)
	    ac = VlanPriToAcMap[skb->priority];
#endif
	}else { //Ipv6, ARP ...etc
	    ac = 0;
	}
    }else { // non-VLAN traffic
	if (veth->h_vlan_proto == htons(ETH_P_IP)) { //IPv4
#if defined (CONFIG_RAETH_QOS_DSCP_BASED)
	    iph= (struct iphdr *)(skb->data + ETH_HLEN);
	    ac = DscpToAcMap[(iph->tos & 0xe0) >> 5];
#elif defined (CONFIG_RAETH_QOS_VPRI_BASED)
	    ac= VlanPriToAcMap[skb->priority];
#endif
	}else { // IPv6, ARP ...etc
	    ac = 0;
	}

	bridge_traffic=1;
    }
    

    /* Set Tx Ring no */
    if(gmac_no==1) { //GMAC1
	if(bridge_traffic) { //Bridge Mode
	    *ring_no = AcToRing_BridgeMap[ac];
	}else { //GateWay Mode
	    *ring_no = AcToRing_GE1Map[lan_traffic][ac];
	}
    }else { //GMAC2
#if defined (CONFIG_RALINK_RT3883) && defined (CONFIG_RAETH_GMAC2)
	*ring_no = AcToRing_GE2Map[ac];
#endif
    }


    /* Set Port No - PN field in Tx Descriptor*/
#if defined(CONFIG_RAETH_GMAC2)
    *port_no = gmac_no;
#else
    if(bridge_traffic) {
	*port_no = 1;
    }else {
	if(lan_traffic==1) { //LAN use VP1
	    *port_no = 1;
	}else { //WAN use VP2
	    *port_no = 2;
	}
    }
#endif // CONFIG_RAETH_GMAC2 //

#endif

    return 1;

}


/*
 *  Routine Description : 
 *  Hi/Li Rings and Queues definition for QoS Purpose
 *
 *  Related registers: (Detail information refer to pp106 of RT3052_DS_20080226.doc)
 *  Priority High/Low Definition - PDMA_FC_CFG, GDMA1_FC_CFG, GDMA2_FC_CFG
 *  Bit 28 -  Allows high priority Q to share low priority Q's reserved pages
 *  Bit 27:24 -  Px high priority definition bitmap 
 *  Weight Configuration - GDMA1_SCH_CFG, GDMA2_SCH_CFG, PDMA_SCH_CFG -> default 3210
 *
 * Parameter: 
 *	NONE
 * 	
*/
#define PSE_P1_LQ_FULL (1<<2)
#define PSE_P1_HQ_FULL (1<<3)
#define PSE_P2_LQ_FULL (1<<4)
#define PSE_P2_HQ_FULL (1<<5)

#define HIGH_QUEUE(queue)   (1<<(queue))
#define LOW_QUEUE(queue)    (0<<(queue))
#define PAGES_SHARING	    (1<<28)
#define RSEV_PAGE_COUNT_HQ  0x10 /* Reserved page count for high priority Q */
#define RSEV_PAGE_COUNT_LQ  0x10 /* Reserved page count for low priority Q */
#define VIQ_FC_ASRT	    0x10 /* Virtual input Q FC assertion threshold */

#define QUEUE_WEIGHT_1	    0
#define QUEUE_WEIGHT_2	    1
#define QUEUE_WEIGHT_4	    2
#define QUEUE_WEIGHT_8	    3
#define QUEUE_WEIGHT_16     4

#define WRR_SCH		    0 /*WRR */
#define STRICT_PRI_SCH	    1 /* Strict Priority */
#define MIX_SCH		    2 /* Mixed : Q3>WRR(Q2,Q1,Q0) */

/*
 *           Ring3  Ring2  Ring1  Ring0
 *            |  |   |  |  |  |   |  |
 *            |  |   |  |  |  |   |  |
 *        --------------------------------
 *        |         WRR Scheduler        |
 *        --------------------------------
 *                       |
 *    ---------------------------------------
 *    |                 PDMA                |
 *    ---------------------------------------
 *     |Q3||Q2||Q1||Q0|    |Q3||Q2||Q1||Q0|
 *     |  ||  ||  ||  |    |  ||  ||  ||  |
 *    ------------------- -------------------
 *    |      GDMA2      | |     GDMA1       |
 *    ------------------- -------------------
 *              |                      |
 *      ------------------------------------
 *      |              GMAC                |
 *      ------------------------------------
 *                       |
 *
 */
void set_scheduler_weight(void)
{
#if !defined (CONFIG_RALINK_RT5350)
    /* 
     * STEP1: Queue scheduling configuration 
     */
    *(unsigned long *)GDMA1_SCH_CFG = (WRR_SCH << 24) | 
	(QUEUE_WEIGHT_16 << 12) | /* queue 3 weight */
	(QUEUE_WEIGHT_8 << 8) |  /* queue 2 weight */
	(QUEUE_WEIGHT_4  << 4) |  /* queue 1 weight */
	(QUEUE_WEIGHT_2  << 0);   /* queue 0 weight */

    *(unsigned long *)GDMA2_SCH_CFG = (WRR_SCH << 24) | 
	(QUEUE_WEIGHT_16 << 12) | /* queue 3 weight */
	(QUEUE_WEIGHT_8 << 8) |  /* queue 2 weight */
	(QUEUE_WEIGHT_4  << 4) |  /* queue 1 weight */
	(QUEUE_WEIGHT_2  << 0);   /* queue 0 weight */
    
#endif
    /* 
     * STEP2: Ring scheduling configuration 
     */
#if defined (CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_RT63365)
    /* MIN_RATE_RATIO0=0, MAX_RATE_ULMT0=1, Weight0=1 */
    *(unsigned long *)SCH_Q01_CFG =  (0 << 10) | (1<<14) | (0 << 12);
    /* MIN_RATE_RATIO1=0, MAX_RATE_ULMT1=1, Weight1=4 */
    *(unsigned long *)SCH_Q01_CFG |= (0 << 26) | (1<<30) | (2 << 28);

    /* MIN_RATE_RATIO2=0, MAX_RATE_ULMT2=1, Weight0=1 */
    *(unsigned long *)SCH_Q23_CFG =  (0 << 10) | (1<<14) | (0 << 12);
    /* MIN_RATE_RATIO3=0, MAX_RATE_ULMT3=1, Weight1=4 */
    *(unsigned long *)SCH_Q23_CFG |= (0 << 26) | (1<<30) | (2 << 28);
#else
    *(unsigned long *)PDMA_SCH_CFG = (WRR_SCH << 24) | 
	(QUEUE_WEIGHT_16 << 12) | /* ring 3 weight */
	(QUEUE_WEIGHT_4 << 8) |  /* ring 2 weight */
	(QUEUE_WEIGHT_16 << 4) |  /* ring 1 weight */
	(QUEUE_WEIGHT_4 << 0);   /* ring 0 weight */
#endif
}

/*
 * Routine Description : 
 * 	Bucket size and related information from ASIC Designer, 
 * 	please check Max Lee to update these values
 *
 *	Related Registers
 *	  FE_GLO_CFG - initialize clock rate for rate limiting
 *	  PDMA_FC_CFG - Pause mechanism for Rings (Ref to pp116 in datasheet)
 *	  :
 * Parameter: 
 *	NONE
 */
/*
 *	Bit 29:24 - Q3 flow control pause condition
 *	Bit 21:16 - Q2 flow control pause condition
 *	Bit 13:8  - Q1 flow control pause condition
 *	Bit 5:0   - Q0 flow control pause condition
 *
 *	detail bitmap -
 *	  Bit[5] - Pause Qx when PSE p2 HQ full
 *	  Bit[4] - Pause Qx when PSE p2 LQ full
 *	  Bit[3] - Pause Qx when PSE p1 HQ full
 *	  Bit[2] - Pause Qx when PSE p1 LQ full
 *	  Bit[1] - Pause Qx when PSE p0 HQ full
 *	  Bit[0] - Pause Qx when PSE p0 LQ full
 */
void set_schedule_pause_condition(void)
{
#if !defined (CONFIG_RALINK_RT5350)
    /* 
     * STEP1: Set queue priority is high or low 
     *
     * Set queue 3 as high queue in GMAC1/GMAC2 
     */	
    *(unsigned long *)GDMA1_FC_CFG = ((HIGH_QUEUE(3)|LOW_QUEUE(2) | 
				      LOW_QUEUE(1)|LOW_QUEUE(0))<<24) |
				      (RSEV_PAGE_COUNT_HQ << 16) |
				      (RSEV_PAGE_COUNT_LQ <<8) |
				      VIQ_FC_ASRT | PAGES_SHARING;

    *(unsigned long *)GDMA2_FC_CFG = ((HIGH_QUEUE(3)|LOW_QUEUE(2) | 
				      LOW_QUEUE(1)|LOW_QUEUE(0))<<24) |
				      (RSEV_PAGE_COUNT_HQ << 16) |
				      (RSEV_PAGE_COUNT_LQ <<8) |
				      VIQ_FC_ASRT | PAGES_SHARING;
    
    /* 
     * STEP2: Set flow control pause condition 
     *
     * CPU always use queue 3, and queue3 is high queue.
     * If P2(GMAC2) high queue is full, pause ring3/ring2
     * If P1(GMAC1) high queue is full, pause ring1/ring0
     */
    *(unsigned long *)PDMA_FC_CFG =  ( PSE_P2_HQ_FULL << 24 ) | /* queue 3 */
	( PSE_P2_HQ_FULL << 16 ) | /* queue 2 */
	( PSE_P1_HQ_FULL << 8 ) |  /* queue 1 */
	( PSE_P1_HQ_FULL << 0 );  /* queue 0 */
#else
    *(unsigned long *)SDM_TRING = (0xC << 28) | (0x3 << 24) | (0xC << 4) | 0x3;
#endif
    
}


void set_output_shaper(void)
{
#define GDMA1_TOKEN_RATE	16  /* unit=64bits/ms */
#define GDMA2_TOKEN_RATE	16  /* unit=64bits/ms */

#if 0
    *(unsigned long *)GDMA1_SHPR_CFG =  (1 << 24) | /* output shaper enable */
		                        (128 << 16) | /* bucket size (unit=1KB) */
					(GDMA1_TOKEN_RATE << 0); /* token rate (unit=8B/ms) */
#endif

#if 0
    *(unsigned long *)GDMA2_SHPR_CFG =  (1 << 24) | /* output shaper enable */
		                        (128 << 16) | /* bucket size (unit=1KB) */
					(GDMA2_TOKEN_RATE << 0); /* token rate (unit=8B/ms) */
#endif
}
