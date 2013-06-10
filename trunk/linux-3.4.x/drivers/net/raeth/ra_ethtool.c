#include <linux/module.h>
#include <linux/version.h>

#include <linux/kernel.h>
#include <linux/sched.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/ethtool.h>

#include "ra2882ethreg.h"
#include "raether.h"
#include "ra_mac.h"
#include "ra_ethtool.h"

#define RAETHER_DRIVER_NAME		"raether"
#define RA_NUM_STATS 			4


static struct {
    const char str[ETH_GSTRING_LEN];
} ethtool_stats_keys[] = {
    { "statistic1" },
    { "statistic2" },
    { "statistic3" },
    { "statistic4" },
};

unsigned char get_current_phy_address(void)
{
	struct net_device *cur_dev_p;
	END_DEVICE *ei_local;
	for_each_netdev(&init_net, cur_dev_p) {
		if (strncmp(cur_dev_p->name, DEV_NAME /* "eth2" usually */, 4) == 0)
			break;
	}
	if(!cur_dev_p)
		return 0;
	ei_local = netdev_priv(cur_dev_p);
	return ei_local->mii_info.phy_id;
}
#if 0
static u32 et_get_tx_csum(struct net_device *dev)
{
	return (sysRegRead(GDMA1_FWD_CFG) & GDM1_DISCRC) ? 0 : 1;	// a pitfall here, "0" means to enable.
}

static u32 et_get_rx_csum(struct net_device *dev)
{
	return (sysRegRead(GDMA1_FWD_CFG) & GDM1_STRPCRC) ? 1 : 0;
}

static int et_set_tx_csum(struct net_device *dev, u32 data)
{
	int value;
	//printk("et_set_tx_csum(): data = %d\n", data);

	value = sysRegRead(GDMA1_FWD_CFG);
	if(data)
		value |= GDM1_DISCRC;
	else
		value &= ~GDM1_DISCRC;

	sysRegWrite(GDMA1_FWD_CFG, value);
    return 0;
}

static int et_set_rx_csum(struct net_device *dev, u32 data)
{
	int value;
	//printk("et_set_rx_csum(): data = %d\n", data);

	value = sysRegRead(GDMA1_FWD_CFG);
	if(data)
		value |= GDM1_STRPCRC;
	else
		value &= ~GDM1_STRPCRC;

	sysRegWrite(GDMA1_FWD_CFG, value);
    return 0;
}
#endif

#define MII_CR_ADDR			0x00
#define MII_CR_MR_AUTONEG_ENABLE	(1 << 12)
#define MII_CR_MR_RESTART_NEGOTIATION	(1 << 9)

#define AUTO_NEGOTIATION_ADVERTISEMENT	0x04
#define AN_PAUSE			(1 << 10)

static void et_get_pauseparam(struct net_device *dev, struct ethtool_pauseparam *epause)
{
	int mii_an_reg;
	int mdio_cfg_reg;
	END_DEVICE *ei_local = netdev_priv(dev);

	// get mii auto-negotiation register
	mii_mgr_read(ei_local->mii_info.phy_id, AUTO_NEGOTIATION_ADVERTISEMENT, &mii_an_reg);
	epause->autoneg = (mii_an_reg & AN_PAUSE) ? 1 : 0; //get autonet_enable flag bit
	
	mdio_cfg_reg = sysRegRead(MDIO_CFG);
	epause->tx_pause = (mdio_cfg_reg & MDIO_CFG_GP1_FC_TX) ? 1 : 0;
	epause->rx_pause = (mdio_cfg_reg & MDIO_CFG_GP1_FC_RX) ? 1 : 0;

	//printk("et_get_pauseparam(): autoneg=%d, tx_pause=%d, rx_pause=%d\n", epause->autoneg, epause->tx_pause, epause->rx_pause);
}

