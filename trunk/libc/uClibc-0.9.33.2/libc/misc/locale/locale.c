/*  Copyright (C) 2002     Manuel Novoa III
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* Nov. 1, 2002
 * Reworked setlocale() return values and locale arg processing to
 *   be more like glibc.  Applications expecting to be able to
 *   query locale settings should now work... at the cost of almost
 *   doubling the size of the setlocale object code.
 * Fixed a bug in the internal fixed-size-string locale specifier code.
 *
 * Dec 20, 2002
 * Added in collation support and updated stub nl_langinfo.
 *
 * Aug 1, 2003
 * Added glibc-like extended locale stuff (newlocale, duplocale, etc).
 *
 * Aug 18, 2003
 * Bug in duplocale... collation data wasn't copied.
 * Bug in newlocale... translate 1<<LC_ALL to LC_ALL_MASK.
 * Bug in _wchar_utf8sntowcs... fix cut-n-paste error.
 *
 * Aug 31, 2003
 * Hack around bg_BG bug; grouping specified but no thousands separator.
 * Also, disable the locale link_warnings for now, as they generate a
 * lot of noise when using libstd++.
 */


/*  TODO:
 *  Implement the shared mmap code so non-mmu platforms can use this.
 *  Add some basic collate functionality similar to what the previous
 *    locale support had (8-bit codesets only).
 */

#define __CTYPE_HAS_8_BIT_LOCALES 1

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>

#ifdef __UCLIBC_MJN3_ONLY__
#ifdef L_setlocale
#warning TODO: Make the link_warning()s a config option?
#endif
#endif
#undef link_warning
#define link_warning(A,B)

#undef __LOCALE_C_ONLY
#ifndef __UCLIBC_HAS_LOCALE__
#define __LOCALE_C_ONLY
#endif /* __UCLIBC_HAS_LOCALE__ */


#ifdef __LOCALE_C_ONLY

#include <locale.h>

#else  /* __LOCALE_C_ONLY */

#ifdef __UCLIBC_MJN3_ONLY__
#ifdef L_setlocale
#warning TODO: Fix the __CTYPE_HAS_8_BIT_LOCALES define at the top of the file.
#warning TODO: Fix __WCHAR_ENABLED.
#endif
#endif

/* Need to include this before locale.h and xlocale.h! */
#include <bits/uClibc_locale.h>

#undef CODESET_LIST
#define CODESET_LIST			(__locale_mmap->codeset_list)

#ifdef __UCLIBC_HAS_XLOCALE__
#include <xlocale.h>
#include <locale.h>
#else /* __UCLIBC_HAS_XLOCALE__ */
/* We need this internally... */
#define __UCLIBC_HAS_XLOCALE__ 1
#include <xlocale.h>
#include <locale.h>
#undef __UCLIBC_HAS_XLOCALE__
#endif /* __UCLIBC_HAS_XLOCALE__ */

#include <wchar.h>

#define LOCALE_NAMES			(__locale_mmap->locale_names5)
#define LOCALES					(__locale_mmap->locales)
#define LOCALE_AT_MODIFIERS		(__locale_mmap->locale_at_modifiers)
#define CATEGORY_NAMES			(__locale_mmap->lc_names)

#ifdef __UCLIBC_MJN3_ONLY__
#warning REMINDER: redo the MAX_LOCALE_STR stuff...
#endif
#define MAX_LOCALE_STR			256 /* TODO: Only sufficient for current case. */
#define MAX_LOCALE_CATEGORY_STR	32 /* TODO: Only sufficient for current case. */
/* Note: Best if MAX_LOCALE_CATEGORY_STR is a power of 2. */

extern int _locale_set_l(const unsigned char *p, __locale_t base) attribute_hidden;
extern void _locale_init_l(__locale_t base) attribute_hidden;

#endif /* __LOCALE_C_ONLY */

#undef LOCALE_STRING_SIZE
#define LOCALE_SELECTOR_SIZE (2 * __LC_ALL + 2)

#ifdef __UCLIBC_MJN3_ONLY__
#ifdef L_setlocale
#warning TODO: Create a C locale selector string.
#endif
#endif
#define C_LOCALE_SELECTOR "\x23\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"


#include <langinfo.h>
#include <nl_types.h>

/**********************************************************************/
#ifdef L_setlocale

#ifdef __LOCALE_C_ONLY

link_warning(setlocale,"REMINDER: The 'setlocale' function supports only C|POSIX locales.")

static const char C_string[] = "C";

char *setlocale(int category, register const char *locale)
{
	return ( (((unsigned int)(category)) <= LC_ALL)
			 && ( (!locale)		/* Request for locale category string. */
				  || (!*locale)	/* Implementation-defined default is C. */
				  || ((*locale == 'C') && !locale[1])
				  || (!strcmp(locale, "POSIX"))) )
		? (char *) C_string		/* Always in C/POSIX locale. */
		: NULL;
}

#else /* ---------------------------------------------- __LOCALE_C_ONLY */

#ifdef __UCLIBC_HAS_THREADS__
link_warning(setlocale,"REMINDER: The 'setlocale' function is _not_ threadsafe except for simple queries.")
#endif

#if !defined(__LOCALE_DATA_NUM_LOCALES) || (__LOCALE_DATA_NUM_LOCALES <= 1)
#error locales enabled, but not data other than for C locale!
#endif

#ifdef __UCLIBC_MJN3_ONLY__
#warning TODO: Move posix and utf8 strings.
#endif
static const char posix[] = "POSIX";
static const char utf8[] = "UTF-8";

#ifdef __UCLIBC_MJN3_ONLY__
#warning TODO: Fix dimensions of hr_locale.
#endif
/* Individual category strings start at hr_locale + category * MAX_LOCALE_CATEGORY.
 * This holds for LC_ALL as well.
 */
static char hr_locale[(MAX_LOCALE_CATEGORY_STR * LC_ALL) + MAX_LOCALE_STR];


