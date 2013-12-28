#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/ctype.h>

#include <asm/io.h>
#include <asm/bitops.h>

#include <linux/errno.h>
#include <linux/init.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>

#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#include <linux/seq_file.h>

#include "raether.h"
#include "ra_ethtool.h"

#if defined (CONFIG_RALINK_RT2880)
#define PROCREG_DIR			"rt2880"
#elif defined (CONFIG_RALINK_RT3052)
#define PROCREG_DIR			"rt3052"
#elif defined (CONFIG_RALINK_RT3352)
#define PROCREG_DIR			"rt3352"
#elif defined (CONFIG_RALINK_RT5350)
#define PROCREG_DIR			"rt5350"
#elif defined (CONFIG_RALINK_RT2883)
#define PROCREG_DIR			"rt2883"
#elif defined (CONFIG_RALINK_RT3883)
#define PROCREG_DIR			"rt3883"
#elif defined (CONFIG_RALINK_RT6855)
#define PROCREG_DIR			"rt6855"
#elif defined (CONFIG_RALINK_MT7620)
#define PROCREG_DIR			"mt7620"
#elif defined (CONFIG_RALINK_MT7621)
#define PROCREG_DIR			"mt7621"
#elif defined (CONFIG_RALINK_RT6855A)
#define PROCREG_DIR			"rt6855a"
#else
#define PROCREG_DIR			"rt2880"
#endif
#define PROCREG_TXRING			"tx_ring"
#define PROCREG_RXRING			"rx_ring"
#define PROCREG_NUM_OF_TXD		"num_of_txd"
#define PROCREG_GMAC			"gmac"
#define PROCREG_CP0			"cp0"
#define PROCREG_ESW_CNT			"esw_cnt"
#define PROCREG_SNMP			"snmp"
#define PROCREG_VLAN_TX			"vlan_tx"

extern struct net_device *dev_raether;

struct proc_dir_entry *procRegDir = NULL;
static struct proc_dir_entry *procGmac;
#ifdef RAETH_DEBUG
static struct proc_dir_entry *procSysCP0, *procTxRing, *procRxRing, *procEswCnt;
#endif
#if defined(CONFIG_RAETH_SNMPD)
static struct proc_dir_entry *procRaSnmp;
#endif
#ifdef CONFIG_RAETH_HW_VLAN_TX
extern unsigned int vlan_tx_idx14;
extern unsigned int vlan_tx_idx15;
extern void update_hw_vlan_tx(void);
static struct proc_dir_entry *procVlanTx;
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
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x3400) = 0x56330;
#endif
#if defined (CONFIG_P5_MAC_TO_PHY_MODE)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x3500) = 0x56330;
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

void ra_mac1_addr_set(unsigned char p[6])
{
	unsigned long regValue;

	regValue = (p[0] << 8) | (p[1]);
#if defined (CONFIG_RALINK_RT5350)
	sysRegWrite(SDM_MAC_ADRH, regValue);
#elif defined (CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_RT6855A)
	sysRegWrite(GDMA1_MAC_ADRH, regValue);
	/* To keep the consistence between RT6855 and RT62806, GSW should keep the register. */
	sysRegWrite(SMACCR1, regValue);
#elif defined (CONFIG_RALINK_MT7620)
	sysRegWrite(SMACCR1, regValue);
#else
	sysRegWrite(GDMA1_MAC_ADRH, regValue);
#endif

	regValue = (p[2] << 24) | (p[3] <<16) | (p[4] << 8) | p[5];
#if defined (CONFIG_RALINK_RT5350)
	sysRegWrite(SDM_MAC_ADRL, regValue);
#elif defined (CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_RT6855A)
	sysRegWrite(GDMA1_MAC_ADRL, regValue);
	/* To keep the consistence between RT6855 and RT62806, GSW should keep the register. */
	sysRegWrite(SMACCR0, regValue);
#elif defined (CONFIG_RALINK_MT7620)
	sysRegWrite(SMACCR0, regValue);
#else
	sysRegWrite(GDMA1_MAC_ADRL, regValue);
#endif
}

#ifdef CONFIG_PSEUDO_SUPPORT
void ra_mac2_addr_set(unsigned char p[6])
{
	unsigned long regValue;

	regValue = (p[0] << 8) | (p[1]);
	sysRegWrite(GDMA2_MAC_ADRH, regValue);

	regValue = (p[2] << 24) | (p[3] <<16) | (p[4] << 8) | p[5];
	sysRegWrite(GDMA2_MAC_ADRL, regValue);
}
#endif


#if defined(CONFIG_RAETH_SNMPD)
static int ra_snmp_seq_show(struct seq_file *m, void *v)
{
#if !defined(CONFIG_RALINK_RT5350) && !defined(CONFIG_RALINK_MT7620)
	seq_printf(m, "rx counters: %x %x %x %x %x %x %x\n", sysRegRead(GDMA_RX_GBCNT0), sysRegRead(GDMA_RX_GPCNT0),sysRegRead(GDMA_RX_OERCNT0), sysRegRead(GDMA_RX_FERCNT0), sysRegRead(GDMA_RX_SERCNT0), sysRegRead(GDMA_RX_LERCNT0), sysRegRead(GDMA_RX_CERCNT0));
	seq_printf(m, "fc config: %x %x %x %x\n", sysRegRead(CDMA_FC_CFG), sysRegRead(GDMA1_FC_CFG), PDMA_FC_CFG, sysRegRead(PDMA_FC_CFG));
	seq_printf(m, "scheduler: %x %x %x\n", sysRegRead(GDMA1_SCH_CFG), sysRegRead(GDMA2_SCH_CFG), sysRegRead(PDMA_SCH_CFG));
#endif
	seq_printf(m, "ports: %x %x %x %x %x %x\n", sysRegRead(PORT0_PKCOUNT), sysRegRead(PORT1_PKCOUNT), sysRegRead(PORT2_PKCOUNT), sysRegRead(PORT3_PKCOUNT), sysRegRead(PORT4_PKCOUNT), sysRegRead(PORT5_PKCOUNT));

	return 0;
}

static int ra_snmp_seq_open(struct inode *inode, struct file *file)
{
	return single_open(file, ra_snmp_seq_show, NULL);
}

static const struct file_operations ra_snmp_seq_fops = {
	.open	 = ra_snmp_seq_open,
	.read	 = seq_read,
	.llseek	 = seq_lseek,
	.release = single_release,
};
#endif

#if defined(CONFIG_RAETH_HW_VLAN_TX)
static int ra_vlan_tx_seq_show(struct seq_file *m, void *v)
{
	seq_printf(m, "HW_VLAN_TX VID14=%d\n", vlan_tx_idx14);
	seq_printf(m, "HW_VLAN_TX VID15=%d\n", vlan_tx_idx15);

	return 0;
}

static ssize_t ra_vlan_tx_seq_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
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

	update_hw_vlan_tx();

	return count;
}

static int ra_vlan_tx_seq_open(struct inode *inode, struct file *file)
{
	return single_open(file, ra_vlan_tx_seq_show, NULL);
}

