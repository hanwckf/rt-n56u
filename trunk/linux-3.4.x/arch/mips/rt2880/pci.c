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
 *
 **************************************************************************
 */

#include <linux/types.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <asm/pci.h>
#include <asm/io.h>
#include <asm/rt2880/eureka_ep430.h>
#include <linux/init.h>
#include <linux/mod_devicetable.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/highmem.h>

#ifdef CONFIG_PCI

/*
 * These functions and structures provide the BIOS scan and mapping of the PCI
 * devices.
 */

#define RALINK_PCI_MM_MAP_BASE	0x20000000
#if defined(CONFIG_RALINK_RT2883) || defined(CONFIG_RALINK_RT3883)
#define RALINK_PCI_IO_MAP_BASE	0x10160000
#else
#define RALINK_PCI_IO_MAP_BASE	0x00460000
#endif

#define RALINK_SYSTEM_CONTROL_BASE	0xb0000000
#define RALINK_SYSCFG1 			*(unsigned long *)(RALINK_SYSTEM_CONTROL_BASE + 0x14)
#define RALINK_CLKCFG1			*(unsigned long *)(RALINK_SYSTEM_CONTROL_BASE + 0x30)
#define RALINK_RSTCTRL			*(unsigned long *)(RALINK_SYSTEM_CONTROL_BASE + 0x34)
#define RALINK_GPIOMODE			*(unsigned long *)(RALINK_SYSTEM_CONTROL_BASE + 0x60)
#define RALINK_PCIE_CLK_GEN		*(unsigned long *)(RALINK_SYSTEM_CONTROL_BASE + 0x7c)
#define RALINK_PCIE_CLK_GEN1		*(unsigned long *)(RALINK_SYSTEM_CONTROL_BASE + 0x80)
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

#if defined(CONFIG_RALINK_RT2883) || defined(CONFIG_RALINK_RT3883)
#define MEMORY_BASE 0x0
#else
#define MEMORY_BASE 0x08000000
#endif

//extern pci_probe_only;

void __inline__ read_config(unsigned long bus, unsigned long dev, unsigned long func, unsigned long reg, unsigned long *val);
void __inline__ write_config(unsigned long bus, unsigned long dev, unsigned long func, unsigned long reg, unsigned long val);

#define PCI_ACCESS_READ_1  0
#define PCI_ACCESS_READ_2  1
#define PCI_ACCESS_READ_4  2
#define PCI_ACCESS_WRITE_1 3
#define PCI_ACCESS_WRITE_2 4
#define PCI_ACCESS_WRITE_4 5

static int config_access(unsigned char access_type, struct pci_bus *bus, unsigned int devfn, unsigned int where, u32 * data)
{
	unsigned int slot = PCI_SLOT(devfn);
	u8 func = PCI_FUNC(devfn);
	uint32_t address_reg, data_reg;
	unsigned int address;

	address_reg = RALINK_PCI_CONFIG_ADDR;
	data_reg = RALINK_PCI_CONFIG_DATA_VIRTUAL_REG;

	if (!bus->number)
		where &= 0x1FF;

	/* Setup address */
#ifdef CONFIG_RALINK_RT2883
	address = (bus->number << 24) | (slot << 19) | (func << 16) | (where & 0xfc) | 0x1;
#elif defined(CONFIG_RALINK_RT3883)
	address = (bus->number << 16) | (slot << 11) | (func << 8) | ((where & 0xf00) << (24 - 8)) | (where & 0xfc) | 0x80000000;
#else
	address = (bus->number << 16) | (slot << 11) | (func << 8) | (where & 0xfc) | 0x80000000;
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
	default:
		printk("no specify access type\n");
		break;
	}
#ifdef DEBUG
	if (bus->number == 1 && where == 0x30) {
		printk("%x->[%x][%x][%x][%x]=%x\n", access_type, bus->number, slot, func, where, *data);
	}