static void update_hr_locale(const unsigned char *spec)
{
	const unsigned char *loc;
	const unsigned char *s;
	char *n;
	int i, category, done;

	done = category = 0;
	do {
		s = spec + 1;
		n = hr_locale + category * MAX_LOCALE_CATEGORY_STR;

		if (category == LC_ALL) {
			done = 1;
			for (i = 0 ; i < LC_ALL-1 ; i += 2) {
				if ((s[i] != s[i+2]) || (s[i+1] != s[i+3])) {
					goto SKIP;
				}
			}
			/* All categories the same, so simplify string by using a single
			 * category. */
			category = LC_CTYPE;
		}

	SKIP:
		i = (category == LC_ALL) ? 0 : category;
		s += 2*i;

		do {
			if ((*s != 0xff) || (s[1] != 0xff)) {
				loc = LOCALES
					+ __LOCALE_DATA_WIDTH_LOCALES * ((((int)(*s & 0x7f)) << 7)
													 + (s[1] & 0x7f));
				if (category == LC_ALL) {
					/* CATEGORY_NAMES is unsigned char* */
					n = stpcpy(n, (char*) CATEGORY_NAMES + (int) CATEGORY_NAMES[i]);
					*n++ = '=';
				}
				if (*loc == 0) {
					*n++ = 'C';
					*n = 0;
				} else {
					char at = 0;
					memcpy(n, LOCALE_NAMES + 5*((*loc)-1), 5);
					if (n[2] != '_') {
						at = n[2];
						n[2] = '_';
					}
					n += 5;
					*n++ = '.';
					if (loc[2] == 2) {
						n = stpcpy(n, utf8);
					} else if (loc[2] >= 3) {
						n = stpcpy(n, (char*) CODESET_LIST + (int)(CODESET_LIST[loc[2] - 3]));
					}
					if (at) {
						const char *q;
						*n++ = '@';
						q = (char*) LOCALE_AT_MODIFIERS;
						do {
							if (q[1] == at) {
								n = stpcpy(n, q+2);
								break;
							}
							q += 2 + *q;
						} while (*q);
					}
				}
				*n++ = ';';
			}
			s += 2;
		} while (++i < category);
		*--n = 0;		/* Remove trailing ';' and nul-terminate. */

		++category;
	} while (!done);
}

char *setlocale(int category, const char *locale)
{
	if (((unsigned int)(category)) > LC_ALL) {
#if 0
		__set_errno(EINVAL);	/* glibc sets errno -- SUSv3 doesn't say. */
#endif
		return NULL;			/* Illegal/unsupported category. */
	}

	if (locale != NULL) {		/* Not just a query... */
		if (!newlocale((1 << category), locale, __global_locale)) {
			return NULL;		/* Failed! */
		}
		update_hr_locale(__global_locale->cur_locale);
	}

	/* Either a query or a successful set, so return current locale string. */
	return hr_locale + (category * MAX_LOCALE_CATEGORY_STR);
}

#endif /* __LOCALE_C_ONLY */

#endif
/**********************************************************************/
#ifdef L_localeconv

/* Note: We assume here that the compiler does the sane thing regarding
 * placement of the fields in the struct.  If necessary, we could ensure
 * this usings an array of offsets but at some size cost. */


#ifdef __LOCALE_C_ONLY

link_warning(localeconv,"REMINDER: The 'localeconv' function is hardwired for C/POSIX locale only.")

static struct lconv the_lconv;

static const char decpt[] = ".";

struct lconv *localeconv(void)
{
	register char *p = (char *)(&the_lconv);

	*((char **)p) = (char *) decpt;
	do {
		p += sizeof(char **);
		*((char **)p) = (char *) (decpt+1);
	} while (p < (char *) &the_lconv.negative_sign);

	p = (&the_lconv.int_frac_digits);
	do {
		*p = CHAR_MAX;
		++p;
	} while (p <= &the_lconv.int_n_sign_posn);

	return &the_lconv;
}

#else /* __LOCALE_C_ONLY */

static struct lconv the_lconv;

struct lconv *localeconv(void)
{
	register char *p = (char *) &the_lconv;
	register char **q = (char **) &(__UCLIBC_CURLOCALE->decimal_point);

	do {
		*((char **)p) = *q;
		p += sizeof(char **);
		++q;
	} while (p < &the_lconv.int_frac_digits);

	do {
		*p = **q;
		++p;
		++q;
	} while (p <= &the_lconv.int_n_sign_posn);

	return &the_lconv;
}

#endif /* __LOCALE_C_ONLY */

libc_hidden_def(localeconv)

#endif
/**********************************************************************/
#if defined(L__locale_init) && !defined(__LOCALE_C_ONLY)

struct __uclibc_locale_struct __global_locale_data;

__locale_t __global_locale = &__global_locale_data;

#ifdef __UCLIBC_HAS_XLOCALE__
__locale_t __curlocale_var = &__global_locale_data;
#endif

/*----------------------------------------------------------------------*/
#ifdef __UCLIBC_MJN3_ONLY__
#warning TODO: Move utf8 and ascii strings.
#endif
static const char utf8[] = "UTF-8";
static const char ascii[] = "ASCII";

typedef struct {
	uint16_t num_base;
	uint16_t num_der;
	uint16_t MAX_WEIGHTS;
	uint16_t num_index2weight;
#define num_index2ruleidx num_index2weight
	uint16_t num_weightstr;
	uint16_t num_multistart;
	uint16_t num_override;
	uint16_t num_ruletable;
} coldata_header_t;

typedef struct {
	uint16_t num_weights;
	uint16_t num_starters;
	uint16_t ii_shift;
	uint16_t ti_shift;
	uint16_t ii_len;
	uint16_t ti_len;
	uint16_t max_weight;
	uint16_t num_col_base;
	uint16_t max_col_index;
	uint16_t undefined_idx;
	uint16_t range_low;
	uint16_t range_count;
	uint16_t range_base_weight;
	uint16_t range_rule_offset;

	uint16_t index2weight_offset;
	uint16_t index2ruleidx_offset;
	uint16_t multistart_offset;
	uint16_t wcs2colidt_offset_low;
	uint16_t wcs2colidt_offset_hi;
} coldata_base_t;

typedef struct {
	uint16_t base_idx;
	uint16_t undefined_idx;
	uint16_t overrides_offset;
	uint16_t multistart_offset;
} coldata_der_t;

