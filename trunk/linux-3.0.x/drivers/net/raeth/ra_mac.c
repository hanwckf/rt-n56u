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
#include <linux/if_vlan.h>
#include <linux/skbuff.h>

#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#include <linux/seq_file.h>

#include "raether.h"
#include "ra_ethtool.h"
#include "ra_esw_reg.h"
#include "ra_esw_base.h"

#if defined (CONFIG_RALINK_RT3052)
#define PROCREG_DIR			"rt3052"
#elif defined (CONFIG_RALINK_RT3352)
#define PROCREG_DIR			"rt3352"
#elif defined (CONFIG_RALINK_RT5350)
#define PROCREG_DIR			"rt5350"
#elif defined (CONFIG_RALINK_RT3883)
#define PROCREG_DIR			"rt3883"
#elif defined (CONFIG_RALINK_MT7620)
#define PROCREG_DIR			"mt7620"
#elif defined (CONFIG_RALINK_MT7621)
#define PROCREG_DIR			"mt7621"
#elif defined (CONFIG_RALINK_MT7628)
#define PROCREG_DIR			"mt7628"
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
#define PROCREG_QDMA			"qdma"

extern struct net_device *dev_raether;

struct proc_dir_entry *procRegDir = NULL;
static struct proc_dir_entry *procGmac;
#if defined (CONFIG_RAETH_DEBUG)
static struct proc_dir_entry *procSysCP0, *procTxRing, *procRxRing, *procEswCnt;
#endif
#if defined (CONFIG_RAETH_QDMA)
static struct proc_dir_entry *procQdma;
#endif
#if defined (CONFIG_RAETH_SNMPD)
static struct proc_dir_entry *procSnmp;
#endif
#if defined (CONFIG_RAETH_HW_VLAN_TX) && !defined (CONFIG_RALINK_MT7621)
static struct proc_dir_entry *procVlanTx;

static int ra_vlan_tx_seq_show(struct seq_file *m, void *v)
{
	u32 i;

	seq_printf(m, "IDX VID\n");
	for (i = 0; i < 16; i++)
		seq_printf(m, "%2d: %d\n", i, get_map_hw_vlan_tx(i));

	return 0;
}

static ssize_t ra_vlan_tx_seq_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	char buf[32];
	u32 idx = 16;
	u32 vid = VLAN_N_VID;

	if (count > (sizeof(buf) - 1))
		count = (sizeof(buf) - 1);
	if (copy_from_user(buf, buffer, count))
		return -EFAULT;

	buf[count] = '\0';
	sscanf(buf, "%d: %d", &idx, &vid);

	if (idx < 16 && vid < VLAN_N_VID)
		set_map_hw_vlan_tx(idx, vid);

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