static int et_set_pauseparam(struct net_device *dev, struct ethtool_pauseparam *epause)
{
	int mdio_cfg_reg;
	int mii_an_reg;
	END_DEVICE *ei_local = netdev_priv(dev);

	//printk("et_set_pauseparam(): autoneg=%d, tx_pause=%d, rx_pause=%d\n", epause->autoneg, epause->tx_pause, epause->rx_pause);

	// auto-neg pause
	mii_mgr_read(ei_local->mii_info.phy_id, AUTO_NEGOTIATION_ADVERTISEMENT, &mii_an_reg);
	if(epause->autoneg)
		mii_an_reg |= AN_PAUSE;
	else
		mii_an_reg &= ~AN_PAUSE;
	mii_mgr_write(ei_local->mii_info.phy_id, AUTO_NEGOTIATION_ADVERTISEMENT, mii_an_reg);

	// tx/rx pause
	mdio_cfg_reg = sysRegRead(MDIO_CFG);
	if(epause->tx_pause)
		mdio_cfg_reg |= MDIO_CFG_GP1_FC_TX;
	else
		mdio_cfg_reg &= ~MDIO_CFG_GP1_FC_TX;
	if(epause->rx_pause)
		mdio_cfg_reg |= MDIO_CFG_GP1_FC_RX;
	else
		mdio_cfg_reg &= ~MDIO_CFG_GP1_FC_RX;
	sysRegWrite(MDIO_CFG, mdio_cfg_reg);

	return 0;
}

static int et_nway_reset(struct net_device *dev)
{
	END_DEVICE *ei_local = netdev_priv(dev);
	return mii_nway_restart(&ei_local->mii_info);
}

static u32 et_get_link(struct net_device *dev)
{
	END_DEVICE *ei_local = netdev_priv(dev);
	return mii_link_ok(&ei_local->mii_info);
}

static int et_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	END_DEVICE *ei_local = netdev_priv(dev);
	int rc;
	rc = mii_ethtool_sset(&ei_local->mii_info, cmd);
	return rc;
}

static int et_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	END_DEVICE *ei_local = netdev_priv(dev);
	mii_ethtool_gset(&ei_local->mii_info, cmd);
	return 0;
}

static u32 et_get_msglevel(struct net_device *dev)
{
	return 0;
}

static void et_set_msglevel(struct net_device *dev, u32 datum)
{
	return;
}

static void et_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
	//END_DEVICE *ei_local = netdev_priv(dev);
	strcpy(info->driver, RAETHER_DRIVER_NAME);
	strcpy(info->version, RAETH_VERSION);
	strcpy(info->bus_info, "n/a");
	info->n_stats = RA_NUM_STATS;
	info->eedump_len = 0;
	info->regdump_len = 0;
}

static int et_get_sset_count(struct net_device *dev, int string_set)
{
	switch (string_set) {
	case ETH_SS_STATS:
		return RA_NUM_STATS;
	default:
		return -EOPNOTSUPP;
	}
}

static void et_get_ethtool_stats(struct net_device *dev, struct ethtool_stats *stats, u64 *data)
{
//	END_DEVICE *ei_local = netdev_priv(dev);
	data[0] = 0;//np->xstats.early_rx;
	data[1] = 0;//np->xstats.tx_buf_mapped;
	data[2] = 0;//np->xstats.tx_timeouts;
	data[3] = 0;//np->xstats.rx_lost_in_ring;
}

static void et_get_strings(struct net_device *dev, u32 stringset, u8 *data)
{
	memcpy(data, ethtool_stats_keys, sizeof(ethtool_stats_keys));
}

/*
 *	mii_mgr_read wrapper for mii.o ethtool
 */
int mdio_read(struct net_device *dev, int phy_id, int location)
{
	unsigned int result;
	END_DEVICE *ei_local = netdev_priv(dev);
	mii_mgr_read( (unsigned int) ei_local->mii_info.phy_id, (unsigned int)location, &result);
	RAETH_PRINT("\n%s mii.o query= phy_id:%d, address:%d retval:%x\n", dev->name, phy_id, location, result);
	return (int)result;
}

/*
 *	mii_mgr_write wrapper for mii.o ethtool
 */
void mdio_write(struct net_device *dev, int phy_id, int location, int value)
{
	END_DEVICE *ei_local = netdev_priv(dev);
	RAETH_PRINT("mii.o write= phy_id:%d, address:%d value:%x\n", phy_id, location, value);
	mii_mgr_write( (unsigned int) ei_local->mii_info.phy_id, (unsigned int)location, (unsigned int)value);
	return;
}

struct ethtool_ops ra_ethtool_ops = {
	.get_drvinfo		= et_get_drvinfo,
	.get_settings		= et_get_settings,
	.set_settings		= et_set_settings,
	.get_pauseparam		= et_get_pauseparam,
	.set_pauseparam		= et_set_pauseparam,
//	.get_rx_csum		= et_get_rx_csum,
//	.set_rx_csum		= et_set_rx_csum,
//	.get_tx_csum		= et_get_tx_csum,
//	.set_tx_csum		= et_set_tx_csum,
	.nway_reset		= et_nway_reset,
	.get_link		= et_get_link,
	.get_msglevel		= et_get_msglevel,
	.set_msglevel		= et_set_msglevel,
	.get_strings		= et_get_strings,
	.get_sset_count		= et_get_sset_count,
	.get_ethtool_stats	= et_get_ethtool_stats,
/*	.get_regs_len		= et_get_regs_len,
	.get_regs		= et_get_regs,
*/
};

