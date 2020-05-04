/**
 * unistr.c - Unicode string handling. Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2000-2004 Anton Altaparmakov
 * Copyright (c) 2002-2009 Szabolcs Szakacsits
 * Copyright (c) 2008-2015 Jean-Pierre Andre
 * Copyright (c) 2008      Bernhard Kaindl
 *
 * This program/include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program/include file is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/* TODO: Make use of functions here in creating/renaming etc. */
/* TODO: Cleanup and check what is really needed and in what context. */

#include "antfs.h"
#include "attrib.h"
#include "types.h"
#include "unistr.h"
#include "debug.h"
#include "misc.h"

#ifndef ALLOW_BROKEN_UNICODE
/* Erik allowing broken UTF-16 surrogate pairs and U+FFFE and U+FFFF by default,
 * open to debate. */
#define ALLOW_BROKEN_UNICODE 1
#endif /* !defined(ALLOW_BROKEN_UNICODE) */

/*
 * IMPORTANT
 * =========
 *
 * All these routines assume that the Unicode characters are in little endian
 * encoding inside the strings!!!
 */

static int use_utf8 = 1; /* use UTF-8 encoding for file names */

/*
 * This is used by the name collation functions to quickly determine what
 * characters are (in)valid.
 */
#if 0
static const u8 legal_ansi_char_array[0x40] = {
	0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,

	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,

	0x17, 0x07, 0x18, 0x17, 0x17, 0x17, 0x17, 0x17,
	0x17, 0x17, 0x18, 0x16, 0x16, 0x17, 0x07, 0x00,

	0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17,
	0x17, 0x17, 0x04, 0x16, 0x18, 0x16, 0x18, 0x18,
};
#endif

/**
 * ntfs_names_are_equal - compare two Unicode names for equality
 * @s1:			name to compare to @s2
 * @s1_len:		length in Unicode characters of @s1
 * @s2:			name to compare to @s1
 * @s2_len:		length in Unicode characters of @s2
 * @ic:			ignore case bool
 * @upcase:		upcase table (only if @ic == IGNORE_CASE)
 * @upcase_size:	length in Unicode characters of @upcase (if present)
 *
 * Compare the names @s1 and @s2 and return TRUE (1) if the names are
 * identical, or FALSE (0) if they are not identical. If @ic is IGNORE_CASE,
 * the @upcase table is used to perform a case insensitive comparison.
 */
bool ntfs_names_are_equal(const ntfschar *s1, size_t s1_len,
		const ntfschar *s2, size_t s2_len,
		const enum IGNORE_CASE_BOOL ic,
		const ntfschar *upcase, const u32 upcase_size)
{
	if (s1_len != s2_len)
		return FALSE;
	if (!s1_len)
		return TRUE;
	if (ic == CASE_SENSITIVE)
		return ntfs_ucsncmp(s1, s2, s1_len) ? FALSE : TRUE;
	return ntfs_ucsncasecmp(s1, s2, s1_len, upcase, upcase_size) ? FALSE :
								       TRUE;
}

/*
 * ntfs_names_full_collate() fully collate two Unicode names
 *
 * @name1:	first Unicode name to compare
 * @name1_len:	length of first Unicode name to compare
 * @name2:	second Unicode name to compare
 * @name2_len:	length of second Unicode name to compare
 * @ic:		either CASE_SENSITIVE or IGNORE_CASE (see below)
 * @upcase:	upcase table
 * @upcase_len:	upcase table size
 *
 * If @ic is CASE_SENSITIVE, then the names are compared primarily ignoring
 * case, but if the names are equal ignoring case, then they are compared
 * case-sensitively.  As an example, "abc" would collate before "BCD" (since
 * "abc" and "BCD" differ ignoring case and 'A' < 'B') but after "ABC" (since
 * "ABC" and "abc" are equal ignoring case and 'A' < 'a').  This matches the
 * collation order of filenames as indexed in NTFS directories.
 *
 * If @ic is IGNORE_CASE, then the names are only compared case-insensitively
 * and are considered to match if and only if they are equal ignoring case.
 *
 * Returns:
 *  -1 if the first name collates before the second one,
 *   0 if the names match, or
 *   1 if the second name collates before the first one
 */
