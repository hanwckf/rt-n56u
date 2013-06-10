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

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/platform_device.h>

#if defined(CONFIG_RT3XXX_EHCI) || defined(CONFIG_RT3XXX_OHCI) 
#define IRQ_RT3XXX_USB 18
static struct resource rt3xxx_ehci_resources[] = {
	[0] = {
		.start  = 0x101c0000,
		.end    = 0x101c0fff,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_RT3XXX_USB,
		.end    = IRQ_RT3XXX_USB,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct resource rt3xxx_ohci_resources[] = {
	[0] = {
		.start  = 0x101c1000,
		.end    = 0x101c1fff,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_RT3XXX_USB,
		.end    = IRQ_RT3XXX_USB,
		.flags  = IORESOURCE_IRQ,
	},
};


/*
 * EHCI/OHCI Host controller.
 */
static u64 rt3xxx_ehci_dmamask = ~(u32)0;
static struct platform_device rt3xxx_ehci_device = {
	.name           = "rt3xxx-ehci",
	.id             = -1,
	.dev            = {
		.dma_mask       = &rt3xxx_ehci_dmamask,
		.coherent_dma_mask  = 0xffffffff,
	},
	.num_resources  = 2,
	.resource       = rt3xxx_ehci_resources,
};

static u64 rt3xxx_ohci_dmamask = ~(u32)0;

static struct platform_device rt3xxx_ohci_device = {
	.name           = "rt3xxx-ohci",
	.id             = -1,
	.dev            = {
		.dma_mask       = &rt3xxx_ohci_dmamask,
		.coherent_dma_mask  = 0xffffffff,
	},
	.num_resources  = 2,
	.resource       = rt3xxx_ohci_resources,
};

static struct platform_device *rt3xxx_devices[] __initdata = {
	&rt3xxx_ehci_device,
	&rt3xxx_ohci_device,
};

int __init init_rt3xxx_ehci_ohci(void)
{
	printk("RT3xxx EHCI/OHCI init.\n");
	platform_add_devices(rt3xxx_devices, ARRAY_SIZE(rt3xxx_devices));
	return 0;
}

device_initcall(init_rt3xxx_ehci_ohci);
#endif	/* defined(CONFIG_RT3XXX_EHCI) || defined(CONFIG_RT3XXX_OHCI) */

