/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#include <sysdep.h>
.file  "initfini.c"

.section .init
ENTRY(_init)
	subi	sp, 8
	stw	lr, (sp, 4)

.section .fini
ENTRY(_fini)
	subi	sp, 8
	stw	lr, (sp, 4)

