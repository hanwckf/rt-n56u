/* TODO:
 *
 * add UNDEFINED at end if not specified
 * convert POSITION -> FORWARD,POSITION
 *
 *
 * deal with lowercase in <Uhhhh>
 *
 * what about reorders that keep the same rule?
 *
 * remove "unused" collation elements? (probably doesn't save much)
 *
 * add_rule function ... returns index into rule table after possibly adding custom-indexed rule
 * but don't forget about multichar weights... replace with strings of indexes
 *
 */


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>
#include <search.h>

typedef struct {
	char *name;					/*  */

	int num_weights;			/*  */

	int ii_shift;				/*  */
	int ti_shift;				/*  */
	int ii_len;					/*  */
	int ti_len;					/*  */
	int max_weight;				/*  */
	int num_col_base;			/*  */
	int max_col_index;			/*  */
	int undefined_idx;			/*  */
	int range_low;				/*  */
	int range_count;			/* high - low */
	int range_base_weight;		/*  */
	int num_starters;			/*  */

	int range_rule_offset;		/*  */
	int wcs2colidt_offset;		/*  */
	int index2weight_offset;	/*  */
	int index2ruleidx_offset;	/*  */
	int multistart_offset;		/*  */

} base_locale_t;

#define BASE_LOCALE_LEN 20
static base_locale_t base_locale_array[BASE_LOCALE_LEN];
static size_t base_locale_len;

typedef struct {
	char *name;					/*  */

	int base_idx;				/*  */

	int undefined_idx;			/*  */
	
	int overrides_offset;		/*  */
	int multistart_offset;		/*  */
} der_locale_t;

#define DER_LOCALE_LEN 300
static der_locale_t der_locale_array[DER_LOCALE_LEN];
static size_t der_locale_len;


#define OVERRIDE_LEN  50000
static uint16_t override_buffer[OVERRIDE_LEN];
static size_t override_len;

#define MULTISTART_LEN 10000
static uint16_t multistart_buffer[MULTISTART_LEN];
static size_t multistart_len;

#define WCS2COLIDT_LEN 200000
static uint16_t wcs2colidt_buffer[WCS2COLIDT_LEN];
static size_t wcs2colidt_len;

#define INDEX2WEIGHT_LEN 200000
static uint16_t index2weight_buffer[INDEX2WEIGHT_LEN];
static size_t index2weight_len;

static uint16_t index2ruleidx_buffer[INDEX2WEIGHT_LEN];
static size_t index2ruleidx_len;

#define WEIGHTSTR_LEN 10000
static uint16_t weightstr_buffer[WEIGHTSTR_LEN];
static size_t weightstr_len;

#define RULETABLE_LEN (1L<<16)
static uint16_t ruletable_buffer[RULETABLE_LEN];
static size_t ruletable_len;


#define RANGE (0x10000UL)

typedef uint16_t tbl_item;

static uint16_t u16_buf[10000];
static int u16_buf_len;
static int u16_starter;

typedef struct {
	uint16_t ii_len;
	uint16_t ti_len;
	uint16_t ut_len;

	unsigned char ii_shift;
	unsigned char ti_shift;

	tbl_item *ii;
	tbl_item *ti;
	tbl_item *ut;
} table_data;


static size_t newopt(tbl_item *ut, size_t usize, int shift, table_data *tbl);


#define MAX_COLLATION_WEIGHTS 4

#define MAX_FNO 1
#define MAX_FILES  (MAX_FNO + 1)

static FILE *fstack[MAX_FILES];
static char *fname[MAX_FILES];
static int lineno[MAX_FILES];
static int fno = -1;

static tbl_item wcs2index[RANGE];

static char linebuf[1024];
static char *pos;
static char *pos_e = NULL;
static char end_of_token = 0;		/* slot to save */

#define IN_ORDER			0x01
#define IN_REORDER			0x02
#define IN_REORDER_SECTIONS	0x04
static int order_state;
static int cur_num_weights;		/* number of weights in current use */
static char cur_rule[MAX_COLLATION_WEIGHTS];

static int anonsection = 0;

typedef struct ll_item_struct ll_item_t;

struct ll_item_struct {
	ll_item_t *next;
	ll_item_t *prev;
	void *data;
	int data_type;
	int idx;
};

static ll_item_t *reorder_section_ptr = NULL;
static int superset;
static int superset_order_start_cnt; /* only support one order for now */
static int superset_in_sync;
static ll_item_t *comm_cur_ptr;
static ll_item_t *comm_prev_ptr;

enum {
	R_FORWARD =		0x01,
	R_POSITION =	0x02,
	R_BACKWARD =	0x04		/* must be largest in value */
};

typedef struct {
	size_t num_weights;
	char rule[MAX_COLLATION_WEIGHTS];
	const char *colitem[MAX_COLLATION_WEIGHTS];
} weight_t;

static void *root_weight = NULL;
size_t unique_weights = 0;

typedef struct {
	const char *symbol;
	weight_t *weight;
} weighted_item_t;

typedef struct {
	const char *symbol1;
	const char *symbol2;
	int length;
	weight_t *weight;
} range_item_t;

typedef struct {
	const char *name;
	ll_item_t *itm_list;		/* weighted_item_t list .. circular!!! */
	size_t num_items;
	size_t num_rules;
	char rules[MAX_COLLATION_WEIGHTS];
} section_t;

static section_t *cur_section = NULL;

typedef struct {
	const char *symbol;
	ll_item_t *node;
} wi_index_t;

typedef struct col_locale_struct col_locale_t;

struct  col_locale_struct {
	char *name;
	void *root_colitem;			/* all base and derived, or just derived */
	void *root_element;
	void *root_scripts;
	void *root_wi_index;
	void *root_wi_index_reordered;
	ll_item_t *section_list;
	col_locale_t *base_locale;	/* null if this is a base */
	void *root_derived_wi;
	ll_item_t *derived_list;
	void *root_starter_char;
	void *root_starter_all;
	ll_item_t *undefined_idx;
};

typedef struct {
	const char *symbol;
	int idx;
} col_index_t;

static void *root_col_locale = NULL;

typedef struct {
    const char *keyword;
    void (*handler)(void);
} keyword_table_t;

typedef struct {
    const char *string;
    const char *element;	/* NULL if collating symbol */
} colitem_t;

static col_locale_t *cur_base = NULL;
static col_locale_t *cur_derived = NULL;
static col_locale_t *cur_col = NULL;

static void *root_sym = NULL;
static size_t num_sym = 0;
static size_t mem_sym = 0;

static void error_msg(const char *fmt, ...) __attribute__ ((noreturn, format (printf, 1, 2)));
static void *xmalloc(size_t n);
static char *xsymdup(const char *s); /* only allocate once... store in a tree */
static void pushfile(char *filename);
static void popfile(void);
static void processfile(void);
static int iscommentchar(int);
static void eatwhitespace(void);
static int next_line(void);
static char *next_token(void);
static void do_unrecognized(void);
static col_locale_t *new_col_locale(char *name);
static ll_item_t *new_ll_item(int data_type, void *data);
static weight_t *register_weight(weight_t *w);
static size_t ll_len(ll_item_t *l);
static size_t ll_count(ll_item_t *l, int mask);
static void add_wi_index(ll_item_t *l);
static size_t tnumnodes(const void *root);
static ll_item_t *find_wi_index(const char *sym, col_locale_t *cl);
static void mark_reordered(const char *sym);
static ll_item_t *find_wi_index_reordered(const char *sym);
static ll_item_t *next_comm_ptr(void);
static ll_item_t *init_comm_ptr(void);
static ll_item_t *find_ll_last(ll_item_t *p);
static void dump_weights(const char *name);
static void finalize_base(void);
static int is_ucode(const char *s);
static int sym_cmp(const void *n1, const void *n2);
static void do_starter_lists(col_locale_t *cl);
static void dump_base_locale(int n);
static void dump_der_locale(int n);
static void dump_collate(FILE *fp);

enum {
	DT_SECTION = 0x01,
	DT_WEIGHTED = 0x02,
	DT_REORDER = 0x04,		  /* a section to support reorder_after */
	DT_COL_LOCALE = 0x08,
	DT_RANGE = 0x10,
};

static section_t *new_section(const char *name)
{
	section_t *p;
	char buf[128];

	p = xmalloc(sizeof(section_t));
	if (!name) {				/* anonymous section */
		name = buf;
		snprintf(buf, sizeof(buf), "anon%05d", anonsection);
		++anonsection;
	} else if (*name != '<') {	/* reorder */
		name = buf;
		snprintf(buf, sizeof(buf), "%s %05d", cur_col->name, anonsection);
		++anonsection;
	}
#warning devel code
/* 	fprintf(stderr, "section %s\n", name); */
	p->name = xsymdup(name);
	p->itm_list = NULL;
	p->num_items = 0;
	p->num_rules = 0;
	memset(p->rules, 0, MAX_COLLATION_WEIGHTS);
/* 	cur_num_weights = p->num_rules = 0; */
/* 	memset(p->rules, 0, MAX_COLLATION_WEIGHTS); */
/* 	memset(cur_rule, R_FORWARD, 4); */

#warning devel code
	if (*p->name == 'a') {
		cur_num_weights = p->num_rules = 4;
		memset(p->rules, R_FORWARD, 4);
		memset(cur_rule, R_FORWARD, 4);
		p->rules[3] |= R_POSITION;
		cur_rule[3] |= R_POSITION;
	}
/* 	fprintf(stderr, "new section %s -- cur_num_weights = %d\n", p->name, cur_num_weights); */

	return p;
}



static void do_order_start(void);
static void do_order_end(void);
static void do_reorder_after(void);
static void do_reorder_end(void);
static void do_reorder_sections_after(void);
static void do_reorder_sections_end(void);
static void do_copy(void);
static void do_colsym(void);
static void do_colele(void);
static void do_script(void);
static void do_range(void);

static col_locale_t *new_col_locale(char *name);
static int colitem_cmp(const void *n1, const void *n2);
static int colelement_cmp(const void *n1, const void *n2);
static void del_colitem(colitem_t *p);
static colitem_t *new_colitem(char *item, char *def);
static void add_colitem(char *item, char *def);
static void add_script(const char *s);
static unsigned int add_rule(weighted_item_t *wi);
static unsigned int add_range_rule(range_item_t *ri);

static const keyword_table_t keyword_table[] = {
    { "collating-symbol", do_colsym },
    { "collating-element", do_colele },
	{ "script", do_script },
    { "copy", do_copy },
    { "order_start", do_order_start },
    { "order_end", do_order_end },
    { "order-end", do_order_end },
    { "reorder-after", do_reorder_after },
    { "reorder-end", do_reorder_end },
    { "reorder-sections-after", do_reorder_sections_after },
    { "reorder-sections-end", do_reorder_sections_end },
	{ "UCLIBC_RANGE", do_range },
    { NULL, do_unrecognized }
};


static void do_unrecognized(void)
{
#if 1
    error_msg("warning: unrecognized: %s", pos);
#else
/*     fprintf(stderr, "warning: unrecognized initial keyword \"%s\"\n", pos); */
	fprintf(stderr, "warning: unrecognized: %s", pos);
	if (end_of_token) {
		fprintf(stderr, "%c%s", end_of_token, pos_e+1);
	}
	fprintf(stderr, "\n");
#endif
}

/* typedef struct { */
/* 	const char *symbol1; */
/* 	const char *symbol2; */
/* 	int length; */
/* 	weight_t *weight; */
/* } range_item_t; */

static void do_range(void)
{
	range_item_t *ri;
	weight_t w;
	int i;
	char *s;
	char *s1;
	char *s2;
	const char **ci;
	ll_item_t *lli;

	assert(!superset);
	assert(order_state == IN_ORDER);

	s1 = next_token();
	if (!s1) {
		error_msg("missing start of range");
	}
	if (!is_ucode(s1)) {
		error_msg("start of range is not a ucode: %s", s1);
	}
	s1 = xsymdup(s1);

	s2 = next_token();
	if (!s2) {
		error_msg("missing end of range");
	}
	if (!is_ucode(s2)) {
		error_msg("end of range is not a ucode: %s", s2);
	}
	s2 = xsymdup(s2);

	ri = (range_item_t *) xmalloc(sizeof(range_item_t));
	ri->symbol1 = s1;
	ri->symbol2 = s2;
	ri->length = strtoul(s2+2, NULL, 16) - strtoul(s1+2, NULL, 16);
	if (ri->length <= 0) {
		error_msg("illegal range length %d", ri->length);
	}

	s = next_token();
	w.num_weights = cur_num_weights;

	for (i=0 ; i < cur_num_weights ; i++) {
		w.rule[i] = cur_rule[i];
	}
	ci = w.colitem + (i-1);
	/* now i == cur_num_weights */

#define STR_DITTO "."

	while (s && *s && i) {
		--i;
		if (*s == ';') {
			ci[-i] = xsymdup(STR_DITTO);
			if (*++s) {
				continue;
			}
		}
		if (*s) {
			ci[-i] = xsymdup(s);
		}
		s = next_token();
		if (s) {
			if (*s == ';') {
				++s;
			} else if (i) {
				error_msg("missing seperator");
			}
		}
	}
	if (s) {
		error_msg("too many weights: %d %d |%s| %d", cur_num_weights, i, s, (int)*s);
	}

	while (i) {					/* missing weights are not an error */
		--i;
		ci[-i] = xsymdup(STR_DITTO);
	}

	ri->weight = register_weight(&w);

/* 	if ((i = is_ucode(t)) != 0) { */
/* 		assert(!t[i]); */
/* 		add_colitem(t, NULL); */
/* 	} */

	lli = new_ll_item(DT_RANGE, ri);
	if (!cur_section->itm_list) {
/* 		printf("creating new item list: %s\n", wi->symbol); */
		cur_section->itm_list = lli;
		lli->prev = lli->next = lli;
		++cur_section->num_items;
	} else {
		insque(lli, cur_section->itm_list->prev);
/* 		printf("adding item to list: %d - %s\n", ll_len(cur_section->itm_list), wi->symbol); */
		++cur_section->num_items;
	}
/* 	add_wi_index(lli); */


}

static weighted_item_t *add_weight(char *t)
{
	weighted_item_t *wi;
	weight_t w;
	int i;
	char *s;
	const char **ci;

	t = xsymdup(t);

	s = next_token();
	w.num_weights = cur_num_weights;

	for (i=0 ; i < cur_num_weights ; i++) {
		w.rule[i] = cur_rule[i];
	}
	ci = w.colitem + (i-1);
	/* now i == cur_num_weights */

	while (s && *s && i) {
		--i;
		if (*s == ';') {
			ci[-i] = xsymdup(STR_DITTO);
			if (*++s) {
				continue;
			}
		}
		if (*s) {
			if (!strcmp(s,t)) {
				s = STR_DITTO;
			}
			ci[-i] = xsymdup(s);
		}
		s = next_token();
		if (s) {
			if (*s == ';') {
				++s;
			} else if (i) {
				error_msg("missing seperator");
			}
		}
	}
	if (s) {
		error_msg("too many weights: %d %d |%s| %d", cur_num_weights, i, s, (int)*s);
	}

	while (i) {					/* missing weights are not an error */
		--i;
		ci[-i] = xsymdup(STR_DITTO);
	}

	wi = xmalloc(sizeof(weighted_item_t));
	wi->symbol = t;
	wi->weight = register_weight(&w);

	if ((i = is_ucode(t)) != 0) {
		assert(!t[i]);
		add_colitem(t, NULL);
	}

	return wi;
}

