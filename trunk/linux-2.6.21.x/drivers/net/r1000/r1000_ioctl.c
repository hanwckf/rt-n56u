#include "r1000.h"

extern int R1000_READ_GMII_REG(unsigned long ioaddr, int RegAddr);
extern int R1000_WRITE_GMII_REG(unsigned long ioaddr, int RegAddr, int value);
extern int r1000_set_speed_duplex(unsigned long ioaddr, unsigned long anar, unsigned long gbcr, unsigned long bmcr);

static int ethtool_get_settings(struct net_device *netdev,struct ethtool_cmd *ecmd){
	struct r1000_private *priv = (struct r1000_private *)(netdev->priv);
	unsigned long ioaddr = priv->ioaddr;
	unsigned int bmcr = R1000_READ_GMII_REG(ioaddr,PHY_STAT_REG);
	unsigned int bmsr = R1000_READ_GMII_REG(ioaddr,PHY_AUTO_NEGO_REG);
	unsigned int gbcr = R1000_READ_GMII_REG(ioaddr,PHY_1000_CTRL_REG);

	ecmd->supported = (SUPPORTED_10baseT_Half|
		SUPPORTED_10baseT_Full|
		SUPPORTED_100baseT_Half|
		SUPPORTED_100baseT_Full|
		SUPPORTED_1000baseT_Full|
		SUPPORTED_Autoneg|
		SUPPORTED_TP);

	ecmd->advertising = ADVERTISED_TP;

	if(bmsr&ADVERTISE_10HALF)
		ecmd->advertising |= ADVERTISED_10baseT_Half;
	if(bmsr&ADVERTISE_10FULL)
		ecmd->advertising |= ADVERTISED_10baseT_Full;
	if(bmsr&ADVERTISE_100HALF)
		ecmd->advertising |= ADVERTISED_100baseT_Half;
	if(bmsr&ADVERTISE_100FULL)
		ecmd->advertising |= ADVERTISED_100baseT_Full;
	if(gbcr&PHY_Cap_1000_Full)
		ecmd->advertising |= ADVERTISED_1000baseT_Full;
	if(bmcr&PHY_Enable_Auto_Nego)
		ecmd->advertising |= ADVERTISED_Autoneg;

	ecmd->port = PORT_TP;

	if(priv->mcfg == MCFG_METHOD_1)
		ecmd->transceiver = XCVR_EXTERNAL;
	else
		ecmd->transceiver = XCVR_INTERNAL;

	if(RTL_R8(PHYstatus)&LinkStatus){
		if(RTL_R8(PHYstatus)&_1000Mbps)
			ecmd->speed = SPEED_1000;
		else if(RTL_R8(PHYstatus)&_100Mbps)
			ecmd->speed = SPEED_100;
		else if(RTL_R8(PHYstatus)&_10Mbps)
			ecmd->speed = SPEED_10;
		
		if(RTL_R8(PHYstatus)&FullDup)
			ecmd->duplex = DUPLEX_FULL;
		else
			ecmd->duplex = DUPLEX_HALF;
		
	}else{
		ecmd->speed = -1;
		ecmd->duplex = -1;
	}
	ecmd->autoneg = AUTONEG_ENABLE;
	return 0;
}

static int ethtool_set_settings(struct net_device *netdev,struct ethtool_cmd *ecmd){
	struct r1000_private *priv = (struct r1000_private *)(netdev->priv);
	unsigned long ioaddr = priv->ioaddr;
	unsigned int anar=0,gbcr=0,bmcr=0,ret=0,val=0;

	val = R1000_READ_GMII_REG( ioaddr, PHY_AUTO_NEGO_REG );
#ifdef R1000_HW_FLOW_CONTROL_SUPPORT
	val |= PHY_Cap_PAUSE | PHY_Cap_ASYM_PAUSE ;
#endif //end #define R1000_HW_FLOW_CONTROL_SUPPORT

	bmcr = PHY_Restart_Auto_Nego|PHY_Enable_Auto_Nego;

	if(ecmd->autoneg==AUTONEG_ENABLE){
		anar = PHY_Cap_10_Half|PHY_Cap_10_Full|PHY_Cap_100_Half|PHY_Cap_100_Full;
		gbcr = PHY_Cap_1000_Half|PHY_Cap_1000_Full;
	}else{
		if(ecmd->speed==SPEED_1000){
			anar = PHY_Cap_10_Half|PHY_Cap_10_Full|PHY_Cap_100_Half|PHY_Cap_100_Full;
			if((priv->mcfg==MCFG_METHOD_13)||(priv->mcfg==MCFG_METHOD_14)||(priv->mcfg==MCFG_METHOD_15))
				gbcr = PHY_Cap_Null;
			else
				gbcr = PHY_Cap_1000_Half|PHY_Cap_1000_Full;
		}else if((ecmd->speed==SPEED_100)&&(ecmd->duplex==DUPLEX_FULL)){
			anar = PHY_Cap_10_Half|PHY_Cap_10_Full|PHY_Cap_100_Half|PHY_Cap_100_Full;
			gbcr = PHY_Cap_Null;
		}else if((ecmd->speed==SPEED_100)&&(ecmd->duplex==DUPLEX_HALF)){
			anar = PHY_Cap_10_Half|PHY_Cap_10_Full|PHY_Cap_100_Half;
			gbcr = PHY_Cap_Null;
		}else if((ecmd->speed==SPEED_10)&&(ecmd->duplex==DUPLEX_FULL)){
			anar = PHY_Cap_10_Half|PHY_Cap_10_Full;
			gbcr = PHY_Cap_Null;
		}else if((ecmd->speed==SPEED_10)&&(ecmd->duplex==DUPLEX_HALF)){
			anar = PHY_Cap_10_Half;
			gbcr = PHY_Cap_Null;
		}else{
			anar = PHY_Cap_10_Half|PHY_Cap_10_Full|PHY_Cap_100_Half|PHY_Cap_100_Full;
			gbcr = PHY_Cap_Null;
		}
	}

	//enable flow control
	anar |=  val&0xC1F;

	ret = r1000_set_speed_duplex(ioaddr,anar,gbcr,bmcr);

	return ret;
}

int ethtool_ioctl(struct ifreq *ifr){
	struct net_device *netdev=__dev_get_by_name(ifr->ifr_name);
	void *useraddr=(void *)ifr->ifr_data;
	uint32_t ethcmd;

	if(!capable(CAP_NET_ADMIN))
		return -EPERM;

	if(!netdev || !netif_device_present(netdev))
		return -ENODEV;
	if(copy_from_user(&ethcmd, useraddr, sizeof(ethcmd)))
		return -EFAULT;
	
	switch (ethcmd){
	case ETHTOOL_GSET:
		return ethtool_get_settings(netdev,useraddr);
	case ETHTOOL_SSET:
		return ethtool_set_settings(netdev,useraddr);
	default:
		return -EOPNOTSUPP;
	}
	return -EOPNOTSUPP;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,5,0)
struct ethtool_ops r1000_ethtool_ops = {
	.get_settings	= ethtool_get_settings,
	.set_settings	= ethtool_set_settings,
};
#endif