#endif
	return 0;
}

static int read_config_byte(struct pci_bus *bus, unsigned int devfn, int where, u8 * val)
{
	int ret;

	ret = config_access(PCI_ACCESS_READ_1, bus, devfn, where, (u32 *) val);
	return ret;
}

static int read_config_word(struct pci_bus *bus, unsigned int devfn, int where, u16 * val)
{
	int ret;

	ret = config_access(PCI_ACCESS_READ_2, bus, devfn, where, (u32 *) val);
	return ret;
}

static int read_config_dword(struct pci_bus *bus, unsigned int devfn, int where, u32 * val)
{
	int ret;

	ret = config_access(PCI_ACCESS_READ_4, bus, devfn, where, val);
	return ret;
}

static int write_config_byte(struct pci_bus *bus, unsigned int devfn, int where, u8 val)
{
	if (config_access(PCI_ACCESS_WRITE_1, bus, devfn, where, (u32 *) & val))
		return -1;

	return PCIBIOS_SUCCESSFUL;
}

static int write_config_word(struct pci_bus *bus, unsigned int devfn, int where, u16 val)
{
	if (config_access(PCI_ACCESS_WRITE_2, bus, devfn, where, (u32 *) & val))
		return -1;

	return PCIBIOS_SUCCESSFUL;
}

static int write_config_dword(struct pci_bus *bus, unsigned int devfn, int where, u32 val)
{
	if (config_access(PCI_ACCESS_WRITE_4, bus, devfn, where, &val))
		return -1;

	return PCIBIOS_SUCCESSFUL;
}

static int pci_config_read(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 * val)
{
	switch (size) {
	case 1:
		return read_config_byte(bus, devfn, where, (u8 *) val);
	case 2:
		return read_config_word(bus, devfn, where, (u16 *) val);
	default:
		return read_config_dword(bus, devfn, where, val);
	}
}

static int pci_config_write(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 val)
{
	switch (size) {
	case 1:
		return write_config_byte(bus, devfn, where, (u8) val);
	case 2:
		return write_config_word(bus, devfn, where, (u16) val);
	default:
		return write_config_dword(bus, devfn, where, val);
	}
}

/*
 *  General-purpose PCI functions.
 */

struct pci_ops rt2880_pci_ops = {
	.read = pci_config_read,
	.write = pci_config_write,
};

static struct resource rt2880_res_pci_mem1 = {
	.name = "PCI MEM1",
	.start = RALINK_PCI_MM_MAP_BASE,
	.end = (RALINK_PCI_MM_MAP_BASE + 0x0fffffff),
	.flags = IORESOURCE_MEM,
};

static struct resource rt2880_res_pci_io1 = {
	.name = "PCI I/O1",
	.start = RALINK_PCI_IO_MAP_BASE,
	.end = (RALINK_PCI_IO_MAP_BASE + 0x0ffff),
	.flags = IORESOURCE_IO,
};

struct pci_controller rt2880_controller = {
	.pci_ops = &rt2880_pci_ops,
	.mem_resource = &rt2880_res_pci_mem1,
	.io_resource = &rt2880_res_pci_io1,
	.mem_offset = 0x00000000UL,
	.io_offset = 0x00000000UL,
};

void __inline__ read_config(unsigned long bus, unsigned long dev, unsigned long func, unsigned long reg, unsigned long *val)
{
	unsigned long address_reg, data_reg, address;

	address_reg = RALINK_PCI_CONFIG_ADDR;
	data_reg = RALINK_PCI_CONFIG_DATA_VIRTUAL_REG;

	/* set addr */
#ifdef CONFIG_RALINK_RT2883
	address = (bus << 24) | (dev << 19) | (func << 16) | (reg & 0xfc);
#elif defined(CONFIG_RALINK_RT3883)
	address = (bus << 16) | (dev << 11) | (func << 8) | (reg & 0xfc) | 0x80000000;
#else
	address = (bus << 16) | (dev << 11) | (func << 8) | (reg & 0xfc) | 0x80000000;
#endif
	/* start the configuration cycle */
	MV_WRITE(address_reg, address);
	/* read the data */
	MV_READ(data_reg, val);
	return;
}

