//=========================================================================
//Realtek Ethernet driver for Linux kernel 2.4.x. and 2.6.x
//=========================================================================

#include "r1000.h"
static int media[MAX_UNITS] = {-1, -1, -1, -1, -1, -1, -1, -1};

/* Maximum events (Rx packets, etc.) to handle at each interrupt. */
static int max_interrupt_work = 20;

/* Maximum number of multicast addresses to filter (vs. Rx-all-multicast).
   The RTL chips use a 64 element hash table based on the Ethernet CRC.  */
static int multicast_filter_limit = 32;

const static struct {
	const char *name;
	u8 mcfg;		 /* depend on documents of Realtek */
	u32 RxConfigMask; 	/* should clear the bits supported by this chip */
} rtl_chip_info[] = {
	{ "RTL8169",  MCFG_METHOD_1,  0xff7e1880 },
	{ "RTL8169S/8110S",  MCFG_METHOD_2,  0xff7e1880 },
	{ "RTL8169S/8110S",  MCFG_METHOD_3,  0xff7e1880 },
	{ "RTL8169SB/8110SB",  MCFG_METHOD_4,  0xff7e1880 },
	{ "RTL8169SC/8110SC",  MCFG_METHOD_5,  0xff7e1880 },
	{ "RTL8168B/8111B",  MCFG_METHOD_11,  0xff7e1880 },
	{ "RTL8168B/8111B",  MCFG_METHOD_12,  0xff7e1880 },
	{ "RTL8101E",  MCFG_METHOD_13,  0xff7e1880 },
	{ "RTL8100E",  MCFG_METHOD_14,  0xff7e1880 },
	{ "RTL8100E",  MCFG_METHOD_15,  0xff7e1880 },
	{ 0 }
};


static struct pci_device_id r1000_pci_tbl[] __devinitdata = {
	{ 0x10ec, 0x8169, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ 0x10ec, 0x8167, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ 0x10ec, 0x8168, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ 0x10ec, 0x8129, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ 0x10ec, 0x8136, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{0,}
};


MODULE_DEVICE_TABLE (pci, r1000_pci_tbl);
MODULE_AUTHOR ("Realtek");
MODULE_DESCRIPTION ("Linux device driver for Realtek Ethernet Controllers");
//MODULE_PARM (media, "1-" __MODULE_STRING(MAX_UNITS) "i");
MODULE_LICENSE("GPL");


static int r1000_open (struct net_device *netdev);
static int r1000_start_xmit (struct sk_buff *skb, struct net_device *netdev);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
//typedef	int				irqreturn_t;
#define	IRQ_NONE		0
#define	IRQ_HANDLED		1
static void r1000_interrupt (int irq, void *dev_instance, struct pt_regs *regs);
#else
static irqreturn_t r1000_interrupt (int irq, void *dev_instance, struct pt_regs *regs);
#endif

static void r1000_init_ring (struct net_device *netdev);
static void r1000_hw_start (struct net_device *netdev);
static int r1000_close (struct net_device *netdev);
static inline u32 ether_crc (int length, unsigned char *data);
static void r1000_set_rx_mode (struct net_device *netdev);
static void r1000_tx_timeout (struct net_device *netdev);
static struct net_device_stats *r1000_get_stats(struct net_device *netdev);

#ifdef R1000_JUMBO_FRAME_SUPPORT
static int r1000_change_mtu(struct net_device *netdev, int new_mtu);
#endif //end #ifdef R1000_JUMBO_FRAME_SUPPORT

static void r1000_hw_PHY_config (struct net_device *netdev);
static void r1000_hw_PHY_reset(struct net_device *netdev);
static const u16 r1000_intr_mask = LinkChg | RxOverflow | RxFIFOOver | TxErr | TxOK | RxErr | RxOK ;
static const unsigned int r1000_rx_config = (RX_FIFO_THRESH << RxCfgFIFOShift) | (RX_DMA_BURST << RxCfgDMAShift) | 0x0000000E;

static int r1000_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd);
extern int ethtool_ioctl(struct ifreq *ifr);
extern struct ethtool_ops r1000_ethtool_ops;
int r1000_set_speed_duplex(unsigned long ioaddr, unsigned int anar, unsigned int gbcr, unsigned int bmcr);

#ifdef R1000_DEBUG
unsigned alloc_rxskb_cnt = 0;
#define R1000_ALLOC_RXSKB(bufsize) 	dev_alloc_skb(bufsize); alloc_rxskb_cnt ++ ;
#define R1000_FREE_RXSKB(skb) 	kfree_skb(skb); alloc_rxskb_cnt -- ;
#define R1000_NETIF_RX(skb) 		netif_rx(skb); alloc_rxskb_cnt -- ;
#else
#define R1000_ALLOC_RXSKB(bufsize) 	dev_alloc_skb(bufsize);
#define R1000_FREE_RXSKB(skb) 	kfree_skb(skb);
#define R1000_NETIF_RX(skb) 		netif_rx(skb);
#endif //end #ifdef R1000_DEBUG


//=================================================================
//	PHYAR
//	bit		Symbol
//	31		Flag
//	30-21	reserved
//	20-16	5-bit GMII/MII register address
//	15-0	16-bit GMII/MII register data
//=================================================================
void R1000_WRITE_GMII_REG( unsigned long ioaddr, int RegAddr, int value )
{
	int	i;

	RTL_W32 ( PHYAR, 0x80000000 | (RegAddr&0xFF)<<16 | value);
	udelay(1000);

	for( i = 2000; i > 0 ; i -- ){
		// Check if the RTL8169 has completed writing to the specified MII register
		if( ! (RTL_R32(PHYAR)&0x80000000) ){
			break;
		}
		else{
			udelay(100);
		}// end of if( ! (RTL_R32(PHYAR)&0x80000000) )
	}// end of for() loop
}
//=================================================================
int R1000_READ_GMII_REG( unsigned long ioaddr, int RegAddr )
{
	int i, value = -1;

	RTL_W32 ( PHYAR, 0x0 | (RegAddr&0xFF)<<16 );
	udelay(1000);

	for( i = 2000; i > 0 ; i -- ){
		// Check if the RTL8169 has completed retrieving data from the specified MII register
		if( RTL_R32(PHYAR) & 0x80000000 ){
			value = (int)( RTL_R32(PHYAR)&0xFFFF );
			break;
		}
		else{
			udelay(100);
		}// end of if( RTL_R32(PHYAR) & 0x80000000 )
	}// end of for() loop
	return value;
}

void r1000_phy_timer_t_handler( void	*timer_data )
{
	struct net_device *netdev = (struct net_device *)timer_data;
	struct r1000_private *priv = (struct r1000_private *) (netdev->priv);
	unsigned long ioaddr = priv->ioaddr;

	assert( priv->mcfg > MCFG_METHOD_1 );
	assert( priv->pcfg < PCFG_METHOD_3 );

	if( RTL_R8(PHYstatus) & LinkStatus ){
		priv->phy_link_down_cnt = 0 ;
	}
	else{
		priv->phy_link_down_cnt ++ ;
		if( priv->phy_link_down_cnt >= 12 ){
			// If link on 1000, perform phy reset.
			if( R1000_READ_GMII_REG( ioaddr, PHY_1000_CTRL_REG ) & PHY_Cap_1000_Full )
			{
				DBG_PRINT("r1000_hw_PHY_reset\n");
				r1000_hw_PHY_reset(netdev);
			}

			priv->phy_link_down_cnt = 0 ;
		}
	}

	//---------------------------------------------------------------------------
	//mod_timer is a more efficient way to update the expire field of an active timer.
	//---------------------------------------------------------------------------
//	r1000_mod_timer( (&priv->phy_timer_t), 100 );
}

