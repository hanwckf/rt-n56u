/* Extended regular expression matching and search library.
   Copyright (C) 2002, 2003, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Isamu Hasegawa <isamu@yamato.ibm.com>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <features.h>

#ifdef __UCLIBC__
# define _REGEX_RE_COMP
# define HAVE_LANGINFO
# define HAVE_LANGINFO_CODESET
# include <stdbool.h>
# include <stdint.h>
# include <string.h>
# include <stdlib.h>
# ifdef __UCLIBC_HAS_WCHAR__
#  define RE_ENABLE_I18N
#  include <wchar.h>
#  include <wctype.h>
#  define __iswctype iswctype
#  define __wcrtomb wcrtomb
#  define __btowc btowc
#  define __wctype wctype
# endif
# include <ctype.h>
#endif

/* Make sure noone compiles this code with a C++ compiler.  */
#ifdef __cplusplus
# error "This is C code, use a C compiler"
#endif

/* On some systems, limits.h sets RE_DUP_MAX to a lower value than
   GNU regex allows.  Include it before <regex.h>, which correctly
   #undefs RE_DUP_MAX and sets it to the right value.  */
#include <limits.h>

#include <regex.h>

#include "regex_internal.h"
#include "regex_internal.c"
#include "regcomp.c"
#include "regexec.c"
