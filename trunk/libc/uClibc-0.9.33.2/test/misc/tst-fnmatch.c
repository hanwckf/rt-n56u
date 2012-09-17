/* Tests for fnmatch function.
   Copyright (C) 2000, 2001 Free Software Foundation, Inc.
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

#include <errno.h>
#include <error.h>
#include <fnmatch.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>


static char *next_input (char **line, int first, int last);
static int convert_flags (const char *str);
static char *flag_output (int flags);
static char *escape (const char *str, size_t *reslenp, char **resbuf);


int str_isalpha(const char *str)
{
	size_t i = strlen(str);
	while (i--)
		if (isascii(str[i]) == 0)
			return 0;
	return 1;
}
int str_has_funk(const char *str, const char x)
{
	size_t i, max = strlen(str);
	for (i=0; i+1<max; ++i)
		if (str[i] == '[' && str[i+1] == x)
			return 1;
	return 0;
}


int
main (void)
{
  char *linebuf = NULL;
  size_t linebuflen = 0;
  int ntests = 0;
  int nfailed = 0;
  int nskipped = 0;
  char *escinput = NULL;
  size_t escinputlen = 0;
  char *escpattern = NULL;
  size_t escpatternlen = 0;
  int nr = 0;

  /* Read lines from stdin with the following format:

       locale  input-string  match-string  flags  result

     where `result' is either 0 or 1.  If the first character of a
     string is '"' we read until the next '"' and handled escaped '"'.  */
  while (! feof (stdin))
    {
      ssize_t n = getline (&linebuf, &linebuflen, stdin);
      char *cp;
      const char *locale;
      const char *input;
      const char *pattern;
      const char *result_str;
      int result;
      const char *flags;
      int flags_val;
      int fnmres;
      char numbuf[24];

      if (n == -1)
	break;

      if (n == 0)
	/* Maybe an empty line.  */
	continue;

      /* Skip over all leading white spaces.  */
      cp = linebuf;

      locale = next_input (&cp, 1, 0);
      if (locale == NULL)
	continue;

      input = next_input (&cp, 0, 0);
      if (input == NULL)
	continue;

      pattern = next_input (&cp, 0, 0);
      if (pattern == NULL)
	continue;

      result_str = next_input (&cp, 0, 0);
      if (result_str == NULL)
	continue;

      if (strcmp (result_str, "0") == 0)
	result = 0;
      else if  (strcasecmp (result_str, "NOMATCH") == 0)
	result = FNM_NOMATCH;
      else
	{
	  char *endp;
	  result = strtol (result_str, &endp, 0);
	  if (*endp != '\0')
	    continue;
	}

      flags = next_input (&cp, 0, 1);
      if (flags == NULL)
	/* We allow the flags missing.  */
	flags = "";

      /* Convert the text describing the flags in a numeric value.  */
      flags_val = convert_flags (flags);
      if (flags_val == -1)
	/* Something went wrong.  */
	continue;

      /* Now run the actual test.  */
      ++ntests;

#ifdef __UCLIBC_HAS_XLOCALE__
      if (setlocale (LC_COLLATE, locale) == NULL
	  || setlocale (LC_CTYPE, locale) == NULL)
	{
	  puts ("*** Cannot set locale");
	  ++nfailed;
	  continue;
	}
#else
      /* skip non-ascii strings */
      if (!str_isalpha(pattern) || !str_isalpha(input))
	{
	  ++nskipped;
	  printf("%3d: fnmatch (\"%s\", \"%s\"): SKIP multibyte test (requires locale support)\n", ++nr, pattern, input);
	  continue;
	}
      /* skip collating symbols */
      if (str_has_funk(pattern, '.') || str_has_funk(input, '.'))
	{
	  ++nskipped;
	  printf("%3d: fnmatch (\"%s\", \"%s\"): SKIP collating symbol test (requires locale support)\n", ++nr, pattern, input);
	  continue;
	}
      /* skip equivalence class expressions */
      if (str_has_funk(pattern, '=') || str_has_funk(input, '='))
	{
	  ++nskipped;
	  printf("%3d: fnmatch (\"%s\", \"%s\"): SKIP equivalence class test (requires locale support)\n", ++nr, pattern, input);
	  continue;
	}
#endif

      fnmres = fnmatch (pattern, input, flags_val);

      printf ("%3d: fnmatch (\"%s\", \"%s\", %s) = %s%c",
	      ++nr,
	      escape (pattern, &escpatternlen, &escpattern),
	      escape (input, &escinputlen, &escinput),
	      flag_output (flags_val),
	      (fnmres == 0
	       ? "0" : (fnmres == FNM_NOMATCH
			? "FNM_NOMATCH"
			: (sprintf (numbuf, "%d", fnmres), numbuf))),
	      (fnmres != 0) != (result != 0) ? ' ' : '\n');

      if ((fnmres != 0) != (result != 0))
	{
	  printf ("(FAIL, expected %s) ***\n",
		  result == 0
		  ? "0" : (result == FNM_NOMATCH
			   ? "FNM_NOMATCH"
			   : (sprintf (numbuf, "%d", result), numbuf)));
	  ++nfailed;
	}
    }

  printf ("=====================\n%3d tests, %3d failed, %3d skipped\n", ntests, nfailed, nskipped);

  free (escpattern);
  free (escinput);
  free (linebuf);

  return nfailed != 0;
}


static char *
next_input (char **line, int first, int last)
{
  char *cp = *line;
  char *result;

  while (*cp == ' ' || *cp == '\t')
    ++cp;

  /* We allow comment lines starting with '#'.  */
  if (first && *cp == '#')
    return NULL;

  if (*cp == '"')
    {
      char *wp;

      result = ++cp;
      wp = cp;

      while (*cp != '"' && *cp != '\0' && *cp != '\n')
	if (*cp == '\\')
	  {
	    if (cp[1] == '\n' || cp[1] == '\0')
	      return NULL;

	    ++cp;
	    if (*cp == 't')
	      *wp++ = '\t';
	    else if (*cp == 'n')
	      *wp++ = '\n';
	    else
	      *wp++ = *cp;

	    ++cp;
	  }
	else
	  *wp++ = *cp++;

      if (*cp != '"')
	return NULL;

      if (wp != cp)
	*wp = '\0';
    }
  else
    {
      result = cp;
      while (*cp != '\0' && *cp != '\n' && *cp != ' ' && *cp != '\t')
	++cp;

      if (cp == result && ! last)
	/* Premature end of line.  */
	return NULL;
    }

  /* Terminate and skip over the next white spaces.  */
  *cp++ = '\0';

  *line = cp;
  return result;
}


static int
convert_flags (const char *str)
{
  int result = 0;

  while (*str != '\0')
    {
      int len;

      if (strncasecmp (str, "PATHNAME", 8) == 0
	  && (str[8] == '|' || str[8] == '\0'))
	{
	  result |= FNM_PATHNAME;
	  len = 8;
	}
      else if (strncasecmp (str, "NOESCAPE", 8) == 0
	       && (str[8] == '|' || str[8] == '\0'))
	{
	  result |= FNM_NOESCAPE;
	  len = 8;
	}
      else if (strncasecmp (str, "PERIOD", 6) == 0
	       && (str[6] == '|' || str[6] == '\0'))
	{
	  result |= FNM_PERIOD;
	  len = 6;
	}
#ifdef FNM_LEADING_DIR
      else if (strncasecmp (str, "LEADING_DIR", 11) == 0
	       && (str[11] == '|' || str[11] == '\0'))
	{
	  result |= FNM_LEADING_DIR;
	  len = 11;
	}
#endif
#ifdef FNM_CASEFOLD
      else if (strncasecmp (str, "CASEFOLD", 8) == 0
	       && (str[8] == '|' || str[8] == '\0'))
	{
	  result |= FNM_CASEFOLD;
	  len = 8;
	}
#endif
#ifdef FNM_EXTMATCH
      else if (strncasecmp (str, "EXTMATCH", 8) == 0
	       && (str[8] == '|' || str[8] == '\0'))
	{
	  result |= FNM_EXTMATCH;
	  len = 8;
	}
#endif
      else
	return -1;

      str += len;
      if (*str != '\0')
	++str;
    }

  return result;
}


static char *
flag_output (int flags)
{
  static char buf[100];
  int first = 1;
  char *cp = buf;

  if (flags & FNM_PATHNAME)
    {
      cp = stpcpy (cp, "FNM_PATHNAME");
      first = 0;
    }
  if (flags & FNM_NOESCAPE)
    {
      if (! first)
	*cp++ = '|';
      cp = stpcpy (cp, "FNM_NOESCAPE");
      first = 0;
    }
  if (flags & FNM_PERIOD)
    {
      if (! first)
	*cp++ = '|';
      cp = stpcpy (cp, "FNM_PERIOD");
      first = 0;
    }
#ifdef FNM_LEADING_DIR
  if (flags & FNM_LEADING_DIR)
    {
      if (! first)
	*cp++ = '|';
      cp = stpcpy (cp, "FNM_LEADING_DIR");
      first = 0;
    }
#endif
#ifdef FNM_CASEFOLD
  if (flags & FNM_CASEFOLD)
    {
      if (! first)
	*cp++ = '|';
      cp = stpcpy (cp, "FNM_CASEFOLD");
      first = 0;
    }
#endif
#ifdef FNM_EXTMATCH
  if (flags & FNM_EXTMATCH)
    {
      if (! first)
	*cp++ = '|';
      cp = stpcpy (cp, "FNM_EXTMATCH");
      first = 0;
    }
#endif
  if (cp == buf)
    *cp++ = '0';
  *cp = '\0';

  return buf;
}


static char *
escape (const char *str, size_t *reslenp, char **resbufp)
{
  size_t reslen = *reslenp;
  char *resbuf = *resbufp;
  size_t len = strlen (str);
  char *wp;

  if (2 * len + 1 > reslen)
    {
      resbuf = (char *) realloc (resbuf, 2 * len + 1);
      if (resbuf == NULL)
	error (EXIT_FAILURE, errno, "while allocating buffer for printing");
      *reslenp = 2 * len + 1;
      *resbufp = resbuf;
    }

  wp = resbuf;
  while (*str != '\0')
    if (*str == '\t')
      {
	*wp++ = '\\';
	*wp++ = 't';
	++str;
      }
    else if (*str == '\n')
      {
	*wp++ = '\\';
	*wp++ = 'n';
	++str;
      }
    else if (*str == '"')
      {
	*wp++ = '\\';
	*wp++ = '"';
	++str;
      }
    else if (*str == '\\')
      {
	*wp++ = '\\';
	*wp++ = '\\';
	++str;
      }
    else
      *wp++ = *str++;

  *wp = '\0';

  return resbuf;
}