static int init_cur_collate(int der_num, __collate_t *cur_collate)
{
	const uint16_t *__locale_collate_tbl = __locale_mmap->collate_data;
	coldata_header_t *cdh;
	coldata_base_t *cdb;
	coldata_der_t *cdd;
	const uint16_t *p;
	size_t n;
	uint16_t i, w;

#ifdef __UCLIBC_MJN3_ONLY__
#warning kill of x86-specific asserts
#endif
#if 0
	assert(sizeof(coldata_base_t) == 19*2);
	assert(sizeof(coldata_der_t) == 4*2);
	assert(sizeof(coldata_header_t) == 8*2);
#endif

	if (!der_num) { 			/* C locale... special */
		cur_collate->num_weights = 0;
		return 1;
	}

	--der_num;

	cdh = (coldata_header_t *) __locale_collate_tbl;

#ifdef __UCLIBC_MJN3_ONLY__
#warning CONSIDER: Should we assert here?
#endif
#if 0
	if (der_num >= cdh->num_der) {
		return 0;
	}
#else
	assert((der_num < cdh->num_der));
#endif

	cdd = (coldata_der_t *)(__locale_collate_tbl
							+ (sizeof(coldata_header_t)
							   + cdh->num_base * sizeof(coldata_base_t)
							   + der_num * sizeof(coldata_der_t)
							   )/2 );

	cdb = (coldata_base_t *)(__locale_collate_tbl
							 + (sizeof(coldata_header_t)
								+ cdd->base_idx * sizeof(coldata_base_t)
								)/2 );

	memcpy(cur_collate, cdb, offsetof(coldata_base_t,index2weight_offset));
	cur_collate->undefined_idx = cdd->undefined_idx;

	cur_collate->ti_mask = (1 << cur_collate->ti_shift)-1;
	cur_collate->ii_mask = (1 << cur_collate->ii_shift)-1;

/* 	fflush(stdout); */
/* 	fprintf(stderr,"base=%d  num_col_base: %d  %d\n", cdd->base_idx ,cur_collate->num_col_base, cdb->num_col_base); */

	n = (sizeof(coldata_header_t) + cdh->num_base * sizeof(coldata_base_t)
		 + cdh->num_der * sizeof(coldata_der_t))/2;

/* 	fprintf(stderr,"n   = %d\n", n); */
	cur_collate->index2weight_tbl = __locale_collate_tbl + n + cdb->index2weight_offset;
/* 	fprintf(stderr,"i2w = %d\n", n + cdb->index2weight_offset); */
	n += cdh->num_index2weight;
	cur_collate->index2ruleidx_tbl = __locale_collate_tbl + n + cdb->index2ruleidx_offset;
/* 	fprintf(stderr,"i2r = %d\n", n + cdb->index2ruleidx_offset); */
	n += cdh->num_index2ruleidx;
	cur_collate->multistart_tbl = __locale_collate_tbl + n + cdd->multistart_offset;
/* 	fprintf(stderr,"mts = %d\n", n + cdb->multistart_offset); */
	n += cdh->num_multistart;
	cur_collate->overrides_tbl = __locale_collate_tbl + n + cdd->overrides_offset;
/* 	fprintf(stderr,"ovr = %d\n", n + cdd->overrides_offset); */
	n += cdh->num_override;
	cur_collate->ruletable = __locale_collate_tbl + n;
/* 	fprintf(stderr, "rtb = %d\n", n); */
	n += cdh->num_ruletable;
	cur_collate->weightstr = __locale_collate_tbl + n;
/* 	fprintf(stderr,"wts = %d\n", n); */
	n += cdh->num_weightstr;
	cur_collate->wcs2colidt_tbl = __locale_collate_tbl + n
		+ (((unsigned long)(cdb->wcs2colidt_offset_hi)) << 16)
		+ cdb->wcs2colidt_offset_low;
/* 	fprintf(stderr,"wcs = %lu\n", n	+ (((unsigned long)(cdb->wcs2colidt_offset_hi)) << 16) */
/* 			+ cdb->wcs2colidt_offset_low); */

	cur_collate->MAX_WEIGHTS = cdh->MAX_WEIGHTS;

#ifdef __UCLIBC_MJN3_ONLY__
#warning CONSIDER: Fix the +1 by increasing max_col_index?
#warning CONSIDER: Since this collate info is dependent only on LC_COLLATE ll_cc and not on codeset, we could just globally allocate this for each in a table
#endif

	cur_collate->index2weight = calloc(2*cur_collate->max_col_index+2,
									   sizeof(uint16_t));
	if (!cur_collate->index2weight) {
		return 0;
	}
	cur_collate->index2ruleidx = cur_collate->index2weight
		+ cur_collate->max_col_index + 1;

	memcpy(cur_collate->index2weight, cur_collate->index2weight_tbl,
		   cur_collate->num_col_base * sizeof(uint16_t));
	memcpy(cur_collate->index2ruleidx, cur_collate->index2ruleidx_tbl,
		   cur_collate->num_col_base * sizeof(uint16_t));

	/* now do the overrides */
	p = cur_collate->overrides_tbl;
	while (*p > 1) {
/* 		fprintf(stderr, "processing override -- count = %d\n", *p); */
		n = *p++;
		w = *p++;
		do {
			i = *p++;
/* 			fprintf(stderr, "	i=%d (%#x) w=%d *p=%d\n", i, i, w, *p); */
			cur_collate->index2weight[i-1] = w++;
			cur_collate->index2ruleidx[i-1] = *p++;
		} while (--n);
	}
	assert(*p == 1);
	while (*++p) {
		i = *p;
/* 		fprintf(stderr, "	i=%d (%#x) w=%d *p=%d\n", i, i, p[1], p[2]); */
		cur_collate->index2weight[i-1] = *++p;
		cur_collate->index2ruleidx[i-1] = *++p;
	}


	for (i=0 ; i < cur_collate->multistart_tbl[0] ; i++) {
		p = cur_collate->multistart_tbl;
/* 		fprintf(stderr, "%2d of %2d: %d ", i,  cur_collate->multistart_tbl[0], p[i]); */
		p += p[i];

		do {
			n = *p++;
			do {
				if (!*p) {		/* found it */
/* 					fprintf(stderr, "found: n=%d (%#lx) |%.*ls|\n", n, (int) *cs->s, n, cs->s); */
/* 					fprintf(stderr, ": %d - single\n", n); */
					goto FOUND;
 				}
				/* the lookup check here is safe since we're assured that *p is a valid colidex */
/* 				fprintf(stderr, "lookup(%lc)==%d  *p==%d\n", cs->s[n], lookup(cs->s[n]), (int) *p); */
/* 				fprintf(stderr, ": %d - ", n); */
				do {
/* 					fprintf(stderr, "%d|",  *p); */
				} while (*p++);
				break;
			} while (1);
		} while (1);
	FOUND:
		continue;
	}

	return 1;
}

