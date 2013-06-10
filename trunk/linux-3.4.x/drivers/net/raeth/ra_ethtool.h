#ifndef RA_ETHTOOL_H
#define RA_ETHTOOL_H

/* ethtool related */
unsigned char get_current_phy_address(void);
int mdio_read(struct net_device *dev, int phy_id, int location);
void mdio_write(struct net_device *dev, int phy_id, int location, int value);

/* for pseudo interface */
int mdio_virt_read(struct net_device *dev, int phy_id, int location);
void mdio_virt_write(struct net_device *dev, int phy_id, int location, int value);

#endif
