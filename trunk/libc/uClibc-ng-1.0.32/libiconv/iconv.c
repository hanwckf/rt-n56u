/* 
 * Copyright © 2018 Waldemar Brodkorb <wbx@uclibc-ng.org>
 * Simplified port of iconv.c from musl C library including 
 * parts of libiconv-tiny.
 */

/* Copyright © 2005-2018 Rich Felker, et al.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <iconv.h>
#include <errno.h>
#include <wchar.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <limits.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>

#define UTF_32BE    0300
#define UTF_16LE    0301
#define UTF_16BE    0302
#define UTF_32LE    0303
#define UCS2BE      0304
#define UCS2LE      0305
#define WCHAR_T     0306
#define US_ASCII    0307
#define UTF_8       0310
#define UTF_16      0312
#define UTF_32      0313
#define UCS2        0314
#define EUC_JP      0320
#define SHIFT_JIS   0321
#define ISO2022_JP  0322
#define GB18030     0330
#define GBK         0331
#define GB2312      0332
#define BIG5        0340
#define EUC_KR      0350

/* Definitions of charmaps. Each charmap consists of:
 * 1. Empty-string-terminated list of null-terminated aliases.
 * 2. Special type code or number of elided quads of entries.
 * 3. Character table (size determined by field 2), consisting
 *    of 5 bytes for every 4 characters, interpreted as 10-bit
 *    indices into the legacy_chars table. */

static const unsigned char charmaps[] =
"utf8\0char\0\0\310"
"wchart\0\0\306"
"ucs2be\0\0\304"
"ucs2le\0\0\305"
"utf16be\0\0\302"
"utf16le\0\0\301"
"ucs4be\0utf32be\0\0\300"
"ucs4le\0utf32le\0\0\303"
"ascii\0usascii\0iso646\0iso646us\0\0\307"
"utf16\0\0\312"
"ucs4\0utf32\0\0\313"
"ucs2\0\0\314"
"eucjp\0\0\320"
"shiftjis\0sjis\0\0\321"
"iso2022jp\0\0\322"
"gb18030\0\0\330"
"gbk\0\0\331"
"gb2312\0\0\332"
"big5\0bigfive\0cp950\0big5hkscs\0\0\340"
"euckr\0ksc5601\0ksx1001\0cp949\0\0\350"
#include "codepages.h"
;

/* Table of characters that appear in legacy 8-bit codepages,
 * limited to 1024 slots (10 bit indices). The first 256 entries
 * are elided since those characters are obviously all included. */
static const unsigned short legacy_chars[] = {
#include "legacychars.h"
};

static const unsigned short jis0208[84][94] = {
#include "jis0208.h"
};

static const unsigned short rev_jis[] = {
#include "revjis.h"
};

static int fuzzycmp(const unsigned char *a, const unsigned char *b)
{
	for (; *a && *b; a++, b++) {
		while (*a && (*a|32U)-'a'>26 && *a-'0'>10U) a++;
		if ((*a|32U) != *b) return 1;
	}
	return *a != *b;
}

static size_t find_charmap(const void *name)
{
	const unsigned char *s;
	if (!*(char *)name) name=charmaps; /* "utf8" */
	for (s=charmaps; *s; ) {
		if (!fuzzycmp(name, s)) {
			for (; *s; s+=strlen((void *)s)+1);
			return s+1-charmaps;
		}
		s += strlen((void *)s)+1;
		if (!*s) {
			if (s[1] > 0200) s+=2;
			else s+=2+(64U-s[1])*5;
		}
	}
	return -1;
}

struct stateful_cd {
	iconv_t base_cd;
	unsigned state;
};

static iconv_t combine_to_from(size_t t, size_t f)
{
	return (void *)(f<<16 | t<<1 | 1);
}

static size_t extract_from(iconv_t cd)
{
	return (size_t)cd >> 16;
}

static size_t extract_to(iconv_t cd)
{
	return (size_t)cd >> 1 & 0x7fff;
}

