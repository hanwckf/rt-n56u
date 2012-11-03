#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/signal.h>
#include <linux/irq.h>
#include <linux/ctype.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/io.h>
#include <asm/dma.h>

#include <asm/rt2880/surfboardint.h>	/* for cp0 reg access, added by bobtseng */

#include <linux/errno.h>
#include <linux/init.h>
#include <linux/mca.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#if defined(CONFIG_RAETH_SNMPD)
#include <linux/seq_file.h>
#endif


#if defined(CONFIG_RAETH_LRO)
#include <linux/inet_lro.h>
#endif

#include "ra2882ethreg.h"
#include "raether.h"
#include "ra_mac.h"
#include "ra_ethtool.h"

extern struct net_device *dev_raether;

#if defined(CONFIG_RAETH_TSO)
int txd_cnt[MAX_SKB_FRAGS/2 + 1];
int tso_cnt[16];
#endif

#if defined(CONFIG_RAETH_LRO)
#define MAX_AGGR 64
#define MAX_DESC  8
int lro_stats_cnt[MAX_AGGR + 1];
int lro_flush_cnt[MAX_AGGR + 1];
int lro_len_cnt1[16];
//int lro_len_cnt2[16];
int aggregated[MAX_DESC];
int lro_aggregated;
int lro_flushed;
int lro_nodesc;
int force_flush;
int tot_called1;
int tot_called2;
#endif

#if defined(CONFIG_RAETH_SNMPD)

static int ra_snmp_seq_show(struct seq_file *seq, void *v)
{
	char strprint[100];

#if !defined(CONFIG_RALINK_RT5350) && !defined(CONFIG_RALINK_MT7620)

	sprintf(strprint, "rx counters: %x %x %x %x %x %x %x\n", sysRegRead(GDMA_RX_GBCNT0), sysRegRead(GDMA_RX_GPCNT0),sysRegRead(GDMA_RX_OERCNT0), sysRegRead(GDMA_RX_FERCNT0), sysRegRead(GDMA_RX_SERCNT0), sysRegRead(GDMA_RX_LERCNT0), sysRegRead(GDMA_RX_CERCNT0));
	seq_puts(seq, strprint);

	sprintf(strprint, "fc config: %x %x %x %x\n", sysRegRead(CDMA_FC_CFG), sysRegRead(GDMA1_FC_CFG), PDMA_FC_CFG, sysRegRead(PDMA_FC_CFG));
	seq_puts(seq, strprint);

	sprintf(strprint, "scheduler: %x %x %x\n", sysRegRead(GDMA1_SCH_CFG), sysRegRead(GDMA2_SCH_CFG), sysRegRead(PDMA_SCH_CFG));
	seq_puts(seq, strprint);

#endif
	sprintf(strprint, "ports: %x %x %x %x %x %x\n", sysRegRead(PORT0_PKCOUNT), sysRegRead(PORT1_PKCOUNT), sysRegRead(PORT2_PKCOUNT), sysRegRead(PORT3_PKCOUNT), sysRegRead(PORT4_PKCOUNT), sysRegRead(PORT5_PKCOUNT));
	seq_puts(seq, strprint);

	return 0;
}

static int ra_snmp_seq_open(struct inode *inode, struct file *file)
{
	return single_open(file, ra_snmp_seq_show, NULL);
}

static const struct file_operations ra_snmp_seq_fops = {
	.owner	 = THIS_MODULE,
	.open	 = ra_snmp_seq_open,
	.read	 = seq_read,
	.llseek	 = seq_lseek,
	.release = single_release,
};
#endif


#if defined (CONFIG_GIGAPHY) || defined (CONFIG_100PHY) || defined (CONFIG_P5_MAC_TO_PHY_MODE)
#if defined (CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_RT6855A) || \
    defined (CONFIG_RALINK_MT7620) || defined(CONFIG_RALINK_MT7621) 
void enable_auto_negotiate(int unused)
{
	u32 regValue;
#if !defined (CONFIG_RALINK_MT7621)
	u32 addr = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR;
#endif

	/* FIXME: we don't know how to deal with PHY end addr */
	regValue = sysRegRead(ESW_PHY_POLLING);
	regValue |= (1<<31);
	regValue &= ~(0x1f);
	regValue &= ~(0x1f<<8);
#if defined (CONFIG_RALINK_MT7620)
	regValue |= (addr-1 << 0);//setup PHY address for auto polling (start Addr).
	regValue |= (addr << 8);// setup PHY address for auto polling (End Addr).
#elif defined (CONFIG_RALINK_MT7621)
	regValue |= (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR << 0);//setup PHY address for auto polling (start Addr).
	regValue |= (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2 << 8);// setup PHY address for auto polling (End Addr).
#else
	regValue |= (addr << 0);// setup PHY address for auto polling (start Addr).
	regValue |= (addr << 8);// setup PHY address for auto polling (End Addr).
#endif

	sysRegWrite(ESW_PHY_POLLING, regValue);

#if defined (CONFIG_P4_MAC_TO_PHY_MODE)
	*(unsigned long *)(RALINK_ETH_SW_BASE+0x3400) = 0x56330;
#endif
#if defined (CONFIG_P5_MAC_TO_PHY_MODE)
	*(unsigned long *)(RALINK_ETH_SW_BASE+0x3500) = 0x56330;
#endif
}
#elif defined (CONFIG_RALINK_RT2880) || defined(CONFIG_RALINK_RT3883) || \
      defined (CONFIG_RALINK_RT3052) || defined(CONFIG_RALINK_RT3352)

void enable_auto_negotiate(int ge)
{
#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352)
        u32 regValue = sysRegRead(0xb01100C8);
#else
	u32 regValue;
	regValue = (ge == 2)? sysRegRead(MDIO_CFG2) : sysRegRead(MDIO_CFG);
#endif

        regValue &= 0xe0ff7fff;                 // clear auto polling related field:
                                                // (MD_PHY1ADDR & GP1_FRC_EN).
        regValue |= 0x20000000;                 // force to enable MDC/MDIO auto polling.

#if defined (CONFIG_GE2_RGMII_AN) || defined (CONFIG_GE2_MII_AN)
	if(ge==2) {
	    regValue |= (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2 << 24);               // setup PHY address for auto polling.
	}
#endif
#if defined (CONFIG_GE1_RGMII_AN) || defined (CONFIG_GE1_MII_AN) || defined (CONFIG_P5_MAC_TO_PHY_MODE)
	if(ge==1) {
	    regValue |= (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR << 24);               // setup PHY address for auto polling.
	}
#endif

#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352)
	sysRegWrite(0xb01100C8, regValue);
#else
	if (ge == 2)
		sysRegWrite(MDIO_CFG2, regValue);
	else
		sysRegWrite(MDIO_CFG, regValue);
#endif
}
#endif
#endif
void ra2880stop(END_DEVICE *ei_local)
{
	unsigned int regValue;
	printk("ra2880stop()...");

	regValue = sysRegRead(PDMA_GLO_CFG);
	regValue &= ~(TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN);
	sysRegWrite(PDMA_GLO_CFG, regValue);
    	
	printk("Done\n");	
	// printk("Done0x%x...\n", readreg(PDMA_GLO_CFG));
}

