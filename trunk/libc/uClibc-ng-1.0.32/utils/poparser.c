#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include "poparser.h"
#include "StringEscape.h"

#define streq(A, B) (!strcmp(A, B))
#define strstarts(S, W) (memcmp(S, W, sizeof(W) - 1) ? NULL : (S + (sizeof(W) - 1)))

static unsigned fuzzymark = 0;
static enum po_entry get_type_and_start(struct po_info *info, char* lp, char* end, size_t *stringstart) {
	enum po_entry result_type;
	char *x, *y;
	size_t start = (size_t) lp;
	while(isspace(*lp) && lp < end) lp++;
	if(lp[0] == '#') {
		char *s;
		if((s = strstr(lp, ", fuzzy"))) {
			if(fuzzymark != 0) fuzzymark++;
			else fuzzymark=2;
		}
		inv:
		*stringstart = 0;
		return pe_invalid;
	} else if((y = strstarts(lp, "msg"))) {
		if((x = strstarts(y, "id")) && isspace(*x))
			result_type = pe_msgid;
		else if  ((x = strstarts(y, "id_plural")) && isspace(*x))
			result_type = pe_plural;
		else if  ((x = strstarts(y, "ctxt")) && isspace(*x))
			result_type = pe_ctxt;
		else if ((x = strstarts(y, "str")) && (isspace(*x) ||
			(x[0] == '[' && (x[1]-'0') < info->nplurals && x[2] == ']' && (x += 3) && isspace(*x)))) 
			result_type = pe_msgstr;
		else
			goto inv;
		while(isspace(*x) && x < end) x++;
		if(*x != '"') abort();
		conv:
		*stringstart = ((size_t) x - start) + 1;
	} else if(lp[0] == '"') {
		if(!(*info->charset)) {
			if((x = strstr(lp, "charset="))) {
				// charset=xxx\\n
				int len = strlen(x+=8) - 4;
				assert(len <= 11);
				if(strncmp(x, "UTF-8", 5) && strncmp(x, "utf-8", 5)) {
					memcpy(info->charset, x, len);
					info->charset[len] = 0;
				}
			}
		}
		if((x = strstr(lp, "nplurals=")))
			if(*(x+9) - '0')
				info->nplurals = *(x+9) - '0';
		result_type = pe_str;
		x = lp;
		goto conv;
	} else {
		goto inv;
	}
	return result_type;
}

/* expects a pointer to the first char after a opening " in a string,
 * converts the string into convbuf, and returns the length of that string */
static size_t get_length_and_convert(struct po_info *info, char* x, char* end, char* convbuf, size_t convbuflen) {
	size_t result = 0;
	char* e = x + strlen(x);
	assert(e > x && e < end && *e == 0);
	e--;
	while(isspace(*e)) e--;
	if(*e != '"') abort();
	*e = 0;
	char *s;
	if(*info->charset) {
		iconv_t ret = iconv_open("UTF-8", info->charset);
		if(ret != (iconv_t)-1) {
			size_t a=end-x, b=a*4;
			char mid[b], *midp=mid;
			iconv(iconv_open("UTF-8", info->charset), &x, &a, &midp, &b);
			if((s = strstr(mid, "charset=")))
				memcpy(s+8, "UTF-8\\n\0", 8);
			result = unescape(mid, convbuf, convbuflen);
		// iconv doesnt recognize the encoding
		} else 	result = unescape(x, convbuf, convbuflen);
	} else 	result = unescape(x, convbuf, convbuflen);
	return result;
}


void poparser_init(struct po_parser *p, char* workbuf, size_t bufsize, poparser_callback cb, void* cbdata) {
	p->buf = workbuf;
	p->bufsize = bufsize;
	p->cb = cb;
	p->prev_type = pe_invalid;
	p->prev_rtype = pe_invalid;
	p->curr_len = 0;
	p->cbdata = cbdata;
	*(p->info.charset) = 0;
	// nplurals = 2 by default
	p->info.nplurals = 2;
	fuzzymark = 0;
}

