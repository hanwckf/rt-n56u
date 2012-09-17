#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>
#include <stdarg.h>
#include <locale.h>
#include <langinfo.h>
#include <nl_types.h>
#include <stdint.h>

#include "c8tables.h"


#define __LOCALE_DATA_CATEGORIES 6

/* must agree with ordering of gen_mmap! */
static const char *lc_names[] = {
	"LC_CTYPE",
	"LC_NUMERIC",
	"LC_MONETARY",
	"LC_TIME",
	"LC_COLLATE",
	"LC_MESSAGES",
#if __LOCALE_DATA_CATEGORIES == 12
	"LC_PAPER",
	"LC_NAME",
	"LC_ADDRESS",
	"LC_TELEPHONE",
	"LC_MEASUREMENT",
	"LC_IDENTIFICATION",
#elif __LOCALE_DATA_CATEGORIES != 6
#error unsupported __LOCALE_DATA_CATEGORIES value!
#endif
};



typedef struct {
	char *glibc_name;
	char name[5];
	char dot_cs;				/* 0 if no codeset specified */
	char cs;
	unsigned char idx_name;
	unsigned char lc_time_row;
	unsigned char lc_numeric_row;
	unsigned char lc_monetary_row;
	unsigned char lc_messages_row;
	unsigned char lc_ctype_row;
#if __LOCALE_DATA_CATEGORIES != 6
#error unsupported __LOCALE_DATA_CATEGORIES value
#endif
} locale_entry;

static void read_at_mappings(void);
static void read_enable_disable(void);
static void read_locale_list(void);

static int find_codeset_num(const char *cs);
static int find_at_string_num(const char *as);
static int le_cmp(const void *, const void *);
static void dump_table8(const char *name, const char *tbl, int len);
static void dump_table8c(const char *name, const char *tbl, int len);
static void dump_table16(const char *name, const int *tbl, int len);

static void do_lc_time(void);
static void do_lc_numeric(void);
static void do_lc_monetary(void);

static void do_lc_messages(void);
static void do_lc_ctype(void);


static FILE *fp;
static FILE *ofp;
static char line_buf[80];
static char at_mappings[256];
static char at_mapto[256];
static char at_strings[1024];
static char *at_strings_end;
static locale_entry locales[700];
static char glibc_locale_names[60000];

static int num_locales;

static int default_utf8;
static int default_8bit;

static int total_size;
static int null_count;

static unsigned verbose = 0;
enum {
	VINFO = (1<<0),
	VDETAIL = (1<<1),
};
static int verbose_msg(const unsigned lvl, const char *fmt, ...)
{
	va_list arg;
	int ret = 0;

	if (verbose & lvl) {
		va_start(arg, fmt);
		ret = vfprintf(stderr, fmt, arg);
		va_end(arg);
	}
	return ret;
}

static void error_msg(const char *fmt, ...)  __attribute__ ((noreturn, format (printf, 1, 2)));
static void error_msg(const char *fmt, ...)
{
	va_list arg;

	fprintf(stderr, "Error: ");
/*	if (fno >= 0) {
	    fprintf(stderr, "file %s (%d): ", fname[fno], lineno[fno]);
	} */
	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	va_end(arg);
	fprintf(stderr, "\n");

	exit(EXIT_FAILURE);
}