int attribute_hidden _locale_set_l(const unsigned char *p, __locale_t base)
{
	const char **x;
	unsigned char *s = base->cur_locale + 1;
	const size_t *stp;
	const unsigned char *r;
	const uint16_t *io;
	const uint16_t *ii;
	const unsigned char *d;
	int row;					/* locale row */
	int crow;					/* category row */
	int len;
	int c;
	int i = 0;
	__collate_t newcol;

	++p;

	newcol.index2weight = NULL;
	if ((p[2*LC_COLLATE] != s[2*LC_COLLATE])
		|| (p[2*LC_COLLATE + 1] != s[2*LC_COLLATE + 1])
		) {
		row = (((int)(*p & 0x7f)) << 7) + (p[1] & 0x7f);
		assert(row < __LOCALE_DATA_NUM_LOCALES);
		if (!init_cur_collate(__locale_mmap->locales[ __LOCALE_DATA_WIDTH_LOCALES
													  * row + 3 + LC_COLLATE ],
							  &newcol)
			) {
			return 0;			/* calloc failed. */
		}
		free(base->collate.index2weight);
		memcpy(&base->collate, &newcol, sizeof(__collate_t));
	}

	do {
		if ((*p != *s) || (p[1] != s[1])) {
			row = (((int)(*p & 0x7f)) << 7) + (p[1] & 0x7f);
			assert(row < __LOCALE_DATA_NUM_LOCALES);

			*s = *p;
			s[1] = p[1];

			if ((i != LC_COLLATE)
				&& ((len = __locale_mmap->lc_common_item_offsets_LEN[i]) != 0)
				) {
				crow = __locale_mmap->locales[ __LOCALE_DATA_WIDTH_LOCALES * row
											   + 3 + i ]
					* len;

				x = (const char **)(((char *) base)
                                    + base->category_offsets[i]);

				stp = __locale_mmap->lc_common_tbl_offsets + 4*i;
				r = (const unsigned char *)( ((char *)__locale_mmap) + *stp );
				io = (const uint16_t *)( ((char *)__locale_mmap) + *++stp );
				ii = (const uint16_t *)( ((char *)__locale_mmap) + *++stp );
				d = (const unsigned char *)( ((char *)__locale_mmap) + *++stp );
				for (c = 0; c < len; c++) {
					x[c] = (char*)(d + ii[r[crow + c] + io[c]]);
				}
			}
			if (i == LC_CTYPE) {
				c = __locale_mmap->locales[ __LOCALE_DATA_WIDTH_LOCALES * row
											+ 2 ]; /* codeset */
				if (c <= 2) {
					if (c == 2) {
						base->codeset = utf8;
						base->encoding = __ctype_encoding_utf8;
						/* TODO - fix for bcc */
						base->mb_cur_max = 6;
					} else {
						assert(c == 1);
						base->codeset = ascii;
						base->encoding = __ctype_encoding_7_bit;
						base->mb_cur_max = 1;
					}
				} else {
					const __codeset_8_bit_t *c8b;
					r = CODESET_LIST;
					c -= 3;
					base->codeset = (char *) (r + r[c]);
					base->encoding = __ctype_encoding_8_bit;
#ifdef __UCLIBC_MJN3_ONLY__
#warning REMINDER: update 8 bit mb_cur_max when translit implemented!
#endif
					/* TODO - update when translit implemented! */
					base->mb_cur_max = 1;
					c8b = __locale_mmap->codeset_8_bit + c;
#ifdef __CTYPE_HAS_8_BIT_LOCALES
					base->idx8ctype = c8b->idx8ctype;
					base->idx8uplow = c8b->idx8uplow;
#ifdef __UCLIBC_HAS_WCHAR__
					base->idx8c2wc = c8b->idx8c2wc;
					base->idx8wc2c = c8b->idx8wc2c;
					/* translit  */
#endif /* __UCLIBC_HAS_WCHAR__ */

					/* What follows is fairly bloated, but it is just a hack
					 * to get the 8-bit codeset ctype stuff functioning.
					 * All of this will be replaced in the next generation
					 * of locale support anyway... */

					memcpy(base->__ctype_b_data,
						   __C_ctype_b - __UCLIBC_CTYPE_B_TBL_OFFSET,
						   (256 + __UCLIBC_CTYPE_B_TBL_OFFSET)
						   * sizeof(__ctype_mask_t));
					memcpy(base->__ctype_tolower_data,
						   __C_ctype_tolower - __UCLIBC_CTYPE_TO_TBL_OFFSET,
						   (256 + __UCLIBC_CTYPE_TO_TBL_OFFSET)
						   * sizeof(__ctype_touplow_t));
					memcpy(base->__ctype_toupper_data,
						   __C_ctype_toupper - __UCLIBC_CTYPE_TO_TBL_OFFSET,
						   (256 + __UCLIBC_CTYPE_TO_TBL_OFFSET)
						   * sizeof(__ctype_touplow_t));

#define Cctype_TBL_MASK		((1 << __LOCALE_DATA_Cctype_IDX_SHIFT) - 1)
#define Cctype_IDX_OFFSET	(128 >> __LOCALE_DATA_Cctype_IDX_SHIFT)

					{
						int u;
						__ctype_mask_t m;

						for (u=0 ; u < 128 ; u++) {
#ifdef __LOCALE_DATA_Cctype_PACKED
							c = base->tbl8ctype
								[ ((int)(c8b->idx8ctype
										 [(u >> __LOCALE_DATA_Cctype_IDX_SHIFT) ])
								   << (__LOCALE_DATA_Cctype_IDX_SHIFT - 1))
								  + ((u & Cctype_TBL_MASK) >> 1)];
							c = (u & 1) ? (c >> 4) : (c & 0xf);
#else
							c = base->tbl8ctype
								[ ((int)(c8b->idx8ctype
										 [(u >> __LOCALE_DATA_Cctype_IDX_SHIFT) ])
								   << __LOCALE_DATA_Cctype_IDX_SHIFT)
								  + (u & Cctype_TBL_MASK) ];
#endif

							m = base->code2flag[c];

							base->__ctype_b_data
								[128 + __UCLIBC_CTYPE_B_TBL_OFFSET + u]
								= m;

#ifdef __UCLIBC_HAS_CTYPE_SIGNED__
							if (((signed char)(128 + u)) != -1) {
								base->__ctype_b_data[__UCLIBC_CTYPE_B_TBL_OFFSET
													 + ((signed char)(128 + u))]
									= m;
							}
#endif

							base->__ctype_tolower_data
								[128 + __UCLIBC_CTYPE_TO_TBL_OFFSET + u]
								= 128 + u;
							base->__ctype_toupper_data
								[128 + __UCLIBC_CTYPE_TO_TBL_OFFSET + u]
								= 128 + u;

							if (m & (_ISlower|_ISupper)) {
								c = base->tbl8uplow
									[ ((int)(c8b->idx8uplow
											 [u >> __LOCALE_DATA_Cuplow_IDX_SHIFT])
									   << __LOCALE_DATA_Cuplow_IDX_SHIFT)
									  + ((128 + u)
										 & ((1 << __LOCALE_DATA_Cuplow_IDX_SHIFT)
											- 1)) ];
								if (m & _ISlower) {
									base->__ctype_toupper_data
										[128 + __UCLIBC_CTYPE_TO_TBL_OFFSET + u]
										= (unsigned char)(128 + u + c);
#ifdef __UCLIBC_HAS_CTYPE_SIGNED__
									if (((signed char)(128 + u)) != -1) {
										base->__ctype_toupper_data
											[__UCLIBC_CTYPE_TO_TBL_OFFSET
											 + ((signed char)(128 + u))]
											= (unsigned char)(128 + u + c);
									}
#endif
								} else {
									base->__ctype_tolower_data
										[128 + __UCLIBC_CTYPE_TO_TBL_OFFSET + u]
										= (unsigned char)(128 + u - c);
#ifdef __UCLIBC_HAS_CTYPE_SIGNED__
									if (((signed char)(128 + u)) != -1) {
										base->__ctype_tolower_data
											[__UCLIBC_CTYPE_TO_TBL_OFFSET
											 + ((signed char)(128 + u))]
											= (unsigned char)(128 + u - c);
									}
#endif
								}
							}
						}
					}

#ifdef __UCLIBC_HAS_XLOCALE__
					base->__ctype_b = base->__ctype_b_data
						+ __UCLIBC_CTYPE_B_TBL_OFFSET;
					base->__ctype_tolower = base->__ctype_tolower_data
						+ __UCLIBC_CTYPE_TO_TBL_OFFSET;
					base->__ctype_toupper = base->__ctype_toupper_data
						+ __UCLIBC_CTYPE_TO_TBL_OFFSET;
#else /* __UCLIBC_HAS_XLOCALE__ */
					__ctype_b = base->__ctype_b_data
						+ __UCLIBC_CTYPE_B_TBL_OFFSET;
					__ctype_tolower = base->__ctype_tolower_data
						+ __UCLIBC_CTYPE_TO_TBL_OFFSET;
					__ctype_toupper = base->__ctype_toupper_data
						+ __UCLIBC_CTYPE_TO_TBL_OFFSET;
#endif /* __UCLIBC_HAS_XLOCALE__ */

#endif /* __CTYPE_HAS_8_BIT_LOCALES */
				}
#ifdef __UCLIBC_MJN3_ONLY__
#warning TODO: Put the outdigit string length in the locale_mmap object.
#endif
				d = base->outdigit_length;
				x = &base->outdigit0_mb;
				for (c = 0 ; c < 10 ; c++) {
					((unsigned char *)d)[c] = strlen(x[c]);
					assert(d[c] > 0);
				}
			} else if (i == LC_NUMERIC) {
				assert(LC_NUMERIC > LC_CTYPE); /* Need ctype initialized. */

				base->decimal_point_len
					= __locale_mbrtowc_l(&base->decimal_point_wc,
											base->decimal_point, base);
				assert(base->decimal_point_len > 0);
				assert(base->decimal_point[base->decimal_point_len] == 0);

				if (*base->grouping) {
					base->thousands_sep_len
						= __locale_mbrtowc_l(&base->thousands_sep_wc,
											 base->thousands_sep, base);
#if 1
#ifdef __UCLIBC_MJN3_ONLY__
#warning TODO: Remove hack involving grouping without a thousep char (bg_BG).
#endif
					assert(base->thousands_sep_len >= 0);
					if (base->thousands_sep_len == 0) {
						base->grouping = base->thousands_sep; /* empty string */
					}
					assert(base->thousands_sep[base->thousands_sep_len] == 0);
#else
					assert(base->thousands_sep_len > 0);
					assert(base->thousands_sep[base->thousands_sep_len] == 0);
#endif
				}

/* 			} else if (i == LC_COLLATE) { */
/* 				init_cur_collate(__locale_mmap->locales[ __LOCALE_DATA_WIDTH_LOCALES */
/* 														 * row + 3 + i ], */
/* 								 &base->collate); */
			}
		}
		++i;
		p += 2;
		s += 2;
	} while (i < LC_ALL);

	return 1;
}

