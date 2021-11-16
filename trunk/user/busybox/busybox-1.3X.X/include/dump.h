/* vi: set sw=4 ts=4: */

PUSH_AND_SET_FUNCTION_VISIBILITY_TO_HIDDEN

enum dump_vflag_t { ALL, DUP, FIRST, WAIT };	/* -v values */

typedef struct PR {
	struct PR *nextpr;		/* next print unit */
	unsigned flags;			/* flag values */
	int bcnt;			/* byte count */
	char *cchar;			/* conversion character */
	char *fmt;			/* printf format */
	char *nospace;			/* no whitespace version */
} PR;

typedef struct FU {
	struct FU *nextfu;		/* next format unit */
	struct PR *nextpr;		/* next print unit */
	unsigned flags;			/* flag values */
	int reps;			/* repetition count */
	int bcnt;			/* byte count */
	char *fmt;			/* format string */
} FU;

typedef struct FS {			/* format strings */
	struct FS *nextfs;		/* linked list of format strings */
	struct FU *nextfu;		/* linked list of format units */
	int bcnt;
} FS;

typedef struct dumper_t {
	off_t dump_skip;                /* bytes to skip */
	int dump_length;                /* max bytes to read */
	smallint dump_vflag;            /*enum dump_vflag_t*/
	FS *fshead;
	const char *xxd_eofstring;
	off_t address;           /* address/offset in stream */
	long long xxd_displayoff;
} dumper_t;

dumper_t* alloc_dumper(void) FAST_FUNC;
extern void bb_dump_add(dumper_t *dumper, const char *fmt) FAST_FUNC;
extern int bb_dump_dump(dumper_t *dumper, char **argv) FAST_FUNC;

POP_SAVED_FUNCTION_VISIBILITY
