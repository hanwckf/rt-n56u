/* Return the offset of one string within another.
   Copyright (C) 1994,1996,1997,2000,2001,2003 Free Software Foundation, Inc.
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
 * My personal strstr() implementation that beats most other algorithms.
 * Until someone tells me otherwise, I assume that this is the
 * fastest implementation of strstr() in C.
 * I deliberately chose not to comment it.  You should have at least
 * as much fun trying to understand it, as I had to write it :-).
 *
 * Stephen R. van den Berg, berg@pool.informatik.rwth-aachen.de	*/

#include <string.h>


typedef unsigned chartype;

char *strstr (const char *phaystack, const char *pneedle)
{
  const unsigned char *haystack, *needle;
  chartype b;
  const unsigned char *rneedle;

  haystack = (const unsigned char *) phaystack;

  if ((b = *(needle = (const unsigned char *) pneedle)))
    {
      chartype c;
      haystack--;		/* possible ANSI violation */

      {
	chartype a;
	do
	  if (!(a = *++haystack))
	    goto ret0;
	while (a != b);
      }

      if (!(c = *++needle))
	goto foundneedle;
      ++needle;
      goto jin;

      for (;;)
	{
	  {
	    chartype a;
	    if (0)
	    jin:{
		if ((a = *++haystack) == c)
		  goto crest;
	      }
	    else
	      a = *++haystack;
	    do
	      {
		for (; a != b; a = *++haystack)
		  {
		    if (!a)
		      goto ret0;
		    if ((a = *++haystack) == b)
		      break;
		    if (!a)
		      goto ret0;
		  }
	      }
	    while ((a = *++haystack) != c);
	  }
	crest:
	  {
	    chartype a;
	    {
	      const unsigned char *rhaystack;
	      if (*(rhaystack = haystack-- + 1) == (a = *(rneedle = needle)))
		do
		  {
		    if (!a)
		      goto foundneedle;
		    if (*++rhaystack != (a = *++needle))
		      break;
		    if (!a)
		      goto foundneedle;
		  }
		while (*++rhaystack == (a = *++needle));
	      needle = rneedle;	/* took the register-poor aproach */
	    }
	    if (!a)
	      break;
	  }
	}
    }
foundneedle:
  return (char *) haystack;
ret0:
  return 0;
}
libc_hidden_def(strstr)
