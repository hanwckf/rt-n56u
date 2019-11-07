/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#ifndef _LINUX_CSKY_SYSDEP_H
#define _LINUX_CSKY_SYSDEP_H 1

#include <common/sysdep.h>
#include <sys/syscall.h>

#undef	SYS_ify
#define	SYS_ify(name) (__NR_##name)

#ifdef __ASSEMBLER__

/* ELF uses byte-counts for .align, most others use log2 of count of bytes.  */
#define ALIGNARG(log2) log2
/* For ELF we need the `.type' directive to make shared libs work right.  */
#define ASM_TYPE_DIRECTIVE(name,typearg) .type name,%##typearg;
#define ASM_SIZE_DIRECTIVE(name) .size name,.-name

#define	ENTRY(name) \
  .globl C_SYMBOL_NAME(name); \
  ASM_TYPE_DIRECTIVE (C_SYMBOL_NAME(name),function) \
  .align ALIGNARG(4); \
  C_LABEL(name)

#undef END
#define END(name) ASM_SIZE_DIRECTIVE(name)

#undef	ret_ERRVAL
#define	ret_ERRVAL rts

#undef	PSEUDO_END
#define	PSEUDO_END(name) END(name)

#ifdef __PIC__
#define __GET_GB \
  bsr 1f; \
  1: lrw gb, 1b@GOTPC; \
  addu gb, lr;

/*
 * __JSR must be used with __GET_GB and SAVE_ARGS
 */
#define __JSR(symbol) \
  lrw a2, symbol@PLT; \
  add a2, gb; \
  ld.w a2, (a2); \
  jsr a2;

#define PSEUDO_ERRJMP \
  subi sp, 8; \
  st.w lr, (sp); \
  st.w gb, (sp, 4); \
  __GET_GB \
  lrw a2, __syscall_error@PLT; \
  addu a2, gb; \
  ld.w a2, (a2); \
  jsr a2; \
  ld.w lr, (sp); \
  ld.w gb, (sp, 4); \
  addi sp, 8; \
  rts;

#else /* __PIC__ */

#define __GET_GB
#define __JSR(symbol) jsri symbol;
#define PSEUDO_ERRJMP \
  subi	sp, 4; \
  stw	lr, (sp, 0); \
  jsri	__syscall_error; \
  ldw	lr, (sp, 0); \
  addi	sp, 4; \
  rts;

#endif /* __PIC__ */

#define PSEUDO_ERRVAL(name, syscall_name, args) \
  .text; \
  99: PSEUDO_ERRJMP; \
  ENTRY(name); \
  DO_CALL(syscall_name, args); \
  btsti	a0, 31; \
  bt 99b;

#undef PSEUDO_END_ERRVAL
#define PSEUDO_END_ERRVAL(name) \
  rts; \
  END(name)

/* DO_CALL */
#undef	DO_CALL
#ifdef	__CSKYABIV2__

#define	DO_CALL(syscall_name, args) \
  DOARGS_##args \
  mov	t0, r7; \
  lrw	r7, SYS_ify (syscall_name); \
  trap	0; \
  mov	r7, t0; \
  UNDOARGS_##args

#define DOARGS_0
#define DOARGS_1
#define DOARGS_2
#define DOARGS_3
#define DOARGS_4
#define DOARGS_5 subi sp, 4; st.w r4, (sp, 0); ld.w r4, (sp, 4);
#define DOARGS_6 subi sp, 8; stm r4-r5, (sp); ld.w r4, (sp, 8); ld.w r5, (sp, 12);

#define UNDOARGS_0
#define UNDOARGS_1
#define UNDOARGS_2
#define UNDOARGS_3
#define UNDOARGS_4
#define UNDOARGS_5 ld.w r4, (sp, 0); addi sp, 4;
#define UNDOARGS_6 ldm r4-r5, (sp); addi sp, 8;

#else /* __CSKYABIV2__ */

#define DO_CALL(syscall_name, args) \
  lrw  r1, SYS_ify (syscall_name); \
  trap 0;

#define DOARGS_0
#define DOARGS_1
#define DOARGS_2
#define DOARGS_3
#define DOARGS_4
#define DOARGS_5
#define DOARGS_6

#define UNDOARGS_0
#define UNDOARGS_1
#define UNDOARGS_2
#define UNDOARGS_3
#define UNDOARGS_4
#define UNDOARGS_5
#define UNDOARGS_6

#endif /* __CSKYABIV2__ */

/*
 * define DO_CALL_2, only ABIV2 need DO_CALL_2
 * to be quite different with DO_CALL, DO_CALL_2 need not save r7.
 */
#ifdef __CSKYABIV2__
#undef  DO_CALL_2
#define DO_CALL_2(syscall_name, args) \
  DOARGS2_##args; \
  lrw	r7, SYS_ify(syscall_name); \
  trap	0; \
  UNDOARGS2_##args
#undef  DOARGS2_0
#define DOARGS2_0

#undef  DOARGS2_1
#define DOARGS2_1 DOARGS2_0
#undef  DOARGS2_2
#define DOARGS2_2 DOARGS2_0
#undef  DOARGS2_3
#define DOARGS2_3 DOARGS2_0
#undef  DOARGS2_4
#define DOARGS2_4 DOARGS2_0
#undef  DOARGS2_5
#define DOARGS2_5 \
  subi sp, 8; \
  cfi_adjust_cfa_offset (8); \
  stw  r4, (sp, 0); \
  ldw  r4, (sp, 24)
#undef  DOARGS2_6
#define DOARGS2_6 \
  subi sp, 8; \
  cfi_adjust_cfa_offset (8); \
  stw  r4, (sp, 0); \
  stw  r5, (sp, 4); \
  ldw  r4, (sp, 24); \
  ldw  r5, (sp, 28)

#undef  UNDOARGS2_0
#define UNDOARGS2_0

#undef  UNDOARGS2_1
#define UNDOARGS2_1 UNDOARGS2_0
#undef  UNDOARGS2_2
#define UNDOARGS2_2 UNDOARGS2_0
#undef  UNDOARGS2_3
#define UNDOARGS2_3 UNDOARGS2_0
#undef  UNDOARGS2_4
#define UNDOARGS2_4 UNDOARGS2_0
#undef  UNDOARGS2_5
#define UNDOARGS2_5 \
  ldw  r4, (sp, 0); \
  addi sp, 8

#undef  UNDOARGS2_6
#define UNDOARGS2_6 \
  ldw  r4, (sp, 0); \
  ldw  r5, (sp, 4); \
  addi sp, 8

#endif  /* __CSKYABIV2__ */

#endif /* __ASSEMBLER__ */
#endif /* _LINUX_CSKY_SYSDEP_H */

