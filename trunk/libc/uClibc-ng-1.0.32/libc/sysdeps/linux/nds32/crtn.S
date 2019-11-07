/*
 * Copyright (C) 2016 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

	.pic
#APP
	.section .init
	! epilogue
	addi 	$sp,$fp,#-4
	lmw.bim $sp,[$sp],$sp,#0x6
	lwi.bi  $fp,[$sp],#4
	ret
.LFE28:
	.section .fini
#NO_APP
	! epilogue
	addi 	$sp,$fp,#-4
	lmw.bim $sp,[$sp],$sp,#0x6
	lwi.bi  $fp,[$sp],#4
	ret
.LFE29:
#APP
	.ident	"GCC: (GNU) 3.4.4"
