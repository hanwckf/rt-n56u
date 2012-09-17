/* Define the machine-dependent type `jmp_buf'.  i960 version.  */

#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H	1

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

/*
 * assume that every single local and global register
 * must be saved.
 *
 * ___SAVEREGS is the number of quads to save.
 *
 * Using the structure will guarantee quad-word alignment for the
 * jmp_buf type.
 */

#define ___SAVEREGS 8

typedef struct __jmp_buf__ {
  long _q0;
  long _q1;
  long _q2;
  long _q3;
} __attribute__ ((aligned (16))) __jmp_buf[___SAVEREGS] ;

/* I have not yet figured out what this should be for the i960... */

#if 0
/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((void *) (address) < (void *) (jmpbuf)[0].__sp)
#endif

#endif	/* bits/setjmp.h */
