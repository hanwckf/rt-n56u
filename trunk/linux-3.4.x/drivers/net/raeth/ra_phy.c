#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/netdevice.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
#include <linux/sched.h>
#endif

#include "raether.h"
#include "ra_mac.h"
#include "mii_mgr.h"

#ifdef CONFIG_RALINK_VISTA_BASIC
extern int is_switch_175c;
#endif

#if defined (CONFIG_RAETH_ESW_CONTROL)
extern int esw_control_post_init(void);
#endif

#if defined (CONFIG_GIGAPHY) || defined (CONFIG_P4_MAC_TO_PHY_MODE) || defined (CONFIG_P5_MAC_TO_PHY_MODE)
static int isICPlusGigaPHY(int ge)
{
	u32 phy_id0 = 0, phy_id1 = 0;

#if defined (CONFIG_GE2_RGMII_AN) || defined (CONFIG_P4_MAC_TO_PHY_MODE)
	if (ge == 2) {
		if (!mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2, 2, &phy_id0)) {
			printk("\n Read PhyID 1 is Fail!!\n");
			phy_id0 =0;
		}
		if (!mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2, 3, &phy_id1)) {
			printk("\n Read PhyID 1 is Fail!!\n");
			phy_id1 = 0;
		}
	}
	else
#endif
#if defined (CONFIG_GE1_RGMII_AN) || defined (CONFIG_P5_MAC_TO_PHY_MODE)
	{
		if (!mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 2, &phy_id0)) {
			printk("\n Read PhyID 0 is Fail!!\n");
			phy_id0 =0;
		}
		if (!mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 3, &phy_id1)) {
			printk("\n Read PhyID 0 is Fail!!\n");
			phy_id1 = 0;
		}
	}
#endif
	if ((phy_id0 == EV_ICPLUS_PHY_ID0) && ((phy_id1 & 0xfff0) == EV_ICPLUS_PHY_ID1))
		return 1;

	return 0;
}

static int isMarvellGigaPHY(int ge)
{
	u32 phy_id0 = 0, phy_id1 = 0;

#if defined (CONFIG_GE2_RGMII_AN) || defined (CONFIG_P4_MAC_TO_PHY_MODE)
	if (ge == 2) {
		if (!mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2, 2, &phy_id0)) {
			printk("\n Read PhyID 1 is Fail!!\n");
			phy_id0 =0;
		}
		if (!mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2, 3, &phy_id1)) {
			printk("\n Read PhyID 1 is Fail!!\n");
			phy_id1 = 0;
		}
	}
	else
#endif
#if defined (CONFIG_GE1_RGMII_AN) || defined (CONFIG_P5_MAC_TO_PHY_MODE)
	{
		if (!mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 2, &phy_id0)) {
			printk("\n Read PhyID 0 is Fail!!\n");
			phy_id0 =0;
		}
		if (!mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 3, &phy_id1)) {
			printk("\n Read PhyID 0 is Fail!!\n");
			phy_id1 = 0;
		}
	}
#endif
	if ((phy_id0 == EV_MARVELL_PHY_ID0) && (phy_id1 == EV_MARVELL_PHY_ID1))
		return 1;

	return 0;
}

static int isVtssGigaPHY(int ge)
{
	u32 phy_id0 = 0, phy_id1 = 0;

#if defined (CONFIG_GE2_RGMII_AN) || defined (CONFIG_P4_MAC_TO_PHY_MODE)
	if (ge == 2) {
		if (!mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2, 2, &phy_id0)) {
			printk("\n Read PhyID 1 is Fail!!\n");
			phy_id0 =0;
		}
		if (!mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2, 3, &phy_id1)) {
			printk("\n Read PhyID 1 is Fail!!\n");
			phy_id1 = 0;
		}
	}
	else
#endif
#if defined (CONFIG_GE1_RGMII_AN) || defined (CONFIG_P5_MAC_TO_PHY_MODE)
	{
		if (!mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 2, &phy_id0)) {
			printk("\n Read PhyID 0 is Fail!!\n");
			phy_id0 =0;
		}
		if (!mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 3, &phy_id1)) {
			printk("\n Read PhyID 0 is Fail!!\n");
			phy_id1 = 0;
		}
	}
#endif
	if ((phy_id0 == EV_VTSS_PHY_ID0) && (phy_id1 == EV_VTSS_PHY_ID1))
		return 1;

	return 0;
}