void ei_irq_clear(void)
{
        sysRegWrite(FE_INT_STATUS, 0xFFFFFFFF);
}

void rt2880_gmac_hard_reset(void)
{
#if !defined (CONFIG_RALINK_RT6855A)
	//FIXME
	sysRegWrite(RSTCTRL, RALINK_FE_RST);
	sysRegWrite(RSTCTRL, 0);
#endif
}

void ra2880EnableInterrupt()
{
	unsigned int regValue = sysRegRead(FE_INT_ENABLE);
	RAETH_PRINT("FE_INT_ENABLE -- : 0x%08x\n", regValue);
//	regValue |= (RX_DONE_INT0 | TX_DONE_INT0);
		
	sysRegWrite(FE_INT_ENABLE, regValue);
}

void ra2880MacAddressSet(unsigned char p[6])
{
        unsigned long regValue;

	regValue = (p[0] << 8) | (p[1]);
#if defined (CONFIG_RALINK_RT5350)
        sysRegWrite(SDM_MAC_ADRH, regValue);
	printk("GMAC1_MAC_ADRH -- : 0x%08x\n", sysRegRead(SDM_MAC_ADRH));
#elif defined (CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_RT6855A)
        sysRegWrite(GDMA1_MAC_ADRH, regValue);
	printk("GMAC1_MAC_ADRH -- : 0x%08x\n", sysRegRead(GDMA1_MAC_ADRH));

	/* To keep the consistence between RT6855 and RT62806, GSW should keep the register. */
        sysRegWrite(SMACCR1, regValue);
	printk("SMACCR1 -- : 0x%08x\n", sysRegRead(SMACCR1));
#elif defined (CONFIG_RALINK_MT7620)
        sysRegWrite(SMACCR1, regValue);
	printk("SMACCR1 -- : 0x%08x\n", sysRegRead(SMACCR1));
#else
        sysRegWrite(GDMA1_MAC_ADRH, regValue);
	printk("GMAC1_MAC_ADRH -- : 0x%08x\n", sysRegRead(GDMA1_MAC_ADRH));
#endif

        regValue = (p[2] << 24) | (p[3] <<16) | (p[4] << 8) | p[5];
#if defined (CONFIG_RALINK_RT5350)
        sysRegWrite(SDM_MAC_ADRL, regValue);
	printk("GMAC1_MAC_ADRL -- : 0x%08x\n", sysRegRead(SDM_MAC_ADRL));	    
#elif defined (CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_RT6855A)
        sysRegWrite(GDMA1_MAC_ADRL, regValue);
	printk("GMAC1_MAC_ADRL -- : 0x%08x\n", sysRegRead(GDMA1_MAC_ADRL));	    

	/* To keep the consistence between RT6855 and RT62806, GSW should keep the register. */
        sysRegWrite(SMACCR0, regValue);
	printk("SMACCR0 -- : 0x%08x\n", sysRegRead(SMACCR0));
#elif defined (CONFIG_RALINK_MT7620)
        sysRegWrite(SMACCR0, regValue);
	printk("SMACCR0 -- : 0x%08x\n", sysRegRead(SMACCR0));
#else
        sysRegWrite(GDMA1_MAC_ADRL, regValue);
	printk("GMAC1_MAC_ADRL -- : 0x%08x\n", sysRegRead(GDMA1_MAC_ADRL));	    
#endif

        return;
}

#ifdef CONFIG_PSEUDO_SUPPORT
void ra2880Mac2AddressSet(unsigned char p[6])
{
        unsigned long regValue;

	regValue = (p[0] << 8) | (p[1]);
        sysRegWrite(GDMA2_MAC_ADRH, regValue);

        regValue = (p[2] << 24) | (p[3] <<16) | (p[4] << 8) | p[5];
        sysRegWrite(GDMA2_MAC_ADRL, regValue);

	printk("GDMA2_MAC_ADRH -- : 0x%08x\n", sysRegRead(GDMA2_MAC_ADRH));
	printk("GDMA2_MAC_ADRL -- : 0x%08x\n", sysRegRead(GDMA2_MAC_ADRL));	    
        return;
}
#endif

/**
 * hard_init - Called by raeth_probe to inititialize network device
 * @dev: device pointer
 *
 * ethdev_init initilize dev->priv and set to END_DEVICE structure
 *
 */
void ethtool_init(struct net_device *dev)
{
#if defined (CONFIG_ETHTOOL)
	END_DEVICE *ei_local = netdev_priv(dev);

	// init mii structure
	ei_local->mii_info.dev = dev;
	ei_local->mii_info.mdio_read = mdio_read;
	ei_local->mii_info.mdio_write = mdio_write;
	ei_local->mii_info.phy_id_mask = 0x1f;
	ei_local->mii_info.reg_num_mask = 0x1f;
	ei_local->mii_info.supports_gmii = mii_check_gmii_support(&ei_local->mii_info);
	// TODO:   phy_id: 0~4
	ei_local->mii_info.phy_id = 1;
#endif
	return;
}

#if defined(CONFIG_RAETH_QOS)
/*
 *	Routine Name : get_idx(mode, index)
 *	Description: calculate ring usage for tx/rx rings
 *	Mode 1 : Tx Ring 
 *	Mode 2 : Rx Ring
 */
int get_ring_usage(int mode, int i)
{
	unsigned long tx_ctx_idx, tx_dtx_idx, tx_usage;
	unsigned long rx_calc_idx, rx_drx_idx, rx_usage;

	struct PDMA_rxdesc* rxring;
	struct PDMA_txdesc* txring;

	END_DEVICE *ei_local = netdev_priv(dev_raether);


	if (mode == 2 ) {
		/* cpu point to the next descriptor of rx dma ring */
	        rx_calc_idx = *(unsigned long*)RX_CALC_IDX0;
	        rx_drx_idx = *(unsigned long*)RX_DRX_IDX0;
		rxring = (struct PDMA_rxdesc*)RX_BASE_PTR0;
		
		rx_usage = (rx_drx_idx - rx_calc_idx -1 + NUM_RX_DESC) % NUM_RX_DESC;
		if ( rx_calc_idx == rx_drx_idx ) {
		    if ( rxring[rx_drx_idx].rxd_info2.DDONE_bit == 1)
			tx_usage = NUM_RX_DESC;
		    else
			tx_usage = 0;
		}
		return rx_usage;
	}

	
	switch (i) {
		case 0:
				tx_ctx_idx = *(unsigned long*)TX_CTX_IDX0;
				tx_dtx_idx = *(unsigned long*)TX_DTX_IDX0;
				txring = ei_local->tx_ring0;
				break;
		case 1:
				tx_ctx_idx = *(unsigned long*)TX_CTX_IDX1;
				tx_dtx_idx = *(unsigned long*)TX_DTX_IDX1;
				txring = ei_local->tx_ring1;
				break;
		case 2:
				tx_ctx_idx = *(unsigned long*)TX_CTX_IDX2;
				tx_dtx_idx = *(unsigned long*)TX_DTX_IDX2;
				txring = ei_local->tx_ring2;
				break;
		case 3:
				tx_ctx_idx = *(unsigned long*)TX_CTX_IDX3;
				tx_dtx_idx = *(unsigned long*)TX_DTX_IDX3;
				txring = ei_local->tx_ring3;
				break;
		default:
			printk("get_tx_idx failed %d %d\n", mode, i);
			return 0;
	};

	tx_usage = (tx_ctx_idx - tx_dtx_idx + NUM_TX_DESC) % NUM_TX_DESC;
	if ( tx_ctx_idx == tx_dtx_idx ) {
		if ( txring[tx_ctx_idx].txd_info2.DDONE_bit == 1)
			tx_usage = 0;
		else
			tx_usage = NUM_TX_DESC;
	}
	return tx_usage;

}

