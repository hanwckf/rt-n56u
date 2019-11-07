/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#include <endian.h>
#include "macro.S"

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
    mov        a3, a0

    or         a0, a1
    andi       a0, 0x3
    M_BNEZ     a0, 4f
    1:                   // if aligned, load word each time.

    ldw        a0, (a3, 0)
    ldw        t0, (a1, 0)
    M_BNE      a0, t0, 1f // if d[i] != s[i], goto 1f
    tstnbz     a0       // if d[i] == s[i], check if d or s is at the end.
    bf         3f       // if at the end, goto 3f (finish comparing)
    ldw        a0, (a3, 4)
    ldw        t0, (a1, 4)
    M_BNE      a0, t0, 1f
    tstnbz     a0
    bf         3f

    ldw        a0, (a3, 8)
    ldw        t0, (a1, 8)
    M_BNE      a0, t0, 1f
    tstnbz     a0
    bf         3f

    ldw        a0, (a3, 12)
    ldw        t0, (a1, 12)
    M_BNE      a0, t0, 1f
    tstnbz     a0
    bf         3f

    ldw        a0, (a3, 16)
    ldw        t0, (a1, 16)
    M_BNE      a0, t0, 1f
    tstnbz     a0
    bf         3f

    ldw        a0, (a3, 20)
    ldw        t0, (a1, 20)
    M_BNE      a0, t0, 1f
    tstnbz     a0
    bf         3f

    ldw        a0, (a3, 24)
    ldw        t0, (a1, 24)
    M_BNE      a0, t0, 1f
    tstnbz     a0
    bf         3f

    ldw        a0, (a3, 28)
    ldw        t0, (a1, 28)
    M_BNE      a0, t0, 1f
    tstnbz     a0
    bf         3f

    addi       a3, 32
    addi       a1, 32
    br         1b

#ifdef __CSKYBE__
    /* d[i] != s[i] in word, so we check byte 0 ? */
    1:
    xtrb0      t1, a0
    mov        a2, t1
    xtrb0      t1, t0
    M_BNE      a2, t1, 2f
    cmpnei     a2, 0
    bf         2f

    /* d[i] != s[i] in word, so we check byte 1 ? */
    xtrb1      t1, a0
    mov        a2, t1
    xtrb1      t1, t0
    M_BNE      a2, t1, 2f
    cmpnei     a2, 0
    bf         2f

    /* d[i] != s[i] in word, so we check byte 1 ? */
    xtrb2      t1, a0
    mov        a2, t1
    xtrb2      t1, t0
    M_BNE      a2, t1, 2f
    cmpnei     a2, 0
    bf         2f

    /* d[i] != s[i] in word, so we check byte 1 ? */
    xtrb3      t1, a0
    mov        a2, t1
    xtrb3      t1, t0

#else /* little endian */
    /* d[i] != s[i] in word, so we check byte 0 ? */
    1:
    xtrb3      t1, a0
    mov        a2, t1
    xtrb3      t1, t0
    M_BNE      a2, t1, 2f
    cmpnei     a2, 0
    bf         2f

    /* d[i] != s[i] in word, so we check byte 1 ? */
    xtrb2      t1, a0
    mov        a2, t1
    xtrb2      t1, t0
    M_BNE      a2, t1, 2f
    cmpnei     a2, 0
    bf         2f

    /* d[i] != s[i] in word, so we check byte 1 ? */
    xtrb1      t1, a0
    mov        a2, t1
    xtrb1      t1, t0
    M_BNE      a2, t1, 2f
    cmpnei     a2, 0
    bf         2f

    /* d[i] != s[i] in word, so we check byte 1 ? */
    xtrb0      t1, a0
    mov        a2, t1
    xtrb0      t1, t0

#endif
    /* get the result when d[i] != s[i] */
    2:
    subu       a2, t1
    mov        a0, a2
    jmp        r15

    /* return when d[i] == s[i] */
    3:
    subu       a0, t0
    jmp        r15

     /* cmp when d or s is not aligned */
    4:
    ldb       a0, (a3,0)
    ldb       t0, (a1, 0)
    M_BNE     a0, t0, 3b
    addi      a1, 1
    addi      a3, 1
    M_BNEZ    a0, 4b
    jmp        r15

    .size   Wstrcmp, .-Wstrcmp

libc_hidden_def(Wstrcmp)
.weak Wstrcmp
#ifndef __UCLIBC_HAS_LOCALE__
/* libc_hidden_proto(Wstrcoll) */
strong_alias(Wstrcmp,Wstrcoll)
libc_hidden_def(Wstrcoll)
#endif