void r1000_timer_handler( void *timer_data )
{
	struct net_device *netdev = (struct net_device *)timer_data;
	struct r1000_private *priv = (struct r1000_private *) (netdev->priv);

	if( (priv->mcfg > MCFG_METHOD_1) && (priv->pcfg < PCFG_METHOD_3) ){
		DBG_PRINT("FIX PCS -> r1000_phy_timer_t_handler\n");
		priv->phy_link_down_cnt = 0;
		r1000_phy_timer_t_handler( timer_data );
	}


#ifdef R1000_DYNAMIC_CONTROL
	{
		struct r1000_cb_t *rt = &(priv->rt);
		if( priv->linkstatus == _1000_Full ){
			r1000_callback(rt);
		}
	}
#endif //end #ifdef R1000_DYNAMIC_CONTROL


	r1000_mod_timer( (&priv->r1000_timer), priv->expire_time );
}

int r1000_set_speed_duplex(unsigned long ioaddr, unsigned int anar, unsigned int gbcr, unsigned int bmcr){
	unsigned int i = 0;
	unsigned int bmsr;

	R1000_WRITE_GMII_REG(ioaddr,PHY_AUTO_NEGO_REG,anar);
	R1000_WRITE_GMII_REG(ioaddr,PHY_1000_CTRL_REG,gbcr);
	R1000_WRITE_GMII_REG(ioaddr,PHY_CTRL_REG,bmcr);

	for(i=0;i<10000;i++){
		bmsr = R1000_READ_GMII_REG(ioaddr,PHY_STAT_REG);
		if(bmsr&PHY_Auto_Neco_Comp)
			return 0;
	}
	return -1;	
}

static int __devinit r1000_init_board ( struct pci_dev *pdev, struct net_device **netdev_out, unsigned long *ioaddr_out)
{
	unsigned long ioaddr = 0;
	struct net_device *netdev;
	struct r1000_private *priv;
	int rc, i;
#ifndef R1000_USE_IO
	unsigned long mmio_start, mmio_end, mmio_flags, mmio_len;
#endif

	assert (pdev != NULL);
	assert (ioaddr_out != NULL);

	*ioaddr_out = 0;
	*netdev_out = NULL;

	// dev zeroed in init_etherdev
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0)
	netdev = init_etherdev (NULL, sizeof (*priv));
#else
	netdev = alloc_etherdev (sizeof (*priv));
#endif

	if (netdev == NULL) {
		printk (KERN_ERR PFX "unable to alloc new ethernet\n");
		return -ENOMEM;
	}

	SET_MODULE_OWNER(netdev);

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,5,0)
	SET_NETDEV_DEV(netdev, &pdev->dev);
#endif

	priv = netdev->priv;

	// enable device (incl. PCI PM wakeup and hotplug setup)
	rc = pci_enable_device (pdev);
	if (rc)
		goto err_out;

#ifndef R1000_USE_IO
	mmio_start = pci_resource_start (pdev, 1);
	mmio_end = pci_resource_end (pdev, 1);
	mmio_flags = pci_resource_flags (pdev, 1);
	mmio_len = pci_resource_len (pdev, 1);

	// make sure PCI base addr 1 is MMIO
	if (!(mmio_flags & IORESOURCE_MEM)) {
		printk (KERN_ERR PFX "region #1 not an MMIO resource, aborting\n");
		rc = -ENODEV;
		goto err_out;
	}

	// check for weird/broken PCI region reporting
	if ( mmio_len < RTL_MIN_IO_SIZE ) {
		printk (KERN_ERR PFX "Invalid PCI region size(s), aborting\n");
		rc = -ENODEV;
		goto err_out;
	}
#endif

	rc = pci_request_regions (pdev, netdev->name);
	if (rc)
		goto err_out;

	// enable PCI bus-mastering
	pci_set_master (pdev);

#ifdef R1000_USE_IO
	ioaddr = pci_resource_start(pdev, 0);
#else
	// ioremap MMIO region
	ioaddr = (unsigned long)ioremap (mmio_start, mmio_len);
	if (ioaddr == 0) {
		printk (KERN_ERR PFX "cannot remap MMIO, aborting\n");
		rc = -EIO;
		goto err_out_free_res;
	}
#endif

	// Soft reset the chip.
	RTL_W8 ( ChipCmd, CmdReset);

	// Check that the chip has finished the reset.
	for (i = 1000; i > 0; i--){
		if ( (RTL_R8(ChipCmd) & CmdReset) == 0){
			break;
		}
		else{
			udelay (10);
		}
	}

	// identify config method
	{
		unsigned long val32 = (RTL_R32(TxConfig)&0x7c800000);

		if( val32 == 0x38800000)
			priv->mcfg = MCFG_METHOD_15;
		else if( val32 == 0x30800000)
			priv->mcfg = MCFG_METHOD_14;
		else if( val32 == 0x34000000)
			priv->mcfg = MCFG_METHOD_13;
		else if( val32 == 0x38000000)
			priv->mcfg = MCFG_METHOD_12;
		else if( val32 == 0x30000000)
			priv->mcfg = MCFG_METHOD_11;
		else if( val32 == 0x18000000)
			priv->mcfg = MCFG_METHOD_5;
		else if( val32 == 0x10000000 )
			priv->mcfg = MCFG_METHOD_4;
		else if( val32 == 0x04000000 )
			priv->mcfg = MCFG_METHOD_3;
		else if( val32 == 0x00800000 )
			priv->mcfg = MCFG_METHOD_2;
		else if( val32 == 0x00000000 )
			priv->mcfg = MCFG_METHOD_1;
		else
			priv->mcfg = MCFG_METHOD_1;
	}
	{
		unsigned char val8 = (unsigned char)(R1000_READ_GMII_REG(ioaddr,3)&0x000f);
		if( val8 == 0x00 ){
			priv->pcfg = PCFG_METHOD_1;
		}
		else if( val8 == 0x01 ){
			priv->pcfg = PCFG_METHOD_2;
		}
		else if( val8 == 0x02 ){
			priv->pcfg = PCFG_METHOD_3;
		}
		else{
			priv->pcfg = PCFG_METHOD_3;
		}
	}


	for (i = ARRAY_SIZE (rtl_chip_info) - 1; i >= 0; i--){
		if (priv->mcfg == rtl_chip_info[i].mcfg) {
			priv->chipset = i;
			goto match;
		}
	}

	//if unknown chip, assume array element #0, original RTL-8169 in this case
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
	printk (KERN_DEBUG PFX "PCI device %s: unknown chip version, assuming RTL-8169\n", pdev->slot_name);
#endif
	priv->chipset = 0;

match:
	*ioaddr_out = ioaddr;
	*netdev_out = netdev;
	return 0;

#ifndef R1000_USE_IO
err_out_free_res:
	pci_release_regions (pdev);
#endif

err_out:
	unregister_netdev(netdev);
	kfree(netdev);
	return rc;
}

static int r1000_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd){

	if(!netif_running(netdev))
		return -EINVAL;

	switch(cmd){
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	case SIOCETHTOOL:
		return ethtool_ioctl(ifr);
#endif
	default:
		return -EOPNOTSUPP;
	}
}





//======================================================================================================
static int __devinit r1000_init_one (struct pci_dev *pdev, const struct pci_device_id *ent)
{
	struct net_device *netdev = NULL;
	struct r1000_private *priv = NULL;
	unsigned long ioaddr = 0;
	static int board_idx = -1;
	int i,rc;
	int option = -1, Cap10_100 = 0, Cap1000 = 0;
	int val=0,bmcr=0;


	assert (pdev != NULL);
	assert (ent != NULL);

	board_idx++;


	i = r1000_init_board (pdev, &netdev, &ioaddr);
	if (i < 0) {
		return i;
	}

	priv = netdev->priv;

	assert (ioaddr != NULL);
	assert (netdev != NULL);
	assert (priv != NULL);

	// Get MAC address //
	for (i = 0; i < MAC_ADDR_LEN ; i++){
		netdev->dev_addr[i] = RTL_R8( MAC0 + i );
	}

	netdev->open		= r1000_open;
	netdev->hard_start_xmit	= r1000_start_xmit;
	netdev->get_stats    	= r1000_get_stats;
	netdev->stop 		= r1000_close;
	netdev->tx_timeout 	= r1000_tx_timeout;
	netdev->set_multicast_list = r1000_set_rx_mode;
	netdev->watchdog_timeo 	= TX_TIMEOUT;
	netdev->irq 		= pdev->irq;
	netdev->base_addr 	= (unsigned long) ioaddr;

#ifdef R1000_JUMBO_FRAME_SUPPORT
	netdev->change_mtu	= r1000_change_mtu;
#endif //end #ifdef R1000_JUMBO_FRAME_SUPPORT

	netdev->do_ioctl 	= r1000_ioctl;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,5,0)
	netdev->ethtool_ops	= &r1000_ethtool_ops;
