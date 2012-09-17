/* Define the machine-dependent type `jmp_buf'.  Vax version. */

#ifndef _SETJMP_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

/* we want to save enough that we can use this to fool RET,
 * So we basically save all of the CALLS stack frame. Plus regs. */
#ifndef	_ASM
typedef int __jmp_buf[16];
#endif

/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((void *) (address) < (void *) (jmpbuf[4]))
/*
	jmp_buf layout. jmp_buf[0]
	void *__cond;		 The condition handler
	void *__psw;		 mask and PSW bits
        void *__ap;		 argument pointer
	void *__fp;		 frame pointer
	void *__pc;		 program counter
			         no need to save r0
	void *__r1;		 regs, r0->r11.
	void *__r2;		 regs, r0->r11.
	void *__r3;		 regs, r0->r11.
	void *__r4;		 regs, r0->r11.
	void *__r5;		 regs, r0->r11.
	void *__r6;		 regs, r0->r11.
	void *__r7;		 regs, r0->r11.
	void *__r8;		 regs, r0->r11.
	void *__r9;		 regs, r0->r11.
	void *__rA;		 regs, r0->r11.
	void *__rB;		 regs, r0->r11.
*/

