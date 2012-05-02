
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <wctype.h>
#include <limits.h>
#include <stdint.h>
#include <wchar.h>
#include <ctype.h>

#ifdef __linux__
#include <sys/resource.h>
#endif

#ifndef _CTYPE_H
#define _CTYPE_H
#endif
#ifndef _WCTYPE_H
#define _WCTYPE_H
#endif
#include "../../libc/sysdeps/linux/common/bits/uClibc_ctype.h"

/*       0x9 : space  blank */
/*       0xa : space */
/*       0xb : space */
/*       0xc : space */
/*       0xd : space */
/*      0x20 : space  blank */
/*    0x1680 : space  blank */
/*    0x2000 : space  blank */
/*    0x2001 : space  blank */
/*    0x2002 : space  blank */
/*    0x2003 : space  blank */
/*    0x2004 : space  blank */
/*    0x2005 : space  blank */
/*    0x2006 : space  blank */
/*    0x2008 : space  blank */
/*    0x2009 : space  blank */
/*    0x200a : space  blank */
/*    0x200b : space  blank */
/*    0x2028 : space */
/*    0x2029 : space */
/*    0x3000 : space  blank */

/*  typecount[ 0] =    88670  C_alpha_nonupper_nonlower */
/*  typecount[ 1] =      742  C_alpha_lower */
/*  typecount[ 2] =        4  C_alpha_upper_lower */
/*  typecount[ 3] =      731  C_alpha_upper */
/*  typecount[ 4] =       10  C_digit */
/*  typecount[ 5] =    10270  C_punct */
/*  typecount[ 6] =        0  C_graph */
/*  typecount[ 7] =        0  C_print_space_nonblank */
/*  typecount[ 8] =       14  C_print_space_blank */
/*  typecount[ 9] =        0  C_space_nonblank_noncntrl */
/*  typecount[10] =        0  C_space_blank_noncntrl */
/*  typecount[11] =        6  C_cntrl_space_nonblank */
/*  typecount[12] =        1  C_cntrl_space_blank */
/*  typecount[13] =       60  C_cntrl_nonspace */
/*  typecount[14] =    96100  C_unclassified */
/*  typecount[15] =        0  empty_slot */



/* Set to #if 0 to restrict wchars to 16 bits. */
#if 1
#define RANGE 0x2ffffUL
#elif 0
#define RANGE 0x1ffffUL
#else
#define RANGE 0xffffUL			/* Restrict for 16-bit wchar_t... */
#endif

#if 0
/* Classification codes. */

static const char *typename[] = {
	"C_unclassified",
	"C_alpha_nonupper_nonlower",
	"C_alpha_lower",
	"C_alpha_upper_lower",
	"C_alpha_upper",
	"C_digit",
	"C_punct",
	"C_graph",
	"C_print_space_nonblank",
	"C_print_space_blank",
	"C_space_nonblank_noncntrl",
	"C_space_blank_noncntrl",
	"C_cntrl_space_nonblank",
	"C_cntrl_space_blank",
	"C_cntrl_nonspace",
	"empty_slot"
};
#endif

#if 0
/* Taking advantage of the C99 mutual-exclusion guarantees for the various
 * (w)ctype classes, including the descriptions of printing and control
 * (w)chars, we can place each in one of the following mutually-exlusive
 * subsets.  Since there are less than 16, we can store the data for
 * each (w)chars in a nibble. In contrast, glibc uses an unsigned int
 * per (w)char, with one bit flag for each is* type.  While this allows
 * a simple '&' operation to determine the type vs. a range test and a
 * little special handling for the "blank" and "xdigit" types in my
 * approach, it also uses 8 times the space for the tables on the typical
 * 32-bit archs we supported.*/
