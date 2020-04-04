#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H	1

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

#ifndef _ASM
typedef struct
  {
    int __regs[15]; /* callee-saved registers r11-r25 */
    void *__gp;     /* global pointer */
    void *__fp;     /* frame pointer */
    void *__sp;     /* stack pointer */
    void *__ra;     /* return address */
  } __jmp_buf[1];
#endif

/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((void *) (address) < (void *) (jmpbuf)[0].__sp)

#endif
