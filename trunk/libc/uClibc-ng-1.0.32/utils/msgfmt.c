/* msgfmt utility (C) 2012 rofl0r
 * released under the MIT license, see LICENSE for details */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "poparser.h"

// in DO_NOTHING mode, we simply write the msgid twice, once for msgid, once for msgstr.
// TODO: maybe make it write "" instead of echoing the msgid.
//#define DO_NOTHING

__attribute__((noreturn))
static void syntax(void) {
	fprintf(stdout,
	"Usage: msgfmt [OPTION] filename.po ...\n");
	exit(1);
}

__attribute__((noreturn))
static void version(void) {
	fprintf(stdout,
		"these are not (GNU gettext-tools) 99.9999.9999\n");
	exit(0);
}

#define streq(A, B) (!strcmp(A, B))
#define strstarts(S, W) (memcmp(S, W, sizeof(W) - 1) ? NULL : (S + (sizeof(W) - 1)))

struct mo_hdr {
	unsigned magic;
	int rev;
	unsigned numstring;
	unsigned off_tbl_org;
	unsigned off_tbl_trans;
	unsigned hash_tbl_size;
	unsigned off_tbl_hash;
};

/* file layout:
	header
	strtable (lenghts/offsets)
	transtable (lenghts/offsets)
	[hashtable]
	strings section
	translations section */

const struct mo_hdr def_hdr = {
	0x950412de,
	0,
	0,
	sizeof(struct mo_hdr),
	0,
	0,
	0,
};


// pass 0: collect numbers of strings, calculate size and offsets for tables
// print header
// pass 1: create in-memory string tables
enum passes {
	pass_first = 0,
	pass_collect_sizes = pass_first,
	pass_second,
	pass_max,
};

struct strtbl {
	unsigned len, off;
};

struct strmap {
	struct strtbl str, *trans;
};

struct callbackdata {
	enum passes pass;
	unsigned off;
	FILE* out;
	unsigned msgidbuf1_len;
	unsigned msgidbuf2_len;
	unsigned pluralbuf1_len;
	unsigned pluralbuf2_len;
	unsigned ctxtbuf_len;
	unsigned msgstr1_len;
	unsigned msgstr2_len;
	unsigned pluralstr_count;
	unsigned string_maxlen;
	char* msgidbuf1;
	char* msgidbuf2;
	char* pluralbuf1;
	char* pluralbuf2;
	char* msgctxtbuf;
	char* msgstrbuf1;
	char* msgstrbuf2;
	unsigned priv_type;
	unsigned priv_len;
	unsigned num[pe_maxstr];
	unsigned len[pe_maxstr];
	struct strmap *strlist;
	struct strtbl *translist;
	char *strbuffer[pe_maxstr];
	unsigned stroff[pe_maxstr];
	unsigned curr[pe_maxstr];
};

static struct callbackdata *cb_for_qsort;
int strmap_comp(const void *a_, const void *b_) {
	const struct strmap *a = a_, *b = b_;
	return strcmp(cb_for_qsort->strbuffer[0] + a->str.off, cb_for_qsort->strbuffer[0] + b->str.off);
}

enum sysdep_types {
	st_priu32 = 0,
	st_priu64,
	st_priumax,
	st_max
};