static void do_locale_names(void)
{
	/* "C" locale name is handled specially by the setlocale code. */
	int uniq = 0;
	int i;

	if (num_locales <= 1) {
/*		error_msg("only C locale?"); */
		fprintf(ofp, "static const unsigned char __locales[%d];\n", (3 + __LOCALE_DATA_CATEGORIES));
		fprintf(ofp, "static const unsigned char __locale_names5[5];\n");
	} else {
		if (default_utf8) {
			fprintf(ofp, "#define __CTYPE_HAS_UTF_8_LOCALES\t\t\t1\n");
		}
		fprintf(ofp, "#define __LOCALE_DATA_CATEGORIES\t\t\t%d\n", __LOCALE_DATA_CATEGORIES);
		fprintf(ofp, "#define __LOCALE_DATA_WIDTH_LOCALES\t\t\t%d\n", 3+__LOCALE_DATA_CATEGORIES);
		fprintf(ofp, "#define __LOCALE_DATA_NUM_LOCALES\t\t\t%d\n", num_locales);
		fprintf(ofp, "static const unsigned char __locales[%d] = {\n",
				(num_locales) * (3 + __LOCALE_DATA_CATEGORIES));
		for (i=0 ; i < num_locales ; i++) {
			if (memcmp(locales[i].name, locales[i-1].name, 5) != 0) {
				locales[i].idx_name = uniq;
				++uniq;
			} else {
				locales[i].idx_name = uniq - 1;
			}
			fprintf(ofp, "\t%#4x, ", (int)((unsigned char) locales[i].idx_name));
			fprintf(ofp, "\t%#4x, ", (int)((unsigned char) locales[i].dot_cs));
			fprintf(ofp, "\t%#4x, ", (int)((unsigned char) locales[i].cs));
			/* lc_ctype would store translit flags and turkish up/low flag. */
			fprintf(ofp, "%#4x, ", (int)((unsigned char) locales[i].lc_ctype_row));
			fprintf(ofp, "%#4x, ", (int)((unsigned char) locales[i].lc_numeric_row));
			fprintf(ofp, "%#4x, ", (int)((unsigned char) locales[i].lc_monetary_row));
			fprintf(ofp, "%#4x, ", (int)((unsigned char) locales[i].lc_time_row));
#if 1
			/* lc_collate */
			if (strlen(locales[i].glibc_name) >= 5) {
			    fprintf(ofp, "COL_IDX_%.2s_%.2s, ", locales[i].glibc_name, locales[i].glibc_name+3);
			} else if (!strcmp(locales[i].glibc_name, "C")) {
			    fprintf(ofp, "COL_IDX_C    , ");
			} else {
			    error_msg("don't know how to handle COL_IDX_ for %s", locales[i].glibc_name);
			}
#else
			fprintf(ofp, "%#4x, ", 0); /* place holder for lc_collate */
#endif
			fprintf(ofp, "%#4x, ", (int)((unsigned char) locales[i].lc_messages_row));
			fprintf(ofp, "\t/* %s */\n", locales[i].glibc_name);
		}
		fprintf(ofp, "};\n\n");

		fprintf(ofp, "#define __LOCALE_DATA_NUM_LOCALE_NAMES\t\t%d\n", uniq );
		fprintf(ofp, "static const unsigned char __locale_names5[%d] = \n\t", uniq * 5);
		uniq = 0;
		for (i=1 ; i < num_locales ; i++) {
			if (memcmp(locales[i].name, locales[i-1].name, 5) != 0) {
				fprintf(ofp, "\"%5.5s\" ", locales[i].name);
				++uniq;
				if ((uniq % 8) == 0) {
					fprintf(ofp, "\n\t");
				}
			}
		}
		fprintf(ofp,";\n\n");

		if (at_strings_end > at_strings) {
			int i, j;
			char *p;
			i = 0;
			p = at_strings;
			while (*p) {
				++i;
				p += 1 + (unsigned char) *p;
			}
			/* len, char, string\0 */
			fprintf(ofp, "#define __LOCALE_DATA_AT_MODIFIERS_LENGTH\t\t%d\n",
					i + (at_strings_end - at_strings));
			fprintf(ofp, "static const unsigned char __locale_at_modifiers[%d] = {",
					i + (at_strings_end - at_strings));
			i = 0;
			p = at_strings;
			while (*p) {
				fprintf(ofp, "\n\t%4d, '%c',",
						(unsigned char) *p, /* len of string\0 */
						at_mapto[i]);
				for (j=1 ; j < ((unsigned char) *p) ; j++) {
					fprintf(ofp, " '%c',", p[j]);
				}
				fprintf(ofp, " 0,");
				++i;
				p += 1 + (unsigned char) *p;
			}
			fprintf(ofp, "\n};\n\n");
		}

		{
			int pos[__LOCALE_DATA_CATEGORIES];
			pos[0] = __LOCALE_DATA_CATEGORIES;
			for (i=0 ; i < __LOCALE_DATA_CATEGORIES ; i++) {
				fprintf(ofp, "#define __%s\t\t%d\n", lc_names[i], i);
				if (i + 1 < __LOCALE_DATA_CATEGORIES) {
					pos[i+1] = 1 + strlen(lc_names[i]) + pos[i];
				}
			}
			if (pos[__LOCALE_DATA_CATEGORIES-1] > 255) {
				error_msg("lc_names is too big (%d)", pos[__LOCALE_DATA_CATEGORIES-1]);
			}
			fprintf(ofp, "#define __LC_ALL\t\t%d\n\n", i);

			fprintf(ofp, "#define __lc_names_LEN\t\t%d\n",
					pos[__LOCALE_DATA_CATEGORIES-1] + strlen(lc_names[__LOCALE_DATA_CATEGORIES-1]) + 1);
			total_size += pos[__LOCALE_DATA_CATEGORIES-1] + strlen(lc_names[__LOCALE_DATA_CATEGORIES-1]) + 1;

			fprintf(ofp, "static unsigned const char lc_names[%d] =\n",
					pos[__LOCALE_DATA_CATEGORIES-1] + strlen(lc_names[__LOCALE_DATA_CATEGORIES-1]) + 1);
			fprintf(ofp, "\t\"");
			for (i=0 ; i < __LOCALE_DATA_CATEGORIES ; i++) {
				fprintf(ofp, "\\x%02x", (unsigned char) pos[i]);
			}
			fprintf(ofp, "\"");
			for (i=0 ; i < __LOCALE_DATA_CATEGORIES ; i++) {
				fprintf(ofp, "\n\t\"%s\\0\"", lc_names[i]);
			}
			fprintf(ofp, ";\n\n");
		}

		verbose_msg(VDETAIL,"locale data = %d  name data = %d for %d uniq\n",
			   num_locales * (3 + __LOCALE_DATA_CATEGORIES), uniq * 5, uniq);

		total_size += num_locales * (3 + __LOCALE_DATA_CATEGORIES) + uniq * 5;
	}

}

