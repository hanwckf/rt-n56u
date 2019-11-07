#ifndef POPARSER_H
#define POPARSER_H
#include <unistd.h>

enum po_entry {
	pe_msgid = 0,
	pe_plural,
	pe_ctxt,
	pe_msgstr,
	pe_maxstr,
	pe_str = pe_maxstr,
	pe_invalid,
	pe_max,
};

struct po_info {
	enum po_entry type;
	char *text;
	char charset[12];
	unsigned int nplurals;
	size_t textlen;
};

typedef int (*poparser_callback)(struct po_info* info, void* user);

struct po_parser {
	struct po_info info;
	char *buf;
	size_t bufsize;
	enum po_entry prev_type;
	enum po_entry prev_rtype;
	unsigned curr_len;
	poparser_callback cb;
	void *cbdata;
};

void poparser_init(struct po_parser *p, char* workbuf, size_t bufsize, poparser_callback cb, void* cbdata);
int poparser_feed_line(struct po_parser *p, char* line, size_t buflen);
int poparser_finish(struct po_parser *p);

#endif