static const char sysdep_str[][10]={
	[st_priu32]  = "\x08<PRIu32>",
	[st_priu64]  = "\x08<PRIu64>",
	[st_priumax] = "\x09<PRIuMAX>",
};
static const char sysdep_repl[][8]={
	[st_priu32]  = "\x02lu\0u",
	[st_priu64]  = "\x02lu\0llu",
	[st_priumax] = "\x01ju"
};
static const char *get_repl(enum sysdep_types type, unsigned nr) {
	assert(nr < (unsigned)sysdep_repl[type][0]);
	const char* p = sysdep_repl[type]+1;
	while(nr--) p+=strlen(p)+1;
	return p;
}
static void replace(char* text, unsigned textlen, const char* what, const char * with) {
	char*p = text;
	size_t la = strlen(what), li=strlen(with);
	assert(la >= li);
	for(p=text;textlen >= la;) {
		if(!memcmp(p,what,la)) {
			memcpy(p, with, li);
			textlen -= la;
			memmove(p+li,p+la,textlen+1);
			p+=li;
		} else {
			p++;
			textlen--;
		}
	}
}
static unsigned get_form(enum sysdep_types type, unsigned no, unsigned occurences[st_max]) {
	unsigned i,divisor = 1;
	for(i=type+1;i<st_max;i++) if(occurences[i]) divisor *= sysdep_repl[i][0];
	return (no/divisor)%sysdep_repl[type][0];
}
static char** sysdep_transform(const char* text, unsigned textlen, unsigned *len, unsigned *count, int simulate) {
	unsigned occurences[st_max] = {0};
	const char *p=text,*o;
	unsigned i,j, l = textlen;
	while(l && (o=strchr(p, '<'))) {
		l-=o-p;p=o;
		unsigned f = 0;
		for(i=0;i<st_max;i++)
		if(l>=(unsigned)sysdep_str[i][0] && !memcmp(p,sysdep_str[i]+1,sysdep_str[i][0])) {
			occurences[i]++;
			f=1;
			p+=sysdep_str[i][0];
			l-=sysdep_str[i][0];
			break;
		}
		if(!f) p++,l--;
	}
	*count = 1;
	for(i=0;i<st_max;i++) if(occurences[i]) *count *= sysdep_repl[i][0];
	l = textlen * *count;
	for(i=0;i<*count;i++) for(j=0;j<st_max;j++)
	if(occurences[j]) l-= occurences[j] * (sysdep_str[j][0] - strlen(get_repl(j, get_form(j, i, occurences))));
	*len = l+*count-1;

	char **out = 0;
	if(!simulate) {
		out = malloc((sizeof(char*)+textlen+1) * *count);
		assert(out);
		char *p = (void*)(out+*count);
		for(i=0;i<*count;i++) {
			out[i]=p;
			memcpy(p, text, textlen+1);
			p+=textlen+1;
		}
		for(i=0;i<*count;i++) for(j=0;j<st_max;j++)
		if(occurences[j])
			replace(out[i], textlen, sysdep_str[j]+1, get_repl(j, get_form(j, i, occurences)));
	}

	return out;
}

static inline void writemsg(struct callbackdata *d) {
	if(d->msgidbuf1_len != 0) {
		if(!d->strlist[d->curr[pe_msgid]].str.off)
			d->strlist[d->curr[pe_msgid]].str.off=d->stroff[pe_msgid];

		if(d->ctxtbuf_len != 0) {
			memcpy(d->strbuffer[pe_msgid] + d->stroff[pe_msgid], d->msgctxtbuf, d->ctxtbuf_len);
			d->strlist[d->curr[pe_msgid]].str.len+=d->ctxtbuf_len;
			d->stroff[pe_msgid]+=d->ctxtbuf_len;
		}
		memcpy(d->strbuffer[pe_msgid] + d->stroff[pe_msgid], d->msgidbuf1, d->msgidbuf1_len);
		d->stroff[pe_msgid]+=d->msgidbuf1_len;
		d->strlist[d->curr[pe_msgid]].str.len+=d->msgidbuf1_len-1;
		if(d->pluralbuf1_len != 0) {
			memcpy(d->strbuffer[pe_msgid] + d->stroff[pe_msgid], d->pluralbuf1, d->pluralbuf1_len);
			d->strlist[d->curr[pe_msgid]].str.len+=d->pluralbuf1_len;
			d->stroff[pe_msgid]+=d->pluralbuf1_len;
		}
		d->curr[pe_msgid]++;
	}
	if(d->msgidbuf2_len != 0) {
		if(!d->strlist[d->curr[pe_msgid]].str.off)
			d->strlist[d->curr[pe_msgid]].str.off=d->stroff[pe_msgid];

		if(d->ctxtbuf_len != 0) {
			memcpy(d->strbuffer[pe_msgid] + d->stroff[pe_msgid], d->msgctxtbuf, d->ctxtbuf_len);
			d->strlist[d->curr[pe_msgid]].str.len+=d->ctxtbuf_len;
			d->stroff[pe_msgid]+=d->ctxtbuf_len;
		}
		memcpy(d->strbuffer[pe_msgid] + d->stroff[pe_msgid], d->msgidbuf2, d->msgidbuf2_len);
		d->stroff[pe_msgid]+=d->msgidbuf2_len;
		d->strlist[d->curr[pe_msgid]].str.len+=d->msgidbuf2_len-1;
		if(d->pluralbuf2_len != 0) {
			memcpy(d->strbuffer[pe_msgid] + d->stroff[pe_msgid], d->pluralbuf2, d->pluralbuf2_len);
			d->strlist[d->curr[pe_msgid]].str.len+=d->pluralbuf2_len;
			d->stroff[pe_msgid]+=d->pluralbuf2_len;
		}
		d->curr[pe_msgid]++;
	}

	d->pluralbuf2_len=d->pluralbuf1_len=d->ctxtbuf_len=d->msgidbuf1_len=d->msgidbuf2_len=0;
}

