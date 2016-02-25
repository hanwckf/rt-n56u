#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/string.h>

#include "ra_eth_reg.h"
#include "ra_esw_reg.h"
#include "ra_rfrw.h"
#include "mii_mgr.h"
#include "ra_eth.h"
#include "ra_phy.h"

#if defined (CONFIG_RAETH_ESW)
int rt305x_esw_wait_wt_mac(void)
{
	int i;
	u32 value;

	for (i = 0; i < 200; i++) {
		udelay(100);
		value = sysRegRead(RALINK_ETH_SW_BASE+REG_ESW_WT_MAC_AD0);
		if (value & 0x2)
			return 0;
	}

	return -1;
}

void rt305x_esw_mac_table_clear(int static_only)
{
	int i, j;
	u32 value, mac_esw[2];

	sysRegWrite(RALINK_ETH_SW_BASE+REG_ESW_TABLE_SEARCH, 0x1);

	for (i = 0; i < 0x400; i++) {
		for (j = 0; j < 200; j++) {
			udelay(100);
			value = sysRegRead(RALINK_ETH_SW_BASE+REG_ESW_TABLE_STATUS0);
			if (value & 0x1) {
				/* check invalid entry */
				if ((value & 0x70) == 0)
					goto next_entry;
				/* skip (non)static entries */
				if (static_only) {
					if ((value & 0x70) != 0x70)
						goto next_entry;
				}
				mac_esw[0] = sysRegRead(RALINK_ETH_SW_BASE+REG_ESW_TABLE_STATUS2);
				mac_esw[1] = sysRegRead(RALINK_ETH_SW_BASE+REG_ESW_TABLE_STATUS1) & 0xffff;
				sysRegWrite(RALINK_ETH_SW_BASE+REG_ESW_WT_MAC_AD2, mac_esw[0]);
				sysRegWrite(RALINK_ETH_SW_BASE+REG_ESW_WT_MAC_AD1, mac_esw[1]);
				sysRegWrite(RALINK_ETH_SW_BASE+REG_ESW_WT_MAC_AD0, (value & 0x780) | 0x01);
				
				rt305x_esw_wait_wt_mac();
				
				if (value & 0x2) {
					/* end of table */
					return;
				}
				break;
			}
			else if (value & 0x2) {
				/* end of table */
				return;
			}
		}
next_entry:
		sysRegWrite(RALINK_ETH_SW_BASE+REG_ESW_TABLE_SEARCH, 0x2);
	}
}

static void esw_ephy_reset(void)
{
	/* reset EPHY */
	u32 val = sysRegRead(REG_RSTCTRL);

	val |= RALINK_EPHY_RST;
	sysRegWrite(REG_RSTCTRL, val);
	udelay(10);

	val &= ~(RALINK_EPHY_RST);
	sysRegWrite(REG_RSTCTRL, val);
	udelay(1000);
}

