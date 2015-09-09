#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/ethtool.h>

#include "raether.h"
#include "mii_mgr.h"
#include "ra_esw_reg.h"
#include "ra_esw_base.h"

#define RA_NUM_STATS		4

#define PHY_CR_REG		0x00
#define PHY_STAT_REG		0x01
#define PHY_AUTO_NEGO_REG	0x04
#define PHY_1000_REG		0x09

/* PHY_CR_REG = 0 */
#define PHY_Enable_Auto_Nego	0x1000
#define PHY_Restart_Auto_Nego	0x0200

/* PHY_STAT_REG = 1 */
#define PHY_Auto_Neco_Comp	0x0020
#define PHY_Link_Status		0x0004

/* PHY_AUTO_NEGO_REG = 4 */
#define PHY_Cap_10_Half		0x0020
#define PHY_Cap_10_Full		0x0040
#define PHY_Cap_100_Half	0x0080
#define PHY_Cap_100_Full	0x0100
#define PHY_Cap_Pause		0x0400

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
	u32 value;

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
	u32 value;

	value = sysRegRead(GDMA1_FWD_CFG);
	if(data)
		value |= GDM1_STRPCRC;
	else
		value &= ~GDM1_STRPCRC;

	sysRegWrite(GDMA1_FWD_CFG, value);
	return 0;
}
#endif

static void et_get_pauseparam(struct net_device *dev, struct ethtool_pauseparam *epause)
{
	u32 __maybe_unused mii_an_reg = 0, mdio_cfg_reg = 0;
	END_DEVICE *ei_local = netdev_priv(dev);

	// get mii auto-negotiation register
	mii_mgr_read(ei_local->mii_info.phy_id, PHY_AUTO_NEGO_REG, &mii_an_reg);
	epause->autoneg = (mii_an_reg & PHY_Cap_Pause) ? 1 : 0; //get autonet_enable flag bit

#if defined (CONFIG_RALINK_MT7621)
#if defined (CONFIG_MT7530_GSW)
	if (ei_local->mii_info.phy_id < 5)
		mdio_cfg_reg = esw_reg_get(REG_ESW_MAC_PMSR_P0 + 0x100 * ei_local->mii_info.phy_id);
#else
	mdio_cfg_reg = sysRegRead(REG_ETH_GE1_MAC_STATUS);
#endif
	epause->tx_pause = (mdio_cfg_reg & BIT(4)) ? 1 : 0;
	epause->rx_pause = (mdio_cfg_reg & BIT(5)) ? 1 : 0;
#elif defined (CONFIG_RALINK_MT7620)
#if defined (CONFIG_RAETH_ESW) || defined (CONFIG_MT7530_GSW)
	if (ei_local->mii_info.phy_id < 5)
		mdio_cfg_reg = esw_reg_get(REG_ESW_MAC_PMSR_P0 + 0x100 * ei_local->mii_info.phy_id);
#endif
	epause->tx_pause = (mdio_cfg_reg & BIT(4)) ? 1 : 0;
	epause->rx_pause = (mdio_cfg_reg & BIT(5)) ? 1 : 0;
#elif defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || \
      defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
	if (ei_local->mii_info.phy_id < 5)
		mdio_cfg_reg = ((esw_reg_get(REG_ESW_POA) >> 16) >> ei_local->mii_info.phy_id);
	epause->tx_pause = (mdio_cfg_reg & BIT(0)) ? 1 : 0;
	epause->rx_pause = (mdio_cfg_reg & BIT(0)) ? 1 : 0;
#else
	mdio_cfg_reg = sysRegRead(MDIO_CFG);
	epause->tx_pause = (mdio_cfg_reg & BIT(11)) ? 1 : 0;
	epause->rx_pause = (mdio_cfg_reg & BIT(10)) ? 1 : 0;
#endif
}