static void add_superset_weight(char *t)
{
	ll_item_t *lli;
	weighted_item_t *wi;

	if (!comm_cur_ptr
		|| (strcmp(t, ((weighted_item_t *)(comm_cur_ptr->data))->symbol) != 0)
		) {						/* now out of sync */
		if (superset_in_sync) {	/* need a new section */
			superset_in_sync = 0;

			cur_section = new_section("R");
			cur_num_weights = cur_section->num_rules
				= ((section_t *)(cur_base->section_list->data))->num_rules;
			memcpy(cur_rule,
				   ((section_t *)(cur_base->section_list->data))->rules,
				   MAX_COLLATION_WEIGHTS);
			memcpy(cur_section->rules,
				   ((section_t *)(cur_base->section_list->data))->rules,
				   MAX_COLLATION_WEIGHTS);

			insque(new_ll_item(DT_REORDER, cur_section), find_ll_last(cur_col->section_list));
			assert(comm_prev_ptr);
			lli = new_ll_item(DT_REORDER, cur_section);
			lli->prev = lli->next = lli;
			insque(lli, comm_prev_ptr);
/* 			fprintf(stderr, "  subsection -----------------------\n"); */
		}

/* 		fprintf(stderr, "     %s   %s\n", t, ((weighted_item_t *)(comm_cur_ptr->data))->symbol); */
		wi = add_weight(t);
		lli = new_ll_item(DT_WEIGHTED, wi);
		mark_reordered(wi->symbol);
		/* 			printf("reorder: %s\n", t); */
		if (!cur_section->itm_list) {
			cur_section->itm_list = lli;
			lli->prev = lli->next = lli;
			++cur_section->num_items;
		} else {
			insque(lli, cur_section->itm_list->prev);
			++cur_section->num_items;
		}
		add_wi_index(lli);

	} else {					/* in sync */
		superset_in_sync = 1;
		next_comm_ptr();
	}
}

static void do_weight(char *t)
{
	weighted_item_t *wi;
	ll_item_t *lli;

	if (superset) {
		add_superset_weight(t);
		return;
	}

	switch(order_state) {
		case 0:
/* 			fprintf(stdout, "no-order weight: %s\n", t); */
/* 			break; */
		case IN_ORDER:
			/* in a section */
/* 			fprintf(stdout, "weight: %s\n", t); */
			wi = add_weight(t);
			lli = new_ll_item(DT_WEIGHTED, wi);
			if (!cur_section->itm_list) {
/* 				fprintf(stdout, "creating new item list: %s  %s  %p\n", wi->symbol, cur_section->name, lli); */
				cur_section->itm_list = lli;
				lli->prev = lli->next = lli;
				++cur_section->num_items;
			} else {
				insque(lli, cur_section->itm_list->prev);
/* 				fprintf(stdout, "adding item to list: %d - %s  %p\n", ll_len(cur_section->itm_list), wi->symbol, lli); */
				++cur_section->num_items;
			}
			add_wi_index(lli);
			break;
		case IN_REORDER:
			/* std rule - but in a block with an insert-after pt */
			wi = add_weight(t);
			lli = new_ll_item(DT_WEIGHTED, wi);
			mark_reordered(wi->symbol);
/* 			fprintf(stdout, "reorder: %s  %s  %p\n", t, cur_section->name, lli); */
			if (!cur_section->itm_list) {
				cur_section->itm_list = lli;
				lli->prev = lli->next = lli;
				++cur_section->num_items;
			} else {
				insque(lli, cur_section->itm_list->prev);
				++cur_section->num_items;
			}
			add_wi_index(lli);
			break;
		case IN_REORDER_SECTIONS:
			t = xsymdup(t);
			if (next_token() != NULL) {
				error_msg("trailing text in reorder section item: %s", pos);
			}
			lli = cur_col->section_list;
			do {
				if (lli->data_type & DT_SECTION) {
					if (!strcmp(((section_t *)(lli->data))->name, t)) {
						lli->data_type = DT_REORDER;
						lli = new_ll_item(DT_REORDER, (section_t *)(lli->data));
						insque(lli, reorder_section_ptr);
						reorder_section_ptr = lli;
						return;
					}
				}
				lli = lli->next;
			} while (lli);
			error_msg("reorder_sections_after for non-base item currently not supported: %s", t);
/* 			fprintf(stdout, "reorder_secitons: %s\n", t); */
			break;
		default:
			error_msg("invalid order_state %d", order_state);
	}
}

static int col_locale_cmp(const void *n1, const void *n2)
{
    return strcmp(((const col_locale_t *) n1)->name, ((const col_locale_t *) n2)->name);
}

static void processfile(void)
{
	char *t;
	const keyword_table_t *k;

	order_state = 0;
#warning devel code
/* 	cur_num_weights = 0; */
/* 	cur_num_weights = 4; */
/* 	memset(cur_rule, R_FORWARD, 4); */

	if (cur_col != cur_base) {
		cur_col->base_locale = cur_base;
		cur_col->undefined_idx = cur_base->undefined_idx;
		if (!cur_base->derived_list) {
			cur_base->derived_list = new_ll_item(DT_COL_LOCALE, cur_col);
		} else {
			insque(new_ll_item(DT_COL_LOCALE, cur_col), find_ll_last(cur_base->derived_list));
		}
	}

	if (tfind(cur_col, &root_col_locale, col_locale_cmp)) {
		error_msg("attempt to read locale: %s", cur_col->name);
	}
	if (!tsearch(cur_col, &root_col_locale, col_locale_cmp)) {
		error_msg("OUT OF MEMORY!");
	}

	if (superset) {
		superset_order_start_cnt = 0;
		superset_in_sync = 0;
		init_comm_ptr();
	}

	while (next_line()) {
/* 		printf("%5d:", lineno[fno]); */
/* 		while ((t = next_token()) != NULL) { */
/* 			printf(" |%s|", t); */
/* 		printf("\n"); */
/* 		} */
		t = next_token();
		assert(t);
		assert(t == pos);
		if ((*t == '<') || (!strcmp(t, "UNDEFINED"))) {
			do_weight(t);
		} else {
			for (k = keyword_table ; k->keyword ; k++) {
				if (!strcmp(k->keyword, t)) {
					break;
				}
			}
			k->handler();
		}
	}

	if (cur_base == cur_col) {
		fprintf(stderr, "Base: %15s", cur_col->name);
	} else {
#if 1
		if (!cur_col->undefined_idx) {
#if 0
			if (superset) {
				if (superset_order_start_cnt == 1) {
					--superset_order_start_cnt;	/* ugh.. hack this */
				}
			}
#endif
			/* This is an awful hack to get around the problem of unspecified UNDEFINED
			 * definitions in the supported locales derived from iso14651_t1. */
			if (!strcmp(cur_base->name, "iso14651_t1")) {
				fprintf(stderr, "Warning: adding UNDEFINED entry for %s\n", cur_col->name);
				strcpy(linebuf, "script <UNDEFINED_SECTION>\n");
				pos_e = NULL;
				pos = linebuf;
				t = next_token();
				assert(t);
				assert(t == pos);
				do_script();
				strcpy(linebuf, "order_start <UNDEFINED_SECTION>;forward;backward;forward;forward,position\n");
				pos_e = NULL;
				pos = linebuf;
				t = next_token();
				assert(t);
				assert(t == pos);
				do_order_start();
				strcpy(linebuf, "UNDEFINED IGNORE;IGNORE;IGNORE\n");
				pos_e = NULL;
				pos = linebuf;
				t = next_token();
				assert(t);
				assert(t == pos);
				do_weight(t);
				strcpy(linebuf, "order_end\n");
				pos_e = NULL;
				pos = linebuf;
				t = next_token();
				assert(t);
				assert(t == pos);
				do_order_end();
			} else {
				error_msg("no definition of UNDEFINED for %s", cur_col->name);
			}
		}
#endif

		fprintf(stderr, " Der: %15s", cur_col->name);
	}
	{
		ll_item_t *p = cur_col->section_list;

		fprintf(stderr, "%6u weights", tnumnodes(cur_col->root_wi_index));
		if (cur_base) {
			fprintf(stderr, "  %6u der %6u reor %6u starter - %u new stubs",
					tnumnodes(cur_base->root_derived_wi),
					tnumnodes(cur_base->root_wi_index_reordered),
					tnumnodes(cur_base->root_starter_char),
					ll_count(cur_col->section_list, DT_REORDER));
		}
		fprintf(stderr, "\n");

#if 0
		while (p) {
			assert(((section_t *)(p->data))->num_items ==
				   ll_len(((section_t *)(p->data))->itm_list));


			if (!p->next &&
				((*((section_t *)(p->data))->name == 'a')
				 && (((section_t *)(p->data))->num_items == 0))
				) {
				break;
			}

			if (!(p->data_type & DT_REORDER)) {
				if ((*((section_t *)(p->data))->name != 'a')
					|| (((section_t *)(p->data))->num_items > 0)
					) {
					fprintf(stderr,
/* 							"\t%-15s %zu\n", */
							"\t%-15s %6u\n",
							((section_t *)(p->data))->name,
							((section_t *)(p->data))->num_items);
				}
			}
			p = p->next;
		}
#endif
	}


}

static void print_colnode(const void *ptr, VISIT order, int level)
{
    const colitem_t *p = *(const colitem_t **) ptr;

    if (order == postorder || order == leaf)  {
        printf("collating item = \"%s\"", p->string);
		if (p->element) {
			printf(" is %s", p->element);
		}
        printf("\n");
    }
}

static void print_weight_node(const void *ptr, VISIT order, int level)
{
    const weight_t *p = *(const weight_t **) ptr;
	int i;

    if (order == postorder || order == leaf)  {
        printf("weight: (%d)  ", p->num_weights);
		for (i = 0 ; i < p->num_weights ; i++) {
			if (p->rule[i] & R_FORWARD) {
				printf("F");
			}
			if (p->rule[i] & R_BACKWARD) {
				printf("B");
			}
			if (p->rule[i] & R_POSITION) {
				printf("P");
			}
			printf(",");
		}
		for (i = 0 ; i < p->num_weights ; i++) {
			printf("   %s", p->colitem[i]);
		}
        printf("\n");
    }
}


typedef struct {
	const char *der_name;
	int base_locale;
} deps_t;

enum {
	BASE_iso14651_t1,
	BASE_comm,
	BASE_cs_CZ,
	BASE_ar_SA,
	BASE_th_TH,
	BASE_ja_JP,
	BASE_ko_KR,
	BASE_MAX
};

static const char *base_name[] = {
	"iso14651_t1",
	"comm",
	"cs_CZ",
	"ar_SA",
	"th_TH",
	"ja_JP",
	"ko_KR"
};



static ll_item_t *locale_list[BASE_MAX];

static void init_locale_list(void)
{
	int i;

	for (i=0 ; i < BASE_MAX ; i++) {
		locale_list[i] = (ll_item_t *) xmalloc(sizeof(ll_item_t));
		locale_list[i]->prev = locale_list[i]->next = locale_list[i];
		locale_list[i]->data = (void *) base_name[i];
	}
}


deps_t deps[] = {
	{ "af_ZA", BASE_iso14651_t1 },
	{ "am_ET", BASE_iso14651_t1 },
	{ "ar_AE", BASE_iso14651_t1 },
	{ "ar_BH", BASE_iso14651_t1 },
	{ "ar_DZ", BASE_iso14651_t1 },
	{ "ar_EG", BASE_iso14651_t1 },
	{ "ar_IN", BASE_iso14651_t1 },
	{ "ar_IQ", BASE_iso14651_t1 },
	{ "ar_JO", BASE_iso14651_t1 },
	{ "ar_KW", BASE_iso14651_t1 },
	{ "ar_LB", BASE_iso14651_t1 },
	{ "ar_LY", BASE_iso14651_t1 },
	{ "ar_MA", BASE_iso14651_t1 },
	{ "ar_OM", BASE_iso14651_t1 },
	{ "ar_QA", BASE_iso14651_t1 },
	{ "ar_SA", BASE_ar_SA },
	{ "ar_SD", BASE_iso14651_t1 },
	{ "ar_SY", BASE_iso14651_t1 },
	{ "ar_TN", BASE_iso14651_t1 },
	{ "ar_YE", BASE_iso14651_t1 },
	{ "az_AZ", BASE_iso14651_t1 },
	{ "be_BY", BASE_iso14651_t1 },
	{ "bg_BG", BASE_iso14651_t1 },
	{ "bn_BD", BASE_iso14651_t1 },
	{ "bn_IN", BASE_iso14651_t1 },
	{ "br_FR", BASE_iso14651_t1 },
	{ "bs_BA", BASE_iso14651_t1 },
	{ "ca_ES", BASE_comm },
	{ "cs_CZ", BASE_cs_CZ },
	{ "cy_GB", BASE_iso14651_t1 },
	{ "da_DK", BASE_comm },
	{ "de_AT", BASE_iso14651_t1 },
	{ "de_BE", BASE_iso14651_t1 },
	{ "de_CH", BASE_iso14651_t1 },
	{ "de_DE", BASE_iso14651_t1 },
	{ "de_LU", BASE_iso14651_t1 },
	{ "el_GR", BASE_iso14651_t1 },
	{ "en_AU", BASE_iso14651_t1 },
	{ "en_BW", BASE_iso14651_t1 },
	{ "en_CA", BASE_comm },
	{ "en_DK", BASE_iso14651_t1 },
	{ "en_GB", BASE_iso14651_t1 },
	{ "en_HK", BASE_iso14651_t1 },
	{ "en_IE", BASE_iso14651_t1 },
	{ "en_IN", BASE_iso14651_t1 },
	{ "en_NZ", BASE_iso14651_t1 },
	{ "en_PH", BASE_iso14651_t1 },
	{ "en_SG", BASE_iso14651_t1 },
	{ "en_US", BASE_iso14651_t1 },
	{ "en_ZA", BASE_iso14651_t1 },
	{ "en_ZW", BASE_iso14651_t1 },
	{ "eo_EO", BASE_iso14651_t1 },
	{ "es_AR", BASE_comm },
	{ "es_BO", BASE_comm },
	{ "es_CL", BASE_comm },
	{ "es_CO", BASE_comm },
	{ "es_CR", BASE_comm },
	{ "es_DO", BASE_comm },
	{ "es_EC", BASE_comm },
	{ "es_ES", BASE_comm },
	{ "es_GT", BASE_comm },
	{ "es_HN", BASE_comm },
	{ "es_MX", BASE_comm },
	{ "es_NI", BASE_comm },
	{ "es_PA", BASE_comm },
	{ "es_PE", BASE_comm },
	{ "es_PR", BASE_comm },
	{ "es_PY", BASE_comm },
	{ "es_SV", BASE_comm },
	{ "es_US", BASE_comm },
	{ "es_UY", BASE_comm },
	{ "es_VE", BASE_comm },
	{ "et_EE", BASE_comm },
	{ "eu_ES", BASE_iso14651_t1 },
	{ "fa_IR", BASE_iso14651_t1 },
	{ "fi_FI", BASE_comm },
	{ "fo_FO", BASE_comm },
	{ "fr_BE", BASE_iso14651_t1 },
	{ "fr_CA", BASE_comm },
	{ "fr_CH", BASE_iso14651_t1 },
	{ "fr_FR", BASE_iso14651_t1 },
	{ "fr_LU", BASE_iso14651_t1 },
	{ "ga_IE", BASE_iso14651_t1 },
	{ "gd_GB", BASE_iso14651_t1 },
	{ "gl_ES", BASE_comm },
	{ "gv_GB", BASE_iso14651_t1 },
	{ "he_IL", BASE_iso14651_t1 },
	{ "hi_IN", BASE_iso14651_t1 },
	{ "hr_HR", BASE_comm },
	{ "hu_HU", BASE_iso14651_t1 },
	{ "hy_AM", BASE_iso14651_t1 },
	{ "id_ID", BASE_iso14651_t1 },
	{ "is_IS", BASE_comm },
	{ "it_CH", BASE_iso14651_t1 },
	{ "it_IT", BASE_iso14651_t1 },
	{ "iw_IL", BASE_iso14651_t1 },
	{ "ja_JP", BASE_ja_JP },
	{ "ka_GE", BASE_iso14651_t1 },
	{ "kl_GL", BASE_comm },
	{ "ko_KR", BASE_ko_KR },
	{ "kw_GB", BASE_iso14651_t1 },
	{ "lt_LT", BASE_comm },
	{ "lv_LV", BASE_comm },
	{ "mi_NZ", BASE_iso14651_t1 },
	{ "mk_MK", BASE_iso14651_t1 },
	{ "mr_IN", BASE_iso14651_t1 },
	{ "ms_MY", BASE_iso14651_t1 },
	{ "mt_MT", BASE_iso14651_t1 },
	{ "nl_BE", BASE_iso14651_t1 },
	{ "nl_NL", BASE_iso14651_t1 },
	{ "nn_NO", BASE_iso14651_t1 },
	{ "no_NO", BASE_comm },
	{ "oc_FR", BASE_iso14651_t1 },
	{ "pl_PL", BASE_comm },
	{ "pt_BR", BASE_iso14651_t1 },
	{ "pt_PT", BASE_iso14651_t1 },
	{ "ro_RO", BASE_iso14651_t1 },
	{ "ru_RU", BASE_iso14651_t1 },
	{ "ru_UA", BASE_iso14651_t1 },
	{ "se_NO", BASE_iso14651_t1 },
	{ "sk_SK", BASE_cs_CZ },
	{ "sl_SI", BASE_comm },
	{ "sq_AL", BASE_iso14651_t1 },
	{ "sr_YU", BASE_iso14651_t1 },
	{ "sv_FI", BASE_comm },
	{ "sv_SE", BASE_iso14651_t1 },
	{ "ta_IN", BASE_iso14651_t1 },
	{ "te_IN", BASE_iso14651_t1 },
	{ "tg_TJ", BASE_iso14651_t1 },
	{ "th_TH", BASE_th_TH },
	{ "ti_ER", BASE_iso14651_t1 },
	{ "ti_ET", BASE_iso14651_t1 },
	{ "tl_PH", BASE_iso14651_t1 },
	{ "tr_TR", BASE_comm },
	{ "tt_RU", BASE_iso14651_t1 },
	{ "uk_UA", BASE_iso14651_t1 },
	{ "ur_PK", BASE_iso14651_t1 },
	{ "uz_UZ", BASE_iso14651_t1 },
	{ "vi_VN", BASE_iso14651_t1 },
	{ "wa_BE", BASE_iso14651_t1 },
	{ "yi_US", BASE_iso14651_t1 },
	{ "zh_CN", BASE_iso14651_t1 },
	{ "zh_HK", BASE_iso14651_t1 },
	{ "zh_SG", BASE_iso14651_t1 },
	{ "zh_TW", BASE_iso14651_t1 },
};