int ntfs_names_full_collate(const ntfschar *name1, const u32 name1_len,
		const ntfschar *name2, const u32 name2_len,
		const enum IGNORE_CASE_BOOL ic, const ntfschar *upcase,
		const u32 upcase_len)
{
	u32 cnt;
	u16 c1, c2;
	u16 u1, u2;

	cnt = min(name1_len, name2_len);
	if (cnt > 0) {
		if (ic == CASE_SENSITIVE) {
			while (--cnt && (*name1 == *name2)) {
				name1++;
				name2++;
			}
			u1 = c1 = le16_to_cpu(*name1);
			u2 = c2 = le16_to_cpu(*name2);
			if (u1 < upcase_len)
				u1 = le16_to_cpu(upcase[u1]);
			if (u2 < upcase_len)
				u2 = le16_to_cpu(upcase[u2]);
			if ((u1 == u2) && cnt)
				do {
					name1++;
					u1 = le16_to_cpu(*name1);
					name2++;
					u2 = le16_to_cpu(*name2);
					if (u1 < upcase_len)
						u1 = le16_to_cpu(upcase[u1]);
					if (u2 < upcase_len)
						u2 = le16_to_cpu(upcase[u2]);
				} while ((u1 == u2) && --cnt);
			if (u1 < u2)
				return -1;
			if (u1 > u2)
				return 1;
			if (name1_len < name2_len)
				return -1;
			if (name1_len > name2_len)
				return 1;
			if (c1 < c2)
				return -1;
			if (c1 > c2)
				return 1;
		} else {
			do {
				u1 = le16_to_cpu(*name1);
				name1++;
				u2 = le16_to_cpu(*name2);
				name2++;
				if (u1 < upcase_len)
					u1 = le16_to_cpu(upcase[u1]);
				if (u2 < upcase_len)
					u2 = le16_to_cpu(upcase[u2]);
			} while ((u1 == u2) && --cnt);
			if (u1 < u2)
				return -1;
			if (u1 > u2)
				return 1;
			if (name1_len < name2_len)
				return -1;
			if (name1_len > name2_len)
				return 1;
		}
	} else {
		if (name1_len < name2_len)
			return -1;
		if (name1_len > name2_len)
			return 1;
	}
	return 0;
}

/**
 * ntfs_ucsncmp - compare two little endian Unicode strings
 * @s1:		first string
 * @s2:		second string
 * @n:		maximum unicode characters to compare
 *
 * Compare the first @n characters of the Unicode strings @s1 and @s2,
 * The strings in little endian format and appropriate le16_to_cpu()
 * conversion is performed on non-little endian machines.
 *
 * The function returns an integer less than, equal to, or greater than zero
 * if @s1 (or the first @n Unicode characters thereof) is found, respectively,
 * to be less than, to match, or be greater than @s2.
 */
int ntfs_ucsncmp(const ntfschar *s1, const ntfschar *s2, size_t n)
{
	u16 c1, c2;
	size_t i;

#ifdef DEBUG
	if (!s1 || !s2) {
		antfs_log_debug("ntfs_wcsncmp() received NULL pointer!");
		BUG();
	}
#endif
	for (i = 0; i < n; ++i) {
		c1 = le16_to_cpu(s1[i]);
		c2 = le16_to_cpu(s2[i]);
		if (c1 < c2)
			return -1;
		if (c1 > c2)
			return 1;
		if (!c1)
			break;
	}
	return 0;
}

/**
 * ntfs_ucsncasecmp - compare two little endian Unicode strings, ignoring case
 * @s1:			first string
 * @s2:			second string
 * @n:			maximum unicode characters to compare
 * @upcase:		upcase table
 * @upcase_size:	upcase table size in Unicode characters
 *
 * Compare the first @n characters of the Unicode strings @s1 and @s2,
 * ignoring case. The strings in little endian format and appropriate
 * le16_to_cpu() conversion is performed on non-little endian machines.
 *
 * Each character is uppercased using the @upcase table before the comparison.
 *
 * The function returns an integer less than, equal to, or greater than zero
 * if @s1 (or the first @n Unicode characters thereof) is found, respectively,
 * to be less than, to match, or be greater than @s2.
 */
int ntfs_ucsncasecmp(const ntfschar *s1, const ntfschar *s2, size_t n,
		const ntfschar *upcase, const u32 upcase_size)
{
	u16 c1, c2;
	size_t i;

#ifdef DEBUG
	if (!s1 || !s2 || !upcase) {
		antfs_log_debug("ntfs_wcsncasecmp() received NULL pointer!");
		BUG();
	}
#endif
	for (i = 0; i < n; ++i) {
		c1 = le16_to_cpu(s1[i]);
		c2 = le16_to_cpu(s2[i]);
		if (c1 < upcase_size)
			c1 = le16_to_cpu(upcase[c1]);
		if (c2 < upcase_size)
			c2 = le16_to_cpu(upcase[c2]);
		if (c1 < c2)
			return -1;
		if (c1 > c2)
			return 1;
		if (!c1)
			break;
	}
	return 0;
}

/**
 * ntfs_ucsnlen - determine the length of a little endian Unicode string
 * @s:		pointer to Unicode string
 * @maxlen:	maximum length of string @s
 *
 * Return the number of Unicode characters in the little endian Unicode
 * string @s up to a maximum of maxlen Unicode characters, not including
 * the terminating (ntfschar)'\0'. If there is no (ntfschar)'\0' between @s
 * and @s + @maxlen, @maxlen is returned.
 *
 * This function never looks beyond @s + @maxlen.
 */
u32 ntfs_ucsnlen(const ntfschar *s, u32 maxlen)
{
	u32 i;

	for (i = 0; i < maxlen; i++) {
		if (!le16_to_cpu(s[i]))
			break;
	}
	return i;
}

/**
 * ntfs_ucsndup - duplicate little endian Unicode string
 * @s:		pointer to Unicode string
 * @maxlen:	maximum length of string @s
 *
 * Return a pointer to a new little endian Unicode string which is a duplicate
 * of the string s.  Memory for the new string is obtained with ntfs_malloc(3),
 * and can be freed with ntfs_free(3).
 *
 * A maximum of @maxlen Unicode characters are copied and a terminating
 * (ntfschar)'\0' little endian Unicode character is added.
 *
 * This function never looks beyond @s + @maxlen.
 *
 * Return a pointer to the new little endian Unicode string on success and
 * on failure the error code.
 */