enum lineactions {
	la_incr,
	la_proc,
	la_abort,
	la_nop,
	la_max,
};

/* return 0 on success */
int poparser_feed_line(struct po_parser *p, char* line, size_t buflen) {
	char *convbuf = p->buf;
	size_t convbuflen = p->bufsize;
	size_t strstart;

	static const enum lineactions action_tbl[pe_max][pe_max] = {
		// pe_str will never be set as curr_type
		[pe_str] = {
			[pe_str] = la_abort,
			[pe_msgid] = la_abort,
			[pe_ctxt] = la_abort,
			[pe_plural] = la_abort,
			[pe_msgstr] = la_abort,
			[pe_invalid] = la_abort,
		},
		[pe_msgid] = {
			[pe_str] = la_incr,
			[pe_msgid] = la_abort,
			[pe_ctxt] = la_abort,
			[pe_plural] = la_proc,
			[pe_msgstr] = la_proc,
			[pe_invalid] = la_proc,
		},
		[pe_ctxt] = {
			[pe_str] = la_incr,
			[pe_msgid] = la_proc,
			[pe_ctxt] = la_abort,
			[pe_plural] = la_abort,
			[pe_msgstr] = la_abort,
			[pe_invalid] = la_proc,
		},
		[pe_plural] = {
			[pe_str] = la_incr,
			[pe_msgid] = la_abort,
			[pe_ctxt] = la_abort,
			[pe_plural] = la_abort,
			[pe_msgstr] = la_proc,
			[pe_invalid] = la_proc,
		},
		[pe_msgstr] = {
			[pe_str] = la_incr,
			[pe_msgid] = la_proc,
			[pe_ctxt] = la_proc,
			[pe_plural] = la_abort,
			[pe_msgstr] = la_proc,
			[pe_invalid] = la_proc,
		},
		[pe_invalid] = {
			[pe_str] = la_nop,
			[pe_msgid] = la_incr,
			[pe_ctxt] = la_incr,
			[pe_plural] = la_nop,
			[pe_msgstr] = la_nop,
			[pe_invalid] = la_nop,
		},
	};

	enum po_entry type;

	type = get_type_and_start(&p->info, line, line + buflen, &strstart);
	if(p->prev_rtype != pe_invalid && action_tbl[p->prev_rtype][type] == la_abort)
		abort();
	if(type != pe_invalid && type != pe_str)
		p->prev_rtype = type;
	if(fuzzymark) {
		if(type == pe_ctxt && fuzzymark == 1) fuzzymark--;
		if(type == pe_msgid) fuzzymark--;
		if(fuzzymark > 0) return 0;
	}
	switch(action_tbl[p->prev_type][type]) {
		case la_incr:
			assert(type == pe_msgid || type == pe_msgstr || type == pe_str || type == pe_plural || pe_ctxt);
			p->curr_len += get_length_and_convert(&p->info, line + strstart, line + buflen, convbuf + p->curr_len, convbuflen - p->curr_len);
			break;
		case la_proc:
			assert(p->prev_type == pe_msgid || p->prev_type == pe_msgstr || p->prev_type == pe_plural || p->prev_type == pe_ctxt);
			p->info.text = convbuf;
			p->info.textlen = p->curr_len;
			p->info.type = p->prev_type;
			p->cb(&p->info, p->cbdata);
			if(type != pe_invalid)
				p->curr_len = get_length_and_convert(&p->info, line + strstart, line + buflen, convbuf, convbuflen);
			else
				p->curr_len = 0;
			break;
		case la_nop:
			break;
		case la_abort:
		default:
			abort();
			// todo : return error code
	}
	if(type != pe_str) {
		p->prev_type = type;
	}
	return 0;
}

int poparser_finish(struct po_parser *p) {
	char empty[4] = "";
	return poparser_feed_line(p, empty, sizeof(empty));
}
