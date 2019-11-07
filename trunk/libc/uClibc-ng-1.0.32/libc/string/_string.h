/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef __STRING_H
#define __STRING_H

#include <features.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#ifdef WANT_WIDE
# include <wchar.h>
# include <wctype.h>
# include <bits/uClibc_uwchar.h>
# define Wvoid			wchar_t
# define Wchar			wchar_t
# define Wuchar			__uwchar_t
# define Wint			wchar_t
#else
# define Wvoid			void
# define Wchar			char
typedef unsigned char		__string_uchar_t;
# define Wuchar			__string_uchar_t
# define Wint			int
#endif

#endif /* __STRING_H */
