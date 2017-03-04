/*
 * Copyright (C) 2001 Palmchip Corporation.  All rights reserved.
 *
 * ########################################################################
 *
 *  This program is free software; you can distribute it and/or modify it
 *  under the terms of the GNU General Public License (Version 2) as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * ########################################################################
 *
 * Defines for the Surfboard interrupt controller.
 *
 */
#ifndef __ASM_MACH_MIPS_RT2880_SURFBOARDINT_H
#define __ASM_MACH_MIPS_RT2880_SURFBOARDINT_H

#if defined (CONFIG_RALINK_MT7621)

/* GCMP specific definitions */
#define GCMP_BASE_ADDR			0x1fbf8000
#define GCMP_ADDRSPACE_SZ		(32 * 1024)

/* CPC specific definitions */
#define CPC_BASE_ADDR			0x1fbf0000
#define CPC_ADDRSPACE_SZ		(32 * 1024)

/* GIC specific definitions */
#define GIC_BASE_ADDR			0x1fbc0000
#define GIC_ADDRSPACE_SZ		(128 * 1024)

/* MIPS GIC controller */
#define GIC_NUM_INTRS			64
#define MIPS_GIC_IRQ_BASE		(MIPS_CPU_IRQ_BASE + 8)
#define MIPS_GIC_LOCAL_INT_COMPARE	1				/* can be 0, 1, 2, 7 */

#define SURFBOARDINT_FE			(MIPS_GIC_IRQ_BASE + 3)		/* FE */
#define SURFBOARDINT_PCIE0		(MIPS_GIC_IRQ_BASE + 4)		/* PCIE0 */
#define SURFBOARDINT_AUX_TIMER		(MIPS_GIC_IRQ_BASE + 5)		/* AUX timer (systick) */
#define SURFBOARDINT_SYSCTL		(MIPS_GIC_IRQ_BASE + 6)		/* SYSCTL */
#define SURFBOARDINT_I2C		(MIPS_GIC_IRQ_BASE + 8)		/* I2C */
#define SURFBOARDINT_DRAMC		(MIPS_GIC_IRQ_BASE + 9)		/* DRAMC */
#define SURFBOARDINT_PCM		(MIPS_GIC_IRQ_BASE + 10)	/* PCM */
#define SURFBOARDINT_HSGDMA		(MIPS_GIC_IRQ_BASE + 11)	/* HSGDMA */
#define SURFBOARDINT_GPIO		(MIPS_GIC_IRQ_BASE + 12)	/* GPIO */
#define SURFBOARDINT_DMA		(MIPS_GIC_IRQ_BASE + 13)	/* GDMA */
#define SURFBOARDINT_NAND		(MIPS_GIC_IRQ_BASE + 14)	/* NAND */
#define SURFBOARDINT_NAND_ECC		(MIPS_GIC_IRQ_BASE + 15)	/* NFI ECC */
#define SURFBOARDINT_I2S		(MIPS_GIC_IRQ_BASE + 16)	/* I2S */
#define SURFBOARDINT_SPI		(MIPS_GIC_IRQ_BASE + 17)	/* SPI */
#define SURFBOARDINT_SPDIF		(MIPS_GIC_IRQ_BASE + 18)	/* SPDIF */
#define SURFBOARDINT_CRYPTO		(MIPS_GIC_IRQ_BASE + 19)	/* CryptoEngine */
#define SURFBOARDINT_SDXC		(MIPS_GIC_IRQ_BASE + 20)	/* SDXC */
#define SURFBOARDINT_R2P		(MIPS_GIC_IRQ_BASE + 21)	/* Rbus to Pbus */
#define SURFBOARDINT_USB		(MIPS_GIC_IRQ_BASE + 22)	/* USB */
#define SURFBOARDINT_ESW		(MIPS_GIC_IRQ_BASE + 23)	/* Switch */
#define SURFBOARDINT_PCIE1		(MIPS_GIC_IRQ_BASE + 24)	/* PCIE1 */
#define SURFBOARDINT_PCIE2		(MIPS_GIC_IRQ_BASE + 25)	/* PCIE2 */
#define SURFBOARDINT_UART_LITE1		(MIPS_GIC_IRQ_BASE + 26)	/* UART Lite */
#define SURFBOARDINT_UART_LITE2		(MIPS_GIC_IRQ_BASE + 27)	/* UART Lite */
#define SURFBOARDINT_UART_LITE3		(MIPS_GIC_IRQ_BASE + 28)	/* UART Lite */
#define SURFBOARDINT_WDG		(MIPS_GIC_IRQ_BASE + 29)	/* WDG timer */
#define SURFBOARDINT_TIMER0		(MIPS_GIC_IRQ_BASE + 30)	/* Timer0 */
#define SURFBOARDINT_TIMER1		(MIPS_GIC_IRQ_BASE + 31)	/* Timer1 */
#define SURFBOARDINT_UART1		SURFBOARDINT_UART_LITE1
#define SURFBOARDINT_UART2		SURFBOARDINT_UART_LITE2

