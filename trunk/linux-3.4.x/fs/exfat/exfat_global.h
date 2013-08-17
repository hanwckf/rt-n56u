/*
 *  Copyright (C) 2012-2013 Samsung Electronics Co., Ltd.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef _EXFAT_GLOBAL_H
#define _EXFAT_GLOBAL_H

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/fs.h>

#include "exfat_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TRUE
#define TRUE                    1
#endif
#ifndef FALSE
#define FALSE                   0
#endif
#ifndef OK
#define OK                      0
#endif
#ifndef FAIL
#define FAIL                    1
#endif
#ifndef NULL
#define NULL                    0
#endif

#define MIN(a, b)               (((a) < (b)) ? (a) : (b))
#define MAX(a, b)               (((a) > (b)) ? (a) : (b))

	typedef char                    INT8;   
	typedef short                   INT16;  
	typedef int                     INT32; 
	typedef long long               INT64; 
	typedef unsigned char           UINT8;  
	typedef unsigned short          UINT16;
	typedef unsigned int            UINT32;
	typedef unsigned long long      UINT64;
	typedef unsigned char           BOOL;

#ifdef MALLOC
#undef MALLOC
#endif
#ifdef FREE
#undef FREE
#endif
#ifdef MEMSET
#undef MEMSET
#endif
#ifdef MEMCPY
#undef MEMCPY
#endif
#ifdef MEMCMP
#undef MEMCMP
#endif

#define MALLOC(size)                    kmalloc(size, GFP_KERNEL)
#define FREE(mem)                       if (mem) kfree(mem)
#define MEMSET(mem, value, size)        memset(mem, value, size)
#define MEMCPY(dest, src, size)         memcpy(dest, src, size)
#define MEMCMP(mem1, mem2, size)        memcmp(mem1, mem2, size)
#define COPY_DENTRY(dest, src)				memcpy(dest, src, sizeof(DENTRY_T))

#define STRCPY(dest, src)               strcpy(dest, src)
#define STRNCPY(dest, src, n)           strncpy(dest, src, n)
#define STRCAT(str1, str2)              strcat(str1, str2)
#define STRCMP(str1, str2)              strcmp(str1, str2)
#define STRNCMP(str1, str2, n)          strncmp(str1, str2, n)
#define STRLEN(str)                     strlen(str)

	INT32 __wstrchr(UINT16 *str, UINT16 wchar);
	INT32 __wstrlen(UINT16 *str);

#define WSTRCHR(str, wchar)             __wstrchr(str, wchar)
#define WSTRLEN(str)                    __wstrlen(str)

#if EXFAT_CONFIG_DEBUG_MSG
#define PRINTK(...)			\
	do {								\
		printk("[EXFAT] " __VA_ARGS__);	\
	} while(0)
#else
#define PRINTK(...)
#endif

	void    Bitmap_set_all(UINT8 *bitmap, INT32 mapsize);
	void    Bitmap_clear_all(UINT8 *bitmap, INT32 mapsize);
	INT32   Bitmap_test(UINT8 *bitmap, INT32 i);
	void    Bitmap_set(UINT8 *bitmap, INT32 i);
	void    Bitmap_clear(UINT8 *bitmpa, INT32 i);
	void    Bitmap_nbits_set(UINT8 *bitmap, INT32 offset, INT32 nbits);
	void    Bitmap_nbits_clear(UINT8 *bitmap, INT32 offset, INT32 nbits);

	void    my_itoa(INT8 *buf, INT32 v);
	INT32   my_log2(UINT32 v);

#ifdef PRINT
#undef PRINT
#endif

#define PRINT                   printk
#ifdef __cplusplus
}
#endif
#endif
