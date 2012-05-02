/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/*
 * SiliconBackplane Chipcommon core hardware definitions.
 *
 * The chipcommon core provides chip identification, SB control,
 * jtag, 0/1/2 uarts, clock frequency control, a watchdog interrupt timer,
 * gpio interface, extbus, and support for serial and parallel flashes.
 *
 * $Id: sbchipc.h,v 1.1 2007/06/08 10:20:42 arthur Exp $
 * Copyright(c) 2003 ASUSTeK Inc.
 */

#ifndef	_SBCHIPC_H
#define	_SBCHIPC_H


/* cpp contortions to concatenate w/arg prescan */
#ifndef PAD
#define	_PADLINE(line)	pad ## line
#define	_XSTR(line)	_PADLINE(line)
#define	PAD		_XSTR(__LINE__)
#endif	/* PAD */

typedef volatile struct {
	uint32	chipid;			/* 0x0 */
	uint32	capabilities;
	uint32	corecontrol;		/* corerev >= 1 */
	uint32	PAD[5];

	/* Interrupt control */
	uint32	intstatus;		/* 0x20 */
	uint32	intmask;
	uint32	PAD[6];

	/* serial flash interface registers */
	uint32	flashcontrol;		/* 0x40 */
	uint32	flashaddress;
	uint32	flashdata;
	uint32	PAD[1];

	/* Silicon backplane configuration broadcast control */
	uint32	broadcastaddress;
	uint32	broadcastdata;
	uint32	PAD[2];

	/* gpio - cleared only by power-on-reset */
	uint32	gpioin;			/* 0x60 */
	uint32	gpioout;
	uint32	gpioouten;
	uint32	gpiocontrol;
	uint32	gpiointpolarity;
	uint32	gpiointmask;
	uint32	PAD[2];

	/* Watchdog timer */
	uint32	watchdog;		/* 0x80 */
	uint32	PAD[3];

	/* clock control */
	uint32	clockcontrol_n;		/* 0x90 */
	uint32	clockcontrol_sb;	/* aka m0 */
	uint32	clockcontrol_pci;	/* aka m1 */
	uint32	clockcontrol_m2;	/* mii/uart/mem */
	uint32	clockcontrol_mips;	/* aka m3 */
	uint32	uart_clkdiv;		/* corerev >= 3 */
	uint32	PAD[2];

	/* pll delay registers (corerev >= 4) */
	uint32	pll_on_delay;		/* 0xb0 */
	uint32	fref_sel_delay;
	uint32	slow_clk_ctl;
	uint32	PAD[17];

	/* ExtBus control registers (corerev >= 3) */
	uint32	cs01config;		/* 0x100 */
	uint32	cs01memwaitcnt;
	uint32	cs01attrwaitcnt;
	uint32	cs01iowaitcnt;
	uint32	cs23config;
	uint32	cs23memwaitcnt;
	uint32	cs23attrwaitcnt;
	uint32	cs23iowaitcnt;
	uint32	cs4config;
	uint32	cs4waitcnt;
	uint32	parallelflashconfig;
	uint32	parallelflashwaitcnt;
	uint32	PAD[116];

	/* uarts */
	uint8	uart0data;		/* 0x300 */
	uint8	uart0imr;
	uint8	uart0fcr;
	uint8	uart0lcr;
	uint8	uart0mcr;
	uint8	uart0lsr;
	uint8	uart0msr;
	uint8	uart0scratch;
	uint8	PAD[248];		/* corerev >= 1 */

	uint8	uart1data;		/* 0x400 */
	uint8	uart1imr;
	uint8	uart1fcr;
	uint8	uart1lcr;
	uint8	uart1mcr;
	uint8	uart1lsr;
	uint8	uart1msr;
	uint8	uart1scratch;
} chipcregs_t;

/* chipid */
#define	CID_ID_MASK		0x0000ffff		/* Chip Id mask */
#define	CID_REV_MASK		0x000f0000		/* Chip Revision mask */
#define	CID_REV_SHIFT		16			/* Chip Revision shift */
#define	CID_PKG_MASK		0x00f00000		/* Package Option mask */
#define	CID_PKG_SHIFT		20			/* Package Option shift */
#define	CID_CC_MASK		0x0f000000		/* CoreCount (corerev >= 4) */
#define CID_CC_SHIFT		24