#ifdef CONFIG_PSEUDO_SUPPORT
/*
 * We unable to re-use the Raether functions because it is hard to tell
 * where the calling from is. From eth2 or eth3?
 *
 * These code size is around 950 bytes.
 */
static void et_virt_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
	// PSEUDO_ADAPTER *pseudo = netdev_priv(dev);
	return et_get_drvinfo(dev, info);
}

static void et_virt_get_pauseparam(struct net_device *dev, struct ethtool_pauseparam *epause)
{
	int mii_an_reg, mdio_cfg_reg;
	PSEUDO_ADAPTER *pseudo = netdev_priv(dev);

	// get mii auto-negotiation register
	mii_mgr_read(pseudo->mii_info.phy_id, AUTO_NEGOTIATION_ADVERTISEMENT, &mii_an_reg);
	epause->autoneg = (mii_an_reg & AN_PAUSE) ? 1 : 0; //get autonet_enable flag bit
	
	mdio_cfg_reg = sysRegRead(MDIO_CFG);
	epause->tx_pause = (mdio_cfg_reg & MDIO_CFG_GP1_FC_TX) ? 1 : 0;
	epause->rx_pause = (mdio_cfg_reg & MDIO_CFG_GP1_FC_RX) ? 1 : 0;

	//printk("et_get_pauseparam(): autoneg=%d, tx_pause=%d, rx_pause=%d\n", epause->autoneg, epause->tx_pause, epause->rx_pause);
}

static int et_virt_set_pauseparam(struct net_device *dev, struct ethtool_pauseparam *epause)
{
	int mdio_cfg_reg;
	int mii_an_reg;
	PSEUDO_ADAPTER *pseudo = netdev_priv(dev);

	//printk("et_set_pauseparam(): autoneg=%d, tx_pause=%d, rx_pause=%d\n", epause->autoneg, epause->tx_pause, epause->rx_pause);
	// auto-neg pause
	mii_mgr_read(pseudo->mii_info.phy_id, AUTO_NEGOTIATION_ADVERTISEMENT, &mii_an_reg);
	if(epause->autoneg)
		mii_an_reg |= AN_PAUSE;
	else
		mii_an_reg &= ~AN_PAUSE;
	mii_mgr_write(pseudo->mii_info.phy_id, AUTO_NEGOTIATION_ADVERTISEMENT, mii_an_reg);

	// tx/rx pause
	mdio_cfg_reg = sysRegRead(MDIO_CFG);
	if(epause->tx_pause)
		mdio_cfg_reg |= MDIO_CFG_GP1_FC_TX;
	else
		mdio_cfg_reg &= ~MDIO_CFG_GP1_FC_TX;
	if(epause->rx_pause)
		mdio_cfg_reg |= MDIO_CFG_GP1_FC_RX;
	else
		mdio_cfg_reg &= ~MDIO_CFG_GP1_FC_RX;
	sysRegWrite(MDIO_CFG, mdio_cfg_reg);

	return 0;
}

static u32 et_virt_get_tx_csum(struct net_device *dev)
{
	return (sysRegRead(GDMA2_FWD_CFG) & GDM1_DISCRC) ? 0 : 1;	// a pitfall here, "0" means to enable.
}

static u32 et_virt_get_rx_csum(struct net_device *dev)
{
	return (sysRegRead(GDMA2_FWD_CFG) & GDM1_STRPCRC) ? 1 : 0;
}

static int et_virt_set_tx_csum(struct net_device *dev, u32 data)
{
	int value;
	//printk("et_set_tx_csum(): data = %d\n", data);
	value = sysRegRead(GDMA2_FWD_CFG);
	if(data)
		value |= GDM1_DISCRC;
	else
		value &= ~GDM1_DISCRC;
	sysRegWrite(GDMA1_FWD_CFG, value);
	return 0;
}

