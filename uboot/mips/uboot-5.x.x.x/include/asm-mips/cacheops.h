/*
 * Cache operations for the cache instruction.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * (C) Copyright 1996, 1997 by Ralf Baechle
 */
#ifndef	__ASM_MIPS_CACHEOPS_H
#define	__ASM_MIPS_CACHEOPS_H

/*
 * Cache Operations
 */
#define Index_Invalidate_I      0x00
#define Index_Writeback_Inv_D   0x01
#define Index_Invalidate_SI     0x02
#define Index_Writeback_Inv_SD  0x03
#define Index_Load_Tag_I	0x04
#define Index_Load_Tag_D	0x05
#define Index_Load_Tag_SI	0x06
#define Index_Load_Tag_SD	0x07
#define Index_Store_Tag_I	0x08
#define Index_Store_Tag_D	0x09
#define Index_Store_Tag_SI	0x0A
#define Index_Store_Tag_SD	0x0B
#define Create_Dirty_Excl_D	0x0d
#define Create_Dirty_Excl_SD	0x0f
#define Hit_Invalidate_I	0x10
#define Hit_Invalidate_D	0x11
#define Hit_Invalidate_SI	0x12
#define Hit_Invalidate_SD	0x13
#define Fill			0x14
#define Hit_Writeback_Inv_D	0x15
					/* 0x16 is unused */
#define Hit_Writeback_Inv_SD	0x17
#define Hit_Writeback_I		0x18
#define Hit_Writeback_D		0x19
					/* 0x1a is unused */
#define Hit_Writeback_SD	0x1b
					/* 0x1c is unused */
					/* 0x1e is unused */
#define Hit_Set_Virtual_SI	0x1e
#define Hit_Set_Virtual_SD	0x1f

/*
 * Cache Operations available on all MIPS processors with R4000-style caches
 */
#define INDEX_INVALIDATE_I      0x00
#define INDEX_WRITEBACK_INV_D   0x01
#define INDEX_LOAD_TAG_I        0x04
#define INDEX_LOAD_TAG_D        0x05
#define INDEX_STORE_TAG_I       0x08
#define INDEX_STORE_TAG_D       0x09
#if defined(CONFIG_CPU_LOONGSON2)
#define HIT_INVALIDATE_I        0x00
#else
#define HIT_INVALIDATE_I        0x10
#endif
#define HIT_INVALIDATE_D        0x11
#define HIT_WRITEBACK_INV_D     0x15

/*
 * R4000-specific cacheops
 */
#define CREATE_DIRTY_EXCL_D     0x0d
#define FILL                    0x14
#define HIT_WRITEBACK_I         0x18
#define HIT_WRITEBACK_D         0x19

#endif	/* __ASM_MIPS_CACHEOPS_H */