void dump_qos()
{
	int usage;
	int i;

	printk("\n-----Raeth QOS -----\n\n");

	for ( i = 0; i < 4; i++)  {
		usage = get_ring_usage(1,i);
		printk("Tx Ring%d Usage : %d/%d\n", i, usage, NUM_TX_DESC);
	}

	usage = get_ring_usage(2,0);
	printk("RX Usage : %d/%d\n\n", usage, NUM_RX_DESC);
#if defined  (CONFIG_RALINK_MT7620)
	printk("PSE_FQFC_CFG(0x%08x)  : 0x%08x\n", PSE_FQFC_CFG, sysRegRead(PSE_FQFC_CFG));
	printk("PSE_IQ_CFG(0x%08x)  : 0x%08x\n", PSE_IQ_CFG, sysRegRead(PSE_IQ_CFG));
	printk("PSE_QUE_STA(0x%08x)  : 0x%08x\n", PSE_QUE_STA, sysRegRead(PSE_QUE_STA));
#elif defined (CONFIG_RALINK_RT5350)

#else
	printk("GDMA1_FC_CFG(0x%08x)  : 0x%08x\n", GDMA1_FC_CFG, sysRegRead(GDMA1_FC_CFG));
	printk("GDMA2_FC_CFG(0x%08x)  : 0x%08x\n", GDMA2_FC_CFG, sysRegRead(GDMA2_FC_CFG));
	printk("PDMA_FC_CFG(0x%08x)  : 0x%08x\n", PDMA_FC_CFG, sysRegRead(PDMA_FC_CFG));
	printk("PSE_FQ_CFG(0x%08x)  : 0x%08x\n", PSE_FQ_CFG, sysRegRead(PSE_FQ_CFG));
#endif
	printk("\n\nTX_CTX_IDX0    : 0x%08x\n", sysRegRead(TX_CTX_IDX0));	
	printk("TX_DTX_IDX0    : 0x%08x\n", sysRegRead(TX_DTX_IDX0));
	printk("TX_CTX_IDX1    : 0x%08x\n", sysRegRead(TX_CTX_IDX1));	
	printk("TX_DTX_IDX1    : 0x%08x\n", sysRegRead(TX_DTX_IDX1));
	printk("TX_CTX_IDX2    : 0x%08x\n", sysRegRead(TX_CTX_IDX2));	
	printk("TX_DTX_IDX2    : 0x%08x\n", sysRegRead(TX_DTX_IDX2));
	printk("TX_CTX_IDX3    : 0x%08x\n", sysRegRead(TX_CTX_IDX3));
	printk("TX_DTX_IDX3    : 0x%08x\n", sysRegRead(TX_DTX_IDX3));
	printk("RX_CALC_IDX0   : 0x%08x\n", sysRegRead(RX_CALC_IDX0));
	printk("RX_DRX_IDX0    : 0x%08x\n", sysRegRead(RX_DRX_IDX0));

	printk("\n------------------------------\n\n");
}
#endif

void dump_reg()
{
	printk("\n\nFE_INT_ENABLE  : 0x%08x\n", sysRegRead(FE_INT_ENABLE));
	printk("DLY_INT_CFG	: 0x%08x\n", sysRegRead(DLY_INT_CFG));
	printk("TX_BASE_PTR0   : 0x%08x\n", sysRegRead(TX_BASE_PTR0));	
	printk("TX_CTX_IDX0    : 0x%08x\n", sysRegRead(TX_CTX_IDX0));	
	printk("TX_DTX_IDX0    : 0x%08x\n", sysRegRead(TX_DTX_IDX0));
	printk("TX_BASE_PTR1(0x%08x)   : 0x%08x\n", TX_BASE_PTR1, sysRegRead(TX_BASE_PTR1));	
	printk("TX_CTX_IDX1(0x%08x)    : 0x%08x\n", TX_CTX_IDX1, sysRegRead(TX_CTX_IDX1));
	printk("TX_DTX_IDX1(0x%08x)    : 0x%08x\n", TX_DTX_IDX1, sysRegRead(TX_DTX_IDX1));
	printk("TX_BASE_PTR2(0x%08x)   : 0x%08x\n", TX_BASE_PTR2, sysRegRead(TX_BASE_PTR2));	
	printk("TX_CTX_IDX2(0x%08x)    : 0x%08x\n", TX_CTX_IDX2, sysRegRead(TX_CTX_IDX2));
	printk("TX_DTX_IDX2(0x%08x)    : 0x%08x\n", TX_DTX_IDX2, sysRegRead(TX_DTX_IDX2));
	printk("TX_BASE_PTR3(0x%08x)   : 0x%08x\n", TX_BASE_PTR3, sysRegRead(TX_BASE_PTR3));	
	printk("TX_CTX_IDX3(0x%08x)    : 0x%08x\n", TX_CTX_IDX3, sysRegRead(TX_CTX_IDX3));
	printk("TX_DTX_IDX3(0x%08x)    : 0x%08x\n", TX_DTX_IDX3, sysRegRead(TX_DTX_IDX3));

	printk("RX_BASE_PTR0   : 0x%08x\n", sysRegRead(RX_BASE_PTR0));	
	printk("RX_MAX_CNT0    : 0x%08x\n", sysRegRead(RX_MAX_CNT0));	
	printk("RX_CALC_IDX0   : 0x%08x\n", sysRegRead(RX_CALC_IDX0));
	printk("RX_DRX_IDX0    : 0x%08x\n", sysRegRead(RX_DRX_IDX0));
	
#if defined (CONFIG_ETHTOOL)
	printk("The current PHY address selected by ethtool is %d\n", get_current_phy_address());
#endif

#if defined (CONFIG_RALINK_RT2883) || defined(CONFIG_RALINK_RT3883)
	printk("GDMA_RX_FCCNT1(0x%08x)     : 0x%08x\n\n", GDMA_RX_FCCNT1, sysRegRead(GDMA_RX_FCCNT1));	
#endif
}