#endif

#ifdef R1000_DYNAMIC_CONTROL
	priv->rt.dev = netdev;
#endif //end #ifdef R1000_DYNAMIC_CONTROL

	priv = netdev->priv;				// private data //
	priv->pci_dev 	= pdev;
	priv->ioaddr 	= ioaddr;

//#ifdef R1000_JUMBO_FRAME_SUPPORT
	priv->curr_mtu_size = netdev->mtu;
	priv->tx_pkt_len = netdev->mtu + ETH_HDR_LEN;
	priv->rx_pkt_len = netdev->mtu + ETH_HDR_LEN;
	priv->hw_rx_pkt_len = priv->rx_pkt_len + 8;
//#endif //end #ifdef R1000_JUMBO_FRAME_SUPPORT

	DBG_PRINT("-------------------------- \n");
	DBG_PRINT("netdev->mtu = %d \n", netdev->mtu);
	DBG_PRINT("priv->curr_mtu_size = %d \n", priv->curr_mtu_size);
	DBG_PRINT("priv->tx_pkt_len = %d \n", priv->tx_pkt_len);
	DBG_PRINT("priv->rx_pkt_len = %d \n", priv->rx_pkt_len);
	DBG_PRINT("priv->hw_rx_pkt_len = %d \n", priv->hw_rx_pkt_len);
	DBG_PRINT("-------------------------- \n");

	spin_lock_init (&priv->lock);

	rc = register_netdev(netdev);
	if(rc){
#ifndef R1000_USE_IO
		iounmap ((void *)(netdev->base_addr));
#endif
		pci_release_regions(pdev);
		pci_disable_device(pdev);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
		kfree(netdev);
#else
		free_netdev(netdev);
#endif
		return rc;
	}

	pci_set_drvdata(pdev,netdev);     //      pdev->driver_data = data;


	printk (KERN_DEBUG "%s: Identified chip type is '%s'.\n",netdev->name,rtl_chip_info[priv->chipset].name);
	printk (KERN_INFO "%s: %s at 0x%lx, "
				"%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x, "
				"IRQ %d\n",
				netdev->name,
				R1000_DRIVER_NAME,
				netdev->base_addr,
				netdev->dev_addr[0],netdev->dev_addr[1],
				netdev->dev_addr[2],netdev->dev_addr[3],
				netdev->dev_addr[4],netdev->dev_addr[5],
				netdev->irq);


	// Config PHY
	r1000_hw_PHY_config(netdev);

	DBG_PRINT("Set MAC Reg C+CR Offset 0x82h = 0x01h\n");
	RTL_W8( 0x82, 0x01 );

	if( priv->mcfg < MCFG_METHOD_3 ){
		DBG_PRINT("Set PCI Latency=0x40\n");
		pci_write_config_byte(pdev, PCI_LATENCY_TIMER, 0x40);
	}

	if( priv->mcfg == MCFG_METHOD_2 ){
		DBG_PRINT("Set MAC Reg C+CR Offset 0x82h = 0x01h\n");
		RTL_W8( 0x82, 0x01 );
		DBG_PRINT("Set PHY Reg 0x0bh = 0x00h\n");
		R1000_WRITE_GMII_REG( ioaddr, 0x0b, 0x0000 );	//w 0x0b 15 0 0
	}

	// if TBI is not endbled
	if( !(RTL_R8(PHYstatus) & TBI_Enable) ){
		val = R1000_READ_GMII_REG( ioaddr, PHY_AUTO_NEGO_REG );

#ifdef R1000_HW_FLOW_CONTROL_SUPPORT
		val |= PHY_Cap_PAUSE | PHY_Cap_ASYM_PAUSE ;
#endif //end #define R1000_HW_FLOW_CONTROL_SUPPORT

		option = (board_idx >= MAX_UNITS) ? 0 : media[board_idx];
		// Force Realtek Ethernet Controller in 10/100/1000Mpbs Full/Half-duplex mode.
		if( option > 0 ){
			printk(KERN_INFO "%s: Force-mode Enabled. \n",netdev->name);
			Cap10_100 = 0;
			Cap1000 = 0;
			switch( option ){
				case _10_Half:
					Cap10_100 = PHY_Cap_10_Half;
					Cap1000 = PHY_Cap_Null;
					break;
				case _10_Full:
					Cap10_100 = PHY_Cap_10_Full | PHY_Cap_10_Half;
					Cap1000 = PHY_Cap_Null;
					break;
				case _100_Half:
					if(priv->mcfg!=MCFG_METHOD_13)
						Cap10_100 = PHY_Cap_100_Half | PHY_Cap_10_Full | PHY_Cap_10_Half;
					else
						Cap10_100 = 0x0081;
						Cap1000 = PHY_Cap_Null;
						break;
				case _100_Full:
					Cap10_100 = PHY_Cap_100_Full | PHY_Cap_100_Half | PHY_Cap_10_Full | PHY_Cap_10_Half;
					Cap1000 = PHY_Cap_Null;
					break;
				case _1000_Full:
						Cap10_100 = PHY_Cap_100_Full | PHY_Cap_100_Half | PHY_Cap_10_Full | PHY_Cap_10_Half;
					if((priv->mcfg==MCFG_METHOD_13)||(priv->mcfg==MCFG_METHOD_14)||(priv->mcfg==MCFG_METHOD_15))
						printk("This Realtek NIC doesn't support 1000Mbps\n");
					else
						Cap1000 = PHY_Cap_1000_Full|PHY_Cap_1000_Half;
					break;
				default:
					Cap10_100 = PHY_Cap_100_Full | PHY_Cap_100_Half | PHY_Cap_10_Full | PHY_Cap_10_Half;
					Cap1000 = PHY_Cap_Null;
					break;
			}
			//flow control enable
			Cap10_100 |=  val&0xC1F;
		}else{
			printk(KERN_INFO "%s: Auto-negotiation Enabled.\n",netdev->name);

			// enable 10/100 Full/Half Mode, leave PHY_AUTO_NEGO_REG bit4:0 unchanged
			Cap10_100 = PHY_Cap_10_Half | PHY_Cap_10_Full | PHY_Cap_100_Half | PHY_Cap_100_Full | ( val&0xC1F );

			// enable 1000 Full Mode
			if((priv->mcfg!=MCFG_METHOD_13)&&(priv->mcfg!=MCFG_METHOD_14)&&(priv->mcfg!=MCFG_METHOD_15))
				Cap1000 = PHY_Cap_1000_Full | PHY_Cap_1000_Half;
		}// end of if( option > 0 )

		bmcr = PHY_Enable_Auto_Nego | PHY_Restart_Auto_Nego;
		r1000_set_speed_duplex(ioaddr,Cap10_100,Cap1000,bmcr);

		option = RTL_R8(PHYstatus);
		if( option & _1000Mbps ){
			priv->linkstatus = _1000_Full;
		}else{
			if(option & _100Mbps){
				priv->linkstatus = (option & FullDup) ? _100_Full : _100_Half;
			}else{
				priv->linkstatus = (option & FullDup) ? _10_Full : _10_Half;
			}
		}
			DBG_PRINT("priv->linkstatus = 0x%02x\n", priv->linkstatus);
	}// end of TBI is not enabled
	else{
		udelay(100);
		DBG_PRINT("1000Mbps Full-duplex operation, TBI Link %s!\n",(RTL_R32(TBICSR) & TBILinkOK) ? "OK" : "Failed" );
	}// end of TBI is not enabled

	//show some information after the driver is inserted
	if(( priv->mcfg == MCFG_METHOD_11 )||( priv->mcfg == MCFG_METHOD_12 ))
		printk("Realtek RTL8168/8111 Family PCI-E Gigabit Ethernet Network Adapter\n");
	else if((priv->mcfg==MCFG_METHOD_13)||(priv->mcfg==MCFG_METHOD_14)||(priv->mcfg==MCFG_METHOD_15))
		printk("Realtek RTL8139/810x Family Fast Ethernet Network Adapter\n");
	else
		printk("Realtek RTL8169/8110 Family Gigabit Ethernet Network Adapter\n");

	printk("Driver version:%s\n",R1000_VERSION);
	printk("Released date:%s\n",RELEASE_DATE);
	if(RTL_R8(PHYstatus) & LinkStatus){
		printk("Link Status:%s\n","Linked");

		if(RTL_R8(PHYstatus) & _1000Mbps)
			printk("Link Speed:1000Mbps\n");
		else if(RTL_R8(PHYstatus) & _100Mbps)
			printk("Link Speed:100Mbps\n");
		else if(RTL_R8(PHYstatus) & _10Mbps)
			printk("Link Speed:10Mbps\n");

		printk("Duplex mode:%s\n",RTL_R8(PHYstatus)&FullDup?"Full-Duplex":"Half-Duplex");
	}else{
		printk("Link Status:%s\n","Not Linked");
	}