static void init_giga_phy(int ge)
{
#if defined(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2)
	u32 phy_addr = (ge == 2) ? CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2 : CONFIG_MAC_TO_GIGAPHY_MODE_ADDR;
#else
	u32 phy_addr = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR;
#endif

	if (isICPlusGigaPHY(ge)) {
		mii_mgr_read(phy_addr, 4, &phy_val);
		phy_val |= 1<<10; //enable pause ability
		mii_mgr_write(phy_addr, 4, phy_val);
		mii_mgr_read(phy_addr, 0, &phy_val);
		phy_val |= 1<<9; //restart AN
		mii_mgr_write(phy_addr, 0, phy_val);
	} else if (isMarvellGigaPHY(ge)) {
		printk("Reset MARVELL phy1\n");
		mii_mgr_read(phy_addr, 20, &phy_val);
		phy_val |= 1<<7; //Add delay to RX_CLK for RXD Outputs
		mii_mgr_write(phy_addr, 20, phy_val);
		mii_mgr_read(phy_addr, 0, &phy_val);
		phy_val |= 1<<15; //PHY Software Reset
		mii_mgr_write(phy_addr, 0, phy_val);
	} else if (isVtssGigaPHY(ge)) {
		mii_mgr_write(phy_addr, 31, 0x0001); //extended page
		mii_mgr_read(phy_addr, 28, &phy_val);
		printk("Vitesse phy skew: %x --> ", phy_val);
		phy_val |= (0x3<<12); // RGMII RX skew compensation= 2.0 ns
		phy_val &= ~(0x3<<14);// RGMII TX skew compensation= 0 ns
		printk("%x\n", phy_val);
		mii_mgr_write(phy_addr, 28, phy_val);
		mii_mgr_write(phy_addr, 31, 0x0000); //main registers
	}
}
#endif

#if defined(CONFIG_RALINK_MT7620)
static void mt7620_gsw_init(void)
{
	u32 is_BGA = (sysRegRead(RALINK_SYSCTL_BASE + 0xc) >> 16) & 0x1;

	*(volatile u32 *)(SYSCFG1) |= (0x1 << 8); // PCIE_RC_MODE=1
	*(volatile u32 *)(CKGCR) &= ~(0x3 << 4); // keep rx/tx port clock ticking, disable internal clock-gating to avoid switch stuck 

	/*
	* Reg 31: Page Control
	* Bit 15     => PortPageSel, 1=local, 0=global
	* Bit 14:12  => PageSel, local:0~3, global:0~4
	*
	* Reg16~30:Local/Global registers
	*
	*/
	/*correct PHY setting L3.0 BGA*/
	mii_mgr_write(1, 31, 0x4000); //global, page 4

	mii_mgr_write(1, 17, 0x7444);
	if (is_BGA)
		mii_mgr_write(1, 19, 0x0114);
	else
		mii_mgr_write(1, 19, 0x0117);

	mii_mgr_write(1, 22, 0x10cf);
	mii_mgr_write(1, 25, 0x6212);
	mii_mgr_write(1, 26, 0x0777);
	mii_mgr_write(1, 29, 0x4000);
	mii_mgr_write(1, 28, 0xc077);
	mii_mgr_write(1, 24, 0x0000);

	mii_mgr_write(1, 31, 0x3000); //global, page 3
	mii_mgr_write(1, 17, 0x4838);

	mii_mgr_write(1, 31, 0x2000); //global, page 2
	if (is_BGA) {
		mii_mgr_write(1, 21, 0x0515);
		mii_mgr_write(1, 22, 0x0053);
		mii_mgr_write(1, 23, 0x00bf);
		mii_mgr_write(1, 24, 0x0aaf);
		mii_mgr_write(1, 25, 0x0fad);
		mii_mgr_write(1, 26, 0x0fc1);
	} else {
		mii_mgr_write(1, 21, 0x0517);
		mii_mgr_write(1, 22, 0x0fd2);
		mii_mgr_write(1, 23, 0x00bf);
		mii_mgr_write(1, 24, 0x0aab);
		mii_mgr_write(1, 25, 0x00ae);
		mii_mgr_write(1, 26, 0x0fff);
	}
	mii_mgr_write(1, 31, 0x1000); //global, page 1
	mii_mgr_write(1, 17, 0xe7f8);

	mii_mgr_write(1, 31, 0x8000); //local, page 0
	mii_mgr_write(0, 30, 0xa000);
	mii_mgr_write(1, 30, 0xa000);
	mii_mgr_write(2, 30, 0xa000);
	mii_mgr_write(3, 30, 0xa000);
#if !defined (CONFIG_RAETH_HAS_PORT4)
	mii_mgr_write(4, 30, 0xa000);
#endif

	mii_mgr_write(0, 4, 0x05e1);
	mii_mgr_write(1, 4, 0x05e1);
	mii_mgr_write(2, 4, 0x05e1);
	mii_mgr_write(3, 4, 0x05e1);
#if !defined (CONFIG_RAETH_HAS_PORT4)
	mii_mgr_write(4, 4, 0x05e1);
#endif

	mii_mgr_write(1, 31, 0xa000); // local, page 2
	mii_mgr_write(0, 16, 0x1111);
	mii_mgr_write(1, 16, 0x1010);
	mii_mgr_write(2, 16, 0x1515);
	mii_mgr_write(3, 16, 0x0f0f);
#if !defined (CONFIG_RAETH_HAS_PORT4)
	mii_mgr_write(4, 16, 0x1313);
#endif

	/* Port 6 (CPU) */
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x3600) = 0x0005e33b;	// CPU Port6 Force Link 1G, FC ON
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x0010) = 0x7f7f7fe0;	// Set Port6 CPU Port

	/* Port 5 */