static int et_set_pauseparam(struct net_device *dev, struct ethtool_pauseparam *epause)
{
	u32 __maybe_unused mii_an_reg = 0, mdio_cfg_reg = 0;
	END_DEVICE *ei_local = netdev_priv(dev);

	// auto-neg pause
	mii_mgr_read(ei_local->mii_info.phy_id, PHY_AUTO_NEGO_REG, &mii_an_reg);
	if (epause->autoneg)
		mii_an_reg |= PHY_Cap_Pause;
	else
		mii_an_reg &= ~PHY_Cap_Pause;
	mii_mgr_write(ei_local->mii_info.phy_id, PHY_AUTO_NEGO_REG, mii_an_reg);

	// tx/rx pause
#if defined (CONFIG_RALINK_MT7621)
#if !defined (CONFIG_MT7530_GSW)
	mdio_cfg_reg = sysRegRead(REG_ETH_GE1_MAC_CONTROL);
	if (epause->tx_pause)
		mdio_cfg_reg |= BIT(4);
	else
		mdio_cfg_reg &= ~BIT(4);
	if (epause->rx_pause)
		mdio_cfg_reg |= BIT(5);
	else
		mdio_cfg_reg &= ~BIT(5);
	sysRegWrite(REG_ETH_GE1_MAC_CONTROL, mdio_cfg_reg);
#endif
#elif defined (CONFIG_RALINK_MT7620)
	// todo
#elif defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || \
      defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
	// todo
#else
	mdio_cfg_reg = sysRegRead(MDIO_CFG);
	if (epause->tx_pause)
		mdio_cfg_reg |= BIT(11);
	else
		mdio_cfg_reg &= ~BIT(11);
	if (epause->rx_pause)
		mdio_cfg_reg |= BIT(10);
	else
		mdio_cfg_reg &= ~BIT(10);
	sysRegWrite(MDIO_CFG, mdio_cfg_reg);
#endif

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
}

static void et_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
	//END_DEVICE *ei_local = netdev_priv(dev);
	strcpy(info->driver, RAETH_DEV_NAME);
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
	u32 result;
	END_DEVICE *ei_local = netdev_priv(dev);

	mii_mgr_read( (u32) ei_local->mii_info.phy_id, (u32)location, &result);
//	RAETH_PRINT("\n%s mii.o query= phy_id:%d, address:%d retval:%x\n", dev->name, phy_id, location, result);
	return (int)result;
}

/*
 *	mii_mgr_write wrapper for mii.o ethtool
 */
void mdio_write(struct net_device *dev, int phy_id, int location, int value)
{
	END_DEVICE *ei_local = netdev_priv(dev);
//	RAETH_PRINT("mii.o write= phy_id:%d, address:%d value:%x\n", phy_id, location, value);
	mii_mgr_write( (u32) ei_local->mii_info.phy_id, (u32)location, (u32)value);
}

struct ethtool_ops ra_ethtool_ops = {
	.get_drvinfo		= et_get_drvinfo,
	.get_settings		= et_get_settings,
	.set_settings		= et_set_settings,
	.get_pauseparam		= et_get_pauseparam,
	.set_pauseparam		= et_set_pauseparam,
#if 0
	.get_rx_csum		= et_get_rx_csum,
	.set_rx_csum		= et_set_rx_csum,
	.get_tx_csum		= et_get_tx_csum,
	.set_tx_csum		= et_set_tx_csum,
#endif
	.nway_reset		= et_nway_reset,
	.get_link		= et_get_link,
	.get_msglevel		= et_get_msglevel,
	.set_msglevel		= et_set_msglevel,
	.get_strings		= et_get_strings,
	.get_sset_count		= et_get_sset_count,
	.get_ethtool_stats	= et_get_ethtool_stats,
#if 0
	.get_regs_len		= et_get_regs_len,
	.get_regs		= et_get_regs,
#endif
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
	u32 __maybe_unused mii_an_reg = 0, mdio_cfg_reg = 0;
	PSEUDO_ADAPTER *pseudo = netdev_priv(dev);

	// get mii auto-negotiation register
	mii_mgr_read(pseudo->mii_info.phy_id, PHY_AUTO_NEGO_REG, &mii_an_reg);
	epause->autoneg = (mii_an_reg & PHY_Cap_Pause) ? 1 : 0; //get autonet_enable flag bit

#if defined (CONFIG_RALINK_MT7621)
	mdio_cfg_reg = sysRegRead(REG_ETH_GE2_MAC_STATUS);
	epause->tx_pause = (mdio_cfg_reg & BIT(4)) ? 1 : 0;
	epause->rx_pause = (mdio_cfg_reg & BIT(5)) ? 1 : 0;
#elif defined (CONFIG_RALINK_RT3883)
	mdio_cfg_reg = sysRegRead(MDIO_CFG2);
	epause->tx_pause = (mdio_cfg_reg & BIT(11)) ? 1 : 0;
	epause->rx_pause = (mdio_cfg_reg & BIT(10)) ? 1 : 0;
#endif
}