#ifdef R1000_USE_IO
	printk("I/O Base:0x%X(I/O port)\n",(unsigned int)(priv->ioaddr));
#else
	printk("I/O Base:0x%X(I/O memory)\n",(unsigned int)(priv->ioaddr));
#endif	//R1000_USE_IO
	printk("IRQ:%d\n",netdev->irq);

	return 0;
}

static void __devexit r1000_remove_one (struct pci_dev *pdev)
{
	struct net_device *netdev = pci_get_drvdata(pdev);

	assert (netdev != NULL);
	assert (priv != NULL);

	unregister_netdev(netdev);

#ifndef R1000_USE_IO
	iounmap ((void *)(netdev->base_addr));
#endif
	pci_release_regions (pdev);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	kfree(netdev);
#else
	free_netdev(netdev);
#endif

	pci_set_drvdata (pdev, NULL);
}

static int r1000_open (struct net_device *netdev)
{
	struct r1000_private *priv = netdev->priv;
	struct pci_dev *pdev = priv->pci_dev;
	int retval;
//	u8 diff;
//	u32 TxPhyAddr, RxPhyAddr;


	if( priv->drvinit_fail == 1 ){
		printk("%s: Gigabit driver open failed.\n", netdev->name );
		return -ENOMEM;
	}

	retval = request_irq (netdev->irq, r1000_interrupt, SA_SHIRQ, netdev->name, netdev);
	if (retval) {
		return retval;
	}

	//2004-05-11
	// Allocate tx/rx descriptor space
	priv->sizeof_txdesc_space = NUM_TX_DESC * sizeof(struct TxDesc)+256;
	priv->txdesc_space = pci_alloc_consistent( pdev, priv->sizeof_txdesc_space, &priv->txdesc_phy_dma_addr );
	if( priv->txdesc_space == NULL ){
		printk("%s: Gigabit driver alloc txdesc_space failed.\n", netdev->name );
		return -ENOMEM;
	}
	priv->sizeof_rxdesc_space = NUM_RX_DESC * sizeof(struct RxDesc)+256;
	priv->rxdesc_space = pci_alloc_consistent( pdev, priv->sizeof_rxdesc_space, &priv->rxdesc_phy_dma_addr );
	if( priv->rxdesc_space == NULL ){
		printk("%s: Gigabit driver alloc rxdesc_space failed.\n", netdev->name );
		return -ENOMEM;
	}

	if(priv->txdesc_phy_dma_addr & 0xff){
		printk("%s: Gigabit driver txdesc_phy_dma_addr is not 256-bytes-aligned.\n", netdev->name );
	}
	if(priv->rxdesc_phy_dma_addr & 0xff){
		printk("%s: Gigabit driver rxdesc_phy_dma_addr is not 256-bytes-aligned.\n", netdev->name );
	}
	// Set tx/rx descriptor space
	priv->TxDescArray = (struct TxDesc *)priv->txdesc_space;
	priv->RxDescArray = (struct RxDesc *)priv->rxdesc_space;

	{
		int i;
		struct sk_buff *skb = NULL;

		for(i=0;i<NUM_RX_DESC;i++){
			skb = R1000_ALLOC_RXSKB(MAX_RX_SKBDATA_SIZE);
			if( skb != NULL ) {
				skb_reserve (skb, 8);	// 16 byte align the IP fields. //
				priv->Rx_skbuff[i] = skb;
			}
			else{
				printk("%s: Gigabit driver failed to allocate skbuff.\n", netdev->name);
				priv->drvinit_fail = 1;
			}
		}
	}


	//////////////////////////////////////////////////////////////////////////////
	r1000_init_ring(netdev);
	r1000_hw_start(netdev);


	// ------------------------------------------------------
	DBG_PRINT("FIX PCS -> r1000_request_timer\n");
	priv->expire_time = R1000_TIMER_EXPIRE_TIME;
	r1000_request_timer( (&priv->r1000_timer), priv->expire_time, r1000_timer_handler, ((void *)netdev) );  //in open()


	DBG_PRINT("%s: %s() alloc_rxskb_cnt = %d\n", netdev->name, __FUNCTION__, alloc_rxskb_cnt );

	return 0;

}//end of r1000_open (struct net_device *netdev)

static void r1000_hw_PHY_reset(struct net_device *netdev)
{
	int val, phy_reset_expiretime = 50;
	struct r1000_private *priv = netdev->priv;
	unsigned long ioaddr = priv->ioaddr;

	DBG_PRINT("%s: Reset RTL8169s PHY\n", netdev->name);

	val = ( R1000_READ_GMII_REG( ioaddr, 0 ) | 0x8000 ) & 0xffff;
	R1000_WRITE_GMII_REG( ioaddr, 0, val );

	do //waiting for phy reset
	{
		if( R1000_READ_GMII_REG( ioaddr, 0 ) & 0x8000 ){
			phy_reset_expiretime --;
			udelay(100);
		}
		else{
			break;
		}
	}while( phy_reset_expiretime >= 0 );

	assert( phy_reset_expiretime > 0 );
}

static void r1000_hw_PHY_config (struct net_device *netdev)
{
	struct r1000_private *priv = netdev->priv;
	void *ioaddr = (void*)priv->ioaddr;

	DBG_PRINT("priv->mcfg=%d, priv->pcfg=%d\n",priv->mcfg,priv->pcfg);

	if( priv->mcfg == MCFG_METHOD_4 ){
#if 0
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x1F, 0x0001 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x1b, 0x841e );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x0e, 0x7bfb );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x09, 0x273a );
#endif

		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x1F, 0x0002 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x01, 0x90D0 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x1F, 0x0000 );
	}else if((priv->mcfg == MCFG_METHOD_2)||(priv->mcfg == MCFG_METHOD_3)){
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x1f, 0x0001 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x06, 0x006e );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x08, 0x0708 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x15, 0x4000 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x18, 0x65c7 );

		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x1f, 0x0001 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x03, 0x00a1 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x02, 0x0008 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x01, 0x0120 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x00, 0x1000 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0x0800 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0x0000 );

		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x03, 0xff41 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x02, 0xdf60 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x01, 0x0140 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x00, 0x0077 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0x7800 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0x7000 );

		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x03, 0x802f );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x02, 0x4f02 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x01, 0x0409 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x00, 0xf0f9 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0x9800 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0x9000 );

		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x03, 0xdf01 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x02, 0xdf20 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x01, 0xff95 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x00, 0xba00 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0xa800 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0xa000 );

		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x03, 0xff41 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x02, 0xdf20 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x01, 0x0140 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x00, 0x00bb );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0xb800 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0xb000 );

		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x03, 0xdf41 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x02, 0xdc60 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x01, 0x6340 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x00, 0x007d );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0xd800 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0xd000 );

		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x03, 0xdf01 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x02, 0xdf20 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x01, 0x100a );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x00, 0xa0ff );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0xf800 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0xf000 );

		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x1f, 0x0000 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x0b, 0x0000 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x00, 0x9200 );