#if defined (CONFIG_P5_RGMII_TO_MAC_MODE)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x3500) = 0x0005e33b;	// (P5, Force mode, Link Up, 1000Mbps, Full-Duplex, FC ON)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x250c) = 0x000fff10;	// disable P5 mac learning
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x7014) = 0x1fec000c;	// disable PHY 0 ~ 4, set phy base address to 12
	*(volatile u32 *)(REG_GPIOMODE) &= ~(1 << 9);			// set RGMII to Normal mode
	//rxclk_skew, txclk_skew = 0
	*(volatile u32 *)(SYSCFG1) &= ~(0x3 << 12);			// GE1_MODE=RGMii Mode
#elif defined (CONFIG_P5_MII_TO_MAC_MODE)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x3500) = 0x0005e337;	// (P5, Force mode, Link Up, 100Mbps, Full-Duplex, FC ON)
	*(volatile u32 *)(REG_GPIOMODE) &= ~(1 << 9);			// set RGMII to Normal mode
	*(volatile u32 *)(SYSCFG1) &= ~(0x3 << 12);			// GE1_MODE=Mii Mode
	*(volatile u32 *)(SYSCFG1) |= (0x1 << 12);
#elif defined (CONFIG_P5_MAC_TO_PHY_MODE)
	*(volatile u32 *)(REG_GPIOMODE) &= ~(1 << 9);			// set RGMII to Normal mode
	*(volatile u32 *)(REG_GPIOMODE) &= ~(3 << 7);			// set MDIO to Normal mode
	*(volatile u32 *)(SYSCFG1) &= ~(0x3 << 12);			// GE1_MODE=RGMii Mode
	enable_auto_negotiate(1);
	init_giga_phy(1);
#elif defined (CONFIG_P5_RMII_TO_MAC_MODE)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x3500) = 0x0005e337;	// (P5, Force mode, Link Up, 100Mbps, Full-Duplex, FC ON)
	*(volatile u32 *)(REG_GPIOMODE) &= ~(1 << 9);			// set RGMII to Normal mode
	*(volatile u32 *)(SYSCFG1) &= ~(0x3 << 12);			// GE1_MODE=RvMii Mode
	*(volatile u32 *)(SYSCFG1) |= (0x2 << 12);
#else /* Port 5 Disabled */
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x3500) = 0x00008000;	// P5 link down
	*(volatile u32 *)(REG_GPIOMODE) |= (1 << 9);			// set RGMII to GPIO mode
#endif

	/* Port 4 */
#if defined (CONFIG_RALINK_MT7620)
#if defined (CONFIG_P4_RGMII_TO_MAC_MODE)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x3400) = 0x0005e33b;	// (P4, Force mode, Link Up, 1000Mbps, Full-Duplex, FC ON)
	*(volatile u32 *)(REG_GPIOMODE) &= ~(1 << 10);			// set GE2 to Normal mode
	//rxclk_skew, txclk_skew = 0
	*(volatile u32 *)(SYSCFG1) &= ~(0x3 << 14);			// GE2_MODE=RGMii Mode
#elif defined (CONFIG_P4_MII_TO_MAC_MODE)
	*(volatile u32 *)(REG_GPIOMODE) &= ~(1 << 10);			// set GE2 to Normal mode
	*(volatile u32 *)(SYSCFG1) &= ~(0x3 << 14);			// GE2_MODE=Mii Mode
	*(volatile u32 *)(SYSCFG1) |= (0x1 << 14);