ntfschar *ntfs_ucsndup(const ntfschar *s, u32 maxlen)
{
	ntfschar *dst;
	u32 len;

	len = ntfs_ucsnlen(s, maxlen);
	dst = ntfs_malloc((len + 1) * sizeof(ntfschar));
	if (dst) {
		memcpy(dst, s, len * sizeof(ntfschar));
		dst[len] = const_cpu_to_le16(L'\0');
	} else {
		dst = ERR_PTR(-ENOMEM);
	}
	return dst;
}

/**
 * ntfs_name_locase - Map a Unicode name to its lowercase equivalent
 */
void ntfs_name_locase(ntfschar *name, u32 name_len, const ntfschar *locase,
		const u32 locase_len)
{
	u32 i;
	u16 u;

	if (locase)
		for (i = 0; i < name_len; i++) {
			u = le16_to_cpu(name[i]);
			if (u < locase_len)
				name[i] = locase[u];
		}
}

/*
   NTFS uses Unicode (UTF-16LE [NTFS-3G uses UCS-2LE, which is enough
   for now]) for path names, but the Unicode code points need to be
   converted before a path can be accessed under NTFS. For 7 bit ASCII/ANSI,
   glibc does this even without a locale in a hard-coded fashion as that
   appears to be is easy because the low 7-bit ASCII range appears to be
   available in all charsets but it does not convert anything if
   there was some error with the locale setup or none set up like
   when mount is called during early boot where he (by policy) do
   not use locales (and may be not available if /usr is not yet mounted),
   so this patch fixes the resulting issues for systems which use
   UTF-8 and for others, specifying the locale in fstab brings them
   the encoding which they want.

   If no locale is defined or there was a problem with setting one
   up and whenever nl_langinfo(CODESET) returns a sting starting with
   "ANSI", use an internal UCS-2LE <-> UTF-8 codeset converter to fix
   the bug where NTFS-3G does not show any path names which include
   international characters!!! (and also fails on creating them) as result.

   Author: Bernhard Kaindl <bk@suse.de>
   Jean-Pierre Andre made it compliant with RFC3629/RFC2781.
*/

/*
 * Return the number of bytes in UTF-8 needed (without the terminating null) to
 * store the given UTF-16LE string.
 *
 * On error, -1 is returned, and errno is set to the error code. The following
 * error codes can be expected:
 *	EILSEQ		The input string is not valid UTF-16LE (only possible
 *			if compiled without ALLOW_BROKEN_UNICODE).
 *	ENAMETOOLONG	The length of the UTF-8 string in bytes (without the
 *			terminating null) would exceed @outs_len.
 */
static int utf16_to_utf8_size(const ntfschar *ins, const int ins_len,
			      int outs_len)
{
	int i, ret = 0;
	int count = 0;
	int surrog = 0;

	for (i = 0; i < ins_len && ins[i] && count <= outs_len; i++) {
		unsigned short c = le16_to_cpu(ins[i]);
		if (surrog) {
			if ((c >= 0xdc00) && (c < 0xe000)) {
				surrog = FALSE;
				count += 4;
			} else {
#if ALLOW_BROKEN_UNICODE
				/* The first UTF-16 unit of a surrogate pair has
				 * a value between 0xd800 and 0xdc00. It can be
				 * encoded as an individual UTF-8 sequence if we
				 * cannot combine it with the next UTF-16 unit
				 * unit as a surrogate pair. */
				surrog = FALSE;
				count += 3;

				--i;
				continue;
#else
				goto fail;
#endif /* ALLOW_BROKEN_UNICODE */
			}
		} else {
			if (c < 0x80)
				count++;
			else if (c < 0x800)
				count += 2;
			else if (c < 0xd800)
				count += 3;
			else if (c < 0xdc00)
				surrog = TRUE;
#if ALLOW_BROKEN_UNICODE
			else if (c < 0xe000)
				count += 3;
			else if (c >= 0xe000)
#else
			else if ((c >= 0xe000) && (c < 0xfffe))
#endif /* ALLOW_BROKEN_UNICODE */
				count += 3;
			else
				goto fail;
		}
	}

	if (surrog && count <= outs_len) {
#if ALLOW_BROKEN_UNICODE
		count += 3; /* ending with a single surrogate */
#else
		goto fail;
#endif /* ALLOW_BROKEN_UNICODE */
	}

	if (count > outs_len) {
		ret = -ENAMETOOLONG;
		goto out;
	}

	ret = count;
out:
	return ret;
fail:
	ret = -EILSEQ;
	goto out;
}

/*
 * ntfs_utf16_to_utf8 - convert a little endian UTF16LE string to an UTF-8
 *			string
 * @ins:	input utf16 string buffer
 * @ins_len:	length of input string in utf16 characters
 * @outs:	on return contains the (allocated) output multibyte string
 * @outs_len:	length of output buffer in bytes (ignored if *@outs is NULL)
 *
 * Return error code if string has invalid byte sequence or too long.
 */