#if 0
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x1F, 0x0001 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x15, 0x1000 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x18, 0x65C7 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0x0000 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x03, 0x00A1 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x02, 0x0008 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x01, 0x1020 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x00, 0x1000 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0x0800 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0x0000 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0x7000 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x03, 0xFF41 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x02, 0xDE60 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x01, 0x0140 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x00, 0x0077 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0x7800 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0x7000 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0xA000 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x03, 0xDF01 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x02, 0xDF20 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x01, 0xFF95 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x00, 0xFA00 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0xA800 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0xA000 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0xB000 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x03, 0xFF41 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x02, 0xDE20 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x01, 0x0140 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x00, 0x00BB );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0xB800 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0xB000 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0xF000 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x03, 0xDF01 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x02, 0xDF20 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x01, 0xFF95 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x00, 0xBF00 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0xF800 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0xF000 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x04, 0x0000 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x1F, 0x0000 );
		R1000_WRITE_GMII_REG( (unsigned long)ioaddr, 0x0B, 0x0000 );
#endif
	}
	else{
		DBG_PRINT("priv->mcfg=%d. Discard hw PHY config.\n",priv->mcfg);
	}
}

static void r1000_hw_start (struct net_device *netdev)
{
	struct r1000_private *priv = netdev->priv;
	struct pci_dev *pdev = priv->pci_dev;
	unsigned long ioaddr = priv->ioaddr;
	u32 i;
	u8 i8;
	u16 i16;

	if((priv->mcfg!=MCFG_METHOD_5)&&(priv->mcfg!=MCFG_METHOD_11)&&
	   (priv->mcfg!=MCFG_METHOD_12)&&(priv->mcfg!=MCFG_METHOD_13)&&
	   (priv->mcfg!=MCFG_METHOD_14)&&(priv->mcfg!=MCFG_METHOD_15)){
		/* Soft reset the chip. */
		RTL_W8 ( ChipCmd, CmdReset);

		/* Check that the chip has finished the reset. */
		for (i = 1000; i > 0; i--){
			if ((RTL_R8( ChipCmd ) & CmdReset) == 0) break;
			else udelay (10);
		}

		RTL_W8 ( Cfg9346, Cfg9346_Unlock);
		RTL_W8 ( ChipCmd, CmdTxEnb | CmdRxEnb);
		RTL_W8 ( ETThReg, ETTh);

		// For gigabit rtl8169
		RTL_W16	( RxMaxSize, (unsigned short)priv->hw_rx_pkt_len );

		// Set Rx Config register
		i = r1000_rx_config | ( RTL_R32( RxConfig ) & rtl_chip_info[priv->chipset].RxConfigMask);
		RTL_W32 ( RxConfig, i);


		/* Set DMA burst size and Interframe Gap Time */
		RTL_W32 ( TxConfig, (TX_DMA_BURST << TxDMAShift) | (InterFrameGap << TxInterFrameGapShift) );



		RTL_W16( CPlusCmd, RTL_R16(CPlusCmd) );

		if(priv->mcfg==MCFG_METHOD_2||priv->mcfg==MCFG_METHOD_3){
			RTL_W16( CPlusCmd, (RTL_R16(CPlusCmd)|(1<<14)|(1<<3)) );
			DBG_PRINT("Set MAC Reg C+CR Offset 0xE0: bit-3 and bit-14\n");
		}else{
			RTL_W16( CPlusCmd, (RTL_R16(CPlusCmd)|(1<<3)) );
			DBG_PRINT("Set MAC Reg C+CR Offset 0xE0: bit-3.\n");
		}

		{
			RTL_W16(0xE2,0x0000);
		}

		priv->cur_rx = 0;

		RTL_W32 ( TxDescStartAddr, priv->txdesc_phy_dma_addr);
		RTL_W32 ( TxDescStartAddr + 4, 0x00);
		RTL_W32 ( RxDescStartAddr, priv->rxdesc_phy_dma_addr);
		RTL_W32 ( RxDescStartAddr + 4, 0x00);

		RTL_W8 ( Cfg9346, Cfg9346_Lock );
		udelay (10);

		RTL_W32 ( RxMissed, 0 );

		r1000_set_rx_mode(netdev);

		RTL_W16 ( MultiIntr, RTL_R16(MultiIntr) & 0xF000);

		RTL_W16 ( IntrMask, r1000_intr_mask);
	}else{
		/* Soft reset the chip. */
		RTL_W8 ( ChipCmd, CmdReset);

		/* Check that the chip has finished the reset. */
		for (i = 1000; i > 0; i--){
			if ((RTL_R8( ChipCmd ) & CmdReset) == 0) break;
			else udelay (10);
		}

		if( priv->mcfg == MCFG_METHOD_13 ){
			pci_write_config_word(pdev,0x68,0x00);
			pci_write_config_word(pdev,0x69,0x08);
		}

		if( priv->mcfg == MCFG_METHOD_5 ){
			i8=RTL_R8(Config2);
			i8=i8&0x07;
			if(i8&&0x01)
				RTL_W32(Off7Ch,0x0007FFFF);
	
			i=0x0007FF00;
			RTL_W32(Off7Ch, i);

			pci_read_config_word(pdev,0x04,&i16);
			i16=i16&0xEF;
			pci_write_config_word(pdev,0x04,i16);
		}

		RTL_W8 ( Cfg9346, Cfg9346_Unlock);
		RTL_W8 ( ETThReg, ETTh);

		// For gigabit rtl8169
		RTL_W16	( RxMaxSize, (unsigned short)priv->hw_rx_pkt_len );

		RTL_W16( CPlusCmd, RTL_R16(CPlusCmd) );

		if(priv->mcfg==MCFG_METHOD_2||priv->mcfg==MCFG_METHOD_3){
			RTL_W16( CPlusCmd, (RTL_R16(CPlusCmd)|(1<<14)|(1<<3)) );
			DBG_PRINT("Set MAC Reg C+CR Offset 0xE0: bit-3 and bit-14\n");
		}else{
			RTL_W16( CPlusCmd, (RTL_R16(CPlusCmd)|(1<<3)) );
			DBG_PRINT("Set MAC Reg C+CR Offset 0xE0: bit-3.\n");
		}

		{
			RTL_W16(0xE2,0x0000);
		}

		priv->cur_rx = 0;

		RTL_W32 ( TxDescStartAddr, priv->txdesc_phy_dma_addr);
		RTL_W32 ( TxDescStartAddr + 4, 0x00);
		RTL_W32 ( RxDescStartAddr, priv->rxdesc_phy_dma_addr);
		RTL_W32 ( RxDescStartAddr + 4, 0x00);
		RTL_W8 ( ChipCmd, CmdTxEnb | CmdRxEnb);
		// Set Rx Config register
		i = r1000_rx_config | ( RTL_R32( RxConfig ) & rtl_chip_info[priv->chipset].RxConfigMask);
		RTL_W32 ( RxConfig, i);

		/* Set DMA burst size and Interframe Gap Time */
		RTL_W32 ( TxConfig, (TX_DMA_BURST << TxDMAShift) | (InterFrameGap << TxInterFrameGapShift) );

		RTL_W8 ( Cfg9346, Cfg9346_Lock );
		udelay (10);

		RTL_W32 ( RxMissed, 0 );

		r1000_set_rx_mode(netdev);

		RTL_W16 ( MultiIntr, RTL_R16(MultiIntr) & 0xF000);

		RTL_W16 ( IntrMask, r1000_intr_mask);
	}

	netif_start_queue(netdev);

}//end of r1000_hw_start (struct net_device *netdev)