static int der_count[BASE_MAX];
static const char *new_args[500];
static int new_arg_count;

static int dep_cmp(const void *s1, const void *s2)
{
	return strcmp( (const char *) s1, ((const deps_t *) s2)->der_name);
}

static int old_main(int argc, char **argv);

int main(int argc, char **argv)
{
	const deps_t *p;
	ll_item_t *lli;
	int i;
	int total;

	if (argc < 2) {
		return EXIT_FAILURE;
	}

	init_locale_list();	

	while (--argc) {
		p = (const deps_t *) bsearch(*++argv, deps, sizeof(deps)/sizeof(deps[0]), sizeof(deps[0]), dep_cmp);
		if (!p) {
			if (!strcmp("C", *argv)) {
				printf("ignoring C locale\n");
				continue;
			} else {
				printf("%s not found\n", *argv);
				return EXIT_FAILURE;
			}
		}
	
		i = p->base_locale;
		++der_count[i];

		if (!strcmp(base_name[i], *argv)) {
			/* same name as base, so skip after count incremented */
			continue;
		}
		
		/* add it to the list.  the main body will catch duplicates */
		lli = (ll_item_t *) xmalloc(sizeof(ll_item_t));
		lli->prev = lli->next = NULL;
		lli->data = (void *) *argv;
		insque(lli, locale_list[i]);
	}

	total = 0;
	for (i=0 ; i < BASE_MAX ; i++) {
/* 		printf("der_count[%2d] = %3d\n", i, der_count[i]); */
		total += der_count[i];
	}
/* 	printf("total = %d\n", total); */

	new_args[new_arg_count++] = "dummyprogramname";
	for (i=0 ; i < BASE_MAX ; i++) {
		if (!der_count[i]) {
			continue;
		}
		new_args[new_arg_count++] = (i == BASE_comm) ? "-c" : "-b";
		lli = locale_list[i];
		do {
			new_args[new_arg_count++] = (const char *) (lli->data);
			lli = lli->next;
		} while (lli != locale_list[i]);
		new_args[new_arg_count++] = "-f";
	}

/* 	for (i=0 ; i < new_arg_count ; i++) { */
/* 		printf("%3d: %s\n", i, new_args[i]); */
/* 	} */

	return old_main(new_arg_count, (char **) new_args);
}


/* usage...  prog -b basefile derived {derived} -s single {single} */

static int old_main(int argc, char **argv)
{
	int next_is_base = 0;
	int next_is_subset = 0;

	superset = 0;

	while (--argc) {
		++argv;
		if (**argv == '-') {
			if ((*argv)[1] == 'd') {
				dump_weights((*argv) + 2);
			} else if ((*argv)[1] == 'f') {	/* dump all weight rules */
				finalize_base();
			} else if ((*argv)[1] == 'R') {	/* dump all weight rules */
				twalk(root_weight, print_weight_node);
			} else if (((*argv)[1] == 'c') && !(*argv)[2]) { /* new common subset */
				cur_base = cur_derived = NULL;
				next_is_subset = 1;
				next_is_base = 1;
				superset = 0;
			} else if (((*argv)[1] == 'b') && !(*argv)[2]) { /* new base locale */
				cur_base = cur_derived = NULL;
				next_is_subset = 0;
				next_is_base = 1;
				superset = 0;
			} else if (((*argv)[1] == 's') && !(*argv)[2]) { /* single locales follow */
				cur_base = cur_derived = NULL;
				next_is_subset = 0;
				next_is_base = 2;
				superset = 0;
			} else {
				error_msg("unrecognized option %s", *argv);
			}
			continue;
		}
		/* new file */
		new_col_locale(*argv);	/* automaticly sets cur_col */
		if (next_is_base) {
			cur_base = cur_col;
		} else {
			cur_derived = cur_col;
		}
		pushfile(*argv);
/* 		fprintf(stderr, "processing file %s\n", *argv); */
		processfile();			/* this does a popfile */

/* 		twalk(cur_col->root_colitem, print_colnode); */
		
		if (next_is_base == 1) {
			next_is_base = 0;
		}
		if (next_is_subset) {
			next_is_subset = 0;
			superset = 1;
		}
	}

	fprintf(stderr, "success!\n");
	fprintf(stderr,
/* 			"num_sym=%zu mem_sym=%zu  unique_weights=%zu\n", */
			"num_sym=%u mem_sym=%u  unique_weights=%u\n",
			num_sym, mem_sym, unique_weights);
/* 	twalk(root_weight, print_weight_node); */

	fprintf(stderr, "num base locales = %d    num derived locales = %d\n",
			base_locale_len, der_locale_len);

	fprintf(stderr,
			"override_len = %d      multistart_len = %d    weightstr_len = %d\n"
			"wcs2colidt_len = %d    index2weight_len = %d  index2ruleidx_len = %d\n"
			"ruletable_len = %d\n"
			"total size is %d bytes or %d kB\n",
			override_len, multistart_len, weightstr_len,
			wcs2colidt_len, index2weight_len, index2ruleidx_len,
			ruletable_len,
#warning mult by 2 for rule indecies
			(override_len + multistart_len + weightstr_len
			 + wcs2colidt_len + index2weight_len + index2ruleidx_len + ruletable_len) * 2,
			(override_len + multistart_len + weightstr_len
			 + wcs2colidt_len + index2weight_len + index2ruleidx_len + ruletable_len + 511) / 512);

#if 0
	{
		int i;

		for (i=0 ; i < base_locale_len ; i++) {
			dump_base_locale(i);
		}
		for (i=0 ; i < der_locale_len ; i++) {
			dump_der_locale(i);
		}
	}
#endif

	{
		FILE *fp = fopen("locale_collate.h", "w");

		if (!fp) {
			error_msg("couldn't open output file!");
		}
		dump_collate(fp);
		if (ferror(fp) || fclose(fp)) {
			error_msg("write error or close error for output file!\n");
		}
	}

    return EXIT_SUCCESS;
}

static void error_msg(const char *fmt, ...) 
{
	va_list arg;

	fprintf(stderr, "Error: ");
	if (fno >= 0) {
	    fprintf(stderr, "file %s (%d): ", fname[fno], lineno[fno]);
	}
	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	va_end(arg);
	fprintf(stderr, "\n");

	exit(EXIT_FAILURE);
}

static void pushfile(char *filename)
{
	static char fbuf[PATH_MAX];

	snprintf(fbuf, PATH_MAX, "collation/%s", filename);

	if (fno >= MAX_FNO) {
		error_msg("file stack size exceeded");
	}
					  
	if (!(fstack[++fno] = fopen(fbuf, "r"))) {
		--fno;					/* oops */
		error_msg("cannot open file %s", fbuf);
	}

	fname[fno] = xsymdup(filename);
	lineno[fno] = 0;
}

static void popfile(void)
{
	if (fno < 0) {
		error_msg("pop on empty file stack");
	}

/* 	free(fname[fno]); */
	fclose(fstack[fno]);
	--fno;
}

static void eatwhitespace(void)
{
	while (isspace(*pos)) {
		++pos;
	}
}

static int iscommentchar(int c)
{
	return ((c == '#') || (c == '%'));
}

static int next_line(void)
{
	size_t n;
	char *s = linebuf;

	assert(fno >= 0);

	pos_e = NULL;
	do {
		if (fgets(s, sizeof(linebuf), fstack[fno]) != NULL) {
			++lineno[fno];
			n = strlen(linebuf);
			if ((n == sizeof(linebuf) - 1) && (linebuf[n-1] != '\n')) {
				/* Either line is too long or last line is very long with
				 * no trailing newline.  But we'll always treat it as an
				 * errro. */
				error_msg("line too long?");
			}

			--n;
			/* Be careful... last line doesn't need a newline. */
			if (linebuf[n] == '\n') {
				linebuf[n--] = 0;	/* trim trailing newline */
			}

			pos = linebuf;
			eatwhitespace();
			if (*pos && !iscommentchar(*pos)) { /* not empty or comment line */
				return 1;		/* got a line */
			}
		} else {				/* eof */
			popfile();
		}
	} while (fno >= 0);

	return 0;
}

static char *next_token(void)
{
	char *p;

#if 0
	if (pos_e == NULL) {
		return NULL
		pos = pos_e;
		*pos = end_of_token;
		end_of_token = 0;
	}
#else
	if (pos_e != NULL) {
		pos = pos_e;
		*pos = end_of_token;
		end_of_token = 0;
	}
#endif
	eatwhitespace();
	p = pos;

	if (!*p || iscommentchar(*p)) {	/* end of line or start of comment */
		pos = pos_e = NULL;
		*p = 0;					/* treat comment as end of line */
/* 		fprintf(stdout, "returning NUL token |%s|\n", pos); */
		return NULL;
#if 1
	} else if (*p == '<') {	 /* collating symbol, element, or value */
		while (*++p) {
			if ((*p == '/') && p[1]) {
				++p;
				continue;
			}
			if (*p == '>') {
				pos_e = ++p;
				end_of_token = *p;
				*p = 0;
/* 				fprintf(stdout, "returning col token |%s|\n", pos); */
				return pos;
			}
		}
	} else if (*p == '"') {		/* collating element value? */
		while (*++p) {
			if (*p == '"') {	/* found the end of the quoted string */
				pos_e = ++p;
				end_of_token = *p;
				*p = 0;
/* 				fprintf(stdout, "returning quote token |%s|\n", pos); */
				return pos;
			}
		}
#endif
	} else {					/* some kind of keyword */
		while (*++p) {
			if (isspace(*p) || (*p == ';')) {
				break;
			}
		}
		pos_e = p;
		end_of_token = *p;
		*p = 0;
/* 		fprintf(stdout, "returning key token |%s|\n", pos); */
		return pos;
	}

	error_msg("illegal token |%s|", pos);
}

static void *xmalloc(size_t n)
{
	void *p;

	if (!(p = malloc(n))) {
		error_msg("OUT OF MEMORY");
	}
	return p;
}

static void do_copy(void)
{
	char *s;
	char *e;

	if ((s = next_token()) != NULL) {
		e = strchr(s + 1, '"');
		if ((*s == '"') && e && (*e == '"') && !e[1]) {
			if (next_token() != NULL) {
				error_msg("illegal trailing text: %s", pos);
			}
			*e = 0;
			++s;
			if (cur_base && !strcmp(cur_base->name,s)) {
/* 				fprintf(stderr, "skipping copy of base file %s\n", s); */
#warning need to update last in order and position or check
				return;
			}
/* 			fprintf(stderr, "full copy of %s\n", s); */
			pushfile(s);
			return;
		}
	}
	error_msg("illegal or missing arg for copy: %s", s);
}

static void do_colsym(void)
{
	char *s;
	char *e;

	if ((s = next_token()) != NULL) {
		e = strrchr(s,'>');
		if ((*s == '<') && e && (*e == '>') && !e[1]) {
			if (next_token() != NULL) {
				error_msg("illegal trailing text: %s", pos);
			}
			e[1] = 0; /* cleanup in case next_token stored something */
			add_colitem(s,NULL);
			return;
		}
	}
	error_msg("illegal or missing arg for collating-symbol: %s", s);
}

static void do_colele(void)
{
	char *s;
	char *e;
	char *s1;
	char *e1;
	int n;

	if ((s = next_token()) != NULL) {
		e = strrchr(s,'>');
		if ((*s == '<') && e && (*e == '>') && !e[1]) {
			if (((s1 = next_token()) == NULL)
				|| (strcmp(s1,"from") != 0)
				|| ((s1 = next_token()) == NULL)
				|| (*s1 != '\"')
				) {
				error_msg("illegal format for collating-element spec");
			}
			e1 = strchr(s1 + 1, '"');
			if ((*s1 != '"') || !e1 || (*e1 != '"') || (e1[1] != 0)) {
				error_msg("illegal definition for collating-element: %s", s1);
			}
			if (next_token() != NULL) {
				error_msg("illegal trailing text: %s", pos);
			}
			e[1] = 0; /* cleanup in case next_token stored something */
			e1[1] = 0;
			add_colitem(s,s1);
			++s1;
			if (!(n = is_ucode(s1))) {
				error_msg("starting char must be a <U####> code: %s", s1);
			}
			assert(s1[n] == '<');
			s1[n] = 0;
			s = xsymdup(s1);
			if (!(tsearch(s, &cur_base->root_starter_char, sym_cmp))) {
				error_msg("OUT OF MEMORY");
			}

			return;
		}
	}
	error_msg("illegal or missing arg for collating-element: %s", s);
}

static ll_item_t *find_section_list_item(const char *name, col_locale_t *loc)
{
	ll_item_t *p;

	if (!loc) {
		return NULL;
	}

	p = loc->section_list;

	while (p) {
#warning devel code
/* 		if (!((p->data_type == DT_SECTION) || (p->data_type == DT_REORDER))) { */
/* 			fprintf(stderr, "fsli = %d\n", p->data_type); */
/* 		} */
		assert((p->data_type == DT_SECTION) || (p->data_type == DT_REORDER));
		if (!strcmp(name, ((section_t *)(p->data))->name)) {
			break;
		}
		p = p->next;
	}
	return p;
}

