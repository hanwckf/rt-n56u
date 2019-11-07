/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H	1

#if !defined _SETJMP_H  && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

typedef struct
{
  unsigned long __sp; /* the return stack address */
  unsigned long __pc; /* pc: r15, return address */
  /*
   * ABIV1 is r8~r14
   * ABIV2 is r4~r11, r16~r17, r26~r31
   */
  unsigned long __regs[16];
} __jmp_buf[1];

#endif /* _BITS_SETJMP_H */
