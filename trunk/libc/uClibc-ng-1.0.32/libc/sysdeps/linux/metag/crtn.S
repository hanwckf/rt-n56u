!    Copyright (C) 2013 Imagination Technologies Ltd.

!    Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.

	.section .init
	.global	__init
	.type	__init,function
	GETL    D0.4, D1RtP, [A0FrP+#8++]
	SUB     A0StP, A0FrP, #8
	MOV     A0FrP, D0.4
	MOV     PC, D1RtP

	.section .fini
	.global	__fini
	.type	__fini,function
	GETL    D0.4, D1RtP, [A0FrP+#8++]
	SUB     A0StP, A0FrP, #8
	MOV     A0FrP, D0.4
	MOV     PC, D1RtP
