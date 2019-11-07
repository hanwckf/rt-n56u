/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

.file  "initfini.c"

.section .init
	ldw	lr, (sp, 4)
	addi	sp, 8
	rts

.section .fini
	ldw	lr, (sp, 4)
	addi	sp, 8
	rts
