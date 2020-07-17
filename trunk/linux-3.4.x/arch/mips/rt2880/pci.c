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
#include <asm/irq.h>

#include <asm/rt2880/eureka_ep430.h>

#ifdef CONFIG_PCI

//#define RAPCI_DEBUG

#if defined (CONFIG_RT2880_DRAM_512M)
#define BAR0_MASK			0x1FFF0000
#elif defined (CONFIG_RT2880_DRAM_256M)
#define BAR0_MASK			0x0FFF0000
#elif defined (CONFIG_RT2880_DRAM_128M)
#define BAR0_MASK			0x07FF0000
#elif defined (CONFIG_RT2880_DRAM_64M)
#define BAR0_MASK			0x03FF0000
#elif defined (CONFIG_RT2880_DRAM_32M)
#define BAR0_MASK			0x01FF0000
#else
#define BAR0_MASK			0x7FFF0000	/* 2G */
#endif

/*
 * These functions and structures provide the BIOS scan and mapping of the PCI
 * devices.
 */

#if defined (CONFIG_RALINK_MT7621)
#define RALINK_SYSTEM_CONTROL_BASE	0xbe000000
#define RALINK_PCI_MM_MAP_BASE		0x60000000
#define RALINK_PCI_IO_MAP_BASE		0x1e160000
#else
#define RALINK_SYSTEM_CONTROL_BASE	0xb0000000
#define RALINK_PCI_MM_MAP_BASE		0x20000000
#define RALINK_PCI_IO_MAP_BASE		0x10160000
#endif

#define BAR0_MEMORY_BASE			0x0

#if defined (CONFIG_RALINK_MT7621)
extern u32 ralink_asic_rev_id;
static int pcie_link_status = 0;

/* use GPIO control instead of PERST_N (pulse from driver) */
#define GPIO_PERST

/* uncomment this to enable PCIe ports Spread Spectrum (MT7603E Ch14 Rx De-sense issue) */
//#define PCIE_PHY_SSC

#define PCIE_SHARE_PIN_SW		10	// PERST_N GPIO Mode
#if defined (GPIO_PERST)
#define GPIO_PCIE_PORT0			19	// PERST_N
#if defined (CONFIG_RALINK_I2S) || defined (CONFIG_RALINK_I2S_MODULE) || defined (CONFIG_PCIE_PERST_ONLY)
#define UARTL3_SHARE_PIN_SW		PCIE_SHARE_PIN_SW
#define GPIO_PCIE_PORT1			GPIO_PCIE_PORT0
#define GPIO_PCIE_PORT2			GPIO_PCIE_PORT0
#else
#define UARTL3_SHARE_PIN_SW		3	// UART3 GPIO Mode
#define GPIO_PCIE_PORT1			8	// RXD3 (I2S_SDI)
#define GPIO_PCIE_PORT2			7	// TXD3 (I2S_WS)
#endif
#define RALINK_GPIO_CTRL0		*(volatile u32 *)(RALINK_PIO_BASE + 0x00)
#define RALINK_GPIO_DSET0		*(volatile u32 *)(RALINK_PIO_BASE + 0x30)
#define RALINK_GPIO_DCLR0		*(volatile u32 *)(RALINK_PIO_BASE + 0x40)
#endif

#define ASSERT_SYSRST_PCIE(val)		do { \
						if ((ralink_asic_rev_id & 0xFFFF) == 0x0101) \
							RALINK_RSTCTRL |= val; \
						else \
							RALINK_RSTCTRL &= ~val; \
					} while(0)
#define DEASSERT_SYSRST_PCIE(val)	do { \
						if ((ralink_asic_rev_id & 0xFFFF) == 0x0101) \
							RALINK_RSTCTRL &= ~val; \
						else \
							RALINK_RSTCTRL |= val; \
					} while(0)
#endif

#define RALINK_SYSCFG1 			*(volatile u32 *)(RALINK_SYSTEM_CONTROL_BASE + 0x14)
#define RALINK_CLKCFG1			*(volatile u32 *)(RALINK_SYSTEM_CONTROL_BASE + 0x30)
#define RALINK_RSTCTRL			*(volatile u32 *)(RALINK_SYSTEM_CONTROL_BASE + 0x34)
#define RALINK_GPIOMODE			*(volatile u32 *)(RALINK_SYSTEM_CONTROL_BASE + 0x60)
#if defined (CONFIG_RALINK_RT3883)
#define RALINK_PCIE_CLK_GEN		*(volatile u32 *)(RALINK_SYSTEM_CONTROL_BASE + 0x7c)
#define RALINK_PCIE_CLK_GEN1		*(volatile u32 *)(RALINK_SYSTEM_CONTROL_BASE + 0x80)
//RALINK_SYSCFG1 bit
#define RALINK_PCI_HOST_MODE_EN		(1<<7)
#define RALINK_PCIE_RC_MODE_EN		(1<<8)
//RALINK_GPIOMODE bit
#define PCI_SLOTx2			(1<<11)
#define PCI_SLOTx1			(2<<11)
#elif defined (CONFIG_RALINK_MT7620)
#define PPLL_CFG1			*(volatile u32 *)(RALINK_SYSTEM_CONTROL_BASE + 0x9c)
#define PPLL_DRV			*(volatile u32 *)(RALINK_SYSTEM_CONTROL_BASE + 0xa0)
//RALINK_SYSCFG1 bit
#define RALINK_PCIE_RC_MODE_EN		(1<<8)
//MTK PCIE PLL bit
#define PDRV_SW_SET			(1<<31)
#define LC_CKDRVPD			(1<<19)
#define LC_CKDRVOHZ			(1<<18)
#define LC_CKDRVHZ			(1<<17)
#endif