iconv_t iconv_open(const char *to, const char *from)
{
	size_t f, t;
	struct stateful_cd *scd;

	if ((t = find_charmap(to))==-1
	 || (f = find_charmap(from))==-1
	 || (charmaps[t] >= 0330)) {
		errno = EINVAL;
		return (iconv_t)-1;
	}
	iconv_t cd = combine_to_from(t, f);

	switch (charmaps[f]) {
	case UTF_16:
	case UTF_32:
	case UCS2:
	case ISO2022_JP:
		scd = malloc(sizeof *scd);
		if (!scd) return (iconv_t)-1;
		scd->base_cd = cd;
		scd->state = 0;
		cd = (iconv_t)scd;
	}

	return cd;
}

static unsigned get_16(const unsigned char *s, int e)
{
	e &= 1;
	return s[e]<<8 | s[1-e];
}

static void put_16(unsigned char *s, unsigned c, int e)
{
	e &= 1;
	s[e] = c>>8;
	s[1-e] = c;
}

static unsigned get_32(const unsigned char *s, int e)
{
	e &= 3;
	return s[e]+0U<<24 | s[e^1]<<16 | s[e^2]<<8 | s[e^3];
}

static void put_32(unsigned char *s, unsigned c, int e)
{
	e &= 3;
	s[e^0] = c>>24;
	s[e^1] = c>>16;
	s[e^2] = c>>8;
	s[e^3] = c;
}


static inline int utf8enc_wchar(char *outb, wchar_t c)
{
	if (c <= 0x7F) {
		*outb = c;
		return 1;
	}
	else if (c <= 0x7FF) {
		*outb++ = ((c >>  6) & 0x1F) | 0xC0;
		*outb++ = ( c        & 0x3F) | 0x80;
		return 2;
	}
	else if (c <= 0xFFFF) {
		*outb++ = ((c >> 12) & 0x0F) | 0xE0;
		*outb++ = ((c >>  6) & 0x3F) | 0x80;
		*outb++ = ( c        & 0x3F) | 0x80;
		return 3;
	}
	else if (c <= 0x10FFFF) {
		*outb++ = ((c >> 18) & 0x07) | 0xF0;
		*outb++ = ((c >> 12) & 0x3F) | 0x80;
		*outb++ = ((c >>  6) & 0x3F) | 0x80;
		*outb++ = ( c        & 0x3F) | 0x80;
		return 4;
	}
	else {
		*outb++ = '?';
		return 1;
	}
}

static inline int utf8seq_is_overlong(char *s, int n)
{
	switch (n)
	{
	case 2:
		/* 1100000x (10xxxxxx) */
		return (((*s >> 1) == 0x60) &&
				((*(s+1) >> 6) == 0x02));

	case 3:
		/* 11100000 100xxxxx (10xxxxxx) */
		return ((*s == 0xE0) &&
				((*(s+1) >> 5) == 0x04) &&
				((*(s+2) >> 6) == 0x02));

	case 4:
		/* 11110000 1000xxxx (10xxxxxx 10xxxxxx) */
		return ((*s == 0xF0) &&
				((*(s+1) >> 4) == 0x08) &&
				((*(s+2) >> 6) == 0x02) &&
				((*(s+3) >> 6) == 0x02));
	}

	return 0;
}

static inline int utf8seq_is_surrogate(char *s, int n)
{
	return ((n == 3) && (*s == 0xED) && (*(s+1) >= 0xA0) && (*(s+1) <= 0xBF));
}

static inline int utf8seq_is_illegal(char *s, int n)
{
	return ((n == 3) && (*s == 0xEF) && (*(s+1) == 0xBF) &&
	        (*(s+2) >= 0xBE) && (*(s+2) <= 0xBF));
}

