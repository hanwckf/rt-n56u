#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/signal.h>
#include <linux/irq.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>

#include "ra2882ethreg.h"
#include "raether.h"
#include "ra_mac.h"

#define RF_CSR_CFG      0xb0180500
#define RF_CSR_KICK     (1<<17)
int rw_rf_reg(int write, int reg, int *data)
{
        unsigned long    rfcsr, i = 0;

        while (1) {
                rfcsr =  sysRegRead(RF_CSR_CFG);
                if (! (rfcsr & (u32)RF_CSR_KICK) )
                        break;
                if (++i > 10000) {
                        printk("Warning: Abort rw rf register: too busy\n");
                        return -1;
                }
        }

        rfcsr = (u32)(RF_CSR_KICK | ((reg&0x3f) << 8) | (*data & 0xff));
        if (write)
                rfcsr |= 0x10000;

         sysRegRead(RF_CSR_CFG) = cpu_to_le32(rfcsr);

        i = 0;
        while (1) {
                rfcsr =  sysRegRead(RF_CSR_CFG);
                if (! (rfcsr & (u32)RF_CSR_KICK) )
                        break;
                if (++i > 10000) {
                        printk("Warning: still busy\n");
                        return -1;
                }
        }

        rfcsr =  sysRegRead(RF_CSR_CFG);

        if (((rfcsr&0x1f00) >> 8) != (reg & 0x1f)) {
                printk("Error: rw register failed\n");
                return -1;
        }
        *data = (int)(rfcsr & 0xff);

        return 0;
}


#if defined (CONFIG_RT_3052_ESW)
void rt305x_esw_init(void)
{
	int i=0;
	u32 phy_val=0, val=0;
#if defined (CONFIG_RT3052_ASIC)
	u32 phy_val2;
#endif

#if defined (CONFIG_RT5350_ASIC)
	*(unsigned long *)(RALINK_ETH_SW_BASE+0x0168) = 0x17;
#endif

	/*
	 * FC_RLS_TH=200, FC_SET_TH=160
	 * DROP_RLS=120, DROP_SET_TH=80
	 */
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x0008) = 0xC8A07850;
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x00E4) = 0x00000000;
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x0014) = 0x00405555;
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x0050) = 0x00002001;
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x0090) = 0x00007f7f;
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x0098) = 0x00007f3f; //disable VLAN
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x00CC) = 0x0002500c;
#ifndef CONFIG_UNH_TEST
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x009C) = 0x0008a301; //hashing algorithm=XOR48, aging interval=300sec
#else
	/*
	 * bit[30]:1	Backoff Algorithm Option: The latest one to pass UNH test
	 * bit[29]:1	Length of Received Frame Check Enable
	 * bit[8]:0	Enable collision 16 packet abort and late collision abort
	 * bit[7:6]:01	Maximum Packet Length: 1518
	 */
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x009C) = 0x6008a241;
#endif
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x008C) = 0x02404040;
#if defined (CONFIG_RT3052_ASIC) || defined (CONFIG_RT3352_ASIC) || defined (CONFIG_RT5350_ASIC)
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x00C8) = 0x3f502b28; //Change polling Ext PHY Addr=0x1F
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x0084) = 0x00000000;
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x0110) = 0x7d000000; //1us cycle number=125 (FE's clock=125Mhz)
#elif defined (CONFIG_RT3052_FPGA) || defined (CONFIG_RT3352_FPGA) || defined (CONFIG_RT5350_FPGA)
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x00C8) = 0x00f03ff9; //polling Ext PHY Addr=0x0, force port5 as 100F/D (disable auto-polling)
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x0084) = 0xffdf1f00;
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x0110) = 0x0d000000; //1us cycle number=13 (FE's clock=12.5Mhz)

	/* In order to use 10M/Full on FPGA board. We configure phy capable to
	 * 10M Full/Half duplex, so we can use auto-negotiation on PC side */
        for(i=0;i<5;i++){
	    mii_mgr_write(i, 4, 0x0461);   //Capable of 10M Full/Half Duplex, flow control on/off
	    mii_mgr_write(i, 0, 0xB100);   //reset all digital logic, except phy_reg
	}
#endif
	
	/*
	 * set port 5 force to 1000M/Full when connecting to switch or iNIC
	 */
