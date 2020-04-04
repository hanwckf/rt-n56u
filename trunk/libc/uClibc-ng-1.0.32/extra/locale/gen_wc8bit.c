/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <stddef.h>
#include <wctype.h>
#include <limits.h>

#ifndef _CTYPE_H
#define _CTYPE_H
#endif
#ifndef _WCTYPE_H
#define _WCTYPE_H
#endif
#include "include/bits/uClibc_ctype.h"

/* TODO: maybe support -v like gen_wctype.c */
#define verbose_msg(msg...) if (verbose) fprintf(stderr, msg)

/*  #define CTYPE_PACKED */
#define UPLOW_IDX_SHIFT		3
/* best if 2 unpacked or 3 packed */
#define CTYPE_IDX_SHIFT		3
/* 3 or 4 are very similar */
#define C2WC_IDX_SHIFT		3

#define CTYPE_IDX_LEN		(128 >> (CTYPE_IDX_SHIFT))
#define UPLOW_IDX_LEN		(128 >> (UPLOW_IDX_SHIFT))
#define C2WC_IDX_LEN		(128 >> (C2WC_IDX_SHIFT))

/*  #ifdef CTYPE_PACKED */
/*  #define CTYPE_ROW_LEN		(1 << ((CTYPE_IDX_SHIFT)-1)) */
/*  #else */
#define CTYPE_ROW_LEN		(1 << (CTYPE_IDX_SHIFT))
/*  #endif */
#define UPLOW_ROW_LEN		(1 << (UPLOW_IDX_SHIFT))
#define C2WC_ROW_LEN		(1 << (C2WC_IDX_SHIFT))



#define MAX_WCHAR	(0x2600-1)

static unsigned char ctype_tbl[256 * CTYPE_ROW_LEN];
static unsigned char uplow_tbl[256 * UPLOW_ROW_LEN];
#ifdef DO_WIDE_CHAR
static unsigned short c2wc_tbl[256 * C2WC_ROW_LEN];
#endif
static unsigned char tt[MAX_WCHAR+1];
static unsigned char ti[MAX_WCHAR+1];
static unsigned char xi[MAX_WCHAR+1];

static int n_ctype_rows;
static int n_uplow_rows;
#ifdef DO_WIDE_CHAR
static int n_c2wc_rows;
#endif
static int tt_num;
static int ti_num;

#define RANGE MAX_WCHAR

#define TT_SHIFT 4
#define TI_SHIFT 4

#define II_LEN		((MAX_WCHAR+1) >> (TT_SHIFT+TI_SHIFT))

typedef struct {
	unsigned long c2w[256];
	unsigned char w2c[MAX_WCHAR];
	unsigned char ii[II_LEN];
	unsigned char ctype_idx[CTYPE_IDX_LEN];
	unsigned char uplow_idx[UPLOW_IDX_LEN];
	unsigned char c2wc_idx[C2WC_IDX_LEN];
} charset_data;

