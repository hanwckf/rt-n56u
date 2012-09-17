/* Copyright (C) 2003, 2004 Red Hat, Inc.
   Contributed by Alexandre Oliva <aoliva@redhat.com>

This file is part of uClibc.

uClibc is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.

uClibc is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with uClibc; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
USA.  */

#ifdef __NR_sram_alloc
#define __NR__dl_sram_alloc __NR_sram_alloc
static __always_inline _syscall2(__ptr_t, _dl_sram_alloc,
			size_t, len, unsigned long, flags)
#endif

#ifdef __NR_sram_free
#define __NR__dl_sram_free __NR_sram_free
static __always_inline _syscall1(int, _dl_sram_free, __ptr_t, addr)
#endif

#ifdef __NR_dma_memcpy
#define __NR__dl_dma_memcpy __NR_dma_memcpy
static __always_inline _syscall3(__ptr_t, _dl_dma_memcpy,
			__ptr_t, dest, __ptr_t, src, size_t, len)
#endif

#define __UCLIBC_MMAP_HAS_6_ARGS__