#if defined (CONFIG_P5_RGMII_TO_MAC_MODE)
	*(unsigned long *)(0xb0000060) &= ~(1 << 9); //set RGMII to Normal mode
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1<<29); //disable port 5 auto-polling
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x00C8) |= 0x3fff; //force 1000M full duplex
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(0xf<<20); //rxclk_skew, txclk_skew = 0
#elif defined (CONFIG_P5_MII_TO_MAC_MODE)
	*(unsigned long *)(0xb0000060) &= ~(1 << 9); //set RGMII to Normal mode
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1<<29); //disable port 5 auto-polling
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(0x3fff); 
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x00C8) |= 0x3ffd; //force 100M full duplex

#if defined (CONFIG_RALINK_RT3352)
        *(unsigned long *)(SYSCFG1) &= ~(0x3 << 12); //GE1_MODE=Mii Mode
        *(unsigned long *)(SYSCFG1) |= (0x1 << 12);
#endif

#elif defined (CONFIG_P5_MAC_TO_PHY_MODE)
	*(unsigned long *)(0xb0000060) &= ~(1 << 9); //set RGMII to Normal mode
	*(unsigned long *)(0xb0000060) &= ~(1 << 7); //set MDIO to Normal mode
#if defined (CONFIG_RT3052_ASIC) || defined (CONFIG_RT3352_ASIC)
	enable_auto_negotiate(1);
#endif
        if (isMarvellGigaPHY(1)) {
#if defined (CONFIG_RT3052_FPGA) || defined (CONFIG_RT3352_FPGA)
		mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 9, &phy_val);
		phy_val &= ~(3<<8); //turn off 1000Base-T Advertisement  (9.9=1000Full, 9.8=1000Half)
		mii_mgr_write(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 9, phy_val);
#endif
		printk("\n Reset MARVELL phy\n");
		mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 20, &phy_val);
		phy_val |= 1<<7; //Add delay to RX_CLK for RXD Outputs
		mii_mgr_write(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 20, phy_val);

		mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 0, &phy_val);
		phy_val |= 1<<15; //PHY Software Reset
	 	mii_mgr_write(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 0, phy_val);
        }
	if (isVtssGigaPHY(1)) {
		mii_mgr_write(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 31, 0x0001); //extended page
		mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 28, &phy_val);
		printk("Vitesse phy skew: %x --> ", phy_val);
		phy_val |= (0x3<<12); // RGMII RX skew compensation= 2.0 ns
		phy_val &= ~(0x3<<14);// RGMII TX skew compensation= 0 ns
		printk("%x\n", phy_val);
		mii_mgr_write(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 28, phy_val);
		mii_mgr_write(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 31, 0x0000); //main registers
        }
       
#elif defined (CONFIG_P5_RMII_TO_MAC_MODE)
	*(unsigned long *)(0xb0000060) &= ~(1 << 9); //set RGMII to Normal mode
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1<<29); //disable port 5 auto-polling
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(0x3fff); 
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x00C8) |= 0x3ffd; //force 100M full duplex
        
#if defined (CONFIG_RALINK_RT3352)
	*(unsigned long *)(SYSCFG1) &= ~(0x3 << 12); //GE1_MODE=RvMii Mode
        *(unsigned long *)(SYSCFG1) |= (0x2 << 12);
#endif
#else // Port 5 Disabled //

#if defined (CONFIG_RALINK_RT3052)
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1 << 29); //port5 auto polling disable
        *(unsigned long *)(0xb0000060) |= (1 << 7); //set MDIO to GPIO mode (GPIO22-GPIO23)
        *(unsigned long *)(0xb0000060) |= (1 << 9); //set RGMII to GPIO mode (GPIO41-GPIO50)
        *(unsigned long *)(0xb0000674) = 0xFFF; //GPIO41-GPIO50 output mode
        *(unsigned long *)(0xb000067C) = 0x0; //GPIO41-GPIO50 output low
#elif defined (CONFIG_RALINK_RT3352)
        *(unsigned long *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1 << 29); //port5 auto polling disable
	*(unsigned long *)(0xb0000060) |= (1 << 7); //set MDIO to GPIO mode (GPIO22-GPIO23)
        *(unsigned long *)(0xb0000624) = 0xC0000000; //GPIO22-GPIO23 output mode
        *(unsigned long *)(0xb000062C) = 0xC0000000; //GPIO22-GPIO23 output high
        
        *(unsigned long *)(0xb0000060) |= (1 << 9); //set RGMII to GPIO mode (GPIO24-GPIO35)
	*(unsigned long *)(0xb000064C) = 0xFFF; //GPIO24-GPIO35 output mode
        *(unsigned long *)(0xb0000654) = 0xFFF; //GPIO24-GPIO35 output high
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