void dump_cp0(void)
{
	printk("CP0 Register dump --\n");
	printk("CP0_INDEX\t: 0x%08x\n", read_32bit_cp0_register(CP0_INDEX));
	printk("CP0_RANDOM\t: 0x%08x\n", read_32bit_cp0_register(CP0_RANDOM));
	printk("CP0_ENTRYLO0\t: 0x%08x\n", read_32bit_cp0_register(CP0_ENTRYLO0));
	printk("CP0_ENTRYLO1\t: 0x%08x\n", read_32bit_cp0_register(CP0_ENTRYLO1));
	printk("CP0_CONF\t: 0x%08x\n", read_32bit_cp0_register(CP0_CONF));
	printk("CP0_CONTEXT\t: 0x%08x\n", read_32bit_cp0_register(CP0_CONTEXT));
	printk("CP0_PAGEMASK\t: 0x%08x\n", read_32bit_cp0_register(CP0_PAGEMASK));
	printk("CP0_WIRED\t: 0x%08x\n", read_32bit_cp0_register(CP0_WIRED));
	printk("CP0_INFO\t: 0x%08x\n", read_32bit_cp0_register(CP0_INFO));
	printk("CP0_BADVADDR\t: 0x%08x\n", read_32bit_cp0_register(CP0_BADVADDR));
	printk("CP0_COUNT\t: 0x%08x\n", read_32bit_cp0_register(CP0_COUNT));
	printk("CP0_ENTRYHI\t: 0x%08x\n", read_32bit_cp0_register(CP0_ENTRYHI));
	printk("CP0_COMPARE\t: 0x%08x\n", read_32bit_cp0_register(CP0_COMPARE));
	printk("CP0_STATUS\t: 0x%08x\n", read_32bit_cp0_register(CP0_STATUS));
	printk("CP0_CAUSE\t: 0x%08x\n", read_32bit_cp0_register(CP0_CAUSE));
	printk("CP0_EPC\t: 0x%08x\n", read_32bit_cp0_register(CP0_EPC));
	printk("CP0_PRID\t: 0x%08x\n", read_32bit_cp0_register(CP0_PRID));
	printk("CP0_CONFIG\t: 0x%08x\n", read_32bit_cp0_register(CP0_CONFIG));
	printk("CP0_LLADDR\t: 0x%08x\n", read_32bit_cp0_register(CP0_LLADDR));
	printk("CP0_WATCHLO\t: 0x%08x\n", read_32bit_cp0_register(CP0_WATCHLO));
	printk("CP0_WATCHHI\t: 0x%08x\n", read_32bit_cp0_register(CP0_WATCHHI));
	printk("CP0_XCONTEXT\t: 0x%08x\n", read_32bit_cp0_register(CP0_XCONTEXT));
	printk("CP0_FRAMEMASK\t: 0x%08x\n", read_32bit_cp0_register(CP0_FRAMEMASK));
	printk("CP0_DIAGNOSTIC\t: 0x%08x\n", read_32bit_cp0_register(CP0_DIAGNOSTIC));
	printk("CP0_DEBUG\t: 0x%08x\n", read_32bit_cp0_register(CP0_DEBUG));
	printk("CP0_DEPC\t: 0x%08x\n", read_32bit_cp0_register(CP0_DEPC));
	printk("CP0_PERFORMANCE\t: 0x%08x\n", read_32bit_cp0_register(CP0_PERFORMANCE));
	printk("CP0_ECC\t: 0x%08x\n", read_32bit_cp0_register(CP0_ECC));
	printk("CP0_CACHEERR\t: 0x%08x\n", read_32bit_cp0_register(CP0_CACHEERR));
	printk("CP0_TAGLO\t: 0x%08x\n", read_32bit_cp0_register(CP0_TAGLO));
	printk("CP0_TAGHI\t: 0x%08x\n", read_32bit_cp0_register(CP0_TAGHI));
	printk("CP0_ERROREPC\t: 0x%08x\n", read_32bit_cp0_register(CP0_ERROREPC));
	printk("CP0_DESAVE\t: 0x%08x\n\n", read_32bit_cp0_register(CP0_DESAVE));
}

struct proc_dir_entry *procRegDir;
static struct proc_dir_entry *procGmac, *procSysCP0, *procTxRing, *procRxRing, *procSkbFree;
#if defined(CONFIG_RAETH_SNMPD)
static struct proc_dir_entry *procRaSnmp;
#endif
#if defined(CONFIG_RAETH_TSO)
static struct proc_dir_entry *procNumOfTxd, *procTsoLen;
#endif

#if defined(CONFIG_RAETH_LRO)
static struct proc_dir_entry *procLroStats;
#endif
#if defined (TASKLET_WORKQUEUE_SW)
static struct proc_dir_entry *procSCHE;
#endif

int RegReadMain(void)
{
	dump_reg();
	return 0;
}

int SkbFreeRead(void)
{
	END_DEVICE *ei_local = netdev_priv(dev_raether);
	int i = 0;

	for (i=0; i < NUM_TX_DESC; i++) {
		printk("%d: %08x\n",i,  *(int *)&ei_local->skb_free[i]);
        }
	return 0;
}
int TxRingRead(void)
{
	END_DEVICE *ei_local = netdev_priv(dev_raether);
	int i = 0;

	for (i=0; i < NUM_TX_DESC; i++) {
#ifdef CONFIG_32B_DESC
		printk("%d: %08x %08x %08x %08x %08x %08x %08x %08x\n",i,  *(int *)&ei_local->tx_ring0[i].txd_info1, 
				*(int *)&ei_local->tx_ring0[i].txd_info2, *(int *)&ei_local->tx_ring0[i].txd_info3, 
				*(int *)&ei_local->tx_ring0[i].txd_info4, *(int *)&ei_local->tx_ring0[i].txd_info5, 
				*(int *)&ei_local->tx_ring0[i].txd_info6, *(int *)&ei_local->tx_ring0[i].txd_info7);
#else
		printk("%d: %08x %08x %08x %08x\n",i,  *(int *)&ei_local->tx_ring0[i].txd_info1,
				*(int *)&ei_local->tx_ring0[i].txd_info2, *(int *)&ei_local->tx_ring0[i].txd_info3,
				*(int *)&ei_local->tx_ring0[i].txd_info4);
#endif
        }
	return 0;
}

int RxRingRead(void)
{
	END_DEVICE *ei_local = netdev_priv(dev_raether);
	int i = 0;

	for (i=0; i < NUM_RX_DESC; i++) {
#ifdef CONFIG_32B_DESC
		printk("%d: %08x %08x %08x %08x %08x %08x %08x %08x\n",i,  *(int *)&ei_local->rx_ring0[i].rxd_info1,
				*(int *)&ei_local->rx_ring0[i].rxd_info2, *(int *)&ei_local->rx_ring0[i].rxd_info3,
				*(int *)&ei_local->rx_ring0[i].rxd_info4, *(int *)&ei_local->rx_ring0[i].rxd_info5,
				*(int *)&ei_local->rx_ring0[i].rxd_info6, *(int *)&ei_local->rx_ring0[i].rxd_info7);
#else
		printk("%d: %08x %08x %08x %08x\n",i,  *(int *)&ei_local->rx_ring0[i].rxd_info1,
				*(int *)&ei_local->rx_ring0[i].rxd_info2, *(int *)&ei_local->rx_ring0[i].rxd_info3,
				*(int *)&ei_local->rx_ring0[i].rxd_info4);
#endif
        }
	return 0;
}

#if defined(CONFIG_RAETH_TSO)

int NumOfTxdUpdate(int num_of_txd)
{

	txd_cnt[num_of_txd]++;

	return 0;	
}