void __inline__ write_config(unsigned long bus, unsigned long dev, unsigned long func, unsigned long reg, unsigned long val)
{
	unsigned long address_reg, data_reg, address;

	address_reg = RALINK_PCI_CONFIG_ADDR;
	data_reg = RALINK_PCI_CONFIG_DATA_VIRTUAL_REG;

	/* set addr */
#ifdef CONFIG_RALINK_RT2883
	address = (bus << 24) | (dev << 19) | (func << 16) | (reg & 0xfc);
#elif defined(CONFIG_RALINK_RT3883)
	address = (bus << 16) | (dev << 11) | (func << 8) | (reg & 0xfc) | 0x80000000;
#else
	address = (bus << 16) | (dev << 11) | (func << 8) | (reg & 0xfc) | 0x80000000;
#endif
	/* start the configuration cycle */
	MV_WRITE(address_reg, address);
	/* read the data */
	MV_WRITE(data_reg, val);
	return;
}

int __init pcibios_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	u16 cmd;
	unsigned long val;
	struct resource *res;
#ifdef DEBUG
	int i;
#endif
	int irq = -1;
	unsigned int bar0setup, bar0msk, memsize_mb, s;
	unsigned long memsize;

#ifdef CONFIG_RALINK_RT2883
	if (dev->bus->number > 1) {
		return irq;
	}
	if (slot > 0) {
		return irq;
	}
#elif defined(CONFIG_RALINK_RT2880)
	if (dev->bus->number != 0) {
		return irq;
	}
#else
#endif

	bar0msk = 0x3FFF;
	memsize = (num_physpages - totalhigh_pages) << PAGE_SHIFT;
	memsize_mb = memsize >> 20;
	for (s = 1024; s > 0 && s > memsize_mb; s >>= 1, bar0msk >>= 1)
		;
	if (memsize & ((1 << 20) - 1))
		bar0msk = (bar0msk << 1) | 1;
#ifdef DEBUG
	printk("BAR0MSK 0x%04x (memory:%uMB)\n", bar0msk, memsize_mb);
#endif
	/* enable BAR0 and open it according to total memory size */
	bar0setup = (bar0msk << 16) | 1;

	//printk("** bus= %x, slot=0x%x\n",dev->bus->number,  slot);
