/* brk system call for Linux/Nios2.
   Copyright (C) 1995, 1996, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

/* This must be initialized data because commons can't have aliases.  */
void *__curbrk attribute_hidden = 0;

int brk (void *addr)
{
    void *newbrk;
    register int r2 __asm__("r2") = TRAP_ID_SYSCALL;
    register int r3 __asm__("r3") = __NR_brk;
    register void *r4 __asm__("r4") = addr;

    __asm__ __volatile__ ("trap\n\t" : "=r"(newbrk) : "0"(r2), "r"(r3), "r"(r4));

    __curbrk = newbrk;

    if (newbrk < addr) {
      __set_errno (ENOMEM);
      return -1;
    }

    return 0;
}
libc_hidden_def(brk)
