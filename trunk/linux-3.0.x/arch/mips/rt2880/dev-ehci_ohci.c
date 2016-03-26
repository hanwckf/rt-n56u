/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     EHCI/OHCI init for Ralink RT3xxx
 *
 *  Copyright 2009 Ralink Inc. (yyhuang@ralinktech.com.tw)
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 **************************************************************************
 * March 2009 YYHuang Initial Release
 **************************************************************************
 */

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#if defined(CONFIG_USB_EHCI_HCD_PLATFORM)
#include <linux/usb/ehci_pdriver.h>
#endif
#if defined(CONFIG_USB_OHCI_HCD_PLATFORM)
#include <linux/usb/ohci_pdriver.h>
#endif

#include <asm/irq.h>
#include <asm/rt2880/rt_mmap.h>

#define RT3XXX_EHCI_MEM_START	(RALINK_USB_HOST_BASE)
#define RT3XXX_OHCI_MEM_START	(RALINK_USB_HOST_BASE + 0x1000)

static struct resource rt3xxx_ehci_resources[] = {
	[0] = {
		.start  = RT3XXX_EHCI_MEM_START,
		.end    = RT3XXX_EHCI_MEM_START + 0xfff,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = SURFBOARDINT_UHST,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct resource rt3xxx_ohci_resources[] = {
	[0] = {
		.start  = RT3XXX_OHCI_MEM_START,
		.end    = RT3XXX_OHCI_MEM_START + 0xfff,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = SURFBOARDINT_UHST,
		.flags  = IORESOURCE_IRQ,
	},
};

static u64 rt3xxx_ehci_dmamask = DMA_BIT_MASK(32);
static u64 rt3xxx_ohci_dmamask = DMA_BIT_MASK(32);

#if defined(CONFIG_USB_EHCI_HCD_PLATFORM) || defined(CONFIG_USB_OHCI_HCD_PLATFORM)
static atomic_t rt3xxx_power_instance = ATOMIC_INIT(0);

#if defined (CONFIG_RALINK_MT7628)
extern void uphy_init(void);
#endif

#define SYSCFG1_REG		(RALINK_SYSCTL_BASE + 0x14)
#define RALINK_UHST_MODE	(1UL<<10)

#define CLKCFG1_REG		(RALINK_SYSCTL_BASE + 0x30)
#define RSTCTRL_REG		(RALINK_SYSCTL_BASE + 0x34)

static void rt_usb_wake_up(void)
{
	u32 val;

	/* enable PHY0/1 clock */
	val = le32_to_cpu(*(volatile u32 *)(CLKCFG1_REG));
#if defined (CONFIG_RALINK_RT5350)
	val |= (RALINK_UPHY0_CLK_EN);
#else
	val |= (RALINK_UPHY0_CLK_EN | RALINK_UPHY1_CLK_EN);
#endif
	*(volatile u32 *)(CLKCFG1_REG) = cpu_to_le32(val);

	mdelay(10);

	/* set HOST mode */
	val = le32_to_cpu(*(volatile u32 *)(SYSCFG1_REG));
#if defined (CONFIG_USB_GADGET_RT)
	val &= ~(RALINK_UHST_MODE);
#else
	val |=  (RALINK_UHST_MODE);
#endif
	*(volatile u32 *)(SYSCFG1_REG) = cpu_to_le32(val);

	mdelay(1);

	/* release reset */
	val = le32_to_cpu(*(volatile u32 *)(RSTCTRL_REG));
	val &= ~(RALINK_UHST_RST | RALINK_UDEV_RST);
	*(volatile u32 *)(RSTCTRL_REG) = cpu_to_le32(val);

	mdelay(100);

#if defined (CONFIG_RALINK_MT7628)
	uphy_init();
#endif
}

static void rt_usb_sleep(void)
{
	u32 val;

	/* raise reset */
	val = le32_to_cpu(*(volatile u32 *)(RSTCTRL_REG));
	val |= (RALINK_UHST_RST | RALINK_UDEV_RST);
	*(volatile u32 *)(RSTCTRL_REG) = cpu_to_le32(val);

	mdelay(10);

	/* disable PHY0/1 clock */
	val = le32_to_cpu(*(volatile u32 *)(CLKCFG1_REG));
#if defined (CONFIG_RALINK_RT5350)
	val &= ~(RALINK_UPHY0_CLK_EN);
#else
	val &= ~(RALINK_UPHY0_CLK_EN | RALINK_UPHY1_CLK_EN);
#endif
	*(volatile u32 *)(CLKCFG1_REG) = cpu_to_le32(val);

	udelay(10);
}

static int rt3xxx_power_on(struct platform_device *pdev)
{
	if (atomic_inc_return(&rt3xxx_power_instance) == 1)
		rt_usb_wake_up();

	return 0;
}

static void rt3xxx_power_off(struct platform_device *pdev)
{
	if (atomic_dec_return(&rt3xxx_power_instance) == 0)
		rt_usb_sleep();
}

static struct usb_ehci_pdata rt3xxx_ehci_pdata = {
	.caps_offset		= 0,
	.has_synopsys_hc_bug	= 1,
	.port_power_off		= 1,
	.power_on		= rt3xxx_power_on,
	.power_off		= rt3xxx_power_off,
};

static struct usb_ohci_pdata rt3xxx_ohci_pdata = {
	.power_on		= rt3xxx_power_on,
	.power_off		= rt3xxx_power_off,
};
#endif

static struct platform_device rt3xxx_ehci_device = {
#if defined(CONFIG_USB_EHCI_HCD_PLATFORM)
	.name		= "ehci-platform",
#else
	.name		= "rt3xxx-ehci",
#endif
	.id		= -1,
	.dev		= {
#if defined(CONFIG_USB_EHCI_HCD_PLATFORM)
		.platform_data = &rt3xxx_ehci_pdata,
#endif
		.dma_mask = &rt3xxx_ehci_dmamask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	},
	.num_resources	= ARRAY_SIZE(rt3xxx_ehci_resources),
	.resource	= rt3xxx_ehci_resources,
};

static struct platform_device rt3xxx_ohci_device = {
#if defined(CONFIG_USB_OHCI_HCD_PLATFORM)
	.name		= "ohci-platform",
#else
	.name		= "rt3xxx-ohci",
#endif
	.id		= -1,
	.dev		= {
#if defined(CONFIG_USB_OHCI_HCD_PLATFORM)
		.platform_data = &rt3xxx_ohci_pdata,
#endif
		.dma_mask = &rt3xxx_ohci_dmamask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	},
	.num_resources	= ARRAY_SIZE(rt3xxx_ohci_resources),
	.resource	= rt3xxx_ohci_resources,
};

static struct platform_device *rt3xxx_devices[] __initdata = {
	&rt3xxx_ehci_device,
	&rt3xxx_ohci_device,
};

int __init init_rt3xxx_ehci_ohci(void)
{
	int retval = 0;

	retval = platform_add_devices(rt3xxx_devices, ARRAY_SIZE(rt3xxx_devices));
	if (retval != 0) {
		printk(KERN_ERR "register %s device fail!\n", "EHCI/OHCI");
		return retval;
	}

	return retval;
}

device_initcall(init_rt3xxx_ehci_ohci);
