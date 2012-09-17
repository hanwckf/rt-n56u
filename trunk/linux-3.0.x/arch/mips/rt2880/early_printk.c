/*
 *  Ralink RT288x SoC early printk support
 *
 *  Copyright (C) 2009 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/io.h>

#include <asm/addrspace.h>

#include <asm/rt2880/rt_mmap.h>
#include "serial_rt2880.h"

#define UART_READ(r) \
	__raw_readl((void __iomem *)(KSEG1ADDR(RALINK_UART_LITE_BASE) + (r)))

#define UART_WRITE(r, v) \
	__raw_writel((v), (void __iomem *)(KSEG1ADDR(RALINK_UART_LITE_BASE) + (r)))

void prom_putchar(unsigned char ch)
{
	while (((UART_READ(UART_LSR)) & UART_LSR_THRE) == 0);
	UART_WRITE(UART_TX, ch);
	while (((UART_READ(UART_LSR)) & UART_LSR_THRE) == 0);
}