static void r1000_init_ring (struct net_device *netdev)
{
	struct r1000_private *priv = netdev->priv;
	struct pci_dev *pdev = priv->pci_dev;
	int i;
	struct sk_buff	*skb;
	

	priv->cur_rx = 0;
	priv->cur_tx = 0;
	priv->dirty_tx = 0;
	memset(priv->TxDescArray, 0x0, NUM_TX_DESC*sizeof(struct TxDesc));
	memset(priv->RxDescArray, 0x0, NUM_RX_DESC*sizeof(struct RxDesc));


	for (i=0 ; i<NUM_TX_DESC ; i++){
		priv->Tx_skbuff[i]=NULL;
		priv->txdesc_array_dma_addr[i] = pci_map_single(pdev, &priv->TxDescArray[i], sizeof(struct TxDesc), PCI_DMA_TODEVICE);
	}

	for (i=0; i <NUM_RX_DESC; i++) {
		if(i==(NUM_RX_DESC-1)){
			priv->RxDescArray[i].status = cpu_to_le32((OWNbit | EORbit) | (unsigned long)priv->hw_rx_pkt_len);
		}
		else{
			priv->RxDescArray[i].status = cpu_to_le32(OWNbit | (unsigned long)priv->hw_rx_pkt_len);
		}

		{//-----------------------------------------------------------------------
			skb = priv->Rx_skbuff[i];
			priv->rx_skbuff_dma_addr[i] = pci_map_single(pdev, skb->data, MAX_RX_SKBDATA_SIZE, PCI_DMA_FROMDEVICE);

			if( skb != NULL ){
				priv->RxDescArray[i].buf_addr = cpu_to_le32(priv->rx_skbuff_dma_addr[i]);
				priv->RxDescArray[i].buf_Haddr = 0;
			}
			else{
				DBG_PRINT("%s: %s() Rx_skbuff == NULL\n", netdev->name, __FUNCTION__);
				priv->drvinit_fail = 1;
			}
		}//-----------------------------------------------------------------------
		priv->rxdesc_array_dma_addr[i] = pci_map_single(pdev, &priv->RxDescArray[i], sizeof(struct RxDesc), PCI_DMA_TODEVICE);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
		pci_dma_sync_single(pdev, priv->rxdesc_array_dma_addr[i], sizeof(struct RxDesc), PCI_DMA_TODEVICE);
#endif
	}
}

static void r1000_tx_clear (struct r1000_private *priv)
{
	int i;

	priv->cur_tx = 0;
	for ( i = 0 ; i < NUM_TX_DESC ; i++ ){
		if ( priv->Tx_skbuff[i] != NULL ) {
			dev_kfree_skb ( priv->Tx_skbuff[i] );
			priv->Tx_skbuff[i] = NULL;
			priv->stats.tx_dropped++;
		}
	}
}

static void r1000_tx_timeout (struct net_device *netdev)
{
	struct r1000_private *priv = netdev->priv;
	unsigned long ioaddr = priv->ioaddr;
	u8 tmp8;

	/* disable Tx, if not already */
	tmp8 = RTL_R8( ChipCmd );
	if (tmp8 & CmdTxEnb){
		RTL_W8 ( ChipCmd, tmp8 & ~CmdTxEnb);
	}

	/* Disable interrupts by clearing the interrupt mask. */
	RTL_W16 ( IntrMask, 0x0000);

	/* Stop a shared interrupt from scavenging while we are. */
	spin_lock_irq (&priv->lock);
	r1000_tx_clear (priv);
	spin_unlock_irq (&priv->lock);


	r1000_hw_start(netdev);

	netif_wake_queue(netdev);
}

static int r1000_start_xmit (struct sk_buff *skb, struct net_device *netdev)
{
	struct r1000_private *priv = netdev->priv;
	unsigned long ioaddr = priv->ioaddr;
	struct pci_dev *pdev = priv->pci_dev;
	int entry = priv->cur_tx % NUM_TX_DESC;
	int buf_len = 60;
	dma_addr_t txbuf_dma_addr;

	spin_lock_irq (&priv->lock);
	if( (le32_to_cpu(priv->TxDescArray[entry].status) & OWNbit)==0 ){

		priv->Tx_skbuff[entry] = skb;
		txbuf_dma_addr = pci_map_single(pdev, skb->data, skb->len, PCI_DMA_TODEVICE);
		
		priv->TxDescArray[entry].buf_addr = cpu_to_le32(txbuf_dma_addr);
		DBG_PRINT("%s: TX pkt_size = %d\n", __FUNCTION__, skb->len);
		if( skb->len <= priv->tx_pkt_len ){
			buf_len = skb->len;
		}
		else{
			printk("%s: Error -- Tx packet size(%d) > mtu(%d)+14\n", netdev->name, skb->len, netdev->mtu);
			buf_len = priv->tx_pkt_len;
		}

		if( entry != (NUM_TX_DESC-1) ){
			priv->TxDescArray[entry].status = cpu_to_le32((OWNbit | FSbit | LSbit) | buf_len);
		}
		else{
			priv->TxDescArray[entry].status = cpu_to_le32((OWNbit | EORbit | FSbit | LSbit) | buf_len);
		}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
		pci_dma_sync_single(pdev, priv->txdesc_array_dma_addr[entry], sizeof(struct TxDesc), PCI_DMA_TODEVICE);
#endif

		RTL_W8 ( TxPoll, 0x40);		//set polling bit

		netdev->trans_start = jiffies;

		priv->stats.tx_bytes += ( (skb->len > ETH_ZLEN) ? skb->len : ETH_ZLEN);
		priv->cur_tx++;
	}//end of if( (priv->TxDescArray[entry].status & 0x80000000)==0 )

	spin_unlock_irq (&priv->lock);

	if ( (priv->cur_tx - NUM_TX_DESC) == priv->dirty_tx ){
		netif_stop_queue(netdev);
	}
	else{
		if (netif_queue_stopped(netdev)){
			netif_wake_queue(netdev);
		}
	}

	return 0;
}

static void r1000_tx_interrupt (struct net_device *netdev, struct r1000_private *priv, unsigned long ioaddr)
{
	unsigned long dirty_tx, tx_left=0;
	int entry = priv->cur_tx % NUM_TX_DESC;
    	int txloop_cnt = 0;

	assert (netdev != NULL);
	assert (priv != NULL);
	assert (ioaddr != NULL);


	dirty_tx = priv->dirty_tx;
	tx_left = priv->cur_tx - dirty_tx;

	while( (tx_left > 0) && (txloop_cnt < max_interrupt_work) ){
		if( (le32_to_cpu(priv->TxDescArray[entry].status) & OWNbit) == 0 ){

#ifdef R1000_DYNAMIC_CONTROL
			r1000_callback_tx(&(priv->rt), 1, priv->Tx_skbuff[dirty_tx % NUM_TX_DESC]->len);
#endif //end #ifdef R1000_DYNAMIC_CONTROL

			dev_kfree_skb_irq( priv->Tx_skbuff[dirty_tx % NUM_TX_DESC] );
			priv->Tx_skbuff[dirty_tx % NUM_TX_DESC] = NULL;
			priv->stats.tx_packets++;
			dirty_tx++;
			tx_left--;
			entry++;
		}
		txloop_cnt ++;
	}

	if (priv->dirty_tx != dirty_tx) {
		priv->dirty_tx = dirty_tx;
		if (netif_queue_stopped(netdev))
			netif_wake_queue(netdev);
	}
}

