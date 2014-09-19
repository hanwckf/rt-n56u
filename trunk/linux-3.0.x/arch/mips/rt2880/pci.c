/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     PCI init for Ralink RT2880 solution
 *
 *  Copyright 2007 Ralink Inc. (bruce_chang@ralinktech.com.tw)
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
 * May 2007 Bruce Chang
 * Initial Release
 *
 * May 2009 Bruce Chang
 * support RT2880/RT3883 PCIe
 *
 * May 2011 Bruce Chang
 * support RT6855/MT7620 PCIe
 *
 **************************************************************************
 */

#include <linux/types.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/mod_devicetable.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include <asm/pci.h>
#include <asm/io.h>
#include <asm/rt2880/eureka_ep430.h>
#include <asm/rt2880/surfboardint.h>

#ifdef CONFIG_PCI

//#define RAPCI_DEBUG

#if defined(CONFIG_RT2880_DRAM_256M)
#define BAR_MASK			0x0FFF0000
#define BAR_HIGH			0x0FFFFFFF
#elif defined(CONFIG_RT2880_DRAM_128M)
#define BAR_MASK			0x07FF0000
#define BAR_HIGH			0x07FFFFFF
#elif defined(CONFIG_RT2880_DRAM_64M)
#define BAR_MASK			0x03FF0000
#define BAR_HIGH			0x03FFFFFF
#elif defined(CONFIG_RT2880_DRAM_32M)
#define BAR_MASK			0x01FF0000
#define BAR_HIGH			0x01FFFFFF
#elif defined(CONFIG_RT2880_DRAM_16M)
#define BAR_MASK			0x00FF0000
#define BAR_HIGH			0x00FFFFFF
#else
#define BAR_MASK			0x007F0000
#define BAR_HIGH			0x007FFFFF
#endif

/*
 * These functions and structures provide the BIOS scan and mapping of the PCI
 * devices.
 */

#define RALINK_PCI_MM_MAP_BASE		0x20000000
#if defined(CONFIG_RALINK_RT2880)
#define RALINK_PCI_IO_MAP_BASE		0x00460000
#define MEMORY_BASE			0x08000000
#else
#define RALINK_PCI_IO_MAP_BASE		0x10160000
#define MEMORY_BASE			0x0
#endif

#define RALINK_SYSTEM_CONTROL_BASE	0xb0000000
#define RALINK_SYSCFG1 			*(volatile u32 *)(RALINK_SYSTEM_CONTROL_BASE + 0x14)
#define RALINK_CLKCFG1			*(volatile u32 *)(RALINK_SYSTEM_CONTROL_BASE + 0x30)
#define RALINK_RSTCTRL			*(volatile u32 *)(RALINK_SYSTEM_CONTROL_BASE + 0x34)
#define RALINK_GPIOMODE			*(volatile u32 *)(RALINK_SYSTEM_CONTROL_BASE + 0x60)
#define RALINK_PCIE_CLK_GEN		*(volatile u32 *)(RALINK_SYSTEM_CONTROL_BASE + 0x7c)
#define RALINK_PCIE_CLK_GEN1		*(volatile u32 *)(RALINK_SYSTEM_CONTROL_BASE + 0x80)
#define PPLL_CFG1			*(volatile u32 *)(RALINK_SYSTEM_CONTROL_BASE + 0x9c)
#define PPLL_DRV			*(volatile u32 *)(RALINK_SYSTEM_CONTROL_BASE + 0xa0)
//RALINK_SYSCFG1 bit
#define RALINK_PCI_HOST_MODE_EN		(1<<7)
#define RALINK_PCIE_RC_MODE_EN		(1<<8)
//RALINK_RSTCTRL bit
#define RALINK_PCIE_RST			(1<<23)
#define RALINK_PCI_RST			(1<<24)
//RALINK_CLKCFG1 bit
#define RALINK_PCI_CLK_EN		(1<<19)
#define RALINK_PCIE_CLK_EN		(1<<21)
//RALINK_GPIOMODE bit
#define PCI_SLOTx2			(1<<11)
#define PCI_SLOTx1			(2<<11)
//MTK PCIE PLL bit
#define PDRV_SW_SET			(1<<31)
#define LC_CKDRVPD			(1<<19)
#define LC_CKDRVOHZ			(1<<18)
#define LC_CKDRVHZ			(1<<17)

