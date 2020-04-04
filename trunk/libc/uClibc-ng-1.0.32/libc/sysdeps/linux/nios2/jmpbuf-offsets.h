/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>

#define JB_R16 0
#define JB_R17 1
#define JB_R18 2
#define JB_R19 3
#define JB_R20 4
#define JB_R21 5
#define JB_R22 6
#define JB_FP  7
#define JB_RA  8
#define JB_SP  9

#ifdef __UCLIBC_HAS_FPU__
# define JB_SIZE 304
#else
# define JB_SIZE 48
#endif
