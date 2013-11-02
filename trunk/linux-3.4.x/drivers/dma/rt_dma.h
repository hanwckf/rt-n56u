/*
 * Copyright (C) 2007, 2008, Marvell International Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef RT_DMA_H
#define RT_DMA_H

#include <linux/types.h>
#include <linux/io.h>
#include <linux/dmaengine.h>
#include <linux/interrupt.h>

#include <asm/rt2880/rt_mmap.h>

#define RT_DMA_NAME             "rt_dma"

struct rt_dma_chan {
	int                     pending;
	dma_cookie_t            completed_cookie;
	spinlock_t              lock; /* protects the descriptor slot pool */
	void __iomem            *mmr_base;
	unsigned int            idx;
	enum dma_transaction_type       current_type;
	struct dma_async_tx_descriptor txd;
	struct list_head        chain;
	struct list_head        completed_slots;
	struct dma_chan         common;
	struct list_head        all_slots;
	int                     slots_allocated;
	struct tasklet_struct   irq_tasklet;
};

#define RT_DMA_READ_REG(addr)             le32_to_cpu(*(volatile u32 *)(addr))
#define RT_DMA_WRITE_REG(addr, val)       *((volatile uint32_t *)(addr)) = cpu_to_le32(val)

#define RT_DMA_SRC_REG(ch)                (RALINK_GDMA_BASE + ch*16)
#define RT_DMA_DST_REG(ch)                (RT_DMA_SRC_REG(ch) + 4)
#define RT_DMA_CTRL_REG(ch)               (RT_DMA_DST_REG(ch) + 4)
#define RT_DMA_CTRL_REG1(ch)              (RT_DMA_CTRL_REG(ch) + 4)
#define RT_DMA_DONEINT			  (RALINK_GDMA_BASE + 0x204)

#endif
