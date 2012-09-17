#
# clone.S, part of the i960 support for the uClibc library.
#
# Copyright (C) 2002 by Okiok Data Ltd.  http://www.okiok.com/
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU Library General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Library General Public License for more
# details.
#
# You should have received a copy of the GNU Library General Public License
# along with this program; if not, write to the Free Software Foundation, Inc.,
# at 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
# Derived from an old port of uC-libc to the i960 by Keith Adams (kma@cse.ogi.edu).
#

#include <sys/syscall.h>        
#include <bits/errno.h>
        
/* clone is even more special than fork as it mucks with stacks
   and invokes a function in the right context after its all over.  */
        
/* int _clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg); */

        .globl clone
clone: 
        /* set up new stack image in regs r4-r7; argument will be in r3 in child.  */
        ldconst 0, r4                   /* pfp == 0 */
        addo    16, g1, r5              /* sp == newfp + 16 */
        mov     g0, r6                  /* rip == fnc */
        mov     g2, r7
        stq     r4, (g1)

        addo    sp, 4, sp
        st      g10, -4(sp)
        mov     sp, g10
        ldconst __NR_clone, g13
        calls   0

        /* Do the system call */
        cmpibg  0, g0, __syscall_error    /* if < 0, error */
        be      thread_start            /* if == 0, we're the child */
        ret                             /* we're the parent */

__syscall_error:
        not     g0, r3
        callx   ___errno_location
        st      r3, (g0)
        ret

thread_start:
        # our new pfp is in g1; here we go
        flushreg
        mov     g1, pfp
        flushreg
        ret