int NumOfTxdRead(void)
{
	int i=0;

	printk("TXD | Count\n");
	for(i=0; i< MAX_SKB_FRAGS/2 + 1; i++) {
		printk("%d: %d\n",i , txd_cnt[i]);
	}

	return 0;
}

int NumOfTxdWrite(struct file *file, const char *buffer, unsigned long count, void *data)
{
	memset(txd_cnt, 0, sizeof(txd_cnt));
        printk("clear txd cnt table\n");

	return count;
}

int TsoLenUpdate(int tso_len)
{

	if(tso_len > 70000) {
		tso_cnt[14]++;
	}else if(tso_len >  65000) {
		tso_cnt[13]++;
	}else if(tso_len >  60000) {
		tso_cnt[12]++;
	}else if(tso_len >  55000) {
		tso_cnt[11]++;
	}else if(tso_len >  50000) {
		tso_cnt[10]++;
	}else if(tso_len >  45000) {
		tso_cnt[9]++;
	}else if(tso_len > 40000) {
		tso_cnt[8]++;
	}else if(tso_len > 35000) {
		tso_cnt[7]++;
	}else if(tso_len > 30000) {
		tso_cnt[6]++;
	}else if(tso_len > 25000) {
		tso_cnt[5]++;
	}else if(tso_len > 20000) {
		tso_cnt[4]++;
	}else if(tso_len > 15000) {
		tso_cnt[3]++;
	}else if(tso_len > 10000) {
		tso_cnt[2]++;
	}else if(tso_len > 5000) {
		tso_cnt[1]++;
	}else {
		tso_cnt[0]++;
	}

	return 0;	
}

int TsoLenWrite(struct file *file, const char *buffer, unsigned long count, void *data)
{
	memset(tso_cnt, 0, sizeof(tso_cnt));
        printk("clear tso cnt table\n");

	return count;
}

int TsoLenRead(void)
{
	int i=0;

	printk(" Length  | Count\n");
	for(i=0; i<15; i++) {
		printk("%d~%d: %d\n", i*5000, (i+1)*5000, tso_cnt[i]);
	}

	return 0;
}

#endif

#if defined(CONFIG_RAETH_LRO)
static int LroLenUpdate(struct net_lro_desc *lro_desc)
{
	int len_idx;

	if(lro_desc->ip_tot_len > 65000) {
		len_idx = 13;
	}else if(lro_desc->ip_tot_len > 60000) {
		len_idx = 12;
	}else if(lro_desc->ip_tot_len > 55000) {
		len_idx = 11;
	}else if(lro_desc->ip_tot_len > 50000) {
		len_idx = 10;
	}else if(lro_desc->ip_tot_len > 45000) {
		len_idx = 9;
	}else if(lro_desc->ip_tot_len > 40000) {
		len_idx = 8;
	}else if(lro_desc->ip_tot_len > 35000) {
		len_idx = 7;
	}else if(lro_desc->ip_tot_len > 30000) {
		len_idx = 6;
	}else if(lro_desc->ip_tot_len > 25000) {
		len_idx = 5;
	}else if(lro_desc->ip_tot_len > 20000) {
		len_idx = 4;
	}else if(lro_desc->ip_tot_len > 15000) {
		len_idx = 3;
	}else if(lro_desc->ip_tot_len > 10000) {
		len_idx = 2;
	}else if(lro_desc->ip_tot_len > 5000) {
		len_idx = 1;
	}else {
		len_idx = 0;
	}

	return len_idx;
}
int LroStatsUpdate(struct net_lro_mgr *lro_mgr, bool all_flushed)
{
	struct net_lro_desc *tmp;
	int len_idx;
	int i, j; 
	
	if (all_flushed) {
		for (i=0; i< MAX_DESC; i++) {
			tmp = & lro_mgr->lro_arr[i];
			if (tmp->pkt_aggr_cnt !=0) {
				for(j=0; j<=MAX_AGGR; j++) {
					if(tmp->pkt_aggr_cnt == j) {
						lro_flush_cnt[j]++;
					}
				}
				len_idx = LroLenUpdate(tmp);
			       	lro_len_cnt1[len_idx]++;
				tot_called1++;
			}
			aggregated[i] = 0;
		}
	} else {
		if (lro_flushed != lro_mgr->stats.flushed) {
			if (lro_aggregated != lro_mgr->stats.aggregated) {
				for (i=0; i<MAX_DESC; i++) {
					tmp = &lro_mgr->lro_arr[i];
					if ((aggregated[i]!= tmp->pkt_aggr_cnt) 
							&& (tmp->pkt_aggr_cnt == 0)) {
						aggregated[i] ++;
						for (j=0; j<=MAX_AGGR; j++) {
							if (aggregated[i] == j) {
								lro_stats_cnt[j] ++;
							}
						}
						aggregated[i] = 0;
						//len_idx = LroLenUpdate(tmp);
			       			//lro_len_cnt2[len_idx]++;
						tot_called2++;
					}
				}
			} else {
				for (i=0; i<MAX_DESC; i++) {
					tmp = &lro_mgr->lro_arr[i];
					if ((aggregated[i] != 0) && (tmp->pkt_aggr_cnt==0)) {
						for (j=0; j<=MAX_AGGR; j++) {
							if (aggregated[i] == j) {
								lro_stats_cnt[j] ++;
							}
						}
						aggregated[i] = 0;
						//len_idx = LroLenUpdate(tmp);
			       			//lro_len_cnt2[len_idx]++;
						force_flush ++;
						tot_called2++;
					}
				}
			}
		} else {
			if (lro_aggregated != lro_mgr->stats.aggregated) {
				for (i=0; i<MAX_DESC; i++) {
					tmp = &lro_mgr->lro_arr[i];
					if (tmp->active) {
						if (aggregated[i] != tmp->pkt_aggr_cnt)
							aggregated[i] = tmp->pkt_aggr_cnt;
					} else
						aggregated[i] = 0;
				}
			} 
		}

	}

	lro_aggregated = lro_mgr->stats.aggregated;
	lro_flushed = lro_mgr->stats.flushed;
	lro_nodesc = lro_mgr->stats.no_desc;

	return 0;
		
}


int LroStatsWrite(struct file *file, const char *buffer, unsigned long count, void *data)
{
	memset(lro_stats_cnt, 0, sizeof(lro_stats_cnt));
	memset(lro_flush_cnt, 0, sizeof(lro_flush_cnt));
	memset(lro_len_cnt1, 0, sizeof(lro_len_cnt1));
	//memset(lro_len_cnt2, 0, sizeof(lro_len_cnt2));
	memset(aggregated, 0, sizeof(aggregated));
	lro_aggregated = 0;
	lro_flushed = 0;
	lro_nodesc = 0;
	force_flush = 0;
	tot_called1 = 0;
	tot_called2 = 0;
        printk("clear lro  cnt table\n");

	return count;
}

