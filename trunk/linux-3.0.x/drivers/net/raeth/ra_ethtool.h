#ifndef __RA_ETHTOOL_H__
#define __RA_ETHTOOL_H__

/* ethtool related */
unsigned char get_current_phy_address(void);

extern struct ethtool_ops ra_ethtool_ops;
int mdio_read(struct net_device *dev, int phy_id, int location);
void mdio_write(struct net_device *dev, int phy_id, int location, int value);

#ifdef CONFIG_PSEUDO_SUPPORT
extern struct ethtool_ops ra_virt_ethtool_ops;
int mdio_virt_read(struct net_device *dev, int phy_id, int location);
void mdio_virt_write(struct net_device *dev, int phy_id, int location, int value);
#endif

#endif