#define PCI_ACCESS_READ_1		0
#define PCI_ACCESS_READ_2		1
#define PCI_ACCESS_READ_4		2
#define PCI_ACCESS_WRITE_1		3
#define PCI_ACCESS_WRITE_2		4
#define PCI_ACCESS_WRITE_4		5

static int config_access(int access_type, u32 busn, u32 slot, u32 func, u32 where, u32 *data)
{
	u32 address_reg, data_reg, address;

	address_reg = RALINK_PCI_CONFIG_ADDR;
	data_reg = RALINK_PCI_CONFIG_DATA_VIRTUAL_REG;

#if defined(CONFIG_RALINK_RT3883)
	if (busn == 0)
		where &= 0xff; // high bits used only for RT3883 PCIe bus (busn 1)
#endif

	/* Setup address */
#if defined(CONFIG_RALINK_RT3883) || defined(CONFIG_RALINK_MT7620) || defined(CONFIG_RALINK_MT7621)
	address = 0x80000000 | (((where & 0xf00)>>8)<<24) | (busn << 16) | (slot << 11) | (func << 8) | (where & 0xfc);
#else
	address = 0x80000000 | (busn << 16) | (slot << 11) | (func << 8) | (where & 0xfc);
#endif
	/* start the configuration cycle */
	MV_WRITE(address_reg, address);

	switch (access_type) {
	case PCI_ACCESS_WRITE_1:
		MV_WRITE_8(data_reg + (where & 0x3), *data);
		break;
	case PCI_ACCESS_WRITE_2:
		MV_WRITE_16(data_reg + (where & 0x3), *data);
		break;
	case PCI_ACCESS_WRITE_4:
		MV_WRITE(data_reg, *data);
		break;
	case PCI_ACCESS_READ_1:
		MV_READ_8(data_reg + (where & 0x3), data);
		break;
	case PCI_ACCESS_READ_2:
		MV_READ_16(data_reg + (where & 0x3), data);
		break;
	case PCI_ACCESS_READ_4:
		MV_READ(data_reg, data);
		break;
	}

	return PCIBIOS_SUCCESSFUL;
}

static int ralink_pci_config_read(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 *val)
{
	u32 busn = bus->number;
	u32 slot = PCI_SLOT(devfn);
	u32 func = PCI_FUNC(devfn);

	switch (size) {
	case 1:
		return config_access(PCI_ACCESS_READ_1, busn, slot, func, (u32)where, val);
	case 2:
		return config_access(PCI_ACCESS_READ_2, busn, slot, func, (u32)where, val);
	default:
		return config_access(PCI_ACCESS_READ_4, busn, slot, func, (u32)where, val);
	}
}

static int ralink_pci_config_write(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 val)
{
	u32 busn = bus->number;
	u32 slot = PCI_SLOT(devfn);
	u32 func = PCI_FUNC(devfn);

	switch (size) {
	case 1:
		return config_access(PCI_ACCESS_WRITE_1, busn, slot, func, (u32)where, &val);
	case 2:
		return config_access(PCI_ACCESS_WRITE_2, busn, slot, func, (u32)where, &val);
	default:
		return config_access(PCI_ACCESS_WRITE_4, busn, slot, func, (u32)where, &val);
	}
}

/*
 *  General-purpose PCI functions.
 */

struct pci_ops ralink_pci_ops = {
	.read = ralink_pci_config_read,
	.write = ralink_pci_config_write,
};

static struct resource ralink_res_pci_mem1 = {
	.name = "PCI MEM1",
	.start = RALINK_PCI_MM_MAP_BASE,
	.end = (RALINK_PCI_MM_MAP_BASE + 0x0fffffff),
	.flags = IORESOURCE_MEM,
};

