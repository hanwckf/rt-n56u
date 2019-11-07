/*
 * Copyright (C) 2016-2017 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef _LINUX_NDS32_SYSDEP_H
#define _LINUX_NDS32_SYSDEP_H 1

#include <common/sysdep.h>

#ifdef	__ASSEMBLER__
/* Define an entry point visible from C.  */
# ifdef PIC
# define ENTRY(name)                      \
  .pic										\
  .align 2;                              \
  .globl C_SYMBOL_NAME(name);            \
  .func  C_SYMBOL_NAME(name);            \
  .type  C_SYMBOL_NAME(name), @function; \
C_SYMBOL_NAME(name):			\
	        cfi_startproc;

# else
# define ENTRY(name)                      \
  .align 2;                              \
  .globl C_SYMBOL_NAME(name);            \
  .func  C_SYMBOL_NAME(name);            \
  .type  C_SYMBOL_NAME(name), @function; \
C_SYMBOL_NAME(name):			\
	        cfi_startproc;
# endif

#undef END
#define END(name) \
  cfi_endproc;        \
  .endfunc;           \
  .size C_SYMBOL_NAME(name), .-C_SYMBOL_NAME(name)

/* If compiled for profiling, call `mcount' at the start of each function.  */
# ifdef HAVE_ELF
	#undef NO_UNDERSCORES
	#define NO_UNDERSCORES
# endif

# ifdef NO_UNDERSCORES
	#define syscall_error __syscall_error
# endif

#define SYS_ify(syscall_name)  (__NR_##syscall_name)

#define __do_syscall(syscall_name)		\
  syscall	SYS_ify(syscall_name);

# ifdef PIC
# define PSEUDO(name, syscall_name, args)				\
  .pic;									\
  .align 2;								\
  99:	mfusr $r15, $PC;						\
	sethi	$r1,	hi20(_GLOBAL_OFFSET_TABLE_ + 4);		\
	ori	$r1,	$r1,	lo12(_GLOBAL_OFFSET_TABLE_ + 8);	\
	add	$r1,	$r15,	$r1;					\
	sethi $r15, hi20(SYSCALL_ERROR@PLT);				\
	ori	$r15,	$r15, 	lo12(SYSCALL_ERROR@PLT);		\
	add	$r15,	$r15, 	$r1;					\
	jr	$r15;							\
	nop;                                   				\
	ENTRY(name);                          				\
	__do_syscall(syscall_name);            				\
	bgez $r0, 2f;							\
	sltsi	$r1, 	$r0, 	-4096;					\
	beqz	$r1, 	99b;						\
  2:
# else
# define PSEUDO(name, syscall_name, args)				\
  .align 2;								\
  99:	j SYSCALL_ERROR;                  				\
	nop;                                   				\
	ENTRY(name);                          				\
	__do_syscall(syscall_name);            				\
        bgez $r0, 2f;                           			\
        sltsi   $r1, $r0, -4096;                			\
        beqz    $r1, 99b;                       			\
  2:
# endif


#define PSEUDO_NOERRNO(name, syscall_name, args) 			\
  ENTRY(name);                                   			\
  __do_syscall(syscall_name);

#undef PSEUDO_END
#define PSEUDO_END(sym)							\
	SYSCALL_ERROR_HANDLER						\
	END(sym)

#undef PSEUDO_END_ERRVAL
#define PSEUDO_END_ERRVAL(sym) END(sym)

#define PSEUDO_ERRVAL(name, syscall_name, args) PSEUDO_NOERRNO(name, syscall_name, args)

#define ret_ERRVAL ret

#define ret_NOERRNO ret
#if defined NOT_IN_libc
	#define SYSCALL_ERROR __local_syscall_error
	#ifdef PIC
		#define SYSCALL_ERROR_HANDLER						\
			__local_syscall_error:						\
				pushm	$gp, 	$lp;					\
			        cfi_adjust_cfa_offset(8)				\
			        cfi_rel_offset(gp, 0)					\
			        cfi_rel_offset(lp, 4)					\
				mfusr 	$r15, 	$PC;					\
				sethi	$gp,	hi20(_GLOBAL_OFFSET_TABLE_+4);		\
				ori	$gp,	$gp,	lo12(_GLOBAL_OFFSET_TABLE_+8);	\
				add	$gp,	$gp,	$r15;				\
				neg	$r0, 	$r0;					\
				push	$r0;						\
			        cfi_adjust_cfa_offset(4)				\
			        cfi_rel_offset(r0, 0)					\
				addi    $sp,    $sp, 	-4; 			\
				bal	C_SYMBOL_NAME(__errno_location@PLT);		\
				addi    $sp,    $sp, 	4; 			\
				pop	$r1;						\
			        cfi_adjust_cfa_offset(-4);                      	\
			        cfi_restore(r1);                                	\
				swi	$r1,	[$r0];					\
				li	$r0,	-1;					\
				popm	$gp,	$lp;					\
			        cfi_adjust_cfa_offset(-8);                      	\
			        cfi_restore(lp);                                	\
			        cfi_restore(gp);  					\
			1:	ret;
	#else
		#define SYSCALL_ERROR_HANDLER						\
			__local_syscall_error:						\
				push	$lp;						\
			        cfi_adjust_cfa_offset(4)				\
			        cfi_rel_offset(lp, 0)					\
				neg	$r0,	$r0;					\
				push	$r0;						\
			        cfi_adjust_cfa_offset(4)				\
			        cfi_rel_offset(r0, 0)					\
				addi    $sp,    $sp, 	-4; 			\
				bal	C_SYMBOL_NAME(__errno_location);		\
				addi    $sp,    $sp, 	4; 			\
				pop	$r1;						\
			        cfi_adjust_cfa_offset(-4);                      	\
			        cfi_restore(r1);                                	\
				swi	$r1,	[$r0];					\
				li	$r0,	-1;					\
				pop	$lp;						\
			        cfi_adjust_cfa_offset(-4);                      	\
			        cfi_restore(lp);                                	\
				ret;
	#endif

#else
	#define SYSCALL_ERROR_HANDLER
	#define SYSCALL_ERROR __syscall_error
#endif

#endif	/* __ASSEMBLER__ */
#endif //_LINUX_NDS32_SYSDEP_H