static int et_virt_set_rx_csum(struct net_device *dev, u32 data)
{
	int value;
	//printk("et_set_rx_csum(): data = %d\n", data);
	value = sysRegRead(GDMA2_FWD_CFG);
	if(data)
		value |= GDM1_STRPCRC;
	else
		value &= ~GDM1_STRPCRC;
	sysRegWrite(GDMA1_FWD_CFG, value);
	return 0;
}

static int et_virt_nway_reset(struct net_device *dev)
{
	PSEUDO_ADAPTER *pseudo  = netdev_priv(dev);
	return mii_nway_restart(&pseudo->mii_info);
}

static u32 et_virt_get_link(struct net_device *dev)
{
	PSEUDO_ADAPTER *pseudo  = netdev_priv(dev);
	return mii_link_ok(&pseudo->mii_info);
}

static int et_virt_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	PSEUDO_ADAPTER *pseudo = netdev_priv(dev);
	int rc = mii_ethtool_sset(&pseudo->mii_info, cmd);
	return rc;
}

static int et_virt_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	PSEUDO_ADAPTER *pseudo  = netdev_priv(dev);
	mii_ethtool_gset(&pseudo->mii_info, cmd);
	return 0;
}

static u32 et_virt_get_msglevel(struct net_device *dev)
{
	return 0;
}

static void et_virt_set_msglevel(struct net_device *dev, u32 datum)
{
	return;
}

static void et_virt_get_ethtool_stats(struct net_device *dev, struct ethtool_stats *stats, u64 *data)
{
//	PSEUDO_ADAPTER *pseudo = netdev_priv(dev);
	data[0] = 0;//np->xstats.early_rx;
	data[1] = 0;//np->xstats.tx_buf_mapped;
	data[2] = 0;//np->xstats.tx_timeouts;
	data[3] = 0;//np->xstats.rx_lost_in_ring;
}

/* for virtual interface dedicated */
#define RA_VIRT_NUM_STATS 			4
static struct {
    const char str[ETH_GSTRING_LEN];
} ethtool_stats_keys_2[] = {
    { "statistic1" },
    { "statistic2" },
    { "statistic3" },
    { "statistic4" },
};

static int et_virt_get_sset_count(struct net_device *dev, int string_set)
{
	switch (string_set) {
	case ETH_SS_STATS:
		return RA_VIRT_NUM_STATS;
	default:
		return -EOPNOTSUPP;
	}
}

static void et_virt_get_strings(struct net_device *dev, u32 stringset, u8 *data)
{
	memcpy(data, ethtool_stats_keys_2, sizeof(ethtool_stats_keys_2));
}

struct ethtool_ops ra_virt_ethtool_ops = {
	.get_drvinfo		= et_virt_get_drvinfo,
	.get_settings		= et_virt_get_settings,
	.set_settings		= et_virt_set_settings,
	.get_pauseparam		= et_virt_get_pauseparam,
	.set_pauseparam		= et_virt_set_pauseparam,
	.get_rx_csum		= et_virt_get_rx_csum,
	.set_rx_csum		= et_virt_set_rx_csum,
	.get_tx_csum		= et_virt_get_tx_csum,
	.set_tx_csum		= et_virt_set_tx_csum,
	.nway_reset		= et_virt_nway_reset,
	.get_link		= et_virt_get_link,
	.get_msglevel		= et_virt_get_msglevel,
	.set_msglevel		= et_virt_set_msglevel,
	.get_strings		= et_virt_get_strings,
	.get_sset_count		= et_virt_get_sset_count,
	.get_ethtool_stats	= et_virt_get_ethtool_stats,
/*	.get_regs_len		= et_virt_get_regs_len,
	.get_regs		= et_virt_get_regs,
*/
};

int mdio_virt_read(struct net_device *dev, int phy_id, int location)
{
	unsigned int result;
	PSEUDO_ADAPTER *pseudo = netdev_priv(dev);
	mii_mgr_read( (unsigned int) pseudo->mii_info.phy_id, (unsigned int)location, &result);
	RAETH_PRINT("%s mii.o query= phy_id:%d, address:%d retval:%d\n", dev->name, phy_id, location, result);
	return (int)result;
}

void mdio_virt_write(struct net_device *dev, int phy_id, int location, int value)
{
	PSEUDO_ADAPTER *pseudo = netdev_priv(dev);
	RAETH_PRINT("mii.o write= phy_id:%d, address:%d value:%d\n", phy_id, location, value);
	mii_mgr_write( (unsigned int) pseudo->mii_info.phy_id, (unsigned int)location, (unsigned int)value);
	return;
}

#endif /* CONFIG_PSEUDO_SUPPORT */