/* capabilities */
#define	CAP_UARTS_MASK		0x00000003		/* Number of uarts */
#define CAP_MIPSEB		0x00000004		/* MIPS is in big-endian mode */
#define CAP_UCLKSEL		0x00000018		/* UARTs clock select */
#define CAP_UINTCLK		0x00000008		/* UARTs are driven by internal divided clock */
#define CAP_UARTGPIO		0x00000020		/* UARTs own Gpio's 15:12 */
#define CAP_EXTBUS		0x00000040		/* External bus present */
#define	CAP_FLASH_MASK		0x00000700		/* Type of flash */
#define	CAP_PLL_MASK		0x00030000		/* Type of PLL */
#define CAP_PWR_CTL		0x00040000		/* Power control */

/* PLL capabilities */
#define PLL_NONE		0x00000000
#define PLL_N3M			0x00010000
#define PLL_N4M			0x00020000

/* corecontrol */
#define CC_UARTCLKO		0x00000001		/* Drive UART with internal clock */
#define	CC_SE			0x00000002		/* sync clk out enable (corerev >= 3) */

/* intstatus/intmask */
#define	CI_EI			0x00000002		/* ro: ext intr pin (corerev >= 3) */

/* slow_clk_ctl */
#define SCC_SLOW_SEL_MASK	0x00000007		/* source of slow clock */
#define	SCC_SLOWSEL_LPO		0x00000000
#define	SCC_SLOWSEL_XTAL	0x00000001
#define	SCC_SLOWSEL_PCI		0x00000002
#define SCC_LPO_FREQ_SEL	0x00000200		/* frequency of LPO, 1/0: 160K/32k */
#define SCC_LPO_PWR_DOWN	0x00000400		/* 1: LPO is disabled, 0: LPO is enabled */
#define SCC_FORCE_SLOW		0x00000800		/* 1: sb and cores running on slow clock, 0: power logic control */
#define SCC_FORCE_PLL		0x00001000		/* 1/0: power logic ignores/honors PLL clock disable requests from core */
#define SCC_DYN_XTAL		0x00002000		/* 1/0: power logic does/doesn't disable crystal when appropriate */
#define SCC_XTAL_PU		0x00004000		/* RO XTAL_PU pad value, 1/0: crystal running/disabled */
#define SCC_CLK_DIV_MASK	0xffff0000		/* 1: Divider to derive slow clock from source, 0: slow clock is half of the source */
#define SCC_CLK_DIV_SHF		16			/* clock divider shift */

/* clockcontrol_n */
#define	CN_N1_MASK		0x3f			/* n1 control */
#define	CN_N2_MASK		0x1f00			/* n2 control */
#define	CN_N2_SHIFT		8

/* clockcontrol_sb/pci/uart */
#define	CC_M1_MASK		0x3f			/* m1 control */
#define	CC_M2_MASK		0x1f00			/* m2 control */
#define	CC_M2_SHIFT		8
#define	CC_M3_MASK		0x3f0000		/* m3 control */
#define	CC_M3_SHIFT		16
#define	CC_MC_MASK		0x1f000000		/* mux control */
#define	CC_MC_SHIFT		24

/* N3M Clock control values for 125Mhz */
#define	CC_125_N		0x0802			/* Default values for bcm4310 */
#define	CC_125_M		0x04020009
#define	CC_125_M25		0x11090009
#define	CC_125_M33		0x11090005

/* N3M Clock control magic field values */
#define	CC_F6_2			0x02			/* A factor of 2 in */
#define	CC_F6_3			0x03			/* 6-bit fields like */
#define	CC_F6_4			0x05			/* N1, M1 or M3 */
#define	CC_F6_5			0x09
#define	CC_F6_6			0x11
#define	CC_F6_7			0x21

#define	CC_F5_BIAS		5			/* 5-bit fields get this added */

