#ifndef _LINUX_SPARC_SYSDEP_H
#define _LINUX_SPARC_SYSDEP_H 1

#include <common/sysdep.h>

#undef ENTRY
#undef END

#ifdef __ASSEMBLER__

#define LOADSYSCALL(x) mov __NR_##x, %g1

#define ENTRY(name)                 \
    .align 4;                       \
    .global C_SYMBOL_NAME(name);    \
    .type   name, @function;        \
C_LABEL(name)                       \
    cfi_startproc;

#define END(name)                   \
    cfi_endproc;                    \
    .size name, . - name

#define LOC(name) .L##name

	/* If the offset to __syscall_error fits into a signed 22-bit
	 * immediate branch offset, the linker will relax the call into
	 * a normal branch.
	 */
#undef PSEUDO
#undef PSEUDO_END
#undef PSEUDO_NOERRNO
#undef PSEUDO_ERRVAL

#define PSEUDO(name, syscall_name, args)	\
	.text;					\
	.globl		__syscall_error;	\
ENTRY(name);					\
	LOADSYSCALL(syscall_name);		\
	ta		0x10;			\
	bcc		1f;			\
	 mov		%o7, %g1;		\
	call		__syscall_error;	\
	 mov		%g1, %o7;		\
1:

#define PSEUDO_NOERRNO(name, syscall_name, args)\
	.text;					\
ENTRY(name);					\
	LOADSYSCALL(syscall_name);		\
	ta		0x10;

#define PSEUDO_ERRVAL(name, syscall_name, args)	\
	.text;					\
ENTRY(name);					\
	LOADSYSCALL(syscall_name);		\
	ta		0x10;

#define PSEUDO_END(name)			\
	END(name)


#endif /* __ASSEMBLER__ */

/* Pointer mangling is not yet supported for SPARC.  */
#define PTR_MANGLE(var) (void) (var)
#define PTR_DEMANGLE(var) (void) (var)

#endif