#ifdef CONFIG_RALINK_RT3883
	if ((dev->bus->number == 0) && (slot == 0)) {
		RALINK_PCI0_BAR0SETUP_ADDR = bar0setup;
		RALINK_PCI0_BAR0SETUP_ADDR = bar0setup;
		RALINK_PCI1_BAR0SETUP_ADDR = bar0setup;
		RALINK_PCI1_BAR0SETUP_ADDR = bar0setup;
		write_config(0, 0, 0, PCI_BASE_ADDRESS_0, MEMORY_BASE);
		read_config(0, 0, 0, PCI_BASE_ADDRESS_0, &val);
#ifdef DEBUG
		printk("BAR0 at slot 0 = %lx\n", val);
		printk("bus=0, slot = 0x%x\n", slot);
#endif
		res = (struct resource*)&dev->resource[0];
		res->start = MEMORY_BASE;
		res->end = MEMORY_BASE + 0x01ffffff;
#ifdef DEBUG
		for (i = 0; i < 16; i++) {
			read_config(0, 0, 0, i << 2, &val);
			printk("P2P(PCI) 0x%02x = %08lx\n", i << 2, val);
		}
#endif
		irq = 0;
	} else if ((dev->bus->number == 0) && (slot == 0x1)) {
		write_config(0, 1, 0, 0x1c, 0x00000101);
#ifdef DEBUG
		for (i = 0; i < 16; i++) {
			read_config(0, 1, 0, i << 2, &val);
			printk("P2P(PCIe)  0x%02x = %08lx\n", i << 2, val);
		}
#endif
	} else if ((dev->bus->number == 0) && (slot == 0x11)) {
#ifdef DEBUG
		printk("bus=0, slot = 0x%x\n", slot);
		for (i = 0; i < 16; i++) {
			read_config(0, 0x11, 0, i << 2, &val);
			printk("dev I(PCI)  0x%02x = %08lx\n", i << 2, val);
		}
#endif
		irq = 2;
	} else if ((dev->bus->number == 0) && (slot == 0x12)) {
#ifdef DEBUG
		printk("bus=0, slot = 0x%x\n", slot);
		for (i = 0; i < 16; i++) {
			read_config(0, 0x12, 0, i << 2, &val);
			printk("dev II(PCI)  0x%02x = %08lx\n", i << 2, val);
		}
#endif
		irq = 15;
	} else if ((dev->bus->number == 1)) {
#ifdef DEBUG
		printk("bus=1, slot = 0x%x\n", slot);
		for (i = 0; i < 16; i++) {
			read_config(1, 0, 0, i << 2, &val);
			printk("dev III(PCIe)  0x%02x = %08lx\n", i << 2, val);
		}
#endif
		irq = 16;
	} else {
		return irq;
	}
#elif defined(CONFIG_RALINK_RT2883)
	if ((dev->bus->number == 0) && (slot == 0)) {
		RALINK_PCI_BAR0SETUP_ADDR = bar0setup;
		write_config(0, 0, 0, PCI_BASE_ADDRESS_0, MEMORY_BASE);
		read_config(0, 0, 0, PCI_BASE_ADDRESS_0, &val);
#ifdef DEBUG
		printk("BAR0 at slot 0 = %lx\n", val);
		printk("bus=0, slot = 0x%x\n", slot);
#endif
		res = &dev->resource[0];
		res->start = MEMORY_BASE;
		res->end = MEMORY_BASE + 0x01ffffff;
#ifdef DEBUG
		for (i = 0; i < 16; i++) {
			read_config(0, 0, 0, i << 2, &val);
			printk("pci-to-pci 0x%02x = %08lx\n", i << 2, val);
		}
#endif
		irq = 0;
	} else if ((dev->bus->number == 1)) {
#ifdef DEBUG
		printk("bus=1, slot = 0x%x\n", slot);
		for (i = 0; i < 16; i++) {
			read_config(1, slot, 0, (i) << 2, &val);
			printk("bus 1 dev %d fun 0: 0x%02x = %08lx\n", slot, i << 2, val);
		}
#endif
		irq = 2;
	} else {
		return irq;
	}
#else //RT2880
	if (slot == 0) {
		printk("*************************************************************\n");
		RALINK_PCI_BAR0SETUP_ADDR = bar0setup;
		printk("MEMORY_BASE = %x\n", MEMORY_BASE);
		write_config(0, 0, 0, PCI_BASE_ADDRESS_0, MEMORY_BASE);
		read_config(0, 0, 0, PCI_BASE_ADDRESS_0, &val);
		irq = 0;
		res = &dev->resource[0];
		res->start = 0x08000000;
		res->end = 0x09ffffff;
#ifdef DEBUG
		printk("BAR0 at slot 0 = %lx\n", val);
#endif
	} else if (slot == 0x11) {
		irq = 2;
	} else if (slot == 0x12) {
		irq = 15;
	} else {
		return irq;
	}
#endif

#ifdef DEBUG
	for (i = 0; i < 6; i++) {
		res = &dev->resource[i];
		printk("res[%d]->start = %x\n", i, res->start);
		printk("res[%d]->end = %x\n", i, res->end);
	}