static const uint16_t __code2flag[16] = {
	0,							/* unclassified = 0 */
	_ISprint|_ISgraph|_ISalnum|_ISalpha, /* alpha_nonupper_nonlower */
	_ISprint|_ISgraph|_ISalnum|_ISalpha|_ISlower, /* alpha_lower */
	_ISprint|_ISgraph|_ISalnum|_ISalpha|_ISlower|_ISupper, /* alpha_upper_lower */
	_ISprint|_ISgraph|_ISalnum|_ISalpha|_ISupper, /* alpha_upper */
	_ISprint|_ISgraph|_ISalnum|_ISdigit, /* digit */
	_ISprint|_ISgraph|_ISpunct,	/* punct */
	_ISprint|_ISgraph,			/* graph */
	_ISprint|_ISspace,			/* print_space_nonblank */
	_ISprint|_ISspace|_ISblank,	/* print_space_blank */
	         _ISspace,			/* space_nonblank_noncntrl */
	         _ISspace|_ISblank,	/* space_blank_noncntrl */
	_IScntrl|_ISspace,			/* cntrl_space_nonblank */
	_IScntrl|_ISspace|_ISblank,	/* cntrl_space_blank */
	_IScntrl					/* cntrl_nonspace */
};

void attribute_hidden _locale_init_l(__locale_t base)
{
	memset(base->cur_locale, 0, LOCALE_SELECTOR_SIZE);
	base->cur_locale[0] = '#';

	memcpy(base->category_item_count,
		   __locale_mmap->lc_common_item_offsets_LEN,
		   LC_ALL);

	++base->category_item_count[0]; /* Increment for codeset entry. */
	base->category_offsets[0] = offsetof(struct __uclibc_locale_struct, outdigit0_mb);
	base->category_offsets[1] = offsetof(struct __uclibc_locale_struct, decimal_point);
	base->category_offsets[2] = offsetof(struct __uclibc_locale_struct, int_curr_symbol);
	base->category_offsets[3] = offsetof(struct __uclibc_locale_struct, abday_1);
/*  	base->category_offsets[4] = offsetof(struct __uclibc_locale_struct, collate???); */
	base->category_offsets[5] = offsetof(struct __uclibc_locale_struct, yesexpr);

#ifdef __CTYPE_HAS_8_BIT_LOCALES
	base->tbl8ctype
		= (const unsigned char *) &__locale_mmap->tbl8ctype;
	base->tbl8uplow
		= (const unsigned char *) &__locale_mmap->tbl8uplow;
#ifdef __UCLIBC_HAS_WCHAR__
	base->tbl8c2wc
		= (const uint16_t *) &__locale_mmap->tbl8c2wc;
	base->tbl8wc2c
		= (const unsigned char *) &__locale_mmap->tbl8wc2c;
	/* translit  */
#endif /* __UCLIBC_HAS_WCHAR__ */
#endif /* __CTYPE_HAS_8_BIT_LOCALES */
#ifdef __UCLIBC_HAS_WCHAR__
	base->tblwctype
		= (const unsigned char *) &__locale_mmap->tblwctype;
	base->tblwuplow
		= (const unsigned char *) &__locale_mmap->tblwuplow;
	base->tblwuplow_diff
		= (const int16_t *) &__locale_mmap->tblwuplow_diff;
/* 	base->tblwcomb */
/* 		= (const unsigned char *) &__locale_mmap->tblwcomb; */
	/* width?? */
#endif /* __UCLIBC_HAS_WCHAR__ */

	/* Initially, set things up to use the global C ctype tables.
	 * This is correct for C (ASCII) and UTF-8 based locales (except tr_TR). */
#ifdef __UCLIBC_HAS_XLOCALE__
	base->__ctype_b = __C_ctype_b;
	base->__ctype_tolower = __C_ctype_tolower;
	base->__ctype_toupper = __C_ctype_toupper;
#else /* __UCLIBC_HAS_XLOCALE__ */
	__ctype_b = __C_ctype_b;
	__ctype_tolower = __C_ctype_tolower;
	__ctype_toupper = __C_ctype_toupper;
#endif /* __UCLIBC_HAS_XLOCALE__ */

#ifdef __UCLIBC_MJN3_ONLY__
#warning TODO: Initialize code2flag correctly based on locale_mmap.
#endif
	base->code2flag = __code2flag;

	base->collate.index2weight = NULL;

	_locale_set_l((unsigned char*) C_LOCALE_SELECTOR, base);
}

void _locale_init(void) attribute_hidden;
void _locale_init(void)
{
	/* TODO: mmap the locale file  */

	/* TODO - ??? */
	_locale_init_l(__global_locale);
}

