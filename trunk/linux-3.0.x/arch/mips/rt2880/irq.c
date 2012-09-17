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
 * support RT2880/RT2883 PCIe
 *
 *
 **************************************************************************
 */

#include <linux/init.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <linux/hardirq.h>
#include <linux/preempt.h>

#include <asm/irq_cpu.h>
#include <asm/irq.h>
#include <asm/rt2880/surfboard.h>
#include <asm/rt2880/surfboardint.h>
#include <asm/rt2880/rt_mmap.h>

#include <asm/rt2880/eureka_ep430.h>

#if defined(CONFIG_RT2880_ASIC)
#define PCI_STATUS_MASK1	0x0x40000
#define PCI_STATUS_MASK2	0x0x80000
#elif defined(CONFIG_RT2880_FPGA)
#define PCI_STATUS_MASK1	0x0x80000
#define PCI_STATUS_MASK2	0x0x40000
#endif

extern volatile unsigned int dma1_intstat;
extern volatile unsigned int dma1_rawstat;
extern volatile unsigned int surfpcmcia_intstat;
extern volatile unsigned int surfpcmcia_rawstat;

extern asmlinkage void mipsIRQ(void);
void mips_timer_interrupt(void);
void __init ralink_gpio_init_irq(void);

struct surfboard_ictrl_regs *surfboard_hw0_icregs = (struct surfboard_ictrl_regs *)RALINK_INTCL_BASE;

/* cpu pipeline flush */
void static inline ramips_sync(void)
{
	__asm__ volatile ("sync");
}

void mask_surfboard_irq(struct irq_data *d)
{
	if (d->irq > 5) {
		surfboard_hw0_icregs->intDisable = (1 << d->irq);
		ramips_sync();
	}
}

void unmask_surfboard_irq(struct irq_data *d)
{
	if (d->irq > 5) {
		surfboard_hw0_icregs->intEnable = (1 << d->irq);
		ramips_sync();
	}
}

static struct irq_chip surfboard_irq_type = {
	.name = "Surfboard",
	.irq_mask = mask_surfboard_irq,
	.irq_unmask = unmask_surfboard_irq,
};

static inline int ls1bit32(unsigned int x)
{
	int b = 31, s;

	s = 16;
	if (x << 16 == 0)
		s = 0;
	b -= s;
	x <<= s;
	s = 8;
	if (x << 8 == 0)
		s = 0;
	b -= s;
	x <<= s;
	s = 4;
	if (x << 4 == 0)
		s = 0;
	b -= s;
	x <<= s;
	s = 2;
	if (x << 2 == 0)
		s = 0;
	b -= s;
	x <<= s;
	s = 1;
	if (x << 1 == 0)
		s = 0;
	b -= s;

	return b;
}