static ll_item_t *find_ll_last(ll_item_t *p)
{
	assert(p);

	while (p->next) {
		p = p->next;
	}
	return p;
}

static void do_script(void)
{
	char *s;
	char *e;

	if ((s = next_token()) != NULL) {
		e = strrchr(s,'>');
		if ((*s == '<') && e && (*e == '>') && !e[1]) {
			if (next_token() != NULL) {
				error_msg("illegal trailing text: %s", pos);
			}
			e[1] = 0; /* cleanup in case next_token stored something */
			add_script(s);
			return;
		}
	}
	error_msg("illegal or missing arg for script: %s", s);
}

static col_locale_t *new_col_locale(char *name)
{
	ll_item_t *lli;
	ll_item_t *lli2;

	cur_col = (col_locale_t *) xmalloc(sizeof(col_locale_t));
	cur_col->name = name;
	cur_col->root_colitem = NULL;
	cur_col->root_element = NULL;
	cur_col->root_scripts = NULL;
	cur_col->base_locale = NULL;
	if (!superset) {
		/* start with an anonymous section */
		cur_section = new_section(NULL);
		cur_col->section_list = new_ll_item(DT_SECTION, cur_section);
	} else {
		/* start with a reorder section */
		cur_section = new_section("R");
		cur_num_weights = cur_section->num_rules
			= ((section_t *)(cur_base->section_list->data))->num_rules;
		memcpy(cur_rule,
			   ((section_t *)(cur_base->section_list->data))->rules,
			   MAX_COLLATION_WEIGHTS);
		memcpy(cur_section->rules,
			   ((section_t *)(cur_base->section_list->data))->rules,
			   MAX_COLLATION_WEIGHTS);
		cur_col->section_list = new_ll_item(DT_REORDER, cur_section);
		assert(cur_base->section_list->next == NULL); /* currently only one section allowed */
		lli = ((section_t *)(cur_base->section_list->data))->itm_list;
		assert(lli);
		lli2 = new_ll_item(DT_REORDER, cur_section);
		lli2->prev = lli2->next = lli2;
		insque(lli2, lli->prev);
		((section_t *)(cur_base->section_list->data))->itm_list = lli2;
	}
/* 	cur_col->section_list = NULL; */
/* 	add_script(((section_t *)(cur_col->section_list->data))->name); */
	cur_col->root_wi_index = NULL;
	cur_col->root_wi_index_reordered = NULL;
	cur_col->root_derived_wi = NULL;
	cur_col->derived_list = NULL;
	cur_col->root_starter_char = NULL;
	cur_col->root_starter_all = NULL;
	cur_col->undefined_idx = NULL;
	return cur_col;
}

static int colitem_cmp(const void *n1, const void *n2)
{
    return strcmp(((colitem_t *)n1)->string, ((colitem_t *)n2)->string);
}

static int colelement_cmp(const void *n1, const void *n2)
{
    int r;

    r = strcmp(((colitem_t *)n1)->string, ((colitem_t *)n2)->string);
    if (!r) {
		if (((colitem_t *)n1)->element && ((colitem_t *)n2)->element) {
			r = strcmp(((colitem_t *)n1)->element, ((colitem_t *)n2)->element);
		} else if (((colitem_t *)n1)->element == ((colitem_t *)n2)->element) {
			r = 0;				/* both null */
		} else {
			r = (((colitem_t *)n1)->element == NULL) ? -1 : 1;
		} 
    }
    return r;
}

static void del_colitem(colitem_t *p)
{
/*     free((void *) p->element); */
/*     free((void *) p->string); */
    free(p);
}

static colitem_t *new_colitem(char *item, char *def)
{
	colitem_t *p;

	p = xmalloc(sizeof(colitem_t));
	p->string = xsymdup(item);
	p->element = (!def) ? def : xsymdup(def);

	return p;
}

static void add_colitem(char *item, char *def)
{
	colitem_t *p;

#if 0
	printf("adding collation item %s", item);
	if (def) {
		printf(" with definition %s", def);
	}
	printf("\n");
#endif

	p = new_colitem(item, def);

#warning devel code
	if (superset) {
		if (tfind(p, &cur_base->root_colitem, colitem_cmp)) {
/* 			fprintf(stderr, "skipping superset duplicate collating item \"%s\"\n", p->string); */
			del_colitem(p);
			return;
/* 		} else { */
/* 			fprintf(stderr, "superset: new collating item \"%s\" = %s\n", p->string, p->element); */
		}
	}

	if (cur_col == cur_derived) {
		if (!tfind(p, &cur_base->root_colitem, colitem_cmp)) {
			/* not in current but could be in base */
			if (!tsearch(p, &cur_base->root_colitem, colitem_cmp)) {
				error_msg("OUT OF MEMORY!");
			}
		} else if (!tfind(p,  &cur_base->root_colitem, colelement_cmp)) {
			error_msg("collating element/symbol mismatch: item=%s def=%s", item, def);
		}
	}


	if (!tfind(p, &cur_col->root_colitem, colitem_cmp)) {
		/* not in current but could be in base */
		if (!tsearch(p, &cur_col->root_colitem, colitem_cmp)) {
			error_msg("OUT OF MEMORY!");
		}
	} else if (!tfind(p,  &cur_col->root_colitem, colelement_cmp)) {
		error_msg("collating element/symbol mismatch");
	} else {					/* already there */
		fprintf(stderr, "duplicate collating item \"%s\"\n", p->string);
		del_colitem(p);
	}
}

/* add a script (section) to the current locale */
static void add_script(const char *s)
{
	ll_item_t *l;

	/* make sure it isn't in base if working with derived */
	if (cur_base != cur_col) {
		if (find_section_list_item(s, cur_base)) {
			error_msg("attempt to add script %s for derived when already in base", s);
		}
	}

	if (find_section_list_item(s, cur_col)) {
		error_msg("attempt to readd script %s", s);
	}
	
	l = find_ll_last(cur_col->section_list);
	insque(new_ll_item(DT_SECTION, new_section(s)), l);
}

static const char str_forward[] =  "forward";
static const char str_backward[] = "backward";
static const char str_position[] = "position";

static void do_order_start(void)
{
	const char *s;
	char *e;
	ll_item_t *l;
	section_t *sect;
	int rule;

	if (order_state & ~IN_ORDER) {
		error_msg("order_start following reorder{_sections}_after");
	}
	order_state |= IN_ORDER;

	if (superset) {
		if (++superset_order_start_cnt > 1) {
			error_msg("currently only a common order_start is supported in superset");
		}
		return;
	}

	if (!(s = next_token())) {
		s = str_forward;		/* if no args */
	}

	if (*s == '<') {		/* section (script) */
		e = strrchr(s,'>');
		if ((*s == '<') && e && (*e == '>') && !e[1]) {
			e[1] = 0; /* cleanup in case next_token stored something */

			if (!(l = find_section_list_item(s, cur_col))) {
				error_msg("ref of undefined sections: %s", s);
			}
			sect = (section_t *)(l->data);
			if (sect->num_rules) {
				error_msg("sections already defined: %s", s);
			}
		} else {
			error_msg("illegal section ref: %s", s);
		}

		if (!(s = next_token())) {
			s = str_forward;		/* if no args */
		} else if (*s != ';') {
			error_msg("missing seperator!");
		}
	} else {					/* need an anonymous section */
		if ((*cur_section->name != '<') && (cur_section->num_items == 0)) { /* already in an empty anonymous section */
			sect = cur_section;
/* 			fprintf(stdout, "using empty anon section %s\n", sect->name); */
		} else {
			sect = new_section(NULL);
			l = find_ll_last(cur_col->section_list);
			insque(new_ll_item(DT_SECTION, sect), l);
/* 			fprintf(stdout, "adding order section after section %s\n", ((section_t *)(l->data))->name); */
/* 			fprintf(stdout, "    last section is %s\n", ((section_t *)(l->next->data))->name); */
		}
		sect->num_rules = 0;	/* setting this below so nix default */
	}
	cur_section = sect;
/* 	fprintf(stdout, "cur_section now %s\n", cur_section->name); */

#warning need to add section to weight list?

	/* now do rules */
	do {
		rule = 0;
		if (*s == ';') {
			++s;
		}
		while (*s) {
			if (!strncmp(str_forward, s, 7)) {
				rule |= R_FORWARD;
				s += 7;
			} else if (!strncmp(str_backward, s, 8)) {
				rule |= R_BACKWARD;
				s += 8;
			} else if (!strncmp(str_position, s, 8)) {
				rule |= R_POSITION;
				s += 8;
			}

			if (*s == ',') {
				++s;
				continue;
			}

			if (!*s || (*s == ';')) {
				if (sect->num_rules >= MAX_COLLATION_WEIGHTS) {
					error_msg("more than %d weight rules!", MAX_COLLATION_WEIGHTS);
				}
				if (!rule) {
					error_msg("missing weight rule!");
				}
				if ((rule & (R_FORWARD|R_BACKWARD|R_POSITION)) > R_BACKWARD) {
					error_msg("backward paired with  forward and/or position!");
				}

				sect->rules[sect->num_rules++] = rule;
				rule = 0;
				continue;
			}

			error_msg("illegal weight rule: %s", s);
		}
	} while ((s = next_token()) != NULL);

	cur_section = sect;

/* 	fprintf(stderr, "setting cur_num_weights to %d for %s\n", sect->num_rules, sect->name); */
	cur_num_weights = sect->num_rules;
	memcpy(cur_rule, sect->rules, MAX_COLLATION_WEIGHTS);
}

static void do_order_end(void)
{
	if (!(order_state & IN_ORDER)) {
		error_msg("order_end with no matching order_start");
	}
	order_state &= ~IN_ORDER;

	cur_section = new_section(NULL);
}

static void do_reorder_after(void)
{
	char *t;
	ll_item_t *lli;
	const weight_t *w;
	int save_cur_num_weights;
	char save_cur_rule[MAX_COLLATION_WEIGHTS];


	if (order_state & ~IN_REORDER) {
		error_msg("reorder_after following order_start or reorder_sections_after");
	}
	order_state |= IN_REORDER;

	if (superset) {
		error_msg("currently reorder_after is not supported in supersets");
	}

#warning have to use rule for current section!!!

	if (!(t = next_token())) {
		error_msg("missing arg for reorder_after");
	}

	t = xsymdup(t);

	if (next_token() != NULL) {
		error_msg("trailing text reorder_after: %s", pos);
	}

	if (cur_col == cur_base) {
		error_msg("sorry.. reorder_after in base locale is not currently supported");
	}

	if (!(lli = find_wi_index(t, cur_base))) {
		error_msg("reorder_after for non-base item currently not supported: %s", t);
	}

	w = ((weighted_item_t *)(lli->data))->weight;


	save_cur_num_weights = cur_num_weights;
	memcpy(save_cur_rule, cur_rule, MAX_COLLATION_WEIGHTS);

	cur_section = new_section("R");
	insque(new_ll_item(DT_REORDER, cur_section), lli);

#if 0

	{
		ll_item_t *l1;
		ll_item_t *l2;
		ll_item_t *l3;
		l1 = new_ll_item(DT_REORDER, cur_section);
		l2 = find_ll_last(cur_col->section_list);
		insque(l1, l2);
		l3 = find_ll_last(cur_col->section_list);

		fprintf(stderr, "reorder_after %p %p %p %s\n", l1, l2, l3, cur_section->name);
	}
#else
	insque(new_ll_item(DT_REORDER, cur_section), find_ll_last(cur_col->section_list));
#endif

	cur_num_weights = cur_section->num_rules = save_cur_num_weights;
	memcpy(cur_rule, save_cur_rule, MAX_COLLATION_WEIGHTS);
	memcpy(cur_section->rules, save_cur_rule, MAX_COLLATION_WEIGHTS);


#warning devel code
/* 	fprintf(stderr, "reorder -- %s %d\n", ((weighted_item_t *)(lli->data))->symbol, w->num_weights); */

#warning hack to get around hu_HU reorder-after problem
/* 	if (!w->num_weights) { */

/* 	} else { */
/* 		cur_num_weights = w->num_weights; */
/* 		memcpy(cur_rule, w->rule, MAX_COLLATION_WEIGHTS); */
/* 	}	 */

/* 	fprintf(stderr, "reorder_after succeeded for %s\n", t); */
}

static void do_reorder_end(void)
{
	if (!(order_state & IN_REORDER)) {
		error_msg("reorder_end with no matching reorder_after");
	}
	order_state &= ~IN_REORDER;
}

static void do_reorder_sections_after(void)
{
	const char *t;
	ll_item_t *lli;

	if (order_state & ~IN_REORDER_SECTIONS) {
		error_msg("reorder_sections_after following order_start or reorder_after");
	}
	order_state |= IN_REORDER_SECTIONS;

	if (superset) {
		error_msg("currently reorder_sections_after is not supported in supersets");
	}

	if (!(t = next_token())) {
		error_msg("missing arg for reorder_sections_after");
	}

	t = xsymdup(t);

	if (next_token() != NULL) {
		error_msg("trailing text reorder_sections_after: %s", pos);
	}

	if (cur_col == cur_base) {
		error_msg("sorry.. reorder_sections_after in base locale is not currently supported");
	}

	lli = cur_base->section_list;
	do {
/* 		fprintf(stderr, "hmm -- |%s|%d|\n", ((section_t *)(lli->data))->name, lli->data_type); */
		if (lli->data_type & DT_SECTION) {
/* 			fprintf(stderr, "checking |%s|%s|\n", ((section_t *)(lli->data))->name, t); */
			if (!strcmp(((section_t *)(lli->data))->name, t)) {
				reorder_section_ptr = lli;
				return;
			}
		}
		lli = lli->next;
	} while (lli);

	error_msg("reorder_sections_after for non-base item currently not supported: %s", t);
}

static void do_reorder_sections_end(void)
{
	if (!(order_state & IN_REORDER_SECTIONS)) {
		error_msg("reorder_sections_end with no matching reorder_sections_after");
	}
	order_state &= ~IN_REORDER_SECTIONS;

	reorder_section_ptr = NULL;
}

static ll_item_t *new_ll_item(int data_type, void *data)
{
	ll_item_t *p;

	p = xmalloc(sizeof(ll_item_t));
	p->next = p->prev = NULL;
	p->data_type = data_type;
	p->data = data;
	p->idx = INT_MIN;

	return p;
}

static int sym_cmp(const void *n1, const void *n2)
{
/* 	fprintf(stderr, "sym_cmp: |%s| |%s|\n", (const char *)n1, (const char *)n2); */
    return strcmp((const char *) n1, (const char *) n2);
}

static char *xsymdup(const char *s)
{
	void *p;

	if (!(p = tfind(s, &root_sym, sym_cmp))) { /* not a currently known symbol */
		if (!(s = strdup(s)) || !(p = tsearch(s, &root_sym, sym_cmp))) {
			error_msg("OUT OF MEMORY!");
		}
		++num_sym;
		mem_sym += strlen(s) + 1;
/* 		fprintf(stderr, "xsymdup: alloc |%s| %p |%s| %p\n", *(char **)p, p, s, s); */
/* 	} else { */
/* 		fprintf(stderr, "xsymdup: found |%s| %p\n", *(char **)p, p); */
	}
	return *(char **) p;
}

