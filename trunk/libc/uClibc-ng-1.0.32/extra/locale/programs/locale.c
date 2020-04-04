/*
 *
 * Copyright (c) 2008  STMicroelectronics Ltd
 * Filippo Arcidiacono (filippo.arcidiacono@st.com)
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 * A 'locale' command implementation for uClibc.
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <langinfo.h>
#include <unistd.h>
#ifdef __UCLIBC_HAS_GETOPT_LONG__
#include <getopt.h>
#endif

typedef struct {
	unsigned char idx_name;
	char dot_cs;		/* 0 if no codeset specified */
	char cs;
	unsigned char lc_ctype_row;
	unsigned char lc_numeric_row;
	unsigned char lc_monetary_row;
	unsigned char lc_time_row;
	unsigned char lc_collate_row;
	unsigned char lc_messages_row;
} locale_entry;

/* Need to include this before locale.h! */
#include <bits/uClibc_locale.h>

#undef CODESET_LIST
#define CODESET_LIST			(__locale_mmap->codeset_list)
#include <locale.h>
#define LOCALE_NAMES			(__locale_mmap->locale_names5)
#define LOCALES					(__locale_mmap->locales)
#define LOCALE_AT_MODIFIERS	(__locale_mmap->locale_at_modifiers)
#define CATEGORY_NAMES			(__locale_mmap->lc_names)

#define GET_CODESET_NAME(N)  (const char *)(CODESET_LIST + *(CODESET_LIST + N - 3))
#define GET_LOCALE_ENTRY(R)  (locale_entry *)(LOCALES + (__LOCALE_DATA_WIDTH_LOCALES * R))
#define GET_CATEGORY_NAME(X) (CATEGORY_NAMES + *(CATEGORY_NAMES + X))
#define GET_LOCALE_NAME(I)   (const char *)(LOCALE_NAMES + 5 * (I - 1))

static const char utf8[] = "UTF-8";
static const char ascii[] = "ASCII";

/* If set print the name of the category.  */
static int show_category_name = 0;

/* If set print the name of the item.  */
static int show_keyword_name = 0;

/* If set print the usage command.  */
static int show_usage = 0;

/* Print names of all available locales.  */
static int do_all = 0;

/* Print names of all available character maps.  */
static int do_charmaps = 0;

static int remaining = 0;

/* We can map the types of the entries into a few categories.  */
enum value_type {
	none,
	string,
	stringarray,
	byte,
	bytearray,
	word,
	stringlist,
	wordarray,
	wstring,
	wstringarray,
	wstringlist
};

/* Definition of the data structure which represents a category and its
   items.  */
struct category {
	int cat_id;
	const char *name;
	size_t number;
	struct cat_item {
		int item_id;
		const char *name;
		enum { std, opt } status;
		enum value_type value_type;
		int min;
		int max;
	} *item_desc;
};

/* Simple helper macro.  */
#define NELEMS(arr) ((sizeof (arr)) / (sizeof (arr[0])))

/* For some tricky stuff.  */
#define NO_PAREN(Item, More...) Item, ## More

/* We have all categories defined in `categories.def'.  Now construct
   the description and data structure used for all categories.  */
#define DEFINE_ELEMENT(Item, More...) { Item, ## More },
#define DEFINE_CATEGORY(category, name, items, postload) \
    static struct cat_item category##_desc[] =				      \
      {									      \
        NO_PAREN items							      \
      };

#include "categories.def"
#undef DEFINE_CATEGORY

