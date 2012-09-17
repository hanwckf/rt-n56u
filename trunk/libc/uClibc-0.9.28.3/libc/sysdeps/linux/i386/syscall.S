/*
 * June 27, 2001             Manuel Novoa III
 *
 * This is a heavily modified version of gcc's output for the _syscall5 macro.
 * The idea (originally from dietlibc) is that all syscall functions simply set
 * the syscall number as the first argument, then set the syscall arguments as
 * the next up-to-five arguments, and then jump here.  All the common work is
 * done by syscall(), saving a fair amount of generated code when a number of
 * syscalls are used.  The (potential) cost is some unnecessary pushes, pops,
 * and movs but the execution time penalty should be relatively small compared
 * to the cost of the syscall itself.
 *
 * July 24, 2002 
 *
 * Modified by Erik Andersen to take all function parameters from off the stack
 * like a proper function and eliminates the old 255 syscall number limit.  So
 * now we can just call this as a function as syscall() per the function
 * prototype in unistd.h, so to call _exit(42) you can just call.
 *         syscall(__NR_exit, 42);
 * and things will just work.
 */

.text
.global syscall
.type   syscall,%function
.align 4
syscall:
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %ebx

	movl  44(%esp),%ebp	/* Load the 6 syscall argument registers */
	movl  40(%esp),%edi
	movl  36(%esp),%esi
	movl  32(%esp),%edx
	movl  28(%esp),%ecx
	movl  24(%esp),%ebx
	movl  20(%esp),%eax	/* Load syscall number into %eax.  */
	int $0x80

	popl %ebx
	popl %esi
	popl %edi
	popl %ebp

	cmpl $-4095,%eax
	jae  __syscall_error
	ret			/* Return to caller.  */

.size syscall,.-syscall