#define SURFBOARDINT_END		(MIPS_GIC_IRQ_BASE + GIC_NUM_INTRS - 1)

/*
 * GIC IPI offsets is hardcoded for MT7621A/S as U-Boot:
 * PIN #1: 56..59 for VPE0..VPE3
 * PIN #2: 60..63 for VPE0..VPE3
 */
#define GIC_IPI_CALL_VPE0		(60)
#define GIC_IPI_RESCHED_VPE0		(56)

#else

#define MIPS_INTC_CHAIN_HW0		(MIPS_CPU_IRQ_BASE + 2)		/* Chain IP2 */
#define MIPS_INTC_CHAIN_HW1		(MIPS_CPU_IRQ_BASE + 3)		/* Chain IP3 */
#define SURFBOARDINT_PCIE0		(MIPS_CPU_IRQ_BASE + 4)		/* PCIe Slot0 */
#define SURFBOARDINT_FE			(MIPS_CPU_IRQ_BASE + 5)		/* Frame Engine */
#define SURFBOARDINT_WLAN		(MIPS_CPU_IRQ_BASE + 6)		/* Wireless */
#define SURFBOARDINT_MIPS_TIMER		(MIPS_CPU_IRQ_BASE + 7)		/* MIPS Timer */

#define INTC_NUM_INTRS			32
#define MIPS_INTC_IRQ_BASE		(MIPS_CPU_IRQ_BASE + 8)

#define SURFBOARDINT_SYSCTL		(MIPS_INTC_IRQ_BASE + 0)	/* SYSCTL */
#define SURFBOARDINT_ILL_ACC		(MIPS_INTC_IRQ_BASE + 3)	/* Illegal access */
#define SURFBOARDINT_PCM		(MIPS_INTC_IRQ_BASE + 4)	/* PCM */
#define SURFBOARDINT_GPIO		(MIPS_INTC_IRQ_BASE + 6)	/* GPIO */
#define SURFBOARDINT_DMA		(MIPS_INTC_IRQ_BASE + 7)	/* DMA */
#define SURFBOARDINT_NAND		(MIPS_INTC_IRQ_BASE + 8)	/* NAND */
#define SURFBOARDINT_PCTRL		(MIPS_INTC_IRQ_BASE + 9)	/* Performance counter */
#define SURFBOARDINT_I2S		(MIPS_INTC_IRQ_BASE + 10)	/* I2S */
#define SURFBOARDINT_SPI		(MIPS_INTC_IRQ_BASE + 11)	/* SPI */

#if defined (CONFIG_RALINK_RT3883)
#define SURFBOARDINT_PCI0		(MIPS_INTC_IRQ_BASE + 15)	/* PCI Slot0 */
#define SURFBOARDINT_PCI1		(MIPS_INTC_IRQ_BASE + 16)	/* PCI Slot1 */
#endif

#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7628)
#define SURFBOARDINT_SDXC		(MIPS_INTC_IRQ_BASE + 14)	/* SDXC */
#define SURFBOARDINT_R2P		(MIPS_INTC_IRQ_BASE + 15)	/* Rbus to Pbus */
#endif