#define PCI_ACCESS_READ_1		0
#define PCI_ACCESS_READ_2		1
#define PCI_ACCESS_READ_4		2
#define PCI_ACCESS_WRITE_1		3
#define PCI_ACCESS_WRITE_2		4
#define PCI_ACCESS_WRITE_4		5

static DEFINE_SPINLOCK(asic_pcr_lock);

static int config_access(int access_type, u32 busn, u32 slot, u32 func, u32 where, u32 *data)
{
	unsigned int address, shift, tmp;
	unsigned long flags;

#if defined(CONFIG_RALINK_RT3883)
	if (busn == 0)
		where &= 0xff; // high bits used only for RT3883 PCIe bus (busn 1)
#endif

	/* setup PCR address */
	address = (1u << 31) | (((where & 0xf00) >> 8) << 24) | (busn << 16) | (slot << 11) | (func << 8) | (where & 0xfc);

	shift = (where & 0x3) << 3;

	spin_lock_irqsave(&asic_pcr_lock, flags);

	/* start the configuration cycle */
	RALINK_PCI_PCR_ADDR = address;

	switch (access_type) {
	case PCI_ACCESS_WRITE_1:
		tmp = RALINK_PCI_PCR_DATA;
		tmp &= ~(0xff << shift);
		tmp |= ((*data & 0xff) << shift);
		RALINK_PCI_PCR_DATA = tmp;
		break;
	case PCI_ACCESS_WRITE_2:
		tmp = RALINK_PCI_PCR_DATA;
		if (shift > 16)
			shift = 16;
		tmp &= ~(0xffff << shift);
		tmp |= ((*data & 0xffff) << shift);
		RALINK_PCI_PCR_DATA = tmp;
		break;
	case PCI_ACCESS_WRITE_4:
		RALINK_PCI_PCR_DATA = *data;
		break;
	case PCI_ACCESS_READ_1:
		tmp = RALINK_PCI_PCR_DATA;
		*data = (tmp >> shift) & 0xff;
		break;
	case PCI_ACCESS_READ_2:
		tmp = RALINK_PCI_PCR_DATA;
		if (shift > 16)
			shift = 16;
		*data = (tmp >> shift) & 0xffff;
		break;
	case PCI_ACCESS_READ_4:
		*data = RALINK_PCI_PCR_DATA;
		break;
	}

	spin_unlock_irqrestore(&asic_pcr_lock, flags);

	return PCIBIOS_SUCCESSFUL;
}

static int ralink_pci_config_read(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 *val)
{
	u32 busn = bus->number;
	u32 slot = PCI_SLOT(devfn);
	u32 func = PCI_FUNC(devfn);
	int access_type = PCI_ACCESS_READ_4;

	switch (size) {
	case 1:
		access_type = PCI_ACCESS_READ_1;
		break;
	case 2:
		access_type = PCI_ACCESS_READ_2;
		break;
	}

	return config_access(access_type, busn, slot, func, (u32)where, val);
}

static int ralink_pci_config_write(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 val)
{
	u32 busn = bus->number;
	u32 slot = PCI_SLOT(devfn);
	u32 func = PCI_FUNC(devfn);
	int access_type = PCI_ACCESS_WRITE_4;

	switch (size) {
	case 1:
		access_type = PCI_ACCESS_WRITE_1;
		break;
	case 2:
		access_type = PCI_ACCESS_WRITE_2;
		break;
	}

	return config_access(access_type, busn, slot, func, (u32)where, &val);
}

/*
 *  General-purpose PCI functions.
 */

struct pci_ops ralink_pci_ops = {
	.read		 = ralink_pci_config_read,
	.write		 = ralink_pci_config_write,
};

static struct resource ralink_res_pci_mem1 = {
	.name		 = "PCI MEM1",
	.start		 = RALINK_PCI_MM_MAP_BASE,
	.end		 = (RALINK_PCI_MM_MAP_BASE + 0x0fffffff),
	.flags		 = IORESOURCE_MEM,
};

static struct resource ralink_res_pci_io1 = {
	.name		 = "PCI I/O1",
	.start		 = RALINK_PCI_IO_MAP_BASE,
	.end		 = (RALINK_PCI_IO_MAP_BASE + 0x0ffff),
	.flags		 = IORESOURCE_IO,
};

struct pci_controller ralink_pci_controller = {
	.pci_ops	 = &ralink_pci_ops,
	.mem_resource	 = &ralink_res_pci_mem1,
	.io_resource	 = &ralink_res_pci_io1,
	.mem_offset	 = 0x00000000UL,
	.io_offset	 = 0x00000000UL,
};

int pcibios_plat_dev_init(struct pci_dev *dev)
{
	u32 __maybe_unused val;
#ifdef RAPCI_DEBUG
	int i;
	struct resource *res;

	printk("%s: ** bus: %d, slot: 0x%x\n", __FUNCTION__, dev->bus->number, PCI_SLOT(dev->devfn));

	pci_read_config_dword(dev, PCI_BASE_ADDRESS_0, &val);
	printk(" PCI_BASE_ADDRESS_0: 0x%08X\n", val);

	pci_read_config_dword(dev, PCI_BASE_ADDRESS_1, &val);
	printk(" PCI_BASE_ADDRESS_1: 0x%08X\n", val);

	pci_read_config_dword(dev, PCI_IO_BASE, &val);
	printk(" PCI_IO_BASE: 0x%08X\n", val);

	for (i = 0; i < 2; i++) {
		res = (struct resource*)&dev->resource[i];
		printk(" res[%d]->start = %x\n", i, res->start);
		printk(" res[%d]->end = %x\n", i, res->end);
	}
#endif

	/* P2P bridge */
	if (dev->bus->number == 0) {
#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621) || \
    defined (CONFIG_RALINK_MT7628)
		/* set N_FTS 0x28 -> 0x50 */
		val = 0;
		pci_read_config_dword(dev, 0x70c, &val);
		val &= ~(0xff<<8);
		val |=  (0x50<<8);
		pci_write_config_dword(dev, 0x70c, val);
#elif defined (CONFIG_RALINK_RT3883)
		/* fix IO_BASE */
		if (PCI_SLOT(dev->devfn) == 0x1)
			pci_write_config_dword(dev, PCI_IO_BASE, 0x00000101);
#endif
		/* set CLS */
		pci_write_config_byte(dev, PCI_CACHE_LINE_SIZE, (L1_CACHE_BYTES >> 2));
	}

	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)
