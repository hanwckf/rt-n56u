/*
 * libc/sysdeps/linux/bfin/sysdep.h
 *
 * Copyright (C) 2007 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef __BFIN_SYSDEP_H__
#define __BFIN_SYSDEP_H__

#include <common/sysdep.h>

#ifdef __ASSEMBLER__

#define ENTRY(sym) .global sym; .type sym, STT_FUNC; sym:
#define ENDPROC(sym) .size sym, . - sym

#endif

#endif