static void r1000_rx_interrupt (struct net_device *netdev, struct r1000_private *priv, unsigned long ioaddr)
{
	struct pci_dev *pdev = priv->pci_dev;
	int cur_rx;
	int pkt_size = 0 ;
	int rxdesc_cnt = 0;
	int ret;
	struct sk_buff *n_skb = NULL;
	struct sk_buff *cur_skb;
	struct sk_buff *rx_skb;
	struct	RxDesc	*rxdesc;

	assert(netdev != NULL);
	assert (priv != NULL);
	assert (ioaddr != NULL);


	cur_rx = priv->cur_rx;

	rxdesc = &priv->RxDescArray[cur_rx];
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
	pci_dma_sync_single(pdev, priv->rxdesc_array_dma_addr[cur_rx], sizeof(struct RxDesc), PCI_DMA_FROMDEVICE);
#endif

	while ( ((le32_to_cpu(rxdesc->status) & OWNbit)== 0) && (rxdesc_cnt < max_interrupt_work) ){

	    rxdesc_cnt++;

	    if( le32_to_cpu(rxdesc->status) & RxRES ){
			printk(KERN_INFO "%s: Rx ERROR!!!\n", netdev->name);
			priv->stats.rx_errors++;
			if ( le32_to_cpu(rxdesc->status) & (RxRWT|RxRUNT) )
				priv->stats.rx_length_errors++;
			if ( le32_to_cpu(rxdesc->status) & RxCRC)
				priv->stats.rx_crc_errors++;
	    }
	    else{
			pkt_size=(int)(le32_to_cpu(rxdesc->status) & 0x00001FFF)-4;

			if( pkt_size > priv->rx_pkt_len ){
				printk("%s: Error -- Rx packet size(%d) > mtu(%d)+14\n", netdev->name, pkt_size, netdev->mtu);
				pkt_size = priv->rx_pkt_len;
			}

			DBG_PRINT("%s: RX pkt_size = %d\n", __FUNCTION__, pkt_size);

			{// -----------------------------------------------------
				rx_skb = priv->Rx_skbuff[cur_rx];
				n_skb = R1000_ALLOC_RXSKB(MAX_RX_SKBDATA_SIZE);
				if( n_skb != NULL ) {
					skb_reserve (n_skb, 8);	// 16 byte align the IP fields. //

					// Indicate rx_skb
					if( rx_skb != NULL ){
						rx_skb->dev = netdev;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
						pci_dma_sync_single(pdev, priv->rx_skbuff_dma_addr[cur_rx], sizeof(struct RxDesc), PCI_DMA_FROMDEVICE);
#endif

						skb_put ( rx_skb, pkt_size );
						rx_skb->protocol = eth_type_trans ( rx_skb, netdev );
						ret = R1000_NETIF_RX (rx_skb);

//						netdev->last_rx = jiffies;
						priv->stats.rx_bytes += pkt_size;
						priv->stats.rx_packets++;

#ifdef R1000_DYNAMIC_CONTROL
						r1000_callback_rx( &(priv->rt), 1, pkt_size);
#endif //end #ifdef R1000_DYNAMIC_CONTROL

					}//end if( rx_skb != NULL )

					priv->Rx_skbuff[cur_rx] = n_skb;
				}
				else{
					DBG_PRINT("%s: Allocate n_skb failed!\n",__FUNCTION__ );
					priv->Rx_skbuff[cur_rx] = rx_skb;
				}


				// Update rx descriptor
				if( cur_rx == (NUM_RX_DESC-1) ){
					priv->RxDescArray[cur_rx].status  = cpu_to_le32((OWNbit | EORbit) | (unsigned long)priv->hw_rx_pkt_len);
				}
				else{
					priv->RxDescArray[cur_rx].status  = cpu_to_le32(OWNbit | (unsigned long)priv->hw_rx_pkt_len);
				}

				cur_skb = priv->Rx_skbuff[cur_rx];

				if( cur_skb != NULL ){
					priv->rx_skbuff_dma_addr[cur_rx] = pci_map_single(pdev, cur_skb->data, MAX_RX_SKBDATA_SIZE, PCI_DMA_FROMDEVICE);
					rxdesc->buf_addr = cpu_to_le32(priv->rx_skbuff_dma_addr[cur_rx]);
				}
				else{
					DBG_PRINT("%s: %s() cur_skb == NULL\n", netdev->name, __FUNCTION__);
				}

			}//------------------------------------------------------------

	    }// end of if( priv->RxDescArray[cur_rx].status & RxRES )

	    cur_rx = (cur_rx +1) % NUM_RX_DESC;
	    rxdesc = &priv->RxDescArray[cur_rx];
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
	    pci_dma_sync_single(pdev, priv->rxdesc_array_dma_addr[cur_rx], sizeof(struct RxDesc), PCI_DMA_FROMDEVICE);
#endif

	}// end of while ( (priv->RxDescArray[cur_rx].status & 0x80000000)== 0)

	if( rxdesc_cnt >= max_interrupt_work ){
		DBG_PRINT("%s: Too much work at Rx interrupt.\n", netdev->name);
	}

	priv->cur_rx = cur_rx;
}

/* The interrupt handler does all of the Rx thread work and cleans up after the Tx thread. */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
static void r1000_interrupt (int irq, void *dev_instance, struct pt_regs *regs)
#else
static irqreturn_t r1000_interrupt (int irq, void *dev_instance, struct pt_regs *regs)
#endif
{
	struct net_device *netdev = (struct net_device *) dev_instance;
	struct r1000_private *priv = netdev->priv;
	int boguscnt = max_interrupt_work;
	unsigned long ioaddr = priv->ioaddr;
	unsigned int status = 0;
//	irqreturn_int interrupt_handled = IRQ_NONE;
	int interrupt_handled = IRQ_NONE;

	unsigned int phy_status = 0;

	RTL_W16 ( IntrMask, 0x0000);

	do {
		status = RTL_R16(IntrStatus);

		if (status == 0xFFFF)
			break;

		RTL_W16( IntrStatus, status );

		if ( (status & r1000_intr_mask ) == 0 )
			break;
		else
			interrupt_handled = IRQ_HANDLED;

		// Rx interrupt
//		if (status & (RxOK | RxErr /* | LinkChg | RxOverflow | RxFIFOOver*/)){
			r1000_rx_interrupt(netdev, priv, ioaddr);
//		}

		// Tx interrupt
//		if (status & (TxOK | TxErr)) {
			spin_lock (&priv->lock);
			r1000_tx_interrupt(netdev, priv, ioaddr);
			spin_unlock (&priv->lock);
//		}
		if ((status&TxOK)&&(status&TxDescUnavail)) 
			RTL_W8(TxPoll,0x40);

		phy_status = RTL_R8(PHYstatus);
		if(((priv->mcfg==MCFG_METHOD_2)||(priv->mcfg==MCFG_METHOD_3))&&(phy_status&_100Mbps)){
			if(status&LinkChg){
				if(phy_status&LinkStatus){
					R1000_WRITE_GMII_REG((unsigned long)ioaddr,0x1f,0x0001);
					R1000_WRITE_GMII_REG((unsigned long)ioaddr,0x10,0xf01b);
					R1000_WRITE_GMII_REG((unsigned long)ioaddr,0x1f,0x0000);
				}else{
					R1000_WRITE_GMII_REG((unsigned long)ioaddr,0x1f,0x0001);
					R1000_WRITE_GMII_REG((unsigned long)ioaddr,0x10,0xf41b);
					R1000_WRITE_GMII_REG((unsigned long)ioaddr,0x1f,0x0000);
				}
			}
		}
			
		boguscnt--;
	} while (boguscnt > 0);

	if (boguscnt <= 0) {
		DBG_PRINT("%s: Too much work at interrupt!\n", netdev->name);
		RTL_W16( IntrStatus, 0xffff);	// Clear all interrupt sources
	}

	RTL_W16 ( IntrMask, r1000_intr_mask);

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,5,0)
	return interrupt_handled;
#endif
}