static int weight_cmp(const void *n1, const void *n2)
{
	const weight_t *w1 = (const weight_t *) n1;
	const weight_t *w2 = (const weight_t *) n2;
	int i, r;

	if (w1->num_weights != w2->num_weights) {
		return w1->num_weights - w2->num_weights;
	}

	for (i=0 ; i < w1->num_weights ; i++) {
		if (w1->rule[i] != w2->rule[i]) {
			return w1->rule[i] - w2->rule[i];
		}
		if ((r = strcmp(w1->colitem[i], w2->colitem[i])) != 0) {
			return r;
		}
	}
	return 0;
}

static weight_t *register_weight(weight_t *w)
{
	void *p;

	if (!(p = tfind(w, &root_weight, weight_cmp))) { /* new weight */
		p = xmalloc(sizeof(weight_t));
		memcpy(p, w, sizeof(weight_t));
		if (!(p = tsearch(p, &root_weight, weight_cmp))) {
			error_msg("OUT OF MEMORY!");
		}
		++unique_weights;
/* 	} else { */
/* 		fprintf(stderr, "rw: found\n"); */
	}
	return *(weight_t **)p;
}

static size_t ll_len(ll_item_t *l)
{
	size_t n = 0;
	ll_item_t *p = l;

	while (p) {
		++n;
		p = p->next;
		if (p == l) {			/* work for circular too */
			break;
		}
	}
	return n;
}

static size_t ll_count(ll_item_t *l, int mask)
{
	size_t n = 0;
	ll_item_t *p = l;

	while (p) {
		if (p->data_type & mask) {
			++n;
		}
		p = p->next;
		if (p == l) {			/* work for circular too */
			break;
		}
	}
	return n;
}


static int wi_index_cmp(const void *n1, const void *n2)
{
	const char *s1 = ((weighted_item_t *)(((ll_item_t *) n1)->data))->symbol;
	const char *s2 = ((weighted_item_t *)(((ll_item_t *) n2)->data))->symbol;

    return strcmp(s1, s2);
}

static void add_wi_index(ll_item_t *l)
{
	assert(l->data_type == DT_WEIGHTED);

	if (!strcmp(((weighted_item_t *)(l->data))->symbol, "UNDEFINED")) {
		cur_col->undefined_idx = l;
	}

	if (!tfind(l, &cur_col->root_wi_index, wi_index_cmp)) { /* new wi_index */
		if (!tsearch(l, &cur_col->root_wi_index, wi_index_cmp)) {
			error_msg("OUT OF MEMORY!");
		}
	}

	if (cur_base != cur_col) {
		if (!tfind(l, &cur_base->root_wi_index, wi_index_cmp)) {/* not a base val */
/* 			printf("derived: %s\n", ((weighted_item_t *)(l->data))->symbol); */
			if (!tfind(l, &cur_base->root_derived_wi, wi_index_cmp)) { /* new derived */
				if (!tsearch(l, &cur_base->root_derived_wi, wi_index_cmp)) {
					error_msg("OUT OF MEMORY!");
				}
			}
		}
	}
}

static int final_index;


static int is_ucode(const char *s)
{
	if ((s[0] == '<')
		&& (s[1] == 'U')
		&& isxdigit(s[2])
		&& isxdigit(s[3])
		&& isxdigit(s[4])
		&& isxdigit(s[5])
		&& (s[6] == '>')
		) {
		return 7;
	} else {
		return 0;
	}
}

static void add_final_col_index(const char *s)
{
	ENTRY e;

	e.key = (char *) s;
	e.data = (void *)(final_index);
	if (!hsearch(e, FIND)) {	/* not in the table */
		if (!hsearch(e, ENTER)) {
			error_msg("OUT OF MEMORY! (hsearch)");
		}
#if 0
		{
			int n;
			void *v;
			colitem_t ci;
			colitem_t *p;
			const char *t;

			if (!strcmp(s, "UNDEFINED")) {
				printf("%6d: %s\n", final_index, s);
			} else {
				assert(*s == '<');
				if ((n = is_ucode(s)) != 0) {
					assert(!s[n]);
					printf("%6d: %s\n", final_index, s);
				} else {
					ci.string = (char *) s;
					ci.element = NULL; /* don't care */
					v = tfind(&ci, &cur_base->root_colitem, colitem_cmp);
					if (!v) {
						fprintf(stderr, "%s  NOT DEFINED!!!\n", s);
					} else {
						p = *((colitem_t **) v);
						if (p->element != NULL) {
							t = p->element;
							assert(*t == '"');
							++t;
							n = is_ucode(t);
							assert(n);
							printf("%6d: %.*s | ", final_index, n, t);
							do {
								t += n;
								assert(*t);
								if (*t == '"') {
									assert(!t[1]);
									break;
								}
								n = is_ucode(t);
								assert(n);
								printf("%.*s", n, t);
							} while (1);
							printf("   collating-element %s\n", s);
						} else {
							printf("%6d: %s  (collating-symbol)\n", final_index, s);
						}
					}
				}
			}
		}
#endif
		++final_index;
	}

}

static int final_index_val0(const char *s)
{
	ENTRY *p;
	ENTRY e;
	e.key = (char *) s;

	if (!(p = hsearch(e, FIND))) {	/* not in the table */
		return 0;
	}

	return (int)(p->data);
}

static int final_index_val(const char *s)
{
	ENTRY *p;
	ENTRY e;
	e.key = (char *) s;

	if (!(p = hsearch(e, FIND))) {	/* not in the table */
		error_msg("can't find final index: %s", s);
	}

	return (int)(p->data);
}

static size_t num_tree_nodes;

static void count_nodes(const void *ptr, VISIT order, int level)
{
    if ((order == postorder) || (order == leaf))  {
		++num_tree_nodes;
    }
}

static size_t tnumnodes(const void *root)
{
	num_tree_nodes = 0;

	twalk(root, count_nodes);

	return num_tree_nodes;

}

static ll_item_t *find_wi_index(const char *sym, col_locale_t *cl)
{
	weighted_item_t w;
	ll_item_t l;
	void *p;

	w.symbol = sym;
	l.data = &w;
	l.data_type = DT_WEIGHTED;

	p = tfind(&l, &cl->root_wi_index, wi_index_cmp);

	if (p) {
		p = *(ll_item_t **)p;
	}

	return (ll_item_t *) p;
}

static void mark_reordered(const char *sym)
{
	ll_item_t *lli;

	lli = find_wi_index(sym, cur_base);

	if (lli) {
		if (!tsearch(lli, &cur_base->root_wi_index_reordered, wi_index_cmp)) {
			error_msg("OUT OF MEMORY!");
		}
	}
}

static ll_item_t *find_wi_index_reordered(const char *sym)
{
	weighted_item_t w;
	ll_item_t l;
	void *p;

	w.symbol = sym;
	l.data = &w;
	l.data_type = DT_WEIGHTED;

	p = tfind(&l, &cur_base->root_wi_index_reordered, wi_index_cmp);

	if (p) {
		p = *(ll_item_t **)p;
	}

	return (ll_item_t *) p;
}

static ll_item_t *init_comm_ptr(void)
{
	assert(cur_base);
	assert(cur_base->section_list);
	/* at the moment, only support one section in comm */
	assert(cur_base->section_list->next == NULL);

	comm_cur_ptr = ((section_t *)(cur_base->section_list->data))->itm_list;

	while (comm_cur_ptr && (comm_cur_ptr->data_type & DT_REORDER)) {
		comm_cur_ptr = comm_cur_ptr->next;
	}

#warning devel code
/* 	{ */
/* 		ll_item_t *p = comm_cur_ptr; */
/* 		fprintf(stderr, "init_comm_ptr\n"); */

/* 		while (p != comm_cur_ptr) { */
/* 			if (p->data_type & DT_WEIGHTED) { */
/* 				fprintf(stderr, "%s", ((weighted_item_t *)p)->symbol); */
/* 			} */
/* 			p = p->next; */
/* 		} */
/* 	} */

	assert(comm_cur_ptr);

/* 	fprintf(stderr, "init_comm_ptr -- %s %p %p %p %d\n", */
/* 			((weighted_item_t *)(comm_cur_ptr->data))->symbol, */
/* 			comm_cur_ptr, comm_cur_ptr->prev, comm_cur_ptr->next, */
/* 			ll_len(comm_cur_ptr)); */

	comm_prev_ptr = NULL;
	return comm_cur_ptr;
}

static ll_item_t *next_comm_ptr(void)
{
	/* at the moment, only support one section in comm */
	assert(cur_base->section_list->next == NULL);

	comm_prev_ptr = comm_cur_ptr;

    while (comm_cur_ptr && ((comm_cur_ptr = comm_cur_ptr->next) != NULL)) {
		if (!(comm_cur_ptr->data_type & DT_REORDER)) {
			break;
		}
	}

	return comm_cur_ptr;
}

static int dump_count;

#if 0
static void dump_section(section_t *s, int mask, col_locale_t *der)
{
	ll_item_t *lli;
	ll_item_t *lli0;
	weighted_item_t *w;
	weight_t *p;
	int i;

	lli0 = lli = s->itm_list;

	if (!lli0) {
		return;
	}

	do {
		if (!(lli->data_type & mask)) {
			lli = lli->next;
			continue;
		}
		if (lli->data_type & DT_WEIGHTED) {
			++dump_count;
			w = (weighted_item_t *)(lli->data);
			p = w->weight;
			printf("%6d: %s (%d) ", dump_count, w->symbol, p->num_weights);
			for (i = 0 ; i < p->num_weights ; i++) {
				if (p->rule[i] & R_FORWARD) {
					printf("F");
				}
				if (p->rule[i] & R_BACKWARD) {
					printf("B");
				}
				if (p->rule[i] & R_POSITION) {
					printf("P");
				}
				printf(",");
			}
			for (i = 0 ; i < p->num_weights ; i++) {
				printf("   %s", p->colitem[i]);
			}
			printf("\n");
		} else if (lli->data_type & (DT_SECTION|DT_REORDER)) {

			if (lli->data_type == DT_REORDER) {
				assert(der);
				if (strncmp(((section_t *)(lli->data))->name, der->name, strlen(der->name))) {
					lli = lli->next;
					continue;
				}
			}

			if (lli->data_type & DT_SECTION) {
				printf("SECTION -----------------\n");
			} else {
				printf("REORDER -----------------\n");
			}

			dump_section((section_t *)(lli->data), mask, der);
			printf("DONE --------------------\n");
		}
		lli = lli->next;
	} while (lli != lli0);
}
#else
static int in_reorder_section = 0;

static void dump_section(section_t *s, int mask, col_locale_t *der)
{
	ll_item_t *lli;
	ll_item_t *lli0;
	weighted_item_t *w;
	weight_t *p;
	int i;

	lli0 = lli = s->itm_list;

	if (!lli0) {
		return;
	}

	do {
		if (!(lli->data_type & mask)) {
			lli = lli->next;
			continue;
		}
		if (lli->data_type & DT_WEIGHTED) {
			++dump_count;
			w = (weighted_item_t *)(lli->data);
			p = w->weight;
#if 1
			if (in_reorder_section) {
				printf(" %p", w);
			}
#else
			printf("%6d: %s (%d) ", dump_count, w->symbol, p->num_weights);
			for (i = 0 ; i < p->num_weights ; i++) {
				if (p->rule[i] & R_FORWARD) {
					printf("F");
				}
				if (p->rule[i] & R_BACKWARD) {
					printf("B");
				}
				if (p->rule[i] & R_POSITION) {
					printf("P");
				}
				printf(",");
			}
			for (i = 0 ; i < p->num_weights ; i++) {
				printf("   %s", p->colitem[i]);
			}
			printf("\n");
#endif
		} else if (lli->data_type & (DT_SECTION|DT_REORDER)) {

			if (lli->data_type == DT_REORDER) {
				assert(der);
				if (strncmp(((section_t *)(lli->data))->name, der->name, strlen(der->name))) {
					lli = lli->next;
					continue;
				}
			}

			if (lli->data_type & DT_SECTION) {
/* 				printf("SECTION -----------------\n"); */
				assert(0);
			} else {
/* 				printf("REORDER -----------------\n"); */
				in_reorder_section = 1;
			}

			dump_section((section_t *)(lli->data), mask, der);
/* 			printf("DONE --------------------\n"); */
			printf("\n");
			in_reorder_section = 0;
		}
		lli = lli->next;
	} while (lli != lli0);
}
#endif

static void dump_weights(const char *name)
{
	ll_item_t *lli;
	col_locale_t *base;
	col_locale_t *der;
	col_locale_t cl;
	void *p;

	assert(name);

	if (!*name) {				/* use last */
		base = cur_base;
		der = cur_derived;
	} else {
		cl.name = (char *) name;
		if (!(p = tfind(&cl, &root_col_locale, col_locale_cmp))) {
			error_msg("unknown locale: %s", name);
		}
		base = *((col_locale_t **) p);
		der = NULL;
		if (base->base_locale) { /* oops... really derived */
			der = base;
			base = der->base_locale;
		}
	}

	dump_count = 0;

	if (base) {
/* 		printf("BASE - %s\n", base->name); */
		for (lli = base->section_list ; lli ; lli = lli->next) {
/* 			printf("SECTION %s\n", ((section_t *)(lli->data))->name); */
			dump_section((section_t *)(lli->data), ~0, der);
		}
	}

	assert(der != base);

	if (der) {
/* 		printf("DERIVED - %s\n", der->name); */
		for (lli = der->section_list ; lli ; lli = lli->next) {
			if (lli->data_type == DT_SECTION) {
				dump_section((section_t *)(lli->data), DT_WEIGHTED, der);
			}
		}
	}
/* 	printf("DONE\n"); */
}

static void print_starter_node(const void *ptr, VISIT order, int level)
{
    if (order == postorder || order == leaf)  {
		fprintf(stderr, "   %s\n", *(const char **) ptr);
    }
}

