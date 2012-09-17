/* Copyright (C) 1997, 1998, 1999, 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Daniel Jacobowitz <dan@debian.org>, 1999.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* Massivly hacked up for uClibc by Erik Andersen */

/* Extracted from ../common/mmap64.c by Alexandre Oliva <aoliva@redhat.com>

   We don't want to use the old mmap interface.  */

#include <features.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/mman.h>


#define __NR___syscall_mmap2	    __NR_mmap2
static __inline__ _syscall6(__ptr_t, __syscall_mmap2, __ptr_t, addr,
	size_t, len, int, prot, int, flags, int, fd, off_t, offset)

/* This is always 12, even on architectures where PAGE_SHIFT != 12.  */
# ifndef MMAP2_PAGE_SHIFT
#  define MMAP2_PAGE_SHIFT 12
# endif

__ptr_t mmap(__ptr_t addr, size_t len, int prot, int flags, int fd, __off_t offset)
{
    if (offset & ((1 << MMAP2_PAGE_SHIFT) - 1)) {
	__set_errno (EINVAL);
	return MAP_FAILED;
    }
    return(__syscall_mmap2(addr, len, prot, flags, fd, (off_t) (offset >> MMAP2_PAGE_SHIFT)));
}
libc_hidden_def(mmap)
