/*
 * RT3883 OHCI HCD (Host Controller Driver) for USB.
 *
 * (C) Copyright 2009 Ralink Tech Company
 *
 * Bus Glue for Ralink OHCI controller.
 *
 * Written by YYHuang <yy_huang@ralinktech.com.tw>
 * Based on fragments of previous driver by Russell King et al.
 *
 * This file is licenced under the GPL.
 */

#include <linux/clk.h>
#include <linux/device.h>
#include <linux/signal.h>
#include <linux/platform_device.h>

/* wake_up/sleep already handled by ehci-rt3xxx.c */
#if !defined(CONFIG_RT3XXX_EHCI)

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
#endif

static int ohci_rt3xxx_start(struct usb_hcd *hcd)
{
	int ret;
	struct ohci_hcd *ohci = hcd_to_ohci(hcd);

	ret = ohci_init(ohci);
	if (ret < 0)
		return ret;

	ret = ohci_run(ohci);
	if (ret < 0) {
		ohci_stop(hcd);
		return ret;
	}

	return 0;
}

static struct hc_driver rt3xxx_ohci_hc_driver = {
	.description		= hcd_name,
	.product_desc		= "Ralink RT3xxx OHCI Controller",
	.hcd_priv_size		= sizeof(struct ohci_hcd),
	.irq			= ohci_irq,
	.flags			= HCD_USB11 | HCD_MEMORY,

	.start			= ohci_rt3xxx_start,
	.stop			= ohci_stop,
	.shutdown		= ohci_shutdown,

	.urb_enqueue		= ohci_urb_enqueue,
	.urb_dequeue		= ohci_urb_dequeue,
	.endpoint_disable	= ohci_endpoint_disable,

	.get_frame_number	= ohci_get_frame,

	.hub_status_data	= ohci_hub_status_data,
	.hub_control		= ohci_hub_control,
#ifdef	CONFIG_PM
	.bus_suspend		= ohci_bus_suspend,
	.bus_resume		= ohci_bus_resume,
#endif
	.start_port_reset	= ohci_start_port_reset,
};

static int rt3xxx_ohci_probe(struct platform_device *pdev)
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

	hcd = usb_create_hcd(&rt3xxx_ohci_hc_driver, &pdev->dev, "rt3xxx-ohci");
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

#if !defined(CONFIG_RT3XXX_EHCI)
	rt_usb_wake_up();
#endif
	ohci_hcd_init(hcd_to_ohci(hcd));

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
	dev_err(&pdev->dev, "RT3xxx OHCI init fail. %d\n", retval);
	return retval;
}

static int rt3xxx_ohci_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	usb_remove_hcd(hcd);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);

#if !defined(CONFIG_RT3XXX_EHCI)
	rt_usb_sleep();
#endif

	return 0;
}

static struct platform_driver rt3xxx_ohci_driver = {
	.probe		= rt3xxx_ohci_probe,
	.remove		= rt3xxx_ohci_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "rt3xxx-ohci",
	},
};

MODULE_ALIAS("rt3xxx-ohci");