#if defined (CONFIG_RT3052_ASIC)
static void rt3052_ephy_init(void)
{
	int i;
	u32 phy_val, phy_val2;

	rw_rf_reg(0, 0, &phy_val);
	phy_val = phy_val >> 4;

	if (phy_val > 0x5) {
		rw_rf_reg(0, 26, &phy_val);
		phy_val2 = (phy_val | (0x3 << 5));
		rw_rf_reg(1, 26, &phy_val2);
		
		// reset EPHY
		esw_ephy_reset();
		
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
}
#endif

#if defined (CONFIG_RT3352_ASIC)
static void rt3352_ephy_init(void)
{
	int i;
	u32 phy_val;

	// reset EPHY
	esw_ephy_reset();

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
}
#endif

#if defined (CONFIG_RT5350_ASIC)
static void rt5350_ephy_init(void)
{
	int i;
	u32 phy_val;

	// reset EPHY
	esw_ephy_reset();

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
}
#endif

#if defined (CONFIG_MT7628_ASIC)
static void mt7628_ephy_init(void)
{
	int i;
	u32 phy_val;

	phy_val = sysRegRead(REG_AGPIOCFG);
	phy_val &= ~(MT7628_P0_EPHY_AIO_EN);
#if defined (CONFIG_RAETH_ESW_ONE_PORT)
	phy_val |=  (MT7628_P1_EPHY_AIO_EN | MT7628_P2_EPHY_AIO_EN | MT7628_P3_EPHY_AIO_EN | MT7628_P4_EPHY_AIO_EN);
#else
	phy_val &= ~(MT7628_P1_EPHY_AIO_EN | MT7628_P2_EPHY_AIO_EN | MT7628_P3_EPHY_AIO_EN | MT7628_P4_EPHY_AIO_EN);
#endif
	sysRegWrite(REG_AGPIOCFG, phy_val);

	// reset EPHY
	esw_ephy_reset();

	phy_val = sysRegRead(RALINK_REG_GPIOMODE2);
	phy_val &= 0xf003f003;
#if defined (CONFIG_RAETH_ESW_ONE_PORT)
	phy_val |= 0x05500550; // set P0 EPHY LED mode
#endif
	sysRegWrite(RALINK_REG_GPIOMODE2, phy_val);

	udelay(5000);

	mii_mgr_write(0, 31, 0x2000);		// change G2 page
	mii_mgr_write(0, 26, 0x0000);

	for(i=0; i<5; i++) {
		mii_mgr_write(i, 31, 0x8000);	// change L0 page
#if !defined (CONFIG_RAETH_ESW_CONTROL)
		/* disable EEE */
		mii_mgr_write_cl45(i, 0x07, 0x3c, 0x0000);
#endif
		mii_mgr_write(i, 30, 0xa000);
		mii_mgr_write(i, 31, 0xa000);	// change L2 page
		mii_mgr_write(i, 16, 0x0606);
		mii_mgr_write(i, 23, 0x0f0e);
		mii_mgr_write(i, 24, 0x1610);
		mii_mgr_write(i, 30, 0x1f15);
		mii_mgr_write(i, 28, 0x6111);

		mii_mgr_read(i, 4, &phy_val);
		phy_val |= (1 << 10);
		mii_mgr_write(i, 4, phy_val);
	}

	/* 100Base AOI setting */
	mii_mgr_write(0, 31, 0x5000);		// change G5 page
	mii_mgr_write(0, 19, 0x004a);
	mii_mgr_write(0, 20, 0x015a);
	mii_mgr_write(0, 21, 0x00ee);
	mii_mgr_write(0, 22, 0x0033);
	mii_mgr_write(0, 23, 0x020a);
	mii_mgr_write(0, 24, 0x0000);
	mii_mgr_write(0, 25, 0x024a);
	mii_mgr_write(0, 26, 0x035a);
	mii_mgr_write(0, 27, 0x02ee);
	mii_mgr_write(0, 28, 0x0233);
	mii_mgr_write(0, 29, 0x000a);
	mii_mgr_write(0, 30, 0x0000);

	/* Fix EPHY idle state abnormal behavior */
	mii_mgr_write(0, 31, 0x4000);		// change G4 page
	mii_mgr_write(0, 29, 0x000d);
	mii_mgr_write(0, 30, 0x0500);
}

void mt7628_esw_eee_enable(int is_eee_enabled)
{
	u32 i;

	for (i = 0; i <= 4; i++) {
		/* EEE 100 LPI */
		mii_mgr_write_cl45(i, 0x07, 0x003c, (is_eee_enabled) ? 0x0002 : 0x0000);
	}
}
#endif
#endif /* CONFIG_RAETH_ESW */

void rt305x_esw_init(void)
{
#if defined (CONFIG_RT5350_ASIC)
	sysRegWrite(RALINK_ETH_SW_BASE+0x0168, 0x17);
#endif

	/*
	 * FC_RLS_TH=200, FC_SET_TH=160
	 * DROP_RLS=120, DROP_SET_TH=80
	 */
	sysRegWrite(RALINK_ETH_SW_BASE+0x0008, 0xC8A07850);
	sysRegWrite(RALINK_ETH_SW_BASE+0x00E4, 0x0000003f);
	sysRegWrite(RALINK_ETH_SW_BASE+0x0014, 0x00405555);
	sysRegWrite(RALINK_ETH_SW_BASE+0x0050, 0x00002001);
	sysRegWrite(RALINK_ETH_SW_BASE+0x0090, 0x00007f7f);
	sysRegWrite(RALINK_ETH_SW_BASE+0x0098, 0x00007f3f);
	sysRegWrite(RALINK_ETH_SW_BASE+0x00CC, 0x0002500c);
	sysRegWrite(RALINK_ETH_SW_BASE+0x009C, 0x0008a301); // hashing algorithm=XOR48, pkt_max_len: 1536, aging interval=300sec

	sysRegWrite(RALINK_ETH_SW_BASE+0x008C, 0x02404040);
	sysRegWrite(RALINK_ETH_SW_BASE+0x00C8, 0x3f502b28); //Change polling Ext PHY Addr=0x1F
	sysRegWrite(RALINK_ETH_SW_BASE+0x0084, 0x00000000);
	sysRegWrite(RALINK_ETH_SW_BASE+0x0110, 0x7d000000); //1us cycle number=125 (FE's clock=125Mhz)

	/*
	 * set port 5 force to 1000M/Full when connecting to switch or iNIC
	 */
#if defined (CONFIG_P5_RGMII_TO_MAC_MODE)
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~(1 << 9);		// set RGMII to Normal mode
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1<<29);	// disable port 5 auto-polling
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) |= 0x3fff;		// force 1000M full duplex
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(0xf<<20);	// rxclk_skew, txclk_skew = 0
#elif defined (CONFIG_P5_MII_TO_MAC_MODE)
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~(1 << 9);		// set RGMII to Normal mode
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1<<29);	// disable port 5 auto-polling
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(0x3fff);
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) |= 0x3ffd;		// force 100M full duplex
#if defined (CONFIG_RALINK_RT3352)
	ge1_set_mode(1, 0);