static int r1000_close (struct net_device *netdev)
{
	struct r1000_private *priv = netdev->priv;
	unsigned long ioaddr = priv->ioaddr;
	int i;

	// -----------------------------------------
	r1000_delete_timer( &(priv->r1000_timer) );


	netif_stop_queue(netdev);

	spin_lock_irq (&priv->lock);

	/* Stop the chip's Tx and Rx processes. */
	RTL_W8 ( ChipCmd, 0x00);

	/* Disable interrupts by clearing the interrupt mask. */
	RTL_W16 ( IntrMask, 0x0000);

	/* Update the error counts. */
	priv->stats.rx_missed_errors += RTL_R32(RxMissed);
	RTL_W32( RxMissed, 0);

	spin_unlock_irq (&priv->lock);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	synchronize_irq ();
#else
	synchronize_irq(entdev->irq);
#endif
	free_irq (netdev->irq, netdev);

	r1000_tx_clear (priv);
	
	//2004-05-11
	if(priv->txdesc_space != NULL){
		pci_free_consistent(
				priv->pci_dev,
				priv->sizeof_txdesc_space,
				priv->txdesc_space,
				priv->txdesc_phy_dma_addr
		);
		priv->txdesc_space = NULL;
	}

	if(priv->rxdesc_space != NULL){
		pci_free_consistent(
				priv->pci_dev,
				priv->sizeof_rxdesc_space,
				priv->rxdesc_space,
				priv->rxdesc_phy_dma_addr
		);
		priv->rxdesc_space = NULL;
	}

	priv->TxDescArray = NULL;
	priv->RxDescArray = NULL;

	{//-----------------------------------------------------------------------------
		for(i=0;i<NUM_RX_DESC;i++){
			if( priv->Rx_skbuff[i] != NULL ) {
				R1000_FREE_RXSKB ( priv->Rx_skbuff[i] );
			}
		}
	}//-----------------------------------------------------------------------------

	DBG_PRINT("%s: %s() alloc_rxskb_cnt = %d\n", netdev->name, __FUNCTION__, alloc_rxskb_cnt );

	return 0;
}

static unsigned const ethernet_polynomial = 0x04c11db7U;
static inline u32 ether_crc (int length, unsigned char *data)
{
	int crc = -1;

	while (--length >= 0) {
		unsigned char current_octet = *data++;
		int bit;
		for (bit = 0; bit < 8; bit++, current_octet >>= 1)
			crc = (crc << 1) ^ ((crc < 0) ^ (current_octet & 1) ? ethernet_polynomial : 0);
	}

	return crc;
}

static void r1000_set_rx_mode (struct net_device *netdev)
{
	struct r1000_private *priv = netdev->priv;
	unsigned long ioaddr = priv->ioaddr;
	unsigned long flags;
	u32 mc_filter[2];	/* Multicast hash filter */
	int i, rx_mode;
	u32 tmp=0;
	

	if (netdev->flags & IFF_PROMISC) {
		/* Unconditionally log net taps. */
		printk (KERN_NOTICE "%s: Promiscuous mode enabled.\n", netdev->name);
		rx_mode = AcceptBroadcast | AcceptMulticast | AcceptMyPhys | AcceptAllPhys;
		mc_filter[1] = mc_filter[0] = 0xffffffff;
	} else if ((netdev->mc_count > multicast_filter_limit) || (netdev->flags & IFF_ALLMULTI)) {
		/* Too many to filter perfectly -- accept all multicasts. */
		rx_mode = AcceptBroadcast | AcceptMulticast | AcceptMyPhys;
		mc_filter[1] = mc_filter[0] = 0xffffffff;
	} else {
		struct dev_mc_list *mclist;
		rx_mode = AcceptBroadcast | AcceptMulticast | AcceptMyPhys;
		mc_filter[1] = mc_filter[0] = 0;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
		for (i = 0, mclist = netdev->mc_list; mclist && i < netdev->mc_count; i++, mclist = mclist->next)
		{
			set_bit (ether_crc (ETH_ALEN, mclist->dmi_addr) >> 26, mc_filter);
		}			
#else
		for (i = 0, mclist = netdev->mc_list; mclist && i < netdev->mc_count; i++, mclist = mclist->next)
		{
			int bit_nr = ether_crc(ETH_ALEN, mclist->dmi_addr) >> 26;
			                                                                                                     
			mc_filter[bit_nr >> 5] |= 1 << (bit_nr & 31);
			rx_mode |= AcceptMulticast;
		}
#endif		
	}

	spin_lock_irqsave (&priv->lock, flags);

	tmp = r1000_rx_config | rx_mode | (RTL_R32(RxConfig) & rtl_chip_info[priv->chipset].RxConfigMask);

	RTL_W32 ( RxConfig, tmp);
	if((priv->mcfg==MCFG_METHOD_11)||(priv->mcfg==MCFG_METHOD_12)||
	   (priv->mcfg==MCFG_METHOD_13)||(priv->mcfg==MCFG_METHOD_14)||
	   (priv->mcfg==MCFG_METHOD_15)){
		RTL_W32 ( MAR0 + 0, 0xFFFFFFFF);
		RTL_W32 ( MAR0 + 4, 0xFFFFFFFF);
	}else{
		RTL_W32 ( MAR0 + 0, mc_filter[0]);
		RTL_W32 ( MAR0 + 4, mc_filter[1]);
	}

	spin_unlock_irqrestore (&priv->lock, flags);

}//end of r1000_set_rx_mode (struct net_device *netdev)

struct net_device_stats *r1000_get_stats(struct net_device *netdev)

{
	struct r1000_private *priv = netdev->priv;

    return &priv->stats;
}

static struct pci_driver r1000_pci_driver = {
	name:		MODULENAME,
	id_table:	r1000_pci_tbl,
	probe:		r1000_init_one,
	remove:		__devexit_p(r1000_remove_one),
	suspend:	NULL,
	resume:		NULL,
};

static int __init r1000_init_module (void)
{
	return pci_module_init (&r1000_pci_driver);	// pci_register_driver (drv)
}

static void __exit r1000_cleanup_module (void)
{
	pci_unregister_driver (&r1000_pci_driver);
}

#ifdef R1000_JUMBO_FRAME_SUPPORT
static int r1000_change_mtu(struct net_device *netdev, int new_mtu)
{
	struct r1000_private *priv = netdev->priv;
	unsigned long ioaddr = priv->ioaddr;

	if( new_mtu > MAX_JUMBO_FRAME_MTU ){
		printk("%s: Error -- new_mtu(%d) > MAX_JUMBO_FRAME_MTU(%d).\n", netdev->name, new_mtu, MAX_JUMBO_FRAME_MTU);
		return -1;
	}

	netdev->mtu = new_mtu;

	priv->curr_mtu_size = new_mtu;
	priv->tx_pkt_len = new_mtu + ETH_HDR_LEN;
	priv->rx_pkt_len = new_mtu + ETH_HDR_LEN;
	priv->hw_rx_pkt_len = priv->rx_pkt_len + 8;

	RTL_W8 ( Cfg9346, Cfg9346_Unlock);
	RTL_W16	( RxMaxSize, (unsigned short)priv->hw_rx_pkt_len );
	RTL_W8 ( Cfg9346, Cfg9346_Lock);

	DBG_PRINT("-------------------------- \n");
	DBG_PRINT("netdev->mtu = %d \n", netdev->mtu);
	DBG_PRINT("priv->curr_mtu_size = %d \n", priv->curr_mtu_size);
	DBG_PRINT("priv->rx_pkt_len = %d \n", priv->rx_pkt_len);
	DBG_PRINT("priv->tx_pkt_len = %d \n", priv->tx_pkt_len);
	DBG_PRINT("RTL_W16( RxMaxSize, %d )\n", priv->hw_rx_pkt_len);
	DBG_PRINT("-------------------------- \n");

	r1000_close(netdev);
	r1000_open(netdev);

	return 0;
}
#endif //end #ifdef R1000_JUMBO_FRAME_SUPPORT

module_init(r1000_init_module);
module_exit(r1000_cleanup_module);
