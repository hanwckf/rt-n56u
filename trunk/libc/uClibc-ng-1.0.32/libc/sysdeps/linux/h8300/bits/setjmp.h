
/* Copyright (C) 2004, Yoshinori Sato <ysato@users.sourceforge.jp> */
/* Define the machine-dependent type `jmp_buf'.  H8/300 version.  */

#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H	1

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

typedef struct
  {
    unsigned long __regs[4];  /* save er4 - er7(sp) */
    unsigned long __pc;       /* the return address */
  } __jmp_buf[1];

#endif	/* bits/setjmp.h */
