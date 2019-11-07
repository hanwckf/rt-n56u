/*
 * Copyright (C) 2000,2001,2003,2004	Manuel Novoa III <mjn3@codepoet.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#ifndef _FPMAXTOSTR_H
#define _FPMAXTOSTR_H 1

#include <features.h>
#define __need_size_t
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <printf.h>
#include <sys/types.h>

#ifdef __UCLIBC_HAS_FLOATS__
# include <float.h>
# include <bits/uClibc_fpmax.h>

/* WARNING: Adjust _fp_out_wide() in _vfprintf.c if this changes! */
/* With 32 bit ints, we can get 9 decimal digits per block. */
# define DIGITS_PER_BLOCK     9

# define NUM_DIGIT_BLOCKS   ((DECIMAL_DIG+DIGITS_PER_BLOCK-1)/DIGITS_PER_BLOCK)

/* WARNING: Adjust _fp_out_wide() in _vfprintf.c if this changes! */
/* extra space for '-', '.', 'e+###', and nul */
# define BUF_SIZE  ( 3 + NUM_DIGIT_BLOCKS * DIGITS_PER_BLOCK )

/* psm: why do these internals differ? */
#  ifdef __USE_OLD_VFPRINTF__
typedef void (__fp_outfunc_t)(FILE *fp, intptr_t type, intptr_t len, intptr_t buf);

extern size_t _fpmaxtostr(FILE * fp, __fpmax_t x, struct printf_info *info,
			  __fp_outfunc_t fp_outfunc) attribute_hidden;
#  else
typedef size_t (__fp_outfunc_t)(FILE *fp, intptr_t type, intptr_t len, intptr_t buf);

extern ssize_t _fpmaxtostr(FILE * fp, __fpmax_t x, struct printf_info *info,
			   __fp_outfunc_t fp_outfunc) attribute_hidden;
#  endif

# endif /* __UCLIBC_HAS_FLOATS__ */
#endif /* _FPMAXTOSTR_H */