int main(int argc, char **argv)
{
	FILE *fp;
	charset_data csd[30];
	unsigned long max_wchar;
	unsigned char *p;
	int numsets;
	int i;
	int j;
	char buf[80];
	unsigned char row[256];
#ifdef DO_WIDE_CHAR
	unsigned short wrow[256];
#endif
	char codeset_list[500];
	char codeset_index[30];
	int codeset_list_end = 0;
	int total_size = 0;

	if (!setlocale(LC_CTYPE, "en_US.UTF-8")) {
		/* Silly foreigners disabling en_US locales */
		FILE *fp = popen("locale -a", "r");
		if (!fp)
			goto locale_failure;

		while (!feof(fp)) {
			char buf[256];
			size_t len;

			if (fgets(buf, sizeof(buf) - 10, fp) == NULL)
				goto locale_failure;

			len = strlen(buf);
			if (len > 0 && buf[len - 1] == '\n')
				buf[--len] = '\0';
			if (len < 5 || strcasecmp(&buf[len-5], ".UTF8") != 0)
				strcat(buf, ".UTF8");
			if (setlocale(LC_CTYPE, buf))
				goto locale_success;
		}

 locale_failure:
		printf("could not find a UTF8 locale ... please enable en_US.UTF-8\n");
		return EXIT_FAILURE;
 locale_success:
		pclose(fp);
	}

	if (argc == 1) {
		printf("#undef __CTYPE_HAS_8_BIT_LOCALES\n\n");

		printf("#define __LOCALE_DATA_NUM_CODESETS\t\t0\n");
		printf("#define __LOCALE_DATA_CODESET_LIST\t\t\"\"\n");
	} else {
		printf("#define __CTYPE_HAS_8_BIT_LOCALES\t\t1\n\n");
	}

	printf("#define __LOCALE_DATA_Cctype_IDX_SHIFT\t%d\n", CTYPE_IDX_SHIFT);
	printf("#define __LOCALE_DATA_Cctype_IDX_LEN\t\t%d\n", CTYPE_IDX_LEN);
#ifdef CTYPE_PACKED
	printf("#define __LOCALE_DATA_Cctype_ROW_LEN\t\t%d\n", CTYPE_ROW_LEN >> 1);
	printf("#define __LOCALE_DATA_Cctype_PACKED\t\t1\n");
#else
	printf("#define __LOCALE_DATA_Cctype_ROW_LEN\t\t%d\n", CTYPE_ROW_LEN);
	printf("#undef __LOCALE_DATA_Cctype_PACKED\n");
#endif

	printf("\n#define __LOCALE_DATA_Cuplow_IDX_SHIFT\t%d\n", UPLOW_IDX_SHIFT);
	printf("#define __LOCALE_DATA_Cuplow_IDX_LEN\t\t%d\n", UPLOW_IDX_LEN);
	printf("#define __LOCALE_DATA_Cuplow_ROW_LEN\t\t%d\n", UPLOW_ROW_LEN);

#ifdef DO_WIDE_CHAR
	printf("\n#define __LOCALE_DATA_Cc2wc_IDX_LEN\t\t%d\n", C2WC_IDX_LEN);
	printf("#define __LOCALE_DATA_Cc2wc_IDX_SHIFT\t\t%d\n", C2WC_IDX_SHIFT);
	printf("#define __LOCALE_DATA_Cc2wc_ROW_LEN\t\t%d\n", C2WC_ROW_LEN);
#endif

	printf("\ntypedef struct {\n");
	printf("\tunsigned char idx8ctype[%d];\n", CTYPE_IDX_LEN);
	printf("\tunsigned char idx8uplow[%d];\n", UPLOW_IDX_LEN);
#ifdef DO_WIDE_CHAR
	printf("\tunsigned char idx8c2wc[%d];\n", C2WC_IDX_LEN);
	printf("\tunsigned char idx8wc2c[%d];\n", II_LEN);
#endif
#ifndef __metag__
	printf("} __codeset_8_bit_t;\n\n");
#else
	printf("} __attribute__((__packed__)) __codeset_8_bit_t;\n\n");
#endif /* __metag__ */

	printf("#ifdef WANT_DATA\n\n");
	printf("static const __codeset_8_bit_t codeset_8_bit[%d] = {\n", argc-1);

	max_wchar = 0x7f;
	numsets = 0;
	codeset_index[0] = 0;
	while (--argc) {
		if (!(fp = fopen(*++argv,"r"))) {
			fprintf(stderr, "cannot open file \"%s\"\n", *argv);
			return EXIT_FAILURE;
		}
		fprintf(stderr, "processing %s... ", *argv);

		{
			char *s0;
			char *s1;
			int n;

			s0 = strrchr(*argv, '/');
			if (!s0) {
				s0 = *argv;
			} else {
				++s0;
			}
			s1 = strrchr(s0, '.');
			if (!s1) {
				n = strlen(s0);
			} else {
				n = s1 - s0;
			}

/*  			if ((numsets == 0) && strncmp("ASCII", s0, n)) { */
/*  				printf("error - first codeset isn't ASCII!\n"); */
/*  				return EXIT_FAILURE; */
/*  			} */

			if (numsets >= sizeof(codeset_index)) {
				fprintf(stderr, "error - too many codesets!\n");
				return EXIT_FAILURE;
			}

			if (codeset_list_end + n + 1 + numsets + 1 + 1 >= 256) {
				fprintf(stderr, "error - codeset list to big!\n");
				return EXIT_FAILURE;
			}

			codeset_index[numsets+1] = codeset_index[numsets] + n+1;
			strncpy(codeset_list + codeset_list_end, s0, n);
			codeset_list_end += (n+1);
			codeset_list[codeset_list_end - 1] = 0;

			printf("\t{ /* %.*s */", n, s0);
		}

		memset(&csd[numsets], 0, sizeof(charset_data));
		memset(xi, 0, sizeof(xi));
		{
			unsigned long c, wc;
			int lines;
			lines = 0;
			while (fgets(buf,sizeof(buf),fp)) {
				if ((2 != sscanf(buf, "{ %lx , %lx", &c, &wc))
					|| (c >= 256) || (wc > MAX_WCHAR)) {
					fprintf(stderr, "error: scanf failure! \"%s\"\n", buf);
					return EXIT_FAILURE;
				}

				/* don't put in w2c... dynamicly build tt instead. */

				if (c <= 0x7f) { /* check the 7bit entries but don't store */
					if (c != wc) {
						fprintf(stderr, "error: c != wc in %s\n", buf);
						return EXIT_FAILURE;
					}
					csd[numsets].c2w[c] = wc;
					csd[numsets].w2c[wc] = 0; /* ignore */
					if (wc > max_wchar) {
						max_wchar = wc;
					}
				} else {
					csd[numsets].c2w[c] = wc;
					csd[numsets].w2c[wc] = c;
					if (wc > max_wchar) {
						max_wchar = wc;
					}
				}
				++lines;
			}
			fprintf(stderr, "%d lines  ", lines);

			for (i = 0 ; i <= MAX_WCHAR ; i += (1 << TT_SHIFT)) {
				p = &csd[numsets].w2c[i];
				for (j = 0 ; j < tt_num ; j++) {
					if (!memcmp(p, &tt[j << TT_SHIFT], (1 << TT_SHIFT))) {
						break;
					}
				}
				if (j == tt_num) { /* new entry */
					memcpy(&tt[j << TT_SHIFT], p, (1 << TT_SHIFT));
					++tt_num;
				}
				xi[i >> TT_SHIFT] = j;
			}

			for (i = 0 ; i <= (MAX_WCHAR >> TT_SHIFT)  ; i += (1 << TI_SHIFT)) {
				p = &xi[i];
				for (j = 0 ; j < ti_num ; j++) {
					if (!memcmp(p, &ti[j << TI_SHIFT], (1 << TI_SHIFT))) {
						break;
					}
				}
				if (j == ti_num) { /* new entry */
					memcpy(&ti[j << TI_SHIFT], p, (1 << TI_SHIFT));
					++ti_num;
				}
				csd[numsets].ii[i >> TI_SHIFT] = j;
/*  				fprintf(stderr, "%d ", i >> TI_SHIFT); */
			}

#if 1
			printf("\n\t\t/* idx8ctype data */\n\t\t{");
			for (i = 128 ; i < 256 ; i++) {
				wchar_t c;
				unsigned int d;

/*  				if (!(i & 0x7)) { */
/*  					printf("\n"); */
/*  				} */

				c = csd[numsets].c2w[i];

				if (c == 0) {	/* non-existant char in codeset */
					d = __CTYPE_unclassified;
				} else if (iswdigit(c)) {
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

#if 1
				row[i & (CTYPE_ROW_LEN-1)] = d;
				if ((i & (CTYPE_ROW_LEN-1)) == (CTYPE_ROW_LEN-1)) {
					p = ctype_tbl;
					for (j=0 ; j < n_ctype_rows ; j++) {
						if (!memcmp(p, row, CTYPE_ROW_LEN)) {
							break;
						}
						p += CTYPE_ROW_LEN;
					}
					if (j == n_ctype_rows) { /* new entry */
						if (++n_ctype_rows > 256) {
							fprintf(stderr, "error -- to many ctype rows!\n");
							return EXIT_FAILURE;
						}
						memcpy(p, row, CTYPE_ROW_LEN);
					}
					csd[numsets].ctype_idx[i >> CTYPE_IDX_SHIFT] = j;
					if (!((i >> CTYPE_IDX_SHIFT) & 0x7)
						&& (i != (127 + CTYPE_ROW_LEN))
						) {
						printf("\n\t\t ");
					}
					printf(" %#4x,", j);
				}
#else
				printf(" %#4x,", d);
#endif
			}
#endif
			printf(" }");

#if 1
			printf(",\n\t\t/* idx8uplow data */\n\t\t{");
			for (i = 128 ; i < 256 ; i++) {
				wchar_t c, u, l;
/*  				if (!(i & 0x7)) { */
/*  					printf("\n"); */
/*  				} */
				c = csd[numsets].c2w[i];
				if ((c != 0) || 1) {
					u = towupper(c);
					l = towlower(c);

					if (u >= 0x80) u = csd[numsets].w2c[u];
					if (l >= 0x80) l = csd[numsets].w2c[l];

					if (u == 0) u = i; /* upper is missing, so ignore */
					if (l == 0) l = i; /* lower is missing, so ignore */

#if 1
					/* store as unsigned char and let overflow handle it. */
/*  					if ((((u-i) < CHAR_MIN) || ((u-i) > CHAR_MAX)) */
/*  						|| (((i-l) < CHAR_MIN) || ((i-l) > CHAR_MAX)) */
/*  						) { */
/*  						fprintf(stderr, "error - uplow diff out of range! %d %ld %ld\n", */
/*  							   i, u, l); */
/*  						return EXIT_FAILURE; */
/*  					} */

					row[i & (UPLOW_ROW_LEN-1)] = ((l==i) ? (u-i) : (i-l));
					if ((i & (UPLOW_ROW_LEN-1)) == (UPLOW_ROW_LEN-1)) {
						p = uplow_tbl;
						for (j=0 ; j < n_uplow_rows ; j++) {
							if (!memcmp(p, row, UPLOW_ROW_LEN)) {
								break;
							}
							p += UPLOW_ROW_LEN;
						}
						if (j == n_uplow_rows) { /* new entry */
							if (++n_uplow_rows > 256) {
								fprintf(stderr, "error -- to many uplow rows!\n");
								return EXIT_FAILURE;
							}
							memcpy(p, row, UPLOW_ROW_LEN);
						}
						csd[numsets].uplow_idx[i >> UPLOW_IDX_SHIFT] = j;
						if (!((i >> UPLOW_IDX_SHIFT) & 0x7)
							&& (i != (127 + UPLOW_ROW_LEN))
							) {
							printf("\n\t\t ");
						}
						printf(" %#4x,", j);
					}

#elif 0
					if (!(i & 0x7) && i) {
						printf("\n");
					}
					printf(" %4ld,", (l==i) ? (u-i) : (i-l));
/*  					printf(" %4ld,", (l==i) ? u : l); */
#else
					if ((u != i) || (l != i)) {
#if 0
						printf(" %#08lx, %#08lx, %#08lx, %#08lx, %#08lx, %#08lx, \n",
								(unsigned long) i,
								(unsigned long) c,
								(unsigned long) l,
								(unsigned long) towlower(c),
								(unsigned long) u,
								(unsigned long) towupper(c));

#else
						printf(" %#08lx, %8ld, %d, %8ld, %d, %#08lx\n",
								(unsigned long) i,
								(long) (l - i),
								iswupper(c),
								(long) (i - u),
								iswlower(c),
								(unsigned long) c);
#endif
					}
#endif
				}
			}
			printf(" }");
#endif

#ifndef DO_WIDE_CHAR
			printf("\n");
#else  /* DO_WIDE_CHAR */

#if 1
			printf(",\n\t\t/* idx8c2wc data */\n\t\t{");
			for (i = 128 ; i < 256 ; i++) {
#if 1
				wrow[i & (C2WC_ROW_LEN-1)] = csd[numsets].c2w[i];
				if ((i & (C2WC_ROW_LEN-1)) == (C2WC_ROW_LEN-1)) {
					p = (unsigned char *) c2wc_tbl;
					for (j=0 ; j < n_c2wc_rows ; j++) {
						if (!memcmp(p, (char *) wrow, 2*C2WC_ROW_LEN)) {
							break;
						}
						p += 2*C2WC_ROW_LEN;
					}
					if (j == n_c2wc_rows) { /* new entry */
						if (++n_c2wc_rows > 256) {
							fprintf(stderr, "error -- to many c2wc rows!\n");
							return EXIT_FAILURE;
						}
						memcpy(p, (char *) wrow, 2*C2WC_ROW_LEN);
					}
					csd[numsets].c2wc_idx[i >> C2WC_IDX_SHIFT] = j;
					if (!((i >> C2WC_IDX_SHIFT) & 0x7)
						&& (i != (127 + C2WC_ROW_LEN))
						) {
						printf("\n\t\t ");
					}
					printf(" %#4x,", j);
				}
#else
				if (!(i & 0x7) && i) {
					printf("\n");
				}
				printf(" %#6lx,", csd[numsets].c2w[i]);
#endif
			}
			printf(" },\n");
#endif

#if 1
/*  			fprintf(stderr, "\nII_LEN = %d\n", II_LEN); */
			printf("\t\t/* idx8wc2c data */\n\t\t{");
			for (i = 0 ; i < II_LEN ; i++) {
				if (!(i & 0x7) && i) {
					printf("\n\t\t ");
				}
				printf(" %#4x,", csd[numsets].ii[i]);
			}
			printf(" }\n");
#endif

#endif /* DO_WIDE_CHAR */
			printf("\t},\n");

		}
		++numsets;
		fprintf(stderr, "done\n");
	}
	printf("};\n");
	printf("\n#endif /* WANT_DATA */\n");

#ifdef DO_WIDE_CHAR
	printf("\n");
	printf("#define __LOCALE_DATA_Cwc2c_DOMAIN_MAX\t%#x\n", RANGE);
	printf("#define __LOCALE_DATA_Cwc2c_TI_SHIFT\t\t%d\n", TI_SHIFT);
	printf("#define __LOCALE_DATA_Cwc2c_TT_SHIFT\t\t%d\n", TT_SHIFT);
	printf("#define __LOCALE_DATA_Cwc2c_II_LEN\t\t%d\n", II_LEN);
	printf("#define __LOCALE_DATA_Cwc2c_TI_LEN\t\t%d\n", ti_num << TI_SHIFT);
	printf("#define __LOCALE_DATA_Cwc2c_TT_LEN\t\t%d\n", tt_num << TT_SHIFT);
	printf("\n");

	printf("\n#define __LOCALE_DATA_Cwc2c_TBL_LEN\t\t%d\n",
			(ti_num << TI_SHIFT) + (tt_num << TT_SHIFT));

	printf("#ifdef WANT_DATA\n\n");
	printf("static const unsigned char __LOCALE_DATA_Cwc2c_data[%d] = {\n",
			(ti_num << TI_SHIFT) + (tt_num << TT_SHIFT));
	printf("\t/* ti_table */\n\t");
	for (i=0 ; i < ti_num << TI_SHIFT ; i++) {
		if (!(i & 7) && i) {
			printf("\n\t");
		}
		printf(" %#4x,", ti[i]);
	}
	printf("\n");
	printf("\t/* tt_table */\n\t");
	for (i=0 ; i < tt_num << TT_SHIFT ; i++) {
		if (!(i & 7) && i) {
			printf("\n\t");
		}
		printf(" %#4x,", tt[i]);
	}
	printf("\n};\n");

	printf("\n#endif /* WANT_DATA */\n");
#endif /* DO_WIDE_CHAR */

	printf("\n#define __LOCALE_DATA_Cuplow_TBL_LEN\t\t%d\n",
			n_uplow_rows * UPLOW_ROW_LEN);
	printf("\n#ifdef WANT_DATA\n\n");

	printf("\nstatic const unsigned char __LOCALE_DATA_Cuplow_data[%d] = {\n",
			n_uplow_rows * UPLOW_ROW_LEN);
	p = uplow_tbl;
	for (j=0 ; j < n_uplow_rows ; j++) {
		printf("\t");
		for (i=0 ; i < UPLOW_ROW_LEN ; i++) {
			printf(" %#4x,", (unsigned int)((unsigned char) p[i]));
		}
		printf("\n");
		p += UPLOW_ROW_LEN;
	}
	printf("};\n");

	printf("\n#endif /* WANT_DATA */\n");
	printf("\n#define __LOCALE_DATA_Cctype_TBL_LEN\t\t%d\n",
#ifdef CTYPE_PACKED
			n_ctype_rows * CTYPE_ROW_LEN / 2
#else
			n_ctype_rows * CTYPE_ROW_LEN
#endif
			);
	printf("\n#ifdef WANT_DATA\n\n");


	printf("\nstatic const unsigned char __LOCALE_DATA_Cctype_data[%d] = {\n",
#ifdef CTYPE_PACKED
			n_ctype_rows * CTYPE_ROW_LEN / 2
#else
			n_ctype_rows * CTYPE_ROW_LEN
#endif
			);
	p = ctype_tbl;
	for (j=0 ; j < n_ctype_rows ; j++) {
		printf("\t");
		for (i=0 ; i < CTYPE_ROW_LEN ; i++) {
#ifdef CTYPE_PACKED
			printf(" %#4x,", (unsigned int)(p[i] + (p[i+1] << 4)));
			++i;
#else
			printf(" %#4x,", (unsigned int)p[i]);
#endif
		}
		printf("\n");
		p += CTYPE_ROW_LEN;
	}
	printf("};\n");

	printf("\n#endif /* WANT_DATA */\n");

#ifdef DO_WIDE_CHAR

	printf("\n#define __LOCALE_DATA_Cc2wc_TBL_LEN\t\t%d\n",
			n_c2wc_rows * C2WC_ROW_LEN);
	printf("\n#ifdef WANT_DATA\n\n");

	printf("\nstatic const unsigned short __LOCALE_DATA_Cc2wc_data[%d] = {\n",
			n_c2wc_rows * C2WC_ROW_LEN);
	p = (unsigned char *) c2wc_tbl;
	for (j=0 ; j < n_c2wc_rows ; j++) {
		printf("\t");
		for (i=0 ; i < C2WC_ROW_LEN ; i++) {
			printf(" %#6x,", (unsigned int)(((unsigned short *)p)[i]));
		}
		printf("\n");
		p += 2*C2WC_ROW_LEN;
	}
	printf("};\n");
	printf("\n#endif /* WANT_DATA */\n");
#endif /* DO_WIDE_CHAR */
	printf("\n\n");

	printf("#define __LOCALE_DATA_NUM_CODESETS\t\t%d\n", numsets);
	printf("#define __LOCALE_DATA_CODESET_LIST \\\n\t\"");
	for (i=0 ; i < numsets ; i++) {
		printf("\\x%02x", numsets + 1 + (unsigned char) codeset_index[i]);
		if (((i & 7) == 7) && (i + 1 < numsets)) {
			printf("\" \\\n\t\"");
		}
	}
	printf("\" \\\n\t\"\\0\"");
	for (i=0 ; i < numsets ; i++) {
		printf(" \\\n\t\"%s\\0\"",
				codeset_list + ((unsigned char)codeset_index[i]));
	}

	printf("\n\n");
	for (i=0 ; i < numsets ; i++) {
		char buf[30];
		char *z;
		strcpy(buf, codeset_list + ((unsigned char)codeset_index[i]));
		for (z=buf ; *z ; z++) {
			if (*z == '-') {
				*z = '_';
			}
		}
		printf("#define __CTYPE_HAS_CODESET_%s\n", buf);
	}
#ifdef DO_WIDE_CHAR
	printf("#define __CTYPE_HAS_CODESET_UTF_8\n");
#endif /* DO_WIDE_CHAR */

#if 0
	printf("\n#endif /* __CTYPE_HAS_8_BIT_LOCALES */\n\n");
#endif

	total_size = 0;
#ifdef DO_WIDE_CHAR
	fprintf(stderr, "tt_num = %d   ti_num = %d\n", tt_num, ti_num);
	fprintf(stderr, "max_wchar = %#lx\n", max_wchar);

	fprintf(stderr, "size is %d * %d  +  %d * %d  + %d * %d  =  %d\n",
		   tt_num, 1 << TT_SHIFT, ti_num, 1 << TI_SHIFT,
		   ((MAX_WCHAR >> (TT_SHIFT + TI_SHIFT)) + 1), numsets,
 		   j = tt_num * (1 << TT_SHIFT) + ti_num * (1 << TI_SHIFT)
		   + ((MAX_WCHAR >> (TT_SHIFT + TI_SHIFT)) + 1) * numsets);
	total_size += j;
#endif /* DO_WIDE_CHAR */

#ifdef CTYPE_PACKED
	i = 2;
#else
	i = 1;
#endif

	fprintf(stderr, "ctype - CTYPE_IDX_SHIFT = %d -- %d * %d + %d * %d = %d\n",
		   CTYPE_IDX_SHIFT, numsets, CTYPE_IDX_LEN, n_ctype_rows, CTYPE_ROW_LEN / i,
		   j = numsets * CTYPE_IDX_LEN +  n_ctype_rows * CTYPE_ROW_LEN / i);
	total_size += j;

	fprintf(stderr, "uplow - UPLOW_IDX_SHIFT = %d -- %d * %d + %d * %d = %d\n",
		   UPLOW_IDX_SHIFT, numsets, UPLOW_IDX_LEN, n_uplow_rows, UPLOW_ROW_LEN,
		   j = numsets * UPLOW_IDX_LEN +  n_uplow_rows * UPLOW_ROW_LEN);
	total_size += j;

#ifdef DO_WIDE_CHAR

	fprintf(stderr, "c2wc - C2WC_IDX_SHIFT = %d -- %d * %d + 2 * %d * %d = %d\n",
		   C2WC_IDX_SHIFT, numsets, C2WC_IDX_LEN, n_c2wc_rows, C2WC_ROW_LEN,
		   j = numsets * C2WC_IDX_LEN +  2 * n_c2wc_rows * C2WC_ROW_LEN);
	total_size += j;

#endif /* DO_WIDE_CHAR */

	fprintf(stderr, "total size = %d\n", total_size);

/*  	for (i=0 ; i < numsets ; i++) { */
/*  		printf("codeset_index[i] = %d  codeset_list[ci[i]] = \"%s\"\n", */
/*  			   (unsigned char) codeset_index[i], */
/*  			   codeset_list + ((unsigned char)codeset_index[i])); */
/*  	} */

	return EXIT_SUCCESS;
}