#if defined(CONFIG_RAETH_SNMPD)
static int ra_snmp_seq_show(struct seq_file *m, void *v)
{
	u32 i;
	ULARGE_INTEGER rx_goct[6], tx_goct[6];

	for (i = 0; i < 6; i++) {
		rx_goct[i].u.LowPart = esw_get_port_mib_rgoc(i, &rx_goct[i].u.HighPart);
		tx_goct[i].u.LowPart = esw_get_port_mib_tgoc(i, &tx_goct[i].u.HighPart);
	}

	seq_printf(m, "rx counters: %u %u %u %u %u %u\n",
		(u32)(rx_goct[0].QuadPart >> 20),
		(u32)(rx_goct[1].QuadPart >> 20),
		(u32)(rx_goct[2].QuadPart >> 20),
		(u32)(rx_goct[3].QuadPart >> 20),
		(u32)(rx_goct[4].QuadPart >> 20),
		(u32)(rx_goct[5].QuadPart >> 20));

	seq_printf(m, "tx counters: %u %u %u %u %u %u\n",
		(u32)(tx_goct[0].QuadPart >> 20),
		(u32)(tx_goct[1].QuadPart >> 20),
		(u32)(tx_goct[2].QuadPart >> 20),
		(u32)(tx_goct[3].QuadPart >> 20),
		(u32)(tx_goct[4].QuadPart >> 20),
		(u32)(tx_goct[5].QuadPart >> 20));

	seq_printf(m, "rx64 counters: %llu %llu %llu %llu %llu %llu\n",
		rx_goct[0].QuadPart,
		rx_goct[1].QuadPart,
		rx_goct[2].QuadPart,
		rx_goct[3].QuadPart,
		rx_goct[4].QuadPart,
		rx_goct[5].QuadPart);

	seq_printf(m, "tx64 counters: %llu %llu %llu %llu %llu %llu\n",
		tx_goct[0].QuadPart,
		tx_goct[1].QuadPart,
		tx_goct[2].QuadPart,
		tx_goct[3].QuadPart,
		tx_goct[4].QuadPart,
		tx_goct[5].QuadPart);

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

static int ra_regs_seq_show(struct seq_file *m, void *v)
{
#if defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
	seq_printf(m, "SDM_CON		: 0x%08x\n", sysRegRead(SDM_CON));
#else
	seq_printf(m, "FE_GLO_CFG	: 0x%08x\n", sysRegRead(FE_GLO_CFG));
#if defined (CONFIG_RALINK_MT7621)
	seq_printf(m, "CDMP_IG_CTRL	: 0x%08x\n", sysRegRead(CDMP_IG_CTRL));
	seq_printf(m, "CDMP_EG_CTRL	: 0x%08x\n\n", sysRegRead(CDMP_EG_CTRL));
#else
	seq_printf(m, "CDMA_CSG_CFG	: 0x%08x\n\n", sysRegRead(CDMA_CSG_CFG));
#endif

	seq_printf(m, "GDMA1_FWD_CFG	: 0x%08x\n", sysRegRead(GDMA1_FWD_CFG));
	seq_printf(m, "GDMA1_SHPR_CFG	: 0x%08x\n\n", sysRegRead(GDMA1_SHPR_CFG));
#if defined (CONFIG_PSEUDO_SUPPORT)
	seq_printf(m, "GDMA2_FWD_CFG	: 0x%08x\n", sysRegRead(GDMA2_FWD_CFG));
	seq_printf(m, "GDMA2_SHPR_CFG	: 0x%08x\n\n", sysRegRead(GDMA2_SHPR_CFG));
#endif
#endif
	seq_printf(m, "PDMA_GLO_CFG	: 0x%08x\n", sysRegRead(PDMA_GLO_CFG));
	seq_printf(m, "DLY_INT_CFG	: 0x%08x\n", sysRegRead(DLY_INT_CFG));
#if defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628) || \
    defined (CONFIG_RALINK_RT3052)
	seq_printf(m, "PDMA_SCH_CFG	: 0x%08x\n", sysRegRead(PDMA_SCH_CFG));
#else
	seq_printf(m, "SCH_Q01_CFG	: 0x%08x\n", sysRegRead(SCH_Q01_CFG));
	seq_printf(m, "SCH_Q23_CFG	: 0x%08x\n", sysRegRead(SCH_Q23_CFG));
#endif
#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3883) || \
    defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
	seq_printf(m, "PDMA_FC_CFG	: 0x%08x\n", sysRegRead(PDMA_FC_CFG));
#endif

#if defined (CONFIG_RAETH_QDMA)
	seq_printf(m, "\n");
	seq_printf(m, "QDMA_GLO_CFG	: 0x%08x\n", sysRegRead(QDMA_GLO_CFG));
	seq_printf(m, "QDMA_DELAY_INT	: 0x%08x\n", sysRegRead(QDMA_DELAY_INT));
#endif

	seq_printf(m, "\n");
	seq_printf(m, "FE_INT_ENABLE	: 0x%08x\n", sysRegRead(FE_INT_ENABLE));
	seq_printf(m, "FE_INT_STATUS	: 0x%08x\n", sysRegRead(FE_INT_STATUS));
#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
	seq_printf(m, "FE_INT_ENABLE2	: 0x%08x\n", sysRegRead(FE_INT_ENABLE2));
	seq_printf(m, "FE_INT_STATUS2	: 0x%08x\n", sysRegRead(FE_INT_STATUS2));
#endif
#if defined (CONFIG_RAETH_QDMA)
	seq_printf(m, "QFE_INT_ENABLE	: 0x%08x\n", sysRegRead(QFE_INT_ENABLE));
	seq_printf(m, "QFE_INT_STATUS	: 0x%08x\n", sysRegRead(QFE_INT_STATUS));
#endif

	seq_printf(m, "\n");
	seq_printf(m, "TX_BASE_PTR0	: 0x%08x\n", sysRegRead(TX_BASE_PTR0));
	seq_printf(m, "TX_CTX_IDX0	: 0x%08x\n", sysRegRead(TX_CTX_IDX0));
	seq_printf(m, "TX_DTX_IDX0	: 0x%08x\n\n", sysRegRead(TX_DTX_IDX0));

	seq_printf(m, "TX_BASE_PTR1	: 0x%08x\n", sysRegRead(TX_BASE_PTR1));
	seq_printf(m, "TX_CTX_IDX1	: 0x%08x\n", sysRegRead(TX_CTX_IDX1));
	seq_printf(m, "TX_DTX_IDX1	: 0x%08x\n\n", sysRegRead(TX_DTX_IDX1));

	seq_printf(m, "TX_BASE_PTR2	: 0x%08x\n", sysRegRead(TX_BASE_PTR2));
	seq_printf(m, "TX_CTX_IDX2	: 0x%08x\n", sysRegRead(TX_CTX_IDX2));
	seq_printf(m, "TX_DTX_IDX2	: 0x%08x\n\n", sysRegRead(TX_DTX_IDX2));

	seq_printf(m, "TX_BASE_PTR3	: 0x%08x\n", sysRegRead(TX_BASE_PTR3));
	seq_printf(m, "TX_CTX_IDX3	: 0x%08x\n", sysRegRead(TX_CTX_IDX3));
	seq_printf(m, "TX_DTX_IDX3	: 0x%08x\n\n", sysRegRead(TX_DTX_IDX3));

	seq_printf(m, "RX_BASE_PTR0	: 0x%08x\n", sysRegRead(RX_BASE_PTR0));
	seq_printf(m, "RX_CALC_IDX0	: 0x%08x\n", sysRegRead(RX_CALC_IDX0));
	seq_printf(m, "RX_DRX_IDX0	: 0x%08x\n\n", sysRegRead(RX_DRX_IDX0));

#if defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628) || \
    defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
	seq_printf(m, "RX_BASE_PTR1	: 0x%08x\n", sysRegRead(RX_BASE_PTR1));
	seq_printf(m, "RX_CALC_IDX1	: 0x%08x\n", sysRegRead(RX_CALC_IDX1));
	seq_printf(m, "RX_DRX_IDX1	: 0x%08x\n\n", sysRegRead(RX_DRX_IDX1));
#endif

#if defined (CONFIG_RAETH_QDMA)
	seq_printf(m, "QRX_BASE_PTR0	: 0x%08x\n", sysRegRead(QRX_BASE_PTR0));
	seq_printf(m, "QRX_CRX_IDX0	: 0x%08x\n", sysRegRead(QRX_CRX_IDX0));
	seq_printf(m, "QRX_DRX_IDX0	: 0x%08x\n\n", sysRegRead(QRX_DRX_IDX0));

	seq_printf(m, "QRX_BASE_PTR1	: 0x%08x\n", sysRegRead(QRX_BASE_PTR1));
	seq_printf(m, "QRX_CRX_IDX1	: 0x%08x\n", sysRegRead(QRX_CRX_IDX1));
	seq_printf(m, "QRX_DRX_IDX1	: 0x%08x\n\n", sysRegRead(QRX_DRX_IDX1));

	seq_printf(m, "QTX_CTX_PTR	: 0x%08x\n", sysRegRead(QTX_CTX_PTR));
	seq_printf(m, "QTX_DTX_PTR	: 0x%08x\n", sysRegRead(QTX_DTX_PTR));
	seq_printf(m, "QTX_CRX_PTR	: 0x%08x\n", sysRegRead(QTX_CRX_PTR));
	seq_printf(m, "QTX_DRX_PTR	: 0x%08x\n\n", sysRegRead(QTX_DRX_PTR));

	seq_printf(m, "QDMA_FQ_HEAD	: 0x%08x\n", sysRegRead(QDMA_FQ_HEAD));
	seq_printf(m, "QDMA_FQ_TAIL	: 0x%08x\n", sysRegRead(QDMA_FQ_TAIL));
	seq_printf(m, "QDMA_FQ_CNT	: 0x%08x\n", sysRegRead(QDMA_FQ_CNT));
	seq_printf(m, "QDMA_FQ_BLEN	: 0x%08x\n\n", sysRegRead(QDMA_FQ_BLEN));
#endif

#if defined (CONFIG_ETHTOOL)
	seq_printf(m, "\nThe current PHY address selected by ethtool is 0x%2X\n", get_current_phy_address());
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

#if defined (CONFIG_PSEUDO_SUPPORT)
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

#if defined (CONFIG_RAETH_QDMA)
extern u8 M2Q_table[64];

static int ra_qdma_seq_show(struct seq_file *m, void *v)
{
	u32 i, queue, tx_des_cnt, hw_resv, sw_resv, sch, min_en, max_en;
	u32 min_rate, max_rate, weight, temp, queue_head, queue_tail;

	for (queue = 0; queue < 16; queue++) {
		temp = sysRegRead(QTX_CFG_0 + 0x10 * queue);
		tx_des_cnt = (temp & 0xffff0000) >> 16;
		hw_resv = (temp & 0xff00) >> 8;
		sw_resv = (temp & 0xff);
		temp = sysRegRead(QTX_CFG_0 +(0x10 * queue) + 0x4);
		sch = (temp >> 31) + 1 ;
		min_en = (temp & 0x8000000) >> 27;
		min_rate = (temp & 0x7f00000) >> 20;
		for (i = 0; i< (temp & 0xf0000) >> 16; i++)
			min_rate *= 10;
		max_en = (temp & 0x800) >> 11;
		max_rate = (temp & 0x7f0) >> 4;
		for (i = 0; i< (temp & 0xf); i++)
			max_rate *= 10;
		weight = (temp & 0xf000) >> 12;
		queue_head = sysRegRead(QTX_HEAD_0 + 0x10 * queue);
		queue_tail = sysRegRead(QTX_TAIL_0 + 0x10 * queue);

		seq_printf(m, "Queue#%d Information:\n", queue);
		seq_printf(m, "%d packets in the queue; head address is 0x%08x, tail address is 0x%08x.\n",
			tx_des_cnt, queue_head, queue_tail);
		seq_printf(m, "HW_RESV: %d; SW_RESV: %d; SCH: %d; Weighting: %d\n",
			hw_resv, sw_resv, sch, weight);
		seq_printf(m, "Min_Rate_En is %d, Min_Rate is %dKbps; Max_Rate_En is %d, Max_Rate is %dKbps.\n\n",
			min_en, min_rate, max_en, max_rate);
	}

	seq_printf(m, "skb->mark to queue mapping(skb->mark, queue):\n");
	for (i = 0; i < 64; i += 8) {
		seq_printf(m, " (%2d,%2d)(%2d,%2d)(%2d,%2d)(%2d,%2d)(%2d,%2d)(%2d,%2d)(%2d,%2d)(%2d,%2d)\n",
			i+0, M2Q_table[i+0], i+1, M2Q_table[i+1], i+2, M2Q_table[i+2], i+3, M2Q_table[i+3],
			i+4, M2Q_table[i+4], i+5, M2Q_table[i+5], i+6, M2Q_table[i+6], i+7, M2Q_table[i+7]);
	}

	return 0;
}

static int ra_qdma_seq_open(struct inode *inode, struct file *file)
{
	return single_open(file, ra_qdma_seq_show, NULL);
}

static const struct file_operations ra_qdma_seq_fops = {
	.open		= ra_qdma_seq_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
#endif

#if defined (CONFIG_RAETH_DEBUG)
static int ra_txring_seq_show(struct seq_file *m, void *v)
{
	int i;
	END_DEVICE *ei_local = netdev_priv(dev_raether);

#if defined (CONFIG_RAETH_QDMA)
	seq_printf(m, "- QDMA TX pool -\n");
	for (i = 0; i < NUM_TX_DESC; i++) {
		seq_printf(m, "%d: %08x %08x %08x %08x\n", i,
			ei_local->txd_pool[i].txd_info1,
			ei_local->txd_pool[i].txd_info2,
			ei_local->txd_pool[i].txd_info3,
			ei_local->txd_pool[i].txd_info4);
	}

#if defined (CONFIG_RA_HW_NAT_QDMA)
	seq_printf(m, "- QDMA FQ pool -\n");
	for (i = 0; i < NUM_QDMA_PAGE; i++) {
		seq_printf(m, "%d: %08x %08x %08x %08x\n", i,
			ei_local->free_head[i].txd_info1,
			ei_local->free_head[i].txd_info2,
			ei_local->free_head[i].txd_info3,
			ei_local->free_head[i].txd_info4);
	}
#endif
#else
	seq_printf(m, "- PDMA TX ring[%d] -\n", 0);
	for (i = 0; i < NUM_TX_DESC; i++) {
		seq_printf(m, "%d: %08x %08x %08x %08x\n", i,
			ei_local->txd_ring[i].txd_info1,
			ei_local->txd_ring[i].txd_info2,
			ei_local->txd_ring[i].txd_info3,
			ei_local->txd_ring[i].txd_info4);
	}
#endif

	return 0;
}

static int ra_rxring_seq_show(struct seq_file *m, void *v)
{
	int i;
	END_DEVICE *ei_local = netdev_priv(dev_raether);

	seq_printf(m, "- PDMA RX ring[%d] -\n", 0);
	for (i = 0; i < NUM_RX_DESC; i++) {
		seq_printf(m, "%d: %08x %08x %08x %08x\n", i,
				ei_local->rxd_ring[i].rxd_info1,
				ei_local->rxd_ring[i].rxd_info2,
				ei_local->rxd_ring[i].rxd_info3,
				ei_local->rxd_ring[i].rxd_info4);
	}

	return 0;
}

static int ra_cp0_seq_show(struct seq_file *m, void *v)
{
#if defined (CONFIG_MIPS)
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
#endif

	return 0;
}

static int ra_esw_seq_show(struct seq_file *m, void *v)
{
	seq_printf(m, "\n                   <<CPU>>\n");
	seq_printf(m, "                      |\n");
#if defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
	seq_printf(m, "+-----------------------------------------------+\n");
	seq_printf(m, "|                  <<PDMA>>                     |\n");
	seq_printf(m, "+-----------------------------------------------+\n");
#else
	seq_printf(m, "+-----------------------------------------------+\n");
	seq_printf(m, "|                  <<PSE>>                      |\n");
	seq_printf(m, "+-----------------------------------------------+\n");
	seq_printf(m, "                      |\n");
	seq_printf(m, "+-----------------------------------------------+\n");
	seq_printf(m, "|                  <<GDMA>>                     |\n");
	seq_printf(m, "+-----------------------------------------------+\n");
#endif
#if defined (CONFIG_MT7530_GSW)
	// todo
#elif defined (CONFIG_RALINK_MT7620)
	seq_printf(m, "                      ^                          \n");
	seq_printf(m, "                      | Port6 Rx:%08u Good Pkt   \n", sysRegRead(RALINK_ETH_SW_BASE+0x4620)&0xFFFF);
	seq_printf(m, "                      | Port6 Rx:%08u Bad Pkt    \n", sysRegRead(RALINK_ETH_SW_BASE+0x4620)>>16);
	seq_printf(m, "                      | Port6 Tx:%08u Good Pkt   \n", sysRegRead(RALINK_ETH_SW_BASE+0x4610)&0xFFFF);
	seq_printf(m, "                      | Port6 Tx:%08u Bad Pkt    \n", sysRegRead(RALINK_ETH_SW_BASE+0x4610)>>16);
	seq_printf(m, "                      | Port7 Rx:%08u Good Pkt   \n", sysRegRead(RALINK_ETH_SW_BASE+0x4720)&0xFFFF);
	seq_printf(m, "                      | Port7 Rx:%08u Bad Pkt    \n", sysRegRead(RALINK_ETH_SW_BASE+0x4720)>>16);
	seq_printf(m, "                      | Port7 Tx:%08u Good Pkt   \n", sysRegRead(RALINK_ETH_SW_BASE+0x4710)&0xFFFF);
	seq_printf(m, "                      | Port7 Tx:%08u Bad Pkt    \n", sysRegRead(RALINK_ETH_SW_BASE+0x4710)>>16);
	seq_printf(m, "+---------------------v-------------------------+\n");
	seq_printf(m, "|                     P6                        |\n");
	seq_printf(m, "|           <<10/100 Embedded Switch>>          |\n");
	seq_printf(m, "|     P0    P1    P2     P3     P4     P5       |\n");
	seq_printf(m, "+-----------------------------------------------+\n");
	seq_printf(m, "       |     |     |     |       |      |        \n");
#elif defined (CONFIG_RALINK_RT3883)
	/* no embedded switch */
#else
	seq_printf(m, "                      ^                          \n");
	seq_printf(m, "                      | Port6 Rx:%08u Good Pkt   \n", sysRegRead(RALINK_ETH_SW_BASE+0xE0)&0xFFFF);
	seq_printf(m, "                      | Port6 Tx:%08u Good Pkt   \n", sysRegRead(RALINK_ETH_SW_BASE+0xE0)>>16);
	seq_printf(m, "+---------------------v-------------------------+\n");
	seq_printf(m, "|                     P6                        |\n");
	seq_printf(m, "|           <<10/100 Embedded Switch>>          |\n");
	seq_printf(m, "|     P0    P1    P2     P3     P4     P5       |\n");
	seq_printf(m, "+-----------------------------------------------+\n");
	seq_printf(m, "       |     |     |     |       |      |        \n");
#endif
#if defined (CONFIG_MT7530_GSW)
	// todo
#elif defined (CONFIG_RALINK_MT7620)
	seq_printf(m, "Port0 Good RX=%08u Tx=%08u (Bad Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0x4020)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4010)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4020)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x4010)>>16);
	seq_printf(m, "Port1 Good RX=%08u Tx=%08u (Bad Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0x4120)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4110)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4120)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x4110)>>16);
	seq_printf(m, "Port2 Good RX=%08u Tx=%08u (Bad Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0x4220)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4210)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4220)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x4210)>>16);
	seq_printf(m, "Port3 Good RX=%08u Tx=%08u (Bad Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0x4320)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4310)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4320)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x4310)>>16);
	seq_printf(m, "Port4 Good RX=%08u Tx=%08u (Bad Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0x4420)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4410)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4420)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x4410)>>16);
	seq_printf(m, "Port5 Good RX=%08u Tx=%08u (Bad Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0x4520)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4510)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x4520)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x4510)>>16);
#elif defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
	seq_printf(m, "Port0 Good Pkt Cnt: RX=%08u Tx=%08u (Bad Pkt Cnt: Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xE8)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x150)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xE8)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x150)>>16);
	seq_printf(m, "Port1 Good Pkt Cnt: RX=%08u Tx=%08u (Bad Pkt Cnt: Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xEC)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x154)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xEC)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x154)>>16);
	seq_printf(m, "Port2 Good Pkt Cnt: RX=%08u Tx=%08u (Bad Pkt Cnt: Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xF0)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x158)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xF0)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x158)>>16);
	seq_printf(m, "Port3 Good Pkt Cnt: RX=%08u Tx=%08u (Bad Pkt Cnt: Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xF4)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x15C)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xF4)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x15c)>>16);
	seq_printf(m, "Port4 Good Pkt Cnt: RX=%08u Tx=%08u (Bad Pkt Cnt: Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xF8)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x160)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xF8)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x160)>>16);
	seq_printf(m, "Port5 Good Pkt Cnt: RX=%08u Tx=%08u (Bad Pkt Cnt: Rx=%08u Tx=%08u)\n", sysRegRead(RALINK_ETH_SW_BASE+0xFC)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0x164)&0xFFFF,sysRegRead(RALINK_ETH_SW_BASE+0xFC)>>16, sysRegRead(RALINK_ETH_SW_BASE+0x164)>>16);
#elif defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352)
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

#if defined(CONFIG_RAETH_HW_VLAN_TX) && !defined (CONFIG_RALINK_MT7621)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	procVlanTx = proc_create(PROCREG_VLAN_TX, S_IRUGO | S_IWUSR, procRegDir, &ra_vlan_tx_seq_fops);
#else
	if ((procVlanTx = create_proc_entry(PROCREG_VLAN_TX, S_IRUGO | S_IWUSR, procRegDir)))
		procVlanTx->proc_fops = &ra_vlan_tx_seq_fops;
#endif
#endif

#if defined (CONFIG_RAETH_SNMPD)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	procSnmp = proc_create(PROCREG_SNMP, S_IRUGO, procRegDir, &ra_snmp_seq_fops);
#else
	if ((procSnmp = create_proc_entry(PROCREG_SNMP, S_IRUGO, procRegDir)))
		procSnmp->proc_fops = &ra_snmp_seq_fops;
#endif
#endif

#if defined (CONFIG_RAETH_QDMA)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	procQdma = proc_create(PROCREG_QDMA, S_IRUGO, procRegDir, &ra_qdma_seq_fops);
#else
	if ((procQdma = create_proc_entry(PROCREG_QDMA, S_IRUGO, procRegDir)))
		procQdma->proc_fops = &ra_qdma_seq_fops;
#endif
#endif

#if defined (CONFIG_RAETH_DEBUG)
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
#if defined (CONFIG_RAETH_DEBUG)
	if (procEswCnt)
		remove_proc_entry(PROCREG_ESW_CNT, procRegDir);

	if (procSysCP0)
		remove_proc_entry(PROCREG_CP0, procRegDir);

	if (procRxRing)
		remove_proc_entry(PROCREG_RXRING, procRegDir);

	if (procTxRing)
		remove_proc_entry(PROCREG_TXRING, procRegDir);
#endif

#if defined (CONFIG_RAETH_QDMA)
	if (procQdma)
		remove_proc_entry(PROCREG_QDMA, procRegDir);
#endif

#if defined (CONFIG_RAETH_SNMPD)
	if (procSnmp)
		remove_proc_entry(PROCREG_SNMP, procRegDir);
#endif

#if defined (CONFIG_RAETH_HW_VLAN_TX) && !defined (CONFIG_RALINK_MT7621)
	if (procVlanTx)
		remove_proc_entry(PROCREG_VLAN_TX, procRegDir);
#endif

	if (procGmac)
		remove_proc_entry(PROCREG_GMAC, procRegDir);

//	if (procRegDir)
//		remove_proc_entry(PROCREG_DIR, 0);
}
EXPORT_SYMBOL(procRegDir);