#elif defined (CONFIG_P4_MAC_TO_PHY_MODE)
	*(volatile u32 *)(REG_GPIOMODE) &= ~(1 << 10);			// set GE2 to Normal mode
	*(volatile u32 *)(REG_GPIOMODE) &= ~(3 << 7);			// set MDIO to Normal mode
	*(volatile u32 *)(SYSCFG1) &= ~(0x3 << 14);			// GE2_MODE=RGMii Mode
	enable_auto_negotiate(1);
	init_giga_phy(2);
#elif defined (CONFIG_P4_RMII_TO_MAC_MODE)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x3400) = 0x0005e337;	// (P4, Force mode, Link Up, 100Mbps, Full-Duplex, FC ON)
	*(volatile u32 *)(REG_GPIOMODE) &= ~(1 << 10);			// set GE2 to Normal mode
	*(volatile u32 *)(SYSCFG1) &= ~(0x3 << 14);			// GE1_MODE=RvMii Mode
	*(volatile u32 *)(SYSCFG1) |= (0x2 << 14);
#else /* Port 4 Disabled */
	*(volatile u32 *)(SYSCFG1) |= (0x3 << 14);			// GE2_MODE=RJ45 Mode
	*(volatile u32 *)(REG_GPIOMODE) |= (1 << 10);			// set RGMII2 to GPIO mode
#endif
#endif
}
#endif

#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350)
static void rt305x_esw_init(void)
{
	int i=0;
	u32 phy_val=0, val=0;
#if defined (CONFIG_RT3052_ASIC)
	u32 phy_val2;
#endif

#if defined (CONFIG_RT5350_ASIC)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x0168) = 0x17;
#endif

	/*
	 * FC_RLS_TH=200, FC_SET_TH=160
	 * DROP_RLS=120, DROP_SET_TH=80
	 */
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x0008) = 0xC8A07850;
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00E4) = 0x00000000;
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x0014) = 0x00405555;
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x0050) = 0x00002001;
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x0090) = 0x00007f7f;
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x0098) = 0x00007f3f; //disable VLAN
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00CC) = 0x0002500c;
#ifndef CONFIG_UNH_TEST
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x009C) = 0x0008a301; //hashing algorithm=XOR48, aging interval=300sec
#else
	/*
	 * bit[30]:1	Backoff Algorithm Option: The latest one to pass UNH test
	 * bit[29]:1	Length of Received Frame Check Enable
	 * bit[8]:0	Enable collision 16 packet abort and late collision abort
	 * bit[7:6]:01	Maximum Packet Length: 1518
	 */
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x009C) = 0x6008a241;
#endif
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x008C) = 0x02404040;
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) = 0x3f502b28; //Change polling Ext PHY Addr=0x1F
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x0084) = 0x00000000;
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x0110) = 0x7d000000; //1us cycle number=125 (FE's clock=125Mhz)
	
	/*
	 * set port 5 force to 1000M/Full when connecting to switch or iNIC
	 */
#if defined (CONFIG_P5_RGMII_TO_MAC_MODE)
	*(volatile u32 *)(REG_GPIOMODE) &= ~(1 << 9);			// set RGMII to Normal mode
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1<<29);	// disable port 5 auto-polling
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) |= 0x3fff;		// force 1000M full duplex
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(0xf<<20);	// rxclk_skew, txclk_skew = 0
#elif defined (CONFIG_P5_MII_TO_MAC_MODE)
	*(volatile u32 *)(REG_GPIOMODE) &= ~(1 << 9);			// set RGMII to Normal mode
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1<<29);	// disable port 5 auto-polling
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(0x3fff);
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) |= 0x3ffd;		// force 100M full duplex
#if defined (CONFIG_RALINK_RT3352)
	*(volatile u32 *)(SYSCFG1) &= ~(0x3 << 12);			// GE1_MODE=Mii Mode
	*(volatile u32 *)(SYSCFG1) |= (0x1 << 12);
#endif
#elif defined (CONFIG_P5_MAC_TO_PHY_MODE)
	*(volatile u32 *)(REG_GPIOMODE) &= ~(1 << 9);			// set RGMII to Normal mode
	*(volatile u32 *)(REG_GPIOMODE) &= ~(1 << 7);			// set MDIO to Normal mode
