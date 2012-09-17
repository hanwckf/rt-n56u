/* Test collation function via transformation using real data.
   Copyright (C) 1997, 1998, 2000, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include <ctype.h>
#include <error.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct lines
{
  char *xfrm;
  char *line;
};

static int xstrcmp (const void *, const void *);

int
main (int argc, char *argv[])
{
  int result = 0;
  size_t nstrings, nstrings_max;
  struct lines *strings;
  char *line = NULL;
  size_t len = 0;
  size_t n;

  if (argc < 2)
    error (1, 0, "usage: %s <random seed>", argv[0]);

  setlocale (LC_ALL, "");

  nstrings_max = 100;
  nstrings = 0;
  strings = (struct lines *) malloc (nstrings_max * sizeof (struct lines));
  if (strings == NULL)
    {
      perror (argv[0]);
      exit (1);
    }

  while (1)
    {
      char saved, *newp;
      int needed;
      int l;
      if (getline (&line, &len, stdin) < 0)
	break;

      if (nstrings == nstrings_max)
	{
	  strings = (struct lines *) realloc (strings,
					      (nstrings_max *= 2)
					       * sizeof (*strings));
	  if (strings == NULL)
	    {
	      perror (argv[0]);
	      exit (1);
	    }
	}
      strings[nstrings].line = strdup (line);
      l = strcspn (line, ":(;");
      while (l > 0 && isspace (line[l - 1]))
	--l;

      saved = line[l];
      line[l] = '\0';
      needed = strxfrm (NULL, line, 0);
      newp = malloc (needed + 1);
      strxfrm (newp, line, needed + 1);
      strings[nstrings].xfrm = newp;
      line[l] = saved;
      ++nstrings;
    }
  free (line);

  /* First shuffle.  */
  srandom (atoi (argv[1]));
  for (n = 0; n < 10 * nstrings; ++n)
    {
      int r1, r2, r;
      size_t idx1 = random () % nstrings;
      size_t idx2 = random () % nstrings;
      struct lines tmp = strings[idx1];
      strings[idx1] = strings[idx2];
      strings[idx2] = tmp;

      /* While we are at it a first little test.  */
      r1 = strcmp (strings[idx1].xfrm, strings[idx2].xfrm);
      r2 = strcmp (strings[idx2].xfrm, strings[idx1].xfrm);
      r = -(r1 ^ r2);
      if (r)
	r /= abs (r1 ^ r2);

      if (r < 0 || (r == 0 && (r1 != 0 || r2 != 0))
	  || (r > 0 && (r1 ^ r2) >= 0))
	printf ("collate wrong: %d vs. %d\n", r1, r2);
    }

  /* Now sort.  */
  qsort (strings, nstrings, sizeof (struct lines), xstrcmp);

  /* Print the result.  */
  for (n = 0; n < nstrings; ++n)
    {
      fputs (strings[n].line, stdout);
      free (strings[n].line);
      free (strings[n].xfrm);
    }
  free (strings);

  return result;
}


static int
xstrcmp (ptr1, ptr2)
     const void *ptr1;
     const void *ptr2;
{
  const struct lines *l1 = (const struct lines *) ptr1;
  const struct lines *l2 = (const struct lines *) ptr2;

  return strcmp (l1->xfrm, l2->xfrm);
}
