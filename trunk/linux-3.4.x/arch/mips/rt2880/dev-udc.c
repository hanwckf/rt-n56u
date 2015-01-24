/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     Ralink USB device init.
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
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>

#include <asm/rt2880/surfboardint.h>
#include <asm/rt2880/rt_mmap.h>

#if defined(CONFIG_USB_GADGET_RT)

#define RT3XXX_UDC_MEM_START	RALINK_USB_DEV_BASE

static struct resource rt3xxx_udc_resources[] = {
	[0] = {
		.start  = RT3XXX_UDC_MEM_START,
		.end    = RT3XXX_UDC_MEM_START + 0x13ff,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = SURFBOARDINT_UDEV,
		.flags  = IORESOURCE_IRQ,
	},
};

static u64 rt3xxx_udc_dmamask = DMA_BIT_MASK(32);

static struct platform_device rt3xxx_udc_device = {
	.name		= "rt_udc",
	.id		= -1,
	.dev		= {
		.dma_mask = &rt3xxx_udc_dmamask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	},
	.num_resources	= ARRAY_SIZE(rt3xxx_udc_resources),
	.resource	= rt3xxx_udc_resources,
};

int __init init_rt3xxx_udc(void)
{
	int retval = 0;

	retval = platform_device_register(&rt3xxx_udc_device);
	if (retval != 0) {
		printk(KERN_ERR "register %s device fail!\n", "UDC");
		return retval;
	}

	return retval;
}

device_initcall(init_rt3xxx_udc);
#endif
