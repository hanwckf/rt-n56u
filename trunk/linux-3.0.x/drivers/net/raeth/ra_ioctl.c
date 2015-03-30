#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/netdevice.h>

#include "ra_ethreg.h"
#include "ra_esw_reg.h"
#include "mii_mgr.h"
#include "ra_ioctl.h"

#if defined (CONFIG_RAETH_QDMA)
#include <asm/rt2880/surfboard.h>
extern u8 M2Q_table[64];
#endif

#if defined (CONFIG_RAETH_ESW) || defined (CONFIG_MT7530_GSW)

#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || \
    defined (CONFIG_RALINK_RT5350)
static void dump_phy_reg(int port_no, int from, int to, int is_local)
{
	u32 i=0;
	u32 temp=0;

	if (is_local==0) {
		printk("Global Register\n");
		printk("===============");
		mii_mgr_write(0, 31, 0); //select global register
		for (i=from;i<=to;i++) {
			if (i%8==0)
				printk("\n");
			mii_mgr_read(port_no, i, &temp);
			printk("%02d: %04X ", i, temp);
		}
	} else {
		mii_mgr_write(0, 31, 0x8000); //select local register
		printk("\n\nLocal Register Port %d\n", port_no);
		printk("===============");
		for (i=from;i<=to;i++) {
			if (i%8==0)
				printk("\n");
			mii_mgr_read(port_no, i, &temp);
			printk("%02d: %04X ", i, temp);
		}
	}
	printk("\n");
}
#else
static void dump_phy_reg(int port_no, int from, int to, int is_local, int page_no)
{
	u32 i=0;
	u32 temp=0;
	u32 r31=0;

	if (is_local==0) {
		printk("\n\nGlobal Register Page %d\n", page_no);
		printk("===============");
		r31 |= 0 << 15; //global
		r31 |= ((page_no&0x7) << 12); //page no
		mii_mgr_write(port_no, 31, r31); //select global page x
		for (i=16;i<32;i++) {
			if (i%8==0)
				printk("\n");
			mii_mgr_read(port_no, i, &temp);
			printk("%02d: %04X ", i, temp);
		}
	} else {
		printk("\n\nLocal Register Port %d Page %d\n", port_no, page_no);
		printk("===============");
		r31 |= 1 << 15; //local
		r31 |= ((page_no&0x7) << 12); //page no
		mii_mgr_write(port_no, 31, r31); //select local page x
		for (i=16;i<32;i++) {
			if (i%8==0)
				printk("\n");
			mii_mgr_read(port_no, i, &temp);
			printk("%02d: %04X ", i, temp);
		}
	}
	printk("\n");
}
#endif
#endif

int ei_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
#if defined (CONFIG_RAETH_ESW) || defined (CONFIG_MT7530_GSW)
	esw_rate ratelimit;
	esw_reg reg;
	u32 offset = 0;
	u32 value = 0;
