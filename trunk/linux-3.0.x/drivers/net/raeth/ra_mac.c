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

#include "ra2882ethreg.h"
#include "raether.h"
#include "ra_mac.h"
#include "ra_ethtool.h"

extern struct net_device *dev_raether;

#if defined(CONFIG_RAETH_SNMPD)

static int ra_snmp_seq_show(struct seq_file *seq, void *v)
{
	char strprint[100];

#if !defined(CONFIG_RALINK_RT5350)
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

#if defined (CONFIG_RALINK_RT2880) || defined(CONFIG_RALINK_RT3883) || \
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

void ra2880MacAddressSet(unsigned char p[6])
{
	unsigned long regValue;

	regValue = (p[0] << 8) | (p[1]);
#if defined (CONFIG_RALINK_RT5350)
	sysRegWrite(SDM_MAC_ADRH, regValue);
#else
	sysRegWrite(GDMA1_MAC_ADRH, regValue);
#endif

	regValue = (p[2] << 24) | (p[3] <<16) | (p[4] << 8) | p[5];
#if defined (CONFIG_RALINK_RT5350)
	sysRegWrite(SDM_MAC_ADRL, regValue);
#else
	sysRegWrite(GDMA1_MAC_ADRL, regValue);
#endif
}

#ifdef CONFIG_PSEUDO_SUPPORT
void ra2880Mac2AddressSet(unsigned char p[6])
{
	unsigned long regValue;

	regValue = (p[0] << 8) | (p[1]);
	sysRegWrite(GDMA2_MAC_ADRH, regValue);

	regValue = (p[2] << 24) | (p[3] <<16) | (p[4] << 8) | p[5];
	sysRegWrite(GDMA2_MAC_ADRL, regValue);
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
static struct proc_dir_entry *procGmac, *procSysCP0, *procTxRing, *procRxRing;
#if defined(CONFIG_RAETH_SNMPD)
static struct proc_dir_entry *procRaSnmp;
#endif

#ifdef CONFIG_RAETH_HW_VLAN_TX
static struct proc_dir_entry *procVlanTx;
#endif


int RegReadMain(void)
{
	dump_reg();
	return 0;
}

int TxRingRead(void)
{
	END_DEVICE *ei_local = netdev_priv(dev_raether);
	int i = 0;

	for (i=0; i < NUM_TX_DESC; i++) {
		printk("%d: %08x %08x %08x %08x\n",i,
				ei_local->tx_ring0[i].txd_info1_u32,
				ei_local->tx_ring0[i].txd_info2_u32,
				ei_local->tx_ring0[i].txd_info3_u32,
				ei_local->tx_ring0[i].txd_info4_u32);
	}
	return 0;
}

int RxRingRead(void)
{
	END_DEVICE *ei_local = netdev_priv(dev_raether);
	int i = 0;

	for (i=0; i < NUM_RX_DESC; i++) {
		printk("%d: %08x %08x %08x %08x\n",i,
				ei_local->rx_ring0[i].rxd_info1_u32,
				ei_local->rx_ring0[i].rxd_info2_u32,
				ei_local->rx_ring0[i].rxd_info3_u32,
				ei_local->rx_ring0[i].rxd_info4_u32);
	}
	return 0;
}

int CP0RegRead(void)
{
	dump_cp0();
	return 0;
}

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
	printk("+-----------------------------------------------+\n");
#endif

#if defined (CONFIG_RALINK_RT3883)
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

#if defined (CONFIG_RALINK_RT5350)
	printk("Port0 Good Pkt Cnt: RX=%08u Tx=%08u (Bad Pkt Cnt: Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xE8)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x150)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xE8)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x150)>>16);
	printk("Port1 Good Pkt Cnt: RX=%08u Tx=%08u (Bad Pkt Cnt: Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xEC)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x154)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xEC)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x154)>>16);
	printk("Port2 Good Pkt Cnt: RX=%08u Tx=%08u (Bad Pkt Cnt: Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xF0)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x158)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xF0)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x158)>>16);
	printk("Port3 Good Pkt Cnt: RX=%08u Tx=%08u (Bad Pkt Cnt: Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xF4)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x15C)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xF4)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x15c)>>16);
	printk("Port4 Good Pkt Cnt: RX=%08u Tx=%08u (Bad Pkt Cnt: Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xF8)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x160)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xF8)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x160)>>16);
	printk("Port5 Good Pkt Cnt: RX=%08u Tx=%08u (Bad Pkt Cnt: Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xFC)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x164)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xFC)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x164)>>16);
#elif defined (CONFIG_RALINK_RT3883)
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

#ifdef CONFIG_RAETH_HW_VLAN_TX
extern unsigned int vlan_tx_idx14;
extern unsigned int vlan_tx_idx15;

static int VlanTxRead(void)
{
	printk("HW_VLAN_TX VID IDX14=%d\n", vlan_tx_idx14);
	printk("HW_VLAN_TX VID IDX15=%d\n", vlan_tx_idx15);

	return 0;
}

static int VlanTxWrite(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char buf[32];
	unsigned int vidx14 = 0;
	unsigned int vidx15 = 0;

	if (count > (sizeof(buf) - 1))
		count = (sizeof(buf) - 1);
	if (copy_from_user(buf, buffer, count))
		return -EFAULT;

	buf[count] = '\0';
	sscanf(buf, "%d %d", &vidx14, &vidx15);

	if (vidx14 > 0)
		vlan_tx_idx14 = (vidx14 & 0xFFF);
	if (vidx15 > 0)
		vlan_tx_idx15 = (vidx15 & 0xFFF);

#if !defined (CONFIG_RALINK_RT5350)
	*(unsigned long *)(RALINK_FRAME_ENGINE_BASE + 0x0c4) = ((vlan_tx_idx15 << 16) | vlan_tx_idx14);
#endif

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

    if ((procTxRing = create_proc_entry(PROCREG_TXRING, 0, procRegDir)))
	 procTxRing->read_proc = (read_proc_t*)&TxRingRead;
    
    if ((procRxRing = create_proc_entry(PROCREG_RXRING, 0, procRegDir)))
	 procRxRing->read_proc = (read_proc_t*)&RxRingRead;

    if ((procSysCP0 = create_proc_entry(PROCREG_CP0, 0, procRegDir)))
	 procSysCP0->read_proc = (read_proc_t*)&CP0RegRead;

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

#ifdef CONFIG_RAETH_HW_VLAN_TX
    if ((procVlanTx = create_proc_entry(PROCREG_VLAN_TX, 0, procRegDir))){
	 procVlanTx->read_proc = (read_proc_t*)&VlanTxRead;
	 procVlanTx->write_proc = (write_proc_t*)&VlanTxWrite;
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
    
    if (procTxRing)
    	remove_proc_entry(PROCREG_TXRING, procRegDir);
    
    if (procRxRing)
    	remove_proc_entry(PROCREG_RXRING, procRegDir);

#ifdef CONFIG_RAETH_HW_VLAN_TX
    if (procVlanTx)
    	remove_proc_entry(PROCREG_VLAN_TX, procRegDir);
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