int LroStatsRead(void)
{
	int i;
	int tot_cnt=0;
	int tot_aggr=0;
	int ave_aggr=0;
	
	printk("LRO statistic dump:\n");
	printk("Cnt:   Kernel | Driver\n");
	for(i=0; i<=MAX_AGGR; i++) {
		tot_cnt = tot_cnt + lro_stats_cnt[i] + lro_flush_cnt[i];
		printk(" %d :      %d        %d\n", i, lro_stats_cnt[i], lro_flush_cnt[i]);
		tot_aggr = tot_aggr + i * (lro_stats_cnt[i] + lro_flush_cnt[i]);
	}
	ave_aggr = lro_aggregated/lro_flushed;
	printk("Total aggregated pkt: %d\n", lro_aggregated);
	printk("Flushed pkt: %d  %d\n", lro_flushed, force_flush);
	printk("Average flush cnt:  %d\n", ave_aggr);
	printk("No descriptor pkt: %d\n\n\n", lro_nodesc);

	printk("Driver flush pkt len:\n");
	printk(" Length  | Count\n");
	for(i=0; i<15; i++) {
		printk("%d~%d: %d\n", i*5000, (i+1)*5000, lro_len_cnt1[i]);
	}
	printk("Kernel flush: %d;  Driver flush: %d\n", tot_called2, tot_called1);
	return 0;
}

int getnext(const char *src, int separator, char *dest)
{
    char *c;
    int len;

    if ( (src == NULL) || (dest == NULL) ) {
        return -1;
    }

    c = strchr(src, separator);
    if (c == NULL) {
        strcpy(dest, src);
        return -1;
    }
    len = c - src;
    strncpy(dest, src, len);
    dest[len] = '\0';
    return len + 1;
}

int str_to_ip(unsigned int *ip, const char *str)
{
    int len;
    const char *ptr = str;
    char buf[128];
    unsigned char c[4];
    int i;

    for (i = 0; i < 3; ++i) {
        if ((len = getnext(ptr, '.', buf)) == -1) {
            return 1; /* parse error */
        }
        c[i] = simple_strtoul(buf, NULL, 10);
        ptr += len;
    }
    c[3] = simple_strtoul(ptr, NULL, 0);
    *ip = (c[0]<<24) + (c[1]<<16) + (c[2]<<8) + c[3];
    return 0;
}
#endif

int CP0RegRead(void)
{
	dump_cp0();
	return 0;
}

#if defined(CONFIG_RAETH_QOS)
static struct proc_dir_entry *procRaQOS, *procRaFeIntr, *procRaEswIntr;
extern uint32_t num_of_rxdone_intr;
extern uint32_t num_of_esw_intr;

int RaQOSRegRead(void)
{
	dump_qos();
	return 0;
}
#endif

static struct proc_dir_entry *procEswCnt;