#endif
/**********************************************************************/
#if defined(L_nl_langinfo) || defined(L_nl_langinfo_l)

#ifdef __LOCALE_C_ONLY

/* We need to index 320 bytes of data, so you might initially think we
 * need to store the offsets in shorts.  But since the offset of the
 * 64th item is 182, we'll store "offset - 2*64" for all items >= 64
 * and always calculate the data offset as "offset[i] + 2*(i & 64)".
 * This allows us to pack the data offsets in an unsigned char while
 * also avoiding an "if".
 *
 * Note: Category order is assumed to be:
 *   ctype, numeric, monetary, time, collate, messages, all
 */

#define C_LC_ALL 6

/* Combine the data to avoid size penalty for seperate char arrays when
 * compiler aligns objects.  The original code is left in as documentation. */
#define cat_start nl_data
#define C_locale_data (nl_data + C_LC_ALL + 1 + 90)

static const unsigned char nl_data[C_LC_ALL + 1 + 90 + 320] = {
/* static const char cat_start[LC_ALL + 1] = { */
	'\x00', '\x0b', '\x0e', '\x24', '\x56', '\x56', '\x5a',
/* }; */
/* static const char item_offset[90] = { */
	'\x00', '\x02', '\x04', '\x06', '\x08', '\x0a', '\x0c', '\x0e',
	'\x10', '\x12', '\x14', '\x1a', '\x1b', '\x1b', '\x1b', '\x1b',
	'\x1b', '\x1b', '\x1b', '\x1b', '\x1b', '\x1c', '\x1c', '\x1c',
	'\x1c', '\x1c', '\x1c', '\x1c', '\x1c', '\x1c', '\x1c', '\x1c',
	'\x1c', '\x1c', '\x1c', '\x1e', '\x20', '\x24', '\x28', '\x2c',
	'\x30', '\x34', '\x38', '\x3c', '\x43', '\x4a', '\x52', '\x5c',
	'\x65', '\x6c', '\x75', '\x79', '\x7d', '\x81', '\x85', '\x89',
	'\x8d', '\x91', '\x95', '\x99', '\x9d', '\xa1', '\xa5', '\xad',
	'\x36', '\x3c', '\x42', '\x46', '\x4b', '\x50', '\x57', '\x61',
	'\x69', '\x72', '\x7b', '\x7e', '\x81', '\x96', '\x9f', '\xa8',
	'\xb3', '\xb3', '\xb3', '\xb3', '\xb3', '\xb3', '\xb4', '\xba',
	'\xbf', '\xbf',
/* }; */
/* static const char C_locale_data[320] = { */
	   '0', '\x00',    '1', '\x00',    '2', '\x00',    '3', '\x00',
	   '4', '\x00',    '5', '\x00',    '6', '\x00',    '7', '\x00',
	   '8', '\x00',    '9', '\x00',    'A',    'S',    'C',    'I',
	   'I', '\x00',    '.', '\x00', '\x7f', '\x00',    '-', '\x00',
	   'S',    'u',    'n', '\x00',    'M',    'o',    'n', '\x00',
	   'T',    'u',    'e', '\x00',    'W',    'e',    'd', '\x00',
	   'T',    'h',    'u', '\x00',    'F',    'r',    'i', '\x00',
	   'S',    'a',    't', '\x00',    'S',    'u',    'n',    'd',
	   'a',    'y', '\x00',    'M',    'o',    'n',    'd',    'a',
	   'y', '\x00',    'T',    'u',    'e',    's',    'd',    'a',
	   'y', '\x00',    'W',    'e',    'd',    'n',    'e',    's',
	   'd',    'a',    'y', '\x00',    'T',    'h',    'u',    'r',
	   's',    'd',    'a',    'y', '\x00',    'F',    'r',    'i',
	   'd',    'a',    'y', '\x00',    'S',    'a',    't',    'u',
	   'r',    'd',    'a',    'y', '\x00',    'J',    'a',    'n',
	'\x00',    'F',    'e',    'b', '\x00',    'M',    'a',    'r',
	'\x00',    'A',    'p',    'r', '\x00',    'M',    'a',    'y',
	'\x00',    'J',    'u',    'n', '\x00',    'J',    'u',    'l',
	'\x00',    'A',    'u',    'g', '\x00',    'S',    'e',    'p',
	'\x00',    'O',    'c',    't', '\x00',    'N',    'o',    'v',
	'\x00',    'D',    'e',    'c', '\x00',    'J',    'a',    'n',
	   'u',    'a',    'r',    'y', '\x00',    'F',    'e',    'b',
	   'r',    'u',    'a',    'r',    'y', '\x00',    'M',    'a',
	   'r',    'c',    'h', '\x00',    'A',    'p',    'r',    'i',
	   'l', '\x00',    'M',    'a',    'y', '\x00',    'J',    'u',
	   'n',    'e', '\x00',    'J',    'u',    'l',    'y', '\x00',
	   'A',    'u',    'g',    'u',    's',    't', '\x00',    'S',
	   'e',    'p',    't',    'e',    'm',    'b',    'e',    'r',
	'\x00',    'O',    'c',    't',    'o',    'b',    'e',    'r',
	'\x00',    'N',    'o',    'v',    'e',    'm',    'b',    'e',
	   'r', '\x00',    'D',    'e',    'c',    'e',    'm',    'b',
	   'e',    'r', '\x00',    'A',    'M', '\x00',    'P',    'M',
	'\x00',    '%',    'a',    ' ',    '%',    'b',    ' ',    '%',
	   'e',    ' ',    '%',    'H',    ':',    '%',    'M',    ':',
	   '%',    'S',    ' ',    '%',    'Y', '\x00',    '%',    'm',
	   '/',    '%',    'd',    '/',    '%',    'y', '\x00',    '%',
	   'H',    ':',    '%',    'M',    ':',    '%',    'S', '\x00',
	   '%',    'I',    ':',    '%',    'M',    ':',    '%',    'S',
	   ' ',    '%',    'p', '\x00',    '^',    '[',    'y',    'Y',
	   ']', '\x00',    '^',    '[',    'n',    'N',    ']', '\x00',
};

char *nl_langinfo(nl_item item)
{
	unsigned int c;
	unsigned int i;

	if ((c = _NL_ITEM_CATEGORY(item)) < C_LC_ALL) {
		if ((i = cat_start[c] + _NL_ITEM_INDEX(item)) < cat_start[c+1]) {
/*  			return (char *) C_locale_data + item_offset[i] + (i & 64); */
			return (char *) C_locale_data + nl_data[C_LC_ALL+1+i] + 2*(i & 64);
		}
	}
	return (char *) cat_start;	/* Conveniently, this is the empty string. */
}
libc_hidden_def(nl_langinfo)

#else /* __LOCALE_C_ONLY */

#if defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE)



char *nl_langinfo(nl_item item)
{
	return nl_langinfo_l(item, __UCLIBC_CURLOCALE);
}
libc_hidden_def(nl_langinfo)