static inline void writestr(struct callbackdata *d, struct po_info *info) {
	// msgid xx; msgstr ""; is widely happened, it's invalid

	// https://github.com/sabotage-linux/gettext-tiny/issues/1
	// no invalid, when empty, check d->num[pe_msgid]
	if(!d->pluralstr_count && d->num[pe_msgid] > 0) {
		d->len[pe_msgid]-=d->msgidbuf1_len;
		d->len[pe_msgid]-=d->msgidbuf2_len;
		d->len[pe_plural]-=d->pluralbuf1_len;
		d->len[pe_plural]-=d->pluralbuf2_len;
		d->len[pe_ctxt]-=d->ctxtbuf_len;
		d->len[pe_msgstr]--;
		d->num[pe_msgid]--;
		d->num[pe_msgstr]--;
		d->pluralbuf2_len=d->pluralbuf1_len=d->ctxtbuf_len=d->msgidbuf1_len=d->msgidbuf2_len=d->msgstr1_len=d->msgstr2_len=d->pluralstr_count=0;
		return;
	}

	if(d->pluralstr_count && d->pluralstr_count <= info->nplurals) {
		writemsg(d);
		// plural <= nplurals is allowed
		d->translist[d->curr[pe_msgstr]].len=d->msgstr1_len-1;
		d->translist[d->curr[pe_msgstr]].off=d->stroff[pe_msgstr];
		d->strlist[d->curr[pe_msgstr]].trans = &d->translist[d->curr[pe_msgstr]];

		memcpy(d->strbuffer[pe_msgstr] + d->stroff[pe_msgstr], d->msgstrbuf1, d->msgstr1_len);
		d->stroff[pe_msgstr]+=d->msgstr1_len;
		d->curr[pe_msgstr]++;

		if(d->msgstr2_len) {
			d->translist[d->curr[pe_msgstr]].len=d->msgstr2_len-1;
			d->translist[d->curr[pe_msgstr]].off=d->stroff[pe_msgstr];
			d->strlist[d->curr[pe_msgstr]].trans = &d->translist[d->curr[pe_msgstr]];

			memcpy(d->strbuffer[pe_msgstr] + d->stroff[pe_msgstr], d->msgstrbuf2, d->msgstr2_len);
			d->stroff[pe_msgstr]+=d->msgstr2_len;
			d->curr[pe_msgstr]++;
		}

		d->msgstr1_len=d->msgstr2_len=d->pluralstr_count=0;
	}
}

