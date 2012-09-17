/*
 * clone.S, part of the i960 support for the uClibc library.
 *
 * Copyright (C) 2002 by Okiok Data Ltd.  http://www.okiok.com/
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */
/*
 * Derived from an old port of uC-libc to the i960 by Keith Adams (kma@cse.ogi.edu).
 */

#include <sys/syscall.h>
        
#ifndef __NR_vfork
#define __NR_vfork __NR_fork /* uClinux-2.0 only has fork which is vfork */
#endif
     
        .globl	___vfork
        .hidden	___vfork
___vfork:
        mov         g13, r3
        ldconst     __NR_vfork, g13
        calls       0
        mov         r3, g13
        cmpible     0, g0, 1f
        subo        g0, 0, r3
        st          r3, _errno
1:      
        ret

weak_alias(__vfork,vfork)
libc_hidden_weak(vfork)