int __init pcibios_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
#else
int __init pcibios_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
#endif
{
	int pci_irq = 0;
#if defined (CONFIG_RALINK_MT7621)
	if ((dev->bus->number == 1) && (slot == 0x0)) {
		switch (pcie_link_status) {
		case 0x2:
		case 0x6:
			pci_irq = SURFBOARDINT_PCIE1;
			break;
		case 0x4:
			pci_irq = SURFBOARDINT_PCIE2;
			break;
		default:
			pci_irq = SURFBOARDINT_PCIE0;
		}
	} else if ((dev->bus->number == 2) && (slot == 0x0)) {
		switch (pcie_link_status) {
		case 0x5:
		case 0x6:
			pci_irq = SURFBOARDINT_PCIE2;
			break;
		default:
			pci_irq = SURFBOARDINT_PCIE1;
		}
	} else if ((dev->bus->number == 2) && (slot == 0x1)) {
		switch (pcie_link_status) {
		case 0x5:
		case 0x6:
			pci_irq = SURFBOARDINT_PCIE2;
			break;
		default:
			pci_irq = SURFBOARDINT_PCIE1;
		}
	} else if ((dev->bus->number == 3) && (slot == 0x0)) {
		pci_irq = SURFBOARDINT_PCIE2;
	} else if ((dev->bus->number == 3) && (slot == 0x1)) {
		pci_irq = SURFBOARDINT_PCIE2;
	} else if ((dev->bus->number == 3) && (slot == 0x2)) {
		pci_irq = SURFBOARDINT_PCIE2;
	}
#elif defined (CONFIG_RALINK_MT7620) || defined(CONFIG_RALINK_MT7628)
	if ((dev->bus->number == 1) && (slot == 0x0)) {
		pci_irq = SURFBOARDINT_PCIE0;
	}
#elif defined (CONFIG_RALINK_RT3883)
	if ((dev->bus->number == 0) && (slot == 0x11)) {
		pci_irq = SURFBOARDINT_PCI0;
	} else if ((dev->bus->number == 0) && (slot == 0x12)) {
		pci_irq = SURFBOARDINT_PCI1;
	} else if ((dev->bus->number == 1)) {
		pci_irq = SURFBOARDINT_PCIE0;
	}
#endif

#ifdef RAPCI_DEBUG
	printk("%s: ** bus: %d, slot: 0x%x -> irq: %d\n", __FUNCTION__, dev->bus->number, slot, pci_irq);
#endif

	return pci_irq;
}

#if defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
static void set_pcie_phy(u32 *addr, int start_b, int bits, int val)
{
	*(volatile u32 *)(addr) &= ~(((1<<bits) - 1)<<start_b);
	*(volatile u32 *)(addr) |= val << start_b;
}
#endif

#if defined (CONFIG_RALINK_MT7621)
static void bypass_pipe_rst(void)
{
#if defined (CONFIG_PCIE_PORT0)
	/* PCIe Port 0 */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x02c), 12, 1, 0x01);	// rg_pe1_pipe_rst_b
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x02c),  4, 1, 0x01);	// rg_pe1_pipe_cmd_frc[4]
#endif
#if defined (CONFIG_PCIE_PORT1)
	/* PCIe Port 1 */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x12c), 12, 1, 0x01);	// rg_pe1_pipe_rst_b
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x12c),  4, 1, 0x01);	// rg_pe1_pipe_cmd_frc[4]
#endif
#if defined (CONFIG_PCIE_PORT2)
	/* PCIe Port 2 */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x02c), 12, 1, 0x01);	// rg_pe1_pipe_rst_b
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x02c),  4, 1, 0x01);	// rg_pe1_pipe_cmd_frc[4]
#endif
}

