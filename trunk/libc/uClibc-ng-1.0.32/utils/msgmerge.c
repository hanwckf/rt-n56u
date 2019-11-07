/* msgfmt utility (C) 2012 rofl0r
 * released under the MIT license, see LICENSE for details */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "poparser.h"
#include "StringEscape.h"

__attribute__((noreturn))
static void syntax(void) {
	fprintf(stdout,
	"Usage: msgmerge [OPTION] def.po ref.pot\n");
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

struct fiLes {
	FILE *out;
	/* we can haz 3 different input files:
	* the .pot, which is the file containing only the ripped out strings from the program
	* (and no translations)
	* a .po, which contains translations and strings made from a previous .pot from that same source file,
	* a compendium, which is basically a huge po file containing all sorts of strings (msgid's) and translations (msgstr's)
	*/
	FILE *po;
	FILE *pot;
	FILE *compend;
	int plural_count;
	char convbuf[16384];
	enum po_entry prev_type;
};

/* currently we only output input strings as output strings
 * i.e. there is no translation lookup at all */
int process_line_callback(struct po_info* info, void* user) {
	struct fiLes* file = (struct fiLes*) user;

	// escape what is unescaped automatically by lib
	escape(info->text, file->convbuf, sizeof(file->convbuf));
	switch (info->type) {
	case pe_msgid:
		file->plural_count = 1;
		fprintf(file->out, "\nmsgid \"%s\"\n", file->convbuf);
		file->prev_type = info->type;
		break;
	case pe_ctxt:
		fprintf(file->out, "msgctxt \"%s\"\n", file->convbuf);
		break;
	case pe_plural:
		fprintf(file->out, "msgid_plural \"%s\"\n", file->convbuf);
		file->prev_type = info->type;
		break;
	case pe_msgstr:
		if (file->prev_type == pe_plural) {
			fprintf(file->out, "msgstr[%d] \"%s\"\n", file->plural_count++, file->convbuf);
		} else {
			fprintf(file->out, "msgstr \"%s\"\n", file->convbuf);
		}
		break;
	}
	return 0;
}

int process(struct fiLes *files, int update, int backup) {
	(void) update; (void) backup;
	struct po_parser pb, *p = &pb;
	char line[4096], conv[8192], *lb;
	poparser_init(p, conv, sizeof(conv), process_line_callback, files);
	while((lb = fgets(line, sizeof(line), files->po))) {
		poparser_feed_line(p, lb, sizeof(line));
	}
	poparser_finish(p);
	return 0;
}

void set_file(int out, char* fn, FILE** dest) {
	if(streq(fn, "-")) {
		*dest = out ? stdout : stdin;
	} else {
		*dest = fopen(fn, out ? "w" : "r");
	}
	if(!*dest) {
		perror("fopen");
		exit(1);
	}
}

int getbackuptype(char* str) {
	if(!str || !*str || streq(str, "none") || streq(str, "off"))
		return 0;
	else if(streq(str, "t") || streq(str, "numbered"))
		return 1;
	else if(streq(str, "nil") || streq(str, "existing"))
		return 2;
	else if(streq(str, "simple") || streq(str, "never"))
		return 3;
	else syntax();
}

int main(int argc, char**argv) {
	if(argc == 1) syntax();
	int arg = 1;
	struct expect {
		int out;
		int po;
		int pot;
		int compend;
	} expect_fn = {
		.out = 0,
		.po = 1,
		.pot = 0,
		.compend = 0,
	};
	struct fiLes files = {0,0,0,0,1,0};
	char* backup_suffix = getenv("SIMPLE_BACKUP_SUFFIX");
	if(!backup_suffix) backup_suffix = "~";
	int update = 0;
	int backup = getbackuptype(getenv("VERSION_CONTROL"));
	char* dest;
	set_file(1, "-", &files.out);
#define A argv[arg]
	for(; arg < argc; arg++) {
		if(A[0] == '-') {
			if(A[1] == '-') {
				if(
					streq(A+2, "strict") ||
					streq(A+2, "properties-input") ||
					streq(A+2, "properties-output") ||
					streq(A+2, "stringtable-input") ||
					streq(A+2, "stringtable-output") ||
					streq(A+2, "no-fuzzy-matching") ||
					streq(A+2, "multi-domain") ||
					streq(A+2, "previous") ||
					streq(A+2, "escape") ||
					streq(A+2, "no-escape") ||
					streq(A+2, "force-po") ||
					streq(A+2, "indent") ||
					streq(A+2, "add-location") ||
					streq(A+2, "no-location") ||
					streq(A+2, "no-wrap") ||
					streq(A+2, "sort-output") ||
					streq(A+2, "sort-by-file") ||

					strstarts(A+2, "lang=") ||
					strstarts(A+2, "color") || // can be --color or --color=xxx
					strstarts(A+2, "style=") ||
					strstarts(A+2, "width=") ||

					streq(A+2, "verbose") ||
					streq(A+2, "quiet") ||
					streq(A+2, "silent") ) {
				} else if(streq(A+2, "version")) {
					version();
				} else if((dest = strstarts(A+2, "output-file="))) {
					set_file(1, dest, &files.out);
				} else if((dest = strstarts(A+2, "compendium="))) {
					set_file(1, dest, &files.compend);
				} else if((dest = strstarts(A+2, "suffix="))) {
					backup_suffix = dest;
				} else if((dest = strstarts(A+2, "directory="))) {
					goto nodir;
				} else if((dest = strstarts(A+2, "backup"))) {
					if (*dest == '=')
						backup = getbackuptype(dest + 1);
					else
						backup = 0;
				} else if(streq(A+2, "update")) {
					set_update:
					update = 1;
				} else if(streq(A+2, "help")) syntax();

			} else if(streq(A + 1, "o")) {
				expect_fn.out = 1;
			} else if(streq(A + 1, "C")) {
				expect_fn.compend = 1;
			} else if(streq(A + 1, "U")) {
				goto set_update;
			} else if(
				streq(A+1, "m") ||
				streq(A+1, "N") ||
				streq(A+1, "P") ||
				streq(A+1, "e") ||
				streq(A+1, "E") ||
				streq(A+1, "i") ||
				streq(A+1, "p") ||
				streq(A+1, "w") ||
				streq(A+1, "s") ||
				streq(A+1, "F") ||
				streq(A+1, "V") ||
				streq(A+1, "q")
			) {

			} else if (streq(A+1, "v")) {
				version();
			} else if (streq(A+1, "D")) {
				// no support for -D at this time
				nodir:
				fprintf(stderr, "EINVAL\n");
				exit(1);
			} else if (streq(A+1, "h")) {
				syntax();
			} else if(expect_fn.out) {
				if(update && streq(A, "/dev/null")) return 0;
				set_file(1, A, &files.out);
				expect_fn.out = 0;
			} else if(expect_fn.compend) {
				set_file(1, A, &files.compend);
				expect_fn.compend = 0;
			} else if(expect_fn.po) {
				if(update && streq(A, "/dev/null")) return 0;
				set_file(0, A, &files.po);
				expect_fn.po = 0;
				expect_fn.pot = 1;
			} else if(expect_fn.pot) {
				if(update && streq(A, "/dev/null")) return 0;
				set_file(0, A, &files.pot);
				expect_fn.pot = 0;
			}

		} else if(expect_fn.out) {
			if(update && streq(A, "/dev/null")) return 0;
			set_file(1, A, &files.out);
			expect_fn.out = 0;
		} else if(expect_fn.compend) {
			set_file(1, A, &files.compend);
			expect_fn.compend = 0;
		} else if(expect_fn.po) {
			if(update && streq(A, "/dev/null")) return 0;
			set_file(0, A, &files.po);
			expect_fn.po = 0;
			expect_fn.pot = 1;
		} else if(expect_fn.pot) {
			if(update && streq(A, "/dev/null")) return 0;
			set_file(0, A, &files.pot);
			expect_fn.pot = 0;
		}
	}
	if(update) {
		fprintf(stdout, "warning: update functionality unimplemented\n");
		return 0;
	}
	if(!files.out || !files.po || !files.pot) syntax();
	int ret = process(&files, update, backup);
	FILE** filearr = (FILE**) &files;
	unsigned i;
	for (i = 0; i < 4; i++) {
		if(filearr[i] != NULL) fflush(filearr[i]);
	}
	for (i = 0; i < 4; i++) {
		if(
			filearr[i] != NULL &&
			filearr[i] != stdout &&
			filearr[i] != stdin
		) fclose(filearr[i]);
	}
	return ret;
}
