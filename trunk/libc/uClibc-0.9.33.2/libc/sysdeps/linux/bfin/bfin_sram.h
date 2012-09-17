/*
 * bfin_sram.h - userspace interface to L1 memory allocator
 *
 * Copyright (c) 2007 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef __BFIN_SRAM_H__
#define __BFIN_SRAM_H__

#include <features.h>
#include <sys/types.h>

__BEGIN_DECLS

#define L1_INST_SRAM            0x00000001
#define L1_DATA_A_SRAM          0x00000002
#define L1_DATA_B_SRAM          0x00000004
#define L1_DATA_SRAM            0x00000006
#define L2_SRAM			0x00000008

extern void *sram_alloc(size_t size, unsigned long flags)
	__attribute_malloc__ __attribute_warn_unused_result__;
extern int sram_free(const void *addr);
extern void *dma_memcpy(void *dest, const void *src, size_t len)
	__nonnull((1, 2));

__END_DECLS

#endif
