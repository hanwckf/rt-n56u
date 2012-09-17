#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H

#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

/*
 * Some of the sneaky macros in the code were taken from
 * glibc-2.2.5/sysdeps/unix/sysv/linux/i386/sysdep.h
 */

#ifndef __ASSEMBLER__

#include <errno.h>

#define INTERNAL_SYSCALL_NCS(name, err, nr, args...) \
(__extension__ \
 ({ \
	register unsigned int resultvar; \
	__asm__ __volatile__ ( \
		LOADARGS_##nr                                   \
		"movl	%1, %%eax\n\t"                          \
		"int	$0x80\n\t"                              \
		RESTOREARGS_##nr                                \
		: "=a" (resultvar)                              \
		: "g" (name) ASMFMT_##nr(args) : "memory", "cc" \
	); \
	(int) resultvar; \
  }) \
)

#if 1 /* defined __PIC__ || defined __pic__ */

/* This code avoids pushing/popping ebx as much as possible.
 * I think the main reason was that older GCCs had problems
 * with proper saving/restoring of ebx if "b" constraint was used,
 * which was breaking -fPIC code really badly.
 * At least gcc 4.2.x seems to not need these tricks anymore,
 * but this code is still useful because it often avoids
 * using stack for saving ebx.
 * Keeping it unconditionally enabled for now.
 */

/* We need some help from the assembler to generate optimal code.
 * We define some macros here which later will be used.  */
/* gcc>=4.6 with LTO need the same guards as IMA (a.k.a --combine) did.
 * See gcc.gnu.org/PR47577  */
/* FIXME: drop these b* macros! */

__asm__ (
#if defined __DOMULTI__ || __GNUC_PREREQ (4, 6)
	/* Protect against asm macro redefinition (happens in __DOMULTI__ mode).
	 * Unfortunately, it ends up visible in .o files. */
	".ifndef _BITS_SYSCALLS_ASM\n\t"
	".set _BITS_SYSCALLS_ASM,1\n\t"
#endif
	".L__X'%ebx = 1\n\t"
	".L__X'%ecx = 2\n\t"
	".L__X'%edx = 2\n\t"
	".L__X'%eax = 3\n\t"
	".L__X'%esi = 3\n\t"
	".L__X'%edi = 3\n\t"
	".L__X'%ebp = 3\n\t"
	".L__X'%esp = 3\n\t"

	/* Loading param #1 (ebx) is done by loading it into
	 * another register, and then performing bpushl+bmovl,
	 * since we must preserve ebx */

	".macro bpushl name reg\n\t"
	".if 1 - \\name\n\t"    /* if reg!=ebx... */
	".if 2 - \\name\n\t"    /* if reg can't be clobbered... */
	"pushl %ebx\n\t"        /* save ebx on stack */
	".else\n\t"
	"xchgl \\reg, %ebx\n\t" /* else save ebx in reg, and load reg to ebx */
	".endif\n\t"
	".endif\n\t"
	".endm\n\t"

	".macro bmovl name reg\n\t"
	".if 1 - \\name\n\t"
	".if 2 - \\name\n\t"    /* if reg can't be clobbered... */
	"movl \\reg, %ebx\n\t"  /* load reg to ebx */
	".endif\n\t"
	".endif\n\t"
	".endm\n\t"

	".macro bpopl name reg\n\t"
	".if 1 - \\name\n\t"
	".if 2 - \\name\n\t"    /* if reg can't be clobbered... */
	"popl %ebx\n\t"         /* restore ebx from stack */
	".else\n\t"
	"xchgl \\reg, %ebx\n\t" /* else restore ebx from reg */
	".endif\n\t"
	".endif\n\t"
	".endm\n\t"

#if defined __DOMULTI__ || __GNUC_PREREQ (4, 6)
	".endif\n\t" /* _BITS_SYSCALLS_ASM */
#endif
);

#define LOADARGS_0
#define LOADARGS_1  "bpushl .L__X'%k2, %k2\n\t" "bmovl .L__X'%k2, %k2\n\t"
#define LOADARGS_2  LOADARGS_1
#define LOADARGS_3  LOADARGS_1
#define LOADARGS_4  LOADARGS_1
#define LOADARGS_5  LOADARGS_1
#define LOADARGS_6  LOADARGS_1 "push %%ebp\n\t" "movl %7, %%ebp\n\t"