static void set_phy_for_ssc(void)
{
	u32 reg = (*(volatile u32 *)(RALINK_SYSCTL_BASE + 0x10));

	reg = (reg >> 6) & 0x7;
#if defined (CONFIG_PCIE_PORT0) || defined (CONFIG_PCIE_PORT1)
	/* Set PCIe Port0 & Port1 PHY to disable SSC */
	/* Debug Xtal Type */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x400),  8, 1, 0x01);	// rg_pe1_frc_h_xtal_type
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x400),  9, 2, 0x00);	// rg_pe1_h_xtal_type
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x000),  4, 1, 0x01);	// rg_pe1_frc_phy_en               //Force Port 0 enable control
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x100),  4, 1, 0x01);	// rg_pe1_frc_phy_en               //Force Port 1 enable control
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x000),  5, 1, 0x00);	// rg_pe1_phy_en                   //Port 0 disable
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x100),  5, 1, 0x00);	// rg_pe1_phy_en                   //Port 1 disable
	if (reg <= 5 && reg >= 3) {
		/* 40MHz Xtal */
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x490),  6, 2, 0x01);	// RG_PE1_H_PLL_PREDIV             //Pre-divider ratio (for host mode)
		
		/* SSC option tune from -5000ppm to -1000ppm */
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a8),  0,12, 0x1a);	// RG_LC_DDS_SSC_DELTA
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a8), 16,12, 0x1a);	// RG_LC_DDS_SSC_DELTA1
	} else {
		/* 25MHz or 20MHz Xtal */
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x490),  6, 2, 0x00);	// RG_PE1_H_PLL_PREDIV             //Pre-divider ratio (for host mode)
		if (reg >= 6) {
			/* 25MHz Xtal */
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4bc),  4, 2, 0x01);	// RG_PE1_H_PLL_FBKSEL             //Feedback clock select
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x49c),  0,31, 0x18000000);	// RG_PE1_H_LCDDS_PCW_NCPO         //DDS NCPO PCW (for host mode)
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a4),  0,16, 0x18d);	// RG_PE1_H_LCDDS_SSC_PRD          //DDS SSC dither period control
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a8),  0,12, 0x4a);	// RG_PE1_H_LCDDS_SSC_DELTA        //DDS SSC dither amplitude control
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a8), 16,12, 0x4a);	// RG_PE1_H_LCDDS_SSC_DELTA1       //DDS SSC dither amplitude control for initial
			
			/* SSC option tune from -5000ppm to -1000ppm */
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a8),  0,12, 0x11);	// RG_LC_DDS_SSC_DELTA
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a8), 16,12, 0x11);	// RG_LC_DDS_SSC_DELTA1
		} else {
			/* 20MHz Xtal */
			
			/* SSC option tune from -5000ppm to -1000ppm */
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a8),  0,12, 0x1a);	// RG_LC_DDS_SSC_DELTA
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a8), 16,12, 0x1a);	// RG_LC_DDS_SSC_DELTA1
		}
	}
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a0),  5, 1, 0x01);	// RG_PE1_LCDDS_CLK_PH_INV         //DDS clock inversion
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x490), 22, 2, 0x02);	// RG_PE1_H_PLL_BC                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x490), 18, 4, 0x06);	// RG_PE1_H_PLL_BP                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x490), 12, 4, 0x02);	// RG_PE1_H_PLL_IR                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x490),  8, 4, 0x01);	// RG_PE1_H_PLL_IC                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4ac), 16, 3, 0x00);	// RG_PE1_H_PLL_BR                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x490),  1, 3, 0x02);	// RG_PE1_PLL_DIVEN                
	if (reg <= 5 && reg >= 3) {
		/* 40MHz Xtal */
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x414),  6, 2, 0x01);	// rg_pe1_mstckdiv		//value of da_pe1_mstckdiv when force mode enable
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x414),  5, 1, 0x01);	// rg_pe1_frc_mstckdiv          //force mode enable of da_pe1_mstckdiv      
	}
#ifdef PCIE_PHY_SSC
	/* Enable Port0&Port1 SSC */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x414), 28, 2, 0x1);	// rg_pe1_frc_lcdds_ssc_en              //value of da_pe1_mstckdiv when force mode enable
#else
	/* Disable Port0&Port1 SSC */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x414), 28, 2, 0x0);	// rg_pe1_frc_lcdds_ssc_en              //value of da_pe1_mstckdiv when force mode enable
#endif
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x040), 17, 4, 0x07);	// rg_pe1_crtmsel                   //value of da[x]_pe1_crtmsel when force mode enable for Port 0
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x040), 16, 1, 0x01);	// rg_pe1_frc_crtmsel               //force mode enable of da[x]_pe1_crtmsel for Port 0
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x140), 17, 4, 0x07);	// rg_pe1_crtmsel                   //value of da[x]_pe1_crtmsel when force mode enable for Port 1
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x140), 16, 1, 0x01);	// rg_pe1_frc_crtmsel               //force mode enable of da[x]_pe1_crtmsel for Port 1
	/* Enable PHY and disable force mode */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x000),  5, 1, 0x01);	// rg_pe1_phy_en                   //Port 0 enable
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x100),  5, 1, 0x01);	// rg_pe1_phy_en                   //Port 1 enable
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x000),  4, 1, 0x00);	// rg_pe1_frc_phy_en               //Force Port 0 disable control
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x100),  4, 1, 0x00);	// rg_pe1_frc_phy_en               //Force Port 1 disable control
#endif
#if defined (CONFIG_PCIE_PORT2)
	/* Set PCIe Port2 PHY to disable SSC */
	/* Debug Xtal Type */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x400),  8, 1, 0x01);	// rg_pe1_frc_h_xtal_type
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x400),  9, 2, 0x00);	// rg_pe1_h_xtal_type
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x000),  4, 1, 0x01);	// rg_pe1_frc_phy_en               //Force Port 0 enable control
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x000),  5, 1, 0x00);	// rg_pe1_phy_en                   //Port 0 disable
	if (reg <= 5 && reg >= 3) {
		/* 40MHz Xtal */
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x490),  6, 2, 0x01);	// RG_PE1_H_PLL_PREDIV             //Pre-divider ratio (for host mode)
		
		/* SSC option tune from -5000ppm to -1000ppm */
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a8),  0,12, 0x1a);	// RG_LC_DDS_SSC_DELTA
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a8), 16,12, 0x1a);	// RG_LC_DDS_SSC_DELTA1
	} else {
		/* 25MHz or 20MHz Xtal */
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x490),  6, 2, 0x00);	// RG_PE1_H_PLL_PREDIV             //Pre-divider ratio (for host mode)
		if (reg >= 6) {
			/* 25MHz Xtal */
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4bc),  4, 2, 0x01);	// RG_PE1_H_PLL_FBKSEL             //Feedback clock select
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x49c),  0,31, 0x18000000);	// RG_PE1_H_LCDDS_PCW_NCPO         //DDS NCPO PCW (for host mode)
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a4),  0,16, 0x18d);	// RG_PE1_H_LCDDS_SSC_PRD          //DDS SSC dither period control
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a8),  0,12, 0x4a);	// RG_PE1_H_LCDDS_SSC_DELTA        //DDS SSC dither amplitude control
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a8), 16,12, 0x4a);	// RG_PE1_H_LCDDS_SSC_DELTA1       //DDS SSC dither amplitude control for initial
			
			 /* SSC option tune from -5000ppm to -1000ppm */
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a8),  0,12, 0x11);	// RG_LC_DDS_SSC_DELTA
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a8), 16,12, 0x11);	// RG_LC_DDS_SSC_DELTA1
		} else {
			/* 20MHz Xtal */
			
			/* SSC option tune from -5000ppm to -1000ppm */
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a8),  0,12, 0x1a);	// RG_LC_DDS_SSC_DELTA
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a8), 16,12, 0x1a);	// RG_LC_DDS_SSC_DELTA1
		}
	}
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a0),  5, 1, 0x01);	// RG_PE1_LCDDS_CLK_PH_INV         //DDS clock inversion
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x490), 22, 2, 0x02);	// RG_PE1_H_PLL_BC                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x490), 18, 4, 0x06);	// RG_PE1_H_PLL_BP                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x490), 12, 4, 0x02);	// RG_PE1_H_PLL_IR                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x490),  8, 4, 0x01);	// RG_PE1_H_PLL_IC                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4ac), 16, 3, 0x00);	// RG_PE1_H_PLL_BR                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x490),  1, 3, 0x02);	// RG_PE1_PLL_DIVEN                
	if (reg <= 5 && reg >= 3) {
		/* 40MHz Xtal */
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x414),  6, 2, 0x01);	// rg_pe1_mstckdiv		//value of da_pe1_mstckdiv when force mode enable
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x414),  5, 1, 0x01);	// rg_pe1_frc_mstckdiv          //force mode enable of da_pe1_mstckdiv      
	}
