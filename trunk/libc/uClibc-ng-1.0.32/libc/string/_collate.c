/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/*  Dec 20, 2002
 *  Initial test implementation of strcoll, strxfrm, wcscoll, and wcsxfrm.
 *  The code needs to be cleaned up a good bit, but I'd like to see people
 *  test it out.
 *
 */

#include "_string.h"
#include <ctype.h>
#include <locale.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#ifdef __UCLIBC_HAS_LOCALE__
#if defined(L_strxfrm) || defined(L_strxfrm_l) || defined(L_wcsxfrm) || defined(L_wcsxfrm_l)

#ifdef L_strxfrm
#ifndef WANT_WIDE
#error WANT_WIDE should be defined for L_strxfrm
#endif
#ifdef L_wcsxfrm
#error L_wcsxfrm already defined for L_strxfrm
#endif
#endif /* L_strxfrm */

#if defined(L_strxfrm) || defined(L_strxfrm_l)

#define wcscoll   strcoll
#define wcscoll_l strcoll_l
#define wcsxfrm   strxfrm
#define wcsxfrm_l strxfrm_l

#undef WANT_WIDE
#undef Wvoid
#undef Wchar
#undef Wuchar
#undef Wint

#define Wchar char

#endif /* defined(L_strxfrm) || defined(L_strxfrm_l) */

#if defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE)


int wcscoll (const Wchar *s0, const Wchar *s1)
{
	return wcscoll_l(s0, s1, __UCLIBC_CURLOCALE );
}
libc_hidden_def(wcscoll)


size_t wcsxfrm(Wchar *__restrict ws1, const Wchar *__restrict ws2, size_t n)
{
	return wcsxfrm_l(ws1, ws2, n, __UCLIBC_CURLOCALE );
}

#else  /* defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE) */


#if 0
#define CUR_COLLATE (&__UCLIBC_CURLOCALE->collate)
#else
#define CUR_COLLATE (& __LOCALE_PTR->collate)
#endif

#define MAX_PENDING 8

typedef struct {
	const Wchar *s;
	const Wchar *eob;			/* end of backward */

	__uwchar_t weight;
	__uwchar_t ui_weight;		/* undefined or invalid */
	int colitem;
	int weightidx;
	int rule;
	size_t position;
	/* should be wchar_t.  if wchar < 0 do EILSEQ? */
	__uwchar_t *cip;
	__uwchar_t ci_pending[MAX_PENDING];	/* nul-terminated */

	char *back_buf;
	char *bbe;					/* end of back_buf (actual last... not 1 past end) */
	char *bp;					/* ptr into backbuf, NULL if not in backward mode */
	char ibb[128];
	size_t bb_size;

	int ru_pushed;
} col_state_t;


#define WEIGHT_MASK	0x3fffU
#define RULE_MASK	0xc000U

#define RULE_FORWARD  (1 << 14)
#define RULE_POSITION (1 << 15)

#define UI_IDX		(WEIGHT_MASK-6)
#define POSIT_IDX	(WEIGHT_MASK-5)
#define RANGE_IDX	(WEIGHT_MASK-4)
#define UNDEF_IDX	(WEIGHT_MASK-3)
#define INVAL_IDX	(WEIGHT_MASK-2)
#define DITTO_IDX   (WEIGHT_MASK-1)


#undef TRACE
#if 0
#define TRACE(X)	printf X
#else
#define TRACE(X)	((void)0)
#endif

static int lookup(wchar_t wc   __LOCALE_PARAM )
{
	unsigned int sc, n, i0, i1;

	if (((__uwchar_t) wc) > 0xffffU) {
		return 0;
	}

	sc = wc & CUR_COLLATE->ti_mask;
	wc >>= CUR_COLLATE->ti_shift;
	n = wc & CUR_COLLATE->ii_mask;
	wc >>= CUR_COLLATE->ii_shift;

	i0 = CUR_COLLATE->wcs2colidt_tbl[wc];
	i0 <<= CUR_COLLATE->ii_shift;
	i1 = CUR_COLLATE->wcs2colidt_tbl[CUR_COLLATE->ii_len + i0 + n];
	i1 <<= CUR_COLLATE->ti_shift;
	return CUR_COLLATE->wcs2colidt_tbl[CUR_COLLATE->ii_len + CUR_COLLATE->ti_len + i1 + sc];

}

