#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/netdevice.h>

#include "ra_eth_reg.h"
#include "ra_esw_reg.h"
#include "mii_mgr.h"
#include "ra_ioctl.h"

#if defined (CONFIG_RAETH_QDMA)
extern u8  M2Q_table[64];
extern int M2Q_wan_lan;
#endif

#if defined (CONFIG_RAETH_ESW) || defined (CONFIG_MT7530_GSW)

#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || \
    defined (CONFIG_RALINK_RT5350)
static void dump_phy_reg(int port_no, int from, int to, int is_local)
{
	u32 i=0;
	u32 temp=0;
	u32 r31=0;

	if (is_local==0) {
		printk("Global Register\n");
		r31 |= (0 << 15);  // select global register
	} else {
		printk("\nLocal Register Port %d\n", port_no);
		r31 |= (1 << 15);  // select local register
	}

	printk("===============");

	mii_mgr_write(0, 31, r31);
	for (i=from; i<=to; i++) {
		if ((i%8)==0)
			printk("\n");
		mii_mgr_read(port_no, i, &temp);
		printk("%02d: %04X ", i, temp);
	}

	printk("\n");
}
#else
static void dump_phy_reg(int port_no, int from, int to, int is_local, int page_no)
{
	u32 i=0;
	u32 temp=0;
	u32 r31=0;

#if !defined (CONFIG_MT7530_GSW)
	if (is_local==0) {
		printk("\nGlobal Register Page %d\n", page_no);
		r31 |= (0 << 15); //global
		r31 |= ((page_no&0x7) << 12); //page no
	} else
#endif
	{
		printk("\nLocal Register Port %d Page %d\n", port_no, page_no);
#if defined (CONFIG_MT7530_GSW)
		r31 |= (page_no&0x3); //page no
#else
		r31 |= (1 << 15); //local
		r31 |= ((page_no&0x7) << 12); //page no
#endif
	}

	printk("===============");

	mii_mgr_write(port_no, 31, r31);
	for (i=from; i<=to; i++) {
		if ((i%8)==0)
			printk("\n");
		mii_mgr_read(port_no, i, &temp);
		printk("%02d: %04X ", i, temp);
	}

	printk("\n");
}
#endif

static void dump_phy(u32 port_id)
{
	int i, max_port_id = 4;

#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || \
    defined (CONFIG_RALINK_RT5350)
	/* Global Register 0~31
	 * Local Register 0~31
	 */
	/* dump global register */
	dump_phy_reg(0, 0, 31, 0);
	
	/* dump local register */
	if (port_id < 5) {
		dump_phy_reg(port_id, 0, 31, 1);
	} else {
		for (i=0; i<=max_port_id; i++)
			dump_phy_reg(i, 0, 31, 1);
	}
#else
	u32 offset;
	u32 value = 0;
#if !defined (CONFIG_MT7530_GSW)
#if defined (CONFIG_RALINK_MT7628)
	u32 max_global_pn = 6;
#else
	u32 max_global_pn = 4;
#endif
#if defined (CONFIG_RAETH_HAS_PORT4)
	max_port_id = 3;
#endif
#endif

	/* SPEC defined Register 0~15
	 * Global Register 16~31 for each page
	 * Local Register 16~31 for each page
	 */
	printk("SPEC defined Register\n");
	if (port_id < 5) {
		u32 mdio_addr = (port_id > max_port_id) ? max_port_id : port_id;
		printk("[Port %d]\n", port_id);
		printk("===============");
		for (offset=0; offset<16; offset++) {
			if ((offset%8)==0)
				printk("\n");
			mii_mgr_read(mdio_addr, offset, &value);
			printk("%02d: %04X ", offset, value);
		}
		printk("\n");
	} else {
		for (i=0; i<=max_port_id; i++) {
			if (i > 0)
				printk("\n");
			printk("[Port %d]\n", i);
			printk("===============");
			for (offset=0; offset<16; offset++) {
				if ((offset%8)==0)
					printk("\n");
				mii_mgr_read(i, offset, &value);
				printk("%02d: %04X ", offset, value);
			}
			printk("\n");
		}
	}

#if !defined (CONFIG_MT7530_GSW)
	/* global register pages 0~x */
	for (offset=0; offset<=max_global_pn; offset++)
		dump_phy_reg(0, 16, 31, 0, offset);
#endif

	if (port_id < 5) {
		u32 mdio_addr = (port_id > max_port_id) ? max_port_id : port_id;
		/* local register page 0~3 */
		for (offset=0; offset<=3; offset++)
			dump_phy_reg(mdio_addr, 16, 31, 1, offset);
	} else {
		for (i=0; i<=max_port_id; i++) {
			/* local register page 0~3 */
			for (offset=0; offset<=3; offset++)
				dump_phy_reg(i, 16, 31, 1, offset);
		}
	}

#if !defined (CONFIG_MT7530_GSW)
	mii_mgr_write(1, 31, 0x8000);
#else
	mii_mgr_write(1, 31, 0);
#endif
#endif
}

