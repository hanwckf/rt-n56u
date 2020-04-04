/*
 * Copyright (C) 2016 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* brk system call for Linux/NDS32.
   Copyright (C) 1995, 1996 Free Software Foundation, Inc.

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

#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>

/* This must be initialized data because commons can't have aliases.  */
void *__curbrk attribute_hidden = 0;

libc_hidden_proto(brk)
int
brk (void *addr)
{
  void *newbrk;

#ifdef NDS32_ABI_V0
  __asm__ __volatile__ ("move       $r0, %1      \n\t"   // save the argment in r0
                        "pushm	    $r7, $r8     \n\t"
			"sethi 	    $r7, hi20(%2)\n\t"   // put the syscall number in r7
			"ori   	    $r7, $r7, lo12(%2)\n\t"
			"syscall    0x7fff       \n\t"   // do the system call
			"popm	    $r7, $r8     \n\t"
                        "move       %0, $r5      \n\t"   // keep the return value
                        : "=r"(newbrk)
                        : "r"(addr), "i"(SYS_ify(brk))
                        : "$r0", "$r5");
#else
  __asm__ __volatile__ ("move       $r0, %1      \n\t"   // save the argment in r0
			"syscall    %2       \n\t"   	 // do the system call
                        "move       %0, $r0      \n\t"   // keep the return value
                        : "=r"(newbrk)
                        : "r"(addr), "i"(SYS_ify(brk))
                        : "$r0");
#endif

  __curbrk = newbrk;

  if (newbrk < addr)
    {
      __set_errno (ENOMEM);
      return -1;
    }

  return 0;
}
libc_hidden_def(brk)
