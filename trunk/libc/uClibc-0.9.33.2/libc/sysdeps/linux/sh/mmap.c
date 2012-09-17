/* Copyright (C) 2001 Hewlett-Packard

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU Library General Public License as published by the Free
 Software Foundation; either version 2 of the License, or (at your option) any
 later version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the GNU Library General Public License for more
 details.

 You should have received a copy of the GNU Library General Public License
 along with this program; if not, write to the Free Software Foundation, Inc.,
 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

 Derived in part from the Linux-8086 C library, the GNU C Library, and several
 other sundry sources.  Files within this library are copyright by their
 respective copyright holders.
*/

#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>


#ifdef HIOS
# define __SH_SYSCALL6_TRAPA 0x2E
#endif

#include <sys/syscall.h>

_syscall6(__ptr_t, mmap, __ptr_t, addr, size_t, len, int, prot, int, flags, int, fd, __off_t, offset)
libc_hidden_def(mmap)