static struct resource ralink_res_pci_io1 = {
	.name = "PCI I/O1",
	.start = RALINK_PCI_IO_MAP_BASE,
	.end = (RALINK_PCI_IO_MAP_BASE + 0x0ffff),
	.flags = IORESOURCE_IO,
};

struct pci_controller ralink_pci_controller = {
	.pci_ops = &ralink_pci_ops,
	.mem_resource = &ralink_res_pci_mem1,
	.io_resource = &ralink_res_pci_io1,
	.mem_offset = 0x00000000UL,
	.io_offset = 0x00000000UL,
};

struct pci_fixup pcibios_fixups[] = {
	{0}
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)
int __init pcibios_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
#else
int __init pcibios_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
#endif
{
	int pci_irq;
	u16 cmd;
	u8 pci_latency, pci_cache_line;
#ifdef RAPCI_DEBUG
	struct resource *res;
	int i;
	u32 val;
#endif

	pci_irq = 0;
	pci_latency = 0xff;
	pci_cache_line = (L1_CACHE_BYTES >> 2);

#ifdef RAPCI_DEBUG
	printk("** bus = %d, slot = 0x%x\n", dev->bus->number, slot);
#endif

#if defined(CONFIG_RALINK_MT7620) || defined(CONFIG_RALINK_MT7621)
	if ((dev->bus->number == 0) && (slot == 0x0)) {
		pci_write_config_dword(dev, PCI_BASE_ADDRESS_0, MEMORY_BASE);
#ifdef RAPCI_DEBUG
		pci_read_config_dword(dev, PCI_IO_BASE, &val);
		printk("PCI_IO_BASE: 0x%08X\n", val);
#endif
	} else if ((dev->bus->number == 1) && (slot == 0x0)) {
		pci_irq = SURFBOARDINT_PCIE_0;
		pci_cache_line = 0; // not available for PCIe
	} else {
		return 0;
	}
#elif defined(CONFIG_RALINK_RT3883)
	if ((dev->bus->number == 0) && (slot == 0x0)) {
		pci_write_config_dword(dev, PCI_BASE_ADDRESS_0, MEMORY_BASE);
#if defined(CONFIG_PCIE_ONLY)
		pci_write_config_dword(dev, PCI_IO_BASE, 0x00000101);
#ifdef RAPCI_DEBUG
		pci_read_config_dword(dev, PCI_IO_BASE, &val);
		printk("PCI_IO_BASE: 0x%08X\n", val);
#endif
#endif
	} else if ((dev->bus->number == 0) && (slot == 0x1)) {
		pci_write_config_dword(dev, PCI_IO_BASE, 0x00000101);
#ifdef RAPCI_DEBUG
		pci_read_config_dword(dev, PCI_IO_BASE, &val);
		printk("PCI_IO_BASE: 0x%08X\n", val);
#endif
	} else if ((dev->bus->number == 0) && (slot == 0x11)) {
		pci_irq = SURFBOARDINT_PCI_0;
		pci_latency = 64;
	} else if ((dev->bus->number == 0) && (slot == 0x12)) {
		pci_irq = SURFBOARDINT_PCI_1;
		pci_latency = 64;
	} else if ((dev->bus->number == 1)) {
		pci_irq = SURFBOARDINT_PCIE_0;
		pci_cache_line = 0; // not available for PCIe
	} else {
		return 0;
	}
#elif defined(CONFIG_RALINK_RT2880)
	if (slot == 0x0) {
		pci_write_config_dword(dev, PCI_BASE_ADDRESS_0, MEMORY_BASE);
	} else if (slot == 0x11) {
		pci_irq = SURFBOARDINT_PCI_0;
		pci_latency = 64;
	} else if (slot == 0x12) {
		pci_irq = SURFBOARDINT_PCI_1;
		pci_latency = 64;
	} else {
		return 0;
	}
#endif

	if (pci_cache_line)
		pci_write_config_byte(dev, PCI_CACHE_LINE_SIZE, pci_cache_line);

	pci_read_config_word(dev, PCI_COMMAND, &cmd);
	cmd |= (PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY);
#if defined(CONFIG_RALINK_RT2880)
	cmd |= (PCI_COMMAND_INVALIDATE | PCI_COMMAND_FAST_BACK | PCI_COMMAND_SERR | PCI_COMMAND_WAIT | PCI_COMMAND_PARITY);
#endif
	pci_write_config_word(dev, PCI_COMMAND, cmd);

	pci_write_config_byte(dev, PCI_LATENCY_TIMER, pci_latency);
	pci_write_config_byte(dev, PCI_INTERRUPT_LINE, (u8)pci_irq);

#ifdef RAPCI_DEBUG
	pci_read_config_byte(dev, PCI_CACHE_LINE_SIZE, &pci_cache_line);
	printk("PCI_CACHE_LINE_SIZE = %d\n", (pci_cache_line << 2));

	pci_read_config_byte(dev, PCI_LATENCY_TIMER, &pci_latency);
	printk("PCI_LATENCY_TIMER = %d\n", pci_latency);

	for (i = 0; i < 2; i++) {
		res = (struct resource*)&dev->resource[i];
		printk("res[%d]->start = %x\n", i, res->start);
		printk("res[%d]->end = %x\n", i, res->end);
	}
#endif

	return pci_irq;
}

