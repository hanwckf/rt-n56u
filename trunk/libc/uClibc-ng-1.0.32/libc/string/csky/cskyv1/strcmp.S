/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#include <features.h>
#include <endian.h>

#ifdef WANT_WIDE
# define Wstrcmp wcscmp
# define Wstrcoll wcscoll
#else
# define Wstrcmp strcmp
# define Wstrcoll strcoll
#endif

/* FIXME attention!!! it may be a bug when WANT_WIDE define */
/*libc_hidden_proto(Wstrcmp)*/
	.align 2
	.global Wstrcmp
	.type   Wstrcmp, @function
Wstrcmp:
    mov        r6, r2

    or         r2, r3
    andi       r2, 0x3
    cmpnei     r2, 0x0     /* d or s is aligned ?*/
    bt         4f          /* if not aligned, goto 4f*/
    1:                   /* if aligned, load word each time.*/
    ldw        r2, (r6, 0)
    ldw        r7, (r3, 0)
    cmpne      r2, r7
    bt         1f       /* if d[i] != s[i], goto 1f */
    tstnbz     r2       /* if d[i] == s[i], check if d or s is at the end. */
    bf         3f       /* if at the end, goto 3f (finish comparing) */

    ldw        r2, (r6, 4)
    ldw        r7, (r3, 4)
    cmpne      r2, r7
    bt         1f
    tstnbz     r2
    bf         3f

    ldw        r2, (r6, 8)
    ldw        r7, (r3, 8)
    cmpne      r2, r7
    bt         1f
    tstnbz     r2
    bf         3f

    ldw        r2, (r6, 12)
    ldw        r7, (r3, 12)
    cmpne      r2, r7
    bt         1f
    tstnbz     r2
    bf         3f

    ldw        r2, (r6, 16)
    ldw        r7, (r3, 16)
    cmpne      r2, r7
    bt         1f
    tstnbz     r2
    bf         3f

    ldw        r2, (r6, 20)
    ldw        r7, (r3, 20)
    cmpne      r2, r7
    bt         1f
    tstnbz     r2
    bf         3f

    ldw        r2, (r6, 24)
    ldw        r7, (r3, 24)
    cmpne      r2, r7
    bt         1f
    tstnbz     r2
    bf         3f

    ldw        r2, (r6, 28)
    ldw        r7, (r3, 28)
    cmpne      r2, r7
    bt         1f
    tstnbz     r2
    bf         3f

    addi       r6, 32
    addi       r3, 32
    br         1b

#ifdef __CSKYBE__
    /* d[i] != s[i] in word, so we check byte 0 ? */
    1:
    xtrb0      r1, r2
    mov        r4, r1
    xtrb0      r1, r7
    cmpne      r4, r1
    bt         2f
    cmpnei     r4, 0
    bf         2f

    /* d[i] != s[i] in word, so we check byte 1 ? */
    xtrb1      r1, r2
    mov        r4, r1
    xtrb1      r1, r7
    cmpne      r4, r1
    bt         2f
    cmpnei     r4, 0
    bf         2f

    /* d[i] != s[i] in word, so we check byte 1 ? */
    xtrb2      r1, r2
    mov        r4, r1
    xtrb2      r1, r7
    cmpne      r4, r1
    bt         2f
    cmpnei     r4, 0
    bf         2f

    /* d[i] != s[i] in word, so we check byte 1 ? */
    xtrb3      r1, r2
    mov        r4, r1
    xtrb3      r1, r7

#else /* little endian */
    /* d[i] != s[i] in word, so we check byte 0 ? */
1:
    xtrb3      r1, r2
    mov        r4, r1
    xtrb3      r1, r7
    cmpne      r4, r1
    bt         2f
    cmpnei     r4, 0
    bf         2f

    /* d[i] != s[i] in word, so we check byte 1 ? */
    xtrb2      r1, r2
    mov        r4, r1
    xtrb2      r1, r7
    cmpne      r4, r1
    bt         2f
    cmpnei     r4, 0
    bf         2f

    /* d[i] != s[i] in word, so we check byte 1 ? */
    xtrb1      r1, r2
    mov        r4, r1
    xtrb1      r1, r7
    cmpne      r4, r1
    bt         2f
    cmpnei     r4, 0
    bf         2f

    /* d[i] != s[i] in word, so we check byte 1 ? */
    xtrb0      r1, r2
    mov        r4, r1
    xtrb0      r1, r7

#endif
    /* get the result when d[i] != s[i] */
2:
    subu       r4, r1
    mov        r2, r4
    jmp        r15

    /* return when d[i] == s[i] */
3:
    subu       r2, r7
    jmp        r15

     /* cmp when d or s is not aligned */
4:
     ldb       r2, (r6,0)
     ldb       r7, (r3, 0)
     cmpne     r2, r7
     bt        3b
     addi      r3, 1
     addi      r6, 1
     cmpnei    r2, 0
     bt        4b
     jmp       r15

     .size   Wstrcmp, .-Wstrcmp

libc_hidden_def(Wstrcmp)
.weak Wstrcmp
#ifndef __UCLIBC_HAS_LOCALE__
/* libc_hidden_proto(Wstrcoll) */
strong_alias(Wstrcmp,Wstrcoll)
libc_hidden_def(Wstrcoll)
#endif