static void finalize_base(void)
{
	ll_item_t *s;
	ll_item_t *h;
	ll_item_t *lli;
	ll_item_t *h2;
	ll_item_t *l2;
	ll_item_t *cli;
	ll_item_t *rli = NULL;
	weighted_item_t *w;
	weight_t *p;
	int i, n, mr, r, mi;
	col_locale_t *cl;
	void *mm;

	int num_invariant = 0;
	int num_varying = 0;
	int max_weight;
	int index2weight_len_inc = 1;

	assert(cur_base);
	assert(base_locale_len+1 < BASE_LOCALE_LEN);

	base_locale_array[base_locale_len].name = cur_base->name;
	base_locale_array[base_locale_len].num_weights = 1;
	base_locale_array[base_locale_len].index2weight_offset = index2weight_len;
	base_locale_array[base_locale_len].index2ruleidx_offset = index2ruleidx_len;
	if (!strcmp(cur_base->name,"ja_JP") || !strcmp(cur_base->name,"ko_KR")) {
#warning fix the index2weight check!!
		index2weight_len_inc = 0;
	}
/* 	printf("%s -- index2weight_len = %d\n", cur_base->name, index2weight_len); */

	if (!hcreate(30000)) {
		error_msg("OUT OF MEMORY!");
	}

	/* first pass ... set the fixed indexes */
	final_index = i = 1;
	mr = 0;
	for (s = cur_base->section_list ; s ; s = s->next) {
#if 1
		if (s->data_type & DT_REORDER) { /* a reordered section */
			fprintf(stderr, "pass1: reordered section %s - xxx\n", ((section_t *)(s->data))->name);
			lli = ((section_t *)(s->data))->itm_list;
			r = 0;
			if (lli) {
/* 				r = ll_len( ((section_t *)(lli->data))->itm_list ); */
				r = ll_len(lli) + 1;
			}
			if (r > mr) {
				mr = r;
			}
			fprintf(stderr, "pass1: reordered section %s - %d\n", ((section_t *)(s->data))->name, r);
			continue;
		}
#endif
		h = lli = ((section_t *)(s->data))->itm_list;
		if (!lli) {
			continue;
		}
		do {
			if (lli->data_type & DT_RANGE) {
				i += mr;
				mr = 0;
#warning check ko_kR and 9
/* 				++i; */
				lli->idx = i;
				assert(!rli);
				rli = lli;
				fprintf(stderr, "range pre = %d  after = ", i);
				i += ((range_item_t *)(lli->data))->length + 1;
#warning check ko_kR and 9
/* 				++i; */
				fprintf(stderr, "%d\n", i);
				if (!index2weight_len_inc) { /* ko_KR hack */
					final_index += ((range_item_t *)(lli->data))->length + 1;
				}
/* 				add_final_col_index("RANGE"); */
			} else if (lli->data_type & DT_WEIGHTED) {
				i += mr;
				mr = 0;
				w = (weighted_item_t *)(lli->data);
				if (find_wi_index_reordered(w->symbol)) { /* reordered symbol so skip on first pass */
					++num_varying;
					++i;
					continue;
				}
				++num_invariant;
				index2weight_buffer[index2weight_len] = lli->idx = i++;
				index2weight_len += index2weight_len_inc;
				add_final_col_index(w->symbol);
				
			} else {
				assert(lli->data_type & DT_REORDER);
				r = ll_len( ((section_t *)(lli->data))->itm_list );
#warning check ko_kR and 9
				if (r > mr) {
					mr = r;
				}
/* 				r = 0; */
			}
		} while ((lli = lli->next) != h);
	}

	/* second pass ... set the reordered indexes */
	mi = i + mr;
	mr = i = 0;
	for (s = cur_base->section_list ; s ; s = s->next) {
		h = lli = ((section_t *)(s->data))->itm_list;
		if (!lli) {
			continue;
		}
		do {
			if (lli->data_type & DT_RANGE) {
				i += mr;
				mr = 0;
				i = lli->idx + ((range_item_t *)(lli->data))->length + 1;
#warning check
			} else if ((lli->data_type & DT_WEIGHTED) && !(s->data_type & DT_REORDER)) {
				i += mr;
				mr = 0;
				w = (weighted_item_t *)(lli->data);
				if (find_wi_index_reordered(w->symbol) /* reordered symbol skipped on first pass */
#if 0
					|| (s->data_type & DT_REORDER) /* or in a reordered section */
#endif
					) {
					assert(!(s->data_type & DT_REORDER));
					index2weight_buffer[index2weight_len] = lli->idx = ++i;
					index2weight_len += index2weight_len_inc;
					add_final_col_index(w->symbol);

/* 					fprintf(stdout, "%11s: r %6d %6d %s\n", */
/* 							cur_base->name, lli->idx, final_index_val(w->symbol), w->symbol); */

					continue;
				}
				i = lli->idx;

/* 				fprintf(stdout, "%11s: w %6d %6d %s\n", */
/* 						cur_base->name, lli->idx, final_index_val(w->symbol), w->symbol); */

			} else {
/* 				fprintf(stderr, "section: %s  %d  %d\n", ((section_t *)(s->data))->name, */
/* 						s->data_type, lli->data_type); */
/* 					assert(!(s->data_type & DT_REORDER)); */
/* 				assert(lli->data_type & DT_REORDER); */
#if 1
				if (s->data_type & DT_REORDER) {
					h2 = l2 = lli;
					if (!h2) {
						continue;
					}
				} else {
					assert(s->data_type & DT_SECTION);
					h2 = l2 = ((section_t *)(lli->data))->itm_list;
					if (!h2) {
						continue;
					}
				}


#else
				h2 = l2 = ((section_t *)(lli->data))->itm_list;
				if (!h2) {
					continue;
				}
#endif
				r = 0;
				do {
					assert(l2->data_type & DT_WEIGHTED);
					++r;
					l2->idx = i + r;

/* 					fprintf(stdout, "%s: R %6d        %s\n", */
/* 							((section_t *)(lli->data))->name, l2->idx, ((weighted_item_t *)(l2->data))->symbol); */

				} while ((l2 = l2->next) != h2);
				if (r > mr) {
					mr = r;
				}
			}
		} while ((lli = lli->next) != h);
	}

	/* finally, walk through all derived locales and set non-reordered section items */
	mr = mi;
	for (cli = cur_base->derived_list ; cli ; cli = cli->next) {
		cl = (col_locale_t *)(cli->data);
/* 		fprintf(stderr, "pass3: %d  %s\n", cli->data_type, cl->name); */

/* 		fprintf(stdout, "pass3: %d  %s\n", cli->data_type, cl->name); */

		assert(cli->data_type == DT_COL_LOCALE);

		i = mi;
		for (s = cl->section_list ; s ; s = s->next) {
/* 			if (s->data_type & DT_REORDER) { */
/* 				continue; */
/* 			} */
			h = lli = ((section_t *)(s->data))->itm_list;
			if (!lli) {
				continue;
			}
			do {
				assert(!(lli->data_type & DT_RANGE));
				if (lli->data_type & DT_WEIGHTED) {
/* 					fprintf(stderr, "     %d %d %s\n", lli->data_type, lli->idx, ((weighted_item_t *)(lli->data))->symbol); */
					add_final_col_index(((weighted_item_t *)(lli->data))->symbol);
					if (s->data_type & DT_REORDER) {
						continue;
					}
					assert(lli->idx == INT_MIN);
					lli->idx = ++i;

/* 					fprintf(stdout, "%11s: S %6d %6d %s\n", */
/* 							cl->name, lli->idx, */
/* 							final_index_val(((weighted_item_t *)(lli->data))->symbol), */
/* 							((weighted_item_t *)(lli->data))->symbol); */

				} else {
					assert(0);
					assert(lli->data_type & DT_SECTION);

					h2 = l2 = ((section_t *)(lli->data))->itm_list;
					if (!h2) {
						continue;
					}
					do {
						assert(l2->data_type & DT_WEIGHTED);
						assert(l2->idx == INT_MIN);
						l2->idx = ++i;
						add_final_col_index(((weighted_item_t *)(l2->data))->symbol);
					} while ((l2 = l2->next) != h2);
				}
			} while ((lli = lli->next) != h);
		}
		if (i > mr) {
			mr = i;
		}
	}
	max_weight = mr;

	assert(num_varying == tnumnodes(cur_base->root_wi_index_reordered));

	/* we can now initialize the wcs2index array */
	{
		ENTRY *p;
		ENTRY e;
		char buf[8];
		static const char xd[] = "0123456789ABCDEF";
		int starter_index = final_index;
		int wcs2index_count = 0;

		strcpy(buf, "<U....>");
		memset(wcs2index, 0, sizeof(wcs2index));
		e.key = (char *) buf;
		for (i=1 ; i <= 0xffff ; i++) {
			buf[5] = xd[ i & 0xf ];
			buf[4] = xd[ (i >> 4) & 0xf ];
			buf[3] = xd[ (i >> 8) & 0xf ];
			buf[2] = xd[ (i >> 12) & 0xf ];

			if ((p = hsearch(e, FIND)) != NULL) {
				++wcs2index_count;
				if ((tfind(buf, &cur_base->root_starter_char, sym_cmp)) != NULL) {
					wcs2index[i] = ++starter_index;
/* 					fprintf(stderr, "wcs2index[ %#06x ] = %d  (starter)\n", i, wcs2index[i]); */
				} else {
					wcs2index[i] = (int)(p->data);
/* 					fprintf(stderr, "wcs2index[ %#06x ] = %d\n", i, wcs2index[i]); */
				}
			} else {
				if ((tfind(buf, &cur_base->root_starter_char, sym_cmp)) != NULL) {
					error_msg("marked starter but not in hash: %s", buf);
				}
			}
		}


	/* ---------------------------------------------------------------------- */
		{
			int i, n;
			table_data table;
			size_t t, smallest;

			n = 0;
			smallest = SIZE_MAX;
			table.ii = NULL;
			for (i=0 ; i < 14 ; i++) {
				if ((RANGE >> i) < 4) {
					break;
				}
				t = newopt(wcs2index, RANGE, i, &table);
				if (smallest >= t) {
					n = i;
					smallest = t;
					/*  			} else { */
					/*  				break; */
				}
			}


/* 			printf("smallest = %u  for range %#x (%u)\n", smallest, RANGE, RANGE); */
			assert(smallest != SIZE_MAX);
			if (smallest + wcs2colidt_len >= WCS2COLIDT_LEN) {
				error_msg("WCS2COLIDT_LEN too small");
			}
			base_locale_array[base_locale_len].wcs2colidt_offset = wcs2colidt_len;
			table.ii = wcs2colidt_buffer + wcs2colidt_len;
			t = smallest;
			smallest = SIZE_MAX;
			smallest = newopt(wcs2index, RANGE, n, &table);
			assert(t == smallest);
			wcs2colidt_len += smallest;
/* 			fprintf(stderr, "smallest = %d   wcs2colidt_len = %d\n", smallest, wcs2colidt_len); */

#if 0
			{
				unsigned int sc, n, i0, i1;
				unsigned int u = 0xe40;
				table_data *tbl = &table;

#define __LOCALE_DATA_WCctype_TI_MASK ((1 << tbl->ti_shift)-1)
#define __LOCALE_DATA_WCctype_TI_SHIFT (tbl->ti_shift)
#define __LOCALE_DATA_WCctype_TI_LEN (tbl->ti_len)
#define __LOCALE_DATA_WCctype_II_MASK ((1 << tbl->ii_shift)-1)
#define __LOCALE_DATA_WCctype_II_SHIFT (tbl->ii_shift)
#define __LOCALE_DATA_WCctype_II_LEN (tbl->ii_len)

				sc = u & __LOCALE_DATA_WCctype_TI_MASK;
				u >>= __LOCALE_DATA_WCctype_TI_SHIFT;
				n = u & __LOCALE_DATA_WCctype_II_MASK;
				u >>= __LOCALE_DATA_WCctype_II_SHIFT;

				i0 = tbl->ii[u];
				fprintf(stderr, "i0 = %d\n", i0);
				i0 <<= __LOCALE_DATA_WCctype_II_SHIFT;
				i1 = tbl->ii[__LOCALE_DATA_WCctype_II_LEN + i0 + n];
				/* 	i1 = tbl->ti[i0 + n]; */
				fprintf(stderr, "i1 = %d\n", i1);
				i1 <<= __LOCALE_DATA_WCctype_TI_SHIFT;
				/* 	return *(uint16_t *)(&(tbl->ii[__LOCALE_DATA_WCctype_II_LEN + __LOCALE_DATA_WCctype_TI_LEN + i1 + sc])); */
				fprintf(stderr, "i2 = %d\n", __LOCALE_DATA_WCctype_II_LEN + __LOCALE_DATA_WCctype_TI_LEN + i1 + sc);
				fprintf(stderr, "val = %d\n",  tbl->ii[__LOCALE_DATA_WCctype_II_LEN + __LOCALE_DATA_WCctype_TI_LEN + i1 + sc]);
				/* 	return tbl->ut[i1 + sc]; */


			}
#endif
			base_locale_array[base_locale_len].ii_shift = table.ii_shift;
			base_locale_array[base_locale_len].ti_shift = table.ti_shift;
			base_locale_array[base_locale_len].ii_len = table.ii_len;
			base_locale_array[base_locale_len].ti_len = table.ti_len;
		}
	/* ---------------------------------------------------------------------- */

		base_locale_array[base_locale_len].num_col_base = num_invariant + num_varying;
		base_locale_array[base_locale_len].max_col_index = final_index;
		base_locale_array[base_locale_len].max_weight = max_weight;

		fprintf(stderr, "%s: %6u invariant  %6u varying  %6u derived  %6u total  %6u max weight  %6u wcs2\n",
				cur_base->name, num_invariant, num_varying,
				tnumnodes(cur_base->root_derived_wi), final_index, max_weight,
				wcs2index_count);

	}

#if 1
	/* ok, now we need to dump out the base and derived tables... */
	/* don't forget to break up collating elements!!! */

/* 	fprintf(stdout, "**************************************************\n"); */
	/* first pass ... set the invariants */
	for (s = cur_base->section_list ; s ; s = s->next) {
#if 1
		if (s->data_type & DT_REORDER) {
			fprintf(stderr, "1: skipping reordered section %s\n", ((section_t *)(s->data))->name);
			continue;
		}
#endif
		h = lli = ((section_t *)(s->data))->itm_list;
		if (!lli) {
			continue;
		}
		do {
			if (lli->data_type & DT_WEIGHTED) {
				w = (weighted_item_t *)(lli->data);
				if (find_wi_index_reordered(w->symbol)) { /* reordered symbol so skip on first pass */
					continue;
				}
				if (index2weight_len_inc) {
					index2ruleidx_buffer[index2ruleidx_len++] = 
						add_rule((weighted_item_t *)(lli->data));
				}
/* 				fprintf(stdout, "%11s: w %6d %6d %s\n", */
/* 						cur_base->name, lli->idx, final_index_val(w->symbol), w->symbol); */
			}
		} while ((lli = lli->next) != h);
	}

	/* second pass ... set varying */
	for (s = cur_base->section_list ; s ; s = s->next) {
#if 1
		if (s->data_type & DT_REORDER) {
			fprintf(stderr, "2: skipping reordered section %s\n", ((section_t *)(s->data))->name);
			continue;
		}
#endif
		h = lli = ((section_t *)(s->data))->itm_list;
		if (!lli) {
			continue;
		}
		do {
			if (lli->data_type & DT_WEIGHTED) {
				w = (weighted_item_t *)(lli->data);
				if (find_wi_index_reordered(w->symbol)) { /* reordered symbol so skip on first pass */
					if (index2weight_len_inc) {
						index2ruleidx_buffer[index2ruleidx_len++] = 
							add_rule((weighted_item_t *)(lli->data));
					}
/* 					fprintf(stdout, "%11s: r %6d %6d %s\n", */
/* 							cur_base->name, lli->idx, final_index_val(w->symbol), w->symbol); */
					continue;
				}
			}
		} while ((lli = lli->next) != h);
	}

	do_starter_lists(cur_base);


/* 	fprintf(stderr,"updated final_index = %d\n", final_index); */

	if (rli) {
		base_locale_array[base_locale_len].range_low
			= strtoul(((range_item_t *)(rli->data))->symbol1 + 2, NULL, 16);
		base_locale_array[base_locale_len].range_count
			= ((range_item_t *)(rli->data))->length;
		base_locale_array[base_locale_len].range_base_weight = rli->idx;
		base_locale_array[base_locale_len].range_rule_offset = add_range_rule((range_item_t *)(rli->data));
/* 		fprintf(stdout, "%11s:   %6d %6d %s %s (%d)\n", */
/* 				"RANGE", rli->idx, -1, */
/* 				((range_item_t *)(rli->data))->symbol1, */
/* 				((range_item_t *)(rli->data))->symbol2, */
/* 				((range_item_t *)(rli->data))->length); */
	}

/* 	fprintf(stdout,"\nDerived\n\n"); */

	/* first, if base name is of the form ll_CC, add a derived locale for it */
	if ((strlen(cur_base->name) == 5)
		&& islower(cur_base->name[0])
		&& islower(cur_base->name[1])
		&& (cur_base->name[2] == '_')
		&& isupper(cur_base->name[3])
		&& isupper(cur_base->name[4])
		) {

		fprintf(stderr, "adding special derived for %s\n", cur_base->name);
/* 	fprintf(stderr,"updated final_index = %d\n", final_index); */


		assert(der_locale_len+1 < DER_LOCALE_LEN);

		der_locale_array[der_locale_len].name = cur_base->name;
		der_locale_array[der_locale_len].base_idx = base_locale_len;
		
		u16_buf[0] = 1;
		u16_buf[1] = 0;
		u16_buf_len = 2;

		mm = NULL;
		if ((u16_buf_len > override_len) ||
			!(mm = memmem(override_buffer, override_len*sizeof(override_buffer[0]),
						  u16_buf, u16_buf_len*sizeof(u16_buf[0])))
			) {
			assert(override_len + u16_buf_len < OVERRIDE_LEN);
			memcpy(override_buffer + override_len, u16_buf, u16_buf_len*sizeof(u16_buf[0]));
			der_locale_array[der_locale_len].overrides_offset = override_len;
			override_len += u16_buf_len;
/* 			printf("%s: override_len = %d   u16_buf_len = %d\n", cl->name, override_len, u16_buf_len); */
		} else if (!(u16_buf_len > override_len)) {
			assert(mm);
			der_locale_array[der_locale_len].overrides_offset = ((uint16_t *)(mm)) - override_buffer;
/* 			printf("%s: memmem found a match with u16_buf_len = %d\n", cl->name, u16_buf_len); */
		}
		der_locale_array[der_locale_len].multistart_offset
			= base_locale_array[base_locale_len].multistart_offset;
		der_locale_array[der_locale_len].undefined_idx = final_index_val0("UNDEFINED");

		if (!der_locale_array[der_locale_len].undefined_idx) {
			error_msg("no UNDEFINED definition for %s", cur_base->name);
		}

		++der_locale_len;
	} else {
		fprintf(stderr, "NOT adding special derived for %s\n", cur_base->name);
	}

	/* now all the derived... */
	for (cli = cur_base->derived_list ; cli ; cli = cli->next) {
		cl = (col_locale_t *)(cli->data);
		assert(cli->data_type == DT_COL_LOCALE);

		assert(der_locale_len+1 < DER_LOCALE_LEN);

		der_locale_array[der_locale_len].name = cl->name;
		der_locale_array[der_locale_len].base_idx = base_locale_len;

		u16_buf_len = 0;

		for (i = 0 ; i < 2 ; i++) {
			if (i) {
/* 				fprintf(stdout, "   section --- (singles)\n"); */
				u16_buf[u16_buf_len++] = 1;	/* single */
			}
			/* we do this in two passes... first all sequences, then all single reorders */
			for (s = cl->section_list ; s ; s = s->next) {
/* 				fprintf(stderr, "doing section %s\n", ((section_t *)(s->data))->name); */
				h = lli = ((section_t *)(s->data))->itm_list;
				if (!lli) {
/* 					fprintf(stdout, "EMPTY ITEM LIST IN SECTION %s\n", ((section_t *)(s->data))->name ); */
					continue;
				}
				assert(u16_buf_len +4 < sizeof(u16_buf)/sizeof(u16_buf[0]));
				if ((!i && (ll_len(h) > 1) ) || (ll_len(h) == i)) {
					if (!i) {
/* 						fprintf(stdout, "   section ----------------- %d %d\n", i, ll_len(h)); */
						u16_buf[u16_buf_len++] = ll_len(h);	/* multi */
						assert(lli->data_type & DT_WEIGHTED);
#if 0
						u16_buf[u16_buf_len++] = final_index_val(((weighted_item_t *)(lli->data))->symbol);	/* start index */
#endif
						u16_buf[u16_buf_len++] = lli->idx; /* start weight */
					}
					do {
						assert(lli->data_type & DT_WEIGHTED);
						if (lli->data_type & DT_WEIGHTED) {
/* 							fprintf(stdout, "%11s: S %6d %6d %s\n", */
/* 									cl->name, lli->idx, */
/* 									final_index_val(((weighted_item_t *)(lli->data))->symbol), */
/* 									((weighted_item_t *)(lli->data))->symbol); */
#if 0
							if (i) {
								assert(u16_buf_len +4 < sizeof(u16_buf)/sizeof(u16_buf[0]));
								u16_buf[u16_buf_len++] = final_index_val(((weighted_item_t *)(lli->data))->symbol);
								assert(u16_buf[u16_buf_len-1]);
								u16_buf[u16_buf_len++] = lli->idx; /* weight */
							}
#else
							assert(u16_buf_len +4 < sizeof(u16_buf)/sizeof(u16_buf[0]));
							u16_buf[u16_buf_len++] = final_index_val(((weighted_item_t *)(lli->data))->symbol);
							assert(u16_buf[u16_buf_len-1]);
							if (i) {
								u16_buf[u16_buf_len++] = lli->idx; /* weight */
							}
#endif
							u16_buf[u16_buf_len++] = add_rule((weighted_item_t *)(lli->data));

						}
					} while ((lli = lli->next) != h);
				}
			}
		}
		u16_buf[u16_buf_len++] = 0;

		mm = NULL;
		if ((u16_buf_len > override_len) ||
			!(mm = memmem(override_buffer, override_len*sizeof(override_buffer[0]),
						  u16_buf, u16_buf_len*sizeof(u16_buf[0])))
			) {
			assert(override_len + u16_buf_len < OVERRIDE_LEN);
			memcpy(override_buffer + override_len, u16_buf, u16_buf_len*sizeof(u16_buf[0]));
			der_locale_array[der_locale_len].overrides_offset = override_len;
			override_len += u16_buf_len;
/* 			printf("%s: override_len = %d   u16_buf_len = %d\n", cl->name, override_len, u16_buf_len); */
		} else if (!(u16_buf_len > override_len)) {
			assert(mm);
			der_locale_array[der_locale_len].overrides_offset = ((uint16_t *)(mm)) - override_buffer;
/* 			printf("%s: memmem found a match with u16_buf_len = %d\n", cl->name, u16_buf_len); */
		}

		do_starter_lists(cl);

		der_locale_array[der_locale_len].undefined_idx = final_index_val0("UNDEFINED");
#if 0
		assert(der_locale_array[der_locale_len].undefined_idx);
		if (!der_locale_array[der_locale_len].undefined_idx) {
			der_locale_array[der_locale_len].undefined_idx = base_locale_array[base_locale_len].undefined_idx;
		}
#endif

		if (!der_locale_array[der_locale_len].undefined_idx) {
			error_msg("no UNDEFINED definition for %s", cl->name);
		}

		++der_locale_len;
	}

#endif

#warning handle UNDEFINED idx specially?  what if in only some of derived?
/* 	base_locale_array[base_locale_len].undefined_idx = final_index_val0("UNDEFINED"); */
	base_locale_array[base_locale_len].undefined_idx = 0;


	hdestroy();

	++base_locale_len;

/* 	if (tnumnodes(cur_base->root_starter_char)) { */
/* 		fprintf(stderr, "starter nodes\n"); */
/* 		twalk(cur_base->root_starter_char, print_starter_node); */
/* 	} */
}

