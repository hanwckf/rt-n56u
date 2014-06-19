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
#include <asm/mipsregs.h>

#include <asm/irq.h>
#include <asm/rt2880/surfboard.h>
#include <asm/rt2880/surfboardint.h>
#include <asm/rt2880/rt_mmap.h>
#include <asm/rt2880/eureka_ep430.h>

#ifdef CONFIG_KGDB
extern void breakpoint(void);
extern int remote_debug;
#endif

static void mask_ralink_irq(struct irq_data *id)
{
	if (id->irq > 5) {
		*(volatile u32 *)(RALINK_INTDIS) = (1 << id->irq);
	}
}

static void unmask_ralink_irq(struct irq_data *id)
{
	if (id->irq > 5) {
		*(volatile u32 *)(RALINK_INTENA) = (1 << id->irq);
	}
}

static struct irq_chip ralink_irq_chip = {
	.name		= "Ralink",
	.irq_mask	= mask_ralink_irq,
	.irq_mask_ack	= mask_ralink_irq,
	.irq_unmask	= unmask_ralink_irq,
};

unsigned int __cpuinit get_c0_compare_int(void)
{
	return RALINK_CPU_TIMER_IRQ;
}

static inline int ls1bit32(unsigned int x)
{
	int b = 31, s;

	s = 16; if (x << 16 == 0) s = 0; b -= s; x <<= s;
	s =  8; if (x <<  8 == 0) s = 0; b -= s; x <<= s;
	s =  4; if (x <<  4 == 0) s = 0; b -= s; x <<= s;
	s =  2; if (x <<  2 == 0) s = 0; b -= s; x <<= s;
	s =  1; if (x <<  1 == 0) s = 0; b -= s;

	return b;
}

void ralink_hw0_irqdispatch(int prio)
{
	unsigned long int_status;
	int irq;

	if (prio)
		int_status = *(volatile u32 *)(RALINK_IRQ1STAT);
	else
		int_status = *(volatile u32 *)(RALINK_IRQ0STAT);

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
#if defined(CONFIG_RALINK_TIMER_DFS)
#if defined(CONFIG_RALINK_RT2880)
	if (irq == 0) {
		irq = SURFBOARDINT_TIMER0;
	}
#else
	if (irq == 1) {
		irq = SURFBOARDINT_TIMER0; // DFS
	} else if (irq == 2) {
		irq = SURFBOARDINT_WDG;
	}
#endif
#endif

#if defined(CONFIG_RALINK_RT2880)
	if (irq == 3) {
#ifdef CONFIG_RALINK_GPIO
		/* cause gpio registered irq 7 */
		irq = SURFBOARDINT_GPIO;
#endif
	}
#else
	/* ILL_ACC */ 
	if (irq == 3) {
		irq = SURFBOARDINT_ILL_ACC;
	}
#endif

#if defined(CONFIG_RALINK_PCM) || defined (CONFIG_RALINK_PCM_MODULE)
	/* PCM */
	if (irq == 4) {
		irq = SURFBOARDINT_PCM;
	}
#endif
	/* UARTF */
	if (irq == 5) {
		irq = SURFBOARDINT_UART;
	}

	do_IRQ(irq);
}

void __init arch_init_irq(void)
{
	int i;

	mips_cpu_irq_init();

#if !defined(CONFIG_RALINK_RT2880)
	cp0_perfcount_irq = SURFBOARDINT_PC;
#endif

	for (i = 0; i <= SURFBOARDINT_END; i++)
		irq_set_chip_and_handler(i, &ralink_irq_chip, handle_level_irq);

#ifdef CONFIG_KGDB
	if (remote_debug) {
		set_debug_traps();
		breakpoint();
	}
#endif

	set_c0_status(ST0_IM);

	/* Enable global interrupt bit */
	*(volatile u32 *)(RALINK_INTENA) = M_SURFBOARD_GLOBAL_INT;
}

asmlinkage void plat_irq_dispatch(void)
{
	unsigned long pending;
#if defined(CONFIG_RALINK_RT2880)
	static unsigned int pci_order = 0;
#if defined(CONFIG_RT2880_ASIC)
#define PCI_STATUS_MASK1	0x0x40000
#define PCI_STATUS_MASK2	0x0x80000
#elif defined(CONFIG_RT2880_FPGA)
#define PCI_STATUS_MASK1	0x0x80000
#define PCI_STATUS_MASK2	0x0x40000
#endif
#endif
#if defined (CONFIG_RALINK_RT2880) || defined(CONFIG_RALINK_RT3883) || \
    defined (CONFIG_RALINK_MT7620) || defined(CONFIG_RALINK_MT7621)
	unsigned long pci_status;
#endif
	pending = read_c0_status() & read_c0_cause() & ST0_IM;
	if (!pending) {
		spurious_interrupt();
		return;
	}

	if (pending & CAUSEF_IP7) {
		do_IRQ(RALINK_CPU_TIMER_IRQ); // CPU Timer
		return;
	}

	if (pending & CAUSEF_IP5)
		do_IRQ(SURFBOARDINT_FE); // Frame Engine

	if (pending & CAUSEF_IP6)
		do_IRQ(SURFBOARDINT_WLAN); // Wireless

	if (pending & CAUSEF_IP4) {
#if defined(CONFIG_RALINK_RT2880)
		pci_order ^= 1;
		pci_status = RALINK_PCI_PCIINT_ADDR;
		if (!pci_order) {
			if (pci_status & PCI_STATUS_MASK1)
				do_IRQ(SURFBOARDINT_PCI_0);
			else
				do_IRQ(SURFBOARDINT_PCI_1);
		} else {
			if (pci_status & PCI_STATUS_MASK2)
				do_IRQ(SURFBOARDINT_PCI_1);
			else
				do_IRQ(SURFBOARDINT_PCI_0);
		}
#elif defined (CONFIG_RALINK_RT2883)
		do_IRQ(SURFBOARDINT_PCI_0);
#elif defined (CONFIG_RALINK_RT3883)
		pci_status = RALINK_PCI_PCIINT_ADDR;
		if (pci_status & 0x100000)
			do_IRQ(SURFBOARDINT_PCIE_0);
#if defined(CONFIG_PCI_ONLY) || defined(CONFIG_PCIE_PCI_CONCURRENT)
		else if (pci_status & 0x040000)
			do_IRQ(SURFBOARDINT_PCI_0);
		else
			do_IRQ(SURFBOARDINT_PCI_1);
#endif
#elif defined (CONFIG_RALINK_MT7620) || defined(CONFIG_RALINK_MT7621)
		pci_status = RALINK_PCI_PCIINT_ADDR;
		if (pci_status & 0x100000)
			do_IRQ(SURFBOARDINT_PCIE_0);
#endif
	}

	if (pending & CAUSEF_IP3)
		ralink_hw0_irqdispatch(1);

	if (pending & CAUSEF_IP2)
		ralink_hw0_irqdispatch(0);

#if 0
	/* clear new potentially pending IP6..IP2 */
	set_c0_status( STATUSF_IP6 | STATUSF_IP5 | STATUSF_IP4 | STATUSF_IP3 | STATUSF_IP2 );
#endif
}