enum {
	__CTYPE_unclassified = 0,
	__CTYPE_alpha_nonupper_nonlower,
	__CTYPE_alpha_lower,
	__CTYPE_alpha_upper_lower,
	__CTYPE_alpha_upper,
	__CTYPE_digit,
	__CTYPE_punct,
	__CTYPE_graph,
	__CTYPE_print_space_nonblank,
	__CTYPE_print_space_blank,
	__CTYPE_space_nonblank_noncntrl,
	__CTYPE_space_blank_noncntrl,
	__CTYPE_cntrl_space_nonblank,
	__CTYPE_cntrl_space_blank,
	__CTYPE_cntrl_nonspace,
};
#endif

#define __CTYPE_isxdigit(D,X) \
	(__CTYPE_isdigit(D) || (((unsigned int)(((X)|0x20) - 'a')) <= 5))

#define mywalnum(x)		__CTYPE_isalnum(d)
#define mywalpha(x)		__CTYPE_isalpha(d)
#define mywblank(x) 	__CTYPE_isblank(d)
#define mywcntrl(x)		__CTYPE_iscntrl(d)
#define mywdigit(x)		__CTYPE_isdigit(d)
#define mywgraph(x)		__CTYPE_isgraph(d)
#define mywlower(x)		__CTYPE_islower(d)
#define mywprint(x)		__CTYPE_isprint(d)
#define mywpunct(x)		__CTYPE_ispunct(d)
#define mywspace(x)		__CTYPE_isspace(d)
#define mywupper(x)		__CTYPE_isupper(d)
#define mywxdigit(x)	__CTYPE_isxdigit(d,x)

typedef struct {
	short l;
	short u;
} uldiff_entry;

typedef struct {
	uint16_t ii_len;
	uint16_t ti_len;
	uint16_t ut_len;

	unsigned char ii_shift;
	unsigned char ti_shift;

	unsigned char *ii;
	unsigned char *ti;
	unsigned char *ut;
} table_data;


void output_table(FILE *fp, const char *name, table_data *tbl)
{
	size_t i;

	fprintf(fp, "#define __LOCALE_DATA_WC%s_II_LEN    %7u\n", name, tbl->ii_len);
	fprintf(fp, "#define __LOCALE_DATA_WC%s_TI_LEN    %7u\n", name, tbl->ti_len);
	fprintf(fp, "#define __LOCALE_DATA_WC%s_UT_LEN    %7u\n", name, tbl->ut_len);

	fprintf(fp, "#define __LOCALE_DATA_WC%s_II_SHIFT  %7u\n", name, tbl->ii_shift);
	fprintf(fp, "#define __LOCALE_DATA_WC%s_TI_SHIFT  %7u\n", name, tbl->ti_shift);

	fprintf(fp, "\n#ifdef WANT_WC%s_data\n", name);

	i = tbl->ii_len + tbl->ti_len + tbl->ut_len;
	fprintf(fp, "\nstatic const unsigned char __LOCALE_DATA_WC%s_data[%zu] = {", name, i);
	for (i=0 ; i < tbl->ii_len ; i++) {
		if (i % 12 == 0) {
			fprintf(fp, "\n");
		}
		fprintf(fp, " %#04x,", tbl->ii[i]);
	}
	for (i=0 ; i < tbl->ti_len ; i++) {
		if (i % 12 == 0) {
			fprintf(fp, "\n");
		}
		fprintf(fp, " %#04x,", tbl->ti[i]);
	}
	for (i=0 ; i < tbl->ut_len ; i++) {
		if (i % 12 == 0) {
			fprintf(fp, "\n");
		}
		fprintf(fp, " %#04x,", tbl->ut[i]);
	}
	fprintf(fp, "\n};\n\n");

	fprintf(fp, "#endif /* WANT_WC%s_data */\n\n", name);
}

static void dump_table_data(table_data *tbl)
{
	printf("ii_shift = %d  ti_shift = %d\n"
		   "ii_len = %d  ti_len = %d  ut_len = %d\n"
		   "total = %d\n",
		   tbl->ii_shift, tbl->ti_shift,
		   tbl->ii_len, tbl->ti_len, tbl->ut_len,
		   (int) tbl->ii_len + (int) tbl->ti_len + (int) tbl->ut_len);
}