static int ntfs_utf16_to_utf8(const ntfschar *ins, const int ins_len,
			      char **outs, int outs_len)
{
	char *t;
	int i, size, ret = 0;
	int halfpair;

	halfpair = 0;
	if (!*outs) {
		/* If no output buffer was provided, we will allocate one and
		 * limit its length to PATH_MAX.  Note: we follow the standard
		 * convention of PATH_MAX including the terminating null. */
		outs_len = PATH_MAX;
	}

	/* The size *with* the terminating null is limited to @outs_len,
	 * so the size *without* the terminating null is limited to one less. */
	size = utf16_to_utf8_size(ins, ins_len, outs_len - 1);

	if (size < 0) {
		ret = size;
		goto out;
	}

	if (!*outs) {
		outs_len = size + 1;
		*outs = ntfs_malloc(outs_len);
		if (!*outs) {
			ret = -ENOMEM;
			goto out;
		}
	}

	t = *outs;

	for (i = 0; i < ins_len && ins[i]; i++) {
		unsigned short c = le16_to_cpu(ins[i]);
			/* size not double-checked */
		if (halfpair) {
			if ((c >= 0xdc00) && (c < 0xe000)) {
				*t++ = 0xf0 + (((halfpair + 64) >> 8) & 7);
				*t++ = 0x80 + (((halfpair + 64) >> 2) & 63);
				*t++ = 0x80 + ((c >> 6) & 15) +
					((halfpair & 3) << 4);
				*t++ = 0x80 + (c & 63);
				halfpair = 0;
			} else {
#if ALLOW_BROKEN_UNICODE
				/* The first UTF-16 unit of a surrogate pair has
				 * a value between 0xd800 and 0xdc00. It can be
				 * encoded as an individual UTF-8 sequence if we
				 * cannot combine it with the next UTF-16 unit
				 * unit as a surrogate pair. */
				*t++ = 0xe0 | (halfpair >> 12);
				*t++ = 0x80 | ((halfpair >> 6) & 0x3f);
				*t++ = 0x80 | (halfpair & 0x3f);
				halfpair = 0;

				--i;
				continue;
#else
				goto fail;
#endif /* ALLOW_BROKEN_UNICODE */
			}
		} else if (c < 0x80) {
			*t++ = c;
		} else {
			if (c < 0x800) {
				*t++ = (0xc0 | ((c >> 6) & 0x3f));
				*t++ = 0x80 | (c & 0x3f);
			} else if (c < 0xd800) {
				*t++ = 0xe0 | (c >> 12);
				*t++ = 0x80 | ((c >> 6) & 0x3f);
				*t++ = 0x80 | (c & 0x3f);
			} else if (c < 0xdc00)
				halfpair = c;
#if ALLOW_BROKEN_UNICODE
			else if (c < 0xe000) {
				*t++ = 0xe0 | (c >> 12);
				*t++ = 0x80 | ((c >> 6) & 0x3f);
				*t++ = 0x80 | (c & 0x3f);
			}
#endif /* ALLOW_BROKEN_UNICODE */
			else if (c >= 0xe000) {
				*t++ = 0xe0 | (c >> 12);
				*t++ = 0x80 | ((c >> 6) & 0x3f);
				*t++ = 0x80 | (c & 0x3f);
			} else {
				goto fail;
			}
		}
	}
#if ALLOW_BROKEN_UNICODE
	if (halfpair) { /* ending with a single surrogate */
		*t++ = 0xe0 | (halfpair >> 12);
		*t++ = 0x80 | ((halfpair >> 6) & 0x3f);
		*t++ = 0x80 | (halfpair & 0x3f);
	}
#endif /* ALLOW_BROKEN_UNICODE */
	*t = '\0';

	ret = t - *outs;
out:
	return ret;
fail:
	ret = -EILSEQ;
	goto out;
}

/*
 * Return the amount of 16-bit elements in UTF-16LE needed
 * (without the terminating null) to store given UTF-8 string.
 *
 * Return error code if it's longer than PATH_MAX or string is invalid.
 *
 * Note: This does not check whether the input sequence is a valid utf8 string,
 *	 and should be used only in context where such check is made!
 */
static int utf8_to_utf16_size(const char *s)
{
	int ret = 0;
	unsigned int byte;
	size_t count = 0;

	while ((byte = *((const unsigned char *)s++))) {
		if (++count >= PATH_MAX)
			goto fail;
		if (byte >= 0xc0) {
			if (byte >= 0xF5) {
				ret = -EILSEQ;
				goto out;
			}
			if (!*s)
				break;
			if (byte >= 0xC0)
				s++;
			if (!*s)
				break;
			if (byte >= 0xE0)
				s++;
			if (!*s)
				break;
			if (byte >= 0xF0) {
				s++;
				if (++count >= PATH_MAX)
					goto fail;
			}
		}
	}
	ret = count;
out:
	return ret;
fail:
	ret = -ENAMETOOLONG;
	goto out;
}
/*
 * This converts one UTF-8 sequence to cpu-endian Unicode value
 * within range U+0 .. U+10ffff and excluding U+D800 .. U+DFFF
 *
 * Return the number of used utf8 bytes or error code if sequence is invalid.
 */