#endif
	ra_mii_ioctl_data mii;

	switch (cmd) {
#if defined (CONFIG_RAETH_QDMA)
		case RAETH_QDMA_REG_READ:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
			if (reg.off > REG_HQOS_MAX)
				return -EINVAL;
			reg.val = sysRegRead(RALINK_FRAME_ENGINE_BASE + QDMA_RELATED + reg.off);
			copy_to_user(ifr->ifr_data, &reg, sizeof(reg));
			break;
		case RAETH_QDMA_REG_WRITE:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
			if (reg.off > REG_HQOS_MAX)
				return -EINVAL;
			sysRegWrite(RALINK_FRAME_ENGINE_BASE + QDMA_RELATED + reg.off, reg.val);
			break;
		case RAETH_QDMA_QUEUE_MAPPING:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
			if (reg.off > 63 || reg.val > 0x0f)
				return -EINVAL;
			M2Q_table[reg.off] = reg.val;
			break;
		case RAETH_QDMA_READ_CPU_CLK:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
			reg.val = get_surfboard_sysclk();
			copy_to_user(ifr->ifr_data, &reg, sizeof(reg));
			break;
#endif
		case RAETH_MII_READ:
			copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
			mii_mgr_read(mii.phy_id, mii.reg_num, &mii.val_out);
			copy_to_user(ifr->ifr_data, &mii, sizeof(mii));
			break;
		case RAETH_MII_WRITE:
			copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
			mii_mgr_write(mii.phy_id, mii.reg_num, mii.val_in);
			break;
#if defined (CONFIG_RAETH_ESW)
		case RAETH_ESW_REG_READ:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
			if (reg.off > REG_ESW_MAX)
				return -EINVAL;
			reg.val = sysRegRead(RALINK_ETH_SW_BASE + reg.off);
			copy_to_user(ifr->ifr_data, &reg, sizeof(reg));
			break;
		case RAETH_ESW_REG_WRITE:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
			if (reg.off > REG_ESW_MAX)
				return -EINVAL;
			sysRegWrite(RALINK_ETH_SW_BASE + reg.off, reg.val);
			break;
#endif
#if defined (CONFIG_RAETH_ESW) || defined (CONFIG_MT7530_GSW)
		case RAETH_ESW_PHY_DUMP:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || \
    defined (CONFIG_RALINK_RT5350)
			if (reg.val == 32) {//dump all phy register
				/* Global Register 0~31
				 * Local Register 0~31
				 */
				dump_phy_reg(0, 0, 31, 0); //dump global register
				for(offset=0;offset<5;offset++) {
					dump_phy_reg(offset, 0, 31, 1); //dump local register
				}
			} else {
				dump_phy_reg(reg.val, 0, 31, 0); //dump global register
				dump_phy_reg(reg.val, 0, 31, 1); //dump local register
			}
#else
			/* SPEC defined Register 0~15
			 * Global Register 16~31 for each page
			 * Local Register 16~31 for each page
			 */
			printk("SPEC defined Register");
			if (reg.val == 32) {//dump all phy register
				int i = 0;
				for(i=0; i<5; i++){
					printk("\n[Port %d]===============",i);
					for(offset=0;offset<16;offset++) {
						if(offset%8==0) {
							printk("\n");
						}
						mii_mgr_read(i, offset, &value);
						printk("%02d: %04X ", offset, value);
					}
				}
			}
			else{
				printk("\n[Port %d]===============",reg.val);
				for(offset=0;offset<16;offset++) {
					if(offset%8==0) {
						printk("\n");
					}
					mii_mgr_read(reg.val, offset, &value);
					printk("%02d: %04X ", offset, value);
				}
			}
			
#if defined (CONFIG_RALINK_MT7628)
			for(offset=0;offset<7;offset++) { //global register  page 0~6
#else
			for(offset=0;offset<5;offset++) { //global register  page 0~4
#endif
				if(reg.val == 32) //dump all phy register
					dump_phy_reg(0, 16, 31, 0, offset);
				else
					dump_phy_reg(reg.val, 16, 31, 0, offset);
			}
			
			if (reg.val == 32) {//dump all phy register
#if !defined (CONFIG_RAETH_HAS_PORT4)
				for(offset=0;offset<5;offset++) { //local register port 0-port4
#else
				for(offset=0;offset<4;offset++) { //local register port 0-port3
#endif
					dump_phy_reg(offset, 16, 31, 1, 0); //dump local page 0
					dump_phy_reg(offset, 16, 31, 1, 1); //dump local page 1
					dump_phy_reg(offset, 16, 31, 1, 2); //dump local page 2
					dump_phy_reg(offset, 16, 31, 1, 3); //dump local page 3
				}
			}else {
				dump_phy_reg(reg.val, 16, 31, 1, 0); //dump local page 0
				dump_phy_reg(reg.val, 16, 31, 1, 1); //dump local page 1
				dump_phy_reg(reg.val, 16, 31, 1, 2); //dump local page 2
				dump_phy_reg(reg.val, 16, 31, 1, 3); //dump local page 3
			}
#endif
			break;

#if defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
		case RAETH_ESW_INGRESS_RATE:
			copy_from_user(&ratelimit, ifr->ifr_data, sizeof(ratelimit));
			offset = 0x11c + (4 * (ratelimit.port / 2));
			value = sysRegRead(RALINK_ETH_SW_BASE + offset);
			
			if((ratelimit.port % 2) == 0)
			{
				value &= 0xffff0000;
				if(ratelimit.on_off == 1)
				{
					value |= (ratelimit.on_off << 14);
					value |= (0x07 << 10);
					value |= ratelimit.bw;
				}
			}
			else if((ratelimit.port % 2) == 1)
			{
				value &= 0x0000ffff;
				if(ratelimit.on_off == 1)
				{
					value |= (ratelimit.on_off << 30);
					value |= (0x07 << 26);
					value |= (ratelimit.bw << 16);
				}
			}
			printk("offset = 0x%4x value=0x%x\n\r", offset, value);
			sysRegWrite(RALINK_ETH_SW_BASE + offset, value);
			break;

		case RAETH_ESW_EGRESS_RATE:
			copy_from_user(&ratelimit, ifr->ifr_data, sizeof(ratelimit));
			offset = 0x140 + (4 * (ratelimit.port / 2));
			value = sysRegRead(RALINK_ETH_SW_BASE + offset);
			
			if((ratelimit.port % 2) == 0)
			{
				value &= 0xffff0000;
				if(ratelimit.on_off == 1)
				{
					value |= (ratelimit.on_off << 12);
					value |= (0x03 << 10);
					value |= ratelimit.bw;
				}
			}
			else if((ratelimit.port % 2) == 1)
			{
				value &= 0x0000ffff;
				if(ratelimit.on_off == 1)
				{
					value |= (ratelimit.on_off << 28);
					value |= (0x03 << 26);
					value |= (ratelimit.bw << 16);
				}
			}
			printk("offset = 0x%4x value=0x%x\n\r", offset, value);
			sysRegWrite(RALINK_ETH_SW_BASE + offset, value);
			break;

#elif defined (CONFIG_RALINK_MT7620) || defined (CONFIG_MT7530_GSW)
		case RAETH_ESW_INGRESS_RATE:
			copy_from_user(&ratelimit, ifr->ifr_data, sizeof(ratelimit));
			offset = 0x1080 + (0x100 * ratelimit.port);
#if defined (CONFIG_MT7530_GSW)
			mii_mgr_read(MT7530_MDIO_ADDR, offset, &value);
#else
			value = sysRegRead(RALINK_ETH_SW_BASE + offset);
#endif
			value &= 0xffff0000;
			if(ratelimit.on_off == 1)
			{
				value |= (ratelimit.on_off << 15);
				if (ratelimit.bw < 100)
				{
					value |= (0x0 << 8);
					value |= ratelimit.bw;
				}
				else if(ratelimit.bw < 1000)
				{
					value |= (0x1 << 8);
					value |= ratelimit.bw/10;
				}
				else if(ratelimit.bw < 10000)
				{
					value |= (0x2 << 8);
					value |= ratelimit.bw/100;
				}
				else if(ratelimit.bw < 100000)
				{
					value |= (0x3 << 8);
					value |= ratelimit.bw/1000;
				}
				else
				{
					value |= (0x4 << 8);
					value |= ratelimit.bw/10000;
				}
			}
			printk("offset = 0x%4x value=0x%x\n\r", offset, value);
#if defined (CONFIG_MT7530_GSW)
			mii_mgr_write(MT7530_MDIO_ADDR, offset, value);
#else
			sysRegWrite(RALINK_ETH_SW_BASE + offset, value);
#endif
			break;

		case RAETH_ESW_EGRESS_RATE:
			copy_from_user(&ratelimit, ifr->ifr_data, sizeof(ratelimit));
			offset = 0x1040 + (0x100 * ratelimit.port);
#if defined (CONFIG_MT7530_GSW)
			mii_mgr_read(MT7530_MDIO_ADDR, offset, &value);
#else
			value = sysRegRead(RALINK_ETH_SW_BASE + offset);
#endif
			value &= 0xffff0000;
			if(ratelimit.on_off == 1)
			{
				value |= (ratelimit.on_off << 15);
				if (ratelimit.bw < 100)
				{
					value |= (0x0 << 8);
					value |= ratelimit.bw;
				}
				else if(ratelimit.bw < 1000)
				{
					value |= (0x1 << 8);
					value |= ratelimit.bw/10;
				}
				else if(ratelimit.bw < 10000)
				{
					value |= (0x2 << 8);
					value |= ratelimit.bw/100;
				}
				else if(ratelimit.bw < 100000)
				{
					value |= (0x3 << 8);
					value |= ratelimit.bw/1000;
				}
				else
				{
					value |= (0x4 << 8);
					value |= ratelimit.bw/10000;
				}
			}
			printk("offset = 0x%4x value=0x%x\n\r", offset, value);
#if defined (CONFIG_MT7530_GSW)
			mii_mgr_write(MT7530_MDIO_ADDR, offset, value);
#else
			sysRegWrite(RALINK_ETH_SW_BASE + offset, value);
#endif
			break;
#endif
#endif
		default:
			return -EOPNOTSUPP;
	}

	return 0;
}

#if defined (CONFIG_PSEUDO_SUPPORT)
int VirtualIF_ioctl(struct net_device * net_dev, struct ifreq * ifr, int cmd)
{
	ra_mii_ioctl_data mii;

	switch (cmd) {
		case RAETH_MII_READ:
			copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
			mii_mgr_read(mii.phy_id, mii.reg_num, &mii.val_out);
			copy_to_user(ifr->ifr_data, &mii, sizeof(mii));
			break;

		case RAETH_MII_WRITE:
			copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
			mii_mgr_write(mii.phy_id, mii.reg_num, mii.val_in);
			break;
		default:
			return -EOPNOTSUPP;
	}

	return 0;
}
#endif