/* For sorting the blocks of unsigned chars. */
static size_t nu_val;

int nu_memcmp(const void *a, const void *b)
{
	return memcmp(*(unsigned char**)a, *(unsigned char**)b, nu_val);
}

static size_t newopt(unsigned char *ut, size_t usize, int shift, table_data *tbl);

#define MAXTO		255			/* Restrict to minimal unsigned char max. */

int main(int argc, char **argv)
{
	long int u, l, tt;
	size_t smallest, t;
	unsigned int c;
	unsigned int d;
	int i, n;
	int ul_count = 0;
	uldiff_entry uldiff[MAXTO];
	table_data cttable;
	table_data ultable;
	table_data combtable;
	table_data widthtable;
	long int last_comb = 0;

	unsigned char wct[(RANGE/2)+1];	/* wctype table (nibble per wchar) */
	unsigned char ult[RANGE+1];	/* upper/lower table */
	unsigned char combt[(RANGE/4)+1];	/* combining */
	unsigned char widtht[(RANGE/4)+1];	/* width */
	wctrans_t totitle;
	wctype_t is_comb, is_comb3;

	long int typecount[16];
	const char *typename[16];
	static const char empty_slot[] = "empty_slot";
	int built = 0;

#ifdef __linux__
	struct rlimit limit;

	limit.rlim_max = RLIM_INFINITY;
	limit.rlim_cur = RLIM_INFINITY;
	setrlimit(RLIMIT_STACK, &limit);
#endif

#define INIT_TYPENAME(X) typename[__CTYPE_##X] = "C_" #X

	for (i=0 ; i < 16 ; i++) {
		typename[i] = empty_slot;
	}

	INIT_TYPENAME(unclassified);
	INIT_TYPENAME(alpha_nonupper_nonlower);
	INIT_TYPENAME(alpha_lower);
	INIT_TYPENAME(alpha_upper_lower);
	INIT_TYPENAME(alpha_upper);
	INIT_TYPENAME(digit);
	INIT_TYPENAME(punct);
	INIT_TYPENAME(graph);
	INIT_TYPENAME(print_space_nonblank);
	INIT_TYPENAME(print_space_blank);
	INIT_TYPENAME(space_nonblank_noncntrl);
	INIT_TYPENAME(space_blank_noncntrl);
	INIT_TYPENAME(cntrl_space_nonblank);
	INIT_TYPENAME(cntrl_space_blank);
	INIT_TYPENAME(cntrl_nonspace);

	setvbuf(stdout, NULL, _IONBF, 0);

	while (--argc) {
		if (!setlocale(LC_CTYPE, *++argv)) {
			printf("setlocale(LC_CTYPE,%s) failed!\n", *argv);
			continue;
		}

		if (!(totitle = wctrans("totitle"))) {
			printf("no totitle transformation.\n");
		}
		if (!(is_comb = wctype("combining"))) {
			printf("no combining wctype.\n");
		}
		if (!(is_comb3 = wctype("combining_level3"))) {
			printf("no combining_level3 wctype.\n");
		}

		if (!built) {
		built = 1;
		ul_count = 1;
		uldiff[0].u = uldiff[0].l = 0;

		memset(wct, 0, sizeof(wct));
		memset(combt, 0, sizeof(combt));
		memset(widtht, 0, sizeof(widtht));

		for (i = 0 ; i < 16 ; i++) {
			typecount[i] = 0;
		}

		for (c=0 ; c <= RANGE ; c++) {
			if (iswdigit(c)) {
				d = __CTYPE_digit;
			} else if (iswalpha(c)) {
				d = __CTYPE_alpha_nonupper_nonlower;
				if (iswlower(c)) {
					d = __CTYPE_alpha_lower;
					if (iswupper(c)) {
						d = __CTYPE_alpha_upper_lower;
					}
				} else if (iswupper(c)) {
					d = __CTYPE_alpha_upper;
				}
			} else if (iswpunct(c)) {
				d = __CTYPE_punct;
			} else if (iswgraph(c)) {
				d = __CTYPE_graph;
			} else if (iswprint(c)) {
				d = __CTYPE_print_space_nonblank;
				if (iswblank(c)) {
					d = __CTYPE_print_space_blank;
				}
			} else if (iswspace(c) && !iswcntrl(c)) {
				d = __CTYPE_space_nonblank_noncntrl;
				if (iswblank(c)) {
					d = __CTYPE_space_blank_noncntrl;
				}
			} else if (iswcntrl(c)) {
				d = __CTYPE_cntrl_nonspace;
				if (iswspace(c)) {
					d = __CTYPE_cntrl_space_nonblank;
					if (iswblank(c)) {
						d = __CTYPE_cntrl_space_blank;
					}
				}
			} else {
				d = __CTYPE_unclassified;
			}

			++typecount[d];

#if 0
			if (iswspace(c)) {
				if (iswblank(c)) {
					printf("%#8x : space  blank\n", c);
				} else {
					printf("%#8x : space\n", c);
				}
			}
#endif

#if 0
			if (c < 256) {
				unsigned int glibc;

				glibc = 0;
				if (isalnum(c)) ++glibc; glibc <<= 1;
				if (isalpha(c)) ++glibc; glibc <<= 1;
				if (isblank(c)) ++glibc; glibc <<= 1;
				if (iscntrl(c)) ++glibc; glibc <<= 1;
				if (isdigit(c)) ++glibc; glibc <<= 1;
				if (isgraph(c)) ++glibc; glibc <<= 1;
				if (islower(c)) ++glibc; glibc <<= 1;
				if (isprint(c)) ++glibc; glibc <<= 1;
				if (ispunct(c)) ++glibc; glibc <<= 1;
				if (isspace(c)) ++glibc; glibc <<= 1;
				if (isupper(c)) ++glibc; glibc <<= 1;
				if (isxdigit(c)) ++glibc;
				printf("%#8x : ctype %#4x\n", c, glibc);
			}
#endif

#if 1
			/* Paranoid checking... */
			{
				unsigned int glibc;
				unsigned int mine;

				glibc = 0;
				if (iswalnum(c)) ++glibc; glibc <<= 1;
				if (iswalpha(c)) ++glibc; glibc <<= 1;
				if (iswblank(c)) ++glibc; glibc <<= 1;
				if (iswcntrl(c)) ++glibc; glibc <<= 1;
				if (iswdigit(c)) ++glibc; glibc <<= 1;
				if (iswgraph(c)) ++glibc; glibc <<= 1;
				if (iswlower(c)) ++glibc; glibc <<= 1;
				if (iswprint(c)) ++glibc; glibc <<= 1;
				if (iswpunct(c)) ++glibc; glibc <<= 1;
				if (iswspace(c)) ++glibc; glibc <<= 1;
				if (iswupper(c)) ++glibc; glibc <<= 1;
				if (iswxdigit(c)) ++glibc;

				mine = 0;
				if (mywalnum(c)) ++mine; mine <<= 1;
				if (mywalpha(c)) ++mine; mine <<= 1;
				if (mywblank(c)) ++mine; mine <<= 1;
				if (mywcntrl(c)) ++mine; mine <<= 1;
				if (mywdigit(c)) ++mine; mine <<= 1;
				if (mywgraph(c)) ++mine; mine <<= 1;
				if (mywlower(c)) ++mine; mine <<= 1;
				if (mywprint(c)) ++mine; mine <<= 1;
				if (mywpunct(c)) ++mine; mine <<= 1;
				if (mywspace(c)) ++mine; mine <<= 1;
				if (mywupper(c)) ++mine; mine <<= 1;
				if (mywxdigit(c)) ++mine;

				if (glibc != mine) {
					printf("%#8x : glibc %#4x != %#4x mine  %u\n", c, glibc, mine, d);
					return EXIT_FAILURE;
				}

#if 0
				if (iswctype(c,is_comb) || iswctype(c,is_comb3)) {
/*  					if (!iswpunct(c)) { */
						printf("%#8x : %d %d %#4x\n",
							   c, iswctype(c,is_comb),iswctype(c,is_comb3), glibc);
/*  					} */
				}
#endif
#if 0
				if (iswctype(c,is_comb) || iswctype(c,is_comb3)) {
					if (!last_comb) {
						printf("%#8x - ", c);
						last_comb = c;
					} else if (last_comb + 1 < c) {
						printf("%#8x\n%#8x - ", last_comb, c);
						last_comb = c;
					} else {
						last_comb = c;
					}
				}
#endif
			}
#endif

			combt[c/4] |= ((((!!iswctype(c,is_comb)) << 1) | !!iswctype(c,is_comb3))
						   << ((c & 3) << 1));
/*  			comb3t[c/8] |= ((!!iswctype(c,is_comb3)) << (c & 7)); */

/* 			widtht[c/4] |= (wcwidth(c) << ((c & 3) << 1)); */

			if (c & 1) {	/* Use the high nibble for odd numbered wchars. */
				d <<= 4;
			}
			wct[c/2] |= d;

			l = (long)(int) towlower(c) - c;
			u = (long)(int) towupper(c) - c;
			ult[c] = 0;
			if (l || u) {
				if ((l != (short)l) || (u != (short)u)) {
					printf("range assumption error!  %x  %ld  %ld\n", c, l, u);
					return EXIT_FAILURE;
				}
				for (i=0 ; i < ul_count ; i++) {
					if ((l == uldiff[i].l) && (u == uldiff[i].u)) {
						goto found;
					}
				}
				uldiff[ul_count].l = l;
				uldiff[ul_count].u = u;
				++ul_count;
				if (ul_count > MAXTO) {
					printf("too many touppers/tolowers!\n");
					return EXIT_FAILURE;
				}
			found:
				ult[c] = i;
			}
		}

		for (i = 0 ; i < 16 ; i++) {
			printf("typecount[%2d] = %8ld  %s\n", i, typecount[i], typename[i]);
		}

		printf("optimizing is* table..\n");
		n = -1;
		smallest = SIZE_MAX;
		cttable.ii = NULL;
		for (i=0 ; i < 14 ; i++) {
			t = newopt(wct, (RANGE/2)+1, i, &cttable);
			if (smallest >= t) {
				n = i;
				smallest = t;
/*  			} else { */
/*  				break; */
			}
		}
		printf("smallest = %zu\n", smallest);
		if (!(cttable.ii = malloc(smallest))) {
			printf("couldn't allocate space!\n");
			return EXIT_FAILURE;
		}
		smallest = SIZE_MAX;
		newopt(wct, (RANGE/2)+1, n, &cttable);
		++cttable.ti_shift;		/* correct for nibble mode */



		printf("optimizing u/l-to table..\n");
		smallest = SIZE_MAX;
		ultable.ii = NULL;
		for (i=0 ; i < 14 ; i++) {
			t = newopt(ult, RANGE+1, i, &ultable);
			if (smallest >= t) {
				n = i;
				smallest = t;
/*  			} else { */
/*  				break; */
			}
		}
		printf("%zu (smallest) + %zu (u/l diffs) = %zu\n",
			   smallest, 4 * ul_count, smallest + 4 * ul_count);
		printf("smallest = %zu\n", smallest);
		if (!(ultable.ii = malloc(smallest))) {
			printf("couldn't allocate space!\n");
			return EXIT_FAILURE;
		}
		smallest = SIZE_MAX;
		newopt(ult, RANGE+1, n, &ultable);


#if 0
		printf("optimizing comb table..\n");
		smallest = SIZE_MAX;
		combtable.ii = NULL;
		for (i=0 ; i < 14 ; i++) {
			t = newopt(combt, sizeof(combt), i, &combtable);
			if (smallest >= t) {
				n = i;
				smallest = t;
/*  			} else { */
/*  				break; */
			}
		}
		printf("smallest = %zu\n", smallest);
		if (!(combtable.ii = malloc(smallest))) {
			printf("couldn't allocate space!\n");
			return EXIT_FAILURE;
		}
		smallest = SIZE_MAX;
		newopt(combt, sizeof(combt), n, &combtable);
		combtable.ti_shift += 4; /* correct for 4 entries per */
#endif


#if 0
		printf("optimizing width table..\n");
		smallest = SIZE_MAX;
		widthtable.ii = NULL;
		for (i=0 ; i < 14 ; i++) {
			t = newopt(widtht, sizeof(widtht), i, &widthtable);
			if (smallest >= t) {
				n = i;
				smallest = t;
/*  			} else { */
/*  				break; */
			}
		}
		printf("smallest = %zu\n", smallest);
		if (!(widthtable.ii = malloc(smallest))) {
			printf("couldn't allocate space!\n");
			return EXIT_FAILURE;
		}
		smallest = SIZE_MAX;
		newopt(widtht, sizeof(widtht), n, &widthtable);
		widthtable.ti_shift += 4; /* correct for 4 entries per */
#endif

#if 0
		printf("optimizing comb3 table..\n");
		smallest = SIZE_MAX;
		comb3table.ii = NULL;
		for (i=0 ; i < 14 ; i++) {
			t = newopt(comb3t, sizeof(comb3t), i, &comb3table);
			if (smallest >= t) {
				n = i;
				smallest = t;
/*  			} else { */
/*  				break; */
			}
		}
		printf("smallest = %zu\n", smallest);
		if (!(comb3table.ii = malloc(smallest))) {
			printf("couldn't allocate space!\n");
			return EXIT_FAILURE;
		}
		smallest = SIZE_MAX;
		newopt(comb3t, sizeof(comb3t), n, &comb3table);
		comb3table.ti_shift += 8; /* correct for 4 entries per */
#endif

		dump_table_data(&cttable);
		dump_table_data(&ultable);
		dump_table_data(&combtable);
		}

		printf("verifying for %s...\n", *argv);
#if RANGE == 0xffffU
		for (c=0 ; c <= 0xffffUL ; c++)
#else
		for (c=0 ; c <= 0x10ffffUL ; c++)
#endif
			{
			unsigned int glibc;
			unsigned int mine;
			unsigned int upper, lower;

#if 0
#if RANGE < 0x10000UL
			if (c == 0x10000UL) {
				c = 0x30000UL;	/* skip 1st and 2nd sup planes */
			}
#elif RANGE < 0x20000UL
			if (c == 0x20000UL) {
				c = 0x30000UL;	/* skip 2nd sup planes */
			}
#endif
#endif

			glibc = 0;
			if (iswalnum(c)) ++glibc; glibc <<= 1;
			if (iswalpha(c)) ++glibc; glibc <<= 1;
			if (iswblank(c)) ++glibc; glibc <<= 1;
			if (iswcntrl(c)) ++glibc; glibc <<= 1;
			if (iswdigit(c)) ++glibc; glibc <<= 1;
			if (iswgraph(c)) ++glibc; glibc <<= 1;
			if (iswlower(c)) ++glibc; glibc <<= 1;
			if (iswprint(c)) ++glibc; glibc <<= 1;
			if (iswpunct(c)) ++glibc; glibc <<= 1;
			if (iswspace(c)) ++glibc; glibc <<= 1;
			if (iswupper(c)) ++glibc; glibc <<= 1;
			if (iswxdigit(c)) ++glibc;

			{
				unsigned int u;
				int n, sc;
				int i0, i1;

				u = c;
				if (u <= RANGE) {
					sc = u & ((1 << cttable.ti_shift) - 1);
					u >>= cttable.ti_shift;
					n = u & ((1 << cttable.ii_shift) - 1);
					u >>= cttable.ii_shift;

					i0 = cttable.ii[u];
					i0 <<= cttable.ii_shift;
					i1 = cttable.ti[i0 + n];
					i1 <<= (cttable.ti_shift-1);
					d = cttable.ut[i1 + (sc >> 1)];

					if (sc & 1) {
						d >>= 4;
					}
					d &= 0x0f;
				} else if ((((unsigned int)(c - 0xe0020UL)) <= 0x5f) || (c == 0xe0001UL)){
					d = __CTYPE_punct;
				} else if (((unsigned int)(c - 0xf0000UL)) < 0x20000UL) {
					if ((c & 0xffffU) <= 0xfffdU) {
						d = __CTYPE_punct;
					} else {
						d = __CTYPE_unclassified;
					}
				} else {
					d = __CTYPE_unclassified;
				}

			mine = 0;
			if (mywalnum(c)) ++mine; mine <<= 1;
			if (mywalpha(c)) ++mine; mine <<= 1;
			if (mywblank(c)) ++mine; mine <<= 1;
			if (mywcntrl(c)) ++mine; mine <<= 1;
			if (mywdigit(c)) ++mine; mine <<= 1;
			if (mywgraph(c)) ++mine; mine <<= 1;
			if (mywlower(c)) ++mine; mine <<= 1;
			if (mywprint(c)) ++mine; mine <<= 1;
			if (mywpunct(c)) ++mine; mine <<= 1;
			if (mywspace(c)) ++mine; mine <<= 1;
			if (mywupper(c)) ++mine; mine <<= 1;
			if (mywxdigit(c)) ++mine;

			if (glibc != mine) {
				printf("%#8x : glibc %#4x != %#4x mine %d\n", c, glibc, mine, d);
				if (c < 0x30000UL) {
					printf("sc=%#x u=%#x n=%#x i0=%#x i1=%#x\n", sc, u, n, i0, i1);
				}
			}
				upper = lower = u = c;
				if (u <= RANGE) {
					sc = u & ((1 << ultable.ti_shift) - 1);
					u >>= ultable.ti_shift;
					n = u & ((1 << ultable.ii_shift) - 1);
					u >>= ultable.ii_shift;

					i0 = ultable.ii[u];
					i0 <<= ultable.ii_shift;
					i1 = ultable.ti[i0 + n];
					i1 <<= (ultable.ti_shift);
					i1 += sc;
					i0 = ultable.ut[i1];
					upper = c + uldiff[i0].u;
					lower = c + uldiff[i0].l;
				}

			if (towupper(c) != upper) {
				printf("%#8x : towupper glibc %#4x != %#4x mine\n",
					   c, towupper(c), upper);
			}
				
			if (towlower(c) != lower) {
				printf("%#8x : towlower glibc %#4x != %#4x mine   i0 = %d\n",
					   c, towlower(c), lower, i0);
			}

			if (totitle && ((tt = towctrans(c, totitle)) != upper)) {
				printf("%#8x : totitle glibc %#4lx != %#4x mine   i0 = %d\n",
					   c, tt, upper, i0);
			}
			}


			if ((c & 0xfff) == 0xfff) printf(".");
		}
		printf("done\n");
	}

	if (1) {
		FILE *fp;

		if (!(fp = fopen("wctables.h", "w"))) {
			printf("couldn't open wctables.h!\n");
			return EXIT_FAILURE;
		}

		fprintf(fp, "#define __LOCALE_DATA_WC_TABLE_DOMAIN_MAX  %#8lx\n\n",
				(unsigned long) RANGE);
		output_table(fp, "ctype", &cttable);
		output_table(fp, "uplow", &ultable);
	

#warning fix the upper bound on the upper/lower tables... save 200 bytes or so
		fprintf(fp, "#define __LOCALE_DATA_WCuplow_diffs  %7u\n", ul_count);
		fprintf(fp, "\n#ifdef WANT_WCuplow_diff_data\n\n");
		fprintf(fp, "\nstatic const short __LOCALE_DATA_WCuplow_diff_data[%zu] = {",
			   2 * (size_t) ul_count);
		for (i=0 ; i < ul_count ; i++) {
			if (i % 4 == 0) {
				fprintf(fp, "\n");
			}
			fprintf(fp, " %6d, %6d,", uldiff[i].u, uldiff[i].l);
		}
		fprintf(fp, "\n};\n\n");
		fprintf(fp, "#endif /* WANT_WCuplow_diff_data */\n\n");


/* 		output_table(fp, "comb", &combtable); */
/* 		output_table(fp, "width", &widthtable); */

		fclose(fp);
	}

	return EXIT_SUCCESS;
}

