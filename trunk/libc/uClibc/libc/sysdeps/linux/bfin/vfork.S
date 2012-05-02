
#include <asm/unistd.h>
	.text
	.globl _vfork
	.globl __libc_vfork
	.type	 _vfork,STT_FUNC;
	.align 4
__libc_vfork:
_vfork:
	p0 = __NR_vfork;
	excpt 0;
	rts;
