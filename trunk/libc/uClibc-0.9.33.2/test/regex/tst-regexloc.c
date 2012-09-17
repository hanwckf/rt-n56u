/* Copyright (C) 2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sys/types.h>
#include <regex.h>
#include <locale.h>
#include <stdio.h>

int
main (int argc, char *argv[])
{
/* If uclibc has extended locale, or if it's a host build
 * (assuming host libc always has locale): */
#if defined __UCLIBC_HAS_XLOCALE__ || !defined __UCLIBC__
  regex_t re;
  regmatch_t mat[1];
  int exitcode = 1;

  if (setlocale (LC_ALL, "de_DE.ISO-8859-1") == NULL)
    puts ("cannot set locale");
  else if (regcomp (&re, "[a-f]*", 0) != REG_NOERROR)
    puts ("cannot compile expression \"[a-f]*\"");
  else if (regexec (&re, "abcdefCDEF", 1, mat, 0) == REG_NOMATCH)
    puts ("no match");
  else
    {
      exitcode = mat[0].rm_so != 0 || mat[0].rm_eo != 6;
      printf ("match from %d to %d - %s\n",
		mat[0].rm_so, mat[0].rm_eo,
		exitcode ? "WRONG!" : "ok"
      );
    }

  return exitcode;
#else
  puts("Test requires locale; skipping");
  return 0;
#endif
}