static inline int utf8dec_wchar(wchar_t *c, unsigned char *in, size_t inb)
{
	int i;
	int n = -1;

	/* trivial char */
	if (*in <= 0x7F) {
		*c = *in;
		return 1;
	}

	/* find utf8 sequence length */
	if      ((*in & 0xE0) == 0xC0) n = 2;
	else if ((*in & 0xF0) == 0xE0) n = 3;
	else if ((*in & 0xF8) == 0xF0) n = 4;
	else if ((*in & 0xFC) == 0xF8) n = 5;
	else if ((*in & 0xFE) == 0xFC) n = 6;

	/* starved? */
	if (n > inb)
		return -2;

	/* decode ... */
	if (n > 1 && n < 5) {
		/* reject invalid sequences */
		if (utf8seq_is_overlong(in, n) ||
			utf8seq_is_surrogate(in, n) ||
			utf8seq_is_illegal(in, n))
			return -1;

		/* decode ... */
		*c = (char)(*in++ & (0x7F >> n));

		for (i = 1; i < n; i++) {
			/* illegal continuation byte */
			if (*in < 0x80 || *in > 0xBF)
				return -1;

			*c = (*c << 6) | (*in++ & 0x3F);
		}

		return n;
	}

	/* unmapped sequence (> 4) */
	return -1;
}

static unsigned legacy_map(const unsigned char *map, unsigned c)
{
	if (c < 4*map[-1]) return c;
	unsigned x = c - 4*map[-1];
	x = map[x*5/4]>>2*x%8 | map[x*5/4+1]<<8-2*x%8 & 1023;
	return x < 256 ? x : legacy_chars[x-256];
}

static unsigned uni_to_jis(unsigned c)
{
	unsigned nel = sizeof rev_jis / sizeof *rev_jis;
	unsigned d, j, i, b = 0;
	for (;;) {
		i = nel/2;
		j = rev_jis[b+i];
		d = jis0208[j/256][j%256];
		if (d==c) return j + 0x2121;
		else if (nel == 1) return 0;
		else if (c < d)
			nel /= 2;
		else {
			b += i;
			nel -= nel/2;
		}
	}
}