static void init_col_state(col_state_t *cs, const Wchar *wcs)
{
	memset(cs, 0, sizeof(col_state_t));
	cs->s = wcs;
	cs->bp = cs->back_buf = cs->ibb;
	cs->bb_size = 128;
	cs->bbe = cs->back_buf + (cs->bb_size -1);
}

static void next_weight(col_state_t *cs, int pass   __LOCALE_PARAM )
{
	int r, w, ru, ri, popping_backup_stack;
	ssize_t n;
	const uint16_t *p;
#ifdef WANT_WIDE
#define WC (*cs->s)
#define N (1)
#else  /* WANT_WIDE */
	wchar_t WC;
	size_t n0, nx = 0;
#define N n0

#endif /* WANT_WIDE */

	do {

		if (cs->ru_pushed) {
			ru = cs->ru_pushed;
			TRACE(("ru_pushed = %d\n", ru));
			cs->ru_pushed = 0;
			goto POSITION_SKIP;
		}

		if (cs->cip) {			/* possible pending weight */
			if ((r = *(cs->cip++)) == 0) {
				cs->cip = NULL;
				continue;
			}
			cs->weightidx = r & WEIGHT_MASK;
			assert(cs->weightidx);
/* 			assert(cs->weightidx != WEIGHT_MASK); */
		} else {				/* get the next collation item from the string */
			TRACE(("clearing popping flag\n"));
			popping_backup_stack = 0;

		IGNORE_LOOP:
			/* keep first pos as 0 for a sentinal */
			if (*cs->bp) {				/* pending backward chars */
			POP_BACKUP:
				popping_backup_stack = 1;
				TRACE(("setting popping flag\n"));
				n = 0;
				if (*cs->bp > 0) {		/* singles pending */
					cs->s -= 1;
					if ((*cs->bp -= 1) == 0) {
						cs->bp -= 1;
					}
				} else {				/* last was a multi */
					cs->s += *cs->bp;
					cs->bp -= 1;
				}
			} else if (!*cs->s) { /* not in backward mode and end of string */
				cs->weight = 0;
				return;
			} else {
				cs->position += 1;
			}

		BACK_LOOP:
#ifdef WANT_WIDE
			n = 1;
			cs->colitem = r = lookup(*cs->s   __LOCALE_ARG );
#else  /* WANT_WIDE */
			n = n0 = __locale_mbrtowc_l(&WC, cs->s, __LOCALE_PTR);
			if (n < 0) {
				__set_errno(EILSEQ);
				cs->weight = 0;
				return;
			}
			cs->colitem = r = lookup(WC   __LOCALE_ARG );
#endif /* WANT_WIDE */

			TRACE((" r=%d WC=%#lx\n", r, (unsigned long)(WC)));

			if (r > CUR_COLLATE->max_col_index) { /* starting char for one or more sequences */
				p = CUR_COLLATE->multistart_tbl;
				p += p[r-CUR_COLLATE->max_col_index -1];
				do {
					n = N;
					r = *p++;
					do {
						if (!*p) {		/* found it */
							cs->colitem = r;
							TRACE(("    found multi %d\n", n));
							goto FOUND;
						}
#ifdef WANT_WIDE
						/* the lookup check here is safe since we're assured that *p is a valid colidx */
						if (!cs->s[n] || (lookup(cs->s[n]   __LOCALE_ARG ) != *p)) {
							do {} while (*p++);
							break;
						}
						++p;
						++n;
#else  /* WANT_WIDE */
						if (cs->s[n]) {
							nx = __locale_mbrtowc_l(&WC, cs->s + n, __LOCALE_PTR);
							if (nx < 0) {
								__set_errno(EILSEQ);
								cs->weight = 0;
								return;
							}
						}
						if (!cs->s[n] || (lookup(WC   __LOCALE_ARG ) != *p)) {
							do {} while (*p++);
							break;
						}
						++p;
						n += nx; /* Only gets here if cs->s[n] != 0, so nx is set. */
#endif /* WANT_WIDE */
					} while (1);
				} while (1);
			} else if (r == 0) {		/* illegal, undefined, or part of a range */
				if ((CUR_COLLATE->range_count)
					&& (((__uwchar_t)(WC - CUR_COLLATE->range_low)) <= CUR_COLLATE->range_count)
					) {					/* part of a range */
					/* Note: cs->colitem = 0 already. */
					TRACE(("    found range\n"));
					ru = CUR_COLLATE->ruletable[CUR_COLLATE->range_rule_offset*CUR_COLLATE->MAX_WEIGHTS + pass];
					assert((ru & WEIGHT_MASK) != DITTO_IDX);
					if ((ru & WEIGHT_MASK) == WEIGHT_MASK) {
						ru = (ru & RULE_MASK) | RANGE_IDX;
						cs->weight = CUR_COLLATE->range_base_weight + (WC - CUR_COLLATE->range_low);
					}
					goto RANGE_SKIP_TO;
				} else if (((__uwchar_t)(WC)) <= 0x7fffffffUL) { /* legal but undefined */
				UNDEFINED:
					/* Note: cs->colitem = 0 already. */
					ri = CUR_COLLATE->undefined_idx;
					assert(ri != 0); /* implicit undefined isn't supported */

					TRACE(("    found explicit UNDEFINED\n"));
					if (CUR_COLLATE->num_weights == 1) {
						TRACE(("    single weight UNDEFINED\n"));
						cs->weightidx = RANGE_IDX;
						cs->weight = ri;
						cs->s += n;
						goto PROCESS_WEIGHT;
					}

					ri = CUR_COLLATE->index2ruleidx[ri - 1];
					ru = CUR_COLLATE->ruletable[ri * CUR_COLLATE->MAX_WEIGHTS + pass];
					assert((ru & WEIGHT_MASK) != WEIGHT_MASK); /* TODO: handle ".." */
					if ((ru & WEIGHT_MASK) == DITTO_IDX) {
						cs->colitem = CUR_COLLATE->undefined_idx;
					}
					goto RANGE_SKIP_TO;
				} else {		/* illegal */
					TRACE(("    found illegal\n"));
					__set_errno(EINVAL);
					/* We put all illegals in the same equiv class with maximal weight,
					 * and ignore them after the first pass. */
					if (pass > 0) {
						cs->s += n;
						goto IGNORE_LOOP;
					}
					ru = (RULE_FORWARD | RANGE_IDX);
					cs->weight = 0xffffU;
					goto RANGE_SKIP_TO;
				}
			} else if (CUR_COLLATE->num_weights == 1) {
				TRACE(("    single weight\n"));
				cs->weightidx = RANGE_IDX;
				cs->weight = cs->colitem;
				cs->s += n;
				goto PROCESS_WEIGHT;
			} else {
				TRACE(("    normal\n"));
			}

			/* if we get here, it is a normal char either singlely weighted, undefined, or in a range */
		FOUND:
			ri = CUR_COLLATE->index2ruleidx[cs->colitem - 1];
			TRACE((" ri=%d ", ri));
			if (!ri) {
				TRACE(("NOT IN THIS LOCALE\n"));
				goto UNDEFINED;
			}
			ru = CUR_COLLATE->ruletable[ri * CUR_COLLATE->MAX_WEIGHTS + pass];

		RANGE_SKIP_TO:

/* 			if (!(ru & WEIGHT_MASK)) { */
/* 				TRACE(("IGNORE\n")); */
/* 				cs->s += n; */
/* 				continue; */
/* 			} */


			TRACE((" rule = %#x  weight = %#x  popping = %d  s = %p  eob = %p\n",
				   ru & RULE_MASK, ru & WEIGHT_MASK, popping_backup_stack,
				   cs->s, cs->eob));
			/* now we need to check if we're going backwards... */

			if (!popping_backup_stack) {
				if (!(ru & RULE_MASK)) { /* backward */
					TRACE(("backwards\n"));
					assert(cs->bp <= cs->bbe);
					if (cs->bp == cs->bbe) {
						if (cs->back_buf == cs->ibb) { /* was using internal buffer */
							cs->bp = malloc(cs->bb_size + 128);
							if (!cs->bp) {
								/* __set_errno(ENOMEM); */
								cs->weight = 0;
								return;
							}
							memcpy(cs->bp, cs->back_buf, cs->bb_size);

						} else {
							cs->bp = realloc(cs->back_buf, cs->bb_size + 128);
							if (!cs->bp) {
								/* __set_errno(ENOMEM); */
								cs->weight = 0;
								return;
							}
						}
						cs->bb_size += 128;
						cs->bbe = cs->bp + (cs->bbe - cs->back_buf);
						cs->back_buf = cs->bp;
						cs->bp = cs->bbe;

					}
					if (n==1) {			/* single char */
						if (*cs->bp && (((unsigned char)(*cs->bp)) < CHAR_MAX)) {
							*cs->bp += 1; /* increment last single's count */
						} else {	  /* last was a multi, or just starting */
							if (!cs->bp) {
								cs->bp = cs->back_buf;
							} else {
								assert(cs->bp < cs->bbe);
								++cs->bp;
							}
							*cs->bp = 1;
						}
					} else {			/* multichar */
						assert(n>1);
						assert(cs->bp < cs->bbe);
						*++cs->bp = -n;
					}
					cs->s += n;
					if (*cs->s) {
						goto BACK_LOOP;
					}
					/* end-of-string so start popping */
					cs->eob = cs->s;
					TRACE(("popping\n"));
					goto POP_BACKUP;
				} else if (*cs->bp) { /* was going backward but this element isn't */
					/* discard current and use previous backward element */
					assert(!cs->cip);
					cs->eob = cs->s;
					TRACE(("popping\n"));
					goto POP_BACKUP;
				} else {				/* was and still going forward */
					TRACE(("forwards\n"));
					if ((ru & (RULE_POSITION|WEIGHT_MASK)) > RULE_POSITION) {
						assert(ru & WEIGHT_MASK);
						cs->ru_pushed = ru;
						cs->weight = cs->position;
						cs->position = 0;	/* reset to reduce size for strcoll? */
						cs->s += n;
						cs->weightidx = RANGE_IDX;
						goto PROCESS_WEIGHT;
					}
				}
			} else {					/* popping backwards stack */
				TRACE(("popping (continued)\n"));
				if (!*cs->bp) {
					cs->s = cs->eob;
				}
				cs->s -= n;
			}

			cs->s += n;
		POSITION_SKIP:
			cs->weightidx = ru & WEIGHT_MASK;
			cs->rule = ru & RULE_MASK;
		}

		if (!cs->weightidx) {	/* ignore */
			continue;
		}

	PROCESS_WEIGHT:
		assert(cs->weightidx);


		if (((unsigned int)(cs->weightidx - UI_IDX)) <= (INVAL_IDX-UI_IDX)) {
			if (cs->weightidx == UI_IDX) {
				cs->weight = cs->ui_weight;
			}
			return;
		}

		assert(cs->weightidx != WEIGHT_MASK);
		if (cs->weightidx == DITTO_IDX) { /* want the weight of the current collating item */
			TRACE(("doing ditto\n"));
			w = CUR_COLLATE->index2weight[cs->colitem -1];
		} else if (cs->weightidx <= CUR_COLLATE->max_col_index) { /* normal */
			TRACE(("doing normal\n"));
			w = CUR_COLLATE->index2weight[cs->weightidx -1];
		} else {				/* a string */
			TRACE(("doing string\n"));
			assert(!(cs->weightidx & RULE_MASK));
			/* note: iso14561 allows null string here */
			p = CUR_COLLATE->weightstr + (cs->weightidx - (CUR_COLLATE->max_col_index + 2));
			if (*p & WEIGHT_MASK) {
				r = 0;
				do {
					assert(r < MAX_PENDING);
					cs->ci_pending[r++] = *p++;
				} while (*p & WEIGHT_MASK);
				cs->cip = cs->ci_pending;
			}
			continue;
		}

		cs->weight = w;
		return;
	} while (1);
}

