/*
 * Copyright (C) 2013, Imagination Technologies Ltd.
 *
 * Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

/* Define the machine-dependent type `jmp_buf' */
#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H	1

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

/*
   jmp_buf[0] - A0StP
   jmp_buf[1] - A1GbP
   jmp_buf[2] - A0FrP
   jmp_buf[3] - A1LbP
   jmp_buf[4] - D0FrT
   jmp_buf[5] - D1RtP
   jmp_buf[6] - D0.5
   jmp_buf[7] - D1.5
   jmp_buf[8] - D0.6
   jmp_buf[9] - D1.6
   jmp_buf[10] - D0.7
   jmp_buf[11] - D1.7
   */

#define _JBLEN 24

typedef int __jmp_buf[_JBLEN] __attribute__((aligned (8)));

#endif	/* bits/setjmp.h */
