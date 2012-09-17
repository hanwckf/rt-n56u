#
# __mmap.S, part of the i960 support for the uClibc library.
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

/* This is a plain system call.  The 6 arguments are already set up correctly */
/* void  *  mmap(void  *start,  size_t length, int prot , int flags, int fd, off_t offset) */
     
           
        .globl _mmap
_mmap:  
        mov     g13, r3
        ldconst __NR_mmap, g13
        calls   0
        mov     r3, g13

        /* We now need to check if the return value is a small negative integer.                    */
        /* This is somewhat tricky as the return code (normally an address) is an                   */
        /* unsigned type, or an ordinal in i960 assembler.                                          */
        /* We'll use the fact that, integers from -256 to -1 are ordinals 0xFFFFFF00 to 0xFFFFFFFF. */
        /* So by checking that the return address is in the top range of the ordinals, we'll        */
        /* in fact be checking if it's not an encoded negated erro code.                            */

        /* The range -256 to -1 should be enough since that in uClinux 2.0.39, there are            */
        /* 124 system calls for the i960.                                                           */
        
        ldconst 0xFFFFFF00, r3  /* This is the integer's -256 representation */
        cmpobl  g0, r3, 1f      /* Something smaller than this means it's out of the range, and a valid address */
        subi    g0, 0, r3       /* If it's an errno, save its negated (now positive) value in _errno. */
        st      r3, _errno      
        subi    1, 0, g0        /* And return -1. */
1:
        ret