static int starter_all_cmp(const void *n1, const void *n2)
{
	const char *s1 = ((weighted_item_t *) n1)->symbol;
	const char *s2 = ((weighted_item_t *) n2)->symbol;
	colitem_t x;
	colitem_t *p;
	int n;

	/* sort by 1st char ... then inverse for string */

	x.element = NULL;
	if (!is_ucode(s1)) {
		x.string = s1;
		p = tfind(&x, &cur_base->root_colitem, colitem_cmp);
		s1 = (*((colitem_t **) p))->element + 1;
	}
	if (!is_ucode(s2)) {
		x.string = s2;
		p = tfind(&x, &cur_base->root_colitem, colitem_cmp);
		s2 = (*((colitem_t **) p))->element + 1;
	}

	/* <U####>< */
	/* 01234567 */

	assert(is_ucode(s1));
	assert(is_ucode(s2));

	n = strncmp(s1+2, s2+2, 4);
	if (n) {
		return n;
	}

	s1 += 7;
	s2 += 7;

	return strcmp(s2, s1);
}

static void print_starter_all_node(const void *ptr, VISIT order, int level)
{
    const weighted_item_t *w = *(const weighted_item_t **) ptr;
	colitem_t *ci;
	void *p;
	int n;
	colitem_t x;

    if (order == postorder || order == leaf)  {
#if 0
		if ((n = is_ucode(w->symbol)) != 0) {
			printf(" %s\n", w->symbol);
		} else {
			x.string = w->symbol;
			x.element = NULL;
			p = tfind(&x, &cur_base->root_colitem, colitem_cmp);
			assert(p);
			ci = *((colitem_t **) p);
			printf("%s = %s\n", ci->element, w->symbol);
		}
#else
		printf("%s|", w->symbol);
/* 		if ((n = is_ucode(w->symbol)) != 0) { */
/* 			printf("\n"); */
/* 		} */
#endif
	}
}

static void process_starter_node(const void *ptr, VISIT order, int level)
{
    const weighted_item_t *w = *(const weighted_item_t **) ptr;
	colitem_t *ci;
	void *p;
	int n;
	colitem_t x;
	const char *s;
	char buf[32];

	/* store index of collation item followed by (unprefixed) nul-terminated string */
    if (order == postorder || order == leaf)  {
		if ((n = is_ucode(w->symbol)) != 0) {
			u16_buf[u16_buf_len++] = final_index_val(w->symbol);
			assert(u16_buf[u16_buf_len-1]);
			u16_buf[u16_buf_len++] = 0;
			if (++u16_starter < base_locale_array[base_locale_len].num_starters) {
				u16_buf[u16_starter] = u16_buf_len;
			}
/* 			fprintf(stderr, "ucode - %d %d\n", u16_buf[u16_starter-1], u16_buf_len); */
		} else {
			x.string = w->symbol;
			x.element = NULL;
			p = tfind(&x, &cur_base->root_colitem, colitem_cmp);
			assert(p);
			ci = *((colitem_t **) p);
			s = ci->element;
			u16_buf[u16_buf_len++] = final_index_val(w->symbol);
			assert(u16_buf[u16_buf_len-1]);
			assert(*s == '"');
			n = is_ucode(++s);
/* 			fprintf(stderr, "s is |%s| with len %d (%d)\n", s, strlen(s), n); */
			assert(n);
			s += n;
			while (*s != '"') {
				n = is_ucode(s);
				assert(n);
				strncpy(buf, s, n+1);
				buf[n] = 0;
/* 				fprintf(stderr, "buf is |%s| with len %d (%d)\n", buf, strlen(buf), n); */
				u16_buf[u16_buf_len++] = final_index_val(buf);
				assert(u16_buf[u16_buf_len-1]);
				s += n;
			}
			u16_buf[u16_buf_len++] = 0;
		}
	}
}

static void **p_cl_root_starter_all;

static void complete_starter_node(const void *ptr, VISIT order, int level)
{
	weighted_item_t w;
	weighted_item_t *p;

    if (order == postorder || order == leaf)  {
		w.symbol = *(const char **) ptr;
		w.weight = NULL;
		if (!tfind(&w, p_cl_root_starter_all, starter_all_cmp)) {
			p = xmalloc(sizeof(weighted_item_t));
			p->symbol = w.symbol;
			p->weight = NULL;
/* 			fprintf(stderr, "complete_starter_node: %s\n", *(const char **) ptr); */
			if (!tsearch(p, p_cl_root_starter_all, starter_all_cmp)) {
				error_msg("OUT OF MEMORY");
			}
		}
    }
}

static void do_starter_lists(col_locale_t *cl)
{
	ll_item_t *s;
	ll_item_t *h;
	ll_item_t *lli;
	col_locale_t *c;
	colitem_t *ci;
	weighted_item_t *w;
	void *p;
	char buf[32];
	int n;
	colitem_t x;
	void *mm;

	c = cl;
	if (c != cur_base) {
		c = cur_base;
	}

/* 	printf("STARTERS %s --------------------\n", cl->name); */
 LOOP:
	for (s = c->section_list ; s ; s = s->next) {
		h = lli = ((section_t *)(s->data))->itm_list;
		if (!lli) {
			continue;
		}
		do {
			if (lli->data_type & DT_WEIGHTED) {
				w = (weighted_item_t *)(lli->data);
				ci = NULL;
				if ((n = is_ucode(w->symbol)) != 0) {
					strcpy(buf, w->symbol);
				} else {
/* 					fprintf(stdout, "looking for |%s|\n", w->symbol); */
					x.string = w->symbol;
					x.element = NULL;
					p = tfind(&x, &cur_base->root_colitem, colitem_cmp);
					if (!p) {
/* 						fprintf(stderr, "Whoa... processing starters for %s and couldn't find %s\n", */
/* 								cl->name, w->symbol); */
						continue;
					}
					ci = *((colitem_t **) p);
					if (!ci->element) {	/* just a collating symbol */
						continue;
					}
					assert(ci->element[0] == '"');
					n = is_ucode(ci->element + 1);
					assert(n);
					strncpy(buf, ci->element + 1, n);
				}
				if ((tfind(buf, &cur_base->root_starter_char, sym_cmp)) != NULL) {
/* 					fprintf(stdout, "adding from %s: %s", c->name, w->symbol); */
/* 					if (ci) { */
/* 						fprintf(stdout, " = %s", ci->element); */
/* 					} */
/* 					fprintf(stdout, "\n"); */

					if (!tsearch(w, &cl->root_starter_all, starter_all_cmp)) {
						error_msg("OUT OF MEMORY");
					}
				}
			}
		} while ((lli = lli->next) != h);
	}

	if (c != cl) {
		c = cl;
		goto LOOP;
	}

	p_cl_root_starter_all = &cl->root_starter_all;
	twalk(cur_base->root_starter_char, complete_starter_node);

	if (cl == cur_base) {
		base_locale_array[base_locale_len].num_starters	= tnumnodes(cur_base->root_starter_char);
	}

#if 0
	printf("\nNow walking tree...\n\n");
	twalk(cl->root_starter_all, print_starter_all_node);
	printf("\n\n");

#endif
	u16_starter = 0;
	u16_buf[0] = u16_buf_len = base_locale_array[base_locale_len].num_starters;
	twalk(cl->root_starter_all, process_starter_node);
/* 	fprintf(stderr, "s=%d n=%d\n", u16_starter,  base_locale_array[base_locale_len].num_starters); */
	assert(u16_starter == base_locale_array[base_locale_len].num_starters);

#if 0
	{ int i;
	for (i=0 ; i < u16_buf_len ; i++) {
		fprintf(stderr, "starter %2d: %d - %#06x\n", i, u16_buf[i], u16_buf[i]);
	}}
#endif

	mm = NULL;
	if (u16_buf_len) {
/* 		assert(base_locale_array[base_locale_len].num_starters); */
		if ((u16_buf_len > multistart_len) ||
			!(mm = memmem(multistart_buffer, multistart_len*sizeof(multistart_buffer[0]),
						  u16_buf, u16_buf_len*sizeof(u16_buf[0])))
			) {
			assert(multistart_len + u16_buf_len < MULTISTART_LEN);
			memcpy(multistart_buffer + multistart_len, u16_buf, u16_buf_len*sizeof(u16_buf[0]));
			if (cl == cur_base) {
				base_locale_array[base_locale_len].multistart_offset = multistart_len;
			} else {
				der_locale_array[der_locale_len].multistart_offset = multistart_len;
			}
			multistart_len += u16_buf_len;
/* 			fprintf(stderr, "%s: multistart_len = %d   u16_buf_len = %d\n", cl->name, multistart_len, u16_buf_len); */
		} else if (!(u16_buf_len > multistart_len)) {
			assert(mm);
			if (cl == cur_base) {
				base_locale_array[base_locale_len].multistart_offset = ((uint16_t *)(mm)) - multistart_buffer;
			} else {
				der_locale_array[der_locale_len].multistart_offset = ((uint16_t *)(mm)) - multistart_buffer;
			}
/* 			fprintf(stderr, "%s: memmem found a match with u16_buf_len = %d\n", cl->name, u16_buf_len); */
		}
	} else {
		assert(!base_locale_array[base_locale_len].num_starters);
	}

/* 	printf("u16_buf_len = %d\n", u16_buf_len); */

/* 	printf("STARTERS %s DONE ---------------\n", cl->name); */
}


