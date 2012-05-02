/* glibc's sysdeps/x86_64/elf/initfini.c used for reference [EPILOG] */

.file	"initfini.c"

.section .init
.global  _init
.type    _init, %function
	addq $8, %rsp
	ret


.section .fini
.global  _fini
.type    _fini, %function
	addq $8, %rsp
	ret