size_t newopt(unsigned char *ut, size_t usize, int shift, table_data *tbl)
{
	static int recurse = 0;
	unsigned char *ti[RANGE+1];	/* table index */
	size_t numblocks;
	size_t blocksize;
	size_t uniq;
	size_t i, j;
	size_t smallest, t;
	unsigned char *ii_save;
	int uniqblock[256];
	unsigned char uit[RANGE+1];
	int shift2;

	ii_save = NULL;
	blocksize = 1 << shift;
	numblocks = usize >> shift;

	/* init table index */
	for (i=j=0 ; i < numblocks ; i++) {
		ti[i] = ut + j;
		j += blocksize;
	}

	/* sort */
	nu_val = blocksize;
	qsort(ti, numblocks, sizeof(unsigned char *), nu_memcmp);
	
	uniq = 1;
	uit[(ti[0]-ut)/blocksize] = 0;
	for (i=1 ; i < numblocks ; i++) {
		if (memcmp(ti[i-1], ti[i], blocksize) < 0) {
			if (++uniq > 255) {
				break;
			}
			uniqblock[uniq - 1] = i;
		}
#if 1
		else if (memcmp(ti[i-1], ti[i], blocksize) > 0) {
			printf("bad sort %i!\n", i);
			abort();
		}
#endif
		uit[(ti[i]-ut)/blocksize] = uniq - 1;
	}

	smallest = SIZE_MAX;
	shift2 = -1;
	if (uniq <= 255) {
		smallest = numblocks + uniq * blocksize;
		if (!recurse) {
			++recurse;
			for (j=1 ; j < 14 ; j++) {
				if ((numblocks >> j) < 2) break;
				if (tbl) {
					ii_save = tbl->ii;
					tbl->ii = NULL;
				}
				if ((t = newopt(uit, numblocks, j, tbl)) < SIZE_MAX) {
					t += uniq * blocksize;
				}
				if (tbl) {
					tbl->ii = ii_save;
				}
				if (smallest >= t) {
					shift2 = j;
					smallest = t;
					if (!tbl->ii) {
						printf("ishift %zu  tshift %zu  size %zu\n",
							   shift2, shift, t);
					}
/*  				} else { */
/*  					break; */
				}
			}
			--recurse;
		}
	} else {
		return SIZE_MAX;
	}

	if (tbl->ii) {
		if (recurse) {
			tbl->ii_shift = shift;
			tbl->ii_len = numblocks;
			memcpy(tbl->ii, uit, numblocks);
			tbl->ti = tbl->ii + tbl->ii_len;
			tbl->ti_len = uniq * blocksize;
			for (i=0 ; i < uniq ; i++) {
				memcpy(tbl->ti + i * blocksize, ti[uniqblock[i]], blocksize);
			}
		} else {
			++recurse;
			printf("setting ishift %zu  tshift %zu\n",
							   shift2, shift);
			newopt(uit, numblocks, shift2, tbl);
			--recurse;
			tbl->ti_shift = shift;
			tbl->ut_len = uniq * blocksize;
			tbl->ut = tbl->ti + tbl->ti_len;
			for (i=0 ; i < uniq ; i++) {
				memcpy(tbl->ut + i * blocksize, ti[uniqblock[i]], blocksize);
			}
		}
	}
	return smallest;
}