int __XL_NPP(wcscoll) (const Wchar *s0, const Wchar *s1   __LOCALE_PARAM )
{
	col_state_t ws[2];
	int pass;

	if (!CUR_COLLATE->num_weights) { /* C locale */
#ifdef WANT_WIDE
		return wcscmp(s0, s1);
#else
		return strcmp(s0, s1);
#endif
	}

	pass = 0;
	do {						/* loop through the weights levels */
		init_col_state(ws, s0);
		init_col_state(ws+1, s1);
		do {					/* loop through the strings */
			/* for each string, get the next weight */
			next_weight(ws, pass   __LOCALE_ARG );
			next_weight(ws+1, pass   __LOCALE_ARG );
			TRACE(("w0=%lu  w1=%lu\n",
				   (unsigned long) ws[0].weight,
				   (unsigned long) ws[1].weight));

			if (ws[0].weight != ws[1].weight) {
				return ws[0].weight - ws[1].weight;
			}
		} while (ws[0].weight);
	} while (++pass < CUR_COLLATE->num_weights);

	return 0;
}
libc_hidden_def(__XL_NPP(wcscoll))

#ifdef WANT_WIDE

size_t __XL_NPP(wcsxfrm)(wchar_t *__restrict ws1, const wchar_t *__restrict ws2,
					 size_t n   __LOCALE_PARAM )
{
	col_state_t cs;
	size_t count;
	int pass;

	if (!CUR_COLLATE->num_weights) { /* C locale */
		return __wcslcpy(ws1, ws2, n);
	}

	count = pass = 0;
	do {						/* loop through the weights levels */
		init_col_state(&cs, ws2);
		do {					/* loop through the string */
			next_weight(&cs, pass   __LOCALE_ARG );
			TRACE(("weight=%lu (%#lx)\n", (unsigned long) cs.weight, (unsigned long) cs.weight));
			if (count < n) {
				ws1[count] = cs.weight +1;
			}
			++count;
			TRACE(("--------------------------------------------\n"));
		} while (cs.weight);
		if (count <= n) {		/* overwrite the trailing 0 end-of-pass marker */
			ws1[count-1] = 1;
		}
		TRACE(("--------------------  pass %d  --------------------\n", pass));
	} while (++pass < CUR_COLLATE->num_weights);
	if (count <= n) {			/* oops... change it back */
		ws1[count-1] = 0;
	}
	return count-1;
}
#if defined L_strxfrm_l || defined L_wcsxfrm_l
libc_hidden_def(__XL_NPP(wcsxfrm))
#endif