static void read_at_mappings(void)
{
	char *p;
	char *m;
	int mc = 0;

	do {
		if (!(p = strtok(line_buf, " \t\n")) || (*p == '#')) {
			if (!fgets(line_buf, sizeof(line_buf), fp)) {
				if (ferror(fp)) {
					error_msg("reading file");
				}
				return;			/* EOF */
			}
			if ((*line_buf == '#') && (line_buf[1] == '-')) {
				break;
			}
			continue;
		}
		if (*p == '@') {
			if (p[1] == 0) {
				error_msg("missing @modifier name");
			}
			m = p;				/* save the modifier name */
			if (!(p = strtok(NULL, " \t\n")) || p[1] || (((unsigned char) *p) > 0x7f)) {
				error_msg("missing or illegal @modifier mapping char");
			}
			if (at_mappings[(int)((unsigned char) *p)]) {
				error_msg("reused @modifier mapping char");
			}
			at_mappings[(int)((unsigned char) *p)] = 1;
			at_mapto[mc] = *p;
			++mc;
			*at_strings_end = (char)( (unsigned char) (strlen(m)) );
			strcpy(++at_strings_end, m+1);
			at_strings_end += (unsigned char) at_strings_end[-1];

			verbose_msg(VDETAIL,"@mapping: \"%s\" to '%c'\n", m, *p);

			if (((p = strtok(NULL, " \t\n")) != NULL) && (*p != '#')) {
				fprintf(stderr,"ignoring trailing text: %s...\n", p);
			}
			*line_buf = 0;
			continue;
		}
		break;
	} while (1);

#if 0
	{
		p = at_strings;

		if (!*p) {
			verbose_msg(VDETAIL,"no @ strings\n");
			return;
		}

		do {
			verbose_msg(VDETAIL,"%s\n", p+1);
			p += 1 + (unsigned char) *p;
		} while (*p);
	}
#endif
}

static void read_enable_disable(void)
{
	char *p;

	do {
		if (!(p = strtok(line_buf, " =\t\n")) || (*p == '#')) {
			if (!fgets(line_buf, sizeof(line_buf), fp)) {
				if (ferror(fp)) {
					error_msg("reading file");
				}
				return;			/* EOF */
			}
			if ((*line_buf == '#') && (line_buf[1] == '-')) {
				break;
			}
			continue;
		}
		if (!strcmp(p, "UTF-8")) {
			if (!(p = strtok(NULL, " =\t\n"))
				|| ((toupper(*p) != 'Y') && (toupper(*p) != 'N'))) {
				error_msg("missing or illegal UTF-8 setting");
			}
			default_utf8 = (toupper(*p) == 'Y');
			verbose_msg(VINFO,"UTF-8 locales are %sabled\n", "dis\0en"+ (default_utf8 << 2));
		} else if (!strcmp(p, "8-BIT")) {
			if (!(p = strtok(NULL, " =\t\n"))
				|| ((toupper(*p) != 'Y') && (toupper(*p) != 'N'))) {
				error_msg("missing or illegal 8-BIT setting");
			}
			default_8bit = (toupper(*p) == 'Y');
			verbose_msg(VINFO,"8-BIT locales are %sabled\n", "dis\0en" + (default_8bit << 2));
		} else {
			break;
		}

		if (((p = strtok(NULL, " \t\n")) != NULL) && (*p != '#')) {
			fprintf(stderr,"ignoring trailing text: %s...\n", p);
		}
		*line_buf = 0;
		continue;

	} while (1);
}

#ifdef __LOCALE_DATA_CODESET_LIST

static int find_codeset_num(const char *cs)
{
	int r = 2;
	char *s = __LOCALE_DATA_CODESET_LIST;

	/* 7-bit is 1, UTF-8 is 2, 8-bits are > 2 */

	if (strcmp(cs, "UTF-8") != 0) {
		++r;
		while (*s && strcmp(__LOCALE_DATA_CODESET_LIST+ ((unsigned char) *s), cs)) {
/*  			verbose_msg(VDETAIL,"tried %s\n", __LOCALE_DATA_CODESET_LIST + ((unsigned char) *s)); */
			++r;
			++s;
		}
		if (!*s) {
			error_msg("unsupported codeset %s", cs);
		}
	}
	return r;
}

#else

static int find_codeset_num(const char *cs)
{
	int r = 2;

	/* 7-bit is 1, UTF-8 is 2, 8-bits are > 2 */

	if (strcmp(cs, "UTF-8") != 0) {
		error_msg("unsupported codeset %s", cs);
	}
	return r;
}

#endif

static int find_at_string_num(const char *as)
{
	int i = 0;
	char *p = at_strings;

	while (*p) {
		if (!strcmp(p+1, as)) {
			return i;
		}
		++i;
		p += 1 + (unsigned char) *p;
	}

	error_msg("error: unmapped @string %s", as);
}