static void set_rate_limit(esw_rate *ratelimit, int is_egress)
{
	u32 offset;
	u32 value = 0;

#if defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
	offset = (is_egress) ? 0x140 : 0x11c;
	offset += (4 * (ratelimit->port / 2));
	value = sysRegRead(RALINK_ETH_SW_BASE + offset);
	if ((ratelimit->port % 2) == 0) {
		value &= 0xffff0000;
		if (ratelimit->on_off == 1) {
			if (is_egress) {
				value |= (1 << 12);
				value |= (0x03 << 10);
			} else {
				value |= (1 << 14);
				value |= (0x07 << 10);
			}
			value |= ratelimit->bw;
		}
	} else if ((ratelimit->port % 2) == 1) {
		value &= 0x0000ffff;
		if (ratelimit->on_off == 1) {
			if (is_egress) {
				value |= (1 << 28);
				value |= (0x03 << 26);
			} else {
				value |= (1 << 30);
				value |= (0x07 << 26);
			}
			value |= (ratelimit->bw << 16);
		}
	}
	sysRegWrite(RALINK_ETH_SW_BASE + offset, value);
#elif defined (CONFIG_RALINK_MT7620) || defined (CONFIG_MT7530_GSW)
#if defined (CONFIG_MT7530_GSW)
	offset = (is_egress) ? 0x1040 : 0x1800;
	offset += (0x100 * ratelimit->port);
	mii_mgr_read(MT7530_MDIO_ADDR, offset, &value);
#else
	offset = (is_egress) ? 0x1040 : 0x1080;
	offset += (0x100 * ratelimit->port);
	value = sysRegRead(RALINK_ETH_SW_BASE + offset);
#endif
	value &= 0xffff0000;
	if (ratelimit->on_off == 1) {
		value |= (1 << 15);
		if (ratelimit->bw < 100) {
			value |= (0x0 << 8);
			value |= ratelimit->bw;
		} else if (ratelimit->bw < 1000) {
			value |= (0x1 << 8);
			value |= ratelimit->bw/10;
		} else if(ratelimit->bw < 10000) {
			value |= (0x2 << 8);
			value |= ratelimit->bw/100;
		} else if(ratelimit->bw < 100000) {
			value |= (0x3 << 8);
			value |= ratelimit->bw/1000;
		} else {
			value |= (0x4 << 8);
			value |= ratelimit->bw/10000;
		}
	}
#if defined (CONFIG_MT7530_GSW)
	mii_mgr_write(MT7530_MDIO_ADDR, offset, value);
#else
	sysRegWrite(RALINK_ETH_SW_BASE + offset, value);
#endif
#endif
}
#endif

int raeth_ioctl(struct ifreq *ifr, int cmd)
{
	ra_mii_ioctl_data mii;
#if defined (CONFIG_RAETH_ESW) || defined (CONFIG_MT7530_GSW)
	esw_rate ratelimit;
#endif
#if defined (CONFIG_RAETH_ESW) || defined (CONFIG_MT7530_GSW) || defined (CONFIG_RAETH_QDMA)
	esw_reg reg;
#endif

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
#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
		case RAETH_MII_READ_CL45:
			copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
			mii_mgr_read_cl45(mii.port_num, mii.dev_addr, mii.reg_addr, &mii.val_out);
			copy_to_user(ifr->ifr_data, &mii, sizeof(mii));
			break;
		case RAETH_MII_WRITE_CL45:
			copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
			mii_mgr_write_cl45(mii.port_num, mii.dev_addr, mii.reg_addr, mii.val_in);
			break;
#endif
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
			dump_phy(reg.val);
			break;
		case RAETH_ESW_INGRESS_RATE:
			copy_from_user(&ratelimit, ifr->ifr_data, sizeof(ratelimit));
			set_rate_limit(&ratelimit, 0);
			break;
		case RAETH_ESW_EGRESS_RATE:
			copy_from_user(&ratelimit, ifr->ifr_data, sizeof(ratelimit));
			set_rate_limit(&ratelimit, 1);
			break;
#endif
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
			if (reg.val > 0x0f)
				return -EINVAL;
			if (reg.off & 0x100)
				M2Q_wan_lan = 1;
			else
				M2Q_wan_lan = 0;
			reg.off &= 0x3f;
			M2Q_table[reg.off] = (u8)reg.val;
			break;
#endif
		default:
			return -EOPNOTSUPP;
	}

	return 0;
}