#endif

	pci_cache_line_size = pci_dfl_cache_line_size;		/* CONFIG_MIPS_L1_CACHE_SHIFT must be configured correctly. */
	pci_write_config_byte(dev, PCI_LATENCY_TIMER, 0xFF);	//configure latency timer 0x10
	pci_read_config_word(dev, PCI_COMMAND, &cmd);
//FIXME
#if defined(CONFIG_RALINK_RT2883)
	cmd = cmd | PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY;
#elif defined(CONFIG_RALINK_RT3883)
	cmd = cmd | PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY;
#else
	cmd = cmd | PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY |
	    PCI_COMMAND_INVALIDATE | PCI_COMMAND_FAST_BACK | PCI_COMMAND_SERR | PCI_COMMAND_WAIT | PCI_COMMAND_PARITY;
#endif
	pci_write_config_word(dev, PCI_COMMAND, cmd);
	pci_write_config_byte(dev, PCI_INTERRUPT_LINE, irq);
	return irq;
}

int __devinit init_rt2880pci(void)
{
	unsigned long val = 0;
#ifdef DEBUG
	int i;
#endif
	PCIBIOS_MIN_IO = 0;
	PCIBIOS_MIN_MEM = 0;

#if defined(CONFIG_PCIE_ONLY) || defined(CONFIG_PCIE_PCI_CONCURRENT)
	RALINK_RSTCTRL = (RALINK_RSTCTRL | RALINK_PCIE_RST);
	RALINK_SYSCFG1 &= ~(0x30);
	RALINK_SYSCFG1 |= (2 << 4);
	RALINK_PCIE_CLK_GEN &= 0x7fffffff;
	RALINK_PCIE_CLK_GEN1 &= 0x80ffffff;
	RALINK_PCIE_CLK_GEN1 |= 0xa << 24;
	RALINK_PCIE_CLK_GEN |= 0x80000000;
	mdelay(50);
	RALINK_RSTCTRL = (RALINK_RSTCTRL & ~RALINK_PCIE_RST);
#endif

#ifdef CONFIG_RALINK_RT3883

#if defined(CONFIG_PCI_ONLY)
//PCI host only, 330T
	RALINK_GPIOMODE = ((RALINK_GPIOMODE & ~(0x3800)) | PCI_SLOTx2);
	RALINK_SYSCFG1 = (RALINK_SYSCFG1 | RALINK_PCI_HOST_MODE_EN | RALINK_PCIE_RC_MODE_EN);
	RALINK_RSTCTRL = (RALINK_RSTCTRL | RALINK_PCIE_RST);
	RALINK_CLKCFG1 = (RALINK_CLKCFG1 & ~RALINK_PCIE_CLK_EN);
#elif defined(CONFIG_PCIE_ONLY)
//PCIe RC only, 220T
	RALINK_SYSCFG1 = (RALINK_SYSCFG1 | RALINK_PCIE_RC_MODE_EN | RALINK_PCI_HOST_MODE_EN);
	RALINK_RSTCTRL = (RALINK_RSTCTRL | RALINK_PCI_RST);
	RALINK_CLKCFG1 = (RALINK_CLKCFG1 & ~RALINK_PCI_CLK_EN);
#elif defined(CONFIG_PCIE_PCI_CONCURRENT)
//PCIe PCI co-exist
	RALINK_GPIOMODE = ((RALINK_GPIOMODE & ~(0x3800)) | PCI_SLOTx2);
	RALINK_SYSCFG1 = (RALINK_SYSCFG1 | RALINK_PCI_HOST_MODE_EN | RALINK_PCIE_RC_MODE_EN);
#endif
	mdelay(500);
#endif

#ifdef CONFIG_RALINK_RT2880
	//pci_probe_only = 1;
	RALINK_PCI_PCICFG_ADDR = 0;
#elif defined(CONFIG_RALINK_RT2883)
	RALINK_PCI_PCICFG_ADDR = 0;
#elif defined(CONFIG_RALINK_RT3883)

#if defined(CONFIG_PCIE_ONLY)
	RALINK_PCI_PCICFG_ADDR = 0;
	//RALINK_PCI_PCICFG_ADDR |= (1<<16);
#elif defined(CONFIG_PCI_ONLY)
	RALINK_PCI_PCICFG_ADDR = 0;
	RALINK_PCI_PCICFG_ADDR |= (1 << 16);
#elif defined(CONFIG_PCIE_PCI_CONCURRENT)
	RALINK_PCI_PCICFG_ADDR = 0;
	RALINK_PCI_PCICFG_ADDR |= (1 << 16);
#endif
	mdelay(500);

#endif
#ifdef DEBUG
	printk("RALINK_PCI_PCICFG_ADDR = %x\n", RALINK_PCI_PCICFG_ADDR);
#endif
#ifdef CONFIG_RALINK_RT3883
	printk("\n*************** Ralink PCIe RC mode *************\n");
	if (RALINK_SYSCFG1 & RALINK_PCIE_RC_MODE_EN) {
		if ((RALINK_PCI1_STATUS & 0x1) == 0) {
#ifdef DEBUG
			printk(" RALINK_PCI1_STATUS = %x\n", RALINK_PCI1_STATUS);
			for (i = 0; i < 16; i++) {
				read_config(0, 1, 0, i << 2, &val);
				printk("pci-to-pci 0x%02x = %08lx\n", i << 2, val);
			}
#endif
#ifdef CONFIG_PCIE_ONLY
			printk("reset PCIe and turn off PCIe clock\n");
			RALINK_RSTCTRL = (RALINK_RSTCTRL | RALINK_PCIE_RST);
			RALINK_RSTCTRL = (RALINK_RSTCTRL & ~RALINK_PCIE_RST);
			RALINK_CLKCFG1 = (RALINK_CLKCFG1 & ~RALINK_PCIE_CLK_EN);
			printk("RALINK_CLKCFG1 = %lx\n", RALINK_CLKCFG1);
			//cgrstb, cgpdb, pexdrven0, pexdrven1, cgpllrstb, cgpllpdb, pexclken
			RALINK_PCIE_CLK_GEN &= 0x0fff3f7f;
			printk("RALINK_PCIE_CLK_GEN= %lx\n", RALINK_PCIE_CLK_GEN);
			return 0;
#else
			RALINK_CLKCFG1 = (RALINK_CLKCFG1 & ~RALINK_PCIE_CLK_EN);
#endif
		}
	}
	if (RALINK_SYSCFG1 & RALINK_PCI_HOST_MODE_EN) {
		RALINK_PCI_ARBCTL = 0x79;
	}
#elif defined(CONFIG_RALINK_RT2883)
	printk("\n*************** Ralink PCIe RC mode *************\n");
	mdelay(500);
	if ((RALINK_PCI_STATUS & 0x1) == 0) {
		printk(" RALINK_PCI_STATUS = %x\n", RALINK_PCI_STATUS);
		printk("************No PCIE device**********\n");
		for (i = 0; i < 16; i++) {
			read_config(0, 0, 0, i << 2, &val);
			printk("pci-to-pci 0x%02x = %08lx\n", i << 2, val);
		}
		return 0;
	}
#else
	for (i = 0; i < 0xfffff; i++) ;
	RALINK_PCI_ARBCTL = 0x79;
#endif
	//printk(" RALINK_PCI_ARBCTL = %x\n", RALINK_PCI_ARBCTL);

/*
	ioport_resource.start = rt2880_res_pci_io1.start;
  	ioport_resource.end = rt2880_res_pci_io1.end;
*/

	RALINK_PCI_MEMBASE = 0xffffffff;	//RALINK_PCI_MM_MAP_BASE;
	RALINK_PCI_IOBASE = RALINK_PCI_IO_MAP_BASE;

#ifdef CONFIG_RALINK_RT2880
	RALINK_PCI_BAR0SETUP_ADDR = 0x07FF0000;	//open 1FF:32M; DISABLE
	RALINK_PCI_IMBASEBAR0_ADDR = MEMORY_BASE;
	RALINK_PCI_ID = 0x08021814;
	RALINK_PCI_CLASS = 0x00800001;
	RALINK_PCI_SUBID = 0x28801814;
#elif defined(CONFIG_RALINK_RT2883)
	RALINK_PCI_BAR0SETUP_ADDR = 0x01FF0000;	//open 1FF:32M; DISABLE
	RALINK_PCI_IMBASEBAR0_ADDR = MEMORY_BASE;
	RALINK_PCI_ID = 0x08021814;
	RALINK_PCI_CLASS = 0x06040001;
	RALINK_PCI_SUBID = 0x28801814;
#elif defined(CONFIG_RALINK_RT3883)
	//PCI
	RALINK_PCI0_BAR0SETUP_ADDR = 0x03FF0000;	//open 3FF:64M; DISABLE
	RALINK_PCI0_IMBASEBAR0_ADDR = MEMORY_BASE;
	RALINK_PCI0_ID = 0x08021814;
	RALINK_PCI0_CLASS = 0x00800001;
	RALINK_PCI0_SUBID = 0x28801814;
	//PCIe
	RALINK_PCI1_BAR0SETUP_ADDR = 0x03FF0000;	//open 3FF:64M; DISABLE
	RALINK_PCI1_IMBASEBAR0_ADDR = MEMORY_BASE;
	RALINK_PCI1_ID = 0x08021814;
	RALINK_PCI1_CLASS = 0x06040001;
	RALINK_PCI1_SUBID = 0x28801814;
#endif

#ifdef CONFIG_RALINK_RT3883
	RALINK_PCI_PCIMSK_ADDR = 0x001c0000;	// enable pcie/pci interrupt
#else
	RALINK_PCI_PCIMSK_ADDR = 0x000c0000;	// enable pci interrupt
#endif

#ifdef CONFIG_RALINK_RT3883
	//PCIe
	read_config(0, 1, 0, 0x4, &val);
	write_config(0, 1, 0, 0x4, val | 0x7);
	//PCI
	read_config(0, 0, 0, 0x4, &val);
	write_config(0, 0, 0, 0x4, val | 0x7);
#elif defined(CONFIG_RALINK_RT2883)
	read_config(0, 0, 0, 0x4, &val);
	write_config(0, 0, 0, 0x4, val | 0x7);
	//FIXME
	////write_config(0, 0, 0, 0x18, 0x10100);
	//write_config(0, 0, 0, PCI_BASE_ADDRESS_0, MEMORY_BASE);
	//read_config(0, 0, 0, PCI_BASE_ADDRESS_0, &val); 
	////printk("BAR0 at slot 0 = %x\n", val); 
#else
	write_config(0, 0, 0, PCI_BASE_ADDRESS_0, MEMORY_BASE);
	read_config(0, 0, 0, PCI_BASE_ADDRESS_0, &val);
	printk("BAR0 at slot 0 = %lx\n", val);
#endif
	register_pci_controller(&rt2880_controller);
	return 0;

}

#ifndef CONFIG_PCIE_PCI_NONE
arch_initcall(init_rt2880pci);
#endif

/* Do platform specific device initialization at pci_enable_device() time */
int pcibios_plat_dev_init(struct pci_dev *dev)
{
	return 0;
}

struct pci_fixup pcibios_fixups[] = {
//      {PCI_ANY_ID, PCI_ANY_ID, pcibios_fixup_resources },
	{0}
};

//DECLARE_PCI_FIXUP_FINAL(PCI_ANY_ID, PCI_ANY_ID, pcibios_fixup_resources)
#endif /* CONFIG_PCI */