int __init init_ralink_pci(void)
{
	u32 val = 0;
#if defined(CONFIG_RALINK_RT3883) || defined(CONFIG_RALINK_MT7620) || defined(CONFIG_RALINK_MT7621)
	int pcie0_disable = 0;
#endif

	PCIBIOS_MIN_IO = 0;
	PCIBIOS_MIN_MEM = 0;

#if defined(CONFIG_RALINK_MT7620)
	RALINK_GPIOMODE &= ~(0x3<<16);		// PERST_GPIO_MODE = 2'b00

	/* enable it since bsp will disable by default for power saving */
	RALINK_RSTCTRL &= ~RALINK_PCIE0_RST;
	RALINK_CLKCFG1 |=  RALINK_PCIE0_CLK_EN;

	mdelay(100);

	if ( !(PPLL_CFG1 & (1<<23)) ) {
		printk("MT7620 PPLL unlock, cannot enable PCIe!\n");
		/* for power saving */
		RALINK_RSTCTRL |=  RALINK_PCIE0_RST;
		RALINK_CLKCFG1 &= ~RALINK_PCIE0_CLK_EN;
		return 0;
	}

	PPLL_DRV |=  LC_CKDRVPD;		// PCIe clock driver power ON
	PPLL_DRV &= ~LC_CKDRVOHZ;		// Reference PCIe Output clock mode enable
	PPLL_DRV &= ~LC_CKDRVHZ;		// PCIe PHY clock enable
	PPLL_DRV |=  PDRV_SW_SET;		// PDRV SW Set

	mdelay(100);

	RALINK_SYSCFG1 |= RALINK_PCIE_RC_MODE_EN;	// PCIe in RC mode

	RALINK_PCI_PCICFG_ADDR &= ~(1<<1);	// release PCIRST
#elif defined(CONFIG_RALINK_MT7621)

	RALINK_SYSCFG1 |= RALINK_PCIE_RC_MODE_EN;	// PCIe in RC mode

#elif defined(CONFIG_RALINK_RT3883)

#if defined(CONFIG_PCIE_ONLY) || defined(CONFIG_PCIE_PCI_CONCURRENT)
	RALINK_RSTCTRL |= RALINK_PCIE_RST;
	RALINK_SYSCFG1 &= ~(0x30);
	RALINK_SYSCFG1 |= (2 << 4);
	RALINK_PCIE_CLK_GEN &= 0x7fffffff;
	RALINK_PCIE_CLK_GEN1 &= 0x80ffffff;
	RALINK_PCIE_CLK_GEN1 |= (0xa << 24);
	RALINK_PCIE_CLK_GEN |= 0x80000000;
	mdelay(50);
	RALINK_RSTCTRL &= ~RALINK_PCIE_RST;
#endif
#if defined(CONFIG_PCI_ONLY) || defined(CONFIG_PCIE_PCI_CONCURRENT)
	RALINK_GPIOMODE = (((RALINK_GPIOMODE) & ~(0x1800)) | PCI_SLOTx2);	// enable PCI slot 1, disable PCI slot 2
#endif
	RALINK_SYSCFG1 |= (RALINK_PCI_HOST_MODE_EN | RALINK_PCIE_RC_MODE_EN);	// PCI in host mode, PCIe in RC mode
#if defined(CONFIG_PCI_ONLY)
	RALINK_RSTCTRL |=  RALINK_PCIE_RST;
	RALINK_CLKCFG1 &= ~RALINK_PCIE_CLK_EN;
#elif defined(CONFIG_PCIE_ONLY)
	RALINK_RSTCTRL |=  RALINK_PCI_RST;
	RALINK_CLKCFG1 &= ~RALINK_PCI_CLK_EN;
#endif
	mdelay(200);

#if defined(CONFIG_PCIE_ONLY)
	RALINK_PCI_PCICFG_ADDR = 0;		// virtual P2P bridge DEVNUM = 0, release PCIRST
#else
	RALINK_PCI_PCICFG_ADDR = (1 << 16);	// virtual P2P bridge DEVNUM = 1, release PCIRST
#endif

#elif defined(CONFIG_RALINK_RT2880)
	RALINK_PCI_PCICFG_ADDR = 0;		// release PCIRST
#endif

	mdelay(500);

#if defined(CONFIG_RALINK_MT7620) || defined(CONFIG_RALINK_MT7621)
	if ((RALINK_PCI0_STATUS & 0x1) == 0) {
		RALINK_RSTCTRL |=  RALINK_PCIE0_RST;
		RALINK_CLKCFG1 &= ~RALINK_PCIE0_CLK_EN;
		PPLL_DRV &= ~LC_CKDRVPD;
		PPLL_DRV |=  PDRV_SW_SET;
		printk("PCIe0: no card, disable it (RST&CLK)\n");
		pcie0_disable = 1;
		return 0;
	}
#elif defined(CONFIG_RALINK_RT3883)
#if defined(CONFIG_PCIE_ONLY) || defined(CONFIG_PCIE_PCI_CONCURRENT)
	if ((RALINK_PCI1_STATUS & 0x1) == 0) {
		RALINK_RSTCTRL |= RALINK_PCIE_RST;
		RALINK_CLKCFG1 &= ~RALINK_PCIE_CLK_EN;
		//cgrstb, cgpdb, pexdrven0, pexdrven1, cgpllrstb, cgpllpdb, pexclken
		RALINK_PCIE_CLK_GEN &= 0x0fff3f7f;
		printk("PCIe: no card, disable it (RST&CLK)\n");
		pcie0_disable = 1;
#if defined(CONFIG_PCIE_ONLY)
		return 0;
#endif
	}
#endif
	RALINK_PCI_ARBCTL = 0x79;
#elif defined(CONFIG_RALINK_RT2880)
	RALINK_PCI_ARBCTL = 0x79;
#endif

	RALINK_PCI_MEMBASE = 0xffffffff;		// valid for host mode only
	RALINK_PCI_IOBASE = RALINK_PCI_IO_MAP_BASE;	// valid for host mode only

#if defined(CONFIG_RALINK_MT7620) || defined(CONFIG_RALINK_MT7621)
	//PCIe0
	RALINK_PCI0_BAR0SETUP_ADDR  = BAR_MASK;
	RALINK_PCI0_IMBASEBAR0_ADDR = MEMORY_BASE;
	RALINK_PCI0_ID              = 0x08021814;
	RALINK_PCI0_CLASS           = 0x06040001;
	RALINK_PCI0_SUBID           = 0x63521814;
	RALINK_PCI0_BAR0SETUP_ADDR  = (BAR_MASK | 1);	// enable PCIe0 BAR0
	RALINK_PCI_PCIMSK_ADDR      = 0x00100000;	// enable PCIe0 interrupt
#elif defined(CONFIG_RALINK_RT3883)
	//PCI
	RALINK_PCI0_BAR0SETUP_ADDR  = BAR_MASK;
	RALINK_PCI0_IMBASEBAR0_ADDR = MEMORY_BASE;
	RALINK_PCI0_ID              = 0x08021814;
	RALINK_PCI0_CLASS           = 0x00800001;
	RALINK_PCI0_SUBID           = 0x38831814;
#if defined(CONFIG_PCI_ONLY) || defined(CONFIG_PCIE_PCI_CONCURRENT)
	RALINK_PCI0_BAR0SETUP_ADDR  = (BAR_MASK | 1);	// enable PCI BAR0
#endif
	//PCIe
	RALINK_PCI1_BAR0SETUP_ADDR  = BAR_MASK;
	RALINK_PCI1_IMBASEBAR0_ADDR = MEMORY_BASE;
	RALINK_PCI1_ID              = 0x08021814;
	RALINK_PCI1_CLASS           = 0x06040001;
	RALINK_PCI1_SUBID           = 0x38831814;
#if defined(CONFIG_PCIE_ONLY) || defined(CONFIG_PCIE_PCI_CONCURRENT)
	if (!pcie0_disable)
		RALINK_PCI1_BAR0SETUP_ADDR  = (BAR_MASK | 1);	// enable PCIe BAR0
#endif
#if defined(CONFIG_PCIE_ONLY)
	RALINK_PCI_PCIMSK_ADDR      = 0x00100000;	// enable PCIe interrupt
#elif defined(CONFIG_PCI_ONLY)
	RALINK_PCI_PCIMSK_ADDR      = 0x000c0000;	// enable PCI interrupt
#elif defined(CONFIG_PCIE_PCI_CONCURRENT)
	RALINK_PCI_PCIMSK_ADDR      = (pcie0_disable) ? 0x000c0000 : 0x001c0000;	// enable PCI/PCIe interrupt
#endif
#elif defined(CONFIG_RALINK_RT2880)
	RALINK_PCI_BAR0SETUP_ADDR   = BAR_MASK;
	RALINK_PCI_IMBASEBAR0_ADDR  = MEMORY_BASE;
	RALINK_PCI_ID               = 0x08021814;
	RALINK_PCI_CLASS            = 0x00800001;
	RALINK_PCI_SUBID            = 0x28801814;
	RALINK_PCI_BAR0SETUP_ADDR   = (BAR_MASK | 1);	// enable PCI BAR0
	RALINK_PCI_PCIMSK_ADDR      = 0x000c0000;	// enable PCI interrupt
#endif

	val = 0;
#if defined(CONFIG_RALINK_MT7620) || defined(CONFIG_RALINK_MT7621)
	// start P2P bridge PCIe0
	config_access(PCI_ACCESS_READ_4, 0, 0x0, 0, PCI_COMMAND, &val);
	val |= PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER;
	config_access(PCI_ACCESS_WRITE_4, 0, 0x0, 0, PCI_COMMAND, &val);
#elif defined(CONFIG_RALINK_RT3883)
#if defined(CONFIG_PCIE_PCI_CONCURRENT)
	// start virtual P2P bridge
	if (!pcie0_disable) {
		config_access(PCI_ACCESS_READ_4, 0, 0x1, 0, PCI_COMMAND, &val);
		val |= PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER;
		config_access(PCI_ACCESS_WRITE_4, 0, 0x1, 0, PCI_COMMAND, &val);
	}
#endif
	// start PCI host bridge (or virtual P2P bridge for PCIe only)
	config_access(PCI_ACCESS_READ_4, 0, 0x0, 0, PCI_COMMAND, &val);
	val |= PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER;
	config_access(PCI_ACCESS_WRITE_4, 0, 0x0, 0, PCI_COMMAND, &val);
#endif
	val = MEMORY_BASE;
	config_access(PCI_ACCESS_WRITE_4, 0, 0x0, 0, PCI_BASE_ADDRESS_0, &val);

	ralink_pci_controller.io_map_base = mips_io_port_base;

	register_pci_controller(&ralink_pci_controller);

	return 0;
}

int pcibios_plat_dev_init(struct pci_dev *dev)
{
	return 0;
}

arch_initcall(init_ralink_pci);

#endif /* CONFIG_PCI */