void surfboard_hw0_irqdispatch(unsigned int prio)
{
	struct irqaction *action;
	unsigned long int_status;
	int irq;

	if (prio)
		int_status = surfboard_hw0_icregs->irq1Status;
	else
		int_status = surfboard_hw0_icregs->irq0Status;

	/* if int_status == 0, then the interrupt has already been cleared */
	if (int_status == 0)
		return;
	irq = ls1bit32(int_status);

	/*
	 * RT2880:
	 * bit[3] PIO Programmable IO Interrupt Status after Mask
	 * bit[2] UART Interrupt Status after Mask
	 * bit[1] WDTIMER Timer 1 Interrupt Status after Mask
	 * bit[0] TIMER0 Timer 0 Interrupt Status after Mask
	 *
	 * RT2883/RT3052:
	 * bit[17] Ethernet switch interrupt status after mask
	 * bit[6] PIO Programmable IO Interrupt Status after Mask
	 * bit[5] UART Interrupt Status after Mask
	 * bit[2] WDTIMER Timer 1 Interrupt Status after Mask
	 * bit[1] TIMER0 Timer 0 Interrupt Status after Mask
	 */
#ifdef CONFIG_RALINK_TIMER_DFS
#if defined (CONFIG_RALINK_RT2880_SHUTTLE) || \
    defined (CONFIG_RALINK_RT2880_MP)
	if (irq == 0) {
#else
	if (irq == 1) {
#endif
		irq = SURFBOARDINT_TIMER0;
	}
#endif

#if defined (CONFIG_RALINK_RT2880_SHUTTLE) ||   \
    defined (CONFIG_RALINK_RT2880_MP)
	if (irq == 3) {
#ifdef CONFIG_RALINK_GPIO
		/* cause gpio registered irq 7 (see rt2880gpio_init_irq()) */
		irq = SURFBOARDINT_GPIO;
		printk("surfboard_hw0_irqdispatch(): INT #7...\n");
#else
		printk("surfboard_hw0_irqdispatch(): External INT #3... surfboard discard!\n");
#endif
	}
#else
	/* ILL_ACC */
	if (irq == 3) {
		irq = SURFBOARDINT_ILL_ACC;
	}
#endif
#if defined (CONFIG_RALINK_PCM) || defined (CONFIG_RALINK_PCM_MODULE)
	/* PCM */
	if (irq == 4) {
		irq = SURFBOARDINT_PCM;
	}
#endif
	/* UARTF */
	if (irq == 5) {
		irq = SURFBOARDINT_UART;
	}

	action = irq_desc[irq].action;
	do_IRQ(irq);

	return;
}

void __init arch_init_irq(void)
{
	int i;

	mips_cpu_irq_init();

	for (i = 0; i <= SURFBOARDINT_END; i++)
		irq_set_chip_and_handler(i, &surfboard_irq_type, handle_level_irq);

	/* Enable global interrupt bit */
	surfboard_hw0_icregs->intEnable = M_SURFBOARD_GLOBAL_INT;
	ramips_sync();

#ifdef CONFIG_RALINK_GPIO
	ralink_gpio_init_irq();
#endif
	set_c0_status(IE_IRQ0 | IE_IRQ1 | IE_IRQ2 | IE_IRQ3 | IE_IRQ4 | IE_IRQ5);
}

void rt2880_irqdispatch(void)
{
	static unsigned int pci_order = 0;
	unsigned int irq = NR_IRQS, pending = read_c0_status() & read_c0_cause() & 0xFC00;
#if defined(CONFIG_RALINK_RT2880) || defined (CONFIG_RALINK_RT2883) || defined(CONFIG_RALINK_RT3883)
	unsigned long pci_status = 0;
#endif

	pci_order ^= 1;

	if (pending & CAUSEF_IP6)
		irq = RALINK_CPU_80211N_NIC;
	else if (pending & CAUSEF_IP5)
		irq = RALINK_CPU_FRAME_ENGINE;
	else if (pending & CAUSEF_IP4) {
#if defined (CONFIG_RALINK_RT2883)
		irq = 2;
#elif defined (CONFIG_RALINK_RT3883)
		pci_status = RALINK_PCI_PCIINT_ADDR;
		if (pci_status & 0x100000) {
			irq = 16;
		} else if (pci_status & 0x40000) {
			irq = 2;
		} else {
			irq = 15;
		}
#elif defined (CONFIG_RALINK_RT3052)
#elif defined (CONFIG_RALINK_RT3352)
#elif defined (CONFIG_RALINK_RT5350)
#else // 2880
#if defined(CONFIG_RALINK_RT2880) || defined(CONFIG_RALINK_RT3883)
		pci_status = RALINK_PCI_PCIINT_ADDR;
#endif
		if (pci_order == 0) {
			if (pci_status & PCI_STATUS_MASK1)
				irq = 2;
			else
				irq = 15;
		} else {
			if (pci_status & PCI_STATUS_MASK2)
				irq = 15;
			else
				irq = 2;
		}
#endif /* CONFIG_RALINK_RT2883 */
	}
	else if (pending & CAUSEF_IP3)
		surfboard_hw0_irqdispatch(1);
	else if (pending & CAUSEF_IP2)
		surfboard_hw0_irqdispatch(0);
	else
		spurious_interrupt();

	if (likely(irq < NR_IRQS))
		do_IRQ(irq);

	return;
}

asmlinkage void plat_irq_dispatch(void)
{
	unsigned int pending = read_c0_status() & read_c0_cause() & ST0_IM;
	if (pending & CAUSEF_IP7)
		do_IRQ(RALINK_CPU_TIMER_IRQ);
	else
		rt2880_irqdispatch();
}