#else  /* WANT_WIDE */

static const unsigned long bound[] = {
	1UL << 7,
	1UL << 11,
	1UL << 16,
	1UL << 21,
	1UL << 26,
};

static unsigned char first[] = {
	0x0, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc
};

/* Use an extension of UTF-8 to store a 32 bit val in max 6 bytes. */

static size_t store(unsigned char *s, size_t count, size_t n, __uwchar_t weight)
{
	int i, r;

	i = 0;
	do {
		if (weight < bound[i]) {
			break;
		}
	} while (++i < sizeof(bound)/sizeof(bound[0]));

	r = i+1;
	if (i + count < n) {
		s += count;
		s[0] = first[i];
		while (i) {
			s[i] = 0x80 | (weight & 0x3f);
			weight >>= 6;
			--i;
		}
		s[0] |= weight;
	}

	return r;
}

size_t __XL_NPP(strxfrm)(char *__restrict ws1, const char *__restrict ws2, size_t n
					 __LOCALE_PARAM )
{
	col_state_t cs;
	size_t count, inc;
	int pass;

	if (!CUR_COLLATE->num_weights) { /* C locale */
		return strlcpy(ws1, ws2, n);
	}

	inc = count = pass = 0;
	do {						/* loop through the weights levels */
		init_col_state(&cs, ws2);
		do {					/* loop through the string */
			next_weight(&cs, pass   __LOCALE_ARG );
			TRACE(("weight=%lu (%#lx)\n", (unsigned long) cs.weight, (unsigned long) cs.weight));
			inc = store((unsigned char *)ws1, count, n, cs.weight + 1);
			count += inc;
			TRACE(("--------------------------------------------\n"));
		} while (cs.weight);
		/* overwrite the trailing 0 end-of-pass marker */
		assert(inc == 1);
		if (count <= n) {
			ws1[count-1] = 1;
		}
		TRACE(("--------------------  pass %d  --------------------\n", pass));
	} while (++pass < CUR_COLLATE->num_weights);
	if (count <= n) {			/* oops... change it back */
		ws1[count-1] = 0;
	}
	return count-1;
}
#ifdef L_strxfrm_l
libc_hidden_def(__XL_NPP(strxfrm))
#endif

#endif /* WANT_WIDE */

#endif /* defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE) */

#endif /* defined(L_strxfrm) || defined(L_strxfrm_l) || defined(L_wcsxfrm) || defined(L_wcsxfrm_l) */

#endif /* __UCLIBC_HAS_LOCALE__ */
/**********************************************************************/