/* For sorting the blocks of unsigned chars. */
static size_t nu_val;

int nu_memcmp(const void *a, const void *b)
{
	return memcmp(*(unsigned char**)a, *(unsigned char**)b, nu_val * sizeof(tbl_item));
}


size_t newopt(tbl_item *ut, size_t usize, int shift, table_data *tbl)
{
	static int recurse = 0;
	tbl_item *ti[RANGE];	/* table index */
	size_t numblocks;
	size_t blocksize;
	size_t uniq;
	size_t i, j;
	size_t smallest, t;
	tbl_item *ii_save;
	int uniqblock[1 << (8*sizeof(tbl_item) - 1)];
	tbl_item uit[RANGE];
	int shift2;

	if (shift > 15) {
		return SIZE_MAX;
	}

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
		if (memcmp(ti[i-1], ti[i], blocksize*sizeof(tbl_item)) < 0) {
			if (++uniq > (1 << (8*sizeof(tbl_item) - 1))) {
				break;
			}
			uniqblock[uniq - 1] = i;
		}
#if 1
		else if (memcmp(ti[i-1], ti[i], blocksize*sizeof(tbl_item)) > 0) {
			printf("bad sort %i!\n", i);
			abort();
		}
#endif
		uit[(ti[i]-ut)/blocksize] = uniq - 1;
	}

	smallest = SIZE_MAX;
	shift2 = -1;
	if (uniq <= (1 << (8*sizeof(tbl_item) - 1))) {
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
/* 					if (!tbl->ii) { */
/* 						printf("ishift %u  tshift %u  size %u\n", */
/* 							   shift2, shift, t); */
/* 					} */
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
			memcpy(tbl->ii, uit, numblocks*sizeof(tbl_item));
			tbl->ti = tbl->ii + tbl->ii_len;
			tbl->ti_len = uniq * blocksize;
			for (i=0 ; i < uniq ; i++) {
				memcpy(tbl->ti + i * blocksize, ti[uniqblock[i]], blocksize*sizeof(tbl_item));
			}
		} else {
			++recurse;
/* 			printf("setting ishift %u  tshift %u\n", shift2, shift); */
			newopt(uit, numblocks, shift2, tbl);
			--recurse;
			tbl->ti_shift = shift;
			tbl->ut_len = uniq * blocksize;
			tbl->ut = tbl->ti + tbl->ti_len;
			for (i=0 ; i < uniq ; i++) {
				memcpy(tbl->ut + i * blocksize, ti[uniqblock[i]], blocksize*sizeof(tbl_item));
			}
		}
	}
	return smallest;
}

static const int rule2val[8] = {
	-1,
	(1 << 14),					/* forward */
	(2 << 14),					/* position */
	(3 << 14),					/* forward,position */
	0,							/* backward */
	-1,
	-1,
	-1,
};


static int final_index_val_x(const char *s, const char *sym)
{
	int r;

	if (!(r = final_index_val0(s))) {
		if (!strcmp(s, "IGNORE")) {
			r = 0;
		} else if (!strcmp(s, "..") || !strcmp(sym, "RANGE")) {
			if (*sym == '.') {
				final_index_val(sym); /* make sure it's known */
			}
			r = 0x3fff;
		} else if (!strcmp(s, ".")) {
			r = 0x3ffe;
		} else {
			error_msg("can't find final index: %s", s);
		}
	}
	return r;
}

/* store rule2val in 2 high bits and collation index in lower.
 * for sort strings, store (offset from base) + max colindex as index.
 */
static unsigned int add_rule(weighted_item_t *wi)
{
	weight_t *w = wi->weight;
	int i, j, r, n;
	uint16_t rbuf[MAX_COLLATION_WEIGHTS];
	uint16_t ws_buf[32];
	void *mm;
	char buf[32];
	const char *s;
	const char *e;

	for (i=0 ; i < MAX_COLLATION_WEIGHTS ; i++) {
		rbuf[i] = rule2val[R_FORWARD]; /* set a default to forward-ignore */
	}

	if (base_locale_array[base_locale_len].num_weights < w->num_weights) {
		base_locale_array[base_locale_len].num_weights = w->num_weights;
	}

	for (i=0 ; i < w->num_weights ; i++) {
		assert(rule2val[(int)(w->rule[i])] >= 0);
		assert(w->colitem[i] && *w->colitem[i]);
		if (*w->colitem[i] == '"') { /* string... */
			s = w->colitem[i] + 1;
			assert(*s == '<');
			n = 0;
			do {
				e = s;
				do {
					if (*e == '/') {
						e += 2;
						continue;
					}
				} while (*e++ != '>');
				assert(((size_t)(e-s) < sizeof(buf)));
				memcpy(buf, s, (size_t)(e-s));
				buf[(size_t)(e-s)] = 0;

				r = final_index_val_x(buf, wi->symbol);
				assert(n + 1 < sizeof(ws_buf)/sizeof(ws_buf[0]));
				ws_buf[n++] = r | rule2val[(int)(w->rule[i])];

				s = e;
			} while (*s != '"');
			ws_buf[n++] = 0;	/* terminator */

			mm = memmem(weightstr_buffer, weightstr_len*sizeof(weightstr_buffer[0]),
						ws_buf, n*sizeof(ws_buf[0]));

			if (!mm) {
				assert(weightstr_len + n < WEIGHTSTR_LEN);
				memcpy(weightstr_buffer + weightstr_len, ws_buf, n*sizeof(ws_buf[0]));
				mm = weightstr_buffer + weightstr_len;
				weightstr_len += n;
			}
			r = (((uint16_t *)(mm)) - weightstr_buffer)
				+ base_locale_array[base_locale_len].max_col_index + 2;
			assert(r < (1 << 14));
			rbuf[i] = r | rule2val[(int)(w->rule[i])];
		} else {				/* item */
			r = final_index_val_x(w->colitem[i], wi->symbol); 
			rbuf[i] = r | rule2val[(int)(w->rule[i])];
		}
	}

	for (i=0 ; i < ruletable_len ; i += MAX_COLLATION_WEIGHTS) {
		if (!memcmp(ruletable_buffer + i, rbuf, MAX_COLLATION_WEIGHTS*sizeof(ruletable_buffer[0]))) {
			return i/MAX_COLLATION_WEIGHTS;
		}
	}

	memcpy(ruletable_buffer + ruletable_len, rbuf, MAX_COLLATION_WEIGHTS*sizeof(ruletable_buffer[0]));
	ruletable_len += MAX_COLLATION_WEIGHTS;

	return  (ruletable_len / MAX_COLLATION_WEIGHTS)-1;
}

static unsigned int add_range_rule(range_item_t *ri)
{
	weight_t *w = ri->weight;
	int i, j, r, n;
	uint16_t rbuf[MAX_COLLATION_WEIGHTS];
	uint16_t ws_buf[32];
	void *mm;
	char buf[32];
	const char *s;
	const char *e;

	for (i=0 ; i < MAX_COLLATION_WEIGHTS ; i++) {
		rbuf[i] = rule2val[R_FORWARD]; /* set a default to forward-ignore */
	}

	if (base_locale_array[base_locale_len].num_weights < w->num_weights) {
		base_locale_array[base_locale_len].num_weights = w->num_weights;
	}

	for (i=0 ; i < w->num_weights ; i++) {
		assert(rule2val[(int)(w->rule[i])] >= 0);
		assert(w->colitem[i] && *w->colitem[i]);
		if (*w->colitem[i] == '"') { /* string... */
			s = w->colitem[i] + 1;
			assert(*s == '<');
			n = 0;
			do {
				e = s;
				do {
					if (*e == '/') {
						e += 2;
						continue;
					}
				} while (*e++ != '>');
				assert(((size_t)(e-s) < sizeof(buf)));
				memcpy(buf, s, (size_t)(e-s));
				buf[(size_t)(e-s)] = 0;

				r = final_index_val_x(buf, "RANGE");
				assert(n + 1 < sizeof(ws_buf)/sizeof(ws_buf[0]));
				ws_buf[n++] = r | rule2val[(int)(w->rule[i])];

				s = e;
			} while (*s != '"');
			ws_buf[n++] = 0;	/* terminator */

			mm = memmem(weightstr_buffer, weightstr_len*sizeof(weightstr_buffer[0]),
						ws_buf, n*sizeof(ws_buf[0]));

			if (!mm) {
				assert(weightstr_len + n < WEIGHTSTR_LEN);
				memcpy(weightstr_buffer + weightstr_len, ws_buf, n*sizeof(ws_buf[0]));
				mm = weightstr_buffer + weightstr_len;
				weightstr_len += n;
			}
			r = (((uint16_t *)(mm)) - weightstr_buffer)
				+ base_locale_array[base_locale_len].max_col_index + 2;
			assert(r < (1 << 14));
			rbuf[i] = r | rule2val[(int)(w->rule[i])];
		} else {				/* item */
			r = final_index_val_x(w->colitem[i], "RANGE");
			rbuf[i] = r | rule2val[(int)(w->rule[i])];
		}
	}

	for (i=0 ; i < ruletable_len ; i += MAX_COLLATION_WEIGHTS) {
		if (!memcmp(ruletable_buffer + i, rbuf, MAX_COLLATION_WEIGHTS*sizeof(ruletable_buffer[0]))) {
			return i/MAX_COLLATION_WEIGHTS;
		}
	}

	memcpy(ruletable_buffer + ruletable_len, rbuf, MAX_COLLATION_WEIGHTS*sizeof(ruletable_buffer[0]));
	ruletable_len += MAX_COLLATION_WEIGHTS;

	return  (ruletable_len / MAX_COLLATION_WEIGHTS)-1;
}

#define DUMPn(X) fprintf(stderr, "%10d-%-.20s", base_locale_array[n]. X, #X);

static void dump_base_locale(int n)
{
	assert(n < base_locale_len);

	fprintf(stderr, "Base Locale: %s\n", base_locale_array[n].name);

	DUMPn(num_weights);

	DUMPn(ii_shift);
	DUMPn(ti_shift);
	DUMPn(ii_len);
	DUMPn(ti_len);
	DUMPn(max_weight);
	fprintf(stderr, "\n");
	DUMPn(num_col_base);
	DUMPn(max_col_index);
	DUMPn(undefined_idx);
	DUMPn(range_low);
	DUMPn(range_count);
	fprintf(stderr, "\n");
	DUMPn(range_base_weight);
	DUMPn(num_starters);

	fprintf(stderr, "\n");
	DUMPn(range_rule_offset);
	DUMPn(wcs2colidt_offset);
	DUMPn(index2weight_offset);
	fprintf(stderr, "\n");
	DUMPn(index2ruleidx_offset);
	DUMPn(multistart_offset);
	fprintf(stderr, "\n");
}

#undef DUMPn
#define DUMPn(X) fprintf(stderr, "%10d-%s", der_locale_array[n]. X, #X);

static void dump_der_locale(int n)
{
	assert(n < der_locale_len);

	fprintf(stderr, "Derived Locale: %s (%.12s)",
			der_locale_array[n].name,
			base_locale_array[der_locale_array[n].base_idx].name);


	DUMPn(base_idx);

	DUMPn(undefined_idx);
	
	DUMPn(overrides_offset);
	DUMPn(multistart_offset);

	fprintf(stderr, "\n");
}


static unsigned long collate_pos;

static void dump_u16_array(FILE *fp, uint16_t *u, int len, const char *name)
{
	int i;

	fprintf(fp, "\t/* %8lu %s */\n", collate_pos, name);
	for (i=0 ; i < len ; i++) {
		if (!(i & 7)) {
			fprintf(fp, "\n\t");
		}
		fprintf(fp,"  %#06x,", (unsigned int)(u[i]));
	}
	fprintf(fp,"\n");
	collate_pos += len;
}

#define OUT_U16C(X,N) fprintf(fp,"\t%10d, /* %8lu %s */\n", X, collate_pos++, N); 

static void dump_collate(FILE *fp)
{
	int n;

	fprintf(fp, "const uint16_t __locale_collate_tbl[] = {\n");

	OUT_U16C(base_locale_len, "numbef of base locales");
	OUT_U16C(der_locale_len, "number of derived locales");
	OUT_U16C(MAX_COLLATION_WEIGHTS, "max collation weights");
	OUT_U16C(index2weight_len, "number of index2{weight|ruleidx} elements");
	OUT_U16C(weightstr_len, "number of weightstr elements");
	OUT_U16C(multistart_len, "number of multistart elements");
	OUT_U16C(override_len, "number of override elements");
	OUT_U16C(ruletable_len, "number of ruletable elements");

#undef DUMPn
#define DUMPn(X) fprintf(fp, "\t%10d, /* %8lu %s */\n", base_locale_array[n]. X, collate_pos++, #X);
	for (n=0 ; n < base_locale_len ; n++) {
		unsigned wcs2colidt_offset_low = base_locale_array[n].wcs2colidt_offset & 0xffffU;
		unsigned wcs2colidt_offset_hi = base_locale_array[n].wcs2colidt_offset >> 16;
		fprintf(fp, "\t/* Base Locale %2d: %s */\n", n, base_locale_array[n].name);
		DUMPn(num_weights);
		DUMPn(num_starters);
		DUMPn(ii_shift);
		DUMPn(ti_shift);
		DUMPn(ii_len);
		DUMPn(ti_len);
		DUMPn(max_weight);
		DUMPn(num_col_base);
		DUMPn(max_col_index);
		DUMPn(undefined_idx);
		DUMPn(range_low);
		DUMPn(range_count);
		DUMPn(range_base_weight);
		DUMPn(range_rule_offset);
		DUMPn(index2weight_offset);
		DUMPn(index2ruleidx_offset);
		DUMPn(multistart_offset);
#undef DUMPn
#define DUMPn(X) fprintf(fp, "\t%10d, /* %8lu %s */\n", X, collate_pos++, #X);
		DUMPn(wcs2colidt_offset_low);
		DUMPn(wcs2colidt_offset_hi);
	}
#undef DUMPn		


	fprintf(fp, "#define COL_IDX_C     %5d\n", 0);
#define DUMPn(X) fprintf(fp, "\t%10d, /* %8lu %s */\n", der_locale_array[n]. X, collate_pos++, #X);
	for (n=0 ; n < der_locale_len ; n++) {
		fprintf(fp, "#define COL_IDX_%s %5d\n", der_locale_array[n].name, n+1);
		fprintf(fp, "\t/* Derived Locale %4d: %s (%.12s) */\n",
				n, der_locale_array[n].name,
				base_locale_array[der_locale_array[n].base_idx].name);
		DUMPn(base_idx);
		DUMPn(undefined_idx);
		DUMPn(overrides_offset);
		DUMPn(multistart_offset);
	}
#undef DUMPn

	fprintf(fp, "\n");

	dump_u16_array(fp, index2weight_buffer, index2weight_len, "index2weight");
	dump_u16_array(fp, index2ruleidx_buffer, index2ruleidx_len, "index2ruleidx");
	dump_u16_array(fp, multistart_buffer, multistart_len, "multistart");
	dump_u16_array(fp, override_buffer, override_len, "override");
	dump_u16_array(fp, ruletable_buffer, ruletable_len, "ruletable");
	dump_u16_array(fp, weightstr_buffer, weightstr_len, "weightstr");
	dump_u16_array(fp, wcs2colidt_buffer, wcs2colidt_len, "wcs2colidt");


	fprintf(fp,"}; /* %8lu */\n", collate_pos);

	fprintf(fp,"#define __lc_collate_data_LEN  %d\n\n", collate_pos);
}