static struct category category[] = {
#define DEFINE_CATEGORY(category, name, items, postload) \
    [category] = { _NL_NUM_##category, name, NELEMS (category##_desc),	      \
		   category##_desc },
#include "categories.def"
#undef DEFINE_CATEGORY
};

#define NCATEGORIES NELEMS (category)

static void usage(const char *name);
static void usage(const char *name)
{
	const char *s;

	s = basename(name);
#ifdef __UCLIBC_HAS_GETOPT_LONG__
	fprintf(stderr,
			"Usage: %s [-a | -m] [FORMAT] name...\n\n"
			"\t-a, --all-locales\tWrite names of all available locales\n"
			"\t-m, --charmaps\tWrite names of available charmaps\n"
			"\nFORMAT:\n"
			"\t-c, --category-name\tWrite names of selected categories\n"
			"\t-k, --keyword-name\tWrite names of selected keywords\n"
			, s);
#else
	fprintf(stderr,
			"Usage: %s [-a | -m] [FORMAT] name...\n\n"
			"\t-a\tWrite names of all available locales\n"
			"\t-m\tWrite names of available charmaps\n"
			"\nFORMAT:\n"
			"\t-c\tWrite names of selected categories\n"
			"\t-k\tWrite names of selected keywords\n"
			, s);
#endif
}

static int argp_parse(int argc, char *argv[]);
static int argp_parse(int argc, char *argv[])
{
	int c;
	char *progname;
#ifdef __UCLIBC_HAS_GETOPT_LONG__
	static const struct option long_options[] = {
		{"all-locales", no_argument, NULL, 'a'},
		{"charmaps", no_argument, NULL, 'm'},
		{"category-name", no_argument, NULL, 'c'},
		{"keyword-name", no_argument, NULL, 'k'},
		{"help", no_argument, NULL, 'h'},
		{NULL, 0, NULL, 0}};
#endif
	progname = *argv;
#ifdef __UCLIBC_HAS_GETOPT_LONG__
	while ((c = getopt_long(argc, argv, "amckh", long_options, NULL)) >= 0)
#else
	while ((c = getopt(argc, argv, "amckh")) >= 0)
#endif
		switch (c) {
		case 'a':
			do_all = 1;
			break;
		case 'c':
			show_category_name = 1;
			break;
		case 'm':
			do_charmaps = 1;
			break;
		case 'k':
			show_keyword_name = 1;
			break;
		case 'h':
			show_usage = 1;
			break;
		case '?':
			fprintf(stderr, "Unknown option.\n");
			usage(progname);
			return 1;

		default:
			fprintf(stderr, "This should never happen!\n");
			return 1;
		}

	remaining = optind;

	return 0;
}

static unsigned const char *find_at(char c);
static unsigned const char *find_at(char c)
{
	const unsigned char *q;

	q = LOCALE_AT_MODIFIERS;
	do {
		if (q[1] == c) {
			return (unsigned const char *) q + 2;
		}
		q += 2 + *q;
	} while (*q);

	return NULL;
}

static void find_locale_string(locale_entry * loc_rec, char *loc)
{
	char at = 0;
	unsigned char idx;
	uint16_t dotcs, cs;

	idx = loc_rec->idx_name;
	if (!idx) {
		*loc++ = 'C';	/* jump the first locale (C) */
		*loc = '\0';
	} else {
		dotcs = (uint16_t) loc_rec->dot_cs;
		cs = (uint16_t) loc_rec->cs;;
		loc = strncpy(loc, GET_LOCALE_NAME(idx), 5);

		if (loc[2] == '_') {
			sprintf(loc, "%5.5s%c%s\0", loc, (dotcs != 0) ? '.' : ' ',
					(cs == 1) ? ascii
							: ((cs == 2) ?
										utf8
: GET_CODESET_NAME(cs)));
		} else {
			at = loc[2];
			loc[2] = '_';
			sprintf(loc, "%5.5s%c%s@%s\0", loc, (dotcs != 0) ? '.' : ' ',
					(cs ==
					 1) ? ascii : ((cs == 2) ? utf8 : GET_CODESET_NAME(cs)),
					find_at(at));
		}
	}
}

static void list_locale(void);
static void list_locale()
{
	char loc[40];
	uint16_t n = 0;
	locale_entry *locales = (locale_entry *) LOCALES;

	do {
		find_locale_string(locales, loc);
		printf("%s\n", loc);
		++n;
		locales++;
	} while (n < __LOCALE_DATA_NUM_LOCALES);
}

static void list_charmaps(void);
static void list_charmaps()
{
	unsigned const char *cl;

	cl = CODESET_LIST;
	do {
		printf("%s\n", CODESET_LIST + *cl);
	} while (*++cl);

}

static void print_item(struct cat_item *item);
static void print_item(struct cat_item *item)
{
	switch (item->value_type) {
	case string:
		if (show_keyword_name)
			printf("%s=\"", item->name);
		fputs(nl_langinfo(item->item_id) ? : "", stdout);
		if (show_keyword_name)
			putchar('"');
		putchar('\n');
		break;
	case stringarray:
	{
		int cnt;
		const char *val;

		if (show_keyword_name)
			printf("%s=\"", item->name);

		for (cnt = 0; cnt < item->max - 1; ++cnt) {
			val = nl_langinfo(item->item_id + cnt);
			if (val != NULL)
				fputs(val, stdout);
			putchar(';');
		}

		val = nl_langinfo(item->item_id + cnt);
		if (val != NULL)
			fputs(val, stdout);

		if (show_keyword_name)
			putchar('"');
		putchar('\n');
	}
		break;
	case stringlist:
	{
		int first = 1;
		const char *val = nl_langinfo(item->item_id) ? : "";
		int cnt;

		if (show_keyword_name)
			printf("%s=", item->name);

		for (cnt = 0; cnt < item->max && *val != '\0'; ++cnt) {
			printf("%s%s%s%s", first ? "" : ";",
				   show_keyword_name ? "\"" : "", val,
				   show_keyword_name ? "\"" : "");
			val = strchr(val, '\0') + 1;
			first = 0;
		}
		putchar('\n');
	}
		break;
	case byte:
	{
		const char *val = nl_langinfo(item->item_id);

		if (show_keyword_name)
			printf("%s=", item->name);

		if (val != NULL)
			printf("%d", *val == '\177' ? -1 : *val);
		putchar('\n');
	}
		break;
	case bytearray:
	{
		const char *val = nl_langinfo(item->item_id);
		int cnt = val ? strlen(val) : 0;

		if (show_keyword_name)
			printf("%s=", item->name);

		while (cnt > 1) {
			printf("%d;", *val == '\177' ? -1 : *val);
			--cnt;
			++val;
		}

		printf("%d\n", cnt == 0 || *val == '\177' ? -1 : *val);
	}
		break;
	case word:
	{
		union {
			unsigned int word;
			char *string;
		} val;

		val.string = nl_langinfo(item->item_id);
		if (show_keyword_name)
			printf("%s=", item->name);

		printf("%d\n", val.word);
	}
		break;
	case wstring:
	case wstringarray:
	case wstringlist:
		/* We don't print wide character information since the same
		   information is available in a multibyte string.  */
	default:
		break;

	}
}

/* Show the information request for NAME.  */
static void show_info(const char *name);
static void show_info(const char *name)
{
	size_t cat_no, item_no;
	const unsigned char *cat_name;

	/* Now all categories in an unspecified order.  */
	for (cat_no = 0; cat_no < __LC_ALL; ++cat_no) {
		cat_name = GET_CATEGORY_NAME(cat_no);
		if (strcmp(name, (const char *) cat_name) == 0) {
			if (show_category_name)
				printf("%s\n", name);

			for (item_no = 0; item_no < category[cat_no].number; ++item_no)
				print_item(&category[cat_no].item_desc[item_no]);

			return;
		}

		for (item_no = 0; item_no < category[cat_no].number; ++item_no)
			if (strcmp(name, category[cat_no].item_desc[item_no].name) == 0) {
				if (show_category_name != 0)
					puts(category[cat_no].name);

				print_item(&category[cat_no].item_desc[item_no]);
				return;
			}
	}
}

static void show_locale_vars(void);
static void show_locale_vars()
{
	size_t cat_no;
	int row;			/* locale row */
	const char *lcall = getenv("LC_ALL");
	const char *lang = getenv("LANG") ? : "";
	unsigned char *cur_loc = __global_locale->cur_locale + 1;
	char loc_name[40];
	locale_entry *locales;

	/* LANG has to be the first value.  */
	printf("LANG=%s\n", lang);

	/* Now all categories in an unspecified order.  */
	for (cat_no = 0; cat_no < __LC_ALL; ++cat_no) {
		row = (((int) (*cur_loc & 0x7f)) << 7) + (cur_loc[1] & 0x7f);
/*		assert(row < __LOCALE_DATA_NUM_LOCALES); */

		locales = GET_LOCALE_ENTRY(row);
		find_locale_string(locales, loc_name);
		printf("%s=%s\n", GET_CATEGORY_NAME(cat_no), loc_name);

		cur_loc += 2;
	}

	/* The last is the LC_ALL value.  */
	printf("LC_ALL=%s\n", lcall ? : "");
}

int main(int argc, char *argv[])
{
	/* Parse and process arguments.  */
	if (argp_parse(argc, argv))
		return 1;

	if (do_all) {
		list_locale();
		exit(EXIT_SUCCESS);
	}

	if (do_charmaps) {
		list_charmaps();
		exit(EXIT_SUCCESS);
	}

	if (show_usage) {
		usage(*argv);
		exit(EXIT_SUCCESS);
	}

	/* If no real argument is given we have to print the contents of the
	   current locale definition variables. These are LANG and the LC_*.  */
	if (remaining == argc && show_category_name == 0
		&& show_keyword_name == 0) {
		show_locale_vars();
		exit(EXIT_SUCCESS);
	}

	/* Process all given names.  */
	while (remaining < argc)
		show_info(argv[remaining++]);

	exit(EXIT_SUCCESS);
}
