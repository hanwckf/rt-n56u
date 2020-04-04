/* Copyright (C) 2011-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/*
 *      ISO C11 Standard: 7.28
 *	Unicode utilities	<uchar.h>
 */

#ifndef _UCHAR_H
#define _UCHAR_H	1

#include <features.h>

#define __need_size_t
#include <stddef.h>
#define __need_mbstate_t
#include <wchar.h>

#ifndef __mbstate_t_defined
__BEGIN_NAMESPACE_C99
/* Public type.  */
typedef __mbstate_t mbstate_t;
__END_NAMESPACE_C99
# define __mbstate_t_defined 1
#endif


#if defined __GNUC__ && !defined __USE_ISOCXX11
/* Define the 16-bit and 32-bit character types.  Use the information
   provided by the compiler.  */
# if !defined __CHAR16_TYPE__ || !defined __CHAR32_TYPE__
#  if defined __STDC_VERSION__ && __STDC_VERSION__ < 201000L
#   error "<uchar.h> requires ISO C11 mode"
#  else
#   error "definitions of __CHAR16_TYPE__ and/or __CHAR32_TYPE__ missing"
#  endif
# endif
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif

#endif	/* uchar.h */
