#include <sysdep.h>

#define CLONE_VM      0x00000100
#define CLONE_THREAD  0x00010000

.text
ENTRY(__or1k_clone)

	/* To handle GCC varargs we need to use our __clone wrapper to pop
	   everything from the stack for us.
	   Now everything is placed in the registers which saves us a lot 
	   of trouble.

	   The userland implementation is:
		  int clone (int (*fn)(void *), void *child_stack,
			    int flags, void *arg, pid_t *ptid,
			    struct user_desc *tls, pid_t *ctid);

	     The kernel entry is:
		  int clone (long flags, void *child_stack, int *parent_tid,
			  int *child_tid, struct void *tls)
	     NB: tls isn't really an argument, it is read from r7 directly.
	*/

	/* Put 'fn', 'arg' and 'flags' on child stack */
	l.addi r4, r4, -12
	l.sw  8(r4), r3
	l.sw  4(r4), r6
	l.sw  0(r4), r5

	l.ori r3, r5, 0
	/* child_stack is already in r4 */
	l.ori r5, r7, 0
	l.lwz r6, 0(r1)
	l.ori r7, r8, 0

	DO_CALL (clone)

	l.sfgeui r11, 0xf001
	l.bf L(error)
	 l.nop

	/* If we are not the child, return the pid */
	l.sfeqi r11, 0
	l.bf L(child)
	 l.nop

	l.jr r9
	 l.nop

L(child):
	/* Load flags */
	l.lwz r3, 0(r1)

	/* Update PID, but only if we do not share the same PID
	   as our parent */
	l.srli  r4, r3, 16
	l.andi r4, r4, hi(CLONE_THREAD)
	l.sfnei r4, 0
	l.bf L(oldpid)
	 l.nop

L(oldpid):
	/* Load function from stack */
	l.lwz r11, 8(r1)
	l.jalr r11
	 l.lwz r3, 4(r1)

	/* Exit the child thread */
	l.jal HIDDEN_JUMPTARGET(_exit)
	 l.ori r3, r11, 0

L(error):
#ifdef __PIC__
	l.j plt(__syscall_error)
#else
	l.j __syscall_error
#endif
	 l.ori r3,r11,0

END (__or1k_clone)