static const struct file_operations ra_vlan_tx_seq_fops = {
	.open		= ra_vlan_tx_seq_open,
	.write		= ra_vlan_tx_seq_write,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
#endif

static int ra_regs_seq_show(struct seq_file *m, void *v)
{
#if defined (CONFIG_RALINK_RT5350)
	seq_printf(m, "SDM_CON		: 0x%08x\n", sysRegRead(SDM_CON));
#else
	seq_printf(m, "CDMA_CSG_CFG	: 0x%08x\n", sysRegRead(CDMA_CSG_CFG));
	seq_printf(m, "GDMA1_FWD_CFG	: 0x%08x\n", sysRegRead(GDMA1_FWD_CFG));
#if defined (CONFIG_PSEUDO_SUPPORT)
	seq_printf(m, "GDMA2_FWD_CFG	: 0x%08x\n", sysRegRead(GDMA2_FWD_CFG));
#endif
#endif
	seq_printf(m, "FE_GLO_CFG	: 0x%08x\n", sysRegRead(FE_GLO_CFG));
	seq_printf(m, "PDMA_GLO_CFG	: 0x%08x\n", sysRegRead(PDMA_GLO_CFG));
	seq_printf(m, "FE_INT_ENABLE	: 0x%08x\n", sysRegRead(FE_INT_ENABLE));
	seq_printf(m, "DLY_INT_CFG	: 0x%08x\n\n", sysRegRead(DLY_INT_CFG));
	seq_printf(m, "TX_BASE_PTR0	: 0x%08x\n", sysRegRead(TX_BASE_PTR0));
	seq_printf(m, "TX_CTX_IDX0	: 0x%08x\n", sysRegRead(TX_CTX_IDX0));
	seq_printf(m, "TX_DTX_IDX0	: 0x%08x\n\n", sysRegRead(TX_DTX_IDX0));
	seq_printf(m, "RX_BASE_PTR0	: 0x%08x\n", sysRegRead(RX_BASE_PTR0));
	seq_printf(m, "RX_CALC_IDX0	: 0x%08x\n", sysRegRead(RX_CALC_IDX0));
	seq_printf(m, "RX_DRX_IDX0	: 0x%08x\n", sysRegRead(RX_DRX_IDX0));

#if defined (CONFIG_ETHTOOL)
	seq_printf(m, "\nThe current PHY address selected by ethtool is %d\n", get_current_phy_address());
#endif

	return 0;
}

#if defined (CONFIG_ETHTOOL)
static ssize_t ra_phyid_seq_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	char buf[32];
	struct net_device *cur_dev_p;
	END_DEVICE *ei_local;
	char if_name[64];
	unsigned int phy_id;

	if (count > 32)
		count = 32;
	memset(buf, 0, sizeof(buf));
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

static int ra_regs_seq_open(struct inode *inode, struct file *file)
{
	return single_open(file, ra_regs_seq_show, NULL);
}

static const struct file_operations ra_regs_seq_fops = {
	.open		= ra_regs_seq_open,
#if defined (CONFIG_ETHTOOL)
	.write		= ra_phyid_seq_write,
#endif
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

#ifdef RAETH_DEBUG
static int ra_txring_seq_show(struct seq_file *m, void *v)
{
	int i = 0;
	END_DEVICE *ei_local = netdev_priv(dev_raether);

	for (i=0; i < NUM_TX_DESC; i++) {
#if defined (CONFIG_RAETH_32B_DESC)
		seq_printf(m, "%d: %08x %08x %08x %08x %08x %08x %08x %08x\n", i,
				ei_local->tx_ring0[i].txd_info1_u32,
				ei_local->tx_ring0[i].txd_info2_u32,
				ei_local->tx_ring0[i].txd_info3_u32,
				ei_local->tx_ring0[i].txd_info4_u32,
				ei_local->tx_ring0[i].txd_info5_u32,
				ei_local->tx_ring0[i].txd_info6_u32,
				ei_local->tx_ring0[i].txd_info7_u32,
				ei_local->tx_ring0[i].txd_info8_u32);
#else
		seq_printf(m, "%d: %08x %08x %08x %08x\n", i,
				ei_local->tx_ring0[i].txd_info1_u32,
				ei_local->tx_ring0[i].txd_info2_u32,
				ei_local->tx_ring0[i].txd_info3_u32,
				ei_local->tx_ring0[i].txd_info4_u32);
#endif
	}

	return 0;
}

static int ra_rxring_seq_show(struct seq_file *m, void *v)
{
	int i = 0;
	END_DEVICE *ei_local = netdev_priv(dev_raether);

	for (i=0; i < NUM_RX_DESC; i++) {
#if defined (CONFIG_RAETH_32B_DESC)
		seq_printf(m, "%d: %08x %08x %08x %08x %08x %08x %08x %08x\n", i,
				ei_local->rx_ring0[i].rxd_info1_u32,
				ei_local->rx_ring0[i].rxd_info2_u32,
				ei_local->rx_ring0[i].rxd_info3_u32,
				ei_local->rx_ring0[i].rxd_info4_u32,
				ei_local->rx_ring0[i].rxd_info5_u32,
				ei_local->rx_ring0[i].rxd_info6_u32,
				ei_local->rx_ring0[i].rxd_info7_u32,
				ei_local->rx_ring0[i].rxd_info8_u32);
#else
		seq_printf(m, "%d: %08x %08x %08x %08x\n", i,
				ei_local->rx_ring0[i].rxd_info1_u32,
				ei_local->rx_ring0[i].rxd_info2_u32,
				ei_local->rx_ring0[i].rxd_info3_u32,
				ei_local->rx_ring0[i].rxd_info4_u32);
#endif
	}

	return 0;
}

static int ra_cp0_seq_show(struct seq_file *m, void *v)
{
	seq_printf(m, "CP0 Register dump --\n");
	seq_printf(m, "CP0_INDEX\t: 0x%08x\n", read_32bit_cp0_register(CP0_INDEX));
	seq_printf(m, "CP0_RANDOM\t: 0x%08x\n", read_32bit_cp0_register(CP0_RANDOM));
	seq_printf(m, "CP0_ENTRYLO0\t: 0x%08x\n", read_32bit_cp0_register(CP0_ENTRYLO0));
	seq_printf(m, "CP0_ENTRYLO1\t: 0x%08x\n", read_32bit_cp0_register(CP0_ENTRYLO1));
	seq_printf(m, "CP0_CONF\t: 0x%08x\n", read_32bit_cp0_register(CP0_CONF));
	seq_printf(m, "CP0_CONTEXT\t: 0x%08x\n", read_32bit_cp0_register(CP0_CONTEXT));
	seq_printf(m, "CP0_PAGEMASK\t: 0x%08x\n", read_32bit_cp0_register(CP0_PAGEMASK));
	seq_printf(m, "CP0_WIRED\t: 0x%08x\n", read_32bit_cp0_register(CP0_WIRED));
	seq_printf(m, "CP0_INFO\t: 0x%08x\n", read_32bit_cp0_register(CP0_INFO));
	seq_printf(m, "CP0_BADVADDR\t: 0x%08x\n", read_32bit_cp0_register(CP0_BADVADDR));
	seq_printf(m, "CP0_COUNT\t: 0x%08x\n", read_32bit_cp0_register(CP0_COUNT));
	seq_printf(m, "CP0_ENTRYHI\t: 0x%08x\n", read_32bit_cp0_register(CP0_ENTRYHI));
	seq_printf(m, "CP0_COMPARE\t: 0x%08x\n", read_32bit_cp0_register(CP0_COMPARE));
	seq_printf(m, "CP0_STATUS\t: 0x%08x\n", read_32bit_cp0_register(CP0_STATUS));
	seq_printf(m, "CP0_CAUSE\t: 0x%08x\n", read_32bit_cp0_register(CP0_CAUSE));
	seq_printf(m, "CP0_EPC\t: 0x%08x\n", read_32bit_cp0_register(CP0_EPC));
	seq_printf(m, "CP0_PRID\t: 0x%08x\n", read_32bit_cp0_register(CP0_PRID));
	seq_printf(m, "CP0_CONFIG\t: 0x%08x\n", read_32bit_cp0_register(CP0_CONFIG));
	seq_printf(m, "CP0_LLADDR\t: 0x%08x\n", read_32bit_cp0_register(CP0_LLADDR));
	seq_printf(m, "CP0_WATCHLO\t: 0x%08x\n", read_32bit_cp0_register(CP0_WATCHLO));
	seq_printf(m, "CP0_WATCHHI\t: 0x%08x\n", read_32bit_cp0_register(CP0_WATCHHI));
	seq_printf(m, "CP0_XCONTEXT\t: 0x%08x\n", read_32bit_cp0_register(CP0_XCONTEXT));
	seq_printf(m, "CP0_FRAMEMASK\t: 0x%08x\n", read_32bit_cp0_register(CP0_FRAMEMASK));
	seq_printf(m, "CP0_DIAGNOSTIC\t: 0x%08x\n", read_32bit_cp0_register(CP0_DIAGNOSTIC));
	seq_printf(m, "CP0_DEBUG\t: 0x%08x\n", read_32bit_cp0_register(CP0_DEBUG));
	seq_printf(m, "CP0_DEPC\t: 0x%08x\n", read_32bit_cp0_register(CP0_DEPC));
	seq_printf(m, "CP0_PERFORMANCE\t: 0x%08x\n", read_32bit_cp0_register(CP0_PERFORMANCE));
	seq_printf(m, "CP0_ECC\t: 0x%08x\n", read_32bit_cp0_register(CP0_ECC));
	seq_printf(m, "CP0_CACHEERR\t: 0x%08x\n", read_32bit_cp0_register(CP0_CACHEERR));
	seq_printf(m, "CP0_TAGLO\t: 0x%08x\n", read_32bit_cp0_register(CP0_TAGLO));
	seq_printf(m, "CP0_TAGHI\t: 0x%08x\n", read_32bit_cp0_register(CP0_TAGHI));
	seq_printf(m, "CP0_ERROREPC\t: 0x%08x\n", read_32bit_cp0_register(CP0_ERROREPC));
	seq_printf(m, "CP0_DESAVE\t: 0x%08x\n\n", read_32bit_cp0_register(CP0_DESAVE));

	return 0;
}

static int ra_esw_seq_show(struct seq_file *m, void *v)
{
	seq_printf(m, "\n		  <<CPU>>			 \n");
	seq_printf(m, "		     |\n");
#if defined (CONFIG_RALINK_RT5350)
	seq_printf(m, "+-----------------------------------------------+\n");
	seq_printf(m, "|		  <<PDMA>>		        |\n");
	seq_printf(m, "+-----------------------------------------------+\n");
#else
	seq_printf(m, "+-----------------------------------------------+\n");
	seq_printf(m, "|		  <<PSE>>		        |\n");
	seq_printf(m, "+-----------------------------------------------+\n");
	seq_printf(m, "		     |\n");
	seq_printf(m, "+-----------------------------------------------+\n");
	seq_printf(m, "|		  <<GDMA>>		        |\n");
	seq_printf(m, "+-----------------------------------------------+\n");
#if defined (CONFIG_RALINK_MT7620)
	seq_printf(m, "| GDMA1_TX_GPCNT  : %010u (Tx Good Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1304));
	seq_printf(m, "| GDMA1_RX_GPCNT  : %010u (Rx Good Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1324));
	seq_printf(m, "|						|\n");
	seq_printf(m, "| GDMA1_TX_SKIPCNT: %010u (skip)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1308));
	seq_printf(m, "| GDMA1_TX_COLCNT : %010u (collision)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x130c));
	seq_printf(m, "| GDMA1_RX_OERCNT : %010u (overflow)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1328));
	seq_printf(m, "| GDMA1_RX_FERCNT : %010u (FCS error)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x132c));
	seq_printf(m, "| GDMA1_RX_SERCNT : %010u (too short)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1330));
	seq_printf(m, "| GDMA1_RX_LERCNT : %010u (too long)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1334));
	seq_printf(m, "| GDMA1_RX_CERCNT : %010u (l3/l4 checksum) |\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1338));
	seq_printf(m, "| GDMA1_RX_FCCNT  : %010u (flow control)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x133c));
	seq_printf(m, "|						|\n");
	seq_printf(m, "| GDMA2_TX_GPCNT  : %010u (Tx Good Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1344));
	seq_printf(m, "| GDMA2_RX_GPCNT  : %010u (Rx Good Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1364));
	seq_printf(m, "|						|\n");
	seq_printf(m, "| GDMA2_TX_SKIPCNT: %010u (skip)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1348));
	seq_printf(m, "| GDMA2_TX_COLCNT : %010u (collision)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x134c));
	seq_printf(m, "| GDMA2_RX_OERCNT : %010u (overflow)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1368));
	seq_printf(m, "| GDMA2_RX_FERCNT : %010u (FCS error)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x136c));
	seq_printf(m, "| GDMA2_RX_SERCNT : %010u (too short)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1370));
	seq_printf(m, "| GDMA2_RX_LERCNT : %010u (too long)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1374));
	seq_printf(m, "| GDMA2_RX_CERCNT : %010u (l3/l4 checksum) |\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x1378));
	seq_printf(m, "| GDMA2_RX_FCCNT  : %010u (flow control)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x137c));
#elif defined (CONFIG_RALINK_MT7621)
	seq_printf(m, "| GDMA1_RX_GBCNT  : %010u (Rx Good Bytes)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2400));
	seq_printf(m, "| GDMA1_RX_GPCNT  : %010u (Rx Good Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2408));
	seq_printf(m, "| GDMA1_RX_OERCNT : %010u (overflow error)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2410));
	seq_printf(m, "| GDMA1_RX_FERCNT : %010u (FCS error)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2414));
	seq_printf(m, "| GDMA1_RX_SERCNT : %010u (too short)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2418));
	seq_printf(m, "| GDMA1_RX_LERCNT : %010u (too long)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x241C));
	seq_printf(m, "| GDMA1_RX_CERCNT : %010u (checksum error)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2420));
	seq_printf(m, "| GDMA1_RX_FCCNT  : %010u (flow control)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2424));
	seq_printf(m, "| GDMA1_TX_SKIPCNT: %010u (about count)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2428));
	seq_printf(m, "| GDMA1_TX_COLCNT : %010u (collision count)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x242C));
	seq_printf(m, "| GDMA1_TX_GBCNT  : %010u (Tx Good Bytes)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2430));
	seq_printf(m, "| GDMA1_TX_GPCNT  : %010u (Tx Good Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2438));
	seq_printf(m, "|						|\n");
	seq_printf(m, "| GDMA2_RX_GBCNT  : %010u (Rx Good Bytes)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2440));
	seq_printf(m, "| GDMA2_RX_GPCNT  : %010u (Rx Good Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2448));
	seq_printf(m, "| GDMA2_RX_OERCNT : %010u (overflow error)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2450));
	seq_printf(m, "| GDMA2_RX_FERCNT : %010u (FCS error)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2454));
	seq_printf(m, "| GDMA2_RX_SERCNT : %010u (too short)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2458));
	seq_printf(m, "| GDMA2_RX_LERCNT : %010u (too long)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x245C));
	seq_printf(m, "| GDMA2_RX_CERCNT : %010u (checksum error)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2460));
	seq_printf(m, "| GDMA2_RX_FCCNT  : %010u (flow control)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2464));
	seq_printf(m, "| GDMA2_TX_SKIPCNT: %010u (skip)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2468));
	seq_printf(m, "| GDMA2_TX_COLCNT : %010u (collision)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x246C));
	seq_printf(m, "| GDMA2_TX_GBCNT  : %010u (Tx Good Bytes)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2470));
	seq_printf(m, "| GDMA2_TX_GPCNT  : %010u (Tx Good Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x2478));
#else
	seq_printf(m, "| GDMA_TX_GPCNT1  : %010u (Tx Good Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x704));
	seq_printf(m, "| GDMA_RX_GPCNT1  : %010u (Rx Good Pkts)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x724));
	seq_printf(m, "|						|\n");
	seq_printf(m, "| GDMA_TX_SKIPCNT1: %010u (skip)		|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x708));
	seq_printf(m, "| GDMA_TX_COLCNT1 : %010u (collision)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x70c));
	seq_printf(m, "| GDMA_RX_OERCNT1 : %010u (overflow)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x728));
	seq_printf(m, "| GDMA_RX_FERCNT1 : %010u (FCS error)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x72c));
	seq_printf(m, "| GDMA_RX_SERCNT1 : %010u (too short)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x730));
	seq_printf(m, "| GDMA_RX_LERCNT1 : %010u (too long)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x734));
	seq_printf(m, "| GDMA_RX_CERCNT1 : %010u (l3/l4 checksum)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x738));
	seq_printf(m, "| GDMA_RX_FCCNT1  : %010u (flow control)	|\n", sysRegRead(RALINK_FRAME_ENGINE_BASE+0x73c));
#endif
	seq_printf(m, "+-----------------------------------------------+\n");
#endif

#if defined (CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_RT6855A) || defined (CONFIG_RALINK_MT7620)
	seq_printf(m, "                      ^                          \n");
	seq_printf(m, "                      | Port6 Rx:%08u Good Pkt   \n", sysRegRead(RALINK_ETH_SW_BASE+0x4620)&0xFFFF);
	seq_printf(m, "                      | Port6 Rx:%08u Bad Pkt    \n", sysRegRead(RALINK_ETH_SW_BASE+0x4620)>>16);
	seq_printf(m, "                      | Port6 Tx:%08u Good Pkt   \n", sysRegRead(RALINK_ETH_SW_BASE+0x4610)&0xFFFF);
	seq_printf(m, "                      | Port6 Tx:%08u Bad Pkt    \n", sysRegRead(RALINK_ETH_SW_BASE+0x4610)>>16);
#if defined (CONFIG_RALINK_MT7620)
	seq_printf(m, "                      | Port7 Rx:%08u Good Pkt   \n", sysRegRead(RALINK_ETH_SW_BASE+0x4720)&0xFFFF);
	seq_printf(m, "                      | Port7 Rx:%08u Bad Pkt    \n", sysRegRead(RALINK_ETH_SW_BASE+0x4720)>>16);
	seq_printf(m, "                      | Port7 Tx:%08u Good Pkt   \n", sysRegRead(RALINK_ETH_SW_BASE+0x4710)&0xFFFF);
	seq_printf(m, "                      | Port7 Tx:%08u Bad Pkt    \n", sysRegRead(RALINK_ETH_SW_BASE+0x4710)>>16);
#endif
	seq_printf(m, "+---------------------v-------------------------+\n");
	seq_printf(m, "|		      P6		        |\n");
	seq_printf(m, "|        <<10/100/1000 Embedded Switch>>        |\n");
	seq_printf(m, "|     P0    P1    P2     P3     P4     P5       |\n");
	seq_printf(m, "+-----------------------------------------------+\n");
	seq_printf(m, "       |     |     |     |       |      |        \n");
#elif defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_MT7621)
	/* no built-in switch */
#else
	seq_printf(m, "                      ^                          \n");
	seq_printf(m, "                      | Port6 Rx:%08u Good Pkt   \n", sysRegRead(RALINK_ETH_SW_BASE+0xE0)&0xFFFF);
	seq_printf(m, "                      | Port6 Tx:%08u Good Pkt   \n", sysRegRead(RALINK_ETH_SW_BASE+0xE0)>>16);
	seq_printf(m, "+---------------------v-------------------------+\n");
	seq_printf(m, "|		      P6		        |\n");
	seq_printf(m, "|  	     <<10/100 Embedded Switch>>	        |\n");
	seq_printf(m, "|     P0    P1    P2     P3     P4     P5       |\n");
	seq_printf(m, "+-----------------------------------------------+\n");
	seq_printf(m, "       |     |     |     |       |      |        \n");
#endif

#if defined (CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_RT6855A) || defined (CONFIG_RALINK_MT7620)
	seq_printf(m, "Port0 Good RX=%08u Tx=%08u (Bad Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0x4020)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4010)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4020)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x4010)>>16);
	seq_printf(m, "Port1 Good RX=%08u Tx=%08u (Bad Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0x4120)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4110)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4120)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x4110)>>16);
	seq_printf(m, "Port2 Good RX=%08u Tx=%08u (Bad Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0x4220)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4210)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4220)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x4210)>>16);
	seq_printf(m, "Port3 Good RX=%08u Tx=%08u (Bad Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0x4320)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4310)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4320)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x4310)>>16);
	seq_printf(m, "Port4 Good RX=%08u Tx=%08u (Bad Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0x4420)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4410)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4420)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x4410)>>16);
	seq_printf(m, "Port5 Good RX=%08u Tx=%08u (Bad Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0x4520)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4510)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4520)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x4510)>>16);
#elif defined (CONFIG_RALINK_RT5350)
	seq_printf(m, "Port0 Good Pkt Cnt: RX=%08u Tx=%08u (Bad Pkt Cnt: Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xE8)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x150)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xE8)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x150)>>16);
	seq_printf(m, "Port1 Good Pkt Cnt: RX=%08u Tx=%08u (Bad Pkt Cnt: Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xEC)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x154)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xEC)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x154)>>16);
	seq_printf(m, "Port2 Good Pkt Cnt: RX=%08u Tx=%08u (Bad Pkt Cnt: Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xF0)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x158)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xF0)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x158)>>16);
	seq_printf(m, "Port3 Good Pkt Cnt: RX=%08u Tx=%08u (Bad Pkt Cnt: Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xF4)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x15C)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xF4)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x15c)>>16);
	seq_printf(m, "Port4 Good Pkt Cnt: RX=%08u Tx=%08u (Bad Pkt Cnt: Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xF8)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x160)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xF8)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x160)>>16);
	seq_printf(m, "Port5 Good Pkt Cnt: RX=%08u Tx=%08u (Bad Pkt Cnt: Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xFC)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x164)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xFC)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x164)>>16);
#elif defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_MT7621)
	/* no built-in switch */
#else /* RT305x, RT3352 */
	seq_printf(m, "Port0: Good Pkt Cnt: RX=%08u (Bad Pkt Cnt: Rx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xE8)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xE8)>>16);
	seq_printf(m, "Port1: Good Pkt Cnt: RX=%08u (Bad Pkt Cnt: Rx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xEC)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xEC)>>16);
	seq_printf(m, "Port2: Good Pkt Cnt: RX=%08u (Bad Pkt Cnt: Rx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xF0)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xF0)>>16);
	seq_printf(m, "Port3: Good Pkt Cnt: RX=%08u (Bad Pkt Cnt: Rx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xF4)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xF4)>>16);
	seq_printf(m, "Port4: Good Pkt Cnt: RX=%08u (Bad Pkt Cnt: Rx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xF8)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xF8)>>16);
	seq_printf(m, "Port5: Good Pkt Cnt: RX=%08u (Bad Pkt Cnt: Rx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xFC)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xFC)>>16);
#endif

	return 0;
}

static int ra_txring_seq_open(struct inode *inode, struct file *file)
{
	return single_open(file, ra_txring_seq_show, NULL);
}

static int ra_rxring_seq_open(struct inode *inode, struct file *file)
{
	return single_open(file, ra_rxring_seq_show, NULL);
}

static int ra_cp0_seq_open(struct inode *inode, struct file *file)
{
	return single_open(file, ra_cp0_seq_show, NULL);
}

static int ra_esw_seq_open(struct inode *inode, struct file *file)
{
	return single_open(file, ra_esw_seq_show, NULL);
}

static const struct file_operations ra_txring_seq_fops = {
	.open		= ra_txring_seq_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static const struct file_operations ra_rxring_seq_fops = {
	.open		= ra_rxring_seq_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static const struct file_operations ra_cp0_seq_fops = {
	.open		= ra_cp0_seq_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static const struct file_operations ra_esw_seq_fops = {
	.open		= ra_esw_seq_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
#endif

int debug_proc_init(void)
{
	if (procRegDir == NULL)
		procRegDir = proc_mkdir(PROCREG_DIR, NULL);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	procGmac = proc_create(PROCREG_GMAC, S_IRUGO | S_IWUSR, procRegDir, &ra_regs_seq_fops);
#else
	if ((procGmac = create_proc_entry(PROCREG_GMAC, S_IRUGO | S_IWUSR, procRegDir)))
		procGmac->proc_fops = &ra_regs_seq_fops;
#endif

#if defined(CONFIG_RAETH_HW_VLAN_TX)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	procVlanTx = proc_create(PROCREG_VLAN_TX, S_IRUGO | S_IWUSR, procRegDir, &ra_vlan_tx_seq_fops);
#else
	if ((procVlanTx = create_proc_entry(PROCREG_VLAN_TX, S_IRUGO | S_IWUSR, procRegDir)))
		procVlanTx->proc_fops = &ra_vlan_tx_seq_fops;
#endif
#endif

#if defined(CONFIG_RAETH_SNMPD)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	procRaSnmp = proc_create(PROCREG_SNMP, S_IRUGO, procRegDir, &ra_snmp_seq_fops);
#else
	if ((procRaSnmp = create_proc_entry(PROCREG_SNMP, S_IRUGO, procRegDir)))
		procRaSnmp->proc_fops = &ra_snmp_seq_fops;
#endif
#endif

#ifdef RAETH_DEBUG
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	procTxRing = proc_create(PROCREG_TXRING, S_IRUGO, procRegDir, &ra_txring_seq_fops);
#else
	if ((procTxRing = create_proc_entry(PROCREG_TXRING, S_IRUGO, procRegDir)))
		procTxRing->proc_fops = &ra_txring_seq_fops;
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	procRxRing = proc_create(PROCREG_RXRING, S_IRUGO, procRegDir, &ra_rxring_seq_fops);
#else
	if ((procRxRing = create_proc_entry(PROCREG_RXRING, S_IRUGO, procRegDir)))
		procRxRing->proc_fops = &ra_rxring_seq_fops;
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	procSysCP0 = proc_create(PROCREG_CP0, S_IRUGO, procRegDir, &ra_cp0_seq_fops);
#else
	if ((procSysCP0 = create_proc_entry(PROCREG_CP0, S_IRUGO, procRegDir)))
		procSysCP0->proc_fops = &ra_cp0_seq_fops;
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	procEswCnt = proc_create(PROCREG_ESW_CNT, S_IRUGO, procRegDir, &ra_esw_seq_fops);
#else
	if ((procEswCnt = create_proc_entry(PROCREG_ESW_CNT, S_IRUGO, procRegDir)))
		procEswCnt->proc_fops = &ra_esw_seq_fops;
#endif
#endif

	return 0;
}

void debug_proc_exit(void)
{
#ifdef RAETH_DEBUG
	if (procEswCnt)
		remove_proc_entry(PROCREG_ESW_CNT, procRegDir);

	if (procSysCP0)
		remove_proc_entry(PROCREG_CP0, procRegDir);

	if (procRxRing)
		remove_proc_entry(PROCREG_RXRING, procRegDir);

	if (procTxRing)
		remove_proc_entry(PROCREG_TXRING, procRegDir);
#endif

#if defined(CONFIG_RAETH_SNMPD)
	if (procRaSnmp)
		remove_proc_entry(PROCREG_SNMP, procRegDir);
#endif

#if defined(CONFIG_RAETH_HW_VLAN_TX)
	if (procVlanTx)
		remove_proc_entry(PROCREG_VLAN_TX, procRegDir);
#endif

	if (procGmac)
		remove_proc_entry(PROCREG_GMAC, procRegDir);

//	if (procRegDir)
//		remove_proc_entry(PROCREG_DIR, 0);
}
EXPORT_SYMBOL(procRegDir);
