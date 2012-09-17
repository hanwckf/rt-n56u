/* Assembly macros for 64-bit PowerPC.
   Copyright (C) 2002, 2003, 2004, 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA
   02110-1301 USA.  */

#ifdef __ELF__

#ifdef __ASSEMBLER__

/* Support macros for CALL_MCOUNT.  */
	.macro SAVE_ARG NARG
	.if \NARG
	SAVE_ARG \NARG-1
	std	2+\NARG,-72+8*(\NARG)(1)
	.endif
	.endm

	.macro REST_ARG NARG
	.if \NARG
	REST_ARG \NARG-1
	ld	2+\NARG,40+8*(\NARG)(1)
	.endif
	.endm

/* If compiled for profiling, call `_mcount' at the start of each function.
   see ppc-mcount.S for more details.  */
	.macro CALL_MCOUNT NARG
#ifdef	PROF
	mflr	r0
	SAVE_ARG \NARG
	std	r0,16(r1)
	stdu	r1,-112(r1)
	bl	JUMPTARGET (_mcount)
	ld	r0,128(r1)
	REST_ARG \NARG
	addi	r1,r1,112
	mtlr	r0
#endif
	.endm

#ifdef USE_PPC64_OVERLAPPING_OPD
# define OPD_ENT(name)	.quad BODY_LABEL (name), .TOC.@tocbase
#else
# define OPD_ENT(name)	.quad BODY_LABEL (name), .TOC.@tocbase, 0
#endif

#define ENTRY_1(name)	\
	.section	".text";		\
	.type BODY_LABEL(name),@function;	\
	.globl name;				\
	.section ".opd","aw";			\
	.align 3;				\
name##: OPD_ENT (name);				\
	.previous;

#ifdef HAVE_ASM_GLOBAL_DOT_NAME
# define DOT_LABEL(X) .##X
# define BODY_LABEL(X) .##X
# define ENTRY_2(name)	\
	.globl BODY_LABEL(name);		\
	ENTRY_1(name)				\
	.size name, 24;
# define END_2(name)	\
	.size BODY_LABEL(name),.-BODY_LABEL(name);
#else
# define DOT_LABEL(X) X
# define BODY_LABEL(X) .LY##X
# define ENTRY_2(name)	\
	.type name,@function;			\
	ENTRY_1(name)
# define END_2(name)	\
	.size name,.-BODY_LABEL(name);		\
	.size BODY_LABEL(name),.-BODY_LABEL(name);
#endif

#define ENTRY(name)	\
	ENTRY_2(name)				\
	.align ALIGNARG(2);			\
BODY_LABEL(name):				\
	cfi_startproc;

#define EALIGN_W_0  /* No words to insert.  */
#define EALIGN_W_1  nop
#define EALIGN_W_2  nop;nop
#define EALIGN_W_3  nop;nop;nop
#define EALIGN_W_4  EALIGN_W_3;nop
#define EALIGN_W_5  EALIGN_W_4;nop
#define EALIGN_W_6  EALIGN_W_5;nop
#define EALIGN_W_7  EALIGN_W_6;nop

/* EALIGN is like ENTRY, but does alignment to 'words'*4 bytes
   past a 2^alignt boundary.  */
#define EALIGN(name, alignt, words) \
	ENTRY_2(name)				\
	.align ALIGNARG(alignt);		\
	EALIGN_W_##words;			\
BODY_LABEL(name):				\
	cfi_startproc;

/* Local labels stripped out by the linker.  */
#undef L
#define L(x) .L##x

#define tostring(s) #s
#define stringify(s) tostring(s)
#define XGLUE(a,b) a##b
#define GLUE(a,b) XGLUE(a,b)
#define LT_LABEL(name) GLUE(.LT,name)
#define LT_LABELSUFFIX(name,suffix) GLUE(GLUE(.LT,name),suffix)

/* Support Traceback tables */
#define TB_ASM			0x000c000000000000
#define TB_GLOBALLINK		0x0000800000000000
#define TB_IS_EPROL		0x0000400000000000
#define TB_HAS_TBOFF		0x0000200000000000
#define TB_INT_PROC		0x0000100000000000
#define TB_HAS_CTL		0x0000080000000000
#define TB_TOCLESS		0x0000040000000000
#define TB_FP_PRESENT		0x0000020000000000
#define TB_LOG_ABORT		0x0000010000000000
#define TB_INT_HANDL		0x0000008000000000
#define TB_NAME_PRESENT		0x0000004000000000
#define TB_USES_ALLOCA		0x0000002000000000
#define TB_SAVES_CR		0x0000000200000000
#define TB_SAVES_LR		0x0000000100000000
#define TB_STORES_BC		0x0000000080000000
#define TB_FIXUP		0x0000000040000000
#define TB_FP_SAVED(fprs)	(((fprs) & 0x3f) << 24)
#define TB_GPR_SAVED(gprs)	(((fprs) & 0x3f) << 16)
#define TB_FIXEDPARMS(parms)	(((parms) & 0xff) << 8)
#define TB_FLOATPARMS(parms)	(((parms) & 0x7f) << 1)
#define TB_PARMSONSTK		0x0000000000000001