static int utf8_to_unicode(u32 *wc, const char *s)
{
	unsigned int byte = *((const unsigned char *)s);
					/* single byte */
	if (byte == 0) {
		*wc = (u32) 0;
		return 0;
	} else if (byte < 0x80) {
		*wc = (u32) byte;
		return 1;
					/* double byte */
	} else if (byte < 0xc2) {
		goto fail;
	} else if (byte < 0xE0) {
		if ((s[1] & 0xC0) == 0x80) {
			*wc = ((u32)(byte & 0x1F) << 6)
			    | ((u32)(s[1] & 0x3F));
			return 2;
		} else {
			goto fail;
		}
					/* three-byte */
	} else if (byte < 0xF0) {
		if (((s[1] & 0xC0) == 0x80) && ((s[2] & 0xC0) == 0x80)) {
			*wc = ((u32)(byte & 0x0F) << 12)
			    | ((u32)(s[1] & 0x3F) << 6)
			    | ((u32)(s[2] & 0x3F));
			/* Check valid ranges */
#if ALLOW_BROKEN_UNICODE
			if (((*wc >= 0x800) && (*wc <= 0xD7FF))
			  || ((*wc >= 0xD800) && (*wc <= 0xDFFF))
			  || ((*wc >= 0xe000) && (*wc <= 0xFFFF)))
				return 3;
#else
			if (((*wc >= 0x800) && (*wc <= 0xD7FF))
			  || ((*wc >= 0xe000) && (*wc <= 0xFFFD)))
				return 3;
#endif /* ALLOW_BROKEN_UNICODE */
		}
		goto fail;
					/* four-byte */
	} else if (byte < 0xF5) {
		if (((s[1] & 0xC0) == 0x80) && ((s[2] & 0xC0) == 0x80)
		  && ((s[3] & 0xC0) == 0x80)) {
			*wc = ((u32)(byte & 0x07) << 18)
			    | ((u32)(s[1] & 0x3F) << 12)
			    | ((u32)(s[2] & 0x3F) << 6)
			    | ((u32)(s[3] & 0x3F));
			/* Check valid ranges */
			if ((*wc <= 0x10ffff) && (*wc >= 0x10000))
				return 4;
		}
		goto fail;
	}
fail:
	return -EILSEQ;
}

/**
 * ntfs_utf8_to_utf16 - convert a UTF-8 string to a UTF-16LE string
 * @ins:	input multibyte string buffer
 * @outs:	on return contains the (allocated) output utf16 string
 * @outs_len:	length of output buffer in utf16 characters
 *
 * Return error code on error.
 */
static int ntfs_utf8_to_utf16(const char *ins, ntfschar **outs)
{
	const char *t = ins;
	u32 wc;
	int allocated;
	ntfschar *outpos;
	int shorts, ret = 0;

	shorts = utf8_to_utf16_size(ins);
	if (shorts < 0) {
		ret = shorts;
		goto fail;
	}

	allocated = FALSE;
	if (!*outs) {
		*outs = ntfs_malloc((shorts + 1) * sizeof(ntfschar));
		if (!*outs) {
			ret = -ENOMEM;
			goto fail;
		}
		allocated = TRUE;
	}

	outpos = *outs;

	while (1) {
		int m  = utf8_to_unicode(&wc, t);
		if (m <= 0) {
			if (m < 0) {
				/* do not leave space allocated if failed */
				if (allocated) {
					ntfs_free(*outs);
					*outs = (ntfschar *)NULL;
				}
				ret = m;
				goto fail;
			}
			*outpos++ = const_cpu_to_le16(0);
			break;
		}
		if (wc < 0x10000) {
			*outpos++ = cpu_to_le16(wc);
		} else {
			wc -= 0x10000;
			*outpos++ = cpu_to_le16((wc >> 10) + 0xd800);
			*outpos++ = cpu_to_le16((wc & 0x3ff) + 0xdc00);
		}
		t += m;
	}
	ret = --outpos - *outs;
fail:
	return ret;
}

/**
 * ntfs_ucstombs - convert a little endian Unicode string to a multibyte string
 * @ins:	input Unicode string buffer
 * @ins_len:	length of input string in Unicode characters
 * @outs:	on return contains the (allocated) output multibyte string
 * @outs_len:	length of output buffer in bytes
 *
 * Convert the input little endian, 2-byte Unicode string @ins, of length
 * @ins_len into the multibyte string format dictated by the current locale.
 *
 * If *@outs is NULL, the function allocates the string and the caller is
 * responsible for calling ntfs_free(*@outs); when finished with it.
 *
 * On success the function returns the number of bytes written to the output
 * string *@outs (>= 0), not counting the terminating NULL byte. If the output
 * string buffer was allocated, *@outs is set to it.
 *
 * On error, error code is returned.
 * The following error codes can be expected:
 *	EINVAL		Invalid arguments (e.g. @ins or @outs is NULL).
 *	EILSEQ		The input string cannot be represented as a multibyte
 *			sequence according to the current locale.
 *	ENAMETOOLONG	Destination buffer is too small for input string.
 *	ENOMEM		Not enough memory to allocate destination buffer.
 */
int ntfs_ucstombs(const ntfschar *ins, const int ins_len, char **outs,
		int outs_len)
{
	char *mbs;
	int ret;
	int mbs_len;

	if (!ins || !outs)
		return -EINVAL;
	mbs = *outs;
	mbs_len = outs_len;
	if (mbs && !mbs_len)
		return -ENAMETOOLONG;
	if (use_utf8)
		return ntfs_utf16_to_utf8(ins, ins_len, outs, outs_len);
	ret = -EILSEQ;
	return ret;
}

