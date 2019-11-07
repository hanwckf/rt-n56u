/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

___gnu_compiled_c:

	.section .init
	.align 4
	.global	_init
	.type	 _init,@function
_init:
	st.a blink,[sp,-4]
	st.a fp,[sp,-4]
	mov fp,sp


	.section .fini
	.align 4
	.global	_fini
	.type	 _fini,@function
_fini:
	st.a blink,[sp,-4]
	st.a fp,[sp,-4]
	mov fp,sp
	.align 4
