/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     Interrupt routines for Ralink RT2880 solution
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
 * support RT3883 PCIe
 *
 **************************************************************************
 */

#include <linux/init.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/interrupt.h>

#include <asm/irq_cpu.h>
#include <asm/mipsregs.h>

#include <asm/rt2880/rt_mmap.h>
#include <asm/rt2880/eureka_ep430.h>

#if defined (CONFIG_RALINK_MT7628)
static unsigned int fiq_mask;
#endif

static void mask_ralink_irq(struct irq_data *id)
{
	unsigned int irq_mask = BIT(id->irq - MIPS_INTC_IRQ_BASE);

#if defined (CONFIG_RALINK_MT7628)
	if (irq_mask & fiq_mask)
		*(volatile u32 *)(RALINK_FIQDIS) = irq_mask;
	else
#endif
		*(volatile u32 *)(RALINK_INTDIS) = irq_mask;
}

static void unmask_ralink_irq(struct irq_data *id)
{
	unsigned int irq_mask = BIT(id->irq - MIPS_INTC_IRQ_BASE);

#if defined (CONFIG_RALINK_MT7628)
	if (irq_mask & fiq_mask)
		*(volatile u32 *)(RALINK_FIQENA) = irq_mask;
	else
#endif
		*(volatile u32 *)(RALINK_INTENA) = irq_mask;
}

static struct irq_chip ralink_irq_chip = {
	.name		= "INTC",
	.irq_mask	= mask_ralink_irq,
	.irq_mask_ack	= mask_ralink_irq,
	.irq_unmask	= unmask_ralink_irq,
	.irq_disable	= mask_ralink_irq,
	.irq_enable	= unmask_ralink_irq,
};

static void ralink_intc_handler(unsigned int irq, struct irq_desc *desc)
{
	unsigned int int_status;

	if (irq == MIPS_INTC_CHAIN_HW1)
		int_status = *(volatile u32 *)(RALINK_IRQ1STAT);
	else
		int_status = *(volatile u32 *)(RALINK_IRQ0STAT);

	if (unlikely(!int_status))
		return;

	generic_handle_irq(MIPS_INTC_IRQ_BASE + __ffs(int_status));
}

unsigned int __cpuinit get_c0_compare_int(void)
{
	return SURFBOARDINT_MIPS_TIMER;
}

void __init prom_init_irq(void)
{
	cp0_perfcount_irq = SURFBOARDINT_PCTRL;
}

void __init arch_init_irq(void)
{
	int i;
	unsigned int int_type = 0;

	mips_cpu_irq_init();

	/* disable all INTC interrupts */
#if defined (CONFIG_RALINK_MT7628)
	*(volatile u32 *)(RALINK_FIQDIS) = ~0u;
#endif
	*(volatile u32 *)(RALINK_INTDIS) = ~0u;

	/* route some INTC interrupts to MIPS HW1 interrupt (high priority) */
#ifdef RALINK_INTCTL_UHST
	int_type |= RALINK_INTCTL_UHST;
#endif
#ifdef RALINK_INTCTL_SDXC
	int_type |= RALINK_INTCTL_SDXC;
#endif
#ifdef RALINK_INTCTL_CRYPTO
	int_type |= RALINK_INTCTL_CRYPTO;
#endif
	int_type |= RALINK_INTCTL_DMA;
	*(volatile u32 *)(RALINK_INTTYPE) = int_type;

	for (i = 0; i < INTC_NUM_INTRS; i++) {
		irq_set_chip_and_handler(MIPS_INTC_IRQ_BASE + i,
			&ralink_irq_chip, handle_level_irq);
	}

	irq_set_chained_handler(MIPS_INTC_CHAIN_HW0, ralink_intc_handler);
	irq_set_chained_handler(MIPS_INTC_CHAIN_HW1, ralink_intc_handler);

	/* Enable global INTC interrupt bit */
#if defined (CONFIG_RALINK_MT7628)
	fiq_mask = int_type;
	*(volatile u32 *)(RALINK_FIQENA) = BIT(31);
#endif
	*(volatile u32 *)(RALINK_INTENA) = BIT(31);
}

asmlinkage void plat_irq_dispatch(void)
{
	unsigned int pending;

	pending = read_c0_status() & read_c0_cause() & ST0_IM;
	if (unlikely(!pending)) {
		spurious_interrupt();
		return;
	}

	if (pending & CAUSEF_IP7)
		do_IRQ(SURFBOARDINT_MIPS_TIMER);

	if (pending & CAUSEF_IP5)
		do_IRQ(SURFBOARDINT_FE);

	if (pending & CAUSEF_IP6)
		do_IRQ(SURFBOARDINT_WLAN);

	if (pending & CAUSEF_IP4) {
#if defined (CONFIG_RALINK_RT3883)
#if defined (CONFIG_PCI_ONLY) || defined (CONFIG_PCIE_PCI_CONCURRENT)
		unsigned int pci_status = RALINK_PCI_PCIINT_ADDR;
		if (pci_status & 0x040000)
			do_IRQ(SURFBOARDINT_PCI0);
		else if (pci_status & 0x080000)
			do_IRQ(SURFBOARDINT_PCI1);
		else
#endif
#endif
		do_IRQ(SURFBOARDINT_PCIE0);
	}

	if (pending & CAUSEF_IP3)
		do_IRQ(MIPS_INTC_CHAIN_HW1);

	if (pending & CAUSEF_IP2)
		do_IRQ(MIPS_INTC_CHAIN_HW0);
}