#endif
#elif defined (CONFIG_P5_MAC_TO_PHY_MODE)
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~(1 << 9);		// set RGMII to Normal mode
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~RALINK_GPIOMODE_MDIO;// set MDIO to Normal mode
	ext_gphy_init(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR);
#if defined (CONFIG_RT3052_ASIC) || defined (CONFIG_RT3352_ASIC)
	enable_autopoll_phy(1);
#endif
#elif defined (CONFIG_P5_RMII_TO_MAC_MODE)
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~(1 << 9);		// set RGMII to Normal mode
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1<<29);	// disable port 5 auto-polling
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(0x3fff);
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) |= 0x3ffd;		// force 100M full duplex
#if defined (CONFIG_RALINK_RT3352)
	ge1_set_mode(2, 0);
#endif
#else // Port 5 Disabled //

#if defined (CONFIG_RALINK_RT3052)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1 << 29);	// port5 auto polling disable
	*(volatile u32 *)(RALINK_REG_GPIOMODE) |= RALINK_GPIOMODE_MDIO;	// set MDIO to GPIO mode (GPIO22-GPIO23)
	*(volatile u32 *)(RALINK_REG_GPIOMODE) |= RALINK_GPIOMODE_RGMII;// set RGMII to GPIO mode (GPIO41-GPIO50)
	*(volatile u32 *)(0xb0000674) = 0xFFF;				// GPIO41-GPIO50 output mode
	*(volatile u32 *)(0xb000067C) = 0x0;				// GPIO41-GPIO50 output low
#elif defined (CONFIG_RALINK_RT3352)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1 << 29);	// port5 auto polling disable
	*(volatile u32 *)(RALINK_REG_GPIOMODE) |= RALINK_GPIOMODE_MDIO;	// set MDIO to GPIO mode (GPIO22-GPIO23)
	*(volatile u32 *)(RALINK_REG_GPIOMODE) |= RALINK_GPIOMODE_GE1;	// set RGMII to GPIO mode (GPIO24-GPIO35)
	*(volatile u32 *)(0xb0000624) = 0xC0000000;			// GPIO22-GPIO23 output mode
	*(volatile u32 *)(0xb000062C) = 0xC0000000;			// GPIO22-GPIO23 output high
	*(volatile u32 *)(0xb000064C) = 0xFFF;				// GPIO24-GPIO35 output mode
	*(volatile u32 *)(0xb0000654) = 0xFFF;				// GPIO24-GPIO35 output high
#elif defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
	/* do nothing */
#endif
#endif // CONFIG_P5_RGMII_TO_MAC_MODE //

#if defined (CONFIG_RAETH_ESW)
#if defined (CONFIG_RT3052_ASIC)
	rt3052_ephy_init();
#elif defined (CONFIG_RT3352_ASIC)
	rt3352_ephy_init();
#elif defined (CONFIG_RT5350_ASIC)
	rt5350_ephy_init();
#elif defined (CONFIG_MT7628_ASIC)
	mt7628_ephy_init();
#endif
#endif
}