/**
 * ntfs_mbstoucs - convert a multibyte string to a little endian Unicode string
 * @ins:	input multibyte string buffer
 * @outs:	on return contains the (allocated) output Unicode string
 *
 * Convert the input multibyte string @ins, from the current locale into the
 * corresponding little endian, 2-byte Unicode string.
 *
 * The function allocates the string and the caller is responsible for calling
 * ntfs_free(*@outs); when finished with it.
 *
 * On success the function returns the number of Unicode characters written to
 * the output string *@outs (>= 0), not counting the terminating Unicode NULL
 * character.
 *
 * On error, the error code is returned.
 * The following error codes can be expected:
 *	EINVAL		Invalid arguments (e.g. @ins or @outs is NULL).
 *	EILSEQ		The input string cannot be represented as a Unicode
 *			string according to the current locale.
 *	ENAMETOOLONG	Destination buffer is too small for input string.
 *	ENOMEM		Not enough memory to allocate destination buffer.
 */
int ntfs_mbstoucs(const char *ins, ntfschar **outs)
{
	int err = 0;

	if (!ins || !outs)
		return -EINVAL;

	if (use_utf8)
		return ntfs_utf8_to_utf16(ins, outs);

	err = -EILSEQ;
	return err;
}

/*
 *		Turn a UTF8 name uppercase
 *
 *	Returns an allocated uppercase name which has to be freed by caller
 *	or error code if there is an error
 */
char *ntfs_uppercase_mbs(const char *low,
			const ntfschar *upcase, u32 upcase_size)
{
	int size;
	char *upp;
	u32 wc;
	int n;
	const char *s;
	char *t;

	size = strlen(low);
	upp = (char *)ntfs_malloc(3*size + 1);
	if (upp) {
		s = low;
		t = upp;
		do {
			n = utf8_to_unicode(&wc, s);
			if (n > 0) {
				if (wc < upcase_size)
					wc = le16_to_cpu(upcase[wc]);
				if (wc < 0x80)
					*t++ = wc;
				else if (wc < 0x800) {
					*t++ = (0xc0 | ((wc >> 6) & 0x3f));
					*t++ = 0x80 | (wc & 0x3f);
				} else if (wc < 0x10000) {
					*t++ = 0xe0 | (wc >> 12);
					*t++ = 0x80 | ((wc >> 6) & 0x3f);
					*t++ = 0x80 | (wc & 0x3f);
				} else {
					*t++ = 0xf0 | ((wc >> 18) & 7);
					*t++ = 0x80 | ((wc >> 12) & 63);
					*t++ = 0x80 | ((wc >> 6) & 0x3f);
					*t++ = 0x80 | (wc & 0x3f);
				}
			s += n;
			}
		} while (n > 0);
		if (n < 0) {
			ntfs_free(upp);
			upp = ERR_PTR(-EILSEQ);
		} else {
			*t = 0;
		}
	}
	return upp;
}

/**
 * ntfs_upcase_table_build - build the default upcase table for NTFS
 * @uc:		destination buffer where to store the built table
 * @uc_len:	size of destination buffer in bytes
 *
 * ntfs_upcase_table_build() builds the default upcase table for NTFS and
 * stores it in the caller supplied buffer @uc of size @uc_len.
 *
 * Note, @uc_len must be at least 128kiB in size or bad things will happen!
 */