#else /* defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE) */

libc_hidden_proto(__XL_NPP(nl_langinfo))

static const char empty[] = "";

char *__XL_NPP(nl_langinfo)(nl_item item __LOCALE_PARAM )
{
	unsigned int c = _NL_ITEM_CATEGORY(item);
	unsigned int i = _NL_ITEM_INDEX(item);

	if ((c < LC_ALL) && (i < __LOCALE_PTR->category_item_count[c])) {
		return ((char **)(((char *) __LOCALE_PTR)
						  + __LOCALE_PTR->category_offsets[c]))[i];
	}

	return (char *) empty;
}
libc_hidden_def(__XL_NPP(nl_langinfo))

#endif /* defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE) */

#endif /* __LOCALE_C_ONLY */

#endif
/**********************************************************************/
#ifdef L_newlocale

#warning mask defines for extra locale categories

#ifdef __UCLIBC_MJN3_ONLY__
#warning TODO: Move posix and utf8 strings.
#endif
static const char posix[] = "POSIX";
static const char utf8[] = "UTF-8";

static int find_locale(int category_mask, const char *p,
					   unsigned char *new_locale)
{
	int i;
	const unsigned char *s;
	uint16_t n;
	unsigned char lang_cult, codeset;

#if defined(__LOCALE_DATA_AT_MODIFIERS_LENGTH) && 1
	/* Support standard locale handling for @-modifiers. */

#ifdef __UCLIBC_MJN3_ONLY__
#warning REMINDER: Fix buf size in find_locale.
#endif
	char buf[18];	/* TODO: 7+{max codeset name length} */
	const char *q;

	if ((q = strchr(p,'@')) != NULL) {
		if ((((size_t)((q-p)-5)) > (sizeof(buf) - 5)) || (p[2] != '_')) {
			return 0;
		}
		/* locale name at least 5 chars long and 3rd char is '_' */
		s = LOCALE_AT_MODIFIERS;
		do {
			if (!strcmp((char*) (s + 2), q + 1)) {
				break;
			}
			s += 2 + *s;		/* TODO - fix this throughout */
		} while (*s);
		if (!*s) {
			return 0;
		}
		assert(q - p < sizeof(buf));
		memcpy(buf, p, q-p);
		buf[q-p] = 0;
		buf[2] = s[1];
		p = buf;
	}
#endif

	lang_cult = codeset = 0;	/* Assume C and default codeset.  */
	if (((*p == 'C') && !p[1]) || !strcmp(p, posix)) {
		goto FIND_LOCALE;
	}

	if ((strlen(p) > 5) && (p[5] == '.')) {	/* Codeset in locale name? */
		/* TODO: maybe CODESET_LIST + *s ??? */
		/* 7bit is 1, UTF-8 is 2, 8-bit is >= 3 */
		codeset = 2;
		if (strcasecmp(utf8, p + 6) != 0) {/* TODO - fix! */
			s = CODESET_LIST;
			do {
				++codeset;		/* Increment codeset first. */
				if (!strcmp((char*) CODESET_LIST + *s, p + 6)) {
					goto FIND_LANG_CULT;
				}
			} while (*++s);
			return 0;			/* No matching codeset! */
		}
	}

 FIND_LANG_CULT:				/* Find language_culture number. */
	s = LOCALE_NAMES;
	do {						/* TODO -- do a binary search? */
		/* TODO -- fix gen_mmap!*/
		++lang_cult;			/* Increment first since C/POSIX is 0. */
		if (!strncmp((char*) s, p, 5)) { /* Found a matching locale name; */
			goto FIND_LOCALE;
		}
		s += 5;
	} while (lang_cult < __LOCALE_DATA_NUM_LOCALE_NAMES);
	return 0;					/* No matching language_culture! */

 FIND_LOCALE:					/* Find locale row matching name and codeset */
	s = LOCALES;
	n = 0;
	do {						/* TODO -- do a binary search? */
		if ((lang_cult == *s) && ((codeset == s[1]) || (codeset == s[2]))) {
			i = 1;
			s = new_locale + 1;
			do {
				if (category_mask & i) {
					/* Encode current locale row number. */
					((unsigned char *) s)[0] = (n >> 7) | 0x80;
					((unsigned char *) s)[1] = (n & 0x7f) | 0x80;
				}
				s += 2;
				i += i;
			} while (i < (1 << LC_ALL));

			return i;			/* Return non-zero */
		}
		s += __LOCALE_DATA_WIDTH_LOCALES;
		++n;
	} while (n <= __LOCALE_DATA_NUM_LOCALES); /* We started at 1!!! */

	return 0;					/* Unsupported locale. */
}

static unsigned char *composite_locale(int category_mask, const char *locale,
									   unsigned char *new_locale)
{
	char buf[MAX_LOCALE_STR];
	char *t;
	char *e;
	int c;
	int component_mask;

	if (!strchr(locale,'=')) {
		if (!find_locale(category_mask, locale, new_locale)) {
			return NULL;
		}
		return new_locale;
	}

	if (strlen(locale) >= sizeof(buf)) {
		return NULL;
	}
	stpcpy(buf, locale);

	component_mask = 0;
	t = strtok_r(buf, "=", &e);	/* This can't fail because of strchr test above. */
	do {
		c = 0;
		/* CATEGORY_NAMES is unsigned char* */
		while (strcmp((char*) CATEGORY_NAMES + (int) CATEGORY_NAMES[c], t)) {
			if (++c == LC_ALL) { /* Unknown category name! */
				return NULL;
			}
		}
		t = strtok_r(NULL, ";", &e);
		c = (1 << c);
		if (component_mask & c) { /* Multiple components for one category. */
			return NULL;
		}
		component_mask |= c;
		if ((category_mask & c) && (!t || !find_locale(c, t, new_locale))) {
			return NULL;
		}
	} while ((t = strtok_r(NULL, "=", &e)) != NULL);

	if (category_mask & ~component_mask) { /* Category component(s) missing. */
		return NULL;
	}

	return new_locale;
}