int EswCntRead(void)
{
	printk("\n		  <<CPU>>			 \n");
	printk("		    |				 \n");
#if defined (CONFIG_RALINK_RT5350)
	printk("+-----------------------------------------------+\n");
	printk("|		  <<PDMA>>		        |\n");
	printk("+-----------------------------------------------+\n");
#else
	printk("+-----------------------------------------------+\n");
	printk("|		  <<PSE>>		        |\n");
	printk("+-----------------------------------------------+\n");
	printk("		   |				 \n");
	printk("+-----------------------------------------------+\n");
	printk("|		  <<GDMA>>		        |\n");
#if defined (CONFIG_RALINK_MT7620)
	printk("| GDMA1_TX_GPCNT  : %010u (Tx Good Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1304));	
	printk("| GDMA1_RX_GPCNT  : %010u (Rx Good Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1324));	
	printk("|						|\n");
	printk("| GDMA1_TX_SKIPCNT: %010u (skip)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1308));	
	printk("| GDMA1_TX_COLCNT : %010u (collision)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x130c));	
	printk("| GDMA1_RX_OERCNT : %010u (overflow)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1328));	
	printk("| GDMA1_RX_FERCNT : %010u (FCS error)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x132c));	
	printk("| GDMA1_RX_SERCNT : %010u (too short)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1330));	
	printk("| GDMA1_RX_LERCNT : %010u (too long)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1334));	
	printk("| GDMA1_RX_CERCNT : %010u (l3/l4 checksum) |\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1338));	
	printk("| GDMA1_RX_FCCNT  : %010u (flow control)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x133c));	

	printk("|						|\n");
	printk("| GDMA2_TX_GPCNT  : %010u (Tx Good Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1344));	
	printk("| GDMA2_RX_GPCNT  : %010u (Rx Good Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1364));	
	printk("|						|\n");
	printk("| GDMA2_TX_SKIPCNT: %010u (skip)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1348));	
	printk("| GDMA2_TX_COLCNT : %010u (collision)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x134c));	
	printk("| GDMA2_RX_OERCNT : %010u (overflow)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1368));	
	printk("| GDMA2_RX_FERCNT : %010u (FCS error)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x136c));	
	printk("| GDMA2_RX_SERCNT : %010u (too short)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1370));	
	printk("| GDMA2_RX_LERCNT : %010u (too long)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1374));	
	printk("| GDMA2_RX_CERCNT : %010u (l3/l4 checksum) |\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1378));	
	printk("| GDMA2_RX_FCCNT  : %010u (flow control)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x137c));	
#elif defined (CONFIG_RALINK_MT7621)
	printk("| GDMA1_RX_GBCNT  : %010u (Rx Good Bytes)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2400));	
	printk("| GDMA1_RX_GPCNT  : %010u (Rx Good Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2408));	
	printk("| GDMA1_RX_OERCNT : %010u (overflow error)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2410));	
	printk("| GDMA1_RX_FERCNT : %010u (FCS error)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2414));	
	printk("| GDMA1_RX_SERCNT : %010u (too short)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2418));	
	printk("| GDMA1_RX_LERCNT : %010u (too long)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x241C));	
	printk("| GDMA1_RX_CERCNT : %010u (checksum error)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2420));	
	printk("| GDMA1_RX_FCCNT  : %010u (flow control)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2424));	
	printk("| GDMA1_TX_SKIPCNT: %010u (about count)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2428));	
	printk("| GDMA1_TX_COLCNT : %010u (collision count)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x242C));	
	printk("| GDMA1_TX_GBCNT  : %010u (Tx Good Bytes)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2430));	
	printk("| GDMA1_TX_GPCNT  : %010u (Tx Good Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2438));	
	printk("|						|\n");
	printk("| GDMA2_RX_GBCNT  : %010u (Rx Good Bytes)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2440));	
	printk("| GDMA2_RX_GPCNT  : %010u (Rx Good Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2448));	
	printk("| GDMA2_RX_OERCNT : %010u (overflow error)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2450));	
	printk("| GDMA2_RX_FERCNT : %010u (FCS error)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2454));	
	printk("| GDMA2_RX_SERCNT : %010u (too short)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2458));	
	printk("| GDMA2_RX_LERCNT : %010u (too long)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x245C));	
	printk("| GDMA2_RX_CERCNT : %010u (checksum error)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2460));	
	printk("| GDMA2_RX_FCCNT  : %010u (flow control)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2464));	
	printk("| GDMA2_TX_SKIPCNT: %010u (skip)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2468));	
	printk("| GDMA2_TX_COLCNT : %010u (collision)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x246C));	
	printk("| GDMA2_TX_GBCNT  : %010u (Tx Good Bytes)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2470));	
	printk("| GDMA2_TX_GPCNT  : %010u (Tx Good Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2478));	
#else
	printk("| GDMA_TX_GPCNT1  : %010u (Tx Good Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x704));	
	printk("| GDMA_RX_GPCNT1  : %010u (Rx Good Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x724));	
	printk("|						|\n");
	printk("| GDMA_TX_SKIPCNT1: %010u (skip)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x708));	
	printk("| GDMA_TX_COLCNT1 : %010u (collision)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x70c));	
	printk("| GDMA_RX_OERCNT1 : %010u (overflow)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x728));	
	printk("| GDMA_RX_FERCNT1 : %010u (FCS error)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x72c));	
	printk("| GDMA_RX_SERCNT1 : %010u (too short)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x730));	
	printk("| GDMA_RX_LERCNT1 : %010u (too long)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x734));	
	printk("| GDMA_RX_CERCNT1 : %010u (l3/l4 checksum)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x738));	
	printk("| GDMA_RX_FCCNT1  : %010u (flow control)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x73c));	

#endif
	printk("+-----------------------------------------------+\n");
#endif

#if defined (CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_RT6855A) || \
    defined (CONFIG_RALINK_MT7620)

	printk("                      ^                          \n");
	printk("                      | Port6 Rx:%08u Good Pkt   \n", sysRegRead(RALINK_ETH_SW_BASE+0x4620)&0xFFFF);
	printk("                      | Port6 Rx:%08u Bad Pkt    \n", sysRegRead(RALINK_ETH_SW_BASE+0x4620)>>16);
	printk("                      | Port6 Tx:%08u Good Pkt   \n", sysRegRead(RALINK_ETH_SW_BASE+0x4610)&0xFFFF);
	printk("                      | Port6 Tx:%08u Bad Pkt    \n", sysRegRead(RALINK_ETH_SW_BASE+0x4610)>>16);
#if defined (CONFIG_RALINK_MT7620)
	printk("                      | Port7 Rx:%08u Good Pkt   \n", sysRegRead(RALINK_ETH_SW_BASE+0x4720)&0xFFFF);
	printk("                      | Port7 Rx:%08u Bad Pkt    \n", sysRegRead(RALINK_ETH_SW_BASE+0x4720)>>16);
	printk("                      | Port7 Tx:%08u Good Pkt   \n", sysRegRead(RALINK_ETH_SW_BASE+0x4710)&0xFFFF);
	printk("                      | Port7 Tx:%08u Bad Pkt    \n", sysRegRead(RALINK_ETH_SW_BASE+0x4710)>>16);
#endif
	printk("+---------------------v-------------------------+\n");
	printk("|		      P6		        |\n");
	printk("|        <<10/100/1000 Embedded Switch>>        |\n");
	printk("|     P0    P1    P2     P3     P4     P5       |\n");
	printk("+-----------------------------------------------+\n");
	printk("       |     |     |     |       |      |        \n");
#elif defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_MT7621) 
	/* no built-in switch */
#else
	printk("                      ^                          \n");
	printk("                      | Port6 Rx:%08u Good Pkt   \n", sysRegRead(RALINK_ETH_SW_BASE+0xE0)&0xFFFF);
	printk("                      | Port6 Tx:%08u Good Pkt   \n", sysRegRead(RALINK_ETH_SW_BASE+0xE0)>>16);
	printk("+---------------------v-------------------------+\n");
	printk("|		      P6		        |\n");
	printk("|  	     <<10/100 Embedded Switch>>	        |\n");
	printk("|     P0    P1    P2     P3     P4     P5       |\n");
	printk("+-----------------------------------------------+\n");
	printk("       |     |     |     |       |      |        \n");
#endif

#if defined (CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_RT6855A) || \
    defined (CONFIG_RALINK_MT7620)
	printk("Port0 Good RX=%08u Tx=%08u (Bad Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0x4020)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4010)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4020)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x4010)>>16);

	printk("Port1 Good RX=%08u Tx=%08u (Bad Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0x4120)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4110)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4120)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x4110)>>16);

	printk("Port2 Good RX=%08u Tx=%08u (Bad Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0x4220)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4210)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4220)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x4210)>>16);

	printk("Port3 Good RX=%08u Tx=%08u (Bad Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0x4320)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4310)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4320)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x4310)>>16);

	printk("Port4 Good RX=%08u Tx=%08u (Bad Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0x4420)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4410)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4420)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x4410)>>16);

	printk("Port5 Good RX=%08u Tx=%08u (Bad Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0x4520)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4510)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4520)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x4510)>>16);

#elif defined (CONFIG_RALINK_RT5350)
	printk("Port0 Good Pkt Cnt: RX=%08u Tx=%08u (Bad Pkt Cnt: Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xE8)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x150)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xE8)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x150)>>16);

	printk("Port1 Good Pkt Cnt: RX=%08u Tx=%08u (Bad Pkt Cnt: Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xEC)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x154)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xEC)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x154)>>16);

	printk("Port2 Good Pkt Cnt: RX=%08u Tx=%08u (Bad Pkt Cnt: Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xF0)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x158)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xF0)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x158)>>16);

	printk("Port3 Good Pkt Cnt: RX=%08u Tx=%08u (Bad Pkt Cnt: Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xF4)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x15C)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xF4)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x15c)>>16);

	printk("Port4 Good Pkt Cnt: RX=%08u Tx=%08u (Bad Pkt Cnt: Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xF8)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x160)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xF8)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x160)>>16);

	printk("Port5 Good Pkt Cnt: RX=%08u Tx=%08u (Bad Pkt Cnt: Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xFC)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x164)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xFC)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x164)>>16);
#elif defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_MT7621) 
	/* no built-in switch */
#else /* RT305x, RT3352 */
	printk("Port0: Good Pkt Cnt: RX=%08u (Bad Pkt Cnt: Rx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xE8)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xE8)>>16);
	printk("Port1: Good Pkt Cnt: RX=%08u (Bad Pkt Cnt: Rx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xEC)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xEC)>>16);
	printk("Port2: Good Pkt Cnt: RX=%08u (Bad Pkt Cnt: Rx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xF0)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xF0)>>16);
	printk("Port3: Good Pkt Cnt: RX=%08u (Bad Pkt Cnt: Rx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xF4)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xF4)>>16);
	printk("Port4: Good Pkt Cnt: RX=%08u (Bad Pkt Cnt: Rx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xF8)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xF8)>>16);
	printk("Port5: Good Pkt Cnt: RX=%08u (Bad Pkt Cnt: Rx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xFC)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xFC)>>16);
#endif
	printk("\n");

	return 0;
}

#if defined (CONFIG_ETHTOOL)
/*
 * proc write procedure
 */
static int change_phyid(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char buf[32];
	struct net_device *cur_dev_p;
	END_DEVICE *ei_local;
	char if_name[64];
	unsigned int phy_id;

	if (count > 32)
		count = 32;
	memset(buf, 0, 32);
	if (copy_from_user(buf, buffer, count))
		return -EFAULT;

	/* determine interface name */
	strcpy(if_name, DEV_NAME);	/* "eth2" by default */
	if(isalpha(buf[0]))
		sscanf(buf, "%s %d", if_name, &phy_id);
	else
		phy_id = simple_strtol(buf, 0, 10);

	for_each_netdev(&init_net, cur_dev_p) {
		if (strncmp(cur_dev_p->name, if_name, 4) == 0)
			break;
	}
	if (cur_dev_p == NULL)
		return -EFAULT;

#ifdef CONFIG_PSEUDO_SUPPORT
	/* This may be wrong when more than 2 gmacs */
	if(!strcmp(cur_dev_p->name, DEV_NAME)){
		ei_local = netdev_priv(cur_dev_p);
		ei_local->mii_info.phy_id = (unsigned char)phy_id;
	}else{
		PSEUDO_ADAPTER *pPseudoAd;
		pPseudoAd = netdev_priv(cur_dev_p);
		pPseudoAd->mii_info.phy_id = (unsigned char)phy_id;
	}
#else
	ei_local = netdev_priv(cur_dev_p);
	ei_local->mii_info.phy_id = (unsigned char)phy_id;
#endif
	return count;
}
#endif

#if defined (TASKLET_WORKQUEUE_SW)
extern int init_schedule;
extern int working_schedule;
static int ScheduleRead(void)
{
	if (init_schedule == 1)
		printk("Initialize Raeth with workqueque<%d>\n", init_schedule);
	else
		printk("Initialize Raeth with tasklet<%d>\n", init_schedule);
	if (working_schedule == 1)
		printk("Raeth is running at workqueque<%d>\n", working_schedule);
	else
		printk("Raeth is running at tasklet<%d>\n", working_schedule);

	return 0;
}

static int ScheduleWrite(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char buf[2];
	int old;
	
	if (copy_from_user(buf, buffer, count))
		return -EFAULT;
	old = init_schedule;
	init_schedule = simple_strtol(buf, 0, 10);
	printk("Change Raeth initial schedule from <%d> to <%d>\n! Not running schedule at present !\n", 
		old, init_schedule);

	return count;
}
#endif

int debug_proc_init(void)
{
    if (procRegDir == NULL)
	procRegDir = proc_mkdir(PROCREG_DIR, NULL);
   
    if ((procGmac = create_proc_entry(PROCREG_GMAC, 0, procRegDir))){
	 procGmac->read_proc = (read_proc_t*)&RegReadMain;
#if defined (CONFIG_ETHTOOL)
	 procGmac->write_proc = (write_proc_t*)&change_phyid;
#endif
	}

    if ((procSkbFree = create_proc_entry(PROCREG_SKBFREE, 0, procRegDir)))
	 procSkbFree->read_proc = (read_proc_t*)&SkbFreeRead;

    if ((procTxRing = create_proc_entry(PROCREG_TXRING, 0, procRegDir)))
	 procTxRing->read_proc = (read_proc_t*)&TxRingRead;
    
    if ((procRxRing = create_proc_entry(PROCREG_RXRING, 0, procRegDir)))
	 procRxRing->read_proc = (read_proc_t*)&RxRingRead;

    if ((procSysCP0 = create_proc_entry(PROCREG_CP0, 0, procRegDir)))
	 procSysCP0->read_proc = (read_proc_t*)&CP0RegRead;
   
#if defined(CONFIG_RAETH_TSO)
    if ((procNumOfTxd = create_proc_entry(PROCREG_NUM_OF_TXD, 0, procRegDir)))
	 procNumOfTxd->read_proc = (read_proc_t*)&NumOfTxdRead;
	 procNumOfTxd->write_proc = (write_proc_t*)&NumOfTxdWrite;
    
    if ((procTsoLen = create_proc_entry(PROCREG_TSO_LEN, 0, procRegDir)))
	 procTsoLen->read_proc = (read_proc_t*)&TsoLenRead;
	 procTsoLen->write_proc = (write_proc_t*)&TsoLenWrite;
#endif

#if defined(CONFIG_RAETH_LRO)
    if ((procLroStats = create_proc_entry(PROCREG_LRO_STATS, 0, procRegDir)))
	 procLroStats->read_proc = (read_proc_t*)&LroStatsRead;
	 procLroStats->write_proc = (write_proc_t*)&LroStatsWrite;
#endif

#if defined(CONFIG_RAETH_QOS)
    if ((procRaQOS = create_proc_entry(PROCREG_RAQOS, 0, procRegDir)))
	 procRaQOS->read_proc = (read_proc_t*)&RaQOSRegRead;
#endif

#if defined(CONFIG_RAETH_SNMPD)
    procRaSnmp = create_proc_entry(PROCREG_SNMP, S_IRUGO, procRegDir);
    if (procRaSnmp == NULL)
    	printk(KERN_ALERT "raeth: snmp proc create failed!!!");
    else
    	procRaSnmp->proc_fops = &ra_snmp_seq_fops;
#endif
   
    if ((procEswCnt = create_proc_entry( PROCREG_ESW_CNT, 0, procRegDir))){
	 procEswCnt->read_proc = (read_proc_t*)&EswCntRead;
    }

#if defined (TASKLET_WORKQUEUE_SW)
    if ((procSCHE = create_proc_entry(PROCREG_SCHE, 0, procRegDir))){
	 procSCHE->read_proc = (read_proc_t*)&ScheduleRead;
	 procSCHE->write_proc = (write_proc_t*)&ScheduleWrite;
    }
#endif

    printk(KERN_ALERT "PROC INIT OK!\n");
    return 0;
}

void debug_proc_exit(void)
{

    if (procSysCP0)
    	remove_proc_entry(PROCREG_CP0, procRegDir);

    if (procGmac)
    	remove_proc_entry(PROCREG_GMAC, procRegDir);
    
    if (procSkbFree)
    	remove_proc_entry(PROCREG_SKBFREE, procRegDir);

    if (procTxRing)
    	remove_proc_entry(PROCREG_TXRING, procRegDir);
    
    if (procRxRing)
    	remove_proc_entry(PROCREG_RXRING, procRegDir);
   
#if defined(CONFIG_RAETH_TSO)
    if (procNumOfTxd)
    	remove_proc_entry(PROCREG_NUM_OF_TXD, procRegDir);
    
    if (procTsoLen)
    	remove_proc_entry(PROCREG_TSO_LEN, procRegDir);
#endif

#if defined(CONFIG_RAETH_LRO)
    if (procLroStats)
    	remove_proc_entry(PROCREG_LRO_STATS, procRegDir);
#endif

#if defined(CONFIG_RAETH_QOS)
    if (procRaQOS)
    	remove_proc_entry(PROCREG_RAQOS, procRegDir);
    if (procRaFeIntr)
    	remove_proc_entry(PROCREG_RXDONE_INTR, procRegDir);
    if (procRaEswIntr)
    	remove_proc_entry(PROCREG_ESW_INTR, procRegDir);
#endif

#if defined(CONFIG_RAETH_SNMPD)
    if (procRaSnmp)
	remove_proc_entry(PROCREG_SNMP, procRegDir);
#endif

    if (procEswCnt)
    	remove_proc_entry(PROCREG_ESW_CNT, procRegDir);
    
    //if (procRegDir)
   	//remove_proc_entry(PROCREG_DIR, 0);
	
    printk(KERN_ALERT "proc exit\n");
}
EXPORT_SYMBOL(procRegDir);
