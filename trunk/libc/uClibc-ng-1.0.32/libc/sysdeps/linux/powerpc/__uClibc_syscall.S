	.text
	.align 2
	.globl __uClibc_syscall
	.type	__uClibc_syscall,@function
__uClibc_syscall:
.Lsize:
	sc
	bnslr

	b	__syscall_error

	.size	__uClibc_syscall,.Lsize-__uClibc_syscall
