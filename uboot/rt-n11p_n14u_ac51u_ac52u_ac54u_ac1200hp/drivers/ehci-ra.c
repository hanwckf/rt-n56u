/*
 * (C) Copyright 2009, Ying Yuan Huang <yy_huang@ralinktech.com>
 * This code is based on ehci freescale driver
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <usb.h>

#include <rt_mmap.h>

#include "ehci.h"
#include "ehci-core.h"


/*
 * enter power saving mode
 */
void leave_power_saving(void)
{
        u32 val;

        val = RALINK_REG(RT2880_RSTCTRL_REG);    // toggle host & device RST bit
        val = val & ~(RALINK_UHST_RST | RALINK_UDEV_RST);
        RALINK_REG(RT2880_RSTCTRL_REG) = val;

        val = RALINK_REG(RT2880_CLKCFG1_REG);
#if defined(RT5350_ASIC_BOARD)
        val = val | (RALINK_UPHY0_CLK_EN) ;  // disable USB port0 PHY.
#else
        val = val | RALINK_UPHY0_CLK_EN | RALINK_UPHY1_CLK_EN ;  // disable USB port0 & port1 PHY.
#endif
        RALINK_REG(RT2880_CLKCFG1_REG) = val;
}


#define USB0_HOST_MODE 0x400

static int rt_set_host(void)
{
	u32 val = RALINK_REG(RT2880_SYSCFG1_REG);
	// host mode
	val |= USB0_HOST_MODE;
	RALINK_REG(RT2880_SYSCFG1_REG) = val;
}

/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */
int ehci_hcd_init(void)
{
#if defined(RT3352_ASIC_BOARD) || defined(RT3883_ASIC_BOARD) || defined(RT5350_ASIC_BOARD) || defined(MT7620_ASIC_BOARD)|| defined(MT7628_ASIC_BOARD)
	printf("*********ehci_hcd_init**********\n");
	leave_power_saving();
	mdelay(100);

	rt_set_host();
	mdelay(100);

	hccr = (struct ehci_hccr *)(0xb01c0000);
	hcor = (struct ehci_hcor *)((uint32_t) hccr + HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	printf("Mediatek/Ralink USB EHCI host init hccr %x and hcor %x hc_length %d\n", (uint32_t)hccr, (uint32_t)hcor, (uint32_t)HC_LENGTH(ehci_readl(&hccr->cr_capbase)));
	return 0;
#else
	return -1;
#endif
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(void)
{
	return 0;
}