int process_line_callback(struct po_info* info, void* user) {
	struct callbackdata *d = (struct callbackdata *) user;
	assert(info->type == pe_msgid || info->type == pe_ctxt || info->type == pe_msgstr || info->type == pe_plural);
	char **sysdeps;
	unsigned len, count, i, l;
	switch(d->pass) {
		case pass_collect_sizes:
			sysdep_transform(info->text, info->textlen, &len, &count, 1);
			d->num[info->type] += count;
			if(info->type == pe_msgid && count == 2 && d->priv_type == pe_ctxt) {
				// ctxt meets msgid with sysdeps, multiply num and len to suit it
				d->len[pe_ctxt] += d->priv_len +1;
				d->num[pe_ctxt]++;
			}
			if(count != 1 && info->type == pe_ctxt) {
				// except msgid, str, plural, all other types should not have sysdeps
				abort();
			}

			d->priv_type = info->type;
			d->priv_len = len;
			d->len[info->type] += len +1;

			if(len+1 > d->string_maxlen)
				d->string_maxlen = len+1;
			break;
		case pass_second:
			sysdeps = sysdep_transform(info->text, info->textlen, &len, &count, 0);
			for(i=0;i<count;i++) {
				l = strlen(sysdeps[i]);
				assert(l+1 <= d->string_maxlen);
				if(info->type == pe_msgid) {
					if(i==0 && d->msgidbuf1_len)
						writestr(d, info);

					// just copy, it's written down when writemsg()
					if(i==0) {
						memcpy(d->msgidbuf1, sysdeps[i], l+1);
						d->msgidbuf1_len = l+1;
					} else {
						memcpy(d->msgidbuf2, sysdeps[i], l+1);
						d->msgidbuf2_len = l+1;
					}
				} else if(info->type == pe_plural) {
					if(i==0) {
						memcpy(d->pluralbuf1, sysdeps[i], l+1);
						d->pluralbuf1_len = l+1;
					} else {
						memcpy(d->pluralbuf2, sysdeps[i], l+1);
						d->pluralbuf2_len = l+1;
					}
				} else if(info->type == pe_ctxt) {
					writestr(d, info);
					d->ctxtbuf_len = l+1;
					memcpy(d->msgctxtbuf, sysdeps[i], l);
					d->msgctxtbuf[l] = 0x4;//EOT
				} else {
					// just copy, it's written down when writestr()
					if(l) {
						if(i==0) {
							memcpy(&d->msgstrbuf1[d->msgstr1_len], sysdeps[i], l+1);
							d->msgstr1_len += l+1;
							d->pluralstr_count++;
						} else {
							// sysdeps exist
							memcpy(&d->msgstrbuf2[d->msgstr2_len], sysdeps[i], l+1);
							d->msgstr2_len += l+1;
						}
					}
				}
			}
			free(sysdeps);
			break;
		default:
			abort();
	}
	return 0;
}

