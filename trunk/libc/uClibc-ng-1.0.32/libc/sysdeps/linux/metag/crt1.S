!    Copyright (C) 2013 Imagination Technologies Ltd.

!    Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.


#include <asm/unistd.h>

	.text
	.global	__start
	.type	__start,function
__start:
	MOV	D0FrT,A0StP
	MOV	A0FrP,A0StP

	MOV	A0.2,#0
	MOV	A0.3,#0

	MOV	A1.1,#0
	MOV	A1.2,#0
	MOV	A1.3,#0

	MOV	D0.5,#0
	MOV	D0.6,#0
	MOV	D0.7,#0

	MOV	D1.5,#0
	MOV	D1.6,#0
	MOV	D1.7,#0

	MOV	D1Ar3,D0Ar2			! argv
	MOV	D0Ar2,D1Ar1			! argc
	MOV	D0Ar6,D0Ar4			! rtld_fini

#ifdef __PIC__
	ADDT	A1LbP,CPC1,#HI(__GLOBAL_OFFSET_TABLE__)
	ADD	A1LbP,A1LbP,#LO(__GLOBAL_OFFSET_TABLE__+4)

	MOV	D1Ar1,A1LbP
	ADDT	D1Ar1,D1Ar1,#HI(_main@GOTOFF)
	ADD	D1Ar1,D1Ar1,#LO(_main@GOTOFF)

	MOV	D0Ar4,A1LbP
	ADDT	D0Ar4,D0Ar4,#HI(__init@GOTOFF)
	ADD	D0Ar4,D0Ar4,#LO(__init@GOTOFF)

	MOV	D1Ar5,A1LbP
	ADDT	D1Ar5,D1Ar5,#HI(__fini@GOTOFF)
	ADD	D1Ar5,D1Ar5,#LO(__fini@GOTOFF)
#else
	MOVT	D1Ar1,#HI(_main)
	ADD	D1Ar1,D1Ar1,#LO(_main)
	MOVT	D0Ar4,#HI(__init)
	ADD	D0Ar4,D0Ar4,#LO(__init)
	MOVT	D1Ar5,#HI(__fini)
	ADD	D1Ar5,D1Ar5,#LO(__fini)
#endif

	MOVT	D1Re0,#0x80

	SETL	[A0StP++],D0Re0,D1Re0		! stack_end (8Mb)

	MOV	D1Re0,#0
	MOV	D0Re0,#0

#ifdef __PIC__
	CALLR   D1RtP, ___uClibc_main@PLT
#else
	CALLR   D1RtP, ___uClibc_main
#endif

	MOV	D1Re0,#__NR_exit
	MOV	D1Ar1,#0x1
	SWITCH	#0x440001			! exit syscall

	.size __start,.-__start
