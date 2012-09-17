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
        
#ifndef __NR_vfork
#define __NR_vfork __NR_fork /* uClinux-2.0 only has fork which is vfork */
#endif
     
        .globl _vfork
_vfork: 
        mov         g13, r3
        ldconst     __NR_vfork, g13
        calls       0
        mov         r3, g13
        cmpible     0, g0, 1f
        subo        g0, 0, r3
        st          r3, _errno
1:      
        ret