static void read_locale_list(void)
{
	char *p;
	char *s;
	char *ln;					/* locale name */
	char *ls;					/* locale name ll_CC */
	char *as;					/* at string */
	char *ds;					/* dot string */
	char *cs;					/* codeset */
	int i;

	typedef struct {
		char *glibc_name;
		char name[5];
		char dot_cs;				/* 0 if no codeset specified */
		char cs;
	} locale_entry;

	/* First the C locale. */
	locales[0].glibc_name = locales[0].name;
	strncpy(locales[0].name,"C",5);
	locales[0].dot_cs = 0;
	locales[0].cs = 1;			/* 7-bit encoding */
	++num_locales;

	do {
		if (!(p = strtok(line_buf, " \t\n")) || (*p == '#')) {
			if (!fgets(line_buf, sizeof(line_buf), fp)) {
				if (ferror(fp)) {
					error_msg("reading file");
				}
				return;			/* EOF */
			}
			if ((*line_buf == '#') && (line_buf[1] == '-')) {
				break;
			}
			continue;
		}

		s = glibc_locale_names;
		for (i=0 ; i < num_locales ; i++) {
			if (!strcmp(s+1, p)) {
				break;
			}
			s += 1 + ((unsigned char) *s);
		}
		if (i < num_locales) {
			fprintf(stderr,"ignoring duplicate locale name: %s", p);
			*line_buf = 0;
			continue;
		}

		/* New locale, but don't increment num until codeset verified! */
		*s = (char)((unsigned char) (strlen(p) + 1));
		strcpy(s+1, p);
		locales[num_locales].glibc_name = s+1;
		ln = p;					/* save locale name */

		if (!(p = strtok(NULL, " \t\n"))) {
			error_msg("missing codeset for locale %s", ln);
		}
		cs = p;
		i = find_codeset_num(p);
		if ((i == 2) && !default_utf8) {
			fprintf(stderr,"ignoring UTF-8 locale %s\n", ln);
			*line_buf = 0;
			continue;
		} else if ((i > 2) && !default_8bit) {
			fprintf(stderr,"ignoring 8-bit codeset locale %s\n", ln);
			*line_buf = 0;
			continue;
		}
		locales[num_locales].cs = (char)((unsigned char) i);

		if (((p = strtok(NULL, " \t\n")) != NULL) && (*p != '#')) {
			verbose_msg(VINFO,"ignoring trailing text: %s...\n", p);
		}

		/* Now go back to locale string for .codeset and @modifier */
		as = strtok(ln, "@");
		if (as) {
			as = strtok(NULL, "@");
		}
		ds = strtok(ln, ".");
		if (ds) {
			ds = strtok(NULL, ".");
		}
		ls = ln;

		if ((strlen(ls) != 5) || (ls[2] != '_')) {
			error_msg("illegal locale name %s", ls);
		}

		i = 0;					/* value for unspecified codeset */
		if (ds) {
			i = find_codeset_num(ds);
			if ((i == 2) && !default_utf8) {
				fprintf(stderr,"ignoring UTF-8 locale %s\n", ln);
				*line_buf = 0;
				continue;
			} else if ((i > 2) && !default_8bit) {
				fprintf(stderr,"ignoring 8-bit codeset locale %s\n", ln);
				*line_buf = 0;
				continue;
			}
		}
		locales[num_locales].dot_cs = (char)((unsigned char) i);

		if (as) {
			i = find_at_string_num(as);
			ls[2] = at_mapto[i];
		}
		memcpy(locales[num_locales].name, ls, 5);
/*  		verbose_msg(VDETAIL,"locale: %5.5s %2d %2d %s\n", */
/*  			   locales[num_locales].name, */
/*  			   locales[num_locales].cs, */
/*  			   locales[num_locales].dot_cs, */
/*  			   locales[num_locales].glibc_name */
/*  			   ); */
		++num_locales;
		*line_buf = 0;
	} while (1);
}

static int le_cmp(const void *a, const void *b)
{
	const locale_entry *p;
	const locale_entry *q;
	int r;

	p = (const locale_entry *) a;
	q = (const locale_entry *) b;

	if (!(r = p->name[0] - q->name[0])
		&& !(r = p->name[1] - q->name[1])
		&& !(r = p->name[3] - q->name[3])
		&& !(r = p->name[4] - q->name[4])
		&& !(r = p->name[2] - q->name[2])
		&& !(r = -(p->cs - q->cs))
		) {
		r = -(p->dot_cs - q->dot_cs);
		/* Reverse the ordering of the codesets so UTF-8 comes last.
		 * Work-around (hopefully) for glibc bug affecting at least
		 * the euro currency symbol. */
	}

	return r;
}

int main(int argc, char **argv)
{
	char *output_file = NULL;

	while (--argc) {
		++argv;
		if (!strcmp(*argv, "-o")) {
			--argc;
			output_file = strdup(*++argv);
		} else if (!strcmp(*argv, "-v")) {
			verbose++;
		} else if (!(fp = fopen(*argv, "r"))) {
no_inputfile:
				error_msg("missing filename or file!");
		}
	}
	if (fp == NULL)
		goto no_inputfile;
	if (output_file == NULL)
		output_file = strdup("locale_tables.h");

	at_strings_end = at_strings;

	read_at_mappings();
	read_enable_disable();
	read_locale_list();

	fclose(fp);

	/* handle C locale specially */
	qsort(locales+1, num_locales-1, sizeof(locale_entry), le_cmp);

#if 0
	for (i=0 ; i < num_locales ; i++) {
		verbose_msg(VDETAIL,"locale: %5.5s %2d %2d %s\n",
			   locales[i].name,
			   locales[i].cs,
			   locales[i].dot_cs,
			   locales[i].glibc_name
			   );
	}
#endif
	if (argc == 3)
		output_file = *++argv;
	if (output_file == NULL || !(ofp = fopen(output_file, "w"))) {
		error_msg("cannot open output file '%s'!", output_file);
	}

	do_lc_time();
	do_lc_numeric();
	do_lc_monetary();
	do_lc_messages();
	do_lc_ctype();

	do_locale_names();

	fclose(ofp);

	verbose_msg(VINFO, "total data size = %d\n", total_size);
	verbose_msg(VDETAIL, "null count = %d\n", null_count);

	return EXIT_SUCCESS;
}

static char *idx[10000];
static char buf[100000];
static char *last;
static int uniq;

static int addblock(const char *s, size_t n) /* l includes nul terminator */
{
	int j;

	if (!s) {
		++null_count;
		return 0;
	}

	for (j=0 ; (j < uniq) && (idx[j] + n < last) ; j++) {
		if (!memcmp(s, idx[j], n)) {
			return idx[j] - buf;
		}
	}
	if (uniq >= sizeof(idx)) {
		error_msg("too many uniq strings!");
	}
	if (last + n >= buf + sizeof(buf)) {
		error_msg("need to increase size of buf!");
	}

	idx[uniq] = last;
	++uniq;
	memcpy(last, s, n);
	last += n;
	return idx[uniq - 1] - buf;
}

