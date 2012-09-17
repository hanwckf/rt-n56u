.file "initfini.c"

.section .init
.global _init
.type   _init,%function
	popl	%ebx
	popl	%ebp
	ret



.section .fini
.global _fini
.type   _fini,%function
	popl	%ebx
	popl	%ebp
	ret



.section .gnu.linkonce.t.__get_pc_thunk_bx,"ax",@progbits
.global __get_pc_thunk_bx
.hidden	__get_pc_thunk_bx
.type   __get_pc_thunk_bx,%function
__get_pc_thunk_bx:
	movl	(%esp), %ebx
	ret