static int et_virt_set_pauseparam(struct net_device *dev, struct ethtool_pauseparam *epause)
{
	u32 __maybe_unused mii_an_reg = 0, mdio_cfg_reg = 0;
	PSEUDO_ADAPTER *pseudo = netdev_priv(dev);

	// auto-neg pause
	mii_mgr_read(pseudo->mii_info.phy_id, PHY_AUTO_NEGO_REG, &mii_an_reg);
	if (epause->autoneg)
		mii_an_reg |= PHY_Cap_Pause;
	else
		mii_an_reg &= ~PHY_Cap_Pause;
	mii_mgr_write(pseudo->mii_info.phy_id, PHY_AUTO_NEGO_REG, mii_an_reg);

	// tx/rx pause
#if defined (CONFIG_RALINK_MT7621)
#if !defined (CONFIG_GE2_INTERNAL_GMAC_P5)
	mdio_cfg_reg = sysRegRead(REG_ETH_GE2_MAC_CONTROL);
	if (epause->tx_pause)
		mdio_cfg_reg |= BIT(4);
	else
		mdio_cfg_reg &= ~BIT(4);
	if (epause->rx_pause)
		mdio_cfg_reg |= BIT(5);
	else
		mdio_cfg_reg &= ~BIT(5);
	sysRegWrite(REG_ETH_GE2_MAC_CONTROL, mdio_cfg_reg);
#endif
#elif defined (CONFIG_RALINK_RT3883)
	mdio_cfg_reg = sysRegRead(MDIO_CFG2);
	if (epause->tx_pause)
		mdio_cfg_reg |= BIT(11);
	else
		mdio_cfg_reg &= ~BIT(11);
	if (epause->rx_pause)
		mdio_cfg_reg |= BIT(10);
	else
		mdio_cfg_reg &= ~BIT(10);
	sysRegWrite(MDIO_CFG2, mdio_cfg_reg);
#endif

	return 0;
}

#if 0
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
	u32 value;

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
	u32 value;

	value = sysRegRead(GDMA2_FWD_CFG);
	if(data)
		value |= GDM1_STRPCRC;
	else
		value &= ~GDM1_STRPCRC;
	sysRegWrite(GDMA1_FWD_CFG, value);
	return 0;
}
#endif

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
#if 0
	.get_rx_csum		= et_virt_get_rx_csum,
	.set_rx_csum		= et_virt_set_rx_csum,
	.get_tx_csum		= et_virt_get_tx_csum,
	.set_tx_csum		= et_virt_set_tx_csum,
#endif
	.nway_reset		= et_virt_nway_reset,
	.get_link		= et_virt_get_link,
	.get_msglevel		= et_virt_get_msglevel,
	.set_msglevel		= et_virt_set_msglevel,
	.get_strings		= et_virt_get_strings,
	.get_sset_count		= et_virt_get_sset_count,
	.get_ethtool_stats	= et_virt_get_ethtool_stats,
#if 0
	.get_regs_len		= et_virt_get_regs_len,
	.get_regs		= et_virt_get_regs,
#endif
};

int mdio_virt_read(struct net_device *dev, int phy_id, int location)
{
	u32 result;
	PSEUDO_ADAPTER *pseudo = netdev_priv(dev);

	mii_mgr_read( (u32) pseudo->mii_info.phy_id, (u32)location, &result);
//	RAETH_PRINT("%s mii.o query= phy_id:%d, address:%d retval:%d\n", dev->name, phy_id, location, result);
	return (int)result;
}

void mdio_virt_write(struct net_device *dev, int phy_id, int location, int value)
{
	PSEUDO_ADAPTER *pseudo = netdev_priv(dev);

//	RAETH_PRINT("mii.o write= phy_id:%d, address:%d value:%d\n", phy_id, location, value);
	mii_mgr_write( (u32) pseudo->mii_info.phy_id, (u32)location, (u32)value);
}

#endif /* CONFIG_PSEUDO_SUPPORT */


