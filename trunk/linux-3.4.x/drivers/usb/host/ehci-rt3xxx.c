/*
 * Ralink 3XXX(3883) EHCI Host Controller Driver
 *
 * Author: Ying Yuan Huang <yyhuang@ralinktech.com.tw>
 * Based on "ehci-fsl.c" by Randy Vinson <rvinson@mvista.com>
 *
 * 2009 (c) Ralink Technology, Inc. This file is licensed under
 * the terms of the GNU General Public License version 2. This program
 * is licensed "as is" without any warranty of any kind, whether express
 * or implied.
 */

#include <linux/platform_device.h>
#include <asm/rt2880/rt_mmap.h>

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
	val |= (RALINK_UHST_MODE);
#endif
	*(volatile u32 *)(SYSCFG1_REG) = cpu_to_le32(val);

	mdelay(1);

	/* release reset */
	val = le32_to_cpu(*(volatile u32 *)(RSTCTRL_REG));
	val &= ~(RALINK_UHST_RST | RALINK_UDEV_RST);
	*(volatile u32 *)(RSTCTRL_REG) = cpu_to_le32(val);

	mdelay(100);
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

	mdelay(1);
}

static int rt3xxx_ehci_init(struct usb_hcd *hcd)
{
	int result;
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);

	hcd->has_tt = 0;
	ehci->caps = hcd->regs;
	ehci->has_synopsys_hc_bug = 1;
	ehci->ignore_oc = 1;

	result = ehci_setup(hcd);
	if (result)
		return result;

	ehci_port_power(ehci, 0);

	return result;
}

static const struct hc_driver rt3xxx_ehci_hc_driver = {
	.description		= hcd_name,
	.product_desc		= "Ralink EHCI Host Controller",
	.hcd_priv_size		= sizeof(struct ehci_hcd),
	.irq			= ehci_irq,
	.flags			= HCD_MEMORY | HCD_USB2,

	.reset			= rt3xxx_ehci_init,
	.start			= ehci_run,
	.stop			= ehci_stop,
	.shutdown		= ehci_shutdown,

	.urb_enqueue		= ehci_urb_enqueue,
	.urb_dequeue		= ehci_urb_dequeue,
	.endpoint_disable	= ehci_endpoint_disable,
	.endpoint_reset		= ehci_endpoint_reset,

	.get_frame_number	= ehci_get_frame,

	.hub_status_data	= ehci_hub_status_data,
	.hub_control		= ehci_hub_control,
#if defined(CONFIG_PM)
	.bus_suspend		= ehci_bus_suspend,
	.bus_resume		= ehci_bus_resume,
#endif
	.relinquish_port	= ehci_relinquish_port,
	.port_handed_over	= ehci_port_handed_over,

	.clear_tt_buffer_complete = ehci_clear_tt_buffer_complete,
};

static int rt3xxx_ehci_probe(struct platform_device *pdev)
{
	struct usb_hcd *hcd;
	struct resource *res;
	int irq;
	int retval;

	if (usb_disabled())
		return -ENODEV;

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		dev_err(&pdev->dev, "Found HC with no IRQ.\n");
		return -ENODEV;
	}
	irq = res->start;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "Found HC with no register addr.\n");
		return -ENODEV;
	}

	hcd = usb_create_hcd(&rt3xxx_ehci_hc_driver, &pdev->dev, "rt3xxx-ehci");
	if (!hcd) {
		retval = -ENOMEM;
		goto fail_create_hcd;
	}

	hcd->rsrc_start = res->start;
	hcd->rsrc_len = resource_size(res);

	if (!request_mem_region(hcd->rsrc_start, hcd->rsrc_len, hcd_name)) {
		dev_dbg(&pdev->dev, "controller already in use\n");
		retval = -EBUSY;
		goto fail_request_resource;
	}

	hcd->regs = ioremap_nocache(hcd->rsrc_start, hcd->rsrc_len);
	if (!hcd->regs) {
		dev_dbg(&pdev->dev, "error mapping memory\n");
		retval = -EFAULT;
		goto fail_ioremap;
	}

	// wake up usb module from power saving mode...
	rt_usb_wake_up();

	retval = usb_add_hcd(hcd, irq, IRQF_SHARED);
	if (retval)
		goto fail_add_hcd;

	return retval;

fail_add_hcd:
	iounmap(hcd->regs);
fail_ioremap:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
fail_request_resource:
	usb_put_hcd(hcd);
fail_create_hcd:
	dev_err(&pdev->dev, "RT3xxx EHCI init fail. %d\n", retval);
	return retval;
}

static int rt3xxx_ehci_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	usb_remove_hcd(hcd);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);

	rt_usb_sleep();

	return 0;
}

static struct platform_driver rt3xxx_ehci_driver = {
	.probe = rt3xxx_ehci_probe,
	.remove = rt3xxx_ehci_remove,
	.shutdown = usb_hcd_platform_shutdown,
	.driver = {
		.owner = THIS_MODULE,
		.name = "rt3xxx-ehci",
	},
};

MODULE_ALIAS("rt3xxx-ehci");