#if defined (CONFIG_RALINK_MT7628)
#define SURFBOARDINT_AESENGINE		(MIPS_INTC_IRQ_BASE + 13)	/* AES Engine */
#define SURFBOARDINT_UART_LITE1		(MIPS_INTC_IRQ_BASE + 20)	/* UART Lite 1 */
#define SURFBOARDINT_UART_LITE2		(MIPS_INTC_IRQ_BASE + 21)	/* UART Lite 2 */
#define SURFBOARDINT_UART_LITE3		(MIPS_INTC_IRQ_BASE + 22)	/* UART Lite 3 */
#define SURFBOARDINT_WDG		(MIPS_INTC_IRQ_BASE + 23)	/* WDG timer */
#define SURFBOARDINT_TIMER0		(MIPS_INTC_IRQ_BASE + 24)	/* Timer0 */
#define SURFBOARDINT_TIMER1		(MIPS_INTC_IRQ_BASE + 25)	/* Timer1 */
#define SURFBOARDINT_PWM		(MIPS_INTC_IRQ_BASE + 26)	/* PWM */
#define SURFBOARDINT_WWAKE		(MIPS_INTC_IRQ_BASE + 27)	/* WLAN Wake */
#define SURFBOARDINT_UART1		SURFBOARDINT_UART_LITE1
#define SURFBOARDINT_UART2		SURFBOARDINT_UART_LITE2
#endif

#define SURFBOARDINT_ESW		(MIPS_INTC_IRQ_BASE + 17)	/* Embedded Switch */
#define SURFBOARDINT_UHST		(MIPS_INTC_IRQ_BASE + 18)	/* USB Host */
#define SURFBOARDINT_UDEV		(MIPS_INTC_IRQ_BASE + 19)	/* USB Device */


#if !defined (CONFIG_RALINK_MT7628)
#define SURFBOARDINT_TIMER0		(MIPS_INTC_IRQ_BASE + 1)	/* Timer0 */
#define SURFBOARDINT_WDG		(MIPS_INTC_IRQ_BASE + 2)	/* Watchdog timer */
#define SURFBOARDINT_UARTF		(MIPS_INTC_IRQ_BASE + 5)	/* UART Full */
#define SURFBOARDINT_UARTL		(MIPS_INTC_IRQ_BASE + 12)	/* UART Lite */
#define SURFBOARDINT_UART1		SURFBOARDINT_UARTL
#define SURFBOARDINT_UART2		SURFBOARDINT_UARTF
#endif

#define SURFBOARDINT_END		(MIPS_INTC_IRQ_BASE + INTC_NUM_INTRS - 1)

#endif

/*
 * Surfboard registers are memory mapped on 32-bit aligned boundaries and
 * only word access are allowed.
 */
#if defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
#define RALINK_IRQ0STAT		(RALINK_INTCL_BASE + 0x9C) //IRQ_STAT
#define RALINK_IRQ1STAT		(RALINK_INTCL_BASE + 0xA0) //FIQ_STAT
#define RALINK_INTTYPE		(RALINK_INTCL_BASE + 0x6C) //FIQ_SEL
#define RALINK_INTRAW		(RALINK_INTCL_BASE + 0xA4) //INT_PURE
#define RALINK_INTENA		(RALINK_INTCL_BASE + 0x80) //IRQ_MASK_SET
#define RALINK_FIQENA		(RALINK_INTCL_BASE + 0x84) //FIQ_MASK_SET
#define RALINK_INTDIS		(RALINK_INTCL_BASE + 0x78) //IRQ_MASK_CLR
#define RALINK_FIQDIS		(RALINK_INTCL_BASE + 0x7C) //FIQ_MASK_CLR
#else
#define RALINK_IRQ0STAT		(RALINK_INTCL_BASE + 0x00)
#define RALINK_IRQ1STAT		(RALINK_INTCL_BASE + 0x04)
#define RALINK_INTTYPE		(RALINK_INTCL_BASE + 0x20)
#define RALINK_INTRAW		(RALINK_INTCL_BASE + 0x30)
#define RALINK_INTENA		(RALINK_INTCL_BASE + 0x34)
#define RALINK_INTDIS		(RALINK_INTCL_BASE + 0x38)
#endif

/* bobtseng added ++, 2006.3.6. */
#define read_32bit_cp0_register(source)                         \
({ int __res;                                                   \
        __asm__ __volatile__(                                   \
        ".set\tpush\n\t"                                        \
        ".set\treorder\n\t"                                     \
        "mfc0\t%0,"STR(source)"\n\t"                            \
        ".set\tpop"                                             \
        : "=r" (__res));                                        \
        __res;})

#define write_32bit_cp0_register(register,value)                \
        __asm__ __volatile__(                                   \
        "mtc0\t%0,"STR(register)"\n\t"                          \
        "nop"                                                   \
        : : "r" (value));

/* bobtseng added --, 2006.3.6. */

#endif