__locale_t newlocale(int category_mask, const char *locale, __locale_t base)
{
	const char *p;
	int i, j, k;
	unsigned char new_selector[LOCALE_SELECTOR_SIZE];

	if (category_mask == (1 << LC_ALL)) {
		category_mask = LC_ALL_MASK;
	}

	if (!locale || ((unsigned)(category_mask) > LC_ALL_MASK)) {
 INVALID:
		__set_errno(EINVAL);
		return NULL; /* No locale or illegal/unsupported category. */
	}

#ifdef __UCLIBC_MJN3_ONLY__
#warning TODO: Rename cur_locale to locale_selector.
#endif
	strcpy((char *) new_selector,
		   (base ? (char *) base->cur_locale : C_LOCALE_SELECTOR));

	if (!locale[0]) {	/* locale == "", so check environment. */
		const char *envstr[4];

		envstr[0] = "LC_ALL";
		envstr[1] = NULL;
		envstr[2] = "LANG";
		envstr[3] = posix;

		i = 1;
		k = 0;
		do {
			if (category_mask & i) {
				/* Note: SUSv3 doesn't define a fallback mechanism here.
				 * So, if LC_ALL is invalid, we do _not_ continue trying
				 * the other environment vars. */
				envstr[1] = (char*) CATEGORY_NAMES + CATEGORY_NAMES[k];
				j = 0;
				while (1) {
					p = envstr[j];
					if (++j >= 4)
						break; /* now p == "POSIX" */
					p = getenv(p);
					if (p && p[0])
						break;
				};

				/* The user set something... is it valid? */
				/* Note: Since we don't support user-supplied locales and
				 * alternate paths, we don't need to worry about special
				 * handling for suid/sgid apps. */
				if (!find_locale(i, p, new_selector)) {
					goto INVALID;
				}
			}
			i += i;
		} while (++k < LC_ALL);
	} else if (!composite_locale(category_mask, locale, new_selector)) {
		goto INVALID;
	}

#ifdef __UCLIBC_MJN3_ONLY__
#warning TODO: Do a compatible codeset check!
#endif

	/* If we get here, the new selector corresponds to a valid locale. */

#ifdef __UCLIBC_MJN3_ONLY__
#warning CONSIDER: Probably want a _locale_new func to allow for caching of locales.
#endif
#if 0
	if (base) {
		_locale_set_l(new_selector, base);
	} else {
		base = _locale_new(new_selector);
	}
#else
	if (!base) {
		base = calloc(1, sizeof(struct __uclibc_locale_struct));
		if (base == NULL)
			return base;
		_locale_init_l(base);
	}

	_locale_set_l(new_selector, base);
#endif

	return base;
}
#ifdef __UCLIBC_HAS_XLOCALE__
libc_hidden_def(newlocale)
#endif

#endif
/**********************************************************************/
#ifdef L_duplocale


#ifdef __UCLIBC_MJN3_ONLY__
#warning REMINDER: When we allocate ctype tables, remember to dup them.
#endif

__locale_t duplocale(__locale_t dataset)
{
	__locale_t r;
	uint16_t * i2w;
	size_t n;

	assert(dataset != LC_GLOBAL_LOCALE);

	r = malloc(sizeof(struct __uclibc_locale_struct));
	if (r != NULL) {
		n = 2 * dataset->collate.max_col_index + 2;
		i2w = calloc(n, sizeof(uint16_t));
		if (i2w != NULL) {
			memcpy(r, dataset, sizeof(struct __uclibc_locale_struct));
			r->collate.index2weight = i2w;
			memcpy(i2w, dataset->collate.index2weight, n * sizeof(uint16_t));
		} else {
			free(r);
			r = NULL;
		}
	}
	return r;
}

#endif
/**********************************************************************/
#ifdef L_freelocale

#ifdef __UCLIBC_MJN3_ONLY__
#warning REMINDER: When we allocate ctype tables, remember to free them.
#endif

void freelocale(__locale_t dataset)
{
	assert(dataset != __global_locale);
	assert(dataset != LC_GLOBAL_LOCALE);

	free(dataset->collate.index2weight); /* Free collation data. */
	free(dataset);				/* Free locale */
}

#endif
/**********************************************************************/
#ifdef L_uselocale

__locale_t uselocale(__locale_t dataset)
{
	__locale_t old;

	if (!dataset) {
		old = __UCLIBC_CURLOCALE;
	} else {
		if (dataset == LC_GLOBAL_LOCALE) {
			dataset = __global_locale;
		}
#ifdef __UCLIBC_HAS_THREADS__
		old = __curlocale_set(dataset);
#else
		old = __curlocale_var;
		__curlocale_var = dataset;
#endif
	}

	if (old == __global_locale) {
		return LC_GLOBAL_LOCALE;
	}
	return old;
}
libc_hidden_def(uselocale)

#endif
/**********************************************************************/
#ifdef L___curlocale

#ifdef __UCLIBC_HAS_THREADS__

__locale_t weak_const_function __curlocale(void)
{
	return __curlocale_var; /* This is overriden by the thread version. */
}

__locale_t weak_function __curlocale_set(__locale_t newloc)
{
	__locale_t oldloc = __curlocale_var;
	assert(newloc != LC_GLOBAL_LOCALE);
	__curlocale_var = newloc;
	return oldloc;
}

#endif

#endif
/**********************************************************************/
#ifdef L___locale_mbrtowc_l

/* NOTE: This returns an int... not size_t.  Also, it is not a general
 * routine.  It is actually a very stripped-down version of mbrtowc
 * that takes a __locale_t arg.  This is used by strcoll and strxfrm.
 * It is also used above to generate wchar_t versions of the decimal point
 * and thousands seperator. */


#ifndef __CTYPE_HAS_UTF_8_LOCALES
#warning __CTYPE_HAS_UTF_8_LOCALES not set!
#endif
#ifndef __CTYPE_HAS_8_BIT_LOCALES
#warning __CTYPE_HAS_8_BIT_LOCALES not set!
#endif

#define Cc2wc_IDX_SHIFT		__LOCALE_DATA_Cc2wc_IDX_SHIFT
#define Cc2wc_ROW_LEN		__LOCALE_DATA_Cc2wc_ROW_LEN

extern size_t _wchar_utf8sntowcs(wchar_t *__restrict pwc, size_t wn,
						 const char **__restrict src, size_t n,
						 mbstate_t *ps, int allow_continuation) attribute_hidden;

int attribute_hidden __locale_mbrtowc_l(wchar_t *__restrict dst,
					   const char *__restrict src,
					   __locale_t loc )
{
#ifdef __CTYPE_HAS_UTF_8_LOCALES
	if (loc->encoding == __ctype_encoding_utf8) {
		mbstate_t ps;
		const char *p = src;
		size_t r;
		ps.__mask = 0;
		r = _wchar_utf8sntowcs(dst, 1, &p, SIZE_MAX, &ps, 1);
		return (r == 1) ? (p-src) : r; /* Need to return 0 if nul char. */
	}
#endif

#ifdef __CTYPE_HAS_8_BIT_LOCALES
	assert((loc->encoding == __ctype_encoding_7_bit) || (loc->encoding == __ctype_encoding_8_bit));
#else
	assert(loc->encoding == __ctype_encoding_7_bit);
#endif

	if ((*dst = ((unsigned char)(*src))) < 0x80) {	/* ASCII... */
		return (*src != 0);
	}

#ifdef __CTYPE_HAS_8_BIT_LOCALES
	if (loc->encoding == __ctype_encoding_8_bit) {
		wchar_t wc = *dst - 0x80;
		*dst = loc->tbl8c2wc[
						(loc->idx8c2wc[wc >> Cc2wc_IDX_SHIFT]
						 << Cc2wc_IDX_SHIFT) + (wc & (Cc2wc_ROW_LEN - 1))];
		if (*dst) {
			return 1;
		}
	}
#endif

	return -1;
}

#endif
/**********************************************************************/