#define PPC_HIGHER(v) 		(((v) >> 32) & 0xffff)
#define TB_DEFAULT		TB_ASM | TB_HAS_TBOFF | TB_NAME_PRESENT

#define TRACEBACK(name) \
LT_LABEL(name): ; \
	.long	0 ; \
	.quad	TB_DEFAULT ; \
	.long	LT_LABEL(name)-BODY_LABEL(name) ; \
	.short	LT_LABELSUFFIX(name,_name_end)-LT_LABELSUFFIX(name,_name_start) ; \
LT_LABELSUFFIX(name,_name_start): ;\
	.ascii	stringify(name) ; \
LT_LABELSUFFIX(name,_name_end): ; \
	.align	2 ;

#define TRACEBACK_MASK(name,mask) \
LT_LABEL(name): ; \
	.long	0 ; \
	.quad	TB_DEFAULT | mask ; \
	.long	LT_LABEL(name)-BODY_LABEL(name) ; \
	.short	LT_LABELSUFFIX(name,_name_end)-LT_LABELSUFFIX(name,_name_start) ; \
LT_LABELSUFFIX(name,_name_start): ;\
	.ascii	stringify(name) ; \
LT_LABELSUFFIX(name,_name_end): ; \
	.align	2 ;

/* END generates Traceback tables */
#undef	END
#define END(name) \
  cfi_endproc;			\
  TRACEBACK(name)		\
  END_2(name)

/* This form supports more informative traceback tables */
#define END_GEN_TB(name,mask)	\
  cfi_endproc;			\
  TRACEBACK_MASK(name,mask)	\
  END_2(name)

#define DO_CALL(syscall) \
    li 0,syscall; \
    sc

/* ppc64 is always PIC */
#undef JUMPTARGET
#define JUMPTARGET(name) DOT_LABEL(name)

#define PSEUDO(name, syscall_name, args) \
  .section ".text";	\
  ENTRY (name) \
  DO_CALL (SYS_ify (syscall_name));

#define PSEUDO_RET \
    bnslr+; \
    b JUMPTARGET(__syscall_error)

#define ret PSEUDO_RET

#undef	PSEUDO_END
#define	PSEUDO_END(name) \
  END (name)

#define PSEUDO_NOERRNO(name, syscall_name, args) \
  .section ".text";	\
  ENTRY (name) \
  DO_CALL (SYS_ify (syscall_name));

#define PSEUDO_RET_NOERRNO \
    blr

#define ret_NOERRNO PSEUDO_RET_NOERRNO

#undef	PSEUDO_END_NOERRNO
#define	PSEUDO_END_NOERRNO(name) \
  END (name)

#define PSEUDO_ERRVAL(name, syscall_name, args) \
  .section ".text";	\
  ENTRY (name) \
  DO_CALL (SYS_ify (syscall_name));

#define PSEUDO_RET_ERRVAL \
    blr

#define ret_ERRVAL PSEUDO_RET_ERRVAL

#undef	PSEUDO_END_ERRVAL
#define	PSEUDO_END_ERRVAL(name) \
  END (name)

#else /* !__ASSEMBLER__ */

#ifdef USE_PPC64_OVERLAPPING_OPD
# define OPD_ENT(name)	".quad " BODY_PREFIX #name ", .TOC.@tocbase;"
#else
# define OPD_ENT(name)	".quad " BODY_PREFIX #name ", .TOC.@tocbase, 0;"
#endif

#ifdef HAVE_ASM_GLOBAL_DOT_NAME
# define DOT_PREFIX "."
# define BODY_PREFIX "."
# define ENTRY_2(name)	\
	".globl " BODY_PREFIX #name ";\n"				\
	".size  " #name ", 24;"
# define END_2(name)	\
	".size " BODY_PREFIX #name ",.-" BODY_PREFIX #name ";"
#else
# define DOT_PREFIX ""
# define BODY_PREFIX ".LY"
# define ENTRY_2(name) ".type " #name ",@function;"
# define END_2(name)	\
	".size " #name ",.-" BODY_PREFIX #name ";\n"			\
	".size " BODY_PREFIX #name ",.-" BODY_PREFIX #name ";"
#endif

#endif	/* __ASSEMBLER__ */

#endif /* __ELF__ */