#if defined (CONFIG_RT3052_ASIC) || defined (CONFIG_RT3352_ASIC)
	enable_auto_negotiate(1);
#endif
	init_giga_phy(1);
#elif defined (CONFIG_P5_RMII_TO_MAC_MODE)
	*(volatile u32 *)(REG_GPIOMODE) &= ~(1 << 9);			// set RGMII to Normal mode
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1<<29);	// disable port 5 auto-polling
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(0x3fff);
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) |= 0x3ffd;		// force 100M full duplex
#if defined (CONFIG_RALINK_RT3352)
	*(volatile u32 *)(SYSCFG1) &= ~(0x3 << 12);			// GE1_MODE=RvMii Mode
	*(volatile u32 *)(SYSCFG1) |= (0x2 << 12);
#endif
#else // Port 5 Disabled //

#if defined (CONFIG_RALINK_RT3052)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1 << 29);	// port5 auto polling disable
	*(volatile u32 *)(REG_GPIOMODE) |= (1 << 7);			// set MDIO to GPIO mode (GPIO22-GPIO23)
	*(volatile u32 *)(REG_GPIOMODE) |= (1 << 9);			// set RGMII to GPIO mode (GPIO41-GPIO50)
	*(volatile u32 *)(0xb0000674) = 0xFFF;				// GPIO41-GPIO50 output mode
	*(volatile u32 *)(0xb000067C) = 0x0;				// GPIO41-GPIO50 output low
#elif defined (CONFIG_RALINK_RT3352)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1 << 29);	// port5 auto polling disable
	*(volatile u32 *)(REG_GPIOMODE) |= (1 << 7);			// set MDIO to GPIO mode (GPIO22-GPIO23)
	*(volatile u32 *)(0xb0000624) = 0xC0000000;			// GPIO22-GPIO23 output mode
	*(volatile u32 *)(0xb000062C) = 0xC0000000;			// GPIO22-GPIO23 output high
	*(volatile u32 *)(REG_GPIOMODE) |= (1 << 9);			// set RGMII to GPIO mode (GPIO24-GPIO35)
	*(volatile u32 *)(0xb000064C) = 0xFFF;				// GPIO24-GPIO35 output mode
	*(volatile u32 *)(0xb0000654) = 0xFFF;				// GPIO24-GPIO35 output high
#elif defined (CONFIG_RALINK_RT5350)
	/* do nothing */
#endif
#endif // CONFIG_P5_RGMII_TO_MAC_MODE //

#if defined (CONFIG_RT3052_ASIC)
	rw_rf_reg(0, 0, &phy_val);
	phy_val = phy_val >> 4;

	if(phy_val > 0x5) {
		rw_rf_reg(0, 26, &phy_val);
		phy_val2 = (phy_val | (0x3 << 5));
		rw_rf_reg(1, 26, &phy_val2);
		
		// reset EPHY
		val = sysRegRead(RSTCTRL);
		val = val | RALINK_EPHY_RST;
		sysRegWrite(RSTCTRL, val);
		val = val & ~(RALINK_EPHY_RST);
		sysRegWrite(RSTCTRL, val);
		
		rw_rf_reg(1, 26, &phy_val);
		
		//select local register
		mii_mgr_write(0, 31, 0x8000);
		for(i=0;i<5;i++){
			mii_mgr_write(i, 26, 0x1600);   //TX10 waveform coefficient //LSB=0 disable PHY
			mii_mgr_write(i, 29, 0x7058);   //TX100/TX10 AD/DA current bias
			mii_mgr_write(i, 30, 0x0018);   //TX100 slew rate control
		}
		
		//select global register
		mii_mgr_write(0, 31, 0x0);
		mii_mgr_write(0,  1, 0x4a40); //enlarge agcsel threshold 3 and threshold 2
		mii_mgr_write(0,  2, 0x6254); //enlarge agcsel threshold 5 and threshold 4
		mii_mgr_write(0,  3, 0xa17f); //enlarge agcsel threshold 6
//#define ENABLE_LDPS
#if defined (ENABLE_LDPS)
		mii_mgr_write(0, 12, 0x7eaa);
		mii_mgr_write(0, 22, 0x252f); //tune TP_IDL tail and head waveform, enable power down slew rate control
#else
		mii_mgr_write(0, 12, 0x0);
		mii_mgr_write(0, 22, 0x052f);
#endif
		mii_mgr_write(0, 14, 0x65);   //longer TP_IDL tail length
		mii_mgr_write(0, 16, 0x0684); //increased squelch pulse count threshold.
		mii_mgr_write(0, 17, 0x0fe0); //set TX10 signal amplitude threshold to minimum
		mii_mgr_write(0, 18, 0x40ba); //set squelch amplitude to higher threshold
		mii_mgr_write(0, 27, 0x2fce); //set PLL/Receive bias current are calibrated
		mii_mgr_write(0, 28, 0xc410); //change PLL/Receive bias current to internal(RT3350)
		mii_mgr_write(0, 29, 0x598b); //change PLL bias current to internal(RT3052_MP3)
		mii_mgr_write(0, 31, 0x8000); //select local register
		
		for(i=0;i<5;i++){
			//LSB=1 enable PHY
			mii_mgr_read(i, 26, &phy_val);
			phy_val |= 0x0001;
			mii_mgr_write(i, 26, phy_val);
		}
	} else {
		//select local register
		mii_mgr_write(0, 31, 0x8000);
		for(i=0;i<5;i++){
			mii_mgr_write(i, 26, 0x1600);   //TX10 waveform coefficient //LSB=0 disable PHY
			mii_mgr_write(i, 29, 0x7058);   //TX100/TX10 AD/DA current bias
			mii_mgr_write(i, 30, 0x0018);   //TX100 slew rate control
		}
		
		//select global register
		mii_mgr_write(0, 31, 0x0);
		mii_mgr_write(0,  1, 0x4a40); //enlarge agcsel threshold 3 and threshold 2
		mii_mgr_write(0,  2, 0x6254); //enlarge agcsel threshold 5 and threshold 4
		mii_mgr_write(0,  3, 0xa17f); //enlarge agcsel threshold 6
		mii_mgr_write(0, 14, 0x65);   //longer TP_IDL tail length
		mii_mgr_write(0, 16, 0x0684); //increased squelch pulse count threshold.
		mii_mgr_write(0, 17, 0x0fe0); //set TX10 signal amplitude threshold to minimum
		mii_mgr_write(0, 18, 0x40ba); //set squelch amplitude to higher threshold
		mii_mgr_write(0, 22, 0x052f); //tune TP_IDL tail and head waveform
		mii_mgr_write(0, 27, 0x2fce); //set PLL/Receive bias current are calibrated
		mii_mgr_write(0, 28, 0xc410); //change PLL/Receive bias current to internal(RT3350)
		mii_mgr_write(0, 29, 0x598b); //change PLL bias current to internal(RT3052_MP3)
		mii_mgr_write(0, 31, 0x8000); //select local register
		
		for(i=0;i<5;i++){
			//LSB=1 enable PHY
			mii_mgr_read(i, 26, &phy_val);
			phy_val |= 0x0001;
			mii_mgr_write(i, 26, phy_val);
		}
	}
#elif defined (CONFIG_RT3352_ASIC)
	//PHY IOT
	// reset EPHY
	val = sysRegRead(RSTCTRL);
	val = val | RALINK_EPHY_RST;
	sysRegWrite(RSTCTRL, val);
	val = val & ~(RALINK_EPHY_RST);
	sysRegWrite(RSTCTRL, val);

	//select local register
	mii_mgr_write(0, 31, 0x8000);
	for(i=0;i<5;i++){
		mii_mgr_write(i, 26, 0x1600);   //TX10 waveform coefficient //LSB=0 disable PHY
		mii_mgr_write(i, 29, 0x7016);   //TX100/TX10 AD/DA current bias
		mii_mgr_write(i, 30, 0x0038);   //TX100 slew rate control
	}

	//select global register
	mii_mgr_write(0, 31, 0x0);
	mii_mgr_write(0,  1, 0x4a40); //enlarge agcsel threshold 3 and threshold 2
	mii_mgr_write(0,  2, 0x6254); //enlarge agcsel threshold 5 and threshold 4
	mii_mgr_write(0,  3, 0xa17f); //enlarge agcsel threshold 6
	mii_mgr_write(0, 12, 0x7eaa);
	mii_mgr_write(0, 14, 0x65);   //longer TP_IDL tail length
	mii_mgr_write(0, 16, 0x0684); //increased squelch pulse count threshold.
	mii_mgr_write(0, 17, 0x0fe0); //set TX10 signal amplitude threshold to minimum
	mii_mgr_write(0, 18, 0x40ba); //set squelch amplitude to higher threshold
	mii_mgr_write(0, 22, 0x253f); //tune TP_IDL tail and head waveform, enable power down slew rate control
	mii_mgr_write(0, 27, 0x2fda); //set PLL/Receive bias current are calibrated
	mii_mgr_write(0, 28, 0xc410); //change PLL/Receive bias current to internal(RT3350)
	mii_mgr_write(0, 29, 0x598b); //change PLL bias current to internal(RT3052_MP3)
	mii_mgr_write(0, 31, 0x8000); //select local register

	for(i=0;i<5;i++){
		//LSB=1 enable PHY
		mii_mgr_read(i, 26, &phy_val);
		phy_val |= 0x0001;
		mii_mgr_write(i, 26, phy_val);
	}

#elif defined (CONFIG_RT5350_ASIC)
	//PHY IOT
	// reset EPHY
	val = sysRegRead(RSTCTRL);
	val = val | RALINK_EPHY_RST;
	sysRegWrite(RSTCTRL, val);
	val = val & ~(RALINK_EPHY_RST);
	sysRegWrite(RSTCTRL, val);

	//select local register
	mii_mgr_write(0, 31, 0x8000);
	for(i=0;i<5;i++){
		mii_mgr_write(i, 26, 0x1600);   //TX10 waveform coefficient //LSB=0 disable PHY
		mii_mgr_write(i, 29, 0x7015);   //TX100/TX10 AD/DA current bias
		mii_mgr_write(i, 30, 0x0038);   //TX100 slew rate control
	}

	//select global register
	mii_mgr_write(0, 31, 0x0);
	mii_mgr_write(0,  1, 0x4a40); //enlarge agcsel threshold 3 and threshold 2
	mii_mgr_write(0,  2, 0x6254); //enlarge agcsel threshold 5 and threshold 4
	mii_mgr_write(0,  3, 0xa17f); //enlarge agcsel threshold 6
	mii_mgr_write(0, 12, 0x7eaa);
	mii_mgr_write(0, 14, 0x65);   //longer TP_IDL tail length
	mii_mgr_write(0, 16, 0x0684); //increased squelch pulse count threshold.
	mii_mgr_write(0, 17, 0x0fe0); //set TX10 signal amplitude threshold to minimum
	mii_mgr_write(0, 18, 0x40ba); //set squelch amplitude to higher threshold
	mii_mgr_write(0, 22, 0x253f); //tune TP_IDL tail and head waveform, enable power down slew rate control
	mii_mgr_write(0, 27, 0x2fda); //set PLL/Receive bias current are calibrated
	mii_mgr_write(0, 28, 0xc410); //change PLL/Receive bias current to internal(RT3350)
	mii_mgr_write(0, 29, 0x598b); //change PLL bias current to internal(RT3052_MP3)
	mii_mgr_write(0, 31, 0x8000); //select local register

	for(i=0;i<5;i++){
		//LSB=1 enable PHY
		mii_mgr_read(i, 26, &phy_val);
		phy_val |= 0x0001;
		mii_mgr_write(i, 26, phy_val);
	}
#else
#error "Chip is not supported"
#endif

}
#endif