#ifdef PCIE_PHY_SSC
	/* Enable Port2 SSC */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x414), 28, 2, 0x1);	// rg_pe1_frc_lcdds_ssc_en              //value of da_pe1_mstckdiv when force mode enable
#else
	/* Disable Port2 SSC */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x414), 28, 2, 0x0);	// rg_pe1_frc_lcdds_ssc_en              //value of da_pe1_mstckdiv when force mode enable
#endif
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x040), 17, 4, 0x07);	// rg_pe1_crtmsel                   //value of da[x]_pe1_crtmsel when force mode enable for Port 0
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x040), 16, 1, 0x01);	// rg_pe1_frc_crtmsel               //force mode enable of da[x]_pe1_crtmsel for Port 0
	/* Enable PHY and disable force mode */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x000),  5, 1, 0x01);	// rg_pe1_phy_en                   //Port 0 enable
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x000),  4, 1, 0x00);	// rg_pe1_frc_phy_en               //Force Port 0 disable control
#endif
}
#endif

#if defined (CONFIG_MT7628_ASIC)
void pcie_phy_config(void)
{
	u32 reg = (*(volatile u32 *)(RALINK_SYSCTL_BASE + 0x10));

	reg = (reg >> 6) & 0x1;

	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x400), 8, 1, 0x01);		// [rg_pe1_frc_h_xtal_type]: Enable Crystal type force mode
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x400), 9, 2, 0x00);		// [rg_pe1_h_xtal_type]: Force Crystal type = 20MHz 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x000), 4, 1, 0x01);		// [rg_pe1_frc_phy_en]: Enable Port 0 force mode
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x000), 5, 1, 0x00);		// [rg_pe1_phy_en]: Port 0 disable
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x4AC),16, 3, 0x03);		// [RG_PE1_H_PLL_BR]
	if (reg == 1) {
		/* 40MHz Xtal */
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x4BC),24, 8, 0x7D);	// [RG_PE1_H_PLL_FBKDIV]
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x490),12, 4, 0x08);	// [RG_PE1_H_PLL_IR]
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x490), 6, 2, 0x01);	// [RG_PE1_H_PLL_PREDIV]: Pre-divider ratio (for host mode)
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x4C0), 0,32, 0x1F400000);	// [RG_PE1_H_LCDDS_PCW_NCPO]: For 40MHz crystal input
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x4A4), 0,16, 0x013D);	// [RG_PE1_H_LCDDS_SSC_PRD]
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x4A8),16,16, 0x74);	// [RG_PE1_H_LCDDS_SSC_DELTA1]: For SSC=4500ppm
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x4A8), 0,16, 0x74);	// [RG_PE1_H_LCDDS_SSC_DELTA]: For SSC=4500ppm
	} else {
		/* 25MHz Xtal */
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x4BC),24, 8, 0x64);	// [RG_PE1_H_PLL_FBKDIV]
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x490),12, 4, 0x0A);	// [RG_PE1_H_PLL_IR]
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x490), 6, 2, 0x00);	// [RG_PE1_H_PLL_PREDIV]: Pre-divider ratio (for host mode)
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x4C0), 0,32, 0x19000000);	// [RG_PE1_H_LCDDS_PCW_NCPO]: For 25MHz crystal input
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x4A4), 0,16, 0x018D);	// [RG_PE1_H_LCDDS_SSC_PRD]
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x4A8),16,16, 0x4A);	// [RG_PE1_H_LCDDS_SSC_DELTA1]: For SSC=4500ppm
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x4A8), 0,16, 0x4A);	// [RG_PE1_H_LCDDS_SSC_DELTA]: For SSC=4500ppm
	}
	/* MT7628 PCIe PHY LDO setting: 0x1 -> 0x5 (1.0V -> 1.2V) */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x498), 0, 8, 0x05);

	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x000), 5, 1, 0x01);	// Port 0 enable			[rg_pe1_phy_en]
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x000), 4, 1, 0x00);	// Disable Port 0 force mode		[rg_pe1_frc_phy_en]
}
#endif