int process(FILE *in, FILE *out) {
	struct mo_hdr mohdr = def_hdr;
	char line[4096]; char *lp;
	char convbuf[16384];

	struct callbackdata d = {
		.num = {
			[pe_msgid] = 0,
			[pe_msgstr] = 0,
			[pe_plural] = 0,
			[pe_ctxt] = 0,
		},
		.len = {
			[pe_msgid] = 0,
			[pe_msgstr] = 0,
			[pe_plural] = 0,
			[pe_ctxt] = 0,
		},
		.off = 0,
		.out = out,
		.pass = pass_first,
		.ctxtbuf_len = 0,
		.pluralbuf1_len = 0,
		.pluralbuf2_len = 0,
		.msgidbuf1_len = 0,
		.msgidbuf2_len = 0,
		.msgstr1_len = 0,
		.msgstr2_len = 0,
		.pluralstr_count = 0,
		.string_maxlen = 0,
	};

	struct po_parser pb, *p = &pb;

	mohdr.off_tbl_trans = mohdr.off_tbl_org;
	for(d.pass = pass_first; d.pass <= pass_second; d.pass++) {
		if(d.pass == pass_second) {
			// start of second pass:
			// ensure we dont output when there's no strings at all
			if(d.num[pe_msgid] == 0) {
				return 1;
			}

			// check that data gathered in first pass is consistent
			if((d.num[pe_msgstr] < d.num[pe_msgid]) || (d.num[pe_msgstr] > (d.num[pe_msgid] + d.num[pe_plural] * (p->info.nplurals - 1)))) {
				// one should actually abort here,
				// but gnu gettext simply writes an empty .mo and returns success.
				//abort();
				fprintf(stderr, "warning: mismatch of msgid/msgstr count, writing empty .mo file\n");
				d.num[pe_msgid] = 0;
				return 0;
			}

			d.msgidbuf1 = calloc(d.string_maxlen*5+2*d.string_maxlen*p->info.nplurals, 1);
			d.msgidbuf2 = d.msgidbuf1 + d.string_maxlen;
			d.pluralbuf1 = d.msgidbuf2 + d.string_maxlen;
			d.pluralbuf2 = d.pluralbuf1 + d.string_maxlen;
			d.msgctxtbuf = d.pluralbuf2 + d.string_maxlen;
			d.msgstrbuf1 = d.msgctxtbuf + d.string_maxlen;
			d.msgstrbuf2 = d.msgstrbuf1 + d.string_maxlen*p->info.nplurals;

			d.strlist = calloc(d.num[pe_msgid] * sizeof(struct strmap), 1);
			d.translist = calloc(d.num[pe_msgstr] * sizeof(struct strtbl), 1);
			d.strbuffer[pe_msgid] = calloc(d.len[pe_msgid]+d.len[pe_plural]+d.len[pe_ctxt], 1);
			d.strbuffer[pe_msgstr] = calloc(d.len[pe_msgstr], 1);
			d.stroff[pe_msgid] = d.stroff[pe_msgstr] = 0;
			assert(d.msgidbuf1 && d.strlist && d.translist && d.strbuffer[pe_msgid] && d.strbuffer[pe_msgstr]);
		}

		poparser_init(p, convbuf, sizeof(convbuf), process_line_callback, &d);

		while((lp = fgets(line, sizeof(line), in))) {
			poparser_feed_line(p, lp, sizeof(line));
		}
		poparser_finish(p);
		if(d.pass == pass_second)
			writestr(&d, &p->info);

		if(d.pass == pass_second) {
			// calculate header fields from len and num arrays
			mohdr.numstring = d.num[pe_msgid];
			mohdr.off_tbl_org = sizeof(struct mo_hdr);
			mohdr.off_tbl_trans = mohdr.off_tbl_org + d.num[pe_msgid] * (sizeof(unsigned)*2);
			// set offset startvalue
			d.off = mohdr.off_tbl_trans + d.num[pe_msgid] * (sizeof(unsigned)*2);
		}
		fseek(in, 0, SEEK_SET);
	}

	cb_for_qsort = &d;
	qsort(d.strlist, d.num[pe_msgid], sizeof (struct strmap), strmap_comp);
	unsigned i;

	// print header
	fwrite(&mohdr, sizeof(mohdr), 1, out);
	for(i = 0; i < d.num[pe_msgid]; i++) {
		d.strlist[i].str.off += d.off;
		fwrite(&d.strlist[i].str, sizeof(struct strtbl), 1, d.out);
	}
	for(i = 0; i < d.num[pe_msgid]; i++) {
		d.strlist[i].trans->off += d.off + d.len[pe_msgid] + d.len[pe_plural] + d.len[pe_ctxt];
		fwrite(d.strlist[i].trans, sizeof(struct strtbl), 1, d.out);
	}
	fwrite(d.strbuffer[pe_msgid], d.len[pe_msgid]+d.len[pe_plural]+d.len[pe_ctxt], 1, d.out);
	fwrite(d.strbuffer[pe_msgstr], d.len[pe_msgstr], 1, d.out);

	return 0;
}