size_t iconv(iconv_t cd, char **restrict in, size_t *restrict inb, char **restrict out, size_t *restrict outb)
{
	size_t x=0;
	struct stateful_cd *scd=0;
	if (!((size_t)cd & 1)) {
		scd = (void *)cd;
		cd = scd->base_cd;
	}
	unsigned to = extract_to(cd);
	unsigned from = extract_from(cd);
	const unsigned char *map = charmaps+from+1;
	const unsigned char *tomap = charmaps+to+1;
	char tmp[MB_LEN_MAX];
	unsigned c, d;
	size_t k, l;
	int err;
	unsigned char type = map[-1];
	unsigned char totype = tomap[-1];

	if (!in || !*in || !*inb) return 0;

	for (; *inb; *in+=l, *inb-=l) {
		c = *(unsigned char *)*in;
		l = 1;

		switch (type) {
		case UTF_8:
			if (c < 128) break;
			l = utf8dec_wchar(&c, *in, *inb);
			if (!l) l++;
			else if (l == (size_t)-1) goto ilseq;
			else if (l == (size_t)-2) goto starved;
			break;
		case US_ASCII:
			if (c >= 128) goto ilseq;
			break;
		case WCHAR_T:
			l = sizeof(wchar_t);
			if (*inb < l) goto starved;
			c = *(wchar_t *)*in;
			if (0) {
		case UTF_32BE:
		case UTF_32LE:
			l = 4;
			if (*inb < 4) goto starved;
			c = get_32((void *)*in, type);
			}
			if (c-0xd800u < 0x800u || c >= 0x110000u) goto ilseq;
			break;
		case UCS2BE:
		case UCS2LE:
		case UTF_16BE:
		case UTF_16LE:
			l = 2;
			if (*inb < 2) goto starved;
			c = get_16((void *)*in, type);
			if ((unsigned)(c-0xdc00) < 0x400) goto ilseq;
			if ((unsigned)(c-0xd800) < 0x400) {
				if (type-UCS2BE < 2U) goto ilseq;
				l = 4;
				if (*inb < 4) goto starved;
				d = get_16((void *)(*in + 2), type);
				if ((unsigned)(d-0xdc00) >= 0x400) goto ilseq;
				c = ((c-0xd7c0)<<10) + (d-0xdc00);
			}
			break;
		case UCS2:
		case UTF_16:
			l = 0;
			if (!scd->state) {
				if (*inb < 2) goto starved;
				c = get_16((void *)*in, 0);
				scd->state = type==UCS2
					? c==0xfffe ? UCS2LE : UCS2BE
					: c==0xfffe ? UTF_16LE : UTF_16BE;
				if (c == 0xfffe || c == 0xfeff)
					l = 2;
			}
			type = scd->state;
			continue;
		case UTF_32:
			l = 0;
			if (!scd->state) {
				if (*inb < 4) goto starved;
				c = get_32((void *)*in, 0);
				scd->state = c==0xfffe0000 ? UTF_32LE : UTF_32BE;
				if (c == 0xfffe0000 || c == 0xfeff)
					l = 4;
			}
			type = scd->state;
			continue;
		case SHIFT_JIS:
			if (c < 128) break;
			if (c-0xa1 <= 0xdf-0xa1) {
				c += 0xff61-0xa1;
				break;
			}
			l = 2;
			if (*inb < 2) goto starved;
			d = *((unsigned char *)*in + 1);
			if (c-129 <= 159-129) c -= 129;
			else if (c-224 <= 239-224) c -= 193;
			else goto ilseq;
			c *= 2;
			if (d-64 <= 158-64) {
				if (d==127) goto ilseq;
				if (d>127) d--;
				d -= 64;
			} else if (d-159 <= 252-159) {
				c++;
				d -= 159;
			}
			c = jis0208[c][d];
			if (!c) goto ilseq;
			break;
		case EUC_JP:
			if (c < 128) break;
			l = 2;
			if (*inb < 2) goto starved;
			d = *((unsigned char *)*in + 1);
			if (c==0x8e) {
				c = d;
				if (c-0xa1 > 0xdf-0xa1) goto ilseq;
				c += 0xff61 - 0xa1;
				break;
			}
			c -= 0xa1;
			d -= 0xa1;
			if (c >= 84 || d >= 94) goto ilseq;
			c = jis0208[c][d];
			if (!c) goto ilseq;
			break;
		case ISO2022_JP:
			if (c >= 128) goto ilseq;
			if (c == '\033') {
				l = 3;
				if (*inb < 3) goto starved;
				c = *((unsigned char *)*in + 1);
				d = *((unsigned char *)*in + 2);
				if (c != '(' && c != '$') goto ilseq;
				switch (128*(c=='$') + d) {
				case 'B': scd->state=0; continue;
				case 'J': scd->state=1; continue;
				case 'I': scd->state=4; continue;
				case 128+'@': scd->state=2; continue;
				case 128+'B': scd->state=3; continue;
				}
				goto ilseq;
			}
			switch (scd->state) {
			case 1:
				if (c=='\\') c = 0xa5;
				if (c=='~') c = 0x203e;
				break;
			case 2:
			case 3:
				l = 2;
				if (*inb < 2) goto starved;
				d = *((unsigned char *)*in + 1);
				c -= 0x21;
				d -= 0x21;
				if (c >= 84 || d >= 94) goto ilseq;
				c = jis0208[c][d];
				if (!c) goto ilseq;
				break;
			case 4:
				if (c-0x60 < 0x1f) goto ilseq;
				if (c-0x21 < 0x5e) c += 0xff61-0x21;
				break;
			}
			break;
		default:
			if (!c) break;
			c = legacy_map(map, c);
			if (!c) goto ilseq;
		}

		switch (totype) {
		case WCHAR_T:
			if (*outb < sizeof(wchar_t)) goto toobig;
			*(wchar_t *)*out = c;
			*out += sizeof(wchar_t);
			*outb -= sizeof(wchar_t);
			break;
		case UTF_8:
			if (*outb < 4) {
				k = utf8enc_wchar(tmp, c);
				if (*outb < k) goto toobig;
				memcpy(*out, tmp, k);
			} else k = utf8enc_wchar(*out, c);
			*out += k;
			*outb -= k;
			break;
		case US_ASCII:
			if (c > 0x7f) subst: x++, c='*';
		default:
			if (*outb < 1) goto toobig;
			if (c<256 && c==legacy_map(tomap, c)) {
			revout:
				*(*out)++ = c;
				*outb -= 1;
				break;
			}
			d = c;
			for (c=4*totype; c<256; c++) {
				if (d == legacy_map(tomap, c)) {
					goto revout;
				}
			}
			goto subst;
		case SHIFT_JIS:
			if (c < 128) goto revout;
			if (c == 0xa5) {
				x++;
				c = '\\';
				goto revout;
			}
			if (c == 0x203e) {
				x++;
				c = '~';
				goto revout;
			}
			if (c-0xff61 <= 0xdf-0xa1) {
				c += 0xa1 - 0xff61;
				goto revout;
			}
			c = uni_to_jis(c);
			if (!c) goto subst;
			if (*outb < 2) goto toobig;
			d = c%256;
			c = c/256;
			*(*out)++ = (c+1)/2 + (c<95 ? 112 : 176);
			*(*out)++ = c%2 ? d + 31 + d/96 : d + 126;
			*outb -= 2;
			break;
		case EUC_JP:
			if (c < 128) goto revout;
			if (c-0xff61 <= 0xdf-0xa1) {
				c += 0x0e00 + 0x21 - 0xff61;
			} else {
				c = uni_to_jis(c);
			}
			if (!c) goto subst;
			if (*outb < 2) goto toobig;
			*(*out)++ = c/256 + 0x80;
			*(*out)++ = c%256 + 0x80;
			*outb -= 2;
			break;
		case ISO2022_JP:
			if (c < 128) goto revout;
			if (c-0xff61 <= 0xdf-0xa1 || c==0xa5 || c==0x203e) {
				if (*outb < 7) goto toobig;
				*(*out)++ = '\033';
				*(*out)++ = '(';
				if (c==0xa5) {
					*(*out)++ = 'J';
					*(*out)++ = '\\';
				} else if (c==0x203e) {
					*(*out)++ = 'J';
					*(*out)++ = '~';
				} else {
					*(*out)++ = 'I';
					*(*out)++ = c-0xff61+0x21;
				}
				*(*out)++ = '\033';
				*(*out)++ = '(';
				*(*out)++ = 'B';
				*outb -= 7;
				break;
			}
			c = uni_to_jis(c);
			if (!c) goto subst;
			if (*outb < 8) goto toobig;
			*(*out)++ = '\033';
			*(*out)++ = '$';
			*(*out)++ = 'B';
			*(*out)++ = c/256;
			*(*out)++ = c%256;
			*(*out)++ = '\033';
			*(*out)++ = '(';
			*(*out)++ = 'B';
			*outb -= 8;
			break;
		case UCS2:
			totype = UCS2BE;
		case UCS2BE:
		case UCS2LE:
		case UTF_16:
		case UTF_16BE:
		case UTF_16LE:
			if (c < 0x10000 || totype-UCS2BE < 2U) {
				if (c >= 0x10000) c = 0xFFFD;
				if (*outb < 2) goto toobig;
				put_16((void *)*out, c, totype);
				*out += 2;
				*outb -= 2;
				break;
			}
			if (*outb < 4) goto toobig;
			c -= 0x10000;
			put_16((void *)*out, (c>>10)|0xd800, totype);
			put_16((void *)(*out + 2), (c&0x3ff)|0xdc00, totype);
			*out += 4;
			*outb -= 4;
			break;
		case UTF_32BE:
		case UTF_32LE:
			if (*outb < 4) goto toobig;
			put_32((void *)*out, c, totype);
			*out += 4;
			*outb -= 4;
			break;
		}
	}
	return x;
ilseq:
	err = EILSEQ;
	x = -1;
	goto end;
toobig:
	err = E2BIG;
	x = -1;
	goto end;
starved:
	err = EINVAL;
	x = -1;
end:
	errno = err;
	return x;
}

int iconv_close(iconv_t cd)
{
	return 0;
}