#define RESTOREARGS_0
#define RESTOREARGS_1  "bpopl .L__X'%k2, %k2\n\t"
#define RESTOREARGS_2  RESTOREARGS_1
#define RESTOREARGS_3  RESTOREARGS_1
#define RESTOREARGS_4  RESTOREARGS_1
#define RESTOREARGS_5  RESTOREARGS_1
#define RESTOREARGS_6  "pop %%ebp\n\t" RESTOREARGS_1

#define ASMFMT_0()
/* "acdSD" constraint would work too, but "SD" would use esi/edi and cause
 * them to be pushed/popped by compiler, "a" would use eax and cause ebx
 * to be saved/restored on stack, not in register. Narrowing choice down
 * to "ecx or edx" results in smaller and faster code: */
#define ASMFMT_1(arg1) \
	, "cd" (arg1)
/* Can use "adSD" constraint here: */
#define ASMFMT_2(arg1, arg2) \
	, "d" (arg1), "c" (arg2)
/* Can use "aSD" constraint here: */
#define ASMFMT_3(arg1, arg2, arg3) \
	, "a" (arg1), "c" (arg2), "d" (arg3)
/* Can use "aD" constraint here: */
#define ASMFMT_4(arg1, arg2, arg3, arg4) \
	, "a" (arg1), "c" (arg2), "d" (arg3), "S" (arg4)
#define ASMFMT_5(arg1, arg2, arg3, arg4, arg5) \
	, "a" (arg1), "c" (arg2), "d" (arg3), "S" (arg4), "D" (arg5)
#define ASMFMT_6(arg1, arg2, arg3, arg4, arg5, arg6) \
	, "a" (arg1), "c" (arg2), "d" (arg3), "S" (arg4), "D" (arg5), "m" (arg6)

#else /* !PIC */

/* Simpler code which just uses "b" constraint to load ebx.
 * Seems to work with gc 4.2.x, and generates slightly smaller,
 * but slightly slower code. Example (time syscall):
 *
 * -	8b 4c 24 04            mov    0x4(%esp),%ecx
 * -	87 cb                  xchg   %ecx,%ebx
 * +	53                     push   %ebx
 * +	8b 5c 24 08            mov    0x8(%esp),%ebx
 *	b8 0d 00 00 00         mov    $0xd,%eax
 *	cd 80                  int    $0x80
 * -	87 cb                  xchg   %ecx,%ebx
 * +	5b                     pop    %ebx
 *	c3                     ret
 *
 * 2 bytes smaller, but uses stack via "push/pop ebx"
 */

#define LOADARGS_0
#define LOADARGS_1
#define LOADARGS_2
#define LOADARGS_3
#define LOADARGS_4
#define LOADARGS_5
#define LOADARGS_6  "push %%ebp\n\t" "movl %7, %%ebp\n\t"

#define RESTOREARGS_0
#define RESTOREARGS_1
#define RESTOREARGS_2
#define RESTOREARGS_3
#define RESTOREARGS_4
#define RESTOREARGS_5
#define RESTOREARGS_6  "pop %%ebp\n\t"

#define ASMFMT_0()
#define ASMFMT_1(arg1) \
	, "b" (arg1)
#define ASMFMT_2(arg1, arg2) \
	, "b" (arg1), "c" (arg2)
#define ASMFMT_3(arg1, arg2, arg3) \
	, "b" (arg1), "c" (arg2), "d" (arg3)
#define ASMFMT_4(arg1, arg2, arg3, arg4) \
	, "b" (arg1), "c" (arg2), "d" (arg3), "S" (arg4)
#define ASMFMT_5(arg1, arg2, arg3, arg4, arg5) \
	, "b" (arg1), "c" (arg2), "d" (arg3), "S" (arg4), "D" (arg5)
#define ASMFMT_6(arg1, arg2, arg3, arg4, arg5, arg6) \
	, "b" (arg1), "c" (arg2), "d" (arg3), "S" (arg4), "D" (arg5), "m" (arg6)

#endif /* !PIC */

#endif /* __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */
