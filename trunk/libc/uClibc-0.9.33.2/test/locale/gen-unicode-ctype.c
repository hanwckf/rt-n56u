/* Generate a Unicode conforming LC_CTYPE category from a UnicodeData file.
   Copyright (C) 2000-2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Bruno Haible <haible@clisp.cons.org>, 2000.

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

/* Usage example:
     $ gen-unicode /usr/local/share/Unidata/UnicodeData.txt 3.1
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

/* This structure represents one line in the UnicodeData.txt file.  */
struct unicode_attribute
{
  const char *name;           /* Character name */
  const char *category;       /* General category */
  const char *combining;      /* Canonical combining classes */
  const char *bidi;           /* Bidirectional category */
  const char *decomposition;  /* Character decomposition mapping */
  const char *decdigit;       /* Decimal digit value */
  const char *digit;          /* Digit value */
  const char *numeric;        /* Numeric value */
  int mirrored;               /* mirrored */
  const char *oldname;        /* Old Unicode 1.0 name */
  const char *comment;        /* Comment */
  unsigned int upper;         /* Uppercase mapping */
  unsigned int lower;         /* Lowercase mapping */
  unsigned int title;         /* Titlecase mapping */
};

/* Missing fields are represented with "" for strings, and NONE for
   characters.  */
#define NONE (~(unsigned int)0)

/* The entire contents of the UnicodeData.txt file.  */
struct unicode_attribute unicode_attributes [0x110000];

/* Stores in unicode_attributes[i] the values from the given fields.  */
static void
fill_attribute (unsigned int i,
		const char *field1, const char *field2,
		const char *field3, const char *field4,
		const char *field5, const char *field6,
		const char *field7, const char *field8,
		const char *field9, const char *field10,
		const char *field11, const char *field12,
		const char *field13, const char *field14)
{
  struct unicode_attribute * uni;

  if (i >= 0x110000)
    {
      fprintf (stderr, "index too large\n");
      exit (1);
    }
  if (strcmp (field2, "Cs") == 0)
    /* Surrogates are UTF-16 artefacts, not real characters. Ignore them.  */
    return;
  uni = &unicode_attributes[i];
  /* Copy the strings.  */
  uni->name          = strdup (field1);
  uni->category      = (field2[0] == '\0' ? "" : strdup (field2));
  uni->combining     = (field3[0] == '\0' ? "" : strdup (field3));
  uni->bidi          = (field4[0] == '\0' ? "" : strdup (field4));
  uni->decomposition = (field5[0] == '\0' ? "" : strdup (field5));
  uni->decdigit      = (field6[0] == '\0' ? "" : strdup (field6));
  uni->digit         = (field7[0] == '\0' ? "" : strdup (field7));
  uni->numeric       = (field8[0] == '\0' ? "" : strdup (field8));
  uni->mirrored      = (field9[0] == 'Y');
  uni->oldname       = (field10[0] == '\0' ? "" : strdup (field10));
  uni->comment       = (field11[0] == '\0' ? "" : strdup (field11));
  uni->upper = (field12[0] =='\0' ? NONE : strtoul (field12, NULL, 16));
  uni->lower = (field13[0] =='\0' ? NONE : strtoul (field13, NULL, 16));
  uni->title = (field14[0] =='\0' ? NONE : strtoul (field14, NULL, 16));
}

/* Maximum length of a field in the UnicodeData.txt file.  */
#define FIELDLEN 120

/* Reads the next field from STREAM.  The buffer BUFFER has size FIELDLEN.
   Reads up to (but excluding) DELIM.
   Returns 1 when a field was successfully read, otherwise 0.  */
static int
getfield (FILE *stream, char *buffer, int delim)
{
  int count = 0;
  int c;

  for (; (c = getc (stream)), (c != EOF && c != delim); )
    {
      /* The original unicode.org UnicodeData.txt file happens to have
	 CR/LF line terminators.  Silently convert to LF.  */
      if (c == '\r')
	continue;

      /* Put c into the buffer.  */
      if (++count >= FIELDLEN - 1)
	{
	  fprintf (stderr, "field too long\n");
	  exit (1);
	}
      *buffer++ = c;
    }

  if (c == EOF)
    return 0;

  *buffer = '\0';
  return 1;
}

/* Stores in unicode_attributes[] the entire contents of the UnicodeData.txt
   file.  */
static void
fill_attributes (const char *unicodedata_filename)
{
  unsigned int i, j;
  FILE *stream;
  char field0[FIELDLEN];
  char field1[FIELDLEN];
  char field2[FIELDLEN];
  char field3[FIELDLEN];
  char field4[FIELDLEN];
  char field5[FIELDLEN];
  char field6[FIELDLEN];
  char field7[FIELDLEN];
  char field8[FIELDLEN];
  char field9[FIELDLEN];
  char field10[FIELDLEN];
  char field11[FIELDLEN];
  char field12[FIELDLEN];
  char field13[FIELDLEN];
  char field14[FIELDLEN];
  int lineno = 0;

  for (i = 0; i < 0x110000; i++)
    unicode_attributes[i].name = NULL;

  stream = fopen (unicodedata_filename, "r");
  if (stream == NULL)
    {
      fprintf (stderr, "error during fopen of '%s'\n", unicodedata_filename);
      exit (1);
    }

  for (;;)
    {
      int n;

      lineno++;
      n = getfield (stream, field0, ';');
      n += getfield (stream, field1, ';');
      n += getfield (stream, field2, ';');
      n += getfield (stream, field3, ';');
      n += getfield (stream, field4, ';');
      n += getfield (stream, field5, ';');
      n += getfield (stream, field6, ';');
      n += getfield (stream, field7, ';');
      n += getfield (stream, field8, ';');
      n += getfield (stream, field9, ';');
      n += getfield (stream, field10, ';');
      n += getfield (stream, field11, ';');
      n += getfield (stream, field12, ';');
      n += getfield (stream, field13, ';');
      n += getfield (stream, field14, '\n');
      if (n == 0)
	break;
      if (n != 15)
	{
	  fprintf (stderr, "short line in'%s':%d\n",
		   unicodedata_filename, lineno);
	  exit (1);
	}
      i = strtoul (field0, NULL, 16);
      if (field1[0] == '<'
	  && strlen (field1) >= 9
	  && !strcmp (field1 + strlen(field1) - 8, ", First>"))
	{
	  /* Deal with a range. */
	  lineno++;
	  n = getfield (stream, field0, ';');
	  n += getfield (stream, field1, ';');
	  n += getfield (stream, field2, ';');
	  n += getfield (stream, field3, ';');
	  n += getfield (stream, field4, ';');
	  n += getfield (stream, field5, ';');
	  n += getfield (stream, field6, ';');
	  n += getfield (stream, field7, ';');
	  n += getfield (stream, field8, ';');
	  n += getfield (stream, field9, ';');
	  n += getfield (stream, field10, ';');
	  n += getfield (stream, field11, ';');
	  n += getfield (stream, field12, ';');
	  n += getfield (stream, field13, ';');
	  n += getfield (stream, field14, '\n');
	  if (n != 15)
	    {
	      fprintf (stderr, "missing end range in '%s':%d\n",
		       unicodedata_filename, lineno);
	      exit (1);
	    }
	  if (!(field1[0] == '<'
		&& strlen (field1) >= 8
		&& !strcmp (field1 + strlen (field1) - 7, ", Last>")))
	    {
	      fprintf (stderr, "missing end range in '%s':%d\n",
		       unicodedata_filename, lineno);
	      exit (1);
	    }
	  field1[strlen (field1) - 7] = '\0';
	  j = strtoul (field0, NULL, 16);
	  for (; i <= j; i++)
	    fill_attribute (i, field1+1, field2, field3, field4, field5,
			       field6, field7, field8, field9, field10,
			       field11, field12, field13, field14);
	}
      else
	{
	  /* Single character line */
	  fill_attribute (i, field1, field2, field3, field4, field5,
			     field6, field7, field8, field9, field10,
			     field11, field12, field13, field14);
	}
    }
  if (ferror (stream) || fclose (stream))
    {
      fprintf (stderr, "error reading from '%s'\n", unicodedata_filename);
      exit (1);
    }
}

/* Character mappings.  */

static unsigned int
to_upper (unsigned int ch)
{
  if (unicode_attributes[ch].name != NULL
      && unicode_attributes[ch].upper != NONE)
    return unicode_attributes[ch].upper;
  else
    return ch;
}

static unsigned int
to_lower (unsigned int ch)
{
  if (unicode_attributes[ch].name != NULL
      && unicode_attributes[ch].lower != NONE)
    return unicode_attributes[ch].lower;
  else
    return ch;
}

static unsigned int
to_title (unsigned int ch)
{
  if (unicode_attributes[ch].name != NULL
      && unicode_attributes[ch].title != NONE)
    return unicode_attributes[ch].title;
  else
    return ch;
}

/* Character class properties.  */

static bool
is_upper (unsigned int ch)
{
  return (to_lower (ch) != ch);
}

static bool
is_lower (unsigned int ch)
{
  return (to_upper (ch) != ch)
	 /* <U00DF> is lowercase, but without simple to_upper mapping.  */
	 || (ch == 0x00DF);
}

static bool
is_alpha (unsigned int ch)
{
  return (unicode_attributes[ch].name != NULL
	  && ((unicode_attributes[ch].category[0] == 'L'
	       /* Theppitak Karoonboonyanan <thep@links.nectec.or.th> says
		  <U0E2F>, <U0E46> should belong to is_punct.  */
	       && (ch != 0x0E2F) && (ch != 0x0E46))
	      /* Theppitak Karoonboonyanan <thep@links.nectec.or.th> says
		 <U0E31>, <U0E34>..<U0E3A>, <U0E47>..<U0E4E> are is_alpha.  */
	      || (ch == 0x0E31)
	      || (ch >= 0x0E34 && ch <= 0x0E3A)
	      || (ch >= 0x0E47 && ch <= 0x0E4E)
	      /* Avoid warning for <U0345>.  */
	      || (ch == 0x0345)
	      /* Avoid warnings for <U2160>..<U217F>.  */
	      || (unicode_attributes[ch].category[0] == 'N'
		  && unicode_attributes[ch].category[1] == 'l')
	      /* Avoid warnings for <U24B6>..<U24E9>.  */
	      || (unicode_attributes[ch].category[0] == 'S'
		  && unicode_attributes[ch].category[1] == 'o'
		  && strstr (unicode_attributes[ch].name, " LETTER ")
		     != NULL)
	      /* Consider all the non-ASCII digits as alphabetic.
		 ISO C 99 forbids us to have them in category "digit",
		 but we want iswalnum to return true on them.  */
	      || (unicode_attributes[ch].category[0] == 'N'
		  && unicode_attributes[ch].category[1] == 'd'
		  && !(ch >= 0x0030 && ch <= 0x0039))));
}

static bool
is_digit (unsigned int ch)
{
#if 0
  return (unicode_attributes[ch].name != NULL
	  && unicode_attributes[ch].category[0] == 'N'
	  && unicode_attributes[ch].category[1] == 'd');
  /* Note: U+0BE7..U+0BEF and U+1369..U+1371 are digit systems without
     a zero.  Must add <0> in front of them by hand.  */
#else
  /* SUSV2 gives us some freedom for the "digit" category, but ISO C 99
     takes it away:
     7.25.2.1.5:
        The iswdigit function tests for any wide character that corresponds
        to a decimal-digit character (as defined in 5.2.1).
     5.2.1:
        the 10 decimal digits 0 1 2 3 4 5 6 7 8 9
   */
  return (ch >= 0x0030 && ch <= 0x0039);
#endif
}

static bool
is_outdigit (unsigned int ch)
{
  return (ch >= 0x0030 && ch <= 0x0039);
}

static bool
is_blank (unsigned int ch)
{
  return (ch == 0x0009 /* '\t' */
	  /* Category Zs without mention of "<noBreak>" */
	  || (unicode_attributes[ch].name != NULL
	      && unicode_attributes[ch].category[0] == 'Z'
	      && unicode_attributes[ch].category[1] == 's'
	      && !strstr (unicode_attributes[ch].decomposition, "<noBreak>")));
}

static bool
is_space (unsigned int ch)
{
  /* Don't make U+00A0 a space. Non-breaking space means that all programs
     should treat it like a punctuation character, not like a space. */
  return (ch == 0x0020 /* ' ' */
	  || ch == 0x000C /* '\f' */
	  || ch == 0x000A /* '\n' */
	  || ch == 0x000D /* '\r' */
	  || ch == 0x0009 /* '\t' */
	  || ch == 0x000B /* '\v' */
	  /* Categories Zl, Zp, and Zs without mention of "<noBreak>" */
	  || (unicode_attributes[ch].name != NULL
	      && unicode_attributes[ch].category[0] == 'Z'
	      && (unicode_attributes[ch].category[1] == 'l'
		  || unicode_attributes[ch].category[1] == 'p'
		  || (unicode_attributes[ch].category[1] == 's'
		      && !strstr (unicode_attributes[ch].decomposition,
				  "<noBreak>")))));
}

static bool
is_cntrl (unsigned int ch)
{
  return (unicode_attributes[ch].name != NULL
	  && (!strcmp (unicode_attributes[ch].name, "<control>")
	      /* Categories Zl and Zp */
	      || (unicode_attributes[ch].category[0] == 'Z'
		  && (unicode_attributes[ch].category[1] == 'l'
		      || unicode_attributes[ch].category[1] == 'p'))));
}

static bool
is_xdigit (unsigned int ch)
{
#if 0
  return is_digit (ch)
	 || (ch >= 0x0041 && ch <= 0x0046)
	 || (ch >= 0x0061 && ch <= 0x0066);
#else
  /* SUSV2 gives us some freedom for the "xdigit" category, but ISO C 99
     takes it away:
     7.25.2.1.12:
        The iswxdigit function tests for any wide character that corresponds
        to a hexadecimal-digit character (as defined in 6.4.4.1).
     6.4.4.1:
        hexadecimal-digit: one of 0 1 2 3 4 5 6 7 8 9 a b c d e f A B C D E F
   */
  return (ch >= 0x0030 && ch <= 0x0039)
	 || (ch >= 0x0041 && ch <= 0x0046)
	 || (ch >= 0x0061 && ch <= 0x0066);
#endif
}

static bool
is_graph (unsigned int ch)
{
  return (unicode_attributes[ch].name != NULL
	  && strcmp (unicode_attributes[ch].name, "<control>")
	  && !is_space (ch));
}

static bool
is_print (unsigned int ch)
{
  return (unicode_attributes[ch].name != NULL
	  && strcmp (unicode_attributes[ch].name, "<control>")
	  /* Categories Zl and Zp */
	  && !(unicode_attributes[ch].name != NULL
	       && unicode_attributes[ch].category[0] == 'Z'
	       && (unicode_attributes[ch].category[1] == 'l'
		   || unicode_attributes[ch].category[1] == 'p')));
}

static bool
is_punct (unsigned int ch)
{
#if 0
  return (unicode_attributes[ch].name != NULL
	  && unicode_attributes[ch].category[0] == 'P');
#else
  /* The traditional POSIX definition of punctuation is every graphic,
     non-alphanumeric character.  */
  return (is_graph (ch) && !is_alpha (ch) && !is_digit (ch));
#endif
}

static bool
is_combining (unsigned int ch)
{
  /* Up to Unicode 3.0.1 we took the Combining property from the PropList.txt
     file. In 3.0.1 it was identical to the union of the general categories
     "Mn", "Mc", "Me". In Unicode 3.1 this property has been dropped from the
     PropList.txt file, so we take the latter definition.  */
  return (unicode_attributes[ch].name != NULL
	  && unicode_attributes[ch].category[0] == 'M'
	  && (unicode_attributes[ch].category[1] == 'n'
	      || unicode_attributes[ch].category[1] == 'c'
	      || unicode_attributes[ch].category[1] == 'e'));
}

static bool
is_combining_level3 (unsigned int ch)
{
  return is_combining (ch)
	 && !(unicode_attributes[ch].combining[0] != '\0'
	      && unicode_attributes[ch].combining[0] != '0'
	      && strtoul (unicode_attributes[ch].combining, NULL, 10) >= 200);
}

/* Return the UCS symbol string for a Unicode character.  */
static const char *
ucs_symbol (unsigned int i)
{
  static char buf[11+1];

  sprintf (buf, (i < 0x10000 ? "<U%04X>" : "<U%08X>"), i);
  return buf;
}

/* Return the UCS symbol range string for a Unicode characters interval.  */
static const char *
ucs_symbol_range (unsigned int low, unsigned int high)
{
  static char buf[24+1];

  strcpy (buf, ucs_symbol (low));
  strcat (buf, "..");
  strcat (buf, ucs_symbol (high));
  return buf;
}

/* Output a character class (= property) table.  */

static void
output_charclass (FILE *stream, const char *classname,
		  bool (*func) (unsigned int))
{
  char table[0x110000];
  unsigned int i;
  bool need_semicolon;
  const int max_column = 75;
  int column;

  for (i = 0; i < 0x110000; i++)
    table[i] = (int) func (i);

  fprintf (stream, "%s ", classname);
  need_semicolon = false;
  column = 1000;
  for (i = 0; i < 0x110000; )
    {
      if (!table[i])
	i++;
      else
	{
	  unsigned int low, high;
	  char buf[25];

	  low = i;
	  do
	    i++;
	  while (i < 0x110000 && table[i]);
	  high = i - 1;

	  if (low == high)
	    strcpy (buf, ucs_symbol (low));
	  else
	    strcpy (buf, ucs_symbol_range (low, high));

	  if (need_semicolon)
	    {
	      fprintf (stream, ";");
	      column++;
	    }

	  if (column + strlen (buf) > max_column)
	    {
	      fprintf (stream, "/\n   ");
	      column = 3;
	    }

	  fprintf (stream, "%s", buf);
	  column += strlen (buf);
	  need_semicolon = true;
	}
    }
  fprintf (stream, "\n");
}

/* Output a character mapping table.  */

static void
output_charmap (FILE *stream, const char *mapname,
		unsigned int (*func) (unsigned int))
{
  char table[0x110000];
  unsigned int i;
  bool need_semicolon;
  const int max_column = 75;
  int column;

  for (i = 0; i < 0x110000; i++)
    table[i] = (func (i) != i);

  fprintf (stream, "%s ", mapname);
  need_semicolon = false;
  column = 1000;
  for (i = 0; i < 0x110000; i++)
    if (table[i])
      {
	char buf[25+1];

	strcpy (buf, "(");
	strcat (buf, ucs_symbol (i));
	strcat (buf, ",");
	strcat (buf, ucs_symbol (func (i)));
	strcat (buf, ")");

	if (need_semicolon)
	  {
	    fprintf (stream, ";");
	    column++;
	  }

	if (column + strlen (buf) > max_column)
	  {
	    fprintf (stream, "/\n   ");
	    column = 3;
	  }

	fprintf (stream, "%s", buf);
	column += strlen (buf);
	need_semicolon = true;
      }
  fprintf (stream, "\n");
}

/* Output the width table.  */

static void
output_widthmap (FILE *stream)
{
}

/* Output the tables to the given file.  */

static void
output_tables (const char *filename, const char *version)
{
  FILE *stream;
  unsigned int ch;

  stream = fopen (filename, "w");
  if (stream == NULL)
    {
      fprintf (stderr, "cannot open '%s' for writing\n", filename);
      exit (1);
    }

  fprintf (stream, "escape_char /\n");
  fprintf (stream, "comment_char %%\n");
  fprintf (stream, "\n");
  fprintf (stream, "%% Generated automatically by gen-unicode-ctype for Unicode %s.\n",
	   version);
  fprintf (stream, "\n");

  fprintf (stream, "LC_IDENTIFICATION\n");
  fprintf (stream, "title     \"Unicode %s FDCC-set\"\n", version);
  fprintf (stream, "source    \"UnicodeData.txt, PropList.txt\"\n");
  fprintf (stream, "address   \"\"\n");
  fprintf (stream, "contact   \"\"\n");
  fprintf (stream, "email     \"bug-glibc-locales@gnu.org\"\n");
  fprintf (stream, "tel       \"\"\n");
  fprintf (stream, "fax       \"\"\n");
  fprintf (stream, "language  \"\"\n");
  fprintf (stream, "territory \"Earth\"\n");
  fprintf (stream, "revision  \"%s\"\n", version);
  {
    time_t now;
    char date[11];
    now = time (NULL);
    strftime (date, sizeof (date), "%Y-%m-%d", gmtime (&now));
    fprintf (stream, "date      \"%s\"\n", date);
  }
  fprintf (stream, "category  \"unicode:2001\";LC_CTYPE\n");
  fprintf (stream, "END LC_IDENTIFICATION\n");
  fprintf (stream, "\n");

  /* Verifications. */
  for (ch = 0; ch < 0x110000; ch++)
    {
      /* toupper restriction: "Only characters specified for the keywords
	 lower and upper shall be specified.  */
      if (to_upper (ch) != ch && !(is_lower (ch) || is_upper (ch)))
	fprintf (stderr,
		 "%s is not upper|lower but toupper(0x%04X) = 0x%04X\n",
		 ucs_symbol (ch), ch, to_upper (ch));

      /* tolower restriction: "Only characters specified for the keywords
	 lower and upper shall be specified.  */
      if (to_lower (ch) != ch && !(is_lower (ch) || is_upper (ch)))
	fprintf (stderr,
		 "%s is not upper|lower but tolower(0x%04X) = 0x%04X\n",
		 ucs_symbol (ch), ch, to_lower (ch));

      /* alpha restriction: "Characters classified as either upper or lower
	 shall automatically belong to this class.  */
      if ((is_lower (ch) || is_upper (ch)) && !is_alpha (ch))
	fprintf (stderr, "%s is upper|lower but not alpha\n", ucs_symbol (ch));

      /* alpha restriction: "No character specified for the keywords cntrl,
	 digit, punct or space shall be specified."  */
      if (is_alpha (ch) && is_cntrl (ch))
	fprintf (stderr, "%s is alpha and cntrl\n", ucs_symbol (ch));
      if (is_alpha (ch) && is_digit (ch))
	fprintf (stderr, "%s is alpha and digit\n", ucs_symbol (ch));
      if (is_alpha (ch) && is_punct (ch))
	fprintf (stderr, "%s is alpha and punct\n", ucs_symbol (ch));
      if (is_alpha (ch) && is_space (ch))
	fprintf (stderr, "%s is alpha and space\n", ucs_symbol (ch));

      /* space restriction: "No character specified for the keywords upper,
	 lower, alpha, digit, graph or xdigit shall be specified."
	 upper, lower, alpha already checked above.  */
      if (is_space (ch) && is_digit (ch))
	fprintf (stderr, "%s is space and digit\n", ucs_symbol (ch));
      if (is_space (ch) && is_graph (ch))
	fprintf (stderr, "%s is space and graph\n", ucs_symbol (ch));
      if (is_space (ch) && is_xdigit (ch))
	fprintf (stderr, "%s is space and xdigit\n", ucs_symbol (ch));

      /* cntrl restriction: "No character specified for the keywords upper,
	 lower, alpha, digit, punct, graph, print or xdigit shall be
	 specified."  upper, lower, alpha already checked above.  */
      if (is_cntrl (ch) && is_digit (ch))
	fprintf (stderr, "%s is cntrl and digit\n", ucs_symbol (ch));
      if (is_cntrl (ch) && is_punct (ch))
	fprintf (stderr, "%s is cntrl and punct\n", ucs_symbol (ch));
      if (is_cntrl (ch) && is_graph (ch))
	fprintf (stderr, "%s is cntrl and graph\n", ucs_symbol (ch));
      if (is_cntrl (ch) && is_print (ch))
	fprintf (stderr, "%s is cntrl and print\n", ucs_symbol (ch));
      if (is_cntrl (ch) && is_xdigit (ch))
	fprintf (stderr, "%s is cntrl and xdigit\n", ucs_symbol (ch));

      /* punct restriction: "No character specified for the keywords upper,
	 lower, alpha, digit, cntrl, xdigit or as the <space> character shall
	 be specified."  upper, lower, alpha, cntrl already checked above.  */
      if (is_punct (ch) && is_digit (ch))
	fprintf (stderr, "%s is punct and digit\n", ucs_symbol (ch));
      if (is_punct (ch) && is_xdigit (ch))
	fprintf (stderr, "%s is punct and xdigit\n", ucs_symbol (ch));
      if (is_punct (ch) && (ch == 0x0020))
	fprintf (stderr, "%s is punct\n", ucs_symbol (ch));

      /* graph restriction: "No character specified for the keyword cntrl
	 shall be specified."  Already checked above.  */

      /* print restriction: "No character specified for the keyword cntrl
	 shall be specified."  Already checked above.  */

      /* graph - print relation: differ only in the <space> character.
	 How is this possible if there are more than one space character?!
	 I think susv2/xbd/locale.html should speak of "space characters",
	 not "space character".  */
      if (is_print (ch) && !(is_graph (ch) || /* ch == 0x0020 */ is_space (ch)))
	fprintf (stderr,
		 "%s is print but not graph|<space>\n", ucs_symbol (ch));
      if (!is_print (ch) && (is_graph (ch) || ch == 0x0020))
	fprintf (stderr,
		 "%s is graph|<space> but not print\n", ucs_symbol (ch));
    }

  fprintf (stream, "LC_CTYPE\n");
  output_charclass (stream, "upper", is_upper);
  output_charclass (stream, "lower", is_lower);
  output_charclass (stream, "alpha", is_alpha);
  output_charclass (stream, "digit", is_digit);
  output_charclass (stream, "outdigit", is_outdigit);
  output_charclass (stream, "blank", is_blank);
  output_charclass (stream, "space", is_space);
  output_charclass (stream, "cntrl", is_cntrl);
  output_charclass (stream, "punct", is_punct);
  output_charclass (stream, "xdigit", is_xdigit);
  output_charclass (stream, "graph", is_graph);
  output_charclass (stream, "print", is_print);
  output_charclass (stream, "class \"combining\";", is_combining);
  output_charclass (stream, "class \"combining_level3\";", is_combining_level3);
  output_charmap (stream, "toupper", to_upper);
  output_charmap (stream, "tolower", to_lower);
  output_charmap (stream, "map \"totitle\";", to_title);
  output_widthmap (stream);
  fprintf (stream, "END LC_CTYPE\n");

  if (ferror (stream) || fclose (stream))
    {
      fprintf (stderr, "error writing to '%s'\n", filename);
      exit (1);
    }
}

int
main (int argc, char * argv[])
{
  if (argc != 3)
    {
      fprintf (stderr, "Usage: %s UnicodeData.txt version\n", argv[0]);
      exit (1);
    }

  fill_attributes (argv[1]);

  output_tables ("unicode", argv[2]);

  return 0;
}
