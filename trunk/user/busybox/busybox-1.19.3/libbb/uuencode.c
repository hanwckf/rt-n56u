/* vi: set sw=4 ts=4: */
/*
 * Copyright 2003, Glenn McGrath
 * Copyright 2006, Rob Landley <rob@landley.net>
 * Copyright 2010, Denys Vlasenko
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */

#include "libbb.h"

/* Conversion table.  for base 64 */
const char bb_uuenc_tbl_base64[65 + 2] ALIGN1 = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/',
	'=' /* termination character */,
	'\n', '\0' /* needed for uudecode.c */
};

const char bb_uuenc_tbl_std[65] ALIGN1 = {
	'`', '!', '"', '#', '$', '%', '&', '\'',
	'(', ')', '*', '+', ',', '-', '.', '/',
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', ':', ';', '<', '=', '>', '?',
	'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
	'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
	'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
	'`' /* termination character */
};

/*
 * Encode bytes at S of length LENGTH to uuencode or base64 format and place it
 * to STORE.  STORE will be 0-terminated, and must point to a writable
 * buffer of at least 1+BASE64_LENGTH(length) bytes.
 * where BASE64_LENGTH(len) = (4 * ((LENGTH + 2) / 3))
 */
void FAST_FUNC bb_uuencode(char *p, const void *src, int length, const char *tbl)
{
	const unsigned char *s = src;

	/* Transform the 3x8 bits to 4x6 bits */
	while (length > 0) {
		unsigned s1, s2;

		/* Are s[1], s[2] valid or should be assumed 0? */
		s1 = s2 = 0;
		length -= 3; /* can be >=0, -1, -2 */
		if (length >= -1) {
			s1 = s[1];
			if (length >= 0)
				s2 = s[2];
		}
		*p++ = tbl[s[0] >> 2];
		*p++ = tbl[((s[0] & 3) << 4) + (s1 >> 4)];
		*p++ = tbl[((s1 & 0xf) << 2) + (s2 >> 6)];
		*p++ = tbl[s2 & 0x3f];
		s += 3;
	}
	/* Zero-terminate */
	*p = '\0';
	/* If length is -2 or -1, pad last char or two */
	while (length) {
		*--p = tbl[64];
		length++;
	}
}

/*
 * Decode base64 encoded stream.
 * Can stop on EOF, specified char, or on uuencode-style "====" line:
 * flags argument controls it.
 */
void FAST_FUNC read_base64(FILE *src_stream, FILE *dst_stream, int flags)
{
/* Note that EOF _can_ be passed as exit_char too */
#define exit_char    ((int)(signed char)flags)
#define uu_style_end (flags & BASE64_FLAG_UU_STOP)

	int term_count = 0;

	while (1) {
		unsigned char translated[4];
		int count = 0;

		/* Process one group of 4 chars */
		while (count < 4) {
			char *table_ptr;
			int ch;

			/* Get next _valid_ character.
			 * bb_uuenc_tbl_base64[] contains this string:
			 *  0         1         2         3         4         5         6
			 *  012345678901234567890123456789012345678901234567890123456789012345
			 * "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=\n"
			 */
			do {
				ch = fgetc(src_stream);
				if (ch == exit_char && count == 0)
					return;
				if (ch == EOF)
					bb_error_msg_and_die("truncated base64 input");
				table_ptr = strchr(bb_uuenc_tbl_base64, ch);
//TODO: add BASE64_FLAG_foo to die on bad char?
//Note that then we may need to still allow '\r' (for mail processing)
			} while (!table_ptr);

			/* Convert encoded character to decimal */
			ch = table_ptr - bb_uuenc_tbl_base64;

			if (ch == 65 /* '\n' */) {
				/* Terminating "====" line? */
				if (uu_style_end && term_count == 4)
					return; /* yes */
				term_count = 0;
				continue;
			}
			/* ch is 64 if char was '=', otherwise 0..63 */
			translated[count] = ch & 63; /* 64 -> 0 */
			if (ch == 64) {
				term_count++;
				break;
			}
			count++;
			term_count = 0;
		}

		/* Merge 6 bit chars to 8 bit.
		 * count can be < 4 when we decode the tail:
		 * "eQ==" -> "y", not "y NUL NUL"
		 */
		if (count > 1)
			fputc(translated[0] << 2 | translated[1] >> 4, dst_stream);
		if (count > 2)
			fputc(translated[1] << 4 | translated[2] >> 2, dst_stream);
		if (count > 3)
			fputc(translated[2] << 6 | translated[3], dst_stream);
	} /* while (1) */
}