void fe_phy_init(void)
{
#if defined (CONFIG_GIGAPHY) || defined (CONFIG_RAETH_ROUTER) || defined (CONFIG_100PHY)
	unsigned int regValue = 0;
#endif
#if defined (CONFIG_RALINK_VISTA_BASIC)
	int sw_id=0;
	mii_mgr_read(29, 31, &sw_id);
	is_switch_175c = (sw_id == 0x175c) ? 1:0;
#endif

	// Case1: RT305x/RT335x/MT7620 + EmbeddedSW
#if defined (CONFIG_RAETH_ESW)
#if defined(CONFIG_RALINK_MT7620)
	mt7620_gsw_init();
#elif defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350)
	rt305x_esw_init();
#endif
#if defined (CONFIG_RAETH_ESW_CONTROL)
	esw_control_post_init();
#endif
#endif

#if !defined (CONFIG_RALINK_MT7621)
	// Case2: RT288x/RT3883 GE1 + GigaPhy
#if defined (CONFIG_GE1_RGMII_AN)
	enable_auto_negotiate(1);
	init_giga_phy(1);
#endif

	// Case3: RT3883 GE2 + GigaPhy
#if defined (CONFIG_GE2_RGMII_AN)
	enable_auto_negotiate(2);
	init_giga_phy(2);
#endif

	// Case4: RT288x/RT388x GE1 + GigaSW
#if defined (CONFIG_GE1_RGMII_FORCE_1000)
	sysRegWrite(MDIO_CFG, INIT_VALUE_OF_FORCE_1000_FD);
#endif

	// Case5: RT388x GE2 + GigaSW
#if defined (CONFIG_GE2_RGMII_FORCE_1000)
	sysRegWrite(MDIO_CFG2, INIT_VALUE_OF_FORCE_1000_FD);
#endif
#endif

	// Case6: RT288x GE1 /RT388x GE1/GE2 + (10/100 Switch or 100PHY)
#if defined (CONFIG_RAETH_ROUTER) || defined (CONFIG_100PHY)
//set GMAC to MII or RvMII mode

#if defined (CONFIG_RALINK_RT3883)
	regValue = sysRegRead(SYSCFG1);
#if defined (CONFIG_GE1_MII_FORCE_100) || defined (CONFIG_GE1_MII_AN)
	regValue &= ~(0x3 << 12);
	regValue |= 0x1 << 12; // GE1 MII Mode
#elif defined (CONFIG_GE1_RVMII_FORCE_100)
	regValue &= ~(0x3 << 12);
	regValue |= 0x2 << 12; // GE1 RvMII Mode
#endif
#if defined (CONFIG_GE2_MII_FORCE_100) || defined (CONFIG_GE2_MII_AN)
	regValue &= ~(0x3 << 14);
	regValue |= 0x1 << 14; // GE2 MII Mode
#elif defined (CONFIG_GE2_RVMII_FORCE_100)
	regValue &= ~(0x3 << 14);
	regValue |= 0x2 << 14; // GE2 RvMII Mode
#endif
	sysRegWrite(SYSCFG1, regValue);
#endif

#if defined (CONFIG_GE1_MII_FORCE_100)
	sysRegWrite(MDIO_CFG, INIT_VALUE_OF_FORCE_100_FD);
#endif
#if defined (CONFIG_GE2_MII_FORCE_100)
	sysRegWrite(MDIO_CFG2, INIT_VALUE_OF_FORCE_100_FD);
#endif

	// add switch configuration here for other switch chips.
#if defined (CONFIG_GE1_MII_FORCE_100) ||  defined (CONFIG_GE2_MII_FORCE_100)
	// IC+ 175x: force IC+ switch cpu port is 100/FD
	mii_mgr_write(29, 22, 0x8420);
#endif

#if defined (CONFIG_GE1_MII_AN)
	enable_auto_negotiate(1);
#endif
#if defined (CONFIG_GE2_MII_AN)
	enable_auto_negotiate(2);
#endif

#endif // defined (CONFIG_RAETH_ROUTER) || defined (CONFIG_100PHY) //

	// Case7: MT7621 GE1/GE2
#if defined (CONFIG_RALINK_MT7621)
#if defined (CONFIG_GE1_MII_FORCE_100) || defined (CONFIG_GE1_RVMII_FORCE_100)
	sysRegWrite(RALINK_ETH_SW_BASE+0x100, 0x5e337);//(P0, Force mode, Link Up, 100Mbps, Full-Duplex, FC ON)
#elif defined (CONFIG_GE1_MII_AN) || defined (CONFIG_GE1_RGMII_AN)
	enable_auto_negotiate(1);
	init_giga_phy(1);
	sysRegWrite(RALINK_ETH_SW_BASE+0x100, 0x56300);//(P0, Auto mode)
#elif defined (CONFIG_GE1_RGMII_FORCE_1000)
	sysRegWrite(RALINK_ETH_SW_BASE+0x100, 0x5633b);//(P0, Force mode, Link Up, 1000Mbps, Full-Duplex, FC ON)
#endif

#if defined (CONFIG_GE2_MII_FORCE_100) || defined (CONFIG_GE2_RVMII_FORCE_100)
	sysRegWrite(RALINK_ETH_SW_BASE+0x200, 0x5e337);//(P0, Force mode, Link Up, 100Mbps, Full-Duplex, FC ON)
#elif defined (CONFIG_GE2_MII_AN) || defined (CONFIG_GE2_RGMII_AN)
	enable_auto_negotiate(2);
	init_giga_phy(2);
	sysRegWrite(RALINK_ETH_SW_BASE+0x200, 0x56300);//(P1, Auto mode)
#elif defined (CONFIG_GE2_RGMII_FORCE_1000)
	sysRegWrite(RALINK_ETH_SW_BASE+0x200, 0x5633b);//(P0, Force mode, Link Up, 1000Mbps, Full-Duplex, FC ON)
#endif
#endif
}