void set_file(int out, char* fn, FILE** dest) {
	if(streq(fn, "-")) {
		if(out) {
			*dest = stdout;
		} else {
			char b[4096];
			size_t n=0;
			FILE* tmpf = tmpfile();
			if(!tmpf)
				perror("tmpfile");

			while((n=fread(b, sizeof(*b), sizeof(b), stdin)) > 0)
				fwrite(b, sizeof(*b), n, tmpf);

			fseek(tmpf, 0, SEEK_SET);
			*dest = tmpf;
		}
	} else {
		*dest = fopen(fn, out ? "w" : "r");
	}
	if(!*dest) {
		perror("fopen");
		exit(1);
	}
}

int main(int argc, char**argv) {
	if(argc == 1) syntax();
	int arg = 1;
	FILE *out = NULL;
	FILE *in = NULL;
	int expect_in_fn = 1;
	char* locale = NULL;
	char* dest = NULL;
#define A argv[arg]
	for(; arg < argc; arg++) {
		if(A[0] == '-') {
			if(A[1] == '-') {
				if(
					streq(A+2, "java") ||
					streq(A+2, "java2") ||
					streq(A+2, "csharp") ||
					streq(A+2, "csharp-resources") ||
					streq(A+2, "tcl") ||
					streq(A+2, "qt") ||
					streq(A+2, "strict") ||
					streq(A+2, "properties-input") ||
					streq(A+2, "stringtable-input") ||
					streq(A+2, "use-fuzzy") ||
					strstarts(A+2, "alignment=") ||
					streq(A+2, "check") ||
					streq(A+2, "check-format") ||
					streq(A+2, "check-header") ||
					streq(A+2, "check-domain") ||
					streq(A+2, "check-compatibility") ||
					streq(A+2, "check-accelerators") ||
					streq(A+2, "no-hash") ||
					streq(A+2, "verbose") ||
					streq(A+2, "statistics") ||
					strstarts(A+2, "check-accelerators=") ||
					strstarts(A+2, "resource=")
				) {
				} else if((dest = strstarts(A+2, "locale="))) {
					locale = dest;
				} else if((dest = strstarts(A+2, "output-file="))) {
					set_file(1, dest, &out);
				} else if(streq(A+2, "version")) {
					version();
				} else if(streq(A+2, "help")) {
					syntax();
				} else if (expect_in_fn) {
					set_file(0, A, &in);
					expect_in_fn = 0;
				}
			} else if(streq(A + 1, "o")) {
				arg++;
				dest = A;
				set_file(1, A, &out);
			} else if(
				streq(A+1, "j") ||
				streq(A+1, "r") ||
				streq(A+1, "P") ||
				streq(A+1, "f") ||
				streq(A+1, "a") ||
				streq(A+1, "c") ||
				streq(A+1, "v") ||
				streq(A+1, "C")
			) {
			} else if (streq(A+1, "V")) {
				version();
			} else if (streq(A+1, "h")) {
				syntax();
			} else if (streq(A+1, "l")) {
				arg++;
				locale = A;
			} else if (streq(A+1, "d")) {
				arg++;
				dest = A;
			} else if (expect_in_fn) {
				set_file(0, A, &in);
				expect_in_fn = 0;
			}
		} else if (expect_in_fn) {
			set_file(0, A, &in);
			expect_in_fn = 0;
		}
	}

	if (locale != NULL && dest != NULL) {
		int sz = snprintf(NULL, 0, "%s/%s.msg", dest, locale);
		char msg[sz+1];
		snprintf(msg, sizeof(msg), "%s/%s.msg", dest, locale);
		FILE *fp = fopen(msg, "w");
		if (fp) {
			fclose(fp);
			return 0;
		} else return 1;
	}

	if(out == NULL) {
		dest = "messages.mo";
		set_file(1, "messages.mo", &out);
	}

	if(in == NULL || out == NULL) {
		return 1;
	}
	int ret = process(in, out);
	fflush(in); fflush(out);
	if(in != stdin) fclose(in);
	if(out != stdout) fclose(out);

	if (ret == 1) {
		return remove(dest);
	}
	return ret;
}