int __init init_ralink_pci(void)
{
	u32 __maybe_unused val;
#if defined (CONFIG_RALINK_RT3883)
	int pcie_disable = 0;
#endif

	PCIBIOS_MIN_IO = 0;
	PCIBIOS_MIN_MEM = 0;

#if defined (CONFIG_RALINK_MT7621)
	pcie_link_status = 0;
	val = RALINK_PCIE0_RST | RALINK_PCIE1_RST | RALINK_PCIE2_RST;
	ASSERT_SYSRST_PCIE(val);			// raise reset all PCIe ports
	udelay(100);
#if defined (GPIO_PERST)
	val = RALINK_GPIOMODE;
	val &= ~((0x3<<PCIE_SHARE_PIN_SW) | (0x3<<UARTL3_SHARE_PIN_SW));
	val |=  ((0x1<<PCIE_SHARE_PIN_SW) | (0x1<<UARTL3_SHARE_PIN_SW));
	RALINK_GPIOMODE = val;
	val = 0;
#if defined (CONFIG_PCIE_PORT0)
	val |= (0x1<<GPIO_PCIE_PORT0);
#endif
#if defined (CONFIG_PCIE_PORT1)
	val |= (0x1<<GPIO_PCIE_PORT1);
#endif
#if defined (CONFIG_PCIE_PORT2)
	val |= (0x1<<GPIO_PCIE_PORT2);
#endif
	mdelay(50);
	RALINK_GPIO_CTRL0 |= val;			// switch PERST_N pin to output mode
	mdelay(50);
	RALINK_GPIO_DCLR0 = val;			// fall PERST_N pin (reset peripherals)
#else /* !defined (GPIO_PERST) */
	RALINK_GPIOMODE &= ~(0x3<<PCIE_SHARE_PIN_SW);	// fall PERST_N pin (reset peripherals)
#endif
	mdelay(100);					// wait 100 ms pulse

	val = 0;
#if defined (CONFIG_PCIE_PORT0)
	val |= RALINK_PCIE0_RST;
#endif
#if defined (CONFIG_PCIE_PORT1)
	val |= RALINK_PCIE1_RST;
#endif
#if defined (CONFIG_PCIE_PORT2)
	val |= RALINK_PCIE2_RST;
#endif
	DEASSERT_SYSRST_PCIE(val);			// release reset for needed PCIe ports

	val = RALINK_CLKCFG1;
	val &= ~(RALINK_PCIE0_CLK_EN | RALINK_PCIE1_CLK_EN | RALINK_PCIE2_CLK_EN);
#if defined (CONFIG_PCIE_PORT0)
	val |= RALINK_PCIE0_CLK_EN;
#endif
#if defined (CONFIG_PCIE_PORT1)
	val |= RALINK_PCIE1_CLK_EN;
#endif
#if defined (CONFIG_PCIE_PORT2)
	val |= RALINK_PCIE2_CLK_EN;
#endif
	RALINK_CLKCFG1 = val;				// enable clock for needed PCIe ports

	mdelay(10);

	if ((ralink_asic_rev_id & 0xFFFF) == 0x0101) // MT7621 E2
		bypass_pipe_rst();
	set_phy_for_ssc();

	mdelay(100);

#if defined (GPIO_PERST)
	val = 0;
#if defined (CONFIG_PCIE_PORT0)
	val |= (0x1<<GPIO_PCIE_PORT0);
#endif
#if defined (CONFIG_PCIE_PORT1)
	val |= (0x1<<GPIO_PCIE_PORT1);
#endif
#if defined (CONFIG_PCIE_PORT2)
	val |= (0x1<<GPIO_PCIE_PORT2);
#endif
	RALINK_GPIO_DSET0 = val;			// rise PERST_N pin (complete reset peripherals)
#else /* !defined (GPIO_PERST) */
	RALINK_PCI_PCICFG_ADDR &= ~(1<<1);		// release PCIRST
#endif

#elif defined (CONFIG_RALINK_MT7628)
	RALINK_GPIOMODE &= ~(0x01<<16);			// PERST_GPIO_MODE = 1'b0

	RALINK_RSTCTRL &= ~RALINK_PCIE0_RST;
	RALINK_CLKCFG1 |=  RALINK_PCIE0_CLK_EN;

	mdelay(100);

	pcie_phy_config();

	RALINK_PCI_PCICFG_ADDR &= ~(1<<1);		// release PCIRST
#elif defined (CONFIG_RALINK_MT7620)
	RALINK_GPIOMODE &= ~(0x3<<16);			// PERST_GPIO_MODE = 2'b00

	RALINK_SYSCFG1 |=  RALINK_PCIE_RC_MODE_EN;	// PCIe in RC mode
	RALINK_RSTCTRL &= ~RALINK_PCIE0_RST;
	RALINK_CLKCFG1 |=  RALINK_PCIE0_CLK_EN;

	mdelay(50);

	if ( !(PPLL_CFG1 & (1<<23)) ) {
		printk("MT7620 PPLL unlock, cannot enable PCIe!\n");
		/* for power saving */
		RALINK_RSTCTRL |=  RALINK_PCIE0_RST;
		RALINK_CLKCFG1 &= ~RALINK_PCIE0_CLK_EN;
		return 0;
	}

	PPLL_DRV |=  LC_CKDRVPD;			// PCIe clock driver power ON
	PPLL_DRV &= ~LC_CKDRVOHZ;			// Reference PCIe Output clock mode enable
	PPLL_DRV &= ~LC_CKDRVHZ;			// PCIe PHY clock enable
	PPLL_DRV |=  PDRV_SW_SET;			// PDRV SW Set

	mdelay(50);

	RALINK_PCI_PCICFG_ADDR &= ~(1<<1);		// release PCIRST
#elif defined (CONFIG_RALINK_RT3883)

#if defined (CONFIG_PCIE_ONLY) || defined (CONFIG_PCIE_PCI_CONCURRENT)
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
#if defined (CONFIG_PCI_ONLY) || defined (CONFIG_PCIE_PCI_CONCURRENT)
	RALINK_GPIOMODE &= ~(PCI_SLOTx2|PCI_SLOTx1);
	RALINK_GPIOMODE |= PCI_SLOTx2;	// enable PCI slot 1, disable PCI slot 2
#endif
	RALINK_SYSCFG1 |= (RALINK_PCI_HOST_MODE_EN | RALINK_PCIE_RC_MODE_EN);	// PCI in host mode, PCIe in RC mode
#if defined (CONFIG_PCI_ONLY)
	RALINK_RSTCTRL |=  RALINK_PCIE_RST;
	RALINK_CLKCFG1 &= ~RALINK_PCIE_CLK_EN;
#elif defined (CONFIG_PCIE_ONLY)
	RALINK_RSTCTRL |=  RALINK_PCI_RST;
	RALINK_CLKCFG1 &= ~RALINK_PCI_CLK_EN;
#endif
	mdelay(200);
#if defined (CONFIG_PCIE_ONLY)
	RALINK_PCI_PCICFG_ADDR = 0;		// virtual P2P bridge DEVNUM = 0, release PCIRST
#else
	RALINK_PCI_PCICFG_ADDR = (1 << 16);	// virtual P2P bridge DEVNUM = 1, release PCIRST
#endif
#endif

	/* wait before detect card in slots */
	mdelay(500);

#if defined (CONFIG_RALINK_MT7621)
#if defined (CONFIG_PCIE_PORT0)
	if ((RALINK_PCI0_STATUS & 0x1) == 0) {
		ASSERT_SYSRST_PCIE(RALINK_PCIE0_RST);
		RALINK_CLKCFG1 &= ~RALINK_PCIE0_CLK_EN;
		printk("%s: no card, disable it (RST&CLK)\n", "PCIe0");
	} else {
		pcie_link_status |= (1<<0);
	}
#endif
#if defined (CONFIG_PCIE_PORT1)
	if ((RALINK_PCI1_STATUS & 0x1) == 0) {
		ASSERT_SYSRST_PCIE(RALINK_PCIE1_RST);
		RALINK_CLKCFG1 &= ~RALINK_PCIE1_CLK_EN;
		printk("%s: no card, disable it (RST&CLK)\n", "PCIe1");
	} else {
		pcie_link_status |= (1<<1);
	}
#endif
#if defined (CONFIG_PCIE_PORT2)
	if ((RALINK_PCI2_STATUS & 0x1) == 0) {
		ASSERT_SYSRST_PCIE(RALINK_PCIE2_RST);
		RALINK_CLKCFG1 &= ~RALINK_PCIE2_CLK_EN;
		printk("%s: no card, disable it (RST&CLK)\n", "PCIe2");
	} else {
		pcie_link_status |= (1<<2);
	}
#endif

	/* No cards, exit */
	if (pcie_link_status == 0)
		return 0;

/*
	pcie(2/1/0) link status	pcie2_num	pcie1_num	pcie0_num
	3'b000			x		x		x
	3'b001			x		x		0
	3'b010			x		0		x
	3'b011			x		1		0
	3'b100			0		x		x
	3'b101			1		x		0
	3'b110			1		0		x
	3'b111			2		1		0
*/
	switch (pcie_link_status) {
	case 0x2:
		/* PCIe1 only  */
		RALINK_PCI_PCICFG_ADDR &= ~0x00ff0000;
		RALINK_PCI_PCICFG_ADDR |= (0x1 << 16);	// PCIe0 -> port1
		RALINK_PCI_PCICFG_ADDR |= (0x0 << 20);	// PCIe1 -> port0 (*)
		break;
	case 0x4:
		/* PCIe2 only  */
		RALINK_PCI_PCICFG_ADDR &= ~0x0fff0000;
		RALINK_PCI_PCICFG_ADDR |= (0x1 << 16);	// PCIe0 -> port1
		RALINK_PCI_PCICFG_ADDR |= (0x2 << 20);	// PCIe1 -> port2
		RALINK_PCI_PCICFG_ADDR |= (0x0 << 24);	// PCIe2 -> port0 (*)
		break;
	case 0x5:
		/* PCIe0 + PCIe2 */
		RALINK_PCI_PCICFG_ADDR &= ~0x0fff0000;
		RALINK_PCI_PCICFG_ADDR |= (0x0 << 16);	// PCIe0 -> port0 (*)
		RALINK_PCI_PCICFG_ADDR |= (0x2 << 20);	// PCIe1 -> port2
		RALINK_PCI_PCICFG_ADDR |= (0x1 << 24);	// PCIe2 -> port1 (*)
		break;
	case 0x6:
		/* PCIe1 + PCIe2 */
		RALINK_PCI_PCICFG_ADDR &= ~0x0fff0000;
		RALINK_PCI_PCICFG_ADDR |= (0x2 << 16);	// PCIe0 -> port2
		RALINK_PCI_PCICFG_ADDR |= (0x0 << 20);	// PCIe1 -> port0 (*)
		RALINK_PCI_PCICFG_ADDR |= (0x1 << 24);	// PCIe2 -> port1 (*)
		break;
	}
#elif defined (CONFIG_RALINK_MT7628)
	if ((RALINK_PCI0_STATUS & 0x1) == 0) {
		RALINK_RSTCTRL |=  RALINK_PCIE0_RST;
		RALINK_CLKCFG1 &= ~RALINK_PCIE0_CLK_EN;
		printk("%s: no card, disable it (RST&CLK)\n", "PCIe0");
		return 0;
	}
#elif defined (CONFIG_RALINK_MT7620)
	if ((RALINK_PCI0_STATUS & 0x1) == 0) {
		RALINK_RSTCTRL |=  RALINK_PCIE0_RST;
		RALINK_CLKCFG1 &= ~RALINK_PCIE0_CLK_EN;
		PPLL_DRV &= ~LC_CKDRVPD;
		PPLL_DRV |=  PDRV_SW_SET;
		printk("%s: no card, disable it (RST&CLK)\n", "PCIe0");
		return 0;
	}
#elif defined (CONFIG_RALINK_RT3883)
#if defined (CONFIG_PCIE_ONLY) || defined (CONFIG_PCIE_PCI_CONCURRENT)
	if ((RALINK_PCI1_STATUS & 0x1) == 0) {
		RALINK_RSTCTRL |= RALINK_PCIE_RST;
		RALINK_CLKCFG1 &= ~RALINK_PCIE_CLK_EN;
		//cgrstb, cgpdb, pexdrven0, pexdrven1, cgpllrstb, cgpllpdb, pexclken
		RALINK_PCIE_CLK_GEN &= 0x0fff3f7f;
		pcie_disable = 1;
		printk("%s: no card, disable it (RST&CLK)\n", "PCIe");
#if defined (CONFIG_PCIE_ONLY)
		return 0;
#endif
	}
#endif
	RALINK_PCI_ARBCTL = 0x79;
#endif

	RALINK_PCI_MEMBASE = 0xffffffff;			// valid for PCI host mode only
	RALINK_PCI_IOBASE = RALINK_PCI_IO_MAP_BASE;		// valid for PCI host mode only

#if defined (CONFIG_RALINK_MT7621)
#if defined (CONFIG_PCIE_PORT0)
	// PCIe0
	if ((pcie_link_status & 0x1) != 0) {
		RALINK_PCI0_BAR0SETUP_ADDR	= 0x7fff0001;	// open BAR0 (2GB)
		RALINK_PCI0_BAR1SETUP_ADDR	= 0x00000000;	// disable BAR1 (used in EP mode)
		RALINK_PCI0_IMBASEBAR0_ADDR	= BAR0_MEMORY_BASE;	// make BAR0
		RALINK_PCI0_CLASS		= 0x06040001;
		RALINK_PCI_PCIMSK_ADDR		|= (1<<20);	// enable PCIe0 interrupt
	}
#endif
#if defined (CONFIG_PCIE_PORT1)
	// PCIe1
	if ((pcie_link_status & 0x2) != 0) {
		RALINK_PCI1_BAR0SETUP_ADDR	= 0x7fff0001;	// open BAR0 (2GB)
		RALINK_PCI1_BAR1SETUP_ADDR	= 0x00000000;	// disable BAR1 (used in EP mode)
		RALINK_PCI1_IMBASEBAR0_ADDR	= BAR0_MEMORY_BASE;	// make BAR0
		RALINK_PCI1_CLASS		= 0x06040001;
		RALINK_PCI_PCIMSK_ADDR		|= (1<<21);	// enable PCIe1 interrupt
	}
#endif
#if defined (CONFIG_PCIE_PORT2)
	// PCIe2
	if ((pcie_link_status & 0x4) != 0) {
		RALINK_PCI2_BAR0SETUP_ADDR	= 0x7fff0001;	// open BAR0 (2GB)
		RALINK_PCI2_BAR1SETUP_ADDR	= 0x00000000;	// disable BAR1 (used in EP mode)
		RALINK_PCI2_IMBASEBAR0_ADDR	= BAR0_MEMORY_BASE;	// make BAR0
		RALINK_PCI2_CLASS		= 0x06040001;
		RALINK_PCI_PCIMSK_ADDR		|= (1<<22);	// enable PCIe2 interrupt
	}
#endif
#elif defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7628)
	//PCIe0
	RALINK_PCI0_BAR0SETUP_ADDR		= 0x7fff0001;	// open BAR0 (2GB)
	RALINK_PCI0_BAR1SETUP_ADDR		= 0x00000000;	// disable BAR1 (used in EP mode)
	RALINK_PCI0_IMBASEBAR0_ADDR		= BAR0_MEMORY_BASE;	// make BAR0
	RALINK_PCI0_CLASS			= 0x06040001;
	RALINK_PCI_PCIMSK_ADDR			= (1<<20);	// enable PCIe0 interrupt
#elif defined (CONFIG_RALINK_RT3883)
#if defined (CONFIG_PCI_ONLY) || defined (CONFIG_PCIE_PCI_CONCURRENT)
	//PCI
	RALINK_PCI0_BAR0SETUP_ADDR		= BAR0_MASK;	// disable BAR0
	RALINK_PCI0_IMBASEBAR0_ADDR		= BAR0_MEMORY_BASE;
	RALINK_PCI0_CLASS			= 0x00800001;
	RALINK_PCI0_BAR0SETUP_ADDR		= BAR0_MASK|1;	// open BAR0
	RALINK_PCI_PCIMSK_ADDR			= 0x000c0000;	// enable PCI interrupts
#endif
#if defined (CONFIG_PCIE_ONLY) || defined (CONFIG_PCIE_PCI_CONCURRENT)
	//PCIe
	if (!pcie_disable) {
		RALINK_PCI1_BAR0SETUP_ADDR	= BAR0_MASK;	// disable BAR0
		RALINK_PCI1_IMBASEBAR0_ADDR	= BAR0_MEMORY_BASE;
		RALINK_PCI1_CLASS		= 0x06040001;
		RALINK_PCI1_BAR0SETUP_ADDR	= BAR0_MASK|1;	// open BAR0
		RALINK_PCI_PCIMSK_ADDR		|= (1<<20);	// enable PCIe interrupt
	}
#endif
#endif

	ralink_pci_controller.io_map_base = mips_io_port_base;

	register_pci_controller(&ralink_pci_controller);

	return 0;
}

#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_MT7620) || \
    defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
arch_initcall(init_ralink_pci);
#else
#error Please disable CONFIG_PCI for unsupported SoC!
#endif

#endif /* CONFIG_PCI */
