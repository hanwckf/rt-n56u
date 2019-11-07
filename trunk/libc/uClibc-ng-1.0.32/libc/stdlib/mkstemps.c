/* Copyright (C) 1998-2012 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "../misc/internals/tempname.h"

/* Generate a unique temporary file name from TEMPLATE.
   The TEMPLATE is of the form "XXXXXXsuffix" where six characters
   after the TEMPLATE must be "XXXXXX" followed by the suffix.
   The suffix length must be specified with suffixlen.
   "XXXXXX" are replaced with a string that makes the filename unique.
   Then open the file and return a fd. */
int mkstemps (char *template, int suffixlen)
{
    return __gen_tempname (template, __GT_FILE, 0, suffixlen,
                           S_IRUSR | S_IWUSR);
}
