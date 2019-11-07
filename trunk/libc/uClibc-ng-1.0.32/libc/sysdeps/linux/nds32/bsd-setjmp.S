/*
 * Copyright (C) 2016 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* This just does a tail-call to `__sigsetjmp (ARG, 1)'.
   We cannot do it in C because it must be a tail-call, so frame-unwinding
   in setjmp doesn't clobber the state restored by longjmp.  */

#include <sysdep.h>

ENTRY(setjmp)
	move	$r1, #1

/* Make a tail call to __sigsetjmp.  */
#ifdef PIC
	/* Initialize $r2 as $gp value.  */
	sethi   $r2, hi20(_GLOBAL_OFFSET_TABLE_-8)
	ori     $r2, $r2, lo12(_GLOBAL_OFFSET_TABLE_-4)
	mfusr   $r15, $pc
	add     $r2, $r15, $r2

	! la    $r3, __sigsetjmp@PLT
	sethi   $r3, hi20(HIDDEN_JUMPTARGET(__sigsetjmp)@PLT)
	ori     $r3, $r3, lo12(HIDDEN_JUMPTARGET(__sigsetjmp)@PLT)
	add     $r3, $r3, $r2

	jr      $r3
#else /* NOT PIC */
	la	$r15, C_SYMBOL_NAME(__sigsetjmp)
	jr	$r15
#endif

END (setjmp)
