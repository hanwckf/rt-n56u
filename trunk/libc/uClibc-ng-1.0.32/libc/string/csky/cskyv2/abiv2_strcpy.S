/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#include <endian.h>
#include "macro.S"

#ifdef WANT_WIDE
# define Wstrcpy wcscpy
#else
# define Wstrcpy strcpy
#endif

	.align 2
	.global Wstrcpy
	.type   Wstrcpy, @function
Wstrcpy:

        mov     a3, a0
        or      a2, a1, a3
        andi    t0, a2, 3
        M_BEZ   t0, 2f
        mov     t0, a1
       1:
        ld.b    a2, (t0)
        stb     a2, (a3)
        addi    t0, t0, 1
        addi    a3, a3, 1
        M_BNEZ  a2, 1b

        jmp     r15

       2:
        ldw     a2, (a1)
        tstnbz  a2
        bf      11f
        stw     a2, (a3)

        ldw     a2, (a1, 4)
        tstnbz  a2
        bf      4f
        stw     a2, (a3, 4)

        ldw     a2, (a1, 8)
        tstnbz  a2
        bf      5f
        stw     a2, (a3, 8)

        ldw     a2, (a1, 12)
        tstnbz  a2
        bf      6f
        stw     a2, (a3, 12)

        ldw     a2, (a1, 16)
        tstnbz  a2
        bf      7f
        stw     a2, (a3, 16)

        ldw     a2, (a1, 20)
        tstnbz  a2
        bf      8f
        stw     a2, (a3, 20)

        ldw     a2, (a1, 24)
        tstnbz  a2
        bf      9f
        stw     a2, (a3, 24)

        ldw     a2, (a1, 28)
        tstnbz  a2
        bf      10f
        stw     a2, (a3, 28)

        addi     a3, 32
        addi     a1, 32
        br       2b

       4:
        addi    a3, 4
        br      11f

       5:
        addi    a3, 8
        br      11f

       6:
        addi    a3, 12
        br      11f

       7:
        addi    a3, 16
        br      11f

       8:
        addi    a3, 20
        br      11f

       9:
        addi    a3, 24
        br      11f

       10:
        addi    a3, 28
       11:
#ifdef __CSKYBE__
        xtrb0   t0, a2
        st.b    t0, (a3)
        M_BEZ   t0, 5f
        xtrb1   t0, a2
        st.b    t0, (a3, 1)
        M_BEZ   t0, 5f
        xtrb2   t0, a2
        st.b    t0, (a3, 2 )
        M_BEZ   t0, 5f
        stw     a2, (a3)
#else
        xtrb3   t0, a2
        st.b    t0, (a3)
        M_BEZ   t0, 5f
        xtrb2   t0, a2
        st.b    t0, (a3, 1)
        M_BEZ   t0, 5f
        xtrb1   t0, a2
        st.b    t0, (a3, 2)
        M_BEZ   t0, 5f
        stw     a2, (a3)
#endif
       5:
	jmp     r15


libc_hidden_def(Wstrcpy)
.weak Wstrcpy