static int addstring(const char *s)
{
	int j;
	size_t l;

	if (!s) {
		++null_count;
		return 0;
	}

	for (j=0 ; j < uniq ; j++) {
		if (!strcmp(s, idx[j])) {
			return idx[j] - buf;
		}
	}
	if (uniq >= sizeof(idx)) {
		error_msg("too many uniq strings!");
	}
	l = strlen(s) + 1;
	if (last + l >= buf + sizeof(buf)) {
		error_msg("need to increase size of buf!");
	}

	idx[uniq] = last;
	++uniq;
	strcpy(last, s);
	last += l;
	return idx[uniq - 1] - buf;
}

#define DO_LC_COMMON(CATEGORY) \
	verbose_msg(VDETAIL, "buf-size=%d  uniq=%d  rows=%d\n", \
		   (int)(last - buf), uniq, lc_##CATEGORY##_uniq); \
	verbose_msg(VDETAIL, "total = %d + %d * %d + %d = %d\n", \
		   num_locales, lc_##CATEGORY##_uniq, NUM_NL_##CATEGORY, (int)(last - buf), \
		   i = num_locales + lc_##CATEGORY##_uniq*NUM_NL_##CATEGORY + (int)(last - buf)); \
	total_size += i; \
	dump_table8c("__lc_" #CATEGORY "_data", buf, (int)(last - buf)); \
	for (i=0 ; i < lc_##CATEGORY##_uniq ; i++) { \
		m = locales[i].lc_##CATEGORY##_row; \
		for (k=0 ; k < NUM_NL_##CATEGORY ; k++) { \
			buf[NUM_NL_##CATEGORY*i + k] = (char)((unsigned char) lc_##CATEGORY##_uniq_X[i][k]); \
		} \
	} \
	dump_table8("__lc_" #CATEGORY "_rows", buf, lc_##CATEGORY##_uniq * NUM_NL_##CATEGORY); \
	buf16[0] =0; \
	for (i=0 ; i < NUM_NL_##CATEGORY - 1 ; i++) { \
		buf16[i+1] = buf16[i] + lc_##CATEGORY##_count[i]; \
	} \
	dump_table16("__lc_" #CATEGORY "_item_offsets", buf16, NUM_NL_##CATEGORY); \
	m = 0; \
	for (k=0 ; k < NUM_NL_##CATEGORY ; k++) { \
		for (i=0 ; i < lc_##CATEGORY##_count[k] ; i++) { \
			buf16[m] = lc_##CATEGORY##_item[k][i]; \
			++m; \
		} \
	} \
	dump_table16("__lc_" #CATEGORY "_item_idx", buf16, m);


#define DL_LC_LOOPTAIL(CATEGORY) \
		if (k > NUM_NL_##CATEGORY) { \
			error_msg("lc_" #CATEGORY " nl_item count > %d!", NUM_NL_##CATEGORY); \
		} \
		{ \
			int r; \
			for (r=0 ; r < lc_##CATEGORY##_uniq ; r++) { \
				if (!memcmp(lc_##CATEGORY##_uniq_X[lc_##CATEGORY##_uniq], \
							lc_##CATEGORY##_uniq_X[r], NUM_NL_##CATEGORY)) { \
					break; \
				} \
			} \
			if (r == lc_##CATEGORY##_uniq) { /* new locale row */ \
				++lc_##CATEGORY##_uniq; \
				if (lc_##CATEGORY##_uniq > 255) { \
					error_msg("too many unique lc_" #CATEGORY " rows!"); \
				} \
			} \
			locales[i].lc_##CATEGORY##_row = r; \
		}



static int buf16[100*256];

static void dump_table8(const char *name, const char *tbl, int len)
{
	int i;

	fprintf(ofp, "#define %s_LEN\t\t%d\n", name, len);
	fprintf(ofp, "static const unsigned char %s[%d] = {", name, len);
	for (i=0 ; i < len ; i++) {
		if ((i % 12) == 0) {
			fprintf(ofp, "\n\t");
		}
		fprintf(ofp, "%#4x, ", (int)((unsigned char) tbl[i]));
	}
	fprintf(ofp, "\n};\n\n");
}

#define __C_isdigit(c) \
	((sizeof(c) == sizeof(char)) \
	 ? (((unsigned char)((c) - '0')) < 10) \
	 : (((unsigned int)((c) - '0')) < 10))
#define __C_isalpha(c) \
	((sizeof(c) == sizeof(char)) \
	 ? (((unsigned char)(((c) | 0x20) - 'a')) < 26) \
	 : (((unsigned int)(((c) | 0x20) - 'a')) < 26))
#define __C_isalnum(c) (__C_isalpha(c) || __C_isdigit(c))

static void dump_table8c(const char *name, const char *tbl, int len)
{
	int i;

	fprintf(ofp, "#define %s_LEN\t\t%d\n", name, len);
	fprintf(ofp, "static const unsigned char %s[%d] = {", name, len);
	for (i=0 ; i < len ; i++) {
		if ((i % 12) == 0) {
			fprintf(ofp, "\n\t");
		}
		if (__C_isalnum(tbl[i]) || (tbl[i] == ' ')) {
			fprintf(ofp, " '%c', ", (int)((unsigned char) tbl[i]));
		} else {
			fprintf(ofp, "%#4x, ", (int)((unsigned char) tbl[i]));
		}
	}
	fprintf(ofp, "\n};\n\n");
}

static void dump_table16(const char *name, const int *tbl, int len)
{
	int i;

	fprintf(ofp, "#define %s_LEN\t\t%d\n", name, len);
	fprintf(ofp, "static const uint16_t %s[%d] = {", name, len);
	for (i=0 ; i < len ; i++) {
		if ((i % 8) == 0) {
			fprintf(ofp, "\n\t");
		}
		if (tbl[i] != (uint16_t) tbl[i]) {
			error_msg("falls outside uint16 range!");
		}
		fprintf(ofp, "%#6x, ", tbl[i]);
	}
	fprintf(ofp, "\n};\n\n");
}


#define NUM_NL_time 50

static int lc_time_item[NUM_NL_time][256];
static int lc_time_count[NUM_NL_time];
static unsigned char lc_time_uniq_X[700][NUM_NL_time];
static int lc_time_uniq;

#define DO_NL_S(X)	lc_time_S(X, k++)

static void lc_time_S(int X, int k)
{
	size_t len;
	int j, m;
	const char *s = nl_langinfo(X);
	const char *p;
	static const char nulbuf[] = "";

	if (X == ALT_DIGITS) {
		len = 1;
		if (!s) {
			s = nulbuf;
		}
		if (*s) {
			p = s;
			for (j = 0 ; j < 100 ; j++) {
				while (*p) {
					++p;
				}
				++p;
			}
			len = p - s;
		}
		j = addblock(s, len);
/* 		if (len > 1) verbose_msg(VDETAIL, "alt_digit: called addblock with len %zd\n", len); */
	} else if (X == ERA) {
		if (!s) {
			s = nulbuf;
		}
		p = s;
		while (*p) {
			while (*p) {
				++p;
			}
			++p;
		}
		++p;
		j = addblock(s, p - s);
/* 		if (p-s > 1) verbose_msg(VDETAIL, "era: called addblock with len %d\n", p-s); */
	} else {
		j = addstring(s);
	}
	for (m=0 ; m < lc_time_count[k] ; m++) {
		if (lc_time_item[k][m] == j) {
			break;
		}
	}
	if (m == lc_time_count[k]) { /* new for this nl_item */
		if (m > 255) {
			error_msg("too many nl_item %d entries in lc_time", k);
		}
		lc_time_item[k][m] = j;
		++lc_time_count[k];
	}
	lc_time_uniq_X[lc_time_uniq][k] = m;
}

static void do_lc_time(void)
{
	int i, k, m;

	last = buf+1;
	uniq = 1;
	*buf = 0;
	*idx = buf;

	for (i=0 ; i < num_locales ; i++) {
		k = 0;

		if (!setlocale(LC_ALL, locales[i].glibc_name)) {
			verbose_msg(VDETAIL, "setlocale(LC_ALL,%s) failed!\n",
				   locales[i].glibc_name);
		}

		DO_NL_S(ABDAY_1);
		DO_NL_S(ABDAY_2);
		DO_NL_S(ABDAY_3);
		DO_NL_S(ABDAY_4);
		DO_NL_S(ABDAY_5);
		DO_NL_S(ABDAY_6);
		DO_NL_S(ABDAY_7);

		DO_NL_S(DAY_1);
		DO_NL_S(DAY_2);
		DO_NL_S(DAY_3);
		DO_NL_S(DAY_4);
		DO_NL_S(DAY_5);
		DO_NL_S(DAY_6);
		DO_NL_S(DAY_7);

		DO_NL_S(ABMON_1);
		DO_NL_S(ABMON_2);
		DO_NL_S(ABMON_3);
		DO_NL_S(ABMON_4);
		DO_NL_S(ABMON_5);
		DO_NL_S(ABMON_6);
		DO_NL_S(ABMON_7);
		DO_NL_S(ABMON_8);
		DO_NL_S(ABMON_9);
		DO_NL_S(ABMON_10);
		DO_NL_S(ABMON_11);
		DO_NL_S(ABMON_12);

		DO_NL_S(MON_1);
		DO_NL_S(MON_2);
		DO_NL_S(MON_3);
		DO_NL_S(MON_4);
		DO_NL_S(MON_5);
		DO_NL_S(MON_6);
		DO_NL_S(MON_7);
		DO_NL_S(MON_8);
		DO_NL_S(MON_9);
		DO_NL_S(MON_10);
		DO_NL_S(MON_11);
		DO_NL_S(MON_12);

		DO_NL_S(AM_STR);
		DO_NL_S(PM_STR);

		DO_NL_S(D_T_FMT);
		DO_NL_S(D_FMT);
		DO_NL_S(T_FMT);
		DO_NL_S(T_FMT_AMPM);
		DO_NL_S(ERA);

		DO_NL_S(ERA_YEAR);		/* non SuSv3 */
		DO_NL_S(ERA_D_FMT);
		DO_NL_S(ALT_DIGITS);
		DO_NL_S(ERA_D_T_FMT);
		DO_NL_S(ERA_T_FMT);

		DL_LC_LOOPTAIL(time)
	}

	DO_LC_COMMON(time)
}

#undef DO_NL_S

#define NUM_NL_numeric 3

static int lc_numeric_item[NUM_NL_numeric][256];
static int lc_numeric_count[NUM_NL_numeric];
static unsigned char lc_numeric_uniq_X[700][NUM_NL_numeric];
static int lc_numeric_uniq;

#define DO_NL_S(X)	lc_numeric_S(X, k++)

static void lc_numeric_S(int X, int k)
{
	int j, m;
	char buf[256];
	char *e;
	char *s;
	char c;

	s = nl_langinfo(X);
	if (X == GROUPING) {
		if (s) {
			if ((*s == CHAR_MAX) || (*s == -1)) { /* stupid glibc... :-( */
				s = "";
			}
			e = s;
			c = 0;
			while (*e) { /* find end of string */
				if (*e == CHAR_MAX) {
					c = CHAR_MAX;
					++e;
					break;
				}
				++e;
			}
			if ((e - s) > sizeof(buf)) {
				error_msg("grouping specifier too long");
			}
			strncpy(buf, s, (e-s));
			e = buf + (e-s);
			*e = 0;				/* Make sure we're null-terminated. */

			if (c != CHAR_MAX) { /* remove duplicate repeats */
				while (e > buf) {
					--e;
					if (*e != e[-1]) {
						break;
					}
				}
				*++e = 0;
			}
			s = buf;
		}
	}
	j = addstring(s);
	for (m=0 ; m < lc_numeric_count[k] ; m++) {
		if (lc_numeric_item[k][m] == j) {
			break;
		}
	}
	if (m == lc_numeric_count[k]) { /* new for this nl_item */
		if (m > 255) {
			error_msg("too many nl_item %d entries in lc_numeric", k);
		}
		lc_numeric_item[k][m] = j;
		++lc_numeric_count[k];
	}
/*  	verbose_msg(VDETAIL, "\\x%02x", m); */
	lc_numeric_uniq_X[lc_numeric_uniq][k] = m;
}

static void do_lc_numeric(void)
{
	int i, k, m;

	last = buf+1;
	uniq = 1;
	*buf = 0;
	*idx = buf;

	for (i=0 ; i < num_locales ; i++) {
		k = 0;

		if (!setlocale(LC_ALL, locales[i].glibc_name)) {
			verbose_msg(VDETAIL,"setlocale(LC_ALL,%s) failed!\n",
				   locales[i].glibc_name);
		}

		DO_NL_S(RADIXCHAR);		/* DECIMAL_POINT */
		DO_NL_S(THOUSEP);		/* THOUSANDS_SEP */
		DO_NL_S(GROUPING);

		DL_LC_LOOPTAIL(numeric)
	}

	DO_LC_COMMON(numeric)
}

#undef DO_NL_S

#define NUM_NL_monetary (7+14+1)

static int lc_monetary_item[NUM_NL_monetary][256];
static int lc_monetary_count[NUM_NL_monetary];
static unsigned char lc_monetary_uniq_X[700][NUM_NL_monetary];
static int lc_monetary_uniq;

#define DO_NL_S(X)	lc_monetary_S(X, k++)

/*  #define DO_NL_C(X)		verbose_msg(VDETAIL,"%#02x", (int)(unsigned char)(*nl_langinfo(X))); */
#define DO_NL_C(X) lc_monetary_C(X, k++)

static void lc_monetary_C(int X, int k)
{
	int j, m;
	char c_buf[2];

#warning fix the char entries for monetary... target signedness of char may be different!

	c_buf[1] = 0;
	c_buf[0] = *nl_langinfo(X);
	j = addstring(c_buf);
	for (m=0 ; m < lc_monetary_count[k] ; m++) {
		if (lc_monetary_item[k][m] == j) {
			break;
		}
	}
	if (m == lc_monetary_count[k]) { /* new for this nl_item */
		if (m > 255) {
			error_msg("too many nl_item %d entries in lc_monetary", k);
		}
		lc_monetary_item[k][m] = j;
		++lc_monetary_count[k];
	}
/*  	verbose_msg(VDETAIL,"\\x%02x", m); */
	lc_monetary_uniq_X[lc_monetary_uniq][k] = m;
}


static void lc_monetary_S(int X, int k)
{
	int j, m;
	char buf[256];
	char *e;
	char *s;
	char c;

	s = nl_langinfo(X);
	if (X == MON_GROUPING) {
		if (s) {
			if ((*s == CHAR_MAX) || (*s == -1)) { /* stupid glibc... :-( */
				s = "";
			}
			e = s;
			c = 0;
			while (*e) { /* find end of string */
				if (*e == CHAR_MAX) {
					c = CHAR_MAX;
					++e;
					break;
				}
				++e;
			}
			if ((e - s) > sizeof(buf)) {
				error_msg("mon_grouping specifier too long");
			}
			strncpy(buf, s, (e-s));
			e = buf + (e-s);
			*e = 0;				/* Make sure we're null-terminated. */

			if (c != CHAR_MAX) { /* remove duplicate repeats */
				while (e > buf) {
					--e;
					if (*e != e[-1]) {
						break;
					}
				}
				*++e = 0;
			}
			s = buf;
		}
	}
	j = addstring(s);
	for (m=0 ; m < lc_monetary_count[k] ; m++) {
		if (lc_monetary_item[k][m] == j) {
			break;
		}
	}
	if (m == lc_monetary_count[k]) { /* new for this nl_item */
		if (m > 255) {
			error_msg("too many nl_item %d entries in lc_monetary", k);
		}
		lc_monetary_item[k][m] = j;
		++lc_monetary_count[k];
	}
/*  	verbose_msg(VDETAIL,"\\x%02x", m); */
	lc_monetary_uniq_X[lc_monetary_uniq][k] = m;
}

static void do_lc_monetary(void)
{
	int i, k, m;

	last = buf+1;
	uniq = 1;
	*buf = 0;
	*idx = buf;

	for (i=0 ; i < num_locales ; i++) {
		k = 0;

		if (!setlocale(LC_ALL, locales[i].glibc_name)) {
			verbose_msg(VDETAIL,"setlocale(LC_ALL,%s) failed!\n",
				   locales[i].glibc_name);
		}


		/* non SUSv3 */
		DO_NL_S(INT_CURR_SYMBOL);
		DO_NL_S(CURRENCY_SYMBOL);
		DO_NL_S(MON_DECIMAL_POINT);
		DO_NL_S(MON_THOUSANDS_SEP);
		DO_NL_S(MON_GROUPING);
		DO_NL_S(POSITIVE_SIGN);
		DO_NL_S(NEGATIVE_SIGN);
		DO_NL_C(INT_FRAC_DIGITS);
		DO_NL_C(FRAC_DIGITS);
		DO_NL_C(P_CS_PRECEDES);
		DO_NL_C(P_SEP_BY_SPACE);
		DO_NL_C(N_CS_PRECEDES);
		DO_NL_C(N_SEP_BY_SPACE);
		DO_NL_C(P_SIGN_POSN);
		DO_NL_C(N_SIGN_POSN);
		DO_NL_C(INT_P_CS_PRECEDES);
		DO_NL_C(INT_P_SEP_BY_SPACE);
		DO_NL_C(INT_N_CS_PRECEDES);
		DO_NL_C(INT_N_SEP_BY_SPACE);
		DO_NL_C(INT_P_SIGN_POSN);
		DO_NL_C(INT_N_SIGN_POSN);

		DO_NL_S(CRNCYSTR);		/* CURRENCY_SYMBOL */

		DL_LC_LOOPTAIL(monetary)
	}

	DO_LC_COMMON(monetary)
}


#undef DO_NL_S

#define NUM_NL_messages 4

static int lc_messages_item[NUM_NL_messages][256];
static int lc_messages_count[NUM_NL_messages];
static unsigned char lc_messages_uniq_X[700][NUM_NL_messages];
static int lc_messages_uniq;

#define DO_NL_S(X)	lc_messages_S(X, k++)

static void lc_messages_S(int X, int k)
{
	int j, m;
	j = addstring(nl_langinfo(X));
	for (m=0 ; m < lc_messages_count[k] ; m++) {
		if (lc_messages_item[k][m] == j) {
			break;
		}
	}
	if (m == lc_messages_count[k]) { /* new for this nl_item */
		if (m > 255) {
			error_msg("too many nl_item %d entries in lc_messages", k);
		}
		lc_messages_item[k][m] = j;
		++lc_messages_count[k];
	}
/*  	verbose_msg(VDETAIL, "\\x%02x", m); */
	lc_messages_uniq_X[lc_messages_uniq][k] = m;
}

static void do_lc_messages(void)
{
	int i, k, m;

	last = buf+1;
	uniq = 1;
	*buf = 0;
	*idx = buf;

	for (i=0 ; i < num_locales ; i++) {
		k = 0;

		if (!setlocale(LC_ALL, locales[i].glibc_name)) {
			verbose_msg(VDETAIL, "setlocale(LC_ALL,%s) failed!\n",
				   locales[i].glibc_name);
		}

		DO_NL_S(YESEXPR);
		DO_NL_S(NOEXPR);
		DO_NL_S(YESSTR);
		DO_NL_S(NOSTR);

		DL_LC_LOOPTAIL(messages)
	}

	DO_LC_COMMON(messages)
}

#undef DO_NL_S

#define NUM_NL_ctype 10

static int lc_ctype_item[NUM_NL_ctype][256];
static int lc_ctype_count[NUM_NL_ctype];
static unsigned char lc_ctype_uniq_X[700][NUM_NL_ctype];
static int lc_ctype_uniq;

#define DO_NL_S(X)	lc_ctype_S(X, k++)

static void lc_ctype_S(int X, int k)
{
	int j, m;
	j = addstring(nl_langinfo(X));
	for (m=0 ; m < lc_ctype_count[k] ; m++) {
		if (lc_ctype_item[k][m] == j) {
			break;
		}
	}
	if (m == lc_ctype_count[k]) { /* new for this nl_item */
		if (m > 255) {
			error_msg("too many nl_item %d entries in lc_ctype", k);
		}
		lc_ctype_item[k][m] = j;
		++lc_ctype_count[k];
	}
/*  	verbose_msg(VDETAIL, "\\x%02x", m); */
	lc_ctype_uniq_X[lc_ctype_uniq][k] = m;
}

static void do_lc_ctype(void)
{
	int i, k, m;

	last = buf+1;
	uniq = 1;
	*buf = 0;
	*idx = buf;

	for (i=0 ; i < num_locales ; i++) {
		k = 0;

		if (!setlocale(LC_ALL, locales[i].glibc_name)) {
			verbose_msg(VDETAIL, "setlocale(LC_ALL,%s) failed!\n",
				   locales[i].glibc_name);
		}

		DO_NL_S(_NL_CTYPE_OUTDIGIT0_MB);
		DO_NL_S(_NL_CTYPE_OUTDIGIT1_MB);
		DO_NL_S(_NL_CTYPE_OUTDIGIT2_MB);
		DO_NL_S(_NL_CTYPE_OUTDIGIT3_MB);
		DO_NL_S(_NL_CTYPE_OUTDIGIT4_MB);
		DO_NL_S(_NL_CTYPE_OUTDIGIT5_MB);
		DO_NL_S(_NL_CTYPE_OUTDIGIT6_MB);
		DO_NL_S(_NL_CTYPE_OUTDIGIT7_MB);
		DO_NL_S(_NL_CTYPE_OUTDIGIT8_MB);
		DO_NL_S(_NL_CTYPE_OUTDIGIT9_MB);

		DL_LC_LOOPTAIL(ctype)
	}

	DO_LC_COMMON(ctype)
}
