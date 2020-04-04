!    Copyright (C) 2013 Imagination Technologies Ltd.

!    Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.

	.section .init
	.global	__init
	.type	__init,function
__init:
        MOV     D0FrT, A0FrP
        ADD     A0FrP, A0StP, #0
        SETL    [A0StP++], D0.4, D1RtP

	.section .fini
	.global	__fini
	.type	__fini,function
__fini:
        MOV     D0FrT, A0FrP
        ADD     A0FrP, A0StP, #0
        SETL    [A0StP++], D0.4, D1RtP