void ntfs_upcase_table_build(ntfschar *uc, u32 uc_len)
{
	struct NEWUPPERCASE {
		unsigned short first;
		unsigned short last;
		short diff;
		unsigned char step;
		unsigned char osmajor;
		unsigned char osminor;
	};

	/*
	 *	This is the table as defined by Windows XP
	 */
	static int uc_run_table[][3] = { /* Start, End, Add */
	{0x0061, 0x007B,  -32}, {0x0451, 0x045D, -80}, {0x1F70, 0x1F72,  74},
	{0x00E0, 0x00F7,  -32}, {0x045E, 0x0460, -80}, {0x1F72, 0x1F76,  86},
	{0x00F8, 0x00FF,  -32}, {0x0561, 0x0587, -48}, {0x1F76, 0x1F78, 100},
	{0x0256, 0x0258, -205}, {0x1F00, 0x1F08,   8}, {0x1F78, 0x1F7A, 128},
	{0x028A, 0x028C, -217}, {0x1F10, 0x1F16,   8}, {0x1F7A, 0x1F7C, 112},
	{0x03AC, 0x03AD,  -38}, {0x1F20, 0x1F28,   8}, {0x1F7C, 0x1F7E, 126},
	{0x03AD, 0x03B0,  -37}, {0x1F30, 0x1F38,   8}, {0x1FB0, 0x1FB2,   8},
	{0x03B1, 0x03C2,  -32}, {0x1F40, 0x1F46,   8}, {0x1FD0, 0x1FD2,   8},
	{0x03C2, 0x03C3,  -31}, {0x1F51, 0x1F52,   8}, {0x1FE0, 0x1FE2,   8},
	{0x03C3, 0x03CC,  -32}, {0x1F53, 0x1F54,   8}, {0x1FE5, 0x1FE6,   7},
	{0x03CC, 0x03CD,  -64}, {0x1F55, 0x1F56,   8}, {0x2170, 0x2180, -16},
	{0x03CD, 0x03CF,  -63}, {0x1F57, 0x1F58,   8}, {0x24D0, 0x24EA, -26},
	{0x0430, 0x0450,  -32}, {0x1F60, 0x1F68,   8}, {0xFF41, 0xFF5B, -32},
	{0}
	};
	static int uc_dup_table[][2] = { /* Start, End */
	{0x0100, 0x012F}, {0x01A0, 0x01A6}, {0x03E2, 0x03EF}, {0x04CB, 0x04CC},
	{0x0132, 0x0137}, {0x01B3, 0x01B7}, {0x0460, 0x0481}, {0x04D0, 0x04EB},
	{0x0139, 0x0149}, {0x01CD, 0x01DD}, {0x0490, 0x04BF}, {0x04EE, 0x04F5},
	{0x014A, 0x0178}, {0x01DE, 0x01EF}, {0x04BF, 0x04BF}, {0x04F8, 0x04F9},
	{0x0179, 0x017E}, {0x01F4, 0x01F5}, {0x04C1, 0x04C4}, {0x1E00, 0x1E95},
	{0x018B, 0x018B}, {0x01FA, 0x0218}, {0x04C7, 0x04C8}, {0x1EA0, 0x1EF9},
	{0}
	};
	static int uc_byte_table[][2] = { /* Offset, Value */
	{0x00FF, 0x0178}, {0x01AD, 0x01AC}, {0x01F3, 0x01F1}, {0x0269, 0x0196},
	{0x0183, 0x0182}, {0x01B0, 0x01AF}, {0x0253, 0x0181}, {0x026F, 0x019C},
	{0x0185, 0x0184}, {0x01B9, 0x01B8}, {0x0254, 0x0186}, {0x0272, 0x019D},
	{0x0188, 0x0187}, {0x01BD, 0x01BC}, {0x0259, 0x018F}, {0x0275, 0x019F},
	{0x018C, 0x018B}, {0x01C6, 0x01C4}, {0x025B, 0x0190}, {0x0283, 0x01A9},
	{0x0192, 0x0191}, {0x01C9, 0x01C7}, {0x0260, 0x0193}, {0x0288, 0x01AE},
	{0x0199, 0x0198}, {0x01CC, 0x01CA}, {0x0263, 0x0194}, {0x0292, 0x01B7},
	{0x01A8, 0x01A7}, {0x01DD, 0x018E}, {0x0268, 0x0197},
	{0}
	};

/*
 *		Changes which were applied to later Windows versions
 *
 *   md5 for $UpCase from Winxp : 6fa3db2468275286210751e869d36373
 *                        Vista : 2f03b5a69d486ff3864cecbd07f24440
 *                        Win8 :  7ff498a44e45e77374cc7c962b1b92f2
 */
	static const struct NEWUPPERCASE newuppercase[] = {
						/* from Windows 6.0 (Vista) */
		{ 0x37b, 0x37d, 0x82, 1, 6, 0 },
		{ 0x1f80, 0x1f87, 0x8, 1, 6, 0 },
		{ 0x1f90, 0x1f97, 0x8, 1, 6, 0 },
		{ 0x1fa0, 0x1fa7, 0x8, 1, 6, 0 },
		{ 0x2c30, 0x2c5e, -0x30, 1, 6, 0 },
		{ 0x2d00, 0x2d25, -0x1c60, 1, 6, 0 },
		{ 0x2c68, 0x2c6c, -0x1, 2, 6, 0 },
		{ 0x219, 0x21f, -0x1, 2, 6, 0 },
		{ 0x223, 0x233, -0x1, 2, 6, 0 },
		{ 0x247, 0x24f, -0x1, 2, 6, 0 },
		{ 0x3d9, 0x3e1, -0x1, 2, 6, 0 },
		{ 0x48b, 0x48f, -0x1, 2, 6, 0 },
		{ 0x4fb, 0x513, -0x1, 2, 6, 0 },
		{ 0x2c81, 0x2ce3, -0x1, 2, 6, 0 },
		{ 0x3f8, 0x3fb, -0x1, 3, 6, 0 },
		{ 0x4c6, 0x4ce, -0x1, 4, 6, 0 },
		{ 0x23c, 0x242, -0x1, 6, 6, 0 },
		{ 0x4ed, 0x4f7, -0x1, 10, 6, 0 },
		{ 0x450, 0x45d, -0x50, 13, 6, 0 },
		{ 0x2c61, 0x2c76, -0x1, 21, 6, 0 },
		{ 0x1fcc, 0x1ffc, -0x9, 48, 6, 0 },
		{ 0x180, 0x180, 0xc3, 1, 6, 0 },
		{ 0x195, 0x195, 0x61, 1, 6, 0 },
		{ 0x19a, 0x19a, 0xa3, 1, 6, 0 },
		{ 0x19e, 0x19e, 0x82, 1, 6, 0 },
		{ 0x1bf, 0x1bf, 0x38, 1, 6, 0 },
		{ 0x1f9, 0x1f9, -0x1, 1, 6, 0 },
		{ 0x23a, 0x23a, 0x2a2b, 1, 6, 0 },
		{ 0x23e, 0x23e, 0x2a28, 1, 6, 0 },
		{ 0x26b, 0x26b, 0x29f7, 1, 6, 0 },
		{ 0x27d, 0x27d, 0x29e7, 1, 6, 0 },
		{ 0x280, 0x280, -0xda, 1, 6, 0 },
		{ 0x289, 0x289, -0x45, 1, 6, 0 },
		{ 0x28c, 0x28c, -0x47, 1, 6, 0 },
		{ 0x3f2, 0x3f2, 0x7, 1, 6, 0 },
		{ 0x4cf, 0x4cf, -0xf, 1, 6, 0 },
		{ 0x1d7d, 0x1d7d, 0xee6, 1, 6, 0 },
		{ 0x1fb3, 0x1fb3, 0x9, 1, 6, 0 },
		{ 0x214e, 0x214e, -0x1c, 1, 6, 0 },
		{ 0x2184, 0x2184, -0x1, 1, 6, 0 },
						/* from Windows 6.1 (Win7) */
		{ 0x23a, 0x23e,  0x0, 4, 6, 1 },
		{ 0x250, 0x250,  0x2a1f, 2, 6, 1 },
		{ 0x251, 0x251,  0x2a1c, 2, 6, 1 },
		{ 0x271, 0x271,  0x29fd, 2, 6, 1 },
		{ 0x371, 0x373, -0x1, 2, 6, 1 },
		{ 0x377, 0x377, -0x1, 2, 6, 1 },
		{ 0x3c2, 0x3c2,  0x0, 2, 6, 1 },
		{ 0x3d7, 0x3d7, -0x8, 2, 6, 1 },
		{ 0x515, 0x523, -0x1, 2, 6, 1 },
			/* below, -0x75fc stands for 0x8a04 and truncation */
		{ 0x1d79, 0x1d79, -0x75fc, 2, 6, 1 },
		{ 0x1efb, 0x1eff, -0x1, 2, 6, 1 },
		{ 0x1fc3, 0x1ff3,  0x9, 48, 6, 1 },
		{ 0x1fcc, 0x1ffc,  0x0, 48, 6, 1 },
		{ 0x2c65, 0x2c65, -0x2a2b, 2, 6, 1 },
		{ 0x2c66, 0x2c66, -0x2a28, 2, 6, 1 },
		{ 0x2c73, 0x2c73, -0x1, 2, 6, 1 },
		{ 0xa641, 0xa65f, -0x1, 2, 6, 1 },
		{ 0xa663, 0xa66d, -0x1, 2, 6, 1 },
		{ 0xa681, 0xa697, -0x1, 2, 6, 1 },
		{ 0xa723, 0xa72f, -0x1, 2, 6, 1 },
		{ 0xa733, 0xa76f, -0x1, 2, 6, 1 },
		{ 0xa77a, 0xa77c, -0x1, 2, 6, 1 },
		{ 0xa77f, 0xa787, -0x1, 2, 6, 1 },
		{ 0xa78c, 0xa78c, -0x1, 2, 6, 1 },
							/* end mark */
		{ 0 }
	};

	int i, r;
	int k, off;
	const struct NEWUPPERCASE *puc;

	memset((char *)uc, 0, uc_len);
	uc_len >>= 1;
	if (uc_len > 65536)
		uc_len = 65536;
	for (i = 0; (u32)i < uc_len; i++)
		uc[i] = cpu_to_le16(i);
	for (r = 0; uc_run_table[r][0]; r++) {
		off = uc_run_table[r][2];
		for (i = uc_run_table[r][0]; i < uc_run_table[r][1]; i++)
			uc[i] = cpu_to_le16(i + off);
	}
	for (r = 0; uc_dup_table[r][0]; r++)
		for (i = uc_dup_table[r][0]; i < uc_dup_table[r][1]; i += 2)
			uc[i + 1] = cpu_to_le16(i);
	for (r = 0; uc_byte_table[r][0]; r++) {
		k = uc_byte_table[r][1];
		uc[uc_byte_table[r][0]] = cpu_to_le16(k);
	}
	for (r = 0; newuppercase[r].first; r++) {
		puc = &newuppercase[r];
		if ((puc->osmajor < UPCASE_MAJOR)
		  || ((puc->osmajor == UPCASE_MAJOR)
		     && (puc->osminor <= UPCASE_MINOR))) {
			off = puc->diff;
			for (i = puc->first; i <= puc->last; i += puc->step)
				uc[i] = cpu_to_le16(i + off);
		}
	}
}

/*
 *		Allocate and build the default upcase table
 *
 *	Returns the number of entries
 *		0 if failed
 */

#define UPCASE_LEN 65536 /* default number of entries in upcase */

u32 ntfs_upcase_build_default(ntfschar **upcase)
{
	u32 upcase_len = 0;

	*upcase = (ntfschar *)ntfs_malloc(UPCASE_LEN*2);
	if (*upcase) {
		ntfs_upcase_table_build(*upcase, UPCASE_LEN*2);
		upcase_len = UPCASE_LEN;
	}
	return upcase_len;
}
