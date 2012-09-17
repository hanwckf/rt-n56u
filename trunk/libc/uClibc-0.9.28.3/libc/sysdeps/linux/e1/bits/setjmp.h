/* This file is lisenced under LGPL.
 * Copyright (C) 2002-2003,    George Thanos <george.thanos@gdt.gr>
 *                             Yannis Mitsos <yannis.mitsos@gdt.gr>
 */

#ifndef _SETJMP_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

typedef struct {
	unsigned long G3;
	unsigned long G4;
	unsigned long SavedSP;
	unsigned long SavedPC;
	unsigned long SavedSR;
	unsigned long ReturnValue;
} __jmp_buf[1];

