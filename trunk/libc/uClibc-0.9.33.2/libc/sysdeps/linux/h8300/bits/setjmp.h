
/* Copyright (C) 2004, Yoshinori Sato <ysato@users.sourceforge.jp> */
/* Define the machine-dependent type `jmp_buf'.  H8/300 version.  */

#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H	1

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

#ifndef	_ASM

typedef struct
  {
    unsigned long __regs[4];  /* save er4 - er7(sp) */
    unsigned long __pc;       /* the return address */
  } __jmp_buf[1];

#endif /* _ASM */

#define JB_REGS   0
#define JB_PC     16
#define JB_SIZE   20


/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((void *) (address) < (void *) (jmpbuf)->__regs[3])

#endif	/* bits/setjmp.h */