#define	CC_MC_BYPASS		0x08
#define	CC_MC_M1		0x04
#define	CC_MC_M1M2		0x02
#define	CC_MC_M1M2M3		0x01
#define	CC_MC_M1M3		0x11

/* N4M Clock control magic field values */
#define	CC_N4M_BIAS		2			/* n1, n2, m1 & m3 bias */
#define	CC_N4M2_BIAS		3			/* m2 bias */

#define	CC_N4MC_M1BYP		1
#define	CC_N4MC_M2BYP		2
#define	CC_N4MC_M3BYP		4

/* Common clock base */
#define	CC_CLOCK_BASE		24000000		/* Half the clock freq */

/* Flash types in the chipcommon capabilities register */
#define FLASH_NONE		0x000		/* No flash */
#define SFLASH_ST		0x100		/* ST serial flash */
#define SFLASH_AT		0x200		/* Atmel serial flash */
#define	PFLASH			0x700		/* Parallel flash */

/* Start/busy bit in flashcontrol */
#define SFLASH_START		0x80000000
#define SFLASH_BUSY		SFLASH_START

/* flashcontrol opcodes for ST flashes */
#define SFLASH_ST_WREN		0x0006		/* Write Enable */
#define SFLASH_ST_WRDIS		0x0004		/* Write Disable */
#define SFLASH_ST_RDSR		0x0105		/* Read Status Register */
#define SFLASH_ST_WRSR		0x0101		/* Write Status Register */
#define SFLASH_ST_READ		0x0303		/* Read Data Bytes */
#define SFLASH_ST_PP		0x0302		/* Page Program */
#define SFLASH_ST_SE		0x02d8		/* Sector Erase */
#define SFLASH_ST_BE		0x00c7		/* Bulk Erase */
#define SFLASH_ST_DP		0x00b9		/* Deep Power-down */
#define SFLASH_ST_RES		0x03ab		/* Read Electronic Signature */

/* Status register bits for ST flashes */
#define SFLASH_ST_WIP		0x01		/* Write In Progress */
#define SFLASH_ST_WEL		0x02		/* Write Enable Latch */
#define SFLASH_ST_BP_MASK	0x1c		/* Block Protect */
#define SFLASH_ST_BP_SHIFT	2
#define SFLASH_ST_SRWD		0x80		/* Status Register Write Disable */

/* flashcontrol opcodes for Atmel flashes */
#define SFLASH_AT_READ				0x07e8
#define SFLASH_AT_PAGE_READ			0x07d2
#define SFLASH_AT_BUF1_READ
#define SFLASH_AT_BUF2_READ
#define SFLASH_AT_STATUS			0x01d7
#define SFLASH_AT_BUF1_WRITE			0x0384
#define SFLASH_AT_BUF2_WRITE			0x0387
#define SFLASH_AT_BUF1_ERASE_PROGRAM		0x0283
#define SFLASH_AT_BUF2_ERASE_PROGRAM		0x0286
#define SFLASH_AT_BUF1_PROGRAM			0x0288
#define SFLASH_AT_BUF2_PROGRAM			0x0289
#define SFLASH_AT_PAGE_ERASE			0x0281
#define SFLASH_AT_BLOCK_ERASE			0x0250
#define SFLASH_AT_BUF1_WRITE_ERASE_PROGRAM	0x0382
#define SFLASH_AT_BUF2_WRITE_ERASE_PROGRAM	0x0385
#define SFLASH_AT_BUF1_LOAD			0x0253
#define SFLASH_AT_BUF2_LOAD			0x0255
#define SFLASH_AT_BUF1_COMPARE			0x0260
#define SFLASH_AT_BUF2_COMPARE			0x0261
#define SFLASH_AT_BUF1_REPROGRAM		0x0258
#define SFLASH_AT_BUF2_REPROGRAM		0x0259

/* Status register bits for Atmel flashes */
#define SFLASH_AT_READY				0x80
#define SFLASH_AT_MISMATCH			0x40
#define SFLASH_AT_ID_MASK			0x38
#define SFLASH_AT_ID_SHIFT			3

#endif	/* _SBCHIPC_H */
